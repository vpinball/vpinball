// license:GPLv3+

#include "core/stdafx.h"

#include "LiveUI.h"

#include "renderer/VRDevice.h"
#include "renderer/Shader.h"
#include "renderer/Anaglyph.h"

#include "core/TableDB.h"

#include "fonts/DroidSans.h"
#include "fonts/DroidSansBold.h"
#include "fonts/IconsForkAwesome.h"
#include "fonts/ForkAwesome.h"

#include "utils/wintimer.h"

#include "plugins/VPXPlugin.h"
#include "core/VPXPluginAPIImpl.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h" // Needed for FindRenderedTextEnd in HelpSplash (should be adapted when this function will refactored in ImGui)

#if defined(ENABLE_SDL_VIDEO)
  #include "imgui/imgui_impl_sdl3.h"
#else
  #include "imgui/imgui_impl_win32.h"
#endif

#if defined(ENABLE_BGFX)
  #include "bgfx-imgui/imgui_impl_bgfx.h"
#elif defined(ENABLE_OPENGL)
  #include "imgui/imgui_impl_opengl3.h"
#elif defined(ENABLE_DX9)
  #include "imgui/imgui_impl_dx9.h"
  #include <shellapi.h>
#endif

#include "implot/implot.h"
#include "imgui/imgui_stdlib.h"
#include "imguizmo/ImGuizmo.h"
#include "imgui_markdown/imgui_markdown.h"

#ifdef __STANDALONE__
#include <unordered_map>
#endif

#if __cplusplus >= 202002L && !defined(__clang__)
#define stable_sort std::ranges::stable_sort
#define sort std::ranges::sort
#else
#define stable_sort std::stable_sort
#define sort std::sort
#endif

#ifndef __STANDALONE__
#include "BAM/BAMView.h"
#endif

// Titles (used as Ids) of modal dialogs
#define ID_MODAL_SPLASH "In Game UI"
#define ID_VIDEO_SETTINGS "Video Options"
#define ID_AUDIO_SETTINGS "Audio Options"
#define ID_RENDERER_INSPECTION "Renderer Inspection"
#define ID_BAM_SETTINGS "Headtracking Settings"
#define ID_ANAGLYPH_CALIBRATION "Anaglyph Calibration"

#define PROP_WIDTH (125.f * m_dpi)
#define PROP_TIMER(is_live, startup_obj, live_obj) \
   if (ImGui::CollapsingHeader("Timer", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE) \
   { \
      PropCheckbox("Enabled", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_tdr.m_TimerEnabled) : nullptr, live_obj ? &(live_obj->m_d.m_tdr.m_TimerEnabled) : nullptr); \
      PropInt("Interval (ms)", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_tdr.m_TimerInterval) : nullptr, live_obj ? &(live_obj->m_d.m_tdr.m_TimerInterval) : nullptr); \
      ImGui::EndTable(); \
   }
#define BEGIN_PROP_TABLE ImGui::BeginTable("props", 2, ImGuiTableFlags_Borders)
#define PROP_TABLE_SETUP \
   if (ImGui::TableGetRowIndex() == -1) \
   { \
      ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthStretch); \
      ImGui::TableSetupColumn("Sync", ImGuiTableColumnFlags_WidthFixed); \
   }
#define PROP_ACCESS(startup_obj, live_obj, prop) startup_obj ? &(startup_obj->prop) : nullptr, live_obj ? &(live_obj->prop) : nullptr

#define ICON_SAVE ICON_FK_FLOPPY_O

// Maps directinput keycode used by VPX to ImGui keycode enum
// FIXME This is not ok and will fail for all non QWERTY keyboards, since VPX (DirectInput Keyboard) is keyboard layout neutral while ImGui is layout aware
// For the time being, this is used to translate global shortcuts for:
// - Debugger (default is D which should be ok on all keyboards), 
// - FPS display (default is F11 which should be ok)
// - Exit (default is Escape which should be ok)
static constexpr ImGuiKey dikToImGuiKeys[] = {
   ImGuiKey_None,
   ImGuiKey_Escape,    //DIK_ESCAPE          0x01
   ImGuiKey_1,         //DIK_1               0x02
   ImGuiKey_2,         //DIK_2               0x03
   ImGuiKey_3,         //DIK_3               0x04
   ImGuiKey_4,         //DIK_4               0x05
   ImGuiKey_5,         //DIK_5               0x06
   ImGuiKey_6,         //DIK_6               0x07
   ImGuiKey_7,         //DIK_7               0x08
   ImGuiKey_8,         //DIK_8               0x09
   ImGuiKey_9,         //DIK_9               0x0A
   ImGuiKey_0,         //DIK_0               0x0B
   ImGuiKey_Minus,     //DIK_MINUS           0x0C    /* - on main keyboard */
   ImGuiKey_Equal,     //DIK_EQUALS          0x0D
   ImGuiKey_Backspace, //DIK_BACK            0x0E    /* backspace */
   ImGuiKey_Tab,       //DIK_TAB             0x0F
   ImGuiKey_Q,         //DIK_Q               0x10
   ImGuiKey_W,         //DIK_W               0x11
   ImGuiKey_E,         //DIK_E               0x12
   ImGuiKey_R,         //DIK_R               0x13
   ImGuiKey_T,         //DIK_T               0x14
   ImGuiKey_Y,         //DIK_Y               0x15
   ImGuiKey_U,         //DIK_U               0x16
   ImGuiKey_I,         //DIK_I               0x17
   ImGuiKey_O,         //DIK_O               0x18
   ImGuiKey_P,         //DIK_P               0x19
   ImGuiKey_LeftBracket, //DIK_LBRACKET        0x1A
   ImGuiKey_RightBracket, //DIK_RBRACKET        0x1B
   ImGuiKey_Enter,     //DIK_RETURN          0x1C    /* Enter on main keyboard */
   ImGuiKey_LeftCtrl,  //DIK_LCONTROL        0x1D
   ImGuiKey_A,         //DIK_A               0x1E
   ImGuiKey_S,         //DIK_S               0x1F
   ImGuiKey_D,         //DIK_D               0x20
   ImGuiKey_F,         //DIK_F               0x21
   ImGuiKey_G,         //DIK_G               0x22
   ImGuiKey_H,         //DIK_H               0x23
   ImGuiKey_J,         //DIK_J               0x24
   ImGuiKey_K,         //DIK_K               0x25
   ImGuiKey_L,         //DIK_L               0x26
   ImGuiKey_Semicolon, //DIK_SEMICOLON       0x27
   ImGuiKey_Apostrophe,//DIK_APOSTROPHE      0x28
   ImGuiKey_GraveAccent, //DIK_GRAVE           0x29    /* accent grave */
   ImGuiKey_LeftShift, //DIK_LSHIFT          0x2A
   ImGuiKey_Backslash, //DIK_BACKSLASH       0x2B
   ImGuiKey_Z,         //DIK_Z               0x2C
   ImGuiKey_X,         //DIK_X               0x2D
   ImGuiKey_C,         //DIK_C               0x2E
   ImGuiKey_V,         //DIK_V               0x2F
   ImGuiKey_B,         //DIK_B               0x30
   ImGuiKey_N,         //DIK_N               0x31
   ImGuiKey_M,         //DIK_M               0x32
   ImGuiKey_Comma,     //DIK_COMMA           0x33
   ImGuiKey_Period,    //DIK_PERIOD          0x34    /* . on main keyboard */
   ImGuiKey_Slash,     //DIK_SLASH           0x35    /* / on main keyboard */
   ImGuiKey_RightShift,//DIK_RSHIFT          0x36
   ImGuiKey_KeypadMultiply, //DIK_MULTIPLY        0x37    /* * on numeric keypad */
   ImGuiKey_Menu,      //DIK_LMENU           0x38    /* left Alt */
   ImGuiKey_Space,     //DIK_SPACE           0x39
   ImGuiKey_CapsLock,  //DIK_CAPITAL         0x3A
   ImGuiKey_F1,        //DIK_F1              0x3B
   ImGuiKey_F2,        //DIK_F2              0x3C
   ImGuiKey_F3,        //DIK_F3              0x3D
   ImGuiKey_F4,        //DIK_F4              0x3E
   ImGuiKey_F5,        //DIK_F5              0x3F
   ImGuiKey_F6,        //DIK_F6              0x40
   ImGuiKey_F7,        //DIK_F7              0x41
   ImGuiKey_F8,        //DIK_F8              0x42
   ImGuiKey_F9,        //DIK_F9              0x43
   ImGuiKey_F10,       //DIK_F10             0x44
   ImGuiKey_NumLock,   //DIK_NUMLOCK         0x45
   ImGuiKey_ScrollLock,//DIK_SCROLL          0x46    /* Scroll Lock */
   ImGuiKey_Keypad7,   //DIK_NUMPAD7         0x47
   ImGuiKey_Keypad8,   //DIK_NUMPAD8         0x48
   ImGuiKey_Keypad9,   //DIK_NUMPAD9         0x49
   ImGuiKey_KeypadSubtract, //DIK_SUBTRACT        0x4A    /* - on numeric keypad */
   ImGuiKey_Keypad4,   //DIK_NUMPAD4         0x4B
   ImGuiKey_Keypad5,   //DIK_NUMPAD5         0x4C
   ImGuiKey_Keypad6,   //DIK_NUMPAD6         0x4D
   ImGuiKey_KeypadAdd, //DIK_ADD             0x4E    /* + on numeric keypad */
   ImGuiKey_Keypad1,   //DIK_NUMPAD1         0x4F
   ImGuiKey_Keypad2,   //DIK_NUMPAD2         0x50
   ImGuiKey_Keypad3,   //DIK_NUMPAD3         0x51
   ImGuiKey_Keypad0,   //DIK_NUMPAD0         0x52
   ImGuiKey_KeypadDecimal, //DIK_DECIMAL         0x53    /* . on numeric keypad */
   ImGuiKey_None,      //0x54
   ImGuiKey_None,      //0x55
   ImGuiKey_None,      //DIK_OEM_102         0x56    /* < > | on UK/Germany keyboards */
   ImGuiKey_F11,       //DIK_F11             0x57
   ImGuiKey_F12,       //DIK_F12             0x58
   ImGuiKey_None,      //0x59
   ImGuiKey_None,      //0x5A
   ImGuiKey_None,      //0x5B
   ImGuiKey_None,      //0x5C
   ImGuiKey_None,      //0x5D
   ImGuiKey_None,      //0x5E
   ImGuiKey_None,      //0x5F
   ImGuiKey_None,      //0x60
   ImGuiKey_None,      //0x61
   ImGuiKey_None,      //0x62
   ImGuiKey_None,      //0x63
   ImGuiKey_None,      //DIK_F13             0x64    /*                     (NEC PC98) */
   ImGuiKey_None,      //DIK_F14             0x65    /*                     (NEC PC98) */
   ImGuiKey_None,      //DIK_F15             0x66    /*                     (NEC PC98) */
   ImGuiKey_None,      //0x67
   ImGuiKey_None,      //0x68
   ImGuiKey_None,      //0x69
   ImGuiKey_None,      //0x6A
   ImGuiKey_None,      //0x6B
   ImGuiKey_None,      //0x6C
   ImGuiKey_None,      //0x6D
   ImGuiKey_None,      //0x6E
   ImGuiKey_None,      //0x6F

   ImGuiKey_None, //0x70
   ImGuiKey_None, //0x71
   ImGuiKey_None, //0x72
   ImGuiKey_None, //0x73
   ImGuiKey_None, //0x74
   ImGuiKey_None, //0x75
   ImGuiKey_None, //0x76
   ImGuiKey_None, //0x77
   ImGuiKey_None, //0x78
   ImGuiKey_None, //0x79
   ImGuiKey_None, //0x7A
   ImGuiKey_None, //0x7B
   ImGuiKey_None, //0x7C
   ImGuiKey_None, //0x7D
   ImGuiKey_None, //0x7E
   ImGuiKey_None, //0x7F

   ImGuiKey_None, //0x80
   ImGuiKey_None, //0x81
   ImGuiKey_None, //0x82
   ImGuiKey_None, //0x83
   ImGuiKey_None, //0x84
   ImGuiKey_None, //0x85
   ImGuiKey_None, //0x86
   ImGuiKey_None, //0x87
   ImGuiKey_None, //0x88
   ImGuiKey_None, //0x89
   ImGuiKey_None, //0x8A
   ImGuiKey_None, //0x8B
   ImGuiKey_None, //0x8C
   ImGuiKey_None, //0x8D
   ImGuiKey_None, //0x8E
   ImGuiKey_None, //0x8F

   ImGuiKey_None, //0x90
   ImGuiKey_None, //0x91
   ImGuiKey_None, //0x92
   ImGuiKey_None, //0x93
   ImGuiKey_None, //0x94
   ImGuiKey_None, //0x95
   ImGuiKey_None, //0x96
   ImGuiKey_None, //0x97
   ImGuiKey_None, //0x98
   ImGuiKey_None, //0x99
   ImGuiKey_None, //0x9A
   ImGuiKey_None, //0x9B

   ImGuiKey_KeypadEnter, //DIK_NUMPADENTER     0x9C    /* Enter on numeric keypad */
   ImGuiKey_RightCtrl,   //DIK_RCONTROL        0x9D

   ImGuiKey_None, //0x9E
   ImGuiKey_None, //0x9F

   ImGuiKey_None, //0xA0
   ImGuiKey_None, //0xA1
   ImGuiKey_None, //0xA2
   ImGuiKey_None, //0xA3
   ImGuiKey_None, //0xA4
   ImGuiKey_None, //0xA5
   ImGuiKey_None, //0xA6
   ImGuiKey_None, //0xA7
   ImGuiKey_None, //0xA8
   ImGuiKey_None, //0xA9
   ImGuiKey_None, //0xAA
   ImGuiKey_None, //0xAB
   ImGuiKey_None, //0xAC
   ImGuiKey_None, //0xAD
   ImGuiKey_None, //0xAE
   ImGuiKey_None, //0xAF

   ImGuiKey_None, //0xB0
   ImGuiKey_None, //0xB1
   ImGuiKey_None, //0xB2
   ImGuiKey_None, //0xB3
   ImGuiKey_None, //0xB4

   ImGuiKey_KeypadDivide, //DIK_DIVIDE          0xB5    /* / on numeric keypad */
   ImGuiKey_None,         //0xB6
   ImGuiKey_None,         //DIK_SYSRQ           0xB7
   ImGuiKey_RightAlt,     //DIK_RMENU           0xB8    /* right Alt */

   ImGuiKey_None, //0xB9
   ImGuiKey_None, //0xBA
   ImGuiKey_None, //0xBB
   ImGuiKey_None, //0xBC
   ImGuiKey_None, //0xBD
   ImGuiKey_None, //0xBE
   ImGuiKey_None, //0xBF

   ImGuiKey_None, //0xC0
   ImGuiKey_None, //0xC1
   ImGuiKey_None, //0xC2
   ImGuiKey_None, //0xC3
   ImGuiKey_None, //0xC4
   ImGuiKey_None, //0xC5
   ImGuiKey_None, //0xC6

   ImGuiKey_Home,       //DIK_HOME            0xC7    /* Home on arrow keypad */
   ImGuiKey_UpArrow,    //DIK_UP              0xC8    /* UpArrow on arrow keypad */
   ImGuiKey_PageUp,     //DIK_PRIOR           0xC9    /* PgUp on arrow keypad */
   ImGuiKey_None,       //0xCA
   ImGuiKey_LeftArrow,  //DIK_LEFT            0xCB    /* LeftArrow on arrow keypad */
   ImGuiKey_None,       //0xCC
   ImGuiKey_RightArrow, //DIK_RIGHT           0xCD    /* RightArrow on arrow keypad */
   ImGuiKey_None,       //0xCE
   ImGuiKey_End,        //DIK_END             0xCF    /* End on arrow keypad */
   ImGuiKey_DownArrow,  //DIK_DOWN            0xD0    /* DownArrow on arrow keypad */
   ImGuiKey_PageDown,   //DIK_NEXT            0xD1    /* PgDn on arrow keypad */
   ImGuiKey_Insert,     //DIK_INSERT          0xD2    /* Insert on arrow keypad */
   ImGuiKey_Delete,     //DIK_DELETE          0xD3    /* Delete on arrow keypad */

   ImGuiKey_None, //0xD4
   ImGuiKey_None, //0xD5
   ImGuiKey_None, //0xD6
   ImGuiKey_None, //0xD7
   ImGuiKey_None, //0xD8
   ImGuiKey_None, //0xD9
   ImGuiKey_None, //0xDA

   ImGuiKey_LeftSuper,  //DIK_LWIN            0xDB    /* Left Windows key */
   ImGuiKey_RightSuper, //DIK_RWIN            0xDC    /* Right Windows key */
   ImGuiKey_None,       //DIK_APPS            0xDD    /* AppMenu key */
};

static void SetupImGuiStyle(const float overall_alpha)
{
   // Rounded Visual Studio style by RedNicStone from ImThemes
   ImGuiStyle &style = ImGui::GetStyle();

   style.Alpha = 1.0f;
   style.DisabledAlpha = 0.6000000238418579f;
   style.WindowPadding = ImVec2(8.0f, 8.0f);
   style.WindowRounding = 4.0f;
   style.WindowBorderSize = 0.0f;
   style.WindowMinSize = ImVec2(32.0f, 32.0f);
   style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
   style.WindowMenuButtonPosition = ImGuiDir_Left;
   style.ChildRounding = 0.0f;
   style.ChildBorderSize = 1.0f;
   style.PopupRounding = 4.0f;
   style.PopupBorderSize = 1.0f;
   style.FramePadding = ImVec2(4.0f, 3.0f);
   style.FrameRounding = 2.5f;
   style.FrameBorderSize = 0.0f;
   style.ItemSpacing = ImVec2(8.0f, 4.0f);
   style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
   style.CellPadding = ImVec2(4.0f, 2.0f);
   style.IndentSpacing = 21.0f;
   style.ColumnsMinSpacing = 6.0f;
   style.ScrollbarSize = 11.0f;
   style.ScrollbarRounding = 2.5f;
   style.GrabMinSize = 10.0f;
   style.GrabRounding = 2.0f;
   style.TabRounding = 3.5f;
   style.TabBorderSize = 0.0f;
   style.TabMinWidthForCloseButton = 0.0f;
   style.ColorButtonPosition = ImGuiDir_Right;
   style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
   style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

   style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
   style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.5921568870544434f, 0.5921568870544434f, 0.5921568870544434f, overall_alpha);
   style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, overall_alpha);
   style.Colors[ImGuiCol_ChildBg] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, overall_alpha);
   style.Colors[ImGuiCol_PopupBg] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, overall_alpha);
   style.Colors[ImGuiCol_Border] = ImVec4(0.3058823645114899f, 0.3058823645114899f, 0.3058823645114899f, overall_alpha);
   style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.3058823645114899f, 0.3058823645114899f, 0.3058823645114899f, overall_alpha);
   style.Colors[ImGuiCol_FrameBg] = ImVec4(0.2000000029802322f, 0.2000000029802322f, 0.2156862765550613f, overall_alpha);
   style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, overall_alpha);
   style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, overall_alpha);
   style.Colors[ImGuiCol_TitleBg] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, overall_alpha);
   style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, overall_alpha);
   style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, overall_alpha);
   style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.2000000029802322f, 0.2000000029802322f, 0.2156862765550613f, overall_alpha);
   style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.2000000029802322f, 0.2000000029802322f, 0.2156862765550613f, overall_alpha);
   style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.321568638086319f, 0.321568638086319f, 0.3333333432674408f, overall_alpha);
   style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.3529411852359772f, 0.3529411852359772f, 0.3725490272045135f, overall_alpha);
   style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.3529411852359772f, 0.3529411852359772f, 0.3725490272045135f, overall_alpha);
   style.Colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, overall_alpha);
   style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, overall_alpha);
   style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, overall_alpha);
   style.Colors[ImGuiCol_Button] = ImVec4(0.2000000029802322f, 0.2000000029802322f, 0.2156862765550613f, overall_alpha);
   style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, overall_alpha);
   style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, overall_alpha);
   style.Colors[ImGuiCol_Header] = ImVec4(0.2000000029802322f, 0.2000000029802322f, 0.2156862765550613f, overall_alpha);
   style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, overall_alpha);
   style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, overall_alpha);
   style.Colors[ImGuiCol_Separator] = ImVec4(0.3058823645114899f, 0.3058823645114899f, 0.3058823645114899f, overall_alpha);
   style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.3058823645114899f, 0.3058823645114899f, 0.3058823645114899f, overall_alpha);
   style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.3058823645114899f, 0.3058823645114899f, 0.3058823645114899f, overall_alpha);
   style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, overall_alpha);
   style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.2000000029802322f, 0.2000000029802322f, 0.2156862765550613f, overall_alpha);
   style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.321568638086319f, 0.321568638086319f, 0.3333333432674408f, overall_alpha);
   style.Colors[ImGuiCol_Tab] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, overall_alpha);
   style.Colors[ImGuiCol_TabHovered] = ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, overall_alpha);
   style.Colors[ImGuiCol_TabActive] = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, overall_alpha);
   style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, overall_alpha);
   style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, overall_alpha);
   style.Colors[ImGuiCol_PlotLines] = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, overall_alpha);
   style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, overall_alpha);
   style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, overall_alpha);
   style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, overall_alpha);
   style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.1882352977991104f, 0.1882352977991104f, 0.2000000029802322f, overall_alpha);
   style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3490196168422699f, overall_alpha);
   style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.2274509817361832f, 0.2274509817361832f, 0.2470588237047195f, overall_alpha);
   style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f * overall_alpha);
   style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.05999999865889549f * overall_alpha);
   style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, overall_alpha);
   style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, overall_alpha);
   style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, overall_alpha);
   style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.699999988079071f * overall_alpha);
   style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.2000000029802322f * overall_alpha);
   style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, overall_alpha);
}

// Helper to display a little (?) mark which shows a tooltip when hovered.
static void HelpMarker(const char *desc)
{
   ImGui::TextDisabled("(?)");
   if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
   {
      ImGui::BeginTooltip();
      ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
      ImGui::TextUnformatted(desc);
      ImGui::PopTextWrapPos();
      ImGui::EndTooltip();
   }
}

template <class T> static std::vector<T> SortedCaseInsensitive(std::vector<T>& list, const std::function<string(T)>& map)
{
   std::vector<T> sorted;
   sorted.reserve(list.size());
   sorted.insert(sorted.begin(), list.begin(), list.end());
   sort(sorted.begin(), sorted.end(), [map](const T &a, const T &b) -> bool 
      {
         string str1 = map(a), str2 = map(b);
         for (string::const_iterator c1 = str1.begin(), c2 = str2.begin(); c1 != str1.end() && c2 != str2.end(); ++c1, ++c2)
         {
            const auto cl1 = cLower(*c1);
            const auto cl2 = cLower(*c2);
            if (cl1 > cl2)
               return false;
            if (cl1 < cl2)
               return true;
         }
         return str1.size() > str2.size();
      });
   return sorted;
}


static void HelpTextCentered(const std::string& text)
{
   const ImVec2 win_size = ImGui::GetWindowSize();
   const ImVec2 text_size = ImGui::CalcTextSize(text.c_str());

   // calculate the indentation that centers the text on one line, relative
   // to window left, regardless of the `ImGuiStyleVar_WindowPadding` value
   float text_indentation = (win_size.x - text_size.x) * 0.5f;

   // if text is too long to be drawn on one line, `text_indentation` can
   // become too small or even negative, so we check a minimum indentation
   constexpr float min_indentation = 20.0f;
   if (text_indentation <= min_indentation)
      text_indentation = min_indentation;

   ImGui::SameLine(text_indentation);
   ImGui::PushTextWrapPos(win_size.x - text_indentation);
   ImGui::TextWrapped("%s", text.c_str());
   ImGui::PopTextWrapPos();
}

static void HelpSplash(const string &text, int rotation)
{
   const ImVec2 win_size = ImGui::GetIO().DisplaySize;

   vector<string> lines;
   ImVec2 text_size(0, 0);

   constexpr float padding = 60.f;
   const float maxWidth = win_size.x - padding;
   ImFont *const font = ImGui::GetFont();

   string line;
   std::istringstream iss(text);
   while (std::getline(iss, line)) {
       const char *textEnd = line.c_str();
       if (*textEnd == '\0') {
          lines.push_back(line);
          continue;
       }
       while (*textEnd) {
          const char *nextLineTextEnd = ImGui::FindRenderedTextEnd(textEnd, nullptr);
          ImVec2 lineSize = font->CalcTextSizeA(font->FontSize, FLT_MAX, 0.0f, textEnd, nextLineTextEnd);
          if (lineSize.x > maxWidth)
          {
             const char *wrapPoint = font->CalcWordWrapPositionA(font->Scale, textEnd, nextLineTextEnd, maxWidth);
             if (wrapPoint == textEnd)
                wrapPoint++;
             nextLineTextEnd = wrapPoint;
             lineSize = font->CalcTextSizeA(font->FontSize, FLT_MAX, 0.0f, textEnd, wrapPoint);
          }

          string newLine(textEnd, nextLineTextEnd);
          lines.push_back(newLine);

          if (lineSize.x > text_size.x)
             text_size.x = lineSize.x;

          textEnd = nextLineTextEnd;

          while (*textEnd == ' ')
             textEnd++;
       }
   }

   text_size.x += (padding / 2.f);
   text_size.y = ((float)lines.size() * ImGui::GetTextLineHeightWithSpacing()) + (padding / 2.f);

   constexpr ImGuiWindowFlags window_flags
      = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
   ImGui::SetNextWindowBgAlpha(0.35f);
   ImGui::SetNextWindowPos(ImVec2((win_size.x - text_size.x) / 2, (win_size.y - text_size.y) / 2));
   ImGui::SetNextWindowSize(ImVec2(text_size.x, text_size.y));
   ImGui::Begin("ToolTip", nullptr, window_flags);
   ImGui::SetCursorPosY(padding / 4.f);
   for (const string& curline : lines)
   {
      const ImVec2 lineSize = font->CalcTextSizeA(font->FontSize, FLT_MAX, 0.0f, curline.c_str());
      ImGui::SetCursorPosX(((text_size.x - lineSize.x) / 2.f));
      ImGui::Text("%s", curline.c_str());
   }
   ImGui::End();
}

void LiveUI::UpdateTouchUI()
{
   if (!m_player->m_supportsTouch)
      return;

#ifdef __LIBVPINBALL__
   if (m_player->m_liveUIOverride)
      return;
#endif

   ImGuiIO &io = ImGui::GetIO();

   float screenWidth = io.DisplaySize.x;
   float screenHeight = io.DisplaySize.y;

   constexpr ImGuiWindowFlags window_flags
      = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

   ImGui::SetNextWindowBgAlpha(0.0f);
   ImGui::SetNextWindowPos(ImVec2(0,0));
   ImGui::SetNextWindowSize(ImVec2(screenWidth, screenHeight));
   ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
   ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

   ImGui::Begin("Touch Controls", nullptr, window_flags);

   const bool showTouchOverlay = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "TouchOverlay"s, false);
   if (showTouchOverlay) {
      ImDrawList* drawList = ImGui::GetWindowDrawList();

      for (int i = 0; i < MAX_TOUCHREGION; ++i) {
         RECT rect = touchregion[i];

         ImVec2 topLeft((float)rect.left * screenWidth / 100.0f, (float)rect.top * screenHeight / 100.0f);
         ImVec2 bottomRight((float)rect.right * screenWidth / 100.0f, (float)rect.bottom * screenHeight / 100.0f);

         ImColor fillColor(255, 255, 255, 5);
         drawList->AddRectFilled(topLeft, bottomRight, fillColor);

         ImColor borderColor(255, 255, 255, 20);
         drawList->AddRect(topLeft, bottomRight, borderColor, 0.0f, ImDrawFlags_RoundCornersAll, 2.0f);
      }
   }

   ImGui::End();
   ImGui::PopStyleVar(2);
}

static void HelpEditableHeader(bool is_live, IEditable *editable, IEditable *live_editable)
{
   IEditable *notnull_editable = editable ? editable : live_editable;
   IEditable *select_editable = is_live ? live_editable : editable;
   if (notnull_editable == nullptr)
      return;
   string title;
   switch (notnull_editable->GetItemType())
   {
   // Missing: eItemLightCenter, eItemDragPoint, eItemCollection
   case eItemBumper: title = "Bumper"s; break;
   case eItemDecal: title = "Decal"s; break;
   case eItemDispReel: title = "Reel"s; break;
   case eItemGate: title = "Gate"s; break;
   case eItemFlasher: title = "Flasher"s; break;
   case eItemFlipper: title = "Flipper"s; break;
   case eItemHitTarget: title = "Target"s; break;
   case eItemKicker: title = "Kicker"s; break;
   case eItemLight: title = "Light"s; break;
   case eItemLightSeq: title = "Light Sequencer"s; break;
   case eItemPlunger: title = "Plunger"s; break;
   case eItemPrimitive: title = ((Primitive *)notnull_editable)->IsPlayfield() ? "Playfield"s : "Primitive"s; break;
   case eItemRamp: title = "Ramp"s; break;
   case eItemRubber: title = "Rubber"s; break;
   case eItemSpinner: title = "Spinner"s; break;
   case eItemSurface: title = "Surface"s; break;
   case eItemTable: title = "Table"s; break;
   case eItemTextbox: title = "TextBox"s; break;
   case eItemTimer: title = "Timer"s; break;
   case eItemTrigger: title = "Trigger"s; break;
   default: break;
   }
   HelpTextCentered(title);
   ImGui::BeginDisabled(is_live); // Do not edit name of live objects, it would likely break the script
   string name = select_editable ? select_editable->GetName() : string();
   if (ImGui::InputText("Name", &name))
   {
      editable->SetName(name);
   }
   ImGui::EndDisabled();
   ImGui::Separator();
}



ImGui::MarkdownConfig LiveUI::markdown_config;

LiveUI::LiveUI(RenderDevice *const rd)
   : m_rd(rd)
{
   m_StartTime_msec = msec();
   m_app = g_pvp;
   m_player = g_pplayer;
   m_table = m_player->m_pEditorTable;
   m_live_table = m_player->m_ptable;
   m_pininput = &(m_player->m_pininput);
   m_renderer = m_player->m_renderer;
   m_disable_esc = m_live_table->m_settings.LoadValueWithDefault(Settings::Player, "DisableESC"s, m_disable_esc);
   memset(m_tweakState, 0, sizeof(m_tweakState));

   m_selection.type = Selection::SelectionType::S_NONE;
   m_useEditorCam = false;

   IMGUI_CHECKVERSION();
   ImGui::CreateContext();
   ImPlot::CreateContext();
   ImGuiIO &io = ImGui::GetIO();
   io.IniFilename = nullptr; //don't use an ini file for configuration

   // Editor camera position. We use a right handed system for easy ImGuizmo integration while VPX renderer is left handed, so reverse X axis
   m_orthoCam = true;
   m_camDistance = m_live_table->m_bottom * 0.7f;
   const vec3 eye(m_live_table->m_right * 0.5f, m_live_table->m_bottom * 0.5f, -m_camDistance);
   const vec3 at(m_live_table->m_right * 0.5f, m_live_table->m_bottom * 0.5f, 0.f);
   const vec3 up(0.f, -1.f, 0.f);
   m_camView = Matrix3D::MatrixLookAtRH(eye, at, up);
   ImGuizmo::AllowAxisFlip(false);

#if defined(ENABLE_SDL_VIDEO) // SDL Windowing
   // using the specialized initializer is not needed
   // ImGui_ImplSDL3_InitForOpenGL(m_player->m_playfieldSdlWnd, rd->m_sdl_context);
   ImGui_ImplSDL3_InitForOther(m_player->m_playfieldWnd->GetCore());
   //int displayIndex = SDL_GetDisplayForWindow(m_player->m_playfieldWnd->GetCore());
   if (m_player->m_vrDevice)
   {
      // VR headset cover full view range, so use a relative part of the full range for the DPI
      m_dpi = min(m_player->m_vrDevice->GetEyeWidth(), m_player->m_vrDevice->GetEyeHeight()) / 2000.f;
      #ifdef ENABLE_BGFX
      ImGui_Implbgfx_SetStereoOfs(m_player->m_vrDevice->GetEyeWidth() * 0.15f);
      #endif
   }
   else
   {
      // Use display DPI setting
      m_dpi = SDL_GetWindowDisplayScale(m_player->m_playfieldWnd->GetCore());
   }
#else // Win32 Windowing
   ImGui_ImplWin32_Init(m_player->m_playfieldWnd->GetCore());
   m_dpi = ImGui_ImplWin32_GetDpiScaleForHwnd(m_player->m_playfieldWnd->GetCore());
#endif
   m_dpi = min(m_dpi, 10.f); // To avoid texture size overflows

   SetupImGuiStyle(1.0f);

   ImGui::GetStyle().ScaleAllSizes(m_dpi);

   // UI fonts
   m_baseFont = io.Fonts->AddFontFromMemoryCompressedTTF(droidsans_compressed_data, droidsans_compressed_size, 13.0f * m_dpi);
   ImFontConfig icons_config;
   icons_config.MergeMode = true;
   icons_config.PixelSnapH = true;
   icons_config.GlyphMinAdvanceX = 13.0f * m_dpi;
   static constexpr ImWchar icons_ranges[] = { ICON_MIN_FK, ICON_MAX_16_FK, 0 };
   io.Fonts->AddFontFromMemoryCompressedTTF(fork_awesome_compressed_data, fork_awesome_compressed_size, 13.0f * m_dpi, &icons_config, icons_ranges);

   const float overlaySize = 13.0f * m_dpi;
   m_overlayFont = io.Fonts->AddFontFromMemoryCompressedTTF(droidsans_compressed_data, droidsans_compressed_size, overlaySize);
   m_overlayBoldFont = io.Fonts->AddFontFromMemoryCompressedTTF(droidsansbold_compressed_data, droidsansbold_compressed_size, overlaySize);
   ImFont *H1 = io.Fonts->AddFontFromMemoryCompressedTTF(droidsansbold_compressed_data, droidsansbold_compressed_size, overlaySize * 20.0f / 13.f);
   ImFont *H2 = io.Fonts->AddFontFromMemoryCompressedTTF(droidsansbold_compressed_data, droidsansbold_compressed_size, overlaySize * 18.0f / 13.f);
   ImFont *H3 = io.Fonts->AddFontFromMemoryCompressedTTF(droidsansbold_compressed_data, droidsansbold_compressed_size, overlaySize * 15.0f / 13.f);
   markdown_config.linkCallback = MarkdownLinkCallback;
   markdown_config.tooltipCallback = nullptr;
   markdown_config.imageCallback = MarkdownImageCallback;
   //markdown_config.linkIcon = ICON_FA_LINK;
   markdown_config.headingFormats[0] = { H1, true };
   markdown_config.headingFormats[1] = { H2, true };
   markdown_config.headingFormats[2] = { H3, false };
   markdown_config.userData = this;
   markdown_config.formatCallback = MarkdownFormatCallback;

#if defined(ENABLE_BGFX)
   ImGui_Implbgfx_Init();
   bgfx::setViewName(bgfx::getCaps()->limits.maxViews - 1, "ImGui");
   if (m_player && m_player->m_playfieldWnd->IsWCGBackBuffer())
   {
      // A value of 1.0 should be sdrWhite * 80, while in the WCG colorspace 80 nits is 0.5
      const float v = 2.0f / m_player->m_playfieldWnd->GetSDRWhitePoint();
      float sdrColor[4] = { v, v, v, 1.f };
      ImGui_Implbgfx_SetSDRColor(sdrColor);
   }
#elif defined(ENABLE_OPENGL)
   ImGui_ImplOpenGL3_Init();
#elif defined(ENABLE_DX9)
   ImGui_ImplDX9_Init(rd->GetCoreDevice());
#endif
}

LiveUI::~LiveUI()
{
   HideUI();
   if (ImGui::GetCurrentContext())
   {
      #if defined(ENABLE_BGFX)
      ImGui_Implbgfx_Shutdown();
      #elif defined(ENABLE_OPENGL)
      ImGui_ImplOpenGL3_Shutdown();
      #elif defined(ENABLE_DX9)
      ImGui_ImplDX9_Shutdown();
      #endif

      #if defined(ENABLE_SDL_VIDEO)
      ImGui_ImplSDL3_Shutdown();
      #else
      ImGui_ImplWin32_Shutdown();
      #endif

      ImPlot::DestroyContext();
      ImGui::DestroyContext();
   }
}

