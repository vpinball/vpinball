#include "stdafx.h"
#include "olectl.h"

robin_hood::unordered_map<wstring, int> VPinMAMEController::m_nameIDMap = {
	robin_hood::pair<wstring, int> { wstring(L"run"), 1 },
	robin_hood::pair<wstring, int> { wstring(L"stop"), 2 },
	robin_hood::pair<wstring, int> { wstring(L"lamp"), 5 },
	robin_hood::pair<wstring, int> { wstring(L"solenoid"), 6 },
	robin_hood::pair<wstring, int> { wstring(L"switch"), 7 },
	robin_hood::pair<wstring, int> { wstring(L"wpcnumbering"), 12 },
	robin_hood::pair<wstring, int> { wstring(L"lamps"), 13 },
	robin_hood::pair<wstring, int> { wstring(L"changedlamps"), 14 },
	robin_hood::pair<wstring, int> { wstring(L"switches"), 15 },
	robin_hood::pair<wstring, int> { wstring(L"changedsolenoids"), 16 },
	robin_hood::pair<wstring, int> { wstring(L"gamename"), 17 },
	robin_hood::pair<wstring, int> { wstring(L"handlekeyboard"), 19 },
	robin_hood::pair<wstring, int> { wstring(L"pause"), 22 },
	robin_hood::pair<wstring, int> { wstring(L"machines"), 23 },
	robin_hood::pair<wstring, int> { wstring(L"running"), 24 },
	robin_hood::pair<wstring, int> { wstring(L"handlemechanics"), 26 },
	robin_hood::pair<wstring, int> { wstring(L"gistring"), 27 },
	robin_hood::pair<wstring, int> { wstring(L"changedgistrings"), 28 },
	robin_hood::pair<wstring, int> { wstring(L"splashinfoline"), 30 },
	robin_hood::pair<wstring, int> { wstring(L"solenoids"), 31 },
	robin_hood::pair<wstring, int> { wstring(L"dip"), 32 },
	robin_hood::pair<wstring, int> { wstring(L"gistrings"), 33 },
	robin_hood::pair<wstring, int> { wstring(L"getmech"), 34 },
	robin_hood::pair<wstring, int> { wstring(L"lockdisplay"), 43 },
	robin_hood::pair<wstring, int> { wstring(L"solmask"), 45 },
	robin_hood::pair<wstring, int> { wstring(L"version"), 48 },
	robin_hood::pair<wstring, int> { wstring(L"mech"), 49 },
	robin_hood::pair<wstring, int> { wstring(L"games"), 51 },
	robin_hood::pair<wstring, int> { wstring(L"settings"), 52 },
	robin_hood::pair<wstring, int> { wstring(L"changedleds"), 53 },
	robin_hood::pair<wstring, int> { wstring(L"hidden"), 54 },
	robin_hood::pair<wstring, int> { wstring(L"game"), 55 },
	robin_hood::pair<wstring, int> { wstring(L"mechsamples"), 56 },
	robin_hood::pair<wstring, int> { wstring(L"getwindowrect"), 57 },
	robin_hood::pair<wstring, int> { wstring(L"getclientrect"), 58 },
	robin_hood::pair<wstring, int> { wstring(L"rawdmdwidth"), 78 },
	robin_hood::pair<wstring, int> { wstring(L"rawdmdheight"), 79 },
	robin_hood::pair<wstring, int> { wstring(L"rawdmdpixels"), 80 },
	robin_hood::pair<wstring, int> { wstring(L"dmdwidth"), 59 },
	robin_hood::pair<wstring, int> { wstring(L"dmdheight"), 60 },
	robin_hood::pair<wstring, int> { wstring(L"dmdpixel"), 61 },
	robin_hood::pair<wstring, int> { wstring(L"updatedmdpixels"), 62 },
	robin_hood::pair<wstring, int> { wstring(L"changedlampsstate"), 63 },
	robin_hood::pair<wstring, int> { wstring(L"lampsstate"), 64 },
	robin_hood::pair<wstring, int> { wstring(L"changedsolenoidsstate"), 65 },
	robin_hood::pair<wstring, int> { wstring(L"solenoidsstate"), 66 },
	robin_hood::pair<wstring, int> { wstring(L"changedgisstate"), 67 },
	robin_hood::pair<wstring, int> { wstring(L"mastervolume"), 68 },
	robin_hood::pair<wstring, int> { wstring(L"enumaudiodevices"), 69 },
	robin_hood::pair<wstring, int> { wstring(L"audiodevicescount"), 70 },
	robin_hood::pair<wstring, int> { wstring(L"audiodevicedescription"), 71 },
	robin_hood::pair<wstring, int> { wstring(L"audiodevicemodule"), 72 },
	robin_hood::pair<wstring, int> { wstring(L"currentaudiodevice"), 73 },
	robin_hood::pair<wstring, int> { wstring(L"fastframes"), 74 },
	robin_hood::pair<wstring, int> { wstring(L"ignoreromcrc"), 75 },
	robin_hood::pair<wstring, int> { wstring(L"cabinetmode"), 76 },
	robin_hood::pair<wstring, int> { wstring(L"showpindmd"), 77 },
	robin_hood::pair<wstring, int> { wstring(L"showwindmd"), 81 },
	robin_hood::pair<wstring, int> { wstring(L"changedledsstate"), 82 },
	robin_hood::pair<wstring, int> { wstring(L"rawdmdcoloredpixels"), 83 },
	robin_hood::pair<wstring, int> { wstring(L"nvram"), 84 },
	robin_hood::pair<wstring, int> { wstring(L"changednvram"), 85 },
	robin_hood::pair<wstring, int> { wstring(L"soundmode"), 86 },
	robin_hood::pair<wstring, int> { wstring(L"romname"), 87 }
};

