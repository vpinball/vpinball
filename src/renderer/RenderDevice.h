// license:GPLv3+

#pragma once

#include "robin_hood.h"
#include "typedefs3D.h"

#include "parts/Material.h"
#include "Texture.h"
#include "Sampler.h"
#include "RenderTarget.h"
#include "RenderState.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "MeshBuffer.h"
#include "TextureManager.h"
#include "RenderFrame.h"
#include "RenderPass.h"
#include "Window.h"

#if defined(ENABLE_SDL_VIDEO)
#include <SDL3/SDL.h>
#endif

#if defined(ENABLE_BGFX)
#include <thread>
#include <mutex>
#include "bx/semaphore.h"
#endif

#if defined(ENABLE_OPENGL) && !defined(__STANDALONE__)
#include <d3d11.h> // Used to get a VSync source if DWM is not available
#endif

#if defined(ENABLE_DX9)
#define CHECKNVAPI(s) { NvAPI_Status hr = (s); if (hr != NVAPI_OK) { NvAPI_ShortString ss; NvAPI_GetErrorMessage(hr,ss); g_pvp->MessageBox(ss, "NVAPI", MB_OK | MB_ICONEXCLAMATION); } }
#endif

void ReportFatalError(const HRESULT hr, const char *file, const int line);
void ReportError(const char *errorText, const HRESULT hr, const char *file, const int line);

#if 1//defined(_DEBUG)
#if defined(ENABLE_BGFX)
#define CHECKD3D(s) { s; } 
#elif defined(ENABLE_OPENGL)
//void checkGLErrors(const char *file, const int line);
#define CHECKD3D(s) { s; } //checkGLErrors(__FILE__, __LINE__); } // by now the callback is used instead
#elif defined(ENABLE_DX9)
#define CHECKD3D(s) { const HRESULT hrTmp = (s); if (FAILED(hrTmp)) ReportFatalError(hrTmp, __FILE__, __LINE__); }
#endif
#else //_DEBUG
#define CHECKD3D(s) { s; }
#endif

class Shader;
class ModelViewProj;

class RenderDeviceState
{
public:
   RenderDeviceState(RenderDevice* rd);
   ~RenderDeviceState();

   const RenderDevice* m_rd;
   Shader::ShaderState* const m_basicShaderState;
   Shader::ShaderState* const m_DMDShaderState;
   Shader::ShaderState* const m_FBShaderState;
   Shader::ShaderState* const m_flasherShaderState;
   Shader::ShaderState* const m_lightShaderState;
   Shader::ShaderState* const m_ballShaderState;
   Shader::ShaderState* const m_stereoShaderState;
   RenderState m_renderState;
};

class RenderDevice final
{
public:
   RenderDevice(VPX::Window* const wnd, const bool isVR, const int nEyes, const bool useNvidiaApi, const bool disable_dwm, const bool compressTextures, const int BWrendering, int nMSAASamples, VideoSyncMode& syncMode);
   ~RenderDevice();

   void AddWindow(VPX::Window* wnd);

   #if defined(ENABLE_BGFX)
      enum PrimitiveTypes
      {
         TRIANGLEFAN,
         TRIANGLESTRIP,
         TRIANGLELIST,
         POINTLIST,
         LINELIST,
         LINESTRIP
      };

   #elif defined(ENABLE_OPENGL)
      enum PrimitiveTypes
      {
         TRIANGLEFAN = GL_TRIANGLE_FAN,
         TRIANGLESTRIP = GL_TRIANGLE_STRIP,
         TRIANGLELIST = GL_TRIANGLES,
         POINTLIST = GL_POINTS,
         LINELIST = GL_LINES,
         LINESTRIP = GL_LINE_STRIP
      };

   #elif defined(ENABLE_DX9)
      enum PrimitiveTypes
      {
         TRIANGLEFAN = D3DPT_TRIANGLEFAN,
         TRIANGLESTRIP = D3DPT_TRIANGLESTRIP,
         TRIANGLELIST = D3DPT_TRIANGLELIST,
         POINTLIST = D3DPT_POINTLIST,
         LINELIST = D3DPT_LINELIST,
         LINESTRIP = D3DPT_LINESTRIP
      };
   #endif

