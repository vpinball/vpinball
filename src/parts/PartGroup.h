// license:GPLv3+

#pragma once

#include "core/ieditable.h"
#include "core/resourceid.h"
#include "core/Scriptable.h"
#include "utils/eventproxy.h"
#include "utils/fileio.h"
#include "timer.h"

class PartGroupData final
{
public:
   // Standard properties
   Vertex2D m_v;

   // PartGroup properties
   enum PlayerModeVisibilityMask
   {
      PMVM_DESKTOP         = 0x0001, // Enable if player is in desktop mode (not VR/AR, played on a landscape device, showing desktop backdrop)
      PMVM_FSS             = 0x0002, // Enable if player is in full single screen mode (not VR/AR, played on a portrait device showing cab & backglass)
      PMVM_CABINET         = 0x0004, // Enable if player is in cabinet mode (not VR/AR, played on a pincab device)
      PMVM_MIXED_REALITY   = 0x0008, // Enable if player is in AR mode
      PMVM_VIRTUAL_REALITY = 0x0010, // Enable if player is in VR mode
      PMVM_ALL             = 0xFFFF,
   };
   unsigned int m_playerModeVisibilityMask = PMVM_ALL;
   enum class SpaceReference : int
   {
      Inherit = 0, // Inherit space reference from parent (note that root defaults to Playfield reference space)
      Playfield = 1, // Relative to the room with scaling, height adjustment and playfield inclination applied (table design coordinate system, including the cabinet which moves with it)
      Room = 2, // Base space, aligned to (offsetted) real world, without any scaling (to match real world room in AR/VR)
   };
   SpaceReference m_spaceReference = SpaceReference::Inherit;
};

class PartGroup :
   public CComObjectRootEx<CComSingleThreadModel>,
   public IDispatchImpl<IPartGroup, &IID_IPartGroup, &LIBID_VPinballLib>,
   //public ISupportErrorInfo,
   public CComCoClass<PartGroup, &CLSID_PartGroup>,
   public EventProxy<PartGroup, &DIID_IPartGroupEvents>,
   public IConnectionPointContainerImpl<PartGroup>,
   public IProvideClassInfo2Impl<&CLSID_PartGroup, &DIID_IPartGroupEvents, &LIBID_VPinballLib>,
   public IEditable,
   public IScriptable,
   public IFireEvents,
   public IPerPropertyBrowsing // Ability to fill in dropdown(s) in property browser
{
public:
#ifdef VPX_MANUAL_SCRIPT_DISPATCH
   STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid,DISPID* rgDispId);
   STDMETHOD(Invoke)(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);
   STDMETHOD(GetDocumentation)(MEMBERID index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile);
   HRESULT FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) final;
#endif
   PartGroup() { }
   virtual ~PartGroup() { }

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

   STANDARD_EDITABLE_DECLARES_NO_RENDERABLE_NO_HITABLE(PartGroup, eItemPartGroup, PARTGROUP)

   void Translate(const Vertex2D& offset) final;
   Vertex2D GetCenter() const final;

   void WriteRegDefaults() final;

   DECLARE_REGISTRY_RESOURCEID(IDR_PARTGROUP)

   // ISupportsErrorInfo
   STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

   // IPartGroup

   unsigned int GetPlayerModeVisibilityMask() const;
   PartGroupData::SpaceReference GetReferenceSpace() const;

   PartGroupData m_d;
};
