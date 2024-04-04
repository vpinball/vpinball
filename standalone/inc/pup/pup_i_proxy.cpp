#include "stdafx.h"
#include "olectl.h"

#include "PUPPinDisplay.h"

STDMETHODIMP PUPPinDisplay::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} namesIdsList[] = {
			{ NULL },
			{ L"B2SData", 0x000000e0 },
			{ L"B2SDisplays", 0x000000f1 },
			{ L"B2SFilter", 0x000000e1 },
			{ L"B2SInit", 0x000000e4 },
			{ L"BlockPlay", 0x000000dc },
			{ L"CloseApp", 0x000000d8 },
			{ L"GameUpdate", 0x000000f3 },
			{ L"GetGame", 0x000000eb },
			{ L"GetRoot", 0x000000ec },
			{ L"GetVersion", 0x000000f5 },
			{ L"GrabDC", 0x000000f4 },
			{ L"GrabDC2", 0x000000f6 },
			{ L"Hide", 0x000000e3 },
			{ L"Init", 0x000000c9 },
			{ L"InitPuPMenu", 0x000000f0 },
			{ L"isPlaying", 0x000000d7 },
			{ L"LabelInit", 0x000000ea },
			{ L"LabelNew", 0x000000e6 },
			{ L"LabelSet", 0x000000e7 },
			{ L"LabelSetEx", 0x000000e8 },
			{ L"LabelShowPage", 0x000000e9 },
			{ L"play", 0x000000cd },
			{ L"playevent", 0x000000f7 },
			{ L"playlistadd", 0x000000ca },
			{ L"playlistplay", 0x000000cb },
			{ L"playlistplayex", 0x000000cc },
			{ L"playpause", 0x000000d4 },
			{ L"playresume", 0x000000d5 },
			{ L"playstop", 0x000000d6 },
			{ L"PuPClose", 0x000000f9 },
			{ L"PuPSound", 0x000000ef },
			{ L"SendMSG", 0x000000e5 },
			{ L"setAspect", 0x000000d2 },
			{ L"SetBackGround", 0x000000db },
			{ L"setHeight", 0x000000cf },
			{ L"SetLength", 0x000000d9 },
			{ L"SetLoop", 0x000000da },
			{ L"SetPosVideo", 0x000000f8 },
			{ L"setPosX", 0x000000d0 },
			{ L"setPosY", 0x000000d1 },
			{ L"SetScreen", 0x000000dd },
			{ L"SetScreenEx", 0x000000de },
			{ L"setVolume", 0x000000d3 },
			{ L"setVolumeCurrent", 0x000000f2 },
			{ L"setWidth", 0x000000ce },
			{ L"Show", 0x000000e2 },
			{ L"SN", 0x000000df },
			{ L"SoundAdd", 0x000000ed },
			{ L"SoundPlay", 0x000000ee }
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

STDMETHODIMP PUPPinDisplay::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
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
		case 0x000000c9: {
			if (wFlags & DISPATCH_METHOD) {
				// line 30: [id(0x000000c9)]HRESULT Init([in] long ScreenNum,[in] BSTR RootDir);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = Init(V_I4(&var0), V_BSTR(&var1));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x000000ca: {
			if (wFlags & DISPATCH_METHOD) {
				// line 34: [id(0x000000ca)]HRESULT playlistadd([in] long ScreenNum,[in] BSTR folder,[in] long sort,[in] long restSeconds);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var2;
				V_VT(&var2) = VT_EMPTY;
				VariantChangeType(&var2, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var3;
				V_VT(&var3) = VT_EMPTY;
				VariantChangeType(&var3, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = playlistadd(V_I4(&var0), V_BSTR(&var1), V_I4(&var2), V_I4(&var3));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
				VariantClear(&var3);
			}
			break;
		}
		case 0x000000cb: {
			if (wFlags & DISPATCH_METHOD) {
				// line 40: [id(0x000000cb)]HRESULT playlistplay([in] long ScreenNum,[in] BSTR playlist);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = playlistplay(V_I4(&var0), V_BSTR(&var1));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x000000cc: {
			if (wFlags & DISPATCH_METHOD) {
				// line 44: [id(0x000000cc)]HRESULT playlistplayex([in] long ScreenNum,[in] BSTR playlist,[in] BSTR playfilename,[in] long volume,[in] long forceplay);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
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
				hres = playlistplayex(V_I4(&var0), V_BSTR(&var1), V_BSTR(&var2), V_I4(&var3), V_I4(&var4));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
				VariantClear(&var3);
				VariantClear(&var4);
			}
			break;
		}
		case 0x000000cd: {
			if (wFlags & DISPATCH_METHOD) {
				// line 51: [id(0x000000cd)]HRESULT play([in] long ScreenNum,[in] BSTR playlist,[in] BSTR playfilename);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var2;
				V_VT(&var2) = VT_EMPTY;
				VariantChangeType(&var2, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = play(V_I4(&var0), V_BSTR(&var1), V_BSTR(&var2));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
			}
			break;
		}
		case 0x000000ce: {
			if (wFlags & DISPATCH_METHOD) {
				// line 56: [id(0x000000ce)]HRESULT setWidth([in] long ScreenNum,[in] long width);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = setWidth(V_I4(&var0), V_I4(&var1));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x000000cf: {
			if (wFlags & DISPATCH_METHOD) {
				// line 60: [id(0x000000cf)]HRESULT setHeight([in] long ScreenNum,[in] long Height);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = setHeight(V_I4(&var0), V_I4(&var1));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x000000d0: {
			if (wFlags & DISPATCH_METHOD) {
				// line 64: [id(0x000000d0)]HRESULT setPosX([in] long ScreenNum,[in] long Posx);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = setPosX(V_I4(&var0), V_I4(&var1));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x000000d1: {
			if (wFlags & DISPATCH_METHOD) {
				// line 68: [id(0x000000d1)]HRESULT setPosY([in] long ScreenNum,[in] long PosY);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = setPosY(V_I4(&var0), V_I4(&var1));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x000000d2: {
			if (wFlags & DISPATCH_METHOD) {
				// line 72: [id(0x000000d2)]HRESULT setAspect([in] long ScreenNum,[in] long aspectWide,[in] long aspectHigh);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var2;
				V_VT(&var2) = VT_EMPTY;
				VariantChangeType(&var2, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = setAspect(V_I4(&var0), V_I4(&var1), V_I4(&var2));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
			}
			break;
		}
		case 0x000000d3: {
			if (wFlags & DISPATCH_METHOD) {
				// line 77: [id(0x000000d3)]HRESULT setVolume([in] long ScreenNum,[in] long vol);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = setVolume(V_I4(&var0), V_I4(&var1));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x000000d4: {
			if (wFlags & DISPATCH_METHOD) {
				// line 81: [id(0x000000d4)]HRESULT playpause([in] long ScreenNum);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = playpause(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x000000d5: {
			if (wFlags & DISPATCH_METHOD) {
				// line 83: [id(0x000000d5)]HRESULT playresume([in] long ScreenNum);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = playresume(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x000000d6: {
			if (wFlags & DISPATCH_METHOD) {
				// line 85: [id(0x000000d6)]HRESULT playstop([in] long ScreenNum);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = playstop(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x000000d8: {
			if (wFlags & DISPATCH_METHOD) {
				// line 87: [id(0x000000d8)]HRESULT CloseApp();
				hres = CloseApp();
			}
			break;
		}
		case 0x000000d7: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 89: [id(0x000000d7), propget]HRESULT isPlaying([in] long ScreenNum,[out, retval] long* Value);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				V_VT(&res) = VT_I4;
				hres = get_isPlaying(V_I4(&var0), (LONG*)&V_I4(&res));
				VariantClear(&var0);
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 93: [id(0x000000d7), propput]HRESULT isPlaying([in] long ScreenNum,[in] long Value);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_isPlaying(V_I4(&var0), V_I4(&var1));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x000000d9: {
			if (wFlags & DISPATCH_METHOD) {
				// line 97: [id(0x000000d9)]HRESULT SetLength([in] long ScreenNum,[in] long StopSecs);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = SetLength(V_I4(&var0), V_I4(&var1));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x000000da: {
			if (wFlags & DISPATCH_METHOD) {
				// line 101: [id(0x000000da)]HRESULT SetLoop([in] long ScreenNum,[in] long LoopState);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = SetLoop(V_I4(&var0), V_I4(&var1));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x000000db: {
			if (wFlags & DISPATCH_METHOD) {
				// line 105: [id(0x000000db)]HRESULT SetBackGround([in] long ScreenNum,[in] long Mode);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = SetBackGround(V_I4(&var0), V_I4(&var1));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x000000dc: {
			if (wFlags & DISPATCH_METHOD) {
				// line 109: [id(0x000000dc)]HRESULT BlockPlay([in] long ScreenNum,[in] long Mode);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = BlockPlay(V_I4(&var0), V_I4(&var1));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x000000dd: {
			if (wFlags & DISPATCH_METHOD) {
				// line 113: [id(0x000000dd)]HRESULT SetScreen([in] long ScreenNum);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = SetScreen(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x000000de: {
			if (wFlags & DISPATCH_METHOD) {
				// line 115: [id(0x000000de)]HRESULT SetScreenEx([in] long ScreenNum,[in] long xpos,[in] long ypos,[in] long swidth,[in] long sheight,[in] long popup);
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
				hres = SetScreenEx(V_I4(&var0), V_I4(&var1), V_I4(&var2), V_I4(&var3), V_I4(&var4), V_I4(&var5));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
				VariantClear(&var3);
				VariantClear(&var4);
				VariantClear(&var5);
			}
			break;
		}
		case 0x000000df: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 123: [id(0x000000df), propget]HRESULT SN([out, retval] long* Value);
				V_VT(&res) = VT_I4;
				hres = get_SN((LONG*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 125: [id(0x000000df), propput]HRESULT SN([in] long Value);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_SN(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x000000e0: {
			if (wFlags & DISPATCH_METHOD) {
				// line 127: [id(0x000000e0)]HRESULT B2SData([in] BSTR tIndex,[in] int Value);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = B2SData(V_BSTR(&var0), V_I4(&var1));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x000000e1: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 131: [id(0x000000e1), propget]HRESULT B2SFilter([out, retval] BSTR* Value);
				V_VT(&res) = VT_BSTR;
				hres = get_B2SFilter(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 133: [id(0x000000e1), propput]HRESULT B2SFilter([in] BSTR Value);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_B2SFilter(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x000000e2: {
			if (wFlags & DISPATCH_METHOD) {
				// line 135: [id(0x000000e2)]HRESULT Show([in] long ScreenNum);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = Show(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x000000e3: {
			if (wFlags & DISPATCH_METHOD) {
				// line 137: [id(0x000000e3)]HRESULT Hide([in] long ScreenNum);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = Hide(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x000000e4: {
			if (wFlags & DISPATCH_METHOD) {
				// line 139: [id(0x000000e4)]HRESULT B2SInit([in] BSTR TName,[in] BSTR RomName);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = B2SInit(V_BSTR(&var0), V_BSTR(&var1));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x000000e5: {
			if (wFlags & DISPATCH_METHOD) {
				// line 143: [id(0x000000e5)]HRESULT SendMSG([in] BSTR cMSG);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = SendMSG(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x000000e6: {
			if (wFlags & DISPATCH_METHOD) {
				// line 145: [id(0x000000e6)]HRESULT LabelNew([in] long ScreenNum,[in] BSTR LabelName,[in] BSTR FontName,[in] long Size,[in] long Color,[in] long Angle,[in] long xAlign,[in] long yAlign,[in] long xMargin,[in] long yMargin,[in] long PageNum,[in] long Visible);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
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
				hres = LabelNew(V_I4(&var0), V_BSTR(&var1), V_BSTR(&var2), V_I4(&var3), V_I4(&var4), V_I4(&var5), V_I4(&var6), V_I4(&var7), V_I4(&var8), V_I4(&var9), V_I4(&var10), V_I4(&var11));
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
		case 0x000000e7: {
			if (wFlags & DISPATCH_METHOD) {
				// line 159: [id(0x000000e7)]HRESULT LabelSet([in] long ScreenNum,[in] BSTR LabelName,[in] BSTR Caption,[in] long Visible,[in] BSTR Special);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
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
				VariantChangeType(&var4, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = LabelSet(V_I4(&var0), V_BSTR(&var1), V_BSTR(&var2), V_I4(&var3), V_BSTR(&var4));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
				VariantClear(&var3);
				VariantClear(&var4);
			}
			break;
		}
		case 0x000000e8: {
			if (wFlags & DISPATCH_METHOD) {
				// line 166: [id(0x000000e8)]HRESULT LabelSetEx();
				hres = LabelSetEx();
			}
			break;
		}
		case 0x000000e9: {
			if (wFlags & DISPATCH_METHOD) {
				// line 168: [id(0x000000e9)]HRESULT LabelShowPage([in] long ScreenNum,[in] long PageNum,[in] long Seconds,[in] BSTR Special);
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
				VariantChangeType(&var3, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = LabelShowPage(V_I4(&var0), V_I4(&var1), V_I4(&var2), V_BSTR(&var3));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
				VariantClear(&var3);
			}
			break;
		}
		case 0x000000ea: {
			if (wFlags & DISPATCH_METHOD) {
				// line 174: [id(0x000000ea)]HRESULT LabelInit([in] long ScreenNum);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = LabelInit(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x000000eb: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 176: [id(0x000000eb), propget]HRESULT GetGame([out, retval] BSTR* Value);
				V_VT(&res) = VT_BSTR;
				hres = get_GetGame(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 178: [id(0x000000eb), propput]HRESULT GetGame([in] BSTR Value);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_GetGame(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x000000ec: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 180: [id(0x000000ec), propget]HRESULT GetRoot([out, retval] BSTR* Value);
				V_VT(&res) = VT_BSTR;
				hres = get_GetRoot(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 182: [id(0x000000ec), propput]HRESULT GetRoot([in] BSTR Value);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_GetRoot(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x000000ed: {
			if (wFlags & DISPATCH_METHOD) {
				// line 184: [id(0x000000ed)]HRESULT SoundAdd([in] BSTR sname,[in] BSTR fname,[in] long svol,[in] double sX,[in] double sy,[in] BSTR SP);
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
				VariantChangeType(&var3, &pDispParams->rgvarg[--index], 0, VT_R8);
				VARIANT var4;
				V_VT(&var4) = VT_EMPTY;
				VariantChangeType(&var4, &pDispParams->rgvarg[--index], 0, VT_R8);
				VARIANT var5;
				V_VT(&var5) = VT_EMPTY;
				VariantChangeType(&var5, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = SoundAdd(V_BSTR(&var0), V_BSTR(&var1), V_I4(&var2), V_R8(&var3), V_R8(&var4), V_BSTR(&var5));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
				VariantClear(&var3);
				VariantClear(&var4);
				VariantClear(&var5);
			}
			break;
		}
		case 0x000000ee: {
			if (wFlags & DISPATCH_METHOD) {
				// line 192: [id(0x000000ee)]HRESULT SoundPlay([in] BSTR sname);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = SoundPlay(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x000000ef: {
			if (wFlags & DISPATCH_METHOD) {
				// line 194: [id(0x000000ef)]HRESULT PuPSound([in] BSTR sname,[in] long sX,[in] long sy,[in] long sz,[in] long vol,[in] BSTR SP);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
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
				VariantChangeType(&var5, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = PuPSound(V_BSTR(&var0), V_I4(&var1), V_I4(&var2), V_I4(&var3), V_I4(&var4), V_BSTR(&var5));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
				VariantClear(&var3);
				VariantClear(&var4);
				VariantClear(&var5);
			}
			break;
		}
		case 0x000000f0: {
			if (wFlags & DISPATCH_METHOD) {
				// line 202: [id(0x000000f0)]HRESULT InitPuPMenu([in] long Param1);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = InitPuPMenu(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x000000f1: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 204: [id(0x000000f1), propget]HRESULT B2SDisplays([out, retval] BSTR* Value);
				V_VT(&res) = VT_BSTR;
				hres = get_B2SDisplays(&V_BSTR(&res));
			}
			break;
		}
		case 0x000000f2: {
			if (wFlags & DISPATCH_METHOD) {
				// line 206: [id(0x000000f2)]HRESULT setVolumeCurrent([in] long ScreenNum,[in] long vol);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = setVolumeCurrent(V_I4(&var0), V_I4(&var1));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x000000f3: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 210: [id(0x000000f3), propget]HRESULT GameUpdate([in] BSTR GameTitle,[in] long Func,[in] long FuncData,[in] BSTR Extra,[out, retval] long* Value);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var2;
				V_VT(&var2) = VT_EMPTY;
				VariantChangeType(&var2, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var3;
				V_VT(&var3) = VT_EMPTY;
				VariantChangeType(&var3, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				V_VT(&res) = VT_I4;
				hres = get_GameUpdate(V_BSTR(&var0), V_I4(&var1), V_I4(&var2), V_BSTR(&var3), (LONG*)&V_I4(&res));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
				VariantClear(&var3);
			}
			break;
		}
		case 0x000000f4: {
			if (wFlags & DISPATCH_METHOD) {
				// line 217: [id(0x000000f4)]HRESULT GrabDC([in] long pWidth,[in] long pHeight,[in] BSTR wintitle,[out, retval] VARIANT* pixels);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var2;
				V_VT(&var2) = VT_EMPTY;
				VariantChangeType(&var2, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = GrabDC(V_I4(&var0), V_I4(&var1), V_BSTR(&var2), &res);
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
			}
			break;
		}
		case 0x000000f5: {
			if (wFlags & DISPATCH_METHOD) {
				// line 223: [id(0x000000f5)]HRESULT GetVersion([out, retval] BSTR* pver);
				V_VT(&res) = VT_BSTR;
				hres = GetVersion(&V_BSTR(&res));
			}
			break;
		}
		case 0x000000f6: {
			if (wFlags & DISPATCH_METHOD) {
				// line 225: [id(0x000000f6)]HRESULT GrabDC2([in] long pWidth,[in] long pHeight,[in] BSTR wintitle,[out, retval] SAFEARRAY(long)* pixels);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var2;
				V_VT(&var2) = VT_EMPTY;
				VariantChangeType(&var2, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				V_VT(&res) = VT_I4|VT_ARRAY;
				hres = GrabDC2(V_I4(&var0), V_I4(&var1), V_BSTR(&var2), (SAFEARRAY**)&V_ARRAY(&res));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
			}
			break;
		}
		case 0x000000f7: {
			if (wFlags & DISPATCH_METHOD) {
				// line 231: [id(0x000000f7)]HRESULT playevent([in] long ScreenNum,[in] BSTR playlist,[in] BSTR playfilename,[in] long volume,[in] long priority,[in] long playtype,[in] long Seconds,[in] BSTR Special);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
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
				VariantChangeType(&var7, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = playevent(V_I4(&var0), V_BSTR(&var1), V_BSTR(&var2), V_I4(&var3), V_I4(&var4), V_I4(&var5), V_I4(&var6), V_BSTR(&var7));
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
		case 0x000000f8: {
			if (wFlags & DISPATCH_METHOD) {
				// line 241: [id(0x000000f8)]HRESULT SetPosVideo([in] long ScreenNum,[in] long StartPos,[in] long EndPos,[in] long Mode,[in] BSTR Special);
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
				VariantChangeType(&var4, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = SetPosVideo(V_I4(&var0), V_I4(&var1), V_I4(&var2), V_I4(&var3), V_BSTR(&var4));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
				VariantClear(&var3);
				VariantClear(&var4);
			}
			break;
		}
		case 0x000000f9: {
			if (wFlags & DISPATCH_METHOD) {
				// line 248: [id(0x000000f9)]HRESULT PuPClose();
				hres = PuPClose();
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
