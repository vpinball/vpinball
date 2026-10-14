// license:GPLv3+

#include "core/stdafx.h"
#include "TableOptionsPage.h"

#include "parts/pintable.h"

namespace VPX::InGameUI
{

TableOptionsPage::TableOptionsPage()
   : InGameUIPage("Table Options"s, ""s, SaveMode::Table)
{
}

const PinTable::TableOption* TableOptionsPage::GetOption(VPX::Properties::PropertyRegistry::PropId id) const
{
   for (const PinTable::TableOption& option : m_player->m_ptable->GetOptions())
      if ((option.id.type == id.type) && (option.id.index == id.index))
         return &option;
   return nullptr;
}

void TableOptionsPage::BuildPage()
{
   for (const PinTable::TableOption& option : m_player->m_ptable->GetOptions())
   {
      const VPX::Properties::PropertyRegistry::PropId id = option.id;
      switch (Settings::GetRegistry().GetProperty(option.id)->m_type)
      {
      case VPX::Properties::PropertyDef::Type::Float:
         AddItem(std::make_unique<InGameUIItem>(
            option.id, option.displayScale, option.format, //
            [this, id]() { return GetOption(id)->value; }, //
            [this, id](float, float v) {
               m_player->m_ptable->SetOptionLiveValue(id, v);
               m_player->m_ptable->FireOptionEvent(PinTable::OptionEventType::Changed);
            }));
         break;
      case VPX::Properties::PropertyDef::Type::Int:
         AddItem(std::make_unique<InGameUIItem>(
            option.id, option.format, //
            [this, id]() { return static_cast<int>(GetOption(id)->value); }, //
            [this, id](int, int v) {
               m_player->m_ptable->SetOptionLiveValue(id, static_cast<float>(v));
               m_player->m_ptable->FireOptionEvent(PinTable::OptionEventType::Changed);
            }));
         break;
      case VPX::Properties::PropertyDef::Type::Bool:
         // The checkbox is the logical on/off value; array-order mapping lives in PinTable::get_Option/put_Option.
         AddItem(std::make_unique<InGameUIItem>(
            option.id, //
            [this, id]() { return GetOption(id)->value != 0.f; }, //
            [this, id](bool v) {
               m_player->m_ptable->SetOptionLiveValue(id, v ? 1.f : 0.f);
               m_player->m_ptable->FireOptionEvent(PinTable::OptionEventType::Changed);
            }));
         break;
      case VPX::Properties::PropertyDef::Type::Enum:
         AddItem(std::make_unique<InGameUIItem>(
            option.id, //
            [this, id]() { return static_cast<int>(GetOption(id)->value); }, //
            [this, id](int, int v) {
               m_player->m_ptable->SetOptionLiveValue(id, static_cast<float>(v));
               m_player->m_ptable->FireOptionEvent(PinTable::OptionEventType::Changed);
            }));
         break;
      case VPX::Properties::PropertyDef::Type::String: break;
      default: assert(false); break;
      }
   }
}

}
