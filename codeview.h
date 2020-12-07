#pragma once

#include <Commdlg.h>
#include <activscp.h>
#include <activdbg.h>
#include <atlcom.h>
#include "codeviewedit.h"
#include "scintilla.h"
#include "scilexer.h"

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

class DebuggerModule :
   public CComObjectRootEx<CComSingleThreadModel>,
   public IDispatchImpl<IVPDebug, &IID_IVPDebug, &LIBID_VPinballLib>,
   public IScriptable
{
   BEGIN_COM_MAP(DebuggerModule)
      COM_INTERFACE_ENTRY(IVPDebug)
      COM_INTERFACE_ENTRY(IDispatch)
   END_COM_MAP()

   STDMETHOD(Print)(VARIANT *pvar);

   void Init(CodeViewer * const pcv);

   virtual IDispatch *GetDispatch() { return (IDispatch *)this; }
   virtual const IDispatch *GetDispatch() const { return (const IDispatch *)this; }

   virtual ISelect *GetISelect() { return NULL; }
   virtual const ISelect *GetISelect() const { return NULL; }

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
   CodeViewDispatch();
   ~CodeViewDispatch();

   WCHAR m_wzName[MAXNAMEBUFFER];
   IUnknown *m_punk;
   IDispatch *m_pdisp;
   IScriptable *m_piscript;
   bool m_global;

   // for VectorSortString
   int SortAgainst(const CodeViewDispatch * const pcvd/*void *pvoid*/) const;
   int SortAgainstValue(const WCHAR* const pv) const;
};



