#include "core/stdafx.h"
#include "PropertyPane.h"

#include "EditorTheme.h"
#include "core/extern.h"
#include "core/player.h"
#include "core/SettingsService.h"
#include "imgui/imgui_stdlib.h"
#include "ui/live/EditorUI.h"
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
   NameField(getName, setName);
   ImGui::Separator();
}

void PropertyPane::NameField(const std::function<string()>& getName, const std::function<void(const string&)>& setName)
{
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
}

void PropertyPane::EditableHeader(const string& /*typeName*/, IEditable* editable)
{
   EditorUI& editor = g_pplayer->m_liveUI->m_editorUI;
   IEditable* const displayed = GetEditedPart<IEditable>(editable);
   const bool locked = displayed->IsUILocked();
   HeaderRow(
      editor.PartIcon(displayed), editable->GetItemType() == eItemPartGroup ? ICON_LC_FOLDER : ICON_LC_TAG, editable->GetPartGroup(), editable, [editable]() { return editable->GetName(); },
      [editable](const string& v) { editable->SetName(v); }, locked,
      [this, displayed, locked]()
      {
         // Same semantics as the context menu's Lock command (and the win32 editor): if any of the edited
         // parts is locked, unlock them all, otherwise lock them all
         bool anyLocked = locked;
         for (IEditable* const part : m_coEdited)
            if (IEditable* const target = GetEditedPart<IEditable>(part); target != nullptr && target->IsUILocked())
               anyLocked = true;
         displayed->SetUILock(!anyLocked);
         for (IEditable* const part : m_coEdited)
            if (IEditable* const target = GetEditedPart<IEditable>(part); target != nullptr)
               target->SetUILock(!anyLocked);
      },
      // Do not edit name of live objects as it would break the script, nor of multi selections (no bulk renaming)
      m_table->m_liveBaseTable || IsMultiEdit());
}

void PropertyPane::TableHeader(PinTable* const table)
{
   // Same header as the parts': the picker's icon is the application's one and its popup lists the root
   // level parts (the table acts as the root group), the toggle locks the table itself
   HeaderRow(
      g_pplayer->m_liveUI->m_editorUI.PartIcon(table), ICON_LC_TAG, nullptr, nullptr, //
      [table]() { return table->GetName(); }, //
      [table](const string& v) { table->SetName(v); }, //
      table->IsLocked(), [table]() { table->ToggleLock(); }, //
      m_table->m_liveBaseTable);
}

