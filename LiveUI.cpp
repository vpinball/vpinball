#include "stdafx.h"
#include "LiveUI.h"

#include "Shader.h"

#include "droidsans.h"

#include "imgui/imgui.h"
#ifdef ENABLE_SDL
#include "imgui/imgui_impl_opengl3.h"
#else
#include "imgui/imgui_impl_dx9.h"
#endif
#include "imgui/imgui_impl_win32.h"
#include "imgui/implot/implot.h"

#if __cplusplus >= 202002L && !defined(__clang__)
#define stable_sort std::ranges::stable_sort
#else
#define stable_sort std::stable_sort
#endif

#ifdef ENABLE_BAM
#include "BAM/BAMView.h"
#endif

// utility structure for realtime plot //!! cleanup
class ScrollingData
{
private:
   int MaxSize;

public:
   int Offset;
   ImVector<ImVec2> Data;
   ScrollingData()
   {
      MaxSize = 500;
      Offset = 0;
      Data.reserve(MaxSize);
   }
   void AddPoint(const float x, const float y)
   {
      if (Data.size() < MaxSize)
         Data.push_back(ImVec2(x, y));
      else
      {
         Data[Offset] = ImVec2(x, y);
         Offset++;
         if (Offset == MaxSize)
            Offset = 0;
      }
   }
   void Erase()
   {
      if (!Data.empty())
      {
         Data.shrink(0);
         Offset = 0;
      }
   }
   ImVec2 GetLast()
   {
      if (Data.empty())
         return ImVec2(0.f, 0.f);
      else if (Data.size() < MaxSize || Offset == 0)
         return Data.back();
      else
         return Data[Offset - 1];
   }
};

// utility structure for realtime plot
/*class RollingData {
    float Span;
    ImVector<ImVec2> Data;
    RollingData() {
        Span = 10.0f;
        Data.reserve(500);
    }
    void AddPoint(const float x, const float y) {
        const float xmod = fmodf(x, Span);
        if (!Data.empty() && xmod < Data.back().x)
            Data.shrink(0);
        Data.push_back(ImVec2(xmod, y));
    }
};*/

