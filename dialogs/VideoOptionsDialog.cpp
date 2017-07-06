#include "StdAfx.h"
#include "resource.h"
#include "VideoOptionsDialog.h"

#define GET_WINDOW_MODES		WM_USER+100
#define GET_FULLSCREENMODES		WM_USER+101
#define RESET_SIZELIST_CONTENT	WM_USER+102

const int rgwindowsize[] = { 640, 720, 800, 912, 1024, 1152, 1280, 1600 };  // windowed resolutions for selection list

VideoOptionsDialog::VideoOptionsDialog() : CDialog(IDD_VIDEO_OPTIONS)
{
}

void VideoOptionsDialog::AddToolTip(char *text, HWND parentHwnd, HWND toolTipHwnd, HWND controlHwnd)
{
   TOOLINFO toolInfo = { 0 };
   toolInfo.cbSize = sizeof(toolInfo);
   toolInfo.hwnd = parentHwnd;
   toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
   toolInfo.uId = (UINT_PTR)controlHwnd;
   toolInfo.lpszText = text;
   SendMessage(toolTipHwnd, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);
}

void VideoOptionsDialog::ResetVideoPreferences()
{
   char tmp[256];
   HWND hwndDlg = GetHwnd();

   HWND hwndCheck = GetDlgItem(IDC_FULLSCREEN).GetHwnd();
   SendMessage(hwndCheck, BM_SETCHECK, false ? BST_CHECKED : BST_UNCHECKED, 0);
   SendMessage(hwndDlg, false ? GET_FULLSCREENMODES : GET_WINDOW_MODES, 0, 0);

   hwndCheck = GetDlgItem(IDC_10BIT_VIDEO).GetHwnd();
   SendMessage(hwndCheck, BM_SETCHECK, false ? BST_CHECKED : BST_UNCHECKED, 0);
   SendMessage(GetDlgItem(IDC_Tex3072).GetHwnd(), BM_SETCHECK, BST_UNCHECKED, 0);
   SendMessage(GetDlgItem(IDC_Tex1024).GetHwnd(), BM_SETCHECK, BST_UNCHECKED, 0);
   SendMessage(GetDlgItem(IDC_Tex2048).GetHwnd(), BM_SETCHECK, BST_UNCHECKED, 0);
   SendMessage(GetDlgItem(IDC_TexUnlimited).GetHwnd(), BM_SETCHECK, BST_CHECKED, 0);
   hwndCheck = GetDlgItem(IDC_GLOBAL_REFLECTION_CHECK).GetHwnd();
   SendMessage(hwndCheck, BM_SETCHECK, true ? BST_CHECKED : BST_UNCHECKED, 0);
   hwndCheck = GetDlgItem(IDC_GLOBAL_TRAIL_CHECK).GetHwnd();
   SendMessage(hwndCheck, BM_SETCHECK, true ? BST_CHECKED : BST_UNCHECKED, 0);
   SetDlgItemInt(IDC_ADAPTIVE_VSYNC, 0, FALSE);
   SetDlgItemInt(IDC_MAX_PRE_FRAMES, 0, FALSE);
   float ballAspecRatioOffsetX = 0.0f;
   sprintf_s(tmp, 256, "%f", ballAspecRatioOffsetX);
   SetDlgItemTextA(IDC_CORRECTION_X, tmp);
   float ballAspecRatioOffsetY = 0.0f;
   sprintf_s(tmp, 256, "%f", ballAspecRatioOffsetY);
   SetDlgItemTextA(IDC_CORRECTION_Y, tmp);
   float latitude = 52.52f;
   sprintf_s(tmp, 256, "%f", latitude);
   SetDlgItemTextA(IDC_DN_LATITUDE, tmp);
   float longitude = 13.37f;
   sprintf_s(tmp, 256, "%f", longitude);
   SetDlgItemTextA(IDC_DN_LONGITUDE, tmp);
   float nudgeStrength = 2e-2f;
   sprintf_s(tmp, 256, "%f", nudgeStrength);
   SetDlgItemTextA(IDC_NUDGE_STRENGTH, tmp);
   hwndCheck = GetDlgItem(IDC_AA_ALL_TABLES).GetHwnd();
   SendMessage(hwndCheck, BM_SETCHECK, false ? BST_CHECKED : BST_UNCHECKED, 0);
   hwndCheck = GetDlgItem(IDC_DYNAMIC_DN).GetHwnd();
   SendMessage(hwndCheck, BM_SETCHECK, false ? BST_CHECKED : BST_UNCHECKED, 0);
   hwndCheck = GetDlgItem(IDC_DYNAMIC_AO).GetHwnd();
   SendMessage(hwndCheck, BM_SETCHECK, false ? BST_CHECKED : BST_UNCHECKED, 0);
   hwndCheck = GetDlgItem(IDC_ENABLE_AO).GetHwnd();
   SendMessage(hwndCheck, BM_SETCHECK, true ? BST_CHECKED : BST_UNCHECKED, 0);
   hwndCheck = GetDlgItem(IDC_OVERWRITE_BALL_IMAGE_CHECK).GetHwnd();
   SendMessage(hwndCheck, BM_SETCHECK, false ? BST_CHECKED : BST_UNCHECKED, 0);
   SetDlgItemText(IDC_BALL_IMAGE_EDIT, "");
   SetDlgItemText(IDC_BALL_DECAL_EDIT, "");
   if (true)
   {
      ::EnableWindow(GetDlgItem(IDC_BROWSE_BALL_IMAGE).GetHwnd(), FALSE);
      ::EnableWindow(GetDlgItem(IDC_BROWSE_BALL_DECAL).GetHwnd(), FALSE);
      ::EnableWindow(GetDlgItem(IDC_BALL_IMAGE_EDIT).GetHwnd(), FALSE);
      ::EnableWindow(GetDlgItem(IDC_BALL_DECAL_EDIT).GetHwnd(), FALSE);
   }
   SendMessage(GetDlgItem(IDC_FXAACB).GetHwnd(), CB_SETCURSEL, 0, 0);
   hwndCheck = GetDlgItem(IDC_SCALE_FX_DMD).GetHwnd();
   SendMessage(hwndCheck, BM_SETCHECK, false ? BST_CHECKED : BST_UNCHECKED, 0);
   hwndCheck = GetDlgItem(IDC_BG_SET).GetHwnd();
   SendMessage(hwndCheck, BM_SETCHECK, false ? BST_CHECKED : BST_UNCHECKED, 0);
   SendMessage(GetDlgItem(IDC_3D_STEREO).GetHwnd(), CB_SETCURSEL, 0, 0);
   hwndCheck = GetDlgItem(IDC_3D_STEREO_Y).GetHwnd();
   SendMessage(hwndCheck, BM_SETCHECK, false ? BST_CHECKED : BST_UNCHECKED, 0);
   float stereo3DOfs = 0.0f;
   sprintf_s(tmp, 256, "%f", stereo3DOfs);
   SetDlgItemTextA(IDC_3D_STEREO_OFS, tmp);
   float stereo3DMS = 0.03f;
   sprintf_s(tmp, 256, "%f", stereo3DMS);
   SetDlgItemTextA(IDC_3D_STEREO_MS, tmp);
   float stereo3DZPD = 0.5f;
   sprintf_s(tmp, 256, "%f", stereo3DZPD);
   SetDlgItemTextA(IDC_3D_STEREO_ZPD, tmp);
   hwndCheck = GetDlgItem(IDC_USE_NVIDIA_API_CHECK).GetHwnd();
   SendMessage(hwndCheck, BM_SETCHECK, false ? BST_CHECKED : BST_UNCHECKED, 0);
   hwndCheck = GetDlgItem(IDC_DISABLE_DWM).GetHwnd();
   SendMessage(hwndCheck, BM_SETCHECK, false ? BST_CHECKED : BST_UNCHECKED, 0);
   SendMessage(GetDlgItem(IDC_FORCE_ANISO).GetHwnd(), BM_SETCHECK, true ? BST_CHECKED : BST_UNCHECKED, 0);
   SendMessage(GetDlgItem(IDC_TEX_COMPRESS).GetHwnd(), BM_SETCHECK, false ? BST_CHECKED : BST_UNCHECKED, 0);
   SendMessage(GetDlgItem(IDC_SOFTWARE_VP).GetHwnd(), BM_SETCHECK, false ? BST_CHECKED : BST_UNCHECKED, 0);
   HWND hwndARASlider = GetDlgItem(IDC_ARASlider).GetHwnd();
   SendMessage(hwndARASlider, TBM_SETPOS, TRUE, 10);
   SendMessage(GetDlgItem(IDC_StretchYes).GetHwnd(), BM_SETCHECK, BST_UNCHECKED, 0);
   SendMessage(GetDlgItem(IDC_StretchMonitor).GetHwnd(), BM_SETCHECK, BST_UNCHECKED, 0);
   SendMessage(GetDlgItem(IDC_StretchNo).GetHwnd(), BM_SETCHECK, BST_CHECKED, 0);
   SendMessage(GetDlgItem(IDC_MonitorCombo).GetHwnd(), CB_SETCURSEL, 1, 0);
}

