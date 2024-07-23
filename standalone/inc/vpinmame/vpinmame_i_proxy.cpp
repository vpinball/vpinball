#include "stdafx.h"
#include "olectl.h"

#include "VPinMAMEController.h"
#include "VPinMAMEControllerSettings.h"
#include "VPinMAMEGame.h"
#include "VPinMAMEGames.h"
#include "VPinMAMEGameSettings.h"
#include "VPinMAMERom.h"
#include "VPinMAMERoms.h"

STDMETHODIMP VPinMAMERom::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} namesIdsList[] = {
			{ NULL },
			{ L"Audit", 4 },
			{ L"Checksum", 7 },
			{ L"ExpChecksum", 8 },
			{ L"ExpLength", 6 },
			{ L"Flags", 9 },
			{ L"Length", 5 },
			{ L"Name", 1 },
			{ L"State", 2 },
			{ L"StateDescription", 3 }
	};

	size_t min = 1, max = ARRAY_SIZE(namesIdsList) - 1, i;
	int r;
	while(min <= max) {
		i = (min + max) / 2;
		r = wcsicmp(namesIdsList[i].name, *rgszNames);
		if(!r) {
			*rgDispId = namesIdsList[i].dispId;
			return S_OK;
		}
		if(r < 0)
		   min = i+1;
		else
		   max = i-1;
	}
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP VPinMAMERom::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
	int index = pDispParams->cArgs;
	VARIANT res;
	HRESULT hres = DISP_E_UNKNOWNNAME;

	V_VT(&res) = VT_EMPTY;

	switch(dispIdMember) {
		case DISPID_VALUE: {
			if (wFlags == (DISPATCH_METHOD | DISPATCH_PROPERTYGET)) {
				V_VT(&res) = VT_DISPATCH;
				V_DISPATCH(&res) = this;
				hres = S_OK;
			}
			break;
		}
		case 1: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 21: [propget, id(1), helpstring("property Name")] HRESULT Name([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Name(&V_BSTR(&res));
			}
			break;
		}
		case 2: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 22: [propget, id(2), helpstring("property State")] HRESULT State([out, retval] long *pVal);
				V_VT(&res) = VT_I4;
				hres = get_State((LONG*)&V_I4(&res));
			}
			break;
		}
		case 3: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 23: [propget, id(3), helpstring("property StateDescription")] HRESULT StateDescription([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_StateDescription(&V_BSTR(&res));
			}
			break;
		}
		case 4: {
			if (wFlags & DISPATCH_METHOD) {
				// line 24: [id(4), helpstring("method Audit")] HRESULT Audit([in,defaultvalue(0)] VARIANT_BOOL fStrict);
				VARIANT var0;
				V_VT(&var0) = VT_BOOL;
				V_BOOL(&var0) = 0;
				VariantChangeType(&var0, (index > 0) ? &pDispParams->rgvarg[--index] : &var0, 0, VT_BOOL);
				hres = Audit(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 5: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 25: [propget, id(5), helpstring("property Length")] HRESULT Length([out, retval] long *pVal);
				V_VT(&res) = VT_I4;
				hres = get_Length((LONG*)&V_I4(&res));
			}
			break;
		}
		case 6: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 26: [propget, id(6), helpstring("property ExpLength")] HRESULT ExpLength([out, retval] long *pVal);
				V_VT(&res) = VT_I4;
				hres = get_ExpLength((LONG*)&V_I4(&res));
			}
			break;
		}
		case 7: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 27: [propget, id(7), helpstring("property Checksum")] HRESULT Checksum([out, retval] long *pVal);
				V_VT(&res) = VT_I4;
				hres = get_Checksum((LONG*)&V_I4(&res));
			}
			break;
		}
		case 8: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 28: [propget, id(8), helpstring("property ExpChecksum")] HRESULT ExpChecksum([out, retval] long *pVal);
				V_VT(&res) = VT_I4;
				hres = get_ExpChecksum((LONG*)&V_I4(&res));
			}
			break;
		}
		case 9: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 29: [propget, id(9), helpstring("property Flags")] HRESULT Flags([out, retval] long *pVal);
				V_VT(&res) = VT_I4;
				hres = get_Flags((LONG*)&V_I4(&res));
			}
			break;
		}
		default:
		break;
	}
	if (hres == S_OK) {
		if (pVarResult)
			*pVarResult = res;
		else
			VariantClear(&res);
	}
	else if (hres != S_FALSE) {
		PLOGI.printf("dispId=%d (0x%08x), wFlags=%d, hres=%d", dispIdMember, dispIdMember, wFlags, hres);
	}
	return hres;
}

