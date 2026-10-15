// license:GPLv3+

#include "core/stdafx.h"
#include "PluginSettingsPage.h"

#include "core/player.h"
#include "plugins/MsgPluginManager.h"
#include "ui/live/LiveUI.h"
#include "ui/live/ingameui/InGameUI.h"

using namespace MsgPI;
using namespace VPX::Properties;

namespace VPX::InGameUI
{

PluginHomePage::PluginHomePage()
   : InGameUIPage("Plugin"s, ""s, SaveMode::None)
{
}

void PluginHomePage::BuildPage()
{
   const MsgPluginManager& manager = m_player->m_pluginManager;
   for (const auto& plugin : manager.GetPlugins())
   {
      const string& id = plugin->m_id;
      if (id == "vpx") // Do not expose core VPX plugin (disabling it would crash the app)
         continue;
      // The plugin's landing page is its root sheet: the declared isRoot sheet, or the "settings"
      // sheet bridging legacy RegisterSetting entries (see PluginSheetPage pluginRoot mode)
      m_player->m_liveUI->m_inGameUI.AddPage("plugin/" + plugin->m_id, [id]() { return std::make_unique<PluginSheetPage>(id, ""s, ""s, true); });
      AddItem(std::make_unique<InGameUIItem>(plugin->m_name, plugin->m_description, "plugin/" + plugin->m_id));
   }
}

namespace
{

   std::shared_ptr<VPXPluginPagedUI::UISheet> FindSheet(const string& pluginId, const string& sheetId)
   {
      return g_pplayer != nullptr ? g_pplayer->m_pluginAPI.PluginUI().FindUISheet(pluginId, sheetId) : nullptr;
   }

   // The sheet providing the plugin's landing page (plugin declared isRoot sheet, else "settings")
   std::shared_ptr<VPXPluginPagedUI::UISheet> FindRootSheet(const string& pluginId) { return g_pplayer != nullptr ? g_pplayer->m_pluginAPI.PluginUI().FindRootUISheet(pluginId) : nullptr; }

   // Index of a page inside a sheet (-1 when unknown); an empty pageId selects the root page
   int ResolvePageIndex(const std::shared_ptr<VPXPluginPagedUI::UISheet>& sheet, const string& pageId)
   {
      if (sheet == nullptr)
         return -1;
      const unsigned int nPages = sheet->def.GetPageCount(sheet->def.context);
      if (pageId.empty())
         return nPages > 0 ? 0 : -1;
      for (unsigned int i = 0; i < nPages; i++)
      {
         PagedUIPageInfo info {};
         sheet->def.GetPageInfo(sheet->def.context, i, &info);
         if (info.id != nullptr && pageId == info.id)
            return static_cast<int>(i);
      }
      return -1;
   }

   string ResolvePageId(const std::shared_ptr<VPXPluginPagedUI::UISheet>& sheet, const string& pageId)
   {
      const int index = ResolvePageIndex(sheet, pageId);
      if (index < 0)
         return pageId;
      PagedUIPageInfo info {};
      sheet->def.GetPageInfo(sheet->def.context, index, &info);
      return info.id != nullptr ? info.id : pageId;
   }

   string ResolveTitle(const string& pluginId, const string& sheetId, const string& pageId, bool pluginRoot)
   {
      if (pluginRoot)
      {
         const auto plugin = g_pplayer != nullptr ? g_pplayer->m_pluginManager.GetPlugin(pluginId) : nullptr;
         return plugin != nullptr ? plugin->m_name : pluginId;
      }
      const auto sheet = FindSheet(pluginId, sheetId);
      if (sheet == nullptr)
         return pageId;
      const int index = ResolvePageIndex(sheet, pageId);
      if (index < 0)
         return sheet->name;
      PagedUIPageInfo info {};
      sheet->def.GetPageInfo(sheet->def.context, index, &info);
      if (info.title != nullptr)
         return info.title;
      return index == 0 ? sheet->name : (info.id != nullptr ? info.id : ""s);
   }

   string ResolveInfo(const string& pluginId, const string& sheetId, const string& pageId, bool pluginRoot)
   {
      if (pluginRoot)
      {
         const auto plugin = g_pplayer != nullptr ? g_pplayer->m_pluginManager.GetPlugin(pluginId) : nullptr;
         return plugin != nullptr ? std::format("{}\nBy {}\nVersion {}", plugin->m_description, plugin->m_author, plugin->m_version) : ""s;
      }
      const auto sheet = FindSheet(pluginId, sheetId);
      if (sheet == nullptr)
         return ""s;
      const int index = ResolvePageIndex(sheet, pageId);
      if (index < 0)
         return ""s;
      PagedUIPageInfo info {};
      sheet->def.GetPageInfo(sheet->def.context, index, &info);
      return info.info != nullptr ? info.info : ""s;
   }