STDMETHODIMP VPinMAMEController::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	wstring name = wstring(*rgszNames);
	std::transform(name.begin(), name.end(), name.begin(), tolower);
	const robin_hood::unordered_map<wstring, int>::iterator it = m_nameIDMap.find(name);
	if (it != m_nameIDMap.end()) {
		*rgDispId = it->second;
		return S_OK;
	}
	return DISP_E_UNKNOWNNAME;
}

STDMETHODIMP VPinMAMEController::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
	switch(dispIdMember) {
		/*case 1: {
			// line 161: [id(1), helpstring("method Run")] HRESULT Run([in,defaultvalue(0)] LONG_PTR hParentWnd, [in,defaultvalue(99)] int nMinVersion);
			int index = pDispParams->cArgs;
			CComVariant var1(99);
			VariantChangeType(&var1, (index > 0) ? &pDispParams->rgvarg[--index] : &var1, 0, VT_I4);
			return Run(LONG_PTR, V_I4(&var1));
		}

		case 2: {
			// line 162: [id(2), helpstring("method Stop")] HRESULT Stop();
			return Stop();
		}

		case 5: {
			// line 165: [propget, id(5), helpstring("property Lamp")] HRESULT Lamp([in] int nLamp, [out, retval] VARIANT_BOOL *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			CComVariant var0;
			VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
			V_VT(pVarResult) = VT_BOOL;
			return get_Lamp(V_I4(&var0), &V_BOOL(pVarResult));
		}

		case 6: {
			// line 166: [propget, id(6), helpstring("property Solenoid")] HRESULT Solenoid([in] int nSolenoid, [out, retval] VARIANT_BOOL *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			CComVariant var0;
			VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
			V_VT(pVarResult) = VT_BOOL;
			return get_Solenoid(V_I4(&var0), &V_BOOL(pVarResult));
		}

		case 7:
		switch(pDispParams->cArgs) {
			case 1: {
				// line 167: [propget, id(7), helpstring("property Switch")] HRESULT Switch([in] int nSwitchNo, [out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				V_VT(pVarResult) = VT_BOOL;
				return get_Switch(V_I4(&var0), &V_BOOL(pVarResult));
			}

			case 2: {
				// line 168: [propput, id(7), helpstring("property Switch")] HRESULT Switch([in] int nSwitchNo, [in] VARIANT_BOOL newVal);
				int index = pDispParams->cArgs;
				CComVariant var0;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				CComVariant var1;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				return put_Switch(V_I4(&var0), V_BOOL(&var1));
			}

			default:
			break;
		}
		break;

		case 12: {
			// line 169: [propget, id(12), helpstring("property WPCNumbering")] HRESULT WPCNumbering([out, retval] VARIANT_BOOL *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_BOOL;
			return get_WPCNumbering(&V_BOOL(pVarResult));
		}

		case 13: {
			// line 170: [propget, id(13), helpstring("property Lamps")] HRESULT Lamps([out, retval] VARIANT *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			return get_Lamps(pVarResult);
		}

		case 14: {
			// line 171: [propget, id(14), helpstring("property ChangedLamps")] HRESULT ChangedLamps([out, retval] VARIANT *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			return get_ChangedLamps(pVarResult);
		}

		case 15:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 172: [propget, id(15), helpstring("property Switches")] HRESULT Switches([out, retval] VARIANT *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				return get_Switches(pVarResult);
			}

			case 1: {
				// line 173: [propput, id(15), helpstring("property Switches")] HRESULT Switches([in] VARIANT newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_VARIANT);
				return put_Switches(var0);
			}

			default:
			break;
		}
		break;

		case 16: {
			// line 174: [propget, id(16), helpstring("property ChangedSolenoids")] HRESULT ChangedSolenoids([out, retval] VARIANT *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			return get_ChangedSolenoids(pVarResult);
		}

		case 17:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 175: [propget, id(17), helpstring("property GameName")] HRESULT GameName([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_GameName(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 176: [propput, id(17), helpstring("property GameName")] HRESULT GameName([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_GameName(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case 19:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 177: [propget, id(19), helpstring("property HandleKeyboard")] HRESULT HandleKeyboard([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_HandleKeyboard(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 178: [propput, id(19), helpstring("property HandleKeyboard")] HRESULT HandleKeyboard([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_HandleKeyboard(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 22:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 181: [propget, id(22), helpstring("property Pause")] HRESULT Pause([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_Pause(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 182: [propput, id(22), helpstring("property Pause")] HRESULT Pause([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_Pause(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 23: {
			// line 183: [propget, id(23), helpstring("property Machines")] HRESULT Machines([in,defaultvalue("")] BSTR sMachine, [out, retval] VARIANT *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			int index = pDispParams->cArgs;
			CComVariant var0("");
			VariantChangeType(&var0, (index > 0) ? &pDispParams->rgvarg[--index] : &var0, 0, VT_BSTR);
			return get_Machines(V_BSTR(&var0), pVarResult);
		}

		case 24: {
			// line 184: [propget, id(24), helpstring("property Running")] HRESULT Running([out, retval] VARIANT_BOOL *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_BOOL;
			return get_Running(&V_BOOL(pVarResult));
		}

		case 26:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 186: [propget, id(26), helpstring("property HandleMechanics")] HRESULT HandleMechanics([out, retval] int *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_HandleMechanics(&V_I4(pVarResult));
			}

			case 1: {
				// line 187: [propput, id(26), helpstring("property HandleMechanics")] HRESULT HandleMechanics([in] int newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_HandleMechanics(V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case 27: {
			// line 188: [propget, id(27), helpstring("property GIString")] HRESULT GIString(int nString, [out, retval] int *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			CComVariant var0;
			VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
			V_VT(pVarResult) = VT_I4;
			return get_GIString(V_I4(&var0), &V_I4(pVarResult));
		}

		case 28: {
			// line 189: [propget, id(28), helpstring("property ChangedGIStrings")] HRESULT ChangedGIStrings([out, retval] VARIANT *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			return get_ChangedGIStrings(pVarResult);
		}

		case 30:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 192: [propget, id(30), helpstring("property SplashInfoLine")] HRESULT SplashInfoLine([out, retval] BSTR *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BSTR;
				return get_SplashInfoLine(&V_BSTR(pVarResult));
			}

			case 1: {
				// line 193: [propput, id(30), helpstring("property SplashInfoLine")] HRESULT SplashInfoLine([in] BSTR newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BSTR);
				return put_SplashInfoLine(V_BSTR(&var0));
			}

			default:
			break;
		}
		break;

		case 31: {
			// line 194: [propget, id(31), helpstring("property Solenoids")] HRESULT Solenoids([out, retval] VARIANT *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			return get_Solenoids(pVarResult);
		}

		case 32:
		switch(pDispParams->cArgs) {
			case 1: {
				// line 195: [propget, id(32), helpstring("property Dip")] HRESULT Dip([in] int nNo, [out, retval] int *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				V_VT(pVarResult) = VT_I4;
				return get_Dip(V_I4(&var0), &V_I4(pVarResult));
			}

			case 2: {
				// line 196: [propput, id(32), helpstring("property Dip")] HRESULT Dip([in] int nNo, [in] int newVal);
				int index = pDispParams->cArgs;
				CComVariant var0;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				CComVariant var1;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_I4);
				return put_Dip(V_I4(&var0), V_I4(&var1));
			}

			default:
			break;
		}
		break;

		case 33: {
			// line 197: [propget, id(33), helpstring("property GIStrings")] HRESULT GIStrings([out, retval] VARIANT *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			return get_GIStrings(pVarResult);
		}

		case 34: {
			// line 198: [propget, id(34), helpstring("property GetMech")] HRESULT GetMech([in] int mechNo, [out, retval] int *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			CComVariant var0;
			VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
			V_VT(pVarResult) = VT_I4;
			return get_GetMech(V_I4(&var0), &V_I4(pVarResult));
		}

		case 43:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 204: [propget, id(43), helpstring("property LockDisplay")] HRESULT LockDisplay([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_LockDisplay(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 205: [propput, id(43), helpstring("property LockDisplay")] HRESULT LockDisplay([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_LockDisplay(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 45:
		switch(pDispParams->cArgs) {
			case 1: {
				// line 206: [propget, id(45), helpstring("property SolMask")] HRESULT SolMask([in] int nLow, [out, retval] long *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				V_VT(pVarResult) = VT_I4;
				return get_SolMask(V_I4(&var0), (long*)&V_I4(pVarResult));
			}

			case 2: {
				// line 207: [propput, id(45), helpstring("property SolMask")] HRESULT SolMask([in] int nLow, [in] long newVal);
				int index = pDispParams->cArgs;
				CComVariant var0;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				CComVariant var1;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_I4);
				return put_SolMask(V_I4(&var0), V_I4(&var1));
			}

			default:
			break;
		}
		break;

		case 48: {
			// line 209: [propget, id(48), helpstring("property Version")] HRESULT Version([out, retval] BSTR *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_BSTR;
			return get_Version(&V_BSTR(pVarResult));
		}

		case 49: {
			// line 210: [propput, id(49), helpstring("property Mech")] HRESULT Mech([in] int param, [in] int newVal);
			int index = pDispParams->cArgs;
			CComVariant var0;
			VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
			CComVariant var1;
			VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_I4);
			return put_Mech(V_I4(&var0), V_I4(&var1));
		}

		case 51: {
			// line 212: [propget, id(51), helpstring("property Games")] HRESULT Games([out, retval] IGames* *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			return get_Games(IGames**);
		}

		case 52: {
			// line 213: [propget, id(52), helpstring("property Settings")] HRESULT Settings([out, retval] IControllerSettings * *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			return get_Settings(IControllerSettings);
		}

		case 53: {
			// line 214: [propget, id(53), helpstring("property ChangedLEDs")] HRESULT ChangedLEDs([in] int nHigh, [in] int nLow, [in,defaultvalue(0)] int nnHigh, [in,defaultvalue(0)] int nnLow, [out, retval] VARIANT *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			int index = pDispParams->cArgs;
			CComVariant var0;
			VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
			CComVariant var1;
			VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_I4);
			CComVariant var2(0);
			VariantChangeType(&var2, (index > 0) ? &pDispParams->rgvarg[--index] : &var2, 0, VT_I4);
			CComVariant var3(0);
			VariantChangeType(&var3, (index > 0) ? &pDispParams->rgvarg[--index] : &var3, 0, VT_I4);
			return get_ChangedLEDs(V_I4(&var0), V_I4(&var1), V_I4(&var2), V_I4(&var3), pVarResult);
		}

		case 54:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 215: [propget, id(54), helpstring("property Hidden")] HRESULT Hidden([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_Hidden(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 216: [propput, id(54), helpstring("property Hidden")] HRESULT Hidden([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_Hidden(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 55: {
			// line 217: [propget, id(55), helpstring("property Game")] HRESULT Game([out, retval] IGame * *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			return get_Game(IGame);
		}

		case 56:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 218: [propget, id(56), helpstring("property MechSamples")] HRESULT MechSamples([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_MechSamples(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 219: [propput, id(56), helpstring("property MechSamples")] HRESULT MechSamples([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_MechSamples(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 57: {
			// line 220: [id(57), helpstring("method GetWindowRect")] HRESULT GetWindowRect([in,defaultvalue(0)] LONG_PTR hWnd, [out, retval] VARIANT *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			int index = pDispParams->cArgs;
			return GetWindowRect(LONG_PTR, pVarResult);
		}

		case 58: {
			// line 221: [id(58), helpstring("method GetClientRect")] HRESULT GetClientRect([in,defaultvalue(0)] LONG_PTR hWnd, [out, retval] VARIANT *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			int index = pDispParams->cArgs;
			return GetClientRect(LONG_PTR, pVarResult);
		}

		case 78: {
			// line 222: [propget, id(78), helpstring("property RawDmdWidth")] HRESULT RawDmdWidth([out, retval] int *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_I4;
			return get_RawDmdWidth(&V_I4(pVarResult));
		}

		case 79: {
			// line 223: [propget, id(79), helpstring("property RawDmdHeight")] HRESULT RawDmdHeight([out, retval] int *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_I4;
			return get_RawDmdHeight(&V_I4(pVarResult));
		}

		case 80: {
			// line 224: [propget, id(80), helpstring("property RawDmdPixels")] HRESULT RawDmdPixels([out, retval] VARIANT *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			return get_RawDmdPixels(pVarResult);
		}

		case 59: {
			// line 225: [propget, id(59), helpstring("property DmdWidth")] HRESULT DmdWidth([out, retval] int *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_I4;
			return get_DmdWidth(&V_I4(pVarResult));
		}

		case 60: {
			// line 226: [propget, id(60), helpstring("property DmdHeight")] HRESULT DmdHeight([out, retval] int *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_I4;
			return get_DmdHeight(&V_I4(pVarResult));
		}

		case 61: {
			// line 227: [propget, id(61), helpstring("property DmdPixel")] HRESULT DmdPixel([in] int x, [in] int y, [out, retval] int *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			int index = pDispParams->cArgs;
			CComVariant var0;
			VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
			CComVariant var1;
			VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_I4);
			V_VT(pVarResult) = VT_I4;
			return get_DmdPixel(V_I4(&var0), V_I4(&var1), &V_I4(pVarResult));
		}

		case 62: {
			// line 228: [propget, id(62), helpstring("property updateDmdPixels")] HRESULT updateDmdPixels([in] int **buf, [in] int width, [in] int height, [out, retval] int *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			int index = pDispParams->cArgs;
			CComVariant var1;
			VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_I4);
			CComVariant var2;
			VariantChangeType(&var2, &pDispParams->rgvarg[--index], 0, VT_I4);
			V_VT(pVarResult) = VT_I4;
			return get_updateDmdPixels(int**, V_I4(&var1), V_I4(&var2), &V_I4(pVarResult));
		}

		case 63: {
			// line 229: [propget, id(63), helpstring("property ChangedLampsState")] HRESULT ChangedLampsState([in] int **buf, [out, retval] int *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_I4;
			return get_ChangedLampsState(int**, &V_I4(pVarResult));
		}

		case 64: {
			// line 230: [propget, id(64), helpstring("property LampsState")] HRESULT LampsState([in] int **buf, [out, retval] int *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_I4;
			return get_LampsState(int**, &V_I4(pVarResult));
		}

		case 65: {
			// line 231: [propget, id(65), helpstring("property ChangedSolenoidsState")] HRESULT ChangedSolenoidsState([in] int **buf, [out, retval] int *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_I4;
			return get_ChangedSolenoidsState(int**, &V_I4(pVarResult));
		}

		case 66: {
			// line 232: [propget, id(66), helpstring("property SolenoidsState")] HRESULT SolenoidsState([in] int **buf, [out, retval] int *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_I4;
			return get_SolenoidsState(int**, &V_I4(pVarResult));
		}

		case 67: {
			// line 233: [propget, id(67), helpstring("property ChangedGIsState")] HRESULT ChangedGIsState([in] int **buf, [out, retval] int *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_I4;
			return get_ChangedGIsState(int**, &V_I4(pVarResult));
		}

		case 68:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 234: [propget, id(68), helpstring("property MasterVolume")] HRESULT MasterVolume([out, retval] int *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_MasterVolume(&V_I4(pVarResult));
			}

			case 1: {
				// line 235: [propput, id(68), helpstring("property MasterVolume")] HRESULT MasterVolume([in] int newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_MasterVolume(V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case 69: {
			// line 236: [propget, id(69), helpstring("property EnumAudioDevices")] HRESULT EnumAudioDevices([out, retval] int *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_I4;
			return get_EnumAudioDevices(&V_I4(pVarResult));
		}

		case 70: {
			// line 237: [propget, id(70), helpstring("property AudioDevicesCount")] HRESULT AudioDevicesCount([out, retval] int *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_I4;
			return get_AudioDevicesCount(&V_I4(pVarResult));
		}

		case 71: {
			// line 238: [propget, id(71), helpstring("property AudioDeviceDescription")] HRESULT AudioDeviceDescription([in] int num,[out, retval] BSTR *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			CComVariant var0;
			VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
			V_VT(pVarResult) = VT_BSTR;
			return get_AudioDeviceDescription(V_I4(&var0), &V_BSTR(pVarResult));
		}

		case 72: {
			// line 239: [propget, id(72), helpstring("property AudioDeviceModule")] HRESULT AudioDeviceModule([in] int num,[out, retval] BSTR *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			CComVariant var0;
			VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
			V_VT(pVarResult) = VT_BSTR;
			return get_AudioDeviceModule(V_I4(&var0), &V_BSTR(pVarResult));
		}

		case 73:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 240: [propget, id(73), helpstring("property CurrentAudioDevice")] HRESULT CurrentAudioDevice([out, retval] int *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_CurrentAudioDevice(&V_I4(pVarResult));
			}

			case 1: {
				// line 241: [propput, id(73), helpstring("property CurrentAudioDevice")] HRESULT CurrentAudioDevice([in] int newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_CurrentAudioDevice(V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case 74:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 242: [propget, id(74), helpstring("property FastFrames")] HRESULT FastFrames([out, retval] int *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_FastFrames(&V_I4(pVarResult));
			}

			case 1: {
				// line 243: [propput, id(74), helpstring("property FastFrames")] HRESULT FastFrames([in] int newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_FastFrames(V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case 75:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 244: [propget, id(75), helpstring("property IgnoreRomCrc")] HRESULT IgnoreRomCrc([out, retval] VARIANT_BOOL  *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				return get_IgnoreRomCrc(VARIANT_BOOL);
			}

			case 1: {
				// line 245: [propput, id(75), helpstring("property IgnoreRomCrc")] HRESULT IgnoreRomCrc([in] VARIANT_BOOL  newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_IgnoreRomCrc(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 76:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 246: [propget, id(76), helpstring("property CabinetMode")] HRESULT CabinetMode([out, retval] VARIANT_BOOL  *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				return get_CabinetMode(VARIANT_BOOL);
			}

			case 1: {
				// line 247: [propput, id(76), helpstring("property CabinetMode")] HRESULT CabinetMode([in] VARIANT_BOOL  newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_CabinetMode(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 77:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 248: [propget, id(77), helpstring("property ShowPinDMD")] HRESULT ShowPinDMD([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_ShowPinDMD(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 249: [propput, id(77), helpstring("property ShowPinDMD")] HRESULT ShowPinDMD([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_ShowPinDMD(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 81:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 250: [propget, id(81), helpstring("property ShowWinDMD")] HRESULT ShowWinDMD([out, retval] VARIANT_BOOL *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_BOOL;
				return get_ShowWinDMD(&V_BOOL(pVarResult));
			}

			case 1: {
				// line 251: [propput, id(81), helpstring("property ShowWinDMD")] HRESULT ShowWinDMD([in] VARIANT_BOOL newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_BOOL);
				return put_ShowWinDMD(V_BOOL(&var0));
			}

			default:
			break;
		}
		break;

		case 82: {
			// line 252: [propget, id(82), helpstring("property ChangedLEDsState")] HRESULT ChangedLEDsState([in] int nHigh, [in] int nLow, [in] int nnHigh, [in] int nnLow, [in] int **buf, [out, retval] int *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			int index = pDispParams->cArgs;
			CComVariant var0;
			VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
			CComVariant var1;
			VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_I4);
			CComVariant var2;
			VariantChangeType(&var2, &pDispParams->rgvarg[--index], 0, VT_I4);
			CComVariant var3;
			VariantChangeType(&var3, &pDispParams->rgvarg[--index], 0, VT_I4);
			V_VT(pVarResult) = VT_I4;
			return get_ChangedLEDsState(V_I4(&var0), V_I4(&var1), V_I4(&var2), V_I4(&var3), int**, &V_I4(pVarResult));
		}

		case 83: {
			// line 253: [propget, id(83), helpstring("property RawDmdColoredPixels")] HRESULT RawDmdColoredPixels([out, retval] VARIANT *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			return get_RawDmdColoredPixels(pVarResult);
		}

		case 84: {
			// line 254: [propget, id(84), helpstring("property NVRAM")] HRESULT NVRAM([out, retval] VARIANT *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			return get_NVRAM(pVarResult);
		}

		case 85: {
			// line 255: [propget, id(85), helpstring("property ChangedNVRAM")] HRESULT ChangedNVRAM([out, retval] VARIANT *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			return get_ChangedNVRAM(pVarResult);
		}

		case 86:
		switch(pDispParams->cArgs) {
			case 0: {
				// line 256: [propget, id(86), helpstring("property SoundMode")] HRESULT SoundMode([out, retval] int *pVal);
				if (pVarResult == NULL) {
					VARIANT valResult;
					VariantInit(&valResult);
					pVarResult = &valResult;
				}
				V_VT(pVarResult) = VT_I4;
				return get_SoundMode(&V_I4(pVarResult));
			}

			case 1: {
				// line 257: [propput, id(86), helpstring("property SoundMode")] HRESULT SoundMode([in] int newVal);
				CComVariant var0;
				VariantChangeType(&var0, pDispParams->rgvarg, 0, VT_I4);
				return put_SoundMode(V_I4(&var0));
			}

			default:
			break;
		}
		break;

		case 87: {
			// line 258: [propget, id(87), helpstring("property ROMName")] HRESULT ROMName([out, retval] BSTR *pVal);
			if (pVarResult == NULL) {
				VARIANT valResult;
				VariantInit(&valResult);
				pVarResult = &valResult;
			}
			V_VT(pVarResult) = VT_BSTR;
			return get_ROMName(&V_BSTR(pVarResult));
		}

		default:
		break;
		*/
	}

	return DISP_E_UNKNOWNNAME;
}

