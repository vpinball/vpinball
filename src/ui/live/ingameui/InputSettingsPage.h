// license:GPLv3+

#pragma once

#include "InGameUIPage.h"

namespace VPX::InGameUI
{

class InputSettingsPage final : public InGameUIPage
{
public:
   InputSettingsPage();

private:
   void BuildPage() override;
};


class InputMappingsPage final : public InGameUIPage
{
public:
   InputMappingsPage();

private:
   void BuildPage() override;
};
}
