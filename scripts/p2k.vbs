'Pinball 2000 (Williams/Midway, 1999)

'Switch numbers here are the machine's own, column*10 + row, which is what the game's tables and
'the service menu use - so sw 13 really is column 1 row 3. Columns 1-9 are the playfield, column 10
'is the coin door's diagnostic buttons and column 11 the cabinet. Coins are not in that scheme:
'the driver board reports them in a register of their own, which PinMAME carries as switches 2/3/8.

'Two things differ from WPC and will catch you out:
' - There is no GameOnSolenoid. The flippers are ROM controlled (the game drives the coils), so use
'   SolCallback(sLLFlipper)/SolCallback(sLRFlipper) - solenoids 48 and 46 - and drive the flipper
'   *buttons* as switches 115/116. There are no upper flippers on either shipped game.
' - Optos (trough, poppers, lockups - RFM 41-47/51-56, Episode I 41-48/51/52/58) are declared
'   active low in the driver, so drive them the normal way round: Switch = True means a ball is
'   there. Do not invert them in the table.
' - The coin door switch reads CLOSED when set (sw 112 is "Coin Door Closed"), the opposite of WPC's
'   sw 22. The machine powers up with the door open and holds high voltage off until it closes, so
'   the table must close it before any coil will fire.

'Solenoid map: the power driver board has 48 outputs. 1-32 are PinMAME solenoids 1-32, the flipper
'coils (board 33-36) come out as 45-48, and board 37-48 are published as custom solenoids 51-62 -
'so Lock Diverter and Up/Down Ramp on RFM are 51-54, and Episode I's neon/knocker/shaker/topper are
'55/56/57/58. See src/wpc/p2k.c in PinMAME for the full table.

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
' Pinball 2000 Data
'-------------------------
' Coin slots. Not column*10+row: these are the coin register's own bits, which land at these
' numbers. Only three are mapped - the board has no known position for a fourth
Const swCoin1 = 2
Const swCoin2 = 3
Const swCoin3 = 8

' Coin door diagnostic buttons (column 10)
Const swEscape = 101
Const swDown   = 102
Const swUp	   = 103
Const swEnter  = 104

' Cabinet (column 11)
Const swSlamTilt	   = 111
Const swCoinDoorClosed = 112 ' set = CLOSED, see the note above
Const swPlumbTilt	   = 113
Const swRFlipperButton = 115
Const swLFlipperButton = 116
Const swRActionButton  = 117 ' the two lockbar Action buttons - Pinball 2000 specific
Const swLActionButton  = 118

' Playfield
Const swStartButton  = 13
Const swLaunchButton = 23

' core.vbs names the flipper coils sLLFlipper = 48 and sLRFlipper = 46; both are correct here

' Help Window
vpmSystemHelp = "Pinball 2000 keys:" & vbNewLine &_
  vpmKeyName(keyInsertCoin1) & vbTab & "Insert Coin #1" & vbNewLine &_
  vpmKeyName(keyInsertCoin2) & vbTab & "Insert Coin #2" & vbNewLine &_
  vpmKeyName(keyInsertCoin3) & vbTab & "Insert Coin #3" & vbNewLine &_
  vpmKeyName(keyEnter) & vbTab & "Enter (Coin Door)" & vbNewLine &_
  vpmKeyName(keyUp) & vbTab & "Up (Coin Door)" & vbNewLine &_
  vpmKeyName(keyDown) & vbTab & "Down (Coin Door)" & vbNewLine &_
  vpmKeyName(keyCancel) & vbTab & "Escape (Coin Door)" & vbNewLine &_
  vpmKeyName(keySlamDoorHit) & vbTab & "Slam Tilt" & vbNewLine &_
  vpmKeyName(keyCoinDoor) & vbTab & "Open/Close Coin Door (must be CLOSED for high voltage)" & vbNewLine &_
  vpmKeyName(keyFire1) & vbTab & "Left Action Button" & vbNewLine &_
  vpmKeyName(keyFire2) & vbTab & "Right Action Button"

