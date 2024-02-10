#pragma once

#include <commdlg.h>
#include <activscp.h>
#include <activdbg.h>
#include <atlcom.h>
#include "codeviewedit.h"
#ifndef __STANDALONE__
#include "ScriptErrorDialog.h"
#include "scintilla.h"
#endif

#define MAX_FIND_LENGTH 81
#define MAX_LINE_LENGTH 2048

enum SecurityLevelEnum
{
   eSecurityNone = 0,
   eSecurityWarnOnUnsafeType = 1,
   eSecurityWarnOnType = 2,
   eSecurityWarnOnAll = 3,
   eSecurityNoControls = 4
};

class IScriptable
{
public:
   IScriptable();

   STDMETHOD(get_Name)(BSTR *pVal) = 0;
   virtual IDispatch *GetDispatch() = 0;
   virtual const IDispatch *GetDispatch() const = 0;
   virtual ISelect *GetISelect() = 0;
   virtual const ISelect *GetISelect() const = 0;

   WCHAR m_wzName[MAXNAMEBUFFER];
};

class CodeViewer;

#ifdef __STANDALONE__
class IProcessDebugManager { };
#endif

class DebuggerModule :
   public CComObjectRootEx<CComSingleThreadModel>,
   public IDispatchImpl<IVPDebug, &IID_IVPDebug, &LIBID_VPinballLib>,
   public IScriptable
{
#ifdef __STANDALONE__
public:
   STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid,DISPID* rgDispId);
   STDMETHOD(Invoke)(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);
   STDMETHOD(GetDocumentation)(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile);
#endif
   BEGIN_COM_MAP(DebuggerModule)
      COM_INTERFACE_ENTRY(IVPDebug)
      COM_INTERFACE_ENTRY(IDispatch)
   END_COM_MAP()

   STDMETHOD(Print)(VARIANT *pvar);

public:
   void Init(CodeViewer * const pcv);

   IDispatch *GetDispatch() final { return (IDispatch *)this; }
   const IDispatch *GetDispatch() const final { return (const IDispatch *)this; }

   ISelect *GetISelect() final { return nullptr; }
   const ISelect *GetISelect() const final { return nullptr; }

   STDMETHOD(get_Name)(BSTR *pVal);

   CodeViewer *m_pcv;
};

class IScriptableHost
{
public:
   virtual void SelectItem(IScriptable *piscript) = 0;
   virtual void SetDirtyScript(SaveDirtyState sds) = 0;
   virtual void DoCodeViewCommand(int command) = 0;
};

class CodeViewDispatch
{
public:
   CodeViewDispatch() {}
   ~CodeViewDispatch() {}

   wstring m_wName;
   IUnknown *m_punk;
   IDispatch *m_pdisp;
   IScriptable *m_piscript;
   bool m_global;

   // for VectorSortString
   int SortAgainst(const CodeViewDispatch * const pcvd/*void *pvoid*/) const { return SortAgainstValue(pcvd->m_wName); }
   int SortAgainstValue(const wstring &pv) const;
};

