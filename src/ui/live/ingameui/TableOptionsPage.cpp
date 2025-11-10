// license:GPLv3+

#include "core/stdafx.h"

#include "TableOptionsPage.h"

namespace VPX::InGameUI
{

TableOptionsPage::TableOptionsPage()
   : InGameUIPage("Table Options"s, ""s, SaveMode::Table)
{
}

const PinTable::TableOption* TableOptionsPage::GetOption(VPX::Properties::PropertyRegistry::PropId id) const
{
   for (const PinTable::TableOption& option : m_player->m_ptable->GetOptions())
      if (option.id.index == id.index)
         return &option;
   return nullptr;
}

void TableOptionsPage::Open(bool isBackwardAnimation)
{
   InGameUIPage::Open(isBackwardAnimation);
   ClearItems();
   for (const PinTable::TableOption& option : m_player->m_ptable->GetOptions())
   {
      const VPX::Properties::PropertyRegistry::PropId id = option.id;
      const bool isReversed = option.displayScale < 0.f;
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
         AddItem(std::make_unique<InGameUIItem>(
            option.id, //
            [this, id]() { return GetOption(id)->value != 0.f; }, //
            [this, id, isReversed](bool v) {
               m_player->m_ptable->SetOptionLiveValue(id, isReversed ? (v ? 0.f : 1.f) : (v ? 1.f : 0.f));
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
