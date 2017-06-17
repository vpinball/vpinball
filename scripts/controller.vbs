'***Controller.vbs version 1.2***'
'
'by arngrim
'
'This script was written to have a generic way to define a controller, no matter if the table is EM or SS based.
'It will also try to load the B2S.Server and if it is not present (or forced off),
'just the standard VPinMAME.Controller is loaded for SS generation games, or no controller for EM ones.
'
'At the first launch of a table using Controller.vbs, it will write into the registry with these default values
'ForceDisableB2S = 0
'DOFContactors   = 2
'DOFKnocker      = 2
'DOFChimes       = 2
'DOFBell         = 2
'DOFGear         = 2
'DOFShaker       = 2
'DOFFlippers     = 2
'DOFTargets      = 2
'DOFDropTargets  = 2
'
'Note that the value can be 0,1 or 2 (0 enables only digital sound, 1 only DOF and 2 both)
'
'If B2S.Server is setup but one doesn't want to use it, one should change the registry entry for ForceDisableB2S to 1
'
'
'Table script usage:
'
'This needs to be added on top of the script on both SS and EM tables:
'
'  On Error Resume Next
'  ExecuteGlobal GetTextFile("Controller.vbs")
'  If Err Then MsgBox "Unable to open Controller.vbs. Ensure that it is in the same folder as this table."
'  On Error Goto 0
'
'In addition the name of the rom (or the fake rom name for EM tables) is needed, because we need it for B2S (and loading VPM):
'
'  cGameName = "rom_name"
'
'For SS tables, the traditional LoadVPM method must be -removed- from the script
'as it is fully integrated into this script (leave the actual call in the script, of course),
'so search for something like this in the table script and -comment out or delete-:
'
'  Sub LoadVPM(VPMver, VBSfile, VBSver)
'    On Error Resume Next
'    If ScriptEngineMajorVersion <5 Then MsgBox "VB Script Engine 5.0 or higher required"
'    ExecuteGlobal GetTextFile(VBSfile)
'    If Err Then MsgBox "Unable to open " & VBSfile & ". Ensure that it is in the same folder as this table. " & vbNewLine & Err.Description
'
'    Set Controller = CreateObject("B2S.Server")
'    'Set Controller = CreateObject("VPinMAME.Controller")
'
'    If Err Then MsgBox "Can't Load VPinMAME." & vbNewLine & Err.Description
'    If VPMver> "" Then If Controller.Version <VPMver Or Err Then MsgBox "VPinMAME ver " & VPMver & " required."
'    If VPinMAMEDriverVer <VBSver Or Err Then MsgBox VBSFile & " ver " & VBSver & " or higher required."
'    On Error Goto 0
'  End Sub
'
'For SS tables with bad/outdated support by B2S Server (unsupported solenoids, lamps) one can call:
'
'  LoadVPMALT
'
'For EM tables, in the table_init, call:
'
'  LoadEM
'
'For PROC tables, in the table_init, call:
'
'  LoadPROC
'
'Finally, all calls to the B2S.Server Controller properties must be surrounded by a B2SOn check, so for example:
'
'  If B2SOn Then Controller.B2SSetGameOver 1
'
'Or "If ... End If" for multiple script lines that feature the B2S.Server Controller properties, for example:
'
'  If B2SOn Then
'    Controller.B2SSetTilt 0
'    Controller.B2SSetCredits Credits
'    Controller.B2SSetGameOver 1
'  End If
'
'That's all :)
'
'
'Optionally, if one wants to add the automatic ability to mute sounds and switch to DOF calls instead
'(based on the toy configuration that is set at the first run of a table), one can use three variants:
'
'For SS tables:
'
'  PlaySound SoundFX("sound", DOF_toy_category)
'
'If the specific DOF_toy_category (knocker, chimes, etc) is set to 1 in the Controller.txt,
'it will not play the sound but play "" instead.
'
'For EM tables, usually DOF calls are scripted and directly linked with a sound, so SoundFX and DOF can be combined to one method:
'
'  PlaySound SoundFXDOF("sound", DOFevent, State, DOF_toy_category)
'
'If the specific DOF_toy_category (knocker, chimes, etc) is set to 1 in the Controller.txt,
'it will not play the sound but just trigger the DOF call instead.
'
'For pure DOF calls without any sound (lights for example), the DOF method can be used:
'
'  DOF(DOFevent, State)
'

