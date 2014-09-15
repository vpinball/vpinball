// Spinner.h: Definition of the Spinner class
//
//////////////////////////////////////////////////////////////////////
#pragma once
#if !defined(AFX_SPINNER_H__8D8CB0E1_8C8F_49BF_A639_4DFA12DD4C3C__INCLUDED_)
#define AFX_SPINNER_H__8D8CB0E1_8C8F_49BF_A639_4DFA12DD4C3C__INCLUDED_

#include "resource.h"       // main symbols

class SpinnerData
	{
public:
	Vertex2D m_vCenter;
	float m_length;
	float m_rotation;
	BOOL m_fSupports; 
    char m_szMaterial[32];
	TimerDataRoot m_tdr;
	float m_overhang;
    float m_height;
	float m_antifriction;
	float m_angleMax;
	float m_angleMin;
	float m_elasticity;
	//float m_friction;
	float m_scatter;
	BOOL m_fVisible;
	BOOL m_fCastsShadow;		//>>> added by Chris
	char m_szImageFront[MAXTOKEN];
	char m_szImageBack[MAXTOKEN];
	char m_szSurface[MAXTOKEN];
	};

class HitSpinner;

/////////////////////////////////////////////////////////////////////////////
// Spinner

class Spinner :
	public IDispatchImpl<ISpinner, &IID_ISpinner, &LIBID_VPinballLib>,
	//public ISupportErrorInfo,
	public CComObjectRoot,
	public CComCoClass<Spinner,&CLSID_Spinner>,
	public EventProxy<Spinner, &DIID_ISpinnerEvents>,
	public IConnectionPointContainerImpl<Spinner>,
	public IProvideClassInfo2Impl<&CLSID_Spinner, &DIID_ISpinnerEvents, &LIBID_VPinballLib>,
	public ISelect,
	public IEditable,
	public Hitable,
	public IScriptable,
	public IFireEvents,
	public IPerPropertyBrowsing // Ability to fill in dropdown in property browser
{
public:
	Spinner();
	virtual ~Spinner();
BEGIN_COM_MAP(Spinner)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISpinner)
	//COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
	COM_INTERFACE_ENTRY(IPerPropertyBrowsing)
	COM_INTERFACE_ENTRY(IProvideClassInfo)
	COM_INTERFACE_ENTRY(IProvideClassInfo2)
END_COM_MAP()
//DECLARE_NOT_AGGREGATABLE(Spinner)
// Remove the comment from the line above if you don't want your object to
// support aggregation.

BEGIN_CONNECTION_POINT_MAP(Spinner)
	CONNECTION_POINT_ENTRY(DIID_ISpinnerEvents)
END_CONNECTION_POINT_MAP()

STANDARD_EDITABLE_DECLARES(Spinner, eItemSpinner, SPINNER, 1)

DECLARE_REGISTRY_RESOURCEID(IDR_SPINNER)
// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

	virtual void GetDialogPanes(Vector<PropertyPane> *pvproppane);

	virtual void MoveOffset(const float dx, const float dy);
	virtual void SetObjectPos();
	// Multi-object manipulation
	virtual void GetCenter(Vertex2D * const pv) const;
	virtual void PutCenter(const Vertex2D * const pv);

	virtual void RenderShadow(ShadowSur * const psur, const float height);

   void PrepareStatic( RenderDevice* pd3dDevice );
   void PrepareMovers( RenderDevice* pd3dDevice );

	void WriteRegDefaults();

	PinTable *m_ptable;

	SpinnerData m_d;
    float m_posZ;

	HitSpinner *m_phitspinner;

private:
   Vertex3D staticVertices[8];
   VertexBuffer *vtxBuf;
   IndexBuffer *idxBuf;

// ISpinner
public:
	STDMETHOD(get_Surface)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_Surface)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_Y)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Y)(/*[in]*/ float newVal);
	STDMETHOD(get_X)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_X)(/*[in]*/ float newVal);
	STDMETHOD(get_Material)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_Material)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_ImageBack)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_ImageBack)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_ImageFront)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_ImageFront)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_Friction)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Friction)(/*[in]*/ float newVal);
	STDMETHOD(get_Overhang)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Overhang)(/*[in]*/ float newVal);
	STDMETHOD(get_Height)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Height)(/*[in]*/ float newVal);
	STDMETHOD(get_Rotation)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Rotation)(/*[in]*/ float newVal);
	STDMETHOD(get_Length)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Length)(/*[in]*/ float newVal);
//>>> added by chris
	STDMETHOD(get_CastsShadow)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_CastsShadow)(/*[in]*/ VARIANT_BOOL newVal);
//<<<
	
	STDMETHOD(get_Visible)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_Visible)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_Supports)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_Supports)(/*[in]*/ VARIANT_BOOL newVal);

	STDMETHOD (get_AngleMax)(/*[out, retval]*/ float *pVal);
	STDMETHOD (put_AngleMax)(/*[in]*/ float newVal);	
	STDMETHOD (get_AngleMin)(/*[out, retval]*/ float *pVal);
	STDMETHOD (put_AngleMin)(/*[in]*/ float newVal);
	STDMETHOD (get_Elasticity)(/*[out, retval]*/ float *pVal);
	STDMETHOD (put_Elasticity)(/*[in]*/ float newVal);
    STDMETHODIMP get_CurrentAngle(float *pVal);

};

#endif // !defined(AFX_SPINNER_H__8D8CB0E1_8C8F_49BF_A639_4DFA12DD4C3C__INCLUDED_)
