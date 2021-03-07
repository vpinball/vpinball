// Trigger.h: Definition of the Trigger class
//
//////////////////////////////////////////////////////////////////////
#pragma once
#if !defined(AFX_TRIGGER_H__2CA7256C_4072_43C3_9D65_AE091B601377__INCLUDED_)
#define AFX_TRIGGER_H__2CA7256C_4072_43C3_9D65_AE091B601377__INCLUDED_

#include "resource.h"       // main symbols

class TriggerData : public BaseProperty
{
public:
   Vertex2D m_vCenter;
   float m_radius;
   TimerDataRoot m_tdr;
   char m_szSurface[MAXTOKEN];
   TriggerShape m_shape;
   float m_rotation;
   float m_scaleX;
   float m_scaleY;
   float m_hit_height; //trigger hit object height ... default 50
   U32 m_time_msec;
   float m_animSpeed;
   float m_wireThickness;
   bool m_enabled;
};

/////////////////////////////////////////////////////////////////////////////
// Trigger

class Trigger :
   public IDispatchImpl<ITrigger, &IID_ITrigger, &LIBID_VPinballLib>,
   //public ISupportErrorInfo,
   public CComObjectRoot,
   public CComCoClass<Trigger, &CLSID_Trigger>,
   public EventProxy<Trigger, &DIID_ITriggerEvents>,
   public IConnectionPointContainerImpl<Trigger>,
   public IProvideClassInfo2Impl<&CLSID_Trigger, &DIID_ITriggerEvents, &LIBID_VPinballLib>,
   public ISelect,
   public IEditable,
   public Hitable,
   public IScriptable,
   public IHaveDragPoints,
   public IFireEvents,
   public IPerPropertyBrowsing // Ability to fill in dropdown in property browser
{
public:
   Trigger();
   virtual ~Trigger();

   BEGIN_COM_MAP(Trigger)
      COM_INTERFACE_ENTRY(IDispatch)
      COM_INTERFACE_ENTRY(ITrigger)
      //COM_INTERFACE_ENTRY(ISupportErrorInfo)
      COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
      COM_INTERFACE_ENTRY(IPerPropertyBrowsing)
      COM_INTERFACE_ENTRY(IProvideClassInfo)
      COM_INTERFACE_ENTRY(IProvideClassInfo2)
   END_COM_MAP()
   //DECLARE_NOT_AGGREGATABLE(Trigger) 
   // Remove the comment from the line above if you don't want your object to 
   // support aggregation.

   BEGIN_CONNECTION_POINT_MAP(Trigger)
      CONNECTION_POINT_ENTRY(DIID_ITriggerEvents)
   END_CONNECTION_POINT_MAP()

   STANDARD_EDITABLE_DECLARES(Trigger, eItemTrigger, TRIGGER, 1)

      DECLARE_REGISTRY_RESOURCEID(IDR_TRIGGER)
   // ISupportsErrorInfo
   STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

   virtual void RenderBlueprint(Sur *psur, const bool solid);

   virtual void MoveOffset(const float dx, const float dy);
   virtual void SetObjectPos();

   virtual void EditMenu(CMenu &hmenu);
   virtual void DoCommand(int icmd, int x, int y);

   // Multi-object manipulation
   virtual void FlipY(const Vertex2D& pvCenter);
   virtual void FlipX(const Vertex2D& pvCenter);
   virtual void Rotate(const float ang, const Vertex2D& pvCenter, const bool useElementCenter);
   virtual void Scale(const float scalex, const float scaley, const Vertex2D& pvCenter, const bool useElementCenter);
   virtual void Translate(const Vertex2D &pvOffset);
   virtual Vertex2D GetCenter() const { return GetPointCenter(); }
   virtual Vertex2D GetScale() const { return Vertex2D(m_d.m_scaleX, m_d.m_scaleY); }
   virtual float GetRotate() const { return m_d.m_rotation; }

   virtual void PutCenter(const Vertex2D& pv) { PutPointCenter(pv); }
   virtual Vertex2D GetPointCenter() const;
   virtual void PutPointCenter(const Vertex2D& pv);
   virtual void ExportMesh(ObjLoader& loader);
   virtual ItemTypeEnum HitableGetItemType() const { return eItemTrigger; }
   virtual void UpdateStatusBarInfo();

   virtual unsigned long long GetMaterialID() const { return m_ptable->GetMaterial(m_d.m_szMaterial)->hash(); }

   virtual void ClearForOverwrite();

   virtual void WriteRegDefaults();

   void TriggerAnimationHit();
   void TriggerAnimationUnhit();

   TriggerData m_d;

   bool m_hitEnabled;		// for custom shape triggers

private:
   void CurvesToShapes(vector<HitObject*> &pvho);
   void AddLine(vector<HitObject*> &pvho, const RenderVertex &pv1, const RenderVertex &pv2, const float height);

   void InitShape(float x, float y);
   void UpdateAnimation();
   void GenerateMesh();

   PinTable *m_ptable;

   TriggerHitCircle *m_ptriggerhitcircle;

   VertexBuffer *m_vertexBuffer;
   IndexBuffer *m_triggerIndexBuffer;
   std::vector<Vertex3Ds> m_vertices;
   const WORD *m_faceIndices;
   Vertex3D_NoTex2 *m_triggerVertices;

   PropertyPane *m_propVisual;

   int m_numVertices;
   int m_numIndices;
   float m_animHeightOffset;
   float m_vertexBuffer_animHeightOffset;
   bool m_hitEvent;
   bool m_unhitEvent;
   bool m_doAnimation;
   bool m_moveDown;

// ITrigger
public:
   STDMETHOD(get_TriggerShape)(/*[out, retval]*/ TriggerShape *pVal);
   STDMETHOD(put_TriggerShape)(/*[in]*/ TriggerShape newVal);
   STDMETHOD(get_Visible)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_Visible)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_Enabled)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_Enabled)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_Surface)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_Surface)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_Y)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Y)(/*[in]*/ float newVal);
   STDMETHOD(get_X)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_X)(/*[in]*/ float newVal);
   STDMETHOD(get_Radius)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Radius)(/*[in]*/ float newVal);
   STDMETHOD(BallCntOver)(/*[out, retval]*/ int *pVal);
   STDMETHOD(DestroyBall)(/*[out, retval]*/ int *pVal);
   STDMETHOD(get_HitHeight)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_HitHeight)(/*[in]*/ float newVal);
   STDMETHOD(get_Rotation)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Rotation)(/*[in]*/ float newVal);
   STDMETHOD(get_WireThickness)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_WireThickness)(/*[in]*/ float newVal);
   STDMETHOD(get_Material)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_Material)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_AnimSpeed)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_AnimSpeed)(/*[in]*/ float newVal);
   STDMETHOD(get_ReflectionEnabled)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_ReflectionEnabled)(/*[in]*/ VARIANT_BOOL newVal);
};

#endif // !defined(AFX_TRIGGER_H__2CA7256C_4072_43C3_9D65_AE091B601377__INCLUDED_)
