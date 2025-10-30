// license:GPLv3+

#pragma once

#include "InGameUIPage.h"

namespace VPX::InGameUI
{

class AudioSettingsPage final : public InGameUIPage
{
public:
   AudioSettingsPage();

private:
   bool m_lockVolume = true;
   vector<string> m_devices;
};

}
