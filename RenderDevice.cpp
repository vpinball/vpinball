#include "stdafx.h"

#include <DxErr.h>

//#include "Dwmapi.h" // use when we get rid of XP at some point, get rid of the manual dll loads in here then

#ifndef DISABLE_FORCE_NVIDIA_OPTIMUS
#include "nvapi.h"
#endif

#include "RenderDevice.h"
#include "Material.h"
#include "BasicShader.h"
#include "DMDShader.h"
#include "FBShader.h"
#include "FlasherShader.h"
#include "LightShader.h"
#ifdef SEPARATE_CLASSICLIGHTSHADER
#include "ClassicLightShader.h"
#endif

#pragma comment(lib, "d3d9.lib")        // TODO: put into build system
#pragma comment(lib, "d3dx9.lib")       // TODO: put into build system
#if _MSC_VER >= 1900
 #pragma comment(lib, "legacy_stdio_definitions.lib") //dxerr.lib needs this
#endif
#pragma comment(lib, "dxerr.lib")       // TODO: put into build system

#if _MSC_VER <= 1700
 #define VerSetConditionMask(_m_,_t_,_c_) (_m_|(_c_<<(1<<_t_))) //!! does not work with VER_SERVICEPACKMAJOR! (see below)
#endif

static bool IsWindowsVistaOr7()
{
	OSVERSIONINFOEXW osvi = { sizeof(osvi), 0, 0, 0, 0,{ 0 }, 0, 0 };
	const DWORDLONG dwlConditionMask = //VerSetConditionMask(
		VerSetConditionMask(
			VerSetConditionMask(
				0, VER_MAJORVERSION, VER_EQUAL),
			VER_MINORVERSION, VER_EQUAL)/*,
		VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL)*/;
	osvi.dwMajorVersion = HIBYTE(_WIN32_WINNT_VISTA);
	osvi.dwMinorVersion = LOBYTE(_WIN32_WINNT_VISTA);
	//osvi.wServicePackMajor = 0;

	const bool vista = VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION /*| VER_SERVICEPACKMAJOR*/, dwlConditionMask) != FALSE;

	OSVERSIONINFOEXW osvi2 = { sizeof(osvi), 0, 0, 0, 0,{ 0 }, 0, 0 };
	osvi2.dwMajorVersion = HIBYTE(_WIN32_WINNT_WIN7);
	osvi2.dwMinorVersion = LOBYTE(_WIN32_WINNT_WIN7);
	//osvi2.wServicePackMajor = 0;

	const bool win7 = VerifyVersionInfoW(&osvi2, VER_MAJORVERSION | VER_MINORVERSION /*| VER_SERVICEPACKMAJOR*/, dwlConditionMask) != FALSE;

	return vista || win7;
}

const VertexElement VertexTexelElement[] =
{
   { 0, 0 * sizeof(float), D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },  // pos
   { 0, 3 * sizeof(float), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },  // tex0
   D3DDECL_END()
};
VertexDeclaration* RenderDevice::m_pVertexTexelDeclaration = NULL;

const VertexElement VertexNormalTexelElement[] =
{
   { 0, 0 * sizeof(float), D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },  // pos
   { 0, 3 * sizeof(float), D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },  // normal
   { 0, 6 * sizeof(float), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },  // tex0
   D3DDECL_END()
};
VertexDeclaration* RenderDevice::m_pVertexNormalTexelDeclaration = NULL;

/*const VertexElement VertexNormalTexelTexelElement[] =
{
{ 0, 0  * sizeof(float),D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },  // pos
{ 0, 3  * sizeof(float),D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0 },  // normal
{ 0, 6  * sizeof(float),D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },  // tex0
{ 0, 8  * sizeof(float),D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },  // tex1
D3DDECL_END()
};

VertexDeclaration* RenderDevice::m_pVertexNormalTexelTexelDeclaration = NULL;*/

const VertexElement VertexTrafoTexelElement[] =
{
   { 0, 0 * sizeof(float), D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITIONT, 0 }, // transformed pos
   { 0, 4 * sizeof(float), D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 }, // legacy //!! unused, just there to share same code as VertexNormalTexelElement
   { 0, 5 * sizeof(float), D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 1 }, // legacy //!! dto.
   { 0, 6 * sizeof(float), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 }, // tex0
   D3DDECL_END()
};
VertexDeclaration* RenderDevice::m_pVertexTrafoTexelDeclaration = NULL;

static unsigned int fvfToSize(const DWORD fvf)
{
   switch (fvf)
   {
   case MY_D3DFVF_NOTEX2_VERTEX:
   case MY_D3DTRANSFORMED_NOTEX2_VERTEX:
      return sizeof(Vertex3D_NoTex2);
   case MY_D3DFVF_TEX:
      return sizeof(Vertex3D_TexelOnly);
   default:
      assert(0 && "Unknown FVF type in fvfToSize");
      return 0;
   }
}

static VertexDeclaration* fvfToDecl(const DWORD fvf)
{
   switch (fvf)
   {
   case MY_D3DFVF_NOTEX2_VERTEX:
      return RenderDevice::m_pVertexNormalTexelDeclaration;
   case MY_D3DTRANSFORMED_NOTEX2_VERTEX:
      return RenderDevice::m_pVertexTrafoTexelDeclaration;
   case MY_D3DFVF_TEX:
      return RenderDevice::m_pVertexTexelDeclaration;
   default:
      assert(0 && "Unknown FVF type in fvfToDecl");
      return NULL;
   }
}

static UINT ComputePrimitiveCount(const D3DPRIMITIVETYPE type, const int vertexCount)
{
   switch (type)
   {
   case D3DPT_POINTLIST:
      return vertexCount;
   case D3DPT_LINELIST:
      return vertexCount / 2;
   case D3DPT_LINESTRIP:
      return std::max(0, vertexCount - 1);
   case D3DPT_TRIANGLELIST:
      return vertexCount / 3;
   case D3DPT_TRIANGLESTRIP:
   case D3DPT_TRIANGLEFAN:
      return std::max(0, vertexCount - 2);
   default:
      return 0;
   }
}


void ReportFatalError(const HRESULT hr, const char *file, const int line)
{
   char msg[128];
   sprintf_s(msg, 128, "Fatal error %s (0x%x: %s) at %s:%d", DXGetErrorString(hr), hr, DXGetErrorDescription(hr), file, line);
   ShowError(msg);
   exit(-1);
}

void ReportError(const char *errorText, const HRESULT hr, const char *file, const int line)
{
   char msg[128];
   sprintf_s(msg, 128, "%s %s (0x%x: %s) at %s:%d", errorText, DXGetErrorString(hr), hr, DXGetErrorDescription(hr), file, line);
   ShowError(msg);
   exit(-1);
}

static unsigned m_curLockCalls, m_frameLockCalls; //!! meh
void VertexBuffer::lock(const unsigned int offsetToLock, const unsigned int sizeToLock, void **dataBuffer, const DWORD flags)
{
    m_curLockCalls++;
    CHECKD3D(this->Lock(offsetToLock, sizeToLock, dataBuffer, flags));
}
void IndexBuffer::lock(const unsigned int offsetToLock, const unsigned int sizeToLock, void **dataBuffer, const DWORD flags)
{
    m_curLockCalls++;
    CHECKD3D(this->Lock(offsetToLock, sizeToLock, dataBuffer, flags));
}
unsigned int RenderDevice::Perf_GetNumLockCalls() const { return m_frameLockCalls; }

D3DTexture* TextureManager::LoadTexture(BaseTexture* memtex)
{
   const Iter it = m_map.find(memtex);
   if (it == m_map.end())
   {
      TexInfo texinfo;
      texinfo.d3dtex = m_rd.UploadTexture(memtex, &texinfo.texWidth, &texinfo.texHeight);
      if (!texinfo.d3dtex)
         return 0;
      texinfo.dirty = false;
      m_map[memtex] = texinfo;
      return texinfo.d3dtex;
   }
   else
   {
      if (it->second.dirty)
      {
         m_rd.UpdateTexture(it->second.d3dtex, memtex);
         it->second.dirty = false;
      }
      return it->second.d3dtex;
   }
}

void TextureManager::SetDirty(BaseTexture* memtex)
{
   const Iter it = m_map.find(memtex);
   if (it != m_map.end())
      it->second.dirty = true;
}

void TextureManager::UnloadTexture(BaseTexture* memtex)
{
   const Iter it = m_map.find(memtex);
   if (it != m_map.end())
   {
      SAFE_RELEASE(it->second.d3dtex);
      m_map.erase(it);
   }
}

void TextureManager::UnloadAll()
{
   for (Iter it = m_map.begin(); it != m_map.end(); ++it)
      SAFE_RELEASE(it->second.d3dtex);

   m_map.clear();
}

////////////////////////////////////////////////////////////////////

void EnumerateDisplayModes(const int adapter, std::vector<VideoMode>& modes)
{
   IDirect3D9 *d3d = Direct3DCreate9(D3D_SDK_VERSION);
   if (d3d == NULL)
   {
      ShowError("Could not create D3D9 object.");
      throw 0;
   }

   modes.clear();

   //for (int j = 0; j < 2; ++j)
   const int j = 0; // limit to 32bit only nowadays
   {
      const D3DFORMAT fmt = (j == 0) ? D3DFMT_X8R8G8B8 : D3DFMT_R5G6B5;
      const unsigned numModes = d3d->GetAdapterModeCount(adapter, fmt);

      for (unsigned i = 0; i < numModes; ++i)
      {
         D3DDISPLAYMODE d3dmode;
         d3d->EnumAdapterModes(adapter, fmt, i, &d3dmode);

         if (d3dmode.Width >= 640)
         {
            VideoMode mode;
            mode.width = d3dmode.Width;
            mode.height = d3dmode.Height;
            mode.depth = (fmt == D3DFMT_R5G6B5) ? 16 : 32;
            mode.refreshrate = d3dmode.RefreshRate;
            modes.push_back(mode);
         }
      }
   }

   SAFE_RELEASE(d3d);
}

////////////////////////////////////////////////////////////////////

#define CHECKNVAPI(s) { NvAPI_Status hr = (s); if (hr != NVAPI_OK) { NvAPI_ShortString ss; NvAPI_GetErrorMessage(hr,ss); MessageBox(NULL, ss, "NVAPI", MB_OK | MB_ICONEXCLAMATION); } }
static bool NVAPIinit = false; //!! meh

