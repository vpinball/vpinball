#pragma once

#include <map>
#include "typedefs3D.h"

#include "Material.h"
#include "Texture.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "TextureManager.h"

#define CHECKD3D(s) { const HRESULT hrTmp = (s); if (FAILED(hrTmp)) ReportFatalError(hrTmp, __FILE__, __LINE__); }

void ReportFatalError(const HRESULT hr, const char *file, const int line);
void ReportError(const char *errorText, const HRESULT hr, const char *file, const int line);

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
void EnumerateDisplayModes(const int display, std::vector<VideoMode>& modes);
bool getDisplaySetupByID(const int display, int &x, int &y, int &width, int &height);
int getDisplayList(std::vector<DisplayConfig>& displays);
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

class RenderDevice
{
public:
   enum RenderStates
   {
      ALPHABLENDENABLE = D3DRS_ALPHABLENDENABLE,
      ALPHATESTENABLE = D3DRS_ALPHATESTENABLE,
      ALPHAREF = D3DRS_ALPHAREF,
      ALPHAFUNC = D3DRS_ALPHAFUNC,
      BLENDOP = D3DRS_BLENDOP,
      CLIPPING = D3DRS_CLIPPING,
      CLIPPLANEENABLE = D3DRS_CLIPPLANEENABLE,
      CULLMODE = D3DRS_CULLMODE,
      DESTBLEND = D3DRS_DESTBLEND,
      LIGHTING = D3DRS_LIGHTING,
      SRCBLEND = D3DRS_SRCBLEND,
      SRGBWRITEENABLE = D3DRS_SRGBWRITEENABLE,
      ZENABLE = D3DRS_ZENABLE,
      ZFUNC = D3DRS_ZFUNC,
      ZWRITEENABLE = D3DRS_ZWRITEENABLE,
      TEXTUREFACTOR = D3DRS_TEXTUREFACTOR,
      DEPTHBIAS = D3DRS_DEPTHBIAS,
      COLORWRITEENABLE = D3DRS_COLORWRITEENABLE,
      RENDERSTATE_COUNT,
      RENDERSTATE_INVALID
   };

   enum RenderStateValue
   {
      //Booleans
      RS_FALSE = FALSE,
      RS_TRUE = TRUE,
      //Culling
      CULL_NONE = D3DCULL_NONE,
      CULL_CW = D3DCULL_CW,
      CULL_CCW = D3DCULL_CCW,
      //Depth functions
      Z_ALWAYS = D3DCMP_ALWAYS,
      Z_LESS = D3DCMP_LESS,
      Z_LESSEQUAL = D3DCMP_LESSEQUAL,
      Z_GREATER = D3DCMP_GREATER,
      Z_GREATEREQUAL = D3DCMP_GREATEREQUAL,
      //Blending ops
      BLENDOP_MAX = D3DBLENDOP_MAX,
      BLENDOP_ADD = D3DBLENDOP_ADD,
      BLENDOP_REVSUBTRACT = D3DBLENDOP_REVSUBTRACT,
      //Blending values
      ZERO = D3DBLEND_ZERO,
      ONE = D3DBLEND_ONE,
      SRC_ALPHA = D3DBLEND_SRCALPHA,
      DST_ALPHA = D3DBLEND_DESTALPHA,
      INVSRC_ALPHA = D3DBLEND_INVSRCALPHA,
      INVSRC_COLOR = D3DBLEND_INVSRCCOLOR,
      //Clipping planes
      PLANE0 = D3DCLIPPLANE0,

      UNDEFINED
   };


   enum TextureAddressMode {
      TEX_WRAP = D3DTADDRESS_WRAP,
      TEX_CLAMP = D3DTADDRESS_CLAMP,
      TEX_MIRROR = D3DTADDRESS_MIRROR
   };

   enum PrimitiveTypes {
      TRIANGLEFAN = D3DPT_TRIANGLEFAN,
      TRIANGLESTRIP = D3DPT_TRIANGLESTRIP,
      TRIANGLELIST = D3DPT_TRIANGLELIST,
      POINTLIST = D3DPT_POINTLIST,
      LINELIST = D3DPT_LINELIST,
      LINESTRIP = D3DPT_LINESTRIP
   };


   RenderDevice(const HWND hwnd, const int width, const int height, const bool fullscreen, const int colordepth, int VSync, const bool useAA, const bool stereo3D, const unsigned int FXAA, const bool sharpen, const bool ss_refl, const bool useNvidiaApi, const bool disable_dwm, const int BWrendering);
   ~RenderDevice();
   void CreateDevice(int &refreshrate, UINT adapterIndex = D3DADAPTER_DEFAULT);
   bool LoadShaders();

