// license:GPLv3+

#include "core/stdafx.h"
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
   m_mergeable = true;
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
   if (FindIndexOf(m_dependencies, dependency) == -1)
      m_dependencies.push_back(dependency);
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
         const bool isKicker1 = r1->GetShaderTechnique() == SHADER_TECHNIQUE_kickerBoolean || r1->GetShaderTechnique() == SHADER_TECHNIQUE_kickerBoolean_isMetal;
         const bool isKicker2 = r2->GetShaderTechnique() == SHADER_TECHNIQUE_kickerBoolean || r2->GetShaderTechnique() == SHADER_TECHNIQUE_kickerBoolean_isMetal;
         if (isKicker1)
            return isKicker2 ? r1->GetDepth() > r2->GetDepth() : true;
         else if (isKicker2)
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
      // TODO optimize rendering by removing dependencies on this render target (but not on others)
   }
   m_commands.push_back(command);
}

bool RenderPass::Execute(const bool log)
{
   m_rt->m_lastRenderPass = nullptr;
   if (m_commands.empty())
      return false;

   int left,bottom,right,top;
   if (m_areaOfInterest.x != FLT_MAX)
   {
      left   = clamp((int)((0.5f + m_areaOfInterest.x * 0.5f) * (float)m_rt->GetWidth() ), 0, m_rt->GetWidth());
      bottom = clamp((int)((0.5f + m_areaOfInterest.y * 0.5f) * (float)m_rt->GetHeight()), 0, m_rt->GetHeight());
      right  = clamp((int)((0.5f + m_areaOfInterest.z * 0.5f) * (float)m_rt->GetWidth() ), 0, m_rt->GetWidth());
      top    = clamp((int)((0.5f + m_areaOfInterest.w * 0.5f) * (float)m_rt->GetHeight()), 0, m_rt->GetHeight());
      assert((left <= right) && (bottom <= top));
      if (left == right || bottom == top)
         return false;
      #if defined(ENABLE_BGFX)
      #elif defined(ENABLE_OPENGL)
      glEnable(GL_SCISSOR_TEST);
      glScissor((GLint)left, (GLint)bottom, (GLsizei)(right - left), (GLsizei)(top - bottom));
      #elif defined(ENABLE_DX9)
      const RECT r = { (LONG)left, (LONG)(m_rt->GetHeight() - top), (LONG)right, (LONG)(m_rt->GetHeight() - bottom) };
      m_rt->GetRenderDevice()->GetCoreDevice()->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
      m_rt->GetRenderDevice()->GetCoreDevice()->SetScissorRect(&r);
      #endif
   }

   #if defined(ENABLE_OPENGL) && !defined(__OPENGLES__) && defined(_DEBUG)
   if (GLAD_GL_VERSION_4_3)
   {
      std::stringstream passName;
      passName << m_name << " [RT=" << m_rt->m_name << ']';
      glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, passName.str().c_str());
   }
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
         ss << ", scissor=(" << m_areaOfInterest.x << ", " << m_areaOfInterest.y << ", " << m_areaOfInterest.z << ", " << m_areaOfInterest.w << ')';
      PLOGI << ss.str();
   }

   if (m_rt->m_nLayers == 1 || (m_singleLayerRendering < 0 && m_rt->GetRenderDevice()->SupportLayeredRendering()))
   {
      m_rt->Activate();
      #if defined(ENABLE_BGFX)
      if (m_areaOfInterest.x != FLT_MAX)
         bgfx::setViewScissor(m_rt->GetRenderDevice()->m_activeViewId, left, m_rt->GetHeight() - top, right - left, top - bottom);
      #if defined(_DEBUG)
      bgfx::setViewName(m_rt->GetRenderDevice()->m_activeViewId, m_name.append(" [RT=").append(m_rt->m_name).append(1,']').c_str());
      #endif
      #endif
      for (RenderCommand* cmd : m_commands)
         cmd->Execute(m_rt->m_nLayers, log);
   }
   else if (m_singleLayerRendering >= 0)
   {
      assert(m_singleLayerRendering < m_rt->m_nLayers);
      m_rt->Activate(m_singleLayerRendering);
      #if defined(ENABLE_BGFX)
      if (m_areaOfInterest.x != FLT_MAX)
         bgfx::setViewScissor(m_rt->GetRenderDevice()->m_activeViewId, left, m_rt->GetHeight() - top, right - left, top - bottom);
      #if defined(_DEBUG)
      bgfx::setViewName(m_rt->GetRenderDevice()->m_activeViewId, m_name.append(" [RT=").append(m_rt->m_name).append(" / Layer=").append(std::to_string(m_singleLayerRendering)).append(1,']').c_str());
      #endif
      #endif
      for (RenderCommand* cmd : m_commands)
         cmd->Execute(1, log);
   }
   else
   {
      for (int layer = 0; layer < m_rt->m_nLayers; layer++)
      {
         m_rt->Activate(layer);
         #if defined(ENABLE_BGFX)
         if (m_areaOfInterest.x != FLT_MAX)
            bgfx::setViewScissor(m_rt->GetRenderDevice()->m_activeViewId, left, m_rt->GetHeight() - top, right - left, top - bottom);
         #if defined(_DEBUG)
         bgfx::setViewName(m_rt->GetRenderDevice()->m_activeViewId, m_name.append(" [RT=").append(m_rt->m_name).append(" / Layer=").append(std::to_string(layer)).append(1,']').c_str());
         #endif
         #endif
         for (RenderCommand* cmd : m_commands)
            cmd->Execute(1, log);
      }
   }

   if (m_areaOfInterest.x != FLT_MAX)
   {
      #if defined(ENABLE_BGFX)
      #elif defined(ENABLE_OPENGL)
      glDisable(GL_SCISSOR_TEST);
      #elif defined(ENABLE_DX9)
      m_rt->GetRenderDevice()->GetCoreDevice()->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
      #endif
   }

   if (m_depthReadback)
      m_rt->UpdateDepthSampler(true);

   #if defined(ENABLE_OPENGL) && !defined(__OPENGLES__) && defined(_DEBUG)
   if (GLAD_GL_VERSION_4_3)
      glPopDebugGroup();
   #endif

   return true;
}