// Maps directinput keycode used by VPX to ImGui keycode enum
// TODO I think this is not fully ok, since I would guess that VPX is keyboard layout neutral while ImGui is likely not => this needs more testing (AZERTY/QWERTY/...)
static constexpr ImGuiKey dikToImGuiKeys[] = {
   ImGuiKey_None,
   ImGuiKey_Escape, //DIK_ESCAPE          0x01
   ImGuiKey_1, //DIK_1               0x02
   ImGuiKey_2, //DIK_2               0x03
   ImGuiKey_3, //DIK_3               0x04
   ImGuiKey_4, //DIK_4               0x05
   ImGuiKey_5, //DIK_5               0x06
   ImGuiKey_6, //DIK_6               0x07
   ImGuiKey_7, //DIK_7               0x08
   ImGuiKey_8, //DIK_8               0x09
   ImGuiKey_9, //DIK_9               0x0A
   ImGuiKey_0, //DIK_0               0x0B
   ImGuiKey_Minus, //DIK_MINUS           0x0C    /* - on main keyboard */
   ImGuiKey_Equal, //DIK_EQUALS          0x0D
   ImGuiKey_Backspace, //DIK_BACK            0x0E    /* backspace */
   ImGuiKey_Tab, //DIK_TAB             0x0F
   ImGuiKey_Q, //DIK_Q               0x10
   ImGuiKey_W, //DIK_W               0x11
   ImGuiKey_E, //DIK_E               0x12
   ImGuiKey_R, //DIK_R               0x13
   ImGuiKey_T, //DIK_T               0x14
   ImGuiKey_Y, //DIK_Y               0x15
   ImGuiKey_U, //DIK_U               0x16
   ImGuiKey_I, //DIK_I               0x17
   ImGuiKey_O, //DIK_O               0x18
   ImGuiKey_P, //DIK_P               0x19
   ImGuiKey_LeftBracket, //DIK_LBRACKET        0x1A
   ImGuiKey_RightBracket, //DIK_RBRACKET        0x1B
   ImGuiKey_Enter, //DIK_RETURN          0x1C    /* Enter on main keyboard */
   ImGuiKey_LeftCtrl, //DIK_LCONTROL        0x1D
   ImGuiKey_A, //DIK_A               0x1E
   ImGuiKey_S, //DIK_S               0x1F
   ImGuiKey_D, //DIK_D               0x20
   ImGuiKey_F, //DIK_F               0x21
   ImGuiKey_G, //DIK_G               0x22
   ImGuiKey_H, //DIK_H               0x23
   ImGuiKey_J, //DIK_J               0x24
   ImGuiKey_K, //DIK_K               0x25
   ImGuiKey_L, //DIK_L               0x26
   ImGuiKey_Semicolon, //DIK_SEMICOLON       0x27
   ImGuiKey_Apostrophe, //DIK_APOSTROPHE      0x28
   ImGuiKey_GraveAccent, //DIK_GRAVE           0x29    /* accent grave */
   ImGuiKey_LeftShift, //DIK_LSHIFT          0x2A
   ImGuiKey_Backslash, //DIK_BACKSLASH       0x2B
   ImGuiKey_Z, //DIK_Z               0x2C
   ImGuiKey_X, //DIK_X               0x2D
   ImGuiKey_C, //DIK_C               0x2E
   ImGuiKey_V, //DIK_V               0x2F
   ImGuiKey_B, //DIK_B               0x30
   ImGuiKey_N, //DIK_N               0x31
   ImGuiKey_M, //DIK_M               0x32
   ImGuiKey_Comma, //DIK_COMMA           0x33
   ImGuiKey_Period, //DIK_PERIOD          0x34    /* . on main keyboard */
   ImGuiKey_Slash, //DIK_SLASH           0x35    /* / on main keyboard */
   ImGuiKey_RightShift, //DIK_RSHIFT          0x36
   ImGuiKey_KeypadMultiply, //DIK_MULTIPLY        0x37    /* * on numeric keypad */
   ImGuiKey_Menu, //DIK_LMENU           0x38    /* left Alt */
   ImGuiKey_Space, //DIK_SPACE           0x39
   ImGuiKey_CapsLock, //DIK_CAPITAL         0x3A
   ImGuiKey_F1, //DIK_F1              0x3B
   ImGuiKey_F2, //DIK_F2              0x3C
   ImGuiKey_F3, //DIK_F3              0x3D
   ImGuiKey_F4, //DIK_F4              0x3E
   ImGuiKey_F5, //DIK_F5              0x3F
   ImGuiKey_F6, //DIK_F6              0x40
   ImGuiKey_F7, //DIK_F7              0x41
   ImGuiKey_F8, //DIK_F8              0x42
   ImGuiKey_F9, //DIK_F9              0x43
   ImGuiKey_F10, //DIK_F10             0x44
   ImGuiKey_NumLock, //DIK_NUMLOCK         0x45
   ImGuiKey_ScrollLock, //DIK_SCROLL          0x46    /* Scroll Lock */
   ImGuiKey_Keypad7, //DIK_NUMPAD7         0x47
   ImGuiKey_Keypad8, //DIK_NUMPAD8         0x48
   ImGuiKey_Keypad9, //DIK_NUMPAD9         0x49
   ImGuiKey_KeypadSubtract, //DIK_SUBTRACT        0x4A    /* - on numeric keypad */
   ImGuiKey_Keypad4, //DIK_NUMPAD4         0x4B
   ImGuiKey_Keypad5, //DIK_NUMPAD5         0x4C
   ImGuiKey_Keypad6, //DIK_NUMPAD6         0x4D
   ImGuiKey_KeypadAdd, //DIK_ADD             0x4E    /* + on numeric keypad */
   ImGuiKey_Keypad1, //DIK_NUMPAD1         0x4F
   ImGuiKey_Keypad2, //DIK_NUMPAD2         0x50
   ImGuiKey_Keypad3, //DIK_NUMPAD3         0x51
   ImGuiKey_Keypad0, //DIK_NUMPAD0         0x52
   ImGuiKey_KeypadDecimal, //DIK_DECIMAL         0x53    /* . on numeric keypad */
   ImGuiKey_None, //0x54
   ImGuiKey_None, //0x55
   ImGuiKey_None, //DIK_OEM_102         0x56    /* < > | on UK/Germany keyboards */
   ImGuiKey_F11, //DIK_F11             0x57
   ImGuiKey_F12, //DIK_F12             0x58
   ImGuiKey_None, //0x59
   ImGuiKey_None, //0x5A
   ImGuiKey_None, //0x5B
   ImGuiKey_None, //0x5C
   ImGuiKey_None, //0x5D
   ImGuiKey_None, //0x5E
   ImGuiKey_None, //0x5F
   ImGuiKey_None, //0x60
   ImGuiKey_None, //0x61
   ImGuiKey_None, //0x62
   ImGuiKey_None, //0x63
   ImGuiKey_None, //DIK_F13             0x64    /*                     (NEC PC98) */
   ImGuiKey_None, //DIK_F14             0x65    /*                     (NEC PC98) */
   ImGuiKey_None, //DIK_F15             0x66    /*                     (NEC PC98) */
   ImGuiKey_None, //0x67
   ImGuiKey_None, //0x68
   ImGuiKey_None, //0x69
   ImGuiKey_None, //0x6A
   ImGuiKey_None, //0x6B
   ImGuiKey_None, //0x6C
   ImGuiKey_None, //0x6D
   ImGuiKey_None, //0x6E
   ImGuiKey_None, //0x6F

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

   ImGuiKey_KeypadEnter, //#define DIK_NUMPADENTER     0x9C    /* Enter on numeric keypad */
   ImGuiKey_RightCtrl, //DIK_RCONTROL        0x9D

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

   ImGuiKey_KeypadDivide, //#define DIK_DIVIDE          0xB5    /* / on numeric keypad */
   ImGuiKey_None, //0xB6
   ImGuiKey_None, //#define DIK_SYSRQ           0xB7
   ImGuiKey_RightAlt, //DIK_RMENU           0xB8    /* right Alt */

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

   ImGuiKey_Home, //DIK_HOME            0xC7    /* Home on arrow keypad */
   ImGuiKey_UpArrow, //DIK_UP              0xC8    /* UpArrow on arrow keypad */
   ImGuiKey_PageUp, //DIK_PRIOR           0xC9    /* PgUp on arrow keypad */
   ImGuiKey_None, //0xCA
   ImGuiKey_LeftArrow, //DIK_LEFT            0xCB    /* LeftArrow on arrow keypad */
   ImGuiKey_None, //0xCC
   ImGuiKey_RightArrow, //DIK_RIGHT           0xCD    /* RightArrow on arrow keypad */
   ImGuiKey_None, //0xCE
   ImGuiKey_End, //DIK_END             0xCF    /* End on arrow keypad */
   ImGuiKey_DownArrow, //DIK_DOWN            0xD0    /* DownArrow on arrow keypad */
   ImGuiKey_PageDown, //DIK_NEXT            0xD1    /* PgDn on arrow keypad */
   ImGuiKey_Insert, //DIK_INSERT          0xD2    /* Insert on arrow keypad */
   ImGuiKey_Delete, //DIK_DELETE          0xD3    /* Delete on arrow keypad */

   ImGuiKey_None, //0xD4
   ImGuiKey_None, //0xD5
   ImGuiKey_None, //0xD6
   ImGuiKey_None, //0xD7
   ImGuiKey_None, //0xD8
   ImGuiKey_None, //0xD9
   ImGuiKey_None, //0xDA

   ImGuiKey_LeftSuper, //DIK_LWIN            0xDB    /* Left Windows key */
   ImGuiKey_RightSuper, //DIK_RWIN            0xDC    /* Right Windows key */
   ImGuiKey_None, //DIK_APPS            0xDD    /* AppMenu key */
};

void SetupImGuiStyle()
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
   style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.5921568870544434f, 0.5921568870544434f, 0.5921568870544434f, 1.0f);
   style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
   style.Colors[ImGuiCol_ChildBg] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
   style.Colors[ImGuiCol_PopupBg] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
   style.Colors[ImGuiCol_Border] = ImVec4(0.3058823645114899f, 0.3058823645114899f, 0.3058823645114899f, 1.0f);
   style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.3058823645114899f, 0.3058823645114899f, 0.3058823645114899f, 1.0f);
   style.Colors[ImGuiCol_FrameBg] = ImVec4(0.2000000029802322f, 0.2000000029802322f, 0.2156862765550613f, 1.0f);
   style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, 1.0f);
   style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, 1.0f);
   style.Colors[ImGuiCol_TitleBg] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
   style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
   style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
   style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.2000000029802322f, 0.2000000029802322f, 0.2156862765550613f, 1.0f);
   style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.2000000029802322f, 0.2000000029802322f, 0.2156862765550613f, 1.0f);
   style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.321568638086319f, 0.321568638086319f, 0.3333333432674408f, 1.0f);
   style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.3529411852359772f, 0.3529411852359772f, 0.3725490272045135f, 1.0f);
   style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.3529411852359772f, 0.3529411852359772f, 0.3725490272045135f, 1.0f);
   style.Colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, 1.0f);
   style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, 1.0f);
   style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, 1.0f);
   style.Colors[ImGuiCol_Button] = ImVec4(0.2000000029802322f, 0.2000000029802322f, 0.2156862765550613f, 1.0f);
   style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, 1.0f);
   style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, 1.0f);
   style.Colors[ImGuiCol_Header] = ImVec4(0.2000000029802322f, 0.2000000029802322f, 0.2156862765550613f, 1.0f);
   style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, 1.0f);
   style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, 1.0f);
   style.Colors[ImGuiCol_Separator] = ImVec4(0.3058823645114899f, 0.3058823645114899f, 0.3058823645114899f, 1.0f);
   style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.3058823645114899f, 0.3058823645114899f, 0.3058823645114899f, 1.0f);
   style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.3058823645114899f, 0.3058823645114899f, 0.3058823645114899f, 1.0f);
   style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
   style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.2000000029802322f, 0.2000000029802322f, 0.2156862765550613f, 1.0f);
   style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.321568638086319f, 0.321568638086319f, 0.3333333432674408f, 1.0f);
   style.Colors[ImGuiCol_Tab] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
   style.Colors[ImGuiCol_TabHovered] = ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, 1.0f);
   style.Colors[ImGuiCol_TabActive] = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, 1.0f);
   style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
   style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, 1.0f);
   style.Colors[ImGuiCol_PlotLines] = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, 1.0f);
   style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, 1.0f);
   style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, 1.0f);
   style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, 1.0f);
   style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.1882352977991104f, 0.1882352977991104f, 0.2000000029802322f, 1.0f);
   style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3490196168422699f, 1.0f);
   style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.2274509817361832f, 0.2274509817361832f, 0.2470588237047195f, 1.0f);
   style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
   style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.05999999865889549f);
   style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, 1.0f);
   style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
   style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
   style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.699999988079071f);
   style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.2000000029802322f);
   style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
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

