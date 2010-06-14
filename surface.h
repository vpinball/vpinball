#pragma once
// Surface.h : Declaration of the Surface

#ifndef __SURFACE_H_
#define __SURFACE_H_


#include "resource.h"       // main symbols

class SurfaceData
	{
public:
	BOOL m_fInner; // Inside or outside wall;
	Vertex2D m_Center;
	TimerDataRoot m_tdr;
	BOOL m_fHitEvent;
	float m_threshold;			// speed at which ball needs to hit to register a hit
	float m_slingshot_threshold;	// speed at which ball needs to trigger slingshot 
	char m_szImage[MAXTOKEN];
	char m_szSideImage[MAXTOKEN];
	ImageAlignment m_ia;
	COLORREF m_sidecolor;
	float m_heightbottom;
	float m_heighttop;
	COLORREF m_topcolor;
	BOOL m_fDroppable;
	BOOL m_fFlipbook;
	BOOL m_fFloor;
	BOOL m_fDisplayTexture; //In editor
	float m_slingshotforce;
	float m_elasticity;
	float m_friction;
	float m_scatter;
	BOOL m_fCastsShadow;
	BOOL m_fVisible;
	BOOL m_fSideVisible;
	BOOL m_fEnabled;
	BOOL m_fCollidable; //wall must be droppable too!
	BOOL m_fSlingshotAnimation;
	COLORREF m_slingshotColor;
	};

/////////////////////////////////////////////////////////////////////////////
// Surface
class ATL_NO_VTABLE Surface :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<Surface, &CLSID_Wall>,
	public IDispatchImpl<IWall, &IID_IWall, &LIBID_VBATESTLib>,
#ifdef VBA
	public CApcProjectItem<Surface>,
#endif
	public EventProxy<Surface, &DIID_IWallEvents>,
	public IConnectionPointContainerImpl<Surface>,
	public IProvideClassInfo2Impl<&CLSID_Wall, &DIID_IWallEvents, &LIBID_VBATESTLib>,
	public ISelect,
	public IEditable,
	public Hitable,
	public IHaveDragPoints,
	public IScriptable,
	public IFireEvents,
	public IPerPropertyBrowsing // Ability to fill in dropdown in property browser
	//public EditableImpl<Surface>
{
public:
	Surface();
	virtual ~Surface();

	HRESULT InitTarget(PinTable *ptable, float x, float y);

	//int GetPointCount();
	//Vertex2D *GetRgVertex(int *pcount);
	//void GetRgVertex(Vector<RenderVertex> *pvv);

	//RenderVertex *GetRgRenderVertex(int *pcount);

	virtual void ClearForOverwrite();

	virtual void MoveOffset(float dx, float dy);

	virtual void RenderShadow(ShadowSur *psur, float height);

	virtual void GetBoundingVertices(Vector<Vertex3D> *pvvertex3D);

	// IHitable
	void CurvesToShapes(Vector<HitObject> *pvho);
	void AddLine(Vector<HitObject> * const pvho, const RenderVertex * const pv1, const RenderVertex * const pv2, const RenderVertex * const pv3, const BOOL fSlingshot);

	STANDARD_DISPATCH_DECLARE
	STANDARD_EDITABLE_DECLARES(eItemSurface)

BEGIN_COM_MAP(Surface)
	COM_INTERFACE_ENTRY(IWall)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
	COM_INTERFACE_ENTRY(IPerPropertyBrowsing)
	COM_INTERFACE_ENTRY(IProvideClassInfo)
	COM_INTERFACE_ENTRY(IProvideClassInfo2)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_SURFACE)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_CONNECTION_POINT_MAP(Surface)
	CONNECTION_POINT_ENTRY(DIID_IWallEvents)
