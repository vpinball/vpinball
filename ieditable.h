#pragma once
class Hitable;
class Collection;

class IScriptable;

class IFireEvents
	{
public:
	virtual void FireGroupEvent(int dispid) = 0;
	virtual IDispatch *GetDispatch() = 0;
	virtual IDebugCommands *GetDebugCommands() = 0;
	};

#define STARTUNDO \
	if (g_fKeepUndoRecords) \
		{ \
		BeginUndo(); \
		MarkForUndo(); \
		}

#define STOPUNDO \
	if (g_fKeepUndoRecords) \
		{ \
		EndUndo(); \
		SetDirtyDraw(); \
		}

#define STARTUNDOSELECT \
	if (g_fKeepUndoRecords) \
		{ \
		GetIEditable()->BeginUndo(); \
		GetIEditable()->MarkForUndo(); \
		}

#define STOPUNDOSELECT \
	if (g_fKeepUndoRecords) \
		{ \
		GetIEditable()->EndUndo(); \
		GetIEditable()->SetDirtyDraw(); \
		}

extern const WCHAR rgwzTypeName[][17];

///////////////////////////
/////// Begin VBA defines
///////////////////////////

#ifdef VBA

#define APCPROJECTUNDEFINE ApcProjectItem.Undefine();

#define APCCONTROLUNDEFINE ApcControl.Undefine();

#define PUTAPCITEMNAME \
	GetIApcProjectItem()->put_Name(newVal);

#define INITVBANODISPATCH \
	virtual EventProxyBase *GetEventProxyBase() {return NULL;} \
	virtual IApcProjectItem *GetIApcProjectItem() {return NULL;}

#define INITVBA \
	virtual IApcProjectItem *GetIApcProjectItem() {return ApcProjectItem.GetApcProjectItem();} \
	virtual IApcControl *GetIApcControl() {return NULL;} \
	virtual HRESULT InitVBA(BOOL fNew, int id, WCHAR *wzName) \
		{ \
		if (fNew) \
			{ \
			WCHAR wzUniqueName[128]; \
			if (!wzName) \
				{ \
				GetPTable()->GetUniqueName(ItemType, wzUniqueName); \
				WideStrCopy(wzUniqueName, (WCHAR *)m_wzName);/*lstrcpyW((WCHAR *)m_wzName, wzUniqueName);*/ \
				} \
			HRESULT hr = ApcProjectItem.Define(GetPTable()->ApcProject, GetDispatch(), axTypeHostProjectItem/*axTypeHostClass*/, wzName ? wzName : wzUniqueName, NULL); \
			InitScript(); \
			return hr; \
			} \
		else \
			{ \
			HRESULT hr = ApcProjectItem.Register(GetPTable()->ApcProject, GetDispatch(), id); \
			InitScript(); \
			return hr; \
			} \
		}
#else // No VBA

#define PUTAPCITEMNAME

#define INITVBANODISPATCH \
	virtual EventProxyBase *GetEventProxyBase() {return NULL;} \

#define APCPROJECTUNDEFINE

#define APCCONTROLUNDEFINE

#define INITVBA(ItemType) \
	virtual HRESULT InitVBA(BOOL fNew, int id, WCHAR *wzName) \
		{ \
		WCHAR wzUniqueName[128]; \
		if (fNew && !wzName) \
			{ \
				{ \
				GetPTable()->GetUniqueName(ItemType, wzUniqueName); \
				WideStrCopy(wzUniqueName, (WCHAR *)m_wzName);/*lstrcpyW((WCHAR *)m_wzName, wzUniqueName);*/ \
				} \
			} \
		InitScript(); \
		return S_OK; \
		}
#endif // VBA

///////////////////////////
/////// End VBA defines
///////////////////////////

#define STANDARD_EDITABLE_DECLARES(ItemType) \
	STANDARD_DISPATCH_EDITABLE_DECLARES(ItemType) \
	STANDARD_DISPATCH_INDEPENDANT_EDITABLE_DECLARES(ItemType) \
	virtual void Delete() {IEditable::Delete();}\
	virtual void Uncreate() {IEditable::Uncreate();}

