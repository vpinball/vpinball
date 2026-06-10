// license:GPLv3+

#include "core/stdafx.h"
#include "TableRulesPage.h"

#include "parts/pintable.h"

namespace VPX::InGameUI
{

TableRulesPage::TableRulesPage()
   : InGameUIPage("Table Rules"s, ""s, SaveMode::None)
{
}

void TableRulesPage::BuildPage()
{
   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Markdown, m_player->m_ptable->m_rules));
}

}
