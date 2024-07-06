#include "core/stdafx.h"
#include "ui/resource.h"
#include "VideoOptionsDialog.h"


// Declared in RenderDevice. Desktop composition may only be disabled on Windows Vista & 7
bool IsWindowsVistaOr7();

// factor is applied to width and to height, so 2.0f increases pixel count by 4. Additional values can be added.
constexpr float AAfactors[] = { 0.5f, 0.75f, 1.0f, 1.25f, (float)(4.0/3.0), 1.5f, 1.75f, 2.0f };
constexpr LPCSTR AAfactorNames[] = { "50%", "75%", "Disabled", "125%", "133%", "150%", "175%", "200%" };
constexpr int AAfactorCount = 8;

constexpr int MSAASamplesOpts[] = { 1, 4, 6, 8 };
constexpr LPCSTR MSAASampleNames[] = { "Disabled", "4 Samples", "6 Samples", "8 Samples" };
constexpr int MSAASampleCount = 4;

constexpr float arFactors[] = { 0.f, 4.f/3.f, 16.f/10.f, 16.f/9.f, 21.f/10.f, 21.f/9.f, 3.f/4.f, 10.f/16.f, 9.f/16.f, 10.f/21.f, 9.f/21.f };


static size_t getBestMatchingAAfactorIndex(float f)
{
   float delta = fabsf(f - AAfactors[0]);
   size_t bestMatch = 0;
   for (size_t i = 1; i < AAfactorCount; ++i)
      if (fabsf(f - AAfactors[i]) < delta) {
         delta = fabsf(f - AAfactors[i]);
         bestMatch = i;
      }
   return bestMatch;
}

VideoOptionsDialog::VideoOptionsDialog() : CDialog(IDD_VIDEO_OPTIONS)
{
}

void VideoOptionsDialog::AddToolTip(const char * const text, HWND parentHwnd, HWND toolTipHwnd, HWND controlHwnd)
{
   TOOLINFO toolInfo = { 0 };
   toolInfo.cbSize = sizeof(toolInfo);
   toolInfo.hwnd = parentHwnd;
   toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
   toolInfo.uId = (UINT_PTR)controlHwnd;
   toolInfo.lpszText = (char*)text;
   SendMessage(toolTipHwnd, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);
}

void VideoOptionsDialog::ResetVideoPreferences(const unsigned int profile) // 0 = default, 1 = lowend PC, 2 = highend PC
{
   if(profile == 0)
   {
      const bool fullscreen = IsWindows10_1803orAbove();
      SendDlgItemMessage(IDC_EXCLUSIVE_FULLSCREEN, BM_SETCHECK, fullscreen ? BST_CHECKED : BST_UNCHECKED, 0);
      SendDlgItemMessage(IDC_WINDOWED, BM_SETCHECK, fullscreen ? BST_UNCHECKED : BST_CHECKED, 0);
      OnCommand(IDC_EXCLUSIVE_FULLSCREEN, 0L); // Force UI update
   }
   if(profile < 2)
   {
      SendDlgItemMessage(IDC_10BIT_VIDEO, BM_SETCHECK, false ? BST_CHECKED : BST_UNCHECKED, 0);
   }

   SendDlgItemMessage(IDC_MAX_TEXTURE_COMBO, CB_SETCURSEL, 7, 0);
   SendDlgItemMessage(IDC_GLOBAL_TRAIL_CHECK, BM_SETCHECK, true ? BST_CHECKED : BST_UNCHECKED, 0);
   SendDlgItemMessage(IDC_GLOBAL_DISABLE_LIGHTING_BALLS, BM_SETCHECK, false ? BST_CHECKED : BST_UNCHECKED, 0);
   SetDlgItemInt(IDC_MAX_FPS, 0, FALSE);
   SetDlgItemInt(IDC_MAX_PRE_FRAMES, 0, FALSE);

   if(profile == 0)
   {
      char tmp[256];
      constexpr float latitude = 52.52f;
      sprintf_s(tmp, sizeof(tmp), "%f", latitude);
      SetDlgItemText(IDC_DN_LATITUDE, tmp);
      constexpr float longitude = 13.37f;
      sprintf_s(tmp, sizeof(tmp), "%f", longitude);
      SetDlgItemText(IDC_DN_LONGITUDE, tmp);
      constexpr float nudgeStrength = 2e-2f;
      sprintf_s(tmp, sizeof(tmp), "%f", nudgeStrength);
      SetDlgItemText(IDC_NUDGE_STRENGTH, tmp);
   }

   SendDlgItemMessage(IDC_SUPER_SAMPLING_COMBO, TBM_SETPOS, TRUE, getBestMatchingAAfactorIndex(1.0f));
   SendDlgItemMessage(IDC_MSAA_COMBO, TBM_SETPOS, TRUE, 1);

   SendDlgItemMessage(IDC_DYNAMIC_DN, BM_SETCHECK, false ? BST_CHECKED : BST_UNCHECKED, 0);
   SendDlgItemMessage(IDC_MAX_AO_COMBO, CB_SETCURSEL,profile == 2 ? 2 : 1, 0);

   SendDlgItemMessage(IDC_GLOBAL_SSREFLECTION_CHECK, BM_SETCHECK, profile == 2 ? BST_CHECKED : BST_UNCHECKED, 0);
   SendDlgItemMessage(IDC_MAX_REFLECTION_COMBO, CB_SETCURSEL, profile == 1 ? RenderProbe::REFL_STATIC_N_BALLS : RenderProbe::REFL_DYNAMIC, 0);

   if (profile == 0)
   {
      SendDlgItemMessage(IDC_OVERWRITE_BALL_IMAGE_CHECK, BM_SETCHECK, false ? BST_CHECKED : BST_UNCHECKED, 0);
      SetDlgItemText(IDC_BALL_IMAGE_EDIT, "");
      SetDlgItemText(IDC_BALL_DECAL_EDIT, "");
      if (true)
      {
         GetDlgItem(IDC_BROWSE_BALL_IMAGE).EnableWindow(FALSE);
         GetDlgItem(IDC_BROWSE_BALL_DECAL).EnableWindow(FALSE);
         GetDlgItem(IDC_BALL_IMAGE_EDIT).EnableWindow(FALSE);
         GetDlgItem(IDC_BALL_DECAL_EDIT).EnableWindow(FALSE);
      }
   }

   SendDlgItemMessage(IDC_POST_PROCESS_COMBO, CB_SETCURSEL, profile == 1 ? Disabled : (profile == 2 ? Quality_FXAA : Standard_FXAA), 0);
   SendDlgItemMessage(IDC_SHARPEN_COMBO, CB_SETCURSEL, profile != 2 ? 0 : 2, 0);
   SendDlgItemMessage(IDC_SCALE_FX_DMD, BM_SETCHECK, false ? BST_CHECKED : BST_UNCHECKED, 0);

   if (profile == 0)
   {
      SendDlgItemMessage(IDC_BG_SET, CB_SETCURSEL, 0, 0);
      SendDlgItemMessage(IDC_3D_STEREO, CB_SETCURSEL, 0, 0);
      SendDlgItemMessage(IDC_3D_STEREO_Y, BM_SETCHECK, false ? BST_CHECKED : BST_UNCHECKED, 0);
      constexpr float stereo3DOfs = 0.0f;
      char tmp[256];
      sprintf_s(tmp, sizeof(tmp), "%f", stereo3DOfs);
      SetDlgItemText(IDC_3D_STEREO_OFS, tmp);
      sprintf_s(tmp, sizeof(tmp), "%f", 63.0f);
      SetDlgItemText(IDC_3D_STEREO_ES, tmp);
      sprintf_s(tmp, sizeof(tmp), "%f", 0.03f);
      SetDlgItemText(IDC_3D_STEREO_MS, tmp);
      constexpr float stereo3DZPD = 0.5f;
      sprintf_s(tmp, sizeof(tmp), "%f", stereo3DZPD);
      SetDlgItemText(IDC_3D_STEREO_ZPD, tmp);
      constexpr float stereo3DBrightness = 1.0f;
      sprintf_s(tmp, sizeof(tmp), "%f", stereo3DBrightness);
      SetDlgItemText(IDC_3D_STEREO_BRIGHTNESS, tmp);
      constexpr float stereo3DSaturation = 1.0f;
      sprintf_s(tmp, sizeof(tmp), "%f", stereo3DSaturation);
      SetDlgItemText(IDC_3D_STEREO_DESATURATION, tmp);
      SendDlgItemMessage(IDC_USE_NVIDIA_API_CHECK, BM_SETCHECK, false ? BST_CHECKED : BST_UNCHECKED, 0);
   }

   SendDlgItemMessage(IDC_BLOOM_OFF, BM_SETCHECK, false ? BST_CHECKED : BST_UNCHECKED, 0);

   if (profile == 0)
   {
      SendDlgItemMessage(IDC_DISABLE_DWM, BM_SETCHECK, false ? BST_CHECKED : BST_UNCHECKED, 0);
   }

   SendDlgItemMessage(IDC_FORCE_ANISO, BM_SETCHECK, profile != 1 ? BST_CHECKED : BST_UNCHECKED, 0);
   SendDlgItemMessage(IDC_TEX_COMPRESS, BM_SETCHECK, false ? BST_CHECKED : BST_UNCHECKED, 0);
   SendDlgItemMessage(IDC_SOFTWARE_VP, BM_SETCHECK, false ? BST_CHECKED : BST_UNCHECKED, 0);
   SendDlgItemMessage(IDC_ARASlider, TBM_SETPOS, TRUE, profile == 1 ? 5 : 10);

   if (profile == 0)
   {
      SendDlgItemMessage(IDC_BALL_ANTISTRETCH, BM_SETCHECK, BST_UNCHECKED, 0);
   }
}

