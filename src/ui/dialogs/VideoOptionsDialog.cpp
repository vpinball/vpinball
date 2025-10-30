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

   void InitDisplayControls(const Settings::Section wndSection, const string& wndSettingPrefix, const bool embeddable);
   void LoadDisplaySettings();
   void SaveDisplaySettings();

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

   void UpdateFullscreenModesList();
   void UpdateDisplayHeightFromWidth();
   void SelectAspectRatio(int w, int h);
   vector<VPX::Window::VideoMode> m_allVideoModes;
   Settings::Section m_wndSection = Settings::Plugin00;
   string m_wndSettingPrefix;
   bool m_wndEmbeddable;
   vector<VPX::Window::DisplayConfig> m_displays;
   CComboBox m_wndDisplay;
   CListBox m_wndVideoModes;
   CComboBox m_wndAspectRatio;
   CButton m_wndFullscreen;
   CButton m_wndWindowed;
   CButton m_wndForce10bit;
   CEdit m_wndWidth;
   CEdit m_wndHeight;
   CEdit m_wndX;
   CEdit m_wndY;
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
   BOOL OnApply() override;

   void LoadSettings(Settings& settings) override;
   void SaveSettings(Settings& settings, bool saveAll) override;

private:
   CButton m_ballOverrideImages;
   CEdit   m_ballImage;
   CEdit   m_ballDecal;
};

class DisplayStyleOptPage final : public VideoOptionPropPage
{
public:
   DisplayStyleOptPage(Settings& appSettings, Settings& tableSettings);
   ~DisplayStyleOptPage() override { }

   DisplayStyleOptPage(const DisplayStyleOptPage&) = delete;
   DisplayStyleOptPage& operator=(const DisplayStyleOptPage&) = delete;

protected:
   BOOL OnInitDialog() override;
   BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
   INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
   BOOL OnApply() override;

   void LoadSettings(Settings& settings) override;
   void SaveSettings(Settings& settings, bool saveAll) override;

   void LoadProfile(const int n);
   void ResetProfile(const int n);
   void SaveProfile();

private:
   int m_editedProfile = -1;

   CListBox m_dmdType;

   CButton m_legacyRenderer;
   CButton m_dmdScaleFX;

   ColorButton m_dotTint;
   CEdit m_dotSize;
   CEdit m_dotBrightness;
   CEdit m_dotSharpness;
   ColorButton m_unlitDotColor;
   CEdit m_diffuseGlow;

   CColorDialog m_colorDialog;
};

class CabinetOptPage final : public VideoOptionPropPage
{
public:
   CabinetOptPage(Settings& appSettings, Settings& tableSettings);
   ~CabinetOptPage() override { }

   CabinetOptPage(const RenderOptPage&) = delete;
   CabinetOptPage& operator=(const CabinetOptPage&) = delete;

protected:
   BOOL OnInitDialog() override;
   BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
   BOOL OnApply() override;

   void LoadSettings(Settings& settings) override;
   void SaveSettings(Settings& settings, bool saveAll) override;

private:
   CEdit m_playerX;
   CEdit m_playerY;
   CEdit m_playerZ;
   CButton m_bamHeadtracking; // TODO move to plugin

   CEdit m_lockbarWidth;
   CEdit m_lockbarHeight;

   CEdit m_playfieldScreenWidth;
   CEdit m_playfieldScreenHeight;
   CEdit m_playfieldScreenInclination;
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

class ScoreViewOptPage final : public VideoOptionPropPage
{
public:
   ScoreViewOptPage(Settings& appSettings, Settings& tableSettings);
   ~ScoreViewOptPage() override { }

   ScoreViewOptPage(const ScoreViewOptPage&) = delete;
   ScoreViewOptPage& operator=(const ScoreViewOptPage&) = delete;

protected:
   BOOL OnInitDialog() override;
   BOOL OnApply() override;

   void LoadSettings(Settings& settings) override;
   void SaveSettings(Settings& settings, bool saveAll) override;
};

class BackglassViewOptPage final : public VideoOptionPropPage
{
public:
   BackglassViewOptPage(Settings& appSettings, Settings& tableSettings);
   ~BackglassViewOptPage() override { }

   BackglassViewOptPage(const BackglassViewOptPage&) = delete;
   BackglassViewOptPage& operator=(const BackglassViewOptPage&) = delete;

protected:
   BOOL OnInitDialog() override;
   BOOL OnApply() override;

   void LoadSettings(Settings& settings) override;
   void SaveSettings(Settings& settings, bool saveAll) override;

private:
   CComboBox m_viewMode;
};

class VRPreviewOptPage final : public VideoOptionPropPage
{
public:
   VRPreviewOptPage(Settings& appSettings, Settings& tableSettings);
   ~VRPreviewOptPage() override { }

