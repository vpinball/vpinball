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

void PropertyPane::Header(const string& typeName, const std::function<string()>& getName, const std::function<void(const string&)>& setName) const
{
   ImGui::NewLine();
   LiveUI::CenteredText(typeName);
   ImGui::BeginDisabled(m_table->m_liveBaseTable); // Do not edit name of live objects as it would break the script
   if (string name = getName(); ImGui::InputText("Name", &name))
      setName(name);
   ImGui::EndDisabled();
   ImGui::Separator();
}

void PropertyPane::EditableHeader(const string& typeName, IEditable* editable) const
{
   Header(typeName, [editable]() { return editable->GetName(); }, [editable](const string& v) { editable->SetName(v); });
}

void PropertyPane::Separator(const string& label) const
{
   ImGui::TableNextColumn();
   ImGui::TextUnformatted(label.c_str());
   // Underline the text
   const ImVec2 headerMin = ImGui::GetItemRectMin();
   const ImVec2 headerMax = ImGui::GetItemRectMax();
   ImDrawList* drawList = ImGui::GetWindowDrawList();
   const ImVec2 lineStart(headerMin.x, headerMax.y);
   const ImVec2 lineEnd(headerMax.x, headerMax.y);
   drawList->AddLine(lineStart, lineEnd, ImGui::GetColorU32(ImGuiCol_Text), 1.0f);

   if (m_sectionHasSync)
      ImGui::TableNextColumn();
}

bool PropertyPane::BeginSection(Section section)
{
   assert(!m_inSection);
   const char* name = section == Section::Visual ? "Visual"
      : section == Section::Lighting             ? "Lighting"
      : section == Section::Physics              ? "Physics"
      : section == Section::Position             ? "Position"
      : section == Section::Timer                ? "Timer"
      : section == Section::Transparency         ? "Transparency"
      : section == Section::Users                ? "Users"
                                                 : nullptr;
   const bool opened = ImGui::CollapsingHeader(name, ImGuiTreeNodeFlags_DefaultOpen);

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
   if (m_sectionHasSync)
   {
      ImGui::BeginTable("props", 2, ImGuiTableFlags_None);
      ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthStretch);
      ImGui::TableSetupColumn("Sync", ImGuiTableColumnFlags_WidthFixed);
   }
   else
   {
      ImGui::BeginTable("props", 1, ImGuiTableFlags_None);
      ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthStretch);
   }
   return true;
}

void PropertyPane::EndSection()
{
   assert(m_inSection);
   m_inSection = false;
   ImGui::EndTable();
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

string PropertyPane::GetUnitLabel(Unit unit)
{
   switch (unit)
   {
   case Unit::Inches: return "inches"s;
   case Unit::Millimeters: return "mm"s;
   case Unit::Degree: return "deg"s;
   case Unit::Percent: return "%"s;
   case Unit::PercentX100: return "%"s;
   case Unit::VPLength: return "vpu"s;
   case Unit::VPMass: return "vpu"s;
   case Unit::VPSpeed: return "vpu"s;
   default: return ""s;
   }
}


}