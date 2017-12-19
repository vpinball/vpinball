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
' IDSA  Data
'-------------------------
' Flipper Solenoid
Const GameOnSolenoid = 10
' Cabinet switches
'Const swTilt		=10       'ok V1
'Const swBallRollTilt	=10 'ok V1
Const swStartButton	=7   'ok
'Const swSlamTilt	=10     'ok V1
Const swCoin1		=4        'ok
Const swCoin2		=5        'ok
Const swCoin3		=6        'ok

Const swLRFlip		=82     'ok
Const swLLFlip		=84     'ok

' Help Window
vpmSystemHelp="IDSA Keys:"&vbNewLine&_
  vpmKeyName(keyInsertCoin1)  & vbTab & "Insert Coin #1" & vbNewLine &_
  vpmKeyName(keyInsertCoin2)  & vbTab & "Insert Coin #2" & vbNewLine &_
  vpmKeyName(keyInsertCoin3)  & vbTab & "Insert Coin #3" & vbNewLine &_
  vpmKeyName(keySlamDoorHit)  & vbTab & "Slam Tilt"

' Dip Switch / Options Menu
Private Sub IDSADips
	If Not IsObject(vpmDips) Then ' First time
		Set vpmDips=New cvpmDips
		With vpmDips
				.AddForm 150, 245, "DIP Switches"
			.AddFrame  0,0, 60, "", 0,_
			  Array("DIP  1",&H00000001,"DIP  2",&H00000002,"DIP  3",&H00000004,"DIP  4",&H00000008,_
			        "DIP  5",&H00000010,"DIP  6",&H00000020,"DIP  7",&H00000040,"DIP  8",&H00000080,_
			        "DIP  9",&H00000100,"DIP 10",&H00000200,"DIP 11",&H00000400,"DIP 12",&H00000800,_
			        "DIP 13",&H00001000,"DIP 14",&H00002000,"DIP 15",&H00004000,"DIP 16",32768)
		End With
	End If
	vpmDips.ViewDips
End Sub
Set vpmShowDips=GetRef("IDSADips")
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
			Case keyInsertCoin1  vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin1'":Playsound SCoin
			Case keyInsertCoin2  vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin2'":Playsound SCoin
			Case keyInsertCoin3  vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin3'":Playsound SCoin
			Case StartGameKey    .Switch(swStartButton) =True
'			Case keySlamDoorHit  .Switch(swSlamTilt)    =True
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
'			Case keySlamDoorHit  .Switch(swSlamTilt)    = False
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
	On Error Goto 0
End Function
