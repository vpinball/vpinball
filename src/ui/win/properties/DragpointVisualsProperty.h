// license:GPLv3+

#pragma once

class DragpointVisualsProperty final : public BasePropertyDialog
{
public:
    DragpointVisualsProperty(int id, const VectorProtected<ISelect> *pvsel);
    ~DragpointVisualsProperty() override { }

    void UpdateVisuals(const int dispid=-1) override;
    void UpdateProperties(const int dispid) override;

protected:
    BOOL OnInitDialog() override;
    BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
    INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

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
