// license:GPLv3+

#include "core/stdafx.h"
#include "InputDevicePage.h"

#include "core/VPApp.h"
#include "core/extern.h"
#include "input/PhysicsSensor.h"
#include "input/PlungerHandler.h"
#include "physics/cabinet/CabinetNudgeSensor.h"
#include "physics/cabinet/GamepadNudge.h"
#include "renderer/RenderDevice.h"
#include "ui/live/LiveUI.h"
#include "ui/live/ingameui/DeviceLayouts.h"


namespace VPX::InGameUI
{

InputDevicePage::InputDevicePage(uint16_t deviceId)
   : InGameUIPage(
        g_pplayer->m_pininput.GetDeviceName(deviceId), std::format("Status: {}", g_pplayer->m_pininput.IsDeviceConnected(deviceId) ? "Connected"s : "Not connected"s), SaveMode::Global)
   , m_deviceId(deviceId)
   , m_deviceSettingId(m_player->m_pininput.GetDeviceSettingId(m_deviceId))
   , m_layout(FindDeviceLayout(m_player->m_pininput, m_deviceId))
{
}

namespace
{

   string ElementLabel(const InputManager& input, uint16_t deviceId, uint16_t elementId, float threshold, bool reversed)
   {
      string label = (input.GetDeviceType(deviceId) == InputManager::DeviceType::Keyboard) ? "'"s : ""s;
      label += XRElementNameToFriendly(input.GetDeviceElementName(deviceId, elementId));
      if (input.GetDeviceType(deviceId) == InputManager::DeviceType::Keyboard)
         label += "'"s;
      if (reversed || threshold != 0.f)
         label += reversed ? std::format(" (<= {:.2f})", threshold) : std::format(" (>= {:.2f})", threshold);
      return label;
   }

   string ElementLabel(const InputManager& input, const ButtonMapping& member)
   {
      return ElementLabel(input, member.GetDeviceId(), member.GetAxisOrButtonId(), member.GetAxisThreshold(), member.IsAxisReversed());
   }

}

void InputDevicePage::BuildPage()
{
   const InputManager& input = m_player->m_pininput;

   // The field under the device diagram edits the action of the element
   // selected by clicking an input area on the artwork
   EnsureSelectedElement();
   if (m_selectedElement.has_value())
   {
      m_selectedElement->label = ElementLabel(input, m_deviceId, m_selectedElement->elementId, m_selectedElement->threshold, m_selectedElement->reversed);
      vector<InputAction*> actions;
      for (const auto& action : input.GetInputActions())
         actions.push_back(action.get());
      AddElementComboItem(*m_selectedElement, actions);
   }

   bool hasActionMapping = false;
   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Action mappings"s));
   BuildActionMappingItems(hasActionMapping);
   bool hasSensorMapping = false;
   BuildSensorMappingItems(hasSensorMapping);
   if (!hasActionMapping && !hasSensorMapping)
      AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Info, "No mapping defined for this device"s));

   const auto noAutoLayoutId = Settings::GetRegistry().GetPropertyId("Input"s, std::format("Device.{}.NoAutoLayout", m_deviceSettingId));
   if (m_player->m_pininput.HasDeviceDefaultMapping(m_deviceId))
   {
      AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Device control"s));
      if (noAutoLayoutId.has_value())
         AddItem(std::make_unique<InGameUIItem>( //
            noAutoLayoutId.value(), //
            [noAutoLayoutId]() { return g_settingsService.GetAppSettings().GetBool(noAutoLayoutId.value()); }, //
            [noAutoLayoutId](bool v) { g_settingsService.GetAppSettings().Set(noAutoLayoutId.value(), v, false); }));

      AddItem(std::make_unique<InGameUIItem>("Apply default layout"s, "Reset the input layout of this device to its default."s,
         [this]()
         {
            m_player->m_pininput.ApplyDefaultDeviceMapping(m_deviceId);
            m_player->m_liveUI->m_inGameUI.NavigateBack();
            RequestRebuild();
         }));
   }

   // The erase also resets the NoAutoLayout flag, so the zone stays visible
   // while it is set even when the device has no mapping
   const bool noAutoLayoutSet = noAutoLayoutId.has_value() && g_settingsService.GetAppSettings().GetBool(noAutoLayoutId.value());
   if (hasActionMapping || hasSensorMapping || noAutoLayoutSet)
   {
      AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Danger zone"s));
      AddItem(std::make_unique<InGameUIItem>("Erase this device"s, "Clean all mappings using this device."s,
         [this, noAutoLayoutId]()
         {
            if (noAutoLayoutId.has_value())
               g_settingsService.GetAppSettings().Set(noAutoLayoutId.value(), false, false);
            m_player->m_pininput.ClearDeviceMappings(m_deviceId);
            m_player->m_liveUI->m_inGameUI.NavigateBack();
            RequestRebuild();
         }));
   }
}

