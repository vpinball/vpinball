#include "stdafx.h"
#include "main.h"

#define INPUT_BUFFER_SIZE 32

#define slintf

static PinInput *s_pPinInput;

static __forceinline unsigned _int64 get_tick_count(void) {__asm rdtsc};

int InputControlRun;

int e_JoyCnt;
int m_PinBallDevice = -1;	// only one accelerometer device

extern int curPlunger;


PinInput::PinInput()
	{

	InputControlRun = 0;

	m_pDI = NULL;
	m_pKeyboard = NULL;
	
	m_head = m_tail = 0;
	m_ptable = NULL;

	memset( m_diq, 0, sizeof( m_diq ) );

	s_pPinInput = this;

	e_JoyCnt = 0;
	for (int k = 0; k < PININ_JOYMXCNT; ++k) m_pJoystick[k] = NULL;
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
    HWND hDlg = (HWND)pContext;
	PinInput *ppinput = (PinInput *)pContext;

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
		dipdw.dwData            = 0;
 
        // Set the deadband to zero
        if( FAILED( ppinput->m_pJoystick[e_JoyCnt]->SetProperty( DIPROP_DEADZONE, &dipdw.diph))) 
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

	PinInput *ppinput = (PinInput *)pvRef;
	
	HRESULT hr;

	hr = ppinput->m_pDI->CreateDeviceEx(lpddi->guidInstance, IID_IDirectInputDevice7
										,(void **)&ppinput->m_pJoystick[e_JoyCnt], NULL);
	if (FAILED(hr))
		{
		ppinput->m_pJoystick[e_JoyCnt] = NULL; //make sure no garbage
		return DIENUM_CONTINUE; //rlc try for another joystick
		}

	hr =  ppinput->m_pJoystick[e_JoyCnt]->GetProperty( DIPROP_PRODUCTNAME,  &dstr.diph);


	//do not allow "UltraCade Pinball" device!!! per contract agreements with UCT/GVR
	//if ((hr == S_OK) && !WzSzStrCmp(dstr.wsz, "UltraCade Pinball")) m_PinBallDevice = e_JoyCnt;	

	if ((hr == S_OK) && !WzSzStrCmp(dstr.wsz, "PinballWizard")) m_PinBallDevice = e_JoyCnt;	// remember accelerometer
	
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


int PinInput::QueueFull( void )
{
	return ( ( ( m_head+1 ) % MAX_KEYQUEUE_SIZE ) == m_tail );
}


int PinInput::QueueEmpty( void )
{
	return m_head == m_tail;
}


void PinInput::AdvanceHead( void )
{
	m_head = ( m_head + 1 ) % MAX_KEYQUEUE_SIZE;
}


void PinInput::AdvanceTail( void )
{
	m_tail = ( m_tail + 1 ) % MAX_KEYQUEUE_SIZE;
}


void PinInput::PushQueue( DIDEVICEOBJECTDATA *data, unsigned int app_data )
{
	if( !data ) return;

	if( QueueFull() ) return;

	m_diq[m_head] = *data;
	//m_diq[m_head].dwTimeStamp = msec();		//rewrite time from game start
	m_diq[m_head].dwSequence = app_data;
	AdvanceHead();
}


DIDEVICEOBJECTDATA *PinInput::GetTail()
{
	if( QueueEmpty() ) return NULL;
	int tmp = m_tail;
	DIDEVICEOBJECTDATA *ptr = &m_diq[tmp];
	AdvanceTail();
	return ptr;	
}


//RLC combine these threads if the Xenon problem is smashed

void InputControlProcess( void *pDummy) 
	{
	DWORD i;
	DIDEVICEOBJECTDATA didod[ INPUT_BUFFER_SIZE ];  // Receives buffered data 
	DWORD dwElements;
	HRESULT hr;		
	LPDIRECTINPUTDEVICE pkyb = s_pPinInput->m_pKeyboard;
	LPDIRECTINPUTDEVICE pjoy;
	HWND hwnd = s_pPinInput->m_hwnd;

	Sleep(1000);  // don't use CPU during intialize
	if(!s_pPinInput) return;	// bad pointer exit

	while(InputControlRun > 0)
		{	
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
						for (i = 0; i < dwElements; i++) s_pPinInput->PushQueue( &didod[i], APP_KEYBOARD ); 

						//slintf( "Pushed %d onto queue %s \n", didod[i].dwData, "keyboard");
						}
					}
				}
			}

		// same for joysticks 

		for (int k = 0; k < e_JoyCnt; ++k)
			{
			pjoy = s_pPinInput->m_pJoystick[k];
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
							for (i = 0; i < dwElements; i++) s_pPinInput->PushQueue( &didod[i], APP_JOYSTICK(k)); 

							//slintf( "Pushed %d onto queue %s\n", didod[i].dwData, "joystick");
							}
						}	
					}
				}
			}
		if (hwnd == GetForegroundWindow())Sleep(1);		// yield only
		else Sleep(100);								// sleep longer if not in focus
		}
		InputControlRun = 2;
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

		hr = m_pKeyboard->SetCooperativeLevel(hwnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);

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

		m_PinBallDevice = -1;

		m_pDI->EnumDevices(DIDEVTYPE_JOYSTICK, DIEnumJoystickCallback, this, DIEDFL_ATTACHEDONLY);//enum Joysticks

		InputControlRun = 1;	//0== stalled, 1==run,  0 < shutting down, 2==terminated
		_beginthread( InputControlProcess, 0, NULL );
		}


