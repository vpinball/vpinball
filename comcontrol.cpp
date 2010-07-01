// ComControl.cpp : Implementation of CComControl

#include "stdafx.h"
#include <initguid.h>

#define FREE_MEMORY WM_USER+1000

DEFINE_GUID(GUID_OLE_COLOR, 0x66504301, 0xBE0F, 0x101A, 0x8B, 0xBB, 0x00, 0xAA, 0x00, 0x30, 0x0C, 0xAB);

int CALLBACK ComListProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

int CALLBACK ComListProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
	switch (uMsg)
		{
		case WM_INITDIALOG:
			{
			ICatInformation *pci;
			HRESULT hr;
			HWND hwndList;
			HWND hwndOk;

			SetWindowLong(hwndDlg, GWL_USERDATA, lParam);

			hwndOk = GetDlgItem(hwndDlg, IDOK);
			EnableWindow(hwndOk, FALSE);

			hwndList = GetDlgItem(hwndDlg, IDC_LIST);
			
			hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr, NULL, CLSCTX_SERVER, IID_ICatInformation, (void **)&pci);
			
			if (pci)
				{
				IEnumCLSID *pec = 0;
				CATID rgcid[1];
				rgcid[0] = CATID_Control;

				hr = pci->EnumClassesOfCategories(1, rgcid, ~0u, 0, &pec);

				if (hr == S_OK)
					{

					// walk list of CLSIDs 64 at a time
					CLSID rgclsid[64];
					do
						{
						ULONG cActual = 0;
						hr = pec->Next(64, rgclsid, &cActual);
						if (SUCCEEDED(hr))
							{
							for (ULONG i = 0; i < cActual; i++)
								{
								WCHAR *ppwz;

								OleRegGetUserType(rgclsid[i], USERCLASSTYPE_FULL, &ppwz);
								//DisplayClass(rgclsid[i]);
								
								char szT[1024];
								WideCharToMultiByte(CP_ACP, 0, ppwz, -1, szT, 1024, NULL, NULL);

								int index = SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)szT);
								CLSID *pclsid = new CLSID();
								*pclsid = rgclsid[i];

								SendMessage(hwndList, LB_SETITEMDATA, index, (LPARAM)pclsid);

								CoTaskMemFree(ppwz);
								}
							}
						} while (hr == S_OK);

					pec->Release();
					}

				pci->Release();
				}
			}

			return TRUE;
			break;

		case FREE_MEMORY:
			{
			HWND hwndList;
			hwndList = GetDlgItem(hwndDlg, IDC_LIST);
			int count;
			CLSID *pclsid;

			count = SendMessage(hwndList, LB_GETCOUNT, 0, 0);

			for (int i=0;i<count;i++)
				{
				pclsid = (CLSID *)SendMessage(hwndList, LB_GETITEMDATA, i, 0);
				delete pclsid;
				}
			}
			break;

		case WM_COMMAND:
			{
			switch (HIWORD(wParam))
				{
				case BN_CLICKED:
					{
					switch (LOWORD(wParam))
						{
						case IDOK:
							{
							HWND hwndList;
							hwndList = GetDlgItem(hwndDlg, IDC_LIST);

							int selection = SendMessage(hwndList, LB_GETCURSEL, 0, 0);

							if (selection == -1)
								{
								SendMessage(hwndDlg, FREE_MEMORY, 0, 0);
								EndDialog(hwndDlg, FALSE);
								}

							CLSID *pclsid;
							pclsid = (CLSID *)SendMessage(hwndList, LB_GETITEMDATA, selection, 0);

							PinComControl *pcc = (PinComControl *)GetWindowLong(hwndDlg, GWL_USERDATA);

							pcc->m_d.m_clsid = *pclsid;

							SendMessage(hwndDlg, FREE_MEMORY, 0, 0);
							EndDialog(hwndDlg, TRUE);
							}
							break;

						case IDCANCEL:
							SendMessage(hwndDlg, FREE_MEMORY, 0, 0);
							EndDialog(hwndDlg, FALSE);
							break;
						}
					}
				case LBN_SELCHANGE:
					{
					HWND hwndOk;
					HWND hwndList;

					hwndOk = GetDlgItem(hwndDlg, IDOK);
					hwndList = GetDlgItem(hwndDlg, IDC_LIST);

					int selection = SendMessage(hwndList, LB_GETCURSEL, 0, 0);

					EnableWindow(hwndOk, (selection != -1));
					}
					break;
				}
			}
		}

	return FALSE;
	}

void MyCaxWindow::SetRect(RECT *prc)
	{
	m_rc = *prc;
	}

HRESULT MyCaxWindow::CanInPlaceActivate()
	{
	return S_OK;
	}

HRESULT MyCaxWindow::CanWindowlessActivate()
	{
	return S_FALSE;
	}

HRESULT MyCaxWindow::CreateControl(LPCOLESTR lpszName, IStream* pStream, IUnknown** ppUnkContainer)
	{
	return AtlAxCreateControl(lpszName, m_hWnd, pStream, ppUnkContainer);
	}

