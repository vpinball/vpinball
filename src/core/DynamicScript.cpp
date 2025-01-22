// license:GPLv3+

#include "core/stdafx.h"
#include "DynamicScript.h"

// Core types Ids
#define INVALID_TYPEID   0
#define VOID_TYPEID      1
#define BOOL_TYPEID      2
#define CHAR_TYPEID      3
#define SHORT_TYPEID     4
#define INT_TYPEID       5
#define LONG_TYPEID      6
#define UCHAR_TYPEID     7
#define USHORT_TYPEID    8
#define UINT_TYPEID      9
#define ULONG_TYPEID     10
#define FLOAT_TYPEID     11
#define DOUBLE_TYPEID    12
#define STRING_TYPEID    13


DynamicTypeLibrary::DynamicTypeLibrary()
{
   m_types.push_back({ TypeDef::TD_NATIVE, ScriptTypeNameDef { "", static_cast<unsigned int>(m_types.size()) } });
   m_types.push_back({ TypeDef::TD_NATIVE, ScriptTypeNameDef { "void", static_cast<unsigned int>(m_types.size()) } });
   m_types.push_back({ TypeDef::TD_NATIVE, ScriptTypeNameDef { "bool", static_cast<unsigned int>(m_types.size()) } });
   m_types.push_back({ TypeDef::TD_NATIVE, ScriptTypeNameDef { "char", static_cast<unsigned int>(m_types.size()) } });
   m_types.push_back({ TypeDef::TD_NATIVE, ScriptTypeNameDef { "short", static_cast<unsigned int>(m_types.size()) } });
   m_types.push_back({ TypeDef::TD_NATIVE, ScriptTypeNameDef { "int", static_cast<unsigned int>(m_types.size()) } });
   m_types.push_back({ TypeDef::TD_NATIVE, ScriptTypeNameDef { "long", static_cast<unsigned int>(m_types.size()) } });
   m_types.push_back({ TypeDef::TD_NATIVE, ScriptTypeNameDef { "uchar", static_cast<unsigned int>(m_types.size()) } });
   m_types.push_back({ TypeDef::TD_NATIVE, ScriptTypeNameDef { "ushort", static_cast<unsigned int>(m_types.size()) } });
   m_types.push_back({ TypeDef::TD_NATIVE, ScriptTypeNameDef { "uint", static_cast<unsigned int>(m_types.size()) } });
   m_types.push_back({ TypeDef::TD_NATIVE, ScriptTypeNameDef { "ulong", static_cast<unsigned int>(m_types.size()) } });
   m_types.push_back({ TypeDef::TD_NATIVE, ScriptTypeNameDef { "float", static_cast<unsigned int>(m_types.size()) } });
   m_types.push_back({ TypeDef::TD_NATIVE, ScriptTypeNameDef { "double", static_cast<unsigned int>(m_types.size()) } });
   m_types.push_back({ TypeDef::TD_NATIVE, ScriptTypeNameDef { "string", static_cast<unsigned int>(m_types.size()) } });
   for (const TypeDef& type : m_types)
      m_typenames[type.nativeType.name] = type.nativeType.id;
}

void DynamicTypeLibrary::RegisterScriptClass(ScriptClassDef* classDef)
{
   const auto& type = m_typenames.find(classDef->name.name);
   if (type != m_typenames.end())
   {
      // TODO Validate that both definitions are the same
      return;
   }
   classDef->name.id = static_cast<unsigned int>(m_types.size());
   #ifdef ENABLE_DX9
   TypeDef typeDef { TypeDef::TD_CLASS }; typeDef.classDef = classDef;
   m_types.push_back(typeDef);
   #else
   m_types.push_back(TypeDef { .category = TypeDef::TD_CLASS, .classDef = classDef });
   #endif
   m_typenames[classDef->name.name] = classDef->name.id;

   // Register members and resolve types (if possible, since forward declaration may happen and will be lazily resolved)
   for (unsigned int i = 0; i < classDef->nMembers; i++)
   {
      ScriptClassMemberDef& memberDef = classDef->members[i];
      memberDef.name.id = static_cast<unsigned int>(m_scriptMembers.size());
      ResolveClassId(memberDef.type);
      for (int j = 0; j < 6; j++)
         ResolveClassId(memberDef.callArgType[j]);
      m_scriptMembers.push_back(std::pair(classDef, i));
   }
   // Shared object must implement reference counting, and for the sake of simplicity/efficiency, we impose the implementation to be at the beginning of the vtable
   assert(classDef->nMembers >= 2);
   assert(strcmp(classDef->members[0].name.name, "AddRef") == 0);
   assert(strcmp(classDef->members[1].name.name, "Release") == 0);
}