static void HelpTextCentered(std::string text)
{
   ImVec2 win_size = ImGui::GetWindowSize();
   ImVec2 text_size = ImGui::CalcTextSize(text.c_str());

   // calculate the indentation that centers the text on one line, relative
   // to window left, regardless of the `ImGuiStyleVar_WindowPadding` value
   float text_indentation = (win_size.x - text_size.x) * 0.5f;

   // if text is too long to be drawn on one line, `text_indentation` can
   // become too small or even negative, so we check a minimum indentation
   float min_indentation = 20.0f;
   if (text_indentation <= min_indentation)
      text_indentation = min_indentation;

   ImGui::SameLine(text_indentation);
   ImGui::PushTextWrapPos(win_size.x - text_indentation);
   ImGui::TextWrapped(text.c_str());
   ImGui::PopTextWrapPos();
}

static void HelpSplash(std::string text, int rotation)
{
   ImVec2 win_size = ImGui::GetIO().DisplaySize;
   ImVec2 text_size = ImGui::CalcTextSize(text.c_str());
   if (rotation == 1 || rotation == 3)
   {
      float tmp = win_size.x;
      win_size.x = win_size.y;
      win_size.y = tmp;
   }
   ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
   ImGui::SetNextWindowBgAlpha(0.35f);
   ImGui::SetNextWindowPos(ImVec2((win_size.x - text_size.x - 10) / 2, (win_size.y - text_size.y - 10) / 2));
   ImGui::SetNextWindowSize(ImVec2(text_size.x + 20, text_size.y + 20));
   ImGui::Begin("ToolTip", NULL, window_flags);
   ImGui::Text(text.c_str());
   ImGui::End();
}


LiveUI::LiveUI(RenderDevice *rd)
   : m_rd(rd)
{
   m_StartTime_usec = usec();
   m_disable_esc = LoadValueBoolWithDefault(regKey[RegName::Player], "DisableESC"s, m_disable_esc);

   IMGUI_CHECKVERSION();
   ImGui::CreateContext();
   ImPlot::CreateContext();
   ImGuiIO &io = ImGui::GetIO();
   io.IniFilename = nullptr; //don't use an ini file for configuration
   
   ImGui_ImplWin32_Init(rd->getHwnd());
   
   SetupImGuiStyle();

   ImGui_ImplWin32_EnableDpiAwareness();
   float dpi = ImGui_ImplWin32_GetDpiScaleForHwnd(rd->getHwnd());
   io.Fonts->AddFontFromMemoryCompressedTTF(droidsans_compressed_data, droidsans_compressed_size, 13.0f * dpi);
   ImGui::GetStyle().ScaleAllSizes(dpi);

#ifdef ENABLE_SDL
   ImGui_ImplOpenGL3_Init();
#else
   ImGui_ImplDX9_Init(rd->GetCoreDevice());
#endif
}

LiveUI::~LiveUI()
{
   if (ImGui::GetCurrentContext())
   {
#ifdef ENABLE_SDL
      ImGui_ImplOpenGL3_Shutdown();
#else
      ImGui_ImplDX9_Shutdown();
#endif
      ImGui_ImplWin32_Shutdown();
      ImPlot::DestroyContext();
      ImGui::DestroyContext();
   }
}

bool LiveUI::HasKeyboardCapture() const
{
   return ImGui::GetIO().WantCaptureKeyboard;
}

bool LiveUI::HasMouseCapture() const
{
   return ImGui::GetIO().WantCaptureKeyboard;
}

void LiveUI::Render()
{
   // For the time being, the UI is only available inside a running player
   if (g_pplayer == nullptr || g_pplayer->m_closing != Player::CS_PLAYING)
      return;
   if (m_rotate != 0)
   {
      // We hack into ImGui renderer for the simple tooltips that must be displayed facing the user
      ImGui::GetBackgroundDrawList()->AddCallback(
         [](const ImDrawList *parent_list, const ImDrawCmd *cmd)
         {
            LiveUI *lui = (LiveUI *)cmd->UserCallbackData;
            Matrix3D matRotate, matTranslate;
            matRotate.RotateZMatrix(lui->m_rotate * M_PI / 2.0f);
            switch (lui->m_rotate)
            {
            case 1: matTranslate.SetTranslation(ImGui::GetIO().DisplaySize.x, 0, 0); break;
            case 2: matTranslate.SetTranslation(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y, 0); break;
            case 3: matTranslate.SetTranslation(0, ImGui::GetIO().DisplaySize.y, 0); break;
            }
            matTranslate.Multiply(matRotate, matTranslate);
#ifdef ENABLE_SDL
            float L = 0, R = ImGui::GetIO().DisplaySize.x;
            float T = 0, B = ImGui::GetIO().DisplaySize.y;
            Matrix3D matProj(
               2.0f / (R - L), 0.0f, 0.0f, 0.0f, 
               0.0f, 2.0f / (T - B), 0.0f, 0.0f, 
               0.0f, 0.0f, -1.0f, 0.0f, 
               (R + L) / (L - R), (T + B) / (B - T), 0.0f, 1.0f);
            matProj.Multiply(matTranslate, matProj);
            GLint shaderHandle;
            glGetIntegerv(GL_CURRENT_PROGRAM, &shaderHandle);
            GLuint attribLocationProjMtx = glGetUniformLocation(shaderHandle, "ProjMtx");
            glUniformMatrix4fv(attribLocationProjMtx, 1, GL_FALSE, (float*)  & (matProj.m[0]));
            glDisable(GL_SCISSOR_TEST);
#else
            lui->m_rd->GetCoreDevice()->SetTransform(D3DTS_WORLD, (const D3DXMATRIX *)&matTranslate);
            lui->m_rd->GetCoreDevice()->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
#endif

         }, this);
   }
   ImGui::Render();
   ImDrawData *draw_data = ImGui::GetDrawData();
#ifdef ENABLE_SDL
   ImGui_ImplOpenGL3_RenderDrawData(draw_data);
#else
   ImGui_ImplDX9_RenderDrawData(draw_data);
#endif
}

