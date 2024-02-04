#pragma once

#include "../common/Window.h"
#include "DMDUtil/DMDUtil.h"

#define PINMAME_SETTINGS_WINDOW_X      15
#define PINMAME_SETTINGS_WINDOW_Y      30 + 218 + 5 + 75 + 5
#define PINMAME_SETTINGS_WINDOW_WIDTH  290
#define PINMAME_SETTINGS_WINDOW_HEIGHT 75
#define PINMAME_ZORDER                 200

class VPinMAMEWindow : public VP::Window
{
public:
   ~VPinMAMEWindow();

   static VPinMAMEWindow* Create();
   void SetDMD(DMDUtil::DMD* pDMD);
   void Render() override;

private:
   VPinMAMEWindow(const std::string& szTitle, VP::Window::RenderMode renderMode, int x, int y, int w, int h, int z, bool highDpi);

   DMDUtil::DMD* m_pDMD;
   DMDUtil::VirtualDMD* m_pVirtualDMD;
   int m_pitch;
   SDL_Texture* m_pTexture;
};
