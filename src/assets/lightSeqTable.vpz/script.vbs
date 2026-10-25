'First, try to load the Controller.vbs (DOF), which helps controlling additional hardware like lights, gears, knockers, bells and chimes (to increase realism)
'This table uses DOF via the 'SoundFX' calls that are inserted in some of the PlaySound commands, which will then fire an additional event, instead of just playing a sample/sound effect

On Error Resume Next
ExecuteGlobal GetTextFile("controller.vbs")
If Err Then MsgBox "You need the Controller.vbs file in order to run this table (installed with the VPX package in the scripts folder)"
On Error Goto 0

Dim timeElapsed,bWaitForFinish


Dim UpdateTextbox
UpdateTextbox = False
LightSeq001.UpdateInterval = 10
LightSeq001.Play SeqHatch1HorizOn,50,1
LightSeq001.UpdateInterval = 10
LightSeq001.Play SeqScrewLeftOn,180,1


Sub Table1_KeyDown(ByVal keycode)

	If keycode = 5 Then		'4
        UpdateTextbox = True
		if bWaitForFinish <> true then
			RunSequence(1)
		end if
	End If

	If keycode = 6 Then		'5 
        UpdateTextbox = True
		if bWaitForFinish <> true then
			RunSequence(0)
		end if
	End If

	If keycode = 7 Then		'6 
        UpdateTextbox = True
		if bWaitForFinish <> true then
			SetupAttractMode(LightSeq001) ' Pass in light sequencer, to run extra long show by Black and Wizards Hat
		end if
	End If

End Sub

Sub RunSequence(printTimeEnabled)

	timeElapsed = 0 ' a rough timer elapsed

	LightSeq001.UpdateInterval = 10
	LightSeq001.Play SeqUpOn,10,1,0 ' total ms: 1000
	LightSeq001.UpdateInterval = 3
	LightSeq001.Play SeqScrewRightOff,0,2,0 ' total ms: 1080	

'   'UnComment for Flashers
'	LightSeq002.UpdateInterval = 10
'	LightSeq002.Play SeqUpOn,0,1,0 ' total ms: 1000
'	LightSeq002.UpdateInterval = 3
'	LightSeq002.Play SeqScrewRightOff,0,2,0 ' total ms: 1080	
'	LightSeq002.Play SeqScrewRightOn,0,2,0 ' total ms: 1080	

'   'UnComment for Prims
'	LightSeq003.UpdateInterval = 10
'	LightSeq003.Play SeqUpOn,0,1,0 ' total ms: 1000
'	LightSeq003.UpdateInterval = 3
'	LightSeq003.Play SeqScrewRightOff,0,2,0 ' total ms: 1080
'	LightSeq003.Play SeqScrewRightOn,0,2,0 ' total ms: 1080	

	bWaitForFinish = True
	if printTimeEnabled Then SeqEndedTimer.Enabled = 1
End Sub

'show the user time elapsed if pressed 4 to run the sequence
Sub LightSeq001_PlayDone()
    if UpdateTextbox = False Then Exit Sub
	bWaitForFinish = False
	SeqEndedTimer.Enabled = 0
	If timeElapsed > 0 Then 
		ScoreText.Text = "4 or 5 -time: " & timeElapsed
	Else 
		ScoreText.Text = "4 or 5" 
	End If
	timeElapsed = 0
End Sub

Sub SeqEndedTimer_Timer
	timeElapsed = timeElapsed + 1
	if bWaitForFinish Then SeqEndedTimer.Enabled = 1 : Exit Sub
End Sub

