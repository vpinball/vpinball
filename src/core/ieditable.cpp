// license:GPLv3+

#include "core/stdafx.h"

IEditable::IEditable()
{
   VariantInit(&m_uservalue);
}

IEditable::~IEditable()
{
   assert(m_phittimer == nullptr); // If TimerRelease was not called, then player will hold an invalid reference
   SetPartGroup(nullptr);
}

void IEditable::SetDirtyDraw()
{
   GetPTable()->SetDirtyDraw();
}

void IEditable::Delete()
{
   MarkForDelete();

   GetPTable()->RemovePart(this);

   for (size_t i = 0; i < m_vCollection.size(); i++)
   {
      Collection * const pcollection = m_vCollection[i];
      pcollection->m_visel.find_erase(GetISelect());
   }
}

void IEditable::Uncreate()
{
   GetPTable()->RemovePart(this);
}

void IEditable::SetPartGroup(PartGroup* partGroup)
{
   if (m_partGroup != partGroup)
   {
      if (partGroup)
      {
         assert(GetPTable()->HasPart(partGroup));
         partGroup->AddRef();
      }
      if (m_partGroup)
         m_partGroup->Release();
      m_partGroup = partGroup;
   }
}

string IEditable::GetPathString(const bool isDirOnly) const
{
   const PartGroup* parent = GetPartGroup();
   if (parent == nullptr)
      return GetName();
   if (parent->GetPartGroup() == nullptr)
      return parent->GetName() + '/' + GetName();
   vector<const PartGroup *> itemPath;
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
      g_pplayer->TimerStateChange(m_phittimer.get(), !!val);

   *pte = val;

   STOPUNDO

   return S_OK;
}

HRESULT IEditable::put_TimerInterval(long newVal, int *pTimerInterval)
{
   STARTUNDO

   *pTimerInterval = newVal;

   if (m_phittimer)
      m_phittimer->SetInterval(newVal);

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
   for (size_t i = 0; i < m_vCollection.size(); i++)
   {
      Collection * const pcollection = m_vCollection[i];
      pcollection->m_visel.push_back(GetISelect());
   }
}

string IEditable::GetName() const
{
   const IScriptable *const pscript = const_cast<IEditable*>(this)->GetScriptable();
   if (pscript)
      return MakeString(pscript->get_Name());
   return string();
}

void IEditable::SetName(const string& name)
{
   IScriptable* scriptable = GetScriptable();
   if (name.empty() || scriptable == nullptr || GetItemType() == eItemDecal)
      return;

   PinTable* const pt = GetPTable();
   if (pt == nullptr)
      return;

   wstring newName = MakeWString(name);
   if (newName.length() >= std::size(scriptable->m_wzName))
      newName.erase(std::size(scriptable->m_wzName) - 1);

   if (newName == scriptable->m_wzName)
      return;
   
   if (!pt->IsNameUnique(newName))
   {
      WCHAR uniqueName[std::size(scriptable->m_wzName)];
      pt->GetUniqueName(newName, uniqueName, std::size(scriptable->m_wzName));
      newName = uniqueName;
   }

   STARTUNDO
   if (pt->HasPart(this))
      pt->RenamePart(this, newName);
   else
      wcsncpy_s(scriptable->m_wzName, std::size(scriptable->m_wzName), newName.c_str());
   STOPUNDO
}
