#pragma once

#include "../common/Window.h"
#include "DMDUtil/DMDUtil.h"

#define FLEXDMD_SETTINGS_WINDOW_X      15
#define FLEXDMD_SETTINGS_WINDOW_Y      30 + 218 + 5 + 75 + 5
#define FLEXDMD_SETTINGS_WINDOW_WIDTH  290
#define FLEXDMD_SETTINGS_WINDOW_HEIGHT 75
#define FLEXDMD_ZORDER                 200

class FlexDMDWindow : public VP::Window
{
public:
   ~FlexDMDWindow();

   static FlexDMDWindow* Create();
   void SetDMD(DMDUtil::DMD* pDMD);
   void Render() override;

private:
   FlexDMDWindow(const std::string& szTitle, VP::Window::RenderMode renderMode, int x, int y, int w, int h, int z, bool highDpi);

   DMDUtil::DMD* m_pDMD;
   DMDUtil::VirtualDMD* m_pVirtualDMD;
   int m_pitch;
   Uint64 m_startTime;
   SDL_Texture* m_pTexture;
};
