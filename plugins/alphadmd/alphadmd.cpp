// license:GPLv3+

#include "MsgPlugin.h"
#include "CorePlugin.h"
#include "LoggingPlugin.h"

#include <cstring>
#include <cstdint>
#include <sstream>
#include <assert.h>
#include <stdarg.h>

///////////////////////////////////////////////////////////////////////////////
//
// AlphaDMD plugin: generate DMD from alphanumeric segment displays
//
// This plugin only rely on the generic messaging plugin API and the core DMD
// and segment API. It listen for alphanumeric source and, when found, broadcast
// corresponding DMD sources (128x32 and 256x64 variants)

MsgPluginAPI* msgApi = nullptr;
uint32_t endpointId;
unsigned int onDmdSrcChangedId, getDmdSrcId, getRenderDmdId;
unsigned int onSegSrcChangedId, getSegSrcId, getSegId;

GetSegSrcMsg segSources;
int nSelectedSources = -1;
int selectedSource[32];
DmdSrcId dmd128Id, dmd256Id;
uint8_t dmd128Frame[128 * 32];
uint8_t dmd256Frame[256 * 64];
unsigned int frameId = 0;

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
DmdLayouts dmdLayout = DmdLayouts::Undefined;

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

static const segDisplay segDisplays[6] = {
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

void DrawChar(const int x, const int y, const segDisplay& display, const float* lum, const int nSeg)
{
   for (int seg = 0; seg < nSeg; seg++)
   {
      uint8_t v = static_cast<uint8_t>((lum[seg] < 0.01f ? 0.01f : lum[seg] > 1.f ? 1.f : lum[seg]) * 255.f);
      for (int i = 0; i < display.segs[seg].nDots; i++)
      {
         const int px = x + display.segs[seg].dots[i][0];
         const int py = y + display.segs[seg].dots[i][1];
         int w = dmd128Frame[py * 128 + px] + v;
         dmd128Frame[py * 128 + px] = w < 255 ? w : 255;
      }
   }
}

void DrawDisplay(int x, int y, int srcIndex, bool large)
{
   SegSrcId& segSrc = segSources.entries[srcIndex];
   GetSegMsg getSegMsg { { segSrc.id.endpointId, segSrc.id.resId } };
   msgApi->BroadcastMsg(endpointId, getSegId, &getSegMsg);
   if (getSegMsg.frame == nullptr)
      return;
   float* lum = getSegMsg.frame;
   for (int i = 0; i < segSrc.nElements; i++)
   {
      SegElementType type = segSrc.elementType[i];
      SegImgs img = SegImg_Invalid;
      switch (segSrc.elementType[i])
      {
      case CTLPI_GETSEG_LAYOUT_7:    img = large ? SegImg_Seg9C_8x10   : SegImg_Seg9C_8x6;    break;
      case CTLPI_GETSEG_LAYOUT_7C:   img = large ? SegImg_Seg9C_8x10   : SegImg_Seg9C_8x6;    break;
      case CTLPI_GETSEG_LAYOUT_7D:   img =         SegImg_Seg9D_8x10;                         break;
      case CTLPI_GETSEG_LAYOUT_9:    img = large ? SegImg_Seg9C_8x10   : SegImg_Seg9C_8x6;    break;
      case CTLPI_GETSEG_LAYOUT_9C:   img = large ? SegImg_Seg9C_8x10   : SegImg_Seg9C_8x6;    break;
      case CTLPI_GETSEG_LAYOUT_14:   img = large ? SegImg_Seg14DC_8x10 : SegImg_Seg14DC_6x10; break;
      case CTLPI_GETSEG_LAYOUT_14D:  img = large ? SegImg_Seg14DC_8x10 : SegImg_Seg14DC_6x10; break;
      case CTLPI_GETSEG_LAYOUT_14DC: img = large ? SegImg_Seg14DC_8x10 : SegImg_Seg14DC_6x10; break;
      case CTLPI_GETSEG_LAYOUT_16:   img =         SegImg_Seg16_8x10;                         break;
      default: assert(false); break;
      }
      static int nSegments[] = { 7, 8, 8, 9, 10, 14, 15, 16, 16 };
      DrawChar(x, y, segDisplays[img], lum, nSegments[segSrc.elementType[i]]);
      x += segDisplays[img].width;
      lum += 16;
   }
}

void onGetRenderDMDSrc(const unsigned int eventId, void* userData, void* msgData)
{
   if ((nSelectedSources <= 0) || (dmdLayout == DmdLayouts::Undefined))
      return;
   GetDmdSrcMsg& msg = *static_cast<GetDmdSrcMsg*>(msgData);
   if (msg.count < msg.maxEntryCount)
   {
      msg.entries[msg.count] = dmd128Id;
      msg.count++;
   }
   if (msg.count < msg.maxEntryCount)
   {
      msg.entries[msg.count] = dmd256Id;
      msg.count++;
   }
}

void onGetRenderDMD(const unsigned int eventId, void* userData, void* msgData)
{
   if ((nSelectedSources <= 0) || (dmdLayout == DmdLayouts::Undefined))
      return;
   GetDmdMsg& getDmdMsg = *static_cast<GetDmdMsg*>(msgData);
   if ((getDmdMsg.frame != nullptr) || (getDmdMsg.dmdId.id.endpointId != endpointId))
      return;

   // Note that we could update only on display changes
   memset(dmd128Frame, 0, sizeof(dmd128Frame));
   memset(dmd256Frame, 0, sizeof(dmd256Frame));
   switch (dmdLayout)
   {
   case Layout_6x4_2x2: // S11 Bowl games
      DrawDisplay( 0,  0, selectedSource[0], true);
      DrawDisplay(80,  0, selectedSource[1], true);
      DrawDisplay( 0, 11, selectedSource[2], true);
      DrawDisplay(80, 11, selectedSource[3], true);
      DrawDisplay( 0, 22, selectedSource[4], true);
      DrawDisplay(80, 22, selectedSource[5], true);
      DrawDisplay(56, 11, selectedSource[6], true);
      DrawDisplay(56, 22, selectedSource[7], true);
      break;
   case Layout_4x6_2x2: // Lots of games (4 players + credit/ball)
      DrawDisplay( 0,  0, selectedSource[0], true);
      DrawDisplay(80,  0, selectedSource[1], true);
      DrawDisplay( 0, 12, selectedSource[2], true);
      DrawDisplay(80, 12, selectedSource[3], true);
      DrawDisplay( 8, 24, selectedSource[4], false);
      DrawDisplay(32, 24, selectedSource[5], false);
      break;
   case Layout_4x6_2x2_1x6: // Black Hole
      DrawDisplay( 0,  0, selectedSource[0], false);
      DrawDisplay(80,  0, selectedSource[1], false);
      DrawDisplay( 0, 12, selectedSource[2], false);
      DrawDisplay(80, 12, selectedSource[3], false);
      DrawDisplay(56,  0, selectedSource[4], false);
      DrawDisplay(56, 12, selectedSource[5], false);
      DrawDisplay(40, 24, selectedSource[6], false);
      break;
   case Layout_4x7:
      DrawDisplay( 0,  2, selectedSource[0], true);
      DrawDisplay(72,  2, selectedSource[1], true);
      DrawDisplay( 0, 19, selectedSource[2], true);
      DrawDisplay(72, 19, selectedSource[3], true);
      break;
   case Layout_4x7_2x2: // Lots of games (4 players + credit/ball)
      DrawDisplay( 0,  0, selectedSource[0], true);
      DrawDisplay(72,  0, selectedSource[1], true);
      DrawDisplay( 0, 12, selectedSource[2], true);
      DrawDisplay(72, 12, selectedSource[3], true);
      DrawDisplay( 8, 24, selectedSource[4], false);
      DrawDisplay(32, 24, selectedSource[5], false);
      break;
   case Layout_4x7_5x2: // Medusa
      DrawDisplay(  0,  0, selectedSource[0], true);
      DrawDisplay( 72,  0, selectedSource[1], true);
      DrawDisplay(  0, 12, selectedSource[2], true);
      DrawDisplay( 72, 12, selectedSource[3], true);
      DrawDisplay( 16, 24, selectedSource[4], false);
      DrawDisplay( 40, 24, selectedSource[5], false);
      DrawDisplay( 64, 24, selectedSource[6], false);
      DrawDisplay( 88, 24, selectedSource[7], false);
      DrawDisplay(112, 24, selectedSource[8], false);
      break;
   case Layout_2x16: // Lots of later games
      DrawDisplay(0,  2, selectedSource[0], true);
      DrawDisplay(0, 19, selectedSource[1], true);
      break;
   case Layout_1x7_2x16: // Police Force
      DrawDisplay(68,  1, selectedSource[0], false);
      DrawDisplay( 0,  9, selectedSource[1], true);
      DrawDisplay( 0, 21, selectedSource[2], true);
      break;
   case Layout_2x16_1x7: // Taxi
      DrawDisplay( 0,  9, selectedSource[0], true);
      DrawDisplay( 0, 21, selectedSource[1], true);
      DrawDisplay(68,  1, selectedSource[2], false);
      break;
   case Layout_1x7_1x4_2x16: // Riverboat Gambler
      DrawDisplay( 0,  1, selectedSource[0], false);
      DrawDisplay(96,  1, selectedSource[1], false);
      DrawDisplay( 0,  9, selectedSource[2], true);
      DrawDisplay( 0, 21, selectedSource[3], true);
      break;
   case Layout_2x7_2x2_1x16: // Hyperball
      DrawDisplay( 0,  0, selectedSource[0], true);
      DrawDisplay(72,  0, selectedSource[1], true);
      DrawDisplay(16, 12, selectedSource[2], false);
      DrawDisplay(40, 12, selectedSource[3], false);
      DrawDisplay(16, 21, selectedSource[4], true);
      break;
   case Layout_2x20: // Lots of later games
      DrawDisplay(4,  2, selectedSource[0], false);
      DrawDisplay(4, 19, selectedSource[1], false);
      break;
   default: break;
   }

   if (memcmp(&getDmdMsg.dmdId, &dmd128Id, sizeof(DmdSrcId)) == 0)
   {
      getDmdMsg.frameId = frameId;
      getDmdMsg.frame = dmd128Frame;
   }
   else if (memcmp(&getDmdMsg.dmdId, &dmd256Id, sizeof(DmdSrcId)) == 0)
   {
      for (int y = 0; y < 64; y++)
         for (int x = 0; x < 256; x++)
            dmd256Frame[x + y * 256] = dmd128Frame[(x >> 1) + (y >> 1) * 128];
      getDmdMsg.frameId = frameId;
      getDmdMsg.frame = dmd256Frame;
   }
   frameId++;
}

void UpdateSegSources()
{
   bool wasRendering = nSelectedSources > 0;

   // Update list of segment sources and select sources (simply the first one for the time being, maybe we oculd have some user setup on this)
   segSources.count = 0;
   msgApi->BroadcastMsg(endpointId, getSegSrcId, &segSources);
   if (segSources.count == 0)
   {
      nSelectedSources = -1;
   }
   else
   {
      unsigned int selectedEndpointId = segSources.entries[0].id.endpointId;

      // Select a DMD layout
      nSelectedSources = 0;
      for (int i = 0; (nSelectedSources < 32) && (i < segSources.count); i++)
      {
         if (segSources.entries[i].id.endpointId == selectedEndpointId)
         {
            selectedSource[nSelectedSources] = i;
            nSelectedSources++;
         }
      }
      int layouts[12][16] = {
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
         { DmdLayouts::Layout_4x7_5x2, 14, 7, 7, 7, 7, 2, 2, 2, 2, 2 }, // Medusa
         { DmdLayouts::Layout_4x6_2x2_1x6, 7, 6, 6, 6, 6, 2, 2, 6 }, // Black Hole
      };
      dmdLayout = DmdLayouts::Undefined;
      for (int i = 0; (dmdLayout == DmdLayouts::Undefined) && (i < 12); i++)
      {
         if (layouts[i][1] == nSelectedSources)
         {
            dmdLayout = static_cast<DmdLayouts>(layouts[i][0]);
            for (int j = 0; j < nSelectedSources; j++)
            {
               if (layouts[i][j + 2] != segSources.entries[selectedSource[j]].nElements)
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
         ss << "Unsupported segment layout (" << nSelectedSources << " displays: ";
         for (int i = 0; i < nSelectedSources; i++)
            ss << (i == 0 ? "" : ", ") << segSources.entries[selectedSource[i]].nElements;
         ss << ")";
         LPI_LOGI("%s", ss.str().c_str());
      }
   }

   // If we are starting or stopping rendering, report it
   if (wasRendering != (nSelectedSources > 0))
   {
      if (nSelectedSources > 0)
      {
         msgApi->SubscribeMsg(endpointId, getDmdSrcId, onGetRenderDMDSrc, nullptr);
         msgApi->SubscribeMsg(endpointId, getRenderDmdId, onGetRenderDMD, nullptr);
      }
      else
      {
         msgApi->UnsubscribeMsg(getDmdSrcId, onGetRenderDMDSrc);
         msgApi->UnsubscribeMsg(getRenderDmdId, onGetRenderDMD);
      }
      msgApi->BroadcastMsg(endpointId, onDmdSrcChangedId, nullptr);
   }
}

void onSegSrcChanged(const unsigned int eventId, void* userData, void* msgData) { UpdateSegSources(); }

MSGPI_EXPORT void MSGPIAPI PluginLoad(const uint32_t sessionId, MsgPluginAPI* api)
{
   msgApi = api;
   endpointId = sessionId;
   segSources = { 1024, 0, new SegSrcId[1024] };
   dmd128Id = { { endpointId, 0 }, 128, 32, CTLPI_GETDMD_FORMAT_LUM8 };
   dmd256Id = { { endpointId, 1 }, 256, 64, CTLPI_GETDMD_FORMAT_LUM8 };
   onDmdSrcChangedId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_ONDMD_SRC_CHG_MSG);
   getDmdSrcId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_GETDMD_SRC_MSG);
   getRenderDmdId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_GETDMD_RENDER_MSG);
   onSegSrcChangedId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_ONSEG_SRC_CHG_MSG);
   getSegSrcId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_GETSEG_SRC_MSG);
   getSegId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_GETSEG_MSG);
   msgApi->SubscribeMsg(endpointId, onSegSrcChangedId, onSegSrcChanged, nullptr);
   UpdateSegSources();
}

MSGPI_EXPORT void MSGPIAPI PluginUnload()
{
   if (segSources.count > 0)
   {
      msgApi->UnsubscribeMsg(getDmdSrcId, onGetRenderDMDSrc);
      msgApi->UnsubscribeMsg(getRenderDmdId, onGetRenderDMD);
   }
   msgApi->UnsubscribeMsg(onSegSrcChangedId, onSegSrcChanged);
   msgApi->ReleaseMsgID(onDmdSrcChangedId);
   msgApi->ReleaseMsgID(getDmdSrcId);
   msgApi->ReleaseMsgID(getRenderDmdId);
   msgApi->ReleaseMsgID(onSegSrcChangedId);
   msgApi->ReleaseMsgID(getSegSrcId);
   msgApi->ReleaseMsgID(getSegId);
   delete[] segSources.entries;
   msgApi = nullptr;
}
