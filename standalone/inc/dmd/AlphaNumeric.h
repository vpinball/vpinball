/*
 * Portions of this code was derived from DMDExt and PinMAME
 *
 * https://github.com/freezy/dmd-extensions/blob/master/LibDmd/DmdDevice/AlphaNumeric.cs
 * https://github.com/vpinball/pinmame/blob/master/ext/dmddevice/usbalphanumeric.h
 */

#pragma once

typedef enum {
   None,
   __2x16Alpha,
   __2x20Alpha,
   __2x7Alpha_2x7Num,
   __2x7Alpha_2x7Num_4x1Num,
   __2x7Num_2x7Num_4x1Num,
   __2x7Num_2x7Num_10x1Num,
   __2x7Num_2x7Num_4x1Num_gen7,
   __2x7Num10_2x7Num10_4x1Num,
   __2x6Num_2x6Num_4x1Num,
   __2x6Num10_2x6Num10_4x1Num,
   __4x7Num10,
   __6x4Num_4x1Num,
   __2x7Num_4x1Num_1x16Alpha,
   __1x16Alpha_1x16Num_1x7Num,
   __1x7Num_1x16Alpha_1x16Num,
   __1x16Alpha_1x16Num_1x7Num_1x4Num
} NumericalLayout;

class AlphaNumeric {
public:
   static UINT8* Render(NumericalLayout layout, const UINT16* const seg_data, bool reset = false);
   static UINT8* Render(NumericalLayout layout, const UINT16* const seg_data, const UINT16* const seg_data2, bool reset = false);

private:
   static void SmoothDigitCorners(const int x, const int y);
   static void SmoothDigitCorners6Px(const int x, const int y);
   static void DrawSegment(const int x, const int y, const UINT8 type, const UINT16 seg, const UINT8 colour);
   static UINT8 GetPixel(const int x, const int y);
   static void DrawPixel(const int x, const int y, const UINT8 colour);
   static void Clear();

   static void Render2x16Alpha(const UINT16* const seg_data);
   static void Render2x20Alpha(const UINT16* const seg_data);
   static void Render2x7Alpha_2x7Num(const UINT16* const seg_data);
   static void Render2x7Alpha_2x7Num_4x1Num(const UINT16* const seg_data);
   static void Render2x6Num_2x6Num_4x1Num(const UINT16* const seg_data);
   static void Render2x6Num10_2x6Num10_4x1Num(const UINT16* const seg_data);
   static void Render2x7Num_2x7Num_4x1Num(const UINT16* const seg_data);
   static void Render2x7Num_2x7Num_10x1Num(const UINT16* const seg_data, const UINT16* const extra_seg_data);
   static void Render2x7Num_2x7Num_4x1Num_gen7(const UINT16* const seg_data);
   static void Render2x7Num10_2x7Num10_4x1Num(const UINT16* const seg_data);
   static void Render4x7Num10(const UINT16* const seg_data);
   static void Render6x4Num_4x1Num(const UINT16* const seg_data);
   static void Render2x7Num_4x1Num_1x16Alpha(const UINT16* const seg_data);
   static void Render1x16Alpha_1x16Num_1x7Num(const UINT16* const seg_data);
   static void Render1x7Num_1x16Alpha_1x16Num(const UINT16* const seg_data);
   static void Render1x16Alpha_1x16Num_1x7Num_1x4Num(const UINT16* const seg_data);

   static UINT8 SegSizes[8][16];
   static UINT8 Segs[8][17][5][2];

   static UINT8 frameBuffer[4096];
   static UINT8 lastFrameBuffer[4096];
};
