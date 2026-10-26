Option Explicit

'*****************************************************************************************************
' CREDITS
' Initial table created by fuzzel, jimmyfingers, jpsalas, toxie & unclewilly (in alphabetical order)
' Flipper primitives by zany
' Ball rolling sound script by jpsalas
' Ball shadow by ninuzzu
' Ball control & ball dropping sound by rothbauerw
' DOF by arngrim
' Positional sound helper functions by djrobx
' Plus a lot of input from the whole community (sorry if we forgot you :/)
'*****************************************************************************************************


'First, try to load the Controller.vbs (DOF), which helps controlling additional hardware like lights, gears, knockers, bells and chimes (to increase realism)
'This table uses DOF via the 'SoundFX' calls that are inserted in some of the PlaySound commands, which will then fire an additional event, instead of just playing a sample/sound effect
On Error Resume Next
ExecuteGlobal GetTextFile("controller.vbs")
If Err Then MsgBox "You need the Controller.vbs file in order to run this table (installed with the VPX package in the scripts folder)"
On Error Goto 0

'If using Visual PinMAME (VPM), place the ROM/game name in the constant below,
'both for VPM, and DOF to load the right DOF config from the Configtool, whether it's a VPM or an Original table

'Const cGameName = ""


Dim EnableRetractPlunger
EnableRetractPlunger = false 'Change to true to enable retracting the plunger at a linear speed; wait for 1 second at the maximum position; move back towards the resting position; nice for button/key plungers


If Table1.ShowDT = false then
    Scoretext.Visible = false
End If


Const BallSize = 25  'Ball radius

Sub Table1_KeyDown(ByVal keycode)
	If keycode = PlungerKey Then
		If EnableRetractPlunger Then
			Plunger.PullBackandRetract
		Else
			Plunger.PullBack
		End If
		PlaySound "plungerpull",0,1,AudioPan(Plunger),0.25,0,0,1,AudioFade(Plunger)
	End If

	If keycode = LeftFlipperKey Then
		LeftFlipper.RotateToEnd
		PlaySound SoundFX("fx_flipperup",DOFFlippers), 0, .67, AudioPan(LeftFlipper), 0.05,0,0,1,AudioFade(LeftFlipper)
	End If

	If keycode = RightFlipperKey Then
		RightFlipper.RotateToEnd
		PlaySound SoundFX("fx_flipperup",DOFFlippers), 0, .67, AudioPan(RightFlipper), 0.05,0,0,1,AudioFade(RightFlipper)
	End If

	If keycode = LeftTiltKey Then
		Nudge 90, 2
	End If

	If keycode = RightTiltKey Then
		Nudge 270, 2
	End If

	If keycode = CenterTiltKey Then
		Nudge 0, 2
	End If
End Sub

Sub Table1_KeyUp(ByVal keycode)
	If keycode = PlungerKey Then
		Plunger.Fire
		PlaySound "plunger",0,1,AudioPan(Plunger),0.25,0,0,1,AudioFade(Plunger)
	End If

	If keycode = LeftFlipperKey Then
		LeftFlipper.RotateToStart
		PlaySound SoundFX("fx_flipperdown",DOFFlippers), 0, 1, AudioPan(LeftFlipper), 0.05,0,0,1,AudioFade(LeftFlipper)
	End If

	If keycode = RightFlipperKey Then
		RightFlipper.RotateToStart
		PlaySound SoundFX("fx_flipperdown",DOFFlippers), 0, 1, AudioPan(RightFlipper), 0.05,0,0,1,AudioFade(RightFlipper)
	End If
End Sub


Sub Drain_Hit()
	PlaySound "drain",0,1,AudioPan(Drain),0.25,0,0,1,AudioFade(Drain)
	Drain.DestroyBall
	BIP = BIP - 1
	If BIP = 0 then
		BallRelease.CreateBall
		BallRelease.Kick 90, 7
		PlaySound SoundFX("ballrelease",DOFContactors), 0,1,AudioPan(BallRelease),0.25,0,0,1,AudioFade(BallRelease)
		BIP = BIP + 1
	End If
End Sub


Dim BIP
BIP = 0

Sub Plunger_Init()
	PlaySound SoundFX("ballrelease",DOFContactors), 0,1,AudioPan(BallRelease),0.25,0,0,1,AudioFade(BallRelease)
	BallRelease.CreateBall
	BallRelease.Kick 90, 7
	BIP = BIP + 1
End Sub


'*****GI Lights On
dim xx

For each xx in GI:xx.State = 1: Next

'**********Sling Shot Animations
' Rstep and Lstep  are the variables that increment the animation
'****************
Dim RStep, Lstep

