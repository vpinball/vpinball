// license:GPLv3+

#pragma once

#include "plugins/MsgPluginManager.h"
#include "plugins/PagedUIPlugin.h"

#include "core/PropertyRegistry.h"

#include "unordered_dense.h"

#include <optional>
#include <variant>

class Settings;

// Host side implementation of the plugin contributed user interfaces:
// - MsgPlugin settings API (RegisterSetting/SaveSetting), owning the registered setting list and
//   applying load/save of their persisted values through the property registry
// - PagedUI sheets API (see plugins/PagedUIPlugin.h): declarative pages rendered inside the
//   InGameUI, including the implicit "settings" sheet bridging legacy RegisterSetting entries
//
// The registry is instance scoped: plugins are (re)loaded for each game and register their
// settings/sheets at load. A single instance is active at a time: plugin-facing entry points are
// C function pointers without context, so they route through s_instance (set/cleared by the
// ctor/dtor; Player and the design-time editor dialog never coexist).
class VPXPluginPagedUI
{
public:
   VPXPluginPagedUI(MsgPI::MsgPluginManager& pluginManager, const MsgPluginAPI& msgApi, uint32_t hostEndpointId, Settings* settings);
   ~VPXPluginPagedUI();

   // Plugin settings (MsgPlugin settings API)
   struct PluginSetting
   {
      string pluginId;
      VPX::Properties::PropertyRegistry::PropId propId;
      MsgSettingDef* setting;
   };
   const vector<PluginSetting>& GetPluginSettings() const { return m_pluginSettings; }
   void UpdateSetting(const std::string& pluginId, MsgPI::MsgPluginManager::SettingAction action, MsgSettingDef* settingDef);

   // Plugin contributed UI sheets (see plugins/PagedUIPlugin.h)
   struct UISheet
   {
      uint32_t endpointId = 0;
      string pluginId;
      // Owned copies of the plugin provided strings (def fields point into these)
      string id;
      string name;
      string description;
      // Persistence key prefix for host stored items ("<sheetId>.", empty for the default "settings"
      // sheet so that legacy RegisterSetting entries keep their raw propId keys)
      string propKeyPrefix;
      PagedUISheetDef def {};
      // Derived content, recomputed each time the sheet's items are enumerated
      bool hasHostItems = false; // At least one host stored property item
      bool hasPluginItems = false; // At least one plugin stored property item
      // Session state
      int openPageCount = 0; // Number of live InGameUI pages of this sheet
      int editCommitted = 0; // Non zero once the user saved during the current edit session
      bool retired = false; // Unregistered while pages were still open (deferred deletion)
      // Host-side snapshot of the plugin stored item values taken when the edit session starts, used to
      // compute per item modified state and to implement undo (re-collected after each commit).
      ankerl::unordered_dense::map<string, std::variant<float, int, bool, string>> docSnapshot;
   };
   // Sheets are held through shared_ptr as UI pages may outlive the registry (game teardown order)
   std::shared_ptr<UISheet> FindUISheet(const string& pluginId, const string& sheetId);
   // Lists registered sheets (transient pointers, only valid until the next registration change)
   void GetUISheets(const string& pluginId, vector<UISheet*>& out);
   // Returns the sheet providing the plugin's landing page: the one declared with isRoot, else the
   // implicit or plugin-declared "settings" sheet (nullptr when the plugin has neither)
   std::shared_ptr<UISheet> FindRootUISheet(const string& pluginId);
   static bool IsSheetPropItem(PagedUIItemType type);
   // Enumerates all items of all pages of a sheet, calling fn for each (on the API thread)
   static void ForEachSheetItem(UISheet& sheet, const std::function<void(const PagedUIItemDef&)>& fn);
   // Recomputes the hasHostItems/hasPluginItems flags of a sheet
   static void ScanSheetItems(UISheet& sheet);
   // Registers the host backed property of a host stored sheet item (first call), returning its persisted value id.
   // On first use of an item in a session, its persisted value is pushed to the plugin (same as RegisterSetting).
   std::optional<VPX::Properties::PropertyRegistry::PropId> EnsureSheetHostProp(UISheet& sheet, const PagedUIItemDef& item);
   // Tracks open/close of sheet pages to bracket plugin edit sessions (BeginEdit/EndEdit)
   static void OnSheetPageOpened(UISheet& sheet);
   static void OnSheetPageClosed(UISheet& sheet);
   // Re-collects all plugin stored item values of a sheet into its snapshot (after a successful commit)
   static void RefreshSheetSnapshot(UISheet& sheet);

private:
   MsgPI::MsgPluginManager& m_pluginManager;
   const MsgPluginAPI& m_msgApi;

   // Single active instance for C entry points routing (see class comment)
   static VPXPluginPagedUI* s_instance;

   // Design-time settings target: when set, plugin settings are persisted to this object (e.g. the
   // edited table's settings) instead of the active app settings.
   Settings* const m_settings;

   // PagedUI plugin API (broadcast query handler and dispatch table)
   static void MSGPIAPI OnGetUIPluginAPI(const unsigned int msgId, void* userData, void* msgData);
   static void MSGPIAPI RegisterSheet(const uint32_t endpointId, const PagedUISheetDef* sheet);
   static void MSGPIAPI UnregisterSheet(const uint32_t endpointId, const char* sheetId);
   static void MSGPIAPI RequestSheetRefresh(const uint32_t endpointId, const char* sheetId);
   void CollectRetiredSheets(); // Erase unregistered sheets once no page uses them anymore
   // Registers the host backed properties of a host stored sheet and pushes their persisted values to the plugin
   void PushSheetValues(UISheet& sheet);
   // Materializes the implicit "settings" sheet bridging a plugin's legacy RegisterSetting entries
   void EnsureImplicitSettingsSheet(const string& pluginId);
   static unsigned int MSGPIAPI ImplicitSettingsPageCount(void* ctx);
   static void MSGPIAPI ImplicitSettingsPageInfo(void* ctx, unsigned int index, PagedUIPageInfo* info);
   static unsigned int MSGPIAPI ImplicitSettingsItemCount(void* ctx, const char* pageId);
   static void MSGPIAPI ImplicitSettingsGetItems(void* ctx, const char* pageId, PagedUIItemDef* items);

   vector<PluginSetting> m_pluginSettings;
   vector<std::shared_ptr<UISheet>> m_uiSheets;
   ankerl::unordered_dense::set<string> m_pushedSheetItems; // Host stored items pushed to the plugin this session
   PagedUIPluginAPI m_uiApi;
   const unsigned int m_getUIAPIMsgId;
};
