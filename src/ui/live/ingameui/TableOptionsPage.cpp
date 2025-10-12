// license:GPLv3+

#include "core/stdafx.h"

#include "TableOptionsPage.h"

namespace VPX::InGameUI
{

TableOptionsPage::TableOptionsPage()
   : InGameUIPage("Table Options"s, ""s, SaveMode::Table)
{
}

void TableOptionsPage::Open(bool isBackwardAnimation)
{
   InGameUIPage::Open(isBackwardAnimation);
   ClearItems();
   for (auto& opt : m_player->m_ptable->GetOptions())
      AddItem(std::make_unique<InGameUIItem>(opt));
}

}