class CodeViewer :
	public CWnd,
	public CComObjectRoot,
	//public IDispatchImpl<IDragPoint, &IID_IDragPoint, &LIBID_VPinballLib>,
	//public CComCoClass<CodeViewer,&CLSID_DragPoint>,
	//public CComObjectRootEx<CComSingleThreadModel>,
	public IActiveScriptSite,
	public IActiveScriptSiteDebug,
	public IActiveScriptSiteWindow,
	public IInternetHostSecurityManager,
	public IServiceProvider
{
public:
   CodeViewer() : m_haccel(nullptr), m_pProcessDebugManager(nullptr), m_parentLevel(0), m_lastErrorWidgetVisible(false), m_suppressErrorDialogs(false) {}
   virtual ~CodeViewer();

   void Init(IScriptableHost *psh);
   void SetVisible(const bool visible);

   void SetEnabled(const bool enabled);

   void SetClean(const SaveDirtyState sds);

   // Script Class
   STDMETHOD(CleanUpScriptEngine)();
   STDMETHOD(InitializeScriptEngine)();

   HRESULT AddItem(IScriptable * const piscript, const bool global);
   void RemoveItem(IScriptable * const piscript);
   HRESULT ReplaceName(IScriptable * const piscript, const WCHAR * const wzNew);
   void SelectItem(IScriptable * const piscript);

   void Compile(const bool message);
   void Start();

   void EndSession();

   HRESULT AddTemporaryItem(const BSTR bstr, IDispatch * const pdisp);

   STDMETHOD(GetItemInfo)(LPCOLESTR pstrName, DWORD dwReturnMask,
      IUnknown **ppiunkItem, ITypeInfo **ppti);

   STDMETHOD(OnScriptError)(IActiveScriptError *pscripterror);

   STDMETHOD(GetLCID)(LCID *plcid) override
   {
      *plcid = 9; return S_OK;
   }

   STDMETHOD(GetDocVersionString)(BSTR *pbstrVersion) override
   {
      *pbstrVersion = SysAllocString(L""); return S_OK;
   }

   STDMETHOD(OnScriptTerminate)(const VARIANT *pvr, const EXCEPINFO *pei) override
   {
      return S_OK;
   }

   STDMETHOD(OnStateChange)(SCRIPTSTATE ssScriptState) override
   {
      return S_OK;
   }

   STDMETHOD(OnEnterScript)();

   STDMETHODIMP OnLeaveScript();

   STDMETHODIMP GetWindow(HWND *phwnd) override
   {
#ifndef __STANDALONE__
      *phwnd = GetDesktopWindow(); return S_OK; //!! ?
#else
      return S_OK;
#endif
   }

   STDMETHODIMP EnableModeless(BOOL) override
   {
      return S_OK;
   }

   // IActiveScriptSiteDebug interface

   STDMETHOD(GetDocumentContextFromPosition)(
       DWORD_PTR dwSourceContext,
       ULONG uCharacterOffset,
       ULONG uNumChars,
       IDebugDocumentContext** ppsc
       );

   STDMETHOD(GetApplication)(
       IDebugApplication** ppda
       );

   STDMETHOD(GetRootApplicationNode)(
       IDebugApplicationNode** ppdanRoot
       );

   STDMETHOD(OnScriptErrorDebug)(
       IActiveScriptErrorDebug* pscripterror,
       BOOL* pfEnterDebugger,
       BOOL* pfCallOnScriptErrorWhenContinuing
       );

   // Internet Security interface

   virtual HRESULT STDMETHODCALLTYPE GetSecurityId(
      /* [size_is][out] */ BYTE *pbSecurityId,
      /* [out][in] */ DWORD *pcbSecurityId,
      /* [in] */ DWORD_PTR dwReserved);

   virtual HRESULT STDMETHODCALLTYPE ProcessUrlAction(
      /* [in] */ DWORD dwAction,
      /* [size_is][out] */ BYTE __RPC_FAR *pPolicy,
      /* [in] */ DWORD cbPolicy,
      /* [in] */ BYTE __RPC_FAR *pContext,
      /* [in] */ DWORD cbContext,
      /* [in] */ DWORD dwFlags,
      /* [in] */ DWORD dwReserved);

   virtual HRESULT STDMETHODCALLTYPE QueryCustomPolicy(
      /* [in] */ REFGUID guidKey,
      /* [size_is][size_is][out] */ BYTE __RPC_FAR *__RPC_FAR *ppPolicy,
      /* [out] */ DWORD __RPC_FAR *pcbPolicy,
      /* [in] */ BYTE __RPC_FAR *pContext,
      /* [in] */ DWORD cbContext,
      /* [in] */ DWORD dwReserved);

   bool FControlAlreadyOkayed(const CONFIRMSAFETY *pcs);
   void AddControlToOkayedList(const CONFIRMSAFETY *pcs);
   bool FControlMarkedSafe(const CONFIRMSAFETY *pcs);
   bool FUserManuallyOkaysControl(const CONFIRMSAFETY *pcs);

   virtual HRESULT STDMETHODCALLTYPE QueryService(
      REFGUID guidService,
      REFIID riid,
      void **ppv);

   // Use CComObject to implement AddRef/Release/QI
   BEGIN_COM_MAP(CodeViewer)
      //COM_INTERFACE_ENTRY(IDispatch)
      COM_INTERFACE_ENTRY(IActiveScriptSite)
      COM_INTERFACE_ENTRY(IActiveScriptSiteDebug)
      COM_INTERFACE_ENTRY(IActiveScriptSiteWindow)
      COM_INTERFACE_ENTRY(IInternetHostSecurityManager)
      COM_INTERFACE_ENTRY(IServiceProvider)
   END_COM_MAP()

   void UncolorError();
   void ParseForFunction();

   void ShowFindDialog();
   void ShowFindReplaceDialog();
   void Find(const FINDREPLACE * const pfr);
   void Replace(const FINDREPLACE * const pfr);
   void SaveToStream(IStream *pistream, HCRYPTHASH const hcrypthash);
   void SaveToFile(const string& filename);
   void LoadFromStream(IStream *pistream, HCRYPTHASH const hcrypthash, const HCRYPTKEY hcryptkey); // incl. table protection
   void LoadFromFile(const string& filename);
   void SetCaption(const string& szCaption);

   bool ShowTooltipOrGoToDefinition(const SCNotification *pSCN, const bool tooltip);
   void ShowAutoComplete(const SCNotification *pSCN);

   void UpdateRegWithPrefs();
   void UpdatePrefsfromReg();

   void GetWordUnderCaret();

   void ListEventsFromItem();
   void FindCodeFromEvent();
   void TellHostToSelectItem();

   void UpdateScinFromPrefs();

#ifndef __STANDALONE__
   void MarginClick(const Sci_Position position, const int modifiers);
#endif

   void EvaluateScriptStatement(const char * const szScript);
   void AddToDebugOutput(const char * const szText);

   bool PreTranslateMessage(MSG* msg);

   IScriptableHost *m_psh;

   IActiveScript* m_pScript;

   VectorSortString<CodeViewDispatch*> m_vcvd;

   COLORREF m_prefCols[16];
   COLORREF m_bgColor;
   COLORREF m_bgSelColor;
   CVPreference *m_prefEverythingElse;
   vector<CVPreference*> *m_lPrefsList;

   int m_displayAutoCompleteLength;

   SaveDirtyState m_sdsDirty;
   bool m_ignoreDirty;

   bool m_scriptError; // Whether a script error has occured - used for polling from the game

   bool m_visible;
   bool m_minimized;

   bool m_displayAutoComplete;
   bool m_toolTipActive;
   bool m_stopErrorDisplay;

   bool m_dwellHelp;
   bool m_dwellDisplay;
   int m_dwellDisplayTime;

   fi_vector<UserData> m_pageConstructsDict;
#ifndef __STANDALONE__
   Sci_TextRange m_wordUnderCaret;
#endif

   CComObject<DebuggerModule> *m_pdm; // Object to expose to script for global functions
   //ULONG m_cref;

   HWND m_hwndMain;
   HWND m_hwndScintilla;
   HWND m_hwndFind;
   HWND m_hwndStatus;
   HWND m_hwndFunctionList;

   HACCEL m_haccel; // Accelerator keys

   int m_errorLineNumber;

   FINDREPLACE m_findreplaceold; // the last thing found/replaced

   string external_script_name;  // loaded from external .vbs?
   vector<char> original_table_script; // if yes, then this one stores the original table script

   string m_script_text;

protected:
   void PreCreate(CREATESTRUCT& cs) final;
   void PreRegisterClass(WNDCLASS& wc) final;
   int  OnCreate(CREATESTRUCT& cs) final;
   LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam) final;
   BOOL OnCommand(WPARAM wparam, LPARAM lparam) final;
   LRESULT OnNotify(WPARAM wparam, LPARAM lparam) final;
   void Destroy() final;

