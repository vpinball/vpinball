// license:GPLv3+

#include "core/stdafx.h"
#include "RenderProbe.h"

#include "math/bluenoise.h"
#include "math/matrix.h"
#include "parts/Collection.h"
#include "renderer/RenderCommand.h"
#include "renderer/Renderer.h"
#include "renderer/Shader.h"

const string RenderProbe::PLAYFIELD_REFLECTION_RENDERPROBE_NAME = "Playfield Reflections"s;

RenderProbe::~RenderProbe()
{
   assert(m_viewState.empty()); // RenderRelease must be called before destructor
}

void RenderProbe::Save(IObjectWriter& writer, const bool saveForUndo)
{
   // Save as a data blob inside the main gamedata. This allows backward compatibility since the block will be blindly discarded on older versions, still hashing it.
   writer.BeginObject(FID(RPRB), true, true);
   writer.WriteInt(FID(TYPE), (int)m_type);
   writer.WriteString(FID(NAME), m_name);
   writer.WriteInt(FID(RBAS), m_roughness);
   writer.WriteRaw(FID(RPLA), (void*)&m_reflection_plane, sizeof(vec4));
   writer.WriteInt(FID(RMOD), (int)m_reflection_mode);
   writer.WriteBool(FID(RLMP), m_disableLightReflection);
   writer.EndObject();
}

void RenderProbe::Load(IObjectReader& reader)
{
   reader.AsObject(
      [this](int tag, IObjectReader& reader)
      {
         switch (tag)
         {
         case FID(TYPE): m_type = static_cast<ProbeType>(reader.AsInt()); break;
         case FID(NAME): m_name = reader.AsString(); break;
         case FID(RBAS): m_roughness = reader.AsInt(); break;
         case FID(RPLA): reader.AsRaw(&m_reflection_plane, sizeof(vec4)); break;
         case FID(RMOD): m_reflection_mode = static_cast<ReflectionMode>(reader.AsInt()); break;
         case FID(RLMP): m_disableLightReflection = reader.AsBool(); break;
         }
         return true;
      },
      true);
}

void RenderProbe::SetName(const string& name) { m_name = name; }

RenderProbe::ViewState& RenderProbe::State(const RenderView* view) { return m_viewState[view]; }

RenderProbe::ViewState& RenderProbe::State() { return m_viewState[m_renderer->GetActiveView()]; }

void RenderProbe::RenderSetup(Renderer* renderer)
{
   assert(m_renderer == nullptr);
   m_renderer = renderer;
   m_rdState = new RenderDeviceState(m_renderer->m_renderDevice);
   MarkDirty();
}

void RenderProbe::RenderRelease()
{
   assert(m_renderer != nullptr);
   m_renderer = nullptr;
   delete m_rdState;
   m_rdState = nullptr;
   for (auto& [view, vs] : m_viewState)
   {
      delete vs.prerenderRT;
      delete vs.dynamicRT;
      delete vs.blurRT;
   }
   m_viewState.clear(); // Passes belong to the render frame of the released renderer
}


void RenderProbe::MarkDirty(RenderView* view)
{
   ViewState& vs = State(view);
   vs.dirty = true;
   vs.reflection_clip_bounds.x = vs.reflection_clip_bounds.y = vs.reflection_clip_bounds.z = vs.reflection_clip_bounds.w = FLT_MAX;
}

void RenderProbe::MarkDirty()
{
   // Views without an entry are already dirty by default: only re-mark the ones that have rendered
   for (auto& [view, vs] : m_viewState)
   {
      vs.dirty = true;
      vs.reflection_clip_bounds.x = vs.reflection_clip_bounds.y = vs.reflection_clip_bounds.z = vs.reflection_clip_bounds.w = FLT_MAX;
   }
}

