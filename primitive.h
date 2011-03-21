// 3dprimitive.h: interface for the 3dprimitive class.
//
//////////////////////////////////////////////////////////////////////
#pragma once
//#if !defined(AFX_PRIMITIVE_H__31CD2D6B-9BDD-4B1B-BC62-B9DE588A0CAA__INCLUDED_)
#if !defined(AFX_PRIMITIVE_H__31CD2D6B_9BDD_4B1B_BC62_B9DE588A0CAA__INCLUDED_)
#define AFX_PRIMITIVE_H__31CD2D6B_9BDD_4B1B_BC62_B9DE588A0CAA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"

class PrimitiveData
	{
public:

	//Vertex2D m_vCenter;
	
	//external Variables
	int m_Sides;
	Vertex3D m_vPosition;
	Vertex3D m_vSize;
	Vertex3D m_vAxisScaleX;
	Vertex3D m_vAxisScaleY;
	Vertex3D m_vAxisScaleZ;
	float m_aRotAndTra[6];
	RotAndTraTypeEnum m_aRotAndTraTypes[6];
	//Vertex3D m_vRotation;
	//Vertex3D m_vTransposition;
	char m_szImage[MAXTOKEN];
	COLORREF m_TopColor;
	COLORREF m_SideColor;
	bool m_TopVisible;
	bool m_SideVisible;
	bool m_DrawTexturesInside;
	bool m_SmoothSideNormals;

	TimerDataRoot m_tdr;

	};

class Primitive :
	public IDispatchImpl<IPrimitive, &IID_IPrimitive, &LIBID_VBATESTLib>,
	public CComObjectRoot,
	public CComCoClass<Primitive,&CLSID_Primitive>,
#ifdef VBA
	//public CApcaaaControl<Primitive>,
	public CApcProjectItem<Primitive>,
#endif
	public EventProxy<Primitive, &DIID_IPrimitiveEvents>,
	public IConnectionPointContainerImpl<Primitive>,
	public IProvideClassInfo2Impl<&CLSID_Primitive, &DIID_IPrimitiveEvents, &LIBID_VBATESTLib>,

	public ISelect,
	public IEditable,
	public Hitable,
	public IScriptable,
	public IFireEvents,
	public IPerPropertyBrowsing // Ability to fill in dropdown in property browser
{
public:
	static const int Max_Primitive_Sides = 100;


	STDMETHOD(get_Sides)(/*[out, retval]*/ int *pVal);
	STDMETHOD(put_Sides)(/*[in]*/ int newVal);
	STDMETHOD(get_TopColor)(/*[out, retval]*/ OLE_COLOR *pVal);
	STDMETHOD(put_TopColor)(/*[in]*/ OLE_COLOR newVal);
	STDMETHOD(get_SideColor)(/*[out, retval]*/ OLE_COLOR *pVal);
	STDMETHOD(put_SideColor)(/*[in]*/ OLE_COLOR newVal);
	STDMETHOD(get_DrawTexturesInside)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_DrawTexturesInside)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_SmoothSideNormals)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_SmoothSideNormals)(/*[in]*/ VARIANT_BOOL newVal);

	STDMETHOD(get_TopVisible)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_TopVisible)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_SideVisible)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_SideVisible)(/*[in]*/ VARIANT_BOOL newVal);


	STDMETHOD(get_X)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_X)(/*[in]*/ float newVal);
	STDMETHOD(get_Y)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Y)(/*[in]*/ float newVal);
	STDMETHOD(get_Z)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Z)(/*[in]*/ float newVal);
	STDMETHOD(get_Size_X)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Size_X)(/*[in]*/ float newVal);
	STDMETHOD(get_Size_Y)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Size_Y)(/*[in]*/ float newVal);
	STDMETHOD(get_Size_Z)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Size_Z)(/*[in]*/ float newVal);
	STDMETHOD(get_AxisScaleX_Y)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_AxisScaleX_Y)(/*[in]*/ float newVal);
	STDMETHOD(get_AxisScaleX_Z)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_AxisScaleX_Z)(/*[in]*/ float newVal);
	STDMETHOD(get_AxisScaleY_X)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_AxisScaleY_X)(/*[in]*/ float newVal);
	STDMETHOD(get_AxisScaleY_Z)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_AxisScaleY_Z)(/*[in]*/ float newVal);
	STDMETHOD(get_AxisScaleZ_X)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_AxisScaleZ_X)(/*[in]*/ float newVal);
	STDMETHOD(get_AxisScaleZ_Y)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_AxisScaleZ_Y)(/*[in]*/ float newVal);
	
	STDMETHOD(get_RotAndTra0)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_RotAndTra0)(/*[in]*/ float newVal);
	STDMETHOD(get_RotAndTra1)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_RotAndTra1)(/*[in]*/ float newVal);
	STDMETHOD(get_RotAndTra2)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_RotAndTra2)(/*[in]*/ float newVal);
	STDMETHOD(get_RotAndTra3)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_RotAndTra3)(/*[in]*/ float newVal);
	STDMETHOD(get_RotAndTra4)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_RotAndTra4)(/*[in]*/ float newVal);
	STDMETHOD(get_RotAndTra5)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_RotAndTra5)(/*[in]*/ float newVal);

	STDMETHOD(get_RotAndTraType0)(/*[out, retval]*/ RotAndTraTypeEnum *pVal);
	STDMETHOD(put_RotAndTraType0)(/*[in]*/ RotAndTraTypeEnum newVal);
	STDMETHOD(get_RotAndTraType1)(/*[out, retval]*/ RotAndTraTypeEnum *pVal);
	STDMETHOD(put_RotAndTraType1)(/*[in]*/ RotAndTraTypeEnum newVal);
	STDMETHOD(get_RotAndTraType2)(/*[out, retval]*/ RotAndTraTypeEnum *pVal);
	STDMETHOD(put_RotAndTraType2)(/*[in]*/ RotAndTraTypeEnum newVal);
	STDMETHOD(get_RotAndTraType3)(/*[out, retval]*/ RotAndTraTypeEnum *pVal);
	STDMETHOD(put_RotAndTraType3)(/*[in]*/ RotAndTraTypeEnum newVal);
	STDMETHOD(get_RotAndTraType4)(/*[out, retval]*/ RotAndTraTypeEnum *pVal);
	STDMETHOD(put_RotAndTraType4)(/*[in]*/ RotAndTraTypeEnum newVal);
	STDMETHOD(get_RotAndTraType5)(/*[out, retval]*/ RotAndTraTypeEnum *pVal);
	STDMETHOD(put_RotAndTraType5)(/*[in]*/ RotAndTraTypeEnum newVal);

	STDMETHOD(get_Image)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_Image)(/*[in]*/ BSTR newVal);
	
	Primitive();
	virtual ~Primitive();

