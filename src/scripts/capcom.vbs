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
' Capcom Data
'-------------------------
Const swCoin1       = 01
Const swCoin2       = 02
Const swCoin3       = 03
Const swCoin4       = 04
Const swDiagnostic  = 08
Const swStartButton = 07
Const swTilt        = 10
Const swSlamTilt    = 09
Const swBuyIn       = 11

Const swLRFlip      = 82
Const swLLFlip      = 84
Const swURFlip      = 81
Const swULFlip      = 83

Const GameOnSolenoid = 51

'-------------------------
' Flipper strength (PinMAME 3.7 and later)
'-------------------------
' The game's flipper strength setting is the value of its flipper solenoids while on: 9 (left flipper), 10 (right),
'  11 (upper right: Breakshot, Pool Player, Big Bang Bar), 11/12 (Flipper Football: upper left/right), and their
'  mirrors in the legacy mapping for DOF: 9 -> 45/46, 10 -> 47/48, 11 -> 33/34, 12 -> 35/36 (power/hold; 33-36 only
'  in games with upper flippers). Kingpin, Flipper Football: setting/32; the others: setting/16
' Without UseVPMModSol: on/off as before, nothing to do
' With UseVPMModSol = 1 (value 0..255) or 2 (value 0..1), read it through SolModCallback, e.g. with 2:
'  SolModCallback(9) = "LeftFlipperStrength"
'  Sub LeftFlipperStrength(value) : If value > 0 Then LeftFlipper.Strength = LeftFlipperFullStrength * value : End Sub
' Caveats:
' - The value may stay below half while on (e.g. Flipper Football's upper flippers: 14/32 from the factory): treat any
'   value above 0 as on, so never test for >= 0.5 (or >= 128). SolCallback does that for these solenoids (SolOnAboveZero below, needs core.vbs 3.62 or later)
' - 45-48 now follow only the flipper coils and 33-36 only the upper flippers, also without UseVPMModSol: check existing SolCallbacks on these
Dim capFlipSol
For Each capFlipSol In Array(9, 10, 11, 12, 33, 34, 35, 36, 45, 46, 47, 48) : SolOnAboveZero(capFlipSol) = True : Next

' Help window
vpmSystemHelp = "Capcom keys" & vbNewLine &_
  vpmKeyName(keyInsertCoin1)  & vbTab & "Insert Coin #1" & vbNewLine &_
  vpmKeyName(keyInsertCoin2)  & vbTab & "Insert Coin #2" & vbNewLine &_
  vpmKeyName(keyInsertCoin3)  & vbTab & "Insert Coin #3" & vbNewLine &_
  vpmKeyName(keyInsertCoin4)  & vbTab & "Insert Coin #4" & vbNewLine &_
  vpmKeyName(keySlamDoorHit)  & vbTab & "Slam Tilt"

' Options Menu (No Dips)
Private Sub CapcomShowDips
	If Not IsObject(vpmDips) Then ' First time
		Set vpmDips = New cvpmDips
		With vpmDips
		.AddForm  80, 0, "Option Menu"
		.AddLabel 0,0,250,20,"No Options In This Table At This Time"
		End With
	End If
	vpmDips.ViewDips
End Sub
Set vpmShowDips = GetRef("CapcomShowDips")
Private vpmDips

' Keyboard handlers
Function vpmKeyDown(ByVal keycode)
	vpmKeyDown = True ' assume we handle the key
	Dim stagedFlipperL, stagedFlipperR
	If keyStagedFlipperL & "" = "" Then stagedFlipperL = -1 Else stagedFlipperL = StagedLeftFlipperKey ' "" disables staged flipper processing
	If keyStagedFlipperR & "" = "" Then stagedFlipperR = -1 Else stagedFlipperR = StagedRightFlipperKey
	With Controller
		Select Case keycode
			Case LeftFlipperKey
				.Switch(swLLFlip) = True : vpmKeyDown = False : vpmFlips.FlipL True
				If keycode = stagedFlipperL Then ' as vbs will not evaluate the Case stagedFlipperL then, also handle it here
					vpmFlips.FlipUL True
					If vpmFlips.FlipperSolNumber(2) <> 0 Then .Switch(swULFlip) = True
				End If
			Case RightFlipperKey
				.Switch(swLRFlip) = True : vpmKeyDown = False : vpmFlips.FlipR True
				If keycode = stagedFlipperR Then ' as vbs will not evaluate the Case stagedFlipperR then, also handle it here
					vpmFlips.FlipUR True
					If vpmFlips.FlipperSolNumber(3) <> 0 Then .Switch(swURFlip) = True
				End If
			Case stagedFlipperL vpmFlips.FlipUL True : If vpmFlips.FlipperSolNumber(2) <> 0 Then .Switch(swULFlip) = True
			Case stagedFlipperR vpmFlips.FlipUR True : If vpmFlips.FlipperSolNumber(3) <> 0 Then .Switch(swURFlip) = True
			Case keyInsertCoin1  vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin1'" : If Not IsEmpty(Eval("SCoin")) Then Playsound SCoin
			Case keyInsertCoin2  vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin2'" : If Not IsEmpty(Eval("SCoin")) Then Playsound SCoin
			Case keyInsertCoin3  vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin3'" : If Not IsEmpty(Eval("SCoin")) Then Playsound SCoin
			Case keyInsertCoin4  vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin4'" : If Not IsEmpty(Eval("SCoin")) Then Playsound SCoin
			Case StartGameKey    .Switch(swStartButton) = True
			Case keySelfTest     .Switch(swDiagnostic)  = NOT .Switch(swDiagnostic)
			Case keySlamDoorHit  .Switch(swSlamTilt)    = True
			Case keyBangBack     vpmNudge.DoMechTilt
			Case Else            vpmKeyDown = False
		End Select
	End With
End Function

Function vpmKeyUp(ByVal keycode)
	vpmKeyUp = True ' assume we handle the key
	Dim stagedFlipperL, stagedFlipperR
	If keyStagedFlipperL & "" = "" Then stagedFlipperL = -1 Else stagedFlipperL = StagedLeftFlipperKey ' "" disables staged flipper processing
	If keyStagedFlipperR & "" = "" Then stagedFlipperR = -1 Else stagedFlipperR = StagedRightFlipperKey
	With Controller
		Select Case keycode
			Case LeftFlipperKey
				.Switch(swLLFlip) = False : vpmKeyUp = False : vpmFlips.FlipL False
				If keycode = stagedFlipperL Then ' as vbs will not evaluate the Case stagedFlipperL then, also handle it here
					vpmFlips.FlipUL False
					If vpmFlips.FlipperSolNumber(2) <> 0 Then .Switch(swULFlip) = False
				End If
			Case RightFlipperKey
				.Switch(swLRFlip) = False : vpmKeyUp = False : vpmFlips.FlipR False
				If keycode = stagedFlipperR Then ' as vbs will not evaluate the Case stagedFlipperR then, also handle it here
					vpmFlips.FlipUR False
					If vpmFlips.FlipperSolNumber(3) <> 0 Then .Switch(swURFlip) = False
				End If
			Case stagedFlipperL vpmFlips.FlipUL False : If vpmFlips.FlipperSolNumber(2) <> 0 Then .Switch(swULFlip) = False
			Case stagedFlipperR vpmFlips.FlipUR False : If vpmFlips.FlipperSolNumber(3) <> 0 Then .Switch(swURFlip) = False
			Case StartGameKey    .Switch(swStartButton) = False
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
