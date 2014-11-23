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

'-----------------------------------
' S4 data
'-----------------------------------
' Cabinet switches
Const swAdvance      = -7
Const swUpDown       = -6
Const swCPUDiag      = -5
Const swSoundDiag	   = -4 ' Seems to start sounds anyway when activated
Const swEnter	   = -3 ' Incorrect & Unknown Currently
Const swTilt         =  1
Const swBallRollTilt =  2
Const swStartButton  =  3
Const swCoin3        =  4
Const swCoin2        =  5
Const swCoin1        =  6
Const swSlamTilt     =  7
Const swHiScoreReset =  8

Const swLRFlip       = 82
Const swLLFlip       = 84

' Help Window
vpmSystemHelp = "Williams System 3/4 keys:" & vbNewLine &_
  vpmKeyName(keyInsertCoin1)  & vbTab & "Insert Coin #1" & vbNewLine &_
  vpmKeyName(keyInsertCoin2)  & vbTab & "Insert Coin #2" & vbNewLine &_
  vpmKeyName(keyInsertCoin3)  & vbTab & "Insert Coin #3" & vbNewLine &_
  vpmKeyName(keyHiscoreReset) & vbTab & "Hiscore Reset"  & vbNewLine &_
  vpmKeyName(keyAdvance)      & vbTab & "Advance"        & vbNewLine &_
  vpmKeyName(keyUpDown)       & vbTab & "Up/Down"        & vbNewLine &_
  vpmKeyName(keyCPUDiag)      & vbTab & "Cpu Diagnostic" & vbNewLine &_
  vpmKeyName(keySlamDoorHit)  & vbTab & "Slam Tilt"

' Dip Switch / Options Menu
Private Sub s4ShowDips
	If Not IsObject(vpmDips) Then ' First time
		Set vpmDips = New cvpmDips
		With vpmDips
			.AddForm  80, 290, "DIP Switches"
			.AddFrame  0,0, 100,"Sound", 0,_
			  Array("Dip 1",&H000000001,"Synth Sound",&H00000002)
			.AddFrame  0,50, 100,"Function / Data", 0,_
				    Array("DIP F8 (1)" ,&H00000100,"DIP F7 (2)",&H00000200,"DIP F6 (4)",&H00000400,"DIP F5 (8)",&H00000800,_
	    			      "DIP F4 (16)",&H00001000,"DIP F3",&H00002000,"DIP F2",&H00004000,"DIP F1",32768,_
	    			      "DIP D8",&H00010000,"DIP D7",&H00020000,"DIP D6",&H00040000,"DIP D5",&H00080000,_
	    			      "DIP D4",&H00100000,"DIP D3",&H00200000,"DIP D2",&H00400000,"DIP D1",&H00800000)
		End With
	End If
	vpmDips.ViewDips
End Sub
Set vpmShowDips = GetRef("s4ShowDips")
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
			Case keyInsertCoin2  vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin2'" : Playsound SCoin
			Case keyInsertCoin3  vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin3'" : Playsound SCoin
			Case StartGameKey    .Switch(swStartButton)  = True
			Case keyUpDown       .Switch(swUpDown)       = Not .Switch(swUpDown)
			Case keyAdvance      .Switch(swAdvance)      = True
			Case keyCPUDiag      .Switch(swCPUDiag)      = True
			Case keySlamDoorHit  .Switch(swSlamTilt)     = True
			Case keyHiscoreReset .Switch(swHiScoreReset) = True
			Case keyEnter	   .Switch(swEnter)        = True
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
			Case StartGameKey    .Switch(swStartButton)  = False
			Case keyAdvance      .Switch(swAdvance)      = False
			Case keyCPUDiag      .Switch(swCPUDiag)      = False
			Case keySlamDoorHit  .Switch(swSlamTilt)     = False
			Case keyHiscoreReset .Switch(swHiScoreReset) = False
			Case keyEnter	   .Switch(swEnter)        = False
			Case keyShowOpts     .Pause = True : .ShowOptsDialog GetPlayerHWnd : .Pause = False
			Case keyShowKeys     .Pause = True : vpmShowHelp : .Pause = False
			Case keyAddBall      .Pause = True : vpmAddBall  : .Pause = False
			Case keyReset        .Stop : BeginModal : .Run : vpmTimer.Reset : EndModal
			Case keyFrame        .LockDisplay = Not .LockDisplay
			Case keyDoubleSize   .DoubleSize  = Not .DoubleSize
			Case keyShowDips     If IsObject(vpmShowDips) Then .Pause = True : vpmShowDips : .Pause = False
			Case Else            vpmKeyUp = False
		End Select
	End With
End Function

