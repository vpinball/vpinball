// Ramp.h: Definition of the Ramp class
//
//////////////////////////////////////////////////////////////////////
#pragma once
#if !defined(AFX_RAMP_H__5EFEDEFB_5504_430A_B000_9B6D1903E3FC__INCLUDED_)
#define AFX_RAMP_H__5EFEDEFB_5504_430A_B000_9B6D1903E3FC__INCLUDED_

#include "resource.h"       // main symbols

class RampData
	{
public:
	COLORREF m_color;
	TimerDataRoot m_tdr;
	float m_heightbottom;
	float m_heighttop;
	float m_widthbottom;
	float m_widthtop;
	RampType m_type;
	char m_szImage[MAXTOKEN];
	RampImageAlignment m_imagealignment;
	BOOL m_fImageWalls;
	BOOL m_fCastsShadow;
	BOOL m_fAcrylic;
	BOOL m_fAlpha;
	float m_leftwallheight;
	float m_rightwallheight;
	float m_leftwallheightvisible;
	float m_rightwallheightvisible;
	float m_elasticity;
	float m_friction;
	float m_scatter;
	BOOL m_fCollidable;
	BOOL m_IsVisible;
	BOOL m_fModify3DStereo;
	BOOL m_fAddBlend;
	};

/////////////////////////////////////////////////////////////////////////////
// Ramp

class Ramp :
	public IDispatchImpl<IRamp, &IID_IRamp, &LIBID_VBATESTLib>,
	public ISupportErrorInfo,
	public CComObjectRoot,
	public CComCoClass<Ramp,&CLSID_Ramp>,
#ifdef VBA
	public CApcProjectItem<Ramp>,
#endif
	public EventProxy<Ramp, &DIID_IRampEvents>,
	public IConnectionPointContainerImpl<Ramp>,
	public IProvideClassInfo2Impl<&CLSID_Ramp, &DIID_IRampEvents, &LIBID_VBATESTLib>,
	public ISelect,
	public IEditable,
	public Hitable,
	public IScriptable,
	public IHaveDragPoints,
	public IFireEvents,
	public IPerPropertyBrowsing // Ability to fill in dropdown in property browser
{
public:
	Ramp();
	virtual ~Ramp();

BEGIN_COM_MAP(Ramp)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IRamp)
	//COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
	COM_INTERFACE_ENTRY(IPerPropertyBrowsing)
	COM_INTERFACE_ENTRY(IProvideClassInfo)
	COM_INTERFACE_ENTRY(IProvideClassInfo2)
END_COM_MAP()

BEGIN_CONNECTION_POINT_MAP(Ramp)
	CONNECTION_POINT_ENTRY(DIID_IRampEvents)
END_CONNECTION_POINT_MAP()

STANDARD_DISPATCH_DECLARE
STANDARD_EDITABLE_DECLARES(eItemRamp)

//DECLARE_NOT_AGGREGATABLE(Ramp)
// Remove the comment from the line above if you don't want your object to
// support aggregation.