void VideoOptionsDialog::UpdateFullscreenModesList()
{
   const HWND hwndList = GetDlgItem(IDC_SIZELIST).GetHwnd();
   LRESULT display = (int)SendDlgItemMessage(IDC_DISPLAY_ID, CB_GETCURSEL, 0, 0);
   if (display == CB_ERR)
      display = 0;
   VPX::Window::GetDisplayModes((int) display, m_allVideoModes);
   int screenwidth, screenheight, x, y;
   vector<VPX::Window::DisplayConfig> displays;
   VPX::Window::GetDisplays(displays);
   for (vector<VPX::Window::DisplayConfig>::iterator displayConf = displays.begin(); displayConf != displays.end(); ++displayConf) {
      if ((display == -1 && displayConf->isPrimary) || display == displayConf->display) {
         x = displayConf->left;
         y = displayConf->top;
         screenwidth = displayConf->width;
         screenheight = displayConf->height;
         break;
      }
   }

   const int depthcur = GetEditedSettings().LoadValueWithDefault(Settings::Player, "ColorDepth"s, 32);
   const int refreshrate = GetEditedSettings().LoadValueWithDefault(Settings::Player, "RefreshRate"s, 0);
   const int widthcur = GetEditedSettings().LoadValueWithDefault(Settings::Player, "Width"s, -1);
   const int heightcur = GetEditedSettings().LoadValueWithDefault(Settings::Player, "Height"s, -1);
   VPX::Window::VideoMode curSelMode;
   curSelMode.width = widthcur;
   curSelMode.height = heightcur;
   curSelMode.depth = depthcur;
   curSelMode.refreshrate = refreshrate;

   SendMessage(hwndList, WM_SETREDRAW, FALSE, 0); // to speed up adding the entries :/
   SendMessage(hwndList, LB_RESETCONTENT, 0, 0);
   SendMessage(hwndList, LB_INITSTORAGE, m_allVideoModes.size(), m_allVideoModes.size() * 128); // *128 is artificial

   int bestMatch = 0; // to find closest matching res
   int bestMatchingPoints = 0; // dto.


   for (size_t i = 0; i < m_allVideoModes.size(); ++i)
   {
      double aspect = (double)m_allVideoModes[i].width / (double)m_allVideoModes[i].height;
      const bool portrait = (aspect < 1.);
      if (portrait)
         aspect = 1./aspect;
      double factor = aspect*3.0;
      int fx,fy;
      if (factor > 4.0)
      {
         factor = aspect*9.0;
         if ((int)(factor+0.5) == 16)
         {
            //16:9
            fx = 16;
            fy = 9;
         }
         else if ((int)(factor+0.5) == 21)
         {
            //21:9
            fx = 21;
            fy = 9;
         }
         else
         {
            factor = aspect*10.0;
            if ((int)(factor+0.5) == 16)
            {
               //16:10
               fx = 16;
               fy = 10;
            }
            else
            {
               //21:10
               fx = 21;
               fy = 10;
            }
         }
      }
      else
      {
         //4:3
         fx = 4;
         fy = 3;
      }

      char szT[128];
      if (m_allVideoModes[i].depth) // i.e. is this windowed or not
         sprintf_s(szT, sizeof(szT), "%d x %d (%dHz %d:%d)", m_allVideoModes[i].width, m_allVideoModes[i].height, /*m_allVideoModes[i].depth,*/ m_allVideoModes[i].refreshrate,
            portrait ? fy : fx, portrait ? fx : fy);
      else
         sprintf_s(szT, sizeof(szT), "%d x %d (%d:%d %s)", m_allVideoModes[i].width, m_allVideoModes[i].height /*,m_allVideoModes[i].depth*/, portrait ? fy : fx, portrait ? fx : fy,
            portrait ? "Portrait" : "Landscape");

      SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)szT);
      int matchingPoints = 0;
      if (m_allVideoModes[i].width == curSelMode.width) matchingPoints += 100;
      if (m_allVideoModes[i].height == curSelMode.height) matchingPoints += 100;
      if (m_allVideoModes[i].depth == curSelMode.depth) matchingPoints += 50;
      if (m_allVideoModes[i].refreshrate == curSelMode.refreshrate) matchingPoints += 10;
      if (m_allVideoModes[i].width == screenwidth) matchingPoints += 3;
      if (m_allVideoModes[i].height == screenheight) matchingPoints += 3;
      if (m_allVideoModes[i].refreshrate == DEFAULT_PLAYER_FS_REFRESHRATE) matchingPoints += 1;
      if (matchingPoints > bestMatchingPoints) {
         bestMatch = (int)i;
         bestMatchingPoints = matchingPoints;
      }
   }
   SendMessage(hwndList, LB_SETCURSEL, bestMatch, 0);
   if (SendMessage(hwndList, LB_GETCURSEL, 0, 0) == LB_ERR)
      SendMessage(hwndList, LB_SETCURSEL, 0, 0);
   SendMessage(hwndList, WM_SETREDRAW, TRUE, 0);
}

