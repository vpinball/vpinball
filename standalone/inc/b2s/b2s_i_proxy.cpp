#include "stdafx.h"
#include "olectl.h"

#include "Server.h"

STDMETHODIMP Server::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} namesIdsList[] = {
			{ NULL },
			{ L"B2SHideScoreDisplays", 0x0000005c },
			{ L"B2SIsAnimationRunning", 0x00000056 },
			{ L"B2SMapSound", 0x00000060 },
			{ L"B2SName", 0x00000006 },
			{ L"B2SPlaySound", 0x0000005e },
			{ L"B2SPulseData", 0x00000037 },
			{ L"B2SServerDirectory", 0x00000003 },
			{ L"B2SServerVersion", 0x00000002 },
			{ L"B2SSetBallInPlay", 0x0000004d },
			{ L"B2SSetCanPlay", 0x0000004c },
			{ L"B2SSetCredits", 0x0000004a },
			{ L"B2SSetData", 0x00000036 },
			{ L"B2SSetGameOver", 0x00000050 },
			{ L"B2SSetIllumination", 0x00000038 },
			{ L"B2SSetLED", 0x00000039 },
			{ L"B2SSetLEDDisplay", 0x0000003a },
			{ L"B2SSetMatch", 0x0000004f },
			{ L"B2SSetPlayerUp", 0x0000004b },
			{ L"B2SSetReel", 0x0000003b },
			{ L"B2SSetScore", 0x0000003c },
			{ L"B2SSetScoreDigit", 0x00000044 },
			{ L"B2SSetScorePlayer", 0x0000003d },
			{ L"B2SSetScorePlayer1", 0x0000003e },
			{ L"B2SSetScorePlayer2", 0x0000003f },
			{ L"B2SSetScorePlayer3", 0x00000040 },
			{ L"B2SSetScorePlayer4", 0x00000041 },
			{ L"B2SSetScorePlayer5", 0x00000042 },
			{ L"B2SSetScorePlayer6", 0x00000043 },
			{ L"B2SSetScoreRollover", 0x00000045 },
			{ L"B2SSetScoreRolloverPlayer1", 0x00000046 },
			{ L"B2SSetScoreRolloverPlayer2", 0x00000047 },
			{ L"B2SSetScoreRolloverPlayer3", 0x00000048 },
			{ L"B2SSetScoreRolloverPlayer4", 0x00000049 },
			{ L"B2SSetShootAgain", 0x00000051 },
			{ L"B2SSetTilt", 0x0000004e },
			{ L"B2SShowScoreDisplays", 0x0000005b },
			{ L"B2SStartAnimation", 0x00000052 },
			{ L"B2SStartAnimationReverse", 0x00000053 },
			{ L"B2SStartRotation", 0x00000059 },
			{ L"B2SStartSound", 0x0000005d },
			{ L"B2SStopAllAnimations", 0x00000055 },
			{ L"B2SStopAnimation", 0x00000054 },
			{ L"B2SStopRotation", 0x0000005a },
			{ L"B2SStopSound", 0x0000005f },
			{ L"ChangedGIStrings", 0x00000024 },
			{ L"ChangedLamps", 0x00000022 },
			{ L"ChangedLEDs", 0x00000025 },
			{ L"ChangedNVRAM", 0x00000033 },
			{ L"ChangedSolenoids", 0x00000023 },
			{ L"CheckROMS", 0x0000001e },
			{ L"Dip", 0x0000002d },
			{ L"Dispose", 0x00000001 },
			{ L"DoubleSize", 0x00000018 },
			{ L"GameName", 0x00000004 },
			{ L"Games", 0x0000000a },
			{ L"GetMech", 0x0000002c },
			{ L"GIString", 0x00000029 },
			{ L"HandleKeyboard", 0x00000020 },
			{ L"HandleMechanics", 0x00000021 },
			{ L"Hidden", 0x00000019 },
			{ L"Lamp", 0x00000027 },
			{ L"LaunchBackglass", 0x00000011 },
			{ L"LockDisplay", 0x00000017 },
			{ L"Mech", 0x0000002b },
			{ L"NewSoundCommands", 0x00000026 },
			{ L"NVRAM", 0x00000034 },
			{ L"Pause", 0x0000000d },
			{ L"PuPHide", 0x0000001f },
			{ L"RawDmdColoredPixels", 0x00000032 },
			{ L"RawDmdHeight", 0x00000030 },
			{ L"RawDmdPixels", 0x00000031 },
			{ L"RawDmdWidth", 0x0000002f },
			{ L"ROMName", 0x00000005 },
			{ L"Run", 0x0000000f },
			{ L"Running", 0x0000000c },
			{ L"SetDisplayPosition", 0x0000001a },
			{ L"SetPath", 0x00000009 },
			{ L"Settings", 0x0000000b },
			{ L"ShowAboutDialog", 0x0000001d },
			{ L"ShowDMDOnly", 0x00000015 },
			{ L"ShowFrame", 0x00000013 },
			{ L"ShowOptsDialog", 0x0000001b },
			{ L"ShowPathesDialog", 0x0000001c },
			{ L"ShowPinDMD", 0x00000016 },
			{ L"ShowTitle", 0x00000014 },
			{ L"Solenoid", 0x00000028 },
			{ L"SolMask", 0x0000002e },
			{ L"SoundMode", 0x00000035 },
			{ L"SplashInfoLine", 0x00000012 },
			{ L"StartAnimation", 0x00000057 },
			{ L"Stop", 0x00000010 },
			{ L"StopAnimation", 0x00000058 },
			{ L"Switch", 0x0000002a },
			{ L"TableName", 0x00000007 },
			{ L"TimeFence", 0x00000061 },
			{ L"Version", 0x0000000e },
			{ L"WorkingDir", 0x00000008 }
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

STDMETHODIMP Server::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
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
		case 0x00000001: {
			if (wFlags & DISPATCH_METHOD) {
				// line 882: [id(0x00000001)]HRESULT Dispose();
				hres = Dispose();
			}
			break;
		}
		case 0x00000002: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 884: [id(0x00000002), propget]HRESULT B2SServerVersion([out, retval] BSTR* pRetVal);
				V_VT(&res) = VT_BSTR;
				hres = get_B2SServerVersion(&V_BSTR(&res));
			}
			break;
		}
		case 0x00000003: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 886: [id(0x00000003), propget]HRESULT B2SServerDirectory([out, retval] BSTR* pRetVal);
				V_VT(&res) = VT_BSTR;
				hres = get_B2SServerDirectory(&V_BSTR(&res));
			}
			break;
		}
		case 0x00000004: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 888: [id(0x00000004), propget]HRESULT GameName([out, retval] BSTR* pRetVal);
				V_VT(&res) = VT_BSTR;
				hres = get_GameName(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 890: [id(0x00000004), propput]HRESULT GameName([in] BSTR pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_GameName(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000005: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 892: [id(0x00000005), propget]HRESULT ROMName([out, retval] BSTR* pRetVal);
				V_VT(&res) = VT_BSTR;
				hres = get_ROMName(&V_BSTR(&res));
			}
			break;
		}
		case 0x00000006: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 894: [id(0x00000006), propget]HRESULT B2SName([out, retval] BSTR* pRetVal);
				V_VT(&res) = VT_BSTR;
				hres = get_B2SName(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 896: [id(0x00000006), propput]HRESULT B2SName([in] BSTR pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_B2SName(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000007: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 898: [id(0x00000007), propget]HRESULT TableName([out, retval] BSTR* pRetVal);
				V_VT(&res) = VT_BSTR;
				hres = get_TableName(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 900: [id(0x00000007), propput]HRESULT TableName([in] BSTR pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_TableName(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000008: {
			if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 902: [id(0x00000008), propput]HRESULT WorkingDir([in] BSTR rhs);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_WorkingDir(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000009: {
			if (wFlags & DISPATCH_METHOD) {
				// line 904: [id(0x00000009)]HRESULT SetPath([in] BSTR path);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = SetPath(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x0000000a: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 906: [id(0x0000000a), propget]HRESULT Games([in] BSTR GameName,[out, retval] VARIANT* pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = get_Games(V_BSTR(&var0), &res);
				VariantClear(&var0);
			}
			break;
		}
		case 0x0000000b: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 910: [id(0x0000000b), propget]HRESULT Settings([out, retval] VARIANT* pRetVal);
				hres = get_Settings(&res);
			}
			break;
		}
		case 0x0000000c: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 912: [id(0x0000000c), propget]HRESULT Running([out, retval] VARIANT_BOOL* pRetVal);
				V_VT(&res) = VT_BOOL;
				hres = get_Running(&V_BOOL(&res));
			}
			break;
		}
		case 0x0000000d: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 914: [id(0x0000000d), propget]HRESULT Pause([out, retval] VARIANT_BOOL* pRetVal);
				V_VT(&res) = VT_BOOL;
				hres = get_Pause(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 916: [id(0x0000000d), propput]HRESULT Pause([in] VARIANT_BOOL pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_Pause(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x0000000e: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 918: [id(0x0000000e), propget]HRESULT Version([out, retval] BSTR* pRetVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Version(&V_BSTR(&res));
			}
			break;
		}
		case 0x0000000f: {
			if (wFlags & DISPATCH_METHOD) {
				// line 920: [id(0x0000000f)]HRESULT Run([in, optional] VARIANT handle);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, (index > 0) ? &pDispParams->rgvarg[--index] : &var0);
				hres = Run(var0);
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000010: {
			if (wFlags & DISPATCH_METHOD) {
				// line 922: [id(0x00000010)]HRESULT Stop();
				hres = Stop();
			}
			break;
		}
		case 0x00000011: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 924: [id(0x00000011), propget]HRESULT LaunchBackglass([out, retval] VARIANT_BOOL* pRetVal);
				V_VT(&res) = VT_BOOL;
				hres = get_LaunchBackglass(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 926: [id(0x00000011), propput]HRESULT LaunchBackglass([in] VARIANT_BOOL pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_LaunchBackglass(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000012: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 928: [id(0x00000012), propget]HRESULT SplashInfoLine([out, retval] BSTR* pRetVal);
				V_VT(&res) = VT_BSTR;
				hres = get_SplashInfoLine(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 930: [id(0x00000012), propput]HRESULT SplashInfoLine([in] BSTR pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_SplashInfoLine(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000013: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 932: [id(0x00000013), propget]HRESULT ShowFrame([out, retval] VARIANT_BOOL* pRetVal);
				V_VT(&res) = VT_BOOL;
				hres = get_ShowFrame(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 934: [id(0x00000013), propput]HRESULT ShowFrame([in] VARIANT_BOOL pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_ShowFrame(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000014: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 936: [id(0x00000014), propget]HRESULT ShowTitle([out, retval] VARIANT_BOOL* pRetVal);
				V_VT(&res) = VT_BOOL;
				hres = get_ShowTitle(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 938: [id(0x00000014), propput]HRESULT ShowTitle([in] VARIANT_BOOL pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_ShowTitle(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000015: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 940: [id(0x00000015), propget]HRESULT ShowDMDOnly([out, retval] VARIANT_BOOL* pRetVal);
				V_VT(&res) = VT_BOOL;
				hres = get_ShowDMDOnly(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 942: [id(0x00000015), propput]HRESULT ShowDMDOnly([in] VARIANT_BOOL pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_ShowDMDOnly(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000016: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 944: [id(0x00000016), propget]HRESULT ShowPinDMD([out, retval] VARIANT_BOOL* pRetVal);
				V_VT(&res) = VT_BOOL;
				hres = get_ShowPinDMD(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 946: [id(0x00000016), propput]HRESULT ShowPinDMD([in] VARIANT_BOOL pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_ShowPinDMD(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000017: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 948: [id(0x00000017), propget]HRESULT LockDisplay([out, retval] VARIANT_BOOL* pRetVal);
				V_VT(&res) = VT_BOOL;
				hres = get_LockDisplay(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 950: [id(0x00000017), propput]HRESULT LockDisplay([in] VARIANT_BOOL pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_LockDisplay(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000018: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 952: [id(0x00000018), propget]HRESULT DoubleSize([out, retval] VARIANT_BOOL* pRetVal);
				V_VT(&res) = VT_BOOL;
				hres = get_DoubleSize(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 954: [id(0x00000018), propput]HRESULT DoubleSize([in] VARIANT_BOOL pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_DoubleSize(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000019: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 956: [id(0x00000019), propget]HRESULT Hidden([out, retval] VARIANT_BOOL* pRetVal);
				V_VT(&res) = VT_BOOL;
				hres = get_Hidden(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 958: [id(0x00000019), propput]HRESULT Hidden([in] VARIANT_BOOL pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_Hidden(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x0000001a: {
			if (wFlags & DISPATCH_METHOD) {
				// line 960: [id(0x0000001a)]HRESULT SetDisplayPosition([in] VARIANT x,[in] VARIANT y,[in, optional] VARIANT handle);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantCopyInd(&var1, &pDispParams->rgvarg[--index]);
				VARIANT var2;
				V_VT(&var2) = VT_EMPTY;
				VariantCopyInd(&var2, (index > 0) ? &pDispParams->rgvarg[--index] : &var2);
				hres = SetDisplayPosition(var0, var1, var2);
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
			}
			break;
		}
		case 0x0000001b: {
			if (wFlags & DISPATCH_METHOD) {
				// line 965: [id(0x0000001b)]HRESULT ShowOptsDialog([in] VARIANT handle);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = ShowOptsDialog(var0);
				VariantClear(&var0);
			}
			break;
		}
		case 0x0000001c: {
			if (wFlags & DISPATCH_METHOD) {
				// line 967: [id(0x0000001c)]HRESULT ShowPathesDialog([in] VARIANT handle);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = ShowPathesDialog(var0);
				VariantClear(&var0);
			}
			break;
		}
		case 0x0000001d: {
			if (wFlags & DISPATCH_METHOD) {
				// line 969: [id(0x0000001d)]HRESULT ShowAboutDialog([in] VARIANT handle);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = ShowAboutDialog(var0);
				VariantClear(&var0);
			}
			break;
		}
		case 0x0000001e: {
			if (wFlags & DISPATCH_METHOD) {
				// line 971: [id(0x0000001e)]HRESULT CheckROMS([in] VARIANT showoptions,[in] VARIANT handle);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantCopyInd(&var1, &pDispParams->rgvarg[--index]);
				hres = CheckROMS(var0, var1);
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x0000001f: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 975: [id(0x0000001f), propget]HRESULT PuPHide([out, retval] VARIANT_BOOL* pRetVal);
				V_VT(&res) = VT_BOOL;
				hres = get_PuPHide(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 977: [id(0x0000001f), propput]HRESULT PuPHide([in] VARIANT_BOOL pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_PuPHide(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000020: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 979: [id(0x00000020), propget]HRESULT HandleKeyboard([out, retval] VARIANT_BOOL* pRetVal);
				V_VT(&res) = VT_BOOL;
				hres = get_HandleKeyboard(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 981: [id(0x00000020), propput]HRESULT HandleKeyboard([in] VARIANT_BOOL pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_HandleKeyboard(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000021: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 983: [id(0x00000021), propget]HRESULT HandleMechanics([out, retval] short* pRetVal);
				V_VT(&res) = VT_I2;
				hres = get_HandleMechanics((short*)&V_I2(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 985: [id(0x00000021), propput]HRESULT HandleMechanics([in] short pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I2);
				hres = put_HandleMechanics(V_I2(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000022: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 987: [id(0x00000022), propget]HRESULT ChangedLamps([out, retval] VARIANT* pRetVal);
				hres = get_ChangedLamps(&res);
			}
			break;
		}
		case 0x00000023: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 989: [id(0x00000023), propget]HRESULT ChangedSolenoids([out, retval] VARIANT* pRetVal);
				hres = get_ChangedSolenoids(&res);
			}
			break;
		}
		case 0x00000024: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 991: [id(0x00000024), propget]HRESULT ChangedGIStrings([out, retval] VARIANT* pRetVal);
				hres = get_ChangedGIStrings(&res);
			}
			break;
		}
		case 0x00000025: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 993: [id(0x00000025), propget]HRESULT ChangedLEDs([in] VARIANT mask2,[in] VARIANT mask1,[in, optional] VARIANT mask3,[in, optional] VARIANT mask4,[out, retval] VARIANT* pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantCopyInd(&var1, &pDispParams->rgvarg[--index]);
				VARIANT var2;
				V_VT(&var2) = VT_EMPTY;
				VariantCopyInd(&var2, (index > 0) ? &pDispParams->rgvarg[--index] : &var2);
				VARIANT var3;
				V_VT(&var3) = VT_EMPTY;
				VariantCopyInd(&var3, (index > 0) ? &pDispParams->rgvarg[--index] : &var3);
				hres = get_ChangedLEDs(var0, var1, var2, var3, &res);
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
				VariantClear(&var3);
			}
			break;
		}
		case 0x00000026: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1000: [id(0x00000026), propget]HRESULT NewSoundCommands([out, retval] VARIANT* pRetVal);
				hres = get_NewSoundCommands(&res);
			}
			break;
		}
		case 0x00000027: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1002: [id(0x00000027), propget]HRESULT Lamp([in] VARIANT number,[out, retval] VARIANT_BOOL* pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				V_VT(&res) = VT_BOOL;
				hres = get_Lamp(var0, &V_BOOL(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000028: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1006: [id(0x00000028), propget]HRESULT Solenoid([in] VARIANT number,[out, retval] VARIANT_BOOL* pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				V_VT(&res) = VT_BOOL;
				hres = get_Solenoid(var0, &V_BOOL(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000029: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1010: [id(0x00000029), propget]HRESULT GIString([in] VARIANT number,[out, retval] VARIANT_BOOL* pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				V_VT(&res) = VT_BOOL;
				hres = get_GIString(var0, &V_BOOL(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 0x0000002a: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1014: [id(0x0000002a), propget]HRESULT Switch([in] VARIANT number,[out, retval] VARIANT_BOOL* pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				V_VT(&res) = VT_BOOL;
				hres = get_Switch(var0, &V_BOOL(&res));
				VariantClear(&var0);
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1018: [id(0x0000002a), propput]HRESULT Switch([in] VARIANT number,[in] VARIANT_BOOL pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_Switch(var0, V_BOOL(&var1));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x0000002b: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1022: [id(0x0000002b), propget]HRESULT Mech([in] VARIANT number,[out, retval] long* pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				V_VT(&res) = VT_I4;
				hres = get_Mech(var0, (LONG*)&V_I4(&res));
				VariantClear(&var0);
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1026: [id(0x0000002b), propput]HRESULT Mech([in] VARIANT number,[in] long pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_Mech(var0, V_I4(&var1));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x0000002c: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1030: [id(0x0000002c), propget]HRESULT GetMech([in] VARIANT number,[out, retval] VARIANT* pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = get_GetMech(var0, &res);
				VariantClear(&var0);
			}
			break;
		}
		case 0x0000002d: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1034: [id(0x0000002d), propget]HRESULT Dip([in] VARIANT number,[out, retval] long* pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				V_VT(&res) = VT_I4;
				hres = get_Dip(var0, (LONG*)&V_I4(&res));
				VariantClear(&var0);
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1038: [id(0x0000002d), propput]HRESULT Dip([in] VARIANT number,[in] long pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_Dip(var0, V_I4(&var1));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x0000002e: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1042: [id(0x0000002e), propget]HRESULT SolMask([in] VARIANT number,[out, retval] long* pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				V_VT(&res) = VT_I4;
				hres = get_SolMask(var0, (LONG*)&V_I4(&res));
				VariantClear(&var0);
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1046: [id(0x0000002e), propput]HRESULT SolMask([in] VARIANT number,[in] long pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_SolMask(var0, V_I4(&var1));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x0000002f: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1050: [id(0x0000002f), propget]HRESULT RawDmdWidth([out, retval] long* pRetVal);
				V_VT(&res) = VT_I4;
				hres = get_RawDmdWidth((LONG*)&V_I4(&res));
			}
			break;
		}
		case 0x00000030: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1052: [id(0x00000030), propget]HRESULT RawDmdHeight([out, retval] long* pRetVal);
				V_VT(&res) = VT_I4;
				hres = get_RawDmdHeight((LONG*)&V_I4(&res));
			}
			break;
		}
		case 0x00000031: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1054: [id(0x00000031), propget]HRESULT RawDmdPixels([out, retval] VARIANT* pRetVal);
				hres = get_RawDmdPixels(&res);
			}
			break;
		}
		case 0x00000032: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1056: [id(0x00000032), propget]HRESULT RawDmdColoredPixels([out, retval] VARIANT* pRetVal);
				hres = get_RawDmdColoredPixels(&res);
			}
			break;
		}
		case 0x00000033: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1058: [id(0x00000033), propget]HRESULT ChangedNVRAM([out, retval] VARIANT* pRetVal);
				hres = get_ChangedNVRAM(&res);
			}
			break;
		}
		case 0x00000034: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1060: [id(0x00000034), propget]HRESULT NVRAM([out, retval] VARIANT* pRetVal);
				hres = get_NVRAM(&res);
			}
			break;
		}
		case 0x00000035: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1062: [id(0x00000035), propget]HRESULT SoundMode([out, retval] long* pRetVal);
				V_VT(&res) = VT_I4;
				hres = get_SoundMode((LONG*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1064: [id(0x00000035), propput]HRESULT SoundMode([in] long pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_SoundMode(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000036: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1066: [id(0x00000036)]HRESULT B2SSetData([in] VARIANT idORname,[in] VARIANT value);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantCopyInd(&var1, &pDispParams->rgvarg[--index]);
				hres = B2SSetData(var0, var1);
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x00000037: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1070: [id(0x00000037)]HRESULT B2SPulseData([in] VARIANT idORname);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = B2SPulseData(var0);
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000038: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1072: [id(0x00000038)]HRESULT B2SSetIllumination([in] VARIANT name,[in] VARIANT value);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantCopyInd(&var1, &pDispParams->rgvarg[--index]);
				hres = B2SSetIllumination(var0, var1);
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x00000039: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1076: [id(0x00000039)]HRESULT B2SSetLED([in] VARIANT digit,[in] VARIANT valueORtext);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantCopyInd(&var1, &pDispParams->rgvarg[--index]);
				hres = B2SSetLED(var0, var1);
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x0000003a: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1080: [id(0x0000003a)]HRESULT B2SSetLEDDisplay([in] VARIANT display,[in] VARIANT text);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantCopyInd(&var1, &pDispParams->rgvarg[--index]);
				hres = B2SSetLEDDisplay(var0, var1);
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x0000003b: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1084: [id(0x0000003b)]HRESULT B2SSetReel([in] VARIANT digit,[in] VARIANT value);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantCopyInd(&var1, &pDispParams->rgvarg[--index]);
				hres = B2SSetReel(var0, var1);
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x0000003c: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1088: [id(0x0000003c)]HRESULT B2SSetScore([in] VARIANT display,[in] VARIANT value);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantCopyInd(&var1, &pDispParams->rgvarg[--index]);
				hres = B2SSetScore(var0, var1);
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x0000003d: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1092: [id(0x0000003d)]HRESULT B2SSetScorePlayer([in] VARIANT playerno,[in] VARIANT score);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantCopyInd(&var1, &pDispParams->rgvarg[--index]);
				hres = B2SSetScorePlayer(var0, var1);
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x0000003e: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1096: [id(0x0000003e)]HRESULT B2SSetScorePlayer1([in] VARIANT score);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = B2SSetScorePlayer1(var0);
				VariantClear(&var0);
			}
			break;
		}
		case 0x0000003f: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1098: [id(0x0000003f)]HRESULT B2SSetScorePlayer2([in] VARIANT score);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = B2SSetScorePlayer2(var0);
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000040: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1100: [id(0x00000040)]HRESULT B2SSetScorePlayer3([in] VARIANT score);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = B2SSetScorePlayer3(var0);
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000041: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1102: [id(0x00000041)]HRESULT B2SSetScorePlayer4([in] VARIANT score);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = B2SSetScorePlayer4(var0);
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000042: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1104: [id(0x00000042)]HRESULT B2SSetScorePlayer5([in] VARIANT score);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = B2SSetScorePlayer5(var0);
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000043: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1106: [id(0x00000043)]HRESULT B2SSetScorePlayer6([in] VARIANT score);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = B2SSetScorePlayer6(var0);
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000044: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1108: [id(0x00000044)]HRESULT B2SSetScoreDigit([in] VARIANT digit,[in] VARIANT value);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantCopyInd(&var1, &pDispParams->rgvarg[--index]);
				hres = B2SSetScoreDigit(var0, var1);
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x00000045: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1112: [id(0x00000045)]HRESULT B2SSetScoreRollover([in] VARIANT id,[in] VARIANT value);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantCopyInd(&var1, &pDispParams->rgvarg[--index]);
				hres = B2SSetScoreRollover(var0, var1);
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x00000046: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1116: [id(0x00000046)]HRESULT B2SSetScoreRolloverPlayer1([in] VARIANT value);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = B2SSetScoreRolloverPlayer1(var0);
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000047: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1118: [id(0x00000047)]HRESULT B2SSetScoreRolloverPlayer2([in] VARIANT value);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = B2SSetScoreRolloverPlayer2(var0);
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000048: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1120: [id(0x00000048)]HRESULT B2SSetScoreRolloverPlayer3([in] VARIANT value);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = B2SSetScoreRolloverPlayer3(var0);
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000049: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1122: [id(0x00000049)]HRESULT B2SSetScoreRolloverPlayer4([in] VARIANT value);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = B2SSetScoreRolloverPlayer4(var0);
				VariantClear(&var0);
			}
			break;
		}
		case 0x0000004a: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1124: [id(0x0000004a)]HRESULT B2SSetCredits([in] VARIANT digitORvalue,[in, optional] VARIANT value);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantCopyInd(&var1, (index > 0) ? &pDispParams->rgvarg[--index] : &var1);
				hres = B2SSetCredits(var0, var1);
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x0000004b: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1128: [id(0x0000004b)]HRESULT B2SSetPlayerUp([in] VARIANT idORvalue,[in, optional] VARIANT value);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantCopyInd(&var1, (index > 0) ? &pDispParams->rgvarg[--index] : &var1);
				hres = B2SSetPlayerUp(var0, var1);
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x0000004c: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1132: [id(0x0000004c)]HRESULT B2SSetCanPlay([in] VARIANT idORvalue,[in, optional] VARIANT value);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantCopyInd(&var1, (index > 0) ? &pDispParams->rgvarg[--index] : &var1);
				hres = B2SSetCanPlay(var0, var1);
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x0000004d: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1136: [id(0x0000004d)]HRESULT B2SSetBallInPlay([in] VARIANT idORvalue,[in, optional] VARIANT value);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantCopyInd(&var1, (index > 0) ? &pDispParams->rgvarg[--index] : &var1);
				hres = B2SSetBallInPlay(var0, var1);
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x0000004e: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1140: [id(0x0000004e)]HRESULT B2SSetTilt([in] VARIANT idORvalue,[in, optional] VARIANT value);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantCopyInd(&var1, (index > 0) ? &pDispParams->rgvarg[--index] : &var1);
				hres = B2SSetTilt(var0, var1);
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x0000004f: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1144: [id(0x0000004f)]HRESULT B2SSetMatch([in] VARIANT idORvalue,[in, optional] VARIANT value);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantCopyInd(&var1, (index > 0) ? &pDispParams->rgvarg[--index] : &var1);
				hres = B2SSetMatch(var0, var1);
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x00000050: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1148: [id(0x00000050)]HRESULT B2SSetGameOver([in] VARIANT idORvalue,[in, optional] VARIANT value);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantCopyInd(&var1, (index > 0) ? &pDispParams->rgvarg[--index] : &var1);
				hres = B2SSetGameOver(var0, var1);
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x00000051: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1152: [id(0x00000051)]HRESULT B2SSetShootAgain([in] VARIANT idORvalue,[in, optional] VARIANT value);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantCopyInd(&var1, (index > 0) ? &pDispParams->rgvarg[--index] : &var1);
				hres = B2SSetShootAgain(var0, var1);
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x00000052: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1156: [id(0x00000052)]HRESULT B2SStartAnimation([in] BSTR animationname,[in, optional] VARIANT_BOOL playreverse);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, (index > 0) ? &pDispParams->rgvarg[--index] : &var1, 0, VT_BOOL);
				hres = B2SStartAnimation(V_BSTR(&var0), V_BOOL(&var1));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x00000053: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1160: [id(0x00000053)]HRESULT B2SStartAnimationReverse([in] BSTR animationname);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = B2SStartAnimationReverse(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000054: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1162: [id(0x00000054)]HRESULT B2SStopAnimation([in] BSTR animationname);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = B2SStopAnimation(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000055: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1164: [id(0x00000055)]HRESULT B2SStopAllAnimations();
				hres = B2SStopAllAnimations();
			}
			break;
		}
		case 0x00000056: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1166: [id(0x00000056)]HRESULT B2SIsAnimationRunning([in] BSTR animationname,[out, retval] VARIANT_BOOL* pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				V_VT(&res) = VT_BOOL;
				hres = B2SIsAnimationRunning(V_BSTR(&var0), &V_BOOL(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000057: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1170: [id(0x00000057)]HRESULT StartAnimation([in] BSTR animationname,[in, optional] VARIANT_BOOL playreverse);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, (index > 0) ? &pDispParams->rgvarg[--index] : &var1, 0, VT_BOOL);
				hres = StartAnimation(V_BSTR(&var0), V_BOOL(&var1));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x00000058: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1174: [id(0x00000058)]HRESULT StopAnimation([in] BSTR animationname);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = StopAnimation(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000059: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1176: [id(0x00000059)]HRESULT B2SStartRotation();
				hres = B2SStartRotation();
			}
			break;
		}
		case 0x0000005a: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1178: [id(0x0000005a)]HRESULT B2SStopRotation();
				hres = B2SStopRotation();
			}
			break;
		}
		case 0x0000005b: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1180: [id(0x0000005b)]HRESULT B2SShowScoreDisplays();
				hres = B2SShowScoreDisplays();
			}
			break;
		}
		case 0x0000005c: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1182: [id(0x0000005c)]HRESULT B2SHideScoreDisplays();
				hres = B2SHideScoreDisplays();
			}
			break;
		}
		case 0x0000005d: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1184: [id(0x0000005d)]HRESULT B2SStartSound([in] BSTR soundname);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = B2SStartSound(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x0000005e: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1186: [id(0x0000005e)]HRESULT B2SPlaySound([in] BSTR soundname);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = B2SPlaySound(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x0000005f: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1188: [id(0x0000005f)]HRESULT B2SStopSound([in] BSTR soundname);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = B2SStopSound(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000060: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1190: [id(0x00000060)]HRESULT B2SMapSound([in] VARIANT digit,[in] BSTR soundname);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = B2SMapSound(var0, V_BSTR(&var1));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x00000061: {
			if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1194: [id(0x00000061), propput]HRESULT TimeFence([in] VARIANT timeInS);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = put_TimeFence(var0);
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

