#ifndef H_PROPERTY_DIALOG
#define H_PROPERTY_DIALOG


class BaseProperty: public CDialog
{
public:    
    BaseProperty(int id, VectorProtected<ISelect> *pvsel) : CDialog(id), m_pvsel(pvsel)
    {
    }
    virtual void UpdateProperties(const int dispid) = 0;
    virtual void UpdateVisuals() = 0;
protected:
    VectorProtected<ISelect> *m_pvsel;

};

class TimerProperty: public BaseProperty
{
public:
    TimerProperty(VectorProtected<ISelect> *pvsel);
    virtual void UpdateProperties(const int dispid);
    virtual void UpdateVisuals();
protected:
    virtual BOOL OnInitDialog();
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
private:
    CEdit   m_timerIntervalEdit;
    CEdit   m_userValueEdit;
};

class PropertyDialog : public CDialog
{
public:
    PropertyDialog();
    void UpdateTabs(VectorProtected<ISelect> *pvsel);

    static void UpdateTextureComboBox(vector<Texture*> contentList, CComboBox &combo, const char *selectName);
    static void UpdateComboBox(vector<string> contentList, CComboBox &combo, const char *selectName);
    static void UpdateMaterialComboBox(vector<Material *> contentList, CComboBox &combo, const char *selectName);
    static void UpdateSurfaceComboBox(PinTable *ptable, CComboBox &combo, const char *selectName);
    static BOOL GetCheckboxState(HWND checkBoxHwnd)
    {
        size_t selected = ::SendMessage(checkBoxHwnd, BM_GETCHECK, 0, 0);
        return selected != 0;
    }
    static void SetCheckboxState(HWND checkBoxHwnd, BOOL checked)
    {
        ::SendMessage(checkBoxHwnd, BM_SETCHECK, checked ? BST_CHECKED : BST_UNCHECKED, 0);
    }
    
    static float GetFloatTextbox(CEdit &textbox)
    {
        const CString textStr(textbox.GetWindowText());
        const float fv = sz2f(textStr.c_str());
        return fv;
    }

    static int GetIntTextbox(CEdit &textbox)
    {
        const CString textStr(textbox.GetWindowText());
        int value = 0;
        sscanf_s(textStr.c_str(), "%i", &value);
        return value;
    }

    static void SetFloatTextbox(CEdit &textbox, const float value)
    {
        char strValue[256];
        f2sz(value, strValue);
        textbox.SetWindowText(strValue);
    }

    static void SetIntTextbox(CEdit &textbox, const int value)
    {
        textbox.SetWindowText(CString(value).c_str());
    }

    static void GetComboBoxText(CComboBox &combo, char *strbuf)
    {
        char buf[MAXTOKEN];
        combo.GetLBText(combo.GetCurSel(), buf);
        CString str(buf);
        strncpy_s(strbuf, MAXNAMEBUFFER, str.c_str(), (str.GetLength()> MAXNAMEBUFFER) ? MAXNAMEBUFFER-1:str.GetLength());
    }

    static int GetComboBoxIndex(CComboBox &combo, vector<string> contentList)
    {
        char buf[MAXTOKEN];
        combo.GetLBText(combo.GetCurSel(), buf);
        for (size_t i = 0; i < contentList.size(); i++)
            if (contentList[i].compare(buf)==0)
                return i;
        return -1;
    }

    BOOL IsSubDialogMessage(MSG &msg) const;

protected:
    virtual BOOL OnInitDialog();
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    virtual INT_PTR DialogProc(UINT msg, WPARAM wparam, LPARAM lparam);

private:
    CTab m_tab;
    BaseProperty *m_tabs[5];
    int  m_curTabIndex;
    CEdit m_nameEdit;
    CResizer m_resizer;
};

#endif

