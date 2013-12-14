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
	COLORREF m_color;
	TimerDataRoot m_tdr;
	char m_szSurface[MAXTOKEN];
	BOOL m_fEnabled;
	KickerType m_kickertype;
	float m_scatter;
	float m_hit_height; //kicker hit object height ... default 40
	};

class KickerHitCircle;

/////////////////////////////////////////////////////////////////////////////
// Kicker

class Kicker : 
	public IDispatchImpl<IKicker, &IID_IKicker, &LIBID_VBATESTLib>, 
	//public ISupportErrorInfo,
	public CComObjectRoot,
	public CComCoClass<Kicker,&CLSID_Kicker>,
#ifdef VBA
	public CApcProjectItem<Kicker>,	
#endif
	public EventProxy<Kicker, &DIID_IKickerEvents>,
	public IConnectionPointContainerImpl<Kicker>,
	public IProvideClassInfo2Impl<&CLSID_Kicker, &DIID_IKickerEvents, &LIBID_VBATESTLib>,
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

STANDARD_DISPATCH_DECLARE
STANDARD_EDITABLE_DECLARES(eItemKicker)

DECLARE_REGISTRY_RESOURCEID(IDR_KICKER)
// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

	//virtual HRESULT GetTypeName(BSTR *pVal);
	//virtual int GetDialogID();
	virtual void GetDialogPanes(Vector<PropertyPane> *pvproppane);

	virtual void MoveOffset(const float dx, const float dy);
	virtual void SetObjectPos();
	// Multi-object manipulation
	virtual void GetCenter(Vertex2D * const pv) const;
	virtual void PutCenter(const Vertex2D * const pv);

	void WriteRegDefaults();

	PinTable *m_ptable;

	KickerData m_d;

	KickerHitCircle *m_phitkickercircle;
   Vertex3D vertices[49];
   Vertex3D borderVerices[16];
   RECT recBounds;
   Material colorMaterial;
   Material blackMaterial;


// IKicker
public:
	STDMETHOD(get_Color)(/*[out, retval]*/ OLE_COLOR *pVal);
	STDMETHOD(put_Color)(/*[in]*/ OLE_COLOR newVal);
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
	STDMETHOD(KickXYZ)(float angle, float speed, float inclination, float x,float y,float z);
	//STDMETHOD(DestroyBall)();
	STDMETHOD(DestroyBall)(/*[out, retval]*/ int *pVal);
    STDMETHOD(CreateBall)(IBall **pBallEx);
	STDMETHOD(CreateSizedBall)(/*[in]*/float radius, /*out, retval]*/ IBall **pBallEx);
	STDMETHOD(BallCntOver)(/*[out, retval]*/ int *pVal);

	STDMETHOD(get_Scatter)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Scatter)(/*[in]*/ float newVal);
	STDMETHOD(get_HitHeight)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_HitHeight)(/*[in]*/ float newVal);
};

class KickerHitCircle : public TriggerHitCircle
	{
public:
	KickerHitCircle();
	virtual void Collide(Ball * const pball, Vertex3Ds * const phitnormal);

	Kicker *m_pkicker;
	Ball *m_pball;  //The ball inside this kicker
	float m_zheight;
	};

#endif // !defined(AFX_KICKER_H__3A9F3FC4_605A_43AD_A430_830279CFE059__INCLUDED_)