Const directory = "HKEY_CURRENT_USER\SOFTWARE\Visual Pinball\Controller\"
Dim objShell
Dim PopupMessage
Dim B2SController
Dim Controller
Const DOFContactors = 1
Const DOFKnocker = 2
Const DOFChimes = 3
Const DOFBell = 4
Const DOFGear = 5
Const DOFShaker = 6
Const DOFFlippers = 7
Const DOFTargets = 8
Const DOFDropTargets = 9
Const DOFOff = 0
Const DOFOn = 1
Const DOFPulse = 2

Dim DOFeffects(9)
Dim B2SOn
Dim B2SOnALT

Sub LoadEM
	LoadController("EM")
End Sub

Sub LoadPROC(VPMver, VBSfile, VBSver)
	LoadVBSFiles VPMver, VBSfile, VBSver
	LoadController("PROC")
End Sub

Sub LoadVPM(VPMver, VBSfile, VBSver)
	LoadVBSFiles VPMver, VBSfile, VBSver
	LoadController("VPM")
End Sub

'This is used for tables that need 2 controllers to be launched, one for VPM and the second one for B2S.Server
'Because B2S.Server can't handle certain solenoid or lamps, we use this workaround to communicate to B2S.Server and DOF
'By scripting the effects using DOFAlT and SoundFXDOFALT and B2SController
Sub LoadVPMALT(VPMver, VBSfile, VBSver)
	LoadVBSFiles VPMver, VBSfile, VBSver
	LoadController("VPMALT")
End Sub

Sub LoadVBSFiles(VPMver, VBSfile, VBSver)
	On Error Resume Next
	If ScriptEngineMajorVersion < 5 Then MsgBox "VB Script Engine 5.0 or higher required"
	ExecuteGlobal GetTextFile(VBSfile)
	If Err Then MsgBox "Unable to open " & VBSfile & ". Ensure that it is in the same folder as this table. " & vbNewLine & Err.Description	
	InitializeOptions
End Sub

Sub LoadVPinMAME
	Set Controller = CreateObject("VPinMAME.Controller")
	If Err Then MsgBox "Can't load VPinMAME." & vbNewLine & Err.Description
	If VPMver > "" Then If Controller.Version < VPMver Or Err Then MsgBox "VPinMAME ver " & VPMver & " required."
	If VPinMAMEDriverVer < VBSver Or Err Then MsgBox VBSFile & " ver " & VBSver & " or higher required."
	On Error Goto 0
End Sub

'Try to load b2s.server and if not possible, load VPinMAME.Controller instead.
'The user can put a value of 1 for ForceDisableB2S, which will force to load VPinMAME or no controller for EM tables.
'Also defines the array of toy categories that will either play the sound or trigger the DOF effect.
Sub LoadController(TableType)
	Dim FileObj
	Dim DOFConfig
	Dim TextStr2
	Dim tempC
	Dim count
	Dim ISDOF
	Dim Answer
	
	B2SOn = False
	B2SOnALT = False
	tempC = 0
	on error resume next
	Set objShell = CreateObject("WScript.Shell")
	objShell.RegRead(directory & "ForceDisableB2S")
	If Err.number <> 0 Then
		PopupMessage = "This latest version of Controller.vbs stores its settings in the registry. To adjust the values, you must use VP 10.2 (or newer) and setup your configuration in the DOF section of the -Keys, Nudge and DOF- dialog of Visual Pinball."
		objShell.RegWrite directory & "ForceDisableB2S",0, "REG_DWORD"
		objShell.RegWrite directory & "DOFContactors",2, "REG_DWORD"
		objShell.RegWrite directory & "DOFKnocker",2, "REG_DWORD"
		objShell.RegWrite directory & "DOFChimes",2, "REG_DWORD"
		objShell.RegWrite directory & "DOFBell",2, "REG_DWORD"
		objShell.RegWrite directory & "DOFGear",2, "REG_DWORD"
		objShell.RegWrite directory & "DOFShaker",2, "REG_DWORD"
		objShell.RegWrite directory & "DOFFlippers",2, "REG_DWORD"
		objShell.RegWrite directory & "DOFTargets",2, "REG_DWORD"
		objShell.RegWrite directory & "DOFDropTargets",2, "REG_DWORD"
		MsgBox PopupMessage
	End If
	tempC = objShell.RegRead(directory & "ForceDisableB2S")
	DOFeffects(1)=objShell.RegRead(directory & "DOFContactors")
	DOFeffects(2)=objShell.RegRead(directory & "DOFKnocker")
	DOFeffects(3)=objShell.RegRead(directory & "DOFChimes")
	DOFeffects(4)=objShell.RegRead(directory & "DOFBell")
	DOFeffects(5)=objShell.RegRead(directory & "DOFGear")
	DOFeffects(6)=objShell.RegRead(directory & "DOFShaker")
	DOFeffects(7)=objShell.RegRead(directory & "DOFFlippers")
	DOFeffects(8)=objShell.RegRead(directory & "DOFTargets")
	DOFeffects(9)=objShell.RegRead(directory & "DOFDropTargets")
	Set objShell = nothing

	If TableType = "PROC" or TableType = "VPMALT" Then
		If TableType = "PROC" Then
			Set Controller = CreateObject("VPROC.Controller")
			If Err Then MsgBox "Can't load PROC"
		Else
			LoadVPinMAME
		End If		
		If tempC = 0 Then
			On Error Resume Next
			If Controller is Nothing Then
				Err.Clear
			Else
				Set B2SController = CreateObject("B2S.Server")
				If B2SController is Nothing Then
					Err.Clear
				Else
					B2SController.B2SName = B2ScGameName
					B2SController.Run()
					On Error Goto 0
					B2SOn = True
					B2SOnALT = True
				End If
			End If
		End If
	Else
		If tempC = 0 Then
			On Error Resume Next
			Set Controller = CreateObject("B2S.Server")
			If Controller is Nothing Then
				Err.Clear
				If TableType = "VPM" Then 
					LoadVPinMAME
				End If
			Else
				Controller.B2SName = cGameName
				If TableType = "EM" Then
					Controller.Run()
				End If
				On Error Goto 0
				B2SOn = True
			End If
		Else
			If TableType = "VPM" Then 
				LoadVPinMAME
			End If
		End If
		Set DOFConfig=Nothing
		Set FileObj=Nothing
	End If
