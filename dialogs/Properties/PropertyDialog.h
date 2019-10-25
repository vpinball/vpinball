#ifndef H_PROPERTY_DIALOG
#define H_PROPERTY_DIALOG

class NameDialog: public CDialog
{
public:
    NameDialog(ISelect *psel) : CDialog(IDD_PROP_NAME), m_iselect(psel)
    {
    }

protected:
    virtual BOOL OnInitDialog();
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

private:
    CEdit m_nameEdit;
    ISelect *m_iselect;
};


class PropertyDialog : public CDialog
{
public:
    PropertyDialog();
    void UpdateTabs(VectorProtected<ISelect> *pvsel);

    static void UpdateTextureComboBox(vector<Texture*> contentList, CComboBox &combo, char *selectName);
    static void UpdateMaterialComboBox(vector<Material *> contentList, CComboBox &combo, char *selectName);
    static BOOL GetCheckboxState(HWND checkBoxHwnd)
    {
        size_t selected = ::SendMessage(checkBoxHwnd, BM_GETCHECK, 0, 0);
        return selected != 0;
    }
    static void SetCheckboxState(HWND checkBoxHwnd, BOOL checked)
    {
        ::SendMessage(checkBoxHwnd, BM_SETCHECK, checked ? BST_CHECKED : BST_UNCHECKED, 0);
    }

protected:
    virtual BOOL OnInitDialog();
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

private:
    CTab m_tab;
    CDialog *m_tabs[5];
};

#endif

