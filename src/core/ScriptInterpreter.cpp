#include "core/stdafx.h"

#include "core/vpversion.h"

#include "ScriptInterpreter.h"
#include "ScriptGlobalTable.h"

#ifndef __STANDALONE__
#include <initguid.h>
#endif

// The GUID used to identify the coclass of the VB Script engine {B54F3741-5B07-11cf-A4B0-00AA004A55E8}
DEFINE_GUID(CLSID_VBScript, 0xb54f3741, 0x5b07, 0x11cf, 0xa4, 0xb0, 0x0, 0xaa, 0x0, 0x4a, 0x55, 0xe8);
//DEFINE_GUID(IID_IActiveScriptParse32, 0xbb1a2ae2, 0xa4f9, 0x11cf, 0x8f, 0x20, 0x0, 0x80, 0x5f, 0x2c, 0xd0, 0x64);
//DEFINE_GUID(IID_IActiveScriptParse64,0xc7ef7658,0xe1ee,0x480e,0x97,0xea,0xd5,0x2c,0xb4,0xd7,0x6d,0x17);
//DEFINE_GUID(IID_IActiveScriptDebug, 0x51973C10, 0xCB0C, 0x11d0, 0xB5, 0xC9, 0x00, 0xA0, 0x24, 0x4A, 0x0E, 0x7A);


ScriptInterpreter::ScriptInterpreter()
{
   CComObject<DebuggerModule>::CreateInstance(&m_pdm);
   m_pdm->AddRef();

   const HRESULT vbScriptResult = CoCreateInstance(
      CLSID_VBScript, 0, CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER | CLSCTX_LOCAL_SERVER, IID_IActiveScriptParse, (LPVOID *)&m_pScriptParse); //!! CLSCTX_INPROC_SERVER good enough?!
   if (vbScriptResult != S_OK)
      return;

#ifndef __STANDALONE__
   // This can fail on some systems (I tested with wine 6.9 and this fails)
   // In that case, m_pProcessDebugManager will remain as nullptr
   CoCreateInstance(CLSID_ProcessDebugManager, 0, CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER | CLSCTX_LOCAL_SERVER, IID_IProcessDebugManager, (LPVOID *)&m_pProcessDebugManager);

   // Also check if we have a debugger installed
   // If not, we should abandon the process debug manager and fall back to plain basic errors
   IDebugApplication *debugApp;
   if (SUCCEEDED(GetApplication(&debugApp)))
   {
      debugApp->Release();
   }
   else
   {
      if (m_pProcessDebugManager)
      {
         m_pProcessDebugManager->Release();
         m_pProcessDebugManager = nullptr;
      }
   }
#endif

   m_pScriptParse->QueryInterface(IID_IActiveScript, (LPVOID *)&m_pScript);
   m_pScriptParse->QueryInterface(IID_IActiveScriptDebug, (LPVOID *)&m_pScriptDebug);
   m_pScriptParse->InitNew();

#ifndef __STANDALONE__
   IObjectSafety *pios;
   m_pScriptParse->QueryInterface(IID_IObjectSafety, (LPVOID *)&pios);
   if (pios)
   {
      DWORD supported, enabled;
      pios->GetInterfaceSafetyOptions(IID_IActiveScript, &supported, &enabled);
      pios->SetInterfaceSafetyOptions(IID_IActiveScript, supported, INTERFACE_USES_SECURITY_MANAGER);
      pios->Release();
   }
#endif
}

ScriptInterpreter::~ScriptInterpreter()
{
   if (m_pScript)
   {
      // Cleanly wait for the script to end to allow Exit event, triggered just before closing, to be processed
      SCRIPTSTATE state;
      m_pScript->GetScriptState(&state);
      if (state != SCRIPTSTATE_CLOSED && state != SCRIPTSTATE_UNINITIALIZED)
      {
         m_pScript->Close();
         const uint32_t startWaitTick = msec();
         while ((msec() - startWaitTick < 5000) && (state != SCRIPTSTATE_CLOSED))
         {
            Sleep(16);
            m_pScript->GetScriptState(&state);
         }
         if (state != SCRIPTSTATE_CLOSED)
         {
            PLOGE << "Script did not terminate within 5s after request. Forcing close of interpreter #" << m_pScript;
            EXCEPINFO eiInterrupt = {};
            eiInterrupt.bstrDescription = MakeWideBSTR(LocalString(IDS_HANG).m_szbuffer);
            //eiInterrupt.scode = E_NOTIMPL;
            eiInterrupt.wCode = 2345;
            m_pScript->InterruptScriptThread(SCRIPTTHREADID_BASE /*SCRIPTTHREADID_ALL*/, &eiInterrupt, /*SCRIPTINTERRUPT_DEBUG*/ SCRIPTINTERRUPT_RAISEEXCEPTION);
         }
         else
         {
            PLOGI << "Script interpreter state is now closed. Releasing interpreter #" << m_pScript;
         }
      }
      SAFE_RELEASE_NO_RCC(m_pScript);
      SAFE_RELEASE_NO_RCC(m_pScriptParse);
      SAFE_RELEASE(m_pScriptDebug);
#ifndef __STANDALONE__
      if (m_pProcessDebugManager != nullptr)
         m_pProcessDebugManager->Release();
#endif
   }
   m_pdm->Release();
}

