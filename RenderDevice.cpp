#include "stdafx.h"

#include <map>
#include "math/math.h"

#include <DxErr.h>

//#include "Dwmapi.h" // use when we get rid of XP at some point, get rid of the manual dll loads in here then

#ifndef DISABLE_FORCE_NVIDIA_OPTIMUS
#include "nvapi.h"
#endif

#include "RenderDevice.h"
#include "Shader.h"
#ifndef ENABLE_SDL
#include "Material.h"
#include "BasicShader.h"
#include "DMDShader.h"
#include "FBShader.h"
#include "FlasherShader.h"
#include "LightShader.h"
#ifdef SEPARATE_CLASSICLIGHTSHADER
#include "ClassicLightShader.h"
#endif
#endif

// SMAA:
#include "shader/AreaTex.h"
#include "shader/SearchTex.h"

#ifndef ENABLE_SDL
#pragma comment(lib, "d3d9.lib")        // TODO: put into build system
#pragma comment(lib, "d3dx9.lib")       // TODO: put into build system
#if _MSC_VER >= 1900
 #pragma comment(lib, "legacy_stdio_definitions.lib") //dxerr.lib needs this
#endif
#pragma comment(lib, "dxerr.lib")       // TODO: put into build system

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
#endif

typedef HRESULT(STDAPICALLTYPE *pRGV)(LPOSVERSIONINFOEXW osi);
static pRGV mRtlGetVersion = nullptr;

bool IsWindows10_1803orAbove()
{
   if (mRtlGetVersion == nullptr)
      mRtlGetVersion = (pRGV)GetProcAddress(GetModuleHandle(TEXT("ntdll")), "RtlGetVersion"); // apparently the only really reliable solution to get the OS version (as of Win10 1803)

   if (mRtlGetVersion != nullptr)
   {
      OSVERSIONINFOEXW osInfo;
      osInfo.dwOSVersionInfoSize = sizeof(osInfo);
      mRtlGetVersion(&osInfo);

      if (osInfo.dwMajorVersion > 10)
         return true;
      if (osInfo.dwMajorVersion == 10 && osInfo.dwMinorVersion > 0)
         return true;
      if (osInfo.dwMajorVersion == 10 && osInfo.dwMinorVersion == 0 && osInfo.dwBuildNumber >= 17134) // which is the more 'common' 1803
         return true;
   }

   return false;
}

#ifdef ENABLE_SDL
//my definition for SDL    GLint size;    GLenum type;    GLboolean normalized;    GLsizei stride;
//D3D definition   WORD Stream;    WORD Offset;    BYTE Type;    BYTE Method;    BYTE Usage;    BYTE UsageIndex;
constexpr VertexElement VertexTexelElement[] =
{
   { 3, GL_FLOAT, GL_FALSE, 0, "POSITION0" },
   { 2, GL_FLOAT, GL_FALSE, 0, "TEXCOORD0" },
   { 0, 0, 0, 0, nullptr}
   /*   { 0, 0 * sizeof(float), D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },  // pos
      { 0, 3 * sizeof(float), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },  // tex0
      D3DDECL_END()*/
};
VertexDeclaration* RenderDevice::m_pVertexTexelDeclaration = (VertexDeclaration*)&VertexTexelElement;

constexpr VertexElement VertexNormalTexelElement[] =
{
   { 3, GL_FLOAT, GL_FALSE, 0, "POSITION0" },
   { 3, GL_FLOAT, GL_FALSE, 0, "NORMAL0" },
   { 2, GL_FLOAT, GL_FALSE, 0, "TEXCOORD0" },
   { 0, 0, 0, 0, nullptr}
/*
      { 0, 0 * sizeof(float), D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },  // pos
      { 0, 3 * sizeof(float), D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },  // normal
      { 0, 6 * sizeof(float), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },  // tex0
      D3DDECL_END()*/
};
VertexDeclaration* RenderDevice::m_pVertexNormalTexelDeclaration = (VertexDeclaration*)&VertexNormalTexelElement;

/*constexpr VertexElement VertexNormalTexelTexelElement[] =
{
   { 0, 0  * sizeof(float),D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },  // pos
   { 0, 3  * sizeof(float),D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0 },  // normal
   { 0, 6  * sizeof(float),D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },  // tex0
   { 0, 8  * sizeof(float),D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },  // tex1
   D3DDECL_END()
};

VertexDeclaration* RenderDevice::m_pVertexNormalTexelTexelDeclaration = nullptr;*/

constexpr VertexElement VertexTrafoTexelElement[] =
{
   { 4, GL_FLOAT, GL_FALSE, 0, "POSITION0" },
   { 2, GL_FLOAT, GL_FALSE, 0, nullptr },//legacy?
   { 2, GL_FLOAT, GL_FALSE, 0, "TEXCOORD0" },
   { 0, 0, 0, 0, nullptr }

   /*   { 0, 0 * sizeof(float), D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITIONT, 0 }, // transformed pos
   { 0, 4 * sizeof(float), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,  1 }, // (mostly, except for classic lights) unused, there to be able to share same code as VertexNormalTexelElement
   { 0, 6 * sizeof(float), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 }, // tex0
   D3DDECL_END()*/
};
VertexDeclaration* RenderDevice::m_pVertexTrafoTexelDeclaration = (VertexDeclaration*)&VertexTrafoTexelElement;
#else
constexpr VertexElement VertexTexelElement[] =
{
   { 0, 0 * sizeof(float), D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },  // pos
   { 0, 3 * sizeof(float), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },  // tex0
   D3DDECL_END()
};
VertexDeclaration* RenderDevice::m_pVertexTexelDeclaration = nullptr;

constexpr VertexElement VertexNormalTexelElement[] =
{
   { 0, 0 * sizeof(float), D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },  // pos
   { 0, 3 * sizeof(float), D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0 },  // normal
   { 0, 6 * sizeof(float), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },  // tex0
   D3DDECL_END()
};
VertexDeclaration* RenderDevice::m_pVertexNormalTexelDeclaration = nullptr;

/*constexpr VertexElement VertexNormalTexelTexelElement[] =
{
   { 0, 0  * sizeof(float),D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },  // pos
   { 0, 3  * sizeof(float),D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0 },  // normal
   { 0, 6  * sizeof(float),D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },  // tex0
   { 0, 8  * sizeof(float),D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },  // tex1
   D3DDECL_END()
};

VertexDeclaration* RenderDevice::m_pVertexNormalTexelTexelDeclaration = nullptr;*/

// pre-transformed, take care that this is a float4 and needs proper w component setup (also see https://docs.microsoft.com/en-us/windows/desktop/direct3d9/mapping-fvf-codes-to-a-directx-9-declaration)
constexpr VertexElement VertexTrafoTexelElement[] =
{
   { 0, 0 * sizeof(float), D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITIONT, 0 }, // transformed pos
   { 0, 4 * sizeof(float), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,  1 }, // (mostly, except for classic lights) unused, there to be able to share same code as VertexNormalTexelElement
   { 0, 6 * sizeof(float), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,  0 }, // tex0
   D3DDECL_END()
};
VertexDeclaration* RenderDevice::m_pVertexTrafoTexelDeclaration = nullptr;

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
      assert(!"Unknown FVF type in fvfToSize");
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
      assert(!"Unknown FVF type in fvfToDecl");
      return nullptr;
   }
}
#endif

static UINT ComputePrimitiveCount(const RenderDevice::PrimitiveTypes type, const int vertexCount)
{
   switch (type)
   {
   case RenderDevice::POINTLIST:
      return vertexCount;
   case RenderDevice::LINELIST:
      return vertexCount / 2;
   case RenderDevice::LINESTRIP:
      return std::max(0, vertexCount - 1);
   case RenderDevice::TRIANGLELIST:
      return vertexCount / 3;
   case RenderDevice::TRIANGLESTRIP:
   case RenderDevice::TRIANGLEFAN:
      return std::max(0, vertexCount - 2);
   default:
      return 0;
   }
}

#ifdef ENABLE_SDL
static const char* glErrorToString(const int error) {
   switch (error) {
   case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
   case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
   case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
   case GL_STACK_OVERFLOW: return "GL_STACK_OVERFLOW";
   case GL_STACK_UNDERFLOW: return "GL_STACK_UNDERFLOW";
   case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
   case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";
   default: return "unknown";
   }
}
#endif

void ReportFatalError(const HRESULT hr, const char *file, const int line)
{
   char msg[2048+128];
#ifdef ENABLE_SDL
   sprintf_s(msg, "GL Fatal Error 0x%0002X %s in %s:%d", hr, glErrorToString(hr), file, line);
   ShowError(msg);
#else
   sprintf_s(msg, "Fatal error %s (0x%x: %s) at %s:%d", DXGetErrorString(hr), hr, DXGetErrorDescription(hr), file, line);
   ShowError(msg);
   exit(-1);
#endif
}

