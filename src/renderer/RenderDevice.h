// license:GPLv3+

#pragma once

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

#include <SDL3/SDL.h>

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

#if defined(ENABLE_BGFX)
#define CHECKD3D(s) { s; } 
#elif defined(ENABLE_OPENGL)
#define CHECKD3D(s) { s; }
#elif defined(ENABLE_DX9)
#define CHECKD3D(s) { const HRESULT hrTmp = (s); if (FAILED(hrTmp)) ReportFatalError(hrTmp, __FILE__, __LINE__); }
#endif

class Shader;
class ModelViewProj;

class RenderDeviceState final
{
public:
   RenderDeviceState(RenderDevice* rd);
   ~RenderDeviceState();

   const RenderDevice* m_rd;
   ShaderState* const m_uiShaderState;
   ShaderState* const m_basicShaderState;
   ShaderState* const m_DMDShaderState;
   ShaderState* const m_FBShaderState;
   ShaderState* const m_flasherShaderState;
   ShaderState* const m_lightShaderState;
   ShaderState* const m_ballShaderState;
   ShaderState* const m_stereoShaderState;
   RenderState m_renderState;
};

class RenderDevice final
{
public:
   RenderDevice(VPX::Window* const wnd, const bool isVR, const int nEyes, const bool useNvidiaApi, const bool compressTextures, int nMSAASamples, VideoSyncMode& syncMode);
   ~RenderDevice();

   void AddWindow(VPX::Window* wnd);
   void RemoveWindow(VPX::Window* wnd);

   #if defined(ENABLE_BGFX)
      enum PrimitiveTypes
      {
         TRIANGLESTRIP,
         TRIANGLELIST,
         POINTLIST,
         LINELIST,
         LINESTRIP
      };

   #elif defined(ENABLE_OPENGL)
      enum PrimitiveTypes
      {
         TRIANGLESTRIP = GL_TRIANGLE_STRIP,
         TRIANGLELIST = GL_TRIANGLES,
         POINTLIST = GL_POINTS,
         LINELIST = GL_LINES,
         LINESTRIP = GL_LINE_STRIP
      };

   #elif defined(ENABLE_DX9)
      enum PrimitiveTypes
      {
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
   void DrawMesh(Shader* shader, const bool isTranparentPass, const Vertex3Ds& center, const float depthBias, std::shared_ptr<MeshBuffer> mb, const PrimitiveTypes type, const uint32_t startIndex, const uint32_t indexCount);
   void DrawTexturedQuad(Shader* shader, const Vertex3D_TexelOnly* vertices, const bool isTransparent = false, const float depth = 0.f);
   void DrawTexturedQuad(Shader* shader, const Vertex3D_NoTex2* vertices, const bool isTransparent = false, const float depth = 0.f);
   void DrawFullscreenTexturedQuad(Shader* shader);
   void DrawGaussianBlur(RenderTarget* source, RenderTarget* tmp, RenderTarget* dest, float kernel_size, int singleLayer = -1);
   void AddBeginOfFrameCmd(const std::function<void()>& cmd) { m_renderFrame->AddBeginOfFrameCmd(cmd); }
   void AddEndOfFrameCmd(const std::function<void()>& cmd) { m_renderFrame->AddEndOfFrameCmd(cmd); }
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
   float GetPredictedDisplayDelayInS() const;

   RenderTarget* GetOutputBackBuffer() const { return m_outputWnd[0]->GetBackBuffer(); } // The screen render target (the only one which is not stereo when doing stereo rendering)

   bool DepthBufferReadBackAvailable() const;
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

   void UploadTexture(ITexManCacheable* texture, const bool linearRGB);
   void SetSamplerState(int unit, SamplerFilter filter, SamplerAddressMode clamp_u, SamplerAddressMode clamp_v);
   std::shared_ptr<Sampler> m_nullTexture = nullptr;
   TextureManager m_texMan;
   const bool m_compressTextures;

   bool UseLowPrecision() const { return m_useLowPrecision; }

   unsigned int m_vsyncCount = 0;

   vector<std::shared_ptr<SharedIndexBuffer>> m_pendingSharedIndexBuffers;
   vector<std::shared_ptr<SharedVertexBuffer>> m_pendingSharedVertexBuffers;

   bool m_framePending = false;

   const int m_nEyes;
   Shader* m_uiShader = nullptr;
   Shader* m_basicShader = nullptr;
   Shader *m_DMDShader = nullptr;
   Shader *m_FBShader = nullptr;
   Shader *m_flasherShader = nullptr;
   Shader *m_lightShader = nullptr;
   Shader *m_stereoShader = nullptr;
   Shader *m_ballShader = nullptr;

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

   uint64_t m_lastPresentFrameTick = 0;

   // Swap chain always has at least one output window (OpenGL & DX9 only supports one, DX10+/Metal/Vulkan support multiple)
   vector<VPX::Window*> m_outputWnd;

   void CaptureScreenshot(const vector<VPX::Window*>& wnd, const vector<std::filesystem::path>& filename, const std::function<void(bool)>& callback, int frameDelay = 3);

   int GetVisualLatencyCorrection() const { return m_visualLatencyCorrection; }
   void SetVisualLatencyCorrection(int latencyMs) { m_visualLatencyCorrection = latencyMs; }

private:
   const bool m_isVR;

   bool m_useLowPrecision = false; // OpenGL ES use low precision float and needs some clamping to avoid artifacts, but the clamping causes artefacts if applied with VR scene scaling on other backends.

   std::unique_ptr<RenderFrame> m_renderFrame = nullptr;
   RenderPass* m_currentPass = nullptr;
   RenderPass* m_nextRenderCommandDependency = nullptr;

