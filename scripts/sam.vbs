'Last Updated in VBS v3.61

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
' Stern S.A.M. Data
'-------------------------
' Cabinet switches
Const swCancel		= -3
Const swDown		= -2
Const swUp			= -1
Const swEnter		=  0
Const swStartButton	= 16
Const swTilt		= -7
Const swSlamTilt	= -6
Const swCoin1		= 65
Const swCoin2		= 66
Const swCoin3		= 67
Const swLRFlip		= 82
Const swLLFlip		= 84
Const swURFlip		= 86
Const swULFlip		= 88

'Const swTournamentButton = 15 'for all machines?

' Help Window
vpmSystemHelp = "Stern S.A.M. keys:" & vbNewLine &_
  vpmKeyName(keyInsertCoin1) & vbTab & "Insert Coin #1" & vbNewLine &_
  vpmKeyName(keyInsertCoin2) & vbTab & "Insert Coin #2" & vbNewLine &_
  vpmKeyName(keyInsertCoin3) & vbTab & "Insert Coin #3" & vbNewLine &_
  vpmKeyName(keyCancel)		 & vbTab & "Back"			& vbNewLine &_
  vpmKeyName(keyDown)		 & vbTab & "Minus"			& vbNewLine &_
  vpmKeyName(keyUp)			 & vbTab & "Plus"			& vbNewLine &_
  vpmKeyName(keyEnter)		 & vbTab & "Select"			& vbNewLine &_
  vpmKeyName(keySlamDoorHit) & vbTab & "Slam Tilt"

' Dip Switch / Options Menu
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
	vpmKeyDown = True ' Assume we handle the key
	With Controller
		Select Case keycode
			Case LeftFlipperKey
				.Switch(swLLFlip) = True : vpmKeyDown = False : vpmFFlipsSam.FlipL True
				If keycode = keyStagedFlipperL Then ' as vbs will not evaluate the Case keyStagedFlipperL then, also handle it here
					vpmFlips.FlipUL True
					If vpmFlips.FlipperSolNumber(2) <> 0 Then .Switch(swULFlip) = True
				End If
			Case RightFlipperKey
				.Switch(swLRFlip) = True : vpmKeyDown = False : : vpmFFlipsSam.FlipR True
				If keycode = keyStagedFlipperR Then ' as vbs will not evaluate the Case keyStagedFlipperR then, also handle it here
					vpmFlips.FlipUR True
					If vpmFlips.FlipperSolNumber(3) <> 0 Then .Switch(swURFlip) = True
				End If
			Case keyStagedFlipperL vpmFlips.FlipUL True : If vpmFlips.FlipperSolNumber(2) <> 0 Then .Switch(swULFlip) = True
			Case keyStagedFlipperR vpmFlips.FlipUR True : If vpmFlips.FlipperSolNumber(3) <> 0 Then .Switch(swURFlip) = True
			Case keyInsertCoin1  vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin1'" : If Not IsEmpty(Eval("SCoin")) Then Playsound SCoin
			Case keyInsertCoin2  vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin2'" : If Not IsEmpty(Eval("SCoin")) Then Playsound SCoin
			Case keyInsertCoin3  vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin3'" : If Not IsEmpty(Eval("SCoin")) Then Playsound SCoin
			Case StartGameKey    .Switch(swStartButton) = True
			Case keyCancel       .Switch(swCancel)      = True
			Case keyDown         .Switch(swDown)        = True
			Case keyUp           .Switch(swUp)          = True
			Case keyEnter        .Switch(swEnter)       = True
			Case keySlamDoorHit  .Switch(swSlamTilt)    = True
			Case keyBangBack     vpmNudge.DoNudge   0, 6
			Case LeftTiltKey     vpmNudge.DoNudge  75, 2
			Case RightTiltKey    vpmNudge.DoNudge 285, 2
			Case CenterTiltKey   vpmNudge.DoNudge   0, 2
			Case Else            vpmKeyDown = False
		End Select
	End With
End Function

