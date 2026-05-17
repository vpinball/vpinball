// license:GPLv3+

#pragma once

#include "InGameUIPage.h"

namespace VPX::InGameUI
{

class InputSettingsPage final : public InGameUIPage
{
public:
   InputSettingsPage();

   void Open(bool isBackwardAnimation) override;

private:
   void BuildPage();
   InputManager& GetInput() const { return m_player->m_pininput; }

};

}
