#include "stdafx.h"
#include "olectl.h"

STDMETHODIMP Collection::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} namesIdsList[] = {
			{ NULL },
			{ L"Count", 8000 }
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

STDMETHODIMP Collection::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
	int index = pDispParams->cArgs;
	VARIANT res;
	HRESULT hres = DISP_E_UNKNOWNNAME;

	V_VT(&res) = VT_EMPTY;

	switch(dispIdMember) {
		case DISPID_VALUE: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 455: [propget, id(0),helpcontext(0x5009003)]HRESULT Item([in] long Index, [out, retval] IDispatch **ppobject);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				V_VT(&res) = VT_DISPATCH;
				hres = get_Item(V_I4(&var0), (IDispatch**)&V_DISPATCH(&res));
				VariantClear(&var0);
			}
			else if (wFlags == (DISPATCH_METHOD | DISPATCH_PROPERTYGET)) {
				V_VT(&res) = VT_DISPATCH;
				V_DISPATCH(&res) = this;
				hres = S_OK;
			}
			break;
		}
		case DISPID_NEWENUM: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 447: [propget, id(DISPID_NEWENUM),helpcontext(0x5009002)]HRESULT _NewEnum([out, retval] IUnknown** ppunk);
				V_VT(&res) = VT_UNKNOWN;
				hres = get__NewEnum(&V_UNKNOWN(&res));
			}
			break;
		}
		case 8000: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 459: [propget,helpcontext(0x5009004)]HRESULT Count([out, retval] long *Count);
				V_VT(&res) = VT_I4;
				hres = get_Count((long*)&V_I4(&res));
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

STDMETHODIMP Collection::GetDocumentation(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile) {
	if (index == MEMBERID_NIL) {
		*pBstrName = SysAllocString(L"Collection");
		return S_OK;
	}
	return E_NOTIMPL;
}