bool RenderDevice::m_INTZ_support = false;
bool RenderDevice::m_useNvidiaApi = false;

#ifdef USE_D3D9EX
 typedef HRESULT(WINAPI *pD3DC9Ex)(UINT SDKVersion, IDirect3D9Ex**);
 static pD3DC9Ex mDirect3DCreate9Ex = NULL;
#endif

#define DWM_EC_DISABLECOMPOSITION         0
#define DWM_EC_ENABLECOMPOSITION          1
typedef HRESULT(STDAPICALLTYPE *pDICE)(BOOL* pfEnabled);
static pDICE mDwmIsCompositionEnabled = NULL;
typedef HRESULT(STDAPICALLTYPE *pDF)();
static pDF mDwmFlush = NULL;
typedef HRESULT(STDAPICALLTYPE *pDEC)(UINT uCompositionAction);
static pDEC mDwmEnableComposition = NULL;

RenderDevice::RenderDevice(const HWND hwnd, const int width, const int height, const bool fullscreen, const int colordepth, int &refreshrate, int VSync, const bool useAA, const bool stereo3D, const unsigned int FXAA, const bool useNvidiaApi, const bool disable_dwm)
   : m_texMan(*this)
{
   m_stats_drawn_triangles = 0;

   m_useNvidiaApi = useNvidiaApi;
   m_adapter = D3DADAPTER_DEFAULT;     // for now, always use the default adapter

   mDwmIsCompositionEnabled = (pDICE)GetProcAddress(GetModuleHandle(TEXT("dwmapi.dll")), "DwmIsCompositionEnabled"); //!! remove as soon as win xp support dropped and use static link
   mDwmEnableComposition = (pDEC)GetProcAddress(GetModuleHandle(TEXT("dwmapi.dll")), "DwmEnableComposition"); //!! remove as soon as win xp support dropped and use static link
   mDwmFlush = (pDF)GetProcAddress(GetModuleHandle(TEXT("dwmapi.dll")), "DwmFlush"); //!! remove as soon as win xp support dropped and use static link

   if (mDwmIsCompositionEnabled && mDwmEnableComposition)
   {
       BOOL dwm = 0;
       mDwmIsCompositionEnabled(&dwm);
       m_dwm_enabled = m_dwm_was_enabled = !!dwm;

       if (m_dwm_was_enabled && disable_dwm && IsWindowsVistaOr7()) // windows 8 and above will not allow do disable it, but will still return S_OK
       {
           mDwmEnableComposition(DWM_EC_DISABLECOMPOSITION);
           m_dwm_enabled = false;
       }
   }
   else
   {
       m_dwm_was_enabled = false;
       m_dwm_enabled = false;
   }

#ifdef USE_D3D9EX
   m_pD3DEx = NULL;
   m_pD3DDeviceEx = NULL;

   mDirect3DCreate9Ex = (pD3DC9Ex)GetProcAddress(GetModuleHandle(TEXT("d3d9.dll")), "Direct3DCreate9Ex"); //!! remove as soon as win xp support dropped and use static link
   if (mDirect3DCreate9Ex)
   {
      const HRESULT hr = mDirect3DCreate9Ex(D3D_SDK_VERSION, &m_pD3DEx);
      if (FAILED(hr))
         ReportError("Fatal Error: unable to create D3D9Ex object!", hr, __FILE__, __LINE__);

      if (m_pD3DEx == NULL)
      {
         ShowError("Could not create D3D9Ex object.");
         throw 0;
      }
      m_pD3DEx->QueryInterface(__uuidof(IDirect3D9), reinterpret_cast<void **>(&m_pD3D));
   }
   else
#endif
   {
      m_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
      if (m_pD3D == NULL)
      {
         ShowError("Could not create D3D9 object.");
         throw 0;
      }
   }

   D3DDEVTYPE devtype = D3DDEVTYPE_HAL;

   // Look for 'NVIDIA PerfHUD' adapter
   // If it is present, override default settings
   // This only takes effect if run under NVPerfHud, otherwise does nothing
   for (UINT adapter = 0; adapter < m_pD3D->GetAdapterCount(); adapter++)
   {
      D3DADAPTER_IDENTIFIER9 Identifier;
      m_pD3D->GetAdapterIdentifier(adapter, 0, &Identifier);
      if (strstr(Identifier.Description, "PerfHUD") != 0)
      {
         m_adapter = adapter;
         devtype = D3DDEVTYPE_REF;
         break;
      }
   }

   D3DCAPS9 caps;
   m_pD3D->GetDeviceCaps(m_adapter, devtype, &caps);

   // check which parameters can be used for anisotropic filter
   m_mag_aniso = (caps.TextureFilterCaps & D3DPTFILTERCAPS_MAGFANISOTROPIC) != 0;
   m_maxaniso = caps.MaxAnisotropy;

   if (((caps.TextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL) != 0) || ((caps.TextureCaps & D3DPTEXTURECAPS_POW2) != 0))
      ShowError("D3D device does only support power of 2 textures");

   //if (caps.NumSimultaneousRTs < 2)
   //   ShowError("D3D device doesn't support multiple render targets!");

   int video10bit;
   HRESULT hr = GetRegInt("Player", "Render10Bit", &video10bit);
   if (hr != S_OK)
      video10bit = fFalse; // The default = off

   if(!fullscreen && video10bit)
   {
      ShowError("10Bit-Monitor support requires 'Force exclusive Fullscreen Mode' to be also enabled!");
      video10bit = fFalse;
   }

   // get the current display format
   D3DFORMAT format;
   if (!fullscreen)
   {
      D3DDISPLAYMODE mode;
      CHECKD3D(m_pD3D->GetAdapterDisplayMode(m_adapter, &mode));
      format = mode.Format;
      refreshrate = mode.RefreshRate;
   }
   else
   {
      format = (video10bit ? D3DFMT_A2R10G10B10 : ((colordepth == 16) ? D3DFMT_R5G6B5 : D3DFMT_X8R8G8B8));
   }

   // limit vsync rate to actual refresh rate, otherwise special handling in renderloop
   if (VSync > refreshrate)
      VSync = 0;

   D3DPRESENT_PARAMETERS params;
   params.BackBufferWidth = width;
   params.BackBufferHeight = height;
   params.BackBufferFormat = format;
   params.BackBufferCount = 1;
   params.MultiSampleType = /*useAA ? D3DMULTISAMPLE_4_SAMPLES :*/ D3DMULTISAMPLE_NONE; // D3DMULTISAMPLE_NONMASKABLE? //!! useAA now uses super sampling/offscreen render
   params.MultiSampleQuality = 0; // if D3DMULTISAMPLE_NONMASKABLE then set to > 0
   params.SwapEffect = D3DSWAPEFFECT_DISCARD;  // FLIP ?
   params.hDeviceWindow = hwnd;
   params.Windowed = !fullscreen;
   params.EnableAutoDepthStencil = FALSE;
   params.AutoDepthStencilFormat = D3DFMT_UNKNOWN;      // ignored
   params.Flags = /*fullscreen ? D3DPRESENTFLAG_LOCKABLE_BACKBUFFER :*/ /*(stereo3D ?*/ 0 /*: D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL)*/; // D3DPRESENTFLAG_LOCKABLE_BACKBUFFER only needed for SetDialogBoxMode() below, but makes rendering slower on some systems :/
   params.FullScreen_RefreshRateInHz = fullscreen ? refreshrate : 0;
#ifdef USE_D3D9EX
   params.PresentationInterval = (m_pD3DEx && (VSync != 1)) ? D3DPRESENT_INTERVAL_IMMEDIATE : (!!VSync ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE); //!! or have a special mode to force normal vsync?
#else
   params.PresentationInterval = !!VSync ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;
#endif

   // check if auto generation of mipmaps can be used, otherwise will be done via d3dx
   m_autogen_mipmap = (caps.Caps2 & D3DCAPS2_CANAUTOGENMIPMAP) != 0;
   if (m_autogen_mipmap)
      m_autogen_mipmap = (m_pD3D->CheckDeviceFormat(m_adapter, devtype, params.BackBufferFormat, D3DUSAGE_AUTOGENMIPMAP, D3DRTYPE_TEXTURE, D3DFMT_A8R8G8B8) == D3D_OK);

#ifndef DISABLE_FORCE_NVIDIA_OPTIMUS
   if (!NVAPIinit)
   {
      if (NvAPI_Initialize() == NVAPI_OK)
         NVAPIinit = true;
   }
#endif

   // Determine if INTZ is supported
   m_INTZ_support = (m_pD3D->CheckDeviceFormat( m_adapter, devtype, params.BackBufferFormat,
                    D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_TEXTURE, ((D3DFORMAT)(MAKEFOURCC('I','N','T','Z'))))) == D3D_OK;

   // check if requested MSAA is possible
   DWORD MultiSampleQualityLevels;
   if (!SUCCEEDED(m_pD3D->CheckDeviceMultiSampleType(m_adapter,
      devtype, params.BackBufferFormat,
      params.Windowed, params.MultiSampleType, &MultiSampleQualityLevels)))
   {
      ShowError("D3D device does not support this MultiSampleType");
      params.MultiSampleType = D3DMULTISAMPLE_NONE;
      params.MultiSampleQuality = 0;
   }
   else
      params.MultiSampleQuality = min(params.MultiSampleQuality, MultiSampleQualityLevels);

   const int softwareVP = GetRegIntWithDefault("Player", "SoftwareVertexProcessing", 0);
   const DWORD flags = softwareVP ? D3DCREATE_SOFTWARE_VERTEXPROCESSING : D3DCREATE_HARDWARE_VERTEXPROCESSING;

   // Create the D3D device. This optionally goes to the proper fullscreen mode.
   // It also creates the default swap chain (front and back buffer).
#ifdef USE_D3D9EX
   if (m_pD3DEx)
   {
      D3DDISPLAYMODEEX mode;
      mode.Size = sizeof(D3DDISPLAYMODEEX);
      if(fullscreen)
      {
          mode.Format = params.BackBufferFormat;
          mode.Width = params.BackBufferWidth;
          mode.Height = params.BackBufferHeight;
          mode.RefreshRate = params.FullScreen_RefreshRateInHz;
          mode.ScanLineOrdering = D3DSCANLINEORDERING_PROGRESSIVE;
      }

      CHECKD3D(m_pD3DEx->CreateDeviceEx(
         m_adapter,
         devtype,
         hwnd,
         flags /*| D3DCREATE_PUREDEVICE*/,
         &params,
         fullscreen ? &mode : NULL,
         &m_pD3DDeviceEx));

      m_pD3DDeviceEx->QueryInterface(__uuidof(IDirect3DDevice9), reinterpret_cast<void**>(&m_pD3DDevice));

      // Get the display mode so that we can report back the actual refresh rate.
      CHECKD3D(m_pD3DDeviceEx->GetDisplayModeEx(m_adapter, &mode, NULL));

      refreshrate = mode.RefreshRate;
   }
   else
#endif
   {
      HRESULT hr = m_pD3D->CreateDevice(
         m_adapter,
         devtype,
         hwnd,
         flags /*| D3DCREATE_PUREDEVICE*/,
         &params,
         &m_pD3DDevice);

      if (FAILED(hr))
         ReportError("Fatal Error: unable to create D3D device!", hr, __FILE__, __LINE__);

      // Get the display mode so that we can report back the actual refresh rate.
      D3DDISPLAYMODE mode;
      hr = m_pD3DDevice->GetDisplayMode(m_adapter, &mode);
      if (FAILED(hr))
         ReportError("Fatal Error: unable to get supported video mode list!", hr, __FILE__, __LINE__);

      refreshrate = mode.RefreshRate;
   }

   /*if(fullscreen)
       hr = m_pD3DDevice->SetDialogBoxMode(TRUE);*/ // needs D3DPRESENTFLAG_LOCKABLE_BACKBUFFER, but makes rendering slower on some systems :/

   // Retrieve a reference to the back buffer.
   hr = m_pD3DDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &m_pBackBuffer);
   if (FAILED(hr))
      ReportError("Fatal Error: unable to create back buffer!", hr, __FILE__, __LINE__);

   // alloc float buffer for rendering (optionally 2x2 res for manual super sampling)
   hr = m_pD3DDevice->CreateTexture(useAA ? 2 * width : width, useAA ? 2 * height : height, 1,
      D3DUSAGE_RENDERTARGET, D3DFMT_A16B16G16R16F, D3DPOOL_DEFAULT, &m_pOffscreenBackBufferTexture, NULL); //!! D3DFMT_A32B32G32R32F?
   if (FAILED(hr))
      ReportError("Fatal Error: unable to create render buffer!", hr, __FILE__, __LINE__);

   if(g_pplayer != NULL)
   {
       const bool drawBallReflection = ((g_pplayer->m_fReflectionForBalls && (g_pplayer->m_ptable->m_useReflectionForBalls == -1)) || (g_pplayer->m_ptable->m_useReflectionForBalls == 1));
       if(g_pplayer->m_ptable->m_fReflectElementsOnPlayfield || drawBallReflection)
       {
           hr = m_pD3DDevice->CreateTexture(useAA ? 2 * width : width, useAA ? 2 * height : height, 1,
                                            D3DUSAGE_RENDERTARGET, D3DFMT_A16B16G16R16F, D3DPOOL_DEFAULT, &m_pMirrorTmpBufferTexture, NULL); //!! D3DFMT_A32B32G32R32F?
           if(FAILED(hr))
               ReportError("Fatal Error: unable to create reflection map!", hr, __FILE__, __LINE__);
       }
   }
   // alloc bloom tex at 1/3 x 1/3 res (allows for simple HQ downscale of clipped input while saving memory)
   hr = m_pD3DDevice->CreateTexture(width / 3, height / 3, 1,
      D3DUSAGE_RENDERTARGET, D3DFMT_A16B16G16R16F, D3DPOOL_DEFAULT, &m_pBloomBufferTexture, NULL); //!! 8bit enough?
   if (FAILED(hr))
      ReportError("Fatal Error: unable to create bloom buffer!", hr, __FILE__, __LINE__);

   // temporary buffer for gaussian blur
   hr = m_pD3DDevice->CreateTexture(width / 3, height / 3, 1,
      D3DUSAGE_RENDERTARGET, D3DFMT_A16B16G16R16F, D3DPOOL_DEFAULT, &m_pBloomTmpBufferTexture, NULL); //!! 8bit are enough! //!! but used also for bulb light transmission hack now!
   if (FAILED(hr))
      ReportError("Fatal Error: unable to create blur buffer!", hr, __FILE__, __LINE__);

   // alloc temporary buffer for postprocessing
   if (stereo3D || (FXAA > 0))
   {
      hr = m_pD3DDevice->CreateTexture(width, height, 1,
         D3DUSAGE_RENDERTARGET, video10bit ? D3DFMT_A2R10G10B10 : D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pOffscreenBackBufferTmpTexture, NULL);
      if (FAILED(hr))
         ReportError("Fatal Error: unable to create stereo3D/post-processing AA buffer!", hr, __FILE__, __LINE__);
   }
   else
      m_pOffscreenBackBufferTmpTexture = NULL;

   // alloc one more temporary buffer for SMAA
   if (FXAA == Quality_SMAA)
   {
      hr = m_pD3DDevice->CreateTexture(width, height, 1,
         D3DUSAGE_RENDERTARGET, video10bit ? D3DFMT_A2R10G10B10 : D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pOffscreenBackBufferTmpTexture2, NULL);
      if (FAILED(hr))
         ReportError("Fatal Error: unable to create SMAA buffer!", hr, __FILE__, __LINE__);
   }
   else
      m_pOffscreenBackBufferTmpTexture2 = NULL;

   if (video10bit && (FXAA == Quality_SMAA || FXAA == Standard_DLAA))
      ShowError("SMAA or DLAA post-processing AA should not be combined with 10bit-output rendering (will result in visible artifacts)!");

   m_curIndexBuffer = 0;
   m_curVertexBuffer = 0;
   currentDeclaration = NULL;
   //m_curShader = NULL;

   // fill state caches with dummy values
   memset(renderStateCache, 0xCC, sizeof(DWORD)*RENDER_STATE_CACHE_SIZE);
   memset(textureStateCache, 0xCC, sizeof(DWORD) * 8 * TEXTURE_STATE_CACHE_SIZE);
   memset(textureSamplerCache, 0xCC, sizeof(DWORD) * 8 * TEXTURE_SAMPLER_CACHE_SIZE);

   // initialize performance counters
   m_curDrawCalls = m_frameDrawCalls = 0;
   m_curStateChanges = m_frameStateChanges = 0;
   m_curTextureChanges = m_frameTextureChanges = 0;
   m_curParameterChanges = m_frameParameterChanges = 0;
   m_curTextureUpdates = m_frameTextureUpdates = 0;

   m_curLockCalls = m_frameLockCalls = 0; //!! meh

   bool shaderCompilationOkay = true;

   basicShader = new Shader(this);
