// license:GPLv3+

#include "core/stdafx.h"
#include "VPXPluginPagedUI.h"

#include "core/VPApp.h"
#include "core/player.h"
#include "ui/live/LiveUI.h"

VPXPluginPagedUI* VPXPluginPagedUI::s_instance = nullptr;

///////////////////////////////////////////////////////////////////////////////
// Settings API

void VPXPluginPagedUI::UpdateSetting(const std::string& pluginId, MsgPI::MsgPluginManager::SettingAction action, MsgSettingDef* settingDef)
{
   if (action == MsgPI::MsgPluginManager::SettingAction::UnregisterAll)
   {
      // We keep the property definition in the settings, but we remove reference to the plugin owned memory block with live data
      m_pluginSettings.erase(
         std::remove_if(m_pluginSettings.begin(), m_pluginSettings.end(), [&pluginId](const PluginSetting& x) { return x.pluginId == pluginId; }), m_pluginSettings.end());
      // Same for the plugin's contributed UI sheets: they are retired, then removed once no page uses them
      for (auto& sheet : m_uiSheets)
         if (sheet->pluginId == pluginId)
            sheet->retired = true;
      CollectRetiredSheets();
      // If the plugin is loaded again, it starts with its defaults: the persisted values must be pushed to it again
      const string pushPrefix = pluginId + '/';
      std::erase_if(m_pushedSheetItems, [&pushPrefix](const string& key) { return key.starts_with(pushPrefix); });
      return;
   }

   const auto item = std::ranges::find_if(
      m_pluginSettings, [&pluginId, &settingDef](const PluginSetting& setting) { return setting.pluginId == pluginId && setting.setting->propId == settingDef->propId; });

   // Register property and get or set value
   Settings& settings = m_settings != nullptr ? *m_settings : g_settingsService.GetActiveSettings();
   const bool asTableOverride = m_settings != nullptr || g_pplayer != nullptr;
   const std::string sectionName = "Plugin."s + pluginId;
   switch (settingDef->type)
   {
   case MSGPI_SETTING_TYPE_FLOAT:
   {
      const auto newId = Settings::GetRegistry().Register(std::make_unique<VPX::Properties::FloatPropertyDef>(sectionName, settingDef->propId, settingDef->name, settingDef->description,
         false, settingDef->floatDef.minVal, settingDef->floatDef.maxVal, settingDef->floatDef.step, settingDef->floatDef.defVal));
      if (item == m_pluginSettings.end())
         m_pluginSettings.emplace_back(pluginId, newId, settingDef);
      else
      {
         item->propId = newId;
         item->setting = settingDef;
      }
      if (action == MsgPI::MsgPluginManager::SettingAction::Save)
         settings.Set(newId, settingDef->floatDef.Get(), asTableOverride);
      else if (action == MsgPI::MsgPluginManager::SettingAction::Load)
         settingDef->floatDef.Set(Settings::GetRegistry().GetFloatProperty(newId)->GetValid(settings.GetFloat(newId)));
      break;
   }

   case MSGPI_SETTING_TYPE_INT:
      if (settingDef->intDef.values)
      {
         vector<string> values;
         for (int i = settingDef->intDef.minVal; i <= settingDef->intDef.maxVal; i++)
            values.emplace_back(settingDef->intDef.values[i - settingDef->intDef.minVal]);
         const auto newId = Settings::GetRegistry().Register(std::make_unique<VPX::Properties::EnumPropertyDef>(
            sectionName, settingDef->propId, settingDef->name, settingDef->description, false, settingDef->intDef.minVal, settingDef->intDef.defVal, values));
         if (item == m_pluginSettings.end())
            m_pluginSettings.emplace_back(pluginId, newId, settingDef);
         else
         {
            item->propId = newId;
            item->setting = settingDef;
         }
         if (action == MsgPI::MsgPluginManager::SettingAction::Save)
            settings.Set(newId, settingDef->intDef.Get(), asTableOverride);
         else if (action == MsgPI::MsgPluginManager::SettingAction::Load)
            settingDef->intDef.Set(Settings::GetRegistry().GetEnumProperty(newId)->GetValid(settings.GetInt(newId)));
      }
      else
      {
         const auto newId = Settings::GetRegistry().Register(std::make_unique<VPX::Properties::IntPropertyDef>(
            sectionName, settingDef->propId, settingDef->name, settingDef->description, false, settingDef->intDef.minVal, settingDef->intDef.maxVal, settingDef->intDef.defVal));
         if (item == m_pluginSettings.end())
            m_pluginSettings.emplace_back(pluginId, newId, settingDef);
         else
         {
            item->propId = newId;
            item->setting = settingDef;
         }
         if (action == MsgPI::MsgPluginManager::SettingAction::Save)
            settings.Set(newId, settingDef->intDef.Get(), asTableOverride);
         else if (action == MsgPI::MsgPluginManager::SettingAction::Load)
            settingDef->intDef.Set(Settings::GetRegistry().GetIntProperty(newId)->GetValid(settings.GetInt(newId)));
      }
      break;

   case MSGPI_SETTING_TYPE_BOOL:
   {
      const auto newId = Settings::GetRegistry().Register(
         std::make_unique<VPX::Properties::BoolPropertyDef>(sectionName, settingDef->propId, settingDef->name, settingDef->description, false, settingDef->boolDef.defVal));
      if (item == m_pluginSettings.end())
         m_pluginSettings.emplace_back(pluginId, newId, settingDef);
      else
      {
         item->propId = newId;
         item->setting = settingDef;
      }
      if (action == MsgPI::MsgPluginManager::SettingAction::Save)
         settings.Set(newId, settingDef->boolDef.Get(), asTableOverride);
      else if (action == MsgPI::MsgPluginManager::SettingAction::Load)
         settingDef->boolDef.Set(settings.GetBool(newId));
      break;
   }

   case MSGPI_SETTING_TYPE_STRING:
   {
      const auto newId = Settings::GetRegistry().Register(
         std::make_unique<VPX::Properties::StringPropertyDef>(sectionName, settingDef->propId, settingDef->name, settingDef->description, false, settingDef->stringDef.defVal));
      if (item == m_pluginSettings.end())
         m_pluginSettings.emplace_back(pluginId, newId, settingDef);
      else
      {
         item->propId = newId;
         item->setting = settingDef;
      }
      if (action == MsgPI::MsgPluginManager::SettingAction::Save)
         settings.Set(newId, settingDef->stringDef.Get(), asTableOverride);
      else if (action == MsgPI::MsgPluginManager::SettingAction::Load)
      {
         const string& value = settings.GetString(newId);
         settingDef->stringDef.Set(value.c_str());
      }
      break;
   }

   case MSGPI_SETTING_TYPE_FOLDER:
   {
      const auto newId = Settings::GetRegistry().Register(std::make_unique<VPX::Properties::StringPropertyDef>(
         sectionName, settingDef->propId, settingDef->name, settingDef->description, false, settingDef->folderDef.defVal, VPX::Properties::StringPropertyDef::PathType::Folder));
      if (item == m_pluginSettings.end())
         m_pluginSettings.emplace_back(pluginId, newId, settingDef);
      else
      {
         item->propId = newId;
         item->setting = settingDef;
      }
      if (action == MsgPI::MsgPluginManager::SettingAction::Save)
         settings.Set(newId, settingDef->folderDef.Get(), asTableOverride);
      else if (action == MsgPI::MsgPluginManager::SettingAction::Load)
      {
         const string& value = settings.GetString(newId);
         settingDef->folderDef.Set(value.c_str());
      }
      break;
   }

   case MSGPI_SETTING_TYPE_FILE:
   {
      const auto newId = Settings::GetRegistry().Register(std::make_unique<VPX::Properties::StringPropertyDef>(sectionName, settingDef->propId, settingDef->name, settingDef->description,
         false, settingDef->fileDef.defVal, VPX::Properties::StringPropertyDef::PathType::File, settingDef->fileDef.fileFilter != nullptr ? settingDef->fileDef.fileFilter : ""));
      if (item == m_pluginSettings.end())
         m_pluginSettings.emplace_back(pluginId, newId, settingDef);
      else
      {
         item->propId = newId;
         item->setting = settingDef;
      }
      if (action == MsgPI::MsgPluginManager::SettingAction::Save)
         settings.Set(newId, settingDef->fileDef.Get(), asTableOverride);
      else if (action == MsgPI::MsgPluginManager::SettingAction::Load)
      {
         const string& value = settings.GetString(newId);
         settingDef->fileDef.Set(value.c_str());
      }
      break;
   }
   }

   if (action == MsgPI::MsgPluginManager::SettingAction::Load)
   {
      // The persisted value was pushed above: mark it so the implicit sheet does not push it again
      m_pushedSheetItems.insert(pluginId + "/settings/" + settingDef->propId);
      // Legacy settings are rendered through the shared sheet pipeline via the implicit default sheet
      EnsureImplicitSettingsSheet(pluginId);
   }
}


