#include "stdafx.h"
#include "Serum.h"

Serum::Serum()
{
   m_frame = NULL;
   m_isLoaded = false;
}

Serum::~Serum()
{
   if (m_frame)
      free(m_frame);

   Serum_Dispose();
}


bool Serum::Load(string path, string romName)
{
   if (m_isLoaded)
      return false;

   unsigned int numColors;
   unsigned int numTriggers;

   if (!Serum_Load(path.c_str(), romName.c_str(), &m_width, &m_height, &numColors, &numTriggers)) {
      Serum_Dispose();
      return false;
   }

   PLOGI.printf("Serum loaded: romName=%s, width=%d, height=%d, numColors=%d, numTriggers=%d", romName.c_str(), m_width, m_height, numColors, numTriggers);

   m_frameSize = m_width * m_height;

   m_frame = (UINT8*)malloc(m_frameSize);
   memset(m_frame, 0, m_frameSize);

   m_isLoaded = true;

   return true;
}

bool Serum::IsLoaded()
{
   return m_isLoaded;
}

bool Serum::Convert(UINT8* frame, UINT8* dstFrame, UINT8* dstPalette)
{
   if (frame)
      memcpy(m_frame, frame, m_frameSize);

   unsigned int triggerId;

   bool update = Serum_ColorizeOrApplyRotations(frame ? m_frame : NULL, m_width, m_height, m_palette, &triggerId);

   if (update) {
      memcpy(dstFrame, m_frame, m_frameSize);
      memcpy(dstPalette, m_palette, 192);
   }

   return update;
}