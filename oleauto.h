//---------------------------------------------------------------------------
// OleAuto.h
//---------------------------------------------------------------------------
// Simple class for doing dual OLE Automation interfaces
//---------------------------------------------------------------------------
/*	Copyright (C) Microsoft Corporation, 1999.  All rights reserved.


	This source code is intended only as a supplement to Microsoft
	Development Tools and/or on-line documentation.  See these other
	materials for detailed information regarding Microsoft code samples.
*/
#pragma once

//---------------------------------------------------------------------------
// A little utility which simplifies firing dispatch events.
//---------------------------------------------------------------------------
HRESULT InvokeEvent
  (
  IDispatch  *pdisp,    // IDispatch of Sink
  DISPID      dispid,   // DISPID of event
  VARIANTARG *pvararg,  // Args to event
  UINT        carg      // # args
  );


//---------------------------------------------------------------------------
// This routine will load a TypeLib and (optionally) find the TypeInfo inside
// which matches the given clsid.  The TypeLib and TypeInfo pointers are
// in/out so you can simply:
//      hr = LoadTypeInfo(..., &g_pMyTypeLib, &m_pMyObjectsTypeInfo);
// and it will fill in g_pMyTypeLib and m_pMyObjectsTypeInfo, if necessary.
//---------------------------------------------------------------------------
HRESULT LoadTypeInfo
  (
  HINSTANCE   hinst,             // hinst of where to load TypeLib from, if not found
  UINT        itinfo,            // index of TypeInfo requested, only 0 supported
  USHORT      dwMaj,             // Maj version # of TypeLib
  USHORT      dwMin,             // Min version # of TypeLib
  LCID        lcid,              // Locale of TypeLib to load
  REFGUID     libid,             // LIBID of TypeLib  to find
  REFCLSID    clsid,             // CLSID of TypeInfo to find
  REFIID      iid,               // IID   of TypeInfo to find
  BOOL        fDispOnly,         // TRUE=ensure *ptinfoIntInOut is a TKIND_DISPATCH, not vtbl
  ITypeLib  **pptlibInOut,       // Ptr to cache of pTypeLib, typically &g_ptlib
  ITypeInfo **pptinfoClassInOut, // Ptr to cache of pTypeInfo, typically &m_ptinfoCls
  ITypeInfo **pptinfoIntInOut    // Ptr to cache of pTypeInfo, typically &m_ptinfoInt
  );


//---------------------------------------------------------------------------
// Derrive from this class to get the standard implementation of IDispatch
// over a vtable.  Assumes that "this" is the proper vtable, so to use, your
// class should first derive from IMyCustomDualInterface, then from COleAuto.
// Then implement GetTypeLibInfo().
//---------------------------------------------------------------------------
class COleAuto : public IDispatch
  {
public:
  // *** IUnknown methods ***
  STDMETHOD(QueryInterface)(REFIID riid, void** ppvObj) PURE;
  STDMETHOD_(ULONG, AddRef)(void) PURE;
  STDMETHOD_(ULONG, Release)(void) PURE;

  // *** IDispatch methods ***
  STDMETHOD(GetTypeInfoCount)(UINT* pctinfo);
  STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo** pptinfoOut);
  STDMETHOD(GetIDsOfNames)(REFIID riid, OLECHAR** rgszNames, UINT cNames, LCID lcid, DISPID* prgdispid);
  STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr);

  // *** IProvideMultipleClassInfo methods ***
  //STDMETHOD(GetMultiTypeInfoCount)(ULONG *pc);
  //STDMETHOD(GetInfoOfIndex)(ULONG itinfo, DWORD dwFlags, ITypeInfo** pptinfoCoClass, DWORD* pdwTIFlags, ULONG* pcdispidReserved, IID* piidPrimary, IID* piidSource);

  // *** Pure virtual methods for derived class to implement ***
  virtual ITypeInfo **GetTinfoClsAddr(void) PURE;
  virtual ITypeInfo **GetTinfoIntAddr(void) PURE;
  virtual HRESULT GetTypeLibInfo(HINSTANCE *phinstOut, const GUID **pplibidOut, 
                                 SHORT *pwMajLibOut, SHORT *pwMinLibOut,
                                 const CLSID **ppclsidOut, const IID **ppiidOut, 
                                 ITypeLib ***ppptlOut) PURE;
  virtual IDispatch *GetPrimary(void) PURE;

  // *** Other methods for derived class to call ***
  HRESULT CheckTypeInfo(UINT itinfo, LCID lcid);
  HRESULT SetBaseObject(IDispatch *pdisp);

  // Member Variables
  IDispatch *m_pdispBaseObject;   // The object this class is extending
  };


//---------------------------------------------------------------------------
// Standard Dispatch
//---------------------------------------------------------------------------
// All objects should declare these in their class definitions so that they
// get standard implementations of IDispatch and ISupportErrorInfo.
//---------------------------------------------------------------------------
#define DECLARE_STANDARD_DISPATCH() \
    STDMETHOD(GetTypeInfoCount)(UINT *pctinfo) \
      { return COleAuto::GetTypeInfoCount(pctinfo); } \
    STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo **pptinfoOut) \
      { return COleAuto::GetTypeInfo(itinfo, lcid, pptinfoOut); } \
    STDMETHOD(GetIDsOfNames)(REFIID iid, OLECHAR **rgszNames, UINT cnames, LCID lcid, DISPID *rgdispid) \
      { return COleAuto::GetIDsOfNames(iid, rgszNames, cnames, lcid, rgdispid); } \
    STDMETHOD(Invoke)(DISPID dispid, REFIID iid, LCID lcid, WORD wFlags, DISPPARAMS *pdispparams, VARIANT *pVarResult, EXCEPINFO *pexcepinfo, UINT *puArgErr) \
      { return COleAuto::Invoke(dispid, iid, lcid, wFlags, pdispparams, pVarResult, pexcepinfo, puArgErr); }

//--- EOF -------------------------------------------------------------------
