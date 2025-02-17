// license:GPLv3+

#pragma once

#include "MsgPlugin.h"

#ifdef __cplusplus
 #include <cstring>
 #include <cstdint>
#else
 #include <string.h>
 #include <stdint.h>
#endif


///////////////////////////////////////////////////////////////////////////////
// Scriptable plugins
//
// WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
// This interface is part of a work in progress and will evolve likely a lot
// before being considered stable. Do not use it, or if you do, use it knowing
// that you're plugin will be broken by the upcoming updates.
// WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
//
// This file defines an API that allows a plugin to contribute scriptable objects
// to a scripting host. As much as possible, the API is independent from the 
// scripting language.
//
// For the sake of simplicity:
// - the API is entirely statically typed. The type library is declared (at runtime,
//   following the plugin lifecycle) with static types that may not change afterward.
// - the API does not provide any threading synchronization. This must be entirely
//   implemented by the host/plugin.
// - the point of the library is to allow plugins to expose data and functions
//   organized as objects. The following types of data are supported:
//   . primitive type (int, float, string,...) and array which are owned by the caller (plugin host)
//   . object reference which lifecycle is managed through reference counting
//   . type alias which allow basic support for enum types
//
// FIXME: unregistering, and type library memory freeing is missing
//

#define SCRIPTPI_NAMESPACE                 "Scriptable" // Namespace used for all scriptable API message definition
#define SCRIPTPI_MSG_GET_API               "GetAPI"     // Get the plugin API

typedef char           sc_bool;
typedef char           sc_int8;
typedef short          sc_int16;
typedef int            sc_int32;
typedef long           sc_int64;
typedef unsigned char  sc_uint8;
typedef unsigned short sc_uint16;
typedef unsigned int   sc_uint32;
typedef unsigned long  sc_uint64;
typedef float          sc_float;
typedef double         sc_double;

typedef struct ScriptTypeNameDef
{
   const char* name;
   unsigned int id;
} ScriptTypeNameDef;

typedef struct ScriptArrayDef
{
   ScriptTypeNameDef name;
   ScriptTypeNameDef type;
   unsigned int nDimensions;
   int lowerBounds[10];
} ScriptArrayDef;

#pragma warning(disable : 4200) // 0 length array is a non standard extension used intentionally, so disable corresponding warning (this breaks default copy constructor & equal)
typedef struct ScriptArray
{
   void (MSGPIAPI *Release)(ScriptArray*);
   unsigned int lengths[];
   //uint8_t pData[];
} ScriptArray;

// The script API being statically typed, ScriptVariant does not hold any type information
typedef union ScriptVariant
{
   sc_bool          vBool;
   sc_int8          vByte;
   sc_int16         vShort;
   sc_int32         vInt;
   sc_int64         vLong;
   sc_uint8         vUByte;
   sc_uint16        vUShort;
   sc_uint32        vUInt;
   sc_uint64        vULong;
   sc_float         vFloat;
   sc_double        vDouble;
   const char*      vString;
   void*            vObject; // When an object is shared by a plugin, it must implement reference counting by exposing AddRef/Release methods
   ScriptArray*     vArray;
} ScriptVariant;

#define PSC_CALL_MAX_ARG_COUNT 16
typedef struct ScriptClassMemberDef
{
   ScriptTypeNameDef name;
   ScriptTypeNameDef type; // return type
   unsigned int nArgs;
   ScriptTypeNameDef callArgType[PSC_CALL_MAX_ARG_COUNT];
   void (MSGPIAPI *Call)(void* me, int memberIndex, ScriptVariant* pArgs, ScriptVariant* pRet);
} ScriptClassMemberDef;

#pragma warning(disable : 4200) // 0 length array is a non standard extension used intentionally, so disable corresponding warning (this breaks default copy constructor & equal)
typedef struct ScriptClassDef
{
   ScriptTypeNameDef name;
   void* (MSGPIAPI *CreateObject)();
   unsigned int nMembers;
   ScriptClassMemberDef members[];
} ScriptClassDef;

#define PSC_ERR_FAIL                 0 // Generic failure
#define PSC_ERR_INVALID_ARG          1 // Call with an invalid argument
#define PSC_ERR_NULL_POINTER         2 // Null pointer error
#define PSC_ERR_NOT_IMPLEMENTED      3 // Call of an unimplemented function