HRESULT MyCaxWindow::AttachControl(IUnknown* pUnkControl, HWND hWnd)
	{
		HRESULT hr = S_FALSE;

		ReleaseAll();

		if (m_hWnd != NULL)
		{
			RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_INTERNALPAINT | RDW_FRAME);
			ReleaseWindow();
		}

		if (::IsWindow(hWnd))
		{
			SubclassWindow(hWnd);

			hr = ActivateAx(pUnkControl, TRUE, NULL);

			if (FAILED(hr))
			{
				ReleaseAll();

				if (m_hWnd != NULL)
				{
					RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_INTERNALPAINT | RDW_FRAME);
					ReleaseWindow();
				}
			}
		}
		return hr;
	}

HRESULT MyCaxWindow::ActivateAx(IUnknown* pUnkControl, bool bInited, IStream* pStream)
	{
	if (pUnkControl == NULL)
		return S_OK;

	m_spUnknown = pUnkControl;

	HRESULT hr = S_OK;
	pUnkControl->QueryInterface(IID_IOleObject, (void**)&m_spOleObject);
	if (m_spOleObject)
	{
		m_spOleObject->GetMiscStatus(DVASPECT_CONTENT, &m_dwMiscStatus);
		if(m_dwMiscStatus & OLEMISC_SETCLIENTSITEFIRST)
		{
			CComQIPtr<IOleClientSite> spClientSite(GetControllingUnknown());
			m_spOleObject->SetClientSite(spClientSite);
		}

		CComQIPtr<IPersistStreamInit, &IID_IPersistStreamInit> spPSI(m_spOleObject);
		if (!bInited && spPSI)
		{
			if (pStream)
				spPSI->Load(pStream);
			else
				spPSI->InitNew();
		}

		if(0 == (m_dwMiscStatus & OLEMISC_SETCLIENTSITEFIRST))
		{
			CComQIPtr<IOleClientSite> spClientSite(GetControllingUnknown());
			m_spOleObject->SetClientSite(spClientSite);
		}

		m_dwViewObjectType = 0;
		HRESULT hr;
		hr = m_spOleObject->QueryInterface(IID_IViewObjectEx, (void**) &m_spViewObject);
		if (FAILED(hr))
		{
			hr = m_spOleObject->QueryInterface(IID_IViewObject2, (void**) &m_spViewObject);
			m_dwViewObjectType = 3;
		} else
			m_dwViewObjectType = 7;

		if (FAILED(hr))
		{
			hr = m_spOleObject->QueryInterface(IID_IViewObject, (void**) &m_spViewObject);
			m_dwViewObjectType = 1;
		}
		CComQIPtr<IAdviseSink> spAdviseSink(GetControllingUnknown());
		m_spOleObject->Advise(spAdviseSink, &m_dwOleObject);
		if (m_dwViewObjectType)
			m_spViewObject->SetAdvise(DVASPECT_CONTENT, 0, spAdviseSink);
		m_spOleObject->SetHostNames(OLESTR("AXWIN"), NULL);
	}
	CComPtr<IObjectWithSite> spSite;
	pUnkControl->QueryInterface(IID_IObjectWithSite, (void**)&spSite);
	if (spSite != NULL)
		spSite->SetSite(GetControllingUnknown());

	return hr;
	}

void MyCaxWindow::OnViewChange(DWORD dwAspect, LONG lindex)
	{
	m_ppcc->SetDirtyDraw();
	}

HRESULT MyCaxWindow::GetWindow(HWND* phwnd)
	{
	*phwnd = g_pplayer->m_hwnd;
	return S_OK;
	}

HRESULT MyCaxWindow::GetWindowContext(IOleInPlaceFrame** ppFrame, IOleInPlaceUIWindow** ppDoc, LPRECT lprcPosRect, LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO pFrameInfo)
	{
		HRESULT hr = S_OK;
		if (ppFrame == NULL || ppDoc == NULL || lprcPosRect == NULL || lprcClipRect == NULL)
			hr = E_POINTER;
		ATLASSERT(SUCCEEDED(hr));
		if (SUCCEEDED(hr))
		{
			if (!m_spInPlaceFrame)
			{
				CComObject<CAxFrameWindow>* pFrameWindow;
				CComObject<CAxFrameWindow>::CreateInstance(&pFrameWindow);
				pFrameWindow->QueryInterface(IID_IOleInPlaceFrame, (void**) &m_spInPlaceFrame);
				ATLASSERT(m_spInPlaceFrame);
			}
			if (!m_spInPlaceUIWindow)
			{
				CComObject<CAxUIWindow>* pUIWindow;
				CComObject<CAxUIWindow>::CreateInstance(&pUIWindow);
				pUIWindow->QueryInterface(IID_IOleInPlaceUIWindow, (void**) &m_spInPlaceUIWindow);
				ATLASSERT(m_spInPlaceUIWindow);
			}
			m_spInPlaceFrame.CopyTo(ppFrame);
			m_spInPlaceUIWindow.CopyTo(ppDoc);

			*lprcPosRect = m_rc;
			*lprcClipRect = m_rc;

			ACCEL ac = { 0,0,0 };
			HACCEL hac = CreateAcceleratorTable(&ac, 1);
			pFrameInfo->cb = sizeof(OLEINPLACEFRAMEINFO);
			pFrameInfo->fMDIApp = m_bMDIApp;
			pFrameInfo->hwndFrame = g_pplayer->m_hwnd;
			pFrameInfo->haccel = hac;
			pFrameInfo->cAccelEntries = 1;
		}
		return hr;
	}

