// license:GPLv3+

#pragma once

#include "InGameUIPage.h"

namespace VPX::InGameUI
{

class CabinetSettingsPage final : public InGameUIPage
{
public:
   CabinetSettingsPage();

private:
   unsigned int m_delayApplyNotifId = 0;
};

}
