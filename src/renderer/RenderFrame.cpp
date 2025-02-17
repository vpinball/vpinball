// license:GPLv3+

#include "core/stdafx.h"
#include "RenderFrame.h"
#include "RenderPass.h"
#include "RenderCommand.h"
#include "RenderDevice.h"

// Uncomment this for logging pass sorting/splitting
//#define LOG_PASS_SORTING

#if defined(ENABLE_DX9)
// Note: Nowadays the original code seems to be counter-productive, so we use the official
// pre-rendered frame mechanism instead where possible
// (e.g. all windows versions except for XP and no "EnableLegacyMaximumPreRenderedFrames" set in the registry)
/*
 * Class to limit the length of the GPU command buffer queue to at most 'numFrames' frames.
 * Excessive buffering of GPU commands creates high latency and can create stuttery overlong
 * frames when the CPU stalls due to a full command buffer ring.
 *
 * Calling Execute() within a BeginScene() / EndScene() pair creates an artificial pipeline
 * stall by locking a vertex buffer which was rendered from (numFrames-1) frames ago. This
 * forces the CPU to wait and let the GPU catch up so that rendering doesn't lag more than
 * numFrames behind the CPU. It does *NOT* limit the framerate itself, only the drawahead.
 * Note that VP is currently usually GPU-bound.
 *
 * This is similar to Flush() in later DX versions, but doesn't flush the entire command
 * buffer, only up to a certain previous frame.
 *
 * Use of this class has been observed to effectively reduce stutter at least on an NVidia/
 * Win7 64 bit setup. The queue limiting effect can be clearly seen in GPUView.
 *
 * The initial cause for the stutter may be that our command buffers are too big (two
 * packets per frame on typical tables, instead of one), so with more optimizations to
 * draw calls/state changes, none of this may be needed anymore.
 */
class DX9Flush
{
public:
   DX9Flush(RenderDevice* const pd3dDevice, const int numFrames)
      : m_pd3dDevice(pd3dDevice)
   {
      m_buffers.resize(numFrames, nullptr);
      m_curIdx = 0;
   }

   ~DX9Flush()
   {
      for (size_t i = 0; i < m_buffers.size(); ++i)
         delete m_buffers[i];
   }

   void Execute()
   {
      if (m_buffers.empty())
         return;

      if (m_buffers[m_curIdx])
      {
         Vertex3Ds pos(0.f, 0.f, 0.f);
         m_pd3dDevice->DrawMesh(m_pd3dDevice->m_basicShader, false, pos, 0.f, m_buffers[m_curIdx], RenderDevice::TRIANGLESTRIP, 0, 3);
      }

      m_curIdx = (m_curIdx + 1) % m_buffers.size();

      if (!m_buffers[m_curIdx])
      {
         VertexBuffer* vb = new VertexBuffer(m_pd3dDevice, 1024);
         m_buffers[m_curIdx] = new MeshBuffer(L"FrameLimiter"s, vb);
      }

      // idea: locking a static vertex buffer stalls the pipeline if that VB is still
      // in the GPU render queue. In effect, this lets the GPU catch up.
      Vertex3D_NoTex2* buf;
      m_buffers[m_curIdx]->m_vb->Lock(buf);
      memset(buf, 0, 3 * sizeof(buf[0]));
      buf[0].z = buf[1].z = buf[2].z = 1e5f; // single triangle, degenerates to point far off screen
      m_buffers[m_curIdx]->m_vb->Unlock();
   }

private:
   RenderDevice* const m_pd3dDevice;
   vector<MeshBuffer*> m_buffers;
   size_t m_curIdx;
};
#endif

RenderFrame::RenderFrame(RenderDevice* renderDevice)
   : m_rd(renderDevice)
{
   #if defined(ENABLE_DX9)
   // TODO remove legacy frame limiter for Windows XP
   const int maxPrerenderedFrames = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "MaxPrerenderedFrames"s, 0);
   const int EnableLegacyMaximumPreRenderedFrames = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "EnableLegacyMaximumPreRenderedFrames"s, 0);
   if (EnableLegacyMaximumPreRenderedFrames || renderDevice->GetCoreDeviceEx() == nullptr || maxPrerenderedFrames > 20)
      m_DX9Flush = new DX9Flush(renderDevice, maxPrerenderedFrames);
   #endif
}

RenderFrame::~RenderFrame()
{
   delete m_rdState;
   for (auto item : m_commandPool)
      delete item;
   for (auto item : m_passPool)
      delete item;
   for (auto item : m_passes)
      delete item;
   #ifdef ENABLE_DX9
   delete m_DX9Flush;
   #endif
}

RenderPass* RenderFrame::AddPass(const string& name, RenderTarget* const rt)
{
   RenderPass* pass;
   if (m_passPool.empty())
   {
      pass = new RenderPass(name, rt);
   }
   else
   {
      pass = m_passPool.back();
      m_passPool.pop_back();
      pass->Reset(name, rt);
   }
   m_passes.push_back(pass);
   return pass;
}

