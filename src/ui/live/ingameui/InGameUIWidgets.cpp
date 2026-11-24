// license:GPLv3+

#include "core/stdafx.h"
#include "InGameUIWidgets.h"

#include "InGameUIPage.h"

#include "core/VPApp.h"
#include "ui/live/LiveUI.h"
#include "ui/live/ImGuiHelpers.h"

#include "imgui/imgui_internal.h"

namespace VPX::InGameUI
{

void RenderToggle(Player* player, const string& label, const ImVec2& size, bool& v)
{
   ImVec2 p = ImGui::GetCursorScreenPos();
   ImDrawList* draw_list = ImGui::GetWindowDrawList();

   const float height = size.y;
   const float width = height * 1.75f;
   const float radius = height * 0.50f;
   float itemEndScreenX = ImGui::GetCursorScreenPos().x + size.x;
   p.x = itemEndScreenX - width;

   ImGui::SetNextItemWidth(size.x);
   if (ImGui::InvisibleButton(label.c_str(), size))
      v = !v;
   const ImU32 col_bg = ImGui::IsItemHovered() ? ImGui::GetColorU32(ImGuiCol_ButtonHovered) : ImGui::GetColorU32(ImGuiCol_Button);
   draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), col_bg, height * 0.5f);
   if (v)
      draw_list->AddCircleFilled(ImVec2(v ? (p.x + width - radius) : (p.x + radius), p.y + radius), radius - 3.5f, IM_COL32(255, 255, 255, 255));
   else
      draw_list->AddCircle(ImVec2(v ? (p.x + width - radius) : (p.x + radius), p.y + radius), radius - 3.5f, IM_COL32(255, 255, 255, 255), 0, 2.f * player->m_liveUI->GetDPI());
}

void TextWithEllipsis(const string& textStr, float maxWidth)
{
   float textWidth = ImGui::CalcTextSize(textStr.c_str()).x;
   if (textWidth > maxWidth)
   {
      float ellipsisWidth = ImGui::CalcTextSize("...").x;
      if (float availableWidth = maxWidth - ellipsisWidth; availableWidth <= 0)
      {
         ImGui::Text("...");
         return;
      }

      // Binary search to find the right length
      int left = 0;
      int right = static_cast<int>(textStr.length());
      while (left < right)
      {
         int mid = (left + right + 1) / 2;
         std::string truncated = textStr.substr(0, mid) + "...";
         float truncatedWidth = ImGui::CalcTextSize(truncated.c_str()).x;
         if (truncatedWidth <= maxWidth)
         {
            left = mid;
         }
         else
         {
            right = mid - 1;
         }
      }

      std::string truncated = textStr.substr(0, left) + "...";
      ImGui::Text("%s", truncated.c_str());
   }
   else
   {
      ImGui::Text("%s", textStr.c_str());
   }
}

