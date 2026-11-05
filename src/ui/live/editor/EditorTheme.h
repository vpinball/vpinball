// Live editor palette (Blender-like dark theme). One place for the colours the editor panels draw by hand.
#pragma once

#include "imgui/imgui.h"

namespace EditorTheme
{
constexpr ImU32 Rgb(int r, int g, int b, int a = 255) { return IM_COL32(r, g, b, a); }

// Surfaces
constexpr ImU32 Bar = Rgb(36, 36, 37); // menu bar + toolbar
constexpr ImU32 PlayBar = Rgb(62, 44, 32); // menu bar + toolbar + status bar while inspecting a running play test (warm tint of Bar)
constexpr ImU32 Strip = Rgb(30, 30, 31); // vertical tab strip / column backgrounds
constexpr ImU32 Raised = Rgb(66, 66, 66); // active tab / active toolbar button
constexpr ImU32 Hover = Rgb(80, 80, 80); // hovered button
constexpr ImU32 Inactive = Rgb(40, 40, 41); // inactive tab
constexpr ImU32 Workspace = Rgb(48, 48, 49); // active workspace tab (connects to the bar)
constexpr ImU32 WorkspaceOff = Rgb(28, 28, 29); // inactive workspace tab
constexpr ImU32 Border = Rgb(50, 50, 50); // 1 px seams
constexpr ImU32 RowSelected = Rgb(51, 77, 110); // selected outliner row

// Accents
constexpr ImU32 Accent = Rgb(237, 145, 62); // orange: active icon, focus
constexpr ImU32 Fill = Rgb(71, 114, 179); // slider fill / selected search row (Blender #4772B3)
constexpr ImU32 FillHovered = Rgb(85, 130, 190);
constexpr ImU32 FillActive = Rgb(96, 146, 205);

// Text
constexpr ImU32 Text = Rgb(230, 230, 230);
constexpr ImU32 TextDim = Rgb(150, 150, 150); // meta text, counts
constexpr ImU32 TextFaint = Rgb(110, 110, 110); // hidden items, disabled icons
constexpr ImU32 IconMuted = Rgb(140, 140, 140); // remove buttons

// Tabs (sidebar tabs, workspace tabs): inactive tabs are transparent
constexpr ImU32 TabHover = Rgb(70, 70, 72);
constexpr ImU32 TabActive = Rgb(80, 80, 82);

inline ImVec4 V(ImU32 c) { return ImGui::ColorConvertU32ToFloat4(c); }

// Outliner / viewport overlay additions (round 4)
constexpr ImU32 RowSelectedHover = Rgb(61, 90, 128); // hovered selected outliner row
constexpr ImU32 RowHover = Rgb(255, 255, 255, 18); // faint row hover highlight (favourite rows)
constexpr ImU32 OverlayGrey = Rgb(120, 120, 124); // neutral overlay lines (nav square)
constexpr ImU32 TextShadow = Rgb(0, 0, 0, 200); // shadow of text drawn over the render

// Properties search: highlight behind the matched part of a field label
constexpr ImU32 SearchMatch = Rgb(237, 145, 62, 110);
}
