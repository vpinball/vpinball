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
// - The data ownership model is the following:
//   - shared data must be part of a reference counted object:
//     .When a method returns an object, it must add a reference before returning it.
//     .When a method receive an object argument and wants to keep a reference to it,
//      it must call AddRef.
//   - unshared data are copied. Variable length datas (arrays and strings) must be
//     disposed by the caller of the method using the provided 'Release' lambda.
//
// FIXME: Not yet implemented features:
// - type library memory freeing
// - byref arguments
//

#define SCRIPTPI_NAMESPACE                 "Scriptable" // Namespace used for all scriptable API message definition
#define SCRIPTPI_MSG_GET_API               "GetAPI"     // Get the plugin API

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
   void (MSGPIAPI *Release)(ScriptArray* me);
   unsigned int lengths[];
   //uint8_t pData[];
} ScriptArray;

typedef struct ScriptString
{
   void (MSGPIAPI *Release)(ScriptString* me);
   char* string;
} ScriptString;

// The script API being statically typed, ScriptVariant does not hold any type information
typedef union ScriptVariant
{
   char             vBool;
   int              vInt;
   unsigned int     vUInt;
   float            vFloat;
   double           vDouble;
   int8_t           vInt8;
   int16_t          vInt16;
   int32_t          vInt32;
   int64_t          vInt64;
   uint8_t          vUInt8;
   uint16_t         vUInt16;
   uint32_t         vUInt32;
   uint64_t         vUInt64;
   ScriptString     vString; // Variable length data must follow the copy/release policy explained in the header of this file
   ScriptArray*     vArray;  // Variable length data must follow the copy/release policy explained in the header of this file
   void*            vObject; // When an object is shared by a plugin, it must implement reference counting by exposing AddRef/Release as its first methods
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
   void (MSGPIAPI *SubmitTypeLibrary)(const unsigned int endpointId);

   // Unregister previously registered types
   void (MSGPIAPI *UnregisterScriptClass)(ScriptClassDef* classDef);
   void (MSGPIAPI *UnregisterScriptTypeAlias)(const char* name);
   void (MSGPIAPI *UnregisterScriptArrayType)(ScriptArrayDef* type);

   // If called while processing a call, the scripting engine will handle the call failure
   void (MSGPIAPI *OnError)(unsigned int type, const char* message);

   // Allows to request a COM object to be overriden by our own implementation
   void (MSGPIAPI *SetCOMObjectOverride)(const char* classname, const ScriptClassDef* classDef);
   
   // Allow to use declared type library by other plugin
   ScriptClassDef* (MSGPIAPI *GetClassDef)(const char* typeName);

} ScriptablePluginAPI;




///////////////////////////////////////////////////////////////////////////////
// 
// The following helper macros are designed to easily expose C++ objects.
//
#ifdef __cplusplus

#include <cassert>
#include <string>
#include <memory>
#include <vector>
#include <map>

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
         va_list args_copy; \
         va_copy(args_copy, args); \
         int size = vsnprintf(nullptr, 0, format, args_copy); \
         va_end(args_copy); \
         if (size > 0) { \
            char* const buffer = new char[size + 1]; \
            vsnprintf(buffer, size + 1, format, args); \
            scriptablePluginApi->OnError(type, buffer); \
            delete [] buffer; \
         } \
         va_end(args); \
      } \
   }

// ScriptVariant access
#define PSC_VAR_bool(variant)         (variant).vBool
#define PSC_VAR_int(variant)          (variant).vInt
#define PSC_VAR_uint(variant)         (variant).vUInt
#define PSC_VAR_float(variant)        (variant).vFloat
#define PSC_VAR_double(variant)       (variant).vDouble
#define PSC_VAR_int8(variant)         (variant).vInt8
#define PSC_VAR_int16(variant)        (variant).vInt16
#define PSC_VAR_int32(variant)        (variant).vInt32
#define PSC_VAR_int64(variant)        (variant).vInt64
#define PSC_VAR_uint8(variant)        (variant).vUInt8
#define PSC_VAR_uint16(variant)       (variant).vUInt16
#define PSC_VAR_uint32(variant)       (variant).vUInt32
#define PSC_VAR_uint64(variant)       (variant).vUInt64
#define PSC_VAR_string(variant)       std::string((variant).vString.string)
#define PSC_VAR_enum(type, variant)   static_cast<type>((variant).vInt)
#define PSC_VAR_object(type, variant) static_cast<type *>((variant).vObject)