#if _MSC_VER >= 1700
   shaderCompilationOkay = basicShader->Load(g_basicShaderCode, sizeof(g_basicShaderCode)) && shaderCompilationOkay;
#else
   shaderCompilationOkay = basicShader->Load(basicShaderCode, sizeof(basicShaderCode)) && shaderCompilationOkay;
#endif

   DMDShader = new Shader(this);
#if _MSC_VER >= 1700
   shaderCompilationOkay = DMDShader->Load(g_dmdShaderCode, sizeof(g_dmdShaderCode)) && shaderCompilationOkay;
#else
   shaderCompilationOkay = DMDShader->Load(dmdShaderCode, sizeof(dmdShaderCode)) && shaderCompilationOkay;
#endif

   FBShader = new Shader(this);
#if _MSC_VER >= 1700
   shaderCompilationOkay = FBShader->Load(g_FBShaderCode, sizeof(g_FBShaderCode)) && shaderCompilationOkay;
#else
   shaderCompilationOkay = FBShader->Load(FBShaderCode, sizeof(FBShaderCode)) && shaderCompilationOkay;
#endif

   flasherShader = new Shader(this);
#if _MSC_VER >= 1700
   shaderCompilationOkay = flasherShader->Load(g_flasherShaderCode, sizeof(g_flasherShaderCode)) && shaderCompilationOkay;
#else
   shaderCompilationOkay = flasherShader->Load(flasherShaderCode, sizeof(flasherShaderCode)) && shaderCompilationOkay;
#endif

   lightShader = new Shader(this);
#if _MSC_VER >= 1700
   shaderCompilationOkay = lightShader->Load(g_lightShaderCode, sizeof(g_lightShaderCode)) && shaderCompilationOkay;
#else
   shaderCompilationOkay = lightShader->Load(lightShaderCode, sizeof(lightShaderCode)) && shaderCompilationOkay;
#endif

#ifdef SEPARATE_CLASSICLIGHTSHADER
   classicLightShader = new Shader(this);
#if _MSC_VER >= 1700
   shaderCompilationOkay = classicLightShader->Load(g_classicLightShaderCode, sizeof(g_classicLightShaderCode)) && shaderCompilationOkay;
#else
   shaderCompilationOkay = classicLightShader->Load(classicLightShaderCode, sizeof(classicLightShaderCode)) && shaderCompilationOkay;
#endif
#endif

   if (!shaderCompilationOkay)
      ReportError("Fatal Error: shader compilation failed!", -1, __FILE__, __LINE__);

   // create default vertex declarations for shaders
   CreateVertexDeclaration(VertexTexelElement, &m_pVertexTexelDeclaration);
   CreateVertexDeclaration(VertexNormalTexelElement, &m_pVertexNormalTexelDeclaration);
   //CreateVertexDeclaration( VertexNormalTexelTexelElement, &m_pVertexNormalTexelTexelDeclaration );
   CreateVertexDeclaration(VertexTrafoTexelElement, &m_pVertexTrafoTexelDeclaration);

   m_quadVertexBuffer = NULL;
   CreateVertexBuffer(4, 0, MY_D3DFVF_TEX, &m_quadVertexBuffer);
   Vertex3D_TexelOnly* bufvb;
   m_quadVertexBuffer->lock(0, 0, (void**)&bufvb, VertexBuffer::WRITEONLY);
   static const float verts[4 * 5] =
   {
      1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
      -1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
      1.0f, -1.0f, 0.0f, 1.0f, 1.0f,
      -1.0f, -1.0f, 0.0f, 0.0f, 1.0f
   };
   memcpy(bufvb,verts,4*sizeof(Vertex3D_TexelOnly));
   m_quadVertexBuffer->unlock();

   //m_quadDynVertexBuffer = NULL;
   //CreateVertexBuffer(4, USAGE_DYNAMIC, MY_D3DFVF_TEX, &m_quadDynVertexBuffer);

   if(FXAA == Quality_SMAA)
       UploadAndSetSMAATextures();
   else
   {
       m_SMAAareaTexture = 0;
       m_SMAAsearchTexture = 0;
   }
}