vector<InputDevicePage::ElementMapping> InputDevicePage::CollectElementMappings()
{
   InputManager& input = m_player->m_pininput;
   vector<ElementMapping> elements;
   std::unordered_set<uint16_t> axisIds;
   for (const InputManager::DeviceElement& el : input.GetDeviceElements(m_deviceId))
      if (el.isAxis)
         axisIds.insert(el.id);
   for (const auto& action : input.GetInputActions())
   {
      for (const InputAction::Binding& binding : action->GetBindings())
      {
         string chord;
         for (const ButtonMapping& member : binding.buttons)
            if (member.GetDeviceId() != m_deviceId)
            {
               if (!chord.empty())
                  chord += " and "s;
               chord += ElementLabel(input, member);
            }
         for (const ButtonMapping& member : binding.buttons)
         {
            if (member.GetDeviceId() != m_deviceId)
               continue;
            // A button member can carry a threshold/reverse copied from a
            // chord partner ("button + stick <= -0.9"); meaningless on a
            // digital element, normalize so its bindings merge into one entry
            const bool isAxis = axisIds.contains(member.GetAxisOrButtonId());
            const float threshold = isAxis ? member.GetAxisThreshold() : 0.f;
            const bool reversed = isAxis && member.IsAxisReversed();
            auto it = std::ranges::find_if(
               elements, [&member, threshold, reversed](const ElementMapping& e) { return e.elementId == member.GetAxisOrButtonId() && e.threshold == threshold && e.reversed == reversed; });
            if (it == elements.end())
            {
               elements.push_back({ member.GetAxisOrButtonId(), threshold, reversed, isAxis, ElementLabel(input, m_deviceId, member.GetAxisOrButtonId(), threshold, reversed), {}, {} });
               it = elements.end() - 1;
            }
            if (std::ranges::find(it->owners, action.get()) == it->owners.end())
               it->owners.push_back(action.get());
            if (!chord.empty() && std::ranges::find(it->chords, chord) == it->chords.end())
               it->chords.push_back(chord);
         }
      }
   }
   std::ranges::sort(elements, [](const ElementMapping& a, const ElementMapping& b) { return a.label < b.label; });
   return elements;
}

// The element variants editable through an anchor: every bound element tuple
// whose device element resolves to it, or the unbound anchor element itself.
// A merged anchor (e.g. stick + stick X/Y) can own several variants; clicking
// cycles them.
vector<InputDevicePage::ElementMapping> InputDevicePage::ResolveAnchorVariants(const DeviceLayoutAnchor& anchor)
{
   InputManager& input = m_player->m_pininput;
   const vector<ElementMapping> bound = CollectElementMappings();
   vector<ElementMapping> variants;
   const vector<InputManager::DeviceElement> deviceElements = input.GetDeviceElements(m_deviceId);
   vector<uint16_t> elementIds;
   for (const InputManager::DeviceElement& el : deviceElements)
      if (FindDeviceAnchor(*m_layout, el.id, input.GetDeviceElementName(m_deviceId, el.id)) == &anchor)
         elementIds.push_back(el.id);
   // Devices which do not enumerate their elements (keyboard, mouse) resolve
   // their anchor's fixed element id directly
   if (elementIds.empty() && deviceElements.empty() && anchor.elementId != 0xFFFF)
      elementIds.push_back(anchor.elementId);
   for (const uint16_t elementId : elementIds)
   {
      bool isBound = false;
      for (const ElementMapping& e : bound)
         if (e.elementId == elementId)
         {
            variants.push_back(e);
            isBound = true;
         }
      if (!isBound)
         variants.push_back({ elementId, 0.f, false, false, ElementLabel(input, m_deviceId, elementId, 0.f, false), {}, {} });
   }
   return variants;
}

void InputDevicePage::SelectAnchorElement(int anchorIndex)
{
   const vector<ElementMapping> variants = ResolveAnchorVariants(m_layout->anchors[anchorIndex]);
   if (variants.empty())
      return;
   if (anchorIndex == m_clickAnchor)
      m_clickVariant = (m_clickVariant + 1) % variants.size();
   else
   {
      m_clickAnchor = anchorIndex;
      m_clickVariant = 0;
   }
   m_selectedAnchorIndex = anchorIndex;
   m_selectedElement = variants[m_clickVariant];
   RequestRebuild();
}

