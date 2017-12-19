'Last Updated in VBS v3.36

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
' Allied Leisure Data
'-------------------------
' Flipper Solenoid
'Const GameOnSolenoid = 19?
' Cabinet switches
Const swTilt		=31
Const swBallRollTilt	=31
Const swStartButton	=32
Const swSlamTilt	=33
Const swCoin1		=34
Const swCoin2		=35
Const swCoin3		=36
Const swCPUDiag		=37
Const swLRFlip		=82
Const swLLFlip		=84

' Help Window
vpmSystemHelp="Allied Leisure Keys:"&vbNewLine&_
  vpmKeyName(keyInsertCoin1)  & vbTab & "Insert Coin #1" & vbNewLine &_
  vpmKeyName(keyInsertCoin2)  & vbTab & "Insert Coin #2" & vbNewLine &_
  vpmKeyName(keyInsertCoin3)  & vbTab & "Insert Coin #3" & vbNewLine &_
  vpmKeyName(keySlamDoorHit)  & vbTab & "Slam Tilt"      & vbNewLine &_
  vpmKeyName(keyCPUDiag)      & vbTab & "Cpu Diagnostic"

' Dip Switch / Options Menu
Private Sub ALIDips
	If Not IsObject(vpmDips) Then ' First time
		Set vpmDips=New cvpmDips
		With vpmDips
			.AddForm  80, 60, "DIP Switches"
			.AddFrame  0,0, 60,"Sound", 0,_
		       Array("DIP 1",&H000000001,"DIP 2",&H00000002)
			.AddFrame  0,45, 60,"Game", 0,_
		       Array("DIP M8",&H00000100,"DIP M7",&H00000200,"DIP M6",&H00000400,"DIP M5",&H00000800,_
			       "DIP M4",&H00001000,"DIP M3",&H00002000,"DIP M2",&H00004000,"DIP M1",32768,_
			       "DIP D8",&H00010000,"DIP D7",&H00020000,"DIP D6",&H00040000,"DIP D5",&H00080000,_
			       "DIP D4",&H00100000,"DIP D3",&H00200000,"DIP D2",&H00400000,"DIP D1",&H00800000)
		End With
	End If
	vpmDips.ViewDips
End Sub
Set vpmShowDips=GetRef("ALIDips")
Private vpmDips

' Keyboard Handlers
Function vpmKeyDown(ByVal KeyCode)
	On Error Resume Next
	vpmKeyDown=True ' Assume we handle the key
	With Controller
		Select Case KeyCode
			Case LeftFlipperKey  .Switch(swLLFlip) = True : vpmKeyDown = False :  vpmFlips.FlipL True : if keycode = keyStagedFlipperL then vpmFlips.FlipUL True
			Case RightFlipperKey .Switch(swLRFlip) = True : vpmKeyDown = False :  vpmFlips.FlipR True : if keycode = keyStagedFlipperR then vpmFlips.FlipUR True
			Case keyStagedFlipperL vpmFlips.FlipUL True
			Case keyStagedFlipperR vpmFlips.FlipUR True
			Case keyInsertCoin1  vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin1'" : Playsound SCoin
			Case keyInsertCoin2  vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin2'" : Playsound SCoin
			Case keyInsertCoin3  vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin3'" : Playsound SCoin
			Case StartGameKey    .Switch(swStartButton) = True
			Case keyCPUDiag      .Switch(swCPUDiag)     = True
			Case keySlamDoorHit  .Switch(swSlamTilt)    = True
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

Function vpmKeyUp(ByVal KeyCode)
	On Error Resume Next
	vpmKeyUp=True ' Assume we handle the key
	With Controller
		Select Case KeyCode
			Case LeftFlipperKey  .Switch(swLLFlip) = False : vpmKeyUp = False :  vpmFlips.FlipL False : if keycode = keyStagedFlipperL then vpmFlips.FlipUL False
			Case RightFlipperKey .Switch(swLRFlip) = False : vpmKeyUp = False :  vpmFlips.FlipR False : if keycode = keyStagedFlipperR then vpmFlips.FlipUR False
			Case keyStagedFlipperL vpmFlips.FlipUL False
			Case keyStagedFlipperR vpmFlips.FlipUR False
			Case StartGameKey    .Switch(swStartButton) = False
			Case keyCPUDiag      .Switch(swCPUDiag)     = False
			Case keySlamDoorHit  .Switch(swSlamTilt)    = False
			Case keyShowOpts     .Pause = True : .ShowOptsDialog GetPlayerHWnd : .Pause = False
			Case keyShowKeys     .Pause = True : vpmShowHelp : .Pause = False
			Case keyAddBall      .Pause = True : vpmAddBall  : .Pause = False
			Case keyShowDips     If IsObject(vpmShowDips) Then .Pause = True : vpmShowDips : .Pause = False
			Case keyReset        .Stop : BeginModal : .Run : vpmTimer.Reset : EndModal
			Case keyFrame        .LockDisplay = Not .LockDisplay
			Case keyDoubleSize   .DoubleSize =  Not .DoubleSize
			Case Else            vpmKeyUp = False
		End Select
	End With
	On Error Goto 0
End Function