bool RenderDevice::DepthBufferReadBackAvailable()
{
    if (m_INTZ_support && !m_useNvidiaApi)
        return true;
    // fall back to NVIDIAs NVAPI, only handle AO if API was initialized
    return NVAPIinit;
}

#ifdef _DEBUG
static void CheckForD3DLeak(IDirect3DDevice9* d3d)
{
   IDirect3DSwapChain9 *swapChain;
   CHECKD3D(d3d->GetSwapChain(0, &swapChain));

   D3DPRESENT_PARAMETERS pp;
   CHECKD3D(swapChain->GetPresentParameters(&pp));
   SAFE_RELEASE(swapChain);

   // idea: device can't be reset if there are still allocated resources
   HRESULT hr = d3d->Reset(&pp);
   if (FAILED(hr))
   {
      MessageBox(0, "WARNING! Direct3D resource leak detected!", "Visual Pinball", MB_ICONWARNING);
   }
}
#endif


static RenderTarget *srcr_cache = NULL; //!! meh, for nvidia depth read only
static D3DTexture *srct_cache = NULL;
static D3DTexture* dest_cache = NULL;

void RenderDevice::FreeShader()
{
   if (basicShader)
   {
      CHECKD3D(basicShader->Core()->SetTexture("Texture0", NULL));
      CHECKD3D(basicShader->Core()->SetTexture("Texture1", NULL));
      CHECKD3D(basicShader->Core()->SetTexture("Texture2", NULL));
      CHECKD3D(basicShader->Core()->SetTexture("Texture3", NULL));
      CHECKD3D(basicShader->Core()->SetTexture("Texture4", NULL));
      delete basicShader;
      basicShader = 0;
   }
   if (DMDShader)
   {
      CHECKD3D(DMDShader->Core()->SetTexture("Texture0", NULL));
      delete DMDShader;
      DMDShader = 0;
   }
   if (FBShader)
   {
      CHECKD3D(FBShader->Core()->SetTexture("Texture0", NULL));
      CHECKD3D(FBShader->Core()->SetTexture("Texture1", NULL));
      CHECKD3D(FBShader->Core()->SetTexture("Texture3", NULL));
      CHECKD3D(FBShader->Core()->SetTexture("Texture4", NULL));

      CHECKD3D(FBShader->Core()->SetTexture("areaTex2D", NULL));
      CHECKD3D(FBShader->Core()->SetTexture("searchTex2D", NULL));

      delete FBShader;
      FBShader = 0;
   }
   if (flasherShader)
   {
      CHECKD3D(flasherShader->Core()->SetTexture("Texture0", NULL));
      CHECKD3D(flasherShader->Core()->SetTexture("Texture1", NULL));
      delete flasherShader;
      flasherShader = 0;
   }
   if (lightShader)
   {
      delete lightShader;
      lightShader = 0;
   }
#ifdef SEPARATE_CLASSICLIGHTSHADER
   if (classicLightShader)
   {
      CHECKD3D(classicLightShader->Core()->SetTexture("Texture0",NULL));
      CHECKD3D(classicLightShader->Core()->SetTexture("Texture1",NULL));
      CHECKD3D(classicLightShader->Core()->SetTexture("Texture2",NULL));
      delete classicLightShader;
      classicLightShader=0;
   }
#endif
}

RenderDevice::~RenderDevice()
{
   m_quadVertexBuffer->release();
   //m_quadDynVertexBuffer->release();

#ifndef DISABLE_FORCE_NVIDIA_OPTIMUS
   if (srcr_cache != NULL)
      CHECKNVAPI(NvAPI_D3D9_UnregisterResource(srcr_cache)); //!! meh
   srcr_cache = NULL;
   if (srct_cache != NULL)
      CHECKNVAPI(NvAPI_D3D9_UnregisterResource(srct_cache)); //!! meh
   srct_cache = NULL;
   if (dest_cache != NULL)
      CHECKNVAPI(NvAPI_D3D9_UnregisterResource(dest_cache)); //!! meh
   dest_cache = NULL;
   if (NVAPIinit) //!! meh
      CHECKNVAPI(NvAPI_Unload());
   NVAPIinit = false;
#endif

   //
   m_pD3DDevice->SetStreamSource(0, NULL, 0, 0);
   m_pD3DDevice->SetIndices(NULL);
   m_pD3DDevice->SetVertexShader(NULL);
   m_pD3DDevice->SetPixelShader(NULL);
   m_pD3DDevice->SetFVF(D3DFVF_XYZ);
   //m_pD3DDevice->SetVertexDeclaration(NULL); // invalid call
   //m_pD3DDevice->SetRenderTarget(0, NULL); // invalid call
   m_pD3DDevice->SetDepthStencilSurface(NULL);

   FreeShader();

   SAFE_RELEASE(m_pVertexTexelDeclaration);
   SAFE_RELEASE(m_pVertexNormalTexelDeclaration);
   //SAFE_RELEASE(m_pVertexNormalTexelTexelDeclaration);
   SAFE_RELEASE(m_pVertexTrafoTexelDeclaration);

   m_texMan.UnloadAll();
   SAFE_RELEASE(m_pOffscreenBackBufferTexture);
   SAFE_RELEASE(m_pOffscreenBackBufferTmpTexture);
   SAFE_RELEASE(m_pOffscreenBackBufferTmpTexture2);

   if(g_pplayer)
   {
       const bool drawBallReflection = ((g_pplayer->m_fReflectionForBalls && (g_pplayer->m_ptable->m_useReflectionForBalls == -1)) || (g_pplayer->m_ptable->m_useReflectionForBalls == 1));
       if(g_pplayer->m_ptable->m_fReflectElementsOnPlayfield || drawBallReflection)
           SAFE_RELEASE(m_pMirrorTmpBufferTexture);
   }
   SAFE_RELEASE(m_pBloomBufferTexture);
   SAFE_RELEASE(m_pBloomTmpBufferTexture);
   SAFE_RELEASE(m_pBackBuffer);

   SAFE_RELEASE(m_SMAAareaTexture);
   SAFE_RELEASE(m_SMAAsearchTexture);

#ifdef _DEBUG
   CheckForD3DLeak(m_pD3DDevice);
#endif

#ifdef USE_D3D9EX
   SAFE_RELEASE_NO_RCC(m_pD3DDeviceEx);
#endif
   SAFE_RELEASE(m_pD3DDevice);
#ifdef USE_D3D9EX
   SAFE_RELEASE_NO_RCC(m_pD3DEx);
#endif
   SAFE_RELEASE(m_pD3D);

   /*
    * D3D sets the FPU to single precision/round to nearest int mode when it's initialized,
    * but doesn't bother to reset the FPU when it's destroyed. We reset it manually here.
    */
   _fpreset();

   if(m_dwm_was_enabled)
      mDwmEnableComposition(DWM_EC_ENABLECOMPOSITION);
}

void RenderDevice::BeginScene()
{
   CHECKD3D(m_pD3DDevice->BeginScene());
}

void RenderDevice::EndScene()
{
   CHECKD3D(m_pD3DDevice->EndScene());
}

static void FlushGPUCommandBuffer(IDirect3DDevice9* pd3dDevice)
{
   IDirect3DQuery9* pEventQuery;
   pd3dDevice->CreateQuery(D3DQUERYTYPE_EVENT, &pEventQuery);

   if (pEventQuery)
   {
      pEventQuery->Issue(D3DISSUE_END);
      while (S_FALSE == pEventQuery->GetData(NULL, 0, D3DGETDATA_FLUSH))
         ;
      SAFE_RELEASE(pEventQuery);
   }
}

bool RenderDevice::SetMaximumPreRenderedFrames(const DWORD frames)
{
#ifdef USE_D3D9EX
	if (m_pD3DEx && frames > 0 && frames <= 20) // frames can range from 1 to 20, 0 resets to default DX
	{
		CHECKD3D(m_pD3DDeviceEx->SetMaximumFrameLatency(frames));
		return true;
	}
	else
#endif
	return false;
}

void RenderDevice::Flip(const bool vsync)
{
   BOOL dwm = 0;
   if (vsync) // xp does neither have d3dex nor dwm, so vsync will always be specified during device set
      dwm = m_dwm_enabled;

#ifdef USE_D3D9EX
   if (m_pD3DEx && vsync && !dwm)
   {
      m_pD3DDeviceEx->WaitForVBlank(0); //!! does not seem to work on win8?? -> may depend on desktop compositing and the like
      /*D3DRASTER_STATUS r;
      CHECKD3D(m_pD3DDevice->GetRasterStatus(0, &r)); // usually not supported, also only for pure devices?!

      while (!r.InVBlank)
      {
      uSleep(10);
      m_pD3DDevice->GetRasterStatus(0, &r);
      }*/
   }
#endif

   CHECKD3D(m_pD3DDevice->Present(NULL, NULL, NULL, NULL)); //!! could use D3DPRESENT_DONOTWAIT and do some physics work meanwhile??

   if (mDwmFlush && vsync && dwm)
      mDwmFlush(); //!! also above present?? (internet sources are not clear about order)

   // reset performance counters
   m_frameDrawCalls = m_curDrawCalls;
   m_frameStateChanges = m_curStateChanges;
   m_frameTextureChanges = m_curTextureChanges;
   m_frameParameterChanges = m_curParameterChanges;
   m_curDrawCalls = m_curStateChanges = m_curTextureChanges = m_curParameterChanges = 0;
   m_frameTextureUpdates = m_curTextureUpdates;
   m_curTextureUpdates = 0;

   m_frameLockCalls = m_curLockCalls;
   m_curLockCalls = 0;
}

