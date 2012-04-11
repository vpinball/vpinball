#include "StdAfx.h"

#define INPUT_BUFFER_SIZE 32

static PinInput *s_pPinInput;

U32 PinInput::m_PreviousKeys;
U32 PinInput::m_ChangedKeys;
//int InputControlRun;

int m_plunger_reverse;
int m_plunger_z;
int m_plunger_rx;
int m_plunger_ry;
int m_plunger_rz;
int m_plunger_slider;
int m_pbw_default_buttons;

int e_JoyCnt;
int uShockDevice = -1;	// only one uShock device
int uShockType = 0;
bool fe_message_sent = false;

extern int Coins;
extern int curPlunger;


PinInput::PinInput()
	{

	//InputControlRun = 0;

	m_pDI = NULL;
	m_pKeyboard = NULL;
	
	m_head = m_tail = 0;
	m_PreviousKeys = 0;
	m_ChangedKeys = 0;
	m_ptable = NULL;

	memset( m_diq, 0, sizeof( m_diq ) );

	s_pPinInput = this;

	e_JoyCnt = 0;
	//m_pJoystick = NULL;
	for (int k = 0; k < PININ_JOYMXCNT; ++k) m_pJoystick[k] = NULL;

// added detection of registry entries to add enabling/disabling of specific axes to use for analog plunger control and ability to turn off the default
// button layout of the PBW controller if so desired. - Koadic
	
	HRESULT hr;
	int tmp;

	m_plunger_reverse = 0;										// Setting EnableReversePlungerAxis in the registry to 1 will enable reversing the axis
	hr = GetRegInt("Player", "EnableReversePlungerAxis", &tmp); // for the plunger on Generic Gamepads and Sidewinder Freestyle Pro (USB) incase it is 
	if (hr == S_OK) m_plunger_reverse = tmp;					// required by the mounting solution. Not used for PBW/Ultracade Plunger 

	m_plunger_z = 1;											// On by default, enables Z-Axis Plunger for Generic Gamepads and allows disabling
	hr = GetRegInt("Player", "Enable_Z", &tmp);					// of the Z-Axis on the PBW so the rZ-Axis can be used instead; because on my unit,
	if (hr == S_OK) m_plunger_z = tmp;							// the rZ-Axis has a much larger detected range than the Z-Axis (both operated by plunger)

	m_plunger_rz = 0;											// Setting Enable_rZ in the registry to 1 enables the rZ-Axis Plunger for Generic Gamepads 
	hr = GetRegInt("Player", "Enable_rZ", &tmp);				// and the PBW. Doing so should also disable usability of the Z axis (which is enabled by
	if (hr == S_OK) m_plunger_rz = tmp;							// default), but won't effect other Axes.

	m_plunger_rx = 0;											// Setting Enable_rX in the registry to 1 enables the rX-Axis Plunger for Generic Gamepads
	hr = GetRegInt("Player", "Enable_rX", &tmp);
	if (hr == S_OK) m_plunger_rx = tmp;

	m_plunger_ry = 0;											// Setting Enable_ry in the registry to 1 enables the rY-Axis Plunger for Generic Gamepads
	hr = GetRegInt("Player", "Enable_rY", &tmp);
	if (hr == S_OK) m_plunger_ry = tmp;

	m_plunger_slider = 0;										// Setting Enable_Slider in the registry to 1 enables the Slider Plunger for Generic Gamepads
	hr = GetRegInt("Player", "Enable_Slider", &tmp);
	if (hr == S_OK) m_plunger_slider = tmp;

	m_pbw_default_buttons = 1;									// On by default. Setting PBWDefaultButtons in the registry to 0 turns off all hard-coded
	hr = GetRegInt("Player", "PBWDefaultButtons", &tmp);		// buttons in the PBW profile so they can be set with another utility such as Joy2Key or XPadder
	if (hr == S_OK) m_pbw_default_buttons = tmp;

// end of additions
	}


PinInput::~PinInput()
{
	s_pPinInput = NULL;
}


//-----------------------------------------------------------------------------
// Name: EnumObjectsCallback()
// Desc: Callback function for enumerating objects (axes, buttons, POVs) on a 
//       joystick. This function enables user interface elements for objects
//       that are found to exist, and scales axes min/max values.
//-----------------------------------------------------------------------------
BOOL CALLBACK EnumObjectsCallback( const DIDEVICEOBJECTINSTANCE* pdidoi,
                                   VOID* pContext )
{
    //HWND hDlg = (HWND)pContext;
	PinInput * const ppinput = (PinInput *)pContext;

#ifdef _DEBUG
	static int nAxis = 0;
	static int nButtons = 0;
    static int nSliderCount = 0;  // Number of returned slider controls
    static int nPOVCount = 0;     // Number of returned POV controls
	static int nKey = 0;
	static int nUnknown = 0;
#endif

    // For axes that are returned, set the DIPROP_RANGE property for the
    // enumerated axis in order to scale min/max values.
    if( pdidoi->dwType & DIDFT_AXIS )
    {
        DIPROPRANGE diprg; 
        diprg.diph.dwSize       = sizeof(DIPROPRANGE); 
        diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
        diprg.diph.dwObj        = pdidoi->dwType; // Specify the enumerated axis
		diprg.diph.dwHow        = DIPH_BYID; 
        diprg.lMin              = JOYRANGEMN; 
        diprg.lMax              = JOYRANGEMX; 
    
        // Set the range for the axis
        if( FAILED( ppinput->m_pJoystick[e_JoyCnt]->SetProperty( DIPROP_RANGE, &diprg.diph ) ) ) 
			{return DIENUM_STOP;}   

		// set DEADBAND to Zero
		DIPROPDWORD dipdw;
		dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
        dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
		dipdw.diph.dwObj        = pdidoi->dwType; // Specify the enumerated axis
		dipdw.diph.dwHow        = DIPH_BYID;
		dipdw.dwData            = 30;//g_pplayer->DeadZ; //allows for 0-100% deadzone in 1% increments
 
        // Set the deadzone
		if(FAILED(ppinput->m_pJoystick[e_JoyCnt]->SetProperty(DIPROP_DEADZONE, &dipdw.diph)))
			{return DIENUM_STOP;}
	}

#ifdef _DEBUG

    if (pdidoi->guidType == GUID_XAxis)			{++nAxis;}
    else if (pdidoi->guidType == GUID_YAxis)	{++nAxis;}
    else if (pdidoi->guidType == GUID_ZAxis)	{++nAxis;}
    else if (pdidoi->guidType == GUID_RxAxis)	{++nAxis;}
    else if (pdidoi->guidType == GUID_RyAxis)	{++nAxis;}
    else if (pdidoi->guidType == GUID_RzAxis)	{++nAxis;}
    else if (pdidoi->guidType == GUID_Slider)	{++nSliderCount;}
	else if (pdidoi->guidType == GUID_Button)	{++nButtons;}
	else if (pdidoi->guidType == GUID_Key)		{++nKey;}
    else if (pdidoi->guidType == GUID_POV)		{++nPOVCount;}	
	else if (pdidoi->guidType == GUID_Unknown)	{++nUnknown;}

#endif

    return DIENUM_CONTINUE;
}


