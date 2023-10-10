#include "stdafx.h"
#include "resource.h"
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
      SendMessage(GetDlgItem(IDC_EXCLUSIVE_FULLSCREEN).GetHwnd(), BM_SETCHECK, fullscreen ? BST_CHECKED : BST_UNCHECKED, 0);
      SendMessage(GetDlgItem(IDC_WINDOWED).GetHwnd(), BM_SETCHECK, fullscreen ? BST_UNCHECKED : BST_CHECKED, 0);
      OnCommand(IDC_EXCLUSIVE_FULLSCREEN, 0L); // Force UI update
   }
   if(profile < 2)
   {
      SendMessage(GetDlgItem(IDC_10BIT_VIDEO).GetHwnd(), BM_SETCHECK, false ? BST_CHECKED : BST_UNCHECKED, 0);
   }

   SendMessage(GetDlgItem(IDC_MAX_TEXTURE_COMBO).GetHwnd(), CB_SETCURSEL, 7, 0);
   SendMessage(GetDlgItem(IDC_GLOBAL_TRAIL_CHECK).GetHwnd(), BM_SETCHECK, true ? BST_CHECKED : BST_UNCHECKED, 0);
   SendMessage(GetDlgItem(IDC_GLOBAL_DISABLE_LIGHTING_BALLS).GetHwnd(), BM_SETCHECK, false ? BST_CHECKED : BST_UNCHECKED, 0);
   SetDlgItemInt(IDC_MAX_FPS, 0, FALSE);
   SetDlgItemInt(IDC_MAX_PRE_FRAMES, 0, FALSE);

   if(profile == 0)
   {
      constexpr float ballAspecRatioOffsetX = 0.0f;
      char tmp[256];
      sprintf_s(tmp, sizeof(tmp), "%f", ballAspecRatioOffsetX);
      SetDlgItemText(IDC_CORRECTION_X, tmp);
      constexpr float ballAspecRatioOffsetY = 0.0f;
      sprintf_s(tmp, sizeof(tmp), "%f", ballAspecRatioOffsetY);
      SetDlgItemText(IDC_CORRECTION_Y, tmp);
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

   SendMessage(GetDlgItem(IDC_SUPER_SAMPLING_COMBO).GetHwnd(), TBM_SETPOS, TRUE, getBestMatchingAAfactorIndex(1.0f));
   SendMessage(GetDlgItem(IDC_MSAA_COMBO).GetHwnd(), TBM_SETPOS, TRUE, 1);

   SendMessage(GetDlgItem(IDC_DYNAMIC_DN).GetHwnd(), BM_SETCHECK, false ? BST_CHECKED : BST_UNCHECKED, 0);
   
   SendMessage(GetDlgItem(IDC_MAX_AO_COMBO).GetHwnd(), CB_SETCURSEL,profile == 2 ? 2 : 1, 0);

   SendMessage(GetDlgItem(IDC_GLOBAL_SSREFLECTION_CHECK).GetHwnd(), BM_SETCHECK, profile == 2 ? BST_CHECKED : BST_UNCHECKED, 0);
   SendMessage(GetDlgItem(IDC_MAX_REFLECTION_COMBO).GetHwnd(), CB_SETCURSEL, profile == 1 ? RenderProbe::REFL_STATIC_N_BALLS : RenderProbe::REFL_DYNAMIC, 0);

   if (profile == 0)
   {
      SendMessage(GetDlgItem(IDC_OVERWRITE_BALL_IMAGE_CHECK).GetHwnd(), BM_SETCHECK, false ? BST_CHECKED : BST_UNCHECKED, 0);
      SetDlgItemText(IDC_BALL_IMAGE_EDIT, "");
      SetDlgItemText(IDC_BALL_DECAL_EDIT, "");
      if (true)
      {
         ::EnableWindow(GetDlgItem(IDC_BROWSE_BALL_IMAGE).GetHwnd(), FALSE);
         ::EnableWindow(GetDlgItem(IDC_BROWSE_BALL_DECAL).GetHwnd(), FALSE);
         ::EnableWindow(GetDlgItem(IDC_BALL_IMAGE_EDIT).GetHwnd(), FALSE);
         ::EnableWindow(GetDlgItem(IDC_BALL_DECAL_EDIT).GetHwnd(), FALSE);
      }
   }

   SendMessage(GetDlgItem(IDC_POST_PROCESS_COMBO).GetHwnd(), CB_SETCURSEL, profile == 1 ? Disabled : (profile == 2 ? Quality_FXAA : Standard_FXAA), 0);
   SendMessage(GetDlgItem(IDC_SHARPEN_COMBO).GetHwnd(), CB_SETCURSEL, profile != 2 ? 0 : 2, 0);
   SendMessage(GetDlgItem(IDC_SCALE_FX_DMD).GetHwnd(), BM_SETCHECK, false ? BST_CHECKED : BST_UNCHECKED, 0);

   if (profile == 0)
   {
      SendMessage(GetDlgItem(IDC_BG_SET).GetHwnd(), CB_SETCURSEL, 0, 0);
      SendMessage(GetDlgItem(IDC_3D_STEREO).GetHwnd(), CB_SETCURSEL, 0, 0);
      SendMessage(GetDlgItem(IDC_3D_STEREO_Y).GetHwnd(), BM_SETCHECK, false ? BST_CHECKED : BST_UNCHECKED, 0);
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
      SendMessage(GetDlgItem(IDC_USE_NVIDIA_API_CHECK).GetHwnd(), BM_SETCHECK, false ? BST_CHECKED : BST_UNCHECKED, 0);
   }

   SendMessage(GetDlgItem(IDC_BLOOM_OFF).GetHwnd(), BM_SETCHECK, false ? BST_CHECKED : BST_UNCHECKED, 0);

   if (profile == 0)
   {
      SendMessage(GetDlgItem(IDC_DISABLE_DWM).GetHwnd(), BM_SETCHECK, false ? BST_CHECKED : BST_UNCHECKED, 0);
   }

   SendMessage(GetDlgItem(IDC_FORCE_ANISO).GetHwnd(), BM_SETCHECK, profile != 1 ? BST_CHECKED : BST_UNCHECKED, 0);
   SendMessage(GetDlgItem(IDC_TEX_COMPRESS).GetHwnd(), BM_SETCHECK, false ? BST_CHECKED : BST_UNCHECKED, 0);
   SendMessage(GetDlgItem(IDC_SOFTWARE_VP).GetHwnd(), BM_SETCHECK, false ? BST_CHECKED : BST_UNCHECKED, 0);
   SendMessage(GetDlgItem(IDC_ARASlider).GetHwnd(), TBM_SETPOS, TRUE, profile == 1 ? 5 : 10);

   if (profile == 0)
   {
      SendMessage(GetDlgItem(IDC_StretchYes).GetHwnd(), BM_SETCHECK, BST_UNCHECKED, 0);
      SendMessage(GetDlgItem(IDC_StretchMonitor).GetHwnd(), BM_SETCHECK, BST_UNCHECKED, 0);
      SendMessage(GetDlgItem(IDC_StretchNo).GetHwnd(), BM_SETCHECK, BST_CHECKED, 0);
      SendMessage(GetDlgItem(IDC_DISPLAY_ID).GetHwnd(), CB_SETCURSEL, 0, 0);
   }
}

