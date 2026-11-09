// license:GPLv3+

#include "core/stdafx.h"
#include "RenderView.h"

#include "core/player.h"
#include "math/bluenoise.h"
#include "math/math.h"
#include "math/matrix.h"
#include "parts/ball.h"
#include "parts/pintable.h"
#include "physics/cabinet/NudgeHandler.h"
#include "renderer/RenderCommand.h"
#include "renderer/RenderDevice.h"
#include "renderer/RenderPass.h"
#include "renderer/RenderProbe.h"
#include "renderer/Renderer.h"
#include "renderer/Shader.h"
#include "renderer/VRDevice.h"
#include "renderer/Window.h"
#include "renderer/trace.h"
#include "ui/live/LiveUI.h"
#include "utils/color.h"

#define MAX_BALL_SHADOW 8

RenderView::RenderView(Renderer* renderer, Kind kind)
   : m_renderer(renderer)
   , m_kind(kind)
   , m_mvp(renderer->m_renderDevice->m_nEyes)
   , m_initialMVP(renderer->m_renderDevice->m_nEyes)
{
}

RenderView::~RenderView()
{
   // Release this view's probe render state (no-op once probes have been released with the renderer)
   for (RenderProbe* probe : m_renderer->m_table->m_vrenderprobe)
      probe->ReleaseRenderTargets(this);
   ReleaseTargets();
}

void RenderView::SetupTargets(const int width, const int height, const SurfaceType rtType, const colorFormat format, const int nMSAASamples)
{
   assert(m_pOffscreenBackBufferTexture1 == nullptr);
   RenderDevice* const rd = m_renderer->m_renderDevice;
#ifdef ENABLE_BGFX
   // BGFX handles MSAA internally and provides the resolved texture directly, so just create one render target for the back buffer
   m_pOffscreenBackBufferTexture1 = new RenderTarget(rd, rtType, "BackBuffer1"s, width, height, format, true, nMSAASamples, "Fatal Error: unable to create offscreen back buffer");
#else
   // MSAA render target which is resolved to the non MSAA render target
   if (nMSAASamples > 1)
      m_pOffscreenMSAABackBufferTexture = new RenderTarget(rd, rtType, "MSAABackBuffer"s, width, height, format, true, nMSAASamples, "Fatal Error: unable to create MSAA render buffer!");
   // Either the main render target for non MSAA, or the buffer where the MSAA render is resolved
   m_pOffscreenBackBufferTexture1 = new RenderTarget(rd, rtType, "BackBuffer1"s, width, height, format, true, 1, "Fatal Error: unable to create offscreen back buffer");
#endif
   // Second render target to swap, allowing to read previous frame render for ball reflection and motion blur
   m_pOffscreenBackBufferTexture2 = m_pOffscreenBackBufferTexture1->Duplicate("BackBuffer2"s, false);

   // alloc bloom tex at 1/4 x 1/4 res (allows for simple HQ downscale of clipped input while saving memory)
   m_pBloomBufferTexture = new RenderTarget(rd, m_pOffscreenBackBufferTexture1->m_type, "BloomBuffer1"s, max(1, m_renderWidth / 4), max(1, m_renderHeight / 4),
      m_pOffscreenBackBufferTexture1->GetColorFormat(), false, 1, "Fatal Error: unable to create bloom buffer!");
   m_pBloomTmpBufferTexture = m_pBloomBufferTexture->Duplicate("BloomBuffer2"s);
}

void RenderView::ReleaseTargets()
{
   delete m_staticPrepassRT;
   m_staticPrepassRT = nullptr;
   delete m_pOffscreenBackBufferTexture1;
   m_pOffscreenBackBufferTexture1 = nullptr;
   delete m_pOffscreenBackBufferTexture2;
   m_pOffscreenBackBufferTexture2 = nullptr;
   delete m_pOffscreenMSAABackBufferTexture;
   m_pOffscreenMSAABackBufferTexture = nullptr;
   delete m_pBloomBufferTexture;
   m_pBloomBufferTexture = nullptr;
   delete m_pBloomTmpBufferTexture;
   m_pBloomTmpBufferTexture = nullptr;
   delete m_pPostProcessRenderTarget1;
   m_pPostProcessRenderTarget1 = nullptr;
   delete m_pPostProcessRenderTarget2;
   m_pPostProcessRenderTarget2 = nullptr;
   delete m_pPostProcessRenderTarget3;
   m_pPostProcessRenderTarget3 = nullptr;
   delete m_pReflectionBufferTexture;
   m_pReflectionBufferTexture = nullptr;
   delete m_pMotionBlurBufferTexture;
   m_pMotionBlurBufferTexture = nullptr;
   delete m_pOffscreenVRLeft;
   m_pOffscreenVRLeft = nullptr;
   delete m_pOffscreenVRRight;
   m_pOffscreenVRRight = nullptr;
   ReleaseAORenderTargets();
}

RenderTarget* RenderView::GetOutput() const
{
   if (m_ownedOutputRT)
      return m_ownedOutputRT.get();
   if (m_sharedOutputRT)
      return m_sharedOutputRT;
   return m_outputWnd ? m_outputWnd->GetBackBuffer() : nullptr;
}

vec4 RenderView::GetOutputAreaClip() const { return vec4(2.f * m_outputArea.x - 1.f, 1.f - 2.f * m_outputArea.w, 2.f * m_outputArea.z - 1.f, 1.f - 2.f * m_outputArea.y); }

void RenderView::GetOutputAreaSize(int& w, int& h) const
{
   const RenderTarget* const output = GetOutput();
   w = output == nullptr ? 0 : max(1, static_cast<int>(static_cast<float>(output->GetWidth()) * (m_outputArea.z - m_outputArea.x)));
   h = output == nullptr ? 0 : max(1, static_cast<int>(static_cast<float>(output->GetHeight()) * (m_outputArea.w - m_outputArea.y)));
}

void RenderView::MapToOutputArea(Vertex3D_TexelOnly* verts) const
{
   const vec4 clip = GetOutputAreaClip();
   for (int i = 0; i < 4; i++)
   {
      verts[i].x = clip.x + (verts[i].x + 1.f) * 0.5f * (clip.z - clip.x);
      verts[i].y = clip.y + (verts[i].y + 1.f) * 0.5f * (clip.w - clip.y);
   }
}

void RenderView::DrawOutputQuad(Shader* shader, RenderTarget* target) const
{
   RenderDevice* const rd = m_renderer->m_renderDevice;
   if (target != GetOutput() || IsFullOutputArea())
   {
      rd->DrawFullscreenTexturedQuad(shader);
      return;
   }
   const vec4 clip = GetOutputAreaClip();
   const Vertex3D_TexelOnly verts[4] = { { clip.z, clip.w, 0.0f, 1.f, 0.f }, { clip.x, clip.w, 0.0f, 0.f, 0.f }, { clip.z, clip.y, 0.0f, 1.f, 1.f }, { clip.x, clip.y, 0.0f, 0.f, 1.f } };
   rd->DrawTexturedQuad(shader, verts);
}

void RenderView::ResizeRenderTargets()
{
   Renderer* const r = m_renderer;
   RenderDevice* const rd = r->m_renderDevice;
   if (r->m_stereo3D == STEREO_VR && !IsMirror())
      return;
   if (m_outputWnd == nullptr)
      return; // Offscreen and shared render target views are resized explicitly by their owner
   const int wndWidth = m_outputWnd->GetPixelWidth(), wndHeight = m_outputWnd->GetPixelHeight();
   if (wndWidth <= 0 || wndHeight <= 0)
      return;

   // The view renders inside a normalized sub area of its output (the full window for the main player view)
   const int outWidth = max(1, static_cast<int>(static_cast<float>(wndWidth) * (m_outputArea.z - m_outputArea.x)));
   const int outHeight = max(1, static_cast<int>(static_cast<float>(wndHeight) * (m_outputArea.w - m_outputArea.y)));

   // Keep the current supersampling factor and stereo layout
   int newRenderWidth, newRenderHeight;
   if (!IsMirror() && r->m_stereo3D == STEREO_SBS)
   {
      newRenderWidth = outWidth / 2;
      newRenderHeight = outHeight;
   }
   else if (!IsMirror() && (r->m_stereo3D == STEREO_TB || r->m_stereo3D == STEREO_INT || r->m_stereo3D == STEREO_FLIPPED_INT))
   {
      newRenderWidth = outWidth;
      newRenderHeight = outHeight / 2;
   }
   else
   {
      newRenderWidth = outWidth;
      newRenderHeight = outHeight;
   }
   if (m_deferResize || (wndWidth == m_renderSourceWidth && wndHeight == m_renderSourceHeight && newRenderWidth == m_renderWidth && newRenderHeight == m_renderHeight))
      return; // While deferred (interactive resize), the render keeps its current targets and is stretched to the new output area
   m_renderSourceWidth = wndWidth;
   m_renderSourceHeight = wndHeight;
   m_renderWidth = newRenderWidth;
   m_renderHeight = newRenderHeight;
   const int renderWidthAA = max(1, static_cast<int>(static_cast<float>(m_renderWidth) * m_aaFactor));
   const int renderHeightAA = max(1, static_cast<int>(static_cast<float>(m_renderHeight) * m_aaFactor));
   PLOGI << "View " << (IsMirror() ? "VR desktop display" : "playfield") << " resized to " << outWidth << 'x' << outHeight << ", rendering at " << renderWidthAA << 'x' << renderHeightAA;

   // Frames already submitted may still reference the previous targets: release them at the end of the frame being prepared
   vector<RenderTarget*> released { m_pOffscreenBackBufferTexture1, m_pOffscreenBackBufferTexture2, m_pOffscreenMSAABackBufferTexture, m_pBloomBufferTexture, m_pBloomTmpBufferTexture,
      m_pPostProcessRenderTarget1, m_pPostProcessRenderTarget2, m_pPostProcessRenderTarget3, m_pReflectionBufferTexture, m_pMotionBlurBufferTexture, m_pAORenderTarget1, m_pAORenderTarget2,
      m_staticPrepassRT };
   std::erase(released, nullptr);
   rd->AddEndOfFrameCmd(
      [released]()
      {
         for (RenderTarget* rt : released)
            delete rt;
      });

   const RenderTarget* const previous = GetBackBufferTexture();
   const SurfaceType rtType = previous->m_type;
   const colorFormat renderFormat = previous->GetColorFormat();
   if (m_pOffscreenMSAABackBufferTexture)
      m_pOffscreenMSAABackBufferTexture = new RenderTarget(rd, rtType, "MSAABackBuffer"s, renderWidthAA, renderHeightAA, renderFormat, true,
         m_pOffscreenMSAABackBufferTexture->GetMSAASamples(), "Fatal Error: unable to create MSAA render buffer!");
   m_pOffscreenBackBufferTexture1
      = new RenderTarget(rd, rtType, "BackBuffer1"s, renderWidthAA, renderHeightAA, renderFormat, true, previous->GetMSAASamples(), "Fatal Error: unable to create offscreen back buffer");
   m_pOffscreenBackBufferTexture2 = m_pOffscreenBackBufferTexture1->Duplicate("BackBuffer2"s, false);
   m_pBloomBufferTexture
      = new RenderTarget(rd, rtType, "BloomBuffer1"s, max(1, m_renderWidth / 4), max(1, m_renderHeight / 4), renderFormat, false, 1, "Fatal Error: unable to create bloom buffer!");
   m_pBloomTmpBufferTexture = m_pBloomBufferTexture->Duplicate("BloomBuffer2"s);

   // Lazily recreated at the new size when next used
   m_pPostProcessRenderTarget1 = nullptr;
   m_pPostProcessRenderTarget2 = nullptr;
   m_pPostProcessRenderTarget3 = nullptr;
   m_pReflectionBufferTexture = nullptr;
   m_pMotionBlurBufferTexture = nullptr;
   m_pAORenderTarget1 = nullptr;
   m_pAORenderTarget2 = nullptr;
   m_staticPrepassRT = nullptr;
   m_isStaticPrepassDirty = true;
   for (RenderProbe* probe : r->m_table->m_vrenderprobe)
      probe->ReleaseRenderTargets(this);

   rd->m_basicShader->SetVector(
      ShaderUniform::w_h_height, (float)(1.0 / (double)GetMSAABackBufferTexture()->GetWidth()), (float)(1.0 / (double)GetMSAABackBufferTexture()->GetHeight()), 0.0f, 0.0f);
   rd->m_ballShader->SetVector(
      ShaderUniform::w_h_disableLighting, 1.5f / (float)GetPreviousBackBufferTexture()->GetWidth(), 1.5f / (float)GetPreviousBackBufferTexture()->GetHeight(), 0.f, 0.f);
   r->DisableBallLighting(r->m_table->GetSettings().GetPlayer_DisableLightingForBalls());
   // Views with an externally driven camera (editor panes push it through SetViewProj) keep it across the layout
   // reset which would otherwise restore the default view setup camera
   const Matrix3D customView = m_initialMVP.GetView(0);
   const Matrix3D customProj = m_initialMVP.GetProj(0);
   InitLayout();
   if (m_customViewProj)
      SetViewProj(customView, customProj);
}

void RenderView::SwapBackBufferRenderTargets()
{
   RenderTarget* tmp = m_pOffscreenBackBufferTexture1;
   m_pOffscreenBackBufferTexture1 = m_pOffscreenBackBufferTexture2;
   m_pOffscreenBackBufferTexture2 = tmp;
}

RenderTarget* RenderView::GetPostProcessRenderTarget1()
{
   if (m_pPostProcessRenderTarget1 == nullptr)
   {
      // Buffers for post-processing. Postprocess is done at scene resolution, on a LDR render target without MSAA nor full scene supersampling
      // excepted when using downsampled render buffer where upscaling is done after postprocessing.
      m_pPostProcessRenderTarget1 = new RenderTarget(m_renderer->m_renderDevice, GetBackBufferTexture()->m_type, "PostProcess1"s, min(GetBackBufferTexture()->GetWidth(), m_renderWidth),
         min(GetBackBufferTexture()->GetHeight(), m_renderHeight), GetBackBufferTexture()->GetColorFormat() == RGBA10 ? colorFormat::RGBA10 : colorFormat::RGBA8, false, 1,
         "Fatal Error: unable to create stereo3D/post-processing AA/sharpen buffer!");
   }
   return m_pPostProcessRenderTarget1;
}

RenderTarget* RenderView::GetPostProcessRenderTarget2()
{
   if (m_pPostProcessRenderTarget2 == nullptr)
      m_pPostProcessRenderTarget2 = GetPostProcessRenderTarget1()->Duplicate("PostProcess2"s);
   return m_pPostProcessRenderTarget2;
}

RenderTarget* RenderView::GetPostProcessRenderTarget3()
{
   // The previous back buffer is unused during post processing: reuse it, unless supersampled as it must have the post process size
   RenderTarget* pp1 = GetPostProcessRenderTarget1();
   RenderTarget* previous = GetPreviousBackBufferTexture();
   if (previous->GetWidth() == pp1->GetWidth() && previous->GetHeight() == pp1->GetHeight())
      return previous;
   if (m_pPostProcessRenderTarget3 == nullptr)
      m_pPostProcessRenderTarget3 = pp1->Duplicate("PostProcess3"s);
   return m_pPostProcessRenderTarget3;
}

RenderTarget* RenderView::GetPostProcessRenderTarget(RenderTarget* renderedRT)
{
   RenderTarget* pp1 = GetPostProcessRenderTarget1();
   if (renderedRT == pp1)
      return GetPostProcessRenderTarget2();
   else
      return pp1;
}

RenderTarget* RenderView::GetReflectionBufferTexture()
{
   if (m_pReflectionBufferTexture == nullptr)
   {
      m_pReflectionBufferTexture = new RenderTarget(m_renderer->m_renderDevice, GetBackBufferTexture()->m_type, "ReflectionBuffer"s, GetBackBufferTexture()->GetWidth(),
         GetBackBufferTexture()->GetHeight(), GetBackBufferTexture()->GetColorFormat(), false, 1, "Fatal Error: unable to create Reflection buffer!");
   }
   return m_pReflectionBufferTexture;
}

RenderTarget* RenderView::GetMotionBlurBufferTexture()
{
   if (m_pMotionBlurBufferTexture == nullptr)
   {
      m_pMotionBlurBufferTexture = new RenderTarget(m_renderer->m_renderDevice, GetBackBufferTexture()->m_type, "MotionBlurBuffer"s, GetBackBufferTexture()->GetWidth(),
         GetBackBufferTexture()->GetHeight(), GetBackBufferTexture()->GetColorFormat(), false, 1, "Fatal Error: unable to create MotionBlur buffer!");
   }
   return m_pMotionBlurBufferTexture;
}

RenderTarget* RenderView::GetAORenderTarget(int idx)
{
   // Lazily creates AO render target since this can be enabled during play from script (at render buffer resolution)
   if (m_pAORenderTarget1 == nullptr)
   {
      m_pAORenderTarget1
         = new RenderTarget(m_renderer->m_renderDevice, GetBackBufferTexture()->m_type, "AO1"s, GetBackBufferTexture()->GetWidth(), GetBackBufferTexture()->GetHeight(), colorFormat::GREY8,
            false, 1, "Unable to create AO buffers!\r\nPlease disable Ambient Occlusion.\r\nOr try to (un)set \"Alternative Depth Buffer processing\" in the video options!");
      m_pAORenderTarget2 = m_pAORenderTarget1->Duplicate("AO2"s);
   }
   return idx == 0 ? m_pAORenderTarget1 : m_pAORenderTarget2;
}