///////////////////////////////////////////////////////////////////////////////
// UI sheets API (declarative plugin user interfaces, see plugins/PagedUIPlugin.h)

// Implicit settings sheet: bridges a plugin's legacy RegisterSetting entries into a sheet named
// "settings" so that all plugin UI flows through the same page pipeline. The items reuse the raw
// propId persistence keys (empty propKeyPrefix) and the settings already registered & pushed by
// UpdateSetting. Get/Set callbacks are thunked since MsgSettingDef callbacks take no context.
static float MSGPIAPI SettingFloatGet(void* ctx) { return static_cast<MsgSettingDef*>(ctx)->floatDef.Get(); }
static void MSGPIAPI SettingFloatSet(void* ctx, float v) { static_cast<MsgSettingDef*>(ctx)->floatDef.Set(v); }
static int MSGPIAPI SettingIntGet(void* ctx) { return static_cast<MsgSettingDef*>(ctx)->intDef.Get(); }
static void MSGPIAPI SettingIntSet(void* ctx, int v) { static_cast<MsgSettingDef*>(ctx)->intDef.Set(v); }
static int MSGPIAPI SettingBoolGet(void* ctx) { return static_cast<MsgSettingDef*>(ctx)->boolDef.Get(); }
static void MSGPIAPI SettingBoolSet(void* ctx, int v) { static_cast<MsgSettingDef*>(ctx)->boolDef.Set(v); }
static const char* MSGPIAPI SettingStringGet(void* ctx) { return static_cast<MsgSettingDef*>(ctx)->stringDef.Get(); }
static void MSGPIAPI SettingStringSet(void* ctx, const char* v) { static_cast<MsgSettingDef*>(ctx)->stringDef.Set(v); }
static const char* MSGPIAPI SettingFolderGet(void* ctx) { return static_cast<MsgSettingDef*>(ctx)->folderDef.Get(); }
static void MSGPIAPI SettingFolderSet(void* ctx, const char* v) { static_cast<MsgSettingDef*>(ctx)->folderDef.Set(v); }
static const char* MSGPIAPI SettingFileGet(void* ctx) { return static_cast<MsgSettingDef*>(ctx)->fileDef.Get(); }
static void MSGPIAPI SettingFileSet(void* ctx, const char* v) { static_cast<MsgSettingDef*>(ctx)->fileDef.Set(v); }

