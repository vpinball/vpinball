// license:BSD-3-Clause

#pragma once

#include <stdint.h>
#include <stdarg.h>

#ifdef _MSC_VER
#define PINMAMEAPI extern "C" __declspec(dllexport)
#define PINMAMECALLBACK __stdcall
#else
#define PINMAMEAPI extern "C" __attribute__((visibility("default")))
#define PINMAMECALLBACK
#endif

#define PINMAME_MAX_PATH 512
#define PINMAME_MAX_MECHSW 20
#define PINMAME_ACCUMULATOR_SAMPLES 8192 // from mixer.c

#define UINT32 uint32_t
#define UINT8  uint8_t
#include "pinmamedef.h"

typedef enum {
	PINMAME_LOG_LEVEL_DEBUG = 0,
	PINMAME_LOG_LEVEL_INFO = 1,
	PINMAME_LOG_LEVEL_ERROR = 2
} PINMAME_LOG_LEVEL;

typedef enum {
	PINMAME_STATUS_OK = 0,
	PINMAME_STATUS_CONFIG_NOT_SET = 1,
	PINMAME_STATUS_GAME_NOT_FOUND = 2,
	PINMAME_STATUS_GAME_ALREADY_RUNNING = 3,
	PINMAME_STATUS_EMULATOR_NOT_RUNNING = 4,
	PINMAME_STATUS_MECH_HANDLE_MECHANICS = 5,
	PINMAME_STATUS_MECH_NO_INVALID = 6
} PINMAME_STATUS;

typedef enum {
	PINMAME_FILE_TYPE_ROMS = 0,
	PINMAME_FILE_TYPE_NVRAM = 1,
	PINMAME_FILE_TYPE_SAMPLES = 2,
	PINMAME_FILE_TYPE_CONFIG = 3,
	PINMAME_FILE_TYPE_HIGHSCORE = 4
} PINMAME_FILE_TYPE;

typedef enum {
	PINMAME_DMD_MODE_BRIGHTNESS = 0,
	PINMAME_DMD_MODE_RAW = 1
} PINMAME_DMD_MODE;

typedef enum {
	PINMAME_SOUND_MODE_DEFAULT = 0,
	PINMAME_SOUND_MODE_ALTSOUND = 1
} PINMAME_SOUND_MODE;

typedef enum {
	PINMAME_AUDIO_FORMAT_INT16 = 0,
	PINMAME_AUDIO_FORMAT_FLOAT = 1
} PINMAME_AUDIO_FORMAT;