// Default selection: the first bound element drawn on the artwork, else the
// first resolvable anchor, so the field under the diagram is never empty
void InputDevicePage::EnsureSelectedElement()
{
   if (m_selectedElement.has_value() || m_layout == nullptr)
      return;
   InputManager& input = m_player->m_pininput;
   for (const ElementMapping& e : CollectElementMappings())
      if (const DeviceLayoutAnchor* anchor = FindDeviceAnchor(*m_layout, e.elementId, input.GetDeviceElementName(m_deviceId, e.elementId)); anchor != nullptr)
      {
         m_selectedAnchorIndex = static_cast<int>(anchor - m_layout->anchors.data());
         m_clickAnchor = m_selectedAnchorIndex;
         m_selectedElement = e;
         return;
      }
   for (const DeviceLayoutAnchor& anchor : m_layout->anchors)
      if (const vector<ElementMapping> variants = ResolveAnchorVariants(anchor); !variants.empty())
      {
         m_selectedAnchorIndex = static_cast<int>(&anchor - m_layout->anchors.data());
         m_clickAnchor = m_selectedAnchorIndex;
         m_selectedElement = variants.front();
         return;
      }
}

void InputDevicePage::AddElementComboItem(const ElementMapping& element, const vector<InputAction*>& actions)
{
   vector<string> actionNames { "Not mapped"s };
   for (const InputAction* action : actions)
      actionNames.push_back(action->GetLabel());

   string tooltip = "Select the action triggered by this input element."s;
   // The actions this element also participates in, besides the one owning the
   // direct mapping shown in the dropdown: chord owners name the other
   // button(s) to press together ("with X and Y"), additional direct bindings
   // are listed bare
   const auto hasElement = [this, &element](const InputAction::Binding& binding)
   { return std::ranges::any_of(binding.buttons, [this, &element](const ButtonMapping& member) { return MatchesElement(element, member, m_deviceId); }); };
   const auto isDirect = [](const InputAction::Binding& binding) { return binding.buttons.size() == 1; };
   // The owner shown selected in the dropdown: first action holding a direct binding
   InputAction* const shownOwner = [this, &element, &hasElement, &isDirect]()
   {
      for (InputAction* owner : element.owners)
         if (std::ranges::any_of(owner->GetBindings(), [&hasElement, &isDirect](const InputAction::Binding& b) { return isDirect(b) && hasElement(b); }))
            return owner;
      return static_cast<InputAction*>(nullptr);
   }();
   vector<string> alsoTriggers;
   for (InputAction* owner : element.owners)
   {
      vector<string> others;
      for (const InputAction::Binding& binding : owner->GetBindings())
      {
         if (!hasElement(binding))
            continue;
         if (isDirect(binding))
            continue;
         else
            for (const ButtonMapping& member : binding.buttons)
            {
               if (MatchesElement(element, member, m_deviceId))
                  continue;
               const string other = ElementLabel(m_player->m_pininput, member);
               if (std::ranges::find(others, other) == others.end())
                  others.push_back(other);
            }
      }
      if (!others.empty())
      {
         string with;
         for (const string& other : others)
            with += (with.empty() ? ""s : " and "s) + other;
         alsoTriggers.push_back(owner->GetLabel() + " with "s + with);
      }
      else if (owner != shownOwner)
         alsoTriggers.push_back(owner->GetLabel());
      // else: the action owning the dropdown's direct mapping, not an "also"
   }
   if (alsoTriggers.size() == 1)
      tooltip += "\nAlso triggers "s + alsoTriggers.front();
   else if (!alsoTriggers.empty())
   {
      tooltip += "\nAlso triggers:"s;
      for (const string& also : alsoTriggers)
         tooltip += "\n- "s + also;
   }
   const auto ownerIndex = [this, element, actions]()
   {
      for (size_t i = 0; i < actions.size(); i++)
         for (const InputAction::Binding& binding : actions[i]->GetBindings())
            // The dropdown only manages the direct mapping; combined mappings
            // (chords) keep their own entry in the list above
            if (binding.buttons.size() == 1)
               for (const ButtonMapping& member : binding.buttons)
                  if (MatchesElement(element, member, m_deviceId))
                     return static_cast<int>(i) + 1;
      return 0;
   };
   AddItem(std::make_unique<InGameUIItem>( //
      VPX::Properties::EnumPropertyDef(""s, ""s, element.label, tooltip, false, 0, 0, actionNames), //
      [ownerIndex]() { return ownerIndex(); }, //
      [ownerIndex](const Settings&) { return ownerIndex(); }, //
      [this, element, actions](int, int v)
      {
         // Reassign this element exclusively to the selected action
         for (InputAction* action : actions)
            if (std::ranges::any_of(action->GetBindings(),
                   [this, &element](const InputAction::Binding& binding)
                   {
                      return binding.buttons.size() == 1
                         && std::ranges::any_of(binding.buttons, [this, &element](const ButtonMapping& member) { return MatchesElement(element, member, m_deviceId); });
                   }))
            {
               action->UnmapElement(m_deviceId, element.elementId, element.threshold, element.reversed, true, !element.isAxis);
               m_pendingSaveActions.insert(action);
            }
         if (v > 0)
         {
            actions[v - 1]->AddMapping(ButtonMapping::Create(m_deviceId, element.elementId, element.threshold, element.reversed));
            m_pendingSaveActions.insert(actions[v - 1]);
         }
         RequestRebuild();
      }, //
      [this](Settings& settings)
      {
         for (InputAction* action : m_pendingSaveActions)
            action->LoadMapping(settings);
      }, //
      [this](int, Settings& settings, bool)
      {
         for (InputAction* action : m_pendingSaveActions)
            action->SaveMapping(settings);
         m_pendingSaveActions.clear();
      }));
}

