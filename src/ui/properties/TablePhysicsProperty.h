// license:GPLv3+

#pragma once

class TablePhysicsProperty: public BasePropertyDialog
{
public:
    TablePhysicsProperty(const VectorProtected<ISelect> *pvsel);
    virtual ~TablePhysicsProperty() { }

    void UpdateVisuals(const int dispid = -1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

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
    EditBox m_minSlopeEdit;
    EditBox m_maxSlopeEdit;
    EditBox m_gameplayDifficultyEdit;
    ComboBox m_overwritePhysicsSetCombo;
    HWND    m_hOverwriteFlipperCheck;
    vector<string> m_physicSetList;
};
