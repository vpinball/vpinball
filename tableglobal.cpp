#include "StdAfx.h"

STDMETHODIMP ScriptGlobalTable::BeginModal()
{
	if (g_pplayer)
		{
		g_pplayer->m_ModalRefCount++;
		}

	return S_OK;
}

STDMETHODIMP ScriptGlobalTable::EndModal()
{
	if (g_pplayer)
		{
		if (g_pplayer->m_ModalRefCount > 0)
			{
			g_pplayer->m_ModalRefCount--;
			}
		g_pplayer->m_LastKnownGoodCounter++;
		}

	return S_OK;
}
