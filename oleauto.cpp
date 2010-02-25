//---------------------------------------------------------------------------
// OleAuto.cpp
//---------------------------------------------------------------------------
// Simple class for doing dual OLE Automation interfaces
//---------------------------------------------------------------------------
/*	Copyright (C) Microsoft Corporation, 1999.  All rights reserved.


	This source code is intended only as a supplement to Microsoft
	Development Tools and/or on-line documentation.  See these other
	materials for detailed information regarding Microsoft code samples.
*/

#include "StdAfx.h"

#define NUM_RESERVED_EXTENDER_DISPIDS 500
#define EXTENDER_DISPID_BASE ((ULONG)(0x80010000))
#define IS_EXTENDER_DISPID(x) ( ( (ULONG)(x) & 0xFFFF0000 ) == EXTENDER_DISPID_BASE )

SZTHISFILE

//---------------------------------------------------------------------------
// A little utility which simplifies firing dispatch events.
//---------------------------------------------------------------------------
HRESULT InvokeEvent
(
  IDispatch  *pdisp,    // IDispatch of Sink
  DISPID      dispid,   // DISPID of event
  VARIANTARG *pvararg,  // Args to event
  UINT        carg      // # args
)
{
  DISPPARAMS dp;
  EXCEPINFO  ei;
  UINT       uArgErr = 0;

  dp.rgvarg            = pvararg;
  dp.rgdispidNamedArgs = NULL;
  dp.cArgs             = carg;
  dp.cNamedArgs        = 0;
  return pdisp->Invoke(dispid, IID_NULL, 0, DISPATCH_METHOD, &dp, NULL, &ei, &uArgErr);
}


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
  ITypeInfo **pptinfoClassInOut, // Ptr to cache of pTypeInfo, typically &s_ptinfoCls
  ITypeInfo **pptinfoIntInOut    // Ptr to cache of pTypeInfo, typically &s_ptinfoInt
)
{
  HRESULT    hr        = S_OK;
  ITypeInfo *ptinfoT   = NULL;
  TYPEATTR  *ptypeattr = NULL;

  // Arg checking
  if (itinfo != 0)
    return DISP_E_BADINDEX;

  if (!pptlibInOut)
    return E_POINTER;

  // Check to see if we have a TypeLib, load it if not.
  if (!*pptlibInOut)
    {
    hr = LoadRegTypeLib(libid, dwMaj, dwMin, lcid, pptlibInOut);
    if (hr)
      {
      //ASSERT(hinst, "hinst==NULL means in-memory TL; shouldn't have to load from resource");

      // If, for some reason, we failed to load the type library this way,
      // we're going to try and load the type library directly out of our
      // resources.  This has the advantage of going and re-setting all the
      // registry information again for us.
      DWORD dwPathLen;
      char  szDllPath[MAX_PATH];

      dwPathLen = GetModuleFileName(hinst, szDllPath, MAX_PATH);
      if (!dwPathLen)
        return E_FAIL;

      MAKE_WIDEPTR_FROMANSI(pwsz, szDllPath);
      CHECK(LoadTypeLib(pwsz, pptlibInOut));
      }
    }
  ASSERT(*pptlibInOut, "Don't have a pTypeLib");

  // If the user wants a coclass TypeInfo...
  if (pptinfoClassInOut)
    {
    // ...Check to see if we already have it, and find it in the TypeLib, if not.
    if (!*pptinfoClassInOut)
      {
      // Get coclass TypeInfo
      CHECK((*pptlibInOut)->GetTypeInfoOfGuid(clsid, pptinfoClassInOut));
      }
    }

  // If the user wants an interface TypeInfo...
  if (pptinfoIntInOut)
    {
    // ...Check to see if we already have it, and find it in the TypeLib, if not.
    if (!*pptinfoIntInOut)
      {
      // Get interface TypeInfo
      CHECK((*pptlibInOut)->GetTypeInfoOfGuid(iid, &ptinfoT));
      CHECK(ptinfoT->GetTypeAttr(&ptypeattr));

      // If we want a TKIND_DISPATCH, but we have a TKIND_INTERFACE, then switch if it's dual
      if (fDispOnly && ptypeattr->typekind == TKIND_INTERFACE)
        {
        // We're on a TKIND_INTERFACE but want a dispatch.  If this is not a dual interface,
        // we're out of luck, so stuff a NULL, and return S_FALSE.
        if (!(ptypeattr->wTypeFlags & TYPEFLAG_FDUAL))
          {
          *pptinfoIntInOut = NULL;
          hr = S_FALSE;
          }

        // This is a dual interface, so get the corresponding Disp interface;
        // 0xffffffff is a magic number which does this for us.
        HREFTYPE hrefType;
        CHECK(ptinfoT->GetRefTypeOfImplType(0xffffffff, &hrefType));
        CHECK(ptinfoT->GetRefTypeInfo(hrefType, pptinfoIntInOut));
        }
      else
        {
        *pptinfoIntInOut = ptinfoT;
        (*pptinfoIntInOut)->AddRef();
        }
      }
    }

CleanUp:
  // Release all frame variables
  if (ptypeattr)
    ptinfoT->ReleaseTypeAttr(ptypeattr);
  if (ptinfoT)
    ptinfoT->Release();

  return hr;
}



