// license:GPLv3+

#pragma once

class CollectionDialogStruct final
{
public:
    Collection *pcol;
    PinTable *ppt;
};

class CollectionManagerDialog final : public CDialog
{
public:
    CollectionManagerDialog();

protected:
    BOOL OnInitDialog() override;
    INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
    BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
    void OnOK() override;
    void OnClose() override;
    void OnCancel() override;

private:
    void EditCollection();
    void LoadPosition();
    void SavePosition();

    HWND hListHwnd;
    static int m_columnSortOrder;
};

class CollectionDialog final : public CDialog
{
public:
    CollectionDialog(CollectionDialogStruct &pcds);

protected:
    BOOL OnInitDialog() override;
    BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
    void OnOK() override;

private:
    CollectionDialogStruct &pCurCollection;
};