RenderTarget* RenderDevice::DuplicateRenderTarget(RenderTarget* src)
{
   D3DSURFACE_DESC desc;
   src->GetDesc(&desc);
   IDirect3DSurface9 *dup;
   CHECKD3D(m_pD3DDevice->CreateRenderTarget(desc.Width, desc.Height, desc.Format,
      desc.MultiSampleType, desc.MultiSampleQuality, FALSE /* lockable */, &dup, NULL));
   return dup;
}

void RenderDevice::CopySurface(RenderTarget* dest, RenderTarget* src)
{
   CHECKD3D(m_pD3DDevice->StretchRect(src, NULL, dest, NULL, D3DTEXF_NONE));
}

D3DTexture* RenderDevice::DuplicateTexture(RenderTarget* src)
{
   D3DSURFACE_DESC desc;
   src->GetDesc(&desc);
   D3DTexture* dup;
   CHECKD3D(m_pD3DDevice->CreateTexture(desc.Width, desc.Height, 1,
      D3DUSAGE_RENDERTARGET, desc.Format, D3DPOOL_DEFAULT, &dup, NULL)); // D3DUSAGE_AUTOGENMIPMAP?
   return dup;
}

D3DTexture* RenderDevice::DuplicateTextureSingleChannel(RenderTarget* src)
{
   D3DSURFACE_DESC desc;
   src->GetDesc(&desc);
   desc.Format = D3DFMT_L8;
   D3DTexture* dup;
   CHECKD3D(m_pD3DDevice->CreateTexture(desc.Width, desc.Height, 1,
      D3DUSAGE_RENDERTARGET, desc.Format, D3DPOOL_DEFAULT, &dup, NULL)); // D3DUSAGE_AUTOGENMIPMAP?
   return dup;
}

D3DTexture* RenderDevice::DuplicateDepthTexture(RenderTarget* src)
{
   D3DSURFACE_DESC desc;
   src->GetDesc(&desc);
   D3DTexture* dup;
   CHECKD3D(m_pD3DDevice->CreateTexture(desc.Width, desc.Height, 1,
      D3DUSAGE_DEPTHSTENCIL, (D3DFORMAT)MAKEFOURCC('I', 'N', 'T', 'Z'), D3DPOOL_DEFAULT, &dup, NULL)); // D3DUSAGE_AUTOGENMIPMAP?
   return dup;
}

void RenderDevice::CopySurface(D3DTexture* dest, RenderTarget* src)
{
   IDirect3DSurface9 *textureSurface;
   CHECKD3D(dest->GetSurfaceLevel(0, &textureSurface));
   CHECKD3D(m_pD3DDevice->StretchRect(src, NULL, textureSurface, NULL, D3DTEXF_NONE));
   SAFE_RELEASE_NO_RCC(textureSurface);
}

void RenderDevice::CopySurface(RenderTarget* dest, D3DTexture* src)
{
   IDirect3DSurface9 *textureSurface;
   CHECKD3D(src->GetSurfaceLevel(0, &textureSurface));
   CHECKD3D(m_pD3DDevice->StretchRect(textureSurface, NULL, dest, NULL, D3DTEXF_NONE));
   SAFE_RELEASE_NO_RCC(textureSurface);
}

void RenderDevice::CopySurface(void* dest, void* src)
{
   if (!m_useNvidiaApi && m_INTZ_support)
      CopySurface((D3DTexture*)dest, (D3DTexture*)src);
   else
      CopySurface((RenderTarget*)dest, (RenderTarget*)src);
}

void RenderDevice::CopySurface(D3DTexture* dest, D3DTexture* src)
{
   IDirect3DSurface9 *destTextureSurface;
   CHECKD3D(dest->GetSurfaceLevel(0, &destTextureSurface));
   IDirect3DSurface9 *srcTextureSurface;
   CHECKD3D(src->GetSurfaceLevel(0, &srcTextureSurface));
   const HRESULT hr = m_pD3DDevice->StretchRect(srcTextureSurface, NULL, destTextureSurface, NULL, D3DTEXF_NONE);
   if (FAILED(hr))
   {
       ShowError("Unable to access texture surface!\r\nTry to set \"Alternative Depth Buffer processing\" in the video options!\r\nOr disable Ambient Occlusion and/or 3D stereo!");
   }
   SAFE_RELEASE_NO_RCC(destTextureSurface);
   SAFE_RELEASE_NO_RCC(srcTextureSurface);
}


void RenderDevice::CopyDepth(D3DTexture* dest, RenderTarget* src)
{
#ifndef DISABLE_FORCE_NVIDIA_OPTIMUS
   if (NVAPIinit)
   {
      if (src != srcr_cache)
      {
         if (srcr_cache != NULL)
            CHECKNVAPI(NvAPI_D3D9_UnregisterResource(srcr_cache)); //!! meh
         CHECKNVAPI(NvAPI_D3D9_RegisterResource(src)); //!! meh
         srcr_cache = src;
      }
      if (dest != dest_cache)
      {
         if (dest_cache != NULL)
            CHECKNVAPI(NvAPI_D3D9_UnregisterResource(dest_cache)); //!! meh
         CHECKNVAPI(NvAPI_D3D9_RegisterResource(dest)); //!! meh
         dest_cache = dest;
      }

      //CHECKNVAPI(NvAPI_D3D9_AliasSurfaceAsTexture(m_pD3DDevice,src,dest,0));
      CHECKNVAPI(NvAPI_D3D9_StretchRectEx(m_pD3DDevice, src, NULL, dest, NULL, D3DTEXF_NONE));
   }
#endif
#if 0 // leftover resolve z code, maybe useful later-on
   else //if(m_RESZ_support)
   {
#define RESZ_CODE 0x7FA05000
      IDirect3DSurface9 *pDSTSurface;
      m_pD3DDevice->GetDepthStencilSurface(&pDSTSurface);
      IDirect3DSurface9 *pINTZDSTSurface;
      dest->GetSurfaceLevel(0, &pINTZDSTSurface);
      // Bind depth buffer
      m_pD3DDevice->SetDepthStencilSurface(pINTZDSTSurface);

      m_pD3DDevice->BeginScene();

      m_pD3DDevice->SetVertexShader(NULL);
      m_pD3DDevice->SetPixelShader(NULL);
      m_pD3DDevice->SetFVF(D3DFVF_XYZ);

      // Bind depth stencil texture to texture sampler 0
      m_pD3DDevice->SetTexture(0, dest);

      // Perform a dummy draw call to ensure texture sampler 0 is set before the resolve is triggered
      // Vertex declaration and shaders may need to me adjusted to ensure no debug
      // error message is produced
      m_pD3DDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
      m_pD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
      m_pD3DDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0);
      D3DXVECTOR3 vDummyPoint(0.0f, 0.0f, 0.0f);
      m_pD3DDevice->DrawPrimitiveUP(D3DPT_POINTLIST, 1, vDummyPoint, sizeof(D3DXVECTOR3));
      m_pD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
      m_pD3DDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
      m_pD3DDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0x0F);

      // Trigger the depth buffer resolve; after this call texture sampler 0
      // will contain the contents of the resolve operation
      m_pD3DDevice->SetRenderState(D3DRS_POINTSIZE, RESZ_CODE);

      // This hack to fix resz hack, has been found by Maksym Bezus!!!
      // Without this line resz will be resolved only for first frame
      m_pD3DDevice->SetRenderState(D3DRS_POINTSIZE, 0); // TROLOLO!!!

      m_pD3DDevice->EndScene();

      m_pD3DDevice->SetDepthStencilSurface(pDSTSurface);
      SAFE_RELEASE_NO_RCC(pINTZDSTSurface);
      SAFE_RELEASE(pDSTSurface);
   }
#endif
}

void RenderDevice::CopyDepth(D3DTexture* dest, D3DTexture* src)
{
   if (!m_useNvidiaApi)
      CopySurface(dest, src); // if INTZ used as texture format this (usually) works, although not really specified somewhere
#ifndef DISABLE_FORCE_NVIDIA_OPTIMUS
   else if (NVAPIinit)
   {
      if (src != srct_cache)
      {
         if (srct_cache != NULL)
            CHECKNVAPI(NvAPI_D3D9_UnregisterResource(srct_cache)); //!! meh
         CHECKNVAPI(NvAPI_D3D9_RegisterResource(src)); //!! meh
         srct_cache = src;
      }
      if (dest != dest_cache)
      {
         if (dest_cache != NULL)
            CHECKNVAPI(NvAPI_D3D9_UnregisterResource(dest_cache)); //!! meh
         CHECKNVAPI(NvAPI_D3D9_RegisterResource(dest)); //!! meh
         dest_cache = dest;
      }

      //CHECKNVAPI(NvAPI_D3D9_AliasSurfaceAsTexture(m_pD3DDevice,src,dest,0));
      CHECKNVAPI(NvAPI_D3D9_StretchRectEx(m_pD3DDevice, src, NULL, dest, NULL, D3DTEXF_NONE));
   }
#endif
#if 0 // leftover manual pixel shader texture copy
   BeginScene(); //!!

   IDirect3DSurface9 *oldRT;
   CHECKD3D(m_pD3DDevice->GetRenderTarget(0, &oldRT));

   IDirect3DSurface9 *destTextureSurface;
   CHECKD3D(dest->GetSurfaceLevel(0, &destTextureSurface));
   SetRenderTarget(destTextureSurface);

   FBShader->SetTexture("Texture0", src);
   FBShader->SetFloat("mirrorFactor", 1.f); //!! use separate pass-through shader instead??
   FBShader->SetTechnique("fb_mirror");

   SetRenderState(RenderDevice::ALPHABLENDENABLE, FALSE); // paranoia set //!!
   SetRenderState(RenderDevice::CULLMODE, D3DCULL_NONE);
   SetRenderState(RenderDevice::ZWRITEENABLE, FALSE);
   SetRenderState(RenderDevice::ZENABLE, FALSE);

   FBShader->Begin(0);
   DrawFullscreenQuad();
   FBShader->End();

   SetRenderTarget(oldRT);
   SAFE_RELEASE_NO_RCC(oldRT);
   SAFE_RELEASE_NO_RCC(destTextureSurface);

   EndScene(); //!!
#endif
}

