// license:GPLv3+

#pragma once

#include "InGameUIItem.h"

namespace VPX::InGameUI
{

class GraphicSettingsPage final : public InGameUIPage
{
public:
   GraphicSettingsPage();

private:
   void BuildPage();
   unsigned int m_notificationId = 0;
};

}
