#ifndef H_PROTECT_TABLE_DIALOG
#define H_PROTECT_TABLE_DIALOG

class ProtectTableDialog : public CDialog
{
public:
    ProtectTableDialog();

protected:
    virtual BOOL OnInitDialog();
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    virtual void OnOK();

};

class UnprotectDialog : public CDialog
{
public:
    UnprotectDialog();

protected:
    virtual BOOL OnInitDialog();
    virtual void OnOK();

};


#endif // !H_PROTECT_TABLE_DIALOG
