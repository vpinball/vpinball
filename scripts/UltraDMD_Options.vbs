'*******************************************************************************************************************


'***UltraDMD_Options.vbs version 1.0***'
'
'by ninuzzu
'
'This script will load UltraDMD and bring a GUI to change some options on the fly.
'Use it for all your UDMD based original tables!
'
'--------
'USAGE
'--------
'Copy this script in your VP Scripts folder
'Add this code at the very beginning of your table script, so the table loads it:
'
'On Error Resume Next
'ExecuteGlobal GetTextFile("UltraDMD_Options.vbs")
'If Err Then MsgBox "Unable to open UltraDMD_Options.vbs. Ensure that it is placed in the Scripts folder"
'On Error Goto 0
'
'
'Now define the name for your table(this is NOT the table name you find in the Properties window of the VP Editor,
'this is a unique name and it's used to save the settings inside the VPReg.stg, and by the B2S aswell), e.g.:
'
'Const cGameName = "uefa_cl"
'
'Define the UltraDMD Project folder name (the folder that contains dmd assets, only the name, without the .UltraDMD)
'
'Const cAssetsFolder="ChampionsLeague"
'
'Add the default text color to use with your table (right click on UDMD window to get the color list). e.g.:
'
'Const cDefaultDMDColor = "PowderBlue"
'
'In order to initialize the options and load UltraDMD, add in table_init:
'
'InitUltraDMD cAssetsFolder,cGameName
'
'For example:
'
'Sub table1_init
'InitUltraDMD cAssetsFolder,cGameName
'....
'End Sub
'
'
'Lastly, on table exit, add 
'
'ExitUltraDMD
'
'So the options will reset and DMD will close correctly when you exit the table:
'
'Sub table1_exit
'ExitUltraDMD
'....
'End Sub
'
'When you are ingame, you can open the options menu anytime with
'
'ShowDMDOptions cGameName
'
'Let's say you want to open it using the F6 key, you'll write (in the keydown sub):
'
'If keycode = 64 Then ShowDMDOptions cGameName
'
'--------
'INFO
'--------
'There are 4 options you can change in the GUI:
'1)The 1st option sets the DMD to render scenes in Full Color, valid only with ultradmd 1.4 and above
'2)The 2nd changes the DMD text color
'3)The 3rd changes the DMD size
'4)The 4th changes the DMD position





Dim UltraDMD,TableShowOptions,UDMDOptions,DMDOldColor,DMDOldFullColor
Dim optOpenAtStart,optUseFullColor,optDMDColorChange,optDMDColorType,optUDMDWidth,optUDMDHeight,optDMDPosX,optDMDPosY

Const UltraDMD_VideoMode_Stretch=0, UltraDMD_VideoMode_Top = 1, UltraDMD_VideoMode_Middle = 2, UltraDMD_VideoMode_Bottom = 3
Const UltraDMD_Animation_FadeIn = 0, UltraDMD_Animation_FadeOut = 1, UltraDMD_Animation_ZoomIn = 2, UltraDMD_Animation_ZoomOut = 3
Const UltraDMD_Animation_ScrollOffLeft = 4, UltraDMD_Animation_ScrollOffRight = 5, UltraDMD_Animation_ScrollOnLeft = 6, UltraDMD_Animation_ScrollOnRight = 7,_
UltraDMD_Animation_ScrollOffUp = 8,UltraDMD_Animation_ScrollOffDown = 9,UltraDMD_Animation_ScrollOnUp = 10,UltraDMD_Animation_ScrollOnDown = 11,UltraDMD_Animation_None = 14

