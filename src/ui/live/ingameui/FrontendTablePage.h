// license:GPLv3+

#pragma once

#include "InGameUIPage.h"

namespace VPX::InGameUI
{

// The frontend's TABLE view: the actions proposed on the catalog entry visible
// behind the chrome (play, live edit, info/media management,
// catalog rescan, VR session toggle) — the frontend's former action popup.
class FrontendTablePage final : public InGameUIPage
{
public:
   FrontendTablePage();

private:
   void BuildPage() override;
};

}