private:
   CodeViewer* GetCodeViewerPtr();
   BOOL ParseClickEvents(const int id, const SCNotification *pSCN);
   BOOL ParseSelChangeEvent(const int id, const SCNotification *pSCN);

   bool ParseOKLineLength(const size_t LineLen);
   string ParseDelimtByColon(string &wholeline);
   void ParseFindConstruct(size_t &Pos, const string &UCLine, WordType &Type, int &ConstructSize);
   bool ParseStructureName(fi_vector<UserData> &ListIn, const UserData &ud, const string &UCline, const string &line, const int Lineno);

   size_t SureFind(const string &LineIn, const string &ToFind);
   void RemoveByVal(string &line); 
   void RemoveNonVBSChars(string &line);
   string ExtractWordOperand(const string &line, const size_t StartPos);

   void ColorLine(const int line);
   void ColorError(const int line, const int nchar);

   void ParseVPCore();

   void ReadLineToParseBrain(string wholeline, const int linecount, fi_vector<UserData> &ListIn);

   void GetMembers(const fi_vector<UserData> &ListIn, const string &StrIn);

   void InitPreferences();

   void GetParamsFromEvent(const UINT iEvent, char * const szParams);

   /**
    * Resizes the Scintilla widget (the text editor) and the last error widget (if it's visible)
    * 
    * This is called when the window is resized (when we get a WM_SIZE message)
    * or when the last error widget is toggled (since that appears below the text editor)
    */
   void ResizeScintillaAndLastError();

   /**
    * Sets the visibility of the last error information, shown below the Scintilla text editor.
    */
   void SetLastErrorVisibility(bool show);
   void SetLastErrorTextW(const LPCWSTR text);
   void AppendLastErrorTextW(const wstring& text);

   IActiveScriptParse* m_pScriptParse;
   IActiveScriptDebug* m_pScriptDebug;

   /**
    * Will be nullptr on systems that don't support debugging.
    * 
    * For example, wine 6.9 says ...
    * > no class object {78a51822-51f4-11d0-8f20-00805f2cd064} could be created for context 0x17
    * ... if I try to create CLSID_PrrocessDebugManager
    */
   IProcessDebugManager* m_pProcessDebugManager;

   FINDREPLACE m_findreplacestruct;
   char szFindString[MAX_FIND_LENGTH];
   char szReplaceString[MAX_FIND_LENGTH];

   VectorSortString<CodeViewDispatch*> m_vcvdTemp; // Objects added through script

   string m_validChars;

   // CodeViewer Preferences
   CVPreference *prefDefault;
   CVPreference *prefVBS;
   CVPreference *prefComps;
   CVPreference *prefSubs;
   CVPreference *prefComments;
   CVPreference *prefLiterals;
   CVPreference *prefVPcore;

   int m_parentLevel;
   string m_currentParentKey; // always lower case
   //bool m_parentTreeInvalid;
   //TODO: int TabStop;

   // Dictionaries
   fi_vector<UserData> m_VBwordsDict;
   fi_vector<UserData> m_componentsDict;
   fi_vector<UserData> m_VPcoreDict;
   fi_vector<UserData> m_currentMembers;
   string m_autoCompString;
   string m_autoCompMembersString;
