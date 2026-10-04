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
' Williams Phantom Haus
'-------------------------
' Cabinet switches
Const swCoin1       =  1
Const swCoin2       =  2
Const swCoin3       =  4
Const swStartButton = 28
Const swCancel      = 15
Const swDown        = 26
Const swUp          = 27
Const swEnter       = 14
Const swMenu        =  7

' Keyboard Assignment
Dim CollectKey, swCollect, CollectKeyCaption
Dim LeftHoldKey, swLeftHold, LeftHoldKeyCaption
Dim MiddleHoldKey, swMiddleHold, MiddleHoldKeyCaption
Dim RightHoldKey, swRightHold, RightHoldKeyCaption
Dim LanguageKey, swLanguage, LanguageKeyCaption
Dim MultiplierKey, swMultiplier, MultiplierKeyCaption 
Dim RedKey, swRed, RedKeyCaption
Dim BlueKey, swBlue, BlueKeyCaption
Dim SuperStartKey, swSuperStart, SuperStartKeyCaption
Dim TransferKey, swTransfer, TransferKeyCaption
Dim PayoutKey, swPayout, PayoutKeyCaption

CollectKey           = 17
CollectKeyCaption    = "W"
swCollect            = 15
MultiplierKey        = 18
MultiplierKeyCaption = "E"
swMultiplier         = 25
SuperStartKey        = 19
SuperStartKeyCaption = "R"
swSuperStart         = 14

LeftHoldKey          = 21
LeftHoldKeyCaption   = "Z"	'Maybe "Y" on an English keyboard
swLeftHold           = 16
MiddleHoldKey        = 22
MiddleHoldKeyCaption = "U"
swMiddleHold         = 17
RightHoldKey         = 23
RightHoldKeyCaption  = "I"
swRightHold          = 18

RedKey               = 30
RedKeyCaption        = "A"
swRed                = 26
BlueKey              = 31
BlueKeyCaption       = "S"
swBlue               = 27

LanguageKey          = 34
LanguageKeyCaption   = "G"
swLanguage           =  8
TransferKey          = 35
TransferKeyCaption   = "H"
swTransfer           = 22

PayoutKey            = 45
PayoutKeyCaption     = "X"
swPayout             = 21

' Help Window
vpmSystemHelp = "WMS Phantom Haus keys:" & vbNewLine &_
  vpmKeyName(keyInsertCoin1) & vbTab & "Insert Coin 3" & vbNewLine &_
  vpmKeyName(keyCoinDoor) & vbTab & "Activate Menu - Hold key" & vbNewLine &_
  vpmKeyName(keyEnter) & vbTab & "Enter (Menu)" & vbNewLine &_
  vpmKeyName(keyDown) & vbTab & "Down (Menu)" & vbNewLine &_
  vpmKeyName(keyUp) & vbTab & "Up (Menu)" & vbNewLine &_
  vpmKeyName(keyCancel) & vbTab & "Escape (Menu)" & vbNewLine & vbNewLine &_
  "Gameplay:" & vbNewLine &_
  "Collect Button" & vbTab & vbTab & CollectKeyCaption & vbNewLine &_
  "Multiplier Button" & vbTab & vbTab & MultiplierKeyCaption  & vbNewLine &_
  "SuperStart Button" & vbTab & vbTab & SuperStartKeyCaption & vbNewLine &_
  "Left Hold Button" & vbTab & vbTab & LeftHoldKeyCaption & vbNewLine &_
  "Middle Hold Button" & vbTab & MiddleHoldKeyCaption & vbNewLine &_
  "Right Hold Button" & vbTab & vbTab & RightHoldKeyCaption & vbNewLine &_
  "Red Button" & vbTab & vbTab & RedKeyCaption & vbNewLine &_
  "Blue Button" & vbTab & vbTab & BlueKeyCaption & vbNewLine &_
  "Language Button" & vbTab & vbTab & LanguageKeyCaption & vbNewLine &_
  "Transfer Button" & vbTab & vbTab & TransferKeyCaption & vbNewLine &_
  "Payout Button" & vbTab & vbTab & PayoutKeyCaption

' Keyboard handlers
Function vpmKeyDown(ByVal keycode)
	vpmKeyDown = True ' assume we handle the key
	With Controller
		Select Case keycode
			Case keyInsertCoin1	 vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin1'" : Playsound SCoin
			Case keyInsertCoin2	 vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin2'" : Playsound SCoin
			Case keyInsertCoin3	 vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCoin3'" : Playsound SCoin
			Case keyEnter		 vpmTimer.AddTimer 750,"vpmTimer.PulseSw swEnter'"
			Case keyDown		 vpmTimer.AddTimer 750,"vpmTimer.PulseSw swDown'"
			Case keyUp			 vpmTimer.AddTimer 750,"vpmTimer.PulseSw swUp'"
			Case keyCancel		 vpmTimer.AddTimer 750,"vpmTimer.PulseSw swCancel'"
			Case keyCoinDoor	 .Switch(swMenu) = True
			Case StartGameKey	 .Switch(swStartButton) = True
			Case keyVPMVolume	 vpmVol
			Case CollectKey		 .Switch(swCollect) = True
			Case LeftHoldKey	 .Switch(swLeftHold) = True
			Case MiddleHoldKey	 .Switch(swMiddleHold) = True
			Case RightHoldKey	 .Switch(swRightHold) = True
			Case LanguageKey	 .Switch(swLanguage) = True
			Case MultiplierKey	 .Switch(swMultiplier) = True
			Case RedKey			 .Switch(swRed) = True
			Case BlueKey		 .Switch(swBlue) = True
			Case SuperStartKey	 .Switch(swSuperStart) = True
			Case TransferKey	 .Switch(swTransfer) = True
			Case PayoutKey		 .Switch(swPayout) = True
			Case Else			 vpmKeyDown = False
		End Select
	End With
End Function

Function vpmKeyUp(ByVal keycode)
	vpmKeyUp = True ' assume we handle the key
	With Controller
		Select Case keycode
			Case StartGameKey	 .Switch(swStartButton) = False
			Case keyCoinDoor	 .Switch(swMenu) = False
			Case keyShowOpts	 .Pause = True : vpmShowOptions : .Pause = False
			Case keyShowKeys	 .Pause = True : vpmShowHelp : .Pause = False
			Case keyReset		 .Stop : BeginModal : .Run : vpmTimer.Reset : EndModal
			Case keyFrame		 .LockDisplay = Not .LockDisplay
			Case keyDoubleSize	 .DoubleSize  = Not .DoubleSize
			Case CollectKey		 .Switch(swCollect) = False
			Case LeftHoldKey	 .Switch(swLeftHold) = False
			Case MiddleHoldKey	 .Switch(swMiddleHold) = False
			Case RightHoldKey	 .Switch(swRightHold) = False
			Case LanguageKey	 .Switch(swLanguage) = False
			Case MultiplierKey	 .Switch(swMultiplier) = False
			Case RedKey			 .Switch(swRed) = False
			Case BlueKey		 .Switch(swBlue) = False
			Case SuperStartKey	 .Switch(swSuperStart) = False
			Case TransferKey	 .Switch(swTransfer) = False
			Case PayoutKey		 .Switch(swPayout) = False
			Case Else			 vpmKeyUp = False
		End Select
	End With
End Function
