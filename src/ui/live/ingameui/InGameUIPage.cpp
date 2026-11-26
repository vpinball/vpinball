// license:GPLv3+

#include "core/stdafx.h"
#include "InGameUIPage.h"

#include "core/VPApp.h"
#include "ui/live/IconsLucide.h"
#include "ui/live/ImGuiHelpers.h"
#include "ui/live/LiveUI.h"
#include "ui/live/FileDialog.h"
#include "ui/live/ingameui/SensorSetupPage.h"

#include "imgui/imgui_internal.h"
#include "imgui/imgui_stdlib.h"

namespace VPX::InGameUI
{

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
   m_pressedItemLabel.clear();
   RequestRebuild();
}

void InGameUIPage::Close(bool isBackwardAnimation)
{
   m_openAnimElapsed = 0.f;
   m_openAnimTarget = isBackwardAnimation ? 1.f : -1.f;
   m_openAnimStart = m_openAnimPos;
}

void InGameUIPage::ClearItems()
{
   assert(m_isBuildingPage);
   m_items.clear();
}

InGameUIItem& InGameUIPage::AddItem(std::unique_ptr<InGameUIItem> item)
{
   assert(m_isBuildingPage);
   m_items.push_back(std::move(item));
   return *m_items.back();
}

Settings& InGameUIPage::GetSettings() { return g_settingsService.GetActiveSettings(); }

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
      , m_canSaveTableOverrides(canSaveTableOverrides)
   {
   }

private:
   void BuildPage()
   {
      AddItem(std::make_unique<InGameUIItem>("Save Globally"s, "Your changes will be saved as the new default for all tables."s,
         [this]()
         {
            m_page->SaveGlobally();
            m_player->m_liveUI->m_inGameUI.NavigateBack();
         }));
      if (m_canSaveTableOverrides)
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
   bool m_canSaveTableOverrides;
};

void InGameUIPage::Save()
{
   if (!IsModified())
      return;
   const bool canSaveTableOverrides = m_player->m_ptable->IsOnDisk();
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
   Settings& tableSettings = g_settingsService.GetActiveSettings();
   for (const auto& item : m_items)
      item->ResetSave(tableSettings);
   // Then save to application settings (except items which are defined as table only)
   Settings& appSettings = g_settingsService.GetAppSettings();
   for (const auto& item : m_items)
      item->Save(item->m_tableOnly ? tableSettings : appSettings, item->m_tableOnly);
   tableSettings.Save();
   appSettings.Save();
   m_savedNotifId = m_player->m_liveUI->PushNotification("Changes saved to the global application settings"s, 5000, m_savedNotifId);
}

void InGameUIPage::SaveTableOverride()
{
   // First reset any table override (to start from a clear ground if saved items depends on user selection, note that some item may impact multiple settings so we save them afterward)
   Settings& tableSettings = g_settingsService.GetActiveSettings();
   for (const auto& item : m_items)
      item->ResetSave(tableSettings);
   // Then save to table override (except items which are defined as global only)
   Settings& appSettings = g_settingsService.GetAppSettings();
   for (const auto& item : m_items)
      item->Save(item->m_globalOnly ? appSettings : tableSettings, !item->m_globalOnly);
   tableSettings.Save();
   appSettings.Save();
   m_savedNotifId = m_player->m_liveUI->PushNotification("Changes saved as a table override"s, 5000, m_savedNotifId);
}

void InGameUIPage::NotifySavedForNextStart()
{
   m_savedForNextStartNotifId
      = m_player->m_liveUI->PushNotification("This change is automatically saved and will be applied after restarting the player."s, 5000, m_savedForNextStartNotifId);
}

void InGameUIPage::NotifyApplyOnNextStart()
{
   m_savedForNextStartNotifId = m_player->m_liveUI->PushNotification("This change will be applied after restarting the player."s, 5000, m_savedForNextStartNotifId);
}

