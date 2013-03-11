#include "stdafx.h"
#include "RenderDevice.h"

static LPDIRECT3DDEVICE7 dx7Device;
RenderDevice* RenderDevice::theDevice=0;

bool RenderDevice::createDevice(const GUID * const _deviceGUID, LPDIRECT3D7 _dx7, Texture *_backBuffer )
{
   memset( theDevice->renderStateCache, 0xFFFFFFFF, sizeof(DWORD)*RENDER_STATE_CACHE_SIZE);
   for( int i=0;i<8;i++ )
      for( int j=0;j<TEXTURE_STATE_CACHE_SIZE;j++ )
         theDevice->textureStateCache[i][j]=0xFFFFFFFF;
   memset(&theDevice->materialStateCache, 0xFFFFFFFF, sizeof(Material));

   HRESULT hr;
   if( FAILED( hr = _dx7->CreateDevice( *_deviceGUID, (LPDIRECTDRAWSURFACE7)_backBuffer, &dx7Device ) ) )
   {
      return false;
   }
   return true;
}

RenderDevice::RenderDevice( void )
{
   theDevice=this;

   memset( renderStateCache, 0xFFFFFFFF, sizeof(DWORD)*RENDER_STATE_CACHE_SIZE);
   for( int i=0;i<8;i++ )
      for( int j=0;j<TEXTURE_STATE_CACHE_SIZE;j++ )
         textureStateCache[i][j]=0xFFFFFFFF;
   memset(&materialStateCache, 0xFFFFFFFF, sizeof(Material));
}

RenderDevice* RenderDevice::instance()
{
   return theDevice;
}

void RenderDevice::SetMaterial( THIS_ Material *_material )
{
   if(_mm_movemask_ps(_mm_and_ps(
	  _mm_and_ps(_mm_cmpeq_ps(_material->d,materialStateCache.d),_mm_cmpeq_ps(_material->a,materialStateCache.a)),
	  _mm_and_ps(_mm_cmpeq_ps(_material->s,materialStateCache.s),_mm_cmpeq_ps(_material->e,materialStateCache.e)))) == 15
	  &&
	  _material->power == materialStateCache.power)
	  return;
   materialStateCache.d = _material->d;
   materialStateCache.a = _material->a;
   materialStateCache.e = _material->e;
   materialStateCache.s = _material->s;
   materialStateCache.power = _material->power;
   dx7Device->SetMaterial( (LPD3DMATERIAL7)_material);
}

void RenderDevice::SetRenderState( RenderStates p1,DWORD p2)
{
   if ( p1<RENDER_STATE_CACHE_SIZE) 
   {
      if( renderStateCache[p1]==p2 )
      {
         // this render state is already set -> don't do anything then
         return;
      }
      renderStateCache[p1]=p2;
   }
   dx7Device->SetRenderState((D3DRENDERSTATETYPE)p1,p2);
}

//########################## simple wrapper functions (interface for DX7)##################################

STDMETHODIMP RenderDevice::QueryInterface( THIS_ REFIID riid, LPVOID * ppvObj )
{
   return dx7Device->QueryInterface(riid,ppvObj);
}

STDMETHODIMP RenderDevice::GetCaps( THIS_ LPD3DDEVICEDESC7 p1)
{
   return dx7Device->GetCaps(p1);
}

STDMETHODIMP RenderDevice::EnumTextureFormats( THIS_ LPD3DENUMPIXELFORMATSCALLBACK p1,LPVOID p2)
{
   return dx7Device->EnumTextureFormats(p1,p2);
}

STDMETHODIMP RenderDevice::BeginScene( THIS )
{
   return dx7Device->BeginScene();
}

STDMETHODIMP RenderDevice::EndScene( THIS )
{
   memset( renderStateCache, 0xFFFFFFFF, sizeof(DWORD)*RENDER_STATE_CACHE_SIZE);
   memset(&materialStateCache, 0xFFFFFFFF, sizeof(Material));
   return dx7Device->EndScene();
}

STDMETHODIMP RenderDevice::GetDirect3D( THIS_ LPDIRECT3D7* p1)
{
   return dx7Device->GetDirect3D(p1);
}

STDMETHODIMP RenderDevice::SetRenderTarget( THIS_ LPDIRECTDRAWSURFACE7 p1,DWORD p2)
{
   return dx7Device->SetRenderTarget(p1,p2);
}

STDMETHODIMP RenderDevice::GetRenderTarget( THIS_ LPDIRECTDRAWSURFACE7 *p1 )
{
   return dx7Device->GetRenderTarget(p1);
}