void InputDevicePage::BuildActionMappingItems(bool& hasMapping)
{
   InputManager& input = m_player->m_pininput;
   vector<InputAction*> actions;
   for (const auto& action : input.GetInputActions())
      actions.push_back(action.get());

   // List the mappings element first: each bound button/axis of this device gets
   // a dropdown item selecting which action it triggers
   const vector<ElementMapping> elements = CollectElementMappings();
   hasMapping = !elements.empty();
   for (const ElementMapping& element : elements)
      AddElementComboItem(element, actions);
}

void InputDevicePage::BuildSensorMappingItems(bool& hasMapping)
{
   const InputManager& input = m_player->m_pininput;
   // Same element-first order as the action mappings, without sensor indices
   // (several sensors can be merged into the same logical one at runtime)
   const auto addSensorItem = [this, &hasMapping](const string& sensorLabel, uint16_t axisId)
   {
      if (!hasMapping)
         AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Sensor mappings"s));
      hasMapping = true;
      AddItem(std::make_unique<InGameUIItem>(
         InGameUIItem::LabelType::Info, std::format("{}: {}", XRElementNameToFriendly(m_player->m_pininput.GetDeviceElementName(m_deviceId, axisId)), sensorLabel)));
   };

   for (int i = 0; i < input.m_plungerHandler->GetSensorCount(); i++)
   {
      const auto& sensor = input.m_plungerHandler->GetSensor(i);
      const auto& positionSensor = sensor->GetPositionSensor();
      if (positionSensor->IsMapped() && positionSensor->GetMapping().GetDeviceId() == m_deviceId)
         addSensorItem("Plunger pos."s, positionSensor->GetMapping().GetAxisId());
      const auto& velocitySensor = sensor->GetVelocitySensor();
      if (velocitySensor->IsMapped() && velocitySensor->GetMapping().GetDeviceId() == m_deviceId)
         addSensorItem("Plunger vel."s, velocitySensor->GetMapping().GetAxisId());
   }

   for (int i = 0; i < input.m_nudgeHandler->GetSensorCount(); i++)
   {
      const auto& sensor = input.m_nudgeHandler->GetSensor(i);
      const auto addNudgeItem = [this, &addSensorItem](const string& name, const PhysicsSensor& nudgeSensor)
      {
         if (nudgeSensor.IsMapped() && nudgeSensor.GetMapping().GetDeviceId() == m_deviceId)
            addSensorItem("Nudge "s + name, nudgeSensor.GetMapping().GetAxisId());
      };
      if (const auto* gamepadSensor = dynamic_cast<const VPX::Physics::GamepadNudge*>(sensor.get()))
      {
         addNudgeItem("X"s, gamepadSensor->GetXSensor());
         addNudgeItem("Y"s, gamepadSensor->GetYSensor());
      }
      else if (const auto* cabinetSensor = dynamic_cast<const VPX::Physics::CabinetNudgeSensor*>(sensor.get()))
      {
         addNudgeItem("X acc."s, cabinetSensor->GetXAccSensor());
         addNudgeItem("Y acc."s, cabinetSensor->GetYAccSensor());
         addNudgeItem("X vel."s, cabinetSensor->GetXVelSensor());
         addNudgeItem("Y vel."s, cabinetSensor->GetYVelSensor());
      }
   }
}

void InputDevicePage::CollectPanes(vector<InGameUIPane>& panes)
{
   InGameUIPage::CollectPanes(panes);

   if (m_layout == nullptr)
      return;

   InGameUIPane pane;
   pane.placement = InGameUIPane::Placement::Header;
   pane.id = "deviceLayout"s;
   pane.measure = [this](float availW, float availH)
   {
      const float labelColW = min(availW * 0.16f, 160.f);
      const float bandH = 2.f * (ImGui::GetTextLineHeight() + 8.f);
      const float imgW = min(availW - 2.f * labelColW, std::max(0.f, availH - 2.f * bandH) * m_layout->aspect);
      return ImVec2(availW, min(2.f * bandH + imgW / m_layout->aspect, availH));
   };
   pane.render = [this](const ImVec4& rect) { RenderDeviceLayout(rect); };
   panes.push_back(std::move(pane));
}

