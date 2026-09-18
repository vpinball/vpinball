// license:GPLv3+

#pragma once

#include "parts/dragpoint.h"
#include "ui/win/IWinUIPart.h"
#include "ui/win/parts/DragPointWinUIPart.h"

// Maintains one IWinUIPart per DragPoint of an DragPointCurve.
// DragPoint pointers are not stable (points are deleted and recreated on undo, point insertion/removal, ...),
// so the list is reconciled lazily by pointer identity on each access, preserving the UI parts of surviving points.
class DragPointUIPartList
{
public:
   DragPointUIPartList(PinTableWnd* editor, DragPointCurve* owner)
      : m_editor(editor)
      , m_owner(owner)
   {
   }

   IWinUIPart* Get(const DragPoint* point)
   {
      Sync();
      for (const auto& part : m_parts)
         if (part->GetDragPoint() == point)
            return part.get();
      return nullptr;
   }

   bool IsDragging(const DragPoint* point)
   {
      const IWinUIPart* const part = Get(point);
      return part && part->m_dragging;
   }

   bool IsSelected(const DragPoint* point)
   {
      const IWinUIPart* const part = Get(point);
      return part && part->m_selectstate != IWinUIPart::SelectState::NotSelected;
   }

private:
   void Sync()
   {
      const vector<CComObject<DragPoint>*>& points = m_owner->m_vdpoint;
      std::erase_if(m_parts, [&points](const std::unique_ptr<IWinUIPart>& part) { return std::ranges::find(points, part->GetDragPoint()) == points.end(); });
      for (CComObject<DragPoint>* const point : points)
         if (std::ranges::none_of(m_parts, [point](const std::unique_ptr<IWinUIPart>& part) { return part->GetDragPoint() == point; }))
            if (std::unique_ptr<IWinUIPart> part = std::make_unique<DragPointWinUIPart>(m_editor, point))
               m_parts.push_back(std::move(part));
   }

   PinTableWnd* const m_editor;
   DragPointCurve* const m_owner;
   vector<std::unique_ptr<IWinUIPart>> m_parts;
};
