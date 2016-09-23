'----------------------
' Alternative Nudge v2
'     by Blur
'----------------------

' For use with core.vbs version 3.41 or greater
' Move into folder with other scripts

'For example you want to save ball easily so you never lose the ball - set aForce to 2 and returnpercent to 0.
'You want to use tilt bob but you get lot's of nudges, instead of 1 - set tiempofiltro to 2000.
'You want to nudge without pause between nudges - set tiempofiltro to 0
'You want diagonal side nudge, instead of horizontal - set angles to 90+45 and 270-45
'You want bigger counter force then force - no problem - set returnpercent to 150 or 200 (it is possible eventhough it is percent)

Option Explicit

' cvpmNudge2 (Object = vpmNudge)
'   Hopefully we can add a real pendulum simulator in the future
'   (Public)  .TiltSwitch    - set tilt switch
'   (Public)  .Sensitivity   - Set tiltsensitivity (0-10)
'   (Public)  .NudgeSound	 - set nudge sound, has to be in table
'   (Public)  .TiltSound	 - set tilt sound, has to be in table
'   (Public)  .NudgeMusic	 - mp3 file with nudge sound in music dir
'   (Public)  .TiltMusic	 - mp3 file with tilt sound in music dir
'   (Public)  .TiltObj       - Set objects affected by tilt
'   (Public)  .DoNudge dir,power - Nudge table
'   (Public)  .SolGameOn     - Game On solenoid handler
'   (Private) .Update        - Handle tilting

class cvpmNudge2
	Private mCount, mSensitivity, pSensitivity, mNudgeTimer, mSlingBump, mForce
	Private mFilterCount, mReturnCount, mReturnDir, mReturnForce
	Public TiltSwitch
	Private NudgeInterval, TiempoFiltro, ReturnTime, ReturnPercent

	Private Sub Class_Initialize
		' set defaults for all tables here or change them from your table script
		NudgeInterval = 1000 	' time in ms after which nudge count goes down
		TiempoFiltro = 1000 '200' nudge filtering time in ms
		ReturnTime = 200		' time after which return force bumps in
		pSensitivity = 5		' preferred sensitivity overrides table settings
		mSensitivity = 5		' default sensitivity used if not defined in table
		ReturnPercent = 0		' return force strength 0-100
		'StartShake				' uncomment to enable table shaking
		mFilterCount = 0 : mReturnCount = 0 : mReturnDir = 0 : mReturnForce = 0
		mCount = 0 : TiltSwitch = 0 : vpmTimer.AddResetObj Me
	End sub

	Private Property Let NeedUpdate(aEnabled) : vpmTimer.EnableUpdate Me, False, aEnabled : End Property

	Public Property Let TiltObj(aSlingBump)
		Dim ii
		ReDim mForce(vpmSetArray(mSlingBump, aSlingBump))
		For ii = 0 To UBound(mForce)
			If TypeName(mSlingBump(ii)) = "Bumper" Then mForce(ii) = mSlingBump(ii).Threshold
			If vpmVPVer >= 90 and TypeName(mSlingBump(ii)) = "Wall" Then mForce(ii) = mSlingBump(ii).SlingshotThreshold
		Next
	End Property

	Public Property Let Sensitivity(aSens) 
		If pSensitivity = 0 Then mSensitivity = (10-aSens)+1 Else mSensitivity = pSensitivity
	End property

	Public Sub DoNudge(ByVal aDir, ByVal aForce)
		If mFilterCount > 0 and mFilterCount < TiempoFiltro\conTimerPulse then exit sub
		mFilterCount = 1
		mReturnCount = 1
		' preferred angles, force and return force are defined in next lines, set them as you like
		If aDir < 5 or aDir > 355 Then aDir = 180 : aForce = 1.25 : ReturnPercent = 100
		If aDir >= 5 and aDir <= 175 Then aDir = 90 : aForce = 1 : ReturnPercent = 100
		If aDir >= 185 and aDir <= 355 Then aDir = 270 : aForce = 1 : ReturnPercent = 100
		aDir = aDir + (Rnd-0.5)*15*aForce : aForce = (0.6+Rnd*0.8)*aForce
		Nudge aDir, aForce
		mReturnDir = aDir + 180
		mReturnForce = aForce * ReturnPercent / 100
		If TiltSwitch = 0 Then Exit Sub ' If no switch why care
		mCount = mCount + aForce * 1.2
		If mCount > mSensitivity + 10 Then mCount = mSensitivity + 10
		If mCount >= mSensitivity Then vpmTimer.PulseSw TiltSwitch
		NeedUpdate = True
	End sub

	Public Sub Update
		if mFilterCount > 0 then mFilterCount = mFilterCount + 1
		If mFilterCount > TiempoFiltro\conTimerPulse Then mFilterCount = 0
		If mReturnCount > 0 Then mReturnCount = mReturnCount + 1
		If mReturnForce > 0 and mReturnCount > ReturnTime\conTimerPulse Then mReturnCount = 0 : Nudge mReturnDir, mReturnForce
		If mCount > 0 Then
			mNudgeTimer = mNudgeTimer + 1
			If mNudgeTimer > NudgeInterval\conTimerPulse Then
				If mCount > mSensitivity+1 Then mCount = mCount - 1 : vpmTimer.PulseSw TiltSwitch
				mCount = mCount - 1 : mNudgeTimer = 0
			End If
		Else
			mCount = 0
		End If
		If mCount = 0 and mFilterCount = 0 and mReturnCount = 0 Then NeedUpdate = False
		'msgbox mCount & " " & mFilterCount & " " & mReturnCount & " " & mNudgeTimer
	End Sub

	Public Sub Reset : mCount = 0 : End Sub

	Public Sub SolGameOn(aEnabled)
		Dim obj, ii
		If aEnabled Then
			ii = 0
			For Each obj In mSlingBump
				If TypeName(obj) = "Bumper" Then obj.Threshold = mForce(ii) 
				If vpmVPVer >= 90 and TypeName(obj) = "Wall" Then obj.SlingshotThreshold = mForce(ii)
				ii = ii + 1
			Next
		Else
			For Each obj In mSlingBump
				If TypeName(obj) = "Bumper" Then obj.Threshold = 100
				If vpmVPVer >= 90 and TypeName(obj) = "Wall" Then obj.SlingshotThreshold = 100
			Next
		End If
	End Sub
End Class