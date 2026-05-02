// license:GPLv3+

#pragma once

#include "utils/fileio.h"

class HitTimer;
class Collection;
class IHitable;
class IRenderable;
class IScriptable;

#define BLUEPRINT_SOLID_COLOR RGB(0,0,0)

class IFireEvents
{
public:
   virtual void FireGroupEvent(const int dispid) = 0;
   virtual IDispatch *GetIDispatch() = 0;
   virtual const IDispatch *GetIDispatch() const = 0;

   float   m_currentHitThreshold; // while playing and the ball hits the mesh the hit threshold is updated here
};

#define STARTUNDO \
	BeginUndo(); \
	MarkForUndo();

#define STOPUNDO \
	EndUndo(); \
	if (GetPTable()) GetPTable()->SetDirtyDraw();

#define STARTUNDOSELECT \
	GetIEditable()->BeginUndo(); \
	GetIEditable()->MarkForUndo();

#define STOPUNDOSELECT \
	GetIEditable()->EndUndo(); \
	if (GetPTable()) GetPTable()->SetDirtyDraw();


// Explanation for AllowedViews:
// Value gets and'ed with 1 (table view) or 2 (backglass view).
// If you want to allow an element to be pasted only into the table view, use 1,
// for only backglass view, use 2, and for both, use 3.

// declare and implement some methods for an IEditable which supports scripting
#define STANDARD_EDITABLE_DECLARES(T, ItemType, ResName, AllowedViews) \
	_STANDARD_EDITABLE_CONSTANTS(ItemType, ResName, AllowedViews) \
	_STANDARD_DISPATCH_INDEPENDENT_EDITABLE_DECLARES(T, ItemType) \
	_STANDARD_DISPATCH_INDEPENDENT_EDITABLE_HITABLE_DECLARES(T, ItemType) \
	_STANDARD_DISPATCH_INDEPENDENT_EDITABLE_RENDERABLE_DECLARES(T, ItemType) \
	_STANDARD_DISPATCH_EDITABLE_DECLARES(ItemType)

#define STANDARD_EDITABLE_DECLARES_NO_RENDERABLE(T, ItemType, ResName, AllowedViews) \
	_STANDARD_EDITABLE_CONSTANTS(ItemType, ResName, AllowedViews) \
	_STANDARD_DISPATCH_INDEPENDENT_EDITABLE_DECLARES(T, ItemType) \
	_STANDARD_DISPATCH_INDEPENDENT_EDITABLE_HITABLE_DECLARES(T, ItemType) \
	_STANDARD_DISPATCH_EDITABLE_DECLARES(ItemType) \
	IRenderable *GetIRenderable() final { return nullptr; } \
	const IRenderable *GetIRenderable() const final { return nullptr; }

#define STANDARD_EDITABLE_DECLARES_NO_HITABLE(T, ItemType, ResName, AllowedViews) \
	_STANDARD_EDITABLE_CONSTANTS(ItemType, ResName, AllowedViews) \
	_STANDARD_DISPATCH_INDEPENDENT_EDITABLE_DECLARES(T, ItemType) \
	_STANDARD_DISPATCH_INDEPENDENT_EDITABLE_RENDERABLE_DECLARES(T, ItemType) \
	_STANDARD_DISPATCH_EDITABLE_DECLARES(ItemType) \
	IHitable *GetIHitable() final { return nullptr; } \
	const IHitable *GetIHitable() const final { return nullptr; }

#define STANDARD_EDITABLE_DECLARES_NO_RENDERABLE_NO_HITABLE(T, ItemType, ResName, AllowedViews) \
	_STANDARD_EDITABLE_CONSTANTS(ItemType, ResName, AllowedViews) \
	_STANDARD_DISPATCH_INDEPENDENT_EDITABLE_DECLARES(T, ItemType) \
	_STANDARD_DISPATCH_EDITABLE_DECLARES(ItemType) \
	IRenderable* GetIRenderable() final { return nullptr; } \
	const IRenderable* GetIRenderable() const final { return nullptr; } \
	IHitable *GetIHitable() final { return nullptr; } \
	const IHitable *GetIHitable() const final { return nullptr; }

// used above, do not invoke directly
#define _STANDARD_DISPATCH_EDITABLE_DECLARES(itemType) \
	IFireEvents *GetIFireEvents() final {return (IFireEvents *)this;} \
	EventProxyBase *GetEventProxyBase() final {return (EventProxyBase *)this;} \
	STDMETHOD(get_Name)(/*[out, retval]*/ BSTR *pVal) { *pVal = SysAllocStringLen(m_wzName.c_str(), static_cast<UINT>(m_wzName.length())); return S_OK; } \
	STDMETHOD(put_Name)(/*[in]*/ BSTR newVal) { SetName(newVal); return S_OK; } \
	STDMETHOD(get_TimerInterval)(/*[out, retval]*/ LONG *pVal) {*pVal = m_timerInterval; return S_OK;} \
	STDMETHOD(put_TimerInterval)(/*[in]*/ LONG newVal) {return IEditable::put_TimerInterval(newVal, &m_timerInterval);} \
	STDMETHOD(get_TimerEnabled)(/*[out, retval]*/ VARIANT_BOOL *pVal) {*pVal = FTOVB(m_timerEnabled); return S_OK;} \
	STDMETHOD(put_TimerEnabled)(/*[in]*/ VARIANT_BOOL newVal) {BOOL tmp = m_timerEnabled ? 1 : 0; const HRESULT res = IEditable::put_TimerEnabled(newVal, &tmp); m_timerEnabled = (tmp != 0); return res;} \
	STDMETHOD(get_UserValue)(VARIANT *pVal) {return IEditable::get_UserValue(pVal);} \
	STDMETHOD(put_UserValue)(VARIANT *newVal) {return IEditable::put_UserValue(newVal);} \
	IScriptable *GetIScriptable() final {return (IScriptable *)this;} \
	const IScriptable *GetIScriptable() const final {return (const IScriptable *)this;} \
	void FireGroupEvent(const int dispid) final {FireVoidGroupEvent(dispid);}

