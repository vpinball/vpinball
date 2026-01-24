'Last Updated in VBS v3.62

'Using direct mappings (key number) should be avoided, as these only work with US keyboards,
'do not support mapping to other input systems (joystick, VR controller, ...), can't be redefined easily,
'may conflict with existing or future input defaults, do not apply the user keyboard layout, and
'only cover a subset of the keyboard.
'
'Using the action mappings provided by VPX should always be favored (see VPXActionKey).

'Key codes that can be used in the mappings below:
'0			11		L CONTROL		29
'1			2		LEFT ARROW		203
'2			3		L SHIFT			42
'3			4		L WIN			219
'4			5		M				50
'5			6		MINUS			12
'6			7		MULTIPLY		55
'7			8		N				49
'8			9		NUM LOCK		69
'9			10		NUMPAD 0		82
'A			30		NUMPAD 1		79
'ADD		78		NUMPAD 2		80
'APOSTROPHE 40		NUMPAD 3		81
'AT			145		NUMPAD 4		75
'AX			150		NUMPAD 5		76
'B			48		NUMPAD 6		77
'BACK		14		NUMPAD 7		71
'BACKSLASH	43		NUMPAD 8		72
'BACKSPACE	14		NUMPAD 9		73
'C			46		NUMPAD COMMA	179
'CALCULATOR 161		NUMPAD ENTER	156
'CAPS LOCK	58		NUMPAD EQUALS	141
'COLON		146		NUMPAD MINUS	74
'COMMA		51		NUMPAD PERIOD	83
'D			32		NUMPAD PLUS		78
'DECIMAL	83		NUMPAD SLASH	181
'DELETE		211		NUMPAD ASTERISK	55
'DIVIDE		181		O				24
'DOWN		208		P				25
'DOWN ARROW 208		PAUSE			197
'E			18		PERIOD			52
'END		207		PAGE DOWN		209
'EQUALS		13		PAGE UP			201
'ESCAPE		1		Q				16
'F			33		R				19
'F1			59		R ALT			184
'F2			60		R BRACKET		27
'F3			61		R CTRL			157
'F4			62		RETURN			28
'F5			63		RIGHT			205
'F6			64		RIGHT ARROW		205
'F7			65		R MENU			184
'F8			66		R SHIFT			54
'F9			67		R WIN			220
'F10		68		S				31
'F11		87		SCROLL			70
'F12		88		SEMICOLO		39
'F13		100		SLASH			53
'F14		101		SPACE			57
'F15		102		STOP			149
'G			34		T				20
'H			35		TAB				15
'HOME		199		U				22
'I			23		UNDERLINE		147
'INSERT		210		UP ARROW		200
'J			36		V				47
'K			37		W				17
'L			38		X				45
'L ALT		56		Y				21
'L BRACKET	26		Z				44

Option Explicit

'
' This is a list of standard keys used by *.vbs files
'

' VBS based user interface
' (only for VPinMAME COM component, not needed for PinMAME plugin)
Const keyShowOpts	= 59                  '(F1)  Show options
Const keyShowKeys	= 60                  '(F2)  Show Keys
Dim keyReset: keyReset = VPXActionKey(19) '(F3)  Reset Emulation
Const keyFrame		= 62                  '(F4)  Toggle Window Lock (no operation if not usig VPinMAME)
Const keyDoubleSize	= 63                  '(F5)  Toggle displaysize (no operation if not usig VPinMAME)
Const keyShowDips	= 64                  '(F6)  Show Dip Switch / Options Menu
Const keyVPMVolume	= 88                  '(F12) Show input box to set VPM Volume (no operation if not usig VPinMAME)

