'Last Updated in VBS v3.50
'Similar to Play4.vbs, except for swSelfTest switch missing

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
' Stargame Iron Balls
'-------------------------
' Cabinet switches
Const swStartButton	    = 7 'ok
Const swCoin1     	    = 1 'ok
Const swCoin2	        = 2 'ok
Const swCoin3	        = 3 'ok
Const swTilt	        = 4 'ok
Const swBallRollTilt    = 4 'ok
'Const swSlamTilt       = -4 'ok

Const swLRFlip	        = 102'ok
Const swLLFlip    	    = 104'ok

' Help Window
vpmSystemHelp="Iron Balls Keys:"&vbNewLine&_
  vpmKeyName(keyInsertCoin1)  & vbTab & "Insert Coin #1" & vbNewLine &_
  vpmKeyName(keyInsertCoin2)  & vbTab & "Insert Coin #2" & vbNewLine &_
  vpmKeyName(keyInsertCoin3)  & vbTab & "Insert Coin #3"
'  vpmKeyName(keySlamDoorHit)  & vbTab & "Slam Tilt"

' Keyboard Handlers
Function vpmKeyDown(ByVal KeyCode)
	On Error Resume Next
	vpmKeyDown=True ' Assume we handle the key
	With Controller
		Select Case KeyCode
			Case RightFlipperKey .Switch(swLRFlip) = True : vpmKeyDown = False
			Case LeftFlipperKey  .Switch(swLLFlip) = True : vpmKeyDown = False
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
			Case RightFlipperKey .Switch(swLRFlip) = False : vpmKeyUp = False
			Case LeftFlipperKey  .Switch(swLLFlip) = False : vpmKeyUp = False
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
