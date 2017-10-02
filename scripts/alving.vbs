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
' Alvin G Data
'-------------------------
Const swCoin1       =  1
Const swCoin2       =  2
Const swCoin3       =  3
Const swCoin4       =  4
Const swSlamTilt    =  5
Const swDiagnostic  = -7
Const swExitDiag    = -6
Const swVolUp	  = 15
Const swVolDn	  = 16
Const swStartButton =  9

Const swLRFlip = 114
Const swLLFlip = 116

' Help window
vpmSystemHelp = "Alvin G keys" & vbNewLine &_
  vpmKeyName(keyInsertCoin1)  & vbTab & "Insert Coin #1" & vbNewLine &_
  vpmKeyName(keyInsertCoin2)  & vbTab & "Insert Coin #2" & vbNewLine &_
  vpmKeyName(keyInsertCoin3)  & vbTab & "Insert Coin #3" & vbNewLine &_
  vpmKeyName(keyInsertCoin4)  & vbTab & "Insert Coin #4" & vbNewLine &_
  vpmKeyName(keyCancel)       & vbTab & "Enter"			 & vbNewLine &_
  vpmKeyName(keyDown)         & vbTab & "Exit"			 & vbNewLine &_
  vpmKeyName(keyUp)			  & vbTab & "Volume +"		 & vbNewLine &_
  vpmKeyName(keyEnter)        & vbTab & "Volume -"		 & vbNewLine &_
  vpmKeyName(keySlamDoorHit)  & vbTab & "Slam Tilt"

'Option Menu (No Dips)
Private Sub AlvinGShowDips
	If Not IsObject(vpmDips) Then ' First time
		Set vpmDips = New cvpmDips
		With vpmDips
	  	.AddForm  80, 0, "Option Menu"
		.AddLabel 0,0,250,20, "No Options In This Table At This Time"
		End With
	End If
	vpmDips.ViewDips
End Sub
Set vpmShowDips = GetRef("AlvinGShowDips")
Private vpmDips


' Keyboard handlers
Function vpmKeyDown(ByVal keycode)
	On Error Resume Next ' SCoin
	vpmKeyDown = True ' assume we handle the key
	With Controller
		Select Case keycode
			Case RightFlipperKey .Switch(swLRFlip) = True : vpmKeyDown = False
			Case LeftFlipperKey  .Switch(swLLFlip) = True : vpmKeyDown = False
			Case keyInsertCoin1  vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin1'" : Playsound SCoin
			Case keyInsertCoin2  vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin2'" : Playsound SCoin
			Case keyInsertCoin3  vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin3'" : Playsound SCoin
			Case keyInsertCoin4  vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin4'" : Playsound SCoin
			Case StartGameKey    .Switch(swStartButton) = True
			Case keyCancel       .Switch(swDiagnostic)  = True
			Case keyDown         .Switch(swExitDiag)    = True
			Case keyUp           .Switch(swVolUp)       = True
			Case keyEnter        .Switch(swVolDn)       = True									
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

Function vpmKeyUp(ByVal keycode)
	On Error Resume Next
	vpmKeyUp = True ' assume we handle the key
	With Controller
		Select Case keycode
			Case RightFlipperKey .Switch(swLRFlip) = False : vpmKeyUp = False
			Case LeftFlipperKey  .Switch(swLLFlip) = False : vpmKeyUp = False
			Case StartGameKey    .Switch(swStartButton) = False
			Case keyCancel       .Switch(swDiagnostic)  = False
			Case keyDown         .Switch(swExitDiag)    = False
			Case keyUp           .Switch(swVolUp)       = False
			Case keyEnter        .Switch(swVolDn)       = False									
			Case keySlamDoorHit  .Switch(swSlamTilt)    = False
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
