#include "stdafx.h"


IEditable::IEditable()
{
   m_phittimer = nullptr;

   m_backglass = false;
   VariantInit(&m_uservalue);
   m_singleEvents = true;
}

IEditable::~IEditable()
{
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
   RemoveFromVectorSingle(GetPTable()->m_vedit, (IEditable *)this);
   MarkForDelete();

   if (GetScriptable())
      GetPTable()->m_pcv->RemoveItem(GetScriptable());

   for (size_t i = 0; i < m_vCollection.size(); i++)
   {
      Collection * const pcollection = m_vCollection[i];
      pcollection->m_visel.find_erase(GetISelect());
   }
}

void IEditable::Uncreate()
{
   RemoveFromVectorSingle(GetPTable()->m_vedit, (IEditable *)this);
   if (GetScriptable())
      GetPTable()->m_pcv->RemoveItem(GetScriptable());
}

HRESULT IEditable::put_TimerEnabled(VARIANT_BOOL newVal, BOOL *pte)
{
   STARTUNDO

   const BOOL val = VBTOF(newVal);

   if (val != *pte && m_phittimer)
   {
       // to avoid problems with timers dis/enabling themselves, store all the changes in a list
       bool found = false;
       for (size_t i = 0; i < g_pplayer->m_changed_vht.size(); ++i)
           if (g_pplayer->m_changed_vht[i].m_timer == m_phittimer)
           {
               g_pplayer->m_changed_vht[i].m_enabled = !!val;
               found = true;
               break;
           }

       if (!found)
       {
         TimerOnOff too;
         too.m_enabled = !!val;
         too.m_timer = m_phittimer;
         g_pplayer->m_changed_vht.push_back(too);
       }

       if (val)
           m_phittimer->m_nextfire = g_pplayer->m_time_msec + m_phittimer->m_interval;
       else
           m_phittimer->m_nextfire = 0xFFFFFFFF; // fakes the disabling of the timer, until it will be catched by the cleanup via m_changed_vht
   }

   *pte = val;

   STOPUNDO

   return S_OK;
}

HRESULT IEditable::put_TimerInterval(long newVal, int *pti)
{
   STARTUNDO

   *pti = newVal;

   if (m_phittimer)
   {
      m_phittimer->m_interval = newVal >= 0 ? max(newVal, (long)MAX_TIMER_MSEC_INTERVAL) : -1;
      m_phittimer->m_nextfire = g_pplayer->m_time_msec + m_phittimer->m_interval;
   }

   STOPUNDO

   return S_OK;
}

HRESULT IEditable::get_UserValue(VARIANT *pVal)
{
   VariantClear(pVal);
   return VariantCopy(pVal, &m_uservalue);
}

HRESULT IEditable::put_UserValue(VARIANT *newVal)
{
   STARTUNDO

   VariantInit(&m_uservalue);
   VariantClear(&m_uservalue);
   const HRESULT hr = VariantCopy(&m_uservalue, newVal);

   STOPUNDO

   return hr;
}

void IEditable::BeginPlay()
{
   m_vEventCollection.clear();
   m_viEventCollection.clear();

   m_singleEvents = true;
   for (size_t i = 0; i < m_vCollection.size(); i++)
   {
      Collection * const pcol = m_vCollection[i];
      if (pcol->m_fireEvents)
      {
         m_vEventCollection.push_back(pcol);
         m_viEventCollection.push_back(m_viCollection[i]);
      }
      if (pcol->m_stopSingleEvents)
         m_singleEvents = false;
   }
}

void IEditable::EndPlay()
{
   if (m_phittimer)
   {
      delete m_phittimer;
      m_phittimer = nullptr;
   }
}

void IEditable::RenderBlueprint(Sur *psur, const bool solid)
{
   UIRenderPass2(psur);
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

   for (size_t i = 0; i < m_vCollection.size(); i++)
   {
      Collection * const pcollection = m_vCollection[i];
      pcollection->m_visel.push_back(GetISelect());
   }
}

const char *IEditable::GetName()
{
    WCHAR *elemName = nullptr;
    if (GetItemType() == eItemDecal)
        return "Decal";

    IScriptable *const pscript = GetScriptable();
    if (pscript)
        elemName = pscript->m_wzName;

    if (elemName)
    {
        static char elementName[256];
        WideCharToMultiByteNull(CP_ACP, 0, elemName, -1, elementName, 256, nullptr, nullptr);
        return elementName;
    }
    return nullptr;
}

void IEditable::SetName(const string& name)
{
    if (name.empty())
        return;
    if (GetItemType() == eItemDecal)
        return;
    PinTable* const pt = GetPTable();
    if (pt == nullptr)
        return;

    WCHAR newName[sizeof(GetScriptable()->m_wzName)/sizeof(GetScriptable()->m_wzName[0])];
    const WCHAR* namePtr = newName;
    MultiByteToWideCharNull(CP_ACP, 0, name.c_str(), -1, newName, sizeof(GetScriptable()->m_wzName)/sizeof(GetScriptable()->m_wzName[0]));
    const bool isEqual = (wcscmp(newName, GetScriptable()->m_wzName) == 0);
    WCHAR uniqueName[sizeof(GetScriptable()->m_wzName)/sizeof(GetScriptable()->m_wzName[0])];
    if(!isEqual && !pt->IsNameUnique(newName))
    {
       pt->GetUniqueName(newName, uniqueName, sizeof(uniqueName) / sizeof(uniqueName[0]));
       namePtr = uniqueName;
    }
    STARTUNDO
    // first update name in the codeview before updating it in the element itself
    pt->m_pcv->ReplaceName(GetScriptable(), namePtr);
    lstrcpynW(GetScriptable()->m_wzName, namePtr, sizeof(GetScriptable()->m_wzName)/sizeof(GetScriptable()->m_wzName[0]));
#ifndef __STANDALONE__
    g_pvp->GetLayersListDialog()->UpdateElement(this);
    g_pvp->SetPropSel(GetPTable()->m_vmultisel);
#endif
    STOPUNDO
}

void IEditable::InitScript()
{
   if (!GetScriptable())
      return;

   if (GetScriptable()->m_wzName[0] == '\0')
      // Just in case something screws up - not good having a null script name
      swprintf_s(GetScriptable()->m_wzName, sizeof(GetScriptable()->m_wzName), L"%d", (long)this);

   GetPTable()->m_pcv->AddItem(GetScriptable(), false);
}
