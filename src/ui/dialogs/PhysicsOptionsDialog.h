#ifndef H_PHYSICS_OPTIONS_DIALOG
#define H_PHYSICS_OPTIONS_DIALOG

class PhysicsOptionsDialog : public CDialog
{
public:
    PhysicsOptionsDialog();

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    virtual void OnOK();
    virtual void OnDestroy();
private:
    bool LoadSetting();
    CString GetItemText(int id);
    void SetItemText(int id, float value);
    void SaveCurrentPhysicsSetting();

};

#endif