STDMETHODIMP RenderDevice::Clear( THIS_ DWORD p1,LPD3DRECT p2,DWORD p3,D3DCOLOR p4,D3DVALUE p5,DWORD p6)
{
   return dx7Device->Clear(p1,p2,p3,p4,p5,p6);
}

STDMETHODIMP RenderDevice::SetTransform( THIS_ D3DTRANSFORMSTATETYPE p1,LPD3DMATRIX p2)
{
   return dx7Device->SetTransform(p1,p2);
}

STDMETHODIMP RenderDevice::GetTransform( THIS_ D3DTRANSFORMSTATETYPE p1,LPD3DMATRIX p2)
{
   return dx7Device->GetTransform(p1,p2);
}

STDMETHODIMP RenderDevice::SetViewport( THIS_ LPD3DVIEWPORT7 p1)
{
   return dx7Device->SetViewport(p1);
}

STDMETHODIMP RenderDevice::MultiplyTransform( THIS_ D3DTRANSFORMSTATETYPE p1,LPD3DMATRIX p2)
{
   return dx7Device->MultiplyTransform(p1,p2);
}

STDMETHODIMP RenderDevice::GetViewport( THIS_ LPD3DVIEWPORT7 p1)
{
   return dx7Device->GetViewport(p1);
}

STDMETHODIMP RenderDevice::SetMaterial( THIS_ LPD3DMATERIAL7 p1)
{
   return dx7Device->SetMaterial(p1);
}

STDMETHODIMP RenderDevice::GetMaterial( THIS_ LPD3DMATERIAL7 p1)
{
   return dx7Device->GetMaterial(p1);
}

void RenderDevice::getMaterial( THIS_ Material *_material )
{
   dx7Device->GetMaterial((LPD3DMATERIAL7)_material);
}

STDMETHODIMP RenderDevice::SetLight( THIS_ DWORD p1,LPD3DLIGHT7 p2)
{
   return dx7Device->SetLight(p1,p2);
}

STDMETHODIMP RenderDevice::GetLight( THIS_ DWORD p1,LPD3DLIGHT7 p2 )
{
   return dx7Device->GetLight(p1,p2);
}

STDMETHODIMP RenderDevice::SetRenderState( THIS_ D3DRENDERSTATETYPE p1,DWORD p2)
{
   return dx7Device->SetRenderState(p1,p2);
}

STDMETHODIMP RenderDevice::GetRenderState( THIS_ D3DRENDERSTATETYPE p1,LPDWORD p2)
{
   return dx7Device->GetRenderState(p1,p2);
}

STDMETHODIMP RenderDevice::BeginStateBlock( THIS )
{
   return dx7Device->BeginStateBlock();
}

STDMETHODIMP RenderDevice::EndStateBlock( THIS_ LPDWORD p1)
{
   return dx7Device->EndStateBlock(p1);
}

STDMETHODIMP RenderDevice::PreLoad( THIS_ LPDIRECTDRAWSURFACE7 p1)
{
   return dx7Device->PreLoad(p1);
}

STDMETHODIMP RenderDevice::DrawPrimitive( THIS_ D3DPRIMITIVETYPE p1,DWORD p2,LPVOID p3,DWORD p4,DWORD p5)
{
   return dx7Device->DrawPrimitive(p1,p2,p3,p4,p5);
}

STDMETHODIMP RenderDevice::DrawIndexedPrimitive( THIS_ D3DPRIMITIVETYPE p1,DWORD p2,LPVOID p3,DWORD p4,LPWORD p5,DWORD p6,DWORD p7)
{
   return dx7Device->DrawIndexedPrimitive(p1,p2,p3,p4,p5,p6,p7);
}

STDMETHODIMP RenderDevice::SetClipStatus( THIS_ LPD3DCLIPSTATUS p1)
{
   return dx7Device->SetClipStatus(p1);
}

STDMETHODIMP RenderDevice::GetClipStatus( THIS_ LPD3DCLIPSTATUS p1)
{
   return dx7Device->GetClipStatus(p1);
}

STDMETHODIMP RenderDevice::DrawPrimitiveStrided( THIS_ D3DPRIMITIVETYPE p1,DWORD p2,LPD3DDRAWPRIMITIVESTRIDEDDATA p3,DWORD p4,DWORD p5)
{
   return dx7Device->DrawPrimitiveStrided(p1,p2,p3,p4,p5);
}

STDMETHODIMP RenderDevice::DrawIndexedPrimitiveStrided( THIS_ D3DPRIMITIVETYPE p1,DWORD p2,LPD3DDRAWPRIMITIVESTRIDEDDATA p3,DWORD p4,LPWORD p5,DWORD p6,DWORD p7)
{
   return dx7Device->DrawIndexedPrimitiveStrided(p1,p2,p3,p4,p5,p6,p7);
}

