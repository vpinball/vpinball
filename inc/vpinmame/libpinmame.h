// license:BSD-3-Clause

#ifndef LIBPINMAME_H
#define LIBPINMAME_H

#include <stdint.h>

#if defined(_WIN32) || defined(_WIN64)
#define LIBPINMAME_API extern "C" __declspec(dllexport)
#define CALLBACK __stdcall
#define strcasecmp _stricmp
#else
#define LIBPINMAME_API extern "C" __attribute__((visibility("default")))
#define CALLBACK
#endif

#define MAX_PATH 512
#define MAX_DISPLAYS 50
#define MAX_MECHSW 20

#define ACCUMULATOR_SAMPLES 8192 // from mixer.c

typedef enum {
	OK = 0,
	CONFIG_NOT_SET = 1,
	GAME_NOT_FOUND = 2,
	GAME_ALREADY_RUNNING = 3,
	EMULATOR_NOT_RUNNING = 4,
	MECH_HANDLE_MECHANICS = 5,
	MECH_NO_INVALID = 6
} PINMAME_STATUS;

typedef enum : int {
	AUDIO_FORMAT_INT16 = 0,
	AUDIO_FORMAT_FLOAT = 1
} PINMAME_AUDIO_FORMAT;

typedef enum {
	SEG16 = 0,                  // 16 segments
	SEG16R = 1,                 // 16 segments with comma and period reversed
	SEG10 = 2,                  // 9 segments and comma
	SEG9 = 3,                   // 9 segments
	SEG8 = 4,                   // 7 segments and comma
	SEG8D = 5,                  // 7 segments and period
	SEG7 = 6,                   // 7 segments
	SEG87 = 7,                  // 7 segments, comma every three
	SEG87F = 8,                 // 7 segments, forced comma every three
	SEG98 = 9,                  // 9 segments, comma every three
	SEG98F = 10,                // 9 segments, forced comma every three
	SEG7S = 11,                 // 7 segments, small
	SEG7SC = 12,                // 7 segments, small, with comma
	SEG16S = 13,                // 16 segments with split top and bottom line
	DMD = 14,                   // DMD Display
	VIDEO = 15,                 // VIDEO Display
	SEG16N = 16,                // 16 segments without commas
	SEG16D = 17,                // 16 segments with periods only
	SEGALL = 0x1f,              // maximum segment definition number
	IMPORT = 0x20,              // Link to another display layout
	SEGMASK = 0x3f,             // Note that CORE_IMPORT must be part of the segmask as well!
	SEGHIBIT = 0x40,
	SEGREV = 0x80,
	DMDNOAA = 0x100,
	NODISP = 0x200,
	SEG8H = SEG8 | SEGHIBIT,
	SEG7H = SEG7 | SEGHIBIT,
	SEG87H = SEG87 | SEGHIBIT,
	SEG87FH = SEG87F | SEGHIBIT,
	SEG7SH = SEG7S | SEGHIBIT,
	SEG7SCH = SEG7SC | SEGHIBIT
} PINMAME_DISPLAY_TYPE;

