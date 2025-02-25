// license:GPLv3+

#include "core/stdafx.h"
#include "DynamicScript.h"

// Core types Ids
#define UNRESOLVED_TYPEID   0
#define VOID_TYPEID         1
#define BOOL_TYPEID         2
#define CHAR_TYPEID         3
#define SHORT_TYPEID        4
#define INT_TYPEID          5
#define LONG_TYPEID         6
#define UCHAR_TYPEID        7
#define USHORT_TYPEID       8
#define UINT_TYPEID         9
#define ULONG_TYPEID       10
#define FLOAT_TYPEID       11
#define DOUBLE_TYPEID      12
#define STRING_TYPEID      13

// Set to 1 to log all COM Invoke
#define LOG_INVOKES 0

DynamicTypeLibrary::DynamicTypeLibrary()
{
   m_types.push_back({ TypeDef::TD_NATIVE, ScriptTypeNameDef { nullptr, static_cast<unsigned int>(m_types.size()) } });
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
      if (type.nativeType.name != nullptr)
         m_typenames[type.nativeType.name] = type.nativeType.id;
}

DynamicTypeLibrary::~DynamicTypeLibrary()
{
   for (TypeDef& typeDef : m_types)
      if (typeDef.category == TypeDef::TD_CLASS)
         delete typeDef.classDef;
}

void DynamicTypeLibrary::RegisterScriptClass(ScriptClassDef* classDef)
{
   string classId(classDef->name.name);
   StrToLower(classId);
   const auto& existingType = m_typenames.find(classId);
   if (existingType != m_typenames.end())
   {
      // TODO Validate that both definitions are equal
      return;
   }

   // Shared object must implement reference counting, and for the sake of simplicity/efficiency, we impose the implementation to be at the beginning of the vtable
   if ((classDef->nMembers < 2)
      || (strcmp(classDef->members[0].name.name, "AddRef") != 0)
      || (strcmp(classDef->members[1].name.name, "Release") != 0))
   {
      PLOGE << "Plugin requested to register an invalid class '" << classDef->name.name << "' which does not implement reference counting in its first 2 members";
      return;
   }

   ClassDef* cd = new ClassDef();
   cd->classDef = classDef;

   // Add class to type library
   classDef->name.id = static_cast<unsigned int>(m_types.size());
   m_typenames[classId] = classDef->name.id;
   #if __cplusplus < 202002L
   TypeDef typeDef { TypeDef::TD_CLASS }; typeDef.classDef = cd;
   m_types.push_back(typeDef);
   #else
   m_types.push_back(TypeDef { .category = TypeDef::TD_CLASS, .classDef = cd });
   #endif

   // Register members
   for (int i = 0; i < static_cast<int>(classDef->nMembers); i++)
   {
      ScriptClassMemberDef& memberDef = classDef->members[i];
      string nameId(memberDef.name.name);
      StrToLower(nameId);
      const auto& memberMapEntry = cd->memberMap.find(nameId);
      if (memberMapEntry == cd->memberMap.end())
      {
         cd->memberMap[nameId] = static_cast<unsigned int>(cd->members.size());
         cd->members.push_back({ i });
      }
      else
      {
         cd->members[memberMapEntry->second].push_back(i);
      }
   }
}

void DynamicTypeLibrary::RegisterScriptTypeAlias(const char* name, const char* aliasedTypeName)
{
   const auto& existingType = m_typenames.find(name);
   if (existingType != m_typenames.end())
   {
      // TODO Validate that both definitions are the same
      return;
   }
   // Aliasing is only supported for core type, and forward declaration are not supported
   int aliasedId = ResolveClassId(aliasedTypeName);
   assert(aliasedId != UNRESOLVED_TYPEID);
   const TypeDef& typeDef = m_types[aliasedId];
   assert(typeDef.category == TypeDef::TD_NATIVE);
   #if __cplusplus < 202002L
   TypeDef typeDef2 { TypeDef::TD_ALIAS }; typeDef2.aliasDef = { name, typeDef.nativeType };
   m_types.push_back(typeDef2);
   #else
   m_types.push_back({ .category = TypeDef::TD_ALIAS, .aliasDef = { name, typeDef.nativeType } });
   #endif
   string classId(name);
   StrToLower(classId);
   m_typenames[classId] = typeDef.nativeType.id;
}

