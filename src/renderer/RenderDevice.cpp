// license:GPLv3+

#include "core/stdafx.h"

//#include "Dwmapi.h" // use when we get rid of XP at some point, get rid of the manual dll loads in here then

#include <thread>

#ifndef DISABLE_FORCE_NVIDIA_OPTIMUS
#include "nvapi/nvapi.h"
#endif

#include "RenderDevice.h"
#include "RenderCommand.h"
#include "Shader.h"
#include "VRDevice.h"
#include "renderer/AreaTex.h"
#include "renderer/SearchTex.h"

#if defined(ENABLE_BGFX)
#ifdef __STANDALONE__
#pragma push_macro("_WIN64")
#undef _WIN64
#endif
#include "bx/platform.h"
#include "bx/string.h"
#include "bgfx/platform.h"
#include "bgfx/bgfx.h"
#ifdef __STANDALONE__
#pragma pop_macro("_WIN64")
#endif

#elif defined(ENABLE_OPENGL)
#include "typedefs3D.h"
#include "TextureManager.h"
#ifndef __STANDALONE__
#include "captureExt.h"
#endif

#elif defined(ENABLE_DX9)
#include "parts/Material.h"
#endif

#ifdef __LIBVPINBALL__
#include "standalone/VPinballLib.h"
#endif

#if defined(ENABLE_BGFX)
struct tBGFXCallback : public bgfx::CallbackI
{
   ~tBGFXCallback() override { }
   virtual void fatal(const char* _filePath, uint16_t _line, bgfx::Fatal::Enum _code, const char* _str) override
   {
      //bgfx::trace(_filePath, _line, "BGFX FATAL 0x%08x: %s\n", _code, _str);
      PLOGE << _filePath << ":" << _line << "BGFX FATAL " << _code << ": " << _str;
      if (bgfx::Fatal::DebugCheck == _code)
         bx::debugBreak();
      else
         abort();
   }
   virtual void traceVargs(const char* _filePath, uint16_t _line, const char* _format, va_list _argList) override
   {
      char temp[2048];
      char* out = temp;
      va_list argListCopy;
      va_copy(argListCopy, _argList);
      int32_t len = bx::snprintf(out, sizeof(temp), "%s (%d): ", _filePath, _line);
      int32_t total = len + bx::vsnprintf(out + len, sizeof(temp) - len, _format, argListCopy);
      va_end(argListCopy);
      if ((int32_t)sizeof(temp) < total)
      {
         out = (char*)alloca(total + 1);
         bx::memCopy(out, temp, len);
         bx::vsnprintf(out + len, total - len, _format, _argList);
      }
      out[total] = '\0';
      bx::debugOutput(out);
      if (total > 0 && out[total - 1] == '\n')
         out[total - 1] = '\0';
      PLOGI << out;
   }
   virtual void profilerBegin(const char* /*_name*/, uint32_t /*_abgr*/, const char* /*_filePath*/, uint16_t /*_line*/) override { }
   virtual void profilerBeginLiteral(const char* /*_name*/, uint32_t /*_abgr*/, const char* /*_filePath*/, uint16_t /*_line*/) override { }
   virtual void profilerEnd() override { }
   virtual uint32_t cacheReadSize(uint64_t /*_id*/) override { return 0; }
   virtual bool cacheRead(uint64_t /*_id*/, void* /*_data*/, uint32_t /*_size*/) override { return false; }
   virtual void cacheWrite(uint64_t /*_id*/, const void* /*_data*/, uint32_t /*_size*/) override { }
   virtual void screenShot(const char* _filePath, uint32_t _width, uint32_t _height, uint32_t _pitch, const void* _data, uint32_t _size, bool _yflip) override { }
   virtual void captureBegin(uint32_t /*_width*/, uint32_t /*_height*/, uint32_t /*_pitch*/, bgfx::TextureFormat::Enum /*_format*/, bool /*_yflip*/) override { }
   virtual void captureEnd() override { }
   virtual void captureFrame(const void* /*_data*/, uint32_t /*_size*/) override { }
} bgfxCallback;