   void BeginScene();
   void EndScene();

   void Clear(const DWORD numRects, const D3DRECT* rects, const DWORD flags, const D3DCOLOR color, const D3DVALUE z, const DWORD stencil);
   void Flip(const bool vsync);

   bool SetMaximumPreRenderedFrames(const DWORD frames);

   D3DTexture* GetBackBufferTexture() const { return m_pOffscreenBackBufferTexture; }
   D3DTexture* GetBackBufferTmpTexture() const { return m_pOffscreenBackBufferTmpTexture; }   // stereo/FXAA only
   D3DTexture* GetBackBufferTmpTexture2() const { return m_pOffscreenBackBufferTmpTexture2; } // SMAA only
   D3DTexture* GetMirrorTmpBufferTexture() const { return m_pMirrorTmpBufferTexture; }
   D3DTexture* GetReflectionBufferTexture() const { return m_pReflectionBufferTexture; }
   RenderTarget* GetOutputBackBuffer() const { return m_pBackBuffer; }

   D3DTexture* GetBloomBufferTexture() const { return m_pBloomBufferTexture; }
   D3DTexture* GetBloomTmpBufferTexture() const { return m_pBloomTmpBufferTexture; }

   RenderTarget* DuplicateRenderTarget(RenderTarget* src);
   D3DTexture* DuplicateTexture(RenderTarget* src);
   D3DTexture* DuplicateTextureSingleChannel(RenderTarget* src);
   D3DTexture* DuplicateDepthTexture(RenderTarget* src);

   void SetRenderTarget(RenderTarget* surf);
   void SetRenderTarget(D3DTexture* tex);
   void SetZBuffer(RenderTarget* surf);
   void UnSetZBuffer();

   void* AttachZBufferTo(RenderTarget* surf);
   void CopySurface(RenderTarget* dest, RenderTarget* src);
   void CopySurface(D3DTexture* dest, RenderTarget* src);
   void CopySurface(RenderTarget* dest, D3DTexture* src);
   void CopySurface(D3DTexture* dest, D3DTexture* src);
   void CopySurface(void* dest, void* src);
   void CopyDepth(D3DTexture* dest, RenderTarget* src);
   void CopyDepth(D3DTexture* dest, D3DTexture* src);
   void CopyDepth(D3DTexture* dest, void* src);

   bool DepthBufferReadBackAvailable();

   D3DTexture* CreateSystemTexture(BaseTexture* const surf, const bool linearRGB);
   D3DTexture* UploadTexture(BaseTexture* const surf, int* const pTexWidth, int* const pTexHeight, const bool linearRGB, const bool clamptoedge = false);
   void UpdateTexture(D3DTexture* const tex, BaseTexture* const surf, const bool linearRGB);

   void SetRenderState(const RenderStates p1, DWORD p2);
   bool SetRenderStateCache(const RenderStates p1, DWORD p2);
   void SetRenderStateCulling(RenderStateValue cull);
   void SetRenderStateDepthBias(float bias);
   void SetRenderStateClipPlane0(const bool enabled);
   void SetRenderStateAlphaTestFunction(const DWORD testValue, const RenderStateValue testFunction, const bool enabled);

   void SetTextureFilter(const DWORD texUnit, DWORD mode);
   void SetTextureAddressMode(const DWORD texUnit, const TextureAddressMode mode);
   void SetTextureStageState(const DWORD stage, const D3DTEXTURESTAGESTATETYPE type, const DWORD value);
   void SetSamplerState(const DWORD Sampler, const D3DSAMPLERSTATETYPE Type, const DWORD Value);

   //!! Remove:
   //void CreateVertexBuffer(const unsigned int numVerts, const DWORD usage, const DWORD fvf, VertexBuffer **vBuffer);
   //void CreateIndexBuffer(const unsigned int numIndices, const DWORD usage, const IndexBuffer::Format format, IndexBuffer **idxBuffer);

   //IndexBuffer* CreateAndFillIndexBuffer(const unsigned int numIndices, const unsigned int * indices);
   //IndexBuffer* CreateAndFillIndexBuffer(const unsigned int numIndices, const WORD * indices);
   //IndexBuffer* CreateAndFillIndexBuffer(const std::vector<unsigned int>& indices);
   //IndexBuffer* CreateAndFillIndexBuffer(const std::vector<WORD>& indices);

   void DrawTexturedQuad(const Vertex3D_TexelOnly* vertices);
   void DrawFullscreenTexturedQuad();
   
