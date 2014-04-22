// ComControl.h : Declaration of the CComControl
#pragma once

#ifndef __COMCONTROL_H_
#define __COMCONTROL_H_

#include "resource.h"       // main symbols

class PinComControl;

class MyCaxWindow : public CAxHostWindow
	{
public:
	void SetRect(RECT *prc);
	STDMETHOD(CanInPlaceActivate)();
	STDMETHOD(CanWindowlessActivate)();
	HRESULT CreateControl(LPCOLESTR lpszName, IStream* pStream = NULL, IUnknown** ppUnkContainer = NULL);
	STDMETHOD(AttachControl)(IUnknown* pUnkControl, HWND hWnd);
	HRESULT ActivateAx(IUnknown* pUnkControl, bool bInited, IStream* pStream);
	STDMETHOD_(void, OnViewChange)(DWORD dwAspect, LONG lindex);
	STDMETHOD(GetWindow)(HWND* phwnd);
	STDMETHOD(GetWindowContext)(IOleInPlaceFrame** ppFrame, IOleInPlaceUIWindow** ppDoc, LPRECT lprcPosRect, LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO pFrameInfo);
	STDMETHOD(ShowObject)();
	STDMETHOD(OnInPlaceActivateEx)(BOOL* pfNoRedraw, DWORD dwFlags);

	PinComControl *m_ppcc;
	RECT m_rc;
	};

class DispExtender : public IDispatch, public IProvideMultipleClassInfo
	{
public:
	DispExtender();
	~DispExtender();

	HRESULT STDMETHODCALLTYPE GetIDsOfNames(REFIID riid, OLECHAR FAR *FAR *rgszNames, unsigned int cNames, LCID lcid, DISPID FAR *rgDispId);
	HRESULT STDMETHODCALLTYPE GetTypeInfo(unsigned int iTInfo, LCID lcid, ITypeInfo FAR *FAR *ppTInfo);
	HRESULT STDMETHODCALLTYPE GetTypeInfoCount(unsigned int FAR *pctinfo);
	HRESULT STDMETHODCALLTYPE Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pDispParams, VARIANT FAR *pVarResult, EXCEPINFO FAR *pExcepInfo, unsigned int FAR *puArgErr);

	ULONG STDMETHODCALLTYPE AddRef();
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void **ppvObject);
	ULONG STDMETHODCALLTYPE Release();

	// *** IProvideClassInfo methods ***
	STDMETHOD(GetClassInfo)(ITypeInfo** pptinfoOut);
	// *** IProvideClassInfo2 methods ***
	STDMETHOD(GetGUID)(DWORD dwGuidKind, GUID* pGUID);
	STDMETHOD(GetMultiTypeInfoCount)(ULONG *pcti);
	STDMETHOD(GetInfoOfIndex)(ULONG iti, DWORD dwFlags, ITypeInfo** pptiCoClass, DWORD* pdwTIFlags, ULONG* pcdispidReserved, IID* piidPrimary, IID* piidSource);

	int m_cref;

	IDispatch *m_pdisp;
	IDispatch *m_pdispControl;

	ITypeInfo *m_pti;
	ITypeInfo *m_pti2;

	IDispatch *m_pdispEdit;
	IDispatch *m_pdispPlayer;
	};

class ComControlData
	{
public:
	Vertex2D m_v1, m_v2;
	TimerDataRoot m_tdr;
	WCHAR m_progid[50]; // progid can't be more than 39 chars, but just to make sure.
	CLSID m_clsid;
	};

/////////////////////////////////////////////////////////////////////////////
// CComControl
class PinComControl : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<PinComControl, &CLSID_ComControl>,
	public IDispatchImpl<IComControl, &IID_IComControl, &LIBID_VPinballLib>,
	public EventProxy<PinComControl, &DIID_IComControlEvents>,
	public IConnectionPointContainerImpl<PinComControl>,
	public IProvideClassInfo2Impl<&CLSID_ComControl, &DIID_IComControlEvents, &LIBID_VPinballLib>,
	public ISelect,
	public IEditable,
	public IScriptable,
	public IFireEvents,
	public Hitable//,
	//public IProvideMultipleClassInfo
{
public:
	PinComControl();

	virtual ~PinComControl();

DECLARE_REGISTRY_RESOURCEID(IDR_COM_CONTROL)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(PinComControl)
	COM_INTERFACE_ENTRY(IComControl)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
	COM_INTERFACE_ENTRY(IProvideClassInfo)
	COM_INTERFACE_ENTRY(IProvideClassInfo2)
	//COM_INTERFACE_ENTRY(IProvideMultipleClassInfo)
END_COM_MAP()

BEGIN_CONNECTION_POINT_MAP(PinComControl)
	CONNECTION_POINT_ENTRY(DIID_IComControlEvents)
END_CONNECTION_POINT_MAP()

STANDARD_EDITABLE_DECLARES(eItemComControl)

	// *** IProvideClassInfo methods ***
	/*STDMETHOD(GetClassInfo)(ITypeInfo** pptinfoOut);
	// *** IProvideClassInfo2 methods ***
	STDMETHOD(GetGUID)(DWORD dwGuidKind, GUID* pGUID);
	// *** IProvideMultipleClassInfo methods ***
	STDMETHOD(GetMultiTypeInfoCount)(ULONG *pcti);
	STDMETHOD(GetInfoOfIndex)(ULONG iti, DWORD dwFlags, ITypeInfo** pptiCoClass, DWORD* pdwTIFlags, ULONG* pcdispidReserved, IID* piidPrimary, IID* piidSource);*/

	virtual void GetDialogPanes(Vector<PropertyPane> *pvproppane);

	virtual void MoveOffset(const float dx, const float dy);
	virtual void SetObjectPos();
	// Multi-object manipulation
	virtual void GetCenter(Vertex2D * const pv) const;
	virtual void PutCenter(const Vertex2D * const pv);

	void FireVoidGroupEvent(int dispid) {} // Since we have no events right now

	inline IDispatch *GetDispatch() {return (IDispatch *)m_pdispextender;}

	void CreateControl(IStream *pstm);
	void CreateControlDialogTemplate();

	HRESULT ChooseComponent();

	ComControlData m_d;

	PinTable *m_ptable;

	//MyCaxWindow *pm_caxwindow;
	IUnknown *m_punk;

	CAxWindow m_caxwindowPlayer;

	LPDLGTEMPLATE m_ptemplate; // dialog template for control properties

	DispExtender *m_pdispextender;

	CComObject<MyCaxWindow> *m_pmcw;
	CComPtr<IAxWinHostWindow> m_pAxWindow;

	CComObject<MyCaxWindow> *m_pmcwPlayer;
	CComPtr<IAxWinHostWindow> m_pAxWindowPlayer;
	//TypeInfoExtender *m_ptiextenderEvents;
	//ITypeInfo *m_ptiPri; // cache of object's main TypeInfo
	//ITypeInfo *m_ptiSec; // cache of object's event TypeInfo

// IComControl
public:
	STDMETHOD(get_Y)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Y)(/*[in]*/ float newVal);
	STDMETHOD(get_X)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_X)(/*[in]*/ float newVal);
	STDMETHOD(get_Height)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Height)(/*[in]*/ float newVal);
	STDMETHOD(get_Width)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Width)(/*[in]*/ float newVal);
};

#endif //__COMCONTROL_H_
