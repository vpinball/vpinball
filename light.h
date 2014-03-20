// Light.h: Definition of the Light class
//
//////////////////////////////////////////////////////////////////////
#pragma once
#if !defined(AFX_LIGHT_H__7445FDB1_1FBE_4975_9AB6_367E6D16098F__INCLUDED_)
#define AFX_LIGHT_H__7445FDB1_1FBE_4975_9AB6_367E6D16098F__INCLUDED_

#include "resource.h"       // main symbols
#include "RenderDevice.h"

class LightData
	{
public:
	Vertex2D m_vCenter;
	float m_radius;
	LightState m_state;
	COLORREF m_color;
	TimerDataRoot m_tdr;
	Shape m_shape;
	//char m_rgblinkpattern[33];
	//int m_blinkinterval;
	float m_borderwidth;
	COLORREF m_bordercolor;
   //Material normalMatr;
   //Material borderMatr;
   //float normalR,normalG,normalB;
   //float borderR,borderG,borderB;
	char m_szSurface[MAXTOKEN];
	char m_szOnImage[MAXTOKEN];
	char m_szOffImage[MAXTOKEN];
	BOOL m_fDisplayImage;
	BOOL m_EnableLighting;
   BOOL m_EnableOffLighting;
   BOOL m_OnImageIsLightMap;
    float m_depthBias;      // for determining depth sorting
	};

class LightCenter : public ISelect
	{
public:
	LightCenter(Light *plight);
	virtual HRESULT GetTypeName(BSTR *pVal);
	virtual IDispatch *GetDispatch();
	virtual void GetDialogPanes(Vector<PropertyPane> *pvproppane);

	virtual void Delete();
	virtual void Uncreate();

	virtual int GetSelectLevel();

	virtual IEditable *GetIEditable();

	virtual PinTable *GetPTable();

	virtual BOOL LoadToken(int id, BiffReader *pbr) {return fTrue;}

	virtual void GetCenter(Vertex2D * const pv) const;
	virtual void PutCenter(const Vertex2D * const pv);

	virtual void MoveOffset(const float dx, const float dy);

	virtual ItemTypeEnum GetItemType() {return eItemLightCenter;}

	Light *m_plight;
	};

/////////////////////////////////////////////////////////////////////////////
// Light

class Light :
	public IDispatchImpl<ILight, &IID_ILight, &LIBID_VBATESTLib>,
	//public ISupportErrorInfo,
	public CComObjectRoot,
	public CComCoClass<Light,&CLSID_Light>,
#ifdef VBA
	public CApcProjectItem<Light>,
#endif
	public EventProxy<Light, &DIID_ILightEvents>,
	public IConnectionPointContainerImpl<Light>,
	public IProvideClassInfo2Impl<&CLSID_Light, &DIID_ILightEvents, &LIBID_VBATESTLib>,
	public ISelect,
	public IEditable,
	public Hitable,
	public IHaveDragPoints,
	public IScriptable,
	public IBlink,
	public IFireEvents,
	public IPerPropertyBrowsing // Ability to fill in dropdown in property browser
{
public:
	Light();
	virtual ~Light();
BEGIN_COM_MAP(Light)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ILight)
	//COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
	COM_INTERFACE_ENTRY(IPerPropertyBrowsing)
	COM_INTERFACE_ENTRY(IProvideClassInfo)
	COM_INTERFACE_ENTRY(IProvideClassInfo2)
END_COM_MAP()
//DECLARE_NOT_AGGREGATABLE(Light)
// Remove the comment from the line above if you don't want your object to
// support aggregation.

BEGIN_CONNECTION_POINT_MAP(Light)
	CONNECTION_POINT_ENTRY(DIID_ILightEvents)
END_CONNECTION_POINT_MAP()

STANDARD_DISPATCH_DECLARE
STANDARD_EDITABLE_DECLARES(eItemLight)

