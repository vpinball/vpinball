#include "StdAfx.h"

#define INPUT_BUFFER_SIZE 32

static PinInput *s_pPinInput;

U32 PinInput::m_PreviousKeys;
U32 PinInput::m_ChangedKeys;
//int InputControlRun;

int e_JoyCnt;
int uShockDevice = -1;	// only one uShock device
int uShockType = 0;
bool fe_message_sent = false;

int m_plunger_axis = 0;
int m_lr_axis = 0;
int m_ud_axis = 0;
int m_plunger_reverse = 0;
int m_lr_axis_reverse = 0;
int m_ud_axis_reverse = 0;
int m_override_default_buttons = 0;
int m_joylflipkey = 0;
int m_joyrflipkey = 0;
int m_joylmagnasave = 0;
int m_joyrmagnasave = 0;
int m_joyplungerkey = 0;
int m_joystartgamekey = 0;
int m_joyexitgamekey = 0;
int m_joyaddcreditkey = 0;
int m_joyaddcreditkey2 = 0;
int m_joyframecount = 0;
int m_joyvolumeup = 0;
int m_joyvolumedown = 0;
int m_joylefttilt = 0;
int m_joycentertilt = 0;
int m_joyrighttilt = 0;
int m_joypmbuyin = 0;
int m_joypmcoin3 = 0;
int m_joypmcoin4 = 0;
int m_joypmcoindoor = 0;
int m_joypmcancel = 0;
int m_joypmdown = 0;
int m_joypmup = 0;
int m_joypmenter = 0;

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

	ZeroMemory( m_diq, sizeof( m_diq ) );

	s_pPinInput = this;

	e_JoyCnt = 0;
	//m_pJoystick = NULL;
	for (int k = 0; k < PININ_JOYMXCNT; ++k) m_pJoystick[k] = NULL;

	HRESULT hr;
	int tmp;

	hr = GetRegInt("Player", "LRAxis", &tmp);
	if (hr == S_OK) m_lr_axis = tmp;

	hr = GetRegInt("Player", "UDAxis", &tmp);
	if (hr == S_OK) m_ud_axis = tmp;

	hr = GetRegInt("Player", "LRAxisFlip", &tmp);
	if (hr == S_OK) m_lr_axis_reverse = tmp;

	hr = GetRegInt("Player", "UDAxisFlip", &tmp);
	if (hr == S_OK) m_ud_axis_reverse = tmp;

	hr = GetRegInt("Player", "PlungerAxis", &tmp);
	if (hr == S_OK) m_plunger_axis = tmp;

	hr = GetRegInt("Player", "ReversePlungerAxis", &tmp);
	if (hr == S_OK) m_plunger_reverse = tmp;

	hr = GetRegInt("Player", "PBWDefaultLayout", &tmp);
	if (hr == S_OK) m_override_default_buttons = tmp;

	hr = GetRegInt("Player", "JoyLFlipKey", &tmp);
	if (hr == S_OK) m_joylflipkey = tmp;

	hr = GetRegInt("Player", "JoyRFlipKey", &tmp);
	if (hr == S_OK) m_joyrflipkey = tmp;

	hr = GetRegInt("Player", "JoyPlungerKey", &tmp);
	if (hr == S_OK) m_joyplungerkey = tmp;

	hr = GetRegInt("Player", "JoyAddCreditKey", &tmp);
	if (hr == S_OK) m_joyaddcreditkey = tmp;

	hr = GetRegInt("Player", "JoyAddCredit2Key", &tmp);
	if (hr == S_OK) m_joyaddcreditkey2 = tmp;

	hr = GetRegInt("Player", "JoyLMagnaSave", &tmp);
	if (hr == S_OK) m_joylmagnasave = tmp;

	hr = GetRegInt("Player", "JoyRMagnaSave", &tmp);
	if (hr == S_OK) m_joyrmagnasave = tmp;

	hr = GetRegInt("Player", "JoyStartGameKey", &tmp);
	if (hr == S_OK) m_joystartgamekey = tmp;

	hr = GetRegInt("Player", "JoyFrameCount", &tmp);
	if (hr == S_OK) m_joyframecount = tmp;

	hr = GetRegInt("Player", "JoyExitGameKey", &tmp);
	if (hr == S_OK) m_joyexitgamekey = tmp;

	hr = GetRegInt("Player", "JoyVolumeUp", &tmp);
	if (hr == S_OK) m_joyvolumeup = tmp;

	hr = GetRegInt("Player", "JoyVolumeDown", &tmp);
	if (hr == S_OK) m_joyvolumedown = tmp;

	hr = GetRegInt("Player", "JoyLTiltKey", &tmp);
	if (hr == S_OK) m_joylefttilt = tmp;

	hr = GetRegInt("Player", "JoyCTiltKey", &tmp);
	if (hr == S_OK) m_joycentertilt = tmp;

	hr = GetRegInt("Player", "JoyRTiltKey", &tmp);
	if (hr == S_OK) m_joyrighttilt = tmp;

	hr = GetRegInt("Player", "JoyPMBuyIn", &tmp);
	if (hr == S_OK) m_joypmbuyin = tmp;

	hr = GetRegInt("Player", "JoyPMCoin3", &tmp);
	if (hr == S_OK) m_joypmcoin3 = tmp;

	hr = GetRegInt("Player", "JoyPMCoin4", &tmp);
	if (hr == S_OK) m_joypmcoin4 = tmp;

	hr = GetRegInt("Player", "JoyPMCoinDoor", &tmp);
	if (hr == S_OK) m_joypmcoindoor = tmp;

	hr = GetRegInt("Player", "JoyPMCancel", &tmp);
	if (hr == S_OK) m_joypmcancel = tmp;

	hr = GetRegInt("Player", "JoyPMDown", &tmp);
	if (hr == S_OK) m_joypmdown = tmp;

	hr = GetRegInt("Player", "JoyPMUp", &tmp);
	if (hr == S_OK) m_joypmup = tmp;

	hr = GetRegInt("Player", "JoyPMEnter", &tmp);
	if (hr == S_OK) m_joypmenter = tmp;
}


PinInput::~PinInput()
{
	s_pPinInput = NULL;
}


//-----------------------------------------------------------------------------
// Name: EnumObjectsCallback()
// Desc: Callback function for enumerating objects (axes, buttons, POVs) on a 
//		joystick. This function enables user interface elements for objects
//		that are found to exist, and scales axes min/max values.
//-----------------------------------------------------------------------------
BOOL CALLBACK EnumObjectsCallback( const DIDEVICEOBJECTINSTANCE* pdidoi,
									VOID* pContext )
{
	//HWND hDlg = (HWND)pContext;
	PinInput * const ppinput = (PinInput *)pContext;

#ifdef _DEBUG
	static int nAxis = 0;
	static int nButtons = 0;
	static int nSliderCount = 0;	// Number of returned slider controls
	static int nPOVCount = 0;		// Number of returned POV controls
	static int nKey = 0;
	static int nUnknown = 0;
#endif

	// For axes that are returned, set the DIPROP_RANGE property for the
	// enumerated axis in order to scale min/max values.
	if( pdidoi->dwType & DIDFT_AXIS )
	{
		DIPROPRANGE diprg; 
		diprg.diph.dwSize		= sizeof(DIPROPRANGE); 
		diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
		diprg.diph.dwObj		= pdidoi->dwType; // Specify the enumerated axis
		diprg.diph.dwHow		= DIPH_BYID; 
		diprg.lMin				= JOYRANGEMN; 
		diprg.lMax				= JOYRANGEMX; 

		// Set the range for the axis
		if( FAILED( ppinput->m_pJoystick[e_JoyCnt]->SetProperty( DIPROP_RANGE, &diprg.diph ) ) ) 
			{return DIENUM_STOP;}   

		// set DEADBAND to Zero
		DIPROPDWORD dipdw;
		dipdw.diph.dwSize		= sizeof(DIPROPDWORD);
		dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
		dipdw.diph.dwObj		= pdidoi->dwType; // Specify the enumerated axis
		dipdw.diph.dwHow		= DIPH_BYID;
		dipdw.dwData			= 30;//g_pplayer->DeadZ; //allows for 0-100% deadzone in 1% increments
 
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
	
	//while (++InputControlRun < 0) Sleep(5);		// set to exit AND WAIT

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

	ZeroMemory( m_diq, sizeof( m_diq ));
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

		 if( mkey == g_pplayer->m_rgKeys[ePlungerKey]		) val |= PININ_PLUNGE;
	else if( mkey == g_pplayer->m_rgKeys[eAddCreditKey]		) val |= PININ_COIN1;
	else if( mkey == g_pplayer->m_rgKeys[eAddCreditKey2]	) val |= PININ_COIN2;
	else if( mkey == g_pplayer->m_rgKeys[eStartGameKey]		) val |= PININ_START;
	else if( mkey == g_pplayer->m_rgKeys[eVolumeUp]			) val |= PININ_VOL_UP;
	else if( mkey == g_pplayer->m_rgKeys[eVolumeDown]		) val |= PININ_VOL_DOWN;
	else if( mkey == g_pplayer->m_rgKeys[eExitGame]			) val |= PININ_EXITGAME;
	else if( mkey == g_pplayer->m_rgKeys[eEnable3D]			) val |= PININ_ENABLE3D;

	else if( mkey == DIK_ESCAPE	) val |= PININ_EXITGAME;
	else if( mkey == DIK_UP		) val |= PININ_START;
	else if( mkey == DIK_DOWN	) val |= PININ_PLUNGE;
	else if( mkey == DIK_1		) val |= PININ_START;
	else if( mkey == DIK_2		) val |= PININ_BUYIN;
	else if( mkey == DIK_3		) val |= PININ_COIN1;
	else if( mkey == DIK_4		) val |= PININ_COIN2;
	else if( mkey == DIK_5		) val |= PININ_COIN3;
	else if( mkey == DIK_6		) val |= PININ_COIN4;
	else if( mkey == DIK_7		) val |= PININ_SERVICECANCEL;
	else if( mkey == DIK_8		) val |= PININ_SERVICE1;
	else if( mkey == DIK_9		) val |= PININ_SERVICE2;
	else if( mkey == DIK_0		) val |= PININ_SERVICEENTER;
	else if( mkey == DIK_END	) val |= PININ_COINDOOR;
//	else if( mkey == DIK_EQUALS	) val |= PININ_VOL_UP;
//	else if( mkey == DIK_MINUS	) val |= PININ_VOL_DOWN;
	else if( mkey == DIK_Z		) val |= PININ_LTILT;
	else if( mkey == DIK_SPACE	) val |= PININ_CTILT;
	else if( mkey == DIK_SLASH	) val |= PININ_RTILT;
	else if( mkey == DIK_T		) val |= PININ_MTILT;
	else if( mkey == DIK_F11	) val |= PININ_FRAMES;

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
		const U32 curr_time_msec = msec();
		if( (firedautocoin > 0) &&														// Initialized.
			(down == 1) &&																// Coin button is down.
			((curr_time_msec - firedautocoin) > 100) )											// Coin button has been down for at least 0.10 seconds.
		{
			// Release coin button.
			firedautocoin = curr_time_msec;
			down = 0;
			FireKeyEvent( DISPID_GameEvents_KeyUp, g_pplayer->m_rgKeys[eAddCreditKey] );

			// Update the counter.
			Coins--;

			OutputDebugString( "**Autocoin: Release.\n" );
		}

		// Logic to do "autocoin"
		if( (down == 0) &&																// Coin button is up.
			(((didonce == 1) && ((curr_time_msec - firedautocoin) > 500))) ||					// Last attempt was at least 0.50 seconds ago.
			((didonce == 0) && ((curr_time_msec - firedautocoin) > ((U32)(secs*1000.0f)))) )	// Never attempted and at least autostart seconds have elapsed.
		{
			// Press coin button.
			firedautocoin = curr_time_msec;
			down = 1;
			didonce = 1;
			FireKeyEvent( DISPID_GameEvents_KeyDown, g_pplayer->m_rgKeys[eAddCreditKey] );

			OutputDebugString( "**Autocoin: Press.\n" );
		}
	}

}


