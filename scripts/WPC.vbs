'Last Updated in VBS v3.56

'sw116 and sw118 are commonly used for UL and UR flippers(in most WPC pins),
'while in IJ (i think the only one that has it different?!) these are the ramp switches.
'Everytime you press the flipper keys those switches are �on�, so one did use the cSingleLFlip variable to prevent that.
'Nowadays cSingleLFlip has been replaced by the separate keyStagedFlipperL.

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
' WPC Data
'-------------------------
' Flipper Solenoid
Const GameOnSolenoid = 31
'-----WPC flipper solenoid notes-----
'***These games will work perfectly:***
'WPC alphanumerics, T2, Gilligan's Island, Hurricane, Party Zone (pre-Addams Family DMD games)

'***These games use rom-controlled flippers, but still have a usable flipper solenoid:***
'WPC Fliptronics & WPC Security (Addam's Family through Jack Bot / WHO dunnit)
'- (YMMV. Beware stuck balls under upper flippers. )
'- Some of these games may have solenoid jitter with solmodcallbacks enabled. If flippers die randomly, set the vpmFlips.InitDelay option.
'- STAGED FLIPPER NOTE: Some games (Red & Ted, probably others) use swapped around upper flipper solenoid numbers, some table script modification may be necessary!

'***These games work only with VPM 3.1 and above:***
'WPC-95 Machines (Congo / Attack From Mars onwards)

' Cabinet switches
Const swCoin1  = 1
Const swCoin2  = 2
Const swCoin3  = 3
Const swCoin4  = 4
Const swCancel = 5
Const swDown   = 6
Const swUp	   = 7
Const swEnter  = 8

' Forgot these in earlier vbs files
Private swStartButtonX,swCoinDoorX,swSlamTiltX
On Error Resume Next
If swStartButton = 13 Or Err Then swStartButtonX = 13 Else swStartButtonX = swStartButton
If swCoinDoor	 = 22 Or Err Then swCoinDoorX	 = 22 Else swCoinDoorX	  = swCoinDoor
If swSlamTilt	 = 21 Or Err Then swSlamTiltX	 = 21 Else swSlamTiltX	  = swSlamTilt
On Error Goto 0

Const swLRFlip = 112
Const swLLFlip = 114
Const swURFlip = 116
Const swULFlip = 118

' Help Window
vpmSystemHelp = "Williams WPC keys:" & vbNewLine &_
  vpmKeyName(keyInsertCoin1) & vbTab & "Insert Coin #1" & vbNewLine &_
  vpmKeyName(keyInsertCoin2) & vbTab & "Insert Coin #2" & vbNewLine &_
  vpmKeyName(keyInsertCoin3) & vbTab & "Insert Coin #3" & vbNewLine &_
  vpmKeyName(keyInsertCoin4) & vbTab & "Insert Coin #4" & vbNewLine &_
  vpmKeyName(keyCancel) & vbTab & "Escape (Coin Door)" & vbNewLine &_
  vpmKeyName(keyDown) & vbTab & "Down (Coin Door)" & vbNewLine &_
  vpmKeyName(keyUp) & vbTab & "Up (Coin Door)" & vbNewLine &_
  vpmKeyName(keyEnter) & vbTab & "Enter (Coin Door)" & vbNewLine &_
  vpmKeyName(keySlamDoorHit) & vbTab & "Slam Tilt" & vbNewLine &_
  vpmKeyName(keyCoinDoor) & vbTab & "Open/Close Coin Door"

' Dip Switch / Options Menu
Private Sub wpcShowDips
	If Not IsObject(vpmDips) Then ' First time
		Set vpmDips = New cvpmDips
		With vpmDips
			.AddForm 100, 240, "DIP Switches"
			.AddFrame 0,190, 80, "Misc", 0, Array("W20",&H04,"W19",&H08)
			.AddFrame 0, 0, 80, "Country", &Hf0,_
				Array("USA", &H00, "USA", &Hf0, "European", &Hd0,_
					  "Export", &Ha0, "Export Alt", &H80, "France", &Hb0,_
					  "France 1", &H10, "France 2", &H30, "France 3", &H90,_
					  "Germany", &H20, "Spain", &He0, "UK", &Hc0)
		End With
	End If
	vpmDips.ViewDips
End Sub
Set vpmShowDips = GetRef("wpcShowDips")
Private vpmDips

