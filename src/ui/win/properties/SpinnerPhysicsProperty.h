// license:GPLv3+

#pragma once

class SpinnerPhysicsProperty final : public BasePropertyDialog
{
public:
    SpinnerPhysicsProperty(const VectorProtected<ISelect> *pvsel);
    ~SpinnerPhysicsProperty() override { }

    void UpdateVisuals(const int dispid=-1) override;
    void UpdateProperties(const int dispid) override;

protected:
    BOOL OnInitDialog() override;
    INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
    EditBox m_dampingEdit;
    EditBox m_elasticityEdit;
};
