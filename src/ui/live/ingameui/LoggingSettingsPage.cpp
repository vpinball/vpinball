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
      [this]() { return m_player->m_ptable->m_settings.GetGlobal_EnableLog(); }, //
      [this](bool v)
      {
         m_player->m_ptable->m_settings.SetGlobal_EnableLog(v, false);
         Logger::SetupLogger(v);
      }));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propGlobal_LogScriptOutput, //
      [this]() { return m_player->m_ptable->m_settings.GetGlobal_LogScriptOutput(); }, //
      [this](bool v) { m_player->m_ptable->m_settings.SetGlobal_LogScriptOutput(v, false); }));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propGlobal_ResetLogOnPlay, //
      [this]() { return m_player->m_ptable->m_settings.GetGlobal_ResetLogOnPlay(); }, //
      [this](bool v) { m_player->m_ptable->m_settings.SetGlobal_ResetLogOnPlay(v, false); }));
}

}
