#pragma once

#include <inc/robin_hood.h>
#include "typedefs3D.h"

#include "Material.h"
#include "Texture.h"
#include "Sampler.h"
#include "RenderTarget.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "TextureManager.h"

#ifdef ENABLE_VR
#include <openvr.h>
#endif

#ifndef ENABLE_SDL
#define CHECKNVAPI(s) { NvAPI_Status hr = (s); if (hr != NVAPI_OK) { NvAPI_ShortString ss; NvAPI_GetErrorMessage(hr,ss); g_pvp->MessageBox(ss, "NVAPI", MB_OK | MB_ICONEXCLAMATION); } }
#endif

void ReportFatalError(const HRESULT hr, const char *file, const int line);
void ReportError(const char *errorText, const HRESULT hr, const char *file, const int line);

#if 1//def _DEBUG
#ifdef ENABLE_SDL
//void checkGLErrors(const char *file, const int line);
#define CHECKD3D(s) { s; } //checkGLErrors(__FILE__, __LINE__); } // by now the callback is used instead
#else //ENABLE_SDL
#define CHECKD3D(s) { const HRESULT hrTmp = (s); if (FAILED(hrTmp)) ReportFatalError(hrTmp, __FILE__, __LINE__); }
#endif
#else //_DEBUG
#define CHECKD3D(s) { s; }
#endif

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
   int display;
   int adapter;
   int top;
   int left;
   int width;
   int height;
   bool isPrimary;
   char DeviceName[CCHDEVICENAME];
   char GPU_Name[MAX_DEVICE_IDENTIFIER_STRING];
};

int getNumberOfDisplays();
void EnumerateDisplayModes(const int display, vector<VideoMode>& modes);
bool getDisplaySetupByID(const int display, int &x, int &y, int &width, int &height);
int getDisplayList(vector<DisplayConfig>& displays);
int getPrimaryDisplay();

enum TransformStateType {
#ifdef ENABLE_SDL
   TRANSFORMSTATE_WORLD = 0,
   TRANSFORMSTATE_VIEW = 1,
   TRANSFORMSTATE_PROJECTION = 2
#else
   TRANSFORMSTATE_WORLD = D3DTS_WORLD,
   TRANSFORMSTATE_VIEW = D3DTS_VIEW,
   TRANSFORMSTATE_PROJECTION = D3DTS_PROJECTION
#endif
};

enum UsageFlags {
#ifdef ENABLE_SDL
   USAGE_STATIC = GL_STATIC_DRAW,
   USAGE_DYNAMIC = GL_DYNAMIC_DRAW
#else
   USAGE_STATIC = D3DUSAGE_WRITEONLY,    // to be used for vertex/index buffers which are uploaded once and never touched again
   USAGE_DYNAMIC = D3DUSAGE_DYNAMIC      // to be used for vertex/index buffers which are locked every frame/very often
#endif
};

class Shader;

class RenderDevice final
{
public:
#define RENDER_STATE(name, bitpos, bitsize) name,
   // These definition must be copy/pasted to RenderDevice.h/cpp when modified to keep the implementation in sync
   enum RenderStates
   {
      RENDER_STATE(ALPHABLENDENABLE, 0, 1) // RS_FALSE or RS_TRUE
      RENDER_STATE(ZENABLE, 1, 1) // RS_FALSE or RS_TRUE
      RENDER_STATE(BLENDOP, 2, 2) // Operation from BLENDOP_MAX, BLENDOP_ADD, BLENDOP_SUB, BLENDOP_REVSUBTRACT
      RENDER_STATE(CLIPPLANEENABLE, 4, 1) // PLANE0 or 0 (for disable)
      RENDER_STATE(CULLMODE, 5, 2) // CULL_NONE, CULL_CW, CULL_CCW
      RENDER_STATE(DESTBLEND, 7, 3) // ZERO, ONE, SRC_ALPHA, DST_ALPHA, INVSRC_ALPHA, INVSRC_COLOR
      RENDER_STATE(SRCBLEND, 10, 3) // ZERO, ONE, SRC_ALPHA, DST_ALPHA, INVSRC_ALPHA, INVSRC_COLOR
      RENDER_STATE(ZFUNC, 13, 3) // Operation from Z_ALWAYS, Z_LESS, Z_LESSEQUAL, Z_GREATER, Z_GREATEREQUAL
      RENDER_STATE(ZWRITEENABLE, 16, 1) // RS_FALSE or RS_TRUE
      RENDER_STATE(COLORWRITEENABLE, 17, 4) // RGBA mask (4 bits)
      RENDERSTATE_COUNT,
      RENDERSTATE_INVALID
   };
#undef RENDER_STATE

