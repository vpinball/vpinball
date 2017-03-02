#ifndef H_COLLECTION_MANAGER_DIALOG
#define H_COLLECTION_MANAGER_DIALOG

class CollectionDialogStruct
{
public:
    Collection *pcol;
    PinTable *ppt;
};

class CollectionManagerDialog : public CDialog
{
public:
    CollectionManagerDialog();

protected:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    virtual void OnOK();
    virtual void OnClose();
    virtual void OnCancel();

private:
    void EditCollection();
    void LoadPosition();
    void SavePosition();

    HWND hListHwnd;
};

class CollectionDialog : public CDialog
{
public:
    CollectionDialog(CollectionDialogStruct &pcds);

protected:
    virtual BOOL OnInitDialog();
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    virtual void OnOK();

private:
    CollectionDialogStruct &pCurCollection;
};

#endif
