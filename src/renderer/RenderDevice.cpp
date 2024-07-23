#include "stdafx.h"

#ifndef __STANDALONE__
#include <DxErr.h>
#endif
#include <thread>

// Undefine this if you want to debug VR mode without a VR headset
//#define VR_PREVIEW_TEST

//#include "Dwmapi.h" // use when we get rid of XP at some point, get rid of the manual dll loads in here then

#ifndef DISABLE_FORCE_NVIDIA_OPTIMUS
#include "nvapi/nvapi.h"
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
#ifndef __STANDALONE__
#include "captureExt.h"
#endif

#else // DirectX 9
#include "Material.h"
#include "BasicShader.h"
#include "DMDShader.h"
#include "FBShader.h"
#include "FlasherShader.h"
#include "LightShader.h"
#include "StereoShader.h"
#include "BallShader.h"
#endif


#if defined(ENABLE_SDL) // OpenGL
GLuint RenderDevice::m_samplerStateCache[3 * 3 * 5];

#else // DirectX 9
#if _MSC_VER >= 1900
 #pragma comment(lib, "legacy_stdio_definitions.lib") //dxerr.lib needs this
#endif

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
#ifndef __STANDALONE__
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
#else
   return true;
#endif
}

bool IsWindowsVistaOr7()
{
#ifndef __STANDALONE__
   OSVERSIONINFOEXW osvi = { sizeof(osvi), 0, 0, 0, 0, { 0 }, 0, 0, 0, 0, 0 };
   const DWORDLONG dwlConditionMask = //VerSetConditionMask(
      VerSetConditionMask(VerSetConditionMask(0, VER_MAJORVERSION, VER_EQUAL), VER_MINORVERSION, VER_EQUAL) /*,
      VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL)*/
      ;
   osvi.dwMajorVersion = HIBYTE(_WIN32_WINNT_VISTA);
   osvi.dwMinorVersion = LOBYTE(_WIN32_WINNT_VISTA);
   //osvi.wServicePackMajor = 0;

   const bool vista = VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION /*| VER_SERVICEPACKMAJOR*/, dwlConditionMask) != FALSE;

   OSVERSIONINFOEXW osvi2 = { sizeof(osvi), 0, 0, 0, 0, { 0 }, 0, 0, 0, 0, 0 };
   osvi2.dwMajorVersion = HIBYTE(_WIN32_WINNT_WIN7);
   osvi2.dwMinorVersion = LOBYTE(_WIN32_WINNT_WIN7);
   //osvi2.wServicePackMajor = 0;

   const bool win7 = VerifyVersionInfoW(&osvi2, VER_MAJORVERSION | VER_MINORVERSION /*| VER_SERVICEPACKMAJOR*/, dwlConditionMask) != FALSE;

   return vista || win7;
#else
   return false;
#endif
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
   char msg[2176];
#ifdef ENABLE_SDL
   sprintf_s(msg, sizeof(msg), "GL Fatal Error 0x%08X %s in %s:%d", hr, glErrorToString(hr), file, line);
#else
   sprintf_s(msg, sizeof(msg), "Fatal error %s (0x%x: %s) at %s:%d", DXGetErrorString(hr), hr, DXGetErrorDescription(hr), file, line);
#endif
   ShowError(msg);
   assert(false);
   exit(-1);
}

