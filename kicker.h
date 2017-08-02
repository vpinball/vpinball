// Kicker.h: Definition of the Kicker class
//
//////////////////////////////////////////////////////////////////////
#pragma once
#if !defined(AFX_KICKER_H__3A9F3FC4_605A_43AD_A430_830279CFE059__INCLUDED_)
#define AFX_KICKER_H__3A9F3FC4_605A_43AD_A430_830279CFE059__INCLUDED_

#include "resource.h"       // main symbols

class KickerData
{
public:
   Vertex2D m_vCenter;
   float m_radius;
   char m_szMaterial[32];
   TimerDataRoot m_tdr;
   char m_szSurface[MAXTOKEN];
   KickerType m_kickertype;
   float m_scatter;
   float m_hitAccuracy; //kicker hit grabbing object height ... default ballsize*0.7
   float m_orientation;
   float m_hit_height;
   bool m_fEnabled;
   bool m_fFallThrough;
   bool m_legacyMode;
};

class KickerHitCircle;

/////////////////////////////////////////////////////////////////////////////
// Kicker

class Kicker :
   public IDispatchImpl<IKicker, &IID_IKicker, &LIBID_VPinballLib>,
   //public ISupportErrorInfo,
   public CComObjectRoot,
   public CComCoClass<Kicker, &CLSID_Kicker>,
   public EventProxy<Kicker, &DIID_IKickerEvents>,
   public IConnectionPointContainerImpl<Kicker>,
   public IProvideClassInfo2Impl<&CLSID_Kicker, &DIID_IKickerEvents, &LIBID_VPinballLib>,
   public ISelect,
   public IEditable,
   public Hitable,
   public IScriptable,
   public IFireEvents,
   public IPerPropertyBrowsing // Ability to fill in dropdown in property browser
{
public:
   Kicker();
   virtual ~Kicker();

   BEGIN_COM_MAP(Kicker)
      COM_INTERFACE_ENTRY(IDispatch)
      COM_INTERFACE_ENTRY(IKicker)
      //COM_INTERFACE_ENTRY(ISupportErrorInfo)
      COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
      COM_INTERFACE_ENTRY(IPerPropertyBrowsing)
      COM_INTERFACE_ENTRY(IProvideClassInfo)
      COM_INTERFACE_ENTRY(IProvideClassInfo2)
   END_COM_MAP()
   //DECLARE_NOT_AGGREGATABLE(Kicker) 
   // Remove the comment from the line above if you don't want your object to 
   // support aggregation.

   BEGIN_CONNECTION_POINT_MAP(Kicker)
      CONNECTION_POINT_ENTRY(DIID_IKickerEvents)
   END_CONNECTION_POINT_MAP()

   STANDARD_EDITABLE_DECLARES(Kicker, eItemKicker, KICKER, 1)

   DECLARE_REGISTRY_RESOURCEID(IDR_KICKER)
   // ISupportsErrorInfo
   STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

   virtual void GetDialogPanes(Vector<PropertyPane> *pvproppane);

   virtual void MoveOffset(const float dx, const float dy);
   virtual void SetObjectPos();
   // Multi-object manipulation
   virtual void GetCenter(Vertex2D * const pv) const;
   virtual void PutCenter(const Vertex2D * const pv);

   virtual void PreRenderStatic(RenderDevice* pd3dDevice);
   virtual void SetDefaultPhysics(bool fromMouseClick);
   virtual void ExportMesh(FILE *f);

   virtual unsigned long long GetMaterialID() { return m_ptable->GetMaterial(m_d.m_szMaterial)->hash(); }
   virtual ItemTypeEnum HitableGetItemType() { return eItemKicker; }

   void WriteRegDefaults();
   void GenerateCupMesh(Vertex3D_NoTex2 *buf);
   void GenerateHoleMesh(Vertex3D_NoTex2 *buf);
   void GenerateSimpleHoleMesh(Vertex3D_NoTex2 *buf);

   PinTable *m_ptable;

   KickerData m_d;

   std::vector<Vertex3Ds> hitMesh;
   KickerHitCircle *m_phitkickercircle;

   VertexBuffer *vertexBuffer;
   IndexBuffer  *indexBuffer;
   unsigned int numVertices;
   unsigned int numIndices;

   Texture texture;

   float  m_baseHeight;

   VertexBuffer *plateVertexBuffer;
   IndexBuffer *plateIndexBuffer;
   // IKicker
public:
   STDMETHOD(get_Material)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_Material)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_DrawStyle)(/*[out, retval]*/ KickerType *pVal);
   STDMETHOD(put_DrawStyle)(/*[in]*/ KickerType newVal);
   STDMETHOD(get_Enabled)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_Enabled)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_Surface)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_Surface)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_Y)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Y)(/*[in]*/ float newVal);
   STDMETHOD(get_X)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_X)(/*[in]*/ float newVal);
   STDMETHOD(Kick)(float angle, float speed, float inclination);
   STDMETHOD(KickZ)(float angle, float speed, float inclination, float heightz);
   STDMETHOD(KickXYZ)(float angle, float speed, float inclination, float x, float y, float z);
   //STDMETHOD(DestroyBall)();
   STDMETHOD(DestroyBall)(/*[out, retval]*/ int *pVal);
   STDMETHOD(CreateBall)(IBall **pBallEx);
   STDMETHOD(CreateSizedBall)(/*[in]*/float radius, /*out, retval]*/ IBall **pBallEx);
   STDMETHOD(CreateSizedBallWithMass)(/*[in]*/float radius, /*[in]*/float mass, /*out, retval]*/ IBall **pBallEx);
   STDMETHOD(BallCntOver)(/*[out, retval]*/ int *pVal);

   STDMETHOD(get_Scatter)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Scatter)(/*[in]*/ float newVal);
   STDMETHOD(get_HitAccuracy)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_HitAccuracy)(/*[in]*/ float newVal);
   STDMETHOD(get_HitHeight)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_HitHeight)(/*[in]*/ float newVal);
   STDMETHOD(get_Orientation)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Orientation)(/*[in]*/ float newVal);
   STDMETHOD(get_Radius)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Radius)(/*[in]*/ float newVal);
   STDMETHOD(get_FallThrough)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_FallThrough)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_Legacy)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_Legacy)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_LastCapturedBall)(/*[out, retval]*/ IBall **pVal);
   };

class KickerHitCircle : public HitCircle
{
public:
   KickerHitCircle(const Vertex2D& c, float r, float zlow, float zhigh)
      : HitCircle(c,r,zlow,zhigh)
   {
      m_pball = NULL;
      m_lastCapturedBall = NULL;
      m_pkicker = NULL;
   }

   virtual float HitTest(const Ball * const pball, const float dtime, CollisionEvent& coll);
   virtual int GetType() const { return eTrigger; }
   virtual void Collide(CollisionEvent& coll)  { DoCollide(coll.ball, coll.hitnormal, coll.hitflag, false); }

   void DoChangeBallVelocity(Ball * const pball, const Vertex3Ds& hitnormal);
   void DoCollide(Ball * const pball, const Vertex3Ds& hitnormal, const bool hitflag, const bool newBall);

   Kicker *m_pkicker;
   Ball *m_pball;  //The ball inside this kicker
   Ball *m_lastCapturedBall; // same as m_pball but this one won't be nulled only overwritten from another captured ball
};

#endif // !defined(AFX_KICKER_H__3A9F3FC4_605A_43AD_A430_830279CFE059__INCLUDED_)
