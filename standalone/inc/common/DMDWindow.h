#pragma once

#include "../common/Window.h"
#include "DMDUtil/DMDUtil.h"

namespace VP {

class DMDWindow : public VP::Window
{
public:
   DMDWindow(const std::string& szTitle, int x, int y, int w, int h, int z);
   ~DMDWindow();

   void SetDMD(DMDUtil::DMD* pDMD);
   void Render() override;

private:
   DMDUtil::DMD* m_pDMD;
   DMDUtil::VirtualDMD* m_pVirtualDMD;
   int m_pitch;
   SDL_Texture* m_pTexture;
};

}