#ifndef __STANDALONE__
   Sci_TextRange m_currentConstruct;
#endif

   HWND m_hwndItemList;
   HWND m_hwndItemText;
   HWND m_hwndEventList;
   HWND m_hwndEventText;
   HWND m_hwndFunctionText;

   /**
    * Whether the last error widget is visible
    */
   bool m_lastErrorWidgetVisible;

   /**
    * If true, error dialogs will be suppressed for the play session
    * 
    * This gets reset to false whenever the script is started
    */
   bool m_suppressErrorDialogs;

   /**
    * Handle for the last error widget
    * 
    * The last error widget is a read-only text area that appears below the Scintilla text editor, with the contents of
    * the last reported compile or runtime error.
    */
   HWND m_hwndLastErrorTextArea;
};

class Collection :
   public IDispatchImpl<ICollection, &IID_ICollection, &LIBID_VPinballLib>,
   public CComObjectRoot,
   public CComCoClass<Collection, &CLSID_Collection>,
   public EventProxy<Collection, &DIID_ICollectionEvents>,
   public IConnectionPointContainerImpl<Collection>,
   public IProvideClassInfo2Impl<&CLSID_Collection, &DIID_ICollectionEvents, &LIBID_VPinballLib>,
   public IScriptable,
   public ILoadable
{
#ifdef __STANDALONE__
public:
   STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid,DISPID* rgDispId);
   STDMETHOD(Invoke)(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);
   STDMETHOD(GetDocumentation)(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile);
   virtual HRESULT FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) override;