void DynamicTypeLibrary::RegisterScriptArray(ScriptArrayDef* arrayDef)
{
   const auto& existingType = m_typenames.find(arrayDef->name.name);
   if (existingType != m_typenames.end())
   {
      // TODO Validate that both definitions are the same
      return;
   }
   arrayDef->name.id = static_cast<unsigned int>(m_types.size());
   #if __cplusplus < 202002L
   TypeDef typeDef { TypeDef::TD_ARRAY }; typeDef.arrayDef = arrayDef;
   m_types.push_back(typeDef);
   #else
   m_types.push_back({ .category = TypeDef::TD_ARRAY, .arrayDef = arrayDef });
   #endif
   string classId(arrayDef->name.name);
   StrToLower(classId);
   m_typenames[classId] = arrayDef->name.id;
}

void DynamicTypeLibrary::ResolveAllClasses()
{
   for (auto type : m_types)
   {
      switch (type.category)
      {
      case TypeDef::TD_ALIAS:
         assert(type.aliasDef.typeDef.id != UNRESOLVED_TYPEID);
         break;

      case TypeDef::TD_NATIVE:
         assert(type.nativeType.name == nullptr || (type.nativeType.id != UNRESOLVED_TYPEID));
         break;

      case TypeDef::TD_CLASS:
         assert(type.classDef->classDef->name.id != UNRESOLVED_TYPEID);
         for (int i = 0; i < static_cast<int>(type.classDef->classDef->nMembers); i++)
         {
            ScriptClassMemberDef& memberDef = type.classDef->classDef->members[i];
            int retTypeId = ResolveClassId(memberDef.type.name);
            if (retTypeId < 0)
            {
               PLOGE << "Invalid type library: class " << type.classDef->classDef->name.name << " reference type " << memberDef.type.name << " which is not defined in the library";
            }
            else
               memberDef.type.id = retTypeId;
            for (unsigned int j = 0; j < memberDef.nArgs; j++)
            {
               int argTypeId = ResolveClassId(memberDef.callArgType[j].name);
               if (argTypeId < 0)
               {
                  PLOGE << "Invalid type library: class " << type.classDef->classDef->name.name << " reference type " << memberDef.callArgType[j].name << " which is not defined in the library";
               }
               else
                  memberDef.callArgType[j].id = argTypeId;
            }
         }
         break;

      case TypeDef::TD_ARRAY:
         type.arrayDef->type.id = ResolveClassId(type.arrayDef->type.name);
         break;

      default:
         break;
      }
   }
}

int DynamicTypeLibrary::ResolveClassId(const char* name) const
{
   string classId(name);
   StrToLower(classId);
   const auto& existingType = m_typenames.find(classId);
   if (existingType == m_typenames.end())
      return -1;
   return existingType->second;
}

ScriptClassDef* DynamicTypeLibrary::ResolveClass(const char* name) const
{
   int id = ResolveClassId(name);
   if (id < 0)
      return nullptr;
   const TypeDef& def = m_types[id];
   assert(def.category == TypeDef::TD_CLASS);
   assert(strcmp(def.classDef->classDef->name.name, name) == 0);
   return def.classDef->classDef;
}

int DynamicTypeLibrary::ResolveMemberId(const ScriptClassDef* classDef, const char* memberName) const
{
   assert(classDef->name.id != UNRESOLVED_TYPEID);
   assert(classDef->name.id < m_types.size());
   TypeDef type = m_types[classDef->name.id];
   assert(type.category == TypeDef::TD_CLASS);
   ClassDef* cd = type.classDef;
   string nameId(memberName);
   StrToLower(nameId);
   const auto& member = cd->memberMap.find(nameId);
   if (member != cd->memberMap.end())
      return member->second;
   return -1;
}

ScriptClassDef* DynamicTypeLibrary::GetClass(const ScriptTypeNameDef& name) const
{
   assert(name.id != UNRESOLVED_TYPEID);
   assert(name.id < static_cast<int>(m_types.size()));
   const TypeDef& def = m_types[name.id];
   assert(def.category == TypeDef::TD_CLASS);
   assert(strcmp(def.classDef->classDef->name.name, name.name) == 0);
   return def.classDef->classDef;
}

