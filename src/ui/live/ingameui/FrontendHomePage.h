// license:GPLv3+

#pragma once

#include "InGameUIPage.h"

namespace VPX::InGameUI
{

// The mini frontend's navigation root: the table of contents feeding the chrome's
// categories (table actions, engine settings, plugins, system info) — there is no
// table section as the frontend session runs on a synthetic empty table.
class FrontendHomePage final : public InGameUIPage
{
public:
   FrontendHomePage();

private:
   void BuildPage() override;
};

}
