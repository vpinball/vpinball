// license:GPLv3+

#pragma once

#include "InGameUIPage.h"

namespace VPX::InGameUI
{

class InputProfilePage final : public InGameUIPage
{
public:
   InputProfilePage(const string& deviceName, const std::function<void(bool, bool)>& handler);

   void Open(bool isBackwardAnimation) override;
   void Render(float elapsedS) override;

private:
   void BuildPage();

   float m_elapsed = 5.f;
   bool m_dontAskAgain = false;
   std::function<void(bool, bool)> m_handler;
   const string m_deviceName;
};

}
