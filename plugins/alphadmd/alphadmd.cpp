// license:GPLv3+

#include "plugins/MsgPlugin.h"
#include "plugins/ControllerPlugin.h"
#include "plugins/LoggingPlugin.h"

#include <array>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>
#include <cstring>
#include <cstdint>
#include <format>
#include <cassert>
#include <cstdarg>
#if defined(__APPLE__) || defined(__linux__) || defined(__ANDROID__)
#include <pthread.h>
#endif

// Uses original bitplane rendering from DmdDevice for backward compatible colorization support
#define LIBPINMAME
#define UINT8 uint8_t
#define UINT16 uint16_t
#include "usbalphanumeric.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <locale>
#endif

///////////////////////////////////////////////////////////////////////////////
//
// AlphaDMD plugin: generate DMD from alphanumeric segment displays
//
// This plugin has 2 purposes:
// - Generate DMD frame for rendering on DMD hardware
// - Provide identification frames for alphanumeric to DMD colorizations
//
// This plugin only relies on the generic messaging plugin API and the generic
// controller display and segment API. It listens for alphanumeric source and, 
// when found, provide corresponding DMD sources (128x32 and 256x64 variants)
// with identify capabilities for the 128x32 variant.
//
// All rendering is done by an ancillary thread, causing a one frame delay, but
// avoiding CPU load on the main thread.

namespace AlphaDMD {

using namespace std::string_literals;
using namespace std::string_view_literals;
using namespace PinballPlugin::Controller;

static const MsgPluginAPI* msgApi = nullptr;
static uint32_t endpointId;
static std::unique_ptr<CtrlItemConsumer<SegSrcId>> segSource;
static std::unique_ptr<class AlphaDMDRenderer> renderer;

LPI_USE_CPP();
#define LOGD AlphaDMD::LPI_LOGD_CPP
#define LOGI AlphaDMD::LPI_LOGI_CPP
#define LOGW AlphaDMD::LPI_LOGW_CPP
#define LOGE AlphaDMD::LPI_LOGE_CPP

LPI_IMPLEMENT_CPP // Implement shared log support

typedef enum {
   Undefined,
   Layout_4x6_2x2,
   Layout_4x7,
   Layout_4x7_2x2,
   Layout_6x4_2x2,
   Layout_2x16_1x7,
   Layout_2x16,
   Layout_2x20,
   Layout_2x7_2x2_1x16,
   Layout_1x7_2x16,
   Layout_1x7_1x4_2x16,
   Layout_4x7_5x2,
   Layout_4x6_2x2_1x6,
} DmdLayouts;

template <typename T> constexpr inline T clamp(const T x, const T mn, const T mx) { return std::max(std::min(x, mx), mn); }

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

class AlphaDMDRenderer
{
public:
   AlphaDMDRenderer(unsigned int sourceEndpointId, const DmdLayouts dmdLayout)
      : m_dmdLayout(dmdLayout)
      , m_dmdProvider(msgApi, endpointId, CTLPI_DISPLAY_GET_SRC_MSG, CTLPI_DISPLAY_ON_SRC_CHG_MSG)
   {
      m_renderThread = std::thread(&AlphaDMDRenderer::RenderThread, this);
      m_dmdProvider.AddItem({
         .id = { { endpointId, 0 } },
         .groupId = { { endpointId, 0 } },
         .overrideId = { { sourceEndpointId, 0xFFFF } }, // We do not override a DMD but we want to be able to identify the source endpointId for colorization purposes
         .width = 128,
         .height = 32,
         .hardware = CTLPI_DISPLAY_HARDWARE_UNKNOWN,
         .frameFormat = CTLPI_DISPLAY_FORMAT_LUM32F,
         .GetRenderFrame = &GetRenderFrame,
         .identifyFormat = CTLPI_DISPLAY_ID_FORMAT_BITPLANE2,
         .GetIdentifyFrame = &GetIdentifyFrame });
   }

   ~AlphaDMDRenderer()
   {
      StopRenderThread();
      m_dmdProvider.ClearItems();
   }

private:
   void StopRenderThread()
   {
      {
         std::lock_guard lock(m_mutex);
         m_isRunning = false;
      }
      m_updateCondVar.notify_all();
      if (m_renderThread.joinable())
         m_renderThread.join();
   }
   
   void RenderThread()
   {
      SetThreadName("AlphaDMD.RenderThread"s);
      for (;;)
      {
         {
            std::unique_lock lock(m_mutex);
            m_updateCondVar.wait(lock, [this] { return m_renderRequested || !m_isRunning; });
            m_renderRequested = false;
            if (!m_isRunning)
               break;
            lock.unlock();
         }

         segSource->With([this](const std::vector<SegSrcId>& selectedSources){ ProcessFrame(selectedSources); });
      }
   }

