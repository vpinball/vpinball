// license:GPLv3+

#include "core/stdafx.h"
#include "AuditDialog.h"

#include "core/editablereg.h"
#include "core/player.h"
#include "parts/Material.h"
#include "parts/pintable.h"
#include "parts/Sound.h"
#include "physics/PhysicsEngine.h"
#include "renderer/Texture.h"
#include "ui/live/IconsLucide.h"
#include "ui/live/LiveUI.h"
#include "EditorTheme.h"
#include "ScriptPanel.h"
#include "ViewportPane.h"

namespace VPX::EditorUI
{

namespace
{
   struct SeverityStyle
   {
      const char *icon;
      ImVec4 color;
   };
   SeverityStyle SeverityIcon(const TableAuditItem::Severity severity)
   {
      switch (severity)
      {
      case TableAuditItem::Severity::Error: return { ICON_LC_X, ImVec4(0.90f, 0.30f, 0.30f, 1.f) };
      case TableAuditItem::Severity::Warning: return { ICON_LC_TRIANGLE_ALERT, ImVec4(0.90f, 0.70f, 0.20f, 1.f) };
      default: return { ICON_LC_CIRCLE, EditorTheme::V(EditorTheme::TextDim) };
      }
   }
   const char *SeverityName(const TableAuditItem::Severity severity)
   {
      switch (severity)
      {
      case TableAuditItem::Severity::Error: return "Error";
      case TableAuditItem::Severity::Warning: return "Warning";
      default: return "Info";
      }
   }
}

AuditDialog::AuditDialog(EditorUI &editor)
   : m_editor(editor)
{
}

void AuditDialog::Show()
{
   m_visible = true;
   RunAudit();
}

void AuditDialog::Close() { m_visible = false; }

void AuditDialog::OnTableSwitch()
{
   // The items hold handles of the previous table's parts and assets: re-audit the new table
   if (m_visible)
      RunAudit();
}

VPX::SoundSpec AuditDialog::GetSoundInformations(const VPX::Sound &sound)
{
   if (const auto it = m_soundSpecs.find(&sound); it != m_soundSpecs.end() && it->second.dataVersion == sound.GetDataVersion() && it->second.fileSize == sound.GetFileSize())
      return it->second.specs;
   const VPX::SoundSpec specs = m_editor.m_player->m_audioPlayer->GetSoundInformations(&sound);
   m_soundSpecs[&sound] = { sound.GetDataVersion(), sound.GetFileSize(), specs };
   return specs;
}

bool AuditDialog::IsAuditStale() const
{
   const PinTable &table = *m_editor.m_table;
   return m_editor.m_undo.TopRecordId() != m_auditUndoId || table.m_vimage != m_auditImages || table.m_vsound != m_auditSounds || table.m_materials != m_auditMaterials;
}

void AuditDialog::RunAudit()
{
   const int selection = m_selected;
   PinTable &table = *m_editor.m_table;
   // Drop the specifications of deleted sounds (a new sound may get the same address)
   std::erase_if(m_soundSpecs, [&table](const auto &entry) { return std::ranges::find(table.m_vsound, entry.first) == table.m_vsound.end(); });
   // Use the running audio player, instead of opening the audio devices on each audit
   if (m_editor.m_player->m_audioPlayer)
      m_audit = TableAudit::Run(table, [this](const VPX::Sound &sound) { return GetSoundInformations(sound); });
   else
      m_audit = TableAudit::Run(table);
   m_auditUndoId = m_editor.m_undo.TopRecordId();
   m_auditImages = table.m_vimage;
   m_auditSounds = table.m_vsound;
   m_auditMaterials = table.m_materials;
   // Keep the selection near its previous index (the list is only stable in content between refreshes)
   m_selected = m_audit.items.empty() ? -1 : clamp(selection, 0, static_cast<int>(m_audit.items.size()) - 1);
   m_scrollToSelected = true;
}

void AuditDialog::SelectItem(const int index)
{
   m_selected = index;
   m_scrollToSelected = true;
   if (index < 0 || index >= static_cast<int>(m_audit.items.size()))
      return;
   const TableAuditItem &item = m_audit.items[index];
   EditorUI &editor = m_editor;
   if (item.part != nullptr)
   {
      if (item.part->GetItemType() == eItemTable)
         editor.ClearSelection(); // The properties of the table itself are shown when nothing is selected
      else
      {
         editor.UpdateEditableList();
         if (const auto it = editor.m_editableMap.find(item.part); it != editor.m_editableMap.end())
            editor.SetSelection(Selection(it->second));
      }
   }
   else if (item.image != nullptr)
      editor.SetSelection(Selection(item.image));
   else if (item.sound != nullptr)
      editor.SetSelection(Selection(item.sound));
   else if (item.material != nullptr)
      editor.SetSelection(Selection(item.material));
   else if (item.scriptLine >= 0)
      editor.m_scriptPanel.ShowLine(item.scriptLine);
}

void AuditDialog::RevealSubject(const TableAuditItem &item)
{
   SelectItem(m_selected); // Apply the subject selection
   // Zoom the target viewport on the subject part (the table itself has nothing to frame)
   if (item.part != nullptr && item.part->GetItemType() != eItemTable)
      if (ViewportPane *const vp = m_editor.GetTargetViewport(); vp != nullptr)
         vp->FrameSelection();
}

void AuditDialog::ApplyFix(TableAuditItem &item)
{
   if (item.fix == nullptr || m_editor.m_table->IsLocked() || m_editor.IsInspectMode())
      return;
   EditorUI &editor = m_editor;
   const string fixLabel = item.fixLabel; // The item list is refreshed by the fix
   editor.m_undo.BeginUndo();
   IEditable *const created = item.fix(*editor.m_table, editor.m_undo);
   editor.m_undo.SetLabel("Audit: "s + fixLabel);
   editor.m_undo.EndUndo();
   if (item.fixFailure && !item.fixFailure->empty())
   {
      editor.m_undo.Discard(); // Nothing was changed: no undo record, no dirty table
      editor.Report(*item.fixFailure);
      return;
   }
   if (created != nullptr)
   {
      // Same player side setup as CreatePart (the fix only registers the part with the table)
      editor.m_player->TimerSetup(created);
      if (auto *const renderable = created->GetIRenderable(); renderable)
      {
         renderable->RenderSetup(editor.m_renderer.get());
         editor.m_renderer->MarkStaticsDirty();
      }
      if (created->GetIHitable())
         editor.m_player->m_physics->Add(created);
      editor.UpdateEditableList();
      if (const auto it = editor.m_editableMap.find(created); it != editor.m_editableMap.end())
         editor.SetSelection(Selection(it->second));
   }
   else
      editor.m_renderer->MarkStaticsDirty(); // Field fixes may affect the static prerender (e.g. translucency)
   editor.Report("Audit fix applied: "s + fixLabel);
   RunAudit();
}

void AuditDialog::Render()
{
   if (!m_visible)
      return;

   // The items hold raw part/asset handles: re-audit when the table changed outside this dialog, through
   // the undo history (edits, part deletes, undo/redo) or by an asset add/delete (not undoable)
   if (IsAuditStale())
      RunAudit();

   const float dpi = m_editor.m_liveUI.GetDPI();
   ImGui::SetNextWindowSize(ImVec2(620.f * dpi, 400.f * dpi), ImGuiCond_Appearing);
   if (!ImGui::Begin("Table Audit & Fix", &m_visible))
   {
      ImGui::End();
      return;
   }

   int errors = 0, warnings = 0, infos = 0;
   for (const TableAuditItem &item : m_audit.items)
      switch (item.severity)
      {
      case TableAuditItem::Severity::Error: ++errors; break;
      case TableAuditItem::Severity::Warning: ++warnings; break;
      default: ++infos; break;
      }
   ImGui::TextDisabled("%d error%s  ·  %d warning%s  ·  %d info", errors, errors == 1 ? "" : "s", warnings, warnings == 1 ? "" : "s", infos);
   ImGui::SameLine(ImGui::GetContentRegionAvail().x + ImGui::GetCursorPosX() - ImGui::CalcTextSize("Refresh").x - ImGui::GetStyle().FramePadding.x * 2.f);
   if (ImGui::SmallButton("Refresh"))
      RunAudit();
   ImGui::Separator();

   // Finding list: a severity-colored row per item; Up/Down navigates while the list is focused
   const float detailsHeight = 140.f * dpi;
   ImGui::BeginChild("##AuditItems", ImVec2(0.f, -detailsHeight), ImGuiChildFlags_Borders);
   const int count = static_cast<int>(m_audit.items.size());
   for (int i = 0; i < count; ++i)
   {
      const TableAuditItem &item = m_audit.items[i];
      ImGui::PushID(i);
      const SeverityStyle style = SeverityIcon(item.severity);
      ImGui::PushStyleColor(ImGuiCol_Text, style.color);
      const string label = string(style.icon) + "  " + item.summary;
      if (ImGui::Selectable(label.c_str(), m_selected == i))
         SelectItem(i);
      ImGui::PopStyleColor();
      if (m_scrollToSelected && m_selected == i)
      {
         ImGui::SetScrollHereY();
         m_scrollToSelected = false;
      }
      ImGui::PopID();
   }
   ImGui::EndChild();

   if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && count > 0)
   {
      if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))
         SelectItem(m_selected < 0 ? 0 : min(m_selected + 1, count - 1));
      else if (ImGui::IsKeyPressed(ImGuiKey_UpArrow))
         SelectItem(m_selected < 0 ? count - 1 : max(m_selected - 1, 0));
   }

   // Details of the selected item: severity, subject, explanation and the available actions
   ImGui::BeginChild("##AuditDetails", ImVec2(0.f, 0.f), ImGuiChildFlags_Borders);
   const TableAuditItem *const item = m_selected >= 0 && m_selected < count ? &m_audit.items[m_selected] : nullptr;
   if (item == nullptr)
      ImGui::TextDisabled("No audit item selected.");
   else
   {
      string subject;
      if (item->part != nullptr)
         subject = item->part->GetItemType() == eItemTable ? "Table"s
                                                           : LocalString(EditableRegistry::GetTypeNameStringID(item->part->GetItemType())).m_szbuffer + " '"s + item->part->GetName() + "'"s;
      else if (item->image != nullptr)
         subject = "Image '"s + item->image->m_name + "'"s;
      else if (item->sound != nullptr)
         subject = "Sound '"s + item->sound->GetName() + "'"s;
      else if (item->material != nullptr)
         subject = "Material '"s + item->material->m_name + "'"s;
      else if (item->scriptLine >= 0)
         subject = "Script, line "s + std::to_string(item->scriptLine);
      else
         subject = "Table"s;
      const SeverityStyle style = SeverityIcon(item->severity);
      ImGui::TextColored(style.color, "%s %s", style.icon, SeverityName(item->severity));
      ImGui::SameLine();
      ImGui::TextDisabled("·  %s", subject.c_str());
      ImGui::PushTextWrapPos(0.f);
      ImGui::TextUnformatted(item->details.c_str());
      ImGui::PopTextWrapPos();

      ImGui::Spacing();
      EditorUI &editor = m_editor;
      const bool canFix = item->fix != nullptr && !editor.m_table->IsLocked() && !editor.IsInspectMode();
      if (item->fix != nullptr)
      {
         ImGui::BeginDisabled(!canFix);
         const bool applyFix = ImGui::Button((ICON_LC_ZAP " " + item->fixLabel).c_str());
         ImGui::EndDisabled();
         if (applyFix)
         {
            ApplyFix(m_audit.items[m_selected]); // Rebuilds the item list: 'item' is dangling below, bail out
            ImGui::EndChild();
            ImGui::End();
            return;
         }
         if (!canFix && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
            ImGui::SetTooltip("%s", editor.m_table->IsLocked() ? "The table is locked" : "Fixes are not available while inspecting a live table");
         if (item->part != nullptr || item->scriptLine >= 0)
            ImGui::SameLine();
      }
      if (item->part != nullptr && item->part->GetItemType() != eItemTable && ImGui::Button(ICON_LC_VIDEO " Frame part"))
         RevealSubject(*item);
      else if (item->scriptLine >= 0 && ImGui::Button(ICON_LC_CODE_XML " Go to script line"))
         editor.m_scriptPanel.ShowLine(item->scriptLine);
   }
   ImGui::EndChild();

   ImGui::End();
}

}
