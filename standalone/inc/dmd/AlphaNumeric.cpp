#include "stdafx.h"

#include "AlphaNumeric.h"

UINT8 AlphaNumeric::SegSizes[8][16] = {
   {5,5,5,5,5,5,2,2,5,5,5,2,5,5,5,1},
   {5,5,5,5,5,5,5,2,0,0,0,0,0,0,0,0},
   {5,5,5,5,5,5,5,2,5,5,0,0,0,0,0,0},
   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
   {5,2,2,5,2,2,5,2,0,0,0,0,0,0,0,0},
   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
   {5,5,5,5,5,5,3,2,5,5,5,3,5,5,5,1}
};

UINT8 AlphaNumeric::Segs[8][17][5][2] = {
   /* alphanumeric display characters */
   {
      {{1,0},{2,0},{3,0},{4,0},{5,0}}, // 0 top
      {{6,0},{6,1},{6,2},{6,3},{6,4}}, // 1 right top
      {{6,6},{6,7},{6,8},{6,9},{6,10}}, // 2 right bottom
      {{1,10},{2,10},{3,10},{4,10},{5,10}}, // 3 bottom
      {{0,6},{0,7},{0,8},{0,9},{0,10}}, // 4 left bottom
      {{0,0},{0,1},{0,2},{0,3},{0,4}}, // 5 left top
      {{1,5},{2,5},{0,0},{0,0},{0,0}}, // 6 middle left
      {{7,9},{7,10},{0,0},{0,0},{0,0}}, // 7 comma
      {{0,0},{1,1},{1,2},{2,3},{2,4}}, // 8 diag top left
      {{3,0},{3,1},{3,2},{3,3},{3,4}}, // 9 center top
      {{6,0},{5,1},{5,2},{4,3},{4,4}}, // 10 diag top right
      {{4,5},{5,5},{0,0},{0,0},{0,0}}, // 11 middle right
      {{4,6},{4,7},{5,8},{5,9},{6,10}}, // 12 diag bottom right
      {{3,6},{3,7},{3,8},{3,9},{3,10}}, // 13 center bottom
      {{0,10},{2,6},{2,7},{1,8},{1,9}}, // 14 diag bottom left
      {{7,10},{0,0},{0,0},{0,0},{0,0}} // 15 period
   }, 
   /* 8 segment LED characters */
   {
      {{1,0},{2,0},{3,0},{4,0},{5,0}}, // 0 top
      {{6,0},{6,1},{6,2},{6,3},{6,4}}, // 1 right top
      {{6,6},{6,7},{6,8},{6,9},{6,10}}, // 2 right bottom
      {{1,10},{2,10},{3,10},{4,10},{5,10}}, // 3 bottom
      {{0,6},{0,7},{0,8},{0,9},{0,10}}, // 4 left bottom
      {{0,0},{0,1},{0,2},{0,3},{0,4}}, // 5 left top
      {{1,5},{2,5},{3,5},{4,5},{5,5}}, // 6 middle
      {{7,9},{7,10},{0,0},{0,0},{0,0}}, // 7 comma
      {{0,0},{0,0},{0,0},{0,0},{0,0}},
      {{0,0},{0,0},{0,0},{0,0},{0,0}},
      {{0,0},{0,0},{0,0},{0,0},{0,0}},
      {{0,0},{0,0},{0,0},{0,0},{0,0}},
      {{0,0},{0,0},{0,0},{0,0},{0,0}},
      {{0,0},{0,0},{0,0},{0,0},{0,0}},
      {{0,0},{0,0},{0,0},{0,0},{0,0}},
      {{0,0},{0,0},{0,0},{0,0},{0,0}}
   },
   /* 10 segment LED characters */
   {
      {{1,0},{2,0},{3,0},{4,0},{5,0}}, // 0 top
      {{6,0},{6,1},{6,2},{6,3},{6,4}}, // 1 right top
      {{6,6},{6,7},{6,8},{6,9},{6,10}}, // 2 right bottom
      {{1,10},{2,10},{3,10},{4,10},{5,10}}, // 3 bottom
      {{0,6},{0,7},{0,8},{0,9},{0,10}}, // 4 left bottom
      {{0,0},{0,1},{0,2},{0,3},{0,4}}, // 5 left top
      {{1,5},{2,5},{3,5},{4,5},{5,5}}, // 6 middle
      {{7,9},{7,10},{0,0},{0,0},{0,0}}, // 7 comma
      {{3,0},{3,1},{3,2},{3,3},{3,4}}, // 8 diag top
      {{3,6},{3,7},{3,8},{3,9},{3,10}}, // 9 diag bottom
      {{0,0},{0,0},{0,0},{0,0},{0,0}},
      {{0,0},{0,0},{0,0},{0,0},{0,0}},
      {{0,0},{0,0},{0,0},{0,0},{0,0}},
      {{0,0},{0,0},{0,0},{0,0},{0,0}},
      {{0,0},{0,0},{0,0},{0,0},{0,0}},
      {{0,0},{0,0},{0,0},{0,0},{0,0}}
   },
   /* alphanumeric display characters (reversed comma with period) */
   {
      {{1,0},{2,0},{3,0},{4,0},{5,0}},
      {{6,0},{6,1},{6,2},{6,3},{6,4}},
      {{6,6},{6,7},{6,8},{6,9},{6,10}},
      {{1,10},{2,10},{3,10},{4,10},{5,10}},
      {{0,6},{0,7},{0,8},{0,9},{0,10}},
      {{0,0},{0,1},{0,2},{0,3},{0,4}},
      {{1,5},{2,5},{0,0},{0,0},{0,0}},
      {{7,9},{7,10},{0,0},{0,0},{0,0}},
      {{0,0},{1,1},{1,2},{2,3},{2,4}},
      {{3,0},{3,1},{3,2},{3,3},{3,4}},
      {{6,0},{5,1},{5,2},{4,3},{4,4}},
      {{4,5},{5,5},{0,0},{0,0},{0,0}},
      {{4,6},{4,7},{5,8},{5,9},{6,10}},
      {{3,6},{3,7},{3,8},{3,9},{3,10}},
      {{0,10},{2,6},{2,7},{1,8},{1,9}},
      {{7,10},{0,0},{0,0},{0,0},{0,0}}
   }, 
   /* 8 segment LED characters with dots instead of commas */
   {
      {{1,0},{2,0},{3,0},{4,0},{5,0}},
      {{6,0},{6,1},{6,2},{6,3},{6,4}},
      {{6,6},{6,7},{6,8},{6,9},{6,10}},
      {{1,10},{2,10},{3,10},{4,10},{5,10}},
      {{0,6},{0,7},{0,8},{0,9},{0,10}},
      {{0,0},{0,1},{0,2},{0,3},{0,4}},
      {{1,5},{2,5},{3,5},{4,5},{5,5}},
      {{7,9},{7,10},{0,0},{0,0},{0,0}},
      {{0,0},{0,0},{0,0},{0,0},{0,0}},
      {{0,0},{0,0},{0,0},{0,0},{0,0}},
      {{0,0},{0,0},{0,0},{0,0},{0,0}},
      {{0,0},{0,0},{0,0},{0,0},{0,0}},
      {{0,0},{0,0},{0,0},{0,0},{0,0}},
      {{0,0},{0,0},{0,0},{0,0},{0,0}},
      {{0,0},{0,0},{0,0},{0,0},{0,0}},
      {{0,0},{0,0},{0,0},{0,0},{0,0}}
   },
   /* 8 segment LED characters SMALL */
   {
      {{1,0},{2,0},{3,0},{4,0},{5,0}}, // 0 top
      {{6,1},{6,2},{0,0},{0,0},{0,0}}, // 1 top right
      {{6,4},{6,5},{0,0},{0,0},{0,0}}, // 2 bottom right
      {{1,6},{2,6},{3,6},{4,6},{5,6}}, // 3 bottom
      {{0,4},{0,5},{0,0},{0,0},{0,0}}, // 4 bottom left
      {{0,1},{0,2},{0,0},{0,0},{0,0}}, // 5 top left
      {{1,3},{2,3},{3,3},{4,3},{5,3}}, // 6 middle
      {{7,5},{7,6},{0,0},{0,0},{0,0}}, // 7 commy
      {{0,0},{0,0},{0,0},{0,0},{0,0}},
      {{0,0},{0,0},{0,0},{0,0},{0,0}},
      {{0,0},{0,0},{0,0},{0,0},{0,0}},
      {{0,0},{0,0},{0,0},{0,0},{0,0}},
      {{0,0},{0,0},{0,0},{0,0},{0,0}},
      {{0,0},{0,0},{0,0},{0,0},{0,0}},
      {{0,0},{0,0},{0,0},{0,0},{0,0}},
      {{0,0},{0,0},{0,0},{0,0},{0,0}}
   },
   /* 10 segment LED characters SMALL */
   {
      {{0,0},{0,0},{0,0},{0,0},{0,0}},
      {{0,0},{0,0},{0,0},{0,0},{0,0}},
      {{0,0},{0,0},{0,0},{0,0},{0,0}},
      {{0,0},{0,0},{0,0},{0,0},{0,0}},
      {{0,0},{0,0},{0,0},{0,0},{0,0}},
      {{0,0},{0,0},{0,0},{0,0},{0,0}},
      {{0,0},{0,0},{0,0},{0,0},{0,0}},
      {{0,0},{0,0},{0,0},{0,0},{0,0}},
      {{0,0},{0,0},{0,0},{0,0},{0,0}},
      {{0,0},{0,0},{0,0},{0,0},{0,0}},
      {{0,0},{0,0},{0,0},{0,0},{0,0}},
      {{0,0},{0,0},{0,0},{0,0},{0,0}},
      {{0,0},{0,0},{0,0},{0,0},{0,0}},
      {{0,0},{0,0},{0,0},{0,0},{0,0}},
      {{0,0},{0,0},{0,0},{0,0},{0,0}},
      {{0,0},{0,0},{0,0},{0,0},{0,0}}
   },
   /* alphanumeric display characters 6pixel width*/
   {   
      {{0,0},{1,0},{2,0},{3,0},{4,0}},
      {{4,0},{4,1},{4,2},{4,3},{4,4}},
      {{4,6},{4,7},{4,8},{4,9},{4,10}},
      {{0,10},{1,10},{2,10},{3,10},{4,10}},
      {{0,6},{0,7},{0,8},{0,9},{0,10}},
      {{0,0},{0,1},{0,2},{0,3},{0,4}},
      {{0,5},{1,5},{2,5},{0,0},{0,0}},
      {{5,9},{5,10},{0,0},{0,0},{0,0}},
      {{0,0},{1,1},{1,2},{2,3},{2,4}},
      {{2,0},{2,1},{2,2},{2,3},{2,4}},
      {{4,0},{3,1},{3,2},{2,3},{2,4}},
      {{2,5},{3,5},{4,5},{0,0},{0,0}},
      {{2,6},{2,7},{3,8},{3,9},{4,10}},
      {{2,6},{2,7},{2,8},{2,9},{2,10}},
      {{0,10},{2,6},{2,7},{1,8},{1,9}},
      {{5,10},{0,0},{0,0},{0,0},{0,0}}
   }
};