unsigned int MSGPIAPI VPXPluginPagedUI::ImplicitSettingsPageCount(void* ctx) { return 1; }

void MSGPIAPI VPXPluginPagedUI::ImplicitSettingsPageInfo(void* ctx, unsigned int index, PagedUIPageInfo* info)
{
   info->id = "settings";
   info->title = "Settings";
   info->info = nullptr;
}

unsigned int MSGPIAPI VPXPluginPagedUI::ImplicitSettingsItemCount(void* ctx, const char* pageId)
{
   const char* const pluginId = static_cast<const char*>(ctx);
   unsigned int n = 0;
   if (s_instance != nullptr)
      for (const auto& s : s_instance->m_pluginSettings)
         n += s.pluginId == pluginId ? 1 : 0;
   return n;
}

void MSGPIAPI VPXPluginPagedUI::ImplicitSettingsGetItems(void* ctx, const char* pageId, PagedUIItemDef* items)
{
   const char* const pluginId = static_cast<const char*>(ctx);
   if (s_instance == nullptr)
      return;
   unsigned int i = 0;
   for (const auto& ps : s_instance->m_pluginSettings)
   {
      if (ps.pluginId != pluginId)
         continue;
      MsgSettingDef* const s = ps.setting;
      PagedUIItemDef& item = items[i++];
      item = {};
      item.id = s->propId;
      item.label = s->name;
      item.tooltip = s->description;
      item.flags = s->isUserEditable ? 0 : PAGEDUI_ITEMF_HIDDEN;
      item.storage = PAGEDUI_STORAGE_HOST;
      item.context = s;
      switch (s->type)
      {
      case MSGPI_SETTING_TYPE_FLOAT:
         item.type = PAGEDUI_ITEM_FLOAT;
         item.floatProp = { s->floatDef.minVal, s->floatDef.maxVal, s->floatDef.step, s->floatDef.defVal, 0.f, nullptr, &SettingFloatGet, &SettingFloatSet };
         break;
      case MSGPI_SETTING_TYPE_INT:
         if (s->intDef.values != nullptr)
         {
            item.type = PAGEDUI_ITEM_ENUM;
            item.enumProp = { s->intDef.minVal, s->intDef.maxVal - s->intDef.minVal + 1, s->intDef.values, s->intDef.defVal, &SettingIntGet, &SettingIntSet };
         }
         else
         {
            item.type = PAGEDUI_ITEM_INT;
            item.intProp = { s->intDef.minVal, s->intDef.maxVal, s->intDef.defVal, nullptr, &SettingIntGet, &SettingIntSet };
         }
         break;
      case MSGPI_SETTING_TYPE_BOOL:
         item.type = PAGEDUI_ITEM_BOOL;
         item.boolProp = { s->boolDef.defVal, &SettingBoolGet, &SettingBoolSet };
         break;
      case MSGPI_SETTING_TYPE_STRING:
         item.type = PAGEDUI_ITEM_STRING;
         item.stringProp = { s->stringDef.defVal, &SettingStringGet, &SettingStringSet };
         break;
      case MSGPI_SETTING_TYPE_FOLDER:
         item.type = PAGEDUI_ITEM_FOLDER;
         item.folderProp = { s->folderDef.defVal, &SettingFolderGet, &SettingFolderSet };
         break;
      case MSGPI_SETTING_TYPE_FILE:
         item.type = PAGEDUI_ITEM_FILE;
         item.fileProp = { s->fileDef.defVal, s->fileDef.fileFilter, &SettingFileGet, &SettingFileSet };
         break;
      default: break;
      }
   }
}