Sub RightSlingShot_Slingshot
    PlaySound SoundFX("right_slingshot",DOFContactors), 0,1, 0.05,0.05 '0,1, AudioPan(RightSlingShot), 0.05,0,0,1,AudioFade(RightSlingShot)
    RSling.Visible = 0
    RSling1.Visible = 1
    sling1.rotx = 20
    RStep = 0
    RightSlingShot.TimerEnabled = 1
	gi1.State = 0:Gi2.State = 0
End Sub

Sub RightSlingShot_Timer
    Select Case RStep
        Case 3:RSLing1.Visible = 0:RSLing2.Visible = 1:sling1.rotx = 10
        Case 4:RSLing2.Visible = 0:RSLing.Visible = 1:sling1.rotx = 0:RightSlingShot.TimerEnabled = 0:gi1.State = 1:Gi2.State = 1
    End Select
    RStep = RStep + 1
End Sub

Sub LeftSlingShot_Slingshot
    PlaySound SoundFX("left_slingshot",DOFContactors), 0,1, -0.05,0.05 '0,1, AudioPan(LeftSlingShot), 0.05,0,0,1,AudioFade(LeftSlingShot)
    LSling.Visible = 0
    LSling1.Visible = 1
    sling2.rotx = 20
    LStep = 0
    LeftSlingShot.TimerEnabled = 1
	gi3.State = 0:Gi4.State = 0
End Sub

Sub LeftSlingShot_Timer
    Select Case LStep
        Case 3:LSLing1.Visible = 0:LSLing2.Visible = 1:sling2.rotx = 10
        Case 4:LSLing2.Visible = 0:LSLing.Visible = 1:sling2.rotx = 0:LeftSlingShot.TimerEnabled = 0:gi3.State = 1:Gi4.State = 1
    End Select
    LStep = LStep + 1
End Sub


'*********************************************************************
'                 Positional Sound Playback Functions
'*********************************************************************

' Play a sound, depending on the X,Y position of the table element (especially cool for surround speaker setups, otherwise stereo panning only)
' parameters (defaults): loopcount (1), volume (1), randompitch (0), pitch (0), useexisting (0), restart (1))
' Note that this will not work (currently) for walls/slingshots as these do not feature a simple, single X,Y position
Sub PlayXYSound(soundname, tableobj, loopcount, volume, randompitch, pitch, useexisting, restart)
	PlaySound soundname, loopcount, volume, AudioPan(tableobj), randompitch, pitch, useexisting, restart, AudioFade(tableobj)
End Sub

' Similar subroutines that are less complicated to use (e.g. simply use standard parameters for the PlaySound call)
Sub PlaySoundAt(soundname, tableobj)
    PlaySound soundname, 1, 1, AudioPan(tableobj), 0,0,0, 1, AudioFade(tableobj)
End Sub

Sub PlaySoundAtBall(soundname)
    PlaySoundAt soundname, ActiveBall
End Sub


'*********************************************************************
'                     Supporting Ball & Sound Functions
'*********************************************************************

Function AudioFade(tableobj) ' Fades between front and back of the table (for surround systems or 2x2 speakers, etc), depending on the Y position on the table. "table1" is the name of the table
    Dim tmp
    tmp = tableobj.y * 2 / table1.height-1
    If tmp > 0 Then
        AudioFade = Csng(tmp ^10)
    Else
        AudioFade = Csng(-((- tmp) ^10) )
    End If
End Function

Function AudioPan(tableobj) ' Calculates the pan for a tableobj based on the X position on the table. "table1" is the name of the table
    Dim tmp
    tmp = tableobj.x * 2 / table1.width-1
    If tmp > 0 Then
        AudioPan = Csng(tmp ^10)
    Else
        AudioPan = Csng(-((- tmp) ^10) )
    End If
End Function

Function Vol(ball) ' Calculates the Volume of the sound based on the ball speed
    Vol = Csng(BallVel(ball) ^2 / 2000)
End Function

Function Pitch(ball) ' Calculates the pitch of the sound based on the ball speed
    Pitch = BallVel(ball) * 20
End Function

Function BallVel(ball) 'Calculates the ball speed
    BallVel = INT(SQR((ball.VelX ^2) + (ball.VelY ^2) ) )
End Function


