// license:GPLv3+

#include "core/stdafx.h"

#include "LiveUI.h"

#include "renderer/VRDevice.h"

#include "fonts/DroidSans.h"
#include "fonts/DroidSansBold.h"
#include "fonts/IconsForkAwesome.h"
#include "fonts/ForkAwesome.h"

#include "plugins/VPXPlugin.h"
#include "core/VPXPluginAPIImpl.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h" // Needed for FindRenderedTextEnd in HelpSplash (should be adapted when this function will refactored in ImGui)

#if defined(ENABLE_SDL_VIDEO)
  #include "imgui/imgui_impl_sdl3.h"
#else
  #include "imgui/imgui_impl_win32.h"
#endif

#if defined(ENABLE_DX9)
  #include <shellapi.h>
#endif

#include "imgui/imgui_stdlib.h"
#include "imgui_markdown/imgui_markdown.h"

#ifndef __STANDALONE__
#include "BAM/BAMView.h"
#endif

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
   ImGuiKey_LeftBracket, //DIK_LBRACKET      0x1A
   ImGuiKey_RightBracket, //DIK_RBRACKET     0x1B
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
   ImGuiKey_GraveAccent, //DIK_GRAVE         0x29    /* accent grave */
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
   ImGuiKey_KeypadMultiply, //DIK_MULTIPLY   0x37    /* * on numeric keypad */
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
   ImGuiKey_KeypadSubtract, //DIK_SUBTRACT   0x4A    /* - on numeric keypad */
   ImGuiKey_Keypad4,   //DIK_NUMPAD4         0x4B
   ImGuiKey_Keypad5,   //DIK_NUMPAD5         0x4C
   ImGuiKey_Keypad6,   //DIK_NUMPAD6         0x4D
   ImGuiKey_KeypadAdd, //DIK_ADD             0x4E    /* + on numeric keypad */
   ImGuiKey_Keypad1,   //DIK_NUMPAD1         0x4F
   ImGuiKey_Keypad2,   //DIK_NUMPAD2         0x50
   ImGuiKey_Keypad3,   //DIK_NUMPAD3         0x51
   ImGuiKey_Keypad0,   //DIK_NUMPAD0         0x52
   ImGuiKey_KeypadDecimal, //DIK_DECIMAL     0x53    /* . on numeric keypad */
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

ImGuiKey LiveUI::GetImGuiKeysFromDIkeycode(const int dik)
{
   if (dik < 0 || dik >= static_cast<int>(std::size(dikToImGuiKeys)))
      return ImGuiKey_None;
   return dikToImGuiKeys[dik];
}

