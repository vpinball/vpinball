// license:GPLv3+

#include "core/stdafx.h"

#include "InputSettingsPage.h"

namespace VPX::InGameUI
{

InputSettingsPage::InputSettingsPage()
   : InGameUIPage("Input Settings"s, ""s, SaveMode::Global)
{
}

void InputSettingsPage::Open()
{
   InGameUIPage::Open();
   InputManager& input = GetInput();

   ClearItems();
   for (auto& action : input.GetInputActions())
   {
      auto actionItem = std::make_unique<InGameUIItem>(
         action->GetLabel(), "Select to add a new input binding which can be composed of multiple pressed button."s, action.get());
      AddItem(actionItem);
   }
}

}