void ReportError(const char *errorText, const HRESULT hr, const char *file, const int line)
{
   char msg[2048+128];
#ifdef ENABLE_SDL
   sprintf_s(msg, "GL Error 0x%0002X %s in %s:%d\n%s", hr, glErrorToString(hr), file, line, errorText);
   ShowError(msg);
#else
   sprintf_s(msg, "%s %s (0x%x: %s) at %s:%d", errorText, DXGetErrorString(hr), hr, DXGetErrorDescription(hr), file, line);
   ShowError(msg);
   exit(-1);
#endif
}

unsigned m_curLockCalls, m_frameLockCalls;
unsigned int RenderDevice::Perf_GetNumLockCalls() const { return m_frameLockCalls; }

#if 0//def ENABLE_SDL //not used anymore
void checkGLErrors(const char *file, const int line) {
   GLenum err;
   unsigned int count = 0;
   while ((err = glGetError()) != GL_NO_ERROR) {
      count++;
      ReportFatalError(err, file, line);
   }
   /*if (count>0) {
      exit(-1);
   }*/
}
#endif

// Callback function for printing debug statements
#if defined(ENABLE_SDL) && defined(_DEBUG)
void APIENTRY GLDebugMessageCallback(GLenum source, GLenum type, GLuint id,
                                     GLenum severity, GLsizei length,
                                     const GLchar *msg, const void *data)
{
    char* _source;
    switch (source) {
        case GL_DEBUG_SOURCE_API:
        _source = "API";
        break;

        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        _source = "WINDOW SYSTEM";
        break;

        case GL_DEBUG_SOURCE_SHADER_COMPILER:
        _source = "SHADER COMPILER";
        break;

        case GL_DEBUG_SOURCE_THIRD_PARTY:
        _source = "THIRD PARTY";
        break;

        case GL_DEBUG_SOURCE_APPLICATION:
        _source = "APPLICATION";
        break;

        case GL_DEBUG_SOURCE_OTHER:
        _source = "UNKNOWN";
        break;

        default:
        _source = "UNHANDLED";
        break;
    }

    char* _type;
    switch (type) {
        case GL_DEBUG_TYPE_ERROR:
        _type = "ERROR";
        break;

        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        _type = "DEPRECATED BEHAVIOR";
        break;

        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        _type = "UNDEFINED BEHAVIOR";
        break;

        case GL_DEBUG_TYPE_PORTABILITY:
        _type = "PORTABILITY";
        break;

        case GL_DEBUG_TYPE_PERFORMANCE:
        _type = "PERFORMANCE";
        break;

        case GL_DEBUG_TYPE_OTHER:
        _type = "OTHER";
        break;

        case GL_DEBUG_TYPE_MARKER:
        _type = "MARKER";
        break;

        case GL_DEBUG_TYPE_PUSH_GROUP:
        _type = "GL_DEBUG_TYPE_PUSH_GROUP";
        break;

        case GL_DEBUG_TYPE_POP_GROUP:
        _type = "GL_DEBUG_TYPE_POP_GROUP";
        break;

    	default:
        _type = "UNHANDLED";
        break;
    }

    char* _severity;
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
        _severity = "HIGH";
        break;

        case GL_DEBUG_SEVERITY_MEDIUM:
        _severity = "MEDIUM";
        break;

        case GL_DEBUG_SEVERITY_LOW:
        _severity = "LOW";
        break;

        case GL_DEBUG_SEVERITY_NOTIFICATION:
        _severity = "NOTIFICATION";
        break;

        default:
        _severity = "UNHANDLED";
        break;
    }

    //if(severity != GL_DEBUG_SEVERITY_NOTIFICATION)
    fprintf(stderr,"%d: %s of %s severity, raised from %s: %s\n", id, _type, _severity, _source, msg);

    if (type == GL_DEBUG_TYPE_ERROR || type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR || severity == GL_DEBUG_SEVERITY_HIGH)
        ShowError(msg);
}
#endif

////////////////////////////////////////////////////////////////////

int getNumberOfDisplays()
{
#ifdef ENABLE_SDL
   return SDL_GetNumVideoDisplays();
#else
   return GetSystemMetrics(SM_CMONITORS);
#endif
}

void EnumerateDisplayModes(const int display, std::vector<VideoMode>& modes)
{
   modes.clear();

#ifdef ENABLE_SDL
   const int amount = SDL_GetNumDisplayModes(display);
   for (int mode = 0; mode < amount; ++mode) {
      SDL_DisplayMode myMode;
      SDL_GetDisplayMode(display, mode, &myMode);
      VideoMode vmode = {};
      vmode.width = myMode.w;
      vmode.height = myMode.h;
      switch (myMode.format) {
      case SDL_PIXELFORMAT_RGB24:
      case SDL_PIXELFORMAT_BGR24:
      case SDL_PIXELFORMAT_RGB888:
      case SDL_PIXELFORMAT_RGBX8888:
      case SDL_PIXELFORMAT_BGR888:
      case SDL_PIXELFORMAT_BGRX8888:
      case SDL_PIXELFORMAT_ARGB8888:
      case SDL_PIXELFORMAT_RGBA8888:
      case SDL_PIXELFORMAT_ABGR8888:
      case SDL_PIXELFORMAT_BGRA8888:
         vmode.depth = 32;
         break;
      case SDL_PIXELFORMAT_RGB565:
      case SDL_PIXELFORMAT_BGR565:
      case SDL_PIXELFORMAT_ABGR1555:
      case SDL_PIXELFORMAT_BGRA5551:
      case SDL_PIXELFORMAT_ARGB1555:
      case SDL_PIXELFORMAT_RGBA5551:
         vmode.depth = 16;
         break;
      case SDL_PIXELFORMAT_ARGB2101010:
         vmode.depth = 30;
         break;
      default:
         vmode.depth = 0;
      }
      vmode.refreshrate = myMode.refresh_rate;
      modes.push_back(vmode);
   }
#else
   std::vector<DisplayConfig> displays;
   getDisplayList(displays);
   if (display >= (int)displays.size())
      return;
   const int adapter = displays[display].adapter;

   IDirect3D9 *d3d = Direct3DCreate9(D3D_SDK_VERSION);
   if (d3d == nullptr)
   {
      ShowError("Could not create D3D9 object.");
      return;
   }

   //for (int j = 0; j < 2; ++j)
   const int j = 0; // limit to 32bit only nowadays
   {
      const D3DFORMAT fmt = (D3DFORMAT)((j == 0) ? colorFormat::RGB8 : colorFormat::RGB5);
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
            mode.depth = (fmt == (D3DFORMAT)colorFormat::RGB5) ? 16 : 32;
            mode.refreshrate = d3dmode.RefreshRate;
            modes.push_back(mode);
         }
      }
   }

   SAFE_RELEASE(d3d);
#endif
}

BOOL CALLBACK MonitorEnumList(__in  HMONITOR hMonitor, __in  HDC hdcMonitor, __in  LPRECT lprcMonitor, __in  LPARAM dwData)
{
   std::map<std::string,DisplayConfig>* data = reinterpret_cast<std::map<std::string,DisplayConfig>*>(dwData);
   MONITORINFOEX info;
   info.cbSize = sizeof(MONITORINFOEX);
   GetMonitorInfo(hMonitor, &info);
   DisplayConfig config = {};
   config.top = info.rcMonitor.top;
   config.left = info.rcMonitor.left;
   config.width = info.rcMonitor.right - info.rcMonitor.left;
   config.height = info.rcMonitor.bottom - info.rcMonitor.top;
   config.isPrimary = (config.top == 0) && (config.left == 0);
   config.display = (int)data->size(); // This number does neither map to the number form display settings nor something else.
#ifdef ENABLE_SDL
   config.adapter = config.display;
#else
   config.adapter = -1;
#endif
   memcpy(config.DeviceName, info.szDevice, CCHDEVICENAME); // Internal display name e.g. "\\\\.\\DISPLAY1"
   data->insert(std::pair<std::string, DisplayConfig>(config.DeviceName, config));
   return TRUE;
}

int getDisplayList(std::vector<DisplayConfig>& displays)
{
   displays.clear();
   std::map<std::string, DisplayConfig> displayMap;
   // Get the resolution of all enabled displays.
   EnumDisplayMonitors(nullptr, nullptr, MonitorEnumList, reinterpret_cast<LPARAM>(&displayMap));

#ifndef ENABLE_SDL
   IDirect3D9* pD3D = Direct3DCreate9(D3D_SDK_VERSION);
   if (pD3D == nullptr)
   {
      ShowError("Could not create D3D9 object.");
      return -1;
   }
   // Map the displays to the DX9 adapter. Otherwise this leads to an performance impact on systems with multiple GPUs
   const int adapterCount = pD3D->GetAdapterCount();
   for (int i = 0;i < adapterCount;++i) {
      D3DADAPTER_IDENTIFIER9 adapter;
      pD3D->GetAdapterIdentifier(i, 0, &adapter);
      std::map<std::string, DisplayConfig>::iterator display = displayMap.find(adapter.DeviceName);
      if (display != displayMap.end()) {
         display->second.adapter = i;
         strncpy_s(display->second.GPU_Name, adapter.Description, sizeof(display->second.GPU_Name)-1);
      }
   }
   SAFE_RELEASE(pD3D);
#endif

   // Apply the same numbering as windows
   int i = 0;
   for (std::map<std::string, DisplayConfig>::iterator display = displayMap.begin(); display != displayMap.end(); ++display)
   {
      if (display->second.adapter >= 0) {
         display->second.display = i;
#ifdef ENABLE_SDL
         const char* name = SDL_GetDisplayName(display->second.adapter);
         if(name != nullptr)
            strncpy_s(display->second.GPU_Name, name, sizeof(display->second.GPU_Name) - 1);
         else
            display->second.GPU_Name[0] = '\0'; //!!
#endif
         displays.push_back(display->second);
      }
      i++;
   }
   return i;
}