typedef enum {
	PINMAME_DISPLAY_TYPE_SEG16 = 0,                  // 16 segments
	PINMAME_DISPLAY_TYPE_SEG16R = 1,                 // 16 segments with comma and period reversed
	PINMAME_DISPLAY_TYPE_SEG10 = 2,                  // 9 segments and comma
	PINMAME_DISPLAY_TYPE_SEG9 = 3,                   // 9 segments
	PINMAME_DISPLAY_TYPE_SEG8 = 4,                   // 7 segments and comma
	PINMAME_DISPLAY_TYPE_SEG8D = 5,                  // 7 segments and period
	PINMAME_DISPLAY_TYPE_SEG7 = 6,                   // 7 segments
	PINMAME_DISPLAY_TYPE_SEG87 = 7,                  // 7 segments, comma every three
	PINMAME_DISPLAY_TYPE_SEG87F = 8,                 // 7 segments, forced comma every three
	PINMAME_DISPLAY_TYPE_SEG98 = 9,                  // 9 segments, comma every three
	PINMAME_DISPLAY_TYPE_SEG98F = 10,                // 9 segments, forced comma every three
	PINMAME_DISPLAY_TYPE_SEG7S = 11,                 // 7 segments, small
	PINMAME_DISPLAY_TYPE_SEG7SC = 12,                // 7 segments, small, with comma
	PINMAME_DISPLAY_TYPE_SEG16S = 13,                // 16 segments with split top and bottom line
	PINMAME_DISPLAY_TYPE_DMD = 14,                   // DMD Display
	PINMAME_DISPLAY_TYPE_VIDEO = 15,                 // VIDEO Display
	PINMAME_DISPLAY_TYPE_SEG16N = 16,                // 16 segments without commas
	PINMAME_DISPLAY_TYPE_SEG16D = 17,                // 16 segments with periods only
	PINMAME_DISPLAY_TYPE_SEGALL = 0x1f,              // maximum segment definition number
	PINMAME_DISPLAY_TYPE_IMPORT = 0x20,              // Link to another display layout
	PINMAME_DISPLAY_TYPE_SEGMASK = 0x3f,             // Note that CORE_IMPORT must be part of the segmask as well!
	PINMAME_DISPLAY_TYPE_SEGHIBIT = 0x40,
	PINMAME_DISPLAY_TYPE_SEGREV = 0x80,
	PINMAME_DISPLAY_TYPE_DMDNOAA = 0x100,
	PINMAME_DISPLAY_TYPE_NODISP = 0x200,
	PINMAME_DISPLAY_TYPE_DMDSEG = 0x400,
	PINMAME_DISPLAY_TYPE_SEG8H = PINMAME_DISPLAY_TYPE_SEG8 | PINMAME_DISPLAY_TYPE_SEGHIBIT,
	PINMAME_DISPLAY_TYPE_SEG7H = PINMAME_DISPLAY_TYPE_SEG7 | PINMAME_DISPLAY_TYPE_SEGHIBIT,
	PINMAME_DISPLAY_TYPE_SEG87H = PINMAME_DISPLAY_TYPE_SEG87 | PINMAME_DISPLAY_TYPE_SEGHIBIT,
	PINMAME_DISPLAY_TYPE_SEG87FH = PINMAME_DISPLAY_TYPE_SEG87F | PINMAME_DISPLAY_TYPE_SEGHIBIT,
	PINMAME_DISPLAY_TYPE_SEG7SH = PINMAME_DISPLAY_TYPE_SEG7S | PINMAME_DISPLAY_TYPE_SEGHIBIT,
	PINMAME_DISPLAY_TYPE_SEG7SCH = PINMAME_DISPLAY_TYPE_SEG7SC | PINMAME_DISPLAY_TYPE_SEGHIBIT
} PINMAME_DISPLAY_TYPE;

typedef enum {
	PINMAME_MOD_OUTPUT_TYPE_SOLENOID = 0,  // Solenoid output type
	PINMAME_MOD_OUTPUT_TYPE_LAMP = 1,      // Lamp output type
	PINMAME_MOD_OUTPUT_TYPE_GI = 2,        // Global Illumination output type
	PINMAME_MOD_OUTPUT_TYPE_ALPHASEG = 3,  // Alpha Numeric segment output type
} PINMAME_MOD_OUTPUT_TYPE;

