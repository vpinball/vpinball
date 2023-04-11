#include "stdafx.h"

#include <DxErr.h>

// Undefine this if you want to debug VR mode without a VR headset
//#define VR_PREVIEW_TEST

//#include "Dwmapi.h" // use when we get rid of XP at some point, get rid of the manual dll loads in here then

#ifndef DISABLE_FORCE_NVIDIA_OPTIMUS
#include "inc/nvapi.h"
#endif

#include "RenderDevice.h"
#include "RenderCommand.h"
#include "Shader.h"
#include "shader/AreaTex.h"
#include "shader/SearchTex.h"

#if defined(ENABLE_SDL) // OpenGL
#include "typedefs3D.h"
#include "TextureManager.h"
#include <SDL2/SDL_syswm.h>

#else // DirectX 9
#include "Material.h"
#include "BasicShader.h"
#include "DMDShader.h"
#include "FBShader.h"
#include "FlasherShader.h"
#include "LightShader.h"
#endif


#if defined(ENABLE_SDL) // OpenGL
GLuint RenderDevice::m_samplerStateCache[3 * 3 * 5];

#else // DirectX 9
#pragma comment(lib, "d3d9.lib")        // TODO: put into build system
#pragma comment(lib, "d3dx9.lib")       // TODO: put into build system
#if _MSC_VER >= 1900
 #pragma comment(lib, "legacy_stdio_definitions.lib") //dxerr.lib needs this
#endif
#pragma comment(lib, "dxerr.lib")       // TODO: put into build system

static bool IsWindowsVistaOr7()
{
   OSVERSIONINFOEXW osvi = { sizeof(osvi), 0, 0, 0, 0,{ 0 }, 0, 0, 0, 0, 0 };
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

   OSVERSIONINFOEXW osvi2 = { sizeof(osvi), 0, 0, 0, 0,{ 0 }, 0, 0, 0, 0, 0 };
   osvi2.dwMajorVersion = HIBYTE(_WIN32_WINNT_WIN7);
   osvi2.dwMinorVersion = LOBYTE(_WIN32_WINNT_WIN7);
   //osvi2.wServicePackMajor = 0;

   const bool win7 = VerifyVersionInfoW(&osvi2, VER_MAJORVERSION | VER_MINORVERSION /*| VER_SERVICEPACKMAJOR*/, dwlConditionMask) != FALSE;

   return vista || win7;
}

constexpr D3DVERTEXELEMENT9 VertexTexelElement[] =
{
   { 0, 0 * sizeof(float), D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },  // pos
   { 0, 3 * sizeof(float), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },  // tex0
   D3DDECL_END()
};

constexpr D3DVERTEXELEMENT9 VertexNormalTexelElement[] =
{
   { 0, 0 * sizeof(float), D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },  // pos
   { 0, 3 * sizeof(float), D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0 },  // normal
   { 0, 6 * sizeof(float), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },  // tex0
   D3DDECL_END()
};

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

static unsigned int ComputePrimitiveCount(const RenderDevice::PrimitiveTypes type, const int vertexCount)
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
#ifndef __OPENGLES__
   case GL_STACK_OVERFLOW: return "GL_STACK_OVERFLOW";
   case GL_STACK_UNDERFLOW: return "GL_STACK_UNDERFLOW";
#endif
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
   sprintf_s(msg, sizeof(msg), "GL Fatal Error 0x%0002X %s in %s:%d", hr, glErrorToString(hr), file, line);
#else
   sprintf_s(msg, sizeof(msg), "Fatal error %s (0x%x: %s) at %s:%d", DXGetErrorString(hr), hr, DXGetErrorDescription(hr), file, line);
#endif
   ShowError(msg);
   assert(false);
   exit(-1);
}

void ReportError(const char *errorText, const HRESULT hr, const char *file, const int line)
{
   char msg[4096];
#ifdef ENABLE_SDL
   sprintf_s(msg, sizeof(msg), "GL Error 0x%0002X %s in %s:%d\n%s", hr, glErrorToString(hr), file, line, errorText);
   ShowError(msg);
#else
   sprintf_s(msg, sizeof(msg), "%s %s (0x%x: %s) at %s:%d", errorText, DXGetErrorString(hr), hr, DXGetErrorDescription(hr), file, line);
   ShowError(msg);
   exit(-1);
#endif
}

unsigned m_curLockCalls, m_frameLockCalls;
unsigned int RenderDevice::Perf_GetNumLockCalls() const { return m_frameLockCalls; }

#if 0 //def ENABLE_SDL // not used anymore
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
#ifndef __OPENGLES__
void APIENTRY GLDebugMessageCallback(GLenum source, GLenum type, GLuint id,
                                     GLenum severity, GLsizei length,
                                     const GLchar *msg, const void *data)
{
    char* _source;
    switch (source) {
        case GL_DEBUG_SOURCE_API:
        _source = (LPSTR) "API";
        break;

        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        _source = (LPSTR) "WINDOW SYSTEM";
        break;

        case GL_DEBUG_SOURCE_SHADER_COMPILER:
        _source = (LPSTR) "SHADER COMPILER";
        break;

        case GL_DEBUG_SOURCE_THIRD_PARTY:
        _source = (LPSTR) "THIRD PARTY";
        break;

        case GL_DEBUG_SOURCE_APPLICATION:
        _source = (LPSTR) "APPLICATION";
        break;

        case GL_DEBUG_SOURCE_OTHER:
        _source = (LPSTR) "UNKNOWN";
        break;

        default:
        _source = (LPSTR) "UNHANDLED";
        break;
    }

    char* _type;
    switch (type) {
        case GL_DEBUG_TYPE_ERROR:
        _type = (LPSTR) "ERROR";
        break;

        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        _type = (LPSTR) "DEPRECATED BEHAVIOR";
        break;

        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        _type = (LPSTR) "UNDEFINED BEHAVIOR";
        break;

        case GL_DEBUG_TYPE_PORTABILITY:
        _type = (LPSTR) "PORTABILITY";
        break;

        case GL_DEBUG_TYPE_PERFORMANCE:
        _type = (LPSTR) "PERFORMANCE";
        break;

        case GL_DEBUG_TYPE_OTHER:
        _type = (LPSTR) "OTHER";
        break;

        case GL_DEBUG_TYPE_MARKER:
        _type = (LPSTR) "MARKER";
        break;

        case GL_DEBUG_TYPE_PUSH_GROUP:
        _type = (LPSTR) "GL_DEBUG_TYPE_PUSH_GROUP";
        break;

        case GL_DEBUG_TYPE_POP_GROUP:
        _type = (LPSTR) "GL_DEBUG_TYPE_POP_GROUP";
        break;

    	default:
        _type = (LPSTR) "UNHANDLED";
        break;
    }

    char* _severity;
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
        _severity = (LPSTR) "HIGH";
        break;

        case GL_DEBUG_SEVERITY_MEDIUM:
        _severity = (LPSTR) "MEDIUM";
        break;

        case GL_DEBUG_SEVERITY_LOW:
        _severity = (LPSTR) "LOW";
        break;

        case GL_DEBUG_SEVERITY_NOTIFICATION:
        _severity = (LPSTR) "NOTIFICATION";
        break;

        default:
        _severity = (LPSTR) "UNHANDLED";
        break;
    }

    //if(severity != GL_DEBUG_SEVERITY_NOTIFICATION)
    fprintf(stderr,"%d: %s of %s severity, raised from %s: %s\n", id, _type, _severity, _source, msg);

    if (type == GL_DEBUG_TYPE_ERROR || type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR || severity == GL_DEBUG_SEVERITY_HIGH)
        ShowError(msg);
}
#endif
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

void EnumerateDisplayModes(const int display, vector<VideoMode>& modes)
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
   vector<DisplayConfig> displays;
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
   constexpr int j = 0; // limit to 32bit only nowadays
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
   std::map<string,DisplayConfig>* data = reinterpret_cast<std::map<string,DisplayConfig>*>(dwData);
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
   data->insert(std::pair<string, DisplayConfig>(config.DeviceName, config));
   return TRUE;
}

int getDisplayList(vector<DisplayConfig>& displays)
{
   displays.clear();

#ifdef _MSC_VER
   std::map<string, DisplayConfig> displayMap;
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
      std::map<string, DisplayConfig>::iterator display = displayMap.find(adapter.DeviceName);
      if (display != displayMap.end()) {
         display->second.adapter = i;
         strncpy_s(display->second.GPU_Name, adapter.Description, sizeof(display->second.GPU_Name)-1);
      }
   }
   SAFE_RELEASE(pD3D);
#endif

   // Apply the same numbering as windows
   int i = 0;
   for (std::map<string, DisplayConfig>::iterator display = displayMap.begin(); display != displayMap.end(); ++display)
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
#else
   //int maxAdapter = SDL_GetNumVideoDrivers(); //!!
   int i;
   for (i = 0; i < getNumberOfDisplays(); ++i) {
      SDL_Rect displayBounds;
      if (SDL_GetDisplayBounds(i, &displayBounds) == 0) {
         DisplayConfig displayConf;
         displayConf.display = i;
         displayConf.adapter = 0;
         displayConf.isPrimary = (displayBounds.x == 0) && (displayBounds.y == 0);
         displayConf.top = displayBounds.x;
         displayConf.left = displayBounds.x;
         displayConf.width = displayBounds.w;
         displayConf.height = displayBounds.h;

         strncpy_s(displayConf.DeviceName, SDL_GetDisplayName(displayConf.display), 32);
         strncpy_s(displayConf.GPU_Name, SDL_GetVideoDriver(displayConf.adapter), MAX_DEVICE_IDENTIFIER_STRING-1);

         displays.push_back(displayConf);
      }
   }
