// license:GPLv3+

#pragma once

#include "unordered_dense.h"

typedef IEditable*(*CreateFuncType)();
typedef IEditable*(*CreateAndInitFuncType)(float x, float y);

struct EditableInfo
{
   ItemTypeEnum type;
   int typeNameID;
   CreateFuncType createFunc;
   CreateAndInitFuncType createAndInitFunc;
};

class EditableRegistry final
{
public:
   template <class T>
   static void RegisterEditable()
   {
      EditableInfo ei;
      ei.type = T::ItemType;
      ei.typeNameID = T::TypeNameID;
      ei.createFunc = &T::COMCreateEditable;
      ei.createAndInitFunc = &T::COMCreateAndInit;
      m_map[ei.type] = ei;
   }

   static IEditable* Create(ItemTypeEnum type)
   {
      const EditableInfo* const info = FindOrFail(type);
      return info->createFunc ? info->createFunc() : nullptr;
   }

   static IEditable* CreateAndInit(ItemTypeEnum type, PinTable *pt, float x, float y)
   {
      const EditableInfo* const info = FindOrFail(type);
      return info->createAndInitFunc ? info->createAndInitFunc(x, y) : nullptr;
   }

   static int GetTypeNameStringID(ItemTypeEnum type)
   {
      return FindOrFail(type)->typeNameID;
   }

private:
   static ankerl::unordered_dense::map<ItemTypeEnum, EditableInfo> m_map;

   static const EditableInfo* FindOrFail(ItemTypeEnum type)
   {
      const ankerl::unordered_dense::map<ItemTypeEnum, EditableInfo>::const_iterator it = m_map.find(type);
      if (it == m_map.end())
      {
         ShowError("Editable type not found.");
         assert(false);
         static constexpr EditableInfo emptyinfo = {};
         return &emptyinfo;
      }
      else
      {
         return &it->second;
      }
   }
};