#endif
public:
   Collection();

   // IScriptable
   STDMETHOD(get_Name)(BSTR *pVal);
   IDispatch *GetDispatch() final { return (IDispatch *)this; }
   const IDispatch *GetDispatch() const final { return (const IDispatch *)this; }

   ISelect *GetISelect() final { return nullptr; }
   const ISelect *GetISelect() const final { return nullptr; }

   //ILoadable
   HRESULT SaveData(IStream *pstm, HCRYPTHASH hcrypthash, const bool saveForUndo);
   HRESULT LoadData(IStream *pstm, PinTable *ppt, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey);
   bool LoadToken(const int id, BiffReader * const pbr) final;

   STDMETHOD(get_Count)(long __RPC_FAR *plCount);
   STDMETHOD(get_Item)(long index, IDispatch __RPC_FAR * __RPC_FAR *ppidisp);
   STDMETHOD(get__NewEnum)(IUnknown** ppunk);

   BEGIN_COM_MAP(Collection)
      COM_INTERFACE_ENTRY(IDispatch)
      COM_INTERFACE_ENTRY(ICollection)
      COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
      COM_INTERFACE_ENTRY(IProvideClassInfo)
      COM_INTERFACE_ENTRY(IProvideClassInfo2)
   END_COM_MAP()

   BEGIN_CONNECTION_POINT_MAP(Collection)
      CONNECTION_POINT_ENTRY(DIID_ICollectionEvents)
   END_CONNECTION_POINT_MAP()

   VectorProtected<ISelect> m_visel;

   bool m_fireEvents;
   bool m_stopSingleEvents;
   bool m_groupElements;
};

class OMCollectionEnum :
   public CComObjectRootEx<CComSingleThreadModel>,
   public IEnumVARIANT
{
public:
   BEGIN_COM_MAP(OMCollectionEnum)
      COM_INTERFACE_ENTRY(IEnumVARIANT)
   END_COM_MAP()

   OMCollectionEnum() {}
   ~OMCollectionEnum() {}

   STDMETHOD(Init)(Collection *pcol);

   STDMETHOD(Next)(ULONG celt, VARIANT __RPC_FAR *rgVar, ULONG __RPC_FAR *pCeltFetched);
   STDMETHOD(Skip)(ULONG celt);
   STDMETHOD(Reset)();
   STDMETHOD(Clone)(IEnumVARIANT __RPC_FAR *__RPC_FAR *ppEnum);

private:
   Collection *m_pcol;
   int m_index;
};

// general string helpers:

constexpr __forceinline bool IsWhitespace(const char ch)
{
   return (ch == ' ' || ch == 9/*tab*/);
}

__forceinline string upperCase(string input)
{
   std::transform(input.begin(), input.end(), input.begin(), ::toupper);
   return input;
}

__forceinline string lowerCase(string input)
{
   std::transform(input.begin(), input.end(), input.begin(), ::tolower);
   return input;
}

inline void RemovePadding(string &line)
{
    const size_t LL = line.length();
    size_t Pos = line.find_first_not_of("\n\r\t ,");
    if (Pos == string::npos)
    {
        line.clear();
        return;
    }

    if (Pos > 0)
    {
        if ((SSIZE_T)(LL - Pos) < 1) return;
        line = line.substr(Pos, (LL - Pos));
    }

    Pos = line.find_last_not_of("\n\r\t ,");
    if (Pos != string::npos)
    {
        if (Pos < 1) return;
        line = line.erase(Pos + 1);
    }
}

inline string ParseRemoveVBSLineComments(string &Line)
{
    const size_t commentIdx = Line.find('\'');
    if (commentIdx == string::npos) return string();
    string RetVal = Line.substr(commentIdx + 1, string::npos);
    RemovePadding(RetVal);
    if (commentIdx > 0)
        Line = Line.substr(0, commentIdx);
    else
        Line.clear();
    return RetVal;
}

inline void szLower(char * pC)
{
    while (*pC)
    {
        if (*pC >= 'A' && *pC <= 'Z')
            *pC += ('a' - 'A');
        pC++;
    }
}

inline void szUpper(char * pC)
{
    while (*pC)
    {
        if (*pC >= 'a' && *pC <= 'z')
            *pC -= ('a' - 'A');
        pC++;
    }
}
