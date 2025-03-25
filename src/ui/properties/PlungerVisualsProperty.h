// license:GPLv3+

#pragma once

class PlungerVisualsProperty final : public BasePropertyDialog
{
public:
    PlungerVisualsProperty(const VectorProtected<ISelect> *pvsel);
    ~PlungerVisualsProperty() override { }

    void UpdateVisuals(const int dispid=-1) override;
    void UpdateProperties(const int dispid) override;

protected:
    BOOL OnInitDialog() override;
    INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
    ComboBox  m_typeCombo;
    ComboBox  m_imageCombo;
    ComboBox  m_materialCombo;
    ComboBox  m_surfaceCombo;
    EditBox   m_flatFramesEdit;
    EditBox   m_widthEdit;
    EditBox   m_zAdjustmentEdit;
    EditBox   m_rodDiameterEdit;
    EditBox   m_tipShapeEdit;
    EditBox   m_ringGapEdit;
    EditBox   m_ringDiamEdit;
    EditBox   m_ringWidthEdit;
    EditBox   m_springDiamEdit;
    EditBox   m_springGaugeEdit;
    EditBox   m_springLoopsEdit;
    EditBox   m_endLoopsEdit;
    EditBox   m_posXEdit;
    EditBox   m_posYEdit;
    vector<string> m_typeList;
};