BOOL VideoOptionsDialog::OnInitDialog()
{
   // Do a copy of the edited settings
   m_editedSettings = nullptr;
   m_appSettings = g_pvp->m_settings;
   if (g_pvp->m_ptableActive)
   {
      m_tableSettings = g_pvp->m_ptableActive->m_settings;
      m_tableSettings.SetParent(&m_appSettings);
   }

   const HWND hwndDlg = GetHwnd();
   const HWND toolTipHwnd = CreateWindowEx(NULL, TOOLTIPS_CLASS, NULL, WS_POPUP | TTS_ALWAYSTIP | TTS_BALLOON, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hwndDlg, NULL, g_pvp->theInstance, NULL);
   if (toolTipHwnd)
   {
      SendMessage(toolTipHwnd, TTM_SETMAXTIPWIDTH, 0, 180);
      AddToolTip("Disable lighting and reflection effects on balls, e.g. to help the visually handicapped.", hwndDlg, toolTipHwnd, GetDlgItem(IDC_GLOBAL_DISABLE_LIGHTING_BALLS).GetHwnd());
      AddToolTip("Activate this if you get the corresponding error message on table start, or if you experience rendering problems.", hwndDlg, toolTipHwnd, GetDlgItem(IDC_USE_NVIDIA_API_CHECK).GetHwnd());
      AddToolTip("Forces the bloom filter to be always off. Only for very low-end graphics cards.", hwndDlg, toolTipHwnd, GetDlgItem(IDC_BLOOM_OFF).GetHwnd());
      AddToolTip("This saves memory on your graphics card but harms quality of the textures.", hwndDlg, toolTipHwnd, GetDlgItem(IDC_TEX_COMPRESS).GetHwnd());
      AddToolTip("Disable Windows Desktop Composition (only works on Windows Vista and Windows 7 systems).\r\nMay reduce lag and improve performance on some setups.", hwndDlg, toolTipHwnd, GetDlgItem(IDC_DISABLE_DWM).GetHwnd());
      AddToolTip("Activate this if you have issues using an Intel graphics chip.", hwndDlg, toolTipHwnd, GetDlgItem(IDC_SOFTWARE_VP).GetHwnd());
      #if defined(ENABLE_BGFX)
      AddToolTip("None: Use this if your display supports variable refresh rate or if you are experiencing stutters.\r\n\r\nVertical Sync: Synchronize on display sync.", hwndDlg, toolTipHwnd, GetDlgItem(IDC_VIDEO_SYNC_MODE).GetHwnd());
      #else
      AddToolTip("None: No synchronization.\r\nVertical Sync: Synchronize on video sync which avoids video tearing, but has higher input latency.\r\nAdaptive Sync: Synchronize on video sync, except for late frames (below target FPS), also has higher input latency.\r\nFrame Pacing: Targets real time simulation with low input- and video-latency (also dynamically adjusts framerate).", hwndDlg, toolTipHwnd, GetDlgItem(IDC_VIDEO_SYNC_MODE).GetHwnd());
      #endif
      AddToolTip("Limit the FPS to the given value (energy saving/less heat, framerate stability), 0 will disable it", hwndDlg, toolTipHwnd, GetDlgItem(IDC_MAX_FPS).GetHwnd());
      AddToolTip("Leave at 0 if you have enabled 'Low Latency' or 'Anti Lag' settings in the graphics driver.\r\nOtherwise experiment with 1 or 2 for a chance of lag reduction at the price of a bit of framerate.", hwndDlg, toolTipHwnd, GetDlgItem(IDC_MAX_PRE_FRAMES).GetHwnd());
      AddToolTip("Changes the visual effect/screen shaking when nudging the table.", hwndDlg, toolTipHwnd, GetDlgItem(IDC_NUDGE_STRENGTH).GetHwnd());
      AddToolTip("Activate this to switch the table brightness automatically based on your PC date,clock and location.\r\nThis requires to fill in geographic coordinates for your PCs location to work correctly.\r\nYou may use openstreetmap.org for example to get these in the correct format.", hwndDlg, toolTipHwnd, GetDlgItem(IDC_DYNAMIC_DN).GetHwnd());
      AddToolTip("In decimal degrees (-90..90, North positive)", hwndDlg, toolTipHwnd, GetDlgItem(IDC_DN_LATITUDE).GetHwnd());
      AddToolTip("In decimal degrees (-180..180, East positive)", hwndDlg, toolTipHwnd, GetDlgItem(IDC_DN_LONGITUDE).GetHwnd());
      
      AddToolTip("Activate this to enable 3D Stereo output using the requested format.\r\nSwitch on/off during play with the F10 key.\r\nThis requires that your TV can display 3D Stereo, and respective 3D glasses.", hwndDlg, toolTipHwnd, GetDlgItem(IDC_3D_STEREO).GetHwnd());
      AddToolTip("Switches 3D Stereo effect to use the Y Axis.\r\nThis should usually be selected for Cabinets/rotated displays.", hwndDlg, toolTipHwnd, GetDlgItem(IDC_3D_STEREO_Y).GetHwnd());
      if (IsWindows10_1803orAbove())
         AddToolTip("Enforces exclusive Fullscreen Mode.\r\nEnforcing exclusive FS can slightly reduce input lag.", hwndDlg, toolTipHwnd, GetDlgItem(IDC_EXCLUSIVE_FULLSCREEN).GetHwnd());
      else
         AddToolTip("Enforces exclusive Fullscreen Mode.\r\nDo not enable if you require to see the VPinMAME or B2S windows for example.\r\nEnforcing exclusive FS can slightly reduce input lag though.", hwndDlg, toolTipHwnd, GetDlgItem(IDC_EXCLUSIVE_FULLSCREEN).GetHwnd());
      AddToolTip("Enforces 10Bit (WCG) rendering.\r\nRequires a corresponding 10Bit output capable graphics card and monitor.\r\nAlso requires to have exclusive fullscreen mode enforced (for now).", hwndDlg, toolTipHwnd, GetDlgItem(IDC_10BIT_VIDEO).GetHwnd());
      AddToolTip("Defines the view mode used when running a table\n\nDesktop/FSS will use the FSS view for table with FSS enabled, desktop otherwise.\n\nCabinet uses the 'fullscreen' view\n\nDesktop always uses the desktop view (no FSS)", hwndDlg, toolTipHwnd, GetDlgItem(IDC_BG_SET).GetHwnd());
      AddToolTip("Enables post-processed Anti-Aliasing.\r\nThis delivers smoother images, at the cost of slight blurring.\r\n'Quality FXAA' and 'Quality SMAA' are recommended and lead to less artifacts,\nbut will harm performance on low-end graphics cards.", hwndDlg, toolTipHwnd, GetDlgItem(IDC_POST_PROCESS_COMBO).GetHwnd());
      AddToolTip("Enables post-processed sharpening of the image.\r\n'Bilateral CAS' is recommended,\nbut will harm performance on low-end graphics cards.\r\n'CAS' is less aggressive and faster, but also rather subtle.", hwndDlg, toolTipHwnd, GetDlgItem(IDC_SHARPEN_COMBO).GetHwnd());
      AddToolTip("Enables brute-force Up/Downsampling (similar to DSR).\r\n\r\nThis delivers very good quality but has a significant impact on performance.\r\n\r\n200% means twice the resolution to be handled while rendering.", hwndDlg, toolTipHwnd, GetDlgItem(IDC_SUPER_SAMPLING_COMBO).GetHwnd());
      AddToolTip("Set the amount of MSAA samples.\r\n\r\nMSAA can help reduce geometry aliasing at the cost of performance and GPU memory.\r\n\r\nThis can improve image quality if not using supersampling.", hwndDlg, toolTipHwnd, GetDlgItem(IDC_MSAA_COMBO).GetHwnd());
      AddToolTip("When checked, it overwrites the ball image/decal image(s) for every table.", hwndDlg, toolTipHwnd, GetDlgItem(IDC_OVERWRITE_BALL_IMAGE_CHECK).GetHwnd());
      AddToolTip("Select Display for Video output.", hwndDlg, toolTipHwnd, GetDlgItem(IDC_DISPLAY_ID).GetHwnd());
      AddToolTip("Enables BAM Headtracking. See https://www.ravarcade.pl for details.", hwndDlg, toolTipHwnd, GetDlgItem(IDC_HEADTRACKING).GetHwnd());
      AddToolTip("Enables additional post-processed reflections on all objects (besides the playfield).", hwndDlg, toolTipHwnd, GetDlgItem(IDC_GLOBAL_SSREFLECTION_CHECK).GetHwnd());

      AddToolTip("Limit the quality of reflections for better performance.\r\n\r\n'Dynamic' is recommended and will give the best results, but may harm performance.\r\n\r\n'Static Only' has no performance cost (except for VR rendering).\r\n\r\nOther options feature different trade-offs between quality and performance.", hwndDlg, toolTipHwnd, GetDlgItem(IDC_MAX_REFLECTION_COMBO).GetHwnd());
      AddToolTip("Limit the quality of ambient occlusion for better performance.\r\nDynamic is the better with contact shadows for dynamic objects but higher performance requirements.", hwndDlg, toolTipHwnd, GetDlgItem(IDC_MAX_AO_COMBO).GetHwnd());
      AddToolTip("Activate this to enhance the texture filtering.\r\nThis slows down performance only a bit (on most systems), but increases quality tremendously.", hwndDlg, toolTipHwnd, GetDlgItem(IDC_FORCE_ANISO).GetHwnd());

      AddToolTip("Physical width of the display area of the screen in centimeters, in landscape orientation (width > height).\r\n\r\nThis is needed to get the correct size when using 'Window' mode for the camera.", hwndDlg, toolTipHwnd, GetDlgItem(IDC_SCREEN_WIDTH).GetHwnd());
      AddToolTip("Physical height of the display area of the screen in centimeters, in landscape orientation (width > height).\r\n\r\nThis is needed to get the correct size when using 'Window' mode for the camera.", hwndDlg, toolTipHwnd, GetDlgItem(IDC_SCREEN_HEIGHT).GetHwnd());
   }

   SendDlgItemMessage(IDC_APPLICATION_SETTINGS, BM_SETCHECK, BST_CHECKED, 0);
   GetDlgItem(IDC_TABLE_OVERRIDE).EnableWindow(g_pvp->m_ptableActive != nullptr);

   HWND hwnd;
   hwnd = GetDlgItem(IDC_MAX_TEXTURE_COMBO).GetHwnd();
   SendMessage(hwnd, WM_SETREDRAW, FALSE, 0); // to speed up adding the entries :/
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "1024");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "2048");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "3172");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "4096");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "5120");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "6144");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "7168");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Unlimited");
   SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);

   hwnd = GetDlgItem(IDC_VIDEO_SYNC_MODE).GetHwnd();
   SendMessage(hwnd, WM_SETREDRAW, FALSE, 0); // to speed up adding the entries :/
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "No Sync");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Vertical Sync");
   #if !defined(ENABLE_BGFX)
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Adaptive Sync");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Frame Pacing");
   #endif
   SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);

   hwnd = GetDlgItem(IDC_SUPER_SAMPLING_COMBO).GetHwnd();
   SendMessage(hwnd, WM_SETREDRAW, FALSE, 0); // to speed up adding the entries :/
   for (size_t i = 0; i < AAfactorCount; ++i)
      SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)AAfactorNames[i]);
   SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);

   hwnd = GetDlgItem(IDC_MSAA_COMBO).GetHwnd();
   SendMessage(hwnd, WM_SETREDRAW, FALSE, 0); // to speed up adding the entries :/
   for (size_t i = 0; i < MSAASampleCount; ++i)
      SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)MSAASampleNames[i]);
   SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);

   hwnd = GetDlgItem(IDC_MAX_AO_COMBO).GetHwnd();
   SendMessage(hwnd, WM_SETREDRAW, FALSE, 0); // to speed up adding the entries :/
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Disable AO");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Static AO");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Dynamic AO");
   SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);

   hwnd = GetDlgItem(IDC_MAX_REFLECTION_COMBO).GetHwnd();
   SendMessage(hwnd, WM_SETREDRAW, FALSE, 0); // to speed up adding the entries :/
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Disable Reflections");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Balls Only");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Static Only");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Static & Balls");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Static & Unsynced Dynamic");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Dynamic");
   SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);

   hwnd = GetDlgItem(IDC_POST_PROCESS_COMBO).GetHwnd();
   SendMessage(hwnd, WM_SETREDRAW, FALSE, 0); // to speed up adding the entries :/
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Disabled");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Fast FXAA");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Standard FXAA");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Quality FXAA");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Fast NFAA");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Standard DLAA");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Quality SMAA");
   SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);

   hwnd = GetDlgItem(IDC_SHARPEN_COMBO).GetHwnd();
   SendMessage(hwnd, WM_SETREDRAW, FALSE, 0); // to speed up adding the entries :/
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Disabled");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "CAS");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Bilateral CAS");
   SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);

   hwnd = GetDlgItem(IDC_BG_SET).GetHwnd();
   SendMessage(hwnd, WM_SETREDRAW, FALSE, 0); // to speed up adding the entries :/
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Desktop & FSS");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Cabinet");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Desktop (no FSS)");
   SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);

   hwnd = GetDlgItem(IDC_3D_STEREO_ANAGLYPH_FILTER).GetHwnd();
   SendMessage(hwnd, WM_SETREDRAW, FALSE, 0); // to speed up adding the entries :/
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "None");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Dubois");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Luminance");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Deghost");
   SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);

   hwnd = GetDlgItem(IDC_AR_COMBO).GetHwnd();
   SendMessage(hwnd, WM_SETREDRAW, FALSE, 0); // to speed up adding the entries :/
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Free");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) " 4: 3 [Landscape]");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "16:10 [Landscape]");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "16: 9 [Landscape]");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "21:10 [Landscape]");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "21: 9 [Landscape]");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) " 4: 3 [Portrait]");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "16:10 [Portrait]");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "16: 9 [Portrait]");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "21:10 [Portrait]");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "21: 9 [Portrait]");
   SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);

   const HWND hwndARASlider = GetDlgItem(IDC_ARASlider).GetHwnd();
   SendMessage(hwndARASlider, TBM_SETRANGE, fTrue, MAKELONG(0, 10));
   SendMessage(hwndARASlider, TBM_SETTICFREQ, 1, 0);
   SendMessage(hwndARASlider, TBM_SETLINESIZE, 0, 1);
   SendMessage(hwndARASlider, TBM_SETPAGESIZE, 0, 1);
   SendMessage(hwndARASlider, TBM_SETTHUMBLENGTH, 10, 0);

   const HWND hwndDNSlider = GetDlgItem(IDC_DAYNIGHT_SLIDER).GetHwnd();
   SendMessage(hwndDNSlider, TBM_SETRANGE, fTrue, MAKELONG(2, 100));
   SendMessage(hwndDNSlider, TBM_SETTICFREQ, 10, 0);
   SendMessage(hwndDNSlider, TBM_SETLINESIZE, 0, 1);
   SendMessage(hwndDNSlider, TBM_SETPAGESIZE, 0, 1);
   SendMessage(hwndDNSlider, TBM_SETTHUMBLENGTH, 10, 0);

   // Disable unsupported features in UI
#if defined(ENABLE_SDL_VIDEO)
   GetDlgItem(IDC_DISABLE_DWM).EnableWindow(false);
   GetDlgItem(IDC_10BIT_VIDEO).EnableWindow(false);
   GetDlgItem(IDC_3D_STEREO_ZPD).EnableWindow(false);
