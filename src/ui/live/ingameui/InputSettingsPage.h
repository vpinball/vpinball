// license:GPLv3+

#pragma once

namespace VPX::InGameUI
{

class InputSettingsPage final : public InGameUIPage
{
public:
   InputSettingsPage();

   void Open(bool isBackwardAnimation) override;

private:
   InputManager& GetInput() const { return m_player->m_pininput; }

};

}
