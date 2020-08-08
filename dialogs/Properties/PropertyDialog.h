#ifndef H_PROPERTY_DIALOG
#define H_PROPERTY_DIALOG

#pragma region BasePropertyDialog

class EditBox;
class ComboBox;
class BasePropertyDialog: public CDialog
{
public:
    BasePropertyDialog(const int id, const VectorProtected<ISelect> *pvsel) : CDialog(id), m_pvsel(pvsel)
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
    virtual void UpdateProperties(const int dispid) = 0;
    virtual void UpdateVisuals(const int dispid=-1) = 0;
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam)
    {
        UNREFERENCED_PARAMETER(lParam);
        const int dispID = LOWORD(wParam);

        if(!m_disableEvents)
        {
            switch (HIWORD(wParam))
            {
            //case CBN_KILLFOCUS:
            case EN_KILLFOCUS:
            {
               if(m_pvsel->Size()>1)
                  break;   //early out here for multi-selected elements, otherwise the value of the last element in the list is assigned to all other elements as well
            }
            case CBN_SELCHANGE:
            case BN_CLICKED:
            {
                UpdateProperties(dispID);
                return TRUE;
            }
            }
        }
        return FALSE;
    }
    void UpdateBaseProperties(ISelect *psel, BaseProperty *property, const int dispid);
    void UpdateBaseVisuals(ISelect *psel, BaseProperty *property, const int dispid = -1);

    const VectorProtected<ISelect>* m_pvsel;
    static bool               m_disableEvents;
protected:
    virtual INT_PTR DialogProc(UINT msg, WPARAM wparam, LPARAM lparam);

    EditBox   *m_baseHitThresholdEdit;
    EditBox   *m_baseElasticityEdit;
    EditBox   *m_baseFrictionEdit;
    EditBox   *m_baseScatterAngleEdit;
    ComboBox  *m_basePhysicsMaterialCombo;
    ComboBox  *m_baseMaterialCombo;
    ComboBox  *m_baseImageCombo;
    HWND      m_hHitEventCheck;
    HWND      m_hCollidableCheck;
    HWND      m_hOverwritePhysicsCheck;
    HWND      m_hReflectionEnabledCheck;
    HWND      m_hVisibleCheck;
};

class EditBox : public CEdit
{
public:
    EditBox() : m_basePropertyDialog(nullptr), m_id(-1) {}
    virtual ~EditBox() {}
    void    SetDialog(BasePropertyDialog* dialog) { m_basePropertyDialog = dialog; }
    virtual void AttachItem(int id)
    {
        m_id = id;
        m_basePropertyDialog->AttachItem(id, *this);
    }
protected:
    virtual LRESULT WndProc(UINT msg, WPARAM wparam, LPARAM lparam);

private:
    BasePropertyDialog* m_basePropertyDialog;
    int m_id;
};

class ComboBox : public CComboBox
{
public:
    ComboBox() : m_basePropertyDialog(nullptr), m_id(-1) {}
    virtual ~ComboBox() {}
    void    SetDialog(BasePropertyDialog* dialog) { m_basePropertyDialog = dialog; }
    virtual void AttachItem(int id)
    {
        m_id = id;
        m_basePropertyDialog->AttachItem(id, *this);
    }
protected:
    virtual LRESULT WndProc(UINT msg, WPARAM wparam, LPARAM lparam);

private:
    BasePropertyDialog* m_basePropertyDialog;
    int m_id;
};
#pragma endregion

#pragma region TimerProperty

class TimerProperty: public BasePropertyDialog
{
public:
    TimerProperty(const VectorProtected<ISelect> *pvsel);
    virtual void UpdateProperties(const int dispid);
    virtual void UpdateVisuals(const int dispid=-1);
protected:
    virtual BOOL OnInitDialog();
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
private:
    EditBox m_timerIntervalEdit;
    EditBox m_userValueEdit;
};
#pragma endregion

#pragma region ColorButton

class ColorButton: public CButton
{
public:
    ColorButton() : m_color(0)
    {
    }
    ~ColorButton(){}
    
