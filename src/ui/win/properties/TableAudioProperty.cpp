// license:GPLv3+

#include "core/stdafx.h"
#include "TableAudioProperty.h"

#include "parts/pintable.h"
#include "ui/win/resource.h"


TableAudioProperty::TableAudioProperty(const VectorProtected<ISelect> *pvsel)
   : BasePropertyDialog(IDD_PROPTABLE_AUDIO, pvsel)
{
   assert(pvsel->size() == 1);
   assert(pvsel->ElementAt(0)->GetItemType() == eItemTable);
   m_soundEffectVolEdit.SetDialog(this);
   m_musicVolEdit.SetDialog(this);
}

void TableAudioProperty::UpdateVisuals(const int dispid /*=-1*/)
{
   PinTable *const table = (PinTable *)m_pvsel->ElementAt(0);
   if (dispid == IDC_TABLESOUNDVOLUME || dispid == -1)
      PropertyDialog::SetIntTextbox(m_soundEffectVolEdit, table->GetTableSoundVolume());
   if (dispid == IDC_TABLEMUSICVOLUME || dispid == -1)
      PropertyDialog::SetIntTextbox(m_musicVolEdit, table->GetTableMusicVolume());
}

void TableAudioProperty::UpdateProperties(const int dispid)
{
   PinTable *const table = (PinTable *)m_pvsel->ElementAt(0);
   switch (dispid)
   {
   case IDC_TABLESOUNDVOLUME: CHECK_UPDATE_VALUE_SETTER(table->SetTableSoundVolume, table->GetTableSoundVolume, PropertyDialog::GetIntTextbox, m_soundEffectVolEdit, table); break;
   case IDC_TABLEMUSICVOLUME: CHECK_UPDATE_VALUE_SETTER(table->SetTableMusicVolume, table->GetTableMusicVolume, PropertyDialog::GetIntTextbox, m_musicVolEdit, table); break;
   default: break;
   }

   UpdateVisuals(dispid);
}

BOOL TableAudioProperty::OnInitDialog()
{
   m_soundEffectVolEdit.AttachItem(IDC_TABLESOUNDVOLUME);
   m_musicVolEdit.AttachItem(IDC_TABLEMUSICVOLUME);

   UpdateVisuals();

   m_resizer.Initialize(GetHwnd(), CRect(0, 0, 0, 0));
   m_resizer.AddChild(GetDlgItem(IDC_STATIC14), CResizer::topleft, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC15), CResizer::topleft, 0);
   m_resizer.AddChild(m_soundEffectVolEdit, CResizer::topleft, RD_STRETCH_WIDTH);
   m_resizer.AddChild(m_musicVolEdit, CResizer::topleft, RD_STRETCH_WIDTH);

   return TRUE;
}

INT_PTR TableAudioProperty::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   m_resizer.HandleMessage(uMsg, wParam, lParam);
   return DialogProcDefault(uMsg, wParam, lParam);
}
