// license:GPLv3+

#pragma once

class WallPhysicsProperty final : public BasePropertyDialog
{
public:
    WallPhysicsProperty(const VectorProtected<ISelect> *pvsel);
    ~WallPhysicsProperty() override { }

    void UpdateVisuals(const int dispid=-1) override;
    void UpdateProperties(const int dispid) override;

protected:
    BOOL OnInitDialog() override;
    INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
    EditBox   m_hitThresholdEdit;
    EditBox   m_slingshotForceEdit;
    EditBox   m_slingshotThresholdEdit;
    EditBox   m_elasticityEdit;
    EditBox   m_elasticityFallOffEdit;
    EditBox   m_frictionEdit;
    EditBox   m_scatterAngleEdit;
    ComboBox  m_physicsMaterialCombo;
    HWND      m_hCanDrop;
    HWND      m_hIsBottomCollidable;
};