void VideoOptionsDialog::UpdateFullscreenModesList()
{
   const HWND hwndList = GetDlgItem(IDC_SIZELIST).GetHwnd();
   const int display = (int)SendMessage(GetDlgItem(IDC_DISPLAY_ID).GetHwnd(), CB_GETCURSEL, 0, 0);
   if (display < 0)
      return;
   EnumerateDisplayModes(display, m_allVideoModes);
   int screenwidth, screenheight, x, y;
   getDisplaySetupByID(display, x, y, screenwidth, screenheight);

   const int depthcur = GetEditedSettings().LoadValueWithDefault(Settings::Player, "ColorDepth"s, 32);
   const int refreshrate = GetEditedSettings().LoadValueWithDefault(Settings::Player, "RefreshRate"s, 0);
   const int widthcur = GetEditedSettings().LoadValueWithDefault(Settings::Player, "Width"s, -1);
   const int heightcur = GetEditedSettings().LoadValueWithDefault(Settings::Player, "Height"s, -1);
   VideoMode curSelMode;
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
   if (SendMessage(hwndList, LB_GETCURSEL, 0, 0) == -1)
      SendMessage(hwndList, LB_SETCURSEL, 0, 0);
   SendMessage(hwndList, WM_SETREDRAW, TRUE, 0);
}

BOOL VideoOptionsDialog::OnInitDialog()
{
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
      AddToolTip("None: No synchronization.\r\nVertical Sync: Synchronize on video sync which avoids video tearing, but has higher input latency.\r\nAdaptive Sync: Synchronize on video sync, except for late frames (below target FPS), also has higher input latency.\r\nFrame Pacing: Targets real time simulation with low input- and video-latency (also dynamically adjusts framerate).", hwndDlg, toolTipHwnd, GetDlgItem(IDC_VIDEO_SYNC_MODE).GetHwnd());
      AddToolTip("Limit the FPS to the given value (energy saving/less heat, framerate stability), 0 will disable it", hwndDlg, toolTipHwnd, GetDlgItem(IDC_MAX_FPS).GetHwnd());
      AddToolTip("Leave at 0 if you have enabled 'Low Latency' or 'Anti Lag' settings in the graphics driver.\r\nOtherwise experiment with 1 or 2 for a chance of lag reduction at the price of a bit of framerate.", hwndDlg, toolTipHwnd, GetDlgItem(IDC_MAX_PRE_FRAMES).GetHwnd());
      AddToolTip("If played in cabinet mode and you get an egg shaped ball activate this.\r\nFor screen ratios other than 16:9 you may have to adjust the offsets.\r\nNormally you have to set the Y offset (around 1.5) but you have to experiment.", hwndDlg, toolTipHwnd, GetDlgItem(IDC_StretchMonitor).GetHwnd());
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

      AddToolTip("Physical width of the display area of the screen in centimeters, in landscape orientation (width > height).\r\n\r\nThis is needed to get correct size when using 'Window' mode for the camera.", hwndDlg, toolTipHwnd, GetDlgItem(IDC_SCREEN_WIDTH).GetHwnd());
      AddToolTip("Physical height of the display area of the screen in centimeters, in landscape orientation (width > height).\r\n\r\nThis is needed to get correct size when using 'Window' mode for the camera.", hwndDlg, toolTipHwnd, GetDlgItem(IDC_SCREEN_HEIGHT).GetHwnd());
   }

   SendMessage(GetDlgItem(IDC_APPLICATION_SETTINGS).GetHwnd(), BM_SETCHECK, BST_CHECKED, 0);
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
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Adaptive Sync");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Frame Pacing");
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
   SendMessage(hwndARASlider, TBM_SETTHUMBLENGTH, 5, 0);

   // Disable unsupported features in UI
#ifdef ENABLE_SDL
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

   OnCommand(IDC_APPLICATION_SETTINGS, 0L);

   return TRUE;
}

Settings& VideoOptionsDialog::GetEditedSettings()
{
   if (g_pvp->m_ptableActive && SendMessage(GetDlgItem(IDC_TABLE_OVERRIDE).GetHwnd(), BM_GETCHECK, 0, 0) == BST_CHECKED)
      return g_pvp->m_ptableActive->m_settings;
   SendMessage(GetDlgItem(IDC_APPLICATION_SETTINGS).GetHwnd(), BM_SETCHECK, BST_CHECKED, 0);
   return g_pvp->m_settings;
}

