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

void IEditable::Delete()
{
   MarkForDelete();

   GetPTable()->RemovePart(this);

   for (size_t i = 0; i < m_vCollection.size(); i++)
   {
      Collection *const pcollection = m_vCollection[i];
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
         partGroup->AddRef();
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

void IEditable::LoadSharedEditableField(const int tag, IObjectReader& reader)
{
   switch (tag)
   {
   case FID(LOCK): m_uiLocked = reader.AsBool(); break;
   case FID(LVIS): m_uiVisible = reader.AsBool(); break;
   case FID(LAYR): // Old layer style (limited number of unnamed layers)
   {
      int layerIndex = reader.AsInt();
      m_onLoadExpectedPartGroup = (layerIndex < 9 ? L"Layer_0" : L"Layer_") + std::to_wstring(layerIndex + 1);
      break;
   }
   case FID(LANR): // 10.7 layers (limited number of named layers)
   {
      string layerName = reader.AsString();
      std::ranges::transform(
         layerName.begin(), layerName.end(), layerName.begin(), [](char c) { return ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) ? c : '_'; });
      m_onLoadExpectedPartGroup = MakeWString(layerName);
      break;
   }
   case FID(GRUP): // 10.8.1 groups (unlimited number of hierarchical parenting with properties)
   {
      string layerName = reader.AsString();
      m_onLoadExpectedPartGroup = MakeWString(layerName);
      break;
   }
   default:
   {
      PLOGE << "Unhandled token: " << (char)(tag & 0xFF) << (char)((tag >> 8) & 0xFF) << (char)((tag >> 16) & 0xFF) << (char)((tag >> 24) & 0xFF);
   }
   }
}

void IEditable::SaveSharedEditableFields(IObjectWriter& writer)
{
   writer.WriteBool(FID(LOCK), m_uiLocked);
   writer.WriteBool(FID(LVIS), m_uiVisible);
   if (GetPartGroup())
   {
      // Implement backwards 'readability' (file will open in previous versions, with unsupported content dropped)
      const PartGroup* layer = GetPartGroup();
      while (layer->GetPartGroup() != nullptr)
         layer = layer->GetPartGroup();
      int index = 0;
      for (const auto edit : GetPTable()->GetParts())
      {
         if (edit == layer)
            break;
         if (edit->GetItemType() == eItemPartGroup && edit->GetPartGroup() == nullptr)
            index++;
      }
      writer.WriteInt(FID(LAYR), min(index, 11));
      writer.WriteString(FID(LANR), layer->GetName());
      writer.WriteString(FID(GRUP), GetPartGroup()->GetName());
   }
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

void IEditable::BeginUndo()
{
   if (GetPTable())
      GetPTable()->BeginUndo();
}

void IEditable::EndUndo()
{
   if (GetPTable())
      GetPTable()->EndUndo();
}

void IEditable::MarkForUndo()
{
   if (GetPTable())
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
      Collection *const pcollection = m_vCollection[i];
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

const wstring& IEditable::GetWName() const
{
   const IScriptable *const pscript = const_cast<IEditable*>(this)->GetScriptable();
   if (pscript)
      return pscript->get_Name();
   static const wstring emptyString;
   return emptyString;
}

void IEditable::SetName(const wstring& name)
{
   IScriptable *const scriptable = GetScriptable();
   if (name.empty() || scriptable == nullptr)
      return;

   wstring newName = name;
   if (newName.length() >= MAXNAMEBUFFER)
      newName.erase(MAXNAMEBUFFER - 1);

   if (newName == scriptable->m_wzName)
      return;

   if (PinTable* const pt = GetPTable(); pt)
   {
      if (!pt->IsNameUnique(newName))
         newName = pt->GetUniqueName(newName);

      STARTUNDO
      if (pt->HasPart(this))
         pt->RenamePart(this, newName);
      else
         scriptable->m_wzName = newName;
      STOPUNDO
   }
   else
   {
      scriptable->m_wzName = newName;
   }
}
