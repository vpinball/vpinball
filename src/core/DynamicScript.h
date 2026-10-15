// license:GPLv3+

#pragma once

#include <atomic>
#include <thread>
#include <variant>
#include "plugins/ScriptablePlugin.h"
#include "unordered_dense.h"

class DynamicDispatch;

class DynamicTypeLibrary final
{
public:
   DynamicTypeLibrary();
   ~DynamicTypeLibrary();

   void Reset();

   void RegisterScriptClass(ScriptClassDef *classDef);
   void RegisterScriptTypeAlias(const char *name, const char *aliasedType);
   void RegisterScriptArray(ScriptArrayDef *arrayDef);
   void ResolveAllClasses();

   void UnregisterScriptClass(ScriptClassDef *classDef);
   void UnregisterScriptTypeAlias(const char *name);
   void UnregisterScriptArray(ScriptArrayDef *arrayDef);

   ScriptClassDef *ResolveClass(const char * name) const;
   int ResolveMemberId(const ScriptClassDef *classDef, const char *memberName) const;

   HRESULT Invoke(const ScriptClassDef * classDef, void *nativeObject, DISPID dispIdMember, REFIID, LCID, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *, UINT *) const;

   // Called through the plugin OnError callback to report a failure while a member call is being
   // processed. When a call is in flight on the calling thread, the message is recorded so that
   // Invoke can surface it as DISP_E_EXCEPTION to the script engine. Reports from other threads
   // (e.g. a plugin timer polling a missing provider) or outside of any call are ignored.
   void ReportCallError(const string &message) const; // message must not be empty (an empty pending error means no error)

private:
   void UnregisterTypeUsers(unsigned int id);
   int ResolveClassId(const char *name) const;
   ScriptClassDef *GetClass(const ScriptTypeNameDef &name) const;

   bool COMToScriptVariant(const VARIANT *cv, const ScriptTypeNameDef &type, ScriptVariant &sv) const;
   void InitScriptVariant(const ScriptTypeNameDef &type, ScriptVariant &sv) const;
   void ReleaseScriptVariant(const ScriptTypeNameDef &type, ScriptVariant &sv) const;
   void ScriptToCOMVariant(const ScriptTypeNameDef &type, ScriptVariant &sv, VARIANT *cv) const;
   string ScriptVariantToString(const ScriptTypeNameDef &type, const ScriptVariant &sv) const;

   struct ClassDef {
      ScriptClassDef * classDef;
      std::vector<std::vector<int>> members; // DispID (index in vector, corresponding to a case insensitive member name) to list of members (allowing overloads)
      ankerl::unordered_dense::map<string, int> memberMap; // Name to DispID map
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
   ankerl::unordered_dense::map<string, int> m_typenames; // Name to type id map

   // Error reported while a member call is processed: m_callThread identifies the thread executing
   // the call (empty when no call runs), m_callDepth tracks nested calls, m_pendingCallError the
   // first reported error of the current call. Only touched on the call thread, except the atomic
   // m_callThread which is read from other threads to filter out their reports.
   mutable std::atomic<std::thread::id> m_callThread;
   mutable int m_callDepth = 0;
   mutable std::string m_pendingCallError;
};


class DynamicDispatch final : public IDispatch
{
public:
   DynamicDispatch(const DynamicTypeLibrary* typeLibrary, const ScriptClassDef* classDef, void * nativeObject)
      : m_refCount(1)
      , m_classDef(classDef)
      , m_typeLibrary(typeLibrary)
      , m_nativeObject(nativeObject)
   {
      #ifdef DEBUG
      assert(classDef == typeLibrary->ResolveClass(classDef->name.name));
      #endif
      PSC_ADD_REF(m_classDef, m_nativeObject);
   }

   ~DynamicDispatch()
   {
      PSC_RELEASE(m_classDef, m_nativeObject);
   }

   // IUnknown methods
   HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject) override
   {
      if (riid == IID_IUnknown)
      {
         *ppvObject = static_cast<IUnknown *>(this);
         AddRef();
         return S_OK;
      }
      else if (riid == IID_IDispatch)
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
      return ++m_refCount;
   }

   ULONG STDMETHODCALLTYPE Release() override
   {
      ULONG refCount = --m_refCount;
      if (refCount == 0)
         delete this;
      return refCount;
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
      for (UINT i = 0; i < cNames; ++i)
      {
         const string sname = MakeString(rgszNames[i]);
         rgDispId[i] = m_typeLibrary->ResolveMemberId(m_classDef, sname.c_str());
         if (rgDispId[i] < 0)
         {
            PLOGE << m_classDef->name.name << '.' << sname << " was referenced while it is not declared. Did you forget to register a class member ?";
            return DISP_E_UNKNOWNNAME;
         }
         else
         {
            rgDispId[i]++; // Add one as DISPID 0 is reserved for default member
         }
      }
      return S_OK;
   }

   HRESULT STDMETHODCALLTYPE Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr) override
   {
      return m_typeLibrary->Invoke(m_classDef, m_nativeObject, dispIdMember, riid, lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);
   }

public:
   ULONG m_refCount;
   const ScriptClassDef *const m_classDef;
   const DynamicTypeLibrary *const m_typeLibrary;
   void *const m_nativeObject;
};