//***************************************************************************
// IDispatch Interface
//***************************************************************************

//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
STDMETHODIMP COleAuto::GetTypeInfoCount
(
  UINT* pctinfo
)
{
  // Validate args
  if (!pctinfo)
    return E_INVALIDARG;

  // Return requested info
  *pctinfo = 1;
  return S_OK;
}


//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
STDMETHODIMP COleAuto::GetTypeInfo
(
  UINT        itinfo,
  LCID        lcid,
  ITypeInfo** pptinfoOut
)
{
  HRESULT hr = S_OK;

  // Validate args
  if (!pptinfoOut)
    return E_INVALIDARG;
  *pptinfoOut = NULL;

  // Make sure we have the TypeInfo
  CHECK(this->CheckTypeInfo(itinfo, lcid));

  // Return requested info
  *pptinfoOut = *this->GetTinfoIntAddr();
  (*pptinfoOut)->AddRef();    // For *pptiOut

CleanUp:
  return hr;
}


//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
HRESULT COleAuto::CheckTypeInfo
(
  UINT itinfo,
  LCID lcid
)
{
  HRESULT      hr;
  HINSTANCE    hinst;
  SHORT        wMaj, wMin;
  const GUID  *plibid;
  const CLSID *pclsid;
  const IID   *piid;
  ITypeLib   **pptl;
  ITypeInfo  **pptinfoCls;
  ITypeInfo  **pptinfoInt;

  pptinfoCls = this->GetTinfoClsAddr();
  pptinfoInt = this->GetTinfoIntAddr();
  if (*pptinfoCls && *pptinfoInt)
    return S_OK;

  hr = GetTypeLibInfo(&hinst, &plibid, &wMaj, &wMin, &pclsid, &piid, &pptl);
  if (hr)
    return hr;

  return LoadTypeInfo(hinst, itinfo, wMaj, wMin, lcid, *plibid, *pclsid, *piid, FALSE,
                      pptl, pptinfoCls, pptinfoInt);
}


//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
STDMETHODIMP COleAuto::GetIDsOfNames
(
  REFIID    iid,
  OLECHAR** rgszNames,
  UINT      cNames,
  LCID      lcid,
  DISPID*   prgdispid
)
{
  HRESULT hr;

  // Validate Args
  if (iid != IID_NULL)
      return E_INVALIDARG;

  // Make sure we have the TypeInfo
  CHECK(this->CheckTypeInfo(0, lcid));

  // Use the standard provided routines to do all the work for us.
  hr = (*this->GetTinfoIntAddr())->GetIDsOfNames(rgszNames, cNames, prgdispid);

  // Try the base object iff there is a base object and the previous call failed
  if (FAILED(hr) && m_pdispBaseObject)
    {
    hr = m_pdispBaseObject->GetIDsOfNames(iid, rgszNames, cNames, lcid, prgdispid);
    if (SUCCEEDED(hr))
      {
      // Adjust extender dispid.
      if (IS_EXTENDER_DISPID(prgdispid[0]))
        prgdispid[0] += NUM_RESERVED_EXTENDER_DISPIDS;
      }
    }

CleanUp:
  return hr;
}