void PinInput::UnInit()
	{
    // Unacquire and release any DirectInputDevice objects.
	HRESULT hr;
							//1==run,  0 < shutting down, 2==terminated
	InputControlRun = -100;	// terminate control thread, force after 500mS
	
	while (++InputControlRun < 0) Sleep(5);		// set to exit AND WAIT

	if (!InputControlRun)	//0 == stalled, 1==run,  0 < shutting down, 2==terminated
		{exit (-1500);}

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
	m_ptable->FireKeyEvent( dispid, key );
}


void PinInput::ProcessKeys(PinTable *ptable)
{

	DIDEVICEOBJECTDATA *input;
	
	m_ptable = ptable;

	if (!g_pplayer ) return;		//only when player running

	while(( input = GetTail()))
	{
		if( input->dwSequence == APP_KEYBOARD )
		{
			if( input->dwOfs == DIK_ESCAPE )
			{
				//slintf( "keyboard escape key seen\n" );
				g_pplayer->m_fCloseDown = fTrue;
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

				if( input->dwOfs == DIJOFS_BUTTON(10-1))		/// left
				{
					FireKeyEvent( updown,g_pplayer->m_rgKeys[eLeftFlipperKey] );
				}
				else if( input->dwOfs == DIJOFS_BUTTON(11-1))			/// left 2
				{
					FireKeyEvent( updown, DIK_A);
				}
				else if( input->dwOfs == DIJOFS_BUTTON(2-1))	/// right
				{
					FireKeyEvent( updown,g_pplayer->m_rgKeys[eRightFlipperKey] );
				}
				else if( input->dwOfs == DIJOFS_BUTTON(3-1))		/// right2
				{
					FireKeyEvent( updown,DIK_APOSTROPHE);
				}
				else if( input->dwOfs == DIJOFS_BUTTON(12-1))	/// coin 1
				{
					FireKeyEvent( updown,g_pplayer->m_rgKeys[eAddCreditKey] );
				}
				else if( input->dwOfs == DIJOFS_BUTTON(9-1))	/// start
				{
					FireKeyEvent( updown,g_pplayer->m_rgKeys[eStartGameKey] );
				}
				else if( input->dwOfs == DIJOFS_BUTTON(1-1))	/// plunge
				{
					FireKeyEvent( updown,g_pplayer->m_rgKeys[ePlungerKey] );
				}				
				else if( input->dwOfs == DIJOFS_BUTTON(8-1))	/// exit
				{
					g_pplayer->m_fCloseDown = fTrue;
				}
				else if( input->dwOfs == DIJOFS_BUTTON(14-1))	/// 
				{
					FireKeyEvent( updown, DIK_T);			///test
				}
				else if( input->dwOfs == DIJOFS_BUTTON(15-1))	/// 
				{
					FireKeyEvent( updown, DIK_S);			//service
				}
				else if( input->dwOfs == DIJOFS_BUTTON(7-1))	/// 
				{
					FireKeyEvent( updown, DIK_F1);			//Pause 
				}
				
#if 0
				//rlc@@@@ remove for now
				else if( input->dwOfs == DIJOFS_BUTTON31 )			// Switch to manual joystick control.
				{
					if(updown&1 && g_pplayer != NULL) 
					{					
					if(g_pplayer->m_NudgeManual >= 0) g_pplayer->m_NudgeManual = -1; //standard mode
					else g_pplayer->m_NudgeManual = joyk;			//only one joystick has manual control
						
					rotLeftManual = g_pplayer->m_NudgeManual >= 0;	// for normal UC Table
					}
				}
				else if( input->dwOfs == DIJOFS_BUTTON30 )			// Manual Joystick control
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
				else if( input->dwOfs == DIJOFS_BUTTON29)	/// volume up
				{
					FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeUp] );
				}
				else if( input->dwOfs == DIJOFS_BUTTON28 )	/// volume down
				{
					FireKeyEvent( updown,g_pplayer->m_rgKeys[eVolumeDown] );
				}
#endif
				else FireKeyEvent(updown,input->dwOfs|0x01000000);		// unknown button events
			}
			else //end joy buttons
			{
				// Convert to signed int
				union { int i; unsigned int ui; } u;
				u.ui = input->dwData;
				
				switch (input->dwOfs)		// Axis, Sliders and POV
				{
					case DIJOFS_X: 
						if( g_pplayer ) //joyk  rotLeftManual
						{
							if (m_PinBallDevice == joyk) 
							{
								g_pplayer->m_PinBallNudgeY(-u.i, joyk); //rotate to match joystick
							}
							else
							{
								if (rotLeftManual)
								{
									g_pplayer->m_PinBallNudgeY(u.i, joyk);
								}
								else
								{
									g_pplayer->m_PinBallNudgeX(u.i, joyk);
								}
							}
						}
						break;

					case DIJOFS_Y: 
						if( g_pplayer ) 
						{
							if (m_PinBallDevice == joyk || rotLeftManual) 
							{
								g_pplayer->m_PinBallNudgeX(-u.i, joyk); //rotate to match joystick
							}
							else 
							{
								g_pplayer->m_PinBallNudgeY(u.i, joyk);		
							}
						}
						break;

					case DIJOFS_Z: 
						if( g_pplayer )
						{
							if (m_PinBallDevice == joyk) 
							{
								g_pplayer->mechPlungerIn(u.i);
							}
						}
						break;

					case DIJOFS_RX:break;
					case DIJOFS_RY:break;
					case DIJOFS_RZ:break;
					case DIJOFS_SLIDER(0):
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

int PinInput::GetNextKey() //rlc flush queue and return last valid keyboard key 
			{
	DIDEVICEOBJECTDATA *key = NULL, *input;
	DWORD dev_type = APP_KEYBOARD; // used to flag the device type, 0 is ky

	while (input = GetTail())		//flush and remember last keyboard key event
		{
		if (input->dwSequence == APP_KEYBOARD ) 
			{
			key = input; // yes remember it
			dev_type = input->dwSequence;
			}
		else if( input->dwSequence >= APP_JOYSTICKMN && input->dwSequence <= APP_JOYSTICKMX )
			{	
			// joystick buttons are now hard mapped to System Registry, 
			// to enable this code for soft mapping, the button names need rgszKeyName[key] equivelent tables
			// in PinInput.cpp, KeysProc(), case WM_TIMER:, need to be extended
			if (input->dwOfs >= DIJOFS_BUTTON0 && input->dwOfs <= DIJOFS_BUTTON31) 
				{
				key = input; // yes remember it
				dev_type = input->dwSequence; //shift type to high nibble
				}
			}
		}

	if (key) { return key->dwOfs|(dev_type << 24);}				//if valid event found return it 

	return NULL;	
}

