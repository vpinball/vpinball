// license:GPLv3+

#pragma once

#include "InGameUIPage.h"

namespace VPX::InGameUI
{

// The frontend's media manager: the media files discovered along the selected catalog entry, one
// row per type, with a delete action on the present ones.
class FrontendMediaManagerPage final : public InGameUIPage
{
public:
   FrontendMediaManagerPage();

private:
   void BuildPage() override;
};

}