UINT8 AlphaNumeric::frameBuffer[4096] = {0};
UINT8 AlphaNumeric::lastFrameBuffer[4096] = {0};

UINT8* AlphaNumeric::Render(NumericalLayout layout, const UINT16* const seg_data, bool reset)
{
   UINT8* pBuffer;

   if (layout != __2x7Num_2x7Num_10x1Num)
      pBuffer = Render(layout, seg_data, NULL, reset);
   else
      pBuffer = Render(layout, seg_data, seg_data + 32, reset);

   return pBuffer;
}

UINT8* AlphaNumeric::Render(NumericalLayout layout, const UINT16* const seg_data, const UINT16* const seg_data2, bool reset)
{
   switch (layout) {
      case __2x16Alpha:
         Render2x16Alpha(seg_data);
         break;
      case __2x20Alpha:
         Render2x20Alpha(seg_data);
         break;
      case __2x7Alpha_2x7Num:
         Render2x7Alpha_2x7Num(seg_data);
         break;
      case __2x7Alpha_2x7Num_4x1Num:
         Render2x7Alpha_2x7Num_4x1Num(seg_data);
         break;
      case __2x7Num_2x7Num_4x1Num:
         Render2x7Num_2x7Num_4x1Num(seg_data);
         break;
      case __2x7Num_2x7Num_10x1Num:
         Render2x7Num_2x7Num_10x1Num(seg_data, seg_data2);
         break;
      case __2x7Num_2x7Num_4x1Num_gen7:
         Render2x7Num_2x7Num_4x1Num_gen7(seg_data);
         break;
      case __2x7Num10_2x7Num10_4x1Num:
         Render2x7Num10_2x7Num10_4x1Num(seg_data);
         break;
      case __2x6Num_2x6Num_4x1Num:
         Render2x6Num_2x6Num_4x1Num(seg_data);
         break;
      case __2x6Num10_2x6Num10_4x1Num:
         Render2x6Num10_2x6Num10_4x1Num(seg_data);
         break;
      case __4x7Num10:
         Render4x7Num10(seg_data);
         break;
      case __6x4Num_4x1Num:
         Render6x4Num_4x1Num(seg_data);
         break;
      case __2x7Num_4x1Num_1x16Alpha:
         Render2x7Num_4x1Num_1x16Alpha(seg_data);
         break;
      case __1x16Alpha_1x16Num_1x7Num:
         Render1x16Alpha_1x16Num_1x7Num(seg_data);
         break;
      case __1x7Num_1x16Alpha_1x16Num:
         Render1x7Num_1x16Alpha_1x16Num(seg_data);
         break;
      case __1x16Alpha_1x16Num_1x7Num_1x4Num:
         Render1x16Alpha_1x16Num_1x7Num_1x4Num(seg_data);
         break;
      default:
         break;
   }

   if (!reset && !memcmp(lastFrameBuffer, frameBuffer, sizeof(lastFrameBuffer))) {
      return NULL;
   }

   memcpy(lastFrameBuffer, frameBuffer, sizeof(lastFrameBuffer));

   return frameBuffer;
}

