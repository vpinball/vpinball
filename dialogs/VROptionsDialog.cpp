#include "stdafx.h"
#include "resource.h"
#include "VROptionsDialog.h"

#define GET_WINDOW_MODES		(WM_USER+100)

//static constexpr int rgwindowsize[] = { 640, 720, 800, 912, 1024, 1152, 1280, 1360, 1366, 1400, 1440, 1600, 1680, 1920, 2048, 2560, 3440, 3840, 4096, 5120, 6400, 7680, 8192, 11520, 15360 };  // windowed resolutions for selection list

constexpr float AAfactors[] = { 0.5f, 0.75f, 1.0f, 1.25f, (float)(4.0/3.0), 1.5f, 1.75f, 2.0f }; // factor is applied to width and to height, so 2.0f increases pixel count by 4. Additional values can be added.
constexpr int AAfactorCount = 8;

constexpr int MSAASamplesOpts[] = { 1, 4, 6, 8 };
constexpr int MSAASampleCount = 4;

static bool oldScaleValue = false;
static float scaleRelative = 1.0f;
static float scaleAbsolute = 55.0f;

static char rgszKeyName[][10] = {
   "",
   "Escape", //DIK_ESCAPE          0x01
   "1", //DIK_1               0x02
   "2", //DIK_2               0x03
   "3", //DIK_3               0x04
   "4", //DIK_4               0x05
   "5", //DIK_5               0x06
   "6", //DIK_6               0x07
   "7", //DIK_7               0x08
   "8", //DIK_8               0x09
   "9", //DIK_9               0x0A
   "0", //DIK_0               0x0B
   "-", //DIK_MINUS           0x0C    /* - on main keyboard */
   "=", //DIK_EQUALS          0x0D
   "Backspace", //DIK_BACK            0x0E    /* backspace */
   "Tab", //DIK_TAB             0x0F
   "Q", //DIK_Q               0x10
   "W", //DIK_W               0x11
   "E", //DIK_E               0x12
   "R", //DIK_R               0x13
   "T", //DIK_T               0x14
   "Y", //DIK_Y               0x15
   "U", //DIK_U               0x16
   "I", //DIK_I               0x17
   "O", //DIK_O               0x18
   "P", //DIK_P               0x19
   "[", //DIK_LBRACKET        0x1A
   "]", //DIK_RBRACKET        0x1B
   "Enter", //DIK_RETURN          0x1C    /* Enter on main keyboard */
   "L Ctrl", //DIK_LCONTROL        0x1D
   "A", //DIK_A               0x1E
   "S", //DIK_S               0x1F
   "D", //DIK_D               0x20
   "F", //DIK_F               0x21
   "G", //DIK_G               0x22
   "H", //DIK_H               0x23
   "J", //DIK_J               0x24
   "K", //DIK_K               0x25
   "L", //DIK_L               0x26
   ";", //DIK_SEMICOLON       0x27
   "'", //DIK_APOSTROPHE      0x28
   "`", //DIK_GRAVE           0x29    /* accent grave */
   "L Shift", //DIK_LSHIFT          0x2A
   "\\", //DIK_BACKSLASH       0x2B
   "Z", //DIK_Z               0x2C
   "X", //DIK_X               0x2D
   "C", //DIK_C               0x2E
   "V", //DIK_V               0x2F
   "B", //DIK_B               0x30
   "N", //DIK_N               0x31
   "M", //DIK_M               0x32
   ",", //DIK_COMMA           0x33
   ".", //DIK_PERIOD          0x34    /* . on main keyboard */
   "/", //DIK_SLASH           0x35    /* / on main keyboard */
   "R Shift", //DIK_RSHIFT          0x36
   "*", //DIK_MULTIPLY        0x37    /* * on numeric keypad */
   "Menu", //DIK_LMENU           0x38    /* left Alt */
   "Space", //DIK_SPACE           0x39
   "Caps Lock", //DIK_CAPITAL         0x3A
   "F1", //DIK_F1              0x3B
   "F2", //DIK_F2              0x3C
   "F3", //DIK_F3              0x3D
   "F4", //DIK_F4              0x3E
   "F5", //DIK_F5              0x3F
   "F6", //DIK_F6              0x40
   "F7", //DIK_F7              0x41
   "F8", //DIK_F8              0x42
   "F9", //DIK_F9              0x43
   "F10", //DIK_F10             0x44
   "NumLock", //DIK_NUMLOCK         0x45
   "ScrollLck", //DIK_SCROLL          0x46    /* Scroll Lock */
   "Numpad 7", //DIK_NUMPAD7         0x47
   "Numpad 8", //DIK_NUMPAD8         0x48
   "Numpad 9", //DIK_NUMPAD9         0x49
   "-", //DIK_SUBTRACT        0x4A    /* - on numeric keypad */
   "Numpad 4", //DIK_NUMPAD4         0x4B
   "Numpad 5", //DIK_NUMPAD5         0x4C
   "Numpad 6", //DIK_NUMPAD6         0x4D
   "+", //DIK_ADD             0x4E    /* + on numeric keypad */
   "Numpad 1", //DIK_NUMPAD1         0x4F
   "Numpad 2", //DIK_NUMPAD2         0x50
   "Numpad 3", //DIK_NUMPAD3         0x51
   "Numpad 0", //DIK_NUMPAD0         0x52
   "Numpad .", //DIK_DECIMAL         0x53    /* . on numeric keypad */
   "", //0x54
   "", //0x55
   "<>|", //DIK_OEM_102         0x56    /* < > | on UK/Germany keyboards */
   "F11", //DIK_F11             0x57
   "F12", //DIK_F12             0x58
   "", //0x59
   "", //0x5A
   "", //0x5B
   "", //0x5C
   "", //0x5D
   "", //0x5E
   "", //0x5F
   "", //0x60
   "", //0x61
   "", //0x62
   "", //0x63
   "F13", //DIK_F13             0x64    /*                     (NEC PC98) */
   "F14", //DIK_F14             0x65    /*                     (NEC PC98) */
   "F15", //DIK_F15             0x66    /*                     (NEC PC98) */
   "", //0x67
   "", //0x68
   "", //0x69
   "", //0x6A
   "", //0x6B
   "", //0x6C
   "", //0x6D
   "", //0x6E
   "", //0x6F

   "", //0x70
   "", //0x71
   "", //0x72
   "", //0x73
   "", //0x74
   "", //0x75
   "", //0x76
   "", //0x77
   "", //0x78
   "", //0x79
   "", //0x7A
   "", //0x7B
   "", //0x7C
   "", //0x7D
   "", //0x7E
   "", //0x7F

   "", //0x80
   "", //0x81
   "", //0x82
   "", //0x83
   "", //0x84
   "", //0x85
   "", //0x86
   "", //0x87
   "", //0x88
   "", //0x89
   "", //0x8A
   "", //0x8B
   "", //0x8C
   "", //0x8D
   "", //0x8E
   "", //0x8F

   "", //0x90
   "", //0x91
   "", //0x92
   "", //0x93
   "", //0x94
   "", //0x95
   "", //0x96
   "", //0x97
   "", //0x98
   "", //0x99
   "", //0x9A
   "", //0x9B

   "Num Enter", //#define DIK_NUMPADENTER     0x9C    /* Enter on numeric keypad */
   "R Ctrl", //DIK_RCONTROL        0x9D

   "", //0x9E
   "", //0x9F

   "", //0xA0
   "", //0xA1
   "", //0xA2
   "", //0xA3
   "", //0xA4
   "", //0xA5
   "", //0xA6
   "", //0xA7
   "", //0xA8
   "", //0xA9
   "", //0xAA
   "", //0xAB
   "", //0xAC
   "", //0xAD
   "", //0xAE
   "", //0xAF

   "", //0xB0
   "", //0xB1
   "", //0xB2
   "", //0xB3
   "", //0xB4

   "Numpad /", //#define DIK_DIVIDE          0xB5    /* / on numeric keypad */
   "", //0xB6
   "", //#define DIK_SYSRQ           0xB7
   "R Alt", //DIK_RMENU           0xB8    /* right Alt */

   "", //0xB9
   "", //0xBA
   "", //0xBB
   "", //0xBC
   "", //0xBD
   "", //0xBE
   "", //0xBF

   "", //0xC0
   "", //0xC1
   "", //0xC2
   "", //0xC3
   "", //0xC4
   "", //0xC5
   "", //0xC6

   "Home", //DIK_HOME            0xC7    /* Home on arrow keypad */
   "Up", //DIK_UP              0xC8    /* UpArrow on arrow keypad */
   "Page Up", //DIK_PRIOR           0xC9    /* PgUp on arrow keypad */
   "", //0xCA
   "Left", //DIK_LEFT            0xCB    /* LeftArrow on arrow keypad */
   "", //0xCC
   "Right", //DIK_RIGHT           0xCD    /* RightArrow on arrow keypad */
   "", //0xCE
   "End", //DIK_END             0xCF    /* End on arrow keypad */
   "Down", //DIK_DOWN            0xD0    /* DownArrow on arrow keypad */
   "Page Down", //DIK_NEXT            0xD1    /* PgDn on arrow keypad */
   "Insert", //DIK_INSERT          0xD2    /* Insert on arrow keypad */
   "Delete", //DIK_DELETE          0xD3    /* Delete on arrow keypad */

   "", //0xD4
   "", //0xD5
   "", //0xD6
   "", //0xD7
   "", //0xD8
   "", //0xD9
   "", //0xDA

   "L Windows", //DIK_LWIN            0xDB    /* Left Windows key */
   "R Windows", //DIK_RWIN            0xDC    /* Right Windows key */
   "Apps Menu", //DIK_APPS            0xDD    /* AppMenu key */
};

