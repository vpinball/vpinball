// license:GPLv3+

#pragma once

#include "math/dragpoint.h"
#include "ui/win/IWinUIPart.h"
#include "ui/win/parts/DragPointWinUIPart.h"

// Maintains one IWinUIPart per DragPoint of an DragPointCurve, in the order of the curve points.
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

   // Returns the UI part of the index-th point of the owner's curve, nullptr if out of range
   IWinUIPart* GetAt(int index)
   {
      Sync();
      return (index >= 0 && index < (int)m_parts.size()) ? m_parts[index].get() : nullptr;
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
      const auto& points = m_owner->GetPoints();
      // Unchanged (always the case while painting, which accesses every point): a single linear pass
      if (std::ranges::equal(points, m_parts, [](const std::unique_ptr<DragPoint>& point, const std::unique_ptr<IWinUIPart>& part) { return point.get() == part->GetDragPoint(); }))
         return;
      // Rebuild in point order, reusing the UI parts of surviving points (keeping their selection & dragging state)
      vector<std::unique_ptr<IWinUIPart>> parts;
      parts.reserve(points.size());
      for (const auto& point : points)
      {
         const auto it = std::ranges::find_if(m_parts, [&point](const std::unique_ptr<IWinUIPart>& part) { return part && part->GetDragPoint() == point.get(); });
         if (it != m_parts.end())
            parts.push_back(std::move(*it));
         else
            parts.push_back(std::make_unique<DragPointWinUIPart>(m_editor, point.get()));
      }
      m_parts = std::move(parts);
   }

   PinTableWnd* const m_editor;
   DragPointCurve* const m_owner;
   vector<std::unique_ptr<IWinUIPart>> m_parts;
};
