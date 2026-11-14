// license:GPLv3+

#pragma once

#include "InGameUIPage.h"

namespace VPX::InGameUI
{

class InputSettingsPage final : public InGameUIPage
{
public:
   InputSettingsPage();

private:
   void BuildPage() override;
   InputManager& GetInput() const { return m_player->m_pininput; }

};

}