class KeyWindowStruct
{
public:
   PinInput pi;
   HWND hwndKeyControl; // window to get the key assignment
   UINT_PTR m_timerid; // timer id for our key assignment
};

WNDPROC g_ButtonProc2;

LRESULT CALLBACK MyKeyButtonProc2(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   if (uMsg == WM_GETDLGCODE)
      // Eat all acceleratable messges
      return (DLGC_WANTARROWS | DLGC_WANTTAB | DLGC_WANTALLKEYS | DLGC_WANTCHARS);
   else
      return CallWindowProc(g_ButtonProc2, hwnd, uMsg, wParam, lParam);
}

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

VROptionsDialog::VROptionsDialog() : CDialog(IDD_VR_OPTIONS)
{
   // Adjust keyboard translation to user keyboard layout
   HKL layout = GetKeyboardLayout(0);
   BYTE State[256];
   // Translate for a default keyboard state
   //if (GetKeyboardState(State) != FALSE)
   memset(State, 0, sizeof(State));
   State[VK_CAPITAL] = 1; // Get CAPS LOCK variant of the key
   for (int scancode = 0x00; scancode <= 0xDD; scancode++)
   {
      BYTE result[2];
      UINT vk = MapVirtualKeyEx(scancode, 1, layout);
      if (rgszKeyName[scancode][1] == 0 && ToAsciiEx(vk, scancode, State, (LPWORD)result, 0, layout) == 1)
      {
         rgszKeyName[scancode][0] = (char)result[0];
         rgszKeyName[scancode][1] = 0;
      }
   }
}

void VROptionsDialog::AddToolTip(const char * const text, HWND parentHwnd, HWND toolTipHwnd, HWND controlHwnd)
{
   TOOLINFO toolInfo = { };
   toolInfo.cbSize = sizeof(toolInfo);
   toolInfo.hwnd = parentHwnd;
   toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
   toolInfo.uId = (UINT_PTR)controlHwnd;
   toolInfo.lpszText = (char*)text;
   SendMessage(toolTipHwnd, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);
}