void RenderView::SwapAORenderTargets()
{
   RenderTarget* tmpAO = m_pAORenderTarget1;
   m_pAORenderTarget1 = m_pAORenderTarget2;
   m_pAORenderTarget2 = tmpAO;
}

int RenderView::GetDisplayWidth() const
{
   // Views may not have their render chain yet (created lazily in Render): use their output area size then
   if (GetBackBufferTexture() != nullptr)
      return m_renderer->m_stereo3D == STEREO_SBS ? (GetBackBufferTexture()->GetWidth() * 2) : GetBackBufferTexture()->GetWidth();
   int w, h;
   GetOutputAreaSize(w, h);
   return m_renderer->m_stereo3D == STEREO_SBS ? (w * 2) : w;
}

int RenderView::GetDisplayHeight() const
{
   if (GetBackBufferTexture() != nullptr)
      return (m_renderer->m_stereo3D == STEREO_TB || m_renderer->m_stereo3D == STEREO_INT || m_renderer->m_stereo3D == STEREO_FLIPPED_INT) ? (GetBackBufferTexture()->GetHeight() * 2)
                                                                                                                                           : GetBackBufferTexture()->GetHeight();
   int w, h;
   GetOutputAreaSize(w, h);
   return (m_renderer->m_stereo3D == STEREO_TB || m_renderer->m_stereo3D == STEREO_INT || m_renderer->m_stereo3D == STEREO_FLIPPED_INT) ? (h * 2) : h;
}

float RenderView::GetDisplayAspectRatio() const { return (float)((double)GetDisplayWidth() / (double)GetDisplayHeight()); }

// Setup the tables camera / rotation / scale (see InitLayout in Renderer.cpp for the layout modes description)
void RenderView::InitLayout(const float xpixoff, const float ypixoff)
{
   // TODO We should not call this function when in VR mode in the first place
   Renderer* const r = m_renderer;
   if (r->m_stereo3D == STEREO_VR)
      return;
   TRACE_FUNCTION();
   // The layout follows the view's own mode when one is forced (editor preview panes), else the table's mode
   const ViewSetupID mode = m_viewModeOverride != BG_INVALID ? m_viewModeOverride : r->m_table->GetViewMode();
   const ViewSetup& viewSetup = r->m_table->mViewSetups[mode];
#if defined(ENABLE_OPENGL) || defined(ENABLE_BGFX)
   const bool stereo = r->m_stereo3Denabled && (r->m_stereo3D != STEREO_OFF) && (r->m_stereo3D != STEREO_VR);
#elif defined(ENABLE_DX9)
   constexpr bool stereo = false;
#endif
   // A view outputting to a sub-rect of its target (editor panes) is framed on its own aspect, not the display's
   int outW = 0, outH = 0;
   GetOutputAreaSize(outW, outH);
   const float aspect = outW > 0 && outH > 0 ? static_cast<float>(outW) / static_cast<float>(outH) : GetDisplayAspectRatio();
   viewSetup.ComputeMVP(r->m_table, aspect, stereo, m_mvp, vec3(r->m_cam.x, r->m_cam.y, r->m_cam.z), r->m_inc, xpixoff / (float)GetDisplayWidth(), ypixoff / (float)GetDisplayHeight());
   for (unsigned int eye = 0; eye < m_mvp.m_nEyes; eye++)
      m_isStaticPrepassDirty |= !(m_mvp.GetView(eye) == m_initialMVP.GetView(eye)) || !(m_mvp.GetProj(eye) == m_initialMVP.GetProj(eye));
   m_initialMVP = m_mvp;
}

void RenderView::ApplyViewJitter(const float xpixoff, const float ypixoff)
{
   assert(m_renderer->m_stereo3D != STEREO_VR);
   // Apply the same clip space offset as InitLayout (see ViewSetup::ComputeMVP projTrans), but on the current MVP, preserving
   // the (eventually reflected) view matrix since the offset is a post projection translation which does not interact with it
   const Matrix3D offset = Matrix3D::MatrixTranslate(xpixoff / (float)GetDisplayWidth(), ypixoff / (float)GetDisplayHeight(), 0.f);
   for (unsigned int eye = 0; eye < m_mvp.m_nEyes; eye++)
      m_mvp.SetProj(eye, m_mvp.GetProj(eye) * offset);
}

void RenderView::SetFlip(ModelViewProj::FlipMode flipMode)
{
   assert(m_renderer->m_stereo3D != STEREO_VR);
   // Set on the current MVP: InitLayout recomputes it, then copies it to the initial MVP (which resets the current MVP each frame)
   m_mvp.SetFlip(flipMode);
   InitLayout();
}

void RenderView::SetReflection(const Matrix3D& reflectionMatrix)
{
   m_mvp.SetReflection(reflectionMatrix);
   SetSpaceReference(m_mvpSpaceReference, true);
}

void RenderView::SetViewProj(const Matrix3D& view, const Matrix3D& proj)
{
   assert(m_renderer->m_stereo3D != STEREO_VR);
   m_customViewProj = true;
   m_initialMVP.SetView(0, view);
   m_initialMVP.SetView(1, view);
   m_initialMVP.SetProj(0, proj);
   m_initialMVP.SetProj(1, proj);
}

Vertex3Ds RenderView::Unproject(const int width, const int height, const Vertex3Ds& point) const
{
   assert(m_renderer->m_stereo3D != STEREO_VR);
   Matrix3D invMVP = m_initialMVP.GetModelViewProj(0);
   invMVP.Invert();
   const Vertex3Ds p(2.0f * point.x / static_cast<float>(width) - 1.0f, 1.0f - 2.0f * point.y / static_cast<float>(height), (point.z - 0.f /* MinZ */) / (1.f /* MaxZ */ - 0.f /* MinZ */));
   return invMVP * p;
}

Vertex3Ds RenderView::Get3DPointFrom2D(const int width, const int height, const Vertex2D& p, float z)
{
   const Vertex3Ds pNear(p.x, p.y, 0.f /* MinZ */);
   const Vertex3Ds pFar(p.x, p.y, 1.f /* MaxZ */);
   const Vertex3Ds p1 = Unproject(width, height, pNear);
   const Vertex3Ds p2 = Unproject(width, height, pFar);
   const float wz = z;
   const float wx = (wz - p1.z) * (p2.x - p1.x) / (p2.z - p1.z) + p1.x;
   const float wy = (wz - p1.z) * (p2.y - p1.y) / (p2.z - p1.z) + p1.y;
   return { wx, wy, wz };
}

Vertex2D RenderView::BackdropToClip(const Vertex2D& pos) const
{
   if (m_editorBackdropEdit)
   {
      // The live editor setup its own backdrop ortho MVP to allow pan & zoom
      return (m_mvp.GetView(0) * m_mvp.GetProj(0) * Vertex3Ds(pos.x, pos.y, 0.f)).xy();
   }
   else
   {
      // Otherwise, defaults to a fixed full screen ortho camera that scales to EDITOR_BG_WIDTH x EDITOR_BG_HEIGHT
      return Vertex2D(2.0f * pos.x / (float)EDITOR_BG_WIDTH - 1.0f, 1.0f - 2.0f * pos.y / (float)EDITOR_BG_HEIGHT);
   }
}

void RenderView::UpdateBasicShaderMatrix(const Matrix3D& objectTrafo)
{
   RenderDevice* const rd = m_renderer->m_renderDevice;
   m_mvp.SetModel(objectTrafo);

#if defined(ENABLE_BGFX)
   rd->m_basicShader->SetMatrix(ShaderUniform::matWorld, &m_mvp.GetModel());
   rd->m_basicShader->SetMatrix(ShaderUniform::matView, &m_mvp.GetView(0), m_mvp.m_nEyes);
   rd->m_basicShader->SetMatrix(ShaderUniform::matWorldView, &m_mvp.GetModelView(0), m_mvp.m_nEyes);
   rd->m_basicShader->SetMatrix(ShaderUniform::matWorldViewInverseTranspose, &m_mvp.GetModelViewInverseTranspose(0), m_mvp.m_nEyes);
   rd->m_DMDShader->SetMatrix(ShaderUniform::matWorld, &m_mvp.GetModel());

   // Camera-relative uniforms. The shader subtracts cameraPosWorld from the world
   // position on the GPU, then applies (viewRotation x proj). This avoids Adreno (Quest) f32
   // precision loss in mul(matWorldViewProj, pos) where the camera-translation column dominates the
   // CPU-composed mvp entries; the cancellation now happens at full f32 precision per-vertex.
   rd->m_basicShader->SetMatrix(ShaderUniform::matRotViewProj, &m_mvp.GetRotViewProj(0), m_mvp.m_nEyes);
   rd->m_basicShader->SetVector(ShaderUniform::cameraPosWorld, &m_mvp.GetCameraPos(0), m_mvp.m_nEyes);
   rd->m_lightShader->SetMatrix(ShaderUniform::matRotViewProj, &m_mvp.GetRotViewProj(0), m_mvp.m_nEyes);
   rd->m_lightShader->SetVector(ShaderUniform::cameraPosWorld, &m_mvp.GetCameraPos(0), m_mvp.m_nEyes);
   rd->m_flasherShader->SetMatrix(ShaderUniform::matRotViewProj, &m_mvp.GetRotViewProj(0), m_mvp.m_nEyes);
   rd->m_flasherShader->SetVector(ShaderUniform::cameraPosWorld, &m_mvp.GetCameraPos(0), m_mvp.m_nEyes);
   rd->m_DMDShader->SetMatrix(ShaderUniform::matRotViewProj, &m_mvp.GetRotViewProj(0), m_mvp.m_nEyes);
   rd->m_DMDShader->SetVector(ShaderUniform::cameraPosWorld, &m_mvp.GetCameraPos(0), m_mvp.m_nEyes);

#elif defined(ENABLE_DX9)
   rd->m_basicShader->SetMatrix(ShaderUniform::matWorld, &m_mvp.GetModel());
   rd->m_basicShader->SetMatrix(ShaderUniform::matView, &m_mvp.GetView(0), m_mvp.m_nEyes);
   rd->m_basicShader->SetMatrix(ShaderUniform::matWorldView, &m_mvp.GetModelView(0), m_mvp.m_nEyes);
   rd->m_basicShader->SetMatrix(ShaderUniform::matWorldViewInverseTranspose, &m_mvp.GetModelViewInverseTranspose(0), m_mvp.m_nEyes);
   rd->m_basicShader->SetMatrix(ShaderUniform::matWorldViewProj, &m_mvp.GetModelViewProj(0), m_mvp.m_nEyes);
   rd->m_flasherShader->SetMatrix(ShaderUniform::matWorldViewProj, &m_mvp.GetModelViewProj(0), m_mvp.m_nEyes);
   rd->m_lightShader->SetMatrix(ShaderUniform::matWorldViewProj, &m_mvp.GetModelViewProj(0), m_mvp.m_nEyes);
   rd->m_DMDShader->SetMatrix(ShaderUniform::matWorldViewProj, &m_mvp.GetModelViewProj(0), m_mvp.m_nEyes);

#elif defined(ENABLE_OPENGL)
   if (m_mvp.m_nEyes == 2)
   {
      struct
      {
         Matrix3D matWorld;
         Matrix3D matView[2];
         Matrix3D matWorldView[2];
         Matrix3D matWorldViewInverseTranspose[2];
         Matrix3D matWorldViewProj[2];
      } matrices;
      matrices.matWorld = m_mvp.GetModel();
      for (unsigned int eye = 0; eye < m_mvp.m_nEyes; eye++)
      {
         matrices.matView[eye] = m_mvp.GetView(eye);
         matrices.matWorldView[eye] = m_mvp.GetModelView(eye);
         matrices.matWorldViewInverseTranspose[eye] = m_mvp.GetModelViewInverseTranspose(eye);
         matrices.matWorldViewProj[eye] = m_mvp.GetModelViewProj(eye);
      }
      rd->m_basicShader->SetUniformBlock(ShaderUniform::basicMatrixBlock, &matrices.matWorld.m[0][0]);
      rd->m_flasherShader->SetMatrix(ShaderUniform::matWorldViewProj, &matrices.matWorldViewProj[0], m_mvp.m_nEyes);
      rd->m_lightShader->SetMatrix(ShaderUniform::matWorldViewProj, &matrices.matWorldViewProj[0], m_mvp.m_nEyes);
      rd->m_DMDShader->SetMatrix(ShaderUniform::matWorldViewProj, &matrices.matWorldViewProj[0], m_mvp.m_nEyes);
   }
   else
   {
      struct
      {
         Matrix3D matWorld;
         Matrix3D matView;
         Matrix3D matWorldView;
         Matrix3D matWorldViewInverseTranspose;
         Matrix3D matWorldViewProj;
      } matrices;
      matrices.matWorld = m_mvp.GetModel();
      matrices.matView = m_mvp.GetView(0);
      matrices.matWorldView = m_mvp.GetModelView(0);
      matrices.matWorldViewInverseTranspose = m_mvp.GetModelViewInverseTranspose(0);
      matrices.matWorldViewProj = m_mvp.GetModelViewProj(0);
      rd->m_basicShader->SetUniformBlock(ShaderUniform::basicMatrixBlock, &matrices.matWorld.m[0][0]);
      rd->m_flasherShader->SetMatrix(ShaderUniform::matWorldViewProj, &matrices.matWorldViewProj, m_mvp.m_nEyes);
      rd->m_lightShader->SetMatrix(ShaderUniform::matWorldViewProj, &matrices.matWorldViewProj, m_mvp.m_nEyes);
      rd->m_DMDShader->SetMatrix(ShaderUniform::matWorldViewProj, &matrices.matWorldViewProj, m_mvp.m_nEyes);
   }
#endif
}

void RenderView::UpdateBallShaderMatrix()
{
   RenderDevice* const rd = m_renderer->m_renderDevice;
   m_mvp.SetModel(Matrix3D::MatrixIdentity());

#if defined(ENABLE_BGFX)
   rd->m_ballShader->SetMatrix(ShaderUniform::matWorldView, &m_mvp.GetModelView(0), m_mvp.m_nEyes);
   rd->m_ballShader->SetMatrix(ShaderUniform::matWorldViewInverse, &m_mvp.GetModelViewInverse(0), m_mvp.m_nEyes);
   rd->m_ballShader->SetMatrix(ShaderUniform::matView, &m_mvp.GetView(0), m_mvp.m_nEyes);
   rd->m_ballShader->SetMatrix(ShaderUniform::matRotViewProj, &m_mvp.GetRotViewProj(0), m_mvp.m_nEyes);
   rd->m_ballShader->SetVector(ShaderUniform::cameraPosWorld, &m_mvp.GetCameraPos(0), m_mvp.m_nEyes);

#elif defined(ENABLE_DX9)
   rd->m_ballShader->SetMatrix(ShaderUniform::matWorldViewProj, &m_mvp.GetModelViewProj(0), m_mvp.m_nEyes);
   rd->m_ballShader->SetMatrix(ShaderUniform::matWorldView, &m_mvp.GetModelView(0), m_mvp.m_nEyes);
   rd->m_ballShader->SetMatrix(ShaderUniform::matWorldViewInverse, &m_mvp.GetModelViewInverse(0), m_mvp.m_nEyes);
   rd->m_ballShader->SetMatrix(ShaderUniform::matView, &m_mvp.GetView(0), m_mvp.m_nEyes);

#elif defined(ENABLE_OPENGL)
   if (m_mvp.m_nEyes == 2)
   {
      struct
      {
         Matrix3D matView[2];
         Matrix3D matWorldView[2];
         Matrix3D matWorldViewInverse[2];
         Matrix3D matWorldViewProj[2];
      } matrices;
      for (unsigned int eye = 0; eye < m_mvp.m_nEyes; eye++)
      {
         matrices.matView[eye] = m_mvp.GetView(eye);
         matrices.matWorldView[eye] = m_mvp.GetModelView(eye);
         matrices.matWorldViewInverse[eye] = m_mvp.GetModelViewInverse(eye);
         matrices.matWorldViewProj[eye] = m_mvp.GetModelViewProj(eye);
      }
      rd->m_ballShader->SetUniformBlock(ShaderUniform::ballMatrixBlock, &matrices.matView[0].m[0][0]);
   }
   else
   {
      struct
      {
         Matrix3D matView;
         Matrix3D matWorldView;
         Matrix3D matWorldViewInverse;
         Matrix3D matWorldViewProj;
      } matrices;
      matrices.matView = m_mvp.GetView(0);
      matrices.matWorldView = m_mvp.GetModelView(0);
      matrices.matWorldViewInverse = m_mvp.GetModelViewInverse(0);
      matrices.matWorldViewProj = m_mvp.GetModelViewProj(0);
      rd->m_ballShader->SetUniformBlock(ShaderUniform::ballMatrixBlock, &matrices.matView.m[0][0]);
   }
#endif
}

