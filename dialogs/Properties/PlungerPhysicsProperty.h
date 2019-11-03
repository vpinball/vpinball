#ifndef H_PLUNGER_PHYSICS_PROPERTY
#define H_PLUNGER_PHYSICS_PROPERTY

class PlungerPhysicsProperty: public BasePropertyDialog
{
public:
    PlungerPhysicsProperty(VectorProtected<ISelect> *pvsel);
    virtual ~PlungerPhysicsProperty()
    {
    }

    void UpdateVisuals();

    void UpdateProperties(const int dispid);
protected:
    virtual BOOL OnInitDialog();
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

private:
    CEdit m_pullSpeedEdit;
    CEdit m_releaseSpeedEdit;
    CEdit m_strokeLengthEdit;
    CEdit m_scatterVelocityEdit;
    HWND  m_hEnableMechPlungerCheck;
    HWND  m_hAutoPlungerCheck;
    CEdit m_mechStrengthEdit;
    CEdit m_momentumXferEdit;
    CEdit m_parkPositionEdit;

};

#endif // !H_PLUNGER_PHYSICS_PROPERTY

