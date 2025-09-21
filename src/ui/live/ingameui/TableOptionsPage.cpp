// license:GPLv3+

#include "core/stdafx.h"

#include "TableOptionsPage.h"

namespace VPX::InGameUI
{

TableOptionsPage::TableOptionsPage()
   : InGameUIPage("table/options"s, "Table Options"s, ""s, SaveMode::None)
{
}

int TableOptionsPage::GetOptionIndex(const string& id) const
{
   for (int i = 0; i < (int)m_player->m_ptable->m_settings.GetTableSettings().size(); i++)
      if (m_player->m_ptable->m_settings.GetTableSettings()[i].name == id)
         return i;
   return -1;
}

void TableOptionsPage::Open()
{
   ClearItems();

   for (auto& opt : m_player->m_ptable->m_settings.GetTableSettings())
   {
      const string optId = opt.name;
      const float scale = opt.unit == Settings::OT_PERCENT ? 100.f : 1.f;
      const string format = opt.unit == Settings::OT_PERCENT ? "%4.1f %%" : "%4.1f";
      if (!opt.literals.empty())
      {
         // TODO detect & implement On/Off or True/False as a toggle ?
         auto item = std::make_unique<InGameUIItem>(
            opt.name, ""s, opt.literals, static_cast<int>(opt.defaultValue),
            [this, optId]()
            {
               int index = GetOptionIndex(optId);
               if (index == -1)
                  return 0;
               const Settings::OptionDef& opt = m_player->m_ptable->m_settings.GetTableSettings()[index];
               return static_cast<int>(opt.value);
            },
            [this, optId](int prev, int v)
            {
               int index = GetOptionIndex(optId);
               if (index == -1)
                  return;
               Settings::OptionDef& opt = m_player->m_ptable->m_settings.GetTableSettings()[index];
               opt.value = static_cast<float>(v);
               m_player->m_ptable->FireOptionEvent(1); // Table option changed event
            },
            [this, optId](Settings& settings)
            {
               int index = GetOptionIndex(optId);
               if (index == -1)
                  return;
               const Settings::OptionDef& opt = settings.GetTableSettings()[index];
               settings.DeleteValue(Settings::TableOption, opt.name);
            },
            [this, optId](int v, Settings& settings, bool isTableOverride)
            {
               int index = GetOptionIndex(optId);
               if (index == -1)
                  return;
               const Settings::OptionDef& opt = settings.GetTableSettings()[index];
               settings.SaveValue(Settings::TableOption, opt.name, v, isTableOverride);
            });
         AddItem(item);
      }
      else if (round(opt.step) == 1.f && round(opt.minValue) == opt.minValue)
      {
         auto item = std::make_unique<InGameUIItem>(
            opt.name, ""s, static_cast<int>(opt.minValue * scale), static_cast<int>(opt.maxValue * scale), static_cast<int>(opt.defaultValue * scale), format,
            [this, optId]()
            {
               int index = GetOptionIndex(optId);
               if (index == -1)
                  return 0;
               const Settings::OptionDef& opt = m_player->m_ptable->m_settings.GetTableSettings()[index];
               return static_cast<int>(opt.value);
            },
            [this, optId](int prev, int v)
            {
               int index = GetOptionIndex(optId);
               if (index == -1)
                  return;
               Settings::OptionDef& opt = m_player->m_ptable->m_settings.GetTableSettings()[index];
               opt.value = static_cast<float>(v);
               m_player->m_ptable->FireOptionEvent(1); // Table option changed event
            },
            [this, optId](Settings& settings)
            {
               int index = GetOptionIndex(optId);
               if (index == -1)
                  return;
               const Settings::OptionDef& opt = settings.GetTableSettings()[index];
               settings.DeleteValue(Settings::TableOption, opt.name);
            },
            [this, optId](int v, Settings& settings, bool isTableOverride)
            {
               int index = GetOptionIndex(optId);
               if (index == -1)
                  return;
               const Settings::OptionDef& opt = settings.GetTableSettings()[index];
               settings.SaveValue(Settings::TableOption, opt.name, v);
            });
         AddItem(item);
      }
      else
      {
         auto item = std::make_unique<InGameUIItem>(
            opt.name, ""s, opt.minValue * scale, opt.maxValue * scale, opt.step * scale, opt.defaultValue * scale, format,
            [this, optId, scale]()
            {
               int index = GetOptionIndex(optId);
               if (index == -1)
                  return 0.f;
               const Settings::OptionDef& opt = m_player->m_ptable->m_settings.GetTableSettings()[index];
               return opt.value * scale;
            },
            [this, optId, scale](float prev, float v)
            {
               int index = GetOptionIndex(optId);
               if (index == -1)
                  return;
               Settings::OptionDef& opt = m_player->m_ptable->m_settings.GetTableSettings()[index];
               opt.value = v / scale;
               m_player->m_ptable->FireOptionEvent(1); // Table option changed event
            },
            [this, optId](Settings& settings)
            {
               int index = GetOptionIndex(optId);
               if (index == -1)
                  return;
               const Settings::OptionDef& opt = settings.GetTableSettings()[index];
               settings.DeleteValue(Settings::TableOption, opt.name);
            },
            [this, optId, scale](float v, Settings& settings, bool isTableOverride)
            {
               int index = GetOptionIndex(optId);
               if (index == -1)
                  return;
               const Settings::OptionDef& opt = settings.GetTableSettings()[index];
               settings.SaveValue(Settings::TableOption, opt.name, v / scale);
            });
         AddItem(item);
      }
   }
}

}