// Callback for enumerating joysticks (gamepads)

BOOL CALLBACK DIEnumJoystickCallback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef)
	{
	DIPROPSTRING dstr;
	
	dstr.diph.dwSize = sizeof(DIPROPSTRING);
	dstr.diph.dwHeaderSize = sizeof(DIPROPHEADER);  
	dstr.diph.dwObj = 0;
	dstr.diph.dwHow = 0;

	PinInput * const ppinput = (PinInput *)pvRef;
	
	HRESULT hr;
	
	hr = ppinput->m_pDI->CreateDeviceEx(lpddi->guidInstance, IID_IDirectInputDevice7
										,(void **)&ppinput->m_pJoystick[e_JoyCnt], NULL);
	if (FAILED(hr))
		{
		ppinput->m_pJoystick[e_JoyCnt] = NULL; //make sure no garbage
		return DIENUM_CONTINUE; //rlc try for another joystick
		}

	hr =  ppinput->m_pJoystick[e_JoyCnt]->GetProperty( DIPROP_PRODUCTNAME,  &dstr.diph);

	if (hr == S_OK)
		{	
		if (!WzSzStrCmp(dstr.wsz, "PinballWizard"))
			{
				uShockDevice = e_JoyCnt;	// remember uShock
				uShockType = USHOCKTYPE_PBWIZARD; //set type 1 = PinballWizard
			}
		else if (!WzSzStrCmp(dstr.wsz, "UltraCade Pinball"))
			{
				uShockDevice = e_JoyCnt;	// remember uShock
				uShockType = USHOCKTYPE_ULTRACADE; //set type 2 = UltraCade Pinball
			}
		
		else if (!WzSzStrCmp(dstr.wsz, "Microsoft SideWinder Freestyle Pro (USB)"))
			{
				uShockDevice = e_JoyCnt;	// remember uShock
				uShockType = USHOCKTYPE_SIDEWINDER; //set type 3 = Microsoft SideWinder Freestyle Pro
			}

		else 
			{
				uShockDevice = e_JoyCnt;	// remember uShock
				uShockType = USHOCKTYPE_GENERIC; //Generic Gamepad
			}
		}	
	hr = ppinput->m_pJoystick[e_JoyCnt]->SetDataFormat(&c_dfDIJoystick);

	// joystick input foreground or background focus
	hr = ppinput->m_pJoystick[e_JoyCnt]->SetCooperativeLevel(ppinput->m_hwnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);

	DIPROPDWORD dipdw;

	dipdw.diph.dwSize = sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dipdw.diph.dwObj = 0;
	dipdw.diph.dwHow = DIPH_DEVICE;
	dipdw.dwData = INPUT_BUFFER_SIZE;

	hr = ppinput->m_pJoystick[e_JoyCnt]->SetProperty( DIPROP_BUFFERSIZE, &dipdw.diph );

	// Enumerate the joystick objects. The callback function enabled user
	// interface elements for objects that are found, and sets the min/max
	// values property for discovered axes.
	hr = ppinput->m_pJoystick[e_JoyCnt]->EnumObjects( EnumObjectsCallback,(VOID*)pvRef, DIDFT_ALL);

	if(++e_JoyCnt < PININ_JOYMXCNT) return DIENUM_CONTINUE;

	return DIENUM_STOP;			//allocation for only PININ_JOYMXCNT joysticks, ignore any others
	
	}


int PinInput::QueueFull()
{
	return ( ( ( m_head+1 ) % MAX_KEYQUEUE_SIZE ) == m_tail );
}


int PinInput::QueueEmpty()
{
	return m_head == m_tail;
}


void PinInput::AdvanceHead()
{
	m_head = ( m_head + 1 ) % MAX_KEYQUEUE_SIZE;
}


void PinInput::AdvanceTail()
{
	m_tail = ( m_tail + 1 ) % MAX_KEYQUEUE_SIZE;
}


void PinInput::PushQueue( DIDEVICEOBJECTDATA *data, unsigned int app_data )
{
	if(( !data ) || QueueFull()) return;

	m_diq[m_head] = *data;
	m_diq[m_head].dwTimeStamp = msec();		//rewrite time from game start
	m_diq[m_head].dwSequence = app_data;
	AdvanceHead();
}


DIDEVICEOBJECTDATA *PinInput::GetTail( U32 cur_sim_msec )
{
	if( QueueEmpty() ) return NULL;

	const int tmp = m_tail;

	DIDEVICEOBJECTDATA *ptr = &m_diq[tmp];

	const int diff = (int) ((U32)( cur_sim_msec - ptr->dwTimeStamp ));

	// If we've simulated to or beyond the timestamp of when this control was received then process the control into the system
	if( diff > 0 || cur_sim_msec == 0xffffffff )
	{
		AdvanceTail();

		return ptr;
	}
	return NULL;
}


