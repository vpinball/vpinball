#include "stdafx.h"
#include "olectl.h"

robin_hood::unordered_map<wstring, int> Collection::m_nameIDMap = {
	robin_hood::pair<wstring, int> { wstring(L"item"), 0 },
	robin_hood::pair<wstring, int> { wstring(L"count"), 8000 }
};

STDMETHODIMP Collection::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	wstring name = wstring(*rgszNames);
	std::transform(name.begin(), name.end(), name.begin(), tolower);
	const robin_hood::unordered_map<wstring, int>::iterator it = m_nameIDMap.find(name);
	if (it != m_nameIDMap.end()) {
		*rgDispId = it->second;
		return S_OK;
	}
	return DISP_E_UNKNOWNNAME;
}

STDMETHODIMP Collection::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
	switch(dispIdMember) {
		case DISPID_NEWENUM: {
			// line 447: [propget, id(DISPID_NEWENUM),helpcontext(0x5009002)]HRESULT _NewEnum([out, retval] IUnknown** ppunk);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_UNKNOWN;
			return get__NewEnum(&V_UNKNOWN(pVarResult));
		}

		case 0: {
			// line 455: [propget, id(0),helpcontext(0x5009003)]HRESULT Item([in] long Index, [out, retval] IDispatch **ppobject);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			CComVariant var0;
			VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
			V_VT(pVarResult) = VT_DISPATCH;
			return get_Item(V_I4(&var0), (IDispatch**)&V_DISPATCH(pVarResult));
		}

		case 8000: {
			// line 459: [propget,helpcontext(0x5009004)]HRESULT Count([out, retval] long *Count);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_I4;
			return get_Count((long*)&V_I4(pVarResult));
		}

		default:
		break;
	}

	return DISP_E_UNKNOWNNAME;
}

robin_hood::unordered_map<int, wstring> Collection::m_idNameMap = {
	robin_hood::pair<int, wstring> { DISPID_GameEvents_Init, wstring(L"_Init") },
	robin_hood::pair<int, wstring> { DISPID_HitEvents_Hit, wstring(L"_Hit") },
	robin_hood::pair<int, wstring> { DISPID_SurfaceEvents_Slingshot, wstring(L"_Slingshot") },
	robin_hood::pair<int, wstring> { DISPID_TimerEvents_Timer, wstring(L"_Timer") },
	robin_hood::pair<int, wstring> { DISPID_SpinnerEvents_Spin, wstring(L"_Spin") },
	robin_hood::pair<int, wstring> { DISPID_HitEvents_Unhit, wstring(L"_Unhit") },
	robin_hood::pair<int, wstring> { DISPID_TargetEvents_Dropped, wstring(L"_Dropped") },
	robin_hood::pair<int, wstring> { DISPID_TargetEvents_Raised, wstring(L"_Raised") }
};

HRESULT Collection::FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) {
	CComPtr<IDispatch> disp;
	g_pplayer->m_ptable->m_pcv->m_pScript->GetScriptDispatch(nullptr, &disp);

	const robin_hood::unordered_map<int, wstring>::iterator it = m_idNameMap.find(dispid);
	if (it != m_idNameMap.end()) {
		wstring name = wstring(m_wzName) + it->second;
		LPOLESTR fnNames = (LPOLESTR)name.c_str();

		DISPID tDispid;
		const HRESULT hr = disp->GetIDsOfNames(IID_NULL, &fnNames, 1, 0, &tDispid);

		if (hr == S_OK) {
			disp->Invoke(tDispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, pdispparams, nullptr, nullptr, nullptr);
		}
	}

	return S_OK;
}

robin_hood::unordered_map<wstring, int> PinTable::m_nameIDMap = {
	robin_hood::pair<wstring, int> { wstring(L"glassheight"), 3 },
	robin_hood::pair<wstring, int> { wstring(L"playfieldmaterial"), 340 },
	robin_hood::pair<wstring, int> { wstring(L"backdropcolor"), 5 },
	robin_hood::pair<wstring, int> { wstring(L"slopemax"), 215 },
	robin_hood::pair<wstring, int> { wstring(L"slopemin"), 6 },
	robin_hood::pair<wstring, int> { wstring(L"inclination"), DISPID_Table_Inclination },
	robin_hood::pair<wstring, int> { wstring(L"fieldofview"), DISPID_Table_FieldOfView },
	robin_hood::pair<wstring, int> { wstring(L"layback"), DISPID_Table_Layback },
	robin_hood::pair<wstring, int> { wstring(L"rotation"), 99 },
	robin_hood::pair<wstring, int> { wstring(L"scalex"), 100 },
	robin_hood::pair<wstring, int> { wstring(L"scaley"), 101 },
	robin_hood::pair<wstring, int> { wstring(L"scalez"), 108 },
	robin_hood::pair<wstring, int> { wstring(L"xlatex"), 102 },
	robin_hood::pair<wstring, int> { wstring(L"xlatey"), 103 },
	robin_hood::pair<wstring, int> { wstring(L"xlatez"), 110 },
	robin_hood::pair<wstring, int> { wstring(L"gravity"), 1100 },
	robin_hood::pair<wstring, int> { wstring(L"friction"), 1101 },
	robin_hood::pair<wstring, int> { wstring(L"elasticity"), 1708 },
	robin_hood::pair<wstring, int> { wstring(L"elasticityfalloff"), 1709 },
	robin_hood::pair<wstring, int> { wstring(L"scatter"), 1710 },
	robin_hood::pair<wstring, int> { wstring(L"defaultscatter"), 1102 },
	robin_hood::pair<wstring, int> { wstring(L"nudgetime"), 1103 },
	robin_hood::pair<wstring, int> { wstring(L"plungernormalize"), 1104 },
	robin_hood::pair<wstring, int> { wstring(L"physicslooptime"), 1105 },
	robin_hood::pair<wstring, int> { wstring(L"plungerfilter"), 1107 },
	robin_hood::pair<wstring, int> { wstring(L"yieldtime"), 7 },
	robin_hood::pair<wstring, int> { wstring(L"ballimage"), DISPID_Image3 },
	robin_hood::pair<wstring, int> { wstring(L"backdropimage_dt"), DISPID_Image2 },
	robin_hood::pair<wstring, int> { wstring(L"backdropimage_fs"), DISPID_Image6 },
	robin_hood::pair<wstring, int> { wstring(L"backdropimage_fss"), DISPID_Image8 },
	robin_hood::pair<wstring, int> { wstring(L"backdropimageapplynightday"), 459 },
	robin_hood::pair<wstring, int> { wstring(L"colorgradeimage"), DISPID_Image5 },
	robin_hood::pair<wstring, int> { wstring(L"width"), DISPID_Table_Width },
	robin_hood::pair<wstring, int> { wstring(L"height"), DISPID_Table_Height },
	robin_hood::pair<wstring, int> { wstring(L"maxseparation"), DISPID_Table_MaxSeparation },
	robin_hood::pair<wstring, int> { wstring(L"zpd"), DISPID_Table_ZPD },
	robin_hood::pair<wstring, int> { wstring(L"offset"), DISPID_Table_Offset },
	robin_hood::pair<wstring, int> { wstring(L"globalstereo3d"), 427 },
	robin_hood::pair<wstring, int> { wstring(L"balldecalmode"), 438 },
	robin_hood::pair<wstring, int> { wstring(L"image"), DISPID_Image },
	robin_hood::pair<wstring, int> { wstring(L"filename"), 1711 },
	robin_hood::pair<wstring, int> { wstring(L"name"), DISPID_Name },
	robin_hood::pair<wstring, int> { wstring(L"enableantialiasing"), 394 },
	robin_hood::pair<wstring, int> { wstring(L"enableao"), 396 },
	robin_hood::pair<wstring, int> { wstring(L"enablefxaa"), 395 },
	robin_hood::pair<wstring, int> { wstring(L"enablessr"), 590 },
	robin_hood::pair<wstring, int> { wstring(L"bloomstrength"), 450 },
	robin_hood::pair<wstring, int> { wstring(L"ballfrontdecal"), DISPID_Image4 },
	robin_hood::pair<wstring, int> { wstring(L"overridephysics"), DISPID_Table_OverridePhysics },
	robin_hood::pair<wstring, int> { wstring(L"overridephysicsflippers"), 584 },
	robin_hood::pair<wstring, int> { wstring(L"enableemreels"), 13432 },
	robin_hood::pair<wstring, int> { wstring(L"enabledecals"), 13433 },
	robin_hood::pair<wstring, int> { wstring(L"showdt"), 13434 },
	robin_hood::pair<wstring, int> { wstring(L"showfss"), 625 },
	robin_hood::pair<wstring, int> { wstring(L"reflectelementsonplayfield"), 431 },
	robin_hood::pair<wstring, int> { wstring(L"environmentimage"), DISPID_Image7 },
	robin_hood::pair<wstring, int> { wstring(L"backglassmode"), 1714 },
	robin_hood::pair<wstring, int> { wstring(L"accelerometer"), 200 },
	robin_hood::pair<wstring, int> { wstring(L"accelnormalmount"), 201 },
	robin_hood::pair<wstring, int> { wstring(L"accelerometerangle"), 202 },
	robin_hood::pair<wstring, int> { wstring(L"globaldifficulty"), 209 },
	robin_hood::pair<wstring, int> { wstring(L"tableheight"), 214 },
	robin_hood::pair<wstring, int> { wstring(L"deadzone"), 217 },
	robin_hood::pair<wstring, int> { wstring(L"lightambient"), 558 },
	robin_hood::pair<wstring, int> { wstring(L"light0emission"), 559 },
	robin_hood::pair<wstring, int> { wstring(L"lightheight"), 564 },
	robin_hood::pair<wstring, int> { wstring(L"lightrange"), 565 },
	robin_hood::pair<wstring, int> { wstring(L"environmentemissionscale"), 566 },
	robin_hood::pair<wstring, int> { wstring(L"lightemissionscale"), 567 },
	robin_hood::pair<wstring, int> { wstring(L"aoscale"), 568 },
	robin_hood::pair<wstring, int> { wstring(L"ssrscale"), 569 },
	robin_hood::pair<wstring, int> { wstring(L"tablesoundvolume"), 579 },
	robin_hood::pair<wstring, int> { wstring(L"tablemusicvolume"), 580 },
	robin_hood::pair<wstring, int> { wstring(L"tableadaptivevsync"), 585 },
	robin_hood::pair<wstring, int> { wstring(L"ballreflection"), 1700 },
	robin_hood::pair<wstring, int> { wstring(L"playfieldreflectionstrength"), 1707 },
	robin_hood::pair<wstring, int> { wstring(L"balltrail"), 1704 },
	robin_hood::pair<wstring, int> { wstring(L"trailstrength"), 1705 },
	robin_hood::pair<wstring, int> { wstring(L"ballplayfieldreflectionscale"), 1712 },
	robin_hood::pair<wstring, int> { wstring(L"defaultbulbintensityscale"), 1713 },
	robin_hood::pair<wstring, int> { wstring(L"detaillevel"), 420 },
	robin_hood::pair<wstring, int> { wstring(L"nightday"), 436 },
	robin_hood::pair<wstring, int> { wstring(L"globalalphaacc"), 398 },
	robin_hood::pair<wstring, int> { wstring(L"globaldaynight"), 588 },
	robin_hood::pair<wstring, int> { wstring(L"version"), 219 },
	robin_hood::pair<wstring, int> { wstring(L"versionmajor"), 38 },
	robin_hood::pair<wstring, int> { wstring(L"versionminor"), 39 },
	robin_hood::pair<wstring, int> { wstring(L"versionrevision"), 40 },
	robin_hood::pair<wstring, int> { wstring(L"vpbuildversion"), 24 }
};

STDMETHODIMP PinTable::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	wstring name = wstring(*rgszNames);
	std::transform(name.begin(), name.end(), name.begin(), tolower);
	const robin_hood::unordered_map<wstring, int>::iterator it = m_nameIDMap.find(name);
	if (it != m_nameIDMap.end()) {
		*rgDispId = it->second;
		return S_OK;
	}
	return DISP_E_UNKNOWNNAME;
}

