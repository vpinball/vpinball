// license:GPLv3+

#pragma once

#include "InGameUIItem.h"

namespace VPX::InGameUI
{

class HomePage final : public InGameUIPage
{
public:
   HomePage();

   void Open(bool isBackwardAnimation) override;
};

}
