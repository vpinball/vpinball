// license:GPLv3+

#pragma once

#include "InGameUIPage.h"

namespace VPX::InGameUI
{

class VRSettingsPage final : public InGameUIPage
{
public:
   VRSettingsPage();

   bool IsDefaults() const override { return false; }
   void ResetToDefaults() override;

private:
   void BuildPage() override;
   struct rgb
   {
      int r, g, b;
   };
   rgb m_arColorKey;
   unsigned int m_notifId = 0;
};

}