   InGameUIPage::SaveMode ResolveSaveMode(const string& pluginId, const string& sheetId, bool pluginRoot)
   {
      const auto sheet = pluginRoot ? FindRootSheet(pluginId) : FindSheet(pluginId, sheetId);
      if (sheet == nullptr)
         // A plugin landing page without any sheet still needs the save flow for the enable entry
         return pluginRoot ? InGameUIPage::SaveMode::Both : InGameUIPage::SaveMode::None;
      VPXPluginPagedUI::ScanSheetItems(*sheet);
      if (sheet->hasHostItems)
         // Host decides if values are persisted application wide or as table overrides (standard settings flow)
         return InGameUIPage::SaveMode::Both;
      if (sheet->def.CommitEdit != nullptr)
         // Plugin stored documents are saved through CommitEdit (Save() is overriden): any non None save
         // mode just enables the save button. The callback (not the item set) is what marks the sheet as
         // a persisted document: the item set may be empty or action-only, e.g. before the first element
         // of a document is added through the page
         return InGameUIPage::SaveMode::Global;
      return InGameUIPage::SaveMode::None;
   }

} // namespace


PluginSheetPage::PluginSheetPage(const string& pluginId, const string& sheetId, const string& pageId, bool pluginRoot)
   : InGameUIPage(ResolveTitle(pluginId, sheetId, pageId, pluginRoot), ResolveInfo(pluginId, sheetId, pageId, pluginRoot), ResolveSaveMode(pluginId, sheetId, pluginRoot))
   , m_sheet(pluginRoot ? FindRootSheet(pluginId) : FindSheet(pluginId, sheetId))
   , m_pluginRoot(pluginRoot)
   , m_pluginId(pluginId)
   , m_sheetId(m_sheet != nullptr ? m_sheet->id : sheetId)
   , m_pageId(ResolvePageId(m_sheet, pluginRoot ? ""s : pageId))
{
   if (m_sheet != nullptr)
      VPXPluginPagedUI::OnSheetPageOpened(*m_sheet);
}

PluginSheetPage::~PluginSheetPage()
{
   if (m_sheet != nullptr)
      VPXPluginPagedUI::OnSheetPageClosed(*m_sheet);
}

bool PluginSheetPage::IsModified() const
{
   if (m_sheet != nullptr && !m_sheet->retired && m_sheet->def.IsDirty != nullptr && m_sheet->def.IsDirty(m_sheet->def.context))
      return true;
   return InGameUIPage::IsModified();
}

void PluginSheetPage::Save()
{
   // The item set may have been enumerated when the document was still empty (hasPluginItems false):
   // the commit callback is what marks the sheet as a persisted plugin document
   if (m_sheet != nullptr && (m_sheet->hasPluginItems || m_sheet->def.CommitEdit != nullptr))
   {
      // Plugin stored items are persisted through the document commit, before the host settings save flow
      if (!IsModified())
         return;
      if (m_sheet->retired || m_sheet->def.CommitEdit == nullptr || m_sheet->def.CommitEdit(m_sheet->def.context) != 0)
      {
         m_player->m_liveUI->PushNotification("Failed to save '"s + m_sheet->name + "'"s, 5000);
         return;
      }
      m_sheet->editCommitted = 1;
      VPXPluginPagedUI::RefreshSheetSnapshot(*m_sheet);
      if (!m_sheet->hasHostItems)
      {
         m_player->m_liveUI->PushNotification("'"s + m_sheet->name + "' was saved"s, 5000);
         return;
      }
   }
   InGameUIPage::Save();
}

void PluginSheetPage::BuildPage()
{
   if (m_pluginRoot)
   {
#ifdef _WIN32
      AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Warning: Plugins are a beta experimental feature, not yet considered stable"s));
#endif
      AddPluginEnableItem();
   }
   if (m_sheet == nullptr || m_sheet->retired || m_pageId.empty())
   {
      if (!m_pluginRoot)
         AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Info, "This content is no longer available."s));
   }
   else
   {
      const unsigned int nItems = m_sheet->def.GetItemCount(m_sheet->def.context, m_pageId.c_str());
      vector<PagedUIItemDef> items(nItems);
      if (nItems > 0)
         m_sheet->def.GetItems(m_sheet->def.context, m_pageId.c_str(), items.data());
      for (const auto& item : items)
         AddItemDef(item);
   }
   if (m_pluginRoot)
      AddSheetNavItems();
}

