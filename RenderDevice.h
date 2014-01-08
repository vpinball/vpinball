#include "stdafx.h"
#include "Material.h"
#pragma once

class VertexBuffer;
class BaseTexture;

class RenderDevice : public IDirect3DDevice7
{
public:

   typedef enum RenderStates
   {
      ALPHABLENDENABLE   = D3DRENDERSTATE_ALPHABLENDENABLE,
      ALPHATESTENABLE    = D3DRENDERSTATE_ALPHATESTENABLE,
      ALPHAREF           = D3DRENDERSTATE_ALPHAREF,
      ALPHAFUNC          = D3DRENDERSTATE_ALPHAFUNC,
      CLIPPING           = D3DRENDERSTATE_CLIPPING,
      CLIPPLANEENABLE    = D3DRENDERSTATE_CLIPPLANEENABLE,
      COLORKEYENABLE     = D3DRENDERSTATE_COLORKEYENABLE,
      CULLMODE           = D3DRENDERSTATE_CULLMODE,
      DITHERENABLE       = D3DRENDERSTATE_DITHERENABLE,
      DESTBLEND          = D3DRENDERSTATE_DESTBLEND,
      LIGHTING           = D3DRENDERSTATE_LIGHTING,
      SPECULARENABLE     = D3DRENDERSTATE_SPECULARENABLE,
      SRCBLEND           = D3DRENDERSTATE_SRCBLEND,
      TEXTUREPERSPECTIVE = D3DRENDERSTATE_TEXTUREPERSPECTIVE,
      ZENABLE            = D3DRENDERSTATE_ZENABLE,
      ZFUNC              = D3DRENDERSTATE_ZFUNC,
      ZWRITEENABLE       = D3DRENDERSTATE_ZWRITEENABLE,
	  NORMALIZENORMALS   = D3DRENDERSTATE_NORMALIZENORMALS,
      TEXTUREFACTOR      = D3DRENDERSTATE_TEXTUREFACTOR
   };
   static bool createDevice(const GUID * const _deviceGUID, LPDIRECT3D7 _dx7, BaseTexture *_backBuffer );

   RenderDevice();

   static RenderDevice* instance();

   virtual void SetMaterial( const THIS_ BaseMaterial * const _material );
   virtual void SetRenderState( const RenderStates p1, const DWORD p2 );
   bool createVertexBuffer( unsigned int _length, DWORD _usage, DWORD _fvf, VertexBuffer **_vBuffer );
   void renderPrimitive(D3DPRIMITIVETYPE _primType, VertexBuffer* _vbuffer, DWORD _startVertex, DWORD _numVertices, LPWORD _indices, DWORD _numIndices, DWORD _flags);
   void renderPrimitiveListed(D3DPRIMITIVETYPE _primType, VertexBuffer* _vbuffer, DWORD _startVertex, DWORD _numVertices, DWORD _flags);

   inline void setVBInVRAM( const BOOL _state )
   {
      vbInVRAM=(_state==1);
   }

   inline void setHardwareAccelerated( const int _hwAcc)
   {
      hardwareAccelerated = _hwAcc;
   }

   inline int getHardwareAccelerated() const
   {
      return hardwareAccelerated;
   }

   //########################## simple wrapper functions (interface for DX7)##################################

   virtual STDMETHODIMP QueryInterface( THIS_ REFIID riid, LPVOID * ppvObj );

   virtual STDOVERRIDEMETHODIMP_(ULONG) AddRef( void );
   virtual STDOVERRIDEMETHODIMP_(ULONG) Release( void );

   virtual STDMETHODIMP GetCaps( THIS_ LPD3DDEVICEDESC7 );

   virtual STDMETHODIMP EnumTextureFormats( THIS_ LPD3DENUMPIXELFORMATSCALLBACK,LPVOID );

   virtual STDMETHODIMP BeginScene( THIS );

   virtual STDMETHODIMP EndScene( THIS );

   virtual STDMETHODIMP GetDirect3D( THIS_ LPDIRECT3D7* );

   virtual STDMETHODIMP SetRenderTarget( THIS_ LPDIRECTDRAWSURFACE7,DWORD );

   virtual STDMETHODIMP GetRenderTarget( THIS_ LPDIRECTDRAWSURFACE7 * );

   virtual STDMETHODIMP Clear( THIS_ DWORD,LPD3DRECT,DWORD,D3DCOLOR,D3DVALUE,DWORD );

   virtual STDMETHODIMP SetTransform( THIS_ D3DTRANSFORMSTATETYPE,LPD3DMATRIX );

   virtual STDMETHODIMP GetTransform( THIS_ D3DTRANSFORMSTATETYPE,LPD3DMATRIX );

   virtual STDMETHODIMP SetViewport( THIS_ LPD3DVIEWPORT7 );

   virtual STDMETHODIMP MultiplyTransform( THIS_ D3DTRANSFORMSTATETYPE,LPD3DMATRIX );

   virtual STDMETHODIMP GetViewport( THIS_ LPD3DVIEWPORT7 );

   virtual STDMETHODIMP SetMaterial( THIS_ LPD3DMATERIAL7 );

   virtual STDMETHODIMP GetMaterial( THIS_ LPD3DMATERIAL7 );

   virtual void getMaterial( THIS_ BaseMaterial *_material );

   virtual STDMETHODIMP SetLight( THIS_ DWORD,LPD3DLIGHT7 );

