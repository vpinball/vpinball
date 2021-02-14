// Bumper.h: Definition of the Bumper class
//
//////////////////////////////////////////////////////////////////////
#pragma once
#if !defined(AFX_BUMPER_H__9A202FF0_7FAE_49BF_AA4C_C01C692E6DD9__INCLUDED_)
#define AFX_BUMPER_H__9A202FF0_7FAE_49BF_AA4C_C01C692E6DD9__INCLUDED_

#include "resource.h"       // main symbols

class BumperData : public BaseProperty
{
public:
   Vertex2D m_vCenter;
   float m_radius;
   float m_force; // force the bumper kicks back with
   float m_heightScale;
   float m_orientation;
   float m_ringSpeed;
   float m_ringDropOffset;
   U32 m_time_msec;
   TimerDataRoot m_tdr;
   std::string m_szCapMaterial;
   std::string m_szBaseMaterial;
   std::string m_szSkirtMaterial;
   std::string m_szRingMaterial;
   char m_szSurface[MAXTOKEN];
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
   
   virtual void MoveOffset(const float dx, const float dy);
   virtual void SetObjectPos();
   
   // Multi-object manipulation
   virtual Vertex2D GetCenter() const;
   virtual void PutCenter(const Vertex2D& pv);

   virtual void SetDefaultPhysics(bool fromMouseClick);
   virtual void ExportMesh(FILE *f);
   virtual void RenderBlueprint(Sur *psur, const bool solid);

   virtual unsigned long long GetMaterialID() const
   {
      if (!m_d.m_baseVisible && m_d.m_capVisible)
         return m_ptable->GetMaterial(m_d.m_szCapMaterial)->hash();
      else
         return 64-3; //!! some constant number
   }
   virtual unsigned long long GetImageID() const
   {
      if (!m_d.m_baseVisible && m_d.m_capVisible)
         return (unsigned long long)&m_capTexture; //!! meh
      else
         return NULL;
   }
   
   virtual ItemTypeEnum HitableGetItemType() const { return eItemBumper; }
   
   virtual void WriteRegDefaults();
   
   BumperData m_d;
   
   BumperHitCircle *m_pbumperhitcircle;

private:
   void RenderBase(const Material * const baseMaterial);
   void RenderCap(const Material * const capMaterial);
   void RenderSocket(const Material * const socketMaterial);
   void UpdateRing();
   void UpdateSkirt(const bool doCalculation);
   void GenerateBaseMesh(Vertex3D_NoTex2 *buf);
   void GenerateSocketMesh(Vertex3D_NoTex2 *buf);
   void GenerateRingMesh(Vertex3D_NoTex2 *buf);
   void GenerateCapMesh(Vertex3D_NoTex2 *buf);

   PinTable *m_ptable;

   VertexBuffer *m_baseVertexBuffer;
   IndexBuffer *m_baseIndexBuffer;

   VertexBuffer *m_socketVertexBuffer;
   IndexBuffer *m_socketIndexBuffer;

   VertexBuffer *m_ringVertexBuffer;
   IndexBuffer *m_ringIndexBuffer;

   VertexBuffer *m_capVertexBuffer;
   IndexBuffer *m_capIndexBuffer;

   Matrix3D m_fullMatrix;
   Vertex3D_NoTex2 *m_ringVertices;
   Texture m_ringTexture;
   Texture m_skirtTexture;
   Texture m_baseTexture;
   Texture m_capTexture;

   PropertyPane *m_propVisual;

   float   m_baseHeight;
   float   m_skirtCounter;
   bool    m_updateSkirt;
   bool    m_doSkirtAnimation;
   bool    m_enableSkirtAnimation;
   bool    m_ringDown;
   bool    m_ringAnimate;

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
   STDMETHOD(PlayHit)();
};

#endif // !defined(AFX_BUMPER_H__9A202FF0_7FAE_49BF_AA4C_C01C692E6DD9__INCLUDED_)
