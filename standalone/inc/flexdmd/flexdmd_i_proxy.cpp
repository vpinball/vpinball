#include "stdafx.h"
#include "olectl.h"

#include "FlexDMD.h"

#include "actions/ActionFactory.h"
#include "actions/SequenceAction.h"
#include "actions/ParallelAction.h"
#include "actions/TweenAction.h"

#include "actors/Frame.h"
#include "actors/Group.h"
#include "actors/Image.h"
#include "actors/Label.h"
#include "actors/AnimatedActor.h"

STDMETHODIMP SequenceAction::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} namesIdsList[] = {
			{ NULL },
			{ L"Add", 0x60020000 }
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

STDMETHODIMP SequenceAction::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
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
			if (wFlags & DISPATCH_METHOD) {
				// line 108: [id(0x60020000)]HRESULT Add([in] IUnknown* action,[out, retval] ICompositeAction** pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				V_VT(&res) = VT_DISPATCH;
				hres = Add(V_UNKNOWN(&var0), (ICompositeAction**)&V_DISPATCH(&res));
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

STDMETHODIMP ParallelAction::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} namesIdsList[] = {
			{ NULL },
			{ L"Add", 0x60020000 }
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

STDMETHODIMP ParallelAction::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
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
			if (wFlags & DISPATCH_METHOD) {
				// line 108: [id(0x60020000)]HRESULT Add([in] IUnknown* action,[out, retval] ICompositeAction** pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				V_VT(&res) = VT_DISPATCH;
				hres = Add(V_UNKNOWN(&var0), (ICompositeAction**)&V_DISPATCH(&res));
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

STDMETHODIMP TweenAction::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} namesIdsList[] = {
			{ NULL },
			{ L"Ease", 0x60020000 }
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

STDMETHODIMP TweenAction::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
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
				// line 124: [id(0x60020000), propget]HRESULT Ease([out, retval] Interpolation* pRetVal);
				V_VT(&res) = VT_I4;
				hres = get_Ease((Interpolation*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 126: [id(0x60020000), propput]HRESULT Ease([in] Interpolation pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_Ease((Interpolation)V_I4(&var0));
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

STDMETHODIMP ActionFactory::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} namesIdsList[] = {
			{ NULL },
			{ L"AddChild", 0x60020009 },
			{ L"AddTo", 0x60020007 },
			{ L"Blink", 0x60020005 },
			{ L"Delayed", 0x60020001 },
			{ L"MoveTo", 0x6002000c },
			{ L"Parallel", 0x60020002 },
			{ L"RemoveChild", 0x6002000a },
			{ L"RemoveFromParent", 0x60020008 },
			{ L"Repeat", 0x60020004 },
			{ L"Seek", 0x6002000b },
			{ L"Sequence", 0x60020003 },
			{ L"Show", 0x60020006 },
			{ L"Wait", 0x60020000 }
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

STDMETHODIMP ActionFactory::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
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
			if (wFlags & DISPATCH_METHOD) {
				// line 140: [id(0x60020000)]HRESULT Wait([in] single secondsToWait,[out, retval] IUnknown** pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				V_VT(&res) = VT_UNKNOWN;
				hres = Wait(V_R4(&var0), &V_UNKNOWN(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 0x60020001: {
			if (wFlags & DISPATCH_METHOD) {
				// line 144: [id(0x60020001)]HRESULT Delayed([in] single secondsToWait,[in] IUnknown* action,[out, retval] IUnknown** pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantCopyInd(&var1, &pDispParams->rgvarg[--index]);
				V_VT(&res) = VT_UNKNOWN;
				hres = Delayed(V_R4(&var0), V_UNKNOWN(&var1), &V_UNKNOWN(&res));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x60020002: {
			if (wFlags & DISPATCH_METHOD) {
				// line 149: [id(0x60020002)]HRESULT Parallel([out, retval] ICompositeAction** pRetVal);
				V_VT(&res) = VT_DISPATCH;
				hres = Parallel((ICompositeAction**)&V_DISPATCH(&res));
			}
			break;
		}
		case 0x60020003: {
			if (wFlags & DISPATCH_METHOD) {
				// line 151: [id(0x60020003)]HRESULT Sequence([out, retval] ICompositeAction** pRetVal);
				V_VT(&res) = VT_DISPATCH;
				hres = Sequence((ICompositeAction**)&V_DISPATCH(&res));
			}
			break;
		}
		case 0x60020004: {
			if (wFlags & DISPATCH_METHOD) {
				// line 153: [id(0x60020004)]HRESULT Repeat([in] IUnknown* action,[in] long count,[out, retval] IUnknown** pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_I4);
				V_VT(&res) = VT_UNKNOWN;
				hres = Repeat(V_UNKNOWN(&var0), V_I4(&var1), &V_UNKNOWN(&res));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x60020005: {
			if (wFlags & DISPATCH_METHOD) {
				// line 158: [id(0x60020005)]HRESULT Blink([in] single secondsShow,[in] single secondsHide,[in] long Repeat,[out, retval] IUnknown** pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var2;
				V_VT(&var2) = VT_EMPTY;
				VariantChangeType(&var2, &pDispParams->rgvarg[--index], 0, VT_I4);
				V_VT(&res) = VT_UNKNOWN;
				hres = Blink(V_R4(&var0), V_R4(&var1), V_I4(&var2), &V_UNKNOWN(&res));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
			}
			break;
		}
		case 0x60020006: {
			if (wFlags & DISPATCH_METHOD) {
				// line 164: [id(0x60020006)]HRESULT Show([in] VARIANT_BOOL visible,[out, retval] IUnknown** pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				V_VT(&res) = VT_UNKNOWN;
				hres = Show(V_BOOL(&var0), &V_UNKNOWN(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 0x60020007: {
			if (wFlags & DISPATCH_METHOD) {
				// line 168: [id(0x60020007)]HRESULT AddTo([in] IGroupActor* parent,[out, retval] IUnknown** pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				V_VT(&res) = VT_UNKNOWN;
				hres = AddTo((IGroupActor*)&var0, &V_UNKNOWN(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 0x60020008: {
			if (wFlags & DISPATCH_METHOD) {
				// line 172: [id(0x60020008)]HRESULT RemoveFromParent([out, retval] IUnknown** pRetVal);
				V_VT(&res) = VT_UNKNOWN;
				hres = RemoveFromParent(&V_UNKNOWN(&res));
			}
			break;
		}
		case 0x60020009: {
			if (wFlags & DISPATCH_METHOD) {
				// line 174: [id(0x60020009)]HRESULT AddChild([in] IUnknown* child,[out, retval] IUnknown** pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				V_VT(&res) = VT_UNKNOWN;
				hres = AddChild(V_UNKNOWN(&var0), &V_UNKNOWN(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 0x6002000a: {
			if (wFlags & DISPATCH_METHOD) {
				// line 178: [id(0x6002000a)]HRESULT RemoveChild([in] IUnknown* child,[out, retval] IUnknown** pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				V_VT(&res) = VT_UNKNOWN;
				hres = RemoveChild(V_UNKNOWN(&var0), &V_UNKNOWN(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 0x6002000b: {
			if (wFlags & DISPATCH_METHOD) {
				// line 182: [id(0x6002000b)]HRESULT Seek([in] single pos,[out, retval] IUnknown** pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				V_VT(&res) = VT_UNKNOWN;
				hres = Seek(V_R4(&var0), &V_UNKNOWN(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 0x6002000c: {
			if (wFlags & DISPATCH_METHOD) {
				// line 186: [id(0x6002000c)]HRESULT MoveTo([in] single x,[in] single y,[in] single duration,[out, retval] ITweenAction** pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var2;
				V_VT(&var2) = VT_EMPTY;
				VariantChangeType(&var2, &pDispParams->rgvarg[--index], 0, VT_R4);
				V_VT(&res) = VT_DISPATCH;
				hres = MoveTo(V_R4(&var0), V_R4(&var1), V_R4(&var2), (ITweenAction**)&V_DISPATCH(&res));
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

STDMETHODIMP Group::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} namesIdsList[] = {
			{ NULL },
			{ L"ActionFactory", 0x60020018 },
			{ L"AddAction", 0x60020019 },
			{ L"AddActor", 0x60020025 },
			{ L"ChildCount", 0x6002001d },
			{ L"ClearActions", 0x6002001a },
			{ L"ClearBackground", 0x6002000e },
			{ L"Clip", 0x6002001b },
			{ L"FillParent", 0x6002000c },
			{ L"GetFrame", 0x60020020 },
			{ L"GetGroup", 0x6002001f },
			{ L"GetImage", 0x60020023 },
			{ L"GetLabel", 0x60020021 },
			{ L"GetVideo", 0x60020022 },
			{ L"HasChild", 0x6002001e },
			{ L"Height", 0x60020008 },
			{ L"Name", 0x60020000 },
			{ L"Pack", 0x60020016 },
			{ L"PrefHeight", 0x60020015 },
			{ L"PrefWidth", 0x60020014 },
			{ L"Remove", 0x60020017 },
			{ L"RemoveActor", 0x60020026 },
			{ L"RemoveAll", 0x60020024 },
			{ L"SetAlignedPosition", 0x60020012 },
			{ L"SetBounds", 0x60020010 },
			{ L"SetPosition", 0x60020011 },
			{ L"SetSize", 0x60020013 },
			{ L"visible", 0x6002000a },
			{ L"Width", 0x60020006 },
			{ L"x", 0x60020002 },
			{ L"y", 0x60020004 }
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

STDMETHODIMP Group::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
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
				// line 281: [id(0x60020000), propget]HRESULT Name([out, retval] BSTR* pRetVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Name(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 283: [id(0x60020000), propput]HRESULT Name([in] BSTR pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Name(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x60020002: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 285: [id(0x60020002), propget]HRESULT x([out, retval] single* pRetVal);
				V_VT(&res) = VT_R4;
				hres = get_x(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 287: [id(0x60020002), propput]HRESULT x([in] single pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_x(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x60020004: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 289: [id(0x60020004), propget]HRESULT y([out, retval] single* pRetVal);
				V_VT(&res) = VT_R4;
				hres = get_y(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 291: [id(0x60020004), propput]HRESULT y([in] single pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_y(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x60020006: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 293: [id(0x60020006), propget]HRESULT Width([out, retval] single* pRetVal);
				V_VT(&res) = VT_R4;
				hres = get_Width(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 295: [id(0x60020006), propput]HRESULT Width([in] single pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Width(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x60020008: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 297: [id(0x60020008), propget]HRESULT Height([out, retval] single* pRetVal);
				V_VT(&res) = VT_R4;
				hres = get_Height(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 299: [id(0x60020008), propput]HRESULT Height([in] single pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Height(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x6002000a: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 301: [id(0x6002000a), propget]HRESULT visible([out, retval] VARIANT_BOOL* pRetVal);
				V_VT(&res) = VT_BOOL;
				hres = get_visible(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 303: [id(0x6002000a), propput]HRESULT visible([in] VARIANT_BOOL pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_visible(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x6002000c: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 305: [id(0x6002000c), propget]HRESULT FillParent([out, retval] VARIANT_BOOL* pRetVal);
				V_VT(&res) = VT_BOOL;
				hres = get_FillParent(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 307: [id(0x6002000c), propput]HRESULT FillParent([in] VARIANT_BOOL pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_FillParent(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x6002000e: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 309: [id(0x6002000e), propget]HRESULT ClearBackground([out, retval] VARIANT_BOOL* pRetVal);
				V_VT(&res) = VT_BOOL;
				hres = get_ClearBackground(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 311: [id(0x6002000e), propput]HRESULT ClearBackground([in] VARIANT_BOOL pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_ClearBackground(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x60020010: {
			if (wFlags & DISPATCH_METHOD) {
				// line 313: [id(0x60020010)]HRESULT SetBounds([in] single x,[in] single y,[in] single Width,[in] single Height);
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
				hres = SetBounds(V_R4(&var0), V_R4(&var1), V_R4(&var2), V_R4(&var3));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
				VariantClear(&var3);
			}
			break;
		}
		case 0x60020011: {
			if (wFlags & DISPATCH_METHOD) {
				// line 319: [id(0x60020011)]HRESULT SetPosition([in] single x,[in] single y);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = SetPosition(V_R4(&var0), V_R4(&var1));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x60020012: {
			if (wFlags & DISPATCH_METHOD) {
				// line 323: [id(0x60020012)]HRESULT SetAlignedPosition([in] single x,[in] single y,[in] Alignment alignment);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var2;
				V_VT(&var2) = VT_EMPTY;
				VariantChangeType(&var2, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = SetAlignedPosition(V_R4(&var0), V_R4(&var1), (Alignment)V_I4(&var2));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
			}
			break;
		}
		case 0x60020013: {
			if (wFlags & DISPATCH_METHOD) {
				// line 328: [id(0x60020013)]HRESULT SetSize([in] single Width,[in] single Height);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = SetSize(V_R4(&var0), V_R4(&var1));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x60020014: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 332: [id(0x60020014), propget]HRESULT PrefWidth([out, retval] single* pRetVal);
				V_VT(&res) = VT_R4;
				hres = get_PrefWidth(&V_R4(&res));
			}
			break;
		}
		case 0x60020015: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 334: [id(0x60020015), propget]HRESULT PrefHeight([out, retval] single* pRetVal);
				V_VT(&res) = VT_R4;
				hres = get_PrefHeight(&V_R4(&res));
			}
			break;
		}
		case 0x60020016: {
			if (wFlags & DISPATCH_METHOD) {
				// line 336: [id(0x60020016)]HRESULT Pack();
				hres = Pack();
			}
			break;
		}
		case 0x60020017: {
			if (wFlags & DISPATCH_METHOD) {
				// line 338: [id(0x60020017)]HRESULT Remove();
				hres = Remove();
			}
			break;
		}
		case 0x60020018: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 340: [id(0x60020018), propget]HRESULT ActionFactory([out, retval] IActionFactory** pRetVal);
				V_VT(&res) = VT_DISPATCH;
				hres = get_ActionFactory((IActionFactory**)&V_DISPATCH(&res));
			}
			break;
		}
		case 0x60020019: {
			if (wFlags & DISPATCH_METHOD) {
				// line 342: [id(0x60020019)]HRESULT AddAction([in] IUnknown* action);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = AddAction(V_UNKNOWN(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x6002001a: {
			if (wFlags & DISPATCH_METHOD) {
				// line 344: [id(0x6002001a)]HRESULT ClearActions();
				hres = ClearActions();
			}
			break;
		}
		case 0x6002001b: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 346: [id(0x6002001b), propget]HRESULT Clip([out, retval] VARIANT_BOOL* pRetVal);
				V_VT(&res) = VT_BOOL;
				hres = get_Clip(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 348: [id(0x6002001b), propput]HRESULT Clip([in] VARIANT_BOOL pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_Clip(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x6002001d: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 350: [id(0x6002001d), propget]HRESULT ChildCount([out, retval] long* pRetVal);
				V_VT(&res) = VT_I4;
				hres = get_ChildCount((LONG*)&V_I4(&res));
			}
			break;
		}
		case 0x6002001e: {
			if (wFlags & DISPATCH_METHOD) {
				// line 352: [id(0x6002001e)]HRESULT HasChild([in] BSTR Name,[out, retval] VARIANT_BOOL* pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				V_VT(&res) = VT_BOOL;
				hres = HasChild(V_BSTR(&var0), &V_BOOL(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 0x6002001f: {
			if (wFlags & DISPATCH_METHOD) {
				// line 356: [id(0x6002001f)]HRESULT GetGroup([in] BSTR Name,[out, retval] IGroupActor** pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				V_VT(&res) = VT_DISPATCH;
				hres = GetGroup(V_BSTR(&var0), (IGroupActor**)&V_DISPATCH(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 0x60020020: {
			if (wFlags & DISPATCH_METHOD) {
				// line 360: [id(0x60020020)]HRESULT GetFrame([in] BSTR Name,[out, retval] IFrameActor** pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				V_VT(&res) = VT_DISPATCH;
				hres = GetFrame(V_BSTR(&var0), (IFrameActor**)&V_DISPATCH(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 0x60020021: {
			if (wFlags & DISPATCH_METHOD) {
				// line 364: [id(0x60020021)]HRESULT GetLabel([in] BSTR Name,[out, retval] ILabelActor** pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				V_VT(&res) = VT_DISPATCH;
				hres = GetLabel(V_BSTR(&var0), (ILabelActor**)&V_DISPATCH(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 0x60020022: {
			if (wFlags & DISPATCH_METHOD) {
				// line 368: [id(0x60020022)]HRESULT GetVideo([in] BSTR Name,[out, retval] IVideoActor** pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				V_VT(&res) = VT_DISPATCH;
				hres = GetVideo(V_BSTR(&var0), (IVideoActor**)&V_DISPATCH(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 0x60020023: {
			if (wFlags & DISPATCH_METHOD) {
				// line 372: [id(0x60020023)]HRESULT GetImage([in] BSTR Name,[out, retval] IImageActor** pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				V_VT(&res) = VT_DISPATCH;
				hres = GetImage(V_BSTR(&var0), (IImageActor**)&V_DISPATCH(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 0x60020024: {
			if (wFlags & DISPATCH_METHOD) {
				// line 376: [id(0x60020024)]HRESULT RemoveAll();
				hres = RemoveAll();
			}
			break;
		}
		case 0x60020025: {
			if (wFlags & DISPATCH_METHOD) {
				// line 378: [id(0x60020025)]HRESULT AddActor([in] IUnknown* child);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = AddActor(V_UNKNOWN(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x60020026: {
			if (wFlags & DISPATCH_METHOD) {
				// line 380: [id(0x60020026)]HRESULT RemoveActor([in] IUnknown* child);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = RemoveActor(V_UNKNOWN(&var0));
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

STDMETHODIMP Frame::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} namesIdsList[] = {
			{ NULL },
			{ L"ActionFactory", 0x60020018 },
			{ L"AddAction", 0x60020019 },
			{ L"BorderColor", 0x6002001d },
			{ L"ClearActions", 0x6002001a },
			{ L"ClearBackground", 0x6002000e },
			{ L"Fill", 0x6002001f },
			{ L"FillColor", 0x60020021 },
			{ L"FillParent", 0x6002000c },
			{ L"Height", 0x60020008 },
			{ L"Name", 0x60020000 },
			{ L"Pack", 0x60020016 },
			{ L"PrefHeight", 0x60020015 },
			{ L"PrefWidth", 0x60020014 },
			{ L"Remove", 0x60020017 },
			{ L"SetAlignedPosition", 0x60020012 },
			{ L"SetBounds", 0x60020010 },
			{ L"SetPosition", 0x60020011 },
			{ L"SetSize", 0x60020013 },
			{ L"Thickness", 0x6002001b },
			{ L"visible", 0x6002000a },
			{ L"Width", 0x60020006 },
			{ L"x", 0x60020002 },
			{ L"y", 0x60020004 }
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

STDMETHODIMP Frame::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
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
				// line 394: [id(0x60020000), propget]HRESULT Name([out, retval] BSTR* pRetVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Name(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 396: [id(0x60020000), propput]HRESULT Name([in] BSTR pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Name(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x60020002: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 398: [id(0x60020002), propget]HRESULT x([out, retval] single* pRetVal);
				V_VT(&res) = VT_R4;
				hres = get_x(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 400: [id(0x60020002), propput]HRESULT x([in] single pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_x(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x60020004: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 402: [id(0x60020004), propget]HRESULT y([out, retval] single* pRetVal);
				V_VT(&res) = VT_R4;
				hres = get_y(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 404: [id(0x60020004), propput]HRESULT y([in] single pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_y(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x60020006: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 406: [id(0x60020006), propget]HRESULT Width([out, retval] single* pRetVal);
				V_VT(&res) = VT_R4;
				hres = get_Width(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 408: [id(0x60020006), propput]HRESULT Width([in] single pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Width(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x60020008: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 410: [id(0x60020008), propget]HRESULT Height([out, retval] single* pRetVal);
				V_VT(&res) = VT_R4;
				hres = get_Height(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 412: [id(0x60020008), propput]HRESULT Height([in] single pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Height(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x6002000a: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 414: [id(0x6002000a), propget]HRESULT visible([out, retval] VARIANT_BOOL* pRetVal);
				V_VT(&res) = VT_BOOL;
				hres = get_visible(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 416: [id(0x6002000a), propput]HRESULT visible([in] VARIANT_BOOL pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_visible(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x6002000c: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 418: [id(0x6002000c), propget]HRESULT FillParent([out, retval] VARIANT_BOOL* pRetVal);
				V_VT(&res) = VT_BOOL;
				hres = get_FillParent(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 420: [id(0x6002000c), propput]HRESULT FillParent([in] VARIANT_BOOL pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_FillParent(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x6002000e: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 422: [id(0x6002000e), propget]HRESULT ClearBackground([out, retval] VARIANT_BOOL* pRetVal);
				V_VT(&res) = VT_BOOL;
				hres = get_ClearBackground(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 424: [id(0x6002000e), propput]HRESULT ClearBackground([in] VARIANT_BOOL pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_ClearBackground(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x60020010: {
			if (wFlags & DISPATCH_METHOD) {
				// line 426: [id(0x60020010)]HRESULT SetBounds([in] single x,[in] single y,[in] single Width,[in] single Height);
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
				hres = SetBounds(V_R4(&var0), V_R4(&var1), V_R4(&var2), V_R4(&var3));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
				VariantClear(&var3);
			}
			break;
		}
		case 0x60020011: {
			if (wFlags & DISPATCH_METHOD) {
				// line 432: [id(0x60020011)]HRESULT SetPosition([in] single x,[in] single y);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = SetPosition(V_R4(&var0), V_R4(&var1));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x60020012: {
			if (wFlags & DISPATCH_METHOD) {
				// line 436: [id(0x60020012)]HRESULT SetAlignedPosition([in] single x,[in] single y,[in] Alignment alignment);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var2;
				V_VT(&var2) = VT_EMPTY;
				VariantChangeType(&var2, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = SetAlignedPosition(V_R4(&var0), V_R4(&var1), (Alignment)V_I4(&var2));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
			}
			break;
		}
		case 0x60020013: {
			if (wFlags & DISPATCH_METHOD) {
				// line 441: [id(0x60020013)]HRESULT SetSize([in] single Width,[in] single Height);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = SetSize(V_R4(&var0), V_R4(&var1));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x60020014: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 445: [id(0x60020014), propget]HRESULT PrefWidth([out, retval] single* pRetVal);
				V_VT(&res) = VT_R4;
				hres = get_PrefWidth(&V_R4(&res));
			}
			break;
		}
		case 0x60020015: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 447: [id(0x60020015), propget]HRESULT PrefHeight([out, retval] single* pRetVal);
				V_VT(&res) = VT_R4;
				hres = get_PrefHeight(&V_R4(&res));
			}
			break;
		}
		case 0x60020016: {
			if (wFlags & DISPATCH_METHOD) {
				// line 449: [id(0x60020016)]HRESULT Pack();
				hres = Pack();
			}
			break;
		}
		case 0x60020017: {
			if (wFlags & DISPATCH_METHOD) {
				// line 451: [id(0x60020017)]HRESULT Remove();
				hres = Remove();
			}
			break;
		}
		case 0x60020018: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 453: [id(0x60020018), propget]HRESULT ActionFactory([out, retval] IActionFactory** pRetVal);
				V_VT(&res) = VT_DISPATCH;
				hres = get_ActionFactory((IActionFactory**)&V_DISPATCH(&res));
			}
			break;
		}
		case 0x60020019: {
			if (wFlags & DISPATCH_METHOD) {
				// line 455: [id(0x60020019)]HRESULT AddAction([in] IUnknown* action);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = AddAction(V_UNKNOWN(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x6002001a: {
			if (wFlags & DISPATCH_METHOD) {
				// line 457: [id(0x6002001a)]HRESULT ClearActions();
				hres = ClearActions();
			}
			break;
		}
		case 0x6002001b: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 459: [id(0x6002001b), propget]HRESULT Thickness([out, retval] long* pRetVal);
				V_VT(&res) = VT_I4;
				hres = get_Thickness((LONG*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 461: [id(0x6002001b), propput]HRESULT Thickness([in] long pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_Thickness(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x6002001d: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 463: [id(0x6002001d), propget]HRESULT BorderColor([out, retval] OLE_COLOR* pRetVal);
				V_VT(&res) = VT_UI4;
				hres = get_BorderColor(&V_UI4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 465: [id(0x6002001d), propput]HRESULT BorderColor([in] OLE_COLOR pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_UI4);
				hres = put_BorderColor((OLE_COLOR)V_UI4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x6002001f: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 467: [id(0x6002001f), propget]HRESULT Fill([out, retval] VARIANT_BOOL* pRetVal);
				V_VT(&res) = VT_BOOL;
				hres = get_Fill(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 469: [id(0x6002001f), propput]HRESULT Fill([in] VARIANT_BOOL pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_Fill(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x60020021: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 471: [id(0x60020021), propget]HRESULT FillColor([out, retval] OLE_COLOR* pRetVal);
				V_VT(&res) = VT_UI4;
				hres = get_FillColor(&V_UI4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 473: [id(0x60020021), propput]HRESULT FillColor([in] OLE_COLOR pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_UI4);
				hres = put_FillColor((OLE_COLOR)V_UI4(&var0));
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

STDMETHODIMP Image::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} namesIdsList[] = {
			{ NULL },
			{ L"ActionFactory", 0x60020018 },
			{ L"AddAction", 0x60020019 },
			{ L"Bitmap", 0x6002001b },
			{ L"ClearActions", 0x6002001a },
			{ L"ClearBackground", 0x6002000e },
			{ L"FillParent", 0x6002000c },
			{ L"Height", 0x60020008 },
			{ L"Name", 0x60020000 },
			{ L"Pack", 0x60020016 },
			{ L"PrefHeight", 0x60020015 },
			{ L"PrefWidth", 0x60020014 },
			{ L"Remove", 0x60020017 },
			{ L"SetAlignedPosition", 0x60020012 },
			{ L"SetBounds", 0x60020010 },
			{ L"SetPosition", 0x60020011 },
			{ L"SetSize", 0x60020013 },
			{ L"visible", 0x6002000a },
			{ L"Width", 0x60020006 },
			{ L"x", 0x60020002 },
			{ L"y", 0x60020004 }
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

STDMETHODIMP Image::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
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
				// line 487: [id(0x60020000), propget]HRESULT Name([out, retval] BSTR* pRetVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Name(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 489: [id(0x60020000), propput]HRESULT Name([in] BSTR pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Name(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x60020002: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 491: [id(0x60020002), propget]HRESULT x([out, retval] single* pRetVal);
				V_VT(&res) = VT_R4;
				hres = get_x(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 493: [id(0x60020002), propput]HRESULT x([in] single pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_x(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x60020004: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 495: [id(0x60020004), propget]HRESULT y([out, retval] single* pRetVal);
				V_VT(&res) = VT_R4;
				hres = get_y(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 497: [id(0x60020004), propput]HRESULT y([in] single pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_y(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x60020006: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 499: [id(0x60020006), propget]HRESULT Width([out, retval] single* pRetVal);
				V_VT(&res) = VT_R4;
				hres = get_Width(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 501: [id(0x60020006), propput]HRESULT Width([in] single pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Width(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x60020008: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 503: [id(0x60020008), propget]HRESULT Height([out, retval] single* pRetVal);
				V_VT(&res) = VT_R4;
				hres = get_Height(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 505: [id(0x60020008), propput]HRESULT Height([in] single pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Height(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x6002000a: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 507: [id(0x6002000a), propget]HRESULT visible([out, retval] VARIANT_BOOL* pRetVal);
				V_VT(&res) = VT_BOOL;
				hres = get_visible(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 509: [id(0x6002000a), propput]HRESULT visible([in] VARIANT_BOOL pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_visible(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x6002000c: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 511: [id(0x6002000c), propget]HRESULT FillParent([out, retval] VARIANT_BOOL* pRetVal);
				V_VT(&res) = VT_BOOL;
				hres = get_FillParent(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 513: [id(0x6002000c), propput]HRESULT FillParent([in] VARIANT_BOOL pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_FillParent(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x6002000e: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 515: [id(0x6002000e), propget]HRESULT ClearBackground([out, retval] VARIANT_BOOL* pRetVal);
				V_VT(&res) = VT_BOOL;
				hres = get_ClearBackground(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 517: [id(0x6002000e), propput]HRESULT ClearBackground([in] VARIANT_BOOL pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_ClearBackground(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x60020010: {
			if (wFlags & DISPATCH_METHOD) {
				// line 519: [id(0x60020010)]HRESULT SetBounds([in] single x,[in] single y,[in] single Width,[in] single Height);
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
				hres = SetBounds(V_R4(&var0), V_R4(&var1), V_R4(&var2), V_R4(&var3));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
				VariantClear(&var3);
			}
			break;
		}
		case 0x60020011: {
			if (wFlags & DISPATCH_METHOD) {
				// line 525: [id(0x60020011)]HRESULT SetPosition([in] single x,[in] single y);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = SetPosition(V_R4(&var0), V_R4(&var1));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x60020012: {
			if (wFlags & DISPATCH_METHOD) {
				// line 529: [id(0x60020012)]HRESULT SetAlignedPosition([in] single x,[in] single y,[in] Alignment alignment);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var2;
				V_VT(&var2) = VT_EMPTY;
				VariantChangeType(&var2, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = SetAlignedPosition(V_R4(&var0), V_R4(&var1), (Alignment)V_I4(&var2));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
			}
			break;
		}
		case 0x60020013: {
			if (wFlags & DISPATCH_METHOD) {
				// line 534: [id(0x60020013)]HRESULT SetSize([in] single Width,[in] single Height);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = SetSize(V_R4(&var0), V_R4(&var1));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x60020014: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 538: [id(0x60020014), propget]HRESULT PrefWidth([out, retval] single* pRetVal);
				V_VT(&res) = VT_R4;
				hres = get_PrefWidth(&V_R4(&res));
			}
			break;
		}
		case 0x60020015: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 540: [id(0x60020015), propget]HRESULT PrefHeight([out, retval] single* pRetVal);
				V_VT(&res) = VT_R4;
				hres = get_PrefHeight(&V_R4(&res));
			}
			break;
		}
		case 0x60020016: {
			if (wFlags & DISPATCH_METHOD) {
				// line 542: [id(0x60020016)]HRESULT Pack();
				hres = Pack();
			}
			break;
		}
		case 0x60020017: {
			if (wFlags & DISPATCH_METHOD) {
				// line 544: [id(0x60020017)]HRESULT Remove();
				hres = Remove();
			}
			break;
		}
		case 0x60020018: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 546: [id(0x60020018), propget]HRESULT ActionFactory([out, retval] IActionFactory** pRetVal);
				V_VT(&res) = VT_DISPATCH;
				hres = get_ActionFactory((IActionFactory**)&V_DISPATCH(&res));
			}
			break;
		}
		case 0x60020019: {
			if (wFlags & DISPATCH_METHOD) {
				// line 548: [id(0x60020019)]HRESULT AddAction([in] IUnknown* action);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = AddAction(V_UNKNOWN(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x6002001a: {
			if (wFlags & DISPATCH_METHOD) {
				// line 550: [id(0x6002001a)]HRESULT ClearActions();
				hres = ClearActions();
			}
			break;
		}
		case 0x6002001b: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 552: [id(0x6002001b), propget]HRESULT Bitmap([out, retval] _Bitmap** pRetVal);
				V_VT(&res) = VT_DISPATCH;
				hres = get_Bitmap((_Bitmap**)&V_DISPATCH(&res));
			}
            else if (wFlags & DISPATCH_PROPERTYPUT) {
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = putref_Bitmap((_Bitmap*)V_UNKNOWN(&var0));
				VariantClear(&var0);
			}
			else if (wFlags & DISPATCH_PROPERTYPUTREF) {
				// line 554: [id(0x6002001b), propputref]HRESULT Bitmap([in] _Bitmap* pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = putref_Bitmap((_Bitmap*)V_UNKNOWN(&var0));
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

STDMETHODIMP AnimatedActor::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} namesIdsList[] = {
			{ NULL },
			{ L"ActionFactory", 0x60020018 },
			{ L"AddAction", 0x60020019 },
			{ L"ClearActions", 0x6002001a },
			{ L"ClearBackground", 0x6002000e },
			{ L"FillParent", 0x6002000c },
			{ L"Height", 0x60020008 },
			{ L"Length", 0x6002001f },
			{ L"Loop", 0x60020020 },
			{ L"Name", 0x60020000 },
			{ L"Pack", 0x60020016 },
			{ L"Paused", 0x60020022 },
			{ L"PlaySpeed", 0x60020024 },
			{ L"PrefHeight", 0x60020015 },
			{ L"PrefWidth", 0x60020014 },
			{ L"Remove", 0x60020017 },
			{ L"Seek", 0x60020026 },
			{ L"SetAlignedPosition", 0x60020012 },
			{ L"SetBounds", 0x60020010 },
			{ L"SetPosition", 0x60020011 },
			{ L"SetSize", 0x60020013 },
			{ L"visible", 0x6002000a },
			{ L"Width", 0x60020006 },
			{ L"x", 0x60020002 },
			{ L"y", 0x60020004 }
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

STDMETHODIMP AnimatedActor::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
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
				// line 568: [id(0x60020000), propget]HRESULT Name([out, retval] BSTR* pRetVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Name(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 570: [id(0x60020000), propput]HRESULT Name([in] BSTR pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Name(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x60020002: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 572: [id(0x60020002), propget]HRESULT x([out, retval] single* pRetVal);
				V_VT(&res) = VT_R4;
				hres = get_x(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 574: [id(0x60020002), propput]HRESULT x([in] single pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_x(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x60020004: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 576: [id(0x60020004), propget]HRESULT y([out, retval] single* pRetVal);
				V_VT(&res) = VT_R4;
				hres = get_y(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 578: [id(0x60020004), propput]HRESULT y([in] single pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_y(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x60020006: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 580: [id(0x60020006), propget]HRESULT Width([out, retval] single* pRetVal);
				V_VT(&res) = VT_R4;
				hres = get_Width(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 582: [id(0x60020006), propput]HRESULT Width([in] single pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Width(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x60020008: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 584: [id(0x60020008), propget]HRESULT Height([out, retval] single* pRetVal);
				V_VT(&res) = VT_R4;
				hres = get_Height(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 586: [id(0x60020008), propput]HRESULT Height([in] single pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Height(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x6002000a: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 588: [id(0x6002000a), propget]HRESULT visible([out, retval] VARIANT_BOOL* pRetVal);
				V_VT(&res) = VT_BOOL;
				hres = get_visible(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 590: [id(0x6002000a), propput]HRESULT visible([in] VARIANT_BOOL pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_visible(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x6002000c: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 592: [id(0x6002000c), propget]HRESULT FillParent([out, retval] VARIANT_BOOL* pRetVal);
				V_VT(&res) = VT_BOOL;
				hres = get_FillParent(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 594: [id(0x6002000c), propput]HRESULT FillParent([in] VARIANT_BOOL pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_FillParent(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x6002000e: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 596: [id(0x6002000e), propget]HRESULT ClearBackground([out, retval] VARIANT_BOOL* pRetVal);
				V_VT(&res) = VT_BOOL;
				hres = get_ClearBackground(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 598: [id(0x6002000e), propput]HRESULT ClearBackground([in] VARIANT_BOOL pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_ClearBackground(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x60020010: {
			if (wFlags & DISPATCH_METHOD) {
				// line 600: [id(0x60020010)]HRESULT SetBounds([in] single x,[in] single y,[in] single Width,[in] single Height);
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
				hres = SetBounds(V_R4(&var0), V_R4(&var1), V_R4(&var2), V_R4(&var3));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
				VariantClear(&var3);
			}
			break;
		}
		case 0x60020011: {
			if (wFlags & DISPATCH_METHOD) {
				// line 606: [id(0x60020011)]HRESULT SetPosition([in] single x,[in] single y);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = SetPosition(V_R4(&var0), V_R4(&var1));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x60020012: {
			if (wFlags & DISPATCH_METHOD) {
				// line 610: [id(0x60020012)]HRESULT SetAlignedPosition([in] single x,[in] single y,[in] Alignment alignment);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var2;
				V_VT(&var2) = VT_EMPTY;
				VariantChangeType(&var2, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = SetAlignedPosition(V_R4(&var0), V_R4(&var1), (Alignment)V_I4(&var2));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
			}
			break;
		}
		case 0x60020013: {
			if (wFlags & DISPATCH_METHOD) {
				// line 615: [id(0x60020013)]HRESULT SetSize([in] single Width,[in] single Height);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = SetSize(V_R4(&var0), V_R4(&var1));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x60020014: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 619: [id(0x60020014), propget]HRESULT PrefWidth([out, retval] single* pRetVal);
				V_VT(&res) = VT_R4;
				hres = get_PrefWidth(&V_R4(&res));
			}
			break;
		}
		case 0x60020015: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 621: [id(0x60020015), propget]HRESULT PrefHeight([out, retval] single* pRetVal);
				V_VT(&res) = VT_R4;
				hres = get_PrefHeight(&V_R4(&res));
			}
			break;
		}
		case 0x60020016: {
			if (wFlags & DISPATCH_METHOD) {
				// line 623: [id(0x60020016)]HRESULT Pack();
				hres = Pack();
			}
			break;
		}
		case 0x60020017: {
			if (wFlags & DISPATCH_METHOD) {
				// line 625: [id(0x60020017)]HRESULT Remove();
				hres = Remove();
			}
			break;
		}
		case 0x60020018: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 627: [id(0x60020018), propget]HRESULT ActionFactory([out, retval] IActionFactory** pRetVal);
				V_VT(&res) = VT_DISPATCH;
				hres = get_ActionFactory((IActionFactory**)&V_DISPATCH(&res));
			}
			break;
		}
		case 0x60020019: {
			if (wFlags & DISPATCH_METHOD) {
				// line 629: [id(0x60020019)]HRESULT AddAction([in] IUnknown* action);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = AddAction(V_UNKNOWN(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x6002001a: {
			if (wFlags & DISPATCH_METHOD) {
				// line 631: [id(0x6002001a)]HRESULT ClearActions();
				hres = ClearActions();
			}
			break;
		}
		case 0x6002001f: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 637: [id(0x6002001f), propget]HRESULT Length([out, retval] single* pRetVal);
				V_VT(&res) = VT_R4;
				hres = get_Length(&V_R4(&res));
			}
			break;
		}
		case 0x60020020: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 639: [id(0x60020020), propget]HRESULT Loop([out, retval] VARIANT_BOOL* pRetVal);
				V_VT(&res) = VT_BOOL;
				hres = get_Loop(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 641: [id(0x60020020), propput]HRESULT Loop([in] VARIANT_BOOL pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_Loop(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x60020022: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 643: [id(0x60020022), propget]HRESULT Paused([out, retval] VARIANT_BOOL* pRetVal);
				V_VT(&res) = VT_BOOL;
				hres = get_Paused(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 645: [id(0x60020022), propput]HRESULT Paused([in] VARIANT_BOOL pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_Paused(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x60020024: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 647: [id(0x60020024), propget]HRESULT PlaySpeed([out, retval] single* pRetVal);
				V_VT(&res) = VT_R4;
				hres = get_PlaySpeed(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 649: [id(0x60020024), propput]HRESULT PlaySpeed([in] single pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_PlaySpeed(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x60020026: {
			if (wFlags & DISPATCH_METHOD) {
				// line 651: [id(0x60020026)]HRESULT Seek([in] single posInSeconds);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = Seek(V_R4(&var0));
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

STDMETHODIMP Label::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} namesIdsList[] = {
			{ NULL },
			{ L"ActionFactory", 0x60020018 },
			{ L"AddAction", 0x60020019 },
			{ L"AutoPack", 0x6002001b },
			{ L"ClearActions", 0x6002001a },
			{ L"ClearBackground", 0x6002000e },
			{ L"FillParent", 0x6002000c },
			{ L"Font", 0x6002001f },
			{ L"Height", 0x60020008 },
			{ L"Name", 0x60020000 },
			{ L"Pack", 0x60020016 },
			{ L"PrefHeight", 0x60020015 },
			{ L"PrefWidth", 0x60020014 },
			{ L"Remove", 0x60020017 },
			{ L"SetAlignedPosition", 0x60020012 },
			{ L"SetBounds", 0x60020010 },
			{ L"SetPosition", 0x60020011 },
			{ L"SetSize", 0x60020013 },
			{ L"Text", 0x60020021 },
			{ L"visible", 0x6002000a },
			{ L"Width", 0x60020006 },
			{ L"x", 0x60020002 },
			{ L"y", 0x60020004 }
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

STDMETHODIMP Label::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
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
				// line 665: [id(0x60020000), propget]HRESULT Name([out, retval] BSTR* pRetVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Name(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 667: [id(0x60020000), propput]HRESULT Name([in] BSTR pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Name(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x60020002: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 669: [id(0x60020002), propget]HRESULT x([out, retval] single* pRetVal);
				V_VT(&res) = VT_R4;
				hres = get_x(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 671: [id(0x60020002), propput]HRESULT x([in] single pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_x(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x60020004: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 673: [id(0x60020004), propget]HRESULT y([out, retval] single* pRetVal);
				V_VT(&res) = VT_R4;
				hres = get_y(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 675: [id(0x60020004), propput]HRESULT y([in] single pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_y(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x60020006: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 677: [id(0x60020006), propget]HRESULT Width([out, retval] single* pRetVal);
				V_VT(&res) = VT_R4;
				hres = get_Width(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 679: [id(0x60020006), propput]HRESULT Width([in] single pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Width(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x60020008: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 681: [id(0x60020008), propget]HRESULT Height([out, retval] single* pRetVal);
				V_VT(&res) = VT_R4;
				hres = get_Height(&V_R4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 683: [id(0x60020008), propput]HRESULT Height([in] single pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = put_Height(V_R4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x6002000a: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 685: [id(0x6002000a), propget]HRESULT visible([out, retval] VARIANT_BOOL* pRetVal);
				V_VT(&res) = VT_BOOL;
				hres = get_visible(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 687: [id(0x6002000a), propput]HRESULT visible([in] VARIANT_BOOL pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_visible(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x6002000c: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 689: [id(0x6002000c), propget]HRESULT FillParent([out, retval] VARIANT_BOOL* pRetVal);
				V_VT(&res) = VT_BOOL;
				hres = get_FillParent(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 691: [id(0x6002000c), propput]HRESULT FillParent([in] VARIANT_BOOL pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_FillParent(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x6002000e: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 693: [id(0x6002000e), propget]HRESULT ClearBackground([out, retval] VARIANT_BOOL* pRetVal);
				V_VT(&res) = VT_BOOL;
				hres = get_ClearBackground(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 695: [id(0x6002000e), propput]HRESULT ClearBackground([in] VARIANT_BOOL pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_ClearBackground(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x60020010: {
			if (wFlags & DISPATCH_METHOD) {
				// line 697: [id(0x60020010)]HRESULT SetBounds([in] single x,[in] single y,[in] single Width,[in] single Height);
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
				hres = SetBounds(V_R4(&var0), V_R4(&var1), V_R4(&var2), V_R4(&var3));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
				VariantClear(&var3);
			}
			break;
		}
		case 0x60020011: {
			if (wFlags & DISPATCH_METHOD) {
				// line 703: [id(0x60020011)]HRESULT SetPosition([in] single x,[in] single y);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = SetPosition(V_R4(&var0), V_R4(&var1));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x60020012: {
			if (wFlags & DISPATCH_METHOD) {
				// line 707: [id(0x60020012)]HRESULT SetAlignedPosition([in] single x,[in] single y,[in] Alignment alignment);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var2;
				V_VT(&var2) = VT_EMPTY;
				VariantChangeType(&var2, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = SetAlignedPosition(V_R4(&var0), V_R4(&var1), (Alignment)V_I4(&var2));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
			}
			break;
		}
		case 0x60020013: {
			if (wFlags & DISPATCH_METHOD) {
				// line 712: [id(0x60020013)]HRESULT SetSize([in] single Width,[in] single Height);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_R4);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_R4);
				hres = SetSize(V_R4(&var0), V_R4(&var1));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x60020014: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 716: [id(0x60020014), propget]HRESULT PrefWidth([out, retval] single* pRetVal);
				V_VT(&res) = VT_R4;
				hres = get_PrefWidth(&V_R4(&res));
			}
			break;
		}
		case 0x60020015: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 718: [id(0x60020015), propget]HRESULT PrefHeight([out, retval] single* pRetVal);
				V_VT(&res) = VT_R4;
				hres = get_PrefHeight(&V_R4(&res));
			}
			break;
		}
		case 0x60020016: {
			if (wFlags & DISPATCH_METHOD) {
				// line 720: [id(0x60020016)]HRESULT Pack();
				hres = Pack();
			}
			break;
		}
		case 0x60020017: {
			if (wFlags & DISPATCH_METHOD) {
				// line 722: [id(0x60020017)]HRESULT Remove();
				hres = Remove();
			}
			break;
		}
		case 0x60020018: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 724: [id(0x60020018), propget]HRESULT ActionFactory([out, retval] IActionFactory** pRetVal);
				V_VT(&res) = VT_DISPATCH;
				hres = get_ActionFactory((IActionFactory**)&V_DISPATCH(&res));
			}
			break;
		}
		case 0x60020019: {
			if (wFlags & DISPATCH_METHOD) {
				// line 726: [id(0x60020019)]HRESULT AddAction([in] IUnknown* action);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = AddAction(V_UNKNOWN(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x6002001a: {
			if (wFlags & DISPATCH_METHOD) {
				// line 728: [id(0x6002001a)]HRESULT ClearActions();
				hres = ClearActions();
			}
			break;
		}
		case 0x6002001b: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 730: [id(0x6002001b), propget]HRESULT AutoPack([out, retval] VARIANT_BOOL* pRetVal);
				V_VT(&res) = VT_BOOL;
				hres = get_AutoPack(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 732: [id(0x6002001b), propput]HRESULT AutoPack([in] VARIANT_BOOL pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_AutoPack(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x6002001f: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 736: [id(0x6002001f), propget]HRESULT Font([out, retval] IUnknown** pRetVal);
				V_VT(&res) = VT_UNKNOWN;
				hres = get_Font(&V_UNKNOWN(&res));
			}
			else if ((wFlags & DISPATCH_PROPERTYPUT) || (wFlags & DISPATCH_PROPERTYPUTREF)) {
				// line 738: [id(0x6002001f), propputref]HRESULT Font([in] IUnknown* pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = putref_Font(V_UNKNOWN(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x60020021: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 740: [id(0x60020021), propget]HRESULT Text([out, retval] BSTR* pRetVal);
				V_VT(&res) = VT_BSTR;
				hres = get_Text(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 742: [id(0x60020021), propput]HRESULT Text([in] BSTR pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_Text(V_BSTR(&var0));
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

STDMETHODIMP FlexDMD::GetIDsOfNames(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} namesIdsList[] = {
			{ NULL },
			{ L"Clear", 0x60020015 },
			{ L"Color", 0x6002000d },
			{ L"DmdColoredPixels", 0x60020017 },
			{ L"DmdPixels", 0x60020018 },
			{ L"GameName", 0x60020007 },
			{ L"Height", 0x6002000b },
			{ L"LockRenderThread", 0x6002001b },
			{ L"NewFont", 0x60020022 },
			{ L"NewFrame", 0x6002001e },
			{ L"NewGroup", 0x6002001d },
			{ L"NewImage", 0x60020021 },
			{ L"NewLabel", 0x6002001f },
			{ L"NewUltraDMD", 0x60020023 },
			{ L"NewVideo", 0x60020020 },
			{ L"ProjectFolder", 0x60020011 },
			{ L"RenderMode", 0x6002000f },
			{ L"Run", 0x60020003 },
			{ L"RuntimeVersion", 0x60020001 },
			{ L"Segments", 0x60020019 },
			{ L"Show", 0x60020005 },
			{ L"Stage", 0x6002001a },
			{ L"TableFile", 0x60020013 },
			{ L"UnlockRenderThread", 0x6002001c },
			{ L"Version", 0x60020000 },
			{ L"Width", 0x60020009 }
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

STDMETHODIMP FlexDMD::Invoke(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
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
				// line 779: [id(0x60020000), propget]HRESULT Version([out, retval] long* pRetVal);
				V_VT(&res) = VT_I4;
				hres = get_Version((LONG*)&V_I4(&res));
			}
			break;
		}
		case 0x60020001: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 781: [id(0x60020001), propget]HRESULT RuntimeVersion([out, retval] long* pRetVal);
				V_VT(&res) = VT_I4;
				hres = get_RuntimeVersion((LONG*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 783: [id(0x60020001), propput]HRESULT RuntimeVersion([in] long pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_RuntimeVersion(V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x60020003: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 785: [id(0x60020003), propget]HRESULT Run([out, retval] VARIANT_BOOL* pRetVal);
				V_VT(&res) = VT_BOOL;
				hres = get_Run(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 787: [id(0x60020003), propput]HRESULT Run([in] VARIANT_BOOL pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_Run(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x60020005: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 789: [id(0x60020005), propget]HRESULT Show([out, retval] VARIANT_BOOL* pRetVal);
				V_VT(&res) = VT_BOOL;
				hres = get_Show(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 791: [id(0x60020005), propput]HRESULT Show([in] VARIANT_BOOL pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_Show(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x60020007: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 793: [id(0x60020007), propget]HRESULT GameName([out, retval] BSTR* pRetVal);
				V_VT(&res) = VT_BSTR;
				hres = get_GameName(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 795: [id(0x60020007), propput]HRESULT GameName([in] BSTR pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_GameName(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x60020009: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 797: [id(0x60020009), propget]HRESULT Width([out, retval] unsigned short* pRetVal);
				V_VT(&res) = VT_UI2;
				hres = get_Width((unsigned short*)&V_UI2(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 799: [id(0x60020009), propput]HRESULT Width([in] unsigned short pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_UI2);
				hres = put_Width(V_UI2(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x6002000b: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 801: [id(0x6002000b), propget]HRESULT Height([out, retval] unsigned short* pRetVal);
				V_VT(&res) = VT_UI2;
				hres = get_Height((unsigned short*)&V_UI2(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 803: [id(0x6002000b), propput]HRESULT Height([in] unsigned short pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_UI2);
				hres = put_Height(V_UI2(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x6002000d: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 805: [id(0x6002000d), propget]HRESULT Color([out, retval] OLE_COLOR* pRetVal);
				V_VT(&res) = VT_UI4;
				hres = get_Color(&V_UI4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 807: [id(0x6002000d), propput]HRESULT Color([in] OLE_COLOR pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_UI4);
				hres = put_Color((OLE_COLOR)V_UI4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x6002000f: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 809: [id(0x6002000f), propget]HRESULT RenderMode([out, retval] RenderMode* pRetVal);
				V_VT(&res) = VT_I4;
				hres = get_RenderMode((RenderMode*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 811: [id(0x6002000f), propput]HRESULT RenderMode([in] RenderMode pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = put_RenderMode((RenderMode)V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x60020011: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 813: [id(0x60020011), propget]HRESULT ProjectFolder([out, retval] BSTR* pRetVal);
				V_VT(&res) = VT_BSTR;
				hres = get_ProjectFolder(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 815: [id(0x60020011), propput]HRESULT ProjectFolder([in] BSTR pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_ProjectFolder(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x60020013: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 817: [id(0x60020013), propget]HRESULT TableFile([out, retval] BSTR* pRetVal);
				V_VT(&res) = VT_BSTR;
				hres = get_TableFile(&V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 819: [id(0x60020013), propput]HRESULT TableFile([in] BSTR pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = put_TableFile(V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x60020015: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 821: [id(0x60020015), propget]HRESULT Clear([out, retval] VARIANT_BOOL* pRetVal);
				V_VT(&res) = VT_BOOL;
				hres = get_Clear(&V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 823: [id(0x60020015), propput]HRESULT Clear([in] VARIANT_BOOL pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = put_Clear(V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x60020017: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 825: [id(0x60020017), propget]HRESULT DmdColoredPixels([out, retval] VARIANT* pRetVal);
				hres = get_DmdColoredPixels(&res);
			}
			break;
		}
		case 0x60020018: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 827: [id(0x60020018), propget]HRESULT DmdPixels([out, retval] VARIANT* pRetVal);
				hres = get_DmdPixels(&res);
			}
			break;
		}
		case 0x60020019: {
			if (wFlags & DISPATCH_PROPERTYPUT) {
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = putref_Segments(var0);
				VariantClear(&var0);
			}
			else if (wFlags & DISPATCH_PROPERTYPUTREF) {
				// line 829: [id(0x60020019), propputref]HRESULT Segments([in] VARIANT rhs);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = putref_Segments(var0);
				VariantClear(&var0);
			}
			break;
		}
		case 0x6002001a: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 831: [id(0x6002001a), propget]HRESULT Stage([out, retval] IGroupActor** pRetVal);
				V_VT(&res) = VT_DISPATCH;
				hres = get_Stage((IGroupActor**)&V_DISPATCH(&res));
			}
			break;
		}
		case 0x6002001b: {
			if (wFlags & DISPATCH_METHOD) {
				// line 833: [id(0x6002001b)]HRESULT LockRenderThread();
				hres = LockRenderThread();
			}
			break;
		}
		case 0x6002001c: {
			if (wFlags & DISPATCH_METHOD) {
				// line 835: [id(0x6002001c)]HRESULT UnlockRenderThread();
				hres = UnlockRenderThread();
			}
			break;
		}
		case 0x6002001d: {
			if (wFlags & DISPATCH_METHOD) {
				// line 837: [id(0x6002001d)]HRESULT NewGroup([in] BSTR Name,[out, retval] IGroupActor** pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				V_VT(&res) = VT_DISPATCH;
				hres = NewGroup(V_BSTR(&var0), (IGroupActor**)&V_DISPATCH(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 0x6002001e: {
			if (wFlags & DISPATCH_METHOD) {
				// line 841: [id(0x6002001e)]HRESULT NewFrame([in] BSTR Name,[out, retval] IFrameActor** pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				V_VT(&res) = VT_DISPATCH;
				hres = NewFrame(V_BSTR(&var0), (IFrameActor**)&V_DISPATCH(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 0x6002001f: {
			if (wFlags & DISPATCH_METHOD) {
				// line 845: [id(0x6002001f)]HRESULT NewLabel([in] BSTR Name,[in] IUnknown* Font,[in] BSTR Text,[out, retval] ILabelActor** pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantCopyInd(&var1, &pDispParams->rgvarg[--index]);
				VARIANT var2;
				V_VT(&var2) = VT_EMPTY;
				VariantChangeType(&var2, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				V_VT(&res) = VT_DISPATCH;
				hres = NewLabel(V_BSTR(&var0), V_UNKNOWN(&var1), V_BSTR(&var2), (ILabelActor**)&V_DISPATCH(&res));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
			}
			break;
		}
		case 0x60020020: {
			if (wFlags & DISPATCH_METHOD) {
				// line 851: [id(0x60020020)]HRESULT NewVideo([in] BSTR Name,[in] BSTR video,[out, retval] IVideoActor** pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				V_VT(&res) = VT_DISPATCH;
				hres = NewVideo(V_BSTR(&var0), V_BSTR(&var1), (IVideoActor**)&V_DISPATCH(&res));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x60020021: {
			if (wFlags & DISPATCH_METHOD) {
				// line 856: [id(0x60020021)]HRESULT NewImage([in] BSTR Name,[in] BSTR image,[out, retval] IImageActor** pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				V_VT(&res) = VT_DISPATCH;
				hres = NewImage(V_BSTR(&var0), V_BSTR(&var1), (IImageActor**)&V_DISPATCH(&res));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x60020022: {
			if (wFlags & DISPATCH_METHOD) {
				// line 861: [id(0x60020022)]HRESULT NewFont([in] BSTR Font,[in] OLE_COLOR tint,[in] OLE_COLOR borderTint,[in] long borderSize,[out, retval] IUnknown** pRetVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_UI4);
				VARIANT var2;
				V_VT(&var2) = VT_EMPTY;
				VariantChangeType(&var2, &pDispParams->rgvarg[--index], 0, VT_UI4);
				VARIANT var3;
				V_VT(&var3) = VT_EMPTY;
				VariantChangeType(&var3, &pDispParams->rgvarg[--index], 0, VT_I4);
				V_VT(&res) = VT_UNKNOWN;
				hres = NewFont(V_BSTR(&var0), (OLE_COLOR)V_UI4(&var1), (OLE_COLOR)V_UI4(&var2), V_I4(&var3), &V_UNKNOWN(&res));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
				VariantClear(&var3);
			}
			break;
		}
		case 0x60020023: {
			if (wFlags & DISPATCH_METHOD) {
				// line 868: [id(0x60020023)]HRESULT NewUltraDMD([out, retval] IUltraDMD** pRetVal);
				V_VT(&res) = VT_DISPATCH;
				hres = NewUltraDMD((IUltraDMD**)&V_DISPATCH(&res));
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