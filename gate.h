// Gate.h: Definition of the Gate class
//
//////////////////////////////////////////////////////////////////////
#pragma once
#if !defined(AFX_GATE_H__EDC63CB2_226F_4606_99A9_0C2DB8FE1E3B__INCLUDED_)
#define AFX_GATE_H__EDC63CB2_226F_4606_99A9_0C2DB8FE1E3B__INCLUDED_

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// Gate

class GateData
{
public:
   Vertex2D m_vCenter;
   float m_length;
   float m_height;
   float m_rotation;
   char m_szMaterial[32];
   TimerDataRoot m_tdr;
   float m_elasticity;
   float m_friction;
   float m_damping;
   float m_scatter;
   char m_szSurface[MAXTOKEN];
   float m_angleMin;
   float m_angleMax;
   GateType m_type;
   bool m_fShowBracket;
   bool m_fCollidable;
   bool m_fVisible;
   bool m_twoWay;
   bool m_fReflectionEnabled;
};

class Gate :
   public IDispatchImpl<IGate, &IID_IGate, &LIBID_VPinballLib>,
   //public ISupportErrorInfo,
   public CComObjectRoot,
   public CComCoClass<Gate, &CLSID_Gate>,
   public EventProxy<Gate, &DIID_IGateEvents>,
   public IConnectionPointContainerImpl<Gate>,
   public IProvideClassInfo2Impl<&CLSID_Gate, &DIID_IGateEvents, &LIBID_VPinballLib>,
   public ISelect,
   public IEditable,
   public Hitable,
   public IScriptable,
   public IFireEvents,
   public IPerPropertyBrowsing // Ability to fill in dropdown in property browser
{
public:
   Gate();
   void SetGateType(GateType type);
   ~Gate();

   BEGIN_COM_MAP(Gate)
      COM_INTERFACE_ENTRY(IDispatch)
      COM_INTERFACE_ENTRY(IGate)
      //COM_INTERFACE_ENTRY(ISupportErrorInfo)
      COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
      COM_INTERFACE_ENTRY(IPerPropertyBrowsing)
      COM_INTERFACE_ENTRY(IProvideClassInfo)
      COM_INTERFACE_ENTRY(IProvideClassInfo2)
   END_COM_MAP()
   //DECLARE_NOT_AGGREGATABLE(Light)
   // Remove the comment from the line above if you don't want your object to
   // support aggregation.

   BEGIN_CONNECTION_POINT_MAP(Gate)
      CONNECTION_POINT_ENTRY(DIID_IGateEvents)
   END_CONNECTION_POINT_MAP()

   STANDARD_EDITABLE_DECLARES(Gate, eItemGate, GATE, 1)

      //DECLARE_NOT_AGGREGATABLE(Gate)
      // Remove the comment from the line above if you don't want your object to
      // support aggregation.

      DECLARE_REGISTRY_RESOURCEID(IDR_GATE)
   // ISupportsErrorInfo
   STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

   virtual void GetDialogPanes(Vector<PropertyPane> *pvproppane);

   virtual void MoveOffset(const float dx, const float dy);
   virtual void SetObjectPos();
   // Multi-object manipulation
   virtual void GetCenter(Vertex2D * const pv) const;
   virtual void PutCenter(const Vertex2D * const pv);
   virtual void SetDefaultPhysics(bool fromMouseClick);

   virtual void RenderBlueprint(Sur *psur, const bool solid=false);
   virtual void ExportMesh(FILE *f);

   virtual unsigned long long GetMaterialID() { return m_ptable->GetMaterial(m_d.m_szMaterial)->hash(); }

   void WriteRegDefaults();
   void UpdateWire(RenderDevice *pd3dDevice);
   void RenderObject(RenderDevice* pd3dDevice);
   void GenerateBracketMesh(Vertex3D_NoTex2 *buf);
   void GenerateWireMesh(Vertex3D_NoTex2 *buf);

   PinTable *m_ptable;

   GateData m_d;

private:
   LineSeg *m_plineseg;
   HitGate *m_phitgate;

   VertexBuffer *wireVertexBuffer;
   IndexBuffer *wireIndexBuffer;
   float m_vertexbuffer_angle;

   VertexBuffer *bracketVertexBuffer;
   IndexBuffer *bracketIndexBuffer;

   const Vertex3D_NoTex2 *m_vertices;
   const WORD            *m_indices;
   unsigned int m_numVertices;
   unsigned int m_numIndices;

   float baseHeight;

   // IGate
public:
   STDMETHOD(get_Elasticity)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Elasticity)(/*[in]*/ float newVal);
   STDMETHOD(get_Open)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_Open)(/*[in]*/ VARIANT_BOOL newVal);

   STDMETHOD(get_Surface)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_Surface)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_Y)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Y)(/*[in]*/ float newVal);
   STDMETHOD(get_X)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_X)(/*[in]*/ float newVal);
   STDMETHOD(get_Rotation)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Rotation)(/*[in]*/ float newVal);
   STDMETHOD(get_Length)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Length)(/*[in]*/ float newVal);
   STDMETHOD(get_Material)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_Material)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_Height)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Height)(/*[in]*/ float newVal);
   STDMETHOD(get_ShowBracket)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_ShowBracket)(/*[in]*/ VARIANT_BOOL newVal);

   STDMETHOD(get_CloseAngle)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_CloseAngle)(/*[in]*/ float newVal);
   STDMETHOD(get_OpenAngle)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_OpenAngle)(/*[in]*/ float newVal);
   STDMETHOD(get_Collidable)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_Collidable)(/*[in]*/ VARIANT_BOOL newVal);

   STDMETHOD(Move)(int dir, float speed, float angle);

   STDMETHOD(get_Friction)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Friction)(/*[in]*/ float newVal);
   STDMETHOD(get_Damping)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Damping)(/*[in]*/ float newVal);
   STDMETHOD(get_Visible)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_Visible)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_TwoWay)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_TwoWay)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_CurrentAngle)(/*[out, retval]*/ float *pVal);
   STDMETHOD(get_ReflectionEnabled)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_ReflectionEnabled)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_DrawStyle)(/*[out, retval]*/ GateType *pVal);
   STDMETHOD(put_DrawStyle)(/*[in]*/ GateType newVal);
};

#endif // !defined(AFX_GATE_H__EDC63CB2_226F_4606_99A9_0C2DB8FE1E3B__INCLUDED_)