void VPXPluginPagedUI::EnsureImplicitSettingsSheet(const string& pluginId)
{
   if (FindUISheet(pluginId, "settings"s) != nullptr)
      return;
   auto sheet = std::make_shared<UISheet>();
   sheet->endpointId = m_msgApi.GetPluginEndpoint(pluginId.c_str());
   sheet->pluginId = pluginId;
   sheet->id = "settings";
   sheet->name = "Settings";
   sheet->propKeyPrefix = "";
   sheet->def.version = 1;
   sheet->def.id = sheet->id.c_str();
   sheet->def.name = sheet->name.c_str();
   sheet->def.description = "";
   sheet->def.context = const_cast<char*>(sheet->pluginId.c_str());
   sheet->def.GetPageCount = &ImplicitSettingsPageCount;
   sheet->def.GetPageInfo = &ImplicitSettingsPageInfo;
   sheet->def.GetItemCount = &ImplicitSettingsItemCount;
   sheet->def.GetItems = &ImplicitSettingsGetItems;
   // Insert first so the default page lists before the plugin's declared sheets
   m_uiSheets.insert(m_uiSheets.begin(), std::move(sheet));
}

void VPXPluginPagedUI::OnGetUIPluginAPI(const unsigned int msgId, void* userData, void* msgData)
{
   VPXPluginPagedUI& pi = *static_cast<VPXPluginPagedUI*>(userData);
   PagedUIPluginAPI** pResult = static_cast<PagedUIPluginAPI**>(msgData);
   *pResult = &pi.m_uiApi;
}

std::shared_ptr<VPXPluginPagedUI::UISheet> VPXPluginPagedUI::FindUISheet(const string& pluginId, const string& sheetId)
{
   const auto it = std::ranges::find_if(m_uiSheets, [&](const auto& sheet) { return !sheet->retired && sheet->pluginId == pluginId && sheet->id == sheetId; });
   return it == m_uiSheets.end() ? nullptr : *it;
}

void VPXPluginPagedUI::GetUISheets(const string& pluginId, vector<UISheet*>& out)
{
   for (auto& sheet : m_uiSheets)
      if (!sheet->retired && (pluginId.empty() || sheet->pluginId == pluginId))
         out.push_back(sheet.get());
}