void LiveUI::CenteredText(const string &text)
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
   style.TabCloseButtonMinWidthUnselected = 0.0f;
   style.ColorButtonPosition = ImGuiDir_Right;
   style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
   style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

   style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
   style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.592f, 0.592f, 0.592f, overall_alpha);
   style.Colors[ImGuiCol_WindowBg] = ImVec4(0.145f, 0.145f, 0.149f, overall_alpha);
   style.Colors[ImGuiCol_ChildBg] = ImVec4(0.145f, 0.145f, 0.149f, overall_alpha);
   style.Colors[ImGuiCol_PopupBg] = ImVec4(0.145f, 0.145f, 0.149f, overall_alpha);
   style.Colors[ImGuiCol_Border] = ImVec4(0.306f, 0.306f, 0.306f, overall_alpha);
   style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.306f, 0.306f, 0.306f, overall_alpha);
   style.Colors[ImGuiCol_FrameBg] = ImVec4(0.2f, 0.2f, 0.216f, overall_alpha);
   style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.114f, 0.592f, 0.925f, overall_alpha);
   style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.0f, 0.467f, 0.784f, overall_alpha);
   style.Colors[ImGuiCol_TitleBg] = ImVec4(0.145f, 0.145f, 0.149f, overall_alpha);
   style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.145f, 0.145f, 0.149f, overall_alpha);
   style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.145f, 0.145f, 0.149f, overall_alpha);
   style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.2f, 0.2f, 0.216f, overall_alpha);
   style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.2f, 0.2f, 0.216f, overall_alpha);
   style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.322f, 0.322f, 0.333f, overall_alpha);
   style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.353f, 0.353f, 0.373f, overall_alpha);
   style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.353f, 0.353f, 0.373f, overall_alpha);
   style.Colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 0.467f, 0.784f, overall_alpha);
   style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.114f, 0.592f, 0.925f, overall_alpha);
   style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.0f, 0.467f, 0.784f, overall_alpha);
   style.Colors[ImGuiCol_Button] = ImVec4(0.2f, 0.2f, 0.216f, overall_alpha);
   style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.114f, 0.592f, 0.925f, overall_alpha);
   style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.114f, 0.592f, 0.925f, overall_alpha);
   style.Colors[ImGuiCol_Header] = ImVec4(0.2f, 0.2f, 0.216f, overall_alpha);
   style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.114f, 0.592f, 0.925f, overall_alpha);
   style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.0f, 0.467f, 0.784f, overall_alpha);
   style.Colors[ImGuiCol_Separator] = ImVec4(0.306f, 0.306f, 0.306f, overall_alpha);
   style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.306f, 0.306f, 0.306f, overall_alpha);
   style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.306f, 0.306f, 0.306f, overall_alpha);
   style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.145f, 0.145f, 0.149f, overall_alpha);
   style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.2f, 0.2f, 0.216f, overall_alpha);
   style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.322f, 0.322f, 0.333f, overall_alpha);
   style.Colors[ImGuiCol_Tab] = ImVec4(0.145f, 0.145f, 0.149f, overall_alpha);
   style.Colors[ImGuiCol_TabHovered] = ImVec4(0.114f, 0.592f, 0.925f, overall_alpha);
   style.Colors[ImGuiCol_TabActive] = ImVec4(0.0f, 0.467f, 0.784f, overall_alpha);
   style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.145f, 0.145f, 0.149f, overall_alpha);
   style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.0f, 0.467f, 0.784f, overall_alpha);
   style.Colors[ImGuiCol_PlotLines] = ImVec4(0.0f, 0.467f, 0.784f, overall_alpha);
   style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.114f, 0.592f, 0.925f, overall_alpha);
   style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.0f, 0.467f, 0.784f, overall_alpha);
   style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.114f, 0.592f, 0.925f, overall_alpha);
   style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.188f, 0.188f, 0.2f, overall_alpha);
   style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.310f, 0.310f, 0.349f, overall_alpha);
   style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.227f, 0.227f, 0.247f, overall_alpha);
   style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f * overall_alpha);
   style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.060f * overall_alpha);
   style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.0f, 0.467f, 0.784f, overall_alpha);
   style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.145f, 0.145f, 0.149f, overall_alpha);
   style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.145f, 0.145f, 0.149f, overall_alpha);
   style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.700f * overall_alpha);
   style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.800f, 0.800f, 0.800f, 0.20f * overall_alpha);
   style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.145f, 0.145f, 0.149f, 0.35f * overall_alpha);
}


ImGui::MarkdownConfig LiveUI::markdown_config;

LiveUI::LiveUI(RenderDevice *const rd)
   : m_inGameUI(*this) 
   , m_editorUI(*this)
   , m_ballControl(*this)
   , m_escSplashModal(*this)
   , m_rd(rd)
   , m_perfUI(g_pplayer)
{
   m_app = g_pvp;
   m_player = g_pplayer;
   m_table = m_player->m_pEditorTable;
   m_live_table = m_player->m_ptable;
   m_pininput = &(m_player->m_pininput);
   m_renderer = m_player->m_renderer;
   
   IMGUI_CHECKVERSION();
   ImGui::CreateContext();
   ImGuiIO &io = ImGui::GetIO();
   io.IniFilename = nullptr; //don't use an ini file for configuration

   #if defined(ENABLE_SDL_VIDEO) // SDL Windowing
      // using the specialized initializer is not needed
      // ImGui_ImplSDL3_InitForOpenGL(m_player->m_playfieldSdlWnd, rd->m_sdl_context);
      ImGui_ImplSDL3_InitForOther(m_player->m_playfieldWnd->GetCore());
      //int displayIndex = SDL_GetDisplayForWindow(m_player->m_playfieldWnd->GetCore());
      if (m_player->m_vrDevice)
      {
         // VR headset cover full view range, so use a relative part of the full range for the DPI
         m_dpi = min(m_player->m_vrDevice->GetEyeWidth(), m_player->m_vrDevice->GetEyeHeight()) / 2000.f;
      }
      else
      {
         // Use display DPI setting
         // On macOS/iOS, keep m_dpi at 1.0f. ImGui_ImplSDL3_NewFrame applies a 2.0f DisplayFramebufferScale
         // for SDL_WINDOW_HIGH_PIXEL_DENSITY windows. A m_dpi of 2.0 would cause the UI to scale at 400%.
         // See: https://wiki.libsdl.org/SDL3/README/highdpi
         m_dpi = SDL_GetWindowDisplayScale(m_player->m_playfieldWnd->GetCore()) / SDL_GetWindowPixelDensity(m_player->m_playfieldWnd->GetCore());
      }
   #else // Win32 Windowing
      ImGui_ImplWin32_Init(m_player->m_playfieldWnd->GetCore());
      m_dpi = ImGui_ImplWin32_GetDpiScaleForHwnd(m_player->m_playfieldWnd->GetCore());
   #endif
   m_dpi = min(m_dpi, 10.f); // To avoid texture size overflows
   m_perfUI.SetDPI(m_dpi);
   m_plumbOverlay.SetDPI(m_dpi);

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

   io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;

   NewFrame();
}

