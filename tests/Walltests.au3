#include <MsgBoxConstants.au3>
#include <WindowsConstants.au3>
#include <WinAPISys.au3>
#include <GuiButton.au3>
#include <GuiEdit.au3>
#include <GuiComboBox.au3>
#include <GuiTab.au3>

Func _Au3RecordSetup()
	Opt('WinWaitDelay',100)
	Opt('WinDetectHiddenText',1)
	Opt('MouseCoordMode',0)
	Local $aResult = DllCall('User32.dll', 'int', 'GetKeyboardLayoutNameW', 'wstr', '')
	If $aResult[1] <> '00000407' Then
	  MsgBox(64, 'Warning', 'Recording has been done under a different Keyboard layout' & @CRLF & '(00000407->' & $aResult[1] & ')')
	EndIf
EndFunc

Local $winHandle

Func _WinWaitActivate($title,$text,$timeout=0)
	WinWait($title,$text,$timeout)
	If Not WinActive($title,$text) Then WinActivate($title,$text)
	local $handle = WinWaitActive($title,$text,$timeout)
	if @error Then
		MsgBox($MB_SYSTEMMODAL, "Error", "Could not find the correct window")
	EndIf
	Return $handle
EndFunc

Func TestCreateDeleteWall()
	_WinWaitActivate("Visual Pinball - [Table","")
	Local $nameEdit = ControlGetHandle($winHandle, "", "[CLASS:Edit; INSTANCE:36]")
	if $nameEdit=0 Then
		MsgBox($MB_SYSTEMMODAL, "FAILED", "Could not find the properties name edit box")
		Return False
	EndIf
	MouseClick("left",150,100,1)

	Send("{CTRLDOWN}w{CTRLUP}")		;create wall
	MouseClick("left",800,600,1)	;place wall
	Sleep(500)
	if not (_GUICtrlEdit_GetText($nameEdit)="Wall001") Then
		MsgBox($MB_SYSTEMMODAL, "FAILED", "Creating 'Wall001' failed.")
		Return False
	EndIf

	MouseClick("left",900,600,1)	;select table
	Sleep(500)
	MouseClick("left",800,600,1)	;select wall again
	Sleep(500)
	Send("{DELETE}")
	Sleep(500)
	if not (_GUICtrlEdit_GetText($nameEdit)="Table1") Then
		MsgBox($MB_SYSTEMMODAL, "FAILED", "Deleting 'Wall001' failed.")
		Return False
	EndIf
	Return True
EndFunc