HRESULT MyCaxWindow::ShowObject()
	{
	return S_OK;
	}

HRESULT MyCaxWindow::OnInPlaceActivateEx(BOOL* pfNoRedraw, DWORD dwFlags)
	{
	m_bInPlaceActive = TRUE;
	OleLockRunning(m_spOleObject, TRUE, FALSE);
	HRESULT hr = E_FAIL;
	if (FAILED(hr))
	{
		m_bWindowless = FALSE;
		hr = m_spOleObject->QueryInterface(IID_IOleInPlaceObject, (void**) &m_spInPlaceObjectWindowless);
	}
	if (m_spInPlaceObjectWindowless)
		m_spInPlaceObjectWindowless->SetObjectRects(&m_rcPos, &m_rcPos);
	return S_OK;
	}

DispExtender::DispExtender()
	{
	m_cref = 0;
	}

DispExtender::~DispExtender()
	{
	m_pdispEdit->Release();
	if (m_pti2)
		{
		m_pti2->Release();
		}
	}

ULONG DispExtender::AddRef()
	{
	m_cref++;
	m_pdisp->AddRef();
	m_pdispEdit->AddRef();
	return m_cref;
	}

ULONG DispExtender::Release()
	{
	m_cref--;
	m_pdisp->Release();
	m_pdispEdit->Release();
	return m_cref;
	}

HRESULT DispExtender::QueryInterface(REFIID iid, void **ppvObject)
	{
	if (iid == IID_IUnknown)
		{
		AddRef();
		*ppvObject = (IUnknown *)(IDispatch *)this;
		return S_OK;
		}
	if (iid == IID_IDispatch)
		{
		AddRef();
		*ppvObject = (IDispatch *)this;
		return S_OK;
		}
	if (iid == IID_IProvideClassInfo)
		{
		AddRef();
		*ppvObject = (IProvideClassInfo *)this;
		return S_OK;
		}
	if (iid == IID_IProvideMultipleClassInfo)
		{
		AddRef();
		*ppvObject = (IProvideMultipleClassInfo *)this;
		return S_OK;
		}
	else
		{
		return m_pdisp->QueryInterface(iid, ppvObject);
		}
	}

HRESULT DispExtender::GetIDsOfNames(REFIID riid, OLECHAR FAR *FAR *rgszNames, unsigned int cNames, LCID lcid, DISPID FAR *rgDispId)
	{
	HRESULT hr = m_pdisp->GetIDsOfNames(riid, rgszNames, cNames, lcid, rgDispId);
	if (hr == DISP_E_UNKNOWNNAME)
		{
		hr = m_pdispControl->GetIDsOfNames(riid, rgszNames, cNames, lcid, rgDispId);
		}

	return hr;
	}

HRESULT DispExtender::GetTypeInfo(unsigned int iTInfo, LCID lcid, ITypeInfo FAR *FAR *ppTInfo)
	{
	return m_pdisp->GetTypeInfo(iTInfo, lcid, ppTInfo);
	}

HRESULT DispExtender::GetTypeInfoCount(unsigned int FAR *pctinfo)
	{
	*pctinfo = 1;
	return S_OK;
	}

HRESULT DispExtender::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pDispParams, VARIANT FAR *pVarResult, EXCEPINFO FAR *pExcepInfo, unsigned int FAR *puArgErr)
	{
	HRESULT hr = m_pdisp->Invoke(dispIdMember, riid, lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);
	
	if (hr == DISP_E_MEMBERNOTFOUND)
		{
		hr = m_pdispControl->Invoke(dispIdMember, riid, lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);
		}

	return hr;
	}

HRESULT DispExtender::GetClassInfo(ITypeInfo** pptinfoOut)
	{
	*pptinfoOut = m_pti;
	return S_OK;
	}

HRESULT DispExtender::GetGUID(DWORD dwGuidKind, GUID* pGUID)
	{
	return S_OK;
	}

HRESULT DispExtender::GetMultiTypeInfoCount(ULONG *pcti)
	{
	if (m_pti2 == NULL)
		{
		*pcti = 1;
		}
	else
		{
		*pcti = 2;
		}
	return S_OK;
	}

