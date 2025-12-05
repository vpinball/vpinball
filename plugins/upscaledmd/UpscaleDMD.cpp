// license:GPLv3+

#include "plugins/MsgPlugin.h"
#include "plugins/ControllerPlugin.h"
#include "plugins/LoggingPlugin.h"

#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>
#include <cstring>
#include <cstdint>
#include <sstream>
#include <cstdarg>
#if defined(__APPLE__) || defined(__linux__) || defined(__ANDROID__)
#include <pthread.h>
#endif
#include <cassert>

#include "xbrz/xbrz.h"

#include "super-xbr/super-xbr.h"

#include "scalefx/scalefx.h"

#include "plugins/ResURIResolver.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <locale>
#endif

#if defined(_M_IX86) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2) || defined(__SSE2__) || defined(_M_X64) || defined(_M_AMD64) || defined(__i386__) || defined(__i386) || defined(__i486__) || defined(__i486) || defined(i386) || defined(__ia64__) || defined(__x86_64__)
 #define ENABLE_SSE_OPTIMIZATIONS
 #include <xmmintrin.h>
#elif (defined(_M_ARM) || defined(_M_ARM64) || defined(__arm__) || defined(__arm64__) || defined(__aarch64__)) && (!defined(__ARM_ARCH) || __ARM_ARCH >= 7) && (!defined(_MSC_VER) || defined(__clang__)) //!! disable sse2neon if MSVC&non-clang
 #define ENABLE_SSE_OPTIMIZATIONS
 #include "sse2neon.h" // from https://github.com/DLTcollab/sse2neon
#endif

#ifdef __SSSE3__
 #include <tmmintrin.h>
#endif

///////////////////////////////////////////////////////////////////////////////
//
// UpscaleDMD plugin: upscale DMD content to a high res DMD or a LCD/CRT display
//
// This plugin only relies on the generic messaging plugin API and the generic
// controller display and segment API. It listens for DMD source and,
// when found, provides a corresponding display source.
//
// All rendering is done by an ancillary thread, causing a one frame delay, but
// avoiding CPU load on the main thread.

