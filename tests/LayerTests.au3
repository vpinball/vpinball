#include <MsgBoxConstants.au3>
#include <WindowsConstants.au3>
#include <WinAPISys.au3>
#include <GuiTreeView.au3>
#include <GuiButton.au3>
#include <GuiEdit.au3>

Func LayersTreeView_GetSelectedItemText($mainWindowHwnd)
	Local $hwnd = ControlGetHandle($mainWindowHwnd,"","[CLASS:SysTreeView32; INSTANCE:1]")
	if $hwnd=0 Then
		MsgBox($MB_SYSTEMMODAL, "Error", "Could not find the correct control")
	Else
		Local $selectedItem = _GUICtrlTreeView_GetSelection($hwnd)
		return _GUICtrlTreeView_GetText($hwnd, $selectedItem)
	EndIf
		Return ""
EndFunc

Func LayersTreeView_GetItemCount($mainWindowHwnd)
	Local $hwnd = ControlGetHandle($mainWindowHwnd,"","[CLASS:SysTreeView32; INSTANCE:1]")
	if $hwnd=0 Then
		MsgBox($MB_SYSTEMMODAL, "Error", "Could not find the correct control")
	Else
		Return _GUICtrlTreeView_GetCount($hwnd)
	EndIf
		Return -1
EndFunc

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

Func TestLayerSelection()
	Local $hwnd = ControlGetHandle($winHandle,"","[CLASS:SysTreeView32; INSTANCE:1]")
	if $hwnd=0 Then
		MsgBox($MB_SYSTEMMODAL, "Error", "Could not find the correct control")
		Return False
	Else
		Local $hItem = _GUICtrlTreeView_GetFirstItem($hwnd)
		While 1
			if $hItem=0 Then
				MsgBox($MB_SYSTEMMODAL,"FAILED","Something is wrong")
				Return False
			EndIf
			if _GUICtrlTreeView_GetText($hwnd, $hItem)="Layer_3" Then
				ExitLoop
			EndIf
			$hItem = _GUICtrlTreeView_GetNext($hwnd, $hItem)
		WEnd
		if not $hItem=0 Then
			_GUICtrlTreeView_Expand($hwnd, $hItem, True)
			Local $hFindItem = _GUICtrlTreeView_FindItem($hwnd, "Wall20", False, $hItem)
			if $hFindItem=0 Then
				MsgBox($MB_SYSTEMMODAL, "FAILED", "Layer TreeView failed. Element 'Wall20' not selected!")
				Return False
			Else
				_GUICtrlTreeView_ClickItem($hwnd, $hFindItem)
			EndIf
		EndIf
	EndIf
	Return True
EndFunc

Func TestLayerCollapseButton()
	$testResult=False
	Local $hwnd = ControlGetHandle($winHandle,"","[CLASS:Button; INSTANCE:4]")
	if $hwnd=0 Then
		MsgBox($MB_SYSTEMMODAL, "FAILED", "Unable to locate layers collapse button")
		Return False
	EndIf
	_GUICtrlButton_Click($hwnd)
	if not LayersTreeView_GetSelectedItemText($winHandle)="Layers" Then
		MsgBox($MB_SYSTEMMODAL, "FAILED", "Collapse layers failed")
		Return False
	EndIf
	Return True
EndFunc

Func TestLayerFilter()
	$testResult=False
	Local $hwnd = ControlGetHandle($winHandle,"","[CLASS:Edit; INSTANCE:1]")
	if $hwnd=0 Then
		MsgBox($MB_SYSTEMMODAL, "FAILED", "Unable to locate layers filter edit box")
		Return False
	EndIf
	_GUICtrlEdit_SetText($hwnd, "Wall20")
	if ControlFocus($winHandle,"","[CLASS:Edit; INSTANCE:1]")=0 Then
		MsgBox($MB_SYSTEMMODAL, "FAILED", "Unable to set focus to filter edit box")
		Return False
	EndIf
	Send("{ENTER}")
	if not LayersTreeView_GetItemCount($winHandle)=3 Then
		MsgBox($MB_SYSTEMMODAL, "FAILED", "Filterting for 'Wall20' failed")
		Return False
	EndIf
	_GUICtrlEdit_SetText($hwnd, "")
	ControlFocus($winHandle,"","[CLASS:Edit; INSTANCE:1]")
	Send("{ENTER}")
	Return True
EndFunc

Func TestAddDeleteLayer()
	Local $layerCount = LayersTreeView_GetItemCount($winHandle)
	Local $hwnd = ControlGetHandle($winHandle,"","[CLASS:Edit; INSTANCE:1]")
	if $hwnd=0 Then
		MsgBox($MB_SYSTEMMODAL, "FAILED", "Unable to locate layers filter edit box")
		Return False
	EndIf
	Local $addButton = ControlGetHandle($winHandle,"","[CLASS:Button; INSTANCE:1]")
	Local $delButton = ControlGetHandle($winHandle,"","[CLASS:Button; INSTANCE:2]")
	_GUICtrlButton_Click($addButton)
	if not LayersTreeView_GetSelectedItemText($winHandle)="New Layer 0" Then
		MsgBox($MB_SYSTEMMODAL, "FAILED", "Layer TreeView failed. New layer not found")
		Return False
	EndIf
	_GUICtrlButton_Click($delButton)

	Local $diag = _WinWaitActivate("[CLASS:#32770]", "")
	if $diag = 0 Then
		MsgBox($MB_SYSTEMMODAL, "FAILED", "Confirm deletion layer dialog failed")
		Return False
	EndIf
	Local $okButton = ControlGetHandle($diag, "", "[CLASS:Button; INSTANCE:1]")
	_GUICtrlButton_Click($okButton)
	if $layerCount <> LayersTreeView_GetItemCount($winHandle) then
		MsgBox($MB_SYSTEMMODAL, "FAILED", "Deleting a layer failed");
		return False
	EndIf

	Return True
EndFunc

_AU3RecordSetup()

$winHandle = _WinWaitActivate("Visual Pinball - [Table","")

if TestLayerSelection()=True And TestLayerCollapseButton()=True And TestLayerFilter()=True And TestAddDeleteLayer()=True Then MsgBox($MB_SYSTEMMODAL,"SUCCESS","Tests ok!")



