#include "stdafx.h"
#include "RenderPass.h"
#include "RenderCommand.h"
#include "RenderDevice.h"

RenderPass::RenderPass(const string& name, RenderTarget* const rt)
   : m_rt(rt)
   , m_name(name)
{
   m_areaOfInterest.x = m_areaOfInterest.y = m_areaOfInterest.z = m_areaOfInterest.w = FLT_MAX;
}

RenderPass::~RenderPass()
{
   for (auto item : m_commands)
      delete item;
}

void RenderPass::Reset(const string& name, RenderTarget* const rt)
{
   m_rt = rt;
   m_name = name;
   m_singleLayerRendering = -1;
   m_areaOfInterest.x = m_areaOfInterest.y = m_areaOfInterest.z = m_areaOfInterest.w = FLT_MAX;
   m_depthReadback = false;
   m_sortKey = 0;
   m_updated = false;
   m_commands.clear();
   m_dependencies.clear();
   m_referencedRT.clear();
}

void RenderPass::RecycleCommands(std::vector<RenderCommand*>& commandPool)
{
   if (commandPool.size() < 1024)
      commandPool.insert(commandPool.end(), m_commands.begin(), m_commands.end());
   else
      for (RenderCommand* cmd : m_commands)
         delete cmd;
   m_commands.clear();
}

void RenderPass::AddPrecursor(RenderPass* dependency)
{
   assert(this != dependency);
   if (std::find(m_dependencies.begin(), m_dependencies.end(), dependency) == m_dependencies.end())
   {
      m_dependencies.push_back(dependency);
      if (std::find(m_referencedRT.begin(), m_referencedRT.end(), dependency->m_rt) == m_referencedRT.end())
         m_referencedRT.push_back(dependency->m_rt);
   }
}

void RenderPass::UpdateDependency(RenderTarget* target, RenderPass* newDependency)
{
   if (m_updated)
      return;
   m_updated = true;
   for (std::vector<RenderPass*>::iterator it = m_dependencies.begin(); it != m_dependencies.end(); ++it)
   {
      if ((*it)->m_rt == target)
         *it = newDependency;
      else
         (*it)->UpdateDependency(target, newDependency);
   }
}

void RenderPass::SortPasses(vector<RenderPass*>& sortedPasses, vector<RenderPass*>& allPasses)
{
   // Perform a depth first sort down the precursor list, sorting/grouping/merging by render target
   if (m_sortKey == 2) // Already processed
      return;
   assert(m_sortKey != 1); // Circular dependency between render pass
   m_sortKey = 1;

   // Sort dependencies:
   // - start with dependencies wich uses the same RT as we directly reference (to avoid content conflict)
   // - others
   // - last are dependencies on the same RT as us (to allow merge)
   // TODO this is slow (but correct). This could be easily done better (but the list is small, so...)
   sort(m_dependencies.begin(), m_dependencies.end(), [this](RenderPass* a, RenderPass* b)
      {
         // a has the same RT as this: place it at the end (a is not < b)
         if (a->m_rt == m_rt)
            return false;
         // b has the same RT as this: place it at the end (b is < a)
         if (b->m_rt == m_rt)
            return true;
         // a reference a RT that is used by b => place a before b
         for (auto dep : b->m_dependencies)
            for (auto rt : a->m_referencedRT)
               if (dep->m_rt == rt)
                  return true;
         // b reference a RT that is used by a => place b before a
         for (auto dep : a->m_dependencies)
            for (auto rt : b->m_referencedRT)
               if (dep->m_rt == rt)
                  return false;
         // no criteria to sort
         return true;
      });
   
   // Depth first recursion
   for (RenderPass* dependency : m_dependencies)
         dependency->SortPasses(sortedPasses, allPasses);

   // Push pass to result, eventually merging it
   m_sortKey = 2;
   if (!sortedPasses.empty())
   {
      // Merge passes
      RenderPass* mergedPass = sortedPasses.back();
      if (mergedPass->m_rt == m_rt
       && mergedPass->m_areaOfInterest.x == m_areaOfInterest.x && mergedPass->m_areaOfInterest.y == m_areaOfInterest.y 
       && mergedPass->m_areaOfInterest.z == m_areaOfInterest.z && mergedPass->m_areaOfInterest.w == m_areaOfInterest.w)
      {
         for (RenderPass* pass : allPasses)
         {
            if (pass == this)
               RemoveFromVector(pass->m_dependencies, mergedPass);
            else
               std::replace(pass->m_dependencies.begin(), pass->m_dependencies.end(), this, mergedPass);
         }
         mergedPass->m_depthReadback |= m_depthReadback;
         mergedPass->m_commands.insert(mergedPass->m_commands.end(), m_commands.begin(), m_commands.end());
         for (auto dep : m_dependencies)
            mergedPass->AddPrecursor(dep);
         //mergedPass->m_dependencies.insert(mergedPass->m_dependencies.end(), m_dependencies.begin(), m_dependencies.end());
         m_commands.clear();
         return;
      }
   }

   // Add passes
   sortedPasses.push_back(this);
}