std::shared_ptr<VPXPluginPagedUI::UISheet> VPXPluginPagedUI::FindRootUISheet(const string& pluginId)
{
   std::shared_ptr<UISheet> settingsSheet;
   for (auto& sheet : m_uiSheets)
   {
      if (sheet->retired || sheet->pluginId != pluginId)
         continue;
      if (sheet->def.isRoot != 0)
         return sheet; // A plugin declared root wins over the default settings page
      if (sheet->id == "settings")
         settingsSheet = sheet;
   }
   return settingsSheet;
}

void VPXPluginPagedUI::CollectRetiredSheets()
{
   std::erase_if(m_uiSheets, [](const auto& sheet) { return sheet->retired && sheet->openPageCount == 0; });
}

void MSGPIAPI VPXPluginPagedUI::RegisterSheet(const uint32_t endpointId, const PagedUISheetDef* sheetDef)
{
   if (s_instance == nullptr || sheetDef == nullptr || sheetDef->id == nullptr || sheetDef->version != 1)
      return;
   s_instance->m_pluginManager.AssertAPIThread();
   if (sheetDef->GetPageCount == nullptr || sheetDef->GetPageInfo == nullptr || sheetDef->GetItemCount == nullptr || sheetDef->GetItems == nullptr)
   {
      PLOGE << "Invalid UI sheet registration for '" << sheetDef->id << "' (missing page enumeration callbacks)";
      return;
   }
   MsgEndpointInfo info {};
   s_instance->m_msgApi.GetEndpointInfo(endpointId, &info);
   if (info.id == nullptr)
      return;
   // Registering over an existing id replaces the previous definition
   if (std::shared_ptr<UISheet> existing = s_instance->FindUISheet(info.id, sheetDef->id))
   {
      existing->retired = true;
      s_instance->CollectRetiredSheets();
   }
   auto sheet = std::make_shared<UISheet>();
   sheet->endpointId = endpointId;
   sheet->pluginId = info.id;
   sheet->id = sheetDef->id;
   sheet->name = sheetDef->name != nullptr ? sheetDef->name : sheetDef->id;
   sheet->description = sheetDef->description != nullptr ? sheetDef->description : "";
   sheet->def = *sheetDef;
   sheet->def.id = sheet->id.c_str();
   sheet->def.name = sheet->name.c_str();
   sheet->def.description = sheet->description.c_str();
   // The default "settings" sheet reuses the raw propIds of the legacy RegisterSetting surface
   sheet->propKeyPrefix = sheet->id == "settings" ? ""s : sheet->id + '.';
   UISheet* const registered = sheet.get();
   s_instance->m_uiSheets.push_back(std::move(sheet));
   PLOGI << "Plugin '" << registered->pluginId << "' registered UI sheet '" << registered->id << '\'';
   // Persisted values of host stored items are pushed to the plugin right away (like plugin settings on load).
   // As plugins are loaded per game, this is guaranteed to happen once per session for each item.
   s_instance->PushSheetValues(*registered);
}

void MSGPIAPI VPXPluginPagedUI::UnregisterSheet(const uint32_t endpointId, const char* sheetId)
{
   if (s_instance == nullptr || sheetId == nullptr)
      return;
   for (auto& sheet : s_instance->m_uiSheets)
      if (sheet->endpointId == endpointId && sheet->id == sheetId)
         sheet->retired = true;
   s_instance->CollectRetiredSheets();
   // A plugin declared default page that goes away reveals the implicit settings sheet again
   if (std::string_view { sheetId } == "settings")
   {
      MsgEndpointInfo info {};
      s_instance->m_msgApi.GetEndpointInfo(endpointId, &info);
      if (info.id != nullptr)
         s_instance->EnsureImplicitSettingsSheet(info.id);
   }
}

void MSGPIAPI VPXPluginPagedUI::RequestSheetRefresh(const uint32_t endpointId, const char* sheetId)
{
   if (g_pplayer != nullptr && g_pplayer->m_liveUI != nullptr) // No UI while plugins load or the player shuts down
      g_pplayer->m_liveUI->m_inGameUI.RequestRebuildAll();
}

bool VPXPluginPagedUI::IsSheetPropItem(PagedUIItemType type)
{
   switch (type)
   {
   case PAGEDUI_ITEM_FLOAT:
   case PAGEDUI_ITEM_INT:
   case PAGEDUI_ITEM_ENUM:
   case PAGEDUI_ITEM_BOOL:
   case PAGEDUI_ITEM_STRING:
   case PAGEDUI_ITEM_FILE:
   case PAGEDUI_ITEM_FOLDER: return true;
   default: return false;
   }
}