void RenderDevice::CopyDepth(D3DTexture* dest, void* src)
{
   if (!m_useNvidiaApi && m_INTZ_support)
      CopyDepth(dest, (D3DTexture*)src);
   else
      CopyDepth(dest, (RenderTarget*)src);
}

D3DTexture* RenderDevice::CreateSystemTexture(BaseTexture* surf)
{
   const int texwidth = surf->width();
   const int texheight = surf->height();
   const BaseTexture::Format basetexformat = surf->m_format;

   const D3DFORMAT texformat = (m_compress_textures && ((texwidth & 3) == 0) && ((texheight & 3) == 0) && (texwidth > 256) && (texheight > 256) && (basetexformat != BaseTexture::RGB_FP)) ? D3DFMT_DXT5 : ((basetexformat == BaseTexture::RGB_FP) ? D3DFMT_A32B32G32R32F : D3DFMT_A8R8G8B8);

   IDirect3DTexture9 *sysTex;
   HRESULT hr;
   hr = m_pD3DDevice->CreateTexture(texwidth, texheight, (texformat != D3DFMT_DXT5 && m_autogen_mipmap) ? 1 : 0, 0, texformat, D3DPOOL_SYSTEMMEM, &sysTex, NULL);
   if (FAILED(hr))
   {
      ReportError("Fatal Error: unable to create texture!", hr, __FILE__, __LINE__);
   }

   // copy data into system memory texture
   if (texformat == D3DFMT_A32B32G32R32F)
   {
      D3DLOCKED_RECT locked;
      CHECKD3D(sysTex->LockRect(0, &locked, NULL, 0));

      // old RGBA copy code, just for reference:
      //BYTE *pdest = (BYTE*)locked.pBits;
      //for (int y = 0; y < surf->height(); ++y)
      //   memcpy(pdest + y*locked.Pitch, surf->data() + y*surf->pitch(), 4 * surf->width());

      float * const pdest = (float*)locked.pBits;
      float * const psrc = (float*)surf->data();
      for (int i = 0; i < surf->width()*surf->height(); ++i)
      {
         pdest[i * 4    ] = psrc[i * 3    ];
         pdest[i * 4 + 1] = psrc[i * 3 + 1];
         pdest[i * 4 + 2] = psrc[i * 3 + 2];
         pdest[i * 4 + 3] = 1.f;
      }

      CHECKD3D(sysTex->UnlockRect(0));
   }
   else
   {
      IDirect3DSurface9* sysSurf;
      CHECKD3D(sysTex->GetSurfaceLevel(0, &sysSurf));
      RECT sysRect;
      sysRect.top = 0;
      sysRect.left = 0;
      sysRect.right = texwidth;
      sysRect.bottom = texheight;
      CHECKD3D(D3DXLoadSurfaceFromMemory(sysSurf, NULL, NULL, surf->data(), D3DFMT_A8R8G8B8, surf->pitch(), NULL, &sysRect, D3DX_FILTER_NONE, 0));
      SAFE_RELEASE_NO_RCC(sysSurf);
   }

   if (!(texformat != D3DFMT_DXT5 && m_autogen_mipmap))
      CHECKD3D(D3DXFilterTexture(sysTex, NULL, D3DX_DEFAULT, D3DX_DEFAULT)); //!! D3DX_FILTER_SRGB

   return sysTex;
}

D3DTexture* RenderDevice::UploadTexture(BaseTexture* surf, int *pTexWidth, int *pTexHeight)
{
   IDirect3DTexture9 *sysTex, *tex;
   HRESULT hr;

   int texwidth = surf->width();
   int texheight = surf->height();

   if (pTexWidth) *pTexWidth = texwidth;
   if (pTexHeight) *pTexHeight = texheight;

   const BaseTexture::Format basetexformat = surf->m_format;

   sysTex = CreateSystemTexture(surf);

   const D3DFORMAT texformat = (m_compress_textures && ((texwidth & 3) == 0) && ((texheight & 3) == 0) && (texwidth > 256) && (texheight > 256) && (basetexformat != BaseTexture::RGB_FP)) ? D3DFMT_DXT5 : ((basetexformat == BaseTexture::RGB_FP) ? D3DFMT_A32B32G32R32F : D3DFMT_A8R8G8B8);

   hr = m_pD3DDevice->CreateTexture(texwidth, texheight, (texformat != D3DFMT_DXT5 && m_autogen_mipmap) ? 0 : sysTex->GetLevelCount(), (texformat != D3DFMT_DXT5 && m_autogen_mipmap) ? D3DUSAGE_AUTOGENMIPMAP : 0, texformat, D3DPOOL_DEFAULT, &tex, NULL);
   if (FAILED(hr))
      ReportError("Fatal Error: out of VRAM!", hr, __FILE__, __LINE__);

   m_curTextureUpdates++;
   hr = m_pD3DDevice->UpdateTexture(sysTex, tex);
   if (FAILED(hr))
      ReportError("Fatal Error: uploading texture failed!", hr, __FILE__, __LINE__);

   SAFE_RELEASE(sysTex);

   if (texformat != D3DFMT_DXT5 && m_autogen_mipmap)
      tex->GenerateMipSubLevels(); // tell driver that now is a good time to generate mipmaps

   return tex;
}

#include "shader/AreaTex.h"
#include "shader/SearchTex.h"
void RenderDevice::UploadAndSetSMAATextures()
{
   {
   IDirect3DTexture9 *sysTex;
   HRESULT hr = m_pD3DDevice->CreateTexture(SEARCHTEX_WIDTH, SEARCHTEX_HEIGHT, 0, 0, D3DFMT_L8, D3DPOOL_SYSTEMMEM, &sysTex, NULL);
   if (FAILED(hr))
      ReportError("Fatal Error: unable to create texture!", hr, __FILE__, __LINE__);
   hr = m_pD3DDevice->CreateTexture(SEARCHTEX_WIDTH, SEARCHTEX_HEIGHT, 0, 0, D3DFMT_L8, D3DPOOL_DEFAULT, &m_SMAAsearchTexture, NULL);
   if (FAILED(hr))
      ReportError("Fatal Error: out of VRAM!", hr, __FILE__, __LINE__);

   //!! use D3DXLoadSurfaceFromMemory
   D3DLOCKED_RECT locked;
   CHECKD3D(sysTex->LockRect(0, &locked, NULL, 0));
   void * const pdest = locked.pBits;
   const void * const psrc = searchTexBytes;
   memcpy(pdest,psrc,SEARCHTEX_SIZE);
   CHECKD3D(sysTex->UnlockRect(0));

   CHECKD3D(m_pD3DDevice->UpdateTexture(sysTex, m_SMAAsearchTexture));
   SAFE_RELEASE(sysTex);
   }
   //
   {
   IDirect3DTexture9 *sysTex;
   HRESULT hr = m_pD3DDevice->CreateTexture(AREATEX_WIDTH, AREATEX_HEIGHT, 0, 0, D3DFMT_A8L8, D3DPOOL_SYSTEMMEM, &sysTex, NULL);
   if (FAILED(hr))
      ReportError("Fatal Error: unable to create texture!", hr, __FILE__, __LINE__);
   hr = m_pD3DDevice->CreateTexture(AREATEX_WIDTH, AREATEX_HEIGHT, 0, 0, D3DFMT_A8L8, D3DPOOL_DEFAULT, &m_SMAAareaTexture, NULL);
   if (FAILED(hr))
      ReportError("Fatal Error: out of VRAM!", hr, __FILE__, __LINE__);

   //!! use D3DXLoadSurfaceFromMemory
   D3DLOCKED_RECT locked;
   CHECKD3D(sysTex->LockRect(0, &locked, NULL, 0));
   void * const pdest = locked.pBits;
   const void * const psrc = areaTexBytes;
   memcpy(pdest,psrc,AREATEX_SIZE);
   CHECKD3D(sysTex->UnlockRect(0));

   CHECKD3D(m_pD3DDevice->UpdateTexture(sysTex, m_SMAAareaTexture));
   SAFE_RELEASE(sysTex);
   }

   //

   CHECKD3D(FBShader->Core()->SetTexture("areaTex2D", m_SMAAareaTexture));
   CHECKD3D(FBShader->Core()->SetTexture("searchTex2D", m_SMAAsearchTexture));
}

void RenderDevice::UpdateTexture(D3DTexture* tex, BaseTexture* surf)
{
   IDirect3DTexture9* sysTex = CreateSystemTexture(surf);
   m_curTextureUpdates++;
   CHECKD3D(m_pD3DDevice->UpdateTexture(sysTex, tex));
   SAFE_RELEASE(sysTex);
}

void RenderDevice::SetSamplerState(const DWORD Sampler, const D3DSAMPLERSTATETYPE Type, const DWORD Value)
{
   if (textureSamplerCache[Sampler][Type] != Value)
   {
      CHECKD3D(m_pD3DDevice->SetSamplerState(Sampler, Type, Value));
      textureSamplerCache[Sampler][Type] = Value;

      m_curStateChanges++;
   }
}

