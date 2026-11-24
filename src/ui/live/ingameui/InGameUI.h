// license:GPLv3+

#pragma once

#include "unordered_dense.h"
#include "InGameUIPage.h"
#include "InGameUIRenderer.h"

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
   // Gameplay analog input (plunger, nudge) is suspended while the UI is opened, as it overlaps UI commands, except on pages
   // that keep the game running to test that input live (plunger/nudge settings, see IsPlayerPauseAllowed)
   bool IsGameplayInputSuspended() const { return IsOpened() && GetActivePage()->IsPlayerPauseAllowed(); }
   void Update();
   void Close();

   bool ProposeInputLayout(const string& deviceName, const std::function<void(bool, bool)>& handler);

   void AddPage(const string &path, const std::function<std::unique_ptr<InGameUIPage>()>& pageFactory);
   void Navigate(const string &path, bool isBack = false);
   // Tab-style navigation used by the desktop renderer: the target replaces every
   // page below the navigation root instead of being pushed on top of them
   void NavigateTo(const string& path);
   void NavigateBack();
   void RequestRebuildAll();

   // Renderer-facing API: navigation location and the current root page instance
   // (its items are the table of contents used to lay out sectioned navigation)
   const vector<string>& GetNavigationHistory() const { return m_navigationHistory; }
   InGameUIPage* GetRootPage() const { return m_rootPage; }
   InGameUIPage* GetActivePage() const { return m_activePages.empty() ? nullptr : m_activePages.back().get(); }

   bool IsFlipperNav() const { return m_useFlipperNav; }
   void OnUIUpAction();
   void OnUIDownAction();
   void OnUILeftAction();
   void OnUIRightAction();
   void OnUIResetToDefaults();
   void OnUICancelChanges();
   void OnUISaveChanges();
   void OnUINavigateBack();

private:
   void HandlePageInput();

   Player *m_player;
   ImVec2 m_prevMousePos;
   bool m_useFlipperNav = false;
   bool m_itemAdjustWasPressed = false;
   int m_flipperNavRepeatCount = 0;
   uint32_t m_flipperNavStart = 0;
   ankerl::unordered_dense::map<string, std::function<std::unique_ptr<InGameUIPage>()>> m_pages;
   vector<string> m_navigationHistory;
   InGameUIPage* m_rootPage = nullptr; // Instance of the first navigation history entry (may have been closed and discarded)
   vector<std::unique_ptr<InGameUIPage>> m_activePages;
   uint32_t m_lastRenderMs = 0;

   // Renders the active pages; implementations selected per running target
   // (mobile touch, desktop, cabinet, VR) through InGameUIRenderer::Create
   const std::unique_ptr<InGameUIRenderer> m_renderer;
};

}