// Set native to ScriptVariant (including copy or AddRef). Value is always evaluated exactly once.
#define PSC_VAR_SET_void(variant, value)          value
#define PSC_VAR_SET_bool(variant, value)          (variant).vBool = value
#define PSC_VAR_SET_int(variant, value)           (variant).vInt = value
#define PSC_VAR_SET_uint(variant, value)          (variant).vUInt = value
#define PSC_VAR_SET_float(variant, value)         (variant).vFloat = value
#define PSC_VAR_SET_double(variant, value)        (variant).vDouble = value
#define PSC_VAR_SET_int8(variant, value)          (variant).vInt8 = value
#define PSC_VAR_SET_int16(variant, value)         (variant).vInt16 = value
#define PSC_VAR_SET_int32(variant, value)         (variant).vInt32 = value
#define PSC_VAR_SET_int64(variant, value)         (variant).vInt64 = value
#define PSC_VAR_SET_uint8(variant, value)         (variant).vUInt8 = value
#define PSC_VAR_SET_uint16(variant, value)        (variant).vUInt16 = value
#define PSC_VAR_SET_uint32(variant, value)        (variant).vUInt32 = value
#define PSC_VAR_SET_uint64(variant, value)        (variant).vUInt64 = value
#define PSC_VAR_SET_string(variant, value)        { const string& v=value; size_t n=v.length()+1; char* p = new char[n];  memcpy(p, v.c_str(), n); (variant).vString = { [](ScriptString* s) { delete[] s->string; }, p }; }
#define PSC_VAR_SET_enum(type, variant, value)    (variant).vInt = static_cast<int>(value)
#define PSC_VAR_SET_object(type, variant, value)  (variant).vObject = static_cast<void*>(value);

// Arrays macro allow to pass C++ std::vector to script arrays back and forth
// Since script arrays are not shared, but single owner, they are copied/released for each assignment
// If this is a performance issue, use reference counted objects instead (which are shared and not copied)

#define PSC_ARRAY1(classname, type, lowerBound) \
   static ScriptArrayDef classname##_SAD = { { #classname }, { #type }, 1, { lowerBound } }; \
   template <typename F> void Register##classname(F && regFunc) { regFunc(&classname##_SAD); } \
   template <typename F> void Unregister##classname(F && unregFunc) { unregFunc(&classname##_SAD); }

#define PSC_ARRAY2(classname, type, lowerBound1, lowerBound2) \
   static ScriptArrayDef classname##_SAD = { { #classname }, { #type }, 2, { lowerBound1, lowerBound2 } }; \
   template <typename F> void Register##classname(F&& regFunc) { regFunc(&classname##_SAD); } \
   template <typename F> void Unregister##classname(F&& unregFunc) { unregFunc(&classname##_SAD); }

#define PSC_VAR_array1(type, variant) \
   std::vector<type>( \
      reinterpret_cast<type *>(&(variant).vArray->lengths[1]), \
      reinterpret_cast<type *>(&(variant).vArray->lengths[1]) + (variant).vArray->lengths[0] )

#define PSC_VAR_SET_array1(type, variant, value) { \
      const unsigned int nDimensions = type##_SAD.nDimensions; \
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
      const unsigned int nDimensions = type##_SAD.nDimensions; \
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

#define PSC_CLASS_ALIAS(classname, alias) \
   template <typename F> void Register##classname(F && regFunc) { regFunc(#classname, #alias); } \
   template <typename F> void Unregister##classname(F && unregFunc) { unregFunc(#classname); }