void ReportError(const char *errorText, const HRESULT hr, const char *file, const int line)
{
   char msg[16384];
#ifdef ENABLE_SDL
   sprintf_s(msg, sizeof(msg), "GL Error 0x%08X %s in %s:%d\n%s", hr, glErrorToString(hr), file, line, errorText);
   ShowError(msg);
#else
   sprintf_s(msg, sizeof(msg), "%s %s (0x%x: %s) at %s:%d", errorText, DXGetErrorString(hr), hr, DXGetErrorDescription(hr), file, line);
   ShowError(msg);
   exit(-1);
#endif
}

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
#if defined(ENABLE_SDL) && defined(_DEBUG) && !defined(__OPENGLES__)
void APIENTRY GLDebugMessageCallback(GLenum source, GLenum type, GLuint id,
                                     GLenum severity, GLsizei length,
                                     const GLchar *msg, const void *data)
{
   char* _source;
   switch (source) {
      case GL_DEBUG_SOURCE_API: _source = (LPSTR) "API"; break;
      case GL_DEBUG_SOURCE_WINDOW_SYSTEM: _source = (LPSTR) "WINDOW SYSTEM"; break;
      case GL_DEBUG_SOURCE_SHADER_COMPILER: _source = (LPSTR) "SHADER COMPILER"; break;
      case GL_DEBUG_SOURCE_THIRD_PARTY: _source = (LPSTR) "THIRD PARTY"; break;
      case GL_DEBUG_SOURCE_APPLICATION: _source = (LPSTR) "APPLICATION"; break;
      case GL_DEBUG_SOURCE_OTHER: _source = (LPSTR) "UNKNOWN"; break;
      default: _source = (LPSTR) "UNHANDLED"; break;
   }
   char* _type;
   switch (type) {
   case GL_DEBUG_TYPE_ERROR: _type = (LPSTR) "ERROR"; break;
   case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: _type = (LPSTR) "DEPRECATED BEHAVIOR"; break;
   case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: _type = (LPSTR) "UNDEFINED BEHAVIOR"; break;
   case GL_DEBUG_TYPE_PORTABILITY: _type = (LPSTR) "PORTABILITY"; break;
   case GL_DEBUG_TYPE_PERFORMANCE: _type = (LPSTR) "PERFORMANCE"; break;
   case GL_DEBUG_TYPE_OTHER: _type = (LPSTR) "OTHER"; break;
   case GL_DEBUG_TYPE_MARKER: _type = (LPSTR) "MARKER"; break;
   case GL_DEBUG_TYPE_PUSH_GROUP: _type = (LPSTR) "GL_DEBUG_TYPE_PUSH_GROUP"; break;
   case GL_DEBUG_TYPE_POP_GROUP: _type = (LPSTR) "GL_DEBUG_TYPE_POP_GROUP"; break;
   default: _type = (LPSTR) "UNHANDLED"; break;
   }
   char* _severity;
   switch (severity) {
      case GL_DEBUG_SEVERITY_HIGH: _severity = (LPSTR) "HIGH"; break;
      case GL_DEBUG_SEVERITY_MEDIUM: _severity = (LPSTR) "MEDIUM"; break;
      case GL_DEBUG_SEVERITY_LOW: _severity = (LPSTR) "LOW"; break;
      case GL_DEBUG_SEVERITY_NOTIFICATION: _severity = (LPSTR) "NOTIFICATION"; break;
      default: _severity = (LPSTR) "UNHANDLED"; break;
   }
   if (severity != GL_DEBUG_SEVERITY_NOTIFICATION)
   {
      // FIXME this will crash if the drivers performs the call on the wrong thread (nvogl does...)
      /* assert(false);
      PLOGE << "OpenGL Error #" << id << ": " << _type << " of " << _severity << "severity, raised from " << _source << ": " << msg;
      fprintf(stderr, "%d: %s of %s severity, raised from %s: %s\n", id, _type, _severity, _source, msg);
      if (type == GL_DEBUG_TYPE_ERROR || type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR || severity == GL_DEBUG_SEVERITY_HIGH)
         ShowError(msg);*/
   }
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

void EnumerateDisplayModes(const int display, vector<VideoMode>& modes)
{
   modes.clear();

   vector<DisplayConfig> displays;
   getDisplayList(displays);
   if (display >= (int)displays.size())
      return;
   const int adapter = displays[display].adapter;

#ifdef ENABLE_SDL
   const int amount = SDL_GetNumDisplayModes(adapter);
   for (int mode = 0; mode < amount; ++mode) {
      SDL_DisplayMode sdlMode;
      SDL_GetDisplayMode(adapter, mode, &sdlMode);
      VideoMode vmode = {};
      vmode.width = sdlMode.w;
      vmode.height = sdlMode.h;
      switch (sdlMode.format) {
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
      vmode.refreshrate = sdlMode.refresh_rate;
      modes.push_back(vmode);
   }
#else
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

#ifndef __STANDALONE__
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
   config.adapter = -1;
   memcpy(config.DeviceName, info.szDevice, CCHDEVICENAME); // Internal display name e.g. "\\\\.\\DISPLAY1"
   data->insert(std::pair<string, DisplayConfig>(config.DeviceName, config));
   return TRUE;
}
#endif

int getDisplayList(vector<DisplayConfig>& displays)
{
   displays.clear();

#if defined(ENABLE_SDL) && defined(_WIN32)
   // Windows and SDL order of display enumeration do not match, therefore the display identifier will not match between DX and OpenGL version
   // SDL2 display identifier do not match the id of the native Windows settings
   // SDL2 does not offer a way to get the adapter (i.e. Graphics Card) associated with a display (i.e. Monitor) so we use the monitor name for both
   // Get the resolution of all enabled displays as they appear in the Windows settings UI.
   std::map<string, DisplayConfig> displayMap;
   EnumDisplayMonitors(nullptr, nullptr, MonitorEnumList, reinterpret_cast<LPARAM>(&displayMap));
   for (int i = 0; i < SDL_GetNumVideoDisplays(); ++i)
   {
      SDL_Rect displayBounds;
      if (SDL_GetDisplayBounds(i, &displayBounds) == 0)
      {
         for (std::map<string, DisplayConfig>::iterator display = displayMap.begin(); display != displayMap.end(); ++display)
         {
            if (display->second.left == displayBounds.x && display->second.top == displayBounds.y && display->second.width == displayBounds.w && display->second.height == displayBounds.h)
            {
               display->second.adapter = i;
               strncpy_s(display->second.GPU_Name, SDL_GetDisplayName(display->second.adapter), MAX_DEVICE_IDENTIFIER_STRING - 1);
            }
         }
      }
   }

   // Apply the same numbering as windows
   int i = 0;
   for (std::map<string, DisplayConfig>::iterator display = displayMap.begin(); display != displayMap.end(); ++display)
   {
      if (display->second.adapter >= 0)
      {
         display->second.display = i;
         displays.push_back(display->second);
      }
      i++;
   }
#elif defined(ENABLE_SDL)
   int i = 0;
   for (; i < SDL_GetNumVideoDisplays(); ++i)
   {
      SDL_Rect displayBounds;
      if (SDL_GetDisplayBounds(i, &displayBounds) == 0) {
         DisplayConfig displayConf;
         displayConf.display = i; // Window Display identifier (the number that appears in the native Windows settings)
         displayConf.adapter = i; // SDL Display identifier. Will be used for creating the display
         displayConf.isPrimary = (displayBounds.x == 0) && (displayBounds.y == 0);
         displayConf.top = displayBounds.y;
         displayConf.left = displayBounds.x;
         displayConf.width = displayBounds.w;
         displayConf.height = displayBounds.h;
         const string devicename = "\\\\.\\DISPLAY"s.append(std::to_string(i));
         strncpy_s(displayConf.DeviceName, devicename.c_str(), CCHDEVICENAME - 1);
         strncpy_s(displayConf.GPU_Name, SDL_GetDisplayName(displayConf.display), MAX_DEVICE_IDENTIFIER_STRING - 1);
         displays.push_back(displayConf);
      }
   }
#else
   // Get the resolution of all enabled displays as they appear in the Windows settings UI.
   std::map<string, DisplayConfig> displayMap;
   EnumDisplayMonitors(nullptr, nullptr, MonitorEnumList, reinterpret_cast<LPARAM>(&displayMap));

   IDirect3D9* pD3D = Direct3DCreate9(D3D_SDK_VERSION);
   if (pD3D == nullptr)
   {
      ShowError("Could not create D3D9 object.");
      return -1;
   }
   // Map the displays to the DX9 adapter. Otherwise this leads to an performance impact on systems with multiple GPUs
   const int adapterCount = pD3D->GetAdapterCount();
   for (int i = 0; i < adapterCount; ++i)
   {
      D3DADAPTER_IDENTIFIER9 adapter;
      pD3D->GetAdapterIdentifier(i, 0, &adapter);
      std::map<string, DisplayConfig>::iterator display = displayMap.find(adapter.DeviceName);
      if (display != displayMap.end())
      {
         display->second.adapter = i;
         strncpy_s(display->second.GPU_Name, adapter.Description, sizeof(display->second.GPU_Name) - 1);
      }
   }
   SAFE_RELEASE(pD3D);
   
   // Apply the same numbering as windows
   int i = 0;
   for (std::map<string, DisplayConfig>::iterator display = displayMap.begin(); display != displayMap.end(); ++display)
   {
      if (display->second.adapter >= 0) {
         display->second.display = i;
         displays.push_back(display->second);
      }
      i++;
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

RenderDeviceState::RenderDeviceState(RenderDevice* rd)
   : m_rd(rd)
   , m_basicShaderState(new Shader::ShaderState(m_rd->basicShader))
   , m_DMDShaderState(new Shader::ShaderState(m_rd->DMDShader))
   , m_FBShaderState(new Shader::ShaderState(m_rd->FBShader))
   , m_flasherShaderState(new Shader::ShaderState(m_rd->flasherShader))
   , m_lightShaderState(new Shader::ShaderState(m_rd->lightShader))
   , m_ballShaderState(new Shader::ShaderState(m_rd->m_ballShader))
   , m_stereoShaderState(new Shader::ShaderState(m_rd->StereoShader))
{
}

RenderDeviceState::~RenderDeviceState()
{
   delete m_basicShaderState;
   delete m_DMDShaderState;
   delete m_FBShaderState;
   delete m_flasherShaderState;
   delete m_lightShaderState;
   delete m_ballShaderState;
   delete m_stereoShaderState;
}

////////////////////////////////////////////////////////////////////

#ifndef ENABLE_SDL
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

RenderDevice::RenderDevice(const HWND hwnd, const int width, const int height, const bool fullscreen, const int colordepth, const float AAfactor, const StereoMode stereo3D, const unsigned int FXAA, const bool sharpen, const bool ss_refl, const bool useNvidiaApi, const bool disable_dwm, const int BWrendering)
    : m_windowHwnd(hwnd), m_width(width), m_height(height), m_fullscreen(fullscreen), 
      m_colorDepth(colordepth), m_AAfactor(AAfactor), m_stereo3D(stereo3D),
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
#endif

#ifndef __STANDALONE__
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
#else
    m_dwm_was_enabled = false;
    m_dwm_enabled = false;
#endif
}

void RenderDevice::CreateDevice(int& refreshrate, VideoSyncMode& syncMode, UINT adapterIndex)
{
   assert(g_pplayer != nullptr); // Player must be created to give access to the output window
   colorFormat back_buffer_format;

#ifdef ENABLE_SDL
   ///////////////////////////////////
   // OpenGL device initialization
   const int displays = SDL_GetNumVideoDisplays();
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
   case SDL_PIXELFORMAT_RGBA8888: back_buffer_format = colorFormat::RGBA8; break;
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
#ifndef __STANDALONE__
   m_windowHwnd = wmInfo.info.win.window;
#endif

   m_sdl_context = SDL_GL_CreateContext(m_sdl_playfieldHwnd);

   SDL_GL_MakeCurrent(m_sdl_playfieldHwnd, m_sdl_context);
   SDL_RaiseWindow(g_pplayer->m_sdl_playfieldHwnd);

#ifndef __OPENGLES__
   if (!gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress))
#else
   if (!gladLoadGLES2((GLADloadfunc)SDL_GL_GetProcAddress))
#endif
   {
      ShowError("Glad failed");
      exit(-1);
   }

#ifdef __STANDALONE__
   unsigned int num_exts_i = 0;
   glad_glGetIntegerv(GL_NUM_EXTENSIONS, (int*) &num_exts_i);
   PLOGD.printf("%d extensions available", num_exts_i);
   for(int index = 0; index < num_exts_i; index++) {
      PLOGD.printf("%s", glad_glGetStringi(GL_EXTENSIONS, index));
   }
#ifdef __OPENGLES__
   int range[2];
   int precision;
   glGetShaderPrecisionFormat(GL_VERTEX_SHADER, GL_HIGH_FLOAT, range, &precision);
   PLOGD.printf("Vertex shader high precision float range: %d %d precision: %d", range[0], range[1], precision);
   glGetShaderPrecisionFormat(GL_FRAGMENT_SHADER, GL_HIGH_FLOAT, range, &precision);
   PLOGD.printf("Fragment shader high precision float range: %d %d precision: %d", range[0], range[1], precision);
#endif
#endif

   int gl_majorVersion = 0;
   int gl_minorVersion = 0;
   glGetIntegerv(GL_MAJOR_VERSION, &gl_majorVersion);
   glGetIntegerv(GL_MINOR_VERSION, &gl_minorVersion);

#ifndef __STANDALONE__
   if (gl_majorVersion < 3 || (gl_majorVersion == 3 && gl_minorVersion < 2))
   {
      char errorMsg[256];
      sprintf_s(errorMsg, sizeof(errorMsg), "Your graphics card only supports OpenGL %d.%d, but VPVR requires OpenGL 3.2 or newer.", gl_majorVersion, gl_minorVersion);
      ShowError(errorMsg);
      exit(-1);
   }
#endif

   m_GLversion = gl_majorVersion * 100 + gl_minorVersion;

   // Enable debugging layer of OpenGL
#ifdef _DEBUG
#ifndef __OPENGLES__
   glEnable(GL_DEBUG_OUTPUT); // on its own is the 'fast' version
   //glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // callback is in sync with errors, so a breakpoint can be placed on the callback in order to get a stacktrace for the GL error
   if (glad_glDebugMessageCallback)
   {
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
   m_scale = 1.0f; // Scale factor from scene (in VP units) to VR view (in meters)
   if (m_stereo3D == STEREO_VR)
   {
      if (g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::PlayerVR, "ScaleToFixedWidth"s, false))
      {
         float width;
         g_pplayer->m_ptable->get_Width(&width);
         m_scale = g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::PlayerVR, "ScaleAbsolute"s, 55.0f) * 0.01f / width;
      }
      else
         m_scale = VPUTOCM(0.01f) * g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::PlayerVR, "ScaleRelative"s, 1.0f);
      if (m_scale < VPUTOCM(0.01f))
         m_scale = VPUTOCM(0.01f); // Scale factor for VPUnits to Meters
      // Initialize VR, this will also override the render buffer size (m_width, m_height) to account for HMD render size and render the 2 eyes simultaneously
      InitVR();
   }
   else
#endif
   if (m_stereo3D == STEREO_SBS)
      // Side by side needs to fit the 2 views along the width, so each view is half the total width
      m_width = m_width / 2;
   else if (m_stereo3D == STEREO_TB)
      // Top/Bottom (and interlaced) needs to fit the 2 views along the height, so each view is half the total height
      m_height = m_height / 2;

   // Flip scheduling: 0 for immediate, 1 for synchronized with the vertical retrace, -1 for adaptive vsync (i.e. synchronized on vsync except for late frame)
   switch (syncMode)
   {
   case VideoSyncMode::VSM_NONE: SDL_GL_SetSwapInterval(0); break;
   case VideoSyncMode::VSM_VSYNC: SDL_GL_SetSwapInterval(1); break;
   case VideoSyncMode::VSM_ADAPTIVE_VSYNC: SDL_GL_SetSwapInterval(-1); break;
   case VideoSyncMode::VSM_FRAME_PACING: SDL_GL_SetSwapInterval(0); break;
   default: break;
   }

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

    m_pD3DEx = nullptr;
    m_pD3DDeviceEx = nullptr;

#ifdef USE_D3D9EX
    mDirect3DCreate9Ex = (pD3DC9Ex)GetProcAddress(GetModuleHandle(TEXT("d3d9.dll")), "Direct3DCreate9Ex"); //!! remove as soon as win xp support dropped and use static link
    if (mDirect3DCreate9Ex)
    {
        const HRESULT hr = mDirect3DCreate9Ex(D3D_SDK_VERSION, &m_pD3DEx);
        if (FAILED(hr) || (m_pD3DEx == nullptr))
        {
            ShowError("Could not create D3D9Ex object.");
            throw 0;
        }
        m_pD3DEx->QueryInterface(__uuidof(IDirect3D9), reinterpret_cast<void**>(&m_pD3D));
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

    bool video10bit = g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "Render10Bit"s, false);

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
        ShowError("Invalid Output format: "s.append(std::to_string(format)));
        exit(-1);
    }
    }

    D3DPRESENT_PARAMETERS params;
    params.BackBufferWidth = m_width;
    params.BackBufferHeight = m_height;
    params.BackBufferFormat = format;
    params.BackBufferCount = 1;
    params.MultiSampleType = D3DMULTISAMPLE_NONE;
    params.MultiSampleQuality = 0;
    params.SwapEffect = D3DSWAPEFFECT_DISCARD;
    params.hDeviceWindow = m_windowHwnd;
    params.Windowed = !m_fullscreen;
    params.EnableAutoDepthStencil = FALSE;
    params.AutoDepthStencilFormat = D3DFMT_UNKNOWN; // ignored
    params.Flags = /*fullscreen ? D3DPRESENTFLAG_LOCKABLE_BACKBUFFER :*/ /*(stereo3D ?*/ 0 /*: D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL)*/
       ; // D3DPRESENTFLAG_LOCKABLE_BACKBUFFER only needed for SetDialogBoxMode() below, but makes rendering slower on some systems :/
    params.FullScreen_RefreshRateInHz = m_fullscreen ? refreshrate : 0;
    params.PresentationInterval = syncMode == VideoSyncMode::VSM_VSYNC ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;

    // This is a horrible hack: DirectX only supports fake stereo made by rendering at half resolution for these mode
    // but on the other hand, for DirectX, we do not implement clean separation between output buffer (which must be at full resolution)
    // so we hack it back here.
    if (g_pplayer->m_stereo3D == STEREO_SBS) // Side by side needs to fit the 2 views along the width, so each view is half the total width
        params.BackBufferWidth *= 2;
    else if (g_pplayer->m_stereo3D == STEREO_TB || m_stereo3D == STEREO_INT || m_stereo3D == STEREO_FLIPPED_INT) // Top/Bottom (and interlaced) needs to fit the 2 views along the height, so each view is half the total height
        params.BackBufferHeight *= 2;

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

   const bool softwareVP = g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "SoftwareVertexProcessing"s, false);
   const DWORD flags = softwareVP ? D3DCREATE_SOFTWARE_VERTEXPROCESSING : D3DCREATE_HARDWARE_VERTEXPROCESSING;

   // Create the D3D device. This optionally goes to the proper fullscreen mode.
   // It also creates the default swap chain (front and back buffer).
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

   // Create default texture
   BaseTexture* surf = new BaseTexture(1, 1, BaseTexture::Format::RGBA);
   memset(surf->data(), 0, 4);
   m_nullTexture = new Sampler(this, surf, false);
   m_nullTexture->SetName("Null"s);
   delete surf;

   // Retrieve a reference to the back buffer.
   int backBufferWidth, backBufferHeight;
