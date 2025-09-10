// license:GPLv3+

#pragma once


namespace VPX::InGameUI
{

class AudioSettingsPage final : public InGameUIPage
{
public:
   AudioSettingsPage();

private:
   bool m_lockVolume = true;
};

}
