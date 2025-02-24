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
   virtual BOOL OnInitDialog();
   virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

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
   Settings& m_appSettings, m_tableSettings;
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

class RenderOptPage : public VideoOptionPropPage
{
public:
   RenderOptPage(Settings& appSettings, Settings& tableSettings);
   virtual ~RenderOptPage() override { }

   RenderOptPage(const RenderOptPage&) = delete;
   RenderOptPage& operator=(const RenderOptPage&) = delete;

protected:
   virtual BOOL OnInitDialog();
   virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
   virtual BOOL OnApply() override;

   virtual void LoadSettings(Settings& settings);
   virtual void SaveSettings(Settings& settings, bool saveAll);

private:
   void ResetVideoPreferences(int profile);

   int m_initialMaxTexDim;

   CComboBox m_syncMode;
   CEdit m_maxFPS;
   CEdit m_maxFrameLatency;

   CComboBox m_stereoMode;
   CButton m_stereoFake;
   CComboBox m_stereoFilter;
   CEdit   m_stereoBrightness;
   CEdit   m_stereoSaturation;
   CEdit   m_stereoEyeSeparation;
   CEdit   m_stereoMaxSeparation;
   CEdit   m_stereoZPD;
   CEdit   m_stereoOffset;
   CButton m_stereoYAxis;

   CComboBox m_maxAO;
   CComboBox m_maxReflection;
   CComboBox m_maxTexSize;
   CButton m_compressTexture;
   CButton m_forceAnisoMax;
   CButton m_forceTMOff;
   CEdit   m_forceTMscale;
   CButton m_forceBloomOff;
   CButton m_forceMotionBlurOff;
   CButton m_useAltDepth;
   CButton m_softwareVertex;
   CButton m_disableDWM;
   CSlider m_rampDetail;

   CButton m_ballTrails;
   CEdit   m_ballTrailStrength;
   CButton m_ballForceRound;
   CButton m_ballDisableLighting;
   CButton m_ballOverrideImages;
   CEdit   m_ballImage;
   CEdit   m_ballDecal;

   CComboBox m_msaaSamples;
   CComboBox m_supersampling;
   CComboBox m_postprocAA;
   CComboBox m_sharpen;

   CEdit   m_visualNudge;
   CButton m_useAdditionalSSR;

   CButton m_overrideNightDay;
   CSlider m_nightDay;
   CButton m_autoNightDay;
   CEdit   m_geoposLat;
   CEdit   m_geoposLon;

   CComboBox m_gfxBackend;

   #if defined(ENABLE_BGFX)
      static const string m_bgfxRendererNames[bgfx::RendererType::Count + 1];
   #endif
};

class DisplayStyleOptPage : public VideoOptionPropPage
{
public:
   DisplayStyleOptPage(Settings& appSettings, Settings& tableSettings);
   virtual ~DisplayStyleOptPage() override { }

   DisplayStyleOptPage(const DisplayStyleOptPage&) = delete;
   DisplayStyleOptPage& operator=(const DisplayStyleOptPage&) = delete;

protected:
   virtual BOOL OnInitDialog();
   virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
   virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
   virtual BOOL OnApply() override;

   virtual void LoadSettings(Settings& settings);
   virtual void SaveSettings(Settings& settings, bool saveAll);

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

class CabinetOptPage : public VideoOptionPropPage
{
public:
   CabinetOptPage(Settings& appSettings, Settings& tableSettings);
   virtual ~CabinetOptPage() override { }

   CabinetOptPage(const RenderOptPage&) = delete;
   CabinetOptPage& operator=(const CabinetOptPage&) = delete;

protected:
   virtual BOOL OnInitDialog();
   virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
   virtual BOOL OnApply() override;

   virtual void LoadSettings(Settings& settings);
   virtual void SaveSettings(Settings& settings, bool saveAll);

private:
   CEdit m_playerX;
   CEdit m_playerY;
   CEdit m_playerZ;
   CButton m_bamHeadtracking; // TODO move to plugin

   CComboBox m_display;
   CEdit m_displayWidth;
   CEdit m_displayHeight;
   CEdit m_displayInclination;
};

class PFViewOptPage : public VideoOptionPropPage
{
public:
   PFViewOptPage(Settings& appSettings, Settings& tableSettings);
   virtual ~PFViewOptPage() override { }

   PFViewOptPage(const PFViewOptPage&) = delete;
   PFViewOptPage& operator=(const PFViewOptPage&) = delete;

protected:
   virtual BOOL OnInitDialog();
   virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
   virtual BOOL OnApply() override;

   virtual void LoadSettings(Settings& settings);
   virtual void SaveSettings(Settings& settings, bool saveAll);

private:
   CComboBox m_viewMode;
};

class ScoreViewOptPage : public VideoOptionPropPage
{
public:
   ScoreViewOptPage(Settings& appSettings, Settings& tableSettings);
   virtual ~ScoreViewOptPage() override { }

   ScoreViewOptPage(const ScoreViewOptPage&) = delete;
   ScoreViewOptPage& operator=(const ScoreViewOptPage&) = delete;

protected:
   virtual BOOL OnInitDialog();
   virtual BOOL OnApply() override;

   virtual void LoadSettings(Settings& settings);
   virtual void SaveSettings(Settings& settings, bool saveAll);
};

class AlphaViewOptPage : public VideoOptionPropPage
{
public:
   AlphaViewOptPage(Settings& appSettings, Settings& tableSettings);
   virtual ~AlphaViewOptPage() override { }

   AlphaViewOptPage(const AlphaViewOptPage&) = delete;
   AlphaViewOptPage& operator=(const AlphaViewOptPage&) = delete;

protected:
   virtual BOOL OnInitDialog();
   virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
   virtual BOOL OnApply() override;

   virtual void LoadSettings(Settings& settings);
   virtual void SaveSettings(Settings& settings, bool saveAll);

private:
   CComboBox m_viewMode;
};

class BackglassViewOptPage : public VideoOptionPropPage
{
public:
   BackglassViewOptPage(Settings& appSettings, Settings& tableSettings);
   virtual ~BackglassViewOptPage() override { }

   BackglassViewOptPage(const BackglassViewOptPage&) = delete;
   BackglassViewOptPage& operator=(const BackglassViewOptPage&) = delete;

protected:
   virtual BOOL OnInitDialog();
   virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
   virtual BOOL OnApply() override;