bool getDisplaySetupByID(const int display, int &x, int &y, int &width, int &height)
{
   std::vector<DisplayConfig> displays;
   getDisplayList(displays);
   for (std::vector<DisplayConfig>::iterator displayConf = displays.begin(); displayConf != displays.end(); ++displayConf) {
      if ((display == -1 && displayConf->isPrimary) || display == displayConf->display) {
         x = displayConf->left;
         y = displayConf->top;
         width = displayConf->width;
         height = displayConf->height;
         return true;
      }
   }
   x = 0;
   y = 0;
   width = GetSystemMetrics(SM_CXSCREEN);
   height = GetSystemMetrics(SM_CYSCREEN);
   return false;
}

int getPrimaryDisplay()
{
   std::vector<DisplayConfig> displays;
   getDisplayList(displays);
   for (std::vector<DisplayConfig>::iterator displayConf = displays.begin(); displayConf != displays.end(); ++displayConf)
      if (displayConf->isPrimary)
         return displayConf->adapter;
   return 0;
}

////////////////////////////////////////////////////////////////////

VertexBuffer* RenderDevice::m_quadVertexBuffer = nullptr;
unsigned int RenderDevice::m_stats_drawn_triangles = 0;

#ifndef ENABLE_SDL
#ifdef USE_D3D9EX
 typedef HRESULT(WINAPI *pD3DC9Ex)(UINT SDKVersion, IDirect3D9Ex**);
 static pD3DC9Ex mDirect3DCreate9Ex = nullptr;
#endif

#define DWM_EC_DISABLECOMPOSITION         0
#define DWM_EC_ENABLECOMPOSITION          1
typedef HRESULT(STDAPICALLTYPE *pDICE)(BOOL* pfEnabled);
static pDICE mDwmIsCompositionEnabled = nullptr;
typedef HRESULT(STDAPICALLTYPE *pDF)();
static pDF mDwmFlush = nullptr;
typedef HRESULT(STDAPICALLTYPE *pDEC)(UINT uCompositionAction);
static pDEC mDwmEnableComposition = nullptr;
#endif

