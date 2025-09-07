// license:GPLv3+

#include "core/stdafx.h"

#include "TableRulesPage.h"

namespace VPX::InGameUI
{

TableRulesPage::TableRulesPage()
   : InGameUIPage("table/rules", "Table Rules", "")
{
   auto rules = std::make_unique<InGameUIItem>(m_player->m_ptable->m_rules);
   AddItem(rules);
}

};