void LiveUI::Update()
{
   // For the time being, the UI is only available inside a running player
   if (g_pplayer == nullptr || g_pplayer->m_closing != Player::CS_PLAYING)
      return;

#ifdef ENABLE_SDL
   ImGui_ImplOpenGL3_NewFrame();
#else
   ImGui_ImplDX9_NewFrame();
#endif
   ImGui_ImplWin32_NewFrame();
   ImGui::NewFrame();

#ifdef ENABLE_BAM
   if (g_pplayer->m_infoMode == IF_BAM_MENU)
   {
      // Head tracking menu
      m_rotate = 0;
      BAMView::drawMenu();
   }
   else
#endif
   if (m_ShowUI > 0)
   {
      // Main UI
      m_rotate = 0;
      UpdateMainUI();
   }
   else if (g_pplayer->m_cameraMode)
   {
      // Camera mode info text
      // this is not a normal UI aligned to the monitor orientation but an overlay used when playing, therefore it should be rotated like the playfield to face the user and only displays for right angles
      m_rotate = ((int)(g_pplayer->m_ptable->m_BG_rotation[g_pplayer->m_ptable->m_BG_current_set] / 90.0f)) & 3;
      if (m_rotate * 90.0f == g_pplayer->m_ptable->m_BG_rotation[g_pplayer->m_ptable->m_BG_current_set])
         UpdateCameraModeUI();
   }
   else
   {
      // Info tooltips
      // this is not a normal UI aligned to the monitor orientation but an overlay used when playing, therefore it should be rotated like the playfield to face the user and only displays for right angles
      m_rotate = ((int)(g_pplayer->m_ptable->m_BG_rotation[g_pplayer->m_ptable->m_BG_current_set] / 90.0f)) & 3;
      if (m_rotate * 90.0f == g_pplayer->m_ptable->m_BG_rotation[g_pplayer->m_ptable->m_BG_current_set])
      {
         if (g_pplayer->m_closing == Player::CS_PLAYING && (g_pplayer->m_stereo3D != STEREO_OFF && !g_pplayer->m_stereo3Denabled && (usec() < m_StartTime_usec + 4e+6))) // show for max. 4 seconds
         HelpSplash("3D Stereo is enabled but currently toggled off, press F10 to toggle 3D Stereo on", m_rotate);
         //!! visualize with real buttons or at least the areas?? Add extra buttons?
         if (g_pplayer->m_closing == Player::CS_PLAYING && g_pplayer->m_supportsTouch && g_pplayer->m_showTouchMessage && (usec() < m_StartTime_usec + 12e+6)) // show for max. 12 seconds
            HelpSplash("You can use Touch controls on this display: bottom left area to Start Game, bottom right area to use the Plunger\n"
                       "lower left/right for Flippers, upper left/right for Magna buttons, top left for Credits and (hold) top right to Exit",
               m_rotate);
      }
   }

   ImGui::EndFrame();
}

void LiveUI::UpdateCameraModeUI()
{
   // UI Context
   VPinball *m_app = g_pvp;
   Player *m_player = g_pplayer;
   PinTable *m_table = g_pplayer->m_ptable;
   Pin3D *m_pin3d = &(g_pplayer->m_pin3d);

   ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
   ImGui::SetNextWindowBgAlpha(0.35f);
   ImGui::SetNextWindowPos(ImVec2(10, 10));
   ImGui::Begin("CameraMode", NULL, window_flags);

   for (int i = 0; i < 14; i++)
   {
      if (m_player->m_cameraMode && (i == m_player->m_backdropSettingActive || (m_player->m_backdropSettingActive == 3 && (i == 4 || i == 5))))
         ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
      switch (i)
      {
      case 0: ImGui::Text("Inclination: %.3f", m_table->m_BG_inclination[m_table->m_BG_current_set]); break;
      case 1: ImGui::Text("Field Of View: %.3f", m_table->m_BG_FOV[m_table->m_BG_current_set]); break;
      case 2: ImGui::Text("Layback: %.3f", m_table->m_BG_layback[m_table->m_BG_current_set]); ImGui::NewLine(); break;
      case 4: ImGui::Text("X Scale: %.3f", m_table->m_BG_scalex[m_table->m_BG_current_set]); break;
      case 5: ImGui::Text("Y Scale: %.3f", m_table->m_BG_scaley[m_table->m_BG_current_set]); break;
      case 6: ImGui::Text("Z Scale: %.3f", m_table->m_BG_scalez[m_table->m_BG_current_set]); ImGui::NewLine(); break;
      case 7: ImGui::Text("X Offset: %.0f", m_table->m_BG_xlatex[m_table->m_BG_current_set]); break;
      case 8: ImGui::Text("Y Offset: %.0f", m_table->m_BG_xlatey[m_table->m_BG_current_set]); break;
      case 9: ImGui::Text("Z Offset: %.0f", m_table->m_BG_xlatez[m_table->m_BG_current_set]); ImGui::NewLine(); break;
      case 10: ImGui::Text("Light Emission Scale: %.0f", m_table->m_lightEmissionScale); break;
      case 11: ImGui::Text("Light Range: %.0f", m_table->m_lightRange); break;
      case 12: ImGui::Text("Light Height: %.0f", m_table->m_lightHeight); ImGui::NewLine(); break;
      case 13: ImGui::Text("Environment Emission: %.3f", m_table->m_envEmissionScale); break;
      }
      if (m_player->m_cameraMode && (i == m_player->m_backdropSettingActive || (m_player->m_backdropSettingActive == 3 && (i == 4 || i == 5))))
         ImGui::PopStyleColor();
   }

   ImGui::NewLine();

   ImGui::Text("Camera at X: %.2f Y: %.2f Z: %.2f,  Rotation: %.2f", -m_pin3d->m_proj.m_matView._41,
      (m_table->m_BG_current_set == 0 || m_table->m_BG_current_set == 2) ? m_pin3d->m_proj.m_matView._42 : -m_pin3d->m_proj.m_matView._42, m_pin3d->m_proj.m_matView._43,
      m_table->m_BG_rotation[m_table->m_BG_current_set]);

   if (m_player->m_cameraMode)
   {
      ImGui::NewLine();
      ImGui::Text("Left / Right flipper key = decrease / increase value highlighted in green");
      ImGui::Text("Left / Right magna save key = previous / next option");
      ImGui::NewLine();
      ImGui::Text("Left / Right nudge key = rotate table orientation (if enabled in the Key settings)");
      ImGui::Text("Navigate around with the Arrow Keys and Left Alt Key (if enabled in the Key settings)");
      if (m_app->m_povEdit)
         ImGui::Text("Start Key: export POV file and quit, or Credit Key: quit without export");
      else
         ImGui::Text("Start Key: reset POV to old values");
   }

   ImGui::End();
}