HRESULT DispExtender::GetInfoOfIndex(ULONG iti, DWORD dwFlags, ITypeInfo** pptiCoClass, DWORD* pdwTIFlags, ULONG* pcdispidReserved, IID* piidPrimary, IID* piidSource)
	{
	if (dwFlags & MULTICLASSINFO_GETTYPEINFO)
		{
		*pdwTIFlags  = 0L;
		*pptiCoClass = NULL;
		if (iti == 0)
			{
			*pptiCoClass = m_pti;
			m_pti->AddRef();
			}
		else
			{
			*pptiCoClass = m_pti2;
			m_pti->AddRef();
			}
		}

	return S_OK;
	}


PinComControl::PinComControl()
	{
	m_punk = NULL;
	m_ptemplate = NULL;

	m_pdispextender = NULL;
	}

PinComControl::~PinComControl()
	{
	if (m_punk)
		{
		m_punk->Release();
		}

	if (m_ptemplate)
		{
		delete m_ptemplate;
		}

	if (m_pdispextender)
		{
		delete m_pdispextender;
		}

	if (m_pmcw)
		{
		m_pmcw->FinalRelease();
		}
	}

HRESULT PinComControl::Init(PinTable *ptable, float x, float y)
	{
	HRESULT hr;

	m_ptable = ptable;

	m_d.m_v1.x = x;
	m_d.m_v1.y = y;
	m_d.m_v2.x = x+100;
	m_d.m_v2.y = y+50;

	SetDefaults();

	hr = ChooseComponent();

	if (hr == E_FAIL)
		{
		return hr;
		}

	CreateControl(NULL);

	return InitVBA(fTrue, 0, NULL);
	}

void PinComControl::SetDefaults()
	{
	m_d.m_tdr.m_fTimerEnabled = fFalse;
	m_d.m_tdr.m_TimerInterval = 100;

	m_fBackglass = fTrue;

	WideStrCopy(L"TestVPControl.VPControl.1", m_d.m_progid);
	}

HRESULT PinComControl::ChooseComponent()
	{
	int response = DialogBoxParam(g_hinstres, MAKEINTRESOURCE(IDD_COMLIST),
		m_ptable->m_hwnd, ComListProc, (LPARAM)this);

	if (!response)
		{
		return E_FAIL;
		}

	return S_OK;
	}

void PinComControl::CreateControl(IStream *pstm)
	{
	HRESULT hr;

	AtlAxWinInit();
	//RECT rcWnd = {0, 0, 200, 200};

	CComPtr<IUnknown> spUnkContainer;

	CComObject<MyCaxWindow>::CreateInstance(&m_pmcw);

	m_pmcw->m_ppcc = this;

	IUnknown *punk;
	hr = CoCreateInstance(m_d.m_clsid, NULL, CLSCTX_SERVER, IID_IUnknown, (void **)&punk);

	if (SUCCEEDED(hr))
	{
		m_pmcw->QueryInterface(IID_IAxWinHostWindow, (void**)&m_pAxWindow);
		hr = m_pmcw->ActivateAx(punk, false, pstm);
	}

	m_pdispextender = new DispExtender();
	m_pdispextender->m_pdisp = (IDispatch *)this;
	m_pAxWindow->QueryControl(IID_IDispatch, (void **)&m_pdispextender->m_pdispEdit);
	m_pdispextender->m_pdispControl = m_pdispextender->m_pdispEdit;

	IProvideClassInfo* pClassInfo;
	QueryInterface(IID_IProvideClassInfo, (void **)&pClassInfo);
	ITypeInfo *pti;
	pClassInfo->GetClassInfo(&pti);

	m_pdispextender->m_pti = pti;

	pClassInfo->Release();

	m_pAxWindow->QueryControl(IID_IProvideClassInfo, (void **)&pClassInfo);

	if (pClassInfo)
		{
		pClassInfo->GetClassInfo(&pti);
		m_pdispextender->m_pti = pti;
		pClassInfo->Release();
		m_pdispextender->m_pti2 = pti;
		}
	else
		{
		m_pdispextender->m_pti2 = NULL;
		}

	punk->Release(); // Don't need to keep the IUnknown pointer around anymore
	
	}


LPWORD lpwAlign ( LPWORD lpIn)
{
    ULONG ul;

    ul = (ULONG) lpIn;
    ul +=3;
    ul >>=2;
    ul <<=2;
    return (LPWORD) ul;
}

int nCopyAnsiToWideChar (LPWORD lpWCStr, LPSTR lpAnsiIn)
{
  int nChar = 0;

  do {
    *lpWCStr++ = (WORD) *lpAnsiIn;
    nChar++;
  } while (*lpAnsiIn++);

  return nChar;
}