' Keyboard handlers
Function vpmKeyDown(ByVal keycode)
	Dim swCopy
	vpmKeyDown = True ' assume we handle the key
	With Controller
		Select Case keycode
			Case LeftFlipperKey
				.Switch(swLLFlip) = True : vpmKeyDown = False : vpmFlips.FlipL True
				If keycode = keyStagedFlipperL Then ' as vbs will not evaluate the Case keyStagedFlipperL then, also handle it here
					vpmFlips.FlipUL True
					If vpmFlips.FlipperSolNumber(2) <> 0 Then .Switch(swULFlip) = True
				End If
			Case RightFlipperKey
				.Switch(swLRFlip) = True : vpmKeyDown = False : vpmFlips.FlipR True
				If keycode = keyStagedFlipperR Then ' as vbs will not evaluate the Case keyStagedFlipperR then, also handle it here
					vpmFlips.FlipUR True
					If vpmFlips.FlipperSolNumber(3) <> 0 Then .Switch(swURFlip) = True
				End If
			Case keyStagedFlipperL vpmFlips.FlipUL True : If vpmFlips.FlipperSolNumber(2) <> 0 Then .Switch(swULFlip) = True
			Case keyStagedFlipperR vpmFlips.FlipUR True : If vpmFlips.FlipperSolNumber(3) <> 0 Then .Switch(swURFlip) = True
			Case keyInsertCoin1  vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin1'" : If Not IsEmpty(Eval("SCoin")) Then Playsound SCoin
			Case keyInsertCoin2  vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin2'" : If Not IsEmpty(Eval("SCoin")) Then Playsound SCoin
			Case keyInsertCoin3  vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin3'" : If Not IsEmpty(Eval("SCoin")) Then Playsound SCoin
			Case keyInsertCoin4  vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin4'" : If Not IsEmpty(Eval("SCoin")) Then Playsound SCoin
			Case StartGameKey	 swCopy = swStartButtonX : .Switch(swCopy) = True
			Case keyCancel		 swCopy = swCancel :	   .Switch(swCopy) = True
			Case keyDown		 swCopy = swDown :		   .Switch(swCopy) = True
			Case keyUp			 swCopy = swUp :		   .Switch(swCopy) = True
			Case keyEnter		 swCopy = swEnter :		   .Switch(swCopy) = True
			Case keySlamDoorHit	 swCopy = swSlamTiltX :	   .Switch(swCopy) = True
			Case keyCoinDoor	 swCopy = swCoinDoorX :	   If toggleKeyCoinDoor Then .Switch(swCopy) = Not .Switch(swCopy) Else .Switch(swCopy) = Not inverseKeyCoinDoor
			Case keyBangBack	 vpmNudge.DoNudge   0, 6
			Case LeftTiltKey	 vpmNudge.DoNudge  75, 2
			Case RightTiltKey	 vpmNudge.DoNudge 285, 2
			Case CenterTiltKey	 vpmNudge.DoNudge   0, 2
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
				.Switch(swLLFlip) = False : vpmKeyUp = False : vpmFlips.FlipL False
				If keycode = keyStagedFlipperL Then ' as vbs will not evaluate the Case keyStagedFlipperL then, also handle it here
					vpmFlips.FlipUL False
					If vpmFlips.FlipperSolNumber(2) <> 0 Then .Switch(swULFlip) = False
				End If
			Case RightFlipperKey
				.Switch(swLRFlip) = False : vpmKeyUp = False : vpmFlips.FlipR False
				If keycode = keyStagedFlipperR Then ' as vbs will not evaluate the Case keyStagedFlipperR then, also handle it here
					vpmFlips.FlipUR False
					If vpmFlips.FlipperSolNumber(3) <> 0 Then .Switch(swURFlip) = False
				End If
			Case keyStagedFlipperL vpmFlips.FlipUL False : If vpmFlips.FlipperSolNumber(2) <> 0 Then .Switch(swULFlip) = False
			Case keyStagedFlipperR vpmFlips.FlipUR False : If vpmFlips.FlipperSolNumber(3) <> 0 Then .Switch(swURFlip) = False
			Case keyCancel		 swCopy = swCancel :	   .Switch(swCopy) = False
			Case keyDown		 swCopy = swDown :		   .Switch(swCopy) = False
			Case keyUp			 swCopy = swUp :		   .Switch(swCopy) = False
			Case keyEnter		 swCopy = swEnter :		   .Switch(swCopy) = False
			Case keySlamDoorHit	 swCopy = swSlamTiltX :	   .Switch(swCopy) = False
			Case StartGameKey	 swCopy = swStartButtonX : .Switch(swCopy) = False
			Case keyCoinDoor	 swCopy = swCoinDoorX :	   If toggleKeyCoinDoor = False Then .Switch(swCopy) = inverseKeyCoinDoor
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
