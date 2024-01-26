void external_log_info(const char* format, ...);

static HRESULT WINAPI drive_GetIDsOfNames(IDrive *iface, REFIID riid, LPOLESTR *rgszNames,
                UINT cNames, LCID lcid, DISPID *rgDispId)
{
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} names_ids_list[] = {
			{ NULL },
			{ L"AvailableSpace", 0x00002715 },
			{ L"DriveLetter", 0x00002710 },
			{ L"DriveType", 0x00002712 },
			{ L"FileSystem", 0x00002718 },
			{ L"FreeSpace", 0x00002714 },
			{ L"IsReady", 0x0000271a },
			{ L"RootFolder", 0x00002713 },
			{ L"SerialNumber", 0x00002719 },
			{ L"ShareName", 0x00002711 },
			{ L"TotalSize", 0x00002716 },
			{ L"VolumeName", 0x00002717 }
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

static HRESULT WINAPI drive_Invoke(IDrive *iface, DISPID dispIdMember,
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
				// line 169: [id(DISPID_VALUE), propget]HRESULT Path([out, retval] BSTR* pbstrPath);
				V_VT(&res) = VT_BSTR;
				hres = drive_get_Path(iface, &V_BSTR(&res));
			}
            else if (wFlags == (DISPATCH_METHOD | DISPATCH_PROPERTYGET)) {
				V_VT(&res) = VT_DISPATCH;
				V_DISPATCH(&res) = (IDispatch*)iface;
				hres = S_OK;
			}
			break;
		}
		case 0x00002710: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 172: [id(0x00002710), propget]HRESULT DriveLetter([out, retval] BSTR* pbstrLetter);
				V_VT(&res) = VT_BSTR;
				hres = drive_get_DriveLetter(iface, &V_BSTR(&res));
			}
			break;
		}
		case 0x00002711: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 175: [id(0x00002711), propget]HRESULT ShareName([out, retval] BSTR* pbstrShareName);
				V_VT(&res) = VT_BSTR;
				hres = drive_get_ShareName(iface, &V_BSTR(&res));
			}
			break;
		}
		case 0x00002712: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 178: [id(0x00002712), propget]HRESULT DriveType([out, retval] DriveTypeConst* pdt);
				V_VT(&res) = VT_I4;
				hres = drive_get_DriveType(iface, (DriveTypeConst*)&V_I4(&res));
			}
			break;
		}
		case 0x00002713: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 181: [id(0x00002713), propget]HRESULT RootFolder([out, retval] IFolder** ppfolder);
				V_VT(&res) = VT_DISPATCH;
				hres = drive_get_RootFolder(iface, (IFolder**)&V_DISPATCH(&res));
			}
			break;
		}
		case 0x00002715: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 184: [id(0x00002715), propget]HRESULT AvailableSpace([out, retval] VARIANT* pvarAvail);
				hres = drive_get_AvailableSpace(iface, &res);
			}
			break;
		}
		case 0x00002714: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 187: [id(0x00002714), propget]HRESULT FreeSpace([out, retval] VARIANT* pvarFree);
				hres = drive_get_FreeSpace(iface, &res);
			}
			break;
		}
		case 0x00002716: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 190: [id(0x00002716), propget]HRESULT TotalSize([out, retval] VARIANT* pvarTotal);
				hres = drive_get_TotalSize(iface, &res);
			}
			break;
		}
		case 0x00002717: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 193: [id(0x00002717), propget]HRESULT VolumeName([out, retval] BSTR* pbstrName);
				V_VT(&res) = VT_BSTR;
				hres = drive_get_VolumeName(iface, &V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 196: [id(0x00002717), propput]HRESULT VolumeName([in] BSTR pbstrName);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = drive_put_VolumeName(iface, V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00002718: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 199: [id(0x00002718), propget]HRESULT FileSystem([out, retval] BSTR* pbstrFileSystem);
				V_VT(&res) = VT_BSTR;
				hres = drive_get_FileSystem(iface, &V_BSTR(&res));
			}
			break;
		}
		case 0x00002719: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 202: [id(0x00002719), propget]HRESULT SerialNumber([out, retval] long* pulSerialNumber);
				V_VT(&res) = VT_I4;
				hres = drive_get_SerialNumber(iface, (LONG*)&V_I4(&res));
			}
			break;
		}
		case 0x0000271a: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 205: [id(0x0000271a), propget]HRESULT IsReady([out, retval] VARIANT_BOOL* pfReady);
				V_VT(&res) = VT_BOOL;
				hres = drive_get_IsReady(iface, &V_BOOL(&res));
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
		external_log_info("drive_Invoke: dispId=%d (0x%08x), wFlags=%d, hres=%d", dispIdMember, dispIdMember, wFlags, hres);
	}
	return hres;
}

static HRESULT WINAPI drivecoll_GetIDsOfNames(IDriveCollection *iface, REFIID riid, LPOLESTR *rgszNames,
                UINT cNames, LCID lcid, DISPID *rgDispId)
{
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} names_ids_list[] = {
			{ NULL },
			{ L"Count", 0x00000001 }
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

static HRESULT WINAPI drivecoll_Invoke(IDriveCollection *iface, DISPID dispIdMember,
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
				// line 218: [id(DISPID_VALUE), propget]HRESULT Item([in] VARIANT Key, [out, retval] IDrive** ppdrive);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_VARIANT);
				V_VT(&res) = VT_DISPATCH;
				hres = drivecoll_get_Item(iface, var0, (IDrive**)&V_DISPATCH(&res));
				VariantClear(&var0);
			}
            else if (wFlags == (DISPATCH_METHOD | DISPATCH_PROPERTYGET)) {
				V_VT(&res) = VT_DISPATCH;
				V_DISPATCH(&res) = (IDispatch*)iface;
				hres = S_OK;
			}
			break;
        }
		case DISPID_NEWENUM: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 221: [id(DISPID_NEWENUM), propget, restricted, hidden]HRESULT _NewEnum([out, retval] IUnknown** ppenum);
				V_VT(&res) = VT_UNKNOWN;
				hres = drivecoll_get__NewEnum(iface, &V_UNKNOWN(&res));
			}
			break;
		}
		case 0x00000001: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 224: [id(0x00000001), propget]HRESULT Count([out, retval] long* plCount);
				V_VT(&res) = VT_I4;
				hres = drivecoll_get_Count(iface, (LONG*)&V_I4(&res));
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
		external_log_info("drivecoll_Invoke: dispId=%d (0x%08x), wFlags=%d, hres=%d", dispIdMember, dispIdMember, wFlags, hres);
	}
	return hres;
}