void VROptionsDialog::ResetVideoPreferences()
{
   const HWND hwndDlg = GetHwnd();
   const HWND toolTipHwnd = CreateWindowEx(NULL, TOOLTIPS_CLASS, NULL, WS_POPUP | TTS_ALWAYSTIP | TTS_BALLOON, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hwndDlg, NULL, g_pvp->theInstance, NULL);
   if (toolTipHwnd)
   {
      SendMessage(toolTipHwnd, TTM_SETMAXTIPWIDTH, 0, 180);
      HWND controlHwnd = GetDlgItem(IDC_CAP_EXTDMD).GetHwnd();
      AddToolTip((LPSTR) "Attempt to capture External DMD window such as Freezy, UltraDMD or P-ROC.\r\n\r\nFor Freezy DmdDevice.ini need to have 'stayontop = true'.", hwndDlg, toolTipHwnd, controlHwnd);
      controlHwnd = GetDlgItem(IDC_CAP_PUP).GetHwnd();
      AddToolTip((LPSTR) "Attempt to capture PUP player window and display it as a Backglass in VR.", hwndDlg, toolTipHwnd, controlHwnd);
   }

   char tmp[256];
   constexpr float nudgeStrength = 2e-2f;
   sprintf_s(tmp, sizeof(tmp), "%f", nudgeStrength);
   SetDlgItemText(IDC_NUDGE_STRENGTH, tmp);

   SendMessage(GetDlgItem(IDC_SUPER_SAMPLING_COMBO).GetHwnd(), TBM_SETPOS, TRUE, getBestMatchingAAfactorIndex(1.0f));
   SetDlgItemText(IDC_SUPER_SAMPLING_LABEL, "Supersampling Factor: 1.0");
   SendMessage(GetDlgItem(IDC_MSAA_COMBO).GetHwnd(), TBM_SETPOS, TRUE, 1);
   SetDlgItemText(IDC_MSAA_LABEL, "MSAA Samples: Disabled");

   SendMessage(GetDlgItem(IDC_DYNAMIC_AO).GetHwnd(), BM_SETCHECK, BST_UNCHECKED, 0);
   SendMessage(GetDlgItem(IDC_ENABLE_AO).GetHwnd(), BM_SETCHECK, BST_UNCHECKED, 0);
   SendMessage(GetDlgItem(IDC_GLOBAL_SSREFLECTION_CHECK).GetHwnd(), BM_SETCHECK, BST_UNCHECKED, 0);

   SendMessage(GetDlgItem(IDC_POST_PROCESS_COMBO).GetHwnd(), CB_SETCURSEL, 0, 0);
   SendMessage(GetDlgItem(IDC_SHARPEN_COMBO).GetHwnd(), CB_SETCURSEL, 0, 0);
   SendMessage(GetDlgItem(IDC_SCALE_FX_DMD).GetHwnd(), BM_SETCHECK, false ? BST_CHECKED : BST_UNCHECKED, 0);

   constexpr float vrSlope = 6.5f;
   sprintf_s(tmp, sizeof(tmp), "%0.1f", vrSlope);
   SetDlgItemText(IDC_VR_SLOPE, tmp);

   constexpr float vrOrientation = 0.0f;
   sprintf_s(tmp, sizeof(tmp), "%0.1f", vrOrientation);
   SetDlgItemText(IDC_3D_VR_ORIENTATION, tmp);

   constexpr float vrX = 0.0f;
   sprintf_s(tmp, sizeof(tmp), "%0.1f", vrX);
   SetDlgItemText(IDC_VR_OFFSET_X, tmp);

   constexpr float vrY = 0.0f;
   sprintf_s(tmp, sizeof(tmp), "%0.1f", vrY);
   SetDlgItemText(IDC_VR_OFFSET_Y, tmp);

   constexpr float vrZ = 80.0f;
   sprintf_s(tmp, sizeof(tmp), "%0.1f", vrZ);
   SetDlgItemText(IDC_VR_OFFSET_Z, tmp);

   SendMessage(GetDlgItem(IDC_BLOOM_OFF).GetHwnd(), BM_SETCHECK, false ? BST_CHECKED : BST_UNCHECKED, 0);
   SendMessage(GetDlgItem(IDC_TURN_VR_ON).GetHwnd(), CB_SETCURSEL, 1, 0);

   SendMessage(GetDlgItem(IDC_DMD_SOURCE).GetHwnd(), CB_SETCURSEL, 1, 0);
   SendMessage(GetDlgItem(IDC_BG_SOURCE).GetHwnd(), CB_SETCURSEL, 1, 0);

   SendMessage(GetDlgItem(IDC_VR_PREVIEW).GetHwnd(), CB_SETCURSEL, 0, 0);

   constexpr bool scaleToFixedWidth = false;
   oldScaleValue = scaleToFixedWidth;
   SendMessage(GetDlgItem(IDC_SCALE_TO_CM).GetHwnd(), BM_SETCHECK, scaleToFixedWidth ? BST_CHECKED : BST_UNCHECKED, 0);

   scaleRelative = 1.0f;
   scaleAbsolute = 55.0f;
   sprintf_s(tmp, sizeof(tmp), scaleToFixedWidth ? "%0.1f" : "%0.3f", scaleToFixedWidth ? scaleAbsolute : scaleRelative);
   SetDlgItemText(IDC_VR_SCALE, tmp);

   constexpr float vrNearPlane = 5.0f;
   sprintf_s(tmp, sizeof(tmp), "%0.1f", vrNearPlane);
   SetDlgItemText(IDC_NEAR_PLANE, tmp);

   constexpr float vrFarPlane = 5000.0f;
   sprintf_s(tmp, sizeof(tmp), "%0.1f", vrFarPlane);
   SetDlgItemText(IDC_FAR_PLANE, tmp);

   //AMD Debug
   SendMessage(GetDlgItem(IDC_COMBO_TEXTURE).GetHwnd(), CB_SETCURSEL, 1, 0);
}