   void ProcessFrame(const std::vector<SegSrcId>& selectedSources)
   {
      // Get segment display state and compute backward compatible binary version
      float* lum = m_groupLum;
      bool changed = false;
      m_lastFrameId.resize(selectedSources.size());
      for (size_t i = 0, pos = 0; i < selectedSources.size(); i++)
      {
         if (const SegDisplayFrame seg = selectedSources[i].GetState(selectedSources[i].id); seg.frameId != m_lastFrameId[i])
         {
            changed = true;
            m_lastFrameId[i] = seg.frameId;
            memcpy(lum, seg.frame, selectedSources[i].nElements * (16 * sizeof(float)));
            for (unsigned int j = 0; j < selectedSources[i].nElements; j++)
            {
               const int nSegs = nSegments[selectedSources[i].elementType[j]];
               m_seg_data[pos + j] = 0;
               for (int k = 0; k < nSegs; k++)
                  if (seg.frame[j * 16 + k] > 0.5f)
                     m_seg_data[pos + j] |= 1u << k;
            }
         }
         lum += selectedSources[i].nElements * 16;
         pos += selectedSources[i].nElements;
      }
      if (!changed)
         return;

      // Render for DMD
      lum = m_groupLum;
      memset(m_renderFrame, 0, sizeof(m_renderFrame));
      switch (m_dmdLayout)
      {
      case Layout_6x4_2x2: // S11 Bowl games
         DrawDisplay(2, 2, lum, selectedSources[0], false);
         DrawDisplay(92, 2, lum, selectedSources[1], false);
         DrawDisplay(2, 12, lum, selectedSources[2], false);
         DrawDisplay(92, 12, lum, selectedSources[3], false);
         DrawDisplay(2, 22, lum, selectedSources[4], false);
         DrawDisplay(92, 22, lum, selectedSources[5], false);
         DrawDisplay(56, 7, lum, selectedSources[6], false);
         DrawDisplay(56, 20, lum, selectedSources[7], false);
         break;
      case Layout_4x6_2x2: // Lots of games (4 players + credit/ball)
         DrawDisplay(0, 0, lum, selectedSources[0], true);
         DrawDisplay(80, 0, lum, selectedSources[1], true);
         DrawDisplay(0, 12, lum, selectedSources[2], true);
         DrawDisplay(80, 12, lum, selectedSources[3], true);
         DrawDisplay(8, 24, lum, selectedSources[4], false);
         DrawDisplay(32, 24, lum, selectedSources[5], false);
         break;
      case Layout_4x6_2x2_1x6: // Black Hole
         DrawDisplay(0, 0, lum, selectedSources[0], false);
         DrawDisplay(80, 0, lum, selectedSources[1], false);
         DrawDisplay(0, 12, lum, selectedSources[2], false);
         DrawDisplay(80, 12, lum, selectedSources[3], false);
         DrawDisplay(56, 0, lum, selectedSources[4], false);
         DrawDisplay(56, 12, lum, selectedSources[5], false);
         DrawDisplay(40, 24, lum, selectedSources[6], false);
         break;
      case Layout_4x7:
         DrawDisplay(0, 2, lum, selectedSources[0], true);
         DrawDisplay(72, 2, lum, selectedSources[1], true);
         DrawDisplay(0, 19, lum, selectedSources[2], true);
         DrawDisplay(72, 19, lum, selectedSources[3], true);
         break;
      case Layout_4x7_2x2: // Lots of games (4 players + credit/ball)
         DrawDisplay(0, 0, lum, selectedSources[0], true);
         DrawDisplay(72, 0, lum, selectedSources[1], true);
         DrawDisplay(0, 12, lum, selectedSources[2], true);
         DrawDisplay(72, 12, lum, selectedSources[3], true);
         DrawDisplay(8, 24, lum, selectedSources[4], false);
         DrawDisplay(32, 24, lum, selectedSources[5], false);
         break;
      case Layout_4x7_5x2: // Medusa
         DrawDisplay(0, 0, lum, selectedSources[0], true);
         DrawDisplay(72, 0, lum, selectedSources[1], true);
         DrawDisplay(0, 12, lum, selectedSources[2], true);
         DrawDisplay(72, 12, lum, selectedSources[3], true);
         DrawDisplay(16, 24, lum, selectedSources[4], false);
         DrawDisplay(40, 24, lum, selectedSources[5], false);
         DrawDisplay(64, 24, lum, selectedSources[6], false);
         DrawDisplay(88, 24, lum, selectedSources[7], false);
         DrawDisplay(112, 24, lum, selectedSources[8], false);
         break;
      case Layout_2x16: // Lots of later games
         DrawDisplay(0, 2, lum, selectedSources[0], true);
         DrawDisplay(0, 19, lum, selectedSources[1], true);
         break;
      case Layout_1x7_2x16: // Police Force
         DrawDisplay(68, 1, lum, selectedSources[0], false);
         DrawDisplay(0, 9, lum, selectedSources[1], true);
         DrawDisplay(0, 21, lum, selectedSources[2], true);
         break;
      case Layout_2x16_1x7: // Taxi
         DrawDisplay(0, 9, lum, selectedSources[0], true);
         DrawDisplay(0, 21, lum, selectedSources[1], true);
         DrawDisplay(68, 1, lum, selectedSources[2], false);
         break;
      case Layout_1x7_1x4_2x16: // Riverboat Gambler
         DrawDisplay(0, 1, lum, selectedSources[0], false);
         DrawDisplay(96, 1, lum, selectedSources[1], false);
         DrawDisplay(0, 9, lum, selectedSources[2], true);
         DrawDisplay(0, 21, lum, selectedSources[3], true);
         break;
      case Layout_2x7_2x2_1x16: // Hyperball
         DrawDisplay(0, 0, lum, selectedSources[0], true);
         DrawDisplay(72, 0, lum, selectedSources[1], true);
         DrawDisplay(16, 12, lum, selectedSources[2], false);
         DrawDisplay(40, 12, lum, selectedSources[3], false);
         DrawDisplay(16, 21, lum, selectedSources[4], true);
         break;
      case Layout_2x20: // Lots of later games
         DrawDisplay(4, 2, lum, selectedSources[0], false);
         DrawDisplay(4, 19, lum, selectedSources[1], false);
         break;
      default: break;
      }
      if (memcmp(m_dmd128Frame, m_renderFrame, sizeof(m_dmd128Frame)) != 0)
      {
         //std::lock_guard<std::mutex> lock(renderMutex);
         memcpy(m_dmd128Frame, m_renderFrame, sizeof(m_dmd128Frame));
         m_renderFrameId++;
      }

      // Render to bitplane surface for frame identification (backward compatible way of rendering to avoid breaking existing colorizations)
      memset(AlphaNumericFrameBuffer, 0, sizeof(AlphaNumericFrameBuffer));
      const SegElementType firstType = selectedSources[0].elementType[0];
      switch (m_dmdLayout)
      {
      case Layout_6x4_2x2: _6x4Num_4x1Num(m_seg_data); break;
      case Layout_4x6_2x2:
         if ((firstType == CTLPI_SEG_LAYOUT_9) || (firstType == CTLPI_SEG_LAYOUT_9C))
            _2x6Num10_2x6Num10_4x1Num(m_seg_data);
         else
            _2x6Num_2x6Num_4x1Num(m_seg_data);
         break;
      case Layout_4x6_2x2_1x6: return; // Unsupported
      case Layout_4x7:
         if (firstType >= CTLPI_SEG_LAYOUT_14)
            _2x7Alpha_2x7Num(m_seg_data);
         else
            _4x7Num10(m_seg_data);
         break;
      case Layout_4x7_2x2:
         if ((firstType == CTLPI_SEG_LAYOUT_9) || (firstType == CTLPI_SEG_LAYOUT_9C))
            _2x7Num10_2x7Num10_4x1Num(m_seg_data);
         else if ((firstType == CTLPI_SEG_LAYOUT_14) || (firstType == CTLPI_SEG_LAYOUT_14D) || (firstType == CTLPI_SEG_LAYOUT_14DC))
            _2x7Alpha_2x7Num_4x1Num(m_seg_data);
         else if (selectedSources[0].elementType[2] == CTLPI_SEG_LAYOUT_7) // No thousands comma
            _2x7Num_2x7Num_4x1Num(m_seg_data);
         else // With thousands comma
            _2x7Num_2x7Num_4x1Num_gen7(m_seg_data);
         break;
      case Layout_4x7_5x2: _2x7Num_2x7Num_10x1Num(m_seg_data, m_seg_data2); break; // FIXME Medusa: m_seg_data2 is not initialized. Is this really needed ?
      case Layout_2x16: _2x16Alpha(m_seg_data); break;
      case Layout_1x7_2x16: _1x7Num_1x16Alpha_1x16Num(m_seg_data); break;
      case Layout_2x16_1x7: _1x16Alpha_1x16Num_1x7Num(m_seg_data); break;
      case Layout_1x7_1x4_2x16: _1x16Alpha_1x16Num_1x7Num_1x4Num(m_seg_data); break; // FIXME Riverboat Gambler: reverse order
      case Layout_2x7_2x2_1x16: _2x7Num_4x1Num_1x16Alpha(m_seg_data); break;
      case Layout_2x20: _2x20Alpha(m_seg_data); break;
      default: break;
      }
      if (memcmp(m_identifyFrame, AlphaNumericFrameBuffer, sizeof(AlphaNumericFrameBuffer)) != 0)
      {
         //std::lock_guard<std::mutex> lock(renderMutex);
         memcpy(m_identifyFrame, AlphaNumericFrameBuffer, sizeof(AlphaNumericFrameBuffer));
         m_identifyFrameId++;
      }
   }

