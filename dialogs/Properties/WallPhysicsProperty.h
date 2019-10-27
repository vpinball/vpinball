#ifndef H_WALL_PHYSICS_PROPERTY
#define H_WALL_PHYSICS_PROPERTY

class WallPhysicsProperty: public CDialog
{
public:
    WallPhysicsProperty(Surface *wall);
    virtual ~WallPhysicsProperty()
    {
    }

    void UpdateVisuals();

    void UpdateProperties(const int dispid);
protected:
    virtual BOOL OnInitDialog();
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

private:
    Surface  *m_wall;
    CEdit     m_hitThresholdEdit;
    CEdit     m_slingshotForceEdit;
    CEdit     m_slingshotThresholdEdit;
    CEdit     m_elasticityEdit;
    CEdit     m_frictionEdit;
    CEdit     m_scatterAngleEdit;
    CComboBox m_physicsMaterialCombo;
};

#endif