' Dip Switch / Options Menu
Private Sub p2kShowDips
	If Not IsObject(vpmDips) Then ' First time
		Set vpmDips = New cvpmDips
		With vpmDips
			.AddForm 100, 240, "DIP Switches"
			.AddFrame 0, 0, 80, "Country", &H0f,_
				Array("USA / Canada", &H00, "Germany", &H01,_
					  "France", &H02, "United Kingdom", &H03,_
					  "Spain", &H04, "Europe",  &H07,_
					  "Japan", &H08)
		End With
	End If
	vpmDips.ViewDips
End Sub
Set vpmShowDips = GetRef("p2kShowDips")
Private vpmDips

' Keyboard handlers
Function vpmKeyDown(ByVal keycode)
	Dim swCopy
	vpmKeyDown = True ' assume we handle the key
	With Controller
		Select Case keycode
			Case LeftFlipperKey
				.Switch(swLFlipperButton) = True : vpmKeyDown = False : vpmFlips.FlipL True
			Case RightFlipperKey
				.Switch(swRFlipperButton) = True : vpmKeyDown = False : vpmFlips.FlipR True
			Case keyInsertCoin1  vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin1'" : If Not IsEmpty(Eval("SCoin")) Then Playsound SCoin
			Case keyInsertCoin2  vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin2'" : If Not IsEmpty(Eval("SCoin")) Then Playsound SCoin
			Case keyInsertCoin3  vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin3'" : If Not IsEmpty(Eval("SCoin")) Then Playsound SCoin
			Case StartGameKey	 swCopy = swStartButton :	  .Switch(swCopy) = True
			Case keyCancel		 swCopy = swEscape :		  .Switch(swCopy) = True
			Case keyDown		 swCopy = swDown :			  .Switch(swCopy) = True
			Case keyUp			 swCopy = swUp :			  .Switch(swCopy) = True
			Case keyEnter		 swCopy = swEnter :			  .Switch(swCopy) = True
			Case keySlamDoorHit	 swCopy = swSlamTilt :		  .Switch(swCopy) = True
			Case keyFire1		 swCopy = swLActionButton :	  .Switch(swCopy) = True
			Case keyFire2		 swCopy = swRActionButton :	  .Switch(swCopy) = True
			' The door switch reads closed when set, so the sense here is the opposite of WPC's: a "closed" door is Switch = True
			Case keyCoinDoor	 swCopy = swCoinDoorClosed :  If toggleKeyCoinDoor Then .Switch(swCopy) = Not .Switch(swCopy) Else .Switch(swCopy) = inverseKeyCoinDoor
			Case keyBangBack	 vpmNudge.DoMechTilt
			Case keyVPMVolume	 vpmVol
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
				.Switch(swLFlipperButton) = False : vpmKeyUp = False : vpmFlips.FlipL False
			Case RightFlipperKey
				.Switch(swRFlipperButton) = False : vpmKeyUp = False : vpmFlips.FlipR False
			Case StartGameKey	 swCopy = swStartButton :	  .Switch(swCopy) = False
			Case keyCancel		 swCopy = swEscape :		  .Switch(swCopy) = False
			Case keyDown		 swCopy = swDown :			  .Switch(swCopy) = False
			Case keyUp			 swCopy = swUp :			  .Switch(swCopy) = False
			Case keyEnter		 swCopy = swEnter :			  .Switch(swCopy) = False
			Case keySlamDoorHit  swCopy = swSlamTilt :		  .Switch(swCopy) = False
			Case keyFire1		 swCopy = swLActionButton :	  .Switch(swCopy) = False
			Case keyFire2		 swCopy = swRActionButton :	  .Switch(swCopy) = False
			Case keyCoinDoor	 swCopy = swCoinDoorClosed :  If toggleKeyCoinDoor = False Then .Switch(swCopy) = Not inverseKeyCoinDoor
			Case keyShowOpts	 .Pause = True : vpmShowOptions : .Pause = False
			Case keyShowKeys	 .Pause = True : vpmShowHelp : .Pause = False
			Case keyShowDips	 If IsObject(vpmShowDips) Then .Pause = True : vpmShowDips : .Pause = False
			Case keyAddBall		 .Pause = True : vpmAddBall	 : .Pause = False
			Case keyReset		 .Stop : BeginModal : .Run : vpmTimer.Reset : EndModal
			Case keyFrame		 .LockDisplay = Not .LockDisplay
			Case keyDoubleSize	 .DoubleSize  = Not .DoubleSize
			Case Else			 vpmKeyUp = False
		End Select
	End With
End Function