RenderCommand* RenderFrame::NewCommand()
{
   if (m_commandPool.empty())
   {
      return new RenderCommand(m_rd);
   }
   else
   {
      RenderCommand* item = m_commandPool.back();
      m_commandPool.pop_back();
      return item;
   }
}

void RenderFrame::SortPasses(RenderPass* finalPass, vector<RenderPass*>& sortedPasses)
{
   sortedPasses.reserve(m_passes.size());
   vector<RenderPass*> waitingPasses; // Used passes gathered recursively from final pass, waiting to be sorted
   waitingPasses.reserve(2 * m_passes.size());
   sortedPasses.push_back(finalPass); // start with the final pass
   waitingPasses.insert(waitingPasses.begin(), finalPass->m_dependencies.begin(), finalPass->m_dependencies.end());
   while (!waitingPasses.empty())
   {
      // Select first pass not yet inserted
      RenderPass* pass = waitingPasses.back();
      waitingPasses.pop_back();
      if (pass->m_sortKey != 0)
         continue;
      // Mark as inserted and enqueue dependencies for later insertion
      pass->m_sortKey = 1;
      waitingPasses.insert(waitingPasses.end(), pass->m_dependencies.begin(), pass->m_dependencies.end());
      // Find first consumers already placed in the sorted list
      std::vector<RenderPass*>::iterator itPass = sortedPasses.begin();
      while (FindIndexOf((*itPass)->m_dependencies, pass) == -1)
         ++itPass;
      std::vector<RenderPass*>::iterator itPassFirstConsumer = itPass;
      // Select insertion position, moving back from first consumer while dependencies & render target constraints are still satisfied, trying to reach a pass using the same RT to optimize merging
      while (true)
      {
         if (itPass == sortedPasses.begin()) // No optimisation found: defaults to place before first consumer
         {
            itPass = itPassFirstConsumer;
            break;
         }
         --itPass;
         if (FindIndexOf(pass->m_dependencies, (*itPass)) != -1 // Invalid since we would end up before one of our dependencies: defaults to place before first consumer
            || FindIndexOf((*itPass)->m_referencedRT, pass->m_rt) != -1) // Invalid since we would overwrite the needed content of a previous pass: defaults to place before first consumer
         {
            itPass = itPassFirstConsumer;
            break;
         }
         if ((*itPass)->m_rt == pass->m_rt) // Here would places us just after a pass on the same render target, place after it
         {
            break;
         }
      }
      // Insert pass before the selected place, inherit constraint from follower if any
      pass->m_referencedRT.insert(pass->m_referencedRT.begin(), (*itPass)->m_referencedRT.begin(), (*itPass)->m_referencedRT.end());
      itPass = sortedPasses.insert(itPass, pass);
      // Add render target constraints from our insertion position toward all consumers (to avoid inserting a pass that would overwrite the content of the render target while it is expected by a consumer)
      std::vector<RenderPass*>::iterator itInsertionPoint = itPass, itPassLastConsumer = itPass;
      while (itInsertionPoint != sortedPasses.end())
      {
         if (FindIndexOf((*itInsertionPoint)->m_dependencies, pass) != -1)
            itPassLastConsumer = itInsertionPoint;
         ++itInsertionPoint;
      }
      while (itPass != itPassLastConsumer)
      {
         ++itPass;
         (*itPass)->m_referencedRT.push_back(pass->m_rt);
      }
   }
   // Merge consecutive passes using the same RT [Warning, dependencies are not updated (not needed) making the log after sort incorrect]
   for (std::vector<RenderPass*>::iterator itPass = sortedPasses.begin(); itPass != sortedPasses.end();)
   {
      std::vector<RenderPass*>::iterator nextPass = itPass + 1;
      if (nextPass != sortedPasses.end() && (*nextPass)->m_mergeable && (*itPass)->m_rt == (*nextPass)->m_rt)
      {
         (*nextPass)->m_depthReadback |= (*itPass)->m_depthReadback;
         (*nextPass)->m_commands.insert((*nextPass)->m_commands.begin(), (*itPass)->m_commands.begin(), (*itPass)->m_commands.end());
         (*itPass)->m_commands.clear();
         itPass = sortedPasses.erase(itPass);
      }
      else
         ++itPass;
   }
}

