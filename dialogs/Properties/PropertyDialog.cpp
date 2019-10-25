#include "stdafx.h"
#include "Properties/PropertyDialog.h"
#include "Properties/WallVisualsProperty.h"
#include <WindowsX.h>


BOOL NameDialog::OnInitDialog()
{
    AttachItem(30000, m_nameEdit);
    m_nameEdit.SetWindowText(m_iselect->GetPTable()->GetElementName(m_iselect->GetIEditable()));
    return TRUE;
}

BOOL NameDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{

    return FALSE;
}


PropertyDialog::PropertyDialog() : CDialog(IDD_PROPERTY_DIALOG)
{
    memset(m_tabs, 0, sizeof(m_tabs));
}

void PropertyDialog::UpdateTextureComboBox(vector<Texture *> contentList, CComboBox &combo, char *selectName)
{
    int selIdx = 0;

    combo.AddString(_T("<None>"));
    for (size_t i = 0; i < contentList.size(); i++)
    {
        combo.AddString(contentList[i]->m_szName);
        if (strncmp(contentList[i]->m_szName, selectName, MAXTOKEN) == 0)
        {
            selIdx = i + 1;
        }
    }
    combo.SetCurSel(selIdx);
}

void PropertyDialog::UpdateMaterialComboBox(vector<Material *> contentList, CComboBox &combo, char *selectName)
{
    int selIdx = 0;

    combo.AddString(_T("<None>"));
    for (size_t i = 0; i < contentList.size(); i++)
    {
        combo.AddString(contentList[i]->m_szName);
        if (strncmp(contentList[i]->m_szName, selectName, MAXTOKEN) == 0)
        {
            selIdx = i + 1;
        }
    }
    combo.SetCurSel(selIdx);
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

    m_tabs[0] = static_cast<CDialog*>(m_tab.AddTabPage(new NameDialog(psel), _T("Name")));

    switch (psel->GetItemType())
    {
        case eItemSurface:
        {
            m_tabs[1] = static_cast<CDialog *>(m_tab.AddTabPage(new WallVisualsProperty((Surface*)psel), _T("Visuals")));
            break;
        }
        default:
            break;
    }
}

BOOL PropertyDialog::OnInitDialog()
{
    AttachItem(IDC_PROP_TAB, m_tab);
    return TRUE;
}

BOOL PropertyDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
    return FALSE;
}