#else
   // adapt layout for the hidden MSAA control
   GetDlgItem(IDC_MSAA_LABEL).ShowWindow(false);
   GetDlgItem(IDC_MSAA_COMBO).ShowWindow(false);
   GetDlgItem(IDC_MSAA_COMBO).EnableWindow(false);
#define SHIFT_WND(id, amount) \
   { \
   CRect rc = GetDlgItem(id).GetClientRect(); \
   GetDlgItem(id).MapWindowPoints(this->GetHwnd(), rc); \
   rc.OffsetRect(0, amount); \
   GetDlgItem(id).MoveWindow(rc); \
   }
   SHIFT_WND(IDC_SUPER_SAMPLING_LABEL, -14)
   SHIFT_WND(IDC_SUPER_SAMPLING_COMBO, -14)
   SHIFT_WND(IDC_POST_PROCESS_AA_LABEL, -9)
   SHIFT_WND(IDC_POST_PROCESS_COMBO, -9)
   SHIFT_WND(IDC_SHARPEN_LABEL, -4)
   SHIFT_WND(IDC_SHARPEN_COMBO, -4)
#endif

   UpdateFullscreenModesList();

   OnCommand(IDC_APPLICATION_SETTINGS, 0L);

   return TRUE;
}

Settings& VideoOptionsDialog::GetEditedSettings()
{
   if (g_pvp->m_ptableActive && IsDlgButtonChecked(IDC_TABLE_OVERRIDE) == BST_CHECKED)
      return m_tableSettings;
   SendDlgItemMessage(IDC_APPLICATION_SETTINGS, BM_SETCHECK, BST_CHECKED, 0);
   return m_appSettings;
}

