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
' Taito Data
'-------------------------
' Cabinet switches
Const swCoin1          =   0
Const swStartButton    =  10
Const swSlamTilt       =  20
Const swService	       =  40
Const swCoinDoor       =  50
Const swBoxDoor	       =  60
Const swCoin2          =  70
Const swDiagnostics    =   7
Const swStatistics     =  17
Const swAdjustments    =  27
Const swConfigurations =  37
Const swEnter          =  47
Const swSoundDiag      =  57

Private swStartButtonX,swCoinDoorX,swSlamTiltX
On Error Resume Next
If swStartButton = 10 Or Err Then swStartButtonX = 10 Else swStartButtonX = swStartButton
If swCoinDoor    = 50 Or Err Then swCoinDoorX    = 50 Else swCoinDoorX    = swCoinDoor
If swSlamTilt    = 20 Or Err Then swSlamTiltX    = 20 Else swSlamTiltX    = swSlamTilt
On Error Goto 0

Const swLRFlip         = 99
Const swLLFlip         = 119

' Help Window
vpmSystemHelp = "Taito keys:"& vbNewLine &_
  vpmKeyName(keyInsertCoin1) & vbTab & "Insert Coin #1"    & vbNewLine &_
  vpmKeyName(keyInsertCoin2) & vbTab & "Insert Coin #2"    & vbNewLine &_
  vpmKeyName(keyHiScoreReset)& vbTab & "Statistics"	     & vbNewLine &_
  vpmKeyName(keySelfTest)    & vbTab & "Service Menu"      & vbNewLine &_
  vpmKeyName(keyDown)        & vbTab & "Adjustments"       & vbNewLine &_
  vpmKeyName(KeyUp)	         & vbTab & "Diagnostics"       & vbNewLine &_
  vpmKeyName(keyEnter)	     & vbTab & "Enter"             & vbNewLine &_
  vpmKeyName(keyConfigurations) & vbTab & "Configurations"    & vbNewLine &_
  vpmKeyName(keySoundDiag)   & vbTab & "Sound Diagnostics" & vbNewLine &_
  vpmKeyName(keyCoinDoor)    & vbTab & "Coin Door"	     & vbNewLine &_
  vpmKeyName(keymasterenter) & vbTab & "Coin Box Door"

Private Sub TaitoShowDips
	If Not IsObject(vpmDips) Then ' First time
		Set vpmDips = New cvpmDips
		With vpmDips
	  	.AddForm  80, 0, "DIP Switches"
        	.AddFrame  0,0, 180,"Dip Format in Adjust Mode ( |B| |A| )", 0,_
	    		Array("CH0  A(1)",&H1, "CH1  A(2)",&H2, "CH2  A(4)",&H4, "CH3  A(8)",&H8,_
	        		  "CH4  B(1)",&H10,"CH5  B(2)",&H20,"CH6  B(4)",&H40,"CH7  B(8)",&H80)
		End With
	End If
	vpmDips.ViewDips
End Sub
Set vpmShowDips = GetRef("TaitoShowDips")
Private vpmDips

' Keyboard handlers
Function vpmKeyDown(ByVal keycode)
	On Error Resume Next
	vpmKeyDown = True ' assume we handle the key
	With Controller
		Select Case keycode
			Case RightFlipperKey .Switch(swLRFlip) = True : vpmKeyDown = False
			Case LeftFlipperKey  .Switch(swLLFlip) = True : vpmKeyDown = False
			Case keyInsertCoin1  vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin1'" : Playsound SCoin
			Case keyInsertCoin2  vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin1'" : Playsound SCoin
			Case keyInsertCoin3  vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin1'" : Playsound SCoin
			Case StartGameKey    .Switch(swStartButton)   = True
			Case keyUp	         .Switch(swDiagnostics)   = Not .Switch(swDiagnostics)
			Case keyHiScoreReset .Switch(swStatistics)    = Not .Switch(swStatistics)
			Case keySelfTest     .Switch(swService)       = Not .Switch(swService)
			Case keyDown         .Switch(swAdjustments)   = Not .Switch(swAdjustments)
			Case keyConfigurations .Switch(swConfigurations) = True
			Case keyEnter        .Switch(swEnter)         = True
			Case keySlamDoorHit  .Switch(swSlamTilt)      = True
			Case keyCoinDoor     If toggleKeyCoinDoor Then .Switch(swCoinDoor) = Not .Switch(swCoinDoor) Else .Switch(swCoinDoor) = Not inverseKeyCoinDoor
			Case keySoundDiag    .Switch(swSoundDiag)     = Not .Switch(swSoundDiag)
			Case keyMasterEnter  .Switch(swBoxDoor)       = Not .Switch(swBoxDoor)
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
			Case StartGameKey    .Switch(swStartButton)   = False
			Case keySlamDoorHit  .Switch(swSlamTilt)      = False
			Case keyCoinDoor     If toggleKeyCoinDoor = False Then .Switch(swCoinDoor) = inverseKeyCoinDoor
			Case keyShowOpts     .Pause = True : .ShowOptsDialog GetPlayerHWnd : .Pause = False
			Case keyShowKeys     .Pause = True : vpmShowHelp : .Pause = False
			Case keyShowDips     If IsObject(vpmShowDips) Then .Pause = True : vpmShowDips : .Pause = False
			Case keyConfigurations .Switch(swConfigurations)= False
			Case keyEnter        .Switch(swEnter)         = False
			Case keyAddBall      .Pause = True : vpmAddBall  : .Pause = False
			Case keyReset        .Stop : BeginModal : .Run : vpmTimer.Reset : EndModal
			Case keyFrame        .LockDisplay = Not .LockDisplay
			Case keyDoubleSize   .DoubleSize  = Not .DoubleSize
			Case Else            vpmKeyUp = False
		End Select
	End With
	On Error Goto 0
End Function