bool RenderProbe::IsStaticAccumulationPending() const
{
   // A probe accumulates its static parts over successive frames only when it is a dynamic reflection
   // rendered while the main renderer performs split static/dynamic rendering
   const auto it = m_viewState.find(m_renderer != nullptr ? m_renderer->GetActiveView() : nullptr);
   return m_renderer != nullptr && m_type == PLANE_REFLECTION && min(m_reflection_mode, m_renderer->GetMaxReflectionMode()) == REFL_DYNAMIC && m_renderer->IsUsingStaticPrepass()
      && (it == m_viewState.end() || it->second.staticAccumCount < STATIC_PRERENDER_ITERATIONS);
}

void RenderProbe::MarkDirtyStatics(RenderView* view)
{
   ViewState& vs = State(view);
   // Discard the accumulated static render. Defer deletion to the render thread end-of-frame as already submitted frames may still hold copy commands sourcing this render target.
   // FIXME why don't we simply reuse it ?
   if (vs.prerenderRT)
      m_renderer->m_renderDevice->AddEndOfFrameCmd([rt = vs.prerenderRT]() { delete rt; });
   vs.prerenderRT = nullptr;
   vs.staticAccumCount = 0;
}

void RenderProbe::ReleaseRenderTargets(RenderView* view)
{
   if (m_renderer == nullptr)
      return;
   const auto it = m_viewState.find(view);
   if (it == m_viewState.end())
      return;
   ViewState& vs = it->second;
   // Deferred to end of frame, as already submitted frames may still use them
   vector<RenderTarget*> released { vs.prerenderRT, vs.dynamicRT, vs.blurRT };
   std::erase(released, nullptr);
   if (!released.empty())
      m_renderer->m_renderDevice->AddEndOfFrameCmd(
         [released]()
         {
            for (RenderTarget* rt : released)
               delete rt;
         });
   // Drop the whole entry: the released renders must be rebuilt anyway on next use
   m_viewState.erase(it);
}

RenderTarget* RenderProbe::Render(const unsigned int renderMask)
{
   assert(m_renderer != nullptr);
   const bool isReflectionPass = renderMask & Renderer::REFLECTION_PASS;

   // Probes are rendered and used in screen space therefore, they can't be recursively used (e.g. reflections
   // of reflections). Beside this, some properties are not cached (clip plane,...) and would break if
   // we would allow this. We simply return nullptr to disable this behavior.
   if (m_rendering || isReflectionPass)
      return nullptr;

   switch (m_type)
   {
   case PLANE_REFLECTION: return RenderReflectionProbe(renderMask);

   case SCREEN_SPACE_TRANSPARENCY: RenderScreenSpaceTransparency(); return State().dynamicRT;

   default: assert(false); return nullptr;
   }
}

void RenderProbe::ExtendAreaOfInterest(const float xMin, const float xMax, const float yMin, const float yMax)
{
   ViewState& vs = State();
   if (vs.reflection_clip_bounds.x == FLT_MAX)
   {
      vs.reflection_clip_bounds.x = xMin;
      vs.reflection_clip_bounds.y = yMin;
      vs.reflection_clip_bounds.z = xMax;
      vs.reflection_clip_bounds.w = yMax;
   }
   else
   {
      vs.reflection_clip_bounds.x = min(vs.reflection_clip_bounds.x, xMin);
      vs.reflection_clip_bounds.y = min(vs.reflection_clip_bounds.y, yMin);
      vs.reflection_clip_bounds.z = max(vs.reflection_clip_bounds.z, xMax);
      vs.reflection_clip_bounds.w = max(vs.reflection_clip_bounds.w, yMax);
   }
}