#ifdef ENABLE_SDL
   SDL_GL_GetDrawableSize(m_sdl_playfieldHwnd, &backBufferWidth, &backBufferHeight);
   PLOGI.printf("Drawable Size: width=%d, height=%d", backBufferWidth, backBufferHeight);
#ifdef __STANDALONE__
   float backBufferScale = g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::Standalone, "BackBufferScale"s, 1.0f);
   if (backBufferScale != 1.0f) {
      backBufferWidth *= backBufferScale;
      backBufferHeight *= backBufferScale;
      PLOGI.printf("Scaled Drawable Size: width=%d, height=%d", backBufferWidth, backBufferHeight);
   }
   m_width = backBufferWidth;
   m_height = backBufferHeight;
#endif
#else
   backBufferWidth = m_width;
   backBufferHeight = m_height;
#endif
   m_pBackBuffer = new RenderTarget(this, backBufferWidth, backBufferHeight, back_buffer_format);

#ifdef ENABLE_SDL
#ifndef __OPENGLES__
   const colorFormat render_format = ((m_BWrendering == 1) ? colorFormat::RG16F : ((m_BWrendering == 2) ? colorFormat::RED16F : colorFormat::RGB16F));
#else
   const colorFormat render_format = ((m_BWrendering == 1) ? colorFormat::RG16F : ((m_BWrendering == 2) ? colorFormat::RED16F : colorFormat::RGBA16F));
