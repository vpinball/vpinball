'Last Updated in VBS v3.36

'Zaccaria generation 2 machines only
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
' Zaccaria Data
'-------------------------
' Cabinet switches
Const swAdvanceTest    =  0
Const swReturnTest     =  1
Const swSlamTilt       =  2
Const swCreditService  =  3
Const swCoin1          =  4
Const swCoin2          =  5
Const swCoin3          =  6
Const swStartButton    =  9
Const swTilt           =  10

Const swLRFlip         = 81
Const swLLFlip         = 83

' Help Window
vpmSystemHelp = "Zaccaria keys:" & vbNewLine &_
  vpmKeyName(keyInsertCoin1) & vbTab & "Insert Coin #1"   & vbNewLine &_
  vpmKeyName(keyInsertCoin2) & vbTab & "Insert Coin #2"   & vbNewLine &_
  vpmKeyName(keyInsertCoin3) & vbTab & "Insert Coin #3"   & vbNewLine &_
  vpmKeyName(keyInsertCoin4) & vbTab & "Service Credit"   & vbNewLine &_
  vpmKeyName(keySelfTest)    & vbTab & "Advance Test"     & vbNewLine &_
  vpmKeyName(keyAdvance)     & vbTab & "Return Test"      & vbNewLine &_
  vpmKeyName(keySlamDoorHit) & vbTab & "Slam Tilt"

' Option Menu / Dip Menu

Private Sub zac2ShowDips
	If Not IsObject(vpmDips) Then ' First time
		Set vpmDips = New cvpmDips
		With vpmDips
			.AddForm 150, 245, "DIP Switches"
			.AddFrame  0,0,60,"", 0,Array("DIP  1",&H00000001,"DIP  2",&H00000002,"DIP  3",&H00000004,"DIP  4",&H00000008)
		End With
	End If
	vpmDips.ViewDips
End Sub
Set vpmShowDips = GetRef("zac2ShowDips")
Private vpmDips

' Keyboard handlers
Function vpmKeyDown(ByVal keycode)
	On Error Resume Next
	vpmKeyDown = True ' Assume we handle the key
	With Controller
		Select Case keycode
			Case RightFlipperKey .Switch(swLRFlip) = True : vpmKeyDown = False
			Case LeftFlipperKey  .Switch(swLLFlip) = True : vpmKeyDown = False
			Case keyInsertCoin1  vpmTimer.PulseSw swCoin1 : Playsound SCoin
			Case keyInsertCoin2  vpmTimer.PulseSw swCoin2 : Playsound SCoin
			Case keyInsertCoin3  vpmTimer.PulseSw swCoin3 : Playsound SCoin
			Case keyInsertCoin4  vpmTimer.PulseSw swCreditService ': Playsound SCoin
			Case StartGameKey    Controller.Switch(swStartButton)=1
			Case keySlamDoorHit  Controller.Switch(swSlamTilt)=1
			Case keySelfTest     vpmTimer.PulseSw swAdvanceTest
			Case keyAdvance      vpmTimer.PulseSw swReturnTest
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
			Case RightFlipperKey .Switch(swLRFlip) = False : vpmKeyUp = False
			Case LeftFlipperKey  .Switch(swLLFlip) = False : vpmKeyUp = False
			Case StartGameKey    Controller.Switch(swStartButton)=0
			Case keySlamDoorHit  Controller.Switch(swSlamTilt)=0
			Case keyShowOpts     .Pause = True : .ShowOptsDialog GetPlayerHWnd : .Pause = False
			Case keyShowKeys     .Pause = True : vpmShowHelp : .Pause = False
			Case keyAddBall      .Pause = True : vpmAddBall  : .Pause = False
			Case keyShowDips     If IsObject(vpmShowDips) Then .Pause = True: vpmShowDips : .Pause = False
			Case keyReset        .Stop : BeginModal : .Run : vpmTimer.Reset : EndModal
			Case keyFrame        .LockDisplay = Not .LockDisplay
			Case keyDoubleSize   .DoubleSize  = Not .DoubleSize
			Case Else            vpmKeyUp = False
		End Select
	End With
	On Error Goto 0
End Function
