// license:GPLv3+

#pragma once

#include "InGameUIPage.h"

namespace VPX::InGameUI
{

class HomePage final : public InGameUIPage
{
public:
   HomePage();

   void CollectPanes(vector<InGameUIPane>& panes) override;

private:
   void BuildPage() override;

};

}