void VideoOptionsDialog::LoadSettings()
{
   HWND hwnd;
   char tmp[256];

   Settings& settings = GetEditedSettings();
   // persist user edition to local copy of settings
   if (m_editedSettings != nullptr && m_editedSettings != &settings)
      SaveSettings(m_editedSettings == &m_appSettings);
   m_editedSettings = &settings;

   m_initialMaxTexDim = settings.LoadValueWithDefault(Settings::Player, "MaxTexDimension"s, 0);
   const int maxTexDim = ((1023 + m_initialMaxTexDim) / 1024) - 1;
   SendDlgItemMessage(IDC_MAX_TEXTURE_COMBO, CB_SETCURSEL, maxTexDim < 0 ? 7 : maxTexDim, 0);

   const bool trail = settings.LoadValueWithDefault(Settings::Player, "BallTrail"s, true);
   SendDlgItemMessage(IDC_GLOBAL_TRAIL_CHECK, BM_SETCHECK, trail ? BST_CHECKED : BST_UNCHECKED, 0);

   const float trailStrength = settings.LoadValueWithDefault(Settings::Player, "BallTrailStrength"s, 0.5f);
   sprintf_s(tmp, sizeof(tmp), "%f", trailStrength);
   SetDlgItemText(IDC_BALL_TRAIL_STRENGTH, tmp);

   const bool disableLighting = settings.LoadValueWithDefault(Settings::Player, "DisableLightingForBalls"s, false);
   SendDlgItemMessage(IDC_GLOBAL_DISABLE_LIGHTING_BALLS, BM_SETCHECK, disableLighting ? BST_CHECKED : BST_UNCHECKED, 0);

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
   SetDlgItemInt(IDC_MAX_FPS, maxFPS, FALSE);

   #if defined(ENABLE_BGFX)
   syncMode = (VideoSyncMode)clamp(syncMode, VSM_NONE, VSM_VSYNC);
   #else
   syncMode = (VideoSyncMode)clamp(syncMode, VSM_NONE, VSM_FRAME_PACING);
   #endif
   SendDlgItemMessage(IDC_VIDEO_SYNC_MODE, CB_SETCURSEL, syncMode, 0);

   const int maxPrerenderedFrames = settings.LoadValueWithDefault(Settings::Player, "MaxPrerenderedFrames"s, 0);
   SetDlgItemInt(IDC_MAX_PRE_FRAMES, maxPrerenderedFrames, FALSE);
   #if defined(ENABLE_SDL_VIDEO)
   GetDlgItem(IDC_MAX_PRE_FRAMES).EnableWindow(false); // OpenGL does not support this option
   #endif

   const float nudgeStrength = settings.LoadValueWithDefault(Settings::Player, "NudgeStrength"s, 2e-2f);
   sprintf_s(tmp, sizeof(tmp), "%f", nudgeStrength);
   SetDlgItemText(IDC_NUDGE_STRENGTH, tmp);

   const float AAfactor = settings.LoadValueWithDefault(Settings::Player, "AAFactor"s, settings.LoadValueWithDefault(Settings::Player, "USEAA"s, false) ? 1.5f : 1.0f);
   SendDlgItemMessage(IDC_SUPER_SAMPLING_COMBO, CB_SETCURSEL, getBestMatchingAAfactorIndex(AAfactor), 0);

   const int MSAASamples = settings.LoadValueWithDefault(Settings::Player, "MSAASamples"s, 1);
   const int CurrMSAAPos = static_cast<const int>(std::find(MSAASamplesOpts, MSAASamplesOpts + (sizeof(MSAASamplesOpts) / sizeof(MSAASamplesOpts[0])), MSAASamples) - MSAASamplesOpts);
   SendDlgItemMessage(IDC_MSAA_COMBO, CB_SETCURSEL, CurrMSAAPos, 0);

   const bool disableAO = settings.LoadValueWithDefault(Settings::Player, "DisableAO"s, false);
   const bool dynAO = settings.LoadValueWithDefault(Settings::Player, "DynamicAO"s, true);
   SendDlgItemMessage(IDC_MAX_AO_COMBO, CB_SETCURSEL, disableAO ? 0 : dynAO ? 2 : 1, 0);

   const bool ssreflection = settings.LoadValueWithDefault(Settings::Player, "SSRefl"s, false);
   SendDlgItemMessage(IDC_GLOBAL_SSREFLECTION_CHECK, BM_SETCHECK, ssreflection ? BST_CHECKED : BST_UNCHECKED, 0);

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
   SendDlgItemMessage(IDC_MAX_REFLECTION_COMBO, CB_SETCURSEL, maxReflection, 0);

   const bool overwiteBallImage = settings.LoadValueWithDefault(Settings::Player, "OverwriteBallImage"s, false);
   SendDlgItemMessage(IDC_OVERWRITE_BALL_IMAGE_CHECK, BM_SETCHECK, overwiteBallImage ? BST_CHECKED : BST_UNCHECKED, 0);

   string imageName;
   bool hr = settings.LoadValue(Settings::Player, "BallImage"s, imageName);
   if (!hr)
      imageName.clear();
   SetDlgItemText(IDC_BALL_IMAGE_EDIT, imageName.c_str());
   hr = settings.LoadValue(Settings::Player, "DecalImage"s, imageName);
   if (!hr)
      imageName.clear();
   SetDlgItemText(IDC_BALL_DECAL_EDIT, imageName.c_str());
   if (overwiteBallImage == 0)
   {
      GetDlgItem(IDC_BROWSE_BALL_IMAGE).EnableWindow(FALSE);
      GetDlgItem(IDC_BROWSE_BALL_DECAL).EnableWindow(FALSE);
      GetDlgItem(IDC_BALL_IMAGE_EDIT).EnableWindow(FALSE);
      GetDlgItem(IDC_BALL_DECAL_EDIT).EnableWindow(FALSE);
   }

   const int fxaa = settings.LoadValueWithDefault(Settings::Player, "FXAA"s, (int)Standard_FXAA);
   SendDlgItemMessage(IDC_POST_PROCESS_COMBO, CB_SETCURSEL, fxaa, 0);

   const int sharpen = settings.LoadValueWithDefault(Settings::Player, "Sharpen"s, 0);
   SendDlgItemMessage(IDC_SHARPEN_COMBO, CB_SETCURSEL, sharpen, 0);

   const bool scaleFX_DMD = settings.LoadValueWithDefault(Settings::Player, "ScaleFXDMD"s, false);
   SendDlgItemMessage(IDC_SCALE_FX_DMD, BM_SETCHECK, scaleFX_DMD ? BST_CHECKED : BST_UNCHECKED, 0);

   const int bgset = settings.LoadValueWithDefault(Settings::Player, "BGSet"s, 0);
   SendDlgItemMessage(IDC_BG_SET, CB_SETCURSEL, bgset, 0);
   // FIXME
   SendDlgItemMessage(IDC_BG_SET, BM_SETCHECK, (bgset != 0) ? BST_CHECKED : BST_UNCHECKED, 0);

   bool fakeStereo = true;
   #if defined(ENABLE_OPENGL) || defined(ENABLE_BGFX)
   fakeStereo = settings.LoadValueWithDefault(Settings::Player, "Stereo3DFake"s, false);
   #elif defined(ENABLE_DX9)
   GetDlgItem(IDC_FAKE_STEREO).EnableWindow(FALSE);
   #endif
   SendDlgItemMessage(IDC_FAKE_STEREO, BM_SETCHECK, fakeStereo ? BST_CHECKED : BST_UNCHECKED, 0);

   const int stereo3D = settings.LoadValueWithDefault(Settings::Player, "Stereo3D"s, 0);
   hwnd = GetDlgItem(IDC_3D_STEREO).GetHwnd();
   SendMessage(hwnd, WM_SETREDRAW, FALSE, 0); // to speed up adding the entries :/
   SendMessage(hwnd, CB_RESETCONTENT, 0, 0);
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Disabled");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Top / Bottom");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Interlaced (e.g. LG TVs)");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Flipped Interlaced (e.g. LG TVs)");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Side by Side");
   static const string defaultNames[] = { "Red/Cyan"s, "Green/Magenta"s, "Blue/Amber"s, "Cyan/Red"s, "Magenta/Green"s, "Amber/Blue"s, "Custom 1"s, "Custom 2"s, "Custom 3"s, "Custom 4"s };
   string name[std::size(defaultNames)];
   for (size_t i = 0; i < std::size(defaultNames); i++)
      if (!settings.LoadValue(Settings::Player, "Anaglyph"s.append(std::to_string(i + 1)).append("Name"s), name[i]))
         name[i] = defaultNames[i];
   for (size_t i = 0; i < std::size(defaultNames); i++)
      SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)name[i].c_str());
   SendMessage(hwnd, CB_SETCURSEL, stereo3D, 0);
   SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);

   OnCommand(IDC_3D_STEREO, 0L); // Force UI update

   const bool stereo3DY = settings.LoadValueWithDefault(Settings::Player, "Stereo3DYAxis"s, false);
   SendDlgItemMessage(IDC_3D_STEREO_Y, BM_SETCHECK, stereo3DY ? BST_CHECKED : BST_UNCHECKED, 0);

   const float stereo3DOfs = settings.LoadValueWithDefault(Settings::Player, "Stereo3DOffset"s, 0.f);
   sprintf_s(tmp, sizeof(tmp), "%f", stereo3DOfs);
   SetDlgItemText(IDC_3D_STEREO_OFS, tmp);

   const float stereo3DMS = settings.LoadValueWithDefault(Settings::Player, "Stereo3DMaxSeparation"s, 0.03f);
   sprintf_s(tmp, sizeof(tmp), "%f", stereo3DMS);
   SetDlgItemText(IDC_3D_STEREO_MS, tmp);

   const float stereo3DES = settings.LoadValueWithDefault(Settings::Player, "Stereo3DEyeSeparation"s, 63.0f);
   sprintf_s(tmp, sizeof(tmp), "%f", stereo3DES);
   SetDlgItemText(IDC_3D_STEREO_ES, tmp);

   const float stereo3DZPD = settings.LoadValueWithDefault(Settings::Player, "Stereo3DZPD"s, 0.5f);
   sprintf_s(tmp, sizeof(tmp), "%f", stereo3DZPD);
   SetDlgItemText(IDC_3D_STEREO_ZPD, tmp);

   const bool bamHeadtracking = settings.LoadValueWithDefault(Settings::Player, "BAMHeadTracking"s, false);
   SendDlgItemMessage(IDC_HEADTRACKING, BM_SETCHECK, bamHeadtracking ? BST_CHECKED : BST_UNCHECKED, 0);

   const float stereo3DBrightness = settings.LoadValueWithDefault(Settings::Player, "Stereo3DBrightness"s, 1.0f);
   sprintf_s(tmp, sizeof(tmp), "%f", stereo3DBrightness);
   SetDlgItemText(IDC_3D_STEREO_BRIGHTNESS, tmp);

   const float stereo3DSaturation = settings.LoadValueWithDefault(Settings::Player, "Stereo3DSaturation"s, 1.0f);
   sprintf_s(tmp, sizeof(tmp), "%f", stereo3DSaturation);
   SetDlgItemText(IDC_3D_STEREO_DESATURATION, tmp);

   const bool disableDWM = settings.LoadValueWithDefault(Settings::Player, "DisableDWM"s, false);
   SendDlgItemMessage(IDC_DISABLE_DWM, BM_SETCHECK, disableDWM ? BST_CHECKED : BST_UNCHECKED, 0);
   GetDlgItem(IDC_DISABLE_DWM).EnableWindow(IsWindowsVistaOr7()); // DWM may not be disabled on Windows 8+

   const bool nvidiaApi = settings.LoadValueWithDefault(Settings::Player, "UseNVidiaAPI"s, false);
   SendDlgItemMessage(IDC_USE_NVIDIA_API_CHECK, BM_SETCHECK, nvidiaApi ? BST_CHECKED : BST_UNCHECKED, 0);

   const bool bloomOff = settings.LoadValueWithDefault(Settings::Player, "ForceBloomOff"s, false);
   SendDlgItemMessage(IDC_BLOOM_OFF, BM_SETCHECK, bloomOff ? BST_CHECKED : BST_UNCHECKED, 0);

   const bool forceAniso = settings.LoadValueWithDefault(Settings::Player, "ForceAnisotropicFiltering"s, true);
   SendDlgItemMessage(IDC_FORCE_ANISO, BM_SETCHECK, forceAniso ? BST_CHECKED : BST_UNCHECKED, 0);

   const bool compressTextures = settings.LoadValueWithDefault(Settings::Player, "CompressTextures"s, false);
   SendDlgItemMessage(IDC_TEX_COMPRESS, BM_SETCHECK, compressTextures ? BST_CHECKED : BST_UNCHECKED, 0);

   const bool softwareVP = settings.LoadValueWithDefault(Settings::Player, "SoftwareVertexProcessing"s, false);
   SendDlgItemMessage(IDC_SOFTWARE_VP, BM_SETCHECK, softwareVP ? BST_CHECKED : BST_UNCHECKED, 0);

   const bool video10bit = settings.LoadValueWithDefault(Settings::Player, "Render10Bit"s, false);
   SendDlgItemMessage(IDC_10BIT_VIDEO, BM_SETCHECK, video10bit ? BST_CHECKED : BST_UNCHECKED, 0);

   //const int depthcur = settings.LoadValueWithDefault(Settings::Player, "ColorDepth"s, 32);
   //const int refreshrate = settings.LoadValueWithDefault(Settings::Player, "RefreshRate"s, 0);

   int display;
   hr = settings.LoadValue(Settings::Player, "Display"s, display);
   vector<VPX::Window::DisplayConfig> displays;
   VPX::Window::GetDisplays(displays);
   if ((!hr) || ((int)displays.size() <= display))
      display = -1;
   hwnd = GetDlgItem(IDC_DISPLAY_ID).GetHwnd();
   SendMessage(hwnd, WM_SETREDRAW, FALSE, 0); // to speed up adding the entries :/
   SendMessage(hwnd, CB_RESETCONTENT, 0, 0);
   for (vector<VPX::Window::DisplayConfig>::iterator dispConf = displays.begin(); dispConf != displays.end(); ++dispConf)
   {
      if (display == -1 && dispConf->isPrimary)
         display = dispConf->display;
      char displayName[256];
      sprintf_s(displayName, sizeof(displayName), "Display %d%s %dx%d %s", dispConf->display + 1, (dispConf->isPrimary) ? "*" : "", dispConf->width, dispConf->height, dispConf->GPU_Name);
      SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)displayName);
   }
   SendMessage(hwnd, CB_SETCURSEL, display, 0);
   SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);

   const bool fullscreen = settings.LoadValueWithDefault(Settings::Player, "FullScreen"s, IsWindows10_1803orAbove());
   SendDlgItemMessage(IDC_EXCLUSIVE_FULLSCREEN, BM_SETCHECK, fullscreen ? BST_CHECKED : BST_UNCHECKED, 0);
   SendDlgItemMessage(IDC_WINDOWED, BM_SETCHECK, fullscreen ? BST_UNCHECKED : BST_CHECKED, 0);
   OnCommand(IDC_EXCLUSIVE_FULLSCREEN, 0L); // Force UI update

   const int widthcur = settings.LoadValueWithDefault(Settings::Player, "Width"s, -1);
   const int heightcur = settings.LoadValueWithDefault(Settings::Player, "Height"s, -1);
   SetDlgItemInt(IDC_WIDTH_EDIT, widthcur, FALSE);
   SetDlgItemInt(IDC_HEIGHT_EDIT, heightcur, FALSE);

   SendDlgItemMessage(IDC_AR_COMBO, CB_SETCURSEL, 0, 0);
   for (int i = 1; i < sizeof(arFactors) / sizeof(arFactors[0]); i++)
      if (heightcur == (int)(widthcur / arFactors[i]))
         SendDlgItemMessage(IDC_AR_COMBO, CB_SETCURSEL, i, 0);

   UpdateDisplayHeightFromWidth();

   const float screenWidth = settings.LoadValueWithDefault(Settings::Player, "ScreenWidth"s, 0.0f);
   sprintf_s(tmp, sizeof(tmp), "%f", screenWidth);
   SetDlgItemText(IDC_SCREEN_WIDTH, tmp);
   const float screenHeight = settings.LoadValueWithDefault(Settings::Player, "ScreenHeight"s, 0.0f);
   sprintf_s(tmp, sizeof(tmp), "%f", screenHeight);
   SetDlgItemText(IDC_SCREEN_HEIGHT, tmp);
   const float screenInclination = settings.LoadValueWithDefault(Settings::Player, "ScreenInclination"s, 0.0f);
   sprintf_s(tmp, sizeof(tmp), "%f", screenInclination);
   SetDlgItemText(IDC_SCREEN_INCLINATION, tmp);
   const float screenPlayerX = settings.LoadValueWithDefault(Settings::Player, "ScreenPlayerX"s, 0.0f);
   sprintf_s(tmp, sizeof(tmp), "%f", screenPlayerX);
   SetDlgItemText(IDC_SCREEN_PLAYERX, tmp);
   const float screenPlayerY = settings.LoadValueWithDefault(Settings::Player, "ScreenPlayerY"s, 0.0f);
   sprintf_s(tmp, sizeof(tmp), "%f", screenPlayerY);
   SetDlgItemText(IDC_SCREEN_PLAYERY, tmp);
   const float screenPlayerZ = settings.LoadValueWithDefault(Settings::Player, "ScreenPlayerZ"s, 70.0f);
   sprintf_s(tmp, sizeof(tmp), "%f", screenPlayerZ);
   SetDlgItemText(IDC_SCREEN_PLAYERZ, tmp);

   const int alphaRampsAccuracy = settings.LoadValueWithDefault(Settings::Player, "AlphaRampAccuracy"s, 10);
   SendDlgItemMessage(IDC_ARASlider, TBM_SETPOS, TRUE, alphaRampsAccuracy);

   const bool ballAntiStretch = settings.LoadValueWithDefault(Settings::Player, "BallAntiStretch"s, false);
   SendDlgItemMessage(IDC_BALL_ANTISTRETCH, BM_SETCHECK, ballAntiStretch ? BST_CHECKED : BST_UNCHECKED, 0);

   SendDlgItemMessage(IDC_OVERRIDE_DN, BM_SETCHECK, settings.LoadValueWithDefault(Settings::TableOverride, "OverrideEmissionScale"s, false) ? BST_CHECKED : BST_UNCHECKED, 0);
   SendDlgItemMessage(IDC_DAYNIGHT_SLIDER, TBM_SETPOS, TRUE, (int) (100.f * settings.LoadValueWithDefault(Settings::Player, "EmissionScale"s, 0.5f)));
   SendDlgItemMessage(IDC_DYNAMIC_DN, BM_SETCHECK, settings.LoadValueWithDefault(Settings::Player, "DynamicDayNight"s, false) ? BST_CHECKED : BST_UNCHECKED, 0);
   SetDlgItemText(IDC_DN_LATITUDE, std::to_string(settings.LoadValueWithDefault(Settings::Player, "Latitude"s, 52.52f)).c_str());
   SetDlgItemText(IDC_DN_LONGITUDE, std::to_string(settings.LoadValueWithDefault(Settings::Player, "Longitude"s, 13.37f)).c_str());
   OnCommand(IDC_OVERRIDE_DN, 0L); // Force UI update
}