DECLARE_REGISTRY_RESOURCEID(IDR_Ramp)
// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

	//virtual HRESULT GetTypeName(BSTR *pVal);
	//virtual int GetDialogID();
	virtual void GetDialogPanes(Vector<PropertyPane> *pvproppane);

	void RenderOutline(Sur * const psur);
	virtual void RenderBlueprint(Sur *psur);

	virtual void ClearForOverwrite();

	void GetRgVertex(Vector<RenderVertex> * const pvv);
	Vertex2D *GetRampVertex(int &pcvertex, float ** const ppheight, bool ** const ppfCross, float ** const ppratio);

	void AddSideWall(Vector<HitObject> * const pvho, const Vertex2D * const pv1, const Vertex2D * const pv2, const float height1, const float height2, const float wallheight);

	void AddLine(Vector<HitObject> * const pvho, const Vertex2D * const pv1, const Vertex2D * const pv2, const Vertex2D * const pv3, const float height1, const float height2);

	virtual void MoveOffset(const float dx, const float dy);
	virtual void SetObjectPos();

	virtual void DoCommand(int icmd, int x, int y);

	virtual int GetMinimumPoints() {return 2;}

	virtual void FlipY(Vertex2D * const pvCenter);
	virtual void FlipX(Vertex2D * const pvCenter);
	virtual void Rotate(float ang, Vertex2D *pvCenter);
	virtual void Scale(float scalex, float scaley, Vertex2D *pvCenter);
	virtual void Translate(Vertex2D *pvOffset);

	virtual void GetCenter(Vertex2D * const pv) const {GetPointCenter(pv);}
	virtual void PutCenter(const Vertex2D * const pv) {PutPointCenter(pv);}

	void WriteRegDefaults();

	PinTable *m_ptable;

	RampData m_d;
		
	Vector<HitObject> m_vhoCollidable; // Objects to that may be collide selectable

	virtual void RenderShadow(ShadowSur * const psur, const float height);

	virtual void GetBoundingVertices(Vector<Vertex3Ds> * const pvvertex3D);

	void CheckJoint(Vector<HitObject> * const pvho, const Hit3DPoly * const ph3d1, const Hit3DPoly * const ph3d2);

	void RenderStaticHabitrail(const RenderDevice* _pd3dDevice);
	void RenderPolygons(const RenderDevice* _pd3dDevice, Vertex3D_NoTex * const rgv3D, WORD * const rgicrosssection, const int start, const int stop);

	//RECT invalidationRect;
	//bool invalidationRectCalculated;

// IRamp
public:
	STDMETHOD(get_Elasticity)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Elasticity)(/*[in]*/ float newVal);
	STDMETHOD(get_VisibleLeftWallHeight)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_VisibleLeftWallHeight)(/*[in]*/ float newVal);
	STDMETHOD(get_VisibleRightWallHeight)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_VisibleRightWallHeight)(/*[in]*/ float newVal);
	STDMETHOD(get_RightWallHeight)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_RightWallHeight)(/*[in]*/ float newVal);
	STDMETHOD(get_LeftWallHeight)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_LeftWallHeight)(/*[in]*/ float newVal);
	STDMETHOD(get_HasWallImage)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_HasWallImage)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_ImageAlignment)(/*[out, retval]*/ RampImageAlignment *pVal);
	STDMETHOD(put_ImageAlignment)(/*[in]*/ RampImageAlignment newVal);
	STDMETHOD(get_Image)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_Image)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_Type)(/*[out, retval]*/ RampType *pVal);
	STDMETHOD(put_Type)(/*[in]*/ RampType newVal);
	STDMETHOD(get_Color)(/*[out, retval]*/ OLE_COLOR *pVal);
	STDMETHOD(put_Color)(/*[in]*/ OLE_COLOR newVal);
	STDMETHOD(get_WidthTop)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_WidthTop)(/*[in]*/ float newVal);
	STDMETHOD(get_WidthBottom)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_WidthBottom)(/*[in]*/ float newVal);
	STDMETHOD(get_HeightTop)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_HeightTop)(/*[in]*/ float newVal);
	STDMETHOD(get_HeightBottom)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_HeightBottom)(/*[in]*/ float newVal);
//>>> added by chris
	STDMETHOD(get_CastsShadow)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_CastsShadow)(/*[in]*/ VARIANT_BOOL newVal);
//<<<
	STDMETHOD(get_Acrylic)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_Acrylic)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_Solid)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_Solid)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_Alpha)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_Alpha)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_Collidable)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_Collidable)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_IsVisible)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_IsVisible)(/*[in]*/ VARIANT_BOOL newVal);

	STDMETHOD(get_Friction)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Friction)(/*[in]*/ float newVal);
	STDMETHOD(get_Scatter)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Scatter)(/*[in]*/ float newVal);

	STDMETHOD(get_Modify3DStereo)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_Modify3DStereo)(/*[in]*/ VARIANT_BOOL newVal);

	STDMETHOD(get_AddBlend)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_AddBlend)(/*[in]*/ VARIANT_BOOL newVal);
};

#endif // !defined(AFX_RAMP_H__5EFEDEFB_5504_430A_B000_9B6D1903E3FC__INCLUDED_)