void RenderProbe::ApplyAreaOfInterest()
{
   // Only apply to the passes recorded by this probe's renders. Dependencies must not be followed as they
   // may reach passes of another scene render using the same render target (e.g. the VR scene render and
   // the additional desktop scene render when VR desktop display is not an eye mirror) which must not be
   // clipped with this render's view dependent bounds.
   ViewState& vs = State();
   for (RenderPass* pass : vs.probePasses)
   {
      if (m_type == SCREEN_SPACE_TRANSPARENCY)
      {
         // Since we don't know where the sampling will occur, we only apply AOI to the blur passes,
         // with a small arbitrary margin to account for IOR shift
         if (pass == vs.copyPass)
            continue;
         constexpr float margin = 0.05f;
         pass->m_areaOfInterest.x = vs.reflection_clip_bounds.x - margin;
         pass->m_areaOfInterest.y = vs.reflection_clip_bounds.y - margin;
         pass->m_areaOfInterest.z = vs.reflection_clip_bounds.z + margin;
         pass->m_areaOfInterest.w = vs.reflection_clip_bounds.w + margin;
      }
      else
      {
         // TODO we should enlarge the AOI to account for the blur kernel size
         // We do not apply to pass with just a clear command to avoid sampling artefacts
         if (!(pass->m_commands.size() == 1 && pass->m_commands[0]->IsFullClear(false)))
            pass->m_areaOfInterest = vs.reflection_clip_bounds;
      }
   }
   // Consume the pass list so that a following scene render (e.g. VR desktop scene) only applies its own bounds to its own passes
   vs.probePasses.clear();
}


//////////////////////////////////////////////////////////////////////////////////////////////////
// Roughness implementation, using downscaling and bluring

int RenderProbe::GetRoughnessDownscale(const int roughness) const
{
   if (m_type == SCREEN_SPACE_TRANSPARENCY)
      return 1; // No downscaling since this is not supported by hardware for depth buffer
   return roughness < 7 ? 1 : 2;
}

