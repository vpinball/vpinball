// license:GPLv3+

#include "MsgPlugin.h"
#include "ControllerPlugin.h"
#include "LoggingPlugin.h"

#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>
#include <cstring>
#include <cstdint>
#include <sstream>
#include <cassert>
#include <cstdarg>

// Uses original bitplane rendering from DmdDevice for backward compatible colorization support
#define LIBPINMAME
#define UINT8 uint8_t
#define UINT16 uint16_t
#include "usbalphanumeric.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
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
// This plugin only rely on the generic messaging plugin API and the generic
// controller display and segment API. It listen for alphanumeric source and, 
// when found, provide corresponding DMD sources (128x32 and 256x64 variants)
// with identify capabilities for the 128x32 variant.
//
// All rendering is done by an anciliary thread, causing a one frame delay, but
// avoiding CPU load on the main thread.

namespace AlphaDMD {

static MsgPluginAPI* msgApi = nullptr;
static uint32_t endpointId;
static unsigned int onDmdSrcChangedId, getDmdSrcId;
static unsigned int onSegSrcChangedId, getSegSrcId;

static std::vector<SegSrcId> selectedSources;

static std::mutex sourceMutex;
static std::mutex renderMutex;
static std::thread renderThread;
static std::condition_variable updateCondVar;
static bool isRunning = false;

static DisplaySrcId dmd128Id, dmd256Id;
static uint8_t renderFrame[128 * 32] = {0};
static uint8_t dmd128Frame[128 * 32] = {0};
static uint8_t dmd256Frame[256 * 64] = {0};
static unsigned int renderFrameId = 0;

static uint8_t identifyFrame[128*32] = {0};
static unsigned int identifyFrameId = 0;

LPI_USE();
LPI_IMPLEMENT // Implement shared login support

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
static DmdLayouts dmdLayout = DmdLayouts::Undefined;

// Number of segments corresponding to CTLPI_SEG_LAYOUT_xxx
static constexpr int nSegments[] = { 7, 8, 8, 10, 10, 15, 15, 16, 16 };

// Segment layouts, derived from PinMame, itself taking it from 'usbalphanumeric.h'

typedef enum
{
   SegImg_Seg14DC_8x10,
   SegImg_Seg14DC_6x10,
   SegImg_Seg16_8x10,
   SegImg_Seg9C_8x10,
   SegImg_Seg9C_8x6,
   SegImg_Seg9D_8x10,
   SegImg_Invalid,
} SegImgs;

typedef struct
{
   int nDots;
   uint8_t dots[5][2];
} segLine;

typedef struct
{
   int width;
   int height;
   segLine segs[16];
} segDisplay;

static constexpr segDisplay segDisplays[6] = {
   // 14 Segments + dot/comma
   { 8, 10,
      {
         { 5, { { 1,  0 }, { 2,  0 }, { 3,  0 }, { 4,  0 }, { 5,  0 } } }, //  0 top
         { 5, { { 6,  0 }, { 6,  1 }, { 6,  2 }, { 6,  3 }, { 6,  4 } } }, //  1 right top
         { 5, { { 6,  6 }, { 6,  7 }, { 6,  8 }, { 6,  9 }, { 6, 10 } } }, //  2 right bottom
         { 5, { { 1, 10 }, { 2, 10 }, { 3, 10 }, { 4, 10 }, { 5, 10 } } }, //  3 bottom
         { 5, { { 0,  6 }, { 0,  7 }, { 0,  8 }, { 0,  9 }, { 0, 10 } } }, //  4 left bottom
         { 5, { { 0,  0 }, { 0,  1 }, { 0,  2 }, { 0,  3 }, { 0,  4 } } }, //  5 left top
         { 2, { { 1,  5 }, { 2,  5 }, { 0,  0 }, { 0,  0 }, { 0,  0 } } }, //  6 middle left
         { 2, { { 7,  9 }, { 7, 10 }, { 0,  0 }, { 0,  0 }, { 0,  0 } } }, //  7 comma
         { 5, { { 0,  0 }, { 1,  1 }, { 1,  2 }, { 2,  3 }, { 2,  4 } } }, //  8 diag top left
         { 5, { { 3,  0 }, { 3,  1 }, { 3,  2 }, { 3,  3 }, { 3,  4 } } }, //  9 center top
         { 5, { { 6,  0 }, { 5,  1 }, { 5,  2 }, { 4,  3 }, { 4,  4 } } }, // 10 diag top right
         { 2, { { 4,  5 }, { 5,  5 }, { 0,  0 }, { 0,  0 }, { 0,  0 } } }, // 11 middle right
         { 5, { { 4,  6 }, { 4,  7 }, { 5,  8 }, { 5,  9 }, { 6, 10 } } }, // 12 diag bottom right
         { 5, { { 3,  6 }, { 3,  7 }, { 3,  8 }, { 3,  9 }, { 3, 10 } } }, // 13 center bottom
         { 5, { { 0, 10 }, { 2,  6 }, { 2,  7 }, { 1,  8 }, { 1,  9 } } }, // 14 diag bottom left
         { 1, { { 7, 10 }, { 0,  0 }, { 0,  0 }, { 0,  0 }, { 0,  0 } } }, // 15 dot
      } },
   { 6, 10,
      {
         { 5, { { 0,  0 }, { 1,  0 }, { 2,  0 }, { 3,  0 }, { 4,  0 } } }, //  0 top
         { 5, { { 4,  0 }, { 4,  1 }, { 4,  2 }, { 4,  3 }, { 4,  4 } } }, //  1 top right
         { 5, { { 4,  6 }, { 4,  7 }, { 4,  8 }, { 4,  9 }, { 4, 10 } } }, //  2 right bottom
         { 5, { { 0, 10 }, { 1, 10 }, { 2, 10 }, { 3, 10 }, { 4, 10 } } }, //  3 bottom
         { 5, { { 0,  6 }, { 0,  7 }, { 0,  8 }, { 0,  9 }, { 0, 10 } } }, //  4 left bottom
         { 5, { { 0,  0 }, { 0,  1 }, { 0,  2 }, { 0,  3 }, { 0,  4 } } }, //  5 left top
         { 3, { { 0,  5 }, { 1,  5 }, { 2,  5 }, { 0,  0 }, { 0,  0 } } }, //  6 middle left
         { 2, { { 5,  9 }, { 5, 10 }, { 0,  0 }, { 0,  0 }, { 0,  0 } } }, //  7 dot
         { 5, { { 0,  0 }, { 1,  1 }, { 1,  2 }, { 2,  3 }, { 2,  4 } } }, //  8 diag top left
         { 5, { { 2,  0 }, { 2,  1 }, { 2,  2 }, { 2,  3 }, { 2,  4 } } }, //  9 center top
         { 5, { { 4,  0 }, { 3,  1 }, { 3,  2 }, { 2,  3 }, { 2,  4 } } }, // 10 diag top right
         { 3, { { 2,  5 }, { 3,  5 }, { 4,  5 }, { 0,  0 }, { 0,  0 } } }, // 11 middle right
         { 5, { { 2,  6 }, { 2,  7 }, { 3,  8 }, { 3,  9 }, { 4, 10 } } }, // 12 diag bottom right
         { 5, { { 2,  6 }, { 2,  7 }, { 2,  8 }, { 2,  9 }, { 2, 10 } } }, // 13 center bottom
         { 5, { { 0, 10 }, { 2,  6 }, { 2,  7 }, { 1,  8 }, { 1,  9 } } }, // 14 diag bottom left
         { 1, { { 5, 10 }, { 0,  0 }, { 0,  0 }, { 0,  0 }, { 0,  0 } } }, // 15 comma
      } },
   // 16 Segments (split top/bottom)
   { 8, 10,
      {
         { 2, { { 1,  0 }, { 2,  0 }, { 0,  0 }, { 0,  0 }, { 0,  0 } } }, //  0 top left
         { 2, { { 4,  0 }, { 5,  0 }, { 0,  0 }, { 0,  0 }, { 0,  0 } } }, //  1 top right
         { 5, { { 6,  0 }, { 6,  1 }, { 6,  2 }, { 6,  3 }, { 6,  4 } } }, //  2 right top
         { 5, { { 6,  6 }, { 6,  7 }, { 6,  8 }, { 6,  9 }, { 6, 10 } } }, //  3 right bottom
         { 2, { { 4, 10 }, { 5, 10 }, { 0,  0 }, { 0,  0 }, { 0,  0 } } }, //  5 bottom right
         { 2, { { 1, 10 }, { 2, 10 }, { 0,  0 }, { 0,  0 }, { 0,  0 } } }, //  4 bottom left
         { 5, { { 0,  6 }, { 0,  7 }, { 0,  8 }, { 0,  9 }, { 0, 10 } } }, //  6 left bottom
         { 5, { { 0,  0 }, { 0,  1 }, { 0,  2 }, { 0,  3 }, { 0,  4 } } }, //  7 left top
         { 5, { { 0,  0 }, { 1,  1 }, { 1,  2 }, { 2,  3 }, { 2,  4 } } }, //  8 diag top left
         { 5, { { 3,  0 }, { 3,  1 }, { 3,  2 }, { 3,  3 }, { 3,  4 } } }, //  9 center top
         { 5, { { 6,  0 }, { 5,  1 }, { 5,  2 }, { 4,  3 }, { 4,  4 } } }, // 10 diag top right
         { 2, { { 4,  5 }, { 5,  5 }, { 0,  0 }, { 0,  0 }, { 0,  0 } } }, // 11 middle right
         { 5, { { 4,  6 }, { 4,  7 }, { 5,  8 }, { 5,  9 }, { 6, 10 } } }, // 12 diag bottom right
         { 5, { { 3,  6 }, { 3,  7 }, { 3,  8 }, { 3,  9 }, { 3, 10 } } }, // 13 center bottom
         { 5, { { 0, 10 }, { 2,  6 }, { 2,  7 }, { 1,  8 }, { 1,  9 } } }, // 14 diag bottom left
         { 2, { { 1,  5 }, { 2,  5 }, { 0,  0 }, { 0,  0 }, { 0,  0 } } }, // 15 middle left
      } },
   // 9 Segments + comma
   { 8, 10,
      {
         { 5, { { 1,  0 }, { 2,  0 }, { 3,  0 }, { 4,  0 }, { 5,  0 } } }, // 0 top
         { 5, { { 6,  0 }, { 6,  1 }, { 6,  2 }, { 6,  3 }, { 6,  4 } } }, // 1 right top
         { 5, { { 6,  6 }, { 6,  7 }, { 6,  8 }, { 6,  9 }, { 6, 10 } } }, // 2 right bottom
         { 5, { { 1, 10 }, { 2, 10 }, { 3, 10 }, { 4, 10 }, { 5, 10 } } }, // 3 bottom
         { 5, { { 0,  6 }, { 0,  7 }, { 0,  8 }, { 0,  9 }, { 0, 10 } } }, // 4 left bottom
         { 5, { { 0,  0 }, { 0,  1 }, { 0,  2 }, { 0,  3 }, { 0,  4 } } }, // 5 left top
         { 5, { { 1,  5 }, { 2,  5 }, { 3,  5 }, { 4,  5 }, { 5,  5 } } }, // 6 middle
         { 2, { { 7,  9 }, { 7, 10 }, { 0,  0 }, { 0,  0 }, { 0,  0 } } }, // 7 comma
         { 5, { { 3,  0 }, { 3,  1 }, { 3,  2 }, { 3,  3 }, { 3,  4 } } }, // 8 center top
         { 5, { { 3,  6 }, { 3,  7 }, { 3,  8 }, { 3,  9 }, { 3, 10 } } }, // 9 center bottom
      } },
   { 8, 6,
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
   { 8, 10,
      {
         { 5, { { 1,  0 }, { 2,  0 }, { 3,  0 }, { 4,  0 }, { 5,  0 } } }, // 0 top
         { 5, { { 6,  0 }, { 6,  1 }, { 6,  2 }, { 6,  3 }, { 6,  4 } } }, // 1 right top
         { 5, { { 6,  6 }, { 6,  7 }, { 6,  8 }, { 6,  9 }, { 6, 10 } } }, // 2 right bottom
         { 5, { { 1, 10 }, { 2, 10 }, { 3, 10 }, { 4, 10 }, { 5, 10 } } }, // 3 bottom
         { 5, { { 0,  6 }, { 0,  7 }, { 0,  8 }, { 0,  9 }, { 0, 10 } } }, // 4 left bottom
         { 5, { { 0,  0 }, { 0,  1 }, { 0,  2 }, { 0,  3 }, { 0,  4 } } }, // 5 left top
         { 5, { { 1,  5 }, { 2,  5 }, { 3,  5 }, { 4,  5 }, { 5,  5 } } }, // 6 middle
         { 1, { { 7, 10 }, { 0,  0 }, { 0,  0 }, { 0,  0 }, { 0,  0 } } }, // 7 dot
         { 5, { { 3,  0 }, { 3,  1 }, { 3,  2 }, { 3,  3 }, { 3,  4 } } }, // 8 center top
         { 5, { { 3,  6 }, { 3,  7 }, { 3,  8 }, { 3,  9 }, { 3, 10 } } }, // 9 center bottom
      } },
};

static void DrawChar(const int x, const int y, const segDisplay& display, const float* const lum, const int nSeg)
{
   for (int seg = 0; seg < nSeg; seg++)
   {
      const uint8_t v = static_cast<uint8_t>((lum[seg] < 0.01f ? 0.01f : lum[seg] > 1.f ? 1.f : lum[seg]) * 255.f);
      for (int i = 0; i < display.segs[seg].nDots; i++)
      {
         const int px = x + display.segs[seg].dots[i][0];
         const int py = y + display.segs[seg].dots[i][1];
         int w = renderFrame[py * 128 + px] + v;
         renderFrame[py * 128 + px] = w < 255 ? w : 255;
      }
   }
}

static void DrawDisplay(int x, int y, float*& lum, int srcIndex, bool large)
{
   SegSrcId& segSrc = selectedSources[srcIndex];
   for (unsigned int i = 0; i < segSrc.nElements; i++)
   {
      const SegElementType type = segSrc.elementType[i];
      SegImgs img = SegImg_Invalid;
      switch (type)
      {
      case CTLPI_SEG_LAYOUT_7:    img = large ? SegImg_Seg9C_8x10   : SegImg_Seg9C_8x6;    break;
      case CTLPI_SEG_LAYOUT_7C:   img = large ? SegImg_Seg9C_8x10   : SegImg_Seg9C_8x6;    break;
      case CTLPI_SEG_LAYOUT_7D:   img =         SegImg_Seg9D_8x10;                         break;
      case CTLPI_SEG_LAYOUT_9:    img = large ? SegImg_Seg9C_8x10   : SegImg_Seg9C_8x6;    break;
      case CTLPI_SEG_LAYOUT_9C:   img = large ? SegImg_Seg9C_8x10   : SegImg_Seg9C_8x6;    break;
      case CTLPI_SEG_LAYOUT_14:   img = large ? SegImg_Seg14DC_8x10 : SegImg_Seg14DC_6x10; break;
      case CTLPI_SEG_LAYOUT_14D:  img = large ? SegImg_Seg14DC_8x10 : SegImg_Seg14DC_6x10; break;
      case CTLPI_SEG_LAYOUT_14DC: img = large ? SegImg_Seg14DC_8x10 : SegImg_Seg14DC_6x10; break;
      case CTLPI_SEG_LAYOUT_16:   img =         SegImg_Seg16_8x10;                         break;
      default: assert(false); return; break;
      }
      DrawChar(x, y, segDisplays[img], lum, nSegments[type]);
      x += segDisplays[img].width;
      lum += 16;
   }
}

#ifdef _WIN32
void SetThreadName(const std::string& name)
{
   int size_needed = MultiByteToWideChar(CP_UTF8, 0, name.c_str(), -1, NULL, 0);
   if (size_needed == 0)
      return;
   std::wstring wstr(size_needed, 0);
   if (MultiByteToWideChar(CP_UTF8, 0, name.c_str(), -1, wstr.data(), size_needed) == 0)
      return;
   HRESULT hr = SetThreadDescription(GetCurrentThread(), wstr.c_str());
}
#else
void SetThreadName(const std::string& name) { }
#endif   

static void RenderThread()
{
   SetThreadName("AlphaDMD.RenderThread");
   uint16_t seg_data[128] = { 0 };
   uint16_t seg_data2[128] = { 0 };
   float groupLum[128 * 16] = { 0 };
   std::vector<unsigned int> lastFrameId;
   while (isRunning)
   {
      std::unique_lock<std::mutex> lock(sourceMutex);
      updateCondVar.wait(lock);

      if (!isRunning)
         break;

      if (selectedSources.empty() || (dmdLayout == DmdLayouts::Undefined))
         continue;

      // Get segment display state and compute backward compatible binary version
      float* lum = groupLum;
      bool changed = false;
      lastFrameId.resize(selectedSources.size());
      for (size_t i = 0, pos = 0; i < selectedSources.size(); i++)
      {
         const SegDisplayFrame seg = selectedSources[i].GetState(selectedSources[i].id);
         if (seg.frameId != lastFrameId[i])
         {
            changed = true;
            lastFrameId[i] = seg.frameId;
            memcpy(lum, seg.frame, selectedSources[i].nElements * 16 * sizeof(float));
            for (unsigned int j = 0; j < selectedSources[i].nElements; j++)
            {
               const int nSegs = nSegments[selectedSources[i].elementType[j]];
               seg_data[pos + j] = 0;
               for (int k = 0; k < nSegs; k++)
                  if (seg.frame[j * 16 + k] > 0.5f)
                     seg_data[pos + j] |= 1 << k;
            }
         }
         lum += selectedSources[i].nElements * 16;
         pos += selectedSources[i].nElements;
      }
      if (!changed)
         continue;

      // Render for DMD
      lum = groupLum;
      memset(renderFrame, 0, sizeof(renderFrame));
      switch (dmdLayout)
      {
      case Layout_6x4_2x2: // S11 Bowl games
         DrawDisplay( 0,  0, lum, 0, true);
         DrawDisplay(80,  0, lum, 1, true);
         DrawDisplay( 0, 11, lum, 2, true);
         DrawDisplay(80, 11, lum, 3, true);
         DrawDisplay( 0, 22, lum, 4, true);
         DrawDisplay(80, 22, lum, 5, true);
         DrawDisplay(56, 11, lum, 6, true);
         DrawDisplay(56, 22, lum, 7, true);
         break;
      case Layout_4x6_2x2: // Lots of games (4 players + credit/ball)
         DrawDisplay( 0,  0, lum, 0, true);
         DrawDisplay(80,  0, lum, 1, true);
         DrawDisplay( 0, 12, lum, 2, true);
         DrawDisplay(80, 12, lum, 3, true);
         DrawDisplay( 8, 24, lum, 4, false);
         DrawDisplay(32, 24, lum, 5, false);
         break;
      case Layout_4x6_2x2_1x6: // Black Hole
         DrawDisplay( 0,  0, lum, 0, false);
         DrawDisplay(80,  0, lum, 1, false);
         DrawDisplay( 0, 12, lum, 2, false);
         DrawDisplay(80, 12, lum, 3, false);
         DrawDisplay(56,  0, lum, 4, false);
         DrawDisplay(56, 12, lum, 5, false);
         DrawDisplay(40, 24, lum, 6, false);
         break;
      case Layout_4x7:
         DrawDisplay( 0,  2, lum, 0, true);
         DrawDisplay(72,  2, lum, 1, true);
         DrawDisplay( 0, 19, lum, 2, true);
         DrawDisplay(72, 19, lum, 3, true);
         break;
      case Layout_4x7_2x2: // Lots of games (4 players + credit/ball)
         DrawDisplay( 0,  0, lum, 0, true);
         DrawDisplay(72,  0, lum, 1, true);
         DrawDisplay( 0, 12, lum, 2, true);
         DrawDisplay(72, 12, lum, 3, true);
         DrawDisplay( 8, 24, lum, 4, false);
         DrawDisplay(32, 24, lum, 5, false);
         break;
      case Layout_4x7_5x2: // Medusa
         DrawDisplay(  0,  0, lum, 0, true);
         DrawDisplay( 72,  0, lum, 1, true);
         DrawDisplay(  0, 12, lum, 2, true);
         DrawDisplay( 72, 12, lum, 3, true);
         DrawDisplay( 16, 24, lum, 4, false);
         DrawDisplay( 40, 24, lum, 5, false);
         DrawDisplay( 64, 24, lum, 6, false);
         DrawDisplay( 88, 24, lum, 7, false);
         DrawDisplay(112, 24, lum, 8, false);
         break;
      case Layout_2x16: // Lots of later games
         DrawDisplay( 0,  2, lum, 0, true);
         DrawDisplay( 0, 19, lum, 1, true);
         break;
      case Layout_1x7_2x16: // Police Force
         DrawDisplay(68,  1, lum, 0, false);
         DrawDisplay( 0,  9, lum, 1, true);
         DrawDisplay( 0, 21, lum, 2, true);
         break;
      case Layout_2x16_1x7: // Taxi
         DrawDisplay( 0,  9, lum, 0, true);
         DrawDisplay( 0, 21, lum, 1, true);
         DrawDisplay(68,  1, lum, 2, false);
         break;
      case Layout_1x7_1x4_2x16: // Riverboat Gambler
         DrawDisplay( 0,  1, lum, 0, false);
         DrawDisplay(96,  1, lum, 1, false);
         DrawDisplay( 0,  9, lum, 2, true);
         DrawDisplay( 0, 21, lum, 3, true);
         break;
      case Layout_2x7_2x2_1x16: // Hyperball
         DrawDisplay( 0,  0, lum, 0, true);
         DrawDisplay(72,  0, lum, 1, true);
         DrawDisplay(16, 12, lum, 2, false);
         DrawDisplay(40, 12, lum, 3, false);
         DrawDisplay(16, 21, lum, 4, true);
         break;
      case Layout_2x20: // Lots of later games
         DrawDisplay( 4,  2, lum, 0, false);
         DrawDisplay( 4, 19, lum, 1, false);
         break;
      default: break;
      }
      if (memcmp(dmd128Frame, renderFrame, 128 * 32) != 0)
      {
         //std::lock_guard<std::mutex> lock(renderMutex);
         memcpy(dmd128Frame, renderFrame, 128 * 32);
         for (int y = 0; y < 64; y++)
            for (int x = 0; x < 256; x++)
               dmd256Frame[x + y * 256] = dmd128Frame[(x >> 1) + (y >> 1) * 128];
         renderFrameId++;
      }
      
      // Render to bitplane surface for frame identification (backward compatible way of rendering to avoid breaking existing colorizations)
      memset(AlphaNumericFrameBuffer, 0, sizeof(AlphaNumericFrameBuffer));
      const SegElementType firstType = selectedSources[0].elementType[0];
      switch (dmdLayout)
      {
      case Layout_6x4_2x2: _6x4Num_4x1Num(seg_data); break;
      case Layout_4x6_2x2:
         if ((firstType == CTLPI_SEG_LAYOUT_9) || (firstType == CTLPI_SEG_LAYOUT_9C))
            _2x6Num10_2x6Num10_4x1Num(seg_data);
         else
            _2x6Num_2x6Num_4x1Num(seg_data);
         break;
      case Layout_4x6_2x2_1x6: return; // Unsupported
      case Layout_4x7:
         _2x7Alpha_2x7Num(seg_data); break;
         _4x7Num10(seg_data); break;
      case Layout_4x7_2x2:
         if ((firstType == CTLPI_SEG_LAYOUT_9) || (firstType == CTLPI_SEG_LAYOUT_9C))
            _2x7Num10_2x7Num10_4x1Num(seg_data);
         else if ((firstType == CTLPI_SEG_LAYOUT_14) || (firstType == CTLPI_SEG_LAYOUT_14D) || (firstType == CTLPI_SEG_LAYOUT_14DC))
            _2x7Alpha_2x7Num_4x1Num(seg_data);
         else if (selectedSources[0].elementType[2] == CTLPI_SEG_LAYOUT_7) // No thousands comma
            _2x7Num_2x7Num_4x1Num(seg_data);
         else // With thousands comma
            _2x7Num_2x7Num_4x1Num_gen7(seg_data);
         break;
      case Layout_4x7_5x2:      _2x7Num_2x7Num_10x1Num(seg_data,seg_data2); break; // FIXME Medusa: seg_data2 is not initialized. Is this really needed ?
      case Layout_2x16:         _2x16Alpha(seg_data); break;
      case Layout_1x7_2x16:     _1x7Num_1x16Alpha_1x16Num(seg_data); break;
      case Layout_2x16_1x7:     _1x16Alpha_1x16Num_1x7Num(seg_data); break;
      case Layout_1x7_1x4_2x16: _1x16Alpha_1x16Num_1x7Num_1x4Num(seg_data); break; // FIXME Riverboat Gambler: reverse order
      case Layout_2x7_2x2_1x16: _2x7Num_4x1Num_1x16Alpha(seg_data); break;
      case Layout_2x20:         _2x20Alpha(seg_data); break;
      default: break;
      }
      if (memcmp(identifyFrame, AlphaNumericFrameBuffer, sizeof(AlphaNumericFrameBuffer)) != 0) {
         //std::lock_guard<std::mutex> lock(renderMutex);
         memcpy(identifyFrame, AlphaNumericFrameBuffer, sizeof(AlphaNumericFrameBuffer));
         identifyFrameId++;
      }
   }
   isRunning = false;
}

static DisplayFrame GetRenderFrame(const CtlResId id) 
{
   // TODO To be fully clean we should do a lock on sourceLock and return a copy of the render
   //std::lock_guard<std::mutex> lock(renderMutex);
   updateCondVar.notify_one();
   return { renderFrameId, id.resId == 0 ? dmd128Frame : dmd256Frame };
}

static DisplayFrame GetIdentifyFrame(const CtlResId id)
{
   // TODO To be fully clean we should do a lock on sourceLock and return a copy of the render
   //std::lock_guard<std::mutex> lock(renderMutex);
   updateCondVar.notify_one();
   return { identifyFrameId, identifyFrame };
}

static void OnGetDisplaySrc(const unsigned int eventId, void* userData, void* msgData)
{
   if (selectedSources.empty() || (dmdLayout == DmdLayouts::Undefined))
      return;
   GetDisplaySrcMsg& msg = *static_cast<GetDisplaySrcMsg*>(msgData);
   if (msg.count < msg.maxEntryCount)
      msg.entries[msg.count] = dmd128Id;
   msg.count++;
   if (msg.count < msg.maxEntryCount)
      msg.entries[msg.count] = dmd256Id;
   msg.count++;
}

static void OnSegSrcChanged(const unsigned int eventId, void* userData, void* msgData)
{
   std::lock_guard<std::mutex> lock(sourceMutex);
   bool wasRendering = !selectedSources.empty();
   selectedSources.clear();

   // Update list of segment sources and select sources (simply the first group for the time being, maybe we could have some user setup on this)
   GetSegSrcMsg getSrcMsg = { 0, 0, nullptr };
   msgApi->BroadcastMsg(endpointId, getSegSrcId, &getSrcMsg);
   if (getSrcMsg.count > 0)
   {
      getSrcMsg = { getSrcMsg.count, 0, new SegSrcId[getSrcMsg.count] };
      msgApi->BroadcastMsg(endpointId, getSegSrcId, &getSrcMsg);
      for (unsigned int i = 0; i < getSrcMsg.count; i++)
      {
         if (getSrcMsg.entries[i].groupId.id == getSrcMsg.entries[0].groupId.id)
         {
            selectedSources.push_back(getSrcMsg.entries[i]);
         }
      }
   }

   dmdLayout = DmdLayouts::Undefined;
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
   for (int i = 0; (dmdLayout == DmdLayouts::Undefined) && (i < 12); i++)
   {
      if (layouts[i][1] == selectedSources.size())
      {
         dmdLayout = static_cast<DmdLayouts>(layouts[i][0]);
         for (size_t j = 0; j < selectedSources.size(); j++)
         {
            if (layouts[i][j + 2] != selectedSources[j].nElements)
            {
               dmdLayout = DmdLayouts::Undefined;
               break;
            }
         }
      }
   }
   if (dmdLayout == DmdLayouts::Undefined)
   {
      std::stringstream ss;
      ss << "Unsupported segment layout (" << selectedSources.size() << " displays: ";
      for (size_t i = 0; i < selectedSources.size(); i++)
         ss << (i == 0 ? "" : ", ") << selectedSources[i].nElements;
      ss << ')';
      LPI_LOGI("%s", ss.str().c_str());
   }

   // If we are starting or stopping rendering, report it
   if (wasRendering != (!selectedSources.empty()))
   {
      if (selectedSources.empty())
         msgApi->UnsubscribeMsg(getDmdSrcId, OnGetDisplaySrc);
      else
         msgApi->SubscribeMsg(endpointId, getDmdSrcId, OnGetDisplaySrc, nullptr);
      msgApi->BroadcastMsg(endpointId, onDmdSrcChangedId, nullptr);
   }
}

}

using namespace AlphaDMD;

MSGPI_EXPORT void MSGPIAPI AlphaDMDPluginLoad(const uint32_t sessionId, MsgPluginAPI* api)
{
   msgApi = api;
   endpointId = sessionId;
   dmd128Id = {
      .id = { endpointId, 0 },
      .groupId = { endpointId, 0 },
      .overrideId = { 0, 0 },
      .width = 128,
      .height = 32,
      .hardware = CTLPI_DISPLAY_HARDWARE_UNKNOWN,
      .frameFormat = CTLPI_DISPLAY_FORMAT_LUM8,
      .GetRenderFrame = &GetRenderFrame,
      .identifyFormat = CTLPI_DISPLAY_ID_FORMAT_BITPLANE2,
      .GetIdentifyFrame = &GetIdentifyFrame
   };
   dmd256Id = {
      .id = { endpointId, 1 },
      .groupId = { endpointId, 0 },
      .overrideId = { 0, 0 },
      .width = 256,
      .height = 64,
      .hardware = CTLPI_DISPLAY_HARDWARE_UNKNOWN,
      .frameFormat = CTLPI_DISPLAY_FORMAT_LUM8,
      .GetRenderFrame = &GetRenderFrame
   };
   isRunning = true;
   renderThread = std::thread(RenderThread);
   onDmdSrcChangedId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DISPLAY_ON_SRC_CHG_MSG);
   getDmdSrcId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DISPLAY_GET_SRC_MSG);
   onSegSrcChangedId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_SEG_ON_SRC_CHG_MSG);
   getSegSrcId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_SEG_GET_SRC_MSG);
   msgApi->SubscribeMsg(endpointId, onSegSrcChangedId, OnSegSrcChanged, nullptr);
   OnSegSrcChanged(onSegSrcChangedId, nullptr, nullptr);
}

MSGPI_EXPORT void MSGPIAPI AlphaDMDPluginUnload()
{
   isRunning = false;
   updateCondVar.notify_all();
   if (renderThread.joinable())
      renderThread.join();
   if (!selectedSources.empty())
      msgApi->UnsubscribeMsg(getDmdSrcId, OnGetDisplaySrc);
   msgApi->UnsubscribeMsg(onSegSrcChangedId, OnSegSrcChanged);
   msgApi->ReleaseMsgID(onDmdSrcChangedId);
   msgApi->ReleaseMsgID(getDmdSrcId);
   msgApi->ReleaseMsgID(onSegSrcChangedId);
   msgApi->ReleaseMsgID(getSegSrcId);
   msgApi = nullptr;
}
