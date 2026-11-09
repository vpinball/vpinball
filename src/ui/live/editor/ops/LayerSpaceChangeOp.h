// license:GPLv3+

#pragma once

#include "math/matrix.h"
#include "parts/PartGroup.h"

class IEditable;
class PinTable;

namespace VPX::EditorUI
{

class EditorUI;
class EditorUIPart;

// Part group (layer) reference space change (property pane) and reparenting across reference
// spaces (outliner drag & drop, layer assignment menus): the children coordinates are re-expressed
// in the new reference space so that the parts do not move. Parts that can not be exactly
// converted (e.g. a surface can not be tilted out of its plane) are confirmed by a modal before
// doing anything.
class LayerSpaceChangeOp final
{
public:
   explicit LayerSpaceChangeOp(EditorUI &editor)
      : m_editor(editor)
   {
   }

   // Requests a change of the group's space reference: applies it right away when all the affected
   // children can be exactly converted, otherwise asks for confirmation through RenderModal
   void Request(PartGroup *group, PartGroupData::SpaceReference space);

   // Requests moving parts to another group (nullptr = top level): transforms expressed in a
   // different reference space than the target's resolved one are re-expressed so that the parts
   // do not move. Applies right away when all the conversions are exact, otherwise asks for
   // confirmation through RenderModal. Returns false while the move waits for confirmation.
   bool RequestMove(const vector<IEditable *> &moved, PartGroup *target);

   // Confirmation modal of the pending requests (rendered in the overlay window scope)
   void RenderModal();

private:
   // A transform to re-express in the change's destination space (a part's editor transform, or a
   // descendant group's center marker)
   struct Conversion
   {
      IEditable *editable = nullptr;
      PartGroupData::SpaceReference fromSpace; // Resolved reference space the transform is expressed in
   };
   struct Change
   {
      PartGroup *group = nullptr; // Layer whose space reference is being changed (nullptr for reparenting changes)
      PartGroupData::SpaceReference space = PartGroupData::SpaceReference::SR_INHERIT; // Requested space reference
      PartGroup *target = nullptr; // Reparenting: destination group (nullptr = top level)
      vector<IEditable *> moved; // Reparenting: parts being moved to the target group
      PartGroupData::SpaceReference newSpace = PartGroupData::SpaceReference::SR_PLAYFIELD; // Resolved destination space
      vector<Conversion> parts; // Descendant parts exactly re-expressed in the new space
      vector<Conversion> approximated; // Descendant parts that can not be exactly converted (best effort applied)
      vector<Conversion> groups; // Descendant groups whose center marker is re-expressed (always exact)
   };
   vector<Change> m_pending; // Space changes awaiting the user's confirmation
   bool m_openModal = false;
   EditorUI &m_editor;

   void Apply(const Change &change);
   void ApplyAll();
   static vector<IEditable *> SpaceChildren(const PartGroup *group);
   static bool TransformForSpace(EditorUIPart &uiPart, const Matrix3D &delta, bool apply);
   std::shared_ptr<EditorUIPart> FindUIPart(IEditable *part) const;
};

}