typedef struct ScriptablePluginAPI
{
   // Define types, then submit them
   void (MSGPIAPI *RegisterScriptClass)(ScriptClassDef* classDef);
   void (MSGPIAPI *RegisterScriptTypeAlias)(const char* name, const char* aliasedType);
   void (MSGPIAPI *RegisterScriptArrayType)(ScriptArrayDef* type);
   void (MSGPIAPI *SubmitTypeLibrary)();

   // If called while processing a call, the scripting engine will handle the call failure
   void (MSGPIAPI *OnError)(unsigned int type, const char* message);

   // Allows to request a COM object to be overriden by our own implementation
   void (MSGPIAPI *SetCOMObjectOverride)(const char* className, const ScriptClassDef* classDef);
} ScriptablePluginAPI;




///////////////////////////////////////////////////////////////////////////////
// 
// The following helper macros are designed to easily bridge C++ objects,
// they use assert and std::vector.
//

#define PSC_USE_ERROR() extern void PSCOnError(unsigned int type, const char* format, ...)
#define PSC_FAIL(...) PSCOnError(PSC_ERR_FAIL, __VA_ARGS__)
#define PSC_INVALID_ARG(...) PSCOnError(PSC_ERR_INVALID_ARG, __VA_ARGS__)
#define PSC_NULL_PTR(...) PSCOnError(PSC_ERR_NULL_POINTER, __VA_ARGS__)
#define PSC_NOT_IMPLEMENTED(...) PSCOnError(PSC_ERR_NOT_IMPLEMENTED, __VA_ARGS__)

#define PSC_ERROR_IMPLEMENT(scriptablePluginApi) \
   void PSCOnError(unsigned int type, const char* format, ...) { \
      if (scriptablePluginApi != nullptr) { \
         va_list args; \
         va_start(args, format); \
         int size = vsnprintf(NULL, 0, format, args); \
         if (size > 0) { \
            char* buffer = static_cast<char*>(malloc(size + 1)); \
            vsnprintf(buffer, size, format, args); \
            scriptablePluginApi->OnError(type, buffer); \
            free(buffer); \
         } \
         va_end(args); \
      } \
   }

#define PSC_VAR_int(variant) (variant).vInt
#define PSC_VAR_uint(variant) (variant).vUInt
#define PSC_VAR_long(variant) (variant).vLong
#define PSC_VAR_bool(variant) (variant).vBool
#define PSC_VAR_float(variant) (variant).vFloat
#define PSC_VAR_double(variant) (variant).vDouble
#define PSC_VAR_string(variant) std::string((variant).vString)
#define PSC_VAR_enum(type, variant) static_cast<type>((variant).vInt)
#define PSC_VAR_object(type, variant) static_cast<type *>((variant).vObject)

#define PSC_VAR_SET_void(variant, value) value;
#define PSC_VAR_SET_int(variant, value) PSC_VAR_int(variant) = value;
#define PSC_VAR_SET_uint(variant, value) PSC_VAR_uint(variant) = value;
#define PSC_VAR_SET_long(variant, value) PSC_VAR_long(variant) = value;
#define PSC_VAR_SET_bool(variant, value) PSC_VAR_bool(variant) = value;
#define PSC_VAR_SET_float(variant, value) PSC_VAR_float(variant) = value;
#define PSC_VAR_SET_double(variant, value) PSC_VAR_double(variant) = value;
#define PSC_VAR_SET_enum(type, variant, value) (variant).vInt = static_cast<int>(value);
#define PSC_VAR_SET_string(variant, value) (variant).vString = (value).c_str(); // FIXME This is awfully not clean, opening doors to access to temp memory allocated for string
#define PSC_VAR_SET_object(vtype, variant, value) { (variant).vObject = static_cast<void*>(value); }

#define PSC_ARRAY1(name, type, lowerBound) \
   static ScriptArrayDef name##_SCD = { { #name }, { #type }, 1, { lowerBound } }; \
   template<typename F> void Register##name##SCD(F&& regFunc) { regFunc(&name##_SCD); } \
   template<typename F> void Unregister##name##SCD(F&& unregFunc) { unregFunc(&name##_SCD); }

