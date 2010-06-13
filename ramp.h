#pragma once
// Ramp.h: Definition of the Ramp class
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RAMP_H__5EFEDEFB_5504_430A_B000_9B6D1903E3FC__INCLUDED_)
#define AFX_RAMP_H__5EFEDEFB_5504_430A_B000_9B6D1903E3FC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"       // main symbols


//#define		RAMP_RENDER8BITALPHA						// Render acrylic ramps with an 8 bit alpha.


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
	float m_leftwallheight;
	float m_rightwallheight;
	float m_leftwallheightvisible;
	float m_rightwallheightvisible;
	float m_elasticity;
	float m_friction;
	float m_scatter;
	BOOL m_fCollidable;
	BOOL m_IsVisible;
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

	void RenderOutline(Sur *psur);
	virtual void RenderBlueprint(Sur *psur);

	virtual void ClearForOverwrite();

	void GetRgVertex(Vector<RenderVertex> *pvv);
	Vertex2D *GetRampVertex(int *pcvertex, float **ppheight, BOOL **ppfCross, float **ppratio);

	void Ramp::AddSideWall(Vector<HitObject> *pvho, Vertex2D *pv1, Vertex2D *pv2,float height1,float height2, float wallheight);

	void AddLine(Vector<HitObject> *pvho, Vertex2D *pv1, Vertex2D *pv2, Vertex2D *pv3, float height1, float height2);

	virtual void MoveOffset(float dx, float dy);
	virtual void SetObjectPos();

	virtual void DoCommand(int icmd, int x, int y);

	virtual int GetMinimumPoints() {return 2;}

	virtual void FlipY(Vertex2D *pvCenter);
	virtual void FlipX(Vertex2D *pvCenter);
	virtual void Rotate(float ang, Vertex2D *pvCenter);
	virtual void Scale(float scalex, float scaley, Vertex2D *pvCenter);
	virtual void Translate(Vertex2D *pvOffset);

	virtual void GetCenter(Vertex2D *pv) {GetPointCenter(pv);}
	virtual void PutCenter(Vertex2D *pv) {PutPointCenter(pv);}

	PinTable *m_ptable;

	RampData m_d;
		
	Vector<HitObject> m_vhoCollidable; // Objects to that may be collide selectable

	virtual void RenderShadow(ShadowSur *psur, float height);

	Vector<Level> m_vlevel;

	virtual void GetBoundingVertices(Vector<Vertex3D> *pvvertex3D);

	void CheckJoint(Vector<HitObject> *pvho, Hit3DPoly *ph3d1, Hit3DPoly *ph3d2);

	void RenderAcrylic(LPDIRECT3DDEVICE7 pd3dDevice);
	void RenderStaticHabitrail(LPDIRECT3DDEVICE7 pd3dDevice);
	void RenderPolygons(LPDIRECT3DDEVICE7 pd3dDevice, Vertex3D *rgv3D, int *rgicrosssection, int start, int stop);

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
	STDMETHOD(get_Collidable)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_Collidable)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_IsVisible)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_IsVisible)(/*[in]*/ VARIANT_BOOL newVal);

	STDMETHOD(get_Friction)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Friction)(/*[in]*/ float newVal);
	STDMETHOD(get_Scatter)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Scatter)(/*[in]*/ float newVal);
};

#endif // !defined(AFX_RAMP_H__5EFEDEFB_5504_430A_B000_9B6D1903E3FC__INCLUDED_)