typedef enum : uint64_t {
	WPCALPHA_1 = 0x0000000000001,  // Alpha-numeric display S11 sound, Dr Dude 10/90
	WPCALPHA_2 = 0x0000000000002,  // Alpha-numeric display,  - The Machine BOP 4/91
	WPCDMD = 0x0000000000004,      // Dot Matrix Display, Terminator 2 7/91 - Party Zone 10/91
	WPCFLIPTRON = 0x0000000000008, // Fliptronic flippers, Addams Family 2/92 - Twilight Zone 5/93
	WPCDCS = 0x0000000000010,      // DCS Sound system, Indiana Jones 10/93 - Popeye 3/94
	WPCSECURITY = 0x0000000000020, // Security chip, World Cup Soccer 3/94 - Jackbot 10/95
	WPC95DCS = 0x0000000000040,    // Hybrid WPC95 driver + DCS sound, Who Dunnit
	WPC95 = 0x0000000000080,       // Integrated boards, Congo 3/96 - Cactus Canyon 2/99
	S11 = 0x0000080000000,         // No external sound board
	S11X = 0x0000000000100,        // S11C sound board
	S11A = S11X,
	S11B = S11X,
	S11B2 = 0x0000000000200,       // Jokerz! sound board
	S11C = 0x0000000000400,        // No CPU board sound
	S9 = 0x0000000000800,          // S9 CPU, 4x7+1x4
	DE = 0x0000000001000,          // DE AlphaSeg
	DEDMD16 = 0x0000000002000,     // DE 128x16
	DEDMD32 = 0x0000000004000,     // DE 128x32
	DEDMD64 = 0x0000000008000,     // DE 192x64
	S7 = 0x0000000010000,          // S7 CPU
	S6 = 0x0000000020000,          // S6 CPU
	S4 = 0x0000000040000,          // S4 CPU
	S3C = 0x0000000080000,         // S3 CPU No Chimes
	S3 = 0x0000000100000,
	BY17 = 0x0000000200000,
	BY35 = 0x0000000400000,
	STMPU100 = 0x0000000800000,    // Stern MPU - 100
	STMPU200 = 0x0000001000000,    // Stern MPU - 200
	ASTRO = 0x0000002000000,       // Unknown Astro game, Stern hardware
	HNK = 0x0000004000000,         // Hankin
	BYPROTO = 0x0000008000000,     // Bally Bow & Arrow prototype
	BY6803 = 0x0000010000000,
	BY6803A = 0x0000020000000,
	BOWLING = 0x0000040000000,     // Big Ball Bowling, Stern hardware
	GTS1 = 0x0000100000000,        // GTS1
	GTS80 = 0x0000200000000,       // GTS80
	GTS80A = GTS80,
	GTS80B = 0x0000400000000,      // GTS80B
	WS = 0x0004000000000,          // Whitestar
	WS_1 = 0x0008000000000,        // Whitestar with extra RAM
	WS_2 = 0x0010000000000,        // Whitestar with extra DMD
	GTS3 = 0x0020000000000,        // GTS3
	ZAC1 = 0x0040000000000,
	ZAC2 = 0x0080000000000,
	SAM = 0x0100000000000,         // Stern SAM
	ALVG = 0x0200000000000,        // Alvin G Hardware
	ALVG_DMD2 = 0x0400000000000,   // Alvin G Hardware, with more shades
	MRGAME = 0x0800000000000,      // Mr.Game Hardware
	SLEIC = 0x1000000000000,       // Sleic Hardware
	WICO = 0x2000000000000,        // Wico Hardware
	SPA = 0x4000000000000,         // Stern PA
	ALLWPC = 0x00000000000ff,      // All WPC
	ALLS11 = 0x000008000ff00,      // All Sys11
	ALLBY35 = 0x0000047e00000,     // All Bally35 and derivatives
	ALLS80 = 0x0000600000000,      // All GTS80
	ALLWS = 0x001c000000000,       // All Whitestar
} PINMAME_HARDWARE_GEN;

typedef enum {
	ORIENTATION_MASK = 0x0007,
	ORIENTATION_FLIP_X = 0x0001,          // mirror everything in the X direction
	ORIENTATION_FLIP_Y = 0x0002,          // mirror everything in the Y direction
	ORIENTATION_SWAP_XY = 0x0004,         // mirror along the top-left/bottom-right diagonal
	GAME_NOT_WORKING = 0x0008,
	GAME_UNEMULATED_PROTECTION = 0x0010,  // game's protection not fully emulated
	GAME_WRONG_COLORS = 0x0020,           // colors are totally wrong
	GAME_IMPERFECT_COLORS = 0x0040,       // colors are not 100% accurate, but close
	GAME_IMPERFECT_GRAPHICS = 0x0080,     // graphics are wrong/incomplete
	GAME_NO_COCKTAIL = 0x0100,            // screen flip support is missing
	GAME_NO_SOUND = 0x0200,               // sound is missing
	GAME_IMPERFECT_SOUND = 0x0400,        // sound is known to be wrong
	NOT_A_DRIVER = 0x4000,                // set by the fake "root" driver_0 and by "containers"
} PINMAME_GAME_DRIVER_FLAGS;