void VPXPluginPagedUI::ForEachSheetItem(UISheet& sheet, const std::function<void(const PagedUIItemDef&)>& fn)
{
   const unsigned int nPages = sheet.def.GetPageCount(sheet.def.context);
   for (unsigned int p = 0; p < nPages; p++)
   {
      PagedUIPageInfo pageInfo {};
      sheet.def.GetPageInfo(sheet.def.context, p, &pageInfo);
      if (pageInfo.id == nullptr)
         continue;
      const unsigned int nItems = sheet.def.GetItemCount(sheet.def.context, pageInfo.id);
      vector<PagedUIItemDef> items(nItems);
      if (nItems > 0)
         sheet.def.GetItems(sheet.def.context, pageInfo.id, items.data());
      for (const auto& item : items)
         fn(item);
   }
}

void VPXPluginPagedUI::ScanSheetItems(UISheet& sheet)
{
   sheet.hasHostItems = false;
   sheet.hasPluginItems = false;
   ForEachSheetItem(sheet,
      [&sheet](const PagedUIItemDef& item)
      {
         if (!IsSheetPropItem(item.type))
            return;
         if (item.storage == PAGEDUI_STORAGE_HOST)
            sheet.hasHostItems = true;
         else if (item.storage == PAGEDUI_STORAGE_PLUGIN)
            sheet.hasPluginItems = true;
      });
}

// Pushes the persisted value of a host stored sheet item to the plugin (same semantics as settings on plugin load)
static void PushSheetItemValue(const PagedUIItemDef& item, VPX::Properties::PropertyRegistry::PropId propId, Settings& settings)
{
   using namespace VPX::Properties;
   void* const ictx = item.context;
   switch (item.type)
   {
   case PAGEDUI_ITEM_FLOAT:
      if (item.floatProp.Set != nullptr)
         item.floatProp.Set(ictx, Settings::GetRegistry().GetFloatProperty(propId)->GetValid(settings.GetFloat(propId)));
      break;
   case PAGEDUI_ITEM_INT:
      if (item.intProp.Set != nullptr)
         item.intProp.Set(ictx, Settings::GetRegistry().GetIntProperty(propId)->GetValid(settings.GetInt(propId)));
      break;
   case PAGEDUI_ITEM_ENUM:
      if (item.enumProp.Set != nullptr)
         item.enumProp.Set(ictx, Settings::GetRegistry().GetEnumProperty(propId)->GetValid(settings.GetInt(propId)));
      break;
   case PAGEDUI_ITEM_BOOL:
      if (item.boolProp.Set != nullptr)
         item.boolProp.Set(ictx, settings.GetBool(propId) ? 1 : 0);
      break;
   case PAGEDUI_ITEM_STRING:
      if (item.stringProp.Set != nullptr)
         item.stringProp.Set(ictx, settings.GetString(propId).c_str());
      break;
   case PAGEDUI_ITEM_FILE:
      if (item.fileProp.Set != nullptr)
         item.fileProp.Set(ictx, settings.GetString(propId).c_str());
      break;
   case PAGEDUI_ITEM_FOLDER:
      if (item.folderProp.Set != nullptr)
         item.folderProp.Set(ictx, settings.GetString(propId).c_str());
      break;
   default: break;
   }
}