#endif
   return i;
}

bool getDisplaySetupByID(const int display, int &x, int &y, int &width, int &height)
{
   vector<DisplayConfig> displays;
   getDisplayList(displays);
   for (vector<DisplayConfig>::iterator displayConf = displays.begin(); displayConf != displays.end(); ++displayConf) {
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
   vector<DisplayConfig> displays;
   getDisplayList(displays);
   for (vector<DisplayConfig>::iterator displayConf = displays.begin(); displayConf != displays.end(); ++displayConf)
      if (displayConf->isPrimary)
         return displayConf->adapter;
   return 0;
}

////////////////////////////////////////////////////////////////////

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

RenderDevice::RenderDevice(const HWND hwnd, const int width, const int height, const bool fullscreen, const int colordepth, int VSync, const float AAfactor, const StereoMode stereo3D, const unsigned int FXAA, const bool sharpen, const bool ss_refl, const bool useNvidiaApi, const bool disable_dwm, const int BWrendering)
    : m_windowHwnd(hwnd), m_width(width), m_height(height), m_fullscreen(fullscreen), 
      m_colorDepth(colordepth), m_vsync(VSync), m_AAfactor(AAfactor), m_stereo3D(stereo3D),
      m_ssRefl(ss_refl), m_disableDwm(disable_dwm), m_sharpen(sharpen), m_FXAA(FXAA), m_BWrendering(BWrendering), m_texMan(*this), m_renderFrame(this)
{
#ifdef ENABLE_SDL
#ifdef ENABLE_VR
   m_pHMD = nullptr;
   m_rTrackedDevicePose = nullptr;
#endif
#else
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
#endif

    m_stats_drawn_triangles = 0;

    //m_curShader = nullptr;

    // initialize performance counters
    m_curDrawCalls = m_frameDrawCalls = 0;
    m_curStateChanges = m_frameStateChanges = 0;
    m_curTextureChanges = m_frameTextureChanges = 0;
    m_curParameterChanges = m_frameParameterChanges = 0;
    m_curTechniqueChanges = m_frameTechniqueChanges = 0;
    m_curTextureUpdates = m_frameTextureUpdates = 0;

    m_curLockCalls = m_frameLockCalls = 0; //!! meh

    StereoShader = nullptr;
}

void RenderDevice::CreateDevice(int &refreshrate, UINT adapterIndex)
{
   assert(g_pplayer != nullptr); // Player must be created to give access to the output window
   colorFormat back_buffer_format;

#ifdef ENABLE_SDL
   ///////////////////////////////////
   // OpenGL device initialization
   const int displays = getNumberOfDisplays();
   if ((int)adapterIndex >= displays)
      m_adapter = 0;
   else
      m_adapter = adapterIndex;

   SDL_DisplayMode mode;
   if (SDL_GetCurrentDisplayMode(m_adapter, &mode) != 0)
   {
      ShowError("Failed to setup SDL output window");
      exit(-1);
   }
   refreshrate = mode.refresh_rate;
   bool video10bit = mode.format == SDL_PIXELFORMAT_ARGB2101010;
   switch (mode.format)
   {
   case SDL_PIXELFORMAT_RGB565: back_buffer_format = colorFormat::RGB5; break;
   case SDL_PIXELFORMAT_RGB888: back_buffer_format = colorFormat::RGB8; break;
   case SDL_PIXELFORMAT_ARGB8888: back_buffer_format = colorFormat::RGBA8; break;
   case SDL_PIXELFORMAT_ARGB2101010: back_buffer_format = colorFormat::RGBA10; break;
#ifdef __OPENGLES__
   case SDL_PIXELFORMAT_ABGR8888: back_buffer_format = colorFormat::RGBA8; break;
   case SDL_PIXELFORMAT_RGBX8888: back_buffer_format = colorFormat::RGBA8; break;
#endif
   default: 
   {
      ShowError("Invalid Output format: "s.append(std::to_string(mode.format).c_str()));
      exit(-1);
   }
   }

   memset(m_samplerStateCache, 0, sizeof(m_samplerStateCache));

   m_sdl_playfieldHwnd = g_pplayer->m_sdl_playfieldHwnd;
   SDL_SysWMinfo wmInfo;
   SDL_VERSION(&wmInfo.version);
   SDL_GetWindowWMInfo(m_sdl_playfieldHwnd, &wmInfo);
   m_windowHwnd = wmInfo.info.win.window;

   m_sdl_context = SDL_GL_CreateContext(m_sdl_playfieldHwnd);

   SDL_GL_MakeCurrent(m_sdl_playfieldHwnd, m_sdl_context);

#ifndef __OPENGLES__
   if (!gladLoadGLLoader(SDL_GL_GetProcAddress)) {
#else
   if (!gladLoadGLES2((GLADloadfunc) SDL_GL_GetProcAddress)) {
#endif
      ShowError("Glad failed");
      exit(-1);
   }

#ifdef __OPENGLES__
   int range[2];
   int precision;
   glGetShaderPrecisionFormat(GL_VERTEX_SHADER, GL_HIGH_FLOAT, range, &precision);
   PLOGD.printf("Vertex shader high precision float range: %d %d precision: %d", range[0], range[1], precision);
   glGetShaderPrecisionFormat(GL_FRAGMENT_SHADER, GL_HIGH_FLOAT, range, &precision);
   PLOGD.printf("Fragment shader high precision float range: %d %d precision: %d", range[0], range[1], precision);
#endif

   int gl_majorVersion = 0;
   int gl_minorVersion = 0;
   glGetIntegerv(GL_MAJOR_VERSION, &gl_majorVersion);
   glGetIntegerv(GL_MINOR_VERSION, &gl_minorVersion);

   if (gl_majorVersion < 3 || (gl_majorVersion == 3 && gl_minorVersion < 2)) {
      char errorMsg[256];
      sprintf_s(errorMsg, sizeof(errorMsg), "Your graphics card only supports OpenGL %d.%d, but VPVR requires OpenGL 3.2 or newer.", gl_majorVersion, gl_minorVersion);
      ShowError(errorMsg);
      exit(-1);
   }

   m_GLversion = gl_majorVersion*100 + gl_minorVersion;

   // Enable debugging layer of OpenGL
#ifdef _DEBUG
#ifndef __OPENGLES__
   glEnable(GL_DEBUG_OUTPUT); // on its own is the 'fast' version
   //glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // callback is in sync with errors, so a breakpoint can be placed on the callback in order to get a stacktrace for the GL error
   if (glad_glDebugMessageCallback) {
      glDebugMessageCallback(GLDebugMessageCallback, nullptr);
   }
#endif
#endif
#if 0
   glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_FALSE); // disable all
   glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_ERROR, GL_DONT_CARE, 0, nullptr, GL_TRUE); // enable only errors
#endif

   /* This fails on some situations, just keep the requested size
   GLint frameBuffer[4];
   glGetIntegerv(GL_VIEWPORT, frameBuffer);
   const int fbWidth = frameBuffer[2];
   const int fbHeight = frameBuffer[3];
   m_width = fbWidth;
   m_height = fbHeight;
   */

#ifdef ENABLE_VR
   m_scale = 1.0f;
   if (m_stereo3D == STEREO_VR)
   {
      if (LoadValueBoolWithDefault(regKey[RegName::PlayerVR], "scaleToFixedWidth"s, false))
      {
         float width;
         g_pplayer->m_ptable->get_Width(&width);
         m_scale = LoadValueFloatWithDefault(regKey[RegName::PlayerVR], "scaleAbsolute"s, 55.0f) * 0.01f / width;
      }
      else
         m_scale = 0.000540425f * LoadValueFloatWithDefault(regKey[RegName::PlayerVR], "scaleRelative"s, 1.0f);
      if (m_scale <= 0.000001f)
         m_scale = 0.000540425f; // Scale factor for VPUnits to Meters
      // Initialize VR, this will also override the render buffer size (m_width, m_height) to account for HMD render size and render the 2 eyes simultaneously
      InitVR();
   }
   else 
#endif
   if (m_stereo3D >= STEREO_ANAGLYPH_RC && m_stereo3D <= STEREO_ANAGLYPH_AB)
   {
      // For anaglyph stereo mode, we need to double the width since the 2 eye images are mixed by colors, each being at the resolution of the output.
      m_width = m_width * 2;
   }

   if (m_stereo3D == STEREO_VR || m_vsync > refreshrate)
      m_vsync = 0;
   SDL_GL_SetSwapInterval(m_vsync == 2 ? -1 : !!m_vsync); // 0 for immediate updates, 1 for updates synchronized with the vertical retrace, -1 for adaptive vsync

   m_maxaniso = 0;
   glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &m_maxaniso);
   int max_frag_unit, max_combined_unit;
   glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_frag_unit);
   glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &max_combined_unit);
   int n_tex_units = min(max_frag_unit, max_combined_unit);
   for (int i = 0; i < n_tex_units; i++)
   {
      SamplerBinding* binding = new SamplerBinding();
      binding->unit = i;
      binding->use_rank = i;
      binding->sampler = nullptr;
      binding->filter = SF_UNDEFINED;
      binding->clamp_u = SA_UNDEFINED;
      binding->clamp_v = SA_UNDEFINED;
      m_samplerBindings.push_back(binding);
   }
   m_autogen_mipmap = true;

   SetRenderState(RenderState::ZFUNC, RenderState::Z_LESSEQUAL);

