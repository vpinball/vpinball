#pragma once

#include <map>
#include "typedefs3D.h"

#include "Material.h"
#include "Texture.h"

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
   TRANSFORMSTATE_WORLD = D3DTS_WORLD,
   TRANSFORMSTATE_VIEW = D3DTS_VIEW,
   TRANSFORMSTATE_PROJECTION = D3DTS_PROJECTION
};

enum UsageFlags {
   USAGE_STATIC = D3DUSAGE_WRITEONLY,    // to be used for vertex/index buffers which are uploaded once and never touched again
   USAGE_DYNAMIC = D3DUSAGE_DYNAMIC      // to be used for vertex/index buffers which are locked every frame/very often
};

class RenderDevice;

class TextureManager
{
public:
   TextureManager(RenderDevice& rd) : m_rd(rd)
   { }

   ~TextureManager()
   {
      UnloadAll();
   }

   D3DTexture* LoadTexture(BaseTexture* memtex, const bool linearRGB);
   void SetDirty(BaseTexture* memtex);
   void UnloadTexture(BaseTexture* memtex);
   void UnloadAll();

private:
   struct TexInfo
   {
      D3DTexture* d3dtex;
      int texWidth;
      int texHeight;
      bool dirty;
   };

   RenderDevice& m_rd;
   std::map<BaseTexture*, TexInfo> m_map;
   typedef std::map<BaseTexture*, TexInfo>::iterator Iter;
};

class VertexBuffer : public IDirect3DVertexBuffer9
{
public:
   enum LockFlags
   {
      WRITEONLY = 0,                        // in DX9, this is specified during VB creation
      NOOVERWRITE = D3DLOCK_NOOVERWRITE,    // meaning: no recently drawn vertices are overwritten. only works with dynamic VBs.
      // it's only needed for VBs which are locked several times per frame
      DISCARDCONTENTS = D3DLOCK_DISCARD     // discard previous contents; only works with dynamic VBs
   };

   void lock(const unsigned int offsetToLock, const unsigned int sizeToLock, void **dataBuffer, const DWORD flags);

   void unlock()
   {
      CHECKD3D(this->Unlock());
   }

   void release()
   {
      SAFE_RELEASE_NO_CHECK_NO_SET(this);
   }
private:
   VertexBuffer();     // disable default constructor
};


class IndexBuffer : public IDirect3DIndexBuffer9
{
public:
   enum Format {
      FMT_INDEX16 = D3DFMT_INDEX16,
      FMT_INDEX32 = D3DFMT_INDEX32
   };
   enum LockFlags
   {
      WRITEONLY = 0,                      // in DX9, this is specified during VB creation
      NOOVERWRITE = D3DLOCK_NOOVERWRITE,  // meaning: no recently drawn vertices are overwritten. only works with dynamic VBs.
      // it's only needed for VBs which are locked several times per frame
      DISCARD = D3DLOCK_DISCARD           // discard previous contents; only works with dynamic VBs
   };

   void lock(const unsigned int offsetToLock, const unsigned int sizeToLock, void **dataBuffer, const DWORD flags);

   void unlock()
   {
      CHECKD3D(this->Unlock());
   }

   void release()
   {
      SAFE_RELEASE_NO_CHECK_NO_SET(this);
   }

private:
   IndexBuffer();      // disable default constructor
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
      COLORWRITEENABLE = D3DRS_COLORWRITEENABLE
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


   RenderDevice(const HWND hwnd, const int width, const int height, const bool fullscreen, const int colordepth, int VSync, const bool useAA, const bool stereo3D, const unsigned int FXAA, const bool ss_refl, const bool useNvidiaApi, const bool disable_dwm, const int BWrendering);
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
   D3DTexture* UploadTexture(BaseTexture* const surf, int * const pTexWidth, int * const pTexHeight, const bool linearRGB);
   void UpdateTexture(D3DTexture* const tex, BaseTexture* const surf, const bool linearRGB);

   void SetRenderState(const RenderStates p1, DWORD p2);
   bool SetRenderStateCache(const RenderStates p1, DWORD p2);
   void SetTextureFilter(const DWORD texUnit, DWORD mode);
   void SetTextureAddressMode(const DWORD texUnit, const TextureAddressMode mode);
   void SetTextureStageState(const DWORD stage, const D3DTEXTURESTAGESTATETYPE type, const DWORD value);
   void SetSamplerState(const DWORD Sampler, const D3DSAMPLERSTATETYPE Type, const DWORD Value);

   void CreateVertexBuffer(const unsigned int numVerts, const DWORD usage, const DWORD fvf, VertexBuffer **vBuffer);
   void CreateIndexBuffer(const unsigned int numIndices, const DWORD usage, const IndexBuffer::Format format, IndexBuffer **idxBuffer);

   IndexBuffer* CreateAndFillIndexBuffer(const unsigned int numIndices, const unsigned int * indices);
   IndexBuffer* CreateAndFillIndexBuffer(const unsigned int numIndices, const WORD * indices);
   IndexBuffer* CreateAndFillIndexBuffer(const std::vector<unsigned int>& indices);
   IndexBuffer* CreateAndFillIndexBuffer(const std::vector<WORD>& indices);

