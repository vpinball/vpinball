// Rubber.h: Definition of the Rubber class
//
//////////////////////////////////////////////////////////////////////
#pragma once
#if !defined(AFX_RUBBER_H__B0715DC0_002F_11E4_9191_0800200C9A66__INCLUDED_)
#define AFX_RUBBER_H__B0715DC0_002F_11E4_9191_0800200C9A66__INCLUDED_

#include "resource.h"       // main symbols

class RubberData
{
public:
    char m_szMaterial[32];
	TimerDataRoot m_tdr;
	float m_height; 
	int m_thickness;
	char m_szImage[MAXTOKEN];
	float m_elasticity;
	float m_friction;
	float m_scatter;
    float m_wireDiameter;
    float m_wireDistanceX;
    float m_wireDistanceY;
   bool m_staticRendering;

	bool m_fCollidable;
	bool m_fVisible;
	bool m_fImageWalls;
	bool m_fCastsShadow;
	//bool m_fAcrylic;
    float m_depthBias;      // for determining depth sorting
};

/////////////////////////////////////////////////////////////////////////////
// Rubber

class Rubber :
	public IDispatchImpl<IRubber, &IID_IRubber, &LIBID_VPinballLib>,
	public ISupportErrorInfo,
	public CComObjectRoot,
	public CComCoClass<Rubber,&CLSID_Rubber>,
	public EventProxy<Rubber, &DIID_IRubberEvents>,
	public IConnectionPointContainerImpl<Rubber>,
	public IProvideClassInfo2Impl<&CLSID_Rubber, &DIID_IRubberEvents, &LIBID_VPinballLib>,
	public ISelect,
	public IEditable,
	public Hitable,
	public IScriptable,
	public IHaveDragPoints,
	public IFireEvents,
	public IPerPropertyBrowsing // Ability to fill in dropdown in property browser
{
public:
	Rubber();
	virtual ~Rubber();

BEGIN_COM_MAP(Rubber)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IRubber)
	//COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
	COM_INTERFACE_ENTRY(IPerPropertyBrowsing)
	COM_INTERFACE_ENTRY(IProvideClassInfo)
	COM_INTERFACE_ENTRY(IProvideClassInfo2)
END_COM_MAP()

BEGIN_CONNECTION_POINT_MAP(Rubber)
	CONNECTION_POINT_ENTRY(DIID_IRubberEvents)
END_CONNECTION_POINT_MAP()

STANDARD_EDITABLE_DECLARES(Rubber, eItemRubber, RUBBER, 1)

//DECLARE_NOT_AGGREGATABLE(Rubber)
// Remove the comment from the line above if you don't want your object to
// support aggregation.

DECLARE_REGISTRY_RESOURCEID(IDR_RUBBER)
// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

	virtual void GetDialogPanes(Vector<PropertyPane> *pvproppane);

	void RenderOutline(Sur * const psur);
	virtual void RenderBlueprint(Sur *psur);

	virtual void ClearForOverwrite();

    float GetSurfaceHeight(float x, float y);

	virtual void MoveOffset(const float dx, const float dy);
	virtual void SetObjectPos();

	virtual void DoCommand(int icmd, int x, int y);

	virtual int GetMinimumPoints() const {return 2;}

	virtual void FlipY(Vertex2D * const pvCenter);
	virtual void FlipX(Vertex2D * const pvCenter);
	virtual void Rotate(float ang, Vertex2D *pvCenter);
	virtual void Scale(float scalex, float scaley, Vertex2D *pvCenter);
	virtual void Translate(Vertex2D *pvOffset);

	virtual void GetCenter(Vertex2D * const pv) const {GetPointCenter(pv);}
	virtual void PutCenter(const Vertex2D * const pv) {PutPointCenter(pv);}

	virtual void RenderShadow(ShadowSur * const psur, const float height);

	virtual void GetBoundingVertices(Vector<Vertex3Ds> * const pvvertex3D);

    virtual float GetDepth(const Vertex3Ds& viewDir) const;

	void WriteRegDefaults();

    // IHaveDragPoints
    virtual void GetPointDialogPanes(Vector<PropertyPane> *pvproppane);
    // end IHaveDragPoints

	PinTable *m_ptable;

	RubberData m_d;

private:
	int splinePoints;
	Vertex2D *rgvInit;    // just for setup/static drawing
    float *rgheightInit,*rgratioInit;

    int m_numVertices;      // this goes along with dynamicVertexBuffer
    int m_numIndices;

    std::vector<HitObject*> m_vhoCollidable; // Objects to that may be collide selectable

	VertexBuffer *staticVertexBuffer;
	VertexBuffer *dynamicVertexBuffer;
    IndexBuffer *dynamicIndexBuffer;
	bool dynamicVertexBufferRegenerate;

	void GetCentralCurve(Vector<RenderVertex> * const pvv);

    Vertex2D *GetSplineVertex(int &pcvertex, bool ** const ppfCross, Vertex2D **pMiddlePoints);
    void AddJoint(Vector<HitObject> * pvho, const Vertex3Ds& v1, const Vertex3Ds& v2);
    void AddJoint2D(Vector<HitObject> * pvho, const Vertex2D& p, float zlow, float zhigh);
	void CheckJoint(Vector<HitObject> * const pvho, const HitTriangle * const ph3d1, const HitTriangle * const ph3d2);

	void RenderPolygons(RenderDevice* pd3dDevice, int offset, WORD * const rgicrosssection, const int start, const int stop);

    void GenerateVertexBuffer(RenderDevice* pd3dDevice);
   
	void AddSideWall(Vector<HitObject> * const pvho, const Vertex2D * const pv1, const Vertex2D * const pv2, const float height1, const float height2, const float wallheight);
	void AddLine(Vector<HitObject> * const pvho, const Vertex2D * const pv1, const Vertex2D * const pv2, const Vertex2D * const pv3, const float height1, const float height2);

   void RenderObject( RenderDevice *pd3dDevice);
// IRamp
public:
	STDMETHOD(get_Image)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_Image)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_Material)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_Material)(/*[in]*/ BSTR newVal);
    STDMETHOD(get_Height)(/*[out, retval]*/ float *pVal);
    STDMETHOD(put_Height)(/*[in]*/ float newVal);
    STDMETHOD(get_Thickness)(/*[out, retval]*/ int *pVal);
    STDMETHOD(put_Thickness)(/*[in]*/ int newVal);
	STDMETHOD(get_CastsShadow)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_CastsShadow)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_Collidable)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_Collidable)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_Visible)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_Visible)(/*[in]*/ VARIANT_BOOL newVal);
    STDMETHOD(get_DepthBias)(/*[out, retval]*/ float *pVal);
    STDMETHOD(put_DepthBias)(/*[in]*/ float newVal);
    STDMETHOD(get_EnableStaticRendering)(/*[out, retval]*/ VARIANT_BOOL *pVal);
    STDMETHOD(put_EnableStaticRendering)(/*[in]*/ VARIANT_BOOL newVal);

    STDMETHOD(get_Elasticity)(/*[out, retval]*/ float *pVal);
    STDMETHOD(put_Elasticity)(/*[in]*/ float newVal);
	STDMETHOD(get_Friction)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Friction)(/*[in]*/ float newVal);
	STDMETHOD(get_Scatter)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Scatter)(/*[in]*/ float newVal);

};

#endif // !defined(AFX_RAMP_H__5EFEDEFB_5504_430A_B000_9B6D1903E3FC__INCLUDED_)