void PinInput::autostart( F32 secs, F32 retrysecs )
{

//	if( !VPinball::m_open_minimized ) 
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

	const U32 curr_time_msec = msec();
		
	if( (firedautostart > 0) &&				// Initialized.
		(down == 1) &&						// Start button is down.
		((curr_time_msec - firedautostart) > 100) )	// Start button has been down for at least 0.10 seconds.
	{
		// Release start.
		firedautostart = curr_time_msec;
		down = 0;
		FireKeyEvent( DISPID_GameEvents_KeyUp, g_pplayer->m_rgKeys[eStartGameKey] );

		OutputDebugString( "Autostart: Release.\n" );
	}

	// Logic to do "autostart"
	if( (down == 0) &&																					// Start button is up.
		(((didonce == 1) && !started() && ((curr_time_msec - firedautostart) > ((U32)(retrysecs*1000.0f))))) ||	// Not started and last attempt was at least AutoStartRetry seconds ago.
		((didonce == 0) && ((curr_time_msec - firedautostart) > ((U32)(secs*1000.0f)))) )						// Never attempted and autostart time has elapsed.
	{
		// Check if we haven't accounted for the play.
		if ( (didonce == 0) &&								// Never attempted autostarted.
							(!started()) )					// Player hasn't already started manually.
		{
			// Update the number of plays.
			VPinball::NumPlays++;
			SetRegValue( "Statistics", "NumPlays", REG_DWORD, &(VPinball::NumPlays), 4 );
		}

		// Press start.
		firedautostart = curr_time_msec;
		down = 1;
		didonce = 1;
		FireKeyEvent( DISPID_GameEvents_KeyDown, g_pplayer->m_rgKeys[eStartGameKey] );

		OutputDebugString( "Autostart: Press.\n" );
	}

}