   RenderState m_current_renderstate, m_renderstate, m_defaultRenderState;
   bool m_logNextFrame = false; // Output a log of next frame to main application log

#if !defined(__STANDALONE__) && !defined(ENABLE_BGFX)
   bool m_dwm_enabled;
#endif

   std::shared_ptr<MeshBuffer> m_quadMeshBuffer; // internal mesh buffer for rendering quads

   void UploadAndSetSMAATextures();
   std::shared_ptr<Sampler> m_SMAAsearchTexture = nullptr;
   std::shared_ptr<Sampler> m_SMAAareaTexture = nullptr;

   int m_visualLatencyCorrection = -1;

   int m_screenshotFrameDelay = 0;
   bool m_screenshotSuccess = true;
   vector<VPX::Window*> m_screenshotWindow;
   vector<std::filesystem::path> m_screenshotFilename;
   std::function<void(bool)> m_screenshotCallback = [](bool) { };

#if defined(ENABLE_BGFX)
public:
   bgfx::ProgramHandle m_program = BGFX_INVALID_HANDLE; // Bound program for next draw submission
   void NextView()
   {
      if (m_activeViewId == bgfx::getCaps()->limits.maxViews - 1)
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
   void ResetActiveView()
   {
      RenderTarget::OnFrameFlushed();
      m_activeViewId = 1; // view 0 & 1 are reserved for mipmap generation (so 1 is before the first available for rendering)
   }
   void SubmitAndFlipFrame()
   {
      ResetActiveView();
      bgfx::frame(); // BGFX always flips backbuffer when its render queue is submitted
   }
   bgfx::VertexLayout* m_pVertexTexelDeclaration = nullptr;
   bgfx::VertexLayout* m_pVertexNormalTexelDeclaration = nullptr;
   int m_activeViewId = 0;
   uint64_t m_bgfxState = 0L;

   bool m_frameNoSync = false; // Flag set when the next frame should be submitted without VBlank sync disabled
   bx::Semaphore m_frameReadySem; // Semaphore to signal when a frame is ready to be submitted
   std::mutex m_frameMutex; // Mutex to lock acces to retained render frame between logic thread and render thread

   std::vector<bgfx::ProgramHandle> m_mipmapPrograms;

private:
   bool m_renderDeviceAlive;
   std::thread m_renderThread;
   static void RenderThread(RenderDevice* rd, const bgfx::Init& init);
   vector<std::shared_ptr<Sampler>> m_pendingTextureUploads;
   std::unique_ptr<ShaderState> m_uniformState = nullptr;

   class tBGFXCallback : public bgfx::CallbackI
   {
   public:
      tBGFXCallback(RenderDevice& rd) : bgfx::CallbackI(), m_rd(rd) { }
      ~tBGFXCallback() override { }
      void fatal(const char* _filePath, uint16_t _line, bgfx::Fatal::Enum _code, const char* _str) override;
      void traceVargs(const char* _filePath, uint16_t _line, const char* _format, va_list _argList) override;
      void profilerBegin(const char* /*_name*/, uint32_t /*_abgr*/, const char* /*_filePath*/, uint16_t /*_line*/) override { }
      void profilerBeginLiteral(const char* /*_name*/, uint32_t /*_abgr*/, const char* /*_filePath*/, uint16_t /*_line*/) override { }
      void profilerEnd() override { }
      uint32_t cacheReadSize(uint64_t /*_id*/) override { return 0; }
      bool cacheRead(uint64_t /*_id*/, void* /*_data*/, uint32_t /*_size*/) override { return false; }
      void cacheWrite(uint64_t /*_id*/, const void* /*_data*/, uint32_t /*_size*/) override { }
      void screenShot(const char* _filePath, uint32_t _width, uint32_t _height, uint32_t _pitch, const void* _data, uint32_t _size, bool _yflip) override;
      void captureBegin(uint32_t /*_width*/, uint32_t /*_height*/, uint32_t /*_pitch*/, bgfx::TextureFormat::Enum /*_format*/, bool /*_yflip*/) override { }
      void captureEnd() override { }
      void captureFrame(const void* /*_data*/, uint32_t /*_size*/) override { }

   private:
      RenderDevice& m_rd;
   } m_bgfxCallback;

#elif defined(ENABLE_OPENGL)
public:
   int getGLVersion() const { return m_GLversion; }
   vector<MeshBuffer::SharedVAO*> m_sharedVAOs;
   vector<Sampler::SamplerBinding*> m_samplerBindings;
   GLuint m_curVAO = 0;
   SDL_GLContext m_sdl_context = nullptr;
   #ifndef __STANDALONE__
      IDXGIOutput* m_DXGIOutput = nullptr;
   #endif
   std::shared_ptr<MeshBuffer> m_quadPNTDynMeshBuffer; // internal vb for rendering dynamic quads (position/normal/texture)
   std::shared_ptr<MeshBuffer> m_quadPTDynMeshBuffer; // internal vb for rendering dynamic quads (position/texture)

private:
   GLfloat m_maxaniso;
   int m_GLversion;
   static GLuint m_samplerStateCache[3 * 3 * 5];

   void CaptureGLScreenshot();

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

   void CaptureDX9Screenshot();

   DWORD m_maxaniso;
   bool m_mag_aniso;
   static constexpr uint32_t TEXTURESET_STATE_CACHE_SIZE = 32;
   SamplerFilter m_bound_filter[TEXTURESET_STATE_CACHE_SIZE];
   SamplerAddressMode m_bound_clampu[TEXTURESET_STATE_CACHE_SIZE];
   SamplerAddressMode m_bound_clampv[TEXTURESET_STATE_CACHE_SIZE];
#endif
};