void LiveUI::MarkdownFormatCallback(const ImGui::MarkdownFormatInfo &markdownFormatInfo, bool start)
{
   LiveUI *ui = (LiveUI *)(markdownFormatInfo.config->userData);
   switch (markdownFormatInfo.type)
   {
   case ImGui::MarkdownFormatType::EMPHASIS:
      ImGui::defaultMarkdownFormatCallback(markdownFormatInfo, start);
      if (markdownFormatInfo.level == 1)
      { // normal emphasis
         if (start)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
         else
            ImGui::PopStyleColor();
      }
      else
      { // strong emphasis
         if (start)
            ImGui::PushFont(ui->m_overlayBoldFont);
         else
            ImGui::PopFont();
      }
      break;
   case ImGui::MarkdownFormatType::HEADING:
   {
      ImGui::MarkdownHeadingFormat fmt;
      if (markdownFormatInfo.level > ImGui::MarkdownConfig::NUMHEADINGS)
         fmt = markdownFormatInfo.config->headingFormats[ImGui::MarkdownConfig::NUMHEADINGS - 1];
      else
         fmt = markdownFormatInfo.config->headingFormats[markdownFormatInfo.level - 1];
      if (start)
      {
         if (fmt.font)
            ImGui::PushFont(fmt.font);
         if (ImGui::GetItemID() != ui->markdown_start_id)
            ImGui::NewLine();
      }
      else
      {
         if (fmt.separator)
         {
            ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(1.f, 1.f, 1.f, 1.f));
            ImGui::Separator();
            ImGui::PopStyleColor();
            ImGui::NewLine();
         }
         else
         {
            ImGui::NewLine();
         }
         if (fmt.font)
            ImGui::PopFont();
      }
      break;
   }
   default: ImGui::defaultMarkdownFormatCallback(markdownFormatInfo, start); break;
   }
}

void LiveUI::MarkdownLinkCallback(ImGui::MarkdownLinkCallbackData data)
{
   if (!data.isImage)
   {
      std::string url(data.link, data.linkLength);
      #ifdef ENABLE_SDL_VIDEO
      SDL_OpenURL(url.c_str());
      #else
      ShellExecuteA(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
      #endif
   }
}

ImGui::MarkdownImageData LiveUI::MarkdownImageCallback(ImGui::MarkdownLinkCallbackData data)
{
   LiveUI *ui = (LiveUI *)data.userData;
   Texture *const ppi = ui->m_live_table->GetImage(std::string(data.link, data.linkLength));
   if (ppi == nullptr)
      return ImGui::MarkdownImageData {};
   Sampler *sampler = ui->m_renderer->m_renderDevice->m_texMan.LoadTexture(ppi->m_pdsBuffer, SamplerFilter::SF_BILINEAR, SamplerAddressMode::SA_CLAMP, SamplerAddressMode::SA_CLAMP, false);
   if (sampler == nullptr)
      return ImGui::MarkdownImageData {};
   #if defined(ENABLE_BGFX)
   ImTextureID image = (ImTextureID)sampler;
   #elif defined(ENABLE_OPENGL)
   ImTextureID image = (ImTextureID)sampler->GetCoreTexture();
   #elif defined(ENABLE_DX9)
   ImTextureID image = (ImTextureID)sampler->GetCoreTexture();
   #endif
   ImGui::MarkdownImageData imageData { true, false, image, ImVec2((float)sampler->GetWidth(), (float)sampler->GetHeight()) };
   ImVec2 const contentSize = ImGui::GetContentRegionAvail();
   if (imageData.size.x > contentSize.x)
   {
      float const ratio = imageData.size.y / imageData.size.x;
      imageData.size.x = contentSize.x;
      imageData.size.y = contentSize.x * ratio;
   }
   return imageData;
}

bool LiveUI::HasKeyboardCapture() const
{
   return ImGui::GetIO().WantCaptureKeyboard;
}

bool LiveUI::HasMouseCapture() const
{
   return ImGui::GetIO().WantCaptureMouse;
}

void LiveUI::Render()
{
   // For the time being, the UI is only available inside a running player
   if (m_player == nullptr || m_player->GetCloseState() != Player::CS_PLAYING)
      return;

   // Rendering must happen on a render target matching the dimension we used to prepare the UI frame
   ImGuiIO &io = ImGui::GetIO();
   assert( ((m_rotate == 0 || m_rotate == 2) && RenderTarget::GetCurrentRenderTarget()->GetWidth() == (int)io.DisplaySize.x && RenderTarget::GetCurrentRenderTarget()->GetHeight() == (int)io.DisplaySize.y)
        || ((m_rotate == 1 || m_rotate == 3) && RenderTarget::GetCurrentRenderTarget()->GetWidth() == (int)io.DisplaySize.y && RenderTarget::GetCurrentRenderTarget()->GetHeight() == (int)io.DisplaySize.x));

   if (m_rotate != 0 && !m_rotation_callback_added)
   {
      // We hack into ImGui renderer for the simple tooltips that must be displayed facing the user
      m_rotation_callback_added = true; // Only add it once per frame
      ImGui::GetBackgroundDrawList()->AddCallback(
         [](const ImDrawList *parent_list, const ImDrawCmd *cmd)
         {
            ImGuiIO &io = ImGui::GetIO();
            LiveUI *const lui = (LiveUI *)cmd->UserCallbackData;
            const Matrix3D matRotate = Matrix3D::MatrixRotateZ((float)(lui->m_rotate * (M_PI / 2.0)));
            Matrix3D matTranslate;
            switch (lui->m_rotate)
            {
            case 1: matTranslate = Matrix3D::MatrixTranslate(io.DisplaySize.y, 0, 0); break;
            case 2: matTranslate = Matrix3D::MatrixTranslate(io.DisplaySize.x, io.DisplaySize.y, 0); break;
            case 3: matTranslate = Matrix3D::MatrixTranslate(0, io.DisplaySize.x, 0); break;
            }
            matTranslate = matRotate * matTranslate;
            const float L = 0, R = (lui->m_rotate == 1 || lui->m_rotate == 3) ? io.DisplaySize.y : io.DisplaySize.x;
            const float T = 0, B = (lui->m_rotate == 1 || lui->m_rotate == 3) ? io.DisplaySize.x : io.DisplaySize.y;

            #if defined(ENABLE_BGFX)
            Matrix3D matProj;
            bx::mtxOrtho(&matProj.m[0][0], L, R, B, T, 0.0f, 1000.0f, 0.0f, bgfx::getCaps()->homogeneousDepth);
            matProj = matTranslate * matProj;
            bgfx::setViewTransform(lui->m_rd->m_activeViewId, NULL, &matProj.m[0][0]);

            #elif defined(ENABLE_OPENGL)
            Matrix3D matProj(
               2.0f / (R - L), 0.0f, 0.0f, 0.0f, 
               0.0f, 2.0f / (T - B), 0.0f, 0.0f, 
               0.0f, 0.0f, -1.0f, 0.0f, 
               (R + L) / (L - R), (T + B) / (B - T), 0.0f, 1.0f);
            matProj = matTranslate * matProj;
            GLint shaderHandle;
            glGetIntegerv(GL_CURRENT_PROGRAM, &shaderHandle);
            GLuint attribLocationProjMtx = glGetUniformLocation(shaderHandle, "ProjMtx");
            glUniformMatrix4fv(attribLocationProjMtx, 1, GL_FALSE, &(matProj.m[0][0]));
            glDisable(GL_SCISSOR_TEST);
            
            #elif defined(ENABLE_DX9)
            lui->m_rd->GetCoreDevice()->SetTransform(D3DTS_WORLD, (const D3DXMATRIX *)&matTranslate);
            lui->m_rd->GetCoreDevice()->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
            #endif

         }, this);
   }
   ImGui::Render();
   ImDrawData * const draw_data = ImGui::GetDrawData();
   if (m_rotate == 1 || m_rotate == 3)
   {
      const float tmp = draw_data->DisplaySize.x;
      draw_data->DisplaySize.x = draw_data->DisplaySize.y;
      draw_data->DisplaySize.y = tmp;
   }

   #if defined(ENABLE_BGFX)
   ImGui_Implbgfx_SetRotation(m_rotate);
   ImGui_Implbgfx_RenderDrawLists(m_rd->m_activeViewId, RenderTarget::GetCurrentRenderTarget()->m_nLayers, draw_data);

   #elif defined(ENABLE_OPENGL)
   #ifndef __OPENGLES__
   if (GLAD_GL_VERSION_4_3)
      glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "ImGui");
   #endif
   ImGui_ImplOpenGL3_RenderDrawData(draw_data);
   #ifndef __OPENGLES__
   if (GLAD_GL_VERSION_4_3)
      glPopDebugGroup();
   #endif

   #elif defined(ENABLE_DX9)
   ImGui_ImplDX9_RenderDrawData(draw_data);
   #endif
}

void LiveUI::OpenMainSplash()
{
   if (!m_ShowUI && !m_ShowSplashModal)
   {
      m_esc_mode = 0; // Get back to game if Esc is pressed
      m_ShowUI = true;
      m_ShowSplashModal = true;
      m_OpenUITime = msec();
      m_player->SetPlayState(false);
   }
}

void LiveUI::OpenLiveUI()
{
   if (!m_ShowUI && !m_ShowSplashModal)
   {
      m_OpenUITime = msec();
      m_player->SetPlayState(false);
      m_ShowUI = true;
      m_ShowSplashModal = false;
      m_useEditorCam = true;
      m_orthoCam = false;
      if (!m_staticPrepassDisabled)
      {
         m_staticPrepassDisabled = true;
         m_renderer->DisableStaticPrePass(true);
      }
      ResetCameraFromPlayer();
   }
}

void LiveUI::ToggleFPS()
{
   m_show_fps = (m_show_fps + 1) % 3;
   if (m_show_fps == 1)
   {
      m_player->InitFPS();
      m_player->m_logicProfiler.EnableWorstFrameLogging(true);
   }
   if (m_show_fps == 0)
      m_rd->LogNextFrame();
}

void LiveUI::UpdatePerfOverlay()
{
   if (m_show_fps <= 0)
      return;

   ImGuiIO &io = ImGui::GetIO();
   constexpr ImGuiWindowFlags window_flags
      = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
   ImGui::SetNextWindowBgAlpha(0.75f);
   if (m_player->m_vrDevice)
   {
      if (m_show_fps == 2)
         ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.3f, io.DisplaySize.y * 0.35f), 0, ImVec2(0.f, 0.f));
      else
         ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.35f), 0, ImVec2(0.5f, 0.f));
   }
   else
      ImGui::SetNextWindowPos(ImVec2(8.f * m_dpi, io.DisplaySize.y - 8.f * m_dpi), 0, ImVec2(0.f, 1.f));

   ImGui::Begin("FPS", nullptr, window_flags);

   // Frame sequence graph
   if (m_show_fps == 2)
   {
      ImGuiWindow *window = ImGui::GetCurrentWindow();
      static const string infoLabels[] = {
         "Misc"s,
         "Script"s,
         "Physics"s,
         "Sleep"s,
         "Prepare Frame"s,
         "Custom 1"s,
         "Custom 2"s,
         "Custom 3"s,
         // Render thread
         "Render Wait"s,
         "Render Submit"s,
         "Render Flip"s,
         "Render Sleep"s,
      };
      static const string infoLabels2[] = {
         "Misc"s,
         "Script"s,
         "Physics"s,
         "Sleep"s,
         "(Prepare frame after next, including script calls)"s,
         "Custom 1"s,
         "Custom 2"s,
         "Custom 3"s,
         // Render thread
         "(Wait for logic thread to prepare a frame)"s,
         "(Submit to driver next frame)"s,
         "(Wait for GPU and display sync to finish current frame)"s,
         "(Sleep to synchronise on user selected FPS)"s,
      };
      FrameProfiler::ProfileSection sections[] = {
         FrameProfiler::PROFILE_PREPARE_FRAME,
         FrameProfiler::PROFILE_RENDER_WAIT,
         FrameProfiler::PROFILE_RENDER_SUBMIT,
         FrameProfiler::PROFILE_RENDER_FLIP,
         FrameProfiler::PROFILE_RENDER_SLEEP,
         FrameProfiler::PROFILE_RENDER_SUBMIT, // For BGFX, since BGFX performs CPU->GPU submit after flip
      };
      constexpr ImU32 cols[] = {
         IM_COL32(128, 255, 128, 255), // Prepare
         IM_COL32(255,   0,   0, 255), // Wait for Render Frame
         IM_COL32(  0, 128, 255, 255), // Submit (VPX->BGFX)
         IM_COL32(  0, 192, 192, 255), // Flip (wait for GPU and sync on display)
         IM_COL32(128, 128, 128, 255), // Sleep
         IM_COL32(  0, 128, 255, 255), // Submit (BGFX->Driver)
      };
      //const ImU32 col_base = ImGui::GetColorU32(ImGuiCol_PlotHistogram) & 0x77FFFFFF;
      //const ImU32 col_hovered = ImGui::GetColorU32(ImGuiCol_PlotHistogramHovered) & 0x77FFFFFF;
      //const ImU32 col_outline_base = ImGui::GetColorU32(ImGuiCol_PlotHistogram) & 0x7FFFFFFF;
      //const ImU32 col_outline_hovered = ImGui::GetColorU32(ImGuiCol_PlotHistogramHovered) & 0x7FFFFFFF;
      ImGuiContext &g = *GImGui;
      const ImGuiStyle &style = g.Style;
      ImVec2 graph_size;
      const auto blockHeight = ImGui::GetTextLineHeight() + (style.FramePadding.y * 2);
      if (graph_size.x == 0.0f)
         graph_size.x = ImGui::CalcItemWidth();
      if (graph_size.y == 0.0f)
      #ifdef ENABLE_BGFX
         graph_size.y = (style.FramePadding.y * 1) + blockHeight * 2;
      #else
         graph_size.y = (style.FramePadding.y * 1) + blockHeight * 1;
      #endif
      const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + graph_size);
      const ImRect inner_bb(frame_bb.Min + style.FramePadding, frame_bb.Max - style.FramePadding);
      const ImRect total_bb(frame_bb.Min, frame_bb.Max);
      ImGui::ItemSize(total_bb, style.FramePadding.y);
      if (ImGui::ItemAdd(total_bb, 0, &frame_bb))
      {
         ImGui::RenderFrame(frame_bb.Min, frame_bb.Max, ImGui::GetColorU32(ImGuiCol_FrameBg), true, style.FrameRounding);
         U64 minTS = UINT64_MAX;
         //U64 maxTS = 0;
         for (int i = 0; i < 5; i++)
         {
            FrameProfiler *profiler = i == 0 ? &m_player->m_logicProfiler : m_player->m_renderProfiler;
            if (profiler->GetPrevStart(sections[i]) == 0)
               continue;
            minTS = std::min(minTS, static_cast<U64>(profiler->GetPrevStart(sections[i])));
            //maxTS = max(maxTS, static_cast<U64>(profiler->GetPrevEnd(sections[i])));
         }
         const float elapse = static_cast<float>(m_player->m_logicProfiler.GetSlidingAvg(FrameProfiler::PROFILE_FRAME)) * 1.5f;
         const float width = inner_bb.Max.x - inner_bb.Min.x;
         for (int i = 0; i < 6; i++)
         {
            FrameProfiler *profiler = i == 0 ? &m_player->m_logicProfiler : m_player->m_renderProfiler;
            if (profiler->GetPrevStart(sections[i]) == 0)
               continue;
            float start = static_cast<float>(profiler->GetPrevStart(sections[i]) - minTS) / elapse;
            float end = static_cast<float>(profiler->GetPrevEnd(sections[i]) - minTS) / elapse;
            #ifdef ENABLE_BGFX
            const float height = blockHeight * (i == 0 ? 1.f : 0.f) - style.FramePadding.y;
            if (i == 5)
            {
               // For BGFX submit is done in 2 parts: VPX->BGFX, then BGFX->GPU after flip
               U64 submit1 = profiler->GetPrevEnd(FrameProfiler::PROFILE_RENDER_SUBMIT) - profiler->GetPrevStart(FrameProfiler::PROFILE_RENDER_SUBMIT);
               U64 submit2 = profiler->GetPrev(FrameProfiler::PROFILE_RENDER_SUBMIT) - submit1;
               start = static_cast<float>(profiler->GetPrevEnd(FrameProfiler::PROFILE_RENDER_FLIP) - minTS) / elapse;
               end = static_cast<float>(profiler->GetPrevEnd(FrameProfiler::PROFILE_RENDER_FLIP) - minTS + submit2) / elapse;
            }
            #else
            const float height = blockHeight * 0 - style.FramePadding.y;
            #endif
            const ImVec2 pos0 = inner_bb.Min + ImVec2(start * width, height);
            const ImVec2 pos1 = inner_bb.Min + ImVec2(end * width, height + blockHeight * 0.9f);
            if (ImGui::IsMouseHoveringRect(pos0, pos1))
               ImGui::SetTooltip("%s: %5.1fms\n%s", infoLabels[sections[i]].c_str(), (end - start) * elapse * 1e-3f, infoLabels2[sections[i]].c_str());
            window->DrawList->AddRectFilled(pos0, pos1, cols[i]);
         }
      }
   }

   // Main frame timing table
   if ((m_show_fps == 2) && ImGui::BeginTable("Timings", 3, ImGuiTableFlags_Borders))
   {
      const U32 period = m_player->m_logicProfiler.GetPrev(FrameProfiler::PROFILE_FRAME);
      ImGui::TableSetupColumn("##Cat", ImGuiTableColumnFlags_WidthFixed);
      ImGui::TableSetupColumn(m_showAvgFPS ? "Avg Time" : "Time", ImGuiTableColumnFlags_WidthFixed);
      ImGui::TableSetupColumn(m_showAvgFPS ? "Avg Ratio" : "Ratio", ImGuiTableColumnFlags_WidthFixed);
      ImGui::TableHeadersRow();

      #define PROF_ROW(name, section) \
      ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::Text("%s",name); \
      if (m_showAvgFPS) { \
      ImGui::TableNextColumn(); ImGui::Text("%4.1fms", profiler->GetSlidingAvg(section) * 1e-3); \
      ImGui::TableNextColumn(); ImGui::Text("%4.1f%%", profiler->GetSlidingRatio(section) * 100.0); \
      } else { \
      ImGui::TableNextColumn(); ImGui::Text("%4.1fms", profiler->GetPrev(section) * 1e-3); \
      ImGui::TableNextColumn(); ImGui::Text("%4.1f%%", profiler->GetPrev(section) * 100.0 / period); \
      }

      const int hoveredRow = ImGui::TableGetHoveredRow();
      int renderRow = 1, logicRow = 1, rowOffset = 0;

      FrameProfiler* profiler = g_pplayer->m_renderProfiler;
      #ifdef ENABLE_BGFX
         PROF_ROW("Render Thread", FrameProfiler::PROFILE_FRAME)
         PROF_ROW("> Wait", FrameProfiler::PROFILE_RENDER_WAIT)
         if (hoveredRow == 2)
            ImGui::SetTooltip("Time spent waiting for the CPU to prepare a frame");
         else if (hoveredRow == 3)
            ImGui::SetTooltip("Time spent submitting frame from VPX to BGFX then from BGFX to GPU");
         else if (hoveredRow == 4)
            ImGui::SetTooltip("Time spent sleeping to satisfy user requested framerate");
         else if (hoveredRow == 5)
            ImGui::SetTooltip("Time spent waiting for GPU to flip (including VSYNC)");
         logicRow = 6;
         rowOffset = 2;
      #else
         PROF_ROW("Frame", FrameProfiler::PROFILE_FRAME)
      #endif
      PROF_ROW("> Submit", FrameProfiler::PROFILE_RENDER_SUBMIT)
      PROF_ROW("> Sleep", FrameProfiler::PROFILE_RENDER_SLEEP)
      PROF_ROW("> Flip", FrameProfiler::PROFILE_RENDER_FLIP)

      profiler = &m_player->m_logicProfiler;
      #ifdef ENABLE_BGFX
         PROF_ROW("Logic Thread", FrameProfiler::PROFILE_FRAME)
      #endif
      PROF_ROW("> Prepare", FrameProfiler::PROFILE_PREPARE_FRAME)
      PROF_ROW("> Physics", FrameProfiler::PROFILE_PHYSICS)
      PROF_ROW("> Script", FrameProfiler::PROFILE_SCRIPT)
      PROF_ROW("> Sleep", FrameProfiler::PROFILE_SLEEP)
      PROF_ROW("> Misc", FrameProfiler::PROFILE_MISC)
      #ifdef DEBUG
      if (m_player->m_logicProfiler.GetSlidingAvg(FrameProfiler::PROFILE_CUSTOM1) > 0) { PROF_ROW("> Debug #1", FrameProfiler::PROFILE_CUSTOM1); ImGui::TableNextRow(); }
      if (m_player->m_logicProfiler.GetSlidingAvg(FrameProfiler::PROFILE_CUSTOM2) > 0) { PROF_ROW("> Debug #2", FrameProfiler::PROFILE_CUSTOM2); ImGui::TableNextRow(); }
      if (m_player->m_logicProfiler.GetSlidingAvg(FrameProfiler::PROFILE_CUSTOM3) > 0) { PROF_ROW("> Debug #3", FrameProfiler::PROFILE_CUSTOM3); ImGui::TableNextRow(); }
      #endif

      if ((hoveredRow == renderRow) || (hoveredRow == logicRow))
         ImGui::SetTooltip("FPS: %4.1f (%4.1f average)", 1e6 / m_player->m_logicProfiler.GetPrev(FrameProfiler::PROFILE_FRAME), 1e6 / m_player->m_logicProfiler.GetSlidingAvg(FrameProfiler::PROFILE_FRAME));
      else if (hoveredRow == 5 + rowOffset)
         ImGui::SetTooltip("Time spent preparing a frame for rendering");
      else if (hoveredRow == 6 + rowOffset)
         ImGui::SetTooltip("Time spent in physics simulation\nMax: %4.1fms, %4.1fms (over last second)",
            1e-3 * m_player->m_logicProfiler.GetMax(FrameProfiler::PROFILE_PHYSICS), 1e-3 * m_player->m_physics->GetPerfLengthMax());
      else if (hoveredRow == 7 + rowOffset)
      {
         string info = m_player->m_logicProfiler.GetWorstScriptInfo();
         ImGui::SetTooltip("Time spent in game logic and script\nMax: %4.1fms, %4.1fms (over last second)\nWorst frame:\n%s",
            1e-3 * m_player->m_logicProfiler.GetMax(FrameProfiler::PROFILE_SCRIPT), 1e-3 * m_player->m_script_max,
            info.c_str());
      }

      #undef PROF_ROW

      ImGui::EndTable();
      ImGui::NewLine();
   }

   // Display simple FPS window
   #if defined(ENABLE_BGFX)
   // TODO We are missing a way to evaluate properly if we are syncing on display or not
   bool pop = (m_player->m_videoSyncMode != VideoSyncMode::VSM_NONE)
      && ((m_player->m_logicProfiler.GetSlidingAvg(FrameProfiler::PROFILE_FRAME) - 100) * m_player->m_playfieldWnd->GetRefreshRate() < 1000000);
      // && (abs(static_cast<float>(m_player->m_logicProfiler.GetPrev(FrameProfiler::PROFILE_FRAME)) - (1000000.f / m_player->m_playfieldWnd->GetRefreshRate())) < 100.f);
   if (pop)
      ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.f, 0.5f, 0.2f, 1.f)); // Rendering at target framerate => green background
   #else
   if (m_player->m_videoSyncMode == VideoSyncMode::VSM_FRAME_PACING && m_player->m_lastFrameSyncOnFPS)
      ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.f, 0.f, 0.75f, 1.f)); // Running at app regulated speed (not hardware)
   else if (m_player->m_videoSyncMode == VideoSyncMode::VSM_FRAME_PACING && !m_player->m_lastFrameSyncOnVBlank)
      ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.75f, 0.f, 0.f, 1.f)); // Running slower than expected
   #endif
   ImGui::BeginChild("FPSText", ImVec2(0.f, 0.f), ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY, ImGuiWindowFlags_NoScrollbar);
   const double frameLength = m_player->m_logicProfiler.GetSlidingAvg(FrameProfiler::PROFILE_FRAME);
   ImGui::Text("Render: %5.1ffps %4.1fms (%4.1fms)\nLatency: %4.1fms (%4.1fms max)",
      1e6 / frameLength, 1e-3 * frameLength, 1e-3 * m_player->m_logicProfiler.GetPrev(FrameProfiler::PROFILE_FRAME),
      1e-3 * m_player->m_logicProfiler.GetSlidingInputLag(false), 1e-3 * m_player->m_logicProfiler.GetSlidingInputLag(true));
   ImGui::EndChild();
   #if defined(ENABLE_BGFX)
   if (pop)
      ImGui::PopStyleColor();
   #else
   if (m_player->m_videoSyncMode == VideoSyncMode::VSM_FRAME_PACING && m_player->m_lastFrameSyncOnFPS)
      ImGui::PopStyleColor();
   else if (m_player->m_videoSyncMode == VideoSyncMode::VSM_FRAME_PACING && !m_player->m_lastFrameSyncOnVBlank)
      ImGui::PopStyleColor();
   #endif

   ImGui::End();

   // Display plots
   if (m_show_fps == 2)
   {
      constexpr ImGuiWindowFlags window_flags_plots = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
      ImGui::SetNextWindowSize(ImVec2(530, 500));
      if (m_player->m_vrDevice)
         ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.70f, io.DisplaySize.y * 0.35f), 0, ImVec2(1.f, 0.f));
      else
         ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 8.f * m_dpi, io.DisplaySize.y - 8.f * m_dpi), 0, ImVec2(1.f, 1.f));
      ImGui::Begin("Plots", nullptr, window_flags_plots);

      float t = static_cast<float>(m_player->m_time_sec);
      constexpr int rt_axis = ImPlotAxisFlags_NoTickLabels;

      m_plotFPS.SetRolling(m_showRollingFPS);
      m_plotFPSSmoothed.SetRolling(m_showRollingFPS);
      m_plotFPSSmoothed.AddPoint(t, 1e-3f * m_player->m_logicProfiler.GetPrev(FrameProfiler::PROFILE_FRAME)); // Frame time in ms
      m_plotFPS.AddPoint(t, m_plotFPS.GetLast().y * 0.95f + m_plotFPSSmoothed.GetLast().y * 0.05f);
      if (m_plotFPS.HasData() && m_plotFPSSmoothed.HasData() && ImPlot::BeginPlot("##FPS", ImVec2(-1, 150), ImPlotFlags_None))
      {
         ImPlot::SetupAxis(ImAxis_X1, nullptr, rt_axis);
         ImPlot::SetupAxis(ImAxis_Y1, nullptr, ImPlotAxisFlags_LockMin);
         ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 2000.f / m_player->GetTargetRefreshRate(), ImGuiCond_Always);
         if (m_plotFPS.m_rolling)
            ImPlot::SetupAxisLimits(ImAxis_X1, 0, m_plotFPS.m_timeSpan, ImGuiCond_Always);
         else
            ImPlot::SetupAxisLimits(ImAxis_X1, static_cast<double>(t - m_plotFPS.m_timeSpan), static_cast<double>(t), ImGuiCond_Always);
         ImPlot::PlotLine("ms Frame", &m_plotFPSSmoothed.m_data[0].x, &m_plotFPSSmoothed.m_data[0].y, m_plotFPSSmoothed.m_data.size(), 0, m_plotFPSSmoothed.m_offset, 2 * sizeof(float));
         ImPlot::PushStyleColor(ImPlotCol_Fill, ImVec4(1, 0, 0, 0.25f));
         ImPlot::PlotLine("Smoothed ms Frame", &m_plotFPS.m_data[0].x, &m_plotFPS.m_data[0].y, m_plotFPS.m_data.size(), 0, m_plotFPS.m_offset, 2 * sizeof(float));
         ImPlot::PopStyleColor();
         ImPlot::EndPlot();
      }

      m_plotPhysx.SetRolling(m_showRollingFPS);
      m_plotPhysxSmoothed.SetRolling(m_showRollingFPS);
      m_plotPhysxSmoothed.AddPoint(t, 1e-3f * m_player->m_logicProfiler.GetPrev(FrameProfiler::PROFILE_PHYSICS)); // Script in ms
      m_plotPhysx.AddPoint(t, m_plotPhysx.GetLast().y * 0.95f + m_plotPhysxSmoothed.GetLast().y * 0.05f);
      if (m_plotPhysx.HasData() && m_plotPhysxSmoothed.HasData() && ImPlot::BeginPlot("##Physics", ImVec2(-1, 150), ImPlotFlags_None))
      {
         ImPlot::SetupAxis(ImAxis_X1, nullptr, rt_axis);
         ImPlot::SetupAxis(ImAxis_Y1, nullptr, ImPlotAxisFlags_LockMin);
         ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 2.f, ImGuiCond_Always);
         if (m_plotPhysx.m_rolling)
            ImPlot::SetupAxisLimits(ImAxis_X1, 0, m_plotPhysx.m_timeSpan, ImGuiCond_Always);
         else
            ImPlot::SetupAxisLimits(ImAxis_X1, static_cast<double>(t - m_plotPhysx.m_timeSpan), static_cast<double>(t), ImGuiCond_Always);
         ImPlot::PlotLine("ms Physics", &m_plotPhysxSmoothed.m_data[0].x, &m_plotPhysxSmoothed.m_data[0].y, m_plotPhysxSmoothed.m_data.size(), 0, m_plotPhysxSmoothed.m_offset, 2 * sizeof(float));
         ImPlot::PushStyleColor(ImPlotCol_Fill, ImVec4(1, 0, 0, 0.25f));
         ImPlot::PlotLine("Smoothed ms Physics", &m_plotPhysx.m_data[0].x, &m_plotPhysx.m_data[0].y, m_plotPhysx.m_data.size(), 0, m_plotPhysx.m_offset, 2 * sizeof(float));
         ImPlot::PopStyleColor();
         ImPlot::EndPlot();
      }

      m_plotScript.SetRolling(m_showRollingFPS);
      m_plotScriptSmoothed.SetRolling(m_showRollingFPS);
      m_plotScriptSmoothed.AddPoint(t, 1e-3f * m_player->m_logicProfiler.GetPrev(FrameProfiler::PROFILE_SCRIPT)); // Physics in ms
      m_plotScript.AddPoint(t, m_plotScript.GetLast().y * 0.95f + m_plotScriptSmoothed.GetLast().y * 0.05f);
      if (m_plotScript.HasData() && m_plotScriptSmoothed.HasData() && ImPlot::BeginPlot("##Script", ImVec2(-1, 150), ImPlotFlags_None))
      {
         ImPlot::SetupAxis(ImAxis_X1, nullptr, rt_axis);
         ImPlot::SetupAxis(ImAxis_Y1, nullptr, ImPlotAxisFlags_LockMin);
         ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 2.f, ImGuiCond_Always);
         if (m_plotScript.m_rolling)
            ImPlot::SetupAxisLimits(ImAxis_X1, 0, m_plotScript.m_timeSpan, ImGuiCond_Always);
         else
            ImPlot::SetupAxisLimits(ImAxis_X1, static_cast<double>(t - m_plotScript.m_timeSpan), static_cast<double>(t), ImGuiCond_Always);
         ImPlot::PlotLine("ms Script", &m_plotScriptSmoothed.m_data[0].x, &m_plotScriptSmoothed.m_data[0].y, m_plotScriptSmoothed.m_data.size(), 0, m_plotScriptSmoothed.m_offset, 2 * sizeof(float));
         ImPlot::PushStyleColor(ImPlotCol_Fill, ImVec4(1, 0, 0, 0.25f));
         ImPlot::PlotLine("Smoothed ms Script", &m_plotScript.m_data[0].x, &m_plotScript.m_data[0].y, m_plotScript.m_data.size(), 0, m_plotScript.m_offset, 2 * sizeof(float));
         ImPlot::PopStyleColor();
         ImPlot::EndPlot();
      }
      ImGui::End();
   }
}

void LiveUI::ResetCameraFromPlayer()
{
   // Try to setup editor camera to match the used one, but only mostly since the LiveUI does not have some view setup features like off-center, ...
   m_camView = Matrix3D::MatrixScale(1.f, 1.f, -1.f) * m_renderer->GetMVP().GetView() * Matrix3D::MatrixScale(1.f, -1.f, 1.f);
}

