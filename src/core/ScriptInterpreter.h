#pragma once

#include <activscp.h>
#include <activdbg.h>
#include <atlcom.h>

enum SecurityLevelEnum
{
   eSecurityNone = 0,
   eSecurityWarnOnUnsafeType = 1,
   eSecurityWarnOnType = 2,
   eSecurityWarnOnAll = 3,
   eSecurityNoControls = 4
};

class ScriptInterpreter : public CComObjectRoot,
                          public IActiveScriptSite,
                          public IActiveScriptSiteDebug,
                          public IActiveScriptSiteWindow,
                          public IInternetHostSecurityManager,
                          public IServiceProvider
{
public:
   ScriptInterpreter();
   virtual ~ScriptInterpreter();

   void Start(PinTable *table);
   void Stop(PinTable *table, bool interruptDirectly = false);
   void AddItem(IScriptable *scriptable, const bool global) { AddItem(scriptable->get_Name(), scriptable->GetDispatch(), global); }
   void AddItem(const WCHAR *name, IDispatch *dispatch, const bool global);
   void RemoveItem(IScriptable *const piscript);
   void Evaluate(const string &script, bool isDebugStatement);
   bool HasError() const { return m_hasError || (m_pScript == nullptr); }
   void GetScriptDispatch(IDispatch **ppdisp) const;

   enum class ErrorType
   {
      Compile,
      Runtime,
      DebugConsole
   };
   void SetScriptErrorHandler(const std::function<void(ErrorType, int, int, const string &, const vector<string> &)> &errorHandler) { m_errorHandler = errorHandler; }

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
   ankerl::unordered_dense::map<std::wstring, std::unique_ptr<ScriptItem>> m_vcvd;

   bool m_hasError = false;
   void HandleScriptError(IActiveScriptError *pScriptError, IActiveScriptErrorDebug *pScriptDebugError);
   std::function<void(ErrorType, int, int, const string &, const vector<string> &)> m_errorHandler;

   IActiveScript *m_pScript = nullptr;
   IActiveScriptParse *m_pScriptParse = nullptr;
   IActiveScriptDebug *m_pScriptDebug = nullptr;

   /**
    * Will be nullptr on systems that don't support debugging.
    * 
    * For example, wine 6.9 says ...
    * > no class object {78a51822-51f4-11d0-8f20-00805f2cd064} could be created for context 0x17
    * ... if I try to create CLSID_PrrocessDebugManager
    */
#ifndef __STANDALONE__
   IProcessDebugManager *m_pProcessDebugManager = nullptr;
#endif

   const DWORD m_compileContextCookie = 1000;
   const DWORD m_debugContextCookie = 1001;

   class DebuggerModule : public CComObjectRootEx<CComSingleThreadModel>, public IDispatchImpl<IVPDebug, &IID_IVPDebug, &LIBID_VPinballLib>, public IScriptable
   {
#ifdef __STANDALONE__
   public:
      STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId);
      STDMETHOD(Invoke)(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr);
      STDMETHOD(GetDocumentation)(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile);
#endif
      BEGIN_COM_MAP(DebuggerModule)
      COM_INTERFACE_ENTRY(IVPDebug)
      COM_INTERFACE_ENTRY(IDispatch)
      END_COM_MAP()

      STDMETHOD(Print)(VARIANT *pvar) override;

   public:
      IDispatch *GetDispatch() final { return (IDispatch *)this; }
      const IDispatch *GetDispatch() const final { return (const IDispatch *)this; }

      ISelect *GetISelect() final { return nullptr; }
      const ISelect *GetISelect() const final { return nullptr; }

      const WCHAR *get_Name() const final { return L"Debug"; }
      STDMETHOD(get_Name)(BSTR *pVal) override
      {
         *pVal = SysAllocString(L"Debug");
         return S_OK;
      }
   };
   CComObject<DebuggerModule> *m_pdm; // Object to expose to script for global Debug.xxx functions
};
