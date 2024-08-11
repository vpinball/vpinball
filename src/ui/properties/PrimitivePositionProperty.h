// license:GPLv3+

#pragma once

class PrimitivePositionProperty : public BasePropertyDialog
{
public:
    PrimitivePositionProperty(const VectorProtected<ISelect> *pvsel);
    virtual ~PrimitivePositionProperty() { }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

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
