'Last Updated in VBS v3.36
' - 3.19 fixes the swLRFlip and swLLFlip switch numbers so the standard SolFlipper function can be used
'
'First Added in VBS v3.18
'
'This is for Atari Generation 1 games with Rotary switch 

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
' Atari Data
'-------------------------
' Cabinet switches
Const swSelfTest       =  0
Const swStartButton    =  3
Const swCoin1          =  1
Const swCoin2          =  2
Const swCPUDiag        =  7
Const swTilt           = 18
Const swSlamDoorHit    =  4
Const swLRFlip         = 82
Const swLLFlip         = 84

' Help Window
vpmSystemHelp = "Atari keys:" & vbNewLine &_
  vpmKeyName(keyInsertCoin1) & vbTab & "Insert Coin #1"   & vbNewLine &_
  vpmKeyName(keyInsertCoin2) & vbTab & "Insert Coin #2"   & vbNewLine &_
  vpmKeyName(keySlamDoorHit) & vbTab & "Slam Tilt" 	  & vbNewLine &_
  vpmKeyName(keySelfTest)    & vbTab & "Self Test"

' Dip Switch / Options Menu
Private Sub atariShowDips
	If Not IsObject(vpmDips) Then ' First time
		Set vpmDips = New cvpmDips
		With vpmDips
			.AddForm 200,300,"Atari generation 1 switches"
			.AddFrame 0,0,80,"DIP switches",0,_
			  Array("SW1-1",&H00000800,"SW1-2",&H00000400,"SW1-3",&H00000200,"SW1-4",&H00000100,_
			        "SW1-5",32768,"SW1-6",&H00004000,"SW1-7",&H00002000,"SW1-8",&H00001000,_
				"SW2-1",&H00000008,"SW2-2",&H00000004,"SW2-3",&H00000002,"SW2-4",&H00000001,_
			        "SW2-5",&H00000080,"SW2-6",&H00000040,"SW2-7",&H00000020,"SW2-8",&H00000010)
			.AddFrame 100,0,80,"Rotary switch",&H000F0000,_
			  Array("setting 0",0,"setting 1",&H00010000,"setting 2",&H00020000,"setting 3",&H00030000,_
			        "setting 4",&H00040000,"setting 5",&H00050000,"setting 6",&H00060000,"setting 7",&H00070000,_
			        "setting 8",&H00080000,"setting 9",&H00090000,"setting 10",&H000A0000,"setting 11",&H000B0000,_
			        "setting 12",&H000C0000,"setting 13",&H000D0000,"setting 14",&H000E0000,"setting 15",&H000F0000)
		End With
	End If
	vpmDips.ViewDips
End Sub
Set vpmShowDips = GetRef("atariShowDips")
Private vpmDips

' Keyboard handlers
Function vpmKeyDown(ByVal keycode)
	On Error Resume Next
	vpmKeyDown = True ' Assume we handle the key
	With Controller
		If keycode = RightFlipperKey Then .Switch(swLRFlip) = True
		If keycode = LeftFlipperKey  Then .Switch(swLLFlip) = True
		Select Case keycode
			Case keyInsertCoin1  vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin1'" : Playsound SCoin
			Case keyInsertCoin3  vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin2'" : Playsound SCoin
			Case StartGameKey    .Switch(swStartButton) = True
			Case keySelfTest     .Switch(swSelfTest)    = True
			Case keyCPUDiag      .Switch(swCPUDiag)     = True
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
		If keycode = RightFlipperKey Then .Switch(swLRFlip) = False
		If keycode = LeftFlipperKey  Then .Switch(swLLFlip) = False
		Select Case keycode
			Case StartGameKey    .Switch(swStartButton) = False
			Case keySelfTest     .Switch(swSelfTest)    = False
			Case keyCPUDiag      .Switch(swCPUDiag)     = False
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
End Function
