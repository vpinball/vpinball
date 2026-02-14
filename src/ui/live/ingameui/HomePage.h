// license:GPLv3+

#pragma once

#include "InGameUIPage.h"

namespace VPX::InGameUI
{

class HomePage final : public InGameUIPage
{
public:
   HomePage();

   void Render(float elapsedMs) override;

private:
   void BuildPage();

};

}
