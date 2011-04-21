#pragma once
// Plunger.h: Definition of the Plunger class
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PLUNGER_H__A729D2E2_D68F_4DD6_BE4B_D8AD1B8C7B66__INCLUDED_)
#define AFX_PLUNGER_H__A729D2E2_D68F_4DD6_BE4B_D8AD1B8C7B66__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"       // main symbols

class PlungerData
	{
public:
	COLORREF m_color;
	Vertex2D m_v;
	float m_width;
	float m_height;
	float m_stroke;
	float m_speedPull;
	float m_speedFire;
	float m_mechStrength;
	PlungerType m_type;
	char m_szImage[MAXTOKEN];
	TimerDataRoot m_tdr;
	BOOL m_mechPlunger;
	BOOL m_autoPlunger;
	BOOL m_fVisible;
	float m_parkPosition;
	char m_szSurface[MAXTOKEN];
	float m_scatterVelocity;
	float m_breakOverVelocity;
	};

/////////////////////////////////////////////////////////////////////////////
// Plunger

class Plunger :
	public IDispatchImpl<IPlunger, &IID_IPlunger, &LIBID_VBATESTLib>,
	public ISupportErrorInfo,
	public CComObjectRoot,
	public CComCoClass<Plunger,&CLSID_Plunger>,
#ifdef VBA
	public CApcProjectItem<Plunger>,
#endif
	public EventProxy<Plunger, &DIID_IPlungerEvents>,
	public IConnectionPointContainerImpl<Plunger>,
	public IProvideClassInfo2Impl<&CLSID_Plunger, &DIID_IPlungerEvents, &LIBID_VBATESTLib>,
	//public CComObjectRootEx<CComSingleThreadModel>,


	public ISelect,
	public IEditable,
	public Hitable,
	public IScriptable,
	public IFireEvents,
	public IPerPropertyBrowsing // Ability to fill in dropdown in property browser
{
public:
BEGIN_COM_MAP(Plunger)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IPlunger)
	COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
	//COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY(IPerPropertyBrowsing)
	COM_INTERFACE_ENTRY(IProvideClassInfo)
	COM_INTERFACE_ENTRY(IProvideClassInfo2)
END_COM_MAP()

BEGIN_CONNECTION_POINT_MAP(Plunger)
	CONNECTION_POINT_ENTRY(DIID_IPlungerEvents)
END_CONNECTION_POINT_MAP()
//DECLARE_NOT_AGGREGATABLE(Plunger)
// Remove the comment from the line above if you don't want your object to
// support aggregation.

	Plunger();
	virtual ~Plunger();

STANDARD_DISPATCH_DECLARE
STANDARD_EDITABLE_DECLARES(eItemPlunger)

	//virtual HRESULT GetTypeName(BSTR *pVal);
	//virtual int GetDialogID();
	virtual void GetDialogPanes(Vector<PropertyPane> *pvproppane);

	virtual void MoveOffset(const float dx, const float dy);
	virtual void SetObjectPos();
	// Multi-object manipulation
	virtual void GetCenter(Vertex2D *pv);
	virtual void PutCenter(Vertex2D *pv);

	void WriteRegDefaults();

DECLARE_REGISTRY_RESOURCEID(IDR_Plunger)
// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

	PinTable *m_ptable;

	PlungerData m_d;

	HitPlunger *m_phitplunger;

// IPlunger
public:
	STDMETHOD(get_Surface)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_Surface)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_Y)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Y)(/*[in]*/ float newVal);
	STDMETHOD(get_X)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_X)(/*[in]*/ float newVal);
	STDMETHOD(CreateBall)(IBall **pBallEx);
	STDMETHOD(get_FireSpeed)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_FireSpeed)(/*[in]*/ float newVal);
	STDMETHOD(get_PullSpeed)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_PullSpeed)(/*[in]*/ float newVal);
	STDMETHOD(get_MechStrength)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_MechStrength)(/*[in]*/ float newVal);
	STDMETHOD(Fire)();
	STDMETHOD(PullBack)();
	STDMETHOD(get_Type)(/*[out, retval]*/ PlungerType *pVal);
	STDMETHOD(put_Type)(/*[in]*/ PlungerType newVal);
	STDMETHOD(get_Color)(/*[out, retval]*/ OLE_COLOR *pVal);
	STDMETHOD(put_Color)(/*[in]*/ OLE_COLOR newVal);
	STDMETHOD(get_Image)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_Image)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_MechPlunger)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_MechPlunger)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_AutoPlunger)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_AutoPlunger)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_Visible)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_Visible)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_ParkPosition)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_ParkPosition)(/*[in]*/ float newVal);
	STDMETHOD(get_Stroke)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Stroke)(/*[in]*/ float newVal);
	STDMETHOD(get_ScatterVelocity)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_ScatterVelocity)(/*[in]*/ float newVal);
	STDMETHOD(get_BreakOverVelocity)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_BreakOverVelocity)(/*[in]*/ float newVal);
	STDMETHOD(Position)(int *pVal); //added for mechanical plunger position
	STDMETHOD(MotionDevice)(int *pVal);
};

#endif // !defined(AFX_PLUNGER_H__A729D2E2_D68F_4DD6_BE4B_D8AD1B8C7B66__INCLUDED_)
