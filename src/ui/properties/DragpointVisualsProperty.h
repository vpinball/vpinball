// license:GPLv3+

#pragma once

class DragpointVisualsProperty: public BasePropertyDialog
{
public:
    DragpointVisualsProperty(int id, const VectorProtected<ISelect> *pvsel);
    virtual ~DragpointVisualsProperty() { }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    EditBox m_posXEdit;
    EditBox m_posYEdit;
    EditBox m_heightOffsetEdit;
    EditBox m_realHeightEdit;
    EditBox m_textureCoordEdit;
    CButton m_copyButton;
    CButton m_pasteButton;
    int     m_id;
};