void PinComControl::CreateControlDialogTemplate()
	{
	IDispatch *pdisp;
	m_pAxWindow->QueryControl(IID_IDispatch, (void **)&pdisp);

	ITypeInfo *piti;

	pdisp->GetTypeInfo(0, 0x409, &piti);

	TYPEATTR *pta;

	piti->GetTypeAttr(&pta);

	int cfunc = pta->cFuncs;

	int cdispfuncs = 0; // Count of functions that will appear in the dialog
	int cfunccur = 0; // Function we are currently creating a template for

	FUNCDESC *pfd;
	for (int i=0;i<cfunc;i++)
		{
		piti->GetFuncDesc(i, &pfd);

		if (pfd->invkind == INVOKE_PROPERTYPUT && !((pfd->wFuncFlags & FUNCFLAG_FHIDDEN) || (pfd->wFuncFlags & FUNCFLAG_FRESTRICTED)))
			{
			cdispfuncs++;
			}

		piti->ReleaseFuncDesc(pfd);
		}

	WORD *p, *pdlgtemplate;
	pdlgtemplate = p = new WORD[100 + (100*cdispfuncs*2)];//(PWORD)LocalAlloc (LPTR, 100 + (100*cdispfuncs*2));

	/* Start to fill in the dlgtemplate information, addressing by WORDs. */ 
	DWORD lStyle = WS_CHILD /*WS_POPUP*/ | WS_VISIBLE | DS_SETFONT | DS_3DLOOK;
	*p++ = 1;          // DlgVer
	*p++ = 0xFFFF;     // Signature
	*p++ = 0;          // LOWORD HelpID
	*p++ = 0;          // HIWORD HelpID
	*p++ = 0;          // LOWORD (lExtendedStyle)
	*p++ = 0;          // HIWORD (lExtendedStyle)
	*p++ = LOWORD (lStyle);
	*p++ = HIWORD (lStyle);
	WORD * const pitemcount = p;
	*p++ = cdispfuncs*2;          // NumberOfItems
	*p++ = 0;        // x
	*p++ = 0;         // y
	*p++ = 100;        // cx
	WORD * const pdialogheight = p;
	*p++ = 200;        // cy
	*p++ = 0;          // Menu
	*p++ = 0;          // Class

	int nchar;

	/* Copy the title of the dialog box. */ 
	//int nchar = nCopyAnsiToWideChar (p, TEXT("Dialog"));
	//p += nchar;
	*p++ = 0;

	   /* Font information because of DS_SETFONT. */ 
		  *p++ = 8;  // Point size 
		  *p++ = FW_DONTCARE;  // Weight
		  *p++ = MAKEWORD( FALSE, DEFAULT_CHARSET );  // italic flag and charset.

	nchar = nCopyAnsiToWideChar (p, TEXT("MS Sans Serif"));  // Face name
	p += nchar;

	/* Make sure the first item starts on a DWORD boundary. */ 
	p = lpwAlign (p);

	for (int i=0;i<cfunc;i++)
		{
		piti->GetFuncDesc(i, &pfd);

		if (pfd->invkind == INVOKE_PROPERTYPUT && !((pfd->wFuncFlags & FUNCFLAG_FHIDDEN) || (pfd->wFuncFlags & FUNCFLAG_FRESTRICTED)))
			{
			char *szControlName;
			DWORD lStyleControl;

			//pfd->elemdescFunc.tdesc.vt;
			VARTYPE vt = pfd->lprgelemdescParam[0].tdesc.vt;

			switch (vt)
				{
				case VT_BSTR:
				case VT_I2:
				case VT_I4:
				case VT_R4:
				case VT_R8:
				case VT_DATE:
				case VT_CY:
				case VT_DECIMAL:
				case VT_VARIANT:
				case VT_I1:
				case VT_UI1:
				case VT_UI2:
				case VT_UI4:
				case VT_INT:
				case VT_UINT:
					lStyleControl = ES_AUTOHSCROLL;
					szControlName = "EDIT";
					break;

				case VT_BOOL:
					szControlName = "BUTTON";
					lStyleControl = BS_AUTOCHECKBOX;
					break;

				case VT_USERDEFINED:
					{
					ITypeInfo *pitiUser;
					piti->GetRefTypeInfo(pfd->lprgelemdescParam[0].tdesc.hreftype, &pitiUser);
					if (pitiUser)
						{
						TYPEATTR *pta2;
						pitiUser->GetTypeAttr(&pta2);

						if (InlineIsEqualGUID(pta2->guid, GUID_OLE_COLOR))
							{
							lStyleControl = BS_PUSHBUTTON;
							szControlName = "ColorControl";
							}

						pitiUser->ReleaseTypeAttr(pta2);
						pitiUser->Release();
						}
					}
					break;

				default:
					cdispfuncs--;
					piti->ReleaseFuncDesc(pfd);
					continue;
					break;
				}

			BSTR *rgstr = (BSTR *) CoTaskMemAlloc(6 * sizeof(BSTR *));

			unsigned int cnames;
			/*const HRESULT hr =*/ piti->GetNames(pfd->memid, rgstr, 6, &cnames);

			/* Now start with the first item. */ 
			lStyle = WS_VISIBLE | WS_CHILD;
			*p++ = 0;          // LOWORD (lHelpID)
			*p++ = 0;          // HIWORD (lHelpID)
			*p++ = 0;          // LOWORD (lExtendedStyle)
			*p++ = 0;          // HIWORD (lExtendedStyle)
			*p++ = LOWORD (lStyle);
			*p++ = HIWORD (lStyle);
			*p++ = 7;         // x
			*p++ = 11+16*cfunccur;         // y
			*p++ = 43;         // cx
			*p++ = 12;         // cy
			*p++ = LOWORD(-1);       // LOWORD (Control ID)
			*p++ = HIWORD(-1);      // HOWORD (Control ID)

			/* Fill in class i.d., this time by name. */ 
			nchar = nCopyAnsiToWideChar (p, TEXT("STATIC"));
			p += nchar;

			/* Copy the text of the first item. */
			//nchar = nCopyAnsiToWideChar (p, TEXT(L"OK"));
			//p += nchar;
			WideStrCopy(rgstr[0], (WCHAR *)p);
			p+=(lstrlenW(rgstr[0])+1);

			*p++ = 0;  // Advance pointer over nExtraStuff WORD.

			/* make sure the second item starts on a DWORD boundary. */ 
			p = lpwAlign (p);

			lStyleControl |= WS_VISIBLE | WS_CHILD | WS_TABSTOP;

			//DWORD lstyleEx = WS_EX_CLIENTEDGE;

			*p++ = 0;          // LOWORD (lHelpID)
			*p++ = 0;          // HIWORD (lHelpID)
			*p++ = 0;//LOWORD (lstyleEx);          // LOWORD (lExtendedStyle)
			*p++ = 0;//LOWORD (lstyleEx);          // HIWORD (lExtendedStyle)
			*p++ = LOWORD (lStyleControl);
			*p++ = HIWORD (lStyleControl);
			*p++ = 56;         // x
			*p++ = 10+16*cfunccur;         // y
			*p++ = 40;         // cx
			*p++ = 12;         // cy
			*p++ = LOWORD(pfd->memid);       // LOWORD (Control ID)
			*p++ = HIWORD(pfd->memid);      // HOWORD (Control ID)

			/* Fill in class i.d., this time by name. */

			nchar = nCopyAnsiToWideChar (p, szControlName);
			p += nchar;
			
			/* Copy the text of the first item. */ 
			nchar = nCopyAnsiToWideChar (p, TEXT(""));
			p += nchar;

			*p++ = 0;  // Advance pointer over nExtraStuff WORD.

			/* make sure the second item starts on a DWORD boundary. */ 
			p = lpwAlign (p);
			cfunccur++;

			for (unsigned int i2=0; i2 < cnames; i2++)
				{
				SysFreeString(rgstr[i2]);
				}

			CoTaskMemFree(rgstr);
			}

		piti->ReleaseFuncDesc(pfd);
		}

	piti->ReleaseTypeAttr(pta);

	piti->Release();

	pdisp->Release();

	*pitemcount = cdispfuncs*2;  // NumberOfItems (post-calculated)
	*pdialogheight = 10+16*cdispfuncs;

	m_ptemplate = (DLGTEMPLATE *)pdlgtemplate;

	}


