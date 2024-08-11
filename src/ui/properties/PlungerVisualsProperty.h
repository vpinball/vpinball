// license:GPLv3+

#pragma once

class PlungerVisualsProperty: public BasePropertyDialog
{
public:
    PlungerVisualsProperty(const VectorProtected<ISelect> *pvsel);
    virtual ~PlungerVisualsProperty() { }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

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