void AlphaNumeric::SmoothDigitCorners(const int x, const int y)
{
   if (GetPixel(x, 1 + y) && GetPixel(1 + x, y))
      DrawPixel(0 + x, y, 0);
   if (GetPixel(x + 6, 1 + y) && GetPixel(5 + x, y))
      DrawPixel(6 + x, y, 0);
   if (GetPixel(x, 9 + y) && GetPixel(1 + x, 10 + y))
      DrawPixel(0 + x, 10 + y, 0);
   if (GetPixel(x + 6, 9 + y) && GetPixel(5 + x, 10 + y))
      DrawPixel(6 + x, 10 + y, 0);
}

void AlphaNumeric::SmoothDigitCorners6Px(int x, int y)
{
   if (GetPixel(x, 1 + y) && GetPixel(1 + x, y))
      DrawPixel(0 + x, y, 0);
   if (GetPixel(x + 4, 1 + y) && GetPixel(3 + x, y))
      DrawPixel(4 + x, y, 0);
   if (GetPixel(x, 9 + y) && GetPixel(1 + x, 10 + y))
      DrawPixel(0 + x, 10 + y, 0);
   if (GetPixel(x + 4, 9 + y) && GetPixel(3 + x, 10 + y))
      DrawPixel(4 + x, 10 + y, 0);
}