BEGIN_COM_MAP(Primitive)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IPrimitive)

	COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)

	COM_INTERFACE_ENTRY(IPerPropertyBrowsing)

	COM_INTERFACE_ENTRY(IProvideClassInfo)
	COM_INTERFACE_ENTRY(IProvideClassInfo2)
END_COM_MAP()
STANDARD_DISPATCH_DECLARE
STANDARD_EDITABLE_DECLARES(eItemPrimitive)


BEGIN_CONNECTION_POINT_MAP(Primitive)
	CONNECTION_POINT_ENTRY(DIID_IPrimitiveEvents)
END_CONNECTION_POINT_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_Primitive)

	virtual void MoveOffset(const float dx, const float dy);
	virtual void SetObjectPos();
	// Multi-object manipulation
	virtual void GetCenter(Vertex2D *pv);
	virtual void PutCenter(Vertex2D *pv);

	//STDMETHOD(get_Name)(BSTR *pVal) {return E_FAIL;}

	//virtual HRESULT InitVBA(BOOL fNew, int id, WCHAR *wzName);
	void WriteRegDefaults();
	//virtual HRESULT GetTypeName(BSTR *pVal);
	//virtual int GetDialogID();
	virtual void GetDialogPanes(Vector<PropertyPane> *pvproppane);

	//virtual IScriptable *GetScriptable() {return NULL;}

#ifdef VBA
	virtual IApcControl *GetIApcControl() {return ApcControl.GetApcControl();}
#endif



	PinTable *m_ptable;

	PrimitiveData m_d;
public:

	// Vertices for 3d Display
	Vertex3D rgv3DTopOriginal[Max_Primitive_Sides]; // without transformation
	Vertex3D rgv3DTop[Max_Primitive_Sides]; // with transformation
	WORD wTopIndices[Max_Primitive_Sides*6]; // *6 because of aech point could be a triangle (*3) and for both sides because of culling (*2)
	Vertex3D rgv3DBottomOriginal[Max_Primitive_Sides];
	Vertex3D rgv3DBottom[Max_Primitive_Sides];
	WORD wBottomIndices[Max_Primitive_Sides*6];

	// is top behind bottom?
	bool topBehindBottom;
	int farthestIndex;

	// Vertices for editor display
	Vector<Vertex3D> verticesTop;
	Vector<Vertex3D> verticesBottom;


	void RecalculateVertices();
	void CalculateRealTimeOriginal();
	void CalculateRealTime();
	
	Matrix3D fullMatrix;
	void RecalculateMatrices();

	RECT m_rcBounds; // For testing against lights

	PinImage m_pinimage;
	float m_leading, m_descent;
	float maxtu, maxtv;
	float m_realwidth, m_realheight;
};

#endif // !defined(AFX_PRIMITIVE_H__31CD2D6B-9BDD-4B1B-BC62-B9DE588A0CAA__INCLUDED_)