#elif defined(ENABLE_OPENGL)
GLuint RenderDevice::m_samplerStateCache[3 * 3 * 5];
static const char* glErrorToString(const int error)
{
   switch (error)
   {
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
#if 0 // not used anymore
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
#if defined(_DEBUG) && !defined(__OPENGLES__)
void APIENTRY GLDebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* msg, const void* data)
{
   char* _source;
   switch (source)
   {
   case GL_DEBUG_SOURCE_API: _source = (LPSTR) "API"; break;
   case GL_DEBUG_SOURCE_WINDOW_SYSTEM: _source = (LPSTR) "WINDOW SYSTEM"; break;
   case GL_DEBUG_SOURCE_SHADER_COMPILER: _source = (LPSTR) "SHADER COMPILER"; break;
   case GL_DEBUG_SOURCE_THIRD_PARTY: _source = (LPSTR) "THIRD PARTY"; break;
   case GL_DEBUG_SOURCE_APPLICATION: _source = (LPSTR) "APPLICATION"; break;
   case GL_DEBUG_SOURCE_OTHER: _source = (LPSTR) "UNKNOWN"; break;
   default: _source = (LPSTR) "UNHANDLED"; break;
   }
   char* _type;
   switch (type)
   {
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
   switch (severity)
   {
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

#elif defined(ENABLE_DX9)
#include <DxErr.h>
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

void ReportFatalError(const HRESULT hr, const char *file, const int line)
{
   char msg[2176];
   #if defined(ENABLE_BGFX)
      sprintf_s(msg, sizeof(msg), "Fatal Error 0x%08X in %s:%d", hr, file, line);
   #elif defined(ENABLE_OPENGL)
      sprintf_s(msg, sizeof(msg), "Fatal Error 0x%08X %s in %s:%d", hr, glErrorToString(hr), file, line);
   #elif defined(ENABLE_DX9)
      sprintf_s(msg, sizeof(msg), "Fatal Error %s (0x%x: %s) at %s:%d", DXGetErrorString(hr), hr, DXGetErrorDescription(hr), file, line);
   #endif
   ShowError(msg);
   assert(false);
   exit(-1);
}

void ReportError(const char *errorText, const HRESULT hr, const char *file, const int line)
{
   char msg[16384];
   #if defined(ENABLE_BGFX)
      sprintf_s(msg, sizeof(msg), "Error 0x%08X in %s:%d\n%s", hr, file, line, errorText);
   #elif defined(ENABLE_OPENGL)
      sprintf_s(msg, sizeof(msg), "Error 0x%08X %s in %s:%d\n%s", hr, glErrorToString(hr), file, line, errorText);
   #elif defined(ENABLE_DX9)
      sprintf_s(msg, sizeof(msg), "%s %s (0x%x: %s) at %s:%d", errorText, DXGetErrorString(hr), hr, DXGetErrorDescription(hr), file, line);
   #endif
   ShowError(msg);
}

////////////////////////////////////////////////////////////////////

RenderDeviceState::RenderDeviceState(RenderDevice* rd)
   : m_rd(rd)
   , m_basicShaderState(new Shader::ShaderState(m_rd->m_basicShader, m_rd->UseLowPrecision()))
   , m_DMDShaderState(new Shader::ShaderState(m_rd->m_DMDShader, m_rd->UseLowPrecision()))
   , m_FBShaderState(new Shader::ShaderState(m_rd->m_FBShader, m_rd->UseLowPrecision()))
   , m_flasherShaderState(new Shader::ShaderState(m_rd->m_flasherShader, m_rd->UseLowPrecision()))
   , m_lightShaderState(new Shader::ShaderState(m_rd->m_lightShader, m_rd->UseLowPrecision()))
   , m_ballShaderState(new Shader::ShaderState(m_rd->m_ballShader, m_rd->UseLowPrecision()))
   , m_stereoShaderState(new Shader::ShaderState(m_rd->m_stereoShader, m_rd->UseLowPrecision()))
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

#if defined(ENABLE_DX9)
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

// MSVC Concurrency Viewer support
// This requires _WIN32_WINNT >= 0x0600 and to add the MSVC Concurrency SDK to the project
//#define MSVC_CONCURRENCY_VIEWER
#ifdef MSVC_CONCURRENCY_VIEWER
#include <cvmarkersobj.h>
using namespace Concurrency::diagnostic;
marker_series series;
#endif

#if defined(ENABLE_BGFX)
void RenderDevice::RenderThread(RenderDevice* rd, const bgfx::Init& initReq)
{
   bgfx::Init init = initReq;

   // If using OpenGl on a WCG display, then create the OpenGL WCG context through SDL since BGFX does not support HDR10 under OpenGl
   /* This won't work as is and needs more work as OpenGL is fairly wonky on this. The same approach could be used for Vulkan WCG but this is also not that well defined
   if (rd->m_outputWnd[0]->IsWCGEnabled() && init.type == bgfx::RendererType::OpenGL)
   {
      SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 10); // HDR10
      SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 10);
      SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 10);
      SDL_GL_SetAttribute(SDL_GL_FLOATBUFFERS, false);
      SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 16); // RGB16F
      SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 16);
      SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 16);
      SDL_GL_SetAttribute(SDL_GL_FLOATBUFFERS, true);
      SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
      #ifndef __OPENGLES__
         #if defined(__APPLE__) && defined(TARGET_OS_MAC)
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
         #else
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
            //This would enforce a 4.1 context, disabling all recent features (storage buffers, debug information,...)
            //SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
            //SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
         #endif
      #else
         SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
         SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
         SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
      #endif
      init.platformData.context = SDL_GL_CreateContext(rd->m_outputWnd[0]->GetCore());
      init.resolution.format = bgfx::TextureFormat::RGB10A2;
   }*/

   // If using OpenXR, we need to create a graphics layer adapted to OpenXR requirements
   #ifdef ENABLE_XR
   if (g_pplayer->m_vrDevice)
   {
      init.type = bgfx::RendererType::Direct3D11; // TODO support other backends
      init.resolution.width = max(init.resolution.width, static_cast<uint32_t>(g_pplayer->m_vrDevice->GetEyeWidth())); // Needed for bgfx::clear to work
      init.resolution.height = max(init.resolution.height, static_cast<uint32_t>(g_pplayer->m_vrDevice->GetEyeHeight())); // Needed for bgfx::clear to work
      init.resolution.reset &= ~BGFX_RESET_VSYNC; // Disable display VSync as we are synced by OpenXR on the headset display
      init.platformData.context = g_pplayer->m_vrDevice->GetGraphicContext(); // Use the context selected by OpenXR
   }
   #endif

   g_pplayer->m_renderProfiler->SetThreadLock();

   // BGFX default behavior is to set its 'API' thread (the one where bgfx API calls are allowed)
   // as the one from which init is called, and spawn a BGFX render thread in charge of submitting
   // render queue from the CPU to the GPU.
   // Since VPX already splits the logic/prepare frame thread (CPU only) from the submit/flip (CPU-GPU)
   // we do not really need BGFX to create its additional thread. Calling bgfx::renderFrame allows
   // to do so, ending up with this thread being the only BGFX thread.
   // This is also required for OpenXR which needs all the GPU submission calls to be performed after WaitFrame (sync) and between Begin/EndFrame
   bgfx::renderFrame();

   if (!bgfx::init(init))
   {
      PLOGE << "BGFX initialization failed";
      exit(-1);
   }
   
   #ifdef ENABLE_XR
   if (g_pplayer->m_vrDevice)
      g_pplayer->m_vrDevice->CreateSession();
   #endif

   // Enable HDR10 rendering if supported (so far, only DirectX 11 & 12 through DXGI)
   if ((bgfx::getCaps()->supported & BGFX_CAPS_HDR10) && (g_pplayer->m_vrDevice == nullptr))
   {
      init.resolution.format = bgfx::TextureFormat::RGB10A2;
      //init.resolution.format = bgfx::TextureFormat::RGBA16F; // Also supported by BGFX, but less efficient and would need and adjusted tonemapper to output in DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709 colorspace (linear sRGB)
      init.resolution.reset |= BGFX_RESET_HDR10;
      bgfx::reset(init.resolution.width, init.resolution.height, init.resolution.reset, init.resolution.format);
   }
   
   // Set up to dynamically toggle vsync
   if (g_pplayer->GetTargetRefreshRate() > rd->m_outputWnd[0]->GetRefreshRate())
      init.resolution.reset &= ~BGFX_RESET_VSYNC; // If targeting a FPS higher than display FPS, then disable VSYNC
   const bool useVSync = init.resolution.reset & BGFX_RESET_VSYNC;
   init.resolution.reset &= ~BGFX_RESET_VSYNC;

   //bgfx::setDebug(BGFX_DEBUG_STATS);

   // Create the back buffer render target
   colorFormat back_buffer_format;
   bool isWcg = false;
   switch (init.resolution.format)
   {
   case bgfx::TextureFormat::RGBA16F: back_buffer_format = colorFormat::RGBA16F; isWcg = true; break;
   case bgfx::TextureFormat::RGB10A2: back_buffer_format = colorFormat::RGBA10; isWcg = true; break;
   case bgfx::TextureFormat::R5G6B5: back_buffer_format = colorFormat::RGB5; break;
   case bgfx::TextureFormat::RGBA8: back_buffer_format = colorFormat::RGBA8; break;
   default: assert(false); back_buffer_format = colorFormat::RGBA8;
   }
   if (g_pplayer->m_vrDevice)
   {
      rd->m_outputWnd[1] = rd->m_outputWnd[0]; // OS window is the preview window (first window is supposed to be main rendered window, as it is directly accessed by other objects, expecting a single render window)
      rd->m_outputWnd[1]->SetBackBuffer(new RenderTarget(rd, SurfaceType::RT_DEFAULT, initReq.resolution.width, initReq.resolution.height, back_buffer_format), isWcg);
      rd->m_outputWnd[0] = new VPX::Window(g_pplayer->m_vrDevice->GetEyeWidth(), g_pplayer->m_vrDevice->GetEyeHeight());
      rd->m_nOutputWnd = 2;
      rd->m_framePending = true; // Delay first frame preparation
   }
   else
   {
      rd->m_outputWnd[0]->SetBackBuffer(new RenderTarget(rd, SurfaceType::RT_DEFAULT, init.resolution.width, init.resolution.height, back_buffer_format), isWcg);
      rd->m_framePending = false; // Request first frame to be prepared as soon as possible
   }

   // Unlock requesting thread and start render loop
   rd->m_frameReadySem.post();

   #ifdef __STANDALONE__
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
   #endif

   #ifdef ENABLE_XR
   if (g_pplayer->m_vrDevice)
   {
      // OpenXR renderloop, synchronized on headset (using xrWaitFrame), with game logic preparing frames when headset request them
      while (rd->m_renderDeviceAlive)
      {
         // Process OpenXR events (headset status, ...)
         g_pplayer->m_vrDevice->PollEvents();

         // Let OpenXR throttle rendering, preparing frame on demand when view positions are acquired and predicted display time is defined
         g_pplayer->m_vrDevice->RenderFrame(rd, [rd](RenderTarget * vrRenderTarget)
         {
            // FIXME No VR target, we should still render to the preview window
            if (vrRenderTarget == nullptr)
               return;

            // Set acquired swapchain images as render target, request a new renderframe from GameLogic thread, and wait for it
            #ifdef MSVC_CONCURRENCY_VIEWER
            span *tagSpanFF = new span(series, 1, _T("vpxWaitFrame"));
            #endif
            g_pplayer->m_renderProfiler->EnterProfileSection(FrameProfiler::PROFILE_RENDER_WAIT);
            rd->m_outputWnd[0]->SetBackBuffer(vrRenderTarget, false);
            rd->m_framePending = false;
            rd->m_frameReadySem.wait();
            rd->m_outputWnd[0]->SetBackBuffer(nullptr, false); // as the vrRenderTarget is not valid outside of this scope
            g_pplayer->m_renderProfiler->ExitProfileSection();
            #ifdef MSVC_CONCURRENCY_VIEWER
            delete tagSpanFF;
            #endif
            if (!rd->m_framePending)
               return;

            // Submit frame to BGFX (which contains all rendering commands, for VR headset but also other windows like preview,...)
            {
               #ifdef MSVC_CONCURRENCY_VIEWER
               span *tagSpan = new span(series, 1, _T("VPX->BGFX"));
               #endif
               std::lock_guard lock(rd->m_frameMutex);
               g_pplayer->m_renderProfiler->NewFrame(g_pplayer->m_time_msec);
               g_pplayer->m_renderProfiler->EnterProfileSection(FrameProfiler::PROFILE_RENDER_SUBMIT);
               rd->SubmitRenderFrame();
               g_pplayer->m_vrDevice->UpdateVisibilityMask(rd);
               g_pplayer->m_renderProfiler->ExitProfileSection();
               #ifdef MSVC_CONCURRENCY_VIEWER
               delete tagSpan;
               #endif
            }
            
            // Request BGFX to submit to GPU (calls bgfx::frame())
            #ifdef MSVC_CONCURRENCY_VIEWER
            span* tagSpan = new span(series, 1, _T("BGFX->GPU"));
            #endif
            g_pplayer->m_renderProfiler->EnterProfileSection(FrameProfiler::PROFILE_RENDER_FLIP);
            rd->Flip();
            const bgfx::Stats* stats = bgfx::getStats();
            const U32 bgfxSubmit = static_cast<U32>((stats->cpuTimeEnd - stats->cpuTimeBegin) * 1000000ull / stats->cpuTimerFreq);
            g_pplayer->m_logicProfiler.OnPresented(usec() - bgfxSubmit);
            g_pplayer->m_renderProfiler->ExitProfileSection();
            g_pplayer->m_renderProfiler->AdjustBGFXSubmit(bgfxSubmit);

            #ifdef MSVC_CONCURRENCY_VIEWER
            delete tagSpan;
            #endif
         });
      }
      g_pplayer->m_vrDevice->ReleaseSession();
      delete rd->m_outputWnd[0];
      rd->m_outputWnd[0] = rd->m_outputWnd[1];
      rd->m_nOutputWnd = 1;
   }
   else
   #endif
   {
      U64 lastFlipTick = 0;
      bool vsync = useVSync;

      // Desktop renderloop, synchronized on main display (playfield window), with game logic preparing frames as soon as possible
      while (rd->m_renderDeviceAlive)
      {
         // wait for a frame to be prepared by the logic thread
         g_pplayer->m_renderProfiler->EnterProfileSection(FrameProfiler::PROFILE_RENDER_WAIT);
         rd->m_frameReadySem.wait();
         g_pplayer->m_renderProfiler->ExitProfileSection();
         if (!rd->m_framePending)
            continue;
         const bool noSync = rd->m_frameNoSync;
         const bool needsVSync = useVSync && !noSync;

         // lock prepared frame and submit it
         {
            #ifdef MSVC_CONCURRENCY_VIEWER
            span *tagSpan = new span(series, 1, _T("VPX->BGFX"));
            #endif
            std::lock_guard lock(rd->m_frameMutex);
            g_pplayer->m_renderProfiler->NewFrame(g_pplayer->m_time_msec);
            g_pplayer->m_renderProfiler->EnterProfileSection(FrameProfiler::PROFILE_RENDER_SUBMIT);
            rd->m_framePending = false; // Request next frame to be prepared as soon as possible
            rd->m_frameNoSync = false;
            if (vsync != needsVSync)
            {
               vsync = needsVSync;
               bgfx::reset(init.resolution.width, init.resolution.height, init.resolution.reset | (vsync ? BGFX_RESET_VSYNC : BGFX_RESET_NONE), init.resolution.format);
            }
            rd->SubmitRenderFrame();
            #ifdef MSVC_CONCURRENCY_VIEWER
            delete tagSpan;
            #endif
            g_pplayer->m_renderProfiler->ExitProfileSection();
         }

         // If the user asked to sync on another frame rate than the refresh rate, then perform manual synchronization
         if (!noSync && (g_pplayer->GetTargetRefreshRate() != rd->m_outputWnd[0]->GetRefreshRate()))
         {
            g_pplayer->m_renderProfiler->EnterProfileSection(FrameProfiler::PROFILE_RENDER_SLEEP);
            #ifdef MSVC_CONCURRENCY_VIEWER
            span* tagSpan = new span(series, 1, _T("WaitSync"));
            #endif
            U64 now = usec();
            const int refreshLength = static_cast<int>(1000000. / (double)rd->m_outputWnd[0]->GetRefreshRate());
            const int minimumFrameLength = static_cast<int>(1000000. / (double)g_pplayer->GetTargetRefreshRate());
            const int maximumFrameLength = 5 * refreshLength;
            const int targetFrameLength = clamp(refreshLength - 2000, min(minimumFrameLength, maximumFrameLength), maximumFrameLength);
            while (now - lastFlipTick < targetFrameLength)
            {
               g_pplayer->m_curFrameSyncOnFPS = true;
               YieldProcessor();
               now = usec();
            }
            lastFlipTick = now;
            #ifdef MSVC_CONCURRENCY_VIEWER
            delete tagSpan;
            #endif
            g_pplayer->m_renderProfiler->ExitProfileSection();
         }

         // Flip (eventually blocking until a VSYNC happens) then submit render commands to GPU
         {
            g_pplayer->m_renderProfiler->EnterProfileSection(FrameProfiler::PROFILE_RENDER_FLIP);
            #ifdef MSVC_CONCURRENCY_VIEWER
            span* tagSpan = new span(series, 1, _T("BGFX->GPU"));
            #endif
            rd->Flip();
            #ifdef MSVC_CONCURRENCY_VIEWER
            delete tagSpan;
            #endif
            const bgfx::Stats* stats = bgfx::getStats();
            const U32 bgfxSubmit = static_cast<U32>((stats->cpuTimeEnd - stats->cpuTimeBegin) * 1000000ull / stats->cpuTimerFreq);
            g_pplayer->m_logicProfiler.OnPresented(usec() - bgfxSubmit);
            g_pplayer->m_renderProfiler->ExitProfileSection();
            g_pplayer->m_renderProfiler->AdjustBGFXSubmit(bgfxSubmit);
         }
      }
   }
   
   // Wait until main thread has released all native resources
   rd->m_frameReadySem.wait();
   delete rd->m_outputWnd[0]->GetBackBuffer();
   rd->m_outputWnd[0]->SetBackBuffer(nullptr);
   bgfx::shutdown();
}
#endif

RenderDevice::RenderDevice(VPX::Window* const wnd, const bool isVR, const int nEyes, const bool useNvidiaApi, const bool disableDWM, const bool compressTextures, const int BWrendering, int nMSAASamples, VideoSyncMode& syncMode)
   : m_isVR(isVR)
   , m_nEyes(nEyes)
   , m_texMan(*this)
   , m_renderFrame(this)
   , m_compressTextures(compressTextures)
{
   m_outputWnd[0] = wnd;

   assert(!isVR || m_nEyes == 2);

   #if defined(ENABLE_DX9)
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

        if (m_dwm_was_enabled && disableDWM && IsWindowsVistaOr7()) // windows 8 and above will not allow do disable it, but will still return S_OK
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

   assert(g_pplayer != nullptr); // Player must be created to give access to the output window

   // 0 means disable limiting of draw-ahead queue
   int maxPrerenderedFrames = isVR ? 0 : g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "MaxPrerenderedFrames"s, 0);

#if defined(ENABLE_BGFX)
   ///////////////////////////////////
   // BGFX device initialization
   bgfx::Init init;
   init.type = bgfx::RendererType::Count; // Tells BGFX to select the default backend for the running platform

   syncMode = syncMode != VideoSyncMode::VSM_NONE ? VideoSyncMode::VSM_VSYNC : VideoSyncMode::VSM_NONE;

   static const string bgfxRendererNames[bgfx::RendererType::Count + 1]
      = { "Noop"s, "Agc"s, "Direct3D11"s, "Direct3D12"s, "Gnm"s, "Metal"s, "Nvn"s, "OpenGLES"s, "OpenGL"s, "Vulkan"s, "Default"s };
#ifdef __ANDROID__
   string gfxBackend = g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "GfxBackend"s, bgfxRendererNames[bgfx::RendererType::OpenGLES]);
#elif defined(__APPLE__)
   string gfxBackend = g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "GfxBackend"s, bgfxRendererNames[bgfx::RendererType::Metal]);
#else
   string gfxBackend = g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "GfxBackend"s, bgfxRendererNames[bgfx::RendererType::Vulkan]);
#endif
   bgfx::RendererType::Enum supportedRenderers[bgfx::RendererType::Count];
   int nRendererSupported = bgfx::getSupportedRenderers(bgfx::RendererType::Count, supportedRenderers);
   string supportedRendererLog;
   for (int i = 0; i < nRendererSupported; ++i)
   {
      supportedRendererLog += (i == 0 ? "" : ", ") + bgfxRendererNames[supportedRenderers[i]];
      if (gfxBackend == bgfxRendererNames[supportedRenderers[i]])
         init.type = supportedRenderers[i];
   }
   PLOGI << "Using graphics backend: " << bgfxRendererNames[init.type] << " (available: " << supportedRendererLog << ')';
   //init.type = bgfx::RendererType::Metal;
   //init.type = bgfx::RendererType::OpenGL;
   //init.type = bgfx::RendererType::OpenGLES;
   //init.type = bgfx::RendererType::Vulkan;
   //init.type = bgfx::RendererType::Direct3D11; // Present with VSYNC & outputs on multiple displays will sequentially sync on each display causing massive framerate drop
   //init.type = bgfx::RendererType::Direct3D12; // Flasher & Ball rendering fails on a call to CreateGraphicsPipelineState, rendering artefacts

   m_useLowPrecision = init.type == bgfx::RendererType::OpenGLES;

   init.callback = &bgfxCallback;

   init.resolution.maxFrameLatency = maxPrerenderedFrames;
   init.resolution.numBackBuffers = maxPrerenderedFrames;
   // - Enable max anisotropy texture filter setting (seems like there is no finer grained setting available in BGFX?).
   // - If synchronizing on display's VSYNC, performs flip as late as possible to increase CPU/GPU parallelism (flip is likely the blocking call when vsynced, so do just before submitting next frame).
   //   If doing user synchronization, flip as soon as possible after submitting frame to limit latency.
   init.resolution.reset = BGFX_RESET_MAXANISOTROPY
                         | (syncMode == VSM_NONE ? BGFX_RESET_FLIP_AFTER_RENDER : BGFX_RESET_NONE)
                         | (syncMode == VSM_NONE ? BGFX_RESET_NONE              : BGFX_RESET_VSYNC);
   init.resolution.width = wnd->GetWidth();
   init.resolution.height = wnd->GetHeight();
   switch (wnd->GetBitDepth())
   {
   case 32: init.resolution.format = bgfx::TextureFormat::RGBA8; break;
   case 30: init.resolution.format = bgfx::TextureFormat::RGB10A2; break;
   default: init.resolution.format = bgfx::TextureFormat::R5G6B5; break;
   }

   init.platformData.context = nullptr;
   init.platformData.backBuffer = nullptr;
   init.platformData.backBufferDS = nullptr;
   #if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
   if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "x11") == 0) {
      init.platformData.ndt = SDL_GetPointerProperty(SDL_GetWindowProperties(m_outputWnd[0]->GetCore()), SDL_PROP_WINDOW_X11_DISPLAY_POINTER, NULL);
      init.platformData.nwh = (void*)SDL_GetNumberProperty(SDL_GetWindowProperties(m_outputWnd[0]->GetCore()), SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0);
   }
   else if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "wayland") == 0) {
      init.platformData.ndt = SDL_GetPointerProperty(SDL_GetWindowProperties(m_outputWnd[0]->GetCore()), SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, NULL);
      init.platformData.nwh = SDL_GetPointerProperty(SDL_GetWindowProperties(m_outputWnd[0]->GetCore()), SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, NULL);
   }
   #elif BX_PLATFORM_OSX
   init.platformData.nwh = SDL_GetRenderMetalLayer(SDL_CreateRenderer(m_outputWnd[0]->GetCore(), "Metal"));
   #elif BX_PLATFORM_IOS
   init.platformData.nwh = VPinballLib::VPinball::SendEvent(VPinballLib::Event::MetalLayerIOS, nullptr);
   #elif BX_PLATFORM_ANDROID
   init.platformData.nwh = SDL_GetPointerProperty(SDL_GetWindowProperties(m_outputWnd[0]->GetCore()), SDL_PROP_WINDOW_ANDROID_WINDOW_POINTER, NULL);
   #elif BX_PLATFORM_WINDOWS
   init.platformData.nwh = m_outputWnd[0]->GetNativeHWND();
   #elif BX_PLATFORM_STEAMLINK
   init.platformData.ndt = wmInfo.info.vivante.display;
   init.platformData.nwh = wmInfo.info.vivante.window;
   #endif // BX_PLATFORM_
   #ifdef DEBUG
   init.debug = true;
   #endif

   m_renderDeviceAlive = true;
   m_renderThread = std::thread(&RenderThread, this, init);
   m_frameReadySem.wait();
   PLOGI << "BGFX initialized using " << bgfxRendererNames[bgfx::getRendererType()] << " backend";

