// license:GPLv3+

#include "core/stdafx.h"

#include "TableRulesPage.h"

namespace VPX::InGameUI
{

TableRulesPage::TableRulesPage()
   : InGameUIPage("Table Rules"s, ""s, SaveMode::None)
{
   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Markdown, m_player->m_ptable->m_rules));
}

}
