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

// A tab of the desktop chrome's top bar (letter-spaced label)
struct DesktopTopTab
{
   string label;
   string group; // Widget id; compared to the selected group for the accent underline
   bool right = false; // Right-justified group, drawn in the secondary style
   int line = 0; // Line the tab sits on when the bar folds to two lines
   float scale = 1.f; // Text scale within the tab font size
   std::function<void()> activate;
};

// Whether the top bar folds to two lines: always on portrait frames, otherwise
// when the tab labels do not fit the given width
bool DesktopTopBarFolds(Player* player, const vector<DesktopTopTab>& tabs, float width, bool portrait);

// The height the top bar occupies: one line, or two when folded
float DesktopTopBarHeight(Player* player, bool folded);

// The InGameUI desktop chrome's top bar: a dark band with a bottom hairline,
// left-justified letter-spaced tabs and right-justified action tabs, the first
// tab set apart by a separator; the tab of `selectedGroup` is underlined and the
// `focused` one (-1 = none) gets the focus ring. A pressed tab runs its activate callback.
void RenderDesktopTopBar(Player* player, ImDrawList* drawList, const ImVec2& pos, float width, const vector<DesktopTopTab>& tabs, bool folded, const string& selectedGroup, int focused);

// A key hint of the desktop chrome's bottom bar (key chip + action label)
struct DesktopHint
{
   string key;
   string label;
};

// The height the bottom hint bar occupies
float DesktopHintBarHeight(Player* player);

// The InGameUI desktop chrome's bottom hint bar: a dark band with a top hairline
// and centered key chips sized on their glyph ink; the two first hints (the
// universal navigation ones) drop when they do not all fit
void RenderDesktopHintBar(Player* player, ImDrawList* drawList, const ImVec2& pos, float width, vector<DesktopHint>& hints);
}