void RenderPass::SortCommands()
{
   /*
   Before 10.8, render command were not buffered and processed in the following order (* is optional static prepass):
      - Playfield *
      - Static render,  not decals * => Unsorted
      - Static render decals * => Unsorted
      - Dynamic render Opaque, not DMD => Unsorted (front to back, state changes,...)
      - Dynamic render Opaque DMD => Unsorted (front to back, state changes,...), only used by Flasher DMD
      - Balls
      - Dynamic render Transparent, not DMD => Sorted back to front
      - Dynamic render Transparent DMD => Sorted back to front, unused feature (none of the parts are simultaneously IsDMD and IsTransparent)
   Note that:
      - Kickers are rendered with a "pass always" depth test
      - Transparent parts do write to depth buffer (they can be used as masks)
      - Depth sorting is not done based on view vector but on depth bias and absolute z coordinate

   For 10.8, the render command sorting has been designed to ensure backward compatibility:
      - Identify transparent parts in a backward compatible way (using IsTransparent, and not according to real 'transparency' state as evaluated from depth & blend state)
      - Sort render commands with the following constraints:
         . Draw kickers first (at least before balls)
         . Draw playfield of old tables before other parts. Old table's PF command is opaque with a very high depth bias (this is enforced when loading the table, see pintable.cpp)
         . Sort opaque parts together based on efficiency (state, real view depth, whatever...)
         . Draw flasher DMD after opaques and before transparents (they are marked as transparent with a depthbias shifted by -10000 to ensure this, see flasher.cpp)
         . Use existing sorting of transparent parts (based on absolute z and depthbias)
         . TODO Sort "deferred draw light render commands" after opaque and before transparents
         . TODO Group draw call of each refraction probe together (after the first part, based on default sorting)
   */
   struct
   {
      inline bool operator()(const RenderCommand* r1, const RenderCommand* r2) const
      {
         // Move Clear/Copy/SubmitVR command at the beginning of the pass
         if (!r2->IsDrawCommand())
            return false; // Return false first to keep submit order if we have multiple clear/copy/submit to sort
         if (!r1->IsDrawCommand())
            return true;

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
            
         // At least one transparent item (identify by legacy transparency flag): render them after opaque ones
         const bool transparent1 = r1->IsTransparent();
         const bool transparent2 = r2->IsTransparent();
         if (transparent1)
         {
            if (transparent2)
            {
               // Both transparent: sorted back to front since their rendering depends on the framebuffer (keep submission order if same depth)
               if (r1->GetDepth() == r2->GetDepth())
                  return false;
               return r1->GetDepth() > r2->GetDepth();
            }
            return false;
         }
         if (transparent2)
            return true;

         // At this point, both commands are draw commands of opaque items

         // HACKY: if marked with a very high depthbias, render them first. This is needed to avoid breaking playfield rendering of old table 
         // since before 10.8, playfield was always rendered before all other parts, with alpha testing and depth writing.
         if (r1->GetDepth() != r2->GetDepth() && fabsf(r1->GetDepth() - r2->GetDepth()) > 50000.f)
            return r1->GetDepth() > r2->GetDepth(); // Back to front

         // Sort by shader to limit the number of shader changes
         if (r1->GetShaderTechnique() != r2->GetShaderTechnique())
         {
            // TODO sort by minimum depth of the technique
            /* if (m_min_depth[r1->technique] == m_min_depth[r2->technique])
               return r1->technique < r2->technique;
            else
               return m_min_depth[r1->technique] < m_min_depth[r2->technique];*/
            return r1->GetShaderTechnique() > r2->GetShaderTechnique();
         }

         // Sort front to back to limit overdraw, limiting the number of processed fragment thanks to early depth test
         if (r1->GetDepth() != r2->GetDepth())
            return r1->GetDepth() < r2->GetDepth(); // Front to back

         // Sort by mesh buffer id, to limit buffer switching
         if (r1->IsDrawMeshCommand() && r2->IsDrawMeshCommand())
         {
            const unsigned int mbS1 = r1->GetMeshBuffer()->GetSortKey();
            const unsigned int mbS2 = r2->GetMeshBuffer()->GetSortKey();
            if (mbS1 != mbS2)
            {
               return mbS1 < mbS2;
            }
         }

         // Sort by render state ot limit the amount of state changes
         return r1->GetRenderState().m_state < r2->GetRenderState().m_state;
      }
   } sortFunc;

   // stable sort is needed since we don't want to change the order of blended draw calls between frames
   stable_sort(m_commands.begin(), m_commands.end(), sortFunc);
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

bool RenderPass::Execute(const bool log)
{
   m_rt->m_lastRenderPass = nullptr;
   if (m_commands.empty())
      return false;

   if (m_areaOfInterest.x != FLT_MAX)
   {
      const int left   = clamp((int)((0.5f + m_areaOfInterest.x * 0.5f) * (float)m_rt->GetWidth() ), 0, m_rt->GetWidth());
      const int bottom = clamp((int)((0.5f + m_areaOfInterest.y * 0.5f) * (float)m_rt->GetHeight()), 0, m_rt->GetHeight());
      const int right  = clamp((int)((0.5f + m_areaOfInterest.z * 0.5f) * (float)m_rt->GetWidth() ), 0, m_rt->GetWidth());
      const int top    = clamp((int)((0.5f + m_areaOfInterest.w * 0.5f) * (float)m_rt->GetHeight()), 0, m_rt->GetHeight());
      assert((left <= right) && (bottom <= top));
      if (left == right || bottom == top)
         return false;
      #ifdef ENABLE_SDL
      glEnable(GL_SCISSOR_TEST);
      glScissor((GLint)left, (GLint)bottom, (GLsizei)(right - left), (GLsizei)(top - bottom));
      #else
      const RECT r = { (LONG)left, (LONG)(m_rt->GetHeight() - top), (LONG)right, (LONG)(m_rt->GetHeight() - bottom) };
      m_rt->GetRenderDevice()->GetCoreDevice()->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
      m_rt->GetRenderDevice()->GetCoreDevice()->SetScissorRect(&r);
      #endif
   }

#ifdef ENABLE_SDL
#ifndef __OPENGLES__
   if (GLAD_GL_VERSION_4_3)
   {
      std::stringstream passName;
      passName << m_name << " [RT=" << m_rt->m_name << ']';
      glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, passName.str().c_str());
   }
#endif
#endif
   if (log)
   {
      std::stringstream ss;
      ss << "Pass '" << m_name << "' [RT='" << m_rt->m_name << "', ";
      if (m_singleLayerRendering >= 0)
         ss << "Layer=" << m_singleLayerRendering << ", ";
      ss << m_commands.size() << " commands, Dependencies:";
      bool first = true;
      for (RenderPass* dep : m_dependencies)
      {
         if (!first)
            ss << ", ";
         first = false;
         ss << '\'' << dep->m_name << '\'';
      }
      ss << ']';
      if (m_areaOfInterest.x != FLT_MAX)
         ss << ", scissor=(" << m_areaOfInterest.x << ", " << m_areaOfInterest.y << ", " << m_areaOfInterest.z << ", " << m_areaOfInterest.w << ")";
      PLOGI << ss.str();
   }

   if (m_rt->m_nLayers == 1 || (m_singleLayerRendering < 0 && m_rt->GetRenderDevice()->SupportLayeredRendering()))
   {
      m_rt->Activate();
      for (RenderCommand* cmd : m_commands)
      {
         #ifdef ENABLE_SDL // Layered rendering is not yet implemented for DirectX
         Shader::ShaderState* state = cmd->GetShaderState();
         if (state)
            state->SetInt(SHADER_layer, 0);
         #endif
         cmd->Execute(m_rt->m_nLayers, log);
      }
   }
   else if (m_singleLayerRendering >= 0)
   {
      assert(m_singleLayerRendering < m_rt->m_nLayers);
      m_rt->Activate(m_singleLayerRendering);
      for (RenderCommand* cmd : m_commands)
      {
         #ifdef ENABLE_SDL // Layered rendering is not yet implemented for DirectX
         Shader::ShaderState* state = cmd->GetShaderState();
         if (state)
            state->SetInt(SHADER_layer, m_singleLayerRendering);
         #endif
         cmd->Execute(1, log);
      }
   }
   else
   {
      for (int layer = 0; layer < m_rt->m_nLayers; layer++)
      {
         m_rt->Activate(layer);
         for (RenderCommand* cmd : m_commands)
         {
            #ifdef ENABLE_SDL // Layered rendering is not yet implemented for DirectX
            Shader::ShaderState* state = cmd->GetShaderState();
            if (state)
               state->SetInt(SHADER_layer, layer);
            #endif
            cmd->Execute(1, log);
         }
      }
   }

   if (m_areaOfInterest.x != FLT_MAX)
   {
      #ifdef ENABLE_SDL
      glDisable(GL_SCISSOR_TEST);
      #else
      m_rt->GetRenderDevice()->GetCoreDevice()->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
      #endif
   }

   if (m_depthReadback)
      m_rt->UpdateDepthSampler(true);

   #ifdef ENABLE_SDL
#ifndef __OPENGLES__
   if (GLAD_GL_VERSION_4_3)
      glPopDebugGroup();
#endif
   #endif

   return true;
}