#else
   ///////////////////////////////////
   // DirectX 9 device initialization

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

   m_adapter = m_pD3D->GetAdapterCount() > adapterIndex ? adapterIndex : 0;

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
   memset(m_bound_filter, 0xCC, TEXTURESET_STATE_CACHE_SIZE * sizeof(SamplerFilter));
   memset(m_bound_clampu, 0xCC, TEXTURESET_STATE_CACHE_SIZE * sizeof(SamplerAddressMode));
   memset(m_bound_clampv, 0xCC, TEXTURESET_STATE_CACHE_SIZE * sizeof(SamplerAddressMode));

   if (((caps.TextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL) != 0) || ((caps.TextureCaps & D3DPTEXTURECAPS_POW2) != 0))
      ShowError("D3D device does only support power of 2 textures");

   //if (caps.NumSimultaneousRTs < 2)
   //   ShowError("D3D device doesn't support multiple render targets!");

   bool video10bit = LoadValueBoolWithDefault(regKey[RegName::Player], "Render10Bit"s, false);

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
   switch (format)
   {
   case D3DFMT_R5G6B5: back_buffer_format = colorFormat::RGB5; break;
   case D3DFMT_X8R8G8B8: back_buffer_format = colorFormat::RGB8; break;
   case D3DFMT_A8R8G8B8: back_buffer_format = colorFormat::RGBA8; break;
   case D3DFMT_A2R10G10B10: back_buffer_format = colorFormat::RGBA10; break;
   default:
   {
      ShowError("Invalid Output format: "s.append(std::to_string(format).c_str()));
      exit(-1);
   }
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
   if (!NVAPIinit && NvAPI_Initialize() == NVAPI_OK)
      NVAPIinit = true;
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

   const bool softwareVP = LoadValueBoolWithDefault(regKey[RegName::Player], "SoftwareVertexProcessing"s, false);
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
#endif

   // Retrieve a reference to the back buffer.
   int back_buffer_width, back_buffer_height;
#ifdef ENABLE_SDL
   SDL_GL_GetDrawableSize(m_sdl_playfieldHwnd, &back_buffer_width, &back_buffer_height);
#else
   back_buffer_width = m_width;
   back_buffer_height = m_height;
#endif
   m_pBackBuffer = new RenderTarget(this, back_buffer_width, back_buffer_height, back_buffer_format);

#ifdef ENABLE_SDL
   const colorFormat render_format = ((m_BWrendering == 1) ? colorFormat::RG16F : ((m_BWrendering == 2) ? colorFormat::RED16F : colorFormat::RGB16F));
#else
   const colorFormat render_format = ((m_BWrendering == 1) ? colorFormat::RG16F : ((m_BWrendering == 2) ? colorFormat::RED16F : colorFormat::RGBA16F));
#endif
   // alloc float buffer for rendering (optionally AA factor res for manual super sampling)
   int m_width_aa = (int)((float)m_width * m_AAfactor);
   int m_height_aa = (int)((float)m_height * m_AAfactor);

   // alloc float buffer for rendering
   int nMSAASamples = (g_pplayer != nullptr) ? g_pplayer->m_MSAASamples : 1;
#ifdef ENABLE_SDL
   int maxSamples;
   glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
   nMSAASamples = min(maxSamples, nMSAASamples);
#endif
   m_pOffscreenMSAABackBufferTexture = new RenderTarget(this, "OffscreenMSAABackBuffer"s, m_width_aa, m_height_aa, render_format, true, nMSAASamples, m_stereo3D, "Fatal Error: unable to create render buffer!");

   // If we are doing MSAA we need a texture with the same dimensions as the Back Buffer to resolve the end result to, can also use it for Post-AA
   if (nMSAASamples > 1)
      m_pOffscreenBackBufferTexture = new RenderTarget(this, "OffscreenBackBuffer"s, m_width_aa, m_height_aa, render_format, true, 1, m_stereo3D, "Fatal Error: unable to create MSAA resolve buffer!");
   else
      m_pOffscreenBackBufferTexture = m_pOffscreenMSAABackBufferTexture;

   // alloc buffer for screen space fake reflection rendering
   if (m_ssRefl)
      m_pReflectionBufferTexture = new RenderTarget(this, "ReflectionBuffer"s, m_width_aa, m_height_aa, render_format, false, 1, STEREO_OFF, "Fatal Error: unable to create reflection buffer!");

   // alloc bloom tex at 1/4 x 1/4 res (allows for simple HQ downscale of clipped input while saving memory)
   m_pBloomBufferTexture = new RenderTarget(this, "BloomBuffer1"s, m_width / 4, m_height / 4, render_format, false, 1, m_stereo3D, "Fatal Error: unable to create bloom buffer!");
   m_pBloomTmpBufferTexture = m_pBloomBufferTexture->Duplicate("BloomBuffer2"s);

#ifdef ENABLE_SDL
   if (m_stereo3D == STEREO_VR) {
      //AMD Debugging
      colorFormat renderBufferFormatVR;
      const int textureModeVR = LoadValueIntWithDefault(regKey[RegName::Player], "textureModeVR"s, 1);
      switch (textureModeVR) {
      case 0:
         renderBufferFormatVR = RGB8;
         break;
      case 2:
         renderBufferFormatVR = RGB16F;
         break;
      case 3:
         renderBufferFormatVR = RGBA16F;
         break;
      case 1:
      default:
         renderBufferFormatVR = RGBA8;
         break;
      }
      m_pOffscreenVRLeft = new RenderTarget(this, "VRLeft"s, m_width / 2, m_height, renderBufferFormatVR, false, 1, STEREO_OFF, "Fatal Error: unable to create left eye buffer!");
      m_pOffscreenVRRight = new RenderTarget(this, "VRRight"s, m_width / 2, m_height, renderBufferFormatVR, false, 1, STEREO_OFF, "Fatal Error: unable to create right eye buffer!");
   }
#endif

   // Buffers for post-processing (postprocess is done at scene resolution, on a LDR render target without MSAA or full scene supersampling)
   if (video10bit && (m_FXAA == Quality_SMAA || m_FXAA == Standard_DLAA))
      ShowError("SMAA or DLAA post-processing AA should not be combined with 10bit-output rendering (will result in visible artifacts)!");

#ifndef ENABLE_SDL
   // create default vertex declarations for shaders
   CHECKD3D(m_pD3DDevice->CreateVertexDeclaration(VertexTexelElement, &m_pVertexTexelDeclaration));
   CHECKD3D(m_pD3DDevice->CreateVertexDeclaration(VertexNormalTexelElement, &m_pVertexNormalTexelDeclaration));
#endif

   // Vertex buffers
   static constexpr float verts[4 * 5] =
   {
       1.0f,  1.0f, 0.0f, 1.0f, 0.0f,
      -1.0f,  1.0f, 0.0f, 0.0f, 0.0f,
       1.0f, -1.0f, 0.0f, 1.0f, 1.0f,
      -1.0f, -1.0f, 0.0f, 0.0f, 1.0f
   };
   delete m_quadMeshBuffer;
   VertexBuffer* quadVertexBuffer = new VertexBuffer(this, 4, verts, false, VertexFormat::VF_POS_TEX);
   m_quadMeshBuffer = new MeshBuffer(L"Fullscreen Quad"s, quadVertexBuffer);

#ifdef ENABLE_SDL
   delete m_quadPNTDynMeshBuffer;
   VertexBuffer* quadPNTDynVertexBuffer = new VertexBuffer(this, 4, nullptr, true, VertexFormat::VF_POS_NORMAL_TEX);
   m_quadPNTDynMeshBuffer = new MeshBuffer(quadPNTDynVertexBuffer);

   delete m_quadPTDynMeshBuffer;
   VertexBuffer* quadPTDynVertexBuffer = new VertexBuffer(this, 4, nullptr, true, VertexFormat::VF_POS_TEX);
   m_quadPTDynMeshBuffer = new MeshBuffer(quadPTDynVertexBuffer);
#endif

   // Always load the (small) SMAA textures since SMAA can be toggled at runtime through the live UI
   UploadAndSetSMAATextures();

   // Force applying a defined initial render state
   m_current_renderstate.m_state = (~m_renderstate.m_state) & ((1 << 21) - 1);
   m_current_renderstate.m_depthBias = m_renderstate.m_depthBias - 1.0f;
   ApplyRenderStates();
}

