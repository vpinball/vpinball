#pragma once

#include <list>

#define MAX_KEYQUEUE_SIZE 32

#if MAX_KEYQUEUE_SIZE & (MAX_KEYQUEUE_SIZE-1)
 #error Note that MAX_KEYQUEUE_SIZE must be power of 2
#endif

#define USHOCKTYPE_PBWIZARD		1
#define USHOCKTYPE_ULTRACADE	2
#define USHOCKTYPE_SIDEWINDER	3
#define USHOCKTYPE_VIRTUAPIN	4
#define USHOCKTYPE_GENERIC		5

// Input type ID - keyboard
#define APP_KEYBOARD 0

// Input type ID - mouse
#define APP_MOUSE 1

// Input type ID - joystick 1 through 4
// handle multiple joysticks, APP_JOYSTICKMN..APP_JOYSTICKMX
#define APP_JOYSTICKMN 2
#define PININ_JOYMXCNT 4
#define APP_JOYSTICK(n) (APP_JOYSTICKMN + (n))
#define APP_JOYSTICKMX  (APP_JOYSTICK(PININ_JOYMXCNT - 1))


// Joystick axis normalized input range
#define JOYRANGEMN (-65536)
#define JOYRANGEMX (+65536)
#define JOYRANGE ((JOYRANGEMX) - (JOYRANGEMN) + 1)


class PinInput
{
public:
	PinInput();
	~PinInput();

	void Init(const HWND hwnd);
	void UnInit();

	// implicitly sync'd with visuals as each keystroke is applied to the sim
	void FireKeyEvent( const int dispid, const int keycode );
	int  QueueFull   () const;
	int  QueueEmpty  () const;
	void AdvanceHead ();
	void AdvanceTail ();

	void PushQueue( DIDEVICEOBJECTDATA * const data, const unsigned int app_data/*, const U32 curr_time_msec*/ );
	const DIDEVICEOBJECTDATA *GetTail ( /*const U32 curr_sim_msec*/ );

    void autostart( const U32 msecs, const U32 retry_msecs, const U32 curr_time_msec );
#ifdef ULTRAPIN
    void autoexit( const U32 msecs );
#endif
	void autocoin( const U32 msecs, const U32 curr_time_msec );
    void button_exit( const U32 msecs, const U32 curr_time_msec );

	void tilt_update();
	
	void ProcessKeys(PinTable * const ptable/*, const U32 curr_sim_msec*/, const U32 curr_time_msec );

	int GetNextKey();

	void GetInputDeviceData(/*const U32 curr_time_msec*/);

#if 0
	U32 Pressed ( const U32 mask ) const;
	U32 Released( const U32 mask ) const;
	U32 Held    ( const U32 mask ) const;
	U32 Changed ( const U32 mask ) const;
#endif
	U32 Down    ( const U32 mask ) const; //!! only still used by mixer

#ifdef VP10
	LPDIRECTINPUT8       m_pDI;
	LPDIRECTINPUTDEVICE8 m_pJoystick[PININ_JOYMXCNT];
#else
	LPDIRECTINPUT        m_pDI;
	LPDIRECTINPUTDEVICE  m_pJoystick[PININ_JOYMXCNT];
#endif

	HWND m_hwnd;

	int e_JoyCnt;
	int uShockDevice;	// only one uShock device
	int uShockType;

	int mouseX;
    int mouseY;
    long mouseDX;
    long mouseDY;
    bool leftMouseButtonDown;
    bool rightMouseButtonDown;

	bool m_linearPlunger;
	bool m_enable_nudge_filter; // enable new nudge filtering code

private:
	int started();
	void Joy(const unsigned int n, const int updown, const bool start);

	void InitOpenPinballDevices();
	void ReadOpenPinballDevices(const U32 cur_time_msec);

	//int InputControlRun;

#ifdef VP10
    LPDIRECTINPUTDEVICE8 m_pKeyboard;
    LPDIRECTINPUTDEVICE8 m_pMouse;
#else
    LPDIRECTINPUTDEVICE m_pKeyboard;
    LPDIRECTINPUTDEVICE m_pMouse;
#endif

	// Open Pinball Device input report v1.0 - input report structure.
	// Fields are packed with no padding bytes; integer fields are little-endian.
	struct __pragma(pack(push, 1)) OpenPinballDeviceReport
	{
		uint64_t timestamp;            // report time, in microseconds since an arbitrary zero point
		uint32_t genericButtons;       // button states for 32 general-purpose on/off buttons
		uint32_t pinballButtons;       // button states for pre-defined pinball simulator function buttons
		int16_t axNudge;               // instantaneous nudge acceleration, X axis (left/right)
		int16_t ayNudge;               // instantaneous nudge acceleration, Y axis (front/back)
		int16_t vxNudge;               // instantaneous nudge velocity, X axis
		int16_t vyNudge;               // instantaneous nudge velocity, Y axis
		int16_t plungerPos;            // current plunger position
		int16_t plungerSpeed;          // instantaneous plunger speed
	} __pragma(pack(pop));