void DynamicTypeLibrary::RegisterScriptTypeAlias(const char* name, const char* aliasedTypeName)
{
   const auto& type = m_typenames.find(name);
   if (type != m_typenames.end())
   {
      // TODO Validate that both definitions are the same
      return;
   }
   // Aliasing is only supported for core type, and forward declaration are not supported
   ScriptTypeNameDef aliasedType { aliasedTypeName, 0 };
   ResolveClassId(aliasedType);
   assert(aliasedType.id != 0);
   const TypeDef& typeDef = m_types[aliasedType.id];
   assert(typeDef.category == TypeDef::TD_NATIVE);
   #ifdef ENABLE_DX9
   TypeDef typeDef2 { TypeDef::TD_ALIAS }; typeDef2.aliasDef = { name, typeDef.nativeType };
   m_types.push_back(typeDef2);
   #else
   m_types.push_back({ .category = TypeDef::TD_ALIAS, .aliasDef = { name, typeDef.nativeType } });
   #endif
   m_typenames[name] = typeDef.nativeType.id;
}

void DynamicTypeLibrary::RegisterScriptArray(ScriptArrayDef* arrayDef)
{
   const auto& type = m_typenames.find(arrayDef->name.name);
   if (type != m_typenames.end())
   {
      // TODO Validate that both definitions are the same
      return;
   }
   ResolveClassId(arrayDef->type);
   arrayDef->name.id = static_cast<unsigned int>(m_types.size());
   #ifdef ENABLE_DX9
   TypeDef typeDef { TypeDef::TD_ARRAY }; typeDef.arrayDef = typeDef.arrayDef;
   m_types.push_back(typeDef);
   #else
   m_types.push_back({ .category = TypeDef::TD_ARRAY, .arrayDef = arrayDef });
   #endif
   m_typenames[arrayDef->name.name] = arrayDef->name.id;
}

ScriptClassDef* DynamicTypeLibrary::ResolveClass(ScriptTypeNameDef& name) const
{
   if (name.id == 0)
      ResolveClassId(name);
   return GetClassDef(const_cast<const ScriptTypeNameDef&>(name));
}

ScriptClassDef* DynamicTypeLibrary::GetClassDef(const ScriptTypeNameDef& name) const
{
   assert((name.id != 0) && (name.id < static_cast<int>(m_types.size())));
   const TypeDef& def = m_types[name.id];
   assert(def.category == TypeDef::TD_CLASS);
   assert(strcmp(def.classDef->name.name, name.name) == 0);
   return def.classDef;
}

void DynamicTypeLibrary::ResolveClassId(ScriptTypeNameDef& typeName) const
{
   if ((typeName.id == 0) && (typeName.name != nullptr))
   {
      const auto& type = m_typenames.find(typeName.name);
      if (type != m_typenames.end())
         typeName.id = type->second;
   }
}