END_CONNECTION_POINT_MAP()

	//virtual HRESULT GetTypeName(BSTR *pVal);
	//virtual int GetDialogID();
	virtual void GetDialogPanes(Vector<PropertyPane> *pvproppane);
	// From IHaveDragPoints
	virtual void GetPointDialogPanes(Vector<PropertyPane> *pvproppane);

	virtual void RenderSlingshots(LPDIRECT3DDEVICE7 pd3dDevice);
	virtual ObjFrame *RenderWallsAtHeight(LPDIRECT3DDEVICE7 pd3dDevice, BOOL fMover, BOOL fDrop);

	virtual void RenderBlueprint(Sur *psur);

	virtual void DoCommand(int icmd, int x, int y);

	virtual void FlipY(Vertex2D *pvCenter);
	virtual void FlipX(Vertex2D *pvCenter);
	virtual void Rotate(float ang, Vertex2D *pvCenter);
	virtual void Scale(float scalex, float scaley, Vertex2D *pvCenter);
	virtual void Translate(Vertex2D *pvOffset);

	virtual void GetCenter(Vertex2D *pv) {GetPointCenter(pv);}
	virtual void PutCenter(Vertex2D *pv) {PutPointCenter(pv);}

	//void CheckIntersecting();

	PinTable *m_ptable;

	BSTR m_bstrName;

	Vertex2D *m_rgvT; // keeps vertices around between PreRender and Render
	int m_cvertexT;

	SurfaceData m_d;

	BOOL m_fSelfIntersecting;

	Vector<LineSegSlingshot> m_vlinesling;

	BOOL m_fIsDropped;
	BOOL m_fDisabled;
	
	Hit3DPolyDrop *m_phitdrop;
	Vector<HitObject> m_vhoDrop; // Objects to disable when dropped
	Vector<HitObject> m_vhoCollidable; // Objects to that may be collide selectable

// ISurface
public:
	STDMETHOD(get_SideVisible)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_SideVisible)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_SideImage)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_SideImage)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_Visible)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_Visible)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_Elasticity)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Elasticity)(/*[in]*/ float newVal);

	STDMETHOD(get_Friction)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Friction)(/*[in]*/ float newVal);
	STDMETHOD(get_Scatter)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Scatter)(/*[in]*/ float newVal);

	STDMETHOD(get_SlingshotStrength)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_SlingshotStrength)(/*[in]*/ float newVal);
	STDMETHOD(get_DisplayTexture)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_DisplayTexture)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_IsDropped)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_IsDropped)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_CanDrop)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_CanDrop)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_FlipbookAnimation)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_FlipbookAnimation)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_FaceColor)(/*[out, retval]*/ OLE_COLOR *pVal);
	STDMETHOD(put_FaceColor)(/*[in]*/ OLE_COLOR newVal);
	STDMETHOD(get_HeightTop)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_HeightTop)(/*[in]*/ float newVal);
	STDMETHOD(get_HeightBottom)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_HeightBottom)(/*[in]*/ float newVal);
	STDMETHOD(get_ImageAlignment)(/*[out, retval]*/ ImageAlignment *pVal);
	STDMETHOD(put_ImageAlignment)(/*[in]*/ ImageAlignment newVal);
	STDMETHOD(get_SideColor)(/*[out, retval]*/ OLE_COLOR *pVal);
	STDMETHOD(put_SideColor)(/*[in]*/ OLE_COLOR newVal);
	STDMETHOD(get_Image)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_Image)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_Threshold)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Threshold)(/*[in]*/ float newVal);
	STDMETHOD(get_HasHitEvent)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_HasHitEvent)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_CastsShadow)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_CastsShadow)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_Disabled)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_Disabled)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_Collidable)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_Collidable)(/*[in]*/ VARIANT_BOOL newVal);

	STDMETHOD (get_SlingshotThreshold)(/*[out, retval]*/ float *pVal);
	STDMETHOD (put_SlingshotThreshold)(/*[in]*/ float newVal);
	STDMETHOD (get_SlingshotAnimation)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD (put_SlingshotAnimation)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_SlingshotColor)(/*[out, retval]*/ OLE_COLOR *pVal);
	STDMETHOD(put_SlingshotColor)(/*[in]*/ OLE_COLOR newVal);

};

#endif //__SURFACE_H_