void LiveUI::Update(const int width, const int height)
{
   // For the time being, the UI is only available inside a running player
   if (m_player == nullptr || m_player->GetCloseState() != Player::CS_PLAYING)
      return;

   m_rotation_callback_added = false;

   #if defined(ENABLE_BGFX)
   ImGui_Implbgfx_NewFrame();
   #elif defined(ENABLE_OPENGL)
   ImGui_ImplOpenGL3_NewFrame();
   #elif defined(ENABLE_DX9)
   ImGui_ImplDX9_NewFrame();
   #endif

   #if defined(ENABLE_SDL_VIDEO)
   ImGui_ImplSDL3_NewFrame();
   #else
   ImGui_ImplWin32_NewFrame();
   #endif

   ImGuiIO &io = ImGui::GetIO();
   io.DisplaySize = ImVec2(static_cast<float>(width), static_cast<float>(height)); // The render size may not match the window size used by ImGui_ImplWin32_NewFrame (for example for VR)
   io.DisplayFramebufferScale = ImVec2(1.f, 1.f); // Retina display scaling is already applied since we override the value fom NewFrame with the rt size 
   const bool isInteractiveUI = m_ShowUI || m_ShowSplashModal || m_ShowBAMModal;
   const bool isVR = m_renderer->m_stereo3D == STEREO_VR;
   // If we are only showing overlays (no mouse interaction), apply main camera rotation
   m_rotate = (isInteractiveUI || isVR) ? 0 : ((int)(m_player->m_ptable->mViewSetups[m_player->m_ptable->m_BG_current_set].GetRotation((int)io.DisplaySize.x, (int)io.DisplaySize.y) / 90.0f));
   if (m_rotate == 1 || m_rotate == 3)
   {
      const float tmp = io.DisplaySize.x;
      io.DisplaySize.x = io.DisplaySize.y;
      io.DisplaySize.y = tmp;
   }
   ImGui::NewFrame();

   ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard; // We use it for main splash popup, but need it to be disabled to allow keyboard shortcuts

   UpdateTouchUI();

   ImGuizmo::SetOrthographic(m_orthoCam);
   ImGuizmo::BeginFrame();
   ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
   ImGui::PushFont(m_baseFont);

   // Display notification (except when script has an unaligned rotation)
   const U32 tick = msec();
   float notifY = io.DisplaySize.y * 0.25f;
   const bool showNotifications = isVR || ((float)m_rotate * 90.0f == m_player->m_ptable->mViewSetups[m_player->m_ptable->m_BG_current_set].GetRotation(m_player->m_playfieldWnd->GetWidth(), m_player->m_playfieldWnd->GetHeight()));
   ImGui::PushFont(m_overlayFont);
   for (int i = (int)m_notifications.size() - 1; i >= 0; i--)
   {
      if (tick > m_notifications[i].disappearTick)
      {
         m_notifications.erase(m_notifications.begin() + i);
      }
      else if (showNotifications)
      {
          ImFont *const font = ImGui::GetFont();

          constexpr float padding = 50.f;
          const float maxWidth = io.DisplaySize.x - padding;

          vector<string> lines;
          ImVec2 text_size(0, 0);

          string line;
          std::istringstream iss(m_notifications[i].message);
          while (std::getline(iss, line)) {
              if (line.empty()) {
                 lines.push_back(line);
                 continue;
              }
              const char *textEnd = line.c_str();
              while (*textEnd) {
                 const char *nextLineTextEnd = ImGui::FindRenderedTextEnd(textEnd, nullptr);
                 ImVec2 lineSize = font->CalcTextSizeA(font->FontSize, FLT_MAX, 0.0f, textEnd, nextLineTextEnd);
                 if (lineSize.x > maxWidth)
                 {
                    const char *wrapPoint = font->CalcWordWrapPositionA(font->Scale, textEnd, nextLineTextEnd, maxWidth);
                    if (wrapPoint == textEnd)
                       wrapPoint++;
                    nextLineTextEnd = wrapPoint;
                    lineSize = font->CalcTextSizeA(font->FontSize, FLT_MAX, 0.0f, textEnd, wrapPoint);
                 }

                 string newLine(textEnd, nextLineTextEnd);
                 lines.push_back(newLine);

                 if (lineSize.x > text_size.x)
                    text_size.x = lineSize.x;

                 textEnd = nextLineTextEnd;

                 while (*textEnd == ' ')
                    textEnd++;
              }
          }
          text_size.x += (padding / 2.f);
          text_size.y = ((float)lines.size() * ImGui::GetTextLineHeightWithSpacing()) + (padding / 2.f);

          constexpr ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
          ImGui::SetNextWindowBgAlpha(0.666f);
          ImGui::SetNextWindowPos(ImVec2((io.DisplaySize.x - text_size.x) / 2, notifY));
          ImGui::SetNextWindowSize(text_size);
          ImGui::Begin("Notification"s.append(std::to_string(i)).c_str(), nullptr, window_flags);
          for (const string& lline : lines) {
             ImVec2 lineSize = font->CalcTextSizeA(font->FontSize, FLT_MAX, 0.0f, lline.c_str());
             ImGui::SetCursorPosX(((text_size.x - lineSize.x) / 2));
             ImGui::Text("%s", lline.c_str());
          }
          ImGui::End();
          notifY += text_size.y + 10.f;
      }
   }
   ImGui::PopFont();

   if (isInteractiveUI)
   { // Main UI
      UpdateMainUI();
   }
   else if (m_tweakMode && showNotifications)
   { // Tweak UI
      UpdateTweakModeUI();
   }
   else if (m_player->m_throwBalls || m_player->m_ballControl)
   { // No UI displayed: process ball control & throw balls
      if (m_player->m_throwBalls && ImGui::IsMouseDown(ImGuiMouseButton_Right))
      {
         const ImVec2 mousePos = ImGui::GetMousePos();
         POINT point { (LONG)mousePos.x, (LONG)mousePos.y };
         const Vertex3Ds vertex = m_renderer->Get3DPointFrom2D(width, height, point);
         for (size_t i = 0; i < m_player->m_vball.size(); i++)
         {
            HitBall *const pBall = m_player->m_vball[i];
            const float dx = fabsf(vertex.x - pBall->m_d.m_pos.x);
            const float dy = fabsf(vertex.y - pBall->m_d.m_pos.y);
            if (dx < pBall->m_d.m_radius * 2.f && dy < pBall->m_d.m_radius * 2.f)
            {
                m_player->DestroyBall(pBall);
                break;
            }
         }
      }
      else if (m_player->m_throwBalls && (ImGui::GetMouseDragDelta().x != 0.f || ImGui::GetMouseDragDelta().y != 0.f))
      {
         const ImVec2 mousePos = ImGui::GetMousePos();
         const ImVec2 mouseDrag = ImGui::GetMouseDragDelta();
         const ImVec2 mouseInitalPos = mousePos - mouseDrag;
         const POINT point { (LONG)mouseInitalPos.x, (LONG)mouseInitalPos.y };
         const Vertex3Ds vertex = m_renderer->Get3DPointFrom2D(width, height, point);
         const POINT newPoint { (LONG)mousePos.x, (LONG)mousePos.y };
         const Vertex3Ds vert = m_renderer->Get3DPointFrom2D(width, height, newPoint);

         ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
         ImGui::SetNextWindowPos(ImVec2(0, 0));
         ImGui::PushStyleColor(ImGuiCol_WindowBg, 0);
         ImGui::PushStyleColor(ImGuiCol_Border, 0);
         ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
         ImGui::Begin("Ball throw overlay", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus);
         ImGui::GetWindowDrawList()->AddLine(mousePos, mouseInitalPos, IM_COL32(255, 128, 0, 255));
         ImGui::End();
         ImGui::PopStyleVar();
         ImGui::PopStyleColor(2);

         if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) || ImGui::IsMouseReleased(ImGuiMouseButton_Middle))
         {
            float vx = mouseDrag.x * 0.1f;
            float vy = mouseDrag.y * 0.1f;
            const float radangle = ANGTORAD(m_live_table->mViewSetups[m_live_table->m_BG_current_set].mViewportRotation);
            const float sn = sinf(radangle);
            const float cs = cosf(radangle);
            const float vx2 = cs * vx - sn * vy;
            const float vy2 = sn * vx + cs * vy;
            vx = -vx2;
            vy = -vy2;

            if (m_player->m_ballControl)
            { // If Ball Control and Throw Balls are both checked, that means we want ball throwing behavior with the sensed active ball, instead of creating new ones
               HitBall *const pBall = m_player->m_pactiveballBC;
               if (pBall)
               {
                  pBall->m_d.m_pos.x = vert.x;
                  pBall->m_d.m_pos.y = vert.y;
                  pBall->m_d.m_vel.x = vx;
                  pBall->m_d.m_vel.y = vy;
               }
            }
            else
            {
               bool ballGrabbed = false;
               if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
               {
                  for (size_t i = 0; i < m_player->m_vball.size(); i++)
                  {
                     HitBall *const pBall = m_player->m_vball[i];
                     const float dx = fabsf(vertex.x - pBall->m_d.m_pos.x);
                     const float dy = fabsf(vertex.y - pBall->m_d.m_pos.y);
                     if (dx < pBall->m_d.m_radius * 2.f && dy < pBall->m_d.m_radius * 2.f)
                     {
                        ballGrabbed = true;
                        pBall->m_d.m_pos.x = vert.x;
                        pBall->m_d.m_pos.y = vert.y;
                        pBall->m_d.m_vel.x = vx;
                        pBall->m_d.m_vel.y = vy;
                        pBall->m_d.m_mass = 1.f;
                        break;
                     }
                  }
               }
               if (!ballGrabbed)
               {
                  const float z = ImGui::IsMouseReleased(ImGuiMouseButton_Middle) ? m_live_table->m_glassTopHeight : 0.f;
                  HitBall *const pball = m_player->CreateBall(vertex.x, vertex.y, z, vx, vy, 0, (float)m_player->m_debugBallSize * 0.5f, m_player->m_debugBallMass);
                  pball->m_pBall->AddRef();
               }
            }
         }
      }
      else if (!m_player->m_throwBalls && m_player->m_ballControl && ImGui::IsMouseDown(ImGuiMouseButton_Left))
      {
         // Note that ball control release is handled by pininput
         const ImVec2 mousePos = ImGui::GetMousePos();
         POINT point { (LONG)mousePos.x, (LONG)mousePos.y };
         m_player->m_pBCTarget = new Vertex3Ds(m_renderer->Get3DPointFrom2D(width, height, point));
         m_player->m_pBCTarget->x = clamp(m_player->m_pBCTarget->x, 0.f, m_live_table->m_right);
         m_player->m_pBCTarget->y = clamp(m_player->m_pBCTarget->y, 0.f, m_live_table->m_bottom);
         if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
         {
            // Double click.  Move the ball directly to the target if possible.
            // Drop it from the glass height, so it will appear over any object (or on a raised playfield)
            HitBall *const pBall = m_player->m_pactiveballBC;
            if (pBall && !pBall->m_d.m_lockedInKicker)
            {
                pBall->m_d.m_pos.x = m_player->m_pBCTarget->x;
                pBall->m_d.m_pos.y = m_player->m_pBCTarget->y;
                pBall->m_d.m_pos.z = m_live_table->m_glassTopHeight;
                pBall->m_d.m_vel.x = 0.0f;
                pBall->m_d.m_vel.y = 0.0f;
                pBall->m_d.m_vel.z = -1000.0f;
            }
         }
      }
   }

   UpdatePerfOverlay();

   ImGui::PopFont();
   ImGui::EndFrame();
}

void LiveUI::OpenTweakMode()
{
   m_ShowUI = false;
   m_ShowSplashModal = false;
   if (!m_staticPrepassDisabled)
   {
      m_staticPrepassDisabled = true;
      m_renderer->DisableStaticPrePass(true);
   }
   m_tweakMode = true;
   m_tweakPages.clear();
   if (!m_table->m_szRules.empty())
      m_tweakPages.push_back(TP_Rules);
   if (m_renderer->m_stereo3D != STEREO_VR)
      m_tweakPages.push_back(TP_PointOfView);
   m_tweakPages.push_back(TP_TableOption);
   for (int j = 0; j < g_pvp->m_settings.GetNPluginSections(); j++)
   {
      int nOptions = 0;
      int nCustomOptions = (int)m_live_table->m_settings.GetPluginSettings().size();
      for (int i = 0; i < nCustomOptions; i++)
         if ((m_live_table->m_settings.GetPluginSettings()[i].section == Settings::Plugin00 + j) && (m_live_table->m_settings.GetPluginSettings()[i].showMask & VPX_OPT_SHOW_TWEAK))
            nOptions++;
      if (nOptions > 0)
         m_tweakPages.push_back((TweakPage)(TP_Plugin00 + j));
   }
   if (!m_table->m_szDescription.empty())
      m_tweakPages.push_back(TP_Info);
   m_activeTweakPageIndex = 0;
   m_activeTweakIndex = 0;
   UpdateTweakPage();
}

void LiveUI::CloseTweakMode()
{
   if (m_tweakMode)
      m_live_table->FireKeyEvent(DISPID_GameEvents_OptionEvent, 3 /* tweak mode closed event */);
   m_tweakMode = false;
}

void LiveUI::UpdateTweakPage()
{
   m_tweakPageOptions.clear();
   m_tweakPageOptions.push_back(BS_Page);
   switch (m_tweakPages[m_activeTweakPageIndex])
   {
   case TP_Info:
   case TP_Rules:
      break;
   case TP_PointOfView:
      switch (m_live_table->mViewSetups[m_live_table->m_BG_current_set].mMode)
      {
      case VLM_LEGACY:
         m_tweakPageOptions.push_back(BS_ViewMode);
         m_tweakPageOptions.push_back(BS_LookAt);
         m_tweakPageOptions.push_back(BS_FOV);
         m_tweakPageOptions.push_back(BS_Layback);
         m_tweakPageOptions.push_back(BS_XYZScale);
         m_tweakPageOptions.push_back(BS_XScale);
         m_tweakPageOptions.push_back(BS_YScale);
         m_tweakPageOptions.push_back(BS_ZScale);
         m_tweakPageOptions.push_back(BS_XOffset);
         m_tweakPageOptions.push_back(BS_YOffset);
         m_tweakPageOptions.push_back(BS_ZOffset);
         break;
      case VLM_CAMERA:
         m_tweakPageOptions.push_back(BS_ViewMode);
         m_tweakPageOptions.push_back(BS_FOV);
         m_tweakPageOptions.push_back(BS_ViewHOfs);
         m_tweakPageOptions.push_back(BS_ViewVOfs);
         m_tweakPageOptions.push_back(BS_XYZScale);
         m_tweakPageOptions.push_back(BS_XScale);
         m_tweakPageOptions.push_back(BS_YScale);
         m_tweakPageOptions.push_back(BS_ZScale);
         m_tweakPageOptions.push_back(BS_LookAt);
         m_tweakPageOptions.push_back(BS_XOffset);
         m_tweakPageOptions.push_back(BS_YOffset);
         m_tweakPageOptions.push_back(BS_ZOffset);
         break;
      case VLM_WINDOW:
         m_tweakPageOptions.push_back(BS_ViewMode);
         m_tweakPageOptions.push_back(BS_ViewHOfs);
         m_tweakPageOptions.push_back(BS_ViewVOfs);
         m_tweakPageOptions.push_back(BS_XYZScale);
         m_tweakPageOptions.push_back(BS_XScale);
         m_tweakPageOptions.push_back(BS_YScale);
         m_tweakPageOptions.push_back(BS_WndTopZOfs);
         m_tweakPageOptions.push_back(BS_WndBottomZOfs);
         m_tweakPageOptions.push_back(BS_XOffset);
         m_tweakPageOptions.push_back(BS_YOffset);
         m_tweakPageOptions.push_back(BS_ZOffset);
         break;
      }
      break;
   case TP_TableOption:
   {
      int nCustomOptions = (int)m_live_table->m_settings.GetTableSettings().size();
      for (int i = 0; i < nCustomOptions; i++)
         m_tweakPageOptions.push_back((BackdropSetting)(BS_Custom + i));
      m_tweakPageOptions.push_back(BS_DayNight);
      m_tweakPageOptions.push_back(BS_Exposure);
      if (!m_player || !m_player->m_renderer->m_HDRforceDisableToneMapper || !m_player->m_playfieldWnd->IsWCGBackBuffer())
         m_tweakPageOptions.push_back(BS_Tonemapper);
      m_tweakPageOptions.push_back(BS_Difficulty);
      m_tweakPageOptions.push_back(BS_MusicVolume);
      m_tweakPageOptions.push_back(BS_SoundVolume);
      break;
   }
   default: // Plugin options
   {
      int nCustomOptions = (int)m_live_table->m_settings.GetPluginSettings().size();
      for (int i = 0; i < nCustomOptions; i++)
         if (m_live_table->m_settings.GetPluginSettings()[i].section == Settings::Plugin00 + (m_tweakPages[m_activeTweakPageIndex] - TP_Plugin00) && (m_live_table->m_settings.GetPluginSettings()[i].showMask & VPX_OPT_SHOW_TWEAK))
            m_tweakPageOptions.push_back((BackdropSetting)(BS_Custom + i));
      break;
   }
   }
   if (m_activeTweakIndex >= (int)m_tweakPageOptions.size())
      m_activeTweakIndex = (int)m_tweakPageOptions.size() - 1;
}

void LiveUI::OnTweakModeEvent(const int keyEvent, const int keycode)
{
   if (!IsTweakMode())
      return;
   BackdropSetting activeTweakSetting = m_tweakPageOptions[m_activeTweakIndex];
   PinTable * const table = m_live_table;
   
   // Handle scrolling in rules/infos
   if ((m_tweakPages[m_activeTweakPageIndex] == TP_Rules || m_tweakPages[m_activeTweakPageIndex] == TP_Info)
      && (keycode == m_player->m_rgKeys[eRightMagnaSave] || keycode == m_player->m_rgKeys[eLeftMagnaSave]) && (keyEvent != 2))
   {
      const float speed = m_overlayFont->FontSize * 0.5f;
      if (keycode == m_player->m_rgKeys[eLeftMagnaSave])
         m_tweakScroll -= speed;
      else if (keycode == m_player->m_rgKeys[eRightMagnaSave])
         m_tweakScroll += speed;
   }

   if (keycode == m_player->m_rgKeys[eLeftFlipperKey] || keycode == m_player->m_rgKeys[eRightFlipperKey])
   {
      static U32 startOfPress = 0;
      if (keyEvent != 0)
         startOfPress = msec();
      if (keyEvent == 2) // Do not react on key up (only key down or long press)
         return;
      const bool up = keycode == m_player->m_rgKeys[eRightFlipperKey];
      const float step = up ? 1.f : -1.f;
      const float incSpeed = step * 0.05f * min(10.f, 0.75f + (float)(msec() - startOfPress) / 500.0f);
      ViewSetup &viewSetup = table->mViewSetups[table->m_BG_current_set];
      const bool isWindow = viewSetup.mMode == VLM_WINDOW;
      bool modified = true;
      switch (activeTweakSetting)
      {
      // UI navigation
      case BS_Page:
      {
         m_tweakState[activeTweakSetting] = 0;
         if (keyEvent != 1) // Only keydown
            return;
         int stepi = up ? 1 : (int)m_tweakPages.size() - 1;
         m_activeTweakPageIndex = ((m_activeTweakPageIndex + stepi) % m_tweakPages.size());
         m_activeTweakIndex = 0;
         m_tweakScroll = 0.f;
         UpdateTweakPage();
         break;
      }

      // View setup settings
      case BS_ViewMode:
      {
         if (keyEvent != 1) // Only keydown
            return;
         int vlm = viewSetup.mMode + (int)step;
         viewSetup.mMode = vlm < 0 ? VLM_WINDOW : vlm >= 3 ? VLM_LEGACY : (ViewLayoutMode)vlm;
         UpdateTweakPage();
         break;
      }
      case BS_LookAt: viewSetup.mLookAt += incSpeed; break;
      case BS_FOV: viewSetup.mFOV += incSpeed; break;
      case BS_Layback: viewSetup.mLayback += incSpeed; break;
      case BS_ViewHOfs: viewSetup.mViewHOfs += incSpeed; break;
      case BS_ViewVOfs: viewSetup.mViewVOfs += incSpeed; break;
      case BS_XYZScale:
         viewSetup.mSceneScaleX += 0.005f * incSpeed;
         viewSetup.mSceneScaleY += 0.005f * incSpeed;
         viewSetup.mSceneScaleZ += 0.005f * incSpeed;
         break;
      case BS_XScale: viewSetup.mSceneScaleX += 0.005f * incSpeed; break;
      case BS_YScale: viewSetup.mSceneScaleY += 0.005f * incSpeed; break;
      case BS_ZScale: viewSetup.mSceneScaleZ += 0.005f * incSpeed; break;
      case BS_XOffset:
         if (isWindow)
            table->m_settings.SaveValue(Settings::Player, "ScreenPlayerX"s, table->m_settings.LoadValueWithDefault(Settings::Player, "ScreenPlayerX"s, 0.0f) + 0.5f * incSpeed);
         else
            viewSetup.mViewX += 10.f * incSpeed;
         break;
      case BS_YOffset:
         if (isWindow)
            table->m_settings.SaveValue(Settings::Player, "ScreenPlayerY"s, table->m_settings.LoadValueWithDefault(Settings::Player, "ScreenPlayerY"s, 0.0f) + 0.5f * incSpeed);
         else
            viewSetup.mViewY += 10.f * incSpeed;
         break;
      case BS_ZOffset:
         if (isWindow)
            table->m_settings.SaveValue(Settings::Player, "ScreenPlayerZ"s, table->m_settings.LoadValueWithDefault(Settings::Player, "ScreenPlayerZ"s, 70.0f) + 0.5f * incSpeed);
         else
            viewSetup.mViewZ += (viewSetup.mMode == VLM_LEGACY ? 100.f : 10.f) * incSpeed;
         break;
      case BS_WndTopZOfs: viewSetup.mWindowTopZOfs += 10.f * incSpeed; break;
      case BS_WndBottomZOfs: viewSetup.mWindowBottomZOfs += 10.f * incSpeed; break;

      // Table customization
      case BS_DayNight:
      {
         m_renderer->m_globalEmissionScale = clamp(m_renderer->m_globalEmissionScale + incSpeed * 0.05f, 0.f, 1.f);
         m_renderer->MarkShaderDirty();
         m_live_table->FireKeyEvent(DISPID_GameEvents_OptionEvent, 1 /* table option changed event */);
         break;
      }
      case BS_Difficulty:
      {
         table->m_globalDifficulty = clamp(table->m_globalDifficulty + incSpeed * 0.05f, 0.f, 1.f);
         m_live_table->FireKeyEvent(DISPID_GameEvents_OptionEvent, 1 /* table option changed event */);
         break;
      }
      case BS_MusicVolume:
      {
         m_player->m_MusicVolume = clamp(m_player->m_MusicVolume + (int) step, 0, 100);
         m_live_table->FireKeyEvent(DISPID_GameEvents_OptionEvent, 1 /* table option changed event */);
         break;
      }
      case BS_SoundVolume:
      {
         m_player->m_SoundVolume = clamp(m_player->m_SoundVolume + (int) step, 0, 100);
         m_live_table->FireKeyEvent(DISPID_GameEvents_OptionEvent, 1 /* table option changed event */);
         break;
      }
      case BS_Exposure:
      {
         m_renderer->m_exposure = clamp(m_renderer->m_exposure + incSpeed * 0.05f, 0.f, 2.0f);
         m_live_table->FireKeyEvent(DISPID_GameEvents_OptionEvent, 1 /* table option changed event */);
         break;
      }
      case BS_Tonemapper:
      {
         if (keyEvent == 1)
         {
            int tm = m_renderer->m_toneMapper + (int)step;
            #ifdef ENABLE_BGFX
            if (tm < 0)
               tm = ToneMapper::TM_AGX_PUNCHY;
            if (tm > ToneMapper::TM_AGX_PUNCHY)
               tm = ToneMapper::TM_REINHARD;
            #else
            if (tm < 0)
               tm = ToneMapper::TM_NEUTRAL;
            if (tm > ToneMapper::TM_NEUTRAL)
               tm = ToneMapper::TM_REINHARD;
            #endif
            m_renderer->m_toneMapper = (ToneMapper)tm;
            m_live_table->FireKeyEvent(DISPID_GameEvents_OptionEvent, 1 /* table option changed event */);
         }
         break;
      }

      default:
         if (activeTweakSetting >= BS_Custom)
         {
            const vector<Settings::OptionDef> &customOptions = m_tweakPages[m_activeTweakPageIndex] == TP_TableOption ? m_live_table->m_settings.GetTableSettings() : m_live_table->m_settings.GetPluginSettings();
            if (activeTweakSetting < BS_Custom + (int)customOptions.size())
            {
               auto opt = customOptions[activeTweakSetting - BS_Custom];
               float nTotalSteps = (opt.maxValue - opt.minValue) / opt.step;
               int nMsecPerStep = nTotalSteps < 20.f ? 500 : max(5, 250 - (int)(msec() - startOfPress) / 10); // discrete vs continuous sliding
               int nSteps = (msec() - m_lastTweakKeyDown) / nMsecPerStep;
               if (keyEvent == 1)
               {
                  nSteps = 1;
                  m_lastTweakKeyDown = msec() - nSteps * nMsecPerStep;
               }
               if (nSteps > 0)
               {
                  m_lastTweakKeyDown += nSteps * nMsecPerStep;
                  float value = m_live_table->m_settings.LoadValueWithDefault(opt.section, opt.id, opt.defaultValue);
                  if (!opt.literals.empty())
                  {
                     value += (float)nSteps * opt.step * step;
                     while (value < opt.minValue)
                        value += opt.maxValue - opt.minValue + 1;
                     while (value > opt.maxValue)
                        value -= opt.maxValue - opt.minValue + 1;
                  }
                  else
                     value = clamp(value + (float)nSteps * opt.step * step, opt.minValue, opt.maxValue);
                  table->m_settings.SaveValue(opt.section, opt.id, value);
                  if (opt.section == Settings::TableOption)
                     m_live_table->FireKeyEvent(DISPID_GameEvents_OptionEvent, 1 /* table option changed event */);
                  else
                     VPXPluginAPIImpl::GetInstance().BroadcastVPXMsg(VPXPluginAPIImpl::GetInstance().GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_SETTINGS_CHANGED), nullptr);
               }
               else
                  modified = false;
            }
         }
         else
         {
            assert(false);
            break;
         }
      }
      m_tweakState[activeTweakSetting] |= modified ? 1 : 0;
   }
   else if (keyEvent == 1) // Key down
   {
      if (keycode == m_player->m_rgKeys[eLeftTiltKey] && m_live_table->m_settings.LoadValueWithDefault(Settings::Player, "EnableCameraModeFlyAround"s, false))
         m_live_table->mViewSetups[m_live_table->m_BG_current_set].mViewportRotation -= 1.0f;
      else if (keycode == m_player->m_rgKeys[eRightTiltKey] && m_live_table->m_settings.LoadValueWithDefault(Settings::Player, "EnableCameraModeFlyAround"s, false))
         m_live_table->mViewSetups[m_live_table->m_BG_current_set].mViewportRotation += 1.0f;
      else if (keycode == m_player->m_rgKeys[eStartGameKey]) // Save tweak page
      {
         string iniFileName = m_live_table->GetSettingsFileName();
         string message;
         if (m_tweakPages[m_activeTweakPageIndex] == TP_PointOfView)
         {
            message = "Point of view";
            m_live_table->mViewSetups[m_live_table->m_BG_current_set].SaveToTableOverrideSettings(m_table->m_settings, m_live_table->m_BG_current_set);
            if (m_live_table->m_BG_current_set == BG_FULLSCREEN)
            { // Player position is saved as an override (not saved if equal to app settings)
               m_table->m_settings.SaveValue(Settings::Player, "ScreenPlayerX", m_live_table->m_settings.LoadValueWithDefault(Settings::Player, "ScreenPlayerX"s, 0.0f), true);
               m_table->m_settings.SaveValue(Settings::Player, "ScreenPlayerY", m_live_table->m_settings.LoadValueWithDefault(Settings::Player, "ScreenPlayerY"s, 0.0f), true);
               m_table->m_settings.SaveValue(Settings::Player, "ScreenPlayerZ", m_live_table->m_settings.LoadValueWithDefault(Settings::Player, "ScreenPlayerZ"s, 70.0f), true);
            }
            // The saved value are the new base value, so all fields are marked as untouched
            for (int i = BS_ViewMode; i < BS_WndBottomZOfs; i++)
               m_tweakState[i] = 0;
         }
         else if (m_tweakPages[m_activeTweakPageIndex] == TP_TableOption)
         {
            // Day Night slider
            if (m_tweakState[BS_DayNight] == 1)
            {
               m_table->m_settings.SaveValue(Settings::Player, "OverrideTableEmissionScale"s, true);
               m_table->m_settings.SaveValue(Settings::Player, "DynamicDayNight"s, false);
               m_table->m_settings.SaveValue(Settings::Player, "EmissionScale"s, m_renderer->m_globalEmissionScale);
            }
            else if (m_tweakState[BS_DayNight] == 2)
            {
               m_table->m_settings.DeleteValue(Settings::Player, "OverrideTableEmissionScale"s);
               m_table->m_settings.DeleteValue(Settings::Player, "DynamicDayNight"s);
               m_table->m_settings.DeleteValue(Settings::Player, "EmissionScale"s);
            }
            m_tweakState[BS_DayNight] = 0;
            // Exposure slider
            if (m_tweakState[BS_Exposure] == 1)
               m_table->m_settings.SaveValue(Settings::TableOverride, "Exposure"s, m_renderer->m_exposure);
            else if (m_tweakState[BS_Exposure] == 2)
               m_table->m_settings.DeleteValue(Settings::TableOverride, "Exposure"s);
            m_tweakState[BS_Exposure] = 0;
            // Tonemapper
            if (m_tweakState[BS_Tonemapper] == 1)
               m_table->m_settings.SaveValue(Settings::TableOverride, "ToneMapper"s, m_renderer->m_toneMapper);
            else if (m_tweakState[BS_Tonemapper] == 2)
               m_table->m_settings.DeleteValue(Settings::TableOverride, "ToneMapper"s);
            m_tweakState[BS_Tonemapper] = 0;
            // Difficulty
            if (m_tweakState[BS_Difficulty] != 0)
               PushNotification("You have changed the difficulty level\nThis change will only be applied after restart.", 10000);
            if (m_tweakState[BS_Difficulty] == 1)
               m_table->m_settings.SaveValue(Settings::TableOverride, "Difficulty"s, m_live_table->m_globalDifficulty);
            else if (m_tweakState[BS_Difficulty] == 2)
               m_table->m_settings.DeleteValue(Settings::TableOverride, "Difficulty"s);
            m_tweakState[BS_Difficulty] = 0;
            // Music/sound volume
            if (m_tweakState[BS_MusicVolume] == 1)
               m_table->m_settings.SaveValue(Settings::Player, "MusicVolume"s, m_player->m_MusicVolume);
            else if (m_tweakState[BS_MusicVolume] == 2)
               m_table->m_settings.DeleteValue(Settings::Player, "MusicVolume"s);
            m_tweakState[BS_MusicVolume] = 0;
            if (m_tweakState[BS_SoundVolume] == 1)
               m_table->m_settings.SaveValue(Settings::Player, "SoundVolume"s, m_player->m_SoundVolume);
            else if (m_tweakState[BS_SoundVolume] == 2)
               m_table->m_settings.DeleteValue(Settings::Player, "SoundVolume"s);
            m_tweakState[BS_SoundVolume] = 0;
         }
         // Custom table/plugin options
         if (m_tweakPages[m_activeTweakPageIndex] >= TP_TableOption)
         {
            const vector<Settings::OptionDef> &customOptions = m_tweakPages[m_activeTweakPageIndex] == TP_TableOption ? m_live_table->m_settings.GetTableSettings() : m_live_table->m_settings.GetPluginSettings();
            message = m_tweakPages[m_activeTweakPageIndex] > TP_TableOption ? "Plugin options" : "Table options";
            int nOptions = (int)customOptions.size();
            for (int i = 0; i < nOptions; i++)
            {
               auto opt = customOptions[i];
               if ((opt.section == Settings::TableOption && m_tweakPages[m_activeTweakPageIndex] == TP_TableOption)
                  || (opt.section > Settings::TableOption && m_tweakPages[m_activeTweakPageIndex] == static_cast<int>(TP_Plugin00) + static_cast<int>(opt.section) - static_cast<int>(Settings::Plugin00)))
               {
                  if (m_tweakState[BS_Custom + i] == 2)
                     m_table->m_settings.DeleteValue(opt.section, opt.id);
                  else
                     m_table->m_settings.SaveValue(opt.section, opt.id, m_live_table->m_settings.LoadValueWithDefault(opt.section, opt.name, opt.defaultValue));
                  m_tweakState[BS_Custom + i] = 0;
               }
            }
         }
         if (m_table->m_szFileName.empty() || !FileExists(m_table->m_szFileName))
         {
            PushNotification("You need to save your table before exporting user settings"s, 5000);
         }
         else
         {
            m_table->m_settings.SaveToFile(iniFileName);
            PushNotification(message + " exported to "s.append(iniFileName), 5000);
         }
         if (g_pvp->m_povEdit && m_tweakPages[m_activeTweakPageIndex] == TP_PointOfView)
            g_pvp->QuitPlayer(Player::CloseState::CS_CLOSE_APP);
      }
      else if (keycode == m_player->m_rgKeys[ePlungerKey]) // Reset tweak page
      {
         if (m_tweakPages[m_activeTweakPageIndex] == TP_TableOption)
         {
            // Remove custom day/night and get back to the one of the table, eventually overriden by app (not table) settings
            // FIXME we just default to the table value, missing the app settings being applied (like day/night from lat/lon,... see in player.cpp)
            m_tweakState[BS_DayNight] = 2;
            m_renderer->m_globalEmissionScale = m_table->m_globalEmissionScale;

            // Exposure
            m_tweakState[BS_Exposure] = 2;
            m_renderer->m_exposure = m_table->m_settings.LoadValueWithDefault(Settings::TableOverride, "Exposure"s, m_table->GetExposure());

            // Tonemapper
            m_tweakState[BS_Tonemapper] = 2;
            m_renderer->m_toneMapper = (ToneMapper)m_table->m_settings.LoadValueWithDefault(Settings::TableOverride, "ToneMapper"s, m_table->GetToneMapper());

            // Remove custom difficulty and get back to the one of the table, eventually overriden by app (not table) settings
            m_tweakState[BS_Difficulty] = 2;
            m_live_table->m_globalDifficulty = g_pvp->m_settings.LoadValueWithDefault(Settings::TableOverride, "Difficulty"s, m_table->m_difficulty);

            // Music/sound volume
            m_player->m_MusicVolume = m_table->m_settings.LoadValueWithDefault(Settings::Player, "MusicVolume"s, 100);
            m_player->m_SoundVolume = m_table->m_settings.LoadValueWithDefault(Settings::Player, "SoundVolume"s, 100);

            m_renderer->MarkShaderDirty();
         }
         else if (m_tweakPages[m_activeTweakPageIndex] == TP_PointOfView)
         {
            for (int i = BS_ViewMode; i < BS_WndBottomZOfs; i++)
               m_tweakState[i] = 2;
            ViewSetupID id = table->m_BG_current_set;
            ViewSetup &viewSetup = table->mViewSetups[id];
            viewSetup.mViewportRotation = 0.f;
            const bool portrait = m_player->m_playfieldWnd->GetWidth() < m_player->m_playfieldWnd->GetHeight();
            switch (id)
            {
            case BG_DESKTOP:
            case BG_FSS:
               PushNotification("POV reset to default values"s, 5000);
               if (id == BG_DESKTOP && !portrait)
               { // Desktop
                  viewSetup.mMode = (ViewLayoutMode)g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "DesktopMode"s, VLM_CAMERA);
                  viewSetup.mViewX = CMTOVPU(g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "DesktopCamX"s, 0.f));
                  viewSetup.mViewY = CMTOVPU(g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "DesktopCamY"s, 20.f));
                  viewSetup.mViewZ = CMTOVPU(g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "DesktopCamZ"s, 70.f));
                  viewSetup.mSceneScaleX = g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "DesktopScaleX"s, 1.f);
                  viewSetup.mSceneScaleY = g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "DesktopScaleY"s, 1.f);
                  viewSetup.mSceneScaleZ = g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "DesktopScaleZ"s, 1.f);
                  viewSetup.mFOV = g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "DesktopFov"s, 50.f);
                  viewSetup.mLookAt = g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "DesktopLookAt"s, 25.0f);
                  viewSetup.mViewVOfs = g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "DesktopViewVOfs"s, 14.f);
               }
               else
               { // FSS
                  viewSetup.mMode = (ViewLayoutMode)g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "FSSMode"s, VLM_CAMERA);
                  viewSetup.mViewX = CMTOVPU(g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "FSSCamX"s, 0.f));
                  viewSetup.mViewY = CMTOVPU(g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "FSSCamY"s, 20.f));
                  viewSetup.mViewZ = CMTOVPU(g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "FSSCamZ"s, 70.f));
                  viewSetup.mSceneScaleX = g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "FSSScaleX"s, 1.f);
                  viewSetup.mSceneScaleY = g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "FSSScaleY"s, 1.f);
                  viewSetup.mSceneScaleZ = g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "FSSScaleZ"s, 1.f);
                  viewSetup.mFOV = g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "FSSFov"s, 77.f);
                  viewSetup.mLookAt = g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "FSSLookAt"s, 50.0f);
                  viewSetup.mViewVOfs = g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "FSSViewVOfs"s, 22.f);
               }
               break;
            case BG_FULLSCREEN:
            {
               const float screenWidth = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "ScreenWidth"s, 0.0f);
               const float screenHeight = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "ScreenHeight"s, 0.0f);
               if (screenWidth <= 1.f || screenHeight <= 1.f)
               {
                  PushNotification("You must setup your screen size before using Window mode"s, 5000);
               }
               else
               {
                  float topHeight = table->m_glassTopHeight;
                  float bottomHeight = table->m_glassBottomHeight;
                  if (bottomHeight == topHeight)
                  { // If table does not define the glass position (for table without it, when loading we set the glass as horizontal)
                     TableDB db;
                     db.Load();
                     int bestSizeMatch = db.GetBestSizeMatch(table->GetTableWidth(), table->GetHeight(), topHeight);
                     if (bestSizeMatch >= 0)
                     {
                        bottomHeight = INCHESTOVPU(db.m_data[bestSizeMatch].glassBottom);
                        topHeight = INCHESTOVPU(db.m_data[bestSizeMatch].glassTop);
                        char textBuf1[MAXNAMEBUFFER], textBuf2[MAXNAMEBUFFER];
                        sprintf_s(textBuf1, sizeof(textBuf1), "%.02f", db.m_data[bestSizeMatch].glassBottom);
                        sprintf_s(textBuf2, sizeof(textBuf2), "%.02f", db.m_data[bestSizeMatch].glassTop);
                        PushNotification("Missing glass position guessed to be "s + textBuf1 + "\" / " + textBuf2 + "\" (" + db.m_data[bestSizeMatch].name + ')', 5000);
                     }
                     else
                     {
                        PushNotification("The table is missing glass position and no good guess was found."s, 5000);
                     }
                  }
                  const float scale = (screenHeight / table->GetTableWidth()) * (table->GetHeight() / screenWidth);
                  const bool isFitted = (viewSetup.mViewHOfs == 0.f) && (viewSetup.mViewVOfs == -2.8f) && (viewSetup.mSceneScaleY == scale) && (viewSetup.mSceneScaleX == scale);
                  viewSetup.mMode = VLM_WINDOW;
                  viewSetup.mViewHOfs = 0.f;
                  viewSetup.mViewVOfs = isFitted ? 0.f : -2.8f;
                  viewSetup.mSceneScaleX = scale;
                  viewSetup.mSceneScaleY = isFitted ? 1.f : scale;
                  viewSetup.mWindowBottomZOfs = bottomHeight;
                  viewSetup.mWindowTopZOfs = topHeight;
                  PushNotification(isFitted ? "POV reset to default values (stretch to fit)"s : "POV reset to default values (no stretching)"s, 5000);
               }
               break;
            }
            case BG_INVALID:
            case NUM_BG_SETS: assert(false); break;
            }
            m_renderer->m_cam = Vertex3Ds(0.f, 0.f, 0.f);
            UpdateTweakPage();
         }
         // Reset custom table/plugin options
         if (m_tweakPages[m_activeTweakPageIndex] >= TP_TableOption)
         {
            if (m_tweakPages[m_activeTweakPageIndex] > TP_TableOption)
               PushNotification("Plugin options reset to default values"s, 5000);
            else
               PushNotification("Table options reset to default values"s, 5000);
            const vector<Settings::OptionDef> &customOptions = m_tweakPages[m_activeTweakPageIndex] == TP_TableOption ? m_live_table->m_settings.GetTableSettings() : m_live_table->m_settings.GetPluginSettings();
            int nOptions = (int)customOptions.size();
            for (int i = 0; i < nOptions; i++)
            {
               auto opt = customOptions[i];
               if ((opt.section == Settings::TableOption && m_tweakPages[m_activeTweakPageIndex] == TP_TableOption)
                  || (opt.section > Settings::TableOption && m_tweakPages[m_activeTweakPageIndex] == static_cast<int>(TP_Plugin00) + static_cast<int>(opt.section) - static_cast<int>(Settings::Plugin00)))
               {
                  if (m_tweakState[BS_Custom + i] == 2)
                     m_table->m_settings.DeleteValue(opt.section, opt.id);
                  else
                     m_table->m_settings.SaveValue(opt.section, opt.id, m_live_table->m_settings.LoadValueWithDefault(opt.section, opt.id, opt.defaultValue));
                  m_tweakState[BS_Custom + i] = 0;
               }
            }
            if (m_tweakPages[m_activeTweakPageIndex] > TP_TableOption)
               VPXPluginAPIImpl::GetInstance().BroadcastVPXMsg(VPXPluginAPIImpl::GetInstance().GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_SETTINGS_CHANGED), nullptr);
            else
               m_live_table->FireKeyEvent(DISPID_GameEvents_OptionEvent, 2 /* custom option resetted event */);
         }
      }
      else if (keycode == m_player->m_rgKeys[eAddCreditKey]) // Undo tweaks of page
      {
         if (g_pvp->m_povEdit)
            // Tweak mode from command line => quit
            g_pvp->QuitPlayer(Player::CloseState::CS_CLOSE_APP);
         else
         {
            // Undo POV: copy from startup table to the live one
            if (m_tweakPages[m_activeTweakPageIndex] == TP_PointOfView)
            {
               PushNotification("POV undo to startup values"s, 5000);
               ViewSetupID id = m_live_table->m_BG_current_set;
               const PinTable *const __restrict src = m_player->m_pEditorTable;
               PinTable *const __restrict dst = m_live_table;
               dst->mViewSetups[id] = src->mViewSetups[id];
               dst->mViewSetups[id].ApplyTableOverrideSettings(m_live_table->m_settings, (ViewSetupID)id);
               m_renderer->m_cam = Vertex3Ds(0.f, 0.f, 0.f);
            }
            if (m_tweakPages[m_activeTweakPageIndex] == TP_TableOption)
            {
               // TODO undo Day/Night, difficulty, ...
            }
         }
      }
      else if (keycode == m_player->m_rgKeys[eRightMagnaSave] || keycode == m_player->m_rgKeys[eLeftMagnaSave])
      {
         if (keycode == m_player->m_rgKeys[eRightMagnaSave])
         {
            m_activeTweakIndex++;
            if (m_activeTweakIndex >= (int) m_tweakPageOptions.size())
               m_activeTweakIndex = 0;
         }
         else
         {
            m_activeTweakIndex--;
            if (m_activeTweakIndex < 0)
               m_activeTweakIndex = (int)m_tweakPageOptions.size() - 1;
         }
      }
   }
   else
   {
      if (keycode == eLeftTiltKey && m_live_table->m_settings.LoadValueWithDefault(Settings::Player, "EnableCameraModeFlyAround"s, false))
         m_live_table->mViewSetups[m_live_table->m_BG_current_set].mViewportRotation -= 1.0f;
      if (keycode == eRightTiltKey && m_live_table->m_settings.LoadValueWithDefault(Settings::Player, "EnableCameraModeFlyAround"s, false))
         m_live_table->mViewSetups[m_live_table->m_BG_current_set].mViewportRotation += 1.0f;
   }
}