   virtual STDMETHODIMP GetLight( THIS_ DWORD,LPD3DLIGHT7 );

   virtual STDMETHODIMP SetRenderState( THIS_ D3DRENDERSTATETYPE,DWORD );

   virtual STDMETHODIMP GetRenderState( THIS_ D3DRENDERSTATETYPE,LPDWORD );

   virtual STDMETHODIMP BeginStateBlock( THIS );

   virtual STDMETHODIMP EndStateBlock( THIS_ LPDWORD );

   virtual STDMETHODIMP PreLoad( THIS_ LPDIRECTDRAWSURFACE7 );

   virtual STDMETHODIMP DrawPrimitive( THIS_ D3DPRIMITIVETYPE,DWORD,LPVOID,DWORD,DWORD );

   virtual STDMETHODIMP DrawIndexedPrimitive( THIS_ D3DPRIMITIVETYPE,DWORD,LPVOID,DWORD,LPWORD,DWORD,DWORD );

   virtual STDMETHODIMP SetClipStatus( THIS_ LPD3DCLIPSTATUS );

   virtual STDMETHODIMP GetClipStatus( THIS_ LPD3DCLIPSTATUS );

   virtual STDMETHODIMP DrawPrimitiveStrided( THIS_ D3DPRIMITIVETYPE,DWORD,LPD3DDRAWPRIMITIVESTRIDEDDATA,DWORD,DWORD );

   virtual STDMETHODIMP DrawIndexedPrimitiveStrided( THIS_ D3DPRIMITIVETYPE,DWORD,LPD3DDRAWPRIMITIVESTRIDEDDATA,DWORD,LPWORD,DWORD,DWORD );

   virtual STDMETHODIMP DrawPrimitiveVB( THIS_ D3DPRIMITIVETYPE,LPDIRECT3DVERTEXBUFFER7,DWORD,DWORD,DWORD );

   virtual STDMETHODIMP DrawIndexedPrimitiveVB( THIS_ D3DPRIMITIVETYPE,LPDIRECT3DVERTEXBUFFER7,DWORD,DWORD,LPWORD,DWORD,DWORD );

   virtual STDMETHODIMP ComputeSphereVisibility( THIS_ LPD3DVECTOR,LPD3DVALUE,DWORD,DWORD,LPDWORD );

   virtual STDMETHODIMP GetTexture( THIS_ DWORD,LPDIRECTDRAWSURFACE7 * );

   virtual STDMETHODIMP SetTexture( THIS_ DWORD,LPDIRECTDRAWSURFACE7 );

   virtual STDMETHODIMP GetTextureStageState( THIS_ DWORD,D3DTEXTURESTAGESTATETYPE,LPDWORD );

   virtual STDMETHODIMP SetTextureStageState( THIS_ DWORD,D3DTEXTURESTAGESTATETYPE,DWORD );

   virtual STDMETHODIMP ValidateDevice( THIS_ LPDWORD );

   virtual STDMETHODIMP ApplyStateBlock( THIS_ DWORD );

   virtual STDMETHODIMP CaptureStateBlock( THIS_ DWORD );

   virtual STDMETHODIMP DeleteStateBlock( THIS_ DWORD );

   virtual STDMETHODIMP CreateStateBlock( THIS_ D3DSTATEBLOCKTYPE,LPDWORD );

   virtual STDMETHODIMP Load( THIS_ LPDIRECTDRAWSURFACE7,LPPOINT,LPDIRECTDRAWSURFACE7,LPRECT,DWORD );

   virtual STDMETHODIMP LightEnable( THIS_ DWORD,BOOL );

   virtual STDMETHODIMP GetLightEnable( THIS_ DWORD,BOOL* );

   virtual STDMETHODIMP SetClipPlane( THIS_ DWORD,D3DVALUE* );

   virtual STDMETHODIMP GetClipPlane( THIS_ DWORD,D3DVALUE* );

   virtual STDMETHODIMP GetInfo( THIS_ DWORD,LPVOID,DWORD );

private:
   static const DWORD RENDER_STATE_CACHE_SIZE=256;
   static const DWORD TEXTURE_STATE_CACHE_SIZE=256;

   static RenderDevice *theDevice;
   DWORD renderStateCache[RENDER_STATE_CACHE_SIZE];
   DWORD textureStateCache[8][TEXTURE_STATE_CACHE_SIZE];
   int hardwareAccelerated;
   BaseMaterial materialStateCache;
   bool vbInVRAM;
};

class VertexBuffer : public IDirect3DVertexBuffer7
{
public:

   enum LockFlags
   {
      WRITEONLY = DDLOCK_WRITEONLY,
      NOOVERWRITE = DDLOCK_NOOVERWRITE,
      DISCARDCONTENTS = DDLOCK_DISCARDCONTENTS
   };
   inline bool lock( unsigned int _offsetToLock, unsigned int _sizeToLock, void **_dataBuffer, DWORD _flags )
   {
      return ( !FAILED(this->Lock( (DWORD)_flags, _dataBuffer, 0 )) );
   }
   inline bool unlock(void)
   {
      return ( !FAILED(this->Unlock() ) );
   }
   inline bool optimize( RenderDevice *device)
   {
      return ( !FAILED(this->Optimize((LPDIRECT3DDEVICE7)device,0)));
   }
   inline ULONG release(void)
   {
      while ( this->Release()!=0 );
      return 0;
   }
};