//RLC combine these threads if the Xenon problem is smashed

void GetInputDeviceData() 
	{
	DIDEVICEOBJECTDATA didod[ INPUT_BUFFER_SIZE ];  // Receives buffered data 
	DWORD dwElements;
	HRESULT hr;		
	HWND hwnd = s_pPinInput->m_hwnd;

	if(!s_pPinInput) return;	// bad pointer exit

	{
	LPDIRECTINPUTDEVICE pkyb = s_pPinInput->m_pKeyboard;
	if (pkyb) //keyboard
		{	
		hr = pkyb->Acquire();				// try to Acquire keyboard input
		if (hr == S_OK || hr == S_FALSE)
			{
			dwElements = INPUT_BUFFER_SIZE;
			hr = pkyb->GetDeviceData( sizeof(DIDEVICEOBJECTDATA), didod, &dwElements, 0 );				
			
            if (hr == S_OK || hr == DI_BUFFEROVERFLOW)
				{					
				if (hwnd == GetForegroundWindow())
					{
					for (DWORD i = 0; i < dwElements; i++) s_pPinInput->PushQueue( &didod[i], APP_KEYBOARD ); 
					}
				}
			}
		}

	// same for joysticks 

	for (int k = 0; k < e_JoyCnt; ++k)
		{
		LPDIRECTINPUTDEVICE pjoy = s_pPinInput->m_pJoystick[k];
		if (pjoy)
			{				
			hr = pjoy->Acquire();							// try to Acquire joustick input
			if (hr == S_OK || hr == S_FALSE)
				{					
				dwElements = INPUT_BUFFER_SIZE;
				hr = pjoy->GetDeviceData( sizeof(DIDEVICEOBJECTDATA), didod, &dwElements, 0);						

				if (hr == S_OK || hr == DI_BUFFEROVERFLOW)
					{	
					if (hwnd == GetForegroundWindow())
						{														
						for (DWORD i = 0; i < dwElements; i++) s_pPinInput->PushQueue( &didod[i], APP_JOYSTICK(k)); 
						}
					}	
				}
			}
		}	
	}
	}

void PinInput::Init(HWND hwnd)
	{
	HRESULT hr;

	m_hwnd = hwnd;

    hr = DirectInputCreateEx(g_hinst, DIRECTINPUT_VERSION, IID_IDirectInput7, (void **)&m_pDI, NULL);

		{
		// Create keyboard device
		hr = m_pDI->CreateDevice( GUID_SysKeyboard, &m_pKeyboard, NULL); //Standard Keyboard device

		hr = m_pKeyboard->SetDataFormat( &c_dfDIKeyboard );

		hr = m_pKeyboard->SetCooperativeLevel(hwnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
		
		DIPROPDWORD dipdw;
		dipdw.diph.dwSize = sizeof(DIPROPDWORD);
		dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
		dipdw.diph.dwObj = 0;
		dipdw.diph.dwHow = DIPH_DEVICE;
		dipdw.dwData = INPUT_BUFFER_SIZE;

		hr = m_pKeyboard->SetProperty( DIPROP_BUFFERSIZE, &dipdw.diph );


		/*  Disable Sticky Keys */

		// get the current state
		m_StartupStickyKeys.cbSize = sizeof(STICKYKEYS);
		SystemParametersInfo(SPI_GETSTICKYKEYS, sizeof(STICKYKEYS), &m_StartupStickyKeys, 0);

		// turn it all OFF
		STICKYKEYS newStickyKeys;
		newStickyKeys.cbSize = sizeof(STICKYKEYS);
		newStickyKeys.dwFlags = 0;
		SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &newStickyKeys, SPIF_SENDCHANGE);
		}

		uShockDevice = -1;
		uShockType = 0;

		m_pDI->EnumDevices(DIDEVTYPE_JOYSTICK, DIEnumJoystickCallback, this, DIEDFL_ATTACHEDONLY);//enum Joysticks

		//InputControlRun = 1;	//0== stalled, 1==run,  0 < shutting down, 2==terminated
		//_beginthread( InputControlProcess, 0, NULL );
	}


void PinInput::UnInit()
{
	// Unacquire and release any DirectInputDevice objects.
	HRESULT hr;
							//1==run,  0 < shutting down, 2==terminated
	//InputControlRun = -100;	// terminate control thread, force after 500mS
	
	//while (++InputControlRun < 0) SLEEP(5);		// set to exit AND WAIT

	//if (!InputControlRun)	//0 == stalled, 1==run,  0 < shutting down, 2==terminated
		//{exit (-1500);}

	if(m_pKeyboard) 
		{
		// Unacquire the device one last time just in case 
		// the app tried to exit while the device is still acquired.
		hr = m_pKeyboard->Unacquire();
		hr = m_pKeyboard->Release();
		m_pKeyboard = NULL;
		}

		// restore the state of the sticky keys
		SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &m_StartupStickyKeys, SPIF_SENDCHANGE);

	for (int k = 0; k < e_JoyCnt; ++k)
		{
		if (m_pJoystick[k])// = NULL) //rlc fixed 
			{
			// Unacquire the device one last time just in case 
			// the app tried to exit while the device is still acquired.
			hr = m_pJoystick[k]->Unacquire();
			hr = m_pJoystick[k]->Release();
			m_pJoystick[k] = NULL;
			}
		}

	s_pPinInput = NULL;							// clean up
	// Release any DirectInput objects.
	if( m_pDI ) 
		{
		hr = m_pDI->Release();
		m_pDI = NULL;
		}

	m_head = m_tail = 0;

	memset( m_diq, 0, sizeof( m_diq ));
}