#elif defined(ENABLE_OPENGL)
   ///////////////////////////////////
   // OpenGL device initialization
   const SDL_DisplayMode* mode = SDL_GetCurrentDisplayMode(m_outputWnd[0]->GetAdapterId());
   if (mode == nullptr)
   {
      ShowError("Failed to setup OpenGL context");
      exit(-1);
   }
   colorFormat back_buffer_format;
   switch (mode->format)
   {
   case SDL_PIXELFORMAT_RGB565: back_buffer_format = colorFormat::RGB5; break;
   case SDL_PIXELFORMAT_XRGB8888: back_buffer_format = colorFormat::RGB8; break;
   case SDL_PIXELFORMAT_ARGB8888: back_buffer_format = colorFormat::RGBA8; break;
   case SDL_PIXELFORMAT_ARGB2101010: back_buffer_format = colorFormat::RGBA10; break;
   #ifdef __OPENGLES__
   case SDL_PIXELFORMAT_ABGR8888: back_buffer_format = colorFormat::RGBA8; break;
   case SDL_PIXELFORMAT_RGBX8888: back_buffer_format = colorFormat::RGBA8; break;
   case SDL_PIXELFORMAT_RGBA8888: back_buffer_format = colorFormat::RGBA8; break;
   #endif
   default:
   {
      ShowError("Invalid Output format: "s.append(std::to_string(mode->format)));
      exit(-1);
   }
   }

   memset(m_samplerStateCache, 0, sizeof(m_samplerStateCache));

   #if defined(__OPENGLES__) || (defined(__APPLE__) && (defined(TARGET_OS_IOS) && TARGET_OS_IOS))
   m_useLowPrecision = true;
   #else
   m_useLowPrecision = false;
   #endif

   // FIXME We only set bit depth for fullscreen desktop modes (otherwise, use the desktop bit depth)
   int channelDepth = m_outputWnd[0]->GetBitDepth() == 32 ?  8 :
                      m_outputWnd[0]->GetBitDepth() == 30 ? 10 :
                                                             5;
   if (m_outputWnd[0]->IsFullScreen())
   {
      SDL_GL_SetAttribute(SDL_GL_RED_SIZE, channelDepth);
      SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, channelDepth);
      SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, channelDepth);
      SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
      SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
   }

   // Multisampling is performed on the offscreen buffers, not the window framebuffer
   SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
   SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);

   #ifndef __OPENGLES__
      #if defined(__APPLE__) && defined(TARGET_OS_MAC)
         SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
         SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
         SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
         SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
      #else
         SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
         //This would enforce a 4.1 context, disabling all recent features (storage buffers, debug information,...)
         //SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
         //SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
      #endif
   #else
      SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
      SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
      SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
   #endif

   SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

   m_sdl_context = SDL_GL_CreateContext(m_outputWnd[0]->GetCore());

   SDL_GL_MakeCurrent(m_outputWnd[0]->GetCore(), m_sdl_context);

   #if defined(ENABLE_SDL_VIDEO) && defined(ENABLE_OPENGL)
   int drawableWidth, drawableHeight, windowWidth, windowHeight;
   SDL_GetWindowSizeInPixels(m_outputWnd[0]->GetCore(), &drawableWidth, &drawableHeight); // Size in pixels
   SDL_GetWindowSize(m_outputWnd[0]->GetCore(), &windowWidth, &windowHeight); // Size in screen coordinates (taking in account HiDPI)
   PLOGI << "SDL drawable size: " << drawableWidth << 'x' << drawableHeight << " (window size: " << windowWidth << 'x' << windowHeight << ")";
   #endif

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
   if (gl_majorVersion < 4 || (gl_majorVersion == 4 && gl_minorVersion < 3))
   {
      char errorMsg[256];
      sprintf_s(errorMsg, sizeof(errorMsg), "Your graphics card only supports OpenGL %d.%d, but VPX requires OpenGL 4.3 or newer.", gl_majorVersion, gl_minorVersion);
      ShowError(errorMsg);
      exit(-1);
   }
   #endif

   m_GLversion = gl_majorVersion * 100 + gl_minorVersion;

   // Enable debugging layer of OpenGL
   #if defined(_DEBUG) && !defined(__OPENGLES__)
   glEnable(GL_DEBUG_OUTPUT); // on its own is the 'fast' version
   //glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // callback is in sync with errors, so a breakpoint can be placed on the callback in order to get a stacktrace for the GL error
   if (glad_glDebugMessageCallback)
   {
      glDebugMessageCallback(GLDebugMessageCallback, nullptr);
   }
   #endif
   #if 0
   glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_FALSE); // disable all
   glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_ERROR, GL_DONT_CARE, 0, nullptr, GL_TRUE); // enable only errors
   #endif

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

   SetRenderState(RenderState::ZFUNC, RenderState::Z_LESSEQUAL);

   // Retrieve a reference to the back buffer.
   wnd->SetBackBuffer(new RenderTarget(this, SurfaceType::RT_DEFAULT, wnd->GetWidth(), wnd->GetHeight(), back_buffer_format));