// Plugin landing page header: the plugin enable/disable entry (on top of the root sheet's items)
void PluginSheetPage::AddPluginEnableItem()
{
   const MsgPI::MsgPluginManager& manager = m_player->m_pluginManager;
   const auto plugin = manager.GetPlugin(m_pluginId);
   const auto enablePropId = Settings::GetRegistry().GetPropertyId("Plugin." + m_pluginId, "Enable"s);
   if (plugin == nullptr || !enablePropId.has_value())
   {
      AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Info, "Internal error..."s));
      return;
   }
   AddItem(std::make_unique<InGameUIItem>( //
              *enablePropId, //
              [this]()
              {
                 const MsgPI::MsgPluginManager& manager = m_player->m_pluginManager;
                 const auto plugin = manager.GetPlugin(m_pluginId);
                 return plugin ? plugin->IsLoaded() : false;
              }, // Live
              [this](bool v)
              {
                 MsgPI::MsgPluginManager& manager = m_player->m_pluginManager;
                 auto& plugin = *manager.GetPlugin(m_pluginId);
                 if (v && !plugin.IsLoaded())
                    manager.LoadPlugin(plugin);
                 else if (!v && plugin.IsLoaded())
                 {
                    if (m_player->m_scriptInterpreter && m_player->m_pluginAPI.IsScriptContributor(plugin.m_endpointId))
                    {
                       m_player->m_scriptInterpreter->Stop(m_player->m_ptable);
                       ULONG refCount = m_player->m_scriptInterpreter->Release();
                       m_player->m_scriptInterpreter = nullptr;
                       assert(refCount == 0);
                       m_player->m_liveUI->PushNotification("The plugin you have disabled contributed to the script engine.\nTherefore, the script was stopped to prevent issues.", 5000);
                    }
                    manager.UnloadPlugin(plugin);
                 }
                 RequestRebuild();
              }))
      .m_excludeFromDefault
      = true;
}

// Plugin landing page footer: navigation entries to the plugin's other sheets
void PluginSheetPage::AddSheetNavItems()
{
   vector<VPXPluginPagedUI::UISheet*> sheets;
   m_player->m_pluginAPI.PluginUI().GetUISheets(m_pluginId, sheets);
   for (const auto& sheet : sheets)
   {
      if (m_sheet != nullptr && sheet->id == m_sheet->id)
         continue; // The displayed root sheet is not listed again
      const string path = RootPagePath(m_pluginId, sheet->id);
      const string pluginId = m_pluginId, sheetId = sheet->id;
      m_player->m_liveUI->m_inGameUI.AddPage(path, [pluginId, sheetId]() { return std::make_unique<PluginSheetPage>(pluginId, sheetId, ""s); });
      AddItem(std::make_unique<InGameUIItem>(sheet->name, sheet->description, path));
   }
}

void PluginSheetPage::AddItemDef(const PagedUIItemDef& item)
{
   if (item.flags & PAGEDUI_ITEMF_HIDDEN)
      return;
   const string label = item.label != nullptr ? item.label : ""s;
   const string tooltip = item.tooltip != nullptr ? item.tooltip : ""s;
   void* const ictx = item.context;
   const std::shared_ptr<VPXPluginPagedUI::UISheet> sheet = m_sheet;
   switch (item.type)
   {
   case PAGEDUI_ITEM_HEADER: AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, label, tooltip)); return;
   case PAGEDUI_ITEM_INFO: AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Info, label, tooltip)); return;
   case PAGEDUI_ITEM_MARKDOWN: AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Markdown, label, tooltip)); return;
   case PAGEDUI_ITEM_ACTION:
      AddItem(std::make_unique<InGameUIItem>(label, tooltip,
         [sheet, run = item.action.Run, ictx]()
         {
            if (run != nullptr && !sheet->retired)
               run(ictx);
         }));
      return;
   case PAGEDUI_ITEM_SUBPAGE:
      if (item.subpage.pageId != nullptr)
      {
         const string path = PagePath(m_pluginId, m_sheetId, item.subpage.pageId);
         const string pluginId = m_pluginId, sheetId = m_sheetId, subPageId = item.subpage.pageId;
         m_player->m_liveUI->m_inGameUI.AddPage(path, [pluginId, sheetId, subPageId]() { return std::make_unique<PluginSheetPage>(pluginId, sheetId, subPageId); });
         AddItem(std::make_unique<InGameUIItem>(label, tooltip, path));
      }
      return;
   case PAGEDUI_ITEM_FLOAT:
   case PAGEDUI_ITEM_INT:
   case PAGEDUI_ITEM_ENUM:
   case PAGEDUI_ITEM_BOOL:
   case PAGEDUI_ITEM_STRING:
   case PAGEDUI_ITEM_FILE:
   case PAGEDUI_ITEM_FOLDER: AddPropItem(item); return;
   default:
      // Unknown item type (this version of the API does not define it)
      AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Info, "(Unsupported content)"s));
      return;
   }
}

