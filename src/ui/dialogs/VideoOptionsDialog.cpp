// license:GPLv3+

#include "core/stdafx.h"
#include <memory>
#include "VideoOptionsDialog.h"
#include "renderer/Window.h"
#include "ui/resource.h"


class VideoOptionPropPage : public CPropertyPage
{
public:
   VideoOptionPropPage(const VideoOptionPropPage&) = delete;
   VideoOptionPropPage& operator=(const VideoOptionPropPage&) = delete;

protected:
   VideoOptionPropPage(UINT templateID, LPCTSTR title, Settings& appSettings, Settings& tableSettings);
   BOOL OnInitDialog() override;
   BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;

   Settings& GetEditedSettings() const { return *m_editedSettings; }
   bool IsTableSettings() const { return m_editedSettings == &m_appSettings; }
   void BeginLoad() { m_loading++;  }
   void PropChanged() { if (m_loading == 0) PropSheet_Changed(GetParent(), GetHwnd()); }
   void EndLoad() { m_loading--; }
   void ApplyChanges();

   virtual void LoadSettings(Settings& settings) = 0;
   virtual void SaveSettings(Settings& settings, bool saveAll) = 0;

   void SetupList(CListBox& combo, int n, ...);
   void SetupCombo(CComboBox& combo, int n, ...);
   void AddToolTip(CWnd& wnd, const char* tip);
   void BrowseImage(CEdit& editCtl);

private:
   int m_loading = 0;
   Settings* m_editedSettings = nullptr;
   Settings& m_appSettings;
   Settings& m_tableSettings;
   CButton m_appSettingBtn;
   CButton m_tableSettingBtn;

   CToolTip m_tooltip;
};

class RenderOptPage final : public VideoOptionPropPage
{
public:
   RenderOptPage(Settings& appSettings, Settings& tableSettings);
   ~RenderOptPage() override { }

   RenderOptPage(const RenderOptPage&) = delete;
   RenderOptPage& operator=(const RenderOptPage&) = delete;

protected:
   BOOL OnInitDialog() override;
   BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
   BOOL OnApply() override;

   void LoadSettings(Settings& settings) override;
   void SaveSettings(Settings& settings, bool saveAll) override;

private:
   CButton m_bamHeadtracking; // TODO move to plugin
   CButton m_ballOverrideImages;
   CEdit   m_ballImage;
   CEdit   m_ballDecal;
};

class PFViewOptPage final : public VideoOptionPropPage
{
public:
   PFViewOptPage(Settings& appSettings, Settings& tableSettings);
   ~PFViewOptPage() override { }

   PFViewOptPage(const PFViewOptPage&) = delete;
   PFViewOptPage& operator=(const PFViewOptPage&) = delete;

protected:
   BOOL OnInitDialog() override;
   BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
   BOOL OnApply() override;

   void LoadSettings(Settings& settings) override;
   void SaveSettings(Settings& settings, bool saveAll) override;

private:
   CComboBox m_viewMode;
};