#elif defined(ENABLE_DX9)
    ///////////////////////////////////
    // DirectX 9 device initialization

    m_pD3DEx = nullptr;
    m_pD3DDeviceEx = nullptr;

    m_useLowPrecision = false;

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
    {
        m_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
        if (m_pD3D == nullptr)
        {
            ShowError("Could not create D3D9 object.");
            throw 0;
        }
    }

   D3DDEVTYPE devtype = D3DDEVTYPE_HAL;
   vector<VPX::Window::DisplayConfig> displays;
   VPX::Window::GetDisplays(displays);
   for (const VPX::Window::DisplayConfig& disp : displays)
   {
      if (disp.adapter == m_outputWnd[0]->GetAdapterId() && strstr(disp.GPU_Name, "PerfHUD") != 0)
      {
         devtype = D3DDEVTYPE_REF;
         break;
      }
   }

   D3DCAPS9 caps;
   m_pD3D->GetDeviceCaps(m_outputWnd[0]->GetAdapterId(), devtype, &caps);

    // check which parameters can be used for anisotropic filter
    m_mag_aniso = (caps.TextureFilterCaps & D3DPTFILTERCAPS_MAGFANISOTROPIC) != 0;
    m_maxaniso = caps.MaxAnisotropy;
    memset(m_bound_filter, 0xCC, TEXTURESET_STATE_CACHE_SIZE * sizeof(SamplerFilter));
    memset(m_bound_clampu, 0xCC, TEXTURESET_STATE_CACHE_SIZE * sizeof(SamplerAddressMode));
    memset(m_bound_clampv, 0xCC, TEXTURESET_STATE_CACHE_SIZE * sizeof(SamplerAddressMode));

    if (((caps.TextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL) != 0) || ((caps.TextureCaps & D3DPTEXTURECAPS_POW2) != 0))
        ShowError("D3D device does only support power of 2 textures");

    // get the current display format
    D3DFORMAT format;
    if (!m_outputWnd[0]->IsFullScreen())
    {
        D3DDISPLAYMODE mode;
        CHECKD3D(m_pD3D->GetAdapterDisplayMode(m_outputWnd[0]->GetAdapterId(), &mode));
        format = mode.Format;
    }
    else
    {
        format = m_outputWnd[0]->GetBitDepth() == 32 ? D3DFMT_X8R8G8B8 :
                 m_outputWnd[0]->GetBitDepth() == 30 ? D3DFMT_A2R10G10B10 :
                                                       D3DFMT_R5G6B5;
    }
    colorFormat back_buffer_format;
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
    params.BackBufferWidth = wnd->GetWidth();
    params.BackBufferHeight = wnd->GetHeight();
    params.BackBufferFormat = format;
    params.BackBufferCount = 1;
    params.MultiSampleType = D3DMULTISAMPLE_NONE;
    params.MultiSampleQuality = 0;
    params.SwapEffect = D3DSWAPEFFECT_DISCARD;
    params.hDeviceWindow = m_outputWnd[0]->GetCore();
    params.Windowed = !m_outputWnd[0]->IsFullScreen();
    params.EnableAutoDepthStencil = FALSE;
    params.AutoDepthStencilFormat = D3DFMT_UNKNOWN; // ignored
    params.Flags = /*fullscreen ? D3DPRESENTFLAG_LOCKABLE_BACKBUFFER :*/ /*(stereo3D ?*/ 0 /*: D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL)*/
       ; // D3DPRESENTFLAG_LOCKABLE_BACKBUFFER only needed for SetDialogBoxMode() below, but makes rendering slower on some systems :/
    params.FullScreen_RefreshRateInHz = m_outputWnd[0]->IsFullScreen() ? (UINT)m_outputWnd[0]->GetRefreshRate() : 0;
    params.PresentationInterval = syncMode == VideoSyncMode::VSM_VSYNC ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;

   // check if our HDR texture format supports/does sRGB conversion on texture reads, which must NOT be the case as we always set SRGBTexture=true independent of the format!
   HRESULT hr = m_pD3D->CheckDeviceFormat(m_outputWnd[0]->GetAdapterId(), devtype, params.BackBufferFormat, D3DUSAGE_QUERY_SRGBREAD, D3DRTYPE_TEXTURE, (D3DFORMAT)colorFormat::RGBA32F);
   if (SUCCEEDED(hr))
      ShowError("D3D device does support D3DFMT_A32B32G32R32F SRGBTexture reads (which leads to wrong tex colors)");
   // now the same for our LDR/8bit texture format the other way round
   hr = m_pD3D->CheckDeviceFormat(m_outputWnd[0]->GetAdapterId(), devtype, params.BackBufferFormat, D3DUSAGE_QUERY_SRGBREAD, D3DRTYPE_TEXTURE, (D3DFORMAT)colorFormat::RGBA8);
   if (!SUCCEEDED(hr))
      ShowError("D3D device does not support D3DFMT_A8R8G8B8 SRGBTexture reads (which leads to wrong tex colors)");

   // check if auto generation of mipmaps can be used, otherwise will be done via d3dx
   m_autogen_mipmap = (caps.Caps2 & D3DCAPS2_CANAUTOGENMIPMAP) != 0;
   if (m_autogen_mipmap)
      m_autogen_mipmap = (m_pD3D->CheckDeviceFormat(m_outputWnd[0]->GetAdapterId(), devtype, params.BackBufferFormat, textureUsage::AUTOMIPMAP, D3DRTYPE_TEXTURE, (D3DFORMAT)colorFormat::RGBA8) == D3D_OK);

   //m_autogen_mipmap = false; //!! could be done to support correct sRGB/gamma correct generation of mipmaps which is not possible with auto gen mipmap in DX9! at the moment disabled, as the sRGB software path is super slow for similar mipmap filter quality

   #ifndef DISABLE_FORCE_NVIDIA_OPTIMUS
   if (!NVAPIinit && NvAPI_Initialize() == NVAPI_OK)
      NVAPIinit = true;
   #endif

   // Determine if INTZ is supported
   m_INTZ_support = (m_pD3D->CheckDeviceFormat(m_outputWnd[0]->GetAdapterId(), devtype, params.BackBufferFormat,
                     D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_TEXTURE, ((D3DFORMAT)(MAKEFOURCC('I','N','T','Z'))))) == D3D_OK;

   // check if requested MSAA is possible
   DWORD MultiSampleQualityLevels;
   if (!SUCCEEDED(m_pD3D->CheckDeviceMultiSampleType(m_outputWnd[0]->GetAdapterId(),
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
      if (m_outputWnd[0]->IsFullScreen())
      {
         mode.Format = params.BackBufferFormat;
         mode.Width = params.BackBufferWidth;
         mode.Height = params.BackBufferHeight;
         mode.RefreshRate = params.FullScreen_RefreshRateInHz;
         mode.ScanLineOrdering = D3DSCANLINEORDERING_PROGRESSIVE;
      }

      hr = m_pD3DEx->CreateDeviceEx(
         m_outputWnd[0]->GetAdapterId(),
         devtype,
         m_outputWnd[0]->GetCore(),
         flags /*| D3DCREATE_PUREDEVICE*/,
         &params,
         m_outputWnd[0]->IsFullScreen() ? &mode : nullptr,
         &m_pD3DDeviceEx);
      if (FAILED(hr))
      {
         if (m_outputWnd[0]->IsFullScreen())
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
      // Not done anymore as the refresh rate is validated before creation
      // CHECKD3D(m_pD3DDeviceEx->GetDisplayModeEx(0, &mode, nullptr)); //!! what is the actual correct value for the swapchain here?
      // refreshrate = mode.RefreshRate;
   }
   else
   {
      hr = m_pD3D->CreateDevice(
         m_outputWnd[0]->GetAdapterId(),
         devtype,
         m_outputWnd[0]->GetCore(),
         flags /*| D3DCREATE_PUREDEVICE*/,
         &params,
         &m_pD3DDevice);

      if (FAILED(hr))
         ReportError("Fatal Error: unable to create D3D device!", hr, __FILE__, __LINE__);

      // Get the display mode so that we can report back the actual refresh rate.
      // Not done anymore as the refresh rate is validated before creation
      // D3DDISPLAYMODE mode;
      // CHECKD3D(m_pD3DDevice->GetDisplayMode(m_outputWnd[0]->GetAdapterId(), &mode));
      // refreshrate = mode.RefreshRate;
   }

   const int EnableLegacyMaximumPreRenderedFrames = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "EnableLegacyMaximumPreRenderedFrames"s, 0);
   if (!EnableLegacyMaximumPreRenderedFrames && m_pD3DEx && maxPrerenderedFrames > 0 && maxPrerenderedFrames <= 20)
   {
      CHECKD3D(m_pD3DDeviceEx->SetMaximumFrameLatency(maxPrerenderedFrames));
   }

   // Retrieve a reference to the back buffer.
   wnd->SetBackBuffer(new RenderTarget(this, SurfaceType::RT_DEFAULT, wnd->GetWidth(), wnd->GetHeight(), back_buffer_format));

   /*if (m_outputWnd[0]->IsFullScreen())
       hr = m_pD3DDevice->SetDialogBoxMode(TRUE);*/ // needs D3DPRESENTFLAG_LOCKABLE_BACKBUFFER, but makes rendering slower on some systems :/
#endif

   // Create default texture
   BaseTexture* surf = new BaseTexture(1, 1, BaseTexture::Format::RGBA);
   memset(surf->data(), 0, 4);
   m_nullTexture = new Sampler(this, surf, false);
   m_nullTexture->SetName("Null"s);
   delete surf;

   // alloc float buffer for rendering
   #if defined(ENABLE_OPENGL)
   int maxSamples;
   glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
   nMSAASamples = min(maxSamples, nMSAASamples);
   #endif

   // create default vertex declarations for shaders
   #if defined(ENABLE_BGFX)
   m_pVertexTexelDeclaration = new bgfx::VertexLayout; // TODO remove Pos/TexCoord format and only use one Pos/Normal/TexCoord
   m_pVertexTexelDeclaration->begin()
      .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
      .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
      .end();
   m_pVertexNormalTexelDeclaration = new bgfx::VertexLayout;
   m_pVertexNormalTexelDeclaration->begin()
      .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
      .add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)
      .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
      .end();
   #elif defined(ENABLE_DX9)
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
   #if defined(ENABLE_BGFX)
   static constexpr float reversedVerts[4 * 5] =
   {
       1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
      -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
       1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
      -1.0f, -1.0f, 0.0f, 0.0f, 0.0f
   };
   VertexBuffer* quadVertexBuffer = new VertexBuffer(this, 4, bgfx::getCaps()->originBottomLeft ? reversedVerts : verts, false, VertexFormat::VF_POS_TEX);
   #else
   VertexBuffer* quadVertexBuffer = new VertexBuffer(this, 4, verts, false, VertexFormat::VF_POS_TEX);
   #endif
   m_quadMeshBuffer = new MeshBuffer(L"Fullscreen Quad"s, quadVertexBuffer);

   #if defined(ENABLE_OPENGL)
   VertexBuffer* quadPNTDynVertexBuffer = new VertexBuffer(this, 4, nullptr, true, VertexFormat::VF_POS_NORMAL_TEX);
   m_quadPNTDynMeshBuffer = new MeshBuffer(quadPNTDynVertexBuffer);

   VertexBuffer* quadPTDynVertexBuffer = new VertexBuffer(this, 4, nullptr, true, VertexFormat::VF_POS_TEX);
   m_quadPTDynMeshBuffer = new MeshBuffer(quadPTDynVertexBuffer);
   #endif

   // Force applying a defined initial render state
   m_current_renderstate.m_state = (~m_renderstate.m_state) & ((1 << 21) - 1);
   m_current_renderstate.m_depthBias = m_renderstate.m_depthBias - 1.0f;
   ApplyRenderStates();
   
   // Ensure we have a VSync source for frame pacing
   bool hasVSync = false;
   if (m_dwm_enabled && mDwmFlush)
   {
      PLOGI << "VSync source set to Windows Desktop compositor (DwmFlush)";
      hasVSync = true;
   }
   #if defined(ENABLE_DX9)
      else if (m_pD3DDeviceEx != nullptr)
      {
         PLOGI << "VSync source set to DX9Ex WaitForBlank";
         hasVSync = true;
      }
   #elif defined(ENABLE_SDL_VIDEO) && defined(ENABLE_OPENGL) && !defined(__STANDALONE__)
      // DXGI VSync source (Windows 7+, only used for Win32 SDL with OpenGL)
      else if (syncMode == VideoSyncMode::VSM_FRAME_PACING)
      {
         DXGIRegistry::Output* out = g_DXGIRegistry.GetForWindow(m_outputWnd[0]->GetNativeHWND());
         if (out != nullptr)
            m_DXGIOutput = out->m_Output;
         if (m_DXGIOutput != nullptr)
         {
            PLOGI << "VSync source set to DXGI WaitForBlank";
            hasVSync = true;
         }
      }
   #endif
   
   if (syncMode == VideoSyncMode::VSM_FRAME_PACING && !hasVSync)
   {
      // This may happen on some old config where D3D9ex is not available (XP/Vista/7) and DWM is disabled
      ShowError("Failed to create the synchronization device.\r\nSynchronization switched to adaptive sync.");
      PLOGE << "Failed to create the synchronization device for frame pacing. Synchronization switched to adaptive sync.";
      syncMode = VideoSyncMode::VSM_ADAPTIVE_VSYNC;
      #if defined(ENABLE_OPENGL)
      SDL_GL_SetSwapInterval(-1);
      #endif
   }

   m_basicShader = new Shader(this, Shader::BASIC_SHADER, m_nEyes == 2);
   m_ballShader = new Shader(this, Shader::BALL_SHADER, m_nEyes == 2);
   m_DMDShader = new Shader(this, m_isVR ? Shader::DMD_VR_SHADER : Shader::DMD_SHADER, m_nEyes == 2);
   m_flasherShader = new Shader(this, Shader::FLASHER_SHADER, m_nEyes == 2);
   m_lightShader = new Shader(this, Shader::LIGHT_SHADER, m_nEyes == 2);
   m_stereoShader = new Shader(this, Shader::STEREO_SHADER, m_nEyes == 2);
   m_FBShader = new Shader(this, Shader::POSTPROCESS_SHADER, m_nEyes == 2);

   if (m_basicShader->HasError() || m_DMDShader->HasError() || m_FBShader->HasError() || m_flasherShader->HasError() || m_lightShader->HasError() || m_stereoShader->HasError())
   {
      ReportError("Fatal Error: shader compilation failed!", -1, __FILE__, __LINE__);
      throw(-1);
   }

   // Initialize uniform to default value
   m_basicShader->SetVector(SHADER_staticColor_Alpha, 1.0f, 1.0f, 1.0f, 1.0f); // No tinting
   // FIXME XR
   #ifndef ENABLE_XR
   m_DMDShader->SetFloat(SHADER_alphaTestValue, 1.0f); // No alpha clipping
   #endif

   #if !defined(__OPENGLES__)
      // Always load the (small) SMAA textures since SMAA can be toggled at runtime through the live UI
      UploadAndSetSMAATextures();
   #endif
}