void RenderInputActionPopup(Player* player, InGameUIPage& page)
{
   InGameUIItem* const item = page.GetInputActionPopupItem();
   if (item && page.IsInputActionPopupPending() && !ImGui::IsPopupOpen((item->m_label + " input binding").c_str()))
   {
      ImGui::OpenPopup((item->m_label + " input binding").c_str());
      ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
      ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoKeyboard;
      player->m_pininput.StartButtonCapture();
   }
   if (item && ImGui::BeginPopupModal((item->m_label + " input binding").c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
   {
      ImGui::Text("Press the key/button or combination of keys/buttons\nyou want to use for the action '%s'", item->m_label.c_str());
      ImGui::Spacing();
      ImGui::Separator();
      ImGui::Spacing();
      ImGui::Text("New mapping: %s", player->m_pininput.GetMappingLabel(player->m_pininput.GetButtonCapture()).c_str());
      ImGui::Spacing();
      ImGui::Separator();
      if (player->m_pininput.IsButtonCaptureDone())
      {
         ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
         ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoKeyboard;
         item->m_inputAction->AddMapping(player->m_pininput.GetButtonCapture());
         player->m_pininput.EndButtonCapture();
         ImGui::CloseCurrentPopup();
         page.CloseInputActionPopup();
      }
      ImGui::EndPopup();
   }
}

bool DesktopTopBarFolds(Player* player, const vector<DesktopTopTab>& tabs, const float width, const bool portrait)
{
   if (portrait)
      return true;
   const float dpi = player->m_liveUI->GetDPI();
   ImFont* const tabFont = player->m_liveUI->GetOverlayFont() ? player->m_liveUI->GetOverlayFont() : ImGui::GetFont();
   const float tabSize = ImGui::GetFontSize() * 1.6f;
   const float tracking = tabSize * 0.30f;
   const float tabGap = 56.f * dpi;
   float leftW = 28.f * dpi, rightW = 28.f * dpi;
   for (const DesktopTopTab& tab : tabs)
      (tab.right ? rightW : leftW) += SpacedTextWidth(tabFont, tabSize * tab.scale, tab.label.c_str(), tracking) + tabGap;
   return leftW + rightW - tabGap > width;
}

float DesktopTopBarHeight(Player* player, const bool folded)
{
   const float dpi = player->m_liveUI->GetDPI();
   return folded ? 2.f * (ImGui::GetFrameHeight() + 16.f * dpi) : ImGui::GetFrameHeight() + 40.f * dpi;
}

void RenderDesktopTopBar(
   Player* player, ImDrawList* const drawList, const ImVec2& pos, const float width, const vector<DesktopTopTab>& tabs, const bool folded, const string& selectedGroup, const int focused)
{
   const float dpi = player->m_liveUI->GetDPI();
   ImFont* const tabFont = player->m_liveUI->GetOverlayFont() ? player->m_liveUI->GetOverlayFont() : ImGui::GetFont();
   const float tabSize = ImGui::GetFontSize() * 1.6f;
   const float tracking = tabSize * 0.30f;
   const float tabGap = 56.f * dpi;
   const float tabLineH = ImGui::GetFrameHeight() + 16.f * dpi;
   const float topBarH = DesktopTopBarHeight(player, folded);

   // Darker band with letter-spaced uppercase categories, underlined when
   // selected — categories left, actions on the right. Folded to two lines on
   // portrait frames or on overflow: categories on the first line, the page tab
   // (left) and the actions (right) on the second
   // Solid in VR, for readability and as translucent backgrounds would blend with the mixed reality key color
   const ImU32 barBg = player->IsVR() ? IM_COL32(0, 0, 0, 255) : IM_COL32(0, 0, 0, 120);
   drawList->AddRectFilled(pos, pos + ImVec2(width, topBarH), ImGui::GetColorU32(barBg));
   drawList->AddLine(pos + ImVec2(0.f, topBarH), pos + ImVec2(width, topBarH), ImGui::GetColorU32(IM_COL32(255, 255, 255, 28)));
   // One left and one right accumulator per line: left-justified tabs advance
   // left-to-right, right-justified ones accumulate right-to-left, rightmost first
   float tabX[2] = { pos.x + 28.f * dpi, pos.x + 28.f * dpi };
   float tabXRight[2] = { pos.x + width - 28.f * dpi, pos.x + width - 28.f * dpi };
   const auto renderTab = [&](const int i)
   {
      const DesktopTopTab& tab = tabs[i];
      // On the folded bar, each tab renders on its assigned line at the secondary style
      const int line = folded ? tab.line : 0;
      const float size = tabSize * (folded && line == 1 ? 0.85f : tab.scale);
      const float w = SpacedTextWidth(tabFont, size, tab.label.c_str(), tracking);
      const float lineY = pos.y + line * tabLineH;
      const float lineH = folded ? tabLineH : topBarH;
      const float x = tab.right ? (tabXRight[line] -= w, tabXRight[line]) : tabX[line];
      ImGui::SetCursorScreenPos(ImVec2(x, lineY));
      const bool pressed = ImGui::InvisibleButton(std::format("##tab{}", tab.group).c_str(), ImVec2(w, lineH));
      const bool hovered = ImGui::IsItemHovered();
      const bool selected = selectedGroup == tab.group;
      const float textY = lineY + (lineH - size) * 0.5f;
      DrawSpacedText(drawList, tabFont, size, ImVec2(x, textY), tracking, ImGui::GetColorU32(selected || hovered || i == focused ? ImGuiCol_Text : ImGuiCol_TextDisabled), tab.label.c_str());
      if (selected)
         drawList->AddLine(ImVec2(x, textY + size + 5.f * dpi), ImVec2(x + w, textY + size + 5.f * dpi), ImGui::GetColorU32(IM_COL32(51, 153, 255, 255)), 2.f * dpi);
      if (i == focused)
         drawList->AddRect(
            ImVec2(x - 8.f * dpi, lineY + 4.f * dpi), ImVec2(x + w + 8.f * dpi, lineY + lineH - 4.f * dpi), ImGui::GetColorU32(IM_COL32(110, 180, 255, 255)), 6.f * dpi, 0, 2.5f * dpi);
      if (tab.right)
         tabXRight[line] -= tabGap;
      else
         tabX[line] += w + tabGap;
      // A thin vertical bar sets the first tab (back to the game) apart — only on
      // the single-line bar; in two-line mode it is already set apart on its own
      // line and the bar would float mid-line
      if (i == 0 && !folded)
      {
         const float sepX = x + w + tabGap * 0.5f;
         drawList->AddLine(ImVec2(sepX, lineY + lineH * 0.28f), ImVec2(sepX, lineY + lineH * 0.72f), ImGui::GetColorU32(IM_COL32(255, 255, 255, 60)), 1.f * dpi);
      }
      if (pressed && tab.activate)
         tab.activate();
   };
   for (int i = 0; i < (int)tabs.size() && !tabs[i].right; ++i)
      renderTab(i);
   for (int i = (int)tabs.size() - 1; i >= 0 && tabs[i].right; --i)
      renderTab(i);
}

float DesktopHintBarHeight(Player* player) { return ImGui::GetFrameHeight() + 14.f * player->m_liveUI->GetDPI(); }

void RenderDesktopHintBar(Player* player, ImDrawList* const drawList, const ImVec2& pos, const float width, vector<DesktopHint>& hints)
{
   const float dpi = player->m_liveUI->GetDPI();
   const float hintH = DesktopHintBarHeight(player);

   // Chips are sized on the glyphs' ink bounds like in the live editor:
   // CalcTextSize only covers advances and the line height, and icon glyphs
   // would overflow the chip. One chip height for the whole row.
   const float chipPadX = 5.f * dpi, chipPadY = 1.f * dpi, gap = 6.f * dpi, hintGap = 18.f * dpi;
   const float textH = ImGui::GetFontSize();
   const auto keyInkBounds = [textH](const string& key, float& x0, float& x1, float& y0, float& y1)
   {
      const ImVec4 ink = TextInkBounds(key.c_str());
      x0 = ink.x;
      x1 = ImMax(ink.z, ImGui::CalcTextSize(key.c_str()).x); // union with the layout box
      y0 = ink.y;
      y1 = ImMax(ink.w, textH);
   };
   float chipTop = 0.f, chipBottom = textH;
   const auto measureHints = [&]() -> float
   {
      chipTop = 0.f;
      chipBottom = textH;
      float w = 0.f;
      for (const DesktopHint& hint : hints)
      {
         float x0, x1, y0, y1;
         keyInkBounds(hint.key, x0, x1, y0, y1);
         chipTop = ImMin(chipTop, y0);
         chipBottom = ImMax(chipBottom, y1);
         w += (x1 - x0) + 2.f * chipPadX + gap + ImGui::CalcTextSize(hint.label.c_str()).x + hintGap;
      }
      return w - hintGap;
   };
   float totalW = measureHints();
   // The two first hints are the universal UI navigation hints, the same on
   // every page: drop them when the bar cannot fit all the hints
   if (totalW > width && hints.size() > 2)
   {
      hints.erase(hints.begin(), hints.begin() + 2);
      totalW = measureHints();
   }

   const float hintY = pos.y;
   const float textY = hintY + (hintH - textH) * 0.5f;
   // Solid in VR, for readability and as translucent backgrounds would blend with the mixed reality key color
   const ImU32 barBg = player->IsVR() ? IM_COL32(0, 0, 0, 255) : IM_COL32(0, 0, 0, 120);
   drawList->AddRectFilled(ImVec2(pos.x, hintY), ImVec2(pos.x + width, hintY + hintH), ImGui::GetColorU32(barBg));
   drawList->AddLine(ImVec2(pos.x, hintY), ImVec2(pos.x + width, hintY), ImGui::GetColorU32(IM_COL32(255, 255, 255, 28)));
   float x = pos.x + (width - totalW) * 0.5f;
   for (const DesktopHint& hint : hints)
   {
      float inkX0, inkX1, inkY0, inkY1;
      keyInkBounds(hint.key, inkX0, inkX1, inkY0, inkY1);
      const ImVec2 chipMin(x, textY + chipTop - chipPadY);
      const ImVec2 chipMax(x + (inkX1 - inkX0) + 2.f * chipPadX, textY + chipBottom + chipPadY);
      drawList->AddRectFilled(chipMin, chipMax, ImGui::GetColorU32(IM_COL32(255, 255, 255, 20)), 3.f * dpi);
      drawList->AddRect(chipMin, chipMax, ImGui::GetColorU32(IM_COL32(255, 255, 255, 50)), 3.f * dpi);
      drawList->AddText(ImVec2(chipMin.x + chipPadX - inkX0, chipMin.y + chipPadY - chipTop), ImGui::GetColorU32(ImGuiCol_Text), hint.key.c_str());
      drawList->AddText(ImVec2(chipMax.x + gap, textY), ImGui::GetColorU32(ImGuiCol_TextDisabled), hint.label.c_str());
      x = chipMax.x + gap + ImGui::CalcTextSize(hint.label.c_str()).x + hintGap;
   }
}
}
