#pragma once

class TableCustomProperty: public BasePropertyDialog
{
public:
    TableCustomProperty(const VectorProtected<ISelect> *pvsel);
    virtual ~TableCustomProperty() { }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    EditBox     m_soundEffectVolEdit;
    EditBox     m_musicVolEdit;

    ComboBox m_overwritePhysicsSetCombo;
    HWND m_hOverwriteFlipperCheck;
    vector<string> m_physicSetList;
};