class CodeViewer :
	public CWnd,
	public CComObjectRoot,
	//public IDispatchImpl<IDragPoint, &IID_IDragPoint, &LIBID_VPinballLib>,
	//public CComCoClass<CodeViewer,&CLSID_DragPoint>,
	//public CComObjectRootEx<CComSingleThreadModel>,
	public IActiveScriptSite,
	public IActiveScriptSiteWindow,
	public IInternetHostSecurityManager,
	public IServiceProvider,
	public UserData,
	public CVPrefrence
{
public:
   CodeViewer();
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
   HRESULT ReplaceName(IScriptable * const piscript, WCHAR * const wzNew);
   void SelectItem(IScriptable * const piscript);

   void Compile(const bool message);
   void Start();

   void EndSession();

   HRESULT AddTemporaryItem(const BSTR bstr, IDispatch * const pdisp);

   STDMETHOD(GetItemInfo)(LPCOLESTR pstrName, DWORD dwReturnMask,
      IUnknown **ppiunkItem, ITypeInfo **ppti);

   STDMETHOD(OnScriptError)(IActiveScriptError *pscripterror);

   STDMETHOD(GetLCID)(LCID *plcid)
   {
      *plcid = 9; return S_OK;
   }

   STDMETHOD(GetDocVersionString)(BSTR *pbstrVersion)
   {
      *pbstrVersion = SysAllocString(L""); return S_OK;
   }

   STDMETHOD(OnScriptTerminate)(const VARIANT *pvr, const EXCEPINFO *pei)
   {
      return S_OK;
   }

   STDMETHOD(OnStateChange)(SCRIPTSTATE ssScriptState)
   {
      return S_OK;
   }

   STDMETHOD(OnEnterScript)();

   STDMETHODIMP OnLeaveScript();

   STDMETHODIMP GetWindow(HWND *phwnd)
   {
      *phwnd = GetDesktopWindow(); return S_OK; //!! ?
   }

   STDMETHODIMP EnableModeless(BOOL)
   {
      return S_OK;
   }

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

   bool FControlAlreadyOkayed(CONFIRMSAFETY *pcs);
   void AddControlToOkayedList(CONFIRMSAFETY *pcs);
   bool FControlMarkedSafe(CONFIRMSAFETY *pcs);
   bool FUserManuallyOkaysControl(CONFIRMSAFETY *pcs);

   virtual HRESULT STDMETHODCALLTYPE QueryService(
      REFGUID guidService,
      REFIID riid,
      void **ppv);

   // Use CComObject to implement AddRef/Release/QI
   BEGIN_COM_MAP(CodeViewer)
      //COM_INTERFACE_ENTRY(IDispatch)
      COM_INTERFACE_ENTRY(IActiveScriptSite)
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

   bool ShowTooltip(SCNotification *Scn);
   void ShowAutoComplete(SCNotification *pSCN);

   void UpdateRegWithPrefs();
   void UpdatePrefsfromReg();

   void GetWordUnderCaret();

   void ListEventsFromItem();
   void FindCodeFromEvent();
   void TellHostToSelectItem();

   void UpdateScinFromPrefs();

   void MarginClick(const Sci_Position position, const int modifiers);

   void EvaluateScriptStatement(const char * const szScript);
   void AddToDebugOutput(const char * const szText);

   bool PreTranslateMessage(MSG* msg);

   IScriptableHost *m_psh;

   IActiveScript* m_pScript;

   VectorSortString<CodeViewDispatch*> m_vcvd;

   COLORREF m_prefCols[16];
   COLORREF m_bgColor;
   CVPrefrence *m_prefEverythingElse;
   vector<CVPrefrence*> *m_lPrefsList;

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

   vector<UserData> *m_pageConstructsDict;
   Sci_TextRange m_wordUnderCaret;

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

protected:
    virtual void PreCreate(CREATESTRUCT& cs);
    virtual void PreRegisterClass(WNDCLASS& wc);
    virtual int  OnCreate(CREATESTRUCT& cs);
    virtual LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual BOOL OnCommand(WPARAM wparam, LPARAM lparam);
    virtual LRESULT OnNotify(WPARAM wparam, LPARAM lparam);
    virtual void Destroy();

private:
   CodeViewer* GetCodeViewerPtr();
   BOOL ParseClickEvents(const int id);
   BOOL ParseSelChangeEvent(const int id, SCNotification *pscn);

   bool ParseOKLineLength(const size_t LineLen);
   void ParseDelimtByColon(string &result, string &wholeline);
   void ParseFindConstruct(size_t &Pos, const string &UCLine, WordType &Type, int &ConstructSize);
   bool ParseStructureName(vector<UserData> *ListIn, UserData ud, const string &UCline, const string &line, const int Lineno);
   
   size_t SureFind(const string &LineIn, const string &ToFind);
   void RemoveByVal(string &line); 
   void RemoveNonVBSChars(string &line);
   string ExtractWordOperand(const string &line, const size_t StartPos);

   void ColorLine(const int line);
   void ColorError(const int line, const int nchar);

   void ParseVPCore();
   
   void ReadLineToParseBrain(string wholeline, const int linecount, vector<UserData> *ListIn);

   void GetMembers(vector<UserData>* ListIn, const string &StrIn);

   void InitPreferences();

   void GetParamsFromEvent(const UINT iEvent, char * const szParams);

   IActiveScriptParse* m_pScriptParse;
   IActiveScriptDebug* m_pScriptDebug;
   FINDREPLACE m_findreplacestruct;
   char szFindString[MAX_FIND_LENGTH];
   char szReplaceString[MAX_FIND_LENGTH];

   VectorSortString<CodeViewDispatch*> m_vcvdTemp; // Objects added through script

   string m_validChars;
   string m_VBvalidChars;

   // CodeViewer Preferences
   CVPrefrence *prefDefault;
   CVPrefrence *prefVBS;
   CVPrefrence *prefComps;
   CVPrefrence *prefSubs;
   CVPrefrence *prefComments;
   CVPrefrence *prefLiterals;
   CVPrefrence *prefVPcore;
   //bool ParentTreeInvalid;
   //TODO: int TabStop;

   // keyword lists
   string m_vbsKeyWords;
   vector<string> *m_autoCompList;
   // Dictionaries
   vector<UserData> *m_VBwordsDict;
   vector<UserData> *m_componentsDict;
   vector<UserData> *m_VPcoreDict;
   vector<UserData> *m_currentMembers;
   string m_autoCompString;
   string m_autoCompMembersString;
   Sci_TextRange m_currentConstruct;

   HWND m_hwndItemList;
   HWND m_hwndItemText;
   HWND m_hwndEventList;
   HWND m_hwndEventText;
   HWND m_hwndFunctionText;
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
public:
   Collection();

   // IScriptable
   STDMETHOD(get_Name)(BSTR *pVal);
   virtual IDispatch *GetDispatch() { return (IDispatch *)this; }
   virtual const IDispatch *GetDispatch() const { return (const IDispatch *)this; }
   virtual ISelect *GetISelect() { return NULL; }
   virtual const ISelect *GetISelect() const { return NULL; }

   //ILoadable
   virtual HRESULT SaveData(IStream *pstm, HCRYPTHASH hcrypthash, const bool backupForPlay);
   virtual HRESULT LoadData(IStream *pstm, PinTable *ppt, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey);
   virtual bool LoadToken(const int id, BiffReader * const pbr);

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

   OMCollectionEnum();
   ~OMCollectionEnum();

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

inline bool IsWhitespace(const char ch)
{
   return (ch == ' ' || ch == 9/*tab*/);
}

inline string upperCase(string input)
{
   for (string::iterator it = input.begin(); it != input.end(); ++it)
      *it = toupper(*it);
   return input;
}

inline string lowerCase(string input)
{
   for (string::iterator it = input.begin(); it != input.end(); ++it)
      *it = tolower(*it);
   return input;
}

inline void RemovePadding(string &line)
{
    const size_t LL = line.length();
    size_t Pos = (line.find_first_not_of("\n\r\t ,"));
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

    Pos = (line.find_last_not_of("\n\r\t ,"));
    if (Pos != string::npos)
    {
        if (Pos < 1) return;
        line = line.erase(Pos + 1);
    }
}

inline string ParseRemoveVBSLineComments(string &Line)
{
    const size_t commentIdx = Line.find("'");
    if (commentIdx == string::npos) return "";
    string RetVal = Line.substr(commentIdx + 1, string::npos);
    RemovePadding(RetVal);
    if (commentIdx > 0)
    {
        Line = Line.substr(0, commentIdx);
        return RetVal;
    }
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