LiveUI::~LiveUI()
{
   HideUI();
   if (ImGui::GetCurrentContext())
   {
      ImGui::EndFrame();

      ImGuiIO &io = ImGui::GetIO();
      io.BackendFlags &= ~ImGuiBackendFlags_RendererHasTextures;
      for (auto tex : ImGui::GetPlatformIO().Textures)
      {
         tex->SetTexID(ImTextureID_Invalid);
         tex->SetStatus(ImTextureStatus_Destroyed);
      }

      #if defined(ENABLE_SDL_VIDEO)
         ImGui_ImplSDL3_Shutdown();
      #else
         ImGui_ImplWin32_Shutdown();
      #endif

      ImGui::DestroyContext();
   }
}

void LiveUI::MarkdownFormatCallback(const ImGui::MarkdownFormatInfo &markdownFormatInfo, bool start)
{
   const LiveUI *const ui = static_cast<LiveUI*>(markdownFormatInfo.config->userData);
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
            ImGui::PushFont(ui->m_overlayBoldFont, ui->m_overlayBoldFont->LegacySize);
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
            ImGui::PushFont(fmt.font, fmt.font->LegacySize);
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
      ShellExecuteA(NULL, "open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
      #endif
   }
}

ImGui::MarkdownImageData LiveUI::MarkdownImageCallback(ImGui::MarkdownLinkCallbackData data)
{
   LiveUI *const ui = (LiveUI *)data.userData;
   Texture *const ppi = ui->m_live_table->GetImage(std::string(data.link, data.linkLength));
   if (ppi == nullptr)
      return ImGui::MarkdownImageData {};
   std::shared_ptr<Sampler> sampler = ui->m_renderer->m_renderDevice->m_texMan.LoadTexture(ppi, false);
   if (sampler == nullptr)
      return ImGui::MarkdownImageData {};
   ImGui::MarkdownImageData imageData { true, false, sampler, ImVec2(static_cast<float>(sampler->GetWidth()), static_cast<float>(sampler->GetHeight())) };
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
   return ImGui::GetIO().WantCaptureMouse || m_ballControl.GetMode() != BallControl::Mode::Disabled;
}

