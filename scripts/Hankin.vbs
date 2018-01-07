'Last Updated in VBS v3.56

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
' Hankin Data
'-------------------------
' Flipper Solenoid
Const GameOnSolenoid = 19
' Cabinet switches
Const swSelfTest       = -7
Const swCPUDiag        = -4
Const swSoundDiag      = -3
Const swTilt           =  2
Const swBallRollTilt   =  2
Const swSlamTilt       =  9
Const swCoin1          = 16
Const swStartButton    =  3

Const swLRFlip         = 82
Const swLLFlip         = 84

' Help Window
vpmSystemHelp = "Hankin keys:" & vbNewLine &_
  vpmKeyName(keyInsertCoin1) & vbTab & "Insert Coin #1"   & vbNewLine &_
  vpmKeyName(keySelfTest)    & vbTab & "Self Test"        & vbNewLine &_
  vpmKeyName(keyCPUDiag)     & vbTab & "Cpu Diagnostic"   & vbNewLine &_
  vpmKeyName(keySoundDiag)   & vbTab & "Sound Diagnostic" & vbNewLine &_
  vpmKeyName(keySlamDoorHit) & vbTab & "Slam Tilt"

' Dips Switch / Options Menu
Private Sub HankinShowDips
	If Not IsObject(vpmDips) Then ' First time
		Set vpmDips = New cvpmDips
		With vpmDips
			.AddForm 200, 245, "DIP Switches"
			.AddFrame 0, 0, 95, "Credit/Coins", &H07,_
			  Array("1/1",&H00,"1/2",&H01,"1/3",&H02,"1/4",&H03,_
				"1/1, 3/2",&H04,"1/2, 2/3",&H05,"2/3, 4/5",&H06,"2/5",&H07)
			.AddFrame 0, 132, 95, "High Score Award", &H60,_
			  Array("No award",&H00,"1 free game",&H20,"2 free games",&H40,"3 free games",&H60)
			.AddFrame 0, 209, 95, "Maximum Credits", &H60000,_
			  Array("5",&H000000,"10",&H20000,"15",&H40000,"20",&H60000)

			.AddFrame 110, 0, 190, "Misc",0,_
			  Array("High score: Free Games/Extra Ball",&H00000008,_
			        "Match Feature",&H00000010,_
				"Game Over Tune",&H00000080,_
			        "Coin Alarm Sound",&H00000100,_
				"'4' Target memory",&H00000200,_
				"Background Sound",&H00000400,_
				"Coin Alarm",&H00004000,_
				"3/5 Balls",32768,_
				"Free Game Sound",&H00200000,_
				"Coin Counter Reset",&H00400000,_
				"Time Out Feature",&H00800000)
		End With
	End If
	vpmDips.ViewDips
End Sub
Set vpmShowDips = GetRef("HankinShowDips")
Private vpmDips

' Keyboard handlers
Function vpmKeyDown(ByVal keycode)
	On Error Resume Next
	vpmKeyDown = True ' Assume we handle the key
	With Controller
		Select Case keycode
			Case LeftFlipperKey  .Switch(swLLFlip) = True : vpmKeyDown = False :  vpmFlips.FlipL True : if keycode = keyStagedFlipperL then vpmFlips.FlipUL True
			Case RightFlipperKey .Switch(swLRFlip) = True : vpmKeyDown = False :  vpmFlips.FlipR True : if keycode = keyStagedFlipperR then vpmFlips.FlipUR True
			Case keyStagedFlipperL vpmFlips.FlipUL True
			Case keyStagedFlipperR vpmFlips.FlipUR True
			Case keyInsertCoin1  vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin1'" : Playsound SCoin
			Case StartGameKey    .Switch(swStartButton) = True
			Case keySelfTest     .Switch(swSelfTest)    = True
			Case keyCPUDiag      .Switch(swCPUDiag)     = True
			Case keySoundDiag    .Switch(swSoundDiag)   = True
			Case keySlamDoorHit  .Switch(swSlamTilt)    = True
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
		Select Case keycode
			Case LeftFlipperKey  .Switch(swLLFlip) = False : vpmKeyUp = False :  vpmFlips.FlipL False : if keycode = keyStagedFlipperL then vpmFlips.FlipUL False
			Case RightFlipperKey .Switch(swLRFlip) = False : vpmKeyUp = False :  vpmFlips.FlipR False : if keycode = keyStagedFlipperR then vpmFlips.FlipUR False
			Case keyStagedFlipperL vpmFlips.FlipUL False
			Case keyStagedFlipperR vpmFlips.FlipUR False
			Case StartGameKey    .Switch(swStartButton) = False
			Case keySelfTest     .Switch(swSelfTest)    = False
			Case keyCPUDiag      .Switch(swCPUDiag)     = False
			Case keySoundDiag    .Switch(swSoundDiag)   = False
			Case keySlamDoorHit  .Switch(swSlamTilt)    = False
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