#define PSC_CLASS_START(classname, typeName) \
   static ScriptClassDef* classname##_SCD = nullptr; \
   template<typename F> void Unregister##classname(F&& unregFunc) { \
      assert(classname##_SCD != nullptr); \
      unregFunc(classname##_SCD); \
      free(classname##_SCD); \
      classname##_SCD = nullptr; \
   } \
   template <typename F> void Register##classname(F && regFunc)                                                                                                                            \
   { \
      assert(classname##_SCD == nullptr); \
      std::vector<ScriptClassMemberDef> members; \
      const auto registerClass = [&members, &regFunc]() { \
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
         classname##_SCD = static_cast<ScriptClassDef*>(malloc(sizeof(ScriptClassDef) + members.size() * sizeof(ScriptClassMemberDef))); \
         classname##_SCD->name.name = #classname; \
         classname##_SCD->name.id = 0; \
         classname##_SCD->CreateObject = nullptr; \
         classname##_SCD->nMembers = static_cast<unsigned int>(members.size()); \
         for (size_t i = 0; i < members.size(); i++) \
            classname##_SCD->members[i] = members[i]; \
         regFunc(classname##_SCD); \
      }; \
      using _BindedClass = typeName; \
      members.push_back( { { "AddRef" }, { "uint32" }, 0, { }, \
         [](void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) { uint32_t rc = static_cast<_BindedClass*>(me)->AddRef(); if (pRet != nullptr) pRet->vUInt32 = rc; } } ); \
      members.push_back( { { "Release" }, { "uint32" }, 0, { }, \
         [](void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) { uint32_t rc = static_cast<_BindedClass*>(me)->Release(); if (pRet != nullptr) pRet->vUInt32 = rc; } } );

#define PSC_CLASS_END() registerClass(); }

#define PSC_IMPLEMENT_REFCOUNT() \
   protected: \
      mutable uint32_t m_refCount = 1; \
   public: \
      uint32_t AddRef() const { m_refCount++; return m_refCount; } \
      uint32_t Release() const { m_refCount--; uint32_t rc = m_refCount; if (rc == 0) delete this; return rc; }

#define PSC_ADD_REF(classDef, object) classDef->members[0].Call(object, 0, nullptr, nullptr)
#define PSC_RELEASE(classDef, object) classDef->members[1].Call(object, 1, nullptr, nullptr)

#define PSC_INHERIT_CLASS(parentName) \
   for (unsigned int i = 0; i < parentName##_SCD->nMembers; i++) { members.push_back(parentName##_SCD->members[i]); }

#define PSC_PROP_R(type, name) \
   members.push_back( { { #name }, { #type }, 0, { }, \
      [](void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) { \
         PSC_VAR_SET_##type(*pRet, static_cast<_BindedClass*>(me)->Get##name()); } });

#define PSC_PROP_R_ARRAY1(type, name, arg1) \
   members.push_back( { { #name }, { #type }, 1, { { #arg1 } }, \
      [](void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) { \
         PSC_VAR_SET_##type(*pRet, static_cast<_BindedClass*>(me)->Get##name(PSC_VAR_##arg1(pArgs[0]))); } });

#define PSC_PROP_W(type, name) \
   members.push_back({ { #name }, { "void" }, 1, { #type }, \
      [](void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) { \
         static_cast<_BindedClass*>(me)->Set##name(PSC_VAR_##type(pArgs[0])); } });

#define PSC_PROP_W_ARRAY1(type, name, arg1) \
   members.push_back({ { #name }, { "void" }, 2, { { #arg1 }, { #type } }, \
      [](void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) { \
         static_cast<_BindedClass*>(me)->Set##name(PSC_VAR_##arg1(pArgs[0]), PSC_VAR_##type(pArgs[1])); } });

#define PSC_PROP_RW(type, name) \
   PSC_PROP_R(type, name) \
   PSC_PROP_W(type, name)

#define PSC_PROP_RW_ARRAY1(type, name, arg1) \
   PSC_PROP_R_ARRAY1(type, name, arg1) \
   PSC_PROP_W_ARRAY1(type, name, arg1)

#define PSC_FUNCTION0(type, name) \
   members.push_back( { { #name }, { #type }, 0, { }, \
      [](void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) { \
         PSC_VAR_SET_##type(*pRet, static_cast<_BindedClass*>(me)->name()); } });

#define PSC_FUNCTION1(type, name, arg1) \
   members.push_back( { { #name }, { #type }, 1, { { #arg1 } }, \
      [](void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) { \
         PSC_VAR_SET_##type(*pRet, static_cast<_BindedClass*>(me)->name(PSC_VAR_##arg1(pArgs[0]))); } });

#define PSC_FUNCTION2(type, name, arg1, arg2) \
   members.push_back( { { #name }, { #type }, 2, { { #arg1 }, { #arg2 } }, \
      [](void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) { \
         PSC_VAR_SET_##type(*pRet, static_cast<_BindedClass*>(me)->name(PSC_VAR_##arg1(pArgs[0]), PSC_VAR_##arg2(pArgs[1]))); } });

#define PSC_FUNCTION3(type, name, arg1, arg2, arg3) \
   members.push_back( { { #name }, { #type }, 3, { { #arg1 }, { #arg2 }, { #arg3 } }, \
      [](void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) { \
         PSC_VAR_SET_##type(*pRet, static_cast<_BindedClass*>(me)->name(PSC_VAR_##arg1(pArgs[0]), PSC_VAR_##arg2(pArgs[1]), PSC_VAR_##arg3(pArgs[2]))); } });

#define PSC_FUNCTION4(type, name, arg1, arg2, arg3, arg4) \
   members.push_back( { { #name }, { #type }, 4, { { #arg1 }, { #arg2 }, { #arg3 }, { #arg4 } }, \
      [](void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) { \
         PSC_VAR_SET_##type(*pRet, static_cast<_BindedClass*>(me)->name(PSC_VAR_##arg1(pArgs[0]), PSC_VAR_##arg2(pArgs[1]), PSC_VAR_##arg3(pArgs[2]), PSC_VAR_##arg4(pArgs[3]))); } });

#define PSC_FUNCTION5(type, name, arg1, arg2, arg3, arg4, arg5) \
   members.push_back( { { #name }, { #type }, 5, { { #arg1 }, { #arg2 }, { #arg3 }, { #arg4 }, { #arg5 } }, \
      [](void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) { \
         PSC_VAR_SET_##type(*pRet, static_cast<_BindedClass *>(me)->name( PSC_VAR_##arg1(pArgs[0]), PSC_VAR_##arg2(pArgs[1]), PSC_VAR_##arg3(pArgs[2]), PSC_VAR_##arg4(pArgs[3]), PSC_VAR_##arg5(pArgs[4]))); } } );

#define PSC_FUNCTION6(type, name, arg1, arg2, arg3, arg4, arg5, arg6) \
   members.push_back( { { #name }, { #type }, 6, { { #arg1 }, { #arg2 }, { #arg3 }, { #arg4 }, { #arg5 }, { #arg6 } }, \
      [](void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) { \
         PSC_VAR_SET_##type(*pRet, static_cast<_BindedClass *>(me)->name( PSC_VAR_##arg1(pArgs[0]), PSC_VAR_##arg2(pArgs[1]), PSC_VAR_##arg3(pArgs[2]), PSC_VAR_##arg4(pArgs[3]), PSC_VAR_##arg5(pArgs[4]), PSC_VAR_##arg6(pArgs[5]))); } } );

#define PSC_FUNCTION7(type, name, arg1, arg2, arg3, arg4, arg5, arg6, arg7) \
   members.push_back( { { #name }, { #type }, 7, { { #arg1 }, { #arg2 }, { #arg3 }, { #arg4 }, { #arg5 }, { #arg6 }, { #arg7 } }, \
      [](void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) { \
         PSC_VAR_SET_##type(*pRet, static_cast<_BindedClass *>(me)->name( PSC_VAR_##arg1(pArgs[0]), PSC_VAR_##arg2(pArgs[1]), PSC_VAR_##arg3(pArgs[2]), PSC_VAR_##arg4(pArgs[3]), PSC_VAR_##arg5(pArgs[4]), PSC_VAR_##arg6(pArgs[5]), \
                                                                          PSC_VAR_##arg7(pArgs[6]))); } } );

#define PSC_FUNCTION8(type, name, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) \
   members.push_back( { { #name }, { #type }, 8, { { #arg1 }, { #arg2 }, { #arg3 }, { #arg4 }, { #arg5 }, { #arg6 }, { #arg7 }, { #arg8 } }, \
      [](void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) { \
         PSC_VAR_SET_##type(*pRet, static_cast<_BindedClass *>(me)->name( PSC_VAR_##arg1(pArgs[0]), PSC_VAR_##arg2(pArgs[1]), PSC_VAR_##arg3(pArgs[2]), PSC_VAR_##arg4(pArgs[3]), PSC_VAR_##arg5(pArgs[4]), PSC_VAR_##arg6(pArgs[5]), \
                                                                          PSC_VAR_##arg7(pArgs[6]), PSC_VAR_##arg8(pArgs[7]))); } } );

#define PSC_FUNCTION9(type, name, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9) \
   members.push_back( { { #name }, { #type }, 9, { { #arg1 }, { #arg2 }, { #arg3 }, { #arg4 }, { #arg5 }, { #arg6 }, { #arg7 }, { #arg8 }, { #arg9 } }, \
      [](void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) { \
         PSC_VAR_SET_##type(*pRet, static_cast<_BindedClass *>(me)->name( PSC_VAR_##arg1(pArgs[0]), PSC_VAR_##arg2(pArgs[1]), PSC_VAR_##arg3(pArgs[2]), PSC_VAR_##arg4(pArgs[3]), PSC_VAR_##arg5(pArgs[4]), PSC_VAR_##arg6(pArgs[5]), \
                                                                          PSC_VAR_##arg7(pArgs[6]), PSC_VAR_##arg8(pArgs[7]), PSC_VAR_##arg9(pArgs[8]))); } } );

#define PSC_FUNCTION10(type, name, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10) \
   members.push_back( { { #name }, { #type }, 10, { { #arg1 }, { #arg2 }, { #arg3 }, { #arg4 }, { #arg5 }, { #arg6 }, { #arg7 }, { #arg8 }, { #arg9 }, { #arg10 } }, \
      [](void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) { \
         PSC_VAR_SET_##type(*pRet, static_cast<_BindedClass *>(me)->name( PSC_VAR_##arg1(pArgs[0]), PSC_VAR_##arg2(pArgs[1]), PSC_VAR_##arg3(pArgs[2]), PSC_VAR_##arg4(pArgs[3]), PSC_VAR_##arg5(pArgs[4]), PSC_VAR_##arg6(pArgs[5]), \
                                                                          PSC_VAR_##arg7(pArgs[6]), PSC_VAR_##arg8(pArgs[7]), PSC_VAR_##arg9(pArgs[8]), PSC_VAR_##arg10(pArgs[9]))); } } );

#define PSC_FUNCTION11(type, name, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11) \
   members.push_back( { { #name }, { #type }, 11, { { #arg1 }, { #arg2 }, { #arg3 }, { #arg4 }, { #arg5 }, { #arg6 }, { #arg7 }, { #arg8 }, { #arg9 }, { #arg10 }, { #arg11 } }, \
      [](void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) { \
         PSC_VAR_SET_##type(*pRet, static_cast<_BindedClass *>(me)->name( PSC_VAR_##arg1(pArgs[0]), PSC_VAR_##arg2(pArgs[1]), PSC_VAR_##arg3(pArgs[2]), PSC_VAR_##arg4(pArgs[3]), PSC_VAR_##arg5(pArgs[4]), PSC_VAR_##arg6(pArgs[5]), \
                                                                          PSC_VAR_##arg7(pArgs[6]), PSC_VAR_##arg8(pArgs[7]), PSC_VAR_##arg9(pArgs[8]), PSC_VAR_##arg10(pArgs[9]), PSC_VAR_##arg11(pArgs[10]))); } } );

#define PSC_FUNCTION12(type, name, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12) \
   members.push_back( { { #name }, { #type }, 12, { { #arg1 }, { #arg2 }, { #arg3 }, { #arg4 }, { #arg5 }, { #arg6 }, { #arg7 }, { #arg8 }, { #arg9 }, { #arg10 }, { #arg10 }, { #arg12 } }, \
      [](void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) { \
         PSC_VAR_SET_##type(*pRet, static_cast<_BindedClass *>(me)->name( PSC_VAR_##arg1(pArgs[0]), PSC_VAR_##arg2(pArgs[1]), PSC_VAR_##arg3(pArgs[2]), PSC_VAR_##arg4(pArgs[3]), PSC_VAR_##arg5(pArgs[4]), PSC_VAR_##arg6(pArgs[5]), \
                                                                          PSC_VAR_##arg7(pArgs[6]), PSC_VAR_##arg8(pArgs[7]), PSC_VAR_##arg9(pArgs[8]), PSC_VAR_##arg10(pArgs[9]), PSC_VAR_##arg11(pArgs[10]), PSC_VAR_##arg12(pArgs[11]))); } } );


namespace ScriptablePlugin
{

class IScriptProxy
{
public:
   virtual ~IScriptProxy() = default;
   virtual void ForwardCall(void* me, int memberIndex, ScriptVariant* pArgs, ScriptVariant* pRet) = 0;
};


class ScriptClassProxy : public IScriptProxy
{
public:
   ScriptClassProxy(const MsgPluginAPI* msgApi, const unsigned int endpointId, const std::string& basePrefix, const std::string& baseName, const std::string& proxyPrefix,
      const ScriptClassDef* proxyClassDef)
      : m_msgApi(msgApi)
      , m_endpointId(endpointId)
      , m_getScriptApiId(msgApi->GetMsgID(SCRIPTPI_NAMESPACE, SCRIPTPI_MSG_GET_API))
      , m_onPluginLoaded(msgApi->GetMsgID(MSGPI_NAMESPACE, MSGPI_EVT_ON_PLUGIN_LOADED))
      , m_onPluginUnloaded(msgApi->GetMsgID(MSGPI_NAMESPACE, MSGPI_EVT_ON_PLUGIN_UNLOADED))
      , m_basePrefix(basePrefix)
      , m_baseName(baseName)
      , m_proxyPrefix(proxyPrefix)
      , m_proxyClassDef(proxyClassDef)
   {
      assert(m_baseName.starts_with(m_basePrefix));
      msgApi->BroadcastMsg(m_endpointId, m_getScriptApiId, &m_scriptApi);
      msgApi->SubscribeMsg(m_endpointId, m_onPluginLoaded, OnPluginLoaded, this);
      msgApi->SubscribeMsg(m_endpointId, m_onPluginUnloaded, OnPluginUnloaded, this);
      OnPluginLoaded(0, this, nullptr);
   }
   ~ScriptClassProxy()
   {
      m_msgApi->UnsubscribeMsg(m_onPluginLoaded, OnPluginLoaded, this);
      m_msgApi->UnsubscribeMsg(m_onPluginUnloaded, OnPluginUnloaded, this);
      m_msgApi->ReleaseMsgID(m_getScriptApiId);
      m_msgApi->ReleaseMsgID(m_onPluginLoaded);
      m_msgApi->ReleaseMsgID(m_onPluginUnloaded);
   }

   // Forward a call on the object to the one we proxied
   void ForwardCall(void* me, int memberIndex, ScriptVariant* pArgs, ScriptVariant* pRet) override
   {
      if (m_baseClassDef == nullptr)
      {
         // FIXME Report error to caller (unimplemented)
         return;
      }
      unsigned int baseMember = 0xFFFFFFFF;
      if (const auto& it = m_proxyToBaseMemberMap.find(memberIndex); it != m_proxyToBaseMemberMap.end())
      {
         baseMember = it->second;
      }
      else
      {
         m_proxyToBaseMemberMap[memberIndex] = 0xFFFFFFFF;
         for (unsigned int j = 0; j < m_baseClassDef->nMembers; j++)
         {
            if (const unsigned int nArgs = m_proxyClassDef->members[memberIndex].nArgs; (nArgs == m_baseClassDef->members[j].nArgs)
               && IsNameMatch(m_proxyClassDef->members[memberIndex].name.name, m_baseClassDef->members[j].name.name)
               && IsNameMatch(m_proxyClassDef->members[memberIndex].type.name, m_baseClassDef->members[j].type.name))
            {
               bool match = true;
               for (unsigned int k = 0; k < nArgs; k++)
                  match &= IsNameMatch(m_proxyClassDef->members[memberIndex].callArgType[k].name, m_baseClassDef->members[j].callArgType[k].name);
               if (match)
               {
                  baseMember = j;
                  m_proxyToBaseMemberMap[memberIndex] = j;
                  break;
               }
            }
         }
      }
      if (baseMember == 0xFFFFFFFF)
      {
         // FIXME Report error to caller (unimplemented)
         return;
      }
      m_baseClassDef->members[baseMember].Call(me, baseMember, pArgs, pRet);
   }

   const ScriptClassDef* GetBaseClassDef() const { return m_baseClassDef; }

private:
   inline bool IsNameMatch(const std::string_view& proxyName, const std::string_view& className) const
   {
      const std::string_view name1 = proxyName.starts_with(m_proxyPrefix) ? proxyName.substr(m_proxyPrefix.length()) : proxyName;
      const std::string_view name2 = className.starts_with(m_basePrefix) ? className.substr(m_basePrefix.length()) : className;
      return (name1.length() == name2.length()) && std::equal(name1.begin(), name1.end(), name2.begin(), [](unsigned char a, unsigned char b) { return std::tolower(a) == std::tolower(b); });
   }

   static void OnPluginLoaded(const unsigned int, void* userData, void*)
   {
      auto me = static_cast<ScriptClassProxy*>(userData);
      if (me->m_scriptApi && me->m_baseClassDef == nullptr)
      {
         if (const ScriptClassDef* classDef = me->m_scriptApi->GetClassDef(me->m_baseName.c_str()); classDef)
         {
            assert(me->m_baseClassDef == nullptr);
            me->m_baseClassDef = classDef;
         }
      }
   }

   static void OnPluginUnloaded(const unsigned int, void* userData, void*)
   {
      auto me = static_cast<ScriptClassProxy*>(userData);
      if (me->m_scriptApi && me->m_baseClassDef)
      {
         if (const ScriptClassDef* classDef = me->m_scriptApi->GetClassDef(me->m_baseName.c_str()); classDef == nullptr)
         {
            assert(me->m_baseClassDef != nullptr);
            me->m_proxyToBaseMemberMap.clear();
            me->m_baseClassDef = nullptr;
         }
      }
   }

   const MsgPluginAPI* m_msgApi;
   const unsigned int m_endpointId;
   const unsigned int m_getScriptApiId;
   const ScriptablePluginAPI* m_scriptApi = nullptr;
   const unsigned int m_onPluginLoaded;
   const unsigned int m_onPluginUnloaded;

   std::map<unsigned int, unsigned int> m_proxyToBaseMemberMap;

   // The base class we are proxying
   const std::string m_basePrefix;
   const std::string m_baseName;
   const ScriptClassDef* m_baseClassDef = nullptr;

   // The proxy (front facing interface)
   const std::string m_proxyPrefix;
   const ScriptClassDef* const m_proxyClassDef;
};

class ScriptObjectProxy : public IScriptProxy
{
public:
   ScriptObjectProxy(ScriptClassProxy& classProxy)
      : m_classProxy(classProxy)
   {
   }
   ~ScriptObjectProxy()
   {
      if (m_classProxy.GetBaseClassDef() && m_instance)
         PSC_RELEASE(m_classProxy.GetBaseClassDef(), m_instance);
   }

   // Forward a call on the object to the one we proxied, eventually creating the proxied instance if needed
   void ForwardCall(void* me, int memberIndex, ScriptVariant* pArgs, ScriptVariant* pRet) override
   {
      if (m_classProxy.GetBaseClassDef() == nullptr)
      {
         // FIXME Report error to caller (unimplemented)
         return;
      }
      if (m_instance == nullptr)
         m_instance = m_classProxy.GetBaseClassDef()->CreateObject();
      if (m_instance == nullptr)
      {
         // FIXME Report error to caller (failed to create object)
         return;
      }
      m_classProxy.ForwardCall(m_instance, memberIndex, pArgs, pRet);
   }

   // FIXME we should discard any instance created if the providing plugin is unloaded

private:
   void* m_instance = nullptr;
   ScriptClassProxy& m_classProxy;
};

#define PSC_PROXY_PROP_R(proxy, type, name)                                                                                                                                                  \
   members.push_back({ { #name }, { #type }, 0, {}, [](void* me, int id, ScriptVariant* pArgs, ScriptVariant* pRet) { static_cast<ScriptablePlugin::IScriptProxy*>(proxy)->ForwardCall(me, id, pArgs, pRet); } });

#define PSC_PROXY_PROP_R_ARRAY1(proxy, type, name, arg1)                                                                                                                                     \
   members.push_back({ { #name }, { #type }, 1, { { #arg1 } }, [](void* me, int id, ScriptVariant* pArgs, ScriptVariant* pRet) { static_cast<ScriptablePlugin::IScriptProxy*>(proxy)->ForwardCall(me, id, pArgs, pRet); } });

#define PSC_PROXY_PROP_R_ARRAY2(proxy, type, name, arg1, arg2)                                                                                                                                     \
   members.push_back({ { #name }, { #type }, 2, { { #arg1 }, { #arg2 } }, [](void* me, int id, ScriptVariant* pArgs, ScriptVariant* pRet) { static_cast<ScriptablePlugin::IScriptProxy*>(proxy)->ForwardCall(me, id, pArgs, pRet); } });

#define PSC_PROXY_PROP_R_ARRAY3(proxy, type, name, arg1, arg2, arg3)                                                                                                                                     \
   members.push_back({ { #name }, { #type }, 3, { { #arg1 }, { #arg2 }, { #arg3 } }, [](void* me, int id, ScriptVariant* pArgs, ScriptVariant* pRet) { static_cast<ScriptablePlugin::IScriptProxy*>(proxy)->ForwardCall(me, id, pArgs, pRet); } });

#define PSC_PROXY_PROP_R_ARRAY4(proxy, type, name, arg1, arg2, arg3, arg4)                                                                                                                                     \
   members.push_back({ { #name }, { #type }, 4, { { #arg1 }, { #arg2 }, { #arg3 }, { #arg4 } }, [](void* me, int id, ScriptVariant* pArgs, ScriptVariant* pRet) { static_cast<ScriptablePlugin::IScriptProxy*>(proxy)->ForwardCall(me, id, pArgs, pRet); } });

#define PSC_PROXY_PROP_W(proxy, type, name)                                                                                                                                                  \
   members.push_back({ { #name }, { "void" }, 1, { #type }, [](void* me, int id, ScriptVariant* pArgs, ScriptVariant* pRet) { static_cast<ScriptablePlugin::IScriptProxy*>(proxy)->ForwardCall(me, id, pArgs, pRet); } });

#define PSC_PROXY_PROP_W_ARRAY1(proxy, type, name, arg1)                                                                                                                                     \
   members.push_back({ { #name }, { "void" }, 2, { { #arg1 }, { #type } }, [](void* me, int id, ScriptVariant* pArgs, ScriptVariant* pRet) { static_cast<ScriptablePlugin::IScriptProxy*>(proxy)->ForwardCall(me, id, pArgs, pRet); } });

#define PSC_PROXY_PROP_RW(proxy, type, name)                                                                                                                                                 \
   PSC_PROXY_PROP_R(proxy, type, name)                                                                                                                                                       \
   PSC_PROXY_PROP_W(proxy, type, name)

#define PSC_PROXY_PROP_RW_ARRAY1(proxy, type, name, arg1)                                                                                                                                    \
   PSC_PROXY_PROP_R_ARRAY1(proxy, type, name, arg1)                                                                                                                                          \
   PSC_PROXY_PROP_W_ARRAY1(proxy, type, name, arg1)

#define PSC_PROXY_FUNCTION0(proxy, type, name)                                                                                                                                               \
   members.push_back({ { #name }, { #type }, 0, {}, [](void* me, int id, ScriptVariant* pArgs, ScriptVariant* pRet) { static_cast<ScriptablePlugin::IScriptProxy*>(proxy)->ForwardCall(me, id, pArgs, pRet); } });

#define PSC_PROXY_FUNCTION1(proxy, type, name, arg1)                                                                                                                                         \
   members.push_back({ { #name }, { #type }, 1, { { #arg1 } }, [](void* me, int id, ScriptVariant* pArgs, ScriptVariant* pRet) { static_cast<ScriptablePlugin::IScriptProxy*>(proxy)->ForwardCall(me, id, pArgs, pRet); } });

#define PSC_PROXY_FUNCTION2(proxy, type, name, arg1, arg2)                                                                                                                                   \
   members.push_back({ { #name }, { #type }, 2, { { #arg1 }, { #arg2 } }, [](void* me, int id, ScriptVariant* pArgs, ScriptVariant* pRet) { static_cast<ScriptablePlugin::IScriptProxy*>(proxy)->ForwardCall(me, id, pArgs, pRet); } });

};


#endif