Func TestWallImageMaterialSelection()
	_WinWaitActivate("Visual Pinball - [Table","")
	Local $nameEdit = ControlGetHandle($winHandle, "", "[CLASS:Edit; INSTANCE:36]")
	if $nameEdit=0 Then
		MsgBox($MB_SYSTEMMODAL, "FAILED", "Could not find the properties name edit box")
		Return False
	EndIf
	MouseClick("left",150,100,1)

	Send("{CTRLDOWN}w{CTRLUP}")		;create wall
	MouseClick("left",800,600,1)	;place wall
	Sleep(500)
	if not _GUICtrlEdit_GetText($nameEdit)="Wall001" Then
		MsgBox($MB_SYSTEMMODAL, "FAILED", "Creating 'Wall001' failed.")
		Return False
	EndIf

	$topImage = ControlGetHandle($winHandle, "", "[CLASS:ComboBox; INSTANCE:1]")
	$topMaterial = ControlGetHandle($winHandle, "", "[CLASS:ComboBox; INSTANCE:2]")
	$sideImage = ControlGetHandle($winHandle, "", "[CLASS:ComboBox; INSTANCE:3]")
	$sideMaterial = ControlGetHandle($winHandle, "", "[CLASS:ComboBox; INSTANCE:4]")
	$slingMaterial = ControlGetHandle($winHandle, "", "[CLASS:ComboBox; INSTANCE:5]")
	$displayImageCheckbox = ControlGetHandle($winHandle, "", "[CLASS:Button; INSTANCE:6]")
	$animateSlingCheckbox = ControlGetHandle($winHandle, "", "[CLASS:Button; INSTANCE:8]")
	_GUICtrlComboBox_ShowDropDown($topImage, True)
	_GUICtrlComboBox_SetCurSel($topImage, 22)
	Sleep(100)
	_GUICtrlButton_Click($displayImageCheckbox)

	_GUICtrlComboBox_ShowDropDown($topMaterial, True)
	_GUICtrlComboBox_SetCurSel($topMaterial, 4)
	Sleep(100)

	_GUICtrlComboBox_ShowDropDown($sideImage, True)
	_GUICtrlComboBox_SetCurSel($sideImage, 15)
	Sleep(100)

	_GUICtrlComboBox_ShowDropDown($sideMaterial, True)
	_GUICtrlComboBox_SetCurSel($sideMaterial, 8)
	Sleep(100)

	_GUICtrlComboBox_ShowDropDown($slingMaterial, True)
	_GUICtrlComboBox_SetCurSel($slingMaterial, 2)
	Sleep(100)
	_GUICtrlButton_Click($animateSlingCheckbox)
	MouseClick("left",900,600,1)	;select table
	Sleep(500)
	MouseClick("left",800,600,1)	;select wall again
	Sleep(500)

	;get handles again because they changed when changing elements
	$topImage = ControlGetHandle($winHandle, "", "[CLASS:ComboBox; INSTANCE:1]")
	$topMaterial = ControlGetHandle($winHandle, "", "[CLASS:ComboBox; INSTANCE:2]")
	$sideImage = ControlGetHandle($winHandle, "", "[CLASS:ComboBox; INSTANCE:3]")
	$sideMaterial = ControlGetHandle($winHandle, "", "[CLASS:ComboBox; INSTANCE:4]")
	$slingMaterial = ControlGetHandle($winHandle, "", "[CLASS:ComboBox; INSTANCE:5]")
	Local $txt=""
	$idx = _GUICtrlComboBox_GetCurSel($topImage)
	_GUICtrlComboBox_GetLBText($topImage, $idx, $txt)
	if Not ($txt="inches") Then
		MsgBox($MB_SYSTEMMODAL, "FAILED","TopImage selection is wrong")
		Return False
	EndIf
	$idx = _GUICtrlComboBox_GetCurSel($topMaterial)
	$txt=""
	_GUICtrlComboBox_GetLBText($topMaterial, $idx, $txt)
	if Not ($txt="Metal") Then
		MsgBox($MB_SYSTEMMODAL, "FAILED","TopMaterial selection is wrong")
		Return False
	EndIf
	$idx = _GUICtrlComboBox_GetCurSel($sideImage)
	$txt=""
	_GUICtrlComboBox_GetLBText($sideImage, $idx, $txt)
	if Not ($txt="emkicker-t1") Then
		MsgBox($MB_SYSTEMMODAL, "FAILED","SideImage selection is wrong")
		Return False
	EndIf
	$idx = _GUICtrlComboBox_GetCurSel($sideMaterial)
	$txt=""
	_GUICtrlComboBox_GetLBText($sideMaterial, $idx, $txt)
	if Not ($txt="Opacity80") Then
		MsgBox($MB_SYSTEMMODAL, "FAILED","SideImage selection is wrong")
		Return False
	EndIf

	Send("{DELETE}")
	Sleep(500)
	if not (_GUICtrlEdit_GetText($nameEdit)="Table1") Then
		MsgBox($MB_SYSTEMMODAL, "FAILED", "Deleting 'Wall001' failed.")
		Return False
	EndIf
	Return True
EndFunc

