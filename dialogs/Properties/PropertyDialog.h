#ifndef H_PROPERTY_DIALOG
#define H_PROPERTY_DIALOG


class BasePropertyDialog: public CDialog
{
public:
    BasePropertyDialog(int id, VectorProtected<ISelect> *pvsel) : CDialog(id), m_pvsel(pvsel)
    {
        m_baseHitThresholdEdit = NULL;
        m_baseElasticityEdit = NULL;
        m_baseFrictionEdit = NULL;
        m_baseScatterAngleEdit = NULL;
        m_basePhysicsMaterialCombo = NULL;
        m_baseMaterialCombo = NULL;
        m_baseImageCombo = NULL;
        m_hCollidableCheck = 0;
        m_hHitEventCheck = 0;
        m_hOverwritePhysicsCheck = 0;
        m_hReflectionEnabledCheck = 0;
        m_hVisibleCheck = 0;
    }
    virtual void UpdateProperties(const int dispid)=0;
    virtual void UpdateVisuals() =0;
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam)
    {
        UNREFERENCED_PARAMETER(lParam);
        const int dispID = LOWORD(wParam);

        switch (HIWORD(wParam))
        {
            case EN_KILLFOCUS:
            case CBN_KILLFOCUS:
            case BN_CLICKED:
            {
                UpdateProperties(dispID);
                return TRUE;
            }
        }
        return FALSE;
    }
    void UpdateBaseProperties(ISelect *psel, BaseProperty *property, const int dispid);
    void UpdateBaseVisuals(ISelect *psel, BaseProperty *property);

protected:
    VectorProtected<ISelect> *m_pvsel;
    CEdit     *m_baseHitThresholdEdit;
    CEdit     *m_baseElasticityEdit;
    CEdit     *m_baseFrictionEdit;
    CEdit     *m_baseScatterAngleEdit;
    CComboBox *m_basePhysicsMaterialCombo;
    CComboBox *m_baseMaterialCombo;
    CComboBox *m_baseImageCombo;
    HWND      m_hHitEventCheck;
    HWND      m_hCollidableCheck;
    HWND      m_hOverwritePhysicsCheck;
    HWND      m_hReflectionEnabledCheck;
    HWND      m_hVisibleCheck;
};

class TimerProperty: public BasePropertyDialog
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

class ColorButton: public CButton
{
public:
    ColorButton() : m_color(0)
    {
    }
    ~ColorButton(){}
    
    void SetColor(COLORREF color)
    {
        m_color = color;
        InvalidateRect(FALSE);
        //UpdateWindow();
    }

    virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
    {
              // get the device context and attach the button handle to it
        TRIVERTEX vertex[2];

        CDC dc;
        dc.Attach(lpDrawItemStruct->hDC);
          // determine the button rectangle
        CRect rect = lpDrawItemStruct->rcItem;
        
        // draw in the button text
        dc.DrawText(GetWindowText(), -1, rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        // get the current state of the button
        UINT state = lpDrawItemStruct->itemState;
        if ((state & ODS_SELECTED))   // if it is pressed
        {
            dc.DrawEdge(rect, EDGE_SUNKEN, BF_RECT); // draw a sunken face
        }
        else
        {
            dc.DrawEdge(rect, EDGE_RAISED, BF_RECT); // draw a raised face
        }
          // draw the focus rectangle, a dotted rectangle just inside the
          // button rectangle when the button has the focus.
        if (lpDrawItemStruct->itemAction & ODA_FOCUS)
        {
            int iChange = 3;
            rect.top += iChange;
            rect.left += iChange;
            rect.right -= iChange;
            rect.bottom -= iChange;
            dc.DrawFocusRect(rect);
        }
        unsigned char r, g, b;

        r = GetRValue(m_color);
        g = GetGValue(m_color);
        b = GetBValue(m_color);
        vertex[0].x = rect.TopLeft().x;
        vertex[0].y = rect.TopLeft().y;
        vertex[0].Red = (r << 8) + r;
        vertex[0].Green = (g << 8) + g;
        vertex[0].Blue = (b << 8) + b;
        // do some shading
        r = (unsigned char)(r * .9f);
        g = (unsigned char)(g * .9f);
        b = (unsigned char)(b * .9f);
        vertex[1].x = rect.BottomRight().x;
        vertex[1].y = rect.BottomRight().y;
        vertex[1].Red = (r << 8) + r;
        vertex[1].Green = (g << 8) + g;
        vertex[1].Blue = (b << 8) + b;

        GRADIENT_RECT gRect;
        gRect.UpperLeft = 0;
        gRect.LowerRight = 1;

        GradientFill(lpDrawItemStruct->hDC, vertex, 2, &gRect, 1, GRADIENT_FILL_RECT_H);

        dc.Detach();
    }
private:
    COLORREF m_color;
};

class PropertyDialog : public CDialog
{
public:
    PropertyDialog();
    void UpdateTabs(VectorProtected<ISelect> *pvsel);

    static void UpdateTextureComboBox(const vector<Texture*>& contentList, CComboBox &combo, const char *selectName);
    static void UpdateComboBox(const vector<string>& contentList, CComboBox &combo, const char *selectName);
    static void UpdateMaterialComboBox(const vector<Material *>& contentList, CComboBox &combo, const char *selectName);
    static void UpdateSurfaceComboBox(const PinTable * const ptable, CComboBox &combo, const char *selectName);

    static void StartUndo(ISelect *psel)
    {
        psel->GetIEditable()->BeginUndo();
        psel->GetIEditable()->MarkForUndo();
    }

    static void EndUndo(ISelect *psel)
    {
        psel->GetIEditable()->EndUndo();
        psel->GetIEditable()->SetDirtyDraw();
    }

    static bool GetCheckboxState(HWND checkBoxHwnd)
    {
        const size_t selected = ::SendMessage(checkBoxHwnd, BM_GETCHECK, 0, 0);
        return selected != 0;
    }
    static void SetCheckboxState(HWND checkBoxHwnd, bool checked)
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
        const CString str(buf);
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
    BasePropertyDialog *m_tabs[5];
    int  m_curTabIndex;
    CEdit m_nameEdit;
    CResizer m_resizer;
};

#endif