void PinInput::autoexit( F32 secs )
{
	if (( !VPinball::m_open_minimized ) ||
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
	const U32 curr_time_msec = msec();

	if( !first_stamp ) 
	{
		first_stamp = curr_time_msec;
	}

	// Don't allow button exit until after game has been running for 1 second.
	if( curr_time_msec - first_stamp < 1000 ) 
		return; 

	// Check if we can exit.
	if( (exit_stamp) &&										// Initialized.
		((curr_time_msec - exit_stamp) > (secs * 1000.0f)) &&		// Held exit button for number of seconds.
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

	const U32 curr_time_msec = msec();

	// Check if we've been initialized.
	if( firedautostart == 0 )
	{
		firedautostart = curr_time_msec;
	}

	// Check if we've been initialized.
	if( firedautocoin == 0 )
	{
		firedautocoin = curr_time_msec;
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

			if (input->dwOfs >= DIJOFS_BUTTON0 && input->dwOfs <= DIJOFS_BUTTON31)
			{
				int updown = (input->dwData & 0x80)?DISPID_GameEvents_KeyDown:DISPID_GameEvents_KeyUp;
				if (input->dwOfs == DIJOFS_BUTTON0)
				{
					 if ((uShockType == USHOCKTYPE_PBWIZARD) && (m_override_default_buttons == 0)) // plunge
						{	FireKeyEvent( updown,g_pplayer->m_rgKeys[ePlungerKey] );}
					else if((uShockType == USHOCKTYPE_ULTRACADE) && (m_override_default_buttons == 0)) // coin 1
						{	FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey] );}
					else if (m_joylflipkey == 1){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftFlipperKey] );}
					else if (m_joyrflipkey == 1){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightFlipperKey] );}
					else if (m_joyplungerkey == 1){FireKeyEvent( updown,g_pplayer->m_rgKeys[ePlungerKey] );}
					else if (m_joyaddcreditkey == 1){FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey] );}
					else if (m_joyaddcreditkey2 == 1){FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey2] );}
					else if (m_joylmagnasave == 1){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftMagnaSave] );}
					else if (m_joyrmagnasave == 1){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightMagnaSave] );}
					else if (m_joystartgamekey == 1)
						{	if( ((curr_time_msec - firedautostart) > ((U32)(ptable->m_tblAutoStart*1000.0f))) || (pressed_start) || started() ) 
							{	pressed_start = 1;
								FireKeyEvent( updown,g_pplayer->m_rgKeys[eStartGameKey] );
								if ( updown == DISPID_GameEvents_KeyDown )
								{	VPinball::NumPlays++;
									SetRegValue( "Statistics", "NumPlays", REG_DWORD, &(VPinball::NumPlays), 4 );
						}	}	}
					else if (m_joyexitgamekey == 1)
						{	if( DISPID_GameEvents_KeyDown == updown ) 
								{g_pplayer->m_fCloseDown = fTrue;}
						}
					else if (m_joyframecount == 1)
						{	if( DISPID_GameEvents_KeyDown == updown ) 
							{g_pplayer->ToggleFPS();}
						}
					else if (m_joyvolumeup == 1){FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeUp] );}
					else if (m_joyvolumedown == 1){FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeDown] );}
					else if (m_joylefttilt == 1){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftTiltKey] );}
					else if (m_joycentertilt == 1){FireKeyEvent( updown,g_pplayer->m_rgKeys[eCenterTiltKey] );}
					else if (m_joyrighttilt == 1){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightTiltKey] );}
					else if (m_joypmbuyin == 1) FireKeyEvent (updown, DIK_2);
					else if (m_joypmcoin3 == 1) FireKeyEvent (updown, DIK_5);
					else if (m_joypmcoin4 == 1) FireKeyEvent (updown, DIK_6);
					else if (m_joypmcoindoor == 1) FireKeyEvent (updown, DIK_END);
					else if (m_joypmcancel == 1) FireKeyEvent (updown, DIK_7);
					else if (m_joypmdown == 1) FireKeyEvent (updown, DIK_8);
					else if (m_joypmup == 1) FireKeyEvent (updown, DIK_9);
					else if (m_joypmenter == 1) FireKeyEvent (updown, DIK_0);
				}

			else if (input->dwOfs == DIJOFS_BUTTON1)
				{
						 if ((uShockType == USHOCKTYPE_PBWIZARD) && (m_override_default_buttons == 0)) // right
							{	FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightFlipperKey] );}
					else if ((uShockType == USHOCKTYPE_ULTRACADE) && (m_override_default_buttons == 0)) // coin 2
							{	FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey2] );}
					else if (m_joylflipkey == 2){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftFlipperKey] );}
					else if (m_joyrflipkey == 2){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightFlipperKey] );}
					else if (m_joyplungerkey == 2){FireKeyEvent( updown,g_pplayer->m_rgKeys[ePlungerKey] );}
					else if (m_joyaddcreditkey == 2){FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey] );}
					else if (m_joyaddcreditkey2 == 2){FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey2] );}
					else if (m_joylmagnasave == 2){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftMagnaSave] );}
					else if (m_joyrmagnasave == 2){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightMagnaSave] );}
					else if (m_joystartgamekey == 2)
						{	if( ((curr_time_msec - firedautostart) > ((U32)(ptable->m_tblAutoStart*1000.0f))) || (pressed_start) || started() ) 
							{	pressed_start = 1;
								FireKeyEvent( updown,g_pplayer->m_rgKeys[eStartGameKey] );
								if ( updown == DISPID_GameEvents_KeyDown )
								{	VPinball::NumPlays++;
									SetRegValue( "Statistics", "NumPlays", REG_DWORD, &(VPinball::NumPlays), 4 );
						}	}	}
					else if (m_joyexitgamekey == 2)
						{	if( DISPID_GameEvents_KeyDown == updown ) 
							{g_pplayer->m_fCloseDown = fTrue;}
						}
					else if (m_joyframecount == 2)
						{	if( DISPID_GameEvents_KeyDown == updown ) 
							{g_pplayer->ToggleFPS();}
						}
					else if (m_joyvolumeup == 2){FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeUp] );}
					else if (m_joyvolumedown == 2){FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeDown] );}
					else if (m_joylefttilt == 2){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftTiltKey] );}
					else if (m_joycentertilt == 2){FireKeyEvent( updown,g_pplayer->m_rgKeys[eCenterTiltKey] );}
					else if (m_joyrighttilt == 2){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightTiltKey] );}
					else if (m_joypmbuyin == 2) FireKeyEvent (updown, DIK_2);
					else if (m_joypmcoin3 == 2) FireKeyEvent (updown, DIK_5);
					else if (m_joypmcoin4 == 2) FireKeyEvent (updown, DIK_6);
					else if (m_joypmcoindoor == 2) FireKeyEvent (updown, DIK_END);
					else if (m_joypmcancel == 2) FireKeyEvent (updown, DIK_7);
					else if (m_joypmdown == 2) FireKeyEvent (updown, DIK_8);
					else if (m_joypmup == 2) FireKeyEvent (updown, DIK_9);
					else if (m_joypmenter == 2) FireKeyEvent (updown, DIK_0);
				}

			else if (input->dwOfs == DIJOFS_BUTTON2)
				{
						 if ((uShockType == USHOCKTYPE_PBWIZARD) && (uShockType == USHOCKTYPE_ULTRACADE) && (m_override_default_buttons == 0))
							{FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightMagnaSave ] );} // right2
					else if (m_joylflipkey == 3){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftFlipperKey] );}
					else if (m_joyrflipkey == 3){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightFlipperKey] );}
					else if (m_joyplungerkey == 3){FireKeyEvent( updown,g_pplayer->m_rgKeys[ePlungerKey] );}
					else if (m_joyaddcreditkey == 3){FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey] );}
					else if (m_joyaddcreditkey2 == 3){FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey2] );}
					else if (m_joylmagnasave == 3){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftMagnaSave] );}
					else if (m_joyrmagnasave == 3){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightMagnaSave] );}
					else if (m_joystartgamekey == 3)
						{	if( ((curr_time_msec - firedautostart) > ((U32)(ptable->m_tblAutoStart*1000.0f))) || (pressed_start) || started() ) 
							{	pressed_start = 1;
								FireKeyEvent( updown,g_pplayer->m_rgKeys[eStartGameKey] );
								if ( updown == DISPID_GameEvents_KeyDown )
								{	VPinball::NumPlays++;
									SetRegValue( "Statistics", "NumPlays", REG_DWORD, &(VPinball::NumPlays), 4 );
						}	}	}
					else if (m_joyexitgamekey == 3)
						{	if( DISPID_GameEvents_KeyDown == updown ) 
							{g_pplayer->m_fCloseDown = fTrue;}
						}
					else if (m_joyframecount == 3)
						{	if( DISPID_GameEvents_KeyDown == updown ) 
							{g_pplayer->ToggleFPS();}
						}
					else if (m_joyvolumeup == 3){FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeUp] );}
					else if (m_joyvolumedown == 3){FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeDown] );}
					else if (m_joylefttilt == 3){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftTiltKey] );}
					else if (m_joycentertilt == 3){FireKeyEvent( updown,g_pplayer->m_rgKeys[eCenterTiltKey] );}
					else if (m_joyrighttilt == 3){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightTiltKey] );}
					else if (m_joypmbuyin == 3) FireKeyEvent (updown, DIK_2);
					else if (m_joypmcoin3 == 3) FireKeyEvent (updown, DIK_5);
					else if (m_joypmcoin4 == 3) FireKeyEvent (updown, DIK_6);
					else if (m_joypmcoindoor == 3) FireKeyEvent (updown, DIK_END);
					else if (m_joypmcancel == 3) FireKeyEvent (updown, DIK_7);
					else if (m_joypmdown == 3) FireKeyEvent (updown, DIK_8);
					else if (m_joypmup == 3) FireKeyEvent (updown, DIK_9);
					else if (m_joypmenter == 3) FireKeyEvent (updown, DIK_0);
				}

			else if (input->dwOfs == DIJOFS_BUTTON3)
				{
						 if ((uShockType == USHOCKTYPE_PBWIZARD) && (m_override_default_buttons == 0)) // volume down
							{	FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeDown] );}
					else if (m_joylflipkey == 4){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftFlipperKey] );}
					else if (m_joyrflipkey == 4){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightFlipperKey] );}
					else if (m_joyplungerkey == 4){FireKeyEvent( updown,g_pplayer->m_rgKeys[ePlungerKey] );}
					else if (m_joyaddcreditkey == 4){FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey] );}
					else if (m_joyaddcreditkey2 == 4){FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey2] );}
					else if (m_joylmagnasave == 4){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftMagnaSave] );}
					else if (m_joyrmagnasave == 4){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightMagnaSave] );}
					else if (m_joystartgamekey == 4)
						{	if( ((curr_time_msec - firedautostart) > ((U32)(ptable->m_tblAutoStart*1000.0f))) || (pressed_start) || started() ) 
							{	pressed_start = 1;
								FireKeyEvent( updown,g_pplayer->m_rgKeys[eStartGameKey] );
								if ( updown == DISPID_GameEvents_KeyDown )
								{	VPinball::NumPlays++;
									SetRegValue( "Statistics", "NumPlays", REG_DWORD, &(VPinball::NumPlays), 4 );
						}	}	}
					else if (m_joyexitgamekey == 4)
						{	if( DISPID_GameEvents_KeyDown == updown ) 
							{g_pplayer->m_fCloseDown = fTrue;}
						}
					else if (m_joyframecount == 4)
						{	if( DISPID_GameEvents_KeyDown == updown ) 
							{g_pplayer->ToggleFPS();}
						}
					else if (m_joyvolumeup == 4){FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeUp] );}
					else if (m_joyvolumedown == 4){FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeDown] );}
					else if (m_joylefttilt == 4){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftTiltKey] );}
					else if (m_joycentertilt == 4){FireKeyEvent( updown,g_pplayer->m_rgKeys[eCenterTiltKey] );}
					else if (m_joyrighttilt == 4){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightTiltKey] );}
					else if (m_joypmbuyin == 4) FireKeyEvent (updown, DIK_2);
					else if (m_joypmcoin3 == 4) FireKeyEvent (updown, DIK_5);
					else if (m_joypmcoin4 == 4) FireKeyEvent (updown, DIK_6);
					else if (m_joypmcoindoor == 4) FireKeyEvent (updown, DIK_END);
					else if (m_joypmcancel == 4) FireKeyEvent (updown, DIK_7);
					else if (m_joypmdown == 4) FireKeyEvent (updown, DIK_8);
					else if (m_joypmup == 4) FireKeyEvent (updown, DIK_9);
					else if (m_joypmenter == 4) FireKeyEvent (updown, DIK_0);
				}

			else if (input->dwOfs == DIJOFS_BUTTON4)
				{
						 if ((uShockType == USHOCKTYPE_PBWIZARD) && (m_override_default_buttons == 0)) // volume up
							{	FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeUp] );}
					else if (m_joylflipkey == 5){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftFlipperKey] );}
					else if (m_joyrflipkey == 5){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightFlipperKey] );}
					else if (m_joyplungerkey == 5){FireKeyEvent( updown,g_pplayer->m_rgKeys[ePlungerKey] );}
					else if (m_joyaddcreditkey == 5){FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey] );}
					else if (m_joyaddcreditkey2 == 5){FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey2] );}
					else if (m_joylmagnasave == 5){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftMagnaSave] );}
					else if (m_joyrmagnasave == 5){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightMagnaSave] );}
					else if (m_joystartgamekey == 5)
						{	if( ((curr_time_msec - firedautostart) > ((U32)(ptable->m_tblAutoStart*1000.0f))) || (pressed_start) || started() ) 
							{	pressed_start = 1;
								FireKeyEvent( updown,g_pplayer->m_rgKeys[eStartGameKey] );
								if ( updown == DISPID_GameEvents_KeyDown )
								{	VPinball::NumPlays++;
									SetRegValue( "Statistics", "NumPlays", REG_DWORD, &(VPinball::NumPlays), 4 );
						}	}	}
					else if (m_joyexitgamekey == 5)
						{	if( DISPID_GameEvents_KeyDown == updown ) 
							{g_pplayer->m_fCloseDown = fTrue;}
						}
					else if (m_joyframecount == 5)
						{	if( DISPID_GameEvents_KeyDown == updown ) 
							{g_pplayer->ToggleFPS();}
						}
					else if (m_joyvolumeup == 5){FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeUp] );}
					else if (m_joyvolumedown == 5){FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeDown] );}
					else if (m_joylefttilt == 5){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftTiltKey] );}
					else if (m_joycentertilt == 5){FireKeyEvent( updown,g_pplayer->m_rgKeys[eCenterTiltKey] );}
					else if (m_joyrighttilt == 5){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightTiltKey] );}
					else if (m_joypmbuyin == 5) FireKeyEvent (updown, DIK_2);
					else if (m_joypmcoin3 == 5) FireKeyEvent (updown, DIK_5);
					else if (m_joypmcoin4 == 5) FireKeyEvent (updown, DIK_6);
					else if (m_joypmcoindoor == 5) FireKeyEvent (updown, DIK_END);
					else if (m_joypmcancel == 5) FireKeyEvent (updown, DIK_7);
					else if (m_joypmdown == 5) FireKeyEvent (updown, DIK_8);
					else if (m_joypmup == 5) FireKeyEvent (updown, DIK_9);
					else if (m_joypmenter == 5) FireKeyEvent (updown, DIK_0);
				}

			else if (input->dwOfs == DIJOFS_BUTTON5)
				{
						 if ((uShockType == USHOCKTYPE_ULTRACADE) && (m_override_default_buttons == 0)) // volume up
							{	FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeUp] );}
					else if (m_joylflipkey == 6){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftFlipperKey] );}
					else if (m_joyrflipkey == 6){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightFlipperKey] );}
					else if (m_joyplungerkey == 6){FireKeyEvent( updown,g_pplayer->m_rgKeys[ePlungerKey] );}
					else if (m_joyaddcreditkey == 6){FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey] );}
					else if (m_joyaddcreditkey2 == 6){FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey2] );}
					else if (m_joylmagnasave == 6){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftMagnaSave] );}
					else if (m_joyrmagnasave == 6){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightMagnaSave] );}
					else if (m_joystartgamekey == 6)
						{	if( ((curr_time_msec - firedautostart) > ((U32)(ptable->m_tblAutoStart*1000.0f))) || (pressed_start) || started() ) 
							{	pressed_start = 1;
								FireKeyEvent( updown,g_pplayer->m_rgKeys[eStartGameKey] );
								if ( updown == DISPID_GameEvents_KeyDown )
								{	VPinball::NumPlays++;
									SetRegValue( "Statistics", "NumPlays", REG_DWORD, &(VPinball::NumPlays), 4 );
						}	}	}
					else if (m_joyexitgamekey == 6)
						{	if( DISPID_GameEvents_KeyDown == updown ) 
							{g_pplayer->m_fCloseDown = fTrue;}
						}
					else if (m_joyframecount == 6)
						{	if( DISPID_GameEvents_KeyDown == updown ) 
							{g_pplayer->ToggleFPS();}
						}
					else if (m_joyvolumeup == 6){FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeUp] );}
					else if (m_joyvolumedown == 6){FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeDown] );}
					else if (m_joylefttilt == 6){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftTiltKey] );}
					else if (m_joycentertilt == 6){FireKeyEvent( updown,g_pplayer->m_rgKeys[eCenterTiltKey] );}
					else if (m_joyrighttilt == 6){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightTiltKey] );}
					else if (m_joypmbuyin == 6) FireKeyEvent (updown, DIK_2);
					else if (m_joypmcoin3 == 6) FireKeyEvent (updown, DIK_5);
					else if (m_joypmcoin4 == 6) FireKeyEvent (updown, DIK_6);
					else if (m_joypmcoindoor == 6) FireKeyEvent (updown, DIK_END);
					else if (m_joypmcancel == 6) FireKeyEvent (updown, DIK_7);
					else if (m_joypmdown == 6) FireKeyEvent (updown, DIK_8);
					else if (m_joypmup == 6) FireKeyEvent (updown, DIK_9);
					else if (m_joypmenter == 6) FireKeyEvent (updown, DIK_0);
				}

			else if (input->dwOfs == DIJOFS_BUTTON6)
				{
						 if ((uShockType == USHOCKTYPE_PBWIZARD) && (m_override_default_buttons == 0)) // pause menu
							{	if( DISPID_GameEvents_KeyDown == updown ) g_pplayer->m_fCloseDown = fTrue;}
					else if ((uShockType == USHOCKTYPE_ULTRACADE) && (m_override_default_buttons == 0)) // volume down
							{	FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeDown] );}
					else if (m_joylflipkey == 7){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftFlipperKey] );}
					else if (m_joyrflipkey == 7){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightFlipperKey] );}
					else if (m_joyplungerkey == 7){FireKeyEvent( updown,g_pplayer->m_rgKeys[ePlungerKey] );}
					else if (m_joyaddcreditkey == 7){FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey] );}
					else if (m_joyaddcreditkey2 == 7){FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey2] );}
					else if (m_joylmagnasave == 7){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftMagnaSave] );}
					else if (m_joyrmagnasave == 7){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightMagnaSave] );}
					else if (m_joystartgamekey == 7)
						{	if( ((curr_time_msec - firedautostart) > ((U32)(ptable->m_tblAutoStart*1000.0f))) || (pressed_start) || started() ) 
							{	pressed_start = 1;
								FireKeyEvent( updown,g_pplayer->m_rgKeys[eStartGameKey] );
								if ( updown == DISPID_GameEvents_KeyDown )
								{	VPinball::NumPlays++;
									SetRegValue( "Statistics", "NumPlays", REG_DWORD, &(VPinball::NumPlays), 4 );
						}	}	}
					else if (m_joyexitgamekey == 7)
						{	if( DISPID_GameEvents_KeyDown == updown ) 
							{g_pplayer->m_fCloseDown = fTrue;}
						}
					else if (m_joyframecount == 7)
						{	if( DISPID_GameEvents_KeyDown == updown ) 
							{g_pplayer->ToggleFPS();}
						}
					else if (m_joyvolumeup == 7){FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeUp] );}
					else if (m_joyvolumedown == 7){FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeDown] );}
					else if (m_joylefttilt == 7){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftTiltKey] );}
					else if (m_joycentertilt == 7){FireKeyEvent( updown,g_pplayer->m_rgKeys[eCenterTiltKey] );}
					else if (m_joyrighttilt == 7){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightTiltKey] );}
					else if (m_joypmbuyin == 7) FireKeyEvent (updown, DIK_2);
					else if (m_joypmcoin3 == 7) FireKeyEvent (updown, DIK_5);
					else if (m_joypmcoin4 == 7) FireKeyEvent (updown, DIK_6);
					else if (m_joypmcoindoor == 7) FireKeyEvent (updown, DIK_END);
					else if (m_joypmcancel == 7) FireKeyEvent (updown, DIK_7);
					else if (m_joypmdown == 7) FireKeyEvent (updown, DIK_8);
					else if (m_joypmup == 7) FireKeyEvent (updown, DIK_9);
					else if (m_joypmenter == 7) FireKeyEvent (updown, DIK_0);
				}

			else if (input->dwOfs == DIJOFS_BUTTON7)
				{
						 if ((uShockType == USHOCKTYPE_PBWIZARD) && (m_override_default_buttons == 0)) // exit
							{	// Check if we have started a game yet.
								if ((started()) || (ptable->m_tblAutoStartEnabled == false))
								{	if( DISPID_GameEvents_KeyDown == updown ) 
									{	first_stamp = curr_time_msec;
										exit_stamp = curr_time_msec;
										FireKeyEvent( DISPID_GameEvents_KeyDown,g_pplayer->m_rgKeys[eExitGame ] );  
									}
									else 
									{	FireKeyEvent( DISPID_GameEvents_KeyUp,g_pplayer->m_rgKeys[eExitGame ] );  
										exit_stamp = 0;
							}	}	}
					else if (m_joylflipkey == 8){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftFlipperKey] );}
					else if (m_joyrflipkey == 8){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightFlipperKey] );}
					else if (m_joyplungerkey == 8){FireKeyEvent( updown,g_pplayer->m_rgKeys[ePlungerKey] );}
					else if (m_joyaddcreditkey == 8){FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey] );}
					else if (m_joyaddcreditkey2 == 8){FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey2] );}
					else if (m_joylmagnasave == 8){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftMagnaSave] );}
					else if (m_joyrmagnasave == 8){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightMagnaSave] );}
					else if (m_joystartgamekey == 8)
						{	if( ((curr_time_msec - firedautostart) > ((U32)(ptable->m_tblAutoStart*1000.0f))) || (pressed_start) || started() ) 
							{	pressed_start = 1;
								FireKeyEvent( updown,g_pplayer->m_rgKeys[eStartGameKey] );
								if ( updown == DISPID_GameEvents_KeyDown )
								{	VPinball::NumPlays++;
									SetRegValue( "Statistics", "NumPlays", REG_DWORD, &(VPinball::NumPlays), 4 );
						}	}	}
					else if (m_joyexitgamekey == 8)
						{	if( DISPID_GameEvents_KeyDown == updown ) 
							{g_pplayer->m_fCloseDown = fTrue;}
						}
					else if (m_joyframecount == 8)
						{	if( DISPID_GameEvents_KeyDown == updown ) 
							{g_pplayer->ToggleFPS();}
						}
					else if (m_joyvolumeup == 8){FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeUp] );}
					else if (m_joyvolumedown == 8){FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeDown] );}
					else if (m_joylefttilt == 8){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftTiltKey] );}
					else if (m_joycentertilt == 8){FireKeyEvent( updown,g_pplayer->m_rgKeys[eCenterTiltKey] );}
					else if (m_joyrighttilt == 8){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightTiltKey] );}
					else if (m_joypmbuyin == 8) FireKeyEvent (updown, DIK_2);
					else if (m_joypmcoin3 == 8) FireKeyEvent (updown, DIK_5);
					else if (m_joypmcoin4 == 8) FireKeyEvent (updown, DIK_6);
					else if (m_joypmcoindoor == 8) FireKeyEvent (updown, DIK_END);
					else if (m_joypmcancel == 8) FireKeyEvent (updown, DIK_7);
					else if (m_joypmdown == 8) FireKeyEvent (updown, DIK_8);
					else if (m_joypmup == 8) FireKeyEvent (updown, DIK_9);
					else if (m_joypmenter == 8) FireKeyEvent (updown, DIK_0);
				}

			else if (input->dwOfs == DIJOFS_BUTTON8)
				{
						 if ((uShockType == USHOCKTYPE_PBWIZARD) && (m_override_default_buttons == 0))
							{	if( ((curr_time_msec - firedautostart) > ((U32)(ptable->m_tblAutoStart*1000.0f))) || (pressed_start) || started() ) 
								{	pressed_start = 1;
									FireKeyEvent( updown,g_pplayer->m_rgKeys[eStartGameKey] );
									if ( updown == DISPID_GameEvents_KeyDown )
									{	VPinball::NumPlays++;
										SetRegValue( "Statistics", "NumPlays", REG_DWORD, &(VPinball::NumPlays), 4 );
							}	}	}
					else if ((uShockType == USHOCKTYPE_ULTRACADE) && (m_override_default_buttons == 0))	// left
							{	FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftFlipperKey] );}
					else if (m_joylflipkey == 9){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftFlipperKey] );}
					else if (m_joyrflipkey == 9){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightFlipperKey] );}
					else if (m_joyplungerkey == 9){FireKeyEvent( updown,g_pplayer->m_rgKeys[ePlungerKey] );}
					else if (m_joyaddcreditkey == 9){FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey] );}
					else if (m_joyaddcreditkey2 == 9){FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey2] );}
					else if (m_joylmagnasave == 9){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftMagnaSave] );}
					else if (m_joyrmagnasave == 9){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightMagnaSave] );}
					else if (m_joystartgamekey == 9)
						{	if( ((curr_time_msec - firedautostart) > ((U32)(ptable->m_tblAutoStart*1000.0f))) || (pressed_start) || started() ) 
							{	pressed_start = 1;
								FireKeyEvent( updown,g_pplayer->m_rgKeys[eStartGameKey] );
								if ( updown == DISPID_GameEvents_KeyDown )
								{	VPinball::NumPlays++;
									SetRegValue( "Statistics", "NumPlays", REG_DWORD, &(VPinball::NumPlays), 4 );
						}	}	}
					else if (m_joyexitgamekey == 9)
						{	if( DISPID_GameEvents_KeyDown == updown ) 
							{g_pplayer->m_fCloseDown = fTrue;}
						}
					else if (m_joyframecount == 9)
						{	if( DISPID_GameEvents_KeyDown == updown ) 
							{g_pplayer->ToggleFPS();}
						}
					else if (m_joyvolumeup == 9){FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeUp] );}
					else if (m_joyvolumedown == 9){FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeDown] );}
					else if (m_joylefttilt == 9){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftTiltKey] );}
					else if (m_joycentertilt == 9){FireKeyEvent( updown,g_pplayer->m_rgKeys[eCenterTiltKey] );}
					else if (m_joyrighttilt == 9){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightTiltKey] );}
					else if (m_joypmbuyin == 9) FireKeyEvent (updown, DIK_2);
					else if (m_joypmcoin3 == 9) FireKeyEvent (updown, DIK_5);
					else if (m_joypmcoin4 == 9) FireKeyEvent (updown, DIK_6);
					else if (m_joypmcoindoor == 9) FireKeyEvent (updown, DIK_END);
					else if (m_joypmcancel == 9) FireKeyEvent (updown, DIK_7);
					else if (m_joypmdown == 9) FireKeyEvent (updown, DIK_8);
					else if (m_joypmup == 9) FireKeyEvent (updown, DIK_9);
					else if (m_joypmenter == 9) FireKeyEvent (updown, DIK_0);
				}

			else if (input->dwOfs == DIJOFS_BUTTON9)
				{
						 if ((uShockType == USHOCKTYPE_PBWIZARD) && (m_override_default_buttons == 0))	// left
							{	FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftFlipperKey] );}
					else if (m_joylflipkey == 10){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftFlipperKey] );}
					else if (m_joyrflipkey == 10){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightFlipperKey] );}
					else if (m_joyplungerkey == 10){FireKeyEvent( updown,g_pplayer->m_rgKeys[ePlungerKey] );}
					else if (m_joyaddcreditkey == 10){FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey] );}
					else if (m_joyaddcreditkey2 == 10){FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey2] );}
					else if (m_joylmagnasave == 10){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftMagnaSave] );}
					else if (m_joyrmagnasave == 10){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightMagnaSave] );}
					else if (m_joystartgamekey == 10)
						{	if( ((curr_time_msec - firedautostart) > ((U32)(ptable->m_tblAutoStart*1000.0f))) || (pressed_start) || started() ) 
							{	pressed_start = 1;
								FireKeyEvent( updown,g_pplayer->m_rgKeys[eStartGameKey] );
								if ( updown == DISPID_GameEvents_KeyDown )
								{	VPinball::NumPlays++;
									SetRegValue( "Statistics", "NumPlays", REG_DWORD, &(VPinball::NumPlays), 4 );
						}	}	}
					else if (m_joyexitgamekey == 10)
						{	if( DISPID_GameEvents_KeyDown == updown ) 
							{g_pplayer->m_fCloseDown = fTrue;}
						}
					else if (m_joyframecount == 10)
						{	if( DISPID_GameEvents_KeyDown == updown ) 
							{g_pplayer->ToggleFPS();}
						}
					else if (m_joyvolumeup == 10){FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeUp] );}
					else if (m_joyvolumedown == 10){FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeDown] );}
					else if (m_joylefttilt == 10){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftTiltKey] );}
					else if (m_joycentertilt == 10){FireKeyEvent( updown,g_pplayer->m_rgKeys[eCenterTiltKey] );}
					else if (m_joyrighttilt == 10){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightTiltKey] );}
					else if (m_joypmbuyin == 10) FireKeyEvent (updown, DIK_2);
					else if (m_joypmcoin3 == 10) FireKeyEvent (updown, DIK_5);
					else if (m_joypmcoin4 == 10) FireKeyEvent (updown, DIK_6);
					else if (m_joypmcoindoor == 10) FireKeyEvent (updown, DIK_END);
					else if (m_joypmcancel == 10) FireKeyEvent (updown, DIK_7);
					else if (m_joypmdown == 10) FireKeyEvent (updown, DIK_8);
					else if (m_joypmup == 10) FireKeyEvent (updown, DIK_9);
					else if (m_joypmenter == 10) FireKeyEvent (updown, DIK_0);
				}

			else if (input->dwOfs == DIJOFS_BUTTON10)
				{
						 if ((uShockType == USHOCKTYPE_PBWIZARD) && (m_override_default_buttons == 0))	// left 2
							{	FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftMagnaSave ] );}
					else if ((uShockType == USHOCKTYPE_ULTRACADE) && (m_override_default_buttons == 0)) // right
							{	FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightFlipperKey] );}
					else if (m_joylflipkey == 11){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftFlipperKey] );}
					else if (m_joyrflipkey == 11){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightFlipperKey] );}
					else if (m_joyplungerkey == 11){FireKeyEvent( updown,g_pplayer->m_rgKeys[ePlungerKey] );}
					else if (m_joyaddcreditkey == 11){FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey] );}
					else if (m_joyaddcreditkey2 == 11){FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey2] );}
					else if (m_joylmagnasave == 11){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftMagnaSave] );}
					else if (m_joyrmagnasave == 11){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightMagnaSave] );}
					else if (m_joystartgamekey == 11)
						{	if( ((curr_time_msec - firedautostart) > ((U32)(ptable->m_tblAutoStart*1000.0f))) || (pressed_start) || started() ) 
							{	pressed_start = 1;
								FireKeyEvent( updown,g_pplayer->m_rgKeys[eStartGameKey] );
								if ( updown == DISPID_GameEvents_KeyDown )
								{	VPinball::NumPlays++;
									SetRegValue( "Statistics", "NumPlays", REG_DWORD, &(VPinball::NumPlays), 4 );
						}	}	}
					else if (m_joyexitgamekey == 11)
						{	if( DISPID_GameEvents_KeyDown == updown ) 
							{g_pplayer->m_fCloseDown = fTrue;}
						}
					else if (m_joyframecount == 11)
						{	if( DISPID_GameEvents_KeyDown == updown ) 
							{g_pplayer->ToggleFPS();}
						}
					else if (m_joyvolumeup == 11){FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeUp] );}
					else if (m_joyvolumedown == 11){FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeDown] );}
					else if (m_joylefttilt == 11){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftTiltKey] );}
					else if (m_joycentertilt == 11){FireKeyEvent( updown,g_pplayer->m_rgKeys[eCenterTiltKey] );}
					else if (m_joyrighttilt == 11){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightTiltKey] );}
					else if (m_joypmbuyin == 11) FireKeyEvent (updown, DIK_2);
					else if (m_joypmcoin3 == 11) FireKeyEvent (updown, DIK_5);
					else if (m_joypmcoin4 == 11) FireKeyEvent (updown, DIK_6);
					else if (m_joypmcoindoor == 11) FireKeyEvent (updown, DIK_END);
					else if (m_joypmcancel == 11) FireKeyEvent (updown, DIK_7);
					else if (m_joypmdown == 11) FireKeyEvent (updown, DIK_8);
					else if (m_joypmup == 11) FireKeyEvent (updown, DIK_9);
					else if (m_joypmenter == 11) FireKeyEvent (updown, DIK_0);
				}

			else if (input->dwOfs == DIJOFS_BUTTON11)
				{
						 if ((uShockType == USHOCKTYPE_PBWIZARD) && (m_override_default_buttons == 0)) // coin 1
							{	FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey] );}
					else if (m_joylflipkey == 12){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftFlipperKey] );}
					else if (m_joyrflipkey == 12){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightFlipperKey] );}
					else if (m_joyplungerkey == 12){FireKeyEvent( updown,g_pplayer->m_rgKeys[ePlungerKey] );}
					else if (m_joyaddcreditkey == 12){FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey] );}
					else if (m_joyaddcreditkey2 == 12){FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey2] );}
					else if (m_joylmagnasave == 12){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftMagnaSave] );}
					else if (m_joyrmagnasave == 12){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightMagnaSave] );}
					else if (m_joystartgamekey == 12)
						{	if( ((curr_time_msec - firedautostart) > ((U32)(ptable->m_tblAutoStart*1000.0f))) || (pressed_start) || started() ) 
							{	pressed_start = 1;
								FireKeyEvent( updown,g_pplayer->m_rgKeys[eStartGameKey] );
								if ( updown == DISPID_GameEvents_KeyDown )
								{	VPinball::NumPlays++;
									SetRegValue( "Statistics", "NumPlays", REG_DWORD, &(VPinball::NumPlays), 4 );
						}	}	}
					else if (m_joyexitgamekey == 12)
						{	if( DISPID_GameEvents_KeyDown == updown ) 
							{g_pplayer->m_fCloseDown = fTrue;}
						}
					else if (m_joyframecount == 12)
						{	if( DISPID_GameEvents_KeyDown == updown ) 
							{g_pplayer->ToggleFPS();}
						}
					else if (m_joyvolumeup == 12){FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeUp] );}
					else if (m_joyvolumedown == 12){FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeDown] );}
					else if (m_joylefttilt == 12){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftTiltKey] );}
					else if (m_joycentertilt == 12){FireKeyEvent( updown,g_pplayer->m_rgKeys[eCenterTiltKey] );}
					else if (m_joyrighttilt == 12){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightTiltKey] );}
					else if (m_joypmbuyin == 12) FireKeyEvent (updown, DIK_2);
					else if (m_joypmcoin3 == 12) FireKeyEvent (updown, DIK_5);
					else if (m_joypmcoin4 == 12) FireKeyEvent (updown, DIK_6);
					else if (m_joypmcoindoor == 12) FireKeyEvent (updown, DIK_END);
					else if (m_joypmcancel == 12) FireKeyEvent (updown, DIK_7);
					else if (m_joypmdown == 12) FireKeyEvent (updown, DIK_8);
					else if (m_joypmup == 12) FireKeyEvent (updown, DIK_9);
					else if (m_joypmenter == 12) FireKeyEvent (updown, DIK_0);
				}

			else if (input->dwOfs == DIJOFS_BUTTON12)
				{
						 if ((uShockType == USHOCKTYPE_PBWIZARD) && (m_override_default_buttons == 0)) // coin 2
							{	FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey2] );}
					else if ((uShockType == USHOCKTYPE_ULTRACADE) && (m_override_default_buttons == 0)) // start
							{ // Check if we can allow the start (table is done initializing).
								if( ((curr_time_msec - firedautostart) > ((U32)(ptable->m_tblAutoStart*1000.0f))) ||
								(pressed_start) || started() ) 
								{	pressed_start = 1;
									FireKeyEvent( updown,g_pplayer->m_rgKeys[eStartGameKey] );
									if ( updown == DISPID_GameEvents_KeyDown ) // Check if this was a press.
									{	VPinball::NumPlays++; // Update the number of plays.
										SetRegValue( "Statistics", "NumPlays", REG_DWORD, &(VPinball::NumPlays), 4 );
							}	}	}
					else if (m_joylflipkey == 13){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftFlipperKey] );}
					else if (m_joyrflipkey == 13){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightFlipperKey] );}
					else if (m_joyplungerkey == 13){FireKeyEvent( updown,g_pplayer->m_rgKeys[ePlungerKey] );}
					else if (m_joyaddcreditkey == 13){FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey] );}
					else if (m_joyaddcreditkey2 == 13){FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey2] );}
					else if (m_joylmagnasave == 13){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftMagnaSave] );}
					else if (m_joyrmagnasave == 13){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightMagnaSave] );}
					else if (m_joystartgamekey == 13)
						{	if( ((curr_time_msec - firedautostart) > ((U32)(ptable->m_tblAutoStart*1000.0f))) || (pressed_start) || started() ) 
							{	pressed_start = 1;
								FireKeyEvent( updown,g_pplayer->m_rgKeys[eStartGameKey] );
								if ( updown == DISPID_GameEvents_KeyDown )
								{	VPinball::NumPlays++;
									SetRegValue( "Statistics", "NumPlays", REG_DWORD, &(VPinball::NumPlays), 4 );
						}	}	}
					else if (m_joyexitgamekey == 13)
						{	if( DISPID_GameEvents_KeyDown == updown ) 
							{g_pplayer->m_fCloseDown = fTrue;}
						}
					else if (m_joyframecount == 13)
						{	if( DISPID_GameEvents_KeyDown == updown ) 
							{g_pplayer->ToggleFPS();}
						}
					else if (m_joyvolumeup == 13){FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeUp] );}
					else if (m_joyvolumedown == 13){FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeDown] );}
					else if (m_joylefttilt == 13){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftTiltKey] );}
					else if (m_joycentertilt == 13){FireKeyEvent( updown,g_pplayer->m_rgKeys[eCenterTiltKey] );}
					else if (m_joyrighttilt == 13){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightTiltKey] );}
					else if (m_joypmbuyin == 13) FireKeyEvent (updown, DIK_2);
					else if (m_joypmcoin3 == 13) FireKeyEvent (updown, DIK_5);
					else if (m_joypmcoin4 == 13) FireKeyEvent (updown, DIK_6);
					else if (m_joypmcoindoor == 13) FireKeyEvent (updown, DIK_END);
					else if (m_joypmcancel == 13) FireKeyEvent (updown, DIK_7);
					else if (m_joypmdown == 13) FireKeyEvent (updown, DIK_8);
					else if (m_joypmup == 13) FireKeyEvent (updown, DIK_9);
					else if (m_joypmenter == 13) FireKeyEvent (updown, DIK_0);
				}

			else if (input->dwOfs == DIJOFS_BUTTON13)
				{
						 if ((uShockType == USHOCKTYPE_ULTRACADE) && (m_override_default_buttons == 0)) // plunge
							{	FireKeyEvent( updown,g_pplayer->m_rgKeys[ePlungerKey] );}
					else if (m_joylflipkey == 14){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftFlipperKey] );}
					else if (m_joyrflipkey == 14){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightFlipperKey] );}
					else if (m_joyplungerkey == 14){FireKeyEvent( updown,g_pplayer->m_rgKeys[ePlungerKey] );}
					else if (m_joyaddcreditkey == 14){FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey] );}
					else if (m_joyaddcreditkey2 == 14){FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey2] );}
					else if (m_joylmagnasave == 14){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftMagnaSave] );}
					else if (m_joyrmagnasave == 14){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightMagnaSave] );}
					else if (m_joystartgamekey == 14)
						{	if( ((curr_time_msec - firedautostart) > ((U32)(ptable->m_tblAutoStart*1000.0f))) || (pressed_start) || started() ) 
							{	pressed_start = 1;
								FireKeyEvent( updown,g_pplayer->m_rgKeys[eStartGameKey] );
								if ( updown == DISPID_GameEvents_KeyDown )
								{	VPinball::NumPlays++;
									SetRegValue( "Statistics", "NumPlays", REG_DWORD, &(VPinball::NumPlays), 4 );
						}	}	}
					else if (m_joyexitgamekey == 14)
						{	if( DISPID_GameEvents_KeyDown == updown ) 
							{g_pplayer->m_fCloseDown = fTrue;}
						}
					else if (m_joyframecount == 14)
						{	if( DISPID_GameEvents_KeyDown == updown ) 
							{g_pplayer->ToggleFPS();}
						}
					else if (m_joyvolumeup == 14){FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeUp] );}
					else if (m_joyvolumedown == 14){FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeDown] );}
					else if (m_joylefttilt == 14){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftTiltKey] );}
					else if (m_joycentertilt == 14){FireKeyEvent( updown,g_pplayer->m_rgKeys[eCenterTiltKey] );}
					else if (m_joyrighttilt == 14){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightTiltKey] );}
					else if (m_joypmbuyin == 14) FireKeyEvent (updown, DIK_2);
					else if (m_joypmcoin3 == 14) FireKeyEvent (updown, DIK_5);
					else if (m_joypmcoin4 == 14) FireKeyEvent (updown, DIK_6);
					else if (m_joypmcoindoor == 14) FireKeyEvent (updown, DIK_END);
					else if (m_joypmcancel == 14) FireKeyEvent (updown, DIK_7);
					else if (m_joypmdown == 14) FireKeyEvent (updown, DIK_8);
					else if (m_joypmup == 14) FireKeyEvent (updown, DIK_9);
					else if (m_joypmenter == 14) FireKeyEvent (updown, DIK_0);
				}

			else if (input->dwOfs == DIJOFS_BUTTON14)
				{
						 if ((uShockType == USHOCKTYPE_ULTRACADE) && (m_override_default_buttons == 0)) // exit
							{	if ((started()) || (ptable->m_tblAutoStartEnabled == false)) // Check if we have started a game yet.
								{	if( DISPID_GameEvents_KeyDown == updown ) 
									{	first_stamp = curr_time_msec;
										exit_stamp = curr_time_msec;
										FireKeyEvent( DISPID_GameEvents_KeyDown,g_pplayer->m_rgKeys[eExitGame ] );  
									}
									else 
									{	FireKeyEvent( DISPID_GameEvents_KeyUp,g_pplayer->m_rgKeys[eExitGame ] );  
										exit_stamp = 0;
							}	}	}
					else if (m_joylflipkey == 15){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftFlipperKey] );}
					else if (m_joyrflipkey == 15){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightFlipperKey] );}
					else if (m_joyplungerkey == 15){FireKeyEvent( updown,g_pplayer->m_rgKeys[ePlungerKey] );}
					else if (m_joyaddcreditkey == 15){FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey] );}
					else if (m_joyaddcreditkey2 == 15){FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey2] );}
					else if (m_joylmagnasave == 15){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftMagnaSave] );}
					else if (m_joyrmagnasave == 15){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightMagnaSave] );}
					else if (m_joystartgamekey == 15)
						{	if( ((curr_time_msec - firedautostart) > ((U32)(ptable->m_tblAutoStart*1000.0f))) || (pressed_start) || started() ) 
							{	pressed_start = 1;
								FireKeyEvent( updown,g_pplayer->m_rgKeys[eStartGameKey] );
								if ( updown == DISPID_GameEvents_KeyDown )
								{	VPinball::NumPlays++;
									SetRegValue( "Statistics", "NumPlays", REG_DWORD, &(VPinball::NumPlays), 4 );
						}	}	}
					else if (m_joyexitgamekey == 15)
						{	if( DISPID_GameEvents_KeyDown == updown ) 
							{g_pplayer->m_fCloseDown = fTrue;}
						}
					else if (m_joyframecount == 15)
						{	if( DISPID_GameEvents_KeyDown == updown ) 
							{g_pplayer->ToggleFPS();}
						}
					else if (m_joyvolumeup == 15){FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeUp] );}
					else if (m_joyvolumedown == 15){FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeDown] );}
					else if (m_joylefttilt == 15){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftTiltKey] );}
					else if (m_joycentertilt == 15){FireKeyEvent( updown,g_pplayer->m_rgKeys[eCenterTiltKey] );}
					else if (m_joyrighttilt == 15){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightTiltKey] );}
					else if (m_joypmbuyin == 15) FireKeyEvent (updown, DIK_2);
					else if (m_joypmcoin3 == 15) FireKeyEvent (updown, DIK_5);
					else if (m_joypmcoin4 == 15) FireKeyEvent (updown, DIK_6);
					else if (m_joypmcoindoor == 15) FireKeyEvent (updown, DIK_END);
					else if (m_joypmcancel == 15) FireKeyEvent (updown, DIK_7);
					else if (m_joypmdown == 15) FireKeyEvent (updown, DIK_8);
					else if (m_joypmup == 15) FireKeyEvent (updown, DIK_9);
					else if (m_joypmenter == 15) FireKeyEvent (updown, DIK_0);
				}

			else if (input->dwOfs == DIJOFS_BUTTON15)
				{
						 if (m_joylflipkey == 16){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftFlipperKey] );}
					else if (m_joyrflipkey == 16){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightFlipperKey] );}
					else if (m_joyplungerkey == 16){FireKeyEvent( updown,g_pplayer->m_rgKeys[ePlungerKey] );}
					else if (m_joyaddcreditkey == 16){FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey] );}
					else if (m_joyaddcreditkey2 == 16){FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey2] );}
					else if (m_joylmagnasave == 16){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftMagnaSave] );}
					else if (m_joyrmagnasave == 16){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightMagnaSave] );}
					else if (m_joystartgamekey == 16)
						{	if( ((curr_time_msec - firedautostart) > ((U32)(ptable->m_tblAutoStart*1000.0f))) || (pressed_start) || started() ) 
							{	pressed_start = 1;
								FireKeyEvent( updown,g_pplayer->m_rgKeys[eStartGameKey] );
								if ( updown == DISPID_GameEvents_KeyDown )
								{	VPinball::NumPlays++;
									SetRegValue( "Statistics", "NumPlays", REG_DWORD, &(VPinball::NumPlays), 4 );
						}	}	}
					else if (m_joyexitgamekey == 16)
						{	if( DISPID_GameEvents_KeyDown == updown ) 
							{g_pplayer->m_fCloseDown = fTrue;}
						}
					else if (m_joyframecount == 16)
						{	if( DISPID_GameEvents_KeyDown == updown ) 
							{g_pplayer->ToggleFPS();}
						}
					else if (m_joyvolumeup == 16){FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeUp] );}
					else if (m_joyvolumedown == 16){FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeDown] );}
					else if (m_joylefttilt == 16){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftTiltKey] );}
					else if (m_joycentertilt == 16){FireKeyEvent( updown,g_pplayer->m_rgKeys[eCenterTiltKey] );}
					else if (m_joyrighttilt == 16){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightTiltKey] );}
					else if (m_joypmbuyin == 16) FireKeyEvent (updown, DIK_2);
					else if (m_joypmcoin3 == 16) FireKeyEvent (updown, DIK_5);
					else if (m_joypmcoin4 == 16) FireKeyEvent (updown, DIK_6);
					else if (m_joypmcoindoor == 16) FireKeyEvent (updown, DIK_END);
					else if (m_joypmcancel == 16) FireKeyEvent (updown, DIK_7);
					else if (m_joypmdown == 16) FireKeyEvent (updown, DIK_8);
					else if (m_joypmup == 16) FireKeyEvent (updown, DIK_9);
					else if (m_joypmenter == 16) FireKeyEvent (updown, DIK_0);
				}

			else if (input->dwOfs == DIJOFS_BUTTON16)
				{
						 if (m_joylflipkey == 17){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftFlipperKey] );}
					else if (m_joyrflipkey == 17){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightFlipperKey] );}
					else if (m_joyplungerkey == 17){FireKeyEvent( updown,g_pplayer->m_rgKeys[ePlungerKey] );}
					else if (m_joyaddcreditkey == 17){FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey] );}
					else if (m_joyaddcreditkey2 == 17){FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey2] );}
					else if (m_joylmagnasave == 17){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftMagnaSave] );}
					else if (m_joyrmagnasave == 17){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightMagnaSave] );}
					else if (m_joystartgamekey == 17)
						{	if( ((curr_time_msec - firedautostart) > ((U32)(ptable->m_tblAutoStart*1000.0f))) || (pressed_start) || started() ) 
							{	pressed_start = 1;
								FireKeyEvent( updown,g_pplayer->m_rgKeys[eStartGameKey] );
								if ( updown == DISPID_GameEvents_KeyDown )
								{	VPinball::NumPlays++;
									SetRegValue( "Statistics", "NumPlays", REG_DWORD, &(VPinball::NumPlays), 4 );
						}	}	}
					else if (m_joyexitgamekey == 17)
						{	if( DISPID_GameEvents_KeyDown == updown ) 
							{g_pplayer->m_fCloseDown = fTrue;}
						}
					else if (m_joyframecount == 17)
						{	if( DISPID_GameEvents_KeyDown == updown ) 
							{g_pplayer->ToggleFPS();}
						}
					else if (m_joyvolumeup == 17){FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeUp] );}
					else if (m_joyvolumedown == 17){FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeDown] );}
					else if (m_joylefttilt == 17){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftTiltKey] );}
					else if (m_joycentertilt == 17){FireKeyEvent( updown,g_pplayer->m_rgKeys[eCenterTiltKey] );}
					else if (m_joyrighttilt == 17){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightTiltKey] );}
					else if (m_joypmbuyin == 17) FireKeyEvent (updown, DIK_2);
					else if (m_joypmcoin3 == 17) FireKeyEvent (updown, DIK_5);
					else if (m_joypmcoin4 == 17) FireKeyEvent (updown, DIK_6);
					else if (m_joypmcoindoor == 17) FireKeyEvent (updown, DIK_END);
					else if (m_joypmcancel == 17) FireKeyEvent (updown, DIK_7);
					else if (m_joypmdown == 17) FireKeyEvent (updown, DIK_8);
					else if (m_joypmup == 17) FireKeyEvent (updown, DIK_9);
					else if (m_joypmenter == 17) FireKeyEvent (updown, DIK_0);
				}

			else if (input->dwOfs == DIJOFS_BUTTON17)
				{
						 if (m_joylflipkey == 18){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftFlipperKey] );}
					else if (m_joyrflipkey == 18){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightFlipperKey] );}
					else if (m_joyplungerkey == 18){FireKeyEvent( updown,g_pplayer->m_rgKeys[ePlungerKey] );}
					else if (m_joyaddcreditkey == 18){FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey] );}
					else if (m_joyaddcreditkey2 == 18){FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey2] );}
					else if (m_joylmagnasave == 18){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftMagnaSave] );}
					else if (m_joyrmagnasave == 18){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightMagnaSave] );}
					else if (m_joystartgamekey == 18)
						{	if( ((curr_time_msec - firedautostart) > ((U32)(ptable->m_tblAutoStart*1000.0f))) || (pressed_start) || started() ) 
							{	pressed_start = 1;
								FireKeyEvent( updown,g_pplayer->m_rgKeys[eStartGameKey] );
								if ( updown == DISPID_GameEvents_KeyDown )
								{	VPinball::NumPlays++;
									SetRegValue( "Statistics", "NumPlays", REG_DWORD, &(VPinball::NumPlays), 4 );
						}	}	}
					else if (m_joyexitgamekey == 18)
						{	if( DISPID_GameEvents_KeyDown == updown ) 
							{g_pplayer->m_fCloseDown = fTrue;}
						}
					else if (m_joyframecount == 18)
						{	if( DISPID_GameEvents_KeyDown == updown ) 
								{g_pplayer->ToggleFPS();}
						}
					else if (m_joyvolumeup == 18){FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeUp] );}
					else if (m_joyvolumedown == 18){FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeDown] );}
					else if (m_joylefttilt == 18){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftTiltKey] );}
					else if (m_joycentertilt == 18){FireKeyEvent( updown,g_pplayer->m_rgKeys[eCenterTiltKey] );}
					else if (m_joyrighttilt == 18){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightTiltKey] );}
					else if (m_joypmbuyin == 18) FireKeyEvent (updown, DIK_2);
					else if (m_joypmcoin3 == 18) FireKeyEvent (updown, DIK_5);
					else if (m_joypmcoin4 == 18) FireKeyEvent (updown, DIK_6);
					else if (m_joypmcoindoor == 18) FireKeyEvent (updown, DIK_END);
					else if (m_joypmcancel == 18) FireKeyEvent (updown, DIK_7);
					else if (m_joypmdown == 18) FireKeyEvent (updown, DIK_8);
					else if (m_joypmup == 18) FireKeyEvent (updown, DIK_9);
					else if (m_joypmenter == 18) FireKeyEvent (updown, DIK_0);
				}

			else if (input->dwOfs == DIJOFS_BUTTON18)
				{
						 if (m_joylflipkey == 19){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftFlipperKey] );}
					else if (m_joyrflipkey == 19){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightFlipperKey] );}
					else if (m_joyplungerkey == 19){FireKeyEvent( updown,g_pplayer->m_rgKeys[ePlungerKey] );}
					else if (m_joyaddcreditkey == 19){FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey] );}
					else if (m_joyaddcreditkey2 == 19){FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey2] );}
					else if (m_joylmagnasave == 19){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftMagnaSave] );}
					else if (m_joyrmagnasave == 19){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightMagnaSave] );}
					else if (m_joystartgamekey == 19)
						{	if( ((curr_time_msec - firedautostart) > ((U32)(ptable->m_tblAutoStart*1000.0f))) || (pressed_start) || started() ) 
							{	pressed_start = 1;
								FireKeyEvent( updown,g_pplayer->m_rgKeys[eStartGameKey] );
								if ( updown == DISPID_GameEvents_KeyDown )
								{	VPinball::NumPlays++;
									SetRegValue( "Statistics", "NumPlays", REG_DWORD, &(VPinball::NumPlays), 4 );
						}	}	}
					else if (m_joyexitgamekey == 19)
						{	if( DISPID_GameEvents_KeyDown == updown ) 
							{g_pplayer->m_fCloseDown = fTrue;}
						}
					else if (m_joyframecount == 19)
						{	if( DISPID_GameEvents_KeyDown == updown ) 
							{g_pplayer->ToggleFPS();}
						}
					else if (m_joyvolumeup == 19){FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeUp] );}
					else if (m_joyvolumedown == 19){FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeDown] );}
					else if (m_joylefttilt == 19){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftTiltKey] );}
					else if (m_joycentertilt == 19){FireKeyEvent( updown,g_pplayer->m_rgKeys[eCenterTiltKey] );}
					else if (m_joyrighttilt == 19){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightTiltKey] );}
					else if (m_joypmbuyin == 19) FireKeyEvent (updown, DIK_2);
					else if (m_joypmcoin3 == 19) FireKeyEvent (updown, DIK_5);
					else if (m_joypmcoin4 == 19) FireKeyEvent (updown, DIK_6);
					else if (m_joypmcoindoor == 19) FireKeyEvent (updown, DIK_END);
					else if (m_joypmcancel == 19) FireKeyEvent (updown, DIK_7);
					else if (m_joypmdown == 19) FireKeyEvent (updown, DIK_8);
					else if (m_joypmup == 19) FireKeyEvent (updown, DIK_9);
					else if (m_joypmenter == 19) FireKeyEvent (updown, DIK_0);
				}

			else if (input->dwOfs == DIJOFS_BUTTON19)
				{
						 if (m_joylflipkey == 20){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftFlipperKey] );}
					else if (m_joyrflipkey == 20){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightFlipperKey] );}
					else if (m_joyplungerkey == 20){FireKeyEvent( updown,g_pplayer->m_rgKeys[ePlungerKey] );}
					else if (m_joyaddcreditkey == 20){FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey] );}
					else if (m_joyaddcreditkey2 == 20){FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey2] );}
					else if (m_joylmagnasave == 20){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftMagnaSave] );}
					else if (m_joyrmagnasave == 20){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightMagnaSave] );}
					else if (m_joystartgamekey == 20)
						{	if( ((curr_time_msec - firedautostart) > ((U32)(ptable->m_tblAutoStart*1000.0f))) || (pressed_start) || started() ) 
							{	pressed_start = 1;
								FireKeyEvent( updown,g_pplayer->m_rgKeys[eStartGameKey] );
								if ( updown == DISPID_GameEvents_KeyDown )
								{	VPinball::NumPlays++;
									SetRegValue( "Statistics", "NumPlays", REG_DWORD, &(VPinball::NumPlays), 4 );
						}	}	}
					else if (m_joyexitgamekey == 20)
						{	if( DISPID_GameEvents_KeyDown == updown ) 
							{g_pplayer->m_fCloseDown = fTrue;}
						}
					else if (m_joyframecount == 20)
						{	if( DISPID_GameEvents_KeyDown == updown ) 
							{g_pplayer->ToggleFPS();}
						}
					else if (m_joyvolumeup == 20){FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeUp] );}
					else if (m_joyvolumedown == 20){FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeDown] );}
					else if (m_joylefttilt == 20){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftTiltKey] );}
					else if (m_joycentertilt == 20){FireKeyEvent( updown,g_pplayer->m_rgKeys[eCenterTiltKey] );}
					else if (m_joyrighttilt == 20){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightTiltKey] );}
					else if (m_joypmbuyin == 20) FireKeyEvent (updown, DIK_2);
					else if (m_joypmcoin3 == 20) FireKeyEvent (updown, DIK_5);
					else if (m_joypmcoin4 == 20) FireKeyEvent (updown, DIK_6);
					else if (m_joypmcoindoor == 20) FireKeyEvent (updown, DIK_END);
					else if (m_joypmcancel == 20) FireKeyEvent (updown, DIK_7);
					else if (m_joypmdown == 20) FireKeyEvent (updown, DIK_8);
					else if (m_joypmup == 20) FireKeyEvent (updown, DIK_9);
					else if (m_joypmenter == 20) FireKeyEvent (updown, DIK_0);
				}

			else if (input->dwOfs == DIJOFS_BUTTON20)
				{
						 if (m_joylflipkey == 21){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftFlipperKey] );}
					else if (m_joyrflipkey == 21){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightFlipperKey] );}
					else if (m_joyplungerkey == 21){FireKeyEvent( updown,g_pplayer->m_rgKeys[ePlungerKey] );}
					else if (m_joyaddcreditkey == 21){FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey] );}
					else if (m_joyaddcreditkey2 == 21){FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey2] );}
					else if (m_joylmagnasave == 21){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftMagnaSave] );}
					else if (m_joyrmagnasave == 21){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightMagnaSave] );}
					else if (m_joystartgamekey == 21)
						{	if( ((curr_time_msec - firedautostart) > ((U32)(ptable->m_tblAutoStart*1000.0f))) || (pressed_start) || started() ) 
							{	pressed_start = 1;
								FireKeyEvent( updown,g_pplayer->m_rgKeys[eStartGameKey] );
								if ( updown == DISPID_GameEvents_KeyDown )
								{	VPinball::NumPlays++;
									SetRegValue( "Statistics", "NumPlays", REG_DWORD, &(VPinball::NumPlays), 4 );
						}	}	}
					else if (m_joyexitgamekey == 21)
						{	if( DISPID_GameEvents_KeyDown == updown ) 
							{g_pplayer->m_fCloseDown = fTrue;}
						}
					else if (m_joyframecount == 21)
						{	if( DISPID_GameEvents_KeyDown == updown ) 
							{g_pplayer->ToggleFPS();}
						}
					else if (m_joyvolumeup == 21){FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeUp] );}
					else if (m_joyvolumedown == 21){FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeDown] );}
					else if (m_joylefttilt == 21){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftTiltKey] );}
					else if (m_joycentertilt == 21){FireKeyEvent( updown,g_pplayer->m_rgKeys[eCenterTiltKey] );}
					else if (m_joyrighttilt == 21){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightTiltKey] );}
					else if (m_joypmbuyin == 21) FireKeyEvent (updown, DIK_2);
					else if (m_joypmcoin3 == 21) FireKeyEvent (updown, DIK_5);
					else if (m_joypmcoin4 == 21) FireKeyEvent (updown, DIK_6);
					else if (m_joypmcoindoor == 21) FireKeyEvent (updown, DIK_END);
					else if (m_joypmcancel == 21) FireKeyEvent (updown, DIK_7);
					else if (m_joypmdown == 21) FireKeyEvent (updown, DIK_8);
					else if (m_joypmup == 21) FireKeyEvent (updown, DIK_9);
					else if (m_joypmenter == 21) FireKeyEvent (updown, DIK_0);
				}

			else if (input->dwOfs == DIJOFS_BUTTON21)
				{
						 if (m_joylflipkey == 22){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftFlipperKey] );}
					else if (m_joyrflipkey == 22){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightFlipperKey] );}
					else if (m_joyplungerkey == 22){FireKeyEvent( updown,g_pplayer->m_rgKeys[ePlungerKey] );}
					else if (m_joyaddcreditkey == 22){FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey] );}
					else if (m_joyaddcreditkey2 == 22){FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey2] );}
					else if (m_joylmagnasave == 22){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftMagnaSave] );}
					else if (m_joyrmagnasave == 22){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightMagnaSave] );}
					else if (m_joystartgamekey == 22)
						{	if( ((curr_time_msec - firedautostart) > ((U32)(ptable->m_tblAutoStart*1000.0f))) || (pressed_start) || started() ) 
							{	pressed_start = 1;
								FireKeyEvent( updown,g_pplayer->m_rgKeys[eStartGameKey] );
								if ( updown == DISPID_GameEvents_KeyDown )
								{	VPinball::NumPlays++;
									SetRegValue( "Statistics", "NumPlays", REG_DWORD, &(VPinball::NumPlays), 4 );
						}	}	}
					else if (m_joyexitgamekey == 22)
						{	if( DISPID_GameEvents_KeyDown == updown ) 
							{g_pplayer->m_fCloseDown = fTrue;}
						}
					else if (m_joyframecount == 22)
						{	if( DISPID_GameEvents_KeyDown == updown ) 
							{g_pplayer->ToggleFPS();}
						}
					else if (m_joyvolumeup == 22){FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeUp] );}
					else if (m_joyvolumedown == 22){FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeDown] );}
					else if (m_joylefttilt == 22){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftTiltKey] );}
					else if (m_joycentertilt == 22){FireKeyEvent( updown,g_pplayer->m_rgKeys[eCenterTiltKey] );}
					else if (m_joyrighttilt == 22){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightTiltKey] );}
					else if (m_joypmbuyin == 22) FireKeyEvent (updown, DIK_2);
					else if (m_joypmcoin3 == 22) FireKeyEvent (updown, DIK_5);
					else if (m_joypmcoin4 == 22) FireKeyEvent (updown, DIK_6);
					else if (m_joypmcoindoor == 22) FireKeyEvent (updown, DIK_END);
					else if (m_joypmcancel == 22) FireKeyEvent (updown, DIK_7);
					else if (m_joypmdown == 22) FireKeyEvent (updown, DIK_8);
					else if (m_joypmup == 22) FireKeyEvent (updown, DIK_9);
					else if (m_joypmenter == 22) FireKeyEvent (updown, DIK_0);
				}

			else if (input->dwOfs == DIJOFS_BUTTON22)
				{
						 if (m_joylflipkey == 23){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftFlipperKey] );}
					else if (m_joyrflipkey == 23){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightFlipperKey] );}
					else if (m_joyplungerkey == 23){FireKeyEvent( updown,g_pplayer->m_rgKeys[ePlungerKey] );}
					else if (m_joyaddcreditkey == 23){FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey] );}
					else if (m_joyaddcreditkey2 == 23){FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey2] );}
					else if (m_joylmagnasave == 23){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftMagnaSave] );}
					else if (m_joyrmagnasave == 23){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightMagnaSave] );}
					else if (m_joystartgamekey == 23)
						{	if( ((curr_time_msec - firedautostart) > ((U32)(ptable->m_tblAutoStart*1000.0f))) || (pressed_start) || started() ) 
							{	pressed_start = 1;
								FireKeyEvent( updown,g_pplayer->m_rgKeys[eStartGameKey] );
								if ( updown == DISPID_GameEvents_KeyDown )
								{	VPinball::NumPlays++;
									SetRegValue( "Statistics", "NumPlays", REG_DWORD, &(VPinball::NumPlays), 4 );
						}	}	}
					else if (m_joyexitgamekey == 23)
						{	if( DISPID_GameEvents_KeyDown == updown ) 
							{g_pplayer->m_fCloseDown = fTrue;}
						}
					else if (m_joyframecount == 23)
						{	if( DISPID_GameEvents_KeyDown == updown ) 
							{g_pplayer->ToggleFPS();}
						}
					else if (m_joyvolumeup == 23){FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeUp] );}
					else if (m_joyvolumedown == 23){FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeDown] );}
					else if (m_joylefttilt == 23){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftTiltKey] );}
					else if (m_joycentertilt == 23){FireKeyEvent( updown,g_pplayer->m_rgKeys[eCenterTiltKey] );}
					else if (m_joyrighttilt == 23){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightTiltKey] );}
					else if (m_joypmbuyin == 23) FireKeyEvent (updown, DIK_2);
					else if (m_joypmcoin3 == 23) FireKeyEvent (updown, DIK_5);
					else if (m_joypmcoin4 == 23) FireKeyEvent (updown, DIK_6);
					else if (m_joypmcoindoor == 23) FireKeyEvent (updown, DIK_END);
					else if (m_joypmcancel == 23) FireKeyEvent (updown, DIK_7);
					else if (m_joypmdown == 23) FireKeyEvent (updown, DIK_8);
					else if (m_joypmup == 23) FireKeyEvent (updown, DIK_9);
					else if (m_joypmenter == 23) FireKeyEvent (updown, DIK_0);
				}

			else if (input->dwOfs == DIJOFS_BUTTON23)
				{
						 if (m_joylflipkey == 24){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftFlipperKey] );}
					else if (m_joyrflipkey == 24){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightFlipperKey] );}
					else if (m_joyplungerkey == 24){FireKeyEvent( updown,g_pplayer->m_rgKeys[ePlungerKey] );}
					else if (m_joyaddcreditkey == 24){FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey] );}
					else if (m_joyaddcreditkey2 == 24){FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey2] );}
					else if (m_joylmagnasave == 24){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftMagnaSave] );}
					else if (m_joyrmagnasave == 24){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightMagnaSave] );}
					else if (m_joystartgamekey == 24)
						{	if( ((curr_time_msec - firedautostart) > ((U32)(ptable->m_tblAutoStart*1000.0f))) || (pressed_start) || started() ) 
							{	pressed_start = 1;
								FireKeyEvent( updown,g_pplayer->m_rgKeys[eStartGameKey] );
								if ( updown == DISPID_GameEvents_KeyDown )
								{	VPinball::NumPlays++;
									SetRegValue( "Statistics", "NumPlays", REG_DWORD, &(VPinball::NumPlays), 4 );
						}	}	}
					else if (m_joyexitgamekey == 24)
						{	if( DISPID_GameEvents_KeyDown == updown ) 
							{g_pplayer->m_fCloseDown = fTrue;}
						}
					else if (m_joyframecount == 24)
						{	if( DISPID_GameEvents_KeyDown == updown ) 
							{g_pplayer->ToggleFPS();}
						}
					else if (m_joyvolumeup == 24){FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeUp] );}
					else if (m_joyvolumedown == 24){FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeDown] );}
					else if (m_joylefttilt == 24){FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftTiltKey] );}
					else if (m_joycentertilt == 24){FireKeyEvent( updown,g_pplayer->m_rgKeys[eCenterTiltKey] );}
					else if (m_joyrighttilt == 24){FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightTiltKey] );}
					else if (m_joypmbuyin == 24) FireKeyEvent (updown, DIK_2);
					else if (m_joypmcoin3 == 24) FireKeyEvent (updown, DIK_5);
					else if (m_joypmcoin4 == 24) FireKeyEvent (updown, DIK_6);
					else if (m_joypmcoindoor == 24) FireKeyEvent (updown, DIK_END);
					else if (m_joypmcancel == 24) FireKeyEvent (updown, DIK_7);
					else if (m_joypmdown == 24) FireKeyEvent (updown, DIK_8);
					else if (m_joypmup == 24) FireKeyEvent (updown, DIK_9);
					else if (m_joypmenter == 24) FireKeyEvent (updown, DIK_0);
				}

			FireKeyEvent(updown,input->dwOfs|0x01000000);		// unknown button events
			}
			else //end joy buttons
			{
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
				{	// with selectable axes added to menu, giving prioity in this order... X Axis (the Left/Right Axis), Y Axis
					case DIJOFS_X: 
						if( g_pplayer ) //joyk  rotLeftManual
						{
							if ((m_lr_axis == 1) || (m_ud_axis == 1) || (uShockType != USHOCKTYPE_GENERIC))
							{ // Check if L/R Axis or U/D Axis is selected (in the case of the Generic controller),
							  // or non Generic controllers are being used...
								// Axis Deadzone
								if((u.i<=0) && (u.i>=DeadZ2*(-10))){u.i = 0;}
								if((u.i>=0) && (u.i<=DeadZ2*10)){u.i = 0;}
								if((u.i<0) && (u.i<DeadZ2*(-10))){u.i = u.i + DeadZ2*10;}
								if((u.i>0) && (u.i>DeadZ2*10)){u.i = u.i - DeadZ2*10;}
								if ((uShockType == USHOCKTYPE_PBWIZARD) && (m_lr_axis != 0))
								{
									g_pplayer->UltraNudgeX(-u.i, joyk); //rotate to match Pinball Wizard
								}
								if ((uShockType == USHOCKTYPE_ULTRACADE) && (m_lr_axis != 0))
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
								if ((uShockType == USHOCKTYPE_SIDEWINDER) && (m_lr_axis != 0))
								{
									if (m_lr_axis_reverse == 0)
									{
										g_pplayer->UltraNudgeX(u.i, joyk);
									}
									else
									{
										g_pplayer->UltraNudgeX(-u.i, joyk);
									}
								}
								if ((m_lr_axis == 1) && (uShockType == USHOCKTYPE_GENERIC))
								{ // giving L/R Axis priority over U/D Axis incase both are assigned to same axis
									if (m_lr_axis_reverse == 0)
									{
										g_pplayer->UltraNudgeX(-u.i, joyk);
									}
									else
									{
										g_pplayer->UltraNudgeX(u.i, joyk);
									}
								}
								else if ((m_ud_axis == 1) && (uShockType == USHOCKTYPE_GENERIC))
								{
									if (m_ud_axis_reverse == 0)
									{
										g_pplayer->UltraNudgeY(u.i, joyk);
									}
									else
									{
										g_pplayer->UltraNudgeY(-u.i, joyk);
									}
								}
							}
							else if (m_plunger_axis == 1)
							{	// if X or Y ARE NOT chosen for this axis and Plunger IS chosen for this axis...
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
							}
						}
						break;

					case DIJOFS_Y: 
						if( g_pplayer )
						{
							if ((m_lr_axis == 2) || (m_ud_axis == 2) || (uShockType != USHOCKTYPE_GENERIC))
							{ // Check if L/R Axis or U/D Axis is selected (in the case of the Generic controller),
							  // or non Generic controllers are being used...
								// Axis Deadzone
								if((u.i<=0) && (u.i>=DeadZ2*(-10))){u.i = 0;}
								if((u.i>=0) && (u.i<=DeadZ2*10)){u.i = 0;}
								if((u.i<0) && (u.i<DeadZ2*(-10))){u.i = u.i + DeadZ2*10;}
								if((u.i>0) && (u.i>DeadZ2*10)){u.i = u.i - DeadZ2*10;}
								if ((uShockType == USHOCKTYPE_PBWIZARD) && (m_ud_axis != 0))
								{
									g_pplayer->UltraNudgeY(u.i, joyk); //rotate to match Pinball Wizard
								}
								if ((uShockType == USHOCKTYPE_ULTRACADE) && (m_ud_axis != 0))
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
								if ((uShockType == USHOCKTYPE_SIDEWINDER) && (m_ud_axis != 0))
								{
									if (m_ud_axis_reverse == 0)
									{
										g_pplayer->UltraNudgeY(u.i, joyk);
									}
									else
									{
										g_pplayer->UltraNudgeY(-u.i, joyk);
									}
								}
								if ((m_lr_axis == 2) && (uShockType == USHOCKTYPE_GENERIC))
								{
									if (m_lr_axis_reverse == 0)
									{
										g_pplayer->UltraNudgeX(-u.i, joyk);
									}
									else
									{
										g_pplayer->UltraNudgeX(u.i, joyk);
									}	
								}
								else if ((m_ud_axis == 2) && (uShockType == USHOCKTYPE_GENERIC))
								{
									if (m_ud_axis_reverse == 0)
									{
										g_pplayer->UltraNudgeY(u.i, joyk);
									}
									else
									{
										g_pplayer->UltraNudgeY(-u.i, joyk);
									}
								}
							}
							else if (m_plunger_axis == 2)
							{	// if X or Y ARE NOT chosen for this axis and Plunger IS chosen for this axis...
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
							}
						}
						break;

					case DIJOFS_Z:
						if( g_pplayer )
						{
							if (uShockType == USHOCKTYPE_ULTRACADE)
							{
								g_pplayer->mechPlungerIn(u.i);
							}
							if (((m_plunger_axis != 6) && (m_plunger_axis != 0)) || (m_override_default_buttons == 0))
							{																// with the ability to use rZ for plunger, checks to see if
								if (uShockType == USHOCKTYPE_PBWIZARD) 						// the override is used and if so, if Plunger is set to Rz or
								{															// disabled. If override isn't used, uses default assignment
									g_pplayer->mechPlungerIn(-u.i);							// of the Z axis.
								}
							}
							if (((m_lr_axis == 3) || (m_ud_axis == 3)) && (uShockType == USHOCKTYPE_GENERIC))
							{ // For the sake of priority, Check if L/R Axis or U/D Axis IS selected, and a Generic Gamepad IS being used...
								// Axis Deadzone
								if((u.i<=0) && (u.i>=DeadZ2*(-10))){u.i = 0;}
								if((u.i>=0) && (u.i<=DeadZ2*10)){u.i = 0;}
								if((u.i<0) && (u.i<DeadZ2*(-10))){u.i = u.i + DeadZ2*10;}
								if((u.i>0) && (u.i>DeadZ2*10)){u.i = u.i - DeadZ2*10;}
								if (m_lr_axis == 3)
								{
									if (m_lr_axis_reverse == 0)
									{
										g_pplayer->UltraNudgeX(-u.i, joyk);
									}
									else
									{
										g_pplayer->UltraNudgeX(u.i, joyk);
									}
								}
								else if (m_ud_axis == 3)
								{
									if (m_ud_axis_reverse == 0)
									{
										g_pplayer->UltraNudgeY(u.i, joyk);
									}
									else
									{
										g_pplayer->UltraNudgeY(-u.i, joyk);
									}
								}
							}
							else if (m_plunger_axis == 3)
							{	// if X or Y ARE NOT chosen for this axis and Plunger IS chosen for this axis...
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
							}
						}
						break;

					case DIJOFS_RX:
						if( g_pplayer )
						{
							if (((m_lr_axis == 4) || (m_ud_axis == 4)) && (uShockType == USHOCKTYPE_GENERIC))
							{ // For the sake of priority, Check if L/R Axis or U/D Axis IS selected, and a Generic Gamepad IS being used...
								// Axis Deadzone
								if((u.i<=0) && (u.i>=DeadZ2*(-10))){u.i = 0;}
								if((u.i>=0) && (u.i<=DeadZ2*10)){u.i = 0;}
								if((u.i<0) && (u.i<DeadZ2*(-10))){u.i = u.i + DeadZ2*10;}
								if((u.i>0) && (u.i>DeadZ2*10)){u.i = u.i - DeadZ2*10;}
								if (m_lr_axis == 4)
								{
									if (m_lr_axis_reverse == 0)
									{
										g_pplayer->UltraNudgeX(-u.i, joyk);
									}
									else
									{
										g_pplayer->UltraNudgeX(u.i, joyk);
									}
								}
								else if (m_ud_axis == 4)
								{
									if (m_ud_axis_reverse == 0)
									{
										g_pplayer->UltraNudgeY(u.i, joyk);
									}
									else
									{
										g_pplayer->UltraNudgeY(-u.i, joyk);
									}
								}
							}
							else if (m_plunger_axis == 4)
							{	// if X or Y ARE NOT chosen for this axis and Plunger IS chosen for this axis...
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
							}
						}
						break;

					case DIJOFS_RY:
						if ( g_pplayer )
						{
							if (((m_lr_axis == 5) || (m_ud_axis == 5)) && (uShockType == USHOCKTYPE_GENERIC))
							{ // For the sake of priority, Check if L/R Axis or U/D Axis IS selected, and a Generic Gamepad IS being used...
								// Axis Deadzone
								if((u.i<=0) && (u.i>=DeadZ2*(-10))){u.i = 0;}
								if((u.i>=0) && (u.i<=DeadZ2*10)){u.i = 0;}
								if((u.i<0) && (u.i<DeadZ2*(-10))){u.i = u.i + DeadZ2*10;}
								if((u.i>0) && (u.i>DeadZ2*10)){u.i = u.i - DeadZ2*10;}
								if (m_lr_axis == 5)
								{
									if (m_lr_axis_reverse == 0)
									{
										g_pplayer->UltraNudgeX(-u.i, joyk);
									}
									else
									{
										g_pplayer->UltraNudgeX(u.i, joyk);
									}
								}
								else if (m_ud_axis == 5)
								{
									if (m_ud_axis_reverse == 0)
									{
										g_pplayer->UltraNudgeY(u.i, joyk);
									}
									else
									{
										g_pplayer->UltraNudgeY(-u.i, joyk);
									}
								}
							}
							else if (m_plunger_axis == 5)
							{	// if X or Y ARE NOT chosen for this axis and Plunger IS chosen for this axis...
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
							}
						}
						break;

					case DIJOFS_RZ:
						if( g_pplayer )
						{
							if ((uShockType == USHOCKTYPE_PBWIZARD) && (m_override_default_buttons == 1) && (m_plunger_axis == 6))
							{
								g_pplayer->mechPlungerIn(u.i);
							}
							if (((m_lr_axis == 6) || (m_ud_axis == 6)) && (uShockType == USHOCKTYPE_GENERIC))
							{ // For the sake of priority, Check if L/R Axis or U/D Axis IS selected, and a Generic Gamepad IS being used...
								// Axis Deadzone
								if((u.i<=0) && (u.i>=DeadZ2*(-10))){u.i = 0;}
								if((u.i>=0) && (u.i<=DeadZ2*10)){u.i = 0;}
								if((u.i<0) && (u.i<DeadZ2*(-10))){u.i = u.i + DeadZ2*10;}
								if((u.i>0) && (u.i>DeadZ2*10)){u.i = u.i - DeadZ2*10;}
								if (m_lr_axis == 6)
								{
									if (m_lr_axis_reverse == 0)
									{
										g_pplayer->UltraNudgeX(-u.i, joyk);
									}
									else
									{
										g_pplayer->UltraNudgeX(u.i, joyk);
									}
								}
								else if (m_ud_axis == 6)
								{
									if (m_ud_axis_reverse == 0)
									{
										g_pplayer->UltraNudgeY(u.i, joyk);
									}
									else
									{
										g_pplayer->UltraNudgeY(-u.i, joyk);
									}
								}
							}
							else if (m_plunger_axis == 6)
							{
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
							}
						}
						break;

					case DIJOFS_SLIDER(0):
						if( g_pplayer )
						{
							if (uShockType == USHOCKTYPE_SIDEWINDER)
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
							if (((m_lr_axis == 7) || (m_ud_axis == 7)) && (uShockType == USHOCKTYPE_GENERIC))
							{ // For the sake of priority, Check if L/R Axis or U/D Axis IS selected, and a Generic Gamepad IS being used...
								// Axis Deadzone
								if((u.i<=0) && (u.i>=DeadZ2*(-10))){u.i = 0;}
								if((u.i>=0) && (u.i<=DeadZ2*10)){u.i = 0;}
								if((u.i<0) && (u.i<DeadZ2*(-10))){u.i = u.i + DeadZ2*10;}
								if((u.i>0) && (u.i>DeadZ2*10)){u.i = u.i - DeadZ2*10;}
								if (m_lr_axis == 7)
								{
									if (m_lr_axis_reverse == 0)
									{
										g_pplayer->UltraNudgeX(-u.i, joyk);
									}
									else
									{
										g_pplayer->UltraNudgeX(u.i, joyk);
									}
								}
								else if (m_ud_axis == 7)
								{
									if (m_ud_axis_reverse == 0)
									{
										g_pplayer->UltraNudgeY(u.i, joyk);
									}
									else
									{
										g_pplayer->UltraNudgeY(-u.i, joyk);
									}
								}
							}
							else if (m_plunger_axis == 7)
							{
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
							}
						}
						break;

					case DIJOFS_SLIDER(1):
						if( g_pplayer )
						{
							if (((m_lr_axis == 8) || (m_ud_axis == 8)) && (uShockType == USHOCKTYPE_GENERIC))
							{ // For the sake of priority, Check if L/R Axis or U/D Axis IS selected, and a Generic Gamepad IS being used...
								// Axis Deadzone
								if((u.i<=0) && (u.i>=DeadZ2*(-10))){u.i = 0;}
								if((u.i>=0) && (u.i<=DeadZ2*10)){u.i = 0;}
								if((u.i<0) && (u.i<DeadZ2*(-10))){u.i = u.i + DeadZ2*10;}
								if((u.i>0) && (u.i>DeadZ2*10)){u.i = u.i - DeadZ2*10;}
								if (m_lr_axis == 8)
								{
									if (m_lr_axis_reverse == 0)
									{
										g_pplayer->UltraNudgeX(-u.i, joyk);
									}
									else
									{
										g_pplayer->UltraNudgeX(u.i, joyk);
									}
								}
								else if (m_ud_axis == 8)
								{
									if (m_ud_axis_reverse == 0)
									{
										g_pplayer->UltraNudgeY(u.i, joyk);
									}
									else
									{
										g_pplayer->UltraNudgeY(-u.i, joyk);
									}
								}
							}
							else if (m_plunger_axis == 8)
							{
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