   void DrawDisplay(int x, const int y, float*& lum, const SegSrcId& segSrc, const bool large)
   {
      for (unsigned int i = 0; i < segSrc.nElements; i++)
      {
         const SegElementType type = segSrc.elementType[i];
         SegImgs img = SegImg_Invalid;
         switch (type)
         {
         case CTLPI_SEG_LAYOUT_7: img = large ? SegImg_Seg9C_8x11 : SegImg_Seg9C_8x7; break;
         case CTLPI_SEG_LAYOUT_7C: img = large ? SegImg_Seg9C_8x11 : SegImg_Seg9C_8x7; break;
         case CTLPI_SEG_LAYOUT_7D: img = SegImg_Seg9D_8x11; break;
         case CTLPI_SEG_LAYOUT_9: img = large ? SegImg_Seg9C_8x11 : SegImg_Seg9C_8x7; break;
         case CTLPI_SEG_LAYOUT_9C: img = large ? SegImg_Seg9C_8x11 : SegImg_Seg9C_8x7; break;
         case CTLPI_SEG_LAYOUT_14: img = large ? SegImg_Seg14DC_8x11 : SegImg_Seg14DC_6x11; break;
         case CTLPI_SEG_LAYOUT_14D: img = large ? SegImg_Seg14DC_8x11 : SegImg_Seg14DC_6x11; break;
         case CTLPI_SEG_LAYOUT_14DC: img = large ? SegImg_Seg14DC_8x11 : SegImg_Seg14DC_6x11; break;
         case CTLPI_SEG_LAYOUT_16: img = SegImg_Seg16_7x11; break;
         default: assert(false); return;
         }
         DrawChar(x, y, segDisplays[img], lum, nSegments[type]);
         x += segDisplays[img].width;
         lum += 16;
      }
   }