STDMETHODIMP VPinMAMERoms::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} namesIdsList[] = {
			{ NULL },
			{ L"Audit", 4 },
			{ L"Available", 1 },
			{ L"Count", 8000 },
			{ L"State", 2 },
			{ L"StateDescription", 3 }
	};

	size_t min = 1, max = ARRAY_SIZE(namesIdsList) - 1, i;
	int r;
	while(min <= max) {
		i = (min + max) / 2;
		r = wcsicmp(namesIdsList[i].name, *rgszNames);
		if(!r) {
			*rgDispId = namesIdsList[i].dispId;
			return S_OK;
		}
		if(r < 0)
		   min = i+1;
		else
		   max = i-1;
	}
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP VPinMAMERoms::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
	int index = pDispParams->cArgs;
	VARIANT res;
	HRESULT hres = DISP_E_UNKNOWNNAME;

	V_VT(&res) = VT_EMPTY;

	switch(dispIdMember) {
		case DISPID_VALUE: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 43: [propget, id(DISPID_VALUE)] HRESULT Item([in] VARIANT *pKey, [out, retval] IRom* *ppGame);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				V_VT(&res) = VT_DISPATCH;
				hres = get_Item(&var0, (IRom**)&V_DISPATCH(&res));
				VariantClear(&var0);
			}
			else if (wFlags == (DISPATCH_METHOD | DISPATCH_PROPERTYGET)) {
				V_VT(&res) = VT_DISPATCH;
				V_DISPATCH(&res) = this;
				hres = S_OK;
			}
			break;
		}
		case 8000: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 42: [propget] HRESULT Count([out, retval] long* pnCount);
				V_VT(&res) = VT_I4;
				hres = get_Count((LONG*)&V_I4(&res));
			}
			break;
		}
		case DISPID_NEWENUM: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 44: [propget, id(DISPID_NEWENUM)] HRESULT _NewEnum([out, retval] IUnknown** ppunkEnum);
				V_VT(&res) = VT_UNKNOWN;
				hres = get__NewEnum(&V_UNKNOWN(&res));
			}
			break;
		}
		case 1: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 45: [propget, id(1), helpstring("property Available")] HRESULT Available([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_Available(&V_BOOL(&res));
			}
			break;
		}
		case 2: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 46: [propget, id(2), helpstring("property State")] HRESULT State([out, retval] long *pVal);
				V_VT(&res) = VT_I4;
				hres = get_State((LONG*)&V_I4(&res));
			}
			break;
		}
		case 3: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 47: [propget, id(3), helpstring("property StateDescription")] HRESULT StateDescription([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_StateDescription(&V_BSTR(&res));
			}
			break;
		}
		case 4: {
			if (wFlags & DISPATCH_METHOD) {
				// line 48: [id(4), helpstring("method Audit")] HRESULT Audit([in,defaultvalue(0)] VARIANT_BOOL fStrict);
				VARIANT var0;
				V_VT(&var0) = VT_BOOL;
				V_BOOL(&var0) = 0;
				VariantChangeType(&var0, (index > 0) ? &pDispParams->rgvarg[--index] : &var0, 0, VT_BOOL);
				hres = Audit(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		default:
		break;
	}
	if (hres == S_OK) {
		if (pVarResult)
			*pVarResult = res;
		else
			VariantClear(&res);
	}
	else if (hres != S_FALSE) {
		PLOGI.printf("dispId=%d (0x%08x), wFlags=%d, hres=%d", dispIdMember, dispIdMember, wFlags, hres);
	}
	return hres;
}

STDMETHODIMP VPinMAMEGameSettings::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} namesIdsList[] = {
			{ NULL },
			{ L"Clear", 2 },
			{ L"SetDisplayPosition", 4 },
			{ L"ShowSettingsDlg", 1 },
			{ L"Value", 3 }
	};

	size_t min = 1, max = ARRAY_SIZE(namesIdsList) - 1, i;
	int r;
	while(min <= max) {
		i = (min + max) / 2;
		r = wcsicmp(namesIdsList[i].name, *rgszNames);
		if(!r) {
			*rgDispId = namesIdsList[i].dispId;
			return S_OK;
		}
		if(r < 0)
		   min = i+1;
		else
		   max = i-1;
	}
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP VPinMAMEGameSettings::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
	int index = pDispParams->cArgs;
	VARIANT res;
	HRESULT hres = DISP_E_UNKNOWNNAME;

	V_VT(&res) = VT_EMPTY;

	switch(dispIdMember) {
		case DISPID_VALUE: {
			if (wFlags == (DISPATCH_METHOD | DISPATCH_PROPERTYGET)) {
				V_VT(&res) = VT_DISPATCH;
				V_DISPATCH(&res) = this;
				hres = S_OK;
			}
			break;
		}
		case 1: {
			if (wFlags & DISPATCH_METHOD) {
				// line 77: [id(1), helpstring("method ShowSettingsDlg")] HRESULT ShowSettingsDlg([in,defaultvalue(0)] LONG_PTR hParentWnd);
				VARIANT var0;
				V_VT(&var0) = VT_UI4;
				V_UI4(&var0) = 0;
				VariantChangeType(&var0, (index > 0) ? &pDispParams->rgvarg[--index] : &var0, 0, VT_UI4);
				hres = ShowSettingsDlg((LONG_PTR)V_UI4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 2: {
			if (wFlags & DISPATCH_METHOD) {
				// line 78: [id(2), helpstring("method Clear")] HRESULT Clear();
				hres = Clear();
			}
			break;
		}
		case 3: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 79: [propget, id(3), helpstring("property Value")] HRESULT Value([in] BSTR sName, [out, retval] VARIANT *pVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = get_Value(V_BSTR(&var0), &res);
				VariantClear(&var0);
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 80: [propput, id(3), helpstring("property Value")] HRESULT Value([in] BSTR sName, [in] VARIANT newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantCopyInd(&var1, &pDispParams->rgvarg[--index]);
				hres = put_Value(V_BSTR(&var0), var1);
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 4: {
			if (wFlags & DISPATCH_METHOD) {
				// line 81: [id(4), helpstring("method SetDisplayPosition")] HRESULT SetDisplayPosition([in] VARIANT newValX, [in] VARIANT newValY, [in,defaultvalue(0)] LONG_PTR hWnd);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantCopyInd(&var1, &pDispParams->rgvarg[--index]);
				VARIANT var2;
				V_VT(&var2) = VT_UI4;
				V_UI4(&var2) = 0;
				VariantChangeType(&var2, (index > 0) ? &pDispParams->rgvarg[--index] : &var2, 0, VT_UI4);
				hres = SetDisplayPosition(var0, var1, (LONG_PTR)V_UI4(&var2));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
			}
			break;
		}
		default:
		break;
	}
	if (hres == S_OK) {
		if (pVarResult)
			*pVarResult = res;
		else
			VariantClear(&res);
	}
	else if (hres != S_FALSE) {
		PLOGI.printf("dispId=%d (0x%08x), wFlags=%d, hres=%d", dispIdMember, dispIdMember, wFlags, hres);
	}
	return hres;
}

STDMETHODIMP VPinMAMEGame::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} namesIdsList[] = {
			{ NULL },
			{ L"CloneOf", 5 },
			{ L"Description", 2 },
			{ L"IsSupported", 9 },
			{ L"Manufacturer", 4 },
			{ L"Name", 1 },
			{ L"Roms", 6 },
			{ L"Settings", 7 },
			{ L"ShowInfoDlg", 8 },
			{ L"Year", 3 }
	};

	size_t min = 1, max = ARRAY_SIZE(namesIdsList) - 1, i;
	int r;
	while(min <= max) {
		i = (min + max) / 2;
		r = wcsicmp(namesIdsList[i].name, *rgszNames);
		if(!r) {
			*rgDispId = namesIdsList[i].dispId;
			return S_OK;
		}
		if(r < 0)
		   min = i+1;
		else
		   max = i-1;
	}
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP VPinMAMEGame::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
	int index = pDispParams->cArgs;
	VARIANT res;
	HRESULT hres = DISP_E_UNKNOWNNAME;

	V_VT(&res) = VT_EMPTY;

	switch(dispIdMember) {
		case DISPID_VALUE: {
			if (wFlags == (DISPATCH_METHOD | DISPATCH_PROPERTYGET)) {
				V_VT(&res) = VT_DISPATCH;
				V_DISPATCH(&res) = this;
				hres = S_OK;
			}
			break;
		}
		case 1: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 94: [propget, id(1), helpstring("property Name")] HRESULT Name([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Name(&V_BSTR(&res));
			}
			break;
		}
		case 2: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 95: [propget, id(2), helpstring("property Description")] HRESULT Description([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Description(&V_BSTR(&res));
			}
			break;
		}
		case 3: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 96: [propget, id(3), helpstring("property Year")] HRESULT Year([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Year(&V_BSTR(&res));
			}
			break;
		}
		case 4: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 97: [propget, id(4), helpstring("property Manufacturer")] HRESULT Manufacturer([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Manufacturer(&V_BSTR(&res));
			}
			break;
		}
		case 5: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 98: [propget, id(5), helpstring("property CloneOf")] HRESULT CloneOf([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_CloneOf(&V_BSTR(&res));
			}
			break;
		}
		case 6: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 99: [propget, id(6), helpstring("property Roms")] HRESULT Roms([out, retval] IRoms* *pVal);
				V_VT(&res) = VT_DISPATCH;
				hres = get_Roms((IRoms**)&V_DISPATCH(&res));
			}
			break;
		}
		case 7: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 100: [propget, id(7), helpstring("property Settings")] HRESULT Settings([out, retval] IGameSettings* *pVal);
				V_VT(&res) = VT_DISPATCH;
				hres = get_Settings((IGameSettings**)&V_DISPATCH(&res));
			}
			break;
		}
		case 8: {
			if (wFlags & DISPATCH_METHOD) {
				// line 101: [id(8), helpstring("method ShowInfoDlg")] HRESULT ShowInfoDlg([in,defaultvalue(0)] int nShowOptions, [in,defaultvalue(0)] LONG_PTR hParentWnd, [out, retval] int *pVal);
				VARIANT var0;
				V_VT(&var0) = VT_I4;
				V_I4(&var0) = 0;
				VariantChangeType(&var0, (index > 0) ? &pDispParams->rgvarg[--index] : &var0, 0, VT_I4);
				VARIANT var1;
				V_VT(&var1) = VT_UI4;
				V_UI4(&var1) = 0;
				VariantChangeType(&var1, (index > 0) ? &pDispParams->rgvarg[--index] : &var1, 0, VT_UI4);
				V_VT(&res) = VT_I4;
				hres = ShowInfoDlg(V_I4(&var0), (LONG_PTR)V_UI4(&var1), (int*)&V_I4(&res));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 9: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 102: [propget, id(9), helpstring("property IsSupported")] HRESULT IsSupported([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_IsSupported(&V_BOOL(&res));
			}
			break;
		}
		default:
		break;
	}
	if (hres == S_OK) {
		if (pVarResult)
			*pVarResult = res;
		else
			VariantClear(&res);
	}
	else if (hres != S_FALSE) {
		PLOGI.printf("dispId=%d (0x%08x), wFlags=%d, hres=%d", dispIdMember, dispIdMember, wFlags, hres);
	}
	return hres;
}

