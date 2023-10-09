#include "stdafx.h"

#include "ZeDMDUtil.h"

ZeDMDUtil* ZeDMDUtil::pInstance = NULL;

void CALLBACK ZeDMDUtil::OnLogMessage(const char* format, va_list args, const void* pUserData)
{
   char buffer[1024];
   vsnprintf(buffer, sizeof(buffer), format, args);

   PLOGI.printf("%s", buffer);
}

ZeDMDUtil* ZeDMDUtil::GetInstance()
{
   if (!pInstance)
      pInstance = new ZeDMDUtil();

   return pInstance;
}

ZeDMDUtil::ZeDMDUtil()
{
#ifdef __ZEDMD__
   m_zedmd.SetLogMessageCallback(&ZeDMDUtil::OnLogMessage, NULL);

#ifdef __ANDROID__
   m_zedmd.SetAndroidGetJNIEnvFunc(&SDL_AndroidGetJNIEnv);
#endif
#endif
}

void ZeDMDUtil::Open(int width, int height)
{
#ifdef __ZEDMD__
   new std::thread([this, width, height]() {
      if (!m_zedmd.Open(width, height))
         return;

      if (g_pvp->m_settings.LoadValueWithDefault(Settings::Standalone, "ZeDMDDebug", false)) {
         m_zedmd.EnableDebug();
         PLOGI.printf("ZeDMD debug enabled");
      }

      const int rgbOrder = (int)g_pvp->m_settings.LoadValueWithDefault(Settings::Standalone, "ZeDMDRGBOrder"s, -1);
      if (rgbOrder != -1) {
         m_zedmd.SetRGBOrder(rgbOrder);
         PLOGI.printf("ZeDMD RGB order override: rgbOrder=%d", rgbOrder);
      }

      const int brightness = (int)g_pvp->m_settings.LoadValueWithDefault(Settings::Standalone, "ZeDMDBrightness"s, -1);
      if (brightness != -1) {
         m_zedmd.SetBrightness(brightness);
         PLOGI.printf("ZeDMD brightness override: brightness=%d", brightness);
      }

      if (g_pvp->m_settings.LoadValueWithDefault(Settings::Standalone, "ZeDMDSave", false)) {
         m_zedmd.SaveSettings();
         PLOGI.printf("ZeDMD save enabled");
      }
   });
#endif
}

void ZeDMDUtil::SetPalette(UINT8* palette)
{
#ifdef __ZEDMD__
    m_zedmd.SetPalette(palette);
#endif
}

void ZeDMDUtil::RenderGray2(UINT8* frame)
{
#ifdef __ZEDMD__
    m_zedmd.RenderGray2(frame);
#endif
}

void ZeDMDUtil::RenderGray4(UINT8* frame)
{
#ifdef __ZEDMD__
   m_zedmd.RenderGray4(frame);
#endif
}

void ZeDMDUtil::RenderColoredGray6(UINT8* frame, UINT8* palette, UINT8* rotations)
{
#ifdef __ZEDMD__
   m_zedmd.RenderColoredGray6(frame, palette, rotations);
#endif
}

void ZeDMDUtil::RenderRgb24(uint8_t *frame)
{
#ifdef __ZEDMD__
   m_zedmd.RenderRgb24(frame);
#endif
}

void ZeDMDUtil::Close()
{
#ifdef __ZEDMD__
   m_zedmd.Close();
#endif
}