typedef enum : uint64_t {
	PINMAME_HARDWARE_GEN_WPCALPHA_1 = 0x0000000000001,  // Alpha-numeric display S11 sound, Dr Dude 10/90
	PINMAME_HARDWARE_GEN_WPCALPHA_2 = 0x0000000000002,  // Alpha-numeric display,  - The Machine BOP 4/91
	PINMAME_HARDWARE_GEN_WPCDMD = 0x0000000000004,      // Dot Matrix Display, Terminator 2 7/91 - Party Zone 10/91
	PINMAME_HARDWARE_GEN_WPCFLIPTRON = 0x0000000000008, // Fliptronic flippers, Addams Family 2/92 - Twilight Zone 5/93
	PINMAME_HARDWARE_GEN_WPCDCS = 0x0000000000010,      // DCS Sound system, Indiana Jones 10/93 - Popeye 3/94
	PINMAME_HARDWARE_GEN_WPCSECURITY = 0x0000000000020, // Security chip, World Cup Soccer 3/94 - Jackbot 10/95
	PINMAME_HARDWARE_GEN_WPC95DCS = 0x0000000000040,    // Hybrid WPC95 driver + DCS sound, Who Dunnit
	PINMAME_HARDWARE_GEN_WPC95 = 0x0000000000080,       // Integrated boards, Congo 3/96 - Cactus Canyon 2/99
	PINMAME_HARDWARE_GEN_S11 = 0x0000080000000,         // No external sound board
	PINMAME_HARDWARE_GEN_S11X = 0x0000000000100,        // S11C sound board
	PINMAME_HARDWARE_GEN_S11A = PINMAME_HARDWARE_GEN_S11X,
	PINMAME_HARDWARE_GEN_S11B = PINMAME_HARDWARE_GEN_S11X,
	PINMAME_HARDWARE_GEN_S11B2 = 0x0000000000200,       // Jokerz! sound board
	PINMAME_HARDWARE_GEN_S11C = 0x0000000000400,        // No CPU board sound
	PINMAME_HARDWARE_GEN_S9 = 0x0000000000800,          // S9 CPU, 4x7+1x4
	PINMAME_HARDWARE_GEN_DE = 0x0000000001000,          // DE AlphaSeg
	PINMAME_HARDWARE_GEN_DEDMD16 = 0x0000000002000,     // DE 128x16
	PINMAME_HARDWARE_GEN_DEDMD32 = 0x0000000004000,     // DE 128x32
	PINMAME_HARDWARE_GEN_DEDMD64 = 0x0000000008000,     // DE 192x64
	PINMAME_HARDWARE_GEN_S7 = 0x0000000010000,          // S7 CPU
	PINMAME_HARDWARE_GEN_S6 = 0x0000000020000,          // S6 CPU
	PINMAME_HARDWARE_GEN_S4 = 0x0000000040000,          // S4 CPU
	PINMAME_HARDWARE_GEN_S3C = 0x0000000080000,         // S3 CPU No Chimes
	PINMAME_HARDWARE_GEN_S3 = 0x0000000100000,
	PINMAME_HARDWARE_GEN_BY17 = 0x0000000200000,
	PINMAME_HARDWARE_GEN_BY35 = 0x0000000400000,
	PINMAME_HARDWARE_GEN_STMPU100 = 0x0000000800000,    // Stern MPU - 100
	PINMAME_HARDWARE_GEN_STMPU200 = 0x0000001000000,    // Stern MPU - 200
	PINMAME_HARDWARE_GEN_ASTRO = 0x0000002000000,       // Unknown Astro game, Stern hardware
	PINMAME_HARDWARE_GEN_HNK = 0x0000004000000,         // Hankin
	PINMAME_HARDWARE_GEN_BYPROTO = 0x0000008000000,     // Bally Bow & Arrow prototype
	PINMAME_HARDWARE_GEN_BY6803 = 0x0000010000000,
	PINMAME_HARDWARE_GEN_BY6803A = 0x0000020000000,
	PINMAME_HARDWARE_GEN_BOWLING = 0x0000040000000,     // Big Ball Bowling, Stern hardware
	PINMAME_HARDWARE_GEN_GTS1 = 0x0000100000000,        // GTS1
	PINMAME_HARDWARE_GEN_GTS80 = 0x0000200000000,       // GTS80
	PINMAME_HARDWARE_GEN_GTS80A = PINMAME_HARDWARE_GEN_GTS80,
	PINMAME_HARDWARE_GEN_GTS80B = 0x0000400000000,      // GTS80B
	PINMAME_HARDWARE_GEN_WS = 0x0004000000000,          // Whitestar
	PINMAME_HARDWARE_GEN_WS_1 = 0x0008000000000,        // Whitestar with extra RAM
	PINMAME_HARDWARE_GEN_WS_2 = 0x0010000000000,        // Whitestar with extra DMD
	PINMAME_HARDWARE_GEN_GTS3 = 0x0020000000000,        // GTS3
	PINMAME_HARDWARE_GEN_ZAC1 = 0x0040000000000,
	PINMAME_HARDWARE_GEN_ZAC2 = 0x0080000000000,
	PINMAME_HARDWARE_GEN_SAM = 0x0100000000000,         // Stern SAM
	PINMAME_HARDWARE_GEN_ALVG = 0x0200000000000,        // Alvin G Hardware
	PINMAME_HARDWARE_GEN_ALVG_DMD2 = 0x0400000000000,   // Alvin G Hardware, with more shades
	PINMAME_HARDWARE_GEN_MRGAME = 0x0800000000000,      // Mr.Game Hardware
	PINMAME_HARDWARE_GEN_SLEIC = 0x1000000000000,       // Sleic Hardware
	PINMAME_HARDWARE_GEN_WICO = 0x2000000000000,        // Wico Hardware
	PINMAME_HARDWARE_GEN_SPA = 0x4000000000000,         // Stern PA
	PINMAME_HARDWARE_GEN_ALLWPC = 0x00000000000ff,      // All WPC
	PINMAME_HARDWARE_GEN_ALLS11 = 0x000008000ff00,      // All Sys11
	PINMAME_HARDWARE_GEN_ALLBY35 = 0x0000047e00000,     // All Bally35 and derivatives
	PINMAME_HARDWARE_GEN_ALLS80 = 0x0000600000000,      // All GTS80
	PINMAME_HARDWARE_GEN_LLWS = 0x001c000000000,       // All Whitestar
} PINMAME_HARDWARE_GEN;