' Cabinet switches
Dim keyAddBall:		keyAddBall		= VPXActionKey(22) '(B)		Add extra ball
Dim keyBangBack:	keyBangBack		= VPXActionKey(13) '(T)		Bang Back
Dim keyInsertCoin1:	keyInsertCoin1	= VPXActionKey( 9) '(5)		Insert coin in slot 1
Dim keyInsertCoin2:	keyInsertCoin2	= VPXActionKey(10) '(4)		Insert coin in slot 2
Dim keyInsertCoin3:	keyInsertCoin3	= VPXActionKey(11) '(3)		Insert coin in slot 3
Dim keyInsertCoin4:	keyInsertCoin4	= VPXActionKey(12) '(6)		Insert coin in slot 4
Dim keySlamDoorHit:	keySlamDoorHit	= VPXActionKey(23) '(Home)	Slam Tilt
Dim keyCoinDoor:	keyCoinDoor		= VPXActionKey(24) '(End)	Opening/Closing Coin Door
Const toggleKeyCoinDoor	 = True		'If true then a key press/switch change will flip the state from on/off, if false then one will have the real coindoor behaviour (for cabinet setups with real coindoors)
Const inverseKeyCoinDoor = False	'If false then the coindoor switch behaves like a original coindoor (key pressed = closed, key not pressed = open), otherwise the switch is inverted (key pressed = open, key not pressed = closed)

' Service switches
Dim keyCancel:			keyCancel			= VPXActionKey(25)	'(7)	Coin Door Cancel Button (WPC)
Dim keyDown:			keyDown				= VPXActionKey(26)	'(8)	Coin Door Down Button (WPC)
Dim keyUp:				keyUp				= VPXActionKey(27)	'(9)	Coin Door Up Button (WPC)
Dim keyEnter:			keyEnter			= VPXActionKey(28)	'(0)	Coin Door Enter Button (WPC)
Dim keyAdvance:			keyAdvance			= VPXActionKey(26)	'(8)	Coin Door Advance Button
Dim keyUpDown:			keyUpDown			= VPXActionKey(25)	'(7)	Coin Door Up/Down Button
Dim keyCPUDiag:			keyCPUDiag			= VPXActionKey(27)	'(9)	CPU Diagnostic Button
Dim keySoundDiag:		keySoundDiag		= VPXActionKey(28)	'(0)	Sound CPU Diagnostic Button
Dim keyMasterEnter:		keyMasterEnter		= VPXActionKey(30)	'(PgUp)	Master Enter
Dim keyHiScoreReset:	keyHiScoreReset		= VPXActionKey(29)	'(6)	Reset Hiscores
Dim keySelfTest:		keySelfTest			= VPXActionKey(25)	'(7)
Dim keyBlack:			keyBlack			= VPXActionKey(25)	'(7)	Sega/Data East/Stern
Dim keyGreen:			keyGreen			= VPXActionKey(26)	'(8)	Sega/Data East/Stern
Dim keyRed:				keyRed				= VPXActionKey(27)	'(9)	Sega/Data East/Stern
Dim keyConfigurations:	keyConfigurations	= VPXActionKey(31)	'(-)	Taito
Dim keyKPGame:			keyKPGame			= VPXActionKey(29)	'(6)	Bally 6803
Const keyKPEnter							= 156				'(NumpadEnter) see 6803.vbs for complete keypad

' Optional keys
' These are not used nor handled by the vbs files
' They are only here to provide consistent keys for the games.

Const keyUpperLeft	   = 30  '(A)	Upper Left Flipper button
Const keyUpperRight	   = 40  '(')	Upper Right Flipper button
Const keyFront		   = 3   '(2)	Front button (e.g. buy-in)
Const keyRules		   = 19  '(R)	Show rules
Const keyJoyUp		   = 200 '(Up arrow)
Const keyJoyDown	   = 208 '(Down arrow)
Const keyJoyLeft	   = 203 '(Left arrow)
Const keyJoyRight	   = 205 '(Right arrow)
Const keyFire1		   = 29  '(Left CTRL)
Const keyFire2		   = 184 '(Right Alt)
Const keyGameSpecific1 = 16  '(Q)
Const keyGameSpecific2 = 17  '(W)
Const keyGameSpecific3 = 18  '(E)
Const keyGameSpecific4 = 19  '(R)

' Backward compatibility: Staged flippers needed the user to edit script files, adapting the 2 following variables.
' These are now managed in the application UI but some tables still expect these 2 variables to be defined. VPX simply mirrors the
' application settings (but does not dynamically update them: old tables will need to be restarted after adjusting)
Dim keyStagedFlipperL : keyStagedFlipperL = StagedLeftFlipperKey
Dim keyStagedFlipperR : keyStagedFlipperR = StagedRightFlipperKey
