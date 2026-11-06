#pragma once

#include "EditorUIPart.h"

#include "unordered_dense.h"

#include <memory>

namespace VPX::EditorUI
{

// Registry mapping editables to their UI parts, following the same pattern as the
// WinUI editor registry. Registered entries are keyed on the part's declared
// ItemTypeEnum (Part::ItemType). Call InitRegistry() once before use.
class EditorUIPartRegistry final
{
public:
   template <class UIPart, class Part> static void Register()
   {
      m_map[Part::ItemType] = [](IEditable* editable) { return std::make_shared<UIPart>(static_cast<Part*>(editable)); };
   }

   static std::shared_ptr<EditorUIPart> Create(IEditable* editable);
   static void InitRegistry();

private:
   static ankerl::unordered_dense::map<ItemTypeEnum, std::function<std::shared_ptr<EditorUIPart>(IEditable*)>> m_map;
};

}
