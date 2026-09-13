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
   using CreateFunc = std::unique_ptr<IWinUIPart> (*)(PinTableWnd*, ISelect*);

   template <class UIPart, class Part> static void Register()
   {
      m_map[Part::ItemType] = [](PinTableWnd* editor, ISelect* part) -> std::unique_ptr<IWinUIPart> { return std::make_unique<UIPart>(editor, static_cast<Part*>(part)); };
   }

   static std::unique_ptr<IWinUIPart> Create(PinTableWnd*, ISelect* select);
   static std::unique_ptr<IWinUIPart> Create(PinTableWnd* editor, IEditable* editable) { return Create(editor, editable ? editable->GetISelect() : nullptr); }
   static void InitRegistry();

private:
   static ankerl::unordered_dense::map<ItemTypeEnum, CreateFunc> m_map;
};