void VideoOptionsDialog::FillVideoModesList(const std::vector<VideoMode>& modes, const VideoMode* curSelMode)
{
   HWND hwndList = GetDlgItem(IDC_SIZELIST).GetHwnd();
   SendMessage(hwndList, LB_RESETCONTENT, 0, 0);

   for (unsigned i = 0; i < modes.size(); ++i)
   {
      char szT[128];
      if (modes[i].depth)
         sprintf_s(szT, "%d x %d (%dHz)", modes[i].width, modes[i].height, /*modes[i].depth,*/ modes[i].refreshrate);
      else
         sprintf_s(szT, "%d x %d", modes[i].width, modes[i].height);
      SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)szT);

      if (curSelMode &&
         modes[i].width == curSelMode->width &&
         modes[i].height == curSelMode->height &&
         modes[i].depth == curSelMode->depth &&
         modes[i].refreshrate == curSelMode->refreshrate)
         SendMessage(hwndList, LB_SETCURSEL, i, 0);
   }
}


BOOL VideoOptionsDialog::OnInitDialog()
{
   char tmp[256];
   HWND hwndDlg = GetHwnd();
   HWND toolTipHwnd = CreateWindowEx(NULL, TOOLTIPS_CLASS, NULL, WS_POPUP | TTS_ALWAYSTIP | TTS_BALLOON, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hwndDlg, NULL, g_hinst, NULL);
   if (toolTipHwnd)
   {
      SendMessage(toolTipHwnd, TTM_SETMAXTIPWIDTH, 0, 180);
      HWND controlHwnd = GetDlgItem(IDC_USE_NVIDIA_API_CHECK).GetHwnd();
      AddToolTip("Activate this if you get the corresponding error message on table start", hwndDlg, toolTipHwnd, controlHwnd);
      controlHwnd = GetDlgItem(IDC_TEX_COMPRESS).GetHwnd();
      AddToolTip("This saves memory on your graphics card but harms quality of the textures", hwndDlg, toolTipHwnd, controlHwnd);
      controlHwnd = GetDlgItem(IDC_DISABLE_DWM).GetHwnd();
      AddToolTip("Disable Windows Desktop Composition (only works on Windows Vista and Windows 7 systems).\r\nMay reduce lag and improve performance on some setups.", hwndDlg, toolTipHwnd, controlHwnd);
      controlHwnd = GetDlgItem(IDC_SOFTWARE_VP).GetHwnd();
      AddToolTip("Activate this if you have issues using an Intel graphics chip", hwndDlg, toolTipHwnd, controlHwnd);
      controlHwnd = GetDlgItem(IDC_ADAPTIVE_VSYNC).GetHwnd();
      AddToolTip("1-activates VSYNC for every frame (avoids tearing)\r\n2-adaptive VSYNC, waits only for fast frames (e.g. over 60fps)\r\nor set it to e.g. 60 or 120 to limit the fps to that value (energy saving/less heat)", hwndDlg, toolTipHwnd, controlHwnd);
      controlHwnd = GetDlgItem(IDC_MAX_PRE_FRAMES).GetHwnd();
      AddToolTip("Experiment with 1 or 2 for a chance of lag reduction at the price of a bit of performance", hwndDlg, toolTipHwnd, controlHwnd);
      controlHwnd = GetDlgItem(IDC_StretchMonitor).GetHwnd();
      AddToolTip("If played in cabinet mode and you get an egg shaped ball activate this.\r\nFor screen ratios other than 16:9 you may have to adjust the offsets.\r\nNormally you have to set the Y offset (around 1.5) but you have to experiment.", hwndDlg, toolTipHwnd, controlHwnd);
      controlHwnd = GetDlgItem(IDC_NUDGE_STRENGTH).GetHwnd();
      AddToolTip("Changes the visual effect/screen shaking when nudging the table.", hwndDlg, toolTipHwnd, controlHwnd);
      controlHwnd = GetDlgItem(IDC_DYNAMIC_DN).GetHwnd();
      AddToolTip("Activate this to switch the table brightness automatically based on your PC date,clock and location.\r\nThis requires to fill in geographic coordinates for your PCs location to work correctly.\r\nYou may use openstreetmap.org for example to get these in the correct format.", hwndDlg, toolTipHwnd, controlHwnd);
      controlHwnd = GetDlgItem(IDC_DN_LATITUDE).GetHwnd();
      AddToolTip("In decimal degrees (-90..90, North positive)", hwndDlg, toolTipHwnd, controlHwnd);
      controlHwnd = GetDlgItem(IDC_DN_LONGITUDE).GetHwnd();
      AddToolTip("In decimal degrees (-180..180, East positive)", hwndDlg, toolTipHwnd, controlHwnd);
      controlHwnd = GetDlgItem(IDC_DYNAMIC_AO).GetHwnd();
      AddToolTip("Activate this to enable dynamic Ambient Occlusion.\r\nThis slows down performance, but enables contact shadows for dynamic objects.", hwndDlg, toolTipHwnd, controlHwnd);
      controlHwnd = GetDlgItem(IDC_FORCE_ANISO).GetHwnd();
      AddToolTip("Activate this to enhance the texture filtering.\r\nThis slows down performance only a bit (on most systems), but increases quality tremendously.", hwndDlg, toolTipHwnd, controlHwnd);
      controlHwnd = GetDlgItem(IDC_ENABLE_AO).GetHwnd();
      AddToolTip("Activate this to enable Ambient Occlusion.\r\nThis enables contact shadows between objects.", hwndDlg, toolTipHwnd, controlHwnd);
      controlHwnd = GetDlgItem(IDC_3D_STEREO).GetHwnd();
      AddToolTip("Activate this to enable 3D Stereo output using the requested format.\r\nSwitch on/off during play with the F10 key.\r\nThis requires that your TV can display 3D Stereo and respective 3D glasses.", hwndDlg, toolTipHwnd, controlHwnd);
      controlHwnd = GetDlgItem(IDC_3D_STEREO_Y).GetHwnd();
      AddToolTip("Switches 3D Stereo effect to use the Y Axis.\r\nThis should usually be selected for Cabinets/rotated displays.", hwndDlg, toolTipHwnd, controlHwnd);
      controlHwnd = GetDlgItem(IDC_FULLSCREEN).GetHwnd();
      AddToolTip("Enforces exclusive Fullscreen Mode.\r\nDo not enable if you require to see the VPinMAME or B2S windows for example.\r\nEnforcing exclusive FS can slightly reduce input lag though.", hwndDlg, toolTipHwnd, controlHwnd);
      controlHwnd = GetDlgItem(IDC_10BIT_VIDEO).GetHwnd();
      AddToolTip("Enforces 10bit (WCG) rendering.\r\nRequires a corresponding 10bit output capable graphics card and monitor.\r\nAlso requires to have exclusive fullscreen mode enforced (for now).", hwndDlg, toolTipHwnd, controlHwnd);
      controlHwnd = GetDlgItem(IDC_BG_SET).GetHwnd();
      AddToolTip("Switches all tables to use the respective Cabinet display setup.\r\nAlso useful if a 270 degree rotated Desktop monitor is used.", hwndDlg, toolTipHwnd, controlHwnd);
      controlHwnd = GetDlgItem(IDC_FXAACB).GetHwnd();
      AddToolTip("Enables post-processed Anti-Aliasing.\r\nThis delivers smoother images, at the cost of slight blurring.\r\n'Quality FXAA' and 'Quality SMAA' are recommended and lead to less artifacts,\nbut will harm performance on low-end graphics cards.", hwndDlg, toolTipHwnd, controlHwnd);
      controlHwnd = GetDlgItem(IDC_AA_ALL_TABLES).GetHwnd();
      AddToolTip("Enables brute-force 4x Anti-Aliasing.\r\nThis delivers very good quality, but slows down performance significantly.", hwndDlg, toolTipHwnd, controlHwnd);
      controlHwnd = GetDlgItem(IDC_OVERWRITE_BALL_IMAGE_CHECK).GetHwnd();
      AddToolTip("When checked it overwrites the ball image/decal image(s) for every table.", hwndDlg, toolTipHwnd, controlHwnd);
   }

   int maxTexDim;
   HRESULT hr = GetRegInt("Player", "MaxTexDimension", &maxTexDim);
   if (hr != S_OK)
      maxTexDim = 0; // default: Don't resize textures
   switch (maxTexDim)
   {
      case 3072:SendMessage(GetDlgItem(IDC_Tex3072).GetHwnd(), BM_SETCHECK, BST_CHECKED, 0);       break;
      case 512: // legacy, map to 1024 now
      case 1024:SendMessage(GetDlgItem(IDC_Tex1024).GetHwnd(), BM_SETCHECK, BST_CHECKED, 0);       break;
      case 2048:SendMessage(GetDlgItem(IDC_Tex2048).GetHwnd(), BM_SETCHECK, BST_CHECKED, 0);       break;
      default:	SendMessage(GetDlgItem(IDC_TexUnlimited).GetHwnd(), BM_SETCHECK, BST_CHECKED, 0);   break;
   }

   HWND hwndCheck = GetDlgItem(IDC_GLOBAL_REFLECTION_CHECK).GetHwnd();
   int reflection;
   hr = GetRegInt("Player", "BallReflection", &reflection);
   if (hr != S_OK)
      reflection = fTrue;
   SendMessage(hwndCheck, BM_SETCHECK, reflection ? BST_CHECKED : BST_UNCHECKED, 0);

   hwndCheck = GetDlgItem(IDC_GLOBAL_TRAIL_CHECK).GetHwnd();
   int trail;
   hr = GetRegInt("Player", "BallTrail", &trail);
   if (hr != S_OK)
      trail = fTrue;
   SendMessage(hwndCheck, BM_SETCHECK, trail ? BST_CHECKED : BST_UNCHECKED, 0);

   int vsync;
   hr = GetRegInt("Player", "AdaptiveVSync", &vsync);
   if (hr != S_OK)
      vsync = 0;
   SetDlgItemInt(IDC_ADAPTIVE_VSYNC, vsync, FALSE);

   int maxPrerenderedFrames;
   hr = GetRegInt("Player", "MaxPrerenderedFrames", &maxPrerenderedFrames);
   if (hr != S_OK)
      maxPrerenderedFrames = 0;
   SetDlgItemInt(IDC_MAX_PRE_FRAMES, maxPrerenderedFrames, FALSE);

   float ballAspecRatioOffsetX;
   hr = GetRegStringAsFloat("Player", "BallCorrectionX", &ballAspecRatioOffsetX);
   if (hr != S_OK)
      ballAspecRatioOffsetX = 0.0f;
   sprintf_s(tmp, 256, "%f", ballAspecRatioOffsetX);
   SetDlgItemTextA(IDC_CORRECTION_X, tmp);

   float ballAspecRatioOffsetY;
   hr = GetRegStringAsFloat("Player", "BallCorrectionY", &ballAspecRatioOffsetY);
   if (hr != S_OK)
      ballAspecRatioOffsetY = 0.0f;
   sprintf_s(tmp, 256, "%f", ballAspecRatioOffsetY);
   SetDlgItemTextA(IDC_CORRECTION_Y, tmp);

   float latitude;
   hr = GetRegStringAsFloat("Player", "Latitude", &latitude);
   if (hr != S_OK)
      latitude = 52.52f;
   sprintf_s(tmp, 256, "%f", latitude);
   SetDlgItemTextA(IDC_DN_LATITUDE, tmp);

   float longitude;
   hr = GetRegStringAsFloat("Player", "Longitude", &longitude);
   if (hr != S_OK)
      longitude = 13.37f;
   sprintf_s(tmp, 256, "%f", longitude);
   SetDlgItemTextA(IDC_DN_LONGITUDE, tmp);

   float nudgeStrength;
   hr = GetRegStringAsFloat("Player", "NudgeStrength", &nudgeStrength);
   if (hr != S_OK)
      nudgeStrength = 2e-2f;
   sprintf_s(tmp, 256, "%f", nudgeStrength);
   SetDlgItemTextA(IDC_NUDGE_STRENGTH, tmp);

   hwndCheck = GetDlgItem(IDC_AA_ALL_TABLES).GetHwnd();
   int useAA;
   hr = GetRegInt("Player", "USEAA", &useAA);
   if (hr != S_OK)
      useAA = 0;
   SendMessage(hwndCheck, BM_SETCHECK, (useAA != 0) ? BST_CHECKED : BST_UNCHECKED, 0);

   hwndCheck = GetDlgItem(IDC_DYNAMIC_DN).GetHwnd();
   int useDN;
   hr = GetRegInt("Player", "DynamicDayNight", &useDN);
   if (hr != S_OK)
      useDN = 0;
   SendMessage(hwndCheck, BM_SETCHECK, (useDN != 0) ? BST_CHECKED : BST_UNCHECKED, 0);

   hwndCheck = GetDlgItem(IDC_DYNAMIC_AO).GetHwnd();
   int useAO;
   hr = GetRegInt("Player", "DynamicAO", &useAO);
   if (hr != S_OK)
      useAO = 0;
   SendMessage(hwndCheck, BM_SETCHECK, (useAO != 0) ? BST_CHECKED : BST_UNCHECKED, 0);

   hwndCheck = GetDlgItem(IDC_ENABLE_AO).GetHwnd();
   hr = GetRegInt("Player", "DisableAO", &useAO);
   if (hr != S_OK)
      useAO = 0;
   SendMessage(hwndCheck, BM_SETCHECK, (useAO != 0) ? BST_UNCHECKED : BST_CHECKED, 0); // inverted logic

   hwndCheck = GetDlgItem(IDC_OVERWRITE_BALL_IMAGE_CHECK).GetHwnd();
   int overwiteBallImage;
   hr = GetRegInt("Player", "OverwriteBallImage", &overwiteBallImage);
   if (hr != S_OK)
      overwiteBallImage = 0;
   SendMessage(hwndCheck, BM_SETCHECK, (overwiteBallImage != 0) ? BST_CHECKED : BST_UNCHECKED, 0);

   char imageName[MAX_PATH] = { 0 };
   hr = GetRegString("Player", "BallImage", imageName, MAX_PATH);
   if (hr != S_OK)
      imageName[0] = 0;
   SetDlgItemText(IDC_BALL_IMAGE_EDIT, imageName);
   imageName[0] = 0;
   hr = GetRegString("Player", "DecalImage", imageName, MAX_PATH);
   if (hr != S_OK)
      imageName[0] = 0;
   SetDlgItemText(IDC_BALL_DECAL_EDIT, imageName);
   if (overwiteBallImage == 0)
   {
      ::EnableWindow(GetDlgItem(IDC_BROWSE_BALL_IMAGE).GetHwnd(), FALSE);
      ::EnableWindow(GetDlgItem(IDC_BROWSE_BALL_DECAL).GetHwnd(), FALSE);
      ::EnableWindow(GetDlgItem(IDC_BALL_IMAGE_EDIT).GetHwnd(), FALSE);
      ::EnableWindow(GetDlgItem(IDC_BALL_DECAL_EDIT).GetHwnd(), FALSE);
   }

   int fxaa;
   hr = GetRegInt("Player", "FXAA", &fxaa);
   if (hr != S_OK)
      fxaa = 0;
   SendMessage(GetDlgItem(IDC_FXAACB).GetHwnd(), CB_ADDSTRING, 0, (LPARAM)"Disabled");
   SendMessage(GetDlgItem(IDC_FXAACB).GetHwnd(), CB_ADDSTRING, 0, (LPARAM)"Fast FXAA");
   SendMessage(GetDlgItem(IDC_FXAACB).GetHwnd(), CB_ADDSTRING, 0, (LPARAM)"Standard FXAA");
   SendMessage(GetDlgItem(IDC_FXAACB).GetHwnd(), CB_ADDSTRING, 0, (LPARAM)"Quality FXAA");
   SendMessage(GetDlgItem(IDC_FXAACB).GetHwnd(), CB_ADDSTRING, 0, (LPARAM)"Fast NFAA");
   SendMessage(GetDlgItem(IDC_FXAACB).GetHwnd(), CB_ADDSTRING, 0, (LPARAM)"Standard DLAA");
   SendMessage(GetDlgItem(IDC_FXAACB).GetHwnd(), CB_ADDSTRING, 0, (LPARAM)"Quality SMAA");
   SendMessage(GetDlgItem(IDC_FXAACB).GetHwnd(), CB_SETCURSEL, fxaa, 0);

   hwndCheck = GetDlgItem(IDC_SCALE_FX_DMD).GetHwnd();
   int scaleFX_DMD;
   hr = GetRegInt("Player", "ScaleFXDMD", &scaleFX_DMD);
   if (hr != S_OK)
      scaleFX_DMD = fFalse;
   SendMessage(hwndCheck, BM_SETCHECK, scaleFX_DMD ? BST_CHECKED : BST_UNCHECKED, 0);

   hwndCheck = GetDlgItem(IDC_BG_SET).GetHwnd();
   int bgset;
   hr = GetRegInt("Player", "BGSet", &bgset);
   if (hr != S_OK)
      bgset = 0;
   SendMessage(hwndCheck, BM_SETCHECK, (bgset != 0) ? BST_CHECKED : BST_UNCHECKED, 0);

   int stereo3D;
   hr = GetRegInt("Player", "Stereo3D", &stereo3D);
   if (hr != S_OK)
      stereo3D = 0;
   SendMessage(GetDlgItem(IDC_3D_STEREO).GetHwnd(), CB_ADDSTRING, 0, (LPARAM)"Disabled");
   SendMessage(GetDlgItem(IDC_3D_STEREO).GetHwnd(), CB_ADDSTRING, 0, (LPARAM)"TB (Top / Bottom)");
   SendMessage(GetDlgItem(IDC_3D_STEREO).GetHwnd(), CB_ADDSTRING, 0, (LPARAM)"Interlaced (e.g. LG TVs)");
   SendMessage(GetDlgItem(IDC_3D_STEREO).GetHwnd(), CB_ADDSTRING, 0, (LPARAM)"SBS (Side by Side)");
   SendMessage(GetDlgItem(IDC_3D_STEREO).GetHwnd(), CB_SETCURSEL, stereo3D, 0);

   hwndCheck = GetDlgItem(IDC_3D_STEREO_Y).GetHwnd();
   int stereo3DY;
   hr = GetRegInt("Player", "Stereo3DYAxis", &stereo3DY);
   if (hr != S_OK)
      stereo3DY = fFalse;
   SendMessage(hwndCheck, BM_SETCHECK, stereo3DY ? BST_CHECKED : BST_UNCHECKED, 0);

   float stereo3DOfs;
   hr = GetRegStringAsFloat("Player", "Stereo3DOffset", &stereo3DOfs);
   if (hr != S_OK)
      stereo3DOfs = 0.0f;
   sprintf_s(tmp, 256, "%f", stereo3DOfs);
   SetDlgItemTextA(IDC_3D_STEREO_OFS, tmp);

   float stereo3DMS;
   hr = GetRegStringAsFloat("Player", "Stereo3DMaxSeparation", &stereo3DMS);
   if (hr != S_OK)
      stereo3DMS = 0.03f;
   sprintf_s(tmp, 256, "%f", stereo3DMS);
   SetDlgItemTextA(IDC_3D_STEREO_MS, tmp);

   float stereo3DZPD;
   hr = GetRegStringAsFloat("Player", "Stereo3DZPD", &stereo3DZPD);
   if (hr != S_OK)
      stereo3DZPD = 0.5f;
   sprintf_s(tmp, 256, "%f", stereo3DZPD);
   SetDlgItemTextA(IDC_3D_STEREO_ZPD, tmp);

   hwndCheck = GetDlgItem(IDC_DISABLE_DWM).GetHwnd();
   int disableDWM;
   hr = GetRegInt("Player", "DisableDWM", &disableDWM);
   if (hr != S_OK)
      disableDWM = 0;
   SendMessage(hwndCheck, BM_SETCHECK, (disableDWM != 0) ? BST_CHECKED : BST_UNCHECKED, 0);

   hwndCheck = GetDlgItem(IDC_USE_NVIDIA_API_CHECK).GetHwnd();
   int nvidiaApi;
   hr = GetRegInt("Player", "UseNVidiaAPI", &nvidiaApi);
   if (hr != S_OK)
      nvidiaApi = 0;
   SendMessage(hwndCheck, BM_SETCHECK, (nvidiaApi != 0) ? BST_CHECKED : BST_UNCHECKED, 0);

   const bool forceAniso = (GetRegIntWithDefault("Player", "ForceAnisotropicFiltering", 1) != 0);
   SendMessage(GetDlgItem(IDC_FORCE_ANISO).GetHwnd(), BM_SETCHECK, forceAniso ? BST_CHECKED : BST_UNCHECKED, 0);

   const bool compressTextures = (GetRegIntWithDefault("Player", "CompressTextures", 0) != 0);
   SendMessage(GetDlgItem(IDC_TEX_COMPRESS).GetHwnd(), BM_SETCHECK, compressTextures ? BST_CHECKED : BST_UNCHECKED, 0);

   const bool softwareVP = (GetRegIntWithDefault("Player", "SoftwareVertexProcessing", 0) != 0);
   SendMessage(GetDlgItem(IDC_SOFTWARE_VP).GetHwnd(), BM_SETCHECK, softwareVP ? BST_CHECKED : BST_UNCHECKED, 0);

   const bool video10bit = (GetRegIntWithDefault("Player", "Render10Bit", 0) != 0);
   SendMessage(GetDlgItem(IDC_10BIT_VIDEO).GetHwnd(), BM_SETCHECK, video10bit ? BST_CHECKED : BST_UNCHECKED, 0);

   int widthcur;
   hr = GetRegInt("Player", "Width", &widthcur);
   if (hr != S_OK)
      widthcur = DEFAULT_PLAYER_WIDTH;

   int heightcur;
   hr = GetRegInt("Player", "Height", &heightcur);
   if (hr != S_OK)
      heightcur = widthcur * 3 / 4;

   int depthcur;
   hr = GetRegInt("Player", "ColorDepth", &depthcur);
   if (hr != S_OK)
      depthcur = 32;

   int refreshrate;
   hr = GetRegInt("Player", "RefreshRate", &refreshrate);
   if (hr != S_OK)
      refreshrate = 0; // The default

   int fullscreen;
   hr = GetRegInt("Player", "FullScreen", &fullscreen);
   if (hr != S_OK)
      fullscreen = 0;

   HWND hwndFullscreen = GetDlgItem(IDC_FULLSCREEN).GetHwnd();
   if (fullscreen)
   {
      SendMessage(hwndDlg, GET_FULLSCREENMODES, widthcur << 16 | refreshrate, heightcur << 16 | depthcur);
      SendMessage(hwndFullscreen, BM_SETCHECK, BST_CHECKED, 0);
   }
   else
   {
      SendMessage(hwndDlg, GET_WINDOW_MODES, widthcur, heightcur);
      SendMessage(hwndFullscreen, BM_SETCHECK, BST_UNCHECKED, 0);
   }

   int alphaRampsAccuracy;
   hr = GetRegInt("Player", "AlphaRampAccuracy", &alphaRampsAccuracy);
   if (hr != S_OK)
      alphaRampsAccuracy = 10;
   HWND hwndARASlider = GetDlgItem(IDC_ARASlider).GetHwnd();
   SendMessage(hwndARASlider, TBM_SETRANGE, fTrue, MAKELONG(0, 10));
   SendMessage(hwndARASlider, TBM_SETTICFREQ, 1, 0);
   SendMessage(hwndARASlider, TBM_SETLINESIZE, 0, 1);
   SendMessage(hwndARASlider, TBM_SETPAGESIZE, 0, 1);
   SendMessage(hwndARASlider, TBM_SETTHUMBLENGTH, 5, 0);
   SendMessage(hwndARASlider, TBM_SETPOS, TRUE, alphaRampsAccuracy);

   int ballStretchMode;
   hr = GetRegInt("Player", "BallStretchMode", &ballStretchMode);
   if (hr != S_OK)
      ballStretchMode = 0;
   switch (ballStretchMode)
   {
      case 0:  SendMessage(GetDlgItem(IDC_StretchNo).GetHwnd(), BM_SETCHECK, BST_CHECKED, 0);      break;
      case 1:  SendMessage(GetDlgItem(IDC_StretchYes).GetHwnd(), BM_SETCHECK, BST_CHECKED, 0);     break;
      case 2:  SendMessage(GetDlgItem(IDC_StretchMonitor).GetHwnd(), BM_SETCHECK, BST_CHECKED, 0); break;
      default: SendMessage(GetDlgItem(IDC_StretchNo).GetHwnd(), BM_SETCHECK, BST_CHECKED, 0);
   }

   // set selected Monitors
   // Monitors: 4:3, 16:9, 16:10, 21:10
   int selected;
   hr = GetRegInt("Player", "BallStretchMonitor", &selected);
   if (hr != S_OK)
      selected = 1; // assume 16:9 as standard
   SendMessage(GetDlgItem(IDC_MonitorCombo).GetHwnd(), CB_ADDSTRING, 0, (LPARAM)"4:3");
   SendMessage(GetDlgItem(IDC_MonitorCombo).GetHwnd(), CB_ADDSTRING, 0, (LPARAM)"16:9");
   SendMessage(GetDlgItem(IDC_MonitorCombo).GetHwnd(), CB_ADDSTRING, 0, (LPARAM)"16:10");
   SendMessage(GetDlgItem(IDC_MonitorCombo).GetHwnd(), CB_ADDSTRING, 0, (LPARAM)"21:10");
   SendMessage(GetDlgItem(IDC_MonitorCombo).GetHwnd(), CB_ADDSTRING, 0, (LPARAM)"3:4 (R)");
   SendMessage(GetDlgItem(IDC_MonitorCombo).GetHwnd(), CB_ADDSTRING, 0, (LPARAM)"9:16 (R)");
   SendMessage(GetDlgItem(IDC_MonitorCombo).GetHwnd(), CB_ADDSTRING, 0, (LPARAM)"10:16 (R)");
   SendMessage(GetDlgItem(IDC_MonitorCombo).GetHwnd(), CB_ADDSTRING, 0, (LPARAM)"10:21 (R)");
   SendMessage(GetDlgItem(IDC_MonitorCombo).GetHwnd(), CB_SETCURSEL, selected, 0);

   return TRUE;
}

