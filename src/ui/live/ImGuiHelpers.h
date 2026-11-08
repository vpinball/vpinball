// Shared ImGui helpers for the live UI.
//
// ImGui sizes text and buttons on the *layout box*: CalcTextSize covers glyph advances
// and the font's line height. The merged icon glyphs are painted a bit larger than that
// box, so widgets sized on it let the icon ink overflow (toolbar buttons touching the
// pane border, status bar key chips). TextInkBounds measures what is really painted so
// widgets can be sized and offset to contain it.
#pragma once

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h" // ImFontBaked, ImTextCharFromUtf8

// Ink bounds of a text run, relative to the text origin, as (x0, y0, x1, y1). The bounds
// union the visible glyphs' boxes with the origin point, so a run without ink is {0,0,0,0}.
inline ImVec4 TextInkBounds(const char *text, const char *textEnd = nullptr)
{
   if (textEnd == nullptr)
      textEnd = text + strlen(text);
   ImFontBaked *const baked = ImGui::GetFontBaked(); // FindGlyph is non-const in this ImGui version
   float x0 = 0.f, y0 = 0.f, x1 = 0.f, y1 = 0.f;
   float penX = 0.f;
   for (const char *s = text; s < textEnd;)
   {
      unsigned int c = 0;
      s += ImTextCharFromUtf8(&c, s, textEnd);
      const ImFontGlyph *const glyph = c != 0 ? baked->FindGlyph(static_cast<ImWchar>(c)) : nullptr;
      if (glyph == nullptr)
         continue;
      if (glyph->Visible)
      {
         x0 = ImMin(x0, penX + glyph->X0);
         x1 = ImMax(x1, penX + glyph->X1);
         y0 = ImMin(y0, glyph->Y0);
         y1 = ImMax(y1, glyph->Y1);
      }
      penX += glyph->AdvanceX;
   }
   return ImVec4(x0, y0, x1, y1);
}

// Icon button sized on the glyph's ink bounds (its ink is drawn aligned inside the frame
// instead of overflowing it, as ImGui::Button(icon) does when the glyph exceeds the line
// box). `id` is the widget id (typically "##Name"), `icon` the glyph(s) drawn inside.
inline bool IconButtonInk(const char *icon, const char *id)
{
   const ImGuiStyle &style = ImGui::GetStyle();
   const ImVec4 ink = TextInkBounds(icon);
   const ImVec2 box = ImGui::CalcTextSize(icon);
   const ImVec2 size(ImMax(box.x, ink.z - ink.x) + style.FramePadding.x * 2.f, ImMax(box.y, ink.w - ink.y) + style.FramePadding.y * 2.f);
   const bool pressed = ImGui::Button(id, size);
   const ImVec2 min = ImGui::GetItemRectMin();
   ImGui::GetWindowDrawList()->AddText(ImVec2(min.x + style.FramePadding.x - ink.x, min.y + style.FramePadding.y - ink.y), ImGui::GetColorU32(ImGuiCol_Text), icon);
   return pressed;
}