    void SetColor(const COLORREF color)
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
        dc.DrawEdge(rect, (state & ODS_SELECTED) ? EDGE_SUNKEN : EDGE_RAISED, BF_RECT); // if pressed draw sunken, otherwise raised face
        // draw the focus rectangle, a dotted rectangle just inside the
        // button rectangle when the button has the focus.
        if (lpDrawItemStruct->itemAction & ODA_FOCUS)
        {
            const int iChange = 3;
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
        vertex[0].Red   = ((unsigned int)r << 8) + r;
        vertex[0].Green = ((unsigned int)g << 8) + g;
        vertex[0].Blue  = ((unsigned int)b << 8) + b;
        // do some shading
        r = (unsigned char)((float)r * .9f);
        g = (unsigned char)((float)g * .9f);
        b = (unsigned char)((float)b * .9f);
        vertex[1].x = rect.BottomRight().x;
        vertex[1].y = rect.BottomRight().y;
        vertex[1].Red   = ((unsigned int)r << 8) + r;
        vertex[1].Green = ((unsigned int)g << 8) + g;
        vertex[1].Blue  = ((unsigned int)b << 8) + b;

        GRADIENT_RECT gRect;
        gRect.UpperLeft = 0;
        gRect.LowerRight = 1;

        GradientFill(lpDrawItemStruct->hDC, vertex, 2, &gRect, 1, GRADIENT_FILL_RECT_H);

        dc.Detach();
    }
private:
    COLORREF m_color;
};
#pragma endregion

#pragma region PropertyDialog

class PropertyTab : public CTab
{
public:
   PropertyTab() : m_activePage(0), m_activeTabText("")
   {}

   virtual LRESULT OnTCNSelChange(LPNMHDR pNMHDR)
   {
      m_activePage = GetCurSel();
      m_activeTabText = GetTabText(m_activePage);
      return CTab::OnTCNSelChange(pNMHDR);
   }

   int m_activePage;
   CString m_activeTabText;
};

#define PROPERTY_TABS 5

class PropertyDialog : public CDialog
{
public:
    PropertyDialog();

    void CreateTabs(VectorProtected<ISelect> &pvsel);
    void DeleteAllTabs();
    void UpdateTabs(VectorProtected<ISelect> &pvsel);
    bool PreTranslateMessage(MSG* msg);

    static void UpdateTextureComboBox(const vector<Texture*>& contentList, CComboBox &combo, const string &selectName);
    static void UpdateComboBox(const vector<string>& contentList, CComboBox &combo, const string &selectName);
    static void UpdateMaterialComboBox(const vector<Material *>& contentList, CComboBox &combo, const string &selectName);
    static void UpdateSurfaceComboBox(const PinTable *const ptable, CComboBox &combo, const char *selectName);
    static void UpdateSoundComboBox(const PinTable *const ptable, CComboBox &combo, const string &selectName);
    static void UpdateCollectionComboBox(const PinTable *const ptable, CComboBox &combo, const char *selectName);

    static void StartUndo(ISelect *const psel)
    {
        psel->GetIEditable()->BeginUndo();
        psel->GetIEditable()->MarkForUndo();
    }

    static void EndUndo(ISelect *const psel)
    {
        psel->GetIEditable()->EndUndo();
        psel->GetIEditable()->SetDirtyDraw();
    }

    static bool GetCheckboxState(const HWND checkBoxHwnd)
    {
        const size_t selected = ::SendMessage(checkBoxHwnd, BM_GETCHECK, 0, 0);
        return selected != 0;
    }

    static void SetCheckboxState(const HWND checkBoxHwnd, const bool checked)
    {
        ::SendMessage(checkBoxHwnd, BM_SETCHECK, checked ? BST_CHECKED : BST_UNCHECKED, 0);
    }
    
    static float GetFloatTextbox(CEdit &textbox)
    {
        const float fv = sz2f(string(textbox.GetWindowText()));
        return fv;
    }

    static int GetIntTextbox(CEdit &textbox)
    {
        int value = 0;
        sscanf_s(textbox.GetWindowText().c_str(), "%i", &value);
        return value;
    }

    static void SetFloatTextbox(CEdit &textbox, const float value)
    {
        string strValue;
        f2sz(value, strValue);
        textbox.SetWindowText(strValue.c_str());
    }