void PropertyPane::HeaderRow(const std::shared_ptr<Sampler>& icon, const char* const iconGlyph, PartGroup* const siblingParent, IEditable* const currentSibling,
   const std::function<string()>& getName, const std::function<void(const string&)>& setName, const bool locked, const std::function<void()>& toggleLock, const bool nameDisabled)
{
   EditorUI& editor = g_pplayer->m_liveUI->m_editorUI;
   const ImVec2 linePos = ImGui::GetCursorScreenPos();
   const float lineWidth = ImGui::GetContentRegionAvail().x;
   const ImGuiStyle& style = ImGui::GetStyle();
   const char* const lockIcon = locked ? ICON_LC_LOCK : ICON_LC_LOCK_OPEN;
   const ImVec2 lockSize = IconButtonInkSize(lockIcon);
   const float frameHeight = ImGui::GetFrameHeight();
   // A part's asset icon (the same as the outliner's and the part picker's), a glyph when it has none
   const auto drawIcon = [](ImDrawList* const drawList, const std::shared_ptr<Sampler>& icon, const char* const glyph, const ImVec2& min, const float side)
   {
      if (icon != nullptr)
         drawList->AddImage(icon, min, ImVec2(min.x + side, min.y + side));
      else
         drawList->AddText(min, ImGui::GetColorU32(ImGuiCol_TextDisabled), glyph);
   };

   // Header line: the part picker styled like the panes' editor type selector (a button with the type icon
   // and a small chevron, opening a popup that lists the parts, each with its type icon before its name),
   // the name field filling the remaining width, and the lock toggle at the right end
   const float iconSide = ImGui::GetTextLineHeight();
   const float chevronWidth = ImGui::GetFontSize() * 0.4f; // Small, drawn in TextDim
   const float comboWidth = style.FramePadding.x * 2.f + iconSide + style.ItemInnerSpacing.x + chevronWidth;
   const float nameX = linePos.x + comboWidth + style.ItemSpacing.x;
   const float nameWidth = max(60.f, linePos.x + lineWidth - lockSize.x - style.ItemSpacing.x - nameX);
   ImGui::SetCursorScreenPos(linePos);
   const bool pickerClicked = ImGui::Button("##Siblings", ImVec2(comboWidth, frameHeight));
   const ImVec2 pickerMin = ImGui::GetItemRectMin();
   const ImVec2 pickerMax = ImGui::GetItemRectMax();
   {
      ImDrawList* const drawList = ImGui::GetWindowDrawList();
      drawIcon(drawList, icon, iconGlyph, ImVec2(pickerMin.x + style.FramePadding.x, pickerMin.y + (frameHeight - iconSide) * 0.5f), iconSide);
      const float half = chevronWidth * 0.5f;
      const float centerX = pickerMax.x - style.FramePadding.x - half;
      const float centerY = (pickerMin.y + pickerMax.y) * 0.5f;
      drawList->AddTriangleFilled(ImVec2(centerX - half, centerY - half * 0.5f), ImVec2(centerX + half, centerY - half * 0.5f), ImVec2(centerX, centerY + half * 0.5f), EditorTheme::TextDim);
   }
   ImGui::SetItemTooltip(currentSibling != nullptr ? "Select a part of the same group" : "Select a part");
   if (pickerClicked)
      ImGui::OpenPopup("##SiblingParts");
   ImGui::SetNextWindowPos(ImVec2(pickerMin.x, pickerMax.y), ImGuiCond_Always);
   if (ImGui::BeginPopup("##SiblingParts"))
   {
      // The popup auto sizes to its widest row: make the rows wide enough for the icon column and the widest name
      float rowWidth = frameHeight;
      for (IEditable* const sibling : m_table->GetParts())
         if (sibling->GetPartGroup() == siblingParent && editor.m_editableMap.contains(sibling))
            rowWidth = max(rowWidth, style.FramePadding.x * 2.f + frameHeight + ImGui::CalcTextSize(sibling->GetName().c_str()).x);
      for (IEditable* const sibling : m_table->GetParts())
      {
         if (sibling->GetPartGroup() != siblingParent)
            continue;
         const auto it = editor.m_editableMap.find(sibling);
         if (it == editor.m_editableMap.end())
            continue;
         ImGui::PushID(sibling);
         if (ImGui::Selectable("##s", sibling == currentSibling, 0, ImVec2(rowWidth, frameHeight)))
            editor.SetSelection(Selection(it->second));
         const ImVec2 itemMin = ImGui::GetItemRectMin();
         ImDrawList* const drawList = ImGui::GetWindowDrawList();
         drawIcon(drawList, editor.PartIcon(sibling), sibling->GetItemType() == eItemPartGroup ? ICON_LC_FOLDER : ICON_LC_TAG,
            ImVec2(itemMin.x + style.FramePadding.x, itemMin.y + style.FramePadding.y), frameHeight - style.FramePadding.y * 2.f);
         drawList->AddText(ImVec2(itemMin.x + style.FramePadding.x + frameHeight, itemMin.y + style.FramePadding.y), ImGui::GetColorU32(ImGuiCol_Text), sibling->GetName().c_str());
         ImGui::PopID();
      }
      ImGui::EndPopup();
   }

   // Lock toggle at the right end (pressed style like the viewport's orbit lock when locked)
   ImGui::SetCursorScreenPos(ImVec2(linePos.x + lineWidth - lockSize.x, linePos.y));
   m_modifyFieldId++;
   if (locked)
      ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
   if (IconButtonInk(lockIcon, "##PartLock"))
   {
      toggleLock();
      m_modified = m_modifyFieldId;
   }
   if (locked)
      ImGui::PopStyleColor();
   ImGui::SetItemTooltip(locked ? "Unlock" : "Lock (protect from edition)");

   // Name field filling the width between the picker and the lock toggle. A locked part can not be
   // modified: disable it and the pane's fields, closed by EndEditScope (the picker and the lock
   // toggle above stay enabled so that the part can be switched to or unlocked)
   ImGui::SetCursorScreenPos(ImVec2(nameX, linePos.y));
   if (locked)
   {
      ImGui::BeginDisabled();
      m_lockScope = true;
   }
   m_modifyFieldId++;
   ImGui::BeginDisabled(nameDisabled);
   ImGui::SetNextItemWidth(nameWidth);
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
   const std::filesystem::path path = PathFromUTF8(file);

   // A file identical to an image the table already holds is not imported again: the field links to the existing image
   Texture* const loaded = Texture::CreateFromFile(path);
   if (loaded == nullptr)
      return;
   Texture* const tex = m_table->ImportImage(loaded);
   g_settingsService.GetAppSettings().SetRecentDir_ImageDir(PathToString(path.parent_path()), false);
   if (link)
      link(*this, tex->m_name);
   if (tex != loaded)
   {
      if (g_pplayer != nullptr && g_pplayer->m_liveUI != nullptr)
         g_pplayer->m_liveUI->PushNotification("'" + PathToUTF8(path.filename()) + "' is a duplicate of image '"s + tex->m_name + "': linked to the existing image"s, 8000);
   }
   else
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