INT_PTR VideoOptionsDialog::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   switch (uMsg)
   {
      case GET_WINDOW_MODES:
      {
         size_t indexcur = -1;
         size_t indx = -1;
         int widthcur = (int)wParam, heightcur = (int)lParam;

         SendMessage(GetHwnd(), RESET_SIZELIST_CONTENT, 0, 0);
         HWND hwndList = GetDlgItem(IDC_SIZELIST).GetHwnd();
         //indx = SendMessage(hwndList, LB_GETCURSEL, 0L, 0L);
         //if (indx == LB_ERR)
         //  indx = 0;

         const size_t csize = sizeof(rgwindowsize) / sizeof(int);
         const int screenwidth = GetSystemMetrics(SM_CXSCREEN);
         const int screenheight = GetSystemMetrics(SM_CYSCREEN);

         //if (indx != -1)
         //  indexcur = indx;

         allVideoModes.clear();
         unsigned int cnt = 0;

         // test video modes first on list

         // add some (windowed) portrait play modes

         // 16:10 aspect ratio resolutions: 1280×800, 1440×900, 1680×1050, 1920×1200 and 2560×1600
         // 16:9 aspect ratio resolutions:  1280×720, 1366×768, 1600×900, 1920×1080, 2560×1440 and 3840×2160
         // 4:3  aspect ratio resolutions:  1024x1280
         const unsigned int num_portrait_modes = 14;
         const int portrait_modes_width[num_portrait_modes] =  { 720, 720, 1024, 768, 800, 900, 900,1050,1050,1080,1200,1440,1600,2160};
         const int portrait_modes_height[num_portrait_modes] = {1024,1280, 1280,1366,1280,1440,1600,1600,1680,1920,1920,2560,2560,3840};

         for(unsigned int i = 0; i < num_portrait_modes; ++i)
            if ((portrait_modes_width[i] <= screenwidth) && (portrait_modes_height[i] <= screenheight))
            {
              VideoMode mymode;
              mymode.width = portrait_modes_width[i];
              mymode.height = portrait_modes_height[i];
              mymode.depth = 0;
              mymode.refreshrate = 0;

              allVideoModes.push_back(mymode);
              if (heightcur > widthcur)
                if ((portrait_modes_width[i] == widthcur) && (portrait_modes_height[i] == heightcur))
                  indx = i;
              cnt++;
            }

         // add landscape play modes

         for (size_t i = 0; i < csize; ++i)  
         {
            const int xsize = rgwindowsize[i];
            if ((xsize <= screenwidth) && ((xsize * 3 / 4) <= screenheight))
            {
               if ((xsize == widthcur) && ((xsize * 3 / 4) == heightcur))
                  indx = i + cnt;

               VideoMode mode;
               mode.width = xsize;
               mode.height = xsize * 3 / 4;
               mode.depth = 0;
               mode.refreshrate = 0;

               allVideoModes.push_back(mode);
            }
         }

         FillVideoModesList(allVideoModes);

         // set up windowed fullscreen mode
         VideoMode mode;
         // TODO: use multi-monitor functions
         mode.width = GetSystemMetrics(SM_CXSCREEN);
         mode.height = GetSystemMetrics(SM_CYSCREEN);
         mode.depth = 0;
         mode.refreshrate = 0;
         allVideoModes.push_back(mode);

         char szT[128];
         //if (indexcur == -1)
         //  indexcur = indx;

         if (mode.height < mode.width) // landscape
         {
              sprintf_s(szT, "%d x %d (Windowed Fullscreen)", mode.width, mode.height);
              SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)szT);
              if (indx == -1)
                indexcur = SendMessage(hwndList, LB_GETCOUNT, 0, 0) - 1;
              else
                indexcur = indx;
         }
         else { // portrait
              if ((indx == -1) || (indx < num_portrait_modes))
              {
                  indexcur = indx;
                  if (cnt > 0)
                  {
                      char szTx[128];
                      SendMessage(hwndList, LB_GETTEXT, cnt - 1, (LPARAM)szTx);
                      SendMessage(hwndList, LB_DELETESTRING, cnt - 1, 0L);

                      if (cnt-1 < num_portrait_modes)
                      {
                          mode.width = portrait_modes_width[cnt-1];
                          mode.height = portrait_modes_height[cnt-1];

                          if ((mode.height == screenheight) && (mode.width == screenwidth))
                              sprintf_s(szT, "%d x %d (Windowed Fullscreen)", mode.width, mode.height);
                          else
                              sprintf_s(szT, "%d x %d", mode.width, mode.height);
                      }
                      else {
                          memset(&szTx,'\x0', sizeof(szTx));
                          strcpy(szT, szTx);
                      }

                      SendMessage(hwndList, LB_INSERTSTRING, cnt - 1, (LPARAM)szT);
                  }// end if cnt > 0
              } //end if indx
         } //end if else mode height < width

         SendMessage(hwndList, LB_SETCURSEL, (indexcur != -1) ? indexcur : 0, 0);
         break;
      } // end case GET_WINDOW_MODES
      case GET_FULLSCREENMODES:
      {
         HWND hwndList = GetDlgItem(IDC_SIZELIST).GetHwnd();
         EnumerateDisplayModes(0, allVideoModes);

         VideoMode curSelMode;
         curSelMode.width = (int)wParam >> 16;
         curSelMode.height = (int)lParam >> 16;
         curSelMode.depth = (int)lParam & 0xffff;
         curSelMode.refreshrate = (int)wParam & 0xffff;

         FillVideoModesList(allVideoModes, &curSelMode);

         if (SendMessage(hwndList, LB_GETCURSEL, 0, 0) == -1)
            SendMessage(hwndList, LB_SETCURSEL, 0, 0);
         break;
      }
      case RESET_SIZELIST_CONTENT:
      {
         HWND hwndList = GetDlgItem(IDC_SIZELIST).GetHwnd();
         SendMessage(hwndList, LB_RESETCONTENT, 0, 0);
         break;
      }
   }

   return DialogProcDefault(uMsg, wParam, lParam);
}

