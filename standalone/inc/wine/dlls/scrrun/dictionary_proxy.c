void external_log_info(const char* format, ...);

static HRESULT WINAPI dictionary_GetIDsOfNames(IDictionary *iface, REFIID riid, LPOLESTR *rgszNames,
                UINT cNames, LCID lcid, DISPID *rgDispId)
{
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} names_ids_list[] = {
			{ NULL },
			{ L"Add", 0x00000001 },
			{ L"CompareMode", 0x00000009 },
			{ L"Count", 0x00000002 },
			{ L"Exists", 0x00000003 },
			{ L"HashVal", 0x0000000a },
            { L"Item", DISPID_VALUE },
			{ L"Items", 0x00000004 },
			{ L"Key", 0x00000005 },
			{ L"Keys", 0x00000006 },
			{ L"Remove", 0x00000007 },
			{ L"RemoveAll", 0x00000008 }
	};

	size_t min = 1, max = ARRAY_SIZE(names_ids_list) - 1, i;
	int r;
	while(min <= max) {
		i = (min + max) / 2;
		r = wcsicmp(names_ids_list[i].name, *rgszNames);
		if(!r) {
			*rgDispId = names_ids_list[i].dispId;
			return S_OK;
		}
		if(r < 0)
		   min = i+1;
		else
		   max = i-1;
	}
	return DISP_E_MEMBERNOTFOUND;
}

static HRESULT WINAPI dictionary_Invoke(IDictionary *iface, DISPID dispIdMember,
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
			if (wFlags & DISPATCH_PROPERTYPUTREF) {
				// line 113: [id(DISPID_VALUE), propputref]HRESULT Item([in] VARIANT* Key, [in] VARIANT* pRetItem);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantCopyInd(&var1, &pDispParams->rgvarg[--index]);
				hres = dictionary_putref_Item(iface, &var0, &var1);
				VariantClear(&var0);
				VariantClear(&var1);
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 116: [id(DISPID_VALUE), propput]HRESULT Item([in] VARIANT* Key, [in] VARIANT* pRetItem);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantCopyInd(&var1, &pDispParams->rgvarg[--index]);
				hres = dictionary_put_Item(iface, &var0, &var1);
				VariantClear(&var0);
				VariantClear(&var1);
			}
			else if (wFlags & DISPATCH_PROPERTYGET) {
				// line 119: [id(DISPID_VALUE), propget]HRESULT Item([in] VARIANT* Key, [out, retval] VARIANT* pRetItem);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = dictionary_get_Item(iface, &var0, &res);
				VariantClear(&var0);
			}
            else if (wFlags == (DISPATCH_METHOD | DISPATCH_PROPERTYGET)) {
				V_VT(&res) = VT_DISPATCH;
				V_DISPATCH(&res) = (IDispatch*)iface;
				hres = S_OK;
			}
			break;
		}
		case 0x00000001: {
			if (wFlags & DISPATCH_METHOD) {
				// line 122: [id(0x00000001)]HRESULT Add([in] VARIANT* Key, [in] VARIANT* Item);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantCopyInd(&var1, &pDispParams->rgvarg[--index]);
				hres = dictionary_Add(iface, &var0, &var1);
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x00000002: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 125: [id(0x00000002), propget]HRESULT Count([out, retval] long* pCount);
				V_VT(&res) = VT_I4;
				hres = dictionary_get_Count(iface, (LONG*)&V_I4(&res));
			}
			break;
		}
		case 0x00000003: {
			if (wFlags & DISPATCH_METHOD) {
				// line 128: [id(0x00000003)]HRESULT Exists([in] VARIANT* Key, [out, retval] VARIANT_BOOL* pExists);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				V_VT(&res) = VT_BOOL;
				hres = dictionary_Exists(iface, &var0, &V_BOOL(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000004: {
			if (wFlags & DISPATCH_METHOD) {
				// line 131: [id(0x00000004)]HRESULT Items([out, retval] VARIANT* pItemsArray);
				hres = dictionary_Items(iface, &res);
			}
			break;
		}
		case 0x00000005: {
			if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 134: [id(0x00000005), propput]HRESULT Key([in] VARIANT* Key, [in] VARIANT* rhs);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantCopyInd(&var1, &pDispParams->rgvarg[--index]);
				hres = dictionary_put_Key(iface, &var0, &var1);
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x00000006: {
			if (wFlags & DISPATCH_METHOD) {
				// line 137: [id(0x00000006)]HRESULT Keys([out, retval] VARIANT* pKeysArray);
				hres = dictionary_Keys(iface, &res);
			}
			break;
		}
		case 0x00000007: {
			if (wFlags & DISPATCH_METHOD) {
				// line 140: [id(0x00000007)]HRESULT Remove([in] VARIANT* Key);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = dictionary_Remove(iface, &var0);
				VariantClear(&var0);
			}
			break;
		}
		case 0x00000008: {
			if (wFlags & DISPATCH_METHOD) {
				// line 143: [id(0x00000008)]HRESULT RemoveAll();
				hres = dictionary_RemoveAll(iface);
			}
			break;
		}
		case 0x00000009: {
			if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 146: [id(0x00000009), propput]HRESULT CompareMode([in] CompareMethod pcomp);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = dictionary_put_CompareMode(iface, (CompareMethod)V_I4(&var0));
				VariantClear(&var0);
			}
			else if (wFlags & DISPATCH_PROPERTYGET) {
				// line 149: [id(0x00000009), propget]HRESULT CompareMode([out, retval] CompareMethod* pcomp);
				V_VT(&res) = VT_I4;
				hres = dictionary_get_CompareMode(iface, (CompareMethod*)&V_I4(&res));
			}
			break;
		}
		case DISPID_NEWENUM: {
			if (wFlags & DISPATCH_METHOD) {
				// line 152: [id(DISPID_NEWENUM), restricted]HRESULT _NewEnum([out, retval] IUnknown** ppunk);
				V_VT(&res) = VT_UNKNOWN;
				hres = dictionary__NewEnum(iface, &V_UNKNOWN(&res));
			}
			break;
		}
		case 0x0000000a: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 155: [id(0x0000000a), propget, hidden]HRESULT HashVal([in] VARIANT* Key, [out, retval] VARIANT* HashVal);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantCopyInd(&var0, &pDispParams->rgvarg[--index]);
				hres = dictionary_get_HashVal(iface, &var0, &res);
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
		external_log_info("dictionary_Invoke: dispId=%d (0x%08x), wFlags=%d, hres=%d", dispIdMember, dispIdMember, wFlags, hres);
	}
	return hres;
}