BOOL VROptionsDialog::OnInitDialog()
{
   const HWND hwndDlg = GetHwnd();
   const HWND toolTipHwnd = CreateWindowEx(NULL, TOOLTIPS_CLASS, NULL, WS_POPUP | TTS_ALWAYSTIP | TTS_BALLOON, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hwndDlg, NULL, g_pvp->theInstance, NULL);
   if (toolTipHwnd)
   {
      SendMessage(toolTipHwnd, TTM_SETMAXTIPWIDTH, 0, 180);
      AddToolTip("Forces the bloom filter to be always off. Only for very low-end graphics cards.", hwndDlg, toolTipHwnd, GetDlgItem(IDC_BLOOM_OFF).GetHwnd());
      AddToolTip("Disable Autodetect if Visual Pinball does not start up.", hwndDlg, toolTipHwnd, GetDlgItem(IDC_TURN_VR_ON).GetHwnd());
      AddToolTip("What sources should be used for DMD?\nOnly internal supplied by script/Text Label/Flasher\nScreenreader (see screenreader.ini)\nFrom Shared Memory API", hwndDlg, toolTipHwnd, GetDlgItem(IDC_DMD_SOURCE).GetHwnd());
      AddToolTip("What sources should be used for Backglass?\nOnly internal background\nTry to open a directb2s file\ndirectb2s file dialog\nScreenreader (see screenreader.ini)\nFrom Shared Memory API", hwndDlg, toolTipHwnd, GetDlgItem(IDC_BG_SOURCE).GetHwnd());
      AddToolTip("Changes the visual effect/screen shaking when nudging the table.", hwndDlg, toolTipHwnd, GetDlgItem(IDC_NUDGE_STRENGTH).GetHwnd());
      AddToolTip("Activate this to enable dynamic Ambient Occlusion.\r\n\r\nThis slows down performance, but enables contact shadows for dynamic objects.", hwndDlg, toolTipHwnd, GetDlgItem(IDC_DYNAMIC_AO).GetHwnd());
      AddToolTip("Activate this to enable Ambient Occlusion.\r\nThis enables contact shadows between objects.", hwndDlg, toolTipHwnd, GetDlgItem(IDC_ENABLE_AO).GetHwnd());
      AddToolTip("Enables post-processed Anti-Aliasing.\r\n\r\nThese settings can make the image quality a bit smoother at cost of performance and a slight blurring.", hwndDlg, toolTipHwnd, GetDlgItem(IDC_POST_PROCESS_COMBO).GetHwnd());
      AddToolTip("Enables post-processed Sharpening of the image.\r\n'Bilateral CAS' is recommended,\nbut will harm performance on low-end graphics cards.\r\n'CAS' is less aggressive and faster, but also rather subtle.", hwndDlg, toolTipHwnd, GetDlgItem(IDC_SHARPEN_COMBO).GetHwnd());
      AddToolTip("Select which eye(s) to be displayed on the computer screen.", hwndDlg, toolTipHwnd, GetDlgItem(IDC_VR_PREVIEW).GetHwnd());
      AddToolTip("Enables brute-force Up/Downsampling.\r\n\r\nThis delivers very good quality but has a significant impact on performance.\r\n\r\n2.0 means twice the resolution to be handled while rendering.", hwndDlg, toolTipHwnd, GetDlgItem(IDC_SUPER_SAMPLING_COMBO).GetHwnd());
      AddToolTip("Set the amount of MSAA samples.\r\n\r\nMSAA can help reduce geometry aliasing in VR at the cost of performance and GPU memory.\r\n\r\nThis can really help improve image quality when not using supersampling.", hwndDlg, toolTipHwnd, GetDlgItem(IDC_MSAA_COMBO).GetHwnd());
      AddToolTip("Enables post-processed reflections on all objects (beside the playfield).", hwndDlg, toolTipHwnd, GetDlgItem(IDC_GLOBAL_SSREFLECTION_CHECK).GetHwnd());
      AddToolTip("Enables playfield reflections.\r\n\r\n'Dynamic' is recommended and will give the best result but may harm performance.\r\n\r\n'Static Only' has no performance cost except in VR.\r\n\r\nOther options are trade-off between quality and performance.", hwndDlg, toolTipHwnd, GetDlgItem(IDC_GLOBAL_PF_REFLECTION).GetHwnd());
      //AMD Debug
      AddToolTip("Pixel format for VR Rendering.", hwndDlg, toolTipHwnd, GetDlgItem(IDC_COMBO_TEXTURE).GetHwnd());
   }

   char tmp[256];

   const float nudgeStrength = LoadValueWithDefault(regKey[RegName::PlayerVR], "NudgeStrength"s, LoadValueWithDefault(regKey[RegName::Player], "NudgeStrength"s, 2e-2f));
   sprintf_s(tmp, sizeof(tmp), "%f", nudgeStrength);
   SetDlgItemText(IDC_NUDGE_STRENGTH, tmp);

   const float AAfactor = LoadValueWithDefault(regKey[RegName::PlayerVR], "AAFactor", LoadValueWithDefault(regKey[RegName::Player], "USEAA"s, false) ? 1.5f : 1.0f);
   const HWND hwndSSSlider = GetDlgItem(IDC_SUPER_SAMPLING_COMBO).GetHwnd();
   SendMessage(hwndSSSlider, TBM_SETRANGE, fTrue, MAKELONG(0, AAfactorCount - 1));
   SendMessage(hwndSSSlider, TBM_SETTICFREQ, 1, 0);
   SendMessage(hwndSSSlider, TBM_SETLINESIZE, 0, 1);
   SendMessage(hwndSSSlider, TBM_SETPAGESIZE, 0, 1);
   SendMessage(hwndSSSlider, TBM_SETTHUMBLENGTH, 5, 0);
   SendMessage(hwndSSSlider, TBM_SETPOS, TRUE, getBestMatchingAAfactorIndex(AAfactor));
   char SSText[32];
   sprintf_s(SSText, sizeof(SSText), "Supersampling Factor: %.2f", AAfactor);
   SetDlgItemText(IDC_SUPER_SAMPLING_LABEL, SSText);

   const int MSAASamples = LoadValueWithDefault(regKey[RegName::PlayerVR], "MSAASamples"s, 1);
   const auto CurrMSAAPos = std::find(MSAASamplesOpts, MSAASamplesOpts + (sizeof(MSAASamplesOpts) / sizeof(MSAASamplesOpts[0])), MSAASamples);
   const HWND hwndMSAASlider = GetDlgItem(IDC_MSAA_COMBO).GetHwnd();
   SendMessage(hwndMSAASlider, TBM_SETRANGE, fTrue, MAKELONG(0, MSAASampleCount - 1));
   SendMessage(hwndMSAASlider, TBM_SETTICFREQ, 1, 0);
   SendMessage(hwndMSAASlider, TBM_SETLINESIZE, 0, 1);
   SendMessage(hwndMSAASlider, TBM_SETPAGESIZE, 0, 1);
   SendMessage(hwndMSAASlider, TBM_SETTHUMBLENGTH, 5, 0);
   SendMessage(hwndMSAASlider, TBM_SETPOS, TRUE, (LPARAM)std::distance(MSAASamplesOpts, CurrMSAAPos));
   char MSAAText[52];
   if (MSAASamples == 1)
   {
      sprintf_s(MSAAText, sizeof(MSAAText), "MSAA Samples: Disabled");
   }
   else
   {
      sprintf_s(MSAAText, sizeof(MSAAText), "MSAA Samples: %d", MSAASamples);
   }
   SetDlgItemText(IDC_MSAA_LABEL, MSAAText);

   bool useAO = LoadValueWithDefault(regKey[RegName::PlayerVR], "DynamicAO"s, LoadValueWithDefault(regKey[RegName::Player], "DynamicAO", false));
   SendMessage(GetDlgItem(IDC_DYNAMIC_AO).GetHwnd(), BM_SETCHECK, useAO ? BST_CHECKED : BST_UNCHECKED, 0);

   useAO = LoadValueWithDefault(regKey[RegName::PlayerVR], "DisableAO"s, LoadValueWithDefault(regKey[RegName::Player], "DisableAO"s, false));
   SendMessage(GetDlgItem(IDC_ENABLE_AO).GetHwnd(), BM_SETCHECK, useAO ? BST_UNCHECKED : BST_CHECKED, 0); // inverted logic
   GetDlgItem(IDC_DYNAMIC_AO).EnableWindow(!useAO);

   const bool ssreflection = LoadValueWithDefault(regKey[RegName::PlayerVR], "SSRefl"s, LoadValueWithDefault(regKey[RegName::Player], "SSRefl"s, false));
   SendMessage(GetDlgItem(IDC_GLOBAL_SSREFLECTION_CHECK).GetHwnd(), BM_SETCHECK, ssreflection ? BST_CHECKED : BST_UNCHECKED, 0);

   HWND hwnd = GetDlgItem(IDC_GLOBAL_PF_REFLECTION).GetHwnd();
   SendMessage(hwnd, WM_SETREDRAW, FALSE, 0); // to speed up adding the entries :/
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Disabled");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Balls Only");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Dynamic");
   int pfr = LoadValueWithDefault(regKey[RegName::PlayerVR], "PFReflection"s, -1);
   RenderProbe::ReflectionMode pfReflection;
   if (pfr != -1)
      pfReflection = (RenderProbe::ReflectionMode)pfr;
   else
   {
      pfReflection = RenderProbe::REFL_NONE;
      if (LoadValueWithDefault(regKey[RegName::PlayerVR], "BallReflection"s, true))
         pfReflection = RenderProbe::REFL_BALLS;
      if (LoadValueWithDefault(regKey[RegName::PlayerVR], "PFRefl"s, true))
         pfReflection = RenderProbe::REFL_DYNAMIC;
   }
   if (pfReflection == RenderProbe::REFL_DYNAMIC)
      SendMessage(hwnd, CB_SETCURSEL, 2, 0);
   else
      SendMessage(hwnd, CB_SETCURSEL, pfReflection, 0);
   SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);

   const int fxaa = LoadValueWithDefault(regKey[RegName::PlayerVR], "FXAA"s, LoadValueWithDefault(regKey[RegName::Player], "FXAA"s, 0));
   hwnd = GetDlgItem(IDC_POST_PROCESS_COMBO).GetHwnd();
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"Disabled");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"Fast FXAA");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"Standard FXAA");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"Quality FXAA");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"Fast NFAA");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"Standard DLAA");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"Quality SMAA");
   SendMessage(hwnd, CB_SETCURSEL, fxaa, 0);

   const int sharpen = LoadValueWithDefault(regKey[RegName::PlayerVR], "Sharpen"s, 0);
   hwnd = GetDlgItem(IDC_SHARPEN_COMBO).GetHwnd();
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Disabled");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "CAS");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Bilateral CAS");
   SendMessage(hwnd, CB_SETCURSEL, sharpen, 0);

   const bool scaleFX_DMD = LoadValueWithDefault(regKey[RegName::PlayerVR], "ScaleFXDMD"s, LoadValueWithDefault(regKey[RegName::Player], "ScaleFXDMD"s, false));
   SendMessage(GetDlgItem(IDC_SCALE_FX_DMD).GetHwnd(), BM_SETCHECK, scaleFX_DMD ? BST_CHECKED : BST_UNCHECKED, 0);

   const VRPreviewMode vrPreview = (VRPreviewMode)LoadValueWithDefault(regKey[RegName::PlayerVR], "VRPreview"s, VRPREVIEW_LEFT);
   hwnd = GetDlgItem(IDC_VR_PREVIEW).GetHwnd();
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Disabled");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Left Eye");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Right Eye");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Both Eyes");
   SendMessage(hwnd, CB_SETCURSEL, (int)vrPreview, 0);

   const bool scaleToFixedWidth = LoadValueWithDefault(regKey[RegName::PlayerVR], "scaleToFixedWidth"s, false);
   oldScaleValue = scaleToFixedWidth;
   SendMessage(GetDlgItem(IDC_SCALE_TO_CM).GetHwnd(), BM_SETCHECK, scaleToFixedWidth ? BST_CHECKED : BST_UNCHECKED, 0);

   scaleRelative = LoadValueWithDefault(regKey[RegName::PlayerVR], "scaleRelative"s, 1.0f);
   scaleAbsolute = LoadValueWithDefault(regKey[RegName::PlayerVR], "scaleAbsolute"s, 55.0f);

   sprintf_s(tmp, sizeof(tmp), scaleToFixedWidth ? "%0.1f" : "%0.3f", scaleToFixedWidth ? scaleAbsolute : scaleRelative);
   SetDlgItemText(IDC_VR_SCALE, tmp);

   const float vrNearPlane = LoadValueWithDefault(regKey[RegName::PlayerVR], "nearPlane"s, 5.0f);
   sprintf_s(tmp, sizeof(tmp), "%0.1f", vrNearPlane);
   SetDlgItemText(IDC_NEAR_PLANE, tmp);

   const float vrFarPlane = LoadValueWithDefault(regKey[RegName::PlayerVR], "farPlane"s, 5000.0f);
   sprintf_s(tmp, sizeof(tmp), "%0.1f", vrFarPlane);
   SetDlgItemText(IDC_FAR_PLANE, tmp);

   const float vrSlope = LoadValueWithDefault(regKey[RegName::PlayerVR], "Slope"s, 6.5f);
   sprintf_s(tmp, sizeof(tmp), "%0.2f", vrSlope);
   SetDlgItemText(IDC_VR_SLOPE, tmp);

   const float vrOrientation = LoadValueWithDefault(regKey[RegName::PlayerVR], "Orientation"s, 0.0f);
   sprintf_s(tmp, sizeof(tmp), "%0.1f", vrOrientation);
   SetDlgItemText(IDC_3D_VR_ORIENTATION, tmp);

   const float vrX = LoadValueWithDefault(regKey[RegName::PlayerVR], "TableX"s, 0.0f);
   sprintf_s(tmp, sizeof(tmp), "%0.1f", vrX);
   SetDlgItemText(IDC_VR_OFFSET_X, tmp);

   const float vrY = LoadValueWithDefault(regKey[RegName::PlayerVR], "TableY"s, 0.0f);
   sprintf_s(tmp, sizeof(tmp), "%0.1f", vrY);
   SetDlgItemText(IDC_VR_OFFSET_Y, tmp);

   const float vrZ = LoadValueWithDefault(regKey[RegName::PlayerVR], "TableZ"s, 80.0f);
   sprintf_s(tmp, sizeof(tmp), "%0.1f", vrZ);
   SetDlgItemText(IDC_VR_OFFSET_Z, tmp);

   const bool bloomOff = LoadValueWithDefault(regKey[RegName::PlayerVR], "ForceBloomOff"s, LoadValueWithDefault(regKey[RegName::Player], "ForceBloomOff"s, false));
   SendMessage(GetDlgItem(IDC_BLOOM_OFF).GetHwnd(), BM_SETCHECK, bloomOff ? BST_CHECKED : BST_UNCHECKED, 0);

   const int askToTurnOn = LoadValueWithDefault(regKey[RegName::PlayerVR], "AskToTurnOn"s, 1);
   SendMessage(GetDlgItem(IDC_TURN_VR_ON).GetHwnd(), CB_ADDSTRING, 0, (LPARAM)"VR enabled");
   SendMessage(GetDlgItem(IDC_TURN_VR_ON).GetHwnd(), CB_ADDSTRING, 0, (LPARAM)"VR autodetect");
   SendMessage(GetDlgItem(IDC_TURN_VR_ON).GetHwnd(), CB_ADDSTRING, 0, (LPARAM)"VR disabled");
   SendMessage(GetDlgItem(IDC_TURN_VR_ON).GetHwnd(), CB_SETCURSEL, askToTurnOn, 0);

   const int DMDsource = LoadValueWithDefault(regKey[RegName::PlayerVR], "DMDsource"s, 1); // Unimplemented for the time being
   SendMessage(GetDlgItem(IDC_DMD_SOURCE).GetHwnd(), CB_ADDSTRING, 0, (LPARAM)"Internal Text/Flasher (via vbscript)");
   SendMessage(GetDlgItem(IDC_DMD_SOURCE).GetHwnd(), CB_ADDSTRING, 0, (LPARAM)"Screenreader");
   SendMessage(GetDlgItem(IDC_DMD_SOURCE).GetHwnd(), CB_ADDSTRING, 0, (LPARAM)"SharedMemory API");
   SendMessage(GetDlgItem(IDC_DMD_SOURCE).GetHwnd(), CB_SETCURSEL, DMDsource, 0);

   bool on = LoadValueWithDefault(regKey[RegName::Player], "CaptureExternalDMD"s, false);
   ::SendMessage(GetDlgItem(IDC_CAP_EXTDMD).GetHwnd(), BM_SETCHECK, on ? BST_CHECKED : BST_UNCHECKED, 0);

   const int BGsource = LoadValueWithDefault(regKey[RegName::PlayerVR], "BGsource"s, 1); // Unimplemented for the time being
   SendMessage(GetDlgItem(IDC_BG_SOURCE).GetHwnd(), CB_ADDSTRING, 0, (LPARAM)"Default table background");
   SendMessage(GetDlgItem(IDC_BG_SOURCE).GetHwnd(), CB_ADDSTRING, 0, (LPARAM)"directb2s File (auto only)");
   SendMessage(GetDlgItem(IDC_BG_SOURCE).GetHwnd(), CB_ADDSTRING, 0, (LPARAM)"directb2s File");
   SendMessage(GetDlgItem(IDC_BG_SOURCE).GetHwnd(), CB_ADDSTRING, 0, (LPARAM)"SharedMemory API");
   SendMessage(GetDlgItem(IDC_BG_SOURCE).GetHwnd(), CB_SETCURSEL, BGsource, 0);

   on = LoadValueWithDefault(regKey[RegName::Player], "CapturePUP"s, false);
   ::SendMessage(GetDlgItem(IDC_CAP_PUP).GetHwnd(), BM_SETCHECK, on ? BST_CHECKED : BST_UNCHECKED, 0);

   //AMD Debugging
   SendMessage(GetDlgItem(IDC_COMBO_TEXTURE).GetHwnd(), CB_ADDSTRING, 0, (LPARAM)"RGB 8");
   SendMessage(GetDlgItem(IDC_COMBO_TEXTURE).GetHwnd(), CB_ADDSTRING, 0, (LPARAM)"RGBA 8 (Recommended)");
   SendMessage(GetDlgItem(IDC_COMBO_TEXTURE).GetHwnd(), CB_ADDSTRING, 0, (LPARAM)"RGB 16F");
   SendMessage(GetDlgItem(IDC_COMBO_TEXTURE).GetHwnd(), CB_ADDSTRING, 0, (LPARAM)"RGBA 16F");
   const int textureModeVR = LoadValueWithDefault(regKey[RegName::PlayerVR], "EyeFBFormat"s, 1);
   SendMessage(GetDlgItem(IDC_COMBO_TEXTURE).GetHwnd(), CB_SETCURSEL, textureModeVR, 0);

   int key;
   for (unsigned int i = eTableRecenter; i <= eTableDown; ++i)
      if (regkey_idc[i] != -1)
      {
         const HRESULT hr = LoadValue(regKey[RegName::Player], regkey_string[i], key);
         if (hr != S_OK || key > 0xdd)
            key = regkey_defdik[i];
         const HWND hwndControl = GetDlgItem(regkey_idc[i]);
         ::SetWindowText(hwndControl, rgszKeyName[key]);
         ::SetWindowLongPtr(hwndControl, GWLP_USERDATA, key);
      }

   for (unsigned int i = 0; i < 3; ++i)
   {
      HRESULT hr;
      int item, selected;
      switch (i)
      {
      case 0: hr = LoadValue(regKey[RegName::Player], "JoyTableRecenterKey"s, selected); item = IDC_JOYTABLERECENTER; break;
      case 1: hr = LoadValue(regKey[RegName::Player], "JoyTableUpKey"s, selected); item = IDC_JOYTABLEUP; break;
      case 2: hr = LoadValue(regKey[RegName::Player], "JoyTableDownKey"s, selected); item = IDC_JOYTABLEDOWN; break;
      }
      
      if (hr != S_OK)
         selected = 0; // assume no assignment as standard

      const HWND hwnd = GetDlgItem(item).GetHwnd();
      ::SendMessage(hwnd, WM_SETREDRAW, FALSE, 0); // to speed up adding the entries :/
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "(none)");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Button 1");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Button 2");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Button 3");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Button 4");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Button 5");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Button 6");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Button 7");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Button 8");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Button 9");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Button 10");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Button 11");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Button 12");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Button 13");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Button 14");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Button 15");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Button 16");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Button 17");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Button 18");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Button 19");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Button 20");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Button 21");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Button 22");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Button 23");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Button 24");
      if (item == IDC_JOYLFLIPCOMBO || item == IDC_JOYRFLIPCOMBO || item == IDC_JOYPLUNGERCOMBO)
      {
         ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Left Mouse");
         ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Right Mouse");
         ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Middle Mouse");
      }
      ::SendMessage(hwnd, CB_SETCURSEL, selected, 0);
      ::SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);
   }

   //

   KeyWindowStruct* const pksw = new KeyWindowStruct();
   pksw->pi.Init(GetHwnd());
   pksw->m_timerid = 0;
   ::SetWindowLongPtr(GetHwnd(), GWLP_USERDATA, (size_t)pksw);

   // Set buttons to ignore keyboard shortcuts when using DirectInput
   HWND hwndButton = GetDlgItem(IDC_BTTABLERECENTER).GetHwnd();
   g_ButtonProc2 = (WNDPROC)::GetWindowLongPtr(hwndButton, GWLP_WNDPROC);
   ::SetWindowLongPtr(hwndButton, GWLP_WNDPROC, (size_t)MyKeyButtonProc2);
   ::SetWindowLongPtr(hwndButton, GWLP_USERDATA, (size_t)pksw);

   hwndButton = GetDlgItem(IDC_BTTABLEUP).GetHwnd();
   ::SetWindowLongPtr(hwndButton, GWLP_WNDPROC, (size_t)MyKeyButtonProc2);
   ::SetWindowLongPtr(hwndButton, GWLP_USERDATA, (size_t)pksw);

   hwndButton = GetDlgItem(IDC_BTTABLEDOWN).GetHwnd();
   ::SetWindowLongPtr(hwndButton, GWLP_WNDPROC, (size_t)MyKeyButtonProc2);
   ::SetWindowLongPtr(hwndButton, GWLP_USERDATA, (size_t)pksw);

   return TRUE;
}

