// license:GPLv3+

#pragma once

class AudioOptionsDialog final : public CDialog
{
public:
   AudioOptionsDialog();

protected:
   BOOL OnInitDialog() override;
   BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;

   Settings& GetEditedSettings();
   void LoadSettings();
   void SaveSettings(const bool saveAll);

   Settings* m_editedSettings = nullptr;
   Settings m_appSettings, m_tableSettings;
};
