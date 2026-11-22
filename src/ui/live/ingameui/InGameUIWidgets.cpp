// license:GPLv3+

#include "core/stdafx.h"
#include "InGameUIWidgets.h"

#include "InGameUIPage.h"

#include "core/VPApp.h"
#include "ui/live/LiveUI.h"

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
         if (!(item->m_inputAction->IsNavigationAction() && !item->m_inputAction->IsMapped()))
         {
            ImGui::CloseCurrentPopup();
            page.CloseInputActionPopup();
         }
      }
      ImGui::EndPopup();
   }
}

}
