// license:GPLv3+

#pragma once

class TablePhysicsProperty final : public BasePropertyDialog
{
public:
    TablePhysicsProperty(const VectorProtected<ISelect> *pvsel);
    ~TablePhysicsProperty() override { }

    void UpdateVisuals(const int dispid = -1) override;
    void UpdateProperties(const int dispid) override;

protected:
    BOOL OnInitDialog() override;
    BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
    INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
    CButton m_importSetButton;
    CButton m_exportSetButton;
    HWND    m_hFilterMechanicalPlungerCheck;
    EditBox m_gravityConstantEdit;
    EditBox m_playfieldFrictionEdit;
    EditBox m_playfieldElasticityEdit;
    EditBox m_playfieldElasticityFalloffEdit;
    EditBox m_playfieldScatterEdit;
    EditBox m_defaultScatterEdit;
    EditBox m_nudgeTimeEdit;
    EditBox m_physicsLoopEdit;
    EditBox m_mechPlungerAdjEdit;
    EditBox m_tableWidthEdit;
    EditBox m_tableHeightEdit;
    EditBox m_bottomGlassHeightEdit;
    EditBox m_topGlassHeightEdit;
    EditBox m_groundToPlayfieldHeightEdit;
    EditBox m_minSlopeEdit;
    EditBox m_maxSlopeEdit;
    EditBox m_gameplayDifficultyEdit;
    ComboBox m_overwritePhysicsSetCombo;
    HWND    m_hOverwriteFlipperCheck;
    vector<string> m_physicSetList;
};
