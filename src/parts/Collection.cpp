#include "core/stdafx.h"

#include "Collection.h"

Collection::Collection()
{
   m_fireEvents = false;
   m_stopSingleEvents = false;

   m_groupElements = g_app->m_settings.GetEditor_GroupElementsInCollection();
}

HRESULT Collection::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, const bool saveForUndo)
{
   BiffWriter bw(pstm, hcrypthash);

   bw.WriteWideString(FID(NAME), m_wzName);

   for (int i = 0; i < m_visel.size(); ++i)
   {
      const IScriptable * const piscript = m_visel[i].GetIEditable()->GetScriptable();
      bw.WriteWideString(FID(ITEM), piscript->m_wzName);
   }

   bw.WriteBool(FID(EVNT), m_fireEvents);
   bw.WriteBool(FID(SSNG), m_stopSingleEvents);
   bw.WriteBool(FID(GREL), m_groupElements);

   bw.WriteTag(FID(ENDB));

   return S_OK;
}

HRESULT Collection::LoadData(IStream *pstm, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   BiffReader br(pstm, this, version, hcrypthash, hcryptkey);

   br.Load();
   return S_OK;
}

bool Collection::LoadToken(const int id, BiffReader * const pbr)
{
   switch(id)
   {
   case FID(NAME):
   {
      //!! workaround: due to a bug in earlier versions, it can happen that the string written was one char too long
      WCHAR tmp[MAXNAMEBUFFER+1];
      pbr->GetWideString(tmp, MAXNAMEBUFFER+1);
      memcpy(m_wzName, tmp, (MAXNAMEBUFFER-1)*sizeof(WCHAR));
      m_wzName[MAXNAMEBUFFER-1] = L'\0';
      break;
   }
   case FID(EVNT): pbr->GetBool(m_fireEvents); break;
   case FID(SSNG): pbr->GetBool(m_stopSingleEvents); break;
   case FID(GREL): pbr->GetBool(m_groupElements); break;
   case FID(ITEM):
   {
      //!! workaround: due to a bug in earlier versions, it can happen that the string written was twice the size
      WCHAR wzT[MAXNAMEBUFFER*2];
      pbr->GetWideString(wzT, MAXNAMEBUFFER*2); //!! rather truncate for these special cases for the comparison in InitPostLoad?

      m_tmp_isel_name.push_back(wzT);
      break;
   }
   }
   return true;
}

HRESULT Collection::InitPostLoad(PinTable *const pt)
{
   for (size_t n = 0; n < m_tmp_isel_name.size(); ++n)
   for (size_t i = 0; i < pt->m_vedit.size(); ++i)
   {
      IScriptable *const piscript = pt->m_vedit[i]->GetScriptable();
      if (piscript) // skip decals
      {
         if (piscript->m_wzName == m_tmp_isel_name[n])
         {
            auto iselect = piscript->GetISelect();
            iselect->GetIEditable()->m_vCollection.push_back(this);
            iselect->GetIEditable()->m_viCollection.push_back(m_visel.size());
            m_visel.push_back(iselect);
            break; // found, continue to search next name/element
         }
      }
   }
   m_tmp_isel_name.clear();

   return S_OK;
}

STDMETHODIMP Collection::get_Count(LONG __RPC_FAR *plCount)
{
   *plCount = m_visel.size();
   return S_OK;
}

STDMETHODIMP Collection::get_Item(LONG index, IDispatch __RPC_FAR * __RPC_FAR *ppidisp)
{
   if (index < 0 || index >= m_visel.size())
      return TYPE_E_OUTOFBOUNDS;

   IDispatch * const pdisp = m_visel[index].GetDispatch();
   return pdisp->QueryInterface(IID_IDispatch, (void **)ppidisp);
}

STDMETHODIMP Collection::get__NewEnum(IUnknown** ppunk)
{
   CComObject<OMCollectionEnum> *pomenum;
   HRESULT hr = CComObject<OMCollectionEnum>::CreateInstance(&pomenum);

   if (SUCCEEDED(hr))
   {
      pomenum->Init(this);
      hr = pomenum->QueryInterface(IID_IEnumVARIANT, (void **)ppunk);
   }

   return hr;
}

STDMETHODIMP OMCollectionEnum::Init(Collection *pcol)
{
   m_pcol = pcol;
   m_index = 0;
   return S_OK;
}

STDMETHODIMP OMCollectionEnum::Next(ULONG celt, VARIANT __RPC_FAR *rgVar, ULONG __RPC_FAR *pCeltFetched)
{
   int last;
   HRESULT hr;
   const int cwanted = celt;
   int creturned;

   if (m_index + cwanted > m_pcol->m_visel.size())
   {
      hr = S_FALSE;
      last = m_pcol->m_visel.size();
      creturned = m_pcol->m_visel.size() - m_index;
   }
   else
   {
      hr = S_OK;
      last = m_index + cwanted;
      creturned = cwanted;
   }

   for (int i = m_index; i < last; ++i)
   {
      IDispatch * const pdisp = m_pcol->m_visel[i].GetDispatch();
      pdisp->QueryInterface(IID_IDispatch, (void **)&pdisp);

      V_VT(&rgVar[i - m_index]) = VT_DISPATCH;
      V_DISPATCH(&rgVar[i - m_index]) = pdisp;
   }

   m_index += creturned;

   if (pCeltFetched)
      *pCeltFetched = creturned;

   return hr;
}

STDMETHODIMP OMCollectionEnum::Skip(ULONG celt)
{
   m_index += celt;
   return (m_index >= m_pcol->m_visel.size()) ? S_FALSE : S_OK;
}

STDMETHODIMP OMCollectionEnum::Reset()
{
   m_index = 0;
   return S_OK;
}

STDMETHODIMP OMCollectionEnum::Clone(IEnumVARIANT __RPC_FAR *__RPC_FAR *ppEnum)
{
   IUnknown *punk;
   HRESULT hr = m_pcol->get__NewEnum(&punk);

   if (SUCCEEDED(hr))
   {
      hr = punk->QueryInterface(IID_IEnumVARIANT, (void **)ppEnum);

      punk->Release();
   }

   return hr;
}