void LiveUI::NewFrame()
{
   #if defined(ENABLE_SDL_VIDEO)
      ImGui_ImplSDL3_NewFrame();
   #else
      ImGui_ImplWin32_NewFrame();
   #endif

   const int width = m_rd->GetCurrentPass() ? m_rd->GetCurrentPass()->m_rt->GetWidth() : 1920;
   const int height = m_rd->GetCurrentPass() ? m_rd->GetCurrentPass()->m_rt->GetHeight() : 1080;
   ImGuiIO &io = ImGui::GetIO();
   io.DisplaySize.x = static_cast<float>(width) / io.DisplayFramebufferScale.x;
   io.DisplaySize.y = static_cast<float>(height) / io.DisplayFramebufferScale.y;
   m_rotate = m_renderer->m_stereo3D == STEREO_VR
      ? 0
      : ((int)(m_player->m_ptable->mViewSetups[m_player->m_ptable->m_BG_current_set].GetRotation((int)io.DisplaySize.x, (int)io.DisplaySize.y) / 90.0f));
   if (m_rotate == 1 || m_rotate == 3)
   {
      const float size = io.DisplaySize.x;
      io.DisplaySize.x = io.DisplaySize.y;
      io.DisplaySize.y = size;
      const float scale = io.DisplayFramebufferScale.x;
      io.DisplayFramebufferScale.x = io.DisplayFramebufferScale.y;
      io.DisplayFramebufferScale.y = scale;
   }

   #if defined(ENABLE_SDL_VIDEO)
      // Enable mouse capture when dragging (needed when dragging main windows)
      {
         bool want_capture = false;
         for (int button_n = 0; button_n < ImGuiMouseButton_COUNT && !want_capture; button_n++)
            if (ImGui::IsMouseDragging(button_n, 1.0f))
               want_capture = true;
         SDL_CaptureMouse(want_capture);
      }
      // Update mouse position to latest global state (needed when dragging main windows)
      SDL_Window *focused_window = SDL_GetKeyboardFocus();
      if (!SDL_GetWindowRelativeMouseMode(focused_window))
      {
         float mouse_x_global, mouse_y_global;
         int window_x, window_y;
         SDL_GetGlobalMouseState(&mouse_x_global, &mouse_y_global);
         SDL_GetWindowPosition(focused_window, &window_x, &window_y);
         const ImVec2 mousePos(mouse_x_global - window_x, mouse_y_global - window_y);
         switch (m_rotate)
         {
         case 0: ImGui::GetIO().AddMousePosEvent(mousePos.x, mousePos.y); break;
         case 1: ImGui::GetIO().AddMousePosEvent(mousePos.y, io.DisplaySize.y - mousePos.x); break;
         case 2: ImGui::GetIO().AddMousePosEvent(mousePos.x, io.DisplaySize.y - mousePos.y); break;
         case 3: ImGui::GetIO().AddMousePosEvent(io.DisplaySize.x - mousePos.y, mousePos.x); break;
         default: assert(false); return;
         }
      }
   #endif

   ImGui::NewFrame();

   // Only enable keyboard navigation for main splash popup as it interfer with UI keyboard shortcuts
   if (m_escSplashModal.IsOpened())
      io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
   else
      io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;
}

