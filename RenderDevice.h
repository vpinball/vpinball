#pragma once

#include <map>
#include <d3d9.h>
#include <d3dx9.h>
#include "Material.h"
#include "Texture.h"

#define CHECKD3D(s) { HRESULT hr = (s); if (FAILED(hr)) ReportError(hr, __FILE__, __LINE__); }

void ReportError(HRESULT hr, const char *file, int line);

typedef IDirect3DTexture9 D3DTexture;
typedef D3DVIEWPORT9 ViewPort;
typedef IDirect3DSurface9 RenderTarget;
typedef D3DVERTEXELEMENT9 VertexElement;
typedef IDirect3DVertexDeclaration9 VertexDeclaration;

struct VideoMode
{
   int width;
   int height;
   int depth;
   int refreshrate;
};

void EnumerateDisplayModes(const int adapter, std::vector<VideoMode>& modes);


enum TransformStateType {
    TRANSFORMSTATE_WORLD      = D3DTS_WORLD,
    TRANSFORMSTATE_VIEW       = D3DTS_VIEW,
    TRANSFORMSTATE_PROJECTION = D3DTS_PROJECTION
};

enum UsageFlags {
    USAGE_DYNAMIC   = D3DUSAGE_DYNAMIC      // to be used for vertex/index buffers which are locked every frame
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

    D3DTexture* LoadTexture(BaseTexture* memtex);
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

	void lock( unsigned int offsetToLock, unsigned int sizeToLock, void **dataBuffer, DWORD flags )
    {
       CHECKD3D(this->Lock(offsetToLock, sizeToLock, dataBuffer, flags));
   }
    
	void unlock(void)
    {
        CHECKD3D(this->Unlock());
    }
    
	void release(void)
    {
        this->Release();
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

	void lock( unsigned int offsetToLock, unsigned int sizeToLock, void **dataBuffer, DWORD flags )
    {
        CHECKD3D(this->Lock(offsetToLock, sizeToLock, dataBuffer, flags) );
    }
    
	void unlock(void)
    {
        CHECKD3D(this->Unlock());
    }
    
	void release(void)
    {
        this->Release();
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
      ALPHABLENDENABLE   = D3DRS_ALPHABLENDENABLE,
      ALPHATESTENABLE    = D3DRS_ALPHATESTENABLE,
      ALPHAREF           = D3DRS_ALPHAREF,
      ALPHAFUNC          = D3DRS_ALPHAFUNC,
	  BLENDOP            = D3DRS_BLENDOP,
      CLIPPING           = D3DRS_CLIPPING,
      CLIPPLANEENABLE    = D3DRS_CLIPPLANEENABLE,
      CULLMODE           = D3DRS_CULLMODE,
      DESTBLEND          = D3DRS_DESTBLEND,
      LIGHTING           = D3DRS_LIGHTING,
      SRCBLEND           = D3DRS_SRCBLEND,
      ZENABLE            = D3DRS_ZENABLE,
      ZFUNC              = D3DRS_ZFUNC,
      ZWRITEENABLE       = D3DRS_ZWRITEENABLE,
      TEXTUREFACTOR      = D3DRS_TEXTUREFACTOR,
      DEPTHBIAS          = D3DRS_DEPTHBIAS,
      COLORWRITEENABLE   = D3DRS_COLORWRITEENABLE
   };

   enum TextureAddressMode {
       TEX_WRAP          = D3DTADDRESS_WRAP,
       TEX_CLAMP         = D3DTADDRESS_CLAMP,
       TEX_MIRROR        = D3DTADDRESS_MIRROR
   };

   RenderDevice(HWND hwnd, int width, int height, bool fullscreen, int screenWidth, int screenHeight, int colordepth, int &refreshrate, int VSync, bool useAA, bool stereo3DFXAA);
   ~RenderDevice();

   void BeginScene();
   void EndScene();

   void Clear(DWORD numRects, D3DRECT* rects, DWORD flags, D3DCOLOR color, D3DVALUE z, DWORD stencil);
   void Flip(bool vsync);

   RenderTarget* GetBackBuffer() { return m_pOffscreenBackBuffer; }
   D3DTexture* GetBackBufferTexture() { return m_pOffscreenBackBufferTexture; }
   RenderTarget* GetOutputBackBuffer() { return m_pBackBuffer; }

   RenderTarget* DuplicateRenderTarget(RenderTarget* src);
   D3DTexture* DuplicateTexture(RenderTarget* src);
   D3DTexture* DuplicateTextureSingleChannel(RenderTarget* src);
   D3DTexture* DuplicateDepthTexture(RenderTarget* src);

   void SetRenderTarget( RenderTarget* );
   void SetZBuffer( RenderTarget* );

   RenderTarget* AttachZBufferTo(RenderTarget* surf);
   void CopySurface(RenderTarget* dest, RenderTarget* src);
   void CopySurface(D3DTexture* dest, RenderTarget* src);
   void CopyDepth(D3DTexture* dest, RenderTarget* src);

   D3DTexture* CreateSystemTexture(BaseTexture* surf);
   D3DTexture* UploadTexture(BaseTexture* surf, int *pTexWidth=NULL, int *pTexHeight=NULL);
   void UpdateTexture(D3DTexture* tex, BaseTexture* surf);

   void SetRenderState( const RenderStates p1, const DWORD p2 );
   void SetTexture( DWORD, D3DTexture* );
   void SetTextureFilter(DWORD texUnit, DWORD mode);
   void SetTextureAddressMode(DWORD texUnit, TextureAddressMode mode);
   void SetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value);