void PinComControl::PreRender(Sur *psur)
	{
	// Draw an invisible box to give us something to hittest
	psur->SetBorderColor(-1,fFalse,0);
	psur->SetFillColor(-1);
	psur->SetObject(this);

	psur->Rectangle(m_d.m_v1.x, m_d.m_v1.y, m_d.m_v2.x, m_d.m_v2.y);

	int savestate = SaveDC(psur->m_hdc);

	int x1,x2;
	int y1,y2;
	psur->SurfaceToScreen(m_d.m_v1.x, m_d.m_v1.y, &x1, &y1);
	psur->SurfaceToScreen(m_d.m_v2.x, m_d.m_v2.y, &x2, &y2);

	IntersectClipRect(psur->m_hdc, x1, y1, x2, y2);

	IViewObject *pvo;
	m_pAxWindow->QueryControl(IID_IViewObject, (void **)&pvo);

	RECT rc;
	rc.left = x1;
	rc.top = y1;
	rc.right = x2;
	rc.bottom = y2;

	/*HRESULT hr =*/ pvo->Draw(DVASPECT_CONTENT, 0, NULL, NULL, NULL, psur->m_hdc,
		(const struct _RECTL *)&rc, NULL, NULL, NULL);

	pvo->Release();

	RestoreDC(psur->m_hdc, savestate);
	
	}


void PinComControl::Render(Sur *psur)
	{
	psur->SetBorderColor(RGB(0,0,0),fFalse,0);
	psur->SetFillColor(-1);
	psur->SetObject(this);
	psur->SetObject(NULL);

	psur->Rectangle(m_d.m_v1.x, m_d.m_v1.y, m_d.m_v2.x, m_d.m_v2.y);
	}


void PinComControl::GetCenter(Vertex2D *pv)
	{
	}

void PinComControl::PutCenter(Vertex2D *pv)
	{
	}

void PinComControl::SetObjectPos()
	{
	g_pvp->SetObjectPosCur(m_d.m_v1.x, m_d.m_v1.y);
	}

void PinComControl::MoveOffset(float dx, float dy)
	{
	m_d.m_v1.x += dx;
	m_d.m_v1.y += dy;

	m_d.m_v2.x += dx;
	m_d.m_v2.y += dy;

	m_ptable->SetDirtyDraw();
	}

