// license:GPLv3+

#include "core/stdafx.h"
#include "../../vpx-test.h"
#include "doctest.h"

#include <cstdint>
#include <cstring>

// Same setup as alphadmd.cpp: one luminance byte per dot in the frame buffer
#define LIBPINMAME
#define UINT8 uint8_t
#define UINT16 uint16_t
#include "alphadmd/usbalphanumeric.h"

TEST_CASE("AlphaDMD alphanumeric segment renderer")
{
   UINT16 segData[40] = {};

   SUBCASE("empty segment data produces a blank frame")
   {
      memset(AlphaNumericFrameBuffer, 0, sizeof(AlphaNumericFrameBuffer));
      _2x16Alpha(segData);
      for (size_t i = 0; i < sizeof(AlphaNumericFrameBuffer); i++)
         CHECK(AlphaNumericFrameBuffer[i] == 0);
   }

   SUBCASE("top segment of a character lights the expected dots")
   {
      memset(AlphaNumericFrameBuffer, 0, sizeof(AlphaNumericFrameBuffer));
      segData[0] = 0x0001; // segment 0 = top bar of first character
      _2x16Alpha(segData);
      // First character is drawn at x=0, y=2; its top segment covers x=1..5 on row 2
      for (int x = 1; x <= 5; x++)
         CHECK(AlphaNumericFrameBuffer[2 * 128 + x] == 3);
      CHECK(AlphaNumericFrameBuffer[2 * 128 + 0] == 0); // left of the segment
      CHECK(AlphaNumericFrameBuffer[2 * 128 + 6] == 0); // right of the segment
      CHECK(AlphaNumericFrameBuffer[1 * 128 + 3] == 0); // row above
      CHECK(AlphaNumericFrameBuffer[3 * 128 + 3] == 0); // row below
      CHECK(AlphaNumericFrameBuffer[19 * 128 + 3] == 0); // second row untouched
   }

   SUBCASE("second row uses the second half of the segment data")
   {
      memset(AlphaNumericFrameBuffer, 0, sizeof(AlphaNumericFrameBuffer));
      segData[16] = 0x0001; // segment 0 of first character, second row
      _2x16Alpha(segData);
      CHECK(AlphaNumericFrameBuffer[19 * 128 + 3] == 3);
      CHECK(AlphaNumericFrameBuffer[2 * 128 + 3] == 0); // first row untouched
   }

   SUBCASE("characters are laid out horizontally")
   {
      memset(AlphaNumericFrameBuffer, 0, sizeof(AlphaNumericFrameBuffer));
      segData[1] = 0x0001; // top segment of second character (x offset 8)
      _2x16Alpha(segData);
      CHECK(AlphaNumericFrameBuffer[2 * 128 + 9] == 3);
      CHECK(AlphaNumericFrameBuffer[2 * 128 + 1] == 0);
   }

   SUBCASE("all segments on rounds the character corners")
   {
      memset(AlphaNumericFrameBuffer, 0, sizeof(AlphaNumericFrameBuffer));
      segData[0] = 0xFFFF;
      _2x16Alpha(segData);
      // Left top segment (x=0, y=2..6) and top segment (x=1..5, y=2) are lit
      CHECK(AlphaNumericFrameBuffer[3 * 128 + 0] == 3);
      CHECK(AlphaNumericFrameBuffer[2 * 128 + 1] == 3);
      // The corner dot is smoothed out where they meet
      CHECK(AlphaNumericFrameBuffer[2 * 128 + 0] == 0);
   }
}