INT_PTR VROptionsDialog::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   switch (uMsg)
   {
   case WM_TIMER:
   {
      KeyWindowStruct* const pksw = (KeyWindowStruct*)::GetWindowLongPtr(GetHwnd(), GWLP_USERDATA);
      const int key = pksw->pi.GetNextKey();
      if (key != 0)
      {
         if (key < 0xDD) // Key mapping
         {
            int key_esc;
            const HRESULT hr = LoadValue(regKey[RegName::Player], "EscapeKey"s, key_esc);
            if (hr != S_OK || key_esc > 0xdd)
               key_esc = DIK_ESCAPE;

            if (key == key_esc)
            {
               // reset key to old value
               const size_t oldkey = ::GetWindowLongPtr(pksw->hwndKeyControl, GWLP_USERDATA);
               ::SetWindowText(pksw->hwndKeyControl, rgszKeyName[oldkey]);
            }
            else
            {
               ::SetWindowText(pksw->hwndKeyControl, rgszKeyName[key]);
               ::SetWindowLongPtr(pksw->hwndKeyControl, GWLP_USERDATA, key);
            }
            ::KillTimer(GetHwnd(), pksw->m_timerid);
            pksw->m_timerid = 0;
         }
      }
      break;
   }
   case WM_HSCROLL:
      {
         if ((HWND)lParam == GetDlgItem(IDC_SUPER_SAMPLING_COMBO).GetHwnd()) {
            const size_t posAAfactor = SendMessage(GetDlgItem(IDC_SUPER_SAMPLING_COMBO).GetHwnd(), TBM_GETPOS, 0, 0);//Reading the value from wParam does not work reliable
            const float AAfactor = ((posAAfactor) < AAfactorCount) ? AAfactors[posAAfactor] : 1.0f;
            char newText[32];
            sprintf_s(newText, sizeof(newText), "Supersampling Factor: %.2f", AAfactor);
            SetDlgItemText(IDC_SUPER_SAMPLING_LABEL, newText);
         }
         else if ((HWND)lParam == GetDlgItem(IDC_MSAA_COMBO).GetHwnd()) {
            const size_t posMSAA = SendMessage(GetDlgItem(IDC_MSAA_COMBO).GetHwnd(), TBM_GETPOS, 0, 0);//Reading the value from wParam does not work reliable
            const int MSAASampleAmount = MSAASamplesOpts[posMSAA];
            char newText[52];
            if (MSAASampleAmount == 1)
            {
               sprintf_s(newText, sizeof(newText), "MSAA Samples: Disabled");
            }
            else
            {
               sprintf_s(newText, sizeof(newText), "MSAA Samples: %d", MSAASampleAmount);
            }
            SetDlgItemText(IDC_MSAA_LABEL, newText);
         }
         break;
      }
   }

   return DialogProcDefault(uMsg, wParam, lParam);
}

