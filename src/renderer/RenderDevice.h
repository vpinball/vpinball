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

#if defined(ENABLE_SDL_VIDEO)
#include "SDL2/SDL.h"
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

bool IsWindowsVistaOr7();
bool IsWindows10_1803orAbove();

struct VideoMode
{
   int width;
   int height;
   int depth;
   int refreshrate;
};

struct DisplayConfig
{
   int display; // Window Display identifier (the number that appears in the native Windows settings)
   int adapter; // DirectX or SDL display/adapter identifier
   int top;
   int left;
   int width;
   int height;
   bool isPrimary;
   char DeviceName[CCHDEVICENAME]; // Device native identifier, e.g. "\\\\.\\DISPLAY1"
   char GPU_Name[MAX_DEVICE_IDENTIFIER_STRING]; // GPU name if available, device (monitor) name otherwise
};

int getNumberOfDisplays();
void EnumerateDisplayModes(const int display, vector<VideoMode>& modes);
bool getDisplaySetupByID(const int display, int &x, int &y, int &width, int &height);
int getDisplayList(vector<DisplayConfig>& displays);
int getPrimaryDisplay();

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

   SDL_Window* m_sdl_playfieldHwnd = nullptr;
   SDL_GLContext m_sdl_context = nullptr;
   #ifndef __STANDALONE__
   IDXGIOutput* m_DXGIOutput = nullptr;
   #endif

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

   RenderDevice(const HWND hwnd, const int width, const int height, const bool fullscreen, const int colordepth, const float AAfactor,
      const StereoMode stereo3D, const bool useNvidiaApi, const bool disable_dwm, const int BWrendering, int nMSAASamples, 
      int& refreshrate, VideoSyncMode& syncMode, UINT adapterIndex = D3DADAPTER_DEFAULT);
   ~RenderDevice();

   RenderPass* GetCurrentPass() { return m_currentPass; }
   const RenderTarget* GetCurrentRenderTarget() const { assert(m_currentPass != nullptr); return m_currentPass->m_rt; }
   void SetRenderTarget(const string& passName, RenderTarget* rt, const bool useRTContent = true, const bool forceNewPass = false);
   void AddRenderTargetDependency(RenderTarget* rt, const bool needDepth = false);
   void AddRenderTargetDependencyOnNextRenderCommand(RenderTarget* rt);
   void Clear(const DWORD flags, const D3DCOLOR color, const D3DVALUE z, const DWORD stencil);
   void BlitRenderTarget(RenderTarget* source, RenderTarget* destination, const bool copyColor = true, const bool copyDepth = true,  
      const int x1 = -1, const int y1 = -1, const int w1 = -1, const int h1 = -1,
      const int x2 = -1, const int y2 = -1, const int w2 = -1, const int h2 = -1,
      const int srcLayer = -1, const int dstLayer = -1);
   void SubmitVR(RenderTarget* source);
   void RenderLiveUI();
   void DrawMesh(Shader* shader, const bool isTranparentPass, const Vertex3Ds& center, const float depthBias, MeshBuffer* mb, const PrimitiveTypes type, const DWORD startIndex, const DWORD indexCount);
   void DrawTexturedQuad(Shader* shader, const Vertex3D_TexelOnly* vertices);
   void DrawTexturedQuad(Shader* shader, const Vertex3D_NoTex2* vertices);
   void DrawFullscreenTexturedQuad(Shader* shader);
   void DrawGaussianBlur(RenderTarget* source, RenderTarget* tmp, RenderTarget* dest, float kernel_size, int singleLayer = -1);
   void LogNextFrame() { m_logNextFrame = true; }
   bool IsLogNextFrame() const { return m_logNextFrame; }
   void FlushRenderFrame();
   void Flip();
   void WaitForVSync(const bool asynchronous);

   bool SetMaximumPreRenderedFrames(const DWORD frames);

   bool SupportLayeredRendering() const
   {
      #if defined(ENABLE_BGFX)
      // TODO actually use BGFX layered rendering
      return false;
      #elif defined(ENABLE_OPENGL)
      // TODO remove geometry shader, and only support layered rendering on driver supporting ARB_shader_viewport_layer_array (all GPU starting GTX950+),
      // the performance impact will be positive for normal rendering, limited for VR/stereo and these old GPU are not really able to render in VR/Stereo
      return true;
      #elif defined(ENABLE_DX9)
      return false;
      #endif
   }

   RenderTarget* GetMSAABackBufferTexture() const { return m_pOffscreenMSAABackBufferTexture ? m_pOffscreenMSAABackBufferTexture : m_pOffscreenBackBufferTexture1; } // Main render target, may be MSAA enabled and not suited for sampling, also may have stereo output (2 viewports)
   void ResolveMSAA(); // Resolve MSAA back buffer texture to be sample  from back buffer texture
   RenderTarget* GetBackBufferTexture() const { return m_pOffscreenBackBufferTexture1; } // Main render target, with MSAA resolved if any, also may have stereo output (2 viewports)
   RenderTarget* GetPreviousBackBufferTexture() const { return m_pOffscreenBackBufferTexture2; } // Same as back buffer but for previous frame
   RenderTarget* GetPostProcessRenderTarget1();
   RenderTarget* GetPostProcessRenderTarget2();
   RenderTarget* GetPostProcessRenderTarget(RenderTarget* renderedRT);
   RenderTarget* GetOffscreenVR(int eye) const { return eye == 0 ? m_pOffscreenVRLeft : m_pOffscreenVRRight; }
   RenderTarget* GetReflectionBufferTexture();
   RenderTarget* GetBloomBufferTexture() const { return m_pBloomBufferTexture; }
   RenderTarget* GetBloomTmpBufferTexture() const { return m_pBloomTmpBufferTexture; }
   RenderTarget* GetAORenderTarget(int idx);
   void SwapBackBufferRenderTargets();
   void SwapAORenderTargets();
   void ReleaseAORenderTargets() { delete m_pAORenderTarget1; m_pAORenderTarget1 = nullptr; delete m_pAORenderTarget2; m_pAORenderTarget2 = nullptr; }
   RenderTarget* GetOutputBackBuffer() const { return m_pBackBuffer; } // The screen render target (the only one which is not stereo when doing stereo rendering)

   bool DepthBufferReadBackAvailable();

   void SetClipPlane(const vec4 &plane);

   // RenderState used in submitted render command
   void SetDefaultRenderState() { m_defaultRenderState = m_renderstate; }
   void ResetRenderState() { m_renderstate = m_defaultRenderState; };
   RenderState& GetRenderState() { return m_renderstate; }
   void SetRenderState(const RenderState::RenderStates p1, const RenderState::RenderStateValue p2);
   void SetRenderStateDepthBias(float bias);
   void CopyRenderStates(const bool copyTo, RenderState& state);
   void CopyRenderStates(const bool copyTo, RenderDeviceState& state);
   void EnableAlphaBlend(const bool additiveBlending, const bool set_dest_blend = true, const bool set_blend_op = true);

   // Active render state
   void ApplyRenderStates();
   RenderState& GetActiveRenderState() { return m_current_renderstate; }