   void DrawTexturedQuad(const Vertex3D_TexelOnly* vertices);
   void DrawFullscreenTexturedQuad();
   
   void DrawPrimitiveVB(const RenderDevice::PrimitiveTypes type, const DWORD fvf, VertexBuffer* vb, const DWORD startVertex, const DWORD vertexCount);
   void DrawIndexedPrimitiveVB(const RenderDevice::PrimitiveTypes type, const DWORD fvf, VertexBuffer* vb, const DWORD startVertex, const DWORD vertexCount, IndexBuffer* ib, const DWORD startIndex, const DWORD indexCount);

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
   unsigned int m_FXAA;
   int          m_BWrendering;

private:
   void DrawPrimitive(const RenderDevice::PrimitiveTypes type, const DWORD fvf, const void* vertices, const DWORD vertexCount);

   void UploadAndSetSMAATextures();
   D3DTexture* m_SMAAsearchTexture;
   D3DTexture* m_SMAAareaTexture;

#ifdef USE_D3D9EX
   IDirect3D9Ex* m_pD3DEx;

   IDirect3DDevice9Ex* m_pD3DDeviceEx;
#endif
   IDirect3D9* m_pD3D;

   IDirect3DDevice9* m_pD3DDevice;

   IDirect3DSurface9* m_pBackBuffer;

   D3DTexture* m_pOffscreenBackBufferTexture;
   D3DTexture* m_pOffscreenBackBufferTmpTexture; // stereo/FXAA only
   D3DTexture* m_pOffscreenBackBufferTmpTexture2;// SMAA only

   D3DTexture* m_pBloomBufferTexture;
   D3DTexture* m_pBloomTmpBufferTexture;
   D3DTexture* m_pMirrorTmpBufferTexture;
   D3DTexture* m_pReflectionBufferTexture;

   UINT m_adapter;      // index of the display adapter to use

   static const DWORD TEXTURE_SAMPLERS = 8;
   static const DWORD TEXTURE_STATE_CACHE_SIZE = 256;
   static const DWORD TEXTURE_SAMPLER_CACHE_SIZE = 14;

   std::map<RenderStates, DWORD> renderStateCache;                          // for caching
   DWORD textureStateCache[TEXTURE_SAMPLERS][TEXTURE_STATE_CACHE_SIZE];     // dto.
   DWORD textureSamplerCache[TEXTURE_SAMPLERS][TEXTURE_SAMPLER_CACHE_SIZE]; // dto.

   VertexBuffer* m_curVertexBuffer;       // for caching
   IndexBuffer* m_curIndexBuffer;         // dto.
   VertexDeclaration *currentDeclaration; // dto.

   VertexBuffer *m_quadVertexBuffer;      // internal vb for rendering quads
   //VertexBuffer *m_quadDynVertexBuffer;   // internal vb for rendering dynamic quads

   DWORD m_maxaniso;
   bool m_mag_aniso;

   bool m_autogen_mipmap;
   //bool m_RESZ_support;
   bool m_force_aniso;
   bool m_compress_textures;

   bool m_dwm_was_enabled;
   bool m_dwm_enabled;

public:
   bool m_useNvidiaApi;
   static bool m_INTZ_support;

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

   unsigned int m_stats_drawn_triangles;

   static VertexDeclaration* m_pVertexTexelDeclaration;
   static VertexDeclaration* m_pVertexNormalTexelDeclaration;
   //static VertexDeclaration* m_pVertexNormalTexelTexelDeclaration;
   static VertexDeclaration* m_pVertexTrafoTexelDeclaration;
};

class Shader
{
public:
   Shader(RenderDevice *renderDevice);
   ~Shader();

   bool Load(const BYTE* shaderCodeName, UINT codeSize);
   void Unload();

   ID3DXEffect *Core() const
   {
      return m_shader;
   }

   void Begin(const unsigned int pass)
   {
      unsigned int cPasses;
      CHECKD3D(m_shader->Begin(&cPasses, 0));
      CHECKD3D(m_shader->BeginPass(pass));
   }

   void End()
   {
      CHECKD3D(m_shader->EndPass());
      CHECKD3D(m_shader->End());
   }

   void SetTexture(const D3DXHANDLE texelName, Texture *texel, const bool linearRGB);
   void SetTexture(const D3DXHANDLE texelName, D3DTexture *texel);
   void SetMaterial(const Material * const mat);

   void SetDisableLighting(const vec4& value) // sets the two top and below lighting flags, z and w unused
   {
      if (currentDisableLighting.x != value.x || currentDisableLighting.y != value.y)
      {
         currentDisableLighting = value;
         SetVector("fDisableLighting_top_below", &value);
      }
   }

   void SetAlphaTestValue(const float value)
   {
      if (currentAlphaTestValue != value)
      {
         currentAlphaTestValue = value;
         SetFloat("alphaTestValue", value);
      }
   }

