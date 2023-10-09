#pragma once

#include "serum-decode.h"

class Serum {
public:
   Serum();
   ~Serum();

   bool Load(string path, string romName);
   bool Convert(UINT8* frame, UINT8* dstFrame, UINT8* dstPalette);

   bool IsLoaded();

private:
   bool m_isLoaded;

   int m_width;
   int m_height;

   UINT8* m_frame;
   int m_frameSize;
   UINT8 m_palette[64 * 3];
};
