// Trigger.h: Definition of the Trigger class
//
//////////////////////////////////////////////////////////////////////
#pragma once
#if !defined(AFX_TRIGGER_H__2CA7256C_4072_43C3_9D65_AE091B601377__INCLUDED_)
#define AFX_TRIGGER_H__2CA7256C_4072_43C3_9D65_AE091B601377__INCLUDED_

#include "resource.h"

class TriggerData final : public BaseProperty
{
public:
   Vertex2D m_vCenter;
   float m_radius;
   TimerDataRoot m_tdr;
   string m_szSurface;
   TriggerShape m_shape;
   float m_rotation;
   float m_scaleX;
   float m_scaleY;
   float m_hit_height; // trigger hit object height ... default 50
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
#ifdef __STANDALONE__
public:
   STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid,DISPID* rgDispId);
   STDMETHOD(Invoke)(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);
   STDMETHOD(GetDocumentation)(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile);
   virtual HRESULT FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) override;
#endif
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

   void RenderBlueprint(Sur *psur, const bool solid) final;

   void MoveOffset(const float dx, const float dy) final;
   void SetObjectPos() final;

   void EditMenu(CMenu &hmenu) final;
   void DoCommand(int icmd, int x, int y) final;

   // Multi-object manipulation
   void FlipY(const Vertex2D& pvCenter) final;
   void FlipX(const Vertex2D& pvCenter) final;
   void Rotate(const float ang, const Vertex2D& pvCenter, const bool useElementCenter) final;
   void Scale(const float scalex, const float scaley, const Vertex2D& pvCenter, const bool useElementCenter) final;
   void Translate(const Vertex2D &pvOffset) final;
   Vertex2D GetCenter() const final { return GetPointCenter(); }
   Vertex2D GetScale() const final { return Vertex2D(m_d.m_scaleX, m_d.m_scaleY); }
   float GetRotate() const final { return m_d.m_rotation; }

   void PutCenter(const Vertex2D& pv) final { PutPointCenter(pv); }
   Vertex2D GetPointCenter() const final;
   void PutPointCenter(const Vertex2D& pv) final;
   void ExportMesh(ObjLoader& loader) final;
   ItemTypeEnum HitableGetItemType() const final { return eItemTrigger; }
   void UpdateStatusBarInfo() final;

   void ClearForOverwrite() final;

   void WriteRegDefaults() final;

   void TriggerAnimationHit();
   void TriggerAnimationUnhit();

   Trigger *CopyForPlay(PinTable *live_table);

   TriggerData m_d;

   bool m_hitEnabled;		// for custom shape triggers

private:
   void CurvesToShapes(vector<HitObject*> &pvho);
   void AddLine(vector<HitObject*> &pvho, const RenderVertex &pv1, const RenderVertex &pv2, const float height);

   void InitShape(float x, float y);
   void GenerateMesh();

   PinTable *m_ptable;

   TriggerHitCircle *m_ptriggerhitcircle;

   RenderDevice *m_rd = nullptr;
   MeshBuffer *m_meshBuffer = nullptr;
   vector<Vertex3Ds> m_vertices;
   const WORD *m_faceIndices = nullptr;
   Vertex3D_NoTex2 *m_triggerVertices = nullptr;
   int m_numVertices = 0;
   int m_numIndices = 0;

   PropertyPane *m_propVisual;

   float m_animHeightOffset;
   float m_vertexBuffer_animHeightOffset;
   bool m_hitEvent;
   bool m_unhitEvent;
   bool m_doAnimation;
   bool m_moveDown;

   Vertex3Ds m_boundingSphereCenter;

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
   STDMETHOD(get_CurrentAnimOffset)(/*[out, retval]*/ float *pVal);
};

#endif // !defined(AFX_TRIGGER_H__2CA7256C_4072_43C3_9D65_AE091B601377__INCLUDED_)
