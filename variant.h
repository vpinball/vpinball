#pragma once

BSTR BstrFromVariant(VARIANT *pvar, LCID lcid = 0x409/*LCID lcid = lcidUSEnglish*/);

// RESULT codes
#define S_FAIL				MAKE_SCODE(SEVERITY_ERROR, FACILITY_CONTROL, 1004)
#define hrNotImplemented	ResultFromScode(E_NOTIMPL)

// Variant helpers
#define SetVarBstr(_pvar,_bstr)		{VariantClear(_pvar);V_VT(_pvar)=VT_BSTR;(_pvar)->bstrVal=(_bstr);}