void VideoOptionsDialog::SaveSettings(const bool saveAll)
{
   if (m_editedSettings == nullptr)
      return;
   Settings& settings = *m_editedSettings;

   BOOL nothing = 0;
   const bool fullscreen = IsDlgButtonChecked(IDC_EXCLUSIVE_FULLSCREEN) == BST_CHECKED;
   if (fullscreen)
   {
      LRESULT index = SendDlgItemMessage(IDC_SIZELIST, LB_GETCURSEL, 0, 0);
      if (index == LB_ERR)
         index = 0;
      if (index >= 0 && (size_t)index < m_allVideoModes.size())
      {
         const VPX::Window::VideoMode* const pvm = &m_allVideoModes[index];
         settings.SaveValue(Settings::Player, "Width"s, pvm->width, !saveAll);
         settings.SaveValue(Settings::Player, "Height"s, pvm->height, !saveAll);
         settings.SaveValue(Settings::Player, "ColorDepth"s, pvm->depth, !saveAll);
         settings.SaveValue(Settings::Player, "RefreshRate"s, pvm->refreshrate, !saveAll);
      }
   }
   else
   {
      LRESULT arMode = SendDlgItemMessage(IDC_AR_COMBO, CB_GETCURSEL, 0, 0);
      if (arMode == CB_ERR)
         arMode = 0;
      int width = GetDlgItemInt(IDC_WIDTH_EDIT, false);
      int height = GetDlgItemInt(IDC_HEIGHT_EDIT, false);
      if (arMode > 0)
         height = (int)(width / arFactors[arMode]);
      if (!saveAll)
      {
         settings.DeleteValue(Settings::Player, "Width"s);
         settings.DeleteValue(Settings::Player, "Height"s);
         settings.DeleteValue(Settings::Player, "ColorDepth"s);
         settings.DeleteValue(Settings::Player, "RefreshRate"s);
      }
      if (width > 0 && height > 0)
      {
         settings.SaveValue(Settings::Player, "Width"s, width, !saveAll);
         settings.SaveValue(Settings::Player, "Height"s, height, !saveAll);
      }
   }
   LRESULT display = SendDlgItemMessage(IDC_DISPLAY_ID, CB_GETCURSEL, 0, 0);
   if (display == CB_ERR)
      display = 0;
   settings.SaveValue(Settings::Player, "Display"s, (int)display, !saveAll);
   settings.SaveValue(Settings::Player, "FullScreen"s, fullscreen, !saveAll);
   settings.SaveValue(Settings::Player, "ScreenWidth"s, GetDlgItemText(IDC_SCREEN_WIDTH).GetString(), !saveAll);
   settings.SaveValue(Settings::Player, "ScreenHeight"s, GetDlgItemText(IDC_SCREEN_HEIGHT).GetString(), !saveAll);
   settings.SaveValue(Settings::Player, "ScreenInclination"s, GetDlgItemText(IDC_SCREEN_INCLINATION).GetString(), !saveAll);
   settings.SaveValue(Settings::Player, "ScreenPlayerX"s, GetDlgItemText(IDC_SCREEN_PLAYERX).GetString(), !saveAll);
   settings.SaveValue(Settings::Player, "ScreenPlayerY"s, GetDlgItemText(IDC_SCREEN_PLAYERY).GetString(), !saveAll);
   settings.SaveValue(Settings::Player, "ScreenPlayerZ"s, GetDlgItemText(IDC_SCREEN_PLAYERZ).GetString(), !saveAll);
   settings.SaveValue(Settings::Player, "BAMHeadTracking"s, IsDlgButtonChecked(IDC_HEADTRACKING) == BST_CHECKED, !saveAll);
   settings.SaveValue(Settings::Player, "Render10Bit"s, IsDlgButtonChecked(IDC_10BIT_VIDEO) == BST_CHECKED, !saveAll);
   LRESULT maxTexDim = SendDlgItemMessage(IDC_MAX_TEXTURE_COMBO, CB_GETCURSEL, 0, 0);
   if (maxTexDim == CB_ERR)
      maxTexDim = 7;
   maxTexDim = maxTexDim == 7 ? 0 : (1024 * (maxTexDim + 1));
   settings.SaveValue(Settings::Player, "MaxTexDimension"s, (int)maxTexDim, !saveAll);
   if (m_initialMaxTexDim != maxTexDim)
      MessageBox("You have changed the maximum texture size.\n\nThis change will only take effect after reloading the tables.", "Reload tables", MB_ICONWARNING);
   settings.SaveValue(Settings::Player, "BallTrail"s, IsDlgButtonChecked(IDC_GLOBAL_TRAIL_CHECK) == BST_CHECKED, !saveAll);
   settings.SaveValue(Settings::Player, "BallTrailStrength"s, GetDlgItemText(IDC_BALL_TRAIL_STRENGTH).GetString(), !saveAll);
   settings.SaveValue(Settings::Player, "DisableLightingForBalls"s, IsDlgButtonChecked(IDC_GLOBAL_DISABLE_LIGHTING_BALLS) == BST_CHECKED, !saveAll);
   settings.SaveValue(Settings::Player, "MaxFramerate"s, (int)GetDlgItemInt(IDC_MAX_FPS, nothing, TRUE), !saveAll);
   LRESULT syncMode = SendDlgItemMessage(IDC_VIDEO_SYNC_MODE, CB_GETCURSEL, 0, 0);
   if (syncMode == CB_ERR)
      syncMode = VideoSyncMode::VSM_FRAME_PACING;
   settings.SaveValue(Settings::Player, "SyncMode"s, (int)syncMode, !saveAll);
   settings.SaveValue(Settings::Player, "MaxPrerenderedFrames"s, (int)GetDlgItemInt(IDC_MAX_PRE_FRAMES, nothing, TRUE), !saveAll);
   settings.SaveValue(Settings::Player, "NudgeStrength"s, GetDlgItemText(IDC_NUDGE_STRENGTH).GetString(), !saveAll);
   LRESULT fxaa = SendDlgItemMessage(IDC_POST_PROCESS_COMBO, CB_GETCURSEL, 0, 0);
   if (fxaa == CB_ERR)
      fxaa = Standard_FXAA;
   settings.SaveValue(Settings::Player, "FXAA"s, (int) fxaa, !saveAll);
   LRESULT sharpen = SendDlgItemMessage(IDC_SHARPEN_COMBO, CB_GETCURSEL, 0, 0);
   if (sharpen == CB_ERR)
      sharpen = 0;
   settings.SaveValue(Settings::Player, "Sharpen"s, (int)sharpen, !saveAll);
   settings.SaveValue(Settings::Player, "ScaleFXDMD"s, IsDlgButtonChecked(IDC_SCALE_FX_DMD) == BST_CHECKED, !saveAll);
   LRESULT bgset = SendDlgItemMessage(IDC_BG_SET, CB_GETCURSEL, 0, 0);
   if (bgset == CB_ERR)
      bgset = 0;
   settings.SaveValue(Settings::Player, "BGSet"s, (int)bgset, !saveAll);
   // update the cached current view setup of all loaded tables since it also depends on this setting
   for (auto table : g_pvp->m_vtable)
      table->UpdateCurrentBGSet();
   LRESULT AAfactorIndex = SendDlgItemMessage(IDC_SUPER_SAMPLING_COMBO, CB_GETCURSEL, 0, 0);
   if (AAfactorIndex == CB_ERR)
      AAfactorIndex = getBestMatchingAAfactorIndex(1);
   const float AAfactor = (AAfactorIndex < AAfactorCount) ? AAfactors[AAfactorIndex] : 1.0f;
   settings.SaveValue(Settings::Player, "USEAA"s, AAfactor > 1.0f, !saveAll);
   settings.SaveValue(Settings::Player, "AAFactor"s, AAfactor, !saveAll);
   LRESULT MSAASamplesIndex = SendDlgItemMessage(IDC_MSAA_COMBO, CB_GETCURSEL, 0, 0);
   if (MSAASamplesIndex == CB_ERR)
      MSAASamplesIndex = 0;
   const int MSAASamples = (MSAASamplesIndex < MSAASampleCount) ? MSAASamplesOpts[MSAASamplesIndex] : 1;
   settings.SaveValue(Settings::Player, "MSAASamples"s, MSAASamples, !saveAll);
   LRESULT maxAOMode = SendDlgItemMessage(IDC_MAX_AO_COMBO, CB_GETCURSEL, 0, 0);
   if (maxAOMode == CB_ERR)
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
   LRESULT maxReflectionMode = SendDlgItemMessage(IDC_MAX_REFLECTION_COMBO, CB_GETCURSEL, 0, 0);
   if (maxReflectionMode == CB_ERR)
      maxReflectionMode = RenderProbe::REFL_STATIC;
   settings.SaveValue(Settings::Player, "PFReflection"s, (int)maxReflectionMode, !saveAll);
   settings.SaveValue(Settings::Player, "SSRefl"s, IsDlgButtonChecked(IDC_GLOBAL_SSREFLECTION_CHECK) == BST_CHECKED, !saveAll);
   settings.SaveValue(Settings::Player, "ForceAnisotropicFiltering"s, IsDlgButtonChecked(IDC_FORCE_ANISO) == BST_CHECKED, !saveAll);
   settings.SaveValue(Settings::Player, "CompressTextures"s, IsDlgButtonChecked(IDC_TEX_COMPRESS) == BST_CHECKED, !saveAll);
   settings.SaveValue(Settings::Player, "SoftwareVertexProcessing"s, IsDlgButtonChecked(IDC_SOFTWARE_VP) == BST_CHECKED, !saveAll);
   settings.SaveValue(Settings::Player, "AlphaRampAccuracy"s, (int)SendDlgItemMessage(IDC_ARASlider, TBM_GETPOS, 0, 0), !saveAll);
   LRESULT stereo3D = SendDlgItemMessage(IDC_3D_STEREO, CB_GETCURSEL, 0, 0);
   if (stereo3D == CB_ERR)
      stereo3D = STEREO_OFF;
   settings.SaveValue(Settings::Player, "Stereo3D"s, (int)stereo3D, !saveAll);
   settings.SaveValue(Settings::Player, "Stereo3DEnabled"s, stereo3D != STEREO_OFF, !saveAll);
   settings.SaveValue(Settings::Player, "Stereo3DYAxis"s, IsDlgButtonChecked(IDC_3D_STEREO_Y) == BST_CHECKED, !saveAll);
   settings.SaveValue(Settings::Player, "Stereo3DOffset"s, GetDlgItemText(IDC_3D_STEREO_OFS).GetString(), !saveAll);
   #if defined(ENABLE_OPENGL) || defined(ENABLE_BGFX)
   settings.SaveValue(Settings::Player, "Stereo3DFake"s, IsDlgButtonChecked(IDC_FAKE_STEREO) == BST_CHECKED, !saveAll);
   #endif
   settings.SaveValue(Settings::Player, "Stereo3DMaxSeparation"s, GetDlgItemText(IDC_3D_STEREO_MS).GetString(), !saveAll);
   settings.SaveValue(Settings::Player, "Stereo3DEyeSeparation"s, GetDlgItemText(IDC_3D_STEREO_ES).GetString(), !saveAll);
   settings.SaveValue(Settings::Player, "Stereo3DZPD"s, GetDlgItemText(IDC_3D_STEREO_ZPD).GetString(), !saveAll);
   settings.SaveValue(Settings::Player, "Stereo3DBrightness"s, GetDlgItemText(IDC_3D_STEREO_BRIGHTNESS).GetString(), !saveAll);
   settings.SaveValue(Settings::Player, "Stereo3DSaturation"s, GetDlgItemText(IDC_3D_STEREO_DESATURATION).GetString(), !saveAll);
   if (IsAnaglyphStereoMode(stereo3D))
   {
      LRESULT glassesIndex = stereo3D - STEREO_ANAGLYPH_1;
      LRESULT anaglyphFilter = SendDlgItemMessage(IDC_3D_STEREO_ANAGLYPH_FILTER, CB_GETCURSEL, 0, 0);
      if (anaglyphFilter == CB_ERR)
         anaglyphFilter = 4;
      settings.SaveValue(Settings::Player, "Anaglyph"s.append(std::to_string(glassesIndex + 1)).append("Filter"s), (int)anaglyphFilter, !saveAll);
   }
   settings.SaveValue(Settings::Player, "BAMheadTracking"s, IsDlgButtonChecked(IDC_HEADTRACKING) == BST_CHECKED, !saveAll);
   settings.SaveValue(Settings::Player, "DisableDWM"s, IsDlgButtonChecked(IDC_DISABLE_DWM) == BST_CHECKED, !saveAll);
   settings.SaveValue(Settings::Player, "UseNVidiaAPI"s, IsDlgButtonChecked(IDC_USE_NVIDIA_API_CHECK) == BST_CHECKED, !saveAll);
   settings.SaveValue(Settings::Player, "ForceBloomOff"s, IsDlgButtonChecked(IDC_BLOOM_OFF) == BST_CHECKED, !saveAll);

   settings.SaveValue(Settings::Player, "BallAntiStretch"s, IsDlgButtonChecked(IDC_BALL_ANTISTRETCH) == BST_CHECKED, !saveAll);
   const bool overwriteEnabled = IsDlgButtonChecked(IDC_OVERWRITE_BALL_IMAGE_CHECK) == BST_CHECKED;
   settings.SaveValue(Settings::Player, "OverwriteBallImage"s, overwriteEnabled, !saveAll);
   if (overwriteEnabled)
   {
      settings.SaveValue(Settings::Player, "BallImage"s, GetDlgItem(IDC_BALL_IMAGE_EDIT).GetWindowText().GetString(), !saveAll);
      settings.SaveValue(Settings::Player, "DecalImage"s, GetDlgItem(IDC_BALL_DECAL_EDIT).GetWindowText().GetString(), !saveAll);
   }
   else if (!saveAll)
   {
      settings.DeleteValue(Settings::Player, "BallImage"s);
      settings.DeleteValue(Settings::Player, "DecalImage"s);
   }

   settings.SaveValue(Settings::TableOverride, "OverrideEmissionScale"s, IsDlgButtonChecked(IDC_OVERRIDE_DN) == BST_CHECKED, !saveAll);
   settings.SaveValue(Settings::Player, "EmissionScale"s, (float)SendDlgItemMessage(IDC_DAYNIGHT_SLIDER, TBM_GETPOS, 0, 0) / 100.f, !saveAll);
   settings.SaveValue(Settings::Player, "DynamicDayNight"s, IsDlgButtonChecked(IDC_DYNAMIC_DN) == BST_CHECKED, !saveAll);
   settings.SaveValue(Settings::Player, "Longitude"s, GetDlgItemText(IDC_DN_LONGITUDE).GetString(), !saveAll);
   settings.SaveValue(Settings::Player, "Latitude"s, GetDlgItemText(IDC_DN_LATITUDE).GetString(), !saveAll);

   settings.Save();
}

