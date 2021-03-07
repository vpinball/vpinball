#pragma once

class HitTimer;
class Hitable;
class Collection;
class IScriptable;

#define BLUEPRINT_SOLID_COLOR RGB(0,0,0)

class IFireEvents
{

public:
   virtual void FireGroupEvent(const int dispid) = 0;
   virtual IDispatch *GetDispatch() = 0;
   virtual const IDispatch *GetDispatch() const = 0;
   virtual IDebugCommands *GetDebugCommands() = 0;

   float   m_currentHitThreshold; // while playing and the ball hits the mesh the hit threshold is updated here
};

#define STARTUNDO \
	if (g_keepUndoRecords) \
		{ \
		BeginUndo(); \
		MarkForUndo(); \
		}

#define STOPUNDO \
	if (g_keepUndoRecords) \
		{ \
		EndUndo(); \
		SetDirtyDraw(); \
		}

#define STARTUNDOSELECT \
	if (g_keepUndoRecords) \
		{ \
		GetIEditable()->BeginUndo(); \
		GetIEditable()->MarkForUndo(); \
		}

#define STOPUNDOSELECT \
	if (g_keepUndoRecords) \
		{ \
		GetIEditable()->EndUndo(); \
		GetIEditable()->SetDirtyDraw(); \
		}


#define INITVBA(ItemType) \
	virtual HRESULT InitVBA(BOOL fNew, int id, WCHAR * const wzName) \
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
		const int len = lstrlenW(newVal); \
		if (len > MAXNAMEBUFFER || len < 1) \
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
	STDMETHOD(get_TimerEnabled)(/*[out, retval]*/ VARIANT_BOOL *pVal) {*pVal = FTOVB(m_d.m_tdr.m_TimerEnabled); return S_OK;} \
	STDMETHOD(put_TimerEnabled)(/*[in]*/ VARIANT_BOOL newVal) {BOOL tmp; const HRESULT res = IEditable::put_TimerEnabled(newVal, &tmp); m_d.m_tdr.m_TimerEnabled = (tmp != 0); return res;} \
	STDMETHOD(get_UserValue)(VARIANT *pVal) {return IEditable::get_UserValue(pVal);} \
	STDMETHOD(put_UserValue)(VARIANT *newVal) {return IEditable::put_UserValue(newVal);} \
	virtual IScriptable *GetScriptable() {return (IScriptable *)this;} \
	virtual void FireGroupEvent(const int dispid) {FireVoidGroupEvent(dispid);}

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
	virtual void UIRenderPass1(Sur * const psur); \
	virtual void UIRenderPass2(Sur * const psur); \
	virtual PinTable *GetPTable() { return m_ptable; } \
	virtual const PinTable *GetPTable() const { return m_ptable; } \
	virtual void GetHitShapes(vector<HitObject*> &pvho); \
	virtual void GetHitShapesDebug(vector<HitObject*> &pvho); \
	virtual void GetTimers(vector<HitTimer*> &pvht); \
	virtual void EndPlay(); \
	virtual void Delete() {IEditable::Delete();} \
	virtual void Uncreate() {IEditable::Uncreate();} \
	virtual HRESULT SaveData(IStream *pstm, HCRYPTHASH hcrypthash, const bool backupForPlay); \
	virtual ItemTypeEnum GetItemType() const { return ItemType; } \
	virtual HRESULT InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey); \
	virtual HRESULT InitPostLoad(); \
	virtual bool LoadToken(const int id, BiffReader * const pbr); \
	virtual IDispatch *GetDispatch() {return static_cast<IDispatch *>(this);} \
	virtual const IDispatch *GetDispatch() const {return static_cast<const IDispatch *>(this);} \
	virtual IEditable *GetIEditable() {return static_cast<IEditable*>(this);} \
	virtual const IEditable *GetIEditable() const {return static_cast<const IEditable*>(this);} \
	virtual ISelect *GetISelect() {return static_cast<ISelect*>(this);} \
	virtual const ISelect *GetISelect() const {return static_cast<const ISelect*>(this);} \
	virtual Hitable *GetIHitable() {return static_cast<Hitable *>(this);} \
	virtual const Hitable *GetIHitable() const {return static_cast<const Hitable *>(this);} \
	virtual void RenderSetup(); \
	virtual void RenderStatic(); \
	virtual void RenderDynamic(); \
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
class ObjLoader;
// IEditable is the subclass for anything class which is a self-contained table element.
// It knows how to draw itself, interact with event and properties,
// And talk to the player
// Example:  Bumper is an IEditable and ISelect, but DragPoint is only ISelect.

class IEditable
{
public:
   IEditable();
   virtual ~IEditable();

   // this function draws the shape of the object with a solid fill
   // only used in the UI/editor and not the game
   //
   // this is called before the grid lines are drawn on the map
   virtual void UIRenderPass1(Sur * const psur) = 0;
   // this function draws the shape of the object with a black outline (no solid fill)
   // only used in the UI/editor and not the game
   //
   // this is called after the grid lines have been drawn on the map.
   // draws a solid outline over the grid lines
   virtual void UIRenderPass2(Sur * const psur) = 0;

   virtual void RenderBlueprint(Sur *psur, const bool solid);

   virtual void ExportMesh(ObjLoader& loader) {}

   virtual ULONG STDMETHODCALLTYPE AddRef() = 0;
   virtual ULONG STDMETHODCALLTYPE Release() = 0;

   virtual PinTable *GetPTable() = 0;
   virtual const PinTable *GetPTable() const = 0;

   void SetDirtyDraw();

   virtual Hitable *GetIHitable() { return NULL; }
   virtual const Hitable *GetIHitable() const { return NULL; }

   virtual HRESULT SaveData(IStream *pstm, HCRYPTHASH hcrypthash, const bool backupForPlay) = 0;
   virtual void ClearForOverwrite();
   virtual HRESULT InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey) = 0;
   virtual HRESULT InitPostLoad() = 0;
   virtual HRESULT InitVBA(BOOL fNew, int id, WCHAR * const wzName) = 0;
   virtual ISelect *GetISelect() = 0;
   virtual const ISelect *GetISelect() const = 0;
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
   char *GetName();
   void SetName(const std::string& name);
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

   vector<Collection*> m_vCollection;
   vector<int> m_viCollection;

   // Optimizations for in-game
   vector<Collection*> m_vEventCollection;
   vector<int> m_viEventCollection;

   bool m_singleEvents;

   bool m_backglass; // if the light/decal (+dispreel/textbox is always true) is on the table (false) or a backglass view
};