void RenderView::UpdateDesktopBackdropShaderMatrix(bool basic, bool light, bool flasherDMD)
{
   RenderDevice* const rd = m_renderer->m_renderDevice;
   Matrix3D matWorldViewProj[2]; // MVP to move from back buffer space (0..w, 0..h) to clip space (-1..1, -1..1)
   matWorldViewProj[0].SetIdentity();
   if (m_editorBackdropEdit)
   {
      // The live editor setup its own backdrop ortho MVP, but we need to adapt it to the render target scale
      matWorldViewProj[0]
         = Matrix3D::MatrixScale((float)EDITOR_BG_WIDTH / (float)rd->GetCurrentRenderTarget()->GetWidth(), (float)EDITOR_BG_HEIGHT / (float)rd->GetCurrentRenderTarget()->GetHeight(), 1.f)
         * m_mvp.GetView(0) * m_mvp.GetProj(0);
   }
   else
   {
      matWorldViewProj[0]._11 = 2.0f / (float)rd->GetCurrentRenderTarget()->GetWidth();
      matWorldViewProj[0]._41 = -1.0f;
      matWorldViewProj[0]._22 = -2.0f / (float)rd->GetCurrentRenderTarget()->GetHeight();
      matWorldViewProj[0]._42 = 1.f;
   }
   const int eyes = rd->GetCurrentRenderTarget()->m_nLayers;
   if (eyes > 1)
      matWorldViewProj[1] = matWorldViewProj[0];

#if defined(ENABLE_BGFX)
   const vec4 cameraPosWorld[2] = { { 0.f, 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f, 0.f } };
   if (basic)
   {
      rd->m_basicShader->SetMatrix(ShaderUniform::matRotViewProj, &matWorldViewProj[0], eyes);
      rd->m_basicShader->SetVector(ShaderUniform::cameraPosWorld, &cameraPosWorld[0], eyes);
   }
   if (light)
   {
      rd->m_lightShader->SetMatrix(ShaderUniform::matRotViewProj, &matWorldViewProj[0], eyes);
      rd->m_lightShader->SetVector(ShaderUniform::cameraPosWorld, &cameraPosWorld[0], eyes);
   }
   if (flasherDMD)
   {
      rd->m_flasherShader->SetMatrix(ShaderUniform::matRotViewProj, &matWorldViewProj[0], eyes);
      rd->m_flasherShader->SetVector(ShaderUniform::cameraPosWorld, &cameraPosWorld[0], eyes);
      rd->m_DMDShader->SetMatrix(ShaderUniform::matRotViewProj, &matWorldViewProj[0], eyes);
      rd->m_DMDShader->SetVector(ShaderUniform::cameraPosWorld, &cameraPosWorld[0], eyes);
   }

#else
   if (basic)
   {
#if defined(ENABLE_OPENGL)
      rd->SetBasicShaderScreenSpaceMatrices(matWorldViewProj[0]);

#elif defined(ENABLE_DX9)
      rd->m_basicShader->SetMatrix(ShaderUniform::matWorldViewProj, &matWorldViewProj[0]);

#endif
   }

   if (light)
      rd->m_lightShader->SetMatrix(ShaderUniform::matWorldViewProj, &matWorldViewProj[0], eyes);

   if (flasherDMD)
   {
      rd->m_flasherShader->SetMatrix(ShaderUniform::matWorldViewProj, &matWorldViewProj[0], eyes);
      rd->m_DMDShader->SetMatrix(ShaderUniform::matWorldViewProj, &matWorldViewProj[0], eyes);
   }
#endif
}

void RenderView::SetSpaceReference(PartGroupData::SpaceReference spaceReference, bool force)
{
   Renderer* const r = m_renderer;
   if (!force && m_mvpSpaceReference == spaceReference)
      return;

#if defined(ENABLE_XR)
   if (r->m_stereo3D == STEREO_VR && !IsMirror())
      g_pplayer->m_vrDevice->UpdateVRPosition(spaceReference, m_mvp);
   else
#endif
   {
      switch (spaceReference)
      {
      case PartGroupData::SpaceReference::SR_CABINET:
      case PartGroupData::SpaceReference::SR_CABINET_FEET:
      case PartGroupData::SpaceReference::SR_ROOM:
         for (unsigned int eye = 0; eye < m_mvp.m_nEyes; eye++)
            m_mvp.SetView(eye, g_pplayer->m_ptable->GetDefaultPlayfieldToCabMatrix() * m_playfieldView[eye]);
         break;

      case PartGroupData::SpaceReference::SR_PLAYFIELD:
      default:
         for (unsigned int eye = 0; eye < m_mvp.m_nEyes; eye++)
            m_mvp.SetView(eye, m_playfieldView[eye]);
         break;
      }
   }

   // Apply nudge to cabinet parts (as we don't nudge the room) but only when not using static prepass (which uses a simple screen shifting)
   if (spaceReference != PartGroupData::SpaceReference::SR_ROOM && !IsUsingStaticPrepass() && r->m_visualNudgeStrength > 0.f)
   {
      if (const auto nudge = g_pplayer->m_pininput.m_nudgeHandler->GetCabinetOffset(); nudge.x != 0.f || nudge.y != 0.f)
      {
         const Matrix3D nudgeMat = Matrix3D::MatrixTranslate(r->m_visualNudgeStrength * MTOVPU(nudge.x), r->m_visualNudgeStrength * MTOVPU(nudge.y), 0.f);
         for (unsigned int eye = 0; eye < m_mvp.m_nEyes; eye++)
            m_mvp.SetView(eye, nudgeMat * m_mvp.GetView(eye));
      }
   }

   m_mvpSpaceReference = spaceReference;
   UpdateBasicShaderMatrix();
   UpdateBallShaderMatrix();
}

void RenderView::SetScreenOffset(const float x, const float y)
{
   int outW, outH;
   GetOutputAreaSize(outW, outH);
   const float rotation = m_renderer->m_stereo3D == STEREO_VR ? 0.f : ANGTORAD(m_renderer->m_table->GetViewSetup().GetRotation(m_renderer->m_stereo3D, outW, outH));
   const float c = cosf(-rotation), s = sinf(-rotation);
   m_screenOffset.x = x * c - y * s;
   m_screenOffset.y = x * s + y * c;
}

bool RenderView::IsUsingStaticPrepass() const
{
   // LiveUI may not exist yet during player startup (e.g. VR setup calls DisableStaticPrePass from the Player constructor)
   return !m_editorView // Editor views do not use the static prepass (FIXME why was it disabled ?)
      && !g_pplayer->IsVR() // VR disable prerender accumulation to avoid the perf impact as the view constantly moves
      && g_pplayer->GetInfoMode()
      != IF_DYNAMIC_ONLY // Dynamic inspection hides static parts (TODO remove embedded inspection as RenderDoc superseed this function and only DX9 still uses it)
#if !defined(ENABLE_BGFX)
      // OpenGL is able to support MSAA + static prepass (glBlitFramebuffer allows non-MSAA to MSAA blits) but it is disabled
      // as the resources to maintain it are too low and it is EOL. Under BGFX, the prepass compose is done with quad draws
      && !GetMSAABackBufferTexture()->IsMSAA()
#endif
      && m_shadeMode == ShadeMode::Default // TODO Wireframe rendering modes are not tested for prerendering optimization yet
      && m_disableStaticPrepass <= 0; // Something resuested to disable static part accumulation (for example headtracking,...)
}

bool RenderView::IsTemporalAccumulationInProgress() const
{
   if (IsUsingStaticPrepass() && (m_isStaticPrepassDirty || m_staticPrepassAccumCount < STATIC_PRERENDER_ITERATIONS))
      return true;
   for (const RenderProbe* probe : m_renderer->m_table->m_vrenderprobe)
      if (probe->IsStaticAccumulationPending())
         return true;
   return false;
}

void RenderView::RenderStatics()
{
   Renderer* const r = m_renderer;
   RenderDevice* const rd = r->m_renderDevice;
   // Static parts may have changed while the prepass was not used (e.g. edited in the editor, which disables it): refresh it when it is used again
   const bool useStaticPrepass = IsUsingStaticPrepass();
   m_isStaticPrepassDirty |= useStaticPrepass && !m_wasUsingStaticPrepass;
   m_wasUsingStaticPrepass = useStaticPrepass;

   if (!useStaticPrepass)
   {
      rd->SetRenderTarget("Render Background"s, GetMSAABackBufferTexture());
      DrawBackground();
      rd->SetRenderTarget("Render Scene"s, GetMSAABackBufferTexture(), true, true); // Force new pass to avoid sorting background draw calls with 3D rendering draw calls
      return;
   }

   if (m_isStaticPrepassDirty)
   {
      m_isStaticPrepassDirty = false;
      PLOGI << "Performing prerendering of static parts."; // For profiling
      // Defer deletion to the render thread end-of-frame: already submitted frames may still hold copy commands sourcing this RT
      if (m_staticPrepassRT)
         rd->AddEndOfFrameCmd([rt = m_staticPrepassRT]() { delete rt; });
      // Always non-MSAA: the accumulated content is per-pixel (under BGFX MSAA the backbuffer itself is MSAA and
      // duplicating it would needlessly allocate MSAA resolve machinery)
      const RenderTarget* const backBuffer = GetBackBufferTexture();
      m_staticPrepassRT = new RenderTarget(rd, backBuffer->m_type, "StaticPreRender"s, backBuffer->GetWidth(), backBuffer->GetHeight(), backBuffer->GetColorFormat(), backBuffer->HasDepth(),
         1, "Failed to create static prerender target");
      for (RenderProbe* probe : r->m_table->m_vrenderprobe)
         probe->MarkDirtyStatics(this);
      m_staticPrepassAccumCount = 0;
   }

   if (m_staticPrepassAccumCount < STATIC_PRERENDER_ITERATIONS)
   {
      const bool isFirstSample = m_staticPrepassAccumCount == 0;
      RenderTarget* sampleRT = GetBackBufferTexture(); // We use the back buffer as a temporary target (then accumulated to the static RT, which is copied back to backbuffer)
      rd->m_curDrawnTriangles = 0;

      // Setup the view matrices with a jitter depending on the accumulation step.
      // The first accumulated sample always uses a centered offset so that the acquired depth buffer is the unjittered one.
      float u1 = xyLDBNbnot[m_staticPrepassAccumCount * 2];
      float u2 = xyLDBNbnot[m_staticPrepassAccumCount * 2 + 1];
      if (!isFirstSample)
      {
         // the following line implements filter importance sampling for a small gauss (i.e. less jaggies as it also samples neighboring pixels) -> but also potentially more artifacts in compositing!
         gaussianDistribution(u1, u2, 0.5f, 0.0f); //!! first 0.5 could be increased for more blur, but is pretty much what is recommended
         // sanity check to be sure to limit filter area to 3x3 in practice, as the gauss transformation is unbound (which is correct, but for our use-case/limited amount of samples very bad)
         assert(u1 > -1.5f && u1 < 1.5f);
         assert(u2 > -1.5f && u2 < 1.5f);
         // First accumulated sample MUST set a sample offset of 0,0 so that the depth buffer features 'correctly' centered pixel sample
         assert(m_staticPrepassAccumCount != 0 || (u1 == 0.f && u2 == 0.f));
         // Apply the jitter to the reflected MVP (the offset is applied to the projection, preserving the reflected view)
         ApplyViewJitter(u1, u2);
      }

      // if rendering static/with heavy oversampling, disable mipmaps & aniso/trilinear filter to get a sharper/more precise result overall!
      ShaderState::m_disableMipmaps = true;
#ifdef ENABLE_BGFX
      rd->m_DMDShader->SetVector(ShaderUniform::u_basic_shade_mode, 0.f, 0.f, 0.f, 1.f);
#endif

      rd->SetRenderTarget("PreRender Background"s, sampleRT, isFirstSample, true);
      DrawBackground();

      rd->SetRenderTarget("PreRender Draw"s, sampleRT, true, true); // Force new pass to avoid sorting background rendering draw calls with 3D rendering draw calls
      UpdateBasicShaderMatrix();
      m_renderer->DrawStatics();

      if (!isFirstSample)
         ApplyViewJitter(-u1, -u2); // Restore the unjittered projection

      // Static only AO is done in a single step, then it is applied after each render before accumulation (could be done more efficiently by sampling it directly)
      const bool useStaticAO = r->GetAOMode() == 1;
      if (useStaticAO)
      {
         if (isFirstSample)
         {
            // Compute static AO from the unjittered first sample (its depth is the one kept for the accumulated render)
            rd->SetRenderTarget("PreRender AO Clear"s, GetAORenderTarget(1));
            rd->Clear(clearType::TARGET, 0x00000000); // Clear stale AO history (freshly created buffers may hold garbage)
            for (int i = 0; i < 50; i++) // 50 iterations to get AO smooth
               UpdateAmbientOcclusion(sampleRT, i);
            rd->m_FBShader->SetTextureNull(ShaderUniform::tex_depth);
         }

         // Apply AO to the rendered sample before accumulating it (the reflection buffer is a transient target, fully overwritten by the SSR pass)
         const bool useAA = m_renderWidth > GetBackBufferTexture()->GetWidth();
         rd->SetRenderTarget("PreRender Apply AO"s, GetReflectionBufferTexture());
         rd->AddRenderTargetDependency(sampleRT);
         rd->AddRenderTargetDependency(GetAORenderTarget(1));
         rd->ResetRenderState();
         rd->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_FALSE);
         rd->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
         rd->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
         rd->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);
         rd->m_FBShader->SetTexture(ShaderUniform::tex_fb_filtered, sampleRT->GetColorSampler());
         rd->m_FBShader->SetTexture(ShaderUniform::tex_fb_unfiltered, sampleRT->GetColorSampler());
         rd->m_FBShader->SetTexture(ShaderUniform::tex_ao, GetAORenderTarget(1)->GetColorSampler());
         rd->m_FBShader->SetVector(ShaderUniform::w_h_height, (float)(1.0 / sampleRT->GetWidth()), (float)(1.0 / sampleRT->GetHeight()), 1.0f, 1.0f);
         rd->m_FBShader->SetTechnique(useAA ? ShaderTechnique::fb_AO_static : ShaderTechnique::fb_AO_no_filter_static);
         rd->DrawFullscreenTexturedQuad(rd->m_FBShader);
         rd->m_FBShader->SetTextureNull(ShaderUniform::tex_ao);
         sampleRT = GetReflectionBufferTexture();
      }

      // Accumulate the new sample into the static render target, weighted by 1/STATIC_PRERENDER_ITERATIONS
      m_staticPrepassAccumCount++;
      rd->SetRenderTarget("PreRender Accumulate"s, m_staticPrepassRT);
      rd->AddRenderTargetDependency(sampleRT);
      if (isFirstSample) // Copy unjiterred depth buffer rendered on first sample
      {
         if (useStaticAO)
            rd->AddRenderTargetDependency(GetBackBufferTexture());
         rd->BlitRenderTarget(GetBackBufferTexture(), m_staticPrepassRT, false, true);
      }
      rd->ResetRenderState();
      rd->SetRenderState(RenderState::ALPHABLENDENABLE, isFirstSample ? RenderState::RS_FALSE : RenderState::RS_TRUE);
      rd->SetRenderState(RenderState::SRCBLEND, RenderState::ONE);
      rd->SetRenderState(RenderState::DESTBLEND, RenderState::ONE);
      rd->SetRenderState(RenderState::BLENDOP, RenderState::BLENDOP_ADD);
      rd->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);
      rd->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
      rd->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
      rd->m_FBShader->SetTechnique(ShaderTechnique::fb_mirror);
      rd->m_FBShader->SetVector(ShaderUniform::w_h_height, (float)(1.0 / (double)GetBackBufferTexture()->GetWidth()), (float)(1.0 / (double)GetBackBufferTexture()->GetHeight()),
         (float)(double)STATIC_PRERENDER_ITERATIONS, 0.0f);
      rd->m_FBShader->SetTexture(ShaderUniform::tex_fb_unfiltered, sampleRT->GetColorSampler());
      rd->DrawFullscreenTexturedQuad(rd->m_FBShader);
      rd->m_FBShader->SetTextureNull(ShaderUniform::tex_fb_unfiltered);

      ShaderState::m_disableMipmaps = false;
#ifdef ENABLE_BGFX
      rd->m_DMDShader->SetVector(ShaderUniform::u_basic_shade_mode, 0.f, 0.f, 0.f, 0.f);