void DynamicTypeLibrary::COMToScriptVariant(const VARIANT& cv, ScriptTypeNameDef& type, ScriptVariant& sv) const
{
   ResolveClassId(type);
   const TypeDef& typeDef = m_types[type.id];
   switch (typeDef.category)
   {
   case TypeDef::TD_ALIAS:
      COMToScriptVariant(cv, const_cast<ScriptTypeNameDef&>(typeDef.aliasDef.typeDef), sv);
      break;

   case TypeDef::TD_NATIVE:
      switch (typeDef.nativeType.id)
      {
      case INVALID_TYPEID:
      case VOID_TYPEID:
         PLOGE << "Class '" << type.name << "' is referenced while it is not registered in the type library. Did you forgot to call 'Register" << type.name << "SCD(...)' ?";
         assert(false);
         break;
      case BOOL_TYPEID:
         assert(V_VT(&cv) == VT_BOOL);
         sv.vBool = static_cast<bool>(V_BOOL(&cv));
         break;
      case CHAR_TYPEID:
         assert(V_VT(&cv) == VT_I1);
         sv.vByte = V_I1(&cv);
         break;
      case SHORT_TYPEID:
         assert(V_VT(&cv) == VT_I2);
         sv.vShort = V_I2(&cv);
         break;
      case INT_TYPEID:
         switch (V_VT(&cv))
         {
         case VT_I1: sv.vInt = V_I1(&cv); break;
         case VT_I2: sv.vInt = V_I2(&cv); break;
         case VT_I4: sv.vInt = V_I4(&cv); break;
         case VT_UI1: sv.vInt = V_UI1(&cv); break;
         case VT_UI2: sv.vInt = V_UI2(&cv); break;
         case VT_UI4: sv.vInt = V_UI4(&cv); break;
         default: assert(false);
         }
         break;
      case LONG_TYPEID:
         assert(V_VT(&cv) == VT_I8);
         sv.vLong = static_cast<sc_int64>(V_I8(&cv));
         break;
      case UCHAR_TYPEID:
         assert(V_VT(&cv) == VT_UI1);
         sv.vByte = V_UI1(&cv);
         break;
      case USHORT_TYPEID:
         assert(V_VT(&cv) == VT_UI2);
         sv.vShort = V_UI2(&cv);
         break;
      case UINT_TYPEID:
         switch (V_VT(&cv))
         {
         case VT_I1: sv.vUInt = V_I1(&cv); break;
         case VT_I2: sv.vUInt = V_I2(&cv); break;
         case VT_I4: sv.vUInt = V_I4(&cv); break;
         case VT_UI1: sv.vUInt = V_UI1(&cv); break;
         case VT_UI2: sv.vUInt = V_UI2(&cv); break;
         case VT_UI4: sv.vUInt = V_UI4(&cv); break;
         default: assert(false);
         }
         break;
      case ULONG_TYPEID:
         assert(V_VT(&cv) == VT_UI8);
         sv.vULong = static_cast<sc_uint64>(V_UI8(&cv));
         break;
      case FLOAT_TYPEID:
         switch (V_VT(&cv))
         {
         case VT_I1: sv.vFloat = V_I1(&cv); break;
         case VT_I2: sv.vFloat = V_I2(&cv); break;
         case VT_I4: sv.vFloat = static_cast<float>(V_I4(&cv)); break;
         case VT_UI1: sv.vFloat = V_UI1(&cv); break;
         case VT_UI2: sv.vFloat = V_UI2(&cv); break;
         case VT_UI4: sv.vFloat = static_cast<float>(V_UI4(&cv)); break;
         case VT_R4: sv.vFloat = V_R4(&cv); break;
         case VT_R8: sv.vFloat = static_cast<float>(V_R8(&cv)); break;
         default: assert(false);
         }
         break;
      case DOUBLE_TYPEID:
         switch (V_VT(&cv))
         {
         case VT_I1: sv.vDouble = V_I1(&cv); break;
         case VT_I2: sv.vDouble = V_I2(&cv); break;
         case VT_I4: sv.vDouble = V_I4(&cv); break;
         case VT_UI1: sv.vDouble = V_UI1(&cv); break;
         case VT_UI2: sv.vDouble = V_UI2(&cv); break;
         case VT_UI4: sv.vDouble = V_UI4(&cv); break;
         case VT_R4: sv.vDouble = V_R4(&cv); break;
         case VT_R8: sv.vDouble = V_R8(&cv); break;
         default: assert(false);
         }
         break;
      case STRING_TYPEID:
      {
         assert(V_VT(&cv) == VT_BSTR);
         const wstring wz(V_BSTR(&cv));
         const int len = (int)wz.length();
         char* const szT = new char[len + 1];
         WideCharToMultiByteNull(CP_ACP, 0, wz.c_str(), -1, szT, len + 1, nullptr, nullptr);
         sv.vString = szT;
         break;
      }
      }
      break;

   case TypeDef::TD_CLASS:
   {
      DynamicDispatch* dd = nullptr;
      switch (V_VT(&cv))
      {
      case VT_DISPATCH:
         dd = static_cast<DynamicDispatch*>(V_DISPATCH(&cv));
         break;
      case VT_VARIANT | VT_BYREF:
         assert(V_VT(V_VARIANTREF(&cv)) == VT_DISPATCH);
         dd = static_cast<DynamicDispatch*>(V_DISPATCH(V_VARIANTREF(&cv)));
         break;
      }
      // assert(dd->m_classDef->name.id == type.id); // Can also be an inherited class
      // Shouldn't we add a reference for the duration of the call ?
      if (dd != nullptr)
         sv.vObject = dd->m_nativeObject;
      else
         sv.vObject = nullptr;
      break;
   }

   case TypeDef::TD_ARRAY:
   {
      assert(V_VT(&cv) == (VT_ARRAY | VT_VARIANT));
      SAFEARRAY* psa = V_ARRAY(&cv);
      assert(typeDef.arrayDef->nDimensions == SafeArrayGetDim(psa));

      ResolveClassId(typeDef.arrayDef->type);
      const TypeDef& arrayTypeDef = m_types[typeDef.arrayDef->type.id];
      assert(arrayTypeDef.category == TypeDef::TD_NATIVE); // Other types are not yet supported
      assert(typeDef.arrayDef->nDimensions == 1);
      LONG lBound, uBound;
      SafeArrayGetLBound(psa, 1, &lBound);
      SafeArrayGetUBound(psa, 1, &uBound);
      VARIANT* p;
      SafeArrayAccessData(psa, (void**)&p);
      ScriptArray* array = nullptr;
      switch (arrayTypeDef.nativeType.id)
      {
      case SHORT_TYPEID:
      {
         int arraySize = sizeof(ScriptArray) + typeDef.arrayDef->nDimensions * sizeof(unsigned int) + (uBound - lBound + 1) * 2;
         array = static_cast<ScriptArray*>(malloc(arraySize));
         array->Release = [](ScriptArray* me) { free(me); };
         array->lengths[0] = static_cast<unsigned int>(uBound - lBound + 1);
         short* pData = reinterpret_cast<short*>(&array->lengths[1]);
         for (LONG i = lBound; i <= uBound; i++, pData++, p++)
         {
            assert(V_VT(p) == VT_I2);
            *pData = V_I2(p);
         }
         break;
      }

      default: assert(false);
      }
      SafeArrayUnaccessData(psa);
      sv.vArray = array;
      break;
   }

   default: assert(false);
   }
}