typedef enum {
	PINMAME_GAME_DRIVER_FLAGS_ORIENTATION_MASK = 0x0007,
	PINMAME_GAME_DRIVER_FLAGS_ORIENTATION_FLIP_X = 0x0001,          // mirror everything in the X direction
	PINMAME_GAME_DRIVER_FLAGS_ORIENTATION_FLIP_Y = 0x0002,          // mirror everything in the Y direction
	PINMAME_GAME_DRIVER_FLAGS_ORIENTATION_SWAP_XY = 0x0004,         // mirror along the top-left/bottom-right diagonal
	PINMAME_GAME_DRIVER_FLAGS_GAME_NOT_WORKING = 0x0008,
	PINMAME_GAME_DRIVER_FLAGS_GAME_UNEMULATED_PROTECTION = 0x0010,  // game's protection not fully emulated
	PINMAME_GAME_DRIVER_FLAGS_GAME_WRONG_COLORS = 0x0020,           // colors are totally wrong
	PINMAME_GAME_DRIVER_FLAGS_GAME_IMPERFECT_COLORS = 0x0040,       // colors are not 100% accurate, but close
	PINMAME_GAME_DRIVER_FLAGS_GAME_IMPERFECT_GRAPHICS = 0x0080,     // graphics are wrong/incomplete
	PINMAME_GAME_DRIVER_FLAGS_GAME_NO_COCKTAIL = 0x0100,            // screen flip support is missing
	PINMAME_GAME_DRIVER_FLAGS_GAME_NO_SOUND = 0x0200,               // sound is missing
	PINMAME_GAME_DRIVER_FLAGS_GAME_IMPERFECT_SOUND = 0x0400,        // sound is known to be wrong
	PINMAME_GAME_DRIVER_FLAGS_NOT_A_DRIVER = 0x4000,                // set by the fake "root" driver_0 and by "containers"
} PINMAME_GAME_DRIVER_FLAGS;

typedef enum {
	PINMAME_MECH_FLAGS_LINEAR = 0x00,
	PINMAME_MECH_FLAGS_NONLINEAR = 0x01,
	PINMAME_MECH_FLAGS_CIRCLE = 0x00,
	PINMAME_MECH_FLAGS_STOPEND = 0x02,
	PINMAME_MECH_FLAGS_REVERSE = 0x04,
	PINMAME_MECH_FLAGS_ONESOL = 0x00,
	PINMAME_MECH_FLAGS_ONEDIRSOL = 0x10,
	PINMAME_MECH_FLAGS_TWODIRSOL = 0x20,
	PINMAME_MECH_FLAGS_TWOSTEPSOL = 0x40,
	PINMAME_MECH_FLAGS_FOURSTEPSOL = (PINMAME_MECH_FLAGS_TWODIRSOL | PINMAME_MECH_FLAGS_TWOSTEPSOL),
	PINMAME_MECH_FLAGS_SLOW = 0x00,
	PINMAME_MECH_FLAGS_FAST = 0x80,
	PINMAME_MECH_FLAGS_STEPSW = 0x00,
	PINMAME_MECH_FLAGS_LENGTHSW = 0x100
} PINMAME_MECH_FLAGS;