HRESULT Collection::FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) {
	static struct {
		DISPID dispId;
		const WCHAR *name;
	} idsNamesList[] = {
			{ NULL },
			{ DISPID_GameEvents_Init, L"_Init" },
			{ DISPID_SurfaceEvents_Slingshot, L"_Slingshot" },
			{ DISPID_TimerEvents_Timer, L"_Timer" },
			{ DISPID_SpinnerEvents_Spin, L"_Spin" },
			{ DISPID_TargetEvents_Dropped, L"_Dropped" },
			{ DISPID_TargetEvents_Raised, L"_Raised" },
			{ DISPID_HitEvents_Hit, L"_Hit" },
			{ DISPID_HitEvents_Unhit, L"_Unhit" }
	};

	static WCHAR wzName[MAXSTRING];
	size_t min = 1, max = ARRAY_SIZE(idsNamesList) - 1, i;
	int r;
	while(min <= max) {
		i = (min + max) / 2;
		if (idsNamesList[i].dispId == dispid) {
			wcscpy(wzName, m_wzName);
			wcscat(wzName, idsNamesList[i].name);
			LPOLESTR fnNames = (LPOLESTR)wzName;
			DISPID tDispid;
			CComPtr<IDispatch> disp;
			g_pplayer->m_ptable->m_pcv->m_pScript->GetScriptDispatch(nullptr, &disp);
			if (SUCCEEDED(disp->GetIDsOfNames(IID_NULL, &fnNames, 1, 0, &tDispid))) {
				return disp->Invoke(tDispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, pdispparams, nullptr, nullptr, nullptr);
			}
			return DISP_E_MEMBERNOTFOUND;
		}
		else if (idsNamesList[i].dispId < dispid)
		   min = i+1;
		else
		   max = i-1;
	}
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP PinTable::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} namesIdsList[] = {
			{ NULL },
			{ L"Accelerometer", 200 },
			{ L"AccelerometerAngle", 202 },
			{ L"AccelNormalMount", 201 },
			{ L"AOScale", 568 },
			{ L"BackdropColor", 5 },
			{ L"BackdropImage_DT", DISPID_Image2 },
			{ L"BackdropImage_FS", DISPID_Image6 },
			{ L"BackdropImage_FSS", DISPID_Image8 },
			{ L"BackdropImageApplyNightDay", 459 },
			{ L"BackglassMode", 1714 },
			{ L"BallDecalMode", 438 },
			{ L"BallFrontDecal", DISPID_Image4 },
			{ L"BallImage", DISPID_Image3 },
			{ L"BallPlayfieldReflectionScale", 1712 },
			{ L"BallReflection", 1700 },
			{ L"BallTrail", 1704 },
			{ L"BloomStrength", 450 },
			{ L"ColorGradeImage", DISPID_Image5 },
			{ L"DeadZone", 217 },
			{ L"DefaultBulbIntensityScale", 1713 },
			{ L"DefaultScatter", 1102 },
			{ L"DetailLevel", 420 },
			{ L"Elasticity", 1708 },
			{ L"ElasticityFalloff", 1709 },
			{ L"EnableAntialiasing", 394 },
			{ L"EnableAO", 396 },
			{ L"EnableDecals", 13433 },
			{ L"EnableEMReels", 13432 },
			{ L"EnableFXAA", 395 },
			{ L"EnableSSR", 590 },
			{ L"EnvironmentEmissionScale", 566 },
			{ L"EnvironmentImage", DISPID_Image7 },
			{ L"FieldOfView", DISPID_Table_FieldOfView },
			{ L"FileName", 1711 },
			{ L"Friction", 1101 },
			{ L"GlassHeight", 3 },
			{ L"GlobalAlphaAcc", 398 },
			{ L"GlobalDayNight", 588 },
			{ L"GlobalDifficulty", 209 },
			{ L"GlobalStereo3D", 427 },
			{ L"Gravity", 1100 },
			{ L"Height", DISPID_Table_Height },
			{ L"Image", DISPID_Image },
			{ L"Inclination", DISPID_Table_Inclination },
			{ L"Layback", DISPID_Table_Layback },
			{ L"Light0Emission", 559 },
			{ L"LightAmbient", 558 },
			{ L"LightEmissionScale", 567 },
			{ L"LightHeight", 564 },
			{ L"LightRange", 565 },
			{ L"MaxSeparation", DISPID_Table_MaxSeparation },
			{ L"Name", DISPID_Name },
			{ L"NightDay", 436 },
			{ L"NudgeTime", 1103 },
			{ L"Offset", DISPID_Table_Offset },
			{ L"Option", 230 },
			{ L"OverridePhysics", DISPID_Table_OverridePhysics },
			{ L"OverridePhysicsFlippers", 584 },
			{ L"PhysicsLoopTime", 1105 },
			{ L"PlayfieldMaterial", 340 },
			{ L"PlayfieldReflectionStrength", 1707 },
			{ L"PlungerFilter", 1107 },
			{ L"PlungerNormalize", 1104 },
			{ L"ReflectElementsOnPlayfield", 431 },
			{ L"Rotation", 99 },
			{ L"Scalex", 100 },
			{ L"Scaley", 101 },
			{ L"Scalez", 108 },
			{ L"Scatter", 1710 },
			{ L"ShowDT", 13434 },
			{ L"ShowFSS", 625 },
			{ L"SlopeMax", 215 },
			{ L"SlopeMin", 6 },
			{ L"SSRScale", 569 },
			{ L"TableAdaptiveVSync", 585 },
			{ L"TableHeight", 214 },
			{ L"TableMusicVolume", 580 },
			{ L"TableSoundVolume", 579 },
			{ L"TrailStrength", 1705 },
			{ L"Version", 219 },
			{ L"VersionMajor", 38 },
			{ L"VersionMinor", 39 },
			{ L"VersionRevision", 40 },
			{ L"VPBuildVersion", 24 },
			{ L"Width", DISPID_Table_Width },
			{ L"Xlatex", 102 },
			{ L"Xlatey", 103 },
			{ L"Xlatez", 110 },
			{ L"YieldTime", 7 },
			{ L"ZPD", DISPID_Table_ZPD }
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

STDMETHODIMP PinTable::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
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
		case 3: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 494: [propget, id(3), helpstring("property GlassHeight")] HRESULT GlassHeight([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_GlassHeight(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 495: [propput, id(3), helpstring("property GlassHeight")] HRESULT GlassHeight([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_GlassHeight(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 340: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 496: [propget, id(340), helpstring("property PlayfieldMaterial")] HRESULT PlayfieldMaterial([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_PlayfieldMaterial(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 497: [propput, id(340), helpstring("property PlayfieldMaterial")] HRESULT PlayfieldMaterial([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_PlayfieldMaterial(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 5: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 498: [propget, id(5), helpstring("property BackdropColor")] HRESULT BackdropColor([out, retval] OLE_COLOR *pVal);
				V_VT(&res) = VT_UI4;
				hres = get_BackdropColor(&V_UI4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 499: [propput, id(5), helpstring("property BackdropColor")] HRESULT BackdropColor([in] OLE_COLOR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_UI4);
				hres = put_BackdropColor((OLE_COLOR)V_UI4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 215: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 500: [propget, id(215), helpstring("property SlopeMax")] HRESULT SlopeMax([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_SlopeMax(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 501: [propput, id(215), helpstring("property SlopeMax")] HRESULT SlopeMax([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_SlopeMax(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 6: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 502: [propget, id(6), helpstring("property SlopeMin")] HRESULT SlopeMin([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_SlopeMin(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 503: [propput, id(6), helpstring("property SlopeMin")] HRESULT SlopeMin([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_SlopeMin(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Table_Inclination: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 505: [propget, id(DISPID_Table_Inclination), helpstring("property Inclination")] HRESULT Inclination([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Inclination(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 506: [propput, id(DISPID_Table_Inclination), helpstring("property Inclination")] HRESULT Inclination([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Inclination(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Table_FieldOfView: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 507: [propget, id(DISPID_Table_FieldOfView), helpstring("property FieldOfView")] HRESULT FieldOfView([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_FieldOfView(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 508: [propput, id(DISPID_Table_FieldOfView), helpstring("property FieldOfView")] HRESULT FieldOfView([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_FieldOfView(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Table_Layback: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 509: [propget, id(DISPID_Table_Layback), helpstring("property Layback")] HRESULT Layback([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Layback(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 510: [propput, id(DISPID_Table_Layback), helpstring("property Layback")] HRESULT Layback([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Layback(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 99: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 511: [propget, id(99), helpstring("property Rotation")] HRESULT Rotation([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Rotation(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 512: [propput, id(99), helpstring("property Rotation")] HRESULT Rotation([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Rotation(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 100: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 513: [propget, id(100), helpstring("property Scalex")] HRESULT Scalex([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Scalex(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 514: [propput, id(100), helpstring("property Scalex")] HRESULT Scalex([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Scalex(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 101: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 515: [propget, id(101), helpstring("property Scaley")] HRESULT Scaley([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Scaley(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 516: [propput, id(101), helpstring("property Scaley")] HRESULT Scaley([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Scaley(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 108: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 517: [propget, id(108), helpstring("property Scalez")] HRESULT Scalez([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Scalez(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 518: [propput, id(108), helpstring("property Scalez")] HRESULT Scalez([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Scalez(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 102: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 519: [propget, id(102), helpstring("property Xlatex")] HRESULT Xlatex([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Xlatex(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 520: [propput, id(102), helpstring("property Xlatex")] HRESULT Xlatex([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Xlatex(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 103: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 521: [propget, id(103), helpstring("property Xlatey")] HRESULT Xlatey([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Xlatey(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 522: [propput, id(103), helpstring("property Xlatey")] HRESULT Xlatey([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Xlatey(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 110: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 523: [propget, id(110), helpstring("property Xlatez")] HRESULT Xlatez([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Xlatez(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 524: [propput, id(110), helpstring("property Xlatez")] HRESULT Xlatez([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Xlatez(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 1100: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 526: [propget, id(1100), helpstring("property Gravity")] HRESULT Gravity([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Gravity(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 527: [propput, id(1100), helpstring("property Gravity")] HRESULT Gravity([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Gravity(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 1101: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 528: [propget, id(1101), helpstring("property Friction")] HRESULT Friction([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Friction(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 529: [propput, id(1101), helpstring("property Friction")] HRESULT Friction([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Friction(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 1708: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 530: [propget, id(1708), helpstring("property Elasticity")] HRESULT Elasticity([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Elasticity(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 531: [propput, id(1708), helpstring("property Elasticity")] HRESULT Elasticity([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Elasticity(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 1709: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 532: [propget, id(1709), helpstring("property ElasticityFalloff")] HRESULT ElasticityFalloff([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_ElasticityFalloff(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 533: [propput, id(1709), helpstring("property ElasticityFalloff")] HRESULT ElasticityFalloff([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_ElasticityFalloff(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 1710: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 534: [propget, id(1710), helpstring("property Scatter")] HRESULT Scatter([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Scatter(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 535: [propput, id(1710), helpstring("property Scatter")] HRESULT Scatter([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Scatter(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 1102: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 536: [propget, id(1102), helpstring("property DefaultScatter")] HRESULT DefaultScatter([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_DefaultScatter(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 537: [propput, id(1102), helpstring("property DefaultScatter")] HRESULT DefaultScatter([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_DefaultScatter(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 1103: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 538: [propget, id(1103), helpstring("property NudgeTime")] HRESULT NudgeTime([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_NudgeTime(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 539: [propput, id(1103), helpstring("property NudgeTime")] HRESULT NudgeTime([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_NudgeTime(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 1104: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 540: [propget, id(1104), helpstring("property PlungerNormalize")] HRESULT PlungerNormalize([out, retval] int *pVal);
				V_VT(&res) = VT_I4;
				hres = get_PlungerNormalize((int*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 541: [propput, id(1104), helpstring("property PlungerNormalize")] HRESULT PlungerNormalize([in] int newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_PlungerNormalize(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 1105: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 542: [propget, id(1105), helpstring("property PhysicsLoopTime")] HRESULT PhysicsLoopTime([out, retval] int *pVal);
				V_VT(&res) = VT_I4;
				hres = get_PhysicsLoopTime((int*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 543: [propput, id(1105), helpstring("property PhysicsLoopTime")] HRESULT PhysicsLoopTime([in] int newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_PhysicsLoopTime(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 1107: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 545: [propget, id(1107), helpstring("property PlungerFilter")] HRESULT PlungerFilter([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_PlungerFilter(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 546: [propput, id(1107), helpstring("property PlungerFilter")] HRESULT PlungerFilter([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_PlungerFilter(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 7: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 548: [propget, id(7), helpstring("property YieldTime")] HRESULT YieldTime([out, retval] long *pVal);
				V_VT(&res) = VT_I4;
				hres = get_YieldTime((long*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 549: [propput, id(7), helpstring("property YieldTime")] HRESULT YieldTime([in] long newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_YieldTime(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Image3: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 550: [propget, id(DISPID_Image3), helpstring("property BallImage")] HRESULT BallImage([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_BallImage(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 551: [propput, id(DISPID_Image3), helpstring("property BallImage")] HRESULT BallImage([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_BallImage(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Image2: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 552: [propget, id(DISPID_Image2), helpstring("property BackdropImage_DT")] HRESULT BackdropImage_DT([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_BackdropImage_DT(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 553: [propput, id(DISPID_Image2), helpstring("property BackdropImage_DT")] HRESULT BackdropImage_DT([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_BackdropImage_DT(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Image6: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 554: [propget, id(DISPID_Image6), helpstring("property BackdropImage_FS")] HRESULT BackdropImage_FS([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_BackdropImage_FS(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 555: [propput, id(DISPID_Image6), helpstring("property BackdropImage_FS")] HRESULT BackdropImage_FS([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_BackdropImage_FS(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Image8: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 556: [propget, id(DISPID_Image8), helpstring("property BackdropImage_FSS")] HRESULT BackdropImage_FSS([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_BackdropImage_FSS(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 557: [propput, id(DISPID_Image8), helpstring("property BackdropImage_FSS")] HRESULT BackdropImage_FSS([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_BackdropImage_FSS(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 459: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 558: [propget, id(459), helpstring("property BackdropImageApplyNightDay")] HRESULT BackdropImageApplyNightDay([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_BackdropImageApplyNightDay(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 559: [propput, id(459), helpstring("property BackdropImageApplyNightDay")] HRESULT BackdropImageApplyNightDay([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_BackdropImageApplyNightDay(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Image5: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 560: [propget, id(DISPID_Image5), helpstring("property ColorGradeImage")] HRESULT ColorGradeImage([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_ColorGradeImage(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 561: [propput, id(DISPID_Image5), helpstring("property ColorGradeImage")] HRESULT ColorGradeImage([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_ColorGradeImage(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Table_Width: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 562: [propget, id(DISPID_Table_Width), helpstring("property Width")] HRESULT Width([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Width(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 563: [propput, id(DISPID_Table_Width), helpstring("property Width")] HRESULT Width([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Width(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Table_Height: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 564: [propget, id(DISPID_Table_Height), helpstring("property Height")] HRESULT Height([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Height(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 565: [propput, id(DISPID_Table_Height), helpstring("property Height")] HRESULT Height([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Height(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Table_MaxSeparation: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 566: [propget, id(DISPID_Table_MaxSeparation), helpstring("property MaxSeparation")] HRESULT MaxSeparation([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_MaxSeparation(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 567: [propput, id(DISPID_Table_MaxSeparation), helpstring("property MaxSeparation")] HRESULT MaxSeparation([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_MaxSeparation(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Table_ZPD: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 568: [propget, id(DISPID_Table_ZPD), helpstring("property ZPD")] HRESULT ZPD([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_ZPD(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 569: [propput, id(DISPID_Table_ZPD), helpstring("property ZPD")] HRESULT ZPD([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_ZPD(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Table_Offset: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 570: [propget, id(DISPID_Table_Offset), helpstring("property Offset")] HRESULT Offset([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Offset(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 571: [propput, id(DISPID_Table_Offset), helpstring("property Offset")] HRESULT Offset([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Offset(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 427: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 572: [propget, id(427), helpstring("property Overwrite3DStereo")] HRESULT GlobalStereo3D([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_GlobalStereo3D(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 573: [propput, id(427), helpstring("property Overwrite3DStereo")] HRESULT GlobalStereo3D([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_GlobalStereo3D(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 438: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 574: [propget, id(438), helpstring("property BallDecalMode")] HRESULT BallDecalMode([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_BallDecalMode(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 575: [propput, id(438), helpstring("property BallDecalMode")] HRESULT BallDecalMode([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_BallDecalMode(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Image: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 576: [propget, id(DISPID_Image), helpstring("property Image")] HRESULT Image([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Image(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 577: [propput, id(DISPID_Image), helpstring("property Image")] HRESULT Image([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Image(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 1711: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 578: [propget, id(1711), helpstring("property FileName")] HRESULT FileName([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_FileName(&V_BSTR(&res));
			}
			break;
		}
		case DISPID_Name: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 579: [propget, id(DISPID_Name), helpstring("property Name")] HRESULT Name([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Name(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 580: [propput, id(DISPID_Name), helpstring("property Name")] HRESULT Name([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Name(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 394: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 581: [propget, id(394), helpstring("property enableAA")] HRESULT EnableAntialiasing([out, retval] UserDefaultOnOff *pVal);
				V_VT(&res) = VT_I4;
				hres = get_EnableAntialiasing((UserDefaultOnOff*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 582: [propput, id(394), helpstring("property enableAA")] HRESULT EnableAntialiasing([in] UserDefaultOnOff newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_EnableAntialiasing((UserDefaultOnOff)V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 396: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 583: [propget, id(396), helpstring("property enableAO")] HRESULT EnableAO([out, retval] UserDefaultOnOff *pVal);
				V_VT(&res) = VT_I4;
				hres = get_EnableAO((UserDefaultOnOff*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 584: [propput, id(396), helpstring("property enableAO")] HRESULT EnableAO([in] UserDefaultOnOff newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_EnableAO((UserDefaultOnOff)V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 395: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 585: [propget, id(395), helpstring("property enableFXAA")] HRESULT EnableFXAA([out, retval] FXAASettings *pVal);
				V_VT(&res) = VT_I4;
				hres = get_EnableFXAA((FXAASettings*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 586: [propput, id(395), helpstring("property enableFXAA")] HRESULT EnableFXAA([in] FXAASettings newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_EnableFXAA((FXAASettings)V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 590: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 587: [propget, id(590), helpstring("property enableSSR")] HRESULT EnableSSR([out, retval] UserDefaultOnOff *pVal);
				V_VT(&res) = VT_I4;
				hres = get_EnableSSR((UserDefaultOnOff*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 588: [propput, id(590), helpstring("property enableSSR")] HRESULT EnableSSR([in] UserDefaultOnOff newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_EnableSSR((UserDefaultOnOff)V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 450: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 589: [propget, id(450), helpstring("property BloomStrength")] HRESULT BloomStrength([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_BloomStrength(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 590: [propput, id(450), helpstring("property BloomStrength")] HRESULT BloomStrength([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_BloomStrength(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Image4: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 591: [propget, id(DISPID_Image4), helpstring("property BallFrontDecal")] HRESULT BallFrontDecal([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_BallFrontDecal(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 592: [propput, id(DISPID_Image4), helpstring("property BallFrontDecal")] HRESULT BallFrontDecal([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_BallFrontDecal(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Table_OverridePhysics: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 593: [propget, id(DISPID_Table_OverridePhysics), helpstring("property OverridePhysics")] HRESULT OverridePhysics([out, retval] PhysicsSet *pVal);
				V_VT(&res) = VT_I4;
				hres = get_OverridePhysics((PhysicsSet*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 594: [propput, id(DISPID_Table_OverridePhysics), helpstring("property OverridePhysics")] HRESULT OverridePhysics([in] PhysicsSet newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_OverridePhysics((PhysicsSet)V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 584: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 595: [propget, id(584), helpstring("property OverridePhysicsFlippers")] HRESULT OverridePhysicsFlippers([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_OverridePhysicsFlippers(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 596: [propput, id(584), helpstring("property OverridePhysicsFlippers")] HRESULT OverridePhysicsFlippers([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_OverridePhysicsFlippers(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 13432: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 597: [propget, id(13432), helpstring("property RenderEMReels")] HRESULT EnableEMReels([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_EnableEMReels(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 598: [propput, id(13432), helpstring("property RenderEMReels")] HRESULT EnableEMReels([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_EnableEMReels(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 13433: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 599: [propget, id(13433), helpstring("property RenderDecals")] HRESULT EnableDecals([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_EnableDecals(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 600: [propput, id(13433), helpstring("property RenderDecals")] HRESULT EnableDecals([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_EnableDecals(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 13434: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 601: [propget, id(13434), helpstring("property ShowDT")] HRESULT ShowDT([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_ShowDT(&V_BOOL(&res));
			}
			break;
		}
		case 625: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 602: [propget, id(625), helpstring("property ShowFSS")] HRESULT ShowFSS([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_ShowFSS(&V_BOOL(&res));
			}
			break;
		}
		case 431: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 603: [propget, id(431), helpstring("property ReflectElementsOnPlayfield")] HRESULT ReflectElementsOnPlayfield([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_ReflectElementsOnPlayfield(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 604: [propput, id(431), helpstring("property ReflectElementsOnPlayfield")] HRESULT ReflectElementsOnPlayfield([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_ReflectElementsOnPlayfield(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Image7: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 605: [propget, id(DISPID_Image7), helpstring("property EnvironmentImage")] HRESULT EnvironmentImage([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_EnvironmentImage(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 606: [propput, id(DISPID_Image7), helpstring("property EnvironmentImage")] HRESULT EnvironmentImage([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_EnvironmentImage(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 1714: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 607: [propget, id(1714), helpstring("property BackglassMode")] HRESULT BackglassMode([out, retval] BackglassIndex *pVal);
				V_VT(&res) = VT_I4;
				hres = get_BackglassMode((BackglassIndex*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 608: [propput, id(1714), helpstring("property BackglassMode")] HRESULT BackglassMode([in] BackglassIndex newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_BackglassMode((BackglassIndex)V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 200: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 611: [propget, id(200), helpstring("property Accelerometer")] HRESULT Accelerometer([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_Accelerometer(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 612: [propput, id(200), helpstring("property Accelerometer")] HRESULT Accelerometer([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_Accelerometer(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 201: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 613: [propget, id(201), helpstring("property AccelNormalMount")] HRESULT AccelNormalMount([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_AccelNormalMount(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 614: [propput, id(201), helpstring("property AccelNormalMount")] HRESULT AccelNormalMount([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_AccelNormalMount(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 202: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 616: [propget, id(202), helpstring("property AccelerometerAngle")] HRESULT AccelerometerAngle([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_AccelerometerAngle(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 617: [propput, id(202), helpstring("property AccelerometerAngle")] HRESULT AccelerometerAngle([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_AccelerometerAngle(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 209: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 630: [propget, id(209), helpstring("property GlobalDifficulty")] HRESULT GlobalDifficulty([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_GlobalDifficulty(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 631: [propput, id(209), helpstring("property GlobalDifficulty")] HRESULT GlobalDifficulty([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_GlobalDifficulty(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 214: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 633: [propget, id(214), helpstring("property TableHeight")] HRESULT TableHeight([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_TableHeight(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 634: [propput, id(214), helpstring("property TableHeight")] HRESULT TableHeight([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_TableHeight(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 217: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 636: [propget, id(217), helpstring("property DeadZone")] HRESULT DeadZone([out, retval] int *pVal);
				V_VT(&res) = VT_I4;
				hres = get_DeadZone((int*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 637: [propput, id(217), helpstring("property DeadZone")] HRESULT DeadZone([in] int newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_DeadZone(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 558: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 639: [propget, id(558), helpstring("property LightAmbient")] HRESULT LightAmbient([out, retval] OLE_COLOR *pVal);
				V_VT(&res) = VT_UI4;
				hres = get_LightAmbient(&V_UI4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 640: [propput, id(558), helpstring("property LightAmbient")] HRESULT LightAmbient([in] OLE_COLOR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_UI4);
				hres = put_LightAmbient((OLE_COLOR)V_UI4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 559: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 641: [propget, id(559), helpstring("property Light0Emission")] HRESULT Light0Emission([out, retval] OLE_COLOR *pVal);
				V_VT(&res) = VT_UI4;
				hres = get_Light0Emission(&V_UI4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 642: [propput, id(559), helpstring("property Light0Emission")] HRESULT Light0Emission([in] OLE_COLOR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_UI4);
				hres = put_Light0Emission((OLE_COLOR)V_UI4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 564: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 643: [propget, id(564), helpstring("property LightHeight")] HRESULT LightHeight([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_LightHeight(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 644: [propput, id(564), helpstring("property LightHeight")] HRESULT LightHeight([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_LightHeight(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 565: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 645: [propget, id(565), helpstring("property LightRange")] HRESULT LightRange([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_LightRange(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 646: [propput, id(565), helpstring("property LightRange")] HRESULT LightRange([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_LightRange(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 566: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 648: [propget, id(566), helpstring("property EnvironmentEmissionScale")] HRESULT EnvironmentEmissionScale([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_EnvironmentEmissionScale(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 649: [propput, id(566), helpstring("property EnvironmentEmissionScale")] HRESULT EnvironmentEmissionScale([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_EnvironmentEmissionScale(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 567: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 650: [propget, id(567), helpstring("property LightEmissionScale")] HRESULT LightEmissionScale([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_LightEmissionScale(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 651: [propput, id(567), helpstring("property LightEmissionScale")] HRESULT LightEmissionScale([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_LightEmissionScale(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 568: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 652: [propget, id(568), helpstring("property AOScale")] HRESULT AOScale([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_AOScale(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 653: [propput, id(568), helpstring("property AOScale")] HRESULT AOScale([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_AOScale(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 569: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 654: [propget, id(569), helpstring("property SSRScale")] HRESULT SSRScale([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_SSRScale(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 655: [propput, id(569), helpstring("property SSRScale")] HRESULT SSRScale([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_SSRScale(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 579: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 657: [propget, id(579), helpstring("property TableSoundVolume")] HRESULT TableSoundVolume([out, retval] int *pVal);
				V_VT(&res) = VT_I4;
				hres = get_TableSoundVolume((int*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 658: [propput, id(579), helpstring("property TableSoundVolume")] HRESULT TableSoundVolume([in] int newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_TableSoundVolume(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 580: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 659: [propget, id(580), helpstring("property TableMusicVolume")] HRESULT TableMusicVolume([out, retval] int *pVal);
				V_VT(&res) = VT_I4;
				hres = get_TableMusicVolume((int*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 660: [propput, id(580), helpstring("property TableMusicVolume")] HRESULT TableMusicVolume([in] int newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_TableMusicVolume(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 585: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 661: [propget, id(585), helpstring("property TableAdaptiveVSync")] HRESULT TableAdaptiveVSync([out, retval] int *pVal);
				V_VT(&res) = VT_I4;
				hres = get_TableAdaptiveVSync((int*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 662: [propput, id(585), helpstring("property TableAdaptiveVSync")] HRESULT TableAdaptiveVSync([in] int newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_TableAdaptiveVSync(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 1700: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 663: [propget, id(1700), helpstring("property BallReflection")] HRESULT BallReflection([out, retval] UserDefaultOnOff *pVal);
				V_VT(&res) = VT_I4;
				hres = get_BallReflection((UserDefaultOnOff*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 664: [propput, id(1700), helpstring("property BallReflection")] HRESULT BallReflection([in] UserDefaultOnOff newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_BallReflection((UserDefaultOnOff)V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 1707: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 665: [propget, id(1707), helpstring("property PlayfieldReflectionStrength")] HRESULT PlayfieldReflectionStrength([out, retval] int *pVal);
				V_VT(&res) = VT_I4;
				hres = get_PlayfieldReflectionStrength((int*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 666: [propput, id(1707), helpstring("property PlayfieldReflectionStrength")] HRESULT PlayfieldReflectionStrength([in] int newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_PlayfieldReflectionStrength(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 1704: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 667: [propget, id(1704), helpstring("property BallTrail")] HRESULT BallTrail([out, retval] UserDefaultOnOff *pVal);
				V_VT(&res) = VT_I4;
				hres = get_BallTrail((UserDefaultOnOff*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 668: [propput, id(1704), helpstring("property BallTrail")] HRESULT BallTrail([in] UserDefaultOnOff newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_BallTrail((UserDefaultOnOff)V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 1705: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 669: [propget, id(1705), helpstring("property TrailStrength")] HRESULT TrailStrength([out, retval] int *pVal);
				V_VT(&res) = VT_I4;
				hres = get_TrailStrength((int*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 670: [propput, id(1705), helpstring("property TrailStrength")] HRESULT TrailStrength([in] int newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_TrailStrength(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 1712: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 671: [propget, id(1712), helpstring("property BallPlayfieldReflectionScale")] HRESULT BallPlayfieldReflectionScale([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_BallPlayfieldReflectionScale(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 672: [propput, id(1712), helpstring("property BallPlayfieldReflectionScale")] HRESULT BallPlayfieldReflectionScale([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_BallPlayfieldReflectionScale(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 1713: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 673: [propget, id(1713), helpstring("property DefaultBulbIntensityScale")] HRESULT DefaultBulbIntensityScale([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_DefaultBulbIntensityScale(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 674: [propput, id(1713), helpstring("property DefaultBulbIntensityScale")] HRESULT DefaultBulbIntensityScale([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_DefaultBulbIntensityScale(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 420: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 675: [propget, id(420), helpstring("property DetailLevel")] HRESULT DetailLevel([out, retval] int *pVal);
				V_VT(&res) = VT_I4;
				hres = get_DetailLevel((int*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 676: [propput, id(420), helpstring("property DetailLevel")] HRESULT DetailLevel([in] int newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_DetailLevel(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 436: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 677: [propget, id(436), helpstring("property NightDay")] HRESULT NightDay([out, retval] int *pVal);
				V_VT(&res) = VT_I4;
				hres = get_NightDay((int*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 678: [propput, id(436), helpstring("property NightDay")] HRESULT NightDay([in] int newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_NightDay(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 398: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 679: [propget, id(398), helpstring("property OverwriteAlphaAcc")] HRESULT GlobalAlphaAcc([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_GlobalAlphaAcc(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 680: [propput, id(398), helpstring("property OverwriteAlphaAcc")] HRESULT GlobalAlphaAcc([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_GlobalAlphaAcc(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 588: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 681: [propget, id(588), helpstring("property OverwriteDayNight")] HRESULT GlobalDayNight([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_GlobalDayNight(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 682: [propput, id(588), helpstring("property OverwriteDayNight")] HRESULT GlobalDayNight([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_GlobalDayNight(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 219: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 683: [propget, id(219), helpstring("property Version")] HRESULT Version([out, retval] int *pVal);
				V_VT(&res) = VT_I4;
				hres = get_Version((int*)&V_I4(&res));
			}
			break;
		}
		case 38: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 684: [propget, id(38), helpstring("property VersionMajor")] HRESULT VersionMajor([out, retval] int *pVal);
				V_VT(&res) = VT_I4;
				hres = get_VersionMajor((int*)&V_I4(&res));
			}
			break;
		}
		case 39: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 685: [propget, id(39), helpstring("property VersionMinor")] HRESULT VersionMinor([out, retval] int *pVal);
				V_VT(&res) = VT_I4;
				hres = get_VersionMinor((int*)&V_I4(&res));
			}
			break;
		}
		case 40: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 686: [propget, id(40), helpstring("property VersionRevision")] HRESULT VersionRevision([out, retval] int *pVal);
				V_VT(&res) = VT_I4;
				hres = get_VersionRevision((int*)&V_I4(&res));
			}
			break;
		}
		case 24: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 687: [propget, id(24), helpstring("property VPBuildVersion")] HRESULT VPBuildVersion([out, retval] double *pVal);
				V_VT(&res) = VT_R8;
				hres = get_VPBuildVersion((double*)&V_R8(&res));
			}
			break;
		}
		case 230: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 689: [propget, id(230), helpstring("property Option")] HRESULT Option([in] BSTR OptionName, [in] float MinValue, [in] float MaxValue, [in] float Step, [in] float DefaultValue, [in] int Unit, [in, optional] VARIANT values, [out, retval] float *pVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var2;
				V_VT(&var2) = VT_EMPTY;
				VariantChangeType(&var2, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var3;
				V_VT(&var3) = VT_EMPTY;
				VariantChangeType(&var3, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var4;
				V_VT(&var4) = VT_EMPTY;
				VariantChangeType(&var4, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var5;
				V_VT(&var5) = VT_EMPTY;
				VariantChangeType(&var5, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var6;
				V_VT(&var6) = VT_EMPTY;
				VariantCopyInd(&var6, (index > 0) ? &pDispParams->rgvarg[--index] : &var6);
				V_VT(&res) = VT_R4;
				hres = get_Option(V_BSTR(&var0), V_R4(&var1), V_R4(&var2), V_R4(&var3), V_R4(&var4), V_I4(&var5), var6, &V_R4(&res));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
				VariantClear(&var3);
				VariantClear(&var4);
				VariantClear(&var5);
				VariantClear(&var6);
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 690: [propput, id(230), helpstring("property Option")] HRESULT Option([in] BSTR OptionName, [in] float MinValue, [in] float MaxValue, [in] float Step, [in] float DefaultValue, [in] int Unit, [in, optional] VARIANT values, [in] float val);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var2;
				V_VT(&var2) = VT_EMPTY;
				VariantChangeType(&var2, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var3;
				V_VT(&var3) = VT_EMPTY;
				VariantChangeType(&var3, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var4;
				V_VT(&var4) = VT_EMPTY;
				VariantChangeType(&var4, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var5;
				V_VT(&var5) = VT_EMPTY;
				VariantChangeType(&var5, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var6;
				V_VT(&var6) = VT_EMPTY;
				VariantCopyInd(&var6, (index > 0) ? &pDispParams->rgvarg[--index] : &var6);
				VARIANT var7;
				V_VT(&var7) = VT_EMPTY;
				VariantChangeType(&var7, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Option(V_BSTR(&var0), V_R4(&var1), V_R4(&var2), V_R4(&var3), V_R4(&var4), V_I4(&var5), var6, V_R4(&var7));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
				VariantClear(&var3);
				VariantClear(&var4);
				VariantClear(&var5);
				VariantClear(&var6);
				VariantClear(&var7);
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

STDMETHODIMP PinTable::GetDocumentation(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile) {
	if (index == MEMBERID_NIL) {
		*pBstrName = SysAllocString(L"Table");
		return S_OK;
	}
	return E_NOTIMPL;
}

STDMETHODIMP ScriptGlobalTable::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} namesIdsList[] = {
			{ NULL },
			{ L"ActiveBall", 19 },
			{ L"ActiveTable", 48 },
			{ L"AddCreditKey", 20 },
			{ L"AddCreditKey2", 67 },
			{ L"AutoSize", 2132 },
			{ L"AutoWidth", 2133 },
			{ L"BeginModal", 25 },
			{ L"CenterTiltKey", 8 },
			{ L"CloseSerial", 251 },
			{ L"DecalImage", 2042 },
			{ L"DecalText", 2041 },
			{ L"DisableStaticPrerendering", 228 },
			{ L"DMDColoredPixels", 47 },
			{ L"DMDHeight", 45 },
			{ L"DMDPixels", 46 },
			{ L"DMDWidth", 44 },
			{ L"DropTargetBeveled", 2025 },
			{ L"DropTargetFlatSimple", 2031 },
			{ L"DropTargetSimple", 2026 },
			{ L"EndModal", 26 },
			{ L"EndMusic", 11 },
			{ L"ExitGame", 34 },
			{ L"FireKnocker", 33 },
			{ L"FlushSerial", 252 },
			{ L"FrameIndex", 232 },
			{ L"GameTime", 22 },
			{ L"GateLongPlate", 2037 },
			{ L"GatePlate", 2036 },
			{ L"GateWireRectangle", 2035 },
			{ L"GateWireW", 2034 },
			{ L"GetBalls", 41 },
			{ L"GetCustomParam", 823 },
			{ L"GetElementByName", 43 },
			{ L"GetElements", 42 },
			{ L"GetMaterial", 231 },
			{ L"GetMaterialPhysics", 248 },
			{ L"GetPlayerHWnd", 14 },
			{ L"GetSerialDevices", 249 },
			{ L"GetTextFile", 23 },
			{ L"HitFatTargetRectangle", 2029 },
			{ L"HitFatTargetSlim", 2032 },
			{ L"HitFatTargetSquare", 2030 },
			{ L"HitTargetRectangle", 2028 },
			{ L"HitTargetRound", 2027 },
			{ L"HitTargetSlim", 2033 },
			{ L"ImageAlignCenter", 2005 },
			{ L"ImageAlignTopLeft", 2004 },
			{ L"ImageAlignWorld", 2003 },
			{ L"ImageModeWorld", 2142 },
			{ L"ImageModeWrap", 2143 },
			{ L"JoyCustomKey", 808 },
			{ L"KickerCup", 2137 },
			{ L"KickerCup2", 2141 },
			{ L"KickerGottlieb", 2140 },
			{ L"KickerHole", 2136 },
			{ L"KickerHoleSimple", 2138 },
			{ L"KickerInvisible", 2135 },
			{ L"KickerWilliams", 2139 },
			{ L"LeftFlipperKey", 4 },
			{ L"LeftMagnaSave", 31 },
			{ L"LeftTiltKey", 6 },
			{ L"LightStateBlinking", 2002 },
			{ L"LightStateOff", 2000 },
			{ L"LightStateOn", 2001 },
			{ L"LoadTexture", 229 },
			{ L"LoadValue", 18 },
			{ L"LockbarKey", 803 },
			{ L"ManualSize", 2134 },
			{ L"MaterialColor", 224 },
			{ L"MechanicalTilt", 30 },
			{ L"MusicDirectory", 257 },
			{ L"MusicVolume", 15 },
			{ L"NightDay", 436 },
			{ L"Nudge", DISPID_Table_Nudge },
			{ L"NudgeGetCalibration", 804 },
			{ L"NudgeSensorStatus", 806 },
			{ L"NudgeSetCalibration", 805 },
			{ L"NudgeTiltStatus", 807 },
			{ L"OpenSerial", 250 },
			{ L"PlatformBits", 261 },
			{ L"PlatformCPU", 260 },
			{ L"PlatformOS", 259 },
			{ L"PlayMusic", 10 },
			{ L"PlaySound", 3 },
			{ L"PlungerKey", 9 },
			{ L"PlungerTypeCustom", 2024 },
			{ L"PlungerTypeFlat", 2023 },
			{ L"PlungerTypeModern", 2022 },
			{ L"QuitPlayer", 37 },
			{ L"RampType1Wire", 2021 },
			{ L"RampType2Wire", 2018 },
			{ L"RampType3WireLeft", 2019 },
			{ L"RampType3WireRight", 2020 },
			{ L"RampType4Wire", 2017 },
			{ L"RampTypeFlat", 2016 },
			{ L"ReadSerial", 254 },
			{ L"RenderingMode", 218 },
			{ L"RightFlipperKey", 5 },
			{ L"RightMagnaSave", 32 },
			{ L"RightTiltKey", 7 },
			{ L"SaveValue", 17 },
			{ L"ScriptsDirectory", 258 },
			{ L"SeqAllOff", 2128 },
			{ L"SeqAllOn", 2129 },
			{ L"SeqArcBottomLeftDownOff", 2110 },
			{ L"SeqArcBottomLeftDownOn", 2109 },
			{ L"SeqArcBottomLeftUpOff", 2108 },
			{ L"SeqArcBottomLeftUpOn", 2107 },
			{ L"SeqArcBottomRightDownOff", 2114 },
			{ L"SeqArcBottomRightDownOn", 2113 },
			{ L"SeqArcBottomRightUpOff", 2112 },
			{ L"SeqArcBottomRightUpOn", 2111 },
			{ L"SeqArcTopLeftDownOff", 2118 },
			{ L"SeqArcTopLeftDownOn", 2117 },
			{ L"SeqArcTopLeftUpOff", 2116 },
			{ L"SeqArcTopLeftUpOn", 2115 },
			{ L"SeqArcTopRightDownOff", 2122 },
			{ L"SeqArcTopRightDownOn", 2121 },
			{ L"SeqArcTopRightUpOff", 2120 },
			{ L"SeqArcTopRightUpOn", 2119 },
			{ L"SeqBlinking", 2130 },
			{ L"SeqCircleInOff", 2086 },
			{ L"SeqCircleInOn", 2085 },
			{ L"SeqCircleOutOff", 2084 },
			{ L"SeqCircleOutOn", 2083 },
			{ L"SeqClockLeftOff", 2090 },
			{ L"SeqClockLeftOn", 2089 },
			{ L"SeqClockRightOff", 2088 },
			{ L"SeqClockRightOn", 2087 },
			{ L"SeqDiagDownLeftOff", 2058 },
			{ L"SeqDiagDownLeftOn", 2057 },
			{ L"SeqDiagDownRightOff", 2056 },
			{ L"SeqDiagDownRightOn", 2055 },
			{ L"SeqDiagUpLeftOff", 2054 },
			{ L"SeqDiagUpLeftOn", 2053 },
			{ L"SeqDiagUpRightOff", 2052 },
			{ L"SeqDiagUpRightOn", 2051 },
			{ L"SeqDownOff", 2046 },
			{ L"SeqDownOn", 2045 },
			{ L"SeqFanLeftDownOff", 2102 },
			{ L"SeqFanLeftDownOn", 2101 },
			{ L"SeqFanLeftUpOff", 2100 },
			{ L"SeqFanLeftUpOn", 2099 },
			{ L"SeqFanRightDownOff", 2106 },
			{ L"SeqFanRightDownOn", 2105 },
			{ L"SeqFanRightUpOff", 2104 },
			{ L"SeqFanRightUpOn", 2103 },
			{ L"SeqHatch1HorizOff", 2076 },
			{ L"SeqHatch1HorizOn", 2075 },
			{ L"SeqHatch1VertOff", 2080 },
			{ L"SeqHatch1VertOn", 2079 },
			{ L"SeqHatch2HorizOff", 2078 },
			{ L"SeqHatch2HorizOn", 2077 },
			{ L"SeqHatch2VertOff", 2082 },
			{ L"SeqHatch2VertOn", 2081 },
			{ L"SeqLastDynamic", 2127 },
			{ L"SeqLeftOff", 2050 },
			{ L"SeqLeftOn", 2049 },
			{ L"SeqMiddleInHorizOff", 2062 },
			{ L"SeqMiddleInHorizOn", 2061 },
			{ L"SeqMiddleInVertOff", 2066 },
			{ L"SeqMiddleInVertOn", 2065 },
			{ L"SeqMiddleOutHorizOff", 2060 },
			{ L"SeqMiddleOutHorizOn", 2059 },
			{ L"SeqMiddleOutVertOff", 2064 },
			{ L"SeqMiddleOutVertOn", 2063 },
			{ L"SeqRadarLeftOff", 2094 },
			{ L"SeqRadarLeftOn", 2093 },
			{ L"SeqRadarRightOff", 2092 },
			{ L"SeqRadarRightOn", 2091 },
			{ L"SeqRandom", 2131 },
			{ L"SeqRightOff", 2048 },
			{ L"SeqRightOn", 2047 },
			{ L"SeqScrewLeftOff", 2126 },
			{ L"SeqScrewLeftOn", 2125 },
			{ L"SeqScrewRightOff", 2124 },
			{ L"SeqScrewRightOn", 2123 },
			{ L"SeqStripe1HorizOff", 2068 },
			{ L"SeqStripe1HorizOn", 2067 },
			{ L"SeqStripe1VertOff", 2072 },
			{ L"SeqStripe1VertOn", 2071 },
			{ L"SeqStripe2HorizOff", 2070 },
			{ L"SeqStripe2HorizOn", 2069 },
			{ L"SeqStripe2VertOff", 2074 },
			{ L"SeqStripe2VertOn", 2073 },
			{ L"SeqUpOff", 2044 },
			{ L"SeqUpOn", 2043 },
			{ L"SeqWiperLeftOff", 2098 },
			{ L"SeqWiperLeftOn", 2097 },
			{ L"SeqWiperRightOff", 2096 },
			{ L"SeqWiperRightOn", 2095 },
			{ L"Setting", 824 },
			{ L"SetupSerial", 253 },
			{ L"ShapeCircle", 2006 },
			{ L"ShapeCustom", 2007 },
			{ L"ShowCursor", 262 },
			{ L"ShowDT", 13434 },
			{ L"ShowFSS", 625 },
			{ L"StagedLeftFlipperKey", 825 },
			{ L"StagedRightFlipperKey", 826 },
			{ L"StartGameKey", 12 },
			{ L"StopSound", 16 },
			{ L"SystemTime", 225 },
			{ L"TablesDirectory", 256 },
			{ L"TextAlignCenter", 2039 },
			{ L"TextAlignLeft", 2038 },
			{ L"TextAlignRight", 2040 },
			{ L"TriggerButton", 2012 },
			{ L"TriggerInder", 2015 },
			{ L"TriggerNone", 2008 },
			{ L"TriggerStar", 2010 },
			{ L"TriggerWireA", 2009 },
			{ L"TriggerWireB", 2011 },
			{ L"TriggerWireC", 2013 },
			{ L"TriggerWireD", 2014 },
			{ L"UpdateMaterial", 230 },
			{ L"UpdateMaterialPhysics", 247 },
			{ L"UserDirectory", 13 },
			{ L"Version", 219 },
			{ L"VersionMajor", 38 },
			{ L"VersionMinor", 39 },
			{ L"VersionRevision", 40 },
			{ L"VPBuildVersion", 24 },
			{ L"WindowHeight", 227 },
			{ L"WindowWidth", 226 },
			{ L"WriteSerial", 255 }
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

STDMETHODIMP ScriptGlobalTable::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
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
		case 3: {
			if (wFlags & DISPATCH_METHOD) {
				// line 701: [id(3), helpstring("method PlaySound")] HRESULT PlaySound(BSTR Sound, [defaultvalue(1)] long LoopCount, [defaultvalue(1)] float Volume, [defaultvalue(0)] float pan, [defaultvalue(0)] float randompitch, [defaultvalue(0)] long pitch, [defaultvalue(0)] VARIANT_BOOL usesame, [defaultvalue(1)] VARIANT_BOOL restart, [defaultvalue(0)] float front_rear_fade);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var1;
				V_VT(&var1) = VT_I4;
				V_I4(&var1) = 1;
				VariantChangeType(&var1, (index > 0) ? &pDispParams->rgvarg[--index] : &var1, 0, VT_I4);
				VARIANT var2;
				V_VT(&var2) = VT_R4;
				V_R4(&var2) = 1;
				VariantChangeType(&var2, (index > 0) ? &pDispParams->rgvarg[--index] : &var2, 0, VT_R4);
				VARIANT var3;
				V_VT(&var3) = VT_R4;
				V_R4(&var3) = 0;
				VariantChangeType(&var3, (index > 0) ? &pDispParams->rgvarg[--index] : &var3, 0, VT_R4);
				VARIANT var4;
				V_VT(&var4) = VT_R4;
				V_R4(&var4) = 0;
				VariantChangeType(&var4, (index > 0) ? &pDispParams->rgvarg[--index] : &var4, 0, VT_R4);
				VARIANT var5;
				V_VT(&var5) = VT_I4;
				V_I4(&var5) = 0;
				VariantChangeType(&var5, (index > 0) ? &pDispParams->rgvarg[--index] : &var5, 0, VT_I4);
				VARIANT var6;
				V_VT(&var6) = VT_BOOL;
				V_BOOL(&var6) = 0;
				VariantChangeType(&var6, (index > 0) ? &pDispParams->rgvarg[--index] : &var6, 0, VT_BOOL);
				VARIANT var7;
				V_VT(&var7) = VT_BOOL;
				V_BOOL(&var7) = 1;
				VariantChangeType(&var7, (index > 0) ? &pDispParams->rgvarg[--index] : &var7, 0, VT_BOOL);
				VARIANT var8;
				V_VT(&var8) = VT_R4;
				V_R4(&var8) = 0;
				VariantChangeType(&var8, (index > 0) ? &pDispParams->rgvarg[--index] : &var8, 0, VT_R4);
				hres = PlaySound(V_BSTR(&var0), V_I4(&var1), V_R4(&var2), V_R4(&var3), V_R4(&var4), V_I4(&var5), V_BOOL(&var6), V_BOOL(&var7), V_R4(&var8));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
				VariantClear(&var3);
				VariantClear(&var4);
				VariantClear(&var5);
				VariantClear(&var6);
				VariantClear(&var7);
				VariantClear(&var8);
			}
			break;
		}
		case 4: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 702: [propget, id(4), helpstring("property LeftFlipperKey")] HRESULT LeftFlipperKey([out, retval] long *pVal);
				V_VT(&res) = VT_I4;
				hres = get_LeftFlipperKey((long*)&V_I4(&res));
			}
			break;
		}
		case 5: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 703: [propget, id(5), helpstring("property RightFlipperKey")] HRESULT RightFlipperKey([out, retval] long *pVal);
				V_VT(&res) = VT_I4;
				hres = get_RightFlipperKey((long*)&V_I4(&res));
			}
			break;
		}
		case 825: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 704: [propget, id(825), helpstring("property StagedLeftFlipperKey")] HRESULT StagedLeftFlipperKey([out, retval] long *pVal);
				V_VT(&res) = VT_I4;
				hres = get_StagedLeftFlipperKey((long*)&V_I4(&res));
			}
			break;
		}
		case 826: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 705: [propget, id(826), helpstring("property StagedRightFlipperKey")] HRESULT StagedRightFlipperKey([out, retval] long *pVal);
				V_VT(&res) = VT_I4;
				hres = get_StagedRightFlipperKey((long*)&V_I4(&res));
			}
			break;
		}
		case 6: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 706: [propget, id(6), helpstring("property LeftTiltKey")] HRESULT LeftTiltKey([out, retval] long *pVal);
				V_VT(&res) = VT_I4;
				hres = get_LeftTiltKey((long*)&V_I4(&res));
			}
			break;
		}
		case 7: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 707: [propget, id(7), helpstring("property RightTiltKey")] HRESULT RightTiltKey([out, retval] long *pVal);
				V_VT(&res) = VT_I4;
				hres = get_RightTiltKey((long*)&V_I4(&res));
			}
			break;
		}
		case 8: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 708: [propget, id(8), helpstring("property CenterTiltKey")] HRESULT CenterTiltKey([out, retval] long *pVal);
				V_VT(&res) = VT_I4;
				hres = get_CenterTiltKey((long*)&V_I4(&res));
			}
			break;
		}
		case 9: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 709: [propget, id(9), helpstring("property PlungerKey")] HRESULT PlungerKey([out, retval] long *pVal);
				V_VT(&res) = VT_I4;
				hres = get_PlungerKey((long*)&V_I4(&res));
			}
			break;
		}
		case 10: {
			if (wFlags & DISPATCH_METHOD) {
				// line 710: [id(10), helpstring("method PlayMusic")] HRESULT PlayMusic(BSTR str, [defaultvalue(1)] float Volume);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var1;
				V_VT(&var1) = VT_R4;
				V_R4(&var1) = 1;
				VariantChangeType(&var1, (index > 0) ? &pDispParams->rgvarg[--index] : &var1, 0, VT_R4);
				hres = PlayMusic(V_BSTR(&var0), V_R4(&var1));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 15: {
			if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 711: [propput, id(15), helpstring("property MusicVolume")] HRESULT MusicVolume(float Volume);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_MusicVolume(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 11: {
			if (wFlags & DISPATCH_METHOD) {
				// line 712: [id(11), helpstring("method EndMusic")] HRESULT EndMusic();
				hres = EndMusic();
			}
			break;
		}
		case 12: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 713: [propget, id(12), helpstring("property StartGameKey")] HRESULT StartGameKey([out, retval] long *pVal);
				V_VT(&res) = VT_I4;
				hres = get_StartGameKey((long*)&V_I4(&res));
			}
			break;
		}
		case 13: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 714: [propget, id(13),  helpstring("property UserDirectory")] HRESULT UserDirectory([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_UserDirectory(&V_BSTR(&res));
			}
			break;
		}
		case 256: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 715: [propget, id(256), helpstring("property TablesDirectory")] HRESULT TablesDirectory([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_TablesDirectory(&V_BSTR(&res));
			}
			break;
		}
		case 257: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 716: [propget, id(257), helpstring("property MusicDirectory")] HRESULT MusicDirectory([in, optional] VARIANT pSubDir, [out, retval] BSTR *pVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, (index > 0) ? &pDispParams->rgvarg[--index] : &var0);
				V_VT(&res) = VT_BSTR;
				hres = get_MusicDirectory(var0, &V_BSTR(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 258: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 717: [propget, id(258), helpstring("property ScriptsDirectory")] HRESULT ScriptsDirectory([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_ScriptsDirectory(&V_BSTR(&res));
			}
			break;
		}
		case 259: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 718: [propget, id(259), helpstring("property PlatformOS")] HRESULT PlatformOS([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_PlatformOS(&V_BSTR(&res));
			}
			break;
		}
		case 260: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 719: [propget, id(260), helpstring("property PlatformCPU")] HRESULT PlatformCPU([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_PlatformCPU(&V_BSTR(&res));
			}
			break;
		}
		case 261: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 720: [propget, id(261), helpstring("property PlatformBits")] HRESULT PlatformBits([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_PlatformBits(&V_BSTR(&res));
			}
			break;
		}
		case 262: {
			if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 721: [propput, id(262), helpstring("property ShowCursor")] HRESULT ShowCursor([in] VARIANT_BOOL show);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_ShowCursor(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 14: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 723: [propget, id(14), helpstring("property GetPlayerHWnd")] HRESULT GetPlayerHWnd([out, retval] SIZE_T *pVal);
				V_VT(&res) = VT_UI4;
				hres = get_GetPlayerHWnd((SIZE_T*)&V_UI4(&res));
			}
			break;
		}
		case 16: {
			if (wFlags & DISPATCH_METHOD) {
				// line 728: [id(16), helpstring("method StopSound")] HRESULT StopSound(BSTR Sound);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = StopSound(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 17: {
			if (wFlags & DISPATCH_METHOD) {
				// line 729: [id(17), helpstring("method SaveValue")] HRESULT SaveValue(BSTR TableName, BSTR ValueName, VARIANT Value);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var2;
				V_VT(&var2) = VT_EMPTY;
				VariantCopyInd(&var2, &pDispParams->rgvarg[--index]);
				hres = SaveValue(V_BSTR(&var0), V_BSTR(&var1), var2);
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
			}
			break;
		}
		case 18: {
			if (wFlags & DISPATCH_METHOD) {
				// line 730: [id(18), helpstring("method LoadValue")] HRESULT LoadValue(BSTR TableName, BSTR ValueName, [out, retval] VARIANT *Value);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = LoadValue(V_BSTR(&var0), V_BSTR(&var1), &res);
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 19: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 731: [propget, id(19), helpstring("property ActiveBall")] HRESULT ActiveBall([out, retval] IBall **pVal);
				V_VT(&res) = VT_DISPATCH;
				hres = get_ActiveBall((IBall**)&V_DISPATCH(&res));
			}
			break;
		}
		case 20: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 732: [propget, id(20), helpstring("property AddCreditKey")] HRESULT AddCreditKey([out, retval] long *pVal);
				V_VT(&res) = VT_I4;
				hres = get_AddCreditKey((long*)&V_I4(&res));
			}
			break;
		}
		case 67: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 733: [propget, id(67), helpstring("property AddCreditKey2")] HRESULT AddCreditKey2([out, retval] long *pVal);
				V_VT(&res) = VT_I4;
				hres = get_AddCreditKey2((long*)&V_I4(&res));
			}
			break;
		}
		case 22: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 734: [propget, id(22), helpstring("property GameTime")] HRESULT GameTime([out, retval] long *pVal);
				V_VT(&res) = VT_I4;
				hres = get_GameTime((long*)&V_I4(&res));
			}
			break;
		}
		case 232: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 735: [propget, id(232), helpstring("property FrameIndex")] HRESULT FrameIndex([out, retval] long *pVal);
				V_VT(&res) = VT_I4;
				hres = get_FrameIndex((long*)&V_I4(&res));
			}
			break;
		}
		case 225: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 736: [propget, id(225), helpstring("property SystemTime")] HRESULT SystemTime([out, retval] long *pVal);
				V_VT(&res) = VT_I4;
				hres = get_SystemTime((long*)&V_I4(&res));
			}
			break;
		}
		case 823: {
			if (wFlags & DISPATCH_METHOD) {
				// line 737: [id(823), helpstring("method GetCustomParam")] HRESULT GetCustomParam([in] long index, [out, retval] BSTR *param);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				V_VT(&res) = VT_BSTR;
				hres = GetCustomParam(V_I4(&var0), &V_BSTR(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 824: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 738: [propget, id(824), helpstring("property Setting") ] HRESULT Setting(BSTR Section, BSTR SettingName, [out, retval] BSTR *pVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				V_VT(&res) = VT_BSTR;
				hres = get_Setting(V_BSTR(&var0), V_BSTR(&var1), &V_BSTR(&res));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 23: {
			if (wFlags & DISPATCH_METHOD) {
				// line 739: [id(23), helpstring("method GetTextFile")] HRESULT GetTextFile(BSTR FileName, [out, retval] BSTR *pContents);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				V_VT(&res) = VT_BSTR;
				hres = GetTextFile(V_BSTR(&var0), &V_BSTR(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 25: {
			if (wFlags & DISPATCH_METHOD) {
				// line 740: [id(25), helpstring("method BeginModal")] HRESULT BeginModal();
				hres = BeginModal();
			}
			break;
		}
		case 26: {
			if (wFlags & DISPATCH_METHOD) {
				// line 741: [id(26), helpstring("method EndModal")] HRESULT EndModal();
				hres = EndModal();
			}
			break;
		}
		case DISPID_Table_Nudge: {
			if (wFlags & DISPATCH_METHOD) {
				// line 742: [id(DISPID_Table_Nudge), helpstring("method Nudge")] HRESULT Nudge([in] float Angle, [in] float Force);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = Nudge(V_R4(&var0), V_R4(&var1));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 804: {
			if (wFlags & DISPATCH_METHOD) {
				// line 743: [id(804), helpstring("method NudgeGetCalibration")] HRESULT NudgeGetCalibration([out] VARIANT *XMax, [out] VARIANT *YMax, [out] VARIANT *XGain, [out] VARIANT *YGain, [out] VARIANT *DeadZone, [out] VARIANT *TiltSensitivty);
				VARIANT* var0 = &pDispParams->rgvarg[--index];
				VARIANT* var1 = &pDispParams->rgvarg[--index];
				VARIANT* var2 = &pDispParams->rgvarg[--index];
				VARIANT* var3 = &pDispParams->rgvarg[--index];
				VARIANT* var4 = &pDispParams->rgvarg[--index];
				VARIANT* var5 = &pDispParams->rgvarg[--index];
				hres = NudgeGetCalibration(V_VARIANTREF(var0), V_VARIANTREF(var1), V_VARIANTREF(var2), V_VARIANTREF(var3), V_VARIANTREF(var4), V_VARIANTREF(var5));
			}
			break;
		}
		case 805: {
			if (wFlags & DISPATCH_METHOD) {
				// line 744: [id(805), helpstring("method NudgeSetCalibration")] HRESULT NudgeSetCalibration([in] int XMax, [in] int YMax, [in] int XGain, [in] int YGain, [in] int DeadZone, [in] int TiltSensitivty);
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
				VariantChangeType(&var4, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var5;
				V_VT(&var5) = VT_EMPTY;
				VariantChangeType(&var5, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = NudgeSetCalibration(V_I4(&var0), V_I4(&var1), V_I4(&var2), V_I4(&var3), V_I4(&var4), V_I4(&var5));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
				VariantClear(&var3);
				VariantClear(&var4);
				VariantClear(&var5);
			}
			break;
		}
		case 806: {
			if (wFlags & DISPATCH_METHOD) {
				// line 745: [id(806), helpstring("method NudgeSensorStatus")] HRESULT NudgeSensorStatus([out] VARIANT *XNudge, [out] VARIANT *YNudge);
				VARIANT* var0 = &pDispParams->rgvarg[--index];
				VARIANT* var1 = &pDispParams->rgvarg[--index];
				hres = NudgeSensorStatus(V_VARIANTREF(var0), V_VARIANTREF(var1));
			}
			break;
		}
		case 807: {
			if (wFlags & DISPATCH_METHOD) {
				// line 746: [id(807), helpstring("method NudgeTiltStatus")] HRESULT NudgeTiltStatus([out] VARIANT *XPlumb, [out] VARIANT *YPlumb, [out] VARIANT *TiltPercent);
				VARIANT* var0 = &pDispParams->rgvarg[--index];
				VARIANT* var1 = &pDispParams->rgvarg[--index];
				VARIANT* var2 = &pDispParams->rgvarg[--index];
				hres = NudgeTiltStatus(V_VARIANTREF(var0), V_VARIANTREF(var1), V_VARIANTREF(var2));
			}
			break;
		}
		case 30: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 748: [propget, id(30), helpstring("property MechanicalTilt")] HRESULT MechanicalTilt([out, retval] long *pVal);
				V_VT(&res) = VT_I4;
				hres = get_MechanicalTilt((long*)&V_I4(&res));
			}
			break;
		}
		case 31: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 749: [propget, id(31), helpstring("property LeftMagnaSave")] HRESULT LeftMagnaSave([out, retval] long *pVal);
				V_VT(&res) = VT_I4;
				hres = get_LeftMagnaSave((long*)&V_I4(&res));
			}
			break;
		}
		case 32: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 750: [propget, id(32), helpstring("property RightMagnaSave")] HRESULT RightMagnaSave([out, retval] long *pVal);
				V_VT(&res) = VT_I4;
				hres = get_RightMagnaSave((long*)&V_I4(&res));
			}
			break;
		}
		case 34: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 751: [propget, id(34), helpstring("property ExitGame")] HRESULT ExitGame([out, retval] long *pVal);
				V_VT(&res) = VT_I4;
				hres = get_ExitGame((long*)&V_I4(&res));
			}
			break;
		}
		case 803: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 752: [propget, id(803), helpstring("property LockbarKey")] HRESULT LockbarKey([out, retval] long *pVal);
				V_VT(&res) = VT_I4;
				hres = get_LockbarKey((long*)&V_I4(&res));
			}
			break;
		}
		case 808: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 753: [propget, id(808), helpstring("property JoyCustomKey")] HRESULT JoyCustomKey([in] long index, [out, retval] long *pVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				V_VT(&res) = VT_I4;
				hres = get_JoyCustomKey(V_I4(&var0), (long*)&V_I4(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 33: {
			if (wFlags & DISPATCH_METHOD) {
				// line 754: [id(33), helpstring("method FireKnocker")] HRESULT FireKnocker([defaultvalue(1)] int Count);
				VARIANT var0;
				V_VT(&var0) = VT_I4;
				V_I4(&var0) = 1;
				VariantChangeType(&var0, (index > 0) ? &pDispParams->rgvarg[--index] : &var0, 0, VT_I4);
				hres = FireKnocker(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 37: {
			if (wFlags & DISPATCH_METHOD) {
				// line 755: [id(37), helpstring("method QuitPlayer")] HRESULT QuitPlayer([defaultvalue(0)] int CloseType);
				VARIANT var0;
				V_VT(&var0) = VT_I4;
				V_I4(&var0) = 0;
				VariantChangeType(&var0, (index > 0) ? &pDispParams->rgvarg[--index] : &var0, 0, VT_I4);
				hres = QuitPlayer(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 13434: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 757: [propget, id(13434), helpstring("property ShowDT")] HRESULT ShowDT([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_ShowDT(&V_BOOL(&res));
			}
			break;
		}
		case 625: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 758: [propget, id(625), helpstring("property ShowFSS")] HRESULT ShowFSS([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_ShowFSS(&V_BOOL(&res));
			}
			break;
		}
		case 436: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 760: [propget, id(436), helpstring("property NightDay")] HRESULT NightDay([out, retval] int *pVal);
				V_VT(&res) = VT_I4;
				hres = get_NightDay((int*)&V_I4(&res));
			}
			break;
		}
		case 41: {
			if (wFlags & DISPATCH_METHOD) {
				// line 762: [id(41), helpstring("method GetBalls")] HRESULT GetBalls([out, retval] SAFEARRAY(VARIANT) *pVal);
				V_VT(&res) = VT_VARIANT|VT_ARRAY;
				hres = GetBalls((SAFEARRAY**)&V_ARRAY(&res));
			}
			break;
		}
		case 42: {
			if (wFlags & DISPATCH_METHOD) {
				// line 763: [id(42), helpstring("method GetElements")] HRESULT GetElements([out, retval] SAFEARRAY(VARIANT) *pVal);
				V_VT(&res) = VT_VARIANT|VT_ARRAY;
				hres = GetElements((SAFEARRAY**)&V_ARRAY(&res));
			}
			break;
		}
		case 43: {
			if (wFlags & DISPATCH_METHOD) {
				// line 764: [id(43), helpstring("method GetElementByName")] HRESULT GetElementByName([in] BSTR name, [out, retval] IDispatch* *pVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				V_VT(&res) = VT_DISPATCH;
				hres = GetElementByName(V_BSTR(&var0), (IDispatch**)&V_DISPATCH(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 48: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 765: [propget, id(48), helpstring("property ActiveTable")] HRESULT ActiveTable([out, retval] ITable **pVal);
				V_VT(&res) = VT_DISPATCH;
				hres = get_ActiveTable((ITable**)&V_DISPATCH(&res));
			}
			break;
		}
		case 44: {
			if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 767: [propput, id(44), helpstring("property DMDWidth")] HRESULT DMDWidth([in] int pVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_DMDWidth(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 45: {
			if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 768: [propput, id(45), helpstring("property DMDHeight")] HRESULT DMDHeight([in] int pVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_DMDHeight(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 46: {
			if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 769: [propput, id(46), helpstring("property DMDPixels")] HRESULT DMDPixels([in] VARIANT pVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = put_DMDPixels(var0);
				VariantClear(&var0);
			}
			break;
		}
		case 47: {
			if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 770: [propput, id(47), helpstring("property DMDColoredPixels")] HRESULT DMDColoredPixels([in] VARIANT pVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = put_DMDColoredPixels(var0);
				VariantClear(&var0);
			}
			break;
		}
		case 219: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 772: [propget, id(219), helpstring("property Version")] HRESULT Version([out, retval] int *pVal);
				V_VT(&res) = VT_I4;
				hres = get_Version((int*)&V_I4(&res));
			}
			break;
		}
		case 38: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 773: [propget, id(38), helpstring("property VersionMajor")] HRESULT VersionMajor([out, retval] int *pVal);
				V_VT(&res) = VT_I4;
				hres = get_VersionMajor((int*)&V_I4(&res));
			}
			break;
		}
		case 39: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 774: [propget, id(39), helpstring("property VersionMinor")] HRESULT VersionMinor([out, retval] int *pVal);
				V_VT(&res) = VT_I4;
				hres = get_VersionMinor((int*)&V_I4(&res));
			}
			break;
		}
		case 40: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 775: [propget, id(40), helpstring("property VersionRevision")] HRESULT VersionRevision([out, retval] int *pVal);
				V_VT(&res) = VT_I4;
				hres = get_VersionRevision((int*)&V_I4(&res));
			}
			break;
		}
		case 24: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 776: [propget, id(24), helpstring("property VPBuildVersion")] HRESULT VPBuildVersion([out, retval] double *pVal);
				V_VT(&res) = VT_R8;
				hres = get_VPBuildVersion((double*)&V_R8(&res));
			}
			break;
		}
		case 249: {
			if (wFlags & DISPATCH_METHOD) {
				// line 778: [id(249), helpstring("method GetSerialDevices")] HRESULT GetSerialDevices([out, retval] VARIANT *pVal);
				hres = GetSerialDevices(&res);
			}
			break;
		}
		case 250: {
			if (wFlags & DISPATCH_METHOD) {
				// line 779: [id(250), helpstring("method OpenSerial")] HRESULT OpenSerial([in] BSTR device);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = OpenSerial(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 251: {
			if (wFlags & DISPATCH_METHOD) {
				// line 780: [id(251), helpstring("method CloseSerial")] HRESULT CloseSerial();
				hres = CloseSerial();
			}
			break;
		}
		case 252: {
			if (wFlags & DISPATCH_METHOD) {
				// line 781: [id(252), helpstring("method FlushSerial")] HRESULT FlushSerial();
				hres = FlushSerial();
			}
			break;
		}
		case 253: {
			if (wFlags & DISPATCH_METHOD) {
				// line 782: [id(253), helpstring("method SetupSerial")] HRESULT SetupSerial([in] int baud, [in] int bits, [in] int parity, [in] int stopbit, [in] VARIANT_BOOL rts, [in] VARIANT_BOOL dtr);
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
				VariantChangeType(&var4, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				VARIANT var5;
				V_VT(&var5) = VT_EMPTY;
				VariantChangeType(&var5, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = SetupSerial(V_I4(&var0), V_I4(&var1), V_I4(&var2), V_I4(&var3), V_BOOL(&var4), V_BOOL(&var5));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
				VariantClear(&var3);
				VariantClear(&var4);
				VariantClear(&var5);
			}
			break;
		}
		case 254: {
			if (wFlags & DISPATCH_METHOD) {
				// line 783: [id(254), helpstring("method ReadSerial")] HRESULT ReadSerial([in] int size, [out] VARIANT *pVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT* var1 = &pDispParams->rgvarg[--index];
				hres = ReadSerial(V_I4(&var0), V_VARIANTREF(var1));
				VariantClear(&var0);
			}
			break;
		}
		case 255: {
			if (wFlags & DISPATCH_METHOD) {
				// line 784: [id(255), helpstring("method WriteSerial")] HRESULT WriteSerial([in] VARIANT pVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = WriteSerial(var0);
				VariantClear(&var0);
			}
			break;
		}
		case 218: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 786: [propget, id(218), helpstring("property RenderingMode")] HRESULT RenderingMode([out, retval] int *pVal);
				V_VT(&res) = VT_I4;
				hres = get_RenderingMode((int*)&V_I4(&res));
			}
			break;
		}
		case 230: {
			if (wFlags & DISPATCH_METHOD) {
				// line 788: [id(230), helpstring("method UpdateMaterial")] HRESULT UpdateMaterial([in] BSTR pVal, [in] float wrapLighting, [in] float roughness, [in] float glossyImageLerp, [in] float thickness, [in] float edge, [in] float edgeAlpha, [in] float opacity,[in] OLE_COLOR base, [in] OLE_COLOR glossy, [in] OLE_COLOR clearcoat, [in] VARIANT_BOOL isMetal, [in] VARIANT_BOOL opacityActive,[in] float elasticity, [in] float elasticityFalloff, [in] float friction, [in] float scatterAngle);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var2;
				V_VT(&var2) = VT_EMPTY;
				VariantChangeType(&var2, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var3;
				V_VT(&var3) = VT_EMPTY;
				VariantChangeType(&var3, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var4;
				V_VT(&var4) = VT_EMPTY;
				VariantChangeType(&var4, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var5;
				V_VT(&var5) = VT_EMPTY;
				VariantChangeType(&var5, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var6;
				V_VT(&var6) = VT_EMPTY;
				VariantChangeType(&var6, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var7;
				V_VT(&var7) = VT_EMPTY;
				VariantChangeType(&var7, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var8;
				V_VT(&var8) = VT_EMPTY;
				VariantChangeType(&var8, &pDispParams->rgvarg[--index], 0, VT_UI4);
				VARIANT var9;
				V_VT(&var9) = VT_EMPTY;
				VariantChangeType(&var9, &pDispParams->rgvarg[--index], 0, VT_UI4);
				VARIANT var10;
				V_VT(&var10) = VT_EMPTY;
				VariantChangeType(&var10, &pDispParams->rgvarg[--index], 0, VT_UI4);
				VARIANT var11;
				V_VT(&var11) = VT_EMPTY;
				VariantChangeType(&var11, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				VARIANT var12;
				V_VT(&var12) = VT_EMPTY;
				VariantChangeType(&var12, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				VARIANT var13;
				V_VT(&var13) = VT_EMPTY;
				VariantChangeType(&var13, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var14;
				V_VT(&var14) = VT_EMPTY;
				VariantChangeType(&var14, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var15;
				V_VT(&var15) = VT_EMPTY;
				VariantChangeType(&var15, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var16;
				V_VT(&var16) = VT_EMPTY;
				VariantChangeType(&var16, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = UpdateMaterial(V_BSTR(&var0), V_R4(&var1), V_R4(&var2), V_R4(&var3), V_R4(&var4), V_R4(&var5), V_R4(&var6), V_R4(&var7), (OLE_COLOR)V_UI4(&var8), (OLE_COLOR)V_UI4(&var9), (OLE_COLOR)V_UI4(&var10), V_BOOL(&var11), V_BOOL(&var12), V_R4(&var13), V_R4(&var14), V_R4(&var15), V_R4(&var16));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
				VariantClear(&var3);
				VariantClear(&var4);
				VariantClear(&var5);
				VariantClear(&var6);
				VariantClear(&var7);
				VariantClear(&var8);
				VariantClear(&var9);
				VariantClear(&var10);
				VariantClear(&var11);
				VariantClear(&var12);
				VariantClear(&var13);
				VariantClear(&var14);
				VariantClear(&var15);
				VariantClear(&var16);
			}
			break;
		}
		case 231: {
			if (wFlags & DISPATCH_METHOD) {
				// line 791: [id(231), helpstring("method GetMaterial")] HRESULT GetMaterial([in] BSTR pVal, [out] VARIANT *wrapLighting, [out] VARIANT *roughness, [out] VARIANT *glossyImageLerp, [out] VARIANT *thickness, [out] VARIANT *edge, [out] VARIANT *edgeAlpha, [out] VARIANT *opacity,[out] VARIANT *base, [out] VARIANT *glossy, [out] VARIANT *clearcoat, [out] VARIANT *isMetal, [out] VARIANT *opacityActive,[out] VARIANT *elasticity, [out] VARIANT *elasticityFalloff, [out] VARIANT *friction, [out] VARIANT *scatterAngle);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
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
				hres = GetMaterial(V_BSTR(&var0), V_VARIANTREF(var1), V_VARIANTREF(var2), V_VARIANTREF(var3), V_VARIANTREF(var4), V_VARIANTREF(var5), V_VARIANTREF(var6), V_VARIANTREF(var7), V_VARIANTREF(var8), V_VARIANTREF(var9), V_VARIANTREF(var10), V_VARIANTREF(var11), V_VARIANTREF(var12), V_VARIANTREF(var13), V_VARIANTREF(var14), V_VARIANTREF(var15), V_VARIANTREF(var16));
				VariantClear(&var0);
			}
			break;
		}
		case 247: {
			if (wFlags & DISPATCH_METHOD) {
				// line 794: [id(247), helpstring("method UpdateMaterialPhysics")] HRESULT UpdateMaterialPhysics([in] BSTR pVal,[in] float elasticity, [in] float elasticityFalloff, [in] float friction, [in] float scatterAngle);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var2;
				V_VT(&var2) = VT_EMPTY;
				VariantChangeType(&var2, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var3;
				V_VT(&var3) = VT_EMPTY;
				VariantChangeType(&var3, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var4;
				V_VT(&var4) = VT_EMPTY;
				VariantChangeType(&var4, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = UpdateMaterialPhysics(V_BSTR(&var0), V_R4(&var1), V_R4(&var2), V_R4(&var3), V_R4(&var4));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
				VariantClear(&var3);
				VariantClear(&var4);
			}
			break;
		}
		case 248: {
			if (wFlags & DISPATCH_METHOD) {
				// line 796: [id(248), helpstring("method GetMaterialPhysics")] HRESULT GetMaterialPhysics([in] BSTR pVal,[out] VARIANT *elasticity, [out] VARIANT *elasticityFalloff, [out] VARIANT *friction, [out] VARIANT *scatterAngle);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT* var1 = &pDispParams->rgvarg[--index];
				VARIANT* var2 = &pDispParams->rgvarg[--index];
				VARIANT* var3 = &pDispParams->rgvarg[--index];
				VARIANT* var4 = &pDispParams->rgvarg[--index];
				hres = GetMaterialPhysics(V_BSTR(&var0), V_VARIANTREF(var1), V_VARIANTREF(var2), V_VARIANTREF(var3), V_VARIANTREF(var4));
				VariantClear(&var0);
			}
			break;
		}
		case 224: {
			if (wFlags & DISPATCH_METHOD) {
				// line 798: [id(224), helpstring("method MaterialColor")] HRESULT MaterialColor([in] BSTR name, [in] OLE_COLOR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_UI4);
				hres = MaterialColor(V_BSTR(&var0), (OLE_COLOR)V_UI4(&var1));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 226: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 800: [propget, id(226), helpstring("property WindowWidth")] HRESULT WindowWidth([out, retval] int *pVal);
				V_VT(&res) = VT_I4;
				hres = get_WindowWidth((int*)&V_I4(&res));
			}
			break;
		}
		case 227: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 801: [propget, id(227), helpstring("property WindowHeight")] HRESULT WindowHeight([out, retval] int *pVal);
				V_VT(&res) = VT_I4;
				hres = get_WindowHeight((int*)&V_I4(&res));
			}
			break;
		}
		case 228: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 803: [propget, id(228), helpstring("property DisableStaticPrerendering")] HRESULT DisableStaticPrerendering([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_DisableStaticPrerendering(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 804: [propput, id(228), helpstring("property DisableStaticPrerendering")] HRESULT DisableStaticPrerendering([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_DisableStaticPrerendering(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 229: {
			if (wFlags & DISPATCH_METHOD) {
				// line 806: [id(229), helpstring("method LoadTexture")] HRESULT LoadTexture([in] BSTR imageName, [in] BSTR fileName);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = LoadTexture(V_BSTR(&var0), V_BSTR(&var1));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 2000: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 0;
				hres = S_OK;
			}
			break;
		}
		case 2001: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 1;
				hres = S_OK;
			}
			break;
		}
		case 2002: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 2;
				hres = S_OK;
			}
			break;
		}
		case 2003: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 0;
				hres = S_OK;
			}
			break;
		}
		case 2004: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 1;
				hres = S_OK;
			}
			break;
		}
		case 2005: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 2;
				hres = S_OK;
			}
			break;
		}
		case 2006: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 0;
				hres = S_OK;
			}
			break;
		}
		case 2007: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 1;
				hres = S_OK;
			}
			break;
		}
		case 2008: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 0;
				hres = S_OK;
			}
			break;
		}
		case 2009: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 1;
				hres = S_OK;
			}
			break;
		}
		case 2010: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 2;
				hres = S_OK;
			}
			break;
		}
		case 2011: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 3;
				hres = S_OK;
			}
			break;
		}
		case 2012: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 4;
				hres = S_OK;
			}
			break;
		}
		case 2013: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 5;
				hres = S_OK;
			}
			break;
		}
		case 2014: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 6;
				hres = S_OK;
			}
			break;
		}
		case 2015: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 7;
				hres = S_OK;
			}
			break;
		}
		case 2016: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 0;
				hres = S_OK;
			}
			break;
		}
		case 2017: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 1;
				hres = S_OK;
			}
			break;
		}
		case 2018: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 2;
				hres = S_OK;
			}
			break;
		}
		case 2019: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 3;
				hres = S_OK;
			}
			break;
		}
		case 2020: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 4;
				hres = S_OK;
			}
			break;
		}
		case 2021: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 5;
				hres = S_OK;
			}
			break;
		}
		case 2022: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 1;
				hres = S_OK;
			}
			break;
		}
		case 2023: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 2;
				hres = S_OK;
			}
			break;
		}
		case 2024: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 3;
				hres = S_OK;
			}
			break;
		}
		case 2025: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 1;
				hres = S_OK;
			}
			break;
		}
		case 2026: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 2;
				hres = S_OK;
			}
			break;
		}
		case 2027: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 3;
				hres = S_OK;
			}
			break;
		}
		case 2028: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 4;
				hres = S_OK;
			}
			break;
		}
		case 2029: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 5;
				hres = S_OK;
			}
			break;
		}
		case 2030: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 6;
				hres = S_OK;
			}
			break;
		}
		case 2031: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 7;
				hres = S_OK;
			}
			break;
		}
		case 2032: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 8;
				hres = S_OK;
			}
			break;
		}
		case 2033: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 9;
				hres = S_OK;
			}
			break;
		}
		case 2034: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 1;
				hres = S_OK;
			}
			break;
		}
		case 2035: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 2;
				hres = S_OK;
			}
			break;
		}
		case 2036: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 3;
				hres = S_OK;
			}
			break;
		}
		case 2037: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 4;
				hres = S_OK;
			}
			break;
		}
		case 2038: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 0;
				hres = S_OK;
			}
			break;
		}
		case 2039: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 1;
				hres = S_OK;
			}
			break;
		}
		case 2040: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 2;
				hres = S_OK;
			}
			break;
		}
		case 2041: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 0;
				hres = S_OK;
			}
			break;
		}
		case 2042: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 1;
				hres = S_OK;
			}
			break;
		}
		case 2043: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 1;
				hres = S_OK;
			}
			break;
		}
		case 2044: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 2;
				hres = S_OK;
			}
			break;
		}
		case 2045: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 3;
				hres = S_OK;
			}
			break;
		}
		case 2046: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 4;
				hres = S_OK;
			}
			break;
		}
		case 2047: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 5;
				hres = S_OK;
			}
			break;
		}
		case 2048: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 6;
				hres = S_OK;
			}
			break;
		}
		case 2049: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 7;
				hres = S_OK;
			}
			break;
		}
		case 2050: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 8;
				hres = S_OK;
			}
			break;
		}
		case 2051: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 9;
				hres = S_OK;
			}
			break;
		}
		case 2052: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 10;
				hres = S_OK;
			}
			break;
		}
		case 2053: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 11;
				hres = S_OK;
			}
			break;
		}
		case 2054: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 12;
				hres = S_OK;
			}
			break;
		}
		case 2055: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 13;
				hres = S_OK;
			}
			break;
		}
		case 2056: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 14;
				hres = S_OK;
			}
			break;
		}
		case 2057: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 15;
				hres = S_OK;
			}
			break;
		}
		case 2058: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 16;
				hres = S_OK;
			}
			break;
		}
		case 2059: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 17;
				hres = S_OK;
			}
			break;
		}
		case 2060: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 18;
				hres = S_OK;
			}
			break;
		}
		case 2061: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 19;
				hres = S_OK;
			}
			break;
		}
		case 2062: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 20;
				hres = S_OK;
			}
			break;
		}
		case 2063: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 21;
				hres = S_OK;
			}
			break;
		}
		case 2064: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 22;
				hres = S_OK;
			}
			break;
		}
		case 2065: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 23;
				hres = S_OK;
			}
			break;
		}
		case 2066: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 24;
				hres = S_OK;
			}
			break;
		}
		case 2067: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 25;
				hres = S_OK;
			}
			break;
		}
		case 2068: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 26;
				hres = S_OK;
			}
			break;
		}
		case 2069: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 27;
				hres = S_OK;
			}
			break;
		}
		case 2070: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 28;
				hres = S_OK;
			}
			break;
		}
		case 2071: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 29;
				hres = S_OK;
			}
			break;
		}
		case 2072: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 30;
				hres = S_OK;
			}
			break;
		}
		case 2073: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 31;
				hres = S_OK;
			}
			break;
		}
		case 2074: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 32;
				hres = S_OK;
			}
			break;
		}
		case 2075: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 33;
				hres = S_OK;
			}
			break;
		}
		case 2076: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 34;
				hres = S_OK;
			}
			break;
		}
		case 2077: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 35;
				hres = S_OK;
			}
			break;
		}
		case 2078: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 36;
				hres = S_OK;
			}
			break;
		}
		case 2079: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 37;
				hres = S_OK;
			}
			break;
		}
		case 2080: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 38;
				hres = S_OK;
			}
			break;
		}
		case 2081: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 39;
				hres = S_OK;
			}
			break;
		}
		case 2082: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 40;
				hres = S_OK;
			}
			break;
		}
		case 2083: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 41;
				hres = S_OK;
			}
			break;
		}
		case 2084: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 42;
				hres = S_OK;
			}
			break;
		}
		case 2085: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 43;
				hres = S_OK;
			}
			break;
		}
		case 2086: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 44;
				hres = S_OK;
			}
			break;
		}
		case 2087: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 45;
				hres = S_OK;
			}
			break;
		}
		case 2088: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 46;
				hres = S_OK;
			}
			break;
		}
		case 2089: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 47;
				hres = S_OK;
			}
			break;
		}
		case 2090: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 48;
				hres = S_OK;
			}
			break;
		}
		case 2091: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 49;
				hres = S_OK;
			}
			break;
		}
		case 2092: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 50;
				hres = S_OK;
			}
			break;
		}
		case 2093: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 51;
				hres = S_OK;
			}
			break;
		}
		case 2094: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 52;
				hres = S_OK;
			}
			break;
		}
		case 2095: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 53;
				hres = S_OK;
			}
			break;
		}
		case 2096: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 54;
				hres = S_OK;
			}
			break;
		}
		case 2097: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 55;
				hres = S_OK;
			}
			break;
		}
		case 2098: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 56;
				hres = S_OK;
			}
			break;
		}
		case 2099: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 57;
				hres = S_OK;
			}
			break;
		}
		case 2100: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 58;
				hres = S_OK;
			}
			break;
		}
		case 2101: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 59;
				hres = S_OK;
			}
			break;
		}
		case 2102: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 60;
				hres = S_OK;
			}
			break;
		}
		case 2103: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 61;
				hres = S_OK;
			}
			break;
		}
		case 2104: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 62;
				hres = S_OK;
			}
			break;
		}
		case 2105: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 63;
				hres = S_OK;
			}
			break;
		}
		case 2106: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 64;
				hres = S_OK;
			}
			break;
		}
		case 2107: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 65;
				hres = S_OK;
			}
			break;
		}
		case 2108: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 66;
				hres = S_OK;
			}
			break;
		}
		case 2109: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 67;
				hres = S_OK;
			}
			break;
		}
		case 2110: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 68;
				hres = S_OK;
			}
			break;
		}
		case 2111: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 69;
				hres = S_OK;
			}
			break;
		}
		case 2112: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 70;
				hres = S_OK;
			}
			break;
		}
		case 2113: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 71;
				hres = S_OK;
			}
			break;
		}
		case 2114: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 72;
				hres = S_OK;
			}
			break;
		}
		case 2115: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 73;
				hres = S_OK;
			}
			break;
		}
		case 2116: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 74;
				hres = S_OK;
			}
			break;
		}
		case 2117: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 75;
				hres = S_OK;
			}
			break;
		}
		case 2118: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 76;
				hres = S_OK;
			}
			break;
		}
		case 2119: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 77;
				hres = S_OK;
			}
			break;
		}
		case 2120: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 78;
				hres = S_OK;
			}
			break;
		}
		case 2121: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 79;
				hres = S_OK;
			}
			break;
		}
		case 2122: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 80;
				hres = S_OK;
			}
			break;
		}
		case 2123: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 81;
				hres = S_OK;
			}
			break;
		}
		case 2124: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 82;
				hres = S_OK;
			}
			break;
		}
		case 2125: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 83;
				hres = S_OK;
			}
			break;
		}
		case 2126: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 84;
				hres = S_OK;
			}
			break;
		}
		case 2127: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 85;
				hres = S_OK;
			}
			break;
		}
		case 2128: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 1000;
				hres = S_OK;
			}
			break;
		}
		case 2129: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 1001;
				hres = S_OK;
			}
			break;
		}
		case 2130: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 1002;
				hres = S_OK;
			}
			break;
		}
		case 2131: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 1003;
				hres = S_OK;
			}
			break;
		}
		case 2132: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 0;
				hres = S_OK;
			}
			break;
		}
		case 2133: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 1;
				hres = S_OK;
			}
			break;
		}
		case 2134: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 2;
				hres = S_OK;
			}
			break;
		}
		case 2135: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 0;
				hres = S_OK;
			}
			break;
		}
		case 2136: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 1;
				hres = S_OK;
			}
			break;
		}
		case 2137: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 2;
				hres = S_OK;
			}
			break;
		}
		case 2138: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 3;
				hres = S_OK;
			}
			break;
		}
		case 2139: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 4;
				hres = S_OK;
			}
			break;
		}
		case 2140: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 5;
				hres = S_OK;
			}
			break;
		}
		case 2141: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 6;
				hres = S_OK;
			}
			break;
		}
		case 2142: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 0;
				hres = S_OK;
			}
			break;
		}
		case 2143: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				V_VT(&res) = VT_I2;
				V_I2(&res) = 1;
				hres = S_OK;
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

STDMETHODIMP ScriptGlobalTable::GetDocumentation(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile) {
	if (index == MEMBERID_NIL) {
		*pBstrName = SysAllocString(L"TableGlobal");
		return S_OK;
	}
	return E_NOTIMPL;
}

HRESULT PinTable::FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) {
	static struct {
		DISPID dispId;
		const WCHAR *name;
	} idsNamesList[] = {
			{ NULL },
			{ DISPID_GameEvents_KeyDown, L"_KeyDown" },
			{ DISPID_GameEvents_KeyUp, L"_KeyUp" },
			{ DISPID_GameEvents_Init, L"_Init" },
			{ DISPID_GameEvents_MusicDone, L"_MusicDone" },
			{ DISPID_GameEvents_Exit, L"_Exit" },
			{ DISPID_GameEvents_Paused, L"_Paused" },
			{ DISPID_GameEvents_UnPaused, L"_UnPaused" },
			{ DISPID_GameEvents_OptionEvent, L"_OptionEvent" }
	};

	static WCHAR wzName[MAXSTRING];
	size_t min = 1, max = ARRAY_SIZE(idsNamesList) - 1, i;
	int r;
	while(min <= max) {
		i = (min + max) / 2;
		if (idsNamesList[i].dispId == dispid) {
			wcscpy(wzName, m_wzName);
			wcscat(wzName, idsNamesList[i].name);
			LPOLESTR fnNames = (LPOLESTR)wzName;
			DISPID tDispid;
			CComPtr<IDispatch> disp;
			g_pplayer->m_ptable->m_pcv->m_pScript->GetScriptDispatch(nullptr, &disp);
			if (SUCCEEDED(disp->GetIDsOfNames(IID_NULL, &fnNames, 1, 0, &tDispid))) {
				return disp->Invoke(tDispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, pdispparams, nullptr, nullptr, nullptr);
			}
			return DISP_E_MEMBERNOTFOUND;
		}
		else if (idsNamesList[i].dispId < dispid)
		   min = i+1;
		else
		   max = i-1;
	}
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP DebuggerModule::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} namesIdsList[] = {
			{ NULL },
			{ L"Print", 10 }
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

STDMETHODIMP DebuggerModule::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
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
		case 10: {
			if (wFlags & DISPATCH_METHOD) {
				// line 836: [id(10), helpstring("method Print")] HRESULT Print([in, optional] VARIANT* pvar);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = Print(&var0);
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

STDMETHODIMP DebuggerModule::GetDocumentation(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile) {
	if (index == MEMBERID_NIL) {
		*pBstrName = SysAllocString(L"VPDebug");
		return S_OK;
	}
	return E_NOTIMPL;
}

STDMETHODIMP Surface::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} namesIdsList[] = {
			{ NULL },
			{ L"BlendDisableLighting", 494 },
			{ L"BlendDisableLightingFromBelow", 496 },
			{ L"CanDrop", 11 },
			{ L"Collidable", 111 },
			{ L"Disabled", 108 },
			{ L"DisableLighting", 484 },
			{ L"DisplayTexture", 13 },
			{ L"Elasticity", 110 },
			{ L"ElasticityFalloff", 120 },
			{ L"FlipbookAnimation", 113 },
			{ L"Friction", 114 },
			{ L"HasHitEvent", 34 },
			{ L"HeightBottom", 8 },
			{ L"HeightTop", 9 },
			{ L"Image", DISPID_Image },
			{ L"ImageAlignment", 7 },
			{ L"IsBottomSolid", 116 },
			{ L"IsDropped", 12 },
			{ L"Name", DISPID_Name },
			{ L"OverwritePhysics", 432 },
			{ L"PhysicsMaterial", 734 },
			{ L"PlaySlingshotHit", 999 },
			{ L"ReflectionEnabled", 431 },
			{ L"Scatter", 115 },
			{ L"SideImage", DISPID_Image2 },
			{ L"SideMaterial", 341 },
			{ L"SideVisible", 109 },
			{ L"SlingshotAnimation", 112 },
			{ L"SlingshotMaterial", 426 },
			{ L"SlingshotStrength", 14 },
			{ L"SlingshotThreshold", 427 },
			{ L"Threshold", 33 },
			{ L"TimerEnabled", DISPID_Timer_Enabled },
			{ L"TimerInterval", DISPID_Timer_Interval },
			{ L"TopMaterial", 340 },
			{ L"UserValue", DISPID_UserValue },
			{ L"Visible", 16 }
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

STDMETHODIMP Surface::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
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
		case DISPID_Name: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 851: [propget, id(DISPID_Name), helpstring("property Name")] HRESULT Name([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Name(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 852: [propput, id(DISPID_Name), helpstring("property Name")] HRESULT Name([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Name(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Timer_Enabled: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 853: [propget, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_TimerEnabled(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 854: [propput, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_TimerEnabled(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Timer_Interval: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 855: [propget, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([out, retval] long *pVal);
				V_VT(&res) = VT_I4;
				hres = get_TimerInterval((long*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 856: [propput, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([in] long newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_TimerInterval(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 34: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 857: [propget, id(34), helpstring("property HasHitEvent")] HRESULT HasHitEvent([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_HasHitEvent(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 858: [propput, id(34), helpstring("property HasHitEvent")] HRESULT HasHitEvent([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_HasHitEvent(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 33: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 859: [propget, id(33), helpstring("property Threshold")] HRESULT Threshold([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Threshold(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 860: [propput, id(33), helpstring("property Threshold")] HRESULT Threshold([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Threshold(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Image: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 861: [propget, id(DISPID_Image), helpstring("property Image")] HRESULT Image([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Image(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 862: [propput, id(DISPID_Image), helpstring("property Image")] HRESULT Image([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Image(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 341: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 863: [propget, id(341), helpstring("property SideMaterial")] HRESULT SideMaterial([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_SideMaterial(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 864: [propput, id(341), helpstring("property SideMaterial")] HRESULT SideMaterial([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_SideMaterial(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 7: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 865: [propget, id(7), helpstring("property ImageAlignment")] HRESULT ImageAlignment([out, retval] ImageAlignment *pVal);
				V_VT(&res) = VT_I4;
				hres = get_ImageAlignment((ImageAlignment*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 866: [propput, id(7), helpstring("property ImageAlignment")] HRESULT ImageAlignment([in] ImageAlignment newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_ImageAlignment((ImageAlignment)V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 8: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 867: [propget, id(8), helpstring("property HeightBottom")] HRESULT HeightBottom([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_HeightBottom(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 868: [propput, id(8), helpstring("property HeightBottom")] HRESULT HeightBottom([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_HeightBottom(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 9: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 869: [propget, id(9), helpstring("property HeightTop")] HRESULT HeightTop([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_HeightTop(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 870: [propput, id(9), helpstring("property HeightTop")] HRESULT HeightTop([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_HeightTop(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 340: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 871: [propget, id(340), helpstring("property TopMaterial")] HRESULT TopMaterial([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_TopMaterial(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 872: [propput, id(340), helpstring("property TopMaterial")] HRESULT TopMaterial([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_TopMaterial(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 11: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 873: [propget, id(11), helpstring("property CanDrop")] HRESULT CanDrop([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_CanDrop(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 874: [propput, id(11), helpstring("property CanDrop")] HRESULT CanDrop([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_CanDrop(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 111: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 876: [propget, id(111), helpstring("property Collidable")] HRESULT Collidable([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_Collidable(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 877: [propput, id(111), helpstring("property Collidable")] HRESULT Collidable([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_Collidable(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 12: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 879: [propget, id(12), helpstring("property IsDropped")] HRESULT IsDropped([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_IsDropped(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 880: [propput, id(12), helpstring("property IsDropped")] HRESULT IsDropped([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_IsDropped(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 13: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 881: [propget, id(13), helpstring("property DisplayTexture")] HRESULT DisplayTexture([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_DisplayTexture(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 882: [propput, id(13), helpstring("property DisplayTexture")] HRESULT DisplayTexture([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_DisplayTexture(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 14: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 883: [propget, id(14), helpstring("property SlingshotStrength")] HRESULT SlingshotStrength([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_SlingshotStrength(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 884: [propput, id(14), helpstring("property SlingshotStrength")] HRESULT SlingshotStrength([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_SlingshotStrength(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 110: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 885: [propget, id(110), helpstring("property Elasticity")] HRESULT Elasticity([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Elasticity(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 886: [propput, id(110), helpstring("property Elasticity")] HRESULT Elasticity([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Elasticity(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 120: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 887: [propget, id(120), helpstring("property ElasticityFalloff")] HRESULT ElasticityFalloff([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_ElasticityFalloff(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 888: [propput, id(120), helpstring("property ElasticityFalloff")] HRESULT ElasticityFalloff([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_ElasticityFalloff(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Image2: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 889: [propget, id(DISPID_Image2), helpstring("property SideImage")] HRESULT SideImage([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_SideImage(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 890: [propput, id(DISPID_Image2), helpstring("property SideImage")] HRESULT SideImage([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_SideImage(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 16: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 891: [propget, id(16), helpstring("property Visible")] HRESULT Visible([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_Visible(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 892: [propput, id(16), helpstring("property Visible")] HRESULT Visible([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_Visible(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 108: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 893: [propget, id(108), helpstring("property Disabled")] HRESULT Disabled([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_Disabled(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 894: [propput, id(108), helpstring("property Disabled")] HRESULT Disabled([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_Disabled(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 109: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 895: [propget, id(109), helpstring("property SideVisible")] HRESULT SideVisible([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_SideVisible(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 896: [propput, id(109), helpstring("property SideVisible")] HRESULT SideVisible([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_SideVisible(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_UserValue: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 897: [propget, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([out, retval] VARIANT* pVal);
				hres = get_UserValue(&res);
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 898: [propput, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([in] VARIANT* newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = put_UserValue(&var0);
				VariantClear(&var0);
			}
			break;
		}
		case 426: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 899: [propget, id(426), helpstring("property SlingshotMaterial")] HRESULT SlingshotMaterial([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_SlingshotMaterial(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 900: [propput, id(426), helpstring("property SlingshotMaterial")] HRESULT SlingshotMaterial([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_SlingshotMaterial(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 427: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 901: [propget, id(427), helpstring("property SlingshotThreshold")] HRESULT SlingshotThreshold([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_SlingshotThreshold(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 902: [propput, id(427), helpstring("property SlingshotThreshold")] HRESULT SlingshotThreshold([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_SlingshotThreshold(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 112: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 903: [propget, id(112), helpstring("property SlingshotAnimation")] HRESULT SlingshotAnimation([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_SlingshotAnimation(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 904: [propput, id(112), helpstring("property SlingshotAnimation")] HRESULT SlingshotAnimation([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_SlingshotAnimation(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 113: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 905: [propget, id(113), helpstring("property FlipbookAnimation")] HRESULT FlipbookAnimation([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_FlipbookAnimation(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 906: [propput, id(113), helpstring("property FlipbookAnimation")] HRESULT FlipbookAnimation([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_FlipbookAnimation(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 116: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 907: [propget, id(116), helpstring("property IsBottomSolid")] HRESULT IsBottomSolid([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_IsBottomSolid(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 908: [propput, id(116), helpstring("property IsBottomSolid")] HRESULT IsBottomSolid([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_IsBottomSolid(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 484: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 909: [propget, id(484), helpstring("property DisableLighting")] HRESULT DisableLighting([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_DisableLighting(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 910: [propput, id(484), helpstring("property DisableLighting")] HRESULT DisableLighting([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_DisableLighting(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 494: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 911: [propget, id(494), helpstring("property BlendDisableLighting")] HRESULT BlendDisableLighting([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_BlendDisableLighting(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 912: [propput, id(494), helpstring("property BlendDisableLighting")] HRESULT BlendDisableLighting([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_BlendDisableLighting(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 496: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 913: [propget, id(496), helpstring("property BlendDisableLightingFromBelow")] HRESULT BlendDisableLightingFromBelow([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_BlendDisableLightingFromBelow(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 914: [propput, id(496), helpstring("property BlendDisableLightingFromBelow")] HRESULT BlendDisableLightingFromBelow([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_BlendDisableLightingFromBelow(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 114: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 915: [propget, id(114), helpstring("property Friction")] HRESULT Friction([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Friction(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 916: [propput, id(114), helpstring("property Friction")] HRESULT Friction([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Friction(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 115: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 917: [propget, id(115), helpstring("property Scatter")] HRESULT Scatter([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Scatter(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 918: [propput, id(115), helpstring("property Scatter")] HRESULT Scatter([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Scatter(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 431: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 919: [propget, id(431), helpstring("property ReflectionEnabled")] HRESULT ReflectionEnabled([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_ReflectionEnabled(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 920: [propput, id(431), helpstring("property ReflectionEnabled")] HRESULT ReflectionEnabled([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_ReflectionEnabled(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 999: {
			if (wFlags & DISPATCH_METHOD) {
				// line 921: [id(999), helpstring("method PlaySlingshotHit")] HRESULT PlaySlingshotHit();
				hres = PlaySlingshotHit();
			}
			break;
		}
		case 734: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 922: [propget, id(734), helpstring("property PhysicsMaterial")] HRESULT PhysicsMaterial([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_PhysicsMaterial(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 923: [propput, id(734), helpstring("property PhysicsMaterial")] HRESULT PhysicsMaterial([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_PhysicsMaterial(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 432: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 924: [propget, id(432), helpstring("property OverwritePhysics")] HRESULT OverwritePhysics([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_OverwritePhysics(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 925: [propput, id(432), helpstring("property OverwritePhysics")] HRESULT OverwritePhysics([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_OverwritePhysics(V_BOOL(&var0));
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

STDMETHODIMP Surface::GetDocumentation(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile) {
	if (index == MEMBERID_NIL) {
		*pBstrName = SysAllocString(L"Wall");
		return S_OK;
	}
	return E_NOTIMPL;
}

HRESULT Surface::FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) {
	static struct {
		DISPID dispId;
		const WCHAR *name;
	} idsNamesList[] = {
			{ NULL },
			{ DISPID_GameEvents_Init, L"_Init" },
			{ DISPID_SurfaceEvents_Slingshot, L"_Slingshot" },
			{ DISPID_TimerEvents_Timer, L"_Timer" },
			{ DISPID_HitEvents_Hit, L"_Hit" }
	};

	static WCHAR wzName[MAXSTRING];
	size_t min = 1, max = ARRAY_SIZE(idsNamesList) - 1, i;
	int r;
	while(min <= max) {
		i = (min + max) / 2;
		if (idsNamesList[i].dispId == dispid) {
			wcscpy(wzName, m_wzName);
			wcscat(wzName, idsNamesList[i].name);
			LPOLESTR fnNames = (LPOLESTR)wzName;
			DISPID tDispid;
			CComPtr<IDispatch> disp;
			g_pplayer->m_ptable->m_pcv->m_pScript->GetScriptDispatch(nullptr, &disp);
			if (SUCCEEDED(disp->GetIDsOfNames(IID_NULL, &fnNames, 1, 0, &tDispid))) {
				return disp->Invoke(tDispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, pdispparams, nullptr, nullptr, nullptr);
			}
			return DISP_E_MEMBERNOTFOUND;
		}
		else if (idsNamesList[i].dispId < dispid)
		   min = i+1;
		else
		   max = i-1;
	}
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP DragPoint::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} namesIdsList[] = {
			{ NULL },
			{ L"CalcHeight", 377 },
			{ L"IsAutoTextureCoordinate", 4 },
			{ L"Smooth", 3 },
			{ L"TextureCoordinateU", 5 },
			{ L"X", 1 },
			{ L"Y", 2 },
			{ L"Z", 6 }
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

STDMETHODIMP DragPoint::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
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
				// line 950: [propget, id(1), helpstring("property X")] HRESULT X([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_X(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 951: [propput, id(1), helpstring("property X")] HRESULT X([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_X(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 2: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 952: [propget, id(2), helpstring("property Y")] HRESULT Y([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Y(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 953: [propput, id(2), helpstring("property Y")] HRESULT Y([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Y(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 6: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 954: [propget, id(6), helpstring("property Z")] HRESULT Z([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Z(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 955: [propput, id(6), helpstring("property Z")] HRESULT Z([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Z(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 377: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 956: [propget, id(377), helpstring("property calcHeight")] HRESULT CalcHeight([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_CalcHeight(&V_R4(&res));
			}
			break;
		}
		case 3: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 957: [propget, id(3), helpstring("property Smooth")] HRESULT Smooth([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_Smooth(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 958: [propput, id(3), helpstring("property Smooth")] HRESULT Smooth([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_Smooth(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 4: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 959: [propget, id(4), helpstring("property IsAutoTextureCoordinate")] HRESULT IsAutoTextureCoordinate([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_IsAutoTextureCoordinate(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 960: [propput, id(4), helpstring("property IsAutoTextureCoordinate")] HRESULT IsAutoTextureCoordinate([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_IsAutoTextureCoordinate(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 5: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 961: [propget, id(5), helpstring("property TextureCoordinateU")] HRESULT TextureCoordinateU([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_TextureCoordinateU(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 962: [propput, id(5), helpstring("property TextureCoordinateU")] HRESULT TextureCoordinateU([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_TextureCoordinateU(V_R4(&var0));
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

STDMETHODIMP DragPoint::GetDocumentation(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile) {
	if (index == MEMBERID_NIL) {
		*pBstrName = SysAllocString(L"ControlPoint");
		return S_OK;
	}
	return E_NOTIMPL;
}

STDMETHODIMP Flipper::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} namesIdsList[] = {
			{ NULL },
			{ L"BaseRadius", 1 },
			{ L"CurrentAngle", 12 },
			{ L"Elasticity", 110 },
			{ L"ElasticityFalloff", 28 },
			{ L"Enabled", 394 },
			{ L"EndAngle", 7 },
			{ L"EndRadius", 2 },
			{ L"EOSTorque", 113 },
			{ L"EOSTorqueAngle", 189 },
			{ L"FlipperRadiusMin", 111 },
			{ L"Friction", 114 },
			{ L"Height", 107 },
			{ L"Image", DISPID_Image },
			{ L"Length", 3 },
			{ L"Mass", DISPID_Flipper_Speed },
			{ L"Material", 340 },
			{ L"Name", DISPID_Name },
			{ L"OverridePhysics", DISPID_Flipper_OverridePhysics },
			{ L"RampUp", 27 },
			{ L"ReflectionEnabled", 431 },
			{ L"Return", 23 },
			{ L"RotateToEnd", 5 },
			{ L"RotateToStart", 6 },
			{ L"RubberHeight", 24 },
			{ L"RubberMaterial", 341 },
			{ L"RubberThickness", 18 },
			{ L"RubberWidth", 25 },
			{ L"Scatter", 115 },
			{ L"StartAngle", 4 },
			{ L"Strength", 19 },
			{ L"Surface", DISPID_Surface },
			{ L"TimerEnabled", DISPID_Timer_Enabled },
			{ L"TimerInterval", DISPID_Timer_Interval },
			{ L"UserValue", DISPID_UserValue },
			{ L"Visible", 458 },
			{ L"X", 13 },
			{ L"Y", 14 }
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

STDMETHODIMP Flipper::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
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
				// line 980: [propget, id(1), helpstring("property BaseRadius")] HRESULT BaseRadius([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_BaseRadius(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 981: [propput, id(1), helpstring("property BaseRadius")] HRESULT BaseRadius([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_BaseRadius(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 2: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 982: [propget, id(2), helpstring("property EndRadius")] HRESULT EndRadius([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_EndRadius(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 983: [propput, id(2), helpstring("property EndRadius")] HRESULT EndRadius([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_EndRadius(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 3: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 984: [propget, id(3), helpstring("property Length")] HRESULT Length([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Length(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 985: [propput, id(3), helpstring("property Length")] HRESULT Length([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Length(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 4: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 986: [propget, id(4), helpstring("property StartAngle")] HRESULT StartAngle([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_StartAngle(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 987: [propput, id(4), helpstring("property StartAngle")] HRESULT StartAngle([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_StartAngle(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 5: {
			if (wFlags & DISPATCH_METHOD) {
				// line 988: [id(5), helpstring("method RotateToEnd")] HRESULT RotateToEnd();
				hres = RotateToEnd();
			}
			break;
		}
		case 6: {
			if (wFlags & DISPATCH_METHOD) {
				// line 989: [id(6), helpstring("method RotateToStart")] HRESULT RotateToStart();
				hres = RotateToStart();
			}
			break;
		}
		case 7: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 990: [propget, id(7), helpstring("property EndAngle")] HRESULT EndAngle([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_EndAngle(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 991: [propput, id(7), helpstring("property EndAngle")] HRESULT EndAngle([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_EndAngle(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Timer_Enabled: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 996: [propget, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_TimerEnabled(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 997: [propput, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_TimerEnabled(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Timer_Interval: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 998: [propget, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([out, retval] long *pVal);
				V_VT(&res) = VT_I4;
				hres = get_TimerInterval((long*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 999: [propput, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([in] long newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_TimerInterval(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 12: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1000: [propget, id(12), nonbrowsable, helpstring("property CurrentAngle")] HRESULT CurrentAngle([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_CurrentAngle(&V_R4(&res));
			}
			break;
		}
		case 13: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1001: [propget, id(13), helpstring("property X")] HRESULT X([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_X(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1002: [propput, id(13), helpstring("property X")] HRESULT X([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_X(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 14: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1003: [propget, id(14), helpstring("property Y")] HRESULT Y([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Y(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1004: [propput, id(14), helpstring("property Y")] HRESULT Y([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Y(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 340: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1005: [propget, id(340), helpstring("property Material")] HRESULT Material([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Material(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1006: [propput, id(340), helpstring("property Material")] HRESULT Material([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Material(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Flipper_Speed: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1007: [propget, id(DISPID_Flipper_Speed), helpstring("property Mass")] HRESULT Mass([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Mass(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1008: [propput, id(DISPID_Flipper_Speed), helpstring("property Mass")] HRESULT Mass([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Mass(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Flipper_OverridePhysics: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1009: [propget, id(DISPID_Flipper_OverridePhysics), helpstring("property OverridePhysics")] HRESULT OverridePhysics([out, retval] PhysicsSet *pVal);
				V_VT(&res) = VT_I4;
				hres = get_OverridePhysics((PhysicsSet*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1010: [propput, id(DISPID_Flipper_OverridePhysics), helpstring("property OverridePhysics")] HRESULT OverridePhysics([in] PhysicsSet newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_OverridePhysics((PhysicsSet)V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 341: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1011: [propget, id(341), helpstring("property RubberMaterial")] HRESULT RubberMaterial([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_RubberMaterial(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1012: [propput, id(341), helpstring("property RubberMaterial")] HRESULT RubberMaterial([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_RubberMaterial(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 18: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1013: [propget, id(18), helpstring("property RubberThickness")] HRESULT RubberThickness([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_RubberThickness(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1014: [propput, id(18), helpstring("property RubberThickness")] HRESULT RubberThickness([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_RubberThickness(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 19: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1015: [propget, id(19), helpstring("property Strength")] HRESULT Strength([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Strength(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1016: [propput, id(19), helpstring("property Strength")] HRESULT Strength([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Strength(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 458: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1017: [propget, id(458), helpstring("property Visible")] HRESULT Visible([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_Visible(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1018: [propput, id(458), helpstring("property Visible")] HRESULT Visible([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_Visible(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 394: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1019: [propget, id(394), helpstring("property Enabled")] HRESULT Enabled([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_Enabled(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1020: [propput, id(394), helpstring("property Enabled")] HRESULT Enabled([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_Enabled(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 110: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1021: [propget, id(110), helpstring("property Elasticity")] HRESULT Elasticity([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Elasticity(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1022: [propput, id(110), helpstring("property Elasticity")] HRESULT Elasticity([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Elasticity(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 115: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1023: [propget, id(115), helpstring("property Scatter")] HRESULT Scatter([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Scatter(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1024: [propput, id(115), helpstring("property Scatter")] HRESULT Scatter([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Scatter(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 23: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1025: [propget, id(23), helpstring("property Return")] HRESULT Return([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Return(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1026: [propput, id(23), helpstring("property Return")] HRESULT Return([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Return(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 24: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1027: [propget, id(24), helpstring("property RubberHeight")] HRESULT RubberHeight([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_RubberHeight(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1028: [propput, id(24), helpstring("property RubberHeight")] HRESULT RubberHeight([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_RubberHeight(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 25: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1029: [propget, id(25), helpstring("property RubberWidth")] HRESULT RubberWidth([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_RubberWidth(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1030: [propput, id(25), helpstring("property RubberWidth")] HRESULT RubberWidth([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_RubberWidth(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 114: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1031: [propget, id(114), helpstring("property Friction")] HRESULT Friction([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Friction(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1032: [propput, id(114), helpstring("property Friction")] HRESULT Friction([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Friction(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 27: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1033: [propget, id(27), helpstring("property RampUp")] HRESULT RampUp([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_RampUp(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1034: [propput, id(27), helpstring("property RampUp")] HRESULT RampUp([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_RampUp(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 28: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1035: [propget, id(28), helpstring("property ElasticityFalloff")] HRESULT ElasticityFalloff([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_ElasticityFalloff(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1036: [propput, id(28), helpstring("property ElasticityFalloff")] HRESULT ElasticityFalloff([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_ElasticityFalloff(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Surface: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1037: [propget, id(DISPID_Surface), helpstring("property Surface")] HRESULT Surface([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Surface(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1038: [propput, id(DISPID_Surface), helpstring("property Surface")] HRESULT Surface([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Surface(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Name: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1039: [propget, id(DISPID_Name), helpstring("property Name")] HRESULT Name([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Name(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1040: [propput, id(DISPID_Name), helpstring("property Name")] HRESULT Name([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Name(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_UserValue: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1041: [propget, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([out, retval] VARIANT* pVal);
				hres = get_UserValue(&res);
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1042: [propput, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([in] VARIANT* newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = put_UserValue(&var0);
				VariantClear(&var0);
			}
			break;
		}
		case 107: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1043: [propget, id(107), helpstring("property Height")] HRESULT Height([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Height(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1044: [propput, id(107), helpstring("property Height")] HRESULT Height([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Height(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 113: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1045: [propget, id(113), helpstring("property EOSTorque")] HRESULT EOSTorque([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_EOSTorque(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1046: [propput, id(113), helpstring("property EOSTorque")] HRESULT EOSTorque([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_EOSTorque(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 189: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1047: [propget, id(189), helpstring("property EOSTorqueAngle")] HRESULT EOSTorqueAngle([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_EOSTorqueAngle(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1048: [propput, id(189), helpstring("property EOSTorqueAngle")] HRESULT EOSTorqueAngle([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_EOSTorqueAngle(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 111: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1050: [propget, id(111), helpstring("property FlipperRadiusMin")] HRESULT FlipperRadiusMin([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_FlipperRadiusMin(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1051: [propput, id(111), helpstring("property FlipperRadiusMin")] HRESULT FlipperRadiusMin([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_FlipperRadiusMin(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Image: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1053: [propget, id(DISPID_Image), helpstring("property Image")] HRESULT Image([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Image(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1054: [propput, id(DISPID_Image), helpstring("property Image")] HRESULT Image([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Image(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 431: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1055: [propget, id(431), helpstring("property ReflectionEnabled")] HRESULT ReflectionEnabled([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_ReflectionEnabled(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1056: [propput, id(431), helpstring("property ReflectionEnabled")] HRESULT ReflectionEnabled([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_ReflectionEnabled(V_BOOL(&var0));
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

STDMETHODIMP Flipper::GetDocumentation(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile) {
	if (index == MEMBERID_NIL) {
		*pBstrName = SysAllocString(L"Flipper");
		return S_OK;
	}
	return E_NOTIMPL;
}

HRESULT Flipper::FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) {
	static struct {
		DISPID dispId;
		const WCHAR *name;
	} idsNamesList[] = {
			{ NULL },
			{ DISPID_GameEvents_Init, L"_Init" },
			{ DISPID_FlipperEvents_Collide, L"_Collide" },
			{ DISPID_TimerEvents_Timer, L"_Timer" },
			{ DISPID_HitEvents_Hit, L"_Hit" },
			{ DISPID_LimitEvents_EOS, L"_LimitEOS" },
			{ DISPID_LimitEvents_BOS, L"_LimitBOS" },
			{ DISPID_AnimateEvents_Animate, L"_Animate" }
	};

	static WCHAR wzName[MAXSTRING];
	size_t min = 1, max = ARRAY_SIZE(idsNamesList) - 1, i;
	int r;
	while(min <= max) {
		i = (min + max) / 2;
		if (idsNamesList[i].dispId == dispid) {
			wcscpy(wzName, m_wzName);
			wcscat(wzName, idsNamesList[i].name);
			LPOLESTR fnNames = (LPOLESTR)wzName;
			DISPID tDispid;
			CComPtr<IDispatch> disp;
			g_pplayer->m_ptable->m_pcv->m_pScript->GetScriptDispatch(nullptr, &disp);
			if (SUCCEEDED(disp->GetIDsOfNames(IID_NULL, &fnNames, 1, 0, &tDispid))) {
				return disp->Invoke(tDispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, pdispparams, nullptr, nullptr, nullptr);
			}
			return DISP_E_MEMBERNOTFOUND;
		}
		else if (idsNamesList[i].dispId < dispid)
		   min = i+1;
		else
		   max = i-1;
	}
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP Timer::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} namesIdsList[] = {
			{ NULL },
			{ L"Enabled", DISPID_Timer_Enabled },
			{ L"Interval", DISPID_Timer_Interval },
			{ L"Name", DISPID_Name },
			{ L"UserValue", DISPID_UserValue }
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

STDMETHODIMP Timer::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
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
		case DISPID_Timer_Enabled: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1088: [propget, id(DISPID_Timer_Enabled), helpstring("property Enabled")] HRESULT Enabled([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_Enabled(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1089: [propput, id(DISPID_Timer_Enabled), helpstring("property Enabled")] HRESULT Enabled([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_Enabled(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Timer_Interval: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1090: [propget, id(DISPID_Timer_Interval), helpstring("property Interval")] HRESULT Interval([out, retval] long *pVal);
				V_VT(&res) = VT_I4;
				hres = get_Interval((long*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1091: [propput, id(DISPID_Timer_Interval), helpstring("property Interval")] HRESULT Interval([in] long newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_Interval(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Name: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1092: [propget, id(DISPID_Name), helpstring("property Name")] HRESULT Name([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Name(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1093: [propput, id(DISPID_Name), helpstring("property Name")] HRESULT Name([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Name(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_UserValue: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1094: [propget, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([out, retval] VARIANT* pVal);
				hres = get_UserValue(&res);
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1095: [propput, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([in] VARIANT* newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = put_UserValue(&var0);
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

STDMETHODIMP Timer::GetDocumentation(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile) {
	if (index == MEMBERID_NIL) {
		*pBstrName = SysAllocString(L"Timer");
		return S_OK;
	}
	return E_NOTIMPL;
}

HRESULT Timer::FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) {
	static struct {
		DISPID dispId;
		const WCHAR *name;
	} idsNamesList[] = {
			{ NULL },
			{ DISPID_GameEvents_Init, L"_Init" },
			{ DISPID_TimerEvents_Timer, L"_Timer" }
	};

	static WCHAR wzName[MAXSTRING];
	size_t min = 1, max = ARRAY_SIZE(idsNamesList) - 1, i;
	int r;
	while(min <= max) {
		i = (min + max) / 2;
		if (idsNamesList[i].dispId == dispid) {
			wcscpy(wzName, m_wzName);
			wcscat(wzName, idsNamesList[i].name);
			LPOLESTR fnNames = (LPOLESTR)wzName;
			DISPID tDispid;
			CComPtr<IDispatch> disp;
			g_pplayer->m_ptable->m_pcv->m_pScript->GetScriptDispatch(nullptr, &disp);
			if (SUCCEEDED(disp->GetIDsOfNames(IID_NULL, &fnNames, 1, 0, &tDispid))) {
				return disp->Invoke(tDispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, pdispparams, nullptr, nullptr, nullptr);
			}
			return DISP_E_MEMBERNOTFOUND;
		}
		else if (idsNamesList[i].dispId < dispid)
		   min = i+1;
		else
		   max = i-1;
	}
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP Plunger::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} namesIdsList[] = {
			{ NULL },
			{ L"AnimFrames", DISPID_PluFrames },
			{ L"AutoPlunger", 116 },
			{ L"CreateBall", 5 },
			{ L"Fire", 2 },
			{ L"FireSpeed", 4 },
			{ L"Image", DISPID_Image },
			{ L"Material", 340 },
			{ L"MechPlunger", 110 },
			{ L"MechStrength", 111 },
			{ L"MomentumXfer", 118 },
			{ L"MotionDevice", 216 },
			{ L"Name", DISPID_Name },
			{ L"ParkPosition", 112 },
			{ L"Position", 6 },
			{ L"PullBack", 1 },
			{ L"PullBackandRetract", 7 },
			{ L"PullSpeed", 3 },
			{ L"ReflectionEnabled", 431 },
			{ L"RingDiam", DISPID_RingDiam },
			{ L"RingGap", DISPID_RingGap },
			{ L"RingWidth", DISPID_RingThickness },
			{ L"RodDiam", DISPID_RodDiam },
			{ L"ScatterVelocity", 114 },
			{ L"SpringDiam", DISPID_SpringDiam },
			{ L"SpringEndLoops", DISPID_SpringEndLoops },
			{ L"SpringGauge", DISPID_SpringGauge },
			{ L"SpringLoops", DISPID_SpringLoops },
			{ L"Stroke", 113 },
			{ L"Surface", DISPID_Surface },
			{ L"TimerEnabled", DISPID_Timer_Enabled },
			{ L"TimerInterval", DISPID_Timer_Interval },
			{ L"TipShape", DISPID_TipShape },
			{ L"Type", 390 },
			{ L"UserValue", DISPID_UserValue },
			{ L"Visible", 117 },
			{ L"Width", DISPID_Width },
			{ L"X", DISPID_X },
			{ L"Y", DISPID_Y },
			{ L"ZAdjust", DISPID_ZAdjust }
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

STDMETHODIMP Plunger::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
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
				// line 1122: [id(1), helpstring("method PullBack")] HRESULT PullBack();
				hres = PullBack();
			}
			break;
		}
		case 2: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1123: [id(2), helpstring("method Fire")] HRESULT Fire();
				hres = Fire();
			}
			break;
		}
		case 3: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1124: [propget, id(3), helpstring("property PullSpeed")] HRESULT PullSpeed([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_PullSpeed(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1125: [propput, id(3), helpstring("property PullSpeed")] HRESULT PullSpeed([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_PullSpeed(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 4: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1126: [propget, id(4), helpstring("property FireSpeed")] HRESULT FireSpeed([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_FireSpeed(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1127: [propput, id(4), helpstring("property FireSpeed")] HRESULT FireSpeed([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_FireSpeed(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 5: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1128: [id(5), helpstring("method CreateBall")] HRESULT CreateBall([out, retval] IBall **Ball);
				V_VT(&res) = VT_DISPATCH;
				hres = CreateBall((IBall**)&V_DISPATCH(&res));
			}
			break;
		}
		case 6: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1129: [id(6), helpstring("method Position")] HRESULT Position([out, retval] float *pVal); //added for mechanical plunger position, also supports normal/digital one by now
				V_VT(&res) = VT_R4;
				hres = Position(&V_R4(&res));
			}
			break;
		}
		case 7: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1130: [id(7), helpstring("method PullBackandRetract")] HRESULT PullBackandRetract(); //added for manual non-mech plunger
				hres = PullBackandRetract();
			}
			break;
		}
		case 216: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1131: [id(216), helpstring("method MotionDevice")] HRESULT MotionDevice([out, retval] int *pVal); //added for uShockType
				V_VT(&res) = VT_I4;
				hres = MotionDevice((int*)&V_I4(&res));
			}
			break;
		}
		case DISPID_Timer_Enabled: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1132: [propget, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_TimerEnabled(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1133: [propput, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_TimerEnabled(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Timer_Interval: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1134: [propget, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([out, retval] long *pVal);
				V_VT(&res) = VT_I4;
				hres = get_TimerInterval((long*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1135: [propput, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([in] long newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_TimerInterval(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_X: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1136: [propget, id(DISPID_X), helpstring("property X")] HRESULT X([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_X(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1137: [propput, id(DISPID_X), helpstring("property X")] HRESULT X([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_X(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Y: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1138: [propget, id(DISPID_Y), helpstring("property Y")] HRESULT Y([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Y(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1139: [propput, id(DISPID_Y), helpstring("property Y")] HRESULT Y([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Y(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Width: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1140: [propget, id(DISPID_Width), helpstring("property Width")] HRESULT Width([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Width(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1141: [propput, id(DISPID_Width), helpstring("property Width")] HRESULT Width([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Width(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_ZAdjust: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1142: [propget, id(DISPID_ZAdjust), helpstring("property ZAdjust")] HRESULT ZAdjust([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_ZAdjust(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1143: [propput, id(DISPID_ZAdjust), helpstring("property ZAdjust")] HRESULT ZAdjust([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_ZAdjust(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Surface: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1144: [propget, id(DISPID_Surface), helpstring("property Surface")] HRESULT Surface([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Surface(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1145: [propput, id(DISPID_Surface), helpstring("property Surface")] HRESULT Surface([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Surface(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Name: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1146: [propget, id(DISPID_Name), helpstring("property Name")] HRESULT Name([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Name(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1147: [propput, id(DISPID_Name), helpstring("property Name")] HRESULT Name([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Name(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_UserValue: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1148: [propget, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([out, retval] VARIANT* pVal);
				hres = get_UserValue(&res);
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1149: [propput, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([in] VARIANT* newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = put_UserValue(&var0);
				VariantClear(&var0);
			}
			break;
		}
		case 390: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1150: [propget, id(390), helpstring("property Type")] HRESULT Type([out, retval] PlungerType *pVal);
				V_VT(&res) = VT_I4;
				hres = get_Type((PlungerType*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1151: [propput, id(390), helpstring("property Type")] HRESULT Type([in] PlungerType newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_Type((PlungerType)V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 340: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1152: [propget, id(340), helpstring("property Material")] HRESULT Material([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Material(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1153: [propput, id(340), helpstring("property Material")] HRESULT Material([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Material(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Image: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1154: [propget, id(DISPID_Image), helpstring("property Image")] HRESULT Image([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Image(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1155: [propput, id(DISPID_Image), helpstring("property Image")] HRESULT Image([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Image(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_PluFrames: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1156: [propget, id(DISPID_PluFrames), helpstring("property AnimFrames")] HRESULT AnimFrames([out, retval] int *pVal);
				V_VT(&res) = VT_I4;
				hres = get_AnimFrames((int*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1157: [propput, id(DISPID_PluFrames), helpstring("property AnimFrames")] HRESULT AnimFrames([in] int newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_AnimFrames(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_TipShape: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1158: [propget, id(DISPID_TipShape), helpstring("property Shape")] HRESULT TipShape([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_TipShape(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1159: [propput, id(DISPID_TipShape), helpstring("property Shape")] HRESULT TipShape([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_TipShape(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_RodDiam: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1160: [propget, id(DISPID_RodDiam), helpstring("property RodDiam")] HRESULT RodDiam([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_RodDiam(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1161: [propput, id(DISPID_RodDiam), helpstring("property RodDiam")] HRESULT RodDiam([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_RodDiam(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_RingGap: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1162: [propget, id(DISPID_RingGap), helpstring("property RingGap")] HRESULT RingGap([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_RingGap(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1163: [propput, id(DISPID_RingGap), helpstring("property RingGap")] HRESULT RingGap([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_RingGap(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_RingDiam: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1164: [propget, id(DISPID_RingDiam), helpstring("property RingDiam")] HRESULT RingDiam([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_RingDiam(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1165: [propput, id(DISPID_RingDiam), helpstring("property RingDiam")] HRESULT RingDiam([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_RingDiam(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_RingThickness: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1166: [propget, id(DISPID_RingThickness), helpstring("property RingWidth")] HRESULT RingWidth([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_RingWidth(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1167: [propput, id(DISPID_RingThickness), helpstring("property RingWidth")] HRESULT RingWidth([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_RingWidth(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_SpringDiam: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1168: [propget, id(DISPID_SpringDiam), helpstring("property SpringDiam")] HRESULT SpringDiam([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_SpringDiam(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1169: [propput, id(DISPID_SpringDiam), helpstring("property SpringDiam")] HRESULT SpringDiam([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_SpringDiam(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_SpringGauge: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1170: [propget, id(DISPID_SpringGauge), helpstring("property SpringGauge")] HRESULT SpringGauge([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_SpringGauge(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1171: [propput, id(DISPID_SpringGauge), helpstring("property SpringGauge")] HRESULT SpringGauge([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_SpringGauge(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_SpringLoops: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1172: [propget, id(DISPID_SpringLoops), helpstring("property SpringLoops")] HRESULT SpringLoops([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_SpringLoops(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1173: [propput, id(DISPID_SpringLoops), helpstring("property SpringLoops")] HRESULT SpringLoops([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_SpringLoops(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_SpringEndLoops: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1174: [propget, id(DISPID_SpringEndLoops), helpstring("property SpringEndLoops")] HRESULT SpringEndLoops([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_SpringEndLoops(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1175: [propput, id(DISPID_SpringEndLoops), helpstring("property SpringEndLoops")] HRESULT SpringEndLoops([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_SpringEndLoops(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 110: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1177: [propget, id(110), helpstring("property MechPlunger")] HRESULT MechPlunger([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_MechPlunger(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1178: [propput, id(110), helpstring("property MechPlunger")] HRESULT MechPlunger([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_MechPlunger(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 116: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1179: [propget, id(116), helpstring("property AutoPlunger")] HRESULT AutoPlunger([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_AutoPlunger(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1180: [propput, id(116), helpstring("property AutoPlunger")] HRESULT AutoPlunger([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_AutoPlunger(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 117: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1181: [propget, id(117), helpstring("property Visible")] HRESULT Visible([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_Visible(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1182: [propput, id(117), helpstring("property Visible")] HRESULT Visible([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_Visible(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 111: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1183: [propget, id(111), helpstring("property MechStrength")] HRESULT MechStrength([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_MechStrength(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1184: [propput, id(111), helpstring("property MechStrength")] HRESULT MechStrength([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_MechStrength(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 112: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1185: [propget, id(112), helpstring("property ParkPosition")] HRESULT ParkPosition([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_ParkPosition(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1186: [propput, id(112), helpstring("property ParkPosition")] HRESULT ParkPosition([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_ParkPosition(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 113: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1187: [propget, id(113), helpstring("property Stroke")] HRESULT Stroke([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Stroke(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1188: [propput, id(113), helpstring("property Stroke")] HRESULT Stroke([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Stroke(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 114: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1189: [propget, id(114), helpstring("property ScatterVelocity")] HRESULT ScatterVelocity([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_ScatterVelocity(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1190: [propput, id(114), helpstring("property ScatterVelocity")] HRESULT ScatterVelocity([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_ScatterVelocity(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 118: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1191: [propget, id(118), helpstring("property MomentumXfer")] HRESULT MomentumXfer([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_MomentumXfer(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1192: [propput, id(118), helpstring("property MomentumXfer")] HRESULT MomentumXfer([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_MomentumXfer(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 431: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1193: [propget, id(431), helpstring("property ReflectionEnabled")] HRESULT ReflectionEnabled([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_ReflectionEnabled(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1194: [propput, id(431), helpstring("property ReflectionEnabled")] HRESULT ReflectionEnabled([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_ReflectionEnabled(V_BOOL(&var0));
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

STDMETHODIMP Plunger::GetDocumentation(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile) {
	if (index == MEMBERID_NIL) {
		*pBstrName = SysAllocString(L"Plunger");
		return S_OK;
	}
	return E_NOTIMPL;
}

HRESULT Plunger::FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) {
	static struct {
		DISPID dispId;
		const WCHAR *name;
	} idsNamesList[] = {
			{ NULL },
			{ DISPID_GameEvents_Init, L"_Init" },
			{ DISPID_TimerEvents_Timer, L"_Timer" },
			{ DISPID_LimitEvents_EOS, L"_LimitEOS" },
			{ DISPID_LimitEvents_BOS, L"_LimitBOS" }
	};

	static WCHAR wzName[MAXSTRING];
	size_t min = 1, max = ARRAY_SIZE(idsNamesList) - 1, i;
	int r;
	while(min <= max) {
		i = (min + max) / 2;
		if (idsNamesList[i].dispId == dispid) {
			wcscpy(wzName, m_wzName);
			wcscat(wzName, idsNamesList[i].name);
			LPOLESTR fnNames = (LPOLESTR)wzName;
			DISPID tDispid;
			CComPtr<IDispatch> disp;
			g_pplayer->m_ptable->m_pcv->m_pScript->GetScriptDispatch(nullptr, &disp);
			if (SUCCEEDED(disp->GetIDsOfNames(IID_NULL, &fnNames, 1, 0, &tDispid))) {
				return disp->Invoke(tDispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, pdispparams, nullptr, nullptr, nullptr);
			}
			return DISP_E_MEMBERNOTFOUND;
		}
		else if (idsNamesList[i].dispId < dispid)
		   min = i+1;
		else
		   max = i-1;
	}
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP Textbox::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} namesIdsList[] = {
			{ NULL },
			{ L"Alignment", 11 },
			{ L"BackColor", DISPID_Textbox_BackColor },
			{ L"DMD", 555 },
			{ L"Font", DISPID_Textbox_Font },
			{ L"FontColor", DISPID_Textbox_FontColor },
			{ L"Height", 60003 },
			{ L"IntensityScale", 587 },
			{ L"IsTransparent", 12 },
			{ L"Name", DISPID_Name },
			{ L"Text", 3 },
			{ L"TimerEnabled", DISPID_Timer_Enabled },
			{ L"TimerInterval", DISPID_Timer_Interval },
			{ L"UserValue", DISPID_UserValue },
			{ L"Visible", 616 },
			{ L"Width", 60002 },
			{ L"X", 60000 },
			{ L"Y", 60001 }
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

STDMETHODIMP Textbox::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
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
		case DISPID_Textbox_BackColor: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1223: [propget, id(DISPID_Textbox_BackColor), helpstring("property BackColor")] HRESULT BackColor([out, retval] OLE_COLOR *pVal);
				V_VT(&res) = VT_UI4;
				hres = get_BackColor(&V_UI4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1224: [propput, id(DISPID_Textbox_BackColor), helpstring("property BackColor")] HRESULT BackColor([in] OLE_COLOR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_UI4);
				hres = put_BackColor((OLE_COLOR)V_UI4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Textbox_FontColor: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1225: [propget, id(DISPID_Textbox_FontColor), helpstring("property FontColor")] HRESULT FontColor([out, retval] OLE_COLOR *pVal);
				V_VT(&res) = VT_UI4;
				hres = get_FontColor(&V_UI4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1226: [propput, id(DISPID_Textbox_FontColor), helpstring("property FontColor")] HRESULT FontColor([in] OLE_COLOR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_UI4);
				hres = put_FontColor((OLE_COLOR)V_UI4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 3: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1227: [propget, id(3), helpstring("property Text")] HRESULT Text([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Text(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1228: [propput, id(3), helpstring("property Text")] HRESULT Text([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Text(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Textbox_Font: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1229: [propget, id(DISPID_Textbox_Font), helpstring("property Font")] HRESULT Font([out, retval] IFontDisp **pVal);
				V_VT(&res) = VT_DISPATCH;
				hres = get_Font((IFontDisp**)&V_DISPATCH(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUTREF) {
				// line 1230: [propputref, id(DISPID_Textbox_Font), helpstring("property Font")] HRESULT Font([in] IFontDisp *newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = putref_Font((IFontDisp*)&var0);
				VariantClear(&var0);
			}
			break;
		}
		case 60002: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1231: [propget, id(60002), helpstring("property Width")] HRESULT Width([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Width(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1232: [propput, id(60002), helpstring("property Width")] HRESULT Width([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Width(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 60003: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1233: [propget, id(60003), helpstring("property Height")] HRESULT Height([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Height(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1234: [propput, id(60003), helpstring("property Height")] HRESULT Height([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Height(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 11: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1235: [propget, id(11), helpstring("property Alignment")] HRESULT Alignment([out, retval] TextAlignment *pVal);
				V_VT(&res) = VT_I4;
				hres = get_Alignment((TextAlignment*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1236: [propput, id(11), helpstring("property Alignment")] HRESULT Alignment([in] TextAlignment newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_Alignment((TextAlignment)V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 12: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1237: [propget, id(12), helpstring("property IsTransparent")] HRESULT IsTransparent([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_IsTransparent(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1238: [propput, id(12), helpstring("property IsTransparent")] HRESULT IsTransparent([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_IsTransparent(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 555: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1239: [propget, id(555), helpstring("property DMD")] HRESULT DMD([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_DMD(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1240: [propput, id(555), helpstring("property DMD")] HRESULT DMD([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_DMD(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Timer_Enabled: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1241: [propget, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_TimerEnabled(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1242: [propput, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_TimerEnabled(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Timer_Interval: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1243: [propget, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([out, retval] long *pVal);
				V_VT(&res) = VT_I4;
				hres = get_TimerInterval((long*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1244: [propput, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([in] long newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_TimerInterval(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 60000: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1245: [propget, id(60000), helpstring("property X")] HRESULT X([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_X(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1246: [propput, id(60000), helpstring("property X")] HRESULT X([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_X(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 60001: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1247: [propget, id(60001), helpstring("property Y")] HRESULT Y([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Y(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1248: [propput, id(60001), helpstring("property Y")] HRESULT Y([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Y(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 587: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1249: [propget, id(587), helpstring("property IntensityScale")] HRESULT IntensityScale([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_IntensityScale(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1250: [propput, id(587), helpstring("property IntensityScale")] HRESULT IntensityScale([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_IntensityScale(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Name: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1251: [propget, id(DISPID_Name), helpstring("property Name")] HRESULT Name([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Name(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1252: [propput, id(DISPID_Name), helpstring("property Name")] HRESULT Name([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Name(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_UserValue: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1253: [propget, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([out, retval] VARIANT* pVal);
				hres = get_UserValue(&res);
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1254: [propput, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([in] VARIANT* newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = put_UserValue(&var0);
				VariantClear(&var0);
			}
			break;
		}
		case 616: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1255: [propget, id(616), helpstring("property Visible")] HRESULT Visible([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_Visible(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1256: [propput, id(616), helpstring("property Visible")] HRESULT Visible([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_Visible(V_BOOL(&var0));
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

STDMETHODIMP Textbox::GetDocumentation(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile) {
	if (index == MEMBERID_NIL) {
		*pBstrName = SysAllocString(L"Textbox");
		return S_OK;
	}
	return E_NOTIMPL;
}

HRESULT Textbox::FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) {
	static struct {
		DISPID dispId;
		const WCHAR *name;
	} idsNamesList[] = {
			{ NULL },
			{ DISPID_GameEvents_Init, L"_Init" },
			{ DISPID_TimerEvents_Timer, L"_Timer" }
	};

	static WCHAR wzName[MAXSTRING];
	size_t min = 1, max = ARRAY_SIZE(idsNamesList) - 1, i;
	int r;
	while(min <= max) {
		i = (min + max) / 2;
		if (idsNamesList[i].dispId == dispid) {
			wcscpy(wzName, m_wzName);
			wcscat(wzName, idsNamesList[i].name);
			LPOLESTR fnNames = (LPOLESTR)wzName;
			DISPID tDispid;
			CComPtr<IDispatch> disp;
			g_pplayer->m_ptable->m_pcv->m_pScript->GetScriptDispatch(nullptr, &disp);
			if (SUCCEEDED(disp->GetIDsOfNames(IID_NULL, &fnNames, 1, 0, &tDispid))) {
				return disp->Invoke(tDispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, pdispparams, nullptr, nullptr, nullptr);
			}
			return DISP_E_MEMBERNOTFOUND;
		}
		else if (idsNamesList[i].dispId < dispid)
		   min = i+1;
		else
		   max = i-1;
	}
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP Bumper::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} namesIdsList[] = {
			{ NULL },
			{ L"BaseMaterial", 341 },
			{ L"BaseVisible", 110 },
			{ L"CapMaterial", 340 },
			{ L"CapVisible", 109 },
			{ L"Collidable", 111 },
			{ L"CurrentRingOffset", 28 },
			{ L"EnableSkirtAnimation", 822 },
			{ L"Force", 2 },
			{ L"HasHitEvent", 34 },
			{ L"HeightScale", 24 },
			{ L"Name", DISPID_Name },
			{ L"Orientation", 25 },
			{ L"PlayHit", 999 },
			{ L"Radius", 8 },
			{ L"ReflectionEnabled", 431 },
			{ L"RingDropOffset", 27 },
			{ L"RingMaterial", 734 },
			{ L"RingSpeed", 26 },
			{ L"RingVisible", 735 },
			{ L"RotX", 827 },
			{ L"RotY", 828 },
			{ L"Scatter", 115 },
			{ L"SkirtMaterial", 426 },
			{ L"SkirtVisible", 736 },
			{ L"Surface", DISPID_Surface },
			{ L"Threshold", 33 },
			{ L"TimerEnabled", DISPID_Timer_Enabled },
			{ L"TimerInterval", DISPID_Timer_Interval },
			{ L"UserValue", DISPID_UserValue },
			{ L"X", DISPID_X },
			{ L"Y", DISPID_Y }
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

STDMETHODIMP Bumper::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
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
		case 8: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1283: [propget, id(8), helpstring("property Radius")] HRESULT Radius([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Radius(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1284: [propput, id(8), helpstring("property Radius")] HRESULT Radius([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Radius(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_X: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1285: [propget, id(DISPID_X), helpstring("property X")] HRESULT X([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_X(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1286: [propput, id(DISPID_X), helpstring("property X")] HRESULT X([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_X(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Y: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1287: [propget, id(DISPID_Y), helpstring("property Y")] HRESULT Y([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Y(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1288: [propput, id(DISPID_Y), helpstring("property Y")] HRESULT Y([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Y(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 341: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1289: [propget, id(341), helpstring("property BaseMaterial")] HRESULT BaseMaterial([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_BaseMaterial(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1290: [propput, id(341), helpstring("property BaseMaterial")] HRESULT BaseMaterial([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_BaseMaterial(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 426: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1291: [propget, id(426), helpstring("property SkirtMaterial")] HRESULT SkirtMaterial([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_SkirtMaterial(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1292: [propput, id(426), helpstring("property SkirtMaterial")] HRESULT SkirtMaterial([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_SkirtMaterial(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_UserValue: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1293: [propget, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([out, retval] VARIANT* pVal);
				hres = get_UserValue(&res);
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1294: [propput, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([in] VARIANT* newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = put_UserValue(&var0);
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Surface: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1295: [propget, id(DISPID_Surface), helpstring("property Surface")] HRESULT Surface([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Surface(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1296: [propput, id(DISPID_Surface), helpstring("property Surface")] HRESULT Surface([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Surface(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 2: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1297: [propget, id(2), helpstring("property Force")] HRESULT Force([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Force(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1298: [propput, id(2), helpstring("property Force")] HRESULT Force([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Force(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 33: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1299: [propget, id(33), helpstring("property Threshold")] HRESULT Threshold([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Threshold(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1300: [propput, id(33), helpstring("property Threshold")] HRESULT Threshold([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Threshold(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Timer_Enabled: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1301: [propget, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_TimerEnabled(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1302: [propput, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_TimerEnabled(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Timer_Interval: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1303: [propget, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([out, retval] long *pVal);
				V_VT(&res) = VT_I4;
				hres = get_TimerInterval((long*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1304: [propput, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([in] long newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_TimerInterval(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 340: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1305: [propget, id(340), helpstring("property CapMaterial")] HRESULT CapMaterial([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_CapMaterial(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1306: [propput, id(340), helpstring("property CapMaterial")] HRESULT CapMaterial([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_CapMaterial(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 734: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1307: [propget, id(734), helpstring("property RingMaterial")] HRESULT RingMaterial([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_RingMaterial(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1308: [propput, id(734), helpstring("property RingMaterial")] HRESULT RingMaterial([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_RingMaterial(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 24: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1309: [propget, id(24), helpstring("property HeightScale")] HRESULT HeightScale([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_HeightScale(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1310: [propput, id(24), helpstring("property HeightScale")] HRESULT HeightScale([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_HeightScale(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 25: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1311: [propget, id(25), helpstring("property Orientation")] HRESULT Orientation([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Orientation(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1312: [propput, id(25), helpstring("property Orientation")] HRESULT Orientation([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Orientation(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 26: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1313: [propget, id(26), helpstring("property RingSpeed")] HRESULT RingSpeed([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_RingSpeed(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1314: [propput, id(26), helpstring("property RingSpeed")] HRESULT RingSpeed([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_RingSpeed(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 27: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1315: [propget, id(27), helpstring("property RingDropOffset")] HRESULT RingDropOffset([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_RingDropOffset(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1316: [propput, id(27), helpstring("property RingDropOffset")] HRESULT RingDropOffset([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_RingDropOffset(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 28: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1317: [propget, id(28), helpstring("property CurrentRingOffset")] HRESULT CurrentRingOffset([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_CurrentRingOffset(&V_R4(&res));
			}
			break;
		}
		case DISPID_Name: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1318: [propget, id(DISPID_Name), helpstring("property Name")] HRESULT Name([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Name(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1319: [propput, id(DISPID_Name), helpstring("property Name")] HRESULT Name([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Name(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 34: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1320: [propget, id(34), helpstring("property HitEvent")] HRESULT HasHitEvent([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_HasHitEvent(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1321: [propput, id(34), helpstring("property HitEvent")] HRESULT HasHitEvent([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_HasHitEvent(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 109: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1322: [propget, id(109), helpstring("property CapVisible")] HRESULT CapVisible([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_CapVisible(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1323: [propput, id(109), helpstring("property CapVisible")] HRESULT CapVisible([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_CapVisible(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 110: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1324: [propget, id(110), helpstring("property BaseVisible")] HRESULT BaseVisible([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_BaseVisible(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1325: [propput, id(110), helpstring("property BaseVisible")] HRESULT BaseVisible([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_BaseVisible(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 735: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1326: [propget, id(735), helpstring("property RingVisible")] HRESULT RingVisible([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_RingVisible(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1327: [propput, id(735), helpstring("property RingVisible")] HRESULT RingVisible([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_RingVisible(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 736: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1328: [propget, id(736), helpstring("property SkirtVisible")] HRESULT SkirtVisible([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_SkirtVisible(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1329: [propput, id(736), helpstring("property SkirtVisible")] HRESULT SkirtVisible([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_SkirtVisible(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 111: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1330: [propget, id(111), helpstring("property Collidable")] HRESULT Collidable([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_Collidable(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1331: [propput, id(111), helpstring("property Collidable")] HRESULT Collidable([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_Collidable(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 431: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1332: [propget, id(431), helpstring("property ReflectionEnabled")] HRESULT ReflectionEnabled([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_ReflectionEnabled(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1333: [propput, id(431), helpstring("property ReflectionEnabled")] HRESULT ReflectionEnabled([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_ReflectionEnabled(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 115: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1334: [propget, id(115), helpstring("property Scatter")] HRESULT Scatter([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Scatter(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1335: [propput, id(115), helpstring("property Scatter")] HRESULT Scatter([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Scatter(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 822: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1336: [propget, id(822), helpstring("property EnableSkirtAnimation")] HRESULT EnableSkirtAnimation([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_EnableSkirtAnimation(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1337: [propput, id(822), helpstring("property EnableSkirtAnimation")] HRESULT EnableSkirtAnimation([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_EnableSkirtAnimation(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 827: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1338: [propget, id(827), helpstring("property RotX")] HRESULT RotX([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_RotX(&V_R4(&res));
			}
			break;
		}
		case 828: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1339: [propget, id(828), helpstring("property RotY")] HRESULT RotY([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_RotY(&V_R4(&res));
			}
			break;
		}
		case 999: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1340: [id(999), helpstring("method PlayHit")] HRESULT PlayHit();
				hres = PlayHit();
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

STDMETHODIMP Bumper::GetDocumentation(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile) {
	if (index == MEMBERID_NIL) {
		*pBstrName = SysAllocString(L"Bumper");
		return S_OK;
	}
	return E_NOTIMPL;
}

HRESULT Bumper::FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) {
	static struct {
		DISPID dispId;
		const WCHAR *name;
	} idsNamesList[] = {
			{ NULL },
			{ DISPID_GameEvents_Init, L"_Init" },
			{ DISPID_TimerEvents_Timer, L"_Timer" },
			{ DISPID_HitEvents_Hit, L"_Hit" },
			{ DISPID_AnimateEvents_Animate, L"_Animate" }
	};

	static WCHAR wzName[MAXSTRING];
	size_t min = 1, max = ARRAY_SIZE(idsNamesList) - 1, i;
	int r;
	while(min <= max) {
		i = (min + max) / 2;
		if (idsNamesList[i].dispId == dispid) {
			wcscpy(wzName, m_wzName);
			wcscat(wzName, idsNamesList[i].name);
			LPOLESTR fnNames = (LPOLESTR)wzName;
			DISPID tDispid;
			CComPtr<IDispatch> disp;
			g_pplayer->m_ptable->m_pcv->m_pScript->GetScriptDispatch(nullptr, &disp);
			if (SUCCEEDED(disp->GetIDsOfNames(IID_NULL, &fnNames, 1, 0, &tDispid))) {
				return disp->Invoke(tDispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, pdispparams, nullptr, nullptr, nullptr);
			}
			return DISP_E_MEMBERNOTFOUND;
		}
		else if (idsNamesList[i].dispId < dispid)
		   min = i+1;
		else
		   max = i-1;
	}
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP Trigger::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} namesIdsList[] = {
			{ NULL },
			{ L"AnimSpeed", 26 },
			{ L"BallCntOver", 312 },
			{ L"CurrentAnimOffset", 27 },
			{ L"DestroyBall", 313 },
			{ L"Enabled", DISPID_Enabled },
			{ L"HitHeight", 314 },
			{ L"Material", 340 },
			{ L"Name", DISPID_Name },
			{ L"Radius", 346 },
			{ L"ReflectionEnabled", 431 },
			{ L"Rotation", 345 },
			{ L"Surface", DISPID_Surface },
			{ L"TimerEnabled", DISPID_Timer_Enabled },
			{ L"TimerInterval", DISPID_Timer_Interval },
			{ L"TriggerShape", DISPID_Shape },
			{ L"UserValue", DISPID_UserValue },
			{ L"Visible", 458 },
			{ L"WireThickness", 347 },
			{ L"X", DISPID_X },
			{ L"Y", DISPID_Y }
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

STDMETHODIMP Trigger::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
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
		case 346: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1369: [propget, id(346), helpstring("property Radius")] HRESULT Radius([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Radius(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1370: [propput, id(346), helpstring("property Radius")] HRESULT Radius([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Radius(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Timer_Enabled: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1371: [propget, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_TimerEnabled(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1372: [propput, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_TimerEnabled(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Timer_Interval: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1373: [propget, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([out, retval] long *pVal);
				V_VT(&res) = VT_I4;
				hres = get_TimerInterval((long*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1374: [propput, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([in] long newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_TimerInterval(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_X: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1375: [propget, id(DISPID_X), helpstring("property X")] HRESULT X([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_X(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1376: [propput, id(DISPID_X), helpstring("property X")] HRESULT X([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_X(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Y: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1377: [propget, id(DISPID_Y), helpstring("property Y")] HRESULT Y([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Y(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1378: [propput, id(DISPID_Y), helpstring("property Y")] HRESULT Y([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Y(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Enabled: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1379: [propget, id(DISPID_Enabled), helpstring("property Enabled")] HRESULT Enabled([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_Enabled(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1380: [propput, id(DISPID_Enabled), helpstring("property Enabled")] HRESULT Enabled([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_Enabled(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 458: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1381: [propget, id(458), helpstring("property Visible")] HRESULT Visible([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_Visible(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1382: [propput, id(458), helpstring("property Visible")] HRESULT Visible([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_Visible(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Shape: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1383: [propget, id(DISPID_Shape), helpstring("property TriggerShape")] HRESULT TriggerShape([out, retval] TriggerShape *pVal);
				V_VT(&res) = VT_I4;
				hres = get_TriggerShape((TriggerShape*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1384: [propput, id(DISPID_Shape), helpstring("property TriggerShape")] HRESULT TriggerShape([in] TriggerShape newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_TriggerShape((TriggerShape)V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Surface: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1385: [propget, id(DISPID_Surface), helpstring("property Surface")] HRESULT Surface([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Surface(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1386: [propput, id(DISPID_Surface), helpstring("property Surface")] HRESULT Surface([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Surface(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Name: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1387: [propget, id(DISPID_Name), helpstring("property Name")] HRESULT Name([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Name(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1388: [propput, id(DISPID_Name), helpstring("property Name")] HRESULT Name([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Name(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_UserValue: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1389: [propget, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([out, retval] VARIANT* pVal);
				hres = get_UserValue(&res);
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1390: [propput, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([in] VARIANT* newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = put_UserValue(&var0);
				VariantClear(&var0);
			}
			break;
		}
		case 312: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1391: [id(312), helpstring("method BallCntOver")] HRESULT BallCntOver([out, retval] int *pVal);
				V_VT(&res) = VT_I4;
				hres = BallCntOver((int*)&V_I4(&res));
			}
			break;
		}
		case 313: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1392: [id(313), helpstring("method DestroyBall")] HRESULT DestroyBall([out, retval] int *pVal);
				V_VT(&res) = VT_I4;
				hres = DestroyBall((int*)&V_I4(&res));
			}
			break;
		}
		case 314: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1393: [propget, id(314), helpstring("property HitHeight")] HRESULT HitHeight([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_HitHeight(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1394: [propput, id(314), helpstring("property HitHeight")] HRESULT HitHeight([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_HitHeight(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 340: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1395: [propget, id(340), helpstring("property Material")] HRESULT Material([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Material(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1396: [propput, id(340), helpstring("property Material")] HRESULT Material([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Material(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 345: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1397: [propget, id(345), helpstring("property Rotation")] HRESULT Rotation([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Rotation(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1398: [propput, id(345), helpstring("property Rotation")] HRESULT Rotation([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Rotation(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 347: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1399: [propget, id(347), helpstring("property WireThickness")] HRESULT WireThickness([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_WireThickness(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1400: [propput, id(347), helpstring("property WireThickness")] HRESULT WireThickness([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_WireThickness(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 26: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1401: [propget, id(26), helpstring("property AnimSpeed")] HRESULT AnimSpeed([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_AnimSpeed(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1402: [propput, id(26), helpstring("property AnimSpeed")] HRESULT AnimSpeed([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_AnimSpeed(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 27: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1403: [propget, id(27), helpstring("property CurrentAnimOffset")] HRESULT CurrentAnimOffset([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_CurrentAnimOffset(&V_R4(&res));
			}
			break;
		}
		case 431: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1404: [propget, id(431), helpstring("property ReflectionEnabled")] HRESULT ReflectionEnabled([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_ReflectionEnabled(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1405: [propput, id(431), helpstring("property ReflectionEnabled")] HRESULT ReflectionEnabled([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_ReflectionEnabled(V_BOOL(&var0));
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

STDMETHODIMP Trigger::GetDocumentation(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile) {
	if (index == MEMBERID_NIL) {
		*pBstrName = SysAllocString(L"Trigger");
		return S_OK;
	}
	return E_NOTIMPL;
}

HRESULT Trigger::FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) {
	static struct {
		DISPID dispId;
		const WCHAR *name;
	} idsNamesList[] = {
			{ NULL },
			{ DISPID_GameEvents_Init, L"_Init" },
			{ DISPID_TimerEvents_Timer, L"_Timer" },
			{ DISPID_HitEvents_Hit, L"_Hit" },
			{ DISPID_HitEvents_Unhit, L"_Unhit" },
			{ DISPID_AnimateEvents_Animate, L"_Animate" }
	};

	static WCHAR wzName[MAXSTRING];
	size_t min = 1, max = ARRAY_SIZE(idsNamesList) - 1, i;
	int r;
	while(min <= max) {
		i = (min + max) / 2;
		if (idsNamesList[i].dispId == dispid) {
			wcscpy(wzName, m_wzName);
			wcscat(wzName, idsNamesList[i].name);
			LPOLESTR fnNames = (LPOLESTR)wzName;
			DISPID tDispid;
			CComPtr<IDispatch> disp;
			g_pplayer->m_ptable->m_pcv->m_pScript->GetScriptDispatch(nullptr, &disp);
			if (SUCCEEDED(disp->GetIDsOfNames(IID_NULL, &fnNames, 1, 0, &tDispid))) {
				return disp->Invoke(tDispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, pdispparams, nullptr, nullptr, nullptr);
			}
			return DISP_E_MEMBERNOTFOUND;
		}
		else if (idsNamesList[i].dispId < dispid)
		   min = i+1;
		else
		   max = i-1;
	}
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP Light::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} namesIdsList[] = {
			{ NULL },
			{ L"BlinkInterval", DISPID_Light_BlinkInterval },
			{ L"BlinkPattern", 9 },
			{ L"Bulb", 340 },
			{ L"BulbHaloHeight", 429 },
			{ L"BulbModulateVsAdd", 431 },
			{ L"Color", 3 },
			{ L"ColorFull", 457 },
			{ L"DepthBias", 397 },
			{ L"Duration", 38 },
			{ L"Fader", 458 },
			{ L"FadeSpeedDown", 437 },
			{ L"FadeSpeedUp", 377 },
			{ L"Falloff", 1 },
			{ L"FalloffPower", 432 },
			{ L"FilamentTemperature", 459 },
			{ L"GetInPlayIntensity", 597 },
			{ L"GetInPlayState", 595 },
			{ L"GetInPlayStateBool", 596 },
			{ L"Image", DISPID_Image },
			{ L"ImageMode", 453 },
			{ L"Intensity", 12 },
			{ L"IntensityScale", 434 },
			{ L"Name", DISPID_Name },
			{ L"ScaleBulbMesh", 425 },
			{ L"Shadows", 456 },
			{ L"ShowBulbMesh", 394 },
			{ L"ShowReflectionOnBall", 455 },
			{ L"State", DISPID_Light_State },
			{ L"StaticBulbMesh", 727 },
			{ L"Surface", DISPID_Surface },
			{ L"TimerEnabled", DISPID_Timer_Enabled },
			{ L"TimerInterval", DISPID_Timer_Interval },
			{ L"TransmissionScale", 617 },
			{ L"UserValue", DISPID_UserValue },
			{ L"Visible", 615 },
			{ L"X", DISPID_X },
			{ L"Y", DISPID_Y }
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

STDMETHODIMP Light::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
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
				// line 1435: [propget, id(1), helpstring("property Falloff")] HRESULT Falloff([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Falloff(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1436: [propput, id(1), helpstring("property Falloff")] HRESULT Falloff([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Falloff(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 432: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1437: [propget, id(432), helpstring("property FalloffPower")] HRESULT FalloffPower([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_FalloffPower(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1438: [propput, id(432), helpstring("property FalloffPower")] HRESULT FalloffPower([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_FalloffPower(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Light_State: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1439: [propget, id(DISPID_Light_State), helpstring("property State")] HRESULT State([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_State(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1440: [propput, id(DISPID_Light_State), helpstring("property State")] HRESULT State([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_State(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 595: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1441: [id(595), helpstring("method GetInPlayState")] HRESULT GetInPlayState([out, retval] float* pVal);
				V_VT(&res) = VT_R4;
				hres = GetInPlayState(&V_R4(&res));
			}
			break;
		}
		case 596: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1442: [id(596), helpstring("method GetInPlayStateBool")] HRESULT GetInPlayStateBool([out, retval] VARIANT_BOOL* pVal);
				V_VT(&res) = VT_BOOL;
				hres = GetInPlayStateBool(&V_BOOL(&res));
			}
			break;
		}
		case 597: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1443: [id(597), helpstring("method GetInPlayIntensity")] HRESULT GetInPlayIntensity([out, retval] float* pVal);
				V_VT(&res) = VT_R4;
				hres = GetInPlayIntensity(&V_R4(&res));
			}
			break;
		}
		case 3: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1444: [propget, id(3), helpstring("property Color")] HRESULT Color([out, retval] OLE_COLOR *pVal);
				V_VT(&res) = VT_UI4;
				hres = get_Color(&V_UI4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1445: [propput, id(3), helpstring("property Color")] HRESULT Color([in] OLE_COLOR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_UI4);
				hres = put_Color((OLE_COLOR)V_UI4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 457: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1446: [propget, id(457), helpstring("property ColorFull")] HRESULT ColorFull([out, retval] OLE_COLOR *pVal);
				V_VT(&res) = VT_UI4;
				hres = get_ColorFull(&V_UI4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1447: [propput, id(457), helpstring("property ColorFull")] HRESULT ColorFull([in] OLE_COLOR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_UI4);
				hres = put_ColorFull((OLE_COLOR)V_UI4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Timer_Enabled: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1448: [propget, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_TimerEnabled(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1449: [propput, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_TimerEnabled(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Timer_Interval: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1450: [propget, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([out, retval] long *pVal);
				V_VT(&res) = VT_I4;
				hres = get_TimerInterval((long*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1451: [propput, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([in] long newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_TimerInterval(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_X: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1452: [propget, id(DISPID_X), helpstring("property X")] HRESULT X([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_X(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1453: [propput, id(DISPID_X), helpstring("property X")] HRESULT X([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_X(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Y: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1454: [propget, id(DISPID_Y), helpstring("property Y")] HRESULT Y([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Y(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1455: [propput, id(DISPID_Y), helpstring("property Y")] HRESULT Y([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Y(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 9: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1456: [propget, id(9), helpstring("property BlinkPattern")] HRESULT BlinkPattern([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_BlinkPattern(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1457: [propput, id(9), helpstring("property BlinkPattern")] HRESULT BlinkPattern([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_BlinkPattern(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Light_BlinkInterval: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1458: [propget, id(DISPID_Light_BlinkInterval), helpstring("property BlinkInterval")] HRESULT BlinkInterval([out, retval] long *pVal);
				V_VT(&res) = VT_I4;
				hres = get_BlinkInterval((long*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1459: [propput, id(DISPID_Light_BlinkInterval), helpstring("property BlinkInterval")] HRESULT BlinkInterval([in] long newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_BlinkInterval(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 38: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1460: [id(38), helpstring("method Duration")] HRESULT Duration([in] float startState,[in] long newVal, [in] float endState);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var2;
				V_VT(&var2) = VT_EMPTY;
				VariantChangeType(&var2, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = Duration(V_R4(&var0), V_I4(&var1), V_R4(&var2));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
			}
			break;
		}
		case 12: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1461: [propget, id(12), helpstring("property Intensity")] HRESULT Intensity([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Intensity(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1462: [propput, id(12), helpstring("property Intensity")] HRESULT Intensity([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Intensity(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 617: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1463: [propget, id(617), helpstring("property TransmissionScale")] HRESULT TransmissionScale([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_TransmissionScale(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1464: [propput, id(617), helpstring("property TransmissionScale")] HRESULT TransmissionScale([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_TransmissionScale(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 434: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1465: [propget, id(434), helpstring("property IntensityScale")] HRESULT IntensityScale([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_IntensityScale(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1466: [propput, id(434), helpstring("property IntensityScale")] HRESULT IntensityScale([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_IntensityScale(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Surface: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1467: [propget, id(DISPID_Surface), helpstring("property Surface")] HRESULT Surface([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Surface(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1468: [propput, id(DISPID_Surface), helpstring("property Surface")] HRESULT Surface([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Surface(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Name: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1469: [propget, id(DISPID_Name), helpstring("property Name")] HRESULT Name([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Name(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1470: [propput, id(DISPID_Name), helpstring("property Name")] HRESULT Name([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Name(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_UserValue: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1471: [propget, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([out, retval] VARIANT* pVal);
				hres = get_UserValue(&res);
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1472: [propput, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([in] VARIANT* newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = put_UserValue(&var0);
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Image: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1473: [propget, id(DISPID_Image), helpstring("property Image")] HRESULT Image([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Image(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1474: [propput, id(DISPID_Image), helpstring("property Image")] HRESULT Image([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Image(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 453: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1475: [propget, id(453), helpstring("property ImageMode")] HRESULT ImageMode([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_ImageMode(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1476: [propput, id(453), helpstring("property ImageMode")] HRESULT ImageMode([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_ImageMode(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 397: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1477: [propget, id(397), helpstring("property DepthBias")] HRESULT DepthBias([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_DepthBias(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1478: [propput, id(397), helpstring("property DepthBias")] HRESULT DepthBias([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_DepthBias(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 377: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1479: [propget, id(377), helpstring("property FadeSpeedUp")] HRESULT FadeSpeedUp([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_FadeSpeedUp(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1480: [propput, id(377), helpstring("property FadeSpeedUp")] HRESULT FadeSpeedUp([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_FadeSpeedUp(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 437: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1481: [propget, id(437), helpstring("property FadeSpeedDown")] HRESULT FadeSpeedDown([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_FadeSpeedDown(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1482: [propput, id(437), helpstring("property FadeSpeedDown")] HRESULT FadeSpeedDown([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_FadeSpeedDown(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 340: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1483: [propget, id(340), helpstring("property Bulb")] HRESULT Bulb([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_Bulb(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1484: [propput, id(340), helpstring("property Bulb")] HRESULT Bulb([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_Bulb(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 394: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1485: [propget, id(394), helpstring("property ShowBulbMesh")] HRESULT ShowBulbMesh([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_ShowBulbMesh(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1486: [propput, id(394), helpstring("property ShowBulbMesh")] HRESULT ShowBulbMesh([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_ShowBulbMesh(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 727: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1487: [propget, id(727), helpstring("property StaticBulbMesh")] HRESULT StaticBulbMesh([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_StaticBulbMesh(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1488: [propput, id(727), helpstring("property StaticBulbMesh")] HRESULT StaticBulbMesh([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_StaticBulbMesh(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 455: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1489: [propget, id(455), helpstring("property ShowReflectionOnBall")] HRESULT ShowReflectionOnBall([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_ShowReflectionOnBall(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1490: [propput, id(455), helpstring("property ShowReflectionOnBall")] HRESULT ShowReflectionOnBall([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_ShowReflectionOnBall(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 425: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1491: [propget, id(425), helpstring("property ScaleBulbMesh")] HRESULT ScaleBulbMesh([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_ScaleBulbMesh(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1492: [propput, id(425), helpstring("property ScaleBulbMesh")] HRESULT ScaleBulbMesh([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_ScaleBulbMesh(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 431: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1493: [propget, id(431), helpstring("property BulbModulateVsAdd")] HRESULT BulbModulateVsAdd([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_BulbModulateVsAdd(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1494: [propput, id(431), helpstring("property BulbModulateVsAdd")] HRESULT BulbModulateVsAdd([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_BulbModulateVsAdd(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 429: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1495: [propget, id(429), helpstring("property BulbHaloHeight")] HRESULT BulbHaloHeight([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_BulbHaloHeight(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1496: [propput, id(429), helpstring("property BulbHaloHeight")] HRESULT BulbHaloHeight([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_BulbHaloHeight(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 615: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1497: [propget, id(615), helpstring("property Visible")] HRESULT Visible([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_Visible(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1498: [propput, id(615), helpstring("property Visible")] HRESULT Visible([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_Visible(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 456: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1499: [propget, id(456), helpstring("property Shadows")] HRESULT Shadows([out, retval] long *pVal);
				V_VT(&res) = VT_I4;
				hres = get_Shadows((long*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1500: [propput, id(456), helpstring("property Shadows")] HRESULT Shadows([in] long newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_Shadows(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 458: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1501: [propget, id(458), helpstring("property Fader")] HRESULT Fader([out, retval] long *pVal);
				V_VT(&res) = VT_I4;
				hres = get_Fader((long*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1502: [propput, id(458), helpstring("property Fader")] HRESULT Fader([in] long newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_Fader(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 459: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1503: [propget, id(459), helpstring("property FilamentTemperature")] HRESULT FilamentTemperature([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_FilamentTemperature(&V_R4(&res));
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

STDMETHODIMP Light::GetDocumentation(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile) {
	if (index == MEMBERID_NIL) {
		*pBstrName = SysAllocString(L"Light");
		return S_OK;
	}
	return E_NOTIMPL;
}

HRESULT Light::FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) {
	static struct {
		DISPID dispId;
		const WCHAR *name;
	} idsNamesList[] = {
			{ NULL },
			{ DISPID_GameEvents_Init, L"_Init" },
			{ DISPID_TimerEvents_Timer, L"_Timer" },
			{ DISPID_AnimateEvents_Animate, L"_Animate" }
	};

	static WCHAR wzName[MAXSTRING];
	size_t min = 1, max = ARRAY_SIZE(idsNamesList) - 1, i;
	int r;
	while(min <= max) {
		i = (min + max) / 2;
		if (idsNamesList[i].dispId == dispid) {
			wcscpy(wzName, m_wzName);
			wcscat(wzName, idsNamesList[i].name);
			LPOLESTR fnNames = (LPOLESTR)wzName;
			DISPID tDispid;
			CComPtr<IDispatch> disp;
			g_pplayer->m_ptable->m_pcv->m_pScript->GetScriptDispatch(nullptr, &disp);
			if (SUCCEEDED(disp->GetIDsOfNames(IID_NULL, &fnNames, 1, 0, &tDispid))) {
				return disp->Invoke(tDispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, pdispparams, nullptr, nullptr, nullptr);
			}
			return DISP_E_MEMBERNOTFOUND;
		}
		else if (idsNamesList[i].dispId < dispid)
		   min = i+1;
		else
		   max = i-1;
	}
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP Kicker::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} namesIdsList[] = {
			{ NULL },
			{ L"BallCntOver", 312 },
			{ L"CreateBall", 1 },
			{ L"CreateSizedBall", 11 },
			{ L"CreateSizedBallWithMass", 444 },
			{ L"DestroyBall", 2 },
			{ L"DrawStyle", 9 },
			{ L"Enabled", DISPID_Enabled },
			{ L"FallThrough", 394 },
			{ L"HitAccuracy", 314 },
			{ L"HitHeight", 315 },
			{ L"Kick", 5 },
			{ L"KickXYZ", 310 },
			{ L"KickZ", 311 },
			{ L"LastCapturedBall", 19 },
			{ L"Legacy", 431 },
			{ L"Material", 340 },
			{ L"Name", DISPID_Name },
			{ L"Orientation", 107 },
			{ L"Radius", 111 },
			{ L"Scatter", 115 },
			{ L"Surface", DISPID_Surface },
			{ L"TimerEnabled", DISPID_Timer_Enabled },
			{ L"TimerInterval", DISPID_Timer_Interval },
			{ L"UserValue", DISPID_UserValue },
			{ L"X", DISPID_X },
			{ L"Y", DISPID_Y }
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

STDMETHODIMP Kicker::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
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
		case DISPID_Name: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1532: [propget, id(DISPID_Name), helpstring("property Name")] HRESULT Name([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Name(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1533: [propput, id(DISPID_Name), helpstring("property Name")] HRESULT Name([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Name(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 1: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1534: [id(1), helpstring("method CreateBall")] HRESULT CreateBall([out, retval] IBall **Ball);
				V_VT(&res) = VT_DISPATCH;
				hres = CreateBall((IBall**)&V_DISPATCH(&res));
			}
			break;
		}
		case 11: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1535: [id(11), helpstring("method CreateSizedBall")] HRESULT CreateSizedBall([in] float radius, [out, retval] IBall **Ball);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				V_VT(&res) = VT_DISPATCH;
				hres = CreateSizedBall(V_R4(&var0), (IBall**)&V_DISPATCH(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 444: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1536: [id(444), helpstring("method CreateSizedBallWithMass")] HRESULT CreateSizedBallWithMass([in] float radius, [in] float mass, [out, retval] IBall **Ball);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_R4);
				V_VT(&res) = VT_DISPATCH;
				hres = CreateSizedBallWithMass(V_R4(&var0), V_R4(&var1), (IBall**)&V_DISPATCH(&res));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 2: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1537: [id(2), helpstring("method DestroyBall")] HRESULT DestroyBall([out, retval] int *pVal);
				V_VT(&res) = VT_I4;
				hres = DestroyBall((int*)&V_I4(&res));
			}
			break;
		}
		case DISPID_Timer_Enabled: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1538: [propget, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_TimerEnabled(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1539: [propput, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_TimerEnabled(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Timer_Interval: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1540: [propget, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([out, retval] long *pVal);
				V_VT(&res) = VT_I4;
				hres = get_TimerInterval((long*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1541: [propput, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([in] long newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_TimerInterval(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 5: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1542: [id(5), helpstring("method Kick")] HRESULT Kick(float angle, float speed, [defaultvalue(0)] float inclination);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var2;
				V_VT(&var2) = VT_R4;
				V_R4(&var2) = 0;
				VariantChangeType(&var2, (index > 0) ? &pDispParams->rgvarg[--index] : &var2, 0, VT_R4);
				hres = Kick(V_R4(&var0), V_R4(&var1), V_R4(&var2));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
			}
			break;
		}
		case 310: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1543: [id(310), helpstring("method KickXYZ")] HRESULT KickXYZ(float angle, float speed, float inclination, float x, float y, float z);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var2;
				V_VT(&var2) = VT_EMPTY;
				VariantChangeType(&var2, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var3;
				V_VT(&var3) = VT_EMPTY;
				VariantChangeType(&var3, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var4;
				V_VT(&var4) = VT_EMPTY;
				VariantChangeType(&var4, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var5;
				V_VT(&var5) = VT_EMPTY;
				VariantChangeType(&var5, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = KickXYZ(V_R4(&var0), V_R4(&var1), V_R4(&var2), V_R4(&var3), V_R4(&var4), V_R4(&var5));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
				VariantClear(&var3);
				VariantClear(&var4);
				VariantClear(&var5);
			}
			break;
		}
		case 311: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1544: [id(311), helpstring("method KickZ")] HRESULT KickZ(float angle, float speed, float inclination, float heightz);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var2;
				V_VT(&var2) = VT_EMPTY;
				VariantChangeType(&var2, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var3;
				V_VT(&var3) = VT_EMPTY;
				VariantChangeType(&var3, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = KickZ(V_R4(&var0), V_R4(&var1), V_R4(&var2), V_R4(&var3));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
				VariantClear(&var3);
			}
			break;
		}
		case DISPID_X: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1545: [propget, id(DISPID_X), helpstring("property X")] HRESULT X([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_X(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1546: [propput, id(DISPID_X), helpstring("property X")] HRESULT X([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_X(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Y: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1547: [propget, id(DISPID_Y), helpstring("property Y")] HRESULT Y([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Y(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1548: [propput, id(DISPID_Y), helpstring("property Y")] HRESULT Y([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Y(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Enabled: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1549: [propget, id(DISPID_Enabled), helpstring("property Enabled")] HRESULT Enabled([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_Enabled(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1550: [propput, id(DISPID_Enabled), helpstring("property Enabled")] HRESULT Enabled([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_Enabled(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 9: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1551: [propget, id(9), helpstring("property DrawStyle")] HRESULT DrawStyle([out, retval] KickerType *pVal);
				V_VT(&res) = VT_I4;
				hres = get_DrawStyle((KickerType*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1552: [propput, id(9), helpstring("property DrawStyle")] HRESULT DrawStyle([in] KickerType newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_DrawStyle((KickerType)V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 340: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1553: [propget, id(340), helpstring("property Material")] HRESULT Material([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Material(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1554: [propput, id(340), helpstring("property Material")] HRESULT Material([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Material(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Surface: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1555: [propget, id(DISPID_Surface), helpstring("property Surface")] HRESULT Surface([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Surface(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1556: [propput, id(DISPID_Surface), helpstring("property Surface")] HRESULT Surface([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Surface(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_UserValue: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1557: [propget, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([out, retval] VARIANT* pVal);
				hres = get_UserValue(&res);
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1558: [propput, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([in] VARIANT* newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = put_UserValue(&var0);
				VariantClear(&var0);
			}
			break;
		}
		case 312: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1559: [id(312), helpstring("method BallCntOver")] HRESULT BallCntOver([out, retval] int *pVal);
				V_VT(&res) = VT_I4;
				hres = BallCntOver((int*)&V_I4(&res));
			}
			break;
		}
		case 115: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1560: [propget, id(115), helpstring("property Scatter")] HRESULT Scatter([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Scatter(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1561: [propput, id(115), helpstring("property Scatter")] HRESULT Scatter([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Scatter(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 314: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1562: [propget, id(314), helpstring("property HitAccuracy")] HRESULT HitAccuracy([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_HitAccuracy(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1563: [propput, id(314), helpstring("property HitAccuracy")] HRESULT HitAccuracy([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_HitAccuracy(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 315: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1564: [propget, id(315), helpstring("property HitHeight")] HRESULT HitHeight([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_HitHeight(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1565: [propput, id(315), helpstring("property HitHeight")] HRESULT HitHeight([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_HitHeight(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 107: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1566: [propget, id(107), helpstring("property Orientation")] HRESULT Orientation([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Orientation(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1567: [propput, id(107), helpstring("property Orientation")] HRESULT Orientation([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Orientation(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 111: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1568: [propget, id(111), helpstring("property Radius")] HRESULT Radius([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Radius(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1569: [propput, id(111), helpstring("property Radius")] HRESULT Radius([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Radius(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 394: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1570: [propget, id(394), helpstring("property FallThrough")] HRESULT FallThrough([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_FallThrough(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1571: [propput, id(394), helpstring("property FallThrough")] HRESULT FallThrough([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_FallThrough(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 431: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1572: [propget, id(431), helpstring("property Legacy")] HRESULT Legacy([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_Legacy(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1573: [propput, id(431), helpstring("property Legacy")] HRESULT Legacy([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_Legacy(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 19: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1574: [propget, id(19), helpstring("property LastCapturedBall")] HRESULT LastCapturedBall([out, retval] IBall **pVal);
				V_VT(&res) = VT_DISPATCH;
				hres = get_LastCapturedBall((IBall**)&V_DISPATCH(&res));
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

STDMETHODIMP Kicker::GetDocumentation(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile) {
	if (index == MEMBERID_NIL) {
		*pBstrName = SysAllocString(L"Kicker");
		return S_OK;
	}
	return E_NOTIMPL;
}

HRESULT Kicker::FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) {
	static struct {
		DISPID dispId;
		const WCHAR *name;
	} idsNamesList[] = {
			{ NULL },
			{ DISPID_GameEvents_Init, L"_Init" },
			{ DISPID_TimerEvents_Timer, L"_Timer" },
			{ DISPID_HitEvents_Hit, L"_Hit" },
			{ DISPID_HitEvents_Unhit, L"_Unhit" }
	};

	static WCHAR wzName[MAXSTRING];
	size_t min = 1, max = ARRAY_SIZE(idsNamesList) - 1, i;
	int r;
	while(min <= max) {
		i = (min + max) / 2;
		if (idsNamesList[i].dispId == dispid) {
			wcscpy(wzName, m_wzName);
			wcscat(wzName, idsNamesList[i].name);
			LPOLESTR fnNames = (LPOLESTR)wzName;
			DISPID tDispid;
			CComPtr<IDispatch> disp;
			g_pplayer->m_ptable->m_pcv->m_pScript->GetScriptDispatch(nullptr, &disp);
			if (SUCCEEDED(disp->GetIDsOfNames(IID_NULL, &fnNames, 1, 0, &tDispid))) {
				return disp->Invoke(tDispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, pdispparams, nullptr, nullptr, nullptr);
			}
			return DISP_E_MEMBERNOTFOUND;
		}
		else if (idsNamesList[i].dispId < dispid)
		   min = i+1;
		else
		   max = i-1;
	}
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP Decal::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} namesIdsList[] = {
			{ NULL },
			{ L"Font", DISPID_Decal_Font },
			{ L"FontColor", 11 },
			{ L"HasVerticalText", 13 },
			{ L"Height", 4 },
			{ L"Image", DISPID_Image },
			{ L"Material", 340 },
			{ L"Rotation", 1 },
			{ L"SizingType", DISPID_Decal_SizingType },
			{ L"Surface", DISPID_Surface },
			{ L"Text", 8 },
			{ L"Type", 7 },
			{ L"Width", 3 },
			{ L"X", 5 },
			{ L"Y", 6 }
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

STDMETHODIMP Decal::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
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
				// line 1606: [propget, id(1), helpstring("property Rotation")] HRESULT Rotation([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Rotation(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1607: [propput, id(1), helpstring("property Rotation")] HRESULT Rotation([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Rotation(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Image: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1608: [propget, id(DISPID_Image), helpstring("property Image")] HRESULT Image([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Image(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1609: [propput, id(DISPID_Image), helpstring("property Image")] HRESULT Image([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Image(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 3: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1610: [propget, id(3), helpstring("property Width")] HRESULT Width([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Width(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1611: [propput, id(3), helpstring("property Width")] HRESULT Width([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Width(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 4: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1612: [propget, id(4), helpstring("property Height")] HRESULT Height([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Height(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1613: [propput, id(4), helpstring("property Height")] HRESULT Height([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Height(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 5: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1614: [propget, id(5), helpstring("property X")] HRESULT X([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_X(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1615: [propput, id(5), helpstring("property X")] HRESULT X([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_X(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 6: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1616: [propget, id(6), helpstring("property Y")] HRESULT Y([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Y(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1617: [propput, id(6), helpstring("property Y")] HRESULT Y([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Y(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 7: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1618: [propget, id(7), helpstring("property Type")] HRESULT Type([out, retval] DecalType *pVal);
				V_VT(&res) = VT_I4;
				hres = get_Type((DecalType*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1619: [propput, id(7), helpstring("property Type")] HRESULT Type([in] DecalType newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_Type((DecalType)V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 8: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1620: [propget, id(8), helpstring("property Text")] HRESULT Text([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Text(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1621: [propput, id(8), helpstring("property Text")] HRESULT Text([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Text(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Decal_SizingType: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1622: [propget, id(DISPID_Decal_SizingType), helpstring("property SizingType")] HRESULT SizingType([out, retval] SizingType *pVal);
				V_VT(&res) = VT_I4;
				hres = get_SizingType((SizingType*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1623: [propput, id(DISPID_Decal_SizingType), helpstring("property SizingType")] HRESULT SizingType([in] SizingType newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_SizingType((SizingType)V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 11: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1624: [propget, id(11), helpstring("property FontColor")] HRESULT FontColor([out, retval] OLE_COLOR *pVal);
				V_VT(&res) = VT_UI4;
				hres = get_FontColor(&V_UI4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1625: [propput, id(11), helpstring("property FontColor")] HRESULT FontColor([in] OLE_COLOR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_UI4);
				hres = put_FontColor((OLE_COLOR)V_UI4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 340: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1626: [propget, id(340), helpstring("property Material")] HRESULT Material([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Material(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1627: [propput, id(340), helpstring("property Material")] HRESULT Material([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Material(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Decal_Font: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1628: [propget, id(DISPID_Decal_Font), helpstring("property Font")] HRESULT Font([out, retval] IFontDisp **pVal);
				V_VT(&res) = VT_DISPATCH;
				hres = get_Font((IFontDisp**)&V_DISPATCH(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUTREF) {
				// line 1629: [propputref, id(DISPID_Decal_Font), helpstring("property Font")] HRESULT Font([in] IFontDisp *newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = putref_Font((IFontDisp*)&var0);
				VariantClear(&var0);
			}
			break;
		}
		case 13: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1630: [propget, id(13), helpstring("property HasVerticalText")] HRESULT HasVerticalText([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_HasVerticalText(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1631: [propput, id(13), helpstring("property HasVerticalText")] HRESULT HasVerticalText([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_HasVerticalText(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Surface: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1632: [propget, id(DISPID_Surface), helpstring("property Surface")] HRESULT Surface([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Surface(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1633: [propput, id(DISPID_Surface), helpstring("property Surface")] HRESULT Surface([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Surface(V_BSTR(&var0));
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

STDMETHODIMP Decal::GetDocumentation(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile) {
	if (index == MEMBERID_NIL) {
		*pBstrName = SysAllocString(L"Decal");
		return S_OK;
	}
	return E_NOTIMPL;
}

STDMETHODIMP Primitive::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} namesIdsList[] = {
			{ NULL },
			{ L"AddBlend", 556 },
			{ L"BackfacesEnabled", 792 },
			{ L"BlendDisableLighting", 494 },
			{ L"BlendDisableLightingFromBelow", 496 },
			{ L"Collidable", 111 },
			{ L"CollisionReductionFactor", 481 },
			{ L"Color", 557 },
			{ L"ContinueAnim", 35 },
			{ L"DepthBias", 397 },
			{ L"DisableLighting", 441 },
			{ L"DisplayTexture", 38 },
			{ L"DrawTexturesInside", 106 },
			{ L"EdgeFactorUI", 454 },
			{ L"Elasticity", 110 },
			{ L"ElasticityFalloff", 112 },
			{ L"EnableDepthMask", 558 },
			{ L"EnableStaticRendering", 398 },
			{ L"Friction", 114 },
			{ L"HasHitEvent", 34 },
			{ L"HitThreshold", 735 },
			{ L"Image", DISPID_Image },
			{ L"IsToy", 395 },
			{ L"Material", 340 },
			{ L"Name", DISPID_Name },
			{ L"NormalMap", DISPID_Image2 },
			{ L"ObjectSpaceNormalMap", 824 },
			{ L"ObjRotX", DISPID_OBJROT_X },
			{ L"ObjRotY", DISPID_OBJROT_Y },
			{ L"ObjRotZ", DISPID_OBJROT_Z },
			{ L"Opacity", 377 },
			{ L"OverwritePhysics", 432 },
			{ L"PhysicsMaterial", 734 },
			{ L"PlayAnim", 1 },
			{ L"PlayAnimEndless", 2 },
			{ L"ReflectionEnabled", 431 },
			{ L"ReflectionProbe", 559 },
			{ L"RefractionProbe", 560 },
			{ L"RotAndTra0", DISPID_ROTRA1 },
			{ L"RotAndTra1", DISPID_ROTRA2 },
			{ L"RotAndTra2", DISPID_ROTRA3 },
			{ L"RotAndTra3", DISPID_ROTRA4 },
			{ L"RotAndTra4", DISPID_ROTRA5 },
			{ L"RotAndTra5", DISPID_ROTRA6 },
			{ L"RotAndTra6", DISPID_ROTRA7 },
			{ L"RotAndTra7", DISPID_ROTRA8 },
			{ L"RotAndTra8", DISPID_ROTRA9 },
			{ L"RotX", DISPID_ROT_X },
			{ L"RotY", DISPID_ROT_Y },
			{ L"RotZ", DISPID_ROT_Z },
			{ L"Scatter", 115 },
			{ L"ShowFrame", 19 },
			{ L"SideColor", 104 },
			{ L"Sides", 101 },
			{ L"Size_X", DISPID_SIZE_X },
			{ L"Size_Y", DISPID_SIZE_Y },
			{ L"Size_Z", DISPID_SIZE_Z },
			{ L"StopAnim", 18 },
			{ L"Threshold", 33 },
			{ L"TransX", DISPID_TRANS_X },
			{ L"TransY", DISPID_TRANS_Y },
			{ L"TransZ", DISPID_TRANS_Z },
			{ L"UserValue", DISPID_UserValue },
			{ L"Visible", 458 },
			{ L"X", DISPID_POSITION_X },
			{ L"Y", DISPID_POSITION_Y },
			{ L"Z", DISPID_POSITION_Z }
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

STDMETHODIMP Primitive::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
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
				// line 1651: [id(1), helpstring("method PlayAnim")] HRESULT PlayAnim([in]float startFrame, [in]float speed);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = PlayAnim(V_R4(&var0), V_R4(&var1));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 2: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1652: [id(2), helpstring("method PlayAnimEndless")] HRESULT PlayAnimEndless([in]float speed);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = PlayAnimEndless(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 18: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1653: [id(18), helpstring("method StopAnim")] HRESULT StopAnim();
				hres = StopAnim();
			}
			break;
		}
		case 19: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1654: [id(19), helpstring("method ShowFrame")] HRESULT ShowFrame([in]float frame);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = ShowFrame(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 35: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1655: [id(35), helpstring("method ContinueAnim")] HRESULT ContinueAnim([in]float speed);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = ContinueAnim(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 38: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1656: [propget, id(38), helpstring("property DisplayTexture")] HRESULT DisplayTexture([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_DisplayTexture(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1657: [propput, id(38), helpstring("property DisplayTexture")] HRESULT DisplayTexture([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_DisplayTexture(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 101: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1659: [propget, id(101), helpstring("property Sides")] HRESULT Sides([out, retval] int *pVal);
				V_VT(&res) = VT_I4;
				hres = get_Sides((int*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1660: [propput, id(101), helpstring("property Sides")] HRESULT Sides([in] int newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_Sides(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 458: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1661: [propget, id(458), helpstring("property Top Visible")] HRESULT Visible([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_Visible(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1662: [propput, id(458), helpstring("property Top Visible")] HRESULT Visible([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_Visible(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 340: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1663: [propget, id(340), helpstring("property Material")] HRESULT Material([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Material(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1664: [propput, id(340), helpstring("property Material")] HRESULT Material([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Material(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 104: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1665: [propget, id(104), helpstring("property Side Color")] HRESULT SideColor([out, retval] OLE_COLOR *pVal);
				V_VT(&res) = VT_UI4;
				hres = get_SideColor(&V_UI4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1666: [propput, id(104), helpstring("property Side Color")] HRESULT SideColor([in] OLE_COLOR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_UI4);
				hres = put_SideColor((OLE_COLOR)V_UI4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 106: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1667: [propget, id(106), helpstring("property Draw Textures Inside")] HRESULT DrawTexturesInside([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_DrawTexturesInside(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1668: [propput, id(106), helpstring("property Draw Textures Inside")] HRESULT DrawTexturesInside([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_DrawTexturesInside(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_UserValue: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1670: [propget, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([out, retval] VARIANT* pVal);
				hres = get_UserValue(&res);
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1671: [propput, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([in] VARIANT* newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = put_UserValue(&var0);
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Image: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1673: [propget, id(DISPID_Image), helpstring("property Image")] HRESULT Image([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Image(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1674: [propput, id(DISPID_Image), helpstring("property Image")] HRESULT Image([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Image(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Image2: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1675: [propget, id(DISPID_Image2), helpstring("property NormalMap")] HRESULT NormalMap([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_NormalMap(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1676: [propput, id(DISPID_Image2), helpstring("property NormalMap")] HRESULT NormalMap([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_NormalMap(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_POSITION_X: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1678: [propget, id(DISPID_POSITION_X), helpstring("property X")] HRESULT X([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_X(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1679: [propput, id(DISPID_POSITION_X), helpstring("property X")] HRESULT X([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_X(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_POSITION_Y: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1680: [propget, id(DISPID_POSITION_Y), helpstring("property Y")] HRESULT Y([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Y(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1681: [propput, id(DISPID_POSITION_Y), helpstring("property Y")] HRESULT Y([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Y(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_POSITION_Z: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1682: [propget, id(DISPID_POSITION_Z), helpstring("property Z")] HRESULT Z([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Z(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1683: [propput, id(DISPID_POSITION_Z), helpstring("property Z")] HRESULT Z([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Z(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_SIZE_X: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1684: [propget, id(DISPID_SIZE_X), helpstring("property Size X")] HRESULT Size_X([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Size_X(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1685: [propput, id(DISPID_SIZE_X), helpstring("property Size X")] HRESULT Size_X([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Size_X(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_SIZE_Y: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1686: [propget, id(DISPID_SIZE_Y), helpstring("property Size Y")] HRESULT Size_Y([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Size_Y(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1687: [propput, id(DISPID_SIZE_Y), helpstring("property Size Y")] HRESULT Size_Y([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Size_Y(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_SIZE_Z: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1688: [propget, id(DISPID_SIZE_Z), helpstring("property Size Z")] HRESULT Size_Z([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Size_Z(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1689: [propput, id(DISPID_SIZE_Z), helpstring("property Size Z")] HRESULT Size_Z([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Size_Z(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_ROTRA1: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1691: [propget, id(DISPID_ROTRA1), helpstring("property Rot and Tra 1")] HRESULT RotAndTra0([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_RotAndTra0(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1692: [propput, id(DISPID_ROTRA1), helpstring("property Rot and Tra 1")] HRESULT RotAndTra0([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_RotAndTra0(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_ROT_X: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1693: [propget, id(DISPID_ROT_X), helpstring("property RotX")] HRESULT RotX([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_RotX(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1694: [propput, id(DISPID_ROT_X), helpstring("property RotX")] HRESULT RotX([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_RotX(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_ROTRA2: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1696: [propget, id(DISPID_ROTRA2), helpstring("property Rot and Tra 2")] HRESULT RotAndTra1([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_RotAndTra1(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1697: [propput, id(DISPID_ROTRA2), helpstring("property Rot and Tra 2")] HRESULT RotAndTra1([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_RotAndTra1(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_ROT_Y: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1698: [propget, id(DISPID_ROT_Y), helpstring("property RotY")] HRESULT RotY([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_RotY(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1699: [propput, id(DISPID_ROT_Y), helpstring("property RotY")] HRESULT RotY([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_RotY(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_ROTRA3: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1701: [propget, id(DISPID_ROTRA3), helpstring("property Rot and Tra 3")] HRESULT RotAndTra2([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_RotAndTra2(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1702: [propput, id(DISPID_ROTRA3), helpstring("property Rot and Tra 3")] HRESULT RotAndTra2([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_RotAndTra2(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_ROT_Z: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1703: [propget, id(DISPID_ROT_Z), helpstring("property RotZ")] HRESULT RotZ([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_RotZ(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1704: [propput, id(DISPID_ROT_Z), helpstring("property RotZ")] HRESULT RotZ([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_RotZ(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_ROTRA4: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1706: [propget, id(DISPID_ROTRA4), helpstring("property Rot and Tra 4")] HRESULT RotAndTra3([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_RotAndTra3(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1707: [propput, id(DISPID_ROTRA4), helpstring("property Rot and Tra 4")] HRESULT RotAndTra3([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_RotAndTra3(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_TRANS_X: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1708: [propget, id(DISPID_TRANS_X), helpstring("property TransX")] HRESULT TransX([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_TransX(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1709: [propput, id(DISPID_TRANS_X), helpstring("property TransX")] HRESULT TransX([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_TransX(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_ROTRA5: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1711: [propget, id(DISPID_ROTRA5), helpstring("property Rot and Tra 5")] HRESULT RotAndTra4([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_RotAndTra4(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1712: [propput, id(DISPID_ROTRA5), helpstring("property Rot and Tra 5")] HRESULT RotAndTra4([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_RotAndTra4(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_TRANS_Y: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1713: [propget, id(DISPID_TRANS_Y), helpstring("property TransY")] HRESULT TransY([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_TransY(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1714: [propput, id(DISPID_TRANS_Y), helpstring("property TransY")] HRESULT TransY([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_TransY(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_ROTRA6: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1716: [propget, id(DISPID_ROTRA6), helpstring("property Rot and Tra 6")] HRESULT RotAndTra5([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_RotAndTra5(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1717: [propput, id(DISPID_ROTRA6), helpstring("property Rot and Tra 6")] HRESULT RotAndTra5([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_RotAndTra5(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_TRANS_Z: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1718: [propget, id(DISPID_TRANS_Z), helpstring("property TransZ")] HRESULT TransZ([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_TransZ(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1719: [propput, id(DISPID_TRANS_Z), helpstring("property TransZ")] HRESULT TransZ([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_TransZ(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_ROTRA7: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1721: [propget, id(DISPID_ROTRA7), helpstring("property Rot and Tra 7")] HRESULT RotAndTra6([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_RotAndTra6(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1722: [propput, id(DISPID_ROTRA7), helpstring("property Rot and Tra 7")] HRESULT RotAndTra6([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_RotAndTra6(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_OBJROT_X: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1723: [propget, id(DISPID_OBJROT_X), helpstring("property ObjRotX")] HRESULT ObjRotX([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_ObjRotX(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1724: [propput, id(DISPID_OBJROT_X), helpstring("property ObjRotX")] HRESULT ObjRotX([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_ObjRotX(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_ROTRA8: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1726: [propget, id(DISPID_ROTRA8), helpstring("property Rot and Tra 8")] HRESULT RotAndTra7([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_RotAndTra7(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1727: [propput, id(DISPID_ROTRA8), helpstring("property Rot and Tra 8")] HRESULT RotAndTra7([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_RotAndTra7(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_OBJROT_Y: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1728: [propget, id(DISPID_OBJROT_Y), helpstring("property ObjRotY")] HRESULT ObjRotY([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_ObjRotY(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1729: [propput, id(DISPID_OBJROT_Y), helpstring("property ObjRotY")] HRESULT ObjRotY([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_ObjRotY(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_ROTRA9: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1731: [propget, id(DISPID_ROTRA9), helpstring("property Rot and Tra 9")] HRESULT RotAndTra8([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_RotAndTra8(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1732: [propput, id(DISPID_ROTRA9), helpstring("property Rot and Tra 9")] HRESULT RotAndTra8([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_RotAndTra8(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_OBJROT_Z: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1733: [propget, id(DISPID_OBJROT_Z), helpstring("property ObjRotZ")] HRESULT ObjRotZ([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_ObjRotZ(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1734: [propput, id(DISPID_OBJROT_Z), helpstring("property ObjRotZ")] HRESULT ObjRotZ([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_ObjRotZ(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 454: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1736: [propget, id(454), helpstring("property EdgeFactorUI")] HRESULT EdgeFactorUI([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_EdgeFactorUI(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1737: [propput, id(454), helpstring("property EdgeFactorUI")] HRESULT EdgeFactorUI([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_EdgeFactorUI(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 481: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1738: [propget, id(481), helpstring("property CollisionReductionFactor")] HRESULT CollisionReductionFactor([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_CollisionReductionFactor(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1739: [propput, id(481), helpstring("property CollisionReductionFactor")] HRESULT CollisionReductionFactor([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_CollisionReductionFactor(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Name: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1741: [propget, id(DISPID_Name), helpstring("property Name")] HRESULT Name([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Name(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1742: [propput, id(DISPID_Name), helpstring("property Name")] HRESULT Name([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Name(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 398: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1743: [propget, id(398), helpstring("property static rendering")] HRESULT EnableStaticRendering([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_EnableStaticRendering(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1744: [propput, id(398), helpstring("property static rendering")] HRESULT EnableStaticRendering([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_EnableStaticRendering(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 110: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1746: [propget, id(110), helpstring("property Elasticity")] HRESULT Elasticity([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Elasticity(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1747: [propput, id(110), helpstring("property Elasticity")] HRESULT Elasticity([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Elasticity(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 112: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1748: [propget, id(112), helpstring("property ElasticityFalloff")] HRESULT ElasticityFalloff([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_ElasticityFalloff(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1749: [propput, id(112), helpstring("property ElasticityFalloff")] HRESULT ElasticityFalloff([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_ElasticityFalloff(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 111: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1751: [propget, id(111), helpstring("property Collidable")] HRESULT Collidable([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_Collidable(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1752: [propput, id(111), helpstring("property Collidable")] HRESULT Collidable([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_Collidable(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 395: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1753: [propget, id(395), helpstring("property IsToy")] HRESULT IsToy([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_IsToy(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1754: [propput, id(395), helpstring("property IsToy")] HRESULT IsToy([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_IsToy(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 792: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1755: [propget, id(792), helpstring("property BackfacesEnabled")] HRESULT BackfacesEnabled([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_BackfacesEnabled(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1756: [propput, id(792), helpstring("property BackfacesEnabled")] HRESULT BackfacesEnabled([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_BackfacesEnabled(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 34: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1757: [propget, id(34), helpstring("property HasHitEvent")] HRESULT HasHitEvent([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_HasHitEvent(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1758: [propput, id(34), helpstring("property HasHitEvent")] HRESULT HasHitEvent([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_HasHitEvent(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 33: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1759: [propget, id(33), helpstring("property Threshold")] HRESULT Threshold([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Threshold(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1760: [propput, id(33), helpstring("property Threshold")] HRESULT Threshold([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Threshold(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 114: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1761: [propget, id(114), helpstring("property Friction")] HRESULT Friction([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Friction(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1762: [propput, id(114), helpstring("property Friction")] HRESULT Friction([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Friction(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 115: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1763: [propget, id(115), helpstring("property Scatter")] HRESULT Scatter([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Scatter(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1764: [propput, id(115), helpstring("property Scatter")] HRESULT Scatter([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Scatter(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 441: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1765: [propget, id(441), helpstring("property DisableLighting")] HRESULT DisableLighting([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_DisableLighting(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1766: [propput, id(441), helpstring("property DisableLighting")] HRESULT DisableLighting([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_DisableLighting(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 494: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1767: [propget, id(494), helpstring("property BlendDisableLighting")] HRESULT BlendDisableLighting([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_BlendDisableLighting(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1768: [propput, id(494), helpstring("property BlendDisableLighting")] HRESULT BlendDisableLighting([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_BlendDisableLighting(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 496: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1769: [propget, id(496), helpstring("property BlendDisableLightingFromBelow")] HRESULT BlendDisableLightingFromBelow([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_BlendDisableLightingFromBelow(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1770: [propput, id(496), helpstring("property BlendDisableLightingFromBelow")] HRESULT BlendDisableLightingFromBelow([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_BlendDisableLightingFromBelow(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 431: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1771: [propget, id(431), helpstring("property ReflectionEnabled")] HRESULT ReflectionEnabled([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_ReflectionEnabled(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1772: [propput, id(431), helpstring("property ReflectionEnabled")] HRESULT ReflectionEnabled([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_ReflectionEnabled(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 377: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1774: [propget, id(377), helpstring("property Opacity")] HRESULT Opacity([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Opacity(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1775: [propput, id(377), helpstring("property Opacity")] HRESULT Opacity([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Opacity(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 556: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1776: [propget, id(556), helpstring("property AddBlend")] HRESULT AddBlend([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_AddBlend(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1777: [propput, id(556), helpstring("property AddBlend")] HRESULT AddBlend([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_AddBlend(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 557: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1778: [propget, id(557), helpstring("property Color")] HRESULT Color([out, retval] OLE_COLOR *pVal);
				V_VT(&res) = VT_UI4;
				hres = get_Color(&V_UI4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1779: [propput, id(557), helpstring("property Color")] HRESULT Color([in] OLE_COLOR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_UI4);
				hres = put_Color((OLE_COLOR)V_UI4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 558: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1781: [propget, id(558), helpstring("property EnableDepthMask")] HRESULT EnableDepthMask([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_EnableDepthMask(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1782: [propput, id(558), helpstring("property EnableDepthMask")] HRESULT EnableDepthMask([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_EnableDepthMask(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 559: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1784: [propget, id(559), helpstring("property ReflectionProbe")] HRESULT ReflectionProbe([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_ReflectionProbe(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1785: [propput, id(559), helpstring("property ReflectionProbe")] HRESULT ReflectionProbe([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_ReflectionProbe(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 560: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1786: [propget, id(560), helpstring("property RefractionProbe")] HRESULT RefractionProbe([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_RefractionProbe(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1787: [propput, id(560), helpstring("property RefractionProbe")] HRESULT RefractionProbe([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_RefractionProbe(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 397: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1789: [propget, id(397), helpstring("property DepthBias")] HRESULT DepthBias([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_DepthBias(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1790: [propput, id(397), helpstring("property DepthBias")] HRESULT DepthBias([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_DepthBias(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 734: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1792: [propget, id(734), helpstring("property PhysicsMaterial")] HRESULT PhysicsMaterial([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_PhysicsMaterial(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1793: [propput, id(734), helpstring("property PhysicsMaterial")] HRESULT PhysicsMaterial([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_PhysicsMaterial(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 432: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1794: [propget, id(432), helpstring("property OverwritePhysics")] HRESULT OverwritePhysics([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_OverwritePhysics(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1795: [propput, id(432), helpstring("property OverwritePhysics")] HRESULT OverwritePhysics([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_OverwritePhysics(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 735: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1796: [propget, id(735), helpstring("property HitThreshold")] HRESULT HitThreshold([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_HitThreshold(&V_R4(&res));
			}
			break;
		}
		case 824: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1797: [propget, id(824), helpstring("property ObjectSpaceNormalMap")] HRESULT ObjectSpaceNormalMap([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_ObjectSpaceNormalMap(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1798: [propput, id(824), helpstring("property ObjectSpaceNormalMap")] HRESULT ObjectSpaceNormalMap([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_ObjectSpaceNormalMap(V_BOOL(&var0));
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

STDMETHODIMP Primitive::GetDocumentation(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile) {
	if (index == MEMBERID_NIL) {
		*pBstrName = SysAllocString(L"Primitive");
		return S_OK;
	}
	return E_NOTIMPL;
}

HRESULT Primitive::FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) {
	static struct {
		DISPID dispId;
		const WCHAR *name;
	} idsNamesList[] = {
			{ NULL },
			{ DISPID_GameEvents_Init, L"_Init" },
			{ DISPID_HitEvents_Hit, L"_Hit" }
	};

	static WCHAR wzName[MAXSTRING];
	size_t min = 1, max = ARRAY_SIZE(idsNamesList) - 1, i;
	int r;
	while(min <= max) {
		i = (min + max) / 2;
		if (idsNamesList[i].dispId == dispid) {
			wcscpy(wzName, m_wzName);
			wcscat(wzName, idsNamesList[i].name);
			LPOLESTR fnNames = (LPOLESTR)wzName;
			DISPID tDispid;
			CComPtr<IDispatch> disp;
			g_pplayer->m_ptable->m_pcv->m_pScript->GetScriptDispatch(nullptr, &disp);
			if (SUCCEEDED(disp->GetIDsOfNames(IID_NULL, &fnNames, 1, 0, &tDispid))) {
				return disp->Invoke(tDispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, pdispparams, nullptr, nullptr, nullptr);
			}
			return DISP_E_MEMBERNOTFOUND;
		}
		else if (idsNamesList[i].dispId < dispid)
		   min = i+1;
		else
		   max = i-1;
	}
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP HitTarget::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} namesIdsList[] = {
			{ NULL },
			{ L"BlendDisableLighting", 494 },
			{ L"BlendDisableLightingFromBelow", 496 },
			{ L"Collidable", 111 },
			{ L"CurrentAnimOffset", 27 },
			{ L"DepthBias", 397 },
			{ L"DisableLighting", 483 },
			{ L"DrawStyle", 9 },
			{ L"DropSpeed", 377 },
			{ L"Elasticity", 110 },
			{ L"ElasticityFalloff", 112 },
			{ L"Friction", 114 },
			{ L"HasHitEvent", 34 },
			{ L"HitThreshold", 727 },
			{ L"Image", DISPID_Image },
			{ L"IsDropped", 435 },
			{ L"LegacyMode", 433 },
			{ L"Material", 340 },
			{ L"Name", DISPID_Name },
			{ L"Orientation", DISPID_ROT_Z },
			{ L"OverwritePhysics", 432 },
			{ L"PhysicsMaterial", 734 },
			{ L"RaiseDelay", 726 },
			{ L"ReflectionEnabled", 431 },
			{ L"ScaleX", DISPID_SIZE_X },
			{ L"ScaleY", DISPID_SIZE_Y },
			{ L"ScaleZ", DISPID_SIZE_Z },
			{ L"Scatter", 115 },
			{ L"Threshold", 33 },
			{ L"TimerEnabled", DISPID_Timer_Enabled },
			{ L"TimerInterval", DISPID_Timer_Interval },
			{ L"UserValue", DISPID_UserValue },
			{ L"Visible", 458 },
			{ L"X", DISPID_POSITION_X },
			{ L"Y", DISPID_POSITION_Y },
			{ L"Z", DISPID_POSITION_Z }
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

STDMETHODIMP HitTarget::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
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
		case DISPID_Timer_Enabled: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1828: [propget, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_TimerEnabled(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1829: [propput, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_TimerEnabled(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Timer_Interval: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1830: [propget, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([out, retval] long *pVal);
				V_VT(&res) = VT_I4;
				hres = get_TimerInterval((long*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1831: [propput, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([in] long newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_TimerInterval(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 458: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1832: [propget, id(458), helpstring("property Top Visible")] HRESULT Visible([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_Visible(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1833: [propput, id(458), helpstring("property Top Visible")] HRESULT Visible([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_Visible(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 340: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1834: [propget, id(340), helpstring("property Material")] HRESULT Material([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Material(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1835: [propput, id(340), helpstring("property Material")] HRESULT Material([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Material(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_UserValue: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1836: [propget, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([out, retval] VARIANT* pVal);
				hres = get_UserValue(&res);
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1837: [propput, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([in] VARIANT* newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = put_UserValue(&var0);
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Image: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1839: [propget, id(DISPID_Image), helpstring("property Image")] HRESULT Image([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Image(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1840: [propput, id(DISPID_Image), helpstring("property Image")] HRESULT Image([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Image(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_POSITION_X: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1842: [propget, id(DISPID_POSITION_X), helpstring("property X")] HRESULT X([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_X(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1843: [propput, id(DISPID_POSITION_X), helpstring("property X")] HRESULT X([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_X(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_POSITION_Y: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1844: [propget, id(DISPID_POSITION_Y), helpstring("property Y")] HRESULT Y([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Y(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1845: [propput, id(DISPID_POSITION_Y), helpstring("property Y")] HRESULT Y([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Y(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_POSITION_Z: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1846: [propget, id(DISPID_POSITION_Z), helpstring("property Z")] HRESULT Z([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Z(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1847: [propput, id(DISPID_POSITION_Z), helpstring("property Z")] HRESULT Z([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Z(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_SIZE_X: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1848: [propget, id(DISPID_SIZE_X), helpstring("property ScaleX")] HRESULT ScaleX([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_ScaleX(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1849: [propput, id(DISPID_SIZE_X), helpstring("property ScaleX")] HRESULT ScaleX([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_ScaleX(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_SIZE_Y: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1850: [propget, id(DISPID_SIZE_Y), helpstring("property ScaleY")] HRESULT ScaleY([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_ScaleY(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1851: [propput, id(DISPID_SIZE_Y), helpstring("property ScaleY")] HRESULT ScaleY([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_ScaleY(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_SIZE_Z: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1852: [propget, id(DISPID_SIZE_Z), helpstring("property ScaleZ")] HRESULT ScaleZ([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_ScaleZ(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1853: [propput, id(DISPID_SIZE_Z), helpstring("property ScaleZ")] HRESULT ScaleZ([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_ScaleZ(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_ROT_Z: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1855: [propget, id(DISPID_ROT_Z), helpstring("property Orientation")] HRESULT Orientation([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Orientation(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1856: [propput, id(DISPID_ROT_Z), helpstring("property Orientation")] HRESULT Orientation([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Orientation(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Name: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1858: [propget, id(DISPID_Name), helpstring("property Name")] HRESULT Name([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Name(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1859: [propput, id(DISPID_Name), helpstring("property Name")] HRESULT Name([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Name(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 110: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1861: [propget, id(110), helpstring("property Elasticity")] HRESULT Elasticity([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Elasticity(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1862: [propput, id(110), helpstring("property Elasticity")] HRESULT Elasticity([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Elasticity(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 112: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1863: [propget, id(112), helpstring("property ElasticityFalloff")] HRESULT ElasticityFalloff([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_ElasticityFalloff(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1864: [propput, id(112), helpstring("property ElasticityFalloff")] HRESULT ElasticityFalloff([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_ElasticityFalloff(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 111: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1866: [propget, id(111), helpstring("property Collidable")] HRESULT Collidable([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_Collidable(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1867: [propput, id(111), helpstring("property Collidable")] HRESULT Collidable([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_Collidable(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 34: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1868: [propget, id(34), helpstring("property HasHitEvent")] HRESULT HasHitEvent([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_HasHitEvent(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1869: [propput, id(34), helpstring("property HasHitEvent")] HRESULT HasHitEvent([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_HasHitEvent(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 33: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1870: [propget, id(33), helpstring("property Threshold")] HRESULT Threshold([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Threshold(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1871: [propput, id(33), helpstring("property Threshold")] HRESULT Threshold([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Threshold(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 114: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1872: [propget, id(114), helpstring("property Friction")] HRESULT Friction([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Friction(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1873: [propput, id(114), helpstring("property Friction")] HRESULT Friction([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Friction(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 115: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1874: [propget, id(115), helpstring("property Scatter")] HRESULT Scatter([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Scatter(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1875: [propput, id(115), helpstring("property Scatter")] HRESULT Scatter([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Scatter(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 483: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1876: [propget, id(483), helpstring("property DisableLighting")] HRESULT DisableLighting([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_DisableLighting(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1877: [propput, id(483), helpstring("property DisableLighting")] HRESULT DisableLighting([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_DisableLighting(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 494: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1878: [propget, id(494), helpstring("property BlendDisableLighting")] HRESULT BlendDisableLighting([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_BlendDisableLighting(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1879: [propput, id(494), helpstring("property BlendDisableLighting")] HRESULT BlendDisableLighting([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_BlendDisableLighting(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 496: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1880: [propget, id(496), helpstring("property BlendDisableLightingFromBelow")] HRESULT BlendDisableLightingFromBelow([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_BlendDisableLightingFromBelow(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1881: [propput, id(496), helpstring("property BlendDisableLightingFromBelow")] HRESULT BlendDisableLightingFromBelow([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_BlendDisableLightingFromBelow(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 431: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1882: [propget, id(431), helpstring("property ReflectionEnabled")] HRESULT ReflectionEnabled([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_ReflectionEnabled(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1883: [propput, id(431), helpstring("property ReflectionEnabled")] HRESULT ReflectionEnabled([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_ReflectionEnabled(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 397: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1885: [propget, id(397), helpstring("property DepthBias")] HRESULT DepthBias([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_DepthBias(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1886: [propput, id(397), helpstring("property DepthBias")] HRESULT DepthBias([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_DepthBias(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 377: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1888: [propget, id(377), helpstring("property DropSpeed")] HRESULT DropSpeed([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_DropSpeed(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1889: [propput, id(377), helpstring("property DropSpeed")] HRESULT DropSpeed([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_DropSpeed(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 27: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1890: [propget, id(27), helpstring("property CurrentAnimOffset")] HRESULT CurrentAnimOffset([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_CurrentAnimOffset(&V_R4(&res));
			}
			break;
		}
		case 435: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1891: [propget, id(435), helpstring("property IsDropped")] HRESULT IsDropped([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_IsDropped(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1892: [propput, id(435), helpstring("property IsDropped")] HRESULT IsDropped([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_IsDropped(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 9: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1893: [propget, id(9), helpstring("property DrawStyle")] HRESULT DrawStyle([out, retval] TargetType *pVal);
				V_VT(&res) = VT_I4;
				hres = get_DrawStyle((TargetType*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1894: [propput, id(9), helpstring("property DrawStyle")] HRESULT DrawStyle([in] TargetType newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_DrawStyle((TargetType)V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 433: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1895: [propget, id(433), helpstring("property LegacyMode")] HRESULT LegacyMode([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_LegacyMode(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1896: [propput, id(433), helpstring("property LegacyMode")] HRESULT LegacyMode([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_LegacyMode(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 726: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1897: [propget, id(726), helpstring("property RaiseDelay")] HRESULT RaiseDelay([out, retval] long *pVal);
				V_VT(&res) = VT_I4;
				hres = get_RaiseDelay((long*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1898: [propput, id(726), helpstring("property RaiseDelay")] HRESULT RaiseDelay([in] long newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_RaiseDelay(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 734: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1899: [propget, id(734), helpstring("property PhysicsMaterial")] HRESULT PhysicsMaterial([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_PhysicsMaterial(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1900: [propput, id(734), helpstring("property PhysicsMaterial")] HRESULT PhysicsMaterial([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_PhysicsMaterial(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 432: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1901: [propget, id(432), helpstring("property OverwritePhysics")] HRESULT OverwritePhysics([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_OverwritePhysics(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1902: [propput, id(432), helpstring("property OverwritePhysics")] HRESULT OverwritePhysics([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_OverwritePhysics(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 727: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1903: [propget, id(727), helpstring("property HitThreshold")] HRESULT HitThreshold([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_HitThreshold(&V_R4(&res));
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

STDMETHODIMP HitTarget::GetDocumentation(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile) {
	if (index == MEMBERID_NIL) {
		*pBstrName = SysAllocString(L"HitTarget");
		return S_OK;
	}
	return E_NOTIMPL;
}

HRESULT HitTarget::FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) {
	static struct {
		DISPID dispId;
		const WCHAR *name;
	} idsNamesList[] = {
			{ NULL },
			{ DISPID_GameEvents_Init, L"_Init" },
			{ DISPID_TimerEvents_Timer, L"_Timer" },
			{ DISPID_TargetEvents_Dropped, L"_Dropped" },
			{ DISPID_TargetEvents_Raised, L"_Raised" },
			{ DISPID_HitEvents_Hit, L"_Hit" },
			{ DISPID_AnimateEvents_Animate, L"_Animate" }
	};

	static WCHAR wzName[MAXSTRING];
	size_t min = 1, max = ARRAY_SIZE(idsNamesList) - 1, i;
	int r;
	while(min <= max) {
		i = (min + max) / 2;
		if (idsNamesList[i].dispId == dispid) {
			wcscpy(wzName, m_wzName);
			wcscat(wzName, idsNamesList[i].name);
			LPOLESTR fnNames = (LPOLESTR)wzName;
			DISPID tDispid;
			CComPtr<IDispatch> disp;
			g_pplayer->m_ptable->m_pcv->m_pScript->GetScriptDispatch(nullptr, &disp);
			if (SUCCEEDED(disp->GetIDsOfNames(IID_NULL, &fnNames, 1, 0, &tDispid))) {
				return disp->Invoke(tDispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, pdispparams, nullptr, nullptr, nullptr);
			}
			return DISP_E_MEMBERNOTFOUND;
		}
		else if (idsNamesList[i].dispId < dispid)
		   min = i+1;
		else
		   max = i-1;
	}
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP Gate::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} namesIdsList[] = {
			{ NULL },
			{ L"CloseAngle", 2144 },
			{ L"Collidable", 111 },
			{ L"CurrentAngle", 16 },
			{ L"Damping", 13 },
			{ L"DrawStyle", 9 },
			{ L"Elasticity", 110 },
			{ L"Friction", 114 },
			{ L"GravityFactor", 17 },
			{ L"Height", 1142 },
			{ L"Length", DISPID_Gate_Length },
			{ L"Material", 340 },
			{ L"Move", 2147 },
			{ L"Name", DISPID_Name },
			{ L"Open", 7 },
			{ L"OpenAngle", 2145 },
			{ L"ReflectionEnabled", 431 },
			{ L"Rotation", DISPID_Gate_Rotation },
			{ L"ShowBracket", 15 },
			{ L"Surface", DISPID_Surface },
			{ L"TimerEnabled", DISPID_Timer_Enabled },
			{ L"TimerInterval", DISPID_Timer_Interval },
			{ L"TwoWay", 427 },
			{ L"UserValue", DISPID_UserValue },
			{ L"Visible", 458 },
			{ L"X", 5 },
			{ L"Y", 6 }
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

STDMETHODIMP Gate::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
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
		case DISPID_Name: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1934: [propget, id(DISPID_Name), helpstring("property Name")] HRESULT Name([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Name(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1935: [propput, id(DISPID_Name), helpstring("property Name")] HRESULT Name([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Name(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Timer_Enabled: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1936: [propget, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_TimerEnabled(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1937: [propput, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_TimerEnabled(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Timer_Interval: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1938: [propget, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([out, retval] long *pVal);
				V_VT(&res) = VT_I4;
				hres = get_TimerInterval((long*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1939: [propput, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([in] long newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_TimerInterval(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Gate_Length: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1940: [propget, id(DISPID_Gate_Length), helpstring("property Length")] HRESULT Length([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Length(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1941: [propput, id(DISPID_Gate_Length), helpstring("property Length")] HRESULT Length([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Length(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 1142: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1942: [propget, id(1142), helpstring("property Height")] HRESULT Height([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Height(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1943: [propput, id(1142), helpstring("property Height")] HRESULT Height([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Height(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Gate_Rotation: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1944: [propget, id(DISPID_Gate_Rotation), helpstring("property Rotation")] HRESULT Rotation([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Rotation(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1945: [propput, id(DISPID_Gate_Rotation), helpstring("property Rotation")] HRESULT Rotation([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Rotation(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 5: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1946: [propget, id(5), helpstring("property X")] HRESULT X([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_X(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1947: [propput, id(5), helpstring("property X")] HRESULT X([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_X(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 6: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1948: [propget, id(6), helpstring("property Y")] HRESULT Y([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Y(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1949: [propput, id(6), helpstring("property Y")] HRESULT Y([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Y(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 7: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1950: [propget, id(7), helpstring("property Open")] HRESULT Open([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_Open(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1951: [propput, id(7), helpstring("property Open")] HRESULT Open([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_Open(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 13: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1952: [propget, id(13), helpstring("property Damping")] HRESULT Damping([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Damping(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1953: [propput, id(13), helpstring("property Damping")] HRESULT Damping([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Damping(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 17: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1954: [propget, id(17), helpstring("property GravityFactor")] HRESULT GravityFactor([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_GravityFactor(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1955: [propput, id(17), helpstring("property GravityFactor")] HRESULT GravityFactor([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_GravityFactor(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 2147: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1957: [id(2147), helpstring("method Move")] HRESULT Move(int dir, [defaultvalue(0)] float speed, [defaultvalue(0)] float angle);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var1;
				V_VT(&var1) = VT_R4;
				V_R4(&var1) = 0;
				VariantChangeType(&var1, (index > 0) ? &pDispParams->rgvarg[--index] : &var1, 0, VT_R4);
				VARIANT var2;
				V_VT(&var2) = VT_R4;
				V_R4(&var2) = 0;
				VariantChangeType(&var2, (index > 0) ? &pDispParams->rgvarg[--index] : &var2, 0, VT_R4);
				hres = Move(V_I4(&var0), V_R4(&var1), V_R4(&var2));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
			}
			break;
		}
		case 340: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1959: [propget, id(340), helpstring("property Material")] HRESULT Material([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Material(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1960: [propput, id(340), helpstring("property Material")] HRESULT Material([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Material(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 110: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1961: [propget, id(110), helpstring("property Elasticity")] HRESULT Elasticity([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Elasticity(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1962: [propput, id(110), helpstring("property Elasticity")] HRESULT Elasticity([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Elasticity(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Surface: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1963: [propget, id(DISPID_Surface), helpstring("property Surface")] HRESULT Surface([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Surface(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1964: [propput, id(DISPID_Surface), helpstring("property Surface")] HRESULT Surface([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Surface(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_UserValue: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1965: [propget, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([out, retval] VARIANT* pVal);
				hres = get_UserValue(&res);
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1966: [propput, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([in] VARIANT* newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = put_UserValue(&var0);
				VariantClear(&var0);
			}
			break;
		}
		case 2144: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1968: [propget, id(2144), helpstring("property CloseAngle")] HRESULT CloseAngle([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_CloseAngle(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1969: [propput, id(2144), helpstring("property CloseAngle")] HRESULT CloseAngle([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_CloseAngle(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 2145: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1970: [propget, id(2145), helpstring("property OpenAngle")] HRESULT OpenAngle([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_OpenAngle(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1971: [propput, id(2145), helpstring("property OpenAngle")] HRESULT OpenAngle([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_OpenAngle(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 111: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1972: [propget, id(111), helpstring("property Collidable")] HRESULT Collidable([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_Collidable(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1973: [propput, id(111), helpstring("property Collidable")] HRESULT Collidable([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_Collidable(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 114: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1974: [propget, id(114), helpstring("property Friction")] HRESULT Friction([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Friction(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1975: [propput, id(114), helpstring("property Friction")] HRESULT Friction([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Friction(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 458: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1976: [propget, id(458), helpstring("property Visible")] HRESULT Visible([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_Visible(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1977: [propput, id(458), helpstring("property Visible")] HRESULT Visible([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_Visible(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 427: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1978: [propget, id(427), helpstring("property TwoWay")] HRESULT TwoWay([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_TwoWay(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1979: [propput, id(427), helpstring("property TwoWay")] HRESULT TwoWay([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_TwoWay(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 15: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1980: [propget, id(15), helpstring("property ShowBracket")] HRESULT ShowBracket([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_ShowBracket(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1981: [propput, id(15), helpstring("property ShowBracket")] HRESULT ShowBracket([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_ShowBracket(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 16: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1982: [propget, id(16), nonbrowsable, helpstring("property CurrentAngle")] HRESULT CurrentAngle([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_CurrentAngle(&V_R4(&res));
			}
			break;
		}
		case 431: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1983: [propget, id(431), helpstring("property ReflectionEnabled")] HRESULT ReflectionEnabled([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_ReflectionEnabled(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1984: [propput, id(431), helpstring("property ReflectionEnabled")] HRESULT ReflectionEnabled([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_ReflectionEnabled(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 9: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1985: [propget, id(9), helpstring("property DrawStyle")] HRESULT DrawStyle([out, retval] GateType *pVal);
				V_VT(&res) = VT_I4;
				hres = get_DrawStyle((GateType*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1986: [propput, id(9), helpstring("property DrawStyle")] HRESULT DrawStyle([in] GateType newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_DrawStyle((GateType)V_I4(&var0));
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

STDMETHODIMP Gate::GetDocumentation(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile) {
	if (index == MEMBERID_NIL) {
		*pBstrName = SysAllocString(L"Gate");
		return S_OK;
	}
	return E_NOTIMPL;
}

HRESULT Gate::FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) {
	static struct {
		DISPID dispId;
		const WCHAR *name;
	} idsNamesList[] = {
			{ NULL },
			{ DISPID_GameEvents_Init, L"_Init" },
			{ DISPID_TimerEvents_Timer, L"_Timer" },
			{ DISPID_HitEvents_Hit, L"_Hit" },
			{ DISPID_LimitEvents_EOS, L"_LimitEOS" },
			{ DISPID_LimitEvents_BOS, L"_LimitBOS" },
			{ DISPID_AnimateEvents_Animate, L"_Animate" }
	};

	static WCHAR wzName[MAXSTRING];
	size_t min = 1, max = ARRAY_SIZE(idsNamesList) - 1, i;
	int r;
	while(min <= max) {
		i = (min + max) / 2;
		if (idsNamesList[i].dispId == dispid) {
			wcscpy(wzName, m_wzName);
			wcscat(wzName, idsNamesList[i].name);
			LPOLESTR fnNames = (LPOLESTR)wzName;
			DISPID tDispid;
			CComPtr<IDispatch> disp;
			g_pplayer->m_ptable->m_pcv->m_pScript->GetScriptDispatch(nullptr, &disp);
			if (SUCCEEDED(disp->GetIDsOfNames(IID_NULL, &fnNames, 1, 0, &tDispid))) {
				return disp->Invoke(tDispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, pdispparams, nullptr, nullptr, nullptr);
			}
			return DISP_E_MEMBERNOTFOUND;
		}
		else if (idsNamesList[i].dispId < dispid)
		   min = i+1;
		else
		   max = i-1;
	}
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP Spinner::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} namesIdsList[] = {
			{ NULL },
			{ L"AngleMax", 13 },
			{ L"AngleMin", 14 },
			{ L"CurrentAngle", 18 },
			{ L"Damping", 7 },
			{ L"Elasticity", 110 },
			{ L"Height", 5 },
			{ L"Image", DISPID_Image },
			{ L"Length", DISPID_Spinner_Length },
			{ L"Material", 340 },
			{ L"Name", DISPID_Name },
			{ L"ReflectionEnabled", 431 },
			{ L"Rotation", 4 },
			{ L"ShowBracket", 108 },
			{ L"Surface", DISPID_Surface },
			{ L"TimerEnabled", DISPID_Timer_Enabled },
			{ L"TimerInterval", DISPID_Timer_Interval },
			{ L"UserValue", DISPID_UserValue },
			{ L"Visible", 458 },
			{ L"X", 11 },
			{ L"Y", 12 }
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

STDMETHODIMP Spinner::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
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
		case DISPID_Name: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2017: [propget, id(DISPID_Name), helpstring("property Name")] HRESULT Name([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Name(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2018: [propput, id(DISPID_Name), helpstring("property Name")] HRESULT Name([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Name(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Timer_Enabled: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2019: [propget, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_TimerEnabled(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2020: [propput, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_TimerEnabled(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Timer_Interval: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2021: [propget, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([out, retval] long *pVal);
				V_VT(&res) = VT_I4;
				hres = get_TimerInterval((long*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2022: [propput, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([in] long newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_TimerInterval(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Spinner_Length: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2023: [propget, id(DISPID_Spinner_Length), helpstring("property Length")] HRESULT Length([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Length(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2024: [propput, id(DISPID_Spinner_Length), helpstring("property Length")] HRESULT Length([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Length(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 4: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2025: [propget, id(4), helpstring("property Rotation")] HRESULT Rotation([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Rotation(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2026: [propput, id(4), helpstring("property Rotation")] HRESULT Rotation([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Rotation(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 5: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2027: [propget, id(5), helpstring("property Height")] HRESULT Height([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Height(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2028: [propput, id(5), helpstring("property Height")] HRESULT Height([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Height(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 7: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2029: [propget, id(7), helpstring("property Damping")] HRESULT Damping([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Damping(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2030: [propput, id(7), helpstring("property Damping")] HRESULT Damping([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Damping(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Image: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2031: [propget, id(DISPID_Image), helpstring("property Image")] HRESULT Image([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Image(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2032: [propput, id(DISPID_Image), helpstring("property Image")] HRESULT Image([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Image(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 340: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2033: [propget, id(340), helpstring("property Material")] HRESULT Material([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Material(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2034: [propput, id(340), helpstring("property Material")] HRESULT Material([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Material(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 11: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2035: [propget, id(11), helpstring("property X")] HRESULT X([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_X(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2036: [propput, id(11), helpstring("property X")] HRESULT X([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_X(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 12: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2037: [propget, id(12), helpstring("property Y")] HRESULT Y([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Y(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2038: [propput, id(12), helpstring("property Y")] HRESULT Y([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Y(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Surface: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2039: [propget, id(DISPID_Surface), helpstring("property Surface")] HRESULT Surface([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Surface(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2040: [propput, id(DISPID_Surface), helpstring("property Surface")] HRESULT Surface([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Surface(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_UserValue: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2041: [propget, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([out, retval] VARIANT* pVal);
				hres = get_UserValue(&res);
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2042: [propput, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([in] VARIANT* newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = put_UserValue(&var0);
				VariantClear(&var0);
			}
			break;
		}
		case 108: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2043: [propget, id(108), helpstring("property ShowBracket")] HRESULT ShowBracket([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_ShowBracket(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2044: [propput, id(108), helpstring("property ShowBracket")] HRESULT ShowBracket([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_ShowBracket(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 13: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2046: [propget, id(13), helpstring("property AngleMax")] HRESULT AngleMax([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_AngleMax(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2047: [propput, id(13), helpstring("property AngleMax")] HRESULT AngleMax([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_AngleMax(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 14: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2048: [propget, id(14), helpstring("property AngleMin")] HRESULT AngleMin([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_AngleMin(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2049: [propput, id(14), helpstring("property AngleMin")] HRESULT AngleMin([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_AngleMin(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 110: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2050: [propget, id(110), helpstring("property Elasticity")] HRESULT Elasticity([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Elasticity(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2051: [propput, id(110), helpstring("property Elasticity")] HRESULT Elasticity([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Elasticity(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 458: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2052: [propget, id(458), helpstring("property Visible")] HRESULT Visible([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_Visible(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2053: [propput, id(458), helpstring("property Visible")] HRESULT Visible([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_Visible(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 431: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2054: [propget, id(431), helpstring("property ReflectionEnabled")] HRESULT ReflectionEnabled([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_ReflectionEnabled(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2055: [propput, id(431), helpstring("property ReflectionEnabled")] HRESULT ReflectionEnabled([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_ReflectionEnabled(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 18: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2056: [propget, id(18), nonbrowsable, helpstring("property CurrentAngle")] HRESULT CurrentAngle([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_CurrentAngle(&V_R4(&res));
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

STDMETHODIMP Spinner::GetDocumentation(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile) {
	if (index == MEMBERID_NIL) {
		*pBstrName = SysAllocString(L"Spinner");
		return S_OK;
	}
	return E_NOTIMPL;
}

HRESULT Spinner::FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) {
	static struct {
		DISPID dispId;
		const WCHAR *name;
	} idsNamesList[] = {
			{ NULL },
			{ DISPID_GameEvents_Init, L"_Init" },
			{ DISPID_TimerEvents_Timer, L"_Timer" },
			{ DISPID_SpinnerEvents_Spin, L"_Spin" },
			{ DISPID_LimitEvents_EOS, L"_LimitEOS" },
			{ DISPID_LimitEvents_BOS, L"_LimitBOS" },
			{ DISPID_AnimateEvents_Animate, L"_Animate" }
	};

	static WCHAR wzName[MAXSTRING];
	size_t min = 1, max = ARRAY_SIZE(idsNamesList) - 1, i;
	int r;
	while(min <= max) {
		i = (min + max) / 2;
		if (idsNamesList[i].dispId == dispid) {
			wcscpy(wzName, m_wzName);
			wcscat(wzName, idsNamesList[i].name);
			LPOLESTR fnNames = (LPOLESTR)wzName;
			DISPID tDispid;
			CComPtr<IDispatch> disp;
			g_pplayer->m_ptable->m_pcv->m_pScript->GetScriptDispatch(nullptr, &disp);
			if (SUCCEEDED(disp->GetIDsOfNames(IID_NULL, &fnNames, 1, 0, &tDispid))) {
				return disp->Invoke(tDispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, pdispparams, nullptr, nullptr, nullptr);
			}
			return DISP_E_MEMBERNOTFOUND;
		}
		else if (idsNamesList[i].dispId < dispid)
		   min = i+1;
		else
		   max = i-1;
	}
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP Ramp::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} namesIdsList[] = {
			{ NULL },
			{ L"Collidable", 111 },
			{ L"DepthBias", 397 },
			{ L"Elasticity", 110 },
			{ L"Friction", 114 },
			{ L"HasHitEvent", 34 },
			{ L"HasWallImage", 9 },
			{ L"HeightBottom", 1 },
			{ L"HeightTop", 2 },
			{ L"Image", DISPID_Image },
			{ L"ImageAlignment", 8 },
			{ L"LeftWallHeight", 10 },
			{ L"Material", 340 },
			{ L"Name", DISPID_Name },
			{ L"OverwritePhysics", 432 },
			{ L"PhysicsMaterial", 734 },
			{ L"ReflectionEnabled", 431 },
			{ L"RightWallHeight", 11 },
			{ L"Scatter", 115 },
			{ L"Threshold", 33 },
			{ L"TimerEnabled", DISPID_Timer_Enabled },
			{ L"TimerInterval", DISPID_Timer_Interval },
			{ L"Type", 6 },
			{ L"UserValue", DISPID_UserValue },
			{ L"Visible", 458 },
			{ L"VisibleLeftWallHeight", 108 },
			{ L"VisibleRightWallHeight", 109 },
			{ L"WidthBottom", 3 },
			{ L"WidthTop", 4 },
			{ L"WireDiameter", 377 },
			{ L"WireDistanceX", 398 },
			{ L"WireDistanceY", 425 }
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

STDMETHODIMP Ramp::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
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
		case DISPID_Name: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2087: [propget, id(DISPID_Name), helpstring("property Name")] HRESULT Name([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Name(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2088: [propput, id(DISPID_Name), helpstring("property Name")] HRESULT Name([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Name(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Timer_Enabled: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2089: [propget, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_TimerEnabled(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2090: [propput, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_TimerEnabled(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Timer_Interval: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2091: [propget, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([out, retval] long *pVal);
				V_VT(&res) = VT_I4;
				hres = get_TimerInterval((long*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2092: [propput, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([in] long newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_TimerInterval(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 1: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2093: [propget, id(1), helpstring("property HeightBottom")] HRESULT HeightBottom([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_HeightBottom(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2094: [propput, id(1), helpstring("property HeightBottom")] HRESULT HeightBottom([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_HeightBottom(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 2: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2095: [propget, id(2), helpstring("property HeightTop")] HRESULT HeightTop([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_HeightTop(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2096: [propput, id(2), helpstring("property HeightTop")] HRESULT HeightTop([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_HeightTop(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 3: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2097: [propget, id(3), helpstring("property WidthBottom")] HRESULT WidthBottom([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_WidthBottom(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2098: [propput, id(3), helpstring("property WidthBottom")] HRESULT WidthBottom([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_WidthBottom(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 4: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2099: [propget, id(4), helpstring("property WidthTop")] HRESULT WidthTop([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_WidthTop(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2100: [propput, id(4), helpstring("property WidthTop")] HRESULT WidthTop([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_WidthTop(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 340: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2101: [propget, id(340), helpstring("property Material")] HRESULT Material([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Material(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2102: [propput, id(340), helpstring("property Material")] HRESULT Material([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Material(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 6: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2103: [propget, id(6), helpstring("property Type")] HRESULT Type([out, retval] RampType *pVal);
				V_VT(&res) = VT_I4;
				hres = get_Type((RampType*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2104: [propput, id(6), helpstring("property Type")] HRESULT Type([in] RampType newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_Type((RampType)V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Image: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2105: [propget, id(DISPID_Image), helpstring("property Image")] HRESULT Image([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Image(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2106: [propput, id(DISPID_Image), helpstring("property Image")] HRESULT Image([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Image(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 8: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2107: [propget, id(8), helpstring("property ImageAlignment")] HRESULT ImageAlignment([out, retval] RampImageAlignment *pVal);
				V_VT(&res) = VT_I4;
				hres = get_ImageAlignment((RampImageAlignment*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2108: [propput, id(8), helpstring("property ImageAlignment")] HRESULT ImageAlignment([in] RampImageAlignment newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_ImageAlignment((RampImageAlignment)V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 9: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2109: [propget, id(9), helpstring("property HasWallImage")] HRESULT HasWallImage([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_HasWallImage(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2110: [propput, id(9), helpstring("property HasWallImage")] HRESULT HasWallImage([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_HasWallImage(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 10: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2111: [propget, id(10), helpstring("property LeftWallHeight")] HRESULT LeftWallHeight([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_LeftWallHeight(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2112: [propput, id(10), helpstring("property LeftWallHeight")] HRESULT LeftWallHeight([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_LeftWallHeight(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 11: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2113: [propget, id(11), helpstring("property RightWallHeight")] HRESULT RightWallHeight([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_RightWallHeight(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2114: [propput, id(11), helpstring("property RightWallHeight")] HRESULT RightWallHeight([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_RightWallHeight(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_UserValue: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2115: [propget, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([out, retval] VARIANT* pVal);
				hres = get_UserValue(&res);
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2116: [propput, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([in] VARIANT* newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = put_UserValue(&var0);
				VariantClear(&var0);
			}
			break;
		}
		case 108: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2117: [propget, id(108), helpstring("property VisibleLeftWallHeight")] HRESULT VisibleLeftWallHeight([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_VisibleLeftWallHeight(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2118: [propput, id(108), helpstring("property VisibleLeftWallHeight")] HRESULT VisibleLeftWallHeight([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_VisibleLeftWallHeight(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 109: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2119: [propget, id(109), helpstring("property VisibleRightWallHeight")] HRESULT VisibleRightWallHeight([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_VisibleRightWallHeight(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2120: [propput, id(109), helpstring("property VisibleRightWallHeight")] HRESULT VisibleRightWallHeight([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_VisibleRightWallHeight(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 110: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2121: [propget, id(110), helpstring("property Elasticity")] HRESULT Elasticity([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Elasticity(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2122: [propput, id(110), helpstring("property Elasticity")] HRESULT Elasticity([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Elasticity(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 111: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2124: [propget, id(111), helpstring("property Collidable")] HRESULT Collidable([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_Collidable(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2125: [propput, id(111), helpstring("property Collidable")] HRESULT Collidable([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_Collidable(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 34: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2126: [propget, id(34), helpstring("property HasHitEvent")] HRESULT HasHitEvent([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_HasHitEvent(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2127: [propput, id(34), helpstring("property HasHitEvent")] HRESULT HasHitEvent([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_HasHitEvent(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 33: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2128: [propget, id(33), helpstring("property Threshold")] HRESULT Threshold([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Threshold(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2129: [propput, id(33), helpstring("property Threshold")] HRESULT Threshold([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Threshold(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 458: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2131: [propget, id(458), helpstring("property Visible")] HRESULT Visible([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_Visible(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2132: [propput, id(458), helpstring("property Visible")] HRESULT Visible([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_Visible(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 114: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2134: [propget, id(114), helpstring("property Friction")] HRESULT Friction([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Friction(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2135: [propput, id(114), helpstring("property Friction")] HRESULT Friction([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Friction(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 115: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2136: [propget, id(115), helpstring("property Scatter")] HRESULT Scatter([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Scatter(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2137: [propput, id(115), helpstring("property Scatter")] HRESULT Scatter([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Scatter(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 397: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2139: [propget, id(397), helpstring("property DepthBias")] HRESULT DepthBias([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_DepthBias(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2140: [propput, id(397), helpstring("property DepthBias")] HRESULT DepthBias([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_DepthBias(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 377: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2142: [propget, id(377), helpstring("property WireDiameter")] HRESULT WireDiameter([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_WireDiameter(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2143: [propput, id(377), helpstring("property WireDiameter")] HRESULT WireDiameter([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_WireDiameter(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 398: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2145: [propget, id(398), helpstring("property WireDistanceX")] HRESULT WireDistanceX([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_WireDistanceX(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2146: [propput, id(398), helpstring("property WireDistanceX")] HRESULT WireDistanceX([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_WireDistanceX(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 425: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2148: [propget, id(425), helpstring("property WireDistanceY")] HRESULT WireDistanceY([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_WireDistanceY(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2149: [propput, id(425), helpstring("property WireDistanceY")] HRESULT WireDistanceY([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_WireDistanceY(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 431: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2151: [propget, id(431), helpstring("property ReflectionEnabled")] HRESULT ReflectionEnabled([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_ReflectionEnabled(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2152: [propput, id(431), helpstring("property ReflectionEnabled")] HRESULT ReflectionEnabled([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_ReflectionEnabled(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 734: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2154: [propget, id(734), helpstring("property PhysicsMaterial")] HRESULT PhysicsMaterial([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_PhysicsMaterial(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2155: [propput, id(734), helpstring("property PhysicsMaterial")] HRESULT PhysicsMaterial([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_PhysicsMaterial(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 432: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2156: [propget, id(432), helpstring("property OverwritePhysics")] HRESULT OverwritePhysics([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_OverwritePhysics(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2157: [propput, id(432), helpstring("property OverwritePhysics")] HRESULT OverwritePhysics([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_OverwritePhysics(V_BOOL(&var0));
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

STDMETHODIMP Ramp::GetDocumentation(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile) {
	if (index == MEMBERID_NIL) {
		*pBstrName = SysAllocString(L"Ramp");
		return S_OK;
	}
	return E_NOTIMPL;
}

STDMETHODIMP Flasher::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} namesIdsList[] = {
			{ NULL },
			{ L"AddBlend", 556 },
			{ L"Amount", 379 },
			{ L"Color", 11 },
			{ L"DepthBias", 397 },
			{ L"DisplayTexture", 13 },
			{ L"DMD", 557 },
			{ L"DMDColoredPixels", 47 },
			{ L"DMDHeight", 45 },
			{ L"DMDPixels", 46 },
			{ L"DMDWidth", 44 },
			{ L"Filter", 32996 },
			{ L"Height", 378 },
			{ L"ImageA", DISPID_Image },
			{ L"ImageAlignment", 8 },
			{ L"ImageB", DISPID_Image2 },
			{ L"IntensityScale", 435 },
			{ L"ModulateVsAdd", 433 },
			{ L"Name", DISPID_Name },
			{ L"Opacity", 377 },
			{ L"RotX", 9 },
			{ L"RotY", 2 },
			{ L"RotZ", 1 },
			{ L"TimerEnabled", DISPID_Timer_Enabled },
			{ L"TimerInterval", DISPID_Timer_Interval },
			{ L"UserValue", DISPID_UserValue },
			{ L"VideoCapHeight", 561 },
			{ L"VideoCapUpdate", 562 },
			{ L"VideoCapWidth", 560 },
			{ L"Visible", 458 },
			{ L"X", 5 },
			{ L"Y", 6 }
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

STDMETHODIMP Flasher::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
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
		case DISPID_Name: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2173: [propget, id(DISPID_Name), helpstring("property Name")] HRESULT Name([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Name(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2174: [propput, id(DISPID_Name), helpstring("property Name")] HRESULT Name([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Name(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 5: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2175: [propget, id(5), helpstring("property X")] HRESULT X([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_X(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2176: [propput, id(5), helpstring("property X")] HRESULT X([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_X(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 6: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2177: [propget, id(6), helpstring("property Y")] HRESULT Y([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Y(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2178: [propput, id(6), helpstring("property Y")] HRESULT Y([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Y(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 8: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2179: [propget, id(8), helpstring("property ImageAlignment")] HRESULT ImageAlignment([out, retval] RampImageAlignment *pVal);
				V_VT(&res) = VT_I4;
				hres = get_ImageAlignment((RampImageAlignment*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2180: [propput, id(8), helpstring("property ImageAlignment")] HRESULT ImageAlignment([in] RampImageAlignment newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_ImageAlignment((RampImageAlignment)V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 378: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2181: [propget, id(378), helpstring("property Height")] HRESULT Height([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Height(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2182: [propput, id(378), helpstring("property Height")] HRESULT Height([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Height(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 1: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2183: [propget, id(1), helpstring("property RotZ")] HRESULT RotZ([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_RotZ(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2184: [propput, id(1), helpstring("property RotZ")] HRESULT RotZ([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_RotZ(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 2: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2185: [propget, id(2), helpstring("property RotY")] HRESULT RotY([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_RotY(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2186: [propput, id(2), helpstring("property RotY")] HRESULT RotY([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_RotY(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 9: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2187: [propget, id(9), helpstring("property RotX")] HRESULT RotX([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_RotX(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2188: [propput, id(9), helpstring("property RotX")] HRESULT RotX([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_RotX(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 11: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2189: [propget, id(11), helpstring("property Color")] HRESULT Color([out, retval] OLE_COLOR *pVal);
				V_VT(&res) = VT_UI4;
				hres = get_Color(&V_UI4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2190: [propput, id(11), helpstring("property Color")] HRESULT Color([in] OLE_COLOR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_UI4);
				hres = put_Color((OLE_COLOR)V_UI4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Image: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2191: [propget, id(DISPID_Image), helpstring("property ImageA")] HRESULT ImageA([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_ImageA(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2192: [propput, id(DISPID_Image), helpstring("property ImageA")] HRESULT ImageA([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_ImageA(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Image2: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2193: [propget, id(DISPID_Image2), helpstring("property ImageB")] HRESULT ImageB([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_ImageB(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2194: [propput, id(DISPID_Image2), helpstring("property ImageB")] HRESULT ImageB([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_ImageB(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 13: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2195: [propget, id(13), helpstring("property DisplayTexture")] HRESULT DisplayTexture([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_DisplayTexture(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2196: [propput, id(13), helpstring("property DisplayTexture")] HRESULT DisplayTexture([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_DisplayTexture(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 377: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2197: [propget, id(377), helpstring("property Opacity")] HRESULT Opacity([out, retval] long *pVal);
				V_VT(&res) = VT_I4;
				hres = get_Opacity((long*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2198: [propput, id(377), helpstring("property Opacity")] HRESULT Opacity([in] long newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_Opacity(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 435: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2199: [propget, id(435), helpstring("property IntensityScale")] HRESULT IntensityScale([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_IntensityScale(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2200: [propput, id(435), helpstring("property IntensityScale")] HRESULT IntensityScale([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_IntensityScale(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 433: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2201: [propget, id(433), helpstring("property ModulateVsAdd")] HRESULT ModulateVsAdd([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_ModulateVsAdd(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2202: [propput, id(433), helpstring("property ModulateVsAdd")] HRESULT ModulateVsAdd([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_ModulateVsAdd(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_UserValue: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2203: [propget, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([out, retval] VARIANT* pVal);
				hres = get_UserValue(&res);
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2204: [propput, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([in] VARIANT* newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = put_UserValue(&var0);
				VariantClear(&var0);
			}
			break;
		}
		case 458: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2206: [propget, id(458), helpstring("property Visible")] HRESULT Visible([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_Visible(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2207: [propput, id(458), helpstring("property Visible")] HRESULT Visible([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_Visible(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 556: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2208: [propget, id(556), helpstring("property AddBlend")] HRESULT AddBlend([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_AddBlend(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2209: [propput, id(556), helpstring("property AddBlend")] HRESULT AddBlend([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_AddBlend(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 557: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2211: [propget, id(557), helpstring("property DMD")] HRESULT DMD([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_DMD(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2212: [propput, id(557), helpstring("property DMD")] HRESULT DMD([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_DMD(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 44: {
			if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2213: [propput, id(44), helpstring("property DMDWidth")] HRESULT DMDWidth([in] int pVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_DMDWidth(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 45: {
			if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2214: [propput, id(45), helpstring("property DMDHeight")] HRESULT DMDHeight([in] int pVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_DMDHeight(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 46: {
			if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2215: [propput, id(46), helpstring("property DMDPixels")] HRESULT DMDPixels([in] VARIANT pVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = put_DMDPixels(var0);
				VariantClear(&var0);
			}
			break;
		}
		case 47: {
			if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2216: [propput, id(47), helpstring("property DMDColoredPixels")] HRESULT DMDColoredPixels([in] VARIANT pVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = put_DMDColoredPixels(var0);
				VariantClear(&var0);
			}
			break;
		}
		case 560: {
			if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2218: [propput, id(560), helpstring("property VideoCapWidth")] HRESULT VideoCapWidth([in] long cWidth);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_VideoCapWidth(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 561: {
			if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2219: [propput, id(561), helpstring("property VideoCapHeight")] HRESULT VideoCapHeight([in] long cHeight);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_VideoCapHeight(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 562: {
			if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2220: [propput, id(562), helpstring("property VideoCapUpdate")] HRESULT VideoCapUpdate([in] BSTR cWinTitle);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_VideoCapUpdate(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 397: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2222: [propget, id(397), helpstring("property DepthBias")] HRESULT DepthBias([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_DepthBias(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2223: [propput, id(397), helpstring("property DepthBias")] HRESULT DepthBias([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_DepthBias(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 32996: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2224: [propget, id(32996), helpstring("property Filter")] HRESULT Filter([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Filter(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2225: [propput, id(32996), helpstring("property Filter")] HRESULT Filter([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Filter(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 379: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2226: [propget, id(379), helpstring("property Amount")] HRESULT Amount([out, retval] long *pVal);
				V_VT(&res) = VT_I4;
				hres = get_Amount((long*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2227: [propput, id(379), helpstring("property Amount")] HRESULT Amount([in] long newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_Amount(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Timer_Enabled: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2228: [propget, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_TimerEnabled(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2229: [propput, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_TimerEnabled(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Timer_Interval: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2230: [propget, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([out, retval] long *pVal);
				V_VT(&res) = VT_I4;
				hres = get_TimerInterval((long*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2231: [propput, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([in] long newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_TimerInterval(V_I4(&var0));
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

STDMETHODIMP Flasher::GetDocumentation(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile) {
	if (index == MEMBERID_NIL) {
		*pBstrName = SysAllocString(L"Flasher");
		return S_OK;
	}
	return E_NOTIMPL;
}

STDMETHODIMP Rubber::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} namesIdsList[] = {
			{ NULL },
			{ L"Collidable", 111 },
			{ L"Elasticity", 110 },
			{ L"ElasticityFalloff", 120 },
			{ L"EnableShowInEditor", 479 },
			{ L"EnableStaticRendering", 398 },
			{ L"Friction", 114 },
			{ L"HasHitEvent", 34 },
			{ L"Height", 2 },
			{ L"HitHeight", 116 },
			{ L"Image", DISPID_Image },
			{ L"Material", 340 },
			{ L"Name", DISPID_Name },
			{ L"OverwritePhysics", 432 },
			{ L"PhysicsMaterial", 734 },
			{ L"ReflectionEnabled", 431 },
			{ L"RotX", 18 },
			{ L"RotY", 24 },
			{ L"RotZ", 25 },
			{ L"Scatter", 115 },
			{ L"Thickness", 4 },
			{ L"TimerEnabled", DISPID_Timer_Enabled },
			{ L"TimerInterval", DISPID_Timer_Interval },
			{ L"UserValue", DISPID_UserValue },
			{ L"Visible", 458 }
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

STDMETHODIMP Rubber::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
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
		case DISPID_Name: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2247: [propget, id(DISPID_Name), helpstring("property Name")] HRESULT Name([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Name(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2248: [propput, id(DISPID_Name), helpstring("property Name")] HRESULT Name([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Name(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 2: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2249: [propget, id(2), helpstring("property Height")] HRESULT Height([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Height(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2250: [propput, id(2), helpstring("property Height")] HRESULT Height([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Height(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 116: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2251: [propget, id(116), helpstring("property HitHeight")] HRESULT HitHeight([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_HitHeight(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2252: [propput, id(116), helpstring("property HitHeight")] HRESULT HitHeight([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_HitHeight(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 34: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2253: [propget, id(34), helpstring("property HitEvent")] HRESULT HasHitEvent([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_HasHitEvent(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2254: [propput, id(34), helpstring("property HitEvent")] HRESULT HasHitEvent([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_HasHitEvent(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 4: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2255: [propget, id(4), helpstring("property Thickness")] HRESULT Thickness([out, retval] int *pVal);
				V_VT(&res) = VT_I4;
				hres = get_Thickness((int*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2256: [propput, id(4), helpstring("property Thickness")] HRESULT Thickness([in] int newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_Thickness(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 18: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2257: [propget, id(18), helpstring("property RotX")] HRESULT RotX([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_RotX(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2258: [propput, id(18), helpstring("property RotX")] HRESULT RotX([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_RotX(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 25: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2259: [propget, id(25), helpstring("property RotZ")] HRESULT RotZ([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_RotZ(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2260: [propput, id(25), helpstring("property RotZ")] HRESULT RotZ([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_RotZ(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 24: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2261: [propget, id(24), helpstring("property RotY")] HRESULT RotY([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_RotY(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2262: [propput, id(24), helpstring("property RotY")] HRESULT RotY([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_RotY(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 340: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2263: [propget, id(340), helpstring("property Material")] HRESULT Material([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Material(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2264: [propput, id(340), helpstring("property Material")] HRESULT Material([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Material(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Image: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2265: [propget, id(DISPID_Image), helpstring("property Image")] HRESULT Image([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Image(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2266: [propput, id(DISPID_Image), helpstring("property Image")] HRESULT Image([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Image(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 110: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2267: [propget, id(110), helpstring("property Elasticity")] HRESULT Elasticity([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Elasticity(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2268: [propput, id(110), helpstring("property Elasticity")] HRESULT Elasticity([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Elasticity(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 120: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2269: [propget, id(120), helpstring("property ElasticityFalloff")] HRESULT ElasticityFalloff([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_ElasticityFalloff(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2270: [propput, id(120), helpstring("property ElasticityFalloff")] HRESULT ElasticityFalloff([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_ElasticityFalloff(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 111: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2272: [propget, id(111), helpstring("property Collidable")] HRESULT Collidable([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_Collidable(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2273: [propput, id(111), helpstring("property Collidable")] HRESULT Collidable([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_Collidable(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 458: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2275: [propget, id(458), helpstring("property Visible")] HRESULT Visible([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_Visible(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2276: [propput, id(458), helpstring("property Visible")] HRESULT Visible([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_Visible(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 114: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2278: [propget, id(114), helpstring("property Friction")] HRESULT Friction([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Friction(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2279: [propput, id(114), helpstring("property Friction")] HRESULT Friction([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Friction(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 115: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2280: [propget, id(115), helpstring("property Scatter")] HRESULT Scatter([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Scatter(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2281: [propput, id(115), helpstring("property Scatter")] HRESULT Scatter([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Scatter(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 398: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2283: [propget, id(398), helpstring("property static rendering")] HRESULT EnableStaticRendering([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_EnableStaticRendering(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2284: [propput, id(398), helpstring("property static rendering")] HRESULT EnableStaticRendering([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_EnableStaticRendering(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 479: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2286: [propget, id(479), helpstring("property show in editor")] HRESULT EnableShowInEditor([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_EnableShowInEditor(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2287: [propput, id(479), helpstring("property show in editor")] HRESULT EnableShowInEditor([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_EnableShowInEditor(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 431: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2289: [propget, id(431), helpstring("property ReflectionEnabled")] HRESULT ReflectionEnabled([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_ReflectionEnabled(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2290: [propput, id(431), helpstring("property ReflectionEnabled")] HRESULT ReflectionEnabled([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_ReflectionEnabled(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Timer_Enabled: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2292: [propget, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_TimerEnabled(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2293: [propput, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_TimerEnabled(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Timer_Interval: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2294: [propget, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([out, retval] long *pVal);
				V_VT(&res) = VT_I4;
				hres = get_TimerInterval((long*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2295: [propput, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([in] long newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_TimerInterval(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_UserValue: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2296: [propget, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([out, retval] VARIANT* pVal);
				hres = get_UserValue(&res);
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2297: [propput, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([in] VARIANT* newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = put_UserValue(&var0);
				VariantClear(&var0);
			}
			break;
		}
		case 734: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2299: [propget, id(734), helpstring("property PhysicsMaterial")] HRESULT PhysicsMaterial([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_PhysicsMaterial(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2300: [propput, id(734), helpstring("property PhysicsMaterial")] HRESULT PhysicsMaterial([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_PhysicsMaterial(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 432: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2301: [propget, id(432), helpstring("property OverwritePhysics")] HRESULT OverwritePhysics([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_OverwritePhysics(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2302: [propput, id(432), helpstring("property OverwritePhysics")] HRESULT OverwritePhysics([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_OverwritePhysics(V_BOOL(&var0));
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

STDMETHODIMP Rubber::GetDocumentation(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile) {
	if (index == MEMBERID_NIL) {
		*pBstrName = SysAllocString(L"Rubber");
		return S_OK;
	}
	return E_NOTIMPL;
}

STDMETHODIMP BallEx::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} namesIdsList[] = {
			{ NULL },
			{ L"AngMomX", 17 },
			{ L"AngMomY", 18 },
			{ L"AngMomZ", 19 },
			{ L"AngVelX", 14 },
			{ L"AngVelY", 15 },
			{ L"AngVelZ", 16 },
			{ L"BulbIntensityScale", 451 },
			{ L"Color", 7 },
			{ L"DecalMode", 497 },
			{ L"DestroyBall", 100 },
			{ L"ForceReflection", 486 },
			{ L"FrontDecal", 9 },
			{ L"ID", 13 },
			{ L"Image", 8 },
			{ L"Mass", 11 },
			{ L"Name", DISPID_Name },
			{ L"PlayfieldReflectionScale", 485 },
			{ L"Radius", 12 },
			{ L"ReflectionEnabled", 484 },
			{ L"UserValue", DISPID_UserValue },
			{ L"VelX", 3 },
			{ L"VelY", 4 },
			{ L"VelZ", 6 },
			{ L"Visible", 487 },
			{ L"X", 1 },
			{ L"Y", 2 },
			{ L"Z", 5 }
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

STDMETHODIMP BallEx::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
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
				// line 2318: [propget, id(1), helpstring("property X")] HRESULT X([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_X(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2319: [propput, id(1), helpstring("property X")] HRESULT X([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_X(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 2: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2320: [propget, id(2), helpstring("property Y")] HRESULT Y([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Y(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2321: [propput, id(2), helpstring("property Y")] HRESULT Y([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Y(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 3: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2322: [propget, id(3), helpstring("property VelX")] HRESULT VelX([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_VelX(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2323: [propput, id(3), helpstring("property VelX")] HRESULT VelX([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_VelX(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 4: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2324: [propget, id(4), helpstring("property VelY")] HRESULT VelY([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_VelY(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2325: [propput, id(4), helpstring("property VelY")] HRESULT VelY([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_VelY(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 5: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2326: [propget, id(5), helpstring("property Z")] HRESULT Z([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Z(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2327: [propput, id(5), helpstring("property Z")] HRESULT Z([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Z(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 6: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2328: [propget, id(6), helpstring("property VelZ")] HRESULT VelZ([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_VelZ(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2329: [propput, id(6), helpstring("property VelZ")] HRESULT VelZ([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_VelZ(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 14: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2330: [propget, id(14), helpstring("property AngVelX")] HRESULT AngVelX([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_AngVelX(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2331: [propput, id(14), helpstring("property AngVelX")] HRESULT AngVelX([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_AngVelX(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 15: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2332: [propget, id(15), helpstring("property AngVelY")] HRESULT AngVelY([out, retval] float* pVal);
				V_VT(&res) = VT_R4;
				hres = get_AngVelY(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2333: [propput, id(15), helpstring("property AngVelY")] HRESULT AngVelY([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_AngVelY(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 16: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2334: [propget, id(16), helpstring("property AngVelZ")] HRESULT AngVelZ([out, retval] float* pVal);
				V_VT(&res) = VT_R4;
				hres = get_AngVelZ(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2335: [propput, id(16), helpstring("property AngVelZ")] HRESULT AngVelZ([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_AngVelZ(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 17: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2336: [propget, id(17), helpstring("property AngMomX")] HRESULT AngMomX([out, retval] float* pVal);
				V_VT(&res) = VT_R4;
				hres = get_AngMomX(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2337: [propput, id(17), helpstring("property AngMomX")] HRESULT AngMomX([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_AngMomX(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 18: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2338: [propget, id(18), helpstring("property AngMomY")] HRESULT AngMomY([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_AngMomY(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2339: [propput, id(18), helpstring("property AngMomY")] HRESULT AngMomY([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_AngMomY(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 19: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2340: [propget, id(19), helpstring("property AngMomZ")] HRESULT AngMomZ([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_AngMomZ(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2341: [propput, id(19), helpstring("property AngMomZ")] HRESULT AngMomZ([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_AngMomZ(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 7: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2343: [propget, id(7), helpstring("property Color")] HRESULT Color([out, retval] OLE_COLOR *pVal);
				V_VT(&res) = VT_UI4;
				hres = get_Color(&V_UI4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2344: [propput, id(7), helpstring("property Color")] HRESULT Color([in] OLE_COLOR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_UI4);
				hres = put_Color((OLE_COLOR)V_UI4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 8: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2345: [propget, id(8), helpstring("property Image")] HRESULT Image([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Image(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2346: [propput, id(8), helpstring("property Image")] HRESULT Image([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Image(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 9: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2347: [propget, id(9), helpstring("property FrontDecal")] HRESULT FrontDecal([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_FrontDecal(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2348: [propput, id(9), helpstring("property FrontDecal")] HRESULT FrontDecal([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_FrontDecal(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 497: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2349: [propget, id(497), helpstring("property DecalMode")] HRESULT DecalMode([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_DecalMode(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2350: [propput, id(497), helpstring("property DecalMode")] HRESULT DecalMode([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_DecalMode(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_UserValue: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2351: [propget, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([out, retval] VARIANT* pVal);
				hres = get_UserValue(&res);
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2352: [propput, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([in] VARIANT* newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = put_UserValue(&var0);
				VariantClear(&var0);
			}
			break;
		}
		case 11: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2353: [propget, id(11), helpstring("property Mass")] HRESULT Mass([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Mass(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2354: [propput, id(11), helpstring("property Mass")] HRESULT Mass([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Mass(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 12: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2355: [propget, id(12), helpstring("property Radius")] HRESULT Radius([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Radius(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2356: [propput, id(12), helpstring("property Radius")] HRESULT Radius([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Radius(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 13: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2357: [propget, id(13), helpstring("property ID")] HRESULT ID([out, retval] int *pVal);
				V_VT(&res) = VT_I4;
				hres = get_ID((int*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2358: [propput, id(13), helpstring("property ID")] HRESULT ID([in] int newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_ID(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Name: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2359: [propget, id(DISPID_Name), helpstring("property Name")] HRESULT Name([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Name(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2360: [propput, id(DISPID_Name), helpstring("property Name")] HRESULT Name([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Name(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 451: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2361: [propget, id(451), helpstring("property BulbIntensityScale")] HRESULT BulbIntensityScale([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_BulbIntensityScale(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2362: [propput, id(451), helpstring("property BulbIntensityScale")] HRESULT BulbIntensityScale([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_BulbIntensityScale(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 484: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2363: [propget, id(484), helpstring("property ReflectionEnabled")] HRESULT ReflectionEnabled([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_ReflectionEnabled(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2364: [propput, id(484), helpstring("property ReflectionEnabled")] HRESULT ReflectionEnabled([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_ReflectionEnabled(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 485: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2365: [propget, id(485), helpstring("property PlayfieldReflectionScale")] HRESULT PlayfieldReflectionScale([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_PlayfieldReflectionScale(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2366: [propput, id(485), helpstring("property PlayfieldReflectionScale")] HRESULT PlayfieldReflectionScale([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_PlayfieldReflectionScale(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 486: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2367: [propget, id(486), helpstring("property ForceReflection")] HRESULT ForceReflection([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_ForceReflection(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2368: [propput, id(486), helpstring("property ForceReflection")] HRESULT ForceReflection([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_ForceReflection(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 487: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2369: [propget, id(487), helpstring("property Visible")] HRESULT Visible([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_Visible(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2370: [propput, id(487), helpstring("property Visible")] HRESULT Visible([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_Visible(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 100: {
			if (wFlags & DISPATCH_METHOD) {
				// line 2371: [id(100), helpstring("method DestroyBall")] HRESULT DestroyBall([out, retval] int *pVal);
				V_VT(&res) = VT_I4;
				hres = DestroyBall((int*)&V_I4(&res));
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

STDMETHODIMP BallEx::GetDocumentation(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile) {
	if (index == MEMBERID_NIL) {
		*pBstrName = SysAllocString(L"IBall");
		return S_OK;
	}
	return E_NOTIMPL;
}

HRESULT Ramp::FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) {
	static struct {
		DISPID dispId;
		const WCHAR *name;
	} idsNamesList[] = {
			{ NULL },
			{ DISPID_GameEvents_Init, L"_Init" }
	};

	static WCHAR wzName[MAXSTRING];
	size_t min = 1, max = ARRAY_SIZE(idsNamesList) - 1, i;
	int r;
	while(min <= max) {
		i = (min + max) / 2;
		if (idsNamesList[i].dispId == dispid) {
			wcscpy(wzName, m_wzName);
			wcscat(wzName, idsNamesList[i].name);
			LPOLESTR fnNames = (LPOLESTR)wzName;
			DISPID tDispid;
			CComPtr<IDispatch> disp;
			g_pplayer->m_ptable->m_pcv->m_pScript->GetScriptDispatch(nullptr, &disp);
			if (SUCCEEDED(disp->GetIDsOfNames(IID_NULL, &fnNames, 1, 0, &tDispid))) {
				return disp->Invoke(tDispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, pdispparams, nullptr, nullptr, nullptr);
			}
			return DISP_E_MEMBERNOTFOUND;
		}
		else if (idsNamesList[i].dispId < dispid)
		   min = i+1;
		else
		   max = i-1;
	}
	return DISP_E_MEMBERNOTFOUND;
}

HRESULT Flasher::FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) {
	static struct {
		DISPID dispId;
		const WCHAR *name;
	} idsNamesList[] = {
			{ NULL },
			{ DISPID_GameEvents_Init, L"_Init" },
			{ DISPID_TimerEvents_Timer, L"_Timer" }
	};

	static WCHAR wzName[MAXSTRING];
	size_t min = 1, max = ARRAY_SIZE(idsNamesList) - 1, i;
	int r;
	while(min <= max) {
		i = (min + max) / 2;
		if (idsNamesList[i].dispId == dispid) {
			wcscpy(wzName, m_wzName);
			wcscat(wzName, idsNamesList[i].name);
			LPOLESTR fnNames = (LPOLESTR)wzName;
			DISPID tDispid;
			CComPtr<IDispatch> disp;
			g_pplayer->m_ptable->m_pcv->m_pScript->GetScriptDispatch(nullptr, &disp);
			if (SUCCEEDED(disp->GetIDsOfNames(IID_NULL, &fnNames, 1, 0, &tDispid))) {
				return disp->Invoke(tDispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, pdispparams, nullptr, nullptr, nullptr);
			}
			return DISP_E_MEMBERNOTFOUND;
		}
		else if (idsNamesList[i].dispId < dispid)
		   min = i+1;
		else
		   max = i-1;
	}
	return DISP_E_MEMBERNOTFOUND;
}

HRESULT Rubber::FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) {
	static struct {
		DISPID dispId;
		const WCHAR *name;
	} idsNamesList[] = {
			{ NULL },
			{ DISPID_GameEvents_Init, L"_Init" },
			{ DISPID_TimerEvents_Timer, L"_Timer" },
			{ DISPID_HitEvents_Hit, L"_Hit" }
	};

	static WCHAR wzName[MAXSTRING];
	size_t min = 1, max = ARRAY_SIZE(idsNamesList) - 1, i;
	int r;
	while(min <= max) {
		i = (min + max) / 2;
		if (idsNamesList[i].dispId == dispid) {
			wcscpy(wzName, m_wzName);
			wcscat(wzName, idsNamesList[i].name);
			LPOLESTR fnNames = (LPOLESTR)wzName;
			DISPID tDispid;
			CComPtr<IDispatch> disp;
			g_pplayer->m_ptable->m_pcv->m_pScript->GetScriptDispatch(nullptr, &disp);
			if (SUCCEEDED(disp->GetIDsOfNames(IID_NULL, &fnNames, 1, 0, &tDispid))) {
				return disp->Invoke(tDispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, pdispparams, nullptr, nullptr, nullptr);
			}
			return DISP_E_MEMBERNOTFOUND;
		}
		else if (idsNamesList[i].dispId < dispid)
		   min = i+1;
		else
		   max = i-1;
	}
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP DispReel::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} namesIdsList[] = {
			{ NULL },
			{ L"AddValue", 30 },
			{ L"BackColor", DISPID_DispReel_BackColor },
			{ L"Height", 6 },
			{ L"Image", DISPID_Image },
			{ L"ImagesPerGridRow", 18 },
			{ L"IsTransparent", 12 },
			{ L"Name", DISPID_Name },
			{ L"Range", 14 },
			{ L"Reels", 11 },
			{ L"ResetToZero", 31 },
			{ L"SetValue", 33 },
			{ L"Sound", DISPID_Sound },
			{ L"Spacing", 7 },
			{ L"SpinReel", 32 },
			{ L"Steps", 8 },
			{ L"TimerEnabled", DISPID_Timer_Enabled },
			{ L"TimerInterval", DISPID_Timer_Interval },
			{ L"UpdateInterval", 15 },
			{ L"UseImageGrid", 17 },
			{ L"UserValue", DISPID_UserValue },
			{ L"Visible", 458 },
			{ L"Width", 5 },
			{ L"X", 9 },
			{ L"Y", 10 }
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

STDMETHODIMP DispReel::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
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
		case DISPID_DispReel_BackColor: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2421: [propget, id(DISPID_DispReel_BackColor), helpstring("property BackColor")] HRESULT BackColor([out, retval] OLE_COLOR *pVal);
				V_VT(&res) = VT_UI4;
				hres = get_BackColor(&V_UI4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2422: [propput, id(DISPID_DispReel_BackColor), helpstring("property BackColor")] HRESULT BackColor([in] OLE_COLOR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_UI4);
				hres = put_BackColor((OLE_COLOR)V_UI4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Image: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2423: [propget, id(DISPID_Image), helpstring("property Image")] HRESULT Image([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Image(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2424: [propput, id(DISPID_Image), helpstring("property Image")] HRESULT Image([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Image(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 11: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2425: [propget, id(11), helpstring("property Reels")] HRESULT Reels([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Reels(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2426: [propput, id(11), helpstring("property Reels")] HRESULT Reels([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Reels(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 5: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2427: [propget, id(5), helpstring("property Width")] HRESULT Width([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Width(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2428: [propput, id(5), helpstring("property Width")] HRESULT Width([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Width(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 6: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2429: [propget, id(6), helpstring("property Height")] HRESULT Height([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Height(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2430: [propput, id(6), helpstring("property Height")] HRESULT Height([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Height(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 7: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2431: [propget, id(7), helpstring("property Spacing")] HRESULT Spacing([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Spacing(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2432: [propput, id(7), helpstring("property Spacing")] HRESULT Spacing([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Spacing(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 12: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2433: [propget, id(12), helpstring("property IsTransparent")] HRESULT IsTransparent([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_IsTransparent(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2434: [propput, id(12), helpstring("property IsTransparent")] HRESULT IsTransparent([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_IsTransparent(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Sound: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2435: [propget, id(DISPID_Sound), helpstring("property Sound")] HRESULT Sound([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Sound(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2436: [propput, id(DISPID_Sound), helpstring("property Sound")] HRESULT Sound([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Sound(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 8: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2437: [propget, id(8), helpstring("property Steps")] HRESULT Steps([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Steps(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2438: [propput, id(8), helpstring("property Steps")] HRESULT Steps([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Steps(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Timer_Enabled: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2439: [propget, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_TimerEnabled(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2440: [propput, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_TimerEnabled(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Timer_Interval: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2441: [propget, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([out, retval] long *pVal);
				V_VT(&res) = VT_I4;
				hres = get_TimerInterval((long*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2442: [propput, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([in] long newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_TimerInterval(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 9: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2443: [propget, id(9), helpstring("property X")] HRESULT X([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_X(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2444: [propput, id(9), helpstring("property X")] HRESULT X([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_X(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 10: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2445: [propget, id(10), helpstring("property Y")] HRESULT Y([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Y(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2446: [propput, id(10), helpstring("property Y")] HRESULT Y([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Y(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 14: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2447: [propget, id(14), helpstring("property Range")] HRESULT Range([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_Range(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2448: [propput, id(14), helpstring("property Range")] HRESULT Range([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Range(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Name: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2449: [propget, id(DISPID_Name), helpstring("property Name")] HRESULT Name([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Name(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2450: [propput, id(DISPID_Name), helpstring("property Name")] HRESULT Name([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Name(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 15: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2451: [propget, id(15), helpstring("property UpdateInterval")] HRESULT UpdateInterval([out, retval] long *pVal);
				V_VT(&res) = VT_I4;
				hres = get_UpdateInterval((long*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2452: [propput, id(15), helpstring("property UpdateInterval")] HRESULT UpdateInterval([in] long newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_UpdateInterval(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_UserValue: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2453: [propget, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([out, retval] VARIANT* pVal);
				hres = get_UserValue(&res);
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2454: [propput, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([in] VARIANT* newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = put_UserValue(&var0);
				VariantClear(&var0);
			}
			break;
		}
		case 17: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2455: [propget, id(17), helpstring("property UseImageGrid")] HRESULT UseImageGrid([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_UseImageGrid(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2456: [propput, id(17), helpstring("property UseImageGrid")] HRESULT UseImageGrid([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_UseImageGrid(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 458: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2457: [propget, id(458), helpstring("property Visible")] HRESULT Visible([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_Visible(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2458: [propput, id(458), helpstring("property Visible")] HRESULT Visible([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_Visible(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 18: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2459: [propget, id(18), helpstring("property ImagesPerGridRow")] HRESULT ImagesPerGridRow([out, retval] long *pVal);
				V_VT(&res) = VT_I4;
				hres = get_ImagesPerGridRow((long*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2460: [propput, id(18), helpstring("property ImagesPerGridRow")] HRESULT ImagesPerGridRow([in] long newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_ImagesPerGridRow(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 30: {
			if (wFlags & DISPATCH_METHOD) {
				// line 2462: [id(30), helpstring("method AddValue")] HRESULT AddValue([in] long Value);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = AddValue(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 31: {
			if (wFlags & DISPATCH_METHOD) {
				// line 2463: [id(31), helpstring("method ResetToZero")] HRESULT ResetToZero();
				hres = ResetToZero();
			}
			break;
		}
		case 32: {
			if (wFlags & DISPATCH_METHOD) {
				// line 2464: [id(32), helpstring("method SpinReel")] HRESULT SpinReel([in] long ReelNumber, [in] long PulseCount);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = SpinReel(V_I4(&var0), V_I4(&var1));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 33: {
			if (wFlags & DISPATCH_METHOD) {
				// line 2465: [id(33), helpstring("method SetValue")] HRESULT SetValue([in] long Value);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = SetValue(V_I4(&var0));
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

STDMETHODIMP DispReel::GetDocumentation(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile) {
	if (index == MEMBERID_NIL) {
		*pBstrName = SysAllocString(L"DispReel");
		return S_OK;
	}
	return E_NOTIMPL;
}

HRESULT DispReel::FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) {
	static struct {
		DISPID dispId;
		const WCHAR *name;
	} idsNamesList[] = {
			{ NULL },
			{ DISPID_GameEvents_Init, L"_Init" },
			{ DISPID_TimerEvents_Timer, L"_Timer" },
			{ DISPID_AnimateEvents_Animate, L"_Animate" }
	};

	static WCHAR wzName[MAXSTRING];
	size_t min = 1, max = ARRAY_SIZE(idsNamesList) - 1, i;
	int r;
	while(min <= max) {
		i = (min + max) / 2;
		if (idsNamesList[i].dispId == dispid) {
			wcscpy(wzName, m_wzName);
			wcscat(wzName, idsNamesList[i].name);
			LPOLESTR fnNames = (LPOLESTR)wzName;
			DISPID tDispid;
			CComPtr<IDispatch> disp;
			g_pplayer->m_ptable->m_pcv->m_pScript->GetScriptDispatch(nullptr, &disp);
			if (SUCCEEDED(disp->GetIDsOfNames(IID_NULL, &fnNames, 1, 0, &tDispid))) {
				return disp->Invoke(tDispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, pdispparams, nullptr, nullptr, nullptr);
			}
			return DISP_E_MEMBERNOTFOUND;
		}
		else if (idsNamesList[i].dispId < dispid)
		   min = i+1;
		else
		   max = i-1;
	}
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP LightSeq::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} namesIdsList[] = {
			{ NULL },
			{ L"CenterX", 9 },
			{ L"CenterY", 10 },
			{ L"Collection", DISPID_Collection },
			{ L"Name", DISPID_Name },
			{ L"Play", 32 },
			{ L"StopPlay", 33 },
			{ L"TimerEnabled", DISPID_Timer_Enabled },
			{ L"TimerInterval", DISPID_Timer_Interval },
			{ L"UpdateInterval", 15 },
			{ L"UserValue", DISPID_UserValue }
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

STDMETHODIMP LightSeq::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
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
		case DISPID_Name: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2489: [propget, id(DISPID_Name), helpstring("property Name")] HRESULT Name([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Name(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2490: [propput, id(DISPID_Name), helpstring("property Name")] HRESULT Name([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Name(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Collection: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2491: [propget, id(DISPID_Collection), helpstring("property Collection")] HRESULT Collection([out, retval] BSTR *pVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Collection(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2492: [propput, id(DISPID_Collection), helpstring("property Collection")] HRESULT Collection([in] BSTR newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Collection(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 9: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2493: [propget, id(9), helpstring("property CenterX")] HRESULT CenterX([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_CenterX(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2494: [propput, id(9), helpstring("property CenterX")] HRESULT CenterX([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_CenterX(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 10: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2495: [propget, id(10), helpstring("property CenterY")] HRESULT CenterY([out, retval] float *pVal);
				V_VT(&res) = VT_R4;
				hres = get_CenterY(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2496: [propput, id(10), helpstring("property CenterY")] HRESULT CenterY([in] float newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_CenterY(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 15: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2497: [propget, id(15), helpstring("property UpdateInterval")] HRESULT UpdateInterval([out, retval] long *pVal);
				V_VT(&res) = VT_I4;
				hres = get_UpdateInterval((long*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2498: [propput, id(15), helpstring("property UpdateInterval")] HRESULT UpdateInterval([in] long newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_UpdateInterval(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Timer_Enabled: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2499: [propget, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([out, retval] VARIANT_BOOL *pVal);
				V_VT(&res) = VT_BOOL;
				hres = get_TimerEnabled(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2500: [propput, id(DISPID_Timer_Enabled), helpstring("property TimerEnabled")] HRESULT TimerEnabled([in] VARIANT_BOOL newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_TimerEnabled(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_Timer_Interval: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2501: [propget, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([out, retval] long *pVal);
				V_VT(&res) = VT_I4;
				hres = get_TimerInterval((long*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2502: [propput, id(DISPID_Timer_Interval), helpstring("property TimerInterval")] HRESULT TimerInterval([in] long newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_TimerInterval(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_UserValue: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 2503: [propget, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([out, retval] VARIANT* pVal);
				hres = get_UserValue(&res);
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 2504: [propput, id(DISPID_UserValue), helpstring("property User")] HRESULT UserValue([in] VARIANT* newVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = put_UserValue(&var0);
				VariantClear(&var0);
			}
			break;
		}
		case 32: {
			if (wFlags & DISPATCH_METHOD) {
				// line 2505: [id(32), helpstring("method Play")] HRESULT Play([in] SequencerState Animation, [defaultvalue(0)] long TailLength, [defaultvalue(1)] long Repeat, [defaultvalue(0)] long Pause);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var1;
				V_VT(&var1) = VT_I4;
				V_I4(&var1) = 0;
				VariantChangeType(&var1, (index > 0) ? &pDispParams->rgvarg[--index] : &var1, 0, VT_I4);
				VARIANT var2;
				V_VT(&var2) = VT_I4;
				V_I4(&var2) = 1;
				VariantChangeType(&var2, (index > 0) ? &pDispParams->rgvarg[--index] : &var2, 0, VT_I4);
				VARIANT var3;
				V_VT(&var3) = VT_I4;
				V_I4(&var3) = 0;
				VariantChangeType(&var3, (index > 0) ? &pDispParams->rgvarg[--index] : &var3, 0, VT_I4);
				hres = Play((SequencerState)V_I4(&var0), V_I4(&var1), V_I4(&var2), V_I4(&var3));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
				VariantClear(&var3);
			}
			break;
		}
		case 33: {
			if (wFlags & DISPATCH_METHOD) {
				// line 2506: [id(33), helpstring("method StopPlay")] HRESULT StopPlay();
				hres = StopPlay();
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

STDMETHODIMP LightSeq::GetDocumentation(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile) {
	if (index == MEMBERID_NIL) {
		*pBstrName = SysAllocString(L"LightSeq");
		return S_OK;
	}
	return E_NOTIMPL;
}

HRESULT LightSeq::FireDispID(const DISPID dispid, DISPPARAMS * const pdispparams) {
	static struct {
		DISPID dispId;
		const WCHAR *name;
	} idsNamesList[] = {
			{ NULL },
			{ DISPID_GameEvents_Init, L"_Init" },
			{ DISPID_TimerEvents_Timer, L"_Timer" },
			{ DISPID_LightSeqEvents_PlayDone, L"_PlayDone" }
	};

	static WCHAR wzName[MAXSTRING];
	size_t min = 1, max = ARRAY_SIZE(idsNamesList) - 1, i;
	int r;
	while(min <= max) {
		i = (min + max) / 2;
		if (idsNamesList[i].dispId == dispid) {
			wcscpy(wzName, m_wzName);
			wcscat(wzName, idsNamesList[i].name);
			LPOLESTR fnNames = (LPOLESTR)wzName;
			DISPID tDispid;
			CComPtr<IDispatch> disp;
			g_pplayer->m_ptable->m_pcv->m_pScript->GetScriptDispatch(nullptr, &disp);
			if (SUCCEEDED(disp->GetIDsOfNames(IID_NULL, &fnNames, 1, 0, &tDispid))) {
				return disp->Invoke(tDispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, pdispparams, nullptr, nullptr, nullptr);
			}
			return DISP_E_MEMBERNOTFOUND;
		}
		else if (idsNamesList[i].dispId < dispid)
		   min = i+1;
		else
		   max = i-1;
	}
	return DISP_E_MEMBERNOTFOUND;
}