   enum RenderStateValue
   {
      //Booleans
      RS_FALSE = 0,
      RS_TRUE = 1,
      //Culling
      CULL_NONE = 0,
      CULL_CW = 1,
      CULL_CCW = 2,
      //Depth functions
      Z_ALWAYS = 0,
      Z_LESS = 1,
      Z_LESSEQUAL = 2,
      Z_GREATER = 3,
      Z_GREATEREQUAL = 4,
      //Blending ops
      BLENDOP_MAX = 0,
      BLENDOP_ADD = 1,
      BLENDOP_REVSUBTRACT = 2,
      //Blending values
      ZERO = 0,
      ONE = 1,
      SRC_ALPHA = 2,
      DST_ALPHA = 3,
      INVSRC_ALPHA = 4,
      INVSRC_COLOR = 5,
      //Clipping planes
      PLANE0 = 1,
      //Color mask
      RGBMASK_NONE = 0x00000000u,
      RGBMASK_RGBA = 0x0000000Fu,

      UNDEFINED
   };

#ifdef ENABLE_SDL
   enum PrimitiveTypes
   {
      TRIANGLEFAN = GL_TRIANGLE_FAN,
      TRIANGLESTRIP = GL_TRIANGLE_STRIP,
      TRIANGLELIST = GL_TRIANGLES,
      POINTLIST = GL_POINTS,
      LINELIST = GL_LINES,
      LINESTRIP = GL_LINE_STRIP
   };

   SDL_Window* m_sdl_playfieldHwnd;
   SDL_GLContext m_sdl_context;
#else
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

   RenderDevice(const HWND hwnd, const int width, const int height, const bool fullscreen, const int colordepth, int VSync, const float AAfactor, const StereoMode stereo3D, const unsigned int FXAA, const bool sharpen, const bool ss_refl, const bool useNvidiaApi, const bool disable_dwm, const int BWrendering);
   ~RenderDevice();
   void CreateDevice(int &refreshrate, UINT adapterIndex = D3DADAPTER_DEFAULT);
   bool LoadShaders();

   void BeginScene();
   void EndScene();

   void Clear(const DWORD flags, const D3DCOLOR color, const D3DVALUE z, const DWORD stencil);
   void Flip(const bool vsync);

   bool SetMaximumPreRenderedFrames(const DWORD frames);

   RenderTarget* GetMSAABackBufferTexture() const { return m_pOffscreenMSAABackBufferTexture; } // Main render target, may be MSAA enabled and not suited for sampling, also may have stereo output (2 viewports)
   void ResolveMSAA(); // Resolve MSAA back buffer texture to be sample  from back buffer texture
   RenderTarget* GetBackBufferTexture() const { return m_pOffscreenBackBufferTexture; } // Main render target, with MSAA resolved if any, also may have stereo output (2 viewports)
   RenderTarget* GetPostProcessRenderTarget1();
   RenderTarget* GetPostProcessRenderTarget2();
   RenderTarget* GetPostProcessRenderTarget(RenderTarget* renderedRT);
   RenderTarget* GetOffscreenVR(int eye) const { return eye == 0 ? m_pOffscreenVRLeft : m_pOffscreenVRRight; }
   RenderTarget* GetReflectionBufferTexture() const { return m_pReflectionBufferTexture; }
   RenderTarget* GetBloomBufferTexture() const { return m_pBloomBufferTexture; }
   RenderTarget* GetBloomTmpBufferTexture() const { return m_pBloomTmpBufferTexture; }
   RenderTarget* GetOutputBackBuffer() const { return m_pBackBuffer; } // The screen render target
   RenderTarget* GetAORenderTarget(int idx);
   void SwapAORenderTargets();
   void ReleaseAORenderTargets() { delete m_pAORenderTarget1; m_pAORenderTarget1 = nullptr; delete m_pAORenderTarget2; m_pAORenderTarget2 = nullptr; }

