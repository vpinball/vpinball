#pragma once

#if !((defined(__APPLE__) && ((defined(TARGET_OS_IOS) && TARGET_OS_IOS) || (defined(TARGET_OS_TV) && TARGET_OS_TV))))
#define __ZEDMD__
#endif

#ifdef __ZEDMD__
#pragma push_macro("_WIN64")
#pragma push_macro("CALLBACK")
#undef _WIN64
#undef CALLBACK
#include "ZeDMD.h"
#pragma pop_macro("CALLBACK")
#pragma pop_macro("_WIN64")
#endif

class ZeDMDUtil {
public:
   static ZeDMDUtil* GetInstance();

   void Open(int width, int height);
   void SetPalette(UINT8* palette);
   void RenderGray2(UINT8* frame);
   void RenderGray4(UINT8 *frame);
   void RenderColoredGray6(UINT8* frame, UINT8 *palette, UINT8 *rotations);
   void RenderRgb24(UINT8 *frame);
   void Close();

   static void CALLBACK OnLogMessage(const char* format, va_list args, const void* pUserData);

private:
   ZeDMDUtil();

   static ZeDMDUtil* instance;

#ifdef __ZEDMD__
   ZeDMD m_zedmd;
#endif
};
