// license:GPLv3+

#include "core/stdafx.h"
#include "InGameUIDefaultRenderer.h"

#include "InGameUI.h"
#include "InGameUIDesktopRenderer.h"
#include "InGameUIPage.h"
#include "InGameUIWidgets.h"

#include "core/player.h"
#include "core/VPApp.h"
#include "ui/live/IconsLucide.h"
#include "ui/live/ImGuiHelpers.h"
#include "ui/live/LiveUI.h"

#include "imgui/imgui_internal.h"
#include "imgui/imgui_stdlib.h"

namespace VPX::InGameUI
{

std::unique_ptr<InGameUIRenderer> InGameUIRenderer::Create(InGameUI& owner)
{
   // The desktop renderer covers all non-touch targets, adapting its layout to
   // the playfield frame (landscape, portrait FSS, portrait cabinet inset). VR
   // gets it too: the UI quad is a 16:9 surface, so the landscape
   // layout applies there as well, including on mobile VR devices. Mobile
   // touch outside VR keeps the pinball card layout.
   if (!g_isMobile || g_pplayer->IsVR())
      return std::make_unique<InGameUIDesktopRenderer>(owner);
   return std::make_unique<InGameUIDefaultRenderer>(owner);
}

InGameUIDefaultRenderer::InGameUIDefaultRenderer(InGameUI& owner)
   : m_inGameUI(owner)
   , m_player(g_pplayer)
{
}

void InGameUIDefaultRenderer::RenderPages(const vector<InGameUIPage*>& pages, float elapsedS)
{
   for (InGameUIPage* page : pages)
      RenderPage(*page, elapsedS);
}

void InGameUIDefaultRenderer::RenderPage(InGameUIPage& page, float elapsedS)
{
   vector<InGameUIPane> panes;
   page.CollectPanes(panes);

   const ImVec2 displaySize = ImGui::GetIO().DisplaySize;

   // A modal pane replaces the whole page
   for (const InGameUIPane& pane : panes)
   {
      if (pane.placement == InGameUIPane::Placement::Modal && pane.render)
      {
         pane.render(ImVec4(0.f, 0.f, displaySize.x, displaySize.y));
         return;
      }
   }

   // Backdrop panes are drawn first, behind the page window
   for (const InGameUIPane& pane : panes)
   {
      if (pane.placement == InGameUIPane::Placement::Backdrop && pane.render)
         pane.render(ImVec4(0.f, 0.f, displaySize.x, displaySize.y));
   }

   ImGuiIO& io = ImGui::GetIO();
   const ImGuiStyle& style = ImGui::GetStyle();

   const float animPos = page.GetOpenCloseAnimPos();

   ImGui::SetNextWindowBgAlpha(m_player->IsVR() ? 1.f : 0.666f);
   // Size is selected to match pinball instruction cards format which have an aspect ratio of roughly 6" x 3.25" (WPC, other varies), except for mobile where we favor size
   constexpr float pinballCardAR = 6.f / 3.5f;
   ImVec2 winSize;
   if (m_player->m_vrDevice)
   {
      winSize.x = min(0.25f * io.DisplaySize.x, 0.25f * io.DisplaySize.y);
      winSize.y = (1 / pinballCardAR) * winSize.x + 5.f * ImGui::GetTextLineHeightWithSpacing();
      ImGui::SetNextWindowPos(ImVec2((animPos + 0.5f) * io.DisplaySize.x, 0.5f * io.DisplaySize.y), 0, ImVec2(0.5f, 0.5f));
   }
   else if (g_isMobile)
   {
      if (io.DisplaySize.x > io.DisplaySize.y)
      { // Landscape mode
         winSize = ImVec2(0.75f * io.DisplaySize.x, 0.8f * io.DisplaySize.y);
         ImGui::SetNextWindowPos(ImVec2((animPos + 0.5f) * io.DisplaySize.x, 0.5f * io.DisplaySize.y), 0, ImVec2(0.5f, 0.5f));
      }
      else
      { // Portrait mode
         winSize = ImVec2(0.9f * io.DisplaySize.x, 0.8f * io.DisplaySize.y);
         ImGui::SetNextWindowPos(ImVec2((animPos + 0.5f) * io.DisplaySize.x, 0.5f * io.DisplaySize.y), 0, ImVec2(0.5f, 0.5f));
      }
   }
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
   ImGui::SetNextWindowSize(winSize);
   ImGui::Begin(std::to_string(reinterpret_cast<size_t>(&page)).c_str(), nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings);

   ImGui::PushStyleColor(ImGuiCol_Separator, style.Colors[ImGuiCol_Text]);

   const bool isAdjustable = page.IsAdjustable();
   const ImU32 hoveredBackgroundColor = IM_COL32(0, 0, 128, 50);
   const ImU32 hoveredBorderColor = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
   //const ImU32 hoveredBorderColor = ImGui::GetColorU32(ImVec4(0.228f, 0.750f, 1.0f, 1.0f));
   const ImU32 hoveredButtonColor = ImGui::GetColorU32(ImGuiCol_ButtonHovered);

   const auto& items = page.GetItems();
   // Indices of the core action items, appended at the tail of the item list by InGameUIPage::Update
   int resetDefaultsIndex = -1;
   int resetStoredIndex = -1;
   int saveChangesIndex = -1;
   int backIndex = -1;
   for (int i = 0; i < (int)items.size(); i++)
   {
      using enum InGameUIItem::Type;
      switch (items[i]->m_type)
      {
      case ResetToDefaults: resetDefaultsIndex = i; break;
      case ResetToStoredValues: resetStoredIndex = i; break;
      case SaveChanges: saveChangesIndex = i; break;
      case Back: backIndex = i; break;
      default: break;
      }
   }

   // Header
   ImGui::SeparatorText(page.GetTitle().c_str());
   bool undoHovered = false;
   bool defaultHovered = false;
   bool saveHovered = false;
   float buttonWidth = IconButtonInkSize(ICON_LC_REPLY).x + style.ItemSpacing.x;
   if (isAdjustable)
   {
      buttonWidth += IconButtonInkSize(ICON_LC_HEART).x;
      buttonWidth += style.ItemSpacing.x;
      buttonWidth += IconButtonInkSize(ICON_LC_UNDO_2).x;
      buttonWidth += style.ItemSpacing.x;
      buttonWidth += IconButtonInkSize(ICON_LC_SAVE).x;
      buttonWidth += style.ItemSpacing.x;
   }
   ImGui::SameLine(ImGui::GetWindowSize().x - buttonWidth);
   if (isAdjustable)
   {
      bool highlighted = false;
      // Reset to defaults
      if (resetDefaultsIndex >= 0)
      {
         ImGui::BeginDisabled(false);
         highlighted = m_inGameUI.IsFlipperNav() && (page.GetSelectedItemIndex() == resetDefaultsIndex);
      }
      else
      {
         ImGui::BeginDisabled(true);
         highlighted = false;
      }
      if (highlighted)
         ImGui::PushStyleColor(ImGuiCol_Button, hoveredButtonColor);
      if (IconButtonInk(ICON_LC_HEART, "##ResetDefaults"))
      {
         page.SelectItem(resetDefaultsIndex);
         page.AdjustItem(1.f, true);
      }
      if (highlighted)
         ImGui::PopStyleColor();
      defaultHovered = ImGui::IsItemHovered();
      ImGui::EndDisabled();

      // Undo changes
      ImGui::SameLine();
      if (resetStoredIndex >= 0)
      {
         ImGui::BeginDisabled(false);
         highlighted = m_inGameUI.IsFlipperNav() && (page.GetSelectedItemIndex() == resetStoredIndex);
      }
      else
      {
         ImGui::BeginDisabled(true);
         highlighted = false;
      }
      if (highlighted)
         ImGui::PushStyleColor(ImGuiCol_Button, hoveredButtonColor);
      if (IconButtonInk(ICON_LC_UNDO_2, "##UndoChanges"))
      {
         page.SelectItem(resetStoredIndex);
         page.AdjustItem(1.f, true);
      }
      if (highlighted)
         ImGui::PopStyleColor();
      undoHovered = ImGui::IsItemHovered();
      ImGui::EndDisabled();

      // Save changes
      ImGui::SameLine();
      if (page.GetSaveMode() == InGameUIPage::SaveMode::None)
      {
         ImGui::Dummy(IconButtonInkSize(ICON_LC_SAVE));
         ImGui::SameLine();
      }
      else
      {
         if (saveChangesIndex >= 0)
         {
            ImGui::BeginDisabled(false);
            highlighted = m_inGameUI.IsFlipperNav() && (page.GetSelectedItemIndex() == saveChangesIndex);
         }
         else
         {
            ImGui::BeginDisabled(true);
            highlighted = false;
         }
         if (highlighted)
            ImGui::PushStyleColor(ImGuiCol_Button, hoveredButtonColor);
         if (IconButtonInk(ICON_LC_SAVE, "##SaveChanges"))
         {
            page.SelectItem(saveChangesIndex);
            page.AdjustItem(1.f, true);
         }
         if (highlighted)
            ImGui::PopStyleColor();
         saveHovered = ImGui::IsItemHovered();
         ImGui::EndDisabled();
         ImGui::SameLine();
      }
   }

   // Get back to previous page or to game
   const bool suppressBack = m_player->m_isLoading && !page.IsBackAllowedWhileLoading();
   if (suppressBack)
      ImGui::NewLine();
   else
   {
      const bool highlighted = m_inGameUI.IsFlipperNav() && (page.GetSelectedItemIndex() == backIndex);
      if (highlighted)
         ImGui::PushStyleColor(ImGuiCol_Button, hoveredButtonColor);
      if (IconButtonInk(ICON_LC_REPLY, "##Back"))
      {
         page.SelectItem(backIndex);
         page.AdjustItem(1.f, true);
      }
      if (highlighted)
         ImGui::PopStyleColor();
   }
   const bool backHovered = !suppressBack && ImGui::IsItemHovered();

   // Page items
   // Note that items may trigger state change which in turn may trigger a rebuild of the page (changing the item list)
   const InGameUIItem* hoveredItem = nullptr;
   const ImVec2 itemPadding = style.ItemSpacing;
   ImGui::BeginChild("PageItems", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y - ImGui::GetTextLineHeight() * 3.f - itemPadding.y * 2.f), ImGuiChildFlags_None,
      ImGuiWindowFlags_NoBackground);

