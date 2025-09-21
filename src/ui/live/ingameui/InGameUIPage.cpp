// license:GPLv3+

#include "core/stdafx.h"

#include "InGameUIPage.h"

#include "fonts/IconsForkAwesome.h"

namespace VPX::InGameUI
{

static constexpr const char* SELECT_TABLE_OR_GLOBAL = "Save globally or for table ?";
static constexpr const char* CONFIRM_GLOBAL_SAVE = "Save to global setting ?";

InGameUIPage::InGameUIPage(const string& path, const string& title, const string& info, SaveMode saveMode)
   : m_player(g_pplayer)
   , m_path(path)
   , m_title(title)
   , m_info(info)
   , m_saveMode(saveMode)
{
   assert(m_player);
}

void InGameUIPage::Open()
{
   m_selectedItem = 0;
   m_pressedItemLabel = ""s;
}

void InGameUIPage::Close() { }

void InGameUIPage::ClearItems() { m_items.clear(); }

void InGameUIPage::AddItem(std::unique_ptr<InGameUIItem>& item) { m_items.push_back(std::move(item)); }

Settings& InGameUIPage::GetSettings() { return m_player->m_ptable->m_settings; }

bool InGameUIPage::IsAdjustable() const
{
   for (const auto& item : m_items)
      if (item->IsAdjustable())
         return true;
   return false;
}

bool InGameUIPage::IsDefaults() const
{
   for (const auto& item : m_items)
      if (!item->IsDefaultValue())
         return false;
   return true;
}

bool InGameUIPage::IsModified() const
{
   for (const auto& item : m_items)
      if (item->IsModified())
         return true;
   return false;
}

InGameUIItem* InGameUIPage::GetItem(const string& label) const
{
   for (const auto& item : m_items)
      if (item->m_label == label)
         return item.get();
   return nullptr;
}

void InGameUIPage::ResetToInitialValues()
{
   if (!IsModified())
      return;
   // Note that changing the value of items may result in changing the content of m_items (page rebuilding)
   assert(!m_resettingToInitialValues);
   m_resettingToInitialValues = true;
   for (size_t i = 0; i < m_items.size(); i++)
      m_items[i]->ResetToInitialValue();
   g_pplayer->m_liveUI->PushNotification("Changes were undone"s, 5000);
   if (!IsModified())
      m_selectedItem = 0;
   m_resettingToInitialValues = false;
}

void InGameUIPage::ResetToDefaults()
{
   if (IsDefaults())
      return;
   // Note that changing the value of items may result in changing the content of m_items (page rebuilding)
   assert(!m_resettingToDefaults);
   m_resettingToDefaults = true;
   for (size_t i = 0; i < m_items.size(); i++)
      m_items[i]->ResetToDefault();
   m_resetNotifId = g_pplayer->m_liveUI->PushNotification("Settings reset to defaults"s, 5000, m_resetNotifId);
   if (IsDefaults())
      m_selectedItem = 0;
   m_resettingToDefaults = false;
}

void InGameUIPage::Save()
{
   if (!IsModified())
      return;
   const bool canSaveTableOverrides = FileExists(m_player->m_ptable->m_filename);
   switch (m_saveMode)
   {
   case SaveMode::Both:
      m_selectGlobalOrTablePopup = canSaveTableOverrides;
      m_selectGlobalOrDiscardPopup = !canSaveTableOverrides;
      break;

   case SaveMode::Global:
      SaveGlobally();
      break;

   case SaveMode::Table:
      if (!canSaveTableOverrides)
         m_player->m_liveUI->PushNotification("You need to save the table before saving table setting overrides", 5000);
      else
         SaveTableOverride();
      break;
   }
   if (!IsModified())
      m_selectedItem = 0;
}

void InGameUIPage::SaveGlobally()
{
   // First reset any table override
   Settings& settings = m_player->m_ptable->m_settings;
   for (const auto& item : m_items)
      item->ResetSave(settings);
   // Then save to application settings
   settings = g_pvp->m_settings;
   for (const auto& item : m_items)
      item->Save(settings, false);
}

void InGameUIPage::SaveTableOverride()
{
   // First reset any table override (to start from a clear ground if saved items depends on user selection)
   Settings& settings = m_player->m_ptable->m_settings;
   for (const auto& item : m_items)
      item->ResetSave(settings);
   // Then save to table override
   for (const auto& item : m_items)
      item->Save(settings, true);
}

void InGameUIPage::SelectNextItem()
{
   m_pressedItemScroll = 0.f; // Start from top of item
   const int nItems = static_cast<int>(m_items.size());
   do
      m_selectedItem = (m_selectedItem + 1) % nItems;
   while (!m_items[m_selectedItem]->IsSelectable());
}

void InGameUIPage::SelectPrevItem()
{
   m_pressedItemScroll = 10000.f; // Start from bottom of item
   const int nItems = static_cast<int>(m_items.size());
   do
      m_selectedItem = (m_selectedItem + nItems - 1) % nItems;
   while (!m_items[m_selectedItem]->IsSelectable());
}

void InGameUIPage::AdjustItem(float direction, bool isInitialPress)
{
   if (m_selectedItem < 0 || m_selectedItem > (int)m_items.size())
      return;
   const auto& item = m_items[m_selectedItem];
   const uint32_t now = msec();
   if (isInitialPress)
   {
      m_pressedItemLabel = item->m_label;
      m_pressStartMs = now;
      m_lastUpdateMs = now;
   }
   else if (m_pressedItemLabel != item->m_label)
   {
      // Different item, discard
      return;
   }
   const float elapsed = static_cast<float>(now - m_lastUpdateMs) / 1000.f;
   m_lastUpdateMs = now;
   const uint32_t elapsedSincePress = now - m_pressStartMs;
   float speedFactor;
   if (elapsedSincePress < 250)
      speedFactor = 1.0f;
   else if (elapsedSincePress < 500)
      speedFactor = 2.f;
   else if (elapsedSincePress < 1000)
      speedFactor = 4.f;
   else if (elapsedSincePress < 1500)
      speedFactor = 8.f;
   else
      speedFactor = 16.f;

   switch (item->m_type)
   {
   case InGameUIItem::Type::Info: m_pressedItemScroll += speedFactor * direction; break;

   case InGameUIItem::Type::Navigation: m_player->m_liveUI->m_inGameUI.Navigate(item->m_path); break;

   case InGameUIItem::Type::ResetToDefaults: // Defaults (allow continuously applying as defaults may be dynamic, for example for VR, headtracking, dynamic room exposure,...)
      ResetToDefaults();
      break;

   case InGameUIItem::Type::ResetToInitialValues: // Undo (allow continuously applying, if something else if modifying the values, as we do not forbid it)
      ResetToInitialValues();
      break;

   case InGameUIItem::Type::SaveChanges:
      if (isInitialPress)
         Save();
      break;

   case InGameUIItem::Type::Back:
      if (isInitialPress)
         m_player->m_liveUI->m_inGameUI.NavigateBack();
      break;

   case InGameUIItem::Type::EnumValue:
      if (isInitialPress)
      {
         if (direction < 0.f)
            item->SetValue((item->GetIntValue() + static_cast<int>(item->m_enum.size()) - 1) % static_cast<int>(item->m_enum.size()));
         else
            item->SetValue((item->GetIntValue() + 1) % static_cast<int>(item->m_enum.size()));
      }
      break;

   case InGameUIItem::Type::FloatValue:
      if (isInitialPress)
         m_adjustedValue = item->GetFloatValue();
      m_adjustedValue += direction * speedFactor * elapsed * (item->m_maxValue - item->m_minValue) / 32.f;
      item->SetValue(m_adjustedValue);
      break;

   case InGameUIItem::Type::IntValue:
      if (isInitialPress)
         m_adjustedValue = static_cast<float>(item->GetIntValue()) + direction;
      else
         m_adjustedValue += direction * speedFactor * elapsed * (item->m_maxValue - item->m_minValue) / 32.f;
      item->SetValue(static_cast<int>(round(m_adjustedValue)));
      break;

   case InGameUIItem::Type::Toggle:
      if (isInitialPress)
         item->SetValue(!item->GetBoolValue());
      break;

   default: break;
   }
}

void InGameUIPage::Render()
{
   const ImGuiIO& io = ImGui::GetIO();
   const ImGuiStyle& style = ImGui::GetStyle();

   ImGui::SetNextWindowBgAlpha(0.5f);
   if (m_player->m_vrDevice)
   {
      const float size = min(0.25f * io.DisplaySize.x, 0.25f * io.DisplaySize.y);
      ImGui::SetNextWindowSize(ImVec2(size, size));
      ImGui::SetNextWindowPos(ImVec2(0.5f * io.DisplaySize.x, 0.5f * io.DisplaySize.y), 0, ImVec2(0.5f, 0.5f));
   }
   else
   {
      if (io.DisplaySize.x > io.DisplaySize.y)
         ImGui::SetNextWindowSize(ImVec2(0.4f * io.DisplaySize.x, 0.4f * io.DisplaySize.y));
      else
         ImGui::SetNextWindowSize(ImVec2(0.8f * io.DisplaySize.x, 0.3f * io.DisplaySize.y));
      ImGui::SetNextWindowPos(ImVec2(0.5f * io.DisplaySize.x, 0.8f * io.DisplaySize.y), 0, ImVec2(0.5f, 1.f));
   }
   ImGui::Begin("InGameUI", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings);

   ImGui::PushStyleColor(ImGuiCol_Separator, style.Colors[ImGuiCol_Text]);

   const bool isAdjustable = IsAdjustable();

   // Header
   std::erase_if(m_items,
      [](auto& item)
      {
         using enum InGameUIItem::Type;
         return item->m_type == SaveChanges || item->m_type == ResetToDefaults || item->m_type == ResetToInitialValues || item->m_type == Back;
      });
   ImGui::SeparatorText(m_title.c_str());
   bool undoHovered = false;
   bool defaultHovered = false;
   bool saveHovered = false;
   float buttonWidth = ImGui ::CalcTextSize(ICON_FK_REPLY, nullptr, true).x + style.FramePadding.x * 2.0f + style.ItemSpacing.x;
   if (isAdjustable)
   {
      buttonWidth += ImGui::CalcTextSize(ICON_FK_HEART, nullptr, true).x + style.FramePadding.x * 2.0f;
      buttonWidth += style.ItemSpacing.x;
      buttonWidth += ImGui::CalcTextSize(ICON_FK_UNDO, nullptr, true).x + style.FramePadding.x * 2.0f;
      buttonWidth += style.ItemSpacing.x;
      buttonWidth += ImGui::CalcTextSize(ICON_FK_FLOPPY_O, nullptr, true).x + style.FramePadding.x * 2.0f;
      buttonWidth += style.ItemSpacing.x;
   }
   ImGui::SameLine(ImGui::GetWindowSize().x - buttonWidth);
   if (isAdjustable)
   {
      bool highlighted = false;
      // Reset to defaults
      if (!IsDefaults())
      {
         m_items.push_back(std::make_unique<InGameUIItem>(InGameUIItem::Type::ResetToDefaults));
         ImGui::BeginDisabled(false);
         highlighted = m_player->m_liveUI->m_inGameUI.IsFlipperNav() && (m_selectedItem == m_items.size() - 1);
      }
      else
      {
         ImGui::BeginDisabled(true);
         highlighted = false;
      }
      if (highlighted)
         ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
      if (ImGui::Button(ICON_FK_HEART))
      {
         m_selectedItem = static_cast<int>(m_items.size()) - 1;
         AdjustItem(1.f, true);
      }
      if (highlighted)
         ImGui::PopStyleColor();
      defaultHovered = ImGui::IsItemHovered();
      ImGui::EndDisabled();

      // Undo changes
      ImGui::SameLine();
      if (IsModified())
      {
         m_items.push_back(std::make_unique<InGameUIItem>(InGameUIItem::Type::ResetToInitialValues));
         ImGui::BeginDisabled(false);
         highlighted = m_player->m_liveUI->m_inGameUI.IsFlipperNav() && (m_selectedItem == m_items.size() - 1);
      }
      else
      {
         ImGui::BeginDisabled(true);
         highlighted = false;
      }
      if (highlighted)
         ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
      if (ImGui::Button(ICON_FK_UNDO))
      {
         m_selectedItem = static_cast<int>(m_items.size()) - 1;
         AdjustItem(1.f, true);
      }
      if (highlighted)
         ImGui::PopStyleColor();
      undoHovered = ImGui::IsItemHovered();
      ImGui::EndDisabled();

      // Save changes
      ImGui::SameLine();
      if (IsModified())
      {
         m_items.push_back(std::make_unique<InGameUIItem>(InGameUIItem::Type::SaveChanges));
         ImGui::BeginDisabled(false);
         highlighted = m_player->m_liveUI->m_inGameUI.IsFlipperNav() && (m_selectedItem == m_items.size() - 1);
      }
      else
      {
         ImGui::BeginDisabled(true);
         highlighted = false;
      }
      if (highlighted)
         ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
      if (ImGui::Button(ICON_FK_FLOPPY_O))
      {
         m_selectedItem = static_cast<int>(m_items.size()) - 1;
         AdjustItem(1.f, true);
      }
      if (highlighted)
         ImGui::PopStyleColor();
      saveHovered = ImGui::IsItemHovered();
      ImGui::EndDisabled();
      ImGui::SameLine();
   }

   // Get back to previous page or to game
   m_items.push_back(std::make_unique<InGameUIItem>(InGameUIItem::Type::Back));
   const bool highlighted = m_player->m_liveUI->m_inGameUI.IsFlipperNav() && (m_selectedItem == m_items.size() - 1);
   if (highlighted)
      ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
   if (ImGui::Button(ICON_FK_REPLY))
   {
      m_selectedItem = static_cast<int>(m_items.size()) - 1;
      AdjustItem(1.f, true);
   }
   if (highlighted)
      ImGui::PopStyleColor();
   const bool backHovered = ImGui::IsItemHovered();

   // As we may have changed the number of selectable items, ensure m_selectedItem is still valid
   m_selectedItem = clamp(m_selectedItem, 0, static_cast<int>(m_items.size()) - 1);

   // Page items
   // Note that items may trigger state change which in turn may trigger a rebuild of the page (changing m_items)
   const InGameUIItem* hoveredItem = nullptr;
   const ImVec2 itemPadding = style.ItemSpacing;
   ImGui::BeginChild("PageItems", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y - ImGui::GetTextLineHeight() * 3.f - itemPadding.y * 2.f), ImGuiChildFlags_None,
      ImGuiWindowFlags_NoBackground);
   float labelEndScreenX = 0.f;
   for (const auto& item : m_items)
      if (item->IsAdjustable())
         labelEndScreenX = max(labelEndScreenX, ImGui::CalcTextSize(item->m_label.c_str()).x);
   labelEndScreenX = ImGui::GetCursorScreenPos().x + labelEndScreenX + style.ItemSpacing.x * 2.0f + 30.f;
   const float itemEndScreenX = ImGui::GetCursorScreenPos().x + ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("*", nullptr, true).x - itemPadding.x;
   for (int i = 0; i < (int)m_items.size(); i++)
   {
      using enum InGameUIItem::Type;
      const auto& item = m_items[i];

      // Skip as these items are rendered in the header
      if (item->m_type == Back || item->m_type == SaveChanges || item->m_type == ResetToDefaults || item->m_type == ResetToInitialValues)
         continue;

      const float itemHeight = ImGui::GetTextLineHeight() + itemPadding.y * 2.f;
      const bool isMouseOver = (ImGui::IsWindowHovered()) && (ImGui::GetMousePos().y >= ImGui::GetCursorScreenPos().y - itemPadding.y - 1.f)
         && (ImGui::GetMousePos().y <= ImGui::GetCursorScreenPos().y + itemHeight - itemPadding.y);
      const bool hovered = (m_player->m_liveUI->m_inGameUI.IsFlipperNav() && i == m_selectedItem) || (!m_player->m_liveUI->m_inGameUI.IsFlipperNav() && isMouseOver && item->IsSelectable());
      if (hovered)
      {
         hoveredItem = item.get();
         ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
         ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetCursorScreenPos() - itemPadding,
            ImGui::GetCursorScreenPos() + ImVec2(itemPadding.x, itemPadding.y * 2.f) + ImVec2(itemEndScreenX - ImGui::GetCursorScreenPos().x + itemPadding.x, ImGui::GetTextLineHeight()),
            IM_COL32(0, 255, 0, 50));
         if (m_player->m_liveUI->m_inGameUI.IsFlipperNav())
         {
            if (ImGui::GetCursorPosY() - ImGui::GetScrollY() < 0.f)
               ImGui::SetScrollY(ImGui::GetCursorPosY());
            else if (ImGui::GetCursorPosY() - ImGui::GetScrollY() > ImGui::GetWindowHeight() - itemHeight)
               ImGui::SetScrollY(ImGui::GetCursorPosY() - ImGui::GetWindowHeight() + itemHeight);
         }
      }

