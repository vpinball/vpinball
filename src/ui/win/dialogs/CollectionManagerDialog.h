// license:GPLv3+

#pragma once

class PinTableWnd;

class CollectionDialogStruct final
{
public:
    Collection *pcol;
    class PinTableWnd *ppt;
};

class CollectionManagerDialog final : public CDialog
{
public:
    explicit CollectionManagerDialog(PinTableWnd *tableEditor);

protected:
    BOOL OnInitDialog() override;
    INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
    BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
    void OnOK() override;
    void OnClose() override;
    void OnCancel() override;

private:
    PinTableWnd *const m_tableEditor;

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
