#pragma once

#define MAX_KEYQUEUE_SIZE 32

#define USHOCKTYPE_PBWIZARD		1
#define USHOCKTYPE_ULTRACADE	2
#define USHOCKTYPE_SIDEWINDER	3
#define USHOCKTYPE_GENERIC		4

#define APP_KEYBOARD 0
#define APP_JOYSTICKMN 1

// handle multiple joysticks, APP_JOYSTICKMN..APP_JOYSTICKMX
#define PININ_JOYMXCNT 4

#define JOYRANGEMN (-1000)
#define JOYRANGEMX (+1000)

#define JOYRANGE (JOYRANGEMX - JOYRANGEMN)

#define APP_JOYSTICKMX (APP_JOYSTICKMN + PININ_JOYMXCNT -1)
#define APP_JOYSTICK(n) (APP_JOYSTICKMN + n)

#if MAX_KEYQUEUE_SIZE & (MAX_KEYQUEUE_SIZE-1)
NOTE THAT MAX_KEYQUEUE_SIZE Must be power of 2
#endif

struct KeySample
{
	U64 stamp;
	U32 keymask;
};

class PinInput
{
public:
	PinInput();
	~PinInput();

	void Init(const HWND hwnd);
	void UnInit();

	void FireKeyEvent( const int dispid, const int keycode );
	int  QueueFull   ();
	int  QueueEmpty  ();
	void AdvanceHead (); // called from sep thread
	void AdvanceTail (); // called from thread sync'd with visuals as each keystroke is applied to the sim

	void                PushQueue( DIDEVICEOBJECTDATA * const data, const unsigned int app_data ); // called from sep thread
	DIDEVICEOBJECTDATA *GetTail  ( const U32 cur_sim_msec=0xffffffff ); // called from visually sync'd main thread

	// Process keys up until msec_age ago .. don't consider keys that are too new for the current simulation step!
    void autostart( const F32 secs, const F32 retrysecs );
#ifdef ULTRAPIN
    void autoexit( const F32 secs );
#endif
	void autocoin( const F32 secs );
    void button_exit( const F32 secs );
    void tilt_update();
	void ProcessKeys(PinTable * const ptable, const U32 cur_sim_msec );

	int GetNextKey();

	void GetInputDeviceData();

	U32 Pressed ( const U32 mask ) const;
	U32 Released( const U32 mask ) const;
	U32 Held    ( const U32 mask ) const;
	U32 Down    ( const U32 mask ) const;
	U32 Changed ( const U32 mask ) const;

	LPDIRECTINPUT7       m_pDI;
	LPDIRECTINPUTDEVICE7 m_pJoystick[PININ_JOYMXCNT];

	HWND m_hwnd;

	//int InputControlRun;

	int e_JoyCnt;
	int uShockDevice;	// only one uShock device
	int uShockType;
	bool fe_message_sent;

private:
	int started();

	LPDIRECTINPUTDEVICE  m_pKeyboard;

	int m_plunger_axis, m_lr_axis, m_ud_axis, m_plunger_reverse, m_lr_axis_reverse, m_ud_axis_reverse, m_override_default_buttons, m_disable_esc;
	int m_joylflipkey, m_joyrflipkey, m_joylmagnasave, m_joyrmagnasave, m_joyplungerkey, m_joystartgamekey, m_joyexitgamekey, m_joyaddcreditkey;
	int m_joyaddcreditkey2, m_joyframecount, m_joyvolumeup, m_joyvolumedown, m_joylefttilt, m_joycentertilt, m_joyrighttilt, m_joypmbuyin;
	int m_joypmcoin3, m_joypmcoin4, m_joypmcoindoor, m_joypmcancel, m_joypmdown, m_joypmup, m_joypmenter, m_joydebug, m_joymechtilt;
	int m_joycustom1, m_joycustom1key, m_joycustom2, m_joycustom2key, m_joycustom3, m_joycustom3key, m_joycustom4, m_joycustom4key;

	U32 m_PreviousKeys;		// Masks of PININ_* inputs used by ultracade - AMH
	U32 m_ChangedKeys;		// Masks of PININ_* inputs used by ultracade - AMH

	U32 firedautostart;
	U32 firedautocoin;
	U32 LastAttempt;

	U32 started_stamp;
	int pressed_start;

	// Writable by real-time thread only, read-only from dispatch thread
	DIDEVICEOBJECTDATA m_diq[MAX_KEYQUEUE_SIZE]; // circular queue of direct input events assembled in real time every couple milliseconds

	STICKYKEYS	m_StartupStickyKeys;

	// Modified by real-time input thread (only)
	int m_head; // head==tail means empty, (head+1)%MAX_KEYQUEUE_SIZE == tail means full

	// Modified by input dispatcher (only)
	int m_tail; // These are integer indices into keyq and should be in domain of 0..MAX_KEYQUEUE_SIZE-1

	PinTable *m_ptable;
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