#define STANDARD_NONAPC_EDITABLE_DECLARES(ItemType) \
	STANDARD_NON_DISPATCH_EDITABLE_DECLARES(ItemType) \
	STANDARD_DISPATCH_INDEPENDANT_EDITABLE_DECLARES(ItemType) \
	virtual void Delete() {GetPTable()->m_vedit.RemoveElement((IEditable *)this); MarkForDelete(); APCCONTROLUNDEFINE} \
	virtual void Uncreate() {GetPTable()->m_vedit.RemoveElement((IEditable *)this); APCCONTROLUNDEFINE}

#define STANDARD_DISPATCH_DECLARE \
	inline IDispatch *GetDispatch() {return (IDispatch *)this;} \

#define STANDARD_DISPATCH_EDITABLE_DECLARES(ItemType) \
	inline IFireEvents *GetIFireEvents() {return (IFireEvents *)this;} \
	inline IDebugCommands *GetDebugCommands() {return NULL;} \
	virtual EventProxyBase *GetEventProxyBase() {return (EventProxyBase *)this;} \
	INITVBA(ItemType) \
	STDMETHOD(get_Name)(/*[out, retval]*/ BSTR *pVal) \
		{ \
		*pVal = SysAllocString((WCHAR *)m_wzName); \
		return S_OK; \
		} \
	STDMETHOD(put_Name)(/*[in]*/ BSTR newVal) \
		{ \
		int len = lstrlenW(newVal); \
		if (len > 32 || len < 1) \
			{ \
			return E_FAIL; \
			} \
		if (GetPTable()->m_pcv->ReplaceName(this, newVal) == S_OK) \
			{ \
			WideStrCopy(newVal, (WCHAR *)m_wzName);/*lstrcpyW((WCHAR *)m_wzName, newVal);*/ \
			PUTAPCITEMNAME \
			return S_OK; \
			} \
		return E_FAIL; \
		} \
	STDMETHOD(get_TimerInterval)(/*[out, retval]*/ long *pVal) {*pVal = m_d.m_tdr.m_TimerInterval; return S_OK;} \
	STDMETHOD(put_TimerInterval)(/*[in]*/ long newVal) {return IEditable::put_TimerInterval(newVal, &m_d.m_tdr.m_TimerInterval);} \
	STDMETHOD(get_TimerEnabled)(/*[out, retval]*/ VARIANT_BOOL *pVal) {*pVal = FTOVB(m_d.m_tdr.m_fTimerEnabled); return S_OK;} \
	STDMETHOD(put_TimerEnabled)(/*[in]*/ VARIANT_BOOL newVal) {return IEditable::put_TimerEnabled(newVal, &m_d.m_tdr.m_fTimerEnabled);} \
	STDMETHOD(get_UserValue)(VARIANT *pVal) {return IEditable::get_UserValue(pVal);} \
	STDMETHOD(put_UserValue)(VARIANT *newVal) {return IEditable::put_UserValue(newVal);} \
	virtual IScriptable *GetScriptable() {return (IScriptable *)this;} \
	virtual void FireGroupEvent(int dispid) {FireVoidGroupEvent(dispid);}

#define STANDARD_NON_DISPATCH_EDITABLE_DECLARES(ItemType) \
	inline IDispatch *GetDispatch() {return (IDispatch *)this;} \
	inline IFireEvents *GetIFireEvents() {return NULL;} \
	inline IDebugCommands *GetDebugCommands() {return NULL;} \
	INITVBANODISPATCH
	//virtual HRESULT InitVBA(BOOL fNew, int id) {return S_OK;}

