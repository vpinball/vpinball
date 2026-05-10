// license:GPLv3+

#pragma once

#include "InGameUIPage.h"

namespace VPX::InGameUI
{

class InputProfilePage final : public InGameUIPage
{
public:
   InputProfilePage(const string& deviceName, const std::function<void(bool, bool, bool)>& handler);

private:
   bool m_dontAskAgain = false;
   std::function<void(bool, bool, bool)> m_handler;
};

}