#define PSC_ARRAY2(name, type, lowerBound1, lowerBound2) \
   static ScriptArrayDef name##_SCD = { { #name }, { #type }, 2, { lowerBound1, lowerBound2 } }; \
   template<typename F> void Register##name##SCD(F&& regFunc) { regFunc(&name##_SCD); } \
   template<typename F> void Unregister##name##SCD(F&& unregFunc) { unregFunc(&name##_SCD); }

// Arrays are owned by caller, therefore a copy from plugin memory is needed for every set operation
#define PSC_VAR_SET_array1(type, variant, value) { \
      const unsigned int nDimensions = type##_SCD.nDimensions; \
      const auto& vec = (value); \
      const size_t dataSize = vec.empty() ? 0 : (vec.size() * sizeof(vec[0])); \
      ScriptArray* array = static_cast<ScriptArray*>(malloc(sizeof(ScriptArray) + nDimensions * sizeof(int) + dataSize)); \
      array->Release = [](ScriptArray* me) { free(me); }; \
      array->lengths[0] = static_cast<unsigned int>(vec.size()); \
      char* pData = reinterpret_cast<char*>(&array->lengths[1]); \
      if (dataSize > 0) memcpy(pData, vec.data(), dataSize); \
      (variant).vArray = array; \
   }

#define PSC_VAR_SET_array2(type, variant, value) { \
      const unsigned int nDimensions = type##_SCD.nDimensions; \
      const auto& vec = (value); \
      const unsigned int subDataSize = (vec.empty() || vec[0].empty()) ? 0 : (sizeof(vec[0][0]) * vec[0].size()); \
      const size_t dataSize = (vec.empty() || vec[0].empty()) ? 0 : (vec.size() * subDataSize); \
      ScriptArray* array = static_cast<ScriptArray*>(malloc(sizeof(ScriptArray) + nDimensions * sizeof(int) + dataSize)); \
      array->Release = [](ScriptArray* me) { free(me); }; \
      array->lengths[0] = static_cast<unsigned int>(vec.size()); \
      array->lengths[1] = array->lengths[0] > 0 ? vec[0].size() : 0; \
      char* pData = reinterpret_cast<char*>(&array->lengths[2]); \
      if (dataSize > 0) for (size_t i = 0; i < vec.size(); i++, pData += subDataSize) \
         memcpy(pData, vec[i].data(), subDataSize); \
      (variant).vArray = array; \
   }

#define PSC_VAR_array1(type, variant) \
   std::vector<type>( \
      reinterpret_cast<type *>(&(variant).vArray->lengths[1]), \
      reinterpret_cast<type *>(&(variant).vArray->lengths[1]) + (variant).vArray->lengths[0] )

#define PSC_CLASS_ALIAS(name, alias) \
   template <typename F> void Register##name##SCD(F&& regFunc) { regFunc(#name, #alias); } \
   template <typename F> void Unregister##name##SCD(F&& regFunc) { regFunc(#name); }

