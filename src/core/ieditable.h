// license:GPLv3+

#pragma once

class HitTimer;
class Hitable;
class Collection;
class IRenderable;
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


// Explanation for AllowedViews:
// Value gets and'ed with 1 (table view) or 2 (backglass view).
// If you want to allow an element to be pasted only into the table view, use 1,
// for only backglass view, use 2, and for both, use 3.

// declare and implement some methods for an IEditable which supports scripting
#define STANDARD_EDITABLE_DECLARES(T, ItemType, ResName, AllowedViews) \
	_STANDARD_EDITABLE_CONSTANTS(ItemType, ResName, AllowedViews) \
	_STANDARD_DISPATCH_INDEPENDANT_EDITABLE_DECLARES(T, ItemType) \
	_STANDARD_DISPATCH_EDITABLE_DECLARES(ItemType)

// used above, do not invoke directly
#define _STANDARD_DISPATCH_EDITABLE_DECLARES(itemType) \
	inline IFireEvents *GetIFireEvents() {return (IFireEvents *)this;} \
	virtual EventProxyBase *GetEventProxyBase() {return (EventProxyBase *)this;} \
	STDMETHOD(get_Name)(/*[out, retval]*/ BSTR *pVal) { *pVal = SysAllocStringLen(m_wzName.c_str(), static_cast<UINT>(m_wzName.length())); return S_OK; } \
	STDMETHOD(put_Name)(/*[in]*/ BSTR newVal) { SetName(newVal); return S_OK; } \
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
            ShowError("Failed to create COM object."); \
        } \
        obj->AddRef(); \
        return obj; \
    } \
    static IEditable* COMCreateEditable()   { return static_cast<IEditable*>(COMCreate()); } \
    static IEditable* COMCreateAndInit(const float x, const float y) \
    { \
        T *obj = T::COMCreate(); \
        obj->Init(x, y, true); \
        return obj; \
    } \
	T *CopyForPlay() const; \
	HRESULT Init(const float x, const float y, const bool fromMouseClick, const bool forPlay = false); \
	virtual void UIRenderPass1(Sur * const psur); \
	virtual void UIRenderPass2(Sur * const psur); \
	bool IsUILocked() const override { return m_uiLocked; } \
	void SetUILock(bool lock) override { m_uiLocked = lock; } \
	bool IsUIVisible() const override { return m_uiVisible; } \
	void SetUIVisible(bool visible) override { m_uiVisible = visible; } \
	virtual PinTable *GetPTable() { return m_ptable; } \
	virtual const PinTable *GetPTable() const { return m_ptable; } \
	virtual void Delete() {IEditable::Delete();} \
	virtual void Uncreate() {IEditable::Uncreate();} \
	virtual void Load(IObjectReader &reader); \
	virtual void Save(IObjectWriter &writer, const bool saveForUndo); \
	virtual ItemTypeEnum GetItemType() const { return ItemType; } \
	virtual IDispatch *GetDispatch() {return static_cast<IDispatch *>(this);} \
	virtual const IDispatch *GetDispatch() const {return static_cast<const IDispatch *>(this);} \
	virtual IEditable *GetIEditable() {return static_cast<IEditable*>(this);} \
	virtual const IEditable *GetIEditable() const {return static_cast<const IEditable*>(this);} \
	virtual ISelect *GetISelect() {return static_cast<ISelect*>(this);} \
	virtual const ISelect *GetISelect() const {return static_cast<const ISelect*>(this);} \
	virtual Hitable *GetIHitable() {return static_cast<Hitable *>(this);} \
	virtual const Hitable *GetIHitable() const {return static_cast<const Hitable *>(this);} \
	virtual IRenderable *GetIRenderable() { return static_cast<IRenderable *>(this); } \
	virtual const IRenderable *GetIRenderable() const { return static_cast<const IRenderable *>(this); } \
	STDMETHOD(GetDisplayString)(DISPID dispID, BSTR * pbstr) { return ResultFromScode(E_NOTIMPL); } \
	STDMETHOD(MapPropertyToPage)(DISPID dispID, CLSID * pclsid) { return ResultFromScode(E_NOTIMPL); } \
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
      IFireEvents * fe = GetIFireEvents(); if (fe) { m_phittimer = std::make_unique<HitTimer>(GetName(), m_d.m_tdr.m_TimerInterval, fe); if (m_d.m_tdr.m_TimerEnabled) pvht.push_back(m_phittimer.get()); } \
	} \
	virtual void TimerRelease() { m_phittimer = nullptr; } \
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

