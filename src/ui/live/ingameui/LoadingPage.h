// license:GPLv3+

#pragma once

#include "InGameUIPage.h"

namespace VPX::InGameUI
{

class LoadingPage final : public InGameUIPage
{
public:
   LoadingPage();
   bool IsPlayerPauseAllowed() const override { return false; }
   void BuildPage() override;
};

}