void VideoOptionsDialog::LoadSettings()
{
   HWND hwnd;
   Settings& settings = GetEditedSettings();

   m_initialMaxTexDim = settings.LoadValueWithDefault(Settings::Player, "MaxTexDimension"s, 0);
   const int maxTexDim = ((1023 + m_initialMaxTexDim) / 1024) - 1;
   SendMessage(GetDlgItem(IDC_MAX_TEXTURE_COMBO).GetHwnd(), CB_SETCURSEL, maxTexDim < 0 ? 7 : maxTexDim, 0);

   const bool trail = settings.LoadValueWithDefault(Settings::Player, "BallTrail"s, true);
   SendMessage(GetDlgItem(IDC_GLOBAL_TRAIL_CHECK).GetHwnd(), BM_SETCHECK, trail ? BST_CHECKED : BST_UNCHECKED, 0);

   const bool disableLighting = settings.LoadValueWithDefault(Settings::Player, "DisableLightingForBalls"s, false);
   SendMessage(GetDlgItem(IDC_GLOBAL_DISABLE_LIGHTING_BALLS).GetHwnd(), BM_SETCHECK, disableLighting ? BST_CHECKED : BST_UNCHECKED, 0);

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
   if (syncMode == VideoSyncMode::VSM_INVALID)
      syncMode = VideoSyncMode::VSM_FRAME_PACING;
   SetDlgItemInt(IDC_MAX_FPS, maxFPS, FALSE);

   hwnd = GetDlgItem(IDC_VIDEO_SYNC_MODE).GetHwnd();
   SendMessage(hwnd, CB_SETCURSEL, syncMode, 0);

   const int maxPrerenderedFrames = settings.LoadValueWithDefault(Settings::Player, "MaxPrerenderedFrames"s, 0);
   SetDlgItemInt(IDC_MAX_PRE_FRAMES, maxPrerenderedFrames, FALSE);
   #ifdef ENABLE_SDL
   GetDlgItem(IDC_MAX_PRE_FRAMES).EnableWindow(false); // OpenGL does not support this option
   #endif

   char tmp[256];

   const float ballAspecRatioOffsetX = settings.LoadValueWithDefault(Settings::Player, "BallCorrectionX"s, 0.f);
   sprintf_s(tmp, sizeof(tmp), "%f", ballAspecRatioOffsetX);
   SetDlgItemText(IDC_CORRECTION_X, tmp);

   const float ballAspecRatioOffsetY = settings.LoadValueWithDefault(Settings::Player, "BallCorrectionY"s, 0.f);
   sprintf_s(tmp, sizeof(tmp), "%f", ballAspecRatioOffsetY);
   SetDlgItemText(IDC_CORRECTION_Y, tmp);

   const float latitude = settings.LoadValueWithDefault(Settings::Player, "Latitude"s, 52.52f);
   sprintf_s(tmp, sizeof(tmp), "%f", latitude);
   SetDlgItemText(IDC_DN_LATITUDE, tmp);

   const float longitude = settings.LoadValueWithDefault(Settings::Player, "Longitude"s, 13.37f);
   sprintf_s(tmp, sizeof(tmp), "%f", longitude);
   SetDlgItemText(IDC_DN_LONGITUDE, tmp);

   const float nudgeStrength = settings.LoadValueWithDefault(Settings::Player, "NudgeStrength"s, 2e-2f);
   sprintf_s(tmp, sizeof(tmp), "%f", nudgeStrength);
   SetDlgItemText(IDC_NUDGE_STRENGTH, tmp);

   hwnd = GetDlgItem(IDC_SUPER_SAMPLING_COMBO).GetHwnd();
   const float AAfactor = settings.LoadValueWithDefault(Settings::Player, "AAFactor"s, settings.LoadValueWithDefault(Settings::Player, "USEAA"s, false) ? 1.5f : 1.0f);
   SendMessage(hwnd, CB_SETCURSEL, getBestMatchingAAfactorIndex(AAfactor), 0);

   hwnd = GetDlgItem(IDC_MSAA_COMBO).GetHwnd();
   const int MSAASamples = settings.LoadValueWithDefault(Settings::Player, "MSAASamples"s, 1);
   const int CurrMSAAPos = static_cast<const int>(std::find(MSAASamplesOpts, MSAASamplesOpts + (sizeof(MSAASamplesOpts) / sizeof(MSAASamplesOpts[0])), MSAASamples) - MSAASamplesOpts);
   SendMessage(hwnd, CB_SETCURSEL, CurrMSAAPos, 0);

   const int useDN = settings.LoadValueWithDefault(Settings::Player, "DynamicDayNight"s, 0);
   SendMessage(GetDlgItem(IDC_DYNAMIC_DN).GetHwnd(), BM_SETCHECK, (useDN != 0) ? BST_CHECKED : BST_UNCHECKED, 0);

   const bool disableAO = settings.LoadValueWithDefault(Settings::Player, "DisableAO"s, false);
   const bool dynAO = settings.LoadValueWithDefault(Settings::Player, "DynamicAO"s, true);
   hwnd = GetDlgItem(IDC_MAX_AO_COMBO).GetHwnd();
   SendMessage(hwnd, CB_SETCURSEL, disableAO ? 0 : dynAO ? 2 : 1, 0);

   const bool ssreflection = settings.LoadValueWithDefault(Settings::Player, "SSRefl"s, false);
   SendMessage(GetDlgItem(IDC_GLOBAL_SSREFLECTION_CHECK).GetHwnd(), BM_SETCHECK, ssreflection ? BST_CHECKED : BST_UNCHECKED, 0);

   hwnd = GetDlgItem(IDC_MAX_REFLECTION_COMBO).GetHwnd();
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
   SendMessage(hwnd, CB_SETCURSEL, maxReflection, 0);

   const bool overwiteBallImage = settings.LoadValueWithDefault(Settings::Player, "OverwriteBallImage"s, false);
   SendMessage(GetDlgItem(IDC_OVERWRITE_BALL_IMAGE_CHECK).GetHwnd(), BM_SETCHECK, overwiteBallImage ? BST_CHECKED : BST_UNCHECKED, 0);

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
      ::EnableWindow(GetDlgItem(IDC_BROWSE_BALL_IMAGE).GetHwnd(), FALSE);
      ::EnableWindow(GetDlgItem(IDC_BROWSE_BALL_DECAL).GetHwnd(), FALSE);
      ::EnableWindow(GetDlgItem(IDC_BALL_IMAGE_EDIT).GetHwnd(), FALSE);
      ::EnableWindow(GetDlgItem(IDC_BALL_DECAL_EDIT).GetHwnd(), FALSE);
   }

   const int fxaa = settings.LoadValueWithDefault(Settings::Player, "FXAA"s, (int)Standard_FXAA);
   hwnd = GetDlgItem(IDC_POST_PROCESS_COMBO).GetHwnd();
   SendMessage(hwnd, CB_SETCURSEL, fxaa, 0);

   const int sharpen = settings.LoadValueWithDefault(Settings::Player, "Sharpen"s, 0);
   hwnd = GetDlgItem(IDC_SHARPEN_COMBO).GetHwnd();
   SendMessage(hwnd, CB_SETCURSEL, sharpen, 0);

   const bool scaleFX_DMD = settings.LoadValueWithDefault(Settings::Player, "ScaleFXDMD"s, false);
   SendMessage(GetDlgItem(IDC_SCALE_FX_DMD).GetHwnd(), BM_SETCHECK, scaleFX_DMD ? BST_CHECKED : BST_UNCHECKED, 0);

   const int bgset = settings.LoadValueWithDefault(Settings::Player, "BGSet"s, 0);
   hwnd = GetDlgItem(IDC_BG_SET).GetHwnd();
   SendMessage(hwnd, CB_SETCURSEL, bgset, 0);
   // FIXME
   SendMessage(GetDlgItem(IDC_BG_SET).GetHwnd(), BM_SETCHECK, (bgset != 0) ? BST_CHECKED : BST_UNCHECKED, 0);

   bool fakeStereo = true;
   #ifdef ENABLE_SDL
   fakeStereo = settings.LoadValueWithDefault(Settings::Player, "Stereo3DFake"s, false);
   #else
   ::EnableWindow(GetDlgItem(IDC_FAKE_STEREO).GetHwnd(), FALSE);
   #endif
   SendMessage(GetDlgItem(IDC_FAKE_STEREO).GetHwnd(), BM_SETCHECK, fakeStereo ? BST_CHECKED : BST_UNCHECKED, 0);

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
   SendMessage(GetDlgItem(IDC_3D_STEREO_Y).GetHwnd(), BM_SETCHECK, stereo3DY ? BST_CHECKED : BST_UNCHECKED, 0);

   const float stereo3DOfs = settings.LoadValueWithDefault(Settings::Player, "Stereo3DOffset"s, 0.f);
   sprintf_s(tmp, sizeof(tmp), "%f", stereo3DOfs);
   SetDlgItemText(IDC_3D_STEREO_OFS, tmp);

   const float stereo3DMS = settings.LoadValueWithDefault(Settings::Player, "Stereo3DMaxSeparation"s, 0.03f);
   const float stereo3DES = settings.LoadValueWithDefault(Settings::Player, "Stereo3DEyeSeparation"s, 63.0f);
   sprintf_s(tmp, sizeof(tmp), "%f", stereo3DMS);
   SetDlgItemText(IDC_3D_STEREO_MS, tmp);
   sprintf_s(tmp, sizeof(tmp), "%f", stereo3DES);
   SetDlgItemText(IDC_3D_STEREO_ES, tmp);

   const float stereo3DZPD = settings.LoadValueWithDefault(Settings::Player, "Stereo3DZPD"s, 0.5f);
   sprintf_s(tmp, sizeof(tmp), "%f", stereo3DZPD);
   SetDlgItemText(IDC_3D_STEREO_ZPD, tmp);

   const bool bamHeadtracking = settings.LoadValueWithDefault(Settings::Player, "BAMheadTracking"s, false);
   SendMessage(GetDlgItem(IDC_HEADTRACKING).GetHwnd(), BM_SETCHECK, bamHeadtracking ? BST_CHECKED : BST_UNCHECKED, 0);

   const float stereo3DBrightness = settings.LoadValueWithDefault(Settings::Player, "Stereo3DBrightness"s, 1.0f);
   sprintf_s(tmp, sizeof(tmp), "%f", stereo3DBrightness);
   SetDlgItemText(IDC_3D_STEREO_BRIGHTNESS, tmp);

   const float stereo3DSaturation = settings.LoadValueWithDefault(Settings::Player, "stereo3DSaturation"s, 1.0f);
   sprintf_s(tmp, sizeof(tmp), "%f", stereo3DSaturation);
   SetDlgItemText(IDC_3D_STEREO_DESATURATION, tmp);

   const bool disableDWM = settings.LoadValueWithDefault(Settings::Player, "DisableDWM"s, false);
   GetDlgItem(IDC_DISABLE_DWM).EnableWindow(IsWindowsVistaOr7());
   SendMessage(GetDlgItem(IDC_DISABLE_DWM).GetHwnd(), BM_SETCHECK, disableDWM ? BST_CHECKED : BST_UNCHECKED, 0);
   GetDlgItem(IDC_DISABLE_DWM).EnableWindow(IsWindowsVistaOr7()); // DWM may not be disabled on Windows 8+

   const bool nvidiaApi = settings.LoadValueWithDefault(Settings::Player, "UseNVidiaAPI"s, false);
   SendMessage(GetDlgItem(IDC_USE_NVIDIA_API_CHECK).GetHwnd(), BM_SETCHECK, nvidiaApi ? BST_CHECKED : BST_UNCHECKED, 0);

   const bool bloomOff = settings.LoadValueWithDefault(Settings::Player, "ForceBloomOff"s, false);
   SendMessage(GetDlgItem(IDC_BLOOM_OFF).GetHwnd(), BM_SETCHECK, bloomOff ? BST_CHECKED : BST_UNCHECKED, 0);

   const bool forceAniso = settings.LoadValueWithDefault(Settings::Player, "ForceAnisotropicFiltering"s, true);
   SendMessage(GetDlgItem(IDC_FORCE_ANISO).GetHwnd(), BM_SETCHECK, forceAniso ? BST_CHECKED : BST_UNCHECKED, 0);

   const bool compressTextures = settings.LoadValueWithDefault(Settings::Player, "CompressTextures"s, false);
   SendMessage(GetDlgItem(IDC_TEX_COMPRESS).GetHwnd(), BM_SETCHECK, compressTextures ? BST_CHECKED : BST_UNCHECKED, 0);

   const bool softwareVP = settings.LoadValueWithDefault(Settings::Player, "SoftwareVertexProcessing"s, false);
   SendMessage(GetDlgItem(IDC_SOFTWARE_VP).GetHwnd(), BM_SETCHECK, softwareVP ? BST_CHECKED : BST_UNCHECKED, 0);

   const bool video10bit = settings.LoadValueWithDefault(Settings::Player, "Render10Bit"s, false);
   SendMessage(GetDlgItem(IDC_10BIT_VIDEO).GetHwnd(), BM_SETCHECK, video10bit ? BST_CHECKED : BST_UNCHECKED, 0);

   //const int depthcur = settings.LoadValueWithDefault(Settings::Player, "ColorDepth"s, 32);
   //const int refreshrate = settings.LoadValueWithDefault(Settings::Player, "RefreshRate"s, 0);

   int display;
   hr = settings.LoadValue(Settings::Player, "Display"s, display);
   vector<DisplayConfig> displays;
   getDisplayList(displays);
   if ((!hr) || ((int)displays.size() <= display))
      display = -1;
   hwnd = GetDlgItem(IDC_DISPLAY_ID).GetHwnd();
   SendMessage(hwnd, WM_SETREDRAW, FALSE, 0); // to speed up adding the entries :/
   SendMessage(hwnd, CB_RESETCONTENT, 0, 0);
   for (vector<DisplayConfig>::iterator dispConf = displays.begin(); dispConf != displays.end(); ++dispConf)
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
   SendMessage(GetDlgItem(fullscreen ? IDC_EXCLUSIVE_FULLSCREEN : IDC_WINDOWED).GetHwnd(), BM_SETCHECK, BST_CHECKED, 0);
   OnCommand(IDC_EXCLUSIVE_FULLSCREEN, 0L); // Force UI update

   const int widthcur = settings.LoadValueWithDefault(Settings::Player, "Width"s, -1);
   const int heightcur = settings.LoadValueWithDefault(Settings::Player, "Height"s, -1);
   SetDlgItemInt(IDC_WIDTH_EDIT, widthcur, FALSE);
   SetDlgItemInt(IDC_HEIGHT_EDIT, heightcur, FALSE);

   hwnd = GetDlgItem(IDC_AR_COMBO).GetHwnd();
   SendMessage(hwnd, CB_SETCURSEL, 0, 0);
   for (int i = 1; i < sizeof(arFactors) / sizeof(arFactors[0]); i++)
      if (heightcur == (int)(widthcur / arFactors[i]))
         SendMessage(hwnd, CB_SETCURSEL, i, 0);

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
   const HWND hwndARASlider = GetDlgItem(IDC_ARASlider).GetHwnd();
   SendMessage(hwndARASlider, TBM_SETRANGE, fTrue, MAKELONG(0, 10));
   SendMessage(hwndARASlider, TBM_SETTICFREQ, 1, 0);
   SendMessage(hwndARASlider, TBM_SETLINESIZE, 0, 1);
   SendMessage(hwndARASlider, TBM_SETPAGESIZE, 0, 1);
   SendMessage(hwndARASlider, TBM_SETTHUMBLENGTH, 5, 0);
   SendMessage(hwndARASlider, TBM_SETPOS, TRUE, alphaRampsAccuracy);

   const int ballStretchMode = settings.LoadValueWithDefault(Settings::Player, "BallStretchMode"s, 0);
   switch (ballStretchMode)
   {
      default:
      case 0:  SendMessage(GetDlgItem(IDC_StretchNo).GetHwnd(), BM_SETCHECK, BST_CHECKED, 0);      break;
      case 1:  SendMessage(GetDlgItem(IDC_StretchYes).GetHwnd(), BM_SETCHECK, BST_CHECKED, 0);     break;
      case 2:  SendMessage(GetDlgItem(IDC_StretchMonitor).GetHwnd(), BM_SETCHECK, BST_CHECKED, 0); break;
   }
}