RenderDevice::~RenderDevice()
{
   #if defined(ENABLE_BGFX)
      // SUspend rendering before deleting anything that could be used
      m_renderDeviceAlive = false;
      m_frameReadySem.post();
   #endif

   delete m_quadMeshBuffer;
   delete m_nullTexture;

   #if defined(ENABLE_DX9)
      m_pD3DDevice->SetStreamSource(0, nullptr, 0, 0);
      m_pD3DDevice->SetIndices(nullptr);
      m_pD3DDevice->SetVertexShader(nullptr);
      m_pD3DDevice->SetPixelShader(nullptr);
      m_pD3DDevice->SetFVF(D3DFVF_XYZ);
      m_pD3DDevice->SetDepthStencilSurface(nullptr);
      SAFE_RELEASE(m_pVertexTexelDeclaration);
      SAFE_RELEASE(m_pVertexNormalTexelDeclaration);
   #endif

   UnbindSampler(nullptr);
   delete m_basicShader;
   m_basicShader = nullptr;
   delete m_DMDShader;
   m_DMDShader = nullptr;
   delete m_FBShader;
   m_FBShader = nullptr;
   delete m_stereoShader;
   m_stereoShader = nullptr;
   delete m_flasherShader;
   m_flasherShader = nullptr;
   delete m_lightShader;
   m_lightShader = nullptr;
   delete m_ballShader;
   m_ballShader = nullptr;

   m_texMan.UnloadAll();

   for (unsigned int i = 0; i < m_nOutputWnd; i++)
   {
      delete m_outputWnd[i]->GetBackBuffer();
      m_outputWnd[i]->SetBackBuffer(nullptr);
   }

   delete m_SMAAareaTexture;
   delete m_SMAAsearchTexture;

#if defined(ENABLE_BGFX)
   delete m_pVertexTexelDeclaration;
   delete m_pVertexNormalTexelDeclaration;

   // Shutdown BGFX once all native resources have been cleaned up
   m_frameReadySem.post();
   if (m_renderThread.joinable())
      m_renderThread.join();

#elif defined(ENABLE_OPENGL)
   for (auto binding : m_samplerBindings)
      delete binding;
   m_samplerBindings.clear();

   for (size_t i = 0; i < std::size(m_samplerStateCache); i++)
   {
      if (m_samplerStateCache[i] != 0)
      {
         glDeleteSamplers(1, &m_samplerStateCache[i]);
         m_samplerStateCache[i] = 0;
      }
   }

   delete m_quadPTDynMeshBuffer;
   delete m_quadPNTDynMeshBuffer;

   SDL_GL_DestroyContext(m_sdl_context);

   assert(m_sharedVAOs.empty());

#elif defined(ENABLE_DX9)
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

   //!! if (m_pD3DDeviceEx == m_pD3DDevice) m_pD3DDevice = nullptr; //!! needed for Caligula if m_outputWnd[0]->GetAdapterId() > 0 ?? weird!! BUT MESSES UP FULLSCREEN EXIT (=hangs)
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
#endif

   assert(m_pendingSharedIndexBuffers.empty());
   assert(m_pendingSharedVertexBuffers.empty());
}