void VideoOptionsDialog::UpdateDisplayHeightFromWidth()
{
   LRESULT arMode = SendDlgItemMessage(IDC_AR_COMBO, CB_GETCURSEL, 0, 0);
   if (arMode == CB_ERR)
      arMode = 0;
   GetDlgItem(IDC_HEIGHT_EDIT).EnableWindow(arMode == 0);
   if (arMode > 0)
   {
      int width = GetDlgItemInt(IDC_WIDTH_EDIT, false);
      SetDlgItemInt(IDC_HEIGHT_EDIT, (int)(width / arFactors[arMode]), FALSE);
   }
}

BOOL VideoOptionsDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
   UNREFERENCED_PARAMETER(lParam);

   switch (LOWORD(wParam))
   {
   case IDC_SAVE_OVERRIDES:
   case IDC_SAVE_ALL:
   {
      if (IsDlgButtonChecked(IDC_APPLICATION_SETTINGS) == BST_CHECKED)
      {
         SaveSettings(true);
         g_pvp->m_settings = m_appSettings;
      }
      else if (g_pvp->m_ptableActive && LOWORD(wParam) == IDC_SAVE_ALL)
      {
         SaveSettings(true);
         g_pvp->m_ptableActive->m_settings = m_tableSettings;
      }
      else if (g_pvp->m_ptableActive && LOWORD(wParam) == IDC_SAVE_OVERRIDES)
      {
         SaveSettings(false);
         g_pvp->m_ptableActive->m_settings = m_tableSettings;
      }
      g_pvp->m_settings.Save();
      if (g_pvp->m_ptableActive && !g_pvp->m_ptableActive->GetSettingsFileName().empty())
         g_pvp->m_ptableActive->m_settings.SaveToFile(g_pvp->m_ptableActive->GetSettingsFileName());
      break;
   }
   case IDC_TABLE_OVERRIDE:
   {
      GetDlgItem(IDC_SAVE_OVERRIDES).ShowWindow(true);
      SetDlgItemText(IDC_SAVE_ALL, "Save All");
      LoadSettings();
      break;
   }
   case IDC_APPLICATION_SETTINGS:
   {
      GetDlgItem(IDC_SAVE_OVERRIDES).ShowWindow(false);
      SetDlgItemText(IDC_SAVE_ALL, "Save Changes");
      LoadSettings();
      break;
   }
   case IDC_OVERRIDE_DN:
   {
      const bool overrideDN = IsDlgButtonChecked(IDC_OVERRIDE_DN) == BST_CHECKED;
      GetDlgItem(IDC_DAYNIGHT_SLIDER).EnableWindow(overrideDN);
      GetDlgItem(IDC_DYNAMIC_DN).EnableWindow(overrideDN);
      GetDlgItem(IDC_DN_LATITUDE).EnableWindow(overrideDN);
      GetDlgItem(IDC_DN_LONGITUDE).EnableWindow(overrideDN);
      break;
   }
   case IDC_DEFAULTS:
   {
      ResetVideoPreferences(0);
      break;
   }
      case IDC_DEFAULTS_LOW:
      {
         ResetVideoPreferences(1);
         break;
      }
      case IDC_DEFAULTS_HIGH:
      {
         ResetVideoPreferences(2);
         break;
      }
      case IDC_RESET_WINDOW:
      {
         (void)g_pvp->m_settings.DeleteValue(Settings::Player, "WindowPosX"s);
         (void)g_pvp->m_settings.DeleteValue(Settings::Player, "WindowPosY"s);
         break;
      }
      case IDC_OVERWRITE_BALL_IMAGE_CHECK:
      {
         const BOOL overwriteEnabled = IsDlgButtonChecked(IDC_OVERWRITE_BALL_IMAGE_CHECK) == BST_CHECKED ? TRUE : FALSE;
         GetDlgItem(IDC_BROWSE_BALL_IMAGE).EnableWindow(overwriteEnabled);
         GetDlgItem(IDC_BROWSE_BALL_DECAL).EnableWindow(overwriteEnabled);
         GetDlgItem(IDC_BALL_IMAGE_EDIT).EnableWindow(overwriteEnabled);
         GetDlgItem(IDC_BALL_DECAL_EDIT).EnableWindow(overwriteEnabled);
         break;
      }
      case IDC_BROWSE_BALL_IMAGE:
      {
         char szFileName[MAXSTRING];
         szFileName[0] = '\0';

         OPENFILENAME ofn = {};
         ofn.lStructSize = sizeof(OPENFILENAME);
         ofn.hInstance = g_pvp->theInstance;
         ofn.hwndOwner = g_pvp->GetHwnd();
         // TEXT
         ofn.lpstrFilter = "Bitmap, JPEG, PNG, TGA, WEBP, EXR, HDR Files (.bmp/.jpg/.png/.tga/.webp/.exr/.hdr)\0*.bmp;*.jpg;*.jpeg;*.png;*.tga;*.webp;*.exr;*.hdr\0";
         ofn.lpstrFile = szFileName;
         ofn.nMaxFile = sizeof(szFileName);
         ofn.lpstrDefExt = "png";
         ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
         const int ret = GetOpenFileName(&ofn);
         if (!ret)
            break;
         SetDlgItemText(IDC_BALL_IMAGE_EDIT, szFileName);

         break;
      }
      case IDC_BROWSE_BALL_DECAL:
      {
         char szFileName[MAXSTRING];
         szFileName[0] = '\0';

         OPENFILENAME ofn = {};
         ofn.lStructSize = sizeof(OPENFILENAME);
         ofn.hInstance = g_pvp->theInstance;
         ofn.hwndOwner = g_pvp->GetHwnd();
         // TEXT
         ofn.lpstrFilter = "Bitmap, JPEG, PNG, TGA, WEBP, EXR, HDR Files (.bmp/.jpg/.png/.tga/.webp/.exr/.hdr)\0*.bmp;*.jpg;*.jpeg;*.png;*.tga;*.webp;*.exr;*.hdr\0";
         ofn.lpstrFile = szFileName;
         ofn.nMaxFile = sizeof(szFileName);
         ofn.lpstrDefExt = "png";
         ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
         const int ret = GetOpenFileName(&ofn);
         if (!ret)
            break;
         SetDlgItemText(IDC_BALL_DECAL_EDIT, szFileName);

         break;
      }
      case IDC_WINDOWED:
      case IDC_EXCLUSIVE_FULLSCREEN:
      {
         const bool fullscreen = IsDlgButtonChecked(IDC_EXCLUSIVE_FULLSCREEN) == BST_CHECKED;
         // Fullscreen settings
         GetDlgItem(IDC_SIZELIST).ShowWindow(fullscreen ? 1 : 0);
         GetDlgItem(IDC_DISABLE_DWM).ShowWindow(fullscreen ? 1 : 0);
         GetDlgItem(IDC_10BIT_VIDEO).ShowWindow(fullscreen ? 1 : 0);
         // Window settings
         GetDlgItem(IDC_AR_LABEL).ShowWindow(fullscreen ? 0 : 1);
         GetDlgItem(IDC_AR_COMBO).ShowWindow(fullscreen ? 0 : 1);
         GetDlgItem(IDC_WIDTH_LABEL).ShowWindow(fullscreen ? 0 : 1);
         GetDlgItem(IDC_WIDTH_EDIT).ShowWindow(fullscreen ? 0 : 1);
         GetDlgItem(IDC_HEIGHT_LABEL).ShowWindow(fullscreen ? 0 : 1);
         GetDlgItem(IDC_HEIGHT_EDIT).ShowWindow(fullscreen ? 0 : 1);
         GetDlgItem(IDC_RESET_WINDOW).ShowWindow(fullscreen ? 0 : 1);
         break;
      }
      case IDC_DISPLAY_ID:
      {
         UpdateFullscreenModesList();

         // TODO clamp user width/height to display size ?
         // TODO select aspect ratio according to display ?
         break;
      }
      case IDC_SIZELIST:
      {
         const bool fullscreen = IsDlgButtonChecked(IDC_EXCLUSIVE_FULLSCREEN) == BST_CHECKED;
         if (fullscreen)
         {
            LRESULT index = SendDlgItemMessage(IDC_SIZELIST, LB_GETCURSEL, 0, 0);
            if (index == LB_ERR)
               index = 0;
            if (index >= 0 && (size_t)index < m_allVideoModes.size())
            {
               const VPX::Window::VideoMode* const pvm = &m_allVideoModes[index];
               SetDlgItemInt(IDC_WIDTH_EDIT, pvm->width, FALSE);
               SetDlgItemInt(IDC_HEIGHT_EDIT, pvm->height, FALSE);
               SendDlgItemMessage(IDC_AR_COMBO, CB_SETCURSEL, 0, 0);
               for (size_t i = 1; i < sizeof(arFactors) / sizeof(arFactors[0]); i++)
                  if (pvm->height == (int)((float)pvm->width / arFactors[i]))
                     SendDlgItemMessage(IDC_AR_COMBO, CB_SETCURSEL, i, 0);
            }
         }
         break;
      }
      case IDC_AR_COMBO:
      case IDC_WIDTH_EDIT:
      {
         UpdateDisplayHeightFromWidth();
         break;
      }
      case IDC_ENABLE_AO:
      {
         const size_t checked = IsDlgButtonChecked(IDC_ENABLE_AO);
         GetDlgItem(IDC_DYNAMIC_AO).EnableWindow(checked ? TRUE : FALSE);
         break;
      }
      case IDC_3D_STEREO:
      case IDC_FAKE_STEREO:
      {
         LRESULT stereo3D = SendDlgItemMessage(IDC_3D_STEREO, CB_GETCURSEL, 0, 0);
         bool fakeStereo = true;
         #if defined(ENABLE_OPENGL) || defined(ENABLE_BGFX)
         fakeStereo = IsDlgButtonChecked(IDC_FAKE_STEREO);
         #endif
         SetDlgItemText(IDC_3D_STEREO_MS_LABEL, fakeStereo ? "Parallax Separation" : "Eye Separation (mm)");
         if (stereo3D == CB_ERR)
            stereo3D = STEREO_OFF;
         if (stereo3D == STEREO_OFF)
         {
            GetDlgItem(IDC_FAKE_STEREO).EnableWindow(false);
            GetDlgItem(IDC_3D_STEREO_Y).EnableWindow(false);
            GetDlgItem(IDC_3D_STEREO_OFS).EnableWindow(false);
            GetDlgItem(IDC_3D_STEREO_ZPD).EnableWindow(false);
            GetDlgItem(IDC_3D_STEREO_MS).EnableWindow(false);
            GetDlgItem(IDC_3D_STEREO_ES).EnableWindow(false);
            GetDlgItem(IDC_3D_STEREO_BRIGHTNESS).EnableWindow(false);
            GetDlgItem(IDC_3D_STEREO_DESATURATION).EnableWindow(false);
            GetDlgItem(IDC_3D_STEREO_ANAGLYPH_FILTER).EnableWindow(false);
         }
         else if (Is3DTVStereoMode(stereo3D))
         {
            #if defined(ENABLE_OPENGL) || defined(ENABLE_BGFX)
            GetDlgItem(IDC_FAKE_STEREO).EnableWindow(true);
            #endif
            GetDlgItem(IDC_3D_STEREO_Y).EnableWindow(fakeStereo);
            GetDlgItem(IDC_3D_STEREO_OFS).EnableWindow(fakeStereo);
            GetDlgItem(IDC_3D_STEREO_ZPD).EnableWindow(fakeStereo);
            GetDlgItem(IDC_3D_STEREO_MS).ShowWindow(fakeStereo);
            GetDlgItem(IDC_3D_STEREO_ES).ShowWindow(!fakeStereo);
            GetDlgItem(IDC_3D_STEREO_MS).EnableWindow(fakeStereo);
            GetDlgItem(IDC_3D_STEREO_ES).EnableWindow(!fakeStereo);
            GetDlgItem(IDC_3D_STEREO_BRIGHTNESS).EnableWindow(false);
            GetDlgItem(IDC_3D_STEREO_DESATURATION).EnableWindow(false);
            GetDlgItem(IDC_3D_STEREO_ANAGLYPH_FILTER).EnableWindow(false);
         }
         else if (IsAnaglyphStereoMode(stereo3D))
         {
            #if defined(ENABLE_OPENGL) || defined(ENABLE_BGFX)
            GetDlgItem(IDC_FAKE_STEREO).EnableWindow(true);
            #endif
            GetDlgItem(IDC_3D_STEREO_Y).EnableWindow(fakeStereo);
            GetDlgItem(IDC_3D_STEREO_OFS).EnableWindow(fakeStereo);
            GetDlgItem(IDC_3D_STEREO_ZPD).EnableWindow(fakeStereo);
            GetDlgItem(IDC_3D_STEREO_MS).ShowWindow(fakeStereo);
            GetDlgItem(IDC_3D_STEREO_ES).ShowWindow(!fakeStereo);
            GetDlgItem(IDC_3D_STEREO_MS).EnableWindow(fakeStereo);
            GetDlgItem(IDC_3D_STEREO_ES).EnableWindow(!fakeStereo);
            GetDlgItem(IDC_3D_STEREO_BRIGHTNESS).EnableWindow(true);
            GetDlgItem(IDC_3D_STEREO_DESATURATION).EnableWindow(true);
            GetDlgItem(IDC_3D_STEREO_ANAGLYPH_FILTER).EnableWindow(true);
            LRESULT glassesIndex = stereo3D - STEREO_ANAGLYPH_1;
            int anaglyphFilter = GetEditedSettings().LoadValueWithDefault(Settings::Player, "Anaglyph"s.append(std::to_string(glassesIndex + 1)).append("Filter"s), 4);
            SendMessage(GetDlgItem(IDC_3D_STEREO_ANAGLYPH_FILTER), CB_SETCURSEL, anaglyphFilter, 0);
         }
         break;
      }
      default:
         return FALSE;
   }
   return TRUE;
}

void VideoOptionsDialog::OnClose()
{
   SendDlgItemMessage(IDC_SIZELIST, LB_RESETCONTENT, 0, 0);
   CDialog::OnClose();
}
