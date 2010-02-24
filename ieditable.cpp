#include "stdafx.h"
#include "main.h"

// TEXT
const WCHAR rgwzTypeName[][16] = {L"Wall",
								L"Flipper",
								L"Timer",
								L"Plunger",
								L"Textbox",
								L"Bumper",
								L"Trigger",
								L"Light",
								L"Kicker",
								L"Decal",
								L"Gate",
								L"Spinner",
								L"Ramp",
                                L"DispReel",    //>>> added by Chris
                            };

IEditable::IEditable()
	{
	m_phittimer = NULL;

	m_wzVBAName = NULL;
	m_wzVBACode = NULL;

	m_fBackglass = fFalse;
	
	VariantInit(&m_uservalue);

	//m_pcollection = NULL;
	//m_iCollection = -1;
	}

IEditable::~IEditable()
	{
	if (m_wzVBAName)
		{
		delete m_wzVBAName;
		}

	if (m_wzVBACode)
		{
		delete m_wzVBACode;
		}
	}

Hitable *IEditable::GetIHitable()
	{
	return NULL;
	}

void IEditable::SetDirtyDraw()
	{
	GetPTable()->SetDirtyDraw();
	}

void IEditable::ClearForOverwrite()
	{
	}

void IEditable::Delete()
	{
	GetPTable()->m_vedit.RemoveElement((IEditable *)this);
	MarkForDelete();
	APCPROJECTUNDEFINE
	GetPTable()->m_pcv->RemoveItem(GetScriptable());
	int i;
	for (i=0;i<m_vCollection.Size();i++)
		{
		Collection *pcollection = m_vCollection.ElementAt(i);
		pcollection->m_visel.RemoveElement(GetISelect());
		}
	}

void IEditable::Uncreate()
	{
	GetPTable()->m_vedit.RemoveElement((IEditable *)this);
	APCPROJECTUNDEFINE
	GetPTable()->m_pcv->RemoveItem(GetScriptable());
	}

HRESULT IEditable::put_TimerEnabled(VARIANT_BOOL newVal, BOOL *pte)
	{
	STARTUNDO

	BOOL fNew = VBTOF(newVal);

	if (fNew != *pte)
		{
		if (m_phittimer)
			{
			if (fNew)
				{
				m_phittimer->m_nextfire = g_pplayer->m_timeCur + m_phittimer->m_interval;
				g_pplayer->m_vht.AddElement(m_phittimer);
				}
			else
				{
				g_pplayer->m_vht.RemoveElement(m_phittimer);
				}
			}
		}

	*pte = fNew;

	STOPUNDO

	return S_OK;
	}

HRESULT IEditable::put_TimerInterval(long newVal, int *pti)
	{
	STARTUNDO

	*pti = newVal;

	if (m_phittimer)
		{
		m_phittimer->m_interval = newVal;
		m_phittimer->m_nextfire = g_pplayer->m_timeCur + m_phittimer->m_interval;
		}

	STOPUNDO

	return S_OK;
	}

HRESULT IEditable::get_UserValue(VARIANT *pVal)
	{
	VariantClear(pVal);
	VariantCopy(pVal, &m_uservalue);
	return S_OK;
	}

HRESULT IEditable::put_UserValue(VARIANT *newVal)
	{
	STARTUNDO

	VariantInit(&m_uservalue);
	VariantClear(&m_uservalue);
	HRESULT hr = VariantCopy(&m_uservalue, newVal);
	
	STOPUNDO
	
	return S_OK;
	}

void IEditable::BeginPlay()
	{
	m_vEventCollection.RemoveAllElements();
	m_viEventCollection.RemoveAllElements();

	int i;
	m_fSingleEvents = fTrue;
	for(i=0;i<m_vCollection.Size();i++)
		{
		Collection *pcol = m_vCollection.ElementAt(i);
		if (pcol->m_fFireEvents)
			{
			m_vEventCollection.AddElement(pcol);
			m_viEventCollection.AddElement(m_viCollection.ElementAt(i));
			}
		if (pcol->m_fStopSingleEvents)
			{
			m_fSingleEvents = fFalse;
			}
		}
	}

void IEditable::EndPlay()
	{
	if (m_phittimer)
		{
		delete m_phittimer;
		m_phittimer = NULL;
		}
	}

void IEditable::RenderBlueprint(Sur *psur)
	{
	Render(psur);
	}

void IEditable::RenderShadow(ShadowSur *psur, float height)
	{
	}

void IEditable::BeginUndo()
	{
	GetPTable()->BeginUndo();
	}

void IEditable::EndUndo()
	{
	GetPTable()->EndUndo();
	}

void IEditable::MarkForUndo()
	{
	GetPTable()->m_undo.MarkForUndo(this);
	}

void IEditable::MarkForDelete()
	{
	GetPTable()->m_undo.BeginUndo();
	GetPTable()->m_undo.MarkForDelete(this);

#ifdef VBA
	CComBSTR bstr;
	if (GetIApcProjectItem())
		{
		GetIApcProjectItem()->get_Name(&bstr);
		int len = bstr.Length();
		// len+1 for trailing null terminator
		m_wzVBAName = new WCHAR[len+1];
		memcpy(m_wzVBAName, (WCHAR *)bstr, (len+1)*sizeof(WCHAR));

		_VBComponent* pComponent;
		VBIDE::_CodeModule* pCodeModule;
		// Gets the corresponding VBComponent
		GetIApcProjectItem()->get_VBComponent(&pComponent);
		// Gets the corresponding CodeModule
		pComponent->get_CodeModule((VBIDE::CodeModule**)&pCodeModule);
		pComponent->Release();
		CComBSTR bstrCode;
		long count;
		pCodeModule->get_CountOfLines(&count);
		// Lines are 1-based
		HRESULT hr = pCodeModule->get_Lines(1, count, &bstrCode);
		len = bstrCode.Length();
		if (len > 0)
			{
			m_wzVBACode = new WCHAR[len+1];
			memcpy(m_wzVBACode, (WCHAR *)bstrCode, (len+1)*sizeof(WCHAR));
			}
		}
#endif

	GetPTable()->m_undo.EndUndo();
	}

