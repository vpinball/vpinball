#ifndef __VARIANT_H__
#define __VARIANT_H__

// VARIANT
BOOL FVariantIsVt(const VARIANT *pvar, VARTYPE vt);
BOOL FVariantIsNumber(const VARIANT *pvar);
BOOL FVariantMissing(const VARIANT *pvar);
BOOL FVariantNullOrEmpty(const VARIANT *pvar);
HRESULT HrVariantToBool(VARIANT *pvar, BOOL *pf);
HRESULT HrVariantToInt(VARIANT *pvar, int *pint);
HRESULT HrVariantToInt(VARIANT *pvar, int iMin, int iMax, int *pint);
HRESULT HrVariantToDouble(VARIANT *pvar, double dblMin, double dblMax, double *pdouble);
HRESULT HrOptionalVarToInt(VARIANT *pvar, int *pi, int iDflt = 0);
HRESULT HrOptionalVarToBool(VARIANT *pvar, BOOL *pb, BOOL bDflt = 0);
HRESULT HrPSafeArrayFromVariant(VARIANT *pvar, SAFEARRAY ** pparray);
HRESULT HrVariantIsBstr(VARIANT * pvar, WCHAR ** pwz );
HRESULT HrVariantIsLong(VARIANT * pvar, long * pl);
HRESULT HrVariantIsDouble(VARIANT * pvar, double * pr);
IDispatch *DispFromVariant(VARIANT *pvar);
int CompareVariants(const VARIANT *pvar1, const VARIANT *pvar2, int msocs);
HRESULT HrCreateSafeArray(VARIANT *pvarDst, WCHAR **rgsz, int csz);
//HRESULT HrCreateSafeArrayVectorWz(VARIANT *pvarDst, VectorWz *pvwz);
BSTR BstrExtractFromVariant(VARIANT *pvar);
BSTR BstrFromVariant(VARIANT *pvar, LCID lcid = 0x409/*LCID lcid = lcidUSEnglish*/);
HRESULT HrSysAllocString(BSTR *pdest, const OLECHAR *);


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

// Automation booleans
/*#define vbTrue  (-1)
#define vbFalse (0)
#define VbBoolFromF(f)		(!(f) - 1)
#define FFromVbBool(vb)		((vb) != 0)
#define HrFromF(f)			((f) ? hrNoError : hrFail)*/

// Variant helpers
#define VariantAlloc(_pvar)			{_pvar = (VARIANT *) MsoPvAlloc(sizeof(VARIANT), dgMisc); if (_pvar != NULL) VariantInit(_pvar);}
#define VariantFree(_pvar)			{Assert(FValidVariant(_pvar)); MsoFreePv(_pvar);}

#define SetVarDisp(_pvar,_pdisp)	{VariantClear(_pvar);V_VT(_pvar)=VT_DISPATCH;(_pvar)->pdispVal=_pdisp;}
#define SetVarBool(_pvar,_f)		{VariantClear(_pvar);V_VT(_pvar)=VT_BOOL;(_pvar)->boolVal=(_f)?vbTrue:vbFalse;}
#define SetVarBoolByRef(_pvar,_pvb) {VariantClear(_pvar);V_VT(_pvar)=VT_BYREF|VT_BOOL;(_pvar)->pbool=(_pvb);}
#define SetVarInt(_pvar,_i)			{VariantClear(_pvar);V_VT(_pvar)=VT_I4;(_pvar)->lVal=(_i);}
#define SetVarLong(_pvar,_l)		{VariantClear(_pvar);V_VT(_pvar)=VT_I4;(_pvar)->lVal=(_l);}
#define SetVarUint(_pvar,_u)		{VariantClear(_pvar);V_VT(_pvar)=VT_UI4;(_pvar)->lVal=(_u);}
#define SetVarNull(_pvar)			{VariantClear(_pvar);V_VT(_pvar)=VT_NULL;}
#define SetVarMissing(_pvar)		{VariantClear(_pvar);V_VT(_pvar)=VT_ERROR;V_ERROR(_pvar)=DISP_E_PARAMNOTFOUND;}
#define SetVarBstr(_pvar,_bstr)		{VariantClear(_pvar);V_VT(_pvar)=VT_BSTR;(_pvar)->bstrVal=(_bstr);}
#define SetVarError(_pvar, _err)	{VariantClear(_pvar);V_VT(_pvar)=VT_ERROR;V_ERROR(_pvar) = _err;}
#define SetVarDouble(_pvar,_dbl)	{VariantClear(_pvar);V_VT(_pvar)=VT_R8;(_pvar)->dblVal=(_dbl);}

#define InitVarNull(_pvar)			{V_VT(_pvar)=VT_NULL;}
#define InitVarMissing(_pvar)		{V_VT(_pvar)=VT_ERROR;V_ERROR(_pvar)=DISP_E_PARAMNOTFOUND;}
#define InitVarBool(_pvar,_f)		{V_VT(_pvar)=VT_BOOL;(_pvar)->boolVal=VbBoolFromF(_f);}
#define InitVarInt(_pvar,_w)		{V_VT(_pvar)=VT_I4;(_pvar)->lVal=(_w);}
#define InitVarBstr(_pvar,_bstr)	{V_VT(_pvar)=VT_BSTR;(_pvar)->bstrVal=(_bstr);}
#define InitVarDisp(_pvar,_pdisp)	{V_VT(_pvar)=VT_DISPATCH;(_pvar)->pdispVal=pdisp;}
#define InitVarError(_pvar, _err)	{V_VT(_pvar)=VT_ERROR;V_ERROR(_pvar) = _err;}
#define InitVarDouble(_pvar,_dbl)	{V_VT(_pvar)=VT_R8;(_pvar)->dblVal=(_dbl);}
#define DeReferenceVar(_pvar)		{while (V_VT(_pvar)==(VT_VARIANT|VT_BYREF)) _pvar=V_VARIANTREF(_pvar);}
HRESULT HrNormalizeVariant(VARIANT *pvarNormal, VARIANT var);


// miow CompareVariant flags
#define mskmiowcs 0xffff0000
#define mskmsocs  0x0000ffff

#define miowcsExact        0x00000000
#define miowcsFuzzyCompare 0x00010000
#endif // __VARIANT_H__
