#include "core/stdafx.h"

#include "PropertyPane.h"
#include "imgui/imgui_stdlib.h"

namespace VPX::EditorUI
{

PropertyPane::PropertyPane(PinTable* table)
   : m_table(table)
{
}

PropertyPane::~PropertyPane() { }

void PropertyPane::SetShowStartup(bool showStartup)
{
   assert(!showStartup || m_table->m_liveBaseTable);
   m_showStartup = showStartup;
}

void PropertyPane::PropertyLabel(const string& label)
{
   const ImVec2 pos = ImGui::GetCursorScreenPos();
   const float xSpacing = ImGui::GetStyle().ItemSpacing.x;
   const float xWidth = ImGui::GetContentRegionAvail().x * 0.5f - m_syncWidth;

   float textWidth = ImGui::CalcTextSize(label.c_str()).x;
   if (textWidth + xSpacing > xWidth)
   {
      string ellipsis = label;
      size_t n = label.length();
      while (n > 1 && textWidth + xSpacing > xWidth)
      {
         n--;
         ellipsis = trim_string(label.substr(0, n)) + "..."s;
         textWidth = ImGui::CalcTextSize(ellipsis.c_str()).x;
      }
      ImGui::SetCursorScreenPos(ImVec2(pos.x + max(0.0f, xWidth - textWidth - xSpacing), pos.y + ImGui::GetStyle().FramePadding.y));
      ImGui::TextUnformatted(ellipsis.c_str());
      ImGui::SetItemTooltip("%s", label.c_str());
   }
   else
   {
      ImGui::SetCursorScreenPos(ImVec2(pos.x + xWidth - textWidth - xSpacing, pos.y + ImGui::GetStyle().FramePadding.y));
      ImGui::TextUnformatted(label.c_str());
   }

   ImGui::SetCursorScreenPos(ImVec2(pos.x + xWidth, pos.y));
   ImGui::SetNextItemWidth(xWidth);
   m_syncPos = ImVec2(pos.x + xWidth + xWidth, pos.y);
}

void PropertyPane::Header(const string& typeName, const std::function<string()>& getName, const std::function<void(const string&)>& setName)
{
   ImGui::NewLine();
   LiveUI::CenteredText(typeName);
   ImGui::BeginDisabled(m_table->m_liveBaseTable); // Do not edit name of live objects as it would break the script
   PropertyLabel("Name"s);
   if (string name = getName(); ImGui::InputText("##Name", &name))
      setName(name);
   ImGui::EndDisabled();
   ImGui::Separator();
}

void PropertyPane::EditableHeader(const string& typeName, IEditable* editable)
{
   Header(typeName, [editable]() { return editable->GetName(); }, [editable](const string& v) { editable->SetName(v); });
}

void PropertyPane::Separator(const string& label) const
{
   ImGui::TextUnformatted(label.c_str());
   // Underline the text
   const ImVec2 headerMin = ImGui::GetItemRectMin();
   const ImVec2 headerMax = ImGui::GetItemRectMax();
   ImDrawList* drawList = ImGui::GetWindowDrawList();
   const ImVec2 lineStart(headerMin.x, headerMax.y);
   const ImVec2 lineEnd(headerMax.x, headerMax.y);
   drawList->AddLine(lineStart, lineEnd, ImGui::GetColorU32(ImGuiCol_Text), 1.0f);
}

bool PropertyPane::BeginSection(const string& name)
{
   assert(!m_inSection);
   const bool opened = ImGui::CollapsingHeader(name.c_str(), ImGuiTreeNodeFlags_DefaultOpen);

   // Add a white line above
   const ImVec2 headerMin = ImGui::GetItemRectMin();
   const ImVec2 headerMax = ImGui::GetItemRectMax();
   ImDrawList* drawList = ImGui::GetWindowDrawList();
   const ImVec2 lineStart(headerMin.x, headerMin.y);
   const ImVec2 lineEnd(headerMax.x, headerMin.y);
   drawList->AddLine(lineStart, lineEnd, ImGui::GetColorU32(ImGuiCol_Text), 1.0f);

   if (!opened)
      return false;

   m_inSection = true;
   m_sectionHasSync = m_table->m_liveBaseTable;
   // for some reason I don't really get, the computed size is too large
   //m_syncWidth = m_sectionHasSync ? (ImGui::CalcTextSize(ICON_SAVE).x + ImGui::GetStyle().ItemSpacing.x) : 0.f;
   m_syncWidth = m_sectionHasSync ? (ImGui::CalcTextSize(ICON_SAVE).x * 0.5f) : 0.f;
   return true;
}

void PropertyPane::EndSection()
{
   assert(m_inSection);
   m_inSection = false;
}

void PropertyPane::ConvertUnit(Unit from, Unit& to, float& value, int& nDecimalAdjust)
{
   nDecimalAdjust = 0;
   if (from == Unit::Percent) // All percent values are shown as 0..100 range
      to = Unit::PercentX100;
   switch (to)
   {
   case Unit::Percent:
      switch (from)
      {
      case Unit::PercentX100:
         value = value / 100.f;
         nDecimalAdjust = 2;
         break;
      default: to = from; break;
      }
      break;

   case Unit::VPLength:
      switch (from)
      {
      case Unit::Inches:
         value = INCHESTOVPU(value);
         nDecimalAdjust = -2;
         break;
      case Unit::Millimeters:
         value = MMTOVPU(value);
         nDecimalAdjust = -2;
         break;
      default: to = from; break;
      }
      break;
   
   case Unit::Inches:
      switch (from)
      {
      case Unit::VPLength:
         value = VPUTOINCHES(value);
         nDecimalAdjust = 2;
         break;
      default: to = from; break;
      }
      break;

   case Unit::Millimeters:
      switch (from)
      {
      case Unit::VPLength:
         value = VPUTOMM(value);
         nDecimalAdjust = 2;
         break;
      default: to = from; break;
      }
      break;

   case Unit::PercentX100:
      switch (from)
      {
      case Unit::Percent:
         value = value * 100.f;
         nDecimalAdjust = -2;
         break;
      default: to = from; break;
      }
      break;

   default: to = from; break;
   }
}

const char* PropertyPane::GetUnitLabel(Unit unit)
{
   switch (unit)
   {
   case Unit::Inches: return "inches";
   case Unit::Millimeters: return "mm";
   case Unit::Degree: return "deg";
   case Unit::Percent:
   case Unit::PercentX100: return "%";
   case Unit::VPLength:
   case Unit::VPMass:
   case Unit::VPSpeed: return "vpu";
   default: return nullptr;
   }
}

}
