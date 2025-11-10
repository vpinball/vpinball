// license:GPLv3+

#include "core/stdafx.h"

#include "PluginSettingsPage.h"

#include "plugins/MsgPluginManager.h"
#include "core/VPXPluginAPIImpl.h"

using namespace MsgPI;

namespace VPX::InGameUI
{

PluginHomePage::PluginHomePage()
   : InGameUIPage("Plugin"s, ""s, SaveMode::None)
{
}

void PluginHomePage::Open(bool isBackwardAnimation)
{
   InGameUIPage::Open(isBackwardAnimation);
   ClearItems();
   const MsgPluginManager& manager = MsgPluginManager::GetInstance();
   for (const auto& plugin : manager.GetPlugins())
   {
      const string& id = plugin->m_id;
      if (id == "vpx") // Do not expose core VPX plugin (disabling it would crash the app)
         continue;
      m_player->m_liveUI->m_inGameUI.AddPage("plugin/" + plugin->m_id, [id]() { return std::make_unique<PluginSettingsPage>(id); });
      AddItem(std::make_unique<InGameUIItem>(plugin->m_name, plugin->m_description, "plugin/" + plugin->m_id));
   }
}


PluginSettingsPage::PluginSettingsPage(const string& pluginId)
   : InGameUIPage(MsgPluginManager::GetInstance().GetPlugin(pluginId)->m_name,
        MsgPluginManager::GetInstance().GetPlugin(pluginId)->m_description + "\nBy " + MsgPluginManager::GetInstance().GetPlugin(pluginId)->m_author + "\nVersion "
           + MsgPluginManager::GetInstance().GetPlugin(pluginId)->m_version,
        SaveMode::Both)
   , m_pluginId(pluginId)
{
}

void PluginSettingsPage::Open(bool isBackwardAnimation)
{
   InGameUIPage::Open(isBackwardAnimation);
   BuildPage();
}

void PluginSettingsPage::BuildPage()
{
   ClearItems();
   const auto enablePropId = Settings::GetRegistry().GetPropertyId("Plugin." + m_pluginId, "Enable"s).value();
   const MsgPluginManager& manager = MsgPluginManager::GetInstance();
   if (auto plugin = manager.GetPlugin(m_pluginId); plugin == nullptr)
   {
      AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Info, "Internal error..."s));
      return;
   }

   // Consider the current state of the plugin as the default to avoid always returning to off state
   const bool isEnabled = m_player->m_ptable->m_settings.GetBool(enablePropId);
   Settings::GetRegistry().Register(Settings::GetRegistry().GetBoolProperty(enablePropId)->WithDefault(isEnabled));
   AddItem(std::make_unique<InGameUIItem>( //
      enablePropId, //
      [this]()
      {
         const MsgPluginManager& manager = MsgPluginManager::GetInstance();
         auto plugin = manager.GetPlugin(m_pluginId);
         return plugin ? plugin->IsLoaded() : false;
      }, //
      [this](bool v)
      {
         const MsgPluginManager& manager = MsgPluginManager::GetInstance();
         auto plugin = manager.GetPlugin(m_pluginId);
         if (v && !plugin->IsLoaded())
            plugin->Load(&manager.GetMsgAPI());
         else if (!v && plugin->IsLoaded())
            plugin->Unload();
         BuildPage();
      }));

   if (!isEnabled)
      return;

   for (const auto& option : VPXPluginAPIImpl::GetInstance().GetPluginSettings())
   {
      if (option.pluginId != m_pluginId)
         continue;

      const bool isReversed = false; //option.displayScale < 0.f;
      switch (Settings::GetRegistry().GetProperty(option.propId)->m_type)
      {
      case VPX::Properties::PropertyDef::Type::Float:
         AddItem(std::make_unique<InGameUIItem>(
            option.propId, 1.f, "%4.1f", /* option.displayScale, option.format,*/ //
            [option]() { return option.setting->floatDef.val; }, //
            [option](float, float v) { option.setting->floatDef.val = v; }));
         break;
      case VPX::Properties::PropertyDef::Type::Int:
         AddItem(std::make_unique<InGameUIItem>(
            option.propId, "%4d", /* option.format, */ //
            [option]() { return option.setting->intDef.val; }, //
            [option](int, int v) { option.setting->intDef.val = v; }));
         break;
      case VPX::Properties::PropertyDef::Type::Bool:
         AddItem(std::make_unique<InGameUIItem>(
            option.propId, //
            [option]() { return option.setting->boolDef.val != 0; }, //
            [option, isReversed](bool v) { option.setting->boolDef.val = isReversed ? (v ? 0 : 1) : (v ? 1 : 0); }));
         break;
      case VPX::Properties::PropertyDef::Type::Enum:
         AddItem(std::make_unique<InGameUIItem>(
            option.propId, //
            [option]() { return option.setting->intDef.val; }, //
            [option](int, int v) { option.setting->intDef.val = v; }));
         break;
      case VPX::Properties::PropertyDef::Type::String:
         AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Info, option.setting->name + ": "s + option.setting->stringDef.val));
         break;
      default: assert(false); break;
      }
   }
}

}