void ScriptInterpreter::Init(PinTable* table)
{
   if (m_pScript)
   {
      m_pScript->SetScriptSite(this);
      m_pScript->SetScriptState(SCRIPTSTATE_INITIALIZED);
      m_pScript->AddTypeLib(LIBID_VPinballLib, 1, 0, 0);
   }
   AddItem(table, false);
   AddItem((ScriptGlobalTable*) table->m_psgt, true);
   AddItem(m_pdm, false);
   for (int i = 0; i < table->m_vcollection.size(); i++)
      AddItem(&table->m_vcollection[i], false);
   for (auto editable : table->GetParts())
      if (editable->GetScriptable())
         AddItem(editable->GetScriptable(), false);
}

void ScriptInterpreter::AddItem(const WCHAR *name, IDispatch *dispatch, const bool global)
{
   if (auto it = m_vcvd.find(name); it != m_vcvd.end())
   {
      PLOGE << "Script item with name '" << MakeString(name) << "' already exists. Skipping addition of this item.";
      return;
   }

   {
      auto pcvd = std::make_unique<ScriptItem>();
      pcvd->m_wName = name;
      pcvd->m_pdisp = dispatch;
      pcvd->m_pdisp->QueryInterface(IID_IUnknown, (void **)&pcvd->m_punk);
      pcvd->m_punk->Release();
      pcvd->m_global = global;
      m_vcvd[pcvd->m_wName] = std::move(pcvd);
   }

   int flags = SCRIPTITEM_ISSOURCE | SCRIPTITEM_ISVISIBLE;
   if (global)
      flags |= SCRIPTITEM_GLOBALMEMBERS;
   if (m_pScript != nullptr)
      m_pScript->AddNamedItem(name, flags);
}

void ScriptInterpreter::RemoveItem(const IScriptable *const piscript)
{
   auto it = m_vcvd.find(piscript->get_Name());
   if (it != m_vcvd.end())
      m_vcvd.erase(it);
}

void ScriptInterpreter::Evaluate(const string &script, bool isDebugStatement)
{
   WCHAR *const wzScript = MakeWide(script);
   EXCEPINFO exception {};
   if (m_pScriptParse)
      m_pScriptParse->ParseScriptText(wzScript, isDebugStatement ? L"Debug" : nullptr, nullptr, nullptr, isDebugStatement ? m_debugContextCookie : m_compileContextCookie, 0,
         isDebugStatement ? 0 : SCRIPTTEXT_ISVISIBLE, nullptr, &exception);
   delete[] wzScript;
   if (m_pScript)
      m_pScript->SetScriptState(SCRIPTSTATE_CONNECTED);
}

void ScriptInterpreter::GetScriptDispatch(IDispatch **ppdisp) const
{
   if (m_pScript)
      m_pScript->GetScriptDispatch(nullptr, ppdisp);
   else
      *ppdisp = nullptr;
}

