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

' Cabinet switches
Const swCoin1  = 0
Const swCoin2  = 1
Const swCoin3  = 2
Const swCoin4  = 3
Const swCancel = 4
Const swDown   = 5
Const swUp     = 6
Const swEnter  = 7
Const swLLFlip = 9
Const swLRFlip = 11
Const swULFlip = 13
Const swURFlip = 15

Private swStartButtonX,swCoinDoorX,swSlamTiltX
On Error Resume Next
If swStartButton = 34 Or Err Then swStartButtonX = 34 Else swStartButtonX = swStartButton
If swCoinDoor    = 22 Or Err Then swCoinDoorX    = 22 Else swCoinDoorX    = swCoinDoor
If swSlamTilt    = 21 Or Err Then swSlamTiltX    = 21 Else swSlamTiltX    = swSlamTilt
On Error Goto 0


' Help window
vpmSystemHelp = "P-ROC PDb Keys:" & vbNewLine &_
  vpmKeyName(keyInsertCoin1)  & vbTab & "Insert Coin #1" & vbNewLine &_
  vpmKeyName(keyInsertCoin2)  & vbTab & "Insert Coin #2" & vbNewLine &_
  vpmKeyName(keyInsertCoin3)  & vbTab & "Insert Coin #3" & vbNewLine &_
  vpmKeyName(keyInsertCoin4)  & vbTab & "Insert Coin #4" & vbNewLine &_
  vpmKeyName(keyCancel) & vbTab & "Escape (Coin Door)" & vbNewLine &_
  vpmKeyName(keyDown)  & vbTab & "Down (Coin Door)" & vbNewLine &_
  vpmKeyName(keyUp)  & vbTab & "Up (Coin Door)" & vbNewLine &_
  vpmKeyName(keyEnter)  & vbTab & "Enter (Coin Door)" & vbNewLine &_
  vpmKeyName(keySlamDoorHit) & vbTab & "Slam Tilt" & vbNewLine &_
  vpmKeyName(keyCoinDoor) & vbTab & "Open/Close Coin Door"

' Keyboard handlers
Function vpmKeyDown(ByVal keycode)
	Dim swCopy
	On Error Resume Next
	vpmKeyDown = True ' assume we handle the key
	With Controller
		Select Case keycode
			Case LeftFlipperKey
				.Switch(swLLFlip) = True : vpmKeyDown = False : vpmFlips.FlipL True
			Case RightFlipperKey
				.Switch(swLRFlip) = True : vpmKeyDown = False : vpmFlips.FlipR True				
			Case keyInsertCoin1  vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin1'" : Playsound SCoin
			Case keyInsertCoin2  vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin2'" : Playsound SCoin
			Case keyInsertCoin3  vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin3'" : Playsound SCoin
			Case keyInsertCoin4  vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin4'" : Playsound SCoin
			Case StartGameKey    swCopy = swStartButtonX : .Switch(swCopy) = True
			Case keyCancel       swCopy = swCancel :       .Switch(swCopy) = True
			Case keyDown         swCopy = swDown :         .Switch(swCopy) = True
			Case keyUp           swCopy = swUp :           .Switch(swCopy) = True
			Case keyEnter        swCopy = swEnter :        .Switch(swCopy) = True
			Case keySlamDoorHit  swCopy = swSlamTiltX :    .Switch(swCopy) = True
			Case keyCoinDoor     swCopy = swCoinDoorX :    If toggleKeyCoinDoor Then .Switch(swCopy) = Not .Switch(swCopy) Else .Switch(swCopy) = Not inverseKeyCoinDoor
			Case keyBangBack     vpmNudge.DoNudge   0,6
			Case LeftTiltKey     vpmNudge.DoNudge  75,2
			Case RightTiltKey    vpmNudge.DoNudge 285,2
			Case CenterTiltKey   vpmNudge.DoNudge   0,2
			Case keyVPMVolume    vpmVol
			Case Else            vpmKeyDown = False
		End Select
	End With
	On Error Goto 0
End Function

Function vpmKeyUp(ByVal keycode)
	Dim swCopy
	On Error Resume Next
	vpmKeyUp = True ' assume we handle the key
	With Controller
		Select Case keycode
			Case LeftFlipperKey
				.Switch(swLLFlip) = False : vpmKeyUp = False : vpmFlips.FlipL False
				If keycode = keyStagedFlipperL Then ' as vbs will not evaluate the Case keyStagedFlipperL then, also handle it here
					vpmFlips.FlipUL False
					If cSingleLFlip Or Err Then .Switch(swULFlip) = False
				End If
			Case RightFlipperKey
				.Switch(swLRFlip) = False : vpmKeyUp = False : vpmFlips.FlipR False
				If keycode = keyStagedFlipperR Then ' as vbs will not evaluate the Case keyStagedFlipperR then, also handle it here
					vpmFlips.FlipUR False
					If cSingleRFlip Or Err Then .Switch(swURFlip) = False
				End If
			Case keyStagedFlipperL vpmFlips.FlipUL False : If cSingleLFlip Or Err Then .Switch(swULFlip) = False
			Case keyStagedFlipperR vpmFlips.FlipUR False : If cSingleRFlip Or Err Then .Switch(swURFlip) = False
			Case keyCancel       swCopy = swCancel :       .Switch(swCopy) = False
			Case keyDown         swCopy = swDown :         .Switch(swCopy) = False
			Case keyUp           swCopy = swUp :           .Switch(swCopy) = False
			Case keyEnter        swCopy = swEnter :        .Switch(swCopy) = False
			Case keySlamDoorHit  swCopy = swSlamTiltX :    .Switch(swCopy) = False
			Case StartGameKey    swCopy = swStartButtonX : .Switch(swCopy) = False
			Case keyCoinDoor     swCopy = swCoinDoorX :    If toggleKeyCoinDoor = False Then .Switch(swCopy) = inverseKeyCoinDoor
			Case keyShowOpts     .Pause = True : .ShowOptsDialog GetPlayerHWnd : .Pause = False
			Case keyShowKeys     .Pause = True : vpmShowHelp : .Pause = False
			Case keyShowDips     If IsObject(vpmShowDips) Then .Pause = True : vpmShowDips : .Pause = False
			Case keyAddBall      .Pause = True : vpmAddBall  : .Pause = False
			Case keyReset        .Stop : BeginModal : .Run : vpmTimer.Reset : EndModal
			Case keyFrame        .LockDisplay = Not .LockDisplay
			Case keyDoubleSize   .DoubleSize  = Not .DoubleSize
			Case Else            vpmKeyUp = False
		End Select
	End With
	On Error Goto 0
End Function