   // VR/Stereo Stuff
#ifdef ENABLE_VR
   void InitVR();
   bool IsVRReady() const { return m_pHMD != nullptr; }
   void SetTransformVR();
   void UpdateVRPosition();
   void tableUp();
   void tableDown();
   void recenterTable();
   void updateTableMatrix();
   static bool isVRinstalled();
   static bool isVRturnedOn();
   static void turnVROff();

   float m_scale = 1.0f;

private:
   static vr::IVRSystem* m_pHMD;
   float m_slope, m_orientation, m_tablex, m_tabley, m_tablez;
   vr::TrackedDevicePose_t hmdPosition;
   Matrix3D m_matProj[2];
   Matrix3D m_matView;
   Matrix3D m_tableWorld;
   vr::TrackedDevicePose_t* m_rTrackedDevicePose;

public:
#endif

   bool DepthBufferReadBackAvailable();

   struct RenderStateCache
   {
      unsigned int state;
      float depth_bias;
   };
   void SetClipPlane0(const vec4 &plane);
   void SetRenderState(const RenderStates p1, const RenderStateValue p2);
   void SetRenderStateCulling(RenderStateValue cull);
   void SetRenderStateDepthBias(float bias);
   void SetRenderStateClipPlane0(const bool enabled);
   void CopyRenderStates(const bool copyTo, RenderStateCache& state);
   void ApplyRenderStates();
   const string GetRenderStateLog() const;

private:
   struct RenderStateMask
   {
      uint32_t shift;
      uint32_t mask;
      uint32_t clear_mask;
   };
   static const RenderStateMask render_state_masks[RENDERSTATE_COUNT];
   RenderStateCache m_current_renderstate, m_renderstate;

public:
   void DrawMesh(MeshBuffer* mb, const PrimitiveTypes type, const DWORD startIndice, const DWORD indexCount);
   void DrawTexturedQuad(const Vertex3D_TexelOnly* vertices);
   void DrawFullscreenTexturedQuad();
   
   void DrawGaussianBlur(Sampler* source, RenderTarget* tmp, RenderTarget* dest, float kernel_size);

   void SetViewport(const ViewPort*);
   void GetViewport(ViewPort*);

   void SetTransform(const TransformStateType p1, const Matrix3D* p2, const int count = 1);
   void GetTransform(const TransformStateType p1, Matrix3D* p2, const int count = 1);

   void SetMainTextureDefaultFiltering(const SamplerFilter filter);
   void CompressTextures(const bool enable) { m_compress_textures = enable; }

   // performance counters
   unsigned int Perf_GetNumDrawCalls() const      { return m_frameDrawCalls; }
   unsigned int Perf_GetNumStateChanges() const   { return m_frameStateChanges; }
   unsigned int Perf_GetNumTextureChanges() const { return m_frameTextureChanges; }
   unsigned int Perf_GetNumParameterChanges() const { return m_frameParameterChanges; }
   unsigned int Perf_GetNumTechniqueChanges() const { return m_frameTechniqueChanges; }
   unsigned int Perf_GetNumTextureUploads() const { return m_frameTextureUpdates; }
   unsigned int Perf_GetNumLockCalls() const;

   void FreeShader();

   void CreateVertexDeclaration(const VertexElement * const element, VertexDeclaration ** declaration);
   void SetVertexDeclaration(VertexDeclaration * declaration);

#ifdef ENABLE_SDL
   int getGLVersion() const { return m_GLversion; }
#else
   IDirect3DDevice9* GetCoreDevice() const { return m_pD3DDevice; }
#endif

   HWND getHwnd() const { return m_windowHwnd; }

