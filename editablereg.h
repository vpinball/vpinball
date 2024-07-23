#pragma once

#include "unordered_dense.h"

typedef IEditable*(*CreateFuncType)();
typedef IEditable*(*CreateAndInitFuncType)(PinTable *pt, float x, float y);

struct EditableInfo
{
   ItemTypeEnum type;
   int typeNameID;
   int toolID;
   int cursorID;
   unsigned int allowedViews;
   CreateFuncType createFunc;
   CreateAndInitFuncType createAndInitFunc;
};

class EditableRegistry
{
public:
   template <class T>
   static void RegisterEditable()
   {
      EditableInfo ei;
      ei.type = T::ItemType;
      ei.typeNameID = T::TypeNameID;
      ei.toolID = T::ToolID;
      ei.cursorID = T::CursorID;
      ei.allowedViews = T::AllowedViews;
      ei.createFunc = &T::COMCreateEditable;
      ei.createAndInitFunc = &T::COMCreateAndInit;
      m_map[ei.type] = ei;
   }

   static IEditable* Create(ItemTypeEnum type)
   {
      return FindOrFail(type)->createFunc();
   }

   static IEditable* CreateAndInit(ItemTypeEnum type, PinTable *pt, float x, float y)
   {
      return FindOrFail(type)->createAndInitFunc(pt, x, y);
   }

   static int GetTypeNameStringID(ItemTypeEnum type)
   {
      return FindOrFail(type)->typeNameID;
   }

   static ItemTypeEnum TypeFromToolID(int toolID)
   {
      for (ankerl::unordered_dense::map<ItemTypeEnum, EditableInfo>::const_iterator it = m_map.begin(); it != m_map.end(); ++it)
      {
         if (it->second.toolID == toolID)
            return it->second.type;
      }
      return eItemInvalid;
   }

   static int GetCursorID(ItemTypeEnum type)
   {
      return FindOrFail(type)->cursorID;
   }

   static unsigned int GetAllowedViews(ItemTypeEnum type)
   {
      return FindOrFail(type)->allowedViews;
   }

private:
   static ankerl::unordered_dense::map<ItemTypeEnum, EditableInfo> m_map;

   static EditableInfo* FindOrFail(ItemTypeEnum type)
   {
      const ankerl::unordered_dense::map<ItemTypeEnum, EditableInfo>::iterator it = m_map.find(type);
      if (it == m_map.end())
      {
         ShowError("Editable type not found.");
         assert(false);
         return nullptr;
      }
      else
      {
         return &it->second;
      }
   }
};