void ScriptInterpreter::Stop(bool interruptDirectly)
{
   if (m_pScript == nullptr)
      return;

   SCRIPTSTATE state;
   m_pScript->GetScriptState(&state);
   if (state != SCRIPTSTATE_CLOSED && state != SCRIPTSTATE_UNINITIALIZED)
   {
      m_pScript->Close();
      if (!interruptDirectly)
      {
         const uint32_t startWaitTick = msec();
         while ((msec() - startWaitTick < 5000) && (state != SCRIPTSTATE_CLOSED))
         {
            Sleep(16);
            m_pScript->GetScriptState(&state);
         }
      }
      if (state != SCRIPTSTATE_CLOSED)
      {
         PLOGE << "Script did not terminate within 5s after request. Forcing close of interpreter #" << m_pScript;
         EXCEPINFO eiInterrupt = {};
         eiInterrupt.bstrDescription = MakeWideBSTR(LocalString(IDS_HANG).m_szbuffer);
         //eiInterrupt.scode = E_NOTIMPL;
         eiInterrupt.wCode = 2345;
         m_pScript->InterruptScriptThread(SCRIPTTHREADID_BASE /*SCRIPTTHREADID_ALL*/, &eiInterrupt, /*SCRIPTINTERRUPT_DEBUG*/ SCRIPTINTERRUPT_RAISEEXCEPTION);
      }
   }
}

