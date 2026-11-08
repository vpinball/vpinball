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

// Part group (layer) reference space change (property pane): the children coordinates are
// re-expressed in the new reference space so that the parts do not move. Parts that can not be
// exactly converted (e.g. a surface can not be tilted out of its plane) are confirmed by a modal
// before doing anything.
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

   // Confirmation modal of the pending requests (rendered in the overlay window scope)
   void RenderModal();

private:
   struct Change
   {
      PartGroup *group = nullptr; // The edited layer
      PartGroupData::SpaceReference space; // Requested space reference
      PartGroupData::SpaceReference oldSpace; // Resolved space before the change
      PartGroupData::SpaceReference newSpace; // Resolved space after the change
      vector<IEditable *> parts; // Descendant parts exactly re-expressed in the new space
      vector<IEditable *> approximated; // Descendant parts that can not be exactly converted (best effort applied)
      vector<PartGroup *> groups; // Descendant groups whose center marker is re-expressed
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