RenderDevice::RenderDevice(const HWND hwnd, const int width, const int height, const bool fullscreen, const int colordepth, int VSync, const bool useAA, const bool stereo3D, const unsigned int FXAA, const bool sharpen, const bool ss_refl, const bool useNvidiaApi, const bool disable_dwm, const int BWrendering)
    : m_windowHwnd(hwnd), m_width(width), m_height(height), m_fullscreen(fullscreen), 
      m_colorDepth(colordepth), m_vsync(VSync), m_useAA(useAA), m_stereo3D(stereo3D),
      m_ssRefl(ss_refl), m_disableDwm(disable_dwm), m_sharpen(sharpen), m_FXAA(FXAA), m_BWrendering(BWrendering), m_texMan(*this)
{
    m_useNvidiaApi = useNvidiaApi;
    m_INTZ_support = false;
    NVAPIinit = false;

    m_stats_drawn_triangles = 0;

    mDwmIsCompositionEnabled = (pDICE)GetProcAddress(GetModuleHandle(TEXT("dwmapi.dll")), "DwmIsCompositionEnabled"); //!! remove as soon as win xp support dropped and use static link
    mDwmEnableComposition = (pDEC)GetProcAddress(GetModuleHandle(TEXT("dwmapi.dll")), "DwmEnableComposition"); //!! remove as soon as win xp support dropped and use static link
    mDwmFlush = (pDF)GetProcAddress(GetModuleHandle(TEXT("dwmapi.dll")), "DwmFlush"); //!! remove as soon as win xp support dropped and use static link

    if (mDwmIsCompositionEnabled && mDwmEnableComposition)
    {
        BOOL dwm = 0;
        mDwmIsCompositionEnabled(&dwm);
        m_dwm_enabled = m_dwm_was_enabled = !!dwm;

        if (m_dwm_was_enabled && m_disableDwm && IsWindowsVistaOr7()) // windows 8 and above will not allow do disable it, but will still return S_OK
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

    currentDeclaration = nullptr;
    //m_curShader = nullptr;

    // fill state caches with dummy values
    memset(textureStateCache, 0xCC, sizeof(DWORD) * TEXTURE_SAMPLERS * TEXTURE_STATE_CACHE_SIZE);
    memset(textureSamplerCache, 0xCC, sizeof(DWORD) * TEXTURE_SAMPLERS * TEXTURE_SAMPLER_CACHE_SIZE);

    // initialize performance counters
    m_curDrawCalls = m_frameDrawCalls = 0;
    m_curStateChanges = m_frameStateChanges = 0;
    m_curTextureChanges = m_frameTextureChanges = 0;
    m_curParameterChanges = m_frameParameterChanges = 0;
    m_curTechniqueChanges = m_frameTechniqueChanges = 0;
    m_curTextureUpdates = m_frameTextureUpdates = 0;

    m_curLockCalls = m_frameLockCalls = 0; //!! meh
}

void RenderDevice::CreateDevice(int &refreshrate, UINT adapterIndex)
{
#ifdef USE_D3D9EX
   m_pD3DEx = nullptr;
   m_pD3DDeviceEx = nullptr;

   mDirect3DCreate9Ex = (pD3DC9Ex)GetProcAddress(GetModuleHandle(TEXT("d3d9.dll")), "Direct3DCreate9Ex"); //!! remove as soon as win xp support dropped and use static link
   if (mDirect3DCreate9Ex)
   {
      const HRESULT hr = mDirect3DCreate9Ex(D3D_SDK_VERSION, &m_pD3DEx);
      if (FAILED(hr) || (m_pD3DEx == nullptr))
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
      if (m_pD3D == nullptr)
      {
         ShowError("Could not create D3D9 object.");
         throw 0;
      }
   }

   m_adapter = m_pD3D->GetAdapterCount() > (int)adapterIndex ? adapterIndex : 0;

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

   bool video10bit = LoadValueBoolWithDefault("Player", "Render10Bit", false);

   if (!m_fullscreen && video10bit)
   {
      ShowError("10Bit-Monitor support requires 'Force exclusive Fullscreen Mode' to be also enabled!");
      video10bit = false;
   }

   // get the current display format
   D3DFORMAT format;
   if (!m_fullscreen)
   {
      D3DDISPLAYMODE mode;
      CHECKD3D(m_pD3D->GetAdapterDisplayMode(m_adapter, &mode));
      format = mode.Format;
      refreshrate = mode.RefreshRate;
   }
   else
   {
      format = (D3DFORMAT)(video10bit ? colorFormat::RGBA10 : ((m_colorDepth == 16) ? colorFormat::RGB5 : colorFormat::RGB8));
   }

   // limit vsync rate to actual refresh rate, otherwise special handling in renderloop
   if (m_vsync > refreshrate)
      m_vsync = 0;

   D3DPRESENT_PARAMETERS params;
   params.BackBufferWidth = m_width;
   params.BackBufferHeight = m_height;
   params.BackBufferFormat = format;
   params.BackBufferCount = 1;
   params.MultiSampleType = /*useAA ? D3DMULTISAMPLE_4_SAMPLES :*/ D3DMULTISAMPLE_NONE; // D3DMULTISAMPLE_NONMASKABLE? //!! useAA now uses super sampling/offscreen render
   params.MultiSampleQuality = 0; // if D3DMULTISAMPLE_NONMASKABLE then set to > 0
   params.SwapEffect = D3DSWAPEFFECT_DISCARD;  // FLIP ?
   params.hDeviceWindow = m_windowHwnd;
   params.Windowed = !m_fullscreen;
   params.EnableAutoDepthStencil = FALSE;
   params.AutoDepthStencilFormat = D3DFMT_UNKNOWN;      // ignored
   params.Flags = /*fullscreen ? D3DPRESENTFLAG_LOCKABLE_BACKBUFFER :*/ /*(stereo3D ?*/ 0 /*: D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL)*/; // D3DPRESENTFLAG_LOCKABLE_BACKBUFFER only needed for SetDialogBoxMode() below, but makes rendering slower on some systems :/
   params.FullScreen_RefreshRateInHz = m_fullscreen ? refreshrate : 0;
#ifdef USE_D3D9EX
   params.PresentationInterval = (m_pD3DEx && (m_vsync != 1)) ? D3DPRESENT_INTERVAL_IMMEDIATE : (!!m_vsync ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE); //!! or have a special mode to force normal vsync?
#else
   params.PresentationInterval = !!m_vsync ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;
#endif

   // check if our HDR texture format supports/does sRGB conversion on texture reads, which must NOT be the case as we always set SRGBTexture=true independent of the format!
   HRESULT hr = m_pD3D->CheckDeviceFormat(m_adapter, devtype, params.BackBufferFormat, D3DUSAGE_QUERY_SRGBREAD, D3DRTYPE_TEXTURE, (D3DFORMAT)colorFormat::RGBA32F);
   if (SUCCEEDED(hr))
      ShowError("D3D device does support D3DFMT_A32B32G32R32F SRGBTexture reads (which leads to wrong tex colors)");
   // now the same for our LDR/8bit texture format the other way round
   hr = m_pD3D->CheckDeviceFormat(m_adapter, devtype, params.BackBufferFormat, D3DUSAGE_QUERY_SRGBREAD, D3DRTYPE_TEXTURE, (D3DFORMAT)colorFormat::RGBA8);
   if (!SUCCEEDED(hr))
      ShowError("D3D device does not support D3DFMT_A8R8G8B8 SRGBTexture reads (which leads to wrong tex colors)");

   // check if auto generation of mipmaps can be used, otherwise will be done via d3dx
   m_autogen_mipmap = (caps.Caps2 & D3DCAPS2_CANAUTOGENMIPMAP) != 0;
   if (m_autogen_mipmap)
      m_autogen_mipmap = (m_pD3D->CheckDeviceFormat(m_adapter, devtype, params.BackBufferFormat, textureUsage::AUTOMIPMAP, D3DRTYPE_TEXTURE, (D3DFORMAT)colorFormat::RGBA8) == D3D_OK);

   //m_autogen_mipmap = false; //!! could be done to support correct sRGB/gamma correct generation of mipmaps which is not possible with auto gen mipmap in DX9! at the moment disabled, as the sRGB software path is super slow for similar mipmap filter quality

#ifndef DISABLE_FORCE_NVIDIA_OPTIMUS
   if (!NVAPIinit)
   {
      if (NvAPI_Initialize() == NVAPI_OK)
         NVAPIinit = true;
   }
#endif

   // Determine if INTZ is supported
#ifdef ENABLE_SDL
   m_INTZ_support = false;
#else
   m_INTZ_support = (m_pD3D->CheckDeviceFormat( m_adapter, devtype, params.BackBufferFormat,
                     D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_TEXTURE, ((D3DFORMAT)(MAKEFOURCC('I','N','T','Z'))))) == D3D_OK;
#endif

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

   const bool softwareVP = LoadValueBoolWithDefault("Player", "SoftwareVertexProcessing", false);
   const DWORD flags = softwareVP ? D3DCREATE_SOFTWARE_VERTEXPROCESSING : D3DCREATE_HARDWARE_VERTEXPROCESSING;

   // Create the D3D device. This optionally goes to the proper fullscreen mode.
   // It also creates the default swap chain (front and back buffer).
#ifdef USE_D3D9EX
   if (m_pD3DEx)
   {
      D3DDISPLAYMODEEX mode;
      mode.Size = sizeof(D3DDISPLAYMODEEX);
      if (m_fullscreen)
      {
         mode.Format = params.BackBufferFormat;
         mode.Width = params.BackBufferWidth;
         mode.Height = params.BackBufferHeight;
         mode.RefreshRate = params.FullScreen_RefreshRateInHz;
         mode.ScanLineOrdering = D3DSCANLINEORDERING_PROGRESSIVE;
      }

      hr = m_pD3DEx->CreateDeviceEx(
         m_adapter,
         devtype,
         m_windowHwnd,
         flags /*| D3DCREATE_PUREDEVICE*/,
         &params,
         m_fullscreen ? &mode : nullptr,
         &m_pD3DDeviceEx);
      if (FAILED(hr))
      {
         if (m_fullscreen)
         {
            const int result = GetSystemMetrics(SM_REMOTESESSION);
            const bool isRemoteSession = (result != 0);
            if (isRemoteSession)
               ShowError("Try disabling exclusive Fullscreen Mode for Remote Desktop Connections");
         }
         ReportFatalError(hr, __FILE__, __LINE__);
      }

      m_pD3DDeviceEx->QueryInterface(__uuidof(IDirect3DDevice9), reinterpret_cast<void**>(&m_pD3DDevice));

      // Get the display mode so that we can report back the actual refresh rate.
      CHECKD3D(m_pD3DDeviceEx->GetDisplayModeEx(0, &mode, nullptr)); //!! what is the actual correct value for the swapchain here?

      refreshrate = mode.RefreshRate;
   }
   else
#endif
   {
      hr = m_pD3D->CreateDevice(
         m_adapter,
         devtype,
         m_windowHwnd,
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

   /*if (m_fullscreen)
       hr = m_pD3DDevice->SetDialogBoxMode(TRUE);*/ // needs D3DPRESENTFLAG_LOCKABLE_BACKBUFFER, but makes rendering slower on some systems :/

   // Retrieve a reference to the back buffer.
   m_pBackBuffer = new RenderTarget(this);

   const colorFormat render_format = ((m_BWrendering == 1) ? colorFormat::RG16F : ((m_BWrendering == 2) ? colorFormat::RED16F : colorFormat::RGBA16F));

   // alloc float buffer for rendering (optionally 2x2 res for manual super sampling)
   m_pOffscreenBackBufferTexture = new RenderTarget(this, m_useAA ? 2 * m_width : m_width, m_useAA ? 2 * m_height : m_height, render_format, true, false, m_stereo3D, "Fatal Error: unable to create render buffer!");

   // alloc buffer for screen space fake reflection rendering (optionally 2x2 res for manual super sampling)
   if (m_ssRefl)
      m_pReflectionBufferTexture = new RenderTarget(this, m_useAA ? 2 * m_width : m_width, m_useAA ? 2 * m_height : m_height, render_format, false, false, m_stereo3D, "Fatal Error: unable to create reflection buffer!");
   else
      m_pReflectionBufferTexture = nullptr;

   if (g_pplayer != nullptr)
   {
      const bool drawBallReflection = ((g_pplayer->m_reflectionForBalls && (g_pplayer->m_ptable->m_useReflectionForBalls == -1)) || (g_pplayer->m_ptable->m_useReflectionForBalls == 1));
      if ((g_pplayer->m_ptable->m_reflectElementsOnPlayfield /*&& g_pplayer->m_pf_refl*/) || drawBallReflection)
         m_pMirrorTmpBufferTexture = new RenderTarget(this, m_useAA ? 2 * m_width : m_width, m_useAA ? 2 * m_height : m_height, render_format, true, false, m_stereo3D, "Fatal Error: unable to create reflection map!");
   }
   // alloc bloom tex at 1/4 x 1/4 res (allows for simple HQ downscale of clipped input while saving memory)
   m_pBloomBufferTexture = new RenderTarget(this, m_width / 4, m_height / 4, render_format, false, false, m_stereo3D, "Fatal Error: unable to create bloom buffer!");

   // temporary buffer for gaussian blur
   m_pBloomTmpBufferTexture = new RenderTarget(this, m_width / 4, m_height / 4, render_format, false, false, m_stereo3D, "Fatal Error: unable to create blur buffer!");

   // alloc temporary buffer for stereo3D/post-processing AA/sharpen
   if ((m_stereo3D) || (m_FXAA > 0) || m_sharpen)
      m_pOffscreenBackBufferTmpTexture = new RenderTarget(this, m_width, m_height, video10bit ? colorFormat::RGBA10 : colorFormat::RGBA8, false, false, m_stereo3D,
         "Fatal Error: unable to create stereo3D/post-processing AA/sharpen buffer!");
   else
      m_pOffscreenBackBufferTmpTexture = nullptr;

   // alloc one more temporary buffer for SMAA
   if (m_FXAA == Quality_SMAA)
      m_pOffscreenBackBufferTmpTexture2 = new RenderTarget(this, m_width, m_height, video10bit ? colorFormat::RGBA10 : colorFormat::RGBA8, false, false, m_stereo3D, "Fatal Error: unable to create SMAA buffer!");
   else
      m_pOffscreenBackBufferTmpTexture2 = nullptr;

   if (video10bit && (m_FXAA == Quality_SMAA || m_FXAA == Standard_DLAA))
      ShowError("SMAA or DLAA post-processing AA should not be combined with 10Bit-output rendering (will result in visible artifacts)!");

   //

   // create default vertex declarations for shaders
   CreateVertexDeclaration(VertexTexelElement, &m_pVertexTexelDeclaration);
   CreateVertexDeclaration(VertexNormalTexelElement, &m_pVertexNormalTexelDeclaration);
   //CreateVertexDeclaration( VertexNormalTexelTexelElement, &m_pVertexNormalTexelTexelDeclaration );
   CreateVertexDeclaration(VertexTrafoTexelElement, &m_pVertexTrafoTexelDeclaration);

   if(m_FXAA == Quality_SMAA)
       UploadAndSetSMAATextures();
   else
   {
      m_SMAAareaTexture = nullptr;
      m_SMAAsearchTexture = nullptr;
   }
}

bool RenderDevice::LoadShaders()
{
   bool shaderCompilationOkay = true;

   basicShader = new Shader(this);
   shaderCompilationOkay = basicShader->Load(g_basicShaderCode, sizeof(g_basicShaderCode)) && shaderCompilationOkay;

   DMDShader = new Shader(this);
   shaderCompilationOkay = DMDShader->Load(g_dmdShaderCode, sizeof(g_dmdShaderCode)) && shaderCompilationOkay;

   FBShader = new Shader(this);
   shaderCompilationOkay = FBShader->Load(g_FBShaderCode, sizeof(g_FBShaderCode)) && shaderCompilationOkay;

   flasherShader = new Shader(this);
   shaderCompilationOkay = flasherShader->Load(g_flasherShaderCode, sizeof(g_flasherShaderCode)) && shaderCompilationOkay;

   lightShader = new Shader(this);
   shaderCompilationOkay = lightShader->Load(g_lightShaderCode, sizeof(g_lightShaderCode)) && shaderCompilationOkay;

#ifdef SEPARATE_CLASSICLIGHTSHADER
   classicLightShader = new Shader(this);
   shaderCompilationOkay = classicLightShader->Load(g_classicLightShaderCode, sizeof(g_classicLightShaderCode)) && shaderCompilationOkay;
#endif

   if (!shaderCompilationOkay)
   {
      ReportError("Fatal Error: shader compilation failed!", -1, __FILE__, __LINE__);
      return false;
   }

   // Now that shaders are compiled, set static textures for SMAA postprocessing shader
   if (m_FXAA == Quality_SMAA)
   {
      CHECKD3D(FBShader->Core()->SetTexture(SHADER_areaTex2D, m_SMAAareaTexture->GetCoreTexture()));
      CHECKD3D(FBShader->Core()->SetTexture(SHADER_searchTex2D, m_SMAAsearchTexture->GetCoreTexture()));
   }

   // Initialize uniform to default value
   basicShader->SetFlasherColorAlpha(vec4(1.0f, 1.0f, 1.0f, 1.0f));

   return true;
}

bool RenderDevice::DepthBufferReadBackAvailable()
{
#ifdef ENABLE_SDL
   return true;
#else
    if (m_INTZ_support && !m_useNvidiaApi)
        return true;
    // fall back to NVIDIAs NVAPI, only handle DepthBuffer ReadBack if API was initialized
    return NVAPIinit;
#endif
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
       g_pvp->MessageBox("WARNING! Direct3D resource leak detected!", "Visual Pinball", MB_ICONWARNING);
   }
}
#endif


void RenderDevice::FreeShader()
{
   if (basicShader)
   {
      CHECKD3D(basicShader->Core()->SetTexture(SHADER_Texture0, nullptr));
      CHECKD3D(basicShader->Core()->SetTexture(SHADER_Texture1, nullptr));
      CHECKD3D(basicShader->Core()->SetTexture(SHADER_Texture2, nullptr));
      CHECKD3D(basicShader->Core()->SetTexture(SHADER_Texture3, nullptr));
      CHECKD3D(basicShader->Core()->SetTexture(SHADER_Texture4, nullptr));
      delete basicShader;
      basicShader = 0;
   }
   if (DMDShader)
   {
      CHECKD3D(DMDShader->Core()->SetTexture(SHADER_Texture0, nullptr));
      delete DMDShader;
      DMDShader = 0;
   }
   if (FBShader)
   {
      CHECKD3D(FBShader->Core()->SetTexture(SHADER_Texture0, nullptr));
      CHECKD3D(FBShader->Core()->SetTexture(SHADER_Texture1, nullptr));
      CHECKD3D(FBShader->Core()->SetTexture(SHADER_Texture3, nullptr));
      CHECKD3D(FBShader->Core()->SetTexture(SHADER_Texture4, nullptr));

      CHECKD3D(FBShader->Core()->SetTexture(SHADER_areaTex2D, nullptr));
      CHECKD3D(FBShader->Core()->SetTexture(SHADER_searchTex2D, nullptr));

      delete FBShader;
      FBShader = 0;
   }
   if (flasherShader)
   {
      CHECKD3D(flasherShader->Core()->SetTexture(SHADER_Texture0, nullptr));
      CHECKD3D(flasherShader->Core()->SetTexture(SHADER_Texture1, nullptr));
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
      CHECKD3D(classicLightShader->Core()->SetTexture(SHADER_Texture0, nullptr));
      CHECKD3D(classicLightShader->Core()->SetTexture(SHADER_Texture1, nullptr));
      CHECKD3D(classicLightShader->Core()->SetTexture(SHADER_Texture2, nullptr));
      delete classicLightShader;
      classicLightShader=0;
   }
#endif
}

RenderDevice::~RenderDevice()
{
   if (m_quadVertexBuffer)
      m_quadVertexBuffer->release();
   m_quadVertexBuffer = nullptr;

   //m_quadDynVertexBuffer->release();

#ifndef ENABLE_SDL
#ifndef DISABLE_FORCE_NVIDIA_OPTIMUS
   if (NVAPIinit) //!! meh
      CHECKNVAPI(NvAPI_Unload());
   NVAPIinit = false;
#endif

   //
   m_pD3DDevice->SetStreamSource(0, nullptr, 0, 0);
   m_pD3DDevice->SetIndices(nullptr);
   m_pD3DDevice->SetVertexShader(nullptr);
   m_pD3DDevice->SetPixelShader(nullptr);
   m_pD3DDevice->SetFVF(D3DFVF_XYZ);
   //m_pD3DDevice->SetVertexDeclaration(nullptr); // invalid call
   //m_pD3DDevice->SetRenderTarget(0, nullptr); // invalid call
   m_pD3DDevice->SetDepthStencilSurface(nullptr);
#endif

   FreeShader();

   SAFE_RELEASE(m_pVertexTexelDeclaration);
   SAFE_RELEASE(m_pVertexNormalTexelDeclaration);
   //SAFE_RELEASE(m_pVertexNormalTexelTexelDeclaration);
   SAFE_RELEASE(m_pVertexTrafoTexelDeclaration);

   m_texMan.UnloadAll();
   delete m_pOffscreenBackBufferTexture;
   delete m_pOffscreenBackBufferTmpTexture;
   delete m_pOffscreenBackBufferTmpTexture2;
   delete m_pReflectionBufferTexture;

   if (g_pplayer)
   {
      const bool drawBallReflection = ((g_pplayer->m_reflectionForBalls && (g_pplayer->m_ptable->m_useReflectionForBalls == -1)) || (g_pplayer->m_ptable->m_useReflectionForBalls == 1));
      if ((g_pplayer->m_ptable->m_reflectElementsOnPlayfield /*&& g_pplayer->m_pf_refl*/) || drawBallReflection)
         delete m_pMirrorTmpBufferTexture;
   }
   delete m_pBloomBufferTexture;
   delete m_pBloomTmpBufferTexture;
   delete m_pBackBuffer;

   delete m_SMAAareaTexture;
   delete m_SMAAsearchTexture;

#ifndef ENABLE_SDL
#ifdef _DEBUG
   CheckForD3DLeak(m_pD3DDevice);
#endif

#ifdef USE_D3D9EX
   //!! if (m_pD3DDeviceEx == m_pD3DDevice) m_pD3DDevice = nullptr; //!! needed for Caligula if m_adapter > 0 ?? weird!! BUT MESSES UP FULLSCREEN EXIT (=hangs)
   SAFE_RELEASE_NO_RCC(m_pD3DDeviceEx);
#endif
#ifdef DEBUG_REFCOUNT_TRIGGER
   SAFE_RELEASE(m_pD3DDevice);
#else
   FORCE_RELEASE(m_pD3DDevice); //!! why is this necessary for some setups? is the refcount still off for some settings?
#endif
#ifdef USE_D3D9EX
   SAFE_RELEASE_NO_RCC(m_pD3DEx);
#endif
#ifdef DEBUG_REFCOUNT_TRIGGER
   SAFE_RELEASE(m_pD3D);
#else
   FORCE_RELEASE(m_pD3D); //!! why is this necessary for some setups? is the refcount still off for some settings?
#endif

   /*
    * D3D sets the FPU to single precision/round to nearest int mode when it's initialized,
    * but doesn't bother to reset the FPU when it's destroyed. We reset it manually here.
    */
   _fpreset();

   if (m_dwm_was_enabled)
      mDwmEnableComposition(DWM_EC_ENABLECOMPOSITION);
#else
#ifdef ENABLE_VR
    if (m_pHMD)
    {
        turnVROff();
        SaveValueFloat("Player", "VRSlope", m_slope);
        SaveValueFloat("Player", "VROrientation", m_orientation);
        SaveValueFloat("Player", "VRTableX", m_tablex);
        SaveValueFloat("Player", "VRTableY", m_tabley);
        SaveValueFloat("Player", "VRTableZ", m_tablez);
        SaveValueFloat("Player", "VRRoomOrientation", m_roomOrientation);
        SaveValueFloat("Player", "VRRoomX", m_roomx);
        SaveValueFloat("Player", "VRRoomY", m_roomy);
    }
#endif

    SDL_GL_DeleteContext(m_sdl_context);
    SDL_DestroyWindow(m_sdl_playfieldHwnd);
#endif
}

void RenderDevice::BeginScene()
{
#ifndef ENABLE_SDL
   CHECKD3D(m_pD3DDevice->BeginScene());
#endif
}

void RenderDevice::EndScene()
{
#ifndef ENABLE_SDL
   CHECKD3D(m_pD3DDevice->EndScene());
#endif
}

/*static void FlushGPUCommandBuffer(IDirect3DDevice9* pd3dDevice)
{
   IDirect3DQuery9* pEventQuery;
   pd3dDevice->CreateQuery(D3DQUERYTYPE_EVENT, &pEventQuery);

   if (pEventQuery)
   {
      pEventQuery->Issue(D3DISSUE_END);
      while (S_FALSE == pEventQuery->GetData(nullptr, 0, D3DGETDATA_FLUSH))
         ;
      SAFE_RELEASE(pEventQuery);
   }
}*/

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
#ifdef ENABLE_SDL
   SDL_GL_SwapWindow(m_sdl_playfieldHwnd);
#ifdef ENABLE_VR
   //glFlush(); //!! ??
   //glFinish();
#endif

#else

   bool dwm = false;
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

   CHECKD3D(m_pD3DDevice->Present(nullptr, nullptr, nullptr, nullptr)); //!! could use D3DPRESENT_DONOTWAIT and do some physics work meanwhile??

   if (mDwmFlush && vsync && dwm)
      mDwmFlush(); //!! also above present?? (internet sources are not clear about order)
#endif
   // reset performance counters
   m_frameDrawCalls = m_curDrawCalls;
   m_frameStateChanges = m_curStateChanges;
   m_frameTextureChanges = m_curTextureChanges;
   m_frameParameterChanges = m_curParameterChanges;
   m_frameTechniqueChanges = m_curTechniqueChanges;
   m_curDrawCalls = m_curStateChanges = m_curTextureChanges = m_curParameterChanges = m_curTechniqueChanges = 0;
   m_frameTextureUpdates = m_curTextureUpdates;
   m_curTextureUpdates = 0;

   m_frameLockCalls = m_curLockCalls;
   m_curLockCalls = 0;
}

#ifdef ENABLE_SDL
void RenderDevice::UploadAndSetSMAATextures()
{
   m_SMAAsearchTexture = CreateTexture(SEARCHTEX_WIDTH, SEARCHTEX_HEIGHT, 0, STATIC, GREY, (void*)&searchTexBytes[0], 0, TextureFilter::TEXTURE_MODE_BILINEAR, false, false);
   m_SMAAareaTexture = CreateTexture(AREATEX_WIDTH, AREATEX_HEIGHT, 0, STATIC, GREY_ALPHA, (void*)&areaTexBytes[0], 0, TextureFilter::TEXTURE_MODE_BILINEAR, false, false);

   FBShader->SetTexture(SHADER_areaTex2D, m_SMAAareaTexture, true);
   FBShader->SetTexture(SHADER_searchTex2D, m_SMAAsearchTexture, true);
}
#else
void RenderDevice::UploadAndSetSMAATextures()
{
   {
      IDirect3DTexture9 *sysTex, *tex;
      HRESULT hr = m_pD3DDevice->CreateTexture(SEARCHTEX_WIDTH, SEARCHTEX_HEIGHT, 0, 0, (D3DFORMAT)colorFormat::GREY8, (D3DPOOL)memoryPool::SYSTEM, &sysTex, nullptr);
      if (FAILED(hr))
         ReportError("Fatal Error: unable to create texture!", hr, __FILE__, __LINE__);
      hr = m_pD3DDevice->CreateTexture(SEARCHTEX_WIDTH, SEARCHTEX_HEIGHT, 0, 0, (D3DFORMAT)colorFormat::GREY8, (D3DPOOL)memoryPool::DEFAULT, &tex, nullptr);
      if (FAILED(hr))
         ReportError("Fatal Error: out of VRAM!", hr, __FILE__, __LINE__);

      //!! use D3DXLoadSurfaceFromMemory
      D3DLOCKED_RECT locked;
      CHECKD3D(sysTex->LockRect(0, &locked, nullptr, 0));
      void* const pdest = locked.pBits;
      const void* const psrc = searchTexBytes;
      memcpy(pdest, psrc, SEARCHTEX_SIZE);
      CHECKD3D(sysTex->UnlockRect(0));

      CHECKD3D(m_pD3DDevice->UpdateTexture(sysTex, tex));
      SAFE_RELEASE(sysTex);

      m_SMAAsearchTexture = new Sampler(this, tex, true, true);
   }
   //
   {
      IDirect3DTexture9 *sysTex, *tex;
      HRESULT hr = m_pD3DDevice->CreateTexture(AREATEX_WIDTH, AREATEX_HEIGHT, 0, 0, (D3DFORMAT)colorFormat::GREYA8, (D3DPOOL)memoryPool::SYSTEM, &sysTex, nullptr);
      if (FAILED(hr))
         ReportError("Fatal Error: unable to create texture!", hr, __FILE__, __LINE__);
      hr = m_pD3DDevice->CreateTexture(AREATEX_WIDTH, AREATEX_HEIGHT, 0, 0, (D3DFORMAT)colorFormat::GREYA8, (D3DPOOL)memoryPool::DEFAULT, &tex, nullptr);
      if (FAILED(hr))
         ReportError("Fatal Error: out of VRAM!", hr, __FILE__, __LINE__);

      //!! use D3DXLoadSurfaceFromMemory
      D3DLOCKED_RECT locked;
      CHECKD3D(sysTex->LockRect(0, &locked, nullptr, 0));
      void* const pdest = locked.pBits;
      const void* const psrc = areaTexBytes;
      memcpy(pdest, psrc, AREATEX_SIZE);
      CHECKD3D(sysTex->UnlockRect(0));

      CHECKD3D(m_pD3DDevice->UpdateTexture(sysTex, tex));
      SAFE_RELEASE(sysTex);

      m_SMAAareaTexture = new Sampler(this, tex, true, true);
   }
}
#endif

void RenderDevice::SetSamplerState(const DWORD Sampler, const D3DSAMPLERSTATETYPE Type, const DWORD Value)
{
#ifdef ENABLE_SDL //!! ??
/*   glSamplerParameteri(Sampler, GL_TEXTURE_MIN_FILTER, minFilter ? (mipFilter ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR) : GL_NEAREST);
   glSamplerParameteri(Sampler, GL_TEXTURE_MAG_FILTER, magFilter ? (mipFilter ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR) : GL_NEAREST);
   m_curStateChanges += 2;*/
#else
   const bool invalid_set = ((unsigned int)Type >= TEXTURE_SAMPLER_CACHE_SIZE || Sampler >= TEXTURE_SAMPLERS);
   if (invalid_set || textureSamplerCache[Sampler][Type] != Value)
   {
      CHECKD3D(m_pD3DDevice->SetSamplerState(Sampler, Type, Value));
      if (!invalid_set)
         textureSamplerCache[Sampler][Type] = Value;

      m_curStateChanges++;
   }
#endif
}

void RenderDevice::SetTextureFilter(const DWORD texUnit, DWORD mode)
{
   // user can override the standard/faster-on-low-end trilinear by aniso filtering
   if ((mode == TEXTURE_MODE_TRILINEAR) && m_force_aniso)
      mode = TEXTURE_MODE_ANISOTROPIC;

   // if in static rendering mode, use the oversampling there to do the texture 'filtering' (i.e. more sharp and crisp than aniso)
   if (mode == TEXTURE_MODE_ANISOTROPIC || mode == TEXTURE_MODE_TRILINEAR)
      if (g_pplayer->m_isRenderingStatic)
      {
          SetSamplerState(texUnit, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
          SetSamplerState(texUnit, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
          SetSamplerState(texUnit, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
          return;
      }

   //

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
   if ((unsigned int)p2 < TEXTURE_STATE_CACHE_SIZE && p1 < TEXTURE_SAMPLERS)
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

inline bool RenderDevice::SetRenderStateCache(const RenderStates p1, DWORD p2)
{
#ifdef DEBUG
   if (p1 >= RENDERSTATE_COUNT)
      return false;//Throw error or similar?
#endif
   const robin_hood::unordered_map<RenderStates, DWORD>::const_iterator it = renderStateCache.find(p1);
   if (it == renderStateCache.end())
   {
      renderStateCache.emplace(std::pair<RenderStates, DWORD>(p1, p2));
      return false;
   }
   else if (it->second != p2) {
      renderStateCache[p1] = p2;
      return false;
   }
   return true;
}

void RenderDevice::SetRenderState(const RenderStates p1, DWORD p2)
{
   if (SetRenderStateCache(p1, p2)) return;

   if (p1 == CULLMODE && (g_pplayer && (g_pplayer->m_ptable->m_tblMirrorEnabled ^ g_pplayer->m_ptable->m_reflectionEnabled)))
   {
      if (p2 == CULL_CCW)
         p2 = CULL_CW;
      else if (p2 == CULL_CW)
         p2 = CULL_CCW;
   }

#ifdef ENABLE_SDL
   switch (p1) {
      //glEnable and glDisable functions
   case ALPHABLENDENABLE:
      if (p2) glEnable(GL_BLEND); else glDisable(GL_BLEND);
      break;
   case ZENABLE:
      if (p2) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
      break;
   case BLENDOP:
      glBlendEquation(p2);
      break;
   case SRCBLEND:
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      break;
   case DESTBLEND:
      glBlendFunc(renderStateCache[SRCBLEND], renderStateCache[DESTBLEND]);
      break;
   case ZFUNC:
      glDepthFunc(p2);
      break;
   case ZWRITEENABLE:
      glDepthMask(p2 ? GL_TRUE : GL_FALSE);
      break;
   case COLORWRITEENABLE:
      glColorMask((p2 & 1) ? GL_TRUE : GL_FALSE, (p2 & 2) ? GL_TRUE : GL_FALSE, (p2 & 4) ? GL_TRUE : GL_FALSE, (p2 & 8) ? GL_TRUE : GL_FALSE);
      break;
      //Replaced by specific function
   case DEPTHBIAS:
   case CULLMODE:
   case CLIPPLANEENABLE:
   case ALPHAFUNC:
   case ALPHATESTENABLE:
      //No effect or not implemented in OpenGL 
   case LIGHTING:
   case CLIPPING:
   case ALPHAREF:
   case SRGBWRITEENABLE:
   default:
      break;
   }
#else
   CHECKD3D(m_pD3DDevice->SetRenderState((D3DRENDERSTATETYPE)p1, p2));
#endif
   m_curStateChanges++;
}

void RenderDevice::SetRenderStateCulling(RenderStateValue cull)
{
   if (SetRenderStateCache(CULLMODE, cull)) return;

   if (g_pplayer && (g_pplayer->m_ptable->m_tblMirrorEnabled ^ g_pplayer->m_ptable->m_reflectionEnabled))
   {
      if (cull == CULL_CCW)
         cull = CULL_CW;
      else if (cull == CULL_CW)
         cull = CULL_CCW;
   }

#ifdef ENABLE_SDL
   if (renderStateCache[RenderStates::CULLMODE] == CULL_NONE && (cull != CULL_NONE)) //!! this differs a bit from VPVR now, recheck!
      glEnable(GL_CULL_FACE);
   if (cull == CULL_NONE)
      glDisable(GL_CULL_FACE);
   else {
      glFrontFace(cull);
      glCullFace(GL_FRONT);
   }
#else
   m_pD3DDevice->SetRenderState((D3DRENDERSTATETYPE)CULLMODE, cull);
#endif
   m_curStateChanges++;
}

void RenderDevice::SetRenderStateDepthBias(float bias)
{
   if (SetRenderStateCache(DEPTHBIAS, *((DWORD*)&bias))) return;

#ifdef ENABLE_SDL
   if (bias == 0.0f)
      glDisable(GL_POLYGON_OFFSET_FILL);
   else {
      glEnable(GL_POLYGON_OFFSET_FILL);
      glPolygonOffset(0.0f, bias);
   }
#else
   bias *= BASEDEPTHBIAS;
   CHECKD3D(m_pD3DDevice->SetRenderState((D3DRENDERSTATETYPE)DEPTHBIAS, *((DWORD*)&bias)));
#endif
   m_curStateChanges++;
}

void RenderDevice::SetRenderStateClipPlane0(const bool enabled)
{
   if (SetRenderStateCache(CLIPPLANEENABLE, enabled ? PLANE0 : 0)) return;

#ifdef ENABLE_SDL
   // Basicshader already prepared with proper clipplane so just need to enable/disable it
   if (enabled)
      glEnable(GL_CLIP_DISTANCE0);
   else
      glDisable(GL_CLIP_DISTANCE0);
#else
   CHECKD3D(m_pD3DDevice->SetRenderState((D3DRENDERSTATETYPE)CLIPPLANEENABLE, enabled ? PLANE0 : 0));
#endif 
   m_curStateChanges++;
}

void RenderDevice::SetRenderStateAlphaTestFunction(const DWORD testValue, const RenderStateValue testFunction, const bool enabled)
{
#ifdef ENABLE_SDL
   //!! TODO Needs to be done in shader
#else 
   if (!SetRenderStateCache(ALPHAREF, testValue))
      CHECKD3D(m_pD3DDevice->SetRenderState((D3DRENDERSTATETYPE)ALPHAREF, testValue));
   if (!SetRenderStateCache(ALPHATESTENABLE, enabled ? RS_TRUE : RS_FALSE))
      CHECKD3D(m_pD3DDevice->SetRenderState((D3DRENDERSTATETYPE)ALPHATESTENABLE, enabled ? RS_TRUE : RS_FALSE));
   if (!SetRenderStateCache(ALPHAFUNC, testFunction))
      CHECKD3D(m_pD3DDevice->SetRenderState((D3DRENDERSTATETYPE)ALPHAFUNC, testFunction));
#endif
}

void RenderDevice::SetTextureAddressMode(const DWORD texUnit, const TextureAddressMode mode)
{
   SetSamplerState(texUnit, D3DSAMP_ADDRESSU, mode);
   SetSamplerState(texUnit, D3DSAMP_ADDRESSV, mode);
}

void RenderDevice::DrawPrimitive(const PrimitiveTypes type, const DWORD fvf, const void* vertices, const DWORD vertexCount)
{
#ifndef ENABLE_SDL
   const unsigned int np = ComputePrimitiveCount(type, vertexCount);
   m_stats_drawn_triangles += np;

   VertexDeclaration * declaration = fvfToDecl(fvf);
   SetVertexDeclaration(declaration);

   HRESULT hr = m_pD3DDevice->DrawPrimitiveUP((D3DPRIMITIVETYPE)type, np, vertices, fvfToSize(fvf));

   if (FAILED(hr))
      ReportError("Fatal Error: DrawPrimitiveUP failed!", hr, __FILE__, __LINE__);

   VertexBuffer::bindNull();    // DrawPrimitiveUP sets the VB to nullptr

   m_curDrawCalls++;
#endif
}

void RenderDevice::DrawTexturedQuad(const Vertex3D_TexelOnly* vertices)
{
   /*Vertex3D_TexelOnly* bufvb;
   m_quadDynVertexBuffer->lock(0, 0, (void**)&bufvb, VertexBuffer::DISCARDCONTENTS);
   memcpy(bufvb,vertices,4*sizeof(Vertex3D_TexelOnly));
   m_quadDynVertexBuffer->unlock();
   DrawPrimitiveVB(RenderDevice::TRIANGLESTRIP,MY_D3DFVF_TEX,m_quadDynVertexBuffer,0,4,true);*/

   DrawPrimitive(RenderDevice::TRIANGLESTRIP,MY_D3DFVF_TEX,vertices,4); // having a VB and lock/copying stuff each time is slower :/
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

   DrawPrimitiveVB(RenderDevice::TRIANGLESTRIP,MY_D3DFVF_TEX,m_quadVertexBuffer,0,4,false);
}

void RenderDevice::DrawPrimitiveVB(const PrimitiveTypes type, const DWORD fvf, VertexBuffer* vb, const DWORD startVertex, const DWORD vertexCount, const bool stereo)
{
   const unsigned int np = ComputePrimitiveCount(type, vertexCount);
   m_stats_drawn_triangles += np;

   vb->bind();
#ifdef ENABLE_SDL
   //glDrawArraysInstanced(type, vb->getOffset() + startVertex, vertexCount, m_stereo3D != STEREO_OFF ? 2 : 1); // Do instancing in geometry shader instead
   glDrawArrays(type, vb->getOffset() + startVertex, vertexCount);
#else
   VertexDeclaration * declaration = fvfToDecl(fvf);
   SetVertexDeclaration(declaration);

   const HRESULT hr = m_pD3DDevice->DrawPrimitive((D3DPRIMITIVETYPE)type, startVertex, np);
   if (FAILED(hr))
      ReportError("Fatal Error: DrawPrimitive failed!", hr, __FILE__, __LINE__);
#endif
   m_curDrawCalls++;
}

void RenderDevice::DrawIndexedPrimitiveVB(const PrimitiveTypes type, const DWORD fvf, VertexBuffer* vb, const DWORD startVertex, const DWORD vertexCount, IndexBuffer* ib, const DWORD startIndex, const DWORD indexCount)
{
   if (vb == nullptr || ib == nullptr) //!! happens for primitives that are grouped on player init render call?!?
      return;

   const unsigned int np = ComputePrimitiveCount(type, indexCount);
   m_stats_drawn_triangles += np;

#ifdef ENABLE_SDL
   const int offset = ib->getOffset() + (ib->getIndexFormat() == IndexBuffer::FMT_INDEX16 ? 2 : 4) * startIndex;
   //glDrawElementsInstancedBaseVertex(type, indexCount, ib->getIndexFormat() == IndexBuffer::FMT_INDEX16 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, (void*)offset, m_stereo3D != STEREO_OFF ? 2 : 1, vb->getOffset() + startVertex); // Do instancing in geometry shader instead
   glDrawElementsBaseVertex(type, indexCount, ib->getIndexFormat() == IndexBuffer::FMT_INDEX16 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, (void*)offset, vb->getOffset() + startVertex);
#else
   VertexDeclaration * declaration = fvfToDecl(fvf);
   SetVertexDeclaration(declaration);

   // bind the vertex and index buffers
   if (VertexBuffer::m_curVertexBuffer != vb)
   {
      const unsigned int vsize = fvfToSize(fvf);
      CHECKD3D(m_pD3DDevice->SetStreamSource(0, vb->m_vb, 0, vsize));
      VertexBuffer::m_curVertexBuffer = vb;
   }

   if (IndexBuffer::m_curIndexBuffer != ib)
   {
      CHECKD3D(m_pD3DDevice->SetIndices(ib->m_ib));
      IndexBuffer::m_curIndexBuffer = ib;
   }

   // render
   CHECKD3D(m_pD3DDevice->DrawIndexedPrimitive((D3DPRIMITIVETYPE)type, startVertex, 0, vertexCount, startIndex, np));
#endif
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

Shader::Shader(RenderDevice *renderDevice) : currentMaterial(-FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX,
                                                             0xCCCCCCCC, 0xCCCCCCCC, 0xCCCCCCCC, false, false, -FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX)
{
   m_renderDevice = renderDevice;
   m_shader = nullptr;
   for (unsigned int i = 0; i < TEXTURESET_STATE_CACHE_SIZE; ++i)
      currentTexture[i] = nullptr;
   currentAlphaTestValue = -FLT_MAX;
   currentDisableLighting =
   currentFlasherData =
   currentFlasherData2 =
   currentFlasherColor =
   currentLightColor =
   currentLightColor2 =
   currentLightData = vec4(-FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);
   currentLightImageMode = ~0u;
   currentLightBackglassMode = ~0u;
   currentTechnique[0] = '\0';
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
   constexpr DWORD dwShaderFlags = 0; //D3DXSHADER_SKIPVALIDATION // these do not have a measurable effect so far (also if used in the offline fxc step): D3DXSHADER_PARTIALPRECISION, D3DXSHADER_PREFER_FLOW_CONTROL/D3DXSHADER_AVOID_FLOW_CONTROL
   HRESULT hr;
   /*
       if(fromFile)
       {
       dwShaderFlags = D3DXSHADER_DEBUG|D3DXSHADER_SKIPOPTIMIZATION;
       hr = D3DXCreateEffectFromFile(m_renderDevice->GetCoreDevice(),		// pDevice
       shaderName,			// pSrcFile
       nullptr,				// pDefines
       nullptr,				// pInclude
       dwShaderFlags,		// Flags
       nullptr,				// pPool
       &m_shader,			// ppEffect
       &pBufferErrors);		// ppCompilationErrors
       }
       else
       {
       hr = D3DXCreateEffectFromResource(m_renderDevice->GetCoreDevice(),		// pDevice
       nullptr,
       shaderName,			// resource name
       nullptr,				// pDefines
       nullptr,				// pInclude
       dwShaderFlags,		// Flags
       nullptr,				// pPool
       &m_shader,			// ppEffect
       &pBufferErrors);		// ppCompilationErrors

       }
       */
   hr = D3DXCreateEffect(m_renderDevice->GetCoreDevice(), shaderCodeName, codeSize, nullptr, nullptr, dwShaderFlags, nullptr, &m_shader, &pBufferErrors);
   if (FAILED(hr))
   {
      if (pBufferErrors)
      {
         const LPVOID pCompileErrors = pBufferErrors->GetBufferPointer();
         g_pvp->MessageBox((const char*)pCompileErrors, "Compile Error", MB_OK | MB_ICONEXCLAMATION);
      }
      else
         g_pvp->MessageBox("Unknown Error", "Compile Error", MB_OK | MB_ICONEXCLAMATION);

      return false;
   }
   return true;
}

void Shader::Unload()
{
   SAFE_RELEASE(m_shader);
}

void Shader::SetTexture(const SHADER_UNIFORM_HANDLE texelName, Texture* texel, const TextureFilter filter, const bool clampU, const bool clampV, const bool force_linear_rgb)
{
   const unsigned int idx = texelName[strlen(texelName) - 1] - '0'; // current convention: SetTexture gets "TextureX", where X 0..4
   assert(idx < TEXTURESET_STATE_CACHE_SIZE);

   if (!texel || !texel->m_pdsBuffer) {
      currentTexture[idx] = nullptr; // invalidate the cache

      CHECKD3D(m_shader->SetTexture(texelName, nullptr));

      m_renderDevice->m_curTextureChanges++;

      return;
   }

   if (texel->m_pdsBuffer != currentTexture[idx])
   {
      currentTexture[idx] = texel->m_pdsBuffer;
      CHECKD3D(m_shader->SetTexture(texelName, m_renderDevice->m_texMan.LoadTexture(texel->m_pdsBuffer, filter, clampU, clampV, force_linear_rgb)->GetCoreTexture()));

      m_renderDevice->m_curTextureChanges++;
   }
}

void Shader::SetTexture(const SHADER_UNIFORM_HANDLE texelName, Sampler* texel)
{
   const unsigned int idx = texelName[strlen(texelName) - 1] - '0'; // current convention: SetTexture gets "TextureX", where X 0..4
   assert(idx < TEXTURESET_STATE_CACHE_SIZE);

   currentTexture[idx] = nullptr; // direct set of device tex invalidates the cache

   CHECKD3D(m_shader->SetTexture(texelName, texel->GetCoreTexture()));

   m_renderDevice->m_curTextureChanges++;
}

void Shader::SetTextureNull(const SHADER_UNIFORM_HANDLE texelName)
{
   const unsigned int idx = texelName[strlen(texelName) - 1] - '0'; // current convention: SetTexture gets "TextureX", where X 0..4
   const bool cache = (idx < TEXTURESET_STATE_CACHE_SIZE);

   if (cache)
      currentTexture[idx] = nullptr; // direct set of device tex invalidates the cache

   CHECKD3D(m_shader->SetTexture(texelName, nullptr));

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
      cBase = g_pvp->m_dummyMaterial.m_cBase;
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
      const vec4 rwem(fRoughness, fWrapLighting, fEdge, fThickness);
      SetVector(SHADER_Roughness_WrapL_Edge_Thickness, &rwem);
      currentMaterial.m_fRoughness = fRoughness;
      currentMaterial.m_fWrapLighting = fWrapLighting;
      currentMaterial.m_fEdge = fEdge;
      currentMaterial.m_fThickness = fThickness;
   }

   const float alpha = bOpacityActive ? fOpacity : 1.0f;
   if (cBase != currentMaterial.m_cBase || alpha != currentMaterial.m_fOpacity)
   {
      const vec4 cBaseF = convertColor(cBase, alpha);
      SetVector(SHADER_cBase_Alpha, &cBaseF);
      currentMaterial.m_cBase = cBase;
      currentMaterial.m_fOpacity = alpha;
   }

   if (!bIsMetal) // Metal has no glossy
      if (cGlossy != currentMaterial.m_cGlossy ||
          fGlossyImageLerp != currentMaterial.m_fGlossyImageLerp)
      {
         const vec4 cGlossyF = convertColor(cGlossy, fGlossyImageLerp);
         SetVector(SHADER_cGlossy_ImageLerp, &cGlossyF);
         currentMaterial.m_cGlossy = cGlossy;
         currentMaterial.m_fGlossyImageLerp = fGlossyImageLerp;
      }

   if (cClearcoat != currentMaterial.m_cClearcoat ||
      (bOpacityActive && fEdgeAlpha != currentMaterial.m_fEdgeAlpha))
   {
      const vec4 cClearcoatF = convertColor(cClearcoat, fEdgeAlpha);
      SetVector(SHADER_cClearcoat_EdgeAlpha, &cClearcoatF);
      currentMaterial.m_cClearcoat = cClearcoat;
      currentMaterial.m_fEdgeAlpha = fEdgeAlpha;
   }

   if (bOpacityActive /*&& (alpha < 1.0f)*/)
      g_pplayer->m_pin3d.EnableAlphaBlend(false);
   else
      g_pplayer->m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::ALPHABLENDENABLE, RenderDevice::RS_FALSE);
}