#endif
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

   #ifdef ENABLE_SDL
   SurfaceType rtType = m_stereo3D == STEREO_OFF ? SurfaceType::RT_DEFAULT : SurfaceType::RT_STEREO;
   #else
   // For the time being DirectX 9 does not support any fancy stereo
   SurfaceType rtType = SurfaceType::RT_DEFAULT;
   #endif
   
   // MSAA render target which is resolved to the non MSAA render target
   if (nMSAASamples > 1) 
      m_pOffscreenMSAABackBufferTexture = new RenderTarget(this, rtType, "MSAABackBuffer"s, m_width_aa, m_height_aa, render_format, true, nMSAASamples, "Fatal Error: unable to create MSAA render buffer!");

   // Either the main render target for non MSAA, or the buffer where the MSAA render is resolved
   m_pOffscreenBackBufferTexture1 = new RenderTarget(this, rtType, "BackBuffer1"s, m_width_aa, m_height_aa, render_format, true, 1, "Fatal Error: unable to create offscreen back buffer");

   // Second render target to swap, allowing to read previous frame render for ball reflection and motion blur
   m_pOffscreenBackBufferTexture2 = m_pOffscreenBackBufferTexture1->Duplicate("BackBuffer2"s, true);

   // alloc buffer for screen space fake reflection rendering
   if (m_ssRefl)
      m_pReflectionBufferTexture = new RenderTarget(this, rtType, "ReflectionBuffer"s, m_width_aa, m_height_aa, render_format, false, 1, "Fatal Error: unable to create reflection buffer!");

   // alloc bloom tex at 1/4 x 1/4 res (allows for simple HQ downscale of clipped input while saving memory)
   m_pBloomBufferTexture = new RenderTarget(this, rtType, "BloomBuffer1"s, m_width / 4, m_height / 4, render_format, false, 1, "Fatal Error: unable to create bloom buffer!");
   m_pBloomTmpBufferTexture = m_pBloomBufferTexture->Duplicate("BloomBuffer2"s);

   #ifdef ENABLE_SDL
   if (m_stereo3D == STEREO_VR) {
      //AMD Debugging
      colorFormat renderBufferFormatVR;
      const int textureModeVR = g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::PlayerVR, "EyeFBFormat"s, 1);
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
      m_pOffscreenVRLeft = new RenderTarget(this, SurfaceType::RT_DEFAULT, "VRLeft"s, m_width, m_height, renderBufferFormatVR, false, 1, "Fatal Error: unable to create left eye buffer!");
      m_pOffscreenVRRight = new RenderTarget(this, SurfaceType::RT_DEFAULT, "VRRight"s, m_width, m_height, renderBufferFormatVR, false, 1, "Fatal Error: unable to create right eye buffer!");
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
#ifndef __OPENGLES__
   UploadAndSetSMAATextures();
#endif

   // Force applying a defined initial render state
   m_current_renderstate.m_state = (~m_renderstate.m_state) & ((1 << 21) - 1);
   m_current_renderstate.m_depthBias = m_renderstate.m_depthBias - 1.0f;
   ApplyRenderStates();
   
   // Ensure we have a VSync source for frame pacing
   bool hasVSync = false;
   if (m_dwm_enabled && mDwmFlush)
   {
      PLOGI << "VSync source set to Desktop compositor (DwmFlush)";
      hasVSync = true;
   }
   #ifndef ENABLE_SDL
   else if (m_pD3DDeviceEx != nullptr)
   {
      PLOGI << "VSync source set to DX9Ex WaitForBlank";
      hasVSync = true;
   }
   #endif
   #ifdef ENABLE_SDL
   // DXGI VSync source (Windows 7+, only used in OpenGL build)
   else if (syncMode == VideoSyncMode::VSM_FRAME_PACING)
   {
#ifndef __STANDALONE__
      DXGIRegistry::Output* out = g_DXGIRegistry.GetForWindow(m_windowHwnd);
      if (out != nullptr)
         m_DXGIOutput = out->m_Output;
      if (m_DXGIOutput != nullptr)
      {
	      PLOGI << "VSync source set to DXGI WaitForBlank";
         hasVSync = true;
      }
#else
      hasVSync = false;
#endif
   }
   #endif
   if (syncMode == VideoSyncMode::VSM_FRAME_PACING && !hasVSync)
   {
      // This may happen on some old config where D3D9ex is not available (XP/Vista/7) and DWM is disabled
      ShowError("Failed to create the synchronization device.\r\nSynchronization switched to adaptive sync.");
      PLOGE << "Failed to create the synchronization device for frame pacing. Synchronization switched to adaptive sync.";
      syncMode = VideoSyncMode::VSM_ADAPTIVE_VSYNC;
      #ifdef ENABLE_SDL
      SDL_GL_SetSwapInterval(-1);
      #endif
   }
}

bool RenderDevice::LoadShaders()
{
#ifdef ENABLE_SDL // OpenGL
   basicShader = new Shader(this, "BasicShader.glfx"s);
   DMDShader = new Shader(this, m_stereo3D == STEREO_VR ? "DMDShaderVR.glfx"s : "DMDShader.glfx"s);
#ifndef __OPENGLES__
   FBShader = new Shader(this, "FBShader.glfx"s, "SMAA.glfx"s);
#else
   FBShader = new Shader(this, "FBShader.glfx"s);
#endif
   flasherShader = new Shader(this, "FlasherShader.glfx"s);
   lightShader = new Shader(this, "LightShader.glfx"s);
   StereoShader = new Shader(this, "StereoShader.glfx"s);
   m_ballShader = new Shader(this, "BallShader.glfx"s);
#else // DirectX 9
   basicShader = new Shader(this, "BasicShader.hlsl"s, g_basicShaderCode, sizeof(g_basicShaderCode));
   DMDShader = new Shader(this, "DMDShader.hlsl"s, g_dmdShaderCode, sizeof(g_dmdShaderCode));
   FBShader = new Shader(this, "FBShader.hlsl"s, g_FBShaderCode, sizeof(g_FBShaderCode));
   flasherShader = new Shader(this, "FlasherShader.hlsl"s, g_flasherShaderCode, sizeof(g_flasherShaderCode));
   lightShader = new Shader(this, "LightShader.hlsl"s, g_lightShaderCode, sizeof(g_lightShaderCode));
   StereoShader = new Shader(this, "StereoShader.hlsl"s, g_stereoShaderCode, sizeof(g_stereoShaderCode));
   m_ballShader = new Shader(this, "BallShader.hlsl"s, g_ballShaderCode, sizeof(g_ballShaderCode));
#endif

   if (basicShader->HasError() || DMDShader->HasError() || FBShader->HasError() || flasherShader->HasError() || lightShader->HasError() || StereoShader->HasError())
   {
      ReportError("Fatal Error: shader compilation failed!", -1, __FILE__, __LINE__);
      return false;
   }

   // Initialize uniform to default value
   basicShader->SetVector(SHADER_w_h_height, (float)(1.0 / (double)GetMSAABackBufferTexture()->GetWidth()), (float)(1.0 / (double)GetMSAABackBufferTexture()->GetHeight()), 0.0f, 0.0f);
   basicShader->SetVector(SHADER_staticColor_Alpha, 1.0f, 1.0f, 1.0f, 1.0f); // No tinting
   DMDShader->SetFloat(SHADER_alphaTestValue, 1.0f); // No alpha clipping
#ifndef __OPENGLES__
   FBShader->SetTexture(SHADER_areaTex, m_SMAAareaTexture);
   FBShader->SetTexture(SHADER_searchTex, m_SMAAsearchTexture);
#endif

   return true;
}