   // Indent the item content so the item highlight quads (drawn from the child's left edge) have a left padding
   ImGui::Indent(itemPadding.x);

   // Header panes are drawn at the top of the item list (e.g. the input device diagram)
   for (const InGameUIPane& pane : panes)
      if (pane.placement == InGameUIPane::Placement::Header && pane.render)
      {
         const ImVec2 avail = ImGui::GetContentRegionAvail();
         const ImVec2 paneSize = pane.measure ? pane.measure(avail.x, avail.y * 0.5f) : ImVec2(avail.x, avail.y * 0.4f);
         const ImVec2 panePos = ImGui::GetCursorScreenPos();
         pane.render(ImVec4(panePos.x, panePos.y, paneSize.x, paneSize.y));
         ImGui::SetCursorPosY(ImGui::GetCursorPosY() + paneSize.y + ImGui::GetStyle().ItemSpacing.y);
      }

   // Drag scroll for easier mobile navigation
   if (g_isMobile)
   {
      constexpr float kDragThreshold = 6.f; // px before we commit to a drag
      constexpr float kFriction = 8.f; // deceleration rate (higher = quicker stop)
      constexpr float kVelocitySmooth = 0.2f; // EMA weight for velocity sampling

      if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
      {
         m_isDraggingScroll = false;
         m_dragScrollStartPos = ImGui::GetMousePos();
         m_dragScrollVelocity = 0.f;
      }

      const bool mouseDown = ImGui::IsMouseDown(ImGuiMouseButton_Left);
      if (mouseDown && !m_isDraggingScroll && fabsf(ImGui::GetMousePos().y - m_dragScrollStartPos.y) > kDragThreshold)
      {
         // Start drag after threshold is passed to avoid interfering with clicks
         m_isDraggingScroll = true;
      }
      else if (mouseDown && m_isDraggingScroll)
      {
         // Scroll by the raw finger delta and track velocity
         const float dy = io.MouseDelta.y;
         ImGui::SetScrollY(ImGui::GetScrollY() - dy);
         const float frameVel = (elapsedS > 0.f) ? (-dy / elapsedS) : 0.f;
         m_dragScrollVelocity = m_dragScrollVelocity * (1.f - kVelocitySmooth) + frameVel * kVelocitySmooth;
      }
      else if (!mouseDown && fabsf(m_dragScrollVelocity) > 0.5f)
      {
         // Inertial glide after finger lifts
         ImGui::SetScrollY(ImGui::GetScrollY() + m_dragScrollVelocity * elapsedS);
         m_dragScrollVelocity *= max(0.f, 1.f - kFriction * elapsedS);
         if (fabsf(m_dragScrollVelocity) < 0.5f)
            m_dragScrollVelocity = 0.f;
      }
   }