void LiveUI::UpdateTweakModeUI()
{
   ImGui::PushFont(m_overlayFont);
   PinTable *const table = m_live_table;
   constexpr ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
   ImVec2 minSize(min(m_overlayFont->FontSize * (m_tweakPages[m_activeTweakPageIndex] == TP_Rules ? 35.0f
                                               : m_tweakPages[m_activeTweakPageIndex] == TP_Info  ? 45.0f
                                                                                                  : 30.0f),
                  min(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y)),0.f);
   ImVec2 maxSize(ImGui::GetIO().DisplaySize.x - 2.f * m_overlayFont->FontSize, 0.8f * ImGui::GetIO().DisplaySize.y - 1.f * m_overlayFont->FontSize);
   ImGui::SetNextWindowBgAlpha(0.5f);
   if (m_player->m_vrDevice)
      ImGui::SetNextWindowPos(ImVec2(0.5f * ImGui::GetIO().DisplaySize.x, 0.5f * ImGui::GetIO().DisplaySize.y), 0, ImVec2(0.5f, 0.5f));
   else
      ImGui::SetNextWindowPos(ImVec2(0.5f * ImGui::GetIO().DisplaySize.x, 0.8f * ImGui::GetIO().DisplaySize.y), 0, ImVec2(0.5f, 1.f));
   ImGui::SetNextWindowSizeConstraints(minSize, maxSize);
   ImGui::Begin("TweakUI", nullptr, window_flags);

   ViewSetupID vsId = table->m_BG_current_set;
   ViewSetup &viewSetup = table->mViewSetups[vsId];
   const bool isLegacy = viewSetup.mMode == VLM_LEGACY;
   const bool isCamera = viewSetup.mMode == VLM_CAMERA;
   const bool isWindow = viewSetup.mMode == VLM_WINDOW;

   BackdropSetting activeTweakSetting = m_tweakPageOptions[m_activeTweakIndex];
   if (ImGui::BeginTable("TweakTable", 4, /* ImGuiTableFlags_Borders */ 0))
   {
      static float vWidth = 50.f;
      ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthStretch);
      ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed, vWidth);
      ImGui::TableSetupColumn("Unit", ImGuiTableColumnFlags_WidthFixed);
      ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
      #define CM_ROW(id, label, format, value, unit) \
      { \
         char buf[1024]; snprintf(buf, sizeof(buf), format, value); \
         ImGui::TableNextColumn(); ImGui::Text("%s",label); ImGui::TableNextColumn(); \
         float textWidth = ImGui::CalcTextSize(buf).x; vWidth = max(vWidth, textWidth); \
         if (textWidth < vWidth) ImGui::SameLine(vWidth - textWidth); \
         ImGui::Text("%s", buf); ImGui::TableNextColumn(); \
         ImGui::Text("%s", unit); ImGui::TableNextColumn(); \
         ImGui::Text("%s", m_tweakState[id] == 0 ? "  " : m_tweakState[id] == 1 ? " **" : " *"); ImGui::TableNextRow(); \
      }
      #define CM_SKIP_LINE {ImGui::TableNextColumn(); ImGui::Dummy(ImVec2(0.f, m_dpi * 3.f)); ImGui::TableNextRow();}
      const float realToVirtual = viewSetup.GetRealToVirtualScale(table);
      for (int setting : m_tweakPageOptions)
      {
         const bool highlight = (setting == activeTweakSetting)
                             || (activeTweakSetting == BS_XYZScale && (setting == BS_XScale || setting == BS_YScale || setting == BS_ZScale));
         if (highlight)
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
         if (setting >= BS_Custom)
         {
            const vector<Settings::OptionDef> &customOptions = m_tweakPages[m_activeTweakPageIndex] == TP_TableOption ? m_live_table->m_settings.GetTableSettings() : m_live_table->m_settings.GetPluginSettings();
            if (setting - BS_Custom >= (int)customOptions.size())
               continue;
            const Settings::OptionDef &opt = customOptions[setting - BS_Custom];
            float value = table->m_settings.LoadValueWithDefault(opt.section, opt.id, opt.defaultValue);
            const string label = opt.name + ": ";
            if (!opt.literals.empty()) // List of values
            {
               int index = (int) (value - opt.minValue);
               if (index < 0 || index >= (int)opt.literals.size())
                  index = (int)(opt.defaultValue - opt.minValue);
               CM_ROW(setting, label.c_str(), "%s", opt.literals[index].c_str(), "");
            }
            else if (opt.unit == Settings::OT_PERCENT) // Percent value
            {
               CM_ROW(setting, label.c_str(), "%.1f", 100.f * value, "%");
            }
            else // OT_NONE
            {
               CM_ROW(setting, label.c_str(), "%.1f", value, "");
            }
         }
         else switch (setting)
         {
         case BS_Page: {
            const int page = m_tweakPages[m_activeTweakPageIndex];
            string title;
            if (page >= TP_Plugin00)
            {
               const string& sectionName = Settings::GetSectionName((Settings::Section)(Settings::Plugin00 + page - TP_Plugin00));
               const std::shared_ptr<MsgPlugin> plugin = sectionName.length() > 7 ? MsgPluginManager::GetInstance().GetPlugin(sectionName.substr(7)) : nullptr;
               if (plugin)
                  title = plugin->m_name + " Plugin";
               else
                  title = "Invalid Plugin"s;
            }
            else 
               title = m_tweakPages[m_activeTweakPageIndex] == TP_TableOption ? "Table Options"s
                     : m_tweakPages[m_activeTweakPageIndex] == TP_PointOfView ? "Point of View"s
                     : m_tweakPages[m_activeTweakPageIndex] == TP_Rules       ? "Rules"s
                                                                              : "Information"s;
            CM_ROW(setting, "Page "s.append(std::to_string(1 + m_activeTweakPageIndex)).append(1,'/').append(std::to_string(m_tweakPages.size())).c_str(), "%s", title.c_str(), "");
            CM_SKIP_LINE;
            break;
         }

         // View setup
         case BS_ViewMode: CM_ROW(setting, "View Layout Mode:", "%s", isLegacy ? "Legacy" : isCamera ? "Camera" : "Window", ""); CM_SKIP_LINE; break;
         case BS_XYZScale: break;
         case BS_XScale: CM_ROW(setting, "Table X Scale", "%.1f", 100.f * viewSetup.mSceneScaleX / realToVirtual, "%"); break;
         case BS_YScale: CM_ROW(setting, isWindow ? "Table YZ Scale" : "Table Y Scale", "%.1f", 100.f * viewSetup.mSceneScaleY / realToVirtual, "%"); break;
         case BS_ZScale: CM_ROW(setting, "Table Z Scale", "%.1f", 100.f * viewSetup.mSceneScaleZ / realToVirtual, "%"); CM_SKIP_LINE; break;
         case BS_LookAt:  if (isLegacy) { CM_ROW(setting, "Inclination", "%.1f", viewSetup.mLookAt, "deg"); } else { CM_ROW(setting, "Look at", "%.1f", viewSetup.mLookAt, "%"); } break;
         case BS_XOffset: CM_ROW(setting, isLegacy ? "X Offset" : isWindow ? "Player X" : "Camera X", "%.1f", isWindow ? table->m_settings.LoadValueWithDefault(Settings::Player, "ScreenPlayerX"s, 0.0f) : VPUTOCM(viewSetup.mViewX), "cm"); break;
         case BS_YOffset: CM_ROW(setting, isLegacy ? "Y Offset" : isWindow ? "Player Y" : "Camera Y", "%.1f", isWindow ? table->m_settings.LoadValueWithDefault(Settings::Player, "ScreenPlayerY"s, 0.0f) : VPUTOCM(viewSetup.mViewY), "cm"); break;
         case BS_ZOffset: CM_ROW(setting, isLegacy ? "Z Offset" : isWindow ? "Player Z" : "Camera Z", "%.1f", isWindow ? table->m_settings.LoadValueWithDefault(Settings::Player, "ScreenPlayerZ"s, 70.0f) : VPUTOCM(viewSetup.mViewZ), "cm"); CM_SKIP_LINE; break;
         case BS_FOV: CM_ROW(setting, "Field Of View (overall scale)", "%.1f", viewSetup.mFOV, "deg"); break;
         case BS_Layback: CM_ROW(setting, "Layback", "%.1f", viewSetup.mLayback, ""); CM_SKIP_LINE; break;
         case BS_ViewHOfs: CM_ROW(setting, "Horizontal Offset", "%.1f", viewSetup.mViewHOfs, isWindow ? "cm" : ""); break;
         case BS_ViewVOfs: CM_ROW(setting, "Vertical Offset", "%.1f", viewSetup.mViewVOfs, isWindow ? "cm" : ""); CM_SKIP_LINE; break;
         case BS_WndTopZOfs: CM_ROW(setting, "Window Top Z Ofs.", "%.1f", VPUTOCM(viewSetup.mWindowTopZOfs), "cm"); break;
         case BS_WndBottomZOfs: CM_ROW(setting, "Window Bottom Z Ofs.", "%.1f", VPUTOCM(viewSetup.mWindowBottomZOfs), "cm"); CM_SKIP_LINE; break;

         // Table options
         case BS_DayNight: CM_ROW(setting, "Day Night: ", "%.1f", 100.f * m_renderer->m_globalEmissionScale, "%"); break;
         case BS_Difficulty:
            char label[64];
            snprintf(label, 64, "Difficulty (%.2f° slope and trajectories scattering):", m_live_table->GetPlayfieldSlope());
            CM_ROW(setting, label, "%.1f", 100.f * m_live_table->m_globalDifficulty, "%");
            break;
         case BS_Exposure: CM_ROW(setting, "Exposure: ", "%.1f", 100.f * m_renderer->m_exposure, "%"); break;
         case BS_Tonemapper: CM_ROW(setting, "Tonemapper: ", "%s", m_renderer->m_toneMapper == TM_REINHARD   ? "Reinhard"
                                                                 : m_renderer->m_toneMapper == TM_FILMIC     ? "Filmic" 
                                                                 : m_renderer->m_toneMapper == TM_NEUTRAL    ? "Neutral"
                                                                 : m_renderer->m_toneMapper == TM_AGX        ? "AgX"
                                                                 : m_renderer->m_toneMapper == TM_AGX_PUNCHY ? "AgX Punchy"
                                                                 : m_renderer->m_toneMapper == TM_WCG_SPLINE ? "WCG Display" : "Invalid",""); // Should not happen as this tonemapper is not exposed to user
            break;
         case BS_MusicVolume: CM_ROW(setting, "Music Volume: ", "%d", m_player->m_MusicVolume, "%"); break;
         case BS_SoundVolume: CM_ROW(setting, "Sound Volume: ", "%d", m_player->m_SoundVolume, "%"); break;

         }
         if (highlight)
            ImGui::PopStyleColor();
      }
      #undef CM_ROW
      #undef CM_SKIP_LINE
      ImGui::EndTable();
   }

   if (m_tweakPages[m_activeTweakPageIndex] == TP_PointOfView)
   {
      if (isLegacy)
      {
         // Useless for absolute mode: the camera is always where we put it
         Matrix3D view = m_renderer->GetMVP().GetView();
         view.Invert();
         const vec3 pos = view.GetOrthoNormalPos();
         ImGui::Text("Camera at X: %.0fcm Y: %.0fcm Z: %.0fcm, Rotation: %.2f", 
            VPUTOCM(pos.x - 0.5f * m_live_table->m_right), 
            VPUTOCM(pos.y - m_live_table->m_bottom), 
            VPUTOCM(pos.z), viewSetup.mViewportRotation);
         ImGui::NewLine();
      }
      if (isWindow)
      {
         // Do not show it as it is more confusing than helpful due to the use of different coordinate systems in settings vs live edit
         //ImGui::Text("Camera at X: %.1fcm Y: %.1fcm Z: %.1fcm", VPUTOCM(viewSetup.mViewX), VPUTOCM(viewSetup.mViewY), VPUTOCM(viewSetup.mViewZ));
         //ImGui::NewLine();
         if (m_live_table->m_settings.LoadValueWithDefault(Settings::Player, "ScreenWidth"s, 0.0f) <= 1.f)
         {
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
            ImGui::Text("You are using 'Window' mode but haven't defined your display physical size.");
            ImGui::Text("This will break the overall scale as well as the stereo rendering.");
            ImGui::NewLine();
            ImGui::PopStyleColor();
         }
         viewSetup.SetWindowModeFromSettings(m_live_table);
      }
      m_renderer->InitLayout();
   }

   if (m_tweakPages[m_activeTweakPageIndex] == TP_Rules)
   {
      static float lastHeight = 0.f, maxScroll = 0.f;
      m_tweakScroll = clamp(m_tweakScroll, 0.f, maxScroll);
      ImGui::SetNextWindowScroll(ImVec2(0.f, m_tweakScroll));
      ImGui::SetNextWindowSizeConstraints(ImVec2(0.f, 0.f), ImVec2(FLT_MAX, lastHeight));
      if (ImGui::BeginChild("Rules", ImVec2(0.f, 0.f), 0, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar /* | ImGuiWindowFlags_AlwaysVerticalScrollbar */))
      {
         markdown_start_id = ImGui::GetItemID();
         ImGui::Markdown(m_table->m_szRules.c_str(), m_table->m_szRules.length(), markdown_config);
         lastHeight = ImGui::GetCursorPos().y - ImGui::GetCursorStartPos().y; // Height of content
         maxScroll = ImGui::GetScrollMaxY();
      }
      ImGui::EndChild();
      ImGui::NewLine();
   }
   else if (m_tweakPages[m_activeTweakPageIndex] == TP_Info)
   {
      static float lastHeight = 0.f, maxScroll = 0.f;
      m_tweakScroll = clamp(m_tweakScroll, 0.f, maxScroll);
      ImGui::SetNextWindowScroll(ImVec2(0.f, m_tweakScroll));
      ImGui::SetNextWindowSizeConstraints(ImVec2(0.f, 0.f), ImVec2(FLT_MAX, lastHeight));
      if (ImGui::BeginChild("Info", ImVec2(0.f, 0.f), 0, ImGuiWindowFlags_NoBackground))
      {
         markdown_start_id = ImGui::GetItemID();
         ImGui::Markdown(m_table->m_szDescription.c_str(), m_table->m_szDescription.length(), markdown_config);
         lastHeight = ImGui::GetCursorPos().y - ImGui::GetCursorStartPos().y; // Height of content
         maxScroll = ImGui::GetScrollMaxY();
      }
      ImGui::EndChild();
      ImGui::NewLine();
   }

   ImGui::NewLine();
   vector<string> infos;
   if (m_tweakPages[m_activeTweakPageIndex] != TP_Rules && m_tweakPages[m_activeTweakPageIndex] != TP_Info)
   {
      infos.push_back("Plunger Key:   Reset page to defaults"s);
      if (m_app->m_povEdit)
      {
         infos.push_back("Start Key:   Export settings and quit"s);
         infos.push_back("Credit Key:   Quit without export"s);
      }
      else
      {
         infos.push_back("Start Key:   Export settings file"s);
         infos.push_back("Credit Key:   Reset page to old values"s);
      }
      infos.push_back("Magna save keys:   Previous/Next option"s);
      if (m_live_table->m_settings.LoadValueWithDefault(Settings::Player, "EnableCameraModeFlyAround"s, false))
      {
         infos.push_back("Nudge key:   Rotate table orientation"s);
         infos.push_back("Arrows & Left Alt Key:   Navigate around"s);
      }
   }
   infos.push_back(activeTweakSetting == BS_Page ? "Flipper keys:   Previous/Next page"s : "Flipper keys:   Adjust highlighted value"s);
   const int info = (((int)((msec() - m_StartTime_msec) / 2000ull))) % (int)infos.size();
   ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
   HelpTextCentered(infos[info]);
   ImGui::PopStyleColor();

   ImGui::End();
   ImGui::PopFont();
}

void LiveUI::HideUI()
{ 
   CloseTweakMode();
   m_table->m_settings.Save();
   g_pvp->m_settings.Save();
   m_ShowSplashModal = false;
   m_ShowUI = false;
   m_flyMode = false;
   if (m_staticPrepassDisabled)
   {
      m_staticPrepassDisabled = false;
      m_renderer->DisableStaticPrePass(false);
   }
   m_player->SetPlayState(true);
}

void LiveUI::UpdateMainUI()
{
#if !((defined(__APPLE__) && ((defined(TARGET_OS_IOS) && TARGET_OS_IOS) || (defined(TARGET_OS_TV) && TARGET_OS_TV))) || defined(__ANDROID__))
   m_menubar_height = 0.0f;
   m_toolbar_height = 0.0f;

   // Gives some transparency when positioning camera to better view camera view bounds
   // TODO for some reasons, this breaks the modal background behavior
   //SetupImGuiStyle(m_selection.type == LiveUI::Selection::SelectionType::S_CAMERA ? 0.3f : 1.0f);

   bool popup_video_settings = false;
   bool popup_audio_settings = false;

   m_ShowBAMModal = ImGui::IsPopupOpen(ID_BAM_SETTINGS);

   bool showFullUI = true;
   showFullUI &= !m_ShowSplashModal;
   showFullUI &= !m_RendererInspection;
   showFullUI &= !m_tweakMode;
   showFullUI &= !m_ShowBAMModal;
   showFullUI &= !ImGui::IsPopupOpen(ID_VIDEO_SETTINGS);
   showFullUI &= !ImGui::IsPopupOpen(ID_ANAGLYPH_CALIBRATION);
   showFullUI &= !m_flyMode;

   if (showFullUI)
   {
      if (!m_staticPrepassDisabled)
      {
         m_staticPrepassDisabled = true;
         m_renderer->DisableStaticPrePass(true);
      }

      // Main menubar
      if (ImGui::BeginMainMenuBar())
      {
         if (!m_table->IsLocked() && ImGui::BeginMenu("Debug"))
         {
            if (ImGui::MenuItem("Open debugger"))
               m_player->m_showDebugger = true;
            if (ImGui::MenuItem("Renderer Inspection"))
               m_RendererInspection = true;
            if (ImGui::MenuItem(m_player->m_debugWindowActive ? "Play" : "Pause"))
               m_player->SetPlayState(!m_player->IsPlaying());
            ImGui::EndMenu();
         }
         if (ImGui::BeginMenu("Preferences"))
         {
            if (ImGui::MenuItem("Audio Options"))
               popup_audio_settings = true;
            if (ImGui::MenuItem("Video Options"))
               popup_video_settings = true;
            if (ImGui::MenuItem("Nudge Options"))
               m_ShowPlumb = true;
            ImGui::EndMenu();
         }
         m_menubar_height = ImGui::GetWindowSize().y;
         ImGui::EndMainMenuBar();
      }

      // Main toolbar
      m_toolbar_height = 20.f * m_dpi;
      const ImGuiViewport *const viewport = ImGui::GetMainViewport();
      ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + m_menubar_height));
      ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, m_toolbar_height));
      constexpr ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings;
      ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
      ImGui::Begin("TOOLBAR", nullptr, window_flags);
      ImGui::PopStyleVar();
      if (ImGui::Button(m_player->IsPlaying() ? ICON_FK_PAUSE : ICON_FK_PLAY))
         m_player->SetPlayState(!m_player->IsPlaying());
      ImGui::SameLine();
      ImGui::BeginDisabled(m_player->IsPlaying());
      if (ImGui::Button(ICON_FK_STEP_FORWARD))
         m_player->m_step = true;
      ImGui::EndDisabled();
      ImGui::SameLine();
      if (ImGui::Button(ICON_FK_STOP))
      {
         ImGui::CloseCurrentPopup();
         HideUI();
         m_table->QuitPlayer(Player::CS_STOP_PLAY);
      }
      // TODO move viewport options to the right of the toolbar
      ImGui::SameLine(ImGui::GetWindowWidth() - 100.f);
      if (ImGui::Button(ICON_FK_STICKY_NOTE)) // Overlay option menu
         ImGui::OpenPopup("Overlay Popup");
      if (ImGui::BeginPopup("Overlay Popup"))
      {
         ImGui::Text("Overlays:");
         ImGui::Separator();
         ImGui::Checkbox("Overlay selection", &m_selectionOverlay);
         ImGui::Separator();
         ImGui::Text("Physic Overlay:");
         if (ImGui::RadioButton("None", m_physOverlay == PO_NONE))
            m_physOverlay = PO_NONE;
         if (ImGui::RadioButton("Selected", m_physOverlay == PO_SELECTED))
            m_physOverlay = PO_SELECTED;
         if (ImGui::RadioButton("All", m_physOverlay == PO_ALL))
            m_physOverlay = PO_ALL;
         ImGui::EndPopup();
      }
      ImGui::SameLine();
      if (ImGui::Button(ICON_FK_FILTER)) // Selection filter
         ImGui::OpenPopup("Selection filter Popup");
      if (ImGui::BeginPopup("Selection filter Popup"))
      {
         bool pf = m_selectionFilter & SelectionFilter::SF_Playfield;
         bool prims = m_selectionFilter & SelectionFilter::SF_Primitives;
         bool lights = m_selectionFilter & SelectionFilter::SF_Lights;
         bool flashers = m_selectionFilter & SelectionFilter::SF_Flashers;
         ImGui::Text("Selection filters:");
         ImGui::Separator();
         if (ImGui::Checkbox("Playfield", &pf))
            m_selectionFilter = (m_selectionFilter & ~SelectionFilter::SF_Playfield) | (pf ? SelectionFilter::SF_Playfield : 0x0000);
         if (ImGui::Checkbox("Primitives", &prims))
            m_selectionFilter = (m_selectionFilter & ~SelectionFilter::SF_Primitives) | (prims ? SelectionFilter::SF_Primitives : 0x0000);
         if (ImGui::Checkbox("Lights", &lights))
            m_selectionFilter = (m_selectionFilter & ~SelectionFilter::SF_Lights) | (lights ? SelectionFilter::SF_Lights : 0x0000);
         if (ImGui::Checkbox("Flashers", &flashers))
            m_selectionFilter = (m_selectionFilter & ~SelectionFilter::SF_Flashers) | (flashers ? SelectionFilter::SF_Flashers : 0x0000);
         ImGui::EndPopup();
      }
      ImGui::End();

      // Overlay Info Text
      ImGuiIO &io = ImGui::GetIO();
      ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x - 200.f * m_dpi, io.DisplaySize.y - m_toolbar_height - m_menubar_height - 5.f * m_dpi)); // Fixed outliner width (to be adjusted when moving ImGui to the docking branch)
      ImGui::SetNextWindowPos(ImVec2(200.f * m_dpi, m_toolbar_height + m_menubar_height + 5.f * m_dpi));
      ImGui::Begin("text overlay", NULL, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoNav);
      switch (m_gizmoOperation)
      {
      case ImGuizmo::NONE: ImGui::Text("Select"); break;
      case ImGuizmo::TRANSLATE: ImGui::Text("Grab"); break;
      case ImGuizmo::ROTATE: ImGui::Text("Rotate"); break;
      case ImGuizmo::SCALE: ImGui::Text("Scale"); break;
      default: break;
      }
      ImGui::End();

      // Side panels
      UpdateOutlinerUI();
      UpdatePropertyUI();
   }

   // Popups & Modal dialogs
   if (popup_video_settings)
      ImGui::OpenPopup(ID_VIDEO_SETTINGS);
   if (ImGui::IsPopupOpen(ID_VIDEO_SETTINGS))
      UpdateVideoOptionsModal();
   if (ImGui::IsPopupOpen(ID_ANAGLYPH_CALIBRATION))
      UpdateAnaglyphCalibrationModal();

   if (popup_audio_settings)
      ImGui::OpenPopup(ID_AUDIO_SETTINGS);
   if (ImGui::IsPopupOpen(ID_AUDIO_SETTINGS))
      UpdateAudioOptionsModal();

   if (m_RendererInspection)
      UpdateRendererInspectionModal();

   if (ImGui::IsPopupOpen(ID_BAM_SETTINGS))
      UpdateHeadTrackingModal();

   UpdatePlumbWindow();
#endif

   if (m_ShowSplashModal && !ImGui::IsPopupOpen(ID_MODAL_SPLASH))
      ImGui::OpenPopup(ID_MODAL_SPLASH);
   if (m_ShowSplashModal)
      UpdateMainSplashModal();

   // Invisible full frame window for overlays
   ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
   ImGui::SetNextWindowPos(ImVec2(0, 0));
   ImGui::PushStyleColor(ImGuiCol_WindowBg, 0);
   ImGui::PushStyleColor(ImGuiCol_Border, 0);
   ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
   ImGui::Begin("overlays", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoInputs 
      | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus);
   ImDrawList *const overlayDrawList = ImGui::GetWindowDrawList();
   ImGui::End();
   ImGui::PopStyleVar();
   ImGui::PopStyleColor(2);

   // Update editor camera
   if (m_useEditorCam)
   {
      // Convert from right handed (ImGuizmo view manipulate is right handed) to VPX's left handed coordinate system
      // Right Hand to Left Hand (note that RH2LH = inverse(RH2LH), so RH2LH.RH2LH is identity, which property is used below)
      const Matrix3D RH2LH = Matrix3D::MatrixScale(1.f,  1.f, -1.f);
      const Matrix3D YAxis = Matrix3D::MatrixScale(1.f, -1.f, -1.f);
      float zNear, zFar;
      m_live_table->ComputeNearFarPlane(RH2LH * m_camView * YAxis, 1.f, zNear, zFar);

      ImGuiIO &io = ImGui::GetIO();
      if (m_orthoCam)
      {
         float viewHeight = m_camDistance;
         float viewWidth = viewHeight * io.DisplaySize.x / io.DisplaySize.y;
         m_camProj = Matrix3D::MatrixOrthoOffCenterRH(-viewWidth, viewWidth, -viewHeight, viewHeight, zNear, -zFar);
      }
      else
      {
         m_camProj = Matrix3D::MatrixPerspectiveFovRH(39.6f, io.DisplaySize.x / io.DisplaySize.y, zNear, zFar);
      }

      /*Matrix3D gridMatrix = Matrix3D::MatrixRotateX(M_PI * 0.5);
      gridMatrix.Scale(10.0f, 1.0f, 10.0f);
      ImGuizmo::DrawGrid((const float *)(m_camView.m), (const float *)(m_camProj.m), (const float *)(gridMatrix.m), 100.f);*/
   }

   // Selection manipulator
   Matrix3D transform;
   bool isSelectionTransformValid = GetSelectionTransform(transform);
   if (isSelectionTransformValid)
   {
      float camViewLH[16];
      memcpy(camViewLH, &m_camView.m[0][0], sizeof(float) * 4 * 4);
      for (int i = 8; i < 12; i++)
         camViewLH[i] = -camViewLH[i];
      Matrix3D prevTransform(transform);
      ImGuizmo::Manipulate(camViewLH, (float *)(m_camProj.m), m_gizmoOperation, m_gizmoMode, (float *)(transform.m));
      if (memcmp(transform.m, prevTransform.m, 16 * sizeof(float)) != 0)
         SetSelectionTransform(transform);
   }

   // Selection and physic colliders overlay
   {
      const float rClipWidth = (float)m_player->m_playfieldWnd->GetWidth() * 0.5f;
      const float rClipHeight = (float)m_player->m_playfieldWnd->GetHeight() * 0.5f;
      Matrix3D mvp = m_renderer->GetMVP().GetModelViewProj(0);
      auto project = [mvp, rClipWidth, rClipHeight](Vertex3Ds v)
      {
         const float xp = mvp._11 * v.x + mvp._21 * v.y + mvp._31 * v.z + mvp._41;
         const float yp = mvp._12 * v.x + mvp._22 * v.y + mvp._32 * v.z + mvp._42;
         //const float zp = mvp._13 * v.x + mvp._23 * v.y + mvp._33 * v.z + mvp._43;
         const float wp = mvp._14 * v.x + mvp._24 * v.y + mvp._34 * v.z + mvp._44;
         if (wp <= 1e-10f) // behind camera (or degenerated)
            return Vertex2D(FLT_MAX, FLT_MAX);
         const float inv_wp = 1.0f / wp;
         return Vertex2D((wp + xp) * rClipWidth * inv_wp, (wp - yp) * rClipHeight * inv_wp);
      };

      if (isSelectionTransformValid)
      {
         Vertex2D pos = project(transform.GetOrthoNormalPos());
         overlayDrawList->AddCircleFilled(ImVec2(pos.x, pos.y), 3.f * m_dpi, IM_COL32(255, 255, 255, 255), 16);
      }

      if (m_selection.type == Selection::S_EDITABLE && m_selectionOverlay)
      {
         ImGui::PushStyleColor(ImGuiCol_PlotLines, IM_COL32(255, 128, 0, 255));
         ImGui::PushStyleColor(ImGuiCol_PlotHistogram, IM_COL32(255, 128, 0, 32));
         for (auto pho : m_player->m_physics->GetUIObjects())
            if (pho->m_editable == m_selection.editable && overlayDrawList->VtxBuffer.Size < 40000)
               pho->DrawUI(project, overlayDrawList);
         ImGui::PopStyleColor(2);
      }
      
      if (m_physOverlay == PO_ALL || (m_physOverlay == PO_SELECTED && m_selection.type == Selection::S_EDITABLE))
      {
         ImGui::PushStyleColor(ImGuiCol_PlotLines, IM_COL32(255, 0, 0, 255)); // We abuse ImGui colors to pass render colors
         ImGui::PushStyleColor(ImGuiCol_PlotHistogram, IM_COL32(255, 0, 0, 64));
         for (auto pho : m_player->m_physics->GetHitObjects())
            if ((m_physOverlay == PO_ALL || (m_physOverlay == PO_SELECTED && pho->m_editable == m_selection.editable)) && overlayDrawList->VtxBuffer.Size < 40000)
               pho->DrawUI(project, overlayDrawList);
         ImGui::PopStyleColor(2);
      }
   }

   // Handle uncaught mouse & keyboard shortcuts
   if (!ImGui::GetIO().WantCaptureMouse)
   {
      // Zoom in/out with mouse wheel
      if (ImGui::GetIO().MouseWheel != 0)
      {
         m_useEditorCam = true;
         Matrix3D view(m_camView);
         view.Invert();
         const vec3 up = view.GetOrthoNormalUp(), dir = view.GetOrthoNormalDir(), pos = view.GetOrthoNormalPos();
         const vec3 camTarget = pos - dir * m_camDistance;
         m_camDistance *= powf(1.1f, -ImGui::GetIO().MouseWheel);
         const vec3 newEye = camTarget + dir * m_camDistance;
         m_camView = Matrix3D::MatrixLookAtRH(newEye, camTarget, up);
      }

      // Mouse pan
      if (ImGui::IsMouseDown(ImGuiMouseButton_Middle))
      {
         const ImVec2 drag = ImGui::GetMouseDragDelta(ImGuiMouseButton_Middle);
         ImGui::ResetMouseDragDelta(ImGuiMouseButton_Middle);
         if (drag.x != 0.f || drag.y != 0.f)
         {
            m_useEditorCam = true;
            Matrix3D viewInverse(m_camView);
            viewInverse.Invert();
            vec3 up = viewInverse.GetOrthoNormalUp(), dir = viewInverse.GetOrthoNormalDir();
            vec3 pos = viewInverse.GetOrthoNormalPos(), right = viewInverse.GetOrthoNormalRight();
            vec3 camTarget = pos - dir * m_camDistance;
            if (ImGui::GetIO().KeyShift)
            {
               camTarget = camTarget - right * drag.x + up * drag.y;
               m_camView = Matrix3D::MatrixLookAtRH(pos - right * drag.x + up * drag.y, camTarget, up);
            }
            else
            {
               const Matrix3D rx = Matrix3D::MatrixRotate(drag.x * 0.01f, up);
               const Matrix3D ry = Matrix3D::MatrixRotate(drag.y * 0.01f, right);
               const Matrix3D roll = rx * ry;
               dir = roll.MultiplyVectorNoPerspective(dir);
               dir.Normalize();

               // clamp
               vec3 planDir = CrossProduct(right, up);
               planDir.y = 0.f;
               planDir.Normalize();
               float dt = planDir.Dot(dir);
               if (dt < 0.0f)
               {
                  dir += planDir * dt;
                  dir.Normalize();
               }

               m_camView = Matrix3D::MatrixLookAtRH(camTarget + dir * m_camDistance, camTarget, up);
               m_orthoCam = false; // switch to perspective when user orbit the view (ortho is only really useful when seen from predefined ortho views)
            }
         }
      }

      // Select
      if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
      {
         // Compute mouse position in clip space
         const float rClipWidth = (float)m_player->m_playfieldWnd->GetWidth() * 0.5f;
         const float rClipHeight = (float)m_player->m_playfieldWnd->GetHeight() * 0.5f;
         const float xcoord = ((float)ImGui::GetMousePos().x - rClipWidth) / rClipWidth;
         const float ycoord = (rClipHeight - (float)ImGui::GetMousePos().y) / rClipHeight;

         // Use the inverse of our 3D transform to determine where in 3D space the
         // screen pixel the user clicked on is at.  Get the point at the near
         // clipping plane (z=0) and the far clipping plane (z=1) to get the whole
         // range we need to hit test
         Matrix3D invMVP = m_renderer->GetMVP().GetModelViewProj(0);
         invMVP.Invert();
         const Vertex3Ds v3d  = invMVP * Vertex3Ds{xcoord, ycoord, 0.f};
         const Vertex3Ds v3d2 = invMVP * Vertex3Ds{xcoord, ycoord, 1.f};
         
         // FIXME This is not really great as:
         // - picking depends on what was visible/enabled when quadtree was built (lazily at first pick), and also uses the physics quadtree for some parts
         // - primitives can have hit bug (Apron Top and Gottlieb arm of default table for example): degenerated geometry ?
         // We would need a dedicated quadtree for UI with all parts, and filter after picking by visibility
         vector<HitTestResult> vhoUnfilteredHit;
         m_player->m_physics->RayCast(v3d, v3d2, true, vhoUnfilteredHit);

         vector<HitTestResult> vhoHit;
         bool noPF = !(m_selectionFilter & SelectionFilter::SF_Playfield);
         bool noPrims = !(m_selectionFilter & SelectionFilter::SF_Primitives);
         bool noLights = !(m_selectionFilter & SelectionFilter::SF_Lights);
         bool noFlashers = !(m_selectionFilter & SelectionFilter::SF_Flashers);
         for (auto hr : vhoUnfilteredHit)
         {
            if (noPF && hr.m_obj->m_editable && hr.m_obj->m_editable->GetItemType() == ItemTypeEnum::eItemPrimitive && ((Primitive*)hr.m_obj->m_editable)->IsPlayfield())
               continue;
            if (noPrims && hr.m_obj->m_editable && hr.m_obj->m_editable->GetItemType() == ItemTypeEnum::eItemPrimitive)
               continue;
            if (noLights && hr.m_obj->m_editable && hr.m_obj->m_editable->GetItemType() == ItemTypeEnum::eItemLight)
               continue;
            if (noFlashers && hr.m_obj->m_editable && hr.m_obj->m_editable->GetItemType() == ItemTypeEnum::eItemFlasher)
               continue;
            vhoHit.push_back(hr);
         }

         if (vhoHit.empty())
            m_selection.type = Selection::SelectionType::S_NONE;
         else
         {
            size_t selectionIndex = vhoHit.size();
            for (size_t i = 0; i <= vhoHit.size(); i++)
            {
               if (i < vhoHit.size() && m_selection.type == Selection::S_EDITABLE && vhoHit[i].m_obj->m_editable == m_selection.editable)
                  selectionIndex = i + 1;
               if (i == selectionIndex)
               {
                  size_t p = selectionIndex % vhoHit.size();
                  if (vhoHit[p].m_obj->m_editable)
                     m_selection = Selection(true, vhoHit[p].m_obj->m_editable);
               }
            }
            // TODO add debug action to make ball active: m_player->m_pactiveballDebug = m_pHitBall;
         }
      }
   }
   if (!ImGui::GetIO().WantCaptureKeyboard)
   {
      if (!m_ShowSplashModal)
      {
         if (ImGui::IsKeyReleased(ImGuiKey_Escape) && m_gizmoOperation != ImGuizmo::NONE)
         {
            // Cancel current operation
            m_gizmoOperation = ImGuizmo::NONE;
         }
         else if (ImGui::IsKeyReleased(ImGuiKey_Escape) || (ImGui::IsKeyReleased(dikToImGuiKeys[m_player->m_rgKeys[eEscape]]) && !m_disable_esc))
         {
            // Open Main modal dialog
            m_esc_mode = 3; // Get back to editor if Esc is pressed
            m_ShowSplashModal = true;
         }
         else if (ImGui::IsKeyPressed(ImGuiKey_F))
         {
            m_flyMode = !m_flyMode;
         }
         else if (ImGui::IsKeyPressed(ImGuiKey_A) && ImGui::GetIO().KeyAlt)
         {
            m_selection = Selection();
         }
         else if (m_useEditorCam && ImGui::IsKeyPressed(ImGuiKey_G))
         {
            // Grab (translate)
            if (ImGui::GetIO().KeyAlt)
            {
               Matrix3D tmp;
               if (GetSelectionTransform(tmp))
                  SetSelectionTransform(tmp, true, false, false);
            }
            else
            {
               m_gizmoOperation = ImGuizmo::TRANSLATE;
               m_gizmoMode = m_gizmoOperation == ImGuizmo::TRANSLATE ? (m_gizmoMode == ImGuizmo::LOCAL ? ImGuizmo::WORLD : ImGuizmo::LOCAL) : ImGuizmo::WORLD;
            }
         }
         else if (m_useEditorCam && ImGui::IsKeyPressed(ImGuiKey_S))
         {
            // Scale
            if (ImGui::GetIO().KeyAlt)
            {
               Matrix3D tmp;
               if (GetSelectionTransform(tmp))
                  SetSelectionTransform(tmp, false, true, false);
            }
            else
            {
               m_gizmoOperation = ImGuizmo::SCALE;
               m_gizmoMode = m_gizmoOperation == ImGuizmo::SCALE ? (m_gizmoMode == ImGuizmo::LOCAL ? ImGuizmo::WORLD : ImGuizmo::LOCAL) : ImGuizmo::WORLD;
            }
         }
         else if (m_useEditorCam && ImGui::IsKeyPressed(ImGuiKey_R))
         {
            // Rotate
            if (ImGui::GetIO().KeyAlt)
            {
               Matrix3D tmp;
               if (GetSelectionTransform(tmp))
                  SetSelectionTransform(tmp, false, false, true);
            }
            else
            {
               m_gizmoOperation = ImGuizmo::ROTATE;
               m_gizmoMode = m_gizmoOperation == ImGuizmo::ROTATE ? (m_gizmoMode == ImGuizmo::LOCAL ? ImGuizmo::WORLD : ImGuizmo::LOCAL) : ImGuizmo::WORLD;
            }
         }
         else if (ImGui::IsKeyPressed(ImGuiKey_Keypad0))
         {
            // Editor toggle play camera / editor camera
            m_useEditorCam = !m_useEditorCam;
            if (m_useEditorCam)
               ResetCameraFromPlayer();
         }
         else if (ImGui::IsKeyPressed(ImGuiKey_Keypad5))
         {
            // Editor toggle orthographic / perspective
            m_useEditorCam = true;
            m_orthoCam = !m_orthoCam;
         }
         else if (ImGui::IsKeyPressed(ImGuiKey_KeypadDecimal))
         {
            // Editor Camera center on selection
            m_useEditorCam = true;
            Matrix3D view(m_camView);
            view.Invert();
            const vec3 up = view.GetOrthoNormalUp(), dir = view.GetOrthoNormalDir(), pos = view.GetOrthoNormalPos();
            const vec3 camTarget = pos - dir * m_camDistance;
            vec3 newTarget = camTarget;
            Matrix3D tmp;
            if (GetSelectionTransform(tmp))
               newTarget = vec3(tmp._41, tmp._42, tmp._43);
            const vec3 newEye = newTarget + dir * m_camDistance;
            m_camView = Matrix3D::MatrixLookAtRH(newEye, newTarget, up);
         }
         else if (ImGui::IsKeyPressed(ImGuiKey_Keypad7))
         {
            // Editor Camera to Top / Bottom
            m_useEditorCam = true;
            m_orthoCam = true;
            Matrix3D view(m_camView);
            view.Invert();
            const vec3 /*up = view.GetOrthoNormalUp(),*/ dir = view.GetOrthoNormalDir(), pos = view.GetOrthoNormalPos();
            const vec3 camTarget = pos - dir * m_camDistance;
            const vec3 newUp(0.f, -1.f, 0.f);
            const vec3 newDir(0.f, 0.f, ImGui::GetIO().KeyCtrl ? 1.f : -1.f);
            const vec3 newEye = camTarget + newDir * m_camDistance;
            m_camView = Matrix3D::MatrixLookAtRH(newEye, camTarget, newUp);
         }
         else if (ImGui::IsKeyPressed(ImGuiKey_Keypad1))
         {
            // Editor Camera to Front / Back
            m_useEditorCam = true;
            m_orthoCam = true;
            Matrix3D view(m_camView);
            view.Invert();
            const vec3 /*up = view.GetOrthoNormalUp(),*/ dir = view.GetOrthoNormalDir(), pos = view.GetOrthoNormalPos();
            const vec3 camTarget = pos - dir * m_camDistance;
            const vec3 newUp(0.f, 0.f, -1.f);
            const vec3 newDir(0.f, ImGui::GetIO().KeyCtrl ? -1.f : 1.f, 0.f);
            const vec3 newEye = camTarget + newDir * m_camDistance;
            m_camView = Matrix3D::MatrixLookAtRH(newEye, camTarget, newUp);
         }
         else if (ImGui::IsKeyPressed(ImGuiKey_Keypad3))
         {
            // Editor Camera to Right / Left
            m_useEditorCam = true;
            m_orthoCam = true;
            Matrix3D view(m_camView);
            view.Invert();
            const vec3 /*up = view.GetOrthoNormalUp(),*/ dir = view.GetOrthoNormalDir(), pos = view.GetOrthoNormalPos();
            const vec3 camTarget = pos - dir * m_camDistance;
            const vec3 newUp(0.f, 0.f, -1.f);
            const vec3 newDir(ImGui::GetIO().KeyCtrl ? 1.f : -1.f, 0.f, 0.f);
            const vec3 newEye = camTarget + newDir * m_camDistance;
            m_camView = Matrix3D::MatrixLookAtRH(newEye, camTarget, newUp);
         }
      }
   }

   const Matrix3D RH2LH = Matrix3D::MatrixScale(1.f,  1.f, -1.f);
   const Matrix3D YAxis = Matrix3D::MatrixScale(1.f, -1.f, 1.f);
   if (m_useEditorCam)
   {
      // Apply editor camera to renderer (move view/projection from right handed to left handed)
      // Convert from right handed (ImGuizmo view manipulate is right handed) to VPX's left handed coordinate system
      // Right Hand to Left Hand (note that RH2LH = inverse(RH2LH), so RH2LH.RH2LH is identity, which property is used below)
      const Matrix3D view = RH2LH * m_camView * YAxis;
      const Matrix3D proj = YAxis * m_camProj;
      m_renderer->GetMVP().SetView(view);
      m_renderer->GetMVP().SetProj(0, proj);
      m_renderer->GetMVP().SetProj(1, proj);
   }
   else
   {
      m_renderer->InitLayout();
      m_camView = RH2LH * m_renderer->GetMVP().GetView() * YAxis;
      m_camProj = YAxis * m_renderer->GetMVP().GetProj(0);
   }
}