BOOL VideoOptionsDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
   UNREFERENCED_PARAMETER(lParam);

   switch (LOWORD(wParam))
   {
      case IDC_DEFAULTS:
      {
         ResetVideoPreferences();
         break;
      }
      case IDC_OVERWRITE_BALL_IMAGE_CHECK:
      {
         const bool overwriteEnabled = IsDlgButtonChecked(IDC_OVERWRITE_BALL_IMAGE_CHECK) == BST_CHECKED;
         if (overwriteEnabled)
         {
            ::EnableWindow(GetDlgItem(IDC_BROWSE_BALL_IMAGE).GetHwnd(), TRUE);
            ::EnableWindow(GetDlgItem(IDC_BROWSE_BALL_DECAL).GetHwnd(), TRUE);
            ::EnableWindow(GetDlgItem(IDC_BALL_IMAGE_EDIT).GetHwnd(), TRUE);
            ::EnableWindow(GetDlgItem(IDC_BALL_DECAL_EDIT).GetHwnd(), TRUE);
         }
         else
         {
            ::EnableWindow(GetDlgItem(IDC_BROWSE_BALL_IMAGE).GetHwnd(), FALSE);
            ::EnableWindow(GetDlgItem(IDC_BROWSE_BALL_DECAL).GetHwnd(), FALSE);
            ::EnableWindow(GetDlgItem(IDC_BALL_IMAGE_EDIT).GetHwnd(), FALSE);
            ::EnableWindow(GetDlgItem(IDC_BALL_DECAL_EDIT).GetHwnd(), FALSE);
         }
         break;
      }
      case IDC_BROWSE_BALL_IMAGE:
      {
         char szFileName[1024];
         szFileName[0] = '\0';

         OPENFILENAME ofn;
         ZeroMemory(&ofn, sizeof(OPENFILENAME));
         ofn.lStructSize = sizeof(OPENFILENAME);
         ofn.hInstance = g_hinst;
         ofn.hwndOwner = g_pvp->m_hwnd;
         // TEXT
         ofn.lpstrFilter = "Bitmap, JPEG, PNG, EXR, HDR Files (.bmp/.jpg/.png/.exr/.hdr)\0*.bmp;*.jpg;*.jpeg;*.png;*.exr;*.hdr\0";
         ofn.lpstrFile = szFileName;
         ofn.nMaxFile = _MAX_PATH;
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
         char szFileName[1024];
         szFileName[0] = '\0';

         OPENFILENAME ofn;
         ZeroMemory(&ofn, sizeof(OPENFILENAME));
         ofn.lStructSize = sizeof(OPENFILENAME);
         ofn.hInstance = g_hinst;
         ofn.hwndOwner = g_pvp->m_hwnd;
         // TEXT
         ofn.lpstrFilter = "Bitmap, JPEG, PNG, EXR, HDR Files (.bmp/.jpg/.png/.exr/.hdr)\0*.bmp;*.jpg;*.jpeg;*.png;*.exr;*.hdr\0";
         ofn.lpstrFile = szFileName;
         ofn.nMaxFile = _MAX_PATH;
         ofn.lpstrDefExt = "png";
         ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
         const int ret = GetOpenFileName(&ofn);
         if (!ret)
            break;
         SetDlgItemText(IDC_BALL_DECAL_EDIT, szFileName);
         break;
      }

      case IDC_FULLSCREEN:
      {
         const size_t checked = SendDlgItemMessage(IDC_FULLSCREEN, BM_GETCHECK, 0, 0);
         SendMessage(checked ? GET_FULLSCREENMODES : GET_WINDOW_MODES, 0, 0);
         break;
      }

      default:
         return FALSE;
   }
   return TRUE;
}

