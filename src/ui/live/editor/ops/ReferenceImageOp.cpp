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
#include "ui/live/LiveUI.h"
#include "ui/live/editor/EditorTheme.h"

namespace VPX::EditorUI
{

using namespace ReferenceImageMath;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Table level helpers
//

bool ReferenceImageOp::HasReferenceImageSetup(const Flasher *flasher)
{
   // Editor only visual setup: a textured flasher, in its simple render mode, with a single image
   // and no lightmap or controller state source modulating it
   const FlasherData &d = flasher->m_d;
   return d.m_displayTexture && d.m_renderMode == FlasherData::RenderMode::FLASHER && !d.m_szImageA.empty() && d.m_szImageB.empty() && d.m_szLightmap.empty() && d.m_stateSrcLink.empty();
}

bool ReferenceImageOp::IsReferenceImage(const IEditable *part, const EditorUI *editor)
{
   if (editor != nullptr)
      if (const auto it = editor->m_editableMap.find(part); it != editor->m_editableMap.end())
         return IsReferenceImage(it->second.get());
   const Flasher *const flasher = part != nullptr && part->GetItemType() == eItemFlasher ? static_cast<const Flasher *>(part) : nullptr;
   return flasher != nullptr && !flasher->m_d.m_isVisible && HasReferenceImageSetup(flasher);
}

bool ReferenceImageOp::IsReferenceImage(const EditorUIPart *uiPart)
{
   // For a part driven by the editor, m_d carries a display value: the authored visibility is held by its UI part
   return uiPart != nullptr && uiPart->GetEditable()->GetItemType() == eItemFlasher && !uiPart->IsAuthoredVisible()
      && HasReferenceImageSetup(static_cast<const Flasher *>(uiPart->GetEditable()));
}

bool ReferenceImageOp::IsLocked(const IEditable *part, const EditorUI *editor) { return part != nullptr && part->IsUILocked() && IsReferenceImage(part, editor); }

bool ReferenceImageOp::IsLocked(const EditorUIPart *uiPart) { return uiPart != nullptr && uiPart->GetEditable()->IsUILocked() && IsReferenceImage(uiPart); }

Flasher *ReferenceImageOp::CreateFlasher(PinTable *table, const string &image, const Vertex2D &center, const float width, const float height)
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
   d.m_isVisible = false; // Never rendered in play: shown in the editor instead (this is what makes it a reference image)
   d.m_depthBias = 0.f;
   d.m_height = Height;
   d.m_rotX = d.m_rotY = d.m_rotZ = 0.f;
   flasher->m_desktopBackdrop = false;
   SetRect(flasher, { center, width, height });
   table->AddPart(flasher);
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

static bool RotateFlasher(Flasher *const flasher, const float angle, const Vertex2D &center, const bool useElementCenter)
{
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

bool ReferenceImageOp::Rotate(IEditable *part, const float angle, const Vertex2D &center, const bool useElementCenter)
{
   return IsReferenceImage(part) && RotateFlasher(static_cast<Flasher *>(part), angle, center, useElementCenter);
}

bool ReferenceImageOp::Rotate(const EditorUIPart *uiPart, const float angle, const Vertex2D &center, const bool useElementCenter)
{
   return IsReferenceImage(uiPart) && RotateFlasher(static_cast<Flasher *>(uiPart->GetEditable()), angle, center, useElementCenter);
}

static bool ScaleFlasher(IEditable *part, const float scaleX, const float scaleY, const Vertex2D &center, const bool useElementCenter)
{
   const float s = fabsf(scaleX - 1.f) >= fabsf(scaleY - 1.f) ? scaleX : scaleY;
   part->Scale(s, s, center, useElementCenter);
   return true;
}

bool ReferenceImageOp::Scale(IEditable *part, const float scaleX, const float scaleY, const Vertex2D &center, const bool useElementCenter)
{
   return IsReferenceImage(part) && ScaleFlasher(part, scaleX, scaleY, center, useElementCenter);
}

bool ReferenceImageOp::Scale(const EditorUIPart *uiPart, const float scaleX, const float scaleY, const Vertex2D &center, const bool useElementCenter)
{
   return IsReferenceImage(uiPart) && ScaleFlasher(uiPart->GetEditable(), scaleX, scaleY, center, useElementCenter);
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

void ReferenceImageOp::OpenAddDialog()
{
   if (!CanAdd())
      return;
   m_dialog = Dialog();
   m_dialog.fromFile = true;
   m_openDialog = true;
}

void ReferenceImageOp::Add(const string &image, const float width, const float height)
{
   EditorUI &ed = m_editor;
   PinTable *const table = ed.m_table;
   const Vertex2D center(0.5f * (table->m_left + table->m_right), 0.5f * (table->m_top + table->m_bottom));
   Flasher *const flasher = CreateFlasher(table, image, center, width, height);
   flasher->SetPartGroup(ed.GetPartGroupForNewPart()); // Freely placed: the target of any new part

   // Same player side setup as EditorUI::CreatePart
   ed.m_player->TimerSetup(flasher);
   flasher->RenderSetup(ed.m_renderer.get());
   ed.m_renderer->MarkStaticsDirty();
   ed.m_player->m_physics->Add(flasher);

   ed.m_undo.BeginUndo();
   ed.m_undo.MarkForCreate(flasher);
   ed.m_undo.SetLabel("Add Reference Image: "s + image);
   ed.m_undo.EndUndo();

   ed.UpdateEditableList();
   if (const auto it = ed.m_editableMap.find(flasher); it != ed.m_editableMap.end())
   {
      ed.SetSelection(Selection(it->second));
      OutlinerPanel::m_anchor = it->second;
   }
   ed.Report("Added reference image "s + flasher->GetName() + " (" + image + "), hidden in play");
}

Texture *ReferenceImageOp::ImportFile(const string &file)
{
   PinTable *const table = m_editor.m_table;
   const std::filesystem::path path = PathFromUTF8(file);
   Texture *const loaded = Texture::CreateFromFile(path);
   if (loaded == nullptr)
      return nullptr;
   // A same-name image would be replaced: pick a free one
   const string base = TitleFromFilename(path);
   string name = base;
   for (int i = 2; table->GetImage(name) != nullptr; i++)
      name = base + '_' + std::to_string(i);
   loaded->m_name = name;
   Texture *const tex = table->ImportImage(loaded); // A file identical to an existing image is not imported again: the existing image is used
   if (tex != loaded)
      m_editor.m_liveUI.PushNotification("'" + PathToUTF8(path.filename()) + "' is a duplicate of image '"s + tex->m_name + "': the existing image is used"s, 8000);
   else
      table->SetNonUndoableDirty(SaveDirtyState::Dirty); // Images are not part of the undo history (Images panel import)
   g_settingsService.GetAppSettings().SetRecentDir_ImageDir(PathToString(path.parent_path()), false);
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
   const char *const popupId = "Add Reference Image###ReferenceImageDialog";
   if (m_openDialog)
   {
      m_openDialog = false;
      ImGui::OpenPopup(popupId);
   }
   ImGui::SetNextWindowSize(ImVec2(ImGui::GetFontSize() * 30.f, 0.f), ImGuiCond_Appearing);
   if (!ImGui::BeginPopupModal(popupId, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
      return;

   ImGui::TextUnformatted("Add a scan, blueprint or photo to trace over (only shown in the editor, never in play)");
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

   // Real size
   const bool hasImage = pxWidth > 0 && pxHeight > 0;
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
   ImGui::BeginDisabled(!hasImage || dlg.width <= 0.f || dlg.height <= 0.f || !CanAdd());
   if (ImGui::Button("Add"))
   {
      string image;
      if (dlg.fromFile)
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
