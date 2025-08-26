// license:GPLv3+

#pragma once


namespace VPX::InGameUI
{

class MiscSettingsPage : public InGameUIPage
{
public:
   MiscSettingsPage();

private:
   bool m_staticPrepassDisabled = false;
};

};