namespace UpscaleDMD
{

using namespace std::string_literals;

static const MsgPluginAPI* msgApi = nullptr;
static uint32_t endpointId;
static unsigned int onDisplaySrcChangedId, getDisplaySrcId;

static DisplaySrcId dmdSrc, displayId;

static std::unique_ptr<ResURIResolver> resURIResolver;
static std::mutex sourceMutex;
static std::thread renderThread;
static std::condition_variable updateCondVar;
static bool isRunning = false;

static std::vector<uint32_t> rgbaSrcFrame;
static std::vector<uint32_t> rgbaDstFrame;
static std::vector<uint8_t> rgbFrame;
static std::vector<float> lumFrame;
static unsigned int renderFrameId = 0;

enum UpscalerMode {
   UM_Disabled,
   UM_xBRZ_2x,
   UM_xBRZ_3x,
   UM_xBRZ_4x,
   UM_xBRZ_5x,
   UM_xBRZ_6x,
   UM_ScaleFX_AA,
   UM_ScaleFX_3x,
   UM_SuperXBR_2x
};
static const char* upscalerNames[] = { "Disabled", "xBRZ 2x", "xBRZ 3x", "xBRZ 4x", "xBRZ 5x", "xBRZ 6x", "ScaleFX AA 1x", "ScaleFX 3x", "Super-XBR 2x" };
static constexpr int scaleFactors[] = { 1, 2, 3, 4, 5, 6, 1, 3, 2 };
static UpscalerMode upscalerMode = UM_Disabled;
static UpscalerMode nextUpscalerMode = UM_Disabled;
static int GetUpscalerMode() { return (int)upscalerMode; }
static void OnDmdSrcChanged(const unsigned int, void*, void*);
static void SetUpscalerMode(int v)
{
   nextUpscalerMode = (UpscalerMode) v;
   OnDmdSrcChanged(endpointId, nullptr, nullptr);
}
MSGPI_ENUM_SETTING(upscaleModeProp, "UpscaleMode", "Mode", "Select upscaler", true, 0, std::size(scaleFactors), upscalerNames, 0, GetUpscalerMode, SetUpscalerMode);

LPI_USE();
LPI_IMPLEMENT // Implement shared log support


#ifdef _WIN32
static void SetThreadName(const std::string& name)
{
   const int size_needed = MultiByteToWideChar(CP_UTF8, 0, name.c_str(), -1, nullptr, 0);
   if (size_needed <= 1)
      return;
   std::wstring wstr(size_needed - 1, L'\0');
   if (MultiByteToWideChar(CP_UTF8, 0, name.c_str(), -1, wstr.data(), size_needed) == 0)
      return;
   SetThreadDescription(GetCurrentThread(), wstr.c_str());
}
#else
static void SetThreadName(const std::string& name)
{
#ifdef __APPLE__
   pthread_setname_np(name.c_str());
#elif defined(__linux__) || defined(__ANDROID__)
   pthread_setname_np(pthread_self(), name.c_str());
#endif
}
#endif

static bool IsSourceSelected() { return dmdSrc.GetRenderFrame != nullptr; }

static inline float linearToGamma(const float f) { return (f <= 0.0031308f) ? (12.92f * f) : (1.055f * powf(f, (float)(1.0 / 2.4)) - 0.055f); }
static inline int32_t linearToGammaU8(const float f) { return (int32_t)((f <= 0.0031308f) ? ((float)(255.*12.92) * f + 0.5f) : ((float)(255.*1.055) * powf(f, (float)(1.0 / 2.4)) - (float)(255.*0.055 - 0.5))); } // scaled and rounded to [0..255]

static inline float gammaToLinear(const float x) { return (x <= 0.04045f) ? (x * (float)(1.0 / 12.92)) : (powf(x * (float)(1.0 / 1.055) + (float)(0.055 / 1.055), 2.4f)); }
// computed using double arithmetic, printed at max precision
static constexpr float gammaToLinearLUT[256] = { 0.f, 0.000303527f, 0.000607054f, 0.000910581f, 0.001214108f, 0.001517635f, 0.001821162f, 0.0021246888f, 0.002428216f, 0.0027317428f, 0.00303527f,
   0.0033465358f, 0.0036765074f, 0.004024717f, 0.004391442f, 0.0047769533f, 0.0051815165f, 0.0056053917f, 0.006048833f, 0.0065120906f, 0.00699541f, 0.007499032f, 0.008023193f, 0.008568126f,
   0.009134059f, 0.009721218f, 0.010329823f, 0.010960094f, 0.011612245f, 0.012286488f, 0.0129830325f, 0.013702083f, 0.014443844f, 0.015208514f, 0.015996294f, 0.016807375f, 0.017641954f,
   0.01850022f, 0.019382361f, 0.020288562f, 0.02121901f, 0.022173885f, 0.023153367f, 0.024157632f, 0.02518686f, 0.026241222f, 0.027320892f, 0.02842604f, 0.029556835f, 0.030713445f,
   0.031896032f, 0.033104766f, 0.034339808f, 0.035601314f, 0.03688945f, 0.038204372f, 0.039546236f, 0.0409152f, 0.04231141f, 0.04373503f, 0.045186203f, 0.046665087f, 0.048171826f,
   0.049706567f, 0.051269457f, 0.052860647f, 0.054480277f, 0.05612849f, 0.05780543f, 0.059511237f, 0.061246052f, 0.063010015f, 0.064803265f, 0.06662594f, 0.06847817f, 0.070360094f,
   0.07227185f, 0.07421357f, 0.07618538f, 0.07818742f, 0.08021982f, 0.08228271f, 0.08437621f, 0.08650046f, 0.08865558f, 0.09084171f, 0.093058966f, 0.09530747f, 0.09758735f, 0.099898726f,
   0.10224173f, 0.104616486f, 0.107023105f, 0.10946171f, 0.11193243f, 0.114435375f, 0.116970666f, 0.11953843f, 0.122138776f, 0.12477182f, 0.12743768f, 0.13013647f, 0.13286832f, 0.13563333f,
   0.13843161f, 0.14126329f, 0.14412847f, 0.14702727f, 0.14995979f, 0.15292615f, 0.15592647f, 0.15896083f, 0.16202937f, 0.1651322f, 0.1682694f, 0.17144111f, 0.1746474f, 0.17788842f,
   0.18116425f, 0.18447499f, 0.18782078f, 0.19120169f, 0.19461784f, 0.19806932f, 0.20155625f, 0.20507874f, 0.20863687f, 0.21223076f, 0.2158605f, 0.2195262f, 0.22322796f, 0.22696587f,
   0.23074006f, 0.23455058f, 0.23839757f, 0.24228112f, 0.24620132f, 0.25015828f, 0.2541521f, 0.25818285f, 0.26225066f, 0.2663556f, 0.2704978f, 0.2746773f, 0.27889428f, 0.28314874f,
   0.28744084f, 0.29177064f, 0.29613826f, 0.30054379f, 0.3049873f, 0.30946892f, 0.31398872f, 0.31854677f, 0.3231432f, 0.3277781f, 0.33245152f, 0.33716363f, 0.34191442f, 0.34670407f,
   0.3515326f, 0.35640013f, 0.3613068f, 0.3662526f, 0.3712377f, 0.37626213f, 0.38132602f, 0.38642943f, 0.39157248f, 0.39675522f, 0.40197778f, 0.4072402f, 0.4125426f, 0.41788507f,
   0.42326766f, 0.4286905f, 0.43415365f, 0.43965718f, 0.4452012f, 0.4507858f, 0.45641103f, 0.462077f, 0.4677838f, 0.47353148f, 0.47932017f, 0.48514995f, 0.49102086f, 0.49693298f, 0.5028865f,
   0.50888133f, 0.5149177f, 0.52099556f, 0.5271151f, 0.5332764f, 0.5394795f, 0.54572445f, 0.55201143f, 0.5583404f, 0.5647115f, 0.57112485f, 0.57758045f, 0.58407843f, 0.59061885f,
   0.59720176f, 0.60382736f, 0.61049557f, 0.6172066f, 0.6239604f, 0.63075715f, 0.63759685f, 0.6444797f, 0.65140563f, 0.65837485f, 0.6653873f, 0.67244315f, 0.6795425f, 0.6866853f,
   0.69387174f, 0.7011019f, 0.70837575f, 0.7156935f, 0.7230551f, 0.73046076f, 0.7379104f, 0.7454042f, 0.7529422f, 0.7605245f, 0.76815116f, 0.7758222f, 0.7835378f, 0.7912979f, 0.7991027f,
   0.80695224f, 0.8148466f, 0.82278574f, 0.8307699f, 0.838799f, 0.8468732f, 0.8549926f, 0.8631572f, 0.8713671f, 0.8796224f, 0.8879231f, 0.8962694f, 0.9046612f, 0.91309863f, 0.92158186f,
   0.9301109f, 0.9386857f, 0.9473065f, 0.9559733f, 0.9646863f, 0.9734453f, 0.9822506f, 0.9911021f, 1.f };

static void RenderThread()
{
   SetThreadName("UpscaleDMD.RenderThread"s);

#ifdef ENABLE_SSE_OPTIMIZATIONS // actually uses SSSE3
#if !(defined(_M_ARM) || defined(_M_ARM64) || defined(__arm__) || defined(__arm64__) || defined(__aarch64__)) && defined(_MSC_VER)
   static int ssse3_supported = -1;
   if (ssse3_supported == -1)
   {
      int cpuInfo[4];
      __cpuid(cpuInfo, 1);
      ssse3_supported = (cpuInfo[2] & (1 << 9));
   }
#else
   constexpr bool ssse3_supported = true;
#endif
#endif

   while (isRunning)
   {
      std::unique_lock lock(sourceMutex);
      updateCondVar.wait(lock); // Wait for a frame to be requested, locking the source for the time of its processing

      if (!isRunning)
         break;

      if (!IsSourceSelected())
         continue;

      // Render display
      const DisplayFrame srcFrame = dmdSrc.GetRenderFrame(dmdSrc.id);

      // Monochrome frames are provided as linear luminance while upscalers expect sRGB data. If this is defined,
      // frames are converted from linear to sRGB for upscaling and back to linear luminance. This gives better
      // results at some performance cost. The difference is really visible when upscaling is null (ScaleFX AA)
      // or low. When the upscale factor goes up, this is less needed and the performance impact goes up.
      const bool applySRGB = scaleFactors[upscalerMode] <= 3;

      if (srcFrame.frame && srcFrame.frameId != renderFrameId)
      {
         const unsigned int dmdSrcSize = dmdSrc.width * dmdSrc.height;
         if (dmdSrc.frameFormat == CTLPI_DISPLAY_FORMAT_LUM32F)
         {
            // Lum32F in float -> sLum8 in uint32_t
            if (upscalerMode == UpscalerMode::UM_ScaleFX_AA || upscalerMode == UpscalerMode::UM_ScaleFX_3x)
            {
               for (unsigned int ofs = 0; ofs < dmdSrcSize; ++ofs)
                  rgbaSrcFrame[ofs] = linearToGammaU8(static_cast<const float*>(srcFrame.frame)[ofs]);
            }
            else
            {
               for (unsigned int ofs = 0; ofs < dmdSrcSize; ++ofs)
               {
                  const uint32_t lum = linearToGammaU8(static_cast<const float*>(srcFrame.frame)[ofs]);
                  rgbaSrcFrame[ofs] = lum | (lum << 8) | (lum << 16) | 0xFF000000u;
               }
            }
         }
         else if (dmdSrc.frameFormat == CTLPI_DISPLAY_FORMAT_SRGB888)
            for (unsigned int ofs = 0; ofs < dmdSrcSize; ++ofs)
            {
               const uint32_t r = static_cast<const uint8_t*>(srcFrame.frame)[ofs * 3 + 0];
               const uint32_t g = static_cast<const uint8_t*>(srcFrame.frame)[ofs * 3 + 1];
               const uint32_t b = static_cast<const uint8_t*>(srcFrame.frame)[ofs * 3 + 2];
               rgbaSrcFrame[ofs] = r | (g << 8) | (b << 16) | 0xFF000000u;
            }
         else if (dmdSrc.frameFormat == CTLPI_DISPLAY_FORMAT_SRGB565)
         {
            static constexpr uint8_t lum32[]
               = { 0, 8, 16, 25, 33, 41, 49, 58, 66, 74, 82, 90, 99, 107, 115, 123, 132, 140, 148, 156, 165, 173, 181, 189, 197, 206, 214, 222, 230, 239, 247, 255 };
            static constexpr uint8_t lum64[] = { 0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 45, 49, 53, 57, 61, 65, 69, 73, 77, 81, 85, 89, 93, 97, 101, 105, 109, 113, 117, 121, 125, 130, 134,
               138, 142, 146, 150, 154, 158, 162, 166, 170, 174, 178, 182, 186, 190, 194, 198, 202, 206, 210, 215, 219, 223, 227, 231, 235, 239, 243, 247, 251, 255 };
            const uint16_t* const __restrict frame = static_cast<const uint16_t*>(srcFrame.frame);
            for (unsigned int ofs = 0; ofs < dmdSrcSize; ++ofs)
            {
               const uint16_t rgb565 = frame[ofs];
               rgbaSrcFrame[ofs] = 0xFF000000u | ((uint32_t)lum32[rgb565 & 0x1F] << 16) | ((uint32_t)lum64[(rgb565 >> 5) & 0x3F] << 8) | (uint32_t)lum32[(rgb565 >> 11) & 0x1F];
            }
         }
         
         switch (upscalerMode)
         {
         case UpscalerMode::UM_Disabled:
            assert(false); // As this thread is supposed to be stopped when there is no upscaling to be done
            break;

         case UpscalerMode::UM_ScaleFX_AA:
            scalefx::upscale<false>(rgbaSrcFrame.data(), rgbaDstFrame.data(), dmdSrc.width, dmdSrc.height, displayId.frameFormat == CTLPI_DISPLAY_FORMAT_LUM32F);
            break;
            
         case UpscalerMode::UM_ScaleFX_3x:
            scalefx::upscale<true>(rgbaSrcFrame.data(), rgbaDstFrame.data(), dmdSrc.width, dmdSrc.height, displayId.frameFormat == CTLPI_DISPLAY_FORMAT_LUM32F);
            break;
            
         case UpscalerMode::UM_SuperXBR_2x:
            superxbr::scale<2, false>(rgbaSrcFrame.data(), rgbaDstFrame.data(), dmdSrc.width, dmdSrc.height);
            break;
            
         case UpscalerMode::UM_xBRZ_2x:
         case UpscalerMode::UM_xBRZ_3x:
         case UpscalerMode::UM_xBRZ_4x:
         case UpscalerMode::UM_xBRZ_5x:
         case UpscalerMode::UM_xBRZ_6x:
            xbrz::scale(scaleFactors[upscalerMode], rgbaSrcFrame.data(), rgbaDstFrame.data(), dmdSrc.width, dmdSrc.height, xbrz::ColorFormat::RGB);
            break;
         }

         const unsigned int displaySize = displayId.width * displayId.height;
         if (displayId.frameFormat == CTLPI_DISPLAY_FORMAT_LUM32F)
         { // sLum8 in uint32_t -> Lum32F in float
            for (unsigned int ofs = 0; ofs < displaySize; ++ofs)
               lumFrame[ofs] = gammaToLinearLUT[rgbaDstFrame[ofs] & 0xFFu];
         }
         else
         { // RGBx -> RGB
            unsigned int ofs = 0;
         #ifdef ENABLE_SSE_OPTIMIZATIONS // actually uses SSSE3
            if (ssse3_supported)
            {
            // Process 16 pixels at a time
            const unsigned int simdCount = displaySize & ~15u;
            const __m128i shuffle  = _mm_setr_epi8(0,1,2, 4,5,6, 8,9,10, 12,13,14, -1,-1,-1,-1);
            const __m128i shuffle2 = _mm_setr_epi8(-1,-1,-1,-1, 0,1,2, 4,5,6, 8,9,10, 12,13,14);
            for (; ofs < simdCount; ofs+=16)
            {
               const __m128i* const __restrict rdf = reinterpret_cast<const __m128i*>(&rgbaDstFrame[ofs]);
               const __m128i col[4] = {_mm_shuffle_epi8(_mm_loadu_si128(rdf  ), shuffle),
                                       _mm_shuffle_epi8(_mm_loadu_si128(rdf+1), shuffle),
                                       _mm_shuffle_epi8(_mm_loadu_si128(rdf+2), shuffle),
                                       _mm_shuffle_epi8(_mm_loadu_si128(rdf+3), shuffle2)};
               __m128i* const __restrict rf = reinterpret_cast<__m128i*>(&rgbFrame[ofs * 3]);
               _mm_storeu_si128(rf  , _mm_or_si128(col[0],_mm_slli_si128(col[1],12)));
               _mm_storeu_si128(rf+1, _mm_castps_si128(_mm_shuffle_ps(_mm_castsi128_ps(col[1]), _mm_castsi128_ps(col[2]), _MM_SHUFFLE(1,0,2,1))));
               _mm_storeu_si128(rf+2, _mm_or_si128(_mm_srli_si128(col[2],8),col[3]));
            }
            }
            // Handle remaining pixels
         #endif
            for (; ofs < displaySize; ++ofs)
            {
               const uint32_t col = rgbaDstFrame[ofs];
               rgbFrame[ofs * 3 + 0] =  col        & 0xFFu;
               rgbFrame[ofs * 3 + 1] = (col >>  8) & 0xFFu;
               rgbFrame[ofs * 3 + 2] = (col >> 16) & 0xFFu;
            }
         }
         renderFrameId = srcFrame.frameId;
      }
   }
   isRunning = false;
}

static DisplayFrame GetRenderFrame(const CtlResId id)
{
   updateCondVar.notify_all();
   return { renderFrameId, displayId.frameFormat == CTLPI_DISPLAY_FORMAT_LUM32F ? static_cast<void*>(lumFrame.data()) : static_cast<void*>(rgbFrame.data()) };
}

static void OnGetDisplaySrc(const unsigned int, void*, void* msgData)
{
   if (!IsSourceSelected())
      return;
   GetDisplaySrcMsg& msg = *static_cast<GetDisplaySrcMsg*>(msgData);
   if (msg.count < msg.maxEntryCount)
      msg.entries[msg.count] = displayId;
   msg.count++;
}

static void OnDmdSrcChanged(const unsigned int, void*, void*)
{
   bool wasRendering = IsSourceSelected();
   bool modeChanged = upscalerMode != nextUpscalerMode;
   DisplaySrcId selectedSrc {};

   // Request new source (without locking the render thread)
   if (nextUpscalerMode != UpscalerMode::UM_Disabled)
   {
      auto frame = resURIResolver->GetDisplayState("ctrl://default/display"s);
      if (frame.source != nullptr)
         selectedSrc = *frame.source;
   }

   // Setup new source and rendering
   {
      std::unique_lock lock(sourceMutex);
      dmdSrc = selectedSrc;
      upscalerMode = nextUpscalerMode;
      if (IsSourceSelected())
      {
         // LPI_LOGI("DMD Upscaler source selected [endpointId=%d, %dx%d]", selectedSrc.id.endpointId, selectedSrc.width, selectedSrc.height);
         displayId = {
            .id = { { endpointId, 0 } },
            .groupId = { { endpointId, 0 } },
            .overrideId = selectedSrc.id,
            .width = selectedSrc.width * scaleFactors[upscalerMode],
            .height = selectedSrc.height * scaleFactors[upscalerMode],
            .hardware = selectedSrc.hardware,
            .frameFormat = selectedSrc.frameFormat == CTLPI_DISPLAY_FORMAT_LUM32F ? CTLPI_DISPLAY_FORMAT_LUM32F : CTLPI_DISPLAY_FORMAT_SRGB888,
            .GetRenderFrame = &GetRenderFrame,
            .identifyFormat = selectedSrc.identifyFormat,
            .GetIdentifyFrame = scaleFactors[upscalerMode] == 1 ? selectedSrc.GetIdentifyFrame : nullptr
         };
         renderFrameId = 0;
         rgbaSrcFrame.resize(dmdSrc.width * dmdSrc.height);
         rgbaDstFrame.resize(displayId.width * displayId.height);
         if (displayId.frameFormat == CTLPI_DISPLAY_FORMAT_LUM32F)
         {
            lumFrame.resize(displayId.width * displayId.height);
            rgbFrame.resize(0);
            memset(lumFrame.data(), 0, lumFrame.size() * 4);
         }
         else
         {
            lumFrame.resize(0);
            rgbFrame.resize(displayId.width * displayId.height * 3);
            memset(rgbFrame.data(), 0, rgbFrame.size());
         }
      }
      if (wasRendering != IsSourceSelected())
      {
         if (IsSourceSelected())
         {
            isRunning = true;
            renderThread = std::thread(RenderThread);
            msgApi->SubscribeMsg(endpointId, getDisplaySrcId, OnGetDisplaySrc, nullptr);
         }
         else
         {
            msgApi->UnsubscribeMsg(getDisplaySrcId, OnGetDisplaySrc);
            isRunning = false;
            lock.unlock();
            updateCondVar.notify_all();
            if (renderThread.joinable())
               renderThread.join();
         }
      }
   }

   // If we are starting or stopping rendering, or changed our display, report it
   if (modeChanged || wasRendering != IsSourceSelected())
      msgApi->BroadcastMsg(endpointId, onDisplaySrcChangedId, nullptr);
}

}