   // Number of segments corresponding to CTLPI_SEG_LAYOUT_xxx
   static constexpr int nSegments[] = { 7, 8, 8, 10, 10, 15, 15, 16, 16 };

   // Segment layouts, derived from PinMAME, itself taking it from 'usbalphanumeric.h'

   typedef enum
   {
      SegImg_Seg14DC_8x11,
      SegImg_Seg14DC_6x11,
      SegImg_Seg16_7x11,
      SegImg_Seg9C_8x11,
      SegImg_Seg9C_8x7,
      SegImg_Seg9D_8x11,
      SegImg_Invalid,
   } SegImgs;

   typedef struct
   {
      int nDots;
      uint8_t dots[5][2];
   } segLine;

   typedef struct
   {
      int width; // The width includes the spacing before next character can be drawn (no additional spacing)
      int height;
      segLine segs[16];
   } segDisplay;

   void DrawChar(const int x, const int y, const segDisplay& display, const float* const __restrict lum, const int nSeg)
   {
      assert((x >= 0) && (x + display.width <= 128));
      assert((y >= 0) && (y + display.height <= 32));
      const int offset = 128 * y + x;
      for (int seg = 0; seg < nSeg; seg++)
      {
         const float v = clamp(lum[seg], 0.01f, 1.f);
         for (int i = 0; i < display.segs[seg].nDots; i++)
         {
            const int pos = 128 * display.segs[seg].dots[i][1] + display.segs[seg].dots[i][0] + offset;
            m_renderFrame[pos] = std::min(m_renderFrame[pos] + v, 1.f);
         }
      }
   }