#define STANDARD_EDITABLE_COPY_FOR_PLAY_IMPL(type) \
   type *dst = type::COMCreate(); \
   dst->Init(0.f, 0.f, false, true); \
   dst->m_wzName = m_wzName; \
   dst->m_desktopBackdrop = m_desktopBackdrop; \
   dst->m_uiLocked = m_uiLocked; \
   dst->m_uiVisible = m_uiVisible; \
   dst->m_d = m_d;

#define STANDARD_EDITABLE_WITH_DRAGPOINT_COPY_FOR_PLAY_IMPL(type, points) \
   STANDARD_EDITABLE_COPY_FOR_PLAY_IMPL(type) \
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


// Each IEditable manage a data block named m_d. All data block derive from these BaseProperty
// FIXME Not everything in here is used in all of the derived classes, it was made this way to simplify the Win32 UI code but this is fairly wrong and should be cleaned up
class BaseProperty
{
public:
   string m_szImage;
   string m_szMaterial;
   string m_szPhysicsMaterial;
   float m_elasticity;
   float m_friction;
   float m_scatter;
   float m_threshold;
   bool m_collidable;
   bool m_hitEvent = false;
   bool m_overwritePhysics = true;
   bool m_reflectionEnabled = true;
   bool m_visible = true;
};

// IEditable is the interface for self-contained table element.
// Example: Bumper is an IEditable and ISelect, but DragPoint is only ISelect.
class IEditable
{
public:
   IEditable();
   virtual ~IEditable();

   virtual ULONG STDMETHODCALLTYPE AddRef() = 0;
   virtual ULONG STDMETHODCALLTYPE Release() = 0;

   virtual PinTable *GetPTable() = 0;
   virtual const PinTable *GetPTable() const = 0;

   virtual ISelect *GetISelect() = 0;
   virtual const ISelect *GetISelect() const = 0;

   virtual Hitable *GetIHitable() = 0;
   virtual const Hitable *GetIHitable() const = 0;

   virtual IRenderable *GetIRenderable() = 0;
   virtual const IRenderable *GetIRenderable() const = 0;

   virtual IScriptable *GetScriptable() = 0;
   virtual const IScriptable *GetScriptable() const = 0;

   virtual IFireEvents *GetIFireEvents() = 0;

   virtual ItemTypeEnum GetItemType() const = 0;

   virtual void SetDefaults(const bool fromMouseClick) = 0;
   virtual void WriteRegDefaults() = 0;

   virtual void Save(IObjectWriter &writer, const bool saveForUndo) = 0;
   virtual void ClearForOverwrite() { }
   virtual void Load(IObjectReader &partReader) = 0;

   // if legacy_bounds != nullptr, can return pre-10.8 bounds, too (depending on which editable exactly)
   virtual void GetBoundingVertices(vector<Vertex3Ds> &bounds, vector<Vertex3Ds> *const legacy_bounds) { }

   virtual void ExportMesh(class ObjLoader &loader) { }

   // Shared implementation
protected:
   void LoadSharedEditableField(const int id, IObjectReader &reader);
   void SaveSharedEditableFields(IObjectWriter &writer);

public:
   wstring m_onLoadExpectedPartGroup; // Name of the part group, this object expects to be added to. Defined when loading a part (should be moved to the loading context)

   // Report a change that would need the Win32 UI to be redrawn
   // FIXME move to ISelect
   virtual void SetDirtyDraw();

   virtual void BeginUndo();
   virtual void EndUndo();
   virtual void Delete();
   virtual void Uncreate();

   void MarkForUndo();
   void MarkForDelete();
   void Undelete();

   string GetName() const;
   const wstring& GetWName() const;
   void SetName(const wstring& name);

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

   PinTable *m_ptable = nullptr; // This may only be set by PinTable in AddPart/RemovePart (guarantee reverse reference with table's list of parts)

   bool m_desktopBackdrop = false; // if true, the element is part of the desktop backdrop

   bool m_uiLocked = false; // Can not be dragged in the editor

   bool m_uiVisible = true; // UI visibility (not the same as rendering visibility which is a member of part data)

private:
   VARIANT m_uservalue;

   class PartGroup* m_partGroup = nullptr; // Parenting to group (or top level layers) for base transform and visibility

#pragma region Script events
public:
   std::unique_ptr<HitTimer> m_phittimer; // timer event defined when playing (between TimerSetup and TimerRelease)

   // In game filtered copy of m_vCollection/m_viCollection for slightly faster event dispatching
   vector<Collection *> m_vEventCollection;
   vector<int> m_viEventCollection;
   bool m_singleEvents;
#pragma endregion
};