bool InGameUIPage::HasSelectableItem() const
{
   return std::ranges::any_of(m_items, [](const auto &item) { return item->IsSelectable(); });
}

void InGameUIPage::SelectNextItem()
{
   if (!HasSelectableItem())
      return;
   m_pressedItemScroll = 0.f; // Start from top of item
   const int nItems = static_cast<int>(m_items.size());
   do
      m_selectedItem = (m_selectedItem + 1) % nItems;
   while (!m_items[m_selectedItem]->IsSelectable());
}

void InGameUIPage::SelectPrevItem()
{
   if (!HasSelectableItem())
      return;
   m_pressedItemScroll = 10000.f; // Start from bottom of item
   const int nItems = static_cast<int>(m_items.size());
   do
      m_selectedItem = (m_selectedItem + nItems - 1) % nItems;
   while (!m_items[m_selectedItem]->IsSelectable());
}

// Adjustment grid of a float property item: 'step' is the discrete step applied on initial press and snapped to on release,
// 'scrollStep' is the base speed step of continuous adjustment (holding the button).
struct FloatAdjustGrid
{
   float anchor;
   float step;
   float scrollStep;
};

static FloatAdjustGrid GetFloatAdjustGrid(const InGameUIItem& item)
{
   const auto prop = static_cast<const VPX::Properties::FloatPropertyDef*>(item.m_property.get());
   if (prop->m_step != 0.f)
      return { prop->m_min, prop->m_step, prop->m_step };
   const float displayScale = fabsf(item.m_floatValueDisplayScale) != 0.f ? fabsf(item.m_floatValueDisplayScale) : 1.f;
   // 'Nice' 1-2-5 step fitting 1/64th of the range, used as scroll speed base and as a fallback step
   float rangeStep = 1.f;
   if (const float displayRange = (prop->m_max - prop->m_min) * displayScale; displayRange > 0.f && displayRange < FLT_MAX)
   {
      const float s = displayRange / 64.f;
      const float pow10 = powf(10.f, floorf(log10f(s)));
      rangeStep = ((5.f * pow10 <= s) ? 5.f * pow10 : (2.f * pow10 <= s) ? 2.f * pow10 : pow10) / displayScale;
   }
   // Precision of the displayed value, e.g. 0.1 for a '%4.1f' format
   float displayStep = 0.f;
   if (const size_t dotPos = item.m_format.find('.');
      dotPos != string::npos && dotPos + 1 < item.m_format.size() && item.m_format[dotPos + 1] >= '0' && item.m_format[dotPos + 1] <= '9')
      displayStep = powf(10.f, static_cast<float>('0' - item.m_format[dotPos + 1])) / displayScale;
   return { 0.f, displayStep != 0.f ? displayStep : rangeStep, rangeStep };
}

