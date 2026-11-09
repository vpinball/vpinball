// license:GPLv3+

#include "core/stdafx.h"
#include "ReferenceImageOp.h"

#include "core/pinundo.h"
#include "core/player.h"
#include "core/SettingsService.h"
#include "parts/flasher.h"
#include "parts/PartGroup.h"
#include "parts/pintable.h"
#include "physics/PhysicsEngine.h"
#include "renderer/Renderer.h"
#include "renderer/Texture.h"
#include "ui/live/EditorUI.h"
#include "ui/live/FileDialog.h"
#include "ui/live/editor/EditorTheme.h"

namespace VPX::EditorUI
{

using namespace ReferenceImageMath;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Table level helpers
//

bool ReferenceImageOp::IsReferenceGroup(const PartGroup *group)
{
   for (; group != nullptr; group = group->GetPartGroup())
      if (StrCompareNoCase(group->GetName(), GroupName))
         return true;
   return false;
}

bool ReferenceImageOp::IsReferenceImage(const IEditable *part)
{
   return part != nullptr && part->GetItemType() == eItemFlasher && IsReferenceGroup(part->GetPartGroup());
}

bool ReferenceImageOp::IsLocked(const IEditable *part) { return part != nullptr && part->IsUILocked() && IsReferenceImage(part); }

PartGroup *ReferenceImageOp::FindGroup(const PinTable *table)
{
   // A root level group is preferred (the one created by Add), a nested one is used as is
   PartGroup *found = nullptr;
   for (IEditable *const part : table->GetParts())
      if (part->GetItemType() == eItemPartGroup && StrCompareNoCase(part->GetName(), GroupName) && (found == nullptr || part->GetPartGroup() == nullptr))
         found = static_cast<PartGroup *>(part);
   return found;
}

PartGroup *ReferenceImageOp::CreateGroup(PinTable *table)
{
   if (!table->IsNameUnique(GroupName))
      return nullptr;
   PartGroup *const group = PartGroup::COMCreate();
   group->Init(0.f, 0.f, false);
   group->m_name = GroupName;
   // No play mode renders the group (the editor views do not apply the mask). Playfield space, so that the parts are placed like the others.
   group->m_d.m_playerModeVisibilityMask = 0;
   group->m_d.m_spaceReference = PartGroupData::SpaceReference::SR_PLAYFIELD;
   table->AddPart(group);
   group->SetPartGroup(nullptr);
   return group;
}

Flasher *ReferenceImageOp::CreateFlasher(PinTable *table, PartGroup *group, const string &image, const Vertex2D &center, const float width, const float height)
{
   Flasher *const flasher = Flasher::COMCreate();
   flasher->Init(center.x, center.y, false); // Built-in defaults, not the user's default flasher properties
   flasher->m_name = table->GetUniqueName("RefImage"s);
   FlasherData &d = flasher->m_d;
   d.m_renderMode = FlasherData::RenderMode::FLASHER;
   d.m_imagealignment = ImageModeWrap; // Mapped over the bounds of the points (World would tie it to the table and distort it)
   d.m_szImageA = image;
   d.m_szImageB.clear();
   d.m_filter = Filter_None;
   d.m_filterAmount = 100;
   d.m_color = RGB(255, 255, 255); // Image colors as they are (a black color would not render)
   d.m_alpha = Opacity;
   d.m_intensity_scale = 1.f;
   d.m_addBlend = FlasherData::AB_NONE;
   d.m_displayTexture = true; // Also drawn in the Blueprint view and the solid / wireframe shadings
   d.m_isVisible = true;
   d.m_depthBias = 0.f;
   d.m_height = Height;
   d.m_rotX = d.m_rotY = d.m_rotZ = 0.f;
   flasher->m_desktopBackdrop = false;
   SetRect(flasher, { center, width, height });
   table->AddPart(flasher);
   flasher->SetPartGroup(group);
   return flasher;
}

Rect ReferenceImageOp::GetRect(const Flasher *flasher)
{
   vector<Vertex2D> points;
   for (const auto &point : flasher->m_curve.GetPoints())
      points.emplace_back(point->GetX(), point->GetY());
   return BoundsOf(points);
}

void ReferenceImageOp::SetRect(Flasher *flasher, const Rect &rect)
{
   DragPointCurve &curve = flasher->m_curve;
   curve.ClearPoints();
   for (const Vertex2D &corner : RectCorners(rect))
      curve.PushPoint(std::make_unique<DragPoint>(&curve, corner.x, corner.y, 0.f, false));
}

void ReferenceImageOp::SetSize(Flasher *flasher, const Vertex2D &size)
{
   const Rect rect = GetRect(flasher);
   SetRect(flasher, { rect.center, max(size.x, 0.1f), max(size.y, 0.1f) });
}

bool ReferenceImageOp::FitToImage(Flasher *flasher)
{
   const Texture *const image = flasher->GetPTable() ? flasher->GetPTable()->GetImage(flasher->m_d.m_szImageA) : nullptr;
   if (image == nullptr || image->m_width == 0 || image->m_height == 0)
      return false;
   Rect rect = GetRect(flasher);
   if (rect.width <= 0.f)
      rect.width = 100.f;
   SetRect(flasher, FitToAspect(rect, image->m_width, image->m_height));
   return true;
}

bool ReferenceImageOp::ReplaceImage(Flasher *flasher, const string &image)
{
   flasher->m_d.m_szImageA = image;
   return FitToImage(flasher);
}

PartGroup *ReferenceImageOp::PasteGroup(PinTable *table, PinUndo *undo, const IEditable *pasted, PartGroup *target)
{
   // The pasted data names the group of its source part (IEditable::m_onLoadExpectedPartGroup, filled by Load)
   if (pasted->GetItemType() != eItemFlasher || pasted->m_onLoadExpectedPartGroup.empty())
      return target;
   for (IEditable *const part : table->GetParts())
      if (part->GetItemType() == eItemPartGroup && StrCompareNoCase(part->GetName(), pasted->m_onLoadExpectedPartGroup))
         return IsReferenceGroup(static_cast<PartGroup *>(part)) ? static_cast<PartGroup *>(part) : target;
   if (!StrCompareNoCase(pasted->m_onLoadExpectedPartGroup, GroupName))
      return target;
   PartGroup *group = FindGroup(table);
   if (group == nullptr && (group = CreateGroup(table)) != nullptr)
   {
      if (undo != nullptr)
         undo->MarkForCreate(group);
      else
         group->Release(); // The table holds it
   }
   return group != nullptr ? group : target;
}

bool ReferenceImageOp::Rotate(IEditable *part, const float angle, const Vertex2D &center, const bool useElementCenter)
{
   if (!IsReferenceImage(part))
      return false;
   Flasher *const flasher = static_cast<Flasher *>(part);
   if (flasher->m_d.m_rotX != 0.f || flasher->m_d.m_rotY != 0.f)
      return true; // Tilted: not supported (see the declaration)
   if (!useElementCenter)
   {
      // Same rotation of the centre about the pivot as the points' (DragPointCurve::RotatePoints)
      const Vertex2D c = flasher->GetCenter();
      const float sn = sinf(ANGTORAD(angle)), cs = cosf(ANGTORAD(angle));
      const float dx = c.x - center.x, dy = c.y - center.y;
      flasher->m_curve.TranslatePoints(Vertex2D(center.x + cs * dx - sn * dy - c.x, center.y + cs * dy + sn * dx - c.y));
   }
   flasher->m_d.m_rotZ += angle; // The render applies the Z rotation first, about the centre: the same turn as rotating the points
   return true;
}

bool ReferenceImageOp::Scale(IEditable *part, const float scaleX, const float scaleY, const Vertex2D &center, const bool useElementCenter)
{
   if (!IsReferenceImage(part))
      return false;
   const float s = fabsf(scaleX - 1.f) >= fabsf(scaleY - 1.f) ? scaleX : scaleY;
   part->Scale(s, s, center, useElementCenter);
   return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Editor ops
//

bool ReferenceImageOp::CanAdd() const
{
   const EditorUI &ed = m_editor;
   return !ed.m_table->IsLocked() && !ed.IsInspectMode() && ed.m_pointEditPart == nullptr && ed.m_fileOp == nullptr;
}

vector<Flasher *> ReferenceImageOp::SelectedReferenceImages() const
{
   const EditorUI &ed = m_editor;
   vector<std::shared_ptr<EditorUIPart>> parts = ed.m_multiSel;
   if (parts.empty() && ed.m_selection.GetType() == Selection::S_EDITABLE && ed.m_selection.GetPart())
      parts.push_back(ed.m_selection.GetPart());
   vector<Flasher *> flashers;
   for (const auto &part : parts)
      if (part && IsReferenceImage(part->GetEditable()))
         flashers.push_back(static_cast<Flasher *>(part->GetEditable()));
   return flashers;
}

bool ReferenceImageOp::CanCreateIn(const PinTable *table) const { return FindGroup(table) != nullptr || table->IsNameUnique(GroupName); }

bool ReferenceImageOp::CanApplyToSelection() const { return CanAdd() && !SelectedReferenceImages().empty(); }

void ReferenceImageOp::OpenAddDialog()
{
   if (!CanAdd())
      return;
   m_dialog = Dialog();
   m_dialog.fromFile = true;
   m_openDialog = true;
}

void ReferenceImageOp::OpenReplaceDialog()
{
   const vector<Flasher *> flashers = SelectedReferenceImages();
   if (!CanAdd() || flashers.empty())
      return;
   m_dialog = Dialog();
   m_dialog.replace = true;
   m_dialog.fromFile = false;
   m_dialog.tableImage = flashers.back()->m_d.m_szImageA;
   m_openDialog = true;
}

void ReferenceImageOp::Add(const string &image, const float width, const float height)
{
   EditorUI &ed = m_editor;
   PinTable *const table = ed.m_table;
   PartGroup *group = FindGroup(table);
   const bool newGroup = group == nullptr;
   if (newGroup)
      group = CreateGroup(table);
   if (group == nullptr)
   {
      ed.Report("Add Reference Image: the name '"s + GroupName + "' is used by a part that is not a part group");
      return;
   }
   const Vertex2D center(0.5f * (table->m_left + table->m_right), 0.5f * (table->m_top + table->m_bottom));
   Flasher *const flasher = CreateFlasher(table, group, image, center, width, height);

   // Same player side setup as EditorUI::CreatePart
   ed.m_player->TimerSetup(flasher);
   flasher->RenderSetup(ed.m_renderer.get());
   ed.m_renderer->MarkStaticsDirty();
   ed.m_player->m_physics->Add(flasher);

   ed.m_undo.BeginUndo();
   if (newGroup)
      ed.m_undo.MarkForCreate(group);
   ed.m_undo.MarkForCreate(flasher);
   ed.m_undo.SetLabel("Add Reference Image: "s + image);
   ed.m_undo.EndUndo();

   ed.UpdateEditableList();
   if (const auto it = ed.m_editableMap.find(flasher); it != ed.m_editableMap.end())
   {
      ed.SetSelection(Selection(it->second));
      OutlinerPanel::m_anchor = it->second;
   }
   ed.Report("Added reference image "s + flasher->GetName() + " (" + image + ") to part group " + group->GetName() + ", hidden in play");
}

void ReferenceImageOp::Replace(const string &image)
{
   EditorUI &ed = m_editor;
   const vector<Flasher *> flashers = SelectedReferenceImages();
   const Texture *const tex = ed.m_table->GetImage(image);
   if (flashers.empty() || tex == nullptr || tex->m_width == 0 || tex->m_height == 0)
      return;
   ed.m_undo.BeginUndo();
   for (Flasher *const flasher : flashers)
      ed.m_undo.MarkForUndo(flasher);
   for (Flasher *const flasher : flashers)
   {
      ReplaceImage(flasher, image);
      ed.m_renderer->ReinitRenderable(flasher);
      ed.m_player->m_physics->Update(flasher);
   }
   ed.m_undo.SetLabel(flashers.size() == 1 ? "Replace Image: "s + flashers[0]->GetName() : "Replace Image: "s + std::to_string(flashers.size()) + " parts");
   ed.m_undo.EndUndo();
}

void ReferenceImageOp::FitSelection()
{
   EditorUI &ed = m_editor;
   if (!CanAdd())
      return;
   const vector<Flasher *> selected = SelectedReferenceImages();
   vector<Flasher *> flashers; // The ones with a valid image (the others would make an empty undo step)
   for (Flasher *const flasher : selected)
      if (const Texture *const image = ed.m_table->GetImage(flasher->m_d.m_szImageA); image != nullptr && image->m_width > 0 && image->m_height > 0)
         flashers.push_back(flasher);
   if (flashers.size() < selected.size())
      ed.Report("Fit to Image: " + std::to_string(selected.size() - flashers.size()) + " part(s) without a valid image");
   if (flashers.empty())
      return;
   ed.m_undo.BeginUndo();
   for (Flasher *const flasher : flashers)
      ed.m_undo.MarkForUndo(flasher);
   for (Flasher *const flasher : flashers)
   {
      FitToImage(flasher);
      ed.m_renderer->ReinitRenderable(flasher);
      ed.m_player->m_physics->Update(flasher);
   }
   ed.m_undo.SetLabel(flashers.size() == 1 ? "Fit to Image: "s + flashers[0]->GetName() : "Fit to Image: "s + std::to_string(flashers.size()) + " parts");
   ed.m_undo.EndUndo();
}

Texture *ReferenceImageOp::ImportFile(const string &file)
{
   PinTable *const table = m_editor.m_table;
   const std::filesystem::path path = PathFromUTF8(file);
   // ImportImage replaces an image of the same name: pick a free one
   const string base = TitleFromFilename(path);
   string name = base;
   for (int i = 2; table->GetImage(name) != nullptr; i++)
      name = base + '_' + std::to_string(i);
   Texture *const tex = table->ImportImage(path, name);
   if (tex != nullptr)
   {
      table->SetNonUndoableDirty(SaveDirtyState::Dirty); // Images are not part of the undo history (Images panel import)
      g_settingsService.GetAppSettings().SetRecentDir_ImageDir(PathToString(path.parent_path()), false);
   }
   return tex;
}

void ReferenceImageOp::PickedFile(const string &file)
{
   m_dialog.file = file;
   m_dialog.fileWidth = m_dialog.fileHeight = 0;
   m_dialog.error.clear();
   if (const std::shared_ptr<const BaseTexture> tex = BaseTexture::CreateFromFile(PathFromUTF8(file)))
   {
      m_dialog.fileWidth = tex->m_realWidth;
      m_dialog.fileHeight = tex->m_realHeight;
   }
   else
      m_dialog.error = "This file could not be read as an image";
}

void ReferenceImageOp::ImageSize(unsigned int &width, unsigned int &height) const
{
   width = height = 0;
   if (m_dialog.fromFile)
   {
      width = m_dialog.fileWidth;
      height = m_dialog.fileHeight;
   }
   else if (const Texture *const tex = m_editor.m_table->GetImage(m_dialog.tableImage))
   {
      width = tex->m_width;
      height = tex->m_height;
   }
}

void ReferenceImageOp::RenderModal()
{
   EditorUI &ed = m_editor;
   Dialog &dlg = m_dialog;
   if (m_pendingFile && !m_pendingFile->empty())
   {
      if (!m_pendingFile->front().empty()) // Empty when the file dialog was cancelled
         PickedFile(m_pendingFile->front());
      m_pendingFile = nullptr;
   }
   const char *const popupId = dlg.replace ? "Replace Reference Image###ReferenceImageDialog" : "Add Reference Image###ReferenceImageDialog";
   if (m_openDialog)
   {
      m_openDialog = false;
      ImGui::OpenPopup(popupId);
   }
   ImGui::SetNextWindowSize(ImVec2(ImGui::GetFontSize() * 30.f, 0.f), ImGuiCond_Appearing);
   if (!ImGui::BeginPopupModal(popupId, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
      return;

   ImGui::TextUnformatted(dlg.replace ? "Replace the image of the selected reference images (they are fitted to its aspect)"
                                      : "Add a scan, blueprint or photo to trace over (only shown in the editor, never in play)");
   ImGui::Separator();

   // Image source
   if (ImGui::RadioButton("Image file", dlg.fromFile))
      dlg.fromFile = true;
   ImGui::SameLine();
   if (ImGui::RadioButton("Image of the table", !dlg.fromFile))
      dlg.fromFile = false;
   if (dlg.fromFile)
   {
      const bool canBrowse = ed.m_player->m_playfieldWnd != nullptr && m_pendingFile == nullptr;
      ImGui::BeginDisabled(!canBrowse);
      if (ImGui::Button("Browse..."))
      {
         m_pendingFile = std::make_shared<vector<string>>();
         const SDL_DialogFileFilter filters[] = { { "Image Files", "bmp;jpg;jpeg;png;tga;webp;exr;hdr" } };
         const string dir = PathToUTF8(PathFromString(g_settingsService.GetAppSettings().GetRecentDir_ImageDir())); // SDL expects UTF-8
         ShowOpenFileDialog(
            [](void *userdata, const char *const *filelist, int filter)
            {
               auto *res = static_cast<std::shared_ptr<vector<string>> *>(userdata);
               (**res).push_back((filelist != nullptr && filelist[0] != nullptr) ? filelist[0] : ""); // Always answers, so that the dialog can be opened again
               delete res;
            },
            new std::shared_ptr<vector<string>>(m_pendingFile), ed.m_player->m_playfieldWnd->GetCore(), filters, 1, dir.empty() ? nullptr : dir.c_str(), false);
      }
      ImGui::EndDisabled();
      ImGui::SameLine();
      ImGui::TextUnformatted(dlg.file.empty() ? "(no file)" : PathToUTF8(PathFromUTF8(dlg.file).filename()).c_str());
   }
   else
   {
      ImGui::SetNextItemWidth(-FLT_MIN);
      if (ImGui::BeginCombo("##TableImage", dlg.tableImage.empty() ? "(choose an image)" : dlg.tableImage.c_str()))
      {
         for (const Texture *const tex : ed.m_table->GetImageList())
            if (ImGui::Selectable(tex->m_name.c_str(), tex->m_name == dlg.tableImage))
               dlg.tableImage = tex->m_name;
         ImGui::EndCombo();
      }
   }
   unsigned int pxWidth, pxHeight;
   ImageSize(pxWidth, pxHeight);
   if (pxWidth > 0 && pxHeight > 0)
      ImGui::TextColored(EditorTheme::V(EditorTheme::TextDim), "%u x %u pixels", pxWidth, pxHeight);
   if (!dlg.error.empty())
      ImGui::TextColored(EditorTheme::V(EditorTheme::Accent), "%s", dlg.error.c_str());

   // Real size (Add only: Replace keeps the part's width)
   const bool hasImage = pxWidth > 0 && pxHeight > 0;
   if (!dlg.replace)
   {
      // A newly picked image starts at the playfield width, at the image's aspect
      const string imageKey = dlg.fromFile ? dlg.file : dlg.tableImage;
      if (hasImage && imageKey != dlg.sizedFor)
      {
         dlg.sizedFor = imageKey;
         dlg.width = FromVPU(ed.m_table->m_right - ed.m_table->m_left, dlg.unit);
         dlg.height = HeightForWidth(dlg.width, pxWidth, pxHeight);
      }
      ImGui::Separator();
      ImGui::TextUnformatted("Real size of the image");
      ImGui::SetNextItemWidth(ImGui::GetFontSize() * 7.f);
      int unit = dlg.unit == Unit::Inches ? 1 : 0;
      if (ImGui::Combo("Unit", &unit, "cm\0inches\0"))
      {
         const Unit newUnit = unit == 1 ? Unit::Inches : Unit::Centimeters;
         dlg.width = FromVPU(ToVPU(dlg.width, dlg.unit), newUnit);
         dlg.height = FromVPU(ToVPU(dlg.height, dlg.unit), newUnit);
         dlg.unit = newUnit;
      }
      const char *const format = dlg.unit == Unit::Inches ? "%.3f" : "%.2f";
      ImGui::SetNextItemWidth(ImGui::GetFontSize() * 7.f);
      if (ImGui::InputFloat("Width", &dlg.width, 0.f, 0.f, format) && hasImage)
         dlg.height = HeightForWidth(dlg.width, pxWidth, pxHeight); // The height follows the image's aspect, it can be changed afterwards
      ImGui::SetNextItemWidth(ImGui::GetFontSize() * 7.f);
      ImGui::InputFloat("Height", &dlg.height, 0.f, 0.f, format);
      ImGui::SetNextItemWidth(ImGui::GetFontSize() * 7.f);
      if (ImGui::InputFloat("Scan resolution (DPI)", &dlg.dpi, 0.f, 0.f, "%.0f") && hasImage && dlg.dpi > 0.f)
      {
         dlg.width = FromVPU(SizeFromDpi(pxWidth, dlg.dpi), dlg.unit);
         dlg.height = FromVPU(SizeFromDpi(pxHeight, dlg.dpi), dlg.unit);
      }
      if (ImGui::IsItemHovered())
         ImGui::SetTooltip("Optional: the size of a scan follows from its resolution");
      if (hasImage && IsAspectMismatch(dlg.width, dlg.height, pxWidth, pxHeight))
         ImGui::TextColored(EditorTheme::V(EditorTheme::Accent), "The size ratio differs from the image's by %.1f%%: the image will be stretched",
            100.f * AspectMismatch(dlg.width, dlg.height, pxWidth, pxHeight));
   }

   ImGui::Separator();
   const bool sizeOk = dlg.replace || (dlg.width > 0.f && dlg.height > 0.f);
   ImGui::BeginDisabled(!hasImage || !sizeOk || !CanAdd());
   if (ImGui::Button(dlg.replace ? "Replace" : "Add"))
   {
      string image;
      if (!dlg.replace && !CanCreateIn(ed.m_table))
         dlg.error = "The name '"s + GroupName + "' is used by a part that is not a part group";
      else if (dlg.fromFile)
      {
         if (const Texture *const tex = ImportFile(dlg.file))
            image = tex->m_name;
         else
            dlg.error = "Failed to import '"s + dlg.file + '\'';
      }
      else
         image = dlg.tableImage;
      if (!image.empty())
      {
         if (dlg.replace)
            Replace(image);
         else
            Add(image, ToVPU(dlg.width, dlg.unit), ToVPU(dlg.height, dlg.unit));
         ImGui::CloseCurrentPopup();
      }
   }
   ImGui::EndDisabled();
   ImGui::SameLine();
   if (ImGui::Button("Cancel"))
      ImGui::CloseCurrentPopup();
   ImGui::EndPopup();
}

}