void InputDevicePage::RenderDeviceLayout(const ImVec4& rect)
{
   if (m_layout == nullptr)
      return;
   if (!m_layoutImageRequested)
   {
      m_layoutImageRequested = true;
      m_layoutImage = LoadDeviceLayoutImage(m_player->m_renderer->m_renderDevice, m_layout->assetFile);
   }

   // Drawn into the current draw list rather than a dedicated window: windows
   // render in focus order, so a separate top-level window would go under the
   // UI background whenever the page window takes focus
   ImDrawList* const drawList = ImGui::GetWindowDrawList();
   drawList->PushClipRect(ImVec2(rect.x, rect.y), ImVec2(rect.x + rect.z, rect.y + rect.w), true);
   const float lineH = ImGui::GetTextLineHeight();

   // Layout: margins are reserved around the artwork for callout chips — a
   // chip column on the sides, up to 3 text lines above and below
   const float labelColW = min(rect.z * 0.2f, 200.f);
   const float bandH = 3.f * (lineH + 8.f) + 12.f;
   const float imgW = min(rect.z - 2.f * labelColW, std::max(0.f, rect.w - 2.f * bandH) * m_layout->aspect);
   const float imgH = imgW / m_layout->aspect;
   const ImVec2 imgPos(rect.x + (rect.z - imgW) * 0.5f, rect.y + (rect.w - imgH) * 0.5f);
   if (m_layoutImage)
      drawList->AddImage(m_layoutImage, imgPos, ImVec2(imgPos.x + imgW, imgPos.y + imgH), ImVec2(0.f, 0.f), ImVec2(1.f, 1.f), IM_COL32_WHITE);
   else
      drawList->AddRect(imgPos, ImVec2(imgPos.x + imgW, imgPos.y + imgH), ImGui::GetColorU32(ImGuiCol_Border));
   const ImVec2 imgEnd(imgPos.x + imgW, imgPos.y + imgH);

   // Collect the mapped elements of this device, merged per layout anchor
   struct CalloutLine
   {
      string elementLabel; // element incl. threshold/direction, shown when the callout merges several mappings
      string actionLabel;
      bool operator==(const CalloutLine&) const = default;
   };
   struct Callout
   {
      const DeviceLayoutAnchor* anchor;
      int anchorIndex;
      vector<CalloutLine> lines;
      bool pressed = false;
   };
   vector<Callout> callouts;
   const auto addCallout = [this, &callouts](const DeviceLayoutAnchor* anchor, const CalloutLine& line, bool pressed)
   {
      auto it = std::ranges::find_if(callouts, [anchor](const Callout& c) { return c.anchor == anchor; });
      if (it == callouts.end())
      {
         callouts.push_back({ anchor, static_cast<int>(anchor - m_layout->anchors.data()), {}, pressed });
         it = callouts.end() - 1;
      }
      it->pressed |= pressed;
      if (std::ranges::find(it->lines, line) == it->lines.end())
         it->lines.push_back(line);
   };
   const InputManager& input = m_player->m_pininput;

   // Hide anchors whose element does not exist on this device (e.g. paddles
   // on a paddle-less gamepad): they cannot be mapped. Devices reporting no
   // elements (keyboard, mouse) keep every anchor
   std::unordered_set<const DeviceLayoutAnchor*> absent;
   if (const vector<InputManager::DeviceElement> elements = input.GetDeviceElements(m_deviceId); !elements.empty())
   {
      std::unordered_set<const DeviceLayoutAnchor*> present;
      for (const InputManager::DeviceElement& el : elements)
         if (const DeviceLayoutAnchor* anchor = FindDeviceAnchor(*m_layout, el.id, el.name))
            present.insert(anchor);
      for (const DeviceLayoutAnchor& anchor : m_layout->anchors)
         if (!present.contains(&anchor))
            absent.insert(&anchor);
   }
   const auto addElementCallout = [this, &input, &addCallout, &absent](uint16_t elementId, const string& elementLabel, bool isPressed, const string& actionLabel)
   {
      const string elementName = input.GetDeviceElementName(m_deviceId, elementId);
      if (const DeviceLayoutAnchor* anchor = FindDeviceAnchor(*m_layout, elementId, elementName); anchor != nullptr && !absent.contains(anchor))
         addCallout(anchor, { elementLabel, actionLabel }, isPressed);
   };
   for (const auto& action : input.GetInputActions())
      for (const InputAction::Binding& binding : action->GetBindings())
      {
         // Only plain mappings are labeled on the artwork: composed ones
         // (chords of several buttons) would repeat the action under each
         // member, hiding the element's own mapping
         if (binding.buttons.size() != 1)
            continue;
         for (const ButtonMapping& member : binding.buttons)
            if (member.GetDeviceId() == m_deviceId)
               addElementCallout(member.GetAxisOrButtonId(), ElementLabel(input, member), member.IsPressed(), action->GetLabel());
      }
   for (int i = 0; i < input.m_plungerHandler->GetSensorCount(); i++)
   {
      const auto& sensor = input.m_plungerHandler->GetSensor(i);
      // Sensor indices are not shown: several sensors can merge into a single
      // logical plunger/nudge at runtime
      const auto addPlungerCallout = [this, &input, &addElementCallout](const std::unique_ptr<PhysicsSensor>& physicsSensor, const string& name)
      {
         const SensorMapping& mapping = physicsSensor->GetMapping();
         if (physicsSensor->IsMapped() && mapping.GetDeviceId() == m_deviceId)
            addElementCallout(
               mapping.GetAxisId(), XRElementNameToFriendly(input.GetDeviceElementName(m_deviceId, mapping.GetAxisId())), std::abs(physicsSensor->GetValue()) > 0.05f, "Plunger "s + name);
      };
      addPlungerCallout(sensor->GetPositionSensor(), "pos."s);
      addPlungerCallout(sensor->GetVelocitySensor(), "vel."s);
   }
   for (int i = 0; i < input.m_nudgeHandler->GetSensorCount(); i++)
   {
      const auto& sensor = input.m_nudgeHandler->GetSensor(i);
      const auto addNudgeCallout = [this, &input, &addElementCallout](const PhysicsSensor& physicsSensor, const string& name)
      {
         const SensorMapping& mapping = physicsSensor.GetMapping();
         if (physicsSensor.IsMapped() && mapping.GetDeviceId() == m_deviceId)
            addElementCallout(
               mapping.GetAxisId(), XRElementNameToFriendly(input.GetDeviceElementName(m_deviceId, mapping.GetAxisId())), std::abs(physicsSensor.GetValue()) > 0.05f, "Nudge "s + name);
      };
      if (const auto* gamepadSensor = dynamic_cast<const VPX::Physics::GamepadNudge*>(sensor.get()))
      {
         addNudgeCallout(gamepadSensor->GetXSensor(), "X"s);
         addNudgeCallout(gamepadSensor->GetYSensor(), "Y"s);
      }
      else if (const auto* cabinetSensor = dynamic_cast<const VPX::Physics::CabinetNudgeSensor*>(sensor.get()))
      {
         addNudgeCallout(cabinetSensor->GetXAccSensor(), "X acc."s);
         addNudgeCallout(cabinetSensor->GetYAccSensor(), "Y acc."s);
         addNudgeCallout(cabinetSensor->GetXVelSensor(), "X vel."s);
         addNudgeCallout(cabinetSensor->GetYVelSensor(), "Y vel."s);
      }
   }

   // Only callouts authored in the SVG 'callouts' layer are drawn; mapped keys
   // without one get no chip (their mapping shows in a tooltip on hover)
   vector<const Callout*> authored;
   for (const Callout& callout : callouts)
      if (callout.anchor->hasCallout)
         authored.push_back(&callout);
   const ImU32 dimLineColor = ImGui::GetColorU32(ImGuiCol_TextDisabled);
   const ImU32 textColor = ImGui::GetColorU32(ImGuiCol_Text);
   const auto accentColor = [](bool pressed) { return pressed ? IM_COL32(255, 180, 60, 255) : IM_COL32(140, 200, 255, 255); };

   // Clickable input areas (anchor chips on the artwork and their callout
   // blocks): clicking one selects its element in the field under the diagram
   struct HotArea
   {
      ImVec2 min, max;
      int anchorIndex;
   };
   vector<HotArea> hotAreas;

   // A callout merging several mappings prefixes each line with the element,
   // but only its discriminant: the family name common to all lines is dropped
   // (D-Pad Up/Down/... -> Up/Down/..., Left Stick X/Y -> X/Y). The prefix is
   // cut on a word boundary so partially shared text (a threshold...) is not
   // split mid-token
   const auto lineText = [](const Callout& callout, const CalloutLine& l)
   {
      if (callout.lines.size() <= 1)
         return l.actionLabel;
      size_t common = l.elementLabel.size();
      for (const CalloutLine& other : callout.lines)
      {
         size_t n = 0;
         const size_t m = std::min(l.elementLabel.size(), other.elementLabel.size());
         while (n < m && l.elementLabel[n] == other.elementLabel[n])
            n++;
         common = std::min(common, n);
      }
      // Cut at the start of a parenthesized group the prefix splits (so (>=
      // -0.30) stays whole), else at the last word boundary
      if (const size_t paren = l.elementLabel.rfind('(', common - 1); paren != string::npos && paren < common && l.elementLabel.find(')', paren) >= common)
         common = paren;
      else
         while (common > 0 && l.elementLabel[common - 1] != ' ' && l.elementLabel[common - 1] != '(')
            common--;
      const string& element = common == 0 || common == l.elementLabel.size() ? l.elementLabel : l.elementLabel.substr(common);
      return element + ": "s + l.actionLabel;
   };

   // The leader line lands on the middle of the chip side facing the anchor
   // (left/right side when the anchor is beside the chip, top/bottom when
   // above/below); returns the anchor position when it is inside the chip
   const auto chipSideLink = [](const ImVec2& anchorPos, const ImVec2& boxMin, const ImVec2& boxMax)
   {
      const float distX = anchorPos.x < boxMin.x ? boxMin.x - anchorPos.x : anchorPos.x > boxMax.x ? anchorPos.x - boxMax.x : 0.f;
      const float distY = anchorPos.y < boxMin.y ? boxMin.y - anchorPos.y : anchorPos.y > boxMax.y ? anchorPos.y - boxMax.y : 0.f;
      if (distX == 0.f && distY == 0.f)
         return anchorPos;
      return (distX >= distY) ? ImVec2(anchorPos.x < boxMin.x ? boxMin.x : boxMax.x, (boxMin.y + boxMax.y) * 0.5f)
                              : ImVec2((boxMin.x + boxMax.x) * 0.5f, anchorPos.y < boxMin.y ? boxMin.y : boxMax.y);
   };

   // Authored callouts: the chip is anchored in the rect drawn by the author in
   // the SVG (normalized to the artwork, may extend into the margins) and the
   // leader line links the anchor to the chip side facing it
   for (const Callout* callout : authored)
   {
      const ImVec4& c = callout->anchor->callout;
      const ImVec2 blockMin(imgPos.x + c.x * imgW, imgPos.y + c.y * imgH);
      const ImVec2 blockMax(blockMin.x + c.z * imgW, blockMin.y + c.w * imgH);
      const ImVec2 anchorPos(imgPos.x + callout->anchor->pos.x * imgW, imgPos.y + callout->anchor->pos.y * imgH);
      const bool hovered = m_hoveredAnchorIndex == callout->anchorIndex;
      // The chip hugs its text and is anchored on the authored rect's edge
      // where the wire lands — the one facing the anchor: middle right/left
      // for side wires, center bottom/top for wires coming from under/above —
      // so the chip's wire point matches the rect edge midpoint exactly
      const float distX = anchorPos.x < blockMin.x ? blockMin.x - anchorPos.x : anchorPos.x > blockMax.x ? anchorPos.x - blockMax.x : 0.f;
      const float distY = anchorPos.y < blockMin.y ? blockMin.y - anchorPos.y : anchorPos.y > blockMax.y ? anchorPos.y - blockMax.y : 0.f;
      const int align = distX >= distY ? (anchorPos.x > (blockMin.x + blockMax.x) * 0.5f ? 1 : -1) : 0;
      float chipW = 0.f;
      for (const CalloutLine& l : callout->lines)
         chipW = std::max(chipW, ImGui::CalcTextSize(lineText(*callout, l).c_str()).x);
      chipW += 12.f;
      const float chipH = callout->lines.size() * lineH + 8.f;
      const float chipX = (align > 0) ? blockMax.x - chipW : (align < 0) ? blockMin.x : (blockMin.x + blockMax.x - chipW) * 0.5f;
      const float chipY = (align == 0) ? (anchorPos.y < blockMin.y ? blockMin.y : blockMax.y - chipH) : (blockMin.y + blockMax.y - chipH) * 0.5f;
      const ImVec2 chipMin(chipX, chipY), chipMax(chipX + chipW, chipY + chipH);
      const ImVec2 link = chipSideLink(anchorPos, chipMin, chipMax);
      if (link.x != anchorPos.x || link.y != anchorPos.y)
         drawList->AddLine(anchorPos, link, callout->pressed ? accentColor(true) : hovered ? textColor : dimLineColor, 1.5f);
      const ImVec2 hotMin(std::max(chipMin.x, rect.x), std::max(chipMin.y, rect.y));
      const ImVec2 hotMax(std::min(chipMax.x, rect.x + rect.z), std::min(chipMax.y, rect.y + rect.w));
      if (hotMax.x > hotMin.x && hotMax.y > hotMin.y)
         hotAreas.push_back({ hotMin, hotMax, callout->anchorIndex });
      drawList->AddRectFilled(chipMin, chipMax, IM_COL32(255, 255, 255, hovered ? 60 : 30), 6.f);
      if (hovered)
         drawList->AddRect(chipMin, chipMax, textColor, 6.f, 0, 1.5f);
      float textY = chipY + 4.f;
      for (const CalloutLine& l : callout->lines)
      {
         const string text = lineText(*callout, l);
         const ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
         const float textX = (align > 0) ? chipMax.x - 6.f - textSize.x : (align < 0) ? chipMin.x + 6.f : chipMin.x + (chipW - textSize.x) * 0.5f;
         drawList->AddText(ImVec2(textX, textY), textColor, text.c_str());
         textY += lineH;
      }
   }

   // Element captions drawn directly on the artwork: the chip marks the element
   // and brightens when the element is mapped (accent when currently pressed,
   // outlined when it is the element selected for the field under the diagram)
   int anchorIndex = 0;
   for (const DeviceLayoutAnchor& anchor : m_layout->anchors)
   {
      const int index = anchorIndex++;
      if (absent.contains(&anchor))
         continue;
      string caption;
      if (anchor.elementId != 0xFFFF && input.GetDeviceType(m_deviceId) == InputManager::DeviceType::Keyboard)
      {
         // Single-char keys keep their localized key name (layout letters like
         // the field under the diagram); longer names fall back to the
         // layout's short caption — except non-ASCII ones (the unicode arrows
         // have no font glyph) which keep the SDL name (Up, Left, ...)
         const string keyName = input.GetDeviceElementName(m_deviceId, anchor.elementId);
         const auto utf8Len = [](const string& s)
         {
            size_t n = 0;
            for (const unsigned char c : s)
               n += (c & 0xC0) != 0x80;
            return n;
         };
         const bool asciiCaption = std::ranges::all_of(anchor.caption, [](const char c) { return static_cast<unsigned char>(c) < 0x80; });
         caption = (utf8Len(keyName) == 1 || anchor.caption.empty() || !asciiCaption) ? keyName : anchor.caption;
      }
      else if (!anchor.caption.empty())
         caption = anchor.caption;
      else if (!anchor.elementNames.empty())
         caption = anchor.elementNames.front();
      else if (anchor.elementId != 0xFFFF)
         caption = input.GetDeviceElementName(m_deviceId, anchor.elementId);
      if (caption.empty())
         continue;
      const Callout* mapped = nullptr;
      for (const Callout& callout : callouts)
         if (callout.anchor == &anchor)
            mapped = &callout;
      const ImVec2 anchorPos(imgPos.x + anchor.pos.x * imgW, imgPos.y + anchor.pos.y * imgH);
      const ImVec2 captionSize = ImGui::CalcTextSize(caption.c_str());
      // Edge anchors clamp their chip inside the artwork so it never clips away
      const ImVec2 captionPos(
         std::clamp(anchorPos.x - captionSize.x * 0.5f, imgPos.x, imgEnd.x - captionSize.x), std::clamp(anchorPos.y - captionSize.y * 0.5f, imgPos.y, imgEnd.y - captionSize.y));
      const ImVec2 chipMin(captionPos.x - 6.f, captionPos.y - 4.f);
      const ImVec2 chipMax(captionPos.x + captionSize.x + 6.f, captionPos.y + captionSize.y + 4.f);
      hotAreas.push_back({ chipMin, chipMax, index });
      const bool hovered = m_hoveredAnchorIndex == index;
      const bool selected = index == m_selectedAnchorIndex;
      drawList->AddRectFilled(chipMin, chipMax, IM_COL32(0, 0, 0, selected || hovered ? 210 : mapped != nullptr ? 170 : 90), 6.f);
      if (selected || hovered)
         drawList->AddRect(chipMin, chipMax, selected ? accentColor(true) : textColor, 6.f, 0, 1.5f);
      drawList->AddText(captionPos, selected ? accentColor(true) : mapped != nullptr ? accentColor(mapped->pressed) : ImGui::GetColorU32(ImGuiCol_TextDisabled), caption.c_str());
      // A mapped key without an authored callout shows its mapping in a tooltip
      if (hovered && mapped != nullptr && !anchor.hasCallout)
      {
         string tooltip;
         for (const CalloutLine& l : mapped->lines)
         {
            if (!tooltip.empty())
               tooltip += '\n';
            tooltip += lineText(*mapped, l);
         }
         ImGui::SetTooltip("%s", tooltip.c_str());
      }
   }

   // Hover and click an input area (caption chip or callout block, last drawn
   // on top wins): hovering links the highlights of both, clicking selects the
   // element in the field under the diagram
   m_hoveredAnchorIndex = -1;
   for (auto it = hotAreas.rbegin(); it != hotAreas.rend(); ++it)
      if (ImGui::IsMouseHoveringRect(it->min, it->max))
      {
         m_hoveredAnchorIndex = it->anchorIndex;
         break;
      }
   if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && m_hoveredAnchorIndex >= 0)
      SelectAnchorElement(m_hoveredAnchorIndex);

   drawList->PopClipRect();
}

}
