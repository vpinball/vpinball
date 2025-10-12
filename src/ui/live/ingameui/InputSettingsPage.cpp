// license:GPLv3+

#include "core/stdafx.h"

#include "InputSettingsPage.h"

namespace VPX::InGameUI
{

InputSettingsPage::InputSettingsPage()
   : InGameUIPage("Input Settings"s, ""s, SaveMode::Global)
{
}

void InputSettingsPage::Open(bool isBackwardAnimation)
{
   InGameUIPage::Open(isBackwardAnimation);
   InputManager& input = GetInput();

   ClearItems();
   for (auto& action : input.GetInputActions())
      AddItem(std::make_unique<InGameUIItem>(action->GetLabel(), "Select to add a new input binding which can be composed of multiple pressed button."s, action.get()));
}

}
