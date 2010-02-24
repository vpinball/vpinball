#include "stdafx.h"
#include "main.h"

BSTR BstrFromVariant(VARIANT *pvar, LCID lcid)
	{
	HRESULT hr = S_OK;
	VARIANT var;

	VariantInit(&var);

	/*if (V_VT(pvar) == VT_ERROR)
		{
		// This is most likely to be an Excel error code.  Search these first,
		// then try to use VBA to convert some generic HRESULT to a string.

		// MUST BE IN THE SAME ORDER AS THE ERROR STRINGS IN THE RC FILE
		// !!! MAKE THIS GLOBAL.
		static const HRESULT s_rhr[8] = {	XE_DIV0, XE_VAL, XE_NAME, XE_NULL,
											XE_CIRC, XE_NUM, XE_REF, XE_NA };
		HRESULT hrVar = V_ERROR(pvar);
		for (int i = 0; i < sizeof(s_rhr)/sizeof(HRESULT); i++)
			{
			if (hrVar == s_rhr[i])
				{
				V_BSTR(&var) = SysAllocString(lcid == lcidUSEnglish ? GetEnglishString(IDS_WEBCALC_ERRVALSTART+i) : GetIntlString(IDS_WEBCALC_ERRVALSTART + i));
				goto Done;
				}
			}
		AssertSz(fFalse,"Error type not recognized in BstrFromVariant");
		}*/

	if (V_VT(pvar) == VT_BOOL)
		{
		const OLECHAR *pszBool;
		pszBool = (V_BOOL(pvar)) ? L"True" : L"False";
		V_BSTR(&var) = SysAllocString(pszBool);
		goto Done;
		}


	// Ask OLEAUT32 to handle this.
	hr = ::VariantChangeTypeEx(&var, pvar, lcid, 0, VT_BSTR);
#ifdef DEBUG
	if (hr == hrNoError)
		{
		Assert(V_VT(&var) == VT_BSTR);
		}
#endif

Done:
	if (hr == hrNoError)
		return V_BSTR(&var);	
	else
		return NULL;
	}