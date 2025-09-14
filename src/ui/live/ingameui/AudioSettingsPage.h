// license:GPLv3+

#pragma once

#include "InGameUIItem.h"

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
