#include "stdafx.h"
#include "olectl.h"

#include "Server.h"

STDMETHODIMP Server::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} namesIdsList[] = {
			{ NULL },
			{ L"B2SBuildVersion", 0x00000003 },
			{ L"B2SHideScoreDisplays", 0x00000060 },
			{ L"B2SIsAnimationRunning", 0x0000005a },
			{ L"B2SMapSound", 0x00000064 },
			{ L"B2SName", 0x00000007 },
			{ L"B2SPlaySound", 0x00000062 },
			{ L"B2SPulseData", 0x0000003a },
			{ L"B2SServerDirectory", 0x00000004 },
			{ L"B2SServerVersion", 0x00000002 },
			{ L"B2SSetBallInPlay", 0x00000051 },
			{ L"B2SSetCanPlay", 0x00000050 },
			{ L"B2SSetCredits", 0x0000004e },
			{ L"B2SSetData", 0x00000039 },
			{ L"B2SSetGameOver", 0x00000054 },
			{ L"B2SSetIllumination", 0x0000003c },
			{ L"B2SSetLED", 0x0000003d },
			{ L"B2SSetLEDDisplay", 0x0000003e },
			{ L"B2SSetMatch", 0x00000053 },
			{ L"B2SSetPlayerUp", 0x0000004f },
			{ L"B2SSetPos", 0x0000003b },
			{ L"B2SSetReel", 0x0000003f },
			{ L"B2SSetScore", 0x00000040 },
			{ L"B2SSetScoreDigit", 0x00000048 },
			{ L"B2SSetScorePlayer", 0x00000041 },
			{ L"B2SSetScorePlayer1", 0x00000042 },
			{ L"B2SSetScorePlayer2", 0x00000043 },
			{ L"B2SSetScorePlayer3", 0x00000044 },
			{ L"B2SSetScorePlayer4", 0x00000045 },
			{ L"B2SSetScorePlayer5", 0x00000046 },
			{ L"B2SSetScorePlayer6", 0x00000047 },
			{ L"B2SSetScoreRollover", 0x00000049 },
			{ L"B2SSetScoreRolloverPlayer1", 0x0000004a },
			{ L"B2SSetScoreRolloverPlayer2", 0x0000004b },
			{ L"B2SSetScoreRolloverPlayer3", 0x0000004c },
			{ L"B2SSetScoreRolloverPlayer4", 0x0000004d },
			{ L"B2SSetShootAgain", 0x00000055 },
			{ L"B2SSetTilt", 0x00000052 },
			{ L"B2SShowScoreDisplays", 0x0000005f },
			{ L"B2SStartAnimation", 0x00000056 },
			{ L"B2SStartAnimationReverse", 0x00000057 },
			{ L"B2SStartRotation", 0x0000005d },
			{ L"B2SStartSound", 0x00000061 },
			{ L"B2SStopAllAnimations", 0x00000059 },
			{ L"B2SStopAnimation", 0x00000058 },
			{ L"B2SStopRotation", 0x0000005e },
			{ L"B2SStopSound", 0x00000063 },
			{ L"ChangedGIStrings", 0x00000027 },
			{ L"ChangedLamps", 0x00000025 },
			{ L"ChangedLEDs", 0x00000028 },
			{ L"ChangedNVRAM", 0x00000036 },
			{ L"ChangedSolenoids", 0x00000026 },
			{ L"CheckROMS", 0x00000021 },
			{ L"Dip", 0x00000030 },
			{ L"Dispose", 0x00000001 },
			{ L"DoubleSize", 0x0000001b },
			{ L"GameName", 0x00000005 },
			{ L"Games", 0x0000000b },
			{ L"GetMech", 0x0000002f },
			{ L"GIString", 0x0000002c },
			{ L"HandleKeyboard", 0x00000023 },
			{ L"HandleMechanics", 0x00000024 },
			{ L"Hidden", 0x0000001c },
			{ L"Lamp", 0x0000002a },
			{ L"LaunchBackglass", 0x00000014 },
			{ L"LockDisplay", 0x0000001a },
			{ L"Mech", 0x0000002e },
			{ L"NewSoundCommands", 0x00000029 },
			{ L"NVRAM", 0x00000037 },
			{ L"Pause", 0x0000000f },
			{ L"PuPHide", 0x00000022 },
			{ L"RawDmdColoredPixels", 0x00000035 },
			{ L"RawDmdHeight", 0x00000033 },
			{ L"RawDmdPixels", 0x00000034 },
			{ L"RawDmdWidth", 0x00000032 },
			{ L"ROMName", 0x00000006 },
			{ L"Run", 0x00000012 },
			{ L"Running", 0x0000000d },
			{ L"SetDisplayPosition", 0x0000001d },
			{ L"SetPath", 0x0000000a },
			{ L"Settings", 0x0000000c },
			{ L"ShowAboutDialog", 0x00000020 },
			{ L"ShowDMDOnly", 0x00000018 },
			{ L"ShowFrame", 0x00000016 },
			{ L"ShowOptsDialog", 0x0000001e },
			{ L"ShowPathesDialog", 0x0000001f },
			{ L"ShowPinDMD", 0x00000019 },
			{ L"ShowTitle", 0x00000017 },
			{ L"Solenoid", 0x0000002b },
			{ L"SolMask", 0x00000031 },
			{ L"SoundMode", 0x00000038 },
			{ L"SplashInfoLine", 0x00000015 },
			{ L"StartAnimation", 0x0000005b },
			{ L"Stop", 0x00000013 },
			{ L"StopAnimation", 0x0000005c },
			{ L"Switch", 0x0000002d },
			{ L"TableName", 0x00000008 },
			{ L"TimeFence", 0x0000000e },
			{ L"Version", 0x00000010 },
			{ L"VPMBuildVersion", 0x00000011 },
			{ L"WorkingDir", 0x00000009 }
	};

	size_t min = 1, max = ARRAY_SIZE(namesIdsList) - 1, i;
	int r;
	while(min <= max) {
		i = (min + max) / 2;
		r = wcsicmp(namesIdsList[i].name, *rgszNames);
		if (!r) {
			*rgDispId = namesIdsList[i].dispId;
			return S_OK;
		}
		if (r < 0)
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
				// Default method
				V_VT(&res) = VT_DISPATCH;
				V_DISPATCH(&res) = this;
				hres = S_OK;
			}
			break;
		}
		case 0x00000001: {
			if (wFlags & DISPATCH_METHOD) {
				// line 908: [id(0x00000001)]HRESULT Dispose();
				hres = Dispose();
			}
			break;
		}
		case 0x00000002: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 910: [id(0x00000002), propget]HRESULT B2SServerVersion([out, retval] BSTR* pRetVal);
				V_VT(&res) = VT_BSTR;
				hres = get_B2SServerVersion(&V_BSTR(&res));
			}
			break;
		}
		case 0x00000003: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 912: [id(0x00000003), propget]HRESULT B2SBuildVersion([out, retval] double* pRetVal);
				V_VT(&res) = VT_R8;
				hres = get_B2SBuildVersion((double*)&V_R8(&res));
			}
			break;
		}
		case 0x00000004: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 914: [id(0x00000004), propget]HRESULT B2SServerDirectory([out, retval] BSTR* pRetVal);
				V_VT(&res) = VT_BSTR;
				hres = get_B2SServerDirectory(&V_BSTR(&res));
			}
			break;
		}
		case 0x00000005: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 916: [id(0x00000005), propget]HRESULT GameName([out, retval] BSTR* pRetVal);
				V_VT(&res) = VT_BSTR;
				hres = get_GameName(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 918: [id(0x00000005), propput]HRESULT GameName([in] BSTR pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_GameName(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000006: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 920: [id(0x00000006), propget]HRESULT ROMName([out, retval] BSTR* pRetVal);
				V_VT(&res) = VT_BSTR;
				hres = get_ROMName(&V_BSTR(&res));
			}
			break;
		}
		case 0x00000007: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 922: [id(0x00000007), propget]HRESULT B2SName([out, retval] BSTR* pRetVal);
				V_VT(&res) = VT_BSTR;
				hres = get_B2SName(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 924: [id(0x00000007), propput]HRESULT B2SName([in] BSTR pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_B2SName(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000008: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 926: [id(0x00000008), propget]HRESULT TableName([out, retval] BSTR* pRetVal);
				V_VT(&res) = VT_BSTR;
				hres = get_TableName(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 928: [id(0x00000008), propput]HRESULT TableName([in] BSTR pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_TableName(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000009: {
			if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 930: [id(0x00000009), propput]HRESULT WorkingDir([in] BSTR rhs);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_WorkingDir(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x0000000a: {
			if (wFlags & DISPATCH_METHOD) {
				// line 932: [id(0x0000000a)]HRESULT SetPath([in] BSTR path);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = SetPath(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x0000000b: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 934: [id(0x0000000b), propget]HRESULT Games([in] BSTR GameName,[out, retval] VARIANT* pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = get_Games(V_BSTR(&var0), &res);
				VariantClear(&var0);
			}
			break;
		}
		case 0x0000000c: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 938: [id(0x0000000c), propget]HRESULT Settings([out, retval] VARIANT* pRetVal);
				hres = get_Settings(&res);
			}
			break;
		}
		case 0x0000000d: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 940: [id(0x0000000d), propget]HRESULT Running([out, retval] VARIANT_BOOL* pRetVal);
				V_VT(&res) = VT_BOOL;
				hres = get_Running(&V_BOOL(&res));
			}
			break;
		}
		case 0x0000000e: {
			if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 942: [id(0x0000000e), propput]HRESULT TimeFence([in] double rhs);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R8);
				hres = put_TimeFence(V_R8(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x0000000f: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 944: [id(0x0000000f), propget]HRESULT Pause([out, retval] VARIANT_BOOL* pRetVal);
				V_VT(&res) = VT_BOOL;
				hres = get_Pause(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 946: [id(0x0000000f), propput]HRESULT Pause([in] VARIANT_BOOL pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_Pause(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000010: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 948: [id(0x00000010), propget]HRESULT Version([out, retval] BSTR* pRetVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Version(&V_BSTR(&res));
			}
			break;
		}
		case 0x00000011: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 950: [id(0x00000011), propget]HRESULT VPMBuildVersion([out, retval] BSTR* pRetVal);
				V_VT(&res) = VT_BSTR;
				hres = get_VPMBuildVersion(&V_BSTR(&res));
			}
			break;
		}
		case 0x00000012: {
			if (wFlags & DISPATCH_METHOD) {
				// line 952: [id(0x00000012)]HRESULT Run([in, optional] VARIANT handle);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, (index > 0) ? &pDispParams->rgvarg[--index] : &var0);
				hres = Run(var0);
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000013: {
			if (wFlags & DISPATCH_METHOD) {
				// line 954: [id(0x00000013)]HRESULT Stop();
				hres = Stop();
			}
			break;
		}
		case 0x00000014: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 956: [id(0x00000014), propget]HRESULT LaunchBackglass([out, retval] VARIANT_BOOL* pRetVal);
				V_VT(&res) = VT_BOOL;
				hres = get_LaunchBackglass(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 958: [id(0x00000014), propput]HRESULT LaunchBackglass([in] VARIANT_BOOL pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_LaunchBackglass(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000015: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 960: [id(0x00000015), propget]HRESULT SplashInfoLine([out, retval] BSTR* pRetVal);
				V_VT(&res) = VT_BSTR;
				hres = get_SplashInfoLine(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 962: [id(0x00000015), propput]HRESULT SplashInfoLine([in] BSTR pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_SplashInfoLine(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000016: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 964: [id(0x00000016), propget]HRESULT ShowFrame([out, retval] VARIANT_BOOL* pRetVal);
				V_VT(&res) = VT_BOOL;
				hres = get_ShowFrame(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 966: [id(0x00000016), propput]HRESULT ShowFrame([in] VARIANT_BOOL pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_ShowFrame(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000017: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 968: [id(0x00000017), propget]HRESULT ShowTitle([out, retval] VARIANT_BOOL* pRetVal);
				V_VT(&res) = VT_BOOL;
				hres = get_ShowTitle(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 970: [id(0x00000017), propput]HRESULT ShowTitle([in] VARIANT_BOOL pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_ShowTitle(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000018: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 972: [id(0x00000018), propget]HRESULT ShowDMDOnly([out, retval] VARIANT_BOOL* pRetVal);
				V_VT(&res) = VT_BOOL;
				hres = get_ShowDMDOnly(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 974: [id(0x00000018), propput]HRESULT ShowDMDOnly([in] VARIANT_BOOL pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_ShowDMDOnly(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000019: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 976: [id(0x00000019), propget]HRESULT ShowPinDMD([out, retval] VARIANT_BOOL* pRetVal);
				V_VT(&res) = VT_BOOL;
				hres = get_ShowPinDMD(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 978: [id(0x00000019), propput]HRESULT ShowPinDMD([in] VARIANT_BOOL pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_ShowPinDMD(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x0000001a: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 980: [id(0x0000001a), propget]HRESULT LockDisplay([out, retval] VARIANT_BOOL* pRetVal);
				V_VT(&res) = VT_BOOL;
				hres = get_LockDisplay(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 982: [id(0x0000001a), propput]HRESULT LockDisplay([in] VARIANT_BOOL pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_LockDisplay(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x0000001b: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 984: [id(0x0000001b), propget]HRESULT DoubleSize([out, retval] VARIANT_BOOL* pRetVal);
				V_VT(&res) = VT_BOOL;
				hres = get_DoubleSize(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 986: [id(0x0000001b), propput]HRESULT DoubleSize([in] VARIANT_BOOL pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_DoubleSize(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x0000001c: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 988: [id(0x0000001c), propget]HRESULT Hidden([out, retval] VARIANT_BOOL* pRetVal);
				V_VT(&res) = VT_BOOL;
				hres = get_Hidden(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 990: [id(0x0000001c), propput]HRESULT Hidden([in] VARIANT_BOOL pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_Hidden(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x0000001d: {
			if (wFlags & DISPATCH_METHOD) {
				// line 992: [id(0x0000001d)]HRESULT SetDisplayPosition([in] VARIANT x,[in] VARIANT y,[in, optional] VARIANT handle);
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
		case 0x0000001e: {
			if (wFlags & DISPATCH_METHOD) {
				// line 997: [id(0x0000001e)]HRESULT ShowOptsDialog([in] VARIANT handle);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = ShowOptsDialog(var0);
				VariantClear(&var0);
			}
			break;
		}
		case 0x0000001f: {
			if (wFlags & DISPATCH_METHOD) {
				// line 999: [id(0x0000001f)]HRESULT ShowPathesDialog([in] VARIANT handle);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = ShowPathesDialog(var0);
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000020: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1001: [id(0x00000020)]HRESULT ShowAboutDialog([in] VARIANT handle);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = ShowAboutDialog(var0);
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000021: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1003: [id(0x00000021)]HRESULT CheckROMS([in] VARIANT showoptions,[in] VARIANT handle);
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
		case 0x00000022: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1007: [id(0x00000022), propget]HRESULT PuPHide([out, retval] VARIANT_BOOL* pRetVal);
				V_VT(&res) = VT_BOOL;
				hres = get_PuPHide(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1009: [id(0x00000022), propput]HRESULT PuPHide([in] VARIANT_BOOL pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_PuPHide(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000023: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1011: [id(0x00000023), propget]HRESULT HandleKeyboard([out, retval] VARIANT_BOOL* pRetVal);
				V_VT(&res) = VT_BOOL;
				hres = get_HandleKeyboard(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1013: [id(0x00000023), propput]HRESULT HandleKeyboard([in] VARIANT_BOOL pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_HandleKeyboard(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000024: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1015: [id(0x00000024), propget]HRESULT HandleMechanics([out, retval] short* pRetVal);
				V_VT(&res) = VT_I2;
				hres = get_HandleMechanics((short*)&V_I2(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1017: [id(0x00000024), propput]HRESULT HandleMechanics([in] short pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I2);
				hres = put_HandleMechanics(V_I2(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000025: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1019: [id(0x00000025), propget]HRESULT ChangedLamps([out, retval] VARIANT* pRetVal);
				hres = get_ChangedLamps(&res);
			}
			break;
		}
		case 0x00000026: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1021: [id(0x00000026), propget]HRESULT ChangedSolenoids([out, retval] VARIANT* pRetVal);
				hres = get_ChangedSolenoids(&res);
			}
			break;
		}
		case 0x00000027: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1023: [id(0x00000027), propget]HRESULT ChangedGIStrings([out, retval] VARIANT* pRetVal);
				hres = get_ChangedGIStrings(&res);
			}
			break;
		}
		case 0x00000028: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1025: [id(0x00000028), propget]HRESULT ChangedLEDs([in] VARIANT mask2,[in] VARIANT mask1,[in, optional] VARIANT mask3,[in, optional] VARIANT mask4,[out, retval] VARIANT* pRetVal);
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
		case 0x00000029: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1032: [id(0x00000029), propget]HRESULT NewSoundCommands([out, retval] VARIANT* pRetVal);
				hres = get_NewSoundCommands(&res);
			}
			break;
		}
		case 0x0000002a: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1034: [id(0x0000002a), propget]HRESULT Lamp([in] VARIANT number,[out, retval] VARIANT_BOOL* pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				V_VT(&res) = VT_BOOL;
				hres = get_Lamp(var0, &V_BOOL(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 0x0000002b: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1038: [id(0x0000002b), propget]HRESULT Solenoid([in] VARIANT number,[out, retval] VARIANT_BOOL* pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				V_VT(&res) = VT_BOOL;
				hres = get_Solenoid(var0, &V_BOOL(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 0x0000002c: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1042: [id(0x0000002c), propget]HRESULT GIString([in] VARIANT number,[out, retval] VARIANT_BOOL* pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				V_VT(&res) = VT_BOOL;
				hres = get_GIString(var0, &V_BOOL(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 0x0000002d: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1046: [id(0x0000002d), propget]HRESULT Switch([in] VARIANT number,[out, retval] VARIANT_BOOL* pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				V_VT(&res) = VT_BOOL;
				hres = get_Switch(var0, &V_BOOL(&res));
				VariantClear(&var0);
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1050: [id(0x0000002d), propput]HRESULT Switch([in] VARIANT number,[in] VARIANT_BOOL pRetVal);
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
		case 0x0000002e: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1054: [id(0x0000002e), propget]HRESULT Mech([in] VARIANT number,[out, retval] long* pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				V_VT(&res) = VT_I4;
				hres = get_Mech(var0, (LONG*)&V_I4(&res));
				VariantClear(&var0);
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1058: [id(0x0000002e), propput]HRESULT Mech([in] VARIANT number,[in] long pRetVal);
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
		case 0x0000002f: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1062: [id(0x0000002f), propget]HRESULT GetMech([in] VARIANT number,[out, retval] VARIANT* pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = get_GetMech(var0, &res);
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000030: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1066: [id(0x00000030), propget]HRESULT Dip([in] VARIANT number,[out, retval] long* pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				V_VT(&res) = VT_I4;
				hres = get_Dip(var0, (LONG*)&V_I4(&res));
				VariantClear(&var0);
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1070: [id(0x00000030), propput]HRESULT Dip([in] VARIANT number,[in] long pRetVal);
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
		case 0x00000031: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1074: [id(0x00000031), propget]HRESULT SolMask([in] VARIANT number,[out, retval] long* pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				V_VT(&res) = VT_I4;
				hres = get_SolMask(var0, (LONG*)&V_I4(&res));
				VariantClear(&var0);
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1078: [id(0x00000031), propput]HRESULT SolMask([in] VARIANT number,[in] long pRetVal);
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
		case 0x00000032: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1082: [id(0x00000032), propget]HRESULT RawDmdWidth([out, retval] long* pRetVal);
				V_VT(&res) = VT_I4;
				hres = get_RawDmdWidth((LONG*)&V_I4(&res));
			}
			break;
		}
		case 0x00000033: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1084: [id(0x00000033), propget]HRESULT RawDmdHeight([out, retval] long* pRetVal);
				V_VT(&res) = VT_I4;
				hres = get_RawDmdHeight((LONG*)&V_I4(&res));
			}
			break;
		}
		case 0x00000034: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1086: [id(0x00000034), propget]HRESULT RawDmdPixels([out, retval] VARIANT* pRetVal);
				hres = get_RawDmdPixels(&res);
			}
			break;
		}
		case 0x00000035: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1088: [id(0x00000035), propget]HRESULT RawDmdColoredPixels([out, retval] VARIANT* pRetVal);
				hres = get_RawDmdColoredPixels(&res);
			}
			break;
		}
		case 0x00000036: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1090: [id(0x00000036), propget]HRESULT ChangedNVRAM([out, retval] VARIANT* pRetVal);
				hres = get_ChangedNVRAM(&res);
			}
			break;
		}
		case 0x00000037: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1092: [id(0x00000037), propget]HRESULT NVRAM([out, retval] VARIANT* pRetVal);
				hres = get_NVRAM(&res);
			}
			break;
		}
		case 0x00000038: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 1094: [id(0x00000038), propget]HRESULT SoundMode([out, retval] long* pRetVal);
				V_VT(&res) = VT_I4;
				hres = get_SoundMode((LONG*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 1096: [id(0x00000038), propput]HRESULT SoundMode([in] long pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_SoundMode(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000039: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1098: [id(0x00000039)]HRESULT B2SSetData([in] VARIANT idORname,[in] VARIANT value);
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
		case 0x0000003a: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1102: [id(0x0000003a)]HRESULT B2SPulseData([in] VARIANT idORname);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = B2SPulseData(var0);
				VariantClear(&var0);
			}
			break;
		}
		case 0x0000003b: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1104: [id(0x0000003b)]HRESULT B2SSetPos([in] VARIANT idORname,[in] VARIANT xpos,[in] VARIANT ypos);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantCopyInd(&var1, &pDispParams->rgvarg[--index]);
				VARIANT var2;
				V_VT(&var2) = VT_EMPTY;
				VariantCopyInd(&var2, &pDispParams->rgvarg[--index]);
				hres = B2SSetPos(var0, var1, var2);
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
			}
			break;
		}
		case 0x0000003c: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1109: [id(0x0000003c)]HRESULT B2SSetIllumination([in] VARIANT name,[in] VARIANT value);
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
		case 0x0000003d: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1113: [id(0x0000003d)]HRESULT B2SSetLED([in] VARIANT digit,[in] VARIANT valueORtext);
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
		case 0x0000003e: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1117: [id(0x0000003e)]HRESULT B2SSetLEDDisplay([in] VARIANT display,[in] VARIANT text);
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
		case 0x0000003f: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1121: [id(0x0000003f)]HRESULT B2SSetReel([in] VARIANT digit,[in] VARIANT value);
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
		case 0x00000040: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1125: [id(0x00000040)]HRESULT B2SSetScore([in] VARIANT display,[in] VARIANT value);
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
		case 0x00000041: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1129: [id(0x00000041)]HRESULT B2SSetScorePlayer([in] VARIANT playerno,[in] VARIANT score);
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
		case 0x00000042: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1133: [id(0x00000042)]HRESULT B2SSetScorePlayer1([in] VARIANT score);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = B2SSetScorePlayer1(var0);
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000043: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1135: [id(0x00000043)]HRESULT B2SSetScorePlayer2([in] VARIANT score);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = B2SSetScorePlayer2(var0);
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000044: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1137: [id(0x00000044)]HRESULT B2SSetScorePlayer3([in] VARIANT score);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = B2SSetScorePlayer3(var0);
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000045: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1139: [id(0x00000045)]HRESULT B2SSetScorePlayer4([in] VARIANT score);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = B2SSetScorePlayer4(var0);
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000046: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1141: [id(0x00000046)]HRESULT B2SSetScorePlayer5([in] VARIANT score);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = B2SSetScorePlayer5(var0);
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000047: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1143: [id(0x00000047)]HRESULT B2SSetScorePlayer6([in] VARIANT score);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = B2SSetScorePlayer6(var0);
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000048: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1145: [id(0x00000048)]HRESULT B2SSetScoreDigit([in] VARIANT digit,[in] VARIANT value);
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
		case 0x00000049: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1149: [id(0x00000049)]HRESULT B2SSetScoreRollover([in] VARIANT id,[in] VARIANT value);
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
		case 0x0000004a: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1153: [id(0x0000004a)]HRESULT B2SSetScoreRolloverPlayer1([in] VARIANT value);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = B2SSetScoreRolloverPlayer1(var0);
				VariantClear(&var0);
			}
			break;
		}
		case 0x0000004b: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1155: [id(0x0000004b)]HRESULT B2SSetScoreRolloverPlayer2([in] VARIANT value);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = B2SSetScoreRolloverPlayer2(var0);
				VariantClear(&var0);
			}
			break;
		}
		case 0x0000004c: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1157: [id(0x0000004c)]HRESULT B2SSetScoreRolloverPlayer3([in] VARIANT value);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = B2SSetScoreRolloverPlayer3(var0);
				VariantClear(&var0);
			}
			break;
		}
		case 0x0000004d: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1159: [id(0x0000004d)]HRESULT B2SSetScoreRolloverPlayer4([in] VARIANT value);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = B2SSetScoreRolloverPlayer4(var0);
				VariantClear(&var0);
			}
			break;
		}
		case 0x0000004e: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1161: [id(0x0000004e)]HRESULT B2SSetCredits([in] VARIANT digitORvalue,[in, optional] VARIANT value);
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
		case 0x0000004f: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1165: [id(0x0000004f)]HRESULT B2SSetPlayerUp([in] VARIANT idORvalue,[in, optional] VARIANT value);
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
		case 0x00000050: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1169: [id(0x00000050)]HRESULT B2SSetCanPlay([in] VARIANT idORvalue,[in, optional] VARIANT value);
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
		case 0x00000051: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1173: [id(0x00000051)]HRESULT B2SSetBallInPlay([in] VARIANT idORvalue,[in, optional] VARIANT value);
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
		case 0x00000052: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1177: [id(0x00000052)]HRESULT B2SSetTilt([in] VARIANT idORvalue,[in, optional] VARIANT value);
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
		case 0x00000053: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1181: [id(0x00000053)]HRESULT B2SSetMatch([in] VARIANT idORvalue,[in, optional] VARIANT value);
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
		case 0x00000054: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1185: [id(0x00000054)]HRESULT B2SSetGameOver([in] VARIANT idORvalue,[in, optional] VARIANT value);
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
		case 0x00000055: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1189: [id(0x00000055)]HRESULT B2SSetShootAgain([in] VARIANT idORvalue,[in, optional] VARIANT value);
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
		case 0x00000056: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1193: [id(0x00000056)]HRESULT B2SStartAnimation([in] BSTR animationname,[in, optional] VARIANT_BOOL playreverse);
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
		case 0x00000057: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1197: [id(0x00000057)]HRESULT B2SStartAnimationReverse([in] BSTR animationname);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = B2SStartAnimationReverse(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000058: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1199: [id(0x00000058)]HRESULT B2SStopAnimation([in] BSTR animationname);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = B2SStopAnimation(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000059: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1201: [id(0x00000059)]HRESULT B2SStopAllAnimations();
				hres = B2SStopAllAnimations();
			}
			break;
		}
		case 0x0000005a: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1203: [id(0x0000005a)]HRESULT B2SIsAnimationRunning([in] BSTR animationname,[out, retval] VARIANT_BOOL* pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				V_VT(&res) = VT_BOOL;
				hres = B2SIsAnimationRunning(V_BSTR(&var0), &V_BOOL(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 0x0000005b: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1207: [id(0x0000005b)]HRESULT StartAnimation([in] BSTR animationname,[in, optional] VARIANT_BOOL playreverse);
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
		case 0x0000005c: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1211: [id(0x0000005c)]HRESULT StopAnimation([in] BSTR animationname);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = StopAnimation(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x0000005d: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1213: [id(0x0000005d)]HRESULT B2SStartRotation();
				hres = B2SStartRotation();
			}
			break;
		}
		case 0x0000005e: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1215: [id(0x0000005e)]HRESULT B2SStopRotation();
				hres = B2SStopRotation();
			}
			break;
		}
		case 0x0000005f: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1217: [id(0x0000005f)]HRESULT B2SShowScoreDisplays();
				hres = B2SShowScoreDisplays();
			}
			break;
		}
		case 0x00000060: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1219: [id(0x00000060)]HRESULT B2SHideScoreDisplays();
				hres = B2SHideScoreDisplays();
			}
			break;
		}
		case 0x00000061: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1221: [id(0x00000061)]HRESULT B2SStartSound([in] BSTR soundname);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = B2SStartSound(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000062: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1223: [id(0x00000062)]HRESULT B2SPlaySound([in] BSTR soundname);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = B2SPlaySound(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000063: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1225: [id(0x00000063)]HRESULT B2SStopSound([in] BSTR soundname);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = B2SStopSound(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000064: {
			if (wFlags & DISPATCH_METHOD) {
				// line 1227: [id(0x00000064)]HRESULT B2SMapSound([in] VARIANT digit,[in] BSTR soundname);
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
		default:
		break;
	}
	if (SUCCEEDED(hres)) {
		if (pVarResult)
			*pVarResult = res;
		else
			VariantClear(&res);
	}
	else {
		PLOGI.printf("dispId=%d (0x%08x), wFlags=%d, hres=%d", dispIdMember, dispIdMember, wFlags, hres);
	}
	return hres;
}

