// license:GPLv3+

#include "core/stdafx.h"
#include "LoggingSettingsPage.h"

#include "ui/live/LiveUI.h"
#include "utils/Logger.h"

namespace VPX::InGameUI
{

LoggingSettingsPage::LoggingSettingsPage()
   : InGameUIPage("Logging Settings"s, ""s, SaveMode::Global)
{
}

void LoggingSettingsPage::BuildPage()
{
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propGlobal_EnableLog, //
      [this]() { return g_settingsService.GetActiveSettings().GetGlobal_EnableLog(); }, //
      [this](bool v)
      {
         g_settingsService.GetActiveSettings().SetGlobal_EnableLog(v, false);
         Logger::SetupLogger(v);
      }));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propGlobal_LogScriptOutput, //
      [this]() { return g_settingsService.GetActiveSettings().GetGlobal_LogScriptOutput(); }, //
      [this](bool v) { g_settingsService.GetActiveSettings().SetGlobal_LogScriptOutput(v, false); }));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propGlobal_ResetLogOnPlay, //
      [this]() { return g_settingsService.GetActiveSettings().GetGlobal_ResetLogOnPlay(); }, //
      [this](bool v) { g_settingsService.GetActiveSettings().SetGlobal_ResetLogOnPlay(v, false); }));
}

}
