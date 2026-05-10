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

   void Open(const string& page);
   bool IsOpened() const { return GetActivePage() != nullptr && GetActivePage()->IsActive(); }
   bool IsOpened(const string &page) const { return IsOpened() && m_navigationHistory.back() == page; }
   void Update();
   void Close();

   bool ProposeInputLayout(const string& deviceName, const std::function<void(bool, bool, bool)>& handler);

   void AddPage(const string &path, const std::function<std::unique_ptr<InGameUIPage>()>& pageFactory);
   void Navigate(const string &path, bool isBack = false);
   void NavigateBack();

   bool IsFlipperNav() const { return m_useFlipperNav; }

private:
   void HandlePageInput(const InputManager::ActionState &state);
   void HandleLegacyFlyOver(const InputManager::ActionState &state);
   InGameUIPage* GetActivePage() const { return m_activePages.empty() ? nullptr : m_activePages.back().get(); }

   Player *m_player;
   ImVec2 m_prevMousePos;
   bool m_useFlipperNav = false;
   int m_flipperNavRepeatCount = 0;
   uint32_t m_flipperNavStart = 0;
   InputManager::ActionState m_prevActionState { };
   ankerl::unordered_dense::map<string, std::function<std::unique_ptr<InGameUIPage>()>> m_pages;
   vector<string> m_navigationHistory;
   vector<std::unique_ptr<InGameUIPage>> m_activePages;
   uint32_t m_lastRenderMs = 0;
};

}