void PinComControl::GetHitShapes(Vector<HitObject> *pvho)
	{
	}

void PinComControl::GetHitShapesDebug(Vector<HitObject> *pvho)
	{
	}

void PinComControl::GetTimers(Vector<HitTimer> *pvht)
	{
	IEditable::BeginPlay();

	HitTimer * const pht = new HitTimer();
	pht->m_interval = m_d.m_tdr.m_TimerInterval;
	pht->m_nextfire = pht->m_interval;
	pht->m_pfe = (IFireEvents *)this;

	Pin3D *ppin3d = &g_pplayer->m_pin3d;

	m_phittimer = pht;

	if (m_d.m_tdr.m_fTimerEnabled)
		{
		pvht->AddElement(pht);
		}

	CComPtr<IUnknown> spUnkContainer;
	
	CComObject<MyCaxWindow>::CreateInstance(&m_pmcwPlayer);

	RECT rcWnd;

	m_pmcwPlayer->m_ppcc = this;

	rcWnd.left = (int)((m_d.m_v1.x / 1000) * ppin3d->m_dwRenderWidth); // substitute 640 for play window width
	rcWnd.top = (int)((m_d.m_v1.y / 750) * ppin3d->m_dwRenderHeight);
	rcWnd.right = (int)((m_d.m_v2.x / 1000) * ppin3d->m_dwRenderWidth);
	rcWnd.bottom = (int)((m_d.m_v2.y / 750) * ppin3d->m_dwRenderHeight);

	m_pmcwPlayer->SetRect(&rcWnd);

	HRESULT hr;
	IUnknown *punk;
	hr = CoCreateInstance(m_d.m_clsid, NULL, CLSCTX_SERVER, IID_IUnknown, (void **)&punk);

	if (SUCCEEDED(hr))
	{
		m_pmcwPlayer->QueryInterface(IID_IAxWinHostWindow, (void**)&m_pAxWindowPlayer);
		hr = m_pmcwPlayer->ActivateAx(punk, false, NULL);
	}

	CComQIPtr<IOleClientSite> spClientSite(m_pmcwPlayer->GetControllingUnknown());
	hr = m_pmcwPlayer->m_spOleObject->DoVerb(OLEIVERB_INPLACEACTIVATE, NULL, spClientSite, 0, g_pplayer->m_hwnd, &rcWnd);

	m_pAxWindowPlayer->QueryControl(IID_IDispatch, (void **)&m_pdispextender->m_pdispPlayer);
	m_pdispextender->m_pdispControl = m_pdispextender->m_pdispPlayer;

	punk->Release(); // Don't need to keep the IUnknown pointer around anymore

	// If in windowed-mode, create a clipper object
    LPDIRECTDRAWCLIPPER pcClipper;
    hr = g_pplayer->m_pin3d.m_pDD->CreateClipper( 0, &pcClipper, NULL );

    // Associate the clipper with the window
    pcClipper->SetHWnd(0, g_pplayer->m_hwnd);
    g_pplayer->m_pin3d.m_pddsFrontBuffer->SetClipper(pcClipper);
    if (pcClipper)
		{
		pcClipper->Release();
		}
	}

void PinComControl::EndPlay()
	{
	m_pdispextender->m_pdispControl = m_pdispextender->m_pdispEdit;
	m_pdispextender->m_pdispPlayer->Release();

	if (m_pmcwPlayer)
		{
		m_pmcwPlayer->FinalRelease();
		}

	IEditable::EndPlay();
	}

void PinComControl::PostRenderStatic(LPDIRECT3DDEVICE7 pd3dDevice)
	{
	}

void PinComControl::RenderStatic(LPDIRECT3DDEVICE7 pd3dDevice)
	{
	}
	
void PinComControl::RenderMoversFromCache(Pin3D *ppin3d)
	{
	}

void PinComControl::RenderMovers(LPDIRECT3DDEVICE7 pd3dDevice)
	{
	}