	// active Open Pinball Device interfaces
	struct OpenPinDev {
		OpenPinDev(HANDLE h, BYTE reportID, DWORD reportSize, const wchar_t *deviceStructVersionString);
		~OpenPinDev();
		HANDLE Release() { HANDLE ret = hDevice; hDevice = INVALID_HANDLE_VALUE; return ret; }
		HANDLE hDevice;
		BYTE reportID;
		DWORD deviceStructVersion;

		// overlapped read
		OVERLAPPED ov{ 0 };
		HANDLE hIOEvent{ CreateEvent(NULL, TRUE, FALSE, NULL) };
		DWORD readStatus = 0;

		DWORD bytesRead = 0;

		// overlapped read buffer - space for the HID report ID prefix and the report struct
		size_t reportSize;
		std::vector<BYTE> buf;

		// start an overlapped read
		void StartRead();

		// read a report into 'r'; returns true if a new report was available
		bool ReadReport();

		// last report read
		OpenPinballDeviceReport r;
	};
	std::list<OpenPinDev> m_openPinDevs;

	// Open Pinball Device button status, for detecting button up/down events
	uint32_t m_openPinDev_generic_buttons;
	uint32_t m_openPinDev_pinball_buttons;
	bool m_openPinDev_flipper_l, m_openPinDev_flipper_r;

	U32 m_PreviousKeys;	// Masks of PININ_* inputs used by ultracade - AMH
	U32 m_ChangedKeys;

	U32 m_firedautostart;
	U32 m_firedautocoin;

	int m_pressed_start;

    U32 m_first_stamp;
	U32 m_exit_stamp;

	int m_as_down;
    int m_as_didonce;

    int m_ac_down;
    int m_ac_didonce;

    int m_tilt_updown;

	DIDEVICEOBJECTDATA m_diq[MAX_KEYQUEUE_SIZE]; // circular queue of direct input events

	STICKYKEYS m_StartupStickyKeys;

	int m_head; // head==tail means empty, (head+1)%MAX_KEYQUEUE_SIZE == tail means full

	int m_tail; // These are integer indices into keyq and should be in domain of 0..MAX_KEYQUEUE_SIZE-1

	PinTable *m_ptable;

	// Axis assignments - these map to the drop-list index in the axis
	// selection combos in the Keys dialog:
	//
	//   0 = Disabled
	//   1 = X
	//   2 = Y
	//   3 = Z
	//   4 = rX
	//   5 = rY
	//   6 = rZ
	//   7 = Slider 1
	//   8 = Slider 2
	//   9 = Open Pinball Device (selects input mapping to the same function as assigned axis)
	//
	int m_plunger_axis, m_plunger_speed_axis, m_lr_axis, m_ud_axis, m_plunger_reverse, m_lr_axis_reverse, m_ud_axis_reverse;
	int m_override_default_buttons, m_disable_esc;
	int m_joylflipkey, m_joyrflipkey, m_joylmagnasave, m_joyrmagnasave, m_joyplungerkey, m_joystartgamekey, m_joyexitgamekey, m_joyaddcreditkey;
	int m_joyaddcreditkey2, m_joyframecount, m_joyvolumeup, m_joyvolumedown, m_joylefttilt, m_joycentertilt, m_joyrighttilt, m_joypmbuyin;
	int m_joypmcoin3, m_joypmcoin4, m_joypmcoindoor, m_joypmcancel, m_joypmdown, m_joypmup, m_joypmenter, m_joydebug, m_joymechtilt;
	int m_joycustom1, m_joycustom1key, m_joycustom2, m_joycustom2key, m_joycustom3, m_joycustom3key, m_joycustom4, m_joycustom4key;
	int m_deadz;
    bool m_enableMouseInPlayer;
};

// - Added by AMH --modified by Koadic (not sure if necessary, but doesn't hurt anything)
#define PININ_LEFT           0x00000001
#define PININ_RIGHT          0x00000002
#define PININ_LEFT2          0x00000004
#define PININ_RIGHT2         0x00000008
#define PININ_PLUNGE         0x00000010
#define PININ_START          0x00000020
#define PININ_BUYIN          0x00000040
#define PININ_COIN1          0x00000080
#define PININ_COIN2		     0x00000100
#define PININ_COIN3          0x00000200
#define PININ_COIN4          0x00000400
#define PININ_SERVICECANCEL  0x00000800
#define PININ_SERVICE1       0x00001000
#define PININ_SERVICE2       0x00002000
#define PININ_SERVICEENTER   0x00004000
#define PININ_COINDOOR       0x00008000
#define PININ_VOL_UP         0x00010000
#define PININ_VOL_DOWN       0x00020000
#define PININ_EXITGAME       0x00040000
#define PININ_LTILT          0x00080000
#define PININ_CTILT          0x00100000
#define PININ_RTILT          0x00200000
#define PININ_MTILT          0x00400000
#define PININ_FRAMES         0x00800000
#define PININ_ENABLE3D       0x01000000

#define PININ_ANY            0xffffffff
// - end input routines added by AMH