void IEditable::Undelete()
	{
#ifdef VBA
	InitVBA(fTrue, 0, m_wzVBAName);
#else
	InitVBA(fTrue, 0, (WCHAR *)this);
#endif

	int i;
	for (i=0;i<m_vCollection.Size();i++)
		{
		Collection *pcollection = m_vCollection.ElementAt(i);
		pcollection->m_visel.AddElement(GetISelect());
		}

	if (!m_wzVBAName)
		{
		// Not a project item (Decal)
		return;
		}

	delete m_wzVBAName;
	m_wzVBAName = NULL;

#ifdef VBA
	if (m_wzVBACode)
		{
		_VBComponent* pComponent;
		VBIDE::_CodeModule* pCodeModule;
		// Gets the corresponding VBComponent
		GetIApcProjectItem()->get_VBComponent(&pComponent);
		// Gets the corresponding CodeModule
		pComponent->get_CodeModule((VBIDE::CodeModule**)&pCodeModule);
		pComponent->Release();
		CComBSTR bstrCode = m_wzVBACode;
		HRESULT hr = pCodeModule->InsertLines(1, bstrCode);

		delete m_wzVBACode;
		m_wzVBACode = NULL;
		}
#endif
	}

void IEditable::InitScript()
	{
	if (lstrlenW((WCHAR *)GetScriptable()->m_wzName) == 0)
		{
		// Just in case something screws up - not good having a null script name
		// TODO
		//swprintf((WCHAR *)GetScriptable()->m_wzName, L"%d", (long)this);
		}

	GetPTable()->m_pcv->AddItem(GetScriptable(), fFalse);
	}

/*HRESULT IEditable::SaveUndoData(IStream *pstm)
	{
	HRESULT hr = S_OK;

	_VBComponent* pComponent;
	VBIDE::_CodeModule* pCodeModule;
	// Gets the corresponding VBComponent
	GetIApcProjectItem()->get_VBComponent(&pComponent);
	// Gets the corresponding CodeModule
	pComponent->get_CodeModule((VBIDE::CodeModule**)&pCodeModule);
	pComponent->Release();
	CComBSTR bstr;
	long count;
	pCodeModule->get_CountOfLines(&count);
	HRESULT hr = pCodeModule->get_Lines(1, count, &bstr);

	int len = lstrlenW(bstr);

	ULONG writ = 0;

	if(FAILED(hr = pstm->Write(&len, sizeof(len), &writ)))
		return hr;

	if(FAILED(hr = pstm->Write((WCHAR *)bstr, len*sizeof(WCHAR), &writ)))
		return hr;

	SaveData(pstm);

	return hr;
	}

/*STDMETHODIMP IEditable::GetPredefinedStrings(DISPID dispID, CALPOLESTR *pcaStringsOut, CADWORD *pcaCookiesOut)
	{
    if (dispID != DISPID_Image && dispID != DISPID_Image2)
		{
		return hrNotImplemented;
		}

	if (pcaStringsOut == NULL || pcaCookiesOut == NULL)
		return S_OK;

	int cvar = GetPTable()->m_vimage.Size();
	int ivar;

	WCHAR **rgstr = (WCHAR **) CoTaskMemAlloc(cvar * sizeof(WCHAR *));
	DWORD *rgdw = (DWORD *) CoTaskMemAlloc(cvar * sizeof DWORD);

	for (ivar = 0 ; ivar < cvar ; ivar++)
		{
		char *szSrc = GetPTable()->m_vimage.ElementAt(ivar)->m_szName;
		DWORD cwch = lstrlen(szSrc)+1;
		WCHAR *wzDst = (WCHAR *) CoTaskMemAlloc(cwch*sizeof(WCHAR));
		if (wzDst == NULL)
			{
			ShowError("Damn");
			}

		MultiByteToWideChar(CP_ACP, 0, szSrc, -1, wzDst, cwch);

		//MsoWzCopy(szSrc,szDst);
		rgstr[ivar] = wzDst;
		rgdw[ivar] = ivar;
		}

	pcaStringsOut->cElems = cvar;
	pcaStringsOut->pElems = rgstr;

	pcaCookiesOut->cElems = cvar;
	pcaCookiesOut->pElems = rgdw;

	return S_OK;
	}

STDMETHODIMP IEditable::GetPredefinedValue(DISPID dispID, DWORD dwCookie, VARIANT *pVarOut)
	{
    if (dispID != DISPID_Image && dispID != DISPID_Image2)
		{
		return hrNotImplemented;
		}

	char *szSrc = GetPTable()->m_vimage.ElementAt(dwCookie)->m_szName;
	DWORD cwch = lstrlen(szSrc)+1;
	WCHAR *wzDst = (WCHAR *) CoTaskMemAlloc(cwch*sizeof(WCHAR));

	MultiByteToWideChar(CP_ACP, 0, szSrc, -1, wzDst, cwch);

	CComVariant var(wzDst);

	CoTaskMemFree(wzDst);

	return var.Detach(pVarOut);
	}*/
