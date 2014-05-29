// Trigger.h: Definition of the Trigger class
//
//////////////////////////////////////////////////////////////////////
#pragma once
#if !defined(AFX_TRIGGER_H__2CA7256C_4072_43C3_9D65_AE091B601377__INCLUDED_)
#define AFX_TRIGGER_H__2CA7256C_4072_43C3_9D65_AE091B601377__INCLUDED_

#include "resource.h"       // main symbols

class TriggerData
	{
public:
	Vertex2D m_vCenter;
	float m_radius;
	TimerDataRoot m_tdr;
	char m_szSurface[MAXTOKEN];
	BOOL m_fEnabled;
	BOOL m_fVisible;
	Shape m_shape;
	float m_hit_height; //trigger hit object height ... default 50
	};

/////////////////////////////////////////////////////////////////////////////
// Trigger

class Trigger : 
	public IDispatchImpl<ITrigger, &IID_ITrigger, &LIBID_VPinballLib>, 
	//public ISupportErrorInfo,
	public CComObjectRoot,
	public CComCoClass<Trigger,&CLSID_Trigger>,
#ifdef VBA
	public CApcProjectItem<Trigger>,	
#endif
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

STANDARD_EDITABLE_DECLARES(Trigger, eItemTrigger)

DECLARE_REGISTRY_RESOURCEID(IDR_TRIGGER)
// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

	virtual void GetDialogPanes(Vector<PropertyPane> *pvproppane);

	virtual void RenderBlueprint(Sur *psur);

	virtual void MoveOffset(const float dx, const float dy);
	virtual void SetObjectPos();

	virtual void EditMenu(HMENU hmenu);
	virtual void DoCommand(int icmd, int x, int y);

	// Multi-object manipulation
	virtual void FlipY(Vertex2D * const pvCenter);
	virtual void FlipX(Vertex2D * const pvCenter);
	virtual void Rotate(float ang, Vertex2D *pvCenter);
	virtual void Scale(float scalex, float scaley, Vertex2D *pvCenter);
	virtual void Translate(Vertex2D *pvOffset);
	virtual void GetCenter(Vertex2D * const pv) const {GetPointCenter(pv);}
	virtual void PutCenter(const Vertex2D * const pv) {PutPointCenter(pv);}
	virtual void GetPointCenter(Vertex2D * const pv) const ;
	virtual void PutPointCenter(const Vertex2D * const pv);
	
	void CurvesToShapes(Vector<HitObject> * const pvho);
	void AddLine(Vector<HitObject> * const pvho, const RenderVertex * const pv1, const RenderVertex * const pv2, const RenderVertex * const pv3, const float height);

	virtual void ClearForOverwrite();

	void WriteRegDefaults();

	PinTable *m_ptable;

	TriggerData m_d;

	TriggerHitCircle *m_ptriggerhitcircle;

	BOOL m_hitEnabled;		// for custom shape triggers
   Material material;
   VertexBuffer *vertexBuffer;

// ITrigger
public:
	STDMETHOD(get_Shape)(/*[out, retval]*/ Shape *pVal);
	STDMETHOD(put_Shape)(/*[in]*/ Shape newVal);
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
};

#endif // !defined(AFX_TRIGGER_H__2CA7256C_4072_43C3_9D65_AE091B601377__INCLUDED_)