   float maxLabelWidth = 0.f;
   for (const auto& item : items)
      if (item->IsAdjustable())
         maxLabelWidth = max(maxLabelWidth, ImGui::CalcTextSize(item->m_label.c_str()).x);
   maxLabelWidth = min(maxLabelWidth, ImGui::CalcTextSize("Maximum label length before ellipsis").x);
   const float rowStartScreenX = ImGui::GetCursorScreenPos().x;
   const float labelEndScreenX = rowStartScreenX + maxLabelWidth + style.ItemSpacing.x * 2.0f + 30.f;
   const float itemMarkerWidth = ImMax(ImGui::CalcTextSize(ICON_LC_PENCIL).x, ImGui::CalcTextSize(ICON_LC_CIRCLE_DOT).x);
   const float itemEndScreenX = rowStartScreenX + ImGui::GetContentRegionAvail().x - itemMarkerWidth - style.ItemSpacing.x - itemPadding.x;
   // Right edge of the item fields: inset by the quad's right padding (the 'modified' pencil is kept outside, in the margin)
   const float itemFieldEndScreenX = itemEndScreenX - itemPadding.x;
   const bool stackFields = (itemFieldEndScreenX - labelEndScreenX) < 12.f * ImGui::GetFontSize();
   const float labelMaxWidth = stackFields ? itemFieldEndScreenX - rowStartScreenX : maxLabelWidth;
   const float closeButtonWidth = IconButtonInkSize(ICON_LC_X).x;
   const float circleTextWidth = IconButtonInkSize(ICON_LC_DISC).x;
   // SameLine(offset) positions relative to the window's content origin, not to the current
   // cursor: this goes to an absolute screen x on the current line
   const auto sameLineToScreenX = [](float screenX)
   {
      ImGui::SameLine(0.f, 0.f);
      ImGui::SetCursorScreenPos(ImVec2(screenX, ImGui::GetCursorScreenPos().y));
   };
   // Status icon (modified / overridden) drawn in the margin right of the item quad, its ink
   // centered on the frame-height line (a plain Text would place the Lucide glyph too high)
   const auto itemMarker = [itemEndScreenX, itemPadding](const char* icon)
   {
      ImGui::SameLine(0.f, 0.f);
      const ImVec2 linePos = ImGui::GetCursorScreenPos();
      const ImVec2 box = ImGui::CalcTextSize(icon);
      const ImVec4 ink = TextInkBounds(icon);
      const float x = itemEndScreenX + (itemPadding.x + ImGui::GetStyle().ItemSpacing.x) * 0.5f;
      ImGui::GetWindowDrawList()->AddText(
         ImVec2(x + (box.x - (ink.z - ink.x)) * 0.5f - ink.x, linePos.y + (ImGui::GetFrameHeight() - (ink.w - ink.y)) * 0.5f - ink.y), ImGui::GetColorU32(ImGuiCol_Text), icon);
      ImGui::SetCursorScreenPos(ImVec2(x, linePos.y));
      ImGui::Dummy(box);
   };
   for (int i = 0; i < (int)items.size(); i++)
   {
      // A value change handled while rendering a previous item may have requested a page rebuild (e.g.
      // switching a window output to embedded destroys the window that the following items query). The
      // rebuild is deferred to the next frame, so stop here to avoid evaluating the now-stale items
      // (their live getters would dereference freed state).
      if (page.IsRebuildRequested())
         break;

      using enum InGameUIItem::Type;
      const auto& item = items[i];

      // Skip as these items are rendered in the header
      if (item->m_type == Back || item->m_type == SaveChanges || item->m_type == ResetToDefaults || item->m_type == ResetToStoredValues)
         continue;

      const bool isStackedItem = stackFields && item->IsAdjustable() && !(item->m_type == Property && item->m_property->m_type == VPX::Properties::PropertyDef::Type::Bool);
      const bool navHovered = m_inGameUI.IsFlipperNav() && i == page.GetSelectedItemIndex();
      const ImVec2 itemScreenPos = ImGui::GetCursorScreenPos();
      const float itemStartY = ImGui::GetCursorPosY();
      ImDrawList* const itemDrawList = ImGui::GetWindowDrawList();

      // The item's bounds are only known once rendered (wrapped text, markdown blocks, custom
      // renders), so it is rendered in the front draw list channel and its highlight quad is
      // drawn afterward in the back channel, on the measured bounds
      itemDrawList->ChannelsSplit(2);
      itemDrawList->ChannelsSetCurrent(1);

      ImGui::SetCursorScreenPos(ImGui::GetCursorScreenPos() + ImVec2(0.f, itemPadding.y));

      // In flipper navigation, draw the highlighted item's controls in their active style
      if (navHovered)
      {
         ImGui::PushStyleColor(ImGuiCol_FrameBg, style.Colors[ImGuiCol_FrameBgActive]);
         ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, style.Colors[ImGuiCol_FrameBgActive]);
         ImGui::PushStyleColor(ImGuiCol_SliderGrab, style.Colors[ImGuiCol_SliderGrabActive]);
         ImGui::PushStyleColor(ImGuiCol_Button, style.Colors[ImGuiCol_ButtonActive]);
         ImGui::PushStyleColor(ImGuiCol_ButtonHovered, style.Colors[ImGuiCol_ButtonActive]);
         ImGui::PushStyleColor(ImGuiCol_Header, style.Colors[ImGuiCol_HeaderActive]);
         ImGui::PushStyleColor(ImGuiCol_HeaderHovered, style.Colors[ImGuiCol_HeaderActive]);
      }
      switch (item->m_type)
      {
      case Label:
      {
         ImGui::SetNextItemWidth(itemFieldEndScreenX - ImGui::GetCursorScreenPos().x - 60.f);
         switch (item->m_labelType)
         {
         case InGameUIItem::LabelType::Info: ImGui::TextWrapped("%s", item->m_label.c_str()); break;
         case InGameUIItem::LabelType::Header:
         {
            ImGui::Text("%s", item->m_label.c_str());
            ImVec2 min = ImGui::GetItemRectMin();
            ImVec2 max = ImGui::GetItemRectMax();
            min.y = max.y;
            itemDrawList->AddLine(min, max, IM_COL32_WHITE, 1.0f);
            break;
         }
         case InGameUIItem::LabelType::Markdown:
            m_player->m_liveUI->SetMarkdownStartId(ImGui::GetItemID());
            ImGui::Markdown(item->m_label.c_str(), item->m_label.length(), m_player->m_liveUI->GetMarkdownConfig());
            break;
         }
         break;
      }

      case Navigation:
      case Runnable:
      {
         // The chevron glyph ink overshoots the text layout box (drawn on the text origin it sits
         // too high against the label): keep its layout slot, but draw its ink vertically centered,
         // dropped a bit to sit on the text body rather than the line box
         const ImVec4 chevronInk = TextInkBounds(ICON_LC_CHEVRONS_RIGHT);
         const ImVec2 linePos = ImGui::GetCursorScreenPos();
         itemDrawList->AddText(ImVec2(linePos.x, linePos.y - chevronInk.y + (ImGui::GetTextLineHeight() - (chevronInk.w - chevronInk.y)) * 0.5f + ImGui::GetTextLineHeight() * 0.1f),
            ImGui::GetColorU32(ImGuiCol_Text), ICON_LC_CHEVRONS_RIGHT);
         ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::CalcTextSize(ICON_LC_CHEVRONS_RIGHT).x + 10.f);
         ImGui::Text("%s", item->m_label.c_str());
         break;
      }

