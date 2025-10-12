// license:GPLv3+

#pragma once

#include "InGameUIPage.h"

namespace VPX::InGameUI
{

class ExitSplashPage final : public InGameUIPage
{
public:
   ExitSplashPage();

   void Render(float elapsedMs) override;

private:
   void BuildPage();

   uint32_t m_openTimestamp = 0; // Used to delay keyboard shortcut
};

};