End sub

'Additional DOF sound vs toy/effect helpers:

'Mostly used for SS tables, returns the sound to be played or no sound, 
'depending on the toy category that is set to play the sound or not.
'The trigger of the DOF Effect is set at the DOF method level
'because for SS tables we usually don't need to script the DOF calls.
'Just map the Solenoid, Switches and Lamps in the ini file directly.
Function SoundFX (Sound, Effect)
	If ((Effect = 0 And B2SOn) Or DOFeffects(Effect)=1) Then
		SoundFX = ""
	Else
		SoundFX = Sound
	End If
End Function

'Mostly used for EM tables, because in EM there is most often a direct link
'between a sound and DOF Trigger, DOFevent is the ID reference of the DOF Call
'that is used in the DOF ini file and State defines if it pulses, goes on or off.
'Example based on the constants that must be present in the table script:
'SoundFXDOF("flipperup",101,DOFOn,contactors)
Function SoundFXDOF (Sound, DOFevent, State, Effect)
	If DOFeffects(Effect)=1 Then
		SoundFXDOF = ""
		DOF DOFevent, State
	ElseIf DOFeffects(Effect)=2 Then
		SoundFXDOF = Sound
		DOF DOFevent, State
	Else
		SoundFXDOF = Sound
	End If
End Function

'Method used to communicate to B2SController instead of the usual Controller
Function SoundFXDOFALT (Sound, DOFevent, State, Effect)
	If DOFeffects(Effect)=1 Then
		SoundFXDOFALT = ""
		DOFALT DOFevent, State
	ElseIf DOFeffects(Effect)=2 Then
		SoundFXDOFALT = Sound
		DOFALT DOFevent, State
	Else
		SoundFXDOFALT = Sound
	End If
End Function

'Pure method that makes it easier to call just a DOF Event.
'Example DOF 123, DOFOn
'Where 123 refers to E123 in a line in the DOF ini.
Sub DOF(DOFevent, State)
	If B2SOn Then
		If State = 2 Then
			Controller.B2SSetData DOFevent, 1:Controller.B2SSetData DOFevent, 0
		Else
			Controller.B2SSetData DOFevent, State
		End If
	End If
End Sub

'If PROC or B2SController is used, we need to pass B2S events to the B2SController instead of the usual Controller
'Use this method to pass information to DOF instead of the Sub DOF
Sub DOFALT(DOFevent, State)
	If B2SOnALT Then
		If State = 2 Then
			B2SController.B2SSetData DOFevent, 1:B2SController.B2SSetData DOFevent, 0
		Else
			B2SController.B2SSetData DOFevent, State
		End If
	End If
End Sub