   static constexpr segDisplay segDisplays[6] = {
      // 14 Segments + dot/comma
      { 8, 11,
         {
            { 5, { { 1, 0 }, { 2, 0 }, { 3, 0 }, { 4, 0 }, { 5, 0 } } }, //  0 top
            { 5, { { 6, 0 }, { 6, 1 }, { 6, 2 }, { 6, 3 }, { 6, 4 } } }, //  1 right top
            { 5, { { 6, 6 }, { 6, 7 }, { 6, 8 }, { 6, 9 }, { 6, 10 } } }, //  2 right bottom
            { 5, { { 1, 10 }, { 2, 10 }, { 3, 10 }, { 4, 10 }, { 5, 10 } } }, //  3 bottom
            { 5, { { 0, 6 }, { 0, 7 }, { 0, 8 }, { 0, 9 }, { 0, 10 } } }, //  4 left bottom
            { 5, { { 0, 0 }, { 0, 1 }, { 0, 2 }, { 0, 3 }, { 0, 4 } } }, //  5 left top
            { 2, { { 1, 5 }, { 2, 5 }, { 0, 0 }, { 0, 0 }, { 0, 0 } } }, //  6 middle left
            { 2, { { 7, 9 }, { 7, 10 }, { 0, 0 }, { 0, 0 }, { 0, 0 } } }, //  7 comma
            { 5, { { 0, 0 }, { 1, 1 }, { 1, 2 }, { 2, 3 }, { 2, 4 } } }, //  8 diag top left
            { 5, { { 3, 0 }, { 3, 1 }, { 3, 2 }, { 3, 3 }, { 3, 4 } } }, //  9 center top
            { 5, { { 6, 0 }, { 5, 1 }, { 5, 2 }, { 4, 3 }, { 4, 4 } } }, // 10 diag top right
            { 2, { { 4, 5 }, { 5, 5 }, { 0, 0 }, { 0, 0 }, { 0, 0 } } }, // 11 middle right
            { 5, { { 4, 6 }, { 4, 7 }, { 5, 8 }, { 5, 9 }, { 6, 10 } } }, // 12 diag bottom right
            { 5, { { 3, 6 }, { 3, 7 }, { 3, 8 }, { 3, 9 }, { 3, 10 } } }, // 13 center bottom
            { 5, { { 0, 10 }, { 2, 6 }, { 2, 7 }, { 1, 8 }, { 1, 9 } } }, // 14 diag bottom left
            { 1, { { 7, 10 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } } }, // 15 dot
         } },
      { 6, 11,
         {
            { 5, { { 0, 0 }, { 1, 0 }, { 2, 0 }, { 3, 0 }, { 4, 0 } } }, //  0 top
            { 5, { { 4, 0 }, { 4, 1 }, { 4, 2 }, { 4, 3 }, { 4, 4 } } }, //  1 top right
            { 5, { { 4, 6 }, { 4, 7 }, { 4, 8 }, { 4, 9 }, { 4, 10 } } }, //  2 right bottom
            { 5, { { 0, 10 }, { 1, 10 }, { 2, 10 }, { 3, 10 }, { 4, 10 } } }, //  3 bottom
            { 5, { { 0, 6 }, { 0, 7 }, { 0, 8 }, { 0, 9 }, { 0, 10 } } }, //  4 left bottom
            { 5, { { 0, 0 }, { 0, 1 }, { 0, 2 }, { 0, 3 }, { 0, 4 } } }, //  5 left top
            { 3, { { 0, 5 }, { 1, 5 }, { 2, 5 }, { 0, 0 }, { 0, 0 } } }, //  6 middle left
            { 2, { { 5, 9 }, { 5, 10 }, { 0, 0 }, { 0, 0 }, { 0, 0 } } }, //  7 dot
            { 5, { { 0, 0 }, { 1, 1 }, { 1, 2 }, { 2, 3 }, { 2, 4 } } }, //  8 diag top left
            { 5, { { 2, 0 }, { 2, 1 }, { 2, 2 }, { 2, 3 }, { 2, 4 } } }, //  9 center top
            { 5, { { 4, 0 }, { 3, 1 }, { 3, 2 }, { 2, 3 }, { 2, 4 } } }, // 10 diag top right
            { 3, { { 2, 5 }, { 3, 5 }, { 4, 5 }, { 0, 0 }, { 0, 0 } } }, // 11 middle right
            { 5, { { 2, 6 }, { 2, 7 }, { 3, 8 }, { 3, 9 }, { 4, 10 } } }, // 12 diag bottom right
            { 5, { { 2, 6 }, { 2, 7 }, { 2, 8 }, { 2, 9 }, { 2, 10 } } }, // 13 center bottom
            { 5, { { 0, 10 }, { 2, 6 }, { 2, 7 }, { 1, 8 }, { 1, 9 } } }, // 14 diag bottom left
            { 1, { { 5, 10 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } } }, // 15 comma
         } },
      // 16 Segments (split top/bottom)
      { 8, 11,
         {
            { 2, { { 1, 0 }, { 2, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } } }, //  0 top left
            { 2, { { 4, 0 }, { 5, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } } }, //  1 top right
            { 5, { { 6, 0 }, { 6, 1 }, { 6, 2 }, { 6, 3 }, { 6, 4 } } }, //  2 right top
            { 5, { { 6, 6 }, { 6, 7 }, { 6, 8 }, { 6, 9 }, { 6, 10 } } }, //  3 right bottom
            { 2, { { 4, 10 }, { 5, 10 }, { 0, 0 }, { 0, 0 }, { 0, 0 } } }, //  5 bottom right
            { 2, { { 1, 10 }, { 2, 10 }, { 0, 0 }, { 0, 0 }, { 0, 0 } } }, //  4 bottom left
            { 5, { { 0, 6 }, { 0, 7 }, { 0, 8 }, { 0, 9 }, { 0, 10 } } }, //  6 left bottom
            { 5, { { 0, 0 }, { 0, 1 }, { 0, 2 }, { 0, 3 }, { 0, 4 } } }, //  7 left top
            { 5, { { 0, 0 }, { 1, 1 }, { 1, 2 }, { 2, 3 }, { 2, 4 } } }, //  8 diag top left
            { 5, { { 3, 0 }, { 3, 1 }, { 3, 2 }, { 3, 3 }, { 3, 4 } } }, //  9 center top
            { 5, { { 6, 0 }, { 5, 1 }, { 5, 2 }, { 4, 3 }, { 4, 4 } } }, // 10 diag top right
            { 2, { { 4, 5 }, { 5, 5 }, { 0, 0 }, { 0, 0 }, { 0, 0 } } }, // 11 middle right
            { 5, { { 4, 6 }, { 4, 7 }, { 5, 8 }, { 5, 9 }, { 6, 10 } } }, // 12 diag bottom right
            { 5, { { 3, 6 }, { 3, 7 }, { 3, 8 }, { 3, 9 }, { 3, 10 } } }, // 13 center bottom
            { 5, { { 0, 10 }, { 2, 6 }, { 2, 7 }, { 1, 8 }, { 1, 9 } } }, // 14 diag bottom left
            { 2, { { 1, 5 }, { 2, 5 }, { 0, 0 }, { 0, 0 }, { 0, 0 } } }, // 15 middle left
         } },
      // 9 Segments + comma
      { 8, 11,
         {
            { 5, { { 1, 0 }, { 2, 0 }, { 3, 0 }, { 4, 0 }, { 5, 0 } } }, // 0 top
            { 5, { { 6, 0 }, { 6, 1 }, { 6, 2 }, { 6, 3 }, { 6, 4 } } }, // 1 right top
            { 5, { { 6, 6 }, { 6, 7 }, { 6, 8 }, { 6, 9 }, { 6, 10 } } }, // 2 right bottom
            { 5, { { 1, 10 }, { 2, 10 }, { 3, 10 }, { 4, 10 }, { 5, 10 } } }, // 3 bottom
            { 5, { { 0, 6 }, { 0, 7 }, { 0, 8 }, { 0, 9 }, { 0, 10 } } }, // 4 left bottom
            { 5, { { 0, 0 }, { 0, 1 }, { 0, 2 }, { 0, 3 }, { 0, 4 } } }, // 5 left top
            { 5, { { 1, 5 }, { 2, 5 }, { 3, 5 }, { 4, 5 }, { 5, 5 } } }, // 6 middle
            { 2, { { 7, 9 }, { 7, 10 }, { 0, 0 }, { 0, 0 }, { 0, 0 } } }, // 7 comma
            { 5, { { 3, 0 }, { 3, 1 }, { 3, 2 }, { 3, 3 }, { 3, 4 } } }, // 8 center top
            { 5, { { 3, 6 }, { 3, 7 }, { 3, 8 }, { 3, 9 }, { 3, 10 } } }, // 9 center bottom
         } },
      { 8, 7,
         {
            { 5, { { 1, 0 }, { 2, 0 }, { 3, 0 }, { 4, 0 }, { 5, 0 } } }, // 0 top
            { 2, { { 6, 1 }, { 6, 2 }, { 0, 0 }, { 0, 0 }, { 0, 0 } } }, // 1 top right
            { 2, { { 6, 4 }, { 6, 5 }, { 0, 0 }, { 0, 0 }, { 0, 0 } } }, // 2 bottom right
            { 5, { { 1, 6 }, { 2, 6 }, { 3, 6 }, { 4, 6 }, { 5, 6 } } }, // 3 bottom
            { 2, { { 0, 4 }, { 0, 5 }, { 0, 0 }, { 0, 0 }, { 0, 0 } } }, // 4 bottom left
            { 2, { { 0, 1 }, { 0, 2 }, { 0, 0 }, { 0, 0 }, { 0, 0 } } }, // 5 top left
            { 5, { { 1, 3 }, { 2, 3 }, { 3, 3 }, { 4, 3 }, { 5, 3 } } }, // 6 middle
            { 2, { { 7, 5 }, { 7, 6 }, { 0, 0 }, { 0, 0 }, { 0, 0 } } }, // 7 comma
            { 3, { { 3, 0 }, { 3, 1 }, { 3, 2 }, { 0, 0 }, { 0, 0 } } }, // 8 center top
            { 3, { { 3, 4 }, { 3, 5 }, { 3, 6 }, { 0, 0 }, { 0, 0 } } }, // 9 center bottom
         } },
      // 9 Segments + dot
      { 8, 11,
         {
            { 5, { { 1, 0 }, { 2, 0 }, { 3, 0 }, { 4, 0 }, { 5, 0 } } }, // 0 top
            { 5, { { 6, 0 }, { 6, 1 }, { 6, 2 }, { 6, 3 }, { 6, 4 } } }, // 1 right top
            { 5, { { 6, 6 }, { 6, 7 }, { 6, 8 }, { 6, 9 }, { 6, 10 } } }, // 2 right bottom
            { 5, { { 1, 10 }, { 2, 10 }, { 3, 10 }, { 4, 10 }, { 5, 10 } } }, // 3 bottom
            { 5, { { 0, 6 }, { 0, 7 }, { 0, 8 }, { 0, 9 }, { 0, 10 } } }, // 4 left bottom
            { 5, { { 0, 0 }, { 0, 1 }, { 0, 2 }, { 0, 3 }, { 0, 4 } } }, // 5 left top
            { 5, { { 1, 5 }, { 2, 5 }, { 3, 5 }, { 4, 5 }, { 5, 5 } } }, // 6 middle
            { 1, { { 7, 10 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } } }, // 7 dot
            { 5, { { 3, 0 }, { 3, 1 }, { 3, 2 }, { 3, 3 }, { 3, 4 } } }, // 8 center top
            { 5, { { 3, 6 }, { 3, 7 }, { 3, 8 }, { 3, 9 }, { 3, 10 } } }, // 9 center bottom
         } },
   };

