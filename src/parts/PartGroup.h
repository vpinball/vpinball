// license:GPLv3+

#pragma once

#include "ui/resource.h"

class PartGroupData final
{
public:
   // Standard properties
   TimerDataRoot m_tdr;
   Vertex2D m_v;

   // PartGroup properties
   enum VisibilityMask
   {
      VM_PLAYFIELD       = 0x0001,
      VM_MIXED_REALITY   = 0x0002,
      VM_VIRTUAL_REALITY = 0x0004,
      VM_SCOREVIEW       = 0x0008, // Reserved (Not yet used)
      VM_BACKGLASS       = 0x0010, // Reserved (Not yet used)
      VM_TOPPER          = 0x0020, // Reserved (Not yet used)
      VM_APRON_LEFT      = 0x0040, // Reserved (Not yet used)
      VM_APRON_RIGHT     = 0x0080, // Reserved (Not yet used)
      VM_ALL             = 0xFFFF,
   };
   unsigned int m_visibilityMask = VM_ALL;
   enum class SpaceReference : int
   {
      SR_INHERIT, // Inherit space reference from parent (note that root defaults to Playfield reference space)
      SR_ROOM, // Base space, aligned to (offset'ed) real world, without any scaling (to match real world room in AR/VR)
      SR_CABINET_FEET, // Relative to room, scaled to fit cabinet size (without any height adjustment, for cabinet feet to touch ground)
      SR_CABINET, // Relative to cabinet feet, with height adjustment (with height adjustment for lockbar ro match cabinet lockbar height after scaling)
      SR_PLAYFIELD, // Relative to cabinet with playfield inclination and local coordinate system applied (usual local playfield coordinate system tailored for table design)
   };
   SpaceReference m_spaceReference = SpaceReference::SR_PLAYFIELD;
};

class PartGroup :
   public CComObjectRootEx<CComSingleThreadModel>,
   public IDispatchImpl<IPartGroup, &IID_IPartGroup, &LIBID_VPinballLib>,
   //public ISupportErrorInfo,
   //public CComObjectRoot,
   public CComCoClass<PartGroup, &CLSID_PartGroup>,
   public EventProxy<PartGroup, &DIID_IPartGroupEvents>,
   public IConnectionPointContainerImpl<PartGroup>,
   public IProvideClassInfo2Impl<&CLSID_PartGroup, &DIID_IPartGroupEvents, &LIBID_VPinballLib>,
   public ISelect,
   public IEditable,
   public IScriptable,
   public IFireEvents,
   public Hitable,
   public IPerPropertyBrowsing     // Ability to fill in dropdown(s) in property browser
{
public:
#ifdef __STANDALONE__
   STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid,DISPID* rgDispId);
   STDMETHOD(Invoke)(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);
   STDMETHOD(GetDocumentation)(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile);
   HRESULT FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) final;
#endif
   PartGroup();
   virtual ~PartGroup();

   BEGIN_COM_MAP(PartGroup)
      COM_INTERFACE_ENTRY(IDispatch)
      COM_INTERFACE_ENTRY(IPartGroup)
      COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
      COM_INTERFACE_ENTRY(IProvideClassInfo)
      COM_INTERFACE_ENTRY(IProvideClassInfo2)
      COM_INTERFACE_ENTRY(IPerPropertyBrowsing)
   END_COM_MAP()

   BEGIN_CONNECTION_POINT_MAP(PartGroup)
      CONNECTION_POINT_ENTRY(DIID_IPartGroupEvents)
   END_CONNECTION_POINT_MAP()

   STANDARD_EDITABLE_DECLARES(PartGroup, eItemPartGroup, PARTGROUP, 3)

   void MoveOffset(const float dx, const float dy) final;
   void SetObjectPos() final;
   Vertex2D GetCenter() const final;
   void PutCenter(const Vertex2D& pv) final;

   void RenderBlueprint(Sur *psur, const bool solid) final;

   ItemTypeEnum HitableGetItemType() const final { return eItemPartGroup; }

   void WriteRegDefaults() final;

   DECLARE_REGISTRY_RESOURCEID(IDR_PARTGROUP)

   // ISupportsErrorInfo
   STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

   // IPartGroup

   unsigned int GetVisibilityMask() const;
   PartGroupData::SpaceReference GetReferenceSpace() const;

   PartGroupData m_d;

private:
   PinTable *m_ptable = nullptr;
};
