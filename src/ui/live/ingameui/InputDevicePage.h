// license:GPLv3+

#pragma once

#include "InGameUIPage.h"

namespace VPX::InGameUI
{

class InputDevicePage final : public InGameUIPage
{
public:
   InputDevicePage(uint16_t deviceId);

private:
   void BuildPage() override;

   const uint16_t m_deviceId;
   const string m_deviceSettingId;
};

}