   void DrawPrimitiveVB(const PrimitiveTypes type, const DWORD fvf, VertexBuffer* vb, const DWORD startVertex, const DWORD vertexCount, const bool stereo);
   void DrawIndexedPrimitiveVB(const PrimitiveTypes type, const DWORD fvf, VertexBuffer* vb, const DWORD startVertex, const DWORD vertexCount, IndexBuffer* ib, const DWORD startIndex, const DWORD indexCount);

   void SetViewport(const ViewPort*);
   void GetViewport(ViewPort*);

   void SetTransform(const TransformStateType, const D3DMATRIX*);
   void GetTransform(const TransformStateType, D3DMATRIX*);

   void ForceAnisotropicFiltering(const bool enable) { m_force_aniso = enable; }
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

   inline void CreateVertexDeclaration(const VertexElement * const element, VertexDeclaration ** declaration)
   {
      CHECKD3D(m_pD3DDevice->CreateVertexDeclaration(element, declaration));
   }

   inline void SetVertexDeclaration(VertexDeclaration * declaration)
   {
      if (declaration != currentDeclaration)
      {
         CHECKD3D(m_pD3DDevice->SetVertexDeclaration(declaration));
         currentDeclaration = declaration;

         m_curStateChanges++;
      }
   }

   inline IDirect3DDevice9* GetCoreDevice() const
   {
      return m_pD3DDevice;
   }

   HWND         m_windowHwnd;
   int          m_width;
   int          m_height;
   bool         m_fullscreen;
   int          m_colorDepth;
   int          m_vsync;
   bool         m_useAA;
   bool         m_stereo3D;
   bool         m_ssRefl;
   bool         m_disableDwm;
   bool         m_sharpen;
   unsigned int m_FXAA;
   int          m_BWrendering;

private:
   void DrawPrimitive(const PrimitiveTypes type, const DWORD fvf, const void* vertices, const DWORD vertexCount);

   void UploadAndSetSMAATextures();
   D3DTexture* m_SMAAsearchTexture;
   D3DTexture* m_SMAAareaTexture;

#ifndef ENABLE_SDL
#ifdef USE_D3D9EX
   IDirect3D9Ex* m_pD3DEx;

   IDirect3DDevice9Ex* m_pD3DDeviceEx;
#endif
   IDirect3D9* m_pD3D;

   IDirect3DDevice9* m_pD3DDevice;
#endif

   IDirect3DSurface9* m_pBackBuffer;

   D3DTexture* m_pOffscreenBackBufferTexture;
   D3DTexture* m_pOffscreenBackBufferTmpTexture; // stereo/FXAA only
   D3DTexture* m_pOffscreenBackBufferTmpTexture2;// SMAA only

   D3DTexture* m_pBloomBufferTexture;
   D3DTexture* m_pBloomTmpBufferTexture;
   D3DTexture* m_pMirrorTmpBufferTexture;
   D3DTexture* m_pReflectionBufferTexture;

   UINT m_adapter;      // index of the display adapter to use

   static constexpr DWORD TEXTURE_SAMPLERS = 8;
   static constexpr DWORD TEXTURE_STATE_CACHE_SIZE = 256;
   static constexpr DWORD TEXTURE_SAMPLER_CACHE_SIZE = 14;

   std::map<RenderStates, DWORD> renderStateCache;                          // for caching
   DWORD textureStateCache[TEXTURE_SAMPLERS][TEXTURE_STATE_CACHE_SIZE];     // dto.
   DWORD textureSamplerCache[TEXTURE_SAMPLERS][TEXTURE_SAMPLER_CACHE_SIZE]; // dto.

   VertexDeclaration *currentDeclaration; // for caching

   DWORD m_maxaniso;
   bool m_mag_aniso;

   bool m_autogen_mipmap;
   //bool m_RESZ_support;
   bool m_force_aniso;
   bool m_compress_textures;

   bool m_dwm_was_enabled;
   bool m_dwm_enabled;

public:
   static bool m_useNvidiaApi;
   static bool m_INTZ_support;

   static VertexBuffer* m_quadVertexBuffer;      // internal vb for rendering quads //!! only on primary device for now!
   //static VertexBuffer *m_quadDynVertexBuffer; // internal vb for rendering dynamic quads //!!

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
#ifdef SEPARATE_CLASSICLIGHTSHADER
   Shader *classicLightShader;
#else
#define classicLightShader basicShader
#endif

   //Shader* m_curShader; // for caching

   TextureManager m_texMan;

   static unsigned int m_stats_drawn_triangles;

   static VertexDeclaration* m_pVertexTexelDeclaration;
   static VertexDeclaration* m_pVertexNormalTexelDeclaration;
   //static VertexDeclaration* m_pVertexNormalTexelTexelDeclaration;
   static VertexDeclaration* m_pVertexTrafoTexelDeclaration;
};

#include "Shader.h"