bool LiveUI::GetSelectionTransform(Matrix3D& transform)
{
   if (m_selection.type == LiveUI::Selection::SelectionType::S_EDITABLE && m_selection.editable->GetItemType() == eItemBall)
   {
      Ball *const ball = static_cast<Ball *>(m_selection.editable);
      transform = Matrix3D::MatrixTranslate(ball->m_hitBall.m_d.m_pos);
      return true;
   }

   if (m_selection.type == LiveUI::Selection::SelectionType::S_EDITABLE && m_selection.editable->GetItemType() == eItemBumper)
   {
      Bumper *const p = static_cast<Bumper *>(m_selection.editable);
      const float height = (m_selection.is_live ? m_live_table : m_table)->GetSurfaceHeight(p->m_d.m_szSurface, p->m_d.m_vCenter.x, p->m_d.m_vCenter.y);
      transform = Matrix3D::MatrixTranslate(p->m_d.m_vCenter.x, p->m_d.m_vCenter.y, height);
      return true;
   }

   if (m_selection.type == LiveUI::Selection::SelectionType::S_EDITABLE && m_selection.editable->GetItemType() == eItemFlasher)
   {
      Flasher *const p = static_cast<Flasher *>(m_selection.editable);
      const Matrix3D trans = Matrix3D::MatrixTranslate(p->m_d.m_vCenter.x, p->m_d.m_vCenter.y, p->m_d.m_height);
      const Matrix3D rotx = Matrix3D::MatrixRotateX(ANGTORAD(p->m_d.m_rotX));
      const Matrix3D roty = Matrix3D::MatrixRotateY(ANGTORAD(p->m_d.m_rotY));
      const Matrix3D rotz = Matrix3D::MatrixRotateZ(ANGTORAD(p->m_d.m_rotZ));
      transform = rotz * roty * rotx * trans;
      return true;
   }

   if (m_selection.type == LiveUI::Selection::SelectionType::S_EDITABLE && m_selection.editable->GetItemType() == eItemFlipper)
   {
      Flipper *const f = static_cast<Flipper *>(m_selection.editable);
      const float height = (m_selection.is_live ? m_live_table : m_table)->GetSurfaceHeight(f->m_d.m_szSurface, f->m_d.m_Center.x, f->m_d.m_Center.y);
      transform = Matrix3D::MatrixTranslate(f->m_d.m_Center.x, f->m_d.m_Center.y, height);
      return true;
   }

   if (m_selection.type == LiveUI::Selection::SelectionType::S_EDITABLE && m_selection.editable->GetItemType() == eItemLight)
   {
      Light *const l = static_cast<Light *>(m_selection.editable);
      const float height = (m_selection.is_live ? m_live_table : m_table)->GetSurfaceHeight(l->m_d.m_szSurface, l->m_d.m_vCenter.x, l->m_d.m_vCenter.y);
      transform = Matrix3D::MatrixTranslate(l->m_d.m_vCenter.x, l->m_d.m_vCenter.y, height + l->m_d.m_height);
      return true;
   }

   if (m_selection.type == LiveUI::Selection::SelectionType::S_EDITABLE && m_selection.editable->GetItemType() == eItemPrimitive)
   {
      Primitive *const p = static_cast<Primitive *>(m_selection.editable);
      const Matrix3D Smatrix = Matrix3D::MatrixScale(p->m_d.m_vSize.x, p->m_d.m_vSize.y, p->m_d.m_vSize.z);
      const Matrix3D Tmatrix = Matrix3D::MatrixTranslate(p->m_d.m_vPosition.x, p->m_d.m_vPosition.y, p->m_d.m_vPosition.z);
      const Matrix3D Rmatrix = (Matrix3D::MatrixRotateZ(ANGTORAD(p->m_d.m_aRotAndTra[2]))
                              * Matrix3D::MatrixRotateY(ANGTORAD(p->m_d.m_aRotAndTra[1])))
                              * Matrix3D::MatrixRotateX(ANGTORAD(p->m_d.m_aRotAndTra[0]));
      transform = (Smatrix * Rmatrix) * Tmatrix; // fullMatrix = Scale * Rotate * Translate
      return true;
   }

   if (m_selection.type == LiveUI::Selection::SelectionType::S_EDITABLE && m_selection.editable->GetItemType() == eItemSurface)
   {
      Surface *const obj = static_cast<Surface *>(m_selection.editable);
      Vertex2D center = obj->GetPointCenter();
      transform = Matrix3D::MatrixTranslate(center.x, center.y, 0.5f * (obj->m_d.m_heightbottom + obj->m_d.m_heighttop));
      return true;
   }

   return false;
}

void LiveUI::SetSelectionTransform(const Matrix3D &newTransform, bool clearPosition, bool clearScale, bool clearRotation)
{
   Matrix3D transform = newTransform;
   const Vertex3Ds right(transform._11, transform._12, transform._13);
   const Vertex3Ds up(transform._21, transform._22, transform._23);
   const Vertex3Ds dir(transform._31, transform._32, transform._33);
   float xscale = right.Length();
   float yscale = up.Length();
   float zscale = dir.Length();
   transform._11 /= xscale; // Normalize transform to evaluate rotation
   transform._12 /= xscale;
   transform._13 /= xscale;
   transform._21 /= yscale;
   transform._22 /= yscale;
   transform._23 /= yscale;
   transform._31 /= zscale;
   transform._32 /= zscale;
   transform._33 /= zscale;
   if (clearScale)
      xscale = yscale = zscale = 1.f;

   float posX = transform._41;
   float posY = transform._42;
   float posZ = transform._43;
   if (clearPosition)
      posX = posY = posZ = 0.f;

   // Derived from https://learnopencv.com/rotation-matrix-to-euler-angles/
   float rotX, rotY, rotZ;
   const float sy = sqrtf(transform._11 * transform._11 + transform._21 * transform._21);
   if (sy > 1e-6f)
   {
      rotX = -RADTOANG(atan2f(transform._32, transform._33));
      rotY = -RADTOANG(atan2f(-transform._31, sy));
      rotZ = -RADTOANG(atan2f(transform._21, transform._11));
   }
   else
   {
      rotX = -RADTOANG(atan2f(transform._23, transform._22));
      rotY = -RADTOANG(atan2f(-transform._22, sy));
      rotZ = 0.f;
   }
   if (clearRotation)
      rotX = rotY = rotZ = 0.f;

   if (m_selection.type == LiveUI::Selection::SelectionType::S_EDITABLE && m_selection.editable->GetItemType() == eItemBall)
   {
      Ball *const ball = static_cast<Ball *>(m_selection.editable);
      ball->m_hitBall.m_d.m_pos.x = posX;
      ball->m_hitBall.m_d.m_pos.y = posY;
      ball->m_hitBall.m_d.m_pos.z = posZ;
   }

   if (m_selection.type == LiveUI::Selection::SelectionType::S_EDITABLE && m_selection.editable->GetItemType() == eItemBumper)
   {
      Bumper *const f = static_cast<Bumper *>(m_selection.editable);
      const float px = f->m_d.m_vCenter.x, py = f->m_d.m_vCenter.y;
      f->Translate(Vertex2D(posX - px, posY - py));
   }

   if (m_selection.type == LiveUI::Selection::SelectionType::S_EDITABLE && m_selection.editable->GetItemType() == eItemFlasher)
   {
      Flasher *const p = static_cast<Flasher *>(m_selection.editable);
      const float px = p->m_d.m_vCenter.x, py = p->m_d.m_vCenter.y;
      p->TranslatePoints(Vertex2D(posX - px, posY - py));
      p->put_Height(posZ);
      p->put_RotX(rotX);
      p->put_RotY(rotY);
      p->put_RotZ(rotZ);
   }

   if (m_selection.type == LiveUI::Selection::SelectionType::S_EDITABLE && m_selection.editable->GetItemType() == eItemFlipper)
   {
      Flipper *const f = static_cast<Flipper *>(m_selection.editable);
      const float px = f->m_d.m_Center.x, py = f->m_d.m_Center.y;
      f->Translate(Vertex2D(posX - px, posY - py));
   }

   if (m_selection.type == LiveUI::Selection::SelectionType::S_EDITABLE && m_selection.editable->GetItemType() == eItemLight)
   {
      Light *const l = static_cast<Light *>(m_selection.editable);
      const float height = (m_selection.is_live ? m_live_table : m_table)->GetSurfaceHeight(l->m_d.m_szSurface, l->m_d.m_vCenter.x, l->m_d.m_vCenter.y);
      const float px = l->m_d.m_vCenter.x, py = l->m_d.m_vCenter.y, pz = height + l->m_d.m_height;
      l->Translate(Vertex2D(posX - px, posY - py));
      l->m_d.m_height += posZ - pz;
      l->m_d.m_bulbHaloHeight += posZ - pz;
   }

   if (m_selection.type == LiveUI::Selection::SelectionType::S_EDITABLE && m_selection.editable->GetItemType() == eItemPrimitive)
   {
      Primitive *const p = static_cast<Primitive *>(m_selection.editable);
      p->m_d.m_vPosition.x = posX;
      p->m_d.m_vPosition.y = posY;
      p->m_d.m_vPosition.z = posZ;
      p->m_d.m_aRotAndTra[0] = rotX;
      p->m_d.m_aRotAndTra[1] = rotY;
      p->m_d.m_aRotAndTra[2] = rotZ;
      p->m_d.m_vSize.x = xscale;
      p->m_d.m_vSize.y = yscale;
      p->m_d.m_vSize.z = zscale;
   }

   if (m_selection.type == LiveUI::Selection::SelectionType::S_EDITABLE && m_selection.editable->GetItemType() == eItemSurface)
   {
      Surface *const obj = static_cast<Surface *>(m_selection.editable);
      Vertex2D center = obj->GetPointCenter();
      const float px = center.x, py = center.y, pz = 0.5f * (obj->m_d.m_heightbottom + obj->m_d.m_heighttop);
      obj->TranslatePoints(Vertex2D(posX - px, posY - py));
      obj->m_d.m_heightbottom += posZ - pz;
      obj->m_d.m_heighttop += posZ - pz;
   }
}

bool LiveUI::IsOutlinerFiltered(const string& name)
{
   if (m_outlinerFilter.empty())
      return true;
   string name_lcase = name;
   StrToLower(name_lcase);
   string filter_lcase = m_outlinerFilter;
   StrToLower(filter_lcase);
   return name_lcase.find(filter_lcase) != std::string::npos;
}

void LiveUI::UpdateOutlinerUI()
{
   if (m_table && m_table->IsLocked())
      return;
   const ImGuiViewport * const viewport = ImGui::GetMainViewport();
   const float pane_width = 200.f * m_dpi;
   ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + m_menubar_height + m_toolbar_height));
   ImGui::SetNextWindowSize(ImVec2(pane_width, viewport->Size.y - m_menubar_height - m_toolbar_height));
   constexpr ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize
      | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
   ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f * m_dpi, 4.0f * m_dpi));
   ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
   ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
   ImGui::Begin("OUTLINER", nullptr, window_flags);

   ImGui::InputTextWithHint("Filter", "Name part filter", &m_outlinerFilter);

   if (ImGui::BeginTabBar("Startup/Live", ImGuiTabBarFlags_AutoSelectNewTabs | ImGuiTabBarFlags_NoCloseWithMiddleMouseButton))
   {
      for (int tab = 0; tab < 2; tab++)
      {
         const bool is_live = (tab == 1);
         PinTable * const table = is_live ? m_live_table : m_table;
         if (ImGui::BeginTabItem(is_live ? "Live" : "Startup", nullptr, (is_live && m_outlinerSelectLiveTab) ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None))
         {
            if (is_live)
               m_outlinerSelectLiveTab = false;
            if (ImGui::TreeNodeEx("View Setups"))
            {
               if (ImGui::Selectable("Live Editor Camera"))
               {
                  m_selection.type = Selection::SelectionType::S_NONE;
                  m_useEditorCam = true;
               }
               Selection cam0(Selection::SelectionType::S_CAMERA, is_live, 0);
               if (ImGui::Selectable("Desktop", m_selection == cam0))
               {
                  m_selection = cam0;
                  m_useEditorCam = false;
                  table->m_BG_override = BG_DESKTOP;
                  table->UpdateCurrentBGSet();
               }
               Selection cam1(Selection::SelectionType::S_CAMERA, is_live, 1);
               if (ImGui::Selectable("Cabinet", m_selection == cam1))
               {
                  m_selection = cam1;
                  m_useEditorCam = false;
                  table->m_BG_override = BG_FULLSCREEN;
                  table->UpdateCurrentBGSet();
               }
               Selection cam2(Selection::SelectionType::S_CAMERA, is_live, 2);
               if (ImGui::Selectable("Full Single Screen", m_selection == cam2))
               {
                  m_selection = cam2;
                  m_useEditorCam = false;
                  table->m_BG_override = BG_FSS;
                  table->UpdateCurrentBGSet();
               }
               ImGui::TreePop();
            }
            if (ImGui::TreeNode("Materials"))
            {
               const std::function<string(Material *)> map = [](Material *image) -> string { return image->m_szName; };
               for (Material *&material : SortedCaseInsensitive(table->m_materials, map))
               {
                  Selection sel(is_live, material);
                  if (IsOutlinerFiltered(material->m_szName) && ImGui::Selectable(material->m_szName.c_str(), m_selection == sel))
                     m_selection = sel;
               }
               ImGui::TreePop();
            }
            if (ImGui::TreeNode("Images"))
            {
               const std::function<string(Texture *)> map = [](Texture *image) -> string { return image->m_szName; };
               for (Texture *&image : SortedCaseInsensitive(table->m_vimage, map))
               {
                  Selection sel(is_live, image);
                  if (IsOutlinerFiltered(image->m_szName) && ImGui::Selectable(image->m_szName.c_str(), m_selection == sel))
                     m_selection = sel;
               }
               ImGui::TreePop();
            }
            if (ImGui::TreeNode("Render Probes"))
            {
               for (RenderProbe *probe : table->m_vrenderprobe)
               {
                  Selection sel(is_live, probe);
                  if (ImGui::Selectable(probe->GetName().c_str(), m_selection == sel))
                     m_selection = sel;
               }
               ImGui::TreePop();
            }
            if (ImGui::TreeNodeEx("Layers", ImGuiTreeNodeFlags_DefaultOpen))
            {
               if (is_live && ImGui::TreeNode("Live Objects"))
               {
                  for (size_t t = 0; t < table->m_vedit.size(); t++)
                  {
                     ISelect *const psel = table->m_vedit[t]->GetISelect();
                     if (psel != nullptr && psel->m_layerName.empty())
                     {
                        Selection sel(is_live, table->m_vedit[t]);
                        if (IsOutlinerFiltered(table->m_vedit[t]->GetName()) && ImGui::Selectable(table->m_vedit[t]->GetName(), m_selection == sel))
                           m_selection = sel;
                     }
                  }
                  ImGui::TreePop();
               }
               // Very very inefficient...
               robin_hood::unordered_map<std::string, vector<IEditable *>> layers;
               for (size_t t = 0; t < table->m_vedit.size(); t++)
               {
                  ISelect *const psel = table->m_vedit[t]->GetISelect();
                  if (psel != nullptr)
                  {
                     auto iter = layers.find(psel->m_layerName);
                     if (iter != layers.end())
                     {
                        iter->second.push_back(table->m_vedit[t]);
                     }
                     else
                     {
                        vector<IEditable *> list;
                        list.push_back(table->m_vedit[t]);
                        layers[psel->m_layerName] = list;
                     }
                  }
               }
               std::vector<std::string> keys;
               keys.reserve(layers.size());
               for (auto &it : layers)
                  keys.push_back(it.first);
               sort(keys.begin(), keys.end());
               for (auto &it : keys)
               {
                  if (it.empty()) // Skip editables without a layer (like live implicit playfield,...)
                     continue;
                  if (ImGui::TreeNode(it.c_str()))
                  {
                     const std::function<string(IEditable *)> map = [](IEditable *editable) -> string { return editable->GetName(); };
                     for (IEditable *&editable : SortedCaseInsensitive(layers[it], map))
                     {
                        Selection sel(is_live, editable);
                        if (IsOutlinerFiltered(editable->GetName()) && ImGui::Selectable(editable->GetName(), m_selection == sel))
                           m_selection = sel;
                     }
                     ImGui::TreePop();
                  }
               }
               ImGui::TreePop();
            }
            ImGui::EndTabItem();
         }
      }
      ImGui::EndTabBar();
   }
   m_outliner_width = ImGui::GetWindowWidth();
   ImGui::End();
   ImGui::PopStyleVar(3);
}

void LiveUI::UpdatePropertyUI()
{
   if (m_table && m_table->IsLocked())
      return;
   const ImGuiViewport *const viewport = ImGui::GetMainViewport();
   const float pane_width = 250.f * m_dpi;
   ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x + viewport->Size.x - pane_width, viewport->Pos.y + m_menubar_height + m_toolbar_height));
   ImGui::SetNextWindowSize(ImVec2(pane_width, viewport->Size.y - m_menubar_height - m_toolbar_height));
   constexpr ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
      | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
   ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f * m_dpi, 4.0f * m_dpi));
   ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
   ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
   ImGui::Begin("PROPERTIES", nullptr, window_flags);
   ImGui::PushItemWidth(PROP_WIDTH);

   if (m_selection.type == Selection::S_IMAGE)
      ImageProperties();
   else if (ImGui::BeginTabBar("Startup/Live", ImGuiTabBarFlags_NoCloseWithMiddleMouseButton))
   {
      for (int tab = 0; tab < 2; tab++)
      {
         const bool is_live = (tab == 1);
         if (ImGui::BeginTabItem(is_live ? "Live" : "Startup", nullptr, (is_live && m_propertiesSelectLiveTab) ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None))
         {
            if (is_live)
               m_propertiesSelectLiveTab = false;
            ImGui::NewLine();
            switch (m_selection.type)
            {
            case Selection::SelectionType::S_NONE: TableProperties(is_live); break; // Use header tab for live since table is displayed when there si no selection
            case Selection::SelectionType::S_CAMERA: CameraProperties(is_live); break;
            case Selection::SelectionType::S_MATERIAL: MaterialProperties(is_live); break;
            case Selection::SelectionType::S_RENDERPROBE: RenderProbeProperties(is_live); break;
            case Selection::SelectionType::S_EDITABLE:
            {
               const bool is_live_selected = m_selection.is_live;
               IEditable *live_obj = (IEditable *)(is_live_selected ? m_selection.editable : m_live_table->m_startupToLive[m_selection.editable]);
               IEditable *startup_obj = (IEditable *)(is_live_selected ? m_live_table->m_liveToStartup[m_selection.editable] : m_selection.editable);
               assert(live_obj == nullptr || std::find(m_live_table->m_vedit.begin(), m_live_table->m_vedit.end(), live_obj) != m_live_table->m_vedit.end());
               assert(startup_obj == nullptr || std::find(m_table->m_vedit.begin(), m_table->m_vedit.end(), startup_obj) != m_table->m_vedit.end());
               if ((is_live && live_obj == nullptr) || (!is_live && startup_obj == nullptr))
               {
                  HelpTextCentered("No Object"s);
               }
               else
               {
                  HelpEditableHeader(is_live, startup_obj, live_obj);
                  switch (m_selection.editable->GetItemType())
                  {
                  // eItemFlipper, eItemTimer, eItemPlunger, eItemTextbox, eItemDecal, eItemGate, eItemSpinner, eItemTable,
                  // eItemLightCenter, eItemDragPoint, eItemCollection, eItemDispReel, eItemLightSeq, eItemHitTarget,
                  case eItemBall: BallProperties(is_live, (Ball *)startup_obj, (Ball *)live_obj); break;
                  case eItemBumper: BumperProperties(is_live, (Bumper *)startup_obj, (Bumper *)live_obj); break;
                  case eItemFlasher: FlasherProperties(is_live, (Flasher *)startup_obj, (Flasher *)live_obj); break;
                  case eItemKicker: KickerProperties(is_live, (Kicker *)startup_obj, (Kicker *)live_obj); break;
                  case eItemLight: LightProperties(is_live, (Light *)startup_obj, (Light *)live_obj); break;
                  case eItemPrimitive: PrimitiveProperties(is_live, (Primitive *)startup_obj, (Primitive *)live_obj); break;
                  case eItemSurface: SurfaceProperties(is_live, (Surface *)startup_obj, (Surface *)live_obj); break;
                  case eItemRamp: RampProperties(is_live, (Ramp *)startup_obj, (Ramp *)live_obj); break;
                  case eItemRubber: RubberProperties(is_live, (Rubber *)startup_obj, (Rubber *)live_obj); break;
                  case eItemTrigger: TriggerProperties(is_live, (Trigger *)startup_obj, (Trigger *)live_obj); break;
                  default: break;
                  }
               }
               break;
            }
            default: break;
            }
            ImGui::EndTabItem();
         }
      }
      ImGui::EndTabBar();
   }
   ImGui::PopItemWidth();
   m_properties_width = ImGui::GetWindowWidth();
   ImGui::End();
   ImGui::PopStyleVar(3);
}

void LiveUI::UpdateAudioOptionsModal()
{
   bool p_open = true;
   if (ImGui::BeginPopupModal(ID_AUDIO_SETTINGS, &p_open, ImGuiWindowFlags_AlwaysAutoResize))
   {
      bool fsound = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "PlayMusic"s, true);
      if (ImGui::Checkbox("Enable music", &fsound))
      {
         g_pvp->m_settings.SaveValue(Settings::Player, "PlayMusic"s, fsound);
         m_player->m_PlayMusic = fsound;
      }

      int volume = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "MusicVolume"s, 100);
      if (ImGui::SliderInt("Music Volume", &volume, 0, 100))
      {
         g_pvp->m_settings.SaveValue(Settings::Player, "MusicVolume"s, volume);
         m_player->m_MusicVolume = volume;
      }

      fsound = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "PlaySound"s, true);
      if (ImGui::Checkbox("Enable sound", &fsound))
      {
         g_pvp->m_settings.SaveValue(Settings::Player, "PlaySound"s, fsound);
         m_player->m_PlaySound = fsound;
      }

      volume = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "SoundVolume"s, 100);
      if (ImGui::SliderInt("Sound Volume", &volume, 0, 100))
      {
         g_pvp->m_settings.SaveValue(Settings::Player, "SoundVolume"s, volume);
         m_player->m_SoundVolume = volume;
      }

      ImGui::EndPopup();
   }
}