void ScriptInterpreter::HandleScriptError(IActiveScriptError *pScriptError, IActiveScriptErrorDebug* pScriptDebugError)
{
   m_hasError = true;

   // Get stack trace
   vector<string> stackDump;
#ifndef __STANDALONE__
   if (pScriptDebugError)
   {
      if (IDebugStackFrame * errStackFrame; pScriptDebugError->GetStackFrame(&errStackFrame) == S_OK)
      {
         IDebugApplicationThread *thread;
         errStackFrame->GetThread(&thread);
         if (thread)
         {
            IEnumDebugStackFrames *stackFramesEnum;
            thread->EnumStackFrames(&stackFramesEnum);

            DebugStackFrameDescriptor stackFrames[128];
            ULONG numStackFrames;
            stackFramesEnum->Next(128, stackFrames, &numStackFrames);

            for (ULONG i = 0; i < numStackFrames; i++)
            {
               std::wstringstream callSite;

               // The frame description is the name of the function in this stack frame
               BSTR frameDesc;
               stackFrames[i].pdsf->GetDescriptionString(TRUE, &frameDesc);
               callSite << frameDesc;
               SysFreeString(frameDesc);

               // Fetch local variables and args
               IDebugProperty *debugProp;
               stackFrames[i].pdsf->GetDebugProperty(&debugProp);

               IEnumDebugPropertyInfo *propInfoEnum;
               debugProp->EnumMembers(PROP_INFO_FULLNAME | PROP_INFO_VALUE,
                  10, // Radix (for numerical info)
                  IID_IDebugPropertyEnumType_LocalsPlusArgs, &propInfoEnum);

               DebugPropertyInfo infos[128];
               ULONG numInfos;
               propInfoEnum->Next(128, infos, &numInfos);

               if (numInfos > 0)
               {
                  callSite << L" (";
                  for (ULONG i2 = 0; i2 < numInfos; i2++)
                  {
                     callSite << infos[i2].m_bstrFullName << L'=' << infos[i2].m_bstrValue;
                     // Add a comma if this isn't the last item in the list
                     if (i2 != numInfos - 1)
                        callSite << L", ";
                  }
                  callSite << L")";
               }

               propInfoEnum->Release();
               debugProp->Release();

               stackDump.push_back(MakeString(callSite.str()));
            }

            stackFramesEnum->Release();
            thread->Release();
         }
      }
   }
#endif

   DWORD dwCookie;
   ULONG nLine;
   LONG nChar;
   pScriptError->GetSourcePosition(&dwCookie, &nLine, &nChar);

   BSTR bstr = nullptr;
   pScriptError->GetSourceLineText(&bstr);
   SysFreeString(bstr);

   EXCEPINFO exception = {};
   pScriptError->GetExceptionInfo(&exception);
   const string description = exception.bstrDescription ? MakeString(exception.bstrDescription) : "Description unavailable"s;
   SysFreeString(exception.bstrDescription);
   SysFreeString(exception.bstrSource);
   SysFreeString(exception.bstrHelpFile);

   SCRIPTSTATE state;
   m_pScript->GetScriptState(&state);
   const bool isRuntimeError = (state == SCRIPTSTATE_CONNECTED);
   const bool isDebugConsole = (dwCookie == m_debugContextCookie);

   const ErrorType errorType = isDebugConsole ? ErrorType::DebugConsole : isRuntimeError ? ErrorType::Runtime : ErrorType::Compile;
   
   if (m_errorHandler)
      m_errorHandler(errorType, nLine + 1, nChar, description, stackDump);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// IActiveScriptSite interface

STDMETHODIMP ScriptInterpreter::GetLCID(LCID *plcid)
{
   //*plcid = 9; // Previous version would return 9 => What codepage is this ?
   *plcid = 1033; // English - United States
   return S_OK;
}

STDMETHODIMP ScriptInterpreter::GetItemInfo(LPCOLESTR pstrName, DWORD dwReturnMask, IUnknown **ppiunkItem, ITypeInfo **ppti)
{
   if (dwReturnMask & SCRIPTINFO_IUNKNOWN)
      *ppiunkItem = nullptr;
   if (dwReturnMask & SCRIPTINFO_ITYPEINFO)
      *ppti = nullptr;

   std::wstring wName(pstrName);
   auto it = m_vcvd.find(wName);
   if (it == m_vcvd.end())
      return E_FAIL;
   const std::unique_ptr<ScriptItem> &pcvd = it->second;

   if (dwReturnMask & SCRIPTINFO_IUNKNOWN)
   {
      if ((*ppiunkItem = pcvd->m_punk))
         (*ppiunkItem)->AddRef();
   }

   if (dwReturnMask & SCRIPTINFO_ITYPEINFO)
   {
      IProvideClassInfo *pClassInfo;
      pcvd->m_punk->QueryInterface(IID_IProvideClassInfo, (LPVOID *)&pClassInfo);
      if (pClassInfo)
      {
         pClassInfo->GetClassInfo(ppti);
         pClassInfo->Release();
      }
   }

   return S_OK;
}

STDMETHODIMP ScriptInterpreter::GetDocVersionString(BSTR *pbstrVersion)
{
   WCHAR *version = MakeWide(VP_VERSION_STRING_POINTS);
   *pbstrVersion = SysAllocString(version);
   delete[] version;
   return S_OK;
}

// Called on compilation errors. Also called on runtime errors in we couldn't create a "process debug manager" (such
// as when running on wine), or if no debug application is available (where a "debug application" is something like
// VS 2010 Isolated Shell).
// See ScriptInterpreter::OnScriptErrorDebug for runtime errors, when a debug application is available
STDMETHODIMP ScriptInterpreter::OnScriptError(IActiveScriptError *pScriptError)
{
   HandleScriptError(pScriptError, nullptr);
   return S_OK;
}

STDMETHODIMP ScriptInterpreter::OnScriptTerminate(const VARIANT *pvr, const EXCEPINFO *pei) { return S_OK; }

STDMETHODIMP ScriptInterpreter::OnStateChange(SCRIPTSTATE ssScriptState) { return S_OK; }

STDMETHODIMP ScriptInterpreter::OnEnterScript() { return S_OK; }

STDMETHODIMP ScriptInterpreter::OnLeaveScript() { return S_OK; }


///////////////////////////////////////////////////////////////////////////////////////////////////
// IActiveScriptSiteWindow interface

STDMETHODIMP ScriptInterpreter::GetWindow(HWND *phwnd)
{
   // We are supposed to return the window to be used as a parent for modal dialog. Why not just nullptr ?
#ifndef __STANDALONE__
   *phwnd = GetDesktopWindow();
#endif
   return S_OK;
}

STDMETHODIMP ScriptInterpreter::EnableModeless(BOOL) { return S_OK; }


///////////////////////////////////////////////////////////////////////////////////////////////////
// IActiveScriptSiteDebug interface

STDMETHODIMP ScriptInterpreter::GetDocumentContextFromPosition(DWORD_PTR dwSourceContext, ULONG uCharacterOffset, ULONG uNumChars, IDebugDocumentContext **ppsc) { return E_NOTIMPL; }

STDMETHODIMP ScriptInterpreter::GetApplication(IDebugApplication **ppda)
{
#ifndef __STANDALONE__
   if (m_pProcessDebugManager != nullptr)
   {
      IDebugApplication *app;
      const HRESULT result = m_pProcessDebugManager->GetDefaultApplication(&app);

      // We want to make sure the debug application supports JIT debugging, otherwise we don't seem to get notified
      // of runtime errors at all (neither in OnScriptError or in OnScriptErrorDebug)!
      if (SUCCEEDED(result) && app->FCanJitDebug())
      {
         *ppda = app;
         return S_OK;
      }
      else
         return E_FAIL;
   }
   else
      return E_NOTIMPL;
#else
   return S_OK;
#endif
}

STDMETHODIMP ScriptInterpreter::GetRootApplicationNode(IDebugApplicationNode **ppdanRoot)
{
   IDebugApplication *app;
   const HRESULT result = GetApplication(&app);
   if (SUCCEEDED(result))
      return app->GetRootNode(ppdanRoot);
   else
      return result;
}

// Called on runtime errors, if debugging is supported, and a debug application is available.
// See CodeViewer::OnScriptError for compilation errors, and also runtime errors when debugging isn't available.
STDMETHODIMP ScriptInterpreter::OnScriptErrorDebug(IActiveScriptErrorDebug *pScriptError, BOOL *pfEnterDebugger, BOOL *pfCallOnScriptErrorWhenContinuing)
{
   // TODO: Which debuggers even work with VBScript? It might be an idea to offer a "Debug" button (set pfEnterDebugger to
   //       true) if it can pop open some old version of visual studio to debug stuff.
   //
   //       VS 2010 Isolated Shell seems to work, but trying to enter debugging with it complains with an "invalid
   //       license" error. I haven't found anything else to work yet, not even regular VS 2010 (though, it might be
   //       that you need to manually set some registry keys to select the default debugger?)
   //
   //       HKEY_CLASSES_ROOT\CLSID\{834128A2-51F4-11D0-8F20-00805F2CD064}\LocalServer32 seems to be the registry key
   //       to select the default debugger.
   //       (https://stackoverflow.com/questions/2288043/how-do-i-debug-a-stand-alone-vbscript-script#comment36315883_2288064)
   *pfEnterDebugger = FALSE;
   *pfCallOnScriptErrorWhenContinuing = FALSE;

   HandleScriptError(reinterpret_cast<IActiveScriptError*>(pScriptError), pScriptError);

   return S_OK;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Internet Security interface


HRESULT STDMETHODCALLTYPE ScriptInterpreter::GetSecurityId(BYTE *pbSecurityId, DWORD *pcbSecurityId, DWORD_PTR dwReserved) { return S_OK; }

HRESULT STDMETHODCALLTYPE ScriptInterpreter::ProcessUrlAction(
   DWORD dwAction, BYTE __RPC_FAR *pPolicy, DWORD cbPolicy, BYTE __RPC_FAR *pContext, DWORD cbContext, DWORD dwFlags, DWORD dwReserved)
{
   *pPolicy = (dwAction == URLACTION_ACTIVEX_RUN && (g_app->m_securitylevel < eSecurityNoControls)) ? URLPOLICY_ALLOW : URLPOLICY_DISALLOW;
   return S_OK;
}

DEFINE_GUID(GUID_CUSTOM_CONFIRMOBJECTSAFETY, 0x10200490, 0xfa38, 0x11d0, 0xac, 0x0e, 0x00, 0xa0, 0xc9, 0x0f, 0xff, 0xc0);

HRESULT STDMETHODCALLTYPE ScriptInterpreter::QueryCustomPolicy(
   REFGUID guidKey, BYTE __RPC_FAR *__RPC_FAR *ppPolicy, DWORD __RPC_FAR *pcbPolicy, BYTE __RPC_FAR *pContext, DWORD cbContext, DWORD dwReserved)
{
#ifndef __STANDALONE__
   uint32_t *const ppolicy = (uint32_t *)CoTaskMemAlloc(sizeof(uint32_t)); // needs to use CoTaskMemAlloc because of COM model
   *ppolicy = URLPOLICY_DISALLOW;

   *ppPolicy = (BYTE *)ppolicy;

   *pcbPolicy = sizeof(DWORD);

   if (InlineIsEqualGUID(guidKey, GUID_CUSTOM_CONFIRMOBJECTSAFETY))
   {
      bool safe = false;
      CONFIRMSAFETY *pcs = (CONFIRMSAFETY *)pContext;

      if (g_app->m_securitylevel == eSecurityNone)
         safe = true;

      if (!safe && ((g_app->m_securitylevel == eSecurityWarnOnUnsafeType) || (g_app->m_securitylevel == eSecurityWarnOnType)))
         safe = IsControlAlreadyOkayed(pcs);

      if (!safe && (g_app->m_securitylevel <= eSecurityWarnOnUnsafeType))
         safe = IsControlMarkedSafe(pcs);

      if (!safe)
      {
         safe = IsUserManuallyOkaysControl(pcs);
         if (safe && ((g_app->m_securitylevel == eSecurityWarnOnUnsafeType) || (g_app->m_securitylevel == eSecurityWarnOnType)))
            AddControlToOkayedList(pcs);
      }

      if (safe)
         *ppolicy = URLPOLICY_ALLOW;
   }
#endif

   return S_OK;
}

bool ScriptInterpreter::IsControlAlreadyOkayed(const CONFIRMSAFETY *pcs) const
{
   if (g_pplayer)
   {
      for (size_t i = 0; i < g_pplayer->m_controlclsidsafe.size(); ++i)
      {
         const CLSID *const pclsid = g_pplayer->m_controlclsidsafe[i];
         if (*pclsid == pcs->clsid)
            return true;
      }
   }

   return false;
}

void ScriptInterpreter::AddControlToOkayedList(const CONFIRMSAFETY *pcs) const
{
   if (g_pplayer)
   {
      CLSID *const pclsid = new CLSID();
      *pclsid = pcs->clsid;
      g_pplayer->m_controlclsidsafe.push_back(pclsid);
   }
}

bool ScriptInterpreter::IsControlMarkedSafe(const CONFIRMSAFETY *pcs)
{
   bool safe = false;
#ifndef __STANDALONE__
   IObjectSafety *pios = nullptr;

   DWORD supported, enabled;
   if (SUCCEEDED(pcs->pUnk->QueryInterface(IID_IObjectSafety, (void **)&pios)) && SUCCEEDED(pios->GetInterfaceSafetyOptions(IID_IDispatch, &supported, &enabled))
      && (supported & INTERFACESAFE_FOR_UNTRUSTED_CALLER) && (supported & INTERFACESAFE_FOR_UNTRUSTED_DATA))
   {
      // either it is already enabled, or we could enable it
      if (((enabled & INTERFACESAFE_FOR_UNTRUSTED_CALLER) && (enabled & INTERFACESAFE_FOR_UNTRUSTED_DATA))
         || SUCCEEDED(pios->SetInterfaceSafetyOptions(IID_IDispatch, supported, INTERFACESAFE_FOR_UNTRUSTED_CALLER | INTERFACESAFE_FOR_UNTRUSTED_DATA)))
         safe = true;
   }

   if (pios)
      pios->Release();
#endif

   return safe;
}

bool ScriptInterpreter::IsUserManuallyOkaysControl(const CONFIRMSAFETY *pcs) const
{
#ifndef __STANDALONE__
   OLECHAR *wzT;
   if (FAILED(OleRegGetUserType(pcs->clsid, USERCLASSTYPE_FULL, &wzT)))
      return false;
   HWND parent = nullptr;
   if (parent == nullptr && g_pplayer)
      parent = g_pplayer->m_playfieldWnd->GetNativeHWND();
   if (parent == nullptr && g_pvp)
      parent = g_pvp->GetHwnd();
   const int ans = MessageBox(
      parent, (LocalString(IDS_UNSECURECONTROL1).m_szbuffer + MakeString(wzT) + LocalString(IDS_UNSECURECONTROL2).m_szbuffer).c_str(), "Visual Pinball", MB_YESNO | MB_DEFBUTTON2);
   return (ans == IDYES);
#else
   return false;
#endif
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// IServiceProvider interface

HRESULT STDMETHODCALLTYPE ScriptInterpreter::QueryService(REFGUID guidService, REFIID riid, void **ppv)
{
   return (riid == IID_IInternetHostSecurityManager) ? QueryInterface(riid /*IID_IInternetHostSecurityManager*/, ppv) : E_NOINTERFACE;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Debug script object

STDMETHODIMP ScriptInterpreter::DebuggerModule::Print(VARIANT *pvar)
{
   // Disable logging in locked tables (there is no debugger in locked mode anyway)
   if (g_pplayer->m_ptable->IsLocked())
      return S_OK;

   if (!g_app->m_settings.GetEditor_EnableLog() || !g_app->m_settings.GetEditor_LogScriptOutput())
      return S_OK;

   if (V_VT(pvar) == VT_EMPTY || V_VT(pvar) == VT_NULL || V_VT(pvar) == VT_ERROR)
   {
      PLOGI << "Script.Print ''";
      return S_OK;
   }

   CComVariant varT;
   const HRESULT hr = VariantChangeType(&varT, pvar, 0, VT_BSTR);

   if (FAILED(hr))
   {
      const LocalString ls(IDS_DEBUGNOCONVERT);
      PLOGI << "Script.Print '" << ls.m_szbuffer << '\'';
      return S_OK;
   }

   const string szT = MakeString(V_BSTR(&varT));
   PLOGI << "Script.Print '" << szT << '\'';

   return S_OK;
}