void DynamicTypeLibrary::ReleaseCOMToScriptVariant(VARIANT& cv, const ScriptTypeNameDef& type, ScriptVariant& sv) const
{
   // TODO handle ByRef variants (update the COM variant from the script variant)
   const TypeDef& typeDef = m_types[type.id];
   switch (typeDef.category)
   {
   case TypeDef::TD_ALIAS:
      ReleaseCOMToScriptVariant(cv, const_cast<ScriptTypeNameDef&>(typeDef.aliasDef.typeDef), sv);
      break;

   case TypeDef::TD_NATIVE:
      switch (typeDef.nativeType.id)
      {
      case STRING_TYPEID:
         delete sv.vString;
         sv.vString = nullptr;
         break;
      }
      break;

   case TypeDef::TD_CLASS:
      break;

   case TypeDef::TD_ARRAY:
      sv.vArray->Release(sv.vArray);
      break;

   default: assert(false);
   }
}

void DynamicTypeLibrary::ScriptToCOMVariant(ScriptTypeNameDef& type, const ScriptVariant& sv, VARIANT& cv) const
{
   ResolveClassId(type);
   const TypeDef& typeDef = m_types[type.id];
   switch (typeDef.category)
   {
   case TypeDef::TD_ALIAS: ScriptToCOMVariant(const_cast<ScriptTypeNameDef&>(typeDef.aliasDef.typeDef), sv, cv); break;

   case TypeDef::TD_NATIVE:
      switch (typeDef.nativeType.id)
      {
      case INVALID_TYPEID:
      case VOID_TYPEID:
         PLOGE << "Class '" << type.name << "' is referenced while it is not registered in the type library. Did you forgot to call 'Register" << type.name << "SCD(...)' ?";
         V_VT(&cv) = VT_EMPTY;
         break;
      case BOOL_TYPEID:
         V_VT(&cv) = VT_BOOL;
         V_BOOL(&cv) = sv.vBool;
         break;
      case CHAR_TYPEID:
         V_VT(&cv) = VT_I1;
         V_I1(&cv) = sv.vByte;
         break;
      case SHORT_TYPEID:
         V_VT(&cv) = VT_I2;
         V_I2(&cv) = sv.vShort;
         break;
      case INT_TYPEID:
         V_VT(&cv) = VT_I4;
         V_I4(&cv) = sv.vInt;
         break;
      case LONG_TYPEID:
         V_VT(&cv) = VT_I8;
         V_I8(&cv) = sv.vLong;
         break;
      case UCHAR_TYPEID:
         V_VT(&cv) = VT_UI1;
         V_UI1(&cv) = sv.vUByte;
         break;
      case USHORT_TYPEID:
         V_VT(&cv) = VT_UI2;
         V_UI2(&cv) = sv.vUShort;
         break;
      case UINT_TYPEID:
         V_VT(&cv) = VT_UI4;
         V_UI4(&cv) = sv.vUInt;
         break;
      case ULONG_TYPEID:
         V_VT(&cv) = VT_UI8;
         V_UI8(&cv) = sv.vULong;
         break;
      case FLOAT_TYPEID:
         V_VT(&cv) = VT_R4;
         V_R4(&cv) = sv.vFloat;
         break;
      case DOUBLE_TYPEID:
         V_VT(&cv) = VT_R8;
         V_R8(&cv) = sv.vDouble;
         break;
      case STRING_TYPEID:
      {
         V_VT(&cv) = VT_BSTR;
         const int len = MultiByteToWideChar(CP_ACP, 0, sv.vString, -1, nullptr, 0);
         V_BSTR(&cv) = SysAllocStringLen(nullptr, len);
         MultiByteToWideChar(CP_ACP, 0, sv.vString, -1, V_BSTR(&cv), len);
         break;
      }
      default: assert(false);
      }
      break;

   case TypeDef::TD_CLASS:
      if (sv.vObject == nullptr)
      {
         V_VT(&cv) = VT_NULL;
      }
      else
      {
         V_VT(&cv) = VT_DISPATCH;
         V_DISPATCH(&cv) = new DynamicDispatch(this, GetClassDef(type), sv.vObject);
         V_DISPATCH(&cv)->AddRef();
      }
      break;

   case TypeDef::TD_ARRAY:
   {
      assert(typeDef.arrayDef->nDimensions == 1); // Other sizes are not yet implemented
      SAFEARRAY* psa = SafeArrayCreateVector(VT_VARIANT, typeDef.arrayDef->lowerBounds[0], sv.vArray->lengths[0]);
      VARIANT* pData;
      SafeArrayAccessData(psa, reinterpret_cast<void**>(&pData));
      ResolveClassId(typeDef.arrayDef->type);
      const TypeDef& arrayTypeDef = m_types[typeDef.arrayDef->type.id];
      assert(arrayTypeDef.category == TypeDef::TD_NATIVE); // Other types are not yet supported
      switch (arrayTypeDef.nativeType.id)
      {
      case CHAR_TYPEID:
      {
         int8_t* pSrc = reinterpret_cast<int8_t*>(&sv.vArray->lengths[1]);
         for (unsigned int i = 0; i < sv.vArray->lengths[0]; i++)
         {
            V_VT(&pData[i]) = VT_I1;
            V_UI1(&pData[i]) = pSrc[i];
         }
         break;
      }
      case INT_TYPEID:
      {
         int32_t* pSrc = reinterpret_cast<int32_t*>(&sv.vArray->lengths[1]);
         for (unsigned int i = 0; i < sv.vArray->lengths[0]; i++)
         {
            V_VT(&pData[i]) = VT_I4;
            V_UI1(&pData[i]) = pSrc[i];
         }
         break;
      }
      case UCHAR_TYPEID:
      {
         uint8_t* pSrc = reinterpret_cast<uint8_t*>(&sv.vArray->lengths[1]);
         for (unsigned int i = 0; i < sv.vArray->lengths[0]; i++)
         {
            V_VT(&pData[i]) = VT_UI1;
            V_UI1(&pData[i]) = pSrc[i];
         }
         break;
      }
      case UINT_TYPEID:
      {
         uint32_t* pSrc = reinterpret_cast<uint32_t*>(&sv.vArray->lengths[1]);
         for (unsigned int i = 0; i < sv.vArray->lengths[0]; i++)
         {
            V_VT(&pData[i]) = VT_UI4;
            V_UI1(&pData[i]) = pSrc[i];
         }
         break;
      }
      default: assert(false); // not yet implemented
      }
      SafeArrayUnaccessData(psa);
      sv.vArray->Release(sv.vArray);
      V_VT(&cv) = VT_ARRAY | VT_VARIANT;
      V_ARRAY(&cv) = psa;
      break;
   }
   }
}