//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
STDMETHODIMP COleAuto::Invoke
(
  DISPID      dispidMember,
  REFIID      iid,
  LCID        lcid,
  WORD        wFlags,
  DISPPARAMS* pdispparams,
  VARIANT*    pvarResult,
  EXCEPINFO*  pexcepinfo,
  UINT*       puArgErr
)
{
  HRESULT hr;

  // Validate args, clear out params
  if (!pdispparams)
    return E_INVALIDARG;
  if (pvarResult)
    VariantInit(pvarResult);
  SetErrorInfo(0L, NULL);
  if (puArgErr)
    *puArgErr = 0;
  if (iid != IID_NULL)
    return E_INVALIDARG;

  // Make sure we have the TypeInfo
  CHECK(this->CheckTypeInfo(0, lcid));

  // Use the standard provided routines to do all the work for us.
  hr = (*this->GetTinfoIntAddr())->Invoke(this->GetPrimary(), dispidMember, wFlags,
                                          pdispparams, pvarResult, pexcepinfo, puArgErr);
  // Try the base object iff there is a base object and the previous invoke failed
  if (FAILED(hr) && m_pdispBaseObject)
    {
    if (IS_EXTENDER_DISPID(dispidMember))
      dispidMember -= NUM_RESERVED_EXTENDER_DISPIDS;

    hr = m_pdispBaseObject->Invoke(dispidMember, iid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
    }

CleanUp:
  return hr;
}


//***************************************************************************
// IProvideMultipleClassInfo Interface
//***************************************************************************

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
/*STDMETHODIMP COleAuto::GetMultiTypeInfoCount
(
  ULONG *pc
)
  {
  HRESULT hr;
  ULONG count;
  IProvideClassInfo         *pclsinfo   = NULL;
  IProvideMultipleClassInfo *pclsinfoex = NULL;

  // This should not be possible!
  ASSERT(m_pdispBaseObject != NULL, "");

  hr = m_pdispBaseObject->QueryInterface(IID_IProvideMultipleClassInfo, (void**)&pclsinfoex);
  if (SUCCEEDED(hr))
    {
    hr = pclsinfoex->GetMultiTypeInfoCount(&count);
    if (SUCCEEDED(hr))
      *pc = count + 1;
    }
  else
    {
    hr = m_pdispBaseObject->QueryInterface(IID_IProvideClassInfo, (void**)&pclsinfo);
    if (FAILED(hr))
      goto Exit;
    *pc = 2;
    }

Exit:
  // Release the temp refs
  if (pclsinfoex)
    pclsinfoex->Release();
  if (pclsinfo)
    pclsinfo->Release();
  return hr;
  }


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
STDMETHODIMP COleAuto::GetInfoOfIndex
(
  ULONG       itinfo,
  DWORD       dwFlags,
  ITypeInfo** pptinfoCoClass,
  DWORD*      pdwTIFlags,
  ULONG*      pcdispidReserved,
  IID*        piidPrimary,
  IID*        piidSource
)
  {
  HRESULT                    hr         = S_OK;
  IProvideClassInfo         *pclsinfo   = NULL;
  IProvideMultipleClassInfo *pclsinfoex = NULL;

  // This should not be possible!
  ASSERT(m_pdispBaseObject != NULL, "");

  if (itinfo == 0)
    {
    // Return info on ourselves
    if (dwFlags & MULTICLASSINFO_GETTYPEINFO)
      {
      // Make sure we have the TypeInfo
      hr = this->CheckTypeInfo(0, 0x0409);
      if (hr)
        goto Exit;

      // Return requested info
      *pptinfoCoClass = *this->GetTinfoClsAddr();
      (*pptinfoCoClass)->AddRef();    // For *pptiOut
      if (pdwTIFlags)
        *pdwTIFlags = 0;
      }
    if (dwFlags & MULTICLASSINFO_GETNUMRESERVEDDISPIDS)
      *pcdispidReserved = NUM_RESERVED_EXTENDER_DISPIDS;
    if (dwFlags & MULTICLASSINFO_GETIIDPRIMARY)
      // REVIEW: Not implemented
      *piidPrimary = IID_NULL;
    if (dwFlags & MULTICLASSINFO_GETIIDSOURCE)
      // REVIEW: Not implemented
      *piidSource = IID_NULL;
    else
      {
      // Look for an extender
      hr = m_pdispBaseObject->QueryInterface(IID_IProvideMultipleClassInfo, (void**)&pclsinfoex);
      if (SUCCEEDED(hr))
        {
        hr = pclsinfoex->GetInfoOfIndex(itinfo-1, dwFlags, pptinfoCoClass, pdwTIFlags,
                                       pcdispidReserved, piidPrimary, piidSource);
        if (SUCCEEDED(hr) && (dwFlags & MULTICLASSINFO_GETTYPEINFO)) 
          *pdwTIFlags = TIFLAGS_EXTENDDISPATCHONLY;
	}
      else
        {
        if (itinfo != 1)
          hr = TYPE_E_ELEMENTNOTFOUND;

        hr = m_pdispBaseObject->QueryInterface(IID_IProvideClassInfo, (void**)&pclsinfo);
        if (FAILED(hr))
          goto Exit;
	        
        // Return info on unextended base object
        if (dwFlags & MULTICLASSINFO_GETTYPEINFO)
          {
          hr = pclsinfo->GetClassInfo(pptinfoCoClass);
          if (FAILED(hr))
            goto Exit;
          if (pdwTIFlags)
            *pdwTIFlags = TIFLAGS_EXTENDDISPATCHONLY;
          }
        if (dwFlags & MULTICLASSINFO_GETNUMRESERVEDDISPIDS)
          *pcdispidReserved = 0;
        if (dwFlags & MULTICLASSINFO_GETIIDPRIMARY)
          // REVIEW: Not implemented
          *piidPrimary = IID_NULL;
        if (dwFlags & MULTICLASSINFO_GETIIDSOURCE)
          // REVIEW: Not implemented
          *piidSource = IID_NULL;
        }
      }
    }

Exit:
  // Release the temp refs
  if (pclsinfoex)
    pclsinfoex->Release();
  if (pclsinfo)
    pclsinfo->Release();
  return hr;
  }*/



//***************************************************************************
// Misc 
//***************************************************************************

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
HRESULT COleAuto::SetBaseObject
(
  IDispatch *pdisp
)
  {
  if (!pdisp)
    {
    if (m_pdispBaseObject)
      {
      IDispatch* pdispTemp = m_pdispBaseObject;

      m_pdispBaseObject = NULL;
      pdispTemp->Release();
      }	
    }
  else
    {
    // Only allow the base object to be set once
    if (m_pdispBaseObject)
      return E_UNEXPECTED;
    m_pdispBaseObject = pdisp;
    m_pdispBaseObject->AddRef();
    }

  return S_OK;
  }

//--- EOF -------------------------------------------------------------------