typedef enum : unsigned int {
	PINMAME_KEYCODE_A = 0,
	PINMAME_KEYCODE_B = 1,
	PINMAME_KEYCODE_C = 2,
	PINMAME_KEYCODE_D = 3,
	PINMAME_KEYCODE_E = 4,
	PINMAME_KEYCODE_F = 5,
	PINMAME_KEYCODE_G = 6,
	PINMAME_KEYCODE_H = 7,
	PINMAME_KEYCODE_I = 8,
	PINMAME_KEYCODE_J = 9,
	PINMAME_KEYCODE_K = 10,
	PINMAME_KEYCODE_L = 11,
	PINMAME_KEYCODE_M = 12,
	PINMAME_KEYCODE_N = 13,
	PINMAME_KEYCODE_O = 14,
	PINMAME_KEYCODE_P = 15,
	PINMAME_KEYCODE_Q = 16,
	PINMAME_KEYCODE_R = 17,
	PINMAME_KEYCODE_S = 18,
	PINMAME_KEYCODE_T = 19,
	PINMAME_KEYCODE_U = 20,
	PINMAME_KEYCODE_V = 21,
	PINMAME_KEYCODE_W = 22,
	PINMAME_KEYCODE_X = 23,
	PINMAME_KEYCODE_Y = 24,
	PINMAME_KEYCODE_Z = 25,
	PINMAME_KEYCODE_NUMBER_0 = 26,
	PINMAME_KEYCODE_NUMBER_1 = 27,
	PINMAME_KEYCODE_NUMBER_2 = 28,
	PINMAME_KEYCODE_NUMBER_3 = 29,
	PINMAME_KEYCODE_NUMBER_4 = 30,
	PINMAME_KEYCODE_NUMBER_5 = 31,
	PINMAME_KEYCODE_NUMBER_6 = 32,
	PINMAME_KEYCODE_NUMBER_7 = 33,
	PINMAME_KEYCODE_NUMBER_8 = 34,
	PINMAME_KEYCODE_NUMBER_9 = 35,
	PINMAME_KEYCODE_KEYPAD_0 = 36,
	PINMAME_KEYCODE_KEYPAD_1 = 37,
	PINMAME_KEYCODE_KEYPAD_2 = 38,
	PINMAME_KEYCODE_KEYPAD_3 = 39,
	PINMAME_KEYCODE_KEYPAD_4 = 40,
	PINMAME_KEYCODE_KEYPAD_5 = 41,
	PINMAME_KEYCODE_KEYPAD_6 = 42,
	PINMAME_KEYCODE_KEYPAD_7 = 43,
	PINMAME_KEYCODE_KEYPAD_8 = 44,
	PINMAME_KEYCODE_KEYPAD_9 = 45,
	PINMAME_KEYCODE_F1 = 46,
	PINMAME_KEYCODE_F2 = 47,
	PINMAME_KEYCODE_F3 = 48,
	PINMAME_KEYCODE_F4 = 49,
	PINMAME_KEYCODE_F5 = 50,
	PINMAME_KEYCODE_F6 = 51,
	PINMAME_KEYCODE_F7 = 52,
	PINMAME_KEYCODE_F8 = 53,
	PINMAME_KEYCODE_F9 = 54,
	PINMAME_KEYCODE_F10 = 55,
	PINMAME_KEYCODE_F11 = 56,
	PINMAME_KEYCODE_F12 = 57,
	PINMAME_KEYCODE_ESCAPE = 58,
	PINMAME_KEYCODE_GRAVE_ACCENT = 59,
	PINMAME_KEYCODE_MINUS = 60,
	PINMAME_KEYCODE_EQUALS = 61,
	PINMAME_KEYCODE_BACKSPACE = 62,
	PINMAME_KEYCODE_TAB = 63,
	PINMAME_KEYCODE_LEFT_BRACKET = 64,
	PINMAME_KEYCODE_RIGHT_BRACKET = 65,
	PINMAME_KEYCODE_ENTER = 66,
	PINMAME_KEYCODE_SEMICOLON = 67,
	PINMAME_KEYCODE_QUOTE = 68,
	PINMAME_KEYCODE_BACKSLASH = 69,
	PINMAME_KEYCODE_COMMA = 71,
	PINMAME_KEYCODE_PERIOD = 72,
	PINMAME_KEYCODE_SLASH = 73,
	PINMAME_KEYCODE_SPACE = 74,
	PINMAME_KEYCODE_INSERT = 75,
	PINMAME_KEYCODE_DELETE = 76,
	PINMAME_KEYCODE_HOME = 77,
	PINMAME_KEYCODE_END = 78,
	PINMAME_KEYCODE_PAGE_UP = 79,
	PINMAME_KEYCODE_PAGE_DOWN = 80,
	PINMAME_KEYCODE_LEFT = 81,
	PINMAME_KEYCODE_RIGHT = 82,
	PINMAME_KEYCODE_UP = 83,
	PINMAME_KEYCODE_DOWN = 84,
	PINMAME_KEYCODE_KEYPAD_DIVIDE = 85,
	PINMAME_KEYCODE_KEYPAD_MULTIPLY = 86,
	PINMAME_KEYCODE_KEYPAD_SUBTRACT = 87,
	PINMAME_KEYCODE_KEYPAD_ADD = 88,
	PINMAME_KEYCODE_KEYPAD_ENTER = 90,
	PINMAME_KEYCODE_PRINT_SCREEN = 91,
	PINMAME_KEYCODE_PAUSE = 92,
	PINMAME_KEYCODE_LEFT_SHIFT = 93,
	PINMAME_KEYCODE_RIGHT_SHIFT = 94,
	PINMAME_KEYCODE_LEFT_CONTROL = 95,
	PINMAME_KEYCODE_RIGHT_CONTROL = 96,
	PINMAME_KEYCODE_LEFT_ALT = 97,
	PINMAME_KEYCODE_RIGHT_ALT = 98,
	PINMAME_KEYCODE_SCROLL_LOCK = 99,
	PINMAME_KEYCODE_NUM_LOCK = 100,
	PINMAME_KEYCODE_CAPS_LOCK = 101,
	PINMAME_KEYCODE_LEFT_SUPER = 102,
	PINMAME_KEYCODE_RIGHT_SUPER = 103,
	PINMAME_KEYCODE_MENU = 104
} PINMAME_KEYCODE;

