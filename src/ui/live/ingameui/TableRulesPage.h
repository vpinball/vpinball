// license:GPLv3+

#pragma once

#include "InGameUIPage.h"

namespace VPX::InGameUI
{

class TableRulesPage final : public InGameUIPage
{
public:
   TableRulesPage();

private:
   void BuildPage() override;
};

}