RenderTarget* RenderDevice::GetPostProcessRenderTarget1()
{
   if (m_pPostProcessRenderTarget1 == nullptr)
   {
      // Buffers for post-processing. Postprocess is done at scene resolution, on a LDR render target without MSAA nor full scene supersampling
      // excepted when using downsampled render buffer where upscaling is done after postprocessing.
      const colorFormat pp_format = GetBackBufferTexture()->GetColorFormat() == RGBA10 ? colorFormat::RGBA10 : colorFormat::RGBA8;
      int width = m_AAfactor < 1 ? m_pOffscreenBackBufferTexture1->GetWidth() : m_width;
      int height = m_AAfactor < 1 ? m_pOffscreenBackBufferTexture1->GetHeight() : m_height;
      m_pPostProcessRenderTarget1 = new RenderTarget(this, m_pOffscreenBackBufferTexture1->m_type, "PostProcess1"s, width, height, pp_format, false, 1, 
         "Fatal Error: unable to create stereo3D/post-processing AA/sharpen buffer!");
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
   // Lazily creates AO render target since this can be enabled during play from script (at render buffer resolution)
   if (m_pAORenderTarget1 == nullptr)
   {
      m_pAORenderTarget1 = new RenderTarget(this, m_pOffscreenBackBufferTexture1->m_type, "AO1"s, 
         m_pOffscreenBackBufferTexture1->GetWidth(), m_pOffscreenBackBufferTexture1->GetHeight(), colorFormat::GREY8, false, 1, 
         "Unable to create AO buffers!\r\nPlease disable Ambient Occlusion.\r\nOr try to (un)set \"Alternative Depth Buffer processing\" in the video options!");
      m_pAORenderTarget2 = m_pAORenderTarget1->Duplicate("AO2"s);

   }
   return idx == 0 ? m_pAORenderTarget1 : m_pAORenderTarget2;
}

void RenderDevice::SwapBackBufferRenderTargets()
{
   RenderTarget* tmp = m_pOffscreenBackBufferTexture1;
   m_pOffscreenBackBufferTexture1 = m_pOffscreenBackBufferTexture2;
   m_pOffscreenBackBufferTexture2 = tmp;
}

void RenderDevice::SwapAORenderTargets()
{
   RenderTarget* tmpAO = m_pAORenderTarget1;
   m_pAORenderTarget1 = m_pAORenderTarget2;
   m_pAORenderTarget2 = tmpAO;
}

void RenderDevice::ResolveMSAA()
{
   if (m_pOffscreenMSAABackBufferTexture)
   {
      const RenderPass* initial_rt = GetCurrentPass();
      SetRenderTarget("Resolve MSAA"s, m_pOffscreenBackBufferTexture1);
      BlitRenderTarget(m_pOffscreenMSAABackBufferTexture, m_pOffscreenBackBufferTexture1, true, true);
      SetRenderTarget(initial_rt->m_name + '+', initial_rt->m_rt);
   }
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


void RenderDevice::FreeShader()
{
   UnbindSampler(nullptr);
   delete basicShader;
   basicShader = nullptr;
   delete DMDShader;
   DMDShader = nullptr;
   delete FBShader;
   FBShader = nullptr;
   delete StereoShader;
   StereoShader = nullptr;
   delete flasherShader;
   flasherShader = nullptr;
   delete lightShader;
   lightShader = nullptr;
   delete m_ballShader;
   m_ballShader = nullptr;
}

void RenderDevice::UnbindSampler(Sampler* sampler)
{
   if (basicShader)
      basicShader->UnbindSampler(sampler);
   if (DMDShader)
      DMDShader->UnbindSampler(sampler);
   if (FBShader)
      FBShader->UnbindSampler(sampler);
   if (flasherShader)
      flasherShader->UnbindSampler(sampler);
   if (lightShader)
      lightShader->UnbindSampler(sampler);
   if (StereoShader)
      StereoShader->UnbindSampler(sampler);
   if (m_ballShader)
      m_ballShader->UnbindSampler(sampler);
}

RenderDevice::~RenderDevice()
{
   delete m_quadMeshBuffer;
   delete m_quadPTDynMeshBuffer;
   delete m_quadPNTDynMeshBuffer;
   delete m_nullTexture;

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
   delete m_pOffscreenBackBufferTexture1;
   delete m_pOffscreenBackBufferTexture2;
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

   // Check for resource leak on debug builds
#ifdef _DEBUG
   IDirect3DSwapChain9* swapChain;
   CHECKD3D(m_pD3DDevice->GetSwapChain(0, &swapChain));

   D3DPRESENT_PARAMETERS pp;
   CHECKD3D(swapChain->GetPresentParameters(&pp));
   SAFE_RELEASE(swapChain);
   pp.SwapEffect = D3DSWAPEFFECT_DISCARD;

   // idea: device can't be reset if there are still allocated resources
   HRESULT hr = m_pD3DDevice->Reset(&pp);
   if (FAILED(hr))
   {
      g_pvp->MessageBox("WARNING! Direct3D resource leak detected!", "Visual Pinball", MB_ICONWARNING);
   }
#endif

   //!! if (m_pD3DDeviceEx == m_pD3DDevice) m_pD3DDevice = nullptr; //!! needed for Caligula if m_adapter > 0 ?? weird!! BUT MESSES UP FULLSCREEN EXIT (=hangs)
   SAFE_RELEASE_NO_RCC(m_pD3DDeviceEx);
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
   SAFE_RELEASE_NO_RCC(m_pD3DEx);
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
      turnVROff();
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

   assert(m_sharedVAOs.empty());
#endif

   assert(m_pendingSharedIndexBuffers.empty());
   assert(m_pendingSharedVertexBuffers.empty());
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
#ifndef ENABLE_SDL
   if (m_pD3DEx && frames > 0 && frames <= 20) // frames can range from 1 to 20, 0 resets to default DX
   {
      CHECKD3D(m_pD3DDeviceEx->SetMaximumFrameLatency(frames));
      return true;
   }
   else
#endif
      return false;
}

void RenderDevice::WaitForVSync(const bool asynchronous)
{
   // - DWM is always disabled for Windows XP, it can be either on or off for Windows Vista/7, it is always enabled for Windows 8+ except on stripped down versions of Windows like Ghost Spectre
   // - Windows XP does not offer any way to sync beside the present parameter on device creation, so this is enforced there and the vsync parameter will be ignored here
   //   (note that the present parameter does not directly sync: it schedules the flip on vsync, leading the GPU to block on another render call, since no backbuffer is available for drawing then)
   auto lambda = [this]()
   {
#ifndef __STANDALONE__
      if (m_dwm_enabled && mDwmFlush != nullptr)
         mDwmFlush(); // Flush all commands submited by this process including the 'Present' command. This actually sync to the vertical blank
      #ifdef ENABLE_SDL
      else if (m_DXGIOutput != nullptr)
         m_DXGIOutput->WaitForVBlank();
      #else
      // When DWM is disabled (Windows Vista/7), exclusive fullscreen without DWM (pre-windows 10), special Windows builds with DWM stripped out (Ghost Spectre Windows 10)
      else if (m_pD3DDeviceEx != nullptr)
         m_pD3DDeviceEx->WaitForVBlank(0);
      #endif
#endif
      m_vsyncCount++;
      const U64 now = usec();
      m_lastVSyncUs = now;
      //static U64 lastUs = 0;
      //PLOGD_(PLOG_NO_DBG_OUT_INSTANCE_ID) << "VSYNC " << ((double)(now - lastUs) / 1000.0) << "ms";
      //lastUs = now;
   };
   if (asynchronous)
      std::thread(lambda).detach(); // Reuse thread ? (we always at most one running at a time)
   else
      lambda();
}

// Schedule frame presentation (usually by flipping the front & back buffer)
void RenderDevice::Flip()
{
   // The calls below may or may not block, depending on the device configuration and the state of its frame queue. The driver may also
   // block on the first draw call that needs to access a backbuffer when they are all waiting to be presented. To ensure non blocking 
   // calls, we need to schedule frames at a pace adjusted to the actual render speed (to avoid filling up the queue, leading to subsequent call to wait).
   //
   // This matters and should be avoided since these blocking calls will delay the input/physics update (they catchup afterward) and that 
   // it will break some pinmame video modes (since input events will be fast forwarded, the controller missing somes like in Lethal 
   // Weapon 3 fight) and make the gameplay (input lag, input-physics sync, input-controller sync) to depend on the framerate.

   // Ensure that all commands have been submitted to the CPU, then pushed to the GPU
   FlushRenderFrame();

   // Schedule frame presentation (non blocking call, simply queueing the present command in the driver's render queue with a schedule for execution)
   if (m_stereo3D != STEREO_VR)
      g_frameProfiler.OnPresent();
   #ifdef ENABLE_SDL
   SDL_GL_SwapWindow(m_sdl_playfieldHwnd);
   #else
   CHECKD3D(m_pD3DDevice->Present(nullptr, nullptr, nullptr, nullptr));
   #endif

   // reset performance counters
   m_frameDrawCalls = m_curDrawCalls;
   m_curDrawCalls = 0;
   m_frameStateChanges = m_curStateChanges;
   m_curStateChanges = 0;
   m_frameTextureChanges = m_curTextureChanges;
   m_curTextureChanges = 0;
   m_frameParameterChanges = m_curParameterChanges;
   m_curParameterChanges = 0;
   m_frameTechniqueChanges = m_curTechniqueChanges;
   m_curTechniqueChanges = 0;
   m_frameDrawnTriangles = m_curDrawnTriangles;
   m_curDrawnTriangles = 0;
   m_frameTextureUpdates = m_curTextureUpdates;
   m_curTextureUpdates = 0;
   m_frameLockCalls = m_curLockCalls;
   m_curLockCalls = 0;
}

void RenderDevice::UploadAndSetSMAATextures()
{
   // FIXME use standard BaseTexture / Sampler code instead
   /* BaseTexture* searchBaseTex = new BaseTexture(SEARCHTEX_WIDTH, SEARCHTEX_HEIGHT, BaseTexture::BW);
   memcpy(searchBaseTex->data(), searchTexBytes, SEARCHTEX_SIZE);
   m_SMAAsearchTexture = new Sampler(this, searchBaseTex, true, SamplerAddressMode::SA_CLAMP, SamplerAddressMode::SA_CLAMP, SamplerFilter::SF_NONE);
   m_SMAAsearchTexture->SetName("SMAA Search"s);
   delete searchBaseTex;*/

#ifdef ENABLE_SDL
   auto tex_unit = m_samplerBindings.back();
   if (tex_unit->sampler != nullptr)
      tex_unit->sampler->m_bindings.erase(tex_unit);
   tex_unit->sampler = nullptr;
   glActiveTexture(GL_TEXTURE0 + tex_unit->unit);
   GLuint glTexture[2];
   glGenTextures(2, glTexture);

   glBindTexture(GL_TEXTURE_2D, glTexture[0]);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexStorage2D(GL_TEXTURE_2D, 1, GL_R8, SEARCHTEX_WIDTH, SEARCHTEX_HEIGHT);
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
   glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, SEARCHTEX_WIDTH, SEARCHTEX_HEIGHT, GL_RED, GL_UNSIGNED_BYTE, (void*)searchTexBytes);
   m_SMAAsearchTexture = new Sampler(this, SurfaceType::RT_DEFAULT, glTexture[0], true, true, SamplerAddressMode::SA_CLAMP, SamplerAddressMode::SA_CLAMP, SamplerFilter::SF_NONE);
   m_SMAAsearchTexture->SetName("SMAA Search"s);

   glBindTexture(GL_TEXTURE_2D, glTexture[1]);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexStorage2D(GL_TEXTURE_2D, 1, GL_RG8, AREATEX_WIDTH, AREATEX_HEIGHT);
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
   glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, AREATEX_WIDTH, AREATEX_HEIGHT, GL_RG, GL_UNSIGNED_BYTE, (void*)areaTexBytes);
   m_SMAAareaTexture = new Sampler(this, SurfaceType::RT_DEFAULT, glTexture[1], true, true, SamplerAddressMode::SA_CLAMP, SamplerAddressMode::SA_CLAMP, SamplerFilter::SF_BILINEAR);
   m_SMAAareaTexture->SetName("SMAA Area"s);

#else
   {
      IDirect3DTexture9 *sysTex, *tex;
      HRESULT hr = m_pD3DDevice->CreateTexture(SEARCHTEX_WIDTH, SEARCHTEX_HEIGHT, 0, 0, D3DFMT_L8, D3DPOOL_SYSTEMMEM, &sysTex, nullptr);
      if (FAILED(hr))
         ReportError("Fatal Error: unable to create texture!", hr, __FILE__, __LINE__);
      hr = m_pD3DDevice->CreateTexture(SEARCHTEX_WIDTH, SEARCHTEX_HEIGHT, 0, 0, D3DFMT_L8, D3DPOOL_DEFAULT, &tex, nullptr);
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
      m_SMAAsearchTexture->SetName("SMAA Search"s);
   }
   {
      IDirect3DTexture9 *sysTex, *tex;
      HRESULT hr = m_pD3DDevice->CreateTexture(AREATEX_WIDTH, AREATEX_HEIGHT, 0, 0, D3DFMT_A8L8, D3DPOOL_SYSTEMMEM, &sysTex, nullptr);
      if (FAILED(hr))
         ReportError("Fatal Error: unable to create texture!", hr, __FILE__, __LINE__);
      hr = m_pD3DDevice->CreateTexture(AREATEX_WIDTH, AREATEX_HEIGHT, 0, 0, D3DFMT_A8L8, D3DPOOL_DEFAULT, &tex, nullptr);
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
      m_SMAAareaTexture->SetName("SMAA Area"s);
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

void RenderDevice::SetRenderStateDepthBias(float bias)
{
   m_renderstate.SetRenderStateDepthBias(bias);
}

void RenderDevice::EnableAlphaBlend(const bool additiveBlending, const bool set_dest_blend, const bool set_blend_op)
{
   SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_TRUE);
   SetRenderState(RenderState::SRCBLEND, RenderState::SRC_ALPHA);
   if (set_dest_blend)
      SetRenderState(RenderState::DESTBLEND, additiveBlending ? RenderState::ONE : RenderState::INVSRC_ALPHA);
   if (set_blend_op)
      SetRenderState(RenderState::BLENDOP, RenderState::BLENDOP_ADD);
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

void RenderDevice::CopyRenderStates(const bool copyTo, RenderDeviceState& state)
{
   assert(state.m_rd == this);
   CopyRenderStates(copyTo, state.m_renderState);
   basicShader->m_state->CopyTo(copyTo, state.m_basicShaderState);
   DMDShader->m_state->CopyTo(copyTo, state.m_DMDShaderState);
   FBShader->m_state->CopyTo(copyTo, state.m_FBShaderState);
   flasherShader->m_state->CopyTo(copyTo, state.m_flasherShaderState);
   lightShader->m_state->CopyTo(copyTo, state.m_lightShaderState);
   m_ballShader->m_state->CopyTo(copyTo, state.m_ballShaderState);
   StereoShader->m_state->CopyTo(copyTo, state.m_stereoShaderState);
}

void RenderDevice::SetClipPlane(const vec4 &plane)
{
#ifdef ENABLE_SDL
   DMDShader->SetVector(SHADER_clip_plane, &plane);
   basicShader->SetVector(SHADER_clip_plane, &plane);
   lightShader->SetVector(SHADER_clip_plane, &plane);
   flasherShader->SetVector(SHADER_clip_plane, &plane);
   m_ballShader->SetVector(SHADER_clip_plane, &plane);
#else
   // FIXME shouldn't we set the Model matrix to identity first ?
   Matrix3D mT = g_pplayer->m_pin3d.GetMVP().GetModelViewProj(0); // = world * view * proj
   mT.Invert();
   mT.Transpose();
   const D3DXMATRIX m(mT);
   D3DXPLANE clipSpacePlane;
   const D3DXPLANE dxplane(-plane.x, -plane.y, -plane.z, -plane.w);
   D3DXPlaneTransform(&clipSpacePlane, &dxplane, &m);
   GetCoreDevice()->SetClipPlane(0, clipSpacePlane);
#endif
}

void RenderDevice::FlushRenderFrame()
{
   bool rendered = m_renderFrame.Execute(m_logNextFrame);
   m_currentPass = nullptr;
   if (rendered)
      m_logNextFrame = false;
}

void RenderDevice::SetRenderTarget(const string& name, RenderTarget* rt, const bool useRTContent, const bool forceNewPass)
{
   if (rt == nullptr)
   {
      m_currentPass = nullptr;
   }
   else if (m_currentPass == nullptr || !useRTContent || rt != m_currentPass->m_rt || forceNewPass)
   {
      m_currentPass = m_renderFrame.AddPass(name, rt);
      if (useRTContent && rt->m_lastRenderPass != nullptr)
      {
         for (auto precursors : rt->m_lastRenderPass->m_dependencies)
            m_currentPass->AddPrecursor(precursors);
         m_currentPass->AddPrecursor(rt->m_lastRenderPass);
      }
      rt->m_lastRenderPass = m_currentPass;
   }
}

void RenderDevice::AddRenderTargetDependency(RenderTarget* rt, const bool needDepth)
{
   if (m_currentPass != nullptr && rt->m_lastRenderPass != nullptr)
   {
      rt->m_lastRenderPass->m_depthReadback |= needDepth;
      m_currentPass->AddPrecursor(rt->m_lastRenderPass);
   }
}

void RenderDevice::AddRenderTargetDependencyOnNextRenderCommand(RenderTarget* rt)
{
   assert(m_nextRenderCommandDependency == nullptr); // Only one dependency can be added on a render command
   m_nextRenderCommandDependency = rt->m_lastRenderPass;
}

void RenderDevice::Clear(const DWORD flags, const D3DCOLOR color, const D3DVALUE z, const DWORD stencil)
{
   ApplyRenderStates();
   RenderCommand* cmd = m_renderFrame.NewCommand();
   cmd->SetClear(flags, color);
   cmd->m_dependency = m_nextRenderCommandDependency;
   m_nextRenderCommandDependency = nullptr;
   m_currentPass->Submit(cmd);
}

void RenderDevice::BlitRenderTarget(RenderTarget* source, RenderTarget* destination, bool copyColor, bool copyDepth, const int x1, const int y1, const int w1, const int h1, const int x2,
   const int y2, const int w2, const int h2, const int srcLayer, const int dstLayer)
{
   assert(m_currentPass->m_rt == destination); // We must be on a render pass targeted at the destination for correct render pass sorting
   AddRenderTargetDependency(source);
   RenderCommand* cmd = m_renderFrame.NewCommand();
   cmd->SetCopy(source, destination, copyColor, copyDepth, x1, y1, w1, h1, x2, y2, w2, h2, srcLayer, dstLayer);
   cmd->m_dependency = m_nextRenderCommandDependency;
   m_nextRenderCommandDependency = nullptr;
   m_currentPass->Submit(cmd);
}

void RenderDevice::SubmitVR(RenderTarget* source)
{
   AddRenderTargetDependency(source);
   RenderCommand* cmd = m_renderFrame.NewCommand();
   cmd->SetSubmitVR(source);
   cmd->m_dependency = m_nextRenderCommandDependency;
   m_nextRenderCommandDependency = nullptr;
   m_currentPass->Submit(cmd);
}

void RenderDevice::RenderLiveUI()
{
   RenderCommand* cmd = m_renderFrame.NewCommand();
   cmd->SetRenderLiveUI();
   cmd->m_dependency = m_nextRenderCommandDependency;
   m_nextRenderCommandDependency = nullptr;
   m_currentPass->Submit(cmd);
}

void RenderDevice::RenderLiveUI(int LR)
{
   RenderCommand* cmd = m_renderFrame.NewCommand();
   cmd->SetRenderLiveUI(LR);
   cmd->m_dependency = m_nextRenderCommandDependency;
   m_nextRenderCommandDependency = nullptr;
   m_currentPass->Submit(cmd);
}

void RenderDevice::DrawTexturedQuad(Shader* shader, const Vertex3D_TexelOnly* vertices)
{
   assert(shader == FBShader || shader == StereoShader); // FrameBuffer/Stereo shader are the only ones using Position/Texture vertex format
   ApplyRenderStates();
   RenderCommand* cmd = m_renderFrame.NewCommand();
   cmd->SetDrawTexturedQuad(shader, vertices);
   cmd->m_dependency = m_nextRenderCommandDependency;
   m_nextRenderCommandDependency = nullptr;
   m_currentPass->Submit(cmd);
}

void RenderDevice::DrawTexturedQuad(Shader* shader, const Vertex3D_NoTex2* vertices)
{
   assert(shader != FBShader && shader != StereoShader); // FrameBuffer/Stereo shader are the only ones using Position/Texture vertex format
   ApplyRenderStates();
   RenderCommand* cmd = m_renderFrame.NewCommand();
   cmd->SetDrawTexturedQuad(shader, vertices);
   cmd->m_dependency = m_nextRenderCommandDependency;
   m_nextRenderCommandDependency = nullptr;
   m_currentPass->Submit(cmd);
}

void RenderDevice::DrawFullscreenTexturedQuad(Shader* shader)
{
   assert(shader == FBShader || shader == StereoShader); // FrameBuffer/Stereo shader are the only ones using Position/Texture vertex format
   static const Vertex3Ds pos(0.f, 0.f, 0.f);
   DrawMesh(shader, false, pos, 0.f, m_quadMeshBuffer, TRIANGLESTRIP, 0, 4);
}

void RenderDevice::DrawMesh(Shader* shader, const bool isTranparentPass, const Vertex3Ds& center, const float depthBias, MeshBuffer* mb, const PrimitiveTypes type, const DWORD startIndex, const DWORD indexCount)
{
   RenderCommand* cmd = m_renderFrame.NewCommand();
   // Legacy sorting order (only along negative z axis, which is reversed for reflections).
   // This is completely wrong but needed to preserve backward compatibility. We should sort along the view axis (especially for reflection probes)
   const float depth = g_pplayer->IsRenderPass(Player::REFLECTION_PASS) ? depthBias + center.z : depthBias - center.z;
   // We can not use the real opacity from render states since some legacy uses alpha part that write to the depth buffer (rendered during transparent pass) to mask out opaque parts
   cmd->SetDrawMesh(shader, mb, type, startIndex, indexCount, isTranparentPass /* && !GetRenderState().IsOpaque() */, depth);
   cmd->m_dependency = m_nextRenderCommandDependency;
   m_nextRenderCommandDependency = nullptr;
   m_currentPass->Submit(cmd);
}

void RenderDevice::DrawGaussianBlur(RenderTarget* source, RenderTarget* tmp, RenderTarget* dest, float kernel_size, int singleLayer)
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

   const RenderPass* initial_rt = GetCurrentPass();
   RenderState initial_state;
   CopyRenderStates(true, initial_state);
   ResetRenderState();
   SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_FALSE);
   SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
   SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
   SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);
   {
      FBShader->SetTextureNull(SHADER_tex_fb_filtered);
      SetRenderTarget(initial_rt->m_name + " HBlur", tmp, false); // switch to temporary output buffer for horizontal phase of gaussian blur
      m_currentPass->m_singleLayerRendering = singleLayer; // We support bluring a single layer (for anaglyph defocusing)
      AddRenderTargetDependency(source);
      FBShader->SetTexture(SHADER_tex_fb_filtered, source->GetColorSampler());
      FBShader->SetVector(SHADER_w_h_height, (float)(1.0 / source->GetWidth()), (float)(1.0 / source->GetHeight()), 1.0f, 1.0f);
      FBShader->SetTechnique(tech_h);
      DrawFullscreenTexturedQuad(FBShader);
   }
   {
      FBShader->SetTextureNull(SHADER_tex_fb_filtered);
      SetRenderTarget(initial_rt->m_name + " VBlur", dest, false); // switch to output buffer for vertical phase of gaussian blur
      m_currentPass->m_singleLayerRendering = singleLayer; // We support bluring a single layer (for anaglyph defocusing)
      AddRenderTargetDependency(tmp);
      FBShader->SetTexture(SHADER_tex_fb_filtered, tmp->GetColorSampler());
      FBShader->SetVector(SHADER_w_h_height, (float)(1.0 / tmp->GetWidth()), (float)(1.0 / tmp->GetHeight()), 1.0f, 1.0f);
      FBShader->SetTechnique(tech_v);
      DrawFullscreenTexturedQuad(FBShader);
   }
   CopyRenderStates(false, initial_state);
   SetRenderTarget(initial_rt->m_name + '+', initial_rt->m_rt, true);
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

