// license:GPLv3+

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

   float   m_currentHitThreshold; // while playing and the ball hits the mesh the hit threshold is updated here
};

#define STARTUNDO \
	BeginUndo(); \
	MarkForUndo();

#define STOPUNDO \
	EndUndo(); \
	SetDirtyDraw();

#define STARTUNDOSELECT \
	GetIEditable()->BeginUndo(); \
	GetIEditable()->MarkForUndo();

#define STOPUNDOSELECT \
	GetIEditable()->EndUndo(); \
	GetIEditable()->SetDirtyDraw();


#define INITVBA(ItemType) \
   virtual HRESULT InitVBA(BOOL fNew, int id, WCHAR * const wzName) \
   { \
      if (fNew && !wzName) /* setup a default unique name */ \
      { \
         WCHAR wzUniqueName[std::size(m_wzName)]; \
         GetPTable()->GetUniqueName(ItemType, wzUniqueName, std::size(m_wzName)); \
         wcscpy_s(m_wzName, wzUniqueName); \
      } \
      if (GetScriptable() != nullptr) \
      { \
         if (GetScriptable()->m_wzName[0] == '\0') \
            /* Just in case something screws up - not good having a null script name */ \
            swprintf_s(GetScriptable()->m_wzName, std::size(GetScriptable()->m_wzName), L"%Id", reinterpret_cast<uintptr_t>(this)); \
         GetPTable()->m_pcv->AddItem(GetScriptable(), false); \
      } \
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

// declare and implement some methods for an IEditable which does not support scripting (only used for legacy, deprecated Decal editable)
#define STANDARD_NOSCRIPT_EDITABLE_DECLARES(T, ItemType, ResName, AllowedViews) \
	_STANDARD_EDITABLE_CONSTANTS(ItemType, ResName, AllowedViews) \
	_STANDARD_DISPATCH_INDEPENDANT_EDITABLE_DECLARES(T, ItemType) \
	virtual EventProxyBase *GetEventProxyBase() {return nullptr;} \
	inline IFireEvents *GetIFireEvents() {return nullptr;} \
	virtual IScriptable *GetScriptable() {return nullptr;} \
	virtual const IScriptable *GetScriptable() const {return nullptr;}

// used above, do not invoke directly
#define _STANDARD_DISPATCH_EDITABLE_DECLARES(itemType) \
	inline IFireEvents *GetIFireEvents() {return (IFireEvents *)this;} \
	virtual EventProxyBase *GetEventProxyBase() {return (EventProxyBase *)this;} \
	virtual const WCHAR *get_Name() const { return m_wzName; } \
	STDMETHOD(get_Name)(/*[out, retval]*/ BSTR *pVal) \
	{ \
		*pVal = SysAllocString(m_wzName); \
		return S_OK; \
	} \
	STDMETHOD(put_Name)(/*[in]*/ BSTR newVal) \
	{ \
		wstring newName = newVal; \
		if (newName.empty() || newName.length() >= MAXNAMEBUFFER) \
			return E_FAIL; \
		if (GetPTable()->m_pcv->ReplaceName(this, newName) != S_OK) \
			return E_FAIL; \
		wcscpy_s(m_wzName, newName.c_str()); \
		return S_OK; \
	} \
	STDMETHOD(get_TimerInterval)(/*[out, retval]*/ LONG *pVal) {*pVal = m_d.m_tdr.m_TimerInterval; return S_OK;} \
	STDMETHOD(put_TimerInterval)(/*[in]*/ LONG newVal) {return IEditable::put_TimerInterval(newVal, &m_d.m_tdr.m_TimerInterval);} \
	STDMETHOD(get_TimerEnabled)(/*[out, retval]*/ VARIANT_BOOL *pVal) {*pVal = FTOVB(m_d.m_tdr.m_TimerEnabled); return S_OK;} \
	STDMETHOD(put_TimerEnabled)(/*[in]*/ VARIANT_BOOL newVal) {BOOL tmp = m_d.m_tdr.m_TimerEnabled ? 1 : 0; const HRESULT res = IEditable::put_TimerEnabled(newVal, &tmp); m_d.m_tdr.m_TimerEnabled = (tmp != 0); return res;} \
	STDMETHOD(get_UserValue)(VARIANT *pVal) {return IEditable::get_UserValue(pVal);} \
	STDMETHOD(put_UserValue)(VARIANT *newVal) {return IEditable::put_UserValue(newVal);} \
	virtual IScriptable *GetScriptable() {return (IScriptable *)this;} \
	virtual const IScriptable *GetScriptable() const {return (const IScriptable *)this;} \
	virtual void FireGroupEvent(const int dispid) {FireVoidGroupEvent(dispid);}

// used above, do not invoke directly
#define _STANDARD_DISPATCH_INDEPENDANT_EDITABLE_DECLARES(T, ItemType) \
    static T* COMCreate() \
    { \
        CComObject<T> *obj = nullptr; \
        if (FAILED(CComObject<T>::CreateInstance(&obj))) \
        { \
            MessageBox(0, "Failed to create COM object.", "Visual Pinball", MB_OK | MB_ICONEXCLAMATION); \
        } \
        obj->AddRef(); \
        return obj; \
    } \
    static IEditable* COMCreateEditable()   { return static_cast<IEditable*>(COMCreate()); } \
    static IEditable* COMCreateAndInit(PinTable * const ptable, const float x, const float y) \
    { \
        T *obj = T::COMCreate(); \
        obj->Init(ptable, x, y, true); \
        return obj; \
    } \
	T *CopyForPlay(PinTable *live_table) const; \
	HRESULT Init(PinTable * const ptable, const float x, const float y, const bool fromMouseClick, const bool forPlay = false); \
	INITVBA(ItemType) \
	virtual void UIRenderPass1(Sur * const psur); \
	virtual void UIRenderPass2(Sur * const psur); \
	virtual PinTable *GetPTable() { return m_ptable; } \
	virtual const PinTable *GetPTable() const { return m_ptable; } \
	virtual void Delete() {IEditable::Delete();} \
	virtual void Uncreate() {IEditable::Uncreate();} \
	virtual HRESULT SaveData(IStream *pstm, HCRYPTHASH hcrypthash, const bool saveForUndo); \
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
	STDMETHOD(GetDisplayString)(DISPID dispID, BSTR *pbstr) {return hrNotImplemented;} \
	STDMETHOD(MapPropertyToPage)(DISPID dispID, CLSID *pclsid) {return hrNotImplemented;} \
	STDMETHOD(GetPredefinedStrings)(DISPID dispID, CALPOLESTR *pcaStringsOut, CADWORD *pcaCookiesOut) {return GetPTable()->GetPredefinedStrings(dispID, pcaStringsOut, pcaCookiesOut, this);} \
	STDMETHOD(GetPredefinedValue)(DISPID dispID, DWORD dwCookie, VARIANT *pVarOut) {return GetPTable()->GetPredefinedValue(dispID, dwCookie, pVarOut, this);} \
	virtual void SetDefaults(const bool fromMouseClick); \
	/* Hitable implementation */ \
	virtual void TimerSetup(vector<HitTimer*> &pvht) { \
      m_singleEvents = true; \
      for (size_t i = 0; i < m_vCollection.size(); i++) \
      { \
         Collection *const pcol = m_vCollection[i]; \
         if (pcol->m_fireEvents) \
         { \
            m_vEventCollection.push_back(pcol); \
            m_viEventCollection.push_back(m_viCollection[i]); \
         } \
         if (pcol->m_stopSingleEvents) \
            m_singleEvents = false; \
      } \
      IFireEvents * fe = GetIFireEvents(); if (fe) { m_phittimer = new HitTimer(GetName(), m_d.m_tdr.m_TimerInterval, fe); if (m_d.m_tdr.m_TimerEnabled) pvht.push_back(m_phittimer); } \
	} \
	virtual void TimerRelease() { delete m_phittimer; m_phittimer = nullptr; } \
	virtual void PhysicSetup(PhysicsEngine* physics, const bool isUI); \
	virtual void PhysicRelease(PhysicsEngine* physics, const bool isUI); \
	/* IRenderable implementation */ \
	virtual void RenderSetup(RenderDevice *device); \
	virtual void UpdateAnimation(const float diff_time_msec); \
	virtual void Render(const unsigned int renderMask); \
	virtual void RenderRelease();

#define _STANDARD_EDITABLE_CONSTANTS(ItTy, ResName, AllwdViews) \
	static const ItemTypeEnum ItemType = ItTy; \
	static const int TypeNameID = IDS_TB_##ResName; \
	static const int ToolID = ID_INSERT_##ResName; \
	static const int CursorID = IDC_##ResName; \
	static const unsigned AllowedViews = AllwdViews;

#define STANDARD_EDITABLE_COPY_FOR_PLAY_IMPL(type, table) \
   type *dst = type::COMCreate(); \
   dst->Init(table, 0.f, 0.f, false, true); \
   memcpy(dst->m_wzName, m_wzName, sizeof(m_wzName)); \
   if (dst->GetScriptable()) \
      table->m_pcv->AddItem(dst->GetScriptable(), false); \
   dst->m_isVisible = m_isVisible; \
   dst->m_backglass = m_backglass; \
   dst->m_locked = m_locked; \
   dst->m_d = m_d;

#define STANDARD_EDITABLE_WITH_DRAGPOINT_COPY_FOR_PLAY_IMPL(type, table, points) \
   STANDARD_EDITABLE_COPY_FOR_PLAY_IMPL(type, table) \
   for (size_t i = 0; i < dst->points.size(); i++) \
      dst->points[i]->Release(); \
   dst->points.clear(); \
   CComObject<DragPoint> *pdp; \
   for (const auto dpt : m_vdpoint) \
   { \
      CComObject<DragPoint>::CreateInstance(&pdp); \
      if (pdp) \
      { \
         pdp->AddRef(); \
         pdp->Init(dst, dpt->m_v.x, dpt->m_v.y, dpt->m_v.z, dpt->m_smooth); \
         pdp->m_slingshot = dpt->m_slingshot; \
         pdp->m_calcHeight = dpt->m_calcHeight; \
         pdp->m_autoTexture = dpt->m_autoTexture; \
         pdp->m_texturecoord = dpt->m_texturecoord; \
         dst->points.push_back(pdp); \
      } \
   }

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

   virtual void ExportMesh(class ObjLoader& loader) {}

   virtual ULONG STDMETHODCALLTYPE AddRef() = 0;
   virtual ULONG STDMETHODCALLTYPE Release() = 0;

   virtual PinTable *GetPTable() = 0;
   virtual const PinTable *GetPTable() const = 0;

   virtual void SetDirtyDraw();

   virtual Hitable *GetIHitable() { return nullptr; }
   virtual const Hitable *GetIHitable() const { return nullptr; }

   virtual HRESULT SaveData(IStream *pstm, HCRYPTHASH hcrypthash, const bool saveForUndo) = 0;
   virtual void ClearForOverwrite() { }
   virtual HRESULT InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey) = 0;
   virtual HRESULT InitPostLoad() = 0;
   virtual HRESULT InitVBA(BOOL fNew, int id, WCHAR * const wzName) = 0;
   virtual ISelect *GetISelect() = 0;
   virtual const ISelect *GetISelect() const = 0;
   virtual void SetDefaults(const bool fromMouseClick) = 0;
   virtual IScriptable *GetScriptable() = 0;
   virtual const IScriptable *GetScriptable() const = 0;
   virtual IFireEvents *GetIFireEvents() = 0;
   virtual ItemTypeEnum GetItemType() const = 0;

   // if legacy_bounds != nullptr, can return pre-10.8 bounds, too (depending on which editable exactly)
   virtual void GetBoundingVertices(vector<Vertex3Ds> &bounds, vector<Vertex3Ds> *const legacy_bounds) {}

   virtual void WriteRegDefaults() {}

   virtual void BeginUndo();
   virtual void EndUndo();
   void MarkForUndo();
   void MarkForDelete();
   void Undelete();
   string GetName() const;
   void SetName(const string& name);
   virtual void Delete();
   virtual void Uncreate();

   bool m_backglass = false; // if the light/decal (+dispreel/textbox is always true) is on the table (false) or a backglass view

   void SetPartGroup(class PartGroup *partGroup);
   class PartGroup* GetPartGroup() const { return m_partGroup; }
   string GetPathString(const bool isDirOnly) const;
   bool IsChild(const PartGroup* group) const;

   HRESULT put_TimerEnabled(VARIANT_BOOL newVal, BOOL *pte);
   HRESULT put_TimerInterval(long newVal, int *pti);

   vector<Collection *> m_vCollection;
   vector<int> m_viCollection;

   HRESULT get_UserValue(VARIANT *pVal);
   HRESULT put_UserValue(VARIANT *newVal);

private:
   VARIANT m_uservalue;

   class PartGroup* m_partGroup = nullptr; // Parenting to group (or top level layers) for base transform and visibility

#pragma region Script events
public:
   HitTimer *m_phittimer = nullptr; // timer event defined when playing (between TimerSetup and TimerRelease)

   // In game filtered copy of m_vCollection/m_viCollection for slightly faster event dispatching
   vector<Collection *> m_vEventCollection;
   vector<int> m_viEventCollection;
   bool m_singleEvents;
#pragma endregion
};
