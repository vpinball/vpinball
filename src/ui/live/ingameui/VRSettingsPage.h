// license:GPLv3+

#pragma once

namespace VPX::InGameUI
{

class VRSettingsPage final : public InGameUIPage
{
public:
   VRSettingsPage();

   bool IsDefaults() const override { return false; }
   void ResetToDefaults() override;

private:
   struct rgb
   {
      int r, g, b;
   };
   rgb m_arColorKey;
};

}