typedef enum {
	LINEAR = 0x00, 
	NONLINEAR = 0x01,
	CIRCLE = 0x00,
	STOPEND = 0x02,
	REVERSE = 0x04,
	ONESOL = 0x00,
	ONEDIRSOL = 0x10,
	TWODIRSOL = 0x20,
	TWOSTEPSOL = 0x40,
	FOURSTEPSOL = (TWODIRSOL | TWOSTEPSOL),
	SLOW = 0x00,
	FAST = 0x80,
	STEPSW = 0x00,
	LENGTHSW = 0x100
} PINMAME_MECH_FLAGS;

typedef enum : unsigned int {
	A = 0,
	B = 1,
	C = 2,
	D = 3,
	E = 4,
	F = 5,
	G = 6,
	H = 7,
	I = 8,
	J = 9,
	K = 10,
	L = 11,
	M = 12,
	N = 13,
	O = 14,
	P = 15,
	Q = 16,
	R = 17,
	S = 18,
	T = 19,
	U = 20,
	V = 21,
	W = 22,
	X = 23,
	Y = 24,
	Z = 25,
	NUMBER_0 = 26,
	NUMBER_1 = 27,
	NUMBER_2 = 28,
	NUMBER_3 = 29,
	NUMBER_4 = 30,
	NUMBER_5 = 31,
	NUMBER_6 = 32,
	NUMBER_7 = 33,
	NUMBER_8 = 34,
	NUMBER_9 = 35,
	KEYPAD_0 = 36,
	KEYPAD_1 = 37,
	KEYPAD_2 = 38,
	KEYPAD_3 = 39,
	KEYPAD_4 = 40,
	KEYPAD_5 = 41,
	KEYPAD_6 = 42,
	KEYPAD_7 = 43,
	KEYPAD_8 = 44,
	KEYPAD_9 = 45,
	F1 = 46,
	F2 = 47,
	F3 = 48,
	F4 = 49,
	F5 = 50,
	F6 = 51,
	F7 = 52,
	F8 = 53,
	F9 = 54,
	F10 = 55,
	F11 = 56,
	F12 = 57,
	ESCAPE = 58,
	GRAVE_ACCENT = 59,
	MINUS = 60,
	EQUALS = 61,
	BACKSPACE = 62,
	TAB = 63,
	LEFT_BRACKET = 64,
	RIGHT_BRACKET = 65,
	ENTER = 66,
	SEMICOLON = 67,
	QUOTE = 68,
	BACKSLASH = 69,
	COMMA = 71,
	PERIOD = 72,
	SLASH = 73,
	SPACE = 74,
	INSERT = 75,
	DELETE = 76,
	HOME = 77,
	END = 78,
	PAGE_UP = 79,
	PAGE_DOWN = 80,
	LEFT = 81,
	RIGHT = 82,
	UP = 83,
	DOWN = 84,
	KEYPAD_DIVIDE = 85,
	KEYPAD_MULTIPLY = 86,
	KEYPAD_SUBTRACT = 87,
	KEYPAD_ADD = 88,
	KEYPAD_ENTER = 90,
	PRINT_SCREEN = 91,
	PAUSE = 92,
	LEFT_SHIFT = 93,
	RIGHT_SHIFT = 94,
	LEFT_CONTROL = 95,
	RIGHT_CONTROL = 96,
	LEFT_ALT = 97,
	RIGHT_ALT = 98,
	SCROLL_LOCK = 99,
	NUM_LOCK = 100,
	CAPS_LOCK = 101,
	LEFT_SUPER = 102,
	RIGHT_SUPER = 103,
	MENU = 104
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
	PinmameMechSwitchConfig sw[MAX_MECHSW];
} PinmameMechConfig;

