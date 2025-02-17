// license:GPLv3+

#pragma once

#include "core/stdafx.h"
#include <variant>
#include "plugins/ScriptablePlugin.h"
#include "robin_hood.h"

class DynamicDispatch;

class DynamicTypeLibrary
{
public:
   DynamicTypeLibrary();
   ~DynamicTypeLibrary();
   
   void RegisterScriptClass(ScriptClassDef *classDef);
   void RegisterScriptTypeAlias(const char *name, const char *aliasedType);
   void RegisterScriptArray(ScriptArrayDef *arrayDef);
   void ResolveAllClasses();

   // FIXME allow to unregister
   
   ScriptClassDef *ResolveClass(const char * name) const;
   int ResolveMemberId(const ScriptClassDef *classDef, const char *memberName) const;

   HRESULT Invoke(const ScriptClassDef * classDef, void *nativeObject, DISPID dispIdMember, REFIID, LCID, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *, UINT *) const;

private:
   int ResolveClassId(const char *name) const;
   ScriptClassDef *GetClass(const ScriptTypeNameDef &name) const;

   void COMToScriptVariant(const VARIANT &cv, const ScriptTypeNameDef &type, ScriptVariant &sv) const;
   void ReleaseCOMToScriptVariant(VARIANT &cv, const ScriptTypeNameDef &type, ScriptVariant &sv) const;
   void ScriptToCOMVariant(const ScriptTypeNameDef &type, const ScriptVariant &sv, VARIANT &cv) const;
   string ScriptVariantToString(const ScriptTypeNameDef &type, const ScriptVariant &sv) const;

   struct ClassDef {
      ScriptClassDef * classDef;
      std::vector<std::vector<int>> members; // DispID (index in vector, corresponding to a case insensitive member name) to list of members (allowing overloads)
      robin_hood::unordered_map<string, int> memberMap; // Name to DispID map
   };
   struct TypeDef {
      enum {
         TD_INVALID, TD_NATIVE, TD_CLASS, TD_ALIAS, TD_ARRAY
      } category;
      union
      {
         ScriptTypeNameDef nativeType;
         ClassDef *classDef;
         ScriptArrayDef *arrayDef;
         struct
         {
            const char* alias;
            ScriptTypeNameDef typeDef;
         } aliasDef;
      };
   };
   std::vector<TypeDef> m_types; // id (index in vector, corresponding to a case insensitive type name) to type definition
   robin_hood::unordered_map<string, int> m_typenames; // Name to type id map
};


class DynamicDispatch : public IDispatch
{
public:
   DynamicDispatch(const DynamicTypeLibrary* typeLibrary, const ScriptClassDef* classDef, void * nativeObject)
      : m_typeLibrary(typeLibrary)
      , m_classDef(classDef)
      , m_nativeObject(nativeObject)
   {
      #ifdef DEBUG
      assert(classDef == typeLibrary->ResolveClass(classDef->name.name));
      #endif
   }

   // IUnknown methods
   HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject) override
   {
      if (riid == IID_IUnknown || riid == IID_IDispatch)
      {
         *ppvObject = static_cast<IDispatch *>(this);
         AddRef();
         return S_OK;
      }
      *ppvObject = nullptr;
      return E_NOINTERFACE;
   }

   ULONG STDMETHODCALLTYPE AddRef() override
   {
      ScriptVariant refCount;
      m_classDef->members[0].Call(m_nativeObject, 0, nullptr, &refCount);
      return refCount.vULong;
   }

   ULONG STDMETHODCALLTYPE Release() override
   {
      ScriptVariant refCount;
      m_classDef->members[1].Call(m_nativeObject, 1, nullptr, &refCount);
      if (refCount.vLong == 0)
         delete this;
      return refCount.vULong;
   }

   // IDispatch methods
   HRESULT STDMETHODCALLTYPE GetTypeInfoCount(UINT *pctinfo) override
   {
      if (!pctinfo)
         return E_POINTER;
      *pctinfo = 0; // No type info
      return S_OK;
   }

   HRESULT STDMETHODCALLTYPE GetTypeInfo(UINT, LCID, ITypeInfo **) override
   {
      return E_NOTIMPL;
   }

   HRESULT STDMETHODCALLTYPE GetIDsOfNames(REFIID, LPOLESTR *rgszNames, UINT cNames, LCID, DISPID *rgDispId) override
   {
      if (!rgszNames || !rgDispId)
         return E_POINTER;
      #ifndef __STANDALONE__
      USES_CONVERSION;
      #endif
      for (UINT i = 0; i < cNames; ++i)
      {
         #ifdef __STANDALONE__
         const string sname = MakeString(wstring(rgszNames[i]));
         LPCSTR name = sname.c_str();
         #else
         LPCSTR name = OLE2CA(rgszNames[i]);
         #endif
         rgDispId[i] = m_typeLibrary->ResolveMemberId(m_classDef, name);
         if (rgDispId[i] < 0)
         {
            PLOGE << m_classDef->name.name << "." << name << " was referenced while it is not declared. Did you forget to register a class member ?";
            return DISP_E_UNKNOWNNAME;
         }
      }
      return S_OK;
   }

   HRESULT STDMETHODCALLTYPE Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr) override
   {
      return m_typeLibrary->Invoke(m_classDef, m_nativeObject, dispIdMember, riid, lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);
   }

public:
   const ScriptClassDef *m_classDef;
   const DynamicTypeLibrary *m_typeLibrary;
   void *const m_nativeObject;
};