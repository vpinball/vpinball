'Last Updated in VBS v3.54

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

'----------------------------
' Stern S.A.M. Data
'----------------------------
' Cabinet switches
Const swCancel         = -3
Const swDown           = -2
Const swUp             = -1
Const swEnter          =  0
Const swStartButton    = 16
Const swTilt           = -7
Const swSlamTilt       = -6
Const swCoin1          = 65
Const swCoin2          = 66
Const swCoin3          = 67
Const swLRFlip         = 82
Const swLLFlip         = 84

' Help Window
vpmSystemHelp = "Stern S.A.M. keys:" & vbNewLine &_
  vpmKeyName(keyInsertCoin1)  & vbTab & "Insert Coin #1"   & vbNewLine &_
  vpmKeyName(keyInsertCoin2)  & vbTab & "Insert Coin #2"   & vbNewLine &_
  vpmKeyName(keyInsertCoin3)  & vbTab & "Insert Coin #3"   & vbNewLine &_
  vpmKeyName(keyCancel)       & vbTab & "Back"            & vbNewLine &_
  vpmKeyName(keyDown)         & vbTab & "Minus"            & vbNewLine &_
  vpmKeyName(keyUp)           & vbTab & "Plus"              & vbNewLine &_
  vpmKeyName(keyEnter)        & vbTab & "Select"              & vbNewLine &_
  vpmKeyName(keySlamDoorHit)  & vbTab & "Slam Tilt"

'Dip Switch / Options Menu
Private Sub segaShowDips
	If Not IsObject(vpmDips) Then ' First time
		Set vpmDips = New cvpmDips
		With vpmDips
			.AddForm 100, 240, "DIP Switches"
			.AddFrame 0, 0, 80, "Country", &H0f,_
			  Array("Austria",&H01, "Belgium", &H02, "Brazil",      &H0d,_
			        "Canada", &H03, "France",  &H06, "Germany",     &H07,_
			        "Italy",  &H08, "Japan",   &H09, "Netherlands", &H04,_
			        "Norway", &H0a, "Sweden",  &H0b, "Switzerland", &H0c,_
			        "UK",     &H05, "UK (New)",&H0e, "USA",         &H00)
		End With
	End If
	vpmDips.ViewDips
End Sub
Set vpmShowDips = GetRef("segaShowDips")
Private vpmDips
' Keyboard handlers
Function vpmKeyDown(ByVal keycode)
	On Error Resume Next
	vpmKeyDown = True ' Assume we handle the key
	With Controller
		Select Case keycode
			Case RightFlipperKey .Switch(swLRFlip) = True : vpmKeyDown = False
			Case LeftFlipperKey  .Switch(swLLFlip) = True : vpmKeyDown = False
			Case keyInsertCoin1  vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin1'" : Playsound SCoin
			Case keyInsertCoin2  vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin2'" : Playsound SCoin
			Case keyInsertCoin3  vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin3'" : Playsound SCoin
			Case StartGameKey    .Switch(swStartButton)  = True
			Case keyCancel       .Switch(swCancel)       = True
			Case keyDown         .Switch(swDown)         = True
			Case keyUp           .Switch(swUp)           = True
			Case keyEnter        .Switch(swEnter)        = True
			Case keySlamDoorHit  .Switch(swSlamTilt)     = True
			Case keyBangBack     vpmNudge.DoNudge 0, 6
			Case LeftTiltKey     vpmNudge.DoNudge 75, 2
			Case RightTiltKey    vpmNudge.DoNudge 285, 2
			Case CenterTiltKey   vpmNudge.DoNudge 0, 2
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
			Case RightFlipperKey .Switch(swLRFlip) = False : vpmKeyUp = False
			Case LeftFlipperKey  .Switch(swLLFlip) = False : vpmKeyUp = False
			Case StartGameKey    .Switch(swStartButton)  = False
			Case keyCancel       .Switch(swCancel)       = False
			Case keyDown         .Switch(swDown)         = False
			Case keyUp           .Switch(swUp)           = False
			Case keyEnter        .Switch(swEnter)        = False
			Case keySlamDoorHit  .Switch(swSlamTilt)     = False
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
