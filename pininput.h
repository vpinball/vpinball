#pragma once

#define MAX_KEYQUEUE_SIZE 32


#define APP_KEYBOARD 0
#define APP_JOYSTICKMN 1 

//rlc handle multiple joysticks, APP_JOYSTICKMN..APP_JOYSTICKMX
#define PININ_JOYMXCNT 4

#define JOYRANGEMN (-1000)
#define JOYRANGEMX (+1000)

#define JOYRANGE (JOYRANGEMX - JOYRANGEMN)

#define APP_JOYSTICKMX (APP_JOYSTICKMN + PININ_JOYMXCNT -1)
#define APP_JOYSTICK(n) (APP_JOYSTICKMN + n)

#if MAX_KEYQUEUE_SIZE & (MAX_KEYQUEUE_SIZE-1)
NOTE THAT MAX_KEYQUEUE_SIZE Must be power of 2
#endif

class PinInput
{
public:
	PinInput();
	~PinInput();


	void Init(HWND hwnd);
	void UnInit();

	//void GetShiftState(BOOL &fLeft, BOOL &fRight);

	void FireKeyEvent( int dispid, int keycode );

	int  QueueFull   ( void );
	int  QueueEmpty  ( void );
	void AdvanceHead ( void ); // called from sep thread
	void AdvanceTail ( void ); // called from thread sync'd with visuals as each keystroke is applied to the sim

	void                PushQueue( DIDEVICEOBJECTDATA *data, unsigned int app_data ); // called from sep thread
	DIDEVICEOBJECTDATA *GetTail  (); // called from visually sync'd main thread

	// Process keys up until msec_age ago .. don't consider keys that are too new for the current simulation step!
    void autostart( float secs, float retrysecs );
    void autoexit( float secs );
    void autocoin( float secs );
    void button_exit( float secs );
    void tilt_update( void );
	void ProcessKeys(PinTable *ptable);

	int GetNextKey();

	LPDIRECTINPUT7       m_pDI;         
	LPDIRECTINPUTDEVICE m_pKeyboard;
	LPDIRECTINPUTDEVICE7 m_pJoystick[PININ_JOYMXCNT];

private:
	// Writable by real-time thread only, read-only from dispatch thread
	DIDEVICEOBJECTDATA m_diq[MAX_KEYQUEUE_SIZE]; // circular queue of direct input events assembled in real time every couple milliseconds

	STICKYKEYS	m_StartupStickyKeys;

	// Modified by real-time input thread (only)
	int m_head; // head==tail means empty, (head+1)%MAX_KEYQUEUE_SIZE == tail means full

	// Modified by input dispatcher (only)
	int m_tail; // These are integer indices into keyq and should be in domain of 0..MAX_KEYQUEUE_SIZE-1

	PinTable *m_ptable;

	

public:
	HWND m_hwnd;
};