#endif

      if (m_staticPrepassAccumCount >= STATIC_PRERENDER_ITERATIONS)
      {
         PLOGI << "Static PreRender done"; // For profiling
         // Delete buffers: we won't need them anymore since static AO was baked in the accumulated render and dynamic AO is disabled in this mode
         if (useStaticAO)
            rd->AddEndOfFrameCmd([this]() { ReleaseAORenderTargets(); });
      }

      // Store the total number of triangles prerendered (including ones done for render probes)
      m_statsDrawnStaticTriangles = rd->m_curDrawnTriangles;
   }

   // Copy the accumulated background + static part renders
   rd->SetRenderTarget("PreRender Compose"s, GetMSAABackBufferTexture());
   rd->AddRenderTargetDependency(m_staticPrepassRT);
   if (m_staticPrepassAccumCount >= STATIC_PRERENDER_ITERATIONS)
   {
      // Accumulation is complete: the static render target holds the final rendering
      rd->BlitRenderTarget(m_staticPrepassRT, GetMSAABackBufferTexture());
   }
   else
   {
      // The static render target holds the scaled accumulation: its depth is copied as-is while its color is
      // scaled back up by STATIC_PRERENDER_ITERATIONS/accumulated samples (weight reaches 1 when accumulation is complete)
      rd->BlitRenderTarget(m_staticPrepassRT, GetMSAABackBufferTexture(), false, true);
      rd->ResetRenderState();
      rd->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_FALSE);
      rd->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);
      rd->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
      rd->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
      rd->m_FBShader->SetTechnique(ShaderTechnique::fb_mirror);
      rd->m_FBShader->SetVector(ShaderUniform::w_h_height, (float)(1.0 / (double)m_staticPrepassRT->GetWidth()), (float)(1.0 / (double)m_staticPrepassRT->GetHeight()),
         (float)(std::max(1, m_staticPrepassAccumCount) / (double)STATIC_PRERENDER_ITERATIONS), 0.0f);
      rd->m_FBShader->SetTexture(ShaderUniform::tex_fb_unfiltered, m_staticPrepassRT->GetColorSampler());
      rd->DrawFullscreenTexturedQuad(rd->m_FBShader);
      rd->m_FBShader->SetTextureNull(ShaderUniform::tex_fb_unfiltered);
   }
   rd->SetRenderTarget("Render Scene"s, GetMSAABackBufferTexture(), true, true); // Force new pass to avoid sorting the statics composite draw calls with 3D rendering draw calls
}

void RenderView::RenderDynamics()
{
   Renderer* const r = m_renderer;
   RenderDevice* const rd = r->m_renderDevice;
   if (g_pplayer->GetInfoMode() == IF_STATIC_ONLY)
      return;

   TRACE_FUNCTION();

   // Setup the projection matrices used for refraction and ball reflection
   SetSpaceReference(PartGroupData::SpaceReference::SR_PLAYFIELD, false);
   Matrix3D matProj[2];
   const int nEyes = rd->m_nEyes;
   for (int eye = 0; eye < nEyes; eye++)
      matProj[eye] = GetMVP().GetFlippedProj(eye);
   rd->m_basicShader->SetMatrix(ShaderUniform::matProj, &matProj[0], nEyes);
   rd->m_ballShader->SetMatrix(ShaderUniform::matProj, &matProj[0], nEyes);

   // Update ball pos uniforms
   vec4 balls[MAX_BALL_SHADOW];
   int p = 0;
   for (size_t i = 0; i < g_pplayer->m_vball.size() && p < MAX_BALL_SHADOW; i++)
   {
      Ball* const pball = g_pplayer->m_vball[i];
      if (!pball->m_d.m_visible)
         continue;
      balls[p] = vec4(pball->GetPosition(), pball->GetRadius());
      p++;
   }
   for (; p < MAX_BALL_SHADOW; p++)
      balls[p] = vec4(-1000.f, -1000.f, -1000.f, 0.0f);
   rd->m_lightShader->SetFloat4v(ShaderUniform::balls, balls, MAX_BALL_SHADOW);
   rd->m_basicShader->SetFloat4v(ShaderUniform::balls, balls, MAX_BALL_SHADOW);
   rd->m_flasherShader->SetFloat4v(ShaderUniform::balls, balls, MAX_BALL_SHADOW);

   UpdateBasicShaderMatrix();
   UpdateBallShaderMatrix();

   // In the live editor's desktop backdrop mode, only render backdrop parts (and skip the playfield bulb light buffer)
   const bool isBackdropEdit = m_editorBackdropEdit;

   r->m_render_mask = IsUsingStaticPrepass() ? Renderer::DYNAMIC_ONLY : Renderer::DEFAULT;
   if (!isBackdropEdit)
      DrawBulbLightBuffer();
   for (auto renderable : g_pplayer->m_ptable->GetParts())
   {
      if (isBackdropEdit && !renderable->m_desktopBackdrop)
         continue;
      r->RenderItem(renderable, m_noBackdrop);
   }

   rd->m_basicShader->SetTextureNull(ShaderUniform::tex_base_transmission); // need to reset the bulb light texture, as its used as render target for bloom again

   for (size_t i = 0; i < r->m_table->m_vrenderprobe.size(); ++i)
      r->m_table->m_vrenderprobe[i]->ApplyAreaOfInterest();
}

void RenderView::DrawBackground()
{
   Renderer* const r = m_renderer;
   RenderDevice* const rd = r->m_renderDevice;
   if (m_editorView)
   {
      rd->Clear(clearType::TARGET | clearType::ZBUFFER, 0x000D0D0D);
      if (!m_editorBackdropEdit)
         return;
   }
   else if (g_pplayer->IsVR() && !IsMirror())
   {
      rd->Clear(clearType::TARGET | clearType::ZBUFFER, 0x00000000);
#ifdef ENABLE_XR
      if (std::shared_ptr<MeshBuffer> mask = g_pplayer->m_vrDevice->GetVisibilityMask(); mask)
      {
         static constexpr Vertex3Ds pos { 0.f, 0.f, 200000.0f }; // Very high depth bias to ensure being rendered before other opaque parts (which are sorted front to back)
         rd->ResetRenderState();
         rd->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
         rd->SetRenderState(RenderState::COLORWRITEENABLE, RenderState::RS_FALSE);
         rd->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_TRUE);
         rd->SetRenderState(RenderState::ZENABLE, RenderState::RS_TRUE);
         rd->SetRenderState(RenderState::ZFUNC, RenderState::Z_ALWAYS);
         rd->m_basicShader->SetMatrix(ShaderUniform::matWorldViewProj, g_pplayer->m_vrDevice->GetVisibilityMaskProjs(), 2);
         rd->m_basicShader->SetTechnique(ShaderTechnique::vr_mask);
         rd->DrawMesh(rd->m_basicShader, false, pos, 0, mask, RenderDevice::TRIANGLELIST, 0, mask->m_ib->m_count);
         UpdateBasicShaderMatrix();
      }
#endif
      return;
   }
   else if (g_pplayer->GetInfoMode() == IF_DYNAMIC_ONLY)
   {
      rd->Clear(clearType::TARGET | clearType::ZBUFFER, 0x00000000);
      return;
   }

   const PinTable* const ptable = g_pplayer->m_ptable;
   const ViewSetupID bgSet = m_editorBackdropEdit ? BG_DESKTOP : (m_viewModeOverride != BG_INVALID ? m_viewModeOverride : ptable->GetViewMode());
   Texture* const pin = ptable->GetDecalsEnabled() ? ptable->GetImage(ptable->m_BG_image[bgSet]) : nullptr;
   rd->ResetRenderState();
   rd->SetRenderState(RenderState::CULLMODE, RenderState::CULL_CCW);
   if (pin)
   {
      rd->Clear(clearType::ZBUFFER, 0);
      rd->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
      rd->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);
      rd->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_FALSE);
      r->DrawSprite(0.f, 0.f, 1.f, 1.f, 0xFFFFFFFF, rd->m_texMan.LoadTexture(pin, false), ptable->m_ImageBackdropNightDay ? sqrtf(r->m_sceneLighting.GetGlobalEmissionScale()) : 1.0f, true);
   }
   else
   {
      const D3DCOLOR d3dcolor = COLORREF_to_D3DCOLOR(ptable->m_colorbackdrop);
      rd->Clear(clearType::TARGET | clearType::ZBUFFER, d3dcolor);
   }
}

void RenderView::DrawBulbLightBuffer()
{
   Renderer* const r = m_renderer;
   RenderDevice* const rd = r->m_renderDevice;
   RenderPass* const initial_rt = rd->GetCurrentPass();
   static int id = 0;
   id++;

   // switch to 'bloom' output buffer to collect all bulb lights
   rd->SetRenderTarget("Transmitted Light " + std::to_string(id) + " Clear", GetBloomBufferTexture(), false);
   rd->ResetRenderState();
   rd->Clear(clearType::TARGET, 0x00000000);

   // Draw bulb lights
   r->m_render_mask |= Renderer::LIGHT_BUFFER;
   rd->SetRenderTarget("Transmitted Light " + std::to_string(id), GetBloomBufferTexture(), true, true);
   rd->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE); // disable all z-tests as zbuffer is in different resolution
   for (auto renderable : g_pplayer->m_ptable->GetParts())
      if (renderable->GetItemType() == eItemLight)
         r->RenderItem(renderable, true);
   r->m_render_mask &= ~Renderer::LIGHT_BUFFER;

   bool hasLight = rd->GetCurrentPass()->GetCommandCount() > 0;
   if (hasLight)
   { // Only apply blur if we have actually rendered some lights
      RenderPass* renderPass = rd->GetCurrentPass();
      rd->DrawGaussianBlur(GetBloomBufferTexture(), GetBloomTmpBufferTexture(), GetBloomBufferTexture(), 19.f); // FIXME kernel size should depend on buffer resolution
      RenderPass* const blurPass2 = rd->GetCurrentPass();
      RenderPass* const blurPass1 = blurPass2->m_dependencies[0];
      constexpr float margin = 0.05f; // margin for the blur
      blurPass1->m_areaOfInterest.x = renderPass->m_areaOfInterest.x - margin;
      blurPass1->m_areaOfInterest.y = renderPass->m_areaOfInterest.y - margin;
      blurPass1->m_areaOfInterest.z = renderPass->m_areaOfInterest.z + margin;
      blurPass1->m_areaOfInterest.w = renderPass->m_areaOfInterest.w + margin;
      blurPass2->m_areaOfInterest = blurPass1->m_areaOfInterest;
   }

   // Restore state and render target
   rd->SetRenderTarget(initial_rt->m_name, initial_rt->m_rt);
   initial_rt->MarkInterrupted();

   if (hasLight)
   {
      // Declare dependency on Bulb Light buffer (actually rendered to the bloom buffer texture)
      rd->AddRenderTargetDependency(GetBloomBufferTexture());
      rd->m_basicShader->SetTexture(ShaderUniform::tex_base_transmission, GetBloomBufferTexture()->GetColorSampler());
   }
   else
   {
      rd->m_basicShader->SetTextureNull(ShaderUniform::tex_base_transmission);
   }
}


#pragma region PostProcess

void RenderView::UpdateAmbientOcclusion(RenderTarget* renderedRT, unsigned int jitterIndex)
{
   Renderer* const r = m_renderer;
   RenderDevice* const rd = r->m_renderDevice;
   rd->ResetRenderState();
   rd->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_FALSE);
   rd->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
   rd->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
   rd->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);

   rd->SetRenderTarget("ScreenSpace AO"s, GetAORenderTarget(0), false);
   rd->AddRenderTargetDependency(GetAORenderTarget(1));
   rd->m_FBShader->SetTexture(ShaderUniform::tex_fb_filtered, GetAORenderTarget(1)->GetColorSampler());
   rd->AddRenderTargetDependency(renderedRT, true);
   rd->m_FBShader->SetTexture(ShaderUniform::tex_depth, renderedRT->GetDepthSampler());
   rd->m_FBShader->SetTexture(ShaderUniform::tex_ao_dither, r->m_aoDitherSampler);
   rd->m_FBShader->SetVector(ShaderUniform::w_h_height, (float)(1.0 / GetAORenderTarget(1)->GetWidth()), (float)(1.0 / GetAORenderTarget(1)->GetHeight()),
      radical_inverse(jitterIndex) * (float)(1. / 8.0),
      /*sobol*/ radical_inverse<3>(jitterIndex) * (float)(1. / 8.0)); // jitter within (64/8)x(64/8) neighborhood of 64x64 tex, good compromise between blotches and noise
   rd->m_FBShader->SetVector(ShaderUniform::AO_scale_timeblur, r->m_table->m_AOScale, 0.4f, 0.f,
      0.f); //!! 0.4f: fake global option in video pref? or time dependent? //!! commonly used is 0.1, but would require to clear history for moving stuff
   // The AO kernel works in view space (linearized depth) so the inverse projection is needed
   Matrix3D matProjInv[2];
   const int nEyes = rd->m_nEyes;
   for (int eye = 0; eye < nEyes; eye++)
   {
      matProjInv[eye] = GetMVP().GetFlippedProj(eye);
      matProjInv[eye].Invert();
   }
   rd->m_FBShader->SetMatrix(ShaderUniform::matProjInv, &matProjInv[0], nEyes);
   rd->m_FBShader->SetTechnique(ShaderTechnique::AO);
   rd->DrawFullscreenTexturedQuad(rd->m_FBShader);

   // flip AO buffers (avoids copy)
   SwapAORenderTargets();

   if (g_pplayer->GetProfilingMode() == PF_ENABLED)
      r->m_gpu_profiler.Timestamp(GTS_AO);
}

void RenderView::UpdateBloom(RenderTarget* renderedRT)
{
   Renderer* const r = m_renderer;
   RenderDevice* const rd = r->m_renderDevice;
   if (!r->IsBloomEnabled())
      return;

   rd->ResetRenderState();
   rd->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_FALSE);
   rd->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
   rd->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
   rd->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);

   const double w = static_cast<double>(renderedRT->GetWidth());
   const double h = static_cast<double>(renderedRT->GetHeight());
   const Vertex3D_TexelOnly shiftedVerts[4]
      = { { 1.0f, 1.0f, 0.0f, 1.0f + (float)(2.25 / w), 0.0f + (float)(2.25 / h) }, { -1.0f, 1.0f, 0.0f, 0.0f + (float)(2.25 / w), 0.0f + (float)(2.25 / h) },
           { 1.0f, -1.0f, 0.0f, 1.0f + (float)(2.25 / w), 1.0f + (float)(2.25 / h) }, { -1.0f, -1.0f, 0.0f, 0.0f + (float)(2.25 / w), 1.0f + (float)(2.25 / h) } };
   {
      rd->m_FBShader->SetTextureNull(ShaderUniform::tex_fb_filtered);

      // switch to 'bloom' output buffer to collect clipped framebuffer values
      rd->SetRenderTarget("Bloom Cut Off"s, GetBloomBufferTexture(), false);
      rd->AddRenderTargetDependency(renderedRT);

      rd->m_FBShader->SetTexture(ShaderUniform::tex_fb_filtered, renderedRT->GetColorSampler());
      rd->m_FBShader->SetVector(ShaderUniform::w_h_height, (float)(1.0 / w), (float)(1.0 / h), r->m_table->m_bloom_strength, 1.0f);
      rd->m_FBShader->SetTechnique(ShaderTechnique::fb_bloom);

      rd->DrawTexturedQuad(rd->m_FBShader, shiftedVerts);
   }

   rd->DrawGaussianBlur(GetBloomBufferTexture(), GetBloomTmpBufferTexture(), GetBloomBufferTexture(), 39.f); // FIXME kernel size should depend on buffer resolution

   if (g_pplayer->GetProfilingMode() == PF_ENABLED)
      r->m_gpu_profiler.Timestamp(GTS_Bloom);
}

RenderTarget* RenderView::ApplyAdditiveScreenSpaceReflection(RenderTarget* renderedRT)
{
   Renderer* const r = m_renderer;
   RenderDevice* const rd = r->m_renderDevice;
   if (!r->m_ss_refl || !r->m_table->m_enableSSR || !rd->DepthBufferReadBackAvailable() || r->m_table->m_SSRScale <= 0.f)
      return renderedRT;

   rd->ResetRenderState();
   rd->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_FALSE);
   rd->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
   rd->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
   rd->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);

   RenderTarget* outputRT = GetReflectionBufferTexture();
   rd->SetRenderTarget("ScreenSpace Reflection"s, outputRT, true);
   rd->AddRenderTargetDependency(GetBackBufferTexture(), true);
   rd->m_FBShader->SetTexture(ShaderUniform::tex_depth, GetBackBufferTexture()->GetDepthSampler());
   rd->AddRenderTargetDependency(renderedRT);
   rd->m_FBShader->SetTexture(ShaderUniform::tex_fb_filtered, renderedRT->GetColorSampler());
   rd->m_FBShader->SetTexture(ShaderUniform::tex_fb_unfiltered, renderedRT->GetColorSampler());
   rd->m_FBShader->SetTexture(ShaderUniform::tex_ao_dither, r->m_aoDitherSampler);
   // FIXME check if size should not be taken from renderdevice to account for stereo (double width/height) or supersampling
   rd->m_FBShader->SetVector(ShaderUniform::w_h_height, (float)(1.0 / renderedRT->GetWidth()), (float)(1.0 / renderedRT->GetHeight()), 1.0f /*radical_inverse(m_overall_frames%2048)*/, 1.0f);
   int outW, outH;
   GetOutputAreaSize(outW, outH);
   const float rotation = r->m_table->GetViewSetup().GetRotation(r->m_stereo3D, outW, outH);
   rd->m_FBShader->SetVector(ShaderUniform::SSR_bumpHeight_fresnelRefl_scale_FS, 0.3f, 0.3f, r->m_table->m_SSRScale, rotation);
   rd->m_FBShader->SetTechnique(ShaderTechnique::SSReflection);
   rd->DrawFullscreenTexturedQuad(rd->m_FBShader);
   if (g_pplayer->GetProfilingMode() == PF_ENABLED)
      r->m_gpu_profiler.Timestamp(GTS_SSR);
   return outputRT;
}


