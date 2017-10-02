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
' Mr. Game Data
'-------------------------
' Cabinet switches
Const swAdvanceTest    =  0
Const swReturnTest     =  1
Const swCreditService  =  3
Const swCoin1          =  4
Const swCoin2          =  5
Const swCoin3          =  6
Const swStartButton    =  9

Const swLRFlip         = 81
Const swLLFlip         = 83

' Help Window
vpmSystemHelp = "Mr. Game keys:" & vbNewLine &_
  vpmKeyName(keyInsertCoin1) & vbTab & "Insert Coin #1"   & vbNewLine &_
  vpmKeyName(keyInsertCoin2) & vbTab & "Insert Coin #2"   & vbNewLine &_
  vpmKeyName(keyInsertCoin3) & vbTab & "Insert Coin #3"   & vbNewLine &_
  vpmKeyName(keyInsertCoin4) & vbTab & "Service Credit"   & vbNewLine &_
  vpmKeyName(keySelfTest)    & vbTab & "Advance Test"     & vbNewLine &_
  vpmKeyName(keyAdvance)     & vbTab & "Return Test"

'Option Menu (No Dips)
Private Sub mrgameShowDips
	If Not IsObject(vpmDips) Then ' First time
		Set vpmDips = New cvpmDips
		With vpmDips
	  	.AddForm  80, 0, "Option Menu"
		.AddLabel 0,0,250,20,"No Options In This Table At This Time"
		End With
	End If
	vpmDips.ViewDips
End Sub
Set vpmShowDips = GetRef("mrgameShowDips")
Private vpmDips

' Keyboard handlers
Function vpmKeyDown(ByVal keycode)
	On Error Resume Next
	vpmKeyDown = True ' Assume we handle the key
	With Controller
		Select Case keycode
			Case RightFlipperKey .Switch(swLRFlip) = True : vpmKeyDown = False
			Case LeftFlipperKey  .Switch(swLLFlip) = True : vpmKeyDown = False
			Case keyInsertCoin1  .Switch(swCoin1)  = True : Playsound SCoin
			Case keyInsertCoin2  .Switch(swCoin2)  = True : Playsound SCoin
			Case keyInsertCoin3  .Switch(swCoin3)  = True : Playsound SCoin
			Case keyInsertCoin4  vpmTimer.PulseSw swCreditService : Playsound SCoin
			Case StartGameKey    .Switch(swStartButton) = True
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
			Case keyInsertCoin1  .Switch(swCoin1)  = False : Playsound SCoin
			Case keyInsertCoin2  .Switch(swCoin2)  = False : Playsound SCoin
			Case keyInsertCoin3  .Switch(swCoin3)  = False : Playsound SCoin
			Case StartGameKey    .Switch(swStartButton) = False
			Case keyShowOpts     .Pause = True : .ShowOptsDialog GetPlayerHWnd : .Pause = False
			Case keyShowKeys     .Pause = True : vpmShowHelp : .Pause = False
			Case keyAddBall      .Pause = True : vpmAddBall  : .Pause = False
			Case keyReset        .Stop : .Run : vpmTimer.Reset
			Case keyFrame        .LockDisplay = Not .LockDisplay
			Case keyDoubleSize   .DoubleSize  = Not .DoubleSize
			Case keyReset        .Stop : BeginModal : .Run : vpmTimer.Reset : EndModal
			Case Else            vpmKeyUp = False
		End Select
	End With
	On Error Goto 0
End Function