   virtual void LoadSettings(Settings& settings);
   virtual void SaveSettings(Settings& settings, bool saveAll);

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
      // AddPage(new AlphaViewOptPage(m_appSettings, m_tableSettings));
      // AddPage(new BackglassViewOptPage(m_appSettings, m_tableSettings));
   #endif
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// Base class with implementation for app/table settings and window output

#pragma region VideoOptionProperties

// Declared in RenderDevice. Desktop composition may only be disabled on Windows Vista & 7
extern bool IsWindowsVistaOr7();

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
   m_tooltip.Create(*this);
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

void VideoOptionPropPage::AddToolTip(CWnd& wnd, const char* const text) {
   m_tooltip.AddTool(wnd, text);
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
   if (m_wndEmbeddable)
   {
      AttachItem(IDC_X_OFFSET_EDIT, m_wndX);
      AttachItem(IDC_Y_OFFSET_EDIT, m_wndY);
   }
   m_wndAspectRatio.SetRedraw(false);
   m_wndAspectRatio.AddString("Free");
   for (int j = 1; j < std::size(aspectRatios); j++)
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
   if (m_wndEmbeddable)
   {
      GetDlgItem(IDC_X_LABEL).ShowWindow(0);
      GetDlgItem(IDC_Y_LABEL).ShowWindow(0);
      m_wndX.ShowWindow(0);
      m_wndY.ShowWindow(0);
   }
   GetDlgItem(IDC_AR_LABEL).ShowWindow(fullscreen ? 0 : 1);
   m_wndAspectRatio.ShowWindow(fullscreen ? 0 : 1);
   GetDlgItem(IDC_WIDTH_LABEL).ShowWindow(fullscreen ? 0 : 1);
   m_wndWidth.ShowWindow(fullscreen ? 0 : 1);
   GetDlgItem(IDC_HEIGHT_LABEL).ShowWindow(fullscreen ? 0 : 1);
   m_wndHeight.ShowWindow(fullscreen ? 0 : 1);
   GetDlgItem(IDC_RESET_WINDOW).ShowWindow(fullscreen ? 0 : 1);

   int screenwidth = -1, screenheight = -1;
   vector<VPX::Window::DisplayConfig> displays;
   VPX::Window::GetDisplays(displays);
   for (const VPX::Window::DisplayConfig& displayConf : displays)
   {
      if ((display == -1 && displayConf.isPrimary) || display == displayConf.display)
      {
         display = displayConf.display;
         screenwidth = displayConf.width;
         screenheight = displayConf.height;
         break;
      }
   }
   
   VPX::Window::GetDisplayModes((int)display, m_allVideoModes);

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
      for (int j = 1; j < std::size(aspectRatios); j++)
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
      if (m_allVideoModes[i].refreshrate == DEFAULT_PLAYER_FS_REFRESHRATE)
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

   #if defined(ENABLE_SDL_VIDEO)
      // Not supported so disabled
      m_wndForce10bit.EnableWindow(false);
   #endif

   m_wndDisplay.SetRedraw(false);
   m_wndDisplay.ResetContent();
   m_wndDisplay.SetItemData((int)m_allVideoModes.size(), (DWORD)(m_allVideoModes.size() * 128));
   int embedded = 2;
   if (m_wndEmbeddable)
   {
      embedded = settings.LoadValueWithDefault(m_wndSection, m_wndSettingPrefix + "Output", VPX::RenderOutput::OM_DISABLED);
      m_wndDisplay.AddString("Disabled");
      m_wndDisplay.AddString("Embedded");
      if (embedded < 2)
         m_wndDisplay.SetCurSel(embedded);
   }
   vector<VPX::Window::DisplayConfig> displays;
   VPX::Window::GetDisplays(displays);
   int display;
   if (!settings.LoadValue(m_wndSection, m_wndSettingPrefix + "Display", display) || (display >= (int)displays.size()))
      display = -1;
   for (vector<VPX::Window::DisplayConfig>::iterator dispConf = displays.begin(); dispConf != displays.end(); ++dispConf)
   {
      if (display == -1 && dispConf->isPrimary)
         display = dispConf->display;
      const string displayName = "Display " + std::to_string(dispConf->display + 1) + (dispConf->isPrimary ? "*: " : " : ") + std::to_string(dispConf->width) + 'x' + std::to_string(dispConf->height) + ' ' + dispConf->GPU_Name;
      m_wndDisplay.AddString(displayName.c_str());
   }
   if (embedded == 2)
      m_wndDisplay.SetCurSel(m_wndEmbeddable ? (display + 2) : display);
   m_wndDisplay.SetRedraw(true);

   const bool fullscreen = settings.LoadValueWithDefault(m_wndSection, m_wndSettingPrefix + "FullScreen", IsWindows10_1803orAbove());
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
      settings.SaveValue(m_wndSection, m_wndSettingPrefix + "Display", display, !saveAll);

   if (embedded)
   {
      int x = GetDlgItemInt(IDC_X_OFFSET_EDIT, TRUE);
      int y = GetDlgItemInt(IDC_Y_OFFSET_EDIT, TRUE);
      int width = GetDlgItemInt(IDC_WIDTH_EDIT, TRUE);
      int height = GetDlgItemInt(IDC_HEIGHT_EDIT, TRUE);
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
         const int width  = GetDlgItemInt(IDC_WIDTH_EDIT, TRUE);
               int height = GetDlgItemInt(IDC_HEIGHT_EDIT, TRUE);
         if (arMode > 0)
            height = (int)(width * (double)aspectRatios[arMode].y / (double)aspectRatios[arMode].x);
         if (!saveAll)
         {
            settings.DeleteValue(m_wndSection, m_wndSettingPrefix + "AspectRatio");
            settings.DeleteValue(m_wndSection, m_wndSettingPrefix + "Width");
            settings.DeleteValue(m_wndSection, m_wndSettingPrefix + "Height");
            settings.DeleteValue(m_wndSection, m_wndSettingPrefix + "ColorDepth");
            settings.DeleteValue(m_wndSection, m_wndSettingPrefix + "RefreshRate");
         }
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
   for (int j = 1; j < std::size(aspectRatios); j++)
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
      if (HIWORD(wParam) == EN_CHANGE)
         PropChanged();
      break;
   case IDC_RESET_WINDOW:
      PropChanged();
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

// factor is applied to width and to height, so 2.0f increases pixel count by 4. Additional values can be added.
constexpr float AAfactors[] = { 0.5f, 0.75f, 1.0f, 1.25f, (float)(4.0 / 3.0), 1.5f, 1.75f, 2.0f };
constexpr LPCSTR AAfactorNames[] = { "50%", "75%", "Disabled", "125%", "133%", "150%", "175%", "200%" };
constexpr int AAfactorCount = 8;

constexpr int MSAASamplesOpts[] = { 1, 4, 6, 8 };
constexpr LPCSTR MSAASampleNames[] = { "Disabled", "4 Samples", "6 Samples", "8 Samples" };
constexpr int MSAASampleCount = 4;


static int getBestMatchingAAfactorIndex(float f)
{
   float delta = fabsf(f - AAfactors[0]);
   int bestMatch = 0;
   for (int i = 1; i < AAfactorCount; ++i)
      if (fabsf(f - AAfactors[i]) < delta)
      {
         delta = fabsf(f - AAfactors[i]);
         bestMatch = i;
      }
   return bestMatch;
}

#if defined(ENABLE_BGFX)
   const string RenderOptPage::m_bgfxRendererNames[bgfx::RendererType::Count + 1] = {"Noop"s, "Agc"s, "Direct3D11"s, "Direct3D12"s, "Gnm"s, "Metal"s, "Nvn"s, "OpenGLES"s, "OpenGL"s, "Vulkan"s, "Default"s };
#endif

BOOL RenderOptPage::OnInitDialog()
{
   VideoOptionPropPage::OnInitDialog();

   AttachItem(IDC_VIDEO_SYNC_MODE, m_syncMode);
   #if defined(ENABLE_BGFX)
      AddToolTip(m_syncMode , "None: Use this if your display supports variable refresh rate or if you are experiencing stutters.\r\n\r\nVertical Sync: Synchronize on display sync.");
   #else
      AddToolTip(m_syncMode, "None: No synchronization.\r\nVertical Sync: Synchronize on video sync which avoids video tearing, but has higher input latency.\r\nAdaptive Sync: Synchronize on video sync, "
                 "except for late frames (below target FPS), also has higher input latency.\r\nFrame Pacing: Targets real time simulation with low input- and video-latency (also dynamically adjusts framerate).");
   #endif
   AttachItem(IDC_MAX_FPS, m_maxFPS);
   AddToolTip(m_maxFPS , "Limit the FPS to the given value (energy saving/less heat, framerate stability), 0 will disable it");
   AttachItem(IDC_MAX_PRE_FRAMES, m_maxFrameLatency);
   AddToolTip(m_maxFrameLatency, "Leave at 0 if you have enabled 'Low Latency' or 'Anti Lag' settings in the graphics driver.\r\nOtherwise experiment with 1 or 2 for a chance of lag reduction at the price of a bit of framerate.");
   #if defined(ENABLE_BGFX)
      SetupCombo(m_syncMode, 2, "No Sync", "Vertical Sync");
   #else
      SetupCombo(m_syncMode, 4, "No Sync", "Vertical Sync", "Adaptive Sync", "Frame Pacing");
   #endif

   AttachItem(IDC_3D_STEREO, m_stereoMode);
   AddToolTip(m_stereoMode, "Activate this to enable 3D Stereo output using the requested format.\r\nSwitch on/off during play with the F10 key.\r\nThis requires that your TV can display 3D Stereo, and respective 3D glasses.");
   AttachItem(IDC_FAKE_STEREO, m_stereoFake);
   AttachItem(IDC_3D_STEREO_ANAGLYPH_FILTER, m_stereoFilter);
   AttachItem(IDC_3D_STEREO_BRIGHTNESS, m_stereoBrightness);
   AttachItem(IDC_3D_STEREO_DESATURATION, m_stereoSaturation);
   AttachItem(IDC_3D_STEREO_ES, m_stereoEyeSeparation);
   AttachItem(IDC_3D_STEREO_MS, m_stereoMaxSeparation);
   AttachItem(IDC_3D_STEREO_ZPD, m_stereoZPD);
   AttachItem(IDC_3D_STEREO_OFS, m_stereoOffset);
   AttachItem(IDC_3D_STEREO_Y, m_stereoYAxis);
   AddToolTip(m_stereoYAxis, "Switches 3D Stereo effect to use the Y Axis.\r\nThis should usually be selected for Cabinets/rotated displays.");
   SetupCombo(m_stereoFilter, 4, "None", "Dubois", "Luminance", "Deghost");

   AttachItem(IDC_MAX_AO_COMBO, m_maxAO);
   AddToolTip(m_maxAO, "Limit the quality of ambient occlusion for better performance.\r\nDynamic is the better with contact shadows for dynamic objects but higher performance requirements.");
   AttachItem(IDC_MAX_REFLECTION_COMBO, m_maxReflection);
   AddToolTip(m_maxReflection, "Limit the quality of reflections for better performance.\r\n\r\n'Dynamic' is recommended and will give the best results, but may harm performance.\r\n\r\n'Static Only' has no performance cost (except for VR rendering).\r\n\r\nOther options feature different trade-offs between quality and performance.");
   AttachItem(IDC_MAX_TEXTURE_COMBO, m_maxTexSize);
   AddToolTip(m_maxTexSize, "Saves memory on your graphics card but harms quality of the textures.");
   AttachItem(IDC_TEX_COMPRESS, m_compressTexture);
   AttachItem(IDC_FORCE_ANISO, m_forceAnisoMax);
   AddToolTip(m_forceAnisoMax, "Activate this to enhance the texture filtering.\r\nThis slows down performance only a bit (on most systems), but increases quality tremendously.");
   AttachItem(IDC_TM_OFF, m_forceTMOff);
   AddToolTip(m_forceTMOff, "Forces tonemapping to be always off on a HDR capable display. This is usually the right thing to do as tonemappers will artifically limit the displayed color and brightness range.");
   AttachItem(IDC_TM_HDR_SCALE, m_forceTMscale);
   AddToolTip(m_forceTMscale, "Global exposure scale multiplier for HDR capable displays.");
   AttachItem(IDC_BLOOM_OFF, m_forceBloomOff);
   AddToolTip(m_forceBloomOff, "Forces the bloom filter to be always off. Only for very low-end graphics cards.");
   AttachItem(IDC_DISABLE_MOTION_BLUR, m_forceMotionBlurOff);
   AddToolTip(m_forceMotionBlurOff, "Disable ball motion blur. Only for low-end graphics cards.");
   AttachItem(IDC_USE_NVIDIA_API_CHECK, m_useAltDepth);
   AddToolTip(m_useAltDepth, "Activate this if you get the corresponding error message on table start, or if you experience rendering problems.");
   AttachItem(IDC_SOFTWARE_VP, m_softwareVertex);
   AddToolTip(m_softwareVertex, "Activate this if you have issues using an Intel graphics chip.");
   AttachItem(IDC_DISABLE_DWM, m_disableDWM);
   AddToolTip(m_disableDWM, "Disable Windows Desktop Composition (only works on Windows Vista and Windows 7 systems).\r\nMay reduce lag and improve performance on some setups.");
   AttachItem(IDC_ARASlider, m_rampDetail);
   SetupCombo(m_maxAO, 3, "Disable AO", "Static AO", "Dynamic AO");
   SetupCombo(m_maxReflection, 6, "Disable Reflections", "Balls Only", "Static Only", "Static & Balls", "Static & Unsynced Dynamic", "Dynamic");
   SetupCombo(m_maxTexSize, 8, "1024", "2048", "3172", "4096", "5120", "6144", "7168", "Unlimited");
   m_rampDetail.SetRangeMax(10);
   m_rampDetail.SetRangeMin(0);
   m_rampDetail.SetTicFreq(1);
   m_rampDetail.SetLineSize(1);
   m_rampDetail.SetPageSize(1);
   SendMessage(m_rampDetail.GetHwnd(), TBM_SETTHUMBLENGTH, 10, 0);

   AttachItem(IDC_GLOBAL_TRAIL_CHECK, m_ballTrails);
   AttachItem(IDC_BALL_TRAIL_STRENGTH, m_ballTrailStrength);
   AttachItem(IDC_BALL_ANTISTRETCH, m_ballForceRound);
   AttachItem(IDC_GLOBAL_DISABLE_LIGHTING_BALLS, m_ballDisableLighting);
   AddToolTip(m_ballDisableLighting, "Disable lighting and reflection effects on balls, e.g. to help the visually handicapped.");
   AttachItem(IDC_OVERWRITE_BALL_IMAGE_CHECK, m_ballOverrideImages);
   AddToolTip(m_ballOverrideImages, "When checked, it overwrites the ball image/decal image(s) for every table.");
   AttachItem(IDC_BALL_IMAGE_EDIT, m_ballImage);
   AttachItem(IDC_BALL_DECAL_EDIT, m_ballDecal);

   AttachItem(IDC_MSAA_COMBO, m_msaaSamples);
   AddToolTip(m_msaaSamples, "Set the amount of MSAA samples.\r\n\r\nMSAA can help reduce geometry aliasing at the cost of performance and GPU memory.\r\n\r\nThis can improve image quality if not using supersampling.");
   AttachItem(IDC_SUPER_SAMPLING_COMBO, m_supersampling);
   AddToolTip(m_supersampling, "Enables brute-force Up/Downsampling (similar to DSR).\r\n\r\nThis delivers very good quality but has a significant impact on performance.\r\n\r\n200% means twice the resolution to be handled while rendering.");
   AttachItem(IDC_POST_PROCESS_COMBO, m_postprocAA);
   AddToolTip(m_postprocAA, "Enables post-processed Anti-Aliasing.\r\nThis delivers smoother images, at the cost of slight blurring.\r\n'Quality FXAA' and 'Quality SMAA' are recommended and lead to less artifacts,\nbut will harm performance on low-end graphics cards.");
   AttachItem(IDC_SHARPEN_COMBO, m_sharpen);
   AddToolTip(m_sharpen, "Enables post-processed sharpening of the image.\r\n'Bilateral CAS' is recommended,\nbut will harm performance on low-end graphics cards.\r\n'CAS' is less aggressive and faster, but also rather subtle.");
   m_msaaSamples.SetRedraw(false);
   for (size_t i = 0; i < MSAASampleCount; ++i)
      m_msaaSamples.AddString(MSAASampleNames[i]);
   m_msaaSamples.SetRedraw(true);
   m_supersampling.SetRedraw(false);
   for (size_t i = 0; i < AAfactorCount; ++i)
      m_supersampling.AddString(AAfactorNames[i]);
   m_supersampling.SetRedraw(true);
   SetupCombo(m_postprocAA, 7, "Disabled", "Fast FXAA", "Standard FXAA", "Quality FXAA", "Fast NFAA", "Standard DLAA", "Quality SMAA");
   SetupCombo(m_sharpen, 3, "Disabled", "CAS", "Bilateral CAS");
   #if defined(ENABLE_BGFX) || defined(ENABLE_DX9)
      // adapt layout for the hidden MSAA control
      GetDlgItem(IDC_MSAA_LABEL).ShowWindow(false);
      m_msaaSamples.ShowWindow(false);
      m_msaaSamples.EnableWindow(false);
      #define SHIFT_WND(id, amount)                              \
         {                                                       \
            CRect rc = GetDlgItem(id).GetClientRect();           \
            GetDlgItem(id).MapWindowPoints(this->GetHwnd(), rc); \
            rc.OffsetRect(0, amount);                            \
            GetDlgItem(id).MoveWindow(rc);                       \
         }
      SHIFT_WND(IDC_SUPER_SAMPLING_LABEL, -14)
      SHIFT_WND(IDC_SUPER_SAMPLING_COMBO, -14)
      SHIFT_WND(IDC_POST_PROCESS_AA_LABEL, -9)
      SHIFT_WND(IDC_POST_PROCESS_COMBO, -9)
      SHIFT_WND(IDC_SHARPEN_LABEL, -4)
      SHIFT_WND(IDC_SHARPEN_COMBO, -4)
      #undef SHIFT_WND
   #endif

   AttachItem(IDC_NUDGE_STRENGTH, m_visualNudge);
   AddToolTip(m_visualNudge, "Changes the visual effect/screen shaking when nudging the table.");
   AttachItem(IDC_GLOBAL_SSREFLECTION_CHECK, m_useAdditionalSSR);
   AddToolTip(m_useAdditionalSSR, "Enables additional post-processed reflections on all objects (besides the playfield).");

   AttachItem(IDC_OVERRIDE_DN, m_overrideNightDay);
   AttachItem(IDC_DAYNIGHT_SLIDER, m_nightDay);
   AttachItem(IDC_DYNAMIC_DN, m_autoNightDay);
   AddToolTip(m_autoNightDay ,"Activate this to switch the table brightness automatically based on your PC date,clock and location.\r\nThis requires to fill in geographic coordinates for your PCs location to work correctly.\r\nYou may use openstreetmap.org for example to get these in the correct format.");
   AttachItem(IDC_DN_LATITUDE, m_geoposLat);
   AddToolTip(m_geoposLat , "In decimal degrees (-90..90, North positive)");
   AttachItem(IDC_DN_LONGITUDE, m_geoposLon);
   AddToolTip(m_geoposLon , "In decimal degrees (-180..180, East positive)");
   m_nightDay.SetRangeMax(100);
   m_nightDay.SetRangeMin(2);
   m_nightDay.SetTicFreq(10);
   m_nightDay.SetLineSize(1);
   m_nightDay.SetPageSize(1);
   SendMessage(m_nightDay.GetHwnd(), TBM_SETTHUMBLENGTH, 10, 0);

   AttachItem(IDC_GFX_BACKEND, m_gfxBackend);
   #if defined(ENABLE_BGFX)
      bgfx::RendererType::Enum supportedRenderers[bgfx::RendererType::Count];
      int nRendererSupported = bgfx::getSupportedRenderers(bgfx::RendererType::Count, supportedRenderers);
      m_gfxBackend.SetRedraw(false);
      for (size_t i = 0; i < nRendererSupported; ++i)
         m_gfxBackend.AddString(m_bgfxRendererNames[supportedRenderers[i]].c_str());
      m_gfxBackend.SetRedraw(true);
   #else
      GetDlgItem(IDC_GFX_BACKEND_LABEL).ShowWindow(false);
      m_gfxBackend.ShowWindow(false);
   #endif

   #ifdef ENABLE_BGFX
      m_forceMotionBlurOff.EnableWindow(true);
   #else
      m_forceMotionBlurOff.EnableWindow(false);
   #endif

   LoadSettings(GetEditedSettings());

   return TRUE;
}

void RenderOptPage::LoadSettings(Settings& settings)
{
   char tmp[256];
   BeginLoad();
   
   { // Performance
      m_initialMaxTexDim = settings.LoadValueWithDefault(Settings::Player, "MaxTexDimension"s, 0);
      const int maxTexDim = ((1023 + m_initialMaxTexDim) / 1024) - 1;
      m_maxTexSize.SetCurSel(maxTexDim < 0 ? 7 : maxTexDim);
      
      const bool disableAO = settings.LoadValueWithDefault(Settings::Player, "DisableAO"s, false);
      const bool dynAO = settings.LoadValueWithDefault(Settings::Player, "DynamicAO"s, true);
      m_maxAO.SetCurSel(disableAO ? 0 : dynAO ? 2 : 1);
      
      int pfr = settings.LoadValueWithDefault(Settings::Player, "PFReflection"s, -1);
      RenderProbe::ReflectionMode maxReflection;
      if (pfr != -1)
         maxReflection = (RenderProbe::ReflectionMode)pfr;
      else
      {
         maxReflection = RenderProbe::REFL_STATIC;
         if (settings.LoadValueWithDefault(Settings::Player, "BallReflection"s, true))
            maxReflection = RenderProbe::REFL_STATIC_N_BALLS;
         if (settings.LoadValueWithDefault(Settings::Player, "PFRefl"s, true))
            maxReflection = RenderProbe::REFL_STATIC_N_DYNAMIC;
      }
      m_maxReflection.SetCurSel(maxReflection);

      m_useAltDepth.SetCheck(settings.LoadValueWithDefault(Settings::Player, "UseNVidiaAPI"s, false) ? BST_CHECKED : BST_UNCHECKED);
      m_forceTMOff.SetCheck(settings.LoadValueWithDefault(Settings::Player, "HDRDisableToneMapper"s, true) ? BST_CHECKED : BST_UNCHECKED);
#if defined(ENABLE_BGFX)
      //if (!SDL_GetBooleanProperty(props, SDL_PROP_DISPLAY_HDR_ENABLED_BOOLEAN, false)) {
      //   m_forceTMOff.EnableWindow(false);
      //   m_forceTMscale.EnableWindow(false);
      //}
#else
      m_forceTMOff.EnableWindow(false);
      m_forceTMscale.EnableWindow(false);
#endif
      const float forceTMscale = settings.LoadValueWithDefault(Settings::Player, "HDRGlobalExposure"s, 1.0f);
      sprintf_s(tmp, sizeof(tmp), "%.2f", forceTMscale);
      m_forceTMscale.SetWindowText(tmp);

      m_forceBloomOff.SetCheck(settings.LoadValueWithDefault(Settings::Player, "ForceBloomOff"s, false) ? BST_CHECKED : BST_UNCHECKED);
      m_forceAnisoMax.SetCheck(settings.LoadValueWithDefault(Settings::Player, "ForceAnisotropicFiltering"s, true) ? BST_CHECKED : BST_UNCHECKED);
      m_forceMotionBlurOff.SetCheck(settings.LoadValueWithDefault(Settings::Player, "ForceMotionBlurOff"s, false) ? BST_CHECKED : BST_UNCHECKED);
      m_compressTexture.SetCheck(settings.LoadValueWithDefault(Settings::Player, "CompressTextures"s, false) ? BST_CHECKED : BST_UNCHECKED);
      m_softwareVertex.SetCheck(settings.LoadValueWithDefault(Settings::Player, "SoftwareVertexProcessing"s, false) ? BST_CHECKED : BST_UNCHECKED);
      m_rampDetail.SetPos(settings.LoadValueWithDefault(Settings::Player, "AlphaRampAccuracy"s, 10),1);

      #if defined(ENABLE_SDL_VIDEO)
         m_disableDWM.EnableWindow(false); // Not supported so disabled
      #else
         m_disableDWM.EnableWindow(IsWindowsVistaOr7()); // DWM may not be disabled on Windows 8+
      #endif
      m_disableDWM.SetCheck(settings.LoadValueWithDefault(Settings::Player, "DisableDWM"s, false) ? BST_CHECKED : BST_UNCHECKED);
      
   }

   int maxFPS = settings.LoadValueWithDefault(Settings::Player, "MaxFramerate"s, -1);
   if(maxFPS > 0 && maxFPS <= 24) // at least 24 fps
      maxFPS = 24;
   VideoSyncMode syncMode = (VideoSyncMode)settings.LoadValueWithDefault(Settings::Player, "SyncMode"s, VSM_INVALID);
   if (maxFPS < 0 && syncMode == VideoSyncMode::VSM_INVALID)
   {
      const int vsync = settings.LoadValueWithDefault(Settings::Player, "AdaptiveVSync"s, -1);
      switch (vsync)
      {
      case -1: maxFPS = 0; syncMode = VideoSyncMode::VSM_FRAME_PACING; break;
      case 0: maxFPS = 0; syncMode = VideoSyncMode::VSM_NONE; break;
      case 1: maxFPS = 0; syncMode = VideoSyncMode::VSM_VSYNC; break;
      case 2: maxFPS = 0; syncMode = VideoSyncMode::VSM_ADAPTIVE_VSYNC; break;
      default: maxFPS = vsync; syncMode = VideoSyncMode::VSM_ADAPTIVE_VSYNC; break;
      }
   }
   if (maxFPS < 0)
      maxFPS = 0;
   SetDlgItemInt(IDC_MAX_FPS, maxFPS, TRUE);
   #if defined(ENABLE_BGFX)
   syncMode = (VideoSyncMode)clamp(syncMode, VSM_NONE, VSM_VSYNC);
   #else
   syncMode = (VideoSyncMode)clamp(syncMode, VSM_NONE, VSM_FRAME_PACING);
   #endif
   SendDlgItemMessage(IDC_VIDEO_SYNC_MODE, CB_SETCURSEL, syncMode, 0);
   const int maxPrerenderedFrames = settings.LoadValueWithDefault(Settings::Player, "MaxPrerenderedFrames"s, 0);
   SetDlgItemInt(IDC_MAX_PRE_FRAMES, maxPrerenderedFrames, TRUE);
   #if defined(ENABLE_OPENGL)
   m_maxFrameLatency.EnableWindow(false); // OpenGL does not support this option
   #endif

   const float AAfactor = settings.LoadValueWithDefault(Settings::Player, "AAFactor"s, settings.LoadValueWithDefault(Settings::Player, "USEAA"s, false) ? 1.5f : 1.0f);
   m_supersampling.SetCurSel(getBestMatchingAAfactorIndex(AAfactor));
   const int MSAASamples = settings.LoadValueWithDefault(Settings::Player, "MSAASamples"s, 1);
   const int CurrMSAAPos = static_cast<const int>(std::find(MSAASamplesOpts, MSAASamplesOpts + std::size(MSAASamplesOpts), MSAASamples) - MSAASamplesOpts);
   m_msaaSamples.SetCurSel(CurrMSAAPos);
   const int fxaa = settings.LoadValueWithDefault(Settings::Player, "FXAA"s, (int)Standard_FXAA);
   m_postprocAA.SetCurSel(fxaa);
   const int sharpen = settings.LoadValueWithDefault(Settings::Player, "Sharpen"s, 0);
   m_sharpen.SetCurSel(sharpen);

   const float nudgeStrength = settings.LoadValueWithDefault(Settings::Player, "NudgeStrength"s, 2e-2f);
   sprintf_s(tmp, sizeof(tmp), "%.3f", nudgeStrength);
   m_visualNudge.SetWindowText(tmp);
   const bool ssreflection = settings.LoadValueWithDefault(Settings::Player, "SSRefl"s, false);
   m_useAdditionalSSR.SetCheck(ssreflection ? BST_CHECKED : BST_UNCHECKED);

   const bool trail = settings.LoadValueWithDefault(Settings::Player, "BallTrail"s, true);
   m_ballTrails.SetCheck(trail ? BST_CHECKED : BST_UNCHECKED);
   const float trailStrength = settings.LoadValueWithDefault(Settings::Player, "BallTrailStrength"s, 0.5f);
   sprintf_s(tmp, sizeof(tmp), "%.3f", trailStrength);
   m_ballTrailStrength.SetWindowText(tmp);
   const bool ballAntiStretch = settings.LoadValueWithDefault(Settings::Player, "BallAntiStretch"s, false);
   m_ballForceRound.SetCheck(ballAntiStretch ? BST_CHECKED : BST_UNCHECKED);
   const bool disableLighting = settings.LoadValueWithDefault(Settings::Player, "DisableLightingForBalls"s, false);
   m_ballDisableLighting.SetCheck(disableLighting ? BST_CHECKED : BST_UNCHECKED);
   const bool overwiteBallImage = settings.LoadValueWithDefault(Settings::Player, "OverwriteBallImage"s, false);
   m_ballOverrideImages.SetCheck(overwiteBallImage ? BST_CHECKED : BST_UNCHECKED);
   string imageName;
   if (!settings.LoadValue(Settings::Player, "BallImage"s, imageName))
      imageName.clear();
   m_ballImage.SetWindowText(imageName.c_str());
   if (!settings.LoadValue(Settings::Player, "DecalImage"s, imageName))
      imageName.clear();
   m_ballDecal.SetWindowText(imageName.c_str());
   if (overwiteBallImage == 0)
   {
      GetDlgItem(IDC_BROWSE_BALL_IMAGE).EnableWindow(FALSE);
      GetDlgItem(IDC_BROWSE_BALL_DECAL).EnableWindow(FALSE);
      m_ballImage.EnableWindow(FALSE);
      m_ballDecal.EnableWindow(FALSE);
   }

   m_overrideNightDay.SetCheck(settings.LoadValueWithDefault(Settings::TableOverride, "OverrideEmissionScale"s, false) ? BST_CHECKED : BST_UNCHECKED);
   m_nightDay.SetPos((int)(100.f * settings.LoadValueWithDefault(Settings::Player, "EmissionScale"s, 0.5f)),1);
   m_autoNightDay.SetCheck(settings.LoadValueWithDefault(Settings::Player, "DynamicDayNight"s, false) ? BST_CHECKED : BST_UNCHECKED);
   const float lat = settings.LoadValueWithDefault(Settings::Player, "Latitude"s, 52.52f);
   sprintf_s(tmp, sizeof(tmp), "%.3f", lat);
   m_geoposLat.SetWindowText(tmp);
   const float lon = settings.LoadValueWithDefault(Settings::Player, "Longitude"s, 13.37f);
   sprintf_s(tmp, sizeof(tmp), "%.3f", lon);
   m_geoposLon.SetWindowText(tmp);
   OnCommand(IDC_OVERRIDE_DN, 0L); // Force UI update
   
   { // Stereo
      bool fakeStereo = true;
      #if defined(ENABLE_OPENGL) || defined(ENABLE_BGFX)
         fakeStereo = settings.LoadValueWithDefault(Settings::Player, "Stereo3DFake"s, false);
      #elif defined(ENABLE_DX9)
         m_stereoFake.EnableWindow(FALSE);
      #endif
         m_stereoFake.SetCheck(fakeStereo ? BST_CHECKED : BST_UNCHECKED);

      const int stereo3D = settings.LoadValueWithDefault(Settings::Player, "Stereo3D"s, 0);
      SetupCombo(m_stereoMode, 5, "Disabled", "Top / Bottom", "Interlaced (e.g. LG TVs)", "Flipped Interlaced (e.g. LG TVs)", "Side by Side");
      static const string defaultNames[] = { "Red/Cyan"s, "Green/Magenta"s, "Blue/Amber"s, "Cyan/Red"s, "Magenta/Green"s, "Amber/Blue"s, "Custom 1"s, "Custom 2"s, "Custom 3"s, "Custom 4"s };
      string name[std::size(defaultNames)];
      for (size_t i = 0; i < std::size(defaultNames); i++)
         if (!settings.LoadValue(Settings::Player, "Anaglyph"s.append(std::to_string(i + 1)).append("Name"s), name[i]))
            name[i] = defaultNames[i];
      m_stereoMode.SetRedraw(false);
      for (size_t i = 0; i < std::size(defaultNames); i++)
         m_stereoMode.AddString(name[i].c_str());
      m_stereoMode.SetCurSel(stereo3D);
      m_stereoMode.SetRedraw(true);

      m_stereoYAxis.SetCheck(settings.LoadValueWithDefault(Settings::Player, "Stereo3DYAxis"s, false) ? BST_CHECKED : BST_UNCHECKED);

      const float stereo3DOfs = settings.LoadValueWithDefault(Settings::Player, "Stereo3DOffset"s, 0.f);
      sprintf_s(tmp, sizeof(tmp), "%f", stereo3DOfs);
      m_stereoOffset.SetWindowText(tmp);

      const float stereo3DMS = settings.LoadValueWithDefault(Settings::Player, "Stereo3DMaxSeparation"s, 0.03f);
      sprintf_s(tmp, sizeof(tmp), "%f", stereo3DMS);
      m_stereoMaxSeparation.SetWindowText(tmp);

      const float stereo3DES = settings.LoadValueWithDefault(Settings::Player, "Stereo3DEyeSeparation"s, 63.0f);
      sprintf_s(tmp, sizeof(tmp), "%.1f", stereo3DES);
      m_stereoEyeSeparation.SetWindowText(tmp);

      const float stereo3DZPD = settings.LoadValueWithDefault(Settings::Player, "Stereo3DZPD"s, 0.5f);
      sprintf_s(tmp, sizeof(tmp), "%f", stereo3DZPD);
      m_stereoZPD.SetWindowText(tmp);

      const float stereo3DBrightness = settings.LoadValueWithDefault(Settings::Player, "Stereo3DBrightness"s, 1.0f);
      sprintf_s(tmp, sizeof(tmp), "%.2f", stereo3DBrightness);
      m_stereoBrightness.SetWindowText(tmp);

      const float stereo3DSaturation = settings.LoadValueWithDefault(Settings::Player, "Stereo3DSaturation"s, 1.0f);
      sprintf_s(tmp, sizeof(tmp), "%.2f", stereo3DSaturation);
      m_stereoSaturation.SetWindowText(tmp);
      
      OnCommand(IDC_FAKE_STEREO, 0L); // Force UI update
   }

   #if defined(ENABLE_BGFX)
   {
      string gfxBackend = settings.LoadValueWithDefault(Settings::Player, "GfxBackend"s, m_bgfxRendererNames[bgfx::RendererType::Vulkan]);
      bgfx::RendererType::Enum supportedRenderers[bgfx::RendererType::Count];
      int nRendererSupported = bgfx::getSupportedRenderers(bgfx::RendererType::Count, supportedRenderers);
      for (size_t i = 0; i < nRendererSupported; ++i)
         if (gfxBackend == m_bgfxRendererNames[supportedRenderers[i]])
            m_gfxBackend.SetCurSel(static_cast<int>(i));
   }
   #endif

   EndLoad();
}

void RenderOptPage::SaveSettings(Settings& settings, bool saveAll)
{
   BOOL nothing = 0;
   
   settings.SaveValue(Settings::Player, "MaxFramerate"s, (int)GetDlgItemInt(IDC_MAX_FPS, nothing, TRUE), !saveAll);
   int syncMode = m_syncMode.GetCurSel();
   settings.SaveValue(Settings::Player, "SyncMode"s, syncMode < 0 ? VideoSyncMode::VSM_FRAME_PACING : syncMode, !saveAll);
   settings.SaveValue(Settings::Player, "MaxPrerenderedFrames"s, (int)GetDlgItemInt(IDC_MAX_PRE_FRAMES, nothing, TRUE), !saveAll);

   int fxaa = m_postprocAA.GetCurSel();
   settings.SaveValue(Settings::Player, "FXAA"s, fxaa == -1 ? Standard_FXAA : fxaa, !saveAll);
   settings.SaveValue(Settings::Player, "Sharpen"s, max(0, m_sharpen.GetCurSel()), !saveAll);
   int AAfactorIndex = m_supersampling.GetCurSel();
   const float AAfactor = AAfactorIndex < 0 || AAfactorIndex >= AAfactorCount ? 1.f : AAfactors[AAfactorIndex];
   settings.SaveValue(Settings::Player, "USEAA"s, AAfactor > 1.0f, !saveAll);
   settings.SaveValue(Settings::Player, "AAFactor"s, AAfactor, !saveAll);
   int MSAASamplesIndex = max(0, m_msaaSamples.GetCurSel());
   const int MSAASamples = (MSAASamplesIndex < MSAASampleCount) ? MSAASamplesOpts[MSAASamplesIndex] : 1;
   settings.SaveValue(Settings::Player, "MSAASamples"s, MSAASamples, !saveAll);

   int maxAOMode = m_maxAO.GetCurSel();
   if (maxAOMode < 0)
      maxAOMode = 2;
   // Saving a table override but this needs to be evaluated from 2 options for backward compatibility reasons...
   const bool disableAO = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "DisableAO"s, false);
   const bool dynAO = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "DynamicAO"s, true);
   int appAO = disableAO ? 0 : dynAO ? 2 : 1;
   if (saveAll || appAO != maxAOMode)
   {
      settings.SaveValue(Settings::Player, "DisableAO"s, maxAOMode == 0);
      settings.SaveValue(Settings::Player, "DynamicAO"s, maxAOMode == 2);
   }
   else
   {
      settings.DeleteValue(Settings::Player, "DisableAO"s);
      settings.DeleteValue(Settings::Player, "DynamicAO"s);
   }
   int maxReflectionMode = m_maxReflection.GetCurSel();
   if (maxReflectionMode < 0)
      maxReflectionMode = RenderProbe::REFL_STATIC;
   settings.SaveValue(Settings::Player, "PFReflection"s, maxReflectionMode, !saveAll);
   LRESULT maxTexDim = SendDlgItemMessage(IDC_MAX_TEXTURE_COMBO, CB_GETCURSEL, 0, 0);
   if (maxTexDim == CB_ERR)
      maxTexDim = 7;
   maxTexDim = maxTexDim == 7 ? 0 : (1024 * (maxTexDim + 1));
   settings.SaveValue(Settings::Player, "MaxTexDimension"s, (int)maxTexDim, !saveAll);
   if (m_initialMaxTexDim != maxTexDim)
      MessageBox("You have changed the maximum texture size.\n\nThis change will only take effect after reloading the tables.", "Reload tables", MB_ICONWARNING);
   settings.SaveValue(Settings::Player, "ForceAnisotropicFiltering"s, m_forceAnisoMax.GetCheck() == BST_CHECKED, !saveAll);
   settings.SaveValue(Settings::Player, "CompressTextures"s, m_compressTexture.GetCheck() == BST_CHECKED, !saveAll);
   settings.SaveValue(Settings::Player, "SoftwareVertexProcessing"s, m_softwareVertex.GetCheck() == BST_CHECKED, !saveAll);
   settings.SaveValue(Settings::Player, "AlphaRampAccuracy"s, (int)SendDlgItemMessage(IDC_ARASlider, TBM_GETPOS, 0, 0), !saveAll);
   settings.SaveValue(Settings::Player, "UseNVidiaAPI"s, m_useAltDepth.GetCheck() == BST_CHECKED, !saveAll);
   settings.SaveValue(Settings::Player, "HDRDisableToneMapper"s, m_forceTMOff.GetCheck() == BST_CHECKED, !saveAll);
   settings.SaveValue(Settings::Player, "HDRGlobalExposure"s, GetDlgItemText(IDC_TM_HDR_SCALE).GetString(), !saveAll);
   settings.SaveValue(Settings::Player, "ForceBloomOff"s, m_forceBloomOff.GetCheck() == BST_CHECKED, !saveAll);
   settings.SaveValue(Settings::Player, "ForceMotionBlurOff"s, m_forceMotionBlurOff.GetCheck() == BST_CHECKED, !saveAll);
   settings.SaveValue(Settings::Player, "DisableDWM"s, m_disableDWM.GetCheck() == BST_CHECKED, !saveAll);
   settings.SaveValue(Settings::Player, "AlphaRampAccuracy"s, m_rampDetail.GetPos(), !saveAll);

   settings.SaveValue(Settings::Player, "NudgeStrength"s, GetDlgItemText(IDC_NUDGE_STRENGTH).GetString(), !saveAll);
   settings.SaveValue(Settings::Player, "SSRefl"s, m_useAdditionalSSR.GetCheck() == BST_CHECKED, !saveAll);

   int stereo3D = m_stereoMode.GetCurSel();
   if (stereo3D < 0)
      stereo3D = STEREO_OFF;
   settings.SaveValue(Settings::Player, "Stereo3D"s, (int)stereo3D, !saveAll);
   settings.SaveValue(Settings::Player, "Stereo3DEnabled"s, stereo3D != STEREO_OFF, !saveAll);
   settings.SaveValue(Settings::Player, "Stereo3DYAxis"s, m_stereoYAxis.GetCheck() == BST_CHECKED, !saveAll);
   settings.SaveValue(Settings::Player, "Stereo3DOffset"s, GetDlgItemText(IDC_3D_STEREO_OFS).GetString(), !saveAll);
   #if defined(ENABLE_OPENGL) || defined(ENABLE_BGFX)
      settings.SaveValue(Settings::Player, "Stereo3DFake"s, m_stereoFake.GetCheck() == BST_CHECKED, !saveAll);
   #endif
   settings.SaveValue(Settings::Player, "Stereo3DMaxSeparation"s, GetDlgItemText(IDC_3D_STEREO_MS).GetString(), !saveAll);
   settings.SaveValue(Settings::Player, "Stereo3DEyeSeparation"s, GetDlgItemText(IDC_3D_STEREO_ES).GetString(), !saveAll);
   settings.SaveValue(Settings::Player, "Stereo3DZPD"s, GetDlgItemText(IDC_3D_STEREO_ZPD).GetString(), !saveAll);
   settings.SaveValue(Settings::Player, "Stereo3DBrightness"s, GetDlgItemText(IDC_3D_STEREO_BRIGHTNESS).GetString(), !saveAll);
   settings.SaveValue(Settings::Player, "Stereo3DSaturation"s, GetDlgItemText(IDC_3D_STEREO_DESATURATION).GetString(), !saveAll);
   if (IsAnaglyphStereoMode(stereo3D))
   {
      int glassesIndex = stereo3D - STEREO_ANAGLYPH_1;
      int anaglyphFilter = m_stereoFilter.GetCurSel();
      if (anaglyphFilter < 0)
         anaglyphFilter = 4;
      settings.SaveValue(Settings::Player, "Anaglyph"s.append(std::to_string(glassesIndex + 1)).append("Filter"s), (int)anaglyphFilter, !saveAll);
   }

   settings.SaveValue(Settings::Player, "BallTrail"s, m_ballTrails.GetCheck() == BST_CHECKED, !saveAll);
   settings.SaveValue(Settings::Player, "BallTrailStrength"s, GetDlgItemText(IDC_BALL_TRAIL_STRENGTH).GetString(), !saveAll);
   settings.SaveValue(Settings::Player, "DisableLightingForBalls"s, m_ballDisableLighting.GetCheck() == BST_CHECKED, !saveAll);
   settings.SaveValue(Settings::Player, "BallAntiStretch"s, m_ballForceRound.GetCheck() == BST_CHECKED, !saveAll);
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

   settings.SaveValue(Settings::TableOverride, "OverrideEmissionScale"s, m_overrideNightDay.GetCheck() == BST_CHECKED, !saveAll);
   settings.SaveValue(Settings::Player, "EmissionScale"s, (float)SendDlgItemMessage(IDC_DAYNIGHT_SLIDER, TBM_GETPOS, 0, 0) / 100.f, !saveAll);
   settings.SaveValue(Settings::Player, "DynamicDayNight"s, m_autoNightDay.GetCheck() == BST_CHECKED, !saveAll);
   settings.SaveValue(Settings::Player, "Longitude"s, GetDlgItemText(IDC_DN_LONGITUDE).GetString(), !saveAll);
   settings.SaveValue(Settings::Player, "Latitude"s, GetDlgItemText(IDC_DN_LATITUDE).GetString(), !saveAll);

   #if defined(ENABLE_BGFX)
      const string gfxBackend = m_gfxBackend.GetWindowText().c_str();
      settings.SaveValue(Settings::Player, "GfxBackend"s, gfxBackend, !saveAll);
   #endif
}

BOOL RenderOptPage::OnApply()
{
   ApplyChanges();
   return TRUE;
}

void RenderOptPage::ResetVideoPreferences(int profile)
{
   PropChanged();
   if (profile == 0) // Set all defaults
   {
      m_geoposLat.SetWindowText("52.52");
      m_geoposLon.SetWindowText("13.37");
      m_autoNightDay.SetCheck(BST_UNCHECKED);

      m_visualNudge.SetWindowText("0.02");

      m_stereoMode.SetCurSel(0);
      m_stereoYAxis.SetCheck(BST_UNCHECKED);
      m_stereoOffset.SetWindowText("0");
      m_stereoEyeSeparation.SetWindowText("63");
      m_stereoMaxSeparation.SetWindowText("0.003");
      m_stereoZPD.SetWindowText("0.5");
      m_stereoBrightness.SetWindowText("1");
      m_stereoSaturation.SetWindowText("1");

      m_useAltDepth.SetCheck(BST_UNCHECKED);

      m_ballForceRound.SetCheck(BST_UNCHECKED);
      m_ballOverrideImages.SetCheck(BST_UNCHECKED);
      m_ballImage.SetWindowText("");
      m_ballDecal.SetWindowText("");
   }

   SetDlgItemInt(IDC_MAX_FPS, 0, TRUE);
   SetDlgItemInt(IDC_MAX_PRE_FRAMES, 0, TRUE);

   m_maxAO.SetCurSel(profile == 2 ? 2 : 1);
   m_maxReflection.SetCurSel(profile == 1 ? RenderProbe::REFL_STATIC_N_BALLS : RenderProbe::REFL_DYNAMIC);
   m_maxTexSize.SetCurSel(7);

   m_ballTrails.SetCheck(BST_UNCHECKED);
   m_ballDisableLighting.SetCheck(BST_UNCHECKED);

   m_supersampling.SetCurSel(getBestMatchingAAfactorIndex(1.0f));
   m_msaaSamples.SetCurSel(1);
   m_postprocAA.SetCurSel(profile == 1 ? Disabled : (profile == 2 ? Quality_FXAA : Standard_FXAA));
   m_sharpen.SetCurSel(profile != 2 ? 0 : 2);

   m_forceTMOff.SetCheck(BST_CHECKED);
   m_forceTMscale.SetWindowText("1.0");
   m_forceBloomOff.SetCheck(BST_UNCHECKED);
   m_forceMotionBlurOff.SetCheck(BST_UNCHECKED);
   m_forceAnisoMax.SetCheck(profile != 1 ? BST_CHECKED : BST_UNCHECKED);
   m_compressTexture.SetCheck(BST_UNCHECKED);
   m_softwareVertex.SetCheck(BST_UNCHECKED);

   SendDlgItemMessage(IDC_ARASlider, TBM_SETPOS, TRUE, profile == 1 ? 5 : 10);

   m_useAdditionalSSR.SetCheck(profile == 2 ? BST_CHECKED : BST_UNCHECKED);
}

BOOL RenderOptPage::OnCommand(WPARAM wParam, LPARAM lParam)
{
   switch (LOWORD(wParam))
   {
   case IDC_3D_STEREO_Y:
   case IDC_TEX_COMPRESS:
   case IDC_FORCE_ANISO:
   case IDC_TM_OFF:
   case IDC_BLOOM_OFF:
   case IDC_DISABLE_MOTION_BLUR:
   case IDC_USE_NVIDIA_API_CHECK:
   case IDC_SOFTWARE_VP:
   case IDC_GLOBAL_SSREFLECTION_CHECK:
   case IDC_GLOBAL_TRAIL_CHECK:
   case IDC_BALL_ANTISTRETCH:
   case IDC_GLOBAL_DISABLE_LIGHTING_BALLS:
      if (HIWORD(wParam) == BN_CLICKED)
         PropChanged();
      break;
   case IDC_3D_STEREO_BRIGHTNESS:
   case IDC_3D_STEREO_DESATURATION:
   case IDC_3D_STEREO_ES:
   case IDC_3D_STEREO_MS:
   case IDC_3D_STEREO_ZPD:
   case IDC_3D_STEREO_OFS:
   case IDC_MAX_FPS:
   case IDC_MAX_PRE_FRAMES:
   case IDC_TM_HDR_SCALE:
   case IDC_DN_LATITUDE:
   case IDC_DN_LONGITUDE:
   case IDC_NUDGE_STRENGTH:
   case IDC_BALL_TRAIL_STRENGTH:
   case IDC_BALL_IMAGE_EDIT:
   case IDC_BALL_DECAL_EDIT:
      if (HIWORD(wParam) == EN_CHANGE)
         PropChanged();
      break;
   case IDC_3D_STEREO_ANAGLYPH_FILTER:
   case IDC_VIDEO_SYNC_MODE:
   case IDC_MAX_AO_COMBO:
   case IDC_MAX_REFLECTION_COMBO:
   case IDC_MAX_TEXTURE_COMBO:
   case IDC_MSAA_COMBO:
   case IDC_SUPER_SAMPLING_COMBO:
   case IDC_POST_PROCESS_COMBO:
   case IDC_SHARPEN_COMBO:
   case IDC_GFX_BACKEND:
      if (HIWORD(wParam) == CBN_SELCHANGE)
         PropChanged();
      break;
   case IDC_DEFAULTS:
      ResetVideoPreferences(0);
      break;
   case IDC_DEFAULTS_LOW:
      ResetVideoPreferences(1);
      break;
   case IDC_DEFAULTS_HIGH:
      ResetVideoPreferences(2);
      break;
   case IDC_3D_STEREO:
   case IDC_FAKE_STEREO:
      if ((LOWORD(wParam) == IDC_3D_STEREO && HIWORD(wParam) == CBN_SELCHANGE)
       || (LOWORD(wParam) == IDC_FAKE_STEREO && HIWORD(wParam) == BN_CLICKED))
         PropChanged();
      {
         int stereo3D = m_stereoMode.GetCurSel();
         if (stereo3D < 0)
            stereo3D = STEREO_OFF;
         bool fakeStereo = true;
         #if defined(ENABLE_OPENGL) || defined(ENABLE_BGFX)
            fakeStereo = m_stereoFake.GetCheck() == BST_CHECKED;
         #endif
         SetDlgItemText(IDC_3D_STEREO_MS_LABEL, fakeStereo ? "Parallax Separation" : "Eye Separation (mm)");
         if (stereo3D == STEREO_OFF)
         {
            m_stereoFake.EnableWindow(false);
            m_stereoYAxis.EnableWindow(false);
            m_stereoOffset.EnableWindow(false);
            m_stereoZPD.EnableWindow(false);
            m_stereoMaxSeparation.EnableWindow(false);
            m_stereoEyeSeparation.EnableWindow(false);
            m_stereoBrightness.EnableWindow(false);
            m_stereoSaturation.EnableWindow(false);
            m_stereoFilter.EnableWindow(false);
         }
         else if (Is3DTVStereoMode(stereo3D))
         {
            #if defined(ENABLE_OPENGL) || defined(ENABLE_BGFX)
               m_stereoFake.EnableWindow(true);
            #endif
            m_stereoYAxis.EnableWindow(fakeStereo);
            m_stereoOffset.EnableWindow(fakeStereo);
            m_stereoZPD.EnableWindow(fakeStereo);
            m_stereoMaxSeparation.ShowWindow(fakeStereo);
            m_stereoEyeSeparation.ShowWindow(!fakeStereo);
            m_stereoMaxSeparation.EnableWindow(fakeStereo);
            m_stereoEyeSeparation.EnableWindow(!fakeStereo);
            m_stereoBrightness.EnableWindow(false);
            m_stereoSaturation.EnableWindow(false);
            m_stereoFilter.EnableWindow(false);
         }
         else if (IsAnaglyphStereoMode(stereo3D))
         {
            #if defined(ENABLE_OPENGL) || defined(ENABLE_BGFX)
               m_stereoFake.EnableWindow(true);
            #endif
            m_stereoYAxis.EnableWindow(fakeStereo);
            m_stereoOffset.EnableWindow(fakeStereo);
            m_stereoZPD.EnableWindow(fakeStereo);
            m_stereoMaxSeparation.ShowWindow(fakeStereo);
            m_stereoEyeSeparation.ShowWindow(!fakeStereo);
            m_stereoMaxSeparation.EnableWindow(fakeStereo);
            m_stereoEyeSeparation.EnableWindow(!fakeStereo);
            m_stereoBrightness.EnableWindow(true);
            m_stereoSaturation.EnableWindow(true);
            m_stereoFilter.EnableWindow(true);
            int glassesIndex = stereo3D - STEREO_ANAGLYPH_1;
            int anaglyphFilter = GetEditedSettings().LoadValueWithDefault(Settings::Player, "Anaglyph"s.append(std::to_string(glassesIndex + 1)).append("Filter"s), 4);
            m_stereoFilter.SetCurSel(anaglyphFilter);
         }
      }
      break;
   case IDC_DYNAMIC_DN:
   case IDC_OVERRIDE_DN:
      if (HIWORD(wParam) == BN_CLICKED)
         PropChanged();
      {
         const bool overrideDN = m_overrideNightDay.GetCheck() == BST_CHECKED;
         m_nightDay.EnableWindow(overrideDN);
         m_autoNightDay.EnableWindow(overrideDN);
         const bool dynamicDN = (m_overrideNightDay.GetCheck() == BST_CHECKED) && (m_autoNightDay.GetCheck() == BST_CHECKED);
         m_geoposLat.EnableWindow(dynamicDN);
         m_geoposLon.EnableWindow(dynamicDN);
      }
      break;
   case IDC_OVERWRITE_BALL_IMAGE_CHECK:
      if (HIWORD(wParam) == BN_CLICKED)
         PropChanged();
      {
         const BOOL overwriteEnabled = IsDlgButtonChecked(IDC_OVERWRITE_BALL_IMAGE_CHECK) == BST_CHECKED ? TRUE : FALSE;
         GetDlgItem(IDC_BROWSE_BALL_IMAGE).EnableWindow(overwriteEnabled);
         GetDlgItem(IDC_BROWSE_BALL_DECAL).EnableWindow(overwriteEnabled);
         m_ballImage.EnableWindow(overwriteEnabled);
         m_ballDecal.EnableWindow(overwriteEnabled);
      }
      break;
   case IDC_BROWSE_BALL_IMAGE:
      BrowseImage(m_ballImage);
      break;
   case IDC_BROWSE_BALL_DECAL:
      BrowseImage(m_ballDecal);
      break;
   default:
      return VideoOptionPropPage::OnCommand(wParam, lParam);
   }
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

   AttachItem(IDC_DISPLAY_ID, m_display);
   AttachItem(IDC_SCREEN_WIDTH, m_displayWidth);
   AddToolTip(m_displayWidth, "Physical width of the display area of the screen in centimeters, in landscape orientation (width > height).\r\n\r\nThis is needed to get the correct size when using 'Window' mode for the camera.");
   AttachItem(IDC_SCREEN_HEIGHT, m_displayHeight);
   AddToolTip(m_displayHeight, "Physical height of the display area of the screen in centimeters, in landscape orientation (width > height).\r\n\r\nThis is needed to get the correct size when using 'Window' mode for the camera.");
   AttachItem(IDC_SCREEN_INCLINATION, m_displayInclination);

   m_display.EnableWindow(false); // FIXME Not yet implemented: screen informations apply to playfield display only

   LoadSettings(GetEditedSettings());
   return TRUE;
}

void CabinetOptPage::LoadSettings(Settings& settings)
{
   BeginLoad();

   char tmp[256];
   const float screenPlayerX = settings.LoadValueWithDefault(Settings::Player, "ScreenPlayerX"s, 0.0f);
   sprintf_s(tmp, sizeof(tmp), "%.1f", screenPlayerX);
   m_playerX.SetWindowText(tmp);
   const float screenPlayerY = settings.LoadValueWithDefault(Settings::Player, "ScreenPlayerY"s, 0.0f);
   sprintf_s(tmp, sizeof(tmp), "%.1f", screenPlayerY);
   m_playerY.SetWindowText(tmp);
   const float screenPlayerZ = settings.LoadValueWithDefault(Settings::Player, "ScreenPlayerZ"s, 70.0f);
   sprintf_s(tmp, sizeof(tmp), "%.1f", screenPlayerZ);
   m_playerZ.SetWindowText(tmp);
   const bool bamHeadtracking = settings.LoadValueWithDefault(Settings::Player, "BAMHeadTracking"s, false);
   m_bamHeadtracking.SetCheck(bamHeadtracking ? BST_CHECKED : BST_UNCHECKED);

   const float screenWidth = settings.LoadValueWithDefault(Settings::Player, "ScreenWidth"s, 0.0f);
   sprintf_s(tmp, sizeof(tmp), "%.1f", screenWidth);
   m_displayWidth.SetWindowText(tmp);
   const float screenHeight = settings.LoadValueWithDefault(Settings::Player, "ScreenHeight"s, 0.0f);
   sprintf_s(tmp, sizeof(tmp), "%.1f", screenHeight);
   m_displayHeight.SetWindowText(tmp);
   const float screenInclination = settings.LoadValueWithDefault(Settings::Player, "ScreenInclination"s, 0.0f);
   sprintf_s(tmp, sizeof(tmp), "%.3f", screenInclination);
   m_displayInclination.SetWindowText(tmp);

   EndLoad();
}

void CabinetOptPage::SaveSettings(Settings& settings, bool saveAll)
{
   settings.SaveValue(Settings::Player, "ScreenPlayerX"s, GetDlgItemText(IDC_SCREEN_PLAYERX).GetString(), !saveAll);
   settings.SaveValue(Settings::Player, "ScreenPlayerY"s, GetDlgItemText(IDC_SCREEN_PLAYERY).GetString(), !saveAll);
   settings.SaveValue(Settings::Player, "ScreenPlayerZ"s, GetDlgItemText(IDC_SCREEN_PLAYERZ).GetString(), !saveAll);
   settings.SaveValue(Settings::Player, "BAMheadTracking"s, m_bamHeadtracking.GetCheck() == BST_CHECKED, !saveAll);

   settings.SaveValue(Settings::Player, "ScreenWidth"s, GetDlgItemText(IDC_SCREEN_WIDTH).GetString(), !saveAll);
   settings.SaveValue(Settings::Player, "ScreenHeight"s, GetDlgItemText(IDC_SCREEN_HEIGHT).GetString(), !saveAll);
   settings.SaveValue(Settings::Player, "ScreenInclination"s, GetDlgItemText(IDC_SCREEN_INCLINATION).GetString(), !saveAll);
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
   char tmp[256];
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
      sprintf_s(tmp, sizeof(tmp), "%.3f", settings.LoadValueFloat(Settings::DMD, prefix + "DotSize"));
      m_dotSize.SetWindowText(tmp);
      sprintf_s(tmp, sizeof(tmp), "%.3f", settings.LoadValueFloat(Settings::DMD, prefix + "DotBrightness"));
      m_dotBrightness.SetWindowText(tmp);
      sprintf_s(tmp, sizeof(tmp), "%.3f", settings.LoadValueFloat(Settings::DMD, prefix + "DotSharpness"));
      m_dotSharpness.SetWindowText(tmp);
      sprintf_s(tmp, sizeof(tmp), "%.3f", settings.LoadValueFloat(Settings::DMD, prefix + "DiffuseGlow"));
      m_diffuseGlow.SetWindowText(tmp);
   }
   else if (n < 7 + 8) // Alpha Seg
   {
      mode = 1;
      const string prefix = "Profile." + std::to_string(n - 7 + 1) + '.';
      m_dotTint.SetColor(settings.LoadValueUInt(Settings::Alpha, prefix + "Color"));
      m_unlitDotColor.SetColor(settings.LoadValueUInt(Settings::Alpha, prefix + "Unlit"));
      sprintf_s(tmp, sizeof(tmp), "%.3f", settings.LoadValueFloat(Settings::Alpha, prefix + "Brightness"));
      m_dotBrightness.SetWindowText(tmp);
      sprintf_s(tmp, sizeof(tmp), "%.3f", settings.LoadValueFloat(Settings::Alpha, prefix + "DiffuseGlow"));
      m_diffuseGlow.SetWindowText(tmp);
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
      settings.SaveValue(Settings::DMD, prefix + "DotSize", m_dotSize.GetWindowText().GetString(), !saveAll);
      settings.SaveValue(Settings::DMD, prefix + "DotBrightness", m_dotBrightness.GetWindowText().GetString(), !saveAll);
      settings.SaveValue(Settings::DMD, prefix + "DotSharpness", m_dotSharpness.GetWindowText().GetString(), !saveAll);
      settings.SaveValue(Settings::DMD, prefix + "DiffuseGlow", m_diffuseGlow.GetWindowText().GetString(), !saveAll);
   }
   else if (m_editedProfile < 7 + 8) // AlphaSeg
   {
      const string prefix = "Profile." + std::to_string(m_editedProfile - 7 + 1) + '.';
      settings.SaveValue(Settings::Alpha, prefix + "Color", static_cast<unsigned int>(m_dotTint.GetColor()), !saveAll);
      settings.SaveValue(Settings::Alpha, prefix + "Unlit", static_cast<unsigned int>(m_unlitDotColor.GetColor()), !saveAll);
      settings.SaveValue(Settings::Alpha, prefix + "Brightness", m_dotBrightness.GetWindowText().GetString(), !saveAll);
      settings.SaveValue(Settings::Alpha, prefix + "DiffuseGlow", m_diffuseGlow.GetWindowText().GetString(), !saveAll);
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
   InitDisplayControls(Settings::DMD, "DMD"s, true);
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
// Alpha segment View Options

#pragma region AlphaViewOptPage

AlphaViewOptPage::AlphaViewOptPage(Settings& appSettings, Settings& tableSettings)
   : VideoOptionPropPage(IDD_ALPHAVIEW_OPT, _T("Alpha View"), appSettings, tableSettings)
{
}

BOOL AlphaViewOptPage::OnInitDialog()
{
   VideoOptionPropPage::OnInitDialog();
   AttachItem(IDC_BG_SET, m_viewMode);
   m_viewMode.SetRedraw(false);
   m_viewMode.AddString("Disabled");
   m_viewMode.SetRedraw(true);
   InitDisplayControls(Settings::Alpha, "Alpha"s, true);
   LoadSettings(GetEditedSettings());
   return TRUE;
}

void AlphaViewOptPage::LoadSettings(Settings& settings)
{
   BeginLoad();
   m_viewMode.SetCurSel(settings.LoadValueWithDefault(Settings::Alpha, "ViewMode"s, 0));
   LoadDisplaySettings();
   EndLoad();
}

void AlphaViewOptPage::SaveSettings(Settings& settings, bool saveAll)
{
   settings.SaveValue(Settings::Alpha, "ViewMode"s, max(m_viewMode.GetCurSel(), 0), !saveAll);
   SaveDisplaySettings();
}

BOOL AlphaViewOptPage::OnApply()
{
   ApplyChanges();
   return TRUE;
}

BOOL AlphaViewOptPage::OnCommand(WPARAM wParam, LPARAM lParam)
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
// Backglass View Options

#pragma region BackglassViewOptPage

BackglassViewOptPage::BackglassViewOptPage(Settings& appSettings, Settings& tableSettings)
   : VideoOptionPropPage(IDD_BACKGLASS_OPT, _T("Backglass View"), appSettings, tableSettings)
{
}

BOOL BackglassViewOptPage::OnInitDialog()
{
   VideoOptionPropPage::OnInitDialog();
   AttachItem(IDC_BG_SET, m_viewMode);
   m_viewMode.SetRedraw(false);
   m_viewMode.AddString("Disabled");
   m_viewMode.SetRedraw(true);
   InitDisplayControls(Settings::Backglass, "Backglass"s, true);
   LoadSettings(GetEditedSettings());
   return TRUE;
}

void BackglassViewOptPage::LoadSettings(Settings& settings)
{
   BeginLoad();
   m_viewMode.SetCurSel(settings.LoadValueWithDefault(Settings::Backglass, "ViewMode"s, 0));
   LoadDisplaySettings();
   EndLoad();
}

void BackglassViewOptPage::SaveSettings(Settings& settings, bool saveAll)
{
   settings.SaveValue(Settings::Backglass, "ViewMode"s, max(m_viewMode.GetCurSel(), 0), !saveAll);
   SaveDisplaySettings();
}

BOOL BackglassViewOptPage::OnApply()
{
   ApplyChanges();
   return TRUE;
}

BOOL BackglassViewOptPage::OnCommand(WPARAM wParam, LPARAM lParam)
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