   void SetFlasherColorAlpha(const vec4& color)
   {
      if (currentFlasherColor.x != color.x || currentFlasherColor.y != color.y || currentFlasherColor.z != color.z || currentFlasherColor.w != color.w)
      {
         currentFlasherColor = color;
         SetVector("staticColor_Alpha", &color);
      }
   }

   void SetFlasherData(const vec4& color, const float mode)
   {
      if (currentFlasherData.x != color.x || currentFlasherData.y != color.y || currentFlasherData.z != color.z || currentFlasherData.w != color.w)
      {
         currentFlasherData = color;
         SetVector("alphaTestValueAB_filterMode_addBlend", &color);
      }
      if (currentFlasherMode != mode)
      {
         currentFlasherMode = mode;
         SetFloat("flasherMode", mode);
      }
   }

   void SetLightColorIntensity(const vec4& color)
   {
      if (currentLightColor.x != color.x || currentLightColor.y != color.y || currentLightColor.z != color.z || currentLightColor.w != color.w)
      {
         currentLightColor = color;
         SetVector("lightColor_intensity", &color);
      }
   }

   void SetLightColor2FalloffPower(const vec4& color)
   {
      if (currentLightColor2.x != color.x || currentLightColor2.y != color.y || currentLightColor2.z != color.z || currentLightColor2.w != color.w)
      {
         currentLightColor2 = color;
         SetVector("lightColor2_falloff_power", &color);
      }
   }

   void SetLightData(const vec4& color)
   {
      if (currentLightData.x != color.x || currentLightData.y != color.y || currentLightData.z != color.z || currentLightData.w != color.w)
      {
         currentLightData = color;
         SetVector("lightCenter_maxRange", &color);
      }
   }

   void SetLightImageBackglassMode(const bool imageMode, const bool backglassMode)
   {
      if (currentLightImageMode != (unsigned int)imageMode || currentLightBackglassMode != (unsigned int)backglassMode)
      {
         currentLightImageMode = (unsigned int)imageMode;
         currentLightBackglassMode = (unsigned int)backglassMode;
         SetBool("lightingOff", imageMode || backglassMode); // at the moment can be combined into a single bool due to what the shader actually does in the end
      }
   }

   //

   void SetTechnique(const D3DXHANDLE technique)
   {
      if (strcmp(currentTechnique, technique) /*|| (m_renderDevice->m_curShader != this)*/)
      {
         strncpy_s(currentTechnique, technique, sizeof(currentTechnique)-1);
         //m_renderDevice->m_curShader = this;
         CHECKD3D(m_shader->SetTechnique(technique));
         m_renderDevice->m_curTechniqueChanges++;
      }
   }

   void SetMatrix(const D3DXHANDLE hParameter, const D3DXMATRIX* pMatrix)
   {
      /*CHECKD3D(*/m_shader->SetMatrix(hParameter, pMatrix)/*)*/; // leads to invalid calls when setting some of the matrices (as hlsl compiler optimizes some down to less than 4x4)
      m_renderDevice->m_curParameterChanges++;
   }

   void SetVector(const D3DXHANDLE hParameter, const vec4* pVector)
   {
      CHECKD3D(m_shader->SetVector(hParameter, pVector));
      m_renderDevice->m_curParameterChanges++;
   }

   void SetFloat(const D3DXHANDLE hParameter, const float f)
   {
      CHECKD3D(m_shader->SetFloat(hParameter, f));
      m_renderDevice->m_curParameterChanges++;
   }

   void SetInt(const D3DXHANDLE hParameter, const int i)
   {
      CHECKD3D(m_shader->SetInt(hParameter, i));
      m_renderDevice->m_curParameterChanges++;
   }

   void SetBool(const D3DXHANDLE hParameter, const bool b)
   {
      CHECKD3D(m_shader->SetBool(hParameter, b));
      m_renderDevice->m_curParameterChanges++;
   }

   void SetValue(const D3DXHANDLE hParameter, const void* pData, const unsigned int Bytes)
   {
      CHECKD3D(m_shader->SetValue(hParameter, pData, Bytes));
      m_renderDevice->m_curParameterChanges++;
   }

private:
   ID3DXEffect* m_shader;
   RenderDevice *m_renderDevice;

   // caches:

   Material currentMaterial;

   vec4 currentDisableLighting; // x and y: top and below, z and w unused

   static const DWORD TEXTURESET_STATE_CACHE_SIZE = 5; // current convention: SetTexture gets "TextureX", where X 0..4
   BaseTexture *currentTexture[TEXTURESET_STATE_CACHE_SIZE];
   float   currentAlphaTestValue;
   char    currentTechnique[64];

   vec4 currentFlasherColor; // all flasher only-data
   vec4 currentFlasherData;
   float currentFlasherMode;

   vec4 currentLightColor; // all light only-data
   vec4 currentLightColor2;
   vec4 currentLightData;
   unsigned int currentLightImageMode;
   unsigned int currentLightBackglassMode;
};
