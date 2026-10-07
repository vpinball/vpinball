'Last Updated in VBS v3.62

'Spooky Pinball PinHeck (America's Most Haunted, Domino's Spectacular Pinball Adventure, Rob Zombie's Spookshow International, The Jetsons)

'Switch numbers are column*10+row (PinMAME): 11-88 the playfield matrix, 1-8 and 91-98 the cabinet inputs
'Lamps likewise: 11-88 the lamp matrix, 91 the start button lamp

'The flippers are CPU controlled and there is no GameOnSolenoid. Drive the flipper *buttons* as switches 112/114,
' never 3/4 directly: PinMAME copies 112/114 onto the game's button switches 3/4 every frame, so a
' table setting 3/4 sees them released again within a frame and the flipper drops after the power stroke.
' The flipper coils are mirrored to the standard outputs, so SolCallback(sLRFlipper)/SolCallback(sLLFlipper)
' (46/48, power 45/47) work as usual; Rob Zombie's upper flipper is sURFlipper (34, power 33).
'The board has 16 GI strings and PinMAME only 8 GI outputs, so PinHeck's GI comes out as solenoids, never as
' GIString callbacks: GI 0-7 are solenoids 25-32, GI 8-15 are 37-44. Some of these drive flashers (Jetsons:
' 37 ramp, 38 scoop flasher). Use SolModCallback with UseVPMModSol for their brightness.
'Solenoids 51-64 are levels 0-255 rather than coils: the RGB LEDs (51-56, 62-64) and the servos (57-61, e.g.
' Jetsons' Orbitty topper on 57). Read them with SolModCallback.
'The coin door has no Down/Up buttons, only Back, Enter and a User button (the launch button on Jetsons);
' the game's menus are navigated with the flipper buttons.

'Solenoid map: 1-24 the coils, 25-32 GI 0-7, 33-36 upper flipper (power/hold), 37-44 GI 8-15, 45-48 lower flippers
'(power/hold), 51-64 RGB LEDs and servos. See src/wpc/pinheck.c in PinMAME for the full table

Option Explicit
LoadCore
Private Sub LoadCore
	On Error Resume Next
	If VPBuildVersion < 0 Or Err Then
		Dim fso : Set fso = CreateObject("Scripting.FileSystemObject") : Err.Clear
		ExecuteGlobal fso.OpenTextFile("core.vbs", 1).ReadAll    : If Err Then MsgBox "Can't open ""core.vbs""" : Exit Sub
		ExecuteGlobal fso.OpenTextFile("VPMKeys.vbs", 1).ReadAll : If Err Then MsgBox "Can't open ""vpmkeys.vbs""" : Exit Sub
	Else
		ExecuteGlobal GetTextFile("core.vbs")    : If Err Then MsgBox "Can't open ""core.vbs"""    : Exit Sub
		ExecuteGlobal GetTextFile("VPMKeys.vbs") : If Err Then MsgBox "Can't open ""vpmkeys.vbs""" : Exit Sub
	End If
End Sub

'-------------------------
' pinHeck Data
'-------------------------
' Cabinet inputs
Const swCoinDoorClosed = 1 ' set = CLOSED, as WPC's sw 22
Const swUser           = 2 ' the User button: the launch button on Jetsons
Const swBack           = 5
Const swEnter          = 6
Const swCoin1          = 7
Const swTilt           = 8
Const swStartButton    = 94

' Flipper buttons, copied onto the game's button switches 3/4 (see the note above)
Const swLRFlip = 112
Const swLLFlip = 114

' core.vbs names the flipper coils sLRFlipper = 46, sLLFlipper = 48 and sURFlipper = 34; all are correct here

' The User button on the cabinet's lockbar key; VPX versions without LockbarKey use keyFire1
Private keyUser : keyUser = keyFire1
On Error Resume Next
keyUser = LockbarKey
On Error Goto 0

