// license:GPLv3+

#pragma once

class TableAudioProperty: public BasePropertyDialog
{
public:
    TableAudioProperty(const VectorProtected<ISelect> *pvsel);
    virtual ~TableAudioProperty() { }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    EditBox     m_soundEffectVolEdit;
    EditBox     m_musicVolEdit;
};
