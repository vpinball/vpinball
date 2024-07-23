
/*
 * Portions of this code was derived from the Active Template Library
 * Copyright (C) Microsoft Corporation
 *
 * https://github.com/adzm/atlmfc/blob/master/include/atlsafe.h
 * https://learn.microsoft.com/en-us/cpp/atl/reference/ccomsafearraybound-class
 * https://learn.microsoft.com/en-us/cpp/atl/reference/ccomsafearray-class
 */

#pragma once

#include <atlbase.h>

template <typename T>
struct _ATL_AutomationType { };

#define DEFINE_AUTOMATION_TYPE_FUNCTION(ctype, typewrapper, oleautomationtype) \
  template <>                                                                  \
  struct _ATL_AutomationType<ctype> {                                          \
    typedef typewrapper _typewrapper;                                          \
    enum { type = oleautomationtype };                                         \
    static void* GetT(const ctype& t) throw() { return (void*)&t; }            \
  };

DEFINE_AUTOMATION_TYPE_FUNCTION(VARIANT, CComVariant, VT_VARIANT)

class CComSafeArrayBound : public SAFEARRAYBOUND {
  public:
	CComSafeArrayBound(ULONG ulCount = 0, LONG lLowerBound = 0) throw()
	{
		cElements = ulCount;
		lLbound = lLowerBound;
	}
	CComSafeArrayBound& operator=(const CComSafeArrayBound& bound) throw()
	{
		cElements = bound.cElements;
		lLbound = bound.lLbound;
		return *this;
	}
	CComSafeArrayBound& operator=(ULONG ulCount) throw() 
	{
		cElements = ulCount;
		lLbound = 0;
		return *this;
	}
	ULONG GetCount() const throw()
	{
		return cElements;
	}
	ULONG SetCount(ULONG ulCount) throw()
	{
		cElements = ulCount;
		return cElements;
	}
	LONG GetLowerBound() const throw()
	{
		return lLbound;
	}
	LONG SetLowerBound(LONG lLowerBound) throw()
	{
		lLbound = lLowerBound;
		return lLbound;
	}
	LONG GetUpperBound() const throw()
	{
		return lLbound + cElements - 1;
	}
};

template <typename T, VARTYPE _vartype = _ATL_AutomationType<T>::type>
class CComSafeArray {
public:
  CComSafeArray() throw() : m_psa(NULL) {}
  CComSafeArray(ULONG ulCount, LONG lLBound = 0) : m_psa(NULL) {
    CComSafeArrayBound bound(ulCount, lLBound);
    HRESULT hRes = Create(&bound);
  }
  ~CComSafeArray() {
    Destroy();
  }

  HRESULT Create(const SAFEARRAYBOUND *pBound, UINT uDims = 1)
  {
    if(m_psa != NULL)
	  return E_FAIL;

    if(pBound == NULL || uDims == 0)
      return E_INVALIDARG;

    HRESULT hRes = S_OK;
    m_psa = SafeArrayCreate(_vartype, uDims, const_cast<LPSAFEARRAYBOUND>(pBound));
    if (NULL == m_psa)
      hRes = E_OUTOFMEMORY;
    else
      hRes = Lock();
    return hRes;
  }

  HRESULT Destroy()
  {
    HRESULT hRes = S_OK;
    if (m_psa != NULL)
    {
      hRes = Unlock();
      if (SUCCEEDED(hRes))
      {
        hRes = SafeArrayDestroy(m_psa);
        if (SUCCEEDED(hRes))
          m_psa = NULL;
        }
    }
    return hRes;
  }

  const typename _ATL_AutomationType<T>::_typewrapper& operator[](LONG nIndex) const
  {
    return GetAt(nIndex);
  }

  typename _ATL_AutomationType<T>::_typewrapper& operator[](LONG nIndex)
  {
    return GetAt(nIndex);
  }

  typename _ATL_AutomationType<T>::_typewrapper& GetAt(LONG lIndex)
  {
    LONG lLBound = GetLowerBound();
    return ((CComVariant*)m_psa->pvData)[lIndex - lLBound];
  }

  LONG GetLowerBound(UINT uDim = 0) const 
  {
    LONG lLBound = 0;
    HRESULT hRes = SafeArrayGetLBound(m_psa, uDim + 1, &lLBound);
    return lLBound;
  }

  LONG GetUpperBound(UINT uDim = 0) const 
  {
    LONG lUBound = 0;
    HRESULT hRes = SafeArrayGetUBound(m_psa, uDim + 1, &lUBound);
    return lUBound;
  }

  LPSAFEARRAY Detach() 
  {
    Unlock();
    LPSAFEARRAY pTemp = m_psa;
    m_psa = NULL;
    return pTemp;
  }

protected:
  HRESULT Lock()
  {
    return SafeArrayLock(m_psa);
  }

  HRESULT Unlock()
  {
    return SafeArrayUnlock(m_psa);
  }

public:
  LPSAFEARRAY m_psa;
};