void LiveUI::UpdateVideoOptionsModal()
{
   bool popup_anaglyph_calibration = false;
   bool p_open = true;
   if (ImGui::BeginPopupModal(ID_VIDEO_SETTINGS, &p_open, ImGuiWindowFlags_AlwaysAutoResize))
   {
      if (ImGui::CollapsingHeader("Anti-Aliasing", ImGuiTreeNodeFlags_DefaultOpen))
      {
         const char *postprocessed_aa_items[] = { "Disabled", "Fast FXAA", "Standard FXAA", "Quality FXAA", "Fast NFAA", "Standard DLAA", "Quality SMAA" };
         if (ImGui::Combo("Postprocessed AA", &m_renderer->m_FXAA, postprocessed_aa_items, IM_ARRAYSIZE(postprocessed_aa_items)))
            g_pvp->m_settings.SaveValue(Settings::Player, "FXAA"s, m_renderer->m_FXAA);
         const char *sharpen_items[] = { "Disabled", "CAS", "Bilateral CAS" };
         if (ImGui::Combo("Sharpen", &m_renderer->m_sharpen, sharpen_items, IM_ARRAYSIZE(sharpen_items)))
            g_pvp->m_settings.SaveValue(Settings::Player, "Sharpen"s, m_renderer->m_sharpen);
      }
      
      if (ImGui::CollapsingHeader("Performance & Troubleshooting", ImGuiTreeNodeFlags_DefaultOpen))
      {
         if (ImGui::Checkbox("Force Tonemapping off on HDR display", &m_renderer->m_HDRforceDisableToneMapper))
            g_pvp->m_settings.SaveValue(Settings::Player, "HDRDisableToneMapper"s, m_renderer->m_HDRforceDisableToneMapper);
         if (ImGui::Checkbox("Force Bloom filter off", &m_renderer->m_bloomOff))
            g_pvp->m_settings.SaveValue(Settings::Player, "ForceBloomOff"s, m_renderer->m_bloomOff);
         if (ImGui::Checkbox("Force Motion blur off", &m_renderer->m_motionBlurOff))
            g_pvp->m_settings.SaveValue(Settings::Player, "ForceMotionBlurOff"s, m_renderer->m_motionBlurOff);
      }
      
      if (ImGui::CollapsingHeader("Ball Rendering", ImGuiTreeNodeFlags_DefaultOpen))
      {
         bool antiStretch = m_live_table->m_settings.LoadValueWithDefault(Settings::Player, "BallAntiStretch"s, false);
         if (ImGui::Checkbox("Force round ball", &antiStretch))
         {
            g_pvp->m_settings.SaveValue(Settings::Player, "BallAntiStretch"s, antiStretch);
            for (auto ball : m_player->m_vball)
               m_renderer->ReinitRenderable(ball->m_pBall);
         }
      }

      if (m_renderer->m_stereo3D != STEREO_VR && ImGui::CollapsingHeader("3D Stereo Output", ImGuiTreeNodeFlags_DefaultOpen))
      {
         if (ImGui::Checkbox("Enable stereo rendering", &m_renderer->m_stereo3Denabled))
            g_pvp->m_settings.SaveValue(Settings::Player, "Stereo3DEnabled"s, m_renderer->m_stereo3Denabled);
         if (m_renderer->m_stereo3Denabled)
         {
            m_renderer->UpdateStereoShaderState();
            bool modeChanged = false;
            const char *stereo_output_items[] = { "Disabled", "3D TV", "Anaglyph" };
            int stereo_mode = m_renderer->m_stereo3D == STEREO_OFF ? 0 : Is3DTVStereoMode(m_renderer->m_stereo3D) ? 1 : 2;
            int tv_mode = Is3DTVStereoMode(m_renderer->m_stereo3D) ? (int)m_renderer->m_stereo3D - STEREO_TB : 0;
            int glassesIndex = IsAnaglyphStereoMode(m_renderer->m_stereo3D) ? (m_renderer->m_stereo3D - STEREO_ANAGLYPH_1) : 0;
            if (stereo_mode == 0) // Stereo mode may not be activated if the player was started without it (wen can only change between the stereo modes)
               ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
            if (ImGui::Combo("Stereo Output", &stereo_mode, stereo_output_items, IM_ARRAYSIZE(stereo_output_items)))
               modeChanged = true;
            if (stereo_mode == 0)
               ImGui::PopItemFlag();
            if (stereo_mode != 0) // Stereo settings
            {
               // The renderer does not support switching between fake/real stereo 
               // ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
               // ImGui::Checkbox("Use Fake Stereo", &m_renderer->m_stereo3DfakeStereo);
               // ImGui::PopItemFlag();
               ImGui::Text(m_renderer->m_stereo3DfakeStereo ? "Renderer uses 'fake' stereo from single render" : "Renderer performs real stereo rendering");
               if (m_renderer->m_stereo3DfakeStereo)
               {
                  float stereo3DEyeSep = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "Stereo3DMaxSeparation"s, 0.03f);
                  if (ImGui::InputFloat("Max Separation", &stereo3DEyeSep, 0.001f, 0.01f, "%.3f"))
                     g_pvp->m_settings.SaveValue(Settings::Player, "Stereo3DMaxSeparation"s, (float)stereo3DEyeSep);
                  bool stereo3DY = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "Stereo3DYAxis"s, false);
                  if (ImGui::Checkbox("Use Y axis", &stereo3DY))
                     g_pvp->m_settings.SaveValue(Settings::Player, "Stereo3DYAxis"s, stereo3DY);
               }
               else
               {
                  int stereo3DEyeSep = (int)g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "Stereo3DEyeSeparation"s, 63.0f);
                  if (ImGui::InputInt("Eye Separation (mm)", &stereo3DEyeSep, 1, 5))
                     g_pvp->m_settings.SaveValue(Settings::Player, "Stereo3DEyeSeparation"s, (float)stereo3DEyeSep);
               }
            }
            if (stereo_mode == 1) // 3D TV
            {
               const char *tv_mode_items[] = { "Top / Bottom", "Interlaced", "Flipped Interlaced", "Side by Side" };
               if (ImGui::Combo("TV type", &tv_mode, tv_mode_items, IM_ARRAYSIZE(tv_mode_items)))
                  modeChanged = true;
            }
            else if (stereo_mode == 2) // Anaglyph
            {
               // Global anaglyph settings
               float anaglyphSaturation = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "Stereo3DSaturation"s, 1.f);
               if (ImGui::InputFloat("Saturation", &anaglyphSaturation, 0.01f, 0.1f))
                  g_pvp->m_settings.SaveValue(Settings::Player, "Stereo3DSaturation"s, anaglyphSaturation);
               float anaglyphBrightness = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "Stereo3DBrightness"s, 1.f);
               if (ImGui::InputFloat("Brightness", &anaglyphBrightness, 0.01f, 0.1f))
                  g_pvp->m_settings.SaveValue(Settings::Player, "Stereo3DBrightness"s, anaglyphBrightness);
               float anaglyphLeftEyeContrast = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "Stereo3DLeftContrast"s, 1.f);
               if (ImGui::InputFloat("Left Eye Contrast", &anaglyphLeftEyeContrast, 0.01f, 0.1f))
                  g_pvp->m_settings.SaveValue(Settings::Player, "Stereo3DLeftContrast"s, anaglyphLeftEyeContrast);
               float anaglyphRightEyeContrast = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "Stereo3DRightContrast"s, 1.f);
               if (ImGui::InputFloat("Right Eye Contrast", &anaglyphRightEyeContrast, 0.01f, 0.1f))
                  g_pvp->m_settings.SaveValue(Settings::Player, "Stereo3DRightContrast"s, anaglyphRightEyeContrast);
               float anaglyphDefocus = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "Stereo3DDefocus"s, 0.f);
               if (ImGui::InputFloat("Lesser Eye Defocus", &anaglyphDefocus, 0.01f, 0.1f))
                  g_pvp->m_settings.SaveValue(Settings::Player, "Stereo3DDefocus"s, anaglyphDefocus);

               // Glasses settings
               static const string defaultNames[] = { "Red/Cyan"s, "Green/Magenta"s, "Blue/Amber"s, "Cyan/Red"s, "Magenta/Green"s, "Amber/Blue"s, "Custom 1"s, "Custom 2"s, "Custom 3"s, "Custom 4"s };
               string name[std::size(defaultNames)];
               for (size_t i = 0; i < std::size(defaultNames); i++)
                  if (!g_pvp->m_settings.LoadValue(Settings::Player, "Anaglyph"s.append(std::to_string(i + 1)).append("Name"s), name[i]))
                     name[i] = defaultNames[i];
               const char *glasses_items[] = { name[0].c_str(),name[1].c_str(),name[2].c_str(),name[3].c_str(),name[4].c_str(),name[5].c_str(),name[6].c_str(),name[7].c_str(),name[8].c_str(),name[9].c_str(), };
               if (ImGui::Combo("Glasses", &glassesIndex, glasses_items, IM_ARRAYSIZE(glasses_items)))
                  modeChanged = true;
               const string prefKey = "Anaglyph"s.append(std::to_string(glassesIndex + 1));

               if (ImGui::InputText("Name", &name[glassesIndex]))
                  g_pvp->m_settings.SaveValue(Settings::Player, prefKey + "Name", name[glassesIndex]);
               static const char *filter_items[] = { "None", "Dubois", "Luminance", "Deghost" };
               int anaglyphFilter = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, prefKey + "Filter", 4);
               if (ImGui::Combo("Filter", &anaglyphFilter, filter_items, IM_ARRAYSIZE(filter_items)))
                  g_pvp->m_settings.SaveValue(Settings::Player, prefKey + "Filter", anaglyphFilter);
               float anaglyphDynDesat = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, prefKey + "DynDesat", 1.f);
               if (ImGui::InputFloat("Dyn. Desaturation", &anaglyphDynDesat, 0.01f, 0.1f))
                  g_pvp->m_settings.SaveValue(Settings::Player, prefKey + "DynDesat", anaglyphDynDesat);
               float anaglyphDeghost = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, prefKey + "Deghost", 0.f);
               if (ImGui::InputFloat("Deghost", &anaglyphDeghost, 0.01f, 0.1f))
                  g_pvp->m_settings.SaveValue(Settings::Player, prefKey + "Deghost", anaglyphDeghost);
               bool srgbDisplay = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, prefKey + "sRGB", true);
               if (ImGui::Checkbox("Calibrated sRGB Display", &srgbDisplay))
                  g_pvp->m_settings.SaveValue(Settings::Player, prefKey + "sRGB", srgbDisplay);

               Anaglyph anaglyph;
               anaglyph.LoadSetupFromRegistry(glassesIndex);
               const vec3 eyeL = anaglyph.GetLeftEyeColor(false), eyeR = anaglyph.GetRightEyeColor(false);
               Anaglyph::AnaglyphPair colors = anaglyph.GetColorPair();
               ImGui::ColorButton("LeftFilter", ImVec4(eyeL.x, eyeL.y, eyeL.z, 1.f), ImGuiColorEditFlags_NoAlpha);
               ImGui::SameLine();
               ImGui::ColorButton("RightFilter", ImVec4(eyeR.x, eyeR.y, eyeR.z, 1.f), ImGuiColorEditFlags_NoAlpha);
               ImGui::SameLine();
               ImGui::Text(anaglyph.IsReversedColorPair() ? colors == Anaglyph::RED_CYAN ? "Cyan/Red" : colors == Anaglyph::GREEN_MAGENTA ? "Magenta/Green" : "Amber/Blue"
                                                          : colors == Anaglyph::RED_CYAN ? "Red/Cyan" : colors == Anaglyph::GREEN_MAGENTA ? "Green/Magenta" : "Blue/Amber");
               ImGui::SameLine();
               ImGui::Text("Gamma %3.2f", anaglyph.GetDisplayGamma());
               ImGui::SameLine();
               vec3 anaglyphRatio = anaglyph.GetAnaglyphRatio();
               ImGui::Text(" Ratios R: %2d%% G: %2d%% B: %2d%%", (int)(100.f * anaglyphRatio.x), (int)(100.f * anaglyphRatio.y), (int)(100.f * anaglyphRatio.z));

               if (ImGui::Button("Calibrate Glasses"))
                  popup_anaglyph_calibration = true;
               ImGui::SameLine();
               if (ImGui::Button("Reset to default"))
               {
                  g_pvp->m_settings.DeleteValue(Settings::Player, prefKey + "Name");
                  g_pvp->m_settings.DeleteValue(Settings::Player, prefKey + "LeftRed");
                  g_pvp->m_settings.DeleteValue(Settings::Player, prefKey + "LeftGreen");
                  g_pvp->m_settings.DeleteValue(Settings::Player, prefKey + "LeftBlue");
                  g_pvp->m_settings.DeleteValue(Settings::Player, prefKey + "RightRed");
                  g_pvp->m_settings.DeleteValue(Settings::Player, prefKey + "RightGreen");
                  g_pvp->m_settings.DeleteValue(Settings::Player, prefKey + "RightBlue");
               }
            }
            if (modeChanged)
            {
               StereoMode mode = STEREO_OFF;
               if (stereo_mode == 1)
                  mode = (StereoMode)(STEREO_TB + tv_mode);
               if (stereo_mode == 2)
                  mode = (StereoMode)(STEREO_ANAGLYPH_1 + glassesIndex);
               g_pvp->m_settings.SaveValue(Settings::Player, "Stereo3D"s, (int) mode);
               if (m_renderer->m_stereo3D != STEREO_OFF && mode != STEREO_OFF) // TODO allow live switching stereo on/off
                  m_renderer->m_stereo3D = mode;
            }
         }
      }
      ImGui::EndPopup();
   }
   if (popup_anaglyph_calibration && IsAnaglyphStereoMode(g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "Stereo3D"s, 0)))
      ImGui::OpenPopup(ID_ANAGLYPH_CALIBRATION);
}

void LiveUI::UpdateAnaglyphCalibrationModal()
{
   int glassesIndex = m_renderer->m_stereo3D - STEREO_ANAGLYPH_1;
   if (glassesIndex < 0 || glassesIndex > 9)
      return;
   static float backgroundOpacity = 1.f;
   const ImGuiIO& io = ImGui::GetIO();
   ImGui::SetNextWindowSize(io.DisplaySize);
   ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.f, 0.f, 0.f, backgroundOpacity));
   if (ImGui::BeginPopupModal(ID_ANAGLYPH_CALIBRATION, nullptr, (ImGuiWindowFlags_)((int)ImGuiWindowFlags_NoTitleBar | (int)ImGuiNextWindowDataFlags_HasBgAlpha)))
   {
      m_renderer->UpdateStereoShaderState();
      const string prefKey = "Anaglyph"s.append(std::to_string(glassesIndex + 1));
      static int calibrationStep = -1;
      static float calibrationBrightness = 0.5f;
      static const string fields[] = { "LeftRed"s, "LeftGreen"s, "LeftBlue"s, "RightRed"s, "RightGreen"s, "RightBlue"s, };
      // use the right setting for the selected glasses (corresponding to their name)
      if (calibrationStep == -1)
      {
         calibrationStep = 0; 
         calibrationBrightness = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, prefKey + fields[calibrationStep], 0.f);
      }
      if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_LeftCtrl))
      {
         if (calibrationStep == 0)
         {
            calibrationStep = -1;
            ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
            ImGui::OpenPopup(ID_VIDEO_SETTINGS);
            ImGui::PopStyleColor();
            return;
         }
         else
         {
            calibrationStep--;
            calibrationBrightness = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, prefKey + fields[calibrationStep], 0.f);
         }
      }
      else if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_RightCtrl))
      {
         if (calibrationStep >= 5)
         {
            calibrationStep = -1;
            ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
            ImGui::OpenPopup(ID_VIDEO_SETTINGS);
            ImGui::PopStyleColor();
            return;
         }
         else
         {
            calibrationStep++;
            calibrationBrightness = m_live_table->m_settings.LoadValueWithDefault(Settings::Player, prefKey + fields[calibrationStep], 0.f);
         }
      }
      else if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_LeftShift))
      {
         calibrationBrightness = clamp(calibrationBrightness - 0.01f, 0.f, 1.f);
         g_pvp->m_settings.SaveValue(Settings::Player, prefKey + fields[calibrationStep], calibrationBrightness);
      }
      else if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_RightShift))
      {
         calibrationBrightness = clamp(calibrationBrightness + 0.01f, 0.f, 1.f);
         g_pvp->m_settings.SaveValue(Settings::Player, prefKey + fields[calibrationStep], calibrationBrightness);
      }

      ImGui::PushFont(m_overlayFont);
      const ImVec2 win_size = ImGui::GetWindowSize();
      ImDrawList *draw_list = ImGui::GetWindowDrawList();
      const float s = min(win_size.x, win_size.y) / 5.f, t = 1.f * s;
      /* Initial implementation based on MBD calibration
      draw_list->AddRectFilled(ImVec2(0.5f * win_size.x - t, 0.5f * win_size.y - t), ImVec2(0.5f * win_size.x + t, 0.5f * win_size.y + t),
         ImColor(calibrationBrightness, calibrationBrightness, calibrationBrightness));
      for (int x = 0; x < 2; x++)
      {
         for (int y = 0; y < 2; y++)
         {
            if ((x & 1) != (y & 1))
            {
               ImVec2 pos(0.5f * win_size.x - t + s * x, 0.5f * win_size.y - t + s * y);
               draw_list->AddRectFilled(pos, pos + ImVec2(s, s),ImColor((calibrationStep % 3) == 0 ? 1.f : 0.f, (calibrationStep % 3) == 1 ? 1.f : 0.f, (calibrationStep % 3) == 2 ? 1.f : 0.f));
            }
         }
      }*/
      // Perform calibration using a human face, see https://people.cs.uchicago.edu/~glk/pubs/pdf/Kindlmann-FaceBasedLuminanceMatching-VIS-2002.pdf
      static constexpr int faceLength[] = {9, 4, 7, 5, 5, 4, 4, 5, 4, 4, 4, 5, 4};
      static constexpr ImVec2 face[] = {
         ImVec2( 96.5f, 86.9f), ImVec2( 17.6f,-48.1f), ImVec2(  7.5f, -1.3f), ImVec2( 13.1f, -0.8f), ImVec2( 19.8f,  0.3f), ImVec2(22.5f,  1.6f), ImVec2(-14.2f,51.9f), ImVec2(-25.7f,14.2f), ImVec2(-16.8f,1.1f),
         ImVec2(176.9f, 38.5f), ImVec2( 37.7f, 25.7f), ImVec2( -7.8f, 33.7f), ImVec2(-44.1f, -7.5f),
         ImVec2(120.3f,105.6f), ImVec2(-14.7f, 39.8f), ImVec2( -1.3f, 13.6f), ImVec2( 16.6f,  1.3f), ImVec2( 24.6f,-35.0f), ImVec2( 0.8f,-15.5f), ImVec2( -9.1f,-5.3f),
         ImVec2(120.8f,160.4f), ImVec2( 20.8f, 11.5f), ImVec2( 68.7f,-10.4f), ImVec2(-19.2f,-27.3f), ImVec2(-45.7f, -8.8f),
         ImVec2(116.8f,171.1f), ImVec2( -3.2f, 16.0f), ImVec2(-24.9f, -0.5f), ImVec2( -2.3f,-12.5f), ImVec2(  9.7f, -6.0f),
         ImVec2(116.8f,171.1f), ImVec2( 24.9f,  0.8f), ImVec2( 12.8f, 22.2f), ImVec2(-40.9f, -6.9f),
         ImVec2(141.6f,171.9f), ImVec2( 68.7f,-10.4f), ImVec2(-20.3f, 40.6f), ImVec2(-35.5f, -8.0f),
         ImVec2(154.5f,194.0f), ImVec2( -3.5f,  8.8f), ImVec2(  3.7f, 30.2f), ImVec2( 25.1f,-10.2f), ImVec2( 10.2f,-20.8f),
         ImVec2(151.0f,202.9f), ImVec2(-23.3f, -2.4f), ImVec2(  2.9f, 43.0f), ImVec2( 24.0f,-10.4f),
         ImVec2( 89.0f,194.8f), ImVec2( 38.8f,  5.6f), ImVec2(  2.9f, 43.0f), ImVec2(-38.0f, -5.3f),
         ImVec2(191.1f,134.2f), ImVec2(  5.3f,-19.0f), ImVec2( 17.1f, -1.1f), ImVec2( -3.2f, 47.3f),
         ImVec2( 74.6f,151.9f), ImVec2(  7.9f,-23.8f), ImVec2( 26.5f, -3.0f), ImVec2(-12.9f, 42.9f), ImVec2( -9.7f,  6.0f),
         ImVec2(65.5f, 148.9f), ImVec2(  6.8f,-38.6f), ImVec2( 10.6f, -0.8f), ImVec2( -0.4f, 18.5f),
      };
      ImColor backCol(calibrationBrightness, calibrationBrightness, calibrationBrightness);
      ImColor calCol((calibrationStep % 3) == 0 ? 1.f : 0.f, (calibrationStep % 3) == 1 ? 1.f : 0.f, (calibrationStep % 3) == 2 ? 1.f : 0.f);
      for (int v = 0; v < 2; v++)
      {
         ImVec2 faceTrans[10], faceOffset(win_size.x * 0.5f - 0.5f * t + (float)v * t, win_size.y * 0.5f);
         draw_list->AddRectFilled(ImVec2(0.5f * win_size.x - t + (float)v * t, 0.5f * win_size.y - t), ImVec2(0.5f * win_size.x + (float)v * t, 0.5f * win_size.y + t), v == 0 ? backCol : calCol);
         ImU32 col = ImGui::GetColorU32(v == 1 ? backCol.Value : calCol.Value);
         for (int i = 0, p = 0; i < 13; p += faceLength[i], i++)
         {
            ImVec2 pos(0.f, 0.f);
            for (int j = 0; j < faceLength[i]; j++)
            {
               pos = pos + face[p + j];
               PLOGD << pos.x << ", " << pos.y;
               faceTrans[j] = faceOffset + (pos + ImVec2(-140.f, -140.f)) * 2.f * t / 320.f;
            }
            draw_list->AddConvexPolyFilled(faceTrans, faceLength[i], col);
         }
      }

      float line_height = ImGui::CalcTextSize("A").y * 1.2f;
      #define CENTERED_TEXT(y, t) ImGui::SetCursorPos(ImVec2((win_size.x - ImGui::CalcTextSize(t).x) * 0.5f, y));ImGui::Text("%s", t);
      float y = win_size.y * 0.5f + t + line_height;
      string step_info = "Anaglyph glasses calibration step #"s.append(std::to_string(calibrationStep + 1)).append("/6");
      CENTERED_TEXT(y + 0 * line_height, step_info.c_str());
      step_info = (calibrationStep < 3 ? "Left eye's "s : "Right eye's "s).append((calibrationStep % 3) == 0 ? "red"s : (calibrationStep % 3) == 1 ? "green"s : "blue"s).append(" perceived luminance: "s).append(std::to_string((int)(calibrationBrightness * 100.f))).append(1,'%');
      CENTERED_TEXT(y + 1 * line_height, step_info.c_str());
      CENTERED_TEXT(y + 3 * line_height, calibrationStep < 3 ? "Close your right eye" : "Close your left eye");
      CENTERED_TEXT(y + 5 * line_height, calibrationStep == 0 ? "Use Left Control to exit calibration" : "Use Left Control to move to previous step");
      CENTERED_TEXT(y + 6 * line_height, calibrationStep == 5 ? "Use Right Control to exit calibration" : "Use Right Control to move to next step");
      CENTERED_TEXT(y + 7 * line_height, "Use Left/Right Shift to adjust face brightness until");
      CENTERED_TEXT(y + 8 * line_height, "your eye does not favor or focus one face over the other.");
      ImGui::PopFont();
      line_height = ImGui::CalcTextSize("A").y * 1.2f;
      y = win_size.y * 0.5f - t - 3.f * line_height;
      CENTERED_TEXT(y - line_height, "Background Opacity");
      ImGui::SetCursorPos(ImVec2((win_size.x - 1.5f * t) * 0.5f, y));
      ImGui::SetNextItemWidth(1.5f * t);
      ImGui::SliderFloat("##Background Opacity", &backgroundOpacity, 0.f, 1.f);
      ImGui::EndPopup();
      #undef CENTERED_TEXT
   }
   ImGui::PopStyleColor();
}

void LiveUI::UpdateHeadTrackingModal()
{
#ifndef __STANDALONE__
   BAMView::drawMenu();
#endif
}

void LiveUI::UpdatePlumbWindow()
{
   // TODO table or global settings ?
   if (!m_ShowPlumb)
      return;
   if (ImGui::Begin("Nudge & Plumb Settings", &m_ShowPlumb, ImGuiWindowFlags_AlwaysAutoResize))
   {
      // Physical accelerator settings
      bool accEnabled = m_live_table->m_settings.LoadValueWithDefault(Settings::Player, "PBWEnabled"s, true);
      if (ImGui::Checkbox("Acc. Enabled", &accEnabled))
      {
         g_pvp->m_settings.SaveValue(Settings::Player, "PBWEnabled"s, accEnabled);
         m_player->ReadAccelerometerCalibration();
      }
      ImGui::BeginDisabled(!accEnabled);
      int accMax[] = { m_live_table->m_settings.LoadValueWithDefault(Settings::Player, "PBWAccelMaxX"s, 100), m_live_table->m_settings.LoadValueWithDefault(Settings::Player, "PBWAccelMaxY"s, 100) };
      if (ImGui::InputInt2("Acc. Maximum", accMax))
      {
         g_pvp->m_settings.SaveValue(Settings::Player, "PBWAccelMaxX"s, accMax[0]);
         g_pvp->m_settings.SaveValue(Settings::Player, "PBWAccelMaxY"s, accMax[1]);
         m_player->ReadAccelerometerCalibration();
      }
      int accGain[] = { m_live_table->m_settings.LoadValueWithDefault(Settings::Player, "PBWAccelGainX"s, 150), m_live_table->m_settings.LoadValueWithDefault(Settings::Player, "PBWAccelGainY"s, 150) };
      if (ImGui::InputInt2("Acc. Gain", accGain))
      {
         g_pvp->m_settings.SaveValue(Settings::Player, "PBWAccelGainX"s, accGain[0]);
         g_pvp->m_settings.SaveValue(Settings::Player, "PBWAccelGainY"s, accGain[1]);
         m_player->ReadAccelerometerCalibration();
      }
      int accSensitivity = m_live_table->m_settings.LoadValueWithDefault(Settings::Player, "NudgeSensitivity"s, 500);
      if (ImGui::InputInt("Acc. Sensitivity", &accSensitivity))
      {
         g_pvp->m_settings.SaveValue(Settings::Player, "NudgeSensitivity"s, accSensitivity);
         m_player->ReadAccelerometerCalibration();
      }
      bool accOrientationEnabled = m_live_table->m_settings.LoadValueWithDefault(Settings::Player, "PBWRotationCB"s, false); // TODO Legacy stuff => remove and only keep rotation
      int accOrientation = accOrientationEnabled ? m_live_table->m_settings.LoadValueWithDefault(Settings::Player, "PBWRotationValue"s, 500) : 0;
      if (ImGui::InputInt("Acc. Rotation", &accOrientation))
      {
         g_pvp->m_settings.SaveValue(Settings::Player, "PBWRotationCB"s, accOrientation != 0);
         g_pvp->m_settings.SaveValue(Settings::Player, "PBWRotationValue"s, accOrientation);
         m_player->ReadAccelerometerCalibration();
      }
      bool accFaceUp = m_live_table->m_settings.LoadValueWithDefault(Settings::Player, "PBWNormalMount"s, true);
      if (ImGui::Checkbox("Acc. Face Up", &accFaceUp))
      {
         g_pvp->m_settings.SaveValue(Settings::Player, "PBWNormalMount"s, accFaceUp);
         m_player->ReadAccelerometerCalibration();
      }
      bool accFilter = m_live_table->m_settings.LoadValueWithDefault(Settings::Player, "EnableNudgeFilter"s, false);
      if (ImGui::Checkbox("Acc. Filter", &accFilter))
      {
         g_pvp->m_settings.SaveValue(Settings::Player, "EnableNudgeFilter"s, accFilter);
         m_player->ReadAccelerometerCalibration();
      }
      ImGui::EndDisabled();

      ImGui::Separator();

      // Tilt plumb settings
      bool enablePlumbTilt = m_live_table->m_settings.LoadValueWithDefault(Settings::Player, "TiltSensCB"s, false);
      if (ImGui::Checkbox("Enable virtual Tilt plumb", &enablePlumbTilt))
      {
         g_pvp->m_settings.SaveValue(Settings::Player, "TiltSensCB"s, enablePlumbTilt);
         m_player->m_physics->ReadNudgeSettings(m_live_table->m_settings);
      }
      ImGui::BeginDisabled(!enablePlumbTilt);
      float plumbTiltThreshold = m_live_table->m_settings.LoadValueWithDefault(Settings::Player, "TiltSensitivity"s, 400) * 45.f / 1000.f;
      if (ImGui::InputFloat("Tilt threshold angle", &plumbTiltThreshold, 0.1f, 1.0f, "%.1f"))
      {
         g_pvp->m_settings.SaveValue(Settings::Player, "TiltSensitivity"s, static_cast<int>(round(plumbTiltThreshold * 1000.f / 45.f)));
         m_player->m_physics->ReadNudgeSettings(m_live_table->m_settings);
      }
      float plumbTiltMass = m_live_table->m_settings.LoadValueWithDefault(Settings::Player, "TiltInertia"s, 100.f);
      if (ImGui::InputFloat("Tilt inertia factor", &plumbTiltMass, 1.f, 10.f, "%.1f"))
      {
         g_pvp->m_settings.SaveValue(Settings::Player, "TiltInertia"s, plumbTiltMass);
         m_player->m_physics->ReadNudgeSettings(m_live_table->m_settings);
      }
      ImGui::EndDisabled();

      ImGui::Separator();

      ImGui::Text("Nudge & Plumb State");
      constexpr int panelSize = 100;
      if (ImGui::BeginTable("PlumbInfo", 2, ImGuiTableFlags_Borders))
      {
         ImGui::TableSetupColumn("Col1", ImGuiTableColumnFlags_WidthFixed, panelSize * m_dpi);
         ImGui::TableSetupColumn("Col2", ImGuiTableColumnFlags_WidthFixed, panelSize * m_dpi);
      
         ImGui::TableNextColumn();
         ImGui::Text("Cab. Sensor");
         ImGui::TableNextColumn();
         ImGui::Text("Plumb Position");
         ImGui::TableNextRow();

         const ImVec2 fullSize = ImVec2(panelSize * m_dpi, panelSize * m_dpi);
         const ImVec2 halfSize = fullSize * 0.5f;
         ImGui::TableNextColumn();
         {
            ImGui::BeginChild("Sensor", fullSize);
            const ImVec2 &pos = ImGui::GetWindowPos();
            ImGui::GetWindowDrawList()->AddLine(pos + ImVec2(0.f, halfSize.y), pos + ImVec2(fullSize.x, halfSize.y), IM_COL32_WHITE);
            ImGui::GetWindowDrawList()->AddLine(pos + ImVec2(halfSize.x, 0.f), pos + ImVec2(halfSize.y, fullSize.y), IM_COL32_WHITE);
            const Vertex2D &acc = m_player->GetRawAccelerometer(); // Range: -1..1
            ImVec2 accPos = pos + halfSize + ImVec2(acc.x, acc.y) * halfSize * 2.f + ImVec2(0.5f, 0.5f);
            ImGui::GetWindowDrawList()->AddCircleFilled(accPos, 5.f * m_dpi, IM_COL32(255, 0, 0, 255));
            ImGui::EndChild();
         }
         ImGui::TableNextColumn();
         {
            ImGui::BeginDisabled(!enablePlumbTilt);
            ImGui::BeginChild("PlumbPos", fullSize);
            const ImVec2 &pos = ImGui::GetWindowPos();
            ImGui::GetWindowDrawList()->AddLine(pos + ImVec2(0.f, halfSize.y), pos + ImVec2(fullSize.x, halfSize.y), IM_COL32_WHITE);
            ImGui::GetWindowDrawList()->AddLine(pos + ImVec2(halfSize.x, 0.f), pos + ImVec2(halfSize.y, fullSize.y), IM_COL32_WHITE);
            // Tilt circle
            const ImVec2 scale = halfSize * 1.5f;
            const ImVec2 radius = scale * sin(m_player->m_physics->GetPlumbTiltThreshold() * (float)(M_PI * 0.25));
            ImGui::GetWindowDrawList()->AddEllipse(pos + halfSize, radius, IM_COL32(255, 0, 0, 255));
            // Plumb position
            const Vertex3Ds &plumb = m_player->m_physics->GetPlumbPos();
            const ImVec2 plumbPos = pos + halfSize + scale * ImVec2(plumb.x, plumb.y) / m_player->m_physics->GetPlumbPoleLength() + ImVec2(0.5f, 0.5f);
            ImGui::GetWindowDrawList()->AddLine(pos + halfSize, plumbPos, IM_COL32(255, 128, 0, 255));
            ImGui::GetWindowDrawList()->AddCircleFilled(plumbPos, 5.f * m_dpi, IM_COL32(255, 0, 0, 255));
            ImGui::EndChild();
            ImGui::EndDisabled();
         }
         ImGui::TableNextRow();

         ImGui::TableNextColumn();
         ImGui::Text("Table Acceleration");
         ImGui::TableNextColumn();
         ImGui::Text("Plumb Angle");
         ImGui::TableNextRow();

         ImGui::TableNextColumn();
         {
            ImGui::BeginChild("Table Acceleration", fullSize);
            const ImVec2 &pos = ImGui::GetWindowPos();
            ImGui::GetWindowDrawList()->AddLine(pos + ImVec2(0.f, halfSize.y), pos + ImVec2(fullSize.x, halfSize.y), IM_COL32_WHITE);
            ImGui::GetWindowDrawList()->AddLine(pos + ImVec2(halfSize.x, 0.f), pos + ImVec2(halfSize.y, fullSize.y), IM_COL32_WHITE);
            const Vertex3Ds &nudge = (float)PHYS_FACTOR * m_player->m_physics->GetNudgeAcceleration(); // Range: -1..1
            ImVec2 nudgePos = pos + halfSize + ImVec2(nudge.x, nudge.y) * halfSize * 2.f + ImVec2(0.5f, 0.5f);
            ImGui::GetWindowDrawList()->AddCircleFilled(nudgePos, 5.f * m_dpi, IM_COL32(255, 0, 0, 255));
            ImGui::EndChild();
         }
         ImGui::TableNextColumn();
         {
            ImGui::BeginDisabled(!enablePlumbTilt);
            ImGui::BeginChild("PlumbAngle", ImVec2(panelSize * m_dpi, panelSize * m_dpi));
            const ImVec2 &pos = ImGui::GetWindowPos();
            const Vertex3Ds& plumb = m_player->m_physics->GetPlumbPos();
            float radius = min(fullSize.x, fullSize.y) * 0.9f;
            // Tilt limits
            float angle = m_player->m_physics->GetPlumbTiltThreshold() * (float)(M_PI * 0.25);
            ImVec2 plumbPos = pos + ImVec2(halfSize.x + sinf(angle) * radius, cosf(angle) * radius);
            ImGui::GetWindowDrawList()->AddLine(pos + ImVec2(halfSize.x, 0.f), plumbPos, IM_COL32(255, 0, 0, 255));
            plumbPos = pos + ImVec2(halfSize.x - sin(angle) * radius, cos(angle) * radius);
            ImGui::GetWindowDrawList()->AddLine(pos + ImVec2(halfSize.x, 0.f), plumbPos, IM_COL32(255, 0, 0, 255));
            // Plumb position
            angle = atan2f(sqrt(plumb.x * plumb.x + plumb.y * plumb.y), -plumb.z);
            const float theta = atan2(plumb.x, plumb.y);
            if (theta + (float)(M_PI/2.) < 0.f || theta + (float)(M_PI/2.) >= (float)M_PI)
               angle = -angle;
            plumbPos = pos + ImVec2(halfSize.x + sinf(angle) * radius, cosf(angle) * radius);
            ImGui::GetWindowDrawList()->AddLine(pos + ImVec2(halfSize.x, 0.f), plumbPos, IM_COL32(255, 128, 0, 255));
            ImGui::GetWindowDrawList()->AddCircleFilled(plumbPos, 5.f * m_dpi, IM_COL32(255, 0, 0, 255));
            ImGui::EndChild();
            ImGui::EndDisabled();
         }
         ImGui::EndTable();
      }
   }
   ImGui::End();
}

void LiveUI::UpdateRendererInspectionModal()
{
   if (m_staticPrepassDisabled)
   {
      m_staticPrepassDisabled = false;
      m_renderer->DisableStaticPrePass(false);
   }
   m_useEditorCam = false;
   m_renderer->InitLayout();

   ImGui::SetNextWindowSize(ImVec2(350.f * m_dpi, 0));
   if (ImGui::Begin(ID_RENDERER_INSPECTION, &m_RendererInspection))
   {
      ImGui::Text("Display single render pass:");
      static int pass_selection = IF_FPS;
      ImGui::RadioButton("Disabled", &pass_selection, IF_FPS);
      #if defined(ENABLE_DX9) // No GPU profiler for OpenGL or BGFX for the time being
      ImGui::RadioButton("Profiler", &pass_selection, IF_PROFILING);
      #endif
      ImGui::RadioButton("Static prerender pass", &pass_selection, IF_STATIC_ONLY);
      ImGui::RadioButton("Dynamic render pass", &pass_selection, IF_DYNAMIC_ONLY);
      ImGui::RadioButton("Transmitted light pass", &pass_selection, IF_LIGHT_BUFFER_ONLY);
      if (m_player->m_renderer->GetAOMode() != 0)
         ImGui::RadioButton("Ambient Occlusion pass", &pass_selection, IF_AO_ONLY);
      for (size_t i = 0; i < m_table->m_vrenderprobe.size(); i++)
      {
         ImGui::RadioButton(m_table->m_vrenderprobe[i]->GetName().c_str(), &pass_selection, 100 + (int)i);
      }
      if (pass_selection < 100)
         m_player->m_infoMode = (InfoMode)pass_selection;
      else
      {
         m_player->m_infoMode = IF_RENDER_PROBES;
         m_player->m_infoProbeIndex = pass_selection - 100;
      }
      ImGui::NewLine();

      // Latency timing table
      if (ImGui::BeginTable("Latencies", 4, ImGuiTableFlags_Borders))
      {
         const U32 period = m_player->m_logicProfiler.GetPrev(FrameProfiler::PROFILE_FRAME);
         ImGui::TableSetupColumn("##Cat", ImGuiTableColumnFlags_WidthFixed);
         ImGui::TableSetupColumn("Min", ImGuiTableColumnFlags_WidthFixed);
         ImGui::TableSetupColumn("Max", ImGuiTableColumnFlags_WidthFixed);
         ImGui::TableSetupColumn("Avg", ImGuiTableColumnFlags_WidthFixed);
         ImGui::TableHeadersRow();
         #define PROF_ROW(name, section) \
         ImGui::TableNextColumn(); ImGui::Text("%s", name); \
         ImGui::TableNextColumn(); ImGui::Text("%4.1fms", m_player->m_logicProfiler.GetSlidingMin(section) * 1e-3); \
         ImGui::TableNextColumn(); ImGui::Text("%4.1fms", m_player->m_logicProfiler.GetSlidingMax(section) * 1e-3); \
         ImGui::TableNextColumn(); ImGui::Text("%4.1fms", m_player->m_logicProfiler.GetSlidingAvg(section) * 1e-3);
         PROF_ROW("Input to Script lag", FrameProfiler::PROFILE_INPUT_POLL_PERIOD)
         PROF_ROW("Input to Present lag", FrameProfiler::PROFILE_INPUT_TO_PRESENT)
         #undef PROF_ROW
         ImGui::EndTable();
         ImGui::NewLine();
      }

      ImGui::Text("Press F11 to reset min/max/average timings");
      if (ImGui::IsKeyPressed(dikToImGuiKeys[m_player->m_rgKeys[eFrameCount]]))
         m_player->InitFPS();

      // Other detailed information
      ImGui::Text("%s", m_player->GetPerfInfo().c_str());
   }
   ImGui::End();
}