std::optional<VPX::Properties::PropertyRegistry::PropId> VPXPluginPagedUI::EnsureSheetHostProp(UISheet& sheet, const PagedUIItemDef& item)
{
   using namespace VPX::Properties;
   if (!IsSheetPropItem(item.type) || item.storage != PAGEDUI_STORAGE_HOST)
      return std::nullopt;
   if (item.id == nullptr || *item.id == '\0')
   {
      PLOGE << "UI sheet '" << sheet.id << "' declares a property item without an id (ignored)";
      return std::nullopt;
   }
   // Property ids are prefixed by the sheet id so that item ids only need to be unique within a sheet
   // (except the default "settings" sheet which keeps the raw propIds of legacy RegisterSetting entries)
   const string groupId = "Plugin."s + sheet.pluginId;
   const string propKey = sheet.propKeyPrefix + item.id;
   const string label = item.label != nullptr ? item.label : "";
   const string tooltip = item.tooltip != nullptr ? item.tooltip : "";
   PropertyRegistry& registry = Settings::GetRegistry();
   PropertyRegistry::PropId propId;
   switch (item.type)
   {
   case PAGEDUI_ITEM_FLOAT:
      propId = registry.Register(
         std::make_unique<FloatPropertyDef>(groupId, propKey, label, tooltip, false, item.floatProp.minVal, item.floatProp.maxVal, item.floatProp.step, item.floatProp.defVal));
      if (registry.GetFloatProperty(propId) == nullptr)
         return std::nullopt;
      break;
   case PAGEDUI_ITEM_INT:
      propId = registry.Register(std::make_unique<IntPropertyDef>(groupId, propKey, label, tooltip, false, item.intProp.minVal, item.intProp.maxVal, item.intProp.defVal));
      if (registry.GetIntProperty(propId) == nullptr)
         return std::nullopt;
      break;
   case PAGEDUI_ITEM_ENUM:
   {
      vector<string> values;
      for (int i = 0; i < item.enumProp.nValues; i++)
         values.emplace_back(item.enumProp.values[i]);
      propId = registry.Register(std::make_unique<EnumPropertyDef>(groupId, propKey, label, tooltip, false, item.enumProp.minVal, item.enumProp.defVal, values));
      if (registry.GetEnumProperty(propId) == nullptr)
         return std::nullopt;
      break;
   }
   case PAGEDUI_ITEM_BOOL:
      propId = registry.Register(std::make_unique<BoolPropertyDef>(groupId, propKey, label, tooltip, false, item.boolProp.defVal != 0));
      if (registry.GetBoolProperty(propId) == nullptr)
         return std::nullopt;
      break;
   case PAGEDUI_ITEM_STRING:
      propId = registry.Register(std::make_unique<StringPropertyDef>(groupId, propKey, label, tooltip, false, item.stringProp.defVal != nullptr ? item.stringProp.defVal : ""));
      if (registry.GetStringProperty(propId) == nullptr)
         return std::nullopt;
      break;
   case PAGEDUI_ITEM_FILE:
      propId = registry.Register(std::make_unique<StringPropertyDef>(groupId, propKey, label, tooltip, false, item.fileProp.defVal != nullptr ? item.fileProp.defVal : "",
         StringPropertyDef::PathType::File, item.fileProp.fileFilter != nullptr ? item.fileProp.fileFilter : ""));
      if (registry.GetStringProperty(propId) == nullptr)
         return std::nullopt;
      break;
   case PAGEDUI_ITEM_FOLDER:
      propId = registry.Register(
         std::make_unique<StringPropertyDef>(groupId, propKey, label, tooltip, false, item.folderProp.defVal != nullptr ? item.folderProp.defVal : "", StringPropertyDef::PathType::Folder));
      if (registry.GetStringProperty(propId) == nullptr)
         return std::nullopt;
      break;
   default: return std::nullopt;
   }
   // Push the persisted value to the plugin once per session
   Settings& settings = m_settings != nullptr ? *m_settings : g_settingsService.GetActiveSettings();
   const string pushKey = sheet.pluginId + '/' + sheet.id + '/' + propKey;
   if (m_pushedSheetItems.insert(pushKey).second)
      PushSheetItemValue(item, propId, settings);
   return propId;
}

void VPXPluginPagedUI::PushSheetValues(UISheet& sheet)
{
   ScanSheetItems(sheet);
   ForEachSheetItem(sheet, [this, &sheet](const PagedUIItemDef& item) { EnsureSheetHostProp(sheet, item); });
}

void VPXPluginPagedUI::OnSheetPageOpened(UISheet& sheet)
{
   if (sheet.openPageCount++ != 0)
      return;
   sheet.editCommitted = 0;
   sheet.docSnapshot.clear();
   ScanSheetItems(sheet);
   if (sheet.hasPluginItems)
   {
      if (sheet.def.BeginEdit != nullptr)
         sheet.def.BeginEdit(sheet.def.context);
      RefreshSheetSnapshot(sheet);
   }
}

void VPXPluginPagedUI::OnSheetPageClosed(UISheet& sheet)
{
   if (sheet.openPageCount <= 0)
      return;
   if (--sheet.openPageCount > 0)
      return;
   if (s_instance == nullptr)
      return; // API implementation is being torn down, plugin is already unloaded
   if (sheet.hasPluginItems)
   {
      sheet.docSnapshot.clear();
      if (!sheet.retired && sheet.def.EndEdit != nullptr)
         sheet.def.EndEdit(sheet.def.context, sheet.editCommitted);
   }
   if (sheet.retired)
      s_instance->CollectRetiredSheets();
}

