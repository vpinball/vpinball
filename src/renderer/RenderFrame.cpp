#include "core/stdafx.h"
#include "RenderFrame.h"
#include "RenderPass.h"
#include "RenderCommand.h"
#include "RenderDevice.h"

// Uncomment this for logging pass sorting/splitting
//#define LOG_PASS_SORTING

RenderFrame::RenderFrame(RenderDevice* renderDevice)
   : m_rd(renderDevice)
{
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
      if (nextPass != sortedPasses.end() && (*itPass)->m_rt == (*nextPass)->m_rt)
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
   if (m_passes.empty())
      return false;

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
            RenderPass* const splitPass = AddPass((*itPass)->m_name + " [Split before " + (*it)->m_dependency->m_name + "]", (*itPass)->m_rt);
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
   
   bool rendered = false;
   for (RenderPass* pass : sortedPasses)
      rendered |= pass->Execute(log);
   
   #if defined(ENABLE_BGFX)
   #elif defined(ENABLE_OPENGL)
   if (rendered)
      glFlush(); // Push command queue to the GPU without blocking (tells the GPU that the render queue is ready to be executed)
   #elif defined(ENABLE_DX9)
   CHECKD3D(m_rd->GetCoreDevice()->EndScene());
   #endif

   // Recycle commands & passes
   for (RenderPass* pass : m_passes)
      pass->RecycleCommands(m_commandPool);
   m_passPool.insert(m_passPool.end(), m_passes.begin(), m_passes.end());
   m_passes.clear();

   // Restore render/shader states
   m_rd->CopyRenderStates(false, *m_rdState);

   return rendered;
}
