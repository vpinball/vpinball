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

void RenderPass::Reset(const string& name, RenderTarget* const rt, const bool ignoreStereo)
{
   m_rt = rt;
   m_ignoreStereo = ignoreStereo;
   m_name = name;
   m_depthReadback = false;
   m_sortKey = 0;
   m_commands.clear();
   m_dependencies.clear();
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
      sortedPasses.back()->m_depthReadback |= m_depthReadback;
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
      #ifdef ENABLE_SDL
      if (GLAD_GL_VERSION_4_3)
      {
         std::stringstream passName;
         passName << m_name << " [RT=" << m_rt->m_name << "]";
         glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, passName.str().c_str());
      }
      #endif

      /*
      Before 10.8, render command were not buffered and processed in the following order (* is optional static prepass):
	      - Playfield *
	      - Static render,  not decals * => Unsorted
	      - Static render decals * => Unsorted
	      - Dynamic render Opaque, not DMD => Unsorted (front to back, state changes,…)
	      - Dynamic render Opaque DMD => Unsorted (front to back, state changes,…)
	      - Balls
	      - Dynamic render Transparent, not DMD => Sorted back to front
	      - Dynamic render Transparent DMD => Sorted back to front
      Note that:
         - Kickers are rendered with a "pass always" depth test
         - Transparent parts do write to depth buffer (they can be used as masks)
         - Depth sorting is not done based on view vector but on depth bias and absolute z coordinate

      For 10.8, the render command sorting has been designed to ensure backward compatibility:
	      - Identify transparent parts in a backward compatible way (using IsTransparent, and not according to real 'transparency' state as evaluated from depth & blend state)
	      - Sort render commands with the following constraints:
		      . Draw kickers first (at least before balls)
		      . TODO Sort opaque DMD after other opaques
		      . TODO Sort transparent DMD after other transparents
		      . Sort opaque parts together based on efficiency (state, real view depth, whatever…)
		      . Use existing sorting of transparent parts (based on absolute z and depthbias)
		      . TODO Sort "deferred draw light render commands" after opaque and before transparents
		      . TODO Group draw call of each refraction probe together (after the first part, based on default sorting)
      */
      struct
      {
         inline bool operator()(const RenderCommand* r1, const RenderCommand* r2) const
         {
            // Move Clear/Copy command at the beginning of the pass
            if (!r1->IsDrawCommand())
               return true;
            if (!r2->IsDrawCommand())
               return false;

            // Move LiveUI command at the end of the pass
            if (r1->IsDrawLiveUICommand())
               return false;
            if (r2->IsDrawLiveUICommand())
               return true;

            // Move kickers before other draw calls.
            // Kickers disable depth test to be visible through playfield. This would make them to be rendered after opaques, but since they hack depth, they need to be rendered before balls
            // > The right fix would be to remove the kicker hack (use stencil masking, alpha punch or CSG on playfield), this would also solve rendering kicker in VR
            if (r1->GetShaderTechnique() == SHADER_TECHNIQUE_kickerBoolean || r1->GetShaderTechnique() == SHADER_TECHNIQUE_kickerBoolean_isMetal)
               return true;
            if (r2->GetShaderTechnique() == SHADER_TECHNIQUE_kickerBoolean || r2->GetShaderTechnique() == SHADER_TECHNIQUE_kickerBoolean_isMetal)
               return false;
            
            // Non opaque items: render them after opaque ones, sorted back to front since their rendering depends on the framebuffer
            bool transparent1 = r1->IsTransparent(); // !r1->GetRenderState().IsOpaque();
            bool transparent2 = r2->IsTransparent(); // !r2->GetRenderState().IsOpaque();
            if (transparent1)
            {
               if (transparent2)
               {
                  if (r1->GetDepth() == r2->GetDepth())
                     return false;
                  return r1->GetDepth() > r2->GetDepth(); // Back to front
               }
               return false;
            }
            if (transparent2)
               return true;
            
            // Opaque items: render them front to back (to limit overdraw, thanks to early depth test), limiting shader/mesh buffer/sampler/uniform/state changes
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
               return r1->GetDepth() < r2->GetDepth(); // Front to back
            if (r1->IsDrawMeshCommand() && r2->IsDrawMeshCommand())
            {
               unsigned int mbS1 = r1->GetMeshBuffer()->GetSortKey();
               unsigned int mbS2 = r2->GetMeshBuffer()->GetSortKey();
               if (mbS1 != mbS2)
               {
                  return mbS1 < mbS2;
               }
            }
            return r1->GetRenderState().m_state < r2->GetRenderState().m_state;
         }
      } sortFunc;

      // stable sort is needed since we don't want to change the order of blended draw calls between frames
      if (log)
      {
         U64 start = usec();
         stable_sort(m_commands.begin(), m_commands.end(), sortFunc);
         PLOGI << "Pass '" << m_name << "' [RT=" << m_rt->m_name << ", " << m_commands.size() << " commands, sort: " << std::fixed << std::setw(8) << std::setprecision(3) << (usec() - start)
               << "us]";
      }
      else
         stable_sort(m_commands.begin(), m_commands.end(), sortFunc);

      m_rt->Activate(m_ignoreStereo);

      for (RenderCommand* cmd : m_commands)
         cmd->Execute(log);

      if (m_depthReadback)
         m_rt->UpdateDepthSampler(true);

      #ifdef ENABLE_SDL
      if (GLAD_GL_VERSION_4_3)
         glPopDebugGroup();
      #endif
   }
   m_rt->m_lastRenderPass = nullptr;
}