#define STANDARD_DISPATCH_INDEPENDANT_EDITABLE_DECLARES(ItemType) \
	HRESULT Init(PinTable *ptable, float x, float y, bool fromMouseClick); \
	virtual void PreRender(Sur * const psur); \
	virtual void Render(Sur * const psur); \
	virtual PinTable *GetPTable() {return m_ptable;} \
	virtual void GetHitShapes(Vector<HitObject> * const pvho); \
	virtual void GetHitShapesDebug(Vector<HitObject> * const pvho); \
	virtual void GetTimers(Vector<HitTimer> * const pvht); \
	virtual void EndPlay(); \
	virtual HRESULT SaveData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey); \
	virtual ItemTypeEnum GetItemType() {return ItemType;} \
	virtual HRESULT InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey); \
	virtual HRESULT InitPostLoad(); \
	virtual BOOL LoadToken(int id, BiffReader *pbr); \
	virtual IEditable *GetIEditable() {return (IEditable*)this;} \
	virtual ISelect *GetISelect() {return (ISelect*)this;} \
	virtual Hitable *GetIHitable() {return (Hitable *)this;} \
   virtual void RenderSetup(const RenderDevice* pd3dDevice); \
   virtual void RenderStatic(const RenderDevice* pd3dDevice); \
	virtual void PostRenderStatic(const RenderDevice* pd3dDevice); \
	virtual void RenderMovers(const RenderDevice* pd3dDevice); \
	STDMETHOD(GetDisplayString)(DISPID dispID, BSTR *pbstr) {return hrNotImplemented;}\
	STDMETHOD(MapPropertyToPage)(DISPID dispID, CLSID *pclsid) {return hrNotImplemented;} \
	STDMETHOD(GetPredefinedStrings)(DISPID dispID, CALPOLESTR *pcaStringsOut, CADWORD *pcaCookiesOut) {return GetPTable()->GetPredefinedStrings(dispID, pcaStringsOut, pcaCookiesOut, this);} \
	STDMETHOD(GetPredefinedValue)(DISPID dispID, DWORD dwCookie, VARIANT *pVarOut) {return GetPTable()->GetPredefinedValue(dispID, dwCookie, pVarOut, this);} \
	virtual void SetDefaults(bool fromMouseClick);

class ShadowSur;

class EventProxyBase;

// IEditable is the subclass for anything class which is a self-contained table element.
// It knows how to draw itself, interact with event and properties,
// And talk to the player
// Example:  Bumper is an IEditable and ISelect, but DragPoint is only ISelect.

class IEditable : public IUndo
	{
public:
	IEditable();
	virtual ~IEditable();

	virtual void PreRender(Sur * const psur) = 0;
	virtual void Render(Sur * const psur) = 0;
	virtual void RenderBlueprint(Sur *psur);
	virtual void RenderShadow(ShadowSur * const psur, const float height);
	virtual ULONG STDMETHODCALLTYPE AddRef() = 0;
	virtual ULONG STDMETHODCALLTYPE Release() = 0;

	virtual PinTable *GetPTable()=0;

	void SetDirtyDraw();

	virtual Hitable *GetIHitable();

	virtual HRESULT SaveData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey) = 0;
#ifdef VBA
	virtual IApcProjectItem *GetIApcProjectItem()=0;
	virtual IApcControl *GetIApcControl()=0;
#endif
	virtual void ClearForOverwrite();
	virtual HRESULT InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)=0;
	virtual HRESULT InitPostLoad()=0;
	virtual HRESULT InitVBA(BOOL fNew, int id, WCHAR *wzName)=0;
	virtual ISelect *GetISelect()=0;
	virtual void SetDefaults(bool fromMouseClick)=0;
	virtual IScriptable *GetScriptable() = 0;
	virtual IFireEvents *GetIFireEvents() = 0;
	virtual ItemTypeEnum GetItemType() = 0;

	virtual void GetBoundingVertices(Vector<Vertex3Ds> * const pvvertex3D) {}
	virtual void WriteRegDefaults(){}
	
	void BeginUndo();
	void EndUndo();
	void MarkForUndo();
	void MarkForDelete();
	void Undelete();

	void Delete();
	void Uncreate();

	void InitScript();

	HRESULT put_TimerEnabled(VARIANT_BOOL newVal, BOOL *pte);
	HRESULT put_TimerInterval(long newVal, int *pti);

	HRESULT get_UserValue(VARIANT *pVal);
	HRESULT put_UserValue(VARIANT *newVal);

	void BeginPlay();
	void EndPlay();

	int m_fBackglass; // if the light is on the table (0) or a backglass view

	HitTimer *m_phittimer;

	VARIANT m_uservalue;

	Vector<Collection> m_vCollection;
	Vector<int> m_viCollection;

	// Optimizations for in-game
	Vector<Collection> m_vEventCollection;
	Vector<int> m_viEventCollection;
	BOOL m_fSingleEvents;

	// For undo - holds VBA stuff that was removed from project
	//  that we have to put back if we undo
	WCHAR *m_wzVBAName;
	WCHAR *m_wzVBACode;
   bool isVisible;
	};