HRESULT PinComControl::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
	{
	BiffWriter bw(pstm, hcrypthash, hcryptkey);

	bw.WriteStruct(FID(VER1), &m_d.m_v1, sizeof(Vertex2D));
	bw.WriteStruct(FID(VER2), &m_d.m_v2, sizeof(Vertex2D));

	bw.WriteBool(FID(TMON), m_d.m_tdr.m_fTimerEnabled);
	bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);

	bw.WriteWideString(FID(PROG), m_d.m_progid);

	ISelect::SaveData(pstm, hcrypthash, hcryptkey);

	if (hcrypthash != NULL) // don't save object properties for undo
		{
		IPersistStreamInit *pps;
		m_pAxWindow->QueryControl(IID_IPersistStreamInit, (void **)&pps);
		if (pps)
			{
			ULONG written;

			bw.WriteTag(FID(STRM));

			LARGE_INTEGER uiT;
			ULARGE_INTEGER uiSeekRecordPlacement;
			ULARGE_INTEGER uiSeekRecord;

			LARGE_INTEGER iSeekRecordPlacement;
			LARGE_INTEGER iSeekRecord;

			uiT.QuadPart = 0;

			pstm->Seek(uiT, STREAM_SEEK_CUR, &uiSeekRecordPlacement);
			uiT.QuadPart = sizeof(ULARGE_INTEGER);
			pstm->Seek(uiT, STREAM_SEEK_CUR, NULL); // Skip room to save the seek record later

			pps->Save(pstm, false);

			uiT.QuadPart = 0;
			pstm->Seek(uiT, STREAM_SEEK_CUR, &uiSeekRecord);

			iSeekRecordPlacement.QuadPart = uiSeekRecordPlacement.QuadPart;
			pstm->Seek(iSeekRecordPlacement, STREAM_SEEK_SET, NULL);
			bw.WriteBytes(&uiSeekRecord, sizeof(ULARGE_INTEGER), &written);
			iSeekRecord.QuadPart = uiSeekRecord.QuadPart;
			pstm->Seek(iSeekRecord, STREAM_SEEK_SET, NULL);

			// Now we're where we should be at the end of the control's save stream,
			// having written the seek info so we know how big the object's save stream is

			pps->Release();
			}
		}

	bw.WriteTag(FID(ENDB));

	return S_OK;
	}

HRESULT PinComControl::InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
	{
	SetDefaults();

	BiffReader br(pstm, this, pid, version, hcrypthash, hcryptkey);

	m_ptable = ptable;

	br.Load();
	return S_OK;
	}

BOOL PinComControl::LoadToken(int id, BiffReader *pbr)
	{
	if (id == FID(PIID))
		{
		pbr->GetInt((int *)pbr->m_pdata);
		}
	else if (id == FID(VER1))
		{
		pbr->GetStruct(&m_d.m_v1, sizeof(Vertex2D));
		}
	else if (id == FID(VER2))
		{
		pbr->GetStruct(&m_d.m_v2, sizeof(Vertex2D));
		}
	else if (id == FID(TMON))
		{
		pbr->GetBool(&m_d.m_tdr.m_fTimerEnabled);
		}
	else if (id == FID(TMIN))
		{
		pbr->GetInt(&m_d.m_tdr.m_TimerInterval);
		}
	else if (id == FID(PROG))
		{
		pbr->GetWideString(m_d.m_progid);
		}
	else if (id == FID(STRM))
		{
		CComPtr<IUnknown> spUnk;
		//AtlAxGetControl(m_pAxWindow->m_hWnd, &spUnk);

		ULONG read;
		LARGE_INTEGER iSeekRecord;
		pbr->ReadBytes(&iSeekRecord, sizeof(LARGE_INTEGER), &read);

		if (m_pAxWindow != NULL)
			{
			// Control already exists - must be undoing something
			}
		else
			{
			CreateControl(pbr->m_pistream);
			}

		pbr->m_pistream->Seek(iSeekRecord, STREAM_SEEK_SET, NULL);
		}
	else
		{
		ISelect::LoadToken(id, pbr);
		}

	return fTrue;
	}

HRESULT PinComControl::InitPostLoad()
	{
	return S_OK;
	}

void PinComControl::GetDialogPanes(Vector<PropertyPane> *pvproppane)
	{
	PropertyPane *pproppane;

	pproppane = new PropertyPane(IDD_PROP_NAME, NULL);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROPTEXTBOX_POSITION, IDS_POSITION);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROP_TIMER, IDS_MISC);
	pvproppane->AddElement(pproppane);

	if (m_ptemplate == NULL)
		{
		CreateControlDialogTemplate();
		}

	pproppane = new PropertyPane(m_ptemplate, IDS_MISC);
	pvproppane->AddElement(pproppane);
	}

STDMETHODIMP PinComControl::get_Width(float *pVal)
{
	*pVal = m_d.m_v2.x - m_d.m_v1.x;

	return S_OK;
}

STDMETHODIMP PinComControl::put_Width(float newVal)
{
	STARTUNDO

	m_d.m_v2.x = m_d.m_v1.x + newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP PinComControl::get_Height(float *pVal)
{
	*pVal = m_d.m_v2.y - m_d.m_v1.y;

	return S_OK;
}

STDMETHODIMP PinComControl::put_Height(float newVal)
{
	STARTUNDO

	m_d.m_v2.y = m_d.m_v1.y + newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP PinComControl::get_X(float *pVal)
{
	*pVal = m_d.m_v1.x;

	return S_OK;
}

STDMETHODIMP PinComControl::put_X(float newVal)
{
	STARTUNDO

	float delta = newVal - m_d.m_v1.x;

	m_d.m_v1.x += delta;
	m_d.m_v2.x += delta;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP PinComControl::get_Y(float *pVal)
{
	*pVal = m_d.m_v1.y;

	return S_OK;
}

STDMETHODIMP PinComControl::put_Y(float newVal)
{
	STARTUNDO

	float delta = newVal - m_d.m_v1.y;

	m_d.m_v1.y += delta;
	m_d.m_v2.y += delta;

	STOPUNDO

	return S_OK;
}