void RenderView::SetupTonemapping(RenderTarget* renderedRT, RenderTarget* tonemapRT, bool isFullTonemap)
{
   Renderer* const r = m_renderer;
   RenderDevice* const rd = r->m_renderDevice;
   //const unsigned int jittertime = (unsigned int)((uint64_t)msec()*90/1000);
   //const float jitter = (float)((msec() & 2047) / 1000.0);
   const float jitter = (float)(radical_inverse(g_pplayer->m_overall_frames % 2048) / 1000.0); // Deterministic jitter to ensure stable render for regression tests
   const bool useAO = r->GetAOMode() == 2;

   // switch to output buffer (main output frame buffer, or a temporary one for postprocessing)
   RenderTarget* outputRT = tonemapRT;
   assert(outputRT != renderedRT);
   // A partial area output keeps the existing content and is serialized with the other views sharing the target
   rd->SetRenderTarget("Tonemap/Dither/ColorGrade"s, outputRT, !isFullTonemap || (outputRT == GetOutput() && !IsFullOutputArea()));

   rd->ResetRenderState();
   rd->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_FALSE);
   rd->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
   rd->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
   rd->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);

   int render_w = renderedRT->GetWidth(), render_h = renderedRT->GetHeight();
   rd->AddRenderTargetDependency(renderedRT);
   rd->m_FBShader->SetTexture(ShaderUniform::tex_fb_unfiltered, renderedRT->GetColorSampler());
   rd->m_FBShader->SetTexture(ShaderUniform::tex_fb_filtered, renderedRT->GetColorSampler());
   rd->AddRenderTargetDependency(GetBackBufferTexture(), true);
   rd->m_FBShader->SetTexture(ShaderUniform::tex_depth, GetBackBufferTexture()->GetDepthSampler());

   if (r->m_table->m_bloom_strength > 0.0f && !r->m_bloomOff)
   {
      rd->AddRenderTargetDependency(GetBloomBufferTexture());
      rd->m_FBShader->SetTexture(ShaderUniform::tex_bloom, GetBloomBufferTexture()->GetColorSampler());
   }

   if (useAO)
   {
      rd->m_FBShader->SetTexture(ShaderUniform::tex_ao, GetAORenderTarget(1)->GetColorSampler());
      rd->AddRenderTargetDependency(GetAORenderTarget(1));
   }

   // For information mode, override with the wanted render target instead of the render buffer
   const InfoMode infoMode = g_pplayer->GetInfoMode();
   if (infoMode == IF_RENDER_PROBES)
   {
      RenderProbe* render_probe = r->m_table->m_vrenderprobe[g_pplayer->m_infoProbeIndex];
      RenderTarget* probe = render_probe->Render(0);
      if (probe)
      {
         rd->AddRenderTargetDependency(probe);
         rd->m_FBShader->SetTexture(ShaderUniform::tex_fb_unfiltered, probe->GetColorSampler());
         rd->m_FBShader->SetTexture(ShaderUniform::tex_fb_filtered, probe->GetColorSampler());
         render_w = probe->GetWidth();
         render_h = probe->GetHeight();
      }
   }
   else if (infoMode == IF_LIGHT_BUFFER_ONLY)
   {
      renderedRT = GetBloomBufferTexture();
      rd->AddRenderTargetDependency(renderedRT);
      rd->m_FBShader->SetTexture(ShaderUniform::tex_fb_unfiltered, renderedRT->GetColorSampler());
      rd->m_FBShader->SetTexture(ShaderUniform::tex_fb_filtered, renderedRT->GetColorSampler());
      render_w = renderedRT->GetWidth();
      render_h = renderedRT->GetHeight();
   }

   // The wide color gamut setup applies to the window being rendered to: the desktop display for the VR desktop scene, the playfield window otherwise
   VPX::Window* const outputWnd = m_outputWnd;
   const bool isHdr2020 = outputWnd != nullptr && (g_pplayer->m_vrDevice == nullptr || IsMirror()) && outputWnd->IsWCGBackBuffer();
   if (isHdr2020)
   {
      const float maxDisplayLuminance = outputWnd->GetHDRHeadRoom()
         * (outputWnd->GetSDRWhitePoint() * 80.f); // Maximum luminance of display in nits, note that GetSDRWhitePoint()*80 should usually be in the 200 nits range
      rd->m_FBShader->SetVector(ShaderUniform::exposure_wcg, r->m_exposure,
         (outputWnd->GetSDRWhitePoint() * 80.f)
            / maxDisplayLuminance, // Apply SDR whitepoint (1.0 -> white point in nits), then scale down by maximum luminance (in nits) of display to get a relative value before tonemapping, equal to 1/GetHDRHeadRoom()
         maxDisplayLuminance / 10000.f, // Apply back maximum luminance in nits of display after tonemapping, scaled down to PQ limits (1.0 is 10000 nits)
         1.f);

      float spline_params[6];
      PrecompSplineTonemap(maxDisplayLuminance, spline_params);
      rd->m_FBShader->SetVector(ShaderUniform::spline1, spline_params[0], spline_params[1], spline_params[2], spline_params[3]);
      rd->m_FBShader->SetVector(ShaderUniform::spline2, spline_params[4], spline_params[5], 0.f, 0.f);
   }
   else
   {
      // VR device expects linear RGB value (for linear layer composition)
      rd->m_FBShader->SetVector(ShaderUniform::exposure_wcg, r->m_exposure, 1.f, /*100.f*/ /*203.f*/ 350.f / 10000.f,
         (g_pplayer->m_vrDevice && !IsMirror())
            ? 2.f
            : 0.f); //!! 203 nits as SDR reference? //!! or 100 as in BT2446 spec? // but both result in too dark images for BT2446 conversion at least compared to the other mappers

      // dummy values only, unused at the moment
      //m_renderDevice->m_FBShader->SetVector(ShaderUniform::spline1, 0.f,0.f,0.f,0.f);
      //m_renderDevice->m_FBShader->SetVector(ShaderUniform::spline2, 0.f,0.f,0.f,0.f);
   }

   float agx_params[4];
   PrecompAgXTonemap(agx_params);
   rd->m_FBShader->SetVector(ShaderUniform::agx_params, agx_params[0], agx_params[1], agx_params[2], agx_params[3]);

   Texture* const pin = r->m_table->GetImage(r->m_table->m_imageColorGrade);
   if (pin)
      // FIXME ensure that we always honor the linear RGB. Here it can be defeated if texture is used for something else (which is very unlikely)
      rd->m_FBShader->SetTexture(ShaderUniform::tex_color_lut, pin, true, SamplerFilter::SF_BILINEAR, SamplerAddressMode::SA_CLAMP, SamplerAddressMode::SA_CLAMP);
   rd->m_FBShader->SetVector(ShaderUniform::bloom_dither_colorgrade,
      r->IsBloomEnabled() ? 1.f : 0.f, // Bloom
      (!isHdr2020 && (outputRT->GetColorFormat() != colorFormat::RGBA10)) ? 1.f : 0.f, // Dither
      (pin != nullptr) ? 1.f : 0.f, /* LUT colorgrade */
      0.f);
   if (r->IsBloomEnabled())
      rd->AddRenderTargetDependency(GetBloomBufferTexture());
   rd->m_FBShader->SetVector(ShaderUniform::w_h_height, (float)(1.0 / (double)render_w), (float)(1.0 / (double)render_h),
      jitter, // radical_inverse(jittertime) * 11.0f,
      jitter); // sobol(jittertime) * 13.0f); // jitter for dither pattern

   ShaderTechnique tonemapTechnique;
   // Filter only when resampling, i.e. resolving a supersampled render (bilinear is a 2x2 box filter at the 200% maximum AA factor, larger factors would need
   // a real downsampling pass). An undersampled render is tonemapped 1:1, then upscaled (see ApplyUpscaling)
   const bool resample = (render_w != outputRT->GetWidth()) || (render_h != outputRT->GetHeight());
   const bool filtered = resample || (m_screenOffset.x != 0.f) || (m_screenOffset.y != 0.f);
   if (infoMode == IF_AO_ONLY)
      tonemapTechnique = ShaderTechnique::fb_AO;
   else if (infoMode == IF_RENDER_PROBES)
      tonemapTechnique = r->m_toneMapper == TM_REINHARD ? ShaderTechnique::fb_rhtonemap
         : r->m_toneMapper == TM_FILMIC                 ? ShaderTechnique::fb_fmtonemap
         : r->m_toneMapper == TM_NEUTRAL                ? ShaderTechnique::fb_nttonemap
         : r->m_toneMapper == TM_AGX                    ? ShaderTechnique::fb_agxtonemap
                                                        : /*m_toneMapper == TM_WCG_SPLINE ?*/ ShaderTechnique::fb_wcgtonemap;
   else if (isHdr2020 && r->m_HDRforceDisableToneMapper)
      tonemapTechnique = useAO ? filtered ? ShaderTechnique::fb_wcgtonemap_AO : ShaderTechnique::fb_wcgtonemap_AO_no_filter
         : filtered            ? ShaderTechnique::fb_wcgtonemap
                               : ShaderTechnique::fb_wcgtonemap_no_filter;
   else if (r->m_toneMapper == TM_REINHARD)
      tonemapTechnique = useAO ? filtered ? ShaderTechnique::fb_rhtonemap_AO : ShaderTechnique::fb_rhtonemap_AO_no_filter
         : filtered            ? ShaderTechnique::fb_rhtonemap
                               : ShaderTechnique::fb_rhtonemap_no_filter;
   else if (r->m_toneMapper == TM_FILMIC)
      tonemapTechnique = useAO ? filtered ? ShaderTechnique::fb_fmtonemap_AO : ShaderTechnique::fb_fmtonemap_AO_no_filter
         : filtered            ? ShaderTechnique::fb_fmtonemap
                               : ShaderTechnique::fb_fmtonemap_no_filter;
   else if (r->m_toneMapper == TM_NEUTRAL)
      tonemapTechnique = useAO ? filtered ? ShaderTechnique::fb_nttonemap_AO : ShaderTechnique::fb_nttonemap_AO_no_filter
         : filtered            ? ShaderTechnique::fb_nttonemap
                               : ShaderTechnique::fb_nttonemap_no_filter;
   else if (r->m_toneMapper == TM_AGX)
      tonemapTechnique = useAO ? filtered ? ShaderTechnique::fb_agxtonemap_AO : ShaderTechnique::fb_agxtonemap_AO_no_filter
         : filtered            ? ShaderTechnique::fb_agxtonemap
                               : ShaderTechnique::fb_agxtonemap_no_filter;
   else
      assert(!"unknown tonemapper");
   rd->m_FBShader->SetTechnique(tonemapTechnique);
}

RenderTarget* RenderView::ApplyTonemapping(RenderTarget* renderedRT, RenderTarget* tonemapRT)
{
   Renderer* const r = m_renderer;
   RenderDevice* const rd = r->m_renderDevice;
   SetupTonemapping(renderedRT, tonemapRT, true);
   Vertex3D_TexelOnly shiftedVerts[4] = { { 1.0f + m_screenOffset.x, 1.0f + m_screenOffset.y, 0.0f, 1.0f, 0.0f }, { -1.0f + m_screenOffset.x, 1.0f + m_screenOffset.y, 0.0f, 0.0f, 0.0f },
      { 1.0f + m_screenOffset.x, -1.0f + m_screenOffset.y, 0.0f, 1.0f, 1.0f }, { -1.0f + m_screenOffset.x, -1.0f + m_screenOffset.y, 0.0f, 0.0f, 1.0f } };
   if (tonemapRT == GetOutput() && !IsFullOutputArea())
      MapToOutputArea(shiftedVerts);
   rd->DrawTexturedQuad(rd->m_FBShader, shiftedVerts);

   // Embedded ancillary windows are (potentially) separate physical displays, so the table's color grade must not
   // reach them, while everything else this pass does (exposure, tonemapper, bloom, dither and the
   // output colorspace, sRGB or HDR10/BT.2100) still must. Rather than masking inside the shader,
   // run the very same pass again over their regions with the grade switched off: it reads the same
   // source and blending is disabled, so it simply overwrites the graded result.
   // Skipped in the info modes, which rebind the source to a probe or the bloom buffer
   if (!r->m_embeddedRegions.empty() && (g_pplayer->GetInfoMode() == IF_NONE) && r->m_table->GetImage(r->m_table->m_imageColorGrade) != nullptr)
   {
      // The flags SetupTonemapping just set, with only the color grade cleared
      VPX::Window* const outputWnd = m_outputWnd;
      const bool isHdr2020 = outputWnd != nullptr && (g_pplayer->m_vrDevice == nullptr || IsMirror()) && outputWnd->IsWCGBackBuffer();
      rd->m_FBShader->SetVector(ShaderUniform::bloom_dither_colorgrade, //
         r->IsBloomEnabled() ? 1.f : 0.f, // Bloom
         (!isHdr2020 && (GetOutput()->GetColorFormat() != colorFormat::RGBA10)) ? 1.f : 0.f, // Dither
         0.f, // No LUT colorgrade
         0.f);
      for (const vec4& region : r->m_embeddedRegions)
      {
         Vertex3D_TexelOnly regionVerts[4] = { { 2.f * region.z - 1.f + m_screenOffset.x, 1.f - 2.f * region.y + m_screenOffset.y, 0.0f, region.z, region.y },
            { 2.f * region.x - 1.f + m_screenOffset.x, 1.f - 2.f * region.y + m_screenOffset.y, 0.0f, region.x, region.y },
            { 2.f * region.z - 1.f + m_screenOffset.x, 1.f - 2.f * region.w + m_screenOffset.y, 0.0f, region.z, region.w },
            { 2.f * region.x - 1.f + m_screenOffset.x, 1.f - 2.f * region.w + m_screenOffset.y, 0.0f, region.x, region.w } };
         if (tonemapRT == GetOutput() && !IsFullOutputArea())
            MapToOutputArea(regionVerts);
         rd->DrawTexturedQuad(rd->m_FBShader, regionVerts);
      }
   }

   return tonemapRT;
}