private:
   RenderState m_current_renderstate, m_renderstate, m_defaultRenderState;
   bool m_logNextFrame = false; // Output a log of next frame to main application log

public:
   void SetViewport(const ViewPort*);
   void GetViewport(ViewPort*);

   void SetMainTextureDefaultFiltering(const SamplerFilter filter);
   void CompressTextures(const bool enable) { m_compress_textures = enable; }

   // performance counters
   unsigned int Perf_GetNumDrawCalls() const        { return m_frameDrawCalls; }
   unsigned int Perf_GetNumStateChanges() const     { return m_frameStateChanges; }
   unsigned int Perf_GetNumTextureChanges() const   { return m_frameTextureChanges; }
   unsigned int Perf_GetNumParameterChanges() const { return m_frameParameterChanges; }
   unsigned int Perf_GetNumTechniqueChanges() const { return m_frameTechniqueChanges; }
   unsigned int Perf_GetNumTextureUploads() const   { return m_frameTextureUpdates; }
   unsigned int Perf_GetNumLockCalls() const        { return m_frameLockCalls; }

   #if defined(ENABLE_BGFX)
   bgfx::ProgramHandle m_program = BGFX_INVALID_HANDLE; // Bound program for next draw submission
   
   #elif defined(ENABLE_OPENGL)
   int getGLVersion() const { return m_GLversion; }
   
   #elif defined(ENABLE_DX9)
   IDirect3DDevice9* GetCoreDevice() const { return m_pD3DDevice; }
   #endif

   HWND getHwnd() const { return m_windowHwnd; }

   const int        m_width;  // Width of the render buffer (not the window width, for example for stereo the render width is doubled, or for VR, the size depends on the headset)
   const int        m_height; // Height of the render buffer
   const bool       m_fullscreen;
   const int        m_colorDepth;
   const StereoMode m_stereo3D;
   const float      m_AAfactor;

private:
   HWND             m_windowHwnd;

   void UploadAndSetSMAATextures();