void PinInput::FireKeyEvent( int dispid, int key )
{
	U32 val = 0;
	U32 tmp = m_PreviousKeys;
	
	// Initialize.
	int mkey = key;

	// Check if we are mirrored.
	if ( g_pplayer->m_ptable->m_tblMirrorEnabled )
	{
		// Swap left & right input.
			 if( mkey == g_pplayer->m_rgKeys[eLeftFlipperKey]  ) {mkey = g_pplayer->m_rgKeys[eRightFlipperKey];		val |= PININ_RIGHT;}
		else if( mkey == g_pplayer->m_rgKeys[eRightFlipperKey] ) {mkey = g_pplayer->m_rgKeys[eLeftFlipperKey];		val |= PININ_LEFT;}
		else if( mkey == g_pplayer->m_rgKeys[eLeftMagnaSave]   ) {mkey = g_pplayer->m_rgKeys[eRightMagnaSave];		val |= PININ_RIGHT2;}
		else if( mkey == g_pplayer->m_rgKeys[eRightMagnaSave]  ) {mkey = g_pplayer->m_rgKeys[eLeftMagnaSave];		val |= PININ_LEFT2;}
		else if( mkey == DIK_LSHIFT ) {mkey = DIK_RSHIFT;	val |= PININ_RIGHT;}
		else if( mkey == DIK_RSHIFT ) {mkey = DIK_LSHIFT;	val |= PININ_LEFT;}
		else if( mkey == DIK_LEFT   ) {mkey = DIK_RIGHT;	val |= PININ_RIGHT;}
		else if( mkey == DIK_RIGHT  ) {mkey = DIK_LEFT;		val |= PININ_LEFT;}
	}
	else
	{
		// Normal left & right input.
			if( mkey == g_pplayer->m_rgKeys[eLeftFlipperKey]  ) val |= PININ_LEFT;
		else if( mkey == g_pplayer->m_rgKeys[eRightFlipperKey] ) val |= PININ_RIGHT;
		else if( mkey == g_pplayer->m_rgKeys[eLeftMagnaSave]   ) val |= PININ_LEFT2;
		else if( mkey == g_pplayer->m_rgKeys[eRightMagnaSave]  ) val |= PININ_RIGHT2;
		else if( mkey == DIK_LSHIFT ) val |= PININ_LEFT;
		else if( mkey == DIK_RSHIFT ) val |= PININ_RIGHT;
		else if( mkey == DIK_LEFT   ) val |= PININ_LEFT;
		else if( mkey == DIK_RIGHT  ) val |= PININ_RIGHT;
	}

         if( mkey == g_pplayer->m_rgKeys[ePlungerKey]      ) val |= PININ_PLUNGE;
    else if( mkey == g_pplayer->m_rgKeys[eAddCreditKey]    ) val |= PININ_COIN1;
    else if( mkey == g_pplayer->m_rgKeys[eAddCreditKey2]   ) val |= PININ_DBA;
    else if( mkey == g_pplayer->m_rgKeys[eStartGameKey]    ) val |= PININ_START;
    else if( mkey == g_pplayer->m_rgKeys[eVolumeUp]		  ) val |= PININ_VOL_UP;
    else if( mkey == g_pplayer->m_rgKeys[eVolumeDown]      ) val |= PININ_VOL_DOWN;
    else if( mkey == g_pplayer->m_rgKeys[eExitGame]        ) val |= PININ_EXITGAME;

    else if( mkey == DIK_ESCAPE ) val |= PININ_EXITGAME;
	else if( mkey == DIK_UP     ) val |= PININ_START;
	else if( mkey == DIK_1      ) val |= PININ_START;
	else if( mkey == DIK_5      ) val |= PININ_COIN1;
	else if( mkey == DIK_6      ) val |= PININ_COIN2;
	else if( mkey == DIK_DOWN   ) val |= PININ_PLUNGE;
//	else if( mkey == DIK_EQUALS ) val |= PININ_VOL_UP;
//	else if( mkey == DIK_MINUS  ) val |= PININ_VOL_DOWN;
	else if( mkey == DIK_7      ) val |= PININ_TEST;
	else if( mkey == DIK_T      ) val |= PININ_TEST;
	else if( mkey == DIK_8      ) val |= PININ_SERVICE1;
	else if( mkey == DIK_3      ) val |= PININ_DBA;

	// Check the the mkey is down.
	if( dispid == DISPID_GameEvents_KeyDown )
	{
		// Turn the bit on.
		tmp |= val;
	}
	else
	{
		// Turn the bit off.
		tmp &= (~val);
	}

	// Get only the bits that have changed (on to off, or off to on).
	m_ChangedKeys |= ( tmp ^ m_PreviousKeys );

	// Save the keys so we can detect changes.
	m_PreviousKeys = tmp;
	m_ptable->FireKeyEvent( dispid, mkey );
}

static U32 started_stamp;
static int pressed_start;


// Returns true if the table has started at least 1 player.
static int started()
{

	// Was the start button pressed?
    if( pressed_start ) 
	{
#if 0	//rlc remove check for old UP game
		// Check if we need to send a message to the front end.
		if ( fe_message_sent == false )
		{
			// Find the front end window.
			HWND	hFrontEndWnd;

			// Find the window.
			hFrontEndWnd = FindWindow( NULL, "Ultrapin (plfe)" );
			if ( hFrontEndWnd != NULL )
			{
				// Make sure we have a player.
				if ( g_pplayer )
				{
					// Show the window.
					ShowWindow(g_pplayer->m_hwnd, SW_SHOW);
					SetForegroundWindow(g_pplayer->m_hwnd);                      
					SetFocus(g_pplayer->m_hwnd);
				}

				// Send the ball eject message to the front end.
				if ( SendMessage( hFrontEndWnd, WM_USER, WINDOWMESSAGE_FIRSTBALLEJECTED, 0 ) )
				{
					fe_message_sent = true;
				}
			}
		}
#endif

		return 1;
	}

	// Are there more balls on the table than what was loaded?
	if ( (Ball::NumInitted() - PinTable::NumStartBalls()) > 0 )
	{
		pressed_start = 1;
		return 1;
	}
	else
	{
		return 0;
	}

}


