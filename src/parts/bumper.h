// Bumper.h: Definition of the Bumper class
//
//////////////////////////////////////////////////////////////////////
#pragma once
#if !defined(AFX_BUMPER_H__9A202FF0_7FAE_49BF_AA4C_C01C692E6DD9__INCLUDED_)
#define AFX_BUMPER_H__9A202FF0_7FAE_49BF_AA4C_C01C692E6DD9__INCLUDED_

#include "resource.h"       // main symbols

class BumperData final : public BaseProperty
{
public:
   Vertex2D m_vCenter;
   float m_radius;
   float m_force; // force the bumper kicks back with
   float m_heightScale;
   float m_orientation;
   float m_ringSpeed;
   float m_ringDropOffset;
   TimerDataRoot m_tdr;
   string m_szCapMaterial;
   string m_szBaseMaterial;
   string m_szSkirtMaterial;
   string m_szRingMaterial;
   string m_szSurface;
   bool m_capVisible;
   bool m_baseVisible;
   bool m_ringVisible;
   bool m_skirtVisible;
};

/////////////////////////////////////////////////////////////////////////////
// Bumper

class Bumper :
   //public CComObjectRootEx<CComSingleThreadModel>,
   public IDispatchImpl<IBumper, &IID_IBumper, &LIBID_VPinballLib>,
   //public ISupportErrorInfo,
   public CComObjectRoot,
   public CComCoClass<Bumper, &CLSID_Bumper>,
   public EventProxy<Bumper, &DIID_IBumperEvents>,
   public IConnectionPointContainerImpl<Bumper>,
   public IProvideClassInfo2Impl<&CLSID_Bumper, &DIID_IBumperEvents, &LIBID_VPinballLib>,
   public ISelect,
   public IEditable,
   public Hitable,
   public IScriptable,
   public IFireEvents,
   public IPerPropertyBrowsing // Ability to fill in dropdown in property browser
   //public EditableImpl<Bumper>
{
public:
   Bumper();
   ~Bumper();

   BEGIN_COM_MAP(Bumper)
      COM_INTERFACE_ENTRY(IDispatch)
      COM_INTERFACE_ENTRY(IBumper)
      //COM_INTERFACE_ENTRY(ISupportErrorInfo)
      COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
      COM_INTERFACE_ENTRY(IPerPropertyBrowsing)
      COM_INTERFACE_ENTRY(IProvideClassInfo)
      COM_INTERFACE_ENTRY(IProvideClassInfo2)
   END_COM_MAP()
   //DECLARE_NOT_AGGREGATABLE(Bumper)
   // Remove the comment from the line above if you don't want your object to
   // support aggregation.

   STANDARD_EDITABLE_DECLARES(Bumper, eItemBumper, BUMPER, 1)

   BEGIN_CONNECTION_POINT_MAP(Bumper)
      CONNECTION_POINT_ENTRY(DIID_IBumperEvents)
   END_CONNECTION_POINT_MAP()

   DECLARE_REGISTRY_RESOURCEID(IDR_BUMPER)

   // ISupportsErrorInfo
   STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

   void MoveOffset(const float dx, const float dy) final;
   void SetObjectPos() final;

   // Multi-object manipulation
   Vertex2D GetCenter() const final;
   void PutCenter(const Vertex2D &pv) final;

   void SetDefaultPhysics(const bool fromMouseClick) final;
   void ExportMesh(ObjLoader &loader) final;
   void RenderBlueprint(Sur *psur, const bool solid) final;

   ItemTypeEnum HitableGetItemType() const final { return eItemBumper; }

   void WriteRegDefaults() final;

   Bumper *CopyForPlay(PinTable *live_table);

   BumperData m_d;

   BumperHitCircle *m_pbumperhitcircle = nullptr;

private:
   void UpdateSkirt(const bool doCalculation);
   void GenerateBaseMesh(Vertex3D_NoTex2 *buf);
   void GenerateSocketMesh(Vertex3D_NoTex2 *buf);
   void GenerateRingMesh(Vertex3D_NoTex2 *buf);
   void GenerateCapMesh(Vertex3D_NoTex2 *buf);

   PinTable *m_ptable;

   RenderDevice *m_rd = nullptr;
   MeshBuffer *m_baseMeshBuffer = nullptr;
   MeshBuffer *m_socketMeshBuffer = nullptr;
   MeshBuffer *m_ringMeshBuffer = nullptr;
   MeshBuffer *m_capMeshBuffer = nullptr;

   Matrix3D m_fullMatrix;
   Vertex3D_NoTex2 *m_ringVertices = nullptr;
   Texture m_ringTexture;
   Texture m_skirtTexture;
   Texture m_baseTexture;
   Texture m_capTexture;

   PropertyPane *m_propVisual = nullptr;

   float   m_baseHeight;
   float   m_skirtCounter = 0.0f;
   bool    m_updateSkirt = false;
   bool    m_doSkirtAnimation = false;
   bool    m_enableSkirtAnimation = true;
   bool    m_ringDown = false;
   bool    m_ringAnimate = false;

   float   m_rotx = 0.0f;
   float   m_roty = 0.0f;

   // IBumper
public:
   STDMETHOD(get_BaseMaterial)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_BaseMaterial)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_SkirtMaterial)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_SkirtMaterial)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_Surface)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_Surface)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_Y)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Y)(/*[in]*/ float newVal);
   STDMETHOD(get_X)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_X)(/*[in]*/ float newVal);
   STDMETHOD(get_CapMaterial)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_CapMaterial)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_RingMaterial)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_RingMaterial)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_Threshold)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Threshold)(/*[in]*/ float newVal);
   STDMETHOD(get_Force)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Force)(/*[in]*/ float newVal);
   STDMETHOD(get_HeightScale)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_HeightScale)(/*[in]*/ float newVal);
   STDMETHOD(get_RingSpeed)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_RingSpeed)(/*[in]*/ float newVal);
   STDMETHOD(get_Orientation)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Orientation)(/*[in]*/ float newVal);
   STDMETHOD(get_RingDropOffset)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_RingDropOffset)(/*[in]*/ float newVal);
   STDMETHOD(get_CurrentRingOffset)(/*[out, retval]*/ float *pVal);
   STDMETHOD(get_Radius)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Radius)(/*[in]*/ float newVal);
   STDMETHOD(get_HasHitEvent)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_HasHitEvent)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_Collidable)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_Collidable)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_CapVisible)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_CapVisible)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_BaseVisible)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_BaseVisible)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_RingVisible)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_RingVisible)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_SkirtVisible)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_SkirtVisible)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_ReflectionEnabled)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_ReflectionEnabled)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_Scatter)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Scatter)(/*[in]*/ float newVal);
   STDMETHOD(get_EnableSkirtAnimation)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_EnableSkirtAnimation)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_RotX)(/*[out, retval]*/ float *pVal);
   STDMETHOD(get_RotY)(/*[out, retval]*/ float *pVal);
   STDMETHOD(PlayHit)();
};

#endif // !defined(AFX_BUMPER_H__9A202FF0_7FAE_49BF_AA4C_C01C692E6DD9__INCLUDED_)
