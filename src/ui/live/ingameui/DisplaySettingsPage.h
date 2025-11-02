// license:GPLv3+

#pragma once

#include "InGameUIPage.h"

#include "plugins/VPXPlugin.h"

namespace VPX::InGameUI
{

class DisplayHomePage final : public InGameUIPage
{
public:
   DisplayHomePage();
};


class DisplaySettingsPage final : public InGameUIPage
{
public:
   explicit DisplaySettingsPage(VPXWindowId wndId);

private:
   void BuildPage();
   void BuildWindowPage();
   void BuildEmbeddedPage();
   VPX::RenderOutput& GetOutput(VPXWindowId wndId);

   const VPXWindowId m_wndId;
   vector<Window::DisplayConfig> m_displays;
   vector<string> m_displayNames;
   int m_arLock = 0;
   unsigned int m_delayApplyNotifId = 0;
};

}
