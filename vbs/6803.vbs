'Last Updated in VBS V3.36

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
' Bally 6803 Data
'-------------------------
' Cabinet switches
Const swCPUDiag        = -7
Const swSoundDiag      = -6
Const swStartButton    =  6
Const swKP0            =  2
Const swKP1            = 11
Const swKP2            = 10
Const swKP3            =  9
Const swKP4            = 19
Const swKP5            = 18
Const swKP6            = 17
Const swKP7            = 27
Const swKP8            = 26
Const swKP9            = 25
Const swKPA            = 12
Const swKPB            = 20
Const swKPC            = 28
Const swKPEnter        =  1
Const swKPClear        =  3
Const swKPGame         =  4
Const swTilt           = 15
Const swSlamTilt       = 14
Const swCoin3          =  9
Const swCoin1          = 10
Const swCoin2          = 11

Const swLRFlip         = 82
Const swLLFlip         = 84

' Help Window
vpmSystemHelp = "Bally 6803 keys:" & vbNewLine &_
  vpmKeyName(keyInsertCoin1) & vbTab & "Insert Coin #1"   & vbNewLine &_
  vpmKeyName(keyInsertCoin2) & vbTab & "Insert Coin #2"   & vbNewLine &_
  vpmKeyName(keyInsertCoin3) & vbTab & "Insert Coin #3"   & vbNewLine &_
  vpmKeyName(keyCpuDiag)     & vbTab & "Test"             & vbNewLine &_
  vpmKeyName(keySoundDiag)   & vbTab & "Sound Diagnostic" & vbNewLine &_
  vpmKeyName(keyCoinDoor)    & vbTab & "Open/Close Coin Door" & vbNewLine &_
  "KP 0-9"                   & vbTab & "Keypad 0-9"       & vbNewLine &_
  "KP / * -"                 & vbTab & "Keypad ABC"       & vbNewLine &_
  "KP Enter"                 & vbTab & "Keypad Enter"     & vbNewLine &_
  vpmKeyName(keyCancel)      & vbTab & "Keypad Kbd/Clr"   & vbNewLine &_
  vpmKeyName(keyKPGame)      & vbTab & "Keypad Game"      & vbNewLine &_
  vpmKeyName(keySlamDoorHit) & vbTab & "Slam Tilt"

' Option / Dip Menu
Private Sub Bally6803ShowDips
	If Not IsObject(vpmDips) Then ' First time
		Set vpmDips = New cvpmDips
		With vpmDips
	  	.AddForm  80, 0, "Option Menu"
		.AddLabel 0,0,250,20,"No Options In This Table At This Time"
		End With
	End If
	vpmDips.ViewDips
End Sub
Set vpmShowDips = GetRef("Bally6803ShowDips")
Private vpmDips