DECLARE_REGISTRY_RESOURCEID(IDR_Light)
// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

	//virtual HRESULT GetTypeName(BSTR *pVal);
	virtual void GetDialogPanes(Vector<PropertyPane> *pvproppane);

	void RenderOutline(Sur * const psur);
	virtual void RenderBlueprint(Sur *psur);

	virtual void MoveOffset(const float dx, const float dy);
	virtual void SetObjectPos();

	virtual void ClearForOverwrite();

	void PrepareStaticCustom();
    void PrepareMoversCustom();

	virtual void EditMenu(HMENU hmenu);
	virtual void DoCommand(int icmd, int x, int y);

	virtual void FlipY(Vertex2D * const pvCenter);
	virtual void FlipX(Vertex2D * const pvCenter);
	virtual void Rotate(float ang, Vertex2D *pvCenter);
	virtual void Scale(float scalex, float scaley, Vertex2D *pvCenter);
	virtual void Translate(Vertex2D *pvOffset);

	// DragPoints
	virtual void GetCenter(Vertex2D * const pv) const {GetPointCenter(pv);}
	virtual void PutCenter(const Vertex2D * const pv) {PutPointCenter(pv);}
	virtual void GetPointCenter(Vertex2D * const pv) const;
	virtual void PutPointCenter(const Vertex2D * const pv);

	virtual void DrawFrame(BOOL fOn)  { }
    void PostRenderStaticCustom(RenderDevice* pd3dDevice);

    virtual bool IsTransparent()    { return m_d.m_OnImageIsLightMap != FALSE; }
    virtual float GetDepth(const Vertex3Ds& viewDir)
      { return m_d.m_depthBias + viewDir.x * m_d.m_vCenter.x + viewDir.y * m_d.m_vCenter.y + viewDir.z * m_surfaceHeight; }

	void WriteRegDefaults();
   void FreeBuffers();

	void		lockLight();
	void		unLockLight();
	void		setLightStateBypass(const LightState newVal);
	void		setLightState(const LightState newVal);

	PinTable *m_ptable;

	LightData m_d;
	LightState 	m_realState;

	// Run-time
private:
    float m_surfaceHeight;
	ObjFrame *m_pobjframe[2];

	LightCenter m_lightcenter;

//>>> Added By Chris
	bool		m_fLockedByLS;
//<<<
   int staticCustomVertexNum;
   int customMoverVertexNum;
   VertexBuffer *customVBuffer;
   VertexBuffer *normalVBuffer;
   VertexBuffer *customMoverVBuffer;
   VertexBuffer *normalMoverVBuffer;
// ILight
public:
	STDMETHOD(get_Surface)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_Surface)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_BorderWidth)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_BorderWidth)(/*[in]*/ float newVal);
	STDMETHOD(get_BorderColor)(/*[out, retval]*/ OLE_COLOR *pVal);
	STDMETHOD(put_BorderColor)(/*[in]*/ OLE_COLOR newVal);
	STDMETHOD(get_BlinkInterval)(/*[out, retval]*/ long *pVal);
	STDMETHOD(put_BlinkInterval)(/*[in]*/ long newVal);
	STDMETHOD(get_BlinkPattern)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_BlinkPattern)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_Shape)(/*[out, retval]*/ Shape *pVal);
	STDMETHOD(put_Shape)(/*[in]*/ Shape newVal);
	STDMETHOD(get_Y)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Y)(/*[in]*/ float newVal);
	STDMETHOD(get_X)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_X)(/*[in]*/ float newVal);
	STDMETHOD(get_Color)(/*[out, retval]*/ OLE_COLOR *pVal);
	STDMETHOD(put_Color)(/*[in]*/ OLE_COLOR newVal);
	STDMETHOD(get_State)(/*[out, retval]*/ LightState *pVal);
	STDMETHOD(put_State)(/*[in]*/ LightState newVal);
	STDMETHOD(get_Radius)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Radius)(/*[in]*/ float newVal);
	STDMETHOD(get_OffImage)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_OffImage)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_OnImage)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_OnImage)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_DisplayImage)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_DisplayImage)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_EnableLighting)(/*[out, retval]*/ int *pVal);
   STDMETHOD(put_EnableLighting)(/*[in]*/ int newVal);
   STDMETHOD(get_EnableOffLighting)(/*[out, retval]*/ int *pVal);
   STDMETHOD(put_EnableOffLighting)(/*[in]*/ int newVal);
   STDMETHOD(get_OnImageIsLightmap)(/*[out, retval]*/ int *pVal);
   STDMETHOD(put_OnImageIsLightmap)(/*[in]*/ int newVal);
   STDMETHOD(get_DepthBias)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_DepthBias)(/*[in]*/ float newVal);
};

#endif // !defined(AFX_LIGHT_H__7445FDB1_1FBE_4975_9AB6_367E6D16098F__INCLUDED_)
