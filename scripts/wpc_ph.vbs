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

' Help Window
vpmSystemHelp = "WMS Phantom Haus keys:" & vbNewLine &_
  vpmKeyName(keyInsertCoin1) & vbTab & "Insert Coin #1" & vbNewLine &_
  vpmKeyName(keyInsertCoin2) & vbTab & "Insert Coin #2" & vbNewLine &_
  vpmKeyName(keyInsertCoin3) & vbTab & "Insert Coin #4" & vbNewLine &_
  vpmKeyName(keyCoinDoor) & vbTab & "Activate Menu - Hold key" & vbNewLine &_
  vpmKeyName(keyEnter) & vbTab & "Enter (Coin Door)" & vbNewLine &_
  vpmKeyName(keyDown) & vbTab & "Down (Coin Door)" & vbNewLine &_
  vpmKeyName(keyUp) & vbTab & "Up (Coin Door)" & vbNewLine &_
  vpmKeyName(keyCancel) & vbTab & "Escape (Coin Door)"

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
			Case Else			 vpmKeyUp = False
		End Select
	End With
End Function