bool RenderDevice::LoadShaders()
{
   basicShader = new Shader(this);
   DMDShader = new Shader(this);
   FBShader = new Shader(this);
   flasherShader = new Shader(this);
   lightShader = new Shader(this);

   bool shaderCompilationOkay = true;
#ifdef ENABLE_SDL // OpenGL
   char glShaderPath[MAX_PATH];
   /*DWORD length =*/ GetModuleFileName(nullptr, glShaderPath, MAX_PATH);
   Shader::Defines = ""s;
   if (m_stereo3D == STEREO_OFF)
   {
      Shader::Defines.append("#define N_EYES 1\n"s);
      Shader::Defines.append("#define ENABLE_VR 0\n"s);
      Shader::Defines.append("#define VERTICAL_STEREO 0\n"s);
   }
   else if (m_stereo3D == STEREO_VR)
   {
      Shader::Defines.append("#define N_EYES 2\n"s);
      Shader::Defines.append("#define ENABLE_VR 1\n"s);
      Shader::Defines.append("#define VERTICAL_STEREO 0\n"s);
   }
   else
   {
      Shader::Defines.append("#define N_EYES 2\n"s);
      Shader::Defines.append("#define ENABLE_VR 0\n"s);
      if (m_stereo3D == STEREO_TB || m_stereo3D == STEREO_INT || m_stereo3D == STEREO_FLIPPED_INT)
         Shader::Defines.append("#define VERTICAL_STEREO 1\n"s);
      else
         Shader::Defines.append("#define VERTICAL_STEREO 0\n"s);
   }
   Shader::shaderPath = string(glShaderPath);
   Shader::shaderPath = Shader::shaderPath.substr(0, Shader::shaderPath.find_last_of("\\/"));
   Shader::shaderPath.append(PATH_SEPARATOR_CHAR + "shader"s + PATH_SEPARATOR_CHAR);
   shaderCompilationOkay = basicShader->Load("BasicShader.glfx"s, nullptr, 0) && shaderCompilationOkay;
   shaderCompilationOkay = DMDShader->Load(m_stereo3D == STEREO_VR ? "DMDShaderVR.glfx"s : "DMDShader.glfx"s, nullptr, 0) && shaderCompilationOkay;
   shaderCompilationOkay = FBShader->Load("FBShader.glfx"s, nullptr, 0) && shaderCompilationOkay;
   shaderCompilationOkay = FBShader->Load("SMAA.glfx"s, nullptr, 0) && shaderCompilationOkay;
   shaderCompilationOkay = flasherShader->Load("FlasherShader.glfx"s, nullptr, 0) && shaderCompilationOkay;
   shaderCompilationOkay = lightShader->Load("LightShader.glfx"s, nullptr, 0) && shaderCompilationOkay;
   if (m_stereo3D != STEREO_OFF) {
      StereoShader = new Shader(this);
      shaderCompilationOkay = StereoShader->Load("StereoShader.glfx"s, nullptr, 0) && shaderCompilationOkay;
   }
#else // DirectX 9
   shaderCompilationOkay = basicShader->Load("BasicShader.hlsl"s, g_basicShaderCode, sizeof(g_basicShaderCode)) && shaderCompilationOkay;
   shaderCompilationOkay = DMDShader->Load("DMDShader.hlsl"s, g_dmdShaderCode, sizeof(g_dmdShaderCode)) && shaderCompilationOkay;
   shaderCompilationOkay = FBShader->Load("FBShader.hlsl"s, g_FBShaderCode, sizeof(g_FBShaderCode)) && shaderCompilationOkay;
   shaderCompilationOkay = flasherShader->Load("FlasherShader.hlsl"s, g_flasherShaderCode, sizeof(g_flasherShaderCode)) && shaderCompilationOkay;
   shaderCompilationOkay = lightShader->Load("LightShader.hlsl"s, g_lightShaderCode, sizeof(g_lightShaderCode)) && shaderCompilationOkay;
#endif

   if (!shaderCompilationOkay)
   {
      ReportError("Fatal Error: shader compilation failed!", -1, __FILE__, __LINE__);
      return false;
   }

   // Initialize uniform to default value
   basicShader->SetVector(SHADER_w_h_height, (float)(1.0 / (double)GetMSAABackBufferTexture()->GetWidth()), (float)(1.0 / (double)GetMSAABackBufferTexture()->GetHeight()), 0.0f, 0.0f);
   basicShader->SetFlasherColorAlpha(vec4(1.0f, 1.0f, 1.0f, 1.0f)); // No tinting
   DMDShader->SetFloat(SHADER_alphaTestValue, 1.0f); // No alpha clipping
   FBShader->SetTexture(SHADER_areaTex, m_SMAAareaTexture);
   FBShader->SetTexture(SHADER_searchTex, m_SMAAsearchTexture);

   return true;
}

RenderTarget* RenderDevice::GetPostProcessRenderTarget1()
{
   if (m_pPostProcessRenderTarget1 == nullptr)
   {
// Buffers for post-processing (postprocess is done at scene resolution, on a LDR render target without MSAA nor full scene supersampling)
#ifdef ENABLE_SDL
      colorFormat pp_format = GetBackBufferTexture()->GetColorFormat() == RGBA10 ? colorFormat::RGB10 : colorFormat::RGB8;
#else
      colorFormat pp_format = GetBackBufferTexture()->GetColorFormat() == RGBA10 ? colorFormat::RGBA10 : colorFormat::RGBA8;
#endif
      m_pPostProcessRenderTarget1 = new RenderTarget(this, "PostProcess1"s, m_width, m_height, pp_format, false, 1, STEREO_OFF, "Fatal Error: unable to create stereo3D/post-processing AA/sharpen buffer!");
   }
   return m_pPostProcessRenderTarget1;
}
   
RenderTarget* RenderDevice::GetPostProcessRenderTarget2()
{
   if (m_pPostProcessRenderTarget2 == nullptr)
      m_pPostProcessRenderTarget2 = GetPostProcessRenderTarget1()->Duplicate("PostProcess2"s);
   return m_pPostProcessRenderTarget2;
}

RenderTarget* RenderDevice::GetPostProcessRenderTarget(RenderTarget* renderedRT)
{
   RenderTarget* pp1 = GetPostProcessRenderTarget1();
   if (renderedRT == pp1)
      return GetPostProcessRenderTarget2();
   else
      return pp1;
}

RenderTarget* RenderDevice::GetAORenderTarget(int idx)
{
   // Lazily creates AO render target since this can be enabled during play from script
   if (m_pAORenderTarget1 == nullptr)
   {
      m_pAORenderTarget1 = new RenderTarget(this, "AO1"s, m_width, m_height, colorFormat::GREY8, false, 1, STEREO_OFF,
         "Unable to create AO buffers!\r\nPlease disable Ambient Occlusion.\r\nOr try to (un)set \"Alternative Depth Buffer processing\" in the video options!");
      m_pAORenderTarget2 = m_pAORenderTarget1->Duplicate("AO2"s);

   }
   return idx == 0 ? m_pAORenderTarget1 : m_pAORenderTarget2;
}

void RenderDevice::SwapAORenderTargets() 
{
   RenderTarget* tmpAO = m_pAORenderTarget1;
   m_pAORenderTarget1 = m_pAORenderTarget2;
   m_pAORenderTarget2 = tmpAO;
}

