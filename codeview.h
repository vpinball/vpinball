#pragma once
#include <activscp.h>

#include "atlcom.h"

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
	virtual ISelect *GetISelect() = 0;

	WCHAR m_wzName[MAXNAMEBUFFER];
	WCHAR m_wzMatchName[MAXNAMEBUFFER]; // case-insensitive
	};

class CodeViewer;

class DebuggerModule:
	public CComObjectRootEx<CComSingleThreadModel>,
	public IDispatchImpl<IVPDebug, &IID_IVPDebug, &LIBID_VPinballLib>,
	public IScriptable
	{
BEGIN_COM_MAP(DebuggerModule)
	COM_INTERFACE_ENTRY(IVPDebug)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

	STDMETHOD(Print)(VARIANT *pvar);

	void Init(CodeViewer *pcv);

	virtual IDispatch *GetDispatch();

	virtual ISelect *GetISelect() {return NULL;}

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

class CodeViewDispatch : public ISort<CodeViewDispatch>/*Strings*/
	{
public:
	CodeViewDispatch();
	~CodeViewDispatch();
	WCHAR m_wzName[32];
	IUnknown *m_punk;
	IDispatch *m_pdisp;
	IScriptable *m_piscript;
	BOOL m_fGlobal;
	virtual int SortAgainst(CodeViewDispatch *pcvd/*void *pvoid*/);
	virtual int SortAgainstValue(void *pv);
	};

class CodeViewer :
	public CComObjectRoot,
	//public IDispatchImpl<IDragPoint, &IID_IDragPoint, &LIBID_VPinballLib>,
	//public CComCoClass<CodeViewer,&CLSID_DragPoint>,
	//public CComObjectRootEx<CComSingleThreadModel>,
	public IActiveScriptSite,
	public IActiveScriptSiteWindow,
	public IInternetHostSecurityManager,
	public IServiceProvider
	{
public:
	CodeViewer();
	virtual ~CodeViewer();

	void Init(IScriptableHost *psh);

	void Create();
	void Destroy();
	void SetVisible(BOOL fVisible);

	void SetEnabled(BOOL fEnabled);

	void SetClean(SaveDirtyState sds);

// Script Class
	STDMETHOD(CleanUpScriptEngine)();
	STDMETHOD(InitializeScriptEngine)();

	HRESULT AddItem(IScriptable *piscript, BOOL fGlobal);
	void RemoveItem(IScriptable *piscript);
	HRESULT ReplaceName(IScriptable *piscript, WCHAR *wzNew);
	void SelectItem(IScriptable *piscript);

	void Compile();
	void Start();

	void EndSession();
	HRESULT AddTemporaryItem(BSTR bstr, IDispatch *pdisp);

	STDMETHOD(GetItemInfo)(LPCOLESTR pstrName, DWORD dwReturnMask,
		IUnknown **ppiunkItem, ITypeInfo **ppti);

	STDMETHOD(OnScriptError)(IActiveScriptError *pscripterror);

	STDMETHOD(GetLCID)(LCID *plcid)
	{ *plcid = 9; return S_OK; }

	STDMETHOD(GetDocVersionString)(BSTR *pbstrVersion)
	{ *pbstrVersion = SysAllocString(L""); return S_OK; }

	STDMETHOD(OnScriptTerminate)(const VARIANT *pvr, const EXCEPINFO *pei)
	{ return S_OK; }

	STDMETHOD(OnStateChange)(SCRIPTSTATE ssScriptState)
	{ return S_OK; }

	STDMETHOD(OnEnterScript)();

	STDMETHODIMP OnLeaveScript();

	STDMETHODIMP GetWindow(HWND *phwnd)
	{ *phwnd = GetDesktopWindow(); return S_OK; }

	STDMETHODIMP EnableModeless(BOOL)
	{ return S_OK; }

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

	BOOL FControlAlreadyOkayed(CONFIRMSAFETY *pcs);
	void AddControlToOkayedList(CONFIRMSAFETY *pcs);
	BOOL FControlMarkedSafe(CONFIRMSAFETY *pcs);
	BOOL FUserManuallyOkaysControl(CONFIRMSAFETY *pcs);

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

	IActiveScriptParse* m_pScriptParse;
	IActiveScript* m_pScript;
	IActiveScriptDebug* m_pScriptDebug;

	VectorSortString<CodeViewDispatch> m_vcvd;

	VectorSortString<CodeViewDispatch> m_vcvdTemp; // Objects added through script

	FINDREPLACE m_findreplacestruct;
	char szFindString[81];
	char szReplaceString[81];

	BOOL m_fScriptError; // Whether a script error has occured - used for polling from the game

	IScriptableHost *m_psh;

// Edit Class
	void ColorLine(int line);
	void UncolorError();
	void ColorError(int line, int nchar);
	void ShowFindDialog();
	void ShowFindReplaceDialog();
	void Find(FINDREPLACE *pfr);
	void Replace(FINDREPLACE *pfr);
	void SaveToStream(IStream *pistream, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey);	//<<< modified by chris as part of table protection
	void LoadFromStream(IStream *pistream, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey);	//<<< modified by chris as part of table protection

	void SetCaption(char *szCaption);

	void ListEventsFromItem();
	void FindCodeFromEvent();
	void TellHostToSelectItem();
	void GetParamsFromEvent(int iEvent, char *szParams);

	void MarginClick(int position, int modifiers);

	void EvaluateScriptStatement(char *szScript);
	void AddToDebugOutput(char *szText);

	CComObject<DebuggerModule> *m_pdm; // Object to expose to script for global functions
	//ULONG m_cref;

	HWND m_hwndMain;
	HWND m_hwndScintilla;
	HWND m_hwndStatus;
	HWND m_hwndFind;

	HWND m_hwndItemList;
	HWND m_hwndEventList;
   HWND m_hwndFunctionList;

	SaveDirtyState m_sdsDirty;
	BOOL m_fIgnoreDirty;

	HACCEL m_haccel; // Accelerator keys

	FINDREPLACE m_findreplaceold; //the last thing found/replaced

	int m_errorLineNumber;
	};

class Collection:
	public IDispatchImpl<ICollection, &IID_ICollection, &LIBID_VPinballLib>,
	public CComObjectRoot,
	public CComCoClass<Collection,&CLSID_Collection>,
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
	virtual IDispatch *GetDispatch();
	virtual ISelect *GetISelect();

	//ILoadable
	virtual HRESULT SaveData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey);
	virtual HRESULT LoadData(IStream *pstm, PinTable *ppt, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey);
	virtual BOOL LoadToken(int id, BiffReader *pbr);

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

	Vector<ISelect> m_visel;

	BOOL m_fFireEvents;
	BOOL m_fStopSingleEvents;
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

inline bool FIsWhitespace(const char ch)
	{
	return (ch == ' ' || ch == 9/*tab*/);
	}

inline void AddEventToList(char * const sz, const int index, const int dispid, const LPARAM lparam)
{
	const HWND hwnd = (HWND)lparam;
	const int listindex = SendMessage(hwnd, CB_ADDSTRING, 0, (size_t)sz);
	SendMessage(hwnd, CB_SETITEMDATA, listindex, index);
}