static HRESULT WINAPI textstream_GetIDsOfNames(ITextStream *iface, REFIID riid, LPOLESTR *rgszNames,
                UINT cNames, LCID lcid, DISPID *rgDispId)
{
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} names_ids_list[] = {
			{ NULL },
			{ L"AtEndOfLine", 0x00002713 },
			{ L"AtEndOfStream", 0x00002712 },
			{ L"Close", 0x0000271c },
			{ L"Column", 0xfffffdef },
			{ L"Line", 0x00002710 },
			{ L"Read", 0x00002714 },
			{ L"ReadAll", 0x00002716 },
			{ L"ReadLine", 0x00002715 },
			{ L"Skip", 0x0000271a },
			{ L"SkipLine", 0x0000271b },
			{ L"Write", 0x00002717 },
			{ L"WriteBlankLines", 0x00002719 },
			{ L"WriteLine", 0x00002718 }
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

static HRESULT WINAPI textstream_Invoke(ITextStream *iface, DISPID dispIdMember,
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
				V_VT(&res) = VT_DISPATCH;
				V_DISPATCH(&res) = (IDispatch*)iface;
				hres = S_OK;
			}
			break;
		}
		case 0x00002710: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 238: [id(0x00002710), propget]HRESULT Line([out, retval] long* Line);
				V_VT(&res) = VT_I4;
				hres = textstream_get_Line(iface, (LONG*)&V_I4(&res));
			}
			break;
		}
		case 0xfffffdef: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 241: [id(0xfffffdef), propget]HRESULT Column([out, retval] long* Column);
				V_VT(&res) = VT_I4;
				hres = textstream_get_Column(iface, (LONG*)&V_I4(&res));
			}
			break;
		}
		case 0x00002712: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 244: [id(0x00002712), propget]HRESULT AtEndOfStream([out, retval] VARIANT_BOOL* EOS);
				V_VT(&res) = VT_BOOL;
				hres = textstream_get_AtEndOfStream(iface, &V_BOOL(&res));
			}
			break;
		}
		case 0x00002713: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 247: [id(0x00002713), propget]HRESULT AtEndOfLine([out, retval] VARIANT_BOOL* EOL);
				V_VT(&res) = VT_BOOL;
				hres = textstream_get_AtEndOfLine(iface, &V_BOOL(&res));
			}
			break;
		}
		case 0x00002714: {
			if (wFlags & DISPATCH_METHOD) {
				// line 250: [id(0x00002714)]HRESULT Read([in] long Characters, [out, retval] BSTR* Text);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				V_VT(&res) = VT_BSTR;
				hres = textstream_Read(iface, V_I4(&var0), &V_BSTR(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00002715: {
			if (wFlags & DISPATCH_METHOD) {
				// line 253: [id(0x00002715)]HRESULT ReadLine([out, retval] BSTR* Text);
				V_VT(&res) = VT_BSTR;
				hres = textstream_ReadLine(iface, &V_BSTR(&res));
			}
			break;
		}
		case 0x00002716: {
			if (wFlags & DISPATCH_METHOD) {
				// line 256: [id(0x00002716)]HRESULT ReadAll([out, retval] BSTR* Text);
				V_VT(&res) = VT_BSTR;
				hres = textstream_ReadAll(iface, &V_BSTR(&res));
			}
			break;
		}
		case 0x00002717: {
			if (wFlags & DISPATCH_METHOD) {
				// line 259: [id(0x00002717)]HRESULT Write([in] BSTR Text);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = textstream_Write(iface, V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00002718: {
			if (wFlags & DISPATCH_METHOD) {
				// line 262: [id(0x00002718)]HRESULT WriteLine([in, defaultvalue("")] BSTR Text);
				VARIANT var0;
				OLECHAR* pszDefault = L"";
				V_VT(&var0) = VT_BSTR;
				V_BSTR(&var0) = SysAllocString(pszDefault);
				VariantChangeType(&var0, (index > 0) ? &pDispParams->rgvarg[--index] : &var0, 0, VT_BSTR);
				hres = textstream_WriteLine(iface, V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00002719: {
			if (wFlags & DISPATCH_METHOD) {
				// line 265: [id(0x00002719)]HRESULT WriteBlankLines([in] long Lines);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = textstream_WriteBlankLines(iface, V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x0000271a: {
			if (wFlags & DISPATCH_METHOD) {
				// line 268: [id(0x0000271a)]HRESULT Skip([in] long Characters);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = textstream_Skip(iface, V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x0000271b: {
			if (wFlags & DISPATCH_METHOD) {
				// line 271: [id(0x0000271b)]HRESULT SkipLine();
				hres = textstream_SkipLine(iface);
			}
			break;
		}
		case 0x0000271c: {
			if (wFlags & DISPATCH_METHOD) {
				// line 274: [id(0x0000271c)]HRESULT Close();
				hres = textstream_Close(iface);
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
		external_log_info("textstream_Invoke: dispId=%d (0x%08x), wFlags=%d, hres=%d", dispIdMember, dispIdMember, wFlags, hres);
	}
	return hres;
}

static HRESULT WINAPI file_GetIDsOfNames(IFile *iface, REFIID riid, LPOLESTR *rgszNames,
                UINT cNames, LCID lcid, DISPID *rgDispId)
{
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} names_ids_list[] = {
			{ NULL },
			{ L"Attributes", 0x000003eb },
			{ L"Copy", 0x000004b2 },
			{ L"DateCreated", 0x000003ee },
			{ L"DateLastAccessed", 0x000003f0 },
			{ L"DateLastModified", 0x000003ef },
			{ L"Delete", 0x000004b0 },
			{ L"Drive", 0x000003ec },
			{ L"Move", 0x000004b4 },
			{ L"Name", 0x000003e8 },
			{ L"OpenAsTextStream", 0x0000044c },
			{ L"ParentFolder", 0x000003ed },
			{ L"ShortName", 0x000003e9 },
			{ L"ShortPath", 0x000003ea },
			{ L"Size", 0x000003f1 },
			{ L"Type", 0x000003f2 }
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

static HRESULT WINAPI file_Invoke(IFile *iface, DISPID dispIdMember,
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
				// line 288: [id(DISPID_VALUE), propget]HRESULT Path([out, retval] BSTR* pbstrPath);
				V_VT(&res) = VT_BSTR;
				hres = file_get_Path(iface, &V_BSTR(&res));
			}
            else if (wFlags == (DISPATCH_METHOD | DISPATCH_PROPERTYGET)) {
				V_VT(&res) = VT_DISPATCH;
				V_DISPATCH(&res) = (IDispatch*)iface;
				hres = S_OK;
			}
			break;
		}
		case 0x000003e8: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 291: [id(0x000003e8), propget]HRESULT Name([out, retval] BSTR* pbstrName);
				V_VT(&res) = VT_BSTR;
				hres = file_get_Name(iface, &V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 294: [id(0x000003e8), propput]HRESULT Name([in] BSTR pbstrName);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = file_put_Name(iface, V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x000003ea: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 297: [id(0x000003ea), propget]HRESULT ShortPath([out, retval] BSTR* pbstrPath);
				V_VT(&res) = VT_BSTR;
				hres = file_get_ShortPath(iface, &V_BSTR(&res));
			}
			break;
		}
		case 0x000003e9: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 300: [id(0x000003e9), propget]HRESULT ShortName([out, retval] BSTR* pbstrName);
				V_VT(&res) = VT_BSTR;
				hres = file_get_ShortName(iface, &V_BSTR(&res));
			}
			break;
		}
		case 0x000003ec: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 303: [id(0x000003ec), propget]HRESULT Drive([out, retval] IDrive** ppdrive);
				V_VT(&res) = VT_DISPATCH;
				hres = file_get_Drive(iface, (IDrive**)&V_DISPATCH(&res));
			}
			break;
		}
		case 0x000003ed: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 306: [id(0x000003ed), propget]HRESULT ParentFolder([out, retval] IFolder** ppfolder);
				V_VT(&res) = VT_DISPATCH;
				hres = file_get_ParentFolder(iface, (IFolder**)&V_DISPATCH(&res));
			}
			break;
		}
		case 0x000003eb: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 309: [id(0x000003eb), propget]HRESULT Attributes([out, retval] FileAttribute* pfa);
				V_VT(&res) = VT_I4;
				hres = file_get_Attributes(iface, (FileAttribute*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 312: [id(0x000003eb), propput]HRESULT Attributes([in] FileAttribute pfa);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = file_put_Attributes(iface, (FileAttribute)V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x000003ee: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 315: [id(0x000003ee), propget]HRESULT DateCreated([out, retval] DATE* pdate);
				V_VT(&res) = VT_DATE;
				hres = file_get_DateCreated(iface, &V_DATE(&res));
			}
			break;
		}
		case 0x000003ef: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 318: [id(0x000003ef), propget]HRESULT DateLastModified([out, retval] DATE* pdate);
				V_VT(&res) = VT_DATE;
				hres = file_get_DateLastModified(iface, &V_DATE(&res));
			}
			break;
		}
		case 0x000003f0: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 321: [id(0x000003f0), propget]HRESULT DateLastAccessed([out, retval] DATE* pdate);
				V_VT(&res) = VT_DATE;
				hres = file_get_DateLastAccessed(iface, &V_DATE(&res));
			}
			break;
		}
		case 0x000003f1: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 324: [id(0x000003f1), propget]HRESULT Size([out, retval] VARIANT* pvarSize);
				hres = file_get_Size(iface, &res);
			}
			break;
		}
		case 0x000003f2: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 327: [id(0x000003f2), propget]HRESULT Type([out, retval] BSTR* pbstrType);
				V_VT(&res) = VT_BSTR;
				hres = file_get_Type(iface, &V_BSTR(&res));
			}
			break;
		}
		case 0x000004b0: {
			if (wFlags & DISPATCH_METHOD) {
				// line 330: [id(0x000004b0)]HRESULT Delete([in, defaultvalue(0)] VARIANT_BOOL Force);
				VARIANT var0;
				V_VT(&var0) = VT_BOOL;
				V_BOOL(&var0) = 0;
				VariantChangeType(&var0, (index > 0) ? &pDispParams->rgvarg[--index] : &var0, 0, VT_BOOL);
				hres = file_Delete(iface, V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x000004b2: {
			if (wFlags & DISPATCH_METHOD) {
				// line 333: [id(0x000004b2)]HRESULT Copy([in] BSTR Destination, [in, defaultvalue(-1)] VARIANT_BOOL OverWriteFiles);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var1;
				V_VT(&var1) = VT_BOOL;
				V_BOOL(&var1) = -1;
				VariantChangeType(&var1, (index > 0) ? &pDispParams->rgvarg[--index] : &var1, 0, VT_BOOL);
				hres = file_Copy(iface, V_BSTR(&var0), V_BOOL(&var1));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x000004b4: {
			if (wFlags & DISPATCH_METHOD) {
				// line 336: [id(0x000004b4)]HRESULT Move([in] BSTR Destination);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = file_Move(iface, V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x0000044c: {
			if (wFlags & DISPATCH_METHOD) {
				// line 339: [id(0x0000044c)]HRESULT OpenAsTextStream([in, defaultvalue(1)] IOMode IOMode,[in, defaultvalue(0)] Tristate Format, [out, retval] ITextStream** ppts);
				VARIANT var0;
				V_VT(&var0) = VT_I4;
				V_I4(&var0) = 1;
				VariantChangeType(&var0, (index > 0) ? &pDispParams->rgvarg[--index] : &var0, 0, VT_I4);
				VARIANT var1;
				V_VT(&var1) = VT_I4;
				V_I4(&var1) = 0;
				VariantChangeType(&var1, (index > 0) ? &pDispParams->rgvarg[--index] : &var1, 0, VT_I4);
				V_VT(&res) = VT_DISPATCH;
				hres = file_OpenAsTextStream(iface, (IOMode)V_I4(&var0), (Tristate)V_I4(&var1), (ITextStream**)&V_DISPATCH(&res));
				VariantClear(&var0);
				VariantClear(&var1);
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
		external_log_info("file_Invoke: dispId=%d (0x%08x), wFlags=%d, hres=%d", dispIdMember, dispIdMember, wFlags, hres);
	}
	return hres;
}

static HRESULT WINAPI filesys_GetIDsOfNames(IFileSystem3 *iface, REFIID riid, LPOLESTR *rgszNames,
                UINT cNames, LCID lcid, DISPID *rgDispId)
{
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} names_ids_list[] = {
			{ NULL },
			{ L"BuildPath", 0x00002710 },
			{ L"CopyFile", 0x000004b2 },
			{ L"CopyFolder", 0x000004b3 },
			{ L"CreateFolder", 0x00000460 },
			{ L"CreateTextFile", 0x0000044d },
			{ L"DeleteFile", 0x000004b0 },
			{ L"DeleteFolder", 0x000004b1 },
			{ L"DriveExists", 0x0000271f },
			{ L"Drives", 0x0000271a },
			{ L"FileExists", 0x00002720 },
			{ L"FolderExists", 0x00002721 },
			{ L"GetAbsolutePathName", 0x00002712 },
			{ L"GetBaseName", 0x00002717 },
			{ L"GetDrive", 0x0000271b },
			{ L"GetDriveName", 0x00002714 },
			{ L"GetExtensionName", 0x00002718 },
			{ L"GetFile", 0x0000271c },
			{ L"GetFileName", 0x00002716 },
            { L"GetFileVersion", 0x00004e2a },
			{ L"GetFolder", 0x0000271d },
			{ L"GetParentFolderName", 0x00002715 },
			{ L"GetSpecialFolder", 0x0000271e },
            { L"GetStandardStream", 0x00004e20 },
			{ L"GetTempName", 0x00002713 },
			{ L"MoveFile", 0x000004b4 },
			{ L"MoveFolder", 0x000004b5 },
			{ L"OpenTextFile", 0x0000044c }
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

static HRESULT WINAPI filesys_Invoke(IFileSystem3 *iface, DISPID dispIdMember,
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
				V_VT(&res) = VT_DISPATCH;
				V_DISPATCH(&res) = (IDispatch*)iface;
				hres = S_OK;
			}
			break;
		}
		case 0x0000271a: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 354: [id(0x0000271a), propget]HRESULT Drives([out, retval] IDriveCollection** ppdrives);
				V_VT(&res) = VT_DISPATCH;
				hres = filesys_get_Drives(iface, (IDriveCollection**)&V_DISPATCH(&res));
			}
			break;
		}
		case 0x00002710: {
			if (wFlags & DISPATCH_METHOD) {
				// line 357: [id(0x00002710)]HRESULT BuildPath([in] BSTR Path, [in] BSTR Name, [out, retval] BSTR* pbstrResult);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				V_VT(&res) = VT_BSTR;
				hres = filesys_BuildPath(iface, V_BSTR(&var0), V_BSTR(&var1), &V_BSTR(&res));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x00002714: {
			if (wFlags & DISPATCH_METHOD) {
				// line 360: [id(0x00002714)]HRESULT GetDriveName([in] BSTR Path, [out, retval] BSTR* pbstrResult);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				V_VT(&res) = VT_BSTR;
				hres = filesys_GetDriveName(iface, V_BSTR(&var0), &V_BSTR(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00002715: {
			if (wFlags & DISPATCH_METHOD) {
				// line 363: [id(0x00002715)]HRESULT GetParentFolderName([in] BSTR Path, [out, retval] BSTR* pbstrResult);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				V_VT(&res) = VT_BSTR;
				hres = filesys_GetParentFolderName(iface, V_BSTR(&var0), &V_BSTR(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00002716: {
			if (wFlags & DISPATCH_METHOD) {
				// line 366: [id(0x00002716)]HRESULT GetFileName([in] BSTR Path, [out, retval] BSTR* pbstrResult);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				V_VT(&res) = VT_BSTR;
				hres = filesys_GetFileName(iface, V_BSTR(&var0), &V_BSTR(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00002717: {
			if (wFlags & DISPATCH_METHOD) {
				// line 369: [id(0x00002717)]HRESULT GetBaseName([in] BSTR Path, [out, retval] BSTR* pbstrResult);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				V_VT(&res) = VT_BSTR;
				hres = filesys_GetBaseName(iface, V_BSTR(&var0), &V_BSTR(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00002718: {
			if (wFlags & DISPATCH_METHOD) {
				// line 372: [id(0x00002718)]HRESULT GetExtensionName([in] BSTR Path, [out, retval] BSTR* pbstrResult);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				V_VT(&res) = VT_BSTR;
				hres = filesys_GetExtensionName(iface, V_BSTR(&var0), &V_BSTR(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00002712: {
			if (wFlags & DISPATCH_METHOD) {
				// line 375: [id(0x00002712)]HRESULT GetAbsolutePathName([in] BSTR Path, [out, retval] BSTR* pbstrResult);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				V_VT(&res) = VT_BSTR;
				hres = filesys_GetAbsolutePathName(iface, V_BSTR(&var0), &V_BSTR(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00002713: {
			if (wFlags & DISPATCH_METHOD) {
				// line 378: [id(0x00002713)]HRESULT GetTempName([out, retval] BSTR* pbstrResult);
				V_VT(&res) = VT_BSTR;
				hres = filesys_GetTempName(iface, &V_BSTR(&res));
			}
			break;
		}
		case 0x0000271f: {
			if (wFlags & DISPATCH_METHOD) {
				// line 381: [id(0x0000271f)]HRESULT DriveExists([in] BSTR DriveSpec, [out, retval] VARIANT_BOOL* pfExists);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				V_VT(&res) = VT_BOOL;
				hres = filesys_DriveExists(iface, V_BSTR(&var0), &V_BOOL(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00002720: {
			if (wFlags & DISPATCH_METHOD) {
				// line 384: [id(0x00002720)]HRESULT FileExists([in] BSTR FileSpec, [out, retval] VARIANT_BOOL* pfExists);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				V_VT(&res) = VT_BOOL;
				hres = filesys_FileExists(iface, V_BSTR(&var0), &V_BOOL(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00002721: {
			if (wFlags & DISPATCH_METHOD) {
				// line 387: [id(0x00002721)]HRESULT FolderExists([in] BSTR FolderSpec, [out, retval] VARIANT_BOOL* pfExists);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				V_VT(&res) = VT_BOOL;
				hres = filesys_FolderExists(iface, V_BSTR(&var0), &V_BOOL(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 0x0000271b: {
			if (wFlags & DISPATCH_METHOD) {
				// line 390: [id(0x0000271b)]HRESULT GetDrive([in] BSTR DriveSpec, [out, retval] IDrive** ppdrive);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				V_VT(&res) = VT_DISPATCH;
				hres = filesys_GetDrive(iface, V_BSTR(&var0), (IDrive**)&V_DISPATCH(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 0x0000271c: {
			if (wFlags & DISPATCH_METHOD) {
				// line 393: [id(0x0000271c)]HRESULT GetFile([in] BSTR FilePath, [out, retval] IFile** ppfile);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				V_VT(&res) = VT_DISPATCH;
				hres = filesys_GetFile(iface, V_BSTR(&var0), (IFile**)&V_DISPATCH(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 0x0000271d: {
			if (wFlags & DISPATCH_METHOD) {
				// line 396: [id(0x0000271d)]HRESULT GetFolder([in] BSTR FolderPath, [out, retval] IFolder** ppfolder);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				V_VT(&res) = VT_DISPATCH;
				hres = filesys_GetFolder(iface, V_BSTR(&var0), (IFolder**)&V_DISPATCH(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 0x0000271e: {
			if (wFlags & DISPATCH_METHOD) {
				// line 399: [id(0x0000271e)]HRESULT GetSpecialFolder([in] SpecialFolderConst SpecialFolder, [out, retval] IFolder** ppfolder);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				V_VT(&res) = VT_DISPATCH;
				hres = filesys_GetSpecialFolder(iface, (SpecialFolderConst)V_I4(&var0), (IFolder**)&V_DISPATCH(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 0x000004b0: {
			if (wFlags & DISPATCH_METHOD) {
				// line 402: [id(0x000004b0)]HRESULT DeleteFile([in] BSTR FileSpec, [in, defaultvalue(0)] VARIANT_BOOL Force);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var1;
				V_VT(&var1) = VT_BOOL;
				V_BOOL(&var1) = 0;
				VariantChangeType(&var1, (index > 0) ? &pDispParams->rgvarg[--index] : &var1, 0, VT_BOOL);
				hres = filesys_DeleteFile(iface, V_BSTR(&var0), V_BOOL(&var1));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x000004b1: {
			if (wFlags & DISPATCH_METHOD) {
				// line 405: [id(0x000004b1)]HRESULT DeleteFolder([in] BSTR FolderSpec, [in, defaultvalue(0)] VARIANT_BOOL Force);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var1;
				V_VT(&var1) = VT_BOOL;
				V_BOOL(&var1) = 0;
				VariantChangeType(&var1, (index > 0) ? &pDispParams->rgvarg[--index] : &var1, 0, VT_BOOL);
				hres = filesys_DeleteFolder(iface, V_BSTR(&var0), V_BOOL(&var1));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x000004b4: {
			if (wFlags & DISPATCH_METHOD) {
				// line 408: [id(0x000004b4), helpstring("Move a file"), helpcontext(0x00214bab)]HRESULT MoveFile([in] BSTR Source, [in] BSTR Destination);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = filesys_MoveFile(iface, V_BSTR(&var0), V_BSTR(&var1));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x000004b5: {
			if (wFlags & DISPATCH_METHOD) {
				// line 411: [id(0x000004b5)]HRESULT MoveFolder([in] BSTR Source, [in] BSTR Destination);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = filesys_MoveFolder(iface, V_BSTR(&var0), V_BSTR(&var1));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x000004b2: {
			if (wFlags & DISPATCH_METHOD) {
				// line 414: [id(0x000004b2)]HRESULT CopyFile([in] BSTR Source, [in] BSTR Destination,[in, defaultvalue(-1)] VARIANT_BOOL OverWriteFiles);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var2;
				V_VT(&var2) = VT_BOOL;
				V_BOOL(&var2) = -1;
				VariantChangeType(&var2, (index > 0) ? &pDispParams->rgvarg[--index] : &var2, 0, VT_BOOL);
				hres = filesys_CopyFile(iface, V_BSTR(&var0), V_BSTR(&var1), V_BOOL(&var2));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
			}
			break;
		}
		case 0x000004b3: {
			if (wFlags & DISPATCH_METHOD) {
				// line 418: [id(0x000004b3)]HRESULT CopyFolder([in] BSTR Source, [in] BSTR Destination,[in, defaultvalue(-1)] VARIANT_BOOL OverWriteFiles);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var1;
				V_VT(&var1) = VT_EMPTY;
				VariantChangeType(&var1, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var2;
				V_VT(&var2) = VT_BOOL;
				V_BOOL(&var2) = -1;
				VariantChangeType(&var2, (index > 0) ? &pDispParams->rgvarg[--index] : &var2, 0, VT_BOOL);
				hres = filesys_CopyFolder(iface, V_BSTR(&var0), V_BSTR(&var1), V_BOOL(&var2));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
			}
			break;
		}
		case 0x00000460: {
			if (wFlags & DISPATCH_METHOD) {
				// line 422: [id(0x00000460)]HRESULT CreateFolder([in] BSTR Path, [out, retval] IFolder** ppfolder);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				V_VT(&res) = VT_DISPATCH;
				hres = filesys_CreateFolder(iface, V_BSTR(&var0), (IFolder**)&V_DISPATCH(&res));
				VariantClear(&var0);
			}
			break;
		}
		case 0x0000044d: {
			if (wFlags & DISPATCH_METHOD) {
				// line 425: [id(0x0000044d)]HRESULT CreateTextFile([in] BSTR FileName, [in, defaultvalue(-1)] VARIANT_BOOL Overwrite,[in, defaultvalue(0)] VARIANT_BOOL Unicode, [out, retval] ITextStream** ppts);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var1;
				V_VT(&var1) = VT_BOOL;
				V_BOOL(&var1) = -1;
				VariantChangeType(&var1, (index > 0) ? &pDispParams->rgvarg[--index] : &var1, 0, VT_BOOL);
				VARIANT var2;
				V_VT(&var2) = VT_BOOL;
				V_BOOL(&var2) = 0;
				VariantChangeType(&var2, (index > 0) ? &pDispParams->rgvarg[--index] : &var2, 0, VT_BOOL);
				V_VT(&res) = VT_DISPATCH;
				hres = filesys_CreateTextFile(iface, V_BSTR(&var0), V_BOOL(&var1), V_BOOL(&var2), (ITextStream**)&V_DISPATCH(&res));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
			}
			break;
		}
		case 0x0000044c: {
			if (wFlags & DISPATCH_METHOD) {
				// line 429: [id(0x0000044c)]HRESULT OpenTextFile([in] BSTR FileName, [in, defaultvalue(1)] IOMode IOMode,[in, defaultvalue(0)] VARIANT_BOOL Create,[in, defaultvalue(0)] Tristate Format,[out, retval] ITextStream** ppts);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var1;
				V_VT(&var1) = VT_I4;
				V_I4(&var1) = 1;
				VariantChangeType(&var1, (index > 0) ? &pDispParams->rgvarg[--index] : &var1, 0, VT_I4);
				VARIANT var2;
				V_VT(&var2) = VT_BOOL;
				V_BOOL(&var2) = 0;
				VariantChangeType(&var2, (index > 0) ? &pDispParams->rgvarg[--index] : &var2, 0, VT_BOOL);
				VARIANT var3;
				V_VT(&var3) = VT_I4;
				V_I4(&var3) = 0;
				VariantChangeType(&var3, (index > 0) ? &pDispParams->rgvarg[--index] : &var3, 0, VT_I4);
				V_VT(&res) = VT_DISPATCH;
				hres = filesys_OpenTextFile(iface, V_BSTR(&var0), (IOMode)V_I4(&var1), V_BOOL(&var2), (Tristate)V_I4(&var3), (ITextStream**)&V_DISPATCH(&res));
				VariantClear(&var0);
				VariantClear(&var1);
				VariantClear(&var2);
				VariantClear(&var3);
			}
			break;
		}
        case 0x00004e20: {
			if (wFlags & DISPATCH_METHOD) {
				// line 563: [id(0x00004e20)]HRESULT GetStandardStream([in] StandardStreamTypes StandardStreamType,[in, defaultvalue(0)] VARIANT_BOOL Unicode, [out, retval] ITextStream** ppts);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				VARIANT var1;
				V_VT(&var1) = VT_BOOL;
				V_BOOL(&var1) = 0;
				VariantChangeType(&var1, (index > 0) ? &pDispParams->rgvarg[--index] : &var1, 0, VT_BOOL);
				V_VT(&res) = VT_DISPATCH;
				hres = filesys_GetStandardStream(iface, (StandardStreamTypes)V_I4(&var0), V_BOOL(&var1), (ITextStream**)&V_DISPATCH(&res));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x00004e2a: {
			if (wFlags & DISPATCH_METHOD) {
				// line 567: [id(0x00004e2a)]HRESULT GetFileVersion([in] BSTR FileName, [out, retval] BSTR* FileVersion);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				V_VT(&res) = VT_BSTR;
				hres = filesys_GetFileVersion(iface, V_BSTR(&var0), &V_BSTR(&res));
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
		external_log_info("filesys_Invoke: dispId=%d (0x%08x), wFlags=%d, hres=%d", dispIdMember, dispIdMember, wFlags, hres);
	}
	return hres;
}

static HRESULT WINAPI foldercoll_GetIDsOfNames(IFolderCollection *iface, REFIID riid, LPOLESTR *rgszNames,
                UINT cNames, LCID lcid, DISPID *rgDispId)
{
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} names_ids_list[] = {
			{ NULL },
			{ L"Add", 0x00000002 },
			{ L"Count", 0x00000001 }
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

static HRESULT WINAPI foldercoll_Invoke(IFolderCollection *iface, DISPID dispIdMember,
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
				// line 469: [id(DISPID_VALUE), propget]HRESULT Item([in] VARIANT Key, [out, retval] IFolder** ppfolder);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_VARIANT);
				V_VT(&res) = VT_DISPATCH;
				hres = foldercoll_get_Item(iface, var0, (IFolder**)&V_DISPATCH(&res));
				VariantClear(&var0);
			}
            else if (wFlags == (DISPATCH_METHOD | DISPATCH_PROPERTYGET)) {
				V_VT(&res) = VT_DISPATCH;
				V_DISPATCH(&res) = (IDispatch*)iface;
				hres = S_OK;
			}
			break;
		}
		case 0x00000002: {
			if (wFlags & DISPATCH_METHOD) {
				// line 466: [id(0x00000002)]HRESULT Add([in] BSTR Name, [out, retval] IFolder** ppfolder);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				V_VT(&res) = VT_DISPATCH;
				hres = foldercoll_Add(iface, V_BSTR(&var0), (IFolder**)&V_DISPATCH(&res));
				VariantClear(&var0);
			}
			break;
		}
		case DISPID_NEWENUM: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 472: [id(DISPID_NEWENUM), propget, restricted, hidden]HRESULT _NewEnum([out, retval] IUnknown** ppenum);
				V_VT(&res) = VT_UNKNOWN;
				hres = foldercoll_get__NewEnum(iface, &V_UNKNOWN(&res));
			}
			break;
		}
		case 0x00000001: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 475: [id(0x00000001), propget]HRESULT Count([out, retval] long* plCount);
				V_VT(&res) = VT_I4;
				hres = foldercoll_get_Count(iface, (LONG*)&V_I4(&res));
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
		external_log_info("foldercoll_Invoke: dispId=%d (0x%08x), wFlags=%d, hres=%d", dispIdMember, dispIdMember, wFlags, hres);
	}
	return hres;
}

static HRESULT WINAPI folder_GetIDsOfNames(IFolder *iface, REFIID riid, LPOLESTR *rgszNames,
                UINT cNames, LCID lcid, DISPID *rgDispId)
{
	static struct {
		const WCHAR *name;
		DISPID dispId;
	} names_ids_list[] = {
			{ NULL },
			{ L"Attributes", 0x000003eb },
			{ L"Copy", 0x000004b3 },
			{ L"CreateTextFile", 0x0000044d },
			{ L"DateCreated", 0x000003ee },
			{ L"DateLastAccessed", 0x000003f0 },
			{ L"DateLastModified", 0x000003ef },
			{ L"Delete", 0x000004b1 },
			{ L"Drive", 0x000003ec },
			{ L"Files", 0x00002712 },
			{ L"IsRootFolder", 0x00002710 },
			{ L"Move", 0x000004b5 },
			{ L"Name", 0x000003e8 },
			{ L"ParentFolder", 0x000003ed },
			{ L"ShortName", 0x000003e9 },
			{ L"ShortPath", 0x000003ea },
			{ L"Size", 0x000003f1 },
			{ L"SubFolders", 0x00002711 },
			{ L"Type", 0x000003f2 }
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

static HRESULT WINAPI folder_Invoke(IFolder *iface, DISPID dispIdMember,
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
				// line 489: [id(DISPID_VALUE), propget]HRESULT Path([out, retval] BSTR* pbstrPath);
				V_VT(&res) = VT_BSTR;
				hres = folder_get_Path(iface, &V_BSTR(&res));
			}
            else if (wFlags == (DISPATCH_METHOD | DISPATCH_PROPERTYGET)) {
				V_VT(&res) = VT_DISPATCH;
				V_DISPATCH(&res) = (IDispatch*)iface;
				hres = S_OK;
			}
			break;
		}
		case 0x000003e8: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 492: [id(0x000003e8), propget]HRESULT Name([out, retval] BSTR* pbstrName);
				V_VT(&res) = VT_BSTR;
				hres = folder_get_Name(iface, &V_BSTR(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 495: [id(0x000003e8), propput]HRESULT Name([in] BSTR pbstrName);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = folder_put_Name(iface, V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x000003ea: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 498: [id(0x000003ea), propget]HRESULT ShortPath([out, retval] BSTR* pbstrPath);
				V_VT(&res) = VT_BSTR;
				hres = folder_get_ShortPath(iface, &V_BSTR(&res));
			}
			break;
		}
		case 0x000003e9: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 501: [id(0x000003e9), propget]HRESULT ShortName([out, retval] BSTR* pbstrName);
				V_VT(&res) = VT_BSTR;
				hres = folder_get_ShortName(iface, &V_BSTR(&res));
			}
			break;
		}
		case 0x000003ec: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 504: [id(0x000003ec), propget]HRESULT Drive([out, retval] IDrive** ppdrive);
				V_VT(&res) = VT_DISPATCH;
				hres = folder_get_Drive(iface, (IDrive**)&V_DISPATCH(&res));
			}
			break;
		}
		case 0x000003ed: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 507: [id(0x000003ed), propget]HRESULT ParentFolder([out, retval] IFolder** ppfolder);
				V_VT(&res) = VT_DISPATCH;
				hres = folder_get_ParentFolder(iface, (IFolder**)&V_DISPATCH(&res));
			}
			break;
		}
		case 0x000003eb: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 510: [id(0x000003eb), propget]HRESULT Attributes([out, retval] FileAttribute* pfa);
				V_VT(&res) = VT_I4;
				hres = folder_get_Attributes(iface, (FileAttribute*)&V_I4(&res));
			}
			else if (wFlags & DISPATCH_PROPERTYPUT) {
				// line 513: [id(0x000003eb), propput]HRESULT Attributes([in] FileAttribute pfa);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_I4);
				hres = folder_put_Attributes(iface, (FileAttribute)V_I4(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x000003ee: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 516: [id(0x000003ee), propget]HRESULT DateCreated([out, retval] DATE* pdate);
				V_VT(&res) = VT_DATE;
				hres = folder_get_DateCreated(iface, &V_DATE(&res));
			}
			break;
		}
		case 0x000003ef: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 519: [id(0x000003ef), propget]HRESULT DateLastModified([out, retval] DATE* pdate);
				V_VT(&res) = VT_DATE;
				hres = folder_get_DateLastModified(iface, &V_DATE(&res));
			}
			break;
		}
		case 0x000003f0: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 522: [id(0x000003f0), propget]HRESULT DateLastAccessed([out, retval] DATE* pdate);
				V_VT(&res) = VT_DATE;
				hres = folder_get_DateLastAccessed(iface, &V_DATE(&res));
			}
			break;
		}
		case 0x000003f2: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 525: [id(0x000003f2), propget]HRESULT Type([out, retval] BSTR* pbstrType);
				V_VT(&res) = VT_BSTR;
				hres = folder_get_Type(iface, &V_BSTR(&res));
			}
			break;
		}
		case 0x000004b1: {
			if (wFlags & DISPATCH_METHOD) {
				// line 528: [id(0x000004b1)]HRESULT Delete([in, defaultvalue(0)] VARIANT_BOOL Force);
				VARIANT var0;
				V_VT(&var0) = VT_BOOL;
				V_BOOL(&var0) = 0;
				VariantChangeType(&var0, (index > 0) ? &pDispParams->rgvarg[--index] : &var0, 0, VT_BOOL);
				hres = folder_Delete(iface, V_BOOL(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x000004b3: {
			if (wFlags & DISPATCH_METHOD) {
				// line 531: [id(0x000004b3)]HRESULT Copy([in] BSTR Destination, [in, defaultvalue(-1)] VARIANT_BOOL OverWriteFiles);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var1;
				V_VT(&var1) = VT_BOOL;
				V_BOOL(&var1) = -1;
				VariantChangeType(&var1, (index > 0) ? &pDispParams->rgvarg[--index] : &var1, 0, VT_BOOL);
				hres = folder_Copy(iface, V_BSTR(&var0), V_BOOL(&var1));
				VariantClear(&var0);
				VariantClear(&var1);
			}
			break;
		}
		case 0x000004b5: {
			if (wFlags & DISPATCH_METHOD) {
				// line 534: [id(0x000004b5)]HRESULT Move([in] BSTR Destination);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				hres = folder_Move(iface, V_BSTR(&var0));
				VariantClear(&var0);
			}
			break;
		}
		case 0x00002710: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 537: [id(0x00002710), propget]HRESULT IsRootFolder([out, retval] VARIANT_BOOL* pfRootFolder);
				V_VT(&res) = VT_BOOL;
				hres = folder_get_IsRootFolder(iface, &V_BOOL(&res));
			}
			break;
		}
		case 0x000003f1: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 540: [id(0x000003f1), propget]HRESULT Size([out, retval] VARIANT* pvarSize);
				hres = folder_get_Size(iface, &res);
			}
			break;
		}
		case 0x00002711: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 543: [id(0x00002711), propget]HRESULT SubFolders([out, retval] IFolderCollection** ppfolders);
				V_VT(&res) = VT_DISPATCH;
				hres = folder_get_SubFolders(iface, (IFolderCollection**)&V_DISPATCH(&res));
			}
			break;
		}
		case 0x00002712: {
			if (wFlags & DISPATCH_PROPERTYGET) {
				// line 546: [id(0x00002712), propget]HRESULT Files([out, retval] IFileCollection** ppfiles);
				V_VT(&res) = VT_DISPATCH;
				hres = folder_get_Files(iface, (IFileCollection**)&V_DISPATCH(&res));
			}
			break;
		}
		case 0x0000044d: {
			if (wFlags & DISPATCH_METHOD) {
				// line 549: [id(0x0000044d)]HRESULT CreateTextFile([in] BSTR FileName, [in, defaultvalue(-1)] VARIANT_BOOL Overwrite,[in, defaultvalue(0)] VARIANT_BOOL Unicode, [out, retval] ITextStream** ppts);
				VARIANT var0;
				V_VT(&var0) = VT_EMPTY;
				VariantChangeType(&var0, &pDispParams->rgvarg[--index], 0, VT_BSTR);
				VARIANT var1;
				V_VT(&var1) = VT_BOOL;
				V_BOOL(&var1) = -1;
				VariantChangeType(&var1, (index > 0) ? &pDispParams->rgvarg[--index] : &var1, 0, VT_BOOL);
				VARIANT var2;
				V_VT(&var2) = VT_BOOL;
				V_BOOL(&var2) = 0;
				VariantChangeType(&var2, (index > 0) ? &pDispParams->rgvarg[--index] : &var2, 0, VT_BOOL);
				V_VT(&res) = VT_DISPATCH;
				hres = folder_CreateTextFile(iface, V_BSTR(&var0), V_BOOL(&var1), V_BOOL(&var2), (ITextStream**)&V_DISPATCH(&res));
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
		external_log_info("folder_Invoke: dispId=%d (0x%08x), wFlags=%d, hres=%d", dispIdMember, dispIdMember, wFlags, hres);
	}
	return hres;
}