      switch (item->m_type)
      {
      case Info:
      {
         float infoHeight = ImGui::GetCursorPosY();
         ImGui::SetCursorScreenPos(ImGui::GetCursorScreenPos() + ImVec2(0.f, itemPadding.y));
         ImGui::SetNextItemWidth(itemEndScreenX - ImGui::GetCursorScreenPos().x - 60.f);
         m_player->m_liveUI->SetMarkdownStartId(ImGui::GetItemID());
         ImGui::Markdown(item->m_label.c_str(), item->m_label.length(), m_player->m_liveUI->GetMarkdownConfig());
         ImGui::SetCursorScreenPos(ImGui::GetCursorScreenPos() + ImVec2(0.f, itemPadding.y));
         infoHeight = ImGui::GetCursorPosY() - infoHeight;
         if (hovered && m_player->m_liveUI->m_inGameUI.IsFlipperNav() && infoHeight > ImGui::GetWindowHeight())
         {
            const float scrollSpread = infoHeight - ImGui::GetWindowHeight();
            m_pressedItemScroll = clamp(m_pressedItemScroll, 0.f, scrollSpread);
            ImGui::SetScrollY(ImGui::GetCursorPosY() - infoHeight + m_pressedItemScroll);
         }
         break;
      }

      case Navigation:
         ImGui::SetCursorScreenPos(ImGui::GetCursorScreenPos() + ImVec2(0.f, itemPadding.y));
         ImGui::Text(ICON_FK_ANGLE_DOUBLE_RIGHT);
         ImGui::SameLine(0.f, 10.f);
         ImGui::Text("%s", item->m_label.c_str());
         ImGui::SetCursorScreenPos(ImGui::GetCursorScreenPos() + ImVec2(0.f, itemPadding.y));
         if (isMouseOver && ImGui::IsMouseClicked(ImGuiMouseButton_::ImGuiMouseButton_Left))
         {
            m_selectedItem = i;
            AdjustItem(1.f, true);
         }
         break;

      case Toggle:
      {
         ImGui::Text("%s", item->m_label.c_str());
         ImGui::SameLine(labelEndScreenX - ImGui::GetCursorScreenPos().x);
         bool v = item->GetBoolValue();
         ImGui::SetNextItemWidth(itemEndScreenX - ImGui::GetCursorScreenPos().x);
         // Basic Toggle button, based on https://github.com/ocornut/imgui/issues/1537#issuecomment-355562097
         {
            ImVec2 p = ImGui::GetCursorScreenPos();
            ImDrawList* draw_list = ImGui::GetWindowDrawList();

            const float height = ImGui::GetFrameHeight();
            const float width = height * 1.75f;
            const float radius = height * 0.50f;
            p.x = itemEndScreenX - width;

            if (ImGui::InvisibleButton(item->m_label.c_str(), ImVec2(itemEndScreenX - ImGui::GetCursorScreenPos().x, height)))
               v = !v;
            ImU32 col_bg;
            if (ImGui::IsItemHovered())
               col_bg = v ? IM_COL32(64 + 20, 180, 32 + 20, 255) : IM_COL32(64, 64, 64, 255);
            else
               col_bg = v ? IM_COL32(64, 180, 32, 255) : IM_COL32(37, 37, 37, 255);

            draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), col_bg, height * 0.5f);
            draw_list->AddCircleFilled(ImVec2(v ? (p.x + width - radius) : (p.x + radius), p.y + radius), radius - 3.5f, IM_COL32(255, 255, 255, 255));
         }
         item->SetValue(v);
         if (item->IsModified())
         {
            ImGui::SameLine(itemEndScreenX - ImGui::GetCursorScreenPos().x);
            ImGui::Text(ICON_FK_PENCIL);
         }
         break;
      }

      case EnumValue:
      {
         ImGui::Text("%s", item->m_label.c_str());
         ImGui::SameLine(labelEndScreenX - ImGui::GetCursorScreenPos().x);
         int v = item->GetIntValue();
         ImGui::SetNextItemWidth(itemEndScreenX - ImGui::GetCursorScreenPos().x);
         ImGui::Combo(("##" + item->m_label).c_str(), &v,
            [](void* data, int idx, const char** out_text)
            {
               const auto* vec = static_cast<const vector<string>*>(data);
               if (idx < 0 || idx >= (int)vec->size())
                  return false;
               *out_text = vec->at(idx).c_str();
               return true;
            },
            (void*)&item->m_enum, (int)item->m_enum.size());
         item->SetValue(v);
         if (item->IsModified())
         {
            ImGui::SameLine(itemEndScreenX - ImGui::GetCursorScreenPos().x);
            ImGui::Text(ICON_FK_PENCIL);
         }
         break;
      }

      case FloatValue:
      {
         ImGui::Text("%s", item->m_label.c_str());
         ImGui::SameLine(labelEndScreenX - ImGui::GetCursorScreenPos().x);
         float v = item->GetFloatValue();
         ImGui::SetNextItemWidth(itemEndScreenX - ImGui::GetCursorScreenPos().x);
         ImGui::SliderFloat(("##" + item->m_label).c_str(), &v, item->m_minValue, item->m_maxValue, item->m_format.c_str(), ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_NoRoundToFormat);
         item->SetValue(v);
         if (item->IsModified())
         {
            ImGui::SameLine(itemEndScreenX - ImGui::GetCursorScreenPos().x);
            ImGui::Text(ICON_FK_PENCIL);
         }
         break;
      }

      case IntValue:
      {
         ImGui::Text("%s", item->m_label.c_str());
         ImGui::SameLine(labelEndScreenX - ImGui::GetCursorScreenPos().x);
         int v = item->GetIntValue();
         ImGui::SetNextItemWidth(itemEndScreenX - ImGui::GetCursorScreenPos().x);
         ImGui::SliderInt(("##" + item->m_label).c_str(), &v, static_cast<int>(item->m_minValue), static_cast<int>(item->m_maxValue), item->m_format.c_str(),
            ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_NoRoundToFormat);
         item->SetValue(v);
         if (item->IsModified())
         {
            ImGui::SameLine(itemEndScreenX - ImGui::GetCursorScreenPos().x);
            ImGui::Text(ICON_FK_PENCIL);
         }
         break;
      }

      default: assert(false); break;
      }

      if (hovered)
         ImGui::PopStyleColor();
   }
   ImGui::Dummy(ImVec2(0, 0));
   ImGui::EndChild();

   ImGui::Separator();

   ImGui::BeginChild("Info", ImVec2(), ImGuiChildFlags_None, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar);
   if (hoveredItem && std::ranges::find_if(m_items, [hoveredItem](auto& item) { return item.get() == hoveredItem; }) != m_items.end() && !hoveredItem->m_tooltip.empty())
      ImGui::TextWrapped("%s", hoveredItem->m_tooltip.c_str());
   else if (undoHovered)
      ImGui::TextWrapped("Undo changes\n[Input shortcut: Credit Button]");
   else if (defaultHovered)
      ImGui::TextWrapped("Reset page to defaults\n[Input shortcut: Launch Button]");
   else if (saveHovered)
      ImGui::TextWrapped("Save changes\n[Input shortcut: Start Button]");
   else if (backHovered)
      ImGui::TextWrapped("Get back\n[Input shortcut: Quit Button]");
   else if (!m_info.empty())
      ImGui::TextWrapped("%s", m_info.c_str());
   ImGui::EndChild();

   ImGui::PopStyleColor();

   ImGui::End();

   if (m_selectGlobalOrTablePopup)
      ImGui::OpenPopup(SELECT_TABLE_OR_GLOBAL);
   if (ImGui::BeginPopupModal(SELECT_TABLE_OR_GLOBAL, &m_selectGlobalOrTablePopup))
   {
      if (ImGui::Button("Save changes globally"))
      {
         SaveGlobally();
         ImGui::CloseCurrentPopup();
         m_selectGlobalOrTablePopup = false;
      }
      if (ImGui::Button("Save only for this table"))
      {
         SaveTableOverride();
         ImGui::CloseCurrentPopup();
         m_selectGlobalOrTablePopup = false;
      }
      ImGui::EndPopup();
   }

   if (m_selectGlobalOrDiscardPopup)
      ImGui::OpenPopup(CONFIRM_GLOBAL_SAVE);
   if (ImGui::BeginPopupModal(CONFIRM_GLOBAL_SAVE, &m_selectGlobalOrDiscardPopup))
   {
      ImGui::Text("This table has not been saved yet.\nChanges may only be saved to global settings.\n\nDo you want to save to global settings?\n");
      if (ImGui::Button("Don't Save"))
      {
         ImGui::CloseCurrentPopup();
         m_selectGlobalOrDiscardPopup = false;
      }
      ImGui::SameLine();
      if (ImGui::Button("Save changes globally"))
      {
         SaveGlobally();
         ImGui::CloseCurrentPopup();
         m_selectGlobalOrDiscardPopup = false;
      }
      ImGui::EndPopup();
   }
}


}