   VRPreviewOptPage(const VRPreviewOptPage&) = delete;
   VRPreviewOptPage& operator=(const VRPreviewOptPage&) = delete;

protected:
   BOOL OnInitDialog() override;
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
   AddPage(new RenderOptPage(m_appSettings, m_tableSettings));
   AddPage(new DisplayStyleOptPage(m_appSettings, m_tableSettings));
   AddPage(new CabinetOptPage(m_appSettings, m_tableSettings));
   AddPage(new PFViewOptPage(m_appSettings, m_tableSettings));
   #if defined(ENABLE_BGFX)
      AddPage(new ScoreViewOptPage(m_appSettings, m_tableSettings));
      AddPage(new BackglassViewOptPage(m_appSettings, m_tableSettings));
   #endif
   #if defined(ENABLE_XR) || defined(ENABLE_VR)
      AddPage(new VRPreviewOptPage(m_appSettings, m_tableSettings));
   #endif
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// Base class with implementation for app/table settings and window output

#pragma region VideoOptionProperties

static const int2 aspectRatios[] = {
   int2( 0,  0), // Free
   int2( 4,  3), // [Landscape]
   int2(16, 10),
   int2(16,  9),
   int2(21, 10),
   int2(21,  9),
   int2( 4,  1), // For DMD
   int2( 3,  4), // [Portrait]
   int2(10, 16),
   int2( 9, 16),
   int2(10, 21),
   int2( 9, 21),
   int2( 1,  4), // For DMD
};

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


void VideoOptionPropPage::InitDisplayControls(const Settings::Section wndSection, const string& wndSettingPrefix, const bool embeddable)
{
   m_wndSection = wndSection;
   m_wndSettingPrefix = wndSettingPrefix;
   m_wndEmbeddable = embeddable;
   AttachItem(IDC_DISPLAY_ID, m_wndDisplay);
   AddToolTip(m_wndDisplay, "Select Display for Video output.");
   AttachItem(IDC_SIZELIST, m_wndVideoModes);
   AttachItem(IDC_AR_COMBO, m_wndAspectRatio);
   AttachItem(IDC_EXCLUSIVE_FULLSCREEN, m_wndFullscreen);
   if (IsWindows10_1803orAbove())
      AddToolTip(m_wndFullscreen, "Enforces exclusive Fullscreen Mode.\r\nEnforcing exclusive FS can slightly reduce input lag.");
   else
      AddToolTip(m_wndFullscreen, "Enforces exclusive Fullscreen Mode.\r\nDo not enable if you require to see the VPinMAME or B2S windows for example.\r\nEnforcing exclusive FS can slightly reduce input lag though.");
   AttachItem(IDC_WINDOWED, m_wndWindowed);
   AttachItem(IDC_10BIT_VIDEO, m_wndForce10bit);
   AddToolTip(m_wndForce10bit, "Enforces 10Bit (WCG) rendering.\r\nRequires a corresponding 10Bit output capable graphics card and monitor.\r\nAlso requires to have exclusive fullscreen mode enforced (for now).");
   AttachItem(IDC_WIDTH_EDIT, m_wndWidth);
   AttachItem(IDC_HEIGHT_EDIT, m_wndHeight);
   AttachItem(IDC_X_OFFSET_EDIT, m_wndX);
   AttachItem(IDC_Y_OFFSET_EDIT, m_wndY);
   m_wndAspectRatio.SetRedraw(false);
   m_wndAspectRatio.AddString("Free");
   for (size_t j = 1; j < std::size(aspectRatios); j++)
   {
      const string tmp = (aspectRatios[j].x > aspectRatios[j].y ? "Landscape: "s : "Portrait: "s) + std::to_string(max(aspectRatios[j].x, aspectRatios[j].y)) + " x " + std::to_string(min(aspectRatios[j].x, aspectRatios[j].y));
      m_wndAspectRatio.AddString(tmp.c_str());
   }
   m_wndAspectRatio.SetRedraw(true);
   UpdateFullscreenModesList();
}

void VideoOptionPropPage::UpdateFullscreenModesList()
{
   int display = m_wndDisplay.GetCurSel();
   if (display < 0)
      return;
   if (m_wndEmbeddable)
      display -= 2;

   if (m_wndEmbeddable && (display == -2))
   {
      // Disabled
      m_wndVideoModes.ShowWindow(0);
      m_wndAspectRatio.ShowWindow(0);
      m_wndFullscreen.ShowWindow(0);
      m_wndWindowed.ShowWindow(0);
      m_wndForce10bit.ShowWindow(0);
      m_wndX.ShowWindow(0);
      m_wndY.ShowWindow(0);
      m_wndWidth.ShowWindow(0);
      m_wndHeight.ShowWindow(0);
      GetDlgItem(IDC_AR_LABEL).ShowWindow(0);
      GetDlgItem(IDC_X_LABEL).ShowWindow(0);
      GetDlgItem(IDC_Y_LABEL).ShowWindow(0);
      GetDlgItem(IDC_WIDTH_LABEL).ShowWindow(0);
      GetDlgItem(IDC_HEIGHT_LABEL).ShowWindow(0);
      GetDlgItem(IDC_RESET_WINDOW).ShowWindow(0);
      return;
   }

   if (m_wndEmbeddable && (display == -1))
   {
      // Display embedded in another display
      m_wndVideoModes.ShowWindow(0);
      m_wndAspectRatio.ShowWindow(1);
      m_wndFullscreen.ShowWindow(0);
      m_wndWindowed.ShowWindow(0);
      m_wndForce10bit.ShowWindow(0);
      m_wndX.ShowWindow(1);
      m_wndY.ShowWindow(1);
      m_wndWidth.ShowWindow(1);
      m_wndHeight.ShowWindow(1);
      GetDlgItem(IDC_AR_LABEL).ShowWindow(1);
      GetDlgItem(IDC_X_LABEL).ShowWindow(1);
      GetDlgItem(IDC_Y_LABEL).ShowWindow(1);
      GetDlgItem(IDC_WIDTH_LABEL).ShowWindow(1);
      GetDlgItem(IDC_HEIGHT_LABEL).ShowWindow(1);
      GetDlgItem(IDC_RESET_WINDOW).ShowWindow(0);
      return;
   }

   m_wndFullscreen.ShowWindow(1);
   m_wndWindowed.ShowWindow(1);
   const bool fullscreen = m_wndFullscreen.GetCheck() == BST_CHECKED;
   // Fullscreen settings
   m_wndVideoModes.ShowWindow(fullscreen ? 1 : 0);
   m_wndForce10bit.ShowWindow(fullscreen ? 1 : 0);
   // Window settings
   GetDlgItem(IDC_AR_LABEL).ShowWindow(fullscreen ? 0 : 1);
   m_wndAspectRatio.ShowWindow(fullscreen ? 0 : 1);
   GetDlgItem(IDC_WIDTH_LABEL).ShowWindow(fullscreen ? 0 : 1);
   m_wndWidth.ShowWindow(fullscreen ? 0 : 1);
   GetDlgItem(IDC_HEIGHT_LABEL).ShowWindow(fullscreen ? 0 : 1);
   m_wndHeight.ShowWindow(fullscreen ? 0 : 1);
   GetDlgItem(IDC_X_LABEL).ShowWindow(fullscreen ? 0 : 1);
   m_wndX.ShowWindow(fullscreen ? 0 : 1);
   GetDlgItem(IDC_Y_LABEL).ShowWindow(fullscreen ? 0 : 1);
   m_wndY.ShowWindow(fullscreen ? 0 : 1);
   GetDlgItem(IDC_RESET_WINDOW).ShowWindow(fullscreen ? 0 : 1);

   m_allVideoModes = VPX::Window::GetDisplayModes(m_displays[display]);
   int screenwidth = m_displays[display].width;
   int screenheight = m_displays[display].height;
   
   const int depthcur = GetEditedSettings().LoadValueWithDefault(m_wndSection, m_wndSettingPrefix + "ColorDepth", 32);
   const float refreshrate = GetEditedSettings().LoadValueWithDefault(m_wndSection, m_wndSettingPrefix + "RefreshRate", 0.f);
   const int widthcur = GetEditedSettings().LoadValueWithDefault(m_wndSection, m_wndSettingPrefix + "Width", -1);
   const int heightcur = GetEditedSettings().LoadValueWithDefault(m_wndSection, m_wndSettingPrefix + "Height", -1);
   VPX::Window::VideoMode curSelMode;
   curSelMode.width = widthcur;
   curSelMode.height = heightcur;
   curSelMode.depth = depthcur;
   curSelMode.refreshrate = refreshrate;
   
   m_wndVideoModes.SetRedraw(false);
   m_wndVideoModes.ResetContent();
   int bestMatch = 0, bestMatchingPoints = 0; // to find closest matching res
   for (size_t i = 0; i < m_allVideoModes.size(); ++i)
   {
      double best = DBL_MAX;
      int2 bestAR;
      for (size_t j = 1; j < std::size(aspectRatios); j++)
      {
         const double fit = abs(1. - (double)(m_allVideoModes[i].height * aspectRatios[j].x) / (double)(m_allVideoModes[i].width * aspectRatios[j].y));
         if (fit < best)
         {
            bestAR = aspectRatios[j];
            best = fit;
         }
      }
      char szT[128];
      sprintf_s(szT, sizeof(szT), "%d x %d (%.1fHz %d:%d)", m_allVideoModes[i].width, m_allVideoModes[i].height, m_allVideoModes[i].refreshrate, max(bestAR.y, bestAR.x), min(bestAR.x, bestAR.y));
      m_wndVideoModes.AddString(szT);

      int matchingPoints = 0;
      if (m_allVideoModes[i].width == curSelMode.width)
         matchingPoints += 100;
      if (m_allVideoModes[i].height == curSelMode.height)
         matchingPoints += 100;
      if (m_allVideoModes[i].depth == curSelMode.depth)
         matchingPoints += 50;
      if (m_allVideoModes[i].refreshrate == curSelMode.refreshrate)
         matchingPoints += 10;
      if (m_allVideoModes[i].width == screenwidth)
         matchingPoints += 3;
      if (m_allVideoModes[i].height == screenheight)
         matchingPoints += 3;
      if (m_allVideoModes[i].refreshrate == 60.f)
         matchingPoints += 1;
      if (matchingPoints > bestMatchingPoints)
      {
         bestMatch = (int)i;
         bestMatchingPoints = matchingPoints;
      }
   }
   m_wndVideoModes.SetCurSel(bestMatch);
   if (m_wndVideoModes.GetCurSel() == -1)
      m_wndVideoModes.SetCurSel(0);
   m_wndVideoModes.SetRedraw(true);
}

void VideoOptionPropPage::LoadDisplaySettings()
{
   Settings& settings = GetEditedSettings();
   BeginLoad();

   // FIXME remove or implement in SDL
   m_wndForce10bit.EnableWindow(false);

   m_wndDisplay.SetRedraw(false);
   m_wndDisplay.ResetContent();
   m_wndDisplay.SetItemData((int)m_allVideoModes.size(), (DWORD)(m_allVideoModes.size() * 128));
   if (m_wndEmbeddable)
   {
      int embedded = settings.LoadValueWithDefault(m_wndSection, m_wndSettingPrefix + "Output", VPX::RenderOutput::OM_DISABLED); // VPX::RenderOutput::OM_WINDOW
      m_wndDisplay.AddString("Disabled");
      m_wndDisplay.AddString("Embedded");
      if (embedded != VPX::RenderOutput::OM_WINDOW)
         m_wndDisplay.SetCurSel(embedded);
   }
   m_displays.clear();
   m_displays = VPX::Window::GetDisplays();
   const string selectedDisplay = settings.LoadValueString(m_wndSection, m_wndSettingPrefix + "Display");
   bool displaySelected = false;
   for (const auto& dispConf : m_displays)
   {
      const string displayName = (dispConf.isPrimary ? '*' : ' ') + std::to_string(dispConf.width) + 'x' + std::to_string(dispConf.height) + " [" + dispConf.displayName + ']';
      m_wndDisplay.AddString(displayName.c_str());
      if (dispConf.displayName == selectedDisplay)
      {
         displaySelected = true;
         m_wndDisplay.SetCurSel(m_wndDisplay.GetCount() - 1);
      }
      else if (!displaySelected && dispConf.isPrimary)
         m_wndDisplay.SetCurSel(m_wndDisplay.GetCount() - 1);
   }
   m_wndDisplay.SetRedraw(true);

   const bool fullscreen = settings.LoadValueBool(m_wndSection, m_wndSettingPrefix + "FullScreen"); // IsWindows10_1803orAbove());
   m_wndFullscreen.SetCheck(fullscreen ? BST_CHECKED : BST_UNCHECKED);
   m_wndWindowed.SetCheck(fullscreen ? BST_UNCHECKED : BST_CHECKED);
   OnCommand(IDC_EXCLUSIVE_FULLSCREEN, 0L); // Force UI update

   const int x = settings.LoadValueWithDefault(m_wndSection, m_wndSettingPrefix + "WndX", -1);
   const int y = settings.LoadValueWithDefault(m_wndSection, m_wndSettingPrefix + "WndY", -1);
   SetDlgItemInt(IDC_X_OFFSET_EDIT, x, TRUE);
   SetDlgItemInt(IDC_Y_OFFSET_EDIT, y, TRUE);

   const int width = settings.LoadValueWithDefault(m_wndSection, m_wndSettingPrefix + "Width", -1);
   const int height = settings.LoadValueWithDefault(m_wndSection, m_wndSettingPrefix + "Height", -1);
   SetDlgItemInt(IDC_WIDTH_EDIT, width, TRUE);
   SetDlgItemInt(IDC_HEIGHT_EDIT, height, TRUE);

   SelectAspectRatio(width, height);
   if(settings.LoadValueWithDefault(m_wndSection, m_wndSettingPrefix + "AspectRatio", -1) == 0) // Free
      m_wndAspectRatio.SetCurSel(0);

   UpdateDisplayHeightFromWidth();
   EndLoad();
}

void VideoOptionPropPage::SaveDisplaySettings()
{
   if (m_editedSettings == nullptr)
      return;
   Settings& settings = *m_editedSettings;
   const bool saveAll = !IsTableSettings();

   int display = m_wndDisplay.GetCurSel();
   if (display < 0)
      return;

   bool embedded = false;
   if (m_wndEmbeddable)
   {
      settings.SaveValue(m_wndSection, m_wndSettingPrefix + "Output", min(display, 2), !saveAll);
      display -= 2;
   }
   if (display < 0)
      embedded = true;
   else
      settings.SaveValue(m_wndSection, m_wndSettingPrefix + "Display", m_displays[display].displayName, !saveAll);

   if (embedded)
   {
      const int x = GetDlgItemInt(IDC_X_OFFSET_EDIT, TRUE);
      const int y = GetDlgItemInt(IDC_Y_OFFSET_EDIT, TRUE);
      const int width = GetDlgItemInt(IDC_WIDTH_EDIT, TRUE);
      const int height = GetDlgItemInt(IDC_HEIGHT_EDIT, TRUE);
      settings.SaveValue(m_wndSection, m_wndSettingPrefix + "WndX", x, !saveAll);
      settings.SaveValue(m_wndSection, m_wndSettingPrefix + "WndY", y, !saveAll);
      settings.SaveValue(m_wndSection, m_wndSettingPrefix + "Width", width, !saveAll);
      settings.SaveValue(m_wndSection, m_wndSettingPrefix + "Height", height, !saveAll);
   }
   else
   {
      const bool fullscreen = m_wndFullscreen.GetCheck() == BST_CHECKED;
      if (fullscreen)
      {
         const int index = max(0, m_wndVideoModes.GetCurSel());
         if (index >= 0 && (size_t)index < m_allVideoModes.size())
         {
            const VPX::Window::VideoMode* const pvm = &m_allVideoModes[index];
            settings.SaveValue(m_wndSection, m_wndSettingPrefix + "Width", pvm->width, !saveAll);
            settings.SaveValue(m_wndSection, m_wndSettingPrefix + "Height", pvm->height, !saveAll);
            settings.SaveValue(m_wndSection, m_wndSettingPrefix + "ColorDepth", pvm->depth, !saveAll);
            settings.SaveValue(m_wndSection, m_wndSettingPrefix + "RefreshRate", pvm->refreshrate, !saveAll);
         }
      }
      else
      {
         const int arMode = m_wndAspectRatio.GetCurSel();
         const int x = GetDlgItemInt(IDC_X_OFFSET_EDIT, TRUE);
         const int y = GetDlgItemInt(IDC_Y_OFFSET_EDIT, TRUE);
         const int width = GetDlgItemInt(IDC_WIDTH_EDIT, TRUE);
               int height = GetDlgItemInt(IDC_HEIGHT_EDIT, TRUE);
         if (arMode > 0)
            height = (int)(width * (double)aspectRatios[arMode].y / (double)aspectRatios[arMode].x);
         if (!saveAll)
         {
            settings.DeleteValue(m_wndSection, m_wndSettingPrefix + "AspectRatio");
            settings.DeleteValue(m_wndSection, m_wndSettingPrefix + "WndX");
            settings.DeleteValue(m_wndSection, m_wndSettingPrefix + "WndY");
            settings.DeleteValue(m_wndSection, m_wndSettingPrefix + "Width");
            settings.DeleteValue(m_wndSection, m_wndSettingPrefix + "Height");
            settings.DeleteValue(m_wndSection, m_wndSettingPrefix + "ColorDepth");
            settings.DeleteValue(m_wndSection, m_wndSettingPrefix + "RefreshRate");
         }
         settings.SaveValue(m_wndSection, m_wndSettingPrefix + "WndX", x, !saveAll);
         settings.SaveValue(m_wndSection, m_wndSettingPrefix + "WndY", y, !saveAll);
         if (width > 0 && height > 0)
         {
            settings.SaveValue(m_wndSection, m_wndSettingPrefix + "AspectRatio", arMode, !saveAll);
            settings.SaveValue(m_wndSection, m_wndSettingPrefix + "Width", width, !saveAll);
            settings.SaveValue(m_wndSection, m_wndSettingPrefix + "Height", height, !saveAll);
         }
      }
      settings.SaveValue(m_wndSection, m_wndSettingPrefix + "FullScreen", fullscreen, !saveAll);
      settings.SaveValue(m_wndSection, m_wndSettingPrefix + "Render10Bit", m_wndForce10bit.GetCheck() == BST_CHECKED, !saveAll);
   }
   settings.Validate(false);
   settings.Save();
}

void VideoOptionPropPage::SelectAspectRatio(int width, int height)
{
   double best = DBL_MAX;
   int bestAR;
   for (int j = 1; j < (int)std::size(aspectRatios); j++)
   {
      const double fit = abs(1. - (double)(height * aspectRatios[j].x) / (double)(width * aspectRatios[j].y));
      if (fit < best)
      {
         bestAR = j;
         best = fit;
      }
   }
   m_wndAspectRatio.SetCurSel(best < 0.01 ? bestAR : 0);
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
   // View display controls
   case IDC_WINDOWED:
   case IDC_EXCLUSIVE_FULLSCREEN:
      if (HIWORD(wParam) == BN_CLICKED)
         PropChanged();
      UpdateFullscreenModesList();
      break;
   case IDC_DISPLAY_ID:
      if (HIWORD(wParam) == CBN_SELCHANGE)
         PropChanged();
      UpdateFullscreenModesList();
      // TODO clamp user width/height to display size ?
      // TODO select aspect ratio according to display ?
      break;
   case IDC_SIZELIST:
      if (HIWORD(wParam) == CBN_SELCHANGE)
         PropChanged();
      if (m_wndFullscreen.GetCheck() == BST_CHECKED)
      {
         int index = m_wndVideoModes.GetCurSel();
         if (index >= 0 && (size_t)index < m_allVideoModes.size())
         {
            const VPX::Window::VideoMode* const pvm = &m_allVideoModes[index];
            SetDlgItemInt(IDC_WIDTH_EDIT, pvm->width, TRUE);
            SetDlgItemInt(IDC_HEIGHT_EDIT, pvm->height, TRUE);
            SelectAspectRatio(pvm->width, pvm->height);
         }
      }
      break;
   case IDC_AR_COMBO:
   case IDC_WIDTH_EDIT:
      if ((LOWORD(wParam) == IDC_AR_COMBO && HIWORD(wParam) == CBN_SELCHANGE)
       || (LOWORD(wParam) == IDC_WIDTH_EDIT && HIWORD(wParam) == EN_CHANGE))
         PropChanged();
      UpdateDisplayHeightFromWidth();
      break;
   case IDC_HEIGHT_EDIT:
   case IDC_X_OFFSET_EDIT:
   case IDC_Y_OFFSET_EDIT:
      if (HIWORD(wParam) == EN_CHANGE)
         PropChanged();
      break;
   case IDC_RESET_WINDOW:
      PropChanged();
      SetDlgItemInt(IDC_X_OFFSET_EDIT, -1, TRUE);
      SetDlgItemInt(IDC_Y_OFFSET_EDIT, -1, TRUE);
      (void)m_appSettings.DeleteValue(m_wndSection, m_wndSettingPrefix + "WndX");
      (void)m_appSettings.DeleteValue(m_wndSection, m_wndSettingPrefix + "WndY");
      break;
   default:
      return FALSE;
   }
   return TRUE;
}

void VideoOptionPropPage::UpdateDisplayHeightFromWidth()
{
   int arMode = max(0, m_wndAspectRatio.GetCurSel());
   m_wndHeight.EnableWindow(arMode == 0);
   if (arMode > 0)
   {
      int width = GetDlgItemInt(IDC_WIDTH_EDIT, TRUE);
      int height = (int)(width * (double)aspectRatios[arMode].y / (double)aspectRatios[arMode].x);
      SetDlgItemInt(IDC_HEIGHT_EDIT, height, TRUE);
   }
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

   const bool overwiteBallImage = settings.LoadValueWithDefault(Settings::Player, "OverwriteBallImage"s, false);
   m_ballOverrideImages.SetCheck(overwiteBallImage ? BST_CHECKED : BST_UNCHECKED);
   string imageName;
   if (!settings.LoadValue(Settings::Player, "BallImage"s, imageName))
      imageName.clear();
   m_ballImage.SetWindowText(imageName.c_str());
   if (!settings.LoadValue(Settings::Player, "DecalImage"s, imageName))
      imageName.clear();
   m_ballDecal.SetWindowText(imageName.c_str());
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
   BOOL nothing = 0;

   const bool overwriteEnabled = m_ballOverrideImages.GetCheck() == BST_CHECKED;
   settings.SaveValue(Settings::Player, "OverwriteBallImage"s, overwriteEnabled, !saveAll);
   if (overwriteEnabled)
   {
      settings.SaveValue(Settings::Player, "BallImage"s, m_ballImage.GetWindowText().GetString(), !saveAll);
      settings.SaveValue(Settings::Player, "DecalImage"s, m_ballDecal.GetWindowText().GetString(), !saveAll);
   }
   else if (!saveAll)
   {
      settings.DeleteValue(Settings::Player, "BallImage"s);
      settings.DeleteValue(Settings::Player, "DecalImage"s);
   }
}

BOOL RenderOptPage::OnApply()
{
   ApplyChanges();
   return TRUE;
}

#pragma endregion



////////////////////////////////////////////////////////////////////////////////////////////////////
// Cabinet Settings

#pragma region CabinetOptPage

CabinetOptPage::CabinetOptPage(Settings& appSettings, Settings& tableSettings)
   : VideoOptionPropPage(IDD_CABINET_OPT, _T("Cabinet Options"), appSettings, tableSettings)
{
}

BOOL CabinetOptPage::OnInitDialog()
{
   VideoOptionPropPage::OnInitDialog();

   AttachItem(IDC_SCREEN_PLAYERX, m_playerX);
   AttachItem(IDC_SCREEN_PLAYERY, m_playerY);
   AttachItem(IDC_SCREEN_PLAYERZ, m_playerZ);
   AttachItem(IDC_HEADTRACKING, m_bamHeadtracking);
   AddToolTip(m_bamHeadtracking, "Enables BAM Headtracking. See https://www.ravarcade.pl for details.");

   AttachItem(IDC_SCREEN_WIDTH, m_playfieldScreenWidth);
   AddToolTip(m_playfieldScreenWidth, "Physical width of the display area of the playfield screen in centimeters, in landscape orientation (width > height).\r\n\r\nThis is needed to get the correct size when using 'Window' mode for the camera.");
   AttachItem(IDC_SCREEN_HEIGHT, m_playfieldScreenHeight);
   AddToolTip(m_playfieldScreenHeight, "Physical height of the display area of the playfield screen in centimeters, in landscape orientation (width > height).\r\n\r\nThis is needed to get the correct size when using 'Window' mode for the camera.");
   AttachItem(IDC_SCREEN_INCLINATION, m_playfieldScreenInclination);

   AttachItem(IDC_LOCKBAR_WIDTH, m_lockbarWidth);
   AddToolTip(m_lockbarWidth, "Physical width of the lockbar in centimeters.\r\n\r\nThis is needed for Virtual Reality automatic scaling mode.");
   AttachItem(IDC_LOCKBAR_HEIGHT_FROM_GROUND, m_lockbarHeight);
   AddToolTip(m_lockbarHeight, "Physical height of the lockbar in centimeters measured from ground.\r\n\r\nThis is used for Virtual Reality positioning.");

   LoadSettings(GetEditedSettings());
   return TRUE;
}

void CabinetOptPage::LoadSettings(Settings& settings)
{
   BeginLoad();

   m_playerX.SetWindowText(f2sz(settings.LoadValueFloat(Settings::Player, "ScreenPlayerX"s)).c_str());
   m_playerY.SetWindowText(f2sz(settings.LoadValueFloat(Settings::Player, "ScreenPlayerY"s)).c_str());
   m_playerZ.SetWindowText(f2sz(settings.LoadValueFloat(Settings::Player, "ScreenPlayerZ"s)).c_str());
   m_bamHeadtracking.SetCheck(settings.LoadValueWithDefault(Settings::Player, "BAMHeadTracking"s, false) ? BST_CHECKED : BST_UNCHECKED);

   m_playfieldScreenWidth.SetWindowText(f2sz(settings.LoadValueFloat(Settings::Player, "ScreenWidth"s)).c_str());
   m_playfieldScreenHeight.SetWindowText(f2sz(settings.LoadValueFloat(Settings::Player, "ScreenHeight"s)).c_str());
   m_playfieldScreenInclination.SetWindowText(f2sz(settings.LoadValueFloat(Settings::Player, "ScreenInclination"s)).c_str());

   m_lockbarWidth.SetWindowText(f2sz(settings.LoadValueFloat(Settings::Player, "LockbarWidth"s)).c_str());
   m_lockbarHeight.SetWindowText(f2sz(settings.LoadValueFloat(Settings::Player, "LockbarHeight"s)).c_str());

   EndLoad();
}

void CabinetOptPage::SaveSettings(Settings& settings, bool saveAll)
{
   settings.SaveValue(Settings::Player, "ScreenPlayerX"s, sz2f(GetDlgItemText(IDC_SCREEN_PLAYERX).GetString()), !saveAll);
   settings.SaveValue(Settings::Player, "ScreenPlayerY"s, sz2f(GetDlgItemText(IDC_SCREEN_PLAYERY).GetString()), !saveAll);
   settings.SaveValue(Settings::Player, "ScreenPlayerZ"s, sz2f(GetDlgItemText(IDC_SCREEN_PLAYERZ).GetString()), !saveAll);
   settings.SaveValue(Settings::Player, "BAMheadTracking"s, m_bamHeadtracking.GetCheck() == BST_CHECKED, !saveAll);

   settings.SaveValue(Settings::Player, "ScreenWidth"s, sz2f(GetDlgItemText(IDC_SCREEN_WIDTH).GetString()), !saveAll);
   settings.SaveValue(Settings::Player, "ScreenHeight"s, sz2f(GetDlgItemText(IDC_SCREEN_HEIGHT).GetString()), !saveAll);
   settings.SaveValue(Settings::Player, "ScreenInclination"s, sz2f(GetDlgItemText(IDC_SCREEN_INCLINATION).GetString()), !saveAll);

   settings.SaveValue(Settings::Player, "LockbarWidth"s, sz2f(GetDlgItemText(IDC_LOCKBAR_WIDTH).GetString()), !saveAll);
   settings.SaveValue(Settings::Player, "LockbarHeight"s, sz2f(GetDlgItemText(IDC_LOCKBAR_HEIGHT_FROM_GROUND).GetString()), !saveAll);
}

BOOL CabinetOptPage::OnApply()
{
   ApplyChanges();
   return TRUE;
}

BOOL CabinetOptPage::OnCommand(WPARAM wParam, LPARAM lParam)
{
   switch (LOWORD(wParam))
   {
   case IDC_SCREEN_PLAYERX:
   case IDC_SCREEN_PLAYERY:
   case IDC_SCREEN_PLAYERZ:
   case IDC_SCREEN_WIDTH:
   case IDC_SCREEN_HEIGHT:
   case IDC_SCREEN_INCLINATION:
   case IDC_LOCKBAR_WIDTH:
   case IDC_LOCKBAR_HEIGHT_FROM_GROUND:
      if (HIWORD(wParam) == EN_CHANGE)
         PropChanged();
      break;
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
   InitDisplayControls(Settings::Player, "Playfield"s, false);
   LoadSettings(GetEditedSettings());
   return TRUE;
}

void PFViewOptPage::LoadSettings(Settings& settings)
{
   BeginLoad();
   m_viewMode.SetCurSel(settings.LoadValueWithDefault(Settings::Player, "BGSet"s, 0));
   LoadDisplaySettings();
   EndLoad();
}

void PFViewOptPage::SaveSettings(Settings& settings, bool saveAll)
{
   settings.SaveValue(Settings::Player, "BGSet"s, max(m_viewMode.GetCurSel(), 0), !saveAll);
   SaveDisplaySettings();
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



////////////////////////////////////////////////////////////////////////////////////////////////////
// Display Render Options (CRT, DMD, Alpha segment)

#pragma region DMDDisplayStyleOptPage

DisplayStyleOptPage::DisplayStyleOptPage(Settings& appSettings, Settings& tableSettings)
   : VideoOptionPropPage(IDD_DMDSTYLE_OPT, _T("Display Styles"), appSettings, tableSettings)
{
}

BOOL DisplayStyleOptPage::OnInitDialog()
{
   VideoOptionPropPage::OnInitDialog();

   AttachItem(IDC_DMD_PROFILE, m_dmdType);
   SetupList(m_dmdType , 15, 
      "DMD: Legacy VPX", "DMD: Neon Plasma", "DMD: Red LED", "DMD: Green LED", "DMD: Yellow LED", "DMD: Generic Plasma", "DMD: Generic LED",
      "Alpha: Neon Plasma", "Alpha: Blue VFD", "Alpha: Green VFD", "Alpha: Red LED", "Alpha: Green LED", "Alpha: Yellow LED", "Alpha: Generic Plasma", "Alpha: Generic LED");

   AttachItem(IDC_LEGACY_RENDERER, m_legacyRenderer);

   AttachItem(IDC_SCALE_FX_DMD, m_dmdScaleFX);

   AttachItem(IDC_DOT_TINT, m_dotTint);
   AttachItem(IDC_DOT_SIZE, m_dotSize);
   AttachItem(IDC_DOT_BRIGHTNESS, m_dotBrightness);
   AttachItem(IDC_DOT_SHARPNESS, m_dotSharpness);
   AttachItem(IDC_UNLIT_DOT_COLOR, m_unlitDotColor);
   AttachItem(IDC_BACK_GLOW, m_diffuseGlow);

   m_dmdType.SetCurSel(0);
   OnCommand(IDC_DMD_PROFILE, 0L);
   OnCommand(IDC_BG_SET, 0L);
   LoadSettings(GetEditedSettings());
   return TRUE;
}

void DisplayStyleOptPage::LoadSettings(Settings& settings)
{
   BeginLoad();
   OnCommand(IDC_DMD_PROFILE, 0L);
   EndLoad();
}

void DisplayStyleOptPage::SaveSettings(Settings& settings, bool saveAll)
{
   SaveProfile();
}

void DisplayStyleOptPage::ResetProfile(const int n)
{
   Settings& settings = GetEditedSettings();
   if (n < 7) // DMD
   {
      const string prefix = "Profile." + std::to_string(n + 1) + '.';
      settings.ResetValue(Settings::DMD, prefix + "Legacy");
      settings.ResetValue(Settings::DMD, prefix + "ScaleFX");
      settings.ResetValue(Settings::DMD, prefix + "DotTint");
      settings.ResetValue(Settings::DMD, prefix + "UnlitDotColor");
      settings.ResetValue(Settings::DMD, prefix + "DotSize");
      settings.ResetValue(Settings::DMD, prefix + "DotBrightness");
      settings.ResetValue(Settings::DMD, prefix + "DotSharpness");
      settings.ResetValue(Settings::DMD, prefix + "DiffuseGlow");
   }
   else if (n < 7 + 8) // Alpha Seg
   {
      const string prefix = "Profile." + std::to_string(n - 7 + 1) + '.';
      settings.ResetValue(Settings::Alpha, prefix + "Color");
      settings.ResetValue(Settings::Alpha, prefix + "Unlit");
      settings.ResetValue(Settings::Alpha, prefix + "Brightness");
      settings.ResetValue(Settings::Alpha, prefix + "DiffuseGlow");
   }
   LoadProfile(n);
}

void DisplayStyleOptPage::LoadProfile(const int n)
{
   BeginLoad();
   Settings& settings = GetEditedSettings();
   m_editedProfile = n;

   int mode = -1;
   if (n < 7) // DMD
   {
      mode = 0;
      const string prefix = "Profile." + std::to_string(n + 1) + '.';
      m_legacyRenderer.SetCheck(settings.LoadValueBool(Settings::DMD, prefix + "Legacy") ? BST_CHECKED : BST_UNCHECKED);
      m_dmdScaleFX.SetCheck(settings.LoadValueBool(Settings::DMD, prefix + "ScaleFX") ? BST_CHECKED : BST_UNCHECKED);
      m_dotTint.SetColor(settings.LoadValueUInt(Settings::DMD, prefix + "DotTint"));
      m_unlitDotColor.SetColor(settings.LoadValueUInt(Settings::DMD, prefix + "UnlitDotColor"));
      m_dotSize.SetWindowText(f2sz(settings.LoadValueFloat(Settings::DMD, prefix + "DotSize")).c_str());
      m_dotBrightness.SetWindowText(f2sz(settings.LoadValueFloat(Settings::DMD, prefix + "DotBrightness")).c_str());
      m_dotSharpness.SetWindowText(f2sz(settings.LoadValueFloat(Settings::DMD, prefix + "DotSharpness")).c_str());
      m_diffuseGlow.SetWindowText(f2sz(settings.LoadValueFloat(Settings::DMD, prefix + "DiffuseGlow")).c_str());
   }
   else if (n < 7 + 8) // Alpha Seg
   {
      mode = 1;
      const string prefix = "Profile." + std::to_string(n - 7 + 1) + '.';
      m_dotTint.SetColor(settings.LoadValueUInt(Settings::Alpha, prefix + "Color"));
      m_unlitDotColor.SetColor(settings.LoadValueUInt(Settings::Alpha, prefix + "Unlit"));
      m_dotBrightness.SetWindowText(f2sz(settings.LoadValueFloat(Settings::Alpha, prefix + "Brightness")).c_str());
      m_diffuseGlow.SetWindowText(f2sz(settings.LoadValueFloat(Settings::Alpha, prefix + "DiffuseGlow")).c_str());
   }

   m_legacyRenderer.ShowWindow(mode == 0 ? SW_SHOWNORMAL : SW_HIDE);
   m_dmdScaleFX.ShowWindow(mode == 0 ? SW_SHOWNORMAL : SW_HIDE);
   GetDlgItem(IDC_STATIC20).ShowWindow(mode == 0 ? SW_SHOWNORMAL : SW_HIDE);
   m_dotSize.ShowWindow(mode == 0 ? SW_SHOWNORMAL : SW_HIDE);
   GetDlgItem(IDC_STATIC21).ShowWindow(mode == 0 ? SW_SHOWNORMAL : SW_HIDE);
   m_dotSharpness.ShowWindow(mode == 0 ? SW_SHOWNORMAL : SW_HIDE);

   OnCommand(IDC_LEGACY_RENDERER, 0L);

   EndLoad();
}

void DisplayStyleOptPage::SaveProfile()
{
   if (m_editedProfile < 0)
      return;

   Settings& settings = GetEditedSettings();
   const bool saveAll = !IsTableSettings();

   if (m_editedProfile < 7) // DMD
   {
      const string prefix = "Profile." + std::to_string(m_editedProfile + 1) + '.';
      settings.SaveValue(Settings::DMD, prefix + "Legacy", m_legacyRenderer.GetCheck() == BST_CHECKED, !saveAll);
      settings.SaveValue(Settings::DMD, prefix + "ScaleFX", m_dmdScaleFX.GetCheck() == BST_CHECKED, !saveAll);
      settings.SaveValue(Settings::DMD, prefix + "DotTint", static_cast<unsigned int>(m_dotTint.GetColor()), !saveAll);
      settings.SaveValue(Settings::DMD, prefix + "UnlitDotColor", static_cast<unsigned int>(m_unlitDotColor.GetColor()), !saveAll);
      settings.SaveValue(Settings::DMD, prefix + "DotSize", sz2f(m_dotSize.GetWindowText().GetString()), !saveAll);
      settings.SaveValue(Settings::DMD, prefix + "DotBrightness", sz2f(m_dotBrightness.GetWindowText().GetString()), !saveAll);
      settings.SaveValue(Settings::DMD, prefix + "DotSharpness", sz2f(m_dotSharpness.GetWindowText().GetString()), !saveAll);
      settings.SaveValue(Settings::DMD, prefix + "DiffuseGlow", sz2f(m_diffuseGlow.GetWindowText().GetString()), !saveAll);
   }
   else if (m_editedProfile < 7 + 8) // AlphaSeg
   {
      const string prefix = "Profile." + std::to_string(m_editedProfile - 7 + 1) + '.';
      settings.SaveValue(Settings::Alpha, prefix + "Color", static_cast<unsigned int>(m_dotTint.GetColor()), !saveAll);
      settings.SaveValue(Settings::Alpha, prefix + "Unlit", static_cast<unsigned int>(m_unlitDotColor.GetColor()), !saveAll);
      settings.SaveValue(Settings::Alpha, prefix + "Brightness", sz2f(m_dotBrightness.GetWindowText().GetString()), !saveAll);
      settings.SaveValue(Settings::Alpha, prefix + "DiffuseGlow", sz2f(m_diffuseGlow.GetWindowText().GetString()), !saveAll);
   }
}

BOOL DisplayStyleOptPage::OnApply()
{
   ApplyChanges();
   return TRUE;
}

INT_PTR DisplayStyleOptPage::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   switch (uMsg)
   {
   case WM_DRAWITEM:
      switch (wParam)
      {
      case IDC_DOT_TINT: m_dotTint.DrawItem(reinterpret_cast<LPDRAWITEMSTRUCT>(lParam)); return TRUE;
      case IDC_UNLIT_DOT_COLOR: m_unlitDotColor.DrawItem(reinterpret_cast<LPDRAWITEMSTRUCT>(lParam)); return TRUE;
      }
      break;
   }
   return DialogProcDefault(uMsg, wParam, lParam);
}

BOOL DisplayStyleOptPage::OnCommand(WPARAM wParam, LPARAM lParam)
{
   switch (LOWORD(wParam))
   {
   case IDC_RESET_WINDOW:
      PropChanged();
      ResetProfile(m_dmdType.GetCurSel());
      break;
   case IDC_SCALE_FX_DMD:
      if (HIWORD(wParam) == BN_CLICKED)
         PropChanged();
      break;
   case IDC_EXPOSURE:
   case IDC_DOT_SIZE:
   case IDC_DOT_BRIGHTNESS:
   case IDC_DOT_SHARPNESS:
   case IDC_BACK_GLOW:
      if (HIWORD(wParam) == EN_CHANGE)
         PropChanged();
      break;
   case IDC_LEGACY_RENDERER:
      if (HIWORD(wParam) == BN_CLICKED)
         PropChanged();
      {
#ifdef ENABLE_BGFX
         const bool isNewRenderer = m_legacyRenderer.GetCheck() == BST_UNCHECKED;
#else
         constexpr bool isNewRenderer = false;
#endif
         m_dotSize.EnableWindow(isNewRenderer);
         m_dotSharpness.EnableWindow(isNewRenderer);
         m_unlitDotColor.EnableWindow(isNewRenderer || (m_dmdType.GetCurSel() > 6));
         m_diffuseGlow.EnableWindow(isNewRenderer || (m_dmdType.GetCurSel() > 6));
      }
      break;
   case IDC_DMD_PROFILE:
      if (HIWORD(wParam) == CBN_SELCHANGE)
         PropChanged();
      SaveProfile();
      LoadProfile(m_dmdType.GetCurSel());
      switch (m_dmdType.GetCurSel())
      {
      case 0: SetDlgItemText(IDC_DMD_TYPE_INFO, "DMD rendering profile matching rendering style used in previous VPX versions."); break;
      case 1: SetDlgItemText(IDC_DMD_TYPE_INFO, "DMD rendering profile matching Neon Plasma DMD used in pinballs from 1991 to 2012."); break;
      case 2: SetDlgItemText(IDC_DMD_TYPE_INFO, "DMD rendering profile matching red LED DMD used in pinball machines after 2012."); break;
      case 3: SetDlgItemText(IDC_DMD_TYPE_INFO, "DMD rendering profile designed for rendering green LED matrix."); break;
      case 4: SetDlgItemText(IDC_DMD_TYPE_INFO, "DMD rendering profile designed for rendering yellow LED matrix."); break;
      case 5: SetDlgItemText(IDC_DMD_TYPE_INFO, "DMD rendering profile for custom setup (Generic uncolored Plasma)."); break;
      case 6: SetDlgItemText(IDC_DMD_TYPE_INFO, "DMD rendering profile for custom setup (Generic uncolored LED)."); break;

      case 7: SetDlgItemText(IDC_DMD_TYPE_INFO, "Alpha rendering profile matching Neon Plasma displays"); break;
      case 8: SetDlgItemText(IDC_DMD_TYPE_INFO, "Alpha rendering profile matching blue VFD displays largely used by Gottlieb manufacturer (Futaba VFD parts)"); break;
      case 9: SetDlgItemText(IDC_DMD_TYPE_INFO, "Alpha rendering profile matching green VFD displays"); break;
      case 10: SetDlgItemText(IDC_DMD_TYPE_INFO, "Alpha rendering profile matching common red LED display"); break;
      case 11: SetDlgItemText(IDC_DMD_TYPE_INFO, "Alpha rendering profile designed for rendering green LED"); break;
      case 12: SetDlgItemText(IDC_DMD_TYPE_INFO, "Alpha rendering profile designed for rendering yellow LED"); break;
      case 13: SetDlgItemText(IDC_DMD_TYPE_INFO, "Alpha rendering profile for custom setup (Generic uncolored Plasma)."); break;
      case 14: SetDlgItemText(IDC_DMD_TYPE_INFO, "Alpha rendering profile for custom setup (Generic uncolored LED)."); break;
      }
      break;
   case IDC_DOT_TINT:
   {
      CHOOSECOLOR cc = m_colorDialog.GetParameters();
      cc.Flags = CC_FULLOPEN | CC_RGBINIT;
      m_colorDialog.SetParameters(cc);
      if (g_pvp->GetActiveTable())
         m_colorDialog.SetCustomColors(g_pvp->GetActiveTable()->m_rgcolorcustom);
      m_colorDialog.SetColor(m_dotTint.GetColor());
      if (m_colorDialog.DoModal(GetHwnd()) == IDOK)
      {
         if (g_pvp->GetActiveTable())
            memcpy(g_pvp->GetActiveTable()->m_rgcolorcustom, m_colorDialog.GetCustomColors(), sizeof(g_pvp->GetActiveTable()->m_rgcolorcustom));
         m_dotTint.SetColor(m_colorDialog.GetColor());
      }
   }
   break;
   case IDC_UNLIT_DOT_COLOR:
   {
      CHOOSECOLOR cc = m_colorDialog.GetParameters();
      cc.Flags = CC_FULLOPEN | CC_RGBINIT;
      m_colorDialog.SetParameters(cc);
      if (g_pvp->GetActiveTable())
         m_colorDialog.SetCustomColors(g_pvp->GetActiveTable()->m_rgcolorcustom);
      m_colorDialog.SetColor(m_unlitDotColor.GetColor());
      if (m_colorDialog.DoModal(GetHwnd()) == IDOK)
      {
         if (g_pvp->GetActiveTable())
            memcpy(g_pvp->GetActiveTable()->m_rgcolorcustom, m_colorDialog.GetCustomColors(), sizeof(g_pvp->GetActiveTable()->m_rgcolorcustom));
         m_unlitDotColor.SetColor(m_colorDialog.GetColor());
      }
   }
   break;
   default: return VideoOptionPropPage::OnCommand(wParam, lParam);
   }
   return TRUE;
}

#pragma endregion




////////////////////////////////////////////////////////////////////////////////////////////////////
// Score View Options (DMD or Alpha Seg)

#pragma region ScoreViewOptPage

ScoreViewOptPage::ScoreViewOptPage(Settings& appSettings, Settings& tableSettings)
   : VideoOptionPropPage(IDD_SCOREVIEW_OPT, _T("Score View"), appSettings, tableSettings)
{
}

BOOL ScoreViewOptPage::OnInitDialog()
{
   VideoOptionPropPage::OnInitDialog();
   InitDisplayControls(Settings::ScoreView, "ScoreView"s, true);
   LoadSettings(GetEditedSettings());
   return TRUE;
}

void ScoreViewOptPage::LoadSettings(Settings& settings)
{
   BeginLoad();
   LoadDisplaySettings();
   EndLoad();
}

void ScoreViewOptPage::SaveSettings(Settings& settings, bool saveAll)
{
   SaveDisplaySettings();
}

BOOL ScoreViewOptPage::OnApply()
{
   ApplyChanges();
   return TRUE;
}

#pragma endregion



////////////////////////////////////////////////////////////////////////////////////////////////////
// Backglass View Options

#pragma region BackglassViewOptPage

BackglassViewOptPage::BackglassViewOptPage(Settings& appSettings, Settings& tableSettings)
   : VideoOptionPropPage(IDD_BACKGLASS_OPT, _T("Backglass View"), appSettings, tableSettings)
{
}

BOOL BackglassViewOptPage::OnInitDialog()
{
   VideoOptionPropPage::OnInitDialog();
   InitDisplayControls(Settings::Backglass, "Backglass"s, true);
   LoadSettings(GetEditedSettings());
   return TRUE;
}

void BackglassViewOptPage::LoadSettings(Settings& settings)
{
   BeginLoad();
   LoadDisplaySettings();
   EndLoad();
}

void BackglassViewOptPage::SaveSettings(Settings& settings, bool saveAll)
{
   SaveDisplaySettings();
}

BOOL BackglassViewOptPage::OnApply()
{
   ApplyChanges();
   return TRUE;
}

#pragma endregion

////////////////////////////////////////////////////////////////////////////////////////////////////
// VR Preview Options

#pragma region VRPreviewOptPage

VRPreviewOptPage::VRPreviewOptPage(Settings& appSettings, Settings& tableSettings)
   : VideoOptionPropPage(IDD_VRPREVIEW_OPT, _T("VR Preview"), appSettings, tableSettings)
{
}

BOOL VRPreviewOptPage::OnInitDialog()
{
   VideoOptionPropPage::OnInitDialog();
   InitDisplayControls(Settings::PlayerVR, "Preview"s, false);
   LoadSettings(GetEditedSettings());
   return TRUE;
}

void VRPreviewOptPage::LoadSettings(Settings& settings)
{
   BeginLoad();
   LoadDisplaySettings();
   EndLoad();
}

void VRPreviewOptPage::SaveSettings(Settings& settings, bool saveAll) { SaveDisplaySettings(); }

BOOL VRPreviewOptPage::OnApply()
{
   ApplyChanges();
   return TRUE;
}

#pragma endregion
