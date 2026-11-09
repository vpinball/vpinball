// license:GPLv3+

#include "core/stdafx.h"
#include "LayerSpaceChangeOp.h"

#include "core/player.h"
#include "parts/pintable.h"
#include "physics/PhysicsEngine.h"
#include "renderer/Renderer.h"
#include "ui/live/EditorUI.h"
#include "ui/live/editor/parts/EditorUIPartRegistry.h"

namespace VPX::EditorUI
{

namespace
{
   const char *const spaceNames[] = { "Playfield", "Cabinet", "Cabinet Feet", "Room", "Inherit" };

   // Decompose a transform into the position/scale/rotation components used by the parts'
   // SetTransform (same decomposition as EditorUI::SetSelectionTransform)
   void ExtractTRS(Matrix3D transform, vec3 &pos, vec3 &scale, vec3 &rot)
   {
      const Vertex3Ds right(transform._11, transform._12, transform._13);
      const Vertex3Ds up(transform._21, transform._22, transform._23);
      const Vertex3Ds dir(transform._31, transform._32, transform._33);
      scale.Set(max(right.Length(), 1e-8f), max(up.Length(), 1e-8f), max(dir.Length(), 1e-8f)); // Clamp to avoid division by zero

      transform._11 /= scale.x; // Normalize transform to evaluate rotation
      transform._12 /= scale.x;
      transform._13 /= scale.x;
      transform._21 /= scale.y;
      transform._22 /= scale.y;
      transform._23 /= scale.y;
      transform._31 /= scale.z;
      transform._32 /= scale.z;
      transform._33 /= scale.z;

      pos.Set(transform._41, transform._42, transform._43);

      // Derived from https://learnopencv.com/rotation-matrix-to-euler-angles/
      const float sy = sqrtf(transform._11 * transform._11 + transform._21 * transform._21);
      if (sy > 1e-6f)
      {
         rot.x = -RADTOANG(atan2f(transform._32, transform._33));
         rot.y = -RADTOANG(atan2f(-transform._31, sy));
         rot.z = -RADTOANG(atan2f(transform._21, transform._11));
      }
      else
      {
         rot.x = -RADTOANG(atan2f(transform._23, transform._22));
         rot.y = -RADTOANG(atan2f(-transform._31, sy));
         rot.z = 0.f;
      }
   }

   bool SameTransform(const Matrix3D &a, const Matrix3D &b)
   {
      for (int r = 0; r < 4; r++)
         for (int c = 0; c < 4; c++)
            if (fabsf(a.m[r][c] - b.m[r][c]) > 0.05f) // ~0.03mm: below visible differences
               return false;
      return true;
   }

