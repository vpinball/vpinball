#pragma once

class AudioOptionsDialog : public CDialog
{
public:
   AudioOptionsDialog();

protected:
   BOOL OnInitDialog() final;
   BOOL OnCommand(WPARAM wParam, LPARAM lParam) final;
   void OnOK() final;

   Settings& GetEditedSettings();
   void LoadSettings();
   void SaveSettings(const bool saveAll);
};
