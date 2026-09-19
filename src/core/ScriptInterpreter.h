#pragma once

#include <activscp.h>
#include <activdbg.h>
#include <atlcom.h>

#include "core/ScriptEngine.h"

class PinTable;

enum SecurityLevelEnum
{
   eSecurityNone = 0,
   eSecurityWarnOnUnsafeType = 1,
   eSecurityWarnOnType = 2,
   eSecurityWarnOnAll = 3,
   eSecurityNoControls = 4
};

class ScriptInterpreter : public IScriptEngine,
                          public CComObjectRoot,
                          public IActiveScriptSite,
                          public IActiveScriptSiteDebug,
                          public IActiveScriptSiteWindow,
                          public IInternetHostSecurityManager,
                          public IServiceProvider
{
public:
   ScriptInterpreter();
   virtual ~ScriptInterpreter();

   ScriptLanguage GetLanguage() const override { return ScriptLanguage::VBScript; }
   void Start(PinTable *table) override;
   void Stop(PinTable *table, bool interruptDirectly = false) override;
   using IScriptEngine::AddItem;
   void AddItem(const wstring& name, IDispatch *dispatch, const bool global) override;
   void RemoveItem(IScriptable *const piscript) override;
   void Evaluate(const string &script, bool isDebugStatement) override;
   bool HasError() const override { return m_hasError || (m_pScript == nullptr); }
   void GetScriptDispatch(IDispatch **ppdisp) const override;
   void SetScriptErrorHandler(const ErrorHandler &errorHandler) override { m_errorHandler = errorHandler; }
   void Dispose() override
   {
      const ULONG refCount = static_cast<IActiveScriptSite *>(this)->Release();
      assert(refCount == 0);
   }

   // IActiveScriptSite interface
   STDMETHOD(GetLCID)(LCID *plcid) override;
   STDMETHOD(GetItemInfo)(LPCOLESTR pstrName, DWORD dwReturnMask, IUnknown **ppiunkItem, ITypeInfo **ppti) override;
   STDMETHOD(GetDocVersionString)(BSTR *pbstrVersion) override;
   STDMETHOD(OnScriptTerminate)(const VARIANT *pvr, const EXCEPINFO *pei) override;
   STDMETHOD(OnStateChange)(SCRIPTSTATE ssScriptState) override;
   STDMETHOD(OnScriptError)(IActiveScriptError *pscripterror) override;
   STDMETHOD(OnEnterScript)() override;
   STDMETHOD(OnLeaveScript)() override;

   // IActiveScriptSiteWindow interface
   STDMETHOD(GetWindow)(HWND *phwnd) override;
   STDMETHOD(EnableModeless)(BOOL) override;

   // IActiveScriptSiteDebug interface
   STDMETHOD(GetDocumentContextFromPosition)(DWORD_PTR dwSourceContext, ULONG uCharacterOffset, ULONG uNumChars, IDebugDocumentContext **ppsc) override;
   STDMETHOD(GetApplication)(IDebugApplication **ppda) override;
   STDMETHOD(GetRootApplicationNode)(IDebugApplicationNode **ppdanRoot) override;
   STDMETHOD(OnScriptErrorDebug)(IActiveScriptErrorDebug *pscripterror, BOOL *pfEnterDebugger, BOOL *pfCallOnScriptErrorWhenContinuing) override;

   // Internet Security interface
   STDMETHOD(GetSecurityId)(BYTE *pbSecurityId, DWORD *pcbSecurityId, DWORD_PTR dwReserved) override;
   STDMETHOD(ProcessUrlAction)(DWORD dwAction, BYTE __RPC_FAR *pPolicy, DWORD cbPolicy, BYTE __RPC_FAR *pContext, DWORD cbContext, DWORD dwFlags, DWORD dwReserved) override;
   STDMETHOD(QueryCustomPolicy)(REFGUID guidKey, BYTE __RPC_FAR *__RPC_FAR *ppPolicy, DWORD __RPC_FAR *pcbPolicy, BYTE __RPC_FAR *pContext, DWORD cbContext, DWORD dwReserved) override;

   // IServiceProvider interface
   STDMETHOD(QueryService)(REFGUID guidService, REFIID riid, void **ppv) override;

   // Use CComObject to implement AddRef/Release/QueryInterface
   BEGIN_COM_MAP(ScriptInterpreter)
   //COM_INTERFACE_ENTRY(IDispatch)
   COM_INTERFACE_ENTRY(IActiveScriptSite)
   COM_INTERFACE_ENTRY(IActiveScriptSiteDebug)
   COM_INTERFACE_ENTRY(IActiveScriptSiteWindow)
   COM_INTERFACE_ENTRY(IInternetHostSecurityManager)
   COM_INTERFACE_ENTRY(IServiceProvider)
   END_COM_MAP()

private:
   bool IsControlAlreadyOkayed(const CONFIRMSAFETY *pcs) const;
   void AddControlToOkayedList(const CONFIRMSAFETY *pcs) const;
   static bool IsControlMarkedSafe(const CONFIRMSAFETY *pcs);
   bool IsUserManuallyOkaysControl(const CONFIRMSAFETY *pcs) const;

   struct ScriptItem final
   {
      wstring m_wName;
      IUnknown *m_punk = nullptr;
      IDispatch *m_pdisp = nullptr;
      bool m_global = false;
   };
   ankerl::unordered_dense::map<std::wstring, std::unique_ptr<ScriptItem>> m_scriptItemMap;

   bool m_hasError = false;
   void HandleScriptError(IActiveScriptError *pScriptError, IActiveScriptErrorDebug *pScriptDebugError);
   ErrorHandler m_errorHandler;

   IActiveScript *m_pScript = nullptr;
   IActiveScriptParse *m_pScriptParse = nullptr;
   IActiveScriptDebug *m_pScriptDebug = nullptr;

   /**
    * Will be nullptr on systems that don't support debugging.
    *
    * For example, wine 6.9 says ...
    * > no class object {78a51822-51f4-11d0-8f20-00805f2cd064} could be created for context 0x17
    * ... if I try to create CLSID_ProcessDebugManager
    */
#ifndef __STANDALONE__
   IProcessDebugManager *m_pProcessDebugManager = nullptr;
#endif

   const DWORD m_compileContextCookie = 1000;
   const DWORD m_debugContextCookie = 1001;

   class DebuggerModule :
      public CComObjectRootEx<CComSingleThreadModel>,
      public IDispatchImpl<IVPDebug, &IID_IVPDebug, &LIBID_VPinballLib>,
      public IScriptable
   {
#ifdef __STANDALONE__
   public:
      STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId);
      STDMETHOD(Invoke)(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr);
      STDMETHOD(GetDocumentation)(MEMBERID index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile);
#endif
      BEGIN_COM_MAP(DebuggerModule)
      COM_INTERFACE_ENTRY(IVPDebug)
      COM_INTERFACE_ENTRY(IDispatch)
      END_COM_MAP()

      STDMETHOD(Print)(VARIANT *pvar) override;

   public:
      DebuggerModule() { m_wzName = L"Debug"sv; }
      IDispatch *GetIDispatch() final { return (IDispatch *)this; }
      const IDispatch *GetIDispatch() const final { return (const IDispatch *)this; }

      STDMETHOD(get_Name)(BSTR *pVal) override
      {
         *pVal = SysAllocStringLen(m_wzName.c_str(), static_cast<UINT>(m_wzName.length()));
         return S_OK;
      }
   };
   CComObject<DebuggerModule> *m_pdm; // Object to expose to script for global Debug.xxx functions
};