void LiveUI::UpdateMainSplashModal()
{
   const bool enableKeyboardShortcuts = (msec() - m_OpenUITime) > 250;

   ImGuiStyle &style = ImGui::GetStyle();
   // FIXME push style
   style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0, 0, 0, 0);

   // Display table name,author,version and blurb and description
   {
      std::ostringstream info;

      if (m_ShowUI) // Move below menu & toolbar
         info << "\n\n\n\n";

      if (!m_table->m_szTableName.empty())
         info << "# " << m_table->m_szTableName << '\n';
      else
         info << "# Table\n";

      const size_t line_length = info.str().size();
      if (!m_table->m_szBlurb.empty())
         info << m_table->m_szBlurb << std::string(line_length, '=') << '\n';
      if (!m_table->m_szDescription.empty())
         info << m_table->m_szDescription;

      info << "\n\n  ";
      if (!m_table->m_szAuthor.empty())
         info << "By " << m_table->m_szAuthor << ", ";
      if (!m_table->m_szVersion.empty())
         info << "Version: " << m_table->m_szVersion;
      info << " (" << (!m_table->m_szDateSaved.empty() ? m_table->m_szDateSaved : "N.A.") << " Revision " << m_table->m_numTimesSaved << ")\n";

      constexpr ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
      ImGui::SetNextWindowBgAlpha(0.5f);
      ImGui::SetNextWindowPos(ImVec2(0, 0));
      ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
      ImGui::Begin("Table Info", nullptr, window_flags);
      markdown_start_id = ImGui::GetItemID();
      ImGui::Markdown(info.str().c_str(), info.str().length(), markdown_config);
      ImGui::End();
   }

   constexpr ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove;
   if (ImGui::BeginPopupModal(ID_MODAL_SPLASH, nullptr, window_flags))
   {
#ifndef __STANDALONE__
      const ImVec2 size(m_dpi * (m_player->m_headTracking ? 120.f : 100.f), 0);
#else
      const ImVec2 size(m_dpi * 170.f, 0);
#endif

      // If displaying the main splash popup, save user changes and exit camera mode started from it
      if (m_tweakMode && m_live_table != nullptr && m_table != nullptr)
         CloseTweakMode();

      // Key shortcut: click on the button, or press escape key (react on key released, otherwise, it would immediately reopen the UI,...)
      int keyShortcut = 0;
      if (enableKeyboardShortcuts && (ImGui::IsKeyReleased(ImGuiKey_Escape) || ((ImGui::IsKeyReleased(dikToImGuiKeys[m_player->m_rgKeys[eEscape]]) && !m_disable_esc))))
         keyShortcut = m_esc_mode == 0 ? 1 : m_esc_mode;

      ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
      if (ImGui::Button("Resume Game", size) || (keyShortcut == 1))
      {
         ImGui::CloseCurrentPopup();
         SetupImGuiStyle(1.0f);
         m_useEditorCam = false;
         m_renderer->InitLayout();
         HideUI();
      }
      ImGui::SetItemDefaultFocus();
      if (ImGui::Button("Table Options", size) || (keyShortcut == 2))
      {
         ImGui::CloseCurrentPopup();
         OpenTweakMode();
      }
      bool popup_headtracking = false;
#if !((defined(__APPLE__) && ((defined(TARGET_OS_IOS) && TARGET_OS_IOS) || (defined(TARGET_OS_TV) && TARGET_OS_TV))) || defined(__ANDROID__))
      if (m_player->m_headTracking && ImGui::Button("Adjust Headtracking", size))
      {
         ImGui::CloseCurrentPopup();
         m_ShowUI = false;
         m_ShowSplashModal = false;
         popup_headtracking = true;
      }
      if (m_renderer->m_stereo3D != STEREO_VR && (ImGui::Button("Live Editor", size) || (keyShortcut == 3)))
      {
         ImGui::CloseCurrentPopup();
         m_ShowUI = true;
         m_ShowSplashModal = false;
         if (!m_staticPrepassDisabled)
         {
            m_staticPrepassDisabled = true;
            m_renderer->DisableStaticPrePass(true);
         }
         ResetCameraFromPlayer();
      }
#endif
      if (g_pvp->m_ptableActive->TournamentModePossible() && ImGui::Button("Generate Tournament File", size))
      {
         g_pvp->GenerateTournamentFile();
      }
#ifndef __STANDALONE__
      // Quit: click on the button, or press exit button
      if (ImGui::Button("Quit to Editor", size) || (enableKeyboardShortcuts && ImGui::IsKeyPressed(dikToImGuiKeys[m_player->m_rgKeys[eExitGame]])))
      {
         ImGui::CloseCurrentPopup();
         HideUI();
         m_table->QuitPlayer(Player::CS_STOP_PLAY);
      }
#else
#if ((defined(__APPLE__) && (defined(TARGET_OS_IOS) && TARGET_OS_IOS)) || defined(__ANDROID__))
      bool showTouchOverlay = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "TouchOverlay"s, false);
      if (ImGui::Button(showTouchOverlay ? "Disable Touch Overlay" : "Enable Touch Overlay", size))
      {
         showTouchOverlay = !showTouchOverlay;
         g_pvp->m_settings.SaveValue(Settings::Player, "TouchOverlay"s, showTouchOverlay);

         ImGui::GetIO().MousePos.x = 0;
         ImGui::GetIO().MousePos.y = 0;
      }
#endif
#if (defined(__APPLE__) && ((defined(TARGET_OS_IOS) && TARGET_OS_IOS) || (defined(TARGET_OS_TV) && TARGET_OS_TV))) || defined(__ANDROID__)
      if (ImGui::Button(m_show_fps > 0 ? "Disable FPS" : "Enable FPS", size))
      {
         m_show_fps = (m_show_fps > 0) ? 0 : 1;

         if (m_show_fps)
            m_player->InitFPS();

         ImGui::GetIO().MousePos.x = 0;
         ImGui::GetIO().MousePos.y = 0;
      }
#endif
#ifndef __LIBVPINBALL__
      bool webServerRunning = g_pvp->m_webServer.IsRunning();
      if (ImGui::Button(webServerRunning ? "Disable Web Server" : "Enable Web Server", size))
      {
         g_pvp->m_settings.SaveValue(Settings::Standalone, "WebServer"s, !webServerRunning);

         if (webServerRunning)
            g_pvp->m_webServer.Stop();
         else
            g_pvp->m_webServer.Start();

#if ((defined(__APPLE__) && (defined(TARGET_OS_IOS) && TARGET_OS_IOS)) || defined(__ANDROID__))
         ImGui::GetIO().MousePos.x = 0;
         ImGui::GetIO().MousePos.y = 0;
#endif
      }
#endif

      if (ImGui::Button("Quit", size) || (enableKeyboardShortcuts && ImGui::IsKeyPressed(dikToImGuiKeys[m_player->m_rgKeys[eExitGame]])))
      {
         ImGui::CloseCurrentPopup();
         HideUI();
         m_table->QuitPlayer(Player::CS_CLOSE_APP);
      }

#if (defined(__APPLE__) && ((defined(TARGET_OS_IOS) && TARGET_OS_IOS && !defined(__LIBVPINBALL__)) || (defined(TARGET_OS_TV) && TARGET_OS_TV))) || defined(__ANDROID__)
      ImGui::Dummy(ImVec2(0.f, m_dpi * 4.f));
      ImGui::Separator();
      ImGui::Dummy(ImVec2(0.f, m_dpi * 4.f));
      ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ((ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Launch Table:").x) * 0.5f));
      ImGui::Text("Launch Table:");

      ImGui::PushItemWidth(size.x);

      const string launchTable = g_pvp->m_settings.LoadValueWithDefault(Settings::Standalone, "LaunchTable"s, "assets/exampleTable.vpx"s);
      if (ImGui::BeginCombo("##Launch Table", launchTable.c_str()))
      {
         vector<string> files = find_files_by_extension(g_pvp->m_szMyPrefPath, "vpx"s);

         for (const auto& file : files) {
            if (ImGui::Selectable(file.c_str()))
               g_pvp->m_settings.SaveValue(Settings::Standalone, "LaunchTable"s, file);
         }

         ImGui::EndCombo();
      }

      ImGui::PopItemWidth();
#endif

#ifndef __LIBVPINBALL__
      string url = g_pvp->m_webServer.GetUrl();
      if (!url.empty())
      {
         ImGui::Dummy(ImVec2(0.f, m_dpi * 4.f));
         ImGui::Separator();
         ImGui::Dummy(ImVec2(0.f, m_dpi * 4.f));
         ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ((ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(url.c_str()).x) * 0.5f));
         ImGui::Text("%s", url.c_str());
      }
#endif

#endif
      const ImVec2 pos = ImGui::GetWindowPos();
      ImVec2 max = ImGui::GetWindowSize();
      const bool hovered = ImGui::IsWindowHovered();
      ImGui::EndPopup();

      if (popup_headtracking)
         ImGui::OpenPopup(ID_BAM_SETTINGS);

      // Handle dragging mouse to allow dragging the main application window
      if (m_player)
      {
         static ImVec2 initialDragPos;
         if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
         {
            max.x += pos.x;
            max.y += pos.y;
            if (!hovered && !(pos.x <= initialDragPos.x && initialDragPos.x <= max.x && pos.y <= initialDragPos.y && initialDragPos.y <= max.y) // Don't drag if mouse is over UI components
             && !m_player->m_playfieldWnd->IsFullScreen()) // Don't drag if window is fullscreen
            {
               //const ImVec2 pos = ImGui::GetMousePos();
               const ImVec2 drag = ImGui::GetMouseDragDelta();
               int x, y;
               m_player->m_playfieldWnd->GetPos(x, y);
               m_player->m_playfieldWnd->SetPos(x + (int)drag.x, y + (int)drag.y);
            }
         }
         else
         {
            initialDragPos = ImGui::GetMousePos();
         }
      }
   }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Property panes
//

void LiveUI::TableProperties(bool is_live)
{
   HelpEditableHeader(is_live, m_table, m_live_table);
   if (ImGui::CollapsingHeader("User", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      ImGui::EndTable();
   }
   if (ImGui::CollapsingHeader("Visuals", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      ImGui::EndTable();
   }
   if (ImGui::CollapsingHeader("Physics", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      ImGui::EndTable();
   }
   if (ImGui::CollapsingHeader("Lighting", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      auto reinit_lights = [this](bool is_live, float prev, float v) { m_renderer->MarkShaderDirty(); }; // Needed to update shaders with new light settings 
      PropRGB("Ambient Color", m_table, is_live, &(m_table->m_lightAmbient), m_live_table ? &(m_live_table->m_lightAmbient) : nullptr);
      
      PropSeparator();
      PropRGB("Light Em. Color", m_table, is_live, &(m_table->m_Light[0].emission), m_live_table ? &(m_live_table->m_Light[0].emission) : nullptr);
      PropFloat("Light Em. Scale", m_table, is_live, &(m_table->m_lightEmissionScale), m_live_table ? &(m_live_table->m_lightEmissionScale) : nullptr, 20000.0f, 100000.0f, "%.0f", ImGuiInputTextFlags_CharsDecimal, reinit_lights);
      PropFloat("Light Height", m_table, is_live, &(m_table->m_lightHeight), m_live_table ? &(m_live_table->m_lightHeight) : nullptr, 20.0f, 100.0f, "%.0f");
      PropFloat("Light Range", m_table, is_live, &(m_table->m_lightRange), m_live_table ? &(m_live_table->m_lightRange) : nullptr, 200.0f, 1000.0f, "%.0f");
      
      PropSeparator();
      // TODO Missing: environment texture combo
      PropFloat("Environment Em. Scale", m_table, is_live, &(m_table->m_envEmissionScale), m_live_table ? &(m_live_table->m_envEmissionScale) : nullptr, 0.1f, 0.5f, "%.3f", ImGuiInputTextFlags_CharsDecimal, reinit_lights);
      PropFloat("Ambient Occlusion Scale", m_table, is_live, &(m_table->m_AOScale), m_live_table ? &(m_live_table->m_AOScale) : nullptr, 0.1f, 1.0f);
      PropFloat("Bloom Strength", m_table, is_live, &(m_table->m_bloom_strength), m_live_table ? &(m_live_table->m_bloom_strength) : nullptr, 0.1f, 1.0f);
      PropFloat("Screen Space Reflection Scale", m_table, is_live, &(m_table->m_SSRScale), m_live_table ? &(m_live_table->m_SSRScale) : nullptr, 0.1f, 1.0f);
      
      PropSeparator();
      #ifdef ENABLE_BGFX
      static const string tonemapperLabels[] = { "Reinhard"s, "AgX"s, "Filmic"s, "Neutral"s, "AgX Punchy"s };
      #else
      static const string tonemapperLabels[] = { "Reinhard"s, "AgX"s, "Filmic"s, "Neutral"s };
      #endif
      int startup_mode = m_table ? (int)m_table->GetToneMapper() : 0;
      int live_mode = m_live_table ? (int)m_renderer->m_toneMapper : 0;
      PinTable * const table = m_table;
      Player * const player = m_player;
      auto upd_tm = [table, player](bool is_live, int prev, int v)
      {
         if (is_live)
            player->m_renderer->m_toneMapper = (ToneMapper)v;
         else
            table->SetToneMapper((ToneMapper)v);
      };
      PropCombo("Tonemapper", m_table, is_live, &startup_mode, &live_mode, std::size(tonemapperLabels), tonemapperLabels, upd_tm);
      ImGui::EndTable();
   }
}

void LiveUI::CameraProperties(bool is_live)
{
   PinTable *const table = (is_live ? m_live_table : m_table);

   switch (m_selection.camera)
   {
   case 0: ImGui::Text("Camera: Desktop"); break;
   case 1: ImGui::Text("Camera: Full Single Screen"); break;
   case 2: ImGui::Text("Camera: Cabinet"); break;
   default: return; // unsupported
   }
   ImGui::Separator();

   if (ImGui::Button("Import"))
   {
      table->ImportBackdropPOV(string());
      if (is_live)
         m_renderer->MarkShaderDirty();
   }
   ImGui::SameLine();
   if (ImGui::Button("Export"))
      table->ExportBackdropPOV();
   ImGui::NewLine();
   if (BEGIN_PROP_TABLE)
   {
      const ViewSetupID vsId = (ViewSetupID) m_selection.camera;
      static const string layoutModeLabels[] = { "Relative"s, "Absolute"s };
      int startup_mode = m_table ? (int)m_table->mViewSetups[vsId].mMode : 0;
      int live_mode = m_live_table ? (int)m_live_table->mViewSetups[vsId].mMode : 0;
      auto upd_mode = [table, vsId](bool is_live, int prev, int v) { table->mViewSetups[vsId].mMode = (ViewLayoutMode)v; };
      // View
      PropCombo("Layout Mode", m_table, is_live, &startup_mode, &live_mode, std::size(layoutModeLabels), layoutModeLabels, upd_mode);
      PropFloat("Field Of View", m_table, is_live, &(m_table->mViewSetups[vsId].mFOV), m_live_table ? &(m_live_table->mViewSetups[vsId].mFOV) : nullptr, 0.2f, 1.0f);
      PropFloat("Layback", m_table, is_live, &(m_table->mViewSetups[vsId].mLayback), m_live_table ? &(m_live_table->mViewSetups[vsId].mLayback) : nullptr, 0.2f, 1.0f);
      // Player position
      PropFloat("Inclination", m_table, is_live, &(m_table->mViewSetups[vsId].mLookAt), m_live_table ? &(m_live_table->mViewSetups[vsId].mLookAt) : nullptr, 0.2f, 1.0f);
      PropFloat("X Offset", m_table, is_live, &(m_table->mViewSetups[vsId].mViewX), m_live_table ? &(m_live_table->mViewSetups[vsId].mViewX) : nullptr, 10.0f, 50.0f, "%.0f");
      PropFloat("Y Offset", m_table, is_live, &(m_table->mViewSetups[vsId].mViewY), m_live_table ? &(m_live_table->mViewSetups[vsId].mViewY) : nullptr, 10.0f, 50.0f, "%.0f");
      PropFloat("Z Offset", m_table, is_live, &(m_table->mViewSetups[vsId].mViewZ), m_live_table ? &(m_live_table->mViewSetups[vsId].mViewZ) : nullptr, 10.0f, 50.0f, "%.0f");
      // Viewport
      PropFloat("Rotation", m_table, true, &(m_table->mViewSetups[vsId].mViewportRotation), m_live_table ? &(m_live_table->mViewSetups[vsId].mViewportRotation) : nullptr, 90.f, 90.0f, "%.0f");
      // Scene scale
      PropFloat("X Scale", m_table, is_live, &(m_table->mViewSetups[vsId].mSceneScaleX), m_live_table ? &(m_live_table->mViewSetups[vsId].mSceneScaleX) : nullptr, 0.002f, 0.01f);
      PropFloat("Y Scale", m_table, is_live, &(m_table->mViewSetups[vsId].mSceneScaleY), m_live_table ? &(m_live_table->mViewSetups[vsId].mSceneScaleY) : nullptr, 0.002f, 0.01f);
      PropFloat("Z Scale", m_table, is_live, &(m_table->mViewSetups[vsId].mSceneScaleZ), m_live_table ? &(m_live_table->mViewSetups[vsId].mSceneScaleZ) : nullptr, 0.002f, 0.01f);
      ImGui::EndTable();
   }
   ImGui::Separator();
   ImGui::Text("Absolute position:\nX: %.2f  Y: %.2f  Z: %.2f", -m_renderer->GetMVP().GetView()._41,
      (m_selection.camera == 0 || m_selection.camera == 2) ? m_renderer->GetMVP().GetView()._42 : -m_renderer->GetMVP().GetView()._42, 
      m_renderer->GetMVP().GetView()._43);
}

void LiveUI::ImageProperties()
{
   HelpTextCentered("Image"s);
   string name = m_selection.image->m_szName;
   ImGui::BeginDisabled(true); // Editing the name of a live item can break the script
   if (ImGui::InputText("Name", &name))
   {
   }
   ImGui::EndDisabled();
   ImGui::Separator();
   ImGui::BeginDisabled(m_selection.image->m_pdsBuffer == nullptr || !m_selection.image->m_pdsBuffer->has_alpha());
   if (ImGui::InputFloat("Alpha Mask", &m_selection.image->m_alphaTestValue))
      m_table->SetNonUndoableDirty(eSaveDirty);
   ImGui::EndDisabled();
   ImGui::Separator();
   Sampler *sampler = m_renderer->m_renderDevice->m_texMan.LoadTexture(m_selection.image->m_pdsBuffer, SamplerFilter::SF_BILINEAR, SamplerAddressMode::SA_CLAMP, SamplerAddressMode::SA_CLAMP, false);
#if defined(ENABLE_BGFX)
   ImTextureID image = (ImTextureID)sampler;
#elif defined(ENABLE_OPENGL)
   ImTextureID image = sampler ? (ImTextureID)sampler->GetCoreTexture() : 0;
#elif defined(ENABLE_DX9)
   ImTextureID image = sampler ? (ImTextureID)sampler->GetCoreTexture() : 0;
#endif
   if (image)
   {
      const float w = ImGui::GetWindowWidth();
      ImGui::Image(image, ImVec2(w, (float)sampler->GetHeight() * w / (float) sampler->GetWidth()));
   }
}

void LiveUI::RenderProbeProperties(bool is_live)
{
   RenderProbe * const live_probe = (RenderProbe *)(m_selection.is_live ? m_selection.renderprobe : m_live_table->m_startupToLive[m_selection.renderprobe]);
   RenderProbe * const startup_probe = (RenderProbe *)(m_selection.is_live ? m_live_table->m_liveToStartup[m_selection.renderprobe] : m_selection.renderprobe);
   HelpTextCentered("Render Probe"s);
   string name = ((RenderProbe *)m_selection.renderprobe)->GetName();
   ImGui::BeginDisabled(is_live); // Editing the name of a live item can break the script
   if (ImGui::InputText("Name", &name))
   {
      // FIXME add undo
      if (startup_probe)
         startup_probe->SetName(name);
   }
   ImGui::EndDisabled();
   ImGui::Separator();
   if (ImGui::CollapsingHeader("Visuals", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      static const string types[] = { "Reflection"s, "Refraction"s };

      auto upd_normal = [startup_probe, live_probe](bool is_live, vec3& prev, const vec3& v)
      {
         RenderProbe * const probe = (is_live ? live_probe : startup_probe);
         if (probe)
         {
            vec4 plane;
            probe->GetReflectionPlane(plane);
            plane.x = v.x;
            plane.y = v.y;
            plane.z = v.z;
            probe->SetReflectionPlane(plane);
         }
      };
      vec4 startup_plane, live_plane;
      Vertex3Ds startup_normal, live_normal;
      if (startup_probe)
      {
         startup_probe->GetReflectionPlane(startup_plane);
         startup_normal = Vertex3Ds(startup_plane.x, startup_plane.y, startup_plane.z);
      }
      if (live_probe)
      {
         live_probe->GetReflectionPlane(live_plane);
         live_normal = Vertex3Ds(live_plane.x, live_plane.y, live_plane.z);
      }
      PropVec3("Normal", nullptr, is_live, startup_probe ? &startup_normal : nullptr, live_probe ? &live_normal : nullptr, "%.0f", ImGuiInputTextFlags_CharsDecimal, upd_normal);

      auto upd_distance = [startup_probe, live_probe](bool is_live, float prev, float v)
      {
         RenderProbe * const probe = (is_live ? live_probe : startup_probe);
         if (probe)
         {
            vec4 plane;
            probe->GetReflectionPlane(plane);
            plane.w = v;
            probe->SetReflectionPlane(plane);
         }
      };
      PropFloat("Distance", nullptr, is_live, startup_probe ? &startup_plane.w : nullptr, live_probe ? &live_plane.w : nullptr, 1.f, 10.f, "%.0f", ImGuiInputTextFlags_CharsDecimal, upd_distance);

      ImGui::EndTable();
   }
   ImGui::Separator();
   if (ImGui::CollapsingHeader("Users", ImGuiTreeNodeFlags_DefaultOpen))
   {
      PinTable *const table = is_live ? m_live_table : m_table;
      RenderProbe *const probe = (is_live ? live_probe : startup_probe);
      for (size_t t = 0; t < table->m_vedit.size(); t++)
      {
         ISelect *const psel = table->m_vedit[t]->GetISelect();
         if (psel != nullptr && psel->GetItemType() == eItemPrimitive 
            && ((probe->GetType() == RenderProbe::PLANE_REFLECTION && ((Primitive *)psel)->m_d.m_szReflectionProbe == probe->GetName())
             || (probe->GetType() == RenderProbe::SCREEN_SPACE_TRANSPARENCY  && ((Primitive *)psel)->m_d.m_szRefractionProbe == probe->GetName()))
            && ImGui::Selectable(((Primitive *)psel)->GetName()))
            m_selection = Selection(is_live, table->m_vedit[t]);
      }
   }
}

void LiveUI::MaterialProperties(bool is_live)
{
   Material * const live_material = (Material *)(m_selection.is_live ? m_selection.editable : m_live_table->m_startupToLive[m_selection.editable]);
   Material * const startup_material = (Material *)(m_selection.is_live ? m_live_table->m_liveToStartup[m_selection.editable] : m_selection.editable);
   Material * const material = (is_live ? live_material : startup_material);
   HelpTextCentered("Material"s);
   string name = ((Material *)m_selection.editable)->m_szName;
   ImGui::BeginDisabled(is_live); // Editing the name of a live item can break the script
   if (ImGui::InputText("Name", &name))
   {
      // FIXME add undo
      if (startup_material)
         startup_material->m_szName = name;
   }
   ImGui::EndDisabled();
   ImGui::Separator();
   if (ImGui::CollapsingHeader("Visual", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      static const string matType[] = { "Default"s, "Metal"s };
      PropCombo("Type", m_table, is_live, startup_material ? (int *)&(startup_material->m_type) : nullptr, live_material ? (int *)&(live_material->m_type) : nullptr, std::size(matType), matType);
      if (material != nullptr)
      {
         PropRGB("Base Color", m_table, is_live, startup_material ? &(startup_material->m_cBase) : nullptr, live_material ? &(live_material->m_cBase) : nullptr);
         PropFloat("Wrap Lighting", m_table, is_live, startup_material ? &(startup_material->m_fWrapLighting) : nullptr, live_material ? &(live_material->m_fWrapLighting) : nullptr, 0.02f, 0.1f);
         if (material->m_type != Material::METAL)
         {
            PropRGB("Glossy Color", m_table, is_live, startup_material ? &(startup_material->m_cGlossy) : nullptr, live_material ? &(live_material->m_cGlossy) : nullptr);
            PropFloat("Glossy Image Lerp", m_table, is_live, startup_material ? &(startup_material->m_fGlossyImageLerp) : nullptr, live_material ? &(live_material->m_fGlossyImageLerp) : nullptr, 0.02f, 0.1f);
         }
         PropFloat("Shininess", m_table, is_live, startup_material ? &(startup_material->m_fRoughness) : nullptr, live_material ? &(live_material->m_fRoughness) : nullptr, 0.02f, 0.1f);
         PropRGB("Clearcoat Color", m_table, is_live, startup_material ? &(startup_material->m_cClearcoat) : nullptr, live_material ? &(live_material->m_cClearcoat) : nullptr);
         PropFloat("Edge Brightness", m_table, is_live, startup_material ? &(startup_material->m_fEdge) : nullptr, live_material ? &(live_material->m_fEdge) : nullptr, 0.02f, 0.1f);
      }
      ImGui::EndTable();
   }
   if (ImGui::CollapsingHeader("Transparency", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      PropCheckbox("Enable Transparency", m_table, is_live, startup_material ? &(startup_material->m_bOpacityActive) : nullptr, live_material ? &(live_material->m_bOpacityActive) : nullptr);
      PropFloat("Opacity", m_table, is_live, startup_material ? &(startup_material->m_fOpacity) : nullptr, live_material ? &(live_material->m_fOpacity) : nullptr, 0.02f, 0.1f);
      PropFloat("Edge Opacity", m_table, is_live, startup_material ? &(startup_material->m_fEdgeAlpha) : nullptr, live_material ? &(live_material->m_fEdgeAlpha) : nullptr, 0.02f, 0.1f);
      PropFloat("Thickness", m_table, is_live, startup_material ? &(startup_material->m_fThickness) : nullptr, live_material ? &(live_material->m_fThickness) : nullptr, 0.02f, 0.1f);
      PropRGB("Refraction Tint", m_table, is_live, startup_material ? &(startup_material->m_cRefractionTint) : nullptr, live_material ? &(live_material->m_cRefractionTint) : nullptr);
      ImGui::EndTable();
   }
}

void LiveUI::BallProperties(bool is_live, Ball *startup_obj, Ball *live_obj)
{
   m_renderer->ReinitRenderable(live_obj);
   Ball *const ball = (is_live ? live_obj : startup_obj);
   if (ImGui::CollapsingHeader("Visual", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      PropCheckbox("Visible", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_d.m_visible));
      PropCheckbox("Reflection enabled", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_d.m_reflectionEnabled));
      PropCheckbox("Reflection forced", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_d.m_forceReflection));
      PropCheckbox("Use Table Settings", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_d.m_useTableRenderSettings));
      ImGui::BeginDisabled(ball->m_d.m_useTableRenderSettings);
      PropRGB("Color", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_d.m_color));
      PropImageCombo("Image", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_d.m_szImage), m_table);
      PropCheckbox("Spherical Map", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_d.m_pinballEnvSphericalMapping));
      PropImageCombo("Decal", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_d.m_imageDecal), m_table);
      PropCheckbox("Decal mode", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_d.m_decalMode));
      PropFloat("PF Reflection Strength", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_d.m_playfieldReflectionStrength), 0.02f, 0.1f);
      PropFloat("Bulb Intensity Scale", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_d.m_bulb_intensity_scale), 0.02f, 0.1f);
      ImGui::EndDisabled();
      ImGui::EndTable();
   }
   if (ImGui::CollapsingHeader("Physics", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      PropVec3("Position", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_hitBall.m_d.m_pos), "%.0f", ImGuiInputTextFlags_CharsDecimal);
      PropFloat("Radius", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_hitBall.m_d.m_radius), 0.02f, 0.1f);
      PropFloat("Mass", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_hitBall.m_d.m_mass), 0.02f, 0.1f);
      PropVec3("Velocity", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_hitBall.m_d.m_vel), "%.3f", ImGuiInputTextFlags_CharsDecimal);
      PropVec3("Angular Momentum", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_hitBall.m_angularmomentum), "%.3f", ImGuiInputTextFlags_CharsDecimal);
      ImGui::EndTable();
   }
}

void LiveUI::BumperProperties(bool is_live, Bumper *startup_obj, Bumper *live_obj)
{
   m_renderer->ReinitRenderable(live_obj);
   m_player->m_physics->ReinitEditable(live_obj);
   if (ImGui::CollapsingHeader("Visuals", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      PropMaterialCombo("Cap Material", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_d.m_szCapMaterial), m_table);
      PropMaterialCombo("Base Material", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_d.m_szBaseMaterial), m_table);
      PropMaterialCombo("Skirt Material", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_d.m_szSkirtMaterial), m_table);
      PropMaterialCombo("Ring Material", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_d.m_szRingMaterial), m_table);
      PropFloat("Radius", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_d.m_radius), 0.1f, 0.5f, "%.1f");
      PropFloat("Height Scale", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_d.m_heightScale), 0.1f, 0.5f, "%.1f");
      PropFloat("Orientation", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_d.m_orientation), 0.1f, 0.5f, "%.1f");
      PropFloat("Ring Speed", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_d.m_ringSpeed), 0.1f, 0.5f, "%.1f");
      PropFloat("Ring Drop", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_d.m_ringDropOffset), 0.1f, 0.5f, "%.1f");
      PropCheckbox("Reflection Enabled", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_d.m_reflectionEnabled));
      PropCheckbox("Cap Visible", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_d.m_capVisible));
      PropCheckbox("Base Visible", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_d.m_baseVisible));
      PropCheckbox("Skirt Visible", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_d.m_skirtVisible));
      PropCheckbox("Ring Visible", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_d.m_ringVisible));
      // Missing position
      ImGui::EndTable();
   }
}

void LiveUI::FlasherProperties(bool is_live, Flasher *startup_obj, Flasher *live_obj)
{
   m_renderer->ReinitRenderable(live_obj);
   m_player->m_physics->ReinitEditable(live_obj);
   Flasher *const flasher = (is_live ? live_obj : startup_obj);
   if (flasher == nullptr)
      return;
   if (ImGui::CollapsingHeader("Visual", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      PropCheckbox("Visible", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_isVisible) : nullptr, live_obj ? &(live_obj->m_d.m_isVisible) : nullptr);
      static const string renderModes[] = { "Flasher"s, "DMD"s, "Display"s, "Alpha.Seg."s };
      PropCombo("Render Mode", m_table, is_live, startup_obj ? (int *)&(startup_obj->m_d.m_renderMode) : nullptr, live_obj ? (int *)&(live_obj->m_d.m_renderMode) : nullptr, std::size(renderModes), renderModes);
      PropRGB("Color", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_color) : nullptr, live_obj ? &(live_obj->m_d.m_color) : nullptr);
      // Missing Tex coord mode
      PropFloat("Depth bias", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_depthBias) : nullptr, live_obj ? &(live_obj->m_d.m_depthBias) : nullptr, 10.f, 100.f);

      if (flasher->m_d.m_renderMode == FlasherData::FLASHER)
      {
         PropImageCombo("Image A", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_szImageA) : nullptr, live_obj ? &(live_obj->m_d.m_szImageA) : nullptr, m_table);
         PropImageCombo("Image B", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_szImageB) : nullptr, live_obj ? &(live_obj->m_d.m_szImageB) : nullptr, m_table);
         // Missing Mode
         // Missing Filter Image B
         // Missing Amount
      }
      else if (flasher->m_d.m_renderMode == FlasherData::DMD)
      {
         static const string renderStyles[] = { "Legacy VPX", "Neon Plasma", "Red LED", "Green LED", "Yellow LED", "Generic Plasma", "Generic LED" };
         PropCombo("Render Style", m_table, is_live, startup_obj ? &(startup_obj->m_d.m_renderStyle) : nullptr, live_obj ? &(live_obj->m_d.m_renderStyle) : nullptr, std::size(renderStyles), renderStyles);
         // Missing source
         PropImageCombo("Glass", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_szImageA) : nullptr, live_obj ? &(live_obj->m_d.m_szImageA) : nullptr, m_table);
         PropFloat("Glass Roughness", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_glassRoughness) : nullptr, live_obj ? &(live_obj->m_d.m_glassRoughness) : nullptr, 0.f, 5.f);
         PropRGB("Glass Ambient", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_glassAmbient) : nullptr, live_obj ? &(live_obj->m_d.m_glassAmbient) : nullptr);
         PropFloat("Glass Pad Left", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_glassPadLeft) : nullptr, live_obj ? &(live_obj->m_d.m_glassPadLeft) : nullptr, 0.f, 1.f);
         PropFloat("Glass Pad Right", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_glassPadRight) : nullptr, live_obj ? &(live_obj->m_d.m_glassPadRight) : nullptr, 0.f, 1.f);
         PropFloat("Glass Pad Top", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_glassPadTop) : nullptr, live_obj ? &(live_obj->m_d.m_glassPadTop) : nullptr, 0.f, 1.f);
         PropFloat("Glass Pad Bottom", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_glassPadBottom) : nullptr, live_obj ? &(live_obj->m_d.m_glassPadBottom) : nullptr, 0.f, 1.f);
      }
      else if (flasher->m_d.m_renderMode == FlasherData::DISPLAY)
      {
         static const string renderStyles[] = { "Pixelated", "Smoothed" };
         PropCombo("Render Mode", m_table, is_live, startup_obj ? &(startup_obj->m_d.m_renderStyle) : nullptr, live_obj ? &(live_obj->m_d.m_renderStyle) : nullptr, std::size(renderStyles), renderStyles);
         // Missing source
      }
      ImGui::EndTable();
   }
   if (ImGui::CollapsingHeader("Transparency", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      PropInt("Opacity", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_alpha) : nullptr, live_obj ? &(live_obj->m_d.m_alpha) : nullptr);
      PropLightmapCombo("Lightmap", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_szLightmap) : nullptr, live_obj ? &(live_obj->m_d.m_szLightmap) : nullptr, m_table);
      PropCheckbox("Additive Blend", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_addBlend) : nullptr, live_obj ? &(live_obj->m_d.m_addBlend) : nullptr);
      PropFloat("Modulate", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_modulate_vs_add) : nullptr, live_obj ? &(live_obj->m_d.m_modulate_vs_add) : nullptr, 0.1f, 0.5f);
      ImGui::EndTable();
   }
   if (ImGui::CollapsingHeader("Position", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      // FIXME This allows to edit the center but does not update dragpoint coordinates accordingly => add a callback and use Translate
      // FIXME we also need to save dragpoint change when saving x/y to startup table as well as center pos => add a save callback and copy to startup table
      PropVec3("Position", startup_obj, is_live, 
         startup_obj ? &(startup_obj->m_d.m_vCenter.x) : nullptr, startup_obj ? &(startup_obj->m_d.m_vCenter.y) : nullptr, startup_obj ? &(startup_obj->m_d.m_height) : nullptr,
         live_obj    ? &(live_obj   ->m_d.m_vCenter.x) : nullptr, live_obj    ? &(live_obj   ->m_d.m_vCenter.y) : nullptr, live_obj    ? &(live_obj   ->m_d.m_height) : nullptr, "%.0f", ImGuiInputTextFlags_CharsDecimal);
      PropVec3("Rotation", startup_obj, is_live, 
         startup_obj ? &(startup_obj->m_d.m_rotX) : nullptr, startup_obj ? &(startup_obj->m_d.m_rotY) : nullptr, startup_obj ? &(startup_obj->m_d.m_rotZ) : nullptr, 
         live_obj    ? &(live_obj   ->m_d.m_rotX) : nullptr, live_obj    ? &(live_obj   ->m_d.m_rotY) : nullptr, live_obj    ? &(live_obj   ->m_d.m_rotZ) : nullptr, "%.0f", ImGuiInputTextFlags_CharsDecimal);
      ImGui::EndTable();
   }
}

void LiveUI::KickerProperties(bool is_live, Kicker *startup_obj, Kicker *live_obj)
{
   m_renderer->ReinitRenderable(live_obj);
   m_player->m_physics->ReinitEditable(live_obj);
   if (ImGui::CollapsingHeader("Visuals", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      PropMaterialCombo("Material", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_szMaterial) : nullptr, live_obj ? &(live_obj->m_d.m_szMaterial) : nullptr, m_table);
      static const string shapes[] = { "Invisible"s, "Hole"s, "Cup"s, "Hole Simple"s, "Williams"s, "Gottlieb"s, "Cup 2"s };
      PropCombo("Shape", startup_obj, is_live, startup_obj ? (int *)&(startup_obj->m_d.m_kickertype) : nullptr, live_obj ? (int *)&(live_obj->m_d.m_kickertype) : nullptr, std::size(shapes), shapes);
      PropFloat("Radius", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_radius) : nullptr, live_obj ? &(live_obj->m_d.m_radius) : nullptr, 0.1f, 0.5f, "%.1f");
      PropFloat("Orientation", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_orientation) : nullptr, live_obj ? &(live_obj->m_d.m_orientation) : nullptr, 0.1f, 0.5f, "%.1f");
      // Missing position
      ImGui::EndTable();
   }
}