STDMETHODIMP VPinMAMEGames::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} namesIdsList[] = {
			{ NULL },
			{ L"Count", 8001 }
	};

	size_t min = 1, max = ARRAY_SIZE(namesIdsList) - 1, i;
	int r;
	while(min <= max) {
		i = (min + max) / 2;
		r = wcsicmp(namesIdsList[i].name, *rgszNames);
		if(!r) {
			*rgDispId = namesIdsList[i].dispId;
			return S_OK;
		}
		if(r < 0)
		   min = i+1;
		else
		   max = i-1;
	}
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP VPinMAMEGames::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
	int index = pDispParams->cArgs;
	VARIANT res;
	HRESULT hres = DISP_E_UNKNOWNNAME;

	V_VT(&res) = VT_EMPTY;

	switch(dispIdMember) {
		case DISPID_VALUE: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 116: [propget, id(DISPID_VALUE)] HRESULT Item([in] VARIANT *pKey, [out, retval] IGame* *ppGame);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				V_VT(&res) = VT_DISPATCH;
				hres = get_Item(&var0, (IGame**)&V_DISPATCH(&res));
				VariantClear(&var0);
			}
			else if (wFlags == (DISPATCH_METHOD | DISPATCH_PROPERTYGET)) {
				V_VT(&res) = VT_DISPATCH;
				V_DISPATCH(&res) = this;
				hres = S_OK;
			}
			break;
		}
		case 8001: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 115: [propget] HRESULT Count([out, retval] long* pnCount);
				V_VT(&res) = VT_I4;
				hres = get_Count((LONG*)&V_I4(&res));
			}
			break;
		}
		case DISPID_NEWENUM: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 117: [propget, id(DISPID_NEWENUM)] HRESULT _NewEnum([out, retval] IUnknown** ppunkEnum);
				V_VT(&res) = VT_UNKNOWN;
				hres = get__NewEnum(&V_UNKNOWN(&res));
			}
			break;
		}
		default:
		break;
	}
	if (hres == S_OK) {
		if (pVarResult)
			*pVarResult = res;
		else
			VariantClear(&res);
	}
	else if (hres != S_FALSE) {
		PLOGI.printf("dispId=%d (0x%08x), wFlags=%d, hres=%d", dispIdMember, dispIdMember, wFlags, hres);
	}
	return hres;
}

