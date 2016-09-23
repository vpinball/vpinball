' MJR's nudge plug-in for cabinets with accelerometers and tilt bobs
' Anyone can use this freely for any purpose, but it has NO WARRANTY of any kind
'
' If your pin cab has an analog accelerometer and a real tilt bob,
' this plug-in will improve how tables handle input from the tilt bob
' switch.  With the standard VP core scripts, the tilt bob has the
' unwanted effect of sending a big simulated jolt to the table
' physics.  If you have an accelerometer, you don't want that extra
' "fake" jolt, because the ball is already getting the right amount
' of nudge action from the accelerometer input.
'
' This script plugs into the VP core scripts to disable the extra,
' fake jolt.  When the tilt bob switch fires, VP will simply pulse
' the ROM tilt switch.
'
' Use this script ONLY if you have BOTH an analog accelerometer AND
' a physical tilt bob switch.  If you don't have an accelerometer,
' you don't want this script, because it will disable all keyboard
' nudging.  (If for some reason you want BOTH accelerometer AND
' keyboard nudging, you'll also have to skip this script, since it
' disables the latter.)
'
' To set this up, follow these steps:
'
' 1. Put this file in your Visual Pinball\Tables directory.
'    It MUST be named NudgePlugIn.vbs
'
' 2. In the Visual Pinball Preferences > Keys dialog, make sure the 
'    key for "Mech Tilt" is set to "T" (that's the default).
'
' 3. In your keyboard encoder setup (i-Pac, etc), set the keystroke
'    that your tilt bob switch sends to "T".  If you're using a
'    key encoder that uses the joystick interface instead of the
'    keyboard, set the "Mech Tilt" entry in the VP Keys dialog to 
'    the button number that your tilt bob is wired to.
'
' Virtually all modern ROM-based tables should work correctly after
' you follow the steps above.  If you find that a table still does
' fake nudging when you press "T" or make contact with your tilt bob
' switch, that table probably has its own special code to handle the
' nudge keys, rather than relying on the VP core scripts (as most
' tables do).  Search the table's script for key handles that respond
' to "keyBangBack" or key code 20 (they're the same thing - that's
' the internal script mapping for the "T" key).  Replace whatever it's
' doing there with a call to vpmNudge.DoNudge 0,6.
'
' Older EM tables that don't have ROM-based controllers are more 
' likely to have custom scripts for this, since the VP common core
' scripts are mostly designed for ROM tables.  If the tilt bob 
' doesn't seem to have any effect in an old EM table, search its
' scripts for "TiltKey" - you'll probably find key handlers for
' LeftTiltKey, RightTiltKey, and CenterTiltKey.  If there's no
' handler for key 20 or keyBangBack, simply add one that does the
' same thing as the others, but omit any call to "Nudge".  E.g.,
' Loserman!'s Hot Shot (Gottlieb 1973) has scripts like this:
'
'  Sub Table1_KeyDown(ByVal keycode)
'     if keycode = CenterTiltKey Then
'        Nudge 90,2
'        TiltIt
'     end if
'     ... many more cases follow ...
'  End Sub
' 
' If you find something like that, you can simply add a new case like
' this:
'
'     if keycode = 20 ' keyBangBack, "T" then TiltIt
'
' In other words, simply call the same subroutine that the other
' handlers call, skipping the call to Nudge (which does that fake
' nudge that we don't want).

Class cvpmNudge2
	Private mCount, mSensitivity, mNudgeTimer, mSlingBump, mForce
	Public TiltSwitch

	Private Sub Class_Initialize
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
               If TiltSwitch <> 0 Then vpmTimer.PulseSw TiltSwitch
        End sub

        Public Sub Update : End Sub

        Public Sub Reset : End Sub

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