void LiveUI::UpdateMainUI()
{
   // UI Context
   VPinball *m_app = g_pvp;
   Player *m_player = g_pplayer;
   PinTable *m_table = g_pplayer->m_ptable;
   PinInput *m_pininput = &(g_pplayer->m_pininput);
   Pin3D *m_pin3d = &(g_pplayer->m_pin3d);

   enum UILocation
   {
      UI_ROOT,
      UI_CAMERA_SETTINGS,
      UI_AUDIO_SETTINGS,
      UI_VIDEO_SETTINGS,
      UI_RENDERER_INSPECTION,
      UI_HEADTRACKING,
   };
   static UILocation ui_pos = UI_ROOT;

   int fps_mode = 0;

   bool display_table_info = ui_pos == UI_ROOT;

   const char *title;
   ImGuiStyle &style = ImGui::GetStyle();
   style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0, 0, 0, !display_table_info && (ui_pos == UI_ROOT || ui_pos == UI_AUDIO_SETTINGS) ? 0.5f : 0.0f);
   switch (ui_pos)
   {
   case UI_ROOT: title = "Settings###In Game UI"; break;
   case UI_CAMERA_SETTINGS: title = "Settings > Camera Point of View###In Game UI"; break;
   case UI_AUDIO_SETTINGS: title = "Settings > Audio Options###In Game UI"; break;
   case UI_VIDEO_SETTINGS: title = "Settings > Video Options###In Game UI"; break;
   case UI_RENDERER_INSPECTION: title = "Settings > Renderer Inspection###In Game UI"; break;
   case UI_HEADTRACKING: title = "Settings > Head Tracking###In Game UI"; break;
   }

   // Display table name,author,version and blurb and description
   if (display_table_info)
   {
      std::ostringstream info;
      if (!m_table->m_szTableName.empty())
         info << m_table->m_szTableName;
      else
         info << "Table";
      if (!m_table->m_szAuthor.empty())
         info << " by " << m_table->m_szAuthor;
      if (!m_table->m_szVersion.empty())
         info << " (" << m_table->m_szVersion << ")";
      //info << std::format(" ({:s} Revision {:d})\n", !m_table->m_szDateSaved.empty() ? m_table->m_szDateSaved : "N.A.", m_table->m_numTimesSaved);
      info << " (" << (!m_table->m_szDateSaved.empty() ? m_table->m_szDateSaved : "N.A.") << " Revision " << m_table->m_numTimesSaved << ")\n";
      size_t line_length = info.str().size();
      info << std::string(line_length, '=') << "\n";
      if (!m_table->m_szBlurb.empty())
         info << m_table->m_szBlurb << std::string(line_length, '=') << "\n";
      if (!m_table->m_szDescription.empty())
         info << m_table->m_szDescription;
      ImGuiWindowFlags window_flags
         = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
      ImGui::SetNextWindowBgAlpha(0.5f);
      ImGui::SetNextWindowPos(ImVec2(0, 0));
      ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
      ImGui::Begin("Table Info", NULL, window_flags);
      HelpTextCentered(info.str().c_str());
      ImGui::End();
   }

   // Directly open the modal dialog at root level on user interaction
   static U32 openTime = 0;
   if (!ImGui::IsPopupOpen(title))
   {
      ui_pos = UI_ROOT;
      ImGui::OpenPopup(title);
      openTime = msec();
   }

   // Main UI window
   if (ImGui::BeginPopupModal(title, NULL, ImGuiWindowFlags_AlwaysAutoResize))
   {
      bool enableKeyboardShortcuts = (msec() - openTime) > 250;
      switch (ui_pos)
      {
      //////////////////////////////////////////////////////////////////////////
      // Root UI panel
      case UI_ROOT:
      {
         static U32 quitToEditor = 0; // Long press keyboard shortcut
         if (((ImGui::IsKeyDown(dikToImGuiKeys[m_player->m_rgKeys[eEscape]]) && !m_disable_esc)  || ImGui::IsKeyDown(dikToImGuiKeys[m_player->m_rgKeys[eExitGame]])))
         {
            if (quitToEditor == 0 && !enableKeyboardShortcuts)
               quitToEditor = msec();
         }
         else
            quitToEditor = 0;
         // Resume: click on the button, or press escape key
         if (ImGui::Button("Resume Game") || (enableKeyboardShortcuts && quitToEditor == 0 && ((ImGui::IsKeyPressed(dikToImGuiKeys[m_player->m_rgKeys[eEscape]]) && !m_disable_esc))))
         {
            ImGui::CloseCurrentPopup();
            m_ShowUI = false;
         }
         if (ImGui::Button("Camera Settings"))
            ui_pos = UI_CAMERA_SETTINGS;
         /* if (ImGui::Button("Audio Settings"))
               ui_pos = UI_AUDIO_SETTINGS;*/
         if (ImGui::Button("Video Settings"))
            ui_pos = UI_VIDEO_SETTINGS;
#ifdef ENABLE_BAM
         if (ImGui::Button("Head Tracking"))
         {
            m_player->m_infoMode = IF_BAM_MENU;
            ImGui::CloseCurrentPopup();
         }
#endif
         if (ImGui::Button("Renderer Inspection"))
            ui_pos = UI_RENDERER_INSPECTION;
         if (ImGui::Button("Debugger") || (enableKeyboardShortcuts && ImGui::IsKeyPressed(dikToImGuiKeys[m_player->m_rgKeys[eDebugger]])))
         {
            m_player->m_showDebugger = true;
            ImGui::CloseCurrentPopup();
            m_ShowUI = false;
         }
         // Quit: click on the button, or press exit button, or long press exit button / escape key
         if (ImGui::Button("Quit to editor") || (enableKeyboardShortcuts && ImGui::IsKeyPressed(dikToImGuiKeys[m_player->m_rgKeys[eExitGame]])) || (quitToEditor != 0 && (msec() - quitToEditor) > m_table->m_tblExitConfirm))
         {
            m_table->QuitPlayer(Player::CS_STOP_PLAY);
            ImGui::CloseCurrentPopup();
            m_ShowUI = false;
         }
         break;
      }

      //////////////////////////////////////////////////////////////////////////
      // Camera settings (point of view, import/export,...)
      case UI_CAMERA_SETTINGS:
      {
         static bool old_player_dynamic_mode = m_player->m_dynamicMode;
         static bool old_player_camera_mode = m_player->m_cameraMode;
         m_player->EnableStaticPrePass(false);

         if (!m_app->m_povEdit)
         {
            if (ImGui::Button("Reset"))
            {
               bool old_camera_mode = m_player->m_cameraMode;
               m_player->m_cameraMode = true;
               m_pininput->FireKeyEvent(DISPID_GameEvents_KeyDown, m_player->m_rgKeys[eStartGameKey]);
               m_player->m_cameraMode = old_camera_mode;
               m_pin3d->InitLights(); // Needed to update shaders with new light settings
               const vec4 st(m_table->m_envEmissionScale * m_player->m_globalEmissionScale,
                  m_pin3d->m_envTexture ? (float)m_pin3d->m_envTexture->m_height /*+m_pin3d->m_envTexture->m_width)*0.5f*/
                                        : (float)m_pin3d->m_builtinEnvTexture.m_height /*+m_pin3d->m_builtinEnvTexture.m_width)*0.5f*/,
                  0.f, 0.f);
               m_rd->basicShader->SetVector(SHADER_fenvEmissionScale_TexWidth, &st);
#ifdef SEPARATE_CLASSICLIGHTSHADER
               m_rd->classicLightShader->SetVector(SHADER_fenvEmissionScale_TexWidth, &st);
#endif
            }
            ImGui::SameLine();
            if (ImGui::Button("Import"))
            {
               m_table->ImportBackdropPOV(string());
               m_pin3d->InitLights(); // Needed to update shaders with new light settings
               const vec4 st(m_table->m_envEmissionScale * m_player->m_globalEmissionScale,
                  m_pin3d->m_envTexture ? (float)m_pin3d->m_envTexture->m_height /*+m_pin3d->m_envTexture->m_width)*0.5f*/
                                        : (float)m_pin3d->m_builtinEnvTexture.m_height /*+m_pin3d->m_builtinEnvTexture.m_width)*0.5f*/,
                  0.f, 0.f);
               m_rd->basicShader->SetVector(SHADER_fenvEmissionScale_TexWidth, &st);
#ifdef SEPARATE_CLASSICLIGHTSHADER
               m_rd->classicLightShader->SetVector(SHADER_fenvEmissionScale_TexWidth, &st);
#endif
            }
            ImGui::SameLine();
            if (ImGui::Button("Export"))
               m_table->ExportBackdropPOV(string());
            ImGui::SameLine();
         }
         if (ImGui::Button("Back"))
         {
            m_player->EnableStaticPrePass(!old_player_dynamic_mode);
            m_player->m_cameraMode = old_player_camera_mode;
            ui_pos = UI_ROOT;
         }

         ImGui::NewLine();

         ImGui::Checkbox("Interactive camera mode", &m_player->m_cameraMode);

         ImGui::NewLine();

         for (int i = 0; i < 14; i++)
         {
            if (m_player->m_cameraMode && (i == m_player->m_backdropSettingActive || (m_player->m_backdropSettingActive == 3 && (i == 4 || i == 5))))
               ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
            switch (i)
            {
            case 0:
               if (ImGui::InputFloat("Inclination", &(m_table->m_BG_inclination[m_table->m_BG_current_set]), 0.2f, 1.0f, "%.3f", ImGuiInputTextFlags_CharsDecimal))
                  m_table->SetNonUndoableDirty(eSaveDirty);
               break;
            case 1:
               if (ImGui::InputFloat("Field Of View", &(m_table->m_BG_FOV[m_table->m_BG_current_set]), 0.2f, 1.0f, "%.3f", ImGuiInputTextFlags_CharsDecimal))
                  m_table->SetNonUndoableDirty(eSaveDirty);
               break;
            case 2:
               if (ImGui::InputFloat("Layback", &(m_table->m_BG_layback[m_table->m_BG_current_set]), 0.2f, 1.0f, "%.3f", ImGuiInputTextFlags_CharsDecimal))
                  m_table->SetNonUndoableDirty(eSaveDirty);
               ImGui::NewLine();
               break;
            case 4:
               if (ImGui::InputFloat("X Scale", &(m_table->m_BG_scalex[m_table->m_BG_current_set]), 0.002f, 0.01f, "%.3f", ImGuiInputTextFlags_CharsDecimal))
                  m_table->SetNonUndoableDirty(eSaveDirty);
               break;
            case 5:
               if (ImGui::InputFloat("Y Scale", &(m_table->m_BG_scaley[m_table->m_BG_current_set]), 0.002f, 0.01f, "%.3f", ImGuiInputTextFlags_CharsDecimal))
                  m_table->SetNonUndoableDirty(eSaveDirty);
               break;
            case 6:
               if (ImGui::InputFloat("Z Scale", &(m_table->m_BG_scalez[m_table->m_BG_current_set]), 0.002f, 0.01f, "%.3f", ImGuiInputTextFlags_CharsDecimal))
                  m_table->SetNonUndoableDirty(eSaveDirty);
               ImGui::NewLine();
               break;
            case 7:
               if (ImGui::InputFloat("X Offset", &(m_table->m_BG_xlatex[m_table->m_BG_current_set]), 10.0f, 50.0f, "%.0f", ImGuiInputTextFlags_CharsDecimal))
                  m_table->SetNonUndoableDirty(eSaveDirty);
               break;
            case 8:
               if (ImGui::InputFloat("Y Offset", &(m_table->m_BG_xlatey[m_table->m_BG_current_set]), 10.0f, 50.0f, "%.0f", ImGuiInputTextFlags_CharsDecimal))
                  m_table->SetNonUndoableDirty(eSaveDirty);
               break;
            case 9:
               if (ImGui::InputFloat("Z Offset", &(m_table->m_BG_xlatez[m_table->m_BG_current_set]), 10.0f, 50.0f, "%.0f", ImGuiInputTextFlags_CharsDecimal))
                  m_table->SetNonUndoableDirty(eSaveDirty);
               ImGui::NewLine();
               break;
            case 10:
               if (ImGui::InputFloat("Light Emission Scale", &(m_table->m_lightEmissionScale), 20000.0f, 100000.0f, "%.0f", ImGuiInputTextFlags_CharsDecimal))
               {
                  m_table->SetNonUndoableDirty(eSaveDirty);
                  m_pin3d->InitLights(); // Needed to update shaders with new light settings
               }
               break;
            case 11:
               if (ImGui::InputFloat("Light Range", &(m_table->m_lightRange), 200.0f, 1000.0f, "%.0f", ImGuiInputTextFlags_CharsDecimal))
                  m_table->SetNonUndoableDirty(eSaveDirty);
               break;
            case 12:
               if (ImGui::InputFloat("Light Height", &(m_table->m_lightHeight), 20.0f, 100.0f, "%.0f", ImGuiInputTextFlags_CharsDecimal))
                  m_table->SetNonUndoableDirty(eSaveDirty);
               ImGui::NewLine();
               break;
            case 13:
               if (ImGui::InputFloat("Environment Emission", &(m_table->m_envEmissionScale), 0.1f, 0.5f, "%.3f", ImGuiInputTextFlags_CharsDecimal))
               {
                  m_table->SetNonUndoableDirty(eSaveDirty);
                  const vec4 st(m_table->m_envEmissionScale * m_player->m_globalEmissionScale,
                     m_pin3d->m_envTexture ? (float)m_pin3d->m_envTexture->m_height /*+m_pin3d->m_envTexture->m_width)*0.5f*/
                                           : (float)m_pin3d->m_builtinEnvTexture.m_height /*+m_pin3d->m_builtinEnvTexture.m_width)*0.5f*/,
                     0.f, 0.f);
                  m_rd->basicShader->SetVector(SHADER_fenvEmissionScale_TexWidth, &st);
#ifdef SEPARATE_CLASSICLIGHTSHADER
                  m_rd->classicLightShader->SetVector(SHADER_fenvEmissionScale_TexWidth, &st);
#endif
               }
               break;
            }
            if (m_player->m_cameraMode && (i == m_player->m_backdropSettingActive || (m_player->m_backdropSettingActive == 3 && (i == 4 || i == 5))))
               ImGui::PopStyleColor();
         }

         ImGui::NewLine();

         if (ImGui::InputFloat("Rotation", &(m_table->m_BG_rotation[m_table->m_BG_current_set]), 90.f, 90.0f, "%.0f", ImGuiInputTextFlags_CharsDecimal))
            m_table->SetNonUndoableDirty(eSaveDirty);

         ImGui::NewLine();

         ImGui::Text("Camera at X: %.2f Y: %.2f Z: %.2f,  Rotation: %.2f", -m_pin3d->m_proj.m_matView._41,
            (m_table->m_BG_current_set == 0 || m_table->m_BG_current_set == 2) ? m_pin3d->m_proj.m_matView._42 : -m_pin3d->m_proj.m_matView._42, m_pin3d->m_proj.m_matView._43,
            m_table->m_BG_rotation[m_table->m_BG_current_set]);

         if (m_player->m_cameraMode)
         {
            ImGui::NewLine();
            ImGui::Text("Left / Right flipper key = decrease / increase value highlighted in green");
            ImGui::Text("Left / Right magna save key = previous / next option");
            ImGui::NewLine();
            ImGui::Text("Left / Right nudge key = rotate table orientation (if enabled in the Key settings)");
            ImGui::Text("Navigate around with the Arrow Keys and Left Alt Key (if enabled in the Key settings)");
            if (m_app->m_povEdit)
               ImGui::Text("Start Key: export POV file and quit, or Credit Key: quit without export");
            else
               ImGui::Text("Start Key: reset POV to old values");
         }
         break;
      }

      //////////////////////////////////////////////////////////////////////////
      // Video options
      case UI_VIDEO_SETTINGS:
      {
         fps_mode = 1; // Show FPS while adjusting video options

         if (ImGui::Button("Back"))
            ui_pos = UI_ROOT;

         ImGui::NewLine();

         ImGui::Text("Global settings");

         ImGui::NewLine();

         if (ImGui::Checkbox("Force Bloom filter off", &m_player->m_bloomOff))
            SaveValueBool(regKey[m_player->m_stereo3D == STEREO_VR ? RegName::PlayerVR : RegName::Player], "ForceBloomOff"s, m_player->m_bloomOff);

         if (m_table->m_useFXAA == -1)
         {
            const char *postprocessed_aa_items[] = { "Disabled", "Fast FXAA", "Standard FXAA", "Quality FXAA", "Fast NFAA", "Standard DLAA", "Quality SMAA" };
            if (ImGui::Combo("Postprocessed AA", &m_player->m_FXAA, postprocessed_aa_items, IM_ARRAYSIZE(postprocessed_aa_items)))
               SaveValueInt(regKey[m_player->m_stereo3D == STEREO_VR ? RegName::PlayerVR : RegName::Player], "FXAA"s, m_player->m_FXAA);
         }

         const char *sharpen_items[] = { "Disabled", "CAS", "Bilateral CAS" };
         if (ImGui::Combo("Sharpen", &m_player->m_sharpen, sharpen_items, IM_ARRAYSIZE(sharpen_items)))
            SaveValueInt(regKey[m_player->m_stereo3D == STEREO_VR ? RegName::PlayerVR : RegName::Player], "Sharpen"s, m_player->m_sharpen);

         if (ImGui::Checkbox("Enable stereo rendering", &m_player->m_stereo3Denabled))
            SaveValueBool(regKey[RegName::Player], "Stereo3DEnabled"s, m_player->m_stereo3Denabled);

         ImGui::NewLine();

         ImGui::Text("Table settings");

         ImGui::NewLine();

         if (ImGui::InputFloat("Bloom Strength", &(m_table->m_bloom_strength), 0.1f, 1.0f, "%.3f", ImGuiInputTextFlags_CharsDecimal))
            m_table->SetNonUndoableDirty(eSaveDirty);

         break;
      }

      //////////////////////////////////////////////////////////////////////////
      // Renderer inspection (display individual render passes, performance indicators,...)
      case UI_RENDERER_INSPECTION:
      {
         if (ImGui::Button("Back"))
            ui_pos = UI_ROOT;

         ImGui::NewLine();

         static bool show_fps_plot = false;
         ImGui::Checkbox("Display FPS plots", &show_fps_plot);
         fps_mode = show_fps_plot ? 2 : 1;

         ImGui::NewLine();

         ImGui::Text("Display single render pass:");
         static int pass_selection = IF_FPS;
         ImGui::RadioButton("Disabled", &pass_selection, IF_FPS);
         ImGui::RadioButton("Profiler", &pass_selection, IF_PROFILING);
         ImGui::RadioButton("Profiler (Split rendering)", &pass_selection, IF_PROFILING_SPLIT_RENDERING);
         ImGui::RadioButton("Static prerender pass", &pass_selection, IF_STATIC_ONLY);
         ImGui::RadioButton("Dynamic render pass", &pass_selection, IF_DYNAMIC_ONLY);
         ImGui::RadioButton("Transmitted light pass", &pass_selection, IF_LIGHT_BUFFER_ONLY);
         if (m_player->GetAOMode() != 0)
            ImGui::RadioButton("Ambient Occlusion pass", &pass_selection, IF_AO_ONLY);
         for (int i = 0; i < 2 * m_table->m_vrenderprobe.size(); i++)
         {
            string name = m_table->m_vrenderprobe[i >> 1]->GetName() + ((i & 1) == 0 ? " - Static pass" : " - Dynamic pass");
            ImGui::RadioButton(name.c_str(), &pass_selection, 100 + i);
         }
         if (pass_selection < 100)
            m_player->m_infoMode = (InfoMode)pass_selection;
         else
         {
            m_player->m_infoMode = IF_RENDER_PROBES;
            m_player->m_infoProbeIndex = pass_selection - 100;
         }

         ImGui::NewLine();

         ImGui::Text(m_player->GetPerfInfo().c_str());
      }
      }

      ImGui::EndPopup();
   }

   // Display simple FPS window
   if (fps_mode > 0)
   {
      ImGuiWindowFlags window_flags
         = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
      ImGui::SetNextWindowBgAlpha(0.35f);
      ImGui::SetNextWindowPos(ImVec2(10, 10));
      ImGui::Begin("FPS", NULL, window_flags);
      const float fpsAvg = (m_player->m_fpsCount == 0) ? 0.0f : m_player->m_fpsAvg / (float)m_player->m_fpsCount;
      ImGui::Text("FPS: %.1f (%.1f avg)", m_player->m_fps + 0.01f, fpsAvg + 0.01f);
      ImGui::End();
   }

   // Display FPS window with plots
   if (fps_mode == 2)
   {
      ImGui::SetNextWindowSize(ImVec2(530, 550), ImGuiCond_FirstUseEver);
      ImGui::SetNextWindowPos(ImVec2((float)(m_player->m_wnd_width - 530 - 10), 10), ImGuiCond_FirstUseEver);
      ImGui::Begin("Plots");
      //!! This example assumes 60 FPS. Higher FPS requires larger buffer size.
      static ScrollingData sdata1, sdata2, sdata3, sdata4, sdata5, sdata6;
      //static RollingData   rdata1, rdata2;
      static double t = 0.f;
      t += ImGui::GetIO().DeltaTime;

      sdata6.AddPoint((float)t, float(1e-3 * m_player->m_script_period) * 1.f);
      sdata5.AddPoint((float)t, sdata5.GetLast().y * 0.95f + sdata6.GetLast().y * 0.05f);

      sdata4.AddPoint((float)t, float(1e-3 * (m_player->m_phys_period - m_player->m_script_period)) * 5.f);
      sdata3.AddPoint((float)t, sdata3.GetLast().y * 0.95f + sdata4.GetLast().y * 0.05f);

      sdata2.AddPoint((float)t, m_player->m_fps * 0.003f);
      //rdata2.AddPoint((float)t, m_fps * 0.003f);
      sdata1.AddPoint((float)t, sdata1.GetLast().y * 0.95f + sdata2.GetLast().y * 0.05f);
      //rdata1.AddPoint((float)t, sdata1.GetLast().y*0.95f + sdata2.GetLast().y*0.05f);

      static float history = 2.5f;
      ImGui::SliderFloat("History", &history, 1, 10, "%.1f s");
      //rdata1.Span = history;
      //rdata2.Span = history;
      constexpr int rt_axis = ImPlotAxisFlags_NoTickLabels;
      if (ImPlot::BeginPlot("##ScrollingFPS", ImVec2(-1, 150), ImPlotFlags_None))
      {
         ImPlot::SetupAxis(ImAxis_X1, nullptr, rt_axis);
         ImPlot::SetupAxis(ImAxis_Y1, nullptr, rt_axis | ImPlotAxisFlags_LockMin);
         ImPlot::SetupAxisLimits(ImAxis_X1, t - history, t, ImGuiCond_Always);
         ImPlot::PlotLine("FPS", &sdata2.Data[0].x, &sdata2.Data[0].y, sdata2.Data.size(), 0, sdata2.Offset, 2 * sizeof(float));
         ImPlot::PushStyleColor(ImPlotCol_Fill, ImVec4(1, 0, 0, 0.25f));
         ImPlot::PlotLine("Smoothed FPS", &sdata1.Data[0].x, &sdata1.Data[0].y, sdata1.Data.size(), 0, sdata1.Offset, 2 * sizeof(float));
         ImPlot::PopStyleColor();
         ImPlot::EndPlot();
      }
      /*
      if (ImPlot::BeginPlot("##RollingFPS", ImVec2(-1, 150), ImPlotFlags_Default)) {
         ImPlot::SetupAxis(ImAxis_X1, nullptr, rt_axis);
         ImPlot::SetupAxis(ImAxis_Y1, nullptr, rt_axis);
         ImPlot::SetupAxis(ImAxis_X1, 0, history, ImGuiCond_Always);
         ImPlot::PlotLine("Average FPS", &rdata1.Data[0].x, &rdata1.Data[0].y, rdata1.Data.size(), 0, 0, 2 * sizeof(float));
         ImPlot::PlotLine("FPS", &rdata2.Data[0].x, &rdata2.Data[0].y, rdata2.Data.size(), 0, 0, 2 * sizeof(float));
         ImPlot::EndPlot();
      }*/
      if (ImPlot::BeginPlot("##ScrollingPhysics", ImVec2(-1, 150), ImPlotFlags_None))
      {
         ImPlot::SetupAxis(ImAxis_X1, nullptr, rt_axis);
         ImPlot::SetupAxis(ImAxis_Y1, nullptr, rt_axis | ImPlotAxisFlags_LockMin);
         ImPlot::SetupAxisLimits(ImAxis_X1, t - history, t, ImGuiCond_Always);
         ImPlot::PlotLine("ms Physics", &sdata4.Data[0].x, &sdata4.Data[0].y, sdata4.Data.size(), 0, sdata4.Offset, 2 * sizeof(float));
         ImPlot::PushStyleColor(ImPlotCol_Fill, ImVec4(1, 0, 0, 0.25f));
         ImPlot::PlotLine("Smoothed ms Physics", &sdata3.Data[0].x, &sdata3.Data[0].y, sdata3.Data.size(), 0, sdata3.Offset, 2 * sizeof(float));
         ImPlot::PopStyleColor();
         ImPlot::EndPlot();
      }
      if (ImPlot::BeginPlot("##ScrollingScript", ImVec2(-1, 150), ImPlotFlags_None))
      {
         ImPlot::SetupAxis(ImAxis_X1, nullptr, rt_axis);
         ImPlot::SetupAxis(ImAxis_Y1, nullptr, rt_axis | ImPlotAxisFlags_LockMin);
         ImPlot::SetupAxisLimits(ImAxis_X1, t - history, t, ImGuiCond_Always);
         ImPlot::PlotLine("ms Script", &sdata6.Data[0].x, &sdata6.Data[0].y, sdata6.Data.size(), 0, sdata6.Offset, 2 * sizeof(float));
         ImPlot::PushStyleColor(ImPlotCol_Fill, ImVec4(1, 0, 0, 0.25f));
         ImPlot::PlotLine("Smoothed ms Script", &sdata5.Data[0].x, &sdata5.Data[0].y, sdata5.Data.size(), 0, sdata5.Offset, 2 * sizeof(float));
         ImPlot::PopStyleColor();
         ImPlot::EndPlot();
      }
      ImGui::End();
   }
}