STDMETHODIMP VPinMAMEControllerSettings::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} namesIdsList[] = {
			{ NULL },
			{ L"Clear", 2 },
			{ L"InstallDir", 4 },
			{ L"ShowSettingsDlg", 1 },
			{ L"Value", 3 }
	};

	size_t min = 1, max = ARRAY_SIZE(namesIdsList) - 1, i;
	int r;
	while(min <= max) {
		i = (min + max) / 2;
		r = wcsicmp(namesIdsList[i].name, *rgszNames);
		if(!r) {
			*rgDispId = namesIdsList[i].dispId;
			return S_OK;
		}
		if(r < 0)
		   min = i+1;
		else
		   max = i-1;
	}
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP VPinMAMEControllerSettings::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
	int index = pDispParams->cArgs;
	VARIANT res;
	HRESULT hres = DISP_E_UNKNOWNNAME;

	V_VT(&res) = VT_EMPTY;

	switch(dispIdMember) {
		case DISPID_VALUE: {
			if (wFlags == (DISPATCH_METHOD | DISPATCH_PROPERTYGET)) {
				V_VT(&res) = VT_DISPATCH;
				V_DISPATCH(&res) = this;
				hres = S_OK;
			}
			break;
		}
		case 1: {
			if (wFlags & DISPATCH_METHOD) {
				// line 145: [id(1), helpstring("method ShowSettingsDlg")] HRESULT ShowSettingsDlg(LONG_PTR hParentWnd);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_UI4);
				hres = ShowSettingsDlg((LONG_PTR)V_UI4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 2: {
			if (wFlags & DISPATCH_METHOD) {
				// line 146: [id(2), helpstring("method Clear")] HRESULT Clear();
				hres = Clear();
			}
			break;
		}
		case 3: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 147: [propget, id(3), helpstring("property Value")] HRESULT Value([in] BSTR sName, [out, retval] VARIANT *pVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = get_Value(V_BSTR(&var0), &res);
				VariantClear(&var0);
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 148: [propput, id(3), helpstring("property Value")] HRESULT Value([in] BSTR sName, [in] VARIANT newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantCopyInd(&var1, &pDispParams->rgvarg[--index]);
				hres = put_Value(V_BSTR(&var0), var1);
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 4: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 149: [propget, id(4), helpstring("property InstallDir")] HRESULT InstallDir([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_InstallDir(&V_BSTR(&res));
			}
			break;
		}
		default:
		break;
	}
	if (hres == S_OK) {
		if (pVarResult)
			*pVarResult = res;
		else
			VariantClear(&res);
	}
	else if (hres != S_FALSE) {
		PLOGI.printf("dispId=%d (0x%08x), wFlags=%d, hres=%d", dispIdMember, dispIdMember, wFlags, hres);
	}
	return hres;
}

STDMETHODIMP VPinMAMEController::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} namesIdsList[] = {
			{ NULL },
			{ L"AudioDeviceDescription", 71 },
			{ L"AudioDeviceModule", 72 },
			{ L"AudioDevicesCount", 70 },
			{ L"CabinetMode", 76 },
			{ L"ChangedGIsState", 67 },
			{ L"ChangedGIStrings", 28 },
			{ L"ChangedLamps", 14 },
			{ L"ChangedLampsState", 63 },
			{ L"ChangedLEDs", 53 },
			{ L"ChangedLEDsState", 82 },
			{ L"ChangedNVRAM", 85 },
			{ L"ChangedSolenoids", 16 },
			{ L"ChangedSolenoidsState", 65 },
			{ L"CheckROMS", 25 },
			{ L"CurrentAudioDevice", 73 },
			{ L"Dip", 32 },
			{ L"DmdHeight", 60 },
			{ L"DmdPixel", 61 },
			{ L"DmdWidth", 59 },
			{ L"DoubleSize", 41 },
			{ L"EnumAudioDevices", 69 },
			{ L"FastFrames", 74 },
			{ L"Game", 55 },
			{ L"GameName", 17 },
			{ L"Games", 51 },
			{ L"GetClientRect", 58 },
			{ L"GetMech", 34 },
			{ L"GetWindowRect", 57 },
			{ L"GIString", 27 },
			{ L"GIStrings", 33 },
			{ L"HandleKeyboard", 19 },
			{ L"HandleMechanics", 26 },
			{ L"Hidden", 54 },
			{ L"IgnoreRomCrc", 75 },
			{ L"Lamp", 5 },
			{ L"Lamps", 13 },
			{ L"LampsState", 64 },
			{ L"LockDisplay", 43 },
			{ L"Machines", 23 },
			{ L"MasterVolume", 68 },
			{ L"Mech", 49 },
			{ L"MechSamples", 56 },
			{ L"ModOutputType", 88 },
			{ L"NewSoundCommands", 50 },
			{ L"NVRAM", 84 },
			{ L"Pause", 22 },
			{ L"RawDmdColoredPixels", 83 },
			{ L"RawDmdHeight", 79 },
			{ L"RawDmdPixels", 80 },
			{ L"RawDmdWidth", 78 },
			{ L"ROMName", 87 },
			{ L"Run", 1 },
			{ L"Running", 24 },
			{ L"SetDisplayPosition", 42 },
			{ L"Settings", 52 },
			{ L"ShowAboutDialog", 4 },
			{ L"ShowDMDOnly", 29 },
			{ L"ShowFrame", 35 },
			{ L"ShowOptsDialog", 3 },
			{ L"ShowPathesDialog", 46 },
			{ L"ShowPinDMD", 77 },
			{ L"ShowTitle", 21 },
			{ L"ShowWinDMD", 81 },
			{ L"Solenoid", 6 },
			{ L"Solenoids", 31 },
			{ L"SolenoidsState", 66 },
			{ L"SolMask", 45 },
			{ L"SoundMode", 86 },
			{ L"SplashInfoLine", 30 },
			{ L"Stop", 2 },
			{ L"Switch", 7 },
			{ L"Switches", 15 },
			{ L"TimeFence", 89 },
			{ L"updateDmdPixels", 62 },
			{ L"Version", 48 },
			{ L"WPCNumbering", 12 }
	};

	size_t min = 1, max = ARRAY_SIZE(namesIdsList) - 1, i;
	int r;
	while(min <= max) {
		i = (min + max) / 2;
		r = wcsicmp(namesIdsList[i].name, *rgszNames);
		if(!r) {
			*rgDispId = namesIdsList[i].dispId;
			return S_OK;
		}
		if(r < 0)
		   min = i+1;
		else
		   max = i-1;
	}
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP VPinMAMEController::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
	int index = pDispParams->cArgs;
	VARIANT res;
	HRESULT hres = DISP_E_UNKNOWNNAME;

	V_VT(&res) = VT_EMPTY;

	switch(dispIdMember) {
		case DISPID_VALUE: {
			if (wFlags == (DISPATCH_METHOD | DISPATCH_PROPERTYGET)) {
				V_VT(&res) = VT_DISPATCH;
				V_DISPATCH(&res) = this;
				hres = S_OK;
			}
			break;
		}
		case 1: {
			if (wFlags & DISPATCH_METHOD) {
				// line 161: [id(1), helpstring("method Run")] HRESULT Run([in,defaultvalue(0)] LONG_PTR hParentWnd, [in,defaultvalue(99)] int nMinVersion);
				VARIANT var0;
				V_VT(&var0) = VT_UI4;
				V_UI4(&var0) = 0;
				VariantChangeType(&var0, (index > 0) ? &pDispParams->rgvarg[--index] : &var0, 0, VT_UI4);
				VARIANT var1;
				V_VT(&var1) = VT_I4;
				V_I4(&var1) = 99;
				VariantChangeType(&var1, (index > 0) ? &pDispParams->rgvarg[--index] : &var1, 0, VT_I4);
				hres = Run((LONG_PTR)V_UI4(&var0), V_I4(&var1));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 2: {
			if (wFlags & DISPATCH_METHOD) {
				// line 162: [id(2), helpstring("method Stop")] HRESULT Stop();
				hres = Stop();
			}
			break;
		}
		case 3: {
			if (wFlags & DISPATCH_METHOD) {
				// line 163: /* ! */	[id(3), helpstring("method ShowOptsDialog")] HRESULT ShowOptsDialog([in,defaultvalue(0)] LONG_PTR hParentWnd);
				VARIANT var0;
				V_VT(&var0) = VT_UI4;
				V_UI4(&var0) = 0;
				VariantChangeType(&var0, (index > 0) ? &pDispParams->rgvarg[--index] : &var0, 0, VT_UI4);
				hres = ShowOptsDialog((LONG_PTR)V_UI4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 4: {
			if (wFlags & DISPATCH_METHOD) {
				// line 164: /* ! */	[id(4), helpstring("method ShowAboutDialog")] HRESULT ShowAboutDialog([in,defaultvalue(0)] LONG_PTR hParentWnd);
				VARIANT var0;
				V_VT(&var0) = VT_UI4;
				V_UI4(&var0) = 0;
				VariantChangeType(&var0, (index > 0) ? &pDispParams->rgvarg[--index] : &var0, 0, VT_UI4);
				hres = ShowAboutDialog((LONG_PTR)V_UI4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 5: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 165: [propget, id(5), helpstring("property Lamp")] HRESULT Lamp([in] int nLamp, [out, retval] VARIANT_BOOL *pVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				V_VT(&res) = VT_BOOL;
				hres = get_Lamp(V_I4(&var0), &V_BOOL(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 6: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 166: [propget, id(6), helpstring("property Solenoid")] HRESULT Solenoid([in] int nSolenoid, [out, retval] VARIANT_BOOL *pVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				V_VT(&res) = VT_BOOL;
				hres = get_Solenoid(V_I4(&var0), &V_BOOL(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 7: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 167: [propget, id(7), helpstring("property Switch")] HRESULT Switch([in] int nSwitchNo, [out, retval] VARIANT_BOOL *pVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				V_VT(&res) = VT_BOOL;
				hres = get_Switch(V_I4(&var0), &V_BOOL(&res));
				VariantClear(&var0);
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 168: [propput, id(7), helpstring("property Switch")] HRESULT Switch([in] int nSwitchNo, [in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_Switch(V_I4(&var0), V_BOOL(&var1));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 12: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 169: [propget, id(12), helpstring("property WPCNumbering")] HRESULT WPCNumbering([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_WPCNumbering(&V_BOOL(&res));
			}
			break;
		}
		case 13: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 170: [propget, id(13), helpstring("property Lamps")] HRESULT Lamps([out, retval] VARIANT *pVal);
				hres = get_Lamps(&res);
			}
			break;
		}
		case 14: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 171: [propget, id(14), helpstring("property ChangedLamps")] HRESULT ChangedLamps([out, retval] VARIANT *pVal);
				hres = get_ChangedLamps(&res);
			}
			break;
		}
		case 15: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 172: [propget, id(15), helpstring("property Switches")] HRESULT Switches([out, retval] VARIANT *pVal);
				hres = get_Switches(&res);
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 173: [propput, id(15), helpstring("property Switches")] HRESULT Switches([in] VARIANT newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = put_Switches(var0);
				VariantClear(&var0);
			}
			break;
		}
		case 16: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 174: [propget, id(16), helpstring("property ChangedSolenoids")] HRESULT ChangedSolenoids([out, retval] VARIANT *pVal);
				hres = get_ChangedSolenoids(&res);
			}
			break;
		}
		case 17: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 175: [propget, id(17), helpstring("property GameName")] HRESULT GameName([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_GameName(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 176: [propput, id(17), helpstring("property GameName")] HRESULT GameName([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_GameName(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 19: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 177: [propget, id(19), helpstring("property HandleKeyboard")] HRESULT HandleKeyboard([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_HandleKeyboard(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 178: [propput, id(19), helpstring("property HandleKeyboard")] HRESULT HandleKeyboard([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_HandleKeyboard(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 21: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 179: /* ! */	[propget, id(21), helpstring("property ShowTitle")] HRESULT ShowTitle([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_ShowTitle(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 180: /* ! */	[propput, id(21), helpstring("property ShowTitle")] HRESULT ShowTitle([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_ShowTitle(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 22: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 181: [propget, id(22), helpstring("property Pause")] HRESULT Pause([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_Pause(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 182: [propput, id(22), helpstring("property Pause")] HRESULT Pause([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_Pause(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 23: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 183: [propget, id(23), helpstring("property Machines")] HRESULT Machines([in,defaultvalue("")] BSTR sMachine, [out, retval] VARIANT *pVal);
				VARIANT var0;
				OLECHAR* pszDefault = (OLECHAR*)L"";
				V_VT(&var0) = VT_BSTR;
				V_BSTR(&var0) = SysAllocString(pszDefault);
				VariantChangeType(&var0, (index > 0) ? &pDispParams->rgvarg[--index] : &var0, 0, VT_BSTR);
				hres = get_Machines(V_BSTR(&var0), &res);
				VariantClear(&var0);
			}
			break;
		}
		case 24: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 184: [propget, id(24), helpstring("property Running")] HRESULT Running([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_Running(&V_BOOL(&res));
			}
			break;
		}
		case 25: {
			if (wFlags & DISPATCH_METHOD) {
				// line 185: /* ! */	[id(25), helpstring("method CheckROMS")] HRESULT CheckROMS([in,defaultvalue(0)] int nShowOptions, [in,defaultvalue(0)] LONG_PTR hParentWnd, [out, retval] VARIANT_BOOL *pVal);
				VARIANT var0;
				V_VT(&var0) = VT_I4;
				V_I4(&var0) = 0;
				VariantChangeType(&var0, (index > 0) ? &pDispParams->rgvarg[--index] : &var0, 0, VT_I4);
				VARIANT var1;
				V_VT(&var1) = VT_UI4;
				V_UI4(&var1) = 0;
				VariantChangeType(&var1, (index > 0) ? &pDispParams->rgvarg[--index] : &var1, 0, VT_UI4);
				V_VT(&res) = VT_BOOL;
				hres = CheckROMS(V_I4(&var0), (LONG_PTR)V_UI4(&var1), &V_BOOL(&res));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 26: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 186: [propget, id(26), helpstring("property HandleMechanics")] HRESULT HandleMechanics([out, retval] int *pVal);
				V_VT(&res) = VT_I4;
				hres = get_HandleMechanics((int*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 187: [propput, id(26), helpstring("property HandleMechanics")] HRESULT HandleMechanics([in] int newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_HandleMechanics(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 27: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 188: [propget, id(27), helpstring("property GIString")] HRESULT GIString(int nString, [out, retval] int *pVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				V_VT(&res) = VT_I4;
				hres = get_GIString(V_I4(&var0), (int*)&V_I4(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 28: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 189: [propget, id(28), helpstring("property ChangedGIStrings")] HRESULT ChangedGIStrings([out, retval] VARIANT *pVal);
				hres = get_ChangedGIStrings(&res);
			}
			break;
		}
		case 29: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 190: /* ! */	[propget, id(29), helpstring("property ShowDMDOnly")] HRESULT ShowDMDOnly([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_ShowDMDOnly(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 191: /* ! */	[propput, id(29), helpstring("property ShowDMDOnly")] HRESULT ShowDMDOnly([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_ShowDMDOnly(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 30: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 192: [propget, id(30), helpstring("property SplashInfoLine")] HRESULT SplashInfoLine([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_SplashInfoLine(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 193: [propput, id(30), helpstring("property SplashInfoLine")] HRESULT SplashInfoLine([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_SplashInfoLine(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 31: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 194: [propget, id(31), helpstring("property Solenoids")] HRESULT Solenoids([out, retval] VARIANT *pVal);
				hres = get_Solenoids(&res);
			}
			break;
		}
		case 32: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 195: [propget, id(32), helpstring("property Dip")] HRESULT Dip([in] int nNo, [out, retval] int *pVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				V_VT(&res) = VT_I4;
				hres = get_Dip(V_I4(&var0), (int*)&V_I4(&res));
				VariantClear(&var0);
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 196: [propput, id(32), helpstring("property Dip")] HRESULT Dip([in] int nNo, [in] int newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_Dip(V_I4(&var0), V_I4(&var1));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 33: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 197: [propget, id(33), helpstring("property GIStrings")] HRESULT GIStrings([out, retval] VARIANT *pVal);
				hres = get_GIStrings(&res);
			}
			break;
		}
		case 34: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 198: [propget, id(34), helpstring("property GetMech")] HRESULT GetMech([in] int mechNo, [out, retval] int *pVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				V_VT(&res) = VT_I4;
				hres = get_GetMech(V_I4(&var0), (int*)&V_I4(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 35: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 199: /* ! */	[propget, id(35), helpstring("property ShowFrame")] HRESULT ShowFrame([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_ShowFrame(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 200: /* ! */	[propput, id(35), helpstring("property ShowFrame")] HRESULT ShowFrame([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_ShowFrame(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 41: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 201: /* ! */	[propget, id(41), helpstring("property DoubleSize")] HRESULT DoubleSize([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_DoubleSize(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 202: /* ! */	[propput, id(41), helpstring("property DoubleSize")] HRESULT DoubleSize([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_DoubleSize(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 42: {
			if (wFlags & DISPATCH_METHOD) {
				// line 203: /* ! */	[id(42), helpstring("method SetDisplayPosition")] HRESULT SetDisplayPosition([in] int x, [in] int y, [in,defaultvalue(0)] LONG_PTR hParentWindow);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var2;
				V_VT(&var2) = VT_UI4;
				V_UI4(&var2) = 0;
				VariantChangeType(&var2, (index > 0) ? &pDispParams->rgvarg[--index] : &var2, 0, VT_UI4);
				hres = SetDisplayPosition(V_I4(&var0), V_I4(&var1), (LONG_PTR)V_UI4(&var2));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
			}
			break;
		}
		case 43: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 204: [propget, id(43), helpstring("property LockDisplay")] HRESULT LockDisplay([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_LockDisplay(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 205: [propput, id(43), helpstring("property LockDisplay")] HRESULT LockDisplay([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_LockDisplay(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 45: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 206: [propget, id(45), helpstring("property SolMask")] HRESULT SolMask([in] int nLow, [out, retval] long *pVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				V_VT(&res) = VT_I4;
				hres = get_SolMask(V_I4(&var0), (LONG*)&V_I4(&res));
				VariantClear(&var0);
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 207: [propput, id(45), helpstring("property SolMask")] HRESULT SolMask([in] int nLow, [in] long newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_SolMask(V_I4(&var0), V_I4(&var1));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 46: {
			if (wFlags & DISPATCH_METHOD) {
				// line 208: /* ! */	[id(46), helpstring("method ShowPathesDialog")] HRESULT ShowPathesDialog([in,defaultvalue(0)] LONG_PTR hParentWnd);
				VARIANT var0;
				V_VT(&var0) = VT_UI4;
				V_UI4(&var0) = 0;
				VariantChangeType(&var0, (index > 0) ? &pDispParams->rgvarg[--index] : &var0, 0, VT_UI4);
				hres = ShowPathesDialog((LONG_PTR)V_UI4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 48: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 209: [propget, id(48), helpstring("property Version")] HRESULT Version([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Version(&V_BSTR(&res));
			}
			break;
		}
		case 49: {
			if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 210: [propput, id(49), helpstring("property Mech")] HRESULT Mech([in] int param, [in] int newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_Mech(V_I4(&var0), V_I4(&var1));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 50: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 211: /* ! */	[propget, id(50), helpstring("property NewSoundCommands")] HRESULT NewSoundCommands([out, retval] VARIANT *pVal);
				hres = get_NewSoundCommands(&res);
			}
			break;
		}
		case 51: {
			if (wFlags & DISPATCH_METHOD) {
				hres = m_pGames->Invoke(DISPID_VALUE, IID_IGames, lcid, DISPATCH_PROPERTYGET, pDispParams, &res, pExcepInfo, puArgErr);
			}
			else if (wFlags & DISPATCH_PROPERTYGET) {
				// line 212: [propget, id(51), helpstring("property Games")] HRESULT Games([out, retval] IGames* *pVal);
				V_VT(&res) = VT_DISPATCH;
				hres = get_Games((IGames**)&V_DISPATCH(&res));
			}
			break;
		}
		case 52: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 213: [propget, id(52), helpstring("property Settings")] HRESULT Settings([out, retval] IControllerSettings * *pVal);
				V_VT(&res) = VT_DISPATCH;
				hres = get_Settings((IControllerSettings**)&V_DISPATCH(&res));
			}
			break;
		}
		case 53: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 214: [propget, id(53), helpstring("property ChangedLEDs")] HRESULT ChangedLEDs([in] int nHigh, [in] int nLow, [in,defaultvalue(0)] int nnHigh, [in,defaultvalue(0)] int nnLow, [out, retval] VARIANT *pVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var2;
				V_VT(&var2) = VT_I4;
				V_I4(&var2) = 0;
				VariantChangeType(&var2, (index > 0) ? &pDispParams->rgvarg[--index] : &var2, 0, VT_I4);
				VARIANT var3;
				V_VT(&var3) = VT_I4;
				V_I4(&var3) = 0;
				VariantChangeType(&var3, (index > 0) ? &pDispParams->rgvarg[--index] : &var3, 0, VT_I4);
				hres = get_ChangedLEDs(V_I4(&var0), V_I4(&var1), V_I4(&var2), V_I4(&var3), &res);
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
				VariantClear(&var3);
			}
			break;
		}
		case 54: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 215: [propget, id(54), helpstring("property Hidden")] HRESULT Hidden([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_Hidden(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 216: [propput, id(54), helpstring("property Hidden")] HRESULT Hidden([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_Hidden(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 55: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 217: [propget, id(55), helpstring("property Game")] HRESULT Game([out, retval] IGame * *pVal);
				V_VT(&res) = VT_DISPATCH;
				hres = get_Game((IGame**)&V_DISPATCH(&res));
			}
			break;
		}
		case 56: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 218: [propget, id(56), helpstring("property MechSamples")] HRESULT MechSamples([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_MechSamples(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 219: [propput, id(56), helpstring("property MechSamples")] HRESULT MechSamples([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_MechSamples(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 57: {
			if (wFlags & DISPATCH_METHOD) {
				// line 220: [id(57), helpstring("method GetWindowRect")] HRESULT GetWindowRect([in,defaultvalue(0)] LONG_PTR hWnd, [out, retval] VARIANT *pVal);
				VARIANT var0;
				V_VT(&var0) = VT_UI4;
				V_UI4(&var0) = 0;
				VariantChangeType(&var0, (index > 0) ? &pDispParams->rgvarg[--index] : &var0, 0, VT_UI4);
				hres = GetWindowRect((LONG_PTR)V_UI4(&var0), &res);
				VariantClear(&var0);
			}
			break;
		}
		case 58: {
			if (wFlags & DISPATCH_METHOD) {
				// line 221: [id(58), helpstring("method GetClientRect")] HRESULT GetClientRect([in,defaultvalue(0)] LONG_PTR hWnd, [out, retval] VARIANT *pVal);
				VARIANT var0;
				V_VT(&var0) = VT_UI4;
				V_UI4(&var0) = 0;
				VariantChangeType(&var0, (index > 0) ? &pDispParams->rgvarg[--index] : &var0, 0, VT_UI4);
				hres = GetClientRect((LONG_PTR)V_UI4(&var0), &res);
				VariantClear(&var0);
			}
			break;
		}
		case 78: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 222: [propget, id(78), helpstring("property RawDmdWidth")] HRESULT RawDmdWidth([out, retval] int *pVal);
				V_VT(&res) = VT_I4;
				hres = get_RawDmdWidth((int*)&V_I4(&res));
			}
			break;
		}
		case 79: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 223: [propget, id(79), helpstring("property RawDmdHeight")] HRESULT RawDmdHeight([out, retval] int *pVal);
				V_VT(&res) = VT_I4;
				hres = get_RawDmdHeight((int*)&V_I4(&res));
			}
			break;
		}
		case 80: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 224: [propget, id(80), helpstring("property RawDmdPixels")] HRESULT RawDmdPixels([out, retval] VARIANT *pVal);
				hres = get_RawDmdPixels(&res);
			}
			break;
		}
		case 59: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 225: [propget, id(59), helpstring("property DmdWidth")] HRESULT DmdWidth([out, retval] int *pVal);
				V_VT(&res) = VT_I4;
				hres = get_DmdWidth((int*)&V_I4(&res));
			}
			break;
		}
		case 60: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 226: [propget, id(60), helpstring("property DmdHeight")] HRESULT DmdHeight([out, retval] int *pVal);
				V_VT(&res) = VT_I4;
				hres = get_DmdHeight((int*)&V_I4(&res));
			}
			break;
		}
		case 61: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 227: [propget, id(61), helpstring("property DmdPixel")] HRESULT DmdPixel([in] int x, [in] int y, [out, retval] int *pVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_I4);
				V_VT(&res) = VT_I4;
				hres = get_DmdPixel(V_I4(&var0), V_I4(&var1), (int*)&V_I4(&res));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 62: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 228: [propget, id(62), helpstring("property updateDmdPixels")] HRESULT updateDmdPixels([in] int **buf, [in] int width, [in] int height, [out, retval] int *pVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_INT_PTR);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var2;
				V_VT(&var2) = VT_EMPTY;
				VariantChangeType(&var2, &pDispParams->rgvarg[--index], 0, VT_I4);
				V_VT(&res) = VT_I4;
				hres = get_updateDmdPixels((int**)V_INT_PTR(&var0), V_I4(&var1), V_I4(&var2), (int*)&V_I4(&res));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
			}
			break;
		}
		case 63: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 229: [propget, id(63), helpstring("property ChangedLampsState")] HRESULT ChangedLampsState([in] int **buf, [out, retval] int *pVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_INT_PTR);
				V_VT(&res) = VT_I4;
				hres = get_ChangedLampsState((int**)V_INT_PTR(&var0), (int*)&V_I4(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 64: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 230: [propget, id(64), helpstring("property LampsState")] HRESULT LampsState([in] int **buf, [out, retval] int *pVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_INT_PTR);
				V_VT(&res) = VT_I4;
				hres = get_LampsState((int**)V_INT_PTR(&var0), (int*)&V_I4(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 65: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 231: [propget, id(65), helpstring("property ChangedSolenoidsState")] HRESULT ChangedSolenoidsState([in] int **buf, [out, retval] int *pVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_INT_PTR);
				V_VT(&res) = VT_I4;
				hres = get_ChangedSolenoidsState((int**)V_INT_PTR(&var0), (int*)&V_I4(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 66: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 232: [propget, id(66), helpstring("property SolenoidsState")] HRESULT SolenoidsState([in] int **buf, [out, retval] int *pVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_INT_PTR);
				V_VT(&res) = VT_I4;
				hres = get_SolenoidsState((int**)V_INT_PTR(&var0), (int*)&V_I4(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 67: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 233: [propget, id(67), helpstring("property ChangedGIsState")] HRESULT ChangedGIsState([in] int **buf, [out, retval] int *pVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_INT_PTR);
				V_VT(&res) = VT_I4;
				hres = get_ChangedGIsState((int**)V_INT_PTR(&var0), (int*)&V_I4(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 68: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 234: [propget, id(68), helpstring("property MasterVolume")] HRESULT MasterVolume([out, retval] int *pVal);
				V_VT(&res) = VT_I4;
				hres = get_MasterVolume((int*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 235: [propput, id(68), helpstring("property MasterVolume")] HRESULT MasterVolume([in] int newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_MasterVolume(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 69: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 236: [propget, id(69), helpstring("property EnumAudioDevices")] HRESULT EnumAudioDevices([out, retval] int *pVal);
				V_VT(&res) = VT_I4;
				hres = get_EnumAudioDevices((int*)&V_I4(&res));
			}
			break;
		}
		case 70: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 237: [propget, id(70), helpstring("property AudioDevicesCount")] HRESULT AudioDevicesCount([out, retval] int *pVal);
				V_VT(&res) = VT_I4;
				hres = get_AudioDevicesCount((int*)&V_I4(&res));
			}
			break;
		}
		case 71: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 238: [propget, id(71), helpstring("property AudioDeviceDescription")] HRESULT AudioDeviceDescription([in] int num,[out, retval] BSTR *pVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				V_VT(&res) = VT_BSTR;
				hres = get_AudioDeviceDescription(V_I4(&var0), &V_BSTR(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 72: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 239: [propget, id(72), helpstring("property AudioDeviceModule")] HRESULT AudioDeviceModule([in] int num,[out, retval] BSTR *pVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				V_VT(&res) = VT_BSTR;
				hres = get_AudioDeviceModule(V_I4(&var0), &V_BSTR(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 73: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 240: [propget, id(73), helpstring("property CurrentAudioDevice")] HRESULT CurrentAudioDevice([out, retval] int *pVal);
				V_VT(&res) = VT_I4;
				hres = get_CurrentAudioDevice((int*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 241: [propput, id(73), helpstring("property CurrentAudioDevice")] HRESULT CurrentAudioDevice([in] int newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_CurrentAudioDevice(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 74: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 242: [propget, id(74), helpstring("property FastFrames")] HRESULT FastFrames([out, retval] int *pVal);
				V_VT(&res) = VT_I4;
				hres = get_FastFrames((int*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 243: [propput, id(74), helpstring("property FastFrames")] HRESULT FastFrames([in] int newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_FastFrames(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 75: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 244: [propget, id(75), helpstring("property IgnoreRomCrc")] HRESULT IgnoreRomCrc([out, retval] VARIANT_BOOL  *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_IgnoreRomCrc(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 245: [propput, id(75), helpstring("property IgnoreRomCrc")] HRESULT IgnoreRomCrc([in] VARIANT_BOOL  newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_IgnoreRomCrc(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 76: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 246: [propget, id(76), helpstring("property CabinetMode")] HRESULT CabinetMode([out, retval] VARIANT_BOOL  *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_CabinetMode(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 247: [propput, id(76), helpstring("property CabinetMode")] HRESULT CabinetMode([in] VARIANT_BOOL  newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_CabinetMode(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 77: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 248: [propget, id(77), helpstring("property ShowPinDMD")] HRESULT ShowPinDMD([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_ShowPinDMD(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 249: [propput, id(77), helpstring("property ShowPinDMD")] HRESULT ShowPinDMD([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_ShowPinDMD(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 81: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 250: [propget, id(81), helpstring("property ShowWinDMD")] HRESULT ShowWinDMD([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_ShowWinDMD(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 251: [propput, id(81), helpstring("property ShowWinDMD")] HRESULT ShowWinDMD([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_ShowWinDMD(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 82: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 252: [propget, id(82), helpstring("property ChangedLEDsState")] HRESULT ChangedLEDsState([in] int nHigh, [in] int nLow, [in] int nnHigh, [in] int nnLow, [in] int **buf, [out, retval] int *pVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var2;
				V_VT(&var2) = VT_EMPTY;
				VariantChangeType(&var2, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var3;
				V_VT(&var3) = VT_EMPTY;
				VariantChangeType(&var3, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var4;
				V_VT(&var4) = VT_EMPTY;
				VariantChangeType(&var4, &pDispParams->rgvarg[--index], 0, VT_INT_PTR);
				V_VT(&res) = VT_I4;
				hres = get_ChangedLEDsState(V_I4(&var0), V_I4(&var1), V_I4(&var2), V_I4(&var3), (int**)V_INT_PTR(&var4), (int*)&V_I4(&res));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
				VariantClear(&var3);
				VariantClear(&var4);
			}
			break;
		}
		case 83: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 253: [propget, id(83), helpstring("property RawDmdColoredPixels")] HRESULT RawDmdColoredPixels([out, retval] VARIANT *pVal);
				hres = get_RawDmdColoredPixels(&res);
			}
			break;
		}
		case 84: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 254: [propget, id(84), helpstring("property NVRAM")] HRESULT NVRAM([out, retval] VARIANT *pVal);
				hres = get_NVRAM(&res);
			}
			break;
		}
		case 85: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 255: [propget, id(85), helpstring("property ChangedNVRAM")] HRESULT ChangedNVRAM([out, retval] VARIANT *pVal);
				hres = get_ChangedNVRAM(&res);
			}
			break;
		}
		case 86: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 256: [propget, id(86), helpstring("property SoundMode")] HRESULT SoundMode([out, retval] int *pVal);
				V_VT(&res) = VT_I4;
				hres = get_SoundMode((int*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 257: [propput, id(86), helpstring("property SoundMode")] HRESULT SoundMode([in] int newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_SoundMode(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 87: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 258: [propget, id(87), helpstring("property ROMName")] HRESULT ROMName([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_ROMName(&V_BSTR(&res));
			}
			break;
		}
		case 88: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 259: [propget, id(88), helpstring("property ModOutputType")] HRESULT ModOutputType([in] int output, [in] int no, [out, retval] int *pVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_I4);
				V_VT(&res) = VT_I4;
				hres = get_ModOutputType(V_I4(&var0), V_I4(&var1), (int*)&V_I4(&res));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 260: [propput, id(88), helpstring("property ModOutputType")] HRESULT ModOutputType([in] int output, [in] int no, [in] int newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var2;
				V_VT(&var2) = VT_EMPTY;
				VariantChangeType(&var2, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_ModOutputType(V_I4(&var0), V_I4(&var1), V_I4(&var2));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
			}
			break;
		}
		case 89: {
			if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 261: [propput, id(89), helpstring("property TimeFence")] HRESULT TimeFence([in] double timeInS);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R8);
				hres = put_TimeFence(V_R8(&var0));
				VariantClear(&var0);
			}
			break;
		}
		default:
		break;
	}
	if (hres == S_OK) {
		if (pVarResult)
			*pVarResult = res;
		else
			VariantClear(&res);
	}
	else if (hres != S_FALSE) {
		PLOGI.printf("dispId=%d (0x%08x), wFlags=%d, hres=%d", dispIdMember, dispIdMember, wFlags, hres);
	}
	return hres;
}

