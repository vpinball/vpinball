#include "stdafx.h"
#include "olectl.h"

#include "UltraDMD.h"

STDMETHODIMP UltraDMD::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} namesIdsList[] = {
			{ NULL },
			{ L"CancelRendering", 0x6002000a },
			{ L"CancelRenderingWithId", 0x6002000b },
			{ L"Clear", 0x6002000c },
			{ L"CreateAnimationFromImages", 0x60020010 },
			{ L"DisplayScene00", 0x60020017 },
			{ L"DisplayScene00Ex", 0x60020018 },
			{ L"DisplayScene00ExWithId", 0x60020019 },
			{ L"DisplayScene01", 0x6002001c },
			{ L"DisplayScoreboard", 0x60020015 },
			{ L"DisplayScoreboard00", 0x60020016 },
			{ L"DisplayText", 0x6002001d },
			{ L"DisplayVersionInfo", 0x60020014 },
			{ L"FloatProperty", 0x60020000 },
			{ L"GetBuildNumber", 0x60020006 },
			{ L"GetMajorVersion", 0x60020004 },
			{ L"GetMinorVersion", 0x60020005 },
			{ L"Init", 0x60020002 },
			{ L"IsRendering", 0x60020009 },
			{ L"ModifyScene00", 0x6002001a },
			{ L"ModifyScene00Ex", 0x6002001b },
			{ L"RegisterFont", 0x60020012 },
			{ L"RegisterVideo", 0x60020011 },
			{ L"ScrollingCredits", 0x6002001e },
			{ L"SetFlipY", 0x60020008 },
			{ L"SetProjectFolder", 0x6002000d },
			{ L"SetScoreboardBackgroundImage", 0x6002000f },
			{ L"SetVideoStretchMode", 0x6002000e },
			{ L"SetVisibleVirtualDMD", 0x60020007 },
			{ L"Uninit", 0x60020003 },
			{ L"UnregisterFont", 0x60020013 }
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

STDMETHODIMP UltraDMD::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
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
		case 0x60020000: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 38: [id(0x60020000), propget]HRESULT FloatProperty([out, retval] single* pRetVal);
				V_VT(&res) = VT_R4;
				hres = get_FloatProperty(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 40: [id(0x60020000), propput]HRESULT FloatProperty([in] single pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_FloatProperty(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x60020002: {
			if (wFlags & DISPATCH_METHOD) {
				// line 42: [id(0x60020002)]HRESULT Init();
				hres = Init();
			}
			break;
		}
		case 0x60020003: {
			if (wFlags & DISPATCH_METHOD) {
				// line 44: [id(0x60020003)]HRESULT Uninit();
				hres = Uninit();
			}
			break;
		}
		case 0x60020004: {
			if (wFlags & DISPATCH_METHOD) {
				// line 46: [id(0x60020004)]HRESULT GetMajorVersion([out, retval] long* pRetVal);
				V_VT(&res) = VT_I4;
				hres = GetMajorVersion((LONG*)&V_I4(&res));
			}
			break;
		}
		case 0x60020005: {
			if (wFlags & DISPATCH_METHOD) {
				// line 48: [id(0x60020005)]HRESULT GetMinorVersion([out, retval] long* pRetVal);
				V_VT(&res) = VT_I4;
				hres = GetMinorVersion((LONG*)&V_I4(&res));
			}
			break;
		}
		case 0x60020006: {
			if (wFlags & DISPATCH_METHOD) {
				// line 50: [id(0x60020006)]HRESULT GetBuildNumber([out, retval] long* pRetVal);
				V_VT(&res) = VT_I4;
				hres = GetBuildNumber((LONG*)&V_I4(&res));
			}
			break;
		}
		case 0x60020007: {
			if (wFlags & DISPATCH_METHOD) {
				// line 52: [id(0x60020007)]HRESULT SetVisibleVirtualDMD([in] VARIANT_BOOL bHide,[out, retval] VARIANT_BOOL* pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				V_VT(&res) = VT_BOOL;
				hres = SetVisibleVirtualDMD(V_BOOL(&var0), &V_BOOL(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 0x60020008: {
			if (wFlags & DISPATCH_METHOD) {
				// line 56: [id(0x60020008)]HRESULT SetFlipY([in] VARIANT_BOOL flipY,[out, retval] VARIANT_BOOL* pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				V_VT(&res) = VT_BOOL;
				hres = SetFlipY(V_BOOL(&var0), &V_BOOL(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 0x60020009: {
			if (wFlags & DISPATCH_METHOD) {
				// line 60: [id(0x60020009)]HRESULT IsRendering([out, retval] VARIANT_BOOL* pRetVal);
				V_VT(&res) = VT_BOOL;
				hres = IsRendering(&V_BOOL(&res));
			}
			break;
		}
		case 0x6002000a: {
			if (wFlags & DISPATCH_METHOD) {
				// line 62: [id(0x6002000a)]HRESULT CancelRendering();
				hres = CancelRendering();
			}
			break;
		}
		case 0x6002000b: {
			if (wFlags & DISPATCH_METHOD) {
				// line 64: [id(0x6002000b)]HRESULT CancelRenderingWithId([in] BSTR sceneId);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = CancelRenderingWithId(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x6002000c: {
			if (wFlags & DISPATCH_METHOD) {
				// line 66: [id(0x6002000c)]HRESULT Clear();
				hres = Clear();
			}
			break;
		}
		case 0x6002000d: {
			if (wFlags & DISPATCH_METHOD) {
				// line 68: [id(0x6002000d)]HRESULT SetProjectFolder([in] BSTR basePath);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = SetProjectFolder(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x6002000e: {
			if (wFlags & DISPATCH_METHOD) {
				// line 70: [id(0x6002000e)]HRESULT SetVideoStretchMode([in] long mode);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = SetVideoStretchMode(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x6002000f: {
			if (wFlags & DISPATCH_METHOD) {
				// line 72: [id(0x6002000f)]HRESULT SetScoreboardBackgroundImage([in] BSTR filename,[in] long selectedBrightness,[in] long unselectedBrightness);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var2;
				V_VT(&var2) = VT_EMPTY;
				VariantChangeType(&var2, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = SetScoreboardBackgroundImage(V_BSTR(&var0), V_I4(&var1), V_I4(&var2));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
			}
			break;
		}
		case 0x60020010: {
			if (wFlags & DISPATCH_METHOD) {
				// line 77: [id(0x60020010)]HRESULT CreateAnimationFromImages([in] long fps,[in] VARIANT_BOOL loop,[in] BSTR imagelist,[out, retval] long* pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				VARIANT var2;
				V_VT(&var2) = VT_EMPTY;
				VariantChangeType(&var2, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				V_VT(&res) = VT_I4;
				hres = CreateAnimationFromImages(V_I4(&var0), V_BOOL(&var1), V_BSTR(&var2), (LONG*)&V_I4(&res));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
			}
			break;
		}
		case 0x60020011: {
			if (wFlags & DISPATCH_METHOD) {
				// line 83: [id(0x60020011)]HRESULT RegisterVideo([in] long videoStretchMode,[in] VARIANT_BOOL loop,[in] BSTR videoFilename,[out, retval] long* pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				VARIANT var2;
				V_VT(&var2) = VT_EMPTY;
				VariantChangeType(&var2, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				V_VT(&res) = VT_I4;
				hres = RegisterVideo(V_I4(&var0), V_BOOL(&var1), V_BSTR(&var2), (LONG*)&V_I4(&res));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
			}
			break;
		}
		case 0x60020012: {
			if (wFlags & DISPATCH_METHOD) {
				// line 89: [id(0x60020012)]HRESULT RegisterFont([in] BSTR file,[out, retval] long* pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				V_VT(&res) = VT_I4;
				hres = RegisterFont(V_BSTR(&var0), (LONG*)&V_I4(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 0x60020013: {
			if (wFlags & DISPATCH_METHOD) {
				// line 93: [id(0x60020013)]HRESULT UnregisterFont([in] long registeredFont);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = UnregisterFont(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x60020014: {
			if (wFlags & DISPATCH_METHOD) {
				// line 95: [id(0x60020014)]HRESULT DisplayVersionInfo();
				hres = DisplayVersionInfo();
			}
			break;
		}
		case 0x60020015: {
			if (wFlags & DISPATCH_METHOD) {
				// line 97: [id(0x60020015)]HRESULT DisplayScoreboard([in] long cPlayers,[in] long highlightedPlayer,[in] long score1,[in] long score2,[in] long score3,[in] long score4,[in] BSTR lowerLeft,[in] BSTR lowerRight);
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
				VARIANT var6;
				V_VT(&var6) = VT_EMPTY;
				VariantChangeType(&var6, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var7;
				V_VT(&var7) = VT_EMPTY;
				VariantChangeType(&var7, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = DisplayScoreboard(V_I4(&var0), V_I4(&var1), V_I4(&var2), V_I4(&var3), V_I4(&var4), V_I4(&var5), V_BSTR(&var6), V_BSTR(&var7));
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
		case 0x60020016: {
			if (wFlags & DISPATCH_METHOD) {
				// line 107: [id(0x60020016)]HRESULT DisplayScoreboard00([in] long cPlayers,[in] long highlightedPlayer,[in] long score1,[in] long score2,[in] long score3,[in] long score4,[in] BSTR lowerLeft,[in] BSTR lowerRight);
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
				VARIANT var6;
				V_VT(&var6) = VT_EMPTY;
				VariantChangeType(&var6, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var7;
				V_VT(&var7) = VT_EMPTY;
				VariantChangeType(&var7, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = DisplayScoreboard00(V_I4(&var0), V_I4(&var1), V_I4(&var2), V_I4(&var3), V_I4(&var4), V_I4(&var5), V_BSTR(&var6), V_BSTR(&var7));
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
		case 0x60020017: {
			if (wFlags & DISPATCH_METHOD) {
				// line 117: [id(0x60020017)]HRESULT DisplayScene00([in] BSTR background,[in] BSTR toptext,[in] long topBrightness,[in] BSTR bottomtext,[in] long bottomBrightness,[in] long animateIn,[in] long pauseTime,[in] long animateOut);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var2;
				V_VT(&var2) = VT_EMPTY;
				VariantChangeType(&var2, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var3;
				V_VT(&var3) = VT_EMPTY;
				VariantChangeType(&var3, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var4;
				V_VT(&var4) = VT_EMPTY;
				VariantChangeType(&var4, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var5;
				V_VT(&var5) = VT_EMPTY;
				VariantChangeType(&var5, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var6;
				V_VT(&var6) = VT_EMPTY;
				VariantChangeType(&var6, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var7;
				V_VT(&var7) = VT_EMPTY;
				VariantChangeType(&var7, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = DisplayScene00(V_BSTR(&var0), V_BSTR(&var1), V_I4(&var2), V_BSTR(&var3), V_I4(&var4), V_I4(&var5), V_I4(&var6), V_I4(&var7));
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
		case 0x60020018: {
			if (wFlags & DISPATCH_METHOD) {
				// line 127: [id(0x60020018)]HRESULT DisplayScene00Ex([in] BSTR background,[in] BSTR toptext,[in] long topBrightness,[in] long topOutlineBrightness,[in] BSTR bottomtext,[in] long bottomBrightness,[in] long bottomOutlineBrightness,[in] long animateIn,[in] long pauseTime,[in] long animateOut);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var2;
				V_VT(&var2) = VT_EMPTY;
				VariantChangeType(&var2, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var3;
				V_VT(&var3) = VT_EMPTY;
				VariantChangeType(&var3, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var4;
				V_VT(&var4) = VT_EMPTY;
				VariantChangeType(&var4, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var5;
				V_VT(&var5) = VT_EMPTY;
				VariantChangeType(&var5, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var6;
				V_VT(&var6) = VT_EMPTY;
				VariantChangeType(&var6, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var7;
				V_VT(&var7) = VT_EMPTY;
				VariantChangeType(&var7, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var8;
				V_VT(&var8) = VT_EMPTY;
				VariantChangeType(&var8, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var9;
				V_VT(&var9) = VT_EMPTY;
				VariantChangeType(&var9, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = DisplayScene00Ex(V_BSTR(&var0), V_BSTR(&var1), V_I4(&var2), V_I4(&var3), V_BSTR(&var4), V_I4(&var5), V_I4(&var6), V_I4(&var7), V_I4(&var8), V_I4(&var9));
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
			}
			break;
		}
		case 0x60020019: {
			if (wFlags & DISPATCH_METHOD) {
				// line 139: [id(0x60020019)]HRESULT DisplayScene00ExWithId([in] BSTR sceneId,[in] VARIANT_BOOL cancelPrevious,[in] BSTR background,[in] BSTR toptext,[in] long topBrightness,[in] long topOutlineBrightness,[in] BSTR bottomtext,[in] long bottomBrightness,[in] long bottomOutlineBrightness,[in] long animateIn,[in] long pauseTime,[in] long animateOut);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				VARIANT var2;
				V_VT(&var2) = VT_EMPTY;
				VariantChangeType(&var2, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var3;
				V_VT(&var3) = VT_EMPTY;
				VariantChangeType(&var3, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var4;
				V_VT(&var4) = VT_EMPTY;
				VariantChangeType(&var4, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var5;
				V_VT(&var5) = VT_EMPTY;
				VariantChangeType(&var5, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var6;
				V_VT(&var6) = VT_EMPTY;
				VariantChangeType(&var6, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var7;
				V_VT(&var7) = VT_EMPTY;
				VariantChangeType(&var7, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var8;
				V_VT(&var8) = VT_EMPTY;
				VariantChangeType(&var8, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var9;
				V_VT(&var9) = VT_EMPTY;
				VariantChangeType(&var9, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var10;
				V_VT(&var10) = VT_EMPTY;
				VariantChangeType(&var10, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var11;
				V_VT(&var11) = VT_EMPTY;
				VariantChangeType(&var11, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = DisplayScene00ExWithId(V_BSTR(&var0), V_BOOL(&var1), V_BSTR(&var2), V_BSTR(&var3), V_I4(&var4), V_I4(&var5), V_BSTR(&var6), V_I4(&var7), V_I4(&var8), V_I4(&var9), V_I4(&var10), V_I4(&var11));
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
			}
			break;
		}
		case 0x6002001a: {
			if (wFlags & DISPATCH_METHOD) {
				// line 153: [id(0x6002001a)]HRESULT ModifyScene00([in] BSTR id,[in] BSTR toptext,[in] BSTR bottomtext);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var2;
				V_VT(&var2) = VT_EMPTY;
				VariantChangeType(&var2, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = ModifyScene00(V_BSTR(&var0), V_BSTR(&var1), V_BSTR(&var2));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
			}
			break;
		}
		case 0x6002001b: {
			if (wFlags & DISPATCH_METHOD) {
				// line 158: [id(0x6002001b)]HRESULT ModifyScene00Ex([in] BSTR id,[in] BSTR toptext,[in] BSTR bottomtext,[in] long pauseTime);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var2;
				V_VT(&var2) = VT_EMPTY;
				VariantChangeType(&var2, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var3;
				V_VT(&var3) = VT_EMPTY;
				VariantChangeType(&var3, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = ModifyScene00Ex(V_BSTR(&var0), V_BSTR(&var1), V_BSTR(&var2), V_I4(&var3));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
				VariantClear(&var3);
			}
			break;
		}
		case 0x6002001c: {
			if (wFlags & DISPATCH_METHOD) {
				// line 164: [id(0x6002001c)]HRESULT DisplayScene01([in] BSTR sceneId,[in] BSTR background,[in] BSTR text,[in] long textBrightness,[in] long textOutlineBrightness,[in] long animateIn,[in] long pauseTime,[in] long animateOut);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var2;
				V_VT(&var2) = VT_EMPTY;
				VariantChangeType(&var2, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var3;
				V_VT(&var3) = VT_EMPTY;
				VariantChangeType(&var3, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var4;
				V_VT(&var4) = VT_EMPTY;
				VariantChangeType(&var4, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var5;
				V_VT(&var5) = VT_EMPTY;
				VariantChangeType(&var5, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var6;
				V_VT(&var6) = VT_EMPTY;
				VariantChangeType(&var6, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var7;
				V_VT(&var7) = VT_EMPTY;
				VariantChangeType(&var7, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = DisplayScene01(V_BSTR(&var0), V_BSTR(&var1), V_BSTR(&var2), V_I4(&var3), V_I4(&var4), V_I4(&var5), V_I4(&var6), V_I4(&var7));
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
		case 0x6002001d: {
			if (wFlags & DISPATCH_METHOD) {
				// line 174: [id(0x6002001d)]HRESULT DisplayText([in] BSTR text,[in] long textBrightness,[in] long textOutlineBrightness);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var2;
				V_VT(&var2) = VT_EMPTY;
				VariantChangeType(&var2, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = DisplayText(V_BSTR(&var0), V_I4(&var1), V_I4(&var2));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
			}
			break;
		}
		case 0x6002001e: {
			if (wFlags & DISPATCH_METHOD) {
				// line 179: [id(0x6002001e)]HRESULT ScrollingCredits([in] BSTR background,[in] BSTR text,[in] long textBrightness,[in] long animateIn,[in] long pauseTime,[in] long animateOut);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_BSTR);
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
				hres = ScrollingCredits(V_BSTR(&var0), V_BSTR(&var1), V_I4(&var2), V_I4(&var3), V_I4(&var4), V_I4(&var5));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
				VariantClear(&var3);
				VariantClear(&var4);
				VariantClear(&var5);
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

