// license:GPLv3+

#pragma once

#include "unordered_dense.h"
#include "InGameUIPage.h"

class LiveUI;

namespace VPX::InGameUI
{

class InGameUI final
{
public:
   explicit InGameUI(LiveUI &liveUI);
   ~InGameUI() = default;

   void Open();
   bool IsOpened() const { return m_isOpened; }
   void Update();
   void Close();

   void AddPage(std::unique_ptr<InGameUIPage> page);
   void Navigate(const string &path);
   void NavigateBack();

   bool IsFlipperNav() const { return m_useFlipperNav; }

private:
   void HandlePageInput(const PinInput::InputState &state);
   void HandleLegacyFlyOver(const PinInput::InputState &state);

   Player *m_player;
   bool m_isOpened = false;
   bool m_useFlipperNav = false;
   PinInput::InputState m_prevInputState { 0 };
   bool m_playerPaused = false;
   ankerl::unordered_dense::map<string, std::unique_ptr<InGameUIPage>> m_pages;
   vector<string> m_navigationHistory;
   InGameUIPage* m_activePage = nullptr;
};

}