void PluginSheetPage::AddPropItem(const PagedUIItemDef& item)
{
   if (item.id == nullptr || *item.id == '\0')
   {
      PLOGE << "Plugin sheet '" << m_sheetId << "' declares a property item without an id (ignored)";
      return;
   }
   const string label = item.label != nullptr ? item.label : ""s;
   const string tooltip = item.tooltip != nullptr ? item.tooltip : ""s;
   void* const ictx = item.context;
   const std::shared_ptr<VPXPluginPagedUI::UISheet> sheet = m_sheet;
   const string itemKey = item.id;

   InGameUIItem* uiItem = nullptr;
   if (item.storage == PAGEDUI_STORAGE_HOST)
   {
      // Host stored values are persisted through the settings registry: use the PropId backed item constructors
      const auto propId = m_player->m_pluginAPI.PluginUI().EnsureSheetHostProp(*m_sheet, item);
      if (!propId.has_value())
         return;
      switch (item.type)
      {
      case PAGEDUI_ITEM_FLOAT:
         uiItem = &AddItem(std::make_unique<InGameUIItem>(
            *propId, //
            item.floatProp.displayScale != 0.f ? item.floatProp.displayScale : 1.f, item.floatProp.format != nullptr ? item.floatProp.format : "%4.1f"s,
            [sheet, get = item.floatProp.Get, ictx]() { return (get == nullptr || sheet->retired) ? 0.f : get(ictx); },
            [sheet, set = item.floatProp.Set, ictx](float, float v)
            {
               if (set != nullptr && !sheet->retired)
                  set(ictx, v);
            }));
         break;
      case PAGEDUI_ITEM_INT:
         uiItem = &AddItem(std::make_unique<InGameUIItem>(
            *propId, item.intProp.format != nullptr ? item.intProp.format : "%4d"s, [sheet, get = item.intProp.Get, ictx]() { return (get == nullptr || sheet->retired) ? 0 : get(ictx); },
            [sheet, set = item.intProp.Set, ictx](int, int v)
            {
               if (set != nullptr && !sheet->retired)
                  set(ictx, v);
            }));
         break;
      case PAGEDUI_ITEM_ENUM:
         uiItem = &AddItem(std::make_unique<InGameUIItem>(
            *propId, //
            [sheet, get = item.enumProp.Get, ictx]() { return (get == nullptr || sheet->retired) ? 0 : get(ictx); },
            [sheet, set = item.enumProp.Set, ictx](int, int v)
            {
               if (set != nullptr && !sheet->retired)
                  set(ictx, v);
            }));
         break;
      case PAGEDUI_ITEM_BOOL:
         uiItem = &AddItem(std::make_unique<InGameUIItem>(
            *propId, //
            [sheet, get = item.boolProp.Get, ictx]() { return (get == nullptr || sheet->retired) ? false : get(ictx) != 0; },
            [sheet, set = item.boolProp.Set, ictx](bool v)
            {
               if (set != nullptr && !sheet->retired)
                  set(ictx, v ? 1 : 0);
            }));
         break;
      case PAGEDUI_ITEM_STRING:
      case PAGEDUI_ITEM_FILE:
      case PAGEDUI_ITEM_FOLDER:
      {
         const auto get = item.type == PAGEDUI_ITEM_FILE ? item.fileProp.Get : item.type == PAGEDUI_ITEM_FOLDER ? item.folderProp.Get : item.stringProp.Get;
         const auto set = item.type == PAGEDUI_ITEM_FILE ? item.fileProp.Set : item.type == PAGEDUI_ITEM_FOLDER ? item.folderProp.Set : item.stringProp.Set;
         uiItem = &AddItem(std::make_unique<InGameUIItem>(
            *propId, //
            [sheet, get, ictx]()
            {
               const char* const v = (get == nullptr || sheet->retired) ? nullptr : get(ictx);
               return v != nullptr ? string(v) : ""s;
            },
            [sheet, set, ictx](const string&, const string& v)
            {
               if (set != nullptr && !sheet->retired)
                  set(ictx, v.c_str());
            }));
         break;
      }
      default: break;
      }
   }
   else
   {
      // Plugin stored documents: the values are snapshotted in the sheet registry at session start, which
      // reuses the host modified state tracking and 'reset to stored value' undo machinery
      auto& snapshot = sheet->docSnapshot;
      const string groupId = "Plugin."s + sheet->pluginId; // Unregistered group, only used to hold the inline property def
      switch (item.type)
      {
      case PAGEDUI_ITEM_FLOAT:
      {
         if (!snapshot.contains(itemKey) && item.floatProp.Get != nullptr)
            snapshot[itemKey] = item.floatProp.Get(ictx);
         const FloatPropertyDef prop(groupId, itemKey, label, tooltip, false, item.floatProp.minVal, item.floatProp.maxVal, item.floatProp.step, item.floatProp.defVal);
         uiItem = &AddItem(std::make_unique<InGameUIItem>(
            prop, //
            item.floatProp.displayScale != 0.f ? item.floatProp.displayScale : 1.f, item.floatProp.format != nullptr ? item.floatProp.format : "%4.1f"s,
            [sheet, get = item.floatProp.Get, ictx]() { return (get == nullptr || sheet->retired) ? 0.f : get(ictx); },
            [sheet, itemKey](const Settings&)
            {
               const auto it = sheet->docSnapshot.find(itemKey);
               return (it != sheet->docSnapshot.end() && std::holds_alternative<float>(it->second)) ? std::get<float>(it->second) : 0.f;
            },
            [sheet, set = item.floatProp.Set, ictx](float, float v)
            {
               if (set != nullptr && !sheet->retired)
                  set(ictx, v);
            },
            [](Settings&) {}, [sheet, itemKey](float v, Settings&, bool) { sheet->docSnapshot[itemKey] = v; }));
         break;
      }
      case PAGEDUI_ITEM_INT:
      {
         if (!snapshot.contains(itemKey) && item.intProp.Get != nullptr)
            snapshot[itemKey] = item.intProp.Get(ictx);
         const IntPropertyDef prop(groupId, itemKey, label, tooltip, false, item.intProp.minVal, item.intProp.maxVal, item.intProp.defVal);
         uiItem = &AddItem(std::make_unique<InGameUIItem>(
            prop, item.intProp.format != nullptr ? item.intProp.format : "%4d"s, [sheet, get = item.intProp.Get, ictx]() { return (get == nullptr || sheet->retired) ? 0 : get(ictx); },
            [sheet, itemKey](const Settings&)
            {
               const auto it = sheet->docSnapshot.find(itemKey);
               return (it != sheet->docSnapshot.end() && std::holds_alternative<int>(it->second)) ? std::get<int>(it->second) : 0;
            },
            [sheet, set = item.intProp.Set, ictx](int, int v)
            {
               if (set != nullptr && !sheet->retired)
                  set(ictx, v);
            },
            [](Settings&) {}, [sheet, itemKey](int v, Settings&, bool) { sheet->docSnapshot[itemKey] = v; }));
         break;
      }
      case PAGEDUI_ITEM_ENUM:
      {
         if (!snapshot.contains(itemKey) && item.enumProp.Get != nullptr)
            snapshot[itemKey] = item.enumProp.Get(ictx);
         vector<string> values;
         for (int i = 0; i < item.enumProp.nValues; i++)
            values.emplace_back(item.enumProp.values[i]);
         const EnumPropertyDef prop(groupId, itemKey, label, tooltip, false, item.enumProp.minVal, item.enumProp.defVal, values);
         uiItem = &AddItem(std::make_unique<InGameUIItem>(
            prop, //
            [sheet, get = item.enumProp.Get, ictx]() { return (get == nullptr || sheet->retired) ? 0 : get(ictx); },
            [sheet, itemKey](const Settings&)
            {
               const auto it = sheet->docSnapshot.find(itemKey);
               return (it != sheet->docSnapshot.end() && std::holds_alternative<int>(it->second)) ? std::get<int>(it->second) : 0;
            },
            [sheet, set = item.enumProp.Set, ictx](int, int v)
            {
               if (set != nullptr && !sheet->retired)
                  set(ictx, v);
            },
            [](Settings&) {}, [sheet, itemKey](int v, Settings&, bool) { sheet->docSnapshot[itemKey] = v; }));
         break;
      }
      case PAGEDUI_ITEM_BOOL:
      {
         if (!snapshot.contains(itemKey) && item.boolProp.Get != nullptr)
            snapshot[itemKey] = item.boolProp.Get(ictx) != 0;
         const BoolPropertyDef prop(groupId, itemKey, label, tooltip, false, item.boolProp.defVal != 0);
         uiItem = &AddItem(std::make_unique<InGameUIItem>(
            prop, //
            [sheet, get = item.boolProp.Get, ictx]() { return (get == nullptr || sheet->retired) ? false : get(ictx) != 0; },
            [sheet, itemKey](const Settings&)
            {
               const auto it = sheet->docSnapshot.find(itemKey);
               return (it != sheet->docSnapshot.end() && std::holds_alternative<bool>(it->second)) ? std::get<bool>(it->second) : false;
            },
            [sheet, set = item.boolProp.Set, ictx](bool v)
            {
               if (set != nullptr && !sheet->retired)
                  set(ictx, v ? 1 : 0);
            },
            [](Settings&) {}, [sheet, itemKey](bool v, Settings&, bool) { sheet->docSnapshot[itemKey] = v; }));
         break;
      }
      case PAGEDUI_ITEM_STRING:
      case PAGEDUI_ITEM_FILE:
      case PAGEDUI_ITEM_FOLDER:
      {
         const auto get = item.type == PAGEDUI_ITEM_FILE ? item.fileProp.Get : item.type == PAGEDUI_ITEM_FOLDER ? item.folderProp.Get : item.stringProp.Get;
         const auto set = item.type == PAGEDUI_ITEM_FILE ? item.fileProp.Set : item.type == PAGEDUI_ITEM_FOLDER ? item.folderProp.Set : item.stringProp.Set;
         const char* const defVal = item.type == PAGEDUI_ITEM_FILE ? item.fileProp.defVal : item.type == PAGEDUI_ITEM_FOLDER ? item.folderProp.defVal : item.stringProp.defVal;
         if (!snapshot.contains(itemKey) && get != nullptr)
            if (const char* const v = get(ictx); v != nullptr)
               snapshot[itemKey] = string(v);
         const StringPropertyDef prop(groupId, itemKey, label, tooltip, false, defVal != nullptr ? defVal : "",
            item.type == PAGEDUI_ITEM_FILE        ? StringPropertyDef::PathType::File
               : item.type == PAGEDUI_ITEM_FOLDER ? StringPropertyDef::PathType::Folder
                                                  : StringPropertyDef::PathType::None,
            item.type == PAGEDUI_ITEM_FILE && item.fileProp.fileFilter != nullptr ? item.fileProp.fileFilter : "");
         uiItem = &AddItem(std::make_unique<InGameUIItem>(
            prop, //
            [sheet, get, ictx]()
            {
               const char* const v = (get == nullptr || sheet->retired) ? nullptr : get(ictx);
               return v != nullptr ? string(v) : ""s;
            },
            [sheet, itemKey](const Settings&)
            {
               const auto it = sheet->docSnapshot.find(itemKey);
               return (it != sheet->docSnapshot.end() && std::holds_alternative<string>(it->second)) ? std::get<string>(it->second) : ""s;
            },
            [sheet, set, ictx](const string&, const string& v)
            {
               if (set != nullptr && !sheet->retired)
                  set(ictx, v.c_str());
            },
            [](Settings&) {}, [sheet, itemKey](const string& v, Settings&, bool) { sheet->docSnapshot[itemKey] = v; }));
         break;
      }
      default: break;
      }
   }
   if (uiItem != nullptr)
   {
      uiItem->m_excludeFromDefault = (item.flags & PAGEDUI_ITEMF_EXCLUDE_DEFAULTS) != 0;
      uiItem->m_globalOnly = (item.flags & PAGEDUI_ITEMF_GLOBAL_ONLY) != 0;
   }
}
}
