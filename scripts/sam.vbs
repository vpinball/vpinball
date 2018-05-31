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
			Case RightFlipperKey .Switch(swLRFlip) = True : vpmKeyDown = False : vpmFFlipsSam.FlipR true 
			Case LeftFlipperKey  .Switch(swLLFlip) = True : vpmKeyDown = False : vpmFFlipsSam.FlipL true
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
			Case RightFlipperKey .Switch(swLRFlip) = False : vpmKeyUp = False : vpmFFlipsSam.FlipR false
			Case LeftFlipperKey  .Switch(swLLFlip) = False : vpmKeyUp = False : vpmFFlipsSam.FlipL false 
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

' *********************************************************************
' *    Fast flips for SAM
' *********************************************************************

dim vpmFFlipsSAM : set vpmFFlipsSAM = New cvpmFFlipsSAM : vpmFFlipsSAM.Name = "vpmFFlipsSAM"

'*************************************************
Sub InitVpmFFlipsSAM()
	' Virtually all SAM tables have left/right flippers at 10/15, but upper lfippers may be different. 
	vpmFFlipsSAM.FlipperSolNumber(0) = 15
	vpmFFlipsSAM.FlipperSolNumber(1) = 16
	vpmFFlipsSAM.CallBackL = SolCallback(vpmFFlipsSAM.FlipperSolNumber(0))          'Lower Flippers
	vpmFFlipsSAM.CallBackR = SolCallback(vpmFFlipsSAM.FlipperSolNumber(1))
	On Error Resume Next
		if cSingleLflip or Err then vpmFFlipsSAM.CallbackUL=SolCallback(vpmFFlipsSAM.FlipperSolNumber(2))
		err.clear
		if cSingleRflip or Err then vpmFFlipsSAM.CallbackUR=SolCallback(vpmFFlipsSAM.FlipperSolNumber(3))
	On Error Goto 0
	'msgbox "~Debug-Active Flipper subs~" & vbnewline & vpmFFlipsSAM.SubL &vbnewline& vpmFFlipsSAM.SubR &vbnewline& vpmFFlipsSAM.SubUL &vbnewline& vpmFFlipsSAM.SubUR' & 
End Sub


