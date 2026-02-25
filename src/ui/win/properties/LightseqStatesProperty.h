// license:GPLv3+

#pragma once

class LightseqStatesProperty final : public BasePropertyDialog
{
public:
    LightseqStatesProperty(const VectorProtected<ISelect> *pvsel);
    ~LightseqStatesProperty() override { }

    void UpdateVisuals(const int dispid=-1) override;
    void UpdateProperties(const int dispid) override;

protected:
    BOOL OnInitDialog() override;
    INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
    EditBox     m_posXEdit;
    EditBox     m_posYEdit;
    EditBox     m_updateIntervalEdit;
    ComboBox    m_collectionCombo;
};
