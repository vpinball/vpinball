// license:GPLv3+

#pragma once

#include "InGameUIPage.h"

namespace VPX::InGameUI
{

class MiscSettingsPage final : public InGameUIPage
{
public:
   MiscSettingsPage();
   void Open(bool isBackwardAnimation) override;
   void Close(bool isBackwardAnimation) override;

private:
   void BuildPage() override;
   void RequestDynamicRendererUpdate();

   unsigned int m_difficultyNotification = 0; 
   bool m_staticPrepassDisabled = false;
};

}