void AlphaNumeric::DrawSegment(const int x, const int y, const UINT8 type, const UINT16 seg, const UINT8 colour)
{
   for (int i = 0; i < SegSizes[type][seg]; i++)
      DrawPixel(Segs[type][seg][i][0] + x, Segs[type][seg][i][1] + y, colour);
}

UINT8 AlphaNumeric::GetPixel(const int x, const int y)
{
   return frameBuffer[y * 128 + x] > 0;
}

void AlphaNumeric::DrawPixel(const int x, const int y, const UINT8 colour)
{
   frameBuffer[y * 128 + x] = colour;
}

void AlphaNumeric::Clear()
{
   memset(frameBuffer, 0, sizeof(frameBuffer));
}

void AlphaNumeric::Render2x16Alpha(const UINT16* const seg_data)
{
   Clear();
   int i;
   int j;
   for (i = 0; i < 16; i++) {
      for (j = 0; j < 16; j++) {
         if ((seg_data[i] >> j) & 0x1)
            DrawSegment(i * 8, 2, 0, j, 3);
         if ((seg_data[i + 16] >> j) & 0x1)
            DrawSegment(i * 8, 19, 0, j, 3);
      }
      SmoothDigitCorners(i * 8, 2);
      SmoothDigitCorners(i * 8, 19);
   }
}

void AlphaNumeric::Render2x20Alpha(const UINT16* const seg_data)
{
   Clear();
   int i;
   int j;
   for (i = 0; i < 20; i++) {
      for (j = 0; j < 16; j++) {
         if ((seg_data[i] >> j) & 0x1)
            DrawSegment((i * 6) + 4, 2, 7, j, 3);
         if ((seg_data[i + 20] >> j) & 0x1)
            DrawSegment((i * 6) + 4, 19, 7, j, 3);
      }
      SmoothDigitCorners6Px((i * 6) + 4, 2);
      SmoothDigitCorners6Px((i * 6) + 4, 19);
   }
}

void AlphaNumeric::Render2x7Alpha_2x7Num(const UINT16* const seg_data)
{
   Clear();
   int i;
   int j;
   for (i = 0; i < 14; i++) {
      for (j = 0; j < 16; j++) {
         // 2x7 alphanumeric
         if ((seg_data[i] >> j) & 0x1)
            DrawSegment((i + ((i < 7) ? 0 : 2)) * 8, 2, 0, j, 3);
         // 2x7 numeric
         if ((seg_data[i + 14] >> j) & 0x1)
            DrawSegment((i + ((i < 7) ? 0 : 2)) * 8, 19, 1, j, 3);
      }
      SmoothDigitCorners((i + ((i < 7) ? 0 : 2)) * 8, 2);
      SmoothDigitCorners((i + ((i < 7) ? 0 : 2)) * 8, 19);
   }
}

