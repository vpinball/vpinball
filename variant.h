#pragma once
#ifndef __VARIANT_H__
#define __VARIANT_H__

BSTR BstrFromVariant(VARIANT *pvar, LCID lcid = 0x409/*LCID lcid = lcidUSEnglish*/);

// RESULT codes
#define S_FAIL				MAKE_SCODE(SEVERITY_ERROR, FACILITY_CONTROL, 1004)
#define hrNoError			S_OK
#define hrFail				S_FAIL
#define hrOutOfMemory		ResultFromScode(E_OUTOFMEMORY)
#define hrNoInterface		ResultFromScode(E_NOINTERFACE)
#define hrParamNotOptional	ResultFromScode(DISP_E_PARAMNOTOPTIONAL)
#define hrBadParamCount		ResultFromScode(DISP_E_BADPARAMCOUNT)
#define hrMemberNotFound	ResultFromScode(DISP_E_MEMBERNOTFOUND)
#define hrException			ResultFromScode(DISP_E_EXCEPTION)
#define hrBadIndex			ResultFromScode(DISP_E_BADINDEX)
#define hrUnknownLcid		ResultFromScode(DISP_E_UNKNOWNLCID)
#define hrTypeMismatch		ResultFromScode(DISP_E_TYPEMISMATCH)
#define hrInvalidArg		ResultFromScode(E_INVALIDARG)
#define hrNotImplemented	ResultFromScode(E_NOTIMPL)
#define hrFalse				ResultFromScode(S_FALSE)
#define hrUnexpected		ResultFromScode(E_UNEXPECTED)

// Variant helpers
#define SetVarBstr(_pvar,_bstr)		{VariantClear(_pvar);V_VT(_pvar)=VT_BSTR;(_pvar)->bstrVal=(_bstr);}

#endif // __VARIANT_H__