void RenderDevice::SetTextureFilter(const DWORD texUnit, DWORD mode)
{
   if ((mode == TEXTURE_MODE_TRILINEAR) && m_force_aniso)
      mode = TEXTURE_MODE_ANISOTROPIC;

   switch (mode)
   {
   default:
   case TEXTURE_MODE_POINT:
      // Don't filter textures, no mipmapping.
      SetSamplerState(texUnit, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
      SetSamplerState(texUnit, D3DSAMP_MINFILTER, D3DTEXF_POINT);
      SetSamplerState(texUnit, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
      break;

   case TEXTURE_MODE_BILINEAR:
      // Interpolate in 2x2 texels, no mipmapping.
      SetSamplerState(texUnit, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
      SetSamplerState(texUnit, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
      SetSamplerState(texUnit, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
      break;

   case TEXTURE_MODE_TRILINEAR:
      // Filter textures on 2 mip levels (interpolate in 2x2 texels). And filter between the 2 mip levels.
      SetSamplerState(texUnit, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
      SetSamplerState(texUnit, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
      SetSamplerState(texUnit, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
      break;

   case TEXTURE_MODE_ANISOTROPIC:
      // Full HQ anisotropic Filter. Should lead to driver doing whatever it thinks is best.
      SetSamplerState(texUnit, D3DSAMP_MAGFILTER, m_mag_aniso ? D3DTEXF_ANISOTROPIC : D3DTEXF_LINEAR);
      SetSamplerState(texUnit, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
      SetSamplerState(texUnit, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
      SetSamplerState(texUnit, D3DSAMP_MAXANISOTROPY, min(m_maxaniso, (DWORD)16));
      break;
   }
}

void RenderDevice::SetTextureStageState(const DWORD p1, const D3DTEXTURESTAGESTATETYPE p2, const DWORD p3)
{
   if ((unsigned int)p2 < TEXTURE_STATE_CACHE_SIZE && p1 < 8)
   {
      if (textureStateCache[p1][p2] == p3)
      {
         // texture stage state hasn't changed since last call of this function -> do nothing here
         return;
      }
      textureStateCache[p1][p2] = p3;
   }
   CHECKD3D(m_pD3DDevice->SetTextureStageState(p1, p2, p3));

   m_curStateChanges++;
}

void RenderDevice::SetRenderTarget(RenderTarget* surf)
{
   CHECKD3D(m_pD3DDevice->SetRenderTarget(0, surf));
}

void RenderDevice::SetZBuffer(RenderTarget* surf)
{
   CHECKD3D(m_pD3DDevice->SetDepthStencilSurface(surf));
}

void RenderDevice::SetRenderState(const RenderStates p1, DWORD p2)
{
   if ((unsigned int)p1 < RENDER_STATE_CACHE_SIZE)
   {
      if (renderStateCache[p1] == p2)
      {
         // this render state is already set -> don't do anything then
         return;
      }
      renderStateCache[p1] = p2;
   }

   if (p1 == CULLMODE && (g_pplayer && (g_pplayer->m_ptable->m_tblMirrorEnabled ^ g_pplayer->m_ptable->m_fReflectionEnabled)))
   {
      if (p2 == D3DCULL_CCW)
         p2 = D3DCULL_CW;
      else if (p2 == D3DCULL_CW)
         p2 = D3DCULL_CCW;
   }

   CHECKD3D(m_pD3DDevice->SetRenderState((D3DRENDERSTATETYPE)p1, p2));

   m_curStateChanges++;
}

void RenderDevice::SetTextureAddressMode(const DWORD texUnit, const TextureAddressMode mode)
{
   SetSamplerState(texUnit, D3DSAMP_ADDRESSU, mode);
   SetSamplerState(texUnit, D3DSAMP_ADDRESSV, mode);
}

void RenderDevice::CreateVertexBuffer(const unsigned int vertexCount, const DWORD usage, const DWORD fvf, VertexBuffer **vBuffer)
{
   // NB: We always specify WRITEONLY since MSDN states,
   // "Buffers created with D3DPOOL_DEFAULT that do not specify D3DUSAGE_WRITEONLY may suffer a severe performance penalty."
   // This means we cannot read from vertex buffers, but I don't think we need to.
   HRESULT hr;
   hr = m_pD3DDevice->CreateVertexBuffer(vertexCount * fvfToSize(fvf), D3DUSAGE_WRITEONLY | usage, 0,
      D3DPOOL_DEFAULT, (IDirect3DVertexBuffer9**)vBuffer, NULL);
   if (FAILED(hr))
      ReportError("Fatal Error: unable to create vertex buffer!", hr, __FILE__, __LINE__);
}

void RenderDevice::CreateIndexBuffer(const unsigned int numIndices, const DWORD usage, const IndexBuffer::Format format, IndexBuffer **idxBuffer)
{
   // NB: We always specify WRITEONLY since MSDN states,
   // "Buffers created with D3DPOOL_DEFAULT that do not specify D3DUSAGE_WRITEONLY may suffer a severe performance penalty."
   HRESULT hr;
   const unsigned idxSize = (format == IndexBuffer::FMT_INDEX16) ? 2 : 4;
   hr = m_pD3DDevice->CreateIndexBuffer(idxSize * numIndices, usage | D3DUSAGE_WRITEONLY, (D3DFORMAT)format,
      D3DPOOL_DEFAULT, (IDirect3DIndexBuffer9**)idxBuffer, NULL);
   if (FAILED(hr))
      ReportError("Fatal Error: unable to create index buffer!", hr, __FILE__, __LINE__);
}

IndexBuffer* RenderDevice::CreateAndFillIndexBuffer(const unsigned int numIndices, const WORD * indices)
{
   IndexBuffer* ib;
   CreateIndexBuffer(numIndices, 0, IndexBuffer::FMT_INDEX16, &ib);

   void* buf;
   ib->lock(0, 0, &buf, 0);
   memcpy(buf, indices, numIndices * sizeof(indices[0]));
   ib->unlock();

   return ib;
}

IndexBuffer* RenderDevice::CreateAndFillIndexBuffer(const unsigned int numIndices, const unsigned int * indices)
{
   IndexBuffer* ib;
   CreateIndexBuffer(numIndices, 0, IndexBuffer::FMT_INDEX32, &ib);

   void* buf;
   ib->lock(0, 0, &buf, 0);
   memcpy(buf, indices, numIndices * sizeof(indices[0]));
   ib->unlock();

   return ib;
}

IndexBuffer* RenderDevice::CreateAndFillIndexBuffer(const std::vector<WORD>& indices)
{
   return CreateAndFillIndexBuffer((unsigned int)indices.size(), indices.data());
}

IndexBuffer* RenderDevice::CreateAndFillIndexBuffer(const std::vector<unsigned int>& indices)
{
   return CreateAndFillIndexBuffer((unsigned int)indices.size(), indices.data());
}


void* RenderDevice::AttachZBufferTo(RenderTarget* surf)
{
   D3DSURFACE_DESC desc;
   surf->GetDesc(&desc);
   
   if (!m_useNvidiaApi && m_INTZ_support)
   {
      D3DTexture* dup;
      CHECKD3D(m_pD3DDevice->CreateTexture(desc.Width, desc.Height, 1,
               D3DUSAGE_DEPTHSTENCIL, (D3DFORMAT)MAKEFOURCC('I', 'N', 'T', 'Z'), D3DPOOL_DEFAULT, &dup, NULL)); // D3DUSAGE_AUTOGENMIPMAP?

      return dup;
   }
   else
   {
      IDirect3DSurface9 *pZBuf;
      HRESULT hr = m_pD3DDevice->CreateDepthStencilSurface(desc.Width, desc.Height, D3DFMT_D16 /*D3DFMT_D24X8*/, //!!
                                                           desc.MultiSampleType, desc.MultiSampleQuality, FALSE, &pZBuf, NULL);
      if (FAILED(hr))
         ReportError("Fatal Error: unable to create depth buffer!", hr, __FILE__, __LINE__);

      return pZBuf;
   }
}

void RenderDevice::DrawPrimitive(const D3DPRIMITIVETYPE type, const DWORD fvf, const void* vertices, const DWORD vertexCount)
{
   HRESULT hr;
   VertexDeclaration * declaration = fvfToDecl(fvf);
   SetVertexDeclaration(declaration);

   const unsigned int np = ComputePrimitiveCount(type, vertexCount);
   m_stats_drawn_triangles += np;
   hr = m_pD3DDevice->DrawPrimitiveUP(type, np, vertices, fvfToSize(fvf));

   if (FAILED(hr))
      ReportError("Fatal Error: DrawPrimitiveUP failed!", hr, __FILE__, __LINE__);

   m_curVertexBuffer = 0;      // DrawPrimitiveUP sets the VB to NULL

   m_curDrawCalls++;
}

void RenderDevice::DrawTexturedQuad(const Vertex3D_TexelOnly* vertices)
{
   /*Vertex3D_TexelOnly* bufvb;
   m_quadDynVertexBuffer->lock(0, 0, (void**)&bufvb, VertexBuffer::DISCARDCONTENTS);
   memcpy(bufvb,vertices,4*sizeof(Vertex3D_TexelOnly));
   m_quadDynVertexBuffer->unlock();
   DrawPrimitiveVB(D3DPT_TRIANGLESTRIP,MY_D3DFVF_TEX,m_quadDynVertexBuffer,0,4);*/

   DrawPrimitive(D3DPT_TRIANGLESTRIP,MY_D3DFVF_TEX,vertices,4); // having a VB and lock/copying stuff each time is slower :/
}

void RenderDevice::DrawFullscreenTexturedQuad()
{
   /*static const float verts[4 * 5] =
   {
      1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
      -1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
      1.0f, -1.0f, 0.0f, 1.0f, 1.0f,
      -1.0f, -1.0f, 0.0f, 0.0f, 1.0f
   };   
   DrawTexturedQuad((Vertex3D_TexelOnly*)verts);*/

   DrawPrimitiveVB(D3DPT_TRIANGLESTRIP,MY_D3DFVF_TEX,m_quadVertexBuffer,0,4);
}

void RenderDevice::DrawPrimitiveVB(const D3DPRIMITIVETYPE type, const DWORD fvf, VertexBuffer* vb, const DWORD startVertex, const DWORD vertexCount)
{
   HRESULT hr;
   VertexDeclaration * declaration = fvfToDecl(fvf);
   SetVertexDeclaration(declaration);

   if (m_curVertexBuffer != vb)
   {
      const unsigned int vsize = fvfToSize(fvf);
      CHECKD3D(m_pD3DDevice->SetStreamSource(0, vb, 0, vsize));
      m_curVertexBuffer = vb;
   }

   const unsigned int np = ComputePrimitiveCount(type, vertexCount);
   m_stats_drawn_triangles += np;
   hr = m_pD3DDevice->DrawPrimitive(type, startVertex, np);
   if (FAILED(hr))
      ReportError("Fatal Error: DrawPrimitive failed!", hr, __FILE__, __LINE__);

   m_curDrawCalls++;
}

void RenderDevice::DrawIndexedPrimitiveVB(const D3DPRIMITIVETYPE type, const DWORD fvf, VertexBuffer* vb, const DWORD startVertex, const DWORD vertexCount, IndexBuffer* ib, const DWORD startIndex, const DWORD indexCount)
{
   VertexDeclaration * declaration = fvfToDecl(fvf);
   SetVertexDeclaration(declaration);

   // bind the vertex and index buffers
   if (m_curVertexBuffer != vb)
   {
      const unsigned int vsize = fvfToSize(fvf);
      CHECKD3D(m_pD3DDevice->SetStreamSource(0, vb, 0, vsize));
      m_curVertexBuffer = vb;
   }

   if (m_curIndexBuffer != ib)
   {
      CHECKD3D(m_pD3DDevice->SetIndices(ib));
      m_curIndexBuffer = ib;
   }

   // render
   const unsigned int np = ComputePrimitiveCount(type, indexCount);
   m_stats_drawn_triangles += np;
   CHECKD3D(m_pD3DDevice->DrawIndexedPrimitive(type, startVertex, 0, vertexCount, startIndex, np));

   m_curDrawCalls++;
}

void RenderDevice::SetTransform(const TransformStateType p1, const D3DMATRIX * p2)
{
   CHECKD3D(m_pD3DDevice->SetTransform((D3DTRANSFORMSTATETYPE)p1, p2));
}

void RenderDevice::GetTransform(const TransformStateType p1, D3DMATRIX* p2)
{
   CHECKD3D(m_pD3DDevice->GetTransform((D3DTRANSFORMSTATETYPE)p1, p2));
}

void RenderDevice::Clear(const DWORD numRects, const D3DRECT* rects, const DWORD flags, const D3DCOLOR color, const D3DVALUE z, const DWORD stencil)
{
   CHECKD3D(m_pD3DDevice->Clear(numRects, rects, flags, color, z, stencil));
}

void RenderDevice::SetViewport(const ViewPort* p1)
{
   CHECKD3D(m_pD3DDevice->SetViewport((D3DVIEWPORT9*)p1));
}

void RenderDevice::GetViewport(ViewPort* p1)
{
   CHECKD3D(m_pD3DDevice->GetViewport((D3DVIEWPORT9*)p1));
}

//
//
//

Shader::Shader(RenderDevice *renderDevice)
{
   m_renderDevice = renderDevice;
   m_shader = 0;
   for (unsigned int i = 0; i < TEXTURESET_STATE_CACHE_SIZE; ++i)
      currentTexture[i] = 0;
   currentAlphaTestValue = -FLT_MAX;
   currentDisableLighting =
   currentFlasherData =
   currentFlasherColor =
   currentLightColor =
   currentLightColor2 =
   currentLightData = D3DXVECTOR4(-FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);
   currentLightImageMode = ~0u;
   currentLightBackglassMode = ~0u;
   currentTechnique[0] = 0;

   memset(&currentMaterial, 0xCC, sizeof(Material));
}

Shader::~Shader()
{
   if (m_shader)
   {
      this->Unload();
   }
}

// loads an HLSL effect file
// if fromFile is true the shaderName should point to the full filename (with path) to the .fx file
// if fromFile is false the shaderName should be the resource name not the IDC_XX_YY value. Search vpinball_eng.rc for ".fx" to see an example
bool Shader::Load(const BYTE* shaderCodeName, UINT codeSize)
{
   LPD3DXBUFFER pBufferErrors;
   DWORD dwShaderFlags = 0; //D3DXSHADER_SKIPVALIDATION // these do not have a measurable effect so far (also if used in the offline fxc step): D3DXSHADER_PARTIALPRECISION, D3DXSHADER_PREFER_FLOW_CONTROL/D3DXSHADER_AVOID_FLOW_CONTROL
   HRESULT hr;
   /*
       if(fromFile)
       {
       dwShaderFlags = D3DXSHADER_DEBUG|D3DXSHADER_SKIPOPTIMIZATION;
       hr = D3DXCreateEffectFromFile(	m_renderDevice->GetCoreDevice(),		// pDevice
       shaderName,			// pSrcFile
       NULL,				// pDefines
       NULL,				// pInclude
       dwShaderFlags,		// Flags
       NULL,				// pPool
       &m_shader,			// ppEffect
       &pBufferErrors);	// ppCompilationErrors
       }
       else
       {
       hr = D3DXCreateEffectFromResource(	m_renderDevice->GetCoreDevice(),		// pDevice
       NULL,
       shaderName,         // resource name
       NULL,				// pDefines
       NULL,				// pInclude
       dwShaderFlags,		// Flags
       NULL,				// pPool
       &m_shader,			// ppEffect
       &pBufferErrors);	// ppCompilationErrors

       }
       */
   hr = D3DXCreateEffect(m_renderDevice->GetCoreDevice(), shaderCodeName, codeSize, NULL, NULL, dwShaderFlags, NULL, &m_shader, &pBufferErrors);
   if (FAILED(hr))
   {
      if (pBufferErrors)
      {
         LPVOID pCompileErrors = pBufferErrors->GetBufferPointer();
         MessageBox(NULL, (const char*)pCompileErrors, "Compile Error", MB_OK | MB_ICONEXCLAMATION);
      }
      else
         MessageBox(NULL, "Unknown Error", "Compile Error", MB_OK | MB_ICONEXCLAMATION);

      return false;
   }
   return true;
}

void Shader::Unload()
{
   SAFE_RELEASE(m_shader);
}

void Shader::SetTexture(const D3DXHANDLE texelName, Texture *texel)
{
   const unsigned int idx = texelName[strlen(texelName) - 1] - '0'; // current convention: SetTexture gets "TextureX", where X 0..4
   assert(idx < TEXTURESET_STATE_CACHE_SIZE);

   if (!texel || !texel->m_pdsBuffer) {
      currentTexture[idx] = NULL; // invalidate the cache

      CHECKD3D(m_shader->SetTexture(texelName, NULL));

      m_renderDevice->m_curTextureChanges++;

      return;
   }

   if (texel->m_pdsBuffer != currentTexture[idx])
   {
      currentTexture[idx] = texel->m_pdsBuffer;
      CHECKD3D(m_shader->SetTexture(texelName, m_renderDevice->m_texMan.LoadTexture(texel->m_pdsBuffer)));

      m_renderDevice->m_curTextureChanges++;
   }
}

void Shader::SetTexture(const D3DXHANDLE texelName, D3DTexture *texel)
{
   const unsigned int idx = texelName[strlen(texelName) - 1] - '0'; // current convention: SetTexture gets "TextureX", where X 0..4
   assert(idx < TEXTURESET_STATE_CACHE_SIZE);

   currentTexture[idx] = NULL; // direct set of device tex invalidates the cache

   CHECKD3D(m_shader->SetTexture(texelName, texel));

   m_renderDevice->m_curTextureChanges++;
}

void Shader::SetMaterial(const Material * const mat)
{
   COLORREF cBase, cGlossy, cClearcoat;
   float fWrapLighting, fRoughness, fGlossyImageLerp, fThickness, fEdge, fEdgeAlpha, fOpacity;
   bool bIsMetal, bOpacityActive;

   if (mat)
   {
      fWrapLighting = mat->m_fWrapLighting;
      fRoughness = exp2f(10.0f * mat->m_fRoughness + 1.0f); // map from 0..1 to 2..2048
      fGlossyImageLerp = mat->m_fGlossyImageLerp;
      fThickness = mat->m_fThickness;
      fEdge = mat->m_fEdge;
      fEdgeAlpha = mat->m_fEdgeAlpha;
      fOpacity = mat->m_fOpacity;
      cBase = mat->m_cBase;
      cGlossy = mat->m_cGlossy;
      cClearcoat = mat->m_cClearcoat;
      bIsMetal = mat->m_bIsMetal;
      bOpacityActive = mat->m_bOpacityActive;
   }
   else
   {
      fWrapLighting = 0.0f;
      fRoughness = exp2f(10.0f * 0.0f + 1.0f); // map from 0..1 to 2..2048
      fGlossyImageLerp = 1.0f;
      fThickness = 0.05f;
      fEdge = 1.0f;
      fEdgeAlpha = 1.0f;
      fOpacity = 1.0f;
      cBase = g_pvp->dummyMaterial.m_cBase;
      cGlossy = 0;
      cClearcoat = 0;
      bIsMetal = false;
      bOpacityActive = false;
   }

   // bIsMetal is nowadays handled via a separate technique! (so not in here)

   if (fRoughness != currentMaterial.m_fRoughness ||
       fEdge != currentMaterial.m_fEdge ||
       fWrapLighting != currentMaterial.m_fWrapLighting ||
       fThickness != currentMaterial.m_fThickness)
   {
      const D3DXVECTOR4 rwem(fRoughness, fWrapLighting, fEdge, fThickness);
      SetVector("Roughness_WrapL_Edge_Thickness", &rwem);
      currentMaterial.m_fRoughness = fRoughness;
      currentMaterial.m_fWrapLighting = fWrapLighting;
      currentMaterial.m_fEdge = fEdge;
      currentMaterial.m_fThickness = fThickness;
   }

   const float alpha = bOpacityActive ? fOpacity : 1.0f;
   if (cBase != currentMaterial.m_cBase || alpha != currentMaterial.m_fOpacity)
   {
      const D3DXVECTOR4 cBaseF = convertColor(cBase, alpha);
      SetVector("cBase_Alpha", &cBaseF);
      currentMaterial.m_cBase = cBase;
      currentMaterial.m_fOpacity = alpha;
   }

   if (!bIsMetal) // Metal has no glossy
      if (cGlossy != currentMaterial.m_cGlossy ||
          fGlossyImageLerp != currentMaterial.m_fGlossyImageLerp)
      {
         const D3DXVECTOR4 cGlossyF = convertColor(cGlossy, fGlossyImageLerp);
         SetVector("cGlossy_ImageLerp", &cGlossyF);
         currentMaterial.m_cGlossy = cGlossy;
         currentMaterial.m_fGlossyImageLerp = fGlossyImageLerp;
      }

   if (cClearcoat != currentMaterial.m_cClearcoat ||
      (bOpacityActive && fEdgeAlpha != currentMaterial.m_fEdgeAlpha))
   {
      const D3DXVECTOR4 cClearcoatF = convertColor(cClearcoat, fEdgeAlpha);
      SetVector("cClearcoat_EdgeAlpha", &cClearcoatF);
      currentMaterial.m_cClearcoat = cClearcoat;
      currentMaterial.m_fEdgeAlpha = fEdgeAlpha;
   }

   if (bOpacityActive /*&& (alpha < 1.0f)*/)
      g_pplayer->m_pin3d.EnableAlphaBlend(false);
   else
      g_pplayer->m_pin3d.DisableAlphaBlend();
}