BOOL VROptionsDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
   UNREFERENCED_PARAMETER(lParam);

   switch (LOWORD(wParam))
   {
      case IDC_DEFAULTS:
      {
         ResetVideoPreferences();
         break;
      }
      case IDC_SCALE_TO_CM:
      {
         const bool newScaleValue = SendMessage(GetDlgItem(IDC_SCALE_TO_CM).GetHwnd(), BM_GETCHECK, 0, 0) > 0;
         if (oldScaleValue != newScaleValue) {
            CString tmpStr = GetDlgItemText(IDC_VR_SCALE);
            tmpStr.Replace(',', '.');
            if (oldScaleValue)
               scaleAbsolute = (float)atof(tmpStr.c_str());
            else
               scaleRelative = (float)atof(tmpStr.c_str());

            char tmp[256];
            sprintf_s(tmp, sizeof(tmp), newScaleValue ? "%0.1f" : "%0.3f", newScaleValue ? scaleAbsolute : scaleRelative);
            SetDlgItemText(IDC_VR_SCALE, tmp);
            oldScaleValue = newScaleValue;
         }
         break;
      }
      case IDC_ENABLE_AO:
      {
         const size_t checked = SendDlgItemMessage(IDC_ENABLE_AO, BM_GETCHECK, 0, 0);
         GetDlgItem(IDC_DYNAMIC_AO).EnableWindow(checked ? 1 : 0);
         break;
      }
      case IDC_BTTABLERECENTER:
      {
         if (HIWORD(wParam) == BN_CLICKED)
            StartTimer(IDC_TABLEREC_TEXT);
         break;
      }
      case IDC_BTTABLEUP:
      {
         if (HIWORD(wParam) == BN_CLICKED)
            StartTimer(IDC_TABLEUP_TEXT);
         break;
      }
      case IDC_BTTABLEDOWN:
      {
         if (HIWORD(wParam) == BN_CLICKED)
            StartTimer(IDC_TABLEDOWN_TEXT);
         break;
      }
      default:
         return FALSE;
   }
   return TRUE;
}

