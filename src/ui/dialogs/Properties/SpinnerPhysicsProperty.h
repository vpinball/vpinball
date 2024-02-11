#ifndef H_SPINNER_PHYSICS_PROPERTY
#define H_SPINNER_PHYSICS_PROPERTY

class SpinnerPhysicsProperty: public BasePropertyDialog
{
public:
    SpinnerPhysicsProperty(const VectorProtected<ISelect> *pvsel);
    virtual ~SpinnerPhysicsProperty()
    {
    }

    void UpdateVisuals(const int dispid=-1);
    void UpdateProperties(const int dispid);

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    EditBox m_dampingEdit;
    EditBox m_elasticityEdit;
};
#endif