'extra long show made by Black and Wizards Hat
Sub SetupAttractMode(LightSeqAttract)

	' blink all the lights 5 times (with 250ms wait between blink)
	LightSeqAttract.Play SeqBlinking,,5,250
   	' randomly blink 40 lights (per frame) for 4 seconds
	LightSeqAttract.UpdateInterval = 25
	LightSeqAttract.Play SeqRandom,40,,4000

    ' scroll up, turning all the lights on with a tail turning them off
    LightSeqAttract.UpdateInterval = 10
    LightSeqAttract.Play SeqUpOn,75,2
    ' as about but with a shorter tail (and quicker)
    LightSeqAttract.UpdateInterval = 5
    LightSeqAttract.Play SeqUpOn,25,2
    ' scroll down, turning all the lights on with a tail turning them off
    LightSeqAttract.UpdateInterval = 10
    LightSeqAttract.Play SeqDownOn,75,2
    ' as about but with a shorter tail (and quicker)
    LightSeqAttract.UpdateInterval = 5
    LightSeqAttract.Play SeqDownOn,25,2
    ' scroll right, turning all the lights on with a tail turning them off
    LightSeqAttract.UpdateInterval = 10
    LightSeqAttract.Play SeqRightOn,75,2
    ' as above but with a shorter tail (and quicker)
    LightSeqAttract.UpdateInterval = 5
    LightSeqAttract.Play SeqRightOn,25,2
    ' scroll left, turning all the lights on with a tail turning them off
    LightSeqAttract.UpdateInterval = 10
    LightSeqAttract.Play SeqLeftOn,75,2
    ' as above but with a shorter tail (and quicker)
    LightSeqAttract.UpdateInterval = 5
    LightSeqAttract.Play SeqLeftOn,25,2
    ' turn on all the lights starting at the bottom/left and moving diagonaly up
    LightSeqAttract.UpdateInterval = 5
    LightSeqAttract.Play SeqDiagUpRightOn,75,1
    ' turn on all the lights starting at the bottom/right and moving diagonaly up
    LightSeqAttract.UpdateInterval = 5
    LightSeqAttract.Play SeqDiagUpLeftOn,75,1
    ' turn on all the lights starting at the top/left and moving diagonaly down
    LightSeqAttract.UpdateInterval = 5
    LightSeqAttract.Play SeqDiagDownRightOn,75,1
    ' turn on all the lights starting at the top/right and moving diagonaly down
    LightSeqAttract.UpdateInterval = 5
    LightSeqAttract.Play SeqDiagDownLeftOn,75,1
    
    ' Turn on all lights starting in the middle and moving outwards to the side edges
    LightSeqAttract.UpdateInterval = 10
    LightSeqAttract.Play SeqMiddleOutHorizOn,50,2
    ' Turn on all lights starting on the side edges and moving into the middle
    LightSeqAttract.UpdateInterval = 10
    LightSeqAttract.Play SeqMiddleInHorizOn,50,2
    ' Turn on all lights starting in the middle and moving outwards to the side edges
    LightSeqAttract.UpdateInterval = 10
    LightSeqAttract.Play SeqMiddleOutVertOn,50,2
	' Turn on all lights starting on the top and bottom edges and moving inwards to the middle
    LightSeqAttract.UpdateInterval = 10
    LightSeqAttract.Play SeqMiddleInVertOn,50,2
	' top half of the playfield wipes on to the right while the bottom half wipes on to the left	  
    LightSeqAttract.UpdateInterval = 10
    LightSeqAttract.Play SeqStripe1HorizOn,50,1
	' top half of the playfield wipes on to the left while the bottom half wipes on to the right	  
    LightSeqAttract.UpdateInterval = 10
    LightSeqAttract.Play SeqStripe2HorizOn,50,1
    ' left side of the playfield wipes on going up while the right side wipes on doing down
    LightSeqAttract.UpdateInterval = 10
    LightSeqAttract.Play SeqStripe1VertOn,50,1
	' left side of the playfield wipes on going down while the right side wipes on doing up
    LightSeqAttract.UpdateInterval = 10
    LightSeqAttract.Play SeqStripe2VertOn,50,1
	' turn lights on, cross-hatch with even lines going right and odd lines going left
	LightSeqAttract.UpdateInterval = 10
    LightSeqAttract.Play SeqHatch1HorizOn,50,1
	' turn lights on, cross-hatch with even lines going left and odd lines going right
    LightSeqAttract.UpdateInterval = 10
    LightSeqAttract.Play SeqHatch2HorizOn,50,1
	' turn lights on, cross-hatch with even lines going up and odd lines going down
    LightSeqAttract.UpdateInterval = 10
    LightSeqAttract.Play SeqHatch1VertOn,50,1
	' turn lights on, cross-hatch with even lines going down and odd lines going up
    LightSeqAttract.UpdateInterval = 10
    LightSeqAttract.Play SeqHatch2VertOn,50,1
    
	' turn on all the lights, starting in the table center and circle out
  	LightSeqAttract.UpdateInterval = 15
    LightSeqAttract.Play SeqCircleOutOn,50,2
	' turn on all the lights, starting at the table edges and circle in
  	LightSeqAttract.UpdateInterval = 15
    LightSeqAttract.Play SeqCircleInOn,50,2
   	' turn on all the lights, starting in the table center and circle out
  	LightSeqAttract.UpdateInterval = 10
    LightSeqAttract.Play SeqCircleOutOn,5,3
	' turn on all the lights starting in the middle and going clockwise around the table
  	LightSeqAttract.UpdateInterval = 10
	LightSeqAttract.Play SeqClockRightOn,360,1
	' as above just faster
  	LightSeqAttract.UpdateInterval = 5
	LightSeqAttract.Play SeqClockRightOn,360,1
	' turn on all the lights starting in the middle and going anti-clockwise around the table
  	LightSeqAttract.UpdateInterval = 10
	LightSeqAttract.Play SeqClockLeftOn,360,1
	' as above just faster
  	LightSeqAttract.UpdateInterval = 5
	LightSeqAttract.Play SeqClockLeftOn,360,1
	' turn on all the lights with a radar sweep to the right 
  	LightSeqAttract.UpdateInterval = 10
	LightSeqAttract.Play SeqRadarRightOn,180,1
	' turn on all the lights with a radar sweep to the left
  	LightSeqAttract.UpdateInterval = 10
	LightSeqAttract.Play SeqRadarLeftOn,180,1
	' quick right sweep
  	LightSeqAttract.UpdateInterval = 5
	LightSeqAttract.Play SeqRadarRightOn,20,1
	' back to the left
  	LightSeqAttract.UpdateInterval = 5
	LightSeqAttract.Play SeqRadarLeftOn,20,1
	' and back to the right again
  	LightSeqAttract.UpdateInterval = 5
	LightSeqAttract.Play SeqRadarRightOn,20,1

	' turn on all the lights with a wiper sweep to the right 
  	LightSeqAttract.UpdateInterval = 10
	LightSeqAttract.Play SeqWiperRightOn,180,1
	' turn on all the lights with a wiper sweep to the left
  	LightSeqAttract.UpdateInterval = 10
	LightSeqAttract.Play SeqWiperLeftOn,180,1
	' quick right sweep
  	LightSeqAttract.UpdateInterval = 5
	LightSeqAttract.Play SeqWiperRightOn,20,1
	' back to the left
  	LightSeqAttract.UpdateInterval = 5
	LightSeqAttract.Play SeqWiperLeftOn,20,1
	' and back to the right again
  	LightSeqAttract.UpdateInterval = 5
	LightSeqAttract.Play SeqWiperRightOn,20,1

	' turn on all the lights with a left fan upwards
  	LightSeqAttract.UpdateInterval = 10
	LightSeqAttract.Play SeqFanLeftUpOn,180,1
	' turn on all the lights with a left fan downwards
  	LightSeqAttract.UpdateInterval = 10
	LightSeqAttract.Play SeqFanLeftDownOn,180,1
	' quick right up
  	LightSeqAttract.UpdateInterval = 5
	LightSeqAttract.Play SeqFanLeftUpOn,20,1
	' back to the down
  	LightSeqAttract.UpdateInterval = 5
	LightSeqAttract.Play SeqFanLeftDownOn,20,1
	' and back up again
  	LightSeqAttract.UpdateInterval = 5
	LightSeqAttract.Play SeqFanLeftUpOn,20,1

	' turn on all the lights with a Right fan upwards
  	LightSeqAttract.UpdateInterval = 10
	LightSeqAttract.Play SeqFanRightUpOn,180,1
	' turn on all the lights with a Right fan downwards
  	LightSeqAttract.UpdateInterval = 10
	LightSeqAttract.Play SeqFanRightDownOn,180,1
	' quick right up
  	LightSeqAttract.UpdateInterval = 5
	LightSeqAttract.Play SeqFanRightUpOn,20,1
	' back to the down
  	LightSeqAttract.UpdateInterval = 5
	LightSeqAttract.Play SeqFanRightDownOn,20,1
	' and back up again
  	LightSeqAttract.UpdateInterval = 5
	LightSeqAttract.Play SeqFanRightUpOn,20,1

	' turn on all the lights from the bottom left corner and arc up
  	LightSeqAttract.UpdateInterval = 10
	LightSeqAttract.Play SeqArcBottomLeftUpOn,90,1
	' turn on all the lights from the bottom left corner and arc down
  	LightSeqAttract.UpdateInterval = 10
	LightSeqAttract.Play SeqArcBottomLeftDownOn,90,1

	' turn on all the lights from the bottom right corner and arc up
  	LightSeqAttract.UpdateInterval = 10
	LightSeqAttract.Play SeqArcBottomRightUpOn,90,1
	' turn on all the lights from the bottom right corner and arc down
  	LightSeqAttract.UpdateInterval = 10
	LightSeqAttract.Play SeqArcBottomRightDownOn,90,1

	' turn on all the lights from the top right corner and arc down
  	LightSeqAttract.UpdateInterval = 10
	LightSeqAttract.Play SeqArcTopRightDownOn,90,1
	' turn on all the lights from the top right corner and arc up
  	LightSeqAttract.UpdateInterval = 10
	LightSeqAttract.Play SeqArcTopRightUpOn,90,1

	' turn on all the lights from the top left corner and arc down
  	LightSeqAttract.UpdateInterval = 10
	LightSeqAttract.Play SeqArcTopLeftDownOn,90,1
	' turn on all the lights from the top left corner and arc up
  	LightSeqAttract.UpdateInterval = 10
	LightSeqAttract.Play SeqArcTopLeftUpOn,90,1

	' turn all the lights on starting in the centre and screwing clockwise
	LightSeqAttract.UpdateInterval = 10
	LightSeqAttract.Play SeqScrewRightOn,180,1

	' turn all the lights on starting in the centre and screwing anti-clockwise
  	LightSeqAttract.UpdateInterval = 10
	LightSeqAttract.Play SeqScrewLeftOn,180,1

	' invalid entry test
  	LightSeqAttract.UpdateInterval = 10
	LightSeqAttract.Play SeqInvalid,100,1
	
	bWaitForFinish = True
End Sub