static U32 firedautostart = 0; 
static U32 firedautocoin = 0; 
static U32 LastAttempt; 


// Adds coins that were passed in from the 
// credit manager via a window message. 
void PinInput::autocoin( F32 secs )
{
	// Make sure we have a player.
	if( !g_pplayer ) 
		return;

	static int down = 0;
	static int didonce = 0;

	// Check if we have coins.
	if ( Coins > 0 )
	{
		if( (firedautocoin > 0) &&														// Initialized.
			(down == 1) &&																// Coin button is down.
			((msec() - firedautocoin) > 100) )											// Coin button has been down for at least 0.10 seconds.
		{
			// Release coin button.
			firedautocoin = msec();
			down = 0;
			FireKeyEvent( DISPID_GameEvents_KeyUp, g_pplayer->m_rgKeys[eAddCreditKey] );

			// Update the counter.
			Coins--;

			OutputDebugString( "**Autocoin: Release.\n" );
		}

		// Logic to do "autocoin"
		if( (down == 0) &&																// Coin button is up.
			(((didonce == 1) && ((msec() - firedautocoin) > 500))) ||					// Last attempt was at least 0.50 seconds ago.
			((didonce == 0) && ((msec() - firedautocoin) > ((U32)(secs*1000.0f)))) )	// Never attempted and at least autostart seconds have elapsed.
		{
			// Press coin button.
			firedautocoin = msec();
			down = 1;
			didonce = 1;
			FireKeyEvent( DISPID_GameEvents_KeyDown, g_pplayer->m_rgKeys[eAddCreditKey] );

			OutputDebugString( "**Autocoin: Press.\n" );
		}
	}

}


void PinInput::autostart( F32 secs, F32 retrysecs )
{

//    if( !VPinball::m_open_minimized ) 
//	{
//		return;
//	}
    
	// Make sure we have a player.
	if(( !g_pplayer ) ||
	// Check if we already started.
	   ( started() ))
		return;

	static int down = 0;
	static int didonce = 0;

	if( (firedautostart > 0) &&				// Initialized.
		(down == 1) &&						// Start button is down.
		((msec() - firedautostart) > 100) )	// Start button has been down for at least 0.10 seconds.
	{
		// Release start.
		firedautostart = msec();
        down = 0;
		FireKeyEvent( DISPID_GameEvents_KeyUp, g_pplayer->m_rgKeys[eStartGameKey] );

		OutputDebugString( "Autostart: Release.\n" );
	}

    // Logic to do "autostart"
    if( (down == 0) &&																					// Start button is up.
        (((didonce == 1) && !started() && ((msec() - firedautostart) > ((U32)(retrysecs*1000.0f))))) ||	// Not started and last attempt was at least AutoStartRetry seconds ago.
        ((didonce == 0) && ((msec() - firedautostart) > ((U32)(secs*1000.0f)))) )						// Never attempted and autostart time has elapsed.
    {
		// Check if we haven't accounted for the play.
		if ( (didonce == 0) &&								// Never attempted autostarted.
			 (!started()) )									// Player hasn't already started manually.
		{
			// Update the number of plays.
			VPinball::NumPlays++;
			SetRegValue( "Statistics", "NumPlays", REG_DWORD, &(VPinball::NumPlays), 4 );
		}

		// Press start.
        firedautostart = msec();
        down = 1;
		didonce = 1;
        FireKeyEvent( DISPID_GameEvents_KeyDown, g_pplayer->m_rgKeys[eStartGameKey] );

		OutputDebugString( "Autostart: Press.\n" );
    }

}



void PinInput::autoexit( F32 secs )
{
    if(( !VPinball::m_open_minimized ) ||
       ( !g_pplayer ) ||
       ( secs == 0.0f) ||
	   // Check if we have not started.
	   ( !started() ))
	   return;

	// Check if we can exit.
    if (started())													// No coins queued for entry.
        ExitApp();
}


static U32 exit_stamp;
static U32 fastexit_stamp;
static U32 first_stamp;

void PinInput::button_exit( F32 secs )
{
    if( !first_stamp ) 
	{
		first_stamp = msec();
	}

	// Don't allow button exit until after game has been running for 1 second.
    if( msec() - first_stamp < 1000 ) 
		return; 

	// Check if we can exit.
    if( (exit_stamp) &&										// Initialized.
		((msec() - exit_stamp) > (secs * 1000.0f)) &&		// Held exit button for number of seconds.
		(Coins == 0) )										// No coins queued to be entered.
	{
#ifndef STUCK_EXIT_BUTTON
		if (uShockType == USHOCKTYPE_ULTRACADE)
		{
			ExitApp();  //remove pesky exit button
		}
		else
		{
			exit(0); //Close out to desktop
		}
#endif
	}

}

void PinInput::tilt_update()
{
    if( !g_pplayer || g_pplayer->m_NudgeManual >= 0) return;

    static int updown;
    int tmp = updown;

    updown = plumb_tilted() ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp;

    if( updown != tmp ) FireKeyEvent( updown, g_pplayer->m_rgKeys[eCenterTiltKey] );
}

