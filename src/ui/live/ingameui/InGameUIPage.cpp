// license:GPLv3+

#include "core/stdafx.h"

#include "InGameUIPage.h"

#include "imgui/imgui_internal.h"

#include "SensorSetupPage.h"
#include "fonts/IconsForkAwesome.h"

namespace VPX::InGameUI
{

static constexpr const char* SELECT_TABLE_OR_GLOBAL = "Save globally or for table ?";
static constexpr const char* CONFIRM_GLOBAL_SAVE = "Save to global setting ?";

InGameUIPage::InGameUIPage(const string& title, const string& info, SaveMode saveMode)
   : m_player(g_pplayer)
   , m_title(title)
   , m_info(info)
   , m_saveMode(saveMode)
{
   assert(m_player);
}

void InGameUIPage::Open(bool isBackwardAnimation)
{
   m_openAnimElapsed = 0.f;
   m_openAnimTarget = 0.f;
   // Only apply if page is no more on screen (otherwise it would jump)
   if (m_openAnimPos == -1.f || m_openAnimPos == 1.f)
      m_openAnimPos = isBackwardAnimation ? -1.f : 1.f;
   m_openAnimStart = m_openAnimPos;
   m_selectedItem = 0;
   m_pressedItemLabel = ""s;
}

void InGameUIPage::Close(bool isBackwardAnimation)
{
   m_openAnimElapsed = 0.f;
   m_openAnimTarget = isBackwardAnimation ? 1.f : -1.f;
   m_openAnimStart = m_openAnimPos;
}

void InGameUIPage::ClearItems() { m_items.clear(); }

InGameUIItem& InGameUIPage::AddItem(std::unique_ptr<InGameUIItem> item)
{
   m_items.push_back(std::move(item));
   return *m_items.back();
}

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
      if (!item->m_excludeFromDefault && !item->IsDefaultValue())
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

void InGameUIPage::ResetToStoredValues()
{
   if (!IsModified())
      return;
   // Note that changing the value of items may result in changing the content of m_items (page rebuilding)
   assert(!m_resettingToInitialValues);
   m_resettingToInitialValues = true;
   for (size_t i = 0; i < m_items.size(); i++)
      m_items[i]->ResetToStoredValue();
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
      if (!m_items[i]->m_excludeFromDefault)
         m_items[i]->ResetToDefault();
   m_resetNotifId = g_pplayer->m_liveUI->PushNotification("Settings reset to defaults"s, 5000, m_resetNotifId);
   if (IsDefaults())
      m_selectedItem = 0;
   m_resettingToDefaults = false;
}

class SelectGlobalOrOverridePage final : public InGameUIPage
{
public:
   explicit SelectGlobalOrOverridePage(InGameUIPage* page, bool canSaveTableOverrides)
      : InGameUIPage(canSaveTableOverrides ? "Save globally or as a table override ?"s : "Save changes ?"s,
           canSaveTableOverrides
              ? "If saved globally, your changes will apply for all played tables.\nIf saved as an override, only the settings adjusted for this specific table will be saved."s
              : "This table was not saved yet, therefore the changes can only be saved globally."s,
           SaveMode::None)
      , m_page(page)
   {
      AddItem(std::make_unique<InGameUIItem>("Save Globally"s, "Your changes will be saved as the new default for all tables."s,
         [this]()
         {
            m_page->SaveGlobally();
            m_player->m_liveUI->m_inGameUI.NavigateBack();
         }));
      if (canSaveTableOverrides)
         AddItem(std::make_unique<InGameUIItem>("Save as Table Override"s, "The settings adjusted for this specific table will be persisted for the next time you play this table."s,
            [this]()
            {
               m_page->SaveTableOverride();
               m_player->m_liveUI->m_inGameUI.NavigateBack();
            }));
      else
         AddItem(std::make_unique<InGameUIItem>("Cancel"s, "Get back"s,
            [this]()
            {
               m_player->m_liveUI->m_inGameUI.NavigateBack();
            }));
   };