   ////////////////////////////////////////////////////////////////////////////////////////////////
   // (retained) RenderFrame API: Following calls will enqueue rendercommand to the renderframe.
   // Getters returns the state of the renderframe, not the of the renderdevice.
   RenderPass* GetCurrentPass() { return m_currentPass; }
   RenderTarget* GetCurrentRenderTarget() const { assert(m_currentPass != nullptr); return m_currentPass->m_rt; }
   void SetRenderTarget(const string& passName, RenderTarget* rt, const bool useRTContent = true, const bool forceNewPass = false);
   void AddRenderTargetDependency(RenderTarget* rt, const bool needDepth = false);
   void AddRenderTargetDependencyOnNextRenderCommand(RenderTarget* rt);
   void Clear(const DWORD flags, const DWORD colorARGB);
   void BlitRenderTarget(RenderTarget* source, RenderTarget* destination, const bool copyColor = true, const bool copyDepth = true,  
                         const int x1 = -1, const int y1 = -1, const int w1 = -1, const int h1 = -1,
                         const int x2 = -1, const int y2 = -1, const int w2 = -1, const int h2 = -1,
                         const int srcLayer = -1, const int dstLayer = -1);
   void SubmitVR(RenderTarget* source);
   void RenderLiveUI();
   void DrawMesh(Shader* shader, const bool isTranparentPass, const Vertex3Ds& center, const float depthBias, MeshBuffer* mb, const PrimitiveTypes type, const DWORD startIndex, const DWORD indexCount);
   void DrawTexturedQuad(Shader* shader, const Vertex3D_TexelOnly* vertices, const bool isTransparent = false, const float depth = 0.f);
   void DrawTexturedQuad(Shader* shader, const Vertex3D_NoTex2* vertices, const bool isTransparent = false, const float depth = 0.f);
   void DrawFullscreenTexturedQuad(Shader* shader);
   void DrawGaussianBlur(RenderTarget* source, RenderTarget* tmp, RenderTarget* dest, float kernel_size, int singleLayer = -1);
   void AddEndOfFrameCmd(const std::function<void()>& cmd) { m_renderFrame.AddEndOfFrameCmd(cmd); }
   void LogNextFrame() { m_logNextFrame = true; }
   bool IsLogNextFrame() const { return m_logNextFrame; }
   void SubmitRenderFrame();
   void DiscardRenderFrame();

   // RenderState used in submitted render command
   void SetDefaultRenderState() { m_defaultRenderState = m_renderstate; }
   void ResetRenderState() { m_renderstate = m_defaultRenderState; }
   RenderState& GetRenderState() { return m_renderstate; }
   void SetRenderState(const RenderState::RenderStates p1, const RenderState::RenderStateValue p2);
   void SetRenderStateDepthBias(float bias);
   void CopyRenderStates(const bool copyTo, RenderState& state);
   void CopyRenderStates(const bool copyTo, RenderDeviceState& state);
   void EnableAlphaBlend(const bool additiveBlending, const bool set_dest_blend = true, const bool set_blend_op = true);

   ////////////////////////////////////////////////////////////////////////////////////////////////
   // (live) RenderDevice state and operation API

   void Flip();
   void WaitForVSync(const bool asynchronous);

   RenderTarget* GetOutputBackBuffer() const { return m_outputWnd[0]->GetBackBuffer(); } // The screen render target (the only one which is not stereo when doing stereo rendering)

   bool DepthBufferReadBackAvailable();
   bool SupportLayeredRendering() const
   {
      #if defined(ENABLE_BGFX)
      return bgfx::getCaps()->supported & (BGFX_CAPS_INSTANCING | BGFX_CAPS_TEXTURE_2D_ARRAY | BGFX_CAPS_VIEWPORT_LAYER_ARRAY);
      #elif defined(ENABLE_OPENGL)
      return true;
      #elif defined(ENABLE_DX9)
      return false;
      #endif
   }

   void SetClipPlane(const vec4& plane);

