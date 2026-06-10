// license:GPLv3+

#pragma once

#include "InGameUIPage.h"

namespace VPX::InGameUI
{

class InputProfilePage final : public InGameUIPage
{
public:
   InputProfilePage(const string& deviceName, const std::function<void(bool, bool)>& handler);

private:
   void BuildPage() override;
   bool m_dontAskAgain = false;
   string m_deviceName;
   std::function<void(bool, bool)> m_handler;
};

}
