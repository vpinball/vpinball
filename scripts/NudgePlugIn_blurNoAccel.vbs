'--------------------
' Alternative Nudge
'     by Blur
'--------------------

' For use with core.vbs version 3.41 or greater
' Move into folder with other scripts

Option Explicit

' cvpmNudge2 (Object = vpmNudge)
'   Hopefully we can add a real pendulum simulator in the future
'   (Public)  .TiltSwitch    - set tilt switch
'   (Public)  .Sensitivity   - Set tiltsensitivity (0-10)
'   (Public)  .NudgeInterval - time in ms after which nudge count goes down
'   (Public)  .TiempoFiltro  - nudge filtering time in ms
'   (Public)  .ReturnTime	 - time after which return force bumps in
'   (Public)  .ReturnPercent - return force strength (0-100)
'   (Public)  .NudgeSound	 - set nudge sound, has to be in table
'   (Public)  .TiltSound	 - set tilt sound, has to be in table
'   (Public)  .NudgeMusic	 - mp3 file with nudge sound in music dir
'   (Public)  .TiltMusic	 - mp3 file with tilt sound in music dir
'   (Public)  .TiltObj       - Set objects affected by tilt
'   (Public)  .DoNudge dir,power - Nudge table
'   (Public)  .SolGameOn     - Game On solenoid handler
'   (Private) .Update        - Handle tilting

class cvpmNudge2
	Private mCount, mSensitivity, mNudgeTimer, mSlingBump, mForce
	Private mCount2, mReturnCount, mReturnDir, mReturnForce
	Public TiltSwitch
	Public NudgeInterval, TiempoFiltro, ReturnTime, ReturnPercent

	Private Sub Class_Initialize
		' set defaults for all tables here or change them from your table script
		NudgeInterval = 1000	' time in ms after which nudge count goes down
		TiempoFiltro = 1000 '200' nudge filtering time in ms
		ReturnTime = 200		' time after which return force bumps in
		ReturnPercent = 0		' return force strength 0-100
		'StartShake				' uncomment to enable table shaking
		mCount2 = 0 : mReturnCount = 0 : mReturnDir = 0 : mReturnForce = 0
		mCount = 0 : TiltSwitch = 0 : mSensitivity = 5 : vpmTimer.AddResetObj Me
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

	Public Property Let Sensitivity(aSens) : mSensitivity = (10-aSens)+1 : End property

	Public Sub DoNudge(ByVal aDir, ByVal aForce)
		If mCount2 > 0 and mCount2 < TiempoFiltro\conTimerPulse then exit sub
		mcount2 = 1
		mReturnCount = 1
		If aDir < 20 or aDir > 340 Then aDir = aDir + 180
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
		mCount2 = mCount2 + 1
		If mReturnCount > 0 Then mReturnCount = mReturnCount + 1
		If mReturnCount > ReturnTime\conTimerPulse Then mReturnCount = 0 : Nudge mReturnDir, mReturnForce
		If mCount > 0 Then
			mNudgeTimer = mNudgeTimer + 1
			If mNudgeTimer > NudgeInterval\conTimerPulse Then
				If mCount > mSensitivity+1 Then mCount = mCount - 1 : vpmTimer.PulseSw TiltSwitch
				mCount = mCount - 1 : mNudgeTimer = 0
			End If
		Else
			mCount = 0 : mCount2 = 0 : NeedUpdate = False
		End If
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
