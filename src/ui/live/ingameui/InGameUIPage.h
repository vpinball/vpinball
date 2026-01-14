// license:GPLv3+

#pragma once

#include "InGameUIItem.h"

namespace VPX::InGameUI
{

class InGameUIPage
{
public:
   enum class SaveMode { None, Global, Table, Both };

   InGameUIPage(const string& title, const string& info, SaveMode saveMode);

   Settings& GetSettings();

   void ClearItems();
   InGameUIItem& AddItem(std::unique_ptr<InGameUIItem> item);

   virtual void Open(bool isBackwardAnimation);
   virtual void Close(bool isBackwardAnimation);
   virtual void Save();
   bool IsActive() const { return m_openAnimTarget == 0.f; }
   bool IsClosed() const { return (m_openAnimPos == m_openAnimTarget) && (m_openAnimPos == -1.f || m_openAnimPos == 1.f); }
   virtual void SaveGlobally();
   virtual void SaveTableOverride();
   virtual void ResetToStoredValues();
   virtual void ResetToDefaults();
   bool IsResettingToDefaults() const { return m_resettingToDefaults; }
   bool IsResettingToInitialValues() const { return m_resettingToInitialValues; }
   bool IsResetting() const { return m_resettingToDefaults || m_resettingToInitialValues; }
   virtual void Render(float elapsedS);
   virtual void SelectNextItem();
   virtual void SelectPrevItem();
   virtual void AdjustItem(float direction, bool isInitialPress);
   bool IsAdjustable() const;
   virtual bool IsDefaults() const;
   virtual bool IsModified() const;

   InGameUIItem* GetItem(const string& label) const;

   virtual bool IsPlayerPauseAllowed() const { return true; }

   ImVec2 GetWindowPos() const { return m_windowPos; }
   ImVec2 GetWindowSize() const { return m_windowSize; }
   bool IsWindowHovered() const { return m_windowHovered; }
   float GetOpenCloseAnimPos() const { return m_openAnimPos; }

   Player* const m_player;

private:
   const string m_title;
   const string m_info;
   const SaveMode m_saveMode;

   // Open/Close animations
   // -1.f = closed, 0.f = opened, 1.f = appearing
   float m_openAnimPos = -1.f;
   float m_openAnimStart = -1.f;
   float m_openAnimTarget = -1.f;
   float m_openAnimElapsed = 0.f;

   ImVec2 m_windowPos = ImVec2(0.f, 0.f);
   ImVec2 m_windowSize = ImVec2(0.f, 0.f);
   bool m_windowHovered = false;
   float m_adjustedValue = 0.f;
   uint32_t m_lastUpdateMs = 0;
   uint32_t m_pressStartMs = 0;
   string m_pressedItemLabel;
   float m_pressedItemScroll = 0.f;
   vector<std::unique_ptr<InGameUIItem>> m_items;
   int m_selectedItem = 0;
   unsigned int m_resetNotifId = 0;
   bool m_resettingToDefaults = false;
   bool m_resettingToInitialValues = false;

   void RenderInputActionPopup();
   bool m_defineActionPopup = false;
   InGameUIItem* m_defineActionItem = nullptr;

   static void RenderToggle(const string& label, const ImVec2& size, bool& v);
   static void TextWithEllipsis(const string& text, float maxWidth);
};

}