void LiveUI::Update()
{
   // For the time being, the UI is only available inside a running player
   if (m_player == nullptr || m_player->GetCloseState() != Player::CS_PLAYING || m_rd->GetCurrentPass() == nullptr)
      return;

   const int width = m_rd->GetCurrentPass()->m_rt->GetWidth();
   const int height = m_rd->GetCurrentPass()->m_rt->GetHeight();

   UpdateTouchUI();

   ImGui::PushFont(m_baseFont, m_baseFont->LegacySize);

   if (m_escSplashModal.IsOpened())
   {
      m_escSplashModal.Update();
   }
   else if (ImGui::IsPopupOpen(ID_BAM_SETTINGS))
   { // BAM headtracking UI (aligned to desktop, using traditional mouse interaction) => hacky, remove and use plugin + plugin settings instead
      #ifndef __STANDALONE__
         BAMView::drawMenu();
      #endif
   }
   else if (m_editorUI.IsOpened())
   { // Editor UI (aligned to desktop, using traditional mouse interaction)
      m_editorUI.Update();
   }
   else if (m_inGameUI.IsOpened())
   { // Tweak UI (aligned to playfield view, using custom flipper controls)
      m_inGameUI.Update();
   }
   else
   { // No UI displayed: process ball control & throw balls
      m_ballControl.Update(width, height);
   }

   // Display plumb state overlay
   m_plumbOverlay.Update();

   // Display notification overlays except when script has an unaligned rotation
   m_notificationOverlay.Update(true, m_overlayFont);

   // Display performance overlays
   m_perfUI.Update();

   ImGui::PopFont();

   ImGui::EndFrame();

   ImGui::Render();

   ImDrawData *const draw_data = ImGui::GetDrawData();

   if (draw_data->Textures != nullptr)
      for (ImTextureData *tex : *draw_data->Textures)
      {
         if (tex->Status == ImTextureStatus_WantCreate || tex->Status == ImTextureStatus_WantUpdates)
         {
            // Somewhat overkill as we treat update as destroy/create but fine enough (just slightly impact performance)
            assert(tex->GetPitch() == tex->Width * 4);
            assert(tex->Format == ImTextureFormat_RGBA32);
            std::shared_ptr<BaseTexture> texture;
            BaseTexture::Update(texture, tex->Width, tex->Height, BaseTexture::RGBA, static_cast<const uint8_t *>(tex->GetPixels()));
            tex->SetTexID(m_renderer->m_renderDevice->m_texMan.LoadTexture(texture.get(), false));
            tex->SetStatus(ImTextureStatus_OK);
         }
         if (tex->Status == ImTextureStatus_WantDestroy && tex->UnusedFrames > 0)
         {
            tex->SetTexID(ImTextureID_Invalid);
            tex->SetStatus(ImTextureStatus_Destroyed);
         }
      }

   const ImGuiIO &io = ImGui::GetIO();
   const Matrix3D matRotate = Matrix3D::MatrixRotateZ(static_cast<float>(m_rotate * (M_PI / 2.0)));
   Matrix3D matTranslate;
   switch (m_rotate)
   {
   case 0: matTranslate = Matrix3D::MatrixIdentity(); break;
   case 1: matTranslate = Matrix3D::MatrixTranslate(io.DisplaySize.y, 0, 0); break;
   case 2: matTranslate = Matrix3D::MatrixTranslate(io.DisplaySize.x, io.DisplaySize.y, 0); break;
   case 3: matTranslate = Matrix3D::MatrixTranslate(0, io.DisplaySize.x, 0); break;
   default: assert(false); return;
   }
   const float right = (m_rotate == 1 || m_rotate == 3) ? io.DisplaySize.y : io.DisplaySize.x;
   const float bottom = (m_rotate == 1 || m_rotate == 3) ? io.DisplaySize.x : io.DisplaySize.y;
   Matrix3D matProj = matRotate * matTranslate * Matrix3D::MatrixOrthoOffCenterRH(0.f, right, bottom, 0.f, 0.f, 1.f);
   m_rd->m_uiShader->SetMatrix(SHADER_matWorldView, &matProj);
   m_rd->m_uiShader->SetVector(SHADER_staticColor_Alpha, 
      // Stereo offset for VR (fake depth)
      m_player->m_vrDevice ? ((float)m_player->m_vrDevice->GetEyeWidth() * 0.15f) : 0.f,
      0.f, // Unused
      0.f, // Unused
      // A value of 1.0 should be sdrWhite * 80, while in the WCG colorspace 80 nits is 0.5
      m_player->m_playfieldWnd->IsWCGBackBuffer() ? (2.0f / m_player->m_playfieldWnd->GetSDRWhitePoint()) : 1.f); // SDR color scaling
   m_rd->ResetRenderState();
   m_rd->SetRenderState(RenderState::COLORWRITEENABLE, RenderState::RGBMASK_RGBA);
   m_rd->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_TRUE);
   m_rd->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_TRUE);
   m_rd->SetRenderState(RenderState::BLENDOP, RenderState::BLENDOP_ADD);
   m_rd->SetRenderState(RenderState::SRCBLEND, RenderState::SRC_ALPHA);
   m_rd->SetRenderState(RenderState::DESTBLEND, RenderState::INVSRC_ALPHA);
   m_rd->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
   m_rd->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
   m_rd->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);
   m_rd->m_uiShader->SetTechnique(SHADER_TECHNIQUE_LiveUI);

   if (m_meshBuffers.size() < draw_data->CmdListsCount)
      m_meshBuffers.resize(draw_data->CmdListsCount);
   m_meshBuffers.resize(draw_data->CmdListsCount);
   for (int n = 0; n < draw_data->CmdListsCount; n++)
   {
      const ImDrawList * const cmd_list = draw_data->CmdLists[n];
      const unsigned int numVertices = cmd_list->VtxBuffer.size();
      const unsigned int numIndices = cmd_list->IdxBuffer.size();

      if ((numVertices != 0) && (numIndices != 0))
      {
         if (const std::unique_ptr<MeshBuffer> &meshBuffer = m_meshBuffers[n]; meshBuffer == nullptr || meshBuffer->m_ib->m_count < numIndices || meshBuffer->m_vb->m_count < numVertices)
         {
            IndexBuffer *ib = new IndexBuffer(m_rd, numIndices, true, IndexBuffer::Format::FMT_INDEX32);
            VertexBuffer *vb = new VertexBuffer(m_rd, numVertices, nullptr, true);
            m_meshBuffers[n] = std::make_unique<MeshBuffer>(vb, ib);
         }

         Vertex3D_NoTex2 *vb;
         m_meshBuffers[n]->m_vb->Lock(vb);
         for (unsigned int i = 0; i < numVertices; i++)
         {
            const uint32_t rgba = cmd_list->VtxBuffer[i].col;
            vb[i].x = cmd_list->VtxBuffer[i].pos.x;
            vb[i].y = cmd_list->VtxBuffer[i].pos.y;
            vb[i].z = (float)((rgba >> 24) & 0xFFu) * (float)(1.0 / 255.0); // alpha
            vb[i].nx = (float)(rgba & 0x000000FFu) * (float)(1.0 / 255.0); // red
            vb[i].ny = (float)(rgba & 0x0000FF00u) * (float)(1.0 / 65280.0); // green
            vb[i].nz = (float)(rgba & 0x00FF0000u) * (float)(1.0 / 16711680.0); // blue
            vb[i].tu = cmd_list->VtxBuffer[i].uv.x;
            vb[i].tv = cmd_list->VtxBuffer[i].uv.y;
         }
         m_meshBuffers[n]->m_vb->Unlock();

         WORD *ib;
         m_meshBuffers[n]->m_ib->Lock(ib);
         memcpy(ib, cmd_list->IdxBuffer.begin(), numIndices * sizeof(ImDrawIdx));
         m_meshBuffers[n]->m_ib->Unlock();
      }

      for (const ImDrawCmd *cmd = cmd_list->CmdBuffer.begin(), *cmdEnd = cmd_list->CmdBuffer.end(); cmd != cmdEnd; cmd++)
      {
         if (cmd->ElemCount != 0)
         {
            /* TODO use RenderPass areaaOfInterest to implement scissor including stereo rendering (not really used) and for rotated view (not really used either)
            if (g_stereoOfs[0] == 0.f && g_rotate == 0)
            {
               const uint16_t xx = (uint16_t)bx::max(cmd->ClipRect.x, 0.0f);
               const uint16_t yy = (uint16_t)bx::max(cmd->ClipRect.y, 0.0f);
               bgfx::setScissor(xx, yy, (uint16_t)bx::min(cmd->ClipRect.z, 65535.0f) - xx, (uint16_t)bx::min(cmd->ClipRect.w, 65535.0f) - yy);
            } */
            m_rd->m_uiShader->SetTexture(SHADER_tex_base_color, cmd->GetTexID());
            m_rd->DrawMesh(m_rd->m_uiShader, true, Vertex3Ds(), -10000.f, &*m_meshBuffers[n], RenderDevice::TRIANGLELIST, cmd->IdxOffset, cmd->ElemCount);
         }
      }
   }

   NewFrame();
}