STDMETHODIMP PinTable::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
	switch(dispIdMember) {
		case 3:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 494: [propget, id(3), helpstring("property GlassHeight")] HRESULT GlassHeight([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_GlassHeight(&V_R4(pVarResult));
			}

			case 1: {
				// line 495: [propput, id(3), helpstring("property GlassHeight")] HRESULT GlassHeight([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_GlassHeight(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 340:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 496: [propget, id(340), helpstring("property PlayfieldMaterial")] HRESULT PlayfieldMaterial([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_PlayfieldMaterial(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 497: [propput, id(340), helpstring("property PlayfieldMaterial")] HRESULT PlayfieldMaterial([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_PlayfieldMaterial(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case 5:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 498: [propget, id(5), helpstring("property BackdropColor")] HRESULT BackdropColor([out, retval] OLE_COLOR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_UI4;
				return get_BackdropColor(&V_UI4(pVarResult));
			}

			case 1: {
				// line 499: [propput, id(5), helpstring("property BackdropColor")] HRESULT BackdropColor([in] OLE_COLOR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_UI4);
				return put_BackdropColor((OLE_COLOR)V_UI4(&var0));
			}

			default:
			break;
		}
		break;

		case 215:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 500: [propget, id(215), helpstring("property SlopeMax")] HRESULT SlopeMax([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_SlopeMax(&V_R4(pVarResult));
			}

			case 1: {
				// line 501: [propput, id(215), helpstring("property SlopeMax")] HRESULT SlopeMax([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_SlopeMax(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 6:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 502: [propget, id(6), helpstring("property SlopeMin")] HRESULT SlopeMin([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_SlopeMin(&V_R4(pVarResult));
			}

			case 1: {
				// line 503: [propput, id(6), helpstring("property SlopeMin")] HRESULT SlopeMin([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_SlopeMin(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Table_Inclination:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 505: [propget, id(DISPID_Table_Inclination), helpstring("property Inclination")] HRESULT Inclination([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Inclination(&V_R4(pVarResult));
			}

			case 1: {
				// line 506: [propput, id(DISPID_Table_Inclination), helpstring("property Inclination")] HRESULT Inclination([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Inclination(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Table_FieldOfView:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 507: [propget, id(DISPID_Table_FieldOfView), helpstring("property FieldOfView")] HRESULT FieldOfView([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_FieldOfView(&V_R4(pVarResult));
			}

			case 1: {
				// line 508: [propput, id(DISPID_Table_FieldOfView), helpstring("property FieldOfView")] HRESULT FieldOfView([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_FieldOfView(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Table_Layback:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 509: [propget, id(DISPID_Table_Layback), helpstring("property Layback")] HRESULT Layback([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Layback(&V_R4(pVarResult));
			}

			case 1: {
				// line 510: [propput, id(DISPID_Table_Layback), helpstring("property Layback")] HRESULT Layback([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Layback(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 99:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 511: [propget, id(99), helpstring("property Rotation")] HRESULT Rotation([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Rotation(&V_R4(pVarResult));
			}

			case 1: {
				// line 512: [propput, id(99), helpstring("property Rotation")] HRESULT Rotation([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Rotation(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 100:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 513: [propget, id(100), helpstring("property Scalex")] HRESULT Scalex([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Scalex(&V_R4(pVarResult));
			}

			case 1: {
				// line 514: [propput, id(100), helpstring("property Scalex")] HRESULT Scalex([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Scalex(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 101:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 515: [propget, id(101), helpstring("property Scaley")] HRESULT Scaley([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Scaley(&V_R4(pVarResult));
			}

			case 1: {
				// line 516: [propput, id(101), helpstring("property Scaley")] HRESULT Scaley([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Scaley(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 108:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 517: [propget, id(108), helpstring("property Scalez")] HRESULT Scalez([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Scalez(&V_R4(pVarResult));
			}

			case 1: {
				// line 518: [propput, id(108), helpstring("property Scalez")] HRESULT Scalez([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Scalez(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 102:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 519: [propget, id(102), helpstring("property Xlatex")] HRESULT Xlatex([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Xlatex(&V_R4(pVarResult));
			}

			case 1: {
				// line 520: [propput, id(102), helpstring("property Xlatex")] HRESULT Xlatex([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Xlatex(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 103:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 521: [propget, id(103), helpstring("property Xlatey")] HRESULT Xlatey([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Xlatey(&V_R4(pVarResult));
			}

			case 1: {
				// line 522: [propput, id(103), helpstring("property Xlatey")] HRESULT Xlatey([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Xlatey(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 110:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 523: [propget, id(110), helpstring("property Xlatez")] HRESULT Xlatez([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Xlatez(&V_R4(pVarResult));
			}

			case 1: {
				// line 524: [propput, id(110), helpstring("property Xlatez")] HRESULT Xlatez([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Xlatez(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 1100:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 526: [propget, id(1100), helpstring("property Gravity")] HRESULT Gravity([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Gravity(&V_R4(pVarResult));
			}

			case 1: {
				// line 527: [propput, id(1100), helpstring("property Gravity")] HRESULT Gravity([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Gravity(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 1101:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 528: [propget, id(1101), helpstring("property Friction")] HRESULT Friction([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Friction(&V_R4(pVarResult));
			}

			case 1: {
				// line 529: [propput, id(1101), helpstring("property Friction")] HRESULT Friction([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Friction(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 1708:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 530: [propget, id(1708), helpstring("property Elasticity")] HRESULT Elasticity([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Elasticity(&V_R4(pVarResult));
			}

			case 1: {
				// line 531: [propput, id(1708), helpstring("property Elasticity")] HRESULT Elasticity([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Elasticity(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 1709:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 532: [propget, id(1709), helpstring("property ElasticityFalloff")] HRESULT ElasticityFalloff([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_ElasticityFalloff(&V_R4(pVarResult));
			}

			case 1: {
				// line 533: [propput, id(1709), helpstring("property ElasticityFalloff")] HRESULT ElasticityFalloff([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_ElasticityFalloff(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 1710:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 534: [propget, id(1710), helpstring("property Scatter")] HRESULT Scatter([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Scatter(&V_R4(pVarResult));
			}

			case 1: {
				// line 535: [propput, id(1710), helpstring("property Scatter")] HRESULT Scatter([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Scatter(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 1102:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 536: [propget, id(1102), helpstring("property DefaultScatter")] HRESULT DefaultScatter([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_DefaultScatter(&V_R4(pVarResult));
			}

			case 1: {
				// line 537: [propput, id(1102), helpstring("property DefaultScatter")] HRESULT DefaultScatter([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_DefaultScatter(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 1103:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 538: [propget, id(1103), helpstring("property NudgeTime")] HRESULT NudgeTime([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_NudgeTime(&V_R4(pVarResult));
			}

			case 1: {
				// line 539: [propput, id(1103), helpstring("property NudgeTime")] HRESULT NudgeTime([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_NudgeTime(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 1104:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 540: [propget, id(1104), helpstring("property PlungerNormalize")] HRESULT PlungerNormalize([out, retval] int *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_PlungerNormalize(&V_I4(pVarResult));
			}

			case 1: {
				// line 541: [propput, id(1104), helpstring("property PlungerNormalize")] HRESULT PlungerNormalize([in] int newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_PlungerNormalize(V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case 1105:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 542: [propget, id(1105), helpstring("property PhysicsLoopTime")] HRESULT PhysicsLoopTime([out, retval] int *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_PhysicsLoopTime(&V_I4(pVarResult));
			}

			case 1: {
				// line 543: [propput, id(1105), helpstring("property PhysicsLoopTime")] HRESULT PhysicsLoopTime([in] int newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_PhysicsLoopTime(V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case 1107:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 545: [propget, id(1107), helpstring("property PlungerFilter")] HRESULT PlungerFilter([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_PlungerFilter(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 546: [propput, id(1107), helpstring("property PlungerFilter")] HRESULT PlungerFilter([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_PlungerFilter(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 7:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 548: [propget, id(7), helpstring("property YieldTime")] HRESULT YieldTime([out, retval] long *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_YieldTime((long*)&V_I4(pVarResult));
			}

			case 1: {
				// line 549: [propput, id(7), helpstring("property YieldTime")] HRESULT YieldTime([in] long newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_YieldTime(V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Image3:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 550: [propget, id(DISPID_Image3), helpstring("property BallImage")] HRESULT BallImage([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_BallImage(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 551: [propput, id(DISPID_Image3), helpstring("property BallImage")] HRESULT BallImage([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_BallImage(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Image2:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 552: [propget, id(DISPID_Image2), helpstring("property BackdropImage_DT")] HRESULT BackdropImage_DT([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_BackdropImage_DT(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 553: [propput, id(DISPID_Image2), helpstring("property BackdropImage_DT")] HRESULT BackdropImage_DT([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_BackdropImage_DT(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Image6:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 554: [propget, id(DISPID_Image6), helpstring("property BackdropImage_FS")] HRESULT BackdropImage_FS([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_BackdropImage_FS(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 555: [propput, id(DISPID_Image6), helpstring("property BackdropImage_FS")] HRESULT BackdropImage_FS([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_BackdropImage_FS(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Image8:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 556: [propget, id(DISPID_Image8), helpstring("property BackdropImage_FSS")] HRESULT BackdropImage_FSS([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_BackdropImage_FSS(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 557: [propput, id(DISPID_Image8), helpstring("property BackdropImage_FSS")] HRESULT BackdropImage_FSS([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_BackdropImage_FSS(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case 459:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 558: [propget, id(459), helpstring("property BackdropImageApplyNightDay")] HRESULT BackdropImageApplyNightDay([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_BackdropImageApplyNightDay(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 559: [propput, id(459), helpstring("property BackdropImageApplyNightDay")] HRESULT BackdropImageApplyNightDay([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_BackdropImageApplyNightDay(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Image5:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 560: [propget, id(DISPID_Image5), helpstring("property ColorGradeImage")] HRESULT ColorGradeImage([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_ColorGradeImage(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 561: [propput, id(DISPID_Image5), helpstring("property ColorGradeImage")] HRESULT ColorGradeImage([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_ColorGradeImage(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Table_Width:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 562: [propget, id(DISPID_Table_Width), helpstring("property Width")] HRESULT Width([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Width(&V_R4(pVarResult));
			}

			case 1: {
				// line 563: [propput, id(DISPID_Table_Width), helpstring("property Width")] HRESULT Width([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Width(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Table_Height:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 564: [propget, id(DISPID_Table_Height), helpstring("property Height")] HRESULT Height([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Height(&V_R4(pVarResult));
			}

			case 1: {
				// line 565: [propput, id(DISPID_Table_Height), helpstring("property Height")] HRESULT Height([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Height(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Table_MaxSeparation:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 566: [propget, id(DISPID_Table_MaxSeparation), helpstring("property MaxSeparation")] HRESULT MaxSeparation([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_MaxSeparation(&V_R4(pVarResult));
			}

			case 1: {
				// line 567: [propput, id(DISPID_Table_MaxSeparation), helpstring("property MaxSeparation")] HRESULT MaxSeparation([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_MaxSeparation(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Table_ZPD:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 568: [propget, id(DISPID_Table_ZPD), helpstring("property ZPD")] HRESULT ZPD([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_ZPD(&V_R4(pVarResult));
			}

			case 1: {
				// line 569: [propput, id(DISPID_Table_ZPD), helpstring("property ZPD")] HRESULT ZPD([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_ZPD(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Table_Offset:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 570: [propget, id(DISPID_Table_Offset), helpstring("property Offset")] HRESULT Offset([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Offset(&V_R4(pVarResult));
			}

			case 1: {
				// line 571: [propput, id(DISPID_Table_Offset), helpstring("property Offset")] HRESULT Offset([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Offset(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 427:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 572: [propget, id(427), helpstring("property Overwrite3DStereo")] HRESULT GlobalStereo3D([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_GlobalStereo3D(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 573: [propput, id(427), helpstring("property Overwrite3DStereo")] HRESULT GlobalStereo3D([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_GlobalStereo3D(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 438:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 574: [propget, id(438), helpstring("property BallDecalMode")] HRESULT BallDecalMode([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_BallDecalMode(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 575: [propput, id(438), helpstring("property BallDecalMode")] HRESULT BallDecalMode([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_BallDecalMode(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Image:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 576: [propget, id(DISPID_Image), helpstring("property Image")] HRESULT Image([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Image(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 577: [propput, id(DISPID_Image), helpstring("property Image")] HRESULT Image([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Image(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case 1711: {
			// line 578: [propget, id(1711), helpstring("property FileName")] HRESULT FileName([out, retval] BSTR *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_BSTR;
			return get_FileName(&V_BSTR(pVarResult));
		}

		case DISPID_Name:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 579: [propget, id(DISPID_Name), helpstring("property Name")] HRESULT Name([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Name(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 580: [propput, id(DISPID_Name), helpstring("property Name")] HRESULT Name([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Name(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case 394:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 581: [propget, id(394), helpstring("property enableAA")] HRESULT EnableAntialiasing([out, retval] UserDefaultOnOff *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_EnableAntialiasing((UserDefaultOnOff*)&V_I4(pVarResult));
			}

			case 1: {
				// line 582: [propput, id(394), helpstring("property enableAA")] HRESULT EnableAntialiasing([in] UserDefaultOnOff newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_EnableAntialiasing((UserDefaultOnOff)V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case 396:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 583: [propget, id(396), helpstring("property enableAO")] HRESULT EnableAO([out, retval] UserDefaultOnOff *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_EnableAO((UserDefaultOnOff*)&V_I4(pVarResult));
			}

			case 1: {
				// line 584: [propput, id(396), helpstring("property enableAO")] HRESULT EnableAO([in] UserDefaultOnOff newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_EnableAO((UserDefaultOnOff)V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case 395:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 585: [propget, id(395), helpstring("property enableFXAA")] HRESULT EnableFXAA([out, retval] FXAASettings *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_EnableFXAA((FXAASettings*)&V_I4(pVarResult));
			}

			case 1: {
				// line 586: [propput, id(395), helpstring("property enableFXAA")] HRESULT EnableFXAA([in] FXAASettings newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_EnableFXAA((FXAASettings)V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case 590:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 587: [propget, id(590), helpstring("property enableSSR")] HRESULT EnableSSR([out, retval] UserDefaultOnOff *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_EnableSSR((UserDefaultOnOff*)&V_I4(pVarResult));
			}

			case 1: {
				// line 588: [propput, id(590), helpstring("property enableSSR")] HRESULT EnableSSR([in] UserDefaultOnOff newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_EnableSSR((UserDefaultOnOff)V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case 450:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 589: [propget, id(450), helpstring("property BloomStrength")] HRESULT BloomStrength([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_BloomStrength(&V_R4(pVarResult));
			}

			case 1: {
				// line 590: [propput, id(450), helpstring("property BloomStrength")] HRESULT BloomStrength([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_BloomStrength(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Image4:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 591: [propget, id(DISPID_Image4), helpstring("property BallFrontDecal")] HRESULT BallFrontDecal([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_BallFrontDecal(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 592: [propput, id(DISPID_Image4), helpstring("property BallFrontDecal")] HRESULT BallFrontDecal([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_BallFrontDecal(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Table_OverridePhysics:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 593: [propget, id(DISPID_Table_OverridePhysics), helpstring("property OverridePhysics")] HRESULT OverridePhysics([out, retval] PhysicsSet *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_OverridePhysics((PhysicsSet*)&V_I4(pVarResult));
			}

			case 1: {
				// line 594: [propput, id(DISPID_Table_OverridePhysics), helpstring("property OverridePhysics")] HRESULT OverridePhysics([in] PhysicsSet newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_OverridePhysics((PhysicsSet)V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case 584:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 595: [propget, id(584), helpstring("property OverridePhysicsFlippers")] HRESULT OverridePhysicsFlippers([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_OverridePhysicsFlippers(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 596: [propput, id(584), helpstring("property OverridePhysicsFlippers")] HRESULT OverridePhysicsFlippers([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_OverridePhysicsFlippers(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 13432:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 597: [propget, id(13432), helpstring("property RenderEMReels")] HRESULT EnableEMReels([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_EnableEMReels(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 598: [propput, id(13432), helpstring("property RenderEMReels")] HRESULT EnableEMReels([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_EnableEMReels(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 13433:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 599: [propget, id(13433), helpstring("property RenderDecals")] HRESULT EnableDecals([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_EnableDecals(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 600: [propput, id(13433), helpstring("property RenderDecals")] HRESULT EnableDecals([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_EnableDecals(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 13434:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 601: [propget, id(13434), helpstring("property ShowDT")] HRESULT ShowDT([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_ShowDT(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 602: [propput, id(13434), helpstring("property ShowDT")] HRESULT ShowDT([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_ShowDT(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 625:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 603: [propget, id(625), helpstring("property ShowFSS")] HRESULT ShowFSS([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_ShowFSS(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 604: [propput, id(625), helpstring("property ShowFSS")] HRESULT ShowFSS([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_ShowFSS(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 431:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 605: [propget, id(431), helpstring("property ReflectElementsOnPlayfield")] HRESULT ReflectElementsOnPlayfield([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_ReflectElementsOnPlayfield(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 606: [propput, id(431), helpstring("property ReflectElementsOnPlayfield")] HRESULT ReflectElementsOnPlayfield([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_ReflectElementsOnPlayfield(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Image7:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 607: [propget, id(DISPID_Image7), helpstring("property EnvironmentImage")] HRESULT EnvironmentImage([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_EnvironmentImage(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 608: [propput, id(DISPID_Image7), helpstring("property EnvironmentImage")] HRESULT EnvironmentImage([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_EnvironmentImage(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case 1714:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 609: [propget, id(1714), helpstring("property BackglassMode")] HRESULT BackglassMode([out, retval] BackglassIndex *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_BackglassMode((BackglassIndex*)&V_I4(pVarResult));
			}

			case 1: {
				// line 610: [propput, id(1714), helpstring("property BackglassMode")] HRESULT BackglassMode([in] BackglassIndex newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_BackglassMode((BackglassIndex)V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case 200:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 613: [propget, id(200), helpstring("property Accelerometer")] HRESULT Accelerometer([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_Accelerometer(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 614: [propput, id(200), helpstring("property Accelerometer")] HRESULT Accelerometer([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_Accelerometer(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 201:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 615: [propget, id(201), helpstring("property AccelNormalMount")] HRESULT AccelNormalMount([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_AccelNormalMount(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 616: [propput, id(201), helpstring("property AccelNormalMount")] HRESULT AccelNormalMount([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_AccelNormalMount(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 202:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 618: [propget, id(202), helpstring("property AccelerometerAngle")] HRESULT AccelerometerAngle([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_AccelerometerAngle(&V_R4(pVarResult));
			}

			case 1: {
				// line 619: [propput, id(202), helpstring("property AccelerometerAngle")] HRESULT AccelerometerAngle([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_AccelerometerAngle(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 209:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 632: [propget, id(209), helpstring("property GlobalDifficulty")] HRESULT GlobalDifficulty([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_GlobalDifficulty(&V_R4(pVarResult));
			}

			case 1: {
				// line 633: [propput, id(209), helpstring("property GlobalDifficulty")] HRESULT GlobalDifficulty([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_GlobalDifficulty(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 214:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 635: [propget, id(214), helpstring("property TableHeight")] HRESULT TableHeight([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_TableHeight(&V_R4(pVarResult));
			}

			case 1: {
				// line 636: [propput, id(214), helpstring("property TableHeight")] HRESULT TableHeight([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_TableHeight(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 217:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 638: [propget, id(217), helpstring("property DeadZone")] HRESULT DeadZone([out, retval] int *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_DeadZone(&V_I4(pVarResult));
			}

			case 1: {
				// line 639: [propput, id(217), helpstring("property DeadZone")] HRESULT DeadZone([in] int newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_DeadZone(V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case 558:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 641: [propget, id(558), helpstring("property LightAmbient")] HRESULT LightAmbient([out, retval] OLE_COLOR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_UI4;
				return get_LightAmbient(&V_UI4(pVarResult));
			}

			case 1: {
				// line 642: [propput, id(558), helpstring("property LightAmbient")] HRESULT LightAmbient([in] OLE_COLOR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_UI4);
				return put_LightAmbient((OLE_COLOR)V_UI4(&var0));
			}

			default:
			break;
		}
		break;

		case 559:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 643: [propget, id(559), helpstring("property Light0Emission")] HRESULT Light0Emission([out, retval] OLE_COLOR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_UI4;
				return get_Light0Emission(&V_UI4(pVarResult));
			}

			case 1: {
				// line 644: [propput, id(559), helpstring("property Light0Emission")] HRESULT Light0Emission([in] OLE_COLOR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_UI4);
				return put_Light0Emission((OLE_COLOR)V_UI4(&var0));
			}

			default:
			break;
		}
		break;

		case 564:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 645: [propget, id(564), helpstring("property LightHeight")] HRESULT LightHeight([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_LightHeight(&V_R4(pVarResult));
			}

			case 1: {
				// line 646: [propput, id(564), helpstring("property LightHeight")] HRESULT LightHeight([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_LightHeight(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 565:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 647: [propget, id(565), helpstring("property LightRange")] HRESULT LightRange([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_LightRange(&V_R4(pVarResult));
			}

			case 1: {
				// line 648: [propput, id(565), helpstring("property LightRange")] HRESULT LightRange([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_LightRange(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 566:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 650: [propget, id(566), helpstring("property EnvironmentEmissionScale")] HRESULT EnvironmentEmissionScale([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_EnvironmentEmissionScale(&V_R4(pVarResult));
			}

			case 1: {
				// line 651: [propput, id(566), helpstring("property EnvironmentEmissionScale")] HRESULT EnvironmentEmissionScale([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_EnvironmentEmissionScale(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 567:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 652: [propget, id(567), helpstring("property LightEmissionScale")] HRESULT LightEmissionScale([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_LightEmissionScale(&V_R4(pVarResult));
			}

			case 1: {
				// line 653: [propput, id(567), helpstring("property LightEmissionScale")] HRESULT LightEmissionScale([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_LightEmissionScale(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 568:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 654: [propget, id(568), helpstring("property AOScale")] HRESULT AOScale([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_AOScale(&V_R4(pVarResult));
			}

			case 1: {
				// line 655: [propput, id(568), helpstring("property AOScale")] HRESULT AOScale([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_AOScale(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 569:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 656: [propget, id(569), helpstring("property SSRScale")] HRESULT SSRScale([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_SSRScale(&V_R4(pVarResult));
			}

			case 1: {
				// line 657: [propput, id(569), helpstring("property SSRScale")] HRESULT SSRScale([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_SSRScale(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 579:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 659: [propget, id(579), helpstring("property TableSoundVolume")] HRESULT TableSoundVolume([out, retval] int *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_TableSoundVolume(&V_I4(pVarResult));
			}

			case 1: {
				// line 660: [propput, id(579), helpstring("property TableSoundVolume")] HRESULT TableSoundVolume([in] int newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_TableSoundVolume(V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case 580:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 661: [propget, id(580), helpstring("property TableMusicVolume")] HRESULT TableMusicVolume([out, retval] int *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_TableMusicVolume(&V_I4(pVarResult));
			}

			case 1: {
				// line 662: [propput, id(580), helpstring("property TableMusicVolume")] HRESULT TableMusicVolume([in] int newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_TableMusicVolume(V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case 585:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 663: [propget, id(585), helpstring("property TableAdaptiveVSync")] HRESULT TableAdaptiveVSync([out, retval] int *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_TableAdaptiveVSync(&V_I4(pVarResult));
			}

			case 1: {
				// line 664: [propput, id(585), helpstring("property TableAdaptiveVSync")] HRESULT TableAdaptiveVSync([in] int newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_TableAdaptiveVSync(V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case 1700:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 665: [propget, id(1700), helpstring("property BallReflection")] HRESULT BallReflection([out, retval] UserDefaultOnOff *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_BallReflection((UserDefaultOnOff*)&V_I4(pVarResult));
			}

			case 1: {
				// line 666: [propput, id(1700), helpstring("property BallReflection")] HRESULT BallReflection([in] UserDefaultOnOff newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_BallReflection((UserDefaultOnOff)V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case 1707:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 667: [propget, id(1707), helpstring("property PlayfieldReflectionStrength")] HRESULT PlayfieldReflectionStrength([out, retval] int *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_PlayfieldReflectionStrength(&V_I4(pVarResult));
			}

			case 1: {
				// line 668: [propput, id(1707), helpstring("property PlayfieldReflectionStrength")] HRESULT PlayfieldReflectionStrength([in] int newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_PlayfieldReflectionStrength(V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case 1704:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 669: [propget, id(1704), helpstring("property BallTrail")] HRESULT BallTrail([out, retval] UserDefaultOnOff *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_BallTrail((UserDefaultOnOff*)&V_I4(pVarResult));
			}

			case 1: {
				// line 670: [propput, id(1704), helpstring("property BallTrail")] HRESULT BallTrail([in] UserDefaultOnOff newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_BallTrail((UserDefaultOnOff)V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case 1705:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 671: [propget, id(1705), helpstring("property TrailStrength")] HRESULT TrailStrength([out, retval] int *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_TrailStrength(&V_I4(pVarResult));
			}

			case 1: {
				// line 672: [propput, id(1705), helpstring("property TrailStrength")] HRESULT TrailStrength([in] int newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_TrailStrength(V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case 1712:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 673: [propget, id(1712), helpstring("property BallPlayfieldReflectionScale")] HRESULT BallPlayfieldReflectionScale([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_BallPlayfieldReflectionScale(&V_R4(pVarResult));
			}

			case 1: {
				// line 674: [propput, id(1712), helpstring("property BallPlayfieldReflectionScale")] HRESULT BallPlayfieldReflectionScale([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_BallPlayfieldReflectionScale(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 1713:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 675: [propget, id(1713), helpstring("property DefaultBulbIntensityScale")] HRESULT DefaultBulbIntensityScale([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_DefaultBulbIntensityScale(&V_R4(pVarResult));
			}

			case 1: {
				// line 676: [propput, id(1713), helpstring("property DefaultBulbIntensityScale")] HRESULT DefaultBulbIntensityScale([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_DefaultBulbIntensityScale(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 420:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 677: [propget, id(420), helpstring("property DetailLevel")] HRESULT DetailLevel([out, retval] int *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_DetailLevel(&V_I4(pVarResult));
			}

			case 1: {
				// line 678: [propput, id(420), helpstring("property DetailLevel")] HRESULT DetailLevel([in] int newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_DetailLevel(V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case 436:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 679: [propget, id(436), helpstring("property NightDay")] HRESULT NightDay([out, retval] int *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_NightDay(&V_I4(pVarResult));
			}

			case 1: {
				// line 680: [propput, id(436), helpstring("property NightDay")] HRESULT NightDay([in] int newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_NightDay(V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case 398:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 681: [propget, id(398), helpstring("property OverwriteAlphaAcc")] HRESULT GlobalAlphaAcc([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_GlobalAlphaAcc(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 682: [propput, id(398), helpstring("property OverwriteAlphaAcc")] HRESULT GlobalAlphaAcc([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_GlobalAlphaAcc(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 588:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 683: [propget, id(588), helpstring("property OverwriteDayNight")] HRESULT GlobalDayNight([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_GlobalDayNight(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 684: [propput, id(588), helpstring("property OverwriteDayNight")] HRESULT GlobalDayNight([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_GlobalDayNight(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 219: {
			// line 685: [propget, id(219), helpstring("property Version")] HRESULT Version([out, retval] int *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_I4;
			return get_Version(&V_I4(pVarResult));
		}

		case 38: {
			// line 686: [propget, id(38), helpstring("property VersionMajor")] HRESULT VersionMajor([out, retval] int *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_I4;
			return get_VersionMajor(&V_I4(pVarResult));
		}

		case 39: {
			// line 687: [propget, id(39), helpstring("property VersionMinor")] HRESULT VersionMinor([out, retval] int *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_I4;
			return get_VersionMinor(&V_I4(pVarResult));
		}

		case 40: {
			// line 688: [propget, id(40), helpstring("property VersionRevision")] HRESULT VersionRevision([out, retval] int *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_I4;
			return get_VersionRevision(&V_I4(pVarResult));
		}

		case 24: {
			// line 689: [propget, id(24), helpstring("property VPBuildVersion")] HRESULT VPBuildVersion([out, retval] int *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_I4;
			return get_VPBuildVersion(&V_I4(pVarResult));
		}

		default:
		break;
	}

	return DISP_E_UNKNOWNNAME;
}

robin_hood::unordered_map<wstring, int> ScriptGlobalTable::m_nameIDMap = {
	robin_hood::pair<wstring, int> { wstring(L"playsound"), 3 },
	robin_hood::pair<wstring, int> { wstring(L"leftflipperkey"), 4 },
	robin_hood::pair<wstring, int> { wstring(L"rightflipperkey"), 5 },
	robin_hood::pair<wstring, int> { wstring(L"lefttiltkey"), 6 },
	robin_hood::pair<wstring, int> { wstring(L"righttiltkey"), 7 },
	robin_hood::pair<wstring, int> { wstring(L"centertiltkey"), 8 },
	robin_hood::pair<wstring, int> { wstring(L"plungerkey"), 9 },
	robin_hood::pair<wstring, int> { wstring(L"playmusic"), 10 },
	robin_hood::pair<wstring, int> { wstring(L"musicvolume"), 15 },
	robin_hood::pair<wstring, int> { wstring(L"endmusic"), 11 },
	robin_hood::pair<wstring, int> { wstring(L"startgamekey"), 12 },
	robin_hood::pair<wstring, int> { wstring(L"userdirectory"), 13 },
	robin_hood::pair<wstring, int> { wstring(L"getplayerhwnd"), 14 },
	robin_hood::pair<wstring, int> { wstring(L"stopsound"), 16 },
	robin_hood::pair<wstring, int> { wstring(L"savevalue"), 17 },
	robin_hood::pair<wstring, int> { wstring(L"loadvalue"), 18 },
	robin_hood::pair<wstring, int> { wstring(L"activeball"), 19 },
	robin_hood::pair<wstring, int> { wstring(L"addcreditkey"), 20 },
	robin_hood::pair<wstring, int> { wstring(L"addcreditkey2"), 67 },
	robin_hood::pair<wstring, int> { wstring(L"gametime"), 22 },
	robin_hood::pair<wstring, int> { wstring(L"systemtime"), 225 },
	robin_hood::pair<wstring, int> { wstring(L"getcustomparam"), 823 },
	robin_hood::pair<wstring, int> { wstring(L"gettextfile"), 23 },
	robin_hood::pair<wstring, int> { wstring(L"beginmodal"), 25 },
	robin_hood::pair<wstring, int> { wstring(L"endmodal"), 26 },
	robin_hood::pair<wstring, int> { wstring(L"nudge"), DISPID_Table_Nudge },
	robin_hood::pair<wstring, int> { wstring(L"nudgegetcalibration"), 804 },
	robin_hood::pair<wstring, int> { wstring(L"nudgesetcalibration"), 805 },
	robin_hood::pair<wstring, int> { wstring(L"nudgesensorstatus"), 806 },
	robin_hood::pair<wstring, int> { wstring(L"nudgetiltstatus"), 807 },
	robin_hood::pair<wstring, int> { wstring(L"mechanicaltilt"), 30 },
	robin_hood::pair<wstring, int> { wstring(L"leftmagnasave"), 31 },
	robin_hood::pair<wstring, int> { wstring(L"rightmagnasave"), 32 },
	robin_hood::pair<wstring, int> { wstring(L"exitgame"), 34 },
	robin_hood::pair<wstring, int> { wstring(L"lockbarkey"), 803 },
	robin_hood::pair<wstring, int> { wstring(L"fireknocker"), 33 },
	robin_hood::pair<wstring, int> { wstring(L"quitplayer"), 37 },
	robin_hood::pair<wstring, int> { wstring(L"showdt"), 13434 },
	robin_hood::pair<wstring, int> { wstring(L"showfss"), 625 },
	robin_hood::pair<wstring, int> { wstring(L"nightday"), 436 },
	robin_hood::pair<wstring, int> { wstring(L"getballs"), 41 },
	robin_hood::pair<wstring, int> { wstring(L"getelements"), 42 },
	robin_hood::pair<wstring, int> { wstring(L"getelementbyname"), 43 },
	robin_hood::pair<wstring, int> { wstring(L"activetable"), 48 },
	robin_hood::pair<wstring, int> { wstring(L"dmdwidth"), 44 },
	robin_hood::pair<wstring, int> { wstring(L"dmdheight"), 45 },
	robin_hood::pair<wstring, int> { wstring(L"dmdpixels"), 46 },
	robin_hood::pair<wstring, int> { wstring(L"dmdcoloredpixels"), 47 },
	robin_hood::pair<wstring, int> { wstring(L"version"), 219 },
	robin_hood::pair<wstring, int> { wstring(L"versionmajor"), 38 },
	robin_hood::pair<wstring, int> { wstring(L"versionminor"), 39 },
	robin_hood::pair<wstring, int> { wstring(L"versionrevision"), 40 },
	robin_hood::pair<wstring, int> { wstring(L"vpbuildversion"), 24 },
	robin_hood::pair<wstring, int> { wstring(L"getserialdevices"), 249 },
	robin_hood::pair<wstring, int> { wstring(L"openserial"), 250 },
	robin_hood::pair<wstring, int> { wstring(L"closeserial"), 251 },
	robin_hood::pair<wstring, int> { wstring(L"flushserial"), 252 },
	robin_hood::pair<wstring, int> { wstring(L"setupserial"), 253 },
	robin_hood::pair<wstring, int> { wstring(L"readserial"), 254 },
	robin_hood::pair<wstring, int> { wstring(L"writeserial"), 255 },
	robin_hood::pair<wstring, int> { wstring(L"renderingmode"), 218 },
	robin_hood::pair<wstring, int> { wstring(L"updatematerial"), 230 },
	robin_hood::pair<wstring, int> { wstring(L"getmaterial"), 231 },
	robin_hood::pair<wstring, int> { wstring(L"updatematerialphysics"), 247 },
	robin_hood::pair<wstring, int> { wstring(L"getmaterialphysics"), 248 },
	robin_hood::pair<wstring, int> { wstring(L"materialcolor"), 224 },
	robin_hood::pair<wstring, int> { wstring(L"windowwidth"), 226 },
	robin_hood::pair<wstring, int> { wstring(L"windowheight"), 227 }
};

STDMETHODIMP ScriptGlobalTable::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	wstring name = wstring(*rgszNames);
	std::transform(name.begin(), name.end(), name.begin(), tolower);
	const robin_hood::unordered_map<wstring, int>::iterator it = m_nameIDMap.find(name);
	if (it != m_nameIDMap.end()) {
		*rgDispId = it->second;
		return S_OK;
	}
	return DISP_E_UNKNOWNNAME;
}

STDMETHODIMP ScriptGlobalTable::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
	switch(dispIdMember) {
		case 3: {
			// line 700: [id(3), helpstring("method PlaySound")] HRESULT PlaySound(BSTR Sound, [defaultvalue(1)] long LoopCount, [defaultvalue(1)] float Volume, [defaultvalue(0)] float pan, [defaultvalue(0)] float randompitch, [defaultvalue(0)] long pitch, [defaultvalue(0)] VARIANT_BOOL usesame, [defaultvalue(1)] VARIANT_BOOL restart, [defaultvalue(0)] float front_rear_fade);
			int index = pDispParams->cArgs;
			CComVariant var0;
			VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
			CComVariant var1(1);
			VariantChangeType(&var1, (index > 0) ? &pDispParams->rgvarg[--index] : &var1, 0, VT_I4);
			CComVariant var2(1);
			VariantChangeType(&var2, (index > 0) ? &pDispParams->rgvarg[--index] : &var2, 0, VT_R4);
			CComVariant var3(0);
			VariantChangeType(&var3, (index > 0) ? &pDispParams->rgvarg[--index] : &var3, 0, VT_R4);
			CComVariant var4(0);
			VariantChangeType(&var4, (index > 0) ? &pDispParams->rgvarg[--index] : &var4, 0, VT_R4);
			CComVariant var5(0);
			VariantChangeType(&var5, (index > 0) ? &pDispParams->rgvarg[--index] : &var5, 0, VT_I4);
			CComVariant var6(0);
			VariantChangeType(&var6, (index > 0) ? &pDispParams->rgvarg[--index] : &var6, 0, VT_BOOL);
			CComVariant var7(1);
			VariantChangeType(&var7, (index > 0) ? &pDispParams->rgvarg[--index] : &var7, 0, VT_BOOL);
			CComVariant var8(0);
			VariantChangeType(&var8, (index > 0) ? &pDispParams->rgvarg[--index] : &var8, 0, VT_R4);
			return PlaySound(V_BSTR(&var0), V_I4(&var1), V_R4(&var2), V_R4(&var3), V_R4(&var4), V_I4(&var5), V_BOOL(&var6), V_BOOL(&var7), V_R4(&var8));
		}

		case 4: {
			// line 701: [propget, id(4), helpstring("property LeftFlipperKey")] HRESULT LeftFlipperKey([out, retval] long *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_I4;
			return get_LeftFlipperKey((long*)&V_I4(pVarResult));
		}

		case 5: {
			// line 702: [propget, id(5), helpstring("property RightFlipperKey")] HRESULT RightFlipperKey([out, retval] long *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_I4;
			return get_RightFlipperKey((long*)&V_I4(pVarResult));
		}

		case 6: {
			// line 703: [propget, id(6), helpstring("property LeftTiltKey")] HRESULT LeftTiltKey([out, retval] long *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_I4;
			return get_LeftTiltKey((long*)&V_I4(pVarResult));
		}

		case 7: {
			// line 704: [propget, id(7), helpstring("property RightTiltKey")] HRESULT RightTiltKey([out, retval] long *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_I4;
			return get_RightTiltKey((long*)&V_I4(pVarResult));
		}

		case 8: {
			// line 705: [propget, id(8), helpstring("property CenterTiltKey")] HRESULT CenterTiltKey([out, retval] long *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_I4;
			return get_CenterTiltKey((long*)&V_I4(pVarResult));
		}

		case 9: {
			// line 706: [propget, id(9), helpstring("property PlungerKey")] HRESULT PlungerKey([out, retval] long *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_I4;
			return get_PlungerKey((long*)&V_I4(pVarResult));
		}

		case 10: {
			// line 707: [id(10), helpstring("method PlayMusic")] HRESULT PlayMusic(BSTR str, [defaultvalue(1)] float Volume);
			int index = pDispParams->cArgs;
			CComVariant var0;
			VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
			CComVariant var1(1);
			VariantChangeType(&var1, (index > 0) ? &pDispParams->rgvarg[--index] : &var1, 0, VT_R4);
			return PlayMusic(V_BSTR(&var0), V_R4(&var1));
		}

		case 15: {
			// line 708: [propput, id(15), helpstring("property MusicVolume")] HRESULT MusicVolume(float Volume);
			CComVariant var0;
			VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
			return put_MusicVolume(V_R4(&var0));
		}

		case 11: {
			// line 709: [id(11), helpstring("method EndMusic")] HRESULT EndMusic();
			return EndMusic();
		}

		case 12: {
			// line 710: [propget, id(12), helpstring("property StartGameKey")] HRESULT StartGameKey([out, retval] long *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_I4;
			return get_StartGameKey((long*)&V_I4(pVarResult));
		}

		case 13: {
			// line 711: [propget, id(13), helpstring("property UserDirectory")] HRESULT UserDirectory([out, retval] BSTR *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_BSTR;
			return get_UserDirectory(&V_BSTR(pVarResult));
		}

		case 14: {
			// line 713: [propget, id(14), helpstring("property GetPlayerHWnd")] HRESULT GetPlayerHWnd([out, retval] SIZE_T *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_UI4;
			return get_GetPlayerHWnd((SIZE_T*)&V_UI4(pVarResult));
		}

		case 16: {
			// line 718: [id(16), helpstring("method StopSound")] HRESULT StopSound(BSTR Sound);
			CComVariant var0;
			VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
			return StopSound(V_BSTR(&var0));
		}

		case 17: {
			// line 719: [id(17), helpstring("method SaveValue")] HRESULT SaveValue(BSTR TableName, BSTR ValueName, VARIANT Value);
			int index = pDispParams->cArgs;
			CComVariant var0;
			VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
			CComVariant var1;
			VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_BSTR);
			CComVariant var2;
			VariantChangeType(&var2, &pDispParams->rgvarg[--index], 0, VT_VARIANT);
			return SaveValue(V_BSTR(&var0), V_BSTR(&var1), var2);
		}

		case 18: {
			// line 720: [id(18), helpstring("method LoadValue")] HRESULT LoadValue(BSTR TableName, BSTR ValueName, [out, retval] VARIANT *Value);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			int index = pDispParams->cArgs;
			CComVariant var0;
			VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
			CComVariant var1;
			VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_BSTR);
			return LoadValue(V_BSTR(&var0), V_BSTR(&var1), pVarResult);
		}

		case 19: {
			// line 721: [propget, id(19), helpstring("property ActiveBall")] HRESULT ActiveBall([out, retval] IBall **pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_DISPATCH;
			return get_ActiveBall((IBall**)&V_DISPATCH(pVarResult));
		}

		case 20: {
			// line 722: [propget, id(20), helpstring("property AddCreditKey")] HRESULT AddCreditKey([out, retval] long *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_I4;
			return get_AddCreditKey((long*)&V_I4(pVarResult));
		}

		case 67: {
			// line 723: [propget, id(67), helpstring("property AddCreditKey2")] HRESULT AddCreditKey2([out, retval] long *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_I4;
			return get_AddCreditKey2((long*)&V_I4(pVarResult));
		}

		case 22: {
			// line 724: [propget, id(22), helpstring("property GameTime")] HRESULT GameTime([out, retval] long *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_I4;
			return get_GameTime((long*)&V_I4(pVarResult));
		}

		case 225: {
			// line 725: [propget, id(225), helpstring("property SystemTime")] HRESULT SystemTime([out, retval] long *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_I4;
			return get_SystemTime((long*)&V_I4(pVarResult));
		}

		case 823: {
			// line 726: [id(823), helpstring("method GetCustomParam")] HRESULT GetCustomParam(long index, [out, retval] BSTR *param);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			CComVariant var0;
			VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
			V_VT(pVarResult) = VT_BSTR;
			return GetCustomParam(V_I4(&var0), &V_BSTR(pVarResult));
		}

		case 23: {
			// line 727: [id(23), helpstring("method GetTextFile")] HRESULT GetTextFile(BSTR FileName, [out, retval] BSTR *pContents);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			CComVariant var0;
			VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
			V_VT(pVarResult) = VT_BSTR;
			return GetTextFile(V_BSTR(&var0), &V_BSTR(pVarResult));
		}

		case 25: {
			// line 728: [id(25), helpstring("method BeginModal")] HRESULT BeginModal();
			return BeginModal();
		}

		case 26: {
			// line 729: [id(26), helpstring("method EndModal")] HRESULT EndModal();
			return EndModal();
		}

		case DISPID_Table_Nudge: {
			// line 730: [id(DISPID_Table_Nudge), helpstring("method Nudge")] HRESULT Nudge([in] float Angle, [in] float Force);
			int index = pDispParams->cArgs;
			CComVariant var0;
			VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
			CComVariant var1;
			VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_R4);
			return Nudge(V_R4(&var0), V_R4(&var1));
		}

		case 804: {
			// line 731: [id(804), helpstring("method NudgeGetCalibration")] HRESULT NudgeGetCalibration([out] VARIANT *XMax, [out] VARIANT *YMax, [out] VARIANT *XGain, [out] VARIANT *YGain, [out] VARIANT *DeadZone, [out] VARIANT *TiltSensitivty);
			int index = pDispParams->cArgs;
			VARIANT* var0 = &pDispParams->rgvarg[--index];
			VARIANT* var1 = &pDispParams->rgvarg[--index];
			VARIANT* var2 = &pDispParams->rgvarg[--index];
			VARIANT* var3 = &pDispParams->rgvarg[--index];
			VARIANT* var4 = &pDispParams->rgvarg[--index];
			VARIANT* var5 = &pDispParams->rgvarg[--index];
			return NudgeGetCalibration(var0, var1, var2, var3, var4, var5);
		}

		case 805: {
			// line 732: [id(805), helpstring("method NudgeSetCalibration")] HRESULT NudgeSetCalibration([in] int XMax, [in] int YMax, [in] int XGain, [in] int YGain, [in] int DeadZone, [in] int TiltSensitivty);
			int index = pDispParams->cArgs;
			CComVariant var0;
			VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
			CComVariant var1;
			VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_I4);
			CComVariant var2;
			VariantChangeType(&var2, &pDispParams->rgvarg[--index], 0, VT_I4);
			CComVariant var3;
			VariantChangeType(&var3, &pDispParams->rgvarg[--index], 0, VT_I4);
			CComVariant var4;
			VariantChangeType(&var4, &pDispParams->rgvarg[--index], 0, VT_I4);
			CComVariant var5;
			VariantChangeType(&var5, &pDispParams->rgvarg[--index], 0, VT_I4);
			return NudgeSetCalibration(V_I4(&var0), V_I4(&var1), V_I4(&var2), V_I4(&var3), V_I4(&var4), V_I4(&var5));
		}

		case 806: {
			// line 733: [id(806), helpstring("method NudgeSensorStatus")] HRESULT NudgeSensorStatus([out] VARIANT *XNudge, [out] VARIANT *YNudge);
			int index = pDispParams->cArgs;
			VARIANT* var0 = &pDispParams->rgvarg[--index];
			VARIANT* var1 = &pDispParams->rgvarg[--index];
			return NudgeSensorStatus(var0, var1);
		}

		case 807: {
			// line 734: [id(807), helpstring("method NudgeTiltStatus")] HRESULT NudgeTiltStatus([out] VARIANT *XPlumb, [out] VARIANT *YPlumb, [out] VARIANT *TiltPercent);
			int index = pDispParams->cArgs;
			VARIANT* var0 = &pDispParams->rgvarg[--index];
			VARIANT* var1 = &pDispParams->rgvarg[--index];
			VARIANT* var2 = &pDispParams->rgvarg[--index];
			return NudgeTiltStatus(var0, var1, var2);
		}

		case 30: {
			// line 736: [propget, id(30), helpstring("property MechanicalTilt")] HRESULT MechanicalTilt([out, retval] long *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_I4;
			return get_MechanicalTilt((long*)&V_I4(pVarResult));
		}

		case 31: {
			// line 737: [propget, id(31), helpstring("property LeftMagnaSave")] HRESULT LeftMagnaSave([out, retval] long *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_I4;
			return get_LeftMagnaSave((long*)&V_I4(pVarResult));
		}

		case 32: {
			// line 738: [propget, id(32), helpstring("property RightMagnaSave")] HRESULT RightMagnaSave([out, retval] long *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_I4;
			return get_RightMagnaSave((long*)&V_I4(pVarResult));
		}

		case 34: {
			// line 739: [propget, id(34), helpstring("property ExitGame")] HRESULT ExitGame([out, retval] long *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_I4;
			return get_ExitGame((long*)&V_I4(pVarResult));
		}

		case 803: {
			// line 740: [propget, id(803), helpstring("property LockbarKey")] HRESULT LockbarKey([out, retval] long *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_I4;
			return get_LockbarKey((long*)&V_I4(pVarResult));
		}

		case 33: {
			// line 741: [id(33), helpstring("method FireKnocker")] HRESULT FireKnocker([defaultvalue(1)] int Count);
			int index = pDispParams->cArgs;
			CComVariant var0(1);
			VariantChangeType(&var0, (index > 0) ? &pDispParams->rgvarg[--index] : &var0, 0, VT_I4);
			return FireKnocker(V_I4(&var0));
		}

		case 37: {
			// line 742: [id(37), helpstring("method QuitPlayer")] HRESULT QuitPlayer([defaultvalue(0)] int CloseType);
			int index = pDispParams->cArgs;
			CComVariant var0(0);
			VariantChangeType(&var0, (index > 0) ? &pDispParams->rgvarg[--index] : &var0, 0, VT_I4);
			return QuitPlayer(V_I4(&var0));
		}

		case 13434: {
			// line 744: [propget, id(13434), helpstring("property ShowDT")] HRESULT ShowDT([out, retval] VARIANT_BOOL *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_BOOL;
			return get_ShowDT(&V_BOOL(pVarResult));
		}

		case 625: {
			// line 745: [propget, id(625), helpstring("property ShowFSS")] HRESULT ShowFSS([out, retval] VARIANT_BOOL *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_BOOL;
			return get_ShowFSS(&V_BOOL(pVarResult));
		}

		case 436: {
			// line 747: [propget, id(436), helpstring("property NightDay")] HRESULT NightDay([out, retval] int *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_I4;
			return get_NightDay(&V_I4(pVarResult));
		}

		case 41: {
			// line 749: [id(41), helpstring("method GetBalls")] HRESULT GetBalls([out, retval] SAFEARRAY(VARIANT) *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_SAFEARRAY;
			return GetBalls((SAFEARRAY**)&V_ARRAY(pVarResult));
		}

		case 42: {
			// line 750: [id(42), helpstring("method GetElements")] HRESULT GetElements([out, retval] SAFEARRAY(VARIANT) *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_SAFEARRAY;
			return GetElements((SAFEARRAY**)&V_ARRAY(pVarResult));
		}

		case 43: {
			// line 751: [id(43), helpstring("method GetElementByName")] HRESULT GetElementByName([in] BSTR name, [out, retval] IDispatch* *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			CComVariant var0;
			VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
			V_VT(pVarResult) = VT_DISPATCH;
			return GetElementByName(V_BSTR(&var0), (IDispatch**)&V_DISPATCH(pVarResult));
		}

		case 48: {
			// line 752: [propget, id(48), helpstring("property ActiveTable")] HRESULT ActiveTable([out, retval] ITable **pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_DISPATCH;
			return get_ActiveTable((ITable**)&V_DISPATCH(pVarResult));
		}

		case 44: {
			// line 754: [propput, id(44), helpstring("property DMDWidth")] HRESULT DMDWidth([in] int pVal);
			CComVariant var0;
			VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
			return put_DMDWidth(V_I4(&var0));
		}

		case 45: {
			// line 755: [propput, id(45), helpstring("property DMDHeight")] HRESULT DMDHeight([in] int pVal);
			CComVariant var0;
			VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
			return put_DMDHeight(V_I4(&var0));
		}

		case 46: {
			// line 756: [propput, id(46), helpstring("property DMDPixels")] HRESULT DMDPixels([in] VARIANT pVal);
			CComVariant var0;
			VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_VARIANT);
			return put_DMDPixels(var0);
		}

		case 47: {
			// line 757: [propput, id(47), helpstring("property DMDColoredPixels")] HRESULT DMDColoredPixels([in] VARIANT pVal);
			CComVariant var0;
			VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_VARIANT);
			return put_DMDColoredPixels(var0);
		}

		case 219: {
			// line 759: [propget, id(219), helpstring("property Version")] HRESULT Version([out, retval] int *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_I4;
			return get_Version(&V_I4(pVarResult));
		}

		case 38: {
			// line 760: [propget, id(38), helpstring("property VersionMajor")] HRESULT VersionMajor([out, retval] int *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_I4;
			return get_VersionMajor(&V_I4(pVarResult));
		}

		case 39: {
			// line 761: [propget, id(39), helpstring("property VersionMinor")] HRESULT VersionMinor([out, retval] int *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_I4;
			return get_VersionMinor(&V_I4(pVarResult));
		}

		case 40: {
			// line 762: [propget, id(40), helpstring("property VersionRevision")] HRESULT VersionRevision([out, retval] int *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_I4;
			return get_VersionRevision(&V_I4(pVarResult));
		}

		case 24: {
			// line 763: [propget, id(24), helpstring("property VPBuildVersion")] HRESULT VPBuildVersion([out, retval] int *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_I4;
			return get_VPBuildVersion(&V_I4(pVarResult));
		}

		case 249: {
			// line 765: [id(249), helpstring("method GetSerialDevices")] HRESULT GetSerialDevices([out, retval] VARIANT *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			return GetSerialDevices(pVarResult);
		}

		case 250: {
			// line 766: [id(250), helpstring("method OpenSerial")] HRESULT OpenSerial([in] BSTR device);
			CComVariant var0;
			VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
			return OpenSerial(V_BSTR(&var0));
		}

		case 251: {
			// line 767: [id(251), helpstring("method CloseSerial")] HRESULT CloseSerial();
			return CloseSerial();
		}

		case 252: {
			// line 768: [id(252), helpstring("method FlushSerial")] HRESULT FlushSerial();
			return FlushSerial();
		}

		case 253: {
			// line 769: [id(253), helpstring("method SetupSerial")] HRESULT SetupSerial([in] int baud, [in] int bits, [in] int parity, [in] int stopbit, [in] VARIANT_BOOL rts, [in] VARIANT_BOOL dtr);
			int index = pDispParams->cArgs;
			CComVariant var0;
			VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
			CComVariant var1;
			VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_I4);
			CComVariant var2;
			VariantChangeType(&var2, &pDispParams->rgvarg[--index], 0, VT_I4);
			CComVariant var3;
			VariantChangeType(&var3, &pDispParams->rgvarg[--index], 0, VT_I4);
			CComVariant var4;
			VariantChangeType(&var4, &pDispParams->rgvarg[--index], 0, VT_BOOL);
			CComVariant var5;
			VariantChangeType(&var5, &pDispParams->rgvarg[--index], 0, VT_BOOL);
			return SetupSerial(V_I4(&var0), V_I4(&var1), V_I4(&var2), V_I4(&var3), V_BOOL(&var4), V_BOOL(&var5));
		}

		case 254: {
			// line 770: [id(254), helpstring("method ReadSerial")] HRESULT ReadSerial([in] int size, [out] VARIANT *pVal);
			int index = pDispParams->cArgs;
			CComVariant var0;
			VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
			VARIANT* var1 = &pDispParams->rgvarg[--index];
			return ReadSerial(V_I4(&var0), var1);
		}

		case 255: {
			// line 771: [id(255), helpstring("method WriteSerial")] HRESULT WriteSerial([in] VARIANT pVal);
			CComVariant var0;
			VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_VARIANT);
			return WriteSerial(var0);
		}

		case 218: {
			// line 773: [propget, id(218), helpstring("property RenderingMode")] HRESULT RenderingMode([out, retval] int *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_I4;
			return get_RenderingMode(&V_I4(pVarResult));
		}

		case 230: {
			// line 775: [id(230), helpstring("method UpdateMaterial")] HRESULT UpdateMaterial([in] BSTR pVal, [in] float wrapLighting, [in] float roughness, [in] float glossyImageLerp, [in] float thickness, [in] float edge, [in] float edgeAlpha, [in] float opacity,[in] OLE_COLOR base, [in] OLE_COLOR glossy, [in] OLE_COLOR clearcoat, [in] VARIANT_BOOL isMetal, [in] VARIANT_BOOL opacityActive,[in] float elasticity, [in] float elasticityFalloff, [in] float friction, [in] float scatterAngle);
			int index = pDispParams->cArgs;
			CComVariant var0;
			VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
			CComVariant var1;
			VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_R4);
			CComVariant var2;
			VariantChangeType(&var2, &pDispParams->rgvarg[--index], 0, VT_R4);
			CComVariant var3;
			VariantChangeType(&var3, &pDispParams->rgvarg[--index], 0, VT_R4);
			CComVariant var4;
			VariantChangeType(&var4, &pDispParams->rgvarg[--index], 0, VT_R4);
			CComVariant var5;
			VariantChangeType(&var5, &pDispParams->rgvarg[--index], 0, VT_R4);
			CComVariant var6;
			VariantChangeType(&var6, &pDispParams->rgvarg[--index], 0, VT_R4);
			CComVariant var7;
			VariantChangeType(&var7, &pDispParams->rgvarg[--index], 0, VT_R4);
			CComVariant var8;
			VariantChangeType(&var8, &pDispParams->rgvarg[--index], 0, VT_UI4);
			CComVariant var9;
			VariantChangeType(&var9, &pDispParams->rgvarg[--index], 0, VT_UI4);
			CComVariant var10;
			VariantChangeType(&var10, &pDispParams->rgvarg[--index], 0, VT_UI4);
			CComVariant var11;
			VariantChangeType(&var11, &pDispParams->rgvarg[--index], 0, VT_BOOL);
			CComVariant var12;
			VariantChangeType(&var12, &pDispParams->rgvarg[--index], 0, VT_BOOL);
			CComVariant var13;
			VariantChangeType(&var13, &pDispParams->rgvarg[--index], 0, VT_R4);
			CComVariant var14;
			VariantChangeType(&var14, &pDispParams->rgvarg[--index], 0, VT_R4);
			CComVariant var15;
			VariantChangeType(&var15, &pDispParams->rgvarg[--index], 0, VT_R4);
			CComVariant var16;
			VariantChangeType(&var16, &pDispParams->rgvarg[--index], 0, VT_R4);
			return UpdateMaterial(V_BSTR(&var0), V_R4(&var1), V_R4(&var2), V_R4(&var3), V_R4(&var4), V_R4(&var5), V_R4(&var6), V_R4(&var7), (OLE_COLOR)V_UI4(&var8), (OLE_COLOR)V_UI4(&var9), (OLE_COLOR)V_UI4(&var10), V_BOOL(&var11), V_BOOL(&var12), V_R4(&var13), V_R4(&var14), V_R4(&var15), V_R4(&var16));
		}

		case 231: {
			// line 778: [id(231), helpstring("method GetMaterial")] HRESULT GetMaterial([in] BSTR pVal, [out] VARIANT *wrapLighting, [out] VARIANT *roughness, [out] VARIANT *glossyImageLerp, [out] VARIANT *thickness, [out] VARIANT *edge, [out] VARIANT *edgeAlpha, [out] VARIANT *opacity,[out] VARIANT *base, [out] VARIANT *glossy, [out] VARIANT *clearcoat, [out] VARIANT *isMetal, [out] VARIANT *opacityActive,[out] VARIANT *elasticity, [out] VARIANT *elasticityFalloff, [out] VARIANT *friction, [out] VARIANT *scatterAngle);
			int index = pDispParams->cArgs;
			CComVariant var0;
			VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
			VARIANT* var1 = &pDispParams->rgvarg[--index];
			VARIANT* var2 = &pDispParams->rgvarg[--index];
			VARIANT* var3 = &pDispParams->rgvarg[--index];
			VARIANT* var4 = &pDispParams->rgvarg[--index];
			VARIANT* var5 = &pDispParams->rgvarg[--index];
			VARIANT* var6 = &pDispParams->rgvarg[--index];
			VARIANT* var7 = &pDispParams->rgvarg[--index];
			VARIANT* var8 = &pDispParams->rgvarg[--index];
			VARIANT* var9 = &pDispParams->rgvarg[--index];
			VARIANT* var10 = &pDispParams->rgvarg[--index];
			VARIANT* var11 = &pDispParams->rgvarg[--index];
			VARIANT* var12 = &pDispParams->rgvarg[--index];
			VARIANT* var13 = &pDispParams->rgvarg[--index];
			VARIANT* var14 = &pDispParams->rgvarg[--index];
			VARIANT* var15 = &pDispParams->rgvarg[--index];
			VARIANT* var16 = &pDispParams->rgvarg[--index];
			return GetMaterial(V_BSTR(&var0), var1, var2, var3, var4, var5, var6, var7, var8, var9, var10, var11, var12, var13, var14, var15, var16);
		}

		case 247: {
			// line 781: [id(247), helpstring("method UpdateMaterialPhysics")] HRESULT UpdateMaterialPhysics([in] BSTR pVal,[in] float elasticity, [in] float elasticityFalloff, [in] float friction, [in] float scatterAngle);
			int index = pDispParams->cArgs;
			CComVariant var0;
			VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
			CComVariant var1;
			VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_R4);
			CComVariant var2;
			VariantChangeType(&var2, &pDispParams->rgvarg[--index], 0, VT_R4);
			CComVariant var3;
			VariantChangeType(&var3, &pDispParams->rgvarg[--index], 0, VT_R4);
			CComVariant var4;
			VariantChangeType(&var4, &pDispParams->rgvarg[--index], 0, VT_R4);
			return UpdateMaterialPhysics(V_BSTR(&var0), V_R4(&var1), V_R4(&var2), V_R4(&var3), V_R4(&var4));
		}

		case 248: {
			// line 783: [id(248), helpstring("method GetMaterialPhysics")] HRESULT GetMaterialPhysics([in] BSTR pVal,[out] VARIANT *elasticity, [out] VARIANT *elasticityFalloff, [out] VARIANT *friction, [out] VARIANT *scatterAngle);
			int index = pDispParams->cArgs;
			CComVariant var0;
			VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
			VARIANT* var1 = &pDispParams->rgvarg[--index];
			VARIANT* var2 = &pDispParams->rgvarg[--index];
			VARIANT* var3 = &pDispParams->rgvarg[--index];
			VARIANT* var4 = &pDispParams->rgvarg[--index];
			return GetMaterialPhysics(V_BSTR(&var0), var1, var2, var3, var4);
		}

		case 224: {
			// line 785: [id(224), helpstring("method MaterialColor")] HRESULT MaterialColor([in] BSTR name, [in] OLE_COLOR newVal);
			int index = pDispParams->cArgs;
			CComVariant var0;
			VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
			CComVariant var1;
			VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_UI4);
			return MaterialColor(V_BSTR(&var0), (OLE_COLOR)V_UI4(&var1));
		}

		case 226: {
			// line 787: [propget, id(226), helpstring("property WindowWidth")] HRESULT WindowWidth([out, retval] int *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_I4;
			return get_WindowWidth(&V_I4(pVarResult));
		}

		case 227: {
			// line 788: [propget, id(227), helpstring("property WindowHeight")] HRESULT WindowHeight([out, retval] int *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_I4;
			return get_WindowHeight(&V_I4(pVarResult));
		}

		default:
		break;
	}

	return DISP_E_UNKNOWNNAME;
}

robin_hood::unordered_map<int, wstring> PinTable::m_idNameMap = {
	robin_hood::pair<int, wstring> { DISPID_GameEvents_Init, wstring(L"_Init") },
	robin_hood::pair<int, wstring> { DISPID_GameEvents_KeyDown, wstring(L"_KeyDown") },
	robin_hood::pair<int, wstring> { DISPID_GameEvents_KeyUp, wstring(L"_KeyUp") },
	robin_hood::pair<int, wstring> { DISPID_GameEvents_MusicDone, wstring(L"_MusicDone") },
	robin_hood::pair<int, wstring> { DISPID_GameEvents_Exit, wstring(L"_Exit") },
	robin_hood::pair<int, wstring> { DISPID_GameEvents_Paused, wstring(L"_Paused") },
	robin_hood::pair<int, wstring> { DISPID_GameEvents_UnPaused, wstring(L"_UnPaused") }
};

HRESULT PinTable::FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) {
	CComPtr<IDispatch> disp;
	g_pplayer->m_ptable->m_pcv->m_pScript->GetScriptDispatch(nullptr, &disp);

	const robin_hood::unordered_map<int, wstring>::iterator it = m_idNameMap.find(dispid);
	if (it != m_idNameMap.end()) {
		wstring name = wstring(m_wzName) + it->second;
		LPOLESTR fnNames = (LPOLESTR)name.c_str();

		DISPID tDispid;
		const HRESULT hr = disp->GetIDsOfNames(IID_NULL, &fnNames, 1, 0, &tDispid);

		if (hr == S_OK) {
			disp->Invoke(tDispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, pdispparams, nullptr, nullptr, nullptr);
		}
	}

	return S_OK;
}

robin_hood::unordered_map<wstring, int> DebuggerModule::m_nameIDMap = {
	robin_hood::pair<wstring, int> { wstring(L"print"), 10 }
};

STDMETHODIMP DebuggerModule::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	wstring name = wstring(*rgszNames);
	std::transform(name.begin(), name.end(), name.begin(), tolower);
	const robin_hood::unordered_map<wstring, int>::iterator it = m_nameIDMap.find(name);
	if (it != m_nameIDMap.end()) {
		*rgDispId = it->second;
		return S_OK;
	}
	return DISP_E_UNKNOWNNAME;
}

STDMETHODIMP DebuggerModule::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
	switch(dispIdMember) {
		case 10: {
			// line 818: [id(10), helpstring("method Print")] HRESULT Print([in, optional] VARIANT* pvar);
			int index = pDispParams->cArgs;
			CComVariant var0;
			if (index > 0) {
				VariantCopy(&var0,  &pDispParams->rgvarg[--index]);
			}
			return Print(&var0);
		}

		default:
		break;
	}

	return DISP_E_UNKNOWNNAME;
}

