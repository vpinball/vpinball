'***Controller.vbs version 1.0***'
'
'by arngrim
'
'This script was written to have a generic way to define a controller, no matter if the table is EM or SS based.
'It will also try to load the B2S.Server and if it is not present (or forced off),
'just the standard VPinMAME.Controller is loaded for SS generation games, or no controller for EM ones.
'
'At the first launch of a table using Controller.vbs, it will launch a popup window to setup whether to use DOF, and if selected,
'there will be additional options if one uses a certain type of toy, and to determine whether the sounds will be replaced by DOF calls (mostly for EM tables).
'
'Controller.vbs will also create a Controller.txt inside the user folder of VP, here an example of its content:
'
'ForceDisableB2S=0
'UseDOFcontactors=1
'UseDOFKnocker=1
'UseDOFChimes=0
'UseDOFBell=1
'UseDOFGear=1
'UseDOFShaker=1
'
'If B2S.Server is setup but one doesn't want to use it, one should change the first line to ForceDisableB2S=1
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
'For EM tables, in the table_init, call:
'
'  LoadEM
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


Const ControllerFile = "Controller.txt"
Dim PopupMessage
Dim B2SController
Dim Controller
Const DOFContactors = 1
Const DOFKnocker = 2
Const DOFChimes = 3
Const DOFBell = 4
Const DOFGear = 5
Const DOFShaker = 6

Const DOFOff = 0
Const DOFOn = 1
Const DOFPulse = 2

Dim DOFeffects(6)
Dim B2SOn

Sub LoadEM
	LoadController("EM")
End Sub

Sub LoadPROC(VPMver, VBSfile, VBSver)
	On Error Resume Next
	If ScriptEngineMajorVersion < 5 Then MsgBox "VB Script Engine 5.0 or higher required"
	ExecuteGlobal GetTextFile(VBSfile)
	If Err Then MsgBox "Unable to open " & VBSfile & ". Ensure that it is in the same folder as this table. " & vbNewLine & Err.Description
	LoadController("PROC")
End Sub

Sub LoadVPM(VPMver, VBSfile, VBSver)
	On Error Resume Next
	If ScriptEngineMajorVersion < 5 Then MsgBox "VB Script Engine 5.0 or higher required"
	ExecuteGlobal GetTextFile(VBSfile)
	If Err Then MsgBox "Unable to open " & VBSfile & ". Ensure that it is in the same folder as this table. " & vbNewLine & Err.Description
	LoadController("VPM")
End Sub

Sub LoadVPinMAME
	Set Controller = CreateObject("VPinMAME.Controller")
	If Err Then MsgBox "Can't Load VPinMAME." & vbNewLine & Err.Description
	If VPMver > "" Then If Controller.Version < VPMver Or Err Then MsgBox "VPinMAME ver " & VPMver & " required."
	If VPinMAMEDriverVer < VBSver Or Err Then MsgBox VBSFile & " ver " & VBSver & " or higher required."
	On Error Goto 0
End Sub

'Check if Controller.txt exists, if not it creates it and adds ForceDisableB2S= with value 0,
'e.g. try to load b2s.server and if not possible, load VPinMAME.Controller instead.
'The user can put a value of 1, which will force to load VPinMAME or no controller for EM tables.
'Also defines the array of toy categories that will either play the sound or trigger the DOF effect.
Sub LoadController(TableType)
	Dim FileObj
	Dim DOFConfig
	Dim TextStr2
	Dim tempc
	Dim count
	Dim ISDOF
	Dim Answer
	
	B2SOn = False
	tempC = 0
	
    Set FileObj=CreateObject("Scripting.FileSystemObject")
        If Not FileObj.FolderExists(UserDirectory) Then
                FileObj.CreateFolder(UserDirectory)
        End If
	If Not FileObj.FileExists(UserDirectory & ControllerFile) Then
		PopupMessage = "A new file, " & ControllerFile & ", has been created in your User folder within your VP Directory. It defines the controller type that VP will use (value 0 is default). "
		PopupMessage = PopupMessage & "This will select B2S for loading, or the VPinMAME.Controller (if B2S fails), or no controller (for EM tables). "
		PopupMessage = PopupMessage & "Changing the value of ForceDisableB2S to 1 in this file will force loading the VPinMAME.Controller, or no controller (for EM tables). "
		ISDOF = msgBox("Are you using DOF?." & vbCrLf & vbCrLf & "This is a question situation with Yes or No only",4+32, "Question?")
		Select Case ISDOF
		Case 6		
			YesNoPrompt "Are you using contactors?", 1
			YesNoPrompt "Are you using a knocker?", 2
			YesNoPrompt "Are you using chimes?", 3
			YesNoPrompt "Are you using a bell?", 4
			YesNoPrompt "Are you using a gear?", 5
			YesNoPrompt "Are you using a shaker?", 6
			PopupMessage = PopupMessage & "DOF lines added based on your answers, a 0 plays the respective sound, a 1 only uses the DOF toy instead. "
        Case 7
			DOFeffects(1) = 0 'contactors
			DOFeffects(2) = 0 'knocker
			DOFeffects(3) = 0 'chimes
			DOFeffects(4) = 0 'bell
			DOFeffects(5) = 0 'gear
			DOFeffects(6) = 0 'shaker
		End Select
		Set DOFConfig=FileObj.CreateTextFile(UserDirectory & ControllerFile)
		DOFConfig.WriteLine "ForceDisableB2S=" & tempC
		DOFConfig.WriteLine "UseDOFcontactors=" & DOFeffects(1)
		DOFConfig.WriteLine "UseDOFKnocker=" & DOFeffects(2)
		DOFConfig.WriteLine "UseDOFChimes=" & DOFeffects(3)
		DOFConfig.WriteLine "UseDOFBell=" & DOFeffects(4)
		DOFConfig.WriteLine "UseDOFGear=" & DOFeffects(5)
		DOFConfig.WriteLine "UseDOFShaker=" & DOFeffects(6)
		DOFConfig.Close
		MsgBox PopupMessage
	Else
		Set DOFConfig=FileObj.GetFile(UserDirectory & ControllerFile)
		Set TextStr2=DOFConfig.OpenAsTextStream(1,0)
		If (TextStr2.AtEndOfStream=False) Then
			count = 0
			Do Until TextStr2.AtEndOfStream
				If count=0 Then
					tempC=(right(TextStr2.ReadLine,1))
				Else
					DOFeffects(count)=(right(TextStr2.ReadLine,1))
				End If
				count = count + 1
			Loop
		End If
	End If

	If TableType = "PROC" Then
		Set Controller = CreateObject("VPROC.Controller")
		If Err Then MsgBox "Can't load PROC"
		If tempC = 0 Then
			Set B2SController = CreateObject("B2S.Server")
				B2SController.B2SName = B2SPROCcGameName
				B2SController.Run()
				On Error Goto 0
				B2SOn = True
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

Sub YesNoPrompt(question,count)
	Answer = MsgBox(question,vbYesNo, "Confirm")
	If Answer = vbYes Then	
		DOFeffects(count) = 1
	Else 
		DOFeffects(count) = 0
	End If
End Sub

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
	Else
		SoundFXDOF = Sound
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