void RenderDevice::AddWindow(VPX::Window* wnd)
{
   assert(wnd->GetBackBuffer() == nullptr);
   if (m_nOutputWnd >= 8)
      return;

#if defined(ENABLE_BGFX) && (ENABLE_SDL_VIDEO)
   if ((bgfx::getCaps()->supported & BGFX_CAPS_SWAP_CHAIN) == 0)
      return;

   colorFormat fmt;
   switch (wnd->GetBitDepth())
   {
   case 32: fmt = colorFormat::RGBA8; break;
   case 30: fmt = colorFormat::RGBA10; break;
   default: fmt = colorFormat::RGB5; break;
   }
   SDL_Window* sdlWnd = wnd->GetCore();
   void* nwh;
#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
   void* ndt;
   if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "x11") == 0) {
      ndt = SDL_GetPointerProperty(SDL_GetWindowProperties(sdlWnd), SDL_PROP_WINDOW_X11_DISPLAY_POINTER, NULL);
      nwh = (void*)SDL_GetNumberProperty(SDL_GetWindowProperties(sdlWnd), SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0);
   }
   else if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "wayland") == 0) {
      ndt = SDL_GetPointerProperty(SDL_GetWindowProperties(sdlWnd), SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, NULL);
      nwh = SDL_GetPointerProperty(SDL_GetWindowProperties(sdlWnd), SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, NULL);
   }