void PinInput::ProcessKeys(PinTable *ptable, U32 cur_sim_msec )
{

	DIDEVICEOBJECTDATA *input;
	
	m_ptable = ptable;

	m_ChangedKeys = 0;

	if (!g_pplayer ) return;	//only when player running

    if( ptable )
    {
		// Check if autostart is enabled.
        if( ptable->m_tblAutoStartEnabled ) 
		{
			// Update autostart.
			autostart( ptable->m_tblAutoStart, ptable->m_tblAutoStartRetry );
		}
		
		// Update autocoin (use autostart seconds to define when nvram is ready).
		autocoin( ptable->m_tblAutoStart );

		// Update autoexit.
		autoexit( ptable->m_timeout );
        
		button_exit( ptable->m_tblExitConfirm );
        
		// Update tilt.
		tilt_update();
    }


	// Check if we've been initialized.
    if( firedautostart == 0 )
    {
		firedautostart = msec();
    }

	// Check if we've been initialized.
    if( firedautocoin == 0 )
    {
		firedautocoin = msec();
    }

	GetInputDeviceData();

	while( ( input = GetTail( cur_sim_msec ) ) )
	{
		if( input->dwSequence == APP_KEYBOARD )
		{
			if( input->dwOfs == (DWORD)g_pplayer->m_rgKeys[eFrameCount])
			{
				if (input->dwData & 0x80)
				{
					g_pplayer->ToggleFPS();
				}
			}
			else if( input->dwOfs == (DWORD)g_pplayer->m_rgKeys[eDBGBalls])
			{	
				// Activate on edge only.
				if (input->dwData & 0x80)
				{
					g_pplayer->m_DebugBalls = !(g_pplayer->m_DebugBalls);
					g_pplayer->m_ToggleDebugBalls = fTrue;
				}
			}
			else if( input->dwOfs == DIK_ESCAPE )
			{
				if (input->dwData & 0x80) g_pplayer->m_fCloseDown = fTrue; //on key down only
			}
			else
			{
				FireKeyEvent((input->dwData & 0x80) ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, input->dwOfs);
			}
		}
		else if( input->dwSequence >= APP_JOYSTICKMN && input->dwSequence <= APP_JOYSTICKMX )
		{
			int joyk = input->dwSequence - APP_JOYSTICKMN ;	//rlc joystick index
			static bool rotLeftManual = false;

			if ((input->dwOfs >= DIJOFS_BUTTON0 && input->dwOfs <= DIJOFS_BUTTON31) && (m_pbw_default_buttons == 1)) //added ability to set registry entry to turn off default button layout
			{
				int updown = (input->dwData & 0x80)?DISPID_GameEvents_KeyDown:DISPID_GameEvents_KeyUp;

				if(( input->dwOfs == DIJOFS_BUTTON9 ) && (uShockType == USHOCKTYPE_PBWIZARD))	// left
				{
					FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftFlipperKey] );
				}
				else if(( input->dwOfs == DIJOFS_BUTTON8 ) && (uShockType == USHOCKTYPE_ULTRACADE))	// left
				{
					FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftFlipperKey] );
				}
				else if(( input->dwOfs == DIJOFS_BUTTON1 ) && (uShockType == USHOCKTYPE_PBWIZARD)) // right
				{
					FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightFlipperKey] );
				}
				else if(( input->dwOfs == DIJOFS_BUTTON10 ) && (uShockType == USHOCKTYPE_ULTRACADE)) // right
				{
					FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightFlipperKey] );
				}
				else if(( input->dwOfs == DIJOFS_BUTTON0 )	&& (uShockType == USHOCKTYPE_PBWIZARD)) // plunge
				{
					FireKeyEvent( updown,g_pplayer->m_rgKeys[ePlungerKey] );
				}
				else if(( input->dwOfs == DIJOFS_BUTTON13 )	&& (uShockType == USHOCKTYPE_ULTRACADE)) // plunge
				{
					FireKeyEvent( updown,g_pplayer->m_rgKeys[ePlungerKey] );
				}
				else if(( input->dwOfs == DIJOFS_BUTTON4 )	&& (uShockType == USHOCKTYPE_PBWIZARD)) // volume up
				{
					FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeUp] );
				}
				else if(( input->dwOfs == DIJOFS_BUTTON5 )	&& (uShockType == USHOCKTYPE_ULTRACADE)) // volume up
				{
					FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeUp] );
				}
				else if(( input->dwOfs == DIJOFS_BUTTON3 )	&& (uShockType == USHOCKTYPE_PBWIZARD)) // volume down
				{
					FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeDown] );
				}
				else if(( input->dwOfs == DIJOFS_BUTTON6 )	&& (uShockType == USHOCKTYPE_ULTRACADE)) // volume down
				{
					FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeDown] );
				}
				else if(( input->dwOfs == DIJOFS_BUTTON11 )	&& (uShockType == USHOCKTYPE_PBWIZARD)) // coin 1
				{
					FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey] );
				}
				else if(( input->dwOfs == DIJOFS_BUTTON0 )	&& (uShockType == USHOCKTYPE_ULTRACADE)) // coin 1
				{
					FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey] );
				}
				else if(( input->dwOfs == DIJOFS_BUTTON12 )	&& (uShockType == USHOCKTYPE_PBWIZARD)) // coin 2
				{
					FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey2] );
				}
				else if(( input->dwOfs == DIJOFS_BUTTON1 )	&& (uShockType == USHOCKTYPE_ULTRACADE)) // coin 2
				{
					FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey2] );
				}
				else if(( input->dwOfs == DIJOFS_BUTTON8 )	&& (uShockType == USHOCKTYPE_PBWIZARD)) // start
				{
					// Check if we can allow the start (table is done initializing).
                    if( ((msec() - firedautostart) > ((U32)(ptable->m_tblAutoStart*1000.0f))) ||
						(pressed_start) || started() ) 
                    {
                        pressed_start = 1;
                        FireKeyEvent( updown,g_pplayer->m_rgKeys[eStartGameKey] );

						// Check if this was a press.
						if ( updown == DISPID_GameEvents_KeyDown )
						{
							// Update the number of plays.
							VPinball::NumPlays++;
							SetRegValue( "Statistics", "NumPlays", REG_DWORD, &(VPinball::NumPlays), 4 );
						}
                    }
				}
				else if(( input->dwOfs == DIJOFS_BUTTON12 )	&& (uShockType == USHOCKTYPE_ULTRACADE)) // start
				{
					// Check if we can allow the start (table is done initializing).
                    if( ((msec() - firedautostart) > ((U32)(ptable->m_tblAutoStart*1000.0f))) ||
						(pressed_start) || started() ) 
                    {
                        pressed_start = 1;
                        FireKeyEvent( updown,g_pplayer->m_rgKeys[eStartGameKey] );

						// Check if this was a press.
						if ( updown == DISPID_GameEvents_KeyDown )
						{
							// Update the number of plays.
							VPinball::NumPlays++;
							SetRegValue( "Statistics", "NumPlays", REG_DWORD, &(VPinball::NumPlays), 4 );
						}
                    }
				}
				else if(( input->dwOfs == DIJOFS_BUTTON7 )	&& (uShockType == USHOCKTYPE_PBWIZARD)) // exit
				{
					// Check if we have started a game yet.
					if ( (started()) ||
						 (ptable->m_tblAutoStartEnabled == false) )
					{
						if( DISPID_GameEvents_KeyDown == updown ) 
						{
							first_stamp = msec();
							exit_stamp = msec();
							FireKeyEvent( DISPID_GameEvents_KeyDown,g_pplayer->m_rgKeys[eExitGame ] );  
						}
						else 
						{
							FireKeyEvent( DISPID_GameEvents_KeyUp,g_pplayer->m_rgKeys[eExitGame ] );  
							exit_stamp = 0;
						}
					}
				}
				else if(( input->dwOfs == DIJOFS_BUTTON14 )	&& (uShockType == USHOCKTYPE_ULTRACADE)) // exit
				{
					// Check if we have started a game yet.
					if ( (started()) ||
						 (ptable->m_tblAutoStartEnabled == false) )
					{
						if( DISPID_GameEvents_KeyDown == updown ) 
						{
							first_stamp = msec();
							exit_stamp = msec();
							FireKeyEvent( DISPID_GameEvents_KeyDown,g_pplayer->m_rgKeys[eExitGame ] );  
						}
						else 
						{
							FireKeyEvent( DISPID_GameEvents_KeyUp,g_pplayer->m_rgKeys[eExitGame ] );  
							exit_stamp = 0;
						}
					}
				}
				else if(( input->dwOfs == DIJOFS_BUTTON6 )	&& (uShockType == USHOCKTYPE_PBWIZARD)) // pause menu
				{
					if( DISPID_GameEvents_KeyDown == updown ) 
					{
						g_pplayer->m_fCloseDown = fTrue;
					}
				}

