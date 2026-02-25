// license:GPLv3+

#pragma once

class PrimitivePositionProperty final : public BasePropertyDialog
{
public:
    PrimitivePositionProperty(const VectorProtected<ISelect> *pvsel);
    ~PrimitivePositionProperty() override { }

    void UpdateVisuals(const int dispid=-1) override;
    void UpdateProperties(const int dispid) override;

protected:
    BOOL OnInitDialog() override;
    INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
    EditBox   m_posXEdit;
    EditBox   m_posYEdit;
    EditBox   m_posZEdit;
    EditBox   m_scaleXEdit;
    EditBox   m_scaleYEdit;
    EditBox   m_scaleZEdit;
    EditBox   m_rotXEdit;
    EditBox   m_rotYEdit;
    EditBox   m_rotZEdit;
    EditBox   m_transXEdit;
    EditBox   m_transYEdit;
    EditBox   m_transZEdit;
    EditBox   m_objRotXEdit;
    EditBox   m_objRotYEdit;
    EditBox   m_objRotZEdit;
};