void VROptionsDialog::OnOK()
{
   SaveValue(regKey[RegName::PlayerVR], "NudgeStrength"s, GetDlgItemText(IDC_NUDGE_STRENGTH).c_str());

   LRESULT fxaa = SendMessage(GetDlgItem(IDC_POST_PROCESS_COMBO).GetHwnd(), CB_GETCURSEL, 0, 0);
   if (fxaa == LB_ERR)
      fxaa = 0;
   SaveValue(regKey[RegName::PlayerVR], "FXAA"s, (int)fxaa);

   LRESULT sharpen = SendMessage(GetDlgItem(IDC_SHARPEN_COMBO).GetHwnd(), CB_GETCURSEL, 0, 0);
   if (sharpen == LB_ERR)
      sharpen = 0;
   SaveValue(regKey[RegName::PlayerVR], "Sharpen"s, (int)sharpen);

   const bool scaleFX_DMD = SendMessage(GetDlgItem(IDC_SCALE_FX_DMD).GetHwnd(), BM_GETCHECK, 0, 0) != 0;
   SaveValue(regKey[RegName::PlayerVR], "ScaleFXDMD"s, scaleFX_DMD);

   const size_t AAfactorIndex = SendMessage(GetDlgItem(IDC_SUPER_SAMPLING_COMBO).GetHwnd(), TBM_GETPOS, 0, 0);
   const float AAfactor = (AAfactorIndex < AAfactorCount) ? AAfactors[AAfactorIndex] : 1.0f;
   SaveValue(regKey[RegName::PlayerVR], "AAFactor"s, AAfactor);

   const size_t MSAASamplesIndex = SendMessage(GetDlgItem(IDC_MSAA_COMBO).GetHwnd(), TBM_GETPOS, 0, 0);
   const int MSAASamples = (MSAASamplesIndex < MSAASampleCount) ? MSAASamplesOpts[MSAASamplesIndex] : 1;
   SaveValue(regKey[RegName::PlayerVR], "MSAASamples"s, MSAASamples);

   bool useAO = SendMessage(GetDlgItem(IDC_DYNAMIC_AO).GetHwnd(), BM_GETCHECK, 0, 0) != 0;
   SaveValue(regKey[RegName::PlayerVR], "DynamicAO"s, useAO);

   useAO = SendMessage(GetDlgItem(IDC_ENABLE_AO).GetHwnd(), BM_GETCHECK, 0, 0) ? false : true; // inverted logic
   SaveValue(regKey[RegName::PlayerVR], "DisableAO"s, useAO);

   const bool ssreflection = SendMessage(GetDlgItem(IDC_GLOBAL_SSREFLECTION_CHECK).GetHwnd(), BM_GETCHECK, 0, 0) != 0;
   SaveValue(regKey[RegName::PlayerVR], "SSRefl"s, ssreflection);

   LRESULT pfReflectionMode = SendMessage(GetDlgItem(IDC_GLOBAL_PF_REFLECTION).GetHwnd(), CB_GETCURSEL, 0, 0);
   if (pfReflectionMode == LB_ERR)
      pfReflectionMode = RenderProbe::REFL_NONE;
   if (pfReflectionMode == 2)
      pfReflectionMode = RenderProbe::REFL_DYNAMIC;
   SaveValue(regKey[RegName::PlayerVR], "PFReflection"s, (int)pfReflectionMode);

   //AMD Debugging
   const size_t textureModeVR = SendMessage(GetDlgItem(IDC_COMBO_TEXTURE).GetHwnd(), CB_GETCURSEL, 0, 0);
   SaveValue(regKey[RegName::PlayerVR], "EyeFBFormat"s, (int)textureModeVR);

   LRESULT vrPreview = SendMessage(GetDlgItem(IDC_VR_PREVIEW).GetHwnd(), CB_GETCURSEL, 0, 0);
   if (vrPreview == LB_ERR)
      vrPreview = VRPREVIEW_LEFT;
   SaveValue(regKey[RegName::PlayerVR], "VRPreview"s, (int)vrPreview);

   const bool scaleToFixedWidth = SendMessage(GetDlgItem(IDC_SCALE_TO_CM).GetHwnd(), BM_GETCHECK, 0, 0) != 0;
   SaveValue(regKey[RegName::PlayerVR], "scaleToFixedWidth"s, scaleToFixedWidth);

   SaveValue(regKey[RegName::PlayerVR], scaleToFixedWidth ? "scaleAbsolute"s : "scaleRelative"s, GetDlgItemText(IDC_VR_SCALE).c_str());
   //SaveValue(regKey[RegName::PlayerVR], scaleToFixedWidth ? "scaleRelative"s : "scaleAbsolute"s, scaleToFixedWidth ? scaleRelative : scaleAbsolute); //Also update hidden value?

   SaveValue(regKey[RegName::PlayerVR], "nearPlane"s, GetDlgItemText(IDC_NEAR_PLANE).c_str());

   SaveValue(regKey[RegName::PlayerVR], "farPlane"s, GetDlgItemText(IDC_FAR_PLANE).c_str());

   //For compatibility keep these in Player instead of PlayerVR
   SaveValue(regKey[RegName::PlayerVR], "Slope"s, GetDlgItemText(IDC_VR_SLOPE).c_str());

   SaveValue(regKey[RegName::PlayerVR], "Orientation"s, GetDlgItemText(IDC_3D_VR_ORIENTATION).c_str());

   SaveValue(regKey[RegName::PlayerVR], "TableX"s, GetDlgItemText(IDC_VR_OFFSET_X).c_str());

   SaveValue(regKey[RegName::PlayerVR], "TableY"s, GetDlgItemText(IDC_VR_OFFSET_Y).c_str());

   SaveValue(regKey[RegName::PlayerVR], "TableZ"s, GetDlgItemText(IDC_VR_OFFSET_Z).c_str());

   const bool bloomOff = SendMessage(GetDlgItem(IDC_BLOOM_OFF).GetHwnd(), BM_GETCHECK, 0, 0) != 0;
   SaveValue(regKey[RegName::PlayerVR], "ForceBloomOff"s, bloomOff);

   const size_t askToTurnOn = SendMessage(GetDlgItem(IDC_TURN_VR_ON).GetHwnd(), CB_GETCURSEL, 0, 0);
   SaveValue(regKey[RegName::PlayerVR], "AskToTurnOn"s, (int)askToTurnOn);

   const size_t dmdSource = SendMessage(GetDlgItem(IDC_DMD_SOURCE).GetHwnd(), CB_GETCURSEL, 0, 0);
   SaveValue(regKey[RegName::PlayerVR], "DMDsource"s, (int)dmdSource);

   const size_t bgSource = SendMessage(GetDlgItem(IDC_BG_SOURCE).GetHwnd(), CB_GETCURSEL, 0, 0);
   SaveValue(regKey[RegName::PlayerVR], "BGsource"s, (int)bgSource);

   bool selected = ::SendMessage(GetDlgItem(IDC_CAP_EXTDMD).GetHwnd(), BM_GETCHECK, 0, 0) != 0;
   SaveValue(regKey[RegName::Player], "CaptureExternalDMD"s, selected);

   selected = ::SendMessage(GetDlgItem(IDC_CAP_PUP).GetHwnd(), BM_GETCHECK, 0, 0) != 0;
   SaveValue(regKey[RegName::Player], "CapturePUP"s, selected);

   SetValue(IDC_JOYTABLERECENTER, regKey[RegName::Player], "JoyTableRecenterKey");
   SetValue(IDC_JOYTABLEUP, regKey[RegName::Player], "JoyTableUpKey");
   SetValue(IDC_JOYTABLEDOWN, regKey[RegName::Player], "JoyTableDownKey");

    for (unsigned int i = eTableRecenter; i <= eTableDown; ++i) if (regkey_idc[i] != -1)
    {
        const size_t key = ::GetWindowLongPtr(GetDlgItem(regkey_idc[i]).GetHwnd(), GWLP_USERDATA);
        SaveValue(regKey[RegName::Player], regkey_string[i], (int)key);
    }

   CDialog::OnOK();
}

