// license:GPLv3+

#include "core/stdafx.h"
#include "DynamicScript.h"

// Core types Ids
enum TypeID
{
   TYPEID_UNRESOLVED,
   TYPEID_VOID,
   TYPEID_BOOL,
   TYPEID_INT,
   TYPEID_UINT,
   TYPEID_FLOAT,
   TYPEID_DOUBLE,
   TYPEID_STRING,
   TYPEID_INT8,
   TYPEID_INT16,
   TYPEID_INT32,
   TYPEID_INT64,
   TYPEID_UINT8,
   TYPEID_UINT16,
   TYPEID_UINT32,
   TYPEID_UINT64,
   TYPEID_COUNT
};

// Set to 1 to log all COM Invoke
#define LOG_INVOKES 0

DynamicTypeLibrary::DynamicTypeLibrary()
{
   m_types.resize(TypeID::TYPEID_COUNT);
   m_types[TypeID::TYPEID_UNRESOLVED] = { TypeDef::TD_NATIVE, ScriptTypeNameDef { nullptr, TYPEID_UNRESOLVED } };
   // Base native type
   m_types[TypeID::TYPEID_VOID  ] = { TypeDef::TD_NATIVE, ScriptTypeNameDef { "void",   TYPEID_VOID } };
   m_types[TypeID::TYPEID_BOOL  ] = { TypeDef::TD_NATIVE, ScriptTypeNameDef { "bool",   TYPEID_BOOL } };
   m_types[TypeID::TYPEID_INT   ] = { TypeDef::TD_NATIVE, ScriptTypeNameDef { "int",    TYPEID_INT } };
   m_types[TypeID::TYPEID_UINT  ] = { TypeDef::TD_NATIVE, ScriptTypeNameDef { "uint",   TYPEID_UINT } };
   m_types[TypeID::TYPEID_FLOAT ] = { TypeDef::TD_NATIVE, ScriptTypeNameDef { "float",  TYPEID_FLOAT } };
   m_types[TypeID::TYPEID_DOUBLE] = { TypeDef::TD_NATIVE, ScriptTypeNameDef { "double", TYPEID_DOUBLE } };
   m_types[TypeID::TYPEID_STRING] = { TypeDef::TD_NATIVE, ScriptTypeNameDef { "string", TYPEID_STRING } };
   // Sized data type
   m_types[TypeID::TYPEID_INT8  ] = { TypeDef::TD_NATIVE, ScriptTypeNameDef { "int8",   TYPEID_INT8 } };
   m_types[TypeID::TYPEID_INT16 ] = { TypeDef::TD_NATIVE, ScriptTypeNameDef { "int16",  TYPEID_INT16 } };
   m_types[TypeID::TYPEID_INT32 ] = { TypeDef::TD_NATIVE, ScriptTypeNameDef { "int32",  TYPEID_INT32 } };
   m_types[TypeID::TYPEID_INT64 ] = { TypeDef::TD_NATIVE, ScriptTypeNameDef { "int64",  TYPEID_INT64 } };
   m_types[TypeID::TYPEID_UINT8 ] = { TypeDef::TD_NATIVE, ScriptTypeNameDef { "uint8",  TYPEID_UINT8 } };
   m_types[TypeID::TYPEID_UINT16] = { TypeDef::TD_NATIVE, ScriptTypeNameDef { "uint16", TYPEID_UINT16 } };
   m_types[TypeID::TYPEID_UINT32] = { TypeDef::TD_NATIVE, ScriptTypeNameDef { "uint32", TYPEID_UINT32 } };
   m_types[TypeID::TYPEID_UINT64] = { TypeDef::TD_NATIVE, ScriptTypeNameDef { "uint64", TYPEID_UINT64 } };
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
   const string classId(lowerCase(classDef->name.name));
   if (m_typenames.contains(classId))
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
   m_types.push_back({ .category = TypeDef::TD_CLASS, .classDef = cd });

   // Register members
   for (int i = 0; i < static_cast<int>(classDef->nMembers); i++)
   {
      const ScriptClassMemberDef& memberDef = classDef->members[i];
      const string nameId(lowerCase(memberDef.name.name));
      const auto& memberMapEntry = cd->memberMap.find(nameId);
      if (memberMapEntry == cd->memberMap.end())
      {
         cd->memberMap[nameId] = static_cast<int>(cd->members.size());
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
   const string classId(lowerCase(name));
   const auto& existingType = m_typenames.find(classId);
   if (existingType != m_typenames.end())
   {
      // TODO Validate that both definitions are the same
      return;
   }
   // Aliasing is only supported for core type, and forward declaration are not supported
   const int aliasedId = ResolveClassId(aliasedTypeName);
   assert(aliasedId != TypeID::TYPEID_UNRESOLVED);
   const TypeDef typeDef = m_types[aliasedId];
   assert(typeDef.category == TypeDef::TD_NATIVE);
   m_types.push_back({ .category = TypeDef::TD_ALIAS, .aliasDef = { name, typeDef.nativeType } });
   m_typenames[classId] = typeDef.nativeType.id;
}

void DynamicTypeLibrary::RegisterScriptArray(ScriptArrayDef* arrayDef)
{
   const string classId(lowerCase(arrayDef->name.name));
   const auto& existingType = m_typenames.find(classId);
   if (existingType != m_typenames.end())
   {
      // TODO Validate that both definitions are the same
      return;
   }
   arrayDef->name.id = static_cast<unsigned int>(m_types.size());
   m_types.push_back({ .category = TypeDef::TD_ARRAY, .arrayDef = arrayDef });
   m_typenames[classId] = arrayDef->name.id;
}

void DynamicTypeLibrary::ResolveAllClasses()
{
   for (const auto& type : m_types)
   {
      switch (type.category)
      {
      case TypeDef::TD_ALIAS:
         assert(type.aliasDef.typeDef.id != TypeID::TYPEID_UNRESOLVED);
         break;

      case TypeDef::TD_NATIVE:
         assert(type.nativeType.name == nullptr || (type.nativeType.id != TypeID::TYPEID_UNRESOLVED));
         break;

      case TypeDef::TD_CLASS:
         assert(type.classDef->classDef->name.id != TypeID::TYPEID_UNRESOLVED);
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
   const string classId(lowerCase(name));
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
   assert(classDef->name.id != TypeID::TYPEID_UNRESOLVED);
   assert(classDef->name.id < m_types.size());
   TypeDef type = m_types[classDef->name.id];
   assert(type.category == TypeDef::TD_CLASS);
   ClassDef* cd = type.classDef;
   const string nameId(lowerCase(memberName));
   const auto& member = cd->memberMap.find(nameId);
   if (member != cd->memberMap.end())
      return member->second;
   return -1;
}

ScriptClassDef* DynamicTypeLibrary::GetClass(const ScriptTypeNameDef& name) const
{
   assert(name.id != TypeID::TYPEID_UNRESOLVED);
   assert(name.id < static_cast<int>(m_types.size()));
   const TypeDef& def = m_types[name.id];
   assert(def.category == TypeDef::TD_CLASS);
   assert(strcmp(def.classDef->classDef->name.name, name.name) == 0);
   return def.classDef->classDef;
}

bool DynamicTypeLibrary::COMToScriptVariant(const VARIANT* cv, const ScriptTypeNameDef& type, ScriptVariant& sv) const
{
   // Since we are copying from COM data structure to our own data layout, there is no difference between normal and 
   // byref arguments. The difference is after call where we need to update byref COM arguments from ScripVariants
   if (V_VT(cv) == (VT_BYREF | VT_VARIANT))
      return COMToScriptVariant(V_VARIANTREF(cv), type, sv);
   
   assert(type.id != TypeID::TYPEID_UNRESOLVED);
   const TypeDef& typeDef = m_types[type.id];
   switch (typeDef.category)
   {
   case TypeDef::TD_ALIAS:
      return COMToScriptVariant(cv, typeDef.aliasDef.typeDef, sv);
      break;

   case TypeDef::TD_NATIVE:
   {
      // Note that VariantChangeType dereference byref variants
      // TODO on failure, raise an error and prevent further processing
      #define CHANGE_TYPE(type) \
         if (VariantChangeType(&v, cv, 0, type) != S_OK) { \
            PLOGE << "Failed to convert from COM variant type " << V_VT(cv); \
            assert(false); \
            return false; \
         }
      VARIANT v;
      VariantInit(&v);
      switch (typeDef.nativeType.id)
      {
      case TypeID::TYPEID_UNRESOLVED:
      case TypeID::TYPEID_VOID:
         // TODO raise an error and prevent further processing
         PLOGE << "Class '" << type.name << "' is referenced while it is not registered in the type library. Did you forgot to call 'Register" << type.name << "SCD(...)' ?";
         assert(false);
         return false;
         break;
      case TypeID::TYPEID_BOOL:   CHANGE_TYPE(VT_BOOL); sv.vBool = (V_BOOL(&v) == VARIANT_TRUE); break;
      case TypeID::TYPEID_INT:    CHANGE_TYPE(VT_INT);  sv.vInt64 = V_INT(&v); break;
      case TypeID::TYPEID_UINT:   CHANGE_TYPE(VT_UINT); sv.vUInt = V_UINT(&v); break;
      case TypeID::TYPEID_FLOAT:  CHANGE_TYPE(VT_R4);   sv.vFloat = V_R4(&v); break;
      case TypeID::TYPEID_DOUBLE: CHANGE_TYPE(VT_R8);   sv.vDouble = V_R8(&v); break;
      case TypeID::TYPEID_INT8:   CHANGE_TYPE(VT_I1);   sv.vInt8 = V_I1(&v); break;
      case TypeID::TYPEID_INT16:  CHANGE_TYPE(VT_I2);   sv.vInt16 = V_I2(&v); break;
      case TypeID::TYPEID_INT32:  CHANGE_TYPE(VT_I4);   sv.vInt32 = V_I4(&v); break;
      case TypeID::TYPEID_INT64:  CHANGE_TYPE(VT_I8);   sv.vInt64 = V_I8(&v); break;
      case TypeID::TYPEID_UINT8:  CHANGE_TYPE(VT_UI1);  sv.vUInt8 = V_UI1(&v); break;
      case TypeID::TYPEID_UINT16: CHANGE_TYPE(VT_UI2);  sv.vUInt16 = V_UI2(&v); break;
      case TypeID::TYPEID_UINT32: CHANGE_TYPE(VT_UI4);  sv.vUInt32 = V_UI4(&v); break;
      case TypeID::TYPEID_UINT64: CHANGE_TYPE(VT_UI8);  sv.vUInt64 = V_UI8(&v); break;
      case TypeID::TYPEID_STRING: CHANGE_TYPE(VT_BSTR);
         {
            int sizeNeeded = WideCharToMultiByte(CP_ACP, 0, V_BSTR(&v), -1, nullptr, 0, nullptr, nullptr);
            if (sizeNeeded <= 0) 
            {
               // TODO raise an error and prevent further processing
               PLOGE << "Failed to convert COM's BSTR to char*";
               assert(false);
               return false;
            }
            char* charStr = new char[sizeNeeded];
            WideCharToMultiByte(CP_ACP, 0, V_BSTR(&v), -1, charStr, sizeNeeded, nullptr, nullptr);
            sv.vString = { [](ScriptString* s) { delete[] s->string; }, charStr };
         }
         break;
      }
      #undef CHANGE_TYPE
      break;
   }

   case TypeDef::TD_CLASS:
   {
      if ((V_VT(cv) == VT_DISPATCH) || (V_VT(cv) == (VT_BYREF | VT_DISPATCH)))
      {
         DynamicDispatch* dispatch = static_cast<DynamicDispatch*>((V_VT(cv) & VT_BYREF) ? *V_DISPATCHREF(cv) : V_DISPATCH(cv));
         sv.vObject = dispatch->m_nativeObject;
         if (sv.vObject != nullptr)
            PSC_ADD_REF(typeDef.classDef->classDef, sv.vObject);
      }
      else if ((V_VT(cv) == VT_UNKNOWN) || (V_VT(cv) == (VT_BYREF | VT_UNKNOWN)))
      {
         DynamicDispatch* dispatch = static_cast<DynamicDispatch*>((V_VT(cv) & VT_BYREF) ? *V_UNKNOWNREF(cv) : V_UNKNOWN(cv));
         sv.vObject = dispatch->m_nativeObject;
         if (sv.vObject != nullptr)
            PSC_ADD_REF(typeDef.classDef->classDef, sv.vObject);
      }
      else
      {
         // TODO raise an error and prevent further processing
         PLOGE << "Failed to convert to object of class " << typeDef.classDef->classDef->name.name << " from variant type " << V_VT(cv);
         //assert(false);
         return false;
      }
      break;
   }

   case TypeDef::TD_ARRAY:
   {
      assert((V_VT(cv) == (VT_ARRAY | VT_VARIANT)) || (V_VT(cv) == (VT_BYREF | VT_ARRAY | VT_VARIANT)));
      SAFEARRAY* psa = (V_VT(cv) & VT_BYREF) ? *V_ARRAYREF(cv) : V_ARRAY(cv);
      assert(typeDef.arrayDef->nDimensions == SafeArrayGetDim(psa));

      assert(typeDef.arrayDef->type.id != TypeID::TYPEID_UNRESOLVED);
      const TypeDef& arrayTypeDef = m_types[typeDef.arrayDef->type.id];
      assert(arrayTypeDef.category == TypeDef::TD_NATIVE); // Other types are not yet supported
      assert(typeDef.arrayDef->nDimensions == 1);
      LONG lBound, uBound;
      SafeArrayGetLBound(psa, 1, &lBound);
      SafeArrayGetUBound(psa, 1, &uBound);
      VARIANT* p;
      SafeArrayAccessData(psa, (void**)&p);
      ScriptArray* array = nullptr;
      ScriptVariant tmpSV;
      #define COPY_ARRAY(natType, svType) { \
            const size_t arraySize = sizeof(ScriptArray) + typeDef.arrayDef->nDimensions * sizeof(unsigned int) + (uBound - lBound + 1) * sizeof(natType); \
            array = static_cast<ScriptArray*>(malloc(arraySize)); \
            array->Release = [](ScriptArray* me) { free(me); }; \
            array->lengths[0] = static_cast<unsigned int>(uBound - lBound + 1); \
            natType* pData = reinterpret_cast<natType*>(&array->lengths[1]); \
            for (LONG i = lBound; i <= uBound; i++, pData++, p++) \
            { \
               if (!COMToScriptVariant(p, arrayTypeDef.nativeType, tmpSV)) \
               { \
                  array->Release(array); \
                  array = nullptr; \
                  break; \
               } \
               *pData = tmpSV.svType; \
            } \
         }
      switch (arrayTypeDef.nativeType.id)
      {
      case TypeID::TYPEID_INT:
      {
         const size_t arraySize = sizeof(ScriptArray) + typeDef.arrayDef->nDimensions * sizeof(unsigned int) + (uBound - lBound + 1) * sizeof(int);
         array = static_cast<ScriptArray*>(malloc(arraySize));
         array->Release = [](ScriptArray* me) { free(me); };
         array->lengths[0] = static_cast<unsigned int>(uBound - lBound + 1);
         int* pData = reinterpret_cast<int*>(&array->lengths[1]);
         for (LONG i = lBound; i <= uBound; i++, pData++, p++)
         {
            if (!COMToScriptVariant(p, arrayTypeDef.nativeType, tmpSV))
            {
               array->Release(array);
               array = nullptr;
               break;
            }
            *pData = tmpSV.vInt;
         }
      };
      break;
      case TypeID::TYPEID_UINT:   COPY_ARRAY(unsigned int, vUInt); break;
      case TypeID::TYPEID_INT8:   COPY_ARRAY(int8_t, vInt8); break;
      case TypeID::TYPEID_INT16:  COPY_ARRAY(int16_t, vInt16); break;
      case TypeID::TYPEID_UINT8:  COPY_ARRAY(uint8_t, vUInt8); break;
      case TypeID::TYPEID_UINT16: COPY_ARRAY(uint16_t, vUInt16); break;
      default: assert(false);
      }
      #undef COPY_ARRAY
      SafeArrayUnaccessData(psa);
      sv.vArray = array;
      return array != nullptr;
      break;
   }

   default: assert(false);
   }

   return true;
}

void DynamicTypeLibrary::ScriptToCOMVariant(const ScriptTypeNameDef& type, ScriptVariant& sv, VARIANT* cv) const
{
   assert(type.id != TypeID::TYPEID_UNRESOLVED);
   const TypeDef& typeDef = m_types[type.id];
   VariantInit(cv);
   switch (typeDef.category)
   {
   case TypeDef::TD_ALIAS: ScriptToCOMVariant(typeDef.aliasDef.typeDef, sv, cv); break;

   case TypeDef::TD_NATIVE:
      switch (typeDef.nativeType.id)
      {
      case TypeID::TYPEID_UNRESOLVED:
      case TypeID::TYPEID_VOID:
         PLOGE << "Class '" << type.name << "' is referenced while it is not registered in the type library. Did you forgot to call 'Register" << type.name << "SCD(...)' ?";
         V_VT(cv) = VT_EMPTY;
         break;
      case TypeID::TYPEID_BOOL:
         V_VT(cv) = VT_BOOL;
         V_BOOL(cv) = sv.vBool ? VARIANT_TRUE : VARIANT_FALSE;
         break;
      case TypeID::TYPEID_INT:
         V_VT(cv) = VT_INT;
         V_I4(cv) = sv.vInt;
         break;
      case TypeID::TYPEID_UINT:
         V_VT(cv) = VT_UINT;
         V_UI4(cv) = sv.vUInt;
         break;
      case TypeID::TYPEID_INT8:
         V_VT(cv) = VT_I1;
         V_I1(cv) = sv.vInt8;
         break;
      case TypeID::TYPEID_INT16:
         V_VT(cv) = VT_I2;
         V_I2(cv) = sv.vInt16;
         break;
      case TypeID::TYPEID_INT32:
         V_VT(cv) = VT_I4;
         V_I4(cv) = sv.vInt32;
         break;
      case TypeID::TYPEID_INT64:
         V_VT(cv) = VT_I8;
         V_I8(cv) = sv.vInt64;
         break;
      case TypeID::TYPEID_UINT8:
         V_VT(cv) = VT_UI1;
         V_UI1(cv) = sv.vUInt8;
         break;
      case TypeID::TYPEID_UINT16:
         V_VT(cv) = VT_UI2;
         V_UI2(cv) = sv.vUInt16;
         break;
      case TypeID::TYPEID_UINT32:
         V_VT(cv) = VT_UI4;
         V_UI4(cv) = sv.vUInt32;
         break;
      case TypeID::TYPEID_UINT64:
         V_VT(cv) = VT_UI8;
         V_UI8(cv) = sv.vUInt64;
         break;
      case TypeID::TYPEID_FLOAT:
         V_VT(cv) = VT_R4;
         V_R4(cv) = sv.vFloat;
         break;
      case TypeID::TYPEID_DOUBLE:
         V_VT(cv) = VT_R8;
         V_R8(cv) = sv.vDouble;
         break;
      case TypeID::TYPEID_STRING:
      {
         V_VT(cv) = VT_BSTR;
         const int len = MultiByteToWideChar(CP_ACP, 0, sv.vString.string, -1, nullptr, 0);
         V_BSTR(cv) = SysAllocStringLen(nullptr, len);
         MultiByteToWideChar(CP_ACP, 0, sv.vString.string, -1, V_BSTR(cv), len);
         break;
      }
      default: assert(false);
      }
      break;

   case TypeDef::TD_CLASS:
      if (sv.vObject == nullptr)
      {
         V_VT(cv) = VT_NULL;
      }
      else
      {
         V_VT(cv) = VT_DISPATCH;
         V_DISPATCH(cv) = new DynamicDispatch(this, typeDef.classDef->classDef, sv.vObject);
      }
      break;

   case TypeDef::TD_ARRAY:
   {
      if (typeDef.arrayDef->nDimensions == 1)
      {
         SAFEARRAY* psa = SafeArrayCreateVector(VT_VARIANT, typeDef.arrayDef->lowerBounds[0], sv.vArray->lengths[0]);
         VARIANT* pData;
         SafeArrayAccessData(psa, reinterpret_cast<void**>(&pData));
         assert(typeDef.arrayDef->type.id != TypeID::TYPEID_UNRESOLVED);
         const TypeDef& arrayTypeDef = m_types[typeDef.arrayDef->type.id];
         assert(arrayTypeDef.category == TypeDef::TD_NATIVE); // Other types are not yet supported
         #define COPY_ARRAY(natType, varType) { \
               const natType* const pSrc = reinterpret_cast<natType*>(&sv.vArray->lengths[1]); \
               for (unsigned int i = 0; i < sv.vArray->lengths[0]; i++) \
               { \
                  VariantInit(&pData[i]); \
                  V_VT(&pData[i]) = VT_##varType; \
                  V_##varType(&pData[i]) = pSrc[i]; \
               } \
            }
         switch (arrayTypeDef.nativeType.id)
         {
         case TypeID::TYPEID_INT:    COPY_ARRAY(int,          INT); break;
         case TypeID::TYPEID_UINT:   COPY_ARRAY(unsigned int, UINT); break;
         case TypeID::TYPEID_FLOAT:  COPY_ARRAY(float,        R4); break;
         case TypeID::TYPEID_DOUBLE: COPY_ARRAY(double,       R8); break;
         case TypeID::TYPEID_INT8:   COPY_ARRAY(int8_t,       I1); break;
         case TypeID::TYPEID_INT16:  COPY_ARRAY(int16_t,      I2); break;
         case TypeID::TYPEID_INT32:  COPY_ARRAY(int32_t,      I4); break;
         case TypeID::TYPEID_INT64:  COPY_ARRAY(int64_t,      I8); break;
         case TypeID::TYPEID_UINT8:  COPY_ARRAY(uint8_t,      UI1); break;
         case TypeID::TYPEID_UINT16: COPY_ARRAY(uint16_t,     UI2); break;
         case TypeID::TYPEID_UINT32: COPY_ARRAY(uint32_t,     UI4); break;
         case TypeID::TYPEID_UINT64: COPY_ARRAY(uint64_t,     UI8); break;
         default: assert(false); // not yet implemented
         }
         #undef COPY_ARRAY
         SafeArrayUnaccessData(psa);
         V_VT(cv) = VT_ARRAY | VT_VARIANT;
         V_ARRAY(cv) = psa;
      }
      else if (typeDef.arrayDef->nDimensions == 2)
      {
         SAFEARRAYBOUND bounds[2] = { { sv.vArray->lengths[0],typeDef.arrayDef->lowerBounds[0] }, { sv.vArray->lengths[1], typeDef.arrayDef->lowerBounds[1] } };
         SAFEARRAY* psa = SafeArrayCreate(VT_VARIANT, 2, bounds);
         assert(typeDef.arrayDef->type.id != TypeID::TYPEID_UNRESOLVED);
         const TypeDef& arrayTypeDef = m_types[typeDef.arrayDef->type.id];
         assert(arrayTypeDef.category == TypeDef::TD_NATIVE); // Other types are not yet supported
         VARIANT varValue;
         VariantInit(&varValue);
         LONG ix[2];
         #define COPY_ARRAY(natType, varType) { \
               V_VT(&varValue) = VT_##varType; \
               const natType* const pSrc = reinterpret_cast<natType*>(&sv.vArray->lengths[2]); \
               for (ix[0] = 0; ix[0] < static_cast<LONG>(sv.vArray->lengths[0]); ix[0]++) \
               { \
                  for (ix[1] = 0; ix[1] < static_cast<LONG>(sv.vArray->lengths[1]); ix[1]++) \
                  { \
                     V_##varType(&varValue) = pSrc[ix[0] * sv.vArray->lengths[1] + ix[1]]; \
                     SafeArrayPutElement(psa, ix, &varValue); \
                  } \
               } \
            }
         switch (arrayTypeDef.nativeType.id)
         {
         case TypeID::TYPEID_INT:    COPY_ARRAY(int,          INT); break;
         case TypeID::TYPEID_UINT:   COPY_ARRAY(unsigned int, UINT); break;
         case TypeID::TYPEID_FLOAT:  COPY_ARRAY(float,        R4); break;
         case TypeID::TYPEID_DOUBLE: COPY_ARRAY(double,       R8); break;
         case TypeID::TYPEID_INT8:   COPY_ARRAY(int8_t,       I1); break;
         case TypeID::TYPEID_INT16:  COPY_ARRAY(int16_t,      I2); break;
         case TypeID::TYPEID_INT32:  COPY_ARRAY(int32_t,      I4); break;
         case TypeID::TYPEID_INT64:  COPY_ARRAY(int64_t,      I8); break;
         case TypeID::TYPEID_UINT8:  COPY_ARRAY(uint8_t,      UI1); break;
         case TypeID::TYPEID_UINT16: COPY_ARRAY(uint16_t,     UI2); break;
         case TypeID::TYPEID_UINT32: COPY_ARRAY(uint32_t,     UI4); break;
         case TypeID::TYPEID_UINT64: COPY_ARRAY(uint64_t,     UI8); break;
         default: assert(false); // not yet implemented
         }
         #undef COPY_ARRAY
         V_VT(cv) = VT_ARRAY | VT_VARIANT;
         V_ARRAY(cv) = psa;
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

void DynamicTypeLibrary::ReleaseScriptVariant(const ScriptTypeNameDef& type, ScriptVariant& sv) const
{
   assert(type.id != TypeID::TYPEID_UNRESOLVED);
   const TypeDef& typeDef = m_types[type.id];
   switch (typeDef.category)
   {
   case TypeDef::TD_ALIAS: ReleaseScriptVariant(typeDef.aliasDef.typeDef, sv); break;

   case TypeDef::TD_NATIVE:
      if (typeDef.nativeType.id == TypeID::TYPEID_STRING)
         sv.vString.Release(&sv.vString);
      break;

   case TypeDef::TD_CLASS:
      if (sv.vObject != nullptr)
         PSC_RELEASE(typeDef.classDef->classDef, sv.vObject);
      break;

   case TypeDef::TD_ARRAY: sv.vArray->Release(sv.vArray); break;

   default: assert(false);
   }
}

string DynamicTypeLibrary::ScriptVariantToString(const ScriptTypeNameDef& type, const ScriptVariant& sv) const
{
   assert(type.id != TypeID::TYPEID_UNRESOLVED);
   const TypeDef& typeDef = m_types[type.id];
   switch (typeDef.category)
   {
   case TypeDef::TD_ALIAS: return ScriptVariantToString(typeDef.aliasDef.typeDef, sv);

   case TypeDef::TD_NATIVE:
      switch (typeDef.nativeType.id)
      {
      case TypeID::TYPEID_UNRESOLVED: return "Invalid"s;
      case TypeID::TYPEID_VOID: return "void"s;
      case TypeID::TYPEID_BOOL: return sv.vBool ? "true"s : "false"s;
      case TypeID::TYPEID_INT: return std::to_string(sv.vInt);
      case TypeID::TYPEID_UINT: return std::to_string(sv.vUInt);
      case TypeID::TYPEID_FLOAT: return std::to_string(sv.vFloat);
      case TypeID::TYPEID_DOUBLE: return std::to_string(sv.vDouble);
      case TypeID::TYPEID_INT8: return std::to_string(sv.vInt8);
      case TypeID::TYPEID_INT16: return std::to_string(sv.vInt16);
      case TypeID::TYPEID_INT32: return std::to_string(sv.vInt32);
      case TypeID::TYPEID_INT64: return std::to_string(sv.vInt64);
      case TypeID::TYPEID_UINT8: return std::to_string(sv.vUInt8);
      case TypeID::TYPEID_UINT16: return std::to_string(sv.vUInt16);
      case TypeID::TYPEID_UINT32: return std::to_string(sv.vUInt32);
      case TypeID::TYPEID_UINT64: return std::to_string(sv.vUInt64);
      case TypeID::TYPEID_STRING: return "\""s.append(sv.vString.string).append(1, '"');
      default: assert(false);
      }
      break;

   case TypeDef::TD_CLASS: return typeDef.classDef->classDef->name.name;

   case TypeDef::TD_ARRAY: return typeDef.arrayDef->name.name;

   default:
      break;
   }

   return "<< bug >>"s;
}

HRESULT DynamicTypeLibrary::Invoke(const ScriptClassDef * classDef, void* nativeObject, DISPID dispIdMember, REFIID, LCID, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO*, UINT* puArgErr) const
{
   assert(classDef->name.id != TypeID::TYPEID_UNRESOLVED);
   assert(classDef->name.id < m_types.size());
   if (pDispParams->cArgs > PSC_CALL_MAX_ARG_COUNT)
   {
      return DISP_E_BADPARAMCOUNT;
   }
   TypeDef type = m_types[classDef->name.id];
   assert(type.category == TypeDef::TD_CLASS);
   ClassDef * cd = type.classDef;
   if ((dispIdMember < 0) || (dispIdMember > (DISPID)cd->members.size()))
   {
      return DISP_E_MEMBERNOTFOUND;
   }

   // Default member
   if (dispIdMember == 0)
   {
      if (wFlags & DISPATCH_PROPERTYGET)
      {
         assert(V_VT(pVarResult) == VT_EMPTY);
         V_VT(pVarResult) = VT_DISPATCH;
         V_DISPATCH(pVarResult) = new DynamicDispatch(this, classDef, nativeObject);
         return S_OK;
      }
      return DISP_E_MEMBERNOTFOUND;
   }

   // Search for the right overload (needed for property which are a member with overloads for getters and setters)
   // FIXME match overload on type and arguments
   const std::vector<int>& members = cd->members[dispIdMember - 1];
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
         if ((memberDef.type.id != TypeID::TYPEID_VOID) && (memberDef.nArgs == pDispParams->cArgs))
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
         if ((memberDef.type.id == TypeID::TYPEID_VOID) && (memberDef.nArgs == pDispParams->cArgs))
         {
            memberIndex = i;
            break;
         }
      }
   }
   else if (wFlags & DISPATCH_PROPERTYPUTREF)
   {
      // TODO implement (when is this used ? Set obj1.prop = obj2 ?)
      assert(false);
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
   if ((memberDef.type.id != TypeID::TYPEID_VOID) && (pVarResult == nullptr))
      return E_POINTER;

   // Convert all incoming COM arguments to ScriptVariants
   ScriptVariant args[PSC_CALL_MAX_ARG_COUNT];
   for (unsigned int i = 0; i < pDispParams->cArgs; i++)
   {
      if (!COMToScriptVariant(&pDispParams->rgvarg[pDispParams->cArgs - 1 - i], memberDef.callArgType[i], args[i]))
      {
         PLOGE << "Failed to convert a parameter in call to " << classDef->name.name << '.' << memberDef.name.name;
         *puArgErr = pDispParams->cArgs - 1 - i;
         return DISP_E_TYPEMISMATCH;
      }
   }

   #if LOG_INVOKES
      std::stringstream ss;
      ss << "Invoke Call: " << classDef->name.name << '.' << memberDef.name.name << '(';
      for (unsigned int i = 0; i < pDispParams->cArgs; i++)
         ss << ((i != 0) ? ", " : "") << ScriptVariantToString(memberDef.callArgType[i], args[i]);
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
      return DISP_E_EXCEPTION;
   }

   // Update byref args and dispose temporary allocations
   for (unsigned int i = 0; i < pDispParams->cArgs; i++)
   {
      VARIANT* cv = &pDispParams->rgvarg[pDispParams->cArgs - 1 - i];
      if (V_VT(cv) & VT_BYREF)
      {
         if (V_VT(cv) == (VT_BYREF | VT_VARIANT))
            cv = V_VARIANTREF(cv);
         switch (V_VT(cv) & ~VT_BYREF)
         {
         case VT_BOOL:
         case VT_INT:
         case VT_UINT:
         case VT_R4:
         case VT_R8:
         case VT_I1:
         case VT_I2:
         case VT_I4:
         case VT_I8:
         case VT_UI1:
         case VT_UI2:
         case VT_UI4:
         case VT_UI8:
         case VT_BSTR:
         {
            // We update the referenced value but, for the time being, this is not really needed as we do not allow byref arguments in script
            VARIANT varValue;
            ScriptToCOMVariant(memberDef.callArgType[i], args[i], &varValue);
            if (VariantChangeType(cv, &varValue, 0, V_VT(cv) & ~VT_BYREF) != S_OK)
            {
               PLOGE << "Failed to update byref COM argument after call";
               assert(false);
            }
            break;
         }
         case VT_DISPATCH:
            // We should update the referenced dispatch if needed, but for the time being, we do not allow byref arguments in script, so this may not happen
            assert(static_cast<DynamicDispatch*>(V_DISPATCH(cv))->m_nativeObject == args[i].vObject);
            break;
         case VT_ARRAY | VT_VARIANT:
            // We should update the referenced array if needed, but for the time being, we do not allow byref arguments in script, so this may not happen
            break;
         case VT_EMPTY:
            // We should handle empty as well here
            break;
         default:
            PLOGE << "Failed to update byref COM argument after call (not implemented)";
            assert(false);
            break;
         }
      }
      ReleaseScriptVariant(memberDef.callArgType[i], args[i]);
   }
   
   // Convert then dispose the return value if any
   if (memberDef.type.id != TypeID::TYPEID_VOID)
   {
      assert(V_VT(pVarResult) == VT_EMPTY);
      ScriptToCOMVariant(memberDef.type, retValue, pVarResult);
      ReleaseScriptVariant(memberDef.type, retValue);
   }

   return S_OK;
}
