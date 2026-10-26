'Last Updated in VBS v3.62

'Stern SPIKE 1

'The titles number their cabinet switches differently, so this script picks swCoin1, swStartButton
'and the rest from cGameName, so this MUST be set! a table must not declare cab switches itself.

'Switch numbers are the factory manual's Switch Reference numbers, which the game's switch test
'shows. The CPU board's own inputs, which the manual calls C1-C16, are 101-116:
' - 101-108 DIP switches 1-8, set from PinMAME's DIP settings (vpmShowDips)
' - 109-112 the coin door's service buttons: Select, Plus, Minus, Back
' - 116     the power interlock: CLOSED while the coin door is shut. The machine switches its
'           coil power off while it is open, as a real one does. It powers up closed
'An optional topper numbers its switches from 121 (Game of Thrones' TOPPER DRAGON is 121).

'The flippers are worked by the node boards: a flipper button switch closing fires the flipper
'coil on the board itself. So a table only closes the button switches (vpmKeyDown does) and moves
'its flippers from the flipper solenoids. GameOnSolenoid (33) is on while the flippers are enabled,
'as SAM's: the game arms their reflexes on the boards while a ball is in play and disarms them at a
'tilt and when a ball or the game ends. The boards fire the slings and pops the same way, armed and
'disarmed with the flippers, so a table that kicks its own slings and pops switches them with
'SolCallback(GameOnSolenoid) = "vpmNudge.SolGameOn" and vpmNudge.TiltObj set (UseSolenoids = 2
'does it through vpmFlips).
'The lower flipper coils come out as sLLFlipper/sLRFlipper (48/46) on every title, besides their own
'Driver Reference numbers; an upper flipper is its coil's number only (Game of Thrones LE: 18 left, 16 right;
'WWE: 22 left). Titles with upper flippers have a second leaf on each flipper button, which the
'flipper keys close with the lower one. A table for a cabinet with staged flipper buttons sets
'Spike1StagedFlippers = True: the upper leaves then follow the staged keys only (the flipper keys
'while staged flipper processing is off).

'Solenoid map: coils by their Driver Reference number, 1-32; GAME ON as 33; a coil numbered 0 or
'above 32 as 51 and up, in number order. Lamps are the LED channels by their Light Reference number
'(an RGB LED is three channels; the GI strings and flashers are LED channels too). The boards fade the
'LEDs, so for lamps that fade as on the machine set Const UseVPMModSol = 2: core.vbs then asks for
'the lamps and coils as levels (0 to 1). A held flipper's coil then reads its hold duty, which is
'below half where the coil holds itself (Ghostbusters: 11%); this script marks each title's flipper coils,
'and 46/48, in SolOnAboveZero, so their SolCallback stays on at any level above 0. Motors and steppers are Controller.GetMech(n).

'Displays: the DMD (Transformers/Avengers The Pin: their alphanumeric display) is PinMAME display 0
'Ghostbusters' LCD insert and WWE's playfield LCD are display 1

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
' Stern SPIKE 1 Data
'-------------------------
' The CPU board (the same on every title)
Const swDip1	 = 101 ' to swDip1 + 7 for DIP 8
Const swEnter	 = 109 ' Service Select
Const swUp		 = 110 ' Service Plus
Const swDown	 = 111 ' Service Minus
Const swCancel	 = 112 ' Service Back
Const swCoinDoor = 116 ' power interlock: set = door CLOSED
Const GameOnSolenoid = 33 ' on while the flippers are enabled: off at a tilt, between balls and after a game

' The cabinet, by title (0 where a title has no such switch)
Dim swCoin1, swCoin2, swCoin3, swCoin4, swStartButton, swTournament, swLaunch, swTilt, swSlamTilt
Dim swLLFlip, swLRFlip, swULFlip, swURFlip
Dim Spike1StagedFlippers : Spike1StagedFlippers = False ' True: the upper flipper leaves on the staged keys only
Private spike1FlipperCoils ' the flipper coils by their Driver Reference numbers, power and hold

Private Sub spike1Cabinet(ByVal aGame)
	Dim title : title = LCase(aGame)
	If InStr(title, "_") > 0 Then title = Left(title, InStr(title, "_") - 1)
	swCoin1 = 0 : swCoin2 = 0 : swCoin3 = 0 : swCoin4 = 0 : swStartButton = 0 : swTilt = 0 : swSlamTilt = 0
	swLLFlip = 0 : swLRFlip = 0 : swULFlip = 0 : swURFlip = 0 : swTournament = 0 : swLaunch = 0
	spike1FlipperCoils = Array()
	Select Case title
		Case "gbust"					' Ghostbusters
			swCoin1 = 81 : swCoin2 = 82 : swCoin3 = 83 : swCoin4 = 84
			swStartButton = 78 : swTournament = 79 : swLaunch = 76 : swTilt = 86 : swSlamTilt = 89
			swLLFlip = 9 : swLRFlip = 10 : spike1FlipperCoils = Array(3, 4)
		Case "wnbjm", "primus", "pabst" ' Whoa Nellie! Big Juicy Melons, Primus, Pabst Can Crusher
			swCoin1 = 53 : swCoin2 = 54 : swCoin3 = 55 : swCoin4 = 56
			swStartButton = 52 : swTilt = 58 : swSlamTilt = 60
			swLLFlip = 3 : swLRFlip = 4 : spike1FlipperCoils = Array(1, 2, 3, 4)
		Case "got"						' Game of Thrones: the LE has upper flippers, the Pro none
			swCoin1 = 66 : swCoin2 = 67 : swCoin3 = 68 : swCoin4 = 69
			swStartButton = 63 : swTournament = 64 : swLaunch = 60 : swTilt = 71 : swSlamTilt = 75
			swLLFlip = 10 : swLRFlip = 11 : spike1FlipperCoils = Array(3, 8)
			If Right(LCase(aGame), 1) = "h" Then
				swULFlip = 12 : swURFlip = 13 : spike1FlipperCoils = Array(3, 8, 18, 16)
				vpmFlips.FlipperSolNumber(2) = 18 : vpmFlips.FlipperSolNumber(3) = 16
			End If
		Case "kiss15"					' KISS
			swCoin1 = 67 : swCoin2 = 68 : swCoin3 = 69 : swCoin4 = 70
			swStartButton = 62 : swTournament = 15 : swLaunch = 73 : swTilt = 72 : swSlamTilt = 77
			swLLFlip = 10 : swLRFlip = 11 : spike1FlipperCoils = Array(15, 16, 20, 21)
		Case "wwe"						' Wrestlemania: an upper left flipper, on the buttons' second leaves
			swCoin1 = 67 : swCoin2 = 68 : swCoin3 = 69 : swCoin4 = 70
			swStartButton = 62 : swTournament = 15 : swTilt = 72 : swSlamTilt = 77
			swLLFlip = 10 : swLRFlip = 11 : swULFlip = 74 : swURFlip = 75 : spike1FlipperCoils = Array(15, 16, 20, 21, 22)
			vpmFlips.FlipperSolNumber(2) = 22
		Case "heavym20"					' Heavy Metal
			swCoin1 = 76 : swCoin2 = 77 : swCoin3 = 78 : swCoin4 = 79
			swStartButton = 73 : swTournament = 74 : swLaunch = 70 : swTilt = 81 : swSlamTilt = 84
			swLLFlip = 8 : swLRFlip = 10 : spike1FlipperCoils = Array(3, 4)
		Case "supreme"					' Supreme: its START BUTTON is 13 (the coin door's own is 73)
			swCoin1 = 76 : swCoin2 = 77 : swCoin3 = 78 : swCoin4 = 79
			swStartButton = 13 : swTournament = 74 : swLaunch = 15 : swTilt = 14 : swSlamTilt = 84
			swLLFlip = 8 : swLRFlip = 10 : spike1FlipperCoils = Array(3, 4)
		Case "smancp"					' Spider-Man Home Pin: free play, no coin or slam switches; Supreme's other numbers
			swCoin1 = 0 : swCoin2 = 0 : swCoin3 = 0 : swCoin4 = 0
			swStartButton = 13 : swLaunch = 15 : swTilt = 14 : swSlamTilt = 0
			swLLFlip = 8 : swLRFlip = 10 : spike1FlipperCoils = Array(3, 4)
		Case "tfcp", "avscp"			' Transformers The Pin, Avengers The Pin (ELG framework): free play, no coin door
			swCoin1 = 0 : swCoin2 = 0 : swCoin3 = 0 : swCoin4 = 0
			swStartButton = 1 : swSlamTilt = 0
			If title = "tfcp" Then swTilt = 46 Else swTilt = 49
			swLLFlip = 2 : swLRFlip = 3 ' flipper coils unknown (no manual): only sLLFlipper/sLRFlipper are marked
		Case Else
			MsgBox "spike1.vbs does not know the set """ & aGame & """."
	End Select
End Sub

Private spike1Game : spike1Game = ""
On Error Resume Next
spike1Game = Eval("cGameName")
If Err Then MsgBox "spike1.vbs needs the table to define cGameName."
On Error Goto 0
spike1Cabinet spike1Game

' A held flipper's coil reads its hold duty: SolCallback on at any level above 0 (see the top)
Private Sub spike1FlipperLevels
	Dim coil
	On Error Resume Next
	For Each coil In spike1FlipperCoils : SolOnAboveZero(coil) = True : Next
	SolOnAboveZero(sLLFlipper) = True : SolOnAboveZero(sLRFlipper) = True
	On Error Goto 0
End Sub
spike1FlipperLevels

' Help Window
vpmSystemHelp = "Stern SPIKE 1 keys:" & vbNewLine &_
  vpmKeyName(keyInsertCoin1) & vbTab & "Insert Coin #1 (left)" & vbNewLine &_
  vpmKeyName(keyInsertCoin2) & vbTab & "Insert Coin #2 (right)" & vbNewLine &_
  vpmKeyName(keyInsertCoin3) & vbTab & "Insert Coin #3 (center)" & vbNewLine &_
  vpmKeyName(keyInsertCoin4) & vbTab & "Insert Coin #4 (fourth)" & vbNewLine &_
  vpmKeyName(keyFront) & vbTab & "Tournament Start" & vbNewLine &_
  vpmKeyName(keyCancel) & vbTab & "Service Back" & vbNewLine &_
  vpmKeyName(keyDown) & vbTab & "Service Minus (volume down in attract mode)" & vbNewLine &_
  vpmKeyName(keyUp) & vbTab & "Service Plus (volume up in attract mode)" & vbNewLine &_
  vpmKeyName(keyEnter) & vbTab & "Service Select (opens the menu)" & vbNewLine &_
  vpmKeyName(keySlamDoorHit) & vbTab & "Slam Tilt" & vbNewLine &_
  vpmKeyName(keyCoinDoor) & vbTab & "Open/Close Coin Door (must be CLOSED for coil power)"

' Dip Switch / Options Menu. The game reads the CPU board's eight DIP switches; its settings
' (pricing, country, volume and the rest) are in its service menu
Private Sub spike1ShowDips
	If Not IsObject(vpmDips) Then ' First time
		Set vpmDips = New cvpmDips
		With vpmDips
			.AddForm 100, 160, "DIP Switches"
			.AddChk 0, 0, 80, Array("DIP 1", &H01, "DIP 2", &H02, "DIP 3", &H04, "DIP 4", &H08,_
									"DIP 5", &H10, "DIP 6", &H20, "DIP 7", &H40, "DIP 8", &H80)
		End With
	End If
	vpmDips.ViewDips
End Sub
Set vpmShowDips = GetRef("spike1ShowDips")
Private vpmDips

' A flipper button leaf: its switch, and the fast flip of its flipper (UseSolenoids = 2)
Private Sub spike1FlipLeaf(ByVal aUpper, ByVal aRight, ByVal aEnabled)
	Dim sw
	If aUpper Then
		If aRight Then sw = swURFlip Else sw = swULFlip
		If sw = 0 Then Exit Sub
		If aRight Then vpmFlips.FlipUR aEnabled Else vpmFlips.FlipUL aEnabled
	Else
		If aRight Then sw = swLRFlip Else sw = swLLFlip
		If sw = 0 Then Exit Sub
		If aRight Then vpmFlips.FlipR aEnabled Else vpmFlips.FlipL aEnabled
	End If
	Controller.Switch(sw) = aEnabled
End Sub

' Keyboard handlers
Function vpmKeyDown(ByVal keycode)
	vpmKeyDown = True ' assume we handle the key
	Dim stagedFlipperL, stagedFlipperR
	If keyStagedFlipperL & "" = "" Then stagedFlipperL = -1 Else stagedFlipperL = StagedLeftFlipperKey ' "" disables staged flipper processing
	If keyStagedFlipperR & "" = "" Then stagedFlipperR = -1 Else stagedFlipperR = StagedRightFlipperKey
	With Controller
		Select Case keycode
			Case LeftFlipperKey
				spike1FlipLeaf False, False, True : vpmKeyDown = False
				If Not Spike1StagedFlippers Or stagedFlipperL = -1 Or keycode = stagedFlipperL Then spike1FlipLeaf True, False, True
			Case RightFlipperKey
				spike1FlipLeaf False, True, True : vpmKeyDown = False
				If Not Spike1StagedFlippers Or stagedFlipperR = -1 Or keycode = stagedFlipperR Then spike1FlipLeaf True, True, True
			Case stagedFlipperL  If Spike1StagedFlippers Then spike1FlipLeaf True, False, True
			Case stagedFlipperR  If Spike1StagedFlippers Then spike1FlipLeaf True, True, True
			Case keyInsertCoin1  If swCoin1 <> 0 Then vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin1'" : If Not IsEmpty(Eval("SCoin")) Then Playsound SCoin
			Case keyInsertCoin2  If swCoin2 <> 0 Then vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin2'" : If Not IsEmpty(Eval("SCoin")) Then Playsound SCoin
			Case keyInsertCoin3  If swCoin3 <> 0 Then vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin3'" : If Not IsEmpty(Eval("SCoin")) Then Playsound SCoin
			Case keyInsertCoin4  If swCoin4 <> 0 Then vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin4'" : If Not IsEmpty(Eval("SCoin")) Then Playsound SCoin
			Case StartGameKey	 If swStartButton <> 0 Then .Switch(swStartButton) = True
			Case keyFront		 If swTournament <> 0 Then .Switch(swTournament) = True
			Case keyCancel		 .Switch(swCancel)		= True
			Case keyDown		 .Switch(swDown)		= True
			Case keyUp			 .Switch(swUp)			= True
			Case keyEnter		 .Switch(swEnter)		= True
			Case keySlamDoorHit	 If swSlamTilt <> 0 Then .Switch(swSlamTilt) = True
			' The door switch reads closed when set: a "closed" door is Switch = True
			Case keyCoinDoor	 If toggleKeyCoinDoor Then .Switch(swCoinDoor) = Not .Switch(swCoinDoor) Else .Switch(swCoinDoor) = inverseKeyCoinDoor
			Case keyBangBack	 vpmNudge.DoMechTilt
			Case Else			 vpmKeyDown = False
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
				spike1FlipLeaf False, False, False : vpmKeyUp = False
				If Not Spike1StagedFlippers Or stagedFlipperL = -1 Or keycode = stagedFlipperL Then spike1FlipLeaf True, False, False
			Case RightFlipperKey
				spike1FlipLeaf False, True, False : vpmKeyUp = False
				If Not Spike1StagedFlippers Or stagedFlipperR = -1 Or keycode = stagedFlipperR Then spike1FlipLeaf True, True, False
			Case stagedFlipperL  If Spike1StagedFlippers Then spike1FlipLeaf True, False, False
			Case stagedFlipperR  If Spike1StagedFlippers Then spike1FlipLeaf True, True, False
			Case StartGameKey	 If swStartButton <> 0 Then .Switch(swStartButton) = False
			Case keyFront		 If swTournament <> 0 Then .Switch(swTournament) = False
			Case keyCancel		 .Switch(swCancel)		= False
			Case keyDown		 .Switch(swDown)		= False
			Case keyUp			 .Switch(swUp)			= False
			Case keyEnter		 .Switch(swEnter)		= False
			Case keySlamDoorHit	 If swSlamTilt <> 0 Then .Switch(swSlamTilt) = False
			Case keyCoinDoor	 If toggleKeyCoinDoor = False Then .Switch(swCoinDoor) = Not inverseKeyCoinDoor
			Case keyShowOpts	 .Pause = True : vpmShowOptions : .Pause = False
			Case keyShowKeys	 .Pause = True : vpmShowHelp : .Pause = False
			Case keyAddBall		 .Pause = True : vpmAddBall	 : .Pause = False
			Case keyReset		 .Stop : BeginModal : .Run : vpmTimer.Reset : EndModal
			Case keyFrame		 .LockDisplay = Not .LockDisplay
			Case keyDoubleSize	 .DoubleSize  = Not .DoubleSize
			Case Else			 vpmKeyUp = False
		End Select
	End With
End Function