typedef struct {
	const char* name;
	const char* clone_of;
	const char* description;
	const char* year;
	const char* manufacturer;
	uint32_t flags;
	int32_t found;
} PinmameGame;

typedef struct {
	PINMAME_DISPLAY_TYPE type;
	int32_t top;
	int32_t left;
	int32_t length;
	int32_t width;
	int32_t height;
	int32_t depth;
} PinmameDisplayLayout;

typedef struct {
	PINMAME_AUDIO_FORMAT format;
	int channels;
	double sampleRate;
	double framesPerSecond;
	int samplesPerFrame;
	int bufferSize;
} PinmameAudioInfo;

typedef struct {
	int swNo;
	int state;
} PinmameSwitchState;

typedef struct {
	int solNo;
	int state;
} PinmameSolenoidState;

typedef struct {
	int lampNo;
	int state;
} PinmameLampState;

typedef struct {
	int giNo;
	int state;
} PinmameGIState;

typedef struct {
	int ledNo;
	int chgSeg;
	int state;
} PinmameLEDState;

typedef struct {
	int swNo;
	int startPos;
	int endPos;
	int pulse;
} PinmameMechSwitchConfig;

typedef struct {
	int type;
	int sol1;
	int sol2;
	int length;
	int steps;
	int initialPos;
	int acc;
	int ret;
	PinmameMechSwitchConfig sw[PINMAME_MAX_MECHSW];
} PinmameMechConfig;

typedef struct {
	int type;
	int length;
	int steps;
	int pos;
	int speed;
} PinmameMechInfo;