      case ActionInputMapping:
      {
         if (!isStackedItem)
            ImGui::AlignTextToFramePadding();
         TextWithEllipsis(item->m_label, labelMaxWidth);
         if (item->m_inputAction->IsMapped())
         {
            if (!stackFields)
               sameLineToScreenX(labelEndScreenX);
            if (IconButtonInk(ICON_LC_X, std::format("##Item{:d}", i).c_str()))
            {
               item->m_inputAction->ClearMapping();
            }
            ImGui::SameLine();
         }
         else if (!stackFields)
         {
            sameLineToScreenX(labelEndScreenX + closeButtonWidth + style.ItemSpacing.x);
         }
         else
         {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + closeButtonWidth + style.ItemSpacing.x);
         }
         const string mappingLabel = item->m_inputAction->GetMappingLabel();
         const float mapButtonWidth = itemFieldEndScreenX - ImGui::GetCursorScreenPos().x - circleTextWidth - style.ItemSpacing.x;
         if (ImGui::Button(std::format("{}##Item{}", mappingLabel, i).c_str(), ImVec2(mapButtonWidth, 0)))
            page.RequestInputActionPopup(*item);
         if (ImGui::CalcTextSize(mappingLabel.c_str()).x >= mapButtonWidth - style.ItemSpacing.x * 2.f)
            ImGui::SetItemTooltip("%s", mappingLabel.c_str());
         {
            string state = item->m_inputAction->IsPressed() ? ICON_LC_DISC : ICON_LC_CIRCLE;
            sameLineToScreenX(itemFieldEndScreenX - circleTextWidth);
            ImGui::Text("%s", state.c_str());
         }
         if (item->IsModified())
         {
            itemMarker(ICON_LC_PENCIL);
         }
         break;
      }

      case Property:
         switch (item->m_property->m_type)
         {
         case VPX::Properties::PropertyDef::Type::Float:
         {
            auto prop = dynamic_cast<VPX::Properties::FloatPropertyDef*>(item->m_property.get());
            if (!isStackedItem)
               ImGui::AlignTextToFramePadding();
            TextWithEllipsis(prop->m_label, labelMaxWidth);
            if (!stackFields)
               sameLineToScreenX(labelEndScreenX);
            float v = item->GetFloatValue() * item->m_floatValueDisplayScale;
            ImGui::SetNextItemWidth(itemFieldEndScreenX - ImGui::GetCursorScreenPos().x);
            ImGui::SliderFloat(std::format("##Item{}", i).c_str(), &v, prop->m_min * item->m_floatValueDisplayScale, prop->m_max * item->m_floatValueDisplayScale, item->m_format.c_str(),
               ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_NoRoundToFormat);
            const bool sliderActive = ImGui::IsItemActive();
            if (item->IsModified())
            {
               itemMarker(ICON_LC_PENCIL);
            }
            else if (item->IsOverriden(g_settingsService.GetAppSettings(), g_settingsService.GetActiveSettings()))
            {
               itemMarker(ICON_LC_CIRCLE_DOT);
            }
            page.SetRebuildDeferred(sliderActive);
            item->SetValue(v / item->m_floatValueDisplayScale);
            page.SetRebuildDeferred(false);
            break;
         }

         case VPX::Properties::PropertyDef::Type::Int:
         {
            auto prop = dynamic_cast<VPX::Properties::IntPropertyDef*>(item->m_property.get());
            if (!isStackedItem)
               ImGui::AlignTextToFramePadding();
            TextWithEllipsis(prop->m_label, labelMaxWidth);
            if (!stackFields)
               sameLineToScreenX(labelEndScreenX);
            int v = item->GetIntValue();
            const auto id = Settings::GetRegistry().GetPropertyId(item->m_property->m_groupId, item->m_property->m_propId);
            if (id.has_value()
               && (((Settings::m_propPlayer_PlayfieldWidth.index == id.value().index) || (Settings::m_propPlayer_PlayfieldHeight.index == id.value().index)) && !m_inGameUI.IsFlipperNav()))
            {
               // Special handling for editing main window size as mouse interaction would break (since the control is moved/resized while interacted)
               const float butWidth = IconButtonInkSize(ICON_LC_MOVE_HORIZONTAL).x;
               ImGui::SetNextItemWidth(itemFieldEndScreenX - ImGui::GetCursorScreenPos().x - ImGui::GetStyle().ItemSpacing.x - butWidth);
               ImGui::InputInt(std::format("##Item{}", i).c_str(), &v, 1, 100, ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_CharsNoBlank);
               ImGui::SameLine();
               if (IconButtonInk((Settings::m_propPlayer_PlayfieldWidth.index == id.value().index) ? ICON_LC_MOVE_HORIZONTAL : ICON_LC_MOVE_VERTICAL, std::format("##Item{}", i).c_str()))
                  v = prop->m_max;
            }
            else
            {
               ImGui::SetNextItemWidth(itemFieldEndScreenX - ImGui::GetCursorScreenPos().x);
               ImGui::SliderInt(std::format("##Item{}", i).c_str(), &v, prop->m_min, prop->m_max, item->m_format.c_str(), ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_NoRoundToFormat);
            }
            const bool sliderActive = ImGui::IsItemActive();
            if (item->IsModified())
            {
               itemMarker(ICON_LC_PENCIL);
            }
            else if (item->IsOverriden(g_settingsService.GetAppSettings(), g_settingsService.GetActiveSettings()))
            {
               itemMarker(ICON_LC_CIRCLE_DOT);
            }
            page.SetRebuildDeferred(sliderActive);
            item->SetValue(v);
            page.SetRebuildDeferred(false);
            break;
         }

         case VPX::Properties::PropertyDef::Type::Enum:
         {
            auto prop = dynamic_cast<VPX::Properties::EnumPropertyDef*>(item->m_property.get());
            if (!isStackedItem)
               ImGui::AlignTextToFramePadding();
            TextWithEllipsis(prop->m_label, labelMaxWidth);
            if (!stackFields)
               sameLineToScreenX(labelEndScreenX);
            int v = item->GetIntValue() - prop->m_min;
            ImGui::SetNextItemWidth(itemFieldEndScreenX - ImGui::GetCursorScreenPos().x);
            ImGui::Combo(
               std::format("##Item{}", i).c_str(), &v,
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
               itemMarker(ICON_LC_PENCIL);
            }
            else if (item->IsOverriden(g_settingsService.GetAppSettings(), g_settingsService.GetActiveSettings()))
            {
               itemMarker(ICON_LC_CIRCLE_DOT);
            }
            item->SetValue(prop->m_min + v);
            break;
         }

         case VPX::Properties::PropertyDef::Type::Bool:
         {
            auto prop = dynamic_cast<VPX::Properties::BoolPropertyDef*>(item->m_property.get());
            if (!isStackedItem)
               ImGui::AlignTextToFramePadding();
            TextWithEllipsis(prop->m_label, stackFields ? itemFieldEndScreenX - rowStartScreenX - ImGui::GetFrameHeight() * 1.75f - style.ItemSpacing.x : maxLabelWidth);
            if (stackFields)
               ImGui::SameLine();
            else
               sameLineToScreenX(labelEndScreenX);
            bool v = item->GetBoolValue();
            RenderToggle(m_player, std::format("{}##Item{}", prop->m_label, i), ImVec2(itemFieldEndScreenX - ImGui::GetCursorScreenPos().x, ImGui::GetFrameHeight()), v);
            if (item->IsModified())
            {
               itemMarker(ICON_LC_PENCIL);
            }
            else if (item->IsOverriden(g_settingsService.GetAppSettings(), g_settingsService.GetActiveSettings()))
            {
               itemMarker(ICON_LC_CIRCLE_DOT);
            }
            item->SetValue(v);
            break;
         }

         case VPX::Properties::PropertyDef::Type::String:
         {
            auto prop = dynamic_cast<VPX::Properties::StringPropertyDef*>(item->m_property.get());
            if (!isStackedItem)
               ImGui::AlignTextToFramePadding();
            TextWithEllipsis(prop->m_label, labelMaxWidth);
            if (!stackFields)
               sameLineToScreenX(labelEndScreenX);
            string v = item->GetStringValue();
            if (prop->m_pathType != VPX::Properties::StringPropertyDef::PathType::None)
            {
               const bool isFolder = prop->m_pathType == VPX::Properties::StringPropertyDef::PathType::Folder;
               const char* browseIcon = isFolder ? ICON_LC_FOLDER_OPEN : ICON_LC_FILE;
               const char* hint = isFolder ? "Select a folder..." : "Select a file...";
               const float browseButtonWidth = IconButtonInkSize(browseIcon).x;
               ImGui::SetNextItemWidth(itemFieldEndScreenX - ImGui::GetCursorScreenPos().x - browseButtonWidth - style.ItemSpacing.x);
               ImGui::InputTextWithHint(std::format("##Item{}", i).c_str(), hint, &v);
               ImGui::SameLine();
               if (IconButtonInk(browseIcon, std::format("##Item{}", i).c_str()))
                  page.BrowseForPath(*item);
               if (ImGui::IsItemHovered())
                  ImGui::SetTooltip("%s", hint);
            }
            else
            {
               ImGui::SetNextItemWidth(itemFieldEndScreenX - ImGui::GetCursorScreenPos().x);
               ImGui::InputText(std::format("##Item{}", i).c_str(), &v);
            }
            if (item->IsModified())
            {
               itemMarker(ICON_LC_PENCIL);
            }
            else if (item->IsOverriden(g_settingsService.GetAppSettings(), g_settingsService.GetActiveSettings()))
            {
               itemMarker(ICON_LC_CIRCLE_DOT);
            }
            item->SetValue(v);
            break;
         }

         default: assert(false); break;
         }
         break;

      default: assert(false); break;
      }
      if (navHovered)
         ImGui::PopStyleColor(7);
      ImGui::SetCursorScreenPos(ImGui::GetCursorScreenPos() + ImVec2(0.f, itemPadding.y));

      // Now that the item's bounds are known, draw its highlight quad in the back channel and
      // evaluate mouse over/keyboard navigation on them
      itemDrawList->ChannelsSetCurrent(0);
      const float itemHeight = ImGui::GetCursorPosY() - itemStartY - style.ItemSpacing.y;
      const bool isMouseOver = ImGui::IsWindowHovered() && (ImGui::GetMousePos().y >= itemScreenPos.y - itemPadding.y - 1.f) && (ImGui::GetMousePos().y <= itemScreenPos.y + itemHeight);
      const bool hovered = navHovered || (!m_inGameUI.IsFlipperNav() && isMouseOver && item->IsSelectable());
      if (hovered)
      {
         hoveredItem = item.get();
         const ImVec2 itemStartScreen(ImMax(itemScreenPos.x - itemPadding.x, ImGui::GetWindowPos().x), itemScreenPos.y);
         const ImVec2 itemEndScreen(itemEndScreenX, itemStartScreen.y + itemHeight);
         itemDrawList->AddRectFilled(itemStartScreen, itemEndScreen, hoveredBackgroundColor, 3.f * m_player->m_liveUI->GetDPI());
         itemDrawList->AddRect(itemStartScreen, itemEndScreen, hoveredBorderColor, 3.f * m_player->m_liveUI->GetDPI(), 2.f * m_player->m_liveUI->GetDPI());
         if (navHovered)
         {
            if (item->m_type == Label && item->m_labelType == InGameUIItem::LabelType::Markdown && itemHeight > ImGui::GetWindowHeight())
            {
               // Scrollable markdown item: scroll within its bounds (item scroll is adjusted by AdjustItem)
               page.SetItemScroll(clamp(page.GetItemScroll(), 0.f, itemHeight - ImGui::GetWindowHeight()));
               ImGui::SetScrollY(itemStartY + page.GetItemScroll());
            }
            else if (itemStartY - ImGui::GetScrollY() < 0.f)
               ImGui::SetScrollY(itemStartY);
            else if (itemStartY + itemHeight - ImGui::GetScrollY() > ImGui::GetWindowHeight())
               ImGui::SetScrollY(itemStartY + itemHeight - ImGui::GetWindowHeight());
         }
      }
      else if (!m_inGameUI.IsFlipperNav() && isMouseOver)
      {
         hoveredItem = item.get();
      }
      itemDrawList->ChannelsMerge();

      if ((item->m_type == Navigation || item->m_type == Runnable) && isMouseOver && !m_isDraggingScroll && ImGui::IsMouseReleased(ImGuiMouseButton_::ImGuiMouseButton_Left))
      {
         page.SelectItem(i);
         page.AdjustItem(1.f, true);
      }
   }
   if (!ImGui::IsMouseDown(ImGuiMouseButton_Left) && m_isDraggingScroll)
      m_isDraggingScroll = false;

   // Embedded panes are drawn inline at the end of the item list (e.g. loading page progress)
   for (const InGameUIPane& pane : panes)
      if (pane.placement == InGameUIPane::Placement::Embedded && pane.render)
         pane.render(ImVec4(0.f, 0.f, ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y));

   ImGui::Dummy(ImVec2(0, 0));
   ImGui::EndChild();

   ImGui::Separator();

   ImGui::BeginChild("Info", ImVec2(), ImGuiChildFlags_None, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar);
   if (hoveredItem && std::ranges::find_if(items, [hoveredItem](auto& item) { return item.get() == hoveredItem; }) != items.end() && !hoveredItem->m_tooltip.empty())
      ImGui::TextWrapped("%s", hoveredItem->m_tooltip.c_str());
   else if (undoHovered)
      ImGui::TextWrapped("Undo changes\n[Input shortcut: Credit Button]");
   else if (defaultHovered)
      ImGui::TextWrapped("Reset page to defaults\n[Input shortcut: Launch Button]");
   else if (saveHovered)
      ImGui::TextWrapped("Save changes\n[Input shortcut: Start Button]");
   else if (backHovered)
      ImGui::TextWrapped("Get back\n[Input shortcut: Quit Button]");
   else if (!page.GetInfo().empty())
      ImGui::TextWrapped("%s", page.GetInfo().c_str());
   ImGui::EndChild();

   ImGui::PopStyleColor();

   page.SetWindowState(ImGui::GetWindowPos(), ImGui::GetWindowSize(), ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem));

   ImGui::End();

   RenderInputActionPopup(m_player, page);
   RenderAttachedPanes(page, panes);
}

void InGameUIDefaultRenderer::RenderAttachedPanes(InGameUIPage& page, const vector<InGameUIPane>& panes)
{
   const ImGuiStyle& style = ImGui::GetStyle();
   const ImVec2 winPos = page.GetWindowPos();
   const ImVec2 winSize = page.GetWindowSize();
   // Attached panes stack above the page window, first declared pane nearest to the window
   float paneBottomY = winPos.y - style.ItemSpacing.y;
   for (const InGameUIPane& pane : panes)
   {
      if (pane.placement != InGameUIPane::Placement::Attached || !pane.render)
         continue;
      const ImVec2 paneSize = pane.measure ? pane.measure(winSize.x, paneBottomY) : ImVec2(winSize.x, min(winSize.x, paneBottomY));
      const ImVec4 rect(winPos.x + (winSize.x - paneSize.x) * 0.5f, paneBottomY - paneSize.y, paneSize.x, paneSize.y);
      pane.render(rect);
      paneBottomY = rect.y - style.ItemSpacing.y;
   }
}

}