Function vpmKeyUp(ByVal keycode)
	vpmKeyUp = True ' Assume we handle the key
	With Controller
		Select Case keycode
			Case LeftFlipperKey
				.Switch(swLLFlip) = False : vpmKeyUp = False : vpmFFlipsSam.FlipL False
				If keycode = keyStagedFlipperL Then ' as vbs will not evaluate the Case keyStagedFlipperL then, also handle it here
					vpmFlips.FlipUL False
					If vpmFlips.FlipperSolNumber(2) <> 0 Then .Switch(swULFlip) = False
				End If
			Case RightFlipperKey
				.Switch(swLRFlip) = False : vpmKeyUp = False : vpmFFlipsSam.FlipR False
				If keycode = keyStagedFlipperR Then ' as vbs will not evaluate the Case keyStagedFlipperR then, also handle it here
					vpmFlips.FlipUR False
					If vpmFlips.FlipperSolNumber(3) <> 0 Then .Switch(swURFlip) = False
				End If
			Case keyStagedFlipperL vpmFlips.FlipUL False : If vpmFlips.FlipperSolNumber(2) <> 0 Then .Switch(swULFlip) = False
			Case keyStagedFlipperR vpmFlips.FlipUR False : If vpmFlips.FlipperSolNumber(3) <> 0 Then .Switch(swURFlip) = False
			Case StartGameKey    .Switch(swStartButton) = False
			Case keyCancel       .Switch(swCancel)      = False
			Case keyDown         .Switch(swDown)        = False
			Case keyUp           .Switch(swUp)          = False
			Case keyEnter        .Switch(swEnter)       = False
			Case keySlamDoorHit  .Switch(swSlamTilt)    = False
			Case keyShowOpts     .Pause = True : vpmShowOptions : .Pause = False
			Case keyShowKeys     .Pause = True : vpmShowHelp : .Pause = False
			Case keyShowDips     If IsObject(vpmShowDips) Then .Pause = True : vpmShowDips : .Pause = False
			Case keyAddBall      .Pause = True : vpmAddBall  : .Pause = False
			Case keyReset        .Stop : BeginModal : .Run : vpmTimer.Reset : EndModal
			Case keyFrame        .LockDisplay = Not .LockDisplay
			Case keyDoubleSize   .DoubleSize  = Not .DoubleSize
			Case Else            vpmKeyUp = False
		End Select
	End With
End Function

' *********************************************************************
' *	   Fast flips for SAM
' *********************************************************************

dim vpmFFlipsSAM : set vpmFFlipsSAM = New cvpmFFlipsSAM : vpmFFlipsSAM.Name = "vpmFFlipsSAM"

'*************************************************
Sub InitVpmFFlipsSAM()
	' Virtually all SAM tables have left/right flippers at 10/15, but upper lfippers may be different.
	vpmFFlipsSAM.FlipperSolNumber(0) = 15
	vpmFFlipsSAM.FlipperSolNumber(1) = 16
	vpmFFlipsSAM.CallBackL = SolCallback(vpmFFlipsSAM.FlipperSolNumber(0))			'Lower Flippers
	vpmFFlipsSAM.CallBackR = SolCallback(vpmFFlipsSAM.FlipperSolNumber(1))
	if vpmFlips.FlipperSolNumber(2) <> 0 then vpmFFlipsSAM.CallbackUL=SolCallback(vpmFFlipsSAM.FlipperSolNumber(2)) Else vpmFFlipsSAM.FlipperSolNumber(2) = 0
	if vpmFlips.FlipperSolNumber(3) <> 0 then vpmFFlipsSAM.CallbackUR=SolCallback(vpmFFlipsSAM.FlipperSolNumber(3)) Else vpmFFlipsSAM.FlipperSolNumber(3) = 0
	'msgbox "~Debug-Active Flipper subs~" & vbnewline & vpmFFlipsSAM.SubL &vbnewline& vpmFFlipsSAM.SubR &vbnewline& vpmFFlipsSAM.SubUL &vbnewline& vpmFFlipsSAM.SubUR' &
End Sub


