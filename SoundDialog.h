#ifndef H_SOUND_DIALOG
#define H_SOUND_DIALOG

class SoundDialog : public CDialog
{
public:
    SoundDialog();
    virtual ~SoundDialog();

protected:
    virtual void OnDestroy();
    virtual void OnClose();
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc( UINT uMsg, WPARAM wParam, LPARAM lParam );
    virtual BOOL OnCommand( WPARAM wParam, LPARAM lParam );
    virtual void OnOK();
    virtual void OnCancel();
private:
    void Import();
    void ReImport();
    void ReImportFrom();
    void Export();
    void SoundToBG();
    void DeleteSound();
    void LoadPosition();
    void SavePosition();

    HWND hSoundList;
    char m_filename[MAX_PATH];
    char m_initDir[MAX_PATH];

};

#endif