//				else if( input->dwOfs == DIJOFS_BUTTON5 )	// BKGL
//				{
//				}
				else if( input->dwOfs == DIJOFS_BUTTON10 )			// left 2
				{
                    FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftMagnaSave ] );
				}
				else if( input->dwOfs == DIJOFS_BUTTON2 )			// right2
				{					
					FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightMagnaSave ] );
				}
				else if(( input->dwOfs == DIJOFS_BUTTON6 )	&& (uShockType == USHOCKTYPE_ULTRACADE))	// Manual Joystick control
				{
					if(updown&1 && g_pplayer != NULL) 
					{					
					if(g_pplayer->m_NudgeManual >= 0) g_pplayer->m_NudgeManual = -1; //standard mode
					else g_pplayer->m_NudgeManual = joyk;			//only one joystick has manual control
						
					rotLeftManual = g_pplayer->m_NudgeManual >= 0;	// for normal UltraCade Table
					}
				}
				else if(( input->dwOfs == DIJOFS_BUTTON7 )	&& (uShockType == USHOCKTYPE_ULTRACADE))	// Switch to manual joystick control.
				{
					if(updown&1 && g_pplayer != NULL) 
					{
						rotLeftManual = false;						// clear 
						if(g_pplayer->m_NudgeManual >= 0) 
						{
							g_pplayer->m_NudgeManual = -1;			//standard mode
						}
						else
						{
							g_pplayer->m_NudgeManual = joyk;		//only one joystick has manual control
						}
					}
				}
				FireKeyEvent(updown,input->dwOfs|0x01000000);		// unknown button events
			}
			else //end joy buttons
			{
//#ifdef ULTRACADE
			
int DeadZ2;
const HRESULT hr = GetRegInt("Player", "DeadZone", &DeadZ2);
if (hr != S_OK)
		{
		DeadZ2=0;
		}
//debugging grab value of deadzone from registry
//char myCharString[8];
//itoa( DeadZ2, myCharString, 10 );
//ShowError(myCharString);
				// Convert to signed int
				union { int i; unsigned int ui; } u;
				u.ui = input->dwData;
				
				switch (input->dwOfs)		// Axis, Sliders and POV
				{
					case DIJOFS_X: 
						if( g_pplayer ) //joyk  rotLeftManual
						{
							//Deadzone X Axis
							if((u.i<=0) && (u.i>=DeadZ2*(-10))){u.i = 0;}
							if((u.i>=0) && (u.i<=DeadZ2*10)){u.i = 0;}
							if((u.i<0) && (u.i<DeadZ2*(-10))){u.i = u.i + DeadZ2*10;}
							if((u.i>0) && (u.i>DeadZ2*10)){u.i = u.i - DeadZ2*10;}
							if (uShockType == USHOCKTYPE_PBWIZARD) 
							{
								g_pplayer->UltraNudgeX(-u.i, joyk); //rotate to match Pinball Wizard
							}
							if (uShockType == USHOCKTYPE_ULTRACADE)
							{
								if (rotLeftManual)
								{
									g_pplayer->UltraNudgeX(u.i, joyk);
								}
								else
								{
								g_pplayer->UltraNudgeY(-u.i, joyk); //rotate to match joystick
								}
							}
							if (uShockType == USHOCKTYPE_GENERIC)
							{
								g_pplayer->UltraNudgeX(-u.i, joyk);
							}
							if (uShockType == USHOCKTYPE_SIDEWINDER) 
							{
								g_pplayer->UltraNudgeX(u.i, joyk);
							}
						}
						break;

					case DIJOFS_Y: 
						if( g_pplayer )
						{
							//Deadzone Y Axis
							if((u.i<=0) && (u.i>=DeadZ2*(-10))){u.i = 0;}
							if((u.i>=0) && (u.i<=DeadZ2*10)){u.i = 0;}
							if((u.i<0) && (u.i<DeadZ2*(-10))){u.i = u.i + DeadZ2*10;}
							if((u.i>0) && (u.i>DeadZ2*10)){u.i = u.i - DeadZ2*10;}
							if (uShockType == USHOCKTYPE_PBWIZARD) 
							{
								g_pplayer->UltraNudgeY(u.i, joyk); //rotate to match Pinball Wizard
							}
							if (uShockType == USHOCKTYPE_ULTRACADE) 
							{
								if (rotLeftManual)
								{
									g_pplayer->UltraNudgeY(u.i, joyk);
								}
								else
								{
								g_pplayer->UltraNudgeX(-u.i, joyk); //rotate to match joystick
								}
							}
							if (uShockType == USHOCKTYPE_GENERIC)
							{
								g_pplayer->UltraNudgeY(u.i, joyk);
							}
							if (uShockType == USHOCKTYPE_SIDEWINDER) 
							{
								g_pplayer->UltraNudgeY(u.i, joyk);
							}
						}
						break;

					case DIJOFS_Z:
						if( g_pplayer )
						{
							if ((m_plunger_z == 1) && (m_plunger_rz == 0))		// can disable use of Z Axis for PBW plunger to use the
							{													// rZ Axis instead, which if enabled disables the Z Axis
								if (uShockType == USHOCKTYPE_PBWIZARD) 
								{
									g_pplayer->mechPlungerIn(-u.i);
								}
								if (uShockType == USHOCKTYPE_GENERIC)
								{
									if (m_plunger_reverse == 0)
									{
										g_pplayer->mechPlungerIn(-u.i);
									}
									else
									{
										g_pplayer->mechPlungerIn(u.i);
									}
								}
								if (uShockType == USHOCKTYPE_ULTRACADE)
								{
									g_pplayer->mechPlungerIn(u.i);
								}
							}
						}
						break;

					case DIJOFS_RX:
						if( g_pplayer )
						{
							if ((uShockType == USHOCKTYPE_GENERIC) && (m_plunger_rx == 1))
							{
								if (m_plunger_reverse == 0)
								{
									g_pplayer->mechPlungerIn(-u.i);
								}
								else
								{
									g_pplayer->mechPlungerIn(u.i);
								}
							}
						}
						break;

					case DIJOFS_RY:
						if ( g_pplayer )
						{
							if ((uShockType == USHOCKTYPE_GENERIC) && (m_plunger_ry == 1))
							{
								if (m_plunger_reverse == 0)
								{
									g_pplayer->mechPlungerIn(-u.i);
								}
								else
								{
									g_pplayer->mechPlungerIn(u.i);
								}
							}
						}
						break;

					case DIJOFS_RZ:
						if( g_pplayer )
						{
							if (uShockType == USHOCKTYPE_PBWIZARD) 
							{
								g_pplayer->mechPlungerIn(u.i);
							}
							if ((uShockType == USHOCKTYPE_GENERIC) && (m_plunger_rz == 1))
							{
								if (m_plunger_reverse == 0)
								{
									g_pplayer->mechPlungerIn(-u.i);
								}
								else
								{
									g_pplayer->mechPlungerIn(u.i);
								}
							}
						}
						break;

					case DIJOFS_SLIDER(0):
						if( g_pplayer )
						{
							if (m_plunger_reverse == 0)
							{
								if (uShockType == USHOCKTYPE_SIDEWINDER)
								{
									g_pplayer->mechPlungerIn(-u.i);
								}
								if ((uShockType == USHOCKTYPE_GENERIC) && (m_plunger_slider == 1))
								{
									g_pplayer->mechPlungerIn(-u.i);
								}
							}
							else
							{
								if (uShockType == USHOCKTYPE_SIDEWINDER)
								{
									g_pplayer->mechPlungerIn(u.i);
								}
								if ((uShockType == USHOCKTYPE_GENERIC) && (m_plunger_slider == 1))
								{
									g_pplayer->mechPlungerIn(u.i);
								}
							}
						}
						break;

					case DIJOFS_POV(0):
						break;
						default:
						break;
				}
			}
		}
	}
}