void RenderProbe::ApplyRoughness(RenderTarget* probe, const int roughness)
{
   assert(0 <= roughness && roughness <= 12);
   ViewState& vs = State();
   if (roughness > 0)
   {
      if (vs.blurRT == nullptr)
         vs.blurRT = probe->Duplicate(probe->m_name + ".Blur", true);
      // The kernel sizes were chosen by reverse engineering the blur shader. So there's a part of guess here.
      // The blur shader seems to mix binomial & gaussian distributions, with a kernel size which does not directly match the pascal triangle size.
      // Ideally this should be a gaussian distribution's sigma, scaled by the render height against a reference height.
      if (m_type == SCREEN_SPACE_TRANSPARENCY)
      {
         // FIXME adjust the kernels to have as many as there are roughness levels
         static constexpr float kernel[] = { 0.f, 7.f, 9.f, 11.f, 13.f, 15.f, 19.f, 23.f, 27.f, 39.f, 39.f, 39.f, 39.f };
         m_renderer->m_renderDevice->DrawGaussianBlur(probe, vs.blurRT, probe, kernel[roughness]);
      }
      else
      {
         static constexpr float kernel[] = { 0.f, 7.f, 9.f, 13.f, 15.f, 19.f, 23.f, 13.f, 15.f, 19.f, 23.f, 27.f, 39.f };
         m_renderer->m_renderDevice->DrawGaussianBlur(probe, vs.blurRT, probe, kernel[roughness]);
      }
   }
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Screen copy

void RenderProbe::RenderScreenSpaceTransparency()
{
   ViewState& vs = State();
   if (!vs.dirty)
      return;

   vs.dirty = false;
   RenderPass* const renderedPass = m_renderer->m_renderDevice->GetCurrentPass();
   const size_t firstPass = m_renderer->m_renderDevice->GetRenderFramePasses().size();
   if (vs.dynamicRT == nullptr)
   {
      const int downscale = GetRoughnessDownscale(m_roughness);
      const int w = renderedPass->m_rt->GetWidth() / downscale, h = renderedPass->m_rt->GetHeight() / downscale;
      vs.dynamicRT = new RenderTarget(
         m_renderer->m_renderDevice, renderedPass->m_rt->m_type, m_name, w, h, renderedPass->m_rt->GetColorFormat(), true, 1, "Failed to create refraction render target", nullptr);
   }
   m_renderer->m_renderDevice->m_basicShader->SetTextureNull(ShaderUniform::tex_refraction);
   m_renderer->m_renderDevice->SetRenderTarget(m_name, vs.dynamicRT, false);
   m_renderer->m_renderDevice->BlitRenderTarget(renderedPass->m_rt, vs.dynamicRT, true, true);
   vs.copyPass = m_renderer->m_renderDevice->GetCurrentPass();
   ApplyRoughness(vs.dynamicRT, m_roughness);
   const vector<RenderPass*>& framePasses = m_renderer->m_renderDevice->GetRenderFramePasses();
   vs.probePasses.insert(vs.probePasses.end(), framePasses.begin() + firstPass, framePasses.end());
   m_renderer->m_renderDevice->SetRenderTarget(renderedPass->m_name, renderedPass->m_rt, false);
   m_renderer->m_renderDevice->GetCurrentPass()->AddPrecursor(renderedPass);
   renderedPass->MarkInterrupted();
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Reflection plane

void RenderProbe::GetReflectionPlane(vec4& plane) const
{
   plane.x = m_reflection_plane.x;
   plane.y = m_reflection_plane.y;
   plane.z = m_reflection_plane.z;
   plane.w = m_reflection_plane.w;
}

void RenderProbe::SetReflectionPlane(const vec4& plane) { m_reflection_plane = plane; }

vec3 RenderProbe::GetReflectionPlaneNormal() const { return vec3(m_reflection_plane.x, m_reflection_plane.y, m_reflection_plane.z); }

float RenderProbe::GetReflectionPlaneDistance() const { return m_reflection_plane.w; }

void RenderProbe::SetReflectionPlaneNormal(const vec3& normal)
{
   m_reflection_plane.x = normal.x;
   m_reflection_plane.y = normal.y;
   m_reflection_plane.z = normal.z;
}

void RenderProbe::SetReflectionPlaneDistance(float distance) { m_reflection_plane.w = distance; }

void RenderProbe::SetReflectionMode(ReflectionMode mode)
{
   assert(m_reflection_mode == mode || m_viewState.empty()); // Reflection mode may not be changed between RenderSetup/RenderRelease
   m_reflection_mode = mode;
}

RenderTarget* RenderProbe::GetRenderTarget(bool isStaticRT)
{
   ViewState& vs = State();
   RenderTarget*& rt = isStaticRT ? vs.prerenderRT : vs.dynamicRT;
   if (rt == nullptr)
   {
      int w, h;
      m_renderer->GetRenderSizeAA(w, h);
      const int downscale = GetRoughnessDownscale(m_roughness);
      rt = new RenderTarget(m_renderer->m_renderDevice, m_renderer->IsStereo() ? SurfaceType::RT_STEREO : SurfaceType::RT_DEFAULT, m_name + (isStaticRT ? ".Stat" : ".Dyn"), w / downscale,
         h / downscale, m_renderer->GetRenderFormat(), true, 1, "Failed to create plane reflection render target", nullptr);
   }
   return rt;
}

// Reflections can be baked in the static prerendering, fully dynamic or a mix of both
// - REFL_NONE              STATIC_ONLY => nothing       DEFAULT => nothing               DYNAMIC_ONLY => nothing
// - REFL_BALLS             STATIC_ONLY => nothing       DEFAULT => balls                 DYNAMIC_ONLY => balls
// - REFL_STATIC            STATIC_ONLY => static parts  DEFAULT => static parts          DYNAMIC_ONLY => nothing
// - REFL_STATIC_N_BALLS    STATIC_ONLY => static parts  DEFAULT => static parts & balls  DYNAMIC_ONLY => balls
// - REFL_STATIC_N_DYNAMIC  STATIC_ONLY => static parts  DEFAULT => everything            DYNAMIC_ONLY => non static parts
// - REFL_DYNAMIC           STATIC_ONLY => nothing       DEFAULT => everything            DYNAMIC_ONLY => everything (eventually optimized with static part prerendering)
RenderTarget* RenderProbe::RenderReflectionProbe(const unsigned int renderMask)
{
   assert(m_renderer != nullptr);
   ViewState& vs = State();
   const bool isStaticOnly = renderMask & Renderer::STATIC_ONLY;
   const bool isDynamicOnly = renderMask & Renderer::DYNAMIC_ONLY;
   assert((renderMask & Renderer::REFLECTION_PASS) == 0);
   const ReflectionMode mode = min(m_reflection_mode, m_renderer->GetMaxReflectionMode());

   if (mode == REFL_NONE || (isStaticOnly && (mode == REFL_BALLS || mode == REFL_DYNAMIC)) || (isDynamicOnly && (mode == REFL_STATIC)))
      return nullptr;

   if (!vs.dirty)
   {
      // The renderer must not switch from split (static then dynamic) rendering to global mid frame
      assert(vs.isSplitRendering == (isStaticOnly || isDynamicOnly));
      return vs.dynamicRT;
   }

   vs.dirty = false;
   vs.isSplitRendering = isStaticOnly || isDynamicOnly;

   m_rendering = true;
   RenderPass* const previousRT = m_renderer->m_renderDevice->GetCurrentPass();
   const size_t firstPass = m_renderer->m_renderDevice->GetRenderFramePasses().size();

   m_renderer->m_renderDevice->ResetRenderState();
   m_renderer->m_renderDevice->CopyRenderAndShaderStates(true, *m_rdState);

   const unsigned int prevRenderMask = m_renderer->m_render_mask;
   m_renderer->m_render_mask = prevRenderMask | Renderer::REFLECTION_PASS;
   if (m_disableLightReflection)
      m_renderer->m_render_mask |= Renderer::DISABLE_LIGHTMAPS;

   // Set the clip plane to only render objects above the reflection plane (do not reflect what is under or the plane itself)
   Vertex3Ds n(m_reflection_plane.x, m_reflection_plane.y, m_reflection_plane.z);
   n.Normalize();
   const vec4 clip_plane(n.x, n.y, n.z, m_reflection_plane.w);
   m_renderer->m_renderDevice->SetClipPlane(clip_plane);
   m_renderer->m_renderDevice->SetRenderState(RenderState::CLIPPLANEENABLE, RenderState::RS_TRUE);
   // Reverse cull mode since we multiply by a reversing matrix (mirror also has a reversing matrix)
   m_renderer->m_renderDevice->SetRenderState(RenderState::CULLMODE, g_pplayer->m_tblMirrorEnabled ? RenderState::CULL_CCW : RenderState::CULL_CW);
   m_renderer->m_renderDevice->SetDefaultRenderState();
   m_renderer->m_renderDevice->m_basicShader->SetTextureNull(ShaderUniform::tex_reflection);

   // Flip camera
   m_renderer->SetReflection(Matrix3D::MatrixPlaneReflection(n, m_reflection_plane.w));

   // The final render is always done on vs.dynamicRT
   RenderTarget* dynamicRT = GetRenderTarget(false);

   if (isStaticOnly)
   { // Static prerendering: simply render static parts in a render target to be accumulated by the caller (not us)
      m_renderer->m_renderDevice->SetRenderTarget(m_name, dynamicRT);
      m_renderer->m_renderDevice->Clear(clearType::TARGET | clearType::ZBUFFER, 0x00000000);
      m_renderer->UpdateBasicShaderMatrix();
      m_renderer->DrawStatics();
      ApplyRoughness(dynamicRT, m_roughness);
   }
   else
   { // Dynamic pass or split rendering disabled
      if (isDynamicOnly && mode == REFL_DYNAMIC)
      {
         // Main renderer is using split rendering and everything is dynamic: perform static prerendering for better perf and quality
         RenderTarget* accumulateRT = GetRenderTarget(true);

         // 1. Accumulate
         if (vs.staticAccumCount < STATIC_PRERENDER_ITERATIONS)
         {
            // Setup the view matrices with a jitter depending on the accumulation step.
            // The first accumulated sample always uses a centered offset so that the acquired depth buffer is the unjittered one.
            const bool isFirstSample = vs.staticAccumCount == 0;
            float u1 = xyLDBNbnot[vs.staticAccumCount * 2];
            float u2 = xyLDBNbnot[vs.staticAccumCount * 2 + 1];
            if (!isFirstSample)
            {
               // the following line implements filter importance sampling for a small gauss (i.e. less jaggies as it also samples neighboring pixels) -> but also potentially more artifacts in compositing!
               gaussianDistribution(u1, u2, 0.5f, 0.0f); //!! first 0.5 could be increased for more blur, but is pretty much what is recommended
               // sanity check to be sure to limit filter area to 3x3 in practice, as the gauss transformation is unbound (which is correct, but for our use-case/limited amount of samples very bad)
               assert(u1 > -1.5f && u1 < 1.5f);
               assert(u2 > -1.5f && u2 < 1.5f);
               // First accumulated sample MUST set a sample offset of 0,0 so that the depth buffer features 'correctly' centered pixel sample
               assert(vs.staticAccumCount != 0 || (u1 == 0.f && u2 == 0.f));
               // Apply the jitter to the reflected MVP (the offset is applied to the projection, preserving the reflected view)
               m_renderer->ApplyViewJitter(u1, u2);
            }

            // Render the jittered sample to the dynamic render target (used as a temporary, its content is discarded below)
            m_renderer->m_render_mask = (m_renderer->m_render_mask & ~Renderer::DYNAMIC_ONLY) | Renderer::STATIC_ONLY;
            m_renderer->m_renderDevice->SetRenderTarget("PreRender Reflection"s, dynamicRT, false);
            m_renderer->m_renderDevice->Clear(clearType::TARGET | clearType::ZBUFFER, 0x00000000);
            m_renderer->UpdateBasicShaderMatrix();
            m_renderer->DrawStatics();
            if (!isFirstSample)
               m_renderer->ApplyViewJitter(-u1, -u2); // Restore the unjittered projection
            m_renderer->m_render_mask = (m_renderer->m_render_mask & ~Renderer::STATIC_ONLY) | Renderer::DYNAMIC_ONLY;

            // Accumulate the new sample into the static render target, weighted by 1/STATIC_PRERENDER_ITERATIONS
            vs.staticAccumCount++;
            m_renderer->m_renderDevice->SetRenderTarget("PreRender Accumulate Reflection"s, accumulateRT);
            m_renderer->m_renderDevice->AddRenderTargetDependency(vs.dynamicRT);
            if (isFirstSample) // Copy unjittered depth buffer rendered on first sample
               m_renderer->m_renderDevice->BlitRenderTarget(vs.dynamicRT, accumulateRT, false, true);
            m_renderer->m_renderDevice->ResetRenderState();
            m_renderer->m_renderDevice->SetRenderState(RenderState::ALPHABLENDENABLE, isFirstSample ? RenderState::RS_FALSE : RenderState::RS_TRUE);
            m_renderer->m_renderDevice->SetRenderState(RenderState::SRCBLEND, RenderState::ONE);
            m_renderer->m_renderDevice->SetRenderState(RenderState::DESTBLEND, RenderState::ONE);
            m_renderer->m_renderDevice->SetRenderState(RenderState::BLENDOP, RenderState::BLENDOP_ADD);
            m_renderer->m_renderDevice->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);
            m_renderer->m_renderDevice->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
            m_renderer->m_renderDevice->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
            m_renderer->m_renderDevice->m_FBShader->SetTechnique(ShaderTechnique::fb_mirror);
            m_renderer->m_renderDevice->m_FBShader->SetVector(ShaderUniform::w_h_height, (float)(1.0 / (double)accumulateRT->GetWidth()), (float)(1.0 / (double)accumulateRT->GetHeight()),
               (float)(double)STATIC_PRERENDER_ITERATIONS, 0.0f);
            m_renderer->m_renderDevice->m_FBShader->SetTexture(ShaderUniform::tex_fb_unfiltered, vs.dynamicRT->GetColorSampler());
            m_renderer->m_renderDevice->DrawFullscreenTexturedQuad(m_renderer->m_renderDevice->m_FBShader);
            m_renderer->m_renderDevice->m_FBShader->SetTextureNull(ShaderUniform::tex_fb_unfiltered);
         }

         // 2. Render dynamic starting from scaled accumulate
         m_renderer->m_renderDevice->SetRenderTarget(m_name, dynamicRT);
         m_renderer->m_renderDevice->AddRenderTargetDependency(accumulateRT);
         if (vs.staticAccumCount >= STATIC_PRERENDER_ITERATIONS)
         {
            // Accumulation is complete: the static render target holds the final rendering
            m_renderer->m_renderDevice->BlitRenderTarget(accumulateRT, dynamicRT, true, true);
         }
         else
         {
            // The static render target holds the scaled accumulation: its depth is copied as-is while its color is
            // scaled back up by STATIC_PRERENDER_ITERATIONS/accumulated samples (weight reaches 1 when accumulation is complete)
            m_renderer->m_renderDevice->BlitRenderTarget(accumulateRT, dynamicRT, false, true);
            m_renderer->m_renderDevice->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_FALSE);
            m_renderer->m_renderDevice->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);
            m_renderer->m_renderDevice->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
            m_renderer->m_renderDevice->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
            m_renderer->m_renderDevice->m_FBShader->SetTechnique(ShaderTechnique::fb_mirror);
            m_renderer->m_renderDevice->m_FBShader->SetVector(ShaderUniform::w_h_height, (float)(1.0 / (double)vs.prerenderRT->GetWidth()),
               (float)(1.0 / (double)vs.prerenderRT->GetHeight()), (float)(std::max(1, vs.staticAccumCount) / (double)STATIC_PRERENDER_ITERATIONS), 0.0f);
            m_renderer->m_renderDevice->m_FBShader->SetTexture(ShaderUniform::tex_fb_unfiltered, vs.prerenderRT->GetColorSampler());
            m_renderer->m_renderDevice->DrawFullscreenTexturedQuad(m_renderer->m_renderDevice->m_FBShader);
            m_renderer->m_renderDevice->m_FBShader->SetTextureNull(ShaderUniform::tex_fb_unfiltered);
         }
      }
      else
      {
         m_renderer->m_renderDevice->SetRenderTarget(m_name, dynamicRT);
         m_renderer->m_renderDevice->Clear(clearType::TARGET | clearType::ZBUFFER, 0x00000000);
      }

      const bool renderStatic = !isDynamicOnly && (mode >= REFL_STATIC);
      const bool renderBalls = (mode == REFL_BALLS) || (mode == REFL_STATIC_N_BALLS);
      const bool renderDynamic = mode >= REFL_STATIC_N_DYNAMIC;
      if (renderStatic || renderDynamic)
         m_renderer->UpdateBasicShaderMatrix();
      if (renderStatic)
         m_renderer->DrawStatics();
      if (renderDynamic || renderBalls)
      {
         m_renderer->UpdateBallShaderMatrix();
         m_renderer->DrawDynamics(!renderDynamic);
      }

      ApplyRoughness(dynamicRT, m_roughness);
   }

   // Restore initial render states and camera
   m_renderer->SetReflection(Matrix3D::MatrixIdentity());
   m_renderer->m_render_mask = prevRenderMask;
   m_renderer->m_renderDevice->CopyRenderAndShaderStates(false, *m_rdState);
   m_renderer->m_renderDevice->SetDefaultRenderState();

   const vector<RenderPass*>& framePasses = m_renderer->m_renderDevice->GetRenderFramePasses();
   vs.probePasses.insert(vs.probePasses.end(), framePasses.begin() + firstPass, framePasses.end());
   m_renderer->m_renderDevice->SetRenderTarget(previousRT->m_name, previousRT->m_rt);
   previousRT->MarkInterrupted();
   m_rendering = false;

   return vs.dynamicRT;
}