Func TestPhysics()
	_WinWaitActivate("Visual Pinball - [Table","")
	Local $nameEdit = ControlGetHandle($winHandle, "", "[CLASS:Edit; INSTANCE:36]")
	if $nameEdit=0 Then
		MsgBox($MB_SYSTEMMODAL, "FAILED", "Could not find the properties name edit box")
		Return False
	EndIf
	MouseClick("left",150,100,1)

	Send("{CTRLDOWN}w{CTRLUP}")		;create wall
	MouseClick("left",800,600,1)	;place wall
	Sleep(500)
	if not _GUICtrlEdit_GetText($nameEdit)="Wall001" Then
		MsgBox($MB_SYSTEMMODAL, "FAILED", "Creating 'Wall001' failed.")
		Return False
	EndIf

	$tab = ControlGetHandle($winHandle, "", "[CLASS:SysTabControl32; INSTANCE:3]")
	_GUICtrlTab_ActivateTab($tab, 1)

	$hitEventCheckbox = ControlGetHandle($winHandle, "", "[CLASS:Button; INSTANCE:5]")
	$hitThreshold = ControlGetHandle($winHandle, "", "[CLASS:Edit; INSTANCE:2]")
	if ControlCommand($winHandle, "", "Edit2", "IsEnabled")=1 And ControlCommand($winHandle, "", "Button5", "IsChecked")=0 Then
		MsgBox($MB_SYSTEMMODAL,"FAILED","Hit Threshold edit box is editabled but hit event checkbox isn't checked!")
		Return False
	EndIf
	_GUICtrlButton_Click($hitEventCheckbox)
	if ControlCommand($winHandle, "", "Edit2", "IsEnabled")=0 And ControlCommand($winHandle, "", "Button5", "IsChecked")=1 Then
		MsgBox($MB_SYSTEMMODAL,"FAILED","Hit Threshold edit box isn't editabled but hit event checkbox is checked!")
		Return False
	EndIf
	$collidableCheckbox = ControlGetHandle($winHandle, "", "[CLASS:Button; INSTANCE:7]")
	$elasticity = ControlGetHandle($winHandle, "", "[CLASS:Button; INSTANCE:5]")
	if ControlCommand($winHandle, "", "Edit5", "IsEnabled")=0 And ControlCommand($winHandle, "", "Button7", "IsChecked")=1 Then
		MsgBox($MB_SYSTEMMODAL,"FAILED","Elasticity box isn't editabled but collidable checkbox is checked!")
		Return False
	EndIf
	_GUICtrlButton_Click($collidableCheckbox)
	if ControlCommand($winHandle, "", "Edit5", "IsEnabled")=1 And ControlCommand($winHandle, "", "Button7", "IsChecked")=0 Then
		MsgBox($MB_SYSTEMMODAL,"FAILED","Elasticity edit box is editabled but collidable checkbox isn't checked!")
		Return False
	EndIf
	_GUICtrlButton_Click($collidableCheckbox)
	if ControlCommand($winHandle, "", "Edit5", "IsEnabled")=0 And ControlCommand($winHandle, "", "Button7", "IsChecked")=1 Then
		MsgBox($MB_SYSTEMMODAL,"FAILED","Elasticity box isn't editabled but collidable checkbox is checked!")
		Return False
	EndIf

	$overwriteMaterialButton = ControlGetHandle($winHandle, "", "[CLASS:Button; INSTANCE:9]")
	$physicsMaterial = ControlGetHandle($winHandle, "", "[CLASS:ComboBox; INSTANCE:1]")
	if ControlCommand($winHandle, "", "Combobox1", "IsEnabled")=1 And ControlCommand($winHandle, "", "Button9", "IsChecked")=0 Then
		MsgBox($MB_SYSTEMMODAL,"FAILED","Physics Material is enabled but overwrite material combo box isn't checked!")
		Return False
	EndIf
	_GUICtrlButton_Click($overwriteMaterialButton)
	if ControlCommand($winHandle, "", "Combobox1", "IsEnabled")=0 And ControlCommand($winHandle, "", "Button9", "IsChecked")=1 Then
		MsgBox($MB_SYSTEMMODAL,"FAILED","Physics Material isn't enabled but overwrite material combo box is checked!")
		Return False
	EndIf
	_GUICtrlComboBox_ShowDropDown($physicsMaterial, True)
	_GUICtrlComboBox_SetCurSel($physicsMaterial, 8)
	Sleep(100)
	_GUICtrlButton_Click($overwriteMaterialButton)

	MouseClick("left",900,600,1)	;select table
	Sleep(500)
	MouseClick("left",800,600,1)	;select wall again
	Sleep(500)
	$physicsMaterial = ControlGetHandle($winHandle, "", "[CLASS:ComboBox; INSTANCE:1]")
	Local $txt=""
	$idx = _GUICtrlComboBox_GetCurSel($physicsMaterial)
	_GUICtrlComboBox_GetLBText($physicsMaterial, $idx, $txt)
	if Not ($txt="Opacity80") Then
		MsgBox($MB_SYSTEMMODAL, "FAILED","Physics Material selection is wrong")
		Return False
	EndIf
	Send("{DELETE}")
	Sleep(500)
	if not (_GUICtrlEdit_GetText($nameEdit)="Table1") Then
		MsgBox($MB_SYSTEMMODAL, "FAILED", "Deleting 'Wall001' failed.")
		Return False
	EndIf
	Return True
EndFunc
_AU3RecordSetup()

$winHandle = _WinWaitActivate("Visual Pinball - [Table","")

if TestCreateDeleteWall()=True And TestWallImageMaterialSelection()=True And TestPhysics()=True Then MsgBox($MB_SYSTEMMODAL,"SUCCESS", "Tests ok!")