bool RenderFrame::Execute(const bool log)
{
   bool rendered = false;
   if (!m_passes.empty())
   {
      // Save render/shader states
      if (m_rdState == nullptr)
         m_rdState = new RenderDeviceState(m_rd);
      m_rd->CopyRenderStates(true, *m_rdState);

      // Clear last render pass to avoid cross frame references
      for (RenderPass* pass : m_passes)
         pass->m_rt->m_lastRenderPass = nullptr;

      if (log)
      {
         PLOGI << "Rendering Frame";
         std::stringstream ss1;
         ss1 << "Submitted passes: [";
         for (RenderPass* pass : m_passes)
         {
            if (pass != m_passes[0])
               ss1 << ", ";
            ss1 << '\'' << pass->m_name << '\'';
            #ifdef LOG_PASS_SORTING
            std::stringstream ss;
            ss << "> Pass '" << pass->m_name << "' [RT=" << pass->m_rt->m_name << ", " << pass->m_commands.size() << " commands, Dependencies:";
            bool first = true;
            for (RenderPass* dep : pass->m_dependencies)
            {
               if (!first)
                  ss << ", ";
               first = false;
               ss << dep->m_name;
            }
            ss << ']';
            PLOGI << ss.str();
            #endif
         }
         PLOGI << ss1.str() << ']';
      }

      // Sort passes to satisfy dependencies, avoid useless render target switching, allow merging passes for better draw call sorting/batching, drop passes that do not contribute to the final pass
      vector<RenderPass*> sortedPasses; // FIXME use list since we are populating through insertions
      SortPasses(m_passes.back(), sortedPasses);
      // Sort commands & split on command level dependencies (commands that needs a pass to be executed just before them, used for refracting parts that uses a screen copy just before using it as a shading texture)
      for (std::vector<RenderPass*>::iterator itPass = sortedPasses.begin(); itPass != sortedPasses.end(); ++itPass)
      {
         (*itPass)->SortCommands();
         for (std::vector<RenderCommand*>::iterator it = (*itPass)->m_commands.begin(); it != (*itPass)->m_commands.end(); ++it)
         {
            if ((*it)->m_dependency != nullptr && (*it)->m_dependency->m_sortKey == 0)
            { // [Warning, dependencies are not updated (not needed) making the log after sort incorrect]
               // Add the pass just before the rendercommand. Warning: this only supports the refraction use scheme where we add a single pass to perform the screen copy
               (*it)->m_dependency->m_sortKey = 1;
               RenderPass* const splitPass = AddPass((*itPass)->m_name + " [Split before " + (*it)->m_dependency->m_name + ']', (*itPass)->m_rt);
               splitPass->m_commands.insert(splitPass->m_commands.begin(), (*itPass)->m_commands.begin(), it);
               (*itPass)->m_commands.erase((*itPass)->m_commands.begin(), it);
               it = (*itPass)->m_commands.begin(); // Continue with remaining commands after split
               itPass = sortedPasses.insert(itPass, splitPass) + 1; // itPass points to the pass after the inserted pass (so the remaining part of the initial pass)
               vector<RenderPass*> subPasses;
               SortPasses((*it)->m_dependency, subPasses);
               for (RenderPass* subPass : subPasses)
                  itPass = sortedPasses.insert(itPass, subPass) + 1; // itPass points after the added dependency pass
            }
         }
      }

      if (log)
      {
         std::stringstream ss1;
         ss1 << "Sorted passes: [";
         for (RenderPass* pass : sortedPasses)
         {
            if (pass != sortedPasses[0])
               ss1 << ", ";
            ss1 << '\'' << pass->m_name << '\'';
            #ifdef LOG_PASS_SORTING
            std::stringstream ss;
            ss << "> Pass '" << pass->m_name << "' [RT=" << pass->m_rt->m_name << ", " << pass->m_commands.size() << " commands, Dependencies:";
            bool first = true;
            for (RenderPass* dep : pass->m_dependencies)
            {
               if (!first)
                  ss << ", ";
               first = false;
               ss << dep->m_name;
            }
            ss << ']';
            PLOGI << ss.str();
            #endif
         }
         PLOGI << ss1.str() << ']';
      }

      #if defined(ENABLE_DX9)
      CHECKD3D(m_rd->GetCoreDevice()->BeginScene());
      #endif

      for (RenderPass* pass : sortedPasses)
         rendered |= pass->Execute(log);

      #if defined(ENABLE_BGFX)
      #elif defined(ENABLE_OPENGL)
      if (rendered)
         glFlush(); // Push command queue to the GPU without blocking (tells the GPU that the render queue is ready to be executed)
      #elif defined(ENABLE_DX9)
      CHECKD3D(m_rd->GetCoreDevice()->EndScene());
      // (Optionally) hack to force queue flushing of the driver. Can be used to artifically limit latency on DX9 (depends on OS/GFXboard/driver if still useful nowadays). This must be done after submiting render commands
      if (m_DX9Flush)
         m_DX9Flush->Execute();
      #endif

      // Recycle commands & passes
      for (RenderPass* pass : m_passes)
         pass->RecycleCommands(m_commandPool);
      m_passPool.insert(m_passPool.end(), m_passes.begin(), m_passes.end());
      m_passes.clear();

      // Restore render/shader states
      m_rd->CopyRenderStates(false, *m_rdState);
   }

   if (!m_endOfFrameCmds.empty())
   {
      rendered = true;
      for (const auto& cmd : m_endOfFrameCmds)
         cmd();
      m_endOfFrameCmds.clear();
   }

   return rendered;
}

void RenderFrame::Discard()
{
   // Recycle commands & passes
   for (RenderPass* pass : m_passes)
   {
      pass->RecycleCommands(m_commandPool);
      pass->m_rt->m_lastRenderPass = nullptr;
   }
   m_passPool.insert(m_passPool.end(), m_passes.begin(), m_passes.end());
   m_passes.clear();
   m_endOfFrameCmds.clear();
}