robin_hood::unordered_map<wstring, int> Surface::m_nameIDMap = {
	robin_hood::pair<wstring, int> { wstring(L"name"), DISPID_Name },
	robin_hood::pair<wstring, int> { wstring(L"timerenabled"), DISPID_Timer_Enabled },
	robin_hood::pair<wstring, int> { wstring(L"timerinterval"), DISPID_Timer_Interval },
	robin_hood::pair<wstring, int> { wstring(L"hashitevent"), 34 },
	robin_hood::pair<wstring, int> { wstring(L"threshold"), 33 },
	robin_hood::pair<wstring, int> { wstring(L"image"), DISPID_Image },
	robin_hood::pair<wstring, int> { wstring(L"sidematerial"), 341 },
	robin_hood::pair<wstring, int> { wstring(L"imagealignment"), 7 },
	robin_hood::pair<wstring, int> { wstring(L"heightbottom"), 8 },
	robin_hood::pair<wstring, int> { wstring(L"heighttop"), 9 },
	robin_hood::pair<wstring, int> { wstring(L"topmaterial"), 340 },
	robin_hood::pair<wstring, int> { wstring(L"candrop"), 11 },
	robin_hood::pair<wstring, int> { wstring(L"collidable"), 111 },
	robin_hood::pair<wstring, int> { wstring(L"isdropped"), 12 },
	robin_hood::pair<wstring, int> { wstring(L"displaytexture"), 13 },
	robin_hood::pair<wstring, int> { wstring(L"slingshotstrength"), 14 },
	robin_hood::pair<wstring, int> { wstring(L"elasticity"), 110 },
	robin_hood::pair<wstring, int> { wstring(L"elasticityfalloff"), 120 },
	robin_hood::pair<wstring, int> { wstring(L"sideimage"), DISPID_Image2 },
	robin_hood::pair<wstring, int> { wstring(L"visible"), 16 },
	robin_hood::pair<wstring, int> { wstring(L"disabled"), 108 },
	robin_hood::pair<wstring, int> { wstring(L"sidevisible"), 109 },
	robin_hood::pair<wstring, int> { wstring(L"uservalue"), DISPID_UserValue },
	robin_hood::pair<wstring, int> { wstring(L"slingshotmaterial"), 426 },
	robin_hood::pair<wstring, int> { wstring(L"slingshotthreshold"), 427 },
	robin_hood::pair<wstring, int> { wstring(L"slingshotanimation"), 112 },
	robin_hood::pair<wstring, int> { wstring(L"flipbookanimation"), 113 },
	robin_hood::pair<wstring, int> { wstring(L"isbottomsolid"), 116 },
	robin_hood::pair<wstring, int> { wstring(L"disablelighting"), 484 },
	robin_hood::pair<wstring, int> { wstring(L"blenddisablelighting"), 494 },
	robin_hood::pair<wstring, int> { wstring(L"blenddisablelightingfrombelow"), 496 },
	robin_hood::pair<wstring, int> { wstring(L"friction"), 114 },
	robin_hood::pair<wstring, int> { wstring(L"scatter"), 115 },
	robin_hood::pair<wstring, int> { wstring(L"reflectionenabled"), 431 },
	robin_hood::pair<wstring, int> { wstring(L"playslingshothit"), 999 },
	robin_hood::pair<wstring, int> { wstring(L"physicsmaterial"), 734 },
	robin_hood::pair<wstring, int> { wstring(L"overwritephysics"), 432 }
};

STDMETHODIMP Surface::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	wstring name = wstring(*rgszNames);
	std::transform(name.begin(), name.end(), name.begin(), tolower);
	const robin_hood::unordered_map<wstring, int>::iterator it = m_nameIDMap.find(name);
	if (it != m_nameIDMap.end()) {
		*rgDispId = it->second;
		return S_OK;
	}
	return DISP_E_UNKNOWNNAME;
}