template <typename T> void VideoOptionsDialog::SaveOrDelete(const bool saveAll, const string& key, T value, T defValue)
{
   Settings& settings = GetEditedSettings();
   if (saveAll || value != g_pvp->m_settings.LoadValueWithDefault(Settings::Player, key, defValue))
      settings.SaveValue(Settings::Player, key, value);
   else
      settings.DeleteValue(Settings::Player, key);
}

void VideoOptionsDialog::SaveSettings(const bool saveAll)
{
   BOOL nothing = 0;
   const Settings& appSettings = g_pvp->m_settings;
   Settings& settings = GetEditedSettings();

   const bool fullscreen = SendMessage(GetDlgItem(IDC_EXCLUSIVE_FULLSCREEN).GetHwnd(), BM_GETCHECK, 0, 0) == BST_CHECKED;
   if (fullscreen)
   {
      const size_t index = SendMessage(GetDlgItem(IDC_SIZELIST).GetHwnd(), LB_GETCURSEL, 0, 0);
      const VideoMode* const pvm = &m_allVideoModes[index];
      SaveOrDelete(saveAll, "Width"s, pvm->width, -1);
      SaveOrDelete(saveAll, "Height"s, pvm->height, -1);
      SaveOrDelete(saveAll, "ColorDepth"s, pvm->depth, -1);
      SaveOrDelete(saveAll, "RefreshRate"s, pvm->refreshrate, -1);
   }
   else
   {
      LRESULT arMode = SendMessage(GetDlgItem(IDC_AR_COMBO).GetHwnd(), CB_GETCURSEL, 0, 0);
      if (arMode == LB_ERR)
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
         SaveOrDelete(saveAll, "Width"s, width, -1);
         SaveOrDelete(saveAll, "Height"s, height, -1);
      }
   }
   SaveOrDelete(saveAll, "Display"s, (int)SendMessage(GetDlgItem(IDC_DISPLAY_ID).GetHwnd(), CB_GETCURSEL, 0, 0), -1);
   SaveOrDelete(saveAll, "FullScreen"s, fullscreen, IsWindows10_1803orAbove());
   SaveOrDelete(saveAll, "ScreenWidth"s, GetDlgItemText(IDC_SCREEN_WIDTH).GetString(), "0.0"s);
   SaveOrDelete(saveAll, "ScreenHeight"s, GetDlgItemText(IDC_SCREEN_HEIGHT).GetString(), "0.0"s);
   SaveOrDelete(saveAll, "ScreenInclination"s, GetDlgItemText(IDC_SCREEN_INCLINATION).GetString(), "0.0"s);
   SaveOrDelete(saveAll, "ScreenPlayerX"s, GetDlgItemText(IDC_SCREEN_PLAYERX).GetString(), "0.0"s);
   SaveOrDelete(saveAll, "ScreenPlayerY"s, GetDlgItemText(IDC_SCREEN_PLAYERY).GetString(), "0.0"s);
   SaveOrDelete(saveAll, "ScreenPlayerZ"s, GetDlgItemText(IDC_SCREEN_PLAYERZ).GetString(), "0.0"s);
   SaveOrDelete(saveAll, "Render10Bit"s, SendMessage(GetDlgItem(IDC_10BIT_VIDEO).GetHwnd(), BM_GETCHECK, 0, 0) == BST_CHECKED, false);
   LRESULT maxTexDim = SendMessage(GetDlgItem(IDC_MAX_TEXTURE_COMBO).GetHwnd(), CB_GETCURSEL, 0, 0);
   if (maxTexDim == LB_ERR)
      maxTexDim = 7;
   maxTexDim = maxTexDim == 7 ? 0 : (1024 * (maxTexDim + 1));
   SaveOrDelete(saveAll, "MaxTexDimension"s, (int)maxTexDim, 0);
   if (m_initialMaxTexDim != maxTexDim)
      MessageBox("You have changed the maximum texture size.\n\nThis change will only take effect after reloading the tables.", "Reload tables", MB_ICONWARNING);
   SaveOrDelete(saveAll, "BallTrail"s, SendMessage(GetDlgItem(IDC_GLOBAL_TRAIL_CHECK).GetHwnd(), BM_GETCHECK, 0, 0) == BST_CHECKED, true);
   SaveOrDelete(saveAll, "DisableLightingForBalls"s, SendMessage(GetDlgItem(IDC_GLOBAL_DISABLE_LIGHTING_BALLS).GetHwnd(), BM_GETCHECK, 0, 0) == BST_CHECKED, false);
   SaveOrDelete(saveAll, "MaxFramerate"s, (int)GetDlgItemInt(IDC_MAX_FPS, nothing, TRUE), 0);
   LRESULT syncMode = SendMessage(GetDlgItem(IDC_VIDEO_SYNC_MODE).GetHwnd(), CB_GETCURSEL, 0, 0);
   if (syncMode == LB_ERR)
      syncMode = VideoSyncMode::VSM_FRAME_PACING;
   SaveOrDelete(saveAll, "SyncMode"s, (int)syncMode, (int)VideoSyncMode::VSM_FRAME_PACING);
   SaveOrDelete(saveAll, "MaxPrerenderedFrames"s, (int)GetDlgItemInt(IDC_MAX_PRE_FRAMES, nothing, TRUE), 0);
   SaveOrDelete(saveAll, "BallCorrectionX"s, GetDlgItemText(IDC_CORRECTION_X).GetString(), "0.0"s);
   SaveOrDelete(saveAll, "BallCorrectionY"s, GetDlgItemText(IDC_CORRECTION_X).GetString(), "0.0"s);
   SaveOrDelete(saveAll, "Longitude"s, GetDlgItemText(IDC_CORRECTION_X).GetString(), "0.0"s);
   SaveOrDelete(saveAll, "Latitude"s, GetDlgItemText(IDC_CORRECTION_X).GetString(), "0.0"s);
   SaveOrDelete(saveAll, "NudgeStrength"s, GetDlgItemText(IDC_CORRECTION_X).GetString(), "0.0"s);
   LRESULT fxaa = SendMessage(GetDlgItem(IDC_POST_PROCESS_COMBO).GetHwnd(), CB_GETCURSEL, 0, 0);
   if (fxaa == LB_ERR)
      fxaa = Standard_FXAA;
   SaveOrDelete(saveAll, "FXAA"s, (int) fxaa, (int) Standard_FXAA);
   LRESULT sharpen = SendMessage(GetDlgItem(IDC_SHARPEN_COMBO).GetHwnd(), CB_GETCURSEL, 0, 0);
   if (sharpen == LB_ERR)
      sharpen = 0;
   SaveOrDelete(saveAll, "Sharpen"s, (int)sharpen, 0);
   SaveOrDelete(saveAll, "ScaleFXDMD"s, SendMessage(GetDlgItem(IDC_SCALE_FX_DMD).GetHwnd(), BM_GETCHECK, 0, 0) == BST_CHECKED, true);
   SaveOrDelete(saveAll, "BGSet"s, (int)SendMessage(GetDlgItem(IDC_BG_SET).GetHwnd(), CB_GETCURSEL, 0, 0), 0);
   // update the cached current view setup of all loaded tables since it also depends on this setting
   for (auto table : g_pvp->m_vtable)
      table->UpdateCurrentBGSet();
   LRESULT AAfactorIndex = SendMessage(GetDlgItem(IDC_SUPER_SAMPLING_COMBO).GetHwnd(), CB_GETCURSEL, 0, 0);
   if (AAfactorIndex == LB_ERR)
      AAfactorIndex = getBestMatchingAAfactorIndex(1);
   const float AAfactor = (AAfactorIndex < AAfactorCount) ? AAfactors[AAfactorIndex] : 1.0f;
   SaveOrDelete(saveAll, "USEAA"s, AAfactor > 1.0f, false);
   SaveOrDelete(saveAll, "AAFactor"s, AAfactor, 1.f);
   LRESULT MSAASamplesIndex = SendMessage(GetDlgItem(IDC_MSAA_COMBO).GetHwnd(), CB_GETCURSEL, 0, 0);
   if (MSAASamplesIndex == LB_ERR)
      MSAASamplesIndex = 0;
   const int MSAASamples = (MSAASamplesIndex < MSAASampleCount) ? MSAASamplesOpts[MSAASamplesIndex] : 1;
   SaveOrDelete(saveAll, "MSAASamples"s, MSAASamples, 1);
   SaveOrDelete(saveAll, "DynamicDayNight"s, SendMessage(GetDlgItem(IDC_DYNAMIC_DN).GetHwnd(), BM_GETCHECK, 0, 0) == BST_CHECKED, false);
   LRESULT maxAOMode = SendMessage(GetDlgItem(IDC_MAX_AO_COMBO).GetHwnd(), CB_GETCURSEL, 0, 0);
   if (maxAOMode == LB_ERR)
      maxAOMode = 2;
   SaveOrDelete(saveAll, "DisableAO"s, maxAOMode == 0, false);
   SaveOrDelete(saveAll, "DynamicAO"s, maxAOMode == 2, true);
   LRESULT maxReflectionMode = SendMessage(GetDlgItem(IDC_MAX_REFLECTION_COMBO).GetHwnd(), CB_GETCURSEL, 0, 0);
   if (maxReflectionMode == LB_ERR)
      maxReflectionMode = RenderProbe::REFL_STATIC;
   SaveOrDelete(saveAll, "PFReflection"s, (int)maxReflectionMode, (int)RenderProbe::REFL_STATIC);
   SaveOrDelete(saveAll, "SSRefl"s, SendMessage(GetDlgItem(IDC_GLOBAL_SSREFLECTION_CHECK).GetHwnd(), BM_GETCHECK, 0, 0) == BST_CHECKED, false);
   SaveOrDelete(saveAll, "ForceAnisotropicFiltering"s, SendMessage(GetDlgItem(IDC_FORCE_ANISO).GetHwnd(), BM_GETCHECK, 0, 0) == BST_CHECKED, true);
   SaveOrDelete(saveAll, "CompressTextures"s, SendMessage(GetDlgItem(IDC_TEX_COMPRESS).GetHwnd(), BM_GETCHECK, 0, 0) == BST_CHECKED, false);
   SaveOrDelete(saveAll, "SoftwareVertexProcessing"s, SendMessage(GetDlgItem(IDC_SOFTWARE_VP).GetHwnd(), BM_GETCHECK, 0, 0) == BST_CHECKED, false);
   SaveOrDelete(saveAll, "AlphaRampAccuracy"s, (int)SendMessage(GetDlgItem(IDC_ARASlider).GetHwnd(), TBM_GETPOS, 0, 0), 0);
   LRESULT stereo3D = SendMessage(GetDlgItem(IDC_3D_STEREO).GetHwnd(), CB_GETCURSEL, 0, 0);
   if (stereo3D == LB_ERR)
      stereo3D = STEREO_OFF;
   SaveOrDelete(saveAll, "Stereo3D"s, (int)stereo3D, 0);
   SaveOrDelete(saveAll, "Stereo3DEnabled"s, stereo3D != STEREO_OFF, false);
   SaveOrDelete(saveAll, "Stereo3DYAxis"s, SendMessage(GetDlgItem(IDC_3D_STEREO_Y).GetHwnd(), BM_GETCHECK, 0, 0) == BST_CHECKED, false);
   SaveOrDelete(saveAll, "Stereo3DOffset"s, GetDlgItemText(IDC_3D_STEREO_OFS).GetString(), "0.0"s);
   bool fakeStereo = true;
   #ifdef ENABLE_SDL
   SaveOrDelete(saveAll, "Stereo3DFake"s, SendDlgItemMessage(IDC_FAKE_STEREO, BM_GETCHECK, 0, 0) == BST_CHECKED, false);
   #endif
   SaveOrDelete(saveAll, "Stereo3DMaxSeparation"s, GetDlgItemText(IDC_3D_STEREO_MS).GetString(), "0.0"s);
   SaveOrDelete(saveAll, "Stereo3DEyeSeparation"s, GetDlgItemText(IDC_3D_STEREO_ES).GetString(), "0.0"s);
   SaveOrDelete(saveAll, "Stereo3DZPD"s, GetDlgItemText(IDC_3D_STEREO_ZPD).GetString(), "0.0"s);
   SaveOrDelete(saveAll, "Stereo3DBrightness"s, GetDlgItemText(IDC_3D_STEREO_BRIGHTNESS).GetString(), "0.0"s);
   SaveOrDelete(saveAll, "Stereo3DSaturation"s, GetDlgItemText(IDC_3D_STEREO_DESATURATION).GetString(), "0.0"s);
   if (IsAnaglyphStereoMode(stereo3D))
   {
      LRESULT glassesIndex = stereo3D - STEREO_ANAGLYPH_1;
      LRESULT anaglyphFilter = SendMessage(GetDlgItem(IDC_3D_STEREO_ANAGLYPH_FILTER).GetHwnd(), CB_GETCURSEL, 0, 0);
      if (anaglyphFilter == LB_ERR)
         anaglyphFilter = 4;
      SaveOrDelete(saveAll, "Anaglyph"s.append(std::to_string(glassesIndex + 1)).append("Filter"s), (int)anaglyphFilter, 4);
   }
   SaveOrDelete(saveAll, "BAMheadTracking"s, SendMessage(GetDlgItem(IDC_HEADTRACKING).GetHwnd(), BM_GETCHECK, 0, 0) == BST_CHECKED, false);
   SaveOrDelete(saveAll, "DisableDWM"s, SendMessage(GetDlgItem(IDC_DISABLE_DWM).GetHwnd(), BM_GETCHECK, 0, 0) == BST_CHECKED, false);
   SaveOrDelete(saveAll, "UseNVidiaAPI"s, SendMessage(GetDlgItem(IDC_USE_NVIDIA_API_CHECK).GetHwnd(), BM_GETCHECK, 0, 0) == BST_CHECKED, false);
   SaveOrDelete(saveAll, "ForceBloomOff"s, SendMessage(GetDlgItem(IDC_BLOOM_OFF).GetHwnd(), BM_GETCHECK, 0, 0) == BST_CHECKED, false);
   const int ballStretchMode = SendMessage(GetDlgItem(IDC_StretchYes).GetHwnd(), BM_GETCHECK, 0, 0) == BST_CHECKED ? 1
            : SendMessage(GetDlgItem(IDC_StretchMonitor).GetHwnd(), BM_GETCHECK, 0, 0) == BST_CHECKED              ? 2
                                                                                                                   : 0;
   SaveOrDelete(saveAll, "BallStretchMode"s, ballStretchMode, 0);
   const bool overwriteEnabled = IsDlgButtonChecked(IDC_OVERWRITE_BALL_IMAGE_CHECK) == BST_CHECKED;
   SaveOrDelete(saveAll, "OverwriteBallImage"s, overwriteEnabled, true);
   if (overwriteEnabled)
   {
      SaveOrDelete(saveAll, "BallImage"s, GetDlgItemText(IDC_BALL_IMAGE_EDIT).GetString(), ""s);
      SaveOrDelete(saveAll, "DecalImage"s, GetDlgItemText(IDC_BALL_IMAGE_EDIT).GetString(), ""s);
   }
   else if (!saveAll)
   {
      settings.DeleteValue(Settings::Player, "BallImage"s);
      settings.DeleteValue(Settings::Player, "DecalImage"s);
   }

   settings.Save();
}

