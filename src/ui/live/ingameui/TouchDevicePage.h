// license:GPLv3+

#pragma once

#include "InGameUIPage.h"

namespace VPX::InGameUI
{

class TouchDevicePage final : public InGameUIPage
{
public:
   TouchDevicePage();

private:
   void BuildPage() override;
};

}
