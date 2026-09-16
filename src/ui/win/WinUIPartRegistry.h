#pragma once

#include "core/ieditable.h"
#include "core/iselect.h"
#include "ui/win/IWinUIPart.h"
#include "ui/win/PinTableWnd.h"

#include "unordered_dense.h"

#include <memory>

class WinUIPartRegistry final
{
public:
   template <class UIPart, class Part> static void Register()
   {
      UIPartInfo info;
      info.toolID = UIPart::ToolID;
      info.cursorID = UIPart::CursorID;
      info.allowedViews = UIPart::AllowedViews;
      info.createFunc = [](PinTableWnd* editor, IEditable* part) -> std::unique_ptr<IWinUIPart> { return std::make_unique<UIPart>(editor, static_cast<Part*>(part)); };
      m_map[Part::ItemType] = info;
   }

   static std::unique_ptr<IWinUIPart> Create(PinTableWnd*, IEditable* editable);
   static void InitRegistry();

   static ItemTypeEnum TypeFromToolID(int toolID);
   static int GetCursorID(ItemTypeEnum type);
   static IWinUIPart::AllowedViews GetAllowedViews(ItemTypeEnum type);

private:
   struct UIPartInfo
   {
      int toolID;
      int cursorID;
      IWinUIPart::AllowedViews allowedViews;
      std::unique_ptr<IWinUIPart> (*createFunc)(PinTableWnd*, IEditable*);
   };

   static ankerl::unordered_dense::map<ItemTypeEnum, UIPartInfo> m_map;
};
