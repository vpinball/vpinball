// license:GPLv3+

#pragma once

class AudioOptionsDialog : public CDialog
{
public:
   AudioOptionsDialog();

protected:
   BOOL OnInitDialog() final;
   BOOL OnCommand(WPARAM wParam, LPARAM lParam) final;

   Settings& GetEditedSettings();
   void LoadSettings();
   void SaveSettings(const bool saveAll);

   Settings* m_editedSettings = nullptr;
   Settings m_appSettings, m_tableSettings;
};
