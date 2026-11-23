// license:GPLv3+

#pragma once

#include <optional>
#include <unordered_set>

#include "InGameUIPage.h"


namespace VPX::InGameUI
{

struct DeviceLayout;
struct DeviceLayoutAnchor;

class InputDevicePage final : public InGameUIPage
{
public:
   InputDevicePage(uint16_t deviceId);

   void CollectPanes(vector<InGameUIPane>& panes) override;

private:
   // One bound device element (a button, or an axis qualified by its direction
   // threshold) with the actions it triggers and the foreign chords it is part of
   struct ElementMapping
   {
      uint16_t elementId;
      float threshold;
      bool reversed;
      bool isAxis; // threshold/reversed are significant only on axis elements
      string label;
      vector<InputAction*> owners;
      vector<string> chords;
   };

   static bool MatchesElement(const ElementMapping& element, const ButtonMapping& member, uint16_t deviceId)
   {
      return member.GetDeviceId() == deviceId && member.GetAxisOrButtonId() == element.elementId
         && (!element.isAxis || (member.GetAxisThreshold() == element.threshold && member.IsAxisReversed() == element.reversed));
   }

   void BuildPage() override;

   void BuildActionMappingItems(bool& hasMapping);
   void BuildSensorMappingItems(bool& hasMapping);
   vector<ElementMapping> CollectElementMappings();
   vector<ElementMapping> ResolveAnchorVariants(const DeviceLayoutAnchor& anchor);
   void SelectAnchorElement(int anchorIndex);
   void EnsureSelectedElement();
   void AddElementComboItem(const ElementMapping& element, const vector<InputAction*>& actions);
   void RenderDeviceLayout(const ImVec4& rect);

   const uint16_t m_deviceId;
   const string m_deviceSettingId;
   const DeviceLayout* m_layout = nullptr;
   ImTextureID m_layoutImage;
   bool m_layoutImageRequested = false;
   std::optional<ElementMapping> m_selectedElement; // element edited by the field under the diagram
   int m_selectedAnchorIndex = -1;
   int m_clickAnchor = -1; // last clicked anchor; repeated clicks cycle its element variants
   int m_hoveredAnchorIndex = -1; // anchor hovered last frame; links caption/callout highlight
   size_t m_clickVariant = 0;
   std::unordered_set<class InputAction*> m_pendingSaveActions;
};

}
