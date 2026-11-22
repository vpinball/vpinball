// license:GPLv3+

#pragma once

#include "imgui/imgui.h"

class Player;

namespace VPX::InGameUI
{

class InGameUIPage;

// Widgets shared by the InGameUI renderer implementations (card layout, desktop
// fullscreen layout): the renderers own widget-level interaction while pages
// keep the model.

// Basic Toggle button, based on https://github.com/ocornut/imgui/issues/1537#issuecomment-355562097
void RenderToggle(Player* player, const string& label, const ImVec2& size, bool& v);

// Draws text truncated with an ellipsis when it exceeds maxWidth
void TextWithEllipsis(const string& text, float maxWidth);

// Input binding popup: the page keeps the request state and the policy that a
// navigation action must always be bound; the renderer draws the modal and
// applies the captured mapping.
void RenderInputActionPopup(Player* player, InGameUIPage& page);

}