#elif BX_PLATFORM_OSX
   nwh = SDL_GetRenderMetalLayer(SDL_CreateRenderer(sdlWnd, "Metal"));
#elif BX_PLATFORM_IOS
   nwh = SDL_GetRenderMetalLayer(SDL_CreateRenderer(sdlWnd, "Metal"));
#elif BX_PLATFORM_ANDROID
   nwh = SDL_GetPointerProperty(SDL_GetWindowProperties(sdlWnd), SDL_PROP_WINDOW_ANDROID_WINDOW_POINTER, NULL);
#elif BX_PLATFORM_WINDOWS
   nwh = wnd->GetNativeHWND();
#elif BX_PLATFORM_STEAMLINK
   nwh = wmInfo.info.vivante.window;
#else
   return nullptr;
#endif // BX_PLATFORM_
   bgfx::FrameBufferHandle fbh = bgfx::createFrameBuffer(nwh, uint16_t(wnd->GetWidth()), uint16_t(wnd->GetHeight()));
   m_outputWnd[m_nOutputWnd] = wnd;
   m_nOutputWnd++;
   wnd->SetBackBuffer(new RenderTarget(this, SurfaceType::RT_DEFAULT, fbh, BGFX_INVALID_HANDLE, BGFX_INVALID_HANDLE, "BackBuffer #" + std::to_string(m_nOutputWnd), wnd->GetWidth(), wnd->GetHeight(), fmt));
#endif
}

bool RenderDevice::DepthBufferReadBackAvailable()
{
#if defined(ENABLE_OPENGL) || defined(ENABLE_BGFX)
   return true;
#elif defined(ENABLE_DX9)
   if (m_INTZ_support && !m_useNvidiaApi)
      return true;
   // fall back to NVIDIAs NVAPI, only handle DepthBuffer ReadBack if API was initialized
   return NVAPIinit;
#endif
}

void RenderDevice::UnbindSampler(Sampler* sampler)
{
   if (m_basicShader)
      m_basicShader->UnbindSampler(sampler);
   if (m_DMDShader)
      m_DMDShader->UnbindSampler(sampler);
   if (m_FBShader)
      m_FBShader->UnbindSampler(sampler);
   if (m_flasherShader)
      m_flasherShader->UnbindSampler(sampler);
   if (m_lightShader)
      m_lightShader->UnbindSampler(sampler);
   if (m_stereoShader)
      m_stereoShader->UnbindSampler(sampler);
   if (m_ballShader)
      m_ballShader->UnbindSampler(sampler);
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
      #if defined(ENABLE_OPENGL)
      else if (m_DXGIOutput != nullptr)
         m_DXGIOutput->WaitForVBlank();
      #elif defined(ENABLE_DX9)
      // When DWM is disabled (Windows Vista/7), exclusive fullscreen without DWM (pre-windows 10), special Windows builds with DWM stripped out (Ghost Spectre Windows 10)
      else if (m_pD3DDeviceEx != nullptr)
         m_pD3DDeviceEx->WaitForVBlank(0);
      #endif
#endif
      m_vsyncCount++;
      //const U64 now = usec();
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

   // Schedule frame presentation (non blocking call, simply queueing the present command in the driver's render queue with a schedule for execution)
   #if defined(ENABLE_BGFX)
   // Process pending texture upload/mipmap generation before flipping the frame
   for (auto it = m_pendingTextureUploads.cbegin(); it != m_pendingTextureUploads.cend();)
   {
      (*it)->GetCoreTexture();
      if ((*it)->IsMipMapGenerated())
      {
         it = m_pendingTextureUploads.erase(it);
      }
      else
      {
         ++it;
      }
   }
   SubmitAndFlipFrame();

   #elif defined(ENABLE_OPENGL)
   SDL_GL_SwapWindow(m_outputWnd[0]->GetCore());
   if (!m_isVR)
      g_pplayer->m_logicProfiler.OnPresented(usec());

   #elif defined(ENABLE_DX9)
   CHECKD3D(m_pD3DDevice->Present(nullptr, nullptr, nullptr, nullptr));
   if (!m_isVR)
      g_pplayer->m_logicProfiler.OnPresented(usec());
   #endif
}

void RenderDevice::UploadAndSetSMAATextures()
{
   // TODO use standard BaseTexture / Sampler code instead
   /* BaseTexture* searchBaseTex = new BaseTexture(SEARCHTEX_WIDTH, SEARCHTEX_HEIGHT, BaseTexture::BW);
   memcpy(searchBaseTex->data(), searchTexBytes, SEARCHTEX_SIZE);
   m_SMAAsearchTexture = new Sampler(this, searchBaseTex, true, SamplerAddressMode::SA_CLAMP, SamplerAddressMode::SA_CLAMP, SamplerFilter::SF_NONE);
   m_SMAAsearchTexture->SetName("SMAA Search"s);
   delete searchBaseTex;*/

#if defined(ENABLE_BGFX)
   bgfx::TextureHandle smaaAreaTex = bgfx::createTexture2D(AREATEX_WIDTH, AREATEX_HEIGHT, false, 1, bgfx::TextureFormat::RG8, BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP, bgfx::makeRef(areaTexBytes, AREATEX_SIZE));
   m_SMAAareaTexture = new Sampler(this, SurfaceType::RT_DEFAULT, smaaAreaTex, AREATEX_WIDTH, AREATEX_HEIGHT, true, true, SamplerAddressMode::SA_CLAMP, SamplerAddressMode::SA_CLAMP, SamplerFilter::SF_BILINEAR);
   m_SMAAareaTexture->SetName("SMAA Area"s);

   bgfx::TextureHandle smaaSearchTex = bgfx::createTexture2D(SEARCHTEX_WIDTH, SEARCHTEX_HEIGHT, false, 1, bgfx::TextureFormat::R8, BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP, bgfx::makeRef(searchTexBytes, SEARCHTEX_SIZE));
   m_SMAAsearchTexture = new Sampler(this, SurfaceType::RT_DEFAULT, smaaSearchTex, SEARCHTEX_WIDTH, SEARCHTEX_HEIGHT, true, true, SamplerAddressMode::SA_CLAMP, SamplerAddressMode::SA_CLAMP, SamplerFilter::SF_NONE);
   m_SMAAsearchTexture->SetName("SMAA Search"s);

#elif defined(ENABLE_OPENGL)
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

#elif defined(ENABLE_DX9)
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

   m_FBShader->SetTexture(SHADER_areaTex, m_SMAAareaTexture);
   m_FBShader->SetTexture(SHADER_searchTex, m_SMAAsearchTexture);
}

void RenderDevice::UploadTexture(BaseTexture* texture, const bool linearRGB)
{
   Sampler* sampler = m_texMan.LoadTexture(texture, SamplerFilter::SF_UNDEFINED, SamplerAddressMode::SA_UNDEFINED, SamplerAddressMode::SA_UNDEFINED, linearRGB);
   #if defined(ENABLE_BGFX)
   // BGFX dispatch operations to the render thread, so the texture manager does not actually loads data to the GPU nor perform mipmap generation
   m_pendingTextureUploads.push_back(sampler);
   #endif
}

