'Last Updated in VBS v3.62

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
' Tecnoplay Data (X Force, Space Team)
'-------------------------
' Flipper Solenoid (game on, cleared by tilt)
Const GameOnSolenoid = 20
' Cabinet switches (numbered as in the manual, starting at 0)
Const swTestUp       =  0
Const swTestDown     =  1
Const swSlamTilt     =  2
Const swService      =  3
Const swCoin1        =  4
Const swCoin2        =  5
Const swCoin3        =  6
Const swLetterRight  =  7
Const swLetterLeft   =  8
Const swStartButton  =  9
Const swTilt         = 10

Const swLRFlip       = 81
Const swLLFlip       = 83

' Help Window
vpmSystemHelp = "Tecnoplay keys:" & vbNewLine &_
  vpmKeyName(keyInsertCoin1) & vbTab & "Insert Coin #1"   & vbNewLine &_
  vpmKeyName(keyInsertCoin2) & vbTab & "Insert Coin #2"   & vbNewLine &_
  vpmKeyName(keyInsertCoin3) & vbTab & "Insert Coin #3"   & vbNewLine &_
  vpmKeyName(keySelfTest)    & vbTab & "Test Up"          & vbNewLine &_
  vpmKeyName(keyAdvance)     & vbTab & "Test Down"        & vbNewLine &_
  vpmKeyName(keyCPUDiag)     & vbTab & "Service"          & vbNewLine &_
  vpmKeyName(keyMasterEnter) & vbTab & "Letter Select ->" & vbNewLine &_
  vpmKeyName(keyJoyLeft)     & vbTab & "Letter Select <-" & vbNewLine &_
  vpmKeyName(keySlamDoorHit) & vbTab & "Slam Tilt"

' Options Menu (No Dips)
Private Sub tecnoplayShowDips
	If Not IsObject(vpmDips) Then ' First time
		Set vpmDips = New cvpmDips
		With vpmDips
			.AddForm  80, 0, "Option Menu"
			.AddLabel 0,0,250,20,"No Options In This Table At This Time"
		End With
	End If
	vpmDips.ViewDips
End Sub
Set vpmShowDips = GetRef("tecnoplayShowDips")
Private vpmDips

' Keyboard handlers
Function vpmKeyDown(ByVal keycode)
	vpmKeyDown = True ' Assume we handle the key
	Dim stagedFlipperL, stagedFlipperR
	If keyStagedFlipperL & "" = "" Then stagedFlipperL = -1 Else stagedFlipperL = StagedLeftFlipperKey ' "" disables staged flipper processing
	If keyStagedFlipperR & "" = "" Then stagedFlipperR = -1 Else stagedFlipperR = StagedRightFlipperKey
	With Controller
		Select Case keycode
			Case LeftFlipperKey  .Switch(swLLFlip) = True : vpmKeyDown = False : vpmFlips.FlipL True : if keycode = stagedFlipperL then vpmFlips.FlipUL True
			Case RightFlipperKey .Switch(swLRFlip) = True : vpmKeyDown = False : vpmFlips.FlipR True : if keycode = stagedFlipperR then vpmFlips.FlipUR True
			Case stagedFlipperL vpmFlips.FlipUL True
			Case stagedFlipperR vpmFlips.FlipUR True
			Case keyInsertCoin1  .Switch(swCoin1)  = True : If Not IsEmpty(Eval("SCoin")) Then Playsound SCoin
			Case keyInsertCoin2  .Switch(swCoin2)  = True : If Not IsEmpty(Eval("SCoin")) Then Playsound SCoin
			Case keyInsertCoin3  .Switch(swCoin3)  = True : If Not IsEmpty(Eval("SCoin")) Then Playsound SCoin
			Case StartGameKey    .Switch(swStartButton) = True
			Case keySelfTest     .Switch(swTestUp)      = True
			Case keyAdvance      .Switch(swTestDown)    = True
			Case keyCPUDiag      .Switch(swService)     = True
			Case keyMasterEnter  .Switch(swLetterRight) = True
			Case keyJoyLeft      .Switch(swLetterLeft)  = True
			Case keySlamDoorHit  .Switch(swSlamTilt)    = True
			Case keyBangBack     vpmNudge.DoMechTilt
			Case Else            vpmKeyDown = False
		End Select
	End With
End Function

Function vpmKeyUp(ByVal keycode)
	vpmKeyUp = True ' Assume we handle the key
	Dim stagedFlipperL, stagedFlipperR
	If keyStagedFlipperL & "" = "" Then stagedFlipperL = -1 Else stagedFlipperL = StagedLeftFlipperKey ' "" disables staged flipper processing
	If keyStagedFlipperR & "" = "" Then stagedFlipperR = -1 Else stagedFlipperR = StagedRightFlipperKey
	With Controller
		Select Case keycode
			Case LeftFlipperKey  .Switch(swLLFlip) = False : vpmKeyUp = False : vpmFlips.FlipL False : if keycode = stagedFlipperL then vpmFlips.FlipUL False
			Case RightFlipperKey .Switch(swLRFlip) = False : vpmKeyUp = False : vpmFlips.FlipR False : if keycode = stagedFlipperR then vpmFlips.FlipUR False
			Case stagedFlipperL vpmFlips.FlipUL False
			Case stagedFlipperR vpmFlips.FlipUR False
			Case keyInsertCoin1  .Switch(swCoin1)  = False
			Case keyInsertCoin2  .Switch(swCoin2)  = False
			Case keyInsertCoin3  .Switch(swCoin3)  = False
			Case StartGameKey    .Switch(swStartButton) = False
			Case keySelfTest     .Switch(swTestUp)      = False
			Case keyAdvance      .Switch(swTestDown)    = False
			Case keyCPUDiag      .Switch(swService)     = False
			Case keyMasterEnter  .Switch(swLetterRight) = False
			Case keyJoyLeft      .Switch(swLetterLeft)  = False
			Case keySlamDoorHit  .Switch(swSlamTilt)    = False
			Case keyShowOpts     .Pause = True : vpmShowOptions : .Pause = False
			Case keyShowKeys     .Pause = True : vpmShowHelp : .Pause = False
			Case keyAddBall      .Pause = True : vpmAddBall  : .Pause = False
			Case keyReset        .Stop : BeginModal : .Run : vpmTimer.Reset : EndModal
			Case keyFrame        .LockDisplay = Not .LockDisplay
			Case keyDoubleSize   .DoubleSize  = Not .DoubleSize
			Case Else            vpmKeyUp = False
		End Select
	End With
End Function
