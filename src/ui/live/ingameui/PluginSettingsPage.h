// license:GPLv3+

#pragma once

#include "InGameUIPage.h"
#include "core/VPXPluginPagedUI.h"

namespace VPX::InGameUI
{

class PluginHomePage final : public InGameUIPage
{
public:
   PluginHomePage();

private:
   void BuildPage() override;
};

// Page rendering one page of a plugin contributed UI sheet (see plugins/PagedUIPlugin.h).
// Sheets are trees of declarative items (labels, properties, actions, sub pages) which are
// translated to InGameUIItem instances at (re)build time. Pages are dynamically registered
// under 'sheet/<pluginId>/<sheetId>/<pageId>' paths.
//
// Items are persisted by whoever declares them: PAGEDUI_STORAGE_HOST items go through the
// settings registry while PAGEDUI_STORAGE_PLUGIN items are owned by the plugin which manages
// its data through the edit session callbacks (BeginEdit when the first page of the sheet is
// opened, EndEdit when the last one is closed). Plugin stored values are snapshotted into the
// sheet registry at session start to reuse the host dirty/undo machinery.
// When pluginRoot is set, the page is the plugin's landing page (its root sheet, by default the
// "settings" sheet): the plugin enable/disable entry is prepended and navigation entries to the
// plugin's other sheets are appended.
class PluginSheetPage : public InGameUIPage
{
public:
   PluginSheetPage(const string& pluginId, const string& sheetId, const string& pageId, bool pluginRoot = false);
   ~PluginSheetPage() override;

   void BuildPage() override;
   void Save() override;
   bool IsModified() const override;

   static string RootPagePath(const string& pluginId, const string& sheetId) { return "sheet/"s + pluginId + '/' + sheetId; }
   static string PagePath(const string& pluginId, const string& sheetId, const string& pageId) { return RootPagePath(pluginId, sheetId) + '/' + pageId; }

protected:
   void AddItemDef(const PagedUIItemDef& item);
   void AddPropItem(const PagedUIItemDef& item);
   // Plugin landing page additions (pluginRoot mode): enable/disable entry and navigation to the
   // plugin's other sheets
   void AddPluginEnableItem();
   void AddSheetNavItems();

   // Sheets are shared_ptr'ed as pages may outlive the API implementation (game teardown)
   std::shared_ptr<VPXPluginPagedUI::UISheet> m_sheet;
   const bool m_pluginRoot;
   const string m_pluginId;
   const string m_sheetId;
   string m_pageId;
};

}
