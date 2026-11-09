// license:GPLv3+

#include "core/stdafx.h"

#include "BlueprintPane.h"

#include "../EditorUI.h"
#include "core/player.h"
#include "physics/PhysicsEngine.h"
#include "ui/live/LiveUI.h"
#include "OutlinerPanel.h"
#include "Selection.h"
#include "core/ieditable.h"
#include "math/dragpoint.h"
#include "parts/Collection.h"
#include "parts/PartGroup.h"
#include "parts/pintable.h"
#include "renderer/RenderDevice.h"
#include "ui/EditorClipboard.h"

namespace VPX::EditorUI
{

// The stacked parts under the context menu position, front to back (the win32 m_allHitElements)
vector<std::shared_ptr<EditorUIPart>> BlueprintPane::MenuHitParts() const
{
   vector<std::shared_ptr<EditorUIPart>> parts;
   for (auto it = m_menuHits.rbegin(); it != m_menuHits.rend(); ++it) // m_menuHits is in draw order: topmost last
      if (it->point == nullptr)
         if (const std::shared_ptr<EditorUIPart> uiPart = Resolve(it->part); uiPart != nullptr && std::ranges::find(parts, uiPart) == parts.end())
            parts.push_back(uiPart);
   return parts;
}

void BlueprintPane::RenderContextMenu()
{
   if (!ImGui::BeginPopup("Blueprint Context"))
      return;
   EditorUI& editor = m_editor;
   const bool canEdit = !editor.IsInspectMode() && !editor.m_table->IsLocked();

   if (m_menuPoint != nullptr && m_menuPointPart != nullptr)
   {
      // Drag point context menu (win32 IDR_POINTMENU / IDR_POINTMENU_SMOOTH): opens on any visible
      // drag point handle. The commands apply to the clicked point, or to the whole point selection
      // when the clicked point belongs to it (drag point edit mode)
      const std::shared_ptr<EditorUIPart> ownerPart = Resolve(m_menuPointPart);
      if (ownerPart == nullptr)
      {
         ImGui::EndPopup();
         return;
      }
      DragPoint* const clickedPoint = const_cast<DragPoint*>(m_menuPoint);
      vector<DragPoint*> targets;
      if (editor.m_pointEditPart == ownerPart && editor.IsPointSelected(clickedPoint))
         targets = editor.m_pointSel;
      else
         targets.push_back(clickedPoint);
      IEditable* const owner = ownerPart->GetEditable();
      DragPointCurve* const curve = ownerPart->GetDragPointCurve();
      const auto pointOp = [this, &editor, owner, curve, &targets](const std::function<void(DragPoint*)>& op)
      {
         editor.m_undo.BeginUndo();
         editor.m_undo.MarkForUndo(owner);
         editor.m_undo.EndUndo();
         for (DragPoint* const point : targets)
            op(point);
         if (curve != nullptr)
            curve->OnPointsModified();
         editor.m_renderer->ReinitRenderable(owner->GetIRenderable());
         editor.m_player->m_physics->Update(owner);
      };

      ImGui::BeginDisabled(!canEdit);
      if (ImGui::MenuItem("Smooth", nullptr, clickedPoint->IsSmooth()))
         pointOp(
            [](DragPoint* point)
            {
               if (!point->IsSmooth())
                  point->ToggleSmooth();
            });
      if (ownerPart->SupportsSlingshotSegments() && ImGui::MenuItem("Slingshot", nullptr, clickedPoint->m_slingshot && !clickedPoint->IsSmooth()))
         pointOp([](DragPoint* point) { point->ToggleSlingshot(); });
      if (ImGui::MenuItem("Delete Point", nullptr, false, std::ranges::any_of(targets, [](const DragPoint* point) { return point->CanDelete(); })))
         pointOp(
            [&editor](DragPoint* point)
            {
               if (DragPointCurve* const c = point->GetCurve(); c != nullptr && point->CanDelete())
               {
                  std::erase(editor.m_pointSel, point);
                  c->DeletePoint(point);
               }
            });
      ImGui::EndDisabled();
   }
   else
   {
      // Part / multi selection / canvas context menu (win32 IDR_SURFACEMENU / IDR_MULTIMENU /
      // IDR_TABLEMENU with the shared appended block)
      const std::shared_ptr<EditorUIPart> menuPart = editor.m_selection.GetPart();
      IEditable* const menuEditable = menuPart ? menuPart->GetEditable() : nullptr;
      const bool canPaste = canEdit && VPX::EditorClipboard::HasParts();

      // Add Point: single selected drag point curve part (win32 IDR_SURFACEMENU)
      if (menuPart != nullptr && menuPart->GetDragPointCurve() != nullptr && editor.m_pointEditPart == nullptr)
      {
         ImGui::BeginDisabled(!canEdit);
         if (ImGui::MenuItem("Add Point"))
         {
            editor.m_undo.BeginUndo();
            editor.m_undo.MarkForUndo(menuEditable);
            editor.m_undo.EndUndo();
            menuPart->AddPointOnCurve(Unproject(m_menuPos));
            editor.m_renderer->ReinitRenderable(menuEditable->GetIRenderable());
            editor.m_player->m_physics->Update(menuEditable);
         }
         ImGui::EndDisabled();
      }

      if (menuEditable != nullptr)
      {
         // Transform commands on the whole selection (in drag point edit mode, on the selected points)
         ImGui::BeginDisabled(!canEdit);
         if (ImGui::MenuItem("Flip X"))
            editor.FlipSelection(true);
         if (ImGui::MenuItem("Flip Y"))
            editor.FlipSelection(false);
         if (ImGui::MenuItem("Rotate..."))
         {
            m_dialog = DialogKind::Rotate;
            m_dialogRequest = true;
         }
         if (ImGui::MenuItem("Scale..."))
         {
            m_dialog = DialogKind::Scale;
            m_dialogRequest = true;
         }
         if (ImGui::MenuItem("Translate..."))
         {
            m_dialog = DialogKind::Translate;
            m_dialogRequest = true;
         }
         ImGui::EndDisabled();
         ImGui::Separator();
      }

      if (ImGui::MenuItem("Copy", "Ctrl+C", false, menuEditable != nullptr))
         editor.CopySelection();
      if (ImGui::MenuItem("Paste", "Ctrl+V", false, canPaste))
         editor.PasteSelection(nullptr);
      if (ImGui::MenuItem("Paste At", "Ctrl+Shift+V", false, canPaste))
         editor.PasteSelection(&m_menuPos);

      if (menuEditable != nullptr)
      {
         ImGui::Separator();
         if (ImGui::MenuItem("Drawing Order (Hit)..."))
         {
            m_dialog = DialogKind::OrderHit;
            m_dialogRequest = true;
         }
         if (ImGui::MenuItem("Drawing Order (Select)..."))
         {
            m_dialog = DialogKind::OrderSelect;
            m_dialogRequest = true;
         }
         ImGui::Separator();
         ImGui::BeginDisabled(!canEdit);
         if (ImGui::MenuItem("Draw In Front"))
            editor.MoveSelectionInDrawOrder(true);
         if (ImGui::MenuItem("Draw In Back"))
            editor.MoveSelectionInDrawOrder(false);
         if (ImGui::MenuItem("Set As Default"))
            menuEditable->WriteRegDefaults();
         ImGui::Separator();
         if (ImGui::BeginMenu("Assign To Layer"))
         {
            bool anyGroup = false;
            for (IEditable* const edit : editor.m_table->GetParts())
               if (edit->GetItemType() == eItemPartGroup && edit->GetPartGroup() == nullptr)
               {
                  anyGroup = true;
                  if (ImGui::MenuItem(edit->GetName().c_str()))
                     editor.MoveSelectionToPartGroup(static_cast<PartGroup*>(edit));
               }
            if (!anyGroup)
               ImGui::MenuItem("(no part group)", nullptr, false, false);
            ImGui::EndMenu();
         }
         // The current layer is never the reference images group: a reference image would leave it and show in play
         const bool hasReferenceImage = std::ranges::any_of(editor.m_multiSel, [](const auto& part) { return ReferenceImageOp::IsReferenceImage(part->GetEditable()); });
         if (ImGui::MenuItem("Assign To Current Layer", nullptr, false, !hasReferenceImage))
            editor.MoveSelectionToPartGroup(editor.GetPartGroupForNewPart());
         if (ImGui::BeginMenu("To Collection"))
         {
            const vector<CComObject<Collection>*>& collections = editor.m_table->GetCollections();
            if (collections.empty())
               ImGui::MenuItem("(no collection)", nullptr, false, false);
            for (size_t i = 0; i < collections.size(); i++)
            {
               Collection* const collection = collections[i];
               const bool member = std::ranges::any_of(editor.m_multiSel, [collection](const std::shared_ptr<EditorUIPart>& part)
                  { return std::ranges::find(part->GetEditable()->m_vCollection, collection) != part->GetEditable()->m_vCollection.end(); });
               if (ImGui::MenuItem(collection->get_Name().c_str(), nullptr, member))
                  editor.ToggleCollectionMembership(static_cast<int>(i));
            }
            ImGui::EndMenu();
         }
         const bool anyLocked = std::ranges::any_of(editor.m_multiSel, [](const std::shared_ptr<EditorUIPart>& part) { return part->GetEditable()->IsUILocked(); });
         if (ImGui::MenuItem("Lock", nullptr, anyLocked))
            editor.LockElements();
         ImGui::EndDisabled();

         // The stacked parts under the menu position (the win32 menu lists them for picking)
         const vector<std::shared_ptr<EditorUIPart>> hitParts = MenuHitParts();
         if (hitParts.size() > 1)
         {
            ImGui::Separator();
            for (const std::shared_ptr<EditorUIPart>& part : hitParts)
            {
               IEditable* const editable = part->GetEditable();
               if (!editable->IsUIVisible(false))
                  continue;
               if (ImGui::MenuItem(editable->GetName().c_str(), nullptr, editor.IsPartSelected(part)))
               {
                  if (ImGui::GetIO().KeyShift)
                     editor.TogglePartSelection(part);
                  else
                     editor.SetSelection(Selection(part));
                  OutlinerPanel::m_anchor = part;
               }
            }
         }
      }
      else
      {
         ImGui::Separator();
         ImGui::BeginDisabled(!canEdit);
         if (ImGui::MenuItem("Add Part..."))
            editor.RequestAddPartPopup(m_menuPos);
         ImGui::EndDisabled();
      }
   }
   ImGui::EndPopup();
}

void BlueprintPane::ApplyTransformDialog()
{
   switch (m_dialog)
   {
   case DialogKind::Rotate: m_editor.RotateMultiSel(m_dlgAngle, m_dlgPivot, m_dlgUseCenter); break;
   case DialogKind::Scale: m_editor.ScaleMultiSel(m_dlgSX, m_dlgSquare ? m_dlgSX : m_dlgSY, m_dlgPivot, m_dlgUseCenter); break;
   case DialogKind::Translate: m_editor.TranslateMultiSel(Vertex2D(m_dlgTX, m_dlgTY)); break;
   default: break;
   }
}

void BlueprintPane::RenderDialogs()
{
   EditorUI& editor = m_editor;
   const float dpi = editor.m_liveUI.GetDPI();
   switch (m_dialog)
   {
   case DialogKind::Rotate:
   case DialogKind::Scale:
   case DialogKind::Translate:
   {
      const char* const title = m_dialog == DialogKind::Rotate ? "Rotate###BlueprintDialog" : m_dialog == DialogKind::Scale ? "Scale###BlueprintDialog" : "Translate###BlueprintDialog";
      if (!ImGui::BeginPopupModal(title, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
      {
         m_dialog = DialogKind::None;
         return;
      }
      if (!m_dialogInit)
      {
         m_dialogInit = true;
         const std::shared_ptr<EditorUIPart>& activePart = editor.m_selection.GetPart();
         if (m_dialog == DialogKind::Rotate)
            m_dlgAngle = activePart ? activePart->GetEditable()->GetRotate() : 0.f;
         else if (m_dialog == DialogKind::Scale)
         {
            const Vertex2D scale = activePart ? activePart->GetEditable()->GetScale() : Vertex2D(1.f, 1.f);
            m_dlgSX = scale.x;
            m_dlgSY = scale.y;
            m_dlgSquare = true;
         }
         else
         {
            m_dlgTX = 0.f;
            m_dlgTY = 0.f;
         }
         m_dlgUseCenter = true;
         m_dlgPivot = editor.GetMultiSelCenter();
      }
      if (m_dialog == DialogKind::Rotate)
      {
         ImGui::SetNextItemWidth(120.f * dpi);
         ImGui::InputFloat("Angle", &m_dlgAngle);
      }
      else if (m_dialog == DialogKind::Scale)
      {
         ImGui::SetNextItemWidth(120.f * dpi);
         ImGui::InputFloat("Factor X", &m_dlgSX);
         if (ImGui::Checkbox("Square", &m_dlgSquare) && m_dlgSquare)
            m_dlgSY = m_dlgSX;
         ImGui::BeginDisabled(m_dlgSquare);
         ImGui::SetNextItemWidth(120.f * dpi);
         ImGui::InputFloat("Factor Y", &m_dlgSY);
         ImGui::EndDisabled();
         if (m_dlgSquare)
            m_dlgSY = m_dlgSX;
      }
      else
      {
         ImGui::SetNextItemWidth(120.f * dpi);
         ImGui::InputFloat("Offset X", &m_dlgTX);
         ImGui::SetNextItemWidth(120.f * dpi);
         ImGui::InputFloat("Offset Y", &m_dlgTY);
      }
      if (m_dialog != DialogKind::Translate)
      {
         // Win32 'use element center' checkbox: unchecked transforms around a custom pivot (the menu position)
         if (ImGui::Checkbox("Each element's center", &m_dlgUseCenter) && !m_dlgUseCenter)
            m_dlgPivot = Unproject(m_menuPos);
         ImGui::BeginDisabled(m_dlgUseCenter);
         ImGui::SetNextItemWidth(160.f * dpi);
         ImGui::InputFloat2("Center", &m_dlgPivot.x);
         ImGui::EndDisabled();
      }
      // Win32 OK / Apply / Undo / Cancel buttons: each apply is an undoable record, Cancel reverts them all
      if (ImGui::Button("OK"))
      {
         if (m_dialogApplyCount == 0)
            ApplyTransformDialog();
         m_dialog = DialogKind::None;
         ImGui::CloseCurrentPopup();
      }
      ImGui::SameLine();
      if (ImGui::Button("Apply"))
      {
         ApplyTransformDialog();
         m_dialogApplyCount++;
      }
      ImGui::SameLine();
      ImGui::BeginDisabled(m_dialogApplyCount == 0);
      if (ImGui::Button("Undo"))
      {
         m_dialogApplyCount--;
         editor.UndoSteps(1);
      }
      ImGui::EndDisabled();
      ImGui::SameLine();
      if (ImGui::Button("Cancel"))
      {
         while (m_dialogApplyCount-- > 0)
            editor.UndoSteps(1);
         m_dialog = DialogKind::None;
         ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
      break;
   }
   case DialogKind::OrderHit:
   case DialogKind::OrderSelect:
   {
      if (!ImGui::BeginPopupModal("Drawing Order###BlueprintDialog", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
      {
         m_dialog = DialogKind::None;
         return;
      }
      if (!m_dialogInit)
      {
         m_dialogInit = true;
         m_orderIndex = -1;
         m_orderList.clear();
         if (m_dialog == DialogKind::OrderHit)
         {
            // The stacked parts under the menu position, front to back
            for (const std::shared_ptr<EditorUIPart>& part : MenuHitParts())
               m_orderList.push_back(part->GetEditable());
         }
         else
         {
            // The selected parts in drawing order, front (topmost) first (win32 GetDrawingOrderParts)
            const vector<IEditable*>& parts = editor.m_table->GetParts();
            for (SSIZE_T i = (SSIZE_T)parts.size() - 1; i >= 0; i--)
               for (const std::shared_ptr<EditorUIPart>& sel : editor.m_multiSel)
                  if (sel->GetEditable() == parts[i])
                     m_orderList.push_back(parts[i]);
         }
      }
      ImGui::TextUnformatted(m_dialog == DialogKind::OrderHit ? "Drawing order of the parts under the cursor:" : "Drawing order of the selected parts:");
      ImGui::BeginChild("##OrderList", ImVec2(280.f * dpi, 180.f * dpi), ImGuiChildFlags_Borders);
      for (int i = 0; i < (int)m_orderList.size(); i++)
         if (ImGui::Selectable(m_orderList[i]->GetName().c_str(), i == m_orderIndex))
            m_orderIndex = i;
      ImGui::EndChild();
      if (ImGui::Button("Up") && m_orderIndex > 0)
      {
         std::swap(m_orderList[m_orderIndex], m_orderList[m_orderIndex - 1]);
         m_orderIndex--;
         m_editor.m_table->ReorderParts(m_orderList);
      }
      ImGui::SameLine();
      ImGui::BeginDisabled(m_orderIndex < 0 || m_orderIndex + 1 >= (int)m_orderList.size());
      if (ImGui::Button("Down"))
      {
         std::swap(m_orderList[m_orderIndex], m_orderList[m_orderIndex + 1]);
         m_orderIndex++;
         m_editor.m_table->ReorderParts(m_orderList);
      }
      ImGui::EndDisabled();
      ImGui::SameLine();
      if (ImGui::Button("Close"))
      {
         m_dialog = DialogKind::None;
         ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
      break;
   }
   default: break;
   }
}

}