void AlphaNumeric::Render2x7Alpha_2x7Num_4x1Num(const UINT16* const seg_data)
{
   Clear();
   int i;
   int j;
   for (i = 0; i < 14; i++) {
      for (j = 0; j < 16; j++) {
         // 2x7 alphanumeric
         if ((seg_data[i] >> j) & 0x1)
            DrawSegment((i + ((i < 7) ? 0 : 2)) * 8, 0, 0, j, 3);
         // 2x7 numeric
         if ((seg_data[i + 14] >> j) & 0x1)
            DrawSegment((i + ((i < 7) ? 0 : 2)) * 8, 21, 1, j, 3);
      }
      SmoothDigitCorners((i + ((i < 7) ? 0 : 2)) * 8, 0);
      SmoothDigitCorners((i + ((i < 7) ? 0 : 2)) * 8, 21);
   }
   // 4x1 numeric small
   for (j = 0; j < 16; j++) {
      if ((seg_data[28] >> j) & 0x1)
         DrawSegment(8, 12, 5, j, 3);
      if ((seg_data[29] >> j) & 0x1)
         DrawSegment(16, 12, 5, j, 3);
      if ((seg_data[30] >> j) & 0x1)
         DrawSegment(32, 12, 5, j, 3);
      if ((seg_data[31] >> j) & 0x1)
         DrawSegment(40, 12, 5, j, 3);
   }
}

void AlphaNumeric::Render2x6Num_2x6Num_4x1Num(const UINT16* const seg_data)
{
   Clear();
   int i;
   int j;
   for (i = 0; i < 12; i++) {
      for (j = 0; j < 16; j++) {
         // 2x7 numeric
         if ((seg_data[i] >> j) & 0x1)
            DrawSegment((i + ((i < 6) ? 0 : 4)) * 8, 0, 1, j, 3);
         
         // 2x7 numeric
         if ((seg_data[i + 12] >> j) & 0x1)
            DrawSegment((i + ((i < 6) ? 0 : 4)) * 8, 12, 1, j, 3);
      }
      SmoothDigitCorners((i + ((i < 6) ? 0 : 4)) * 8, 0);
      SmoothDigitCorners((i + ((i < 6) ? 0 : 4)) * 8, 12);
   }
   // 4x1 numeric small
   for (j = 0; j < 16; j++) {
      if ((seg_data[24] >> j) & 0x1)
         DrawSegment(8, 24, 5, j, 3);
      if ((seg_data[25] >> j) & 0x1)
         DrawSegment(16, 24, 5, j, 3);
      if ((seg_data[26] >> j) & 0x1)
         DrawSegment(32, 24, 5, j, 3);
      if ((seg_data[27] >> j) & 0x1)
         DrawSegment(40, 24, 5, j, 3);
   }
}

void AlphaNumeric::Render2x6Num10_2x6Num10_4x1Num(const UINT16* const seg_data)
{
   Clear();
   int i;
   int j;
   for (i = 0; i < 12; i++) {
      for (j = 0; j < 16; j++) {
         // 2x7 numeric
         if ((seg_data[i] >> j) & 0x1)
            DrawSegment((i + ((i < 6) ? 0 : 4)) * 8, 0, 2, j, 3);
         // 2x7 numeric
         if ((seg_data[i + 12] >> j) & 0x1)
            DrawSegment((i + ((i < 6) ? 0 : 4)) * 8, 20, 2, j, 3);
      }
      SmoothDigitCorners((i + ((i < 6) ? 0 : 4)) * 8, 0);
      SmoothDigitCorners((i + ((i < 6) ? 0 : 4)) * 8, 20);
   }
   // 4x1 numeric small
   for (j = 0; j < 16; j++) {
      if ((seg_data[24] >> j) & 0x1)
         DrawSegment(8, 12, 5, j, 3);
      if ((seg_data[25] >> j) & 0x1)
         DrawSegment(16, 12, 5, j, 3);
      if ((seg_data[26] >> j) & 0x1)
         DrawSegment(32, 12, 5, j, 3);
      if ((seg_data[27] >> j) & 0x1)
         DrawSegment(40, 12, 5, j, 3);
   }
}