HRESULT STDMETHODCALLTYPE DynamicTypeLibrary::Invoke(void* nativeObject, DISPID dispIdMember, REFIID, LCID, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO*, UINT*) const
{
   assert(0 <= dispIdMember && dispIdMember < m_scriptMembers.size());
   try
   {
      auto member = m_scriptMembers[dispIdMember];
      ScriptClassDef* classDef = member.first;
      ScriptClassMemberDef& memberDef = classDef->members[member.second];
      PLOGD << "Invoke: " << classDef->name.name << "." << memberDef.name.name;
      if ((wFlags & DISPATCH_PROPERTYGET) && (memberDef.Get != nullptr))
      {
         if (pVarResult == nullptr)
            return E_POINTER;
         ScriptVariant value;
         memberDef.Get(nativeObject, member.second, &value);
         ScriptToCOMVariant(memberDef.type, value, *pVarResult);
         return S_OK;
      }
      if ((wFlags & DISPATCH_PROPERTYPUT) && (memberDef.Set != nullptr))
      {
         if (!pDispParams || pDispParams->cArgs != 1)
            return DISP_E_BADPARAMCOUNT;
         ScriptVariant value;
         assert(pDispParams->cArgs == 1);
         COMToScriptVariant(pDispParams->rgvarg[0], memberDef.type, value);
         memberDef.Set(nativeObject, member.second, &value);
         ReleaseCOMToScriptVariant(pDispParams->rgvarg[0], memberDef.type, value);
         return S_OK;
      }
      if ((wFlags & DISPATCH_METHOD) && (memberDef.Call != nullptr))
      {
         assert(pDispParams != nullptr);
         if (pVarResult == nullptr && (memberDef.type.id != VOID_TYPEID))
            return E_POINTER;
         ScriptVariant retVal;
         ScriptVariant args[6];
         assert(pDispParams->cNamedArgs == 0);
         for (unsigned int i = 0; i < pDispParams->cArgs; i++)
            COMToScriptVariant(pDispParams->rgvarg[pDispParams->cArgs - 1 - i], memberDef.callArgType[i], args[i]);
         memberDef.Call(nativeObject, member.second, &retVal, args);
         if (pVarResult != nullptr)
            ScriptToCOMVariant(memberDef.type, retVal, *pVarResult);
         for (unsigned int i = 0; i < pDispParams->cArgs; i++)
            ReleaseCOMToScriptVariant(pDispParams->rgvarg[pDispParams->cArgs - 1 - i], memberDef.callArgType[i], args[i]);
         return S_OK;
      }
   }
   catch (...)
   {
      return DISP_E_EXCEPTION;
   }
   return DISP_E_MEMBERNOTFOUND;
}