void VROptionsDialog::OnDestroy()
{
   KeyWindowStruct* const pksw = (KeyWindowStruct*)::GetWindowLongPtr(GetHwnd(), GWLP_USERDATA);
   if (pksw->m_timerid)
   {
      ::KillTimer(GetHwnd(), pksw->m_timerid);
      pksw->m_timerid = 0;
   }
   pksw->pi.UnInit();
   CDialog::OnDestroy();
}

void VROptionsDialog::SetValue(int nID, const string& regKey, const string& regValue)
{
   LRESULT selected = ::SendMessage(GetDlgItem(nID).GetHwnd(), CB_GETCURSEL, 0, 0);
   if (selected == LB_ERR)
      selected = 2; // assume both as standard
   SaveValue(regKey, regValue, (int)selected);
}

void VROptionsDialog::StartTimer(int nID)
{
   KeyWindowStruct* const pksw = (KeyWindowStruct*)::GetWindowLongPtr(GetHwnd(), GWLP_USERDATA);
   const HWND hwndKeyWindow = GetDlgItem(nID).GetHwnd();
   if (pksw->m_timerid == NULL) //add
   { //add
      // corrects input error with space bar
      const int key = pksw->pi.GetNextKey();
      if (key == 0x39)
      {
         pksw->pi.GetNextKey(); // Clear the current buffer out
         return;
      }

      pksw->pi.GetNextKey(); // Clear the current buffer out

      pksw->m_timerid = ::SetTimer(GetHwnd(), 100, 50, nullptr);
      pksw->hwndKeyControl = hwndKeyWindow;
      ::SetWindowText(pksw->hwndKeyControl, "????");
      while (pksw->pi.GetNextKey() != NULL) //clear entire keyboard buffer contents
      {
         pksw->pi.GetNextKey();
      }
   }
}