    static void SetIntTextbox(CEdit &textbox, const int value)
    {
        textbox.SetWindowText(std::to_string(value).c_str());
    }

    static void GetComboBoxText(CComboBox &combo, char * const strbuf, const size_t maxlength)
    {
        char buf[MAXSTRING];
        combo.GetLBText(combo.GetCurSel(), buf);
        strncpy_s(strbuf, maxlength, buf, maxlength-1);
    }

    static int GetComboBoxIndex(CComboBox &combo, const vector<string>& contentList)
    {
        char buf[MAXSTRING];
        combo.GetLBText(combo.GetCurSel(), buf);
        const string s(buf);
        for (size_t i = 0; i < contentList.size(); i++)
            if (contentList[i] == s)
                return (int)i;
        return -1;
    }

    BOOL IsSubDialogMessage(MSG &msg) const;
    virtual LRESULT OnMouseActivate(UINT msg, WPARAM wparam, LPARAM lparam);

protected:
    virtual BOOL OnInitDialog();
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    virtual INT_PTR DialogProc(UINT msg, WPARAM wparam, LPARAM lparam);
    virtual void OnClose();

private:
    PropertyTab  m_tab;
    BasePropertyDialog *m_tabs[PROPERTY_TABS];
    ItemTypeEnum m_previousType;
    bool         m_backglassView;

    int      m_curTabIndex;
    CEdit    m_nameEdit;
    CResizer m_resizer;
    CStatic  m_multipleElementsStatic;
    CStatic  m_elementTypeName;
};
#pragma endregion

#pragma region UpdateMacros

#define CHECK_UPDATE_ITEM(classValue, uiValue, element)\
{\
    auto value = uiValue; \
    if(classValue!=value) \
    { \
        PropertyDialog::StartUndo(element); \
        classValue=value; \
        PropertyDialog::EndUndo(element); \
    }\
}
#define CHECK_UPDATE_COMBO_TEXT_STRING(classValue, uiCombo, element)\
{\
    char szName[MAXSTRING]={0}; \
    PropertyDialog::GetComboBoxText(uiCombo, szName, sizeof(szName)); \
    if(szName!=classValue) \
    { \
        PropertyDialog::StartUndo(element); \
        classValue = szName; \
        PropertyDialog::EndUndo(element); \
    }\
}

#define CHECK_UPDATE_COMBO_TEXT(classValue, uiCombo, element)\
{\
    char szName[sizeof(classValue)]={0}; \
    PropertyDialog::GetComboBoxText(uiCombo, szName, sizeof(classValue)); \
    if (strcmp(szName, classValue) != 0) \
    { \
        PropertyDialog::StartUndo(element); \
        strncpy_s(classValue, szName, sizeof(classValue)-1); \
        PropertyDialog::EndUndo(element); \
    }\
}

#define CHECK_UPDATE_VALUE_SETTER(classSetter, classGetter, uiGetter, uiGetterParameter, element) \
{ \
    auto value = uiGetter(uiGetterParameter); \
    if (classGetter() != value) \
    { \
        PropertyDialog::StartUndo(element); \
        classSetter(value); \
        PropertyDialog::EndUndo(element); \
    } \
}

#define CHECK_UPDATE_COMBO_VALUE_SETTER(classSetter, classGetter, uiComboValue, element) \
{\
   auto value =  uiComboValue; \
   if(classGetter() != value) \
   { \
      PropertyDialog::StartUndo(element); \
      classSetter(value); \
      PropertyDialog::EndUndo(element); \
   } \
}

#pragma endregion

#pragma region Docking

class CContainProperties: public CDockContainer
{
public:
    CContainProperties();
    ~CContainProperties()
    {
    }
    PropertyDialog *GetPropertyDialog()
    {
        return &m_propertyDialog;
    }

private:
    PropertyDialog m_propertyDialog;
};

class CDockProperty: public CDocker
{
public:
    CDockProperty();

    virtual ~CDockProperty()
    {
    }
    virtual void OnClose();

    CContainProperties *GetContainProperties()
    {
        return &m_propContainer;
    }

private:
    CContainProperties m_propContainer;
};
#pragma endregion

#endif