   static DisplayFrame GetRenderFrame(const CtlResId id)
   {
      // For the time being, we have a single renderer with a single id, so no need to derive it from the id
      // Note that to be fully clean we should do a copy of the render (since the direct data is updated asynchronously, so eventually while it is read by consumer)
      {
         std::lock_guard lock(renderer->m_mutex);
         renderer->m_renderRequested = true;
      }
      renderer->m_updateCondVar.notify_one();
      return { renderer->m_renderFrameId, renderer->m_dmd128Frame };
   }

   static DisplayFrame GetIdentifyFrame(const CtlResId id)
   {
      // For the time being, we have a single renderer with a single id, so no need to derive it from the id
      // Note that to be fully clean we should do a copy of the render (since the direct data is updated asynchronously, so eventually while it is read by consumer)
      {
         std::lock_guard lock(renderer->m_mutex);
         renderer->m_renderRequested = true;
      }
      renderer->m_updateCondVar.notify_one();
      return { renderer->m_identifyFrameId, renderer->m_identifyFrame };
   }

   const DmdLayouts m_dmdLayout;
   CtrlItemProvider<DisplaySrcId> m_dmdProvider;

   std::thread m_renderThread;
   std::condition_variable m_updateCondVar;
   std::mutex m_mutex;
   bool m_isRunning = true;
   bool m_renderRequested = true;
   