int PinInput::GetNextKey() // return last valid keyboard key 
{
	if (m_pKeyboard != NULL && s_pPinInput != NULL)
	{
		DIDEVICEOBJECTDATA didod[1];  // Receives buffered data
		DWORD dwElements;
		HRESULT hr;
		LPDIRECTINPUTDEVICE pkyb = s_pPinInput->m_pKeyboard;

		for (int j = 0; j < 2; ++j)
		{
			dwElements = 1;
			hr = m_pKeyboard->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), didod, &dwElements, 0);

			if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED) pkyb->Acquire();
			else break;
		}

		if ((hr == S_OK || hr == DI_BUFFEROVERFLOW) && dwElements != 0) return didod[0].dwOfs;
	}
	return 0;
}
// Returns non-zero if the key was pressed.
U32 Pressed( U32 val )
{
	return PinInput::m_ChangedKeys & ( PinInput::m_PreviousKeys & val );
}

// Returns non-zero if the key was released.
U32 Released( U32 val )
{
	return PinInput::m_ChangedKeys & (~PinInput::m_PreviousKeys);
}

// Returns non-zero if the key is held down.
U32 Held( U32 val )
{
	return PinInput::m_PreviousKeys & val;
}

// Returns non-zero if the key is held down.
U32 Down( U32 val )
{
	return Held( val );
}

// Returns non-zero if the key was changed.
U32 Changed( U32 val )
{
	return PinInput::m_ChangedKeys & val;
}
