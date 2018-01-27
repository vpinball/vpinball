#pragma once

class Hitable;
class Collection;

class IScriptable;

#define BLUEPRINT_SOLID_COLOR RGB(0,0,0)

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

// Explanation for AllowedViews:
// Value gets and'ed with 1 (table view) or 2 (backglass view).
// If you want to allow an element to be pasted only into the table view, use 1,
// for only backglass view, use 2, and for both, use 3.

// declare and implement some methods for an IEditable which supports scripting
#define STANDARD_EDITABLE_DECLARES(T, ItemType, ResName, AllowedViews) \
	_STANDARD_EDITABLE_CONSTANTS(ItemType, ResName, AllowedViews) \
	_STANDARD_DISPATCH_INDEPENDANT_EDITABLE_DECLARES(T, ItemType) \
	_STANDARD_DISPATCH_EDITABLE_DECLARES(ItemType)

// declare and implement some methods for an IEditable which does not support scripting
#define STANDARD_NOSCRIPT_EDITABLE_DECLARES(T, ItemType, ResName, AllowedViews) \
	_STANDARD_EDITABLE_CONSTANTS(ItemType, ResName, AllowedViews) \
	_STANDARD_DISPATCH_INDEPENDANT_EDITABLE_DECLARES(T, ItemType) \
	virtual EventProxyBase *GetEventProxyBase() {return NULL;} \
	inline IFireEvents *GetIFireEvents() {return NULL;} \
	inline IDebugCommands *GetDebugCommands() {return NULL;} \
	virtual IScriptable *GetScriptable() {return NULL;}

// used above, do not invoke directly
#define _STANDARD_DISPATCH_EDITABLE_DECLARES(itemType) \
	inline IFireEvents *GetIFireEvents() {return (IFireEvents *)this;} \
	inline IDebugCommands *GetDebugCommands() {return NULL;} \
	virtual EventProxyBase *GetEventProxyBase() {return (EventProxyBase *)this;} \
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
			return S_OK; \
      			} \
		return E_FAIL; \
		} \
	STDMETHOD(get_TimerInterval)(/*[out, retval]*/ long *pVal) {*pVal = m_d.m_tdr.m_TimerInterval; return S_OK;} \
	STDMETHOD(put_TimerInterval)(/*[in]*/ long newVal) {return IEditable::put_TimerInterval(newVal, &m_d.m_tdr.m_TimerInterval);} \
	STDMETHOD(get_TimerEnabled)(/*[out, retval]*/ VARIANT_BOOL *pVal) {*pVal = FTOVB(m_d.m_tdr.m_fTimerEnabled); return S_OK;} \
	STDMETHOD(put_TimerEnabled)(/*[in]*/ VARIANT_BOOL newVal) {BOOL tmp; const HRESULT res = IEditable::put_TimerEnabled(newVal, &tmp); m_d.m_tdr.m_fTimerEnabled = (tmp != 0); return res;} \
	STDMETHOD(get_UserValue)(VARIANT *pVal) {return IEditable::get_UserValue(pVal);} \
	STDMETHOD(put_UserValue)(VARIANT *newVal) {return IEditable::put_UserValue(newVal);} \
	virtual IScriptable *GetScriptable() {return (IScriptable *)this;} \
	virtual void FireGroupEvent(int dispid) {FireVoidGroupEvent(dispid);}

