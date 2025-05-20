void external_log_info(const char* format, ...);

static HRESULT WINAPI RegExp_GetIDsOfNames(IRegExp *iface, REFIID riid, LPOLESTR *rgszNames,
		UINT cNames, LCID lcid, DISPID *rgDispId)
{
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} names_ids_list[] = {
			{ NULL },
			{ L"Execute", DISPID_REGEXP_EXECUTE },
			{ L"Global", DISPID_REGEXP_GLOBAL },
			{ L"IgnoreCase", DISPID_REGEXP_IGNORECASE },
			{ L"Pattern", DISPID_REGEXP_PATTERN },
			{ L"Replace", DISPID_REGEXP_REPLACE },
			{ L"Test", DISPID_REGEXP_TEST }
	};

	size_t min = 1, max = ARRAY_SIZE(names_ids_list) - 1, i;
	int r;
	while(min <= max) {
		i = (min + max) / 2;
		r = wcsicmp(names_ids_list[i].name, *rgszNames);
		if (!r) {
			*rgDispId = names_ids_list[i].dispId;
			return S_OK;
		}
		if (r < 0)
			min = i+1;
		else
			max = i-1;
	}
	return DISP_E_MEMBERNOTFOUND;
}

static HRESULT WINAPI RegExp_Invoke(IRegExp *iface, DISPID dispIdMember,
		REFIID riid, LCID lcid, WORD wFlags,
		DISPPARAMS *pDispParams, VARIANT *pVarResult,
		EXCEPINFO *pExcepInfo, UINT *puArgErr)
{
	int index = pDispParams->cArgs;
	VARIANT res;
	HRESULT hres = DISP_E_UNKNOWNNAME;

	V_VT(&res) = VT_EMPTY;

	switch(dispIdMember) {
		case DISPID_VALUE: {
			if (wFlags == (DISPATCH_METHOD | DISPATCH_PROPERTYGET)) {
				// Default method
				V_VT(&res) = VT_DISPATCH;
				V_DISPATCH(&res) = (IDispatch*)iface;
				hres = S_OK;
			}
			break;
		}
		case DISPID_REGEXP_PATTERN: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 45: [id(DISPID_REGEXP_PATTERN), propget]HRESULT Pattern([out, retval] BSTR *pPattern);
				V_VT(&res) = VT_BSTR;
				hres = RegExp_get_Pattern(iface, &V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 48: [id(DISPID_REGEXP_PATTERN), propput]HRESULT Pattern([in] BSTR pPattern);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = RegExp_put_Pattern(iface, V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_REGEXP_IGNORECASE: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 51: [id(DISPID_REGEXP_IGNORECASE), propget]HRESULT IgnoreCase([out, retval] VARIANT_BOOL *pIgnoreCase);
				V_VT(&res) = VT_BOOL;
				hres = RegExp_get_IgnoreCase(iface, &V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 54: [id(DISPID_REGEXP_IGNORECASE), propput]HRESULT IgnoreCase([in] VARIANT_BOOL pIgnoreCase);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = RegExp_put_IgnoreCase(iface, V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_REGEXP_GLOBAL: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 57: [id(DISPID_REGEXP_GLOBAL), propget]HRESULT Global([out, retval] VARIANT_BOOL *pGlobal);
				V_VT(&res) = VT_BOOL;
				hres = RegExp_get_Global(iface, &V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 60: [id(DISPID_REGEXP_GLOBAL), propput]HRESULT Global([in] VARIANT_BOOL pGlobal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = RegExp_put_Global(iface, V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_REGEXP_EXECUTE: {
			if (wFlags & DISPATCH_METHOD) {
				// line 63: [id(DISPID_REGEXP_EXECUTE)]HRESULT Execute([in] BSTR sourceString,[out, retval] IDispatch **ppMatches);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				V_VT(&res) = VT_DISPATCH;
				hres = RegExp_Execute(iface, V_BSTR(&var0), (IDispatch**)&V_DISPATCH(&res));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_REGEXP_TEST: {
			if (wFlags & DISPATCH_METHOD) {
				// line 68: [id(DISPID_REGEXP_TEST)]HRESULT Test([in] BSTR sourceString,[out, retval] VARIANT_BOOL *pMatch);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				V_VT(&res) = VT_BOOL;
				hres = RegExp_Test(iface, V_BSTR(&var0), &V_BOOL(&res));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_REGEXP_REPLACE: {
			if (wFlags & DISPATCH_METHOD) {
				// line 73: [id(DISPID_REGEXP_REPLACE)]HRESULT Replace([in] BSTR sourceString,[in] BSTR replaceString,[out, retval] BSTR *pDestString);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				V_VT(&res) = VT_BSTR;
				hres = RegExp_Replace(iface, V_BSTR(&var0), V_BSTR(&var1), &V_BSTR(&res));
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
		external_log_info("RegExp_Invoke: dispId=%d (0x%08x), wFlags=%d, hres=%d", dispIdMember, dispIdMember, wFlags, hres);
	}
	return hres;
}

static HRESULT WINAPI RegExp2_GetIDsOfNames(IRegExp2 *iface, REFIID riid, LPOLESTR *rgszNames,
		UINT cNames, LCID lcid, DISPID *rgDispId)
{
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} names_ids_list[] = {
			{ NULL },
			{ L"Execute", DISPID_REGEXP_EXECUTE },
			{ L"Global", DISPID_REGEXP_GLOBAL },
			{ L"IgnoreCase", DISPID_REGEXP_IGNORECASE },
			{ L"Multiline", DISPID_REGEXP_MULTILINE },
			{ L"Pattern", DISPID_REGEXP_PATTERN },
			{ L"Replace", DISPID_REGEXP_REPLACE },
			{ L"Test", DISPID_REGEXP_TEST }
	};

	size_t min = 1, max = ARRAY_SIZE(names_ids_list) - 1, i;
	int r;
	while(min <= max) {
		i = (min + max) / 2;
		r = wcsicmp(names_ids_list[i].name, *rgszNames);
		if (!r) {
			*rgDispId = names_ids_list[i].dispId;
			return S_OK;
		}
		if (r < 0)
			min = i+1;
		else
			max = i-1;
	}
	return DISP_E_MEMBERNOTFOUND;
}

static HRESULT WINAPI RegExp2_Invoke(IRegExp2 *iface, DISPID dispIdMember,
		REFIID riid, LCID lcid, WORD wFlags,
		DISPPARAMS *pDispParams, VARIANT *pVarResult,
		EXCEPINFO *pExcepInfo, UINT *puArgErr)
{
	int index = pDispParams->cArgs;
	VARIANT res;
	HRESULT hres = DISP_E_UNKNOWNNAME;

	V_VT(&res) = VT_EMPTY;

	switch(dispIdMember) {
		case DISPID_VALUE: {
			if (wFlags == (DISPATCH_METHOD | DISPATCH_PROPERTYGET)) {
				// Default method
				V_VT(&res) = VT_DISPATCH;
				V_DISPATCH(&res) = (IDispatch*)iface;
				hres = S_OK;
			}
			break;
		}
		case DISPID_REGEXP_PATTERN: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 90: [id(DISPID_REGEXP_PATTERN), propget]HRESULT Pattern([out, retval] BSTR *pPattern);
				V_VT(&res) = VT_BSTR;
				hres = RegExp2_get_Pattern(iface, &V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 93: [id(DISPID_REGEXP_PATTERN), propput]HRESULT Pattern([in] BSTR pPattern);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = RegExp2_put_Pattern(iface, V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_REGEXP_IGNORECASE: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 96: [id(DISPID_REGEXP_IGNORECASE), propget]HRESULT IgnoreCase([out, retval] VARIANT_BOOL *pIgnoreCase);
				V_VT(&res) = VT_BOOL;
				hres = RegExp2_get_IgnoreCase(iface, &V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 99: [id(DISPID_REGEXP_IGNORECASE), propput]HRESULT IgnoreCase([in] VARIANT_BOOL pIgnoreCase);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = RegExp2_put_IgnoreCase(iface, V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_REGEXP_GLOBAL: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 102: [id(DISPID_REGEXP_GLOBAL), propget]HRESULT Global([out, retval] VARIANT_BOOL *pGlobal);
				V_VT(&res) = VT_BOOL;
				hres = RegExp2_get_Global(iface, &V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 105: [id(DISPID_REGEXP_GLOBAL), propput]HRESULT Global([in] VARIANT_BOOL pGlobal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = RegExp2_put_Global(iface, V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_REGEXP_MULTILINE: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 108: [id(DISPID_REGEXP_MULTILINE), propget]HRESULT Multiline([out, retval] VARIANT_BOOL *pMultiline);
				V_VT(&res) = VT_BOOL;
				hres = RegExp2_get_Multiline(iface, &V_BOOL(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 111: [id(DISPID_REGEXP_MULTILINE), propput]HRESULT Multiline([in] VARIANT_BOOL pMultiline);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BOOL);
				hres = RegExp2_put_Multiline(iface, V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_REGEXP_EXECUTE: {
			if (wFlags & DISPATCH_METHOD) {
				// line 114: [id(DISPID_REGEXP_EXECUTE)]HRESULT Execute([in] BSTR sourceString,[out, retval] IDispatch **ppMatches);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				V_VT(&res) = VT_DISPATCH;
				hres = RegExp2_Execute(iface, V_BSTR(&var0), (IDispatch**)&V_DISPATCH(&res));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_REGEXP_TEST: {
			if (wFlags & DISPATCH_METHOD) {
				// line 119: [id(DISPID_REGEXP_TEST)]HRESULT Test([in] BSTR sourceString,[out, retval] VARIANT_BOOL *pMatch);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				V_VT(&res) = VT_BOOL;
				hres = RegExp2_Test(iface, V_BSTR(&var0), &V_BOOL(&res));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_REGEXP_REPLACE: {
			if (wFlags & DISPATCH_METHOD) {
				// line 124: [id(DISPID_REGEXP_REPLACE)]HRESULT Replace([in] BSTR sourceString,[in] VARIANT replaceVar,[out, retval] BSTR *pDestString);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_VARIANT);
				V_VT(&res) = VT_BSTR;
				hres = RegExp2_Replace(iface, V_BSTR(&var0), var1, &V_BSTR(&res));
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
		external_log_info("RegExp2_Invoke: dispId=%d (0x%08x), wFlags=%d, hres=%d", dispIdMember, dispIdMember, wFlags, hres);
	}
	return hres;
}

static HRESULT WINAPI Match_GetIDsOfNames(IMatch *iface, REFIID riid, LPOLESTR *rgszNames,
		UINT cNames, LCID lcid, DISPID *rgDispId)
{
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} names_ids_list[] = {
			{ NULL },
			{ L"FirstIndex", DISPID_MATCH_FIRSTINDEX },
			{ L"Length", DISPID_MATCH_LENGTH },
			{ L"Value", DISPID_VALUE }
	};

	size_t min = 1, max = ARRAY_SIZE(names_ids_list) - 1, i;
	int r;
	while(min <= max) {
		i = (min + max) / 2;
		r = wcsicmp(names_ids_list[i].name, *rgszNames);
		if (!r) {
			*rgDispId = names_ids_list[i].dispId;
			return S_OK;
		}
		if (r < 0)
			min = i+1;
		else
			max = i-1;
	}
	return DISP_E_MEMBERNOTFOUND;
}

static HRESULT WINAPI Match_Invoke(IMatch *iface, DISPID dispIdMember,
		REFIID riid, LCID lcid, WORD wFlags,
		DISPPARAMS *pDispParams, VARIANT *pVarResult,
		EXCEPINFO *pExcepInfo, UINT *puArgErr)
{
	int index = pDispParams->cArgs;
	VARIANT res;
	HRESULT hres = DISP_E_UNKNOWNNAME;

	V_VT(&res) = VT_EMPTY;

	switch(dispIdMember) {
		case DISPID_VALUE: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 141: [id(DISPID_VALUE), propget]HRESULT Value([out, retval] BSTR *pValue);
				V_VT(&res) = VT_BSTR;
				hres = Match_get_Value(iface, &V_BSTR(&res));
			}
			else if (wFlags == (DISPATCH_METHOD | DISPATCH_PROPERTYGET)) {
				// Default method
				V_VT(&res) = VT_DISPATCH;
				V_DISPATCH(&res) = (IDispatch*)iface;
				hres = S_OK;
			}
			break;
		}
		case DISPID_MATCH_FIRSTINDEX: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 144: [id(DISPID_MATCH_FIRSTINDEX), propget]HRESULT FirstIndex([out, retval] LONG *pFirstIndex);
				V_VT(&res) = VT_I4;
				hres = Match_get_FirstIndex(iface, (LONG*)&V_I4(&res));
			}
			break;
		}
		case DISPID_MATCH_LENGTH: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 147: [id(DISPID_MATCH_LENGTH), propget]HRESULT Length([out, retval] LONG *pLength);
				V_VT(&res) = VT_I4;
				hres = Match_get_Length(iface, (LONG*)&V_I4(&res));
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
		external_log_info("Match_Invoke: dispId=%d (0x%08x), wFlags=%d, hres=%d", dispIdMember, dispIdMember, wFlags, hres);
	}
	return hres;
}

static HRESULT WINAPI Match2_GetIDsOfNames(IMatch2 *iface, REFIID riid, LPOLESTR *rgszNames,
		UINT cNames, LCID lcid, DISPID *rgDispId)
{
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} names_ids_list[] = {
			{ NULL },
			{ L"FirstIndex", DISPID_MATCH_FIRSTINDEX },
			{ L"Length", DISPID_MATCH_LENGTH },
			{ L"SubMatches", DISPID_MATCH_SUBMATCHES },
			{ L"Value", DISPID_VALUE }
	};

	size_t min = 1, max = ARRAY_SIZE(names_ids_list) - 1, i;
	int r;
	while(min <= max) {
		i = (min + max) / 2;
		r = wcsicmp(names_ids_list[i].name, *rgszNames);
		if (!r) {
			*rgDispId = names_ids_list[i].dispId;
			return S_OK;
		}
		if (r < 0)
			min = i+1;
		else
			max = i-1;
	}
	return DISP_E_MEMBERNOTFOUND;
}

static HRESULT WINAPI Match2_Invoke(IMatch2 *iface, DISPID dispIdMember,
		REFIID riid, LCID lcid, WORD wFlags,
		DISPPARAMS *pDispParams, VARIANT *pVarResult,
		EXCEPINFO *pExcepInfo, UINT *puArgErr)
{
	int index = pDispParams->cArgs;
	VARIANT res;
	HRESULT hres = DISP_E_UNKNOWNNAME;

	V_VT(&res) = VT_EMPTY;

	switch(dispIdMember) {
		case DISPID_VALUE: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 161: [id(DISPID_VALUE), propget]HRESULT Value([out, retval] BSTR *pValue);
				V_VT(&res) = VT_BSTR;
				hres = Match2_get_Value(iface, &V_BSTR(&res));
			}
			else if (wFlags == (DISPATCH_METHOD | DISPATCH_PROPERTYGET)) {
				// Default method
				V_VT(&res) = VT_DISPATCH;
				V_DISPATCH(&res) = (IDispatch*)iface;
				hres = S_OK;
			}
			break;
		}
		case DISPID_MATCH_FIRSTINDEX: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 164: [id(DISPID_MATCH_FIRSTINDEX), propget]HRESULT FirstIndex([out, retval] LONG *pFirstIndex);
				V_VT(&res) = VT_I4;
				hres = Match2_get_FirstIndex(iface, (LONG*)&V_I4(&res));
			}
			break;
		}
		case DISPID_MATCH_LENGTH: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 167: [id(DISPID_MATCH_LENGTH), propget]HRESULT Length([out, retval] LONG *pLength);
				V_VT(&res) = VT_I4;
				hres = Match2_get_Length(iface, (LONG*)&V_I4(&res));
			}
			break;
		}
		case DISPID_MATCH_SUBMATCHES: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 170: [id(DISPID_MATCH_SUBMATCHES), propget]HRESULT SubMatches([out, retval] IDispatch **ppSubMatches);
				V_VT(&res) = VT_DISPATCH;
				hres = Match2_get_SubMatches(iface, (IDispatch**)&V_DISPATCH(&res));
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
		external_log_info("Match2_Invoke: dispId=%d (0x%08x), wFlags=%d, hres=%d", dispIdMember, dispIdMember, wFlags, hres);
	}
	return hres;
}

static HRESULT WINAPI MatchCollection_GetIDsOfNames(IMatchCollection *iface, REFIID riid, LPOLESTR *rgszNames,
		UINT cNames, LCID lcid, DISPID *rgDispId)
{
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} names_ids_list[] = {
			{ NULL },
			{ L"Count", DISPID_MATCHCOLLECTION_COUNT },
			{ L"Item", DISPID_VALUE }
	};

	size_t min = 1, max = ARRAY_SIZE(names_ids_list) - 1, i;
	int r;
	while(min <= max) {
		i = (min + max) / 2;
		r = wcsicmp(names_ids_list[i].name, *rgszNames);
		if (!r) {
			*rgDispId = names_ids_list[i].dispId;
			return S_OK;
		}
		if (r < 0)
			min = i+1;
		else
			max = i-1;
	}
	return DISP_E_MEMBERNOTFOUND;
}

static HRESULT WINAPI MatchCollection_Invoke(IMatchCollection *iface, DISPID dispIdMember,
		REFIID riid, LCID lcid, WORD wFlags,
		DISPPARAMS *pDispParams, VARIANT *pVarResult,
		EXCEPINFO *pExcepInfo, UINT *puArgErr)
{
	int index = pDispParams->cArgs;
	VARIANT res;
	HRESULT hres = DISP_E_UNKNOWNNAME;

	V_VT(&res) = VT_EMPTY;

	switch(dispIdMember) {
		case DISPID_VALUE: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 184: [id(DISPID_VALUE), propget]HRESULT Item([in] LONG index,[out, retval] IDispatch **ppMatch);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				V_VT(&res) = VT_DISPATCH;
				hres = MatchCollection_get_Item(iface, V_I4(&var0), (IDispatch**)&V_DISPATCH(&res));
				VariantClear(&var0);
			}
			else if (wFlags == (DISPATCH_METHOD | DISPATCH_PROPERTYGET)) {
				// Default method
				V_VT(&res) = VT_DISPATCH;
				V_DISPATCH(&res) = (IDispatch*)iface;
				hres = S_OK;
			}
			break;
		}
		case DISPID_MATCHCOLLECTION_COUNT: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 189: [id(DISPID_MATCHCOLLECTION_COUNT), propget]HRESULT Count([out, retval] LONG *pCount);
				V_VT(&res) = VT_I4;
				hres = MatchCollection_get_Count(iface, (LONG*)&V_I4(&res));
			}
			break;
		}
		case DISPID_NEWENUM: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 192: [id(DISPID_NEWENUM), propget]HRESULT _NewEnum([out, retval] IUnknown **ppEnum);
				V_VT(&res) = VT_UNKNOWN;
				hres = MatchCollection_get__NewEnum(iface, &V_UNKNOWN(&res));
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
		external_log_info("MatchCollection_Invoke: dispId=%d (0x%08x), wFlags=%d, hres=%d", dispIdMember, dispIdMember, wFlags, hres);
	}
	return hres;
}

static HRESULT WINAPI MatchCollection2_GetIDsOfNames(IMatchCollection2 *iface, REFIID riid, LPOLESTR *rgszNames,
		UINT cNames, LCID lcid, DISPID *rgDispId)
{
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} names_ids_list[] = {
			{ NULL },
			{ L"Count", DISPID_MATCHCOLLECTION_COUNT },
			{ L"Item", DISPID_VALUE }
	};

	size_t min = 1, max = ARRAY_SIZE(names_ids_list) - 1, i;
	int r;
	while(min <= max) {
		i = (min + max) / 2;
		r = wcsicmp(names_ids_list[i].name, *rgszNames);
		if (!r) {
			*rgDispId = names_ids_list[i].dispId;
			return S_OK;
		}
		if (r < 0)
			min = i+1;
		else
			max = i-1;
	}
	return DISP_E_MEMBERNOTFOUND;
}

static HRESULT WINAPI MatchCollection2_Invoke(IMatchCollection2 *iface, DISPID dispIdMember,
		REFIID riid, LCID lcid, WORD wFlags,
		DISPPARAMS *pDispParams, VARIANT *pVarResult,
		EXCEPINFO *pExcepInfo, UINT *puArgErr)
{
	int index = pDispParams->cArgs;
	VARIANT res;
	HRESULT hres = DISP_E_UNKNOWNNAME;

	V_VT(&res) = VT_EMPTY;

	switch(dispIdMember) {
		case DISPID_VALUE: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 206: [id(DISPID_VALUE), propget]HRESULT Item([in] LONG index,[out, retval] IDispatch **ppMatch);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				V_VT(&res) = VT_DISPATCH;
				hres = MatchCollection2_get_Item(iface, V_I4(&var0), (IDispatch**)&V_DISPATCH(&res));
				VariantClear(&var0);
			}
			else if (wFlags == (DISPATCH_METHOD | DISPATCH_PROPERTYGET)) {
				// Default method
				V_VT(&res) = VT_DISPATCH;
				V_DISPATCH(&res) = (IDispatch*)iface;
				hres = S_OK;
			}
			break;
		}
		case DISPID_MATCHCOLLECTION_COUNT: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 211: [id(DISPID_MATCHCOLLECTION_COUNT), propget]HRESULT Count([out, retval] LONG *pCount);
				V_VT(&res) = VT_I4;
				hres = MatchCollection2_get_Count(iface, (LONG*)&V_I4(&res));
			}
			break;
		}
		case DISPID_NEWENUM: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 214: [id(DISPID_NEWENUM), propget]HRESULT _NewEnum([out, retval] IUnknown **ppEnum);
				V_VT(&res) = VT_UNKNOWN;
				hres = MatchCollection2_get__NewEnum(iface, &V_UNKNOWN(&res));
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
		external_log_info("MatchCollection2_Invoke: dispId=%d (0x%08x), wFlags=%d, hres=%d", dispIdMember, dispIdMember, wFlags, hres);
	}
	return hres;
}

static HRESULT WINAPI SubMatches_GetIDsOfNames(ISubMatches *iface, REFIID riid, LPOLESTR *rgszNames,
		UINT cNames, LCID lcid, DISPID *rgDispId)
{
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} names_ids_list[] = {
			{ NULL },
			{ L"Count", DISPID_SUBMATCHES_COUNT },
			{ L"Item", DISPID_VALUE }
	};

	size_t min = 1, max = ARRAY_SIZE(names_ids_list) - 1, i;
	int r;
	while(min <= max) {
		i = (min + max) / 2;
		r = wcsicmp(names_ids_list[i].name, *rgszNames);
		if (!r) {
			*rgDispId = names_ids_list[i].dispId;
			return S_OK;
		}
		if (r < 0)
			min = i+1;
		else
			max = i-1;
	}
	return DISP_E_MEMBERNOTFOUND;
}

static HRESULT WINAPI SubMatches_Invoke(ISubMatches *iface, DISPID dispIdMember,
		REFIID riid, LCID lcid, WORD wFlags,
		DISPPARAMS *pDispParams, VARIANT *pVarResult,
		EXCEPINFO *pExcepInfo, UINT *puArgErr)
{
	int index = pDispParams->cArgs;
	VARIANT res;
	HRESULT hres = DISP_E_UNKNOWNNAME;

	V_VT(&res) = VT_EMPTY;

	switch(dispIdMember) {
		case DISPID_VALUE: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 228: [id(DISPID_VALUE), propget]HRESULT Item([in] LONG index,[out, retval] VARIANT *pSubMatch);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = SubMatches_get_Item(iface, V_I4(&var0), &res);
				VariantClear(&var0);
			}
			else if (wFlags == (DISPATCH_METHOD | DISPATCH_PROPERTYGET)) {
				// Default method
				V_VT(&res) = VT_DISPATCH;
				V_DISPATCH(&res) = (IDispatch*)iface;
				hres = S_OK;
			}
			break;
		}
		case DISPID_SUBMATCHES_COUNT: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 233: [id(DISPID_SUBMATCHES_COUNT), propget]HRESULT Count([out, retval] LONG *pCount);
				V_VT(&res) = VT_I4;
				hres = SubMatches_get_Count(iface, (LONG*)&V_I4(&res));
			}
			break;
		}
		case DISPID_NEWENUM: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 236: [id(DISPID_NEWENUM), propget]HRESULT _NewEnum([out, retval] IUnknown **ppEnum);
				V_VT(&res) = VT_UNKNOWN;
				hres = SubMatches_get__NewEnum(iface, &V_UNKNOWN(&res));
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
		external_log_info("SubMatches_Invoke: dispId=%d (0x%08x), wFlags=%d, hres=%d", dispIdMember, dispIdMember, wFlags, hres);
	}
	return hres;
}

