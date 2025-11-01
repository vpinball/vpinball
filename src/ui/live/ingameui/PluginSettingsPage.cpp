// license:GPLv3+

#include "core/stdafx.h"

#include "PluginSettingsPage.h"

#include "plugins/MsgPluginManager.h"

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
      const string id = plugin->m_id;
      if (id == "vpx"s) // Do not expose core VPX plugin (disabling it would crash the app)
         continue;
      m_player->m_liveUI->m_inGameUI.AddPage("plugin/"s + plugin->m_id, [id]() { return std::make_unique<PluginSettingsPage>(id); });
      AddItem(std::make_unique<InGameUIItem>(plugin->m_name, plugin->m_description, "plugin/"s + plugin->m_id));
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
   const auto enablePropId = Settings::GetRegistry().GetPropertyId("Plugin"s + m_pluginId, "Enable"s).value();
   const MsgPluginManager& manager = MsgPluginManager::GetInstance();
   if (auto plugin = manager.GetPlugin(m_pluginId); plugin == nullptr)
   {
      AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Info, "Internal error..."));
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

   /* for (const PinTable::TableOption& option : m_player->m_ptable->GetOptions())
   {
      const VPX::Properties::PropertyRegistry::PropId id = option.id;
      const bool isReversed = option.displayScale < 0.f;
      switch (Settings::GetRegistry().GetProperty(option.id)->m_type)
      {
      case VPX::Properties::PropertyDef::Type::Float:
         AddItem(std::make_unique<InGameUIItem>(
            option.id, option.displayScale, option.format, //
            [this, id]() { return GetOption(id)->value; }, //
            [this, id](float, float v) { m_player->m_ptable->SetOptionLiveValue(id, v); }));
         break;
      case VPX::Properties::PropertyDef::Type::Int:
         AddItem(std::make_unique<InGameUIItem>(
            option.id, option.format, //
            [this, id]() { return static_cast<int>(GetOption(id)->value); }, //
            [this, id](int, int v) { m_player->m_ptable->SetOptionLiveValue(id, static_cast<float>(v)); }));
         break;
      case VPX::Properties::PropertyDef::Type::Bool:
         AddItem(std::make_unique<InGameUIItem>(
            option.id, //
            [this, id]() { return GetOption(id)->value != 0.f; }, //
            [this, id, isReversed](bool v) { m_player->m_ptable->SetOptionLiveValue(id, isReversed ? (v ? 0.f : 1.f) : (v ? 1.f : 0.f)); }));
         break;
      case VPX::Properties::PropertyDef::Type::Enum:
         AddItem(std::make_unique<InGameUIItem>(
            option.id, //
            [this, id]() { return static_cast<int>(GetOption(id)->value); }, //
            [this, id](int, int v) { m_player->m_ptable->SetOptionLiveValue(id, static_cast<float>(v)); }));
         break;
      case VPX::Properties::PropertyDef::Type::String: break;
      default: assert(false); break;
      }
   }*/
}

}