void AlphaNumeric::Render2x7Num_2x7Num_4x1Num(const UINT16* const seg_data)
{
   Clear();
   int i;
   int j;
   for (i = 0; i < 14; i++) {
      for (j = 0; j < 16; j++) {
         // 2x7 numeric
         if ((seg_data[i] >> j) & 0x1)
            DrawSegment((i + ((i < 7) ? 0 : 2)) * 8, 0, 1, j, 3);
         // 2x7 numeric
         if ((seg_data[i + 14] >> j) & 0x1)
            DrawSegment((i + ((i < 7) ? 0 : 2)) * 8, 12, 1, j, 3);
      }
      SmoothDigitCorners((i + ((i < 7) ? 0 : 2)) * 8, 0);
      SmoothDigitCorners((i + ((i < 7) ? 0 : 2)) * 8, 12);
   }
   // 4x1 numeric small
   for (j = 0; j < 16; j++) {
      if ((seg_data[28] >> j) & 0x1)
         DrawSegment(16, 24, 5, j, 3);
      if ((seg_data[29] >> j) & 0x1)
         DrawSegment(24, 24, 5, j, 3);
      if ((seg_data[30] >> j) & 0x1)
         DrawSegment(40, 24, 5, j, 3);
      if ((seg_data[31] >> j) & 0x1)
         DrawSegment(48, 24, 5, j, 3);
   }
}

void AlphaNumeric::Render2x7Num_2x7Num_10x1Num(const UINT16* const seg_data, const UINT16* const extra_seg_data)
{
   Clear();
   int i;
   int j;
   for (i = 0; i < 14; i++) {
      for (j = 0; j < 16; j++) {
         // 2x7 numeric
         if ((seg_data[i] >> j) & 0x1)
            DrawSegment((i + ((i < 7) ? 0 : 2)) * 8, 0, 1, j, 3);
         // 2x7 numeric
         if ((seg_data[i + 14] >> j) & 0x1)
            DrawSegment((i + ((i < 7) ? 0 : 2)) * 8, 12, 1, j, 3);
      }
      SmoothDigitCorners((i + ((i < 7) ? 0 : 2)) * 8, 0);
      SmoothDigitCorners((i + ((i < 7) ? 0 : 2)) * 8, 12);
   }
   // 10x1 numeric small
   for (j = 0; j < 16; j++) {
      if ((seg_data[28] >> j) & 0x1)
         DrawSegment(16, 24, 5, j, 3);
      if ((seg_data[29] >> j) & 0x1)
         DrawSegment(24, 24, 5, j, 3);
      if ((seg_data[30] >> j) & 0x1)
         DrawSegment(40, 24, 5, j, 3);
      if ((seg_data[31] >> j) & 0x1)
         DrawSegment(48, 24, 5, j, 3);
      if ((extra_seg_data[0] >> j) & 0x1)
         DrawSegment(64, 24, 5, j, 3);
      if ((extra_seg_data[1] >> j) & 0x1)
         DrawSegment(72, 24, 5, j, 3);
      if ((extra_seg_data[2] >> j) & 0x1)
         DrawSegment(88, 24, 5, j, 3);
      if ((extra_seg_data[3] >> j) & 0x1)
         DrawSegment(96, 24, 5, j, 3);
      if ((extra_seg_data[4] >> j) & 0x1)
         DrawSegment(112, 24, 5, j, 3);
      if ((extra_seg_data[5] >> j) & 0x1)
         DrawSegment(120, 24, 5, j, 3);
   }
}

void AlphaNumeric::Render2x7Num_2x7Num_4x1Num_gen7(const UINT16* const seg_data)
{
   Clear();
   int i;
   int j;
   for (i = 0; i < 14; i++) {
      for (j = 0; j < 16; j++) {
         // 2x7 numeric
         if ((seg_data[i] >> j) & 0x1)
            DrawSegment((i + ((i < 7) ? 0 : 2)) * 8, 21, 1, j, 3);
         // 2x7 numeric
         if ((seg_data[i + 14] >> j) & 0x1)
            DrawSegment((i + ((i < 7) ? 0 : 2)) * 8, 1, 1, j, 3);
      }
      SmoothDigitCorners((i + ((i < 7) ? 0 : 2)) * 8, 21);
      SmoothDigitCorners((i + ((i < 7) ? 0 : 2)) * 8, 1);
   }
   // 4x1 numeric small
   for (j = 0; j < 16; j++) {
      if ((seg_data[28] >> j) & 0x1)
         DrawSegment(8, 13, 5, j, 3);
      if ((seg_data[29] >> j) & 0x1)
         DrawSegment(16, 13, 5, j, 3);
      if ((seg_data[30] >> j) & 0x1)
         DrawSegment(32, 13, 5, j, 3);
      if ((seg_data[31] >> j) & 0x1)
         DrawSegment(40, 13, 5, j, 3);
   }
}

