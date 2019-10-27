#include "stdafx.h"
#include "Properties/PropertyDialog.h"
#include "Properties/WallVisualsProperty.h"
#include "Properties/WallPhysicsProperty.h"
#include <WindowsX.h>


PropertyDialog::PropertyDialog() : CDialog(IDD_PROPERTY_DIALOG)
{
    memset(m_tabs, 0, sizeof(m_tabs));
}

void PropertyDialog::UpdateTextureComboBox(vector<Texture *> contentList, CComboBox &combo, char *selectName)
{
    combo.ResetContent();
    combo.AddString(_T("<None>"));
    for (size_t i = 0; i < contentList.size(); i++)
    {
        combo.AddString(contentList[i]->m_szName);
    }
    combo.SetCurSel(combo.FindStringExact(1,selectName));
}

void PropertyDialog::UpdateMaterialComboBox(vector<Material *> contentList, CComboBox &combo, char *selectName)
{
    combo.ResetContent();
    combo.AddString(_T("<None>"));
    for (size_t i = 0; i < contentList.size(); i++)
    {
        combo.AddString(contentList[i]->m_szName);
    }
    combo.SetCurSel(combo.FindStringExact(1, selectName));
}

void PropertyDialog::UpdateTabs(VectorProtected<ISelect> *pvsel)
{
    ISelect *psel = pvsel->ElementAt(0);

    if (psel == NULL)
        return;

    for (int i = 0; i < 5; i++)
        if (m_tabs[i] != NULL)
        {
            m_tab.RemoveTabPage(m_tab.GetTabIndex(m_tabs[i]));
            m_tabs[i] = NULL;
        }

    m_nameEdit.SetWindowText(psel->GetPTable()->GetElementName(psel->GetIEditable()));

    switch (psel->GetItemType())
    {
        case eItemSurface:
        {
            m_tabs[0] = static_cast<CDialog *>(m_tab.AddTabPage(new WallVisualsProperty((Surface*)psel), _T("Visuals")));
            m_tabs[1] = static_cast<CDialog *>(m_tab.AddTabPage(new WallPhysicsProperty((Surface*)psel), _T("Physics")));
            break;
        }
        default:
            break;
    }
}

BOOL PropertyDialog::OnInitDialog()
{
    AttachItem(IDC_PROP_TAB, m_tab);
    AttachItem(IDC_NAME_EDIT, m_nameEdit);
    m_resizer.Initialize(*this, CRect(0, 0, 243, 308));
    m_resizer.AddChild(m_nameEdit, topleft, RD_STRETCH_WIDTH);
    m_resizer.AddChild(m_tab, topcenter, RD_STRETCH_HEIGHT | RD_STRETCH_WIDTH);
    return TRUE;
}

INT_PTR PropertyDialog::DialogProc(UINT msg, WPARAM wparam, LPARAM lparam)
{
    // Pass resizing messages on to the resizer
    m_resizer.HandleMessage(msg, wparam, lparam);

//  switch (msg)
//  {
        //Additional messages to be handled go here
//  }

    // Pass unhandled messages on to parent DialogProc
    return DialogProcDefault(msg, wparam, lparam);
}

BOOL PropertyDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
    return FALSE;
}