RenderTarget* RenderView::ApplyBallMotionBlur(RenderTarget* beforeTonemapRT, RenderTarget* afterTonemapRT)
{
#ifndef ENABLE_BGFX
   return afterTonemapRT;
#endif

   Renderer* const r = m_renderer;
   RenderDevice* const rd = r->m_renderDevice;

   // We do not support dynamic view point yet (i.e. MVP must be the same between the previous render and this one)
   if (m_disableStaticPrepass)
      return afterTonemapRT;

   // We do not support stereo mode yet
   if (r->m_stereo3D != STEREO_OFF)
      return afterTonemapRT;

   if (r->m_motionBlurOff)
      return afterTonemapRT;

   rd->ResetRenderState();
   rd->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_FALSE);
   rd->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
   rd->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
   rd->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);

   // Draw motion blur to a temporary render target
   RenderTarget* tempRT = GetMotionBlurBufferTexture(); // Use a dedicated buffer since we need HDR (RGB16F) and we can't use the existing ones (Backbuffer 1 & 2 and SSR)
   rd->SetRenderTarget("Ball Motion Blur - Compute"s, tempRT, false);
   rd->AddRenderTargetDependency(GetBackBufferTexture(), true);
   rd->m_FBShader->SetTexture(ShaderUniform::tex_depth, GetBackBufferTexture()->GetDepthSampler());
   rd->AddRenderTargetDependency(GetPreviousBackBufferTexture());
   rd->m_FBShader->SetTexture(ShaderUniform::tex_bloom, GetPreviousBackBufferTexture()->GetColorSampler());
   rd->AddRenderTargetDependency(beforeTonemapRT);
   rd->m_FBShader->SetTexture(ShaderUniform::tex_fb_filtered, beforeTonemapRT->GetColorSampler());
   Matrix3D matProj[2];
   Matrix3D matProjInv[2];
   const int nEyes = rd->m_nEyes;
   m_mvp.SetModel(Matrix3D::MatrixIdentity());
   for (int eye = 0; eye < nEyes; eye++)
   {
      matProj[eye] = GetMVP().GetFlippedProj(eye);
      matProjInv[eye] = matProj[eye];
      matProjInv[eye].Invert();
   }
   rd->m_FBShader->SetMatrix(ShaderUniform::matProjInv, &matProjInv[0], nEyes);
   rd->m_FBShader->SetMatrix(ShaderUniform::matProj, &matProj[0], nEyes);
   const float invX = static_cast<float>(1.0 / beforeTonemapRT->GetWidth());
   const float invY = static_cast<float>(1.0 / beforeTonemapRT->GetHeight());
   const float padX = invX; // Padding (in clip space, half a pixel) to account for filtering in the final copy which applies visual nudge with a non integral offset
   const float padY = invY;
   // Per ball blur draw, whose quad, sample count and ball positions are only known at the beginning of the frame (see below)
   struct BlurDraw
   {
      Ball* ball;
      ShaderState* shaderState;
      Vertex3D_TexelOnly* vertices;
      vec4 prevBall;
   };
   std::array<BlurDraw, 16> blurDraws;
   int nQuads = 0;
   const Matrix3D view = GetMVP().GetView(0);
   for (size_t i = 0; i < g_pplayer->m_vball.size() && nQuads < 16; i++)
   {
      Ball* const pball = g_pplayer->m_vball[i];
      if (!pball->m_d.m_visible || pball->m_hitBall.m_d.m_lockedInKicker)
         continue;

      // Discard stable balls or balls that have moved too much (which means the ball was likely created/moved)
      // We assume that velocity won't change before rendering (which is wrong) but extend it by a magic factor of 10
      const vec3 posl = pball->m_hitBall.m_d.m_pos + 0.5f * pball->m_hitBall.m_d.m_vel;
      const vec3 newPos = view.MultiplyVectorNoPerspective(posl);
      const vec3 delta = newPos - pball->m_lastRenderedPos;
      if (const float deltaSquared = delta.Dot(delta); deltaSquared < 0.01f || deltaSquared > 1000.f)
      {
         pball->m_lastRenderedPos = newPos;
         continue;
      }

      const Vertex3D_TexelOnly verts[4] = {};
      rd->m_FBShader->SetTechnique(ShaderTechnique::fb_motionblur);
      rd->DrawTexturedQuad(rd->m_FBShader, verts);
      RenderCommand* const blurCmd = rd->GetCurrentPass()->m_commands.back();
      blurDraws[nQuads] = { pball, blurCmd->GetShaderState(), static_cast<Vertex3D_TexelOnly*>(blurCmd->GetQuadVertices()), vec4(pball->m_lastRenderedPos, pball->GetRadius()) };
      nQuads++;
   }
   if (nQuads == 0)
      return afterTonemapRT;

   // Then copy back from temporary buffer, applying tonemap since destination buffer is the tonemapped one
   SetupTonemapping(tempRT, afterTonemapRT, false);
   const bool mapToArea = afterTonemapRT == GetOutput() && !IsFullOutputArea();
   const vec4 areaClip = GetOutputAreaClip();
   for (int i = 0; i < nQuads; i++)
   {
      const Vertex3D_TexelOnly verts[4] = {};
      rd->DrawTexturedQuad(rd->m_FBShader, verts);
      Vertex3D_TexelOnly* const copyVertices = static_cast<Vertex3D_TexelOnly*>(rd->GetCurrentPass()->m_commands.back()->GetQuadVertices());

      // Late adjustment, at the beginning of the frame: from the ball position predicted for display, set the blur and copy quads,
      // the sample count and the ball positions of the blur
      rd->AddBeginOfFrameCmd(
         [this, blur = blurDraws[i], copyVertices, view, invX, invY, padX, padY, screenOffset = m_screenOffset, mapToArea, areaClip]()
         {
            Ball* const pball = blur.ball;
            RenderTarget* const tempRT = GetMotionBlurBufferTexture();
            const vec3 posl = pball->GetPosition() + m_renderer->m_renderDevice->GetPredictedDisplayDelay() * pball->GetVelocity();
            const vec3 newPos = view.MultiplyVectorNoPerspective(posl);
            const int nEyes = m_renderer->m_renderDevice->m_nEyes;

            // Clip space bounds (xMin, xMax, yMin, yMax) of the ball at its previous and new positions
            float bounds[2][4] = { { FLT_MAX, -FLT_MAX, FLT_MAX, -FLT_MAX }, { FLT_MAX, -FLT_MAX, FLT_MAX, -FLT_MAX } };
            const vec3 ballPos[2] = { pball->m_lastRenderedPos, newPos };
            bool behindCamera = false;
            for (int k = 0; k < 2; k++)
               for (int eye = 0; eye < nEyes; eye++)
                  behindCamera |= Ball::m_ash.computeProjBounds(GetMVP().GetFlippedProj(eye), ballPos[k].x, ballPos[k].y, ballPos[k].z, pball->m_hitBall.m_d.m_radius,
                     bounds[k][0], bounds[k][1], bounds[k][2], bounds[k][3]);

            int nSamples = 2;
            if (behindCamera)
            {
               // Ball (partly) behind the camera, unusable bounds: no blur (degenerate quads draw nothing)
               std::fill_n(blur.vertices, 4, Vertex3D_TexelOnly {});
               std::fill_n(copyVertices, 4, Vertex3D_TexelOnly {});
            }
            else
            {
               // Quad bound: the smaller of the rectangles enclosing both bounds along the screen motion and along the screen axes. The
               // former follows their convex hull on long diagonal motions (where an axis aligned bound grows with the motion squared), the
               // latter wins on short ones (a square bounded at 45 degrees doubles its area). In pixels, so the axes are perpendicular on
               // screen whatever the aspect ratio
               const float sx = 0.5f * static_cast<float>(tempRT->GetWidth()), sy = 0.5f * static_cast<float>(tempRT->GetHeight()); // Clip space to pixels
               const Vertex2D prevCenter((bounds[0][0] + bounds[0][1]) * 0.5f * sx, (bounds[0][2] + bounds[0][3]) * 0.5f * sy);
               const Vertex2D newCenter((bounds[1][0] + bounds[1][1]) * 0.5f * sx, (bounds[1][2] + bounds[1][3]) * 0.5f * sy);
               const Vertex2D motion = newCenter - prevCenter;
               const float motionLength = motion.Length();
               // Rectangle along the axes (u, v), padded by the clip space padding box projected on each axis
               struct Rect
               {
                  Vertex2D u, v;
                  float aMin, aMax, bMin, bMax, padU, padV;
                  float Area() const { return (aMax - aMin + 2.f * padU) * (bMax - bMin + 2.f * padV); }
               };
               const auto enclose = [&bounds, sx, sy, padX, padY](const Vertex2D& u)
               {
                  Rect r { u, Vertex2D(-u.y, u.x), FLT_MAX, -FLT_MAX, FLT_MAX, -FLT_MAX, 0.f, 0.f };
                  for (int k = 0; k < 2; k++)
                     for (int c = 0; c < 4; c++)
                     {
                        const Vertex2D corner(bounds[k][c & 1] * sx, bounds[k][2 + (c >> 1)] * sy);
                        const float a = corner.Dot(r.u), b = corner.Dot(r.v);
                        r.aMin = min(r.aMin, a);
                        r.aMax = max(r.aMax, a);
                        r.bMin = min(r.bMin, b);
                        r.bMax = max(r.bMax, b);
                     }
                  r.padU = padX * sx * fabsf(r.u.x) + padY * sy * fabsf(r.u.y);
                  r.padV = padX * sx * fabsf(r.v.x) + padY * sy * fabsf(r.v.y);
                  return r;
               };
               Rect rect = enclose(Vertex2D(1.f, 0.f));
               if (motionLength >= 1.f)
                  if (const Rect alongMotion = enclose(motion / motionLength); alongMotion.Area() < rect.Area())
                     rect = alongMotion;
               const Vertex2D& u = rect.u;
               const Vertex2D& v = rect.v;
               const float aMin = rect.aMin, aMax = rect.aMax, bMin = rect.bMin, bMax = rect.bMax, padU = rect.padU, padV = rect.padV;
               // Same corner order as an axis aligned quad (u = X, v = Y): (max, max), (min, max), (max, min), (min, min)
               static constexpr float signU[4] = { 1.f, -1.f, 1.f, -1.f };
               static constexpr float signV[4] = { 1.f, 1.f, -1.f, -1.f };
               for (int c = 0; c < 4; c++)
               {
                  const float a = signU[c] > 0.f ? aMax : aMin;
                  const float b = signV[c] > 0.f ? bMax : bMin;
                  // Blur, in the temporary buffer: padded
                  const Vertex2D blurPx = u * (a + signU[c] * padU) + v * (b + signV[c] * padV);
                  const float bx = blurPx.x / sx, by = blurPx.y / sy;
                  blur.vertices[c] = { bx, by, 0.f, 0.5f + bx * 0.5f, 0.5f - by * 0.5f };
                  // Copy, unpadded: from the temporary buffer (no screen offset) to the tonemapped buffer (screen offset applied)
                  const Vertex2D copyPx = u * a + v * b;
                  const float cx = copyPx.x / sx, cy = copyPx.y / sy;
                  Vertex3D_TexelOnly& copy = copyVertices[c];
                  copy.tu = 0.5f + cx * 0.5f;
                  copy.tv = 0.5f - cy * 0.5f;
                  copy.x = cx + screenOffset.x;
                  copy.y = cy + screenOffset.y;
                  copy.z = 0.f;
                  // Map from view clip space to the view's area of a shared output
                  if (mapToArea)
                  {
                     copy.x = areaClip.x + (copy.x + 1.f) * 0.5f * (areaClip.z - areaClip.x);
                     copy.y = areaClip.y + (copy.y + 1.f) * 0.5f * (areaClip.w - areaClip.y);
                  }
               }
               nSamples = clamp(static_cast<int>(motionLength), 2, 32); // About one sample per pixel of screen motion
            }
            vec4 whHeightWithNSamples(invX, invY, 0.f, static_cast<float>(nSamples));
            blur.shaderState->SetVector(ShaderUniform::w_h_height, &whHeightWithNSamples);

            pball->m_lastRenderedPos = newPos;
            // Local, as a discarded frame drops this command without running it
            vec4 balls[MAX_BALL_SHADOW] = {};
            balls[0] = vec4(newPos, pball->GetRadius());
            balls[1] = blur.prevBall;
            blur.shaderState->SetVector(ShaderUniform::balls, balls, MAX_BALL_SHADOW);
         });
   }

   return afterTonemapRT;
}

RenderTarget* RenderView::ApplyPostProcessedAntialiasing(RenderTarget* renderedRT, RenderTarget* outputBackBuffer)
{
   Renderer* const r = m_renderer;
   RenderDevice* const rd = r->m_renderDevice;
   const bool SMAA = r->m_FXAA == Quality_SMAA && r->m_stereo3D == STEREO_OFF;
   const bool DLAA = r->m_FXAA == Standard_DLAA;
   const bool NFAA = r->m_FXAA == Fast_NFAA;
   const bool FXAA1 = r->m_FXAA == Fast_FXAA;
   const bool FXAA2 = r->m_FXAA == Standard_FXAA;
   const bool FXAA3 = r->m_FXAA == Quality_FXAA;
   const bool FAAA = r->m_FXAA == Quality_FAAA;

   if (r->m_FXAA == Quality_SMAA && r->m_stereo3D != STEREO_OFF)
   {
      static unsigned int notifId = 0;
      notifId = g_pplayer->m_liveUI->PushNotification("SMAA is not supported in stereo modes", 5000, notifId);
   }

   rd->ResetRenderState();
   rd->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_FALSE);
   rd->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
   rd->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
   rd->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);

   if (NFAA || FXAA1 || FXAA2 || FXAA3 || FAAA)
   {
      assert(renderedRT == GetPostProcessRenderTarget1());
      RenderTarget* outputRT = outputBackBuffer ? outputBackBuffer : GetPostProcessRenderTarget(renderedRT);
      assert(outputRT != renderedRT);
      rd->SetRenderTarget("Post Process AA Pass"s, outputRT, outputRT == GetOutput() && !IsFullOutputArea());
      rd->AddRenderTargetDependency(renderedRT);
      rd->m_FBShader->SetTexture(ShaderUniform::tex_fb_filtered, renderedRT->GetColorSampler());
      rd->m_FBShader->SetTexture(ShaderUniform::tex_fb_unfiltered, renderedRT->GetColorSampler());
      rd->AddRenderTargetDependency(GetBackBufferTexture(), true); // Depth is always taken from the MSAA resolved render buffer
      // Depth cannot be used as an early-out for VR since the hidden area mesh writes depth 0 in the render buffer
      rd->m_FBShader->SetVector(
         ShaderUniform::w_h_height, (float)(1.0 / renderedRT->GetWidth()), (float)(1.0 / renderedRT->GetHeight()), (float)renderedRT->GetWidth(), r->m_stereo3D == STEREO_VR ? 0.f : 1.f);
      rd->m_FBShader->SetTechnique(NFAA ? ShaderTechnique::NFAA : FXAA3 ? ShaderTechnique::FXAA3 : FXAA2 ? ShaderTechnique::FXAA2 : FXAA1 ? ShaderTechnique::FXAA1 : ShaderTechnique::FAAA);
      DrawOutputQuad(rd->m_FBShader, outputRT);
      return outputRT;
   }
   else if (DLAA)
   {
      assert(renderedRT == GetPostProcessRenderTarget1());
      // First pass detect edges and write it to alpha channel (keeping RGB)
      RenderTarget* outputRT = GetPostProcessRenderTarget(renderedRT);
      assert(outputRT != renderedRT);
      rd->SetRenderTarget("DLAA Edge Detection"s, outputRT, false);
      rd->AddRenderTargetDependency(renderedRT);
      rd->m_FBShader->SetTexture(ShaderUniform::tex_fb_filtered, renderedRT->GetColorSampler());
      rd->m_FBShader->SetTexture(ShaderUniform::tex_fb_unfiltered, renderedRT->GetColorSampler());
      rd->AddRenderTargetDependency(GetBackBufferTexture(), true); // Depth is always taken from the MSAA resolved render buffer
      // Depth cannot be used as an early-out for VR since the hidden area mesh writes depth 0 in the render buffer
      rd->m_FBShader->SetVector(
         ShaderUniform::w_h_height, (float)(1.0 / renderedRT->GetWidth()), (float)(1.0 / renderedRT->GetHeight()), (float)renderedRT->GetWidth(), r->m_stereo3D == STEREO_VR ? 0.f : 1.f);
      rd->m_FBShader->SetTechnique(ShaderTechnique::DLAA_edge);
      rd->DrawFullscreenTexturedQuad(rd->m_FBShader);
      renderedRT = outputRT;

      // Second pass: use edge detection from first pass (alpha channel) and RGB colors for actual filtering
      outputRT = outputBackBuffer ? outputBackBuffer : GetPostProcessRenderTarget(renderedRT);
      assert(outputRT != renderedRT);
      rd->SetRenderTarget("DLAA Neigborhood blending"s, outputRT, outputRT == GetOutput() && !IsFullOutputArea());
      rd->AddRenderTargetDependency(renderedRT);
      rd->m_FBShader->SetTexture(ShaderUniform::tex_fb_filtered, renderedRT->GetColorSampler());
      rd->m_FBShader->SetTexture(ShaderUniform::tex_fb_unfiltered, renderedRT->GetColorSampler());
      rd->m_FBShader->SetTechnique(ShaderTechnique::DLAA);
      // Depth cannot be used as an early-out for VR since the hidden area mesh writes depth 0 in the render buffer
      rd->m_FBShader->SetVector(
         ShaderUniform::w_h_height, (float)(1.0 / renderedRT->GetWidth()), (float)(1.0 / renderedRT->GetHeight()), (float)renderedRT->GetWidth(), r->m_stereo3D == STEREO_VR ? 0.f : 1.f);
      DrawOutputQuad(rd->m_FBShader, outputRT);
      return outputRT;
   }
#ifndef __OPENGLES__
   else if (SMAA)
   {
      assert(renderedRT == GetPostProcessRenderTarget1());
      // SMAA uses 3 passes, all reading the tonemapped render from post process RT 1, so post process RT 3 and 2 hold the intermediate results
      RenderTarget* sourceRT = renderedRT;
      rd->m_FBShader->SetTexture(ShaderUniform::tex_fb_filtered, sourceRT->GetColorSampler());
      rd->m_FBShader->SetTexture(ShaderUniform::tex_fb_unfiltered, sourceRT->GetColorSampler());
      rd->m_FBShader->SetVector(
         ShaderUniform::w_h_height, (float)(1.0 / sourceRT->GetWidth()), (float)(1.0 / sourceRT->GetHeight()), (float)sourceRT->GetWidth(), (float)sourceRT->GetHeight());

      RenderTarget* outputRT = GetPostProcessRenderTarget3();
      assert(outputRT != renderedRT);
      rd->SetRenderTarget("SMAA Color/Edge Detection"s, outputRT, false);
      rd->AddRenderTargetDependency(sourceRT); // PostProcess RT 1
      rd->Clear(clearType::TARGET, 0x00000000); // Needed since shader uses discard
      rd->m_FBShader->SetTechnique(ShaderTechnique::SMAA_ColorEdgeDetection);
      rd->DrawFullscreenTexturedQuad(rd->m_FBShader);
      renderedRT = outputRT;

      outputRT = GetPostProcessRenderTarget(sourceRT);
      assert(outputRT != renderedRT);
      rd->SetRenderTarget("SMAA Blend weight calculation"s, outputRT, false);
      rd->AddRenderTargetDependency(sourceRT); // PostProcess RT 1
      rd->AddRenderTargetDependency(renderedRT); // BackBuffer RT
      rd->m_FBShader->SetTechnique(ShaderTechnique::SMAA_BlendWeightCalculation);
      rd->m_FBShader->SetTexture(ShaderUniform::edgesTex, renderedRT->GetColorSampler());
      rd->DrawFullscreenTexturedQuad(rd->m_FBShader);
      renderedRT = outputRT;

      outputRT = outputBackBuffer ? outputBackBuffer : GetPostProcessRenderTarget3();
      assert(outputRT != renderedRT);
      rd->SetRenderTarget("SMAA Neigborhood blending"s, outputRT, outputRT == GetOutput() && !IsFullOutputArea());
      rd->AddRenderTargetDependency(sourceRT); // PostProcess RT 1
      rd->AddRenderTargetDependency(renderedRT); // PostProcess RT 2
      rd->m_FBShader->SetTechnique(ShaderTechnique::SMAA_NeighborhoodBlending);
      rd->m_FBShader->SetTexture(ShaderUniform::blendTex, renderedRT->GetColorSampler());
      DrawOutputQuad(rd->m_FBShader, outputRT);
      return outputRT;
   }
