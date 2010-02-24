// Gate.h: Definition of the Gate class
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GATE_H__EDC63CB2_226F_4606_99A9_0C2DB8FE1E3B__INCLUDED_)
#define AFX_GATE_H__EDC63CB2_226F_4606_99A9_0C2DB8FE1E3B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// Gate

class GateData
	{
public:
	Vertex m_vCenter;
	float m_length;
	float m_rotation;
	COLORREF m_color;
	TimerDataRoot m_tdr;
	float m_elasticity;
	char m_szSurface[MAXTOKEN];
	};

class Gate :
	public IDispatchImpl<IGate, &IID_IGate, &LIBID_VBATESTLib>,
	//public ISupportErrorInfo,
	public CComObjectRoot,
	public CComCoClass<Gate,&CLSID_Gate>,
#ifdef VBA
	public CApcProjectItem<Gate>,
#endif
	public EventProxy<Gate, &DIID_IGateEvents>,
	public IConnectionPointContainerImpl<Gate>,
	public IProvideClassInfo2Impl<&CLSID_Gate, &DIID_IGateEvents, &LIBID_VBATESTLib>,
	public ISelect,
	public IEditable,
	public Hitable,
	public IScriptable,
	public IFireEvents,
	public IPerPropertyBrowsing // Ability to fill in dropdown in property browser
{
public:
	Gate();
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

STANDARD_DISPATCH_DECLARE
STANDARD_EDITABLE_DECLARES(eItemGate)

//DECLARE_NOT_AGGREGATABLE(Gate)
// Remove the comment from the line above if you don't want your object to
// support aggregation.

DECLARE_REGISTRY_RESOURCEID(IDR_Gate)
// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

	//virtual HRESULT GetTypeName(BSTR *pVal);
	//virtual int GetDialogID();
	virtual void GetDialogPanes(Vector<PropertyPane> *pvproppane);

	virtual void MoveOffset(float dx, float dy);
	virtual void SetObjectPos();
	// Multi-object manipulation
	virtual void GetCenter(Vertex *pv);
	virtual void PutCenter(Vertex *pv);

	virtual void RenderBlueprint(Sur *psur);

	PinTable *m_ptable;

	GateData m_d;

	LineSeg *m_plineseg;
	HitGate *m_phitgate;

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
//>>> added by chris
	STDMETHOD(get_Color)(/*[out, retval]*/ OLE_COLOR *pVal);
	STDMETHOD(put_Color)(/*[in]*/ OLE_COLOR newVal);
//<<<
};

#endif // !defined(AFX_GATE_H__EDC63CB2_226F_4606_99A9_0C2DB8FE1E3B__INCLUDED_)
