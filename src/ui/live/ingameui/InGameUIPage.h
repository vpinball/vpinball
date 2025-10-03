// license:GPLv3+

#pragma once

#include "InGameUIItem.h"

namespace VPX::InGameUI
{

class InGameUIPage
{
public:
   enum class SaveMode { None, Global, Table, Both };

   InGameUIPage(const string& path, const string& title, const string& info, SaveMode saveMode);

   Settings& GetSettings();

   void ClearItems();
   void AddItem(std::unique_ptr<InGameUIItem>& item);

   virtual void Open();
   virtual void Close();
   virtual void Save();
   void SaveGlobally();
   void SaveTableOverride();
   virtual void ResetToInitialValues();
   virtual void ResetToDefaults();
   bool IsResettingToDefaults() const { return m_resettingToDefaults; }
   bool IsResettingToInitialValues() const { return m_resettingToInitialValues; }
   bool IsResetting() const { return m_resettingToDefaults || m_resettingToInitialValues; }
   virtual void Render();
   void SelectNextItem();
   void SelectPrevItem();
   void AdjustItem(float direction, bool isInitialPress);
   bool IsAdjustable() const;
   virtual bool IsDefaults() const;
   bool IsModified() const;

   InGameUIItem* GetItem(const string& label) const;

   const string &GetPath() const { return m_path; }

   virtual bool IsPlayerPauseAllowed() const { return true; }

   ImVec2 GetWindowPos() const { return m_windowPos; }
   ImVec2 GetWindowSize() const { return m_windowSize; }

   Player* const m_player;

private:
   const string m_path;
   const string m_title;
   const string m_info;
   const SaveMode m_saveMode;
   ImVec2 m_windowPos = ImVec2(0.f, 0.f);
   ImVec2 m_windowSize = ImVec2(0.f, 0.f);
   float m_adjustedValue = 0.f;
   uint32_t m_lastUpdateMs = 0;
   uint32_t m_pressStartMs = 0;
   string m_pressedItemLabel = ""s;
   float m_pressedItemScroll = 0.f;
   vector<std::unique_ptr<InGameUIItem>> m_items;
   int m_selectedItem = 0;
   unsigned int m_resetNotifId = 0;
   bool m_resettingToDefaults = false;
   bool m_resettingToInitialValues = false;

   void RenderSaveOptionPopup();
   bool m_selectGlobalOrTablePopup = false;
   bool m_selectGlobalOrDiscardPopup = false;

   void RenderInputActionPopup();
   bool m_defineActionPopup = false;
   InGameUIItem* m_defineActionItem = nullptr;

   void RenderSensorPopup();
   bool m_defineSensorPopup = false;
   InGameUIItem* m_defineSensorItem = nullptr;

   void RenderToggle(const string& label, const ImVec2& size, bool& v) const;
};

}