void DynamicTypeLibrary::COMToScriptVariant(const VARIANT& cv, const ScriptTypeNameDef& type, ScriptVariant& sv) const
{
   assert(type.id != UNRESOLVED_TYPEID);
   const TypeDef& typeDef = m_types[type.id];
   if (V_VT(&cv) == (VT_BYREF | VT_VARIANT))
   {
      COMToScriptVariant(*V_VARIANTREF(&cv), type, sv);
      return;
   }
   switch (typeDef.category)
   {
   case TypeDef::TD_ALIAS:
      COMToScriptVariant(cv, typeDef.aliasDef.typeDef, sv);
      break;

   case TypeDef::TD_NATIVE:
   {
      VARIANT v;
      VariantInit(&v);
      switch (typeDef.nativeType.id)
      {
      case UNRESOLVED_TYPEID:
      case VOID_TYPEID:
         PLOGE << "Class '" << type.name << "' is referenced while it is not registered in the type library. Did you forgot to call 'Register" << type.name << "SCD(...)' ?";
         assert(false);
         break;
      case BOOL_TYPEID: VariantChangeType(&v, &cv, 0, VT_BOOL); sv.vBool = static_cast<sc_bool>(V_BOOL(&v)); break;
      case CHAR_TYPEID: VariantChangeType(&v, &cv, 0, VT_I1); sv.vByte = V_I1(&v); break;
      case SHORT_TYPEID: VariantChangeType(&v, &cv, 0, VT_I2); sv.vShort = V_I2(&v); break;
      case INT_TYPEID: VariantChangeType(&v, &cv, 0, VT_I4); sv.vLong = V_I4(&v); break;
      case LONG_TYPEID: VariantChangeType(&v, &cv, 0, VT_I8); sv.vLong = static_cast<sc_int64>(V_I8(&v)); break;
      case UCHAR_TYPEID: VariantChangeType(&v, &cv, 0, VT_UI1); sv.vUByte = V_UI1(&v); break;
      case USHORT_TYPEID: VariantChangeType(&v, &cv, 0, VT_UI2); sv.vUShort = V_UI2(&v); break;
      case UINT_TYPEID: VariantChangeType(&v, &cv, 0, VT_UI4); sv.vUInt = V_UI4(&v); break;
      case ULONG_TYPEID: VariantChangeType(&v, &cv, 0, VT_UI8); sv.vULong = static_cast<sc_uint64>(V_UI8(&v)); break;
      case FLOAT_TYPEID: VariantChangeType(&v, &cv, 0, VT_R4); sv.vFloat = V_R4(&v); break;
      case DOUBLE_TYPEID: VariantChangeType(&v, &cv, 0, VT_R8); sv.vDouble = V_R8(&v); break;
      case STRING_TYPEID:
         switch (V_VT(&cv))
         {
         case VT_I2:
         {
            const string txt = std::to_string(V_I2(&cv));
            char* const szT = new char[txt.size() + 1];
            memcpy(szT, txt.c_str(), txt.size() + 1);
            sv.vString = szT;
            break;
         }
         case VT_I4:
         {
            const string txt = std::to_string(V_I4(&cv));
            char* const szT = new char[txt.size() + 1];
            memcpy(szT, txt.c_str(), txt.size() + 1);
            sv.vString = szT;
            break;
         }
         case VT_R8:
         {
            const string txt = std::to_string(V_R8(&cv));
            char* const szT = new char[txt.size() + 1];
            memcpy(szT, txt.c_str(), txt.size() + 1);
            sv.vString = szT;
            break;
         }
         case VT_BSTR:
         {
            const wstring wz(V_BSTR(&cv));
            const int len = (int)wz.length();
            char* const szT = new char[len + 1];
            WideCharToMultiByteNull(CP_ACP, 0, wz.c_str(), -1, szT, len + 1, nullptr, nullptr);
            sv.vString = szT;
            break;
         }
         default: assert(false);
         }
         break;
      }
      break;
   }

   case TypeDef::TD_CLASS:
   {
      assert(V_VT(&cv) == VT_DISPATCH);
      // Shouldn't we add a reference for the duration of the call ?
      // assert(static_cast<DynamicDispatch*>(V_DISPATCH(&cv))->m_classDef->name.id == type.id); // Can also be an inherited class
      sv.vObject = static_cast<DynamicDispatch*>(V_DISPATCH(&cv))->m_nativeObject;
      break;
   }

   case TypeDef::TD_ARRAY:
   {
      assert(V_VT(&cv) == (VT_ARRAY | VT_VARIANT));
      SAFEARRAY* psa = V_ARRAY(&cv);
      assert(typeDef.arrayDef->nDimensions == SafeArrayGetDim(psa));

      assert(typeDef.arrayDef->type.id != UNRESOLVED_TYPEID);
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
      case CHAR_TYPEID:
      {
         const int arraySize = sizeof(ScriptArray) + typeDef.arrayDef->nDimensions * sizeof(unsigned int) + (uBound - lBound + 1) * 1;
         array = static_cast<ScriptArray*>(malloc(arraySize));
         array->Release = [](ScriptArray* me) { free(me); };
         array->lengths[0] = static_cast<unsigned int>(uBound - lBound + 1);
         int8_t* pData = reinterpret_cast<int8_t*>(&array->lengths[1]);
         for (LONG i = lBound; i <= uBound; i++, pData++, p++)
         {
            assert(V_VT(p) == VT_I1);
            *pData = V_I1(p);
         }
         break;
      }
      case SHORT_TYPEID:
      {
         const int arraySize = sizeof(ScriptArray) + typeDef.arrayDef->nDimensions * sizeof(unsigned int) + (uBound - lBound + 1) * 2;
         array = static_cast<ScriptArray*>(malloc(arraySize));
         array->Release = [](ScriptArray* me) { free(me); };
         array->lengths[0] = static_cast<unsigned int>(uBound - lBound + 1);
         int16_t* pData = reinterpret_cast<int16_t*>(&array->lengths[1]);
         for (LONG i = lBound; i <= uBound; i++, pData++, p++)
         {
            assert(V_VT(p) == VT_I2);
            *pData = V_I2(p);
         }
         break;
      }
      case INT_TYPEID:
      {
         const int arraySize = sizeof(ScriptArray) + typeDef.arrayDef->nDimensions * sizeof(unsigned int) + (uBound - lBound + 1) * 4;
         array = static_cast<ScriptArray*>(malloc(arraySize));
         array->Release = [](ScriptArray* me) { free(me); };
         array->lengths[0] = static_cast<unsigned int>(uBound - lBound + 1);
         int32_t* pData = reinterpret_cast<int32_t*>(&array->lengths[1]);
         for (LONG i = lBound; i <= uBound; i++, pData++, p++)
         {
            assert(V_VT(p) == VT_I4);
            *pData = V_I4(p);
         }
         break;
      }
      case UCHAR_TYPEID:
      {
         const int arraySize = sizeof(ScriptArray) + typeDef.arrayDef->nDimensions * sizeof(unsigned int) + (uBound - lBound + 1) * 1;
         array = static_cast<ScriptArray*>(malloc(arraySize));
         array->Release = [](ScriptArray* me) { free(me); };
         array->lengths[0] = static_cast<unsigned int>(uBound - lBound + 1);
         uint8_t* pData = reinterpret_cast<uint8_t*>(&array->lengths[1]);
         for (LONG i = lBound; i <= uBound; i++, pData++, p++)
         {
            assert(V_VT(p) == VT_UI1);
            *pData = V_UI1(p);
         }
         break;
      }
      case USHORT_TYPEID:
      {
         const int arraySize = sizeof(ScriptArray) + typeDef.arrayDef->nDimensions * sizeof(unsigned int) + (uBound - lBound + 1) * 2;
         array = static_cast<ScriptArray*>(malloc(arraySize));
         array->Release = [](ScriptArray* me) { free(me); };
         array->lengths[0] = static_cast<unsigned int>(uBound - lBound + 1);
         uint16_t* pData = reinterpret_cast<uint16_t*>(&array->lengths[1]);
         for (LONG i = lBound; i <= uBound; i++, pData++, p++)
         {
            assert(V_VT(p) == VT_UI2);
            *pData = V_UI2(p);
         }
         break;
      }
      case UINT_TYPEID:
      {
         const int arraySize = sizeof(ScriptArray) + typeDef.arrayDef->nDimensions * sizeof(unsigned int) + (uBound - lBound + 1) * 4;
         array = static_cast<ScriptArray*>(malloc(arraySize));
         array->Release = [](ScriptArray* me) { free(me); };
         array->lengths[0] = static_cast<unsigned int>(uBound - lBound + 1);
         uint32_t* pData = reinterpret_cast<uint32_t*>(&array->lengths[1]);
         for (LONG i = lBound; i <= uBound; i++, pData++, p++)
         {
            assert(V_VT(p) == VT_UI4);
            *pData = V_UI4(p);
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
         delete [] sv.vString;
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

void DynamicTypeLibrary::ScriptToCOMVariant(const ScriptTypeNameDef& type, const ScriptVariant& sv, VARIANT& cv) const
{
   assert(type.id != UNRESOLVED_TYPEID);
   const TypeDef& typeDef = m_types[type.id];
   switch (typeDef.category)
   {
   case TypeDef::TD_ALIAS: ScriptToCOMVariant(const_cast<ScriptTypeNameDef&>(typeDef.aliasDef.typeDef), sv, cv); break;

   case TypeDef::TD_NATIVE:
      switch (typeDef.nativeType.id)
      {
      case UNRESOLVED_TYPEID:
      case VOID_TYPEID:
         PLOGE << "Class '" << type.name << "' is referenced while it is not registered in the type library. Did you forgot to call 'Register" << type.name << "SCD(...)' ?";
         V_VT(&cv) = VT_EMPTY;
         break;
      case BOOL_TYPEID:
         V_VT(&cv) = VT_BOOL;
         V_BOOL(&cv) = sv.vBool ? VARIANT_TRUE : VARIANT_FALSE;
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
         V_DISPATCH(&cv) = new DynamicDispatch(this, GetClass(type), sv.vObject);
         //V_DISPATCH(&cv)->AddRef(); // Don't add a ref, as the returned object already support reference counting and is returned refCounted for us
      }
      break;

   case TypeDef::TD_ARRAY:
   {
      if (typeDef.arrayDef->nDimensions == 1)
      {
         SAFEARRAY* psa = SafeArrayCreateVector(VT_VARIANT, typeDef.arrayDef->lowerBounds[0], sv.vArray->lengths[0]);
         VARIANT* pData;
         SafeArrayAccessData(psa, reinterpret_cast<void**>(&pData));
         assert(typeDef.arrayDef->type.id != UNRESOLVED_TYPEID);
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
               V_I1(&pData[i]) = pSrc[i];
            }
            break;
         }
         case SHORT_TYPEID:
         {
            int16_t* pSrc = reinterpret_cast<int16_t*>(&sv.vArray->lengths[1]);
            for (unsigned int i = 0; i < sv.vArray->lengths[0]; i++)
            {
               V_VT(&pData[i]) = VT_I2;
               V_I2(&pData[i]) = pSrc[i];
            }
            break;
         }
         case INT_TYPEID:
         {
            int32_t* pSrc = reinterpret_cast<int32_t*>(&sv.vArray->lengths[1]);
            for (unsigned int i = 0; i < sv.vArray->lengths[0]; i++)
            {
               V_VT(&pData[i]) = VT_I4;
               V_I4(&pData[i]) = pSrc[i];
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
         case USHORT_TYPEID:
         {
            uint16_t* pSrc = reinterpret_cast<uint16_t*>(&sv.vArray->lengths[1]);
            for (unsigned int i = 0; i < sv.vArray->lengths[0]; i++)
            {
               V_VT(&pData[i]) = VT_UI2;
               V_UI2(&pData[i]) = pSrc[i];
            }
            break;
         }
         case UINT_TYPEID:
         {
            uint32_t* pSrc = reinterpret_cast<uint32_t*>(&sv.vArray->lengths[1]);
            for (unsigned int i = 0; i < sv.vArray->lengths[0]; i++)
            {
               V_VT(&pData[i]) = VT_UI4;
               V_UI4(&pData[i]) = pSrc[i];
            }
            break;
         }
         default: assert(false); // not yet implemented
         }
         SafeArrayUnaccessData(psa);
         sv.vArray->Release(sv.vArray);
         V_VT(&cv) = VT_ARRAY | VT_VARIANT;
         V_ARRAY(&cv) = psa;
      }
      else if (typeDef.arrayDef->nDimensions == 2)
      {
         SAFEARRAYBOUND bounds[2] = { { sv.vArray->lengths[0],typeDef.arrayDef->lowerBounds[0] }, { sv.vArray->lengths[1], typeDef.arrayDef->lowerBounds[1] } };
         SAFEARRAY* psa = SafeArrayCreate(VT_VARIANT, 2, bounds);
         assert(typeDef.arrayDef->type.id != UNRESOLVED_TYPEID);
         const TypeDef& arrayTypeDef = m_types[typeDef.arrayDef->type.id];
         assert(arrayTypeDef.category == TypeDef::TD_NATIVE); // Other types are not yet supported
         VARIANT varValue;
         V_VT(&varValue) = VT_I4;
         switch (arrayTypeDef.nativeType.id)
         {
         case INT_TYPEID:
         {
            int32_t* pSrc = reinterpret_cast<int32_t*>(&sv.vArray->lengths[2]);
            for (unsigned int i = 0; i < sv.vArray->lengths[0]; i++)
            {
               LONG ix[2];
               ix[0] = i;
               for (unsigned int j = 0; j < sv.vArray->lengths[1]; j++)
               {
                  ix[1] = j;
                  V_I4(&varValue) = pSrc[i * sv.vArray->lengths[1] + j];
                  SafeArrayPutElement(psa, ix, &varValue);
               }
            }
            break;
         }
         default: assert(false); // not yet implemented
         }
         sv.vArray->Release(sv.vArray);
         V_VT(&cv) = VT_ARRAY | VT_VARIANT;
         V_ARRAY(&cv) = psa;
      }
      else
      {
         assert(false);
      }
      break;
   }
   default:
      break;
   }
}

string DynamicTypeLibrary::ScriptVariantToString(const ScriptTypeNameDef& type, const ScriptVariant& sv) const
{
   assert(type.id != UNRESOLVED_TYPEID);
   const TypeDef& typeDef = m_types[type.id];
   switch (typeDef.category)
   {
   case TypeDef::TD_ALIAS: return ScriptVariantToString(const_cast<ScriptTypeNameDef&>(typeDef.aliasDef.typeDef), sv);

   case TypeDef::TD_NATIVE:
      switch (typeDef.nativeType.id)
      {
      case UNRESOLVED_TYPEID: return "Invalid"s;
      case VOID_TYPEID: return "void"s;
      case BOOL_TYPEID: return sv.vBool ? "true"s : "false"s;
      case CHAR_TYPEID: return std::to_string(sv.vByte);
      case SHORT_TYPEID: return std::to_string(sv.vShort);
      case INT_TYPEID: return std::to_string(sv.vInt);
      case LONG_TYPEID: return std::to_string(sv.vLong);
      case UCHAR_TYPEID: return std::to_string(sv.vUByte);
      case USHORT_TYPEID: return std::to_string(sv.vUShort);
      case UINT_TYPEID: return std::to_string(sv.vUInt);
      case ULONG_TYPEID: return std::to_string(sv.vULong);
      case FLOAT_TYPEID: return std::to_string(sv.vFloat);
      case DOUBLE_TYPEID: return std::to_string(sv.vDouble);
      case STRING_TYPEID: return "\""s.append(sv.vString).append(1,'"');
      default: assert(false);
      }
      break;

   case TypeDef::TD_CLASS: return GetClass(type)->name.name;

   case TypeDef::TD_ARRAY: return typeDef.arrayDef->name.name;

   default:
      break;
   }

   return "<< bug >>"s;
}

HRESULT DynamicTypeLibrary::Invoke(const ScriptClassDef * classDef, void* nativeObject, DISPID dispIdMember, REFIID, LCID, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO*, UINT*) const
{
   assert(classDef->name.id != UNRESOLVED_TYPEID);
   assert(classDef->name.id < m_types.size());
   //if (pDispParams->cNamedArgs != 0)
   //   return DISP_E_NONAMEDARGS;
   if (pDispParams->cArgs > PSC_CALL_MAX_ARG_COUNT)
      return DISP_E_BADPARAMCOUNT;
   TypeDef type = m_types[classDef->name.id];
   assert(type.category == TypeDef::TD_CLASS);
   ClassDef * cd = type.classDef;
   if ((dispIdMember < 0) || (dispIdMember >= (DISPID)cd->members.size()))
      return DISP_E_MEMBERNOTFOUND;

   // Search for the right overload (needed for property which are a member with overloads for getters and setters)
   // FIXME match overload on type and arguments
   const std::vector<int>& members = cd->members[dispIdMember];
   int memberIndex = -1;
   if (wFlags & DISPATCH_METHOD)
   {
      for (int i : members)
      {
         const ScriptClassMemberDef& memberDef = cd->classDef->members[i];
         if (memberDef.nArgs == pDispParams->cArgs)
         {
            memberIndex = i;
            break;
         }
      }
   }
   else if (wFlags & DISPATCH_PROPERTYGET)
   {
      for (int i : members)
      {
         const ScriptClassMemberDef& memberDef = cd->classDef->members[i];
         if ((memberDef.type.id != VOID_TYPEID) && (memberDef.nArgs == pDispParams->cArgs))
         {
            memberIndex = i;
            break;
         }
      }
   }
   else if (wFlags & DISPATCH_PROPERTYPUT)
   {
      for (int i : members)
      {
         const ScriptClassMemberDef& memberDef = cd->classDef->members[i];
         if ((memberDef.type.id == VOID_TYPEID) && (memberDef.nArgs == pDispParams->cArgs))
         {
            memberIndex = i;
            break;
         }
      }
   }
   else if (wFlags & DISPATCH_PROPERTYPUTREF)
   {
      // TODO implement (when is this used ? Set obj1.prop = obj2 ?)
   }
   if (memberIndex == -1)
   {
      bool first = true;
      for (int i : members)
      {
         if (first)
         {
            PLOGE << "Invalid member called " << classDef->name.name << '.' << cd->classDef->members[i].name.name << "(...)";
            first = false;
         }
         //PLOGE << "> Not matched overload: " << classDef->name.name << '.' << cd->classDef->members[i].name.name << "(...)";
      }
      return DISP_E_MEMBERNOTFOUND;
   }
   const ScriptClassMemberDef& memberDef = classDef->members[memberIndex];
   if ((memberDef.type.id != VOID_TYPEID) && (pVarResult == nullptr))
      return E_POINTER;

   ScriptVariant args[PSC_CALL_MAX_ARG_COUNT];
   for (unsigned int i = 0; i < pDispParams->cArgs; i++)
      COMToScriptVariant(pDispParams->rgvarg[pDispParams->cArgs - 1 - i], memberDef.callArgType[i], args[i]);

   #if LOG_INVOKES
      std::stringstream ss;
      ss << "Invoke Call: " << classDef->name.name << '.' << memberDef.name.name << '(';
      for (unsigned int i = 0; i < pDispParams->cArgs; i++)
         ss << ((i != 0) ? ", " : "") << ScriptVariantToString(memberDef.callArgType[i], args[i]).c_str();
      PLOGD << ss.str() << ')';
   #endif

   ScriptVariant retValue;
   try
   {
      memberDef.Call(nativeObject, memberIndex, args, &retValue);
   }
   catch (...)
   {
      PLOGE << "Exception occured while processing script call";
      // As C++ does not have a 'finally' exception clause
      for (unsigned int i = 0; i < pDispParams->cArgs; i++)
         ReleaseCOMToScriptVariant(pDispParams->rgvarg[pDispParams->cArgs - 1 - i], memberDef.callArgType[i], args[i]);
      return DISP_E_EXCEPTION;
   }
   for (unsigned int i = 0; i < pDispParams->cArgs; i++)
      ReleaseCOMToScriptVariant(pDispParams->rgvarg[pDispParams->cArgs - 1 - i], memberDef.callArgType[i], args[i]);

   if (memberDef.type.id != VOID_TYPEID)
      ScriptToCOMVariant(memberDef.type, retValue, *pVarResult);

   return S_OK;
}