void AlphaNumeric::Render2x7Num10_2x7Num10_4x1Num(const UINT16* const seg_data)
{
   Clear();
   int i;
   int j;
   for (i = 0; i < 14; i++) {
      for (j = 0; j < 16; j++) {
         // 2x7 numeric
         if ((seg_data[i] >> j) & 0x1)
            DrawSegment((i + ((i < 7) ? 0 : 2)) * 8, 0, 2, j, 3);
         // 2x7 numeric
         if ((seg_data[i + 14] >> j) & 0x1)
            DrawSegment((i + ((i < 7) ? 0 : 2)) * 8, 20, 2, j, 3);
      }
      SmoothDigitCorners((i + ((i < 7) ? 0 : 2)) * 8, 0);
      SmoothDigitCorners((i + ((i < 7) ? 0 : 2)) * 8, 20);
   }
   // 4x1 numeric small
   for (j = 0; j < 16; j++) {
      if ((seg_data[28] >> j) & 0x1)
         DrawSegment(8, 12, 5, j, 3);
      if ((seg_data[29] >> j) & 0x1)
         DrawSegment(16, 12, 5, j, 3);
      if ((seg_data[30] >> j) & 0x1)
         DrawSegment(32, 12, 5, j, 3);
      if ((seg_data[31] >> j) & 0x1)
         DrawSegment(40, 12, 5, j, 3);
   }
}

void AlphaNumeric::Render4x7Num10(const UINT16* const seg_data)
{
   Clear();
   int i;
   int j;
   for (i = 0; i < 14; i++) {
      for (j = 0; j < 16; j++) {
         // 2x7 numeric10
         if ((seg_data[i] >> j) & 0x1)
            DrawSegment((i + ((i < 7) ? 0 : 2)) * 8, 1, 2, j, 3);
         // 2x7 numeric10
         if ((seg_data[i + 14] >> j) & 0x1)
            DrawSegment((i + ((i < 7) ? 0 : 2)) * 8, 13, 2, j, 3);
      }
      SmoothDigitCorners((i + ((i < 7) ? 0 : 2)) * 8, 1);
      SmoothDigitCorners((i + ((i < 7) ? 0 : 2)) * 8, 13);
   }
}

void AlphaNumeric::Render6x4Num_4x1Num(const UINT16* const seg_data)
{
   Clear();
   int i;
   int j;
   for (i = 0; i < 8; i++) {
      for (j = 0; j < 16; j++) {
         // 2x4 numeric
         if ((seg_data[i] >> j) & 0x1)
            DrawSegment((i + ((i < 4) ? 0 : 2)) * 8, 1, 5, j, 3);
         // 2x4 numeric
         if ((seg_data[i + 8] >> j) & 0x1)
            DrawSegment((i + ((i < 4) ? 0 : 2)) * 8, 9, 5, j, 3);
         // 2x4 numeric
         if ((seg_data[i + 16] >> j) & 0x1)
            DrawSegment((i + ((i < 4) ? 0 : 2)) * 8, 17, 5, j, 3);
      }
      SmoothDigitCorners((i + ((i < 4) ? 0 : 2)) * 8, 1);
      SmoothDigitCorners((i + ((i < 4) ? 0 : 2)) * 8, 9);
      SmoothDigitCorners((i + ((i < 4) ? 0 : 2)) * 8, 17);
   }
   // 4x1 numeric small
   for (j = 0; j < 16; j++) {
      if ((seg_data[24] >> j) & 0x1)
         DrawSegment(16, 25, 5, j, 3);
      if ((seg_data[25] >> j) & 0x1)
         DrawSegment(24, 25, 5, j, 3);
      if ((seg_data[26] >> j) & 0x1)
         DrawSegment(48, 25, 5, j, 3);
      if ((seg_data[27] >> j) & 0x1)
         DrawSegment(56, 25, 5, j, 3);
   }
}