   // Same space transforms as the editor renderer (see RenderView::SetSpaceReference): the playfield
   // is the reference space, all the other spaces share the default playfield to cabinet transform
   Matrix3D SpaceTransform(const PinTable *table, const PartGroupData::SpaceReference space)
   {
      return space == PartGroupData::SpaceReference::SR_PLAYFIELD ? Matrix3D::MatrixIdentity() : table->GetDefaultPlayfieldToCabMatrix();
   }
}

// Descendants of the given group whose stored coordinates are expressed in the group's resolved
// reference space: children whose parent chain reaches the group through space-inheriting
// groups only (an explicit space reference breaks the chain and isolates its own children)
vector<IEditable *> LayerSpaceChangeOp::SpaceChildren(const PartGroup *group)
{
   vector<IEditable *> children;
   for (IEditable *const part : group->GetPTable()->GetParts())
      for (PartGroup *parent = part->GetPartGroup(); parent != nullptr; parent = parent->GetPartGroup())
      {
         if (parent == group)
         {
            children.push_back(part);
            break;
         }
         if (parent->m_d.m_spaceReference != PartGroupData::SpaceReference::SR_INHERIT)
            break;
      }
   return children;
}

// The UI part driving a part's editor transform, preferring the live part's instance (it may hold
// an in-progress curve transform) and falling back to a fresh one (e.g. startup table parts)
std::shared_ptr<EditorUIPart> LayerSpaceChangeOp::FindUIPart(IEditable *const part) const
{
   const auto it = m_editor.m_editableMap.find(part);
   return it != m_editor.m_editableMap.end() ? it->second : EditorUIPartRegistry::Create(part);
}

// Re-expresses a part's transform in the new reference space: its reported transform goes
// through the space delta, and the part keeps the components it can express.
// In check mode the transform is applied then reverted, returning whether the part can be
// exactly re-expressed; in apply mode the best effort conversion is written for real.
bool LayerSpaceChangeOp::TransformForSpace(EditorUIPart &uiPart, const Matrix3D &delta, const bool apply)
{
   Matrix3D initial;
   if (uiPart.GetTransform(initial) == EditorUIPart::TM_None)
      return true; // Nothing spatial to convert (e.g. timers)
   const Matrix3D target = initial * delta;
   vec3 pos, scale, rot;
   ExtractTRS(target, pos, scale, rot);
   if (apply)
   {
      uiPart.SetTransform(pos, scale, rot);
      return true;
   }
   uiPart.SetTransform(pos, scale, rot);
   Matrix3D achieved;
   uiPart.GetTransform(achieved);
   ExtractTRS(initial, pos, scale, rot);
   uiPart.SetTransform(pos, scale, rot); // Restore the part exactly as it was
   return SameTransform(achieved, target);
}

void LayerSpaceChangeOp::Apply(const Change &change)
{
   PinTable *const table = change.group != nullptr ? change.group->GetPTable() : change.target != nullptr ? change.target->GetPTable() : change.moved.front()->GetPTable();
   const bool editedTable = table == m_editor.m_table;
   const bool useUndo = editedTable && !m_editor.IsInspectMode();

   const auto mark = [this, useUndo, editedTable](IEditable *const editable)
   {
      if (useUndo)
         m_editor.m_undo.MarkForUndo(editable);
      else if (editedTable)
         m_editor.m_liveEditedParts.insert(editable);
   };

   // The change itself: a layer's space reference write, or the reparenting to the target group
   if (change.group != nullptr)
   {
      mark(change.group);
      change.group->m_d.m_spaceReference = change.space;
   }
   for (IEditable *const editable : change.moved)
   {
      mark(editable);
      editable->SetPartGroup(change.target);
   }

   // Re-expression of the transforms stored in a space different from the destination's
   const Matrix3D toNew = Matrix3D::MatrixInverse(SpaceTransform(table, change.newSpace));
   const auto convert = [&](const Conversion &conversion)
   {
      IEditable *const editable = conversion.editable;
      mark(editable);
      const Matrix3D delta = SpaceTransform(table, conversion.fromSpace) * toNew;
      if (editable->GetItemType() == eItemPartGroup)
      {
         PartGroup *const sub = static_cast<PartGroup *>(editable);
         const Vertex3Ds v = delta * Vertex3Ds(sub->m_d.m_v.x, sub->m_d.m_v.y, 0.f);
         sub->m_d.m_v.x = v.x;
         sub->m_d.m_v.y = v.y;
      }
      else if (const std::shared_ptr<EditorUIPart> &uiPart = FindUIPart(editable))
         TransformForSpace(*uiPart, delta, true);
      if (editedTable)
      {
         m_editor.m_renderer->ReinitRenderable(editable->GetIRenderable());
         m_editor.m_player->m_physics->Update(editable);
      }
   };
   for (const Conversion &conversion : change.groups)
      convert(conversion);
   for (const Conversion &conversion : change.parts)
      convert(conversion);
   for (const Conversion &conversion : change.approximated)
      convert(conversion);
}

void LayerSpaceChangeOp::Request(PartGroup *const group, const PartGroupData::SpaceReference space)
{
   // Resolve the requested space (SR_INHERIT resolves against the parent chain): when the resolved
   // space does not change, the children are unaffected and only the reference field is written
   const PartGroupData::SpaceReference oldSpace = group->GetReferenceSpace();
   const PartGroupData::SpaceReference newSpace = space == PartGroupData::SpaceReference::SR_INHERIT
      ? (group->GetPartGroup() != nullptr ? group->GetPartGroup()->GetReferenceSpace() : PartGroupData::SpaceReference::SR_PLAYFIELD)
      : space;
   if (newSpace == oldSpace)
   {
      group->m_d.m_spaceReference = space;
      return;
   }

   const Matrix3D delta = SpaceTransform(group->GetPTable(), oldSpace) * Matrix3D::MatrixInverse(SpaceTransform(group->GetPTable(), newSpace));
   Change change;
   change.group = group;
   change.space = space;
   change.newSpace = newSpace;
   vector<std::shared_ptr<EditorUIPart>> tempParts; // Keeps the UI parts of the dry run alive
   for (IEditable *const editable : SpaceChildren(group))
   {
      if (editable->GetItemType() == eItemPartGroup)
         change.groups.push_back({ editable, oldSpace }); // Center marker, in the parent's space
      else if (!editable->m_desktopBackdrop) // Desktop backdrop parts are 2D, outside of the 3D reference spaces
      {
         if (const std::shared_ptr<EditorUIPart> &uiPart = FindUIPart(editable))
         {
            tempParts.push_back(uiPart);
            (TransformForSpace(*uiPart, delta, false) ? change.parts : change.approximated).push_back({ editable, oldSpace });
         }
      }
   }

   if (!change.approximated.empty())
   {
      // Some children can not be exactly re-expressed in the new space: confirm before doing anything
      m_pending.push_back(std::move(change));
      m_openModal = true;
      return;
   }

   Apply(change);
   m_editor.Report("Layer '"s + group->GetName() + "' moved to the "s + spaceNames[static_cast<int>(newSpace)] + " reference space: "s
      + std::to_string(change.parts.size() + change.groups.size()) + " children re-expressed"s);
}

// Reparenting parts to a group on another reference space re-expresses their transforms (the same
// conversions as a layer space change): a part's transform is stored in its parent group's resolved
// space, and a space-inheriting group's children follow its space.
bool LayerSpaceChangeOp::RequestMove(const vector<IEditable *> &moved, PartGroup *const target)
{
   if (moved.empty())
      return true;
   Change change;
   change.target = target;
   change.moved = moved;
   change.newSpace = target != nullptr ? target->GetReferenceSpace() : PartGroupData::SpaceReference::SR_PLAYFIELD;

   PinTable *const table = target != nullptr ? target->GetPTable() : moved.front()->GetPTable();
   const Matrix3D toNew = Matrix3D::MatrixInverse(SpaceTransform(table, change.newSpace));
   vector<std::shared_ptr<EditorUIPart>> tempParts; // Keeps the UI parts of the dry run alive
   vector<IEditable *> converted; // Already queued for conversion (a moved part also lists its inheriting children)
   const auto queue = [&](IEditable *const editable, const PartGroupData::SpaceReference from)
   {
      if (std::ranges::find(converted, editable) != converted.end())
         return;
      converted.push_back(editable);
      if (editable->GetItemType() == eItemPartGroup)
         change.groups.push_back({ editable, from }); // Center marker, in the parent's space
      else if (!editable->m_desktopBackdrop) // Desktop backdrop parts are 2D, outside of the 3D reference spaces
         if (const std::shared_ptr<EditorUIPart> &uiPart = FindUIPart(editable))
         {
            tempParts.push_back(uiPart);
            const Matrix3D delta = SpaceTransform(table, from) * toNew;
            (TransformForSpace(*uiPart, delta, false) ? change.parts : change.approximated).push_back({ editable, from });
         }
   };
   for (IEditable *const editable : moved)
   {
      // The space a transform is expressed in is the resolved space of the part's parent group
      const PartGroup *const parent = editable->GetPartGroup();
      const PartGroupData::SpaceReference from = parent != nullptr ? parent->GetReferenceSpace() : PartGroupData::SpaceReference::SR_PLAYFIELD;
      if (from == change.newSpace)
         continue;
      queue(editable, from);
      // The children of a space-inheriting group follow its space (an explicit one keeps its own);
      // moved parts re-express themselves through their own entry
      if (editable->GetItemType() == eItemPartGroup && static_cast<const PartGroup *>(editable)->m_d.m_spaceReference == PartGroupData::SpaceReference::SR_INHERIT)
         for (IEditable *const child : SpaceChildren(static_cast<PartGroup *>(editable)))
            if (std::ranges::find(moved, child) == moved.end())
               queue(child, from);
   }

   if (!change.approximated.empty())
   {
      // Some children can not be exactly re-expressed in the new space: confirm before doing anything
      m_pending.push_back(std::move(change));
      m_openModal = true;
      return false;
   }

   const bool useUndo = !m_editor.IsInspectMode();
   if (useUndo)
      m_editor.m_undo.BeginUndo();
   Apply(change);
   if (useUndo)
      m_editor.m_undo.EndUndo();
   return true;
}

void LayerSpaceChangeOp::ApplyAll()
{
   if (m_pending.empty())
      return;
   int movedCount = 0, partCount = 0, approximatedCount = 0;
   for (const Change &change : m_pending)
   {
      movedCount += (int)change.moved.size();
      partCount += (int)(change.parts.size() + change.approximated.size() + change.groups.size());
      approximatedCount += (int)change.approximated.size();
   }
   const bool useUndo = !m_editor.IsInspectMode();
   if (useUndo)
      m_editor.m_undo.BeginUndo();
   for (const Change &change : m_pending)
      Apply(change);
   if (useUndo)
   {
      m_editor.m_undo.SetLabel(movedCount > 0 ? "Move to layer"s : "Layer reference space"s);
      m_editor.m_undo.EndUndo();
   }
   m_pending.clear();
   if (movedCount > 0)
      m_editor.UpdateEditableList(); // Refresh the outliner paths and ordering after the reparenting
   const string prefix = movedCount > 0 ? std::to_string(movedCount) + " part(s) moved: "s : ""s;
   m_editor.Report(prefix + std::to_string(partCount) + " children re-expressed in the new reference space"s
      + (approximatedCount > 0 ? ", "s + std::to_string(approximatedCount) + " approximated"s : ""s));
}

void LayerSpaceChangeOp::RenderModal()
{
   if (m_pending.empty())
      return;
   if (m_openModal)
   {
      m_openModal = false;
      ImGui::OpenPopup("Reference Space Change###ConfirmSpaceChange");
   }
   if (ImGui::BeginPopupModal("Reference Space Change###ConfirmSpaceChange", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
   {
      vector<string> names;
      for (const Change &change : m_pending)
         for (const Conversion &conversion : change.approximated)
            names.push_back(conversion.editable->GetName());
      ImGui::Text("%d part(s) can not be exactly re-expressed in the new reference space:", (int)names.size());
      for (size_t i = 0; i < names.size() && i < 10; i++)
         ImGui::BulletText("%s", names[i].c_str());
      if (names.size() > 10)
         ImGui::BulletText("...and %d more", (int)names.size() - 10);
      ImGui::NewLine();
      ImGui::TextUnformatted("They will be converted on a best effort basis and will move.");
      ImGui::NewLine();
      if (ImGui::Button("Apply anyway"))
      {
         ImGui::CloseCurrentPopup();
         ApplyAll();
      }
      ImGui::SameLine();
      ImGui::SetItemDefaultFocus();
      if (ImGui::Button("Cancel"))
      {
         ImGui::CloseCurrentPopup();
         m_pending.clear();
      }
      ImGui::EndPopup();
   }
}

}
