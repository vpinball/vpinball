// Decal.h: interface for the Decal class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DECAL_H__447B3CE2_C9EA_4ED1_AA3D_A8328F6DFD48__INCLUDED_)
#define AFX_DECAL_H__447B3CE2_C9EA_4ED1_AA3D_A8328F6DFD48__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class DecalData
	{
public:
	Vertex m_vCenter;
	float m_width, m_height;
	float m_rotation;
	char m_szImage[MAXTOKEN];
	char m_szSurface[MAXTOKEN];
	DecalType m_decaltype;
	//BOOL m_fImage;
	char m_sztext[MAXSTRING];
	SizingType m_sizingtype;
	//BOOL m_fAutoSize;
	//BOOL m_fAutoAspect;
	COLORREF m_color;
	BOOL m_fVerticalText;
	};

class Decal :
	public IDispatchImpl<IDecal, &IID_IDecal, &LIBID_VBATESTLib>,
	public CComObjectRoot,
#ifdef VBA
	public CApcControl<Decal>,
#endif
	public ISelect,
	public IEditable,
	public Hitable,
	public IScriptable,
	public IPerPropertyBrowsing // Ability to fill in dropdown in property browser
{
public:
	STDMETHOD(get_HasVerticalText)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_HasVerticalText)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_Font)(/*[out, retval]*/ IFontDisp **pVal);
	STDMETHOD(putref_Font)(/*[in]*/ IFontDisp *newVal);
	STDMETHOD(get_FontColor)(/*[out, retval]*/ OLE_COLOR *pVal);
	STDMETHOD(put_FontColor)(/*[in]*/ OLE_COLOR newVal);
	STDMETHOD(get_SizingType)(/*[out, retval]*/ SizingType *pVal);
	STDMETHOD(put_SizingType)(/*[in]*/ SizingType newVal);
	STDMETHOD(get_Text)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_Text)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_Type)(/*[out, retval]*/ DecalType *pVal);
	STDMETHOD(put_Type)(/*[in]*/ DecalType newVal);
	STDMETHOD(get_Surface)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_Surface)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_Y)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Y)(/*[in]*/ float newVal);
	STDMETHOD(get_X)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_X)(/*[in]*/ float newVal);
	STDMETHOD(get_Height)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Height)(/*[in]*/ float newVal);
	STDMETHOD(get_Width)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Width)(/*[in]*/ float newVal);
	STDMETHOD(get_Image)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_Image)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_Rotation)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Rotation)(/*[in]*/ float newVal);
	Decal();
	virtual ~Decal();

BEGIN_COM_MAP(Decal)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IDecal)
	COM_INTERFACE_ENTRY(IPerPropertyBrowsing)
END_COM_MAP()

STANDARD_NONAPC_EDITABLE_DECLARES(eItemDecal)

	virtual void MoveOffset(float dx, float dy);
	virtual void SetObjectPos();
	// Multi-object manipulation
	virtual void GetCenter(Vertex *pv);
	virtual void PutCenter(Vertex *pv);

	virtual void Rotate(float ang, Vertex *pvCenter);

	STDMETHOD(get_Name)(BSTR *pVal) {return E_FAIL;}

	virtual HRESULT InitVBA(BOOL fNew, int id, WCHAR *wzName);
	//virtual HRESULT GetTypeName(BSTR *pVal);
	//virtual int GetDialogID();
	virtual void GetDialogPanes(Vector<PropertyPane> *pvproppane);

	void EnsureSize();
	HFONT GetFont();
	void GetTextSize(int *px, int *py);

	virtual IScriptable *GetScriptable() {return NULL;}

#ifdef VBA
	virtual IApcControl *GetIApcControl() {return ApcControl.GetApcControl();}
#endif

	IFont *m_pIFont;

	PinTable *m_ptable;

	DecalData m_d;

	RECT m_rcBounds; // For testing against lights

	PinImage m_pinimage;
	float m_leading, m_descent;

	float m_realwidth, m_realheight;
};

#endif // !defined(AFX_DECAL_H__447B3CE2_C9EA_4ED1_AA3D_A8328F6DFD48__INCLUDED_)
