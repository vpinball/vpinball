'Last Updated in VBS v3.36
'
'First Added in VBS v3.20

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
' Peyper / Sonic Data
'-------------------------
' Flipper Solenoid
Const GameOnSolenoid = 12
' Cabinet switches
Const swStartButton    = 48
Const swCoin1          = 2
Const swCoin2          = 3
Const swCoin3          = 4
Const swTilt           = 31
Const swSlamDoorHit    = 30
Const swLRFlip         = 82
Const swLLFlip         = 84

' Help Window
vpmSystemHelp = "Atari keys:" & vbNewLine &_
  vpmKeyName(keyInsertCoin1) & vbTab & "Insert Coin #1"   & vbNewLine &_
  vpmKeyName(keyInsertCoin2) & vbTab & "Insert Coin #2"   & vbNewLine &_
  vpmKeyName(keyInsertCoin3) & vbTab & "Insert Coin #3"   & vbNewLine &_
  vpmKeyName(keySlamDoorHit) & vbTab & "Slam Tilt"

' Dip Switch / Options Menu
Private Sub juegosShowDips
	If Not IsObject(vpmDips) Then ' First time
		Set vpmDips = New cvpmDips
		With vpmDips
			.AddForm 200,300,"Juegos Populares switches"
			.AddFrame 0,0,80,"DIP switches",0,_
			  Array("Dip 0",&H00000800,"Dip 1",&H00000400,"Dip 2",&H00000200,"Dip 3",&H00000100,_
			        "Dip 4",32768,     "Dip 5",&H00004000,"Dip 6",&H00002000,"Dip 7",&H00001000,_
				"Dip 8",&H00000008,"Dip 9",&H00000004,"Dip10",&H00000002,"Dip11",&H00000001,_
			        "Dip12",&H00000080,"Dip13",&H00000040,"Dip14",&H00000020,"Dip15",&H00000010)
		End With
	End If
	vpmDips.ViewDips
End Sub
Set vpmShowDips = GetRef("juegosShowDips")
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
			Case keyInsertCoin2  vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin2'" : Playsound SCoin
			Case keyInsertCoin3  vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin3'" : Playsound SCoin
			Case StartGameKey    .Switch(swStartButton) = True
			Case keySlamDoorHit  .Switch(swSlamDoorHit) = True
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
			Case keySlamDoorHit  .Switch(swSlamDoorHit) = False
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
