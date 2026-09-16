// license:GPLv3+

#pragma once

#include "InGameUIPage.h"

namespace VPX::InGameUI
{

class LoggingSettingsPage final : public InGameUIPage
{
public:
   LoggingSettingsPage();

private:
   void BuildPage() override;
};

}
