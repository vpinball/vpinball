// license:GPLv3+

#pragma once

#include "parts/dragpoint.h"
#include "ui/win/IWinUIPart.h"
#include "ui/win/WinUIPartRegistry.h"

// Maintains one IWinUIPart per DragPoint of an IHaveDragPoints.
// DragPoint pointers are not stable (points are deleted and recreated on undo, point insertion/removal, ...),
// so the list is reconciled lazily by pointer identity on each access, preserving the UI parts of surviving points.
class DragPointUIPartList
{
public:
   DragPointUIPartList(PinTableWnd* editor, IHaveDragPoints* owner)
      : m_editor(editor)
      , m_owner(owner)
   {
   }

   IWinUIPart* Get(const ISelect* select)
   {
      Sync();
      for (const auto& part : m_parts)
         if (part->GetSelect() == select)
            return part.get();
      return nullptr;
   }

   bool IsDragging(const ISelect* select)
   {
      const IWinUIPart* const part = Get(select);
      return part && part->m_dragging;
   }

   bool IsSelected(const ISelect* select)
   {
      const IWinUIPart* const part = Get(select);
      return part && part->m_selectstate != IWinUIPart::SelectState::NotSelected;
   }

private:
   void Sync()
   {
      const vector<CComObject<DragPoint>*>& points = m_owner->m_vdpoint;
      std::erase_if(
         m_parts, [&points](const std::unique_ptr<IWinUIPart>& part) { return std::ranges::find(points, static_cast<CComObject<DragPoint>*>(part->GetSelect())) == points.end(); });
      for (CComObject<DragPoint>* const point : points)
         if (std::ranges::none_of(m_parts, [point](const std::unique_ptr<IWinUIPart>& part) { return part->GetSelect() == static_cast<ISelect*>(point); }))
            if (std::unique_ptr<IWinUIPart> part = WinUIPartRegistry::Create(m_editor, point))
               m_parts.push_back(std::move(part));
   }

   PinTableWnd* const m_editor;
   IHaveDragPoints* const m_owner;
   vector<std::unique_ptr<IWinUIPart>> m_parts;
};