typedef struct {
	int sndNo;
} PinmameSoundCommand;

typedef struct {
	int nvramNo;
	uint8_t oldStat;
	uint8_t currStat;
} PinmameNVRAMState;

typedef struct {
	const char* name;
	PINMAME_KEYCODE code;
	unsigned int standardcode;
} PinmameKeyboardInfo;

typedef void (PINMAMECALLBACK *PinmameGameCallback)(PinmameGame* p_game, void* const p_userData);
typedef void (PINMAMECALLBACK *PinmameOnStateUpdatedCallback)(int state, void* const p_userData);
typedef void (PINMAMECALLBACK *PinmameOnDisplayAvailableCallback)(int index, int displayCount, PinmameDisplayLayout* p_displayLayout, void* const p_userData);
typedef void (PINMAMECALLBACK *PinmameOnDisplayUpdatedCallback)(int index, void* p_displayData, PinmameDisplayLayout* p_displayLayout, void* const p_userData);
typedef int (PINMAMECALLBACK *PinmameOnAudioAvailableCallback)(PinmameAudioInfo* p_audioInfo, void* const p_userData);
typedef int (PINMAMECALLBACK *PinmameOnAudioUpdatedCallback)(void* p_buffer, int samples, void* const p_userData);
typedef void (PINMAMECALLBACK *PinmameOnMechAvailableCallback)(int mechNo, PinmameMechInfo* p_mechInfo, void* const p_userData);
typedef void (PINMAMECALLBACK *PinmameOnMechUpdatedCallback)(int mechNo, PinmameMechInfo* p_mechInfo, void* const p_userData);
typedef void (PINMAMECALLBACK *PinmameOnSolenoidUpdatedCallback)(PinmameSolenoidState* p_solenoidState, void* const p_userData);
typedef void (PINMAMECALLBACK *PinmameOnConsoleDataUpdatedCallback)(void* p_data, int size, void* const p_userData);
typedef int (PINMAMECALLBACK *PinmameIsKeyPressedFunction)(PINMAME_KEYCODE keycode, void* const p_userData);
typedef void (PINMAMECALLBACK *PinmameOnLogMessageCallback)(PINMAME_LOG_LEVEL logLevel, const char* format, va_list args, void* const p_userData);
typedef void (PINMAMECALLBACK *PinmameOnSoundCommandCallback)(int boardNo, int cmd, void* const p_userData);

typedef struct {
	const PINMAME_AUDIO_FORMAT audioFormat;
	const int sampleRate;
	const char vpmPath[PINMAME_MAX_PATH];
	PinmameOnStateUpdatedCallback cb_OnStateUpdated;
	PinmameOnDisplayAvailableCallback cb_OnDisplayAvailable;
	PinmameOnDisplayUpdatedCallback cb_OnDisplayUpdated;
	PinmameOnAudioAvailableCallback cb_OnAudioAvailable;
	PinmameOnAudioUpdatedCallback cb_OnAudioUpdated;
	PinmameOnMechAvailableCallback cb_OnMechAvailable;
	PinmameOnMechUpdatedCallback cb_OnMechUpdated;
	PinmameOnSolenoidUpdatedCallback cb_OnSolenoidUpdated;
	PinmameOnConsoleDataUpdatedCallback cb_OnConsoleDataUpdated;
	PinmameIsKeyPressedFunction fn_IsKeyPressed;
	PinmameOnLogMessageCallback cb_OnLogMessage;
	PinmameOnSoundCommandCallback cb_OnSoundCommand;
} PinmameConfig;