#endif

   return renderedRT;
}

RenderTarget* RenderView::ApplySharpening(RenderTarget* renderedRT, RenderTarget* outputBackBuffer)
{
   Renderer* const r = m_renderer;
   RenderDevice* const rd = r->m_renderDevice;
   if (!r->m_sharpen)
      return renderedRT;

   rd->ResetRenderState();
   rd->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_FALSE);
   rd->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
   rd->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
   rd->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);

   assert(renderedRT != outputBackBuffer); // At this point, renderedRT may be PP1, PP2 or backbuffer
   RenderTarget* outputRT = outputBackBuffer ? outputBackBuffer : GetPostProcessRenderTarget(renderedRT);
   assert(outputRT != renderedRT);
   rd->SetRenderTarget("Sharpen"s, outputRT, outputRT == GetOutput() && !IsFullOutputArea());
   rd->AddRenderTargetDependency(renderedRT);
   rd->AddRenderTargetDependency(GetBackBufferTexture(), true); // Depth is always taken from the MSAA resolved render buffer
   rd->m_FBShader->SetTexture(ShaderUniform::tex_fb_filtered, renderedRT->GetColorSampler());
   rd->m_FBShader->SetTexture(ShaderUniform::tex_fb_unfiltered, renderedRT->GetColorSampler());
   // Depth cannot be used as an early-out for VR since the hidden area mesh writes depth 0 in the render buffer
   rd->m_FBShader->SetVector(
      ShaderUniform::w_h_height, (float)(1.0 / renderedRT->GetWidth()), (float)(1.0 / renderedRT->GetHeight()), (float)renderedRT->GetWidth(), r->m_stereo3D == STEREO_VR ? 0.f : 1.f);
   rd->m_FBShader->SetTechnique((r->m_sharpen == 1) ? ShaderTechnique::CAS : ShaderTechnique::BilateralSharp_CAS);
   DrawOutputQuad(rd->m_FBShader, outputRT);
   return outputRT;
}

RenderTarget* RenderView::ApplyUpscaling(RenderTarget* renderedRT, RenderTarget* outputBackBuffer)
{
   Renderer* const r = m_renderer;
   RenderDevice* const rd = r->m_renderDevice;
   // Only for an undersampled render (AA factor below 100%), a supersampled one is resolved by the tonemapping
   if (GetBackBufferTexture()->GetWidth() >= m_renderWidth)
      return renderedRT;

   // TODO add AMD FidelityFX Super Resolution (FSR) support

   rd->ResetRenderState();
   rd->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_FALSE);
   rd->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
   rd->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
   rd->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);

   assert(renderedRT != outputBackBuffer); // At this point, renderedRT may be PP1, PP2 or backbuffer
   RenderTarget* outputRT = outputBackBuffer ? outputBackBuffer : GetPostProcessRenderTarget(renderedRT);
   assert(outputRT != renderedRT);
   rd->SetRenderTarget("Upscale"s, outputRT, outputRT == GetOutput() && !IsFullOutputArea());
   rd->AddRenderTargetDependency(renderedRT);
   rd->m_FBShader->SetTexture(ShaderUniform::tex_fb_filtered, renderedRT->GetColorSampler());
   rd->m_FBShader->SetTechnique(ShaderTechnique::fb_copy);
   rd->m_FBShader->SetVector(ShaderUniform::w_h_height, (float)(1.0 / renderedRT->GetWidth()), (float)(1.0 / renderedRT->GetHeight()), 1.0f, 1.0f);
   DrawOutputQuad(rd->m_FBShader, outputRT);
   return outputRT;
}

RenderTarget* RenderView::ApplyStereo(RenderTarget* renderedRT, RenderTarget* outputBackBuffer)
{
   Renderer* const r = m_renderer;
   RenderDevice* const rd = r->m_renderDevice;

   // Always applied as the last pass
   assert(outputBackBuffer != nullptr);

   // STEREO_OFF: nothing to do
   if (r->m_stereo3D == STEREO_OFF)
      return renderedRT;

   rd->ResetRenderState();
   rd->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_FALSE);
   rd->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
   rd->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
   rd->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);

   if (r->m_stereo3D == STEREO_VR)
   {
#if defined(ENABLE_XR)
      int w = renderedRT->GetWidth();
      int h = renderedRT->GetHeight();

      assert(outputBackBuffer == rd->m_outputWnd[0]->GetBackBuffer()); // XR swapchain
      // Rendering is already directly being performed to the XR swapchain image, so nothing to do except for depth buffer
      // TODO we should directly use the swapchain depth buffer too to avoid the copy
      // FIXME this does not work:
      // - the XR swapchain back buffer is declared without depth buffer, so the depth copy does nothing
      // - BGFX does not support blitting depth to the default back buffer
      if (g_pplayer->m_vrDevice->UseDepthBuffer())
      {
         // Copy depth buffer to OpenXR swapchain's current depth target
         rd->SetRenderTarget("OpenXR-Depth"s, outputBackBuffer, true, true);
         rd->AddRenderTargetDependency(GetBackBufferTexture(), true);
         rd->BlitRenderTarget(GetBackBufferTexture(), outputBackBuffer, false, true);
      }

      // Mirror the rendered eye(s) to the desktop display (unless it shows a dedicated render of the scene, see VRMirror view)
      RenderTarget* desktopRT = nullptr;
      if (rd->m_outputWnd.size() >= 2 && !r->NeedsDesktopSceneRender())
      {
         assert(rd->m_outputWnd.size() >= 2); // For the time being, we rely on the fact that the First output is the VR Headset, and the second is the desktop display OS window
         desktopRT = rd->m_outputWnd[1]->GetBackBuffer();
         rd->SetRenderTarget("Desktop Display"s, desktopRT, false, true);

         rd->AddRenderTargetDependency(renderedRT);
         const int desktopW = r->m_vrDesktop == VRDesktopMode::Both ? desktopRT->GetWidth() / 2 : desktopRT->GetWidth(), desktopH = desktopRT->GetHeight();
         const float ar = (float)w / (float)h, desktopAr = (float)desktopW / (float)desktopH;
         int x = 0, y = 0;
         int fw = w, fh = h;
         if ((r->m_vrDesktopShrink && ar < desktopAr) || (!r->m_vrDesktopShrink && ar > desktopAr))
         { // Fit on Y
            const int scaledW = (int)((float)h * desktopAr);
            x = (w - scaledW) / 2;
            fw = scaledW;
         }
         else
         { // Fit on X
            const int scaledH = (int)((float)w / desktopAr);
            y = (h - scaledH) / 2;
            fh = scaledH;
         }
         if (r->m_vrDesktopShrink || r->m_vrDesktop == VRDesktopMode::Disabled)
            rd->Clear(clearType::TARGET | clearType::ZBUFFER, 0x00000000);

         Vertex3D_TexelOnly verts[4]
            = { { -1.0f, 1.0f, 0.0f, static_cast<float>(x) / w, static_cast<float>(y) / h }, { 1.0f, 1.0f, 0.0f, static_cast<float>(x + fw) / w, static_cast<float>(y) / h },
                 { -1.0f, -1.0f, 0.0f, static_cast<float>(x) / w, static_cast<float>(y + fh) / h }, { 1.0f, -1.0f, 0.0f, static_cast<float>(x + fw) / w, static_cast<float>(y + fh) / h } };
         rd->m_FBShader->SetTechnique(ShaderTechnique::fb_mirror);
         rd->m_FBShader->SetVector(ShaderUniform::w_h_height, 1.f, 1.f, 1.f, 1.f);
         rd->m_FBShader->SetTexture(ShaderUniform::tex_fb_unfiltered, renderedRT->GetColorSampler(), SamplerFilter::SF_BILINEAR);
         if (r->m_vrDesktop == VRDesktopMode::Left || r->m_vrDesktop == VRDesktopMode::Right)
         {
            rd->m_FBShader->SetInt(ShaderUniform::layer, r->m_vrDesktop == VRDesktopMode::Left ? 0 : 1);
            rd->DrawTexturedQuad(rd->m_FBShader, verts);
         }
         else if (r->m_vrDesktop == VRDesktopMode::Both)
         {
            verts[0].x = verts[2].x = -1.f;
            verts[1].x = verts[3].x = 0.f;
            rd->m_FBShader->SetInt(ShaderUniform::layer, 0);
            rd->DrawTexturedQuad(rd->m_FBShader, verts);
            verts[0].x = verts[2].x = 0.f;
            verts[1].x = verts[3].x = 1.f;
            rd->m_FBShader->SetInt(ShaderUniform::layer, 1);
            rd->DrawTexturedQuad(rd->m_FBShader, verts);
         }

         // Composite the UI on the desktop display, as it is submitted to the headset as a dedicated composition layer
         if (RenderTarget* const uiLayer = g_pplayer->m_vrDevice->GetUIRenderTarget(); uiLayer != nullptr && r->m_vrDesktop != VRDesktopMode::Disabled)
         {
            rd->AddRenderTargetDependency(uiLayer);
            const float uiW = static_cast<float>(uiLayer->GetWidth()), uiH = static_cast<float>(uiLayer->GetHeight());
            // Same cropping as the mirrored eye texture (uv span), with position mapped through a pixel ortho transform
            const float tu1 = static_cast<float>(x) / w, tv1 = static_cast<float>(y) / h;
            const float tu2 = static_cast<float>(x + fw) / w, tv2 = static_cast<float>(y + fh) / h;
            const Matrix3D ortho = Matrix3D::MatrixOrthoOffCenterRH(0.f, uiW, uiH, 0.f, 0.f, 1.f);
            const Matrix3D orthoEyes[2] = { ortho, ortho }; // One per eye
            // TODO the UI shader takes its vertex color from the VF_IMGUI COLOR0 attribute, which these Vertex3D_NoTex2 quads (and the one
            // in RenderView::Render) lack: BGFX D3D11 reads it as R8G8B8A8_UINT from the position bytes, so the UI is garbage or invisible
            // (OpenGL only works by chance, through the (1,1,1) normal at location 1). Use a textured copy shader, or a white VF_IMGUI quad
            rd->m_uiShader->SetTechnique(ShaderTechnique::LiveUI_mono);
            rd->m_uiShader->SetMatrix(ShaderUniform::matWorldView, orthoEyes, rd->m_nEyes);
            rd->m_uiShader->SetVector(ShaderUniform::staticColor_Alpha, 0.f, 0.f, 0.f, 0.f);
            rd->m_uiShader->SetVector(ShaderUniform::clip_plane, 0.f, 0.f, uiW, uiH);
            rd->m_uiShader->SetTexture(ShaderUniform::tex_base_color, uiLayer->GetColorSampler());
            Vertex3D_NoTex2 overlay[4] = { { 0.f, 0.f, 1.f, 1.f, 1.f, 1.f, tu1, tv1 }, { uiW, 0.f, 1.f, 1.f, 1.f, 1.f, tu2, tv1 }, { 0.f, uiH, 1.f, 1.f, 1.f, 1.f, tu1, tv2 },
               { uiW, uiH, 1.f, 1.f, 1.f, 1.f, tu2, tv2 } };
            rd->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_TRUE);
            rd->SetRenderState(RenderState::SRCBLEND, RenderState::SRC_ALPHA);
            rd->SetRenderState(RenderState::DESTBLEND, RenderState::INVSRC_ALPHA);
            rd->SetRenderState(RenderState::BLENDOP, RenderState::BLENDOP_ADD);
            if (r->m_vrDesktop == VRDesktopMode::Both)
            {
               overlay[0].x = overlay[2].x = 0.f;
               overlay[1].x = overlay[3].x = uiW * 0.5f;
               rd->DrawTexturedQuad(rd->m_uiShader, overlay, true);
               overlay[0].x = overlay[2].x = uiW * 0.5f;
               overlay[1].x = overlay[3].x = uiW;
            }
            rd->DrawTexturedQuad(rd->m_uiShader, overlay, true);
            rd->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_FALSE);
         }
      }

      if (r->m_vrApplyColorKey)
      {
         // Apply a color mask for color keying. For the time being, this is the only way we have to support mixed reality
         // as HMD does not expose passthrough layers to PCVR (at least Meta Quest 3, used for development).
         // Therefore we leverage VirtualDesktop color keying feature. This needs to be performed as a post process to avoid
         // blending the color key with the rendered scene (alpha blending which would be kept, as it is not fullfilling the
         // color key after blending).
         rd->SetRenderTarget("VR ColorKeying"s, rd->GetOutputBackBuffer(), true, true);
         if (desktopRT)
            rd->AddRenderTargetDependency(desktopRT); // Add a dependency on the desktop display to ensure color keying is done after its copy
         rd->AddRenderTargetDependency(renderedRT);
         rd->SetRenderState(RenderState::ZENABLE, RenderState::RS_TRUE);
         rd->SetRenderState(RenderState::ZFUNC, RenderState::Z_LESSEQUAL);
         rd->m_FBShader->SetTexture(ShaderUniform::tex_depth, GetBackBufferTexture()->GetDepthSampler());
         rd->m_FBShader->SetTexture(ShaderUniform::tex_fb_unfiltered, renderedRT->GetColorSampler());
         rd->m_FBShader->SetTechnique(ShaderTechnique::vr_passthrough);
         rd->DrawFullscreenTexturedQuad(rd->m_FBShader);
      }
#endif

      return outputBackBuffer;
   }

   if (r->m_stereo3Denabled)
   {
      // Anaglyph and 3DTV
      // For anaglyph, defocus the "lesser" eye (the one with a darker color, which should be the non dominant eye of the player)
      if (IsAnaglyphStereoMode(r->m_stereo3D) && r->m_stereo3DDefocus != 0.f)
      {
         RenderTarget* tmpRT = GetPostProcessRenderTarget(renderedRT);
         RenderTarget* outputRT = GetPostProcessRenderTarget(tmpRT);
         rd->DrawGaussianBlur(renderedRT, tmpRT, outputRT, abs(r->m_stereo3DDefocus) * 39.f, r->m_stereo3DDefocus > 0.f ? 0 : 1);
         renderedRT = outputRT;
      }
      // Stereo composition
      RenderTarget* outputRT = outputBackBuffer;
      rd->SetRenderTarget("Stereo"s, outputRT, outputRT == GetOutput() && !IsFullOutputArea());
      rd->AddRenderTargetDependency(renderedRT);
      rd->m_stereoShader->SetTexture(ShaderUniform::tex_stereo_fb, renderedRT->GetColorSampler(),
         (renderedRT->GetWidth() == outputRT->GetWidth() && renderedRT->GetHeight() == outputRT->GetHeight()) ? SamplerFilter::SF_NONE : SamplerFilter::SF_BILINEAR);
      DrawOutputQuad(rd->m_stereoShader, outputRT);
      return outputRT;
   }

   // 3D stereo rendering, but display mono (first eye)
   // Nothing to do: last pass will just output to the backbuffer first layer (not fully clean as we are still pushing an instance draw, but driver will discard missing layers)
   return renderedRT;
}

#pragma endregion