VideoOptionProperties::VideoOptionProperties(HWND hParent /* = nullptr*/)
   : CPropertySheet(_T("Video Options"), hParent)
{
   m_appSettings = g_pvp->m_settings;
   if (g_pvp->m_ptableActive)
   {
      m_tableSettings = g_pvp->m_ptableActive->m_settings;
      m_tableSettings.SetParent(&m_appSettings);
   }
   AddPage(new PFViewOptPage(m_appSettings, m_tableSettings));
   AddPage(new RenderOptPage(m_appSettings, m_tableSettings));
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// Base class with implementation for app/table settings

#pragma region VideoOptionProperties

VideoOptionPropPage::VideoOptionPropPage(UINT templateID, LPCTSTR title, Settings& appSettings, Settings& tableSettings)
   : CPropertyPage(templateID, title)
   , m_appSettings(appSettings)
   , m_tableSettings(tableSettings)
{
}

BOOL VideoOptionPropPage::OnInitDialog()
{
   AttachItem(IDC_APPLICATION_SETTINGS, m_appSettingBtn);
   AttachItem(IDC_TABLE_OVERRIDE, m_tableSettingBtn);
   m_appSettingBtn.SetCheck(BST_CHECKED);
   m_tableSettingBtn.EnableWindow(g_pvp->m_ptableActive != nullptr);
   m_editedSettings = &m_appSettings;
   m_tooltip.Create(GetHwnd());
   m_tooltip.SetMaxTipWidth(320);
   //m_tooltip.SetWindowTheme(L" ", L" "); // Turn XP themes off
   return TRUE;
}

void VideoOptionPropPage::SetupList(CListBox& combo, int n, ...)
{
   va_list arguments;
   va_start(arguments, n);
   combo.SetRedraw(false);
   combo.ResetContent();
   for (int i = 0; i < n; i++)
      combo.AddString(va_arg(arguments, char*));
   combo.SetRedraw(true);
   va_end(arguments);
}

void VideoOptionPropPage::SetupCombo(CComboBox& combo, int n, ...)
{
   va_list arguments;
   va_start(arguments, n);
   combo.SetRedraw(false);
   combo.ResetContent();
   for (int i = 0; i < n; i++)
      combo.AddString(va_arg(arguments, char*));
   combo.SetRedraw(true);
   va_end(arguments);
}

void VideoOptionPropPage::AddToolTip(CWnd& wnd, const char* const tip) {
   m_tooltip.AddTool(wnd, tip);
}

void VideoOptionPropPage::BrowseImage(CEdit& editCtl)
{
   char szFileName[MAXSTRING];
   szFileName[0] = '\0';
   OPENFILENAME ofn = {};
   ofn.lStructSize = sizeof(OPENFILENAME);
   ofn.hInstance = g_pvp->theInstance;
   ofn.hwndOwner = g_pvp->GetHwnd();
   ofn.lpstrFilter = "Bitmap, JPEG, PNG, TGA, WEBP, EXR, HDR Files (.bmp/.jpg/.png/.tga/.webp/.exr/.hdr)\0*.bmp;*.jpg;*.jpeg;*.png;*.tga;*.webp;*.exr;*.hdr\0";
   ofn.lpstrFile = szFileName;
   ofn.nMaxFile = sizeof(szFileName);
   ofn.lpstrDefExt = "png";
   ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
   if (!GetOpenFileName(&ofn))
      return;
   PropChanged();
   editCtl.SetWindowText(szFileName);
}

BOOL VideoOptionPropPage::OnCommand(WPARAM wParam, LPARAM lParam)
{
   UNREFERENCED_PARAMETER(lParam);
   switch (LOWORD(wParam))
   {
   // App/Table settings
   case IDC_TABLE_OVERRIDE:
      if (m_editedSettings != nullptr && m_editedSettings != &m_tableSettings)
         SaveSettings(m_appSettings, true);
      m_editedSettings = &m_tableSettings;
      LoadSettings(GetEditedSettings());
      break;
   case IDC_APPLICATION_SETTINGS:
      if (m_editedSettings != nullptr && m_editedSettings != &m_appSettings)
         SaveSettings(m_tableSettings, false);
      m_editedSettings = &m_appSettings;
      LoadSettings(GetEditedSettings());
      break;
   default:
      return FALSE;
   }
   return TRUE;
}

void VideoOptionPropPage::ApplyChanges()
{
   if (m_editedSettings != nullptr)
      SaveSettings(*m_editedSettings, m_editedSettings == &m_appSettings);
   g_pvp->m_settings = m_appSettings;
   if (g_pvp->m_ptableActive)
   {
      g_pvp->m_ptableActive->m_settings = m_tableSettings;
      g_pvp->m_ptableActive->m_settings.SetParent(&g_pvp->m_settings);
   }
   g_pvp->m_settings.Save();
   if (g_pvp->m_ptableActive && !g_pvp->m_ptableActive->GetSettingsFileName().empty())
      g_pvp->m_ptableActive->m_settings.SaveToFile(g_pvp->m_ptableActive->GetSettingsFileName());
}

#pragma endregion



////////////////////////////////////////////////////////////////////////////////////////////////////
// Global Render Options

#pragma region RenderOptionProperties

RenderOptPage::RenderOptPage(Settings& appSettings, Settings& tableSettings)
   : VideoOptionPropPage(IDD_RENDER_OPT, _T("Render Options"), appSettings, tableSettings)
{
}

BOOL RenderOptPage::OnInitDialog()
{
   VideoOptionPropPage::OnInitDialog();
   AttachItem(IDC_HEADTRACKING, m_bamHeadtracking);
   AddToolTip(m_bamHeadtracking, "Enables BAM Headtracking. See https://www.ravarcade.pl for details.");
   AttachItem(IDC_OVERWRITE_BALL_IMAGE_CHECK, m_ballOverrideImages);
   AddToolTip(m_ballOverrideImages, "When checked, it overwrites the ball image/decal image(s) for every table.");
   AttachItem(IDC_BALL_IMAGE_EDIT, m_ballImage);
   AttachItem(IDC_BALL_DECAL_EDIT, m_ballDecal);
   LoadSettings(GetEditedSettings());
   return TRUE;
}

void RenderOptPage::LoadSettings(Settings& settings)
{
   BeginLoad();

   m_bamHeadtracking.SetCheck(settings.GetPlayer_BAMHeadTracking() ? BST_CHECKED : BST_UNCHECKED);

   const bool overwiteBallImage = settings.GetPlayer_OverwriteBallImage();
   m_ballOverrideImages.SetCheck(overwiteBallImage ? BST_CHECKED : BST_UNCHECKED);
   m_ballImage.SetWindowText(settings.GetPlayer_BallImage().c_str());
   m_ballDecal.SetWindowText(settings.GetPlayer_DecalImage().c_str());
   if (!overwiteBallImage)
   {
      GetDlgItem(IDC_BROWSE_BALL_IMAGE).EnableWindow(FALSE);
      GetDlgItem(IDC_BROWSE_BALL_DECAL).EnableWindow(FALSE);
      m_ballImage.EnableWindow(FALSE);
      m_ballDecal.EnableWindow(FALSE);
   }
   EndLoad();
}

void RenderOptPage::SaveSettings(Settings& settings, bool saveAll)
{
   settings.SetPlayer_BAMHeadTracking(m_bamHeadtracking.GetCheck() == BST_CHECKED, !saveAll);

   const bool overwriteEnabled = m_ballOverrideImages.GetCheck() == BST_CHECKED;
   settings.SetPlayer_OverwriteBallImage(overwriteEnabled, !saveAll);
   settings.SetPlayer_BallImage(overwriteEnabled ? m_ballImage.GetWindowText().GetString() : ""s, !saveAll);
   settings.SetPlayer_DecalImage(overwriteEnabled ? m_ballDecal.GetWindowText().GetString() : ""s, !saveAll);
}

BOOL RenderOptPage::OnApply()
{
   ApplyChanges();
   return TRUE;
}

BOOL RenderOptPage::OnCommand(WPARAM wParam, LPARAM lParam)
{
   switch (LOWORD(wParam))
   {
   case IDC_HEADTRACKING:
      if (HIWORD(wParam) == BN_CLICKED)
         PropChanged();
      break;
   default: return VideoOptionPropPage::OnCommand(wParam, lParam);
   }
   return TRUE;
}

#pragma endregion


////////////////////////////////////////////////////////////////////////////////////////////////////
// Main Playfield View Options

#pragma region PFViewOptPage

PFViewOptPage::PFViewOptPage(Settings& appSettings, Settings& tableSettings)
   : VideoOptionPropPage(IDD_PFVIEW_OPT, _T("Playfield View"), appSettings, tableSettings)
{
}

BOOL PFViewOptPage::OnInitDialog()
{
   VideoOptionPropPage::OnInitDialog();
   AttachItem(IDC_BG_SET, m_viewMode);
   m_viewMode.SetRedraw(false);
   m_viewMode.AddString("Desktop & FSS");
   m_viewMode.AddString("Cabinet");
   m_viewMode.AddString("Desktop (no FSS)");
   m_viewMode.SetRedraw(true);
   AddToolTip(m_viewMode, "Defines the view mode used when running a table\n\nDesktop/FSS will use the FSS view for table with FSS enabled, desktop otherwise.\n\nCabinet uses the 'fullscreen' view\n\nDesktop always uses the desktop view (no FSS)");
   LoadSettings(GetEditedSettings());
   return TRUE;
}

void PFViewOptPage::LoadSettings(Settings& settings)
{
   BeginLoad();
   m_viewMode.SetCurSel(settings.GetPlayer_BGSet());
   EndLoad();
}

void PFViewOptPage::SaveSettings(Settings& settings, bool saveAll)
{
   settings.SetPlayer_BGSet(max(m_viewMode.GetCurSel(), 0), !saveAll);
   // update the cached current view setup of all loaded tables since it also depends on this setting
   for (auto table : g_pvp->m_vtable)
      table->UpdateCurrentBGSet();
}

BOOL PFViewOptPage::OnApply()
{
   ApplyChanges();
   return TRUE;
}

BOOL PFViewOptPage::OnCommand(WPARAM wParam, LPARAM lParam)
{
   switch (LOWORD(wParam))
   {
   case IDC_BG_SET:
      if (HIWORD(wParam) == CBN_SELCHANGE)
         PropChanged();
      break;
   default: return VideoOptionPropPage::OnCommand(wParam, lParam);
   }
   return TRUE;
}

#pragma endregion
