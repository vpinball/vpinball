#include "StdAfx.h"


IEditable::IEditable()
	{
	m_phittimer = NULL;

	m_fBackglass = false;
	m_isVisible = true;
	VariantInit(&m_uservalue);
	}

IEditable::~IEditable()
	{
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

    if (GetScriptable())
        GetPTable()->m_pcv->RemoveItem(GetScriptable());

	for (int i=0;i<m_vCollection.Size();i++)
		{
		Collection *pcollection = m_vCollection.ElementAt(i);
		pcollection->m_visel.RemoveElement(GetISelect());
		}
	}

void IEditable::Uncreate()
	{
	GetPTable()->m_vedit.RemoveElement((IEditable *)this);
    if (GetScriptable())
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
				m_phittimer->m_nextfire = g_pplayer->m_time_msec + m_phittimer->m_interval;
				g_pplayer->m_vht.AddElement(m_phittimer);
				}
			else
				g_pplayer->m_vht.RemoveElement(m_phittimer);
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
		m_phittimer->m_nextfire = g_pplayer->m_time_msec + m_phittimer->m_interval;
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
	/*const HRESULT hr =*/ VariantCopy(&m_uservalue, newVal);
	
	STOPUNDO
	
	return S_OK;
	}

void IEditable::BeginPlay()
	{
	m_vEventCollection.RemoveAllElements();
	m_viEventCollection.RemoveAllElements();

	m_fSingleEvents = true;
	for(int i=0;i<m_vCollection.Size();i++)
		{
		Collection *pcol = m_vCollection.ElementAt(i);
		if (pcol->m_fFireEvents)
			{
			m_vEventCollection.AddElement(pcol);
			m_viEventCollection.AddElement(m_viCollection.ElementAt(i));
			}
		if (pcol->m_fStopSingleEvents)
			m_fSingleEvents = false;
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
	GetPTable()->m_undo.EndUndo();
	}

void IEditable::Undelete()
	{
	InitVBA(fTrue, 0, (WCHAR *)this);

	for (int i=0;i<m_vCollection.Size();i++)
		{
		Collection *pcollection = m_vCollection.ElementAt(i);
		pcollection->m_visel.AddElement(GetISelect());
		}
	}

void IEditable::InitScript()
	{
    if (!GetScriptable())
        return;

	if (lstrlenW(GetScriptable()->m_wzName) == 0)
		// Just in case something screws up - not good having a null script name
		swprintf_s(GetScriptable()->m_wzName, L"%d", (long)this);

	GetPTable()->m_pcv->AddItem(GetScriptable(), fFalse);
	}