' Keyboard handlers
Private vpmCoinDoor : vpmCoinDoor = False
Function vpmKeyDown(ByVal keycode)
	On Error Resume Next
	vpmKeyDown = True ' Assume we handle the key
	With Controller
		If keycode = RightFlipperKey Then .Switch(swLRFlip) = True
		If keycode = LeftFlipperKey  Then .Switch(swLLFlip) = True
		Select Case keycode
			Case keyInsertCoin1  vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin1'" : Playsound SCoin
			Case keyInsertCoin2  vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin2'" : Playsound SCoin
			Case keyInsertCoin3  vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin3'" : Playsound SCoin
			Case StartGameKey    .Switch(swStartButton) = True
			Case keySoundDiag    .Switch(swSoundDiag)   = True
			Case keySlamDoorHit  .Switch(swSlamTilt)    = True
			Case keyCoinDoor     If toggleKeyCoinDoor Then vpmCoinDoor = Not vpmCoinDoor Else vpmCoinDoor = Not inverseKeyCoinDoor
			Case keyCPUDiag      .Switch(swCPUDiag)     = vpmCoinDoor
			Case 82              .Switch(swKP0)         = vpmCoinDoor
			Case 79              .Switch(swKP1)         = vpmCoinDoor
			Case 80              .Switch(swKP2)         = vpmCoinDoor
			Case 81              .Switch(swKP3)         = vpmCoinDoor
			Case 75              .Switch(swKP4)         = vpmCoinDoor
			Case 76              .Switch(swKP5)         = vpmCoinDoor
			Case 77              .Switch(swKP6)         = vpmCoinDoor
			Case 71              .Switch(swKP7)         = vpmCoinDoor
			Case 72              .Switch(swKP8)         = vpmCoinDoor
			Case 73              .Switch(swKP9)         = vpmCoinDoor
			Case 181             .Switch(swKPA)         = vpmCoinDoor
			Case 55              .Switch(swKPB)         = vpmCoinDoor
			Case 74              .Switch(swKPC)         = vpmCoinDoor
			Case keyKPEnter      .Switch(swKPEnter)     = vpmCoinDoor
			Case keyCancel       .Switch(swKPClear)     = vpmCoinDoor
			Case keyKPGame       .Switch(swKPGame)      = vpmCoinDoor
			Case keyBangBack     vpmNudge.DoNudge   0, 6
			Case LeftTiltKey     vpmNudge.DoNudge  75, 2
			Case RightTiltKey    vpmNudge.DoNudge 285, 2
			Case CenterTiltKey   vpmNudge.DoNudge   0, 2
			Case keyVPMVolume    vpmVol
			Case Else            vpmKeyDown = False
		End Select
	End With
	On Error Goto 0
End Function

Function vpmKeyUp(ByVal keycode)
	On Error Resume Next
	vpmKeyUp = True ' Assume we handle the key
	With Controller
		If keycode = RightFlipperKey Then .Switch(swLRFlip) = False
		If keycode = LeftFlipperKey  Then .Switch(swLLFlip) = False
		Select Case keycode
			Case StartGameKey    .Switch(swStartButton) = False
			Case keyCPUDiag      .Switch(swCPUDiag)     = False
			Case keySoundDiag    .Switch(swSoundDiag)   = False
			Case keySlamDoorHit  .Switch(swSlamTilt)    = False
			Case keyCoinDoor     If toggleKeyCoinDoor = False Then vpmCoinDoor = inverseKeyCoinDoor
			Case 82              .Switch(swKP0)         = False
			Case 79              .Switch(swKP1)         = False
			Case 80              .Switch(swKP2)         = False
			Case 81              .Switch(swKP3)         = False
			Case 75              .Switch(swKP4)         = False
			Case 76              .Switch(swKP5)         = False
			Case 77              .Switch(swKP6)         = False
			Case 71              .Switch(swKP7)         = False
			Case 72              .Switch(swKP8)         = False
			Case 73              .Switch(swKP9)         = False
			Case 181             .Switch(swKPA)         = False
			Case 55              .Switch(swKPB)         = False
			Case 74              .Switch(swKPC)         = False
			Case keyKPEnter      .Switch(swKPEnter)     = False
			Case keyCancel       .Switch(swKPClear)     = False
			Case keyKPGame       .Switch(swKPGame)      = False
			Case keyShowOpts     .Pause = True : .ShowOptsDialog GetPlayerHWnd : .Pause = False
			Case keyShowKeys     .Pause = True : vpmShowHelp : .Pause = False
			Case keyAddBall      .Pause = True : vpmAddBall  : .Pause = False
			Case keyShowDips     If IsObject(vpmShowDips) Then .Pause = True : vpmShowDips : .Pause = False
			Case keyReset        .Stop : BeginModal : .Run : vpmTimer.Reset : EndModal
			Case keyFrame        .LockDisplay = Not .LockDisplay
			Case keyDoubleSize   .DoubleSize  = Not .DoubleSize
			Case Else            vpmKeyUp = False
		End Select
	End With
End Function