void VideoOptionsDialog::UpdateDisplayHeightFromWidth()
{
   LRESULT arMode = SendMessage(GetDlgItem(IDC_AR_COMBO).GetHwnd(), CB_GETCURSEL, 0, 0);
   if (arMode == LB_ERR)
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
   case IDC_SAVE_ALL:
   {
      SaveSettings(true);
      Close();
      break;
   }
   case IDC_TABLE_OVERRIDE:
   {
      GetDlgItem(IDC_SAVE_ALL).ShowWindow(true);
      GetDlgItem(IDOK).SetWindowText("Save overrides");
      LoadSettings();
      break;
   }
   case IDC_APPLICATION_SETTINGS:
   {
      GetDlgItem(IDC_SAVE_ALL).ShowWindow(false);
      GetDlgItem(IDOK).SetWindowText("OK");
      LoadSettings();
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
         const BOOL overwriteEnabled = (IsDlgButtonChecked(IDC_OVERWRITE_BALL_IMAGE_CHECK) == BST_CHECKED) ? TRUE : FALSE;
         ::EnableWindow(GetDlgItem(IDC_BROWSE_BALL_IMAGE).GetHwnd(), overwriteEnabled);
         ::EnableWindow(GetDlgItem(IDC_BROWSE_BALL_DECAL).GetHwnd(), overwriteEnabled);
         ::EnableWindow(GetDlgItem(IDC_BALL_IMAGE_EDIT).GetHwnd(), overwriteEnabled);
         ::EnableWindow(GetDlgItem(IDC_BALL_DECAL_EDIT).GetHwnd(), overwriteEnabled);
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
         const bool fullscreen = SendMessage(GetDlgItem(IDC_EXCLUSIVE_FULLSCREEN).GetHwnd(), BM_GETCHECK, 0, 0) == BST_CHECKED;
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
         const bool fullscreen = SendMessage(GetDlgItem(IDC_EXCLUSIVE_FULLSCREEN).GetHwnd(), BM_GETCHECK, 0, 0) == BST_CHECKED;
         if (fullscreen)
         {
            const size_t index = SendMessage(GetDlgItem(IDC_SIZELIST).GetHwnd(), LB_GETCURSEL, 0, 0);
            const VideoMode* const pvm = &m_allVideoModes[index];
            SetDlgItemInt(IDC_WIDTH_EDIT, pvm->width, FALSE);
            SetDlgItemInt(IDC_HEIGHT_EDIT, pvm->height, FALSE);
            SendMessage(GetDlgItem(IDC_AR_COMBO).GetHwnd(), CB_SETCURSEL, 0, 0);
            for (int i = 1; i < sizeof(arFactors) / sizeof(arFactors[0]); i++)
               if (pvm->height == (int)(pvm->width / arFactors[i]))
                  SendMessage(GetDlgItem(IDC_AR_COMBO).GetHwnd(), CB_SETCURSEL, i, 0);
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
         const size_t checked = SendDlgItemMessage(IDC_ENABLE_AO, BM_GETCHECK, 0, 0);
         GetDlgItem(IDC_DYNAMIC_AO).EnableWindow(checked ? TRUE : FALSE);
         break;
      }
      case IDC_3D_STEREO:
      case IDC_FAKE_STEREO:
      {
         LRESULT stereo3D = SendMessage(GetDlgItem(IDC_3D_STEREO).GetHwnd(), CB_GETCURSEL, 0, 0);
         bool fakeStereo = true;
         #ifdef ENABLE_SDL
         fakeStereo = SendDlgItemMessage(IDC_FAKE_STEREO, BM_GETCHECK, 0, 0);
         #endif
         SetDlgItemText(IDC_3D_STEREO_MS_LABEL, fakeStereo ? "Parallax Separation" : "Eye Separation (mm)");
         if (stereo3D == LB_ERR)
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
            #ifdef ENABLE_SDL
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
            #ifdef ENABLE_SDL
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

void VideoOptionsDialog::OnOK()
{
   SaveSettings(SendMessage(GetDlgItem(IDC_APPLICATION_SETTINGS).GetHwnd(), BM_GETCHECK, 0, 0) == BST_CHECKED);
   CDialog::OnOK();
}

void VideoOptionsDialog::OnClose()
{
   SendMessage(GetDlgItem(IDC_SIZELIST).GetHwnd(), LB_RESETCONTENT, 0, 0);
   CDialog::OnClose();
}