PINMAMEAPI PINMAME_STATUS PinmameGetGame(const char* const p_name, PinmameGameCallback callback, void* const p_userData);
PINMAMEAPI PINMAME_STATUS PinmameGetGames(PinmameGameCallback callback, void* const p_userData);
PINMAMEAPI void PinmameSetConfig(const PinmameConfig* const p_config);
PINMAMEAPI void PinmameSetPath(const PINMAME_FILE_TYPE fileType, const char* const p_path);
PINMAMEAPI int PinmameGetCheat();
PINMAMEAPI void PinmameSetCheat(const int cheat);
PINMAMEAPI int PinmameGetHandleKeyboard();
PINMAMEAPI void PinmameSetHandleKeyboard(const int handleKeyboard);
PINMAMEAPI int PinmameGetHandleMechanics();
PINMAMEAPI void PinmameSetHandleMechanics(const int handleMechanics);
PINMAMEAPI PINMAME_DMD_MODE PinmameGetDmdMode();
PINMAMEAPI void PinmameSetDmdMode(const PINMAME_DMD_MODE dmdMode);
PINMAMEAPI PINMAME_SOUND_MODE PinmameGetSoundMode();
PINMAMEAPI void PinmameSetSoundMode(const PINMAME_SOUND_MODE soundMode);
PINMAMEAPI PINMAME_STATUS PinmameRun(const char* const p_name);
PINMAMEAPI int PinmameIsRunning();
PINMAMEAPI PINMAME_STATUS PinmamePause(const int pause);
PINMAMEAPI int PinmameIsPaused();
PINMAMEAPI PINMAME_STATUS PinmameReset();
PINMAMEAPI void PinmameStop();
PINMAMEAPI PINMAME_HARDWARE_GEN PinmameGetHardwareGen();
PINMAMEAPI int PinmameGetSwitch(const int swNo);
PINMAMEAPI void PinmameSetSwitch(const int swNo, const int state);
PINMAMEAPI void PinmameSetSwitches(const PinmameSwitchState* const p_states, const int numSwitches);
PINMAMEAPI uint32_t PinmameGetSolenoidMask(const int low);
PINMAMEAPI void PinmameSetSolenoidMask(const int low, const uint32_t mask);
PINMAMEAPI PINMAME_MOD_OUTPUT_TYPE PinmameGetModOutputType(const int output, const int no);
PINMAMEAPI void PinmameSetModOutputType(const int output, const int no, const PINMAME_MOD_OUTPUT_TYPE type);
PINMAMEAPI void PinmameSetTimeFence(const double timeInS);
PINMAMEAPI int PinmameGetMaxSolenoids();
PINMAMEAPI int PinmameGetSolenoid(const int solNo);
PINMAMEAPI int PinmameGetChangedSolenoids(PinmameSolenoidState* const p_changedStates);
PINMAMEAPI int PinmameGetMaxLamps();
PINMAMEAPI int PinmameGetLamp(const int lampNo);
PINMAMEAPI int PinmameGetChangedLamps(PinmameLampState* const p_changedStates);
PINMAMEAPI int PinmameGetMaxGIs();
PINMAMEAPI int PinmameGetGI(const int giNo);
PINMAMEAPI int PinmameGetChangedGIs(PinmameGIState* const p_changedStates);
PINMAMEAPI int PinmameGetMaxLEDs();
PINMAMEAPI int PinmameGetChangedLEDs(const uint64_t mask, const uint64_t, PinmameLEDState* const p_changedStates);
PINMAMEAPI int PinmameGetMaxMechs();
PINMAMEAPI int PinmameGetMech(const int mechNo);
PINMAMEAPI PINMAME_STATUS PinmameSetMech(const int mechNo, const PinmameMechConfig* const p_mechConfig);
PINMAMEAPI int PinmameGetMaxSoundCommands();
PINMAMEAPI int PinmameGetNewSoundCommands(PinmameSoundCommand* const p_newCommands);
PINMAMEAPI int PinmameGetDIP(const int dipBank);
PINMAMEAPI void PinmameSetDIP(const int dipBank, const int value);
PINMAMEAPI int PinmameGetMaxNVRAM();
PINMAMEAPI int PinmameGetNVRAM(PinmameNVRAMState* const p_nvramStates);
PINMAMEAPI int PinmameGetChangedNVRAM(PinmameNVRAMState* const p_nvramStates);
PINMAMEAPI void PinmameSetUserData(void* const p_userData);
PINMAMEAPI int PinmameGetStateBlock(const unsigned int updateMask, pinmame_tMachineOutputState** pp_outputState);