// used above, do not invoke directly
#define _STANDARD_DISPATCH_INDEPENDANT_EDITABLE_DECLARES(T, ItemType) \
    static T* COMCreate() \
    { \
        CComObject<T> *obj = NULL; \
        if (FAILED(CComObject<T>::CreateInstance(&obj))) \
                { \
            MessageBox(0, "Failed to create COM object.", "Visual Pinball", MB_ICONEXCLAMATION); \
                } \
        obj->AddRef(); \
        return obj; \
    } \
    static IEditable* COMCreateEditable()   { return static_cast<IEditable*>(COMCreate()); } \
    static IEditable* COMCreateAndInit(PinTable *ptable, float x, float y) \
    { \
        T *obj = T::COMCreate(); \
        obj->Init(ptable, x, y, true); \
        return obj; \
    } \
	HRESULT Init(PinTable *ptable, float x, float y, bool fromMouseClick); \
	INITVBA(ItemType) \
	virtual void PreRender(Sur * const psur); \
	virtual void Render(Sur * const psur); \
	virtual PinTable *GetPTable() {return m_ptable;} \
	virtual void GetHitShapes(Vector<HitObject> * const pvho); \
	virtual void GetHitShapesDebug(Vector<HitObject> * const pvho); \
	virtual void GetTimers(Vector<HitTimer> * const pvht); \
	virtual void EndPlay(); \
	virtual void Delete() {IEditable::Delete();} \
	virtual void Uncreate() {IEditable::Uncreate();} \
	virtual HRESULT SaveData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey); \
	virtual ItemTypeEnum GetItemType() const { return ItemType; } \
	virtual HRESULT InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey); \
	virtual HRESULT InitPostLoad(); \
	virtual BOOL LoadToken(int id, BiffReader *pbr); \
	virtual IDispatch *GetDispatch() {return static_cast<IDispatch *>(this);} \
	virtual IEditable *GetIEditable() {return static_cast<IEditable*>(this);} \
	virtual ISelect *GetISelect() {return static_cast<ISelect*>(this);} \
	virtual Hitable *GetIHitable() {return static_cast<Hitable *>(this);} \
    virtual void RenderSetup(RenderDevice* pd3dDevice); \
    virtual void RenderStatic(RenderDevice* pd3dDevice); \
	virtual void PostRenderStatic(RenderDevice* pd3dDevice); \
	STDMETHOD(GetDisplayString)(DISPID dispID, BSTR *pbstr) {return hrNotImplemented;}\
	STDMETHOD(MapPropertyToPage)(DISPID dispID, CLSID *pclsid) {return hrNotImplemented;} \
	STDMETHOD(GetPredefinedStrings)(DISPID dispID, CALPOLESTR *pcaStringsOut, CADWORD *pcaCookiesOut) {return GetPTable()->GetPredefinedStrings(dispID, pcaStringsOut, pcaCookiesOut, this);} \
	STDMETHOD(GetPredefinedValue)(DISPID dispID, DWORD dwCookie, VARIANT *pVarOut) {return GetPTable()->GetPredefinedValue(dispID, dwCookie, pVarOut, this);} \
	virtual void SetDefaults(bool fromMouseClick);

#define _STANDARD_EDITABLE_CONSTANTS(ItTy, ResName, AllwdViews) \
    static const ItemTypeEnum ItemType = ItTy; \
    static const int TypeNameID = IDS_TB_##ResName; \
    static const int ToolID = ID_INSERT_##ResName; \
    static const int CursorID = IDC_##ResName; \
    static const unsigned AllowedViews = AllwdViews;


class EventProxyBase;

// IEditable is the subclass for anything class which is a self-contained table element.
// It knows how to draw itself, interact with event and properties,
// And talk to the player
// Example:  Bumper is an IEditable and ISelect, but DragPoint is only ISelect.

class IEditable
{
public:
   IEditable();
   virtual ~IEditable();

   virtual void PreRender(Sur * const psur) = 0;
   virtual void Render(Sur * const psur) = 0;
   virtual void RenderBlueprint(Sur *psur, const bool solid=false);
   virtual void ExportMesh(FILE *f) {}

   virtual ULONG STDMETHODCALLTYPE AddRef() = 0;
   virtual ULONG STDMETHODCALLTYPE Release() = 0;

   virtual PinTable *GetPTable() = 0;

   void SetDirtyDraw();

   virtual Hitable *GetIHitable();

   virtual HRESULT SaveData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey) = 0;
   virtual void ClearForOverwrite();
   virtual HRESULT InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey) = 0;
   virtual HRESULT InitPostLoad() = 0;
   virtual HRESULT InitVBA(BOOL fNew, int id, WCHAR *wzName) = 0;
   virtual ISelect *GetISelect() = 0;
   virtual void SetDefaults(bool fromMouseClick) = 0;
   virtual IScriptable *GetScriptable() = 0;
   virtual IFireEvents *GetIFireEvents() = 0;
   virtual ItemTypeEnum GetItemType() const = 0;

   virtual void GetBoundingVertices(std::vector<Vertex3Ds>& pvvertex3D) {}
   virtual void WriteRegDefaults() {}

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

   HitTimer *m_phittimer;

   VARIANT m_uservalue;

   Vector<Collection> m_vCollection;
   Vector<int> m_viCollection;

   // Optimizations for in-game
   Vector<Collection> m_vEventCollection;
   Vector<int> m_viEventCollection;
   bool m_fSingleEvents;

   bool m_fBackglass; // if the light is on the table (false) or a backglass view
   bool m_isVisible;
};