Class cvpmFFlipsSAM	'test fastflips with support for both Rom and Game-On Solenoid flipping
	Public TiltObjects, DebugOn, Name, Delay
	Public FlipperSub(3)	'Set to the flipper subs by .init
	Public FlipperSubRef(3)	'Set to the flipper subs by .init
	Public FlippersEnabled,  LagCompensation, ButtonState(3), Sol	'set private
	Public RomMode,	SolState(3)'set private
	Public FlipperSolNumber(3)	'0=left 1=right 2=Uleft 3=URight

	Private Sub Class_Initialize()
		dim idx: For idx = 0 to 3
			ButtonState(idx) = False
			SolState(idx) = False
			FlipperSub(idx) = "NullFunction"
			Set FlipperSubRef(idx) = Nothing
		Next
		Delay=0: FlippersEnabled=1: DebugOn=0 : LagCompensation=0 : Sol=0 : TiltObjects=1: RomMode=True
		FlipperSolNumber(0)=sLLFlipper
		FlipperSolNumber(1)=sLRFlipper
		FlipperSolNumber(2)=sULFlipper
		FlipperSolNumber(3)=sURFlipper
		SolCallback(33)="vpmFFlipsSAM.RomControl = not"
		SolCallbackInitialized = False
	End Sub

	'set callbacks
	Public Property Let CallBackL(ByVal aInput) : CallBack(0) = aInput: End Property
	Public Property Let CallBackR(ByVal aInput) : CallBack(1) = aInput: End Property
	Public Property Let CallBackUL(ByVal aInput) : CallBack(2) = aInput: End Property
	Public Property Let CallBackUR(ByVal aInput) : CallBack(3) = aInput: End Property
	Public Property Let CallBack(aIdx, ByVal aInput)
		If Not IsEmpty(aInput) Then
			FlipperSub(aIdx) = aInput
			SolCallback(FlipperSolNumber(aIdx)) = name & ".RomFlip(" & aIdx & ")="
			SolCallbackInitialized = False
			Dim cbs: cbs = "Sub XXXSAMFlipperSub_" & aIdx & "(state)" & vblf & aInput & " state" & vblf & "End Sub"
			ExecuteGlobal cbs
			Set FlipperSubRef(aIdx) = GetRef("XXXSAMFlipperSub_" & aIdx)
		End If
	End Property

	Public Property Let RomFlip(idx, ByVal aEnabled)
		aEnabled = abs(aEnabled)
		SolState(idx) = aEnabled
		If Not RomMode then Exit Property
		Dim cb: Set cb = FlipperSubRef(idx)
		If Not cb is Nothing Then cb aEnabled
	End property

	Public Property Let RomControl(ByVal aEnabled)	'todo improve choreography
		'MsgBox "Rom Control " & CStr(aEnabled)
		RomMode = aEnabled
		Dim idx, cb: For idx = 0 To 3
			Set cb = FlipperSubRef(idx)
			If aEnabled then					'Switch to ROM solenoid states or button states
				If Not cb is Nothing Then cb SolState(idx)
			Else
				If Not cb is Nothing Then cb ButtonState(idx)
			End If
		Next
	End Property
	Public Property Get RomControl : RomControl = RomMode : End Property

	public DebugTestKeys, DebugTestInit	'orphaned (stripped out the debug stuff)

	Public Property Let Solenoid(aInput) : if not IsEmpty(aInput) then Sol = aInput : end if : End Property	'set solenoid
	Public Property Get Solenoid : Solenoid = sol : End Property

	'call callbacks
	Public Sub FlipL(ByVal aEnabled) : Flip 0, aEnabled : End Sub
	Public Sub FlipR(ByVal aEnabled) : Flip 1, aEnabled : End Sub
	Public Sub FlipUL(ByVal aEnabled) : Flip 2, aEnabled : End Sub
	Public Sub FlipUR(ByVal aEnabled) : Flip 3, aEnabled : End Sub
	Public Sub Flip(aIdx, ByVal aEnabled)
		aEnabled = abs(aEnabled)
		ButtonState(aIdx) = aEnabled
		If FlippersEnabled and Not Romcontrol or DebugOn then
			Dim cb: Set cb = FlipperSubRef(aIdx)
			If Not cb is Nothing Then cb aEnabled
		end If
	End Sub

	Public Sub TiltSol(aEnabled)	'Handle solenoid / Delay (if delayinit)
		If delay > 0 and not aEnabled then	'handle delay
			vpmtimer.addtimer Delay, Name & ".FireDelay" & "'"
			LagCompensation = 1
		else
			If Delay > 0 then LagCompensation = 0
			EnableFlippers(aEnabled)
		end If
	End Sub

	Sub FireDelay() : If LagCompensation then EnableFlippers 0 End If : End Sub

	Public Sub EnableFlippers(ByVal aEnabled)	'private
		Dim idx, cb
		If aEnabled then
			For idx = 0 To 3
				Set cb = FlipperSubRef(idx)
				If Not cb is Nothing Then cb ButtonState(idx)
			Next
		End If
		FlippersEnabled = aEnabled
		If TiltObjects then vpmnudge.solgameon aEnabled
		If Not aEnabled then
			For idx = 0 To 3
				Set cb = FlipperSubRef(idx)
				If Not cb is Nothing Then cb False
			Next
		End If
	End Sub
End Class
