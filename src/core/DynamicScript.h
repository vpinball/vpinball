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
   void RegisterScriptClass(ScriptClassDef *classDef);
   void RegisterScriptTypeAlias(const char *name, const char *aliasedType);
   void RegisterScriptArray(ScriptArrayDef *arrayDef);

   ScriptClassDef *ResolveClass(ScriptTypeNameDef &name) const;

   HRESULT STDMETHODCALLTYPE Invoke(void *nativeObject, DISPID dispIdMember, REFIID, LCID, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *, UINT *) const;

private:
   void ResolveClassId(ScriptTypeNameDef &typeName) const;
   ScriptClassDef *GetClassDef(const ScriptTypeNameDef &name) const;

   void COMToScriptVariant(const VARIANT &cv, ScriptTypeNameDef &type, ScriptVariant &sv) const;
   void ReleaseCOMToScriptVariant(VARIANT &cv, const ScriptTypeNameDef &type, ScriptVariant &sv) const;
   void ScriptToCOMVariant(ScriptTypeNameDef &type, const ScriptVariant &sv, VARIANT &cv) const;

   struct TypeDef {
      enum {
         TD_NATIVE, TD_CLASS, TD_ALIAS, TD_ARRAY
      } category;
      union
      {
         ScriptTypeNameDef nativeType;
         ScriptClassDef *classDef;
         ScriptArrayDef *arrayDef;
         struct
         {
            const char* alias;
            ScriptTypeNameDef typeDef;
         } aliasDef;
      };
   };
   std::vector<TypeDef> m_types;
   robin_hood::unordered_map<string, int> m_typenames;

   std::vector<ScriptClassDef *> m_scriptClasses;
   std::vector<std::pair<ScriptClassDef *, unsigned int>> m_scriptMembers;
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
      ScriptTypeNameDef type { classDef->name.name };
      assert(classDef == typeLibrary->ResolveClass(type));
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
      m_classDef->members[0].Call(m_nativeObject, 0, &refCount, nullptr);
      return refCount.vLong;
   }

   ULONG STDMETHODCALLTYPE Release() override
   {
      ScriptVariant refCount;
      m_classDef->members[1].Call(m_nativeObject, 1, &refCount, nullptr);
      if (refCount.vLong == 0)
         delete this;
      return refCount.vLong;
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
         bool found = false;
         #ifdef __STANDALONE__
         const string sname = MakeString(wstring(rgszNames[i]));
         LPCSTR name = sname.c_str();
         #else
         LPCSTR name = OLE2CA(rgszNames[i]);
         #endif
         // TODO use a hash map to speed up things
         for (UINT j = 0; j < m_classDef->nMembers; j++)
         {
            if (lstrcmpi(m_classDef->members[j].name.name, name) == 0)
            {
               found = true;
               rgDispId[i] = static_cast<DISPID>(m_classDef->members[j].name.id);
               break;
            }
         }
         if (!found)
         {
            PLOGE << m_classDef->name.name << "." << name << " was referenced while it is not declared. Did you forget to register a class member ?";
            return DISP_E_UNKNOWNNAME;
         }
      }
      return S_OK;
   }

   HRESULT STDMETHODCALLTYPE Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr) override
   {
      return m_typeLibrary->Invoke(m_nativeObject, dispIdMember, riid, lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);
   }

public:
   const ScriptClassDef *m_classDef;
   const DynamicTypeLibrary *m_typeLibrary;
   void *const m_nativeObject;
};