void RenderDevice::SaveVRSettings(Settings& settings) const
{
   #ifdef ENABLE_VR
   settings.SaveValue(Settings::PlayerVR, "Slope"s, m_slope);
   settings.SaveValue(Settings::PlayerVR, "Orientation"s, m_orientation);
   settings.SaveValue(Settings::PlayerVR, "TableX"s, m_tablex);
   settings.SaveValue(Settings::PlayerVR, "TableY"s, m_tabley);
   settings.SaveValue(Settings::PlayerVR, "TableZ"s, m_tablez);
   #endif
}

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

   // Move from VP units to meters, and also apply user scene scaling if any
   Matrix3D sceneScale = Matrix3D::MatrixScale(m_scale);

   // Convert from VPX coords to VR (270deg rotation around X axis, and flip x axis)
   Matrix3D coords;
   coords.SetIdentity();
   coords._11 = -1.f; coords._12 = 0.f; coords._13 =  0.f;
   coords._21 =  0.f; coords._22 = 0.f; coords._23 = -1.f;
   coords._31 =  0.f; coords._32 = 1.f; coords._33 =  0.f;

   float zNear, zFar;
   g_pplayer->m_ptable->ComputeNearFarPlane(coords * sceneScale, m_scale, zNear, zFar);
   zNear = g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::PlayerVR, "NearPlane"s, 5.0f) / 100.0f; // Replace near value to allow player to move near parts up to user defined value
   zFar *= 1.2f;

   if (m_pHMD == nullptr)
   {
      // Basic debug setup (All OpenVR matrices are left handed, using meter units)
      // For debugging without a headset, the null driver of OpenVR should be enabled. To do so:
      // - Set "enable": true in default.vrsettings from C:\Program Files (x86)\Steam\steamapps\common\SteamVR\drivers\null\resources\settings
      // - Add "activateMultipleDrivers" : true, "forcedDriver" : "null", to "steamvr" section of steamvr.vrsettings from C :\Program Files(x86)\Steam\config
      uint32_t eye_width = 1080, eye_height = 1200; // Oculus Rift resolution
      m_width = eye_width;
      m_height = eye_height;
      for (int i = 0; i < 2; i++)
      {
         Matrix3D proj;
         proj.SetPerspectiveFovLH(90.f, 1.f, zNear, zFar);
         m_vrMatProj[i] = coords * sceneScale * proj;
      }
   }
   else
   {
      uint32_t eye_width, eye_height;
      m_pHMD->GetRecommendedRenderTargetSize(&eye_width, &eye_height);
      m_width = eye_width;
      m_height = eye_height;
      vr::HmdMatrix34_t left_eye_pos = m_pHMD->GetEyeToHeadTransform(vr::Eye_Left);
      vr::HmdMatrix34_t right_eye_pos = m_pHMD->GetEyeToHeadTransform(vr::Eye_Right);
      vr::HmdMatrix44_t left_eye_proj = m_pHMD->GetProjectionMatrix(vr::Eye_Left, zNear, zFar);
      vr::HmdMatrix44_t right_eye_proj = m_pHMD->GetProjectionMatrix(vr::Eye_Right, zNear, zFar);

      Matrix3D matEye2Head, matProjection;

      //Calculate left EyeProjection Matrix relative to HMD position
      matEye2Head.SetIdentity();
      for (int i = 0; i < 3; i++)
         for (int j = 0;j < 4;j++)
            matEye2Head.m[j][i] = left_eye_pos.m[i][j];
      matEye2Head.Invert();

      left_eye_proj.m[2][2] = -1.0f;
      left_eye_proj.m[2][3] = -zNear;
      for (int i = 0;i < 4;i++)
         for (int j = 0;j < 4;j++)
            matProjection.m[j][i] = left_eye_proj.m[i][j];

      m_vrMatProj[0] = coords * sceneScale * matEye2Head * matProjection;

      //Calculate right EyeProjection Matrix relative to HMD position
      matEye2Head.SetIdentity();
      for (int i = 0; i < 3; i++)
         for (int j = 0;j < 4;j++)
            matEye2Head.m[j][i] = right_eye_pos.m[i][j];
      matEye2Head.Invert();

      right_eye_proj.m[2][2] = -1.0f;
      right_eye_proj.m[2][3] = -zNear;
      for (int i = 0;i < 4;i++)
         for (int j = 0;j < 4;j++)
            matProjection.m[j][i] = right_eye_proj.m[i][j];

      m_vrMatProj[1] = coords * sceneScale * matEye2Head * matProjection;
   }

   if (vr::k_unMaxTrackedDeviceCount > 0) {
      m_rTrackedDevicePose = new vr::TrackedDevicePose_t[vr::k_unMaxTrackedDeviceCount];
   }
   else {
      std::runtime_error noDevicesFound("No Tracking devices found");
      throw(noDevicesFound);
   }

   m_slope = g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::PlayerVR, "Slope"s, 6.5f);
   m_orientation = g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::PlayerVR, "Orientation"s, 0.0f);
   m_tablex = g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::PlayerVR, "TableX"s, 0.0f);
   m_tabley = g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::PlayerVR, "TableY"s, 0.0f);
   m_tablez = g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::PlayerVR, "TableZ"s, 80.0f);

   updateTableMatrix();
}