void LiveUI::UpdateTouchUI()
{
   if (!m_player->m_supportsTouch)
      return;

#ifdef __LIBVPINBALL__
   if (m_player->m_liveUIOverride)
      return;
#endif

   if (!g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "TouchOverlay"s, false))
      return;

   const ImGuiIO &io = ImGui::GetIO();

   float screenWidth = io.DisplaySize.x;
   float screenHeight = io.DisplaySize.y;

   constexpr ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

   ImGui::SetNextWindowBgAlpha(0.0f);
   ImGui::SetNextWindowPos(ImVec2(0, 0));
   ImGui::SetNextWindowSize(ImVec2(screenWidth, screenHeight));
   ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
   ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

   ImGui::Begin("Touch Controls", nullptr, window_flags);
   ImDrawList *drawList = ImGui::GetWindowDrawList();
   for (int i = 0; i < MAX_TOUCHREGION; ++i)
   {
      RECT rect = touchregion[i];

      ImVec2 topLeft((float)rect.left * screenWidth / 100.0f, (float)rect.top * screenHeight / 100.0f);
      ImVec2 bottomRight((float)rect.right * screenWidth / 100.0f, (float)rect.bottom * screenHeight / 100.0f);

      ImColor fillColor(255, 255, 255, 5);
      drawList->AddRectFilled(topLeft, bottomRight, fillColor);

      ImColor borderColor(255, 255, 255, 20);
      drawList->AddRect(topLeft, bottomRight, borderColor, 0.0f, ImDrawFlags_RoundCornersAll, 2.0f);
   }
   ImGui::End();
   ImGui::PopStyleVar(2);
}

void LiveUI::OpenTweakMode()
{
   m_editorUI.Close();
   m_inGameUI.Open();
}

void LiveUI::HideUI()
{ 
   SetupImGuiStyle(1.0f);
   m_renderer->InitLayout();
   m_inGameUI.Close();
   m_editorUI.Close();
   m_table->m_settings.Save();
   g_pvp->m_settings.Save();
   m_player->SetPlayState(true);
}