' Help Window
vpmSystemHelp = "Spooky Pinball pinHeck keys:" & vbNewLine &_
  vpmKeyName(keyInsertCoin1) & vbTab & "Insert Coin" & vbNewLine &_
  vpmKeyName(keyCancel) & vbTab & "Back (Coin Door)" & vbNewLine &_
  vpmKeyName(keyEnter) & vbTab & "Enter (Coin Door, opens the service menu)" & vbNewLine &_
  vpmKeyName(keyUser) & vbTab & "User / Launch Button" & vbNewLine &_
  vpmKeyName(keyCoinDoor) & vbTab & "Open/Close Coin Door"

' Options Menu (No Dips)
Private Sub pinheckShowDips
	If Not IsObject(vpmDips) Then ' First time
		Set vpmDips = New cvpmDips
		With vpmDips
			.AddForm  80, 0, "Option Menu"
			.AddLabel 0, 0, 250, 20, "No Options In This Table At This Time"
		End With
	End If
	vpmDips.ViewDips
End Sub
Set vpmShowDips = GetRef("pinheckShowDips")
Private vpmDips

' Keyboard handlers
Function vpmKeyDown(ByVal keycode)
	Dim swCopy
	vpmKeyDown = True ' assume we handle the key
	With Controller
		Select Case keycode
			Case LeftFlipperKey
				.Switch(swLLFlip) = True : vpmKeyDown = False : vpmFlips.FlipL True
			Case RightFlipperKey
				.Switch(swLRFlip) = True : vpmKeyDown = False : vpmFlips.FlipR True
			Case keyInsertCoin1  vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin1'" : If Not IsEmpty(Eval("SCoin")) Then Playsound SCoin
			Case StartGameKey	 swCopy = swStartButton :	 .Switch(swCopy) = True
			Case keyCancel		 swCopy = swBack :			 .Switch(swCopy) = True
			Case keyEnter		 swCopy = swEnter :			 .Switch(swCopy) = True
			Case keyUser		 swCopy = swUser :			 .Switch(swCopy) = True
			Case keyCoinDoor	 swCopy = swCoinDoorClosed : If toggleKeyCoinDoor Then .Switch(swCopy) = Not .Switch(swCopy) Else .Switch(swCopy) = Not inverseKeyCoinDoor
			Case keyBangBack	 vpmNudge.DoMechTilt
			Case Else			 vpmKeyDown = False
		End Select
	End With
End Function

Function vpmKeyUp(ByVal keycode)
	Dim swCopy
	vpmKeyUp = True ' assume we handle the key
	With Controller
		Select Case keycode
			Case LeftFlipperKey
				.Switch(swLLFlip) = False : vpmKeyUp = False : vpmFlips.FlipL False
			Case RightFlipperKey
				.Switch(swLRFlip) = False : vpmKeyUp = False : vpmFlips.FlipR False
			Case StartGameKey	 swCopy = swStartButton :	 .Switch(swCopy) = False
			Case keyCancel		 swCopy = swBack :			 .Switch(swCopy) = False
			Case keyEnter		 swCopy = swEnter :			 .Switch(swCopy) = False
			Case keyUser		 swCopy = swUser :			 .Switch(swCopy) = False
			Case keyCoinDoor	 swCopy = swCoinDoorClosed : If toggleKeyCoinDoor = False Then .Switch(swCopy) = inverseKeyCoinDoor
			Case keyShowOpts	 .Pause = True : vpmShowOptions : .Pause = False
			Case keyShowKeys	 .Pause = True : vpmShowHelp : .Pause = False
			Case keyAddBall		 .Pause = True : vpmAddBall	 : .Pause = False
			Case keyReset		 .Stop : BeginModal : .Run : vpmTimer.Reset : EndModal
			Case keyFrame		 .LockDisplay = Not .LockDisplay
			Case keyDoubleSize	 .DoubleSize  = Not .DoubleSize
			Case Else			 vpmKeyUp = False
		End Select
	End With
End Function