void RenderDevice::UpdateVRPosition(ModelViewProj& mvp)
{
   mvp.SetProj(0, m_vrMatProj[0]);
   mvp.SetProj(1, m_vrMatProj[1]);

   Matrix3D matView;
   matView.SetIdentity();

   if (IsVRReady())
   {
      vr::VRCompositor()->WaitGetPoses(m_rTrackedDevicePose, vr::k_unMaxTrackedDeviceCount, nullptr, 0);
      for (unsigned int device = 0; device < vr::k_unMaxTrackedDeviceCount; device++)
      {
         if ((m_rTrackedDevicePose[device].bPoseIsValid) && (m_pHMD->GetTrackedDeviceClass(device) == vr::TrackedDeviceClass_HMD))
         {
            m_hmdPosition = m_rTrackedDevicePose[device];

            // Convert to 4x4 inverse matrix (world to head)
            for (int i = 0; i < 3; i++)
               for (int j = 0; j < 4; j++)
                  matView.m[j][i] = m_hmdPosition.mDeviceToAbsoluteTracking.m[i][j];
            matView.Invert();

            // Scale translation part
            for (int i = 0; i < 3; i++)
               matView.m[3][i] /= m_scale;

            // Convert from VPX coords to VR and back (270deg rotation around X axis, and flip x axis)
            Matrix3D coords, revCoords;
            coords.SetIdentity();
            coords._11 = -1.f; coords._12 = 0.f; coords._13 =  0.f;
            coords._21 =  0.f; coords._22 = 0.f; coords._23 = -1.f;
            coords._31 =  0.f; coords._32 = 1.f; coords._33 =  0.f;
            revCoords.SetIdentity();
            revCoords._11 = -1.f; revCoords._12 =  0.f; revCoords._13 = 0.f;
            revCoords._21 =  0.f; revCoords._22 =  0.f; revCoords._23 = 1.f;
            revCoords._31 =  0.f; revCoords._32 = -1.f; revCoords._33 = 0.f;
            matView = coords * matView * revCoords;

            break;
         }
      }
   }

   // Apply table world position
   mvp.SetView(m_tableWorld * matView);
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
   const float w = m_scale * (g_pplayer->m_ptable->m_right - g_pplayer->m_ptable->m_left) * 0.5f;
   const float h = m_scale * (g_pplayer->m_ptable->m_bottom - g_pplayer->m_ptable->m_top) + 0.2f;
   float headX = 0.f, headY = 0.f;
   if (IsVRReady())
   {
      m_orientation = -RADTOANG(atan2f(m_hmdPosition.mDeviceToAbsoluteTracking.m[0][2], m_hmdPosition.mDeviceToAbsoluteTracking.m[0][0]));
      if (m_orientation < 0.0f)
         m_orientation += 360.0f;
      headX = m_hmdPosition.mDeviceToAbsoluteTracking.m[0][3];
      headY = -m_hmdPosition.mDeviceToAbsoluteTracking.m[2][3];
   }
   const float c = cosf(ANGTORAD(m_orientation));
   const float s = sinf(ANGTORAD(m_orientation));
   m_tablex = 100.0f * (headX - c * w + s * h);
   m_tabley = 100.0f * (headY + s * w + c * h);
   updateTableMatrix();
}

void RenderDevice::updateTableMatrix()
{
   Matrix3D rotx, rotz, trans, coords;

   // Tilt playfield.
   rotx.SetRotateX(ANGTORAD(-m_slope));

   // Rotate table around VR height axis
   rotz.SetRotateZ(ANGTORAD(180.f + m_orientation));
   
   // Locate front left corner of the table in the room -x is to the right, -y is up and -z is back - all units in meters
   const float inv_transScale = 1.0f / (100.0f * m_scale);
   trans.SetTranslation(-m_tablex * inv_transScale, m_tabley * inv_transScale, m_tablez * inv_transScale);

   m_tableWorld = rotx * rotz * trans;
}
#endif