Class cvpmFFlipsSAM	'test fastflips with support for both Rom and Game-On Solenoid flipping
	Public TiltObjects, DebugOn, Name, Delay
	Public SubL, SubUL, SubR, SubUR, FlippersEnabled,  LagCompensation, ButtonState(3), Sol	'set private
	Public RomMode,	SolState(3)'set private
	Public FlipperSolNumber(3)	'0=left 1=right 2=Uleft 3=URight

	Private Sub Class_Initialize()
		dim idx :for idx = 0 to 3 :ButtonState(idx)=0:SolState(idx)=0: Next : Delay=0: FlippersEnabled=1: DebugOn=0 : LagCompensation=0 : Sol=0 : TiltObjects=1
		SubL = "NullFunction": SubR = "NullFunction" : SubUL = "NullFunction": SubUR = "NullFunction"
		RomMode=True :FlipperSolNumber(0)=sLLFlipper :FlipperSolNumber(1)=sLRFlipper :FlipperSolNumber(2)=sULFlipper :FlipperSolNumber(3)=sURFlipper
		SolCallback(33)="vpmFFlipsSAM.RomControl = not "
	End Sub

	'set callbacks
	Public Property Let CallBackL(aInput) : if Not IsEmpty(aInput) then SubL  = aInput :SolCallback(FlipperSolNumber(0)) = name & ".RomFlip(0)=":end if :End Property	'execute
	Public Property Let CallBackR(aInput) : if Not IsEmpty(aInput) then SubR  = aInput :SolCallback(FlipperSolNumber(1)) = name & ".RomFlip(1)=":end if :End Property
	Public Property Let CallBackUL(aInput): if Not IsEmpty(aInput) then SubUL = aInput :SolCallback(FlipperSolNumber(2)) = name & ".RomFlip(2)=":end if :End Property	'this should no op if aInput is empty
	Public Property Let CallBackUR(aInput): if Not IsEmpty(aInput) then SubUR = aInput :SolCallback(FlipperSolNumber(3)) = name & ".RomFlip(3)=":end if :End Property
	
	Public Property Let RomFlip(idx, ByVal aEnabled)
		aEnabled = abs(aEnabled)
		SolState(idx) = aEnabled
		If Not RomMode then Exit Property
		Select Case idx
			Case 0 : execute subL & " " & aEnabled
			Case 1 : execute subR & " " & aEnabled
			Case 2 : execute subUL &" " & aEnabled
			Case 3 : execute subUR &" " & aEnabled
		End Select
	End property
	
	Public Property Let RomControl(aEnabled) 		'todo improve choreography
		'MsgBox "Rom Control " & CStr(aEnabled)
		RomMode = aEnabled
		If aEnabled then 					'Switch to ROM solenoid states or button states
			Execute SubL &" "& SolState(0)
			Execute SubR &" "& SolState(1)
			Execute SubUL &" "& SolState(2)
			Execute SubUR &" "& SolState(3)
		Else
			Execute SubL &" "& ButtonState(0)
			Execute SubR &" "& ButtonState(1)
			Execute SubUL &" "& ButtonState(2)
			Execute SubUR &" "& ButtonState(3)
		End If
	End Property
	Public Property Get RomControl : RomControl = RomMode : End Property

	public DebugTestKeys, DebugTestInit	'orphaned (stripped out the debug stuff)

	Public Property Let Solenoid(aInput) : if not IsEmpty(aInput) then Sol = aInput : end if : End Property	'set solenoid
	Public Property Get Solenoid : Solenoid = sol : End Property
	
	'call callbacks
	Public Sub FlipL(ByVal aEnabled)
		aEnabled = abs(aEnabled) 'True / False is not region safe with execute. Convert to 1 or 0 instead.
		DebugTestKeys = 1
		ButtonState(0) = aEnabled	'track flipper button states: the game-on sol flips immediately if the button is held down (1.1)
		If FlippersEnabled and Not Romcontrol or DebugOn then execute subL & " " & aEnabled end If
	End Sub

	Public Sub FlipR(ByVal aEnabled)
		aEnabled = abs(aEnabled) : ButtonState(1) = aEnabled : DebugTestKeys = 1
		If FlippersEnabled and Not Romcontrol or DebugOn then execute subR & " " & aEnabled end If
	End Sub

	Public Sub FlipUL(ByVal aEnabled)
		aEnabled = abs(aEnabled)  : ButtonState(2) = aEnabled
		If FlippersEnabled and Not Romcontrol or DebugOn then execute subUL & " " & aEnabled end If
	End Sub	

	Public Sub FlipUR(ByVal aEnabled)
		aEnabled = abs(aEnabled)  : ButtonState(3) = aEnabled
		If FlippersEnabled and Not Romcontrol or DebugOn then execute subUR & " " & aEnabled end If
	End Sub	
	
	Public Sub TiltSol(aEnabled)	'Handle solenoid / Delay (if delayinit)
		If delay > 0 and not aEnabled then 	'handle delay
			vpmtimer.addtimer Delay, Name & ".FireDelay" & "'"
			LagCompensation = 1
		else
			If Delay > 0 then LagCompensation = 0
			EnableFlippers(aEnabled)
		end If
	End Sub
	
	Sub FireDelay() : If LagCompensation then EnableFlippers 0 End If : End Sub
	
	Public Sub EnableFlippers(aEnabled)	'private
		If aEnabled then execute SubL &" "& ButtonState(0) :execute SubR &" "& ButtonState(1) :execute subUL &" "& ButtonState(2): execute subUR &" "& ButtonState(3)':end if
		FlippersEnabled = aEnabled
		If TiltObjects then vpmnudge.solgameon aEnabled
		If Not aEnabled then
			execute subL & " " & 0 : execute subR & " " & 0
			execute subUL & " " & 0 : execute subUR & " " & 0
		End If
	End Sub
End Class