void RenderDevice::ResolveMSAA()
{ 
   if (m_pOffscreenMSAABackBufferTexture != m_pOffscreenBackBufferTexture)
      BlitRenderTarget(m_pOffscreenMSAABackBufferTexture, m_pOffscreenBackBufferTexture, true, true);
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
#ifdef ENABLE_SDL
static void CheckForGLLeak()
{
   //TODO
}
#else
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
#endif


void RenderDevice::FreeShader()
{
   if (basicShader)
   {
      basicShader->SetTextureNull(SHADER_tex_base_color);
      basicShader->SetTextureNull(SHADER_tex_base_transmission);
      basicShader->SetTextureNull(SHADER_tex_base_normalmap);
      basicShader->SetTextureNull(SHADER_tex_env);
      basicShader->SetTextureNull(SHADER_tex_diffuse_env);
      delete basicShader;
      basicShader = nullptr;
   }
   if (DMDShader)
   {
      DMDShader->SetTextureNull(SHADER_tex_dmd);
      DMDShader->SetTextureNull(SHADER_tex_sprite);
      delete DMDShader;
      DMDShader = nullptr;
   }
   if (FBShader)
   {
      FBShader->SetTextureNull(SHADER_tex_fb_filtered);
      FBShader->SetTextureNull(SHADER_tex_fb_unfiltered);
      FBShader->SetTextureNull(SHADER_tex_bloom);
      FBShader->SetTextureNull(SHADER_tex_ao);
      FBShader->SetTextureNull(SHADER_tex_depth);
      FBShader->SetTextureNull(SHADER_tex_color_lut);
      FBShader->SetTextureNull(SHADER_tex_ao_dither);
      FBShader->SetTextureNull(SHADER_areaTex);
      FBShader->SetTextureNull(SHADER_searchTex);
      delete FBShader;
      FBShader = nullptr;
   }
   #ifdef ENABLE_SDL
   if (StereoShader)
   {
      StereoShader->SetTextureNull(SHADER_tex_stereo_fb);
      delete StereoShader;
      StereoShader = nullptr;
   }
   #endif
   if (flasherShader)
   {
      flasherShader->SetTextureNull(SHADER_tex_flasher_A);
      flasherShader->SetTextureNull(SHADER_tex_flasher_B);
      delete flasherShader;
      flasherShader = nullptr;
   }
   if (lightShader)
   {
      delete lightShader;
      lightShader = nullptr;
   }
}

RenderDevice::~RenderDevice()
{
   delete m_quadMeshBuffer;
   delete m_quadPTDynMeshBuffer;
   delete m_quadPNTDynMeshBuffer;

#ifndef ENABLE_SDL
   m_pD3DDevice->SetStreamSource(0, nullptr, 0, 0);
   m_pD3DDevice->SetIndices(nullptr);
   m_pD3DDevice->SetVertexShader(nullptr);
   m_pD3DDevice->SetPixelShader(nullptr);
   m_pD3DDevice->SetFVF(D3DFVF_XYZ);
   m_pD3DDevice->SetDepthStencilSurface(nullptr);
   SAFE_RELEASE(m_pVertexTexelDeclaration);
   SAFE_RELEASE(m_pVertexNormalTexelDeclaration);
#endif

   FreeShader();

   m_texMan.UnloadAll();
   delete m_pOffscreenBackBufferTexture;
   delete m_pPostProcessRenderTarget1;
   delete m_pPostProcessRenderTarget2;
   delete m_pReflectionBufferTexture;

   delete m_pBloomBufferTexture;
   delete m_pBloomTmpBufferTexture;
   delete m_pBackBuffer;
   delete m_pOffscreenVRLeft;
   delete m_pOffscreenVRRight;

   delete m_pAORenderTarget1;
   delete m_pAORenderTarget2;

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
#ifndef DISABLE_FORCE_NVIDIA_OPTIMUS
   if (NVAPIinit) //!! meh
      CHECKNVAPI(NvAPI_Unload());
   NVAPIinit = false;
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
   for (auto binding : m_samplerBindings)
      delete binding;
   m_samplerBindings.clear();
#ifdef ENABLE_VR
   if (m_pHMD)
   {
      turnVROff();
      SaveValueFloat(regKey[RegName::Player], "VRSlope"s, m_slope);
      SaveValueFloat(regKey[RegName::Player], "VROrientation"s, m_orientation);
      SaveValueFloat(regKey[RegName::Player], "VRTableX"s, m_tablex);
      SaveValueFloat(regKey[RegName::Player], "VRTableY"s, m_tabley);
      SaveValueFloat(regKey[RegName::Player], "VRTableZ"s, m_tablez);
   }
#endif

   for (size_t i = 0; i < sizeof(m_samplerStateCache)/sizeof(m_samplerStateCache[0]); i++)
   {
      if (m_samplerStateCache[i] != 0)
      {
         glDeleteSamplers(1, &m_samplerStateCache[i]);
         m_samplerStateCache[i] = 0;
      }
   }

   SDL_GL_DeleteContext(m_sdl_context);
   SDL_DestroyWindow(m_sdl_playfieldHwnd);
#endif
}

void RenderDevice::BeginScene()
{
#ifndef ENABLE_SDL
   //CHECKD3D(m_pD3DDevice->BeginScene());
#endif
}

void RenderDevice::EndScene()
{
#ifndef ENABLE_SDL
   //CHECKD3D(m_pD3DDevice->EndScene());
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
   FlushRenderFrame();

#ifdef ENABLE_SDL
   SDL_GL_SwapWindow(m_sdl_playfieldHwnd);
#ifdef ENABLE_VR
   //glFlush();
   //glFinish();
#endif
   //!! misses vsync handling (e.g. manually trigger vsync)
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

void RenderDevice::UploadAndSetSMAATextures()
{
   BaseTexture* searchBaseTex = new BaseTexture(SEARCHTEX_WIDTH, SEARCHTEX_HEIGHT, BaseTexture::BW);
   memcpy(searchBaseTex->data(), searchTexBytes, SEARCHTEX_SIZE);
   m_SMAAsearchTexture = new Sampler(this, searchBaseTex, true, SamplerAddressMode::SA_CLAMP, SamplerAddressMode::SA_CLAMP, SamplerFilter::SF_NONE);
   delete searchBaseTex;

   // FIXME use standard BaseTexture / Sampler code instead
#ifdef ENABLE_SDL
   // Update bind cache
   auto tex_unit = m_samplerBindings.back();
   if (tex_unit->sampler != nullptr)
      tex_unit->sampler->m_bindings.erase(tex_unit);
   tex_unit->sampler = nullptr;
   glActiveTexture(GL_TEXTURE0 + tex_unit->unit);

   GLuint glTexture;
   glGenTextures(1, &glTexture);
   glBindTexture(GL_TEXTURE_2D, glTexture);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_RED);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_RED);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_GREEN);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   int num_mips = (int)std::log2(float(max(AREATEX_WIDTH, AREATEX_HEIGHT))) + 1;
#ifndef __OPENGLES__
   if (m_GLversion >= 403)
#endif
      glTexStorage2D(GL_TEXTURE_2D, num_mips, RGB8, AREATEX_WIDTH, AREATEX_HEIGHT);
#ifndef __OPENGLES__
   else
   { // should never be triggered nowadays
      GLsizei w = AREATEX_WIDTH;
      GLsizei h = AREATEX_HEIGHT;
      for (int i = 0; i < num_mips; i++)
      {
         glTexImage2D(GL_TEXTURE_2D, i, RGB8, w, h, 0, GL_RG, GL_UNSIGNED_BYTE, nullptr);
         w = max(1, (w / 2));
         h = max(1, (h / 2));
      }
   }
#endif
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
   glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, AREATEX_WIDTH, AREATEX_HEIGHT, GL_RG, GL_UNSIGNED_BYTE, (void*)areaTexBytes);
   glGenerateMipmap(GL_TEXTURE_2D); // Generate mip-maps, when using TexStorage will generate same amount as specified in TexStorage, otherwise good idea to limit by GL_TEXTURE_MAX_LEVEL
   m_SMAAareaTexture = new Sampler(this, glTexture, true, true, SamplerAddressMode::SA_CLAMP, SamplerAddressMode::SA_CLAMP, SamplerFilter::SF_BILINEAR);
#else
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
#endif
}

