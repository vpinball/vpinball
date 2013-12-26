// Flasher.h: Definition of the Flasher class
//
//////////////////////////////////////////////////////////////////////
#pragma once
#if !defined(AFX_FLASHER_H__87DAB93E_7D6F_4fe4_A5F9_632FD82BDB4A__INCLUDED_)
#define AFX_FLASHER_H__87DAB93E_7D6F_4fe4_A5F9_632FD82BDB4A__INCLUDED_

#include "resource.h"       // main symbols

class FlasherData
{
public:
   Vertex2D m_vCenter;
   float m_sizeX; 
   float m_sizeY;
   float m_height;
   float m_rotation;
   bool m_fDisplayTexture;
   COLORREF m_color;
	TimerDataRoot m_tdr;
	char m_szImage[MAXTOKEN];
   RECT m_boundRectangle;
	
	bool m_IsVisible;
	bool m_fAddBlend;
	long m_fAlpha;
   bool m_wasVisible;
	bool m_triggerUpdateRegion;
   bool m_triggerSingleUpdateRegion;
};

/////////////////////////////////////////////////////////////////////////////
// Flasher

class Flasher :
	public IDispatchImpl<IFlasher, &IID_IFlasher, &LIBID_VBATESTLib>,
	public ISupportErrorInfo,
	public CComObjectRoot,
	public CComCoClass<Flasher,&CLSID_Flasher>,
#ifdef VBA
	public CApcProjectItem<Flasher>,
#endif
	public EventProxy<Flasher, &DIID_IFlasherEvents>,
	public IConnectionPointContainerImpl<Flasher>,
	public IProvideClassInfo2Impl<&CLSID_Flasher, &DIID_IFlasherEvents, &LIBID_VBATESTLib>,
	public ISelect,
	public IEditable,
	public Hitable,
	public IScriptable,
	public IFireEvents,
	public IPerPropertyBrowsing // Ability to fill in dropdown in property browser
{
public:
	Flasher();
	virtual ~Flasher();

BEGIN_COM_MAP(Flasher)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IFlasher)
	//COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
	COM_INTERFACE_ENTRY(IPerPropertyBrowsing)
	COM_INTERFACE_ENTRY(IProvideClassInfo)
	COM_INTERFACE_ENTRY(IProvideClassInfo2)
END_COM_MAP()

BEGIN_CONNECTION_POINT_MAP(Flasher)
	CONNECTION_POINT_ENTRY(DIID_IFlasherEvents)
END_CONNECTION_POINT_MAP()

STANDARD_DISPATCH_DECLARE
STANDARD_EDITABLE_DECLARES(eItemFlasher)

//DECLARE_NOT_AGGREGATABLE(Flasher)
// Remove the comment from the line above if you don't want your object to
// support aggregation.

DECLARE_REGISTRY_RESOURCEID(IDR_Flasher)
// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

	//virtual HRESULT GetTypeName(BSTR *pVal);
	virtual void GetDialogPanes(Vector<PropertyPane> *pvproppane);

	virtual void RenderBlueprint(Sur *psur);

	virtual void MoveOffset(const float dx, const float dy);
	virtual void SetObjectPos();

	virtual int GetMinimumPoints() {return 2;}

   virtual void GetCenter(Vertex2D * const pv) const {*pv = m_d.m_vCenter;}
   virtual void PutCenter(const Vertex2D * const pv) {m_d.m_vCenter = *pv; m_ptable->SetDirtyDraw();}

	void WriteRegDefaults();

	PinTable *m_ptable;

	FlasherData m_d;
	int rampVertex;
	Vertex3D_NoTex2 *rgvbuf;
    
	Vertex2D *rgvInit;    // just for setup/static drawing
    float *rgheightInit,*rgratioInit;

	WORD *rgibuf;
	WORD *invrgibuf;

	VertexBuffer *staticVertexBuffer;
	VertexBuffer *dynamicVertexBuffer;
	BOOL dynamicVertexBufferRegenerate;
    Material solidMaterial;
    Material textureMaterial;
    Material habitrailMaterial;

	Vector<HitObject> m_vhoCollidable; // Objects to that may be collide selectable

// IFlasher
public:
	STDMETHOD(get_Image)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_Image)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_Color)(/*[out, retval]*/ OLE_COLOR *pVal);
	STDMETHOD(put_Color)(/*[in]*/ OLE_COLOR newVal);
	STDMETHOD(get_SizeX)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_SizeX)(/*[in]*/ float newVal);
   STDMETHOD(get_SizeY)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_SizeY)(/*[in]*/ float newVal);
   STDMETHOD(get_Height)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Height)(/*[in]*/ float newVal);
	STDMETHOD(get_X)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_X)(/*[in]*/ float newVal);
   STDMETHOD(get_Y)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Y)(/*[in]*/ float newVal);
   STDMETHOD(get_Rotation)(/*[out, retval]*/ float *pVal);
   STDMETHOD(put_Rotation)(/*[in]*/ float newVal);
	STDMETHOD(get_Alpha)(/*[out, retval]*/ long *pVal);
	STDMETHOD(put_Alpha)(/*[in]*/ long newVal);
   STDMETHOD(get_IsVisible)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_IsVisible)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_DisplayTexture)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_DisplayTexture)(/*[in]*/ VARIANT_BOOL newVal);

	STDMETHOD(get_UpdateRegions)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_UpdateRegions)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(TriggerSingleUpdate)();

	STDMETHOD(get_AddBlend)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_AddBlend)(/*[in]*/ VARIANT_BOOL newVal);
};

#endif // !defined(AFX_FLASHER_H__87DAB93E_7D6F_4fe4_A5F9_632FD82BDB4A__INCLUDED_)