   void CreateVertexBuffer( unsigned int numVerts, DWORD usage, DWORD fvf, VertexBuffer **vBuffer );
   void CreateIndexBuffer(unsigned int numIndices, DWORD usage, IndexBuffer::Format format, IndexBuffer **idxBuffer);

   IndexBuffer* CreateAndFillIndexBuffer(unsigned int numIndices, const unsigned int * indices);
   IndexBuffer* CreateAndFillIndexBuffer(unsigned int numIndices, const WORD * indices);
   IndexBuffer* CreateAndFillIndexBuffer(const std::vector<unsigned int>& indices);
   IndexBuffer* CreateAndFillIndexBuffer(const std::vector<WORD>& indices);

   void DrawPrimitive(D3DPRIMITIVETYPE type, DWORD fvf, const void* vertices, DWORD vertexCount);
   void DrawIndexedPrimitive(D3DPRIMITIVETYPE type, DWORD fvf, const void* vertices, DWORD vertexCount, const WORD* indices, DWORD indexCount);
   void DrawPrimitiveVB(D3DPRIMITIVETYPE type, VertexBuffer* vb, DWORD startVertex, DWORD vertexCount);
   void DrawIndexedPrimitiveVB(D3DPRIMITIVETYPE type, VertexBuffer* vb, DWORD startVertex, DWORD vertexCount, const WORD* indices, DWORD indexCount);
   void DrawIndexedPrimitiveVB(D3DPRIMITIVETYPE type, VertexBuffer* vb, DWORD startVertex, DWORD vertexCount, IndexBuffer* ib, DWORD startIndex, DWORD indexCount);

   void SetViewport( ViewPort* );
   void GetViewport( ViewPort* );

   void SetTransform( TransformStateType, D3DMATRIX* );
   void GetTransform( TransformStateType, D3DMATRIX* );

   void ForceAnisotropicFiltering( const bool enable ) { m_force_aniso = enable; }

   // performance counters
   unsigned int Perf_GetNumDrawCalls()         { return m_frameDrawCalls; }
   unsigned int Perf_GetNumStateChanges()      { return m_frameStateChanges; }
   unsigned int Perf_GetNumTextureChanges()    { return m_frameTextureChanges; }

   inline void CreateVertexDeclaration( const VertexElement *element, VertexDeclaration **declaration )
   {
       CHECKD3D(m_pD3DDevice->CreateVertexDeclaration( element, declaration ));
   }

   inline void SetVertexDeclaration( VertexDeclaration *declaration )
   {
      if ( declaration!=currentDeclaration)
      {
         currentDeclaration = declaration;
         CHECKD3D(m_pD3DDevice->SetVertexDeclaration(declaration));
      }
   }

   inline IDirect3DDevice9* GetCoreDevice()
   {
       return m_pD3DDevice;
   }

   Material materialStateCache;

private:
#ifdef USE_D3D9EX
   IDirect3D9Ex* m_pD3D;

   IDirect3DDevice9Ex* m_pD3DDevice;
#else
   IDirect3D9* m_pD3D;

   IDirect3DDevice9* m_pD3DDevice;
#endif

   IDirect3DSurface9* m_pBackBuffer;
   IndexBuffer* m_dynIndexBuffer;      // workaround for DrawIndexedPrimitiveVB

   IDirect3DSurface9* m_pOffscreenBackBuffer;
   D3DTexture* m_pOffscreenBackBufferTexture;

   UINT m_adapter;      // index of the display adapter to use

   static const DWORD RENDER_STATE_CACHE_SIZE=256;
   static const DWORD TEXTURE_STATE_CACHE_SIZE=256;

   DWORD renderStateCache[RENDER_STATE_CACHE_SIZE];
   DWORD textureStateCache[8][TEXTURE_STATE_CACHE_SIZE];

   VertexBuffer* m_curVertexBuffer;     // for caching
   IndexBuffer* m_curIndexBuffer;       // for caching
   D3DTexture* m_curTexture[8];         // for caching
   VertexDeclaration *currentDeclaration;
   DWORD m_maxaniso;
   bool m_mag_aniso;

   bool m_autogen_mipmap;
   //bool m_RESZ_support;
   DWORD currentFVF;
   bool m_force_aniso;

   // performance counters
   unsigned m_curDrawCalls, m_frameDrawCalls;
   unsigned m_curStateChanges, m_frameStateChanges;
   unsigned m_curTextureChanges, m_frameTextureChanges;

public:
   Shader *basicShader;
   Shader *DMDShader;
   TextureManager m_texMan;

   static VertexDeclaration* m_pVertexTexelDeclaration;
   static VertexDeclaration* m_pVertexNormalTexelDeclaration;
   //static VertexDeclaration* m_pVertexNormalTexelTexelDeclaration;
};

class Shader 
{
public:
    Shader(RenderDevice *renderDevice);
    ~Shader();

    bool Load( const BYTE* shaderCodeName, UINT codeSize );
    void Unload();

    ID3DXEffect *Core()
    {
        return m_shader;
    }

    void Begin( unsigned int pass );
    void End();

    void PerformAlphaTest(bool enable);
    void SetAlphaTestValue(float value);
    void SetAlphaValue(float value);
    void SetTexture(D3DXHANDLE texelName, Texture *texel);
    void SetTexture( D3DXHANDLE texelName, D3DTexture *texel);
    void SetStaticColor(D3DXVECTOR4 color);
    void SetMaterial( const Material * const mat );
    void SetTechnique(char *technique);

private:
    ID3DXEffect* m_shader;
    RenderDevice *m_renderDevice;
    
	// caches:
	Texture *currentTexture;
    bool    currentAlphaTest;
    float   currentAlphaTestValue;
    float   currentAlphaValue;
    D3DXVECTOR4 currentColor;
    char   currentTechnique[64];
};