void RenderDevice::SetSamplerState(int unit, SamplerFilter filter, SamplerAddressMode clamp_u, SamplerAddressMode clamp_v)
{
#if defined(ENABLE_BGFX)
#elif defined(ENABLE_OPENGL)
   assert(std::size(m_samplerStateCache) == 3*3*5);
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
#elif defined(ENABLE_DX9)
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
   m_basicShader->m_state->CopyTo(copyTo, state.m_basicShaderState);
   m_DMDShader->m_state->CopyTo(copyTo, state.m_DMDShaderState);
   m_FBShader->m_state->CopyTo(copyTo, state.m_FBShaderState);
   m_flasherShader->m_state->CopyTo(copyTo, state.m_flasherShaderState);
   m_lightShader->m_state->CopyTo(copyTo, state.m_lightShaderState);
   m_ballShader->m_state->CopyTo(copyTo, state.m_ballShaderState);
   m_stereoShader->m_state->CopyTo(copyTo, state.m_stereoShaderState);
}

void RenderDevice::SetClipPlane(const vec4 &plane)
{
#if defined(__OPENGLES__)
   // FIXME GLES implement (or use BGFX OpenGL ES implementation)
   return;
#elif defined(ENABLE_BGFX)
   m_DMDShader->SetVector(SHADER_clip_plane, &plane);
   m_basicShader->SetVector(SHADER_clip_plane, &plane);
   m_lightShader->SetVector(SHADER_clip_plane, &plane);
   m_flasherShader->SetVector(SHADER_clip_plane, &plane);
   m_ballShader->SetVector(SHADER_clip_plane, &plane);
#elif defined(ENABLE_OPENGL)
   m_DMDShader->SetVector(SHADER_clip_plane, &plane);
   m_basicShader->SetVector(SHADER_clip_plane, &plane);
   m_lightShader->SetVector(SHADER_clip_plane, &plane);
   m_flasherShader->SetVector(SHADER_clip_plane, &plane);
   m_ballShader->SetVector(SHADER_clip_plane, &plane);
#elif defined(ENABLE_DX9)
   // FIXME DX9 shouldn't we set the Model matrix to identity first ?
   Matrix3D mT = g_pplayer->m_renderer->GetMVP().GetModelViewProj(0); // = world * view * proj
   mT.Invert();
   mT.Transpose();
   const D3DXMATRIX m(mT);
   D3DXPLANE clipSpacePlane;
   const D3DXPLANE dxplane(-plane.x, -plane.y, -plane.z, -plane.w);
   D3DXPlaneTransform(&clipSpacePlane, &dxplane, &m);
   GetCoreDevice()->SetClipPlane(0, clipSpacePlane);
#endif
}

void RenderDevice::SubmitRenderFrame()
{
   #ifdef ENABLE_BGFX
   if (std::this_thread::get_id() != m_renderThread.get_id())
   {
      // post semaphore and wait for render thread to process frame
      m_framePending = true;
      m_frameNoSync = true;
      m_frameMutex.unlock(); // release the lock and wait for render thread to process the frame
      m_frameReadySem.post();
      while (m_framePending)
         //YieldProcessor();
         Sleep(0);
      m_frameMutex.lock();
      return;
   }
   #endif

   m_currentPass = nullptr;
   bool rendered = m_renderFrame.Execute(m_logNextFrame);
   if (rendered)
      m_logNextFrame = false;
   m_lastPresentFrameTick = usec();
}

void RenderDevice::DiscardRenderFrame()
{
   m_currentPass = nullptr;
   m_renderFrame.Discard();
   #ifdef ENABLE_BGFX
      RenderTarget::OnFrameFlushed();
      m_activeViewId = -1;
   #endif
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
      m_currentPass->m_mergeable = !forceNewPass;
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

void RenderDevice::Clear(const DWORD flags, const DWORD color)
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

void RenderDevice::DrawTexturedQuad(Shader* shader, const Vertex3D_TexelOnly* vertices, const bool isTransparent, const float depth)
{
   assert(shader == m_FBShader || shader == m_stereoShader); // FrameBuffer/Stereo shader are the only ones using Position/Texture vertex format
   ApplyRenderStates();
   RenderCommand* cmd = m_renderFrame.NewCommand();
   cmd->SetDrawTexturedQuad(shader, vertices, isTransparent, depth);
   cmd->m_dependency = m_nextRenderCommandDependency;
   m_nextRenderCommandDependency = nullptr;
   m_currentPass->Submit(cmd);
}

void RenderDevice::DrawTexturedQuad(Shader* shader, const Vertex3D_NoTex2* vertices, const bool isTransparent, const float depth)
{
   assert(shader != m_FBShader && shader != m_stereoShader); // FrameBuffer/Stereo shader are the only ones using Position/Texture vertex format
   ApplyRenderStates();
   RenderCommand* cmd = m_renderFrame.NewCommand();
   cmd->SetDrawTexturedQuad(shader, vertices, isTransparent, depth);
   cmd->m_dependency = m_nextRenderCommandDependency;
   m_nextRenderCommandDependency = nullptr;
   m_currentPass->Submit(cmd);
}

void RenderDevice::DrawFullscreenTexturedQuad(Shader* shader)
{
   assert(shader == m_FBShader || shader == m_stereoShader); // FrameBuffer/Stereo shader are the only ones using Position/Texture vertex format
   static const Vertex3Ds pos(0.f, 0.f, 0.f);
   DrawMesh(shader, false, pos, 0.f, m_quadMeshBuffer, TRIANGLESTRIP, 0, 4);
}

void RenderDevice::DrawMesh(Shader* shader, const bool isTranparentPass, const Vertex3Ds& center, const float depthBias, MeshBuffer* mb, const PrimitiveTypes type, const DWORD startIndex, const DWORD indexCount)
{
   RenderCommand* cmd = m_renderFrame.NewCommand();
   // Legacy sorting order (only along negative z axis, which is reversed for reflections).
   // This is completely wrong but needed to preserve backward compatibility. We should sort along the view axis (especially for reflection probes)
   const float depth = g_pplayer->m_renderer && g_pplayer->m_renderer->IsRenderPass(Renderer::REFLECTION_PASS) ? depthBias + center.z : depthBias - center.z;
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

   RenderPass* const initial_rt = GetCurrentPass();
   RenderState initial_state;
   CopyRenderStates(true, initial_state);
   ResetRenderState();
   SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_FALSE);
   SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
   SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
   SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);
   {
      m_FBShader->SetTextureNull(SHADER_tex_fb_filtered);
      SetRenderTarget(initial_rt->m_name + " HBlur", tmp, false); // switch to temporary output buffer for horizontal phase of gaussian blur
      m_currentPass->m_singleLayerRendering = singleLayer; // We support bluring a single layer (for anaglyph defocusing)
      AddRenderTargetDependency(source);
      m_FBShader->SetTexture(SHADER_tex_fb_filtered, source->GetColorSampler());
      m_FBShader->SetVector(SHADER_w_h_height, (float)(1.0 / source->GetWidth()), (float)(1.0 / source->GetHeight()), 1.0f, 1.0f);
      m_FBShader->SetTechnique(tech_h);
      DrawFullscreenTexturedQuad(m_FBShader);
   }
   {
      m_FBShader->SetTextureNull(SHADER_tex_fb_filtered);
      SetRenderTarget(initial_rt->m_name + " VBlur", dest, false); // switch to output buffer for vertical phase of gaussian blur
      m_currentPass->m_singleLayerRendering = singleLayer; // We support bluring a single layer (for anaglyph defocusing)
      AddRenderTargetDependency(tmp);
      m_FBShader->SetTexture(SHADER_tex_fb_filtered, tmp->GetColorSampler());
      m_FBShader->SetVector(SHADER_w_h_height, (float)(1.0 / tmp->GetWidth()), (float)(1.0 / tmp->GetHeight()), 1.0f, 1.0f);
      m_FBShader->SetTechnique(tech_v);
      DrawFullscreenTexturedQuad(m_FBShader);
   }
   CopyRenderStates(false, initial_state);
   SetRenderTarget(initial_rt->m_name, initial_rt->m_rt, true);
   initial_rt->m_name += '-';
}

void RenderDevice::SetMainTextureDefaultFiltering(const SamplerFilter filter)
{
   Shader::SetDefaultSamplerFilter(SHADER_tex_sprite, filter);
   Shader::SetDefaultSamplerFilter(SHADER_tex_flasher_A, filter);
   Shader::SetDefaultSamplerFilter(SHADER_tex_flasher_B, filter);
   Shader::SetDefaultSamplerFilter(SHADER_tex_base_color, filter);
   Shader::SetDefaultSamplerFilter(SHADER_tex_base_normalmap, filter);
}