'Function to initialize dmd options and load UltraDMD
Function InitUltraDMD(ProjectFolderName,TableOptName)
	UDMDoptions = LoadValue(TableOptName,"DMD Options")				    'load saved dmd options
	If UDMDoptions = "" Then										                'if no existing options
		UDMDoptions = 1*2^0 + 0*2^1 + 1*2^2				          		'set dmd options to default options
		GetDMDOptions(TableOptName)												        'get DMD Options
		TableShowOptions = createInput("ULTRADMD Options",TableOptName)	'show the options menu
	Else
		GetDMDOptions(TableOptName)
		If optOpenAtStart=1 Then				    				              'if Enable Next Start was selected
			UDMDoptions = UDMDoptions - 1							              'clear setting to avoid future executions
			GetDMDOptions(TableOptName)											        'get DMD Options			    
			TableShowOptions = createInput("ULTRADMD Options",TableOptName)		  'show the options menu
		End If
	End If
	LoadUltraDMD ProjectFolderName,TableOptName
End Function

'Function to initialize UltraDMD
Function LoadUltraDMD(ProjectFolderName,TableOptName)
	Set UltraDMD = CreateObject("UltraDMD.DMDObject")
	' A Major version change indicates the version is no longer backward compatible
    If Not UltraDMD.GetMajorVersion = 1 Then
        MsgBox "Incompatible Version of UltraDMD found."
        QuitPlayer:Exit Function
    End If
	'Check if UMD v1.4 is running
    If UltraDMD.GetMinorVersion<4 AND optUseFullColor=1 Then
        MsgBox "Full Color not supported. Please upgrade to UltraDMD v1.4" & VbCrLf & "Exiting so changes will take effect..."
        Dim WshShell:Set WshShell = CreateObject("WScript.Shell")
        WshShell.RegWrite "HKCU\Software\UltraDMD\fullcolor","False","REG_SZ"
		SaveValue TableOptName, "DMD Options", 1
        QuitPlayer:Exit Function
    End If
	UltraDMD.Init
    'Sets the project folder and video stretch mode
	Dim fso,curDir
	Set fso = CreateObject("Scripting.FileSystemObject")
	curDir = fso.GetAbsolutePathName(".")
    UltraDMD.SetProjectFolder curDir & "\"& ProjectFolderName &".UltraDMD"
	If NOT fso.FolderExists(curDir & "\"& ProjectFolderName &".UltraDMD") Then
		MsgBox ProjectFolderName & ".UltraDMD" & " not found." & VbCrLf & "Please copy it in the same folder as this table" & VbCrLf & "DMD animations can't be shown!!" 
	End If
    UltraDMD.SetVideoStretchMode UltraDMD_VideoMode_Middle
End Function

 'Function to reset the DMD color settings and close UltraDMD on table exit
Function ExitUltraDMD
	On Error Resume Next
	If UltraDMD.IsRendering Then UltraDMD.CancelRendering
	If Error.Number<>0 Then UltraDMD = Null:Exit Function
	Dim WshShell:Set WshShell = CreateObject("WScript.Shell")
	WshShell.RegWrite "HKCU\Software\UltraDMD\color",DMDOldColor,"REG_SZ"
	WshShell.RegWrite "HKCU\Software\UltraDMD\fullcolor",DMDOldFullColor,"REG_SZ"
	UltraDMD = Null
End Function

'Function to show the dmd options menu on screen
Function ShowDMDOptions(TableOptName)
	GetDMDOptions(TableOptName):TableShowOptions = createInput("ULTRADMD Options",TableOptName)
End Function

 'Function to get the DMD options from both registry and \User\VPReg.stg
Function GetDMDOptions(TableOptName)
	Dim WshShell:Set WshShell = CreateObject("WScript.Shell")
	On Error Resume Next
	WshShell.RegRead ("HKCU\Software\UltraDMD\")
	If Err.number <> 0 Then
		On Error Goto 0
		'Resetting to defaults
		WshShell.RegWrite "HKCU\Software\UltraDMD\fullcolor","False","REG_SZ"
		WshShell.RegWrite "HKCU\Software\UltraDMD\color","OrangeRed","REG_SZ"
		WshShell.RegWrite "HKCU\Software\UltraDMD\hideVDMD","False","REG_SZ"       
	End If
	Dim tmp:tmp=UDMDoptions
	optUseFullColor=Int(tmp/2^2)
	tmp=tmp mod 2^2
	optDMDColorChange=Int(tmp/2)
	tmp=tmp mod 2
	optOpenAtStart=Int(tmp)
	tmp=tmp mod 1
	On Error Resume Next
	'temporary save the previous color and full color settings
	DMDOldColor=WshShell.RegRead ("HKCU\Software\UltraDMD\color")
	DMDOldFullColor=WshShell.RegRead ("HKCU\Software\UltraDMD\fullcolor")
	If DMDOldFullColor="" Then DMDOldFullColor = "False"
	On Error Resume Next
	optUDMDWidth = WshShell.RegRead ("HKCU\Software\UltraDMD\w")
	optUDMDHeight = WshShell.RegRead ("HKCU\Software\UltraDMD\h")
	optDMDPosX = WshShell.RegRead ("HKCU\Software\UltraDMD\x")
	optDMDPosY = WshShell.RegRead ("HKCU\Software\UltraDMD\y")
	'if user changed the color, remember the value
	If optDMDColorChange=0 Then optDMDColorType=cDefaultDMDColor Else optDMDColorType = LoadValue (TableOptName, "DMD Color") End If
	If optUDMDWidth = "" Then optUDMDWidth = 512
	If optUDMDHeight = "" Then optUDMDHeight = 128
	If optDMDPosX = "" Then optDMDPosX = 20
	If optDMDPosY = "" Then optDMDPosY = 20
	If optUseFullColor=0 Then
		WshShell.RegWrite "HKCU\Software\UltraDMD\fullcolor","False","REG_SZ"
	Else
		WshShell.RegWrite "HKCU\Software\UltraDMD\fullcolor","True","REG_SZ"
	End If
	WshShell.RegWrite "HKCU\Software\UltraDMD\color",optDMDColorType,"REG_SZ"
	WshShell.Terminate
End Function

'Function to create a window in HTML
Function createInput(title,TableOptName)
Dim sHTMLCode,  status
	'Set the HTML code for the window
	sHTMLCode = _
	"<html>" &_
	"<head>" &_
	"<style>" &_
	".button{" &_
	"position:absolute;" &_
	"color: black; border: 2px solid #000000;" &_
	"left:200px;top:350px; height:40px; width=60px; font-size=18; cursor:pointer}" &_
	"</style>" &_
	"</head>" &_
	"<script language='VbScript'>" &_
	"sub UpdateOpt:If resizeDMD.Checked Then DMDWPos.disabled=false:DMDHPos.disabled=false Else DMDWPos.disabled=true:DMDHPos.disabled=true End If:" &_
	"If colorDMDSet.Checked Then dmdtxtcol.disabled=false Else dmdtxtcol.disabled=true End If:" &_
	"If posDMD.Checked Then DMDXPos.disabled=false:DMDYPos.disabled=false Else DMDXPos.disabled=true:DMDYPos.disabled=true End If:end sub" &_
	"</script>" &_
	"<table>" &_
	"<TD><font size='2'>Use Full Color DMD (only for UDMD v1.4 and above)</font><input type='checkbox' name='colorDMD'></TD>" &_
	"</table>" &_
	"</br>" &_
	"<table>" &_
	"<TD><font size='2'>Change DMD Text Color</font><input type='checkbox' name='colorDMDSet' Onclick='UpdateOpt'></TD>" &_
	"<TD>" &_
	"<font size='2'>Color:  </font>" &_
	"<SELECT NAME='dmdtxtcol' disabled>" &_
	"<OPTION value='AliceBlue'>AliceBlue" &_
	"<OPTION value='AntiqueWhite'>AntiqueWhite" &_
	"<OPTION value='Aqua'>Aqua" &_
	"<OPTION value='Aquamarine'>Aquamarine" &_
	"<OPTION value='Azure'>Azure" &_
	"<OPTION value='Beige'>Beige" &_
	"<OPTION value='Bisque'>Bisque" &_
	"<OPTION value='BlanchedAlmond'>BlanchedAlmond" &_
	"<OPTION value='Blue'>Blue" &_
	"<OPTION value='BlueViolet'>BlueViolet" &_
	"<OPTION value='Brown'>Brown" &_
	"<OPTION value='BurlyWood'>BurlyWood" &_
	"<OPTION value='CadetBlue'>CadetBlue" &_
	"<OPTION value='Chartreuse'>Chartreuse" &_
	"<OPTION value='Chocolate'>Chocolate" &_
	"<OPTION value='Coral'>Coral" &_
	"<OPTION value='CornflowerBlue'>CornflowerBlue" &_
	"<OPTION value='Cornsilk'>Cornsilk" &_
	"<OPTION value='Crimson'>Crimson" &_
	"<OPTION value='Cyan'>Cyan" &_
	"<OPTION value='DarkBlue'>DarkBlue" &_
	"<OPTION value='DarkCyan'>DarkCyan" &_
	"<OPTION value='DarkGoldenrod'>DarkGoldenrod" &_
	"<OPTION value='DarkGray'>DarkGray" &_
	"<OPTION value='DarkGreen'>DarkGreen" &_
	"<OPTION value='DarkKhaki'>DarkKhaki" &_
	"<OPTION value='DarkMagenta'>DarkMagenta" &_
	"<OPTION value='DarkOliveGreen'>DarkOliveGreen" &_
	"<OPTION value='DarkOrange'>DarkOrange" &_
	"<OPTION value='DarkOrchid'>DarkOrchid" &_
	"<OPTION value='DarkRed'>DarkRed" &_
	"<OPTION value='DarkSalmon'>DarkSalmon" &_
	"<OPTION value='DarkSeaGreen'>DarkSeaGreen" &_
	"<OPTION value='DarkSlateBlue'>DarkSlateBlue" &_
	"<OPTION value='DarkSlateGray'>DarkSlateGray" &_
	"<OPTION value='DarkTurquoise'>DarkTurquoise" &_
	"<OPTION value='DarkViolet'>DarkViolet" &_
	"<OPTION value='DeepPink'>DeepPink" &_
	"<OPTION value='DeepSkyBlue'>DeepSkyBlue" &_
	"<OPTION value='DimGray'>DimGray" &_
	"<OPTION value='DodgerBlue'>DodgerBlue" &_
	"<OPTION value='Firebrick'>Firebrick" &_
	"<OPTION value='FloralWhite'>FloralWhite" &_
	"<OPTION value='ForestGreen'>ForestGreen" &_
	"<OPTION value='Fuchsia'>Fuchsia" &_
	"<OPTION value='Gainsboro'>Gainsboro" &_
	"<OPTION value='GhostWhite'>GhostWhite" &_
	"<OPTION value='Gold'>Gold" &_
	"<OPTION value='Goldenrod'>Goldenrod" &_
	"<OPTION value='Gray'>Gray" &_
	"<OPTION value='Green'>Green" &_
	"<OPTION value='GreenYellow'>GreenYellow" &_
	"<OPTION value='Honeydew'>Honeydew" &_
	"<OPTION value='HotPink'>HotPink" &_
	"<OPTION value='IndianRed'>IndianRed" &_
	"<OPTION value='Indigo'>Indigo" &_
	"<OPTION value='Ivory'>Ivory" &_
	"<OPTION value='Khaki'>Khaki" &_
	"<OPTION value='Lavender'>Lavender" &_
	"<OPTION value='LavenderBlush'>LavenderBlush" &_
	"<OPTION value='LawnGreen'>LawnGreen" &_
	"<OPTION value='LemonChiffon'>LemonChiffon" &_
	"<OPTION value='LightBlue'>LightBlue" &_
	"<OPTION value='LightCoral'>LightCoral" &_
	"<OPTION value='LightCyan'>LightCyan" &_
	"<OPTION value='LightGoldenrodYellow'>LightGoldenrodYellow" &_
	"<OPTION value='LightGray'>LightGray" &_
	"<OPTION value='LightGreen'>LightGreen" &_
	"<OPTION value='LightPink'>LightPink" &_
	"<OPTION value='LightSalmon'>LightSalmon" &_
	"<OPTION value='LightSeaGreen'>LightSeaGreen" &_
	"<OPTION value='LightSkyBlue'>LightSkyBlue" &_
	"<OPTION value='LightSlateGray'>LightSlateGray" &_
	"<OPTION value='LightSteelBlue'>LightSteelBlue" &_
	"<OPTION value='LightYellow'>LightYellow" &_
	"<OPTION value='Lime'>Lime" &_
	"<OPTION value='LimeGreen'>LimeGreen" &_
	"<OPTION value='Linen'>Linen" &_
	"<OPTION value='Magenta'>Magenta" &_
	"<OPTION value='Maroon'>Maroon" &_
	"<OPTION value='MediumAquamarine'>MediumAquamarine" &_
	"<OPTION value='MediumBlue'>MediumBlue" &_
	"<OPTION value='MediumOrchid'>MediumOrchid" &_
	"<OPTION value='MediumPurple'>MediumPurple" &_
	"<OPTION value='MediumSeaGreen'>MediumSeaGreen" &_
	"<OPTION value='MediumSlateBlue'>MediumSlateBlue" &_
	"<OPTION value='MediumSpringGreen'>MediumSpringGreen" &_
	"<OPTION value='MediumTurquoise'>MediumTurquoise" &_
	"<OPTION value='MediumVioletRed'>MediumVioletRed" &_
	"<OPTION value='MidnightBlue'>MidnightBlue" &_
	"<OPTION value='MintCream'>MintCream" &_
	"<OPTION value='MistyRose'>MistyRose" &_
	"<OPTION value='Moccasin'>Moccasin" &_
	"<OPTION value='NavajoWhite'>NavajoWhite" &_
	"<OPTION value='Navy'>Navy" &_
	"<OPTION value='OldLace'>OldLace" &_
	"<OPTION value='Olive'>Olive" &_
	"<OPTION value='OliveDrab'>OliveDrab" &_
	"<OPTION value='Orange'>Orange" &_
	"<OPTION value='OrangeRed'>OrangeRed" &_
	"<OPTION value='Orchid'>Orchid" &_
	"<OPTION value='PaleGoldenrod'>PaleGoldenrod" &_
	"<OPTION value='PaleGreen'>PaleGreen" &_
	"<OPTION value='PaleTurquoise'>PaleTurquoise" &_
	"<OPTION value='PaleVioletRed'>PaleVioletRed" &_
	"<OPTION value='PapayaWhip'>PapayaWhip" &_
	"<OPTION value='PeachPuff'>PeachPuff" &_
	"<OPTION value='Peru'>Peru" &_
	"<OPTION value='Pink'>Pink" &_
	"<OPTION value='Plum'>Plum" &_
	"<OPTION value='PowderBlue'>PowderBlue" &_
	"<OPTION value='Purple'>Purple" &_
	"<OPTION value='Red'>Red" &_
	"<OPTION value='RosyBrown'>RosyBrown" &_
	"<OPTION value='RoyalBlue'>RoyalBlue" &_
	"<OPTION value='SaddleBrown'>SaddleBrown" &_
	"<OPTION value='Salmon'>Salmon" &_
	"<OPTION value='SandyBrown'>SandyBrown" &_
	"<OPTION value='SeaGreen'>SeaGreen" &_
	"<OPTION value='SeaShell'>SeaShell" &_
	"<OPTION value='Sienna'>Sienna" &_
	"<OPTION value='Silver'>Silver" &_
	"<OPTION value='SkyBlue'>SkyBlue" &_
	"<OPTION value='SlateBlue'>SlateBlue" &_
	"<OPTION value='SlateGray'>SlateGray" &_
	"<OPTION value='Snow'>Snow" &_
	"<OPTION value='SpringGreen'>SpringGreen" &_
	"<OPTION value='SteelBlue'>SteelBlue" &_
	"<OPTION value='Tan'>Tan" &_
	"<OPTION value='Teal'>Teal" &_
	"<OPTION value='Thistle'>Thistle" &_
	"<OPTION value='Tomato'>Tomato" &_
	"<OPTION value='Transparent'>Transparent" &_
	"<OPTION value='Turquoise'>Turquoise" &_
	"<OPTION value='Violet'>Violet" &_
	"<OPTION value='Wheat'>Wheat" &_
	"<OPTION value='White'>White" &_
	"<OPTION value='WhiteSmoke'>WhiteSmoke" &_
	"<OPTION value='Yellow'>Yellow" &_
	"<OPTION value='YellowGreen'>YellowGreen" &_
	"</SELECT>" &_
	"</table>" &_
	"</br>" &_
	"<table>" &_
	"<TD><font size='2'>Auto Resize DMD</font><input type='checkbox' name='resizeDMD' Onclick='UpdateOpt'></TD>" &_
	"<TD><font size='2'>DMD Width</font><input type='text' name='DMDWPos' size='10' disabled></TD>" &_
	"</tr>" &_
	"<TD></TD>" &_
	"<TD><font size='2'>DMD Height</font><input type='text' name='DMDHPos' size='10' disabled></TD>" &_
	"</table>" &_
	"</br>" &_
	"<table>" &_
	"<tr>" &_
	"<TD><font size='2'>Auto Position DMD</font><input type='checkbox' name='posDMD' Onclick='UpdateOpt'></TD>" &_
	"<TD><font size='2'>DMD X Position</font><input type='text' name='DMDXPos' size='10' disabled></TD>" &_
	"</tr>" &_
	"<TD></TD>" &_
	"<TD><font size='2'>DMD Y Position</font><input type='text' name='DMDYPos' size='10' disabled></TD>" &_
	"</table>" &_
	"</br>" &_
	"</br>" &_
	"<TD><font size='2'>Enable this menu on next start</font><input type='checkbox' name='enabledonstart'></TD>" &_
	"<td><input id=1 class='button' type='button' name='btnSubmit' value='OK' onClick='status=1'></td>" &_
	"</html>"

	'Create the window we need
with HTABox("lightgrey", 500, 450, 700, 250,TableOptName)
		 'Set window title 
  .document.title = title
    	'Fill the objects we need
  .document.write sHTMLCode
'  .msg.innerHTML = sHTMLCode
  	.enabledonstart.Checked=optOpenAtStart
		.colorDMD.Checked=optUseFullColor
		.colorDMDSet.Checked=optDMDColorChange
		If .colorDMDSet.Checked Then .dmdtxtcol.disabled=false Else .dmdtxtcol.disabled=true End If
		.dmdtxtcol.Value=optDMDColorType
		.DMDWPos.Value=optUDMDWidth
		.DMDHPos.Value=optUDMDHeight
		.DMDXPos.Value=optDMDPosX
		.DMDYPos.Value=optDMDPosY
  	'Put focus on the OK button
		.btnSubmit.focus
		'set the status to 0 (then we will check it in a loop)
		.status=0
		
		'Check the status of the window
    Do
        On Error Resume Next
        status = .status
         'If the window is closed with the [X] button, there will be an error of access to it
        If Err.number <> 0 Then 
            On Error Goto 0
            'In this case we leave the cycle
            Exit Do
        Else
            if status = 1 Then Exit Do
        End if
    Loop

		'If the status is "1", then we write the filled values
	If status = 1 Then 
		Dim WshShell
		Set WshShell = CreateObject("WScript.Shell")
		If .enabledonstart.Checked Then optOpenAtStart=1 Else optOpenAtStart=0
		If .colorDMD.Checked Then optUseFullColor=1 Else optUseFullColor=0
		If optUseFullColor=1 then
			WshShell.RegWrite "HKCU\Software\UltraDMD\fullcolor", "True" ,"REG_SZ"
		Else
			WshShell.RegWrite "HKCU\Software\UltraDMD\fullcolor", "False" ,"REG_SZ"
		End If
		If .colorDMDSet.Checked Then
			SaveValue TableOptName, "DMD Color", .dmdtxtcol.Value
			WshShell.RegWrite "HKCU\Software\UltraDMD\color",.dmdtxtcol.Value,"REG_SZ"
		Else
			WshShell.RegWrite "HKCU\Software\UltraDMD\color",cDefaultDMDColor,"REG_SZ"
		End If
		If StrComp(cDefaultDMDColor,WshShell.RegRead("HKCU\Software\UltraDMD\color"),1)=0 Then optDMDColorChange=0 Else optDMDColorChange=1 End If
		If .resizeDMD.Checked Then
			WshShell.RegWrite "HKCU\Software\UltraDMD\w",.DMDWPos.Value,"REG_DWORD"
			WshShell.RegWrite "HKCU\Software\UltraDMD\h",.DMDHPos.Value,"REG_DWORD"
		End If
		If .posDMD.Checked Then
			WshShell.RegWrite "HKCU\Software\UltraDMD\x",.DMDXPos.Value,"REG_DWORD"
			WshShell.RegWrite "HKCU\Software\UltraDMD\y",.DMDYPos.Value,"REG_DWORD"
		End If
		UDMDoptions = optOpenAtStart*2^0 + optDMDColorChange*2^1 + optUseFullColor*2^2
		'msgbox "Enabled on Start: " & optOpenAtStart & VbCrLf &_
		'     "Use Full color: " & optUseFullColor & VbCrLf &_
		'     "Change DMD color: " & optDMDColorChange
		SaveValue TableOptName, "DMD Options", UDMDoptions
		WshShell.Terminate
	End If
	.close
	End With
End Function

'Function to create an HTA application
Function HTABox(sBgColor, w, h, x, y, TableOptName) 
Dim IE, HTA, nRnd, sCmd, objShell,objExec
 
  randomize : nRnd = Int(1000000 * rnd) 
  sCmd = "mshta.exe ""javascript:{new " _ 
       & "ActiveXObject(""InternetExplorer.Application"")" _ 
       & ".PutProperty('" & nRnd & "',window);" _ 
       & "window.resizeTo(" & w & "," & h & ");" _ 
       & "window.moveTo(" & x & "," & y & ")}""" 

Set objShell = CreateObject("WScript.Shell")
Set objExec = objShell.Exec (sCmd)

    do until objShell.AppActivate("javascript:{new "): On Error Resume Next:loop

  For Each IE In CreateObject("Shell.Application").windows 
    If IsObject(IE.GetProperty(nRnd)) Then 
      set HTABox = IE.GetProperty(nRnd) 
      IE.Quit 
      HTABox.document.title = "HTABox"

      HTABox.document.write _
             "<HTA:Application contextMenu=no border=thin minimizebutton=no maximizebutton=no >" _ 
             & "<body scroll='no' style='background-color:" _ 
             & sBgColor & ";font:normal 10pt Arial;" _ 
             & "border-Style:outset;border-Width:3px'>" _  
             & "<center><span id=msg>&nbsp;</span></center></body>" 
      Exit Function 
    End If 
  Next 

' In case of error
objExec.Terminate
SaveValue TableOptName, "DMD Options", 4
  MsgBox "There was an error with DMD Options window ! Options are set to defaults."
End Function