void InGameUIPage::AdjustItem(float direction, bool isInitialPress)
{
   if (m_selectedItem < 0 || m_selectedItem >= (int)m_items.size())
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
         }
         else
         {
            RequestInputActionPopup(*item);
         }
      }
      break;

   case InGameUIItem::Type::Property:
      switch (item->m_property->m_type)
      {
      case VPX::Properties::PropertyDef::Type::String:
         if (isInitialPress && dynamic_cast<VPX::Properties::StringPropertyDef*>(item->m_property.get())->m_pathType != VPX::Properties::StringPropertyDef::PathType::None)
            BrowseForPath(*item);
         break;

      case VPX::Properties::PropertyDef::Type::Enum:
         if (isInitialPress)
         {
            const auto prop = dynamic_cast<VPX::Properties::EnumPropertyDef*>(item->m_property.get());
            const int nValues = static_cast<int>(prop->m_values.size());
            const int index = std::clamp(item->GetIntValue() - prop->m_min, 0, nValues - 1); // Clamped, as a stored value may be out of range
            if (direction < 0.f)
               item->SetValue(prop->m_min + (index + nValues - 1) % nValues);
            else
               item->SetValue(prop->m_min + (index + 1) % nValues);
         }
         break;

      case VPX::Properties::PropertyDef::Type::Float:
      {
         auto prop = dynamic_cast<const VPX::Properties::FloatPropertyDef*>(item->m_property.get());
         // Adjust on a step grid: the one defined by the property if any, otherwise the displayed precision (or a 'nice' step matching the displayed range when unknown)
         const FloatAdjustGrid grid = GetFloatAdjustGrid(*item);
         if (isInitialPress)
         {
            // Move to the next step in the pressed direction (full step when already on the grid, else closest grid position in this direction)
            const float pos = (item->GetFloatValue() - grid.anchor) / grid.step;
            const float gridPos = roundf(pos);
            m_adjustedValue = grid.anchor + (fabsf(pos - gridPos) < 0.001f ? gridPos + direction : (direction < 0.f ? floorf(pos) : ceilf(pos))) * grid.step;
         }
         else if (elapsedSincePress > 150)
            // Continuous adjustment is not snapped to the step grid to keep the feedback smooth (it is snapped back on release)
            m_adjustedValue += direction * speedFactor * elapsed * grid.scrollStep;
         item->SetValue(prop->GetSteppedClamped(m_adjustedValue));
         break;
      }

      case VPX::Properties::PropertyDef::Type::Int:
      {
         auto prop = dynamic_cast<const VPX::Properties::IntPropertyDef*>(item->m_property.get());
         if (isInitialPress)
            m_adjustedValue = static_cast<float>(item->GetIntValue()) + direction;
         else if (elapsedSincePress > 150)
            m_adjustedValue += direction * speedFactor * elapsed * static_cast<float>(prop->m_max - prop->m_min) / 64.f;
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

void InGameUIPage::ReleaseAdjustedItem()
{
   if (m_pressedItemLabel.empty())
      return;
   const string pressedItemLabel = m_pressedItemLabel;
   m_pressedItemLabel.clear();
   // Continuous adjustment is not snapped to the value grid to keep the feedback smooth: snap to the closest step on release
   for (const auto& item : m_items)
   {
      if (item->m_label != pressedItemLabel || item->m_type != InGameUIItem::Type::Property || item->m_property->m_type != VPX::Properties::PropertyDef::Type::Float)
         continue;
      const auto prop = dynamic_cast<const VPX::Properties::FloatPropertyDef*>(item->m_property.get());
      const FloatAdjustGrid grid = GetFloatAdjustGrid(*item);
      item->SetValue(prop->GetSteppedClamped(grid.anchor + roundf((item->GetFloatValue() - grid.anchor) / grid.step) * grid.step));
      break;
   }
}

void InGameUIPage::Update(float elapsedS)
{
   // Rebuild requests made while a slider is being dragged are held until it is released. A rebuild
   // stops the frame at the item that requested it (see the renderer's item loop), so the page is
   // drawn shorter: its scrollbar disappears, the content area gets wider and the slider's scale
   // with it, and the same pointer position now maps to a slightly different value. That value
   // change requests another rebuild on the next, full frame, and the view oscillates between the
   // two values for as long as the slider is held (very visible on the point of view page where
   // each change moves the camera).
   if (m_rebuildPending && !ImGui::IsAnyItemActive())
   {
      m_rebuildPending = false;
      m_needsRebuild = true;
   }

   if (m_needsRebuild)
   {
      // Pages are always built here, before rendering and interaction handling, to avoid item lifecycle issues (for example when changing a setting may result in other settings being added/removed).
      assert(!m_isBuildingPage);
      m_needsRebuild = false;
      m_isBuildingPage = true;
      ClearItems();
      BuildPage();
      m_isBuildingPage = false;
   }

   // Apply the result of an asynchronous folder/file selection dialog to the item it was opened for (as the page may have been rebuilt in between, look it up by property id)
   if (m_pathSelectResult && !m_pathSelectResult->empty())
   {
      const string path = PathToString(PathFromUTF8(*m_pathSelectResult));
      for (const auto& item : m_items)
         if (item->m_property && item->m_property->m_groupId == m_pathSelectGroupId && item->m_property->m_propId == m_pathSelectPropId)
            item->SetValue(path);
      m_pathSelectResult = nullptr;
   }

   // Append the core action items: they are rendered in the header by the renderer but kept in the
   // item list so that selection and adjustment treat them like regular items
   std::erase_if(m_items,
      [](auto& item)
      {
         using enum InGameUIItem::Type;
         return item->m_type == SaveChanges || item->m_type == ResetToDefaults || item->m_type == ResetToStoredValues || item->m_type == Back;
      });
   if (IsAdjustable())
   {
      if (!IsDefaults())
         m_items.push_back(std::make_unique<InGameUIItem>(InGameUIItem::Type::ResetToDefaults));
      if (IsModified())
         m_items.push_back(std::make_unique<InGameUIItem>(InGameUIItem::Type::ResetToStoredValues));
      if (m_saveMode != SaveMode::None && IsModified())
         m_items.push_back(std::make_unique<InGameUIItem>(InGameUIItem::Type::SaveChanges));
   }
   if (!m_player->m_isLoading || IsBackAllowedWhileLoading())
      m_items.push_back(std::make_unique<InGameUIItem>(InGameUIItem::Type::Back));

   // As we may have changed the number of selectable items, ensure m_selectedItem is still valid and pointing to a selectable item
   if (!HasSelectableItem())
      m_selectedItem = -1; // Nothing to select (AdjustItem checks the range)
   else
   {
      m_selectedItem = clamp(m_selectedItem, 0, static_cast<int>(m_items.size()) - 1);
      while (!m_items[m_selectedItem]->IsSelectable())
         m_selectedItem = (m_selectedItem + 1) % static_cast<int>(m_items.size());
   }

   if (m_openAnimTarget != m_openAnimPos)
   {
      m_openAnimElapsed += elapsedS;
      m_openAnimPos = lerp(m_openAnimStart, m_openAnimTarget, smoothstep(0.f, 0.5f, m_openAnimElapsed));
      if (fabsf(m_openAnimTarget - m_openAnimPos) < 0.001f)
         m_openAnimPos = m_openAnimTarget;
   }
}

void InGameUIPage::BrowseForPath(const InGameUIItem& item)
{
   if (m_player->m_playfieldWnd == nullptr)
      return;
   const auto prop = dynamic_cast<const VPX::Properties::StringPropertyDef*>(item.m_property.get());
   if (prop == nullptr)
      return;
   m_pathSelectResult = std::make_shared<string>();
   m_pathSelectGroupId = prop->m_groupId;
   m_pathSelectPropId = prop->m_propId;
   const auto callback = [](void* userdata, const char* const* filelist, int filter)
   {
      auto* res = static_cast<std::shared_ptr<string>*>(userdata);
      if (filelist != nullptr && filelist[0] != nullptr)
         **res = filelist[0];
      delete res;
   };
   auto* result = new std::shared_ptr<string>(m_pathSelectResult);
   if (prop->m_pathType == VPX::Properties::StringPropertyDef::PathType::File)
   {
      // For file dialogs, the default location must be a folder
      const string location = PathToUTF8(PathFromString(item.GetStringValue()).parent_path()); // SDL expects UTF-8
      const SDL_DialogFileFilter filters[] = { { prop->m_label.c_str(), prop->m_fileFilter.c_str() } };
      ShowOpenFileDialog(callback, result, m_player->m_playfieldWnd->GetCore(), prop->m_fileFilter.empty() ? nullptr : filters, prop->m_fileFilter.empty() ? 0 : 1,
         location.empty() ? nullptr : location.c_str(), false);
   }
   else
   {
      const string location = PathToUTF8(PathFromString(item.GetStringValue())); // SDL expects UTF-8
      ShowOpenFolderDialog(callback, result, m_player->m_playfieldWnd->GetCore(), location.empty() ? nullptr : location.c_str(), false);
   }
}

}