void VideoOptionsDialog::OnOK()
{
   BOOL nothing = 0;
   HWND hwndFullScreen = GetDlgItem(IDC_FULLSCREEN).GetHwnd();
   size_t fullscreen = SendMessage(hwndFullScreen, BM_GETCHECK, 0, 0);
   SetRegValue("Player", "FullScreen", REG_DWORD, &fullscreen, 4);

   HWND hwndList = GetDlgItem(IDC_SIZELIST).GetHwnd();
   size_t index = SendMessage(hwndList, LB_GETCURSEL, 0, 0);
   VideoMode* pvm = &allVideoModes[index];
   SetRegValue("Player", "Width", REG_DWORD, &pvm->width, 4);
   SetRegValue("Player", "Height", REG_DWORD, &pvm->height, 4);
   if (fullscreen)
   {
      SetRegValue("Player", "ColorDepth", REG_DWORD, &pvm->depth, 4);
      SetRegValue("Player", "RefreshRate", REG_DWORD, &pvm->refreshrate, 4);
   }

   HWND hwnd10bit = GetDlgItem(IDC_10BIT_VIDEO).GetHwnd();
   size_t video10bit = SendMessage(hwnd10bit, BM_GETCHECK, 0, 0);
   SetRegValue("Player", "Render10Bit", REG_DWORD, &video10bit, 4);

   HWND maxTexDim3072 = GetDlgItem(IDC_Tex3072).GetHwnd();
   HWND maxTexDim1024 = GetDlgItem(IDC_Tex1024).GetHwnd();
   HWND maxTexDim2048 = GetDlgItem(IDC_Tex2048).GetHwnd();
   //HWND maxTexDimUnlimited = GetDlgItem(hwndDlg, IDC_TexUnlimited);
   int maxTexDim = 0;
   if (SendMessage(maxTexDim3072, BM_GETCHECK, 0, 0) == BST_CHECKED)
      maxTexDim = 3072;
   if (SendMessage(maxTexDim1024, BM_GETCHECK, 0, 0) == BST_CHECKED)
      maxTexDim = 1024;
   if (SendMessage(maxTexDim2048, BM_GETCHECK, 0, 0) == BST_CHECKED)
      maxTexDim = 2048;
   SetRegValue("Player", "MaxTexDimension", REG_DWORD, &maxTexDim, 4);

   HWND hwndReflect = GetDlgItem(IDC_GLOBAL_REFLECTION_CHECK).GetHwnd();
   size_t reflection = SendMessage(hwndReflect, BM_GETCHECK, 0, 0);
   SetRegValue("Player", "BallReflection", REG_DWORD, &reflection, 4);

   HWND hwndTrail = GetDlgItem(IDC_GLOBAL_TRAIL_CHECK).GetHwnd();
   size_t trail = SendMessage(hwndTrail, BM_GETCHECK, 0, 0);
   SetRegValue("Player", "BallTrail", REG_DWORD, &trail, 4);

   int vsync = GetDlgItemInt(IDC_ADAPTIVE_VSYNC, nothing, TRUE);
   SetRegValue("Player", "AdaptiveVSync", REG_DWORD, &vsync, 4);

   int maxPrerenderedFrames = GetDlgItemInt(IDC_MAX_PRE_FRAMES, nothing, TRUE);
   SetRegValue("Player", "MaxPrerenderedFrames", REG_DWORD, &maxPrerenderedFrames, 4);

   CString tmpStr;
   tmpStr = GetDlgItemTextA(IDC_CORRECTION_X);
   SetRegValue("Player", "BallCorrectionX", REG_SZ, tmpStr.c_str(), lstrlen(tmpStr.c_str()));

   tmpStr = GetDlgItemTextA(IDC_CORRECTION_Y);
   SetRegValue("Player", "BallCorrectionY", REG_SZ, tmpStr.c_str(), lstrlen(tmpStr.c_str()));

   tmpStr = GetDlgItemTextA(IDC_DN_LONGITUDE);
   SetRegValue("Player", "Longitude", REG_SZ, tmpStr.c_str(), lstrlen(tmpStr.c_str()));

   tmpStr = GetDlgItemTextA(IDC_DN_LATITUDE);
   SetRegValue("Player", "Latitude", REG_SZ, tmpStr.c_str(), lstrlen(tmpStr.c_str()));

   tmpStr = GetDlgItemTextA(IDC_NUDGE_STRENGTH);
   SetRegValue("Player", "NudgeStrength", REG_SZ, tmpStr.c_str(), lstrlen(tmpStr.c_str()));

   HWND hwndFXAA = GetDlgItem(IDC_FXAACB).GetHwnd();
   size_t fxaa = SendMessage(hwndFXAA, CB_GETCURSEL, 0, 0);
   if (fxaa == LB_ERR)
      fxaa = 0;
   SetRegValue("Player", "FXAA", REG_DWORD, &fxaa, 4);

   HWND hwndScaleFXDMD = GetDlgItem(IDC_SCALE_FX_DMD).GetHwnd();
   size_t scaleFX_DMD = SendMessage(hwndScaleFXDMD, BM_GETCHECK, 0, 0);
   SetRegValue("Player", "ScaleFXDMD", REG_DWORD, &scaleFX_DMD, 4);

   HWND hwndBGSet = GetDlgItem(IDC_BG_SET).GetHwnd();
   size_t BGSet = SendMessage(hwndBGSet, BM_GETCHECK, 0, 0);
   SetRegValue("Player", "BGSet", REG_DWORD, &BGSet, 4);

   HWND hwndUseAA = GetDlgItem(IDC_AA_ALL_TABLES).GetHwnd();
   size_t useAA = SendMessage(hwndUseAA, BM_GETCHECK, 0, 0);
   SetRegValue("Player", "USEAA", REG_DWORD, &useAA, 4);

   HWND hwndUseDN = GetDlgItem(IDC_DYNAMIC_DN).GetHwnd();
   size_t useDN = SendMessage(hwndUseDN, BM_GETCHECK, 0, 0);
   SetRegValue("Player", "DynamicDayNight", REG_DWORD, &useDN, 4);

   HWND hwndUseAO = GetDlgItem(IDC_DYNAMIC_AO).GetHwnd();
   size_t useAO = SendMessage(hwndUseAO, BM_GETCHECK, 0, 0);
   SetRegValue("Player", "DynamicAO", REG_DWORD, &useAO, 4);

   hwndUseAO = GetDlgItem(IDC_ENABLE_AO).GetHwnd();
   useAO = SendMessage(hwndUseAO, BM_GETCHECK, 0, 0) ? 0 : 1; // inverted logic
   SetRegValue("Player", "DisableAO", REG_DWORD, &useAO, 4);

   HWND hwndStereo3D = GetDlgItem(IDC_3D_STEREO).GetHwnd();
   size_t stereo3D = SendMessage(hwndStereo3D, CB_GETCURSEL, 0, 0);
   if (stereo3D == LB_ERR)
      stereo3D = 0;
   SetRegValue("Player", "Stereo3D", REG_DWORD, &stereo3D, 4);
   SetRegValue("Player", "Stereo3DEnabled", REG_DWORD, &stereo3D, 4);

   HWND hwndStereo3DY = GetDlgItem(IDC_3D_STEREO_Y).GetHwnd();
   size_t stereo3DY = SendMessage(hwndStereo3DY, BM_GETCHECK, 0, 0);
   SetRegValue("Player", "Stereo3DYAxis", REG_DWORD, &stereo3DY, 4);

   HWND hwndForceAniso = GetDlgItem(IDC_FORCE_ANISO).GetHwnd();
   size_t forceAniso = SendMessage(hwndForceAniso, BM_GETCHECK, 0, 0);
   SetRegValue("Player", "ForceAnisotropicFiltering", REG_DWORD, &forceAniso, 4);

   HWND hwndTexCompress = GetDlgItem(IDC_TEX_COMPRESS).GetHwnd();
   size_t texCompress = SendMessage(hwndTexCompress, BM_GETCHECK, 0, 0);
   SetRegValue("Player", "CompressTextures", REG_DWORD, &texCompress, 4);

   HWND hwndSoftwareVP = GetDlgItem(IDC_SOFTWARE_VP).GetHwnd();
   size_t softwareVP = SendMessage(hwndSoftwareVP, BM_GETCHECK, 0, 0);
   SetRegValueBool("Player", "SoftwareVertexProcessing", softwareVP != 0);

   HWND hwndAraSlider = GetDlgItem(IDC_ARASlider).GetHwnd();
   size_t alphaRampsAccuracy = SendMessage(hwndAraSlider, TBM_GETPOS, 0, 0);
   SetRegValue("Player", "AlphaRampAccuracy", REG_DWORD, &alphaRampsAccuracy, 4);

   tmpStr = GetDlgItemTextA(IDC_3D_STEREO_OFS);
   SetRegValue("Player", "Stereo3DOffset", REG_SZ, tmpStr.c_str(), lstrlen(tmpStr.c_str()));

   tmpStr = GetDlgItemTextA(IDC_3D_STEREO_MS);
   SetRegValue("Player", "Stereo3DMaxSeparation", REG_SZ, tmpStr.c_str(), lstrlen(tmpStr.c_str()));

   tmpStr = GetDlgItemTextA(IDC_3D_STEREO_ZPD);
   SetRegValue("Player", "Stereo3DZPD", REG_SZ, tmpStr.c_str(), lstrlen(tmpStr.c_str()));

   HWND hwndDisableDWM = GetDlgItem(IDC_DISABLE_DWM).GetHwnd();
   size_t disableDWM = SendMessage(hwndDisableDWM, BM_GETCHECK, 0, 0);
   SetRegValue("Player", "DisableDWM", REG_DWORD, &disableDWM, 4);

   HWND hwndNvidiaApi = GetDlgItem(IDC_USE_NVIDIA_API_CHECK).GetHwnd();
   size_t nvidiaApi = SendMessage(hwndNvidiaApi, BM_GETCHECK, 0, 0);
   SetRegValue("Player", "UseNVidiaAPI", REG_DWORD, &nvidiaApi, 4);

   //HWND hwndBallStretchNo = GetDlgItem(hwndDlg, IDC_StretchNo);
   HWND hwndBallStretchYes = GetDlgItem(IDC_StretchYes).GetHwnd();
   HWND hwndBallStretchMonitor = GetDlgItem(IDC_StretchMonitor).GetHwnd();
   int ballStretchMode = 0;
   if (SendMessage(hwndBallStretchYes, BM_GETCHECK, 0, 0) == BST_CHECKED)
      ballStretchMode = 1;
   if (SendMessage(hwndBallStretchMonitor, BM_GETCHECK, 0, 0) == BST_CHECKED)
      ballStretchMode = 2;
   SetRegValue("Player", "BallStretchMode", REG_DWORD, &ballStretchMode, 4);

   // get selected Monitors
   // Monitors: 4:3, 16:9, 16:10, 21:10
   HWND hwndBallStretchCombo = GetDlgItem(IDC_MonitorCombo).GetHwnd();
   size_t selected = SendMessage(hwndBallStretchCombo, CB_GETCURSEL, 0, 0);
   if (selected == LB_ERR)
      selected = 1; // assume a 16:9 Monitor as standard
   SetRegValue("Player", "BallStretchMonitor", REG_DWORD, &selected, 4);

   const bool overwriteEnabled = IsDlgButtonChecked(IDC_OVERWRITE_BALL_IMAGE_CHECK) == BST_CHECKED;
   if (overwriteEnabled)
   {
      SetRegValueInt("Player", "OverwriteBallImage", 1);
      tmpStr = GetDlgItemText(IDC_BALL_IMAGE_EDIT);
      SetRegValueString("Player", "BallImage", tmpStr.c_str());
      tmpStr = GetDlgItemText(IDC_BALL_DECAL_EDIT);
      SetRegValueString("Player", "DecalImage", tmpStr.c_str());
   }
   else
      SetRegValueInt("Player", "OverwriteBallImage", 0);

   CDialog::OnOK();

}

void VideoOptionsDialog::OnClose()
{
   SendMessage(RESET_SIZELIST_CONTENT, 0, 0);
   CDialog::OnClose();
}