   // Active (live on GPU) render state
   void ApplyRenderStates();
   RenderState& GetActiveRenderState() { return m_current_renderstate; }

   void SetMainTextureDefaultFiltering(const SamplerFilter filter);

   void UploadTexture(BaseTexture* texture, const bool linearRGB);
   void SetSamplerState(int unit, SamplerFilter filter, SamplerAddressMode clamp_u, SamplerAddressMode clamp_v);
   void UnbindSampler(Sampler* sampler);
   Sampler* m_nullTexture = nullptr;
   TextureManager m_texMan;
   const bool m_compressTextures;

   bool UseLowPrecision() const { return m_useLowPrecision; }

   unsigned int m_vsyncCount = 0;

   vector<SharedIndexBuffer*> m_pendingSharedIndexBuffers;
   vector<SharedVertexBuffer*> m_pendingSharedVertexBuffers;

   bool m_framePending = false;

   const int m_nEyes;
   Shader* m_basicShader = nullptr;
   Shader *m_DMDShader = nullptr;
   Shader *m_FBShader = nullptr;
   Shader *m_flasherShader = nullptr;
   Shader *m_lightShader = nullptr;
   Shader *m_stereoShader = nullptr;
   Shader* m_ballShader = nullptr;

   // performance counters
   unsigned int Perf_GetNumDrawCalls() const        { return m_frameDrawCalls; }
   unsigned int Perf_GetNumStateChanges() const     { return m_frameStateChanges; }
   unsigned int Perf_GetNumTextureChanges() const   { return m_frameTextureChanges; }
   unsigned int Perf_GetNumParameterChanges() const { return m_frameParameterChanges; }
   unsigned int Perf_GetNumTechniqueChanges() const { return m_frameTechniqueChanges; }
   unsigned int Perf_GetNumTextureUploads() const   { return m_frameTextureUpdates; }
   unsigned int Perf_GetNumLockCalls() const        { return m_frameLockCalls; }
   unsigned int m_curDrawCalls = 0, m_frameDrawCalls = 0;
   unsigned int m_curStateChanges = 0, m_frameStateChanges = 0;
   unsigned int m_curTextureChanges = 0, m_frameTextureChanges = 0;
   unsigned int m_curParameterChanges = 0, m_frameParameterChanges = 0;
   unsigned int m_curTechniqueChanges = 0, m_frameTechniqueChanges = 0;
   unsigned int m_curTextureUpdates = 0, m_frameTextureUpdates = 0;
   unsigned int m_curLockCalls = 0, m_frameLockCalls = 0;
   unsigned int m_curDrawnTriangles = 0, m_frameDrawnTriangles = 0;

   U64 m_lastPresentFrameTick = 0;

   unsigned int m_nOutputWnd = 1; // Swap chain always has at least one output window (OpenGL & DX9 only supports one, DX10+/Metal/Vulkan support multiple)
   VPX::Window* m_outputWnd[8];

private:
   const bool m_isVR;

   bool m_useLowPrecision = false; // OpenGL ES use low precision float and needs some clamping to avoid artifacts, but the clamping causes artefacts if applied with VR scene scaling on other backends.

   RenderFrame m_renderFrame;
   RenderPass* m_currentPass = nullptr;
   RenderPass* m_nextRenderCommandDependency = nullptr;

   RenderState m_current_renderstate, m_renderstate, m_defaultRenderState;
   bool m_logNextFrame = false; // Output a log of next frame to main application log

   bool m_dwm_was_enabled;
   bool m_dwm_enabled;

   MeshBuffer* m_quadMeshBuffer = nullptr; // internal mesh buffer for rendering quads