void RenderDevice::SetSamplerState(int unit, SamplerFilter filter, SamplerAddressMode clamp_u, SamplerAddressMode clamp_v)
{
#ifdef ENABLE_SDL
   assert(sizeof(m_samplerStateCache)/sizeof(m_samplerStateCache[0]) == 3*3*5);
   int samplerStateId = min((int)clamp_u, 2) * 5 * 3
                      + min((int)clamp_v, 2) * 5
                      + min((int)filter, 4);
   GLuint sampler_state = m_samplerStateCache[samplerStateId];
   if (sampler_state == 0)
   {
      m_curStateChanges += 5;
      glGenSamplers(1, &sampler_state);
      m_samplerStateCache[samplerStateId] = sampler_state;
      constexpr int glAddress[] = { GL_REPEAT, GL_CLAMP_TO_EDGE, GL_MIRRORED_REPEAT, GL_REPEAT };
      glSamplerParameteri(sampler_state, GL_TEXTURE_WRAP_S, glAddress[clamp_u]);
      glSamplerParameteri(sampler_state, GL_TEXTURE_WRAP_T, glAddress[clamp_v]);
      switch (filter)
      {
      default: assert(!"unknown filter");
      case SF_NONE: // No mipmapping
         glSamplerParameteri(sampler_state, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
         glSamplerParameteri(sampler_state, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
         glSamplerParameterf(sampler_state, GL_TEXTURE_MAX_ANISOTROPY, 1.0f);
         break;
      case SF_POINT: // Point sampled (aka nearest mipmap) texture filtering.
         glSamplerParameteri(sampler_state, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
         glSamplerParameteri(sampler_state, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
         glSamplerParameterf(sampler_state, GL_TEXTURE_MAX_ANISOTROPY, 1.0f);
         break;
      case SF_BILINEAR: // Bilinar texture filtering.
         glSamplerParameteri(sampler_state, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
         glSamplerParameteri(sampler_state, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
         glSamplerParameterf(sampler_state, GL_TEXTURE_MAX_ANISOTROPY, 1.0f);
         break;
      case SF_TRILINEAR: // Trilinar texture filtering.
         glSamplerParameteri(sampler_state, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
         glSamplerParameteri(sampler_state, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
         glSamplerParameterf(sampler_state, GL_TEXTURE_MAX_ANISOTROPY, 1.0f);
         break;
      case SF_ANISOTROPIC: // Anisotropic texture filtering.
         glSamplerParameteri(sampler_state, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
         glSamplerParameteri(sampler_state, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
         glSamplerParameterf(sampler_state, GL_TEXTURE_MAX_ANISOTROPY, m_maxaniso);
         break;
      }
   }
   glBindSampler(unit, sampler_state);
   m_curStateChanges++;
#else
   if (filter != m_bound_filter[unit])
   {
      switch (filter)
      {
      default:
      case SF_NONE:
         // Don't filter textures, no mipmapping.
         CHECKD3D(m_pD3DDevice->SetSamplerState(unit, D3DSAMP_MAGFILTER, D3DTEXF_POINT));
         CHECKD3D(m_pD3DDevice->SetSamplerState(unit, D3DSAMP_MINFILTER, D3DTEXF_POINT));
         CHECKD3D(m_pD3DDevice->SetSamplerState(unit, D3DSAMP_MIPFILTER, D3DTEXF_NONE));
         m_curStateChanges+=3;
         break;

      case SF_POINT:
         // Point sampled (aka nearest mipmap) texture filtering.
         CHECKD3D(m_pD3DDevice->SetSamplerState(unit, D3DSAMP_MAGFILTER, D3DTEXF_POINT));
         CHECKD3D(m_pD3DDevice->SetSamplerState(unit, D3DSAMP_MINFILTER, D3DTEXF_POINT));
         CHECKD3D(m_pD3DDevice->SetSamplerState(unit, D3DSAMP_MIPFILTER, D3DTEXF_POINT));
         m_curStateChanges += 3;
         break;

      case SF_BILINEAR:
         // Interpolate in 2x2 texels, no mipmapping.
         CHECKD3D(m_pD3DDevice->SetSamplerState(unit, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR));
         CHECKD3D(m_pD3DDevice->SetSamplerState(unit, D3DSAMP_MINFILTER, D3DTEXF_LINEAR));
         CHECKD3D(m_pD3DDevice->SetSamplerState(unit, D3DSAMP_MIPFILTER, D3DTEXF_NONE));
         m_curStateChanges += 3;
         break;

      case SF_TRILINEAR:
         // Filter textures on 2 mip levels (interpolate in 2x2 texels). And filter between the 2 mip levels.
         CHECKD3D(m_pD3DDevice->SetSamplerState(unit, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR));
         CHECKD3D(m_pD3DDevice->SetSamplerState(unit, D3DSAMP_MINFILTER, D3DTEXF_LINEAR));
         CHECKD3D(m_pD3DDevice->SetSamplerState(unit, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR));
         m_curStateChanges += 3;
         break;

      case SF_ANISOTROPIC:
         // Full HQ anisotropic Filter. Should lead to driver doing whatever it thinks is best.
         CHECKD3D(m_pD3DDevice->SetSamplerState(unit, D3DSAMP_MAGFILTER, m_mag_aniso ? D3DTEXF_ANISOTROPIC : D3DTEXF_LINEAR));
         CHECKD3D(m_pD3DDevice->SetSamplerState(unit, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC));
         CHECKD3D(m_pD3DDevice->SetSamplerState(unit, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR));
         CHECKD3D(m_pD3DDevice->SetSamplerState(unit, D3DSAMP_MAXANISOTROPY, min(m_maxaniso, (DWORD)16)));
         m_curStateChanges += 4;
         break;
      }
      m_bound_filter[unit] = filter;
   }
   if (clamp_u != m_bound_clampu[unit])
   {
      switch (clamp_u)
      {
         case SA_REPEAT: CHECKD3D(m_pD3DDevice->SetSamplerState(unit, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP)); m_curStateChanges++; break;
         case SA_CLAMP: CHECKD3D(m_pD3DDevice->SetSamplerState(unit, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP)); m_curStateChanges++; break;
         case SA_MIRROR: CHECKD3D(m_pD3DDevice->SetSamplerState(unit, D3DSAMP_ADDRESSU, D3DTADDRESS_MIRROR)); m_curStateChanges++; break;
      }
      m_bound_clampu[unit] = clamp_u;
   }
   if (clamp_v != m_bound_clampv[unit])
   {
      switch (clamp_v)
      {
         case SA_REPEAT: CHECKD3D(m_pD3DDevice->SetSamplerState(unit, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP)); m_curStateChanges++; break;
         case SA_CLAMP: CHECKD3D(m_pD3DDevice->SetSamplerState(unit, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP)); m_curStateChanges++; break;
         case SA_MIRROR: CHECKD3D(m_pD3DDevice->SetSamplerState(unit, D3DSAMP_ADDRESSV, D3DTADDRESS_MIRROR)); m_curStateChanges++; break;
      }
      m_bound_clampv[unit] = clamp_v;
   }
#endif
}

void RenderDevice::SetRenderState(const RenderState::RenderStates p1, const RenderState::RenderStateValue p2) 
{
   m_renderstate.SetRenderState(p1, p2);
}

void RenderDevice::SetRenderStateCulling(RenderState::RenderStateValue cull)
{
   m_renderstate.SetRenderStateCulling((RenderState::RenderStateValue)cull);
}

void RenderDevice::SetRenderStateDepthBias(float bias)
{
   m_renderstate.SetRenderStateDepthBias(bias);
}

void RenderDevice::CopyRenderStates(const bool copyTo, RenderState& state)
{
   if (copyTo)
   {
      state.m_state = m_renderstate.m_state;
      state.m_depthBias = m_renderstate.m_depthBias;
   }
   else
   {
      m_renderstate.m_state = state.m_state;
      m_renderstate.m_depthBias = state.m_depthBias;
   }
}

void RenderDevice::ApplyRenderStates()
{
   m_renderstate.Apply(this);
}

void RenderDevice::SetClipPlane(const vec4 &plane)
{
#ifdef ENABLE_SDL
   const int eyes = m_stereo3D == STEREO_OFF ? 1 : 2;
   Matrix3D mT;
   float clip_planes[2][4];
   for (int eye = 0; eye < eyes; ++eye)
   {
      memcpy(mT.m, g_pplayer->m_pin3d.m_proj.m_matrixTotal[eye].m, 64); // = world * view * proj
      mT.Invert();
      mT.Transpose();
      clip_planes[eye][0] = mT._11 * plane.x + mT._21 * plane.y + mT._31 * plane.z + mT._41 * plane.w;
      clip_planes[eye][1] = mT._12 * plane.x + mT._22 * plane.y + mT._32 * plane.z + mT._42 * plane.w;
      clip_planes[eye][2] = mT._13 * plane.x + mT._23 * plane.y + mT._33 * plane.z + mT._43 * plane.w;
      clip_planes[eye][3] = mT._14 * plane.x + mT._24 * plane.y + mT._34 * plane.z + mT._44 * plane.w;
   }
   basicShader->SetFloat4v(SHADER_clip_planes, (vec4 *)clip_planes, eyes);
#else
   Matrix3D mT = g_pplayer->m_pin3d.m_proj.m_matrixTotal[0]; // = world * view * proj
   mT.Invert();
   mT.Transpose();
   const D3DXMATRIX m(mT);
   D3DXPLANE clipSpacePlane;
   const D3DXPLANE dxplane(plane.x, plane.y, plane.z, plane.w);
   D3DXPlaneTransform(&clipSpacePlane, &dxplane, &m);
   GetCoreDevice()->SetClipPlane(0, clipSpacePlane);
#endif
}

void RenderDevice::FlushRenderFrame()
{
   m_renderFrame.Execute(m_logNextFrame);
   m_currentPass = nullptr;
   m_logNextFrame = false;
}

void RenderDevice::SetRenderTarget(const string& name, RenderTarget* rt, bool ignoreStereo)
{
   if (rt == nullptr)
   {
      m_currentPass = nullptr;
   }
   else if (m_currentPass == nullptr || rt != m_currentPass->m_rt)
   {
      m_currentPass = new RenderPass(name, rt, ignoreStereo);
      if (rt->m_lastRenderPass != nullptr)
         m_currentPass->AddPrecursor(rt->m_lastRenderPass);
      m_renderFrame.AddPass(m_currentPass);
      rt->m_lastRenderPass = m_currentPass;
   }
}

void RenderDevice::AddRenderTargetDependency(RenderTarget* rt)
{
   if (m_currentPass != nullptr && rt->m_lastRenderPass != nullptr)
   {
      m_currentPass->AddPrecursor(rt->m_lastRenderPass);
   }
}

void RenderDevice::Clear(const DWORD flags, const D3DCOLOR color, const D3DVALUE z, const DWORD stencil)
{
   ApplyRenderStates();
   RenderCommand* cmd = m_renderFrame.NewCommand();
   cmd->SetClear(flags, color);
   m_currentPass->Submit(cmd);
}

void RenderDevice::BlitRenderTarget(RenderTarget* source, RenderTarget* destination, bool copyColor, bool copyDepth,
   const int x1, const int y1, const int w1, const int h1,
   const int x2, const int y2, const int w2, const int h2)
{
   assert(m_currentPass->m_rt == destination); // We must be on a render pass targeted at the destination for correct render pass sorting
   AddRenderTargetDependency(source);
   RenderCommand* cmd = m_renderFrame.NewCommand();
   cmd->SetCopy(source, destination, copyColor, copyDepth, x1, y1, w1, h1, x2, y2, w2, h2);
   m_currentPass->Submit(cmd);
}

void RenderDevice::DrawTexturedQuad(const Vertex3D_TexelOnly* vertices)
{
   assert(Shader::GetCurrentShader() == FBShader || Shader::GetCurrentShader() == StereoShader); // FrameBuffer/Stereo shader are the only ones using Position/Texture vertex format
   ApplyRenderStates();
   RenderCommand* cmd = m_renderFrame.NewCommand();
   cmd->SetDrawTexturedQuad(vertices);
   m_currentPass->Submit(cmd);
}

void RenderDevice::DrawTexturedQuad(const Vertex3D_NoTex2* vertices)
{
   assert(Shader::GetCurrentShader() != FBShader && Shader::GetCurrentShader() != StereoShader); // FrameBuffer/Stereo shader are the only ones using Position/Texture vertex format
   ApplyRenderStates();
   RenderCommand* cmd = m_renderFrame.NewCommand();
   cmd->SetDrawTexturedQuad(vertices);
   m_currentPass->Submit(cmd);
}

void RenderDevice::DrawFullscreenTexturedQuad() {
   assert(Shader::GetCurrentShader() == FBShader || Shader::GetCurrentShader() == StereoShader); // FrameBuffer/Stereo shader are the only ones using Position/Texture vertex format
   DrawMesh(m_quadMeshBuffer, TRIANGLESTRIP, 0, 4);
}

void RenderDevice::DrawMesh(Shader* shader, const Vertex3Ds& center, const float depthBias, MeshBuffer* mb, const PrimitiveTypes type, const DWORD startIndice, const DWORD indexCount)
{
   shader->Begin(); // TODO do not actually bind it, just copy the shader state (needs shader state cleanup first)
   ApplyRenderStates(); // TODO do not actually apply it, just copy the state
   RenderCommand* cmd = m_renderFrame.NewCommand();
   // float depth = depthBias - center.z; // Legacy sorting order (only along negative z axis, which is reversed for reflections)
   float depth = depthBias + (m_viewVec.x * center.x + m_viewVec.y * center.y + m_viewVec.z * center.z);
   cmd->SetDrawMesh(mb, type, startIndice, indexCount, depth);
   m_currentPass->Submit(cmd);
   shader->End();
}

// FIXME remove when all draw calls will be made with a mesh depth
void RenderDevice::DrawMesh(MeshBuffer* mb, const PrimitiveTypes type, const DWORD startIndice, const DWORD indexCount)
{
   ApplyRenderStates();
   RenderCommand* cmd = m_renderFrame.NewCommand();
   cmd->SetDrawMesh(mb, type, startIndice, indexCount, 0.f);
   m_currentPass->Submit(cmd);
}

void RenderDevice::DrawGaussianBlur(RenderTarget* source, RenderTarget* tmp, RenderTarget* dest, float kernel_size)
{
   ShaderTechniques tech_h, tech_v;
   if (kernel_size < 8)
   {
      tech_h = SHADER_TECHNIQUE_fb_blur_horiz7x7;
      tech_v = SHADER_TECHNIQUE_fb_blur_vert7x7;
   }
   else if (kernel_size < 10)
   {
      tech_h = SHADER_TECHNIQUE_fb_blur_horiz9x9;
      tech_v = SHADER_TECHNIQUE_fb_blur_vert9x9;
   }
   else if (kernel_size < 12)
   {
      tech_h = SHADER_TECHNIQUE_fb_blur_horiz11x11;
      tech_v = SHADER_TECHNIQUE_fb_blur_vert11x11;
   }
   else if (kernel_size < 14)
   {
      tech_h = SHADER_TECHNIQUE_fb_blur_horiz13x13;
      tech_v = SHADER_TECHNIQUE_fb_blur_vert13x13;
   }
   else if (kernel_size < 17)
   {
      tech_h = SHADER_TECHNIQUE_fb_blur_horiz15x15;
      tech_v = SHADER_TECHNIQUE_fb_blur_vert15x15;
   }
   else if (kernel_size < 21)
   {
      tech_h = SHADER_TECHNIQUE_fb_blur_horiz19x19;
      tech_v = SHADER_TECHNIQUE_fb_blur_vert19x19;
   }
   else if (kernel_size < 25)
   {
      tech_h = SHADER_TECHNIQUE_fb_blur_horiz23x23;
      tech_v = SHADER_TECHNIQUE_fb_blur_vert23x23;
   }
   else if (kernel_size < 31)
   {
      tech_h = SHADER_TECHNIQUE_fb_blur_horiz27x27;
      tech_v = SHADER_TECHNIQUE_fb_blur_vert27x27;
   }
   else
   {
      tech_h = SHADER_TECHNIQUE_fb_blur_horiz39x39;
      tech_v = SHADER_TECHNIQUE_fb_blur_vert39x39;
   }

   RenderTarget* initial_rt = GetCurrentRenderTarget();
   RenderState initial_state;
   CopyRenderStates(true, initial_state);
   SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_FALSE);
   SetRenderStateCulling(RenderState::CULL_NONE);
   SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
   SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);
   {
      FBShader->SetTextureNull(SHADER_tex_fb_filtered);
      SetRenderTarget("Horizontal Blur"s, tmp, true); // switch to temporary output buffer for horizontal phase of gaussian blur
      AddRenderTargetDependency(source);
      FBShader->SetTexture(SHADER_tex_fb_filtered, source->GetColorSampler());
      FBShader->SetVector(SHADER_w_h_height, (float)(1.0 / source->GetWidth()), (float)(1.0 / source->GetHeight()), 1.0f, 1.0f);
      FBShader->SetTechnique(tech_h);
      FBShader->Begin();
      DrawFullscreenTexturedQuad();
      FBShader->End();
   }
   {
      FBShader->SetTextureNull(SHADER_tex_fb_filtered);
      SetRenderTarget("Vertical Blur"s, dest, true); // switch to output buffer for vertical phase of gaussian blur
      AddRenderTargetDependency(tmp);
      FBShader->SetTexture(SHADER_tex_fb_filtered, tmp->GetColorSampler());
      FBShader->SetVector(SHADER_w_h_height, (float)(1.0 / tmp->GetWidth()), (float)(1.0 / tmp->GetHeight()), 1.0f, 1.0f);
      FBShader->SetTechnique(tech_v);
      FBShader->Begin();
      DrawFullscreenTexturedQuad();
      FBShader->End();
   }
   CopyRenderStates(false, initial_state);
   SetRenderTarget(""s, initial_rt);
}

void RenderDevice::SetTransform(const TransformStateType p1, const Matrix3D * p2, const int count)
{
   switch (p1)
   {
   case TRANSFORMSTATE_WORLD:
      m_matWorld = *p2;
      break;
   case TRANSFORMSTATE_VIEW:
   {
      m_matView = *p2;
      Matrix3D view(m_matView);
      view.Invert();
      m_viewVec.Set(view._31, view._32, view._33);
      break;
   }
   case TRANSFORMSTATE_PROJECTION:
      for (int i = 0; i < count; ++i)
         m_matProj[i] = p2[i];
      break;
   }
}

void RenderDevice::GetTransform(const TransformStateType p1, Matrix3D* p2, const int count)
{
   switch (p1)
   {
   case TRANSFORMSTATE_WORLD: *p2 = m_matWorld; break;
   case TRANSFORMSTATE_VIEW: *p2 = m_matView; break;
   case TRANSFORMSTATE_PROJECTION:
      for (int i = 0; i < count; ++i)
         p2[i] = m_matProj[i];
      break;
   }
}

void RenderDevice::SetMainTextureDefaultFiltering(const SamplerFilter filter)
{
   Shader::SetDefaultSamplerFilter(SHADER_tex_sprite, filter);
   Shader::SetDefaultSamplerFilter(SHADER_tex_flasher_A, filter);
   Shader::SetDefaultSamplerFilter(SHADER_tex_flasher_B, filter);
   Shader::SetDefaultSamplerFilter(SHADER_tex_base_color, filter);
   Shader::SetDefaultSamplerFilter(SHADER_tex_base_normalmap, filter);
}

#ifdef ENABLE_SDL
static ViewPort viewPort;
#endif

void RenderDevice::SetViewport(const ViewPort* p1)
{
#ifdef ENABLE_SDL
   memcpy(&viewPort, p1, sizeof(ViewPort));
#else
   CHECKD3D(m_pD3DDevice->SetViewport((D3DVIEWPORT9*)p1));
#endif
}

void RenderDevice::GetViewport(ViewPort* p1)
{
#ifdef ENABLE_SDL
   memcpy(p1, &viewPort, sizeof(ViewPort));
#else
   CHECKD3D(m_pD3DDevice->GetViewport((D3DVIEWPORT9*)p1));
#endif
}

//////////////////////////////////////////////////////////////////
// VR device implementation

#ifdef ENABLE_VR
bool RenderDevice::isVRinstalled()
{
#ifdef VR_PREVIEW_TEST
   return true;
#else
   return vr::VR_IsRuntimeInstalled();
#endif
}

vr::IVRSystem* RenderDevice::m_pHMD = nullptr;

bool RenderDevice::isVRturnedOn()
{
#ifdef VR_PREVIEW_TEST
   return true;
#else
   if (vr::VR_IsHmdPresent())
   {
      vr::EVRInitError VRError = vr::VRInitError_None;
      if (!m_pHMD)
         m_pHMD = vr::VR_Init(&VRError, vr::VRApplication_Background);
      if (VRError == vr::VRInitError_None && vr::VRCompositor()) {
         for (uint32_t device = 0; device < vr::k_unMaxTrackedDeviceCount; device++) {
            if ((m_pHMD->GetTrackedDeviceClass(device) == vr::TrackedDeviceClass_HMD)) {
               vr::VR_Shutdown();
               m_pHMD = nullptr;
               return true;
            }
         }
      } else
         m_pHMD = nullptr;
   }
#endif
   return false;
}

void RenderDevice::turnVROff()
{
   if (m_pHMD)
   {
      vr::VR_Shutdown();
      m_pHMD = nullptr;
   }
}

void RenderDevice::InitVR() {
#ifdef VR_PREVIEW_TEST
   m_pHMD = nullptr;
#else
   vr::EVRInitError VRError = vr::VRInitError_None;
   if (!m_pHMD) {
      m_pHMD = vr::VR_Init(&VRError, vr::VRApplication_Scene);
      if (VRError != vr::VRInitError_None) {
         m_pHMD = nullptr;
         char buf[1024];
         sprintf_s(buf, sizeof(buf), "Unable to init VR runtime: %s", vr::VR_GetVRInitErrorAsEnglishDescription(VRError));
         ShowError(buf);
      }
      else if (!vr::VRCompositor())
      /*if (VRError != vr::VRInitError_None)*/ {
         m_pHMD = nullptr;
         char buf[1024];
         sprintf_s(buf, sizeof(buf), "Unable to init VR compositor");// :% s", vr::VR_GetVRInitErrorAsEnglishDescription(VRError));
         ShowError(buf);
      }
   }
#endif

   const float nearPlane = LoadValueFloatWithDefault(regKey[RegName::PlayerVR], "nearPlane"s, 5.0f) / 100.0f;
   const float farPlane = 5000.0f; //LoadValueFloatWithDefault(regKey[RegName::PlayerVR], "farPlane"s, 5000.0f) / 100.0f;

   vr::HmdMatrix34_t left_eye_pos, right_eye_pos;
   vr::HmdMatrix44_t left_eye_proj, right_eye_proj;
   if (m_pHMD == nullptr)
   {
      // Default debug output
      uint32_t eye_width = 1080, eye_height = 1200; // Oculus Rift resolution
      m_width = eye_width * 2;
      m_height = eye_height;
      Matrix3D left, right;
      left.SetIdentity(); // TODO find sensible value and use them instead of the desktop projection
      right.SetIdentity();
      for (int i = 0; i < 3; i++)
         for (int j = 0; j < 4; j++)
         {
            left_eye_pos.m[i][j] = left.m[j][i];
            right_eye_pos.m[i][j] = right.m[j][i];
         }
      for (int i = 0; i < 4; i++)
         for (int j = 0; j < 4; j++)
         {
            left_eye_proj.m[i][j] = left.m[j][i];
            right_eye_proj.m[i][j] = right.m[j][i];
         }
   }
   else
   {
      uint32_t eye_width, eye_height;
      m_pHMD->GetRecommendedRenderTargetSize(&eye_width, &eye_height);
      m_width = eye_width * 2;
      m_height = eye_height;
      left_eye_pos = m_pHMD->GetEyeToHeadTransform(vr::Eye_Left);
      right_eye_pos = m_pHMD->GetEyeToHeadTransform(vr::Eye_Right);
      left_eye_proj = m_pHMD->GetProjectionMatrix(vr::Eye_Left, nearPlane, farPlane); //5cm to 50m should be a reasonable range
      right_eye_proj = m_pHMD->GetProjectionMatrix(vr::Eye_Right, nearPlane, farPlane); //5cm to 500m should be a reasonable range
   }

   //Calculate left EyeProjection Matrix relative to HMD position
   Matrix3D matEye2Head;
   for (int i = 0;i < 3;i++)
      for (int j = 0;j < 4;j++)
         matEye2Head.m[j][i] = left_eye_pos.m[i][j];
   for (int j = 0;j < 4;j++)
      matEye2Head.m[j][3] = (j == 3) ? 1.0f : 0.0f;

   matEye2Head.Invert();

   left_eye_proj.m[2][2] = -1.0f;
   left_eye_proj.m[2][3] = -nearPlane;
   Matrix3D matProjection;
   for (int i = 0;i < 4;i++)
      for (int j = 0;j < 4;j++)
         matProjection.m[j][i] = left_eye_proj.m[i][j];

   m_vrMatProj[0] = matEye2Head * matProjection;

   //Calculate right EyeProjection Matrix relative to HMD position
   for (int i = 0;i < 3;i++)
      for (int j = 0;j < 4;j++)
         matEye2Head.m[j][i] = right_eye_pos.m[i][j];
   for (int j = 0;j < 4;j++)
      matEye2Head.m[j][3] = (j == 3) ? 1.0f : 0.0f;

   matEye2Head.Invert();

   right_eye_proj.m[2][2] = -1.0f;
   right_eye_proj.m[2][3] = -nearPlane;
   for (int i = 0;i < 4;i++)
      for (int j = 0;j < 4;j++)
         matProjection.m[j][i] = right_eye_proj.m[i][j];

   m_vrMatProj[1] = matEye2Head * matProjection;

   if (vr::k_unMaxTrackedDeviceCount > 0) {
      m_rTrackedDevicePose = new vr::TrackedDevicePose_t[vr::k_unMaxTrackedDeviceCount];
   }
   else {
      std::runtime_error noDevicesFound("No Tracking devices found");
      throw(noDevicesFound);
   }

   m_slope = LoadValueFloatWithDefault(regKey[RegName::Player], "VRSlope"s, 6.5f);
   m_orientation = LoadValueFloatWithDefault(regKey[RegName::Player], "VROrientation"s, 0.0f);
   m_tablex = LoadValueFloatWithDefault(regKey[RegName::Player], "VRTableX"s, 0.0f);
   m_tabley = LoadValueFloatWithDefault(regKey[RegName::Player], "VRTableY"s, 0.0f);
   m_tablez = LoadValueFloatWithDefault(regKey[RegName::Player], "VRTableZ"s, 80.0f);

   updateTableMatrix();
}

void RenderDevice::UpdateVRPosition()
{
   if (!m_pHMD)
      return;

   vr::VRCompositor()->WaitGetPoses(m_rTrackedDevicePose, vr::k_unMaxTrackedDeviceCount, nullptr, 0);

   m_vrMatView.SetIdentity();
   for (unsigned int device = 0; device < vr::k_unMaxTrackedDeviceCount; device++)
   {
      if ((m_rTrackedDevicePose[device].bPoseIsValid) && (m_pHMD->GetTrackedDeviceClass(device) == vr::TrackedDeviceClass_HMD))
      {
         hmdPosition = m_rTrackedDevicePose[device];
         for (int i = 0; i < 3; i++)
            for (int j = 0; j < 4; j++)
               m_vrMatView.m[j][i] = hmdPosition.mDeviceToAbsoluteTracking.m[i][j];
         break;
      }
   }
   m_vrMatView.Invert();
   m_vrMatView = m_tableWorld * m_vrMatView;
}

void RenderDevice::tableUp()
{
   m_tablez += 1.0f;
   if (m_tablez > 250.0f)
      m_tablez = 250.0f;
   updateTableMatrix();
}

void RenderDevice::tableDown()
{
   m_tablez -= 1.0f;
   if (m_tablez < 0.0f)
      m_tablez = 0.0f;
   updateTableMatrix();
}

void RenderDevice::recenterTable()
{
   m_orientation = -RADTOANG(atan2(hmdPosition.mDeviceToAbsoluteTracking.m[0][2], hmdPosition.mDeviceToAbsoluteTracking.m[0][0]));
   if (m_orientation < 0.0f)
      m_orientation += 360.0f;
   const float w = 100.f * 0.5f * m_scale * (g_pplayer->m_ptable->m_right - g_pplayer->m_ptable->m_left);
   const float h = 100.f * m_scale * (g_pplayer->m_ptable->m_bottom - g_pplayer->m_ptable->m_top) + 20.0f;
   const float c = cos(ANGTORAD(m_orientation));
   const float s = sin(ANGTORAD(m_orientation));
   m_tablex = 100.0f * hmdPosition.mDeviceToAbsoluteTracking.m[0][3] - c * w + s * h;
   m_tabley = -100.0f * hmdPosition.mDeviceToAbsoluteTracking.m[2][3] + s * w + c * h;
   updateTableMatrix();
}

void RenderDevice::updateTableMatrix()
{
   Matrix3D tmp;
   //Tilt playfield.
   m_tableWorld.SetIdentity();
   m_tableWorld.RotateXMatrix(ANGTORAD(-m_slope));
   //Convert from VPX scale and coords to VR
   tmp.SetIdentity();
   tmp.m[0][0] = -m_scale;  tmp.m[0][1] =    0.0f;  tmp.m[0][2] =     0.0f;
   tmp.m[1][0] =     0.0f;  tmp.m[1][1] =    0.0f;  tmp.m[1][2] = -m_scale;
   tmp.m[2][0] =     0.0f;  tmp.m[2][1] = m_scale;  tmp.m[2][2] =     0.0f;
   m_tableWorld = m_tableWorld * tmp;
   //Rotate table around VR height axis
   tmp.SetIdentity();
   tmp.RotateYMatrix(ANGTORAD(180.f - m_orientation));
   m_tableWorld = m_tableWorld * tmp;
   //Locate front left corner of the table in the room -x is to the right, -y is up and -z is back - all units in meters
   tmp.SetIdentity();
   tmp.SetTranslation(m_tablex / 100.0f, m_tablez / 100.0f, -m_tabley / 100.0f);
   m_tableWorld = m_tableWorld * tmp;
}

void RenderDevice::SetTransformVR()
{
   SetTransform(TRANSFORMSTATE_PROJECTION, m_vrMatProj, m_stereo3D != STEREO_OFF ? 2:1);
   SetTransform(TRANSFORMSTATE_VIEW, &m_vrMatView, 1);
}
#endif