   InGameUIPage* m_page;
};

void InGameUIPage::Save()
{
   if (!IsModified())
      return;
   const bool canSaveTableOverrides = FileExists(m_player->m_ptable->m_filename);
   switch (m_saveMode)
   {
   case SaveMode::None: break;

   case SaveMode::Both:
      m_player->m_liveUI->m_inGameUI.AddPage("popup/save_select"s, [this, canSaveTableOverrides]() { return std::make_unique<SelectGlobalOrOverridePage>(this, canSaveTableOverrides); });
      m_player->m_liveUI->m_inGameUI.Navigate("popup/save_select"s);
      break;

   case SaveMode::Global: SaveGlobally(); break;

   case SaveMode::Table:
      if (!canSaveTableOverrides)
         m_player->m_liveUI->PushNotification("You need to save the table before saving table setting overrides"s, 5000);
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
   Settings& tableSettings = m_player->m_ptable->m_settings;
   for (const auto& item : m_items)
      item->ResetSave(tableSettings);
   tableSettings.Save();
   // Then save to application settings
   Settings& appSettings = g_pvp->m_settings;
   for (const auto& item : m_items)
      item->Save(appSettings, false);
   appSettings.Save();
}

void InGameUIPage::SaveTableOverride()
{
   // First reset any table override (to start from a clear ground if saved items depends on user selection, note that some item may impact multiple settings so we save them afterward)
   Settings& tableSettings = m_player->m_ptable->m_settings;
   for (const auto& item : m_items)
      item->ResetSave(tableSettings);
   // Then save to table override
   for (const auto& item : m_items)
      item->Save(tableSettings, true);
   tableSettings.Save();
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
   case InGameUIItem::Type::Label:
      if (item->m_labelType == InGameUIItem::LabelType::Markdown)
         m_pressedItemScroll += speedFactor * direction;
      break;

   case InGameUIItem::Type::Navigation: m_player->m_liveUI->m_inGameUI.Navigate(item->m_path); break;

   case InGameUIItem::Type::Runnable: item->m_runnable(); break;

   case InGameUIItem::Type::ResetToDefaults: // Defaults (allow continuously applying as defaults may be dynamic, for example for VR, headtracking, dynamic room exposure,...)
      ResetToDefaults();
      break;

   case InGameUIItem::Type::ResetToStoredValues: // Undo (allow continuously applying, if something else if modifying the values, as we do not forbid it)
      ResetToStoredValues();
      break;

   case InGameUIItem::Type::SaveChanges:
      if (isInitialPress)
         Save();
      break;

   case InGameUIItem::Type::Back:
      if (isInitialPress)
         m_player->m_liveUI->m_inGameUI.NavigateBack();
      break;

   case InGameUIItem::Type::ActionInputMapping:
      if (isInitialPress)
      {
         if (direction < 0.f)
         {
            item->m_inputAction->ClearMapping();
            // Base navigation items must always be mapped (otherwise navigation ends up broken), so immediately start a mapping definition
            if (item->m_inputAction->IsNavigationAction())
            {
               m_defineActionPopup = true;
               m_defineActionItem = item.get();
            }
         }
         else
         {
            m_defineActionPopup = true;
            m_defineActionItem = item.get();
         }
      }
      break;

   case InGameUIItem::Type::PhysicsSensorMapping:
      if (isInitialPress)
      {
         if (direction < 0.f)
         {
            item->m_physicsSensor->ClearMapping();
         }
         else
         {
            m_player->m_liveUI->m_inGameUI.AddPage("popup/sensor_setup"s, [this, ptr = item.get()]() { return std::make_unique<SensorSetupPage>(*ptr); });
            m_player->m_liveUI->m_inGameUI.Navigate("popup/sensor_setup"s);
         }
      }
      break;

   case InGameUIItem::Type::Property:
      switch (item->m_property->m_type)
      {
      case VPX::Properties::PropertyDef::Type::String:
         // Unsupported for now
         assert(false);
         break;

      case VPX::Properties::PropertyDef::Type::Enum:
         if (isInitialPress)
         {
            const int nValues = static_cast<int>(dynamic_cast<VPX::Properties::EnumPropertyDef*>(item->m_property.get())->m_values.size());
            if (direction < 0.f)
               item->SetValue((item->GetIntValue() + nValues - 1) % nValues);
            else
               item->SetValue((item->GetIntValue() + 1) % nValues);
         }
         break;

      case VPX::Properties::PropertyDef::Type::Float:
      {
         auto prop = dynamic_cast<const VPX::Properties::FloatPropertyDef*>(item->m_property.get());
         if (isInitialPress)
            m_adjustedValue = item->GetFloatValue();
         m_adjustedValue += direction * speedFactor * elapsed * (prop->m_max - prop->m_min) / 32.f;
         item->SetValue(prop->GetSteppedClamped(m_adjustedValue));
         break;
      }

      case VPX::Properties::PropertyDef::Type::Int:
      {
         auto prop = dynamic_cast<const VPX::Properties::IntPropertyDef*>(item->m_property.get());
         if (isInitialPress)
            m_adjustedValue = static_cast<float>(item->GetIntValue()) + direction;
         else if (elapsedSincePress > 100)
            m_adjustedValue += direction * speedFactor * elapsed * static_cast<float>(prop->m_max - prop->m_min) / 32.f;
         item->SetValue(prop->GetClamped(static_cast<int>(round(m_adjustedValue))));
         break;
      }

      case VPX::Properties::PropertyDef::Type::Bool:
         if (isInitialPress)
            item->SetValue(!item->GetBoolValue());
         break;
      }
      break;

   default: break;
   }
}

void InGameUIPage::Render(float elapsedS)
{
   ImGuiIO& io = ImGui::GetIO();
   const ImGuiStyle& style = ImGui::GetStyle();

   if (m_openAnimTarget != m_openAnimPos)
   {
      m_openAnimElapsed += elapsedS;
      m_openAnimPos = lerp(m_openAnimStart, m_openAnimTarget, smoothstep(0.f, 0.5f, m_openAnimElapsed));
      if (fabsf(m_openAnimTarget - m_openAnimPos) < 0.001f)
         m_openAnimPos = m_openAnimTarget;
   }
   const float animPos = m_openAnimPos;

   ImGui::SetNextWindowBgAlpha(m_player->m_renderer->m_vrApplyColorKey ? 1.f : 0.666f);
   // Size is selected to match pinball instruction cards format which have an aspect ratio of roughly 6" x 3.25" (WPC, other varies), except for mobile where we favor size
   constexpr float pinballCardAR = 6.f / 3.5f;
   ImVec2 winSize;
   if (m_player->m_vrDevice)
   {
      winSize.x = min(0.25f * io.DisplaySize.x, 0.25f * io.DisplaySize.y);
      winSize.y = (1 / pinballCardAR) * winSize.x + 5.f * ImGui::GetTextLineHeightWithSpacing();
      ImGui::SetNextWindowPos(ImVec2((animPos + 0.5f) * io.DisplaySize.x, 0.5f * io.DisplaySize.y), 0, ImVec2(0.5f, 0.5f));
   }
#ifdef __LIBVPINBALL__
   else if (io.DisplaySize.x > io.DisplaySize.y)
   { // Landscape mode
      winSize = ImVec2(0.75f * io.DisplaySize.x, 0.8f * io.DisplaySize.y);
      ImGui::SetNextWindowPos(ImVec2((animPos + 0.5f) * io.DisplaySize.x, 0.5f * io.DisplaySize.y), 0, ImVec2(0.5f, 0.5f));
   }
   else
   { // Portrait mode
      winSize = ImVec2(0.9f * io.DisplaySize.x, 0.8f * io.DisplaySize.y);
      ImGui::SetNextWindowPos(ImVec2((animPos + 0.5f) * io.DisplaySize.x, 0.5f * io.DisplaySize.y), 0, ImVec2(0.5f, 0.5f));
   }
#else
   else if (io.DisplaySize.x > io.DisplaySize.y)
   { // Landscape mode, fit on height
      winSize.y = 0.5f * io.DisplaySize.y;
      winSize.x = pinballCardAR * (winSize.y - 5.f * ImGui::GetTextLineHeightWithSpacing());
      ImGui::SetNextWindowPos(ImVec2((animPos + 0.5f) * io.DisplaySize.x, 0.9f * io.DisplaySize.y), 0, ImVec2(0.5f, 1.f));
   }
   else
   { // Portrait mode, fit on width
      winSize.x = 0.8f * io.DisplaySize.x;
      winSize.y = (1.f / pinballCardAR) * winSize.x + 5.f * ImGui::GetTextLineHeightWithSpacing();
      ImGui::SetNextWindowPos(ImVec2((animPos + 0.5f) * io.DisplaySize.x, 0.8f * io.DisplaySize.y), 0, ImVec2(0.5f, 1.f));
   }
#endif
   ImGui::SetNextWindowSize(winSize);
   ImGui::Begin(std::to_string(reinterpret_cast<uint64_t>(this)).c_str(), nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings);

   ImGui::PushStyleColor(ImGuiCol_Separator, style.Colors[ImGuiCol_Text]);

   const bool isAdjustable = IsAdjustable();

   // Header
   std::erase_if(m_items,
      [](auto& item)
      {
         using enum InGameUIItem::Type;
         return item->m_type == SaveChanges || item->m_type == ResetToDefaults || item->m_type == ResetToStoredValues || item->m_type == Back;
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
         m_items.push_back(std::make_unique<InGameUIItem>(InGameUIItem::Type::ResetToStoredValues));
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
      if (m_saveMode == SaveMode::None)
      {
         ImGui::Dummy(ImGui::CalcTextSize(ICON_FK_FLOPPY_O, nullptr, true) + style.FramePadding * 2.0f);
         ImGui::SameLine();
      }
      else
      {
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
   float maxLabelWidth = 0.f;
   for (const auto& item : m_items)
      if (item->IsAdjustable())
         maxLabelWidth = max(maxLabelWidth, ImGui::CalcTextSize(item->m_label.c_str()).x);
   maxLabelWidth = min(maxLabelWidth, ImGui::CalcTextSize("Maximum label length before ellipsis").x);
   const float labelEndScreenX = ImGui::GetCursorScreenPos().x + maxLabelWidth + style.ItemSpacing.x * 2.0f + 30.f;
   const float itemEndScreenX = ImGui::GetCursorScreenPos().x + ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("*", nullptr, true).x - itemPadding.x;
   const float closeButtonWidth = ImGui::CalcTextSize(ICON_FK_TIMES, nullptr, true).x + style.FramePadding.x * 2.0f;
   const float circleTextWidth = ImGui::CalcTextSize(ICON_FK_CIRCLE, nullptr, true).x + style.FramePadding.x * 2.0f;
   for (int i = 0; i < (int)m_items.size(); i++)
   {
      using enum InGameUIItem::Type;
      const auto& item = m_items[i];

      // Skip as these items are rendered in the header
      if (item->m_type == Back || item->m_type == SaveChanges || item->m_type == ResetToDefaults || item->m_type == ResetToStoredValues)
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
      case Label:
      {
         float infoHeight = ImGui::GetCursorPosY();
         ImGui::SetCursorScreenPos(ImGui::GetCursorScreenPos() + ImVec2(0.f, itemPadding.y));
         ImGui::SetNextItemWidth(itemEndScreenX - ImGui::GetCursorScreenPos().x - 60.f);
         switch (item->m_labelType)
         {
         case InGameUIItem::LabelType::Info:
            ImGui::Text("%s", item->m_label.c_str());
            ImGui::SetCursorScreenPos(ImGui::GetCursorScreenPos() + ImVec2(0.f, itemPadding.y));
            break;
         case InGameUIItem::LabelType::Header:
         {
            ImGui::Text("%s", item->m_label.c_str());
            ImVec2 min = ImGui::GetItemRectMin();
            ImVec2 max = ImGui::GetItemRectMax();
            min.y = max.y;
            ImGui::GetWindowDrawList()->AddLine(min, max, IM_COL32_WHITE, 1.0f);
            ImGui::SetCursorScreenPos(ImGui::GetCursorScreenPos() + ImVec2(0.f, itemPadding.y));
            break;
         }
         case InGameUIItem::LabelType::Markdown:
         {
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
         }
         break;
      }

      case Navigation:
      case Runnable:
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

      case CustomRender:
      {
         ImGui::SetCursorScreenPos(ImGui::GetCursorScreenPos() + ImVec2(0.f, itemPadding.y));
         if (item->m_customRender)
         {
            item->m_customRender(i, item.get());
         }
         ImGui::SetCursorScreenPos(ImGui::GetCursorScreenPos() + ImVec2(0.f, itemPadding.y));
         break;
      }

      case ActionInputMapping:
      {
         ImGui::Text("%s", item->m_label.c_str());
         if (item->m_inputAction->IsMapped())
         {
            ImGui::SameLine(labelEndScreenX - ImGui::GetCursorScreenPos().x);
            if (ImGui::Button((ICON_FK_TIMES + "##"s + item->m_label).c_str(), ImVec2(closeButtonWidth, 0)))
            {
               item->m_inputAction->ClearMapping();
               if (item->m_inputAction->IsNavigationAction())
               {
                  m_defineActionPopup = true;
                  m_defineActionItem = item.get();
               }
            }
            ImGui::SameLine();
         }
         else
         {
            ImGui::SameLine(labelEndScreenX - ImGui::GetCursorScreenPos().x + closeButtonWidth + style.ItemSpacing.x);
         }
         const string mappingLabel = item->m_inputAction->GetMappingLabel();
         const float mapButtonWidth = itemEndScreenX - ImGui::GetCursorScreenPos().x - circleTextWidth - style.ItemSpacing.x;
         if (ImGui::Button((mappingLabel + "##" + item->m_label).c_str(), ImVec2(mapButtonWidth, 0)))
         {
            m_defineActionPopup = true;
            m_defineActionItem = item.get();
         }
         if (ImGui::CalcTextSize(mappingLabel.c_str()).x >= mapButtonWidth - style.ItemSpacing.x * 2.f)
            ImGui::SetItemTooltip("%s", mappingLabel.c_str());
         {
            string state = item->m_inputAction->IsPressed() ? ICON_FK_CIRCLE : ICON_FK_CIRCLE_O;
            ImGui::SameLine(itemEndScreenX - ImGui::GetCursorScreenPos().x - circleTextWidth);
            ImGui::Text("%s", state.c_str());
         }
         if (item->IsModified())
         {
            ImGui::SameLine(itemEndScreenX - ImGui::GetCursorScreenPos().x);
            ImGui::Text(ICON_FK_PENCIL);
         }
         break;
      }

      case PhysicsSensorMapping:
      {
         ImGui::Text("%s", item->m_label.c_str());
         if (!item->m_physicsSensor->IsMapped())
            ImGui::SameLine(labelEndScreenX - ImGui::GetCursorScreenPos().x + closeButtonWidth + style.ItemSpacing.x);
         else
         {
            ImGui::SameLine(labelEndScreenX - ImGui::GetCursorScreenPos().x);
            if (ImGui::Button((ICON_FK_TIMES + "##"s + item->m_label).c_str(), ImVec2(closeButtonWidth, 0)))
               item->m_physicsSensor->ClearMapping();
            ImGui::SameLine();
         }
         const string mappingLabel = item->m_physicsSensor->GetMappingLabel();
         const float mapButtonWidth = itemEndScreenX - ImGui::GetCursorScreenPos().x - closeButtonWidth;
         if (ImGui::Button((mappingLabel + "##" + item->m_label).c_str(), ImVec2(mapButtonWidth, 0)))
         {
            m_selectedItem = i;
            AdjustItem(1.f, true);
         }
         if (ImGui::CalcTextSize(mappingLabel.c_str()).x >= mapButtonWidth - style.ItemSpacing.x * 2.f)
            ImGui::SetItemTooltip("%s", mappingLabel.c_str());
         break;
      }

      case Property:
         switch (item->m_property->m_type)
         {
         case VPX::Properties::PropertyDef::Type::Float:
         {
            auto prop = dynamic_cast<VPX::Properties::FloatPropertyDef*>(item->m_property.get());
            ImGui::Text("%s", prop->m_label.c_str());
            ImGui::SameLine(labelEndScreenX - ImGui::GetCursorScreenPos().x);
            float v = item->GetFloatValue() * item->m_floatValueDisplayScale;
            ImGui::SetNextItemWidth(itemEndScreenX - ImGui::GetCursorScreenPos().x);
            ImGui::SliderFloat(("##" + prop->m_label).c_str(), &v, prop->m_min * item->m_floatValueDisplayScale, prop->m_max * item->m_floatValueDisplayScale, item->m_format.c_str(),
               ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_NoRoundToFormat);
            if (item->IsModified())
            {
               ImGui::SameLine(itemEndScreenX - ImGui::GetCursorScreenPos().x);
               ImGui::Text(ICON_FK_PENCIL);
            }
            else if (auto id = Settings::GetRegistry().GetPropertyId(item->m_property->m_groupId, item->m_property->m_propId);
               id.has_value() && g_pvp->m_settings.GetFloat(id.value()) != m_player->m_ptable->m_settings.GetFloat(id.value()))
            {
               ImGui::SameLine(itemEndScreenX - ImGui::GetCursorScreenPos().x);
               ImGui::Text(ICON_FK_DOT_CIRCLE_O);
            }
            item->SetValue(v / item->m_floatValueDisplayScale);
            break;
         }

         case VPX::Properties::PropertyDef::Type::Int:
         {
            auto prop = dynamic_cast<VPX::Properties::IntPropertyDef*>(item->m_property.get());
            TextWithEllipsis(prop->m_label, maxLabelWidth);
            ImGui::SameLine(labelEndScreenX - ImGui::GetCursorScreenPos().x);
            int v = item->GetIntValue();
            const auto id = Settings::GetRegistry().GetPropertyId(item->m_property->m_groupId, item->m_property->m_propId);
            if (((Settings::m_propPlayer_PlayfieldWidth.index == id.value().index) || (Settings::m_propPlayer_PlayfieldHeight.index == id.value().index)) && !m_player->m_liveUI->m_inGameUI.IsFlipperNav())
            {
               // Special handling for editing main window size as mouse interaction would break (since the control is moved/resized while interacted)
               const float butWidth = ImGui::CalcTextSize(ICON_FK_ARROWS_H, nullptr, true).x + ImGui::GetStyle().FramePadding.x * 2.0f;
               ImGui::SetNextItemWidth(itemEndScreenX - ImGui::GetCursorScreenPos().x - ImGui::GetStyle().ItemSpacing.x - butWidth);
               ImGui::InputInt(("##" + prop->m_label).c_str(), &v, 1, 100, ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_CharsNoBlank);
               ImGui::SameLine();
               if (ImGui::Button((Settings::m_propPlayer_PlayfieldWidth.index == id.value().index) ? ICON_FK_ARROWS_H : ICON_FK_ARROWS_V))
                  v = prop->m_max;
            }
            else
            {
               ImGui::SetNextItemWidth(itemEndScreenX - ImGui::GetCursorScreenPos().x);
               ImGui::SliderInt(("##" + prop->m_label).c_str(), &v, prop->m_min, prop->m_max, item->m_format.c_str(), ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_NoRoundToFormat);
            }
            if (item->IsModified())
            {
               ImGui::SameLine(itemEndScreenX - ImGui::GetCursorScreenPos().x);
               ImGui::Text(ICON_FK_PENCIL);
            }
            else if (id.has_value() && g_pvp->m_settings.GetInt(id.value()) != m_player->m_ptable->m_settings.GetInt(id.value()))
            {
               ImGui::SameLine(itemEndScreenX - ImGui::GetCursorScreenPos().x);
               ImGui::Text(ICON_FK_DOT_CIRCLE_O);
            }
            item->SetValue(v);
            break;
         }

         case VPX::Properties::PropertyDef::Type::Enum:
         {
            auto prop = dynamic_cast<VPX::Properties::EnumPropertyDef*>(item->m_property.get());
            TextWithEllipsis(prop->m_label, maxLabelWidth);
            ImGui::SameLine(labelEndScreenX - ImGui::GetCursorScreenPos().x);
            int v = item->GetIntValue() - prop->m_min;
            ImGui::SetNextItemWidth(itemEndScreenX - ImGui::GetCursorScreenPos().x);
            ImGui::Combo(("##" + prop->m_label).c_str(), &v,
               [](void* data, int idx)
               {
                  const auto* vec = static_cast<const vector<string>*>(data);
                  if (idx < 0 || idx >= (int)vec->size())
                     return "";
                  return vec->at(idx).c_str();
               },
               (void*)&prop->m_values, (int)prop->m_values.size());
            if (item->IsModified())
            {
               ImGui::SameLine(itemEndScreenX - ImGui::GetCursorScreenPos().x);
               ImGui::Text(ICON_FK_PENCIL);
            }
            else if (auto id = Settings::GetRegistry().GetPropertyId(item->m_property->m_groupId, item->m_property->m_propId);
               id.has_value() && g_pvp->m_settings.GetInt(id.value()) != m_player->m_ptable->m_settings.GetInt(id.value()))
            {
               ImGui::SameLine(itemEndScreenX - ImGui::GetCursorScreenPos().x);
               ImGui::Text(ICON_FK_DOT_CIRCLE_O);
            }
            item->SetValue(prop->m_min + v);
            break;
         }

         case VPX::Properties::PropertyDef::Type::Bool:
         {
            auto prop = dynamic_cast<VPX::Properties::BoolPropertyDef*>(item->m_property.get());
            ImGui::Text("%s", prop->m_label.c_str());
            ImGui::SameLine(labelEndScreenX - ImGui::GetCursorScreenPos().x);
            bool v = item->GetBoolValue();
            RenderToggle(prop->m_label, ImVec2(itemEndScreenX - ImGui::GetCursorScreenPos().x, ImGui::GetFrameHeight()), v);
            if (item->IsModified())
            {
               ImGui::SameLine(itemEndScreenX - ImGui::GetCursorScreenPos().x);
               ImGui::Text(ICON_FK_PENCIL);
            }
            else if (auto id = Settings::GetRegistry().GetPropertyId(item->m_property->m_groupId, item->m_property->m_propId);
               id.has_value() && g_pvp->m_settings.GetBool(id.value()) != m_player->m_ptable->m_settings.GetBool(id.value()))
            {
               ImGui::SameLine(itemEndScreenX - ImGui::GetCursorScreenPos().x);
               ImGui::Text(ICON_FK_DOT_CIRCLE_O);
            }
            item->SetValue(v);
            break;
         }

         default: assert(false); break;
         }
         break;

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

   m_windowPos = ImGui::GetWindowPos();
   m_windowSize = ImGui::GetWindowSize();
   m_windowHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);

   ImGui::End();

   RenderInputActionPopup();
}

void InGameUIPage::RenderInputActionPopup()
{
   if (!m_defineActionPopup && m_defineActionItem)
   {
      assert(false); // Not supposed to happen as the only way to close the popup is to actually define a mapping which deselects the item
      if (m_defineActionItem->m_inputAction->IsNavigationAction() && !m_defineActionItem->m_inputAction->IsMapped())
         m_defineActionPopup = true;
      else
         m_defineActionItem = nullptr;
   }
   if (m_defineActionItem && m_defineActionPopup && !ImGui::IsPopupOpen((m_defineActionItem->m_label + " input binding").c_str()))
   {
      ImGui::OpenPopup((m_defineActionItem->m_label + " input binding").c_str());
      ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
      ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoKeyboard;
      m_player->m_pininput.StartButtonCapture();
   }
   if (m_defineActionItem && ImGui::BeginPopupModal((m_defineActionItem->m_label + " input binding").c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
   {
      ImGui::Text("Press the key/button or combination of keys/buttons\nyou want to use for the action '%s'", m_defineActionItem->m_label.c_str());
      ImGui::Spacing();
      ImGui::Separator();
      ImGui::Spacing();
      ImGui::Text("New mapping: %s", m_player->m_pininput.GetMappingLabel(m_player->m_pininput.GetButtonCapture()).c_str());
      ImGui::Spacing();
      ImGui::Separator();
      if (m_player->m_pininput.IsButtonCaptureDone())
      {
         ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
         ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoKeyboard;
         m_defineActionItem->m_inputAction->AddMapping(m_player->m_pininput.GetButtonCapture());
         if (!(m_defineActionItem->m_inputAction->IsNavigationAction() && !m_defineActionItem->m_inputAction->IsMapped()))
         {
            ImGui::CloseCurrentPopup();
            m_defineActionItem = nullptr;
         }
      }
      ImGui::EndPopup();
   }
}

// Basic Toggle button, based on https://github.com/ocornut/imgui/issues/1537#issuecomment-355562097
void InGameUIPage::RenderToggle(const string& label, const ImVec2& size, bool& v)
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
   ImU32 col_bg;
   if (ImGui::IsItemHovered())
      col_bg = v ? IM_COL32(64 + 20, 180, 32 + 20, 255) : IM_COL32(64, 64, 64, 255);
   else
      col_bg = v ? IM_COL32(64, 180, 32, 255) : IM_COL32(37, 37, 37, 255);

   draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), col_bg, height * 0.5f);
   draw_list->AddCircleFilled(ImVec2(v ? (p.x + width - radius) : (p.x + radius), p.y + radius), radius - 3.5f, IM_COL32(255, 255, 255, 255));
}

void InGameUIPage::TextWithEllipsis(const string& textStr, float maxWidth)
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

}