   void UploadAndSetSMAATextures();
   Sampler* m_SMAAsearchTexture = nullptr;
   Sampler* m_SMAAareaTexture = nullptr;

#if defined(ENABLE_BGFX)
public:
   bgfx::ProgramHandle m_program = BGFX_INVALID_HANDLE; // Bound program for next draw submission
   void NextView()
   {
      if (m_activeViewId == bgfx::getCaps()->limits.maxViews - 2) // Last view is reserved for ImGui
      {
         PLOGE << "Frame submitted and flipped since BGFX view limit was reached. [BGFX was compiled with a maximum of " << bgfx::getCaps()->limits.maxViews << " views]";
         SubmitRenderFrame();
         SubmitAndFlipFrame();
      }
      m_activeViewId++;
      bgfx::resetView(m_activeViewId);
      bgfx::setViewMode(m_activeViewId, bgfx::ViewMode::Sequential);
      bgfx::setViewClear(m_activeViewId, BGFX_CLEAR_NONE);
      bgfx::touch(m_activeViewId);
   }
   void SubmitAndFlipFrame()
   {
      RenderTarget::OnFrameFlushed();
      m_activeViewId = -1;
      // BGFX always flips backbuffer when its render queue is submitted
      bgfx::frame();
   }
   bgfx::VertexLayout* m_pVertexTexelDeclaration = nullptr;
   bgfx::VertexLayout* m_pVertexNormalTexelDeclaration = nullptr;
   int m_activeViewId = -1;
   uint64_t m_bgfxState = 0L;

   bool m_frameNoSync = false; // Flag set when the next frame should be submitted without VBlank sync disabled
   bx::Semaphore m_frameReadySem; // Semaphore to signal when a frame is ready to be submitted
   std::mutex m_frameMutex; // Mutex to lock acces to retained render frame between logic thread and render thread

private:
   bool m_renderDeviceAlive;
   std::thread m_renderThread;
   static void RenderThread(RenderDevice* rd, const bgfx::Init& init);
   vector<Sampler*> m_pendingTextureUploads;

#elif defined(ENABLE_OPENGL)
public:
   int getGLVersion() const { return m_GLversion; }
   vector<MeshBuffer::SharedVAO*> m_sharedVAOs;
   std::vector<SamplerBinding*> m_samplerBindings;
   GLuint m_curVAO = 0;
   SDL_GLContext m_sdl_context = nullptr;
   #ifndef __STANDALONE__
      IDXGIOutput* m_DXGIOutput = nullptr;
   #endif
   MeshBuffer* m_quadPNTDynMeshBuffer = nullptr; // internal vb for rendering dynamic quads (position/normal/texture)
   MeshBuffer* m_quadPTDynMeshBuffer = nullptr; // internal vb for rendering dynamic quads (position/texture)

private:
   GLfloat m_maxaniso;
   int m_GLversion;
   static GLuint m_samplerStateCache[3 * 3 * 5];

#elif defined(ENABLE_DX9)
public:
   IDirect3DDevice9* GetCoreDevice() const { return m_pD3DDevice; }
   IDirect3DDevice9Ex* GetCoreDeviceEx() const { return m_pD3DDeviceEx; }

   IDirect3DVertexBuffer9* m_curVertexBuffer = nullptr;
   IDirect3DIndexBuffer9* m_curIndexBuffer = nullptr;
   IDirect3DVertexDeclaration9* m_currentVertexDeclaration = nullptr;
   IDirect3DVertexDeclaration9* m_pVertexTexelDeclaration = nullptr;
   IDirect3DVertexDeclaration9* m_pVertexNormalTexelDeclaration = nullptr;

   bool m_autogen_mipmap;
   bool m_useNvidiaApi;
   bool m_INTZ_support;
   bool NVAPIinit;

private:
   IDirect3D9Ex* m_pD3DEx;
   IDirect3DDevice9Ex* m_pD3DDeviceEx;
   IDirect3D9* m_pD3D;
   IDirect3DDevice9* m_pD3DDevice;

   DWORD m_maxaniso;
   bool m_mag_aniso;
   static constexpr DWORD TEXTURESET_STATE_CACHE_SIZE = 32;
   SamplerFilter m_bound_filter[TEXTURESET_STATE_CACHE_SIZE];
   SamplerAddressMode m_bound_clampu[TEXTURESET_STATE_CACHE_SIZE];
   SamplerAddressMode m_bound_clampv[TEXTURESET_STATE_CACHE_SIZE];
#endif
};