// used above, do not invoke directly
#define _STANDARD_DISPATCH_INDEPENDENT_EDITABLE_DECLARES(T, ItemType) \
    static T* COMCreate() \
    { \
        CComObject<T> *obj = nullptr; \
        if (FAILED(CComObject<T>::CreateInstance(&obj))) \
            ShowError("Failed to create COM object."); \
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
	T *CopyForPlay() const final; \
	HRESULT Init(const float x, const float y, const bool fromMouseClick, const bool forPlay = false); \
	void UIRenderPass1(Sur * const psur) final; \
	void UIRenderPass2(Sur * const psur) final; \
	bool IsUILocked() const final { return m_uiLocked; } \
	void SetUILock(bool lock) final { m_uiLocked = lock; } \
	bool IsUIVisible() const final { return m_uiVisible; } \
	void SetUIVisible(bool visible) final { m_uiVisible = visible; } \
	PinTable *GetPTable() final { return m_ptable; } \
	const PinTable *GetPTable() const final { return m_ptable; } \
	void Delete() final {IEditable::Delete();} \
	void Uncreate() final {IEditable::Uncreate();} \
	void Load(IObjectReader &reader) final; \
	void Save(IObjectWriter &writer, const bool saveForUndo) final; \
	ItemTypeEnum GetItemType() const final { return ItemType; } \
	IDispatch *GetIDispatch() final {return static_cast<IDispatch *>(this);} \
	const IDispatch *GetIDispatch() const final {return static_cast<const IDispatch *>(this);} \
	IEditable *GetIEditable() final {return static_cast<IEditable*>(this);} \
	const IEditable *GetIEditable() const final {return static_cast<const IEditable*>(this);} \
	ISelect *GetISelect() final {return static_cast<ISelect*>(this);} \
	const ISelect *GetISelect() const final {return static_cast<const ISelect*>(this);} \
	STDMETHOD(GetDisplayString)(DISPID dispID, BSTR * pbstr) { return ResultFromScode(E_NOTIMPL); } \
	STDMETHOD(MapPropertyToPage)(DISPID dispID, CLSID * pclsid) { return ResultFromScode(E_NOTIMPL); } \
	STDMETHOD(GetPredefinedStrings)(DISPID dispID, CALPOLESTR *pcaStringsOut, CADWORD *pcaCookiesOut) {return GetPTable()->GetPredefinedStrings(dispID, pcaStringsOut, pcaCookiesOut, this);} \
	STDMETHOD(GetPredefinedValue)(DISPID dispID, DWORD dwCookie, VARIANT *pVarOut) {return GetPTable()->GetPredefinedValue(dispID, dwCookie, pVarOut, this);} \
	void SetDefaults(const bool fromMouseClick) final;

// used above, do not invoke directly
#define _STANDARD_DISPATCH_INDEPENDENT_EDITABLE_HITABLE_DECLARES(T, ItemType) \
   IHitable *GetIHitable() final { return static_cast<IHitable *>(this); } \
   const IHitable *GetIHitable() const final { return static_cast<const IHitable *>(this); } \
   void PhysicSetup(PhysicsEngine *physics, const bool isUI) final; \
   void PhysicRelease(PhysicsEngine* physics, const bool isUI) final;

// used above, do not invoke directly
#define _STANDARD_DISPATCH_INDEPENDENT_EDITABLE_RENDERABLE_DECLARES(T, ItemType) \
   IRenderable *GetIRenderable() final { return static_cast<IRenderable *>(this); } \
   const IRenderable *GetIRenderable() const final { return static_cast<const IRenderable *>(this); } \
   void RenderSetup(RenderDevice *device) final; \
   void UpdateAnimation(const float diff_time_msec) final; \
   void Render(const unsigned int renderMask) final; \
   void RenderRelease() final;

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
   dst->m_d = m_d; \
   dst->m_timerInterval = m_timerInterval; \
   dst->m_timerEnabled = m_timerEnabled;

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

   virtual IHitable *GetIHitable() = 0;
   virtual const IHitable *GetIHitable() const = 0;

   virtual IRenderable *GetIRenderable() = 0;
   virtual const IRenderable *GetIRenderable() const = 0;

   virtual IScriptable *GetIScriptable() = 0;
   virtual const IScriptable *GetIScriptable() const = 0;

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

   virtual IEditable *CopyForPlay() const = 0;

   virtual EventProxyBase *GetEventProxyBase() = 0;

   // Shared implementation
protected:
   void LoadSharedEditableField(const int id, IObjectReader &reader);
   void SaveSharedEditableFields(IObjectWriter &writer);

public:
   wstring m_onLoadExpectedPartGroup; // Name of the part group, this object expects to be added to. Defined when loading a part (should be moved to the loading context)

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
   void TimerSetup(vector<HitTimer *> &pvht);
   void TimerRelease(vector<HitTimer *> &pvht);

   // was: TimerDataRoot m_tdr: then it was limited to be used by a limited amount of table elements (most prominently not by decals and primitives though), could be changed/generalized nowadays
   int m_timerInterval = 100;
   bool m_timerEnabled = false;

   std::unique_ptr<HitTimer> m_phittimer; // timer event defined when playing (between TimerSetup and TimerRelease)

   // In game filtered copy of m_vCollection/m_viCollection for slightly faster event dispatching
   vector<Collection *> m_vEventCollection;
   vector<int> m_viEventCollection;
   bool m_singleEvents;
#pragma endregion
};