'********************************************************************
'      JP's VP10 Rolling Sounds (+rothbauerw's Dropping Sounds)
'********************************************************************

Const tnob = 5 ' total number of balls
ReDim rolling(tnob)
InitRolling

Sub InitRolling
    Dim i
    For i = 0 to tnob
        rolling(i) = False
    Next
End Sub

Sub RollingTimer_Timer()
    Dim BOT, b
    BOT = GetBalls

    ' stop the sound of deleted balls
    For b = UBound(BOT) + 1 to tnob
        rolling(b) = False
        StopSound("fx_ballrolling" & b)
    Next

    ' exit the sub if no balls on the table
    If UBound(BOT) = -1 Then Exit Sub

    For b = 0 to UBound(BOT)
        ' play the rolling sound for each ball
        If BallVel(BOT(b) ) > 1 AND BOT(b).z < 30 Then
            rolling(b) = True
            PlaySound("fx_ballrolling" & b), -1, Vol(BOT(b)), AudioPan(BOT(b)), 0, Pitch(BOT(b)), 1, 0, AudioFade(BOT(b))
        Else
            If rolling(b) = True Then
                StopSound("fx_ballrolling" & b)
                rolling(b) = False
            End If
        End If

        ' play ball drop sounds
        If BOT(b).VelZ < -1 and BOT(b).z < 55 and BOT(b).z > 27 Then 'height adjust for ball drop sounds
            PlaySound "fx_ball_drop" & b, 0, ABS(BOT(b).velz)/17, AudioPan(BOT(b)), 0, Pitch(BOT(b)), 1, 0, AudioFade(BOT(b))
        End If
    Next
End Sub

'**********************
' Ball Collision Sound
'**********************

Sub OnBallBallCollision(ball1, ball2, velocity)
	PlaySound("fx_collide"), 0, Csng(velocity) ^2 / 2000, AudioPan(ball1), 0, Pitch(ball1), 0, 0, AudioFade(ball1)
End Sub


'*****************************************
'	ninuzzu's	FLIPPER SHADOWS v3 (VPX 10.8)
'*****************************************

Sub LeftFlipper_Animate()
    FlipperLSh.RotZ = LeftFlipper.CurrentAngle
End Sub

Sub RightFlipper_Animate()
    FlipperRSh.RotZ = RightFlipper.CurrentAngle
End Sub

'*****************************************
'	ninuzzu's	BALL SHADOW
'*****************************************
Dim BallShadow
BallShadow = Array (BallShadow1,BallShadow2,BallShadow3,BallShadow4,BallShadow5)

Sub BallShadowUpdate_timer()
    Dim BOT, b
    BOT = GetBalls
    ' hide shadow of deleted balls
    If UBound(BOT)<(tnob-1) Then
        For b = (UBound(BOT) + 1) to (tnob-1)
            BallShadow(b).visible = 0
        Next
    End If
    ' exit the Sub if no balls on the table
    If UBound(BOT) = -1 Then Exit Sub
    ' render the shadow for each ball
    For b = 0 to UBound(BOT)
        'If BOT(b).X < Table1.Width/2 Then
        '    BallShadow(b).X = ((BOT(b).X) - (Ballsize/6) + ((BOT(b).X - (Table1.Width/2))/7)) + 6
        'Else
        '    BallShadow(b).X = ((BOT(b).X) + (Ballsize/6) + ((BOT(b).X - (Table1.Width/2))/7)) - 6
        'End If
        BallShadow(b).X = BOT(b).X + (BOT(b).X - (Table1.Width/2)) * 1.25 / BallSize
        BallShadow(b).Y = BOT(b).Y + 12
        BallShadow(b).Size_X = 5 / 6.5 ' mesh has the former 6.5 x 5 part size baked in: scale back to 5 x 5
        BallShadow(b).Size_Y = 1
        If BOT(b).Z > 20 Then
            BallShadow(b).visible = 1
        Else
            BallShadow(b).visible = 0
        End If
    Next
End Sub



'************************************
' What you need to add to your table
'************************************

' a timer called RollingTimer. With a fast interval, like 10
' one collision sound, in this script is called fx_collide
' as many sound files as max number of balls, with names ending with 0, 1, 2, 3, etc
' for ex. as used in this script: fx_ballrolling0, fx_ballrolling1, fx_ballrolling2, fx_ballrolling3, etc


'******************************************
' Explanation of the rolling sound routine
'******************************************

' sounds are played based on the ball speed and position

' the routine checks first for deleted balls and stops the rolling sound.

' The For loop goes through all the balls on the table and checks for the ball speed and 
' if the ball is on the table (height lower than 30) then then it plays the sound
' otherwise the sound is stopped, like when the ball has stopped or is on a ramp or flying.

' The sound is played using the VOL, AUDIOPAN, AUDIOFADE and PITCH functions, so the volume and pitch of the sound
' will change according to the ball speed, and the AUDIOPAN & AUDIOFADE functions will change the stereo position
' according to the position of the ball on the table.


'**************************************
' Explanation of the collision routine
'**************************************

' The collision is built in VP.
' You only need to add a Sub OnBallBallCollision(ball1, ball2, velocity) and when two balls collide they 
' will call this routine. What you add in the sub is up to you. As an example is a simple Playsound with volume and paning
' depending of the speed of the collision.


Sub Pins_Hit (idx)
	PlaySound "pinhit_low", 0, Vol(ActiveBall), AudioPan(ActiveBall), 0, Pitch(ActiveBall), 0, 0, AudioFade(ActiveBall)
End Sub

Sub Targets_Hit (idx)
	PlaySound "target", 0, Vol(ActiveBall), AudioPan(ActiveBall), 0, Pitch(ActiveBall), 0, 0, AudioFade(ActiveBall)
End Sub

Sub Metals_Thin_Hit (idx)
	PlaySound "metalhit_thin", 0, Vol(ActiveBall), AudioPan(ActiveBall), 0, Pitch(ActiveBall), 1, 0, AudioFade(ActiveBall)
End Sub

Sub Metals_Medium_Hit (idx)
	PlaySound "metalhit_medium", 0, Vol(ActiveBall), AudioPan(ActiveBall), 0, Pitch(ActiveBall), 1, 0, AudioFade(ActiveBall)
End Sub

Sub Metals2_Hit (idx)
	PlaySound "metalhit2", 0, Vol(ActiveBall), AudioPan(ActiveBall), 0, Pitch(ActiveBall), 1, 0, AudioFade(ActiveBall)
End Sub

Sub Gates_Hit (idx)
	PlaySound "gate4", 0, Vol(ActiveBall), AudioPan(ActiveBall), 0, Pitch(ActiveBall), 1, 0, AudioFade(ActiveBall)
End Sub

Sub Spinner_Spin
	PlaySound "fx_spinner", 0, .25, AudioPan(Spinner), 0.25, 0, 0, 1, AudioFade(Spinner)
End Sub

Sub Rubbers_Hit(idx)
	dim finalspeed
	finalspeed=SQR(activeball.velx * activeball.velx + activeball.vely * activeball.vely)
	If finalspeed > 20 then 
		PlaySound "fx_rubber2", 0, Vol(ActiveBall), AudioPan(ActiveBall), 0, Pitch(ActiveBall), 1, 0, AudioFade(ActiveBall)
	End if
	If finalspeed >= 6 AND finalspeed <= 20 then
		RandomSoundRubber()
	End If
End Sub

Sub Posts_Hit(idx)
	dim finalspeed
	finalspeed=SQR(activeball.velx * activeball.velx + activeball.vely * activeball.vely)
	If finalspeed > 16 then 
		PlaySound "fx_rubber2", 0, Vol(ActiveBall), AudioPan(ActiveBall), 0, Pitch(ActiveBall), 1, 0, AudioFade(ActiveBall)
	End if
	If finalspeed >= 6 AND finalspeed <= 16 then
		RandomSoundRubber()
	End If
End Sub

Sub RandomSoundRubber()
	Select Case Int(Rnd*3)+1
		Case 1 : PlaySound "rubber_hit_1", 0, Vol(ActiveBall), AudioPan(ActiveBall), 0, Pitch(ActiveBall), 1, 0, AudioFade(ActiveBall)
		Case 2 : PlaySound "rubber_hit_2", 0, Vol(ActiveBall), AudioPan(ActiveBall), 0, Pitch(ActiveBall), 1, 0, AudioFade(ActiveBall)
		Case 3 : PlaySound "rubber_hit_3", 0, Vol(ActiveBall), AudioPan(ActiveBall), 0, Pitch(ActiveBall), 1, 0, AudioFade(ActiveBall)
	End Select
End Sub

Sub LeftFlipper_Collide(parm)
	RandomSoundFlipper()
End Sub

Sub RightFlipper_Collide(parm)
	RandomSoundFlipper()
End Sub

Sub RandomSoundFlipper()
	Select Case Int(Rnd*3)+1
		Case 1 : PlaySound "flip_hit_1", 0, Vol(ActiveBall), AudioPan(ActiveBall), 0, Pitch(ActiveBall), 1, 0, AudioFade(ActiveBall)
		Case 2 : PlaySound "flip_hit_2", 0, Vol(ActiveBall), AudioPan(ActiveBall), 0, Pitch(ActiveBall), 1, 0, AudioFade(ActiveBall)
		Case 3 : PlaySound "flip_hit_3", 0, Vol(ActiveBall), AudioPan(ActiveBall), 0, Pitch(ActiveBall), 1, 0, AudioFade(ActiveBall)
	End Select
End Sub
