#include "stdafx.h"
#include "RenderPass.h"
#include "RenderCommand.h"
#include "RenderDevice.h"

RenderPass::RenderPass(const string& name, RenderTarget* const rt, const bool ignoreStereo)
   : m_rt(rt)
   , m_ignoreStereo(ignoreStereo)
   , m_name(name)
{
}

RenderPass::~RenderPass()
{
   for (auto item : m_commands)
      delete item;
}

void RenderPass::RecycleCommands(std::vector<RenderCommand*>& commandPool)
{
   commandPool.insert(commandPool.end(), m_commands.begin(), m_commands.end());
   m_commands.clear();
}

void RenderPass::AddPrecursor(RenderPass* dependency)
{
   m_dependencies.push_back(dependency);
}

void RenderPass::Sort(vector<RenderPass*>& sortedPasses)
{
   // Perform a depth first sort down the precursor list, grouping by render target
   if (m_sortKey == 2) // Already processed
      return;
   assert(m_sortKey != 1); // Circular dependency between render pass
   m_sortKey = 1;
   RenderPass* me = nullptr;
   for (RenderPass* dependency : m_dependencies)
   {
      if (me == nullptr && dependency->m_rt == m_rt)
         me = dependency;
      else
         dependency->Sort(sortedPasses);
   }
   if (me) // Process pass on the same render target after others to allow merging
      me->Sort(sortedPasses);
   m_sortKey = 2;
   if (sortedPasses.size() > 0 && sortedPasses.back()->m_rt == m_rt)
   {
      // Merge passes
      sortedPasses.back()->m_commands.insert(sortedPasses.back()->m_commands.end(), m_commands.begin(), m_commands.end());
      m_commands.clear();
   }
   else
   {
      // Add passes
      sortedPasses.push_back(this);
   }
}

void RenderPass::Submit(RenderCommand* command)
{
   if (command->IsFullClear(m_rt->HasDepth()))
   {
      for (RenderCommand* cmd : m_commands)
         delete cmd;
      m_commands.clear();
      // FIXME remove dependencies on this render target (but not on others)
   }
   m_commands.push_back(command);
}

void RenderPass::Execute(const bool log)
{
   if (m_commands.size() > 0)
   {
      if (log)
         PLOGI << "Pass '" << m_name << "' [RT=" << m_rt->m_name << "]";

      #ifdef ENABLE_SDL
      if (GLAD_GL_VERSION_4_3)
      {
         std::stringstream passName;
         passName << m_name << " [RT=" << m_rt->m_name << "]";
         glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, passName.str().c_str());
      }
      #endif

      struct
      {
         // TODO add depth + depthbias sorting (for the time being, we rely on the render command to be pre sorted which breaks in VR)
         inline bool operator()(const RenderCommand* r1, const RenderCommand* r2) const
         {
            // Move Clear/Copy command at the beginniing of the pass
            if (!r1->IsDrawCommand())
               return true;
            if (!r2->IsDrawCommand())
               return false;
            // Move kickers before other draw calls.
            // Kickers disable depth test to be visible through playfield. This would make them to be rendered after opaques, but since they hack depth, they need to be rendered before balls
            // > The right fix would be to remove the kicker hack (use stencil masking, alpha punch or CSG on playfield), this would also solve rendering kicker in VR
            if (r1->GetShaderTechnique() == SHADER_TECHNIQUE_kickerBoolean || r1->GetShaderTechnique() == SHADER_TECHNIQUE_kickerBoolean_isMetal)
               return true;
            if (r2->GetShaderTechnique() == SHADER_TECHNIQUE_kickerBoolean || r2->GetShaderTechnique() == SHADER_TECHNIQUE_kickerBoolean_isMetal)
               return false;
            // Blended items: for the time being, don't sort blended command
            bool transparent1 = !r1->GetRenderState().IsOpaque();
            if (transparent1)
               return false;
            bool transparent2 = !r2->GetRenderState().IsOpaque();
            if (transparent2)
               return true;
            // Opaque items
            // FIXME kicker shader must be drawn before ball shader since it tweaks it's z to appear in front of the playfield, but must stay behind the ball
            if (r1->GetShaderTechnique() != r2->GetShaderTechnique())
            {
               // TODO sort by minimum depth of the technique
               /* if (m_min_depth[r1->technique] == m_min_depth[r2->technique])
                  return r1->technique < r2->technique;
               else
                  return m_min_depth[r1->technique] < m_min_depth[r2->technique];*/
               return r1->GetShaderTechnique() > r2->GetShaderTechnique();
            }
            if (r1->GetDepth() != r2->GetDepth())
               return r1->GetDepth() < r2->GetDepth();
            if (r1->IsDrawMeshCommand() && r2->IsDrawMeshCommand())
            {
               unsigned int mbS1 = r1->GetMeshBuffer()->GetSortKey();
               unsigned int mbS2 = r2->GetMeshBuffer()->GetSortKey();
               if (mbS1 != mbS2)
               {
                  return mbS1 < mbS2;
               }
            }
            // TODO sort by uniform/texture hash
            return r1->GetRenderState().m_state < r2->GetRenderState().m_state;
         }
      } sortFunc;

      // For the time being, stable sort is needed since we don't want to change the order of blended draw calls
      stable_sort(m_commands.begin(), m_commands.end(), sortFunc);

      m_rt->Activate(m_ignoreStereo);
      for (RenderCommand* cmd : m_commands)
         cmd->Execute(log);

      #ifdef ENABLE_SDL
      if (GLAD_GL_VERSION_4_3)
         glPopDebugGroup();
      #endif
   }
   m_rt->m_lastRenderPass = nullptr;
}