void LiveUI::LightProperties(bool is_live, Light *startup_light, Light *live_light)
{
   m_renderer->ReinitRenderable(live_light);
   m_player->m_physics->ReinitEditable(live_light);
   Light *const light = (is_live ? live_light : startup_light);
   if (light && ImGui::CollapsingHeader("Visual", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      auto upd_intensity = [startup_light, live_light, light](bool is_live, float prev, float v)
      {
         if (prev > 0.1f && v > 0.1f)
         {
            const float fade_up_ms = prev / light->m_d.m_fadeSpeedUp;
            light->m_d.m_fadeSpeedUp = fade_up_ms < 0.1f ? 100000.0f : v / fade_up_ms;
            const float fade_down_ms = prev / light->m_d.m_fadeSpeedDown;
            light->m_d.m_fadeSpeedDown = fade_down_ms < 0.1f ? 100000.0f : v / fade_down_ms;
         }
         startup_light->m_currentIntensity = startup_light->m_d.m_intensity * startup_light->m_d.m_intensity_scale * startup_light->m_inPlayState;
         live_light->m_currentIntensity = live_light->m_d.m_intensity * live_light->m_d.m_intensity_scale * live_light->m_inPlayState;
      };
      float startup_fadeup = startup_light ? (startup_light->m_d.m_intensity / startup_light->m_d.m_fadeSpeedUp) : 0.f;
      float live_fadeup = live_light ? (live_light->m_d.m_intensity / live_light->m_d.m_fadeSpeedUp) : 0.f;
      auto upd_fade_up = [light](bool is_live, float prev, float v) { light->m_d.m_fadeSpeedUp = v < 0.1f ? 100000.0f : light->m_d.m_intensity / v;  };
      float startup_fadedown = startup_light ? (startup_light->m_d.m_intensity / startup_light->m_d.m_fadeSpeedDown) : 0.f;
      float live_fadedown = live_light ? (live_light->m_d.m_intensity / live_light->m_d.m_fadeSpeedDown) : 0.f;
      auto upd_fade_down = [light](bool is_live, float prev, float v) { light->m_d.m_fadeSpeedDown = v < 0.1f ? 100000.0f : light->m_d.m_intensity / v; };
      bool startup_shadow = startup_light ? (startup_light->m_d.m_shadows == ShadowMode::RAYTRACED_BALL_SHADOWS) : ShadowMode::NONE;
      bool live_shadow = live_light ? (live_light->m_d.m_shadows == ShadowMode::RAYTRACED_BALL_SHADOWS) : ShadowMode::NONE;
      auto upd_shadow = [light](bool is_live, bool prev, bool v) { light->m_d.m_shadows = v ? ShadowMode::RAYTRACED_BALL_SHADOWS : ShadowMode::NONE; };

      PropSeparator("Light Settings");
      PropFloat("Intensity", startup_light, is_live, startup_light ? &(startup_light->m_d.m_intensity) : nullptr, live_light ? &(live_light->m_d.m_intensity) : nullptr, 0.1f, 1.0f, "%.1f", ImGuiInputTextFlags_CharsDecimal, upd_intensity);
      static const string faders[] = { "None"s, "Linear"s, "Incandescent"s };
      PropCombo("Fader", startup_light, is_live, startup_light ? (int *)&(startup_light->m_d.m_fader) : nullptr, live_light ? (int *)&(live_light->m_d.m_fader) : nullptr, std::size(faders), faders);
      PropFloat("Fade Up (ms)", startup_light, is_live, startup_light ? &startup_fadeup : nullptr, live_light ? &live_fadeup : nullptr, 10.0f, 50.0f, "%.0f", ImGuiInputTextFlags_CharsDecimal, upd_fade_up);
      PropFloat("Fade Down (ms)", startup_light, is_live, startup_light ? &startup_fadedown : nullptr, live_light ? &live_fadedown : nullptr, 10.0f, 50.0f, "%.0f", ImGuiInputTextFlags_CharsDecimal, upd_fade_down);
      PropRGB("Light Color", startup_light, is_live, startup_light ? &(startup_light->m_d.m_color) : nullptr, live_light ? &(live_light->m_d.m_color) : nullptr);
      PropRGB("Center Burst", startup_light, is_live, startup_light ? &(startup_light->m_d.m_color2) : nullptr, live_light ? &(live_light->m_d.m_color2) : nullptr);
      PropFloat("Falloff Range", startup_light, is_live, startup_light ? &(startup_light->m_d.m_falloff) : nullptr, live_light ? &(live_light->m_d.m_falloff) : nullptr, 10.f, 100.f, "%.0f");
      PropFloat("Falloff Power", startup_light, is_live, startup_light ? &(startup_light->m_d.m_falloff_power) : nullptr, live_light ? &(live_light->m_d.m_falloff_power) : nullptr, 0.1f, 0.5f, "%.2f");

      PropSeparator("Render Mode");
      static const string modes[] = { "Hidden"s, "Classic"s, "Halo"s };
      int startup_mode = startup_light ? startup_light->m_d.m_visible ? startup_light->m_d.m_BulbLight ? 2 : 1 : 0 : -1;
      int live_mode = live_light ? live_light->m_d.m_visible ? live_light->m_d.m_BulbLight ? 2 : 1 : 0 : -1;
      auto upd_mode = [light](bool is_live, bool prev, int v) { light->m_d.m_visible = (v != 0); light->m_d.m_BulbLight = (v != 1); };
      PropCombo("Type", startup_light, is_live, startup_mode >= 0 ? &startup_mode : nullptr, live_mode >= 0 ? &live_mode : nullptr, std::size(modes), modes, upd_mode);
      if (!light->m_d.m_visible)
      {
      }
      else if (light->m_d.m_BulbLight)
      {
         PropCheckbox("Reflection Enabled", startup_light, is_live, startup_light ? &(startup_light->m_d.m_reflectionEnabled) : nullptr, live_light ? &(live_light->m_d.m_reflectionEnabled) : nullptr);
         PropFloat("Depth Bias", startup_light, is_live, startup_light ? &(startup_light->m_d.m_depthBias) : nullptr, live_light ? &(live_light->m_d.m_depthBias) : nullptr, 10.f, 50.f, "%.0f");
         PropFloat("Halo Height", startup_light, is_live, startup_light ? &(startup_light->m_d.m_bulbHaloHeight) : nullptr, live_light ? &(live_light->m_d.m_bulbHaloHeight) : nullptr, 1.f, 5.f, "%.1f");
         PropFloat("Modulate", startup_light, is_live, startup_light ? &(startup_light->m_d.m_modulate_vs_add) : nullptr, live_light ? &(live_light->m_d.m_modulate_vs_add) : nullptr, 0.1f, 0.5f, "%.1f");
         PropFloat("Transmission", startup_light, is_live, startup_light ? &(startup_light->m_d.m_transmissionScale) : nullptr, live_light ? &(live_light->m_d.m_transmissionScale) : nullptr, 0.1f, 0.5f, "%.1f");
      }
      else
      {
         PropCheckbox("Reflection Enabled", startup_light, is_live, startup_light ? &(startup_light->m_d.m_reflectionEnabled) : nullptr, live_light ? &(live_light->m_d.m_reflectionEnabled) : nullptr);
         PropFloat("Depth Bias", startup_light, is_live, startup_light ? &(startup_light->m_d.m_depthBias) : nullptr, live_light ? &(live_light->m_d.m_depthBias) : nullptr, 10.f, 50.f, "%.0f");
         PropCheckbox("PassThrough", startup_light, is_live, startup_light ? &(startup_light->m_d.m_imageMode) : nullptr, live_light ? &(live_light->m_d.m_imageMode) : nullptr);
         PropImageCombo("Image", startup_light, is_live, startup_light ? &(startup_light->m_d.m_szImage) : nullptr, live_light ? &(live_light->m_d.m_szImage) : nullptr, m_table);
      }

      PropSeparator("Bulb");
      PropCheckbox("Render bulb", startup_light, is_live, startup_light ? &(startup_light->m_d.m_showBulbMesh) : nullptr, live_light ? &(live_light->m_d.m_showBulbMesh) : nullptr);
      PropCheckbox("Static rendering", startup_light, is_live, startup_light ? &(startup_light->m_d.m_staticBulbMesh) : nullptr, live_light ? &(live_light->m_d.m_staticBulbMesh) : nullptr);
      PropFloat("Bulb Size", startup_light, is_live, startup_light ? &(startup_light->m_d.m_meshRadius) : nullptr, live_light ? &(live_light->m_d.m_meshRadius) : nullptr, 1.0f, 5.0f, "%.0f");

      PropSeparator("Ball reflections & Shadows");
      PropCheckbox("Show Reflection on Balls", startup_light, is_live, startup_light ? &(startup_light->m_d.m_showReflectionOnBall) : nullptr, live_light ? &(live_light->m_d.m_showReflectionOnBall) : nullptr);
      PropCheckbox("Raytraced ball shadows", startup_light, is_live, startup_light ? &startup_shadow : nullptr, live_light ? &live_shadow : nullptr, upd_shadow);

      PropSeparator("Position");
      // FIXME This allows to edit the center but does not update dragpoint coordinates accordingly => add a callback and use Translate
      // FIXME we also need to save dragpoint change when saving x/y to startup table as well as center pos => add a save callback and copy to startup table
      PropFloat("X", startup_light, is_live, startup_light ? &(startup_light->m_d.m_vCenter.x) : nullptr, live_light ? &(live_light->m_d.m_vCenter.x) : nullptr, 0.1f, 0.5f, "%.1f");
      PropFloat("Y", startup_light, is_live, startup_light ? &(startup_light->m_d.m_vCenter.y) : nullptr, live_light ? &(live_light->m_d.m_vCenter.y) : nullptr, 0.1f, 0.5f, "%.1f");
      PropFloat("Z", startup_light, is_live, startup_light ? &(startup_light->m_d.m_height) : nullptr, live_light ? &(live_light->m_d.m_height) : nullptr, 0.1f, 0.5f, "%.1f");

      ImGui::EndTable();
   }
   if (light && ImGui::CollapsingHeader("States", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      auto upd_inplaystate = [startup_light, live_light](bool is_live, float prev, float v)
      {
         Light * const light = (is_live ? live_light : startup_light);
         light->setInPlayState(v > 1.f ? (float)LightStateBlinking : v);
      };
      PropFloat("State", startup_light, is_live, startup_light ? &(startup_light->m_d.m_state) : nullptr, live_light ? &(live_light->m_d.m_state) : nullptr, 0.1f, 0.5f, "%.1f", ImGuiInputTextFlags_CharsDecimal, upd_inplaystate);
      // Missing blink pattern
      PropInt("Blink interval", startup_light, is_live, startup_light ? &(startup_light->m_d.m_blinkinterval) : nullptr, live_light ? &(live_light->m_d.m_blinkinterval) : nullptr);
      ImGui::EndTable();
   }
   PROP_TIMER(is_live, startup_light, live_light)
   if (is_live && ImGui::CollapsingHeader("Live state", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      PROP_TABLE_SETUP
      ImGui::BeginDisabled();
      ImGui::TableNextColumn();
      ImGui::InputFloat("Intensity", &live_light->m_currentIntensity);
      ImGui::TableNextColumn();
      ImGui::Button(ICON_SAVE "##t2");
      if (live_light->m_d.m_fader == FADER_INCANDESCENT)
      {
         ImGui::TableNextColumn();
         float temperature = (float)live_light->m_currentFilamentTemperature;
         ImGui::InputFloat("Filament Temperature", &temperature);
         ImGui::TableNextColumn();
         ImGui::Button(ICON_SAVE "##t1");
      }
      ImGui::EndDisabled();
      ImGui::EndTable();
   }
}

void LiveUI::PrimitiveProperties(bool is_live, Primitive *startup_obj, Primitive *live_obj)
{
   m_renderer->ReinitRenderable(live_obj);
   m_player->m_physics->ReinitEditable(live_obj);
   if (ImGui::CollapsingHeader("Visuals", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      PropSeparator("Render Options");
      PropCheckbox("Visible", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_visible) : nullptr, live_obj ? &(live_obj->m_d.m_visible) : nullptr);
      PropCheckbox("Static Rendering", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_staticRendering) : nullptr, live_obj ? &(live_obj->m_d.m_staticRendering) : nullptr);
      PropCheckbox("Reflection Enabled", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_reflectionEnabled) : nullptr, live_obj ? &(live_obj->m_d.m_reflectionEnabled) : nullptr);
      PropFloat("Depth Bias", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_depthBias) : nullptr, live_obj ? &(live_obj->m_d.m_depthBias) : nullptr, 10.f, 50.f, "%.0f");
      PropCheckbox("Depth Mask", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_useDepthMask) : nullptr, live_obj ? &(live_obj->m_d.m_useDepthMask) : nullptr);
      PropCheckbox("Render Backfaces", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_backfacesEnabled) : nullptr, live_obj ? &(live_obj->m_d.m_backfacesEnabled) : nullptr);
      PropCheckbox("Additive Blend", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_addBlend) : nullptr, live_obj ? &(live_obj->m_d.m_addBlend) : nullptr);
      PropLightmapCombo("Lightmap", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_szLightmap) : nullptr, live_obj ? &(live_obj->m_d.m_szLightmap) : nullptr, m_table);

      PropSeparator("Material");
      PropMaterialCombo("Material", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_szMaterial) : nullptr, live_obj ? &(live_obj->m_d.m_szMaterial) : nullptr, m_table);
      PropImageCombo("Image", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_szImage) : nullptr, live_obj ? &(live_obj->m_d.m_szImage) : nullptr, m_table);
      PropImageCombo("Normal Map", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_szNormalMap) : nullptr, live_obj ? &(live_obj->m_d.m_szNormalMap) : nullptr, m_table);
      PropCheckbox("Object Space NM", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_objectSpaceNormalMap) : nullptr, live_obj ? &(live_obj->m_d.m_objectSpaceNormalMap) : nullptr);
      PropFloat("Disable Spot Lights", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_disableLightingTop) : nullptr, live_obj ? &(live_obj->m_d.m_disableLightingTop) : nullptr, 0.01f, 0.05f, "%.3f");
      PropFloat("Translucency", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_disableLightingBelow) : nullptr, live_obj ? &(live_obj->m_d.m_disableLightingBelow) : nullptr, 0.01f, 0.05f, "%.3f");
      PropFloat("Modulate Opacity", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_alpha) : nullptr, live_obj ? &(live_obj->m_d.m_alpha) : nullptr, 0.01f, 0.05f, "%.3f");
      PropRGB("Modulate Color", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_color) : nullptr, live_obj ? &(live_obj->m_d.m_color) : nullptr);

      PropSeparator("Reflections");
      PropRenderProbeCombo("Reflection Probe", RenderProbe::PLANE_REFLECTION, startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_szReflectionProbe) : nullptr, live_obj ? &(live_obj->m_d.m_szReflectionProbe) : nullptr, m_table);
      PropFloat("Reflection strength", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_reflectionStrength) : nullptr, live_obj ? &(live_obj->m_d.m_reflectionStrength) : nullptr, 0.01f, 0.05f, "%.3f");

      PropSeparator("Refractions");
      PropRenderProbeCombo("Refraction Probe", RenderProbe::SCREEN_SPACE_TRANSPARENCY, startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_szRefractionProbe) : nullptr, live_obj ? &(live_obj->m_d.m_szRefractionProbe) : nullptr, m_table);
      PropFloat("Refraction thickness", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_refractionThickness) : nullptr, live_obj ? &(live_obj->m_d.m_refractionThickness) : nullptr, 0.01f, 0.05f, "%.3f");
      ImGui::EndTable();
   }
   if (ImGui::CollapsingHeader("Position", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      PropSeparator("Position, Rotation & Size");
      PropVec3("Position", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_vPosition) : nullptr, live_obj ? &(live_obj->m_d.m_vPosition) : nullptr, "%.0f", ImGuiInputTextFlags_CharsDecimal);
      PropVec3("Orientation", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_aRotAndTra[0]) : nullptr, live_obj ? &(live_obj->m_d.m_aRotAndTra[0]) : nullptr, "%.0f", ImGuiInputTextFlags_CharsDecimal);
      PropVec3("Scale", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_vSize) : nullptr, live_obj ? &(live_obj->m_d.m_vSize) : nullptr, "%.0f", ImGuiInputTextFlags_CharsDecimal);
      PropSeparator("Additional Transform");
      PropVec3("Translation", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_aRotAndTra[3]) : nullptr, live_obj ? &(live_obj->m_d.m_aRotAndTra[3]) : nullptr, "%.0f", ImGuiInputTextFlags_CharsDecimal);
      PropVec3("Rotation", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_aRotAndTra[6]) : nullptr, live_obj ? &(live_obj->m_d.m_aRotAndTra[6]) : nullptr, "%.0f", ImGuiInputTextFlags_CharsDecimal);
      ImGui::EndTable();
   }
   /* if (ImGui::CollapsingHeader("Physics", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      ImGui::EndTable();
   }*/
}

void LiveUI::RampProperties(bool is_live, Ramp *startup_obj, Ramp *live_obj)
{
   m_renderer->ReinitRenderable(live_obj);
   m_player->m_physics->ReinitEditable(live_obj);
   if (ImGui::CollapsingHeader("Visuals", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      // Missing type
      PropImageCombo("Image", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_szImage) : nullptr, live_obj ? &(live_obj->m_d.m_szImage) : nullptr, m_table);
      PropMaterialCombo("Material", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_szMaterial) : nullptr, live_obj ? &(live_obj->m_d.m_szMaterial) : nullptr, m_table);
      // Missing World
      PropCheckbox("Apply Image to Wall", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_imageWalls) : nullptr, live_obj ? &(live_obj->m_d.m_imageWalls) : nullptr);
      PropFloat("Depth Bias", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_depthBias) : nullptr, live_obj ? &(live_obj->m_d.m_depthBias) : nullptr, 10.f, 50.f, "%.0f");
      PropCheckbox("Visible", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_visible) : nullptr, live_obj ? &(live_obj->m_d.m_visible) : nullptr);
      PropCheckbox("Reflection Enabled", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_reflectionEnabled) : nullptr, live_obj ? &(live_obj->m_d.m_reflectionEnabled) : nullptr);
      // Missing all dimensions
      ImGui::EndTable();
   }
   /* if (ImGui::CollapsingHeader("Physics", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      ImGui::EndTable();
   }*/
}

void LiveUI::RubberProperties(bool is_live, Rubber *startup_obj, Rubber *live_obj)
{
   m_renderer->ReinitRenderable(live_obj);
   m_player->m_physics->ReinitEditable(live_obj);
   if (ImGui::CollapsingHeader("Visuals", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      PropImageCombo("Image", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_szImage) : nullptr, live_obj ? &(live_obj->m_d.m_szImage) : nullptr, m_table);
      PropMaterialCombo("Material", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_szMaterial) : nullptr, live_obj ? &(live_obj->m_d.m_szMaterial) : nullptr, m_table);
      PropCheckbox("Static Rendering", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_staticRendering) : nullptr, live_obj ? &(live_obj->m_d.m_staticRendering) : nullptr);
      PropCheckbox("Visible", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_visible) : nullptr, live_obj ? &(live_obj->m_d.m_visible) : nullptr);
      PropCheckbox("Reflection Enabled", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_reflectionEnabled) : nullptr, live_obj ? &(live_obj->m_d.m_reflectionEnabled) : nullptr);

      PropFloat("Height", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_height) : nullptr, live_obj ? &(live_obj->m_d.m_height) : nullptr, 0.1f, 0.5f, "%.1f");
      PropInt("Thickness", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_thickness) : nullptr, live_obj ? &(live_obj->m_d.m_thickness) : nullptr);
      PropVec3("Rotation", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_rotX) : nullptr, startup_obj ? &(startup_obj->m_d.m_rotY) : nullptr, startup_obj ? &(startup_obj->m_d.m_rotZ) : nullptr,
         live_obj ? &(live_obj->m_d.m_rotX) : nullptr, live_obj ? &(live_obj->m_d.m_rotY) : nullptr, live_obj ? &(live_obj->m_d.m_rotZ) : nullptr, "%.0f", ImGuiInputTextFlags_CharsDecimal);

      ImGui::EndTable();
   }
   /* if (ImGui::CollapsingHeader("Physics", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      ImGui::EndTable();
   }*/
   PROP_TIMER(is_live, startup_obj, live_obj)
}

void LiveUI::SurfaceProperties(bool is_live, Surface *startup_obj, Surface *live_obj)
{
   m_renderer->ReinitRenderable(live_obj);
   m_player->m_physics->ReinitEditable(live_obj);
   if (ImGui::CollapsingHeader("Visuals", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      PropCheckbox("Top Visible", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_topBottomVisible) : nullptr, live_obj ? &(live_obj->m_d.m_topBottomVisible) : nullptr);
      PropImageCombo("Top Image", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_szImage) : nullptr, live_obj ? &(live_obj->m_d.m_szImage) : nullptr, m_table);
      PropMaterialCombo("Top Material", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_szTopMaterial) : nullptr, live_obj ? &(live_obj->m_d.m_szTopMaterial) : nullptr, m_table);
      PropCheckbox("Side Visible", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_sideVisible) : nullptr, live_obj ? &(live_obj->m_d.m_sideVisible) : nullptr);
      PropImageCombo("Side Image", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_szSideImage) : nullptr, live_obj ? &(live_obj->m_d.m_szSideImage) : nullptr, m_table);
      PropMaterialCombo("Side Material", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_szSideMaterial) : nullptr, live_obj ? &(live_obj->m_d.m_szSideMaterial) : nullptr, m_table);
      // Missing animate slingshot
      // Missing flipbook
      PropFloat("Disable Spot Lights", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_disableLightingTop) : nullptr, live_obj ? &(live_obj->m_d.m_disableLightingTop) : nullptr, 0.01f, 0.05f, "%.3f");
      PropFloat("Translucency", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_disableLightingBelow) : nullptr, live_obj ? &(live_obj->m_d.m_disableLightingBelow) : nullptr, 0.01f, 0.05f, "%.3f");
      PropCheckbox("Reflection Enabled", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_reflectionEnabled) : nullptr, live_obj ? &(live_obj->m_d.m_reflectionEnabled) : nullptr);
      PropFloat("Top Height", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_heighttop) : nullptr, live_obj ? &(live_obj->m_d.m_heighttop) : nullptr, 0.1f, 0.5f, "%.1f");
      PropFloat("Bottom Height", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_heightbottom) : nullptr, live_obj ? &(live_obj->m_d.m_heightbottom) : nullptr, 0.1f, 0.5f, "%.1f");
      ImGui::EndTable();
   }
   /* if (ImGui::CollapsingHeader("Physics", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      ImGui::EndTable();
   }*/
   PROP_TIMER(is_live, startup_obj, live_obj)
}

void LiveUI::TriggerProperties(bool is_live, Trigger *startup_obj, Trigger *live_obj)
{
   m_renderer->ReinitRenderable(live_obj);
   m_player->m_physics->ReinitEditable(live_obj);
   if (ImGui::CollapsingHeader("Visuals", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      PropCheckbox("Visible", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_visible) : nullptr, live_obj ? &(live_obj->m_d.m_visible) : nullptr);
      PropCheckbox("Reflection Enabled", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_reflectionEnabled) : nullptr, live_obj ? &(live_obj->m_d.m_reflectionEnabled) : nullptr);
      static const string shapes[] = { "None"s, "Wire A"s, "Star"s, "Wire B"s, "Button"s, "Wire C"s, "Wire D"s, "Inder"s };
      PropCombo("Shape", startup_obj, is_live, startup_obj ? (int *)&(startup_obj->m_d.m_shape) : nullptr, live_obj ? (int *)&(live_obj->m_d.m_shape) : nullptr, std::size(shapes), shapes);
      PropFloat("Wire Thickness", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_wireThickness) : nullptr, live_obj ? &(live_obj->m_d.m_wireThickness) : nullptr, 0.1f, 0.5f, "%.1f");
      PropFloat("Star Radius", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_radius) : nullptr, live_obj ? &(live_obj->m_d.m_radius) : nullptr, 0.1f, 0.5f, "%.1f");
      PropFloat("Rotation", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_rotation) : nullptr, live_obj ? &(live_obj->m_d.m_rotation) : nullptr, 0.1f, 0.5f, "%.1f");
      PropFloat("Anim Speed", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_animSpeed) : nullptr, live_obj ? &(live_obj->m_d.m_animSpeed) : nullptr, 0.1f, 0.5f, "%.1f");
      PropMaterialCombo("Material", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_szMaterial) : nullptr, live_obj ? &(live_obj->m_d.m_szMaterial) : nullptr, m_table);
      // Missing position
      ImGui::EndTable();
   }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Property field helpers
//

#define PROP_HELPER_BEGIN(type)                                                                                                                                                              \
   PROP_TABLE_SETUP                                                                                                                                                                          \
   type * const v = is_live ? live_v : startup_v;                                                                                                                                            \
   type * const ov = is_live ? startup_v : live_v;                                                                                                                                           \
   ImGui::TableNextColumn();                                                                                                                                                                 \
   if (v == nullptr)                                                                                                                                                                         \
   {                                                                                                                                                                                         \
      /* Missing value just skip */                                                                                                                                                          \
      ImGui::TableNextColumn();                                                                                                                                                              \
      return;                                                                                                                                                                                \
   }                                                                                                                                                                                         \
   ImGui::PushID(label); \
   type prev_v = *v;

#define PROP_HELPER_SYNC(type)                                                                                                                                                               \
   /* Sync button(also show if there are difference between live and startup through the enable state) */                                                                                    \
   ImGui::TableNextColumn();                                                                                                                                                                 \
   if (ov != nullptr)                                                                                                                                                                        \
   {                                                                                                                                                                                         \
      const bool synced = ((*ov) == (*v));                                                                                                                                                   \
      if (synced)                                                                                                                                                                            \
         ImGui::BeginDisabled(); \
      type prev_ov = *ov; \
      if (ImGui::Button(ICON_SAVE)) \
      { \
         *ov = *v; \

/* 
TODO update undo stack instead of SetNonUndoableDirty
psel->GetIEditable()->BeginUndo();
psel->GetIEditable()->MarkForUndo();
// Change value
psel->GetIEditable()->EndUndo();
psel->GetIEditable()->SetDirtyDraw();
*/

#define PROP_HELPER_END                                                                                                                                                                      \
      if (is_live) \
         m_table->SetNonUndoableDirty(eSaveDirty); \
   }                                                                                                                                                                                         \
   if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))                                                                                                                                   \
   {                                                                                                                                                                                         \
      ImGui::BeginTooltip();                                                                                                                                                                 \
      ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);                                                                                                                                  \
      ImGui::Text("Copy this value to the %s version", is_live ? "startup" : "live");                                                                                                        \
      ImGui::PopTextWrapPos();                                                                                                                                                               \
      ImGui::EndTooltip();                                                                                                                                                                   \
   }                                                                                                                                                                                         \
   if (synced)                                                                                                                                                                               \
      ImGui::EndDisabled();                                                                                                                                                                  \
   }                                                                                                                                                                                         \
   ImGui::PopID();

void LiveUI::PropSeparator(const char *label)
{
   PROP_TABLE_SETUP
   ImGui::TableNextColumn();
   if (label)
      ImGui::Text("%s", label);
   ImGui::TableNextColumn();
}

void LiveUI::PropCheckbox(const char *label, IEditable *undo_obj, bool is_live, bool *startup_v, bool *live_v, const OnBoolPropChange &chg_callback)
{
   PROP_HELPER_BEGIN(bool)
   if (ImGui::Checkbox(label, v))
   {
      if (chg_callback)
         chg_callback(is_live, prev_v, *v);
      if (!is_live)
         m_table->SetNonUndoableDirty(eSaveDirty);
   }
   PROP_HELPER_SYNC(bool)
   if (chg_callback)
      chg_callback(!is_live, prev_ov, *ov);
   PROP_HELPER_END
}

void LiveUI::PropFloat(const char *label, IEditable* undo_obj, bool is_live, float *startup_v, float *live_v, float step, float step_fast, const char *format, ImGuiInputTextFlags flags, const OnFloatPropChange &chg_callback)
{
   PROP_HELPER_BEGIN(float)
   if (ImGui::InputFloat(label, v, step, step_fast, format, flags))
   {
      if (chg_callback)
         chg_callback(is_live, prev_v, *v);
      if (!is_live)
         m_table->SetNonUndoableDirty(eSaveDirty);
   }
   PROP_HELPER_SYNC(float)
   if (chg_callback)
      chg_callback(!is_live, prev_ov, *ov);
   PROP_HELPER_END
}

void LiveUI::PropInt(const char *label, IEditable *undo_obj, bool is_live, int *startup_v, int *live_v)
{
   PROP_HELPER_BEGIN(int)
   if (ImGui::InputInt(label, v))
   {
      if (!is_live)
         m_table->SetNonUndoableDirty(eSaveDirty);
   }
   PROP_HELPER_SYNC(int)
   PROP_HELPER_END
}

void LiveUI::PropRGB(const char *label, IEditable *undo_obj, bool is_live, COLORREF *startup_v, COLORREF *live_v, ImGuiColorEditFlags flags)
{
   PROP_HELPER_BEGIN(COLORREF)
   float col[3];
   col[0] = (float)((*v) & 255) * (float)(1.0 / 255.0);
   col[1] = (float)((*v) & 65280) * (float)(1.0 / 65280.0);
   col[2] = (float)((*v) & 16711680) * (float)(1.0 / 16711680.0);
   if (ImGui::ColorEdit3(label, col, flags))
   {
      const int r = clamp((int)(col[0] * 255.f + 0.5f), 0, 255);
      const int g = clamp((int)(col[1] * 255.f + 0.5f), 0, 255);
      const int b = clamp((int)(col[2] * 255.f + 0.5f), 0, 255);
      *v = RGB(r, g, b);
      if (!is_live)
         m_table->SetNonUndoableDirty(eSaveDirty);
   }
   PROP_HELPER_SYNC(COLORREF)
   PROP_HELPER_END
}

void LiveUI::PropVec3(const char *label, IEditable *undo_obj, bool is_live, float *startup_x, float *startup_y, float *startup_z, float *live_x, float *live_y, float *live_z, const char *format, ImGuiInputTextFlags flags, const OnVec3PropChange &chg_callback)
{
   PROP_TABLE_SETUP
   ImGui::TableNextColumn();
   if ((is_live ? live_x : startup_x) == nullptr)
   { /* Missing value just skip */
      ImGui::TableNextColumn();
      return;
   }
   vec3 v = is_live ? vec3(*live_x, *live_y, *live_z) : vec3(*startup_x, *startup_y, *startup_z);
   ImGui::PushID(label);
   vec3 prev_v = v;
   if (ImGui::InputFloat3(label, (float*) &v.x, format, flags))
   {
      *(is_live ? live_x : startup_x) = v.x;
      *(is_live ? live_y : startup_y) = v.y;
      *(is_live ? live_z : startup_z) = v.z;
      if (chg_callback)
      {
         chg_callback(is_live, prev_v, v);
      }
      if (!is_live)
         m_table->SetNonUndoableDirty(eSaveDirty);
   }
   /* Sync button(also show if there are difference between live and startup through the enable state) */
   ImGui::TableNextColumn();
   if ((is_live ? startup_x : live_x) != nullptr)
   {
      const bool synced = ((*startup_x) == (*live_x)) && ((*startup_y) == (*live_y)) && ((*startup_z) == (*live_z));
      if (synced)
         ImGui::BeginDisabled();
      if (ImGui::Button(ICON_SAVE))
      {
         *(is_live ? startup_x : live_x) = v.x;
         *(is_live ? startup_y : live_y) = v.y;
         *(is_live ? startup_z : live_z) = v.z;
   PROP_HELPER_END
}

void LiveUI::PropVec3(const char *label, IEditable *undo_obj, bool is_live, float *startup_v2, float *live_v2, const char *format, ImGuiInputTextFlags flags, const OnVec3PropChange &chg_callback)
{
   Vertex3Ds startV, liveV;
   Vertex3Ds *startup_v = nullptr, *live_v = nullptr;
   if (startup_v2)
   {
      startup_v = &startV;
      startV.Set(startup_v2[0], startup_v2[1], startup_v2[2]);
   }
   if (live_v2)
   {
      live_v = &liveV;
      liveV.Set(live_v2[0], live_v2[1], live_v2[2]);
   }
   PropVec3(label, undo_obj, is_live, startup_v, live_v, format, flags, chg_callback);
   if (startup_v2)
   {
      startup_v2[0] = startV.x;
      startup_v2[1] = startV.y;
      startup_v2[2] = startV.z;
   }
   if (live_v2)
   {
      live_v2[0] = liveV.x;
      live_v2[1] = liveV.y;
      live_v2[2] = liveV.z;
   }
}

void LiveUI::PropVec3(const char *label, IEditable *undo_obj, bool is_live, Vertex3Ds *startup_v, Vertex3Ds *live_v, const char *format, ImGuiInputTextFlags flags, const OnVec3PropChange &chg_callback)
{
   PROP_HELPER_BEGIN(Vertex3Ds)
   float col[3] = { v->x, v->y, v->z };
   if (ImGui::InputFloat3(label, col, format, flags))
   {
      v->Set(col[0], col[1], col[2]);
      if (chg_callback)
      {
         vec3 v1(prev_v.x, prev_v.y, prev_v.z), v2(v->x, v->y, v->z);
         chg_callback(is_live, v1, v2);
      }
      if (!is_live)
         m_table->SetNonUndoableDirty(eSaveDirty);
   }
   PROP_HELPER_SYNC(Vertex3Ds)
   PROP_HELPER_END
}

void LiveUI::PropCombo(const char *label, IEditable *undo_obj, bool is_live, int *startup_v, int *live_v, size_t n_values, const string labels[], const OnIntPropChange &chg_callback)
{
   PROP_HELPER_BEGIN(int)
   const char * const preview_value = labels[clamp(*v, 0, static_cast<int>(n_values) - 1)].c_str();
   if (ImGui::BeginCombo(label, preview_value))
   {
      for (int i = 0; i < (int)n_values; i++)
      {
         if (ImGui::Selectable(labels[i].c_str()))
         {
            *v = i;
            if (chg_callback)
               chg_callback(is_live, prev_v, i);
            if (!is_live)
               m_table->SetNonUndoableDirty(eSaveDirty);
         }
      }
      ImGui::EndCombo();
   }
   PROP_HELPER_SYNC(int)
   PROP_HELPER_END
}

void LiveUI::PropImageCombo(const char *label, IEditable *undo_obj, bool is_live, string *startup_v, string *live_v, PinTable *table, const OnStringPropChange &chg_callback)
{
   PROP_HELPER_BEGIN(string)
   const char *const preview_value = v->c_str();
   if (ImGui::BeginCombo(label, preview_value))
   {
      const std::function<string(Texture *)> map = [](Texture *image) -> string { return image->m_szName; };
      for (Texture *texture : SortedCaseInsensitive(table->m_vimage, map))
      {
         if (ImGui::Selectable(texture->m_szName.c_str()))
         {
            *v = texture->m_szName;
            if (chg_callback)
               chg_callback(is_live, prev_v, *v);
            if (!is_live)
               m_table->SetNonUndoableDirty(eSaveDirty);
         }
      }
      ImGui::EndCombo();
   }
   PROP_HELPER_SYNC(string)
   if (chg_callback)
      chg_callback(!is_live, prev_ov, *ov);
   PROP_HELPER_END
}

void LiveUI::PropMaterialCombo(const char *label, IEditable *undo_obj, bool is_live, string *startup_v, string *live_v, PinTable *table, const OnStringPropChange &chg_callback)
{
   PROP_HELPER_BEGIN(string)
   const char *const preview_value = v->c_str();
   if (ImGui::BeginCombo(label, preview_value))
   {
      const std::function<string(Material *)> map = [](Material *material) -> string { return material->m_szName; };
      for (Material *material : SortedCaseInsensitive(table->m_materials, map))
      {
         if (ImGui::Selectable(material->m_szName.c_str()))
         {
            *v = material->m_szName;
            if (chg_callback)
               chg_callback(is_live, prev_v, *v);
            if (!is_live)
               m_table->SetNonUndoableDirty(eSaveDirty);
         }
      }
      ImGui::EndCombo();
   }
   PROP_HELPER_SYNC(string)
   if (chg_callback)
      chg_callback(!is_live, prev_ov, *ov);
   PROP_HELPER_END
}

void LiveUI::PropLightmapCombo(const char *label, IEditable *undo_obj, bool is_live, string *startup_v, string *live_v, PinTable *table, const OnStringPropChange &chg_callback)
{
   PROP_HELPER_BEGIN(string)
   const char *const preview_value = v->c_str();
   if (ImGui::BeginCombo(label, preview_value))
   {
      const std::function<string(IEditable *)> map = [](IEditable *pe) -> string { return pe->GetItemType() == ItemTypeEnum::eItemLight ? pe->GetName() : ""s; };
      for (IEditable *pe : SortedCaseInsensitive(table->m_vedit, map))
      {
         if (pe->GetItemType() == ItemTypeEnum::eItemLight && ImGui::Selectable(pe->GetName()))
         {
            *v = pe->GetName();
            if (chg_callback)
               chg_callback(is_live, prev_v, *v);
            if (!is_live)
               m_table->SetNonUndoableDirty(eSaveDirty);
         }
      }
      ImGui::EndCombo();
   }
   PROP_HELPER_SYNC(string)
   if (chg_callback)
      chg_callback(!is_live, prev_ov, *ov);
   PROP_HELPER_END
}

void LiveUI::PropRenderProbeCombo(const char *label, RenderProbe::ProbeType type, IEditable *undo_obj, bool is_live, string *startup_v, string *live_v, PinTable *table, const OnStringPropChange &chg_callback)
{
   PROP_HELPER_BEGIN(string)
   const char *const preview_value = v->c_str();
   if (ImGui::BeginCombo(label, preview_value))
   {
      const std::function<string(RenderProbe *)> map = [](RenderProbe *probe) -> string { return probe->GetName(); };
      for (RenderProbe *probe : SortedCaseInsensitive(table->m_vrenderprobe, map))
      {
         if (probe->GetType() == type && ImGui::Selectable(probe->GetName().c_str()))
         {
            *v = probe->GetName();
            if (chg_callback)
               chg_callback(is_live, prev_v, *v);
            if (!is_live)
               m_table->SetNonUndoableDirty(eSaveDirty);
         }
      }
      ImGui::EndCombo();
   }
   PROP_HELPER_SYNC(string)
   if (chg_callback)
      chg_callback(!is_live, prev_ov, *ov);
   PROP_HELPER_END
}

#undef PROP_HELPER_BEGIN
#undef PROP_HELPER_SYNC
#undef PROP_HELPER_END
