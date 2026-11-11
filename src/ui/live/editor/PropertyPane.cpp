#include "core/stdafx.h"
#include "PropertyPane.h"

#include "core/extern.h"
#include "core/player.h"
#include "core/SettingsService.h"
#include "imgui/imgui_stdlib.h"
#include "ui/live/FileDialog.h"
#include "ui/live/LiveUI.h"

namespace VPX::EditorUI
{
namespace
{
   // Result slot and field link of an asynchronous image import requested from an image field
   std::shared_ptr<vector<string>> s_imageFieldImportFile;
   std::function<void(PropertyPane&, const string&)> s_imageFieldImportLink;
}

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

void PropertyPane::PropertyLabel(const string& label, const char* tooltip)
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
         ellipsis = trim_string(label.substr(0, n)) + "...";
         textWidth = ImGui::CalcTextSize(ellipsis.c_str()).x;
      }
      ImGui::SetCursorScreenPos(ImVec2(pos.x + max(0.0f, xWidth - textWidth - xSpacing), pos.y + ImGui::GetStyle().FramePadding.y));
      ImGui::TextUnformatted(ellipsis.c_str());
      if (tooltip != nullptr)
         ImGui::SetItemTooltip("%s\n%s", label.c_str(), tooltip);
      else
         ImGui::SetItemTooltip("%s", label.c_str());
   }
   else
   {
      ImGui::SetCursorScreenPos(ImVec2(pos.x + xWidth - textWidth - xSpacing, pos.y + ImGui::GetStyle().FramePadding.y));
      ImGui::TextUnformatted(label.c_str());
      if (tooltip != nullptr)
         ImGui::SetItemTooltip("%s", tooltip);
   }

   ImGui::SetCursorScreenPos(ImVec2(pos.x + xWidth, pos.y));
   ImGui::SetNextItemWidth(xWidth);
   m_syncPos = ImVec2(pos.x + xWidth + xWidth, pos.y);
}

void PropertyPane::Header(const string& typeName, const std::function<string()>& getName, const std::function<void(const string&)>& setName)
{
   ImGui::NewLine();
   LiveUI::CenteredText(IsMultiEdit() ? typeName + " x" + std::to_string(GetEditCount()) : typeName);
   m_modifyFieldId++;
   // Do not edit name of live objects as it would break the script, nor of multi selections (no bulk renaming)
   ImGui::BeginDisabled(m_table->m_liveBaseTable || IsMultiEdit());
   PropertyLabel("Name"s);
   const string current = getName();
   string name = current;
   // Apply on Enter only: renaming on each keystroke would make each intermediate name unique, corrupting the name being typed
   if (ImGui::InputText("##Name", &name, ImGuiInputTextFlags_EnterReturnsTrue) && name != current)
   {
      setName(name);
      if (getName() != current) // The setter may reject or adjust the name
         m_modified = m_modifyFieldId; // Makes the rename undoable and marks the table as modified
   }
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
   PollImageFieldImport();
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

bool PropertyPane::CanImportImageField() const { return s_imageFieldImportFile == nullptr && g_pplayer != nullptr && g_pplayer->m_playfieldWnd != nullptr; }

void PropertyPane::RequestImageFieldImport(const std::function<void(PropertyPane&, const string&)>& link)
{
   if (!CanImportImageField())
      return;
   s_imageFieldImportLink = link;
   s_imageFieldImportFile = std::make_shared<vector<string>>();
   const SDL_DialogFileFilter filters[] = { { "Image Files", "bmp;jpg;jpeg;png;tga;webp;exr;hdr" } };
   const string dir = PathToUTF8(PathFromString(g_settingsService.GetAppSettings().GetRecentDir_ImageDir())); // SDL expects UTF-8
   ShowOpenFileDialog(
      [](void* userdata, const char* const* filelist, int filter)
      {
         auto* res = static_cast<std::shared_ptr<vector<string>>*>(userdata);
         (**res).push_back((filelist != nullptr && filelist[0] != nullptr) ? filelist[0] : ""); // Always answers, so that the dialog can be opened again
         delete res;
      },
      new std::shared_ptr<vector<string>>(s_imageFieldImportFile), g_pplayer->m_playfieldWnd->GetCore(), filters, 1, dir.empty() ? nullptr : dir.c_str(), false);
}

void PropertyPane::PollImageFieldImport()
{
   if (s_imageFieldImportFile == nullptr || s_imageFieldImportFile->empty())
      return;
   const string file = std::move(s_imageFieldImportFile->front());
   const std::function<void(PropertyPane&, const string&)> link = std::move(s_imageFieldImportLink);
   s_imageFieldImportFile = nullptr;
   s_imageFieldImportLink = nullptr;
   if (file.empty()) // Cancelled dialog
      return;
   Texture* const tex = m_table->ImportImage(PathFromUTF8(file), ""s);
   if (tex == nullptr)
      return;
   g_settingsService.GetAppSettings().SetRecentDir_ImageDir(PathToString(PathFromUTF8(file).parent_path()), false);
   if (link)
      link(*this, tex->m_name);
   m_table->SetNonUndoableDirty(SaveDirtyState::Dirty);
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

void PropertyPane::ResolveUnit(Unit from, Unit& to, int& nDecimalAdjust)
{
   float value = 0.f;
   ConvertUnit(from, to, value, nDecimalAdjust);
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
