// license:GPLv3+

#include "core/stdafx.h"

IEditable::IEditable()
{
   VariantInit(&m_uservalue);
}

IEditable::~IEditable()
{
   SetPartGroup(nullptr);
}

void IEditable::SetDirtyDraw()
{
   GetPTable()->SetDirtyDraw();
}

void IEditable::Delete()
{
   RemoveFromVectorSingle(GetPTable()->m_vedit, this);
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
   RemoveFromVectorSingle(GetPTable()->m_vedit, this);
   if (GetScriptable())
      GetPTable()->m_pcv->RemoveItem(GetScriptable());
}

void IEditable::SetPartGroup(PartGroup* partGroup)
{
   if (m_partGroup != partGroup)
   {
      if (partGroup)
      {
         assert(std::ranges::find(GetPTable()->m_vedit, partGroup) != GetPTable()->m_vedit.end());
         partGroup->AddRef();
      }
      if (m_partGroup)
         m_partGroup->Release();
      m_partGroup = partGroup;
   }
}

string IEditable::GetPathString(const bool isDirOnly) const
{
   vector<const PartGroup*> itemPath;
   const PartGroup* parent = GetPartGroup();
   while (parent != nullptr)
   {
      itemPath.insert(itemPath.begin(), parent);
      parent = parent->GetPartGroup();
   }
   std::stringstream ss;
   for (const auto& group : itemPath)
      ss << group->GetName() << '/';
   if (!isDirOnly)
      ss << GetName();
   return ss.str();
}

bool IEditable::IsChild(const PartGroup* group) const
{
   const PartGroup* parent = GetPartGroup();
   while ((parent != group) && (parent != nullptr))
      parent = parent->GetPartGroup();
   return parent == group;
}


HRESULT IEditable::put_TimerEnabled(VARIANT_BOOL newVal, BOOL *pte)
{
   STARTUNDO

   const BOOL val = VBTOF(newVal);

   if (val != *pte && m_phittimer)
      g_pplayer->DeferTimerStateChange(m_phittimer, !!val);

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
      m_phittimer->m_interval = newVal >= 0 ? max(newVal, (long)MAX_TIMER_MSEC_INTERVAL) : max(-2l, newVal);
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

const char *IEditable::GetName() const
{
    if (GetItemType() == eItemDecal)
        return "Decal";

    const IScriptable *const pscript = const_cast<IEditable*>(this)->GetScriptable();
    if (pscript)
    {
        static char elementName[256];
        WideCharToMultiByteNull(CP_ACP, 0, pscript->m_wzName, -1, elementName, sizeof(elementName), nullptr, nullptr);
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
    const PinTable* const pt = GetPTable();
    if (pt == nullptr)
        return;

    char oldName[sizeof(GetScriptable()->m_wzName)/sizeof(GetScriptable()->m_wzName[0])];
    WideCharToMultiByteNull(CP_ACP, 0, GetScriptable()->m_wzName, -1, oldName, sizeof(oldName), nullptr, nullptr);

    WCHAR newName[sizeof(oldName)];
    const WCHAR* namePtr = newName;
    MultiByteToWideCharNull(CP_ACP, 0, name.c_str(), -1, newName, sizeof(oldName));
    const bool isEqual = (wcscmp(newName, GetScriptable()->m_wzName) == 0);
    if(!isEqual && !pt->IsNameUnique(newName))
    {
       WCHAR uniqueName[sizeof(oldName)];
       pt->GetUniqueName(newName, uniqueName, std::size(uniqueName));
       namePtr = uniqueName;
    }
    STARTUNDO
    // first update name in the codeview before updating it in the element itself
    pt->m_pcv->ReplaceName(GetScriptable(), namePtr);
    lstrcpynW(GetScriptable()->m_wzName, namePtr, sizeof(oldName));
#ifndef __STANDALONE__
    g_pvp->SetPropSel(GetPTable()->m_vmultisel);
    g_pvp->GetLayersListDialog()->Update();

    if (GetItemType() == eItemSurface && g_pvp->MessageBox("Replace the name also in all table elements that use this surface?", "Replace", MB_ICONQUESTION | MB_YESNO) == IDYES)
    for (size_t i = 0; i < pt->m_vedit.size(); i++)
    {
       IEditable *const pedit = pt->m_vedit[i];
       if (pedit->GetItemType() == ItemTypeEnum::eItemBumper && ((Bumper *)pedit)->m_d.m_szSurface == oldName)
          ((Bumper *)pedit)->m_d.m_szSurface = name;
       else if (pedit->GetItemType() == ItemTypeEnum::eItemDecal && ((Decal *)pedit)->m_d.m_szSurface == oldName)
          ((Decal *)pedit)->m_d.m_szSurface = name;
       else if (pedit->GetItemType() == ItemTypeEnum::eItemFlipper && ((Flipper *)pedit)->m_d.m_szSurface == oldName)
          ((Flipper *)pedit)->m_d.m_szSurface = name;
       else if (pedit->GetItemType() == ItemTypeEnum::eItemGate && ((Gate *)pedit)->m_d.m_szSurface == oldName)
          ((Gate *)pedit)->m_d.m_szSurface = name;
       else if (pedit->GetItemType() == ItemTypeEnum::eItemKicker && ((Kicker *)pedit)->m_d.m_szSurface == oldName)
          ((Kicker *)pedit)->m_d.m_szSurface = name;
       else if (pedit->GetItemType() == ItemTypeEnum::eItemLight && ((Light *)pedit)->m_d.m_szSurface == oldName)
          ((Light *)pedit)->m_d.m_szSurface = name;
       else if (pedit->GetItemType() == ItemTypeEnum::eItemPlunger && ((Plunger *)pedit)->m_d.m_szSurface == oldName)
          ((Plunger *)pedit)->m_d.m_szSurface = name;
       else if (pedit->GetItemType() == ItemTypeEnum::eItemSpinner && ((Spinner *)pedit)->m_d.m_szSurface == oldName)
          ((Spinner *)pedit)->m_d.m_szSurface = name;
       else if (pedit->GetItemType() == ItemTypeEnum::eItemTrigger && ((Trigger *)pedit)->m_d.m_szSurface == oldName)
          ((Trigger *)pedit)->m_d.m_szSurface = name;
    }
#endif
    STOPUNDO
}
