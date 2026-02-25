// license:GPLv3+

#pragma once

class BumperPhysicsProperty final : public BasePropertyDialog
{
public:
    BumperPhysicsProperty(const VectorProtected<ISelect> *pvsel);
    ~BumperPhysicsProperty() override{ }

    void UpdateVisuals(const int dispid=-1) override;
    void UpdateProperties(const int dispid) override;

protected:
    BOOL OnInitDialog() override;
    INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
    EditBox   m_forceEdit;
    EditBox   m_hitThresholdEdit;
    EditBox   m_scatterAngleEdit;
};