void VPXPluginPagedUI::RefreshSheetSnapshot(UISheet& sheet)
{
   sheet.docSnapshot.clear();
   sheet.hasHostItems = false;
   sheet.hasPluginItems = false;
   ForEachSheetItem(sheet,
      [&sheet](const PagedUIItemDef& item)
      {
         if (!IsSheetPropItem(item.type))
            return;
         if (item.storage == PAGEDUI_STORAGE_HOST)
            sheet.hasHostItems = true;
         if (item.storage != PAGEDUI_STORAGE_PLUGIN || item.id == nullptr || *item.id == '\0')
            return;
         sheet.hasPluginItems = true;
         void* const ictx = item.context;
         switch (item.type)
         {
         case PAGEDUI_ITEM_FLOAT:
            if (item.floatProp.Get != nullptr)
               sheet.docSnapshot[item.id] = item.floatProp.Get(ictx);
            break;
         case PAGEDUI_ITEM_INT:
            if (item.intProp.Get != nullptr)
               sheet.docSnapshot[item.id] = item.intProp.Get(ictx);
            break;
         case PAGEDUI_ITEM_ENUM:
            if (item.enumProp.Get != nullptr)
               sheet.docSnapshot[item.id] = item.enumProp.Get(ictx);
            break;
         case PAGEDUI_ITEM_BOOL:
            if (item.boolProp.Get != nullptr)
               sheet.docSnapshot[item.id] = item.boolProp.Get(ictx) != 0;
            break;
         case PAGEDUI_ITEM_STRING:
            if (item.stringProp.Get != nullptr)
               if (const char* v = item.stringProp.Get(ictx); v != nullptr)
                  sheet.docSnapshot[item.id] = string(v);
            break;
         case PAGEDUI_ITEM_FILE:
            if (item.fileProp.Get != nullptr)
               if (const char* v = item.fileProp.Get(ictx); v != nullptr)
                  sheet.docSnapshot[item.id] = string(v);
            break;
         case PAGEDUI_ITEM_FOLDER:
            if (item.folderProp.Get != nullptr)
               if (const char* v = item.folderProp.Get(ictx); v != nullptr)
                  sheet.docSnapshot[item.id] = string(v);
            break;
         default: break;
         }
      });
}


///////////////////////////////////////////////////////////////////////////////
//

VPXPluginPagedUI::VPXPluginPagedUI(MsgPI::MsgPluginManager& pluginManager, const MsgPluginAPI& msgApi, uint32_t hostEndpointId, Settings* settings)
   : m_pluginManager(pluginManager)
   , m_msgApi(msgApi)
   , m_settings(settings)
   , m_getUIAPIMsgId(m_msgApi.GetMsgID(PAGEDUIPI_NAMESPACE, PAGEDUIPI_MSG_GET_API))
{
   assert(s_instance == nullptr);
   s_instance = this;

   // MsgPlugin settings API
   pluginManager.SetSettingsHandler(
      [this](const std::string& pluginId, MsgPI::MsgPluginManager::SettingAction action, MsgSettingDef* settingDef) { UpdateSetting(pluginId, action, settingDef); });

   // PagedUI sheets API
   m_uiApi.version = 1;
   m_uiApi.RegisterSheet = RegisterSheet;
   m_uiApi.UnregisterSheet = UnregisterSheet;
   m_uiApi.RequestSheetRefresh = RequestSheetRefresh;
   m_msgApi.SubscribeMsg(hostEndpointId, m_getUIAPIMsgId, &OnGetUIPluginAPI, this);
}

VPXPluginPagedUI::~VPXPluginPagedUI()
{
   s_instance = nullptr;
   m_pluginManager.SetSettingsHandler(nullptr);

   // Retire all sheets: open pages may outlive this object (kept alive by shared_ptr) but must not
   // call back into plugins which are unloaded as part of the game teardown
   for (auto& sheet : m_uiSheets)
      sheet->retired = true;

   m_msgApi.UnsubscribeMsg(m_getUIAPIMsgId, &OnGetUIPluginAPI, this);
   m_msgApi.ReleaseMsgID(m_getUIAPIMsgId);
}