void RenderView::Render()
{
   Renderer* const r = m_renderer;
   RenderDevice* const rd = r->m_renderDevice;

   if (GetOutput() == nullptr)
      return;

   // The mirror view stays registered when the desktop display switches back to an eye mirror (e.g. after an
   // in-place table switch): it then records nothing and the eye mirror blit in ApplyStereo owns the output
   if (IsMirror() && !r->NeedsDesktopSceneRender())
      return;

   if (!m_renderScene)
   {
      // Views may skip recording their scene (hidden editor pane, or the primary view while the editor
      // areas own the frame). Only the primary view keeps working then: it clears the shared output the
      // other views draw their areas to, and still ticks the game logic once per frame. Its LiveUI pass is
      // deferred to the end of the frame record (Renderer::Render3DScene), compositing the UI over the views
      if (m_runGameLogic)
      {
         rd->SetRenderTarget("Sceneless"s, GetOutput(), false, true);
         rd->Clear(clearType::TARGET | clearType::ZBUFFER, 0x00000000);
         g_pplayer->UpdateGameLogic();
      }
      return;
   }

   // Lazily create this view's render target chain on first use (VR desktop mirror, offscreen views)
   if (m_pOffscreenBackBufferTexture1 == nullptr)
   {
      RenderView* const primary = r->GetPrimaryView();
      GetOutputAreaSize(m_renderWidth, m_renderHeight);
      if (m_outputWnd != nullptr)
      {
         m_renderSourceWidth = m_outputWnd->GetPixelWidth();
         m_renderSourceHeight = m_outputWnd->GetPixelHeight();
      }
      else
      {
         m_renderSourceWidth = m_renderWidth;
         m_renderSourceHeight = m_renderHeight;
      }
      // The mirror view renders a single layer. Its render targets still share the primary's stereo
      // type since they are sampled by the shared shaders (which read framebuffer textures as arrays
      // when stereo is enabled), but it does not need the playfield's MSAA.
      SetupTargets(m_renderWidth, m_renderHeight, primary->GetBackBufferTexture()->m_type, primary->GetBackBufferTexture()->GetColorFormat(),
         IsMirror() ? 1 : primary->GetMSAABackBufferTexture()->GetMSAASamples());
      if (!IsMirror())
         InitLayout();
   }

   ResizeRenderTargets();

   // Keep previous render as a reflection probe for ball reflection and for hires motion blur
   SwapBackBufferRenderTargets();

   // Mirror and offscreen views may restrict rendering to a single layer of the (stereo typed) render targets
   rd->m_singleLayerRendering = m_outputLayer;

   // Setup initial MVP to setup shaders and rendering
   if (IsMirror())
   {
      // The desktop display shows the scene as a "cabinet" view of the playfield (see RenderDesktopScene)
      r->m_table->GetViewSetup().ComputeMVP(
         r->m_table, (float)GetBackBufferTexture()->GetWidth() / (float)GetBackBufferTexture()->GetHeight(), false, m_mvp, vec3(r->m_cam.x, r->m_cam.y, r->m_cam.z), r->m_inc);
      for (unsigned int eye = 0; eye < m_mvp.m_nEyes; eye++)
         m_playfieldView[eye] = m_mvp.GetView(eye);
   }
   else if (r->m_stereo3D != STEREO_VR)
   {
      m_mvp = m_initialMVP;
      for (unsigned int eye = 0; eye < m_mvp.m_nEyes; eye++)
         m_playfieldView[eye] = m_mvp.GetView(eye);
   }
   // For VR, the headset pose is updated in SetSpaceReference (through VRDevice::UpdateVRPosition)
   SetSpaceReference(PartGroupData::SpaceReference::SR_PLAYFIELD, true);

   // Per view shader state (the shared shaders are used by all views, each setting them before its own passes)
   rd->m_basicShader->SetVector(
      ShaderUniform::w_h_height, (float)(1.0 / (double)GetMSAABackBufferTexture()->GetWidth()), (float)(1.0 / (double)GetMSAABackBufferTexture()->GetHeight()), 0.0f, 0.0f);
   const vec4 ballLighting = rd->m_ballShader->GetVector(ShaderUniform::w_h_disableLighting);
   rd->m_ballShader->SetVector(ShaderUniform::w_h_disableLighting, 1.5f / (float)GetPreviousBackBufferTexture()->GetWidth(), 1.5f / (float)GetPreviousBackBufferTexture()->GetHeight(),
      ballLighting.z, ballLighting.w);

   // Update backdrop visibility and visibility mask
   // For the time being, the Render3DScene only support rendering one 3D view for main scene: dedicated 3D rendering for backglass, topper, apron are not yet implemented
   if (IsMirror())
   {
      m_noBackdrop = !m_editorBackdropEdit && ((m_viewModeOverride != BG_INVALID ? m_viewModeOverride : r->m_table->GetViewMode()) == BG_FULLSCREEN || m_editorView);
      m_visibilityMask = (m_viewModeOverride != BG_INVALID ? m_viewModeOverride : r->m_table->GetViewMode()) == BG_FULLSCREEN ? PartGroupData::PlayerModeVisibilityMask::PMVM_CABINET
         : (m_viewModeOverride != BG_INVALID ? m_viewModeOverride : r->m_table->GetViewMode()) == BG_FSS                      ? PartGroupData::PlayerModeVisibilityMask::PMVM_FSS
                                                                                                                              : PartGroupData::PlayerModeVisibilityMask::PMVM_DESKTOP;
   }
   else
   {
      m_noBackdrop = !m_editorBackdropEdit // Force backdrop rendering if its being edited
         && ((g_pplayer->m_vrDevice != nullptr) || ((m_viewModeOverride != BG_INVALID ? m_viewModeOverride : r->m_table->GetViewMode()) == BG_FULLSCREEN) || m_editorView);
      if (g_pplayer->m_vrDevice)
         m_visibilityMask = r->m_vrApplyColorKey ? PartGroupData::PlayerModeVisibilityMask::PMVM_MIXED_REALITY : PartGroupData::PlayerModeVisibilityMask::PMVM_VIRTUAL_REALITY;
      else if ((m_viewModeOverride != BG_INVALID ? m_viewModeOverride : r->m_table->GetViewMode()) == BG_FULLSCREEN)
         m_visibilityMask = PartGroupData::PlayerModeVisibilityMask::PMVM_CABINET;
      else if ((m_viewModeOverride != BG_INVALID ? m_viewModeOverride : r->m_table->GetViewMode()) == BG_FSS)
         m_visibilityMask = PartGroupData::PlayerModeVisibilityMask::PMVM_FSS;
      else if ((m_viewModeOverride != BG_INVALID ? m_viewModeOverride : r->m_table->GetViewMode()) == BG_DESKTOP)
         m_visibilityMask = PartGroupData::PlayerModeVisibilityMask::PMVM_DESKTOP;
   }

   // We don't need to set the dependency on the previous frame render as this would be a cross frame dependency which does not have any meaning since dependencies are resolved per frame
   // m_renderDevice->AddRenderTargetDependency(m_renderDevice->GetPreviousBackBufferTexture());
   rd->m_ballShader->SetTexture(ShaderUniform::tex_ball_playfield, GetPreviousBackBufferTexture()->GetColorSampler());

   // If using static prerendering, apply nudging by shaking the screen (otherwise, apply table displacement)
   if (!IsMirror() && !m_disableStaticPrepass && r->m_visualNudgeStrength > 0.0f && g_pplayer->m_playMode != Player::PlayMode::CaptureAttract)
   {
      // FIXME Screen offset is applied in clip space (-1..1, -1..1) independently of the output resolution resulting in non uniform scaling of nudge
      Vertex2D offset = r->m_visualNudgeStrength * g_pplayer->m_pininput.m_nudgeHandler->GetCabinetOffset() * 2.f;
      // The offset is in table space, but applied after the mirroring flip: mirror it back to the screen
      if (g_pplayer->m_tblMirrorEnabled)
         offset.x = -offset.x;
      SetScreenOffset(offset.x, offset.y);
   }
   else
      SetScreenOffset(0.f, 0.f);

   // Mark all probes to be re-rendered for this frame (only if needed, lazily rendered)
   for (auto probe : r->m_table->m_vrenderprobe)
      probe->MarkDirty(this);

   r->m_render_mask = Renderer::DEFAULT;

   RenderStatics();

   // While static parts are being accumulated, the static pass renders probes with static parts only: re-render them for the dynamic pass
   for (auto probe : r->m_table->m_vrenderprobe)
      probe->MarkDirty(this);

   rd->m_noMovingBalls = true;

   RenderDynamics();

   // Render live UI 3D content (editors overlays, selection, ...)
   if (m_renderOverlay3D)
      g_pplayer->m_liveUI->Render3D();

   // Keep latency low by proceeding input, ... (as next passes are not affected by the game logic)
   if (m_runGameLogic)
      g_pplayer->UpdateGameLogic();

   // Resolve MSAA buffer to a normal one (noop if not using MSAA), allowing sampling it for postprocessing
   if (GetMSAABackBufferTexture() != GetBackBufferTexture())
   {
      RenderPass* const initial_rt = rd->GetCurrentPass();
      rd->SetRenderTarget("Resolve MSAA"s, GetBackBufferTexture());
      rd->BlitRenderTarget(GetMSAABackBufferTexture(), GetBackBufferTexture(), true, true);
      rd->SetRenderTarget(initial_rt->m_name, initial_rt->m_rt);
      initial_rt->m_name += '-';
   }

   // Starts from the output of the render passes
   RenderTarget* renderedRT = GetBackBufferTexture();

   // Add screen space reflections
   renderedRT = ApplyAdditiveScreenSpaceReflection(renderedRT);

   // Clear embedded ancillary windows before updarting bloom & AO
   r->ClearEmbeddedAncillaryWindow(VPXWindowId::VPXWINDOW_Backglass, g_pplayer->m_backglassOutput, renderedRT);
   r->ClearEmbeddedAncillaryWindow(VPXWindowId::VPXWINDOW_ScoreView, g_pplayer->m_scoreViewOutput, renderedRT);
   r->ClearEmbeddedAncillaryWindow(VPXWindowId::VPXWINDOW_Topper, g_pplayer->m_topperOutput, renderedRT);

   // Compute AO contribution (to be applied later, with tonemapping)
   if (r->GetAOMode() == 2) // Only process for dynamic AO
      UpdateAmbientOcclusion(GetBackBufferTexture(), g_pplayer->m_overall_frames % 2048);

   // Compute bloom (to be applied later, with tonemapping)
   UpdateBloom(renderedRT);

   // Render ancillary windows. Embedded ones are composited into the linear render buffer so that
   // the postprocess chain below converts them along with the rest of the frame: that is what knows
   // whether the backbuffer is sRGB or HDR10/BT.2100, and what applies the scene exposure and the
   // table's tonemapper. Only the color grade has to be kept off them, which ApplyTonemapping does
   r->m_embeddedRegions.clear();
   r->RenderAncillaryWindow(VPXWindowId::VPXWINDOW_Backglass, g_pplayer->m_backglassOutput, renderedRT, g_pplayer->m_ancillaryWndRenderers[VPXWindowId::VPXWINDOW_Backglass]);
   r->RenderAncillaryWindow(VPXWindowId::VPXWINDOW_ScoreView, g_pplayer->m_scoreViewOutput, renderedRT, g_pplayer->m_ancillaryWndRenderers[VPXWindowId::VPXWINDOW_ScoreView]);
   r->RenderAncillaryWindow(VPXWindowId::VPXWINDOW_Topper, g_pplayer->m_topperOutput, renderedRT, g_pplayer->m_ancillaryWndRenderers[VPXWindowId::VPXWINDOW_Topper]);

   const bool hasAntialiasPass = m_applyPostFX && r->m_FXAA != Disabled;
   const bool hasSharpenPass = m_applyPostFX && r->m_sharpen != 0;
   const bool hasUpscalerPass = m_applyPostFX && GetBackBufferTexture()->GetWidth() < m_renderWidth; // A downsampled render will be upscaled after postprocessing
// Stereo depends on the render device capabilities and if all the needed buffers and shaders are available
#ifdef ENABLE_XR
   const bool hasStereoPass
      = m_applyPostFX && ((r->m_stereo3Denabled && (r->m_stereo3D != STEREO_OFF) && (r->m_stereo3D != STEREO_VR)) || ((r->m_stereo3D == STEREO_VR) && r->m_vrApplyColorKey));
#else
   const bool hasStereoPass = m_applyPostFX && r->m_stereo3Denabled && (r->m_stereo3D != STEREO_OFF);
#endif

   // Perform color grade LUT / dither / tonemapping, also applying bloom and AO
   RenderTarget* const tonemapRT = ApplyTonemapping(renderedRT, (hasAntialiasPass || hasSharpenPass || hasStereoPass || hasUpscalerPass) ? GetPostProcessRenderTarget1() : GetOutput());

   // Postprocess tail: ball motion blur, AA, sharpening, upscaling (skipped by views without postprocess, e.g. the VR desktop display scene render)
   if (m_applyPostFX)
   {
      // Raytraced ball motion blur (BGFX only)
      renderedRT = ApplyBallMotionBlur(renderedRT, tonemapRT);

      // Perform post processed anti aliasing
      renderedRT = ApplyPostProcessedAntialiasing(renderedRT, (hasSharpenPass || hasStereoPass || hasUpscalerPass) ? nullptr : GetOutput());

      // Performs sharpening
      renderedRT = ApplySharpening(renderedRT, (hasStereoPass || hasUpscalerPass) ? nullptr : GetOutput());

      // Upscale: When using downscaled backbuffer (for performance reason), upscaling is done after postprocessing
      renderedRT = ApplyUpscaling(renderedRT, hasStereoPass ? nullptr : GetOutput());
   }
   else
      renderedRT = tonemapRT; // Without postprocess, the tonemapped render is the final output
   // At this point renderedRT may be the MSAA resolved back buffer, the postprocess buffer or the output backbuffer depending on the enabled postprocessing

   // If using OpenVR, render LiveUI before pushing eyes to headset
   // If using 3D TV stereo mode, render LiveUI before stereo as it must be duplicated per view to be correct
   // For other modes, render UI after all other steps (otherwise it would break the calibration process for stereo anaglyph, and breaks XR passthrough color keying)
   const bool uiBeforeStero = m_renderLiveUI && (r->m_stereo3D == STEREO_SBS || r->m_stereo3D == STEREO_INT || r->m_stereo3D == STEREO_TB || r->m_stereo3D == STEREO_FLIPPED_INT);
   if (uiBeforeStero)
   {
      rd->SetRenderTarget("LiveUI"s, renderedRT, true, true);
      if (renderedRT == GetOutput() && !IsFullOutputArea()) // Clip the UI to this view's area of its shared output
         rd->GetCurrentPass()->m_areaOfInterest = GetOutputAreaClip();
      g_pplayer->m_liveUI->RenderUI();
   }

   // For OpenXR, the LiveUI is rendered to a dedicated mono render target submitted as a view locked quad composition
   // layer (a head locked overlay rendered inside the world locked projection layer was shaken by the compositor
   // reprojection). It is recorded before stereo to allow compositing it on the desktop display.
   RenderTarget* xrUILayer = nullptr;
#ifdef ENABLE_XR
   if (m_renderLiveUI && r->m_stereo3D == STEREO_VR)
      xrUILayer = g_pplayer->m_vrDevice->GetUIRenderTarget();
#endif
   if (xrUILayer != nullptr)
   {
      rd->SetRenderTarget("LiveUI"s, xrUILayer, false, true);
      rd->Clear(clearType::TARGET, 0x00000000);
      g_pplayer->m_liveUI->RenderUI();
      if (rd->GetCurrentPass()->GetCommandCount() <= 1)
         rd->GetCurrentPass()->ClearCommands();
   }

   // Apply stereo
   if (m_applyPostFX)
      renderedRT = ApplyStereo(renderedRT, GetOutput());

   if (m_renderLiveUI && !uiBeforeStero && xrUILayer == nullptr)
   {
      rd->SetRenderTarget("LiveUI"s, renderedRT, true, true);
      if (renderedRT == GetOutput() && !IsFullOutputArea()) // Clip the UI to this view's area of its shared output
         rd->GetCurrentPass()->m_areaOfInterest = GetOutputAreaClip();
      g_pplayer->m_liveUI->RenderUI();
   }

#ifdef ENABLE_XR
   if (IsMirror())
   {
      // Add the XR LiveUI render layer which is submitted to the HMD as a separate composition layer, so it needs to be composited back on the desktop display
      if (RenderTarget* const uiLayer = g_pplayer->m_vrDevice->GetUIRenderTarget(); uiLayer != nullptr)
      {
         rd->AddRenderTargetDependency(uiLayer);
         // TODO the UI shader gets no vertex color from this Vertex3D_NoTex2 quad (see the VR desktop composite in RenderView::ApplyStereo)
         rd->m_uiShader->SetTechnique(ShaderTechnique::LiveUI_mono);
         const Matrix3D ortho = Matrix3D::MatrixOrthoOffCenterRH(0.f, static_cast<float>(uiLayer->GetWidth()), static_cast<float>(uiLayer->GetHeight()), 0.f, 0.f, 1.f);
         const Matrix3D orthoEyes[2] = { ortho, ortho }; // One per eye
         rd->m_uiShader->SetMatrix(ShaderUniform::matWorldView, orthoEyes, rd->m_nEyes);
         rd->m_uiShader->SetVector(ShaderUniform::staticColor_Alpha, 0.f, 0.f, 0.f, 0.f);
         rd->m_uiShader->SetVector(ShaderUniform::clip_plane, 0.f, 0.f, static_cast<float>(uiLayer->GetWidth()), static_cast<float>(uiLayer->GetHeight()));
         rd->m_uiShader->SetTexture(ShaderUniform::tex_base_color, uiLayer->GetColorSampler());
         const float uiW = static_cast<float>(uiLayer->GetWidth()), uiH = static_cast<float>(uiLayer->GetHeight());
         const Vertex3D_NoTex2 overlay[4] = { { 0.f, 0.f, 1.f, 1.f, 1.f, 1.f, 0.f, 0.f }, { uiW, 0.f, 1.f, 1.f, 1.f, 1.f, 1.f, 0.f }, { 0.f, uiH, 1.f, 1.f, 1.f, 1.f, 0.f, 1.f },
            { uiW, uiH, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f } };
         rd->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_TRUE);
         rd->SetRenderState(RenderState::SRCBLEND, RenderState::SRC_ALPHA);
         rd->SetRenderState(RenderState::DESTBLEND, RenderState::INVSRC_ALPHA);
         rd->SetRenderState(RenderState::BLENDOP, RenderState::BLENDOP_ADD);
         rd->DrawTexturedQuad(rd->m_uiShader, overlay, true);
         rd->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_FALSE);
      }
   }
#endif

   // The last rendered render target must be the output back buffer
   assert(renderedRT == GetOutput());
   rd->m_singleLayerRendering = -1;
}