STDMETHODIMP Surface::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
	switch(dispIdMember) {
		case DISPID_Name:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 833: [propget, id(DISPID_Name), helpstring("property Name")] HRESULT Name([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Name(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 834: [propput, id(DISPID_Name), helpstring("property Name")] HRESULT Name([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Name(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Timer_Enabled:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 835: [propget, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_TimerEnabled(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 836: [propput, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_TimerEnabled(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Timer_Interval:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 837: [propget, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([out, retval] long *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_TimerInterval((long*)&V_I4(pVarResult));
			}

			case 1: {
				// line 838: [propput, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([in] long newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_TimerInterval(V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case 34:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 839: [propget, id(34), helpstring("property HasHitEvent")] HRESULT HasHitEvent([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_HasHitEvent(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 840: [propput, id(34), helpstring("property HasHitEvent")] HRESULT HasHitEvent([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_HasHitEvent(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 33:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 841: [propget, id(33), helpstring("property Threshold")] HRESULT Threshold([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Threshold(&V_R4(pVarResult));
			}

			case 1: {
				// line 842: [propput, id(33), helpstring("property Threshold")] HRESULT Threshold([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Threshold(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Image:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 843: [propget, id(DISPID_Image), helpstring("property Image")] HRESULT Image([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Image(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 844: [propput, id(DISPID_Image), helpstring("property Image")] HRESULT Image([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Image(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case 341:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 845: [propget, id(341), helpstring("property SideMaterial")] HRESULT SideMaterial([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_SideMaterial(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 846: [propput, id(341), helpstring("property SideMaterial")] HRESULT SideMaterial([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_SideMaterial(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case 7:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 847: [propget, id(7), helpstring("property ImageAlignment")] HRESULT ImageAlignment([out, retval] ImageAlignment *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_ImageAlignment((ImageAlignment*)&V_I4(pVarResult));
			}

			case 1: {
				// line 848: [propput, id(7), helpstring("property ImageAlignment")] HRESULT ImageAlignment([in] ImageAlignment newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_ImageAlignment((ImageAlignment)V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case 8:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 849: [propget, id(8), helpstring("property HeightBottom")] HRESULT HeightBottom([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_HeightBottom(&V_R4(pVarResult));
			}

			case 1: {
				// line 850: [propput, id(8), helpstring("property HeightBottom")] HRESULT HeightBottom([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_HeightBottom(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 9:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 851: [propget, id(9), helpstring("property HeightTop")] HRESULT HeightTop([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_HeightTop(&V_R4(pVarResult));
			}

			case 1: {
				// line 852: [propput, id(9), helpstring("property HeightTop")] HRESULT HeightTop([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_HeightTop(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 340:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 853: [propget, id(340), helpstring("property TopMaterial")] HRESULT TopMaterial([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_TopMaterial(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 854: [propput, id(340), helpstring("property TopMaterial")] HRESULT TopMaterial([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_TopMaterial(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case 11:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 855: [propget, id(11), helpstring("property CanDrop")] HRESULT CanDrop([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_CanDrop(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 856: [propput, id(11), helpstring("property CanDrop")] HRESULT CanDrop([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_CanDrop(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 111:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 858: [propget, id(111), helpstring("property Collidable")] HRESULT Collidable([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_Collidable(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 859: [propput, id(111), helpstring("property Collidable")] HRESULT Collidable([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_Collidable(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 12:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 861: [propget, id(12), helpstring("property IsDropped")] HRESULT IsDropped([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_IsDropped(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 862: [propput, id(12), helpstring("property IsDropped")] HRESULT IsDropped([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_IsDropped(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 13:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 863: [propget, id(13), helpstring("property DisplayTexture")] HRESULT DisplayTexture([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_DisplayTexture(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 864: [propput, id(13), helpstring("property DisplayTexture")] HRESULT DisplayTexture([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_DisplayTexture(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 14:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 865: [propget, id(14), helpstring("property SlingshotStrength")] HRESULT SlingshotStrength([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_SlingshotStrength(&V_R4(pVarResult));
			}

			case 1: {
				// line 866: [propput, id(14), helpstring("property SlingshotStrength")] HRESULT SlingshotStrength([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_SlingshotStrength(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 110:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 867: [propget, id(110), helpstring("property Elasticity")] HRESULT Elasticity([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Elasticity(&V_R4(pVarResult));
			}

			case 1: {
				// line 868: [propput, id(110), helpstring("property Elasticity")] HRESULT Elasticity([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Elasticity(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 120:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 869: [propget, id(120), helpstring("property ElasticityFalloff")] HRESULT ElasticityFalloff([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_ElasticityFalloff(&V_R4(pVarResult));
			}

			case 1: {
				// line 870: [propput, id(120), helpstring("property ElasticityFalloff")] HRESULT ElasticityFalloff([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_ElasticityFalloff(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Image2:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 871: [propget, id(DISPID_Image2), helpstring("property SideImage")] HRESULT SideImage([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_SideImage(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 872: [propput, id(DISPID_Image2), helpstring("property SideImage")] HRESULT SideImage([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_SideImage(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case 16:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 873: [propget, id(16), helpstring("property Visible")] HRESULT Visible([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_Visible(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 874: [propput, id(16), helpstring("property Visible")] HRESULT Visible([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_Visible(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 108:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 875: [propget, id(108), helpstring("property Disabled")] HRESULT Disabled([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_Disabled(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 876: [propput, id(108), helpstring("property Disabled")] HRESULT Disabled([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_Disabled(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 109:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 877: [propget, id(109), helpstring("property SideVisible")] HRESULT SideVisible([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_SideVisible(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 878: [propput, id(109), helpstring("property SideVisible")] HRESULT SideVisible([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_SideVisible(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_UserValue:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 879: [propget, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([out, retval] VARIANT* pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				return get_UserValue(pVarResult);
			}

			case 1: {
				// line 880: [propput, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([in] VARIANT* newVal);
				CComVariant var0;
				VariantCopy(&var0, pDispParams->rgvarg);
				return put_UserValue(&var0);
			}

			default:
			break;
		}
		break;

		case 426:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 881: [propget, id(426), helpstring("property SlingshotMaterial")] HRESULT SlingshotMaterial([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_SlingshotMaterial(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 882: [propput, id(426), helpstring("property SlingshotMaterial")] HRESULT SlingshotMaterial([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_SlingshotMaterial(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case 427:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 883: [propget, id(427), helpstring("property SlingshotThreshold")] HRESULT SlingshotThreshold([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_SlingshotThreshold(&V_R4(pVarResult));
			}

			case 1: {
				// line 884: [propput, id(427), helpstring("property SlingshotThreshold")] HRESULT SlingshotThreshold([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_SlingshotThreshold(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 112:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 885: [propget, id(112), helpstring("property SlingshotAnimation")] HRESULT SlingshotAnimation([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_SlingshotAnimation(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 886: [propput, id(112), helpstring("property SlingshotAnimation")] HRESULT SlingshotAnimation([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_SlingshotAnimation(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 113:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 887: [propget, id(113), helpstring("property FlipbookAnimation")] HRESULT FlipbookAnimation([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_FlipbookAnimation(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 888: [propput, id(113), helpstring("property FlipbookAnimation")] HRESULT FlipbookAnimation([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_FlipbookAnimation(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 116:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 889: [propget, id(116), helpstring("property IsBottomSolid")] HRESULT IsBottomSolid([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_IsBottomSolid(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 890: [propput, id(116), helpstring("property IsBottomSolid")] HRESULT IsBottomSolid([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_IsBottomSolid(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 484:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 891: [propget, id(484), helpstring("property DisableLighting")] HRESULT DisableLighting([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_DisableLighting(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 892: [propput, id(484), helpstring("property DisableLighting")] HRESULT DisableLighting([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_DisableLighting(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 494:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 893: [propget, id(494), helpstring("property BlendDisableLighting")] HRESULT BlendDisableLighting([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_BlendDisableLighting(&V_R4(pVarResult));
			}

			case 1: {
				// line 894: [propput, id(494), helpstring("property BlendDisableLighting")] HRESULT BlendDisableLighting([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_BlendDisableLighting(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 496:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 895: [propget, id(496), helpstring("property BlendDisableLightingFromBelow")] HRESULT BlendDisableLightingFromBelow([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_BlendDisableLightingFromBelow(&V_R4(pVarResult));
			}

			case 1: {
				// line 896: [propput, id(496), helpstring("property BlendDisableLightingFromBelow")] HRESULT BlendDisableLightingFromBelow([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_BlendDisableLightingFromBelow(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 114:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 897: [propget, id(114), helpstring("property Friction")] HRESULT Friction([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Friction(&V_R4(pVarResult));
			}

			case 1: {
				// line 898: [propput, id(114), helpstring("property Friction")] HRESULT Friction([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Friction(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 115:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 899: [propget, id(115), helpstring("property Scatter")] HRESULT Scatter([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Scatter(&V_R4(pVarResult));
			}

			case 1: {
				// line 900: [propput, id(115), helpstring("property Scatter")] HRESULT Scatter([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Scatter(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 431:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 901: [propget, id(431), helpstring("property ReflectionEnabled")] HRESULT ReflectionEnabled([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_ReflectionEnabled(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 902: [propput, id(431), helpstring("property ReflectionEnabled")] HRESULT ReflectionEnabled([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_ReflectionEnabled(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 999: {
			// line 903: [id(999), helpstring("method PlaySlingshotHit")] HRESULT PlaySlingshotHit();
			return PlaySlingshotHit();
		}

		case 734:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 904: [propget, id(734), helpstring("property PhysicsMaterial")] HRESULT PhysicsMaterial([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_PhysicsMaterial(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 905: [propput, id(734), helpstring("property PhysicsMaterial")] HRESULT PhysicsMaterial([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_PhysicsMaterial(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case 432:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 906: [propget, id(432), helpstring("property OverwritePhysics")] HRESULT OverwritePhysics([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_OverwritePhysics(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 907: [propput, id(432), helpstring("property OverwritePhysics")] HRESULT OverwritePhysics([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_OverwritePhysics(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		default:
		break;
	}

	return DISP_E_UNKNOWNNAME;
}

robin_hood::unordered_map<int, wstring> Surface::m_idNameMap = {
	robin_hood::pair<int, wstring> { DISPID_GameEvents_Init, wstring(L"_Init") },
	robin_hood::pair<int, wstring> { DISPID_HitEvents_Hit, wstring(L"_Hit") },
	robin_hood::pair<int, wstring> { DISPID_SurfaceEvents_Slingshot, wstring(L"_Slingshot") },
	robin_hood::pair<int, wstring> { DISPID_TimerEvents_Timer, wstring(L"_Timer") }
};

HRESULT Surface::FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) {
	CComPtr<IDispatch> disp;
	g_pplayer->m_ptable->m_pcv->m_pScript->GetScriptDispatch(nullptr, &disp);

	const robin_hood::unordered_map<int, wstring>::iterator it = m_idNameMap.find(dispid);
	if (it != m_idNameMap.end()) {
		wstring name = wstring(m_wzName) + it->second;
		LPOLESTR fnNames = (LPOLESTR)name.c_str();

		DISPID tDispid;
		const HRESULT hr = disp->GetIDsOfNames(IID_NULL, &fnNames, 1, 0, &tDispid);

		if (hr == S_OK) {
			disp->Invoke(tDispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, pdispparams, nullptr, nullptr, nullptr);
		}
	}

	return S_OK;
}

robin_hood::unordered_map<wstring, int> DragPoint::m_nameIDMap = {
	robin_hood::pair<wstring, int> { wstring(L"x"), 1 },
	robin_hood::pair<wstring, int> { wstring(L"y"), 2 },
	robin_hood::pair<wstring, int> { wstring(L"z"), 6 },
	robin_hood::pair<wstring, int> { wstring(L"calcheight"), 377 },
	robin_hood::pair<wstring, int> { wstring(L"smooth"), 3 },
	robin_hood::pair<wstring, int> { wstring(L"isautotexturecoordinate"), 4 },
	robin_hood::pair<wstring, int> { wstring(L"texturecoordinateu"), 5 }
};

STDMETHODIMP DragPoint::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	wstring name = wstring(*rgszNames);
	std::transform(name.begin(), name.end(), name.begin(), tolower);
	const robin_hood::unordered_map<wstring, int>::iterator it = m_nameIDMap.find(name);
	if (it != m_nameIDMap.end()) {
		*rgDispId = it->second;
		return S_OK;
	}
	return DISP_E_UNKNOWNNAME;
}

STDMETHODIMP DragPoint::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
	switch(dispIdMember) {
		case 1:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 932: [propget, id(1), helpstring("property X")] HRESULT X([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_X(&V_R4(pVarResult));
			}

			case 1: {
				// line 933: [propput, id(1), helpstring("property X")] HRESULT X([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_X(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 2:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 934: [propget, id(2), helpstring("property Y")] HRESULT Y([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Y(&V_R4(pVarResult));
			}

			case 1: {
				// line 935: [propput, id(2), helpstring("property Y")] HRESULT Y([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Y(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 6:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 936: [propget, id(6), helpstring("property Z")] HRESULT Z([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Z(&V_R4(pVarResult));
			}

			case 1: {
				// line 937: [propput, id(6), helpstring("property Z")] HRESULT Z([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Z(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 377: {
			// line 938: [propget, id(377), helpstring("property calcHeight")] HRESULT CalcHeight([out, retval] float *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_R4;
			return get_CalcHeight(&V_R4(pVarResult));
		}

		case 3:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 939: [propget, id(3), helpstring("property Smooth")] HRESULT Smooth([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_Smooth(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 940: [propput, id(3), helpstring("property Smooth")] HRESULT Smooth([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_Smooth(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 4:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 941: [propget, id(4), helpstring("property IsAutoTextureCoordinate")] HRESULT IsAutoTextureCoordinate([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_IsAutoTextureCoordinate(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 942: [propput, id(4), helpstring("property IsAutoTextureCoordinate")] HRESULT IsAutoTextureCoordinate([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_IsAutoTextureCoordinate(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 5:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 943: [propget, id(5), helpstring("property TextureCoordinateU")] HRESULT TextureCoordinateU([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_TextureCoordinateU(&V_R4(pVarResult));
			}

			case 1: {
				// line 944: [propput, id(5), helpstring("property TextureCoordinateU")] HRESULT TextureCoordinateU([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_TextureCoordinateU(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		default:
		break;
	}

	return DISP_E_UNKNOWNNAME;
}

robin_hood::unordered_map<wstring, int> Flipper::m_nameIDMap = {
	robin_hood::pair<wstring, int> { wstring(L"baseradius"), 1 },
	robin_hood::pair<wstring, int> { wstring(L"endradius"), 2 },
	robin_hood::pair<wstring, int> { wstring(L"length"), 3 },
	robin_hood::pair<wstring, int> { wstring(L"startangle"), 4 },
	robin_hood::pair<wstring, int> { wstring(L"rotatetoend"), 5 },
	robin_hood::pair<wstring, int> { wstring(L"rotatetostart"), 6 },
	robin_hood::pair<wstring, int> { wstring(L"endangle"), 7 },
	robin_hood::pair<wstring, int> { wstring(L"timerenabled"), DISPID_Timer_Enabled },
	robin_hood::pair<wstring, int> { wstring(L"timerinterval"), DISPID_Timer_Interval },
	robin_hood::pair<wstring, int> { wstring(L"currentangle"), 12 },
	robin_hood::pair<wstring, int> { wstring(L"x"), 13 },
	robin_hood::pair<wstring, int> { wstring(L"y"), 14 },
	robin_hood::pair<wstring, int> { wstring(L"material"), 340 },
	robin_hood::pair<wstring, int> { wstring(L"mass"), DISPID_Flipper_Speed },
	robin_hood::pair<wstring, int> { wstring(L"overridephysics"), DISPID_Flipper_OverridePhysics },
	robin_hood::pair<wstring, int> { wstring(L"rubbermaterial"), 341 },
	robin_hood::pair<wstring, int> { wstring(L"rubberthickness"), 18 },
	robin_hood::pair<wstring, int> { wstring(L"strength"), 19 },
	robin_hood::pair<wstring, int> { wstring(L"visible"), 458 },
	robin_hood::pair<wstring, int> { wstring(L"enabled"), 394 },
	robin_hood::pair<wstring, int> { wstring(L"elasticity"), 110 },
	robin_hood::pair<wstring, int> { wstring(L"scatter"), 115 },
	robin_hood::pair<wstring, int> { wstring(L"return"), 23 },
	robin_hood::pair<wstring, int> { wstring(L"rubberheight"), 24 },
	robin_hood::pair<wstring, int> { wstring(L"rubberwidth"), 25 },
	robin_hood::pair<wstring, int> { wstring(L"friction"), 114 },
	robin_hood::pair<wstring, int> { wstring(L"rampup"), 27 },
	robin_hood::pair<wstring, int> { wstring(L"elasticityfalloff"), 28 },
	robin_hood::pair<wstring, int> { wstring(L"surface"), DISPID_Surface },
	robin_hood::pair<wstring, int> { wstring(L"name"), DISPID_Name },
	robin_hood::pair<wstring, int> { wstring(L"uservalue"), DISPID_UserValue },
	robin_hood::pair<wstring, int> { wstring(L"height"), 107 },
	robin_hood::pair<wstring, int> { wstring(L"eostorque"), 113 },
	robin_hood::pair<wstring, int> { wstring(L"eostorqueangle"), 189 },
	robin_hood::pair<wstring, int> { wstring(L"flipperradiusmin"), 111 },
	robin_hood::pair<wstring, int> { wstring(L"image"), DISPID_Image },
	robin_hood::pair<wstring, int> { wstring(L"reflectionenabled"), 431 }
};

STDMETHODIMP Flipper::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	wstring name = wstring(*rgszNames);
	std::transform(name.begin(), name.end(), name.begin(), tolower);
	const robin_hood::unordered_map<wstring, int>::iterator it = m_nameIDMap.find(name);
	if (it != m_nameIDMap.end()) {
		*rgDispId = it->second;
		return S_OK;
	}
	return DISP_E_UNKNOWNNAME;
}

STDMETHODIMP Flipper::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
	switch(dispIdMember) {
		case 1:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 962: [propget, id(1), helpstring("property BaseRadius")] HRESULT BaseRadius([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_BaseRadius(&V_R4(pVarResult));
			}

			case 1: {
				// line 963: [propput, id(1), helpstring("property BaseRadius")] HRESULT BaseRadius([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_BaseRadius(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 2:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 964: [propget, id(2), helpstring("property EndRadius")] HRESULT EndRadius([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_EndRadius(&V_R4(pVarResult));
			}

			case 1: {
				// line 965: [propput, id(2), helpstring("property EndRadius")] HRESULT EndRadius([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_EndRadius(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 3:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 966: [propget, id(3), helpstring("property Length")] HRESULT Length([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Length(&V_R4(pVarResult));
			}

			case 1: {
				// line 967: [propput, id(3), helpstring("property Length")] HRESULT Length([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Length(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 4:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 968: [propget, id(4), helpstring("property StartAngle")] HRESULT StartAngle([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_StartAngle(&V_R4(pVarResult));
			}

			case 1: {
				// line 969: [propput, id(4), helpstring("property StartAngle")] HRESULT StartAngle([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_StartAngle(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 5: {
			// line 970: [id(5), helpstring("method RotateToEnd")] HRESULT RotateToEnd();
			return RotateToEnd();
		}

		case 6: {
			// line 971: [id(6), helpstring("method RotateToStart")] HRESULT RotateToStart();
			return RotateToStart();
		}

		case 7:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 972: [propget, id(7), helpstring("property EndAngle")] HRESULT EndAngle([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_EndAngle(&V_R4(pVarResult));
			}

			case 1: {
				// line 973: [propput, id(7), helpstring("property EndAngle")] HRESULT EndAngle([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_EndAngle(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Timer_Enabled:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 978: [propget, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_TimerEnabled(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 979: [propput, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_TimerEnabled(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Timer_Interval:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 980: [propget, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([out, retval] long *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_TimerInterval((long*)&V_I4(pVarResult));
			}

			case 1: {
				// line 981: [propput, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([in] long newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_TimerInterval(V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case 12: {
			// line 982: [propget, id(12), nonbrowsable, helpstring("property CurrentAngle")] HRESULT CurrentAngle([out, retval] float *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_R4;
			return get_CurrentAngle(&V_R4(pVarResult));
		}

		case 13:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 983: [propget, id(13), helpstring("property X")] HRESULT X([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_X(&V_R4(pVarResult));
			}

			case 1: {
				// line 984: [propput, id(13), helpstring("property X")] HRESULT X([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_X(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 14:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 985: [propget, id(14), helpstring("property Y")] HRESULT Y([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Y(&V_R4(pVarResult));
			}

			case 1: {
				// line 986: [propput, id(14), helpstring("property Y")] HRESULT Y([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Y(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 340:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 987: [propget, id(340), helpstring("property Material")] HRESULT Material([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Material(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 988: [propput, id(340), helpstring("property Material")] HRESULT Material([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Material(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Flipper_Speed:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 989: [propget, id(DISPID_Flipper_Speed), helpstring("property Mass")] HRESULT Mass([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Mass(&V_R4(pVarResult));
			}

			case 1: {
				// line 990: [propput, id(DISPID_Flipper_Speed), helpstring("property Mass")] HRESULT Mass([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Mass(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Flipper_OverridePhysics:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 991: [propget, id(DISPID_Flipper_OverridePhysics), helpstring("property OverridePhysics")] HRESULT OverridePhysics([out, retval] PhysicsSet *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_OverridePhysics((PhysicsSet*)&V_I4(pVarResult));
			}

			case 1: {
				// line 992: [propput, id(DISPID_Flipper_OverridePhysics), helpstring("property OverridePhysics")] HRESULT OverridePhysics([in] PhysicsSet newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_OverridePhysics((PhysicsSet)V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case 341:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 993: [propget, id(341), helpstring("property RubberMaterial")] HRESULT RubberMaterial([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_RubberMaterial(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 994: [propput, id(341), helpstring("property RubberMaterial")] HRESULT RubberMaterial([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_RubberMaterial(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case 18:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 995: [propget, id(18), helpstring("property RubberThickness")] HRESULT RubberThickness([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_RubberThickness(&V_R4(pVarResult));
			}

			case 1: {
				// line 996: [propput, id(18), helpstring("property RubberThickness")] HRESULT RubberThickness([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_RubberThickness(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 19:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 997: [propget, id(19), helpstring("property Strength")] HRESULT Strength([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Strength(&V_R4(pVarResult));
			}

			case 1: {
				// line 998: [propput, id(19), helpstring("property Strength")] HRESULT Strength([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Strength(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 458:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 999: [propget, id(458), helpstring("property Visible")] HRESULT Visible([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_Visible(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1000: [propput, id(458), helpstring("property Visible")] HRESULT Visible([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_Visible(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 394:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1001: [propget, id(394), helpstring("property Enabled")] HRESULT Enabled([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_Enabled(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1002: [propput, id(394), helpstring("property Enabled")] HRESULT Enabled([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_Enabled(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 110:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1003: [propget, id(110), helpstring("property Elasticity")] HRESULT Elasticity([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Elasticity(&V_R4(pVarResult));
			}

			case 1: {
				// line 1004: [propput, id(110), helpstring("property Elasticity")] HRESULT Elasticity([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Elasticity(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 115:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1005: [propget, id(115), helpstring("property Scatter")] HRESULT Scatter([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Scatter(&V_R4(pVarResult));
			}

			case 1: {
				// line 1006: [propput, id(115), helpstring("property Scatter")] HRESULT Scatter([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Scatter(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 23:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1007: [propget, id(23), helpstring("property Return")] HRESULT Return([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Return(&V_R4(pVarResult));
			}

			case 1: {
				// line 1008: [propput, id(23), helpstring("property Return")] HRESULT Return([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Return(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 24:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1009: [propget, id(24), helpstring("property RubberHeight")] HRESULT RubberHeight([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_RubberHeight(&V_R4(pVarResult));
			}

			case 1: {
				// line 1010: [propput, id(24), helpstring("property RubberHeight")] HRESULT RubberHeight([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_RubberHeight(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 25:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1011: [propget, id(25), helpstring("property RubberWidth")] HRESULT RubberWidth([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_RubberWidth(&V_R4(pVarResult));
			}

			case 1: {
				// line 1012: [propput, id(25), helpstring("property RubberWidth")] HRESULT RubberWidth([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_RubberWidth(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 114:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1013: [propget, id(114), helpstring("property Friction")] HRESULT Friction([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Friction(&V_R4(pVarResult));
			}

			case 1: {
				// line 1014: [propput, id(114), helpstring("property Friction")] HRESULT Friction([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Friction(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 27:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1015: [propget, id(27), helpstring("property RampUp")] HRESULT RampUp([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_RampUp(&V_R4(pVarResult));
			}

			case 1: {
				// line 1016: [propput, id(27), helpstring("property RampUp")] HRESULT RampUp([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_RampUp(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 28:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1017: [propget, id(28), helpstring("property ElasticityFalloff")] HRESULT ElasticityFalloff([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_ElasticityFalloff(&V_R4(pVarResult));
			}

			case 1: {
				// line 1018: [propput, id(28), helpstring("property ElasticityFalloff")] HRESULT ElasticityFalloff([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_ElasticityFalloff(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Surface:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1019: [propget, id(DISPID_Surface), helpstring("property Surface")] HRESULT Surface([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Surface(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 1020: [propput, id(DISPID_Surface), helpstring("property Surface")] HRESULT Surface([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Surface(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Name:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1021: [propget, id(DISPID_Name), helpstring("property Name")] HRESULT Name([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Name(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 1022: [propput, id(DISPID_Name), helpstring("property Name")] HRESULT Name([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Name(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_UserValue:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1023: [propget, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([out, retval] VARIANT* pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				return get_UserValue(pVarResult);
			}

			case 1: {
				// line 1024: [propput, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([in] VARIANT* newVal);
				CComVariant var0;
				VariantCopy(&var0, pDispParams->rgvarg);
				return put_UserValue(&var0);
			}

			default:
			break;
		}
		break;

		case 107:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1025: [propget, id(107), helpstring("property Height")] HRESULT Height([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Height(&V_R4(pVarResult));
			}

			case 1: {
				// line 1026: [propput, id(107), helpstring("property Height")] HRESULT Height([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Height(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 113:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1027: [propget, id(113), helpstring("property EOSTorque")] HRESULT EOSTorque([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_EOSTorque(&V_R4(pVarResult));
			}

			case 1: {
				// line 1028: [propput, id(113), helpstring("property EOSTorque")] HRESULT EOSTorque([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_EOSTorque(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 189:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1029: [propget, id(189), helpstring("property EOSTorqueAngle")] HRESULT EOSTorqueAngle([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_EOSTorqueAngle(&V_R4(pVarResult));
			}

			case 1: {
				// line 1030: [propput, id(189), helpstring("property EOSTorqueAngle")] HRESULT EOSTorqueAngle([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_EOSTorqueAngle(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 111:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1032: [propget, id(111), helpstring("property FlipperRadiusMin")] HRESULT FlipperRadiusMin([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_FlipperRadiusMin(&V_R4(pVarResult));
			}

			case 1: {
				// line 1033: [propput, id(111), helpstring("property FlipperRadiusMin")] HRESULT FlipperRadiusMin([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_FlipperRadiusMin(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Image:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1035: [propget, id(DISPID_Image), helpstring("property Image")] HRESULT Image([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Image(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 1036: [propput, id(DISPID_Image), helpstring("property Image")] HRESULT Image([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Image(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case 431:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1037: [propget, id(431), helpstring("property ReflectionEnabled")] HRESULT ReflectionEnabled([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_ReflectionEnabled(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1038: [propput, id(431), helpstring("property ReflectionEnabled")] HRESULT ReflectionEnabled([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_ReflectionEnabled(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		default:
		break;
	}

	return DISP_E_UNKNOWNNAME;
}

robin_hood::unordered_map<int, wstring> Flipper::m_idNameMap = {
	robin_hood::pair<int, wstring> { DISPID_GameEvents_Init, wstring(L"_Init") },
	robin_hood::pair<int, wstring> { DISPID_HitEvents_Hit, wstring(L"_Hit") },
	robin_hood::pair<int, wstring> { DISPID_TimerEvents_Timer, wstring(L"_Timer") },
	robin_hood::pair<int, wstring> { DISPID_LimitEvents_EOS, wstring(L"_LimitEOS") },
	robin_hood::pair<int, wstring> { DISPID_LimitEvents_BOS, wstring(L"_LimitBOS") },
	robin_hood::pair<int, wstring> { DISPID_FlipperEvents_Collide, wstring(L"_Collide") }
};

HRESULT Flipper::FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) {
	CComPtr<IDispatch> disp;
	g_pplayer->m_ptable->m_pcv->m_pScript->GetScriptDispatch(nullptr, &disp);

	const robin_hood::unordered_map<int, wstring>::iterator it = m_idNameMap.find(dispid);
	if (it != m_idNameMap.end()) {
		wstring name = wstring(m_wzName) + it->second;
		LPOLESTR fnNames = (LPOLESTR)name.c_str();

		DISPID tDispid;
		const HRESULT hr = disp->GetIDsOfNames(IID_NULL, &fnNames, 1, 0, &tDispid);

		if (hr == S_OK) {
			disp->Invoke(tDispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, pdispparams, nullptr, nullptr, nullptr);
		}
	}

	return S_OK;
}

robin_hood::unordered_map<wstring, int> Timer::m_nameIDMap = {
	robin_hood::pair<wstring, int> { wstring(L"enabled"), DISPID_Timer_Enabled },
	robin_hood::pair<wstring, int> { wstring(L"interval"), DISPID_Timer_Interval },
	robin_hood::pair<wstring, int> { wstring(L"name"), DISPID_Name },
	robin_hood::pair<wstring, int> { wstring(L"uservalue"), DISPID_UserValue }
};

STDMETHODIMP Timer::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	wstring name = wstring(*rgszNames);
	std::transform(name.begin(), name.end(), name.begin(), tolower);
	const robin_hood::unordered_map<wstring, int>::iterator it = m_nameIDMap.find(name);
	if (it != m_nameIDMap.end()) {
		*rgDispId = it->second;
		return S_OK;
	}
	return DISP_E_UNKNOWNNAME;
}

STDMETHODIMP Timer::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
	switch(dispIdMember) {
		case DISPID_Timer_Enabled:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1069: [propget, id(DISPID_Timer_Enabled), helpstring("property Enabled")] HRESULT Enabled([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_Enabled(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1070: [propput, id(DISPID_Timer_Enabled), helpstring("property Enabled")] HRESULT Enabled([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_Enabled(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Timer_Interval:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1071: [propget, id(DISPID_Timer_Interval), helpstring("property Interval")] HRESULT Interval([out, retval] long *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_Interval((long*)&V_I4(pVarResult));
			}

			case 1: {
				// line 1072: [propput, id(DISPID_Timer_Interval), helpstring("property Interval")] HRESULT Interval([in] long newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_Interval(V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Name:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1073: [propget, id(DISPID_Name), helpstring("property Name")] HRESULT Name([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Name(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 1074: [propput, id(DISPID_Name), helpstring("property Name")] HRESULT Name([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Name(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_UserValue:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1075: [propget, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([out, retval] VARIANT* pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				return get_UserValue(pVarResult);
			}

			case 1: {
				// line 1076: [propput, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([in] VARIANT* newVal);
				CComVariant var0;
				VariantCopy(&var0, pDispParams->rgvarg);
				return put_UserValue(&var0);
			}

			default:
			break;
		}
		break;

		default:
		break;
	}

	return DISP_E_UNKNOWNNAME;
}

robin_hood::unordered_map<int, wstring> Timer::m_idNameMap = {
	robin_hood::pair<int, wstring> { DISPID_TimerEvents_Timer, wstring(L"_Timer") },
	robin_hood::pair<int, wstring> { DISPID_GameEvents_Init, wstring(L"_Init") }
};

HRESULT Timer::FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) {
	CComPtr<IDispatch> disp;
	g_pplayer->m_ptable->m_pcv->m_pScript->GetScriptDispatch(nullptr, &disp);

	const robin_hood::unordered_map<int, wstring>::iterator it = m_idNameMap.find(dispid);
	if (it != m_idNameMap.end()) {
		wstring name = wstring(m_wzName) + it->second;
		LPOLESTR fnNames = (LPOLESTR)name.c_str();

		DISPID tDispid;
		const HRESULT hr = disp->GetIDsOfNames(IID_NULL, &fnNames, 1, 0, &tDispid);

		if (hr == S_OK) {
			disp->Invoke(tDispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, pdispparams, nullptr, nullptr, nullptr);
		}
	}

	return S_OK;
}

robin_hood::unordered_map<wstring, int> Plunger::m_nameIDMap = {
	robin_hood::pair<wstring, int> { wstring(L"pullback"), 1 },
	robin_hood::pair<wstring, int> { wstring(L"fire"), 2 },
	robin_hood::pair<wstring, int> { wstring(L"pullspeed"), 3 },
	robin_hood::pair<wstring, int> { wstring(L"firespeed"), 4 },
	robin_hood::pair<wstring, int> { wstring(L"createball"), 5 },
	robin_hood::pair<wstring, int> { wstring(L"position"), 6 },
	robin_hood::pair<wstring, int> { wstring(L"timerenabled"), DISPID_Timer_Enabled },
	robin_hood::pair<wstring, int> { wstring(L"timerinterval"), DISPID_Timer_Interval },
	robin_hood::pair<wstring, int> { wstring(L"x"), DISPID_X },
	robin_hood::pair<wstring, int> { wstring(L"y"), DISPID_Y },
	robin_hood::pair<wstring, int> { wstring(L"width"), DISPID_Width },
	robin_hood::pair<wstring, int> { wstring(L"zadjust"), DISPID_ZAdjust },
	robin_hood::pair<wstring, int> { wstring(L"surface"), DISPID_Surface },
	robin_hood::pair<wstring, int> { wstring(L"name"), DISPID_Name },
	robin_hood::pair<wstring, int> { wstring(L"uservalue"), DISPID_UserValue },
	robin_hood::pair<wstring, int> { wstring(L"type"), 390 },
	robin_hood::pair<wstring, int> { wstring(L"material"), 340 },
	robin_hood::pair<wstring, int> { wstring(L"image"), DISPID_Image },
	robin_hood::pair<wstring, int> { wstring(L"animframes"), DISPID_PluFrames },
	robin_hood::pair<wstring, int> { wstring(L"tipshape"), DISPID_TipShape },
	robin_hood::pair<wstring, int> { wstring(L"roddiam"), DISPID_RodDiam },
	robin_hood::pair<wstring, int> { wstring(L"ringgap"), DISPID_RingGap },
	robin_hood::pair<wstring, int> { wstring(L"ringdiam"), DISPID_RingDiam },
	robin_hood::pair<wstring, int> { wstring(L"ringwidth"), DISPID_RingThickness },
	robin_hood::pair<wstring, int> { wstring(L"springdiam"), DISPID_SpringDiam },
	robin_hood::pair<wstring, int> { wstring(L"springgauge"), DISPID_SpringGauge },
	robin_hood::pair<wstring, int> { wstring(L"springloops"), DISPID_SpringLoops },
	robin_hood::pair<wstring, int> { wstring(L"springendloops"), DISPID_SpringEndLoops },
	robin_hood::pair<wstring, int> { wstring(L"mechplunger"), 110 },
	robin_hood::pair<wstring, int> { wstring(L"autoplunger"), 116 },
	robin_hood::pair<wstring, int> { wstring(L"visible"), 117 },
	robin_hood::pair<wstring, int> { wstring(L"mechstrength"), 111 },
	robin_hood::pair<wstring, int> { wstring(L"parkposition"), 112 },
	robin_hood::pair<wstring, int> { wstring(L"stroke"), 113 },
	robin_hood::pair<wstring, int> { wstring(L"scattervelocity"), 114 },
	robin_hood::pair<wstring, int> { wstring(L"momentumxfer"), 118 },
	robin_hood::pair<wstring, int> { wstring(L"reflectionenabled"), 431 }
};

STDMETHODIMP Plunger::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	wstring name = wstring(*rgszNames);
	std::transform(name.begin(), name.end(), name.begin(), tolower);
	const robin_hood::unordered_map<wstring, int>::iterator it = m_nameIDMap.find(name);
	if (it != m_nameIDMap.end()) {
		*rgDispId = it->second;
		return S_OK;
	}
	return DISP_E_UNKNOWNNAME;
}

STDMETHODIMP Plunger::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
	switch(dispIdMember) {
		case 1: {
			// line 1103: [id(1), helpstring("method PullBack")] HRESULT PullBack();
			return PullBack();
		}

		case 2: {
			// line 1104: [id(2), helpstring("method Fire")] HRESULT Fire();
			return Fire();
		}

		case 3:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1105: [propget, id(3), helpstring("property PullSpeed")] HRESULT PullSpeed([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_PullSpeed(&V_R4(pVarResult));
			}

			case 1: {
				// line 1106: [propput, id(3), helpstring("property PullSpeed")] HRESULT PullSpeed([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_PullSpeed(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 4:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1107: [propget, id(4), helpstring("property FireSpeed")] HRESULT FireSpeed([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_FireSpeed(&V_R4(pVarResult));
			}

			case 1: {
				// line 1108: [propput, id(4), helpstring("property FireSpeed")] HRESULT FireSpeed([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_FireSpeed(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 5: {
			// line 1109: [id(5), helpstring("method CreateBall")] HRESULT CreateBall([out, retval] IBall **Ball);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_DISPATCH;
			return CreateBall((IBall**)&V_DISPATCH(pVarResult));
		}

		case 6: {
			// line 1110: [id(6), helpstring("method Position")] HRESULT Position([out, retval] float *pVal); //added for mechanical plunger position, also supports normal/digital one by now[id(7), helpstring("method PullBackandRetract")] HRESULT PullBackandRetract(); //added for manual non-mech plunger[id(216), helpstring("method MotionDevice")] HRESULT MotionDevice([out, retval] int *pVal); //added for uShockType[propget, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([out, retval] VARIANT_BOOL *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_R4;
			return Position(&V_R4(pVarResult));
		}

		case DISPID_Timer_Enabled: {
			// line 1114: [propput, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([in] VARIANT_BOOL newVal);
			CComVariant var0;
			VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
			return put_TimerEnabled(V_BOOL(&var0));
		}

		case DISPID_Timer_Interval:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1115: [propget, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([out, retval] long *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_TimerInterval((long*)&V_I4(pVarResult));
			}

			case 1: {
				// line 1116: [propput, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([in] long newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_TimerInterval(V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_X:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1117: [propget, id(DISPID_X), helpstring("property X")] HRESULT X([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_X(&V_R4(pVarResult));
			}

			case 1: {
				// line 1118: [propput, id(DISPID_X), helpstring("property X")] HRESULT X([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_X(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Y:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1119: [propget, id(DISPID_Y), helpstring("property Y")] HRESULT Y([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Y(&V_R4(pVarResult));
			}

			case 1: {
				// line 1120: [propput, id(DISPID_Y), helpstring("property Y")] HRESULT Y([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Y(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Width:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1121: [propget, id(DISPID_Width), helpstring("property Width")] HRESULT Width([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Width(&V_R4(pVarResult));
			}

			case 1: {
				// line 1122: [propput, id(DISPID_Width), helpstring("property Width")] HRESULT Width([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Width(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_ZAdjust:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1123: [propget, id(DISPID_ZAdjust), helpstring("property ZAdjust")] HRESULT ZAdjust([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_ZAdjust(&V_R4(pVarResult));
			}

			case 1: {
				// line 1124: [propput, id(DISPID_ZAdjust), helpstring("property ZAdjust")] HRESULT ZAdjust([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_ZAdjust(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Surface:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1125: [propget, id(DISPID_Surface), helpstring("property Surface")] HRESULT Surface([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Surface(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 1126: [propput, id(DISPID_Surface), helpstring("property Surface")] HRESULT Surface([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Surface(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Name:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1127: [propget, id(DISPID_Name), helpstring("property Name")] HRESULT Name([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Name(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 1128: [propput, id(DISPID_Name), helpstring("property Name")] HRESULT Name([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Name(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_UserValue:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1129: [propget, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([out, retval] VARIANT* pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				return get_UserValue(pVarResult);
			}

			case 1: {
				// line 1130: [propput, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([in] VARIANT* newVal);
				CComVariant var0;
				VariantCopy(&var0, pDispParams->rgvarg);
				return put_UserValue(&var0);
			}

			default:
			break;
		}
		break;

		case 390:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1131: [propget, id(390), helpstring("property Type")] HRESULT Type([out, retval] PlungerType *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_Type((PlungerType*)&V_I4(pVarResult));
			}

			case 1: {
				// line 1132: [propput, id(390), helpstring("property Type")] HRESULT Type([in] PlungerType newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_Type((PlungerType)V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case 340:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1133: [propget, id(340), helpstring("property Material")] HRESULT Material([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Material(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 1134: [propput, id(340), helpstring("property Material")] HRESULT Material([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Material(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Image:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1135: [propget, id(DISPID_Image), helpstring("property Image")] HRESULT Image([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Image(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 1136: [propput, id(DISPID_Image), helpstring("property Image")] HRESULT Image([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Image(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_PluFrames:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1137: [propget, id(DISPID_PluFrames), helpstring("property AnimFrames")] HRESULT AnimFrames([out, retval] int *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_AnimFrames(&V_I4(pVarResult));
			}

			case 1: {
				// line 1138: [propput, id(DISPID_PluFrames), helpstring("property AnimFrames")] HRESULT AnimFrames([in] int newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_AnimFrames(V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_TipShape:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1139: [propget, id(DISPID_TipShape), helpstring("property Shape")] HRESULT TipShape([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_TipShape(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 1140: [propput, id(DISPID_TipShape), helpstring("property Shape")] HRESULT TipShape([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_TipShape(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_RodDiam:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1141: [propget, id(DISPID_RodDiam), helpstring("property RodDiam")] HRESULT RodDiam([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_RodDiam(&V_R4(pVarResult));
			}

			case 1: {
				// line 1142: [propput, id(DISPID_RodDiam), helpstring("property RodDiam")] HRESULT RodDiam([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_RodDiam(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_RingGap:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1143: [propget, id(DISPID_RingGap), helpstring("property RingGap")] HRESULT RingGap([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_RingGap(&V_R4(pVarResult));
			}

			case 1: {
				// line 1144: [propput, id(DISPID_RingGap), helpstring("property RingGap")] HRESULT RingGap([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_RingGap(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_RingDiam:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1145: [propget, id(DISPID_RingDiam), helpstring("property RingDiam")] HRESULT RingDiam([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_RingDiam(&V_R4(pVarResult));
			}

			case 1: {
				// line 1146: [propput, id(DISPID_RingDiam), helpstring("property RingDiam")] HRESULT RingDiam([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_RingDiam(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_RingThickness:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1147: [propget, id(DISPID_RingThickness), helpstring("property RingWidth")] HRESULT RingWidth([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_RingWidth(&V_R4(pVarResult));
			}

			case 1: {
				// line 1148: [propput, id(DISPID_RingThickness), helpstring("property RingWidth")] HRESULT RingWidth([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_RingWidth(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_SpringDiam:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1149: [propget, id(DISPID_SpringDiam), helpstring("property SpringDiam")] HRESULT SpringDiam([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_SpringDiam(&V_R4(pVarResult));
			}

			case 1: {
				// line 1150: [propput, id(DISPID_SpringDiam), helpstring("property SpringDiam")] HRESULT SpringDiam([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_SpringDiam(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_SpringGauge:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1151: [propget, id(DISPID_SpringGauge), helpstring("property SpringGauge")] HRESULT SpringGauge([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_SpringGauge(&V_R4(pVarResult));
			}

			case 1: {
				// line 1152: [propput, id(DISPID_SpringGauge), helpstring("property SpringGauge")] HRESULT SpringGauge([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_SpringGauge(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_SpringLoops:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1153: [propget, id(DISPID_SpringLoops), helpstring("property SpringLoops")] HRESULT SpringLoops([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_SpringLoops(&V_R4(pVarResult));
			}

			case 1: {
				// line 1154: [propput, id(DISPID_SpringLoops), helpstring("property SpringLoops")] HRESULT SpringLoops([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_SpringLoops(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_SpringEndLoops:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1155: [propget, id(DISPID_SpringEndLoops), helpstring("property SpringEndLoops")] HRESULT SpringEndLoops([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_SpringEndLoops(&V_R4(pVarResult));
			}

			case 1: {
				// line 1156: [propput, id(DISPID_SpringEndLoops), helpstring("property SpringEndLoops")] HRESULT SpringEndLoops([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_SpringEndLoops(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 110:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1158: [propget, id(110), helpstring("property MechPlunger")] HRESULT MechPlunger([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_MechPlunger(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1159: [propput, id(110), helpstring("property MechPlunger")] HRESULT MechPlunger([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_MechPlunger(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 116:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1160: [propget, id(116), helpstring("property AutoPlunger")] HRESULT AutoPlunger([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_AutoPlunger(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1161: [propput, id(116), helpstring("property AutoPlunger")] HRESULT AutoPlunger([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_AutoPlunger(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 117:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1162: [propget, id(117), helpstring("property Visible")] HRESULT Visible([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_Visible(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1163: [propput, id(117), helpstring("property Visible")] HRESULT Visible([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_Visible(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 111:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1164: [propget, id(111), helpstring("property MechStrength")] HRESULT MechStrength([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_MechStrength(&V_R4(pVarResult));
			}

			case 1: {
				// line 1165: [propput, id(111), helpstring("property MechStrength")] HRESULT MechStrength([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_MechStrength(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 112:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1166: [propget, id(112), helpstring("property ParkPosition")] HRESULT ParkPosition([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_ParkPosition(&V_R4(pVarResult));
			}

			case 1: {
				// line 1167: [propput, id(112), helpstring("property ParkPosition")] HRESULT ParkPosition([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_ParkPosition(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 113:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1168: [propget, id(113), helpstring("property Stroke")] HRESULT Stroke([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Stroke(&V_R4(pVarResult));
			}

			case 1: {
				// line 1169: [propput, id(113), helpstring("property Stroke")] HRESULT Stroke([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Stroke(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 114:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1170: [propget, id(114), helpstring("property ScatterVelocity")] HRESULT ScatterVelocity([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_ScatterVelocity(&V_R4(pVarResult));
			}

			case 1: {
				// line 1171: [propput, id(114), helpstring("property ScatterVelocity")] HRESULT ScatterVelocity([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_ScatterVelocity(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 118:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1172: [propget, id(118), helpstring("property MomentumXfer")] HRESULT MomentumXfer([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_MomentumXfer(&V_R4(pVarResult));
			}

			case 1: {
				// line 1173: [propput, id(118), helpstring("property MomentumXfer")] HRESULT MomentumXfer([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_MomentumXfer(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 431:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1174: [propget, id(431), helpstring("property ReflectionEnabled")] HRESULT ReflectionEnabled([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_ReflectionEnabled(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1175: [propput, id(431), helpstring("property ReflectionEnabled")] HRESULT ReflectionEnabled([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_ReflectionEnabled(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		default:
		break;
	}

	return DISP_E_UNKNOWNNAME;
}

robin_hood::unordered_map<int, wstring> Plunger::m_idNameMap = {
	robin_hood::pair<int, wstring> { DISPID_GameEvents_Init, wstring(L"_Init") },
	robin_hood::pair<int, wstring> { DISPID_TimerEvents_Timer, wstring(L"_Timer") },
	robin_hood::pair<int, wstring> { DISPID_LimitEvents_EOS, wstring(L"_LimitEOS") },
	robin_hood::pair<int, wstring> { DISPID_LimitEvents_BOS, wstring(L"_LimitBOS") }
};

HRESULT Plunger::FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) {
	CComPtr<IDispatch> disp;
	g_pplayer->m_ptable->m_pcv->m_pScript->GetScriptDispatch(nullptr, &disp);

	const robin_hood::unordered_map<int, wstring>::iterator it = m_idNameMap.find(dispid);
	if (it != m_idNameMap.end()) {
		wstring name = wstring(m_wzName) + it->second;
		LPOLESTR fnNames = (LPOLESTR)name.c_str();

		DISPID tDispid;
		const HRESULT hr = disp->GetIDsOfNames(IID_NULL, &fnNames, 1, 0, &tDispid);

		if (hr == S_OK) {
			disp->Invoke(tDispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, pdispparams, nullptr, nullptr, nullptr);
		}
	}

	return S_OK;
}

robin_hood::unordered_map<wstring, int> Textbox::m_nameIDMap = {
	robin_hood::pair<wstring, int> { wstring(L"backcolor"), DISPID_Textbox_BackColor },
	robin_hood::pair<wstring, int> { wstring(L"fontcolor"), DISPID_Textbox_FontColor },
	robin_hood::pair<wstring, int> { wstring(L"text"), 3 },
	robin_hood::pair<wstring, int> { wstring(L"font"), DISPID_Textbox_Font },
	robin_hood::pair<wstring, int> { wstring(L"width"), 60002 },
	robin_hood::pair<wstring, int> { wstring(L"height"), 60003 },
	robin_hood::pair<wstring, int> { wstring(L"alignment"), 11 },
	robin_hood::pair<wstring, int> { wstring(L"istransparent"), 12 },
	robin_hood::pair<wstring, int> { wstring(L"dmd"), 555 },
	robin_hood::pair<wstring, int> { wstring(L"timerenabled"), DISPID_Timer_Enabled },
	robin_hood::pair<wstring, int> { wstring(L"timerinterval"), DISPID_Timer_Interval },
	robin_hood::pair<wstring, int> { wstring(L"x"), 60000 },
	robin_hood::pair<wstring, int> { wstring(L"y"), 60001 },
	robin_hood::pair<wstring, int> { wstring(L"intensityscale"), 587 },
	robin_hood::pair<wstring, int> { wstring(L"name"), DISPID_Name },
	robin_hood::pair<wstring, int> { wstring(L"uservalue"), DISPID_UserValue },
	robin_hood::pair<wstring, int> { wstring(L"visible"), 616 }
};

STDMETHODIMP Textbox::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	wstring name = wstring(*rgszNames);
	std::transform(name.begin(), name.end(), name.begin(), tolower);
	const robin_hood::unordered_map<wstring, int>::iterator it = m_nameIDMap.find(name);
	if (it != m_nameIDMap.end()) {
		*rgDispId = it->second;
		return S_OK;
	}
	return DISP_E_UNKNOWNNAME;
}

STDMETHODIMP Textbox::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
	switch(dispIdMember) {
		case DISPID_Textbox_BackColor:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1204: [propget, id(DISPID_Textbox_BackColor), helpstring("property BackColor")] HRESULT BackColor([out, retval] OLE_COLOR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_UI4;
				return get_BackColor(&V_UI4(pVarResult));
			}

			case 1: {
				// line 1205: [propput, id(DISPID_Textbox_BackColor), helpstring("property BackColor")] HRESULT BackColor([in] OLE_COLOR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_UI4);
				return put_BackColor((OLE_COLOR)V_UI4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Textbox_FontColor:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1206: [propget, id(DISPID_Textbox_FontColor), helpstring("property FontColor")] HRESULT FontColor([out, retval] OLE_COLOR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_UI4;
				return get_FontColor(&V_UI4(pVarResult));
			}

			case 1: {
				// line 1207: [propput, id(DISPID_Textbox_FontColor), helpstring("property FontColor")] HRESULT FontColor([in] OLE_COLOR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_UI4);
				return put_FontColor((OLE_COLOR)V_UI4(&var0));
			}

			default:
			break;
		}
		break;

		case 3:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1208: [propget, id(3), helpstring("property Text")] HRESULT Text([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Text(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 1209: [propput, id(3), helpstring("property Text")] HRESULT Text([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Text(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Textbox_Font:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1210: [propget, id(DISPID_Textbox_Font), helpstring("property Font")] HRESULT Font([out, retval] IFontDisp **pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_DISPATCH;
				return get_Font((IFontDisp**)&V_DISPATCH(pVarResult));
			}

			case 1: {
				// line 1211: [propputref, id(DISPID_Textbox_Font), helpstring("property Font")] HRESULT Font([in] IFontDisp *newVal);
				return putref_Font((IFontDisp*)pDispParams->rgvarg);
			}

			default:
			break;
		}
		break;

		case 60002:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1212: [propget, id(60002), helpstring("property Width")] HRESULT Width([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Width(&V_R4(pVarResult));
			}

			case 1: {
				// line 1213: [propput, id(60002), helpstring("property Width")] HRESULT Width([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Width(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 60003:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1214: [propget, id(60003), helpstring("property Height")] HRESULT Height([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Height(&V_R4(pVarResult));
			}

			case 1: {
				// line 1215: [propput, id(60003), helpstring("property Height")] HRESULT Height([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Height(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 11:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1216: [propget, id(11), helpstring("property Alignment")] HRESULT Alignment([out, retval] TextAlignment *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_Alignment((TextAlignment*)&V_I4(pVarResult));
			}

			case 1: {
				// line 1217: [propput, id(11), helpstring("property Alignment")] HRESULT Alignment([in] TextAlignment newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_Alignment((TextAlignment)V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case 12:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1218: [propget, id(12), helpstring("property IsTransparent")] HRESULT IsTransparent([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_IsTransparent(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1219: [propput, id(12), helpstring("property IsTransparent")] HRESULT IsTransparent([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_IsTransparent(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 555:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1220: [propget, id(555), helpstring("property DMD")] HRESULT DMD([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_DMD(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1221: [propput, id(555), helpstring("property DMD")] HRESULT DMD([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_DMD(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Timer_Enabled:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1222: [propget, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_TimerEnabled(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1223: [propput, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_TimerEnabled(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Timer_Interval:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1224: [propget, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([out, retval] long *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_TimerInterval((long*)&V_I4(pVarResult));
			}

			case 1: {
				// line 1225: [propput, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([in] long newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_TimerInterval(V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case 60000:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1226: [propget, id(60000), helpstring("property X")] HRESULT X([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_X(&V_R4(pVarResult));
			}

			case 1: {
				// line 1227: [propput, id(60000), helpstring("property X")] HRESULT X([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_X(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 60001:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1228: [propget, id(60001), helpstring("property Y")] HRESULT Y([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Y(&V_R4(pVarResult));
			}

			case 1: {
				// line 1229: [propput, id(60001), helpstring("property Y")] HRESULT Y([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Y(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 587:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1230: [propget, id(587), helpstring("property IntensityScale")] HRESULT IntensityScale([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_IntensityScale(&V_R4(pVarResult));
			}

			case 1: {
				// line 1231: [propput, id(587), helpstring("property IntensityScale")] HRESULT IntensityScale([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_IntensityScale(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Name:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1232: [propget, id(DISPID_Name), helpstring("property Name")] HRESULT Name([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Name(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 1233: [propput, id(DISPID_Name), helpstring("property Name")] HRESULT Name([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Name(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_UserValue:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1234: [propget, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([out, retval] VARIANT* pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				return get_UserValue(pVarResult);
			}

			case 1: {
				// line 1235: [propput, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([in] VARIANT* newVal);
				CComVariant var0;
				VariantCopy(&var0, pDispParams->rgvarg);
				return put_UserValue(&var0);
			}

			default:
			break;
		}
		break;

		case 616:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1236: [propget, id(616), helpstring("property Visible")] HRESULT Visible([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_Visible(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1237: [propput, id(616), helpstring("property Visible")] HRESULT Visible([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_Visible(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		default:
		break;
	}

	return DISP_E_UNKNOWNNAME;
}

robin_hood::unordered_map<int, wstring> Textbox::m_idNameMap = {
	robin_hood::pair<int, wstring> { DISPID_GameEvents_Init, wstring(L"_Init") },
	robin_hood::pair<int, wstring> { DISPID_TimerEvents_Timer, wstring(L"_Timer") }
};

HRESULT Textbox::FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) {
	CComPtr<IDispatch> disp;
	g_pplayer->m_ptable->m_pcv->m_pScript->GetScriptDispatch(nullptr, &disp);

	const robin_hood::unordered_map<int, wstring>::iterator it = m_idNameMap.find(dispid);
	if (it != m_idNameMap.end()) {
		wstring name = wstring(m_wzName) + it->second;
		LPOLESTR fnNames = (LPOLESTR)name.c_str();

		DISPID tDispid;
		const HRESULT hr = disp->GetIDsOfNames(IID_NULL, &fnNames, 1, 0, &tDispid);

		if (hr == S_OK) {
			disp->Invoke(tDispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, pdispparams, nullptr, nullptr, nullptr);
		}
	}

	return S_OK;
}

robin_hood::unordered_map<wstring, int> Bumper::m_nameIDMap = {
	robin_hood::pair<wstring, int> { wstring(L"radius"), 8 },
	robin_hood::pair<wstring, int> { wstring(L"x"), DISPID_X },
	robin_hood::pair<wstring, int> { wstring(L"y"), DISPID_Y },
	robin_hood::pair<wstring, int> { wstring(L"basematerial"), 341 },
	robin_hood::pair<wstring, int> { wstring(L"skirtmaterial"), 426 },
	robin_hood::pair<wstring, int> { wstring(L"uservalue"), DISPID_UserValue },
	robin_hood::pair<wstring, int> { wstring(L"surface"), DISPID_Surface },
	robin_hood::pair<wstring, int> { wstring(L"force"), 2 },
	robin_hood::pair<wstring, int> { wstring(L"threshold"), 33 },
	robin_hood::pair<wstring, int> { wstring(L"timerenabled"), DISPID_Timer_Enabled },
	robin_hood::pair<wstring, int> { wstring(L"timerinterval"), DISPID_Timer_Interval },
	robin_hood::pair<wstring, int> { wstring(L"capmaterial"), 340 },
	robin_hood::pair<wstring, int> { wstring(L"ringmaterial"), 734 },
	robin_hood::pair<wstring, int> { wstring(L"heightscale"), 24 },
	robin_hood::pair<wstring, int> { wstring(L"orientation"), 25 },
	robin_hood::pair<wstring, int> { wstring(L"ringspeed"), 26 },
	robin_hood::pair<wstring, int> { wstring(L"ringdropoffset"), 27 },
	robin_hood::pair<wstring, int> { wstring(L"name"), DISPID_Name },
	robin_hood::pair<wstring, int> { wstring(L"hashitevent"), 34 },
	robin_hood::pair<wstring, int> { wstring(L"capvisible"), 109 },
	robin_hood::pair<wstring, int> { wstring(L"basevisible"), 110 },
	robin_hood::pair<wstring, int> { wstring(L"ringvisible"), 735 },
	robin_hood::pair<wstring, int> { wstring(L"skirtvisible"), 736 },
	robin_hood::pair<wstring, int> { wstring(L"collidable"), 111 },
	robin_hood::pair<wstring, int> { wstring(L"reflectionenabled"), 431 },
	robin_hood::pair<wstring, int> { wstring(L"scatter"), 115 },
	robin_hood::pair<wstring, int> { wstring(L"enableskirtanimation"), 822 },
	robin_hood::pair<wstring, int> { wstring(L"playhit"), 999 }
};

STDMETHODIMP Bumper::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	wstring name = wstring(*rgszNames);
	std::transform(name.begin(), name.end(), name.begin(), tolower);
	const robin_hood::unordered_map<wstring, int>::iterator it = m_nameIDMap.find(name);
	if (it != m_nameIDMap.end()) {
		*rgDispId = it->second;
		return S_OK;
	}
	return DISP_E_UNKNOWNNAME;
}

STDMETHODIMP Bumper::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
	switch(dispIdMember) {
		case 8:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1264: [propget, id(8), helpstring("property Radius")] HRESULT Radius([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Radius(&V_R4(pVarResult));
			}

			case 1: {
				// line 1265: [propput, id(8), helpstring("property Radius")] HRESULT Radius([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Radius(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_X:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1266: [propget, id(DISPID_X), helpstring("property X")] HRESULT X([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_X(&V_R4(pVarResult));
			}

			case 1: {
				// line 1267: [propput, id(DISPID_X), helpstring("property X")] HRESULT X([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_X(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Y:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1268: [propget, id(DISPID_Y), helpstring("property Y")] HRESULT Y([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Y(&V_R4(pVarResult));
			}

			case 1: {
				// line 1269: [propput, id(DISPID_Y), helpstring("property Y")] HRESULT Y([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Y(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 341:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1270: [propget, id(341), helpstring("property BaseMaterial")] HRESULT BaseMaterial([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_BaseMaterial(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 1271: [propput, id(341), helpstring("property BaseMaterial")] HRESULT BaseMaterial([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_BaseMaterial(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case 426:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1272: [propget, id(426), helpstring("property SkirtMaterial")] HRESULT SkirtMaterial([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_SkirtMaterial(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 1273: [propput, id(426), helpstring("property SkirtMaterial")] HRESULT SkirtMaterial([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_SkirtMaterial(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_UserValue:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1274: [propget, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([out, retval] VARIANT* pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				return get_UserValue(pVarResult);
			}

			case 1: {
				// line 1275: [propput, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([in] VARIANT* newVal);
				CComVariant var0;
				VariantCopy(&var0, pDispParams->rgvarg);
				return put_UserValue(&var0);
			}

			default:
			break;
		}
		break;

		case DISPID_Surface:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1276: [propget, id(DISPID_Surface), helpstring("property Surface")] HRESULT Surface([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Surface(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 1277: [propput, id(DISPID_Surface), helpstring("property Surface")] HRESULT Surface([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Surface(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case 2:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1278: [propget, id(2), helpstring("property Force")] HRESULT Force([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Force(&V_R4(pVarResult));
			}

			case 1: {
				// line 1279: [propput, id(2), helpstring("property Force")] HRESULT Force([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Force(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 33:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1280: [propget, id(33), helpstring("property Threshold")] HRESULT Threshold([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Threshold(&V_R4(pVarResult));
			}

			case 1: {
				// line 1281: [propput, id(33), helpstring("property Threshold")] HRESULT Threshold([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Threshold(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Timer_Enabled:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1282: [propget, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_TimerEnabled(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1283: [propput, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_TimerEnabled(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Timer_Interval:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1284: [propget, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([out, retval] long *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_TimerInterval((long*)&V_I4(pVarResult));
			}

			case 1: {
				// line 1285: [propput, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([in] long newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_TimerInterval(V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case 340:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1286: [propget, id(340), helpstring("property CapMaterial")] HRESULT CapMaterial([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_CapMaterial(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 1287: [propput, id(340), helpstring("property CapMaterial")] HRESULT CapMaterial([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_CapMaterial(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case 734:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1288: [propget, id(734), helpstring("property RingMaterial")] HRESULT RingMaterial([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_RingMaterial(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 1289: [propput, id(734), helpstring("property RingMaterial")] HRESULT RingMaterial([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_RingMaterial(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case 24:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1290: [propget, id(24), helpstring("property HeightScale")] HRESULT HeightScale([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_HeightScale(&V_R4(pVarResult));
			}

			case 1: {
				// line 1291: [propput, id(24), helpstring("property HeightScale")] HRESULT HeightScale([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_HeightScale(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 25:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1292: [propget, id(25), helpstring("property Orientation")] HRESULT Orientation([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Orientation(&V_R4(pVarResult));
			}

			case 1: {
				// line 1293: [propput, id(25), helpstring("property Orientation")] HRESULT Orientation([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Orientation(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 26:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1294: [propget, id(26), helpstring("property RingSpeed")] HRESULT RingSpeed([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_RingSpeed(&V_R4(pVarResult));
			}

			case 1: {
				// line 1295: [propput, id(26), helpstring("property RingSpeed")] HRESULT RingSpeed([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_RingSpeed(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 27:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1296: [propget, id(27), helpstring("property RingDropOffset")] HRESULT RingDropOffset([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_RingDropOffset(&V_R4(pVarResult));
			}

			case 1: {
				// line 1297: [propput, id(27), helpstring("property RingDropOffset")] HRESULT RingDropOffset([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_RingDropOffset(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Name:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1298: [propget, id(DISPID_Name), helpstring("property Name")] HRESULT Name([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Name(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 1299: [propput, id(DISPID_Name), helpstring("property Name")] HRESULT Name([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Name(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case 34:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1300: [propget, id(34), helpstring("property HitEvent")] HRESULT HasHitEvent([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_HasHitEvent(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1301: [propput, id(34), helpstring("property HitEvent")] HRESULT HasHitEvent([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_HasHitEvent(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 109:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1302: [propget, id(109), helpstring("property CapVisible")] HRESULT CapVisible([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_CapVisible(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1303: [propput, id(109), helpstring("property CapVisible")] HRESULT CapVisible([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_CapVisible(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 110:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1304: [propget, id(110), helpstring("property BaseVisible")] HRESULT BaseVisible([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_BaseVisible(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1305: [propput, id(110), helpstring("property BaseVisible")] HRESULT BaseVisible([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_BaseVisible(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 735:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1306: [propget, id(735), helpstring("property RingVisible")] HRESULT RingVisible([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_RingVisible(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1307: [propput, id(735), helpstring("property RingVisible")] HRESULT RingVisible([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_RingVisible(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 736:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1308: [propget, id(736), helpstring("property SkirtVisible")] HRESULT SkirtVisible([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_SkirtVisible(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1309: [propput, id(736), helpstring("property SkirtVisible")] HRESULT SkirtVisible([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_SkirtVisible(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 111:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1310: [propget, id(111), helpstring("property Collidable")] HRESULT Collidable([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_Collidable(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1311: [propput, id(111), helpstring("property Collidable")] HRESULT Collidable([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_Collidable(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 431:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1312: [propget, id(431), helpstring("property ReflectionEnabled")] HRESULT ReflectionEnabled([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_ReflectionEnabled(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1313: [propput, id(431), helpstring("property ReflectionEnabled")] HRESULT ReflectionEnabled([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_ReflectionEnabled(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 115:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1314: [propget, id(115), helpstring("property Scatter")] HRESULT Scatter([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Scatter(&V_R4(pVarResult));
			}

			case 1: {
				// line 1315: [propput, id(115), helpstring("property Scatter")] HRESULT Scatter([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Scatter(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 822:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1316: [propget, id(822), helpstring("property EnableSkirtAnimation")] HRESULT EnableSkirtAnimation([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_EnableSkirtAnimation(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1317: [propput, id(822), helpstring("property EnableSkirtAnimation")] HRESULT EnableSkirtAnimation([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_EnableSkirtAnimation(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 999: {
			// line 1318: [id(999), helpstring("method PlayHit")] HRESULT PlayHit();
			return PlayHit();
		}

		default:
		break;
	}

	return DISP_E_UNKNOWNNAME;
}

robin_hood::unordered_map<int, wstring> Bumper::m_idNameMap = {
	robin_hood::pair<int, wstring> { DISPID_GameEvents_Init, wstring(L"_Init") },
	robin_hood::pair<int, wstring> { DISPID_TimerEvents_Timer, wstring(L"_Timer") },
	robin_hood::pair<int, wstring> { DISPID_HitEvents_Hit, wstring(L"_Hit") }
};

HRESULT Bumper::FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) {
	CComPtr<IDispatch> disp;
	g_pplayer->m_ptable->m_pcv->m_pScript->GetScriptDispatch(nullptr, &disp);

	const robin_hood::unordered_map<int, wstring>::iterator it = m_idNameMap.find(dispid);
	if (it != m_idNameMap.end()) {
		wstring name = wstring(m_wzName) + it->second;
		LPOLESTR fnNames = (LPOLESTR)name.c_str();

		DISPID tDispid;
		const HRESULT hr = disp->GetIDsOfNames(IID_NULL, &fnNames, 1, 0, &tDispid);

		if (hr == S_OK) {
			disp->Invoke(tDispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, pdispparams, nullptr, nullptr, nullptr);
		}
	}

	return S_OK;
}

robin_hood::unordered_map<wstring, int> Trigger::m_nameIDMap = {
	robin_hood::pair<wstring, int> { wstring(L"radius"), 346 },
	robin_hood::pair<wstring, int> { wstring(L"timerenabled"), DISPID_Timer_Enabled },
	robin_hood::pair<wstring, int> { wstring(L"timerinterval"), DISPID_Timer_Interval },
	robin_hood::pair<wstring, int> { wstring(L"x"), DISPID_X },
	robin_hood::pair<wstring, int> { wstring(L"y"), DISPID_Y },
	robin_hood::pair<wstring, int> { wstring(L"enabled"), DISPID_Enabled },
	robin_hood::pair<wstring, int> { wstring(L"visible"), 458 },
	robin_hood::pair<wstring, int> { wstring(L"triggershape"), DISPID_Shape },
	robin_hood::pair<wstring, int> { wstring(L"surface"), DISPID_Surface },
	robin_hood::pair<wstring, int> { wstring(L"name"), DISPID_Name },
	robin_hood::pair<wstring, int> { wstring(L"uservalue"), DISPID_UserValue },
	robin_hood::pair<wstring, int> { wstring(L"ballcntover"), 312 },
	robin_hood::pair<wstring, int> { wstring(L"destroyball"), 313 },
	robin_hood::pair<wstring, int> { wstring(L"hitheight"), 314 },
	robin_hood::pair<wstring, int> { wstring(L"material"), 340 },
	robin_hood::pair<wstring, int> { wstring(L"rotation"), 345 },
	robin_hood::pair<wstring, int> { wstring(L"wirethickness"), 347 },
	robin_hood::pair<wstring, int> { wstring(L"animspeed"), 26 },
	robin_hood::pair<wstring, int> { wstring(L"reflectionenabled"), 431 }
};

STDMETHODIMP Trigger::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	wstring name = wstring(*rgszNames);
	std::transform(name.begin(), name.end(), name.begin(), tolower);
	const robin_hood::unordered_map<wstring, int>::iterator it = m_nameIDMap.find(name);
	if (it != m_nameIDMap.end()) {
		*rgDispId = it->second;
		return S_OK;
	}
	return DISP_E_UNKNOWNNAME;
}

STDMETHODIMP Trigger::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
	switch(dispIdMember) {
		case 346:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1346: [propget, id(346), helpstring("property Radius")] HRESULT Radius([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Radius(&V_R4(pVarResult));
			}

			case 1: {
				// line 1347: [propput, id(346), helpstring("property Radius")] HRESULT Radius([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Radius(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Timer_Enabled:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1348: [propget, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_TimerEnabled(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1349: [propput, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_TimerEnabled(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Timer_Interval:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1350: [propget, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([out, retval] long *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_TimerInterval((long*)&V_I4(pVarResult));
			}

			case 1: {
				// line 1351: [propput, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([in] long newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_TimerInterval(V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_X:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1352: [propget, id(DISPID_X), helpstring("property X")] HRESULT X([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_X(&V_R4(pVarResult));
			}

			case 1: {
				// line 1353: [propput, id(DISPID_X), helpstring("property X")] HRESULT X([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_X(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Y:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1354: [propget, id(DISPID_Y), helpstring("property Y")] HRESULT Y([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Y(&V_R4(pVarResult));
			}

			case 1: {
				// line 1355: [propput, id(DISPID_Y), helpstring("property Y")] HRESULT Y([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Y(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Enabled:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1356: [propget, id(DISPID_Enabled), helpstring("property Enabled")] HRESULT Enabled([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_Enabled(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1357: [propput, id(DISPID_Enabled), helpstring("property Enabled")] HRESULT Enabled([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_Enabled(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 458:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1358: [propget, id(458), helpstring("property Visible")] HRESULT Visible([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_Visible(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1359: [propput, id(458), helpstring("property Visible")] HRESULT Visible([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_Visible(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Shape:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1360: [propget, id(DISPID_Shape), helpstring("property TriggerShape")] HRESULT TriggerShape([out, retval] TriggerShape *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_TriggerShape((TriggerShape*)&V_I4(pVarResult));
			}

			case 1: {
				// line 1361: [propput, id(DISPID_Shape), helpstring("property TriggerShape")] HRESULT TriggerShape([in] TriggerShape newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_TriggerShape((TriggerShape)V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Surface:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1362: [propget, id(DISPID_Surface), helpstring("property Surface")] HRESULT Surface([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Surface(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 1363: [propput, id(DISPID_Surface), helpstring("property Surface")] HRESULT Surface([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Surface(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Name:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1364: [propget, id(DISPID_Name), helpstring("property Name")] HRESULT Name([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Name(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 1365: [propput, id(DISPID_Name), helpstring("property Name")] HRESULT Name([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Name(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_UserValue:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1366: [propget, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([out, retval] VARIANT* pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				return get_UserValue(pVarResult);
			}

			case 1: {
				// line 1367: [propput, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([in] VARIANT* newVal);
				CComVariant var0;
				VariantCopy(&var0, pDispParams->rgvarg);
				return put_UserValue(&var0);
			}

			default:
			break;
		}
		break;

		case 312: {
			// line 1368: [id(312), helpstring("method BallCntOver")] HRESULT BallCntOver([out, retval] int *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_I4;
			return BallCntOver(&V_I4(pVarResult));
		}

		case 313: {
			// line 1369: [id(313), helpstring("method DestroyBall")] HRESULT DestroyBall([out, retval] int *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_I4;
			return DestroyBall(&V_I4(pVarResult));
		}

		case 314:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1370: [propget, id(314), helpstring("property HitHeight")] HRESULT HitHeight([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_HitHeight(&V_R4(pVarResult));
			}

			case 1: {
				// line 1371: [propput, id(314), helpstring("property HitHeight")] HRESULT HitHeight([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_HitHeight(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 340:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1372: [propget, id(340), helpstring("property Material")] HRESULT Material([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Material(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 1373: [propput, id(340), helpstring("property Material")] HRESULT Material([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Material(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case 345:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1374: [propget, id(345), helpstring("property Rotation")] HRESULT Rotation([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Rotation(&V_R4(pVarResult));
			}

			case 1: {
				// line 1375: [propput, id(345), helpstring("property Rotation")] HRESULT Rotation([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Rotation(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 347:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1376: [propget, id(347), helpstring("property WireThickness")] HRESULT WireThickness([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_WireThickness(&V_R4(pVarResult));
			}

			case 1: {
				// line 1377: [propput, id(347), helpstring("property WireThickness")] HRESULT WireThickness([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_WireThickness(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 26:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1378: [propget, id(26), helpstring("property AnimSpeed")] HRESULT AnimSpeed([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_AnimSpeed(&V_R4(pVarResult));
			}

			case 1: {
				// line 1379: [propput, id(26), helpstring("property AnimSpeed")] HRESULT AnimSpeed([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_AnimSpeed(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 431:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1380: [propget, id(431), helpstring("property ReflectionEnabled")] HRESULT ReflectionEnabled([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_ReflectionEnabled(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1381: [propput, id(431), helpstring("property ReflectionEnabled")] HRESULT ReflectionEnabled([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_ReflectionEnabled(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		default:
		break;
	}

	return DISP_E_UNKNOWNNAME;
}

robin_hood::unordered_map<int, wstring> Trigger::m_idNameMap = {
	robin_hood::pair<int, wstring> { DISPID_GameEvents_Init, wstring(L"_Init") },
	robin_hood::pair<int, wstring> { DISPID_TimerEvents_Timer, wstring(L"_Timer") },
	robin_hood::pair<int, wstring> { DISPID_HitEvents_Hit, wstring(L"_Hit") },
	robin_hood::pair<int, wstring> { DISPID_HitEvents_Unhit, wstring(L"_Unhit") }
};

HRESULT Trigger::FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) {
	CComPtr<IDispatch> disp;
	g_pplayer->m_ptable->m_pcv->m_pScript->GetScriptDispatch(nullptr, &disp);

	const robin_hood::unordered_map<int, wstring>::iterator it = m_idNameMap.find(dispid);
	if (it != m_idNameMap.end()) {
		wstring name = wstring(m_wzName) + it->second;
		LPOLESTR fnNames = (LPOLESTR)name.c_str();

		DISPID tDispid;
		const HRESULT hr = disp->GetIDsOfNames(IID_NULL, &fnNames, 1, 0, &tDispid);

		if (hr == S_OK) {
			disp->Invoke(tDispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, pdispparams, nullptr, nullptr, nullptr);
		}
	}

	return S_OK;
}

robin_hood::unordered_map<wstring, int> Light::m_nameIDMap = {
	robin_hood::pair<wstring, int> { wstring(L"falloff"), 1 },
	robin_hood::pair<wstring, int> { wstring(L"falloffpower"), 432 },
	robin_hood::pair<wstring, int> { wstring(L"state"), DISPID_Light_State },
	robin_hood::pair<wstring, int> { wstring(L"getinplaystate"), 595 },
	robin_hood::pair<wstring, int> { wstring(L"getinplaystatebool"), 596 },
	robin_hood::pair<wstring, int> { wstring(L"getinplayintensity"), 597 },
	robin_hood::pair<wstring, int> { wstring(L"color"), 3 },
	robin_hood::pair<wstring, int> { wstring(L"colorfull"), 457 },
	robin_hood::pair<wstring, int> { wstring(L"timerenabled"), DISPID_Timer_Enabled },
	robin_hood::pair<wstring, int> { wstring(L"timerinterval"), DISPID_Timer_Interval },
	robin_hood::pair<wstring, int> { wstring(L"x"), DISPID_X },
	robin_hood::pair<wstring, int> { wstring(L"y"), DISPID_Y },
	robin_hood::pair<wstring, int> { wstring(L"blinkpattern"), 9 },
	robin_hood::pair<wstring, int> { wstring(L"blinkinterval"), DISPID_Light_BlinkInterval },
	robin_hood::pair<wstring, int> { wstring(L"duration"), 38 },
	robin_hood::pair<wstring, int> { wstring(L"intensity"), 12 },
	robin_hood::pair<wstring, int> { wstring(L"transmissionscale"), 617 },
	robin_hood::pair<wstring, int> { wstring(L"intensityscale"), 434 },
	robin_hood::pair<wstring, int> { wstring(L"surface"), DISPID_Surface },
	robin_hood::pair<wstring, int> { wstring(L"name"), DISPID_Name },
	robin_hood::pair<wstring, int> { wstring(L"uservalue"), DISPID_UserValue },
	robin_hood::pair<wstring, int> { wstring(L"image"), DISPID_Image },
	robin_hood::pair<wstring, int> { wstring(L"imagemode"), 453 },
	robin_hood::pair<wstring, int> { wstring(L"depthbias"), 397 },
	robin_hood::pair<wstring, int> { wstring(L"fadespeedup"), 377 },
	robin_hood::pair<wstring, int> { wstring(L"fadespeeddown"), 437 },
	robin_hood::pair<wstring, int> { wstring(L"bulb"), 340 },
	robin_hood::pair<wstring, int> { wstring(L"showbulbmesh"), 394 },
	robin_hood::pair<wstring, int> { wstring(L"staticbulbmesh"), 727 },
	robin_hood::pair<wstring, int> { wstring(L"showreflectiononball"), 455 },
	robin_hood::pair<wstring, int> { wstring(L"scalebulbmesh"), 425 },
	robin_hood::pair<wstring, int> { wstring(L"bulbmodulatevsadd"), 431 },
	robin_hood::pair<wstring, int> { wstring(L"bulbhaloheight"), 429 },
	robin_hood::pair<wstring, int> { wstring(L"visible"), 615 }
};

STDMETHODIMP Light::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	wstring name = wstring(*rgszNames);
	std::transform(name.begin(), name.end(), name.begin(), tolower);
	const robin_hood::unordered_map<wstring, int>::iterator it = m_nameIDMap.find(name);
	if (it != m_nameIDMap.end()) {
		*rgDispId = it->second;
		return S_OK;
	}
	return DISP_E_UNKNOWNNAME;
}

STDMETHODIMP Light::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
	switch(dispIdMember) {
		case 1:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1410: [propget, id(1), helpstring("property Falloff")] HRESULT Falloff([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Falloff(&V_R4(pVarResult));
			}

			case 1: {
				// line 1411: [propput, id(1), helpstring("property Falloff")] HRESULT Falloff([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Falloff(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 432:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1412: [propget, id(432), helpstring("property FalloffPower")] HRESULT FalloffPower([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_FalloffPower(&V_R4(pVarResult));
			}

			case 1: {
				// line 1413: [propput, id(432), helpstring("property FalloffPower")] HRESULT FalloffPower([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_FalloffPower(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Light_State:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1414: [propget, id(DISPID_Light_State), helpstring("property State")] HRESULT State([out, retval] LightState *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_State((LightState*)&V_I4(pVarResult));
			}

			case 1: {
				// line 1415: [propput, id(DISPID_Light_State), helpstring("property State")] HRESULT State([in] LightState newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_State((LightState)V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case 595: {
			// line 1416: [id(595), helpstring("method GetInPlayState")] HRESULT GetInPlayState([out, retval] LightState* pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_I4;
			return GetInPlayState((LightState*)&V_I4(pVarResult));
		}

		case 596: {
			// line 1417: [id(596), helpstring("method GetInPlayStateBool")] HRESULT GetInPlayStateBool([out, retval] VARIANT_BOOL* pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_BOOL;
			return GetInPlayStateBool(&V_BOOL(pVarResult));
		}

		case 597: {
			// line 1418: [id(597), helpstring("method GetInPlayIntensity")] HRESULT GetInPlayIntensity([out, retval] float* pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_R4;
			return GetInPlayIntensity(&V_R4(pVarResult));
		}

		case 3:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1419: [propget, id(3), helpstring("property Color")] HRESULT Color([out, retval] OLE_COLOR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_UI4;
				return get_Color(&V_UI4(pVarResult));
			}

			case 1: {
				// line 1420: [propput, id(3), helpstring("property Color")] HRESULT Color([in] OLE_COLOR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_UI4);
				return put_Color((OLE_COLOR)V_UI4(&var0));
			}

			default:
			break;
		}
		break;

		case 457:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1421: [propget, id(457), helpstring("property ColorFull")] HRESULT ColorFull([out, retval] OLE_COLOR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_UI4;
				return get_ColorFull(&V_UI4(pVarResult));
			}

			case 1: {
				// line 1422: [propput, id(457), helpstring("property ColorFull")] HRESULT ColorFull([in] OLE_COLOR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_UI4);
				return put_ColorFull((OLE_COLOR)V_UI4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Timer_Enabled:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1423: [propget, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_TimerEnabled(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1424: [propput, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_TimerEnabled(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Timer_Interval:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1425: [propget, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([out, retval] long *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_TimerInterval((long*)&V_I4(pVarResult));
			}

			case 1: {
				// line 1426: [propput, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([in] long newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_TimerInterval(V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_X:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1427: [propget, id(DISPID_X), helpstring("property X")] HRESULT X([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_X(&V_R4(pVarResult));
			}

			case 1: {
				// line 1428: [propput, id(DISPID_X), helpstring("property X")] HRESULT X([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_X(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Y:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1429: [propget, id(DISPID_Y), helpstring("property Y")] HRESULT Y([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Y(&V_R4(pVarResult));
			}

			case 1: {
				// line 1430: [propput, id(DISPID_Y), helpstring("property Y")] HRESULT Y([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Y(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 9:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1431: [propget, id(9), helpstring("property BlinkPattern")] HRESULT BlinkPattern([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_BlinkPattern(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 1432: [propput, id(9), helpstring("property BlinkPattern")] HRESULT BlinkPattern([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_BlinkPattern(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Light_BlinkInterval:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1433: [propget, id(DISPID_Light_BlinkInterval), helpstring("property BlinkInterval")] HRESULT BlinkInterval([out, retval] long *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_BlinkInterval((long*)&V_I4(pVarResult));
			}

			case 1: {
				// line 1434: [propput, id(DISPID_Light_BlinkInterval), helpstring("property BlinkInterval")] HRESULT BlinkInterval([in] long newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_BlinkInterval(V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case 38: {
			// line 1435: [id(38), helpstring("method Duration")] HRESULT Duration([in] long startState,[in] long newVal, [in] long endState);
			int index = pDispParams->cArgs;
			CComVariant var0;
			VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
			CComVariant var1;
			VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_I4);
			CComVariant var2;
			VariantChangeType(&var2, &pDispParams->rgvarg[--index], 0, VT_I4);
			return Duration(V_I4(&var0), V_I4(&var1), V_I4(&var2));
		}

		case 12:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1436: [propget, id(12), helpstring("property Intensity")] HRESULT Intensity([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Intensity(&V_R4(pVarResult));
			}

			case 1: {
				// line 1437: [propput, id(12), helpstring("property Intensity")] HRESULT Intensity([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Intensity(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 617:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1438: [propget, id(617), helpstring("property TransmissionScale")] HRESULT TransmissionScale([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_TransmissionScale(&V_R4(pVarResult));
			}

			case 1: {
				// line 1439: [propput, id(617), helpstring("property TransmissionScale")] HRESULT TransmissionScale([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_TransmissionScale(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 434:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1440: [propget, id(434), helpstring("property IntensityScale")] HRESULT IntensityScale([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_IntensityScale(&V_R4(pVarResult));
			}

			case 1: {
				// line 1441: [propput, id(434), helpstring("property IntensityScale")] HRESULT IntensityScale([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_IntensityScale(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Surface:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1442: [propget, id(DISPID_Surface), helpstring("property Surface")] HRESULT Surface([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Surface(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 1443: [propput, id(DISPID_Surface), helpstring("property Surface")] HRESULT Surface([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Surface(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Name:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1444: [propget, id(DISPID_Name), helpstring("property Name")] HRESULT Name([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Name(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 1445: [propput, id(DISPID_Name), helpstring("property Name")] HRESULT Name([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Name(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_UserValue:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1446: [propget, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([out, retval] VARIANT* pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				return get_UserValue(pVarResult);
			}

			case 1: {
				// line 1447: [propput, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([in] VARIANT* newVal);
				CComVariant var0;
				VariantCopy(&var0, pDispParams->rgvarg);
				return put_UserValue(&var0);
			}

			default:
			break;
		}
		break;

		case DISPID_Image:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1448: [propget, id(DISPID_Image), helpstring("property Image")] HRESULT Image([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Image(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 1449: [propput, id(DISPID_Image), helpstring("property Image")] HRESULT Image([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Image(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case 453:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1450: [propget, id(453), helpstring("property ImageMode")] HRESULT ImageMode([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_ImageMode(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1451: [propput, id(453), helpstring("property ImageMode")] HRESULT ImageMode([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_ImageMode(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 397:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1452: [propget, id(397), helpstring("property DepthBias")] HRESULT DepthBias([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_DepthBias(&V_R4(pVarResult));
			}

			case 1: {
				// line 1453: [propput, id(397), helpstring("property DepthBias")] HRESULT DepthBias([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_DepthBias(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 377:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1454: [propget, id(377), helpstring("property FadeSpeedUp")] HRESULT FadeSpeedUp([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_FadeSpeedUp(&V_R4(pVarResult));
			}

			case 1: {
				// line 1455: [propput, id(377), helpstring("property FadeSpeedUp")] HRESULT FadeSpeedUp([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_FadeSpeedUp(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 437:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1456: [propget, id(437), helpstring("property FadeSpeedDown")] HRESULT FadeSpeedDown([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_FadeSpeedDown(&V_R4(pVarResult));
			}

			case 1: {
				// line 1457: [propput, id(437), helpstring("property FadeSpeedDown")] HRESULT FadeSpeedDown([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_FadeSpeedDown(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 340:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1458: [propget, id(340), helpstring("property Bulb")] HRESULT Bulb([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_Bulb(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1459: [propput, id(340), helpstring("property Bulb")] HRESULT Bulb([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_Bulb(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 394:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1460: [propget, id(394), helpstring("property ShowBulbMesh")] HRESULT ShowBulbMesh([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_ShowBulbMesh(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1461: [propput, id(394), helpstring("property ShowBulbMesh")] HRESULT ShowBulbMesh([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_ShowBulbMesh(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 727:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1462: [propget, id(727), helpstring("property StaticBulbMesh")] HRESULT StaticBulbMesh([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_StaticBulbMesh(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1463: [propput, id(727), helpstring("property StaticBulbMesh")] HRESULT StaticBulbMesh([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_StaticBulbMesh(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 455:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1464: [propget, id(455), helpstring("property ShowReflectionOnBall")] HRESULT ShowReflectionOnBall([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_ShowReflectionOnBall(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1465: [propput, id(455), helpstring("property ShowReflectionOnBall")] HRESULT ShowReflectionOnBall([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_ShowReflectionOnBall(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 425:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1466: [propget, id(425), helpstring("property ScaleBulbMesh")] HRESULT ScaleBulbMesh([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_ScaleBulbMesh(&V_R4(pVarResult));
			}

			case 1: {
				// line 1467: [propput, id(425), helpstring("property ScaleBulbMesh")] HRESULT ScaleBulbMesh([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_ScaleBulbMesh(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 431:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1468: [propget, id(431), helpstring("property BulbModulateVsAdd")] HRESULT BulbModulateVsAdd([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_BulbModulateVsAdd(&V_R4(pVarResult));
			}

			case 1: {
				// line 1469: [propput, id(431), helpstring("property BulbModulateVsAdd")] HRESULT BulbModulateVsAdd([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_BulbModulateVsAdd(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 429:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1470: [propget, id(429), helpstring("property BulbHaloHeight")] HRESULT BulbHaloHeight([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_BulbHaloHeight(&V_R4(pVarResult));
			}

			case 1: {
				// line 1471: [propput, id(429), helpstring("property BulbHaloHeight")] HRESULT BulbHaloHeight([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_BulbHaloHeight(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 615:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1472: [propget, id(615), helpstring("property Visible")] HRESULT Visible([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_Visible(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1473: [propput, id(615), helpstring("property Visible")] HRESULT Visible([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_Visible(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		default:
		break;
	}

	return DISP_E_UNKNOWNNAME;
}

robin_hood::unordered_map<int, wstring> Light::m_idNameMap = {
	robin_hood::pair<int, wstring> { DISPID_GameEvents_Init, wstring(L"_Init") },
	robin_hood::pair<int, wstring> { DISPID_TimerEvents_Timer, wstring(L"_Timer") }
};

HRESULT Light::FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) {
	CComPtr<IDispatch> disp;
	g_pplayer->m_ptable->m_pcv->m_pScript->GetScriptDispatch(nullptr, &disp);

	const robin_hood::unordered_map<int, wstring>::iterator it = m_idNameMap.find(dispid);
	if (it != m_idNameMap.end()) {
		wstring name = wstring(m_wzName) + it->second;
		LPOLESTR fnNames = (LPOLESTR)name.c_str();

		DISPID tDispid;
		const HRESULT hr = disp->GetIDsOfNames(IID_NULL, &fnNames, 1, 0, &tDispid);

		if (hr == S_OK) {
			disp->Invoke(tDispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, pdispparams, nullptr, nullptr, nullptr);
		}
	}

	return S_OK;
}

robin_hood::unordered_map<wstring, int> Kicker::m_nameIDMap = {
	robin_hood::pair<wstring, int> { wstring(L"name"), DISPID_Name },
	robin_hood::pair<wstring, int> { wstring(L"createball"), 1 },
	robin_hood::pair<wstring, int> { wstring(L"createsizedball"), 11 },
	robin_hood::pair<wstring, int> { wstring(L"createsizedballwithmass"), 444 },
	robin_hood::pair<wstring, int> { wstring(L"destroyball"), 2 },
	robin_hood::pair<wstring, int> { wstring(L"timerenabled"), DISPID_Timer_Enabled },
	robin_hood::pair<wstring, int> { wstring(L"timerinterval"), DISPID_Timer_Interval },
	robin_hood::pair<wstring, int> { wstring(L"kick"), 5 },
	robin_hood::pair<wstring, int> { wstring(L"kickxyz"), 310 },
	robin_hood::pair<wstring, int> { wstring(L"kickz"), 311 },
	robin_hood::pair<wstring, int> { wstring(L"x"), DISPID_X },
	robin_hood::pair<wstring, int> { wstring(L"y"), DISPID_Y },
	robin_hood::pair<wstring, int> { wstring(L"enabled"), DISPID_Enabled },
	robin_hood::pair<wstring, int> { wstring(L"drawstyle"), 9 },
	robin_hood::pair<wstring, int> { wstring(L"material"), 340 },
	robin_hood::pair<wstring, int> { wstring(L"surface"), DISPID_Surface },
	robin_hood::pair<wstring, int> { wstring(L"uservalue"), DISPID_UserValue },
	robin_hood::pair<wstring, int> { wstring(L"ballcntover"), 312 },
	robin_hood::pair<wstring, int> { wstring(L"scatter"), 115 },
	robin_hood::pair<wstring, int> { wstring(L"hitaccuracy"), 314 },
	robin_hood::pair<wstring, int> { wstring(L"hitheight"), 315 },
	robin_hood::pair<wstring, int> { wstring(L"orientation"), 107 },
	robin_hood::pair<wstring, int> { wstring(L"radius"), 111 },
	robin_hood::pair<wstring, int> { wstring(L"fallthrough"), 394 },
	robin_hood::pair<wstring, int> { wstring(L"legacy"), 431 },
	robin_hood::pair<wstring, int> { wstring(L"lastcapturedball"), 19 }
};

STDMETHODIMP Kicker::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	wstring name = wstring(*rgszNames);
	std::transform(name.begin(), name.end(), name.begin(), tolower);
	const robin_hood::unordered_map<wstring, int>::iterator it = m_nameIDMap.find(name);
	if (it != m_nameIDMap.end()) {
		*rgDispId = it->second;
		return S_OK;
	}
	return DISP_E_UNKNOWNNAME;
}

STDMETHODIMP Kicker::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
	switch(dispIdMember) {
		case DISPID_Name:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1501: [propget, id(DISPID_Name), helpstring("property Name")] HRESULT Name([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Name(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 1502: [propput, id(DISPID_Name), helpstring("property Name")] HRESULT Name([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Name(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case 1: {
			// line 1503: [id(1), helpstring("method CreateBall")] HRESULT CreateBall([out, retval] IBall **Ball);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_DISPATCH;
			return CreateBall((IBall**)&V_DISPATCH(pVarResult));
		}

		case 11: {
			// line 1504: [id(11), helpstring("method CreateSizedBall")] HRESULT CreateSizedBall([in] float radius, [out, retval] IBall **Ball);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			CComVariant var0;
			VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
			V_VT(pVarResult) = VT_DISPATCH;
			return CreateSizedBall(V_R4(&var0), (IBall**)&V_DISPATCH(pVarResult));
		}

		case 444: {
			// line 1505: [id(444), helpstring("method CreateSizedBallWithMass")] HRESULT CreateSizedBallWithMass([in] float radius, [in] float mass, [out, retval] IBall **Ball);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			int index = pDispParams->cArgs;
			CComVariant var0;
			VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
			CComVariant var1;
			VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_R4);
			V_VT(pVarResult) = VT_DISPATCH;
			return CreateSizedBallWithMass(V_R4(&var0), V_R4(&var1), (IBall**)&V_DISPATCH(pVarResult));
		}

		case 2: {
			// line 1506: [id(2), helpstring("method DestroyBall")] HRESULT DestroyBall([out, retval] int *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_I4;
			return DestroyBall(&V_I4(pVarResult));
		}

		case DISPID_Timer_Enabled:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1507: [propget, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_TimerEnabled(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1508: [propput, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_TimerEnabled(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Timer_Interval:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1509: [propget, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([out, retval] long *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_TimerInterval((long*)&V_I4(pVarResult));
			}

			case 1: {
				// line 1510: [propput, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([in] long newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_TimerInterval(V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case 5: {
			// line 1511: [id(5), helpstring("method Kick")] HRESULT Kick(float angle, float speed, [defaultvalue(0)] float inclination);
			int index = pDispParams->cArgs;
			CComVariant var0;
			VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
			CComVariant var1;
			VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_R4);
			CComVariant var2(0);
			VariantChangeType(&var2, (index > 0) ? &pDispParams->rgvarg[--index] : &var2, 0, VT_R4);
			return Kick(V_R4(&var0), V_R4(&var1), V_R4(&var2));
		}

		case 310: {
			// line 1512: [id(310), helpstring("method KickXYZ")] HRESULT KickXYZ(float angle, float speed, float inclination, float x, float y, float z);
			int index = pDispParams->cArgs;
			CComVariant var0;
			VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
			CComVariant var1;
			VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_R4);
			CComVariant var2;
			VariantChangeType(&var2, &pDispParams->rgvarg[--index], 0, VT_R4);
			CComVariant var3;
			VariantChangeType(&var3, &pDispParams->rgvarg[--index], 0, VT_R4);
			CComVariant var4;
			VariantChangeType(&var4, &pDispParams->rgvarg[--index], 0, VT_R4);
			CComVariant var5;
			VariantChangeType(&var5, &pDispParams->rgvarg[--index], 0, VT_R4);
			return KickXYZ(V_R4(&var0), V_R4(&var1), V_R4(&var2), V_R4(&var3), V_R4(&var4), V_R4(&var5));
		}

		case 311: {
			// line 1513: [id(311), helpstring("method KickZ")] HRESULT KickZ(float angle, float speed, float inclination, float heightz);
			int index = pDispParams->cArgs;
			CComVariant var0;
			VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
			CComVariant var1;
			VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_R4);
			CComVariant var2;
			VariantChangeType(&var2, &pDispParams->rgvarg[--index], 0, VT_R4);
			CComVariant var3;
			VariantChangeType(&var3, &pDispParams->rgvarg[--index], 0, VT_R4);
			return KickZ(V_R4(&var0), V_R4(&var1), V_R4(&var2), V_R4(&var3));
		}

		case DISPID_X:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1514: [propget, id(DISPID_X), helpstring("property X")] HRESULT X([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_X(&V_R4(pVarResult));
			}

			case 1: {
				// line 1515: [propput, id(DISPID_X), helpstring("property X")] HRESULT X([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_X(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Y:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1516: [propget, id(DISPID_Y), helpstring("property Y")] HRESULT Y([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Y(&V_R4(pVarResult));
			}

			case 1: {
				// line 1517: [propput, id(DISPID_Y), helpstring("property Y")] HRESULT Y([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Y(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Enabled:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1518: [propget, id(DISPID_Enabled), helpstring("property Enabled")] HRESULT Enabled([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_Enabled(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1519: [propput, id(DISPID_Enabled), helpstring("property Enabled")] HRESULT Enabled([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_Enabled(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 9:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1520: [propget, id(9), helpstring("property DrawStyle")] HRESULT DrawStyle([out, retval] KickerType *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_DrawStyle((KickerType*)&V_I4(pVarResult));
			}

			case 1: {
				// line 1521: [propput, id(9), helpstring("property DrawStyle")] HRESULT DrawStyle([in] KickerType newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_DrawStyle((KickerType)V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case 340:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1522: [propget, id(340), helpstring("property Material")] HRESULT Material([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Material(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 1523: [propput, id(340), helpstring("property Material")] HRESULT Material([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Material(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Surface:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1524: [propget, id(DISPID_Surface), helpstring("property Surface")] HRESULT Surface([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Surface(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 1525: [propput, id(DISPID_Surface), helpstring("property Surface")] HRESULT Surface([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Surface(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_UserValue:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1526: [propget, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([out, retval] VARIANT* pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				return get_UserValue(pVarResult);
			}

			case 1: {
				// line 1527: [propput, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([in] VARIANT* newVal);
				CComVariant var0;
				VariantCopy(&var0, pDispParams->rgvarg);
				return put_UserValue(&var0);
			}

			default:
			break;
		}
		break;

		case 312: {
			// line 1528: [id(312), helpstring("method BallCntOver")] HRESULT BallCntOver([out, retval] int *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_I4;
			return BallCntOver(&V_I4(pVarResult));
		}

		case 115:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1529: [propget, id(115), helpstring("property Scatter")] HRESULT Scatter([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Scatter(&V_R4(pVarResult));
			}

			case 1: {
				// line 1530: [propput, id(115), helpstring("property Scatter")] HRESULT Scatter([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Scatter(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 314:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1531: [propget, id(314), helpstring("property HitAccuracy")] HRESULT HitAccuracy([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_HitAccuracy(&V_R4(pVarResult));
			}

			case 1: {
				// line 1532: [propput, id(314), helpstring("property HitAccuracy")] HRESULT HitAccuracy([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_HitAccuracy(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 315:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1533: [propget, id(315), helpstring("property HitHeight")] HRESULT HitHeight([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_HitHeight(&V_R4(pVarResult));
			}

			case 1: {
				// line 1534: [propput, id(315), helpstring("property HitHeight")] HRESULT HitHeight([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_HitHeight(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 107:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1535: [propget, id(107), helpstring("property Orientation")] HRESULT Orientation([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Orientation(&V_R4(pVarResult));
			}

			case 1: {
				// line 1536: [propput, id(107), helpstring("property Orientation")] HRESULT Orientation([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Orientation(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 111:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1537: [propget, id(111), helpstring("property Radius")] HRESULT Radius([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Radius(&V_R4(pVarResult));
			}

			case 1: {
				// line 1538: [propput, id(111), helpstring("property Radius")] HRESULT Radius([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Radius(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 394:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1539: [propget, id(394), helpstring("property FallThrough")] HRESULT FallThrough([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_FallThrough(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1540: [propput, id(394), helpstring("property FallThrough")] HRESULT FallThrough([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_FallThrough(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 431:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1541: [propget, id(431), helpstring("property Legacy")] HRESULT Legacy([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_Legacy(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1542: [propput, id(431), helpstring("property Legacy")] HRESULT Legacy([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_Legacy(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 19: {
			// line 1543: [propget, id(19), helpstring("property LastCapturedBall")] HRESULT LastCapturedBall([out, retval] IBall **pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_DISPATCH;
			return get_LastCapturedBall((IBall**)&V_DISPATCH(pVarResult));
		}

		default:
		break;
	}

	return DISP_E_UNKNOWNNAME;
}

robin_hood::unordered_map<int, wstring> Kicker::m_idNameMap = {
	robin_hood::pair<int, wstring> { DISPID_GameEvents_Init, wstring(L"_Init") },
	robin_hood::pair<int, wstring> { DISPID_TimerEvents_Timer, wstring(L"_Timer") },
	robin_hood::pair<int, wstring> { DISPID_HitEvents_Hit, wstring(L"_Hit") },
	robin_hood::pair<int, wstring> { DISPID_HitEvents_Unhit, wstring(L"_Unhit") }
};

HRESULT Kicker::FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) {
	CComPtr<IDispatch> disp;
	g_pplayer->m_ptable->m_pcv->m_pScript->GetScriptDispatch(nullptr, &disp);

	const robin_hood::unordered_map<int, wstring>::iterator it = m_idNameMap.find(dispid);
	if (it != m_idNameMap.end()) {
		wstring name = wstring(m_wzName) + it->second;
		LPOLESTR fnNames = (LPOLESTR)name.c_str();

		DISPID tDispid;
		const HRESULT hr = disp->GetIDsOfNames(IID_NULL, &fnNames, 1, 0, &tDispid);

		if (hr == S_OK) {
			disp->Invoke(tDispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, pdispparams, nullptr, nullptr, nullptr);
		}
	}

	return S_OK;
}

robin_hood::unordered_map<wstring, int> Decal::m_nameIDMap = {
	robin_hood::pair<wstring, int> { wstring(L"rotation"), 1 },
	robin_hood::pair<wstring, int> { wstring(L"image"), DISPID_Image },
	robin_hood::pair<wstring, int> { wstring(L"width"), 3 },
	robin_hood::pair<wstring, int> { wstring(L"height"), 4 },
	robin_hood::pair<wstring, int> { wstring(L"x"), 5 },
	robin_hood::pair<wstring, int> { wstring(L"y"), 6 },
	robin_hood::pair<wstring, int> { wstring(L"type"), 7 },
	robin_hood::pair<wstring, int> { wstring(L"text"), 8 },
	robin_hood::pair<wstring, int> { wstring(L"sizingtype"), DISPID_Decal_SizingType },
	robin_hood::pair<wstring, int> { wstring(L"fontcolor"), 11 },
	robin_hood::pair<wstring, int> { wstring(L"material"), 340 },
	robin_hood::pair<wstring, int> { wstring(L"font"), DISPID_Decal_Font },
	robin_hood::pair<wstring, int> { wstring(L"hasverticaltext"), 13 },
	robin_hood::pair<wstring, int> { wstring(L"surface"), DISPID_Surface }
};

STDMETHODIMP Decal::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	wstring name = wstring(*rgszNames);
	std::transform(name.begin(), name.end(), name.begin(), tolower);
	const robin_hood::unordered_map<wstring, int>::iterator it = m_nameIDMap.find(name);
	if (it != m_nameIDMap.end()) {
		*rgDispId = it->second;
		return S_OK;
	}
	return DISP_E_UNKNOWNNAME;
}

STDMETHODIMP Decal::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
	switch(dispIdMember) {
		case 1:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1575: [propget, id(1), helpstring("property Rotation")] HRESULT Rotation([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Rotation(&V_R4(pVarResult));
			}

			case 1: {
				// line 1576: [propput, id(1), helpstring("property Rotation")] HRESULT Rotation([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Rotation(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Image:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1577: [propget, id(DISPID_Image), helpstring("property Image")] HRESULT Image([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Image(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 1578: [propput, id(DISPID_Image), helpstring("property Image")] HRESULT Image([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Image(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case 3:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1579: [propget, id(3), helpstring("property Width")] HRESULT Width([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Width(&V_R4(pVarResult));
			}

			case 1: {
				// line 1580: [propput, id(3), helpstring("property Width")] HRESULT Width([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Width(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 4:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1581: [propget, id(4), helpstring("property Height")] HRESULT Height([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Height(&V_R4(pVarResult));
			}

			case 1: {
				// line 1582: [propput, id(4), helpstring("property Height")] HRESULT Height([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Height(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 5:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1583: [propget, id(5), helpstring("property X")] HRESULT X([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_X(&V_R4(pVarResult));
			}

			case 1: {
				// line 1584: [propput, id(5), helpstring("property X")] HRESULT X([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_X(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 6:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1585: [propget, id(6), helpstring("property Y")] HRESULT Y([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Y(&V_R4(pVarResult));
			}

			case 1: {
				// line 1586: [propput, id(6), helpstring("property Y")] HRESULT Y([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Y(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 7:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1587: [propget, id(7), helpstring("property Type")] HRESULT Type([out, retval] DecalType *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_Type((DecalType*)&V_I4(pVarResult));
			}

			case 1: {
				// line 1588: [propput, id(7), helpstring("property Type")] HRESULT Type([in] DecalType newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_Type((DecalType)V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case 8:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1589: [propget, id(8), helpstring("property Text")] HRESULT Text([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Text(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 1590: [propput, id(8), helpstring("property Text")] HRESULT Text([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Text(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Decal_SizingType:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1591: [propget, id(DISPID_Decal_SizingType), helpstring("property SizingType")] HRESULT SizingType([out, retval] SizingType *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_SizingType((SizingType*)&V_I4(pVarResult));
			}

			case 1: {
				// line 1592: [propput, id(DISPID_Decal_SizingType), helpstring("property SizingType")] HRESULT SizingType([in] SizingType newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_SizingType((SizingType)V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case 11:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1593: [propget, id(11), helpstring("property FontColor")] HRESULT FontColor([out, retval] OLE_COLOR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_UI4;
				return get_FontColor(&V_UI4(pVarResult));
			}

			case 1: {
				// line 1594: [propput, id(11), helpstring("property FontColor")] HRESULT FontColor([in] OLE_COLOR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_UI4);
				return put_FontColor((OLE_COLOR)V_UI4(&var0));
			}

			default:
			break;
		}
		break;

		case 340:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1595: [propget, id(340), helpstring("property Material")] HRESULT Material([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Material(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 1596: [propput, id(340), helpstring("property Material")] HRESULT Material([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Material(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Decal_Font:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1597: [propget, id(DISPID_Decal_Font), helpstring("property Font")] HRESULT Font([out, retval] IFontDisp **pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_DISPATCH;
				return get_Font((IFontDisp**)&V_DISPATCH(pVarResult));
			}

			case 1: {
				// line 1598: [propputref, id(DISPID_Decal_Font), helpstring("property Font")] HRESULT Font([in] IFontDisp *newVal);
				return putref_Font((IFontDisp*)pDispParams->rgvarg);
			}

			default:
			break;
		}
		break;

		case 13:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1599: [propget, id(13), helpstring("property HasVerticalText")] HRESULT HasVerticalText([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_HasVerticalText(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1600: [propput, id(13), helpstring("property HasVerticalText")] HRESULT HasVerticalText([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_HasVerticalText(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Surface:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1601: [propget, id(DISPID_Surface), helpstring("property Surface")] HRESULT Surface([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Surface(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 1602: [propput, id(DISPID_Surface), helpstring("property Surface")] HRESULT Surface([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Surface(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		default:
		break;
	}

	return DISP_E_UNKNOWNNAME;
}

robin_hood::unordered_map<wstring, int> Primitive::m_nameIDMap = {
	robin_hood::pair<wstring, int> { wstring(L"playanim"), 1 },
	robin_hood::pair<wstring, int> { wstring(L"playanimendless"), 2 },
	robin_hood::pair<wstring, int> { wstring(L"stopanim"), 18 },
	robin_hood::pair<wstring, int> { wstring(L"showframe"), 19 },
	robin_hood::pair<wstring, int> { wstring(L"continueanim"), 35 },
	robin_hood::pair<wstring, int> { wstring(L"displaytexture"), 38 },
	robin_hood::pair<wstring, int> { wstring(L"sides"), 101 },
	robin_hood::pair<wstring, int> { wstring(L"visible"), 458 },
	robin_hood::pair<wstring, int> { wstring(L"material"), 340 },
	robin_hood::pair<wstring, int> { wstring(L"sidecolor"), 104 },
	robin_hood::pair<wstring, int> { wstring(L"drawtexturesinside"), 106 },
	robin_hood::pair<wstring, int> { wstring(L"uservalue"), DISPID_UserValue },
	robin_hood::pair<wstring, int> { wstring(L"image"), DISPID_Image },
	robin_hood::pair<wstring, int> { wstring(L"normalmap"), DISPID_Image2 },
	robin_hood::pair<wstring, int> { wstring(L"x"), DISPID_POSITION_X },
	robin_hood::pair<wstring, int> { wstring(L"y"), DISPID_POSITION_Y },
	robin_hood::pair<wstring, int> { wstring(L"z"), DISPID_POSITION_Z },
	robin_hood::pair<wstring, int> { wstring(L"size_x"), DISPID_SIZE_X },
	robin_hood::pair<wstring, int> { wstring(L"size_y"), DISPID_SIZE_Y },
	robin_hood::pair<wstring, int> { wstring(L"size_z"), DISPID_SIZE_Z },
	robin_hood::pair<wstring, int> { wstring(L"rotandtra0"), DISPID_ROTRA1 },
	robin_hood::pair<wstring, int> { wstring(L"rotx"), DISPID_ROT_X },
	robin_hood::pair<wstring, int> { wstring(L"rotandtra1"), DISPID_ROTRA2 },
	robin_hood::pair<wstring, int> { wstring(L"roty"), DISPID_ROT_Y },
	robin_hood::pair<wstring, int> { wstring(L"rotandtra2"), DISPID_ROTRA3 },
	robin_hood::pair<wstring, int> { wstring(L"rotz"), DISPID_ROT_Z },
	robin_hood::pair<wstring, int> { wstring(L"rotandtra3"), DISPID_ROTRA4 },
	robin_hood::pair<wstring, int> { wstring(L"transx"), DISPID_TRANS_X },
	robin_hood::pair<wstring, int> { wstring(L"rotandtra4"), DISPID_ROTRA5 },
	robin_hood::pair<wstring, int> { wstring(L"transy"), DISPID_TRANS_Y },
	robin_hood::pair<wstring, int> { wstring(L"rotandtra5"), DISPID_ROTRA6 },
	robin_hood::pair<wstring, int> { wstring(L"transz"), DISPID_TRANS_Z },
	robin_hood::pair<wstring, int> { wstring(L"rotandtra6"), DISPID_ROTRA7 },
	robin_hood::pair<wstring, int> { wstring(L"objrotx"), DISPID_OBJROT_X },
	robin_hood::pair<wstring, int> { wstring(L"rotandtra7"), DISPID_ROTRA8 },
	robin_hood::pair<wstring, int> { wstring(L"objroty"), DISPID_OBJROT_Y },
	robin_hood::pair<wstring, int> { wstring(L"rotandtra8"), DISPID_ROTRA9 },
	robin_hood::pair<wstring, int> { wstring(L"objrotz"), DISPID_OBJROT_Z },
	robin_hood::pair<wstring, int> { wstring(L"edgefactorui"), 454 },
	robin_hood::pair<wstring, int> { wstring(L"collisionreductionfactor"), 481 },
	robin_hood::pair<wstring, int> { wstring(L"name"), DISPID_Name },
	robin_hood::pair<wstring, int> { wstring(L"enablestaticrendering"), 398 },
	robin_hood::pair<wstring, int> { wstring(L"elasticity"), 110 },
	robin_hood::pair<wstring, int> { wstring(L"elasticityfalloff"), 112 },
	robin_hood::pair<wstring, int> { wstring(L"collidable"), 111 },
	robin_hood::pair<wstring, int> { wstring(L"istoy"), 395 },
	robin_hood::pair<wstring, int> { wstring(L"backfacesenabled"), 792 },
	robin_hood::pair<wstring, int> { wstring(L"hashitevent"), 34 },
	robin_hood::pair<wstring, int> { wstring(L"threshold"), 33 },
	robin_hood::pair<wstring, int> { wstring(L"friction"), 114 },
	robin_hood::pair<wstring, int> { wstring(L"scatter"), 115 },
	robin_hood::pair<wstring, int> { wstring(L"disablelighting"), 441 },
	robin_hood::pair<wstring, int> { wstring(L"blenddisablelighting"), 494 },
	robin_hood::pair<wstring, int> { wstring(L"blenddisablelightingfrombelow"), 496 },
	robin_hood::pair<wstring, int> { wstring(L"reflectionenabled"), 431 },
	robin_hood::pair<wstring, int> { wstring(L"opacity"), 377 },
	robin_hood::pair<wstring, int> { wstring(L"addblend"), 556 },
	robin_hood::pair<wstring, int> { wstring(L"color"), 557 },
	robin_hood::pair<wstring, int> { wstring(L"depthbias"), 397 },
	robin_hood::pair<wstring, int> { wstring(L"physicsmaterial"), 734 },
	robin_hood::pair<wstring, int> { wstring(L"overwritephysics"), 432 },
	robin_hood::pair<wstring, int> { wstring(L"hitthreshold"), 735 },
	robin_hood::pair<wstring, int> { wstring(L"objectspacenormalmap"), 824 }
};

STDMETHODIMP Primitive::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	wstring name = wstring(*rgszNames);
	std::transform(name.begin(), name.end(), name.begin(), tolower);
	const robin_hood::unordered_map<wstring, int>::iterator it = m_nameIDMap.find(name);
	if (it != m_nameIDMap.end()) {
		*rgDispId = it->second;
		return S_OK;
	}
	return DISP_E_UNKNOWNNAME;
}

STDMETHODIMP Primitive::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
	switch(dispIdMember) {
		case 1: {
			// line 1620: [id(1), helpstring("method PlayAnim")] HRESULT PlayAnim([in]float startFrame, [in]float speed);
			int index = pDispParams->cArgs;
			CComVariant var0;
			VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
			CComVariant var1;
			VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_R4);
			return PlayAnim(V_R4(&var0), V_R4(&var1));
		}

		case 2: {
			// line 1621: [id(2), helpstring("method PlayAnimEndless")] HRESULT PlayAnimEndless([in]float speed);
			CComVariant var0;
			VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
			return PlayAnimEndless(V_R4(&var0));
		}

		case 18: {
			// line 1622: [id(18), helpstring("method StopAnim")] HRESULT StopAnim();
			return StopAnim();
		}

		case 19: {
			// line 1623: [id(19), helpstring("method ShowFrame")] HRESULT ShowFrame([in]float frame);
			CComVariant var0;
			VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
			return ShowFrame(V_R4(&var0));
		}

		case 35: {
			// line 1624: [id(35), helpstring("method ContinueAnim")] HRESULT ContinueAnim([in]float speed);
			CComVariant var0;
			VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
			return ContinueAnim(V_R4(&var0));
		}

		case 38:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1625: [propget, id(38), helpstring("property DisplayTexture")] HRESULT DisplayTexture([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_DisplayTexture(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1626: [propput, id(38), helpstring("property DisplayTexture")] HRESULT DisplayTexture([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_DisplayTexture(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 101:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1628: [propget, id(101), helpstring("property Sides")] HRESULT Sides([out, retval] int *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_Sides(&V_I4(pVarResult));
			}

			case 1: {
				// line 1629: [propput, id(101), helpstring("property Sides")] HRESULT Sides([in] int newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_Sides(V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case 458:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1630: [propget, id(458), helpstring("property Top Visible")] HRESULT Visible([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_Visible(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1631: [propput, id(458), helpstring("property Top Visible")] HRESULT Visible([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_Visible(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 340:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1632: [propget, id(340), helpstring("property Material")] HRESULT Material([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Material(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 1633: [propput, id(340), helpstring("property Material")] HRESULT Material([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Material(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case 104:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1634: [propget, id(104), helpstring("property Side Color")] HRESULT SideColor([out, retval] OLE_COLOR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_UI4;
				return get_SideColor(&V_UI4(pVarResult));
			}

			case 1: {
				// line 1635: [propput, id(104), helpstring("property Side Color")] HRESULT SideColor([in] OLE_COLOR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_UI4);
				return put_SideColor((OLE_COLOR)V_UI4(&var0));
			}

			default:
			break;
		}
		break;

		case 106:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1636: [propget, id(106), helpstring("property Draw Textures Inside")] HRESULT DrawTexturesInside([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_DrawTexturesInside(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1637: [propput, id(106), helpstring("property Draw Textures Inside")] HRESULT DrawTexturesInside([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_DrawTexturesInside(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_UserValue:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1639: [propget, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([out, retval] VARIANT* pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				return get_UserValue(pVarResult);
			}

			case 1: {
				// line 1640: [propput, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([in] VARIANT* newVal);
				CComVariant var0;
				VariantCopy(&var0, pDispParams->rgvarg);
				return put_UserValue(&var0);
			}

			default:
			break;
		}
		break;

		case DISPID_Image:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1642: [propget, id(DISPID_Image), helpstring("property Image")] HRESULT Image([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Image(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 1643: [propput, id(DISPID_Image), helpstring("property Image")] HRESULT Image([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Image(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Image2:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1644: [propget, id(DISPID_Image2), helpstring("property NormalMap")] HRESULT NormalMap([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_NormalMap(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 1645: [propput, id(DISPID_Image2), helpstring("property NormalMap")] HRESULT NormalMap([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_NormalMap(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_POSITION_X:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1647: [propget, id(DISPID_POSITION_X), helpstring("property X")] HRESULT X([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_X(&V_R4(pVarResult));
			}

			case 1: {
				// line 1648: [propput, id(DISPID_POSITION_X), helpstring("property X")] HRESULT X([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_X(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_POSITION_Y:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1649: [propget, id(DISPID_POSITION_Y), helpstring("property Y")] HRESULT Y([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Y(&V_R4(pVarResult));
			}

			case 1: {
				// line 1650: [propput, id(DISPID_POSITION_Y), helpstring("property Y")] HRESULT Y([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Y(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_POSITION_Z:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1651: [propget, id(DISPID_POSITION_Z), helpstring("property Z")] HRESULT Z([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Z(&V_R4(pVarResult));
			}

			case 1: {
				// line 1652: [propput, id(DISPID_POSITION_Z), helpstring("property Z")] HRESULT Z([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Z(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_SIZE_X:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1653: [propget, id(DISPID_SIZE_X), helpstring("property Size X")] HRESULT Size_X([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Size_X(&V_R4(pVarResult));
			}

			case 1: {
				// line 1654: [propput, id(DISPID_SIZE_X), helpstring("property Size X")] HRESULT Size_X([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Size_X(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_SIZE_Y:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1655: [propget, id(DISPID_SIZE_Y), helpstring("property Size Y")] HRESULT Size_Y([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Size_Y(&V_R4(pVarResult));
			}

			case 1: {
				// line 1656: [propput, id(DISPID_SIZE_Y), helpstring("property Size Y")] HRESULT Size_Y([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Size_Y(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_SIZE_Z:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1657: [propget, id(DISPID_SIZE_Z), helpstring("property Size Z")] HRESULT Size_Z([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Size_Z(&V_R4(pVarResult));
			}

			case 1: {
				// line 1658: [propput, id(DISPID_SIZE_Z), helpstring("property Size Z")] HRESULT Size_Z([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Size_Z(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_ROTRA1:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1660: [propget, id(DISPID_ROTRA1), helpstring("property Rot and Tra 1")] HRESULT RotAndTra0([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_RotAndTra0(&V_R4(pVarResult));
			}

			case 1: {
				// line 1661: [propput, id(DISPID_ROTRA1), helpstring("property Rot and Tra 1")] HRESULT RotAndTra0([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_RotAndTra0(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_ROT_X:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1662: [propget, id(DISPID_ROT_X), helpstring("property RotX")] HRESULT RotX([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_RotX(&V_R4(pVarResult));
			}

			case 1: {
				// line 1663: [propput, id(DISPID_ROT_X), helpstring("property RotX")] HRESULT RotX([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_RotX(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_ROTRA2:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1665: [propget, id(DISPID_ROTRA2), helpstring("property Rot and Tra 2")] HRESULT RotAndTra1([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_RotAndTra1(&V_R4(pVarResult));
			}

			case 1: {
				// line 1666: [propput, id(DISPID_ROTRA2), helpstring("property Rot and Tra 2")] HRESULT RotAndTra1([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_RotAndTra1(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_ROT_Y:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1667: [propget, id(DISPID_ROT_Y), helpstring("property RotY")] HRESULT RotY([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_RotY(&V_R4(pVarResult));
			}

			case 1: {
				// line 1668: [propput, id(DISPID_ROT_Y), helpstring("property RotY")] HRESULT RotY([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_RotY(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_ROTRA3:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1670: [propget, id(DISPID_ROTRA3), helpstring("property Rot and Tra 3")] HRESULT RotAndTra2([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_RotAndTra2(&V_R4(pVarResult));
			}

			case 1: {
				// line 1671: [propput, id(DISPID_ROTRA3), helpstring("property Rot and Tra 3")] HRESULT RotAndTra2([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_RotAndTra2(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_ROT_Z:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1672: [propget, id(DISPID_ROT_Z), helpstring("property RotZ")] HRESULT RotZ([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_RotZ(&V_R4(pVarResult));
			}

			case 1: {
				// line 1673: [propput, id(DISPID_ROT_Z), helpstring("property RotZ")] HRESULT RotZ([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_RotZ(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_ROTRA4:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1675: [propget, id(DISPID_ROTRA4), helpstring("property Rot and Tra 4")] HRESULT RotAndTra3([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_RotAndTra3(&V_R4(pVarResult));
			}

			case 1: {
				// line 1676: [propput, id(DISPID_ROTRA4), helpstring("property Rot and Tra 4")] HRESULT RotAndTra3([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_RotAndTra3(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_TRANS_X:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1677: [propget, id(DISPID_TRANS_X), helpstring("property TransX")] HRESULT TransX([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_TransX(&V_R4(pVarResult));
			}

			case 1: {
				// line 1678: [propput, id(DISPID_TRANS_X), helpstring("property TransX")] HRESULT TransX([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_TransX(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_ROTRA5:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1680: [propget, id(DISPID_ROTRA5), helpstring("property Rot and Tra 5")] HRESULT RotAndTra4([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_RotAndTra4(&V_R4(pVarResult));
			}

			case 1: {
				// line 1681: [propput, id(DISPID_ROTRA5), helpstring("property Rot and Tra 5")] HRESULT RotAndTra4([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_RotAndTra4(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_TRANS_Y:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1682: [propget, id(DISPID_TRANS_Y), helpstring("property TransY")] HRESULT TransY([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_TransY(&V_R4(pVarResult));
			}

			case 1: {
				// line 1683: [propput, id(DISPID_TRANS_Y), helpstring("property TransY")] HRESULT TransY([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_TransY(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_ROTRA6:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1685: [propget, id(DISPID_ROTRA6), helpstring("property Rot and Tra 6")] HRESULT RotAndTra5([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_RotAndTra5(&V_R4(pVarResult));
			}

			case 1: {
				// line 1686: [propput, id(DISPID_ROTRA6), helpstring("property Rot and Tra 6")] HRESULT RotAndTra5([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_RotAndTra5(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_TRANS_Z:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1687: [propget, id(DISPID_TRANS_Z), helpstring("property TransZ")] HRESULT TransZ([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_TransZ(&V_R4(pVarResult));
			}

			case 1: {
				// line 1688: [propput, id(DISPID_TRANS_Z), helpstring("property TransZ")] HRESULT TransZ([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_TransZ(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_ROTRA7:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1690: [propget, id(DISPID_ROTRA7), helpstring("property Rot and Tra 7")] HRESULT RotAndTra6([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_RotAndTra6(&V_R4(pVarResult));
			}

			case 1: {
				// line 1691: [propput, id(DISPID_ROTRA7), helpstring("property Rot and Tra 7")] HRESULT RotAndTra6([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_RotAndTra6(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_OBJROT_X:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1692: [propget, id(DISPID_OBJROT_X), helpstring("property ObjRotX")] HRESULT ObjRotX([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_ObjRotX(&V_R4(pVarResult));
			}

			case 1: {
				// line 1693: [propput, id(DISPID_OBJROT_X), helpstring("property ObjRotX")] HRESULT ObjRotX([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_ObjRotX(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_ROTRA8:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1695: [propget, id(DISPID_ROTRA8), helpstring("property Rot and Tra 8")] HRESULT RotAndTra7([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_RotAndTra7(&V_R4(pVarResult));
			}

			case 1: {
				// line 1696: [propput, id(DISPID_ROTRA8), helpstring("property Rot and Tra 8")] HRESULT RotAndTra7([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_RotAndTra7(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_OBJROT_Y:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1697: [propget, id(DISPID_OBJROT_Y), helpstring("property ObjRotY")] HRESULT ObjRotY([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_ObjRotY(&V_R4(pVarResult));
			}

			case 1: {
				// line 1698: [propput, id(DISPID_OBJROT_Y), helpstring("property ObjRotY")] HRESULT ObjRotY([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_ObjRotY(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_ROTRA9:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1700: [propget, id(DISPID_ROTRA9), helpstring("property Rot and Tra 9")] HRESULT RotAndTra8([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_RotAndTra8(&V_R4(pVarResult));
			}

			case 1: {
				// line 1701: [propput, id(DISPID_ROTRA9), helpstring("property Rot and Tra 9")] HRESULT RotAndTra8([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_RotAndTra8(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_OBJROT_Z:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1702: [propget, id(DISPID_OBJROT_Z), helpstring("property ObjRotZ")] HRESULT ObjRotZ([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_ObjRotZ(&V_R4(pVarResult));
			}

			case 1: {
				// line 1703: [propput, id(DISPID_OBJROT_Z), helpstring("property ObjRotZ")] HRESULT ObjRotZ([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_ObjRotZ(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 454:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1705: [propget, id(454), helpstring("property EdgeFactorUI")] HRESULT EdgeFactorUI([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_EdgeFactorUI(&V_R4(pVarResult));
			}

			case 1: {
				// line 1706: [propput, id(454), helpstring("property EdgeFactorUI")] HRESULT EdgeFactorUI([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_EdgeFactorUI(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 481:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1707: [propget, id(481), helpstring("property CollisionReductionFactor")] HRESULT CollisionReductionFactor([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_CollisionReductionFactor(&V_R4(pVarResult));
			}

			case 1: {
				// line 1708: [propput, id(481), helpstring("property CollisionReductionFactor")] HRESULT CollisionReductionFactor([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_CollisionReductionFactor(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Name:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1710: [propget, id(DISPID_Name), helpstring("property Name")] HRESULT Name([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Name(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 1711: [propput, id(DISPID_Name), helpstring("property Name")] HRESULT Name([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Name(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case 398:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1712: [propget, id(398), helpstring("property static rendering")] HRESULT EnableStaticRendering([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_EnableStaticRendering(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1713: [propput, id(398), helpstring("property static rendering")] HRESULT EnableStaticRendering([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_EnableStaticRendering(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 110:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1715: [propget, id(110), helpstring("property Elasticity")] HRESULT Elasticity([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Elasticity(&V_R4(pVarResult));
			}

			case 1: {
				// line 1716: [propput, id(110), helpstring("property Elasticity")] HRESULT Elasticity([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Elasticity(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 112:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1717: [propget, id(112), helpstring("property ElasticityFalloff")] HRESULT ElasticityFalloff([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_ElasticityFalloff(&V_R4(pVarResult));
			}

			case 1: {
				// line 1718: [propput, id(112), helpstring("property ElasticityFalloff")] HRESULT ElasticityFalloff([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_ElasticityFalloff(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 111:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1720: [propget, id(111), helpstring("property Collidable")] HRESULT Collidable([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_Collidable(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1721: [propput, id(111), helpstring("property Collidable")] HRESULT Collidable([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_Collidable(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 395:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1722: [propget, id(395), helpstring("property IsToy")] HRESULT IsToy([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_IsToy(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1723: [propput, id(395), helpstring("property IsToy")] HRESULT IsToy([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_IsToy(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 792:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1724: [propget, id(792), helpstring("property BackfacesEnabled")] HRESULT BackfacesEnabled([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_BackfacesEnabled(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1725: [propput, id(792), helpstring("property BackfacesEnabled")] HRESULT BackfacesEnabled([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_BackfacesEnabled(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 34:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1726: [propget, id(34), helpstring("property HasHitEvent")] HRESULT HasHitEvent([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_HasHitEvent(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1727: [propput, id(34), helpstring("property HasHitEvent")] HRESULT HasHitEvent([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_HasHitEvent(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 33:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1728: [propget, id(33), helpstring("property Threshold")] HRESULT Threshold([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Threshold(&V_R4(pVarResult));
			}

			case 1: {
				// line 1729: [propput, id(33), helpstring("property Threshold")] HRESULT Threshold([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Threshold(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 114:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1730: [propget, id(114), helpstring("property Friction")] HRESULT Friction([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Friction(&V_R4(pVarResult));
			}

			case 1: {
				// line 1731: [propput, id(114), helpstring("property Friction")] HRESULT Friction([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Friction(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 115:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1732: [propget, id(115), helpstring("property Scatter")] HRESULT Scatter([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Scatter(&V_R4(pVarResult));
			}

			case 1: {
				// line 1733: [propput, id(115), helpstring("property Scatter")] HRESULT Scatter([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Scatter(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 441:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1734: [propget, id(441), helpstring("property DisableLighting")] HRESULT DisableLighting([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_DisableLighting(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1735: [propput, id(441), helpstring("property DisableLighting")] HRESULT DisableLighting([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_DisableLighting(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 494:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1736: [propget, id(494), helpstring("property BlendDisableLighting")] HRESULT BlendDisableLighting([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_BlendDisableLighting(&V_R4(pVarResult));
			}

			case 1: {
				// line 1737: [propput, id(494), helpstring("property BlendDisableLighting")] HRESULT BlendDisableLighting([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_BlendDisableLighting(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 496:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1738: [propget, id(496), helpstring("property BlendDisableLightingFromBelow")] HRESULT BlendDisableLightingFromBelow([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_BlendDisableLightingFromBelow(&V_R4(pVarResult));
			}

			case 1: {
				// line 1739: [propput, id(496), helpstring("property BlendDisableLightingFromBelow")] HRESULT BlendDisableLightingFromBelow([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_BlendDisableLightingFromBelow(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 431:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1740: [propget, id(431), helpstring("property ReflectionEnabled")] HRESULT ReflectionEnabled([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_ReflectionEnabled(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1741: [propput, id(431), helpstring("property ReflectionEnabled")] HRESULT ReflectionEnabled([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_ReflectionEnabled(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 377:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1743: [propget, id(377), helpstring("property Opacity")] HRESULT Opacity([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Opacity(&V_R4(pVarResult));
			}

			case 1: {
				// line 1744: [propput, id(377), helpstring("property Opacity")] HRESULT Opacity([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Opacity(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 556:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1745: [propget, id(556), helpstring("property AddBlend")] HRESULT AddBlend([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_AddBlend(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1746: [propput, id(556), helpstring("property AddBlend")] HRESULT AddBlend([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_AddBlend(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 557:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1747: [propget, id(557), helpstring("property Color")] HRESULT Color([out, retval] OLE_COLOR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_UI4;
				return get_Color(&V_UI4(pVarResult));
			}

			case 1: {
				// line 1748: [propput, id(557), helpstring("property Color")] HRESULT Color([in] OLE_COLOR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_UI4);
				return put_Color((OLE_COLOR)V_UI4(&var0));
			}

			default:
			break;
		}
		break;

		case 397:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1750: [propget, id(397), helpstring("property DepthBias")] HRESULT DepthBias([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_DepthBias(&V_R4(pVarResult));
			}

			case 1: {
				// line 1751: [propput, id(397), helpstring("property DepthBias")] HRESULT DepthBias([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_DepthBias(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 734:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1753: [propget, id(734), helpstring("property PhysicsMaterial")] HRESULT PhysicsMaterial([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_PhysicsMaterial(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 1754: [propput, id(734), helpstring("property PhysicsMaterial")] HRESULT PhysicsMaterial([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_PhysicsMaterial(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case 432:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1755: [propget, id(432), helpstring("property OverwritePhysics")] HRESULT OverwritePhysics([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_OverwritePhysics(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1756: [propput, id(432), helpstring("property OverwritePhysics")] HRESULT OverwritePhysics([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_OverwritePhysics(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 735: {
			// line 1757: [propget, id(735), helpstring("property HitThreshold")] HRESULT HitThreshold([out, retval] float *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_R4;
			return get_HitThreshold(&V_R4(pVarResult));
		}

		case 824:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1758: [propget, id(824), helpstring("property ObjectSpaceNormalMap")] HRESULT ObjectSpaceNormalMap([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_ObjectSpaceNormalMap(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1759: [propput, id(824), helpstring("property ObjectSpaceNormalMap")] HRESULT ObjectSpaceNormalMap([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_ObjectSpaceNormalMap(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		default:
		break;
	}

	return DISP_E_UNKNOWNNAME;
}

robin_hood::unordered_map<int, wstring> Primitive::m_idNameMap = {
	robin_hood::pair<int, wstring> { DISPID_GameEvents_Init, wstring(L"_Init") },
	robin_hood::pair<int, wstring> { DISPID_HitEvents_Hit, wstring(L"_Hit") }
};

HRESULT Primitive::FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) {
	CComPtr<IDispatch> disp;
	g_pplayer->m_ptable->m_pcv->m_pScript->GetScriptDispatch(nullptr, &disp);

	const robin_hood::unordered_map<int, wstring>::iterator it = m_idNameMap.find(dispid);
	if (it != m_idNameMap.end()) {
		wstring name = wstring(m_wzName) + it->second;
		LPOLESTR fnNames = (LPOLESTR)name.c_str();

		DISPID tDispid;
		const HRESULT hr = disp->GetIDsOfNames(IID_NULL, &fnNames, 1, 0, &tDispid);

		if (hr == S_OK) {
			disp->Invoke(tDispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, pdispparams, nullptr, nullptr, nullptr);
		}
	}

	return S_OK;
}

robin_hood::unordered_map<wstring, int> HitTarget::m_nameIDMap = {
	robin_hood::pair<wstring, int> { wstring(L"timerenabled"), DISPID_Timer_Enabled },
	robin_hood::pair<wstring, int> { wstring(L"timerinterval"), DISPID_Timer_Interval },
	robin_hood::pair<wstring, int> { wstring(L"visible"), 458 },
	robin_hood::pair<wstring, int> { wstring(L"material"), 340 },
	robin_hood::pair<wstring, int> { wstring(L"uservalue"), DISPID_UserValue },
	robin_hood::pair<wstring, int> { wstring(L"image"), DISPID_Image },
	robin_hood::pair<wstring, int> { wstring(L"x"), DISPID_POSITION_X },
	robin_hood::pair<wstring, int> { wstring(L"y"), DISPID_POSITION_Y },
	robin_hood::pair<wstring, int> { wstring(L"z"), DISPID_POSITION_Z },
	robin_hood::pair<wstring, int> { wstring(L"scalex"), DISPID_SIZE_X },
	robin_hood::pair<wstring, int> { wstring(L"scaley"), DISPID_SIZE_Y },
	robin_hood::pair<wstring, int> { wstring(L"scalez"), DISPID_SIZE_Z },
	robin_hood::pair<wstring, int> { wstring(L"orientation"), DISPID_ROT_Z },
	robin_hood::pair<wstring, int> { wstring(L"name"), DISPID_Name },
	robin_hood::pair<wstring, int> { wstring(L"elasticity"), 110 },
	robin_hood::pair<wstring, int> { wstring(L"elasticityfalloff"), 112 },
	robin_hood::pair<wstring, int> { wstring(L"collidable"), 111 },
	robin_hood::pair<wstring, int> { wstring(L"hashitevent"), 34 },
	robin_hood::pair<wstring, int> { wstring(L"threshold"), 33 },
	robin_hood::pair<wstring, int> { wstring(L"friction"), 114 },
	robin_hood::pair<wstring, int> { wstring(L"scatter"), 115 },
	robin_hood::pair<wstring, int> { wstring(L"disablelighting"), 483 },
	robin_hood::pair<wstring, int> { wstring(L"blenddisablelighting"), 494 },
	robin_hood::pair<wstring, int> { wstring(L"blenddisablelightingfrombelow"), 496 },
	robin_hood::pair<wstring, int> { wstring(L"reflectionenabled"), 431 },
	robin_hood::pair<wstring, int> { wstring(L"depthbias"), 397 },
	robin_hood::pair<wstring, int> { wstring(L"dropspeed"), 377 },
	robin_hood::pair<wstring, int> { wstring(L"isdropped"), 435 },
	robin_hood::pair<wstring, int> { wstring(L"drawstyle"), 9 },
	robin_hood::pair<wstring, int> { wstring(L"legacymode"), 433 },
	robin_hood::pair<wstring, int> { wstring(L"raisedelay"), 726 },
	robin_hood::pair<wstring, int> { wstring(L"physicsmaterial"), 734 },
	robin_hood::pair<wstring, int> { wstring(L"overwritephysics"), 432 },
	robin_hood::pair<wstring, int> { wstring(L"hitthreshold"), 727 }
};

STDMETHODIMP HitTarget::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	wstring name = wstring(*rgszNames);
	std::transform(name.begin(), name.end(), name.begin(), tolower);
	const robin_hood::unordered_map<wstring, int>::iterator it = m_nameIDMap.find(name);
	if (it != m_nameIDMap.end()) {
		*rgDispId = it->second;
		return S_OK;
	}
	return DISP_E_UNKNOWNNAME;
}

STDMETHODIMP HitTarget::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
	switch(dispIdMember) {
		case DISPID_Timer_Enabled:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1789: [propget, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_TimerEnabled(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1790: [propput, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_TimerEnabled(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Timer_Interval:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1791: [propget, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([out, retval] long *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_TimerInterval((long*)&V_I4(pVarResult));
			}

			case 1: {
				// line 1792: [propput, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([in] long newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_TimerInterval(V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case 458:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1793: [propget, id(458), helpstring("property Top Visible")] HRESULT Visible([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_Visible(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1794: [propput, id(458), helpstring("property Top Visible")] HRESULT Visible([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_Visible(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 340:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1795: [propget, id(340), helpstring("property Material")] HRESULT Material([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Material(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 1796: [propput, id(340), helpstring("property Material")] HRESULT Material([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Material(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_UserValue:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1797: [propget, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([out, retval] VARIANT* pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				return get_UserValue(pVarResult);
			}

			case 1: {
				// line 1798: [propput, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([in] VARIANT* newVal);
				CComVariant var0;
				VariantCopy(&var0, pDispParams->rgvarg);
				return put_UserValue(&var0);
			}

			default:
			break;
		}
		break;

		case DISPID_Image:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1800: [propget, id(DISPID_Image), helpstring("property Image")] HRESULT Image([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Image(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 1801: [propput, id(DISPID_Image), helpstring("property Image")] HRESULT Image([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Image(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_POSITION_X:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1803: [propget, id(DISPID_POSITION_X), helpstring("property X")] HRESULT X([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_X(&V_R4(pVarResult));
			}

			case 1: {
				// line 1804: [propput, id(DISPID_POSITION_X), helpstring("property X")] HRESULT X([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_X(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_POSITION_Y:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1805: [propget, id(DISPID_POSITION_Y), helpstring("property Y")] HRESULT Y([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Y(&V_R4(pVarResult));
			}

			case 1: {
				// line 1806: [propput, id(DISPID_POSITION_Y), helpstring("property Y")] HRESULT Y([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Y(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_POSITION_Z:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1807: [propget, id(DISPID_POSITION_Z), helpstring("property Z")] HRESULT Z([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Z(&V_R4(pVarResult));
			}

			case 1: {
				// line 1808: [propput, id(DISPID_POSITION_Z), helpstring("property Z")] HRESULT Z([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Z(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_SIZE_X:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1809: [propget, id(DISPID_SIZE_X), helpstring("property ScaleX")] HRESULT ScaleX([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_ScaleX(&V_R4(pVarResult));
			}

			case 1: {
				// line 1810: [propput, id(DISPID_SIZE_X), helpstring("property ScaleX")] HRESULT ScaleX([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_ScaleX(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_SIZE_Y:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1811: [propget, id(DISPID_SIZE_Y), helpstring("property ScaleY")] HRESULT ScaleY([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_ScaleY(&V_R4(pVarResult));
			}

			case 1: {
				// line 1812: [propput, id(DISPID_SIZE_Y), helpstring("property ScaleY")] HRESULT ScaleY([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_ScaleY(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_SIZE_Z:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1813: [propget, id(DISPID_SIZE_Z), helpstring("property ScaleZ")] HRESULT ScaleZ([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_ScaleZ(&V_R4(pVarResult));
			}

			case 1: {
				// line 1814: [propput, id(DISPID_SIZE_Z), helpstring("property ScaleZ")] HRESULT ScaleZ([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_ScaleZ(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_ROT_Z:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1816: [propget, id(DISPID_ROT_Z), helpstring("property Orientation")] HRESULT Orientation([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Orientation(&V_R4(pVarResult));
			}

			case 1: {
				// line 1817: [propput, id(DISPID_ROT_Z), helpstring("property Orientation")] HRESULT Orientation([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Orientation(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Name:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1819: [propget, id(DISPID_Name), helpstring("property Name")] HRESULT Name([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Name(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 1820: [propput, id(DISPID_Name), helpstring("property Name")] HRESULT Name([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Name(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case 110:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1822: [propget, id(110), helpstring("property Elasticity")] HRESULT Elasticity([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Elasticity(&V_R4(pVarResult));
			}

			case 1: {
				// line 1823: [propput, id(110), helpstring("property Elasticity")] HRESULT Elasticity([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Elasticity(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 112:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1824: [propget, id(112), helpstring("property ElasticityFalloff")] HRESULT ElasticityFalloff([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_ElasticityFalloff(&V_R4(pVarResult));
			}

			case 1: {
				// line 1825: [propput, id(112), helpstring("property ElasticityFalloff")] HRESULT ElasticityFalloff([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_ElasticityFalloff(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 111:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1827: [propget, id(111), helpstring("property Collidable")] HRESULT Collidable([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_Collidable(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1828: [propput, id(111), helpstring("property Collidable")] HRESULT Collidable([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_Collidable(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 34:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1829: [propget, id(34), helpstring("property HasHitEvent")] HRESULT HasHitEvent([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_HasHitEvent(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1830: [propput, id(34), helpstring("property HasHitEvent")] HRESULT HasHitEvent([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_HasHitEvent(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 33:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1831: [propget, id(33), helpstring("property Threshold")] HRESULT Threshold([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Threshold(&V_R4(pVarResult));
			}

			case 1: {
				// line 1832: [propput, id(33), helpstring("property Threshold")] HRESULT Threshold([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Threshold(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 114:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1833: [propget, id(114), helpstring("property Friction")] HRESULT Friction([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Friction(&V_R4(pVarResult));
			}

			case 1: {
				// line 1834: [propput, id(114), helpstring("property Friction")] HRESULT Friction([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Friction(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 115:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1835: [propget, id(115), helpstring("property Scatter")] HRESULT Scatter([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Scatter(&V_R4(pVarResult));
			}

			case 1: {
				// line 1836: [propput, id(115), helpstring("property Scatter")] HRESULT Scatter([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Scatter(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 483:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1837: [propget, id(483), helpstring("property DisableLighting")] HRESULT DisableLighting([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_DisableLighting(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1838: [propput, id(483), helpstring("property DisableLighting")] HRESULT DisableLighting([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_DisableLighting(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 494:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1839: [propget, id(494), helpstring("property BlendDisableLighting")] HRESULT BlendDisableLighting([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_BlendDisableLighting(&V_R4(pVarResult));
			}

			case 1: {
				// line 1840: [propput, id(494), helpstring("property BlendDisableLighting")] HRESULT BlendDisableLighting([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_BlendDisableLighting(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 496:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1841: [propget, id(496), helpstring("property BlendDisableLightingFromBelow")] HRESULT BlendDisableLightingFromBelow([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_BlendDisableLightingFromBelow(&V_R4(pVarResult));
			}

			case 1: {
				// line 1842: [propput, id(496), helpstring("property BlendDisableLightingFromBelow")] HRESULT BlendDisableLightingFromBelow([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_BlendDisableLightingFromBelow(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 431:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1843: [propget, id(431), helpstring("property ReflectionEnabled")] HRESULT ReflectionEnabled([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_ReflectionEnabled(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1844: [propput, id(431), helpstring("property ReflectionEnabled")] HRESULT ReflectionEnabled([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_ReflectionEnabled(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 397:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1846: [propget, id(397), helpstring("property DepthBias")] HRESULT DepthBias([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_DepthBias(&V_R4(pVarResult));
			}

			case 1: {
				// line 1847: [propput, id(397), helpstring("property DepthBias")] HRESULT DepthBias([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_DepthBias(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 377:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1849: [propget, id(377), helpstring("property DropSpeed")] HRESULT DropSpeed([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_DropSpeed(&V_R4(pVarResult));
			}

			case 1: {
				// line 1850: [propput, id(377), helpstring("property DropSpeed")] HRESULT DropSpeed([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_DropSpeed(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 435:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1851: [propget, id(435), helpstring("property IsDropped")] HRESULT IsDropped([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_IsDropped(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1852: [propput, id(435), helpstring("property IsDropped")] HRESULT IsDropped([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_IsDropped(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 9:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1853: [propget, id(9), helpstring("property DrawStyle")] HRESULT DrawStyle([out, retval] TargetType *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_DrawStyle((TargetType*)&V_I4(pVarResult));
			}

			case 1: {
				// line 1854: [propput, id(9), helpstring("property DrawStyle")] HRESULT DrawStyle([in] TargetType newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_DrawStyle((TargetType)V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case 433:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1855: [propget, id(433), helpstring("property LegacyMode")] HRESULT LegacyMode([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_LegacyMode(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1856: [propput, id(433), helpstring("property LegacyMode")] HRESULT LegacyMode([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_LegacyMode(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 726:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1857: [propget, id(726), helpstring("property RaiseDelay")] HRESULT RaiseDelay([out, retval] long *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_RaiseDelay((long*)&V_I4(pVarResult));
			}

			case 1: {
				// line 1858: [propput, id(726), helpstring("property RaiseDelay")] HRESULT RaiseDelay([in] long newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_RaiseDelay(V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case 734:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1859: [propget, id(734), helpstring("property PhysicsMaterial")] HRESULT PhysicsMaterial([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_PhysicsMaterial(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 1860: [propput, id(734), helpstring("property PhysicsMaterial")] HRESULT PhysicsMaterial([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_PhysicsMaterial(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case 432:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1861: [propget, id(432), helpstring("property OverwritePhysics")] HRESULT OverwritePhysics([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_OverwritePhysics(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1862: [propput, id(432), helpstring("property OverwritePhysics")] HRESULT OverwritePhysics([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_OverwritePhysics(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 727: {
			// line 1863: [propget, id(727), helpstring("property HitThreshold")] HRESULT HitThreshold([out, retval] float *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_R4;
			return get_HitThreshold(&V_R4(pVarResult));
		}

		default:
		break;
	}

	return DISP_E_UNKNOWNNAME;
}

robin_hood::unordered_map<int, wstring> HitTarget::m_idNameMap = {
	robin_hood::pair<int, wstring> { DISPID_GameEvents_Init, wstring(L"_Init") },
	robin_hood::pair<int, wstring> { DISPID_TimerEvents_Timer, wstring(L"_Timer") },
	robin_hood::pair<int, wstring> { DISPID_HitEvents_Hit, wstring(L"_Hit") },
	robin_hood::pair<int, wstring> { DISPID_TargetEvents_Dropped, wstring(L"_Dropped") },
	robin_hood::pair<int, wstring> { DISPID_TargetEvents_Raised, wstring(L"_Raised") }
};

HRESULT HitTarget::FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) {
	CComPtr<IDispatch> disp;
	g_pplayer->m_ptable->m_pcv->m_pScript->GetScriptDispatch(nullptr, &disp);

	const robin_hood::unordered_map<int, wstring>::iterator it = m_idNameMap.find(dispid);
	if (it != m_idNameMap.end()) {
		wstring name = wstring(m_wzName) + it->second;
		LPOLESTR fnNames = (LPOLESTR)name.c_str();

		DISPID tDispid;
		const HRESULT hr = disp->GetIDsOfNames(IID_NULL, &fnNames, 1, 0, &tDispid);

		if (hr == S_OK) {
			disp->Invoke(tDispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, pdispparams, nullptr, nullptr, nullptr);
		}
	}

	return S_OK;
}

robin_hood::unordered_map<wstring, int> Gate::m_nameIDMap = {
	robin_hood::pair<wstring, int> { wstring(L"name"), DISPID_Name },
	robin_hood::pair<wstring, int> { wstring(L"timerenabled"), DISPID_Timer_Enabled },
	robin_hood::pair<wstring, int> { wstring(L"timerinterval"), DISPID_Timer_Interval },
	robin_hood::pair<wstring, int> { wstring(L"length"), DISPID_Gate_Length },
	robin_hood::pair<wstring, int> { wstring(L"height"), 1142 },
	robin_hood::pair<wstring, int> { wstring(L"rotation"), DISPID_Gate_Rotation },
	robin_hood::pair<wstring, int> { wstring(L"x"), 5 },
	robin_hood::pair<wstring, int> { wstring(L"y"), 6 },
	robin_hood::pair<wstring, int> { wstring(L"open"), 7 },
	robin_hood::pair<wstring, int> { wstring(L"damping"), 13 },
	robin_hood::pair<wstring, int> { wstring(L"gravityfactor"), 17 },
	robin_hood::pair<wstring, int> { wstring(L"move"), 2147 },
	robin_hood::pair<wstring, int> { wstring(L"material"), 340 },
	robin_hood::pair<wstring, int> { wstring(L"elasticity"), 110 },
	robin_hood::pair<wstring, int> { wstring(L"surface"), DISPID_Surface },
	robin_hood::pair<wstring, int> { wstring(L"uservalue"), DISPID_UserValue },
	robin_hood::pair<wstring, int> { wstring(L"closeangle"), 2144 },
	robin_hood::pair<wstring, int> { wstring(L"openangle"), 2145 },
	robin_hood::pair<wstring, int> { wstring(L"collidable"), 111 },
	robin_hood::pair<wstring, int> { wstring(L"friction"), 114 },
	robin_hood::pair<wstring, int> { wstring(L"visible"), 458 },
	robin_hood::pair<wstring, int> { wstring(L"twoway"), 427 },
	robin_hood::pair<wstring, int> { wstring(L"showbracket"), 15 },
	robin_hood::pair<wstring, int> { wstring(L"currentangle"), 16 },
	robin_hood::pair<wstring, int> { wstring(L"reflectionenabled"), 431 },
	robin_hood::pair<wstring, int> { wstring(L"drawstyle"), 9 }
};

STDMETHODIMP Gate::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	wstring name = wstring(*rgszNames);
	std::transform(name.begin(), name.end(), name.begin(), tolower);
	const robin_hood::unordered_map<wstring, int>::iterator it = m_nameIDMap.find(name);
	if (it != m_nameIDMap.end()) {
		*rgDispId = it->second;
		return S_OK;
	}
	return DISP_E_UNKNOWNNAME;
}

STDMETHODIMP Gate::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
	switch(dispIdMember) {
		case DISPID_Name:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1893: [propget, id(DISPID_Name), helpstring("property Name")] HRESULT Name([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Name(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 1894: [propput, id(DISPID_Name), helpstring("property Name")] HRESULT Name([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Name(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Timer_Enabled:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1895: [propget, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_TimerEnabled(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1896: [propput, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_TimerEnabled(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Timer_Interval:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1897: [propget, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([out, retval] long *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_TimerInterval((long*)&V_I4(pVarResult));
			}

			case 1: {
				// line 1898: [propput, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([in] long newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_TimerInterval(V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Gate_Length:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1899: [propget, id(DISPID_Gate_Length), helpstring("property Length")] HRESULT Length([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Length(&V_R4(pVarResult));
			}

			case 1: {
				// line 1900: [propput, id(DISPID_Gate_Length), helpstring("property Length")] HRESULT Length([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Length(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 1142:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1901: [propget, id(1142), helpstring("property Height")] HRESULT Height([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Height(&V_R4(pVarResult));
			}

			case 1: {
				// line 1902: [propput, id(1142), helpstring("property Height")] HRESULT Height([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Height(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Gate_Rotation:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1903: [propget, id(DISPID_Gate_Rotation), helpstring("property Rotation")] HRESULT Rotation([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Rotation(&V_R4(pVarResult));
			}

			case 1: {
				// line 1904: [propput, id(DISPID_Gate_Rotation), helpstring("property Rotation")] HRESULT Rotation([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Rotation(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 5:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1905: [propget, id(5), helpstring("property X")] HRESULT X([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_X(&V_R4(pVarResult));
			}

			case 1: {
				// line 1906: [propput, id(5), helpstring("property X")] HRESULT X([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_X(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 6:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1907: [propget, id(6), helpstring("property Y")] HRESULT Y([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Y(&V_R4(pVarResult));
			}

			case 1: {
				// line 1908: [propput, id(6), helpstring("property Y")] HRESULT Y([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Y(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 7:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1909: [propget, id(7), helpstring("property Open")] HRESULT Open([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_Open(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1910: [propput, id(7), helpstring("property Open")] HRESULT Open([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_Open(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 13:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1911: [propget, id(13), helpstring("property Damping")] HRESULT Damping([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Damping(&V_R4(pVarResult));
			}

			case 1: {
				// line 1912: [propput, id(13), helpstring("property Damping")] HRESULT Damping([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Damping(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 17:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1913: [propget, id(17), helpstring("property GravityFactor")] HRESULT GravityFactor([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_GravityFactor(&V_R4(pVarResult));
			}

			case 1: {
				// line 1914: [propput, id(17), helpstring("property GravityFactor")] HRESULT GravityFactor([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_GravityFactor(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 2147: {
			// line 1916: [id(2147), helpstring("method Move")] HRESULT Move(int dir, [defaultvalue(0)] float speed, [defaultvalue(0)] float angle);
			int index = pDispParams->cArgs;
			CComVariant var0;
			VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
			CComVariant var1(0);
			VariantChangeType(&var1, (index > 0) ? &pDispParams->rgvarg[--index] : &var1, 0, VT_R4);
			CComVariant var2(0);
			VariantChangeType(&var2, (index > 0) ? &pDispParams->rgvarg[--index] : &var2, 0, VT_R4);
			return Move(V_I4(&var0), V_R4(&var1), V_R4(&var2));
		}

		case 340:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1918: [propget, id(340), helpstring("property Material")] HRESULT Material([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Material(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 1919: [propput, id(340), helpstring("property Material")] HRESULT Material([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Material(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case 110:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1920: [propget, id(110), helpstring("property Elasticity")] HRESULT Elasticity([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Elasticity(&V_R4(pVarResult));
			}

			case 1: {
				// line 1921: [propput, id(110), helpstring("property Elasticity")] HRESULT Elasticity([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Elasticity(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Surface:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1922: [propget, id(DISPID_Surface), helpstring("property Surface")] HRESULT Surface([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Surface(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 1923: [propput, id(DISPID_Surface), helpstring("property Surface")] HRESULT Surface([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Surface(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_UserValue:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1924: [propget, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([out, retval] VARIANT* pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				return get_UserValue(pVarResult);
			}

			case 1: {
				// line 1925: [propput, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([in] VARIANT* newVal);
				CComVariant var0;
				VariantCopy(&var0, pDispParams->rgvarg);
				return put_UserValue(&var0);
			}

			default:
			break;
		}
		break;

		case 2144:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1927: [propget, id(2144), helpstring("property CloseAngle")] HRESULT CloseAngle([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_CloseAngle(&V_R4(pVarResult));
			}

			case 1: {
				// line 1928: [propput, id(2144), helpstring("property CloseAngle")] HRESULT CloseAngle([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_CloseAngle(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 2145:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1929: [propget, id(2145), helpstring("property OpenAngle")] HRESULT OpenAngle([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_OpenAngle(&V_R4(pVarResult));
			}

			case 1: {
				// line 1930: [propput, id(2145), helpstring("property OpenAngle")] HRESULT OpenAngle([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_OpenAngle(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 111:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1931: [propget, id(111), helpstring("property Collidable")] HRESULT Collidable([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_Collidable(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1932: [propput, id(111), helpstring("property Collidable")] HRESULT Collidable([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_Collidable(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 114:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1933: [propget, id(114), helpstring("property Friction")] HRESULT Friction([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Friction(&V_R4(pVarResult));
			}

			case 1: {
				// line 1934: [propput, id(114), helpstring("property Friction")] HRESULT Friction([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Friction(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 458:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1935: [propget, id(458), helpstring("property Visible")] HRESULT Visible([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_Visible(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1936: [propput, id(458), helpstring("property Visible")] HRESULT Visible([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_Visible(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 427:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1937: [propget, id(427), helpstring("property TwoWay")] HRESULT TwoWay([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_TwoWay(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1938: [propput, id(427), helpstring("property TwoWay")] HRESULT TwoWay([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_TwoWay(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 15:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1939: [propget, id(15), helpstring("property ShowBracket")] HRESULT ShowBracket([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_ShowBracket(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1940: [propput, id(15), helpstring("property ShowBracket")] HRESULT ShowBracket([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_ShowBracket(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 16: {
			// line 1941: [propget, id(16), nonbrowsable, helpstring("property CurrentAngle")] HRESULT CurrentAngle([out, retval] float *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_R4;
			return get_CurrentAngle(&V_R4(pVarResult));
		}

		case 431:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1942: [propget, id(431), helpstring("property ReflectionEnabled")] HRESULT ReflectionEnabled([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_ReflectionEnabled(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1943: [propput, id(431), helpstring("property ReflectionEnabled")] HRESULT ReflectionEnabled([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_ReflectionEnabled(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 9:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1944: [propget, id(9), helpstring("property DrawStyle")] HRESULT DrawStyle([out, retval] GateType *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_DrawStyle((GateType*)&V_I4(pVarResult));
			}

			case 1: {
				// line 1945: [propput, id(9), helpstring("property DrawStyle")] HRESULT DrawStyle([in] GateType newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_DrawStyle((GateType)V_I4(&var0));
			}

			default:
			break;
		}
		break;

		default:
		break;
	}

	return DISP_E_UNKNOWNNAME;
}

robin_hood::unordered_map<int, wstring> Gate::m_idNameMap = {
	robin_hood::pair<int, wstring> { DISPID_GameEvents_Init, wstring(L"_Init") },
	robin_hood::pair<int, wstring> { DISPID_TimerEvents_Timer, wstring(L"_Timer") },
	robin_hood::pair<int, wstring> { DISPID_HitEvents_Hit, wstring(L"_Hit") },
	robin_hood::pair<int, wstring> { DISPID_LimitEvents_EOS, wstring(L"_LimitEOS") },
	robin_hood::pair<int, wstring> { DISPID_LimitEvents_BOS, wstring(L"_LimitBOS") }
};

HRESULT Gate::FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) {
	CComPtr<IDispatch> disp;
	g_pplayer->m_ptable->m_pcv->m_pScript->GetScriptDispatch(nullptr, &disp);

	const robin_hood::unordered_map<int, wstring>::iterator it = m_idNameMap.find(dispid);
	if (it != m_idNameMap.end()) {
		wstring name = wstring(m_wzName) + it->second;
		LPOLESTR fnNames = (LPOLESTR)name.c_str();

		DISPID tDispid;
		const HRESULT hr = disp->GetIDsOfNames(IID_NULL, &fnNames, 1, 0, &tDispid);

		if (hr == S_OK) {
			disp->Invoke(tDispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, pdispparams, nullptr, nullptr, nullptr);
		}
	}

	return S_OK;
}

robin_hood::unordered_map<wstring, int> Spinner::m_nameIDMap = {
	robin_hood::pair<wstring, int> { wstring(L"name"), DISPID_Name },
	robin_hood::pair<wstring, int> { wstring(L"timerenabled"), DISPID_Timer_Enabled },
	robin_hood::pair<wstring, int> { wstring(L"timerinterval"), DISPID_Timer_Interval },
	robin_hood::pair<wstring, int> { wstring(L"length"), DISPID_Spinner_Length },
	robin_hood::pair<wstring, int> { wstring(L"rotation"), 4 },
	robin_hood::pair<wstring, int> { wstring(L"height"), 5 },
	robin_hood::pair<wstring, int> { wstring(L"damping"), 7 },
	robin_hood::pair<wstring, int> { wstring(L"image"), DISPID_Image },
	robin_hood::pair<wstring, int> { wstring(L"material"), 340 },
	robin_hood::pair<wstring, int> { wstring(L"x"), 11 },
	robin_hood::pair<wstring, int> { wstring(L"y"), 12 },
	robin_hood::pair<wstring, int> { wstring(L"surface"), DISPID_Surface },
	robin_hood::pair<wstring, int> { wstring(L"uservalue"), DISPID_UserValue },
	robin_hood::pair<wstring, int> { wstring(L"showbracket"), 108 },
	robin_hood::pair<wstring, int> { wstring(L"anglemax"), 13 },
	robin_hood::pair<wstring, int> { wstring(L"anglemin"), 14 },
	robin_hood::pair<wstring, int> { wstring(L"elasticity"), 110 },
	robin_hood::pair<wstring, int> { wstring(L"visible"), 458 },
	robin_hood::pair<wstring, int> { wstring(L"reflectionenabled"), 431 },
	robin_hood::pair<wstring, int> { wstring(L"currentangle"), 18 }
};

STDMETHODIMP Spinner::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	wstring name = wstring(*rgszNames);
	std::transform(name.begin(), name.end(), name.begin(), tolower);
	const robin_hood::unordered_map<wstring, int>::iterator it = m_nameIDMap.find(name);
	if (it != m_nameIDMap.end()) {
		*rgDispId = it->second;
		return S_OK;
	}
	return DISP_E_UNKNOWNNAME;
}

STDMETHODIMP Spinner::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
	switch(dispIdMember) {
		case DISPID_Name:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1975: [propget, id(DISPID_Name), helpstring("property Name")] HRESULT Name([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Name(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 1976: [propput, id(DISPID_Name), helpstring("property Name")] HRESULT Name([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Name(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Timer_Enabled:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1977: [propget, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_TimerEnabled(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 1978: [propput, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_TimerEnabled(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Timer_Interval:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1979: [propget, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([out, retval] long *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_TimerInterval((long*)&V_I4(pVarResult));
			}

			case 1: {
				// line 1980: [propput, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([in] long newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_TimerInterval(V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Spinner_Length:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1981: [propget, id(DISPID_Spinner_Length), helpstring("property Length")] HRESULT Length([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Length(&V_R4(pVarResult));
			}

			case 1: {
				// line 1982: [propput, id(DISPID_Spinner_Length), helpstring("property Length")] HRESULT Length([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Length(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 4:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1983: [propget, id(4), helpstring("property Rotation")] HRESULT Rotation([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Rotation(&V_R4(pVarResult));
			}

			case 1: {
				// line 1984: [propput, id(4), helpstring("property Rotation")] HRESULT Rotation([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Rotation(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 5:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1985: [propget, id(5), helpstring("property Height")] HRESULT Height([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Height(&V_R4(pVarResult));
			}

			case 1: {
				// line 1986: [propput, id(5), helpstring("property Height")] HRESULT Height([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Height(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 7:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1987: [propget, id(7), helpstring("property Damping")] HRESULT Damping([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Damping(&V_R4(pVarResult));
			}

			case 1: {
				// line 1988: [propput, id(7), helpstring("property Damping")] HRESULT Damping([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Damping(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Image:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1989: [propget, id(DISPID_Image), helpstring("property Image")] HRESULT Image([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Image(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 1990: [propput, id(DISPID_Image), helpstring("property Image")] HRESULT Image([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Image(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case 340:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1991: [propget, id(340), helpstring("property Material")] HRESULT Material([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Material(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 1992: [propput, id(340), helpstring("property Material")] HRESULT Material([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Material(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case 11:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1993: [propget, id(11), helpstring("property X")] HRESULT X([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_X(&V_R4(pVarResult));
			}

			case 1: {
				// line 1994: [propput, id(11), helpstring("property X")] HRESULT X([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_X(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 12:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1995: [propget, id(12), helpstring("property Y")] HRESULT Y([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Y(&V_R4(pVarResult));
			}

			case 1: {
				// line 1996: [propput, id(12), helpstring("property Y")] HRESULT Y([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Y(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Surface:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1997: [propget, id(DISPID_Surface), helpstring("property Surface")] HRESULT Surface([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Surface(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 1998: [propput, id(DISPID_Surface), helpstring("property Surface")] HRESULT Surface([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Surface(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_UserValue:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 1999: [propget, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([out, retval] VARIANT* pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				return get_UserValue(pVarResult);
			}

			case 1: {
				// line 2000: [propput, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([in] VARIANT* newVal);
				CComVariant var0;
				VariantCopy(&var0, pDispParams->rgvarg);
				return put_UserValue(&var0);
			}

			default:
			break;
		}
		break;

		case 108:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2001: [propget, id(108), helpstring("property ShowBracket")] HRESULT ShowBracket([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_ShowBracket(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 2002: [propput, id(108), helpstring("property ShowBracket")] HRESULT ShowBracket([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_ShowBracket(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 13:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2004: [propget, id(13), helpstring("property AngleMax")] HRESULT AngleMax([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_AngleMax(&V_R4(pVarResult));
			}

			case 1: {
				// line 2005: [propput, id(13), helpstring("property AngleMax")] HRESULT AngleMax([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_AngleMax(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 14:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2006: [propget, id(14), helpstring("property AngleMin")] HRESULT AngleMin([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_AngleMin(&V_R4(pVarResult));
			}

			case 1: {
				// line 2007: [propput, id(14), helpstring("property AngleMin")] HRESULT AngleMin([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_AngleMin(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 110:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2008: [propget, id(110), helpstring("property Elasticity")] HRESULT Elasticity([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Elasticity(&V_R4(pVarResult));
			}

			case 1: {
				// line 2009: [propput, id(110), helpstring("property Elasticity")] HRESULT Elasticity([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Elasticity(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 458:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2010: [propget, id(458), helpstring("property Visible")] HRESULT Visible([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_Visible(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 2011: [propput, id(458), helpstring("property Visible")] HRESULT Visible([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_Visible(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 431:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2012: [propget, id(431), helpstring("property ReflectionEnabled")] HRESULT ReflectionEnabled([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_ReflectionEnabled(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 2013: [propput, id(431), helpstring("property ReflectionEnabled")] HRESULT ReflectionEnabled([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_ReflectionEnabled(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 18: {
			// line 2014: [propget, id(18), nonbrowsable, helpstring("property CurrentAngle")] HRESULT CurrentAngle([out, retval] float *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_R4;
			return get_CurrentAngle(&V_R4(pVarResult));
		}

		default:
		break;
	}

	return DISP_E_UNKNOWNNAME;
}

robin_hood::unordered_map<int, wstring> Spinner::m_idNameMap = {
	robin_hood::pair<int, wstring> { DISPID_GameEvents_Init, wstring(L"_Init") },
	robin_hood::pair<int, wstring> { DISPID_TimerEvents_Timer, wstring(L"_Timer") },
	robin_hood::pair<int, wstring> { DISPID_SpinnerEvents_Spin, wstring(L"_Spin") },
	robin_hood::pair<int, wstring> { DISPID_LimitEvents_EOS, wstring(L"_LimitEOS") },
	robin_hood::pair<int, wstring> { DISPID_LimitEvents_BOS, wstring(L"_LimitBOS") }
};

HRESULT Spinner::FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) {
	CComPtr<IDispatch> disp;
	g_pplayer->m_ptable->m_pcv->m_pScript->GetScriptDispatch(nullptr, &disp);

	const robin_hood::unordered_map<int, wstring>::iterator it = m_idNameMap.find(dispid);
	if (it != m_idNameMap.end()) {
		wstring name = wstring(m_wzName) + it->second;
		LPOLESTR fnNames = (LPOLESTR)name.c_str();

		DISPID tDispid;
		const HRESULT hr = disp->GetIDsOfNames(IID_NULL, &fnNames, 1, 0, &tDispid);

		if (hr == S_OK) {
			disp->Invoke(tDispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, pdispparams, nullptr, nullptr, nullptr);
		}
	}

	return S_OK;
}

robin_hood::unordered_map<wstring, int> Ramp::m_nameIDMap = {
	robin_hood::pair<wstring, int> { wstring(L"name"), DISPID_Name },
	robin_hood::pair<wstring, int> { wstring(L"timerenabled"), DISPID_Timer_Enabled },
	robin_hood::pair<wstring, int> { wstring(L"timerinterval"), DISPID_Timer_Interval },
	robin_hood::pair<wstring, int> { wstring(L"heightbottom"), 1 },
	robin_hood::pair<wstring, int> { wstring(L"heighttop"), 2 },
	robin_hood::pair<wstring, int> { wstring(L"widthbottom"), 3 },
	robin_hood::pair<wstring, int> { wstring(L"widthtop"), 4 },
	robin_hood::pair<wstring, int> { wstring(L"material"), 340 },
	robin_hood::pair<wstring, int> { wstring(L"type"), 6 },
	robin_hood::pair<wstring, int> { wstring(L"image"), DISPID_Image },
	robin_hood::pair<wstring, int> { wstring(L"imagealignment"), 8 },
	robin_hood::pair<wstring, int> { wstring(L"haswallimage"), 9 },
	robin_hood::pair<wstring, int> { wstring(L"leftwallheight"), 10 },
	robin_hood::pair<wstring, int> { wstring(L"rightwallheight"), 11 },
	robin_hood::pair<wstring, int> { wstring(L"uservalue"), DISPID_UserValue },
	robin_hood::pair<wstring, int> { wstring(L"visibleleftwallheight"), 108 },
	robin_hood::pair<wstring, int> { wstring(L"visiblerightwallheight"), 109 },
	robin_hood::pair<wstring, int> { wstring(L"elasticity"), 110 },
	robin_hood::pair<wstring, int> { wstring(L"collidable"), 111 },
	robin_hood::pair<wstring, int> { wstring(L"hashitevent"), 34 },
	robin_hood::pair<wstring, int> { wstring(L"threshold"), 33 },
	robin_hood::pair<wstring, int> { wstring(L"visible"), 458 },
	robin_hood::pair<wstring, int> { wstring(L"friction"), 114 },
	robin_hood::pair<wstring, int> { wstring(L"scatter"), 115 },
	robin_hood::pair<wstring, int> { wstring(L"depthbias"), 397 },
	robin_hood::pair<wstring, int> { wstring(L"wirediameter"), 377 },
	robin_hood::pair<wstring, int> { wstring(L"wiredistancex"), 398 },
	robin_hood::pair<wstring, int> { wstring(L"wiredistancey"), 425 },
	robin_hood::pair<wstring, int> { wstring(L"reflectionenabled"), 431 },
	robin_hood::pair<wstring, int> { wstring(L"physicsmaterial"), 734 },
	robin_hood::pair<wstring, int> { wstring(L"overwritephysics"), 432 }
};

STDMETHODIMP Ramp::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	wstring name = wstring(*rgszNames);
	std::transform(name.begin(), name.end(), name.begin(), tolower);
	const robin_hood::unordered_map<wstring, int>::iterator it = m_nameIDMap.find(name);
	if (it != m_nameIDMap.end()) {
		*rgDispId = it->second;
		return S_OK;
	}
	return DISP_E_UNKNOWNNAME;
}

STDMETHODIMP Ramp::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
	switch(dispIdMember) {
		case DISPID_Name:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2044: [propget, id(DISPID_Name), helpstring("property Name")] HRESULT Name([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Name(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 2045: [propput, id(DISPID_Name), helpstring("property Name")] HRESULT Name([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Name(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Timer_Enabled:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2046: [propget, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_TimerEnabled(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 2047: [propput, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_TimerEnabled(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Timer_Interval:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2048: [propget, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([out, retval] long *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_TimerInterval((long*)&V_I4(pVarResult));
			}

			case 1: {
				// line 2049: [propput, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([in] long newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_TimerInterval(V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case 1:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2050: [propget, id(1), helpstring("property HeightBottom")] HRESULT HeightBottom([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_HeightBottom(&V_R4(pVarResult));
			}

			case 1: {
				// line 2051: [propput, id(1), helpstring("property HeightBottom")] HRESULT HeightBottom([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_HeightBottom(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 2:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2052: [propget, id(2), helpstring("property HeightTop")] HRESULT HeightTop([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_HeightTop(&V_R4(pVarResult));
			}

			case 1: {
				// line 2053: [propput, id(2), helpstring("property HeightTop")] HRESULT HeightTop([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_HeightTop(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 3:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2054: [propget, id(3), helpstring("property WidthBottom")] HRESULT WidthBottom([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_WidthBottom(&V_R4(pVarResult));
			}

			case 1: {
				// line 2055: [propput, id(3), helpstring("property WidthBottom")] HRESULT WidthBottom([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_WidthBottom(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 4:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2056: [propget, id(4), helpstring("property WidthTop")] HRESULT WidthTop([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_WidthTop(&V_R4(pVarResult));
			}

			case 1: {
				// line 2057: [propput, id(4), helpstring("property WidthTop")] HRESULT WidthTop([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_WidthTop(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 340:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2058: [propget, id(340), helpstring("property Material")] HRESULT Material([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Material(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 2059: [propput, id(340), helpstring("property Material")] HRESULT Material([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Material(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case 6:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2060: [propget, id(6), helpstring("property Type")] HRESULT Type([out, retval] RampType *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_Type((RampType*)&V_I4(pVarResult));
			}

			case 1: {
				// line 2061: [propput, id(6), helpstring("property Type")] HRESULT Type([in] RampType newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_Type((RampType)V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Image:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2062: [propget, id(DISPID_Image), helpstring("property Image")] HRESULT Image([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Image(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 2063: [propput, id(DISPID_Image), helpstring("property Image")] HRESULT Image([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Image(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case 8:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2064: [propget, id(8), helpstring("property ImageAlignment")] HRESULT ImageAlignment([out, retval] RampImageAlignment *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_ImageAlignment((RampImageAlignment*)&V_I4(pVarResult));
			}

			case 1: {
				// line 2065: [propput, id(8), helpstring("property ImageAlignment")] HRESULT ImageAlignment([in] RampImageAlignment newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_ImageAlignment((RampImageAlignment)V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case 9:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2066: [propget, id(9), helpstring("property HasWallImage")] HRESULT HasWallImage([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_HasWallImage(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 2067: [propput, id(9), helpstring("property HasWallImage")] HRESULT HasWallImage([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_HasWallImage(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 10:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2068: [propget, id(10), helpstring("property LeftWallHeight")] HRESULT LeftWallHeight([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_LeftWallHeight(&V_R4(pVarResult));
			}

			case 1: {
				// line 2069: [propput, id(10), helpstring("property LeftWallHeight")] HRESULT LeftWallHeight([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_LeftWallHeight(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 11:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2070: [propget, id(11), helpstring("property RightWallHeight")] HRESULT RightWallHeight([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_RightWallHeight(&V_R4(pVarResult));
			}

			case 1: {
				// line 2071: [propput, id(11), helpstring("property RightWallHeight")] HRESULT RightWallHeight([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_RightWallHeight(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_UserValue:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2072: [propget, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([out, retval] VARIANT* pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				return get_UserValue(pVarResult);
			}

			case 1: {
				// line 2073: [propput, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([in] VARIANT* newVal);
				CComVariant var0;
				VariantCopy(&var0, pDispParams->rgvarg);
				return put_UserValue(&var0);
			}

			default:
			break;
		}
		break;

		case 108:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2074: [propget, id(108), helpstring("property VisibleLeftWallHeight")] HRESULT VisibleLeftWallHeight([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_VisibleLeftWallHeight(&V_R4(pVarResult));
			}

			case 1: {
				// line 2075: [propput, id(108), helpstring("property VisibleLeftWallHeight")] HRESULT VisibleLeftWallHeight([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_VisibleLeftWallHeight(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 109:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2076: [propget, id(109), helpstring("property VisibleRightWallHeight")] HRESULT VisibleRightWallHeight([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_VisibleRightWallHeight(&V_R4(pVarResult));
			}

			case 1: {
				// line 2077: [propput, id(109), helpstring("property VisibleRightWallHeight")] HRESULT VisibleRightWallHeight([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_VisibleRightWallHeight(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 110:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2078: [propget, id(110), helpstring("property Elasticity")] HRESULT Elasticity([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Elasticity(&V_R4(pVarResult));
			}

			case 1: {
				// line 2079: [propput, id(110), helpstring("property Elasticity")] HRESULT Elasticity([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Elasticity(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 111:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2081: [propget, id(111), helpstring("property Collidable")] HRESULT Collidable([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_Collidable(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 2082: [propput, id(111), helpstring("property Collidable")] HRESULT Collidable([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_Collidable(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 34:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2083: [propget, id(34), helpstring("property HasHitEvent")] HRESULT HasHitEvent([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_HasHitEvent(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 2084: [propput, id(34), helpstring("property HasHitEvent")] HRESULT HasHitEvent([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_HasHitEvent(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 33:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2085: [propget, id(33), helpstring("property Threshold")] HRESULT Threshold([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Threshold(&V_R4(pVarResult));
			}

			case 1: {
				// line 2086: [propput, id(33), helpstring("property Threshold")] HRESULT Threshold([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Threshold(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 458:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2088: [propget, id(458), helpstring("property Visible")] HRESULT Visible([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_Visible(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 2089: [propput, id(458), helpstring("property Visible")] HRESULT Visible([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_Visible(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 114:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2091: [propget, id(114), helpstring("property Friction")] HRESULT Friction([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Friction(&V_R4(pVarResult));
			}

			case 1: {
				// line 2092: [propput, id(114), helpstring("property Friction")] HRESULT Friction([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Friction(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 115:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2093: [propget, id(115), helpstring("property Scatter")] HRESULT Scatter([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Scatter(&V_R4(pVarResult));
			}

			case 1: {
				// line 2094: [propput, id(115), helpstring("property Scatter")] HRESULT Scatter([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Scatter(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 397:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2096: [propget, id(397), helpstring("property DepthBias")] HRESULT DepthBias([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_DepthBias(&V_R4(pVarResult));
			}

			case 1: {
				// line 2097: [propput, id(397), helpstring("property DepthBias")] HRESULT DepthBias([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_DepthBias(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 377:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2099: [propget, id(377), helpstring("property WireDiameter")] HRESULT WireDiameter([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_WireDiameter(&V_R4(pVarResult));
			}

			case 1: {
				// line 2100: [propput, id(377), helpstring("property WireDiameter")] HRESULT WireDiameter([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_WireDiameter(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 398:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2102: [propget, id(398), helpstring("property WireDistanceX")] HRESULT WireDistanceX([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_WireDistanceX(&V_R4(pVarResult));
			}

			case 1: {
				// line 2103: [propput, id(398), helpstring("property WireDistanceX")] HRESULT WireDistanceX([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_WireDistanceX(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 425:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2105: [propget, id(425), helpstring("property WireDistanceY")] HRESULT WireDistanceY([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_WireDistanceY(&V_R4(pVarResult));
			}

			case 1: {
				// line 2106: [propput, id(425), helpstring("property WireDistanceY")] HRESULT WireDistanceY([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_WireDistanceY(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 431:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2108: [propget, id(431), helpstring("property ReflectionEnabled")] HRESULT ReflectionEnabled([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_ReflectionEnabled(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 2109: [propput, id(431), helpstring("property ReflectionEnabled")] HRESULT ReflectionEnabled([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_ReflectionEnabled(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 734:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2111: [propget, id(734), helpstring("property PhysicsMaterial")] HRESULT PhysicsMaterial([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_PhysicsMaterial(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 2112: [propput, id(734), helpstring("property PhysicsMaterial")] HRESULT PhysicsMaterial([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_PhysicsMaterial(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case 432:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2113: [propget, id(432), helpstring("property OverwritePhysics")] HRESULT OverwritePhysics([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_OverwritePhysics(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 2114: [propput, id(432), helpstring("property OverwritePhysics")] HRESULT OverwritePhysics([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_OverwritePhysics(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		default:
		break;
	}

	return DISP_E_UNKNOWNNAME;
}

robin_hood::unordered_map<wstring, int> Flasher::m_nameIDMap = {
	robin_hood::pair<wstring, int> { wstring(L"name"), DISPID_Name },
	robin_hood::pair<wstring, int> { wstring(L"x"), 5 },
	robin_hood::pair<wstring, int> { wstring(L"y"), 6 },
	robin_hood::pair<wstring, int> { wstring(L"imagealignment"), 8 },
	robin_hood::pair<wstring, int> { wstring(L"height"), 378 },
	robin_hood::pair<wstring, int> { wstring(L"rotz"), 1 },
	robin_hood::pair<wstring, int> { wstring(L"roty"), 2 },
	robin_hood::pair<wstring, int> { wstring(L"rotx"), 9 },
	robin_hood::pair<wstring, int> { wstring(L"color"), 11 },
	robin_hood::pair<wstring, int> { wstring(L"imagea"), DISPID_Image },
	robin_hood::pair<wstring, int> { wstring(L"imageb"), DISPID_Image2 },
	robin_hood::pair<wstring, int> { wstring(L"displaytexture"), 13 },
	robin_hood::pair<wstring, int> { wstring(L"opacity"), 377 },
	robin_hood::pair<wstring, int> { wstring(L"intensityscale"), 435 },
	robin_hood::pair<wstring, int> { wstring(L"modulatevsadd"), 433 },
	robin_hood::pair<wstring, int> { wstring(L"uservalue"), DISPID_UserValue },
	robin_hood::pair<wstring, int> { wstring(L"visible"), 458 },
	robin_hood::pair<wstring, int> { wstring(L"addblend"), 556 },
	robin_hood::pair<wstring, int> { wstring(L"dmd"), 557 },
	robin_hood::pair<wstring, int> { wstring(L"videocapwidth"), 560 },
	robin_hood::pair<wstring, int> { wstring(L"videocapheight"), 561 },
	robin_hood::pair<wstring, int> { wstring(L"videocapupdate"), 562 },
	robin_hood::pair<wstring, int> { wstring(L"depthbias"), 397 },
	robin_hood::pair<wstring, int> { wstring(L"filter"), 32996 },
	robin_hood::pair<wstring, int> { wstring(L"amount"), 379 },
	robin_hood::pair<wstring, int> { wstring(L"timerenabled"), DISPID_Timer_Enabled },
	robin_hood::pair<wstring, int> { wstring(L"timerinterval"), DISPID_Timer_Interval }
};

STDMETHODIMP Flasher::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	wstring name = wstring(*rgszNames);
	std::transform(name.begin(), name.end(), name.begin(), tolower);
	const robin_hood::unordered_map<wstring, int>::iterator it = m_nameIDMap.find(name);
	if (it != m_nameIDMap.end()) {
		*rgDispId = it->second;
		return S_OK;
	}
	return DISP_E_UNKNOWNNAME;
}

STDMETHODIMP Flasher::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
	switch(dispIdMember) {
		case DISPID_Name:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2130: [propget, id(DISPID_Name), helpstring("property Name")] HRESULT Name([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Name(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 2131: [propput, id(DISPID_Name), helpstring("property Name")] HRESULT Name([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Name(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case 5:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2132: [propget, id(5), helpstring("property X")] HRESULT X([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_X(&V_R4(pVarResult));
			}

			case 1: {
				// line 2133: [propput, id(5), helpstring("property X")] HRESULT X([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_X(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 6:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2134: [propget, id(6), helpstring("property Y")] HRESULT Y([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Y(&V_R4(pVarResult));
			}

			case 1: {
				// line 2135: [propput, id(6), helpstring("property Y")] HRESULT Y([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Y(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 8:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2136: [propget, id(8), helpstring("property ImageAlignment")] HRESULT ImageAlignment([out, retval] RampImageAlignment *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_ImageAlignment((RampImageAlignment*)&V_I4(pVarResult));
			}

			case 1: {
				// line 2137: [propput, id(8), helpstring("property ImageAlignment")] HRESULT ImageAlignment([in] RampImageAlignment newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_ImageAlignment((RampImageAlignment)V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case 378:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2138: [propget, id(378), helpstring("property Height")] HRESULT Height([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Height(&V_R4(pVarResult));
			}

			case 1: {
				// line 2139: [propput, id(378), helpstring("property Height")] HRESULT Height([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Height(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 1:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2140: [propget, id(1), helpstring("property RotZ")] HRESULT RotZ([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_RotZ(&V_R4(pVarResult));
			}

			case 1: {
				// line 2141: [propput, id(1), helpstring("property RotZ")] HRESULT RotZ([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_RotZ(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 2:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2142: [propget, id(2), helpstring("property RotY")] HRESULT RotY([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_RotY(&V_R4(pVarResult));
			}

			case 1: {
				// line 2143: [propput, id(2), helpstring("property RotY")] HRESULT RotY([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_RotY(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 9:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2144: [propget, id(9), helpstring("property RotX")] HRESULT RotX([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_RotX(&V_R4(pVarResult));
			}

			case 1: {
				// line 2145: [propput, id(9), helpstring("property RotX")] HRESULT RotX([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_RotX(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 11:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2146: [propget, id(11), helpstring("property Color")] HRESULT Color([out, retval] OLE_COLOR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_UI4;
				return get_Color(&V_UI4(pVarResult));
			}

			case 1: {
				// line 2147: [propput, id(11), helpstring("property Color")] HRESULT Color([in] OLE_COLOR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_UI4);
				return put_Color((OLE_COLOR)V_UI4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Image:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2148: [propget, id(DISPID_Image), helpstring("property ImageA")] HRESULT ImageA([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_ImageA(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 2149: [propput, id(DISPID_Image), helpstring("property ImageA")] HRESULT ImageA([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_ImageA(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Image2:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2150: [propget, id(DISPID_Image2), helpstring("property ImageB")] HRESULT ImageB([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_ImageB(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 2151: [propput, id(DISPID_Image2), helpstring("property ImageB")] HRESULT ImageB([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_ImageB(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case 13:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2152: [propget, id(13), helpstring("property DisplayTexture")] HRESULT DisplayTexture([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_DisplayTexture(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 2153: [propput, id(13), helpstring("property DisplayTexture")] HRESULT DisplayTexture([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_DisplayTexture(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 377:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2154: [propget, id(377), helpstring("property Opacity")] HRESULT Opacity([out, retval] long *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_Opacity((long*)&V_I4(pVarResult));
			}

			case 1: {
				// line 2155: [propput, id(377), helpstring("property Opacity")] HRESULT Opacity([in] long newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_Opacity(V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case 435:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2156: [propget, id(435), helpstring("property IntensityScale")] HRESULT IntensityScale([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_IntensityScale(&V_R4(pVarResult));
			}

			case 1: {
				// line 2157: [propput, id(435), helpstring("property IntensityScale")] HRESULT IntensityScale([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_IntensityScale(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 433:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2158: [propget, id(433), helpstring("property ModulateVsAdd")] HRESULT ModulateVsAdd([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_ModulateVsAdd(&V_R4(pVarResult));
			}

			case 1: {
				// line 2159: [propput, id(433), helpstring("property ModulateVsAdd")] HRESULT ModulateVsAdd([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_ModulateVsAdd(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_UserValue:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2160: [propget, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([out, retval] VARIANT* pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				return get_UserValue(pVarResult);
			}

			case 1: {
				// line 2161: [propput, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([in] VARIANT* newVal);
				CComVariant var0;
				VariantCopy(&var0, pDispParams->rgvarg);
				return put_UserValue(&var0);
			}

			default:
			break;
		}
		break;

		case 458:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2163: [propget, id(458), helpstring("property Visible")] HRESULT Visible([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_Visible(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 2164: [propput, id(458), helpstring("property Visible")] HRESULT Visible([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_Visible(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 556:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2165: [propget, id(556), helpstring("property AddBlend")] HRESULT AddBlend([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_AddBlend(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 2166: [propput, id(556), helpstring("property AddBlend")] HRESULT AddBlend([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_AddBlend(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 557:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2167: [propget, id(557), helpstring("property DMD")] HRESULT DMD([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_DMD(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 2168: [propput, id(557), helpstring("property DMD")] HRESULT DMD([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_DMD(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 560: {
			// line 2170: [propput, id(560), helpstring("property VideoCapWidth")] HRESULT VideoCapWidth([in] long cWidth);
			CComVariant var0;
			VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
			return put_VideoCapWidth(V_I4(&var0));
		}

		case 561: {
			// line 2171: [propput, id(561), helpstring("property VideoCapHeight")] HRESULT VideoCapHeight([in] long cHeight);
			CComVariant var0;
			VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
			return put_VideoCapHeight(V_I4(&var0));
		}

		case 562: {
			// line 2172: [propput, id(562), helpstring("property VideoCapUpdate")] HRESULT VideoCapUpdate([in] BSTR cWinTitle);
			CComVariant var0;
			VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
			return put_VideoCapUpdate(V_BSTR(&var0));
		}

		case 397:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2174: [propget, id(397), helpstring("property DepthBias")] HRESULT DepthBias([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_DepthBias(&V_R4(pVarResult));
			}

			case 1: {
				// line 2175: [propput, id(397), helpstring("property DepthBias")] HRESULT DepthBias([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_DepthBias(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 32996:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2176: [propget, id(32996), helpstring("property Filter")] HRESULT Filter([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Filter(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 2177: [propput, id(32996), helpstring("property Filter")] HRESULT Filter([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Filter(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case 379:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2178: [propget, id(379), helpstring("property Amount")] HRESULT Amount([out, retval] long *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_Amount((long*)&V_I4(pVarResult));
			}

			case 1: {
				// line 2179: [propput, id(379), helpstring("property Amount")] HRESULT Amount([in] long newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_Amount(V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Timer_Enabled:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2180: [propget, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_TimerEnabled(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 2181: [propput, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_TimerEnabled(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Timer_Interval:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2182: [propget, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([out, retval] long *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_TimerInterval((long*)&V_I4(pVarResult));
			}

			case 1: {
				// line 2183: [propput, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([in] long newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_TimerInterval(V_I4(&var0));
			}

			default:
			break;
		}
		break;

		default:
		break;
	}

	return DISP_E_UNKNOWNNAME;
}

robin_hood::unordered_map<wstring, int> Rubber::m_nameIDMap = {
	robin_hood::pair<wstring, int> { wstring(L"name"), DISPID_Name },
	robin_hood::pair<wstring, int> { wstring(L"height"), 2 },
	robin_hood::pair<wstring, int> { wstring(L"hitheight"), 116 },
	robin_hood::pair<wstring, int> { wstring(L"hashitevent"), 34 },
	robin_hood::pair<wstring, int> { wstring(L"thickness"), 4 },
	robin_hood::pair<wstring, int> { wstring(L"rotx"), 18 },
	robin_hood::pair<wstring, int> { wstring(L"rotz"), 25 },
	robin_hood::pair<wstring, int> { wstring(L"roty"), 24 },
	robin_hood::pair<wstring, int> { wstring(L"material"), 340 },
	robin_hood::pair<wstring, int> { wstring(L"image"), DISPID_Image },
	robin_hood::pair<wstring, int> { wstring(L"elasticity"), 110 },
	robin_hood::pair<wstring, int> { wstring(L"elasticityfalloff"), 120 },
	robin_hood::pair<wstring, int> { wstring(L"collidable"), 111 },
	robin_hood::pair<wstring, int> { wstring(L"visible"), 458 },
	robin_hood::pair<wstring, int> { wstring(L"friction"), 114 },
	robin_hood::pair<wstring, int> { wstring(L"scatter"), 115 },
	robin_hood::pair<wstring, int> { wstring(L"enablestaticrendering"), 398 },
	robin_hood::pair<wstring, int> { wstring(L"enableshowineditor"), 479 },
	robin_hood::pair<wstring, int> { wstring(L"reflectionenabled"), 431 },
	robin_hood::pair<wstring, int> { wstring(L"timerenabled"), DISPID_Timer_Enabled },
	robin_hood::pair<wstring, int> { wstring(L"timerinterval"), DISPID_Timer_Interval },
	robin_hood::pair<wstring, int> { wstring(L"uservalue"), DISPID_UserValue },
	robin_hood::pair<wstring, int> { wstring(L"physicsmaterial"), 734 },
	robin_hood::pair<wstring, int> { wstring(L"overwritephysics"), 432 }
};

STDMETHODIMP Rubber::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	wstring name = wstring(*rgszNames);
	std::transform(name.begin(), name.end(), name.begin(), tolower);
	const robin_hood::unordered_map<wstring, int>::iterator it = m_nameIDMap.find(name);
	if (it != m_nameIDMap.end()) {
		*rgDispId = it->second;
		return S_OK;
	}
	return DISP_E_UNKNOWNNAME;
}

STDMETHODIMP Rubber::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
	switch(dispIdMember) {
		case DISPID_Name:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2199: [propget, id(DISPID_Name), helpstring("property Name")] HRESULT Name([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Name(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 2200: [propput, id(DISPID_Name), helpstring("property Name")] HRESULT Name([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Name(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case 2:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2201: [propget, id(2), helpstring("property Height")] HRESULT Height([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Height(&V_R4(pVarResult));
			}

			case 1: {
				// line 2202: [propput, id(2), helpstring("property Height")] HRESULT Height([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Height(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 116:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2203: [propget, id(116), helpstring("property HitHeight")] HRESULT HitHeight([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_HitHeight(&V_R4(pVarResult));
			}

			case 1: {
				// line 2204: [propput, id(116), helpstring("property HitHeight")] HRESULT HitHeight([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_HitHeight(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 34:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2205: [propget, id(34), helpstring("property HitEvent")] HRESULT HasHitEvent([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_HasHitEvent(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 2206: [propput, id(34), helpstring("property HitEvent")] HRESULT HasHitEvent([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_HasHitEvent(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 4:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2207: [propget, id(4), helpstring("property Thickness")] HRESULT Thickness([out, retval] int *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_Thickness(&V_I4(pVarResult));
			}

			case 1: {
				// line 2208: [propput, id(4), helpstring("property Thickness")] HRESULT Thickness([in] int newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_Thickness(V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case 18:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2209: [propget, id(18), helpstring("property RotX")] HRESULT RotX([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_RotX(&V_R4(pVarResult));
			}

			case 1: {
				// line 2210: [propput, id(18), helpstring("property RotX")] HRESULT RotX([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_RotX(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 25:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2211: [propget, id(25), helpstring("property RotZ")] HRESULT RotZ([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_RotZ(&V_R4(pVarResult));
			}

			case 1: {
				// line 2212: [propput, id(25), helpstring("property RotZ")] HRESULT RotZ([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_RotZ(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 24:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2213: [propget, id(24), helpstring("property RotY")] HRESULT RotY([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_RotY(&V_R4(pVarResult));
			}

			case 1: {
				// line 2214: [propput, id(24), helpstring("property RotY")] HRESULT RotY([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_RotY(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 340:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2215: [propget, id(340), helpstring("property Material")] HRESULT Material([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Material(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 2216: [propput, id(340), helpstring("property Material")] HRESULT Material([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Material(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Image:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2217: [propget, id(DISPID_Image), helpstring("property Image")] HRESULT Image([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Image(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 2218: [propput, id(DISPID_Image), helpstring("property Image")] HRESULT Image([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Image(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case 110:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2219: [propget, id(110), helpstring("property Elasticity")] HRESULT Elasticity([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Elasticity(&V_R4(pVarResult));
			}

			case 1: {
				// line 2220: [propput, id(110), helpstring("property Elasticity")] HRESULT Elasticity([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Elasticity(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 120:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2221: [propget, id(120), helpstring("property ElasticityFalloff")] HRESULT ElasticityFalloff([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_ElasticityFalloff(&V_R4(pVarResult));
			}

			case 1: {
				// line 2222: [propput, id(120), helpstring("property ElasticityFalloff")] HRESULT ElasticityFalloff([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_ElasticityFalloff(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 111:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2224: [propget, id(111), helpstring("property Collidable")] HRESULT Collidable([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_Collidable(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 2225: [propput, id(111), helpstring("property Collidable")] HRESULT Collidable([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_Collidable(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 458:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2227: [propget, id(458), helpstring("property Visible")] HRESULT Visible([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_Visible(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 2228: [propput, id(458), helpstring("property Visible")] HRESULT Visible([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_Visible(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 114:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2230: [propget, id(114), helpstring("property Friction")] HRESULT Friction([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Friction(&V_R4(pVarResult));
			}

			case 1: {
				// line 2231: [propput, id(114), helpstring("property Friction")] HRESULT Friction([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Friction(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 115:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2232: [propget, id(115), helpstring("property Scatter")] HRESULT Scatter([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Scatter(&V_R4(pVarResult));
			}

			case 1: {
				// line 2233: [propput, id(115), helpstring("property Scatter")] HRESULT Scatter([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Scatter(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 398:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2235: [propget, id(398), helpstring("property static rendering")] HRESULT EnableStaticRendering([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_EnableStaticRendering(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 2236: [propput, id(398), helpstring("property static rendering")] HRESULT EnableStaticRendering([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_EnableStaticRendering(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 479:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2238: [propget, id(479), helpstring("property show in editor")] HRESULT EnableShowInEditor([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_EnableShowInEditor(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 2239: [propput, id(479), helpstring("property show in editor")] HRESULT EnableShowInEditor([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_EnableShowInEditor(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 431:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2241: [propget, id(431), helpstring("property ReflectionEnabled")] HRESULT ReflectionEnabled([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_ReflectionEnabled(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 2242: [propput, id(431), helpstring("property ReflectionEnabled")] HRESULT ReflectionEnabled([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_ReflectionEnabled(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Timer_Enabled:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2244: [propget, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_TimerEnabled(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 2245: [propput, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_TimerEnabled(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Timer_Interval:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2246: [propget, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([out, retval] long *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_TimerInterval((long*)&V_I4(pVarResult));
			}

			case 1: {
				// line 2247: [propput, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([in] long newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_TimerInterval(V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_UserValue:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2248: [propget, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([out, retval] VARIANT* pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				return get_UserValue(pVarResult);
			}

			case 1: {
				// line 2249: [propput, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([in] VARIANT* newVal);
				CComVariant var0;
				VariantCopy(&var0, pDispParams->rgvarg);
				return put_UserValue(&var0);
			}

			default:
			break;
		}
		break;

		case 734:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2251: [propget, id(734), helpstring("property PhysicsMaterial")] HRESULT PhysicsMaterial([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_PhysicsMaterial(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 2252: [propput, id(734), helpstring("property PhysicsMaterial")] HRESULT PhysicsMaterial([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_PhysicsMaterial(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case 432:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2253: [propget, id(432), helpstring("property OverwritePhysics")] HRESULT OverwritePhysics([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_OverwritePhysics(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 2254: [propput, id(432), helpstring("property OverwritePhysics")] HRESULT OverwritePhysics([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_OverwritePhysics(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		default:
		break;
	}

	return DISP_E_UNKNOWNNAME;
}

robin_hood::unordered_map<wstring, int> BallEx::m_nameIDMap = {
	robin_hood::pair<wstring, int> { wstring(L"x"), 1 },
	robin_hood::pair<wstring, int> { wstring(L"y"), 2 },
	robin_hood::pair<wstring, int> { wstring(L"velx"), 3 },
	robin_hood::pair<wstring, int> { wstring(L"vely"), 4 },
	robin_hood::pair<wstring, int> { wstring(L"z"), 5 },
	robin_hood::pair<wstring, int> { wstring(L"velz"), 6 },
	robin_hood::pair<wstring, int> { wstring(L"angvelx"), 14 },
	robin_hood::pair<wstring, int> { wstring(L"angvely"), 15 },
	robin_hood::pair<wstring, int> { wstring(L"angvelz"), 16 },
	robin_hood::pair<wstring, int> { wstring(L"angmomx"), 17 },
	robin_hood::pair<wstring, int> { wstring(L"angmomy"), 18 },
	robin_hood::pair<wstring, int> { wstring(L"angmomz"), 19 },
	robin_hood::pair<wstring, int> { wstring(L"color"), 7 },
	robin_hood::pair<wstring, int> { wstring(L"image"), 8 },
	robin_hood::pair<wstring, int> { wstring(L"frontdecal"), 9 },
	robin_hood::pair<wstring, int> { wstring(L"decalmode"), 497 },
	robin_hood::pair<wstring, int> { wstring(L"uservalue"), DISPID_UserValue },
	robin_hood::pair<wstring, int> { wstring(L"mass"), 11 },
	robin_hood::pair<wstring, int> { wstring(L"radius"), 12 },
	robin_hood::pair<wstring, int> { wstring(L"id"), 13 },
	robin_hood::pair<wstring, int> { wstring(L"name"), DISPID_Name },
	robin_hood::pair<wstring, int> { wstring(L"bulbintensityscale"), 451 },
	robin_hood::pair<wstring, int> { wstring(L"reflectionenabled"), 484 },
	robin_hood::pair<wstring, int> { wstring(L"playfieldreflectionscale"), 485 },
	robin_hood::pair<wstring, int> { wstring(L"forcereflection"), 486 },
	robin_hood::pair<wstring, int> { wstring(L"visible"), 487 },
	robin_hood::pair<wstring, int> { wstring(L"destroyball"), 100 }
};

STDMETHODIMP BallEx::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	wstring name = wstring(*rgszNames);
	std::transform(name.begin(), name.end(), name.begin(), tolower);
	const robin_hood::unordered_map<wstring, int>::iterator it = m_nameIDMap.find(name);
	if (it != m_nameIDMap.end()) {
		*rgDispId = it->second;
		return S_OK;
	}
	return DISP_E_UNKNOWNNAME;
}

STDMETHODIMP BallEx::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
	switch(dispIdMember) {
		case 1:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2270: [propget, id(1), helpstring("property X")] HRESULT X([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_X(&V_R4(pVarResult));
			}

			case 1: {
				// line 2271: [propput, id(1), helpstring("property X")] HRESULT X([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_X(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 2:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2272: [propget, id(2), helpstring("property Y")] HRESULT Y([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Y(&V_R4(pVarResult));
			}

			case 1: {
				// line 2273: [propput, id(2), helpstring("property Y")] HRESULT Y([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Y(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 3:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2274: [propget, id(3), helpstring("property VelX")] HRESULT VelX([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_VelX(&V_R4(pVarResult));
			}

			case 1: {
				// line 2275: [propput, id(3), helpstring("property VelX")] HRESULT VelX([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_VelX(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 4:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2276: [propget, id(4), helpstring("property VelY")] HRESULT VelY([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_VelY(&V_R4(pVarResult));
			}

			case 1: {
				// line 2277: [propput, id(4), helpstring("property VelY")] HRESULT VelY([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_VelY(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 5:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2278: [propget, id(5), helpstring("property Z")] HRESULT Z([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Z(&V_R4(pVarResult));
			}

			case 1: {
				// line 2279: [propput, id(5), helpstring("property Z")] HRESULT Z([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Z(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 6:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2280: [propget, id(6), helpstring("property VelZ")] HRESULT VelZ([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_VelZ(&V_R4(pVarResult));
			}

			case 1: {
				// line 2281: [propput, id(6), helpstring("property VelZ")] HRESULT VelZ([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_VelZ(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 14:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2282: [propget, id(14), helpstring("property AngVelX")] HRESULT AngVelX([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_AngVelX(&V_R4(pVarResult));
			}

			case 1: {
				// line 2283: [propput, id(14), helpstring("property AngVelX")] HRESULT AngVelX([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_AngVelX(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 15:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2284: [propget, id(15), helpstring("property AngVelY")] HRESULT AngVelY([out, retval] float* pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_AngVelY(&V_R4(pVarResult));
			}

			case 1: {
				// line 2285: [propput, id(15), helpstring("property AngVelY")] HRESULT AngVelY([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_AngVelY(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 16:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2286: [propget, id(16), helpstring("property AngVelZ")] HRESULT AngVelZ([out, retval] float* pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_AngVelZ(&V_R4(pVarResult));
			}

			case 1: {
				// line 2287: [propput, id(16), helpstring("property AngVelZ")] HRESULT AngVelZ([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_AngVelZ(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 17:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2288: [propget, id(17), helpstring("property AngMomX")] HRESULT AngMomX([out, retval] float* pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_AngMomX(&V_R4(pVarResult));
			}

			case 1: {
				// line 2289: [propput, id(17), helpstring("property AngMomX")] HRESULT AngMomX([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_AngMomX(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 18:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2290: [propget, id(18), helpstring("property AngMomY")] HRESULT AngMomY([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_AngMomY(&V_R4(pVarResult));
			}

			case 1: {
				// line 2291: [propput, id(18), helpstring("property AngMomY")] HRESULT AngMomY([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_AngMomY(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 19:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2292: [propget, id(19), helpstring("property AngMomZ")] HRESULT AngMomZ([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_AngMomZ(&V_R4(pVarResult));
			}

			case 1: {
				// line 2293: [propput, id(19), helpstring("property AngMomZ")] HRESULT AngMomZ([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_AngMomZ(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 7:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2295: [propget, id(7), helpstring("property Color")] HRESULT Color([out, retval] OLE_COLOR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_UI4;
				return get_Color(&V_UI4(pVarResult));
			}

			case 1: {
				// line 2296: [propput, id(7), helpstring("property Color")] HRESULT Color([in] OLE_COLOR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_UI4);
				return put_Color((OLE_COLOR)V_UI4(&var0));
			}

			default:
			break;
		}
		break;

		case 8:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2297: [propget, id(8), helpstring("property Image")] HRESULT Image([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Image(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 2298: [propput, id(8), helpstring("property Image")] HRESULT Image([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Image(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case 9:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2299: [propget, id(9), helpstring("property FrontDecal")] HRESULT FrontDecal([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_FrontDecal(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 2300: [propput, id(9), helpstring("property FrontDecal")] HRESULT FrontDecal([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_FrontDecal(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case 497:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2301: [propget, id(497), helpstring("property DecalMode")] HRESULT DecalMode([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_DecalMode(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 2302: [propput, id(497), helpstring("property DecalMode")] HRESULT DecalMode([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_DecalMode(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_UserValue:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2303: [propget, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([out, retval] VARIANT* pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				return get_UserValue(pVarResult);
			}

			case 1: {
				// line 2304: [propput, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([in] VARIANT* newVal);
				CComVariant var0;
				VariantCopy(&var0, pDispParams->rgvarg);
				return put_UserValue(&var0);
			}

			default:
			break;
		}
		break;

		case 11:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2305: [propget, id(11), helpstring("property Mass")] HRESULT Mass([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Mass(&V_R4(pVarResult));
			}

			case 1: {
				// line 2306: [propput, id(11), helpstring("property Mass")] HRESULT Mass([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Mass(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 12:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2307: [propget, id(12), helpstring("property Radius")] HRESULT Radius([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Radius(&V_R4(pVarResult));
			}

			case 1: {
				// line 2308: [propput, id(12), helpstring("property Radius")] HRESULT Radius([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Radius(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 13:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2309: [propget, id(13), helpstring("property ID")] HRESULT ID([out, retval] int *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_ID(&V_I4(pVarResult));
			}

			case 1: {
				// line 2310: [propput, id(13), helpstring("property ID")] HRESULT ID([in] int newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_ID(V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Name:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2311: [propget, id(DISPID_Name), helpstring("property Name")] HRESULT Name([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Name(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 2312: [propput, id(DISPID_Name), helpstring("property Name")] HRESULT Name([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Name(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case 451:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2313: [propget, id(451), helpstring("property BulbIntensityScale")] HRESULT BulbIntensityScale([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_BulbIntensityScale(&V_R4(pVarResult));
			}

			case 1: {
				// line 2314: [propput, id(451), helpstring("property BulbIntensityScale")] HRESULT BulbIntensityScale([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_BulbIntensityScale(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 484:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2315: [propget, id(484), helpstring("property ReflectionEnabled")] HRESULT ReflectionEnabled([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_ReflectionEnabled(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 2316: [propput, id(484), helpstring("property ReflectionEnabled")] HRESULT ReflectionEnabled([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_ReflectionEnabled(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 485:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2317: [propget, id(485), helpstring("property PlayfieldReflectionScale")] HRESULT PlayfieldReflectionScale([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_PlayfieldReflectionScale(&V_R4(pVarResult));
			}

			case 1: {
				// line 2318: [propput, id(485), helpstring("property PlayfieldReflectionScale")] HRESULT PlayfieldReflectionScale([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_PlayfieldReflectionScale(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 486:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2319: [propget, id(486), helpstring("property ForceReflection")] HRESULT ForceReflection([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_ForceReflection(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 2320: [propput, id(486), helpstring("property ForceReflection")] HRESULT ForceReflection([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_ForceReflection(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 487:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2321: [propget, id(487), helpstring("property Visible")] HRESULT Visible([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_Visible(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 2322: [propput, id(487), helpstring("property Visible")] HRESULT Visible([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_Visible(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 100: {
			// line 2323: [id(100), helpstring("method DestroyBall")] HRESULT DestroyBall([out, retval] int *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_I4;
			return DestroyBall(&V_I4(pVarResult));
		}

		default:
		break;
	}

	return DISP_E_UNKNOWNNAME;
}

robin_hood::unordered_map<int, wstring> Ramp::m_idNameMap = {
	robin_hood::pair<int, wstring> { DISPID_GameEvents_Init, wstring(L"_Init") }
};

HRESULT Ramp::FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) {
	CComPtr<IDispatch> disp;
	g_pplayer->m_ptable->m_pcv->m_pScript->GetScriptDispatch(nullptr, &disp);

	const robin_hood::unordered_map<int, wstring>::iterator it = m_idNameMap.find(dispid);
	if (it != m_idNameMap.end()) {
		wstring name = wstring(m_wzName) + it->second;
		LPOLESTR fnNames = (LPOLESTR)name.c_str();

		DISPID tDispid;
		const HRESULT hr = disp->GetIDsOfNames(IID_NULL, &fnNames, 1, 0, &tDispid);

		if (hr == S_OK) {
			disp->Invoke(tDispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, pdispparams, nullptr, nullptr, nullptr);
		}
	}

	return S_OK;
}

robin_hood::unordered_map<int, wstring> Flasher::m_idNameMap = {
	robin_hood::pair<int, wstring> { DISPID_GameEvents_Init, wstring(L"_Init") },
	robin_hood::pair<int, wstring> { DISPID_TimerEvents_Timer, wstring(L"_Timer") }
};

HRESULT Flasher::FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) {
	CComPtr<IDispatch> disp;
	g_pplayer->m_ptable->m_pcv->m_pScript->GetScriptDispatch(nullptr, &disp);

	const robin_hood::unordered_map<int, wstring>::iterator it = m_idNameMap.find(dispid);
	if (it != m_idNameMap.end()) {
		wstring name = wstring(m_wzName) + it->second;
		LPOLESTR fnNames = (LPOLESTR)name.c_str();

		DISPID tDispid;
		const HRESULT hr = disp->GetIDsOfNames(IID_NULL, &fnNames, 1, 0, &tDispid);

		if (hr == S_OK) {
			disp->Invoke(tDispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, pdispparams, nullptr, nullptr, nullptr);
		}
	}

	return S_OK;
}

robin_hood::unordered_map<int, wstring> Rubber::m_idNameMap = {
	robin_hood::pair<int, wstring> { DISPID_GameEvents_Init, wstring(L"_Init") },
	robin_hood::pair<int, wstring> { DISPID_HitEvents_Hit, wstring(L"_Hit") },
	robin_hood::pair<int, wstring> { DISPID_TimerEvents_Timer, wstring(L"_Timer") }
};

HRESULT Rubber::FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) {
	CComPtr<IDispatch> disp;
	g_pplayer->m_ptable->m_pcv->m_pScript->GetScriptDispatch(nullptr, &disp);

	const robin_hood::unordered_map<int, wstring>::iterator it = m_idNameMap.find(dispid);
	if (it != m_idNameMap.end()) {
		wstring name = wstring(m_wzName) + it->second;
		LPOLESTR fnNames = (LPOLESTR)name.c_str();

		DISPID tDispid;
		const HRESULT hr = disp->GetIDsOfNames(IID_NULL, &fnNames, 1, 0, &tDispid);

		if (hr == S_OK) {
			disp->Invoke(tDispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, pdispparams, nullptr, nullptr, nullptr);
		}
	}

	return S_OK;
}

robin_hood::unordered_map<wstring, int> DispReel::m_nameIDMap = {
	robin_hood::pair<wstring, int> { wstring(L"backcolor"), DISPID_DispReel_BackColor },
	robin_hood::pair<wstring, int> { wstring(L"image"), DISPID_Image },
	robin_hood::pair<wstring, int> { wstring(L"reels"), 11 },
	robin_hood::pair<wstring, int> { wstring(L"width"), 5 },
	robin_hood::pair<wstring, int> { wstring(L"height"), 6 },
	robin_hood::pair<wstring, int> { wstring(L"spacing"), 7 },
	robin_hood::pair<wstring, int> { wstring(L"istransparent"), 12 },
	robin_hood::pair<wstring, int> { wstring(L"sound"), DISPID_Sound },
	robin_hood::pair<wstring, int> { wstring(L"steps"), 8 },
	robin_hood::pair<wstring, int> { wstring(L"timerenabled"), DISPID_Timer_Enabled },
	robin_hood::pair<wstring, int> { wstring(L"timerinterval"), DISPID_Timer_Interval },
	robin_hood::pair<wstring, int> { wstring(L"x"), 9 },
	robin_hood::pair<wstring, int> { wstring(L"y"), 10 },
	robin_hood::pair<wstring, int> { wstring(L"range"), 14 },
	robin_hood::pair<wstring, int> { wstring(L"name"), DISPID_Name },
	robin_hood::pair<wstring, int> { wstring(L"updateinterval"), 15 },
	robin_hood::pair<wstring, int> { wstring(L"uservalue"), DISPID_UserValue },
	robin_hood::pair<wstring, int> { wstring(L"useimagegrid"), 17 },
	robin_hood::pair<wstring, int> { wstring(L"visible"), 458 },
	robin_hood::pair<wstring, int> { wstring(L"imagespergridrow"), 18 },
	robin_hood::pair<wstring, int> { wstring(L"addvalue"), 30 },
	robin_hood::pair<wstring, int> { wstring(L"resettozero"), 31 },
	robin_hood::pair<wstring, int> { wstring(L"spinreel"), 32 },
	robin_hood::pair<wstring, int> { wstring(L"setvalue"), 33 }
};

STDMETHODIMP DispReel::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	wstring name = wstring(*rgszNames);
	std::transform(name.begin(), name.end(), name.begin(), tolower);
	const robin_hood::unordered_map<wstring, int>::iterator it = m_nameIDMap.find(name);
	if (it != m_nameIDMap.end()) {
		*rgDispId = it->second;
		return S_OK;
	}
	return DISP_E_UNKNOWNNAME;
}

STDMETHODIMP DispReel::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
	switch(dispIdMember) {
		case DISPID_DispReel_BackColor:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2373: [propget, id(DISPID_DispReel_BackColor), helpstring("property BackColor")] HRESULT BackColor([out, retval] OLE_COLOR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_UI4;
				return get_BackColor(&V_UI4(pVarResult));
			}

			case 1: {
				// line 2374: [propput, id(DISPID_DispReel_BackColor), helpstring("property BackColor")] HRESULT BackColor([in] OLE_COLOR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_UI4);
				return put_BackColor((OLE_COLOR)V_UI4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Image:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2375: [propget, id(DISPID_Image), helpstring("property Image")] HRESULT Image([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Image(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 2376: [propput, id(DISPID_Image), helpstring("property Image")] HRESULT Image([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Image(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case 11:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2377: [propget, id(11), helpstring("property Reels")] HRESULT Reels([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Reels(&V_R4(pVarResult));
			}

			case 1: {
				// line 2378: [propput, id(11), helpstring("property Reels")] HRESULT Reels([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Reels(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 5:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2379: [propget, id(5), helpstring("property Width")] HRESULT Width([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Width(&V_R4(pVarResult));
			}

			case 1: {
				// line 2380: [propput, id(5), helpstring("property Width")] HRESULT Width([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Width(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 6:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2381: [propget, id(6), helpstring("property Height")] HRESULT Height([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Height(&V_R4(pVarResult));
			}

			case 1: {
				// line 2382: [propput, id(6), helpstring("property Height")] HRESULT Height([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Height(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 7:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2383: [propget, id(7), helpstring("property Spacing")] HRESULT Spacing([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Spacing(&V_R4(pVarResult));
			}

			case 1: {
				// line 2384: [propput, id(7), helpstring("property Spacing")] HRESULT Spacing([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Spacing(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 12:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2385: [propget, id(12), helpstring("property IsTransparent")] HRESULT IsTransparent([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_IsTransparent(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 2386: [propput, id(12), helpstring("property IsTransparent")] HRESULT IsTransparent([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_IsTransparent(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Sound:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2387: [propget, id(DISPID_Sound), helpstring("property Sound")] HRESULT Sound([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Sound(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 2388: [propput, id(DISPID_Sound), helpstring("property Sound")] HRESULT Sound([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Sound(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case 8:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2389: [propget, id(8), helpstring("property Steps")] HRESULT Steps([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Steps(&V_R4(pVarResult));
			}

			case 1: {
				// line 2390: [propput, id(8), helpstring("property Steps")] HRESULT Steps([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Steps(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Timer_Enabled:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2391: [propget, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_TimerEnabled(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 2392: [propput, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_TimerEnabled(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Timer_Interval:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2393: [propget, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([out, retval] long *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_TimerInterval((long*)&V_I4(pVarResult));
			}

			case 1: {
				// line 2394: [propput, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([in] long newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_TimerInterval(V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case 9:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2395: [propget, id(9), helpstring("property X")] HRESULT X([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_X(&V_R4(pVarResult));
			}

			case 1: {
				// line 2396: [propput, id(9), helpstring("property X")] HRESULT X([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_X(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 10:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2397: [propget, id(10), helpstring("property Y")] HRESULT Y([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Y(&V_R4(pVarResult));
			}

			case 1: {
				// line 2398: [propput, id(10), helpstring("property Y")] HRESULT Y([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Y(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 14:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2399: [propget, id(14), helpstring("property Range")] HRESULT Range([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_Range(&V_R4(pVarResult));
			}

			case 1: {
				// line 2400: [propput, id(14), helpstring("property Range")] HRESULT Range([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_Range(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Name:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2401: [propget, id(DISPID_Name), helpstring("property Name")] HRESULT Name([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Name(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 2402: [propput, id(DISPID_Name), helpstring("property Name")] HRESULT Name([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Name(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case 15:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2403: [propget, id(15), helpstring("property UpdateInterval")] HRESULT UpdateInterval([out, retval] long *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_UpdateInterval((long*)&V_I4(pVarResult));
			}

			case 1: {
				// line 2404: [propput, id(15), helpstring("property UpdateInterval")] HRESULT UpdateInterval([in] long newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_UpdateInterval(V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_UserValue:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2405: [propget, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([out, retval] VARIANT* pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				return get_UserValue(pVarResult);
			}

			case 1: {
				// line 2406: [propput, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([in] VARIANT* newVal);
				CComVariant var0;
				VariantCopy(&var0, pDispParams->rgvarg);
				return put_UserValue(&var0);
			}

			default:
			break;
		}
		break;

		case 17:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2407: [propget, id(17), helpstring("property UseImageGrid")] HRESULT UseImageGrid([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_UseImageGrid(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 2408: [propput, id(17), helpstring("property UseImageGrid")] HRESULT UseImageGrid([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_UseImageGrid(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 458:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2409: [propget, id(458), helpstring("property Visible")] HRESULT Visible([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_Visible(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 2410: [propput, id(458), helpstring("property Visible")] HRESULT Visible([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_Visible(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 18:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2411: [propget, id(18), helpstring("property ImagesPerGridRow")] HRESULT ImagesPerGridRow([out, retval] long *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_ImagesPerGridRow((long*)&V_I4(pVarResult));
			}

			case 1: {
				// line 2412: [propput, id(18), helpstring("property ImagesPerGridRow")] HRESULT ImagesPerGridRow([in] long newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_ImagesPerGridRow(V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case 30: {
			// line 2414: [id(30), helpstring("method AddValue")] HRESULT AddValue([in] long Value);
			CComVariant var0;
			VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
			return AddValue(V_I4(&var0));
		}

		case 31: {
			// line 2415: [id(31), helpstring("method ResetToZero")] HRESULT ResetToZero();
			return ResetToZero();
		}

		case 32: {
			// line 2416: [id(32), helpstring("method SpinReel")] HRESULT SpinReel([in] long ReelNumber, [in] long PulseCount);
			int index = pDispParams->cArgs;
			CComVariant var0;
			VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
			CComVariant var1;
			VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_I4);
			return SpinReel(V_I4(&var0), V_I4(&var1));
		}

		case 33: {
			// line 2417: [id(33), helpstring("method SetValue")] HRESULT SetValue([in] long Value);
			CComVariant var0;
			VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
			return SetValue(V_I4(&var0));
		}

		default:
		break;
	}

	return DISP_E_UNKNOWNNAME;
}

robin_hood::unordered_map<int, wstring> DispReel::m_idNameMap = {
	robin_hood::pair<int, wstring> { DISPID_GameEvents_Init, wstring(L"_Init") },
	robin_hood::pair<int, wstring> { DISPID_TimerEvents_Timer, wstring(L"_Timer") }
};

HRESULT DispReel::FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) {
	CComPtr<IDispatch> disp;
	g_pplayer->m_ptable->m_pcv->m_pScript->GetScriptDispatch(nullptr, &disp);

	const robin_hood::unordered_map<int, wstring>::iterator it = m_idNameMap.find(dispid);
	if (it != m_idNameMap.end()) {
		wstring name = wstring(m_wzName) + it->second;
		LPOLESTR fnNames = (LPOLESTR)name.c_str();

		DISPID tDispid;
		const HRESULT hr = disp->GetIDsOfNames(IID_NULL, &fnNames, 1, 0, &tDispid);

		if (hr == S_OK) {
			disp->Invoke(tDispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, pdispparams, nullptr, nullptr, nullptr);
		}
	}

	return S_OK;
}

robin_hood::unordered_map<wstring, int> LightSeq::m_nameIDMap = {
	robin_hood::pair<wstring, int> { wstring(L"name"), DISPID_Name },
	robin_hood::pair<wstring, int> { wstring(L"collection"), DISPID_Collection },
	robin_hood::pair<wstring, int> { wstring(L"centerx"), 9 },
	robin_hood::pair<wstring, int> { wstring(L"centery"), 10 },
	robin_hood::pair<wstring, int> { wstring(L"updateinterval"), 15 },
	robin_hood::pair<wstring, int> { wstring(L"timerenabled"), DISPID_Timer_Enabled },
	robin_hood::pair<wstring, int> { wstring(L"timerinterval"), DISPID_Timer_Interval },
	robin_hood::pair<wstring, int> { wstring(L"uservalue"), DISPID_UserValue },
	robin_hood::pair<wstring, int> { wstring(L"play"), 32 },
	robin_hood::pair<wstring, int> { wstring(L"stopplay"), 33 }
};

STDMETHODIMP LightSeq::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	wstring name = wstring(*rgszNames);
	std::transform(name.begin(), name.end(), name.begin(), tolower);
	const robin_hood::unordered_map<wstring, int>::iterator it = m_nameIDMap.find(name);
	if (it != m_nameIDMap.end()) {
		*rgDispId = it->second;
		return S_OK;
	}
	return DISP_E_UNKNOWNNAME;
}

STDMETHODIMP LightSeq::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
	switch(dispIdMember) {
		case DISPID_Name:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2440: [propget, id(DISPID_Name), helpstring("property Name")] HRESULT Name([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Name(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 2441: [propput, id(DISPID_Name), helpstring("property Name")] HRESULT Name([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Name(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Collection:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2442: [propget, id(DISPID_Collection), helpstring("property Collection")] HRESULT Collection([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_Collection(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 2443: [propput, id(DISPID_Collection), helpstring("property Collection")] HRESULT Collection([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_Collection(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case 9:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2444: [propget, id(9), helpstring("property CenterX")] HRESULT CenterX([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_CenterX(&V_R4(pVarResult));
			}

			case 1: {
				// line 2445: [propput, id(9), helpstring("property CenterX")] HRESULT CenterX([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_CenterX(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 10:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2446: [propget, id(10), helpstring("property CenterY")] HRESULT CenterY([out, retval] float *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_R4;
				return get_CenterY(&V_R4(pVarResult));
			}

			case 1: {
				// line 2447: [propput, id(10), helpstring("property CenterY")] HRESULT CenterY([in] float newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_R4);
				return put_CenterY(V_R4(&var0));
			}

			default:
			break;
		}
		break;

		case 15:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2448: [propget, id(15), helpstring("property UpdateInterval")] HRESULT UpdateInterval([out, retval] long *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_UpdateInterval((long*)&V_I4(pVarResult));
			}

			case 1: {
				// line 2449: [propput, id(15), helpstring("property UpdateInterval")] HRESULT UpdateInterval([in] long newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_UpdateInterval(V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Timer_Enabled:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2450: [propget, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_TimerEnabled(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 2451: [propput, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_TimerEnabled(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_Timer_Interval:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2452: [propget, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([out, retval] long *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_TimerInterval((long*)&V_I4(pVarResult));
			}

			case 1: {
				// line 2453: [propput, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([in] long newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_TimerInterval(V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case DISPID_UserValue:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 2454: [propget, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([out, retval] VARIANT* pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				return get_UserValue(pVarResult);
			}

			case 1: {
				// line 2455: [propput, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([in] VARIANT* newVal);
				CComVariant var0;
				VariantCopy(&var0, pDispParams->rgvarg);
				return put_UserValue(&var0);
			}

			default:
			break;
		}
		break;

		case 32: {
			// line 2456: [id(32), helpstring("method Play")] HRESULT Play([in] SequencerState Animation, [defaultvalue(0)] long TailLength, [defaultvalue(1)] long Repeat, [defaultvalue(0)] long Pause);
			int index = pDispParams->cArgs;
			CComVariant var0;
			VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
			CComVariant var1(0);
			VariantChangeType(&var1, (index > 0) ? &pDispParams->rgvarg[--index] : &var1, 0, VT_I4);
			CComVariant var2(1);
			VariantChangeType(&var2, (index > 0) ? &pDispParams->rgvarg[--index] : &var2, 0, VT_I4);
			CComVariant var3(0);
			VariantChangeType(&var3, (index > 0) ? &pDispParams->rgvarg[--index] : &var3, 0, VT_I4);
			return Play((SequencerState)V_I4(&var0), V_I4(&var1), V_I4(&var2), V_I4(&var3));
		}

		case 33: {
			// line 2457: [id(33), helpstring("method StopPlay")] HRESULT StopPlay();
			return StopPlay();
		}

		default:
		break;
	}

	return DISP_E_UNKNOWNNAME;
}

robin_hood::unordered_map<int, wstring> LightSeq::m_idNameMap = {
	robin_hood::pair<int, wstring> { DISPID_GameEvents_Init, wstring(L"_Init") },
	robin_hood::pair<int, wstring> { DISPID_TimerEvents_Timer, wstring(L"_Timer") },
	robin_hood::pair<int, wstring> { DISPID_LightSeqEvents_PlayDone, wstring(L"_PlayDone") }
};

HRESULT LightSeq::FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) {
	CComPtr<IDispatch> disp;
	g_pplayer->m_ptable->m_pcv->m_pScript->GetScriptDispatch(nullptr, &disp);

	const robin_hood::unordered_map<int, wstring>::iterator it = m_idNameMap.find(dispid);
	if (it != m_idNameMap.end()) {
		wstring name = wstring(m_wzName) + it->second;
		LPOLESTR fnNames = (LPOLESTR)name.c_str();

		DISPID tDispid;
		const HRESULT hr = disp->GetIDsOfNames(IID_NULL, &fnNames, 1, 0, &tDispid);

		if (hr == S_OK) {
			disp->Invoke(tDispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, pdispparams, nullptr, nullptr, nullptr);
		}
	}

	return S_OK;
}