using namespace UpscaleDMD;

MSGPI_EXPORT void MSGPIAPI UpscaleDMDPluginLoad(const uint32_t sessionId, const MsgPluginAPI* api)
{
   msgApi = api;
   endpointId = sessionId;
   LPISetup(endpointId, api);
   resURIResolver = std::make_unique<ResURIResolver>(*api, endpointId, true, false, false, false);
   resURIResolver->SetDisplayFilter([](const DisplaySrcId& src){ return src.id.endpointId != endpointId; });
   onDisplaySrcChangedId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DISPLAY_ON_SRC_CHG_MSG);
   getDisplaySrcId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DISPLAY_GET_SRC_MSG);
   msgApi->RegisterSetting(endpointId, &upscaleModeProp);
   msgApi->SubscribeMsg(endpointId, onDisplaySrcChangedId, OnDmdSrcChanged, nullptr);
   OnDmdSrcChanged(onDisplaySrcChangedId, nullptr, nullptr);
}

MSGPI_EXPORT void MSGPIAPI UpscaleDMDPluginUnload()
{
   msgApi->UnsubscribeMsg(onDisplaySrcChangedId, OnDmdSrcChanged);
   if (IsSourceSelected())
      msgApi->UnsubscribeMsg(getDisplaySrcId, OnGetDisplaySrc);
   isRunning = false;
   updateCondVar.notify_all();
   if (renderThread.joinable())
      renderThread.join();
   dmdSrc = {};
   msgApi->BroadcastMsg(endpointId, onDisplaySrcChangedId, nullptr);
   msgApi->ReleaseMsgID(onDisplaySrcChangedId);
   msgApi->ReleaseMsgID(getDisplaySrcId);
   resURIResolver = nullptr;
   msgApi = nullptr;
}
