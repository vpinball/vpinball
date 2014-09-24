// Bumper.h: Definition of the Bumper class
//
//////////////////////////////////////////////////////////////////////
#pragma once
#if !defined(AFX_BUMPER_H__9A202FF0_7FAE_49BF_AA4C_C01C692E6DD9__INCLUDED_)
#define AFX_BUMPER_H__9A202FF0_7FAE_49BF_AA4C_C01C692E6DD9__INCLUDED_

#include "resource.h"       // main symbols

class IBlink
	{
public:
	virtual void DrawFrame(BOOL fOn) = 0;

	char m_rgblinkpattern[33];
	int m_blinkinterval;

	int m_timenextblink;
	int m_iblinkframe;

    void UpdateBlinker(int time_msec)
    {
        if (m_timenextblink <= time_msec)
        {
            m_iblinkframe++;
            char cnew = m_rgblinkpattern[m_iblinkframe];
            if (cnew == 0)
            {
                m_iblinkframe = 0;
                cnew = m_rgblinkpattern[0];
            }

            DrawFrame(cnew == '1');
            m_timenextblink += m_blinkinterval;
        }
    }

    void RestartBlinker(int cur_time_msec)
    {
        m_iblinkframe = 0;
        const char cnew = m_rgblinkpattern[m_iblinkframe];

        DrawFrame(cnew == '1');
        m_timenextblink = cur_time_msec + m_blinkinterval;
    }

	};

class BumperData
	{
public:
	Vertex2D m_vCenter;
	float m_radius;
	float m_threshold; // speed at which ball needs to hit to register a hit
	float m_force; // force the bumper kicks back with
	float m_heightoffset;
	TimerDataRoot m_tdr;
	float m_overhang;
    char m_szCapMaterial[32];
    char m_szBaseMaterial[32];
	char m_szImage[MAXTOKEN];
	char m_szSurface[MAXTOKEN];
	LightState m_state;
	BOOL m_fFlashWhenHit; // Hacky flag for cool auto-behavior
	BOOL m_fCastsShadow;
	BOOL m_fVisible;
	BOOL m_fSideVisible;
	//char m_rgblinkpattern[33];
	//int m_blinkinterval;
	};

/////////////////////////////////////////////////////////////////////////////
// Bumper

class Bumper :
	//public CComObjectRootEx<CComSingleThreadModel>,
	public IDispatchImpl<IBumper, &IID_IBumper, &LIBID_VPinballLib>,
	//public ISupportErrorInfo,
	public CComObjectRoot,
	public CComCoClass<Bumper,&CLSID_Bumper>,
	public EventProxy<Bumper, &DIID_IBumperEvents>,
	public IConnectionPointContainerImpl<Bumper>,
	public IProvideClassInfo2Impl<&CLSID_Bumper, &DIID_IBumperEvents, &LIBID_VPinballLib>,
	public ISelect,
	public IEditable,
	public Hitable,
	public IScriptable,
	public IBlink,
	public IFireEvents,
	public IPerPropertyBrowsing // Ability to fill in dropdown in property browser
	//public EditableImpl<Bumper>
{
public:
	Bumper();
	~Bumper();

BEGIN_COM_MAP(Bumper)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IBumper)
	//COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
	COM_INTERFACE_ENTRY(IPerPropertyBrowsing)
	COM_INTERFACE_ENTRY(IProvideClassInfo)
	COM_INTERFACE_ENTRY(IProvideClassInfo2)
END_COM_MAP()
//DECLARE_NOT_AGGREGATABLE(Bumper)
// Remove the comment from the line above if you don't want your object to
// support aggregation.

STANDARD_EDITABLE_DECLARES(Bumper, eItemBumper, BUMPER, 1)

BEGIN_CONNECTION_POINT_MAP(Bumper)
	CONNECTION_POINT_ENTRY(DIID_IBumperEvents)
END_CONNECTION_POINT_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_BUMPER)

// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

	virtual void MoveOffset(const float dx, const float dy);
	virtual void SetObjectPos();
	virtual void RenderShadow(ShadowSur * const psur, const float height);

	virtual void GetDialogPanes(Vector<PropertyPane> *pvproppane);

	// Multi-object manipulation
	virtual void GetCenter(Vertex2D * const pv) const;
	virtual void PutCenter(const Vertex2D * const pv);

	virtual void DrawFrame(BOOL fOn);

	void WriteRegDefaults();

	PinTable *m_ptable;

	BumperData m_d;

	BumperHitCircle *m_pbumperhitcircle;

//>>> Added By Chris
	BOOL		m_fDisabled;
	LightState 	m_realState;
	void		lockLight();
	void		unLockLight();
	void		setLightStateBypass(const LightState newVal);
	void		setLightState(const LightState newVal);
//<<<

private:
    VertexBuffer *vtxBuf;
    IndexBuffer *idxBuf;

	bool m_fLockedByLS;

// IBumper
public:
	STDMETHOD(get_BaseMaterial)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_BaseMaterial)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_FlashWhenHit)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_FlashWhenHit)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_BlinkInterval)(/*[out, retval]*/ long *pVal);
	STDMETHOD(put_BlinkInterval)(/*[in]*/ long newVal);
	STDMETHOD(get_BlinkPattern)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_BlinkPattern)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_State)(/*[out, retval]*/ LightState *pVal);
	STDMETHOD(put_State)(/*[in]*/ LightState newVal);
	STDMETHOD(get_Surface)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_Surface)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_Y)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Y)(/*[in]*/ float newVal);
	STDMETHOD(get_X)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_X)(/*[in]*/ float newVal);
	STDMETHOD(get_Image)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_Image)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_CapMaterial)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_CapMaterial)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_Overhang)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Overhang)(/*[in]*/ float newVal);
	STDMETHOD(get_Threshold)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Threshold)(/*[in]*/ float newVal);
	STDMETHOD(get_Force)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Force)(/*[in]*/ float newVal);
	STDMETHOD(get_HeightOffset)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_HeightOffset)(/*[in]*/ float newVal);
	STDMETHOD(get_Radius)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Radius)(/*[in]*/ float newVal);
	STDMETHOD(get_CastsShadow)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_CastsShadow)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_Disabled)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_Disabled)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_Visible)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_Visible)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_SideVisible)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_SideVisible)(/*[in]*/ VARIANT_BOOL newVal);
};

#endif // !defined(AFX_BUMPER_H__9A202FF0_7FAE_49BF_AA4C_C01C692E6DD9__INCLUDED_)
