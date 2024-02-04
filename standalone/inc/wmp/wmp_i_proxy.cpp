#include "stdafx.h"
#include "olectl.h"

#include "WMPControls.h"
#include "WMPCore.h"
#include "WMPSettings.h"

STDMETHODIMP WMPControls::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} namesIdsList[] = {
			{ NULL },
			{ L"currentItem", 0x003c },
			{ L"currentMarker", 0x003d },
			{ L"currentPosition", 0x0038 },
			{ L"currentPositionString", 0x0039 },
			{ L"fastForward", 0x0036 },
			{ L"fastReverse", 0x0037 },
			{ L"isAvailable", 0x003e },
			{ L"next", 0x003a },
			{ L"pause", 0x0035 },
			{ L"play", 0x0033 },
			{ L"playItem", 0x003f },
			{ L"previous", 0x003b },
			{ L"stop", 0x0034 }
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

STDMETHODIMP WMPControls::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
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
		case 0x003e: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 313: [id(0x003e), propget]HRESULT isAvailable([in] BSTR bstrItem,[out, retval] VARIANT_BOOL *pIsAvailable);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				V_VT(&res) = VT_BOOL;
				hres = get_isAvailable(V_BSTR(&var0), &V_BOOL(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 0x0033: {
			if (wFlags & DISPATCH_METHOD) {
				// line 318: [id(0x0033)]HRESULT play();
				hres = play();
			}
			break;
		}
		case 0x0034: {
			if (wFlags & DISPATCH_METHOD) {
				// line 321: [id(0x0034)]HRESULT stop();
				hres = stop();
			}
			break;
		}
		case 0x0035: {
			if (wFlags & DISPATCH_METHOD) {
				// line 324: [id(0x0035)]HRESULT pause();
				hres = pause();
			}
			break;
		}
		case 0x0036: {
			if (wFlags & DISPATCH_METHOD) {
				// line 327: [id(0x0036)]HRESULT fastForward();
				hres = fastForward();
			}
			break;
		}
		case 0x0037: {
			if (wFlags & DISPATCH_METHOD) {
				// line 330: [id(0x0037)]HRESULT fastReverse();
				hres = fastReverse();
			}
			break;
		}
		case 0x0038: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 333: [id(0x0038), propget]HRESULT currentPosition([out, retval] double *pdCurrentPosition);
				V_VT(&res) = VT_R8;
				hres = get_currentPosition((double*)&V_R8(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 335: [id(0x0038), propput]HRESULT currentPosition([in] double pdCurrentPosition);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R8);
				hres = put_currentPosition(V_R8(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x0039: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 338: [id(0x0039), propget]HRESULT currentPositionString([out, retval] BSTR *pbstrCurrentPosition);
				V_VT(&res) = VT_BSTR;
				hres = get_currentPositionString(&V_BSTR(&res));
			}
			break;
		}
		case 0x003a: {
			if (wFlags & DISPATCH_METHOD) {
				// line 341: [id(0x003a)]HRESULT next();
				hres = next();
			}
			break;
		}
		case 0x003b: {
			if (wFlags & DISPATCH_METHOD) {
				// line 344: [id(0x003b)]HRESULT previous();
				hres = previous();
			}
			break;
		}
		case 0x003c: {
			if (wFlags & DISPATCH_METHOD) {
				// line 347: [id(0x003c)]HRESULT currentItem([out, retval] IWMPMedia **ppIWMPMedia);
				V_VT(&res) = VT_DISPATCH;
				hres = currentItem((IWMPMedia**)&V_DISPATCH(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 350: [id(0x003c), propput]HRESULT currentItem([in] IWMPMedia *ppIWMPMedia);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = put_currentItem((IWMPMedia*)&var0);
				VariantClear(&var0);
			}
			break;
		}
		case 0x003d: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 353: [id(0x003d), propget]HRESULT currentMarker([out, retval] long *plMarker);
				V_VT(&res) = VT_I4;
				hres = get_currentMarker((LONG*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 356: [id(0x003d), propput]HRESULT currentMarker([in] long plMarker);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_currentMarker(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x003f: {
			if (wFlags & DISPATCH_METHOD) {
				// line 359: [id(0x003f)]HRESULT playItem([in] IWMPMedia *pIWMPMedia);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = playItem((IWMPMedia*)&var0);
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

STDMETHODIMP WMPControls::GetDocumentation(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile) {
	if (index == MEMBERID_NIL) {
		*pBstrName = SysAllocString(L"WMPControls");
		return S_OK;
	}
	return E_NOTIMPL;
}

STDMETHODIMP WMPSettings::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} namesIdsList[] = {
			{ NULL },
			{ L"autoStart", 0x0065 },
			{ L"balance", 0x0066 },
			{ L"baseURL", 0x006c },
			{ L"defaultFrame", 0x006d },
			{ L"enableErrorDialogs", 0x0070 },
			{ L"getMode", 0x006e },
			{ L"invokeURLs", 0x0067 },
			{ L"isAvailable", 0x0071 },
			{ L"mute", 0x0068 },
			{ L"playCount", 0x0069 },
			{ L"rate", 0x006a },
			{ L"setMode", 0x006f },
			{ L"volume", 0x006b }
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

STDMETHODIMP WMPSettings::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
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
		case 0x0071: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 399: [id(0x0071), propget]HRESULT isAvailable([in] BSTR bstrItem,[out, retval] VARIANT_BOOL *pIsAvailable);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				V_VT(&res) = VT_BOOL;
				hres = get_isAvailable(V_BSTR(&var0), &V_BOOL(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 0x0065: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 404: [id(0x0065), propget]HRESULT autoStart([out, retval] VARIANT_BOOL *pfAutoStart);
				V_VT(&res) = VT_BOOL;
				hres = get_autoStart(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 406: [id(0x0065), propput]HRESULT autoStart([in] VARIANT_BOOL pfAutoStart);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_autoStart(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x006c: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 409: [id(0x006c), propget]HRESULT baseURL([out, retval] BSTR *pbstrBaseURL);
				V_VT(&res) = VT_BSTR;
				hres = get_baseURL(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 411: [id(0x006c), propput]HRESULT baseURL([in] BSTR pbstrBaseURL);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_baseURL(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x006d: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 414: [id(0x006d), propget]HRESULT defaultFrame([out, retval] BSTR *pbstrDefaultFrame);
				V_VT(&res) = VT_BSTR;
				hres = get_defaultFrame(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 416: [id(0x006d), propput]HRESULT defaultFrame([in] BSTR pbstrDefaultFrame);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_defaultFrame(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x0067: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 419: [id(0x0067), propget]HRESULT invokeURLs([out, retval] VARIANT_BOOL *pfInvokeURLs);
				V_VT(&res) = VT_BOOL;
				hres = get_invokeURLs(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 421: [id(0x0067), propput]HRESULT invokeURLs([in] VARIANT_BOOL pfInvokeURLs);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_invokeURLs(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x0068: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 424: [id(0x0068), propget]HRESULT mute([out, retval] VARIANT_BOOL *pfMute);
				V_VT(&res) = VT_BOOL;
				hres = get_mute(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 426: [id(0x0068), propput]HRESULT mute([in] VARIANT_BOOL pfMute);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_mute(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x0069: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 429: [id(0x0069), propget]HRESULT playCount([out, retval] long *plCount);
				V_VT(&res) = VT_I4;
				hres = get_playCount((LONG*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 431: [id(0x0069), propput]HRESULT playCount([in] long plCount);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_playCount(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x006a: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 434: [id(0x006a), propget]HRESULT rate([out, retval] double *pdRate);
				V_VT(&res) = VT_R8;
				hres = get_rate((double*)&V_R8(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 436: [id(0x006a), propput]HRESULT rate([in] double pdRate);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R8);
				hres = put_rate(V_R8(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x0066: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 439: [id(0x0066), propget]HRESULT balance([out, retval] long *plBalance);
				V_VT(&res) = VT_I4;
				hres = get_balance((LONG*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 441: [id(0x0066), propput]HRESULT balance([in] long plBalance);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_balance(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x006b: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 444: [id(0x006b), propget]HRESULT volume([out, retval] long *plVolume);
				V_VT(&res) = VT_I4;
				hres = get_volume((LONG*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 446: [id(0x006b), propput]HRESULT volume([in] long plVolume);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_volume(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x006e: {
			if (wFlags & DISPATCH_METHOD) {
				// line 449: [id(0x006e)]HRESULT getMode([in] BSTR bstrMode,[out, retval] VARIANT_BOOL *pvarfMode);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				V_VT(&res) = VT_BOOL;
				hres = getMode(V_BSTR(&var0), &V_BOOL(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 0x006f: {
			if (wFlags & DISPATCH_METHOD) {
				// line 454: [id(0x006f)]HRESULT setMode([in] BSTR bstrMode,[in] VARIANT_BOOL varfMode);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = setMode(V_BSTR(&var0), V_BOOL(&var1));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x0070: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 459: [id(0x0070), propget]HRESULT enableErrorDialogs([out, retval] VARIANT_BOOL *pfEnableErrorDialogs);
				V_VT(&res) = VT_BOOL;
				hres = get_enableErrorDialogs(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 462: [id(0x0070), propput]HRESULT enableErrorDialogs([in] VARIANT_BOOL pfEnableErrorDialogs);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_enableErrorDialogs(V_BOOL(&var0));
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

STDMETHODIMP WMPSettings::GetDocumentation(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile) {
	if (index == MEMBERID_NIL) {
		*pBstrName = SysAllocString(L"WMPSettings");
		return S_OK;
	}
	return E_NOTIMPL;
}

STDMETHODIMP WMPCore::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} namesIdsList[] = {
			{ NULL },
			{ L"cdromCollection", 0x000e },
			{ L"close", 0x0003 },
			{ L"closedCaption", 0x000f },
			{ L"controls", 0x0004 },
			{ L"currentMedia", 0x0006 },
			{ L"currentPlaylist", 0x000d },
			{ L"Error", 0x0011 },
			{ L"isOnline", 0x0010 },
			{ L"launchURL", 0x000c },
			{ L"mediaCollection", 0x0008 },
			{ L"network", 0x0007 },
			{ L"openState", 0x0002 },
			{ L"playlistCollection", 0x0009 },
			{ L"playState", 0x000a },
			{ L"settings", 0x0005 },
			{ L"status", 0x0012 },
			{ L"URL", 0x0001 },
			{ L"versionInfo", 0x000b }
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

STDMETHODIMP WMPCore::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
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
		case 0x0003: {
			if (wFlags & DISPATCH_METHOD) {
				// line 925: [id(0x0003)]HRESULT close();
				hres = close();
			}
			break;
		}
		case 0x0001: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 928: [id(0x0001), propget]HRESULT URL([out, retval] BSTR *pbstrURL);
				V_VT(&res) = VT_BSTR;
				hres = get_URL(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 930: [id(0x0001), propput]HRESULT URL([in] BSTR pbstrURL);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_URL(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x0002: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 933: [id(0x0002), propget]HRESULT openState([out, retval] WMPOpenState *pwmpos);
				V_VT(&res) = VT_I4;
				hres = get_openState((WMPOpenState*)&V_I4(&res));
			}
			break;
		}
		case 0x000a: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 936: [id(0x000a), propget]HRESULT playState([out, retval] WMPPlayState *pwmpps);
				V_VT(&res) = VT_I4;
				hres = get_playState((WMPPlayState*)&V_I4(&res));
			}
			break;
		}
		case 0x0004: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 939: [id(0x0004), propget]HRESULT controls([out, retval] IWMPControls **ppControl);
				V_VT(&res) = VT_DISPATCH;
				hres = get_controls((IWMPControls**)&V_DISPATCH(&res));
			}
			break;
		}
		case 0x0005: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 942: [id(0x0005), propget]HRESULT settings([out, retval] IWMPSettings **ppSettings);
				V_VT(&res) = VT_DISPATCH;
				hres = get_settings((IWMPSettings**)&V_DISPATCH(&res));
			}
			break;
		}
		case 0x0006: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 945: [id(0x0006), propget]HRESULT currentMedia([out, retval] IWMPMedia **ppMedia);
				V_VT(&res) = VT_DISPATCH;
				hres = get_currentMedia((IWMPMedia**)&V_DISPATCH(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 947: [id(0x0006), propput]HRESULT currentMedia([in] IWMPMedia *ppMedia);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = put_currentMedia((IWMPMedia*)&var0);
				VariantClear(&var0);
			}
			break;
		}
		case 0x0008: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 950: [id(0x0008), propget]HRESULT mediaCollection([out, retval] IWMPMediaCollection **ppMediaCollection);
				V_VT(&res) = VT_DISPATCH;
				hres = get_mediaCollection((IWMPMediaCollection**)&V_DISPATCH(&res));
			}
			break;
		}
		case 0x0009: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 953: [id(0x0009), propget]HRESULT playlistCollection([out, retval] IWMPPlaylistCollection **ppPlaylistCollection);
				V_VT(&res) = VT_DISPATCH;
				hres = get_playlistCollection((IWMPPlaylistCollection**)&V_DISPATCH(&res));
			}
			break;
		}
		case 0x000b: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 956: [id(0x000b), propget]HRESULT versionInfo([out, retval] BSTR *pbstrVersionInfo);
				V_VT(&res) = VT_BSTR;
				hres = get_versionInfo(&V_BSTR(&res));
			}
			break;
		}
		case 0x000c: {
			if (wFlags & DISPATCH_METHOD) {
				// line 959: [id(0x000c)]HRESULT launchURL([in] BSTR bstrURL);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = launchURL(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x0007: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 962: [id(0x0007), propget]HRESULT network([out, retval] IWMPNetwork **ppQNI);
				V_VT(&res) = VT_DISPATCH;
				hres = get_network((IWMPNetwork**)&V_DISPATCH(&res));
			}
			break;
		}
		case 0x000d: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 965: [id(0x000d), propget]HRESULT currentPlaylist([out, retval] IWMPPlaylist **ppPL);
				V_VT(&res) = VT_DISPATCH;
				hres = get_currentPlaylist((IWMPPlaylist**)&V_DISPATCH(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 967: [id(0x000d), propput]HRESULT currentPlaylist([in] IWMPPlaylist *ppPL);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = put_currentPlaylist((IWMPPlaylist*)&var0);
				VariantClear(&var0);
			}
			break;
		}
		case 0x000e: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 970: [id(0x000e), propget]HRESULT cdromCollection([out, retval] IWMPCdromCollection **ppCdromCollection);
				V_VT(&res) = VT_DISPATCH;
				hres = get_cdromCollection((IWMPCdromCollection**)&V_DISPATCH(&res));
			}
			break;
		}
		case 0x000f: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 973: [id(0x000f), propget]HRESULT closedCaption([out, retval] IWMPClosedCaption **ppClosedCaption);
				V_VT(&res) = VT_DISPATCH;
				hres = get_closedCaption((IWMPClosedCaption**)&V_DISPATCH(&res));
			}
			break;
		}
		case 0x0010: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 976: [id(0x0010), propget]HRESULT isOnline([out, retval] VARIANT_BOOL *pfOnline);
				V_VT(&res) = VT_BOOL;
				hres = get_isOnline(&V_BOOL(&res));
			}
			break;
		}
		case 0x0011: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 979: [id(0x0011), propget]HRESULT Error([out, retval] IWMPError **ppError);
				V_VT(&res) = VT_DISPATCH;
				hres = get_Error((IWMPError**)&V_DISPATCH(&res));
			}
			break;
		}
		case 0x0012: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 982: [id(0x0012), propget]HRESULT status([out, retval] BSTR *pbstrStatus);
				V_VT(&res) = VT_BSTR;
				hres = get_status(&V_BSTR(&res));
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

STDMETHODIMP WMPCore::GetDocumentation(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile) {
	if (index == MEMBERID_NIL) {
		*pBstrName = SysAllocString(L"WMPCore");
		return S_OK;
	}
	return E_NOTIMPL;
}