STDMETHODIMP RenderDevice::DrawPrimitiveVB( THIS_ D3DPRIMITIVETYPE p1,LPDIRECT3DVERTEXBUFFER7 p2,DWORD p3,DWORD p4,DWORD p5)
{
   return dx7Device->DrawPrimitiveVB(p1,p2,p3,p4,p5);
}

STDMETHODIMP RenderDevice::DrawIndexedPrimitiveVB( THIS_ D3DPRIMITIVETYPE p1,LPDIRECT3DVERTEXBUFFER7 p2,DWORD p3,DWORD p4,LPWORD p5,DWORD p6,DWORD p7)
{
   return dx7Device->DrawIndexedPrimitiveVB(p1,p2,p3,p4,p5,p6,p7);
}

STDMETHODIMP RenderDevice::ComputeSphereVisibility( THIS_ LPD3DVECTOR p1,LPD3DVALUE p2,DWORD p3,DWORD p4,LPDWORD p5)
{
   return dx7Device->ComputeSphereVisibility(p1,p2,p3,p4,p5);
}

STDMETHODIMP RenderDevice::GetTexture( THIS_ DWORD p1,LPDIRECTDRAWSURFACE7 *p2 )
{
   return dx7Device->GetTexture(p1,p2);
}

STDMETHODIMP RenderDevice::SetTexture( THIS_ DWORD p1,LPDIRECTDRAWSURFACE7 p2 )
{
   return dx7Device->SetTexture(p1,p2);
}

STDMETHODIMP RenderDevice::GetTextureStageState( THIS_ DWORD p1,D3DTEXTURESTAGESTATETYPE p2,LPDWORD p3)
{
   return dx7Device->GetTextureStageState(p1,p2,p3);
}

STDMETHODIMP RenderDevice::SetTextureStageState( THIS_ DWORD p1,D3DTEXTURESTAGESTATETYPE p2,DWORD p3)
{
   if( p2<TEXTURE_STATE_CACHE_SIZE && p1<8) 
   {
      if( textureStateCache[p1][p2]==p3 )
      {
         // texture stage state hasn't changed since last call of this function -> do nothing here
         return D3D_OK;
      }
   }
   textureStateCache[p1][p2]=p3;
   return dx7Device->SetTextureStageState(p1,p2,p3);
}

STDMETHODIMP RenderDevice::ValidateDevice( THIS_ LPDWORD p1)
{
   return dx7Device->ValidateDevice(p1);
}

STDMETHODIMP RenderDevice::ApplyStateBlock( THIS_ DWORD p1)
{
   return dx7Device->ApplyStateBlock(p1);
}

STDMETHODIMP RenderDevice::CaptureStateBlock( THIS_ DWORD p1)
{
   return dx7Device->CaptureStateBlock(p1);
}

STDMETHODIMP RenderDevice::DeleteStateBlock( THIS_ DWORD p1)
{
   return dx7Device->DeleteStateBlock(p1);
}

STDMETHODIMP RenderDevice::CreateStateBlock( THIS_ D3DSTATEBLOCKTYPE p1,LPDWORD p2)
{
   return dx7Device->CaptureStateBlock(*p2);
}

STDMETHODIMP RenderDevice::Load( THIS_ LPDIRECTDRAWSURFACE7 p1,LPPOINT p2,LPDIRECTDRAWSURFACE7 p3,LPRECT p4,DWORD p5)
{
   return dx7Device->Load(p1,p2,p3,p4,p5);
}

STDMETHODIMP RenderDevice::LightEnable( THIS_ DWORD p1,BOOL p2)
{
   return dx7Device->LightEnable(p1,p2);
}

STDMETHODIMP RenderDevice::GetLightEnable( THIS_ DWORD p1,BOOL* p2)
{
   return dx7Device->GetLightEnable(p1,p2);
}

STDMETHODIMP RenderDevice::SetClipPlane( THIS_ DWORD p1,D3DVALUE* p2)
{
   return dx7Device->SetClipPlane(p1,p2);
}

STDMETHODIMP RenderDevice::GetClipPlane( THIS_ DWORD p1,D3DVALUE* p2)
{
   return dx7Device->GetClipPlane(p1,p2);
}

STDMETHODIMP RenderDevice::GetInfo( THIS_ DWORD p1,LPVOID p2,DWORD p3 )
{
   return dx7Device->GetInfo(p1, p2, p3);
}

STDOVERRIDEMETHODIMP_(ULONG) RenderDevice::Release( ) 
{
   return dx7Device->Release();
}

STDOVERRIDEMETHODIMP_(ULONG) RenderDevice::AddRef( void ) 
{
   return dx7Device->AddRef();
}
