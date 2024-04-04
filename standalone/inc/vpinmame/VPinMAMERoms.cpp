#include "stdafx.h"
#include "VPinMAMERoms.h"

VPinMAMERoms::VPinMAMERoms()
{
}

VPinMAMERoms::~VPinMAMERoms()
{
}

STDMETHODIMP VPinMAMERoms::get_Count(LONG* pnCount)
{
   PLOGW << "Not implemented";

   return E_NOTIMPL;
}

STDMETHODIMP VPinMAMERoms::get_Item(VARIANT *pKey, IRom **pRom)
{
   PLOGW << "Not implemented";

   return E_NOTIMPL;
}

STDMETHODIMP VPinMAMERoms::get__NewEnum(IUnknown** ppunkEnum)
{
   PLOGW << "Not implemented";

   return E_NOTIMPL;
}

STDMETHODIMP VPinMAMERoms::get_Available(VARIANT_BOOL *pVal)
{
   PLOGW << "Not implemented";

   return E_NOTIMPL;
}

STDMETHODIMP VPinMAMERoms::get_State(LONG *pVal)
{
   PLOGW << "Not implemented";

   return E_NOTIMPL;
}

STDMETHODIMP VPinMAMERoms::get_StateDescription(BSTR *pVal)
{
   PLOGW << "Not implemented";

   return E_NOTIMPL;
}

STDMETHODIMP VPinMAMERoms::Audit(VARIANT_BOOL fStrict)
{
   PLOGW << "Not implemented";

   return E_NOTIMPL;
}