typedef struct {
	int type;
	int length;
	int steps;
	int pos;
	int speed;
} PinmameMechInfo;
	
typedef struct {
	const char* name;
	PINMAME_KEYCODE code;
	unsigned int standardcode;
} PinmameKeyboardInfo;

typedef void (CALLBACK *PinmameGameCallback)(PinmameGame* p_game);
typedef void (CALLBACK *PinmameOnStateUpdatedCallback)(int state);
typedef void (CALLBACK *PinmameOnDisplayAvailableCallback)(int index, int displayCount, PinmameDisplayLayout* p_displayLayout);
typedef void (CALLBACK *PinmameOnDisplayUpdatedCallback)(int index, void* p_displayData, PinmameDisplayLayout* p_displayLayout);
typedef int (CALLBACK *PinmameOnAudioAvailableCallback)(PinmameAudioInfo* p_audioInfo);
typedef int (CALLBACK *PinmameOnAudioUpdatedCallback)(void* p_buffer, int samples);
typedef void (CALLBACK *PinmameOnMechAvailableCallback)(int mechNo, PinmameMechInfo* p_mechInfo);
typedef void (CALLBACK *PinmameOnMechUpdatedCallback)(int mechNo, PinmameMechInfo* p_mechInfo);
typedef void (CALLBACK *PinmameOnSolenoidUpdatedCallback)(int solenoid, int isActive);
typedef void (CALLBACK *PinmameOnConsoleDataUpdatedCallback)(void* p_data, int size);
typedef int (CALLBACK *PinmameIsKeyPressedFunction)(PINMAME_KEYCODE keycode);

typedef struct {
	const PINMAME_AUDIO_FORMAT audioFormat;
	const int sampleRate;
	const char vpmPath[MAX_PATH];
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
} PinmameConfig;

LIBPINMAME_API PINMAME_STATUS PinmameGetGame(const char* const p_name, PinmameGameCallback callback);
LIBPINMAME_API PINMAME_STATUS PinmameGetGames(PinmameGameCallback callback);
LIBPINMAME_API void PinmameSetConfig(const PinmameConfig* const p_config);
LIBPINMAME_API int PinmameGetHandleKeyboard();
LIBPINMAME_API void PinmameSetHandleKeyboard(const int handleKeyboard);
LIBPINMAME_API int PinmameGetHandleMechanics();
LIBPINMAME_API void PinmameSetHandleMechanics(const int handleMechanics);
LIBPINMAME_API PINMAME_STATUS PinmameRun(const char* const p_name);
LIBPINMAME_API int PinmameIsRunning();
LIBPINMAME_API PINMAME_STATUS PinmamePause(const int pause);
LIBPINMAME_API PINMAME_STATUS PinmameReset();
LIBPINMAME_API void PinmameStop();
LIBPINMAME_API PINMAME_HARDWARE_GEN PinmameGetHardwareGen();
LIBPINMAME_API int PinmameGetSwitch(const int swNo);
LIBPINMAME_API void PinmameSetSwitch(const int swNo, const int state);
LIBPINMAME_API void PinmameSetSwitches(const int* const p_states, const int numSwitches);
LIBPINMAME_API int PinmameGetMaxLamps();
LIBPINMAME_API int PinmameGetChangedLamps(int* const p_changedStates);
LIBPINMAME_API int PinmameGetMaxGIs();
LIBPINMAME_API int PinmameGetChangedGIs(int* const p_changedStates);
LIBPINMAME_API int PinmameGetMaxMechs();
LIBPINMAME_API PINMAME_STATUS PinmameSetMech(const int mechNo, const PinmameMechConfig* const p_mechConfig);

#endif
