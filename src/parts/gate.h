// Gate.h: Definition of the Gate class
//
//////////////////////////////////////////////////////////////////////
#pragma once
#if !defined(AFX_GATE_H__EDC63CB2_226F_4606_99A9_0C2DB8FE1E3B__INCLUDED_)
#define AFX_GATE_H__EDC63CB2_226F_4606_99A9_0C2DB8FE1E3B__INCLUDED_

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// Gate

class GateData final : public BaseProperty
{
public:
   Vertex2D m_vCenter;
   float m_length;
   float m_height;
   float m_rotation;
   TimerDataRoot m_tdr;
   float m_damping;
   float m_gravityfactor;
   string m_szSurface;
   float m_angleMin;
   float m_angleMax;
   GateType m_type;
   bool m_showBracket;
   bool m_twoWay;
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
#ifdef __STANDALONE__
public:
   STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid,DISPID* rgDispId);
   STDMETHOD(Invoke)(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);
   STDMETHOD(GetDocumentation)(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile);
   virtual HRESULT FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) override;
#endif
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

   void MoveOffset(const float dx, const float dy) final;
   void SetObjectPos() final;
   // Multi-object manipulation
   Vertex2D GetCenter() const final;
   void PutCenter(const Vertex2D &pv) final;
   void SetDefaultPhysics(const bool fromMouseClick) final;

   void RenderBlueprint(Sur *psur, const bool solid) final;
   void ExportMesh(ObjLoader &loader) final;

   ItemTypeEnum HitableGetItemType() const final { return eItemGate; }
   void UpdateStatusBarInfo() final;

   void WriteRegDefaults() final;

   float GetOpenAngle() const;
   void SetOpenAngle(const float angle);
   float GetCloseAngle() const;
   void SetCloseAngle(const float angle);

   Gate *CopyForPlay(PinTable *live_table);

   GateData m_d;

private:
   void GenerateBracketMesh(Vertex3D_NoTex2 *buf);
   void GenerateWireMesh(Vertex3D_NoTex2 *buf);

   PinTable *m_ptable = nullptr;
   
   RenderDevice *m_rd = nullptr;

   LineSeg *m_plineseg = nullptr;
   HitGate *m_phitgate = nullptr;
   float m_lastAngle;

   float m_vertexbuffer_angle = FLT_MAX;
   MeshBuffer *m_wireMeshBuffer = nullptr;
   MeshBuffer *m_bracketMeshBuffer = nullptr;

   const Vertex3D_NoTex2 *m_vertices = nullptr;
   const WORD            *m_indices = nullptr;
   unsigned int m_numVertices = 0;
   unsigned int m_numIndices = 0;

   float m_baseHeight;

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
   STDMETHOD(get_GravityFactor)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_GravityFactor)(/*[in]*/ float newVal);
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
