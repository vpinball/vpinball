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

   // Sort passes to avoid useless render target switching, allow merging passes for better draw call sorting/batching, drop passes that do not contribute to the final pass
   RenderPass* finalPass = m_passes.back();
   vector<RenderPass*> sortedPasses;
   sortedPasses.reserve(m_passes.size());
   finalPass->SortPasses(sortedPasses, m_passes);
   finalPass = sortedPasses.back(); // we need to request it again since it may have changed due to pass merging

   // Add passes that are linked to a specific render command if any (needed for refraction probes)
   bool splitted = false;
   for (RenderPass* pass : sortedPasses)
   {
      pass->m_sortKey = 0;
      pass->SortCommands();
      // Split on command dependencies (commands that needs a pass to be executed just before them)
      for (std::vector<RenderCommand*>::iterator it = pass->m_commands.begin(); it != pass->m_commands.end(); ++it)
      {
         if ((*it)->m_dependency != nullptr)
         {
            // Create a pass from the first commands
            RenderPass* splitPass = AddPass(pass->m_name, pass->m_rt);
            splitPass->m_dependencies.insert(splitPass->m_dependencies.begin(), pass->m_dependencies.begin(), pass->m_dependencies.end());
            splitPass->m_commands.insert(splitPass->m_commands.begin(), pass->m_commands.begin(), it);
            // Continue with tail, adding dependencies on the splitted pass and the command's dependency
            (*it)->m_dependency->UpdateDependency(pass->m_rt, splitPass); // update to the latest state (filtered to only apply to first call in RenderPass to avoid cyclic dependencies when using a refraction probe multiple time)
            pass->AddPrecursor((*it)->m_dependency);
            pass->AddPrecursor(splitPass);
            pass->m_commands.erase(pass->m_commands.begin(), it);
            it = pass->m_commands.begin();
            splitted = true;
         }
      }
   }
   if (splitted)
   {
      sortedPasses.clear();
      finalPass->SortPasses(sortedPasses, m_passes);
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
   
   #if defined(ENABLE_OPENGL)
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