#define PSC_CLASS_START(name) \
   static ScriptClassDef* name##_SCD = nullptr; \
   template<typename F> void Register##name##SCD(F&& regFunc) { \
      assert(name##_SCD == nullptr); \
      std::vector<ScriptClassMemberDef> members; \
      members.push_back( { { "AddRef" }, { "ulong" }, 0, { }, \
         [](void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) { unsigned long rc = static_cast<name *>(me)->AddRef(); if (pRet != nullptr) pRet->vULong = rc; } } ); \
      members.push_back( { { "Release" }, { "ulong" }, 0, { }, \
         [](void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) { unsigned long rc = static_cast<name *>(me)->Release(); if (pRet != nullptr) pRet->vULong = rc; } } );

#define PSC_IMPLEMENT_REFCOUNT() \
   protected: \
      int m_refCount = 1; \
   public: \
      unsigned long AddRef() { m_refCount++; return m_refCount; } \
      unsigned long Release() { m_refCount--; unsigned long rc = m_refCount; if (rc == 0) delete this; return rc; }

#define PSC_ADD_REF(classDef, object) classDef->members[0].Call(object, 0, nullptr, nullptr);
#define PSC_RELEASE(classDef, object) classDef->members[1].Call(object, 1, nullptr, nullptr);

#define PSC_CLASS_END(className) \
      for (std::vector<ScriptClassMemberDef>::iterator i = members.begin(); i != members.end(); i++) { \
         for (std::vector<ScriptClassMemberDef>::iterator j = i + 1; j != members.end(); j++) { \
            bool isOverload = true; \
            isOverload &= strcmp(i->name.name, j->name.name) == 0; \
            isOverload &= strcmp(i->type.name, j->type.name) == 0; \
            for (int k = 0; isOverload && (k < PSC_CALL_MAX_ARG_COUNT); k++) { \
               isOverload &= (i->callArgType[k].name == nullptr) != (j->callArgType[k].name == nullptr); \
               if ((i->callArgType[k].name != nullptr) && (j->callArgType[k].name != nullptr)) \
                  isOverload &= strcmp(i->callArgType[k].name, j->callArgType[k].name) == 0; \
            } \
            if (isOverload) { \
               *i = *j; \
               members.erase(j); \
               break;  \
            } \
         } \
      } \
      className##_SCD = static_cast<ScriptClassDef*>(malloc(sizeof(ScriptClassDef) + members.size() * sizeof(ScriptClassMemberDef))); \
      className##_SCD->name.name = #className; \
      className##_SCD->name.id = 0; \
      className##_SCD->CreateObject = nullptr; \
      className##_SCD->nMembers = static_cast<unsigned int>(members.size()); \
      for (size_t i = 0; i < members.size(); i++) \
         className##_SCD->members[i] = members[i]; \
      regFunc(className##_SCD); \
   } \
   template<typename F> void Unregister##className##SCD(F&& regFunc) { \
      assert(className##_SCD != nullptr); \
      regFunc(className##_SCD); \
      free(className##_SCD); \
      className##_SCD = nullptr; \
   }      

#define PSC_INHERIT_CLASS(className, parentName) \
   for (unsigned int i = 0; i < parentName##_SCD->nMembers; i++) { members.push_back(parentName##_SCD->members[i]); }

#define PSC_PROP_R(className, type, name) \
   members.push_back( { { #name }, { #type }, 0, { }, \
      [](void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) { PSC_VAR_SET_##type(*pRet, static_cast<className*>(me)->Get##name()) } } );

#define PSC_PROP_R_ARRAY1(className, type, name, arg1) \
   members.push_back( { { #name }, { #type }, 1, { { #arg1 } }, \
      [](void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) { PSC_VAR_SET_##type(*pRet, static_cast<className*>(me)->Get##name(PSC_VAR_##arg1(pArgs[0]))) } });

#define PSC_PROP_W(className, type, name) \
   members.push_back({ { #name }, { "void" }, 1, { #type }, \
      [](void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) { static_cast<className*>(me)->Set##name(PSC_VAR_##type(pArgs[0])); } });

#define PSC_PROP_W_ARRAY1(className, type, name, arg1) \
   members.push_back({ { #name }, { "void" }, 2, { { #arg1 }, { #type } }, \
      [](void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) { static_cast<className*>(me)->Set##name(PSC_VAR_##arg1(pArgs[0]), PSC_VAR_##type(pArgs[1])); } } );

#define PSC_PROP_RW(className, type, name) \
   PSC_PROP_R(className, type, name) \
   PSC_PROP_W(className, type, name)

#define PSC_PROP_RW_ARRAY1(className, type, name, arg1) \
   PSC_PROP_R_ARRAY1(className, type, name, arg1) \
   PSC_PROP_W_ARRAY1(className, type, name, arg1)

#define PSC_FUNCTION0(className, type, name) \
   members.push_back( { { #name }, { #type }, 0, { }, \
      [](void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) \
         { PSC_VAR_SET_##type(*pRet, static_cast<className*>(me)->name()) } } );

#define PSC_FUNCTION1(className, type, name, arg1) \
   members.push_back( { { #name }, { #type }, 1, { { #arg1 } }, \
      [](void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) \
         { PSC_VAR_SET_##type(*pRet, static_cast<className*>(me)->name(PSC_VAR_##arg1(pArgs[0]))) } } );

#define PSC_FUNCTION2(className, type, name, arg1, arg2) \
   members.push_back( { { #name }, { #type }, 2, { { #arg1 }, { #arg2 } }, \
      [](void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) \
         { PSC_VAR_SET_##type(*pRet, static_cast<className *>(me)->name( PSC_VAR_##arg1(pArgs[0]), PSC_VAR_##arg2(pArgs[1]) )) } } );

#define PSC_FUNCTION3(className, type, name, arg1, arg2, arg3) \
   members.push_back( { { #name }, { #type }, 3, { { #arg1 }, { #arg2 }, { #arg3 } }, \
      [](void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) \
         { PSC_VAR_SET_##type(*pRet, static_cast<className *>(me)->name( PSC_VAR_##arg1(pArgs[0]), PSC_VAR_##arg2(pArgs[1]), PSC_VAR_##arg3(pArgs[2]) )) } } );

#define PSC_FUNCTION4(className, type, name, arg1, arg2, arg3, arg4) \
   members.push_back( { { #name }, { #type }, 4, { { #arg1 }, { #arg2 }, { #arg3 }, { #arg4 } }, \
      [](void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) \
         { PSC_VAR_SET_##type(*pRet, static_cast<className *>(me)->name( PSC_VAR_##arg1(pArgs[0]), PSC_VAR_##arg2(pArgs[1]), PSC_VAR_##arg3(pArgs[2]), PSC_VAR_##arg4(pArgs[3]) )) } } );

#define PSC_FUNCTION5(className, type, name, arg1, arg2, arg3, arg4, arg5) \
   members.push_back( { { #name }, { #type }, 5, { { #arg1 }, { #arg2 }, { #arg3 }, { #arg4 }, { #arg5 } }, \
      [](void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) \
         { PSC_VAR_SET_##type(*pRet, static_cast<className *>(me)->name( PSC_VAR_##arg1(pArgs[0]), PSC_VAR_##arg2(pArgs[1]), PSC_VAR_##arg3(pArgs[2]), PSC_VAR_##arg4(pArgs[3]), PSC_VAR_##arg4(pArgs[4]) )) } } );

#define PSC_FUNCTION6(className, type, name, arg1, arg2, arg3, arg4, arg5, arg6) \
   members.push_back( { { #name }, { #type }, 6, { { #arg1 }, { #arg2 }, { #arg3 }, { #arg4 }, { #arg5 }, { #arg6 } }, \
      [](void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) \
         { PSC_VAR_SET_##type(*pRet, static_cast<className *>(me)->name( PSC_VAR_##arg1(pArgs[0]), PSC_VAR_##arg2(pArgs[1]), PSC_VAR_##arg3(pArgs[2]), PSC_VAR_##arg4(pArgs[3]), PSC_VAR_##arg4(pArgs[4]), PSC_VAR_##arg4(pArgs[5]) )) } } );

#define PSC_FUNCTION7(className, type, name, arg1, arg2, arg3, arg4, arg5, arg6, arg7) \
   members.push_back( { { #name }, { #type }, 7, { { #arg1 }, { #arg2 }, { #arg3 }, { #arg4 }, { #arg5 }, { #arg6 }, { #arg7 } }, \
      [](void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) \
         { PSC_VAR_SET_##type(*pRet, static_cast<className *>(me)->name( PSC_VAR_##arg1(pArgs[0]), PSC_VAR_##arg2(pArgs[1]), PSC_VAR_##arg3(pArgs[2]), PSC_VAR_##arg4(pArgs[3]), PSC_VAR_##arg5(pArgs[4]), PSC_VAR_##arg6(pArgs[5]), \
                                                                         PSC_VAR_##arg7(pArgs[6]) )) } } );

#define PSC_FUNCTION8(className, type, name, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) \
   members.push_back( { { #name }, { #type }, 8, { { #arg1 }, { #arg2 }, { #arg3 }, { #arg4 }, { #arg5 }, { #arg6 }, { #arg7 }, { #arg8 } }, \
      [](void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) \
         { PSC_VAR_SET_##type(*pRet, static_cast<className *>(me)->name( PSC_VAR_##arg1(pArgs[0]), PSC_VAR_##arg2(pArgs[1]), PSC_VAR_##arg3(pArgs[2]), PSC_VAR_##arg4(pArgs[3]), PSC_VAR_##arg5(pArgs[4]), PSC_VAR_##arg6(pArgs[5]), \
                                                                         PSC_VAR_##arg7(pArgs[6]), PSC_VAR_##arg8(pArgs[7]) )) } } );

#define PSC_FUNCTION9(className, type, name, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9) \
   members.push_back( { { #name }, { #type }, 9, { { #arg1 }, { #arg2 }, { #arg3 }, { #arg4 }, { #arg5 }, { #arg6 }, { #arg7 }, { #arg8 }, { #arg9 } }, \
      [](void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) \
         { PSC_VAR_SET_##type(*pRet, static_cast<className *>(me)->name( PSC_VAR_##arg1(pArgs[0]), PSC_VAR_##arg2(pArgs[1]), PSC_VAR_##arg3(pArgs[2]), PSC_VAR_##arg4(pArgs[3]), PSC_VAR_##arg5(pArgs[4]), PSC_VAR_##arg6(pArgs[5]), \
                                                                         PSC_VAR_##arg7(pArgs[6]), PSC_VAR_##arg8(pArgs[7]), PSC_VAR_##arg9(pArgs[8]) )) } } );

#define PSC_FUNCTION10(className, type, name, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10) \
   members.push_back( { { #name }, { #type }, 10, { { #arg1 }, { #arg2 }, { #arg3 }, { #arg4 }, { #arg5 }, { #arg6 }, { #arg7 }, { #arg8 }, { #arg9 }, { #arg10 } }, \
      [](void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) \
         { PSC_VAR_SET_##type(*pRet, static_cast<className *>(me)->name( PSC_VAR_##arg1(pArgs[0]), PSC_VAR_##arg2(pArgs[1]), PSC_VAR_##arg3(pArgs[2]), PSC_VAR_##arg4(pArgs[3]), PSC_VAR_##arg5(pArgs[4]), PSC_VAR_##arg6(pArgs[5]), \
                                                                         PSC_VAR_##arg7(pArgs[6]), PSC_VAR_##arg8(pArgs[7]), PSC_VAR_##arg9(pArgs[8]), PSC_VAR_##arg10(pArgs[9]) )) } } );

#define PSC_FUNCTION11(className, type, name, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11) \
   members.push_back( { { #name }, { #type }, 11, { { #arg1 }, { #arg2 }, { #arg3 }, { #arg4 }, { #arg5 }, { #arg6 }, { #arg7 }, { #arg8 }, { #arg9 }, { #arg10 }, { #arg11 } }, \
      [](void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) \
         { PSC_VAR_SET_##type(*pRet, static_cast<className *>(me)->name( PSC_VAR_##arg1(pArgs[0]), PSC_VAR_##arg2(pArgs[1]), PSC_VAR_##arg3(pArgs[2]), PSC_VAR_##arg4(pArgs[3]), PSC_VAR_##arg5(pArgs[4]), PSC_VAR_##arg6(pArgs[5]), \
                                                                         PSC_VAR_##arg7(pArgs[6]), PSC_VAR_##arg8(pArgs[7]), PSC_VAR_##arg9(pArgs[8]), PSC_VAR_##arg10(pArgs[9]), PSC_VAR_##arg11(pArgs[10]) )) } } );

#define PSC_FUNCTION12(className, type, name, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12) \
   members.push_back( { { #name }, { #type }, 12, { { #arg1 }, { #arg2 }, { #arg3 }, { #arg4 }, { #arg5 }, { #arg6 }, { #arg7 }, { #arg8 }, { #arg9 }, { #arg10 }, { #arg10 }, { #arg12 } }, \
      [](void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) \
         { PSC_VAR_SET_##type(*pRet, static_cast<className *>(me)->name( PSC_VAR_##arg1(pArgs[0]), PSC_VAR_##arg2(pArgs[1]), PSC_VAR_##arg3(pArgs[2]), PSC_VAR_##arg4(pArgs[3]), PSC_VAR_##arg5(pArgs[4]), PSC_VAR_##arg6(pArgs[5]), \
                                                                         PSC_VAR_##arg7(pArgs[6]), PSC_VAR_##arg8(pArgs[7]), PSC_VAR_##arg9(pArgs[8]), PSC_VAR_##arg10(pArgs[9]), PSC_VAR_##arg11(pArgs[10]), PSC_VAR_##arg12(pArgs[11]) )) } } );