   uint16_t m_seg_data[128] = { };
   uint16_t m_seg_data2[128] = { };
   float m_groupLum[128 * 16] = { };
   std::vector<unsigned int> m_lastFrameId;

   float m_renderFrame[128 * 32] = { };
   float m_dmd128Frame[128 * 32] = { };
   unsigned int m_renderFrameId = 0;

   uint8_t m_identifyFrame[128 * 32] = { };
   unsigned int m_identifyFrameId = 0;
};

static void SelectSource(std::vector<SegSrcId>& items)
{
   // Simply select the first group for the time being, maybe we could have some user setup on this
   const auto firstGroupId = items[0].groupId.id;
   std::erase_if(items, [firstGroupId](const SegSrcId& src) { return src.groupId.id != firstGroupId; });
}

static void SetupRenderer()
{
   unsigned int selectedEndpoint = 0;
   DmdLayouts selectedLayout = DmdLayouts::Undefined;
   segSource->With(
      [&selectedLayout, &selectedEndpoint](const std::vector<SegSrcId>& selectedSources)
      {
         if (selectedSources.empty())
            return;
         
         selectedEndpoint = selectedSources.front().id.endpointId;
         
         // Find a matching layout
         static constexpr int layouts[13][16] = {
            { DmdLayouts::Undefined, 0 },
            { DmdLayouts::Layout_4x6_2x2, 6, 6, 6, 6, 6, 2, 2 }, // Bally, GTS1, GTS80, S3, S4, S6, ...
            { DmdLayouts::Layout_4x7, 4, 7, 7, 7, 7 }, // Bally, S11, ...
            { DmdLayouts::Layout_4x7_2x2, 6, 7, 7, 7, 7, 2, 2 }, // Bally, GTS1, GTS80, Data East, ...
            { DmdLayouts::Layout_6x4_2x2, 8, 4, 4, 4, 4, 4, 4, 2, 2 }, // S11 bowling games
            { DmdLayouts::Layout_2x16_1x7, 3, 16, 16, 7 }, // S11
            { DmdLayouts::Layout_2x16, 2, 16, 16 }, // WPC, Data East, S11, ...
            { DmdLayouts::Layout_2x20, 2, 20, 20 }, // Hankin, GTS3, GTS80B, ...
            { DmdLayouts::Layout_2x7_2x2_1x16, 6, 7, 7, 2, 2, 16 }, // Hyperball
            { DmdLayouts::Layout_1x7_2x16, 3, 7, 16, 16 }, // Police Force
            { DmdLayouts::Layout_1x7_1x4_2x16, 4, 7, 4, 16, 16 }, // River Boat Gambler
            { DmdLayouts::Layout_4x7_5x2, 9, 7, 7, 7, 7, 2, 2, 2, 2, 2 }, // Medusa
            { DmdLayouts::Layout_4x6_2x2_1x6, 7, 6, 6, 6, 6, 2, 2, 6 }, // Black Hole
         };
         for (int i = 0; (selectedLayout == DmdLayouts::Undefined) && (i < 12); i++)
         {
            if (layouts[i][1] == (int)selectedSources.size())
            {
               selectedLayout = static_cast<DmdLayouts>(layouts[i][0]);
               for (size_t j = 0; j < selectedSources.size(); j++)
               {
                  if (layouts[i][j + 2] != selectedSources[j].nElements)
                  {
                     selectedLayout = DmdLayouts::Undefined;
                     break;
                  }
               }
            }
         }

         std::string elements;
         for (size_t i = 0; i < selectedSources.size(); i++)
            elements += std::format("{}{}", i == 0 ? "" : ", ", selectedSources[i].nElements);
         if (selectedLayout == DmdLayouts::Undefined)
            LOGI(std::format("Unsupported segment layout ({} displays: {})", selectedSources.size(), elements));
         else
         {
            static const std::array<std::string_view, 13> dmdLayoutNames { "Undefined"sv, "4x6+2x2"sv, "4x7"sv, "4x7+2x2"sv, "6x4+2x2"sv, "2x16+1x7"sv, "2x16"sv, "2x20"sv, "2x7+2x2+1x16"sv,
               "1x7+2x16"sv, "1x7+1x4+2x16"sv, "4x7+5x2"sv, "4x6+2x2+1x6"sv };
            const std::string_view name = selectedLayout < dmdLayoutNames.size() ? dmdLayoutNames[selectedLayout] : "Undefined"sv;
            LOGI(std::format("Matched layout {} ({} displays: {})", name, selectedSources.size(), elements));
         }
      });

   if (selectedLayout != DmdLayouts::Undefined)
      renderer = std::make_unique<AlphaDMDRenderer>(selectedEndpoint, selectedLayout);
}

}

using namespace AlphaDMD;

MSGPI_EXPORT void MSGPIAPI AlphaDMDPluginLoad(const uint32_t sessionId, const MsgPluginAPI* api)
{
   msgApi = api;
   endpointId = sessionId;
   LPISetup(endpointId, msgApi);
   segSource = std::make_unique<CtrlItemConsumer<SegSrcId>>(
      msgApi, endpointId, CTLPI_SEG_GET_SRC_MSG, CTLPI_SEG_ON_SRC_CHG_MSG,
      [](std::vector<SegSrcId>& items) { SelectSource(items); },
      []() { renderer = nullptr; },
      []() { SetupRenderer(); });
   segSource->Subscribe();
}

MSGPI_EXPORT void MSGPIAPI AlphaDMDPluginUnload()
{
   segSource->Unsubscribe();
   segSource = nullptr;
   msgApi = nullptr;
}