void AlphaNumeric::Render2x7Num_4x1Num_1x16Alpha(const UINT16* const seg_data)
{
   Clear();
   int i;
   int j;
   for (i = 0; i < 14; i++) {
      for (j = 0; j < 16; j++) {
         // 2x7 numeric
         if ((seg_data[i] >> j) & 0x1)
            DrawSegment((i + ((i < 7) ? 0 : 2)) * 8, 0, 1, j, 3);
      }
      SmoothDigitCorners((i + ((i < 7) ? 0 : 2)) * 8, 0);
   }
   // 4x1 numeric small
   for (j = 0; j < 16; j++) {
      if ((seg_data[14] >> j) & 0x1)
         DrawSegment(16, 12, 5, j, 3);
      if ((seg_data[15] >> j) & 0x1)
         DrawSegment(24, 12, 5, j, 3);
      if ((seg_data[16] >> j) & 0x1)
         DrawSegment(40, 12, 5, j, 3);
      if ((seg_data[17] >> j) & 0x1)
         DrawSegment(48, 12, 5, j, 3);
   }
   // 1x16 alphanumeric
   for (i = 0; i < 12; i++) {
      for (j = 0; j < 16; j++) {
         if ((seg_data[i + 18] >> j) & 0x1)
            DrawSegment((i * 8) + 16, 21, 0, j, 3);
      }
      SmoothDigitCorners((i * 8) + 16, 21);
   }
}

void AlphaNumeric::Render1x16Alpha_1x16Num_1x7Num(const UINT16* const seg_data)
{
   Clear();
   int i;
   int j;
   // 1x16 alphanumeric
   for (i = 0; i < 16; i++) {
      for (j = 0; j < 16; j++) {
         if ((seg_data[i] >> j) & 0x1)
            DrawSegment((i * 8), 9, 0, j, 3);
      }
      SmoothDigitCorners((i * 8), 9);
   }

   // 1x16 numeric
   for (i = 0; i < 16; i++) {
      for (j = 0; j < 16; j++) {
         if ((seg_data[i + 16] >> j) & 0x1)
            DrawSegment((i * 8), 21, 1, j, 3);
      }
      SmoothDigitCorners((i * 8), 21);
   }

   // 1x7 numeric small
   for (i = 0; i < 7; i++) {
      for (j = 0; j < 16; j++) {
         if ((seg_data[i + 32] >> j) & 0x1)
            DrawSegment((i * 8) + 68, 1, 5, j, 3);
      }
   }
}

void AlphaNumeric::Render1x7Num_1x16Alpha_1x16Num(const UINT16* const seg_data)
{
   Clear();
   int i;
   int j;
   // 1x16 alphanumeric
   for (i = 0; i < 16; i++) {
      for (j = 0; j < 16; j++) {
         if ((seg_data[i+8] >> j) & 0x1)
            DrawSegment((i * 8), 9, 0, j, 3);
      }
      SmoothDigitCorners((i * 8), 9);
   }
   // 1x16 numeric
   for (i = 0; i < 16; i++) {
      for (j = 0; j < 16; j++) {
         if ((seg_data[i + 24] >> j) & 0x1)
            DrawSegment((i * 8), 21, 1, j, 3);
      }
      SmoothDigitCorners((i * 8), 21);
   }
   // 1x7 numeric small
   for (i = 0; i < 7; i++) {
      for (j = 0; j < 16; j++) {
         if ((seg_data[i+1] >> j) & 0x1)
            DrawSegment((i * 8) + 68, 1, 5, j, 3);
      }
   }
}

void AlphaNumeric::Render1x16Alpha_1x16Num_1x7Num_1x4Num(const UINT16* const seg_data)
{
   Clear();
   int i;
   int j;
   // 1x16 alphanumeric
   for (i = 0; i < 16; i++) {
      for (j = 0; j < 16; j++) {
         if ((seg_data[i + 11] >> j) & 0x1)
            DrawSegment((i * 8), 9, 0, j, 3);
      }
      SmoothDigitCorners((i * 8), 9);
   }
   // 1x16 numeric
   for (i = 0; i < 16; i++) {
      for (j = 0; j < 16; j++) {
         if ((seg_data[i + 27] >> j) & 0x1)
            DrawSegment((i * 8), 21, 1, j, 3);
      }
      SmoothDigitCorners((i * 8), 21);
   }
   // 1x4 numeric small
   for (i = 0; i < 4; i++) {
      for (j = 0; j < 16; j++) {
         if ((seg_data[i + 7] >> j) & 0x1)
            DrawSegment((i * 8)+4, 1, 5, j, 3);
      }
   }
   // 1x7 numeric small
   for (i = 0; i < 7; i++) {
      for (j = 0; j < 16; j++) {
         if ((seg_data[i] >> j) & 0x1)
            DrawSegment((i * 8)+68, 1, 5, j, 3);
      }
   }
}