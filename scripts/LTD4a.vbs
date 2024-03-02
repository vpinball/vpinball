'Last Updated in VBS v3.58
' Written by Wiesshund to run Haunted Hotel ROM, feel free to improve this

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
' LTD System 4 Data
'-------------------------
' Cabinet switches
Const swSelfTest    = 9
Const swTilt        = -6
Const swCoin1       = 13
Const swCoin2       = 13
Const swStartButton = 14
Const swEnter       = 10
Const swDown        = 11

Const swURFlip      = 3
Const swULFlip      = 4
Const swLRFlip      = 84
Const swLLFlip      = 82

' Help Window
vpmSystemHelp = "LTD System 4 keys:" & vbNewLine &_
  vpmKeyName(keyInsertCoin2) & vbTab & "Insert Coin #1" & vbNewLine &_
  vpmKeyName(keyInsertCoin3) & vbTab & "Insert Coin #2" & vbNewLine &_
  vpmKeyName(keyEnter) & vbTab & "Enter (Coin Door)" & vbNewLine &_
  vpmKeyName(keySelfTest) & vbTab & "Self Test"

' Dip Switch / Options Menu
Private Sub LTDShowDips
	If Not IsObject(vpmDips) Then ' First time
		Set vpmDips = New cvpmDips
		With vpmDips
			.AddForm 150, 245, "DIP Switches"
			.AddFrame 0, 0, 60, "", 0,_
			  Array("DIP  1",&H00000001,"DIP  2",&H00000002,"DIP  3",&H00000004,"DIP  4",&H00000008,_
			        "DIP  5",&H00000010,"DIP  6",&H00000020,"DIP  7",&H00000040,"DIP  8",&H00000080,_
			        "DIP  9",&H00000100,"DIP 10",&H00000200,"DIP 11",&H00000400,"DIP 12",&H00000800,_
			        "DIP 13",&H00001000,"DIP 14",&H00002000,"DIP 15",&H00004000,"DIP 16",32768)
			.AddFrame 80,0, 60, "", 0,_
			  Array("DIP 17",&H00010000,"DIP 18",&H00020000,"DIP 19",&H00040000,"DIP 20",&H00080000,_
			        "DIP 21",&H00100000,"DIP 22",&H00200000,"DIP 23",&H00400000,"DIP 24",&H00800000,_
			        "DIP 25",&H01000000,"DIP 26",&H02000000,"DIP 27",&H04000000,"DIP 28",&H08000000,_
			        "DIP 29",&H10000000,"DIP 30",&H20000000,"DIP 31",&H40000000,"DIP 32",&H80000000)
		End With
	End If
	vpmDips.ViewDips
End Sub
Set vpmShowDips = GetRef("LTDShowDips")
Private vpmDips

' Keyboard handlers
Function vpmKeyDown(ByVal keycode)
	vpmKeyDown = True ' Assume we handle the key
	With Controller
		Select Case keycode
			Case RightFlipperKey .Switch(swLRFlip) = True : vpmKeyDown = False
				If keycode = RightMagnaSave then 
					.Switch(swURFlip) = True 
					vpmKeyDown = False
				End If
			Case LeftFlipperKey  .Switch(swLLFlip) = True : vpmKeyDown = False
				If keycode = LeftMagnaSave Then
					.Switch(swULFlip) = True
					vpmKeyDown = False
				End If
			Case RightMagnaSave .Switch(swURFlip) = True : vpmKeyDown = False
			Case LeftMagnaSave  .Switch(swULFlip) = True : vpmKeyDown = False
			Case keyStagedFlipperL .Switch(swULFlip) = True : vpmKeyDown = False
			Case keyStagedFlipperR .Switch(swURFlip) = True : vpmKeyDown = False
			Case keyInsertCoin1  vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin1'" : If Not IsEmpty(Eval("SCoin")) Then Playsound SCoin
			Case keyInsertCoin2  vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin2'" : If Not IsEmpty(Eval("SCoin")) Then Playsound SCoin
			Case StartGameKey    .Switch(swStartButton) = True
			Case keySelfTest     .Switch(swSelfTest)    = True
			Case keyEnter        .Switch(swEnter)       = True
			Case keyBangBack     vpmNudge.DoNudge   0, 6
			Case LeftTiltKey     vpmNudge.DoNudge  75, 2
			Case RightTiltKey    vpmNudge.DoNudge 285, 2
			Case CenterTiltKey   vpmNudge.DoNudge   0, 2
			Case keyVPMVolume    vpmVol
			Case Else            vpmKeyDown = False
		End Select
	End With
End Function

Function vpmKeyUp(ByVal keycode)
	vpmKeyUp = True ' Assume we handle the key
	With Controller
		Select Case keycode
			Case RightFlipperKey .Switch(swLRFlip) = False : vpmKeyUp = False
				If keycode = RightMagnaSave then 
					.Switch(swURFlip) = False 
					vpmKeyUp = False
				End If
			Case LeftFlipperKey  .Switch(swLLFlip) = False : vpmKeyUp = False
				If keycode = LeftMagnaSave Then
					.Switch(swULFlip) = False
					vpmKeyUp = False
				End If
			Case RightMagnaSave .Switch(swURFlip) = False : vpmKeyUp = False
			Case LeftMagnaSave  .Switch(swULFlip) = False : vpmKeyUp = False
			Case keyStagedFlipperL .Switch(swULFlip) = False : vpmKeyUp = False
			Case keyStagedFlipperR .Switch(swURFlip) = False : vpmKeyUp = False
			Case StartGameKey    .Switch(swStartButton) = False
			Case keySelfTest     .Switch(swSelfTest)    = False
			Case keyEnter        .Switch(swEnter)       = False
			Case keyShowOpts     .Pause = True : vpmShowOptions : .Pause = False
			Case keyShowKeys     .Pause = True : vpmShowHelp : .Pause = False
			Case keyAddBall      .Pause = True : vpmAddBall  : .Pause = False
			Case keyShowDips     If IsObject(vpmShowDips) Then .Pause = True : vpmShowDips : .Pause = False
			Case keyReset        .Stop : BeginModal : .Run : vpmTimer.Reset : EndModal
			Case keyFrame        .LockDisplay = Not .LockDisplay
			Case keyDoubleSize   .DoubleSize  = Not .DoubleSize
			Case Else            vpmKeyUp = False
		End Select
	End With
End Function