public:
   Sampler* m_SMAAsearchTexture = nullptr;
   Sampler* m_SMAAareaTexture = nullptr;
   Sampler* m_nullTexture = nullptr;

private:
   RenderTarget* m_pBackBuffer = nullptr;

   RenderTarget* m_pOffscreenMSAABackBufferTexture = nullptr;
   RenderTarget* m_pOffscreenBackBufferTexture1 = nullptr;
   RenderTarget* m_pOffscreenBackBufferTexture2 = nullptr;
   RenderTarget* m_pPostProcessRenderTarget1 = nullptr;
   RenderTarget* m_pPostProcessRenderTarget2 = nullptr;
   RenderTarget* m_pOffscreenVRLeft = nullptr;
   RenderTarget* m_pOffscreenVRRight = nullptr;
   RenderTarget* m_pBloomBufferTexture = nullptr;
   RenderTarget* m_pBloomTmpBufferTexture = nullptr;
   RenderTarget* m_pReflectionBufferTexture = nullptr;
   RenderTarget* m_pAORenderTarget1 = nullptr;
   RenderTarget* m_pAORenderTarget2 = nullptr;

   unsigned int m_adapter; // index of the display adapter to use

public:
   void SetSamplerState(int unit, SamplerFilter filter, SamplerAddressMode clamp_u, SamplerAddressMode clamp_v);

   bool m_autogen_mipmap;
   bool m_compress_textures;

private:
   bool m_dwm_was_enabled;
   bool m_dwm_enabled;

public:
   U64 m_lastVSyncUs = 0;
   unsigned int m_vsyncCount = 0;

private:
   MeshBuffer* m_quadMeshBuffer = nullptr;       // internal vb for rendering quads

public:
   MeshBuffer* m_quadPNTDynMeshBuffer = nullptr; // internal vb for rendering dynamic quads (position/normal/texture)
   MeshBuffer* m_quadPTDynMeshBuffer = nullptr;  // internal vb for rendering dynamic quads (position/texture)

   vector<SharedIndexBuffer*> m_pendingSharedIndexBuffers;
   vector<SharedVertexBuffer*> m_pendingSharedVertexBuffers;

   // performance counters
   unsigned int m_curDrawCalls = 0, m_frameDrawCalls = 0;
   unsigned int m_curStateChanges = 0, m_frameStateChanges = 0;
   unsigned int m_curTextureChanges = 0, m_frameTextureChanges = 0;
   unsigned int m_curParameterChanges = 0, m_frameParameterChanges = 0;
   unsigned int m_curTechniqueChanges = 0, m_frameTechniqueChanges = 0;
   unsigned int m_curTextureUpdates = 0, m_frameTextureUpdates = 0;
   unsigned int m_curLockCalls = 0, m_frameLockCalls = 0;
   unsigned int m_curDrawnTriangles = 0, m_frameDrawnTriangles = 0;

   Shader *m_basicShader = nullptr;
   Shader *m_DMDShader = nullptr;
   Shader *m_FBShader = nullptr;
   Shader *m_flasherShader = nullptr;
   Shader *m_lightShader = nullptr;
   Shader *m_stereoShader = nullptr;
   Shader* m_ballShader = nullptr;

   void UnbindSampler(Sampler* sampler);

   TextureManager m_texMan;

private :
   RenderFrame m_renderFrame;
   RenderPass* m_currentPass = nullptr;
   RenderPass* m_nextRenderCommandDependency = nullptr;

#if defined(ENABLE_BGFX)
public:
   bgfx::VertexLayout* m_pVertexTexelDeclaration = nullptr;
   bgfx::VertexLayout* m_pVertexNormalTexelDeclaration = nullptr;
   int m_activeViewId = -1;
   int m_maxViewId = 254;
   uint64_t m_bgfxState = 0L;
   
#elif defined(ENABLE_OPENGL)
public:
   vector<MeshBuffer::SharedVAO*> m_sharedVAOs;
   std::vector<SamplerBinding*> m_samplerBindings;
   GLuint m_curVAO = 0;

private:
   GLfloat m_maxaniso;
   int m_GLversion;
   static GLuint m_samplerStateCache[3 * 3 * 5];

#elif defined(ENABLE_DX9)
public:
   IDirect3DVertexBuffer9* m_curVertexBuffer = nullptr;
   IDirect3DIndexBuffer9* m_curIndexBuffer = nullptr;
   IDirect3DVertexDeclaration9* m_currentVertexDeclaration = nullptr;
   IDirect3DVertexDeclaration9* m_pVertexTexelDeclaration = nullptr;
   IDirect3DVertexDeclaration9* m_pVertexNormalTexelDeclaration = nullptr;

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