   HWND         m_windowHwnd;
   int          m_width; // Width of the render buffer (not the window width, for example for stereo the render width is doubled, or for VR, the size depends on the headset)
   int          m_height; // Height of the render buffer
   bool         m_fullscreen;
   int          m_colorDepth;
   int          m_vsync;
   StereoMode   m_stereo3D;
   float        m_AAfactor;
   bool         m_ssRefl;
   bool         m_disableDwm;
   bool         m_sharpen;
   unsigned int m_FXAA;
   int          m_BWrendering;

private:
   void UploadAndSetSMAATextures();

public:
   Sampler* m_SMAAsearchTexture = nullptr;
   Sampler* m_SMAAareaTexture = nullptr;

private:
#ifndef ENABLE_SDL
#ifdef USE_D3D9EX
   IDirect3D9Ex* m_pD3DEx;
   IDirect3DDevice9Ex* m_pD3DDeviceEx;
#endif
   IDirect3D9* m_pD3D;
   IDirect3DDevice9* m_pD3DDevice;
#endif

   RenderTarget* m_pBackBuffer = nullptr;

   RenderTarget* m_pOffscreenMSAABackBufferTexture = nullptr;
   RenderTarget* m_pOffscreenBackBufferTexture = nullptr;
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

   VertexDeclaration *currentDeclaration; // for caching

public:
   void SetSamplerState(int unit, SamplerFilter filter, SamplerAddressMode clamp_u, SamplerAddressMode clamp_v);

private:
#ifdef ENABLE_SDL
   static GLuint m_samplerStateCache[3 * 3 * 5];
#else
   static constexpr DWORD TEXTURESET_STATE_CACHE_SIZE = 32;
   SamplerFilter m_bound_filter[TEXTURESET_STATE_CACHE_SIZE];
   SamplerAddressMode m_bound_clampu[TEXTURESET_STATE_CACHE_SIZE];
   SamplerAddressMode m_bound_clampv[TEXTURESET_STATE_CACHE_SIZE];
#endif

#ifdef ENABLE_SDL
   GLfloat m_maxaniso;
   int m_GLversion;
   Matrix3D m_MatWorld, m_MatView, m_MatProj[2];
#else
   DWORD m_maxaniso;
   bool m_mag_aniso;
#endif

public:
   bool m_autogen_mipmap;
   bool m_compress_textures;

private:
   bool m_dwm_was_enabled;
   bool m_dwm_enabled;

   MeshBuffer* m_quadMeshBuffer = nullptr; // internal vb for rendering quads
   MeshBuffer* m_quadDynMeshBuffer = nullptr; // internal vb for rendering dynamic quads

public:
   // for caching
   VertexBuffer* m_curVertexBuffer = nullptr;
   IndexBuffer* m_curIndexBuffer = nullptr;

public:
#ifndef ENABLE_SDL
   bool m_useNvidiaApi;
   bool m_INTZ_support;
   bool NVAPIinit;
#endif

   // performance counters
   unsigned int m_curDrawCalls, m_frameDrawCalls;
   unsigned int m_curStateChanges, m_frameStateChanges;
   unsigned int m_curTextureChanges, m_frameTextureChanges;
   unsigned int m_curParameterChanges, m_frameParameterChanges;
   unsigned int m_curTechniqueChanges, m_frameTechniqueChanges;
   unsigned int m_curTextureUpdates, m_frameTextureUpdates;

   Shader *basicShader;
   Shader *DMDShader;
   Shader *FBShader;
   Shader *flasherShader;
   Shader *lightShader;
   Shader *StereoShader;
#ifdef SEPARATE_CLASSICLIGHTSHADER
   Shader *classicLightShader;
#else
#define classicLightShader basicShader
#endif

   //Shader* m_curShader; // for caching

   TextureManager m_texMan;

#ifdef ENABLE_SDL
   std::vector<SamplerBinding*> m_samplerBindings;
#endif

   static unsigned int m_stats_drawn_triangles;

#ifndef ENABLE_SDL
   VertexDeclaration* m_pVertexTexelDeclaration;
   VertexDeclaration* m_pVertexNormalTexelDeclaration;
   VertexDeclaration* m_pVertexTrafoTexelDeclaration;
#endif
};
