Option Explicit

Const VPinMAMEDriverVer = 3.61

'======================
' VPinMAME driver core
'======================
'
'see Scripts.txt for details on how to use this, plus the actual functions below for more help in its comments

Dim Controller	 ' VPinMAME/B2S/etc Controller Object
Dim vpmTimer	 ' Timer Object
Dim vpmNudge	 ' Nudge handler Object
Dim Lights(260)	 ' Put all lamps in an array for easier handling
' If more than one lamp is connected, fill this with an array of each light
Dim vpmMultiLights() : ReDim vpmMultiLights(0)
Private gNextMechNo : gNextMechNo = 0 ' keep track of created mech handlers (would be nice with static members)

' Callbacks
Dim SolCallback(68) ' Solenoids (parsed at Runtime)
Dim SolModCallback(68) ' Solenoid modulated callbacks (parsed at Runtime)
Dim SolCallbackRef(68) ' Parsed callbacks appended to script to avoid stutters caused by calling Execute
Dim SolModCallbackRef(68) ' Parsed callbacks appended to script to avoid stutters caused by calling Execute
Dim SolPrevState(68) ' When modulating solenoids are in use, needed to keep positive value levels from changing boolean state
Dim SolCallbackInitialized ' Late initialization of solenoid callback
Dim LampCallback	' Called after lamps are updated
Dim PDLedCallback	' Called after leds are updated
Dim GICallback		' Called for each changed GI String
Dim GICallback2		' Called for each changed GI String
Dim MotorCallback	' Called after solenoids are updated
Dim vpmCreateBall	' Called whenever a vpm class needs to create a ball
Dim BSize:If IsEmpty(Eval("BallSize"))=true Then BSize=25 Else BSize = BallSize/2
Dim BMass:If IsEmpty(Eval("BallMass"))=true Then BMass=1 Else BMass = BallMass
Dim UseDMD:If IsEmpty(Eval("UseVPMDMD"))=true Then UseDMD=false Else UseDMD = UseVPMDMD
Dim UseModSol:If IsEmpty(Eval("UseVPMModSol"))=true Then UseModSol=0 Else If UseVPMModSol=True Then UseModSol=1 Else UseModSol = UseVPMModSol ' True or 1 for legacy modulated solenoids (0..255 value), 2 for physical solenoids/lamps/GI/AlphaNumSegments (0..1 value)
Dim UseColoredDMD:If IsEmpty(Eval("UseVPMColoredDMD"))=true Then UseColoredDMD=false Else UseColoredDMD = UseVPMColoredDMD
Dim UseNVRAM:If IsEmpty(Eval("UseVPMNVRAM"))=true Then UseNVRAM=false Else UseNVRAM = UseVPMNVRAM
Dim NVRAMCallback

Set GICallback = Nothing
Set GICallback2 = Nothing
SolCallbackInitialized = False

' Game specific info
Dim ExtraKeyHelp ' Help string for game specific keys
Dim vpmShowDips  ' Show DIPs function

Private vpmVPVer : vpmVPVer = vpmCheckVPVer()

Private Function PinMAMEInterval
	If vpmVPVer >= 10800 Then
		PinMAMEInterval = -2 ' VP10.8 introduced special controller-sync'ed timers (more than once a frame to limit latency, without affecting other -1 timers which should run only once per frame)
	ElseIf vpmVPVer >= 10200 Then
		PinMAMEInterval = -1 ' VP10.2 introduced special frame-sync'ed timers (run at least once per frame, can be 2 if DJRobX latency reduction code is used)
	ElseIf vpmVPVer >= 10000 Then
		PinMAMEInterval = 3  ' as old VP9 timers pretended to run at 1000Hz but actually did only a max of 100Hz (e.g. corresponding nowadays to interval=10), we do something inbetween for VP10+ by default
	Else
		PinMAMEInterval = 1
	End If
End Function

Private Const conStackSw	= 8  ' Stack switches
Private Const conMaxBalls	= 13 ' Because of Apollo 13
Private Const conMaxTimers	= 20 ' Spinners can generate a lot of timers
Private Const conTimerPulse = 40 ' Timer runs at 25Hz
Private Const conFastTicks	= 4  ' Fast is 4 times per timer pulse
Private Const conMaxSwHit	= 5  ' Don't stack up more than 5 events for each switch

' DEPRECATED Flipper constants:
Private Const conFlipRetStrength = 0.01  ' Flipper return strength
Private Const conFlipRetSpeed	 = 0.137 ' Flipper return speed

Function CheckScript(file) 'Checks Tables and Scripts directories for specified vbs file, and if it exists, will load it.
	CheckScript = False
	On Error Resume Next
	Dim TablesDirectory:TablesDirectory = Left(UserDirectory,InStrRev(UserDirectory,"\",InStrRev(UserDirectory,"\")-1))&"tables\"
	Dim ScriptsDirectory:ScriptsDirectory = Left(UserDirectory,InStrRev(UserDirectory,"\",InStrRev(UserDirectory,"\")-1))&"scripts\"
	Dim check:Set check = CreateObject("Scripting.FileSystemObject")
	If check.FileExists(tablesdirectory & file) Or check.FileExists(scriptsdirectory & file) Or check.FileExists(file) Then CheckScript = True
	On Error Goto 0
End Function

Function LoadScript(file) 'Checks Tables and Scripts directories for specified vbs file, and if it exists, will load it.
	LoadScript = False
	On Error Resume Next
	If CheckScript(file) Then ExecuteGlobal GetTextFile(file):LoadScript = True
	On Error Goto 0
End Function

' Dictionary
' At one point, Microsoft had made Scripting.Dictionary "unsafe for scripting", but it's
' been a long time since that was true. So now, to maintain compatibility with all tables
' and scripts that use cvpmDictionary, this class is now a simple wrapper around Microsoft's
' more efficient implementation.
Class cvpmDictionary
	Private mDict
	Private Sub Class_Initialize : Set mDict = CreateObject("Scripting.Dictionary") : End Sub

	' DEPRECATED: MS Dictionaries are not index-based. Use "Exists" method instead.
	Private Function FindKey(aKey)
		Dim ii, key : FindKey = -1
		If mDict.Count > 0 Then
			ii = 0
			For Each key In mDict.Keys
				If key = aKey Then FindKey = ii : Exit Function
			Next
		End If
	End Function

	Public Property Get Count : Count = mDict.Count : End Property

	Public Property Get Item(aKey)
		Item = Empty
		If mDict.Exists(aKey) Then
			If IsObject(mDict(aKey)) Then
				Set Item = mDict(aKey)
			Else
				Item = mDict(aKey)
			End If
		End If
	End Property

	Public Property Let Item(aKey, aData)
		If IsObject(aData) Then
			Set mDict(aKey) = aData
		Else
			mDict(aKey) = aData
		End If
	End Property

	Public Property Set Key(aKey)
		' This function is (and always has been) a no-op. Previous definition
		' just looked up aKey in the keys list, and if found, set the key to itself.
	End Property

	Public Sub Add(aKey, aItem)
		If IsObject(aItem) Then
			Set mDict(aKey) = aItem
		Else
			mDict(aKey) = aItem
		End If
	End Sub

	Public Sub Remove(aKey)		 : mDict.Remove(aKey)		   : End Sub
	Public Sub		RemoveAll	 : mDict.RemoveAll			   : End Sub
	Public Function Exists(aKey) : Exists = mDict.Exists(aKey) : End Function
	Public Function Items		 : Items  = mDict.Items		   : End Function
	Public Function Keys		 : Keys	  = mDict.Keys		   : End Function
End Class

'--------------------
'		Timer
'--------------------
Class cvpmTimer
	Private mNow, mTimers
	Private mQue0, mQue1, mQue2, mQue3, mQue4
	Private mSlowUpdates, mFastUpdates, mResets, mFastTimer

	Private Sub Class_Initialize
		mNow = 0 : mTimers = 0
		ReDim mQue0(conMaxTimers)
		ReDim mQue1(conMaxTimers)
		ReDim mQue2(conMaxTimers)
		ReDim mQue3(conMaxTimers)
		ReDim mQue4(conMaxTimers)
		Set mSlowUpdates = New cvpmDictionary
		Set mFastUpdates = New cvpmDictionary
		Set mResets		 = New cvpmDictionary
	End Sub

	Public Sub InitTimer(aTimerObj, aFast)
		If aFast Then
			Set mFastTimer = aTimerObj
			aTimerObj.TimerInterval = conTimerPulse \ conFastTicks
			aTimerObj.TimerEnabled = False
			vpmBuildEvent aTimerObj, "Timer", "vpmTimer.FastUpdate"
		Else
			aTimerObj.Interval = conTimerPulse : aTimerObj.Enabled = True
			vpmBuildEvent aTimerObj, "Timer", "vpmTimer.Update"
		End If
	End Sub

	Sub EnableUpdate(aClass, aFast, aEnabled)
		On Error Resume Next
		If aFast Then
			If aEnabled Then mFastUpdates.Add aClass, 0 : Else mFastUpdates.Remove aClass
			mFastTimer.TimerEnabled = mFastUpdates.Count > 0
		Else
			If aEnabled Then mSlowUpdates.Add aClass, 0 : Else mSlowUpdates.Remove aClass
		End If
	End Sub

	Public Sub Reset
		Dim obj : For Each obj In mResets.Keys : obj.Reset : Next
	End Sub

	Public Sub FastUpdate
		Dim obj : For Each obj In mFastUpdates.Keys : obj.Update : Next
	End Sub

	Public Sub Update
		Dim ii, jj, sw, obj, mQuecopy

		For Each obj In mSlowUpdates.Keys : obj.Update : Next
		If mTimers = 0 Then Exit Sub
		mNow = mNow + 1 : ii = 1

		Do While ii <= mTimers
			If mQue0(ii) <= mNow Then
				If mQue1(ii) = 0 Then
					If isObject(mQue3(ii)) Then
						Call mQue3(ii)(mQue2(ii))
					ElseIf varType(mQue3(ii)) = vbString Then
						' Warning: calling Execute is a direct cause of stutters since it triggers OS security. We should avoid using it here
						If mQue3(ii) > "" Then Execute mQue3(ii) & " " & mQue2(ii) & " "
					End If
					mTimers = mTimers - 1
					For jj = ii To mTimers
						mQue0(jj) = mQue0(jj+1)
						mQue1(jj) = mQue1(jj+1)
						mQue2(jj) = mQue2(jj+1)
						If isObject(mQue3(jj+1)) Then
						   Set mQue3(jj) = mQue3(jj+1)
						Else
						   mQue3(jj) = mQue3(jj+1)
						End If
						mQue3(jj) = mQue3(jj+1)
						mQue4(jj) = mQue4(jj+1)
					Next
					ii = ii - 1
				ElseIf mQue1(ii) = 1 Then
					mQuecopy = mQue2(ii)
					Controller.Switch(mQuecopy) = False
					mQue0(ii) = mNow + mQue4(ii) : mQue1(ii) = 0
				Else '2
					mQuecopy = mQue2(ii)
					Controller.Switch(mQuecopy) = True
					mQue1(ii) = 1
				End If
			End If
			ii = ii + 1
		Loop
	End Sub

	Public Sub AddResetObj(aObj) : mResets.Add aObj, 0 : End Sub

	Public Sub PulseSw(aSwNo) : PulseSwitch aSwNo, 0, 0 : End Sub

	Public Default Sub PulseSwitch(aSwNo, aDelay, aCallback)
		Dim ii, count, last
		count = 0
		For ii = 1 To mTimers
			If mQue1(ii) > 0 And mQue2(ii) = aSwNo Then count = count + 1 : last = ii
		Next
		If count >= conMaxSwHit Or mTimers = conMaxTimers Then Exit Sub
		mTimers = mTimers + 1
		mQue0(mTimers) = mNow
		mQue1(mTimers) = 2
		mQue2(mTimers) = aSwNo
		If isObject(aCallback) Then
		   Set mQue3(mTimers) = aCallback
		Else
		   mQue3(mTimers) = aCallback
		End If
		mQue4(mTimers) = aDelay \ conTimerPulse
		If count Then mQue0(mTimers) = mQue0(last) + mQue1(last)
	End Sub

	Public Sub AddTimer(aDelay, aCallback)
		If mTimers = conMaxTimers Then Exit Sub
		mTimers = mTimers + 1
		mQue0(mTimers) = mNow + aDelay \ conTimerPulse
		mQue1(mTimers) = 0
		mQue2(mTimers) = 0
		If isObject(aCallback) Then
		   Set mQue3(mTimers) = aCallback
		Else
		   mQue3(mTimers) = aCallback
		End If
		mQue4(mTimers) = 0
	End Sub

	Public Sub AddTimer2(aDelay, aCallback, aID)
		If mTimers = conMaxTimers Then Exit Sub
		mTimers = mTimers + 1
		mQue0(mTimers) = mNow + aDelay \ conTimerPulse
		mQue1(mTimers) = 0
		mQue2(mTimers) = aID
		If isObject(aCallback) Then
		   Set mQue3(mTimers) = aCallback
		Else
		   mQue3(mTimers) = aCallback
		End If
		mQue4(mTimers) = 0
	End Sub
End Class

'--------------------
'	  Trough
'--------------------
Class cvpmTrough
	' Takes over for older cvpmBallStack in "trough mode". Theory of operation:
	' A trough can hold up to N balls, and has N*2 "slots". A ball effectively takes
	' up two slots, so no two adjacent slots (0 and 1) can be occupied at the same time.
	' Switches are assigned to even slots only, which means that as balls move through
	' the trough, each switch is allowed to flip between open and closed.
	' Slot 0 is the exit, and can have additional balls "stacked" on it, simulating balls
	' falling onto the exit kicker instead of coming in from the entrance. Extra balls
	' can be queued up at the entrance, and will enter the trough only if there's room
	' for them.

	Private mSlot(), mSw(), mEntrySw
	Private mBallsInEntry, mMaxBallsPerKick, mStackExitBalls
	Private mExitKicker, mExitDir, mExitForce, mDirVar, mForceVar
	Private mSounds

	' If you want to see what the trough is doing internally, add a TextBox to your table
	' named "DebugBox" (recommend Courier New or FixedSys at a small font size) and set
	' this variable to true via .isDebug = True.
	Private mDebug

	Private Sub Class_Initialize
		Dim ii

		ReDim mSw(conMaxBalls), mSlot(conMaxBalls * 2)
		For ii = 0 to UBound(mSlot) : mSlot(ii) = 0 : Next ' All slots empty to start
		For ii = 0 to UBound(mSw)	: mSw(ii) = 0	: Next ' All switches unassigned to start.
		mEntrySw = 0

		Set mExitKicker = Nothing
		mExitDir = 0 : mExitForce = 1 : mDirVar = 0 : mForceVar = 0
		mBallsInEntry = 0 : mMaxBallsPerKick = 1 : mStackExitBalls = 1

		Set mSounds = New cvpmDictionary

		mDebug = False

		If Not IsObject(vpmTrough) Then Set vpmTrough = Me
	End Sub

	Public Property Let IsTrough(aYes)
		If aYes Then
			Set vpmTrough = Me
		ElseIf Me Is vpmTrough Then
			Set vpmTrough = Nothing
		End If
	End Property

	Public Property Get IsTrough
		IsTrough = (Me Is vpmTrough)
	End Property

	' Initialization

	Public Property Let isDebug(enabled) : mDebug = enabled : End Property

	Public Property Let Size(aSize)
		Dim oldSize, newSize, ii
		oldSize = UBound(mSw)
		newSize = vpMax(1, aSize)

		ReDim Preserve mSlot(newSize * 2)
		ReDim Preserve mSw(newSize)
		For ii = oldSize+1 To newSize : mSw(ii) = 0 : Next
		For ii = (oldSize*2) + 1 to (newSize*2) : mSlot(ii) = 0 : Next
	End Property
	Public Property Get Size : Size = UBound(mSw) : End Property

	' Set EntrySw = 0 if you want balls to just fall into the trough automatically.
	' Set it to a real switch number to indicate that a ball is occupying an entry kicker.
	' The ROM in the controller is then responsible for kicking the ball into the trough.
	Public Property Let EntrySw(swNo) : mEntrySw = swNo : End Property

	' Assign switches, starting from slot 0 and going to entrance.
	' This sub allows you to pass in as many switches as you wish.
	Public Sub InitSwitches(switchArray)
		If Not IsArray(switchArray) Then
			Err.Raise 17, "cvpmTrough.InitSwitches: Input must be an array."
		End If

		Dim ii
		For ii = 0 to UBound(mSw)
			If ii > UBound(switchArray) Then
				mSw(ii) = 0
			Else
				mSw(ii) = switchArray(ii)
			End If
		Next
	End Sub

	' Alternative: Assign a switch to a specific slot.
	Public Sub AddSw(slotNo, swNo)
		If slotNo < 0 OR slotNo > UBound(mSw) Then Exit Sub
		mSw(slotNo) = swNo
	End Sub

	' MaxBallsPerKick: Kick up to N balls total per exit kick. Balls are only kicked from Slot 0.
	' StackExitBalls: Automatically stack up to N balls in Slot 0 regardless of where they came from.

	' Example: Subway where exit kicker is on the same level as the trough and a ball can
	' come in from the exit: StackExitBalls = 1, MaxBallsPerKick = 2. If Slot 0 has 1
	' ball and Slot 1 is occupied, only one ball will be kicked. If Slot 0 has 2 or more
	' balls, it'll kick out 2 balls.

	' Example: Twilight Zone Slot Kicker: Kicker is below trough, so if a ball is in the
	' exit chute, another ball can fall into the chute as well whether it came in from the
	' exit (Slot Machine) or any other entrance (Piano, Camera). In both cases, the kicker
	' will eject 2 balls at once. Set StackExitBalls = 2, maxBallsPerKick = 2 to simulate.

	Public Property Let MaxBallsPerKick(n) : mMaxBallsPerKick = vpMax(1, n) : End Property
	Public Property Let StackExitBalls(n) : mStackExitBalls = vpMax(1, n) : End Property

	Public Sub InitExit(aKicker, aDir, aForce)
		If TypeName(aKicker) <> "Kicker" Then
			Err.Raise 17, "cvpmTrough.InitExit: Cannot use object of type '" & TypeName(aKicker) & "'."
		End If

		Set mExitKicker = aKicker
		mExitDir = aDir
		mExitForce = vpMax(1, aForce)
	End Sub

	Public Sub InitExitVariance(aDirVar, aForceVar)
		mDirVar = aDirVar
		mForceVar = aForceVar
	End Sub

	' Setup sounds
	Public Sub InitEntrySounds(addSound, entrySoundEmpty, entrySoundBall)
		mSounds.Item("add") = addSound
		mSounds.Item("entry") = entrySoundEmpty
		mSounds.Item("entryBall") = entrySoundBall
	End Sub

	Public Sub InitExitSounds(exitSoundEmpty, exitSoundBall)
		mSounds.Item("exit") = exitSoundEmpty
		mSounds.Item("exitBall") = exitSoundBall
	End Sub

	' Start trough with this many balls
	Public Property Let Balls(numBalls)
		Dim ii, ballsAdded

		' First clear all slots.
		For ii = 0 to UBound(mSlot) : mSlot(ii) = 0 : Next

		' Now put a ball in each even-numbered slot up to the number requested.
		' First, stack exit slot. (Note, we may get a negative number. vpMin/vpMax prevent that.)
		mSlot(0) = vpMax(0, vpMin(mStackExitBalls, numBalls))
		ballsAdded = mSlot(0)

		' Fill remaining slots.
		For ii = 1 to vpMin(numBalls - mSlot(0), UBound(mSw))
			mSlot(ii*2) = 1
			ballsAdded = ballsAdded + 1
		Next

		' If we asked to put more balls in the trough than it can handle, queue up the rest.
		mBallsInEntry = vpMax(0, numBalls-ballsAdded)

		UpdateTroughSwitches
	End Property

	Public Property Get Balls
		Balls = 0
		Dim ii : For ii = 0 to UBound(mSlot) : Balls = Balls + mSlot(ii) : Next
	End Property

	Public Property Get BallsPending : BallsPending = mBallsInEntry : End Property

	' Auto-generate events for any entry kickers (eg. outhole, TZ Camera and Piano, etc.)
	' Accepts a single kicker, an Array, or a Collection.
	Public Sub CreateEvents(aName, aKicker)
		Dim obj, tmp
		If Not vpmCheckEvent(aName, Me) Then Exit Sub
		vpmSetArray tmp, aKicker
		For Each obj In tmp
			If isObject(obj) Then
				vpmBuildEvent obj, "Hit", aName & ".AddBall Me"
			Else
				vpmBuildEvent mKicker, "Hit", aName & ".AddBall Me"
			End If
		Next
	End Sub

	' VPM Update management

	Private Property Let NeedUpdate(aEnabled) : vpmTimer.EnableUpdate Me, False, aEnabled : End Property

	Public Sub Reset
		Dim mEntrySwcopy
		UpdateTroughSwitches
		If mEntrySw Then
			mEntrySwcopy = mEntrySw
			Controller.Switch(mEntrySwcopy) = (mBallsInEntry > 0)
		End If
	End Sub

	Public Sub Update
		NeedUpdate = AdvanceBalls
		UpdateTroughSwitches
	End Sub

	' Switch and slot management

	Private Sub setSw(slotNo, enabled)
		Dim mSwcopy
		If mSw(slotNo) Then
			mSwcopy = mSw(slotNo)
			Controller.Switch(mSwcopy) = enabled
		End If
	End Sub

	Private Sub UpdateTroughSwitches
		Dim ii, mSwcopy
		For ii = 0 to UBound(mSw)
			If mSw(ii) Then
				mSwcopy = mSw(ii)
				Controller.Switch(mSwcopy) = (mSlot(ii*2) > 0)
			End If
		Next
		If mDebug Then UpdateDebugBox
	End Sub

	Private Sub UpdateDebugBox ' Requires a textbox named DebugBox
		Dim str, ii, mSwcopy
		str = "Entry: " & mBallsInEntry & " (sw" & mEntrySw & " = "
		If mEntrySw > 0 Then
			mSwcopy = mEntrySw
			str = str & Controller.Switch(mSwcopy)
		Else
			str = str & "n/a"
		End If
		str = str & ")" & vbNewLine

		str = str & "["
		For ii = UBound(mSlot) To 0 Step -1 : str = str & mSlot(ii) : Next
		str = str & "]" & vbNewLine

		str = str & "["
		For ii = UBound(mSlot) To 0 Step -1
			If ii Mod 2 = 0 Then
				If mSw(ii\2) Then
					mSwcopy = mSw(ii\2)
					If Controller.Switch(mSwcopy) Then
						str = str & "1"
					Else
						str = str & "0"
					End If
				Else
					str = str & "-"
				End If
			Else
				str = str & " "
			End If
		Next
		str = str & "]"

		DebugBox.Text = str
	End Sub

	Private Function AdvanceBalls
		Dim ii, canMove, maxSlot
		maxSlot = UBound(mSlot)
		AdvanceBalls = False

		' Move balls through slots, one slot at a time.
		For ii = 0 to maxSlot
			If mSlot(ii) Then ' Ball in this slot.
				canMove = False

				' Can this ball move? (Slot 0 = no)
				If ii = 0 Then
					' Slot 0 never moves (except when ejected)
					canMove = False
				ElseIf ii = 1 Then
					' Slot 1 automatically moves to Slot 0
					canMove = True
				ElseIf ii = 2 Then
					' Slot 2 moves if the number of balls in slot 0 is less than the stack target.
					canMove = (mSlot(0) < mStackExitBalls)
				Else
					' Only move if there is no ball in ii-1 or ii-2.
					canMove = (mSlot(ii-2) = 0) AND (mSlot(ii-1) = 0)
				End If

				If canMove Then
					mSlot(ii) = mSlot(ii) - 1
					mSlot(ii-1) = mSlot(ii-1) + 1
					AdvanceBalls = True ' Mark balls as having moved.
				End If
			End If
		Next

		' If balls are supposed to fall into the trough without going through a kicker,
		' see if any balls are pending and try to add one automatically if so.
		If mBallsInEntry > 0 AND mEntrySw <= 0 Then
			AdvanceBalls = AddBallAtEntrance OR AdvanceBalls
		End If
	End Function

	' Ball management

	Private Function AddBallAtEntrance
		Dim mSwcopy
		Dim maxSlot : maxSlot = UBound(mSlot)
		AddBallAtEntrance = False

		' Only add a ball if there's room for it at the entrance.
		' If the trough is full (or the entrance is occupied), the ball will remain
		' in the entry queue. In a kicker-gated trough, the entry switch will remain
		' pressed down, usually resulting in the machine retrying the load. In a fall-in
		' trough, the ball will just remain queued until the entrance opens up.
		If mSlot(maxSlot) = 0 AND mSlot(maxSlot-1) = 0 Then
			mSlot(maxSlot) = 1
			mBallsInEntry = vpMax(0, mBallsInEntry - 1)
			If mBallsInEntry = 0 AND mEntrySw Then
				mSwcopy = mEntrySw
				Controller.Switch(mSwcopy) = False
			End If
			AddBallAtEntrance = True
		End If
	End Function

	Public Sub AddBall(aKicker)
		Dim mSwcopy
		Dim addDone : addDone = False
		If IsObject(aKicker) Then
			aKicker.DestroyBall
			If aKicker Is mExitKicker Then
				' Ball fell in from exit. Stack it up on Slot 0.
				mSlot(0) = mSlot(0) + 1
				NeedUpdate = True
				UpdateTroughSwitches
				addDone = True
			End If
		End If

		If Not addDone Then
			' Ball came in from entrance. Queue it up for entry.
			mBallsInEntry = mBallsInEntry + 1
		If mEntrySw > 0 Then
			mSwcopy = mEntrySw
			' Trough has an entry kicker. Ball will not enter trough
			' until the entry solenoid is fired.
			Controller.Switch(mSwcopy) = True
			End If
			NeedUpdate = True
		End If

	If isObject(aKicker) Then
		If VP8sound then
			PlaySound mSounds.Item("add")
		ElseIf VP9sound then
			PlaySound mSounds.Item("add"), 1, 1, CoreAudioPan(aKicker.x), 0
		Else
			PlaySound mSounds.Item("add"), 1, 1, CoreAudioPan(aKicker.x), 0, 0, False, False, CoreAudioFade(aKicker.y)
		End If
	Else
		PlaySound mSounds.Item("add")
	End If
	End Sub

	' Use solCallback(solNo) on the trough entry kicker solenoid.
	Public Sub solIn(aEnabled)
		If aEnabled Then
			If mBallsInEntry > 0 Then
				NeedUpdate = AddBallAtEntrance
				PlaySound mSounds.Item("entryBall")
			Else
				PlaySound mSounds.Item("entry")
			End If
		End If
	End Sub
	Public Sub EntrySol_On : solIn(true) : End Sub

	' Use solCallback(solNo) on the trough exit kicker solenoid.
	Public Sub solOut(aEnabled)
		Dim iiBall, kDir, kForce, kBaseDir, ballsEjected
		ballsEjected = 0

		If aEnabled Then
			For iiBall = 0 to (mMaxBallsPerKick - 1)
				kDir = (mExitDir + (Rnd - 0.5) * mDirVar)
				kForce = vpMax(1, mExitForce + (Rnd - 0.5) * mForceVar * (0.8 * iiBall)) ' Dampen force a bit on subsequent balls.

				If mSlot(0) > 0 Then
					' Remove ball from this slot.
					mSlot(0) = mSlot(0) - 1
					If isObject(mExitKicker) Then
						vpmTimer.AddTimer ballsEjected*200, "vpmCreateBall(" & mExitKicker.Name & ").Kick " &_
							CInt(kDir) & "," & Replace(kForce,",",".") & ", 0 '"
					End If

					ballsEjected = ballsEjected + 1
				End If
			Next

			If ballsEjected > 0 Then
				If isObject(mExitKicker) Then
					If VP8sound then
						PlaySound mSounds.Item("exitBall")
					ElseIf VP9sound then
						PlaySound mSounds.Item("exitBall"), 1, 1, CoreAudioPan(mExitKicker.x), 0
					Else
						PlaySound mSounds.Item("exitBall"), 1, 1, CoreAudioPan(mExitKicker.x), 0, 0, False, False, CoreAudioFade(mExitKicker.y)
					End If
				Else
					PlaySound mSounds.Item("exitBall")
				End If
				UpdateTroughSwitches
				NeedUpdate = True
			Else
				If isObject(mExitKicker) Then
					If VP8sound then
						PlaySound mSounds.Item("exit")
					ElseIf VP9sound then
						PlaySound mSounds.Item("exit"), 1, 1, CoreAudioPan(mExitKicker.x), 0
					Else
						PlaySound mSounds.Item("exit"), 1, 1, CoreAudioPan(mExitKicker.x), 0, 0, False, False, CoreAudioFade(mExitKicker.y)
					End If
				Else
					PlaySound mSounds.Item("exit")
				End If
			End If
		End If
	End Sub
	Public Sub ExitSol_On : solOut(true) : End Sub
End Class

'--------------------
'	  Saucer
'--------------------
Class cvpmSaucer
	' Takes over for older cvpmBallStack in "saucer mode".

	Private mSw, mKicker, mExternalKicker
	Private mDir(1), mForce(1), mZForce(1), mDirVar, mForceVar
	Private mSounds

	Private Sub Class_Initialize
		mSw = 0

		mKicker = 0
		mExternalKicker = 0
		mDir(0) = 0 : mForce(0) = 1 : mZForce(0) = 0
		mDir(1) = 0 : mForce(1) = 1 : mZForce(1) = 0
		mDirVar = 0 : mForceVar = 0

		Set mSounds = New cvpmDictionary
	End Sub

	' Initialization

	Public Sub InitKicker(aKicker, aSw, aDir, aForce, aZForce)
		If TypeName(aKicker) <> "Kicker" Then
			Err.Raise 17, "cvpmSaucer.InitKicker: Cannot use object of type '" & TypeName(aKicker) & "'."
		End If

		Set mKicker = aKicker
		mSw = aSw
		mDir(0) = aDir
		mForce(0) = vpMax(1, aForce)
		mZForce(0) = vpMax(0, aZForce)
	End Sub

	Public Sub InitExitVariance(aDirVar, aForceVar)
		mDirVar = aDirVar
		mForceVar = aForceVar
	End Sub

	' Alternate kick params (simulates a saucer with two kickers)
	Public Sub InitAltKick(aDir, aForce, aZForce)
		mDir(1) = aDir
		mForce(1) = vpMax(1, aForce)
		mZForce(1) = vpMax(0, aZForce)
	End Sub

	' Setup sounds
	Public Sub InitSounds(addSound, exitSoundEmpty, exitSoundBall)
		mSounds.Item("add") = addSound
		mSounds.Item("exit") = exitSoundEmpty
		mSounds.Item("exitBall") = exitSoundBall
	End Sub

	' Generate hit event for the kicker(s) associated with this saucer.
	' Accepts a single kicker, an Array, or a Collection.
	Public Sub CreateEvents(aName, aKicker)
		Dim obj, tmp
		If Not vpmCheckEvent(aName, Me) Then Exit Sub
		vpmSetArray tmp, aKicker
		For Each obj In tmp
			If isObject(obj) Then
				vpmBuildEvent obj, "Hit", aName & ".AddBall Me"
			Else
				vpmBuildEvent mKicker, "Hit", aName & ".AddBall Me"
			End If
		Next
	End Sub

	' Ball management

	Public Sub AddBall(aKicker)
		Dim mSwcopy
		If isObject(aKicker) Then
			If aKicker Is mKicker Then
				mKicker.Enabled = False
				mExternalKicker = 0
			Else
				aKicker.Enabled = False
				Set mExternalKicker = aKicker
			End If
		Else
			mKicker.Enabled = False
			mExternalKicker = 0
		End If

		If mSw Then
			mSwcopy = mSw
			Controller.Switch(mSwcopy) = True
		End If
		If isObject(mKicker) Then
			If VP8sound then
				PlaySound mSounds.Item("add")
			ElseIf VP9sound then
				PlaySound mSounds.Item("add"), 1, 1, CoreAudioPan(mKicker.x), 0
			Else
				PlaySound mSounds.Item("add"), 1, 1, CoreAudioPan(mKicker.x), 0, 0, False, False, CoreAudioFade(mKicker.y)
			End If
		Else
			PlaySound mSounds.Item("add")
		End If
	End Sub

	Public Property Get HasBall
		HasBall = False
		If IsObject(mExternalKicker) Then
			HasBall = True
		Else
			HasBall = Not mKicker.Enabled
		End If
	End Property

	' SolCallback solNo, "mySaucer.solOut"
	Public Sub solOut(aEnabled) : If aEnabled Then KickOut 0 : End If : End Sub
	Public Sub ExitSol_On : KickOut 0 : End Sub

	' SolCallback solNo, "mySaucer.solOutAlt"
	Public Sub solOutAlt(aEnabled) : If aEnabled Then KickOut 1 : End If : End Sub
	Public Sub ExitAltSol_On : KickOut 1 : End Sub

	Private Sub KickOut(kickIndex)
		Dim mSwcopy
		If HasBall Then
			Dim kDir, kForce, kZForce

			kDir = mDir(kickIndex) + (Rnd - 0.5)*mDirVar
			kForce = vpMax(1, mForce(kickIndex) + (Rnd - 0.5)*mForceVar)
			kZForce = mZForce(kickIndex)

			If IsObject(mExternalKicker) Then
				' Transfer ball to internal kicker and remove relationship
				vpmCreateBall mKicker
				mExternalKicker.DestroyBall
				mExternalKicker.Enabled = True
			Else
				mKicker.Enabled = True
			End If

			mKicker.Kick kDir, kForce, kZForce
			If mSw Then
				mSwcopy = mSw
				Controller.Switch(mSwcopy) = False
			End If
			If isObject(mKicker) Then
				If VP8sound then
					PlaySound mSounds.Item("exitBall")
				ElseIf VP9sound then
					PlaySound mSounds.Item("exitBall"), 1, 1, CoreAudioPan(mKicker.x), 0
				Else
					PlaySound mSounds.Item("exitBall"), 1, 1, CoreAudioPan(mKicker.x), 0, 0, False, False, CoreAudioFade(mKicker.y)
				End If
			Else
				PlaySound mSounds.Item("exitBall")
			End If
		Else
			If isObject(mKicker) Then
				If VP8sound then
					PlaySound mSounds.Item("exit")
				ElseIf VP9sound then
					PlaySound mSounds.Item("exit"), 1, 1, CoreAudioPan(mKicker.x), 0
				Else
					PlaySound mSounds.Item("exit"), 1, 1, CoreAudioPan(mKicker.x), 0, 0, False, False, CoreAudioFade(mKicker.y)
				End If
			Else
				PlaySound mSounds.Item("exit")
			End If
		End If
	End Sub
End Class

'--------------------
'	  BallStack (DEPRECATED/LEGACY)
'	  Known issues:
'	  - Adding more balls than conMaxBalls will crash the script.
'	  - If there are more balls in trough than are ever used in a game (e.g. Bride of Pinbot),
'		one or more trough switches will be permanently stuck down and may result in a ROM test report.
'	  - Trough does not handle stacking balls at exit.
'	  - Saucer mode is essentially a hack on top of the trough logic.
'--------------------
Class cvpmBallStack
	Private mSw(), mEntrySw, mBalls, mBallIn, mBallPos(), mSaucer, mBallsMoving
	Private mInitKicker, mExitKicker, mExitDir, mExitForce
	Private mExitDir2, mExitForce2
	Private mEntrySnd, mEntrySndBall, mExitSnd, mExitSndBall, mAddSnd
	Private mSoundKicker
	Public KickZ, KickBalls, KickForceVar, KickAngleVar

	Private Sub Class_Initialize
		ReDim mSw(conStackSw), mBallPos(conMaxBalls)
		mBallIn = 0 : mBalls = 0 : mExitKicker = 0 : mInitKicker = 0 : mBallsMoving = False
		KickBalls = 1 : mSaucer = False : mExitDir = 0 : mExitForce = 0
		mExitDir2 = 0 : mExitForce2 = 0 : KickZ = 0 : KickForceVar = 0 : KickAngleVar = 0
		mAddSnd = 0 : mEntrySnd = 0 : mEntrySndBall = 0 : mExitSnd = 0 : mExitSndBall = 0 : mSoundKicker = 0
		vpmTimer.AddResetObj Me
	End Sub

	Private Property Let NeedUpdate(aEnabled) : vpmTimer.EnableUpdate Me, False, aEnabled : End Property

	Private Function SetSw(aNo, aStatus)
		Dim mSwcopy
		SetSw = False
		If HasSw(aNo) Then
			mSwcopy = mSw(aNo)
			Controller.Switch(mSwcopy) = aStatus
			SetSw = True
		End If
	End Function

	Private Function HasSw(aNo)
		HasSw = False : If aNo <= conStackSw Then If mSw(aNo) Then HasSw = True
	End Function

	Public Sub Reset
		Dim mSwcopy
		Dim ii : If mBalls Then For ii = 1 to mBalls : SetSw mBallPos(ii), True : Next
		If mEntrySw And mBallIn > 0 Then
			mSwcopy = mEntrySw
			Controller.Switch(mSwcopy) = True
		End If
	End Sub

	Public Sub Update
		Dim BallQue, ii, mSwcopy
		NeedUpdate = False : BallQue = 1
		For ii = 1 To mBalls
			If mBallpos(ii) > BallQue Then ' next slot available
				NeedUpdate = True
				If HasSw(mBallPos(ii)) Then ' has switch
					mSwcopy = mSw(mBallPos(ii))
					If Controller.Switch(mSwcopy) Then
						SetSw mBallPos(ii), False
					Else
						mBallPos(ii) = mBallPos(ii) - 1
						SetSw mBallPos(ii), True
					End If
				Else ' no switch. Move ball to first switch or occupied slot
					Do
						mBallPos(ii) = mBallPos(ii) - 1
					Loop Until SetSw(mBallPos(ii), True) Or mBallPos(ii) = BallQue
				End If
			End If
			BallQue = mBallPos(ii) + 1
		Next
	End Sub

	Public Sub AddBall(aKicker)
		Dim mSwcopy
		If isObject(aKicker) Then
			If mSaucer Then
				If aKicker Is mExitKicker Then
					mExitKicker.Enabled = False : mInitKicker = 0
				Else
					aKicker.Enabled = False : Set mInitKicker = aKicker
				End If
			Else
				aKicker.DestroyBall
			End If
		ElseIf mSaucer Then
			mExitKicker.Enabled = False : mInitKicker = 0
		End If
		If mEntrySw Then
			mSwcopy = mEntrySw
			Controller.Switch(mSwcopy) = True : mBallIn = mBallIn + 1
		Else
			mBalls = mBalls + 1 : mBallPos(mBalls) = conStackSw + 1 : NeedUpdate = True
		End If
		If isObject(mSoundKicker) Then
			If VP8sound then
				PlaySound mAddSnd
			ElseIf VP9sound then
				PlaySound mAddSnd, 1, 1, CoreAudioPan(mSoundKicker.x), 0
			Else
				PlaySound mAddSnd, 1, 1, CoreAudioPan(mSoundKicker.x), 0, 0, False, False, CoreAudioFade(mSoundKicker.y)
			End If
		Else
			PlaySound mAddSnd
		End If
	End Sub

	' A bug in the script engine forces the "End If" at the end
	Public Sub SolIn(aEnabled)	   : If aEnabled Then KickIn		: End If : End Sub
	Public Sub SolOut(aEnabled)	   : If aEnabled Then KickOut False : End If : End Sub
	Public Sub SolOutAlt(aEnabled) : If aEnabled Then KickOut True	: End If : End Sub
	Public Sub EntrySol_On	 : KickIn		 : End Sub
	Public Sub ExitSol_On	 : KickOut False : End Sub
	Public Sub ExitAltSol_On : KickOut True	 : End Sub

	Private Sub KickIn
		Dim mSwcopy
		If mBallIn Then PlaySound mEntrySndBall Else PlaySound mEntrySnd : Exit Sub
		mBalls = mBalls + 1 : mBallIn = mBallIn - 1 : mBallPos(mBalls) = conStackSw + 1 : NeedUpdate = True
		If mEntrySw And mBallIn = 0 Then
			mSwcopy = mEntrySw
			Controller.Switch(mSwcopy) = False
		End If
	End Sub

	Private Sub KickOut(aAltSol)
		Dim ii,jj, kForce, kDir, kBaseDir
		If mBalls Then
			If isObject(mSoundKicker) Then
				If VP8sound then
					PlaySound mExitSndBall
				ElseIf VP9sound then
					PlaySound mExitSndBall, 1, 1, CoreAudioPan(mSoundKicker.x), 0
				Else
					PlaySound mExitSndBall, 1, 1, CoreAudioPan(mSoundKicker.x), 0, 0, False, False, CoreAudioFade(mSoundKicker.y)
				End If
			Else
				PlaySound mExitSndBall
			End If
		Else
			If isObject(mSoundKicker) Then
				If VP8sound then
					PlaySound mExitSnd
				ElseIf VP9sound then
					PlaySound mExitSnd, 1, 1, CoreAudioPan(mSoundKicker.x), 0
				Else
					PlaySound mExitSnd, 1, 1, CoreAudioPan(mSoundKicker.x), 0, 0, False, False, CoreAudioFade(mSoundKicker.y)
				End If
			Else
				PlaySound mExitSnd
			End If
			Exit Sub
		End If
		If aAltSol Then kForce = mExitForce2 : kBaseDir = mExitDir2 Else kForce = mExitForce : kBaseDir = mExitDir
		kForce = kForce + (Rnd - 0.5)*KickForceVar
		If mSaucer Then
			SetSw 1, False : mBalls = 0 : kDir = kBaseDir + (Rnd - 0.5)*KickAngleVar
			If isObject(mInitKicker) Then
				vpmCreateBall mExitKicker : mInitKicker.Destroyball : mInitKicker.Enabled = True
			Else
				mExitKicker.Enabled = True
			End If
			mExitKicker.Kick kDir, kForce, KickZ
		Else
			For ii = 1 To kickballs
				If mBalls = 0 Or mBallPos(1) <> ii Then Exit For ' No more balls
				For jj = 2 To mBalls ' Move balls in array
					mBallPos(jj-1) = mBallPos(jj)
				Next
				mBallPos(mBalls) = 0 : mBalls = mBalls - 1 : NeedUpdate = True
				SetSw ii, False
				If isObject(mExitKicker) Then
					If kForce < 1 Then kForce = 1
					kDir = kBaseDir + (Rnd - 0.5)*KickAngleVar
					vpmTimer.AddTimer 200*(ii-1), "vpmCreateBall(" & mExitKicker.Name & ").Kick " &_
					  CInt(kDir) & "," & Replace(kForce,",",".") & "," & Replace(KickZ,",",".") & " '"
				End If
				kForce = kForce * 0.8
			Next
		End If
	End Sub

	Public Sub InitSaucer(aKicker, aSw, aDir, aPower)
		InitKick aKicker, aDir, aPower : mSaucer = True
		If aSw Then mSw(1) = aSw Else mSw(1) = aKicker.TimerInterval
	End Sub

	Public Sub InitNoTrough(aKicker, aSw, aDir, aPower)
		InitKick aKicker, aDir, aPower : Balls = 1
		If aSw Then mSw(1) = aSw Else mSw(1) = aKicker.TimerInterval
		If Not IsObject(vpmTrough) Then Set vpmTrough = Me
	End Sub

	Public Sub InitSw(aEntry, aSw1, aSw2, aSw3, aSw4, aSw5, aSw6, aSw7)
		mEntrySw = aEntry : mSw(1) = aSw1 : mSw(2) = aSw2 : mSw(3) = aSw3 : mSw(4) = aSw4
		mSw(5) = aSw5 : mSw(6) = aSw6 : mSw(7) = aSw7 : mSw(8) = 0
		If Not IsObject(vpmTrough) Then Set vpmTrough = Me
	End Sub

	Public Sub InitSw8(aEntry, aSw1, aSw2, aSw3, aSw4, aSw5, aSw6, aSw7, aSw8)
		mEntrySw = aEntry : mSw(1) = aSw1 : mSw(2) = aSw2 : mSw(3) = aSw3 : mSw(4) = aSw4
		mSw(5) = aSw5 : mSw(6) = aSw6 : mSw(7) = aSw7 : mSw(8) = aSw8
		If Not IsObject(vpmTrough) Then Set vpmTrough = Me
	End Sub

	Public Sub InitKick(aKicker, aDir, aForce)
		Set mExitKicker = aKicker : mExitDir = aDir : mExitForce = aForce
		Set mSoundKicker = aKicker
	End Sub

	Public Sub CreateEvents(aName, aKicker)
		Dim obj, tmp
		If Not vpmCheckEvent(aName, Me) Then Exit Sub
		vpmSetArray tmp, aKicker
		For Each obj In tmp
			If isObject(obj) Then
				vpmBuildEvent obj, "Hit", aName & ".AddBall Me"
			Else
				vpmBuildEvent mExitKicker, "Hit", aName & ".AddBall Me"
			End If
		Next
	End Sub

	Public Property Let IsTrough(aIsTrough)
		If aIsTrough Then
			Set vpmTrough = Me
		ElseIf IsObject(vpmTrough) Then
			If vpmTrough Is Me Then vpmTrough = 0
		End If
	End Property

	Public Property Get IsTrough : IsTrough = vpmTrough Is Me : End Property

	Public Sub InitAltKick(aDir, aForce)
		mExitDir2 = aDir : mExitForce2 = aForce
	End Sub

	Public Sub InitEntrySnd(aBall, aNoBall) : mEntrySndBall = aBall : mEntrySnd = aNoBall : End Sub
	Public Sub InitExitSnd(aBall, aNoBall)	: mExitSndBall = aBall	: mExitSnd = aNoBall  : End Sub
	Public Sub InitAddSnd(aSnd) : mAddSnd = aSnd : End Sub

	Public Property Let Balls(aBalls)
		Dim ii
		For ii = 1 To conStackSw
			SetSw ii, False : mBallPos(ii) = conStackSw + 1
		Next
		If mSaucer And aBalls > 0 And mBalls = 0 Then vpmCreateBall mExitKicker
		mBalls = aBalls : NeedUpdate = True
	End Property

	Public Default Property Get Balls : Balls = mBalls		   : End Property
	Public Property Get BallsPending  : BallsPending = mBallIn : End Property

	' Obsolete stuff
	Public Sub SolEntry(aSnd1, aSnd2, aEnabled)
		If aEnabled Then mEntrySndBall = aSnd1 : mEntrySnd = aSnd2 : KickIn
	End Sub
	Public Sub SolExit(aSnd1, aSnd2, aEnabled)
		If aEnabled Then mExitSndBall = aSnd1 : mExitSnd = aSnd2 : KickOut False
	End Sub
	Public Sub InitProxy(aProxyPos, aSwNo) : End Sub
	Public TempBallColour, TempBallImage, BallColour
	Public Property Let BallImage(aImage) : vpmBallImage = aImage : End Property
End Class

'--------------------
'		Nudge
'--------------------
class cvpmNudge
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
			If vpmVPVer >= 9000 and TypeName(mSlingBump(ii)) = "Wall" Then mForce(ii) = mSlingBump(ii).SlingshotThreshold
		Next
	End Property

	Public Property Let Sensitivity(aSens) : mSensitivity = (10-aSens)+1 : End property

	Public Sub DoNudge(ByVal aDir, ByVal aForce)
		aDir = aDir + (Rnd-0.5)*15*aForce : aForce = (0.6+Rnd*0.8)*aForce
		Nudge aDir, aForce
		If TiltSwitch = 0 Then Exit Sub ' If no switch why care
		mCount = mCount + aForce * 1.2
		If mCount > mSensitivity + 10 Then mCount = mSensitivity + 10
		If mCount >= mSensitivity Then vpmTimer.PulseSw TiltSwitch
		NeedUpdate = True
	End sub

	Public Sub Update
		If mCount > 0 Then
			mNudgeTimer = mNudgeTimer + 1
			If mNudgeTimer > 1000\conTimerPulse Then
				If mCount > mSensitivity+1 Then mCount = mCount - 1 : vpmTimer.PulseSw TiltSwitch
				mCount = mCount - 1 : mNudgeTimer = 0
			End If
		Else
			mCount = 0 : NeedUpdate = False
		End If
	End Sub

	Public Sub Reset : mCount = 0 : End Sub

	Public Sub SolGameOn(aEnabled)
		if IsEmpty(mForce) then exit sub 'prevent errors if vpmNudge.TiltObj isn't set
		Dim obj, ii
		If aEnabled Then
			ii = 0
			For Each obj In mSlingBump
				If TypeName(obj) = "Bumper" Then obj.Threshold = mForce(ii)
				If vpmVPVer >= 9000 and TypeName(obj) = "Wall" Then obj.SlingshotThreshold = mForce(ii)
				ii = ii + 1
			Next
		Else
			For Each obj In mSlingBump
				If TypeName(obj) = "Bumper" Then obj.Threshold = 100
				If vpmVPVer >= 9000 and TypeName(obj) = "Wall" Then obj.SlingshotThreshold = 100
			Next
		End If
	End Sub
End Class

'--------------------
'	 DropTarget
'--------------------
Class cvpmDropTarget
	Private mDropObj, mDropSw(), mDropSnd, mRaiseSnd, mSwAnyUp, mSwAllDn, mAllDn, mLink

	Private Sub Class_Initialize
		mDropSnd = 0 : mRaiseSnd = 0 : mSwAnyUp = 0 : mSwAllDn = 0 : mAllDn = False : mLink = Empty
	End sub

	Private Sub CheckAllDn(ByVal aStatus)
		Dim obj
		If Not IsEmpty(mLink) Then
			If aStatus Then
				For Each obj In mLink : aStatus = aStatus And obj.AllDown : Next
			End If
			For Each obj In mLink: obj.SetAllDn aStatus : Next
		End If
		SetAllDn aStatus
	End Sub

	Public Sub SetAllDn(aStatus)
		Dim mSwcopy
		If mSwAllDn Then
			mSwcopy = mSwAllDn
			Controller.Switch(mSwcopy) = aStatus
		End If
		If mSwAnyUp Then
			mSwcopy = mSwAnyUp
			Controller.Switch(mSwcopy) = Not aStatus
		End If
	End Sub

	Public Sub InitDrop(aWalls, aSw)
		Dim obj, obj2, ii
		' Fill in switch number
		On Error Resume Next : ReDim mDropSw(0)
		If IsArray(aSw) Then
			ReDim mDropSw(UBound(aSw))
			For ii = 0 To UBound(aSw) : mDropSw(ii) = aSw(ii) : Next
		ElseIf aSw = 0 Or Err Then
			On Error Goto 0
			If vpmIsArray(aWalls) Then
				ii = 0 : If IsArray(aWalls) Then ReDim mDropSw(UBound(aWalls)) Else ReDim mDropSw(aWalls.Count-1)
				For Each obj In aWalls
					If vpmIsArray(obj) Then
						For Each obj2 In obj
							If obj2.HasHitEvent Then mDropSw(ii) = obj2.TimerInterval : Exit For
						Next
					Else
						mDropSw(ii) = obj.TimerInterval
					End If
					ii = ii + 1
				Next
			Else
				mDropSw(0) = aWalls.TimerInterval
			End If
		Else
			mDropSw(0) = aSw
		End If
		' Copy walls
		vpmSetArray mDropObj, aWalls
	End Sub

	Public Sub CreateEvents(aName)
		Dim ii, obj1, obj2
		If Not vpmCheckEvent(aName, Me) Then Exit Sub
		ii = 1
		For Each obj1 In mDropObj
			If vpmIsArray(obj1) Then
				For Each obj2 In obj1
					if TypeName(obj2) = "HitTarget" Then 'if object in array is a Target, use .Dropped
						vpmBuildEvent obj2, "Dropped", aName & ".Hit " & ii	'Droptarget_Dropped : DTbank.Hit 1 : End Sub
					else
						If obj2.HasHitEvent Then vpmBuildEvent obj2, "Hit", aName & ".Hit " & ii
					End If
				Next
			Else
				if TypeName(obj1) = "HitTarget" Then 'if object in array is a Target, use .Dropped
					vpmBuildEvent obj1, "Dropped", aName & ".Hit " & ii
				else
					vpmBuildEvent obj1, "Hit", aName & ".Hit " & ii
				End If
			End If
			ii = ii + 1
		Next
	End Sub

	Public Property Let AnyUpSw(aSwAnyUp)
		Dim mSwcopy
		mSwAnyUp = aSwAnyUp
		mSwcopy = mSwAnyUp
		Controller.Switch(mSwcopy) = True
	End Property
	Public Property Let AllDownSw(aSwAllDn) : mSwAllDn = aSwAllDn : End Property
	Public Property Get AllDown : AllDown = mAllDn : End Property
	Public Sub InitSnd(aDrop, aRaise) : mDropSnd = aDrop : mRaiseSnd = aRaise : End Sub
	Public Property Let LinkedTo(aLink)
		If IsArray(aLink) Then mLink = aLink Else mLink = Array(aLink)
	End Property

	Public Sub Hit(aNo)
		Dim ii, mSwcopy

'		vpmSolWall mDropObj(aNo-1), mDropSnd, True

		If TypeName(mDropObj(aNo-1)) = "HitTarget" Then
			If VP8sound then
				PlaySound mDropSnd
			ElseIf VP9sound then
				PlaySound mDropSnd, 1, 1, CoreAudioPan(mDropObj(aNo-1).x), 0
			Else
				PlaySound mDropSnd, 1, 1, CoreAudioPan(mDropObj(aNo-1).x), 0, 0, False, False, CoreAudioFade(mDropObj(aNo-1).y)
			End If
		Else
			PlaySound mDropSnd
		End If
		vpmSolWall mDropObj(aNo-1), False, True

		mSwcopy = mDropSw(aNo-1)
		Controller.Switch(mSwcopy) = True
		For Each ii In mDropSw
			mSwcopy = ii
			If Not Controller.Switch(mSwcopy) Then Exit Sub
		Next
		mAllDn = True : CheckAllDn True
	End Sub

	Public Sub SolHit(aNo, aEnabled) : If aEnabled Then Hit aNo : End If : End Sub

	Public Sub SolUnhit(aNo, aEnabled)
		Dim mSwcopy
		Dim ii : If Not aEnabled Then Exit Sub
		If TypeName(mDropObj(aNo-1)) = "HitTarget" Then
			If VP8sound then
				PlaySound mRaiseSnd
			ElseIf VP9sound then
				PlaySound mRaiseSnd, 1, 1, CoreAudioPan(mDropObj(aNo-1).x), 0
			Else
				PlaySound mRaiseSnd, 1, 1, CoreAudioPan(mDropObj(aNo-1).x), 0, 0, False, False, CoreAudioFade(mDropObj(aNo-1).y): vpmSolWall mDropObj(aNo-1), False, False
			End If
		Else
			PlaySound mRaiseSnd
		End If

		mSwcopy = mDropSw(aNo-1)
		Controller.Switch(mSwcopy) = False
		mAllDn = False : CheckAllDn False
	End Sub

	Public Sub SolDropDown(aEnabled)
		Dim mSwcopy
		Dim ii : If Not aEnabled Then Exit Sub
		If TypeName(mDropObj(0)) = "HitTarget" Then
			If VP8sound then
				PlaySound mDropSnd
			ElseIf VP9sound then
				PlaySound mDropSnd, 1, 1, CoreAudioPan(mDropObj(0).x), 0
			Else
				PlaySound mDropSnd, 1, 1, CoreAudioPan(mDropObj(0).x), 0, 0, False, False, CoreAudioFade(mDropObj(0).y)
			End If
		Else
			PlaySound mDropSnd
		End If

		For Each ii In mDropObj : vpmSolWall ii, False, True : Next
		For Each ii In mDropSw	: mSwcopy = ii : Controller.Switch(mSwcopy) = True : Next
		mAllDn = True : CheckAllDn True
	End Sub

	Public Sub SolDropUp(aEnabled)
		Dim mSwcopy
		Dim ii : If Not aEnabled Then Exit Sub
		If TypeName(mDropObj(0)) = "HitTarget" Then
			If VP8sound then
				PlaySound mRaiseSnd
			ElseIf VP9sound then
				PlaySound mRaiseSnd, 1, 1, CoreAudioPan(mDropObj(0).x), 0
			Else
				PlaySound mRaiseSnd, 1, 1, CoreAudioPan(mDropObj(0).x), 0, 0, False, False, CoreAudioFade(mDropObj(0).y)
			End If
		Else
			PlaySound mRaiseSnd
		End If

		For Each ii In mDropObj : vpmSolWall ii, False, False : Next
		For Each ii In mDropSw	: mSwcopy = ii : Controller.Switch(mSwcopy) = False : Next
		mAllDn = False : CheckAllDn False
	End Sub

	Public Sub DropSol_On : SolDropUp True : End Sub
End Class

'--------------------
'		Magnet
'--------------------
Class cvpmMagnet
	Private mEnabled, mBalls, mTrigger
	Public X, Y, Strength, Size, GrabCenter, Solenoid

	Private Sub Class_Initialize
		Size = 1 : Strength = 0 : Solenoid = 0 : mEnabled = False
		Set mBalls = New cvpmDictionary
	End Sub

	Private Property Let NeedUpdate(aEnabled) : vpmTimer.EnableUpdate Me, True, aEnabled : End Property

	Public Sub InitMagnet(aTrigger, aStrength)
		Dim tmp
		If vpmIsArray(aTrigger) Then Set tmp = aTrigger(0) Else Set tmp = aTrigger
		X = tmp.X : Y = tmp.Y : Size = tmp.Radius : vpmTimer.InitTimer tmp, True
		If IsArray(aTrigger) Then mTrigger = aTrigger Else Set mTrigger = aTrigger
		Strength = aStrength : GrabCenter = aStrength > 14
	End Sub

	Public Sub CreateEvents(aName)
		If vpmCheckEvent(aName, Me) Then
			vpmBuildEvent mTrigger, "Hit", aName & ".AddBall ActiveBall"
			vpmBuildEvent mTrigger, "UnHit", aName & ".RemoveBall ActiveBall"
		End If
	End Sub

	Public Property Let MagnetOn(aEnabled) : mEnabled = aEnabled : End Property
	Public Property Get MagnetOn
		If Solenoid > 0 Then MagnetOn = Controller.Solenoid(Solenoid) Else MagnetOn = mEnabled
	End Property

	Public Sub AddBall(aBall)
		With mBalls
			If .Exists(aBall) Then .Item(aBall) = .Item(aBall) + 1 Else .Add aBall, 1 : NeedUpdate = True
		End With
	End Sub

	Public Sub RemoveBall(aBall)
		With mBalls
			If .Exists(aBall) Then .Item(aBall) = .Item(aBall) - 1 : If .Item(aBall) <= 0 Then .Remove aBall
			NeedUpdate = (.Count > 0)
		End With
	End Sub

	Public Property Get Balls : Balls = mBalls.Keys : End Property

	Public Sub Update
		Dim obj
		If MagnetOn Then
			On Error Resume Next
			For Each obj In mBalls.Keys
				If obj.X < 0 Or Err Then mBalls.Remove obj Else AttractBall obj
			Next
			On Error Goto 0
		End If
	End Sub

	Public Sub AttractBall(aBall)
		Dim dX, dY, dist, force, ratio
		dX = aBall.X - X : dY = aBall.Y - Y : dist = Sqr(dX*dX + dY*dY)
		If dist > Size Or dist < 1 Then Exit Sub 'Just to be safe
		If GrabCenter And dist < 20 Then
			aBall.VelX = 0 : aBall.VelY = 0 : aBall.X = X : aBall.Y = Y
		Else
			ratio = dist / (1.5*Size)
			force = Strength * exp(-0.2/ratio)/(ratio*ratio*56) * 1.5
			aBall.VelX = (aBall.VelX - dX * force / dist) * 0.985
			aBall.VelY = (aBall.VelY - dY * force / dist) * 0.985
		End if
	End Sub
	' obsolete
	Public Property Let Range(aSize) : Size = aSize : End Property
	Public Property Get Range		 : Range = Size : End Property
End Class

'--------------------
'	  Turntable
'--------------------
Class cvpmTurntable
	Private mX, mY, mSize, mTrigger, mBalls, mSpinUp, mSpinDown
	Private mMotorOn, mSpinCW
	Private mMaxSpeed, mTargetSpeed, mCurrentAccel
	Public Speed

	Private Sub Class_Initialize
		Set mBalls = New cvpmDictionary
		mMotorOn = False : mSpinCW = True : Speed = 0 : mSpinUp = 10 : mSpinDown = 4
		AdjustTargets
	End Sub

	Private Property Let NeedUpdate(aEnabled) : vpmTimer.EnableUpdate Me, True, aEnabled : End Property

	Public Sub InitTurntable(aTrigger, aMaxSpeed)
		mX = aTrigger.X : mY = aTrigger.Y : mSize = aTrigger.Radius : vpmTimer.InitTimer aTrigger, True
		mMaxSpeed = aMaxSpeed : Set mTrigger = aTrigger
		AdjustTargets
	End Sub

	Public Sub CreateEvents(aName)
		If vpmCheckEvent(aName, Me) Then
			vpmBuildEvent mTrigger, "Hit", aName & ".AddBall ActiveBall"
			vpmBuildEvent mTrigger, "UnHit", aName & ".RemoveBall ActiveBall"
		End If
	End Sub

	Public Sub SolMotorState(aCW, aMotorOn)
		mSpinCW = aCW
		mMotorOn = aMotorOn
		AdjustTargets
	End Sub

	Private Sub AdjustTargets
		If mMotorOn Then
			mTargetSpeed = MaxSpeed
			mCurrentAccel = SpinUp
			If Not mSpinCW Then mTargetSpeed = -MaxSpeed
		Else
			mTargetSpeed = 0
			mCurrentAccel = SpinDown
		End If

		NeedUpdate = mBalls.Count Or SpinUp Or SpinDown
	End Sub

	Public Property Let MaxSpeed(newSpeed) : mMaxSpeed = newSpeed : AdjustTargets : End Property
	Public Property Let SpinUp(newRate) : mSpinUp = newRate : AdjustTargets : End Property
	Public Property Let SpinDown(newRate) : mSpinDown = newRate : AdjustTargets : End Property

	Public Property Get MaxSpeed : MaxSpeed = mMaxSpeed : End Property
	Public Property Get SpinUp : SpinUp = mSpinup : End Property
	Public Property Get SpinDown : SpinDown = mSpinDown : End Property

	Public Property Let MotorOn(aEnabled) : SolMotorState mSpinCW, aEnabled : End Property
	Public Property Let SpinCW(aCW) : SolMotorState aCW, mMotorOn : End Property

	Public Property Get MotorOn : MotorOn = mMotorOn : End Property
	Public Property Get SpinCW : SpinCW = mSpinCW : End Property

	Public Sub AddBall(aBall)
		On Error Resume Next : mBalls.Add aBall,0 : NeedUpdate = True
	End Sub
	Public Sub RemoveBall(aBall)
		On Error Resume Next
		mBalls.Remove aBall : NeedUpdate = mBalls.Count Or SpinUp Or SpinDown
	End Sub
	Public Property Get Balls : Balls = mBalls.Keys : End Property

	Public Sub Update
		If Speed > mTargetSpeed Then
			Speed = Speed - mCurrentAccel/100
			If Speed < mTargetSpeed Then Speed = mTargetSpeed : NeedUpdate = mBalls.Count
		ElseIf Speed < mTargetSpeed Then
			Speed = Speed + mCurrentAccel/100
			If Speed > mTargetSpeed Then Speed = mTargetSpeed : NeedUpdate = mBalls.Count
		End If

		If Speed Then
			Dim obj
			On Error Resume Next
			For Each obj In mBalls.Keys
				If obj.X < 0 Or Err Then mBalls.Remove obj Else AffectBall obj
			Next
			On Error Goto 0
		End If
	End Sub

	Public Sub AffectBall(aBall)
		Dim dX, dY, dist
		dX = aBall.X - mX : dY = aBall.Y - mY : dist = Sqr(dX*dX + dY*dY)
		If dist > mSize Or dist < 1 Or Speed = 0 Then Exit Sub
		aBall.VelX = aBall.VelX - (dY * Speed / 8000)
		aBall.VelY = aBall.VelY + (dX * Speed / 8000)
	End Sub
End Class

'--------------------
'	  Mech
'--------------------
Const vpmMechLinear	   = &H00
Const vpmMechNonLinear = &H01
Const vpmMechCircle	   = &H00
Const vpmMechStopEnd   = &H02
Const vpmMechReverse   = &H04
Const vpmMechOneSol	   = &H00
Const vpmMechOneDirSol = &H10
Const vpmMechTwoDirSol = &H20
Const vpmMechStepSol   = &H40
Const vpmFourStepSol   = &H60
Const vpmMechSlow	   = &H00
Const vpmMechFast	   = &H80
Const vpmMechStepSw	   = &H00
Const vpmMechLengthSw  = &H100

Class cvpmMech
	Public Sol1, Sol2, MType, Length, Steps, Acc, Ret
	Private mMechNo, mNextSw, mSw(), mLastPos, mLastSpeed, mCallback

	Private Sub Class_Initialize
		ReDim mSw(10)
		gNextMechNo = gNextMechNo + 1 : mMechNo = gNextMechNo : mNextSw = 0 : mLastPos = 0 : mLastSpeed = 0
		MType = 0 : Length = 0 : Steps = 0 : Acc = 0 : Ret = 0 : vpmTimer.addResetObj Me
	End Sub

	Public Sub AddSw(aSwNo, aStart, aEnd)
		mSw(mNextSw) = Array(aSwNo, aStart, aEnd, 0)
		mNextSw = mNextSw + 1
	End Sub

	Public Sub AddPulseSwNew(aSwNo, aInterval, aStart, aEnd)
		If Controller.Version >= "01200000" Then
			mSw(mNextSw) = Array(aSwNo, aStart, aEnd, aInterval)
		Else
			mSw(mNextSw) = Array(aSwNo, -aInterval, aEnd - aStart + 1, 0)
		End If
		mNextSw = mNextSw + 1
	End Sub

	Public Sub Start
		Dim sw, ii
		With Controller
			.Mech(1) = Sol1 : .Mech(2) = Sol2 : .Mech(3) = Length
			.Mech(4) = Steps : .Mech(5) = MType : .Mech(6) = Acc : .Mech(7) = Ret
			ii = 10
			For Each sw In mSw
				If IsArray(sw) Then
					.Mech(ii) = sw(0) : .Mech(ii+1) = sw(1)
					.Mech(ii+2) = sw(2) : .Mech(ii+3) = sw(3)
					ii = ii + 10
				End If
			Next
			.Mech(0) = mMechNo
		End With
		If IsObject(mCallback) Then mCallBack 0, 0, 0 : mLastPos = 0 : vpmTimer.EnableUpdate Me, False, True
	End Sub

	Public Property Get Position : Position = Controller.GetMech(mMechNo) : End Property
	Public Property Get Speed	 : Speed = Controller.GetMech(-mMechNo)   : End Property
	Public Property Let Callback(aCallBack) : Set mCallback = aCallBack   : End Property

	Public Sub Update
		Dim currPos, speed
		currPos = Controller.GetMech(mMechNo)
		speed = Controller.GetMech(-mMechNo)
		If currPos < 0 Or (mLastPos = currPos And mLastSpeed = speed) Then Exit Sub
		mCallBack currPos, speed, mLastPos : mLastPos = currPos : mLastSpeed = speed
	End Sub

	Public Sub Reset : Start : End Sub
	' Obsolete
	Public Sub AddPulseSw(aSwNo, aInterval, aLength) : AddSw aSwNo, -aInterval, aLength : End Sub
End Class

'--------------------
'	Captive Ball
'--------------------
Class cvpmCaptiveBall
	Private mBallKicked, mBallDir, mBallCos, mBallSin, mTrigHit
	Private mTrig, mWall, mKickers, mVelX, mVelY, mKickNo
	Public ForceTrans, MinForce, RestSwitch, NailedBalls

	Private Sub Class_Initialize
		mBallKicked = False : ForceTrans = 0.5 : mTrigHit = False : MinForce = 3 : NailedBalls = 0
		vpmTimer.addResetObj Me
	End Sub

	Public Sub InitCaptive(aTrig, aWall, aKickers, aBallDir)
		Set mTrig = aTrig : Set mWall = aWall
		mKickNo = vpmSetArray(mKickers, aKickers)
		mBallDir = aBallDir : mBallCos = Cos(aBallDir * (3.14159265358979/180)) : mBallSin = Sin(aBallDir * (3.14159265358979/180))
	End Sub

	Public Sub Start
		Dim mSwcopy
		vpmCreateBall mKickers(mKickNo + (mKickNo <> NailedBalls))
		If RestSwitch Then
			mSwcopy = RestSwitch
			Controller.Switch(mSwcopy) = True
		End If
	End Sub

	Public Sub TrigHit(aBall)
		mTrigHit = IsObject(aBall) : If mTrigHit Then mVelX = aBall.VelX : mVelY = aBall.VelY
	End Sub

	Public Sub Reset
		Dim mSwcopy
		If RestSwitch Then
			mSwcopy = RestSwitch
			Controller.Switch(mSwcopy) = True
		End If
	End Sub

	Public Sub BallHit(aBall)
		Dim dX, dY, force, mSwcopy
		If mBallKicked Then Exit Sub ' Ball is not here
		If mTrigHit Then mTrigHit = False Else mVelX = aBall.VelX : mVelY = aBall.VelY
		dX = aBall.X - mKickers(0).X : dY = aBall.Y - mKickers(0).Y
		force = -ForceTrans * (dY * mVelY + dX * mVelX) * (dY * mBallCos + dX * mBallSin) / (dX*dX + dY*dY)
		If force < 1 Then Exit Sub
		If force < MinForce Then force = MinForce
		If mKickNo <> NailedBalls Then
			vpmCreateBall mKickers(mKickNo)
			mKickers(mKickNo-1).DestroyBall
		End If
		mKickers(mKickNo).Kick mBallDir, force : mBallKicked = True
		If RestSwitch Then
			mSwcopy = RestSwitch
			Controller.Switch(mSwcopy) = False
		End If
	End Sub

	Public Sub BallReturn(aKicker)
		Dim mSwcopy
		If mKickNo <> NailedBalls Then vpmCreateBall mKickers(mKickNo-1) : aKicker.DestroyBall
		mBallKicked = False
		If RestSwitch Then
			mSwcopy = RestSwitch
			Controller.Switch(mSwcopy) = True
		End If
	End Sub

	Public Sub CreateEvents(aName)
		If vpmCheckEvent(aName, Me) Then
			If Not mTrig Is Nothing Then
				vpmBuildEvent mTrig, "Hit", aName & ".TrigHit ActiveBall"
				vpmBuildEvent mTrig, "UnHit", aName & ".TrigHit 0"
			End If
			vpmBuildEvent mWall, "Hit", aName & ".BallHit ActiveBall"
			vpmBuildEvent mKickers(mKickNo), "Hit", aName & ".BallReturn Me"
		End If
	End Sub
	' Obsolete
	Public BallImage, BallColour
End Class

'--------------------
'	Visible Locks
'--------------------
Class cvpmVLock
	Private mTrig, mKick, mSw(), mSize, mBalls, mGateOpen, mRealForce, mBallSnd, mNoBallSnd
	Public ExitDir, ExitForce, KickForceVar

	Private Sub Class_Initialize
		mBalls = 0 : ExitDir = 0 : ExitForce = 0 : KickForceVar = 0 : mGateOpen = False
		vpmTimer.addResetObj Me
	End Sub

	Public Sub InitVLock(aTrig, aKick, aSw)
		Dim ii
		mSize = vpmSetArray(mTrig, aTrig)
		If vpmSetArray(mKick, aKick) <> mSize Then MsgBox "cvpmVLock: Unmatched kick+trig" : Exit Sub
		On Error Resume Next
		ReDim mSw(mSize)
		If IsArray(aSw) Then
			For ii = 0 To UBound(aSw) : mSw(ii) = aSw(ii) : Next
		ElseIf aSw = 0 Or Err Then
			For ii = 0 To mSize: mSw(ii) = mTrig(ii).TimerInterval : Next
		Else
			mSw(0) = aSw
		End If
	End Sub

	Public Sub InitSnd(aBall, aNoBall) : mBallSnd = aBall : mNoBallSnd = aNoBall : End Sub
	Public Sub CreateEvents(aName)
		Dim ii
		If Not vpmCheckEvent(aName, Me) Then Exit Sub
		For ii = 0 To mSize
			vpmBuildEvent mTrig(ii), "Hit", aName & ".TrigHit ActiveBall," & ii+1
			vpmBuildEvent mTrig(ii), "Unhit", aName & ".TrigUnhit ActiveBall," & ii+1
			vpmBuildEvent mKick(ii), "Hit", aName & ".KickHit " & ii+1
		Next
	End Sub

	Public Sub SolExit(aEnabled)
		Dim ii, mSwcopy
		mGateOpen = aEnabled
		If Not aEnabled Then Exit Sub
		If mBalls > 0 Then
			If isObject(mKick(0)) Then
				If VP8sound then
					PlaySound mBallSnd
				ElseIf VP9sound then
					PlaySound mBallSnd, 1, 1, CoreAudioPan(mKick(0).x), 0
				Else
					PlaySound mBallSnd, 1, 1, CoreAudioPan(mKick(0).x), 0, 0, False, False, CoreAudioFade(mKick(0).y)
				End If
			Else
				PlaySound mBallSnd
			End If
		Else
			If isObject(mKick(0)) Then
				If VP8sound then
					PlaySound mNoBallSnd
				ElseIf VP9sound then
					PlaySound mNoBallSnd, 1, 1, CoreAudioPan(mKick(0).x), 0
				Else
					PlaySound mNoBallSnd, 1, 1, CoreAudioPan(mKick(0).x), 0, 0, False, False, CoreAudioFade(mKick(0).y)
				End If
			Else
				PlaySound mNoBallSnd
			End If
			Exit Sub
		End If
		For ii = 0 To mBalls-1
			mKick(ii).Enabled = False
			If mSw(ii) Then
				mSwcopy = mSw(ii)
				Controller.Switch(mSwcopy) = False
			End If
		Next
		If ExitForce > 0 Then ' Up
			mRealForce = ExitForce + (Rnd - 0.5)*KickForceVar : mKick(mBalls-1).Kick ExitDir, mRealForce
		Else ' Down
			mKick(0).Kick 0, 0
		End If
	End Sub

	Public Sub Reset
		Dim mSwcopy
		Dim ii : If mBalls = 0 Then Exit Sub
		For ii = 0 To mBalls-1
			If mSw(ii) Then
				mSwcopy = mSw(ii)
				Controller.Switch(mSwcopy) = True
			End If
		Next
	End Sub

	Public Property Get Balls : Balls = mBalls : End Property

	Public Property Let Balls(aBalls)
		Dim mSwcopy
		Dim ii : mBalls = aBalls
		For ii = 0 To mSize
			mSwcopy = mSw(ii)
			If ii >= aBalls Then
				mKick(ii).DestroyBall : If mSwcopy Then Controller.Switch(mSwcopy) = False
			Else
				vpmCreateBall mKick(ii) : If mSwcopy Then Controller.Switch(mSwcopy) = True
			End If
		Next
	End Property

	Public Sub TrigHit(aBall, aNo)
		Dim mSwcopy
		aNo = aNo - 1
		If mSw(aNo) Then
			mSwcopy = mSw(aNo)
			Controller.Switch(mSwcopy) = True
		End If
		If aBall.VelY < -1 Then Exit Sub ' Allow small upwards speed
		If aNo = mSize Then mBalls = mBalls + 1
		If mBalls > aNo Then mKick(aNo).Enabled = Not mGateOpen
	End Sub

	Public Sub TrigUnhit(aBall, aNo)
		Dim mSwcopy
		aNo = aNo - 1
		If mSw(aNo) Then
			mSwcopy = mSw(aNo)
			Controller.Switch(mSwcopy) = False
		End If
		If aBall.VelY > -1 Then
			If aNo = 0 Then mBalls = mBalls - 1
			If aNo < mSize Then mKick(aNo+1).Kick 0, 0
		Else
			If aNo = mSize Then mBalls = mBalls - 1
			If aNo > 0 Then mKick(aNo-1).Kick ExitDir, mRealForce
		End If
	End Sub

	Public Sub KickHit(aNo) : mKick(aNo-1).Enabled = False : End Sub
End Class

'--------------------
'	View Dips
'--------------------
Class cvpmDips
	Private mLWF, mChkCount, mOptCount, mItems0(), mItems1(), mItems2(), mItems3(), mItems4()

	Private Sub Class_Initialize
		ReDim mItems0(100), mItems1(100), mItems2(100), mItems3(100), mItems4(100)
	End Sub

	Private Sub addChkBox(aType, aLeft, aTop, aWidth, aNames)
		Dim ii, obj
		If Not isObject(mLWF) Then Exit Sub
		For ii = 0 To UBound(aNames) Step 2
			Set obj = mLWF.AddCtrl("chkBox", 10+aLeft, 5+aTop+ii*7, aWidth, 14, aNames(ii))
			mChkCount = mChkCount + 1
			mItems0(mChkCount+mOptCount) = aType
			Set mItems1(mChkCount+mOptCount) = obj
			mItems2(mChkCount+mOptCount) = mChkCount
			mItems3(mChkCount+mOptCount) = aNames(ii+1)
			mItems4(mChkCount+mOptCount) = aNames(ii+1)
		Next
	End Sub

	Private Sub addOptBox(aType, aLeft, aTop, aWidth, aHeading, aMask, aNames)
		Dim ii, obj
		If Not isObject(mLWF) Then Exit Sub
		mLWF.AddCtrl "Frame", 10+aLeft, 5+aTop, 10+aWidth, 7*UBound(aNames)+25, aHeading
		If aMask Then
			For ii = 0 To UBound(aNames) Step 2
				Set obj = mLWF.AddCtrl("OptBtn", 10+aLeft+5, 5+aTop+ii*7+14, aWidth, 14, aNames(ii))
				mOptCount = mOptCount + 1
				mItems0(mChkCount+mOptCount) = aType+2
				Set mItems1(mChkCount+mOptCount) = obj
				mItems2(mChkCount+mOptCount) = mOptCount
				mItems3(mChkCount+mOptCount) = aNames(ii+1)
				mItems4(mChkCount+mOptCount) = aMask
			Next
		Else
			addChkBox aType, 5+aLeft, 15+aTop, aWidth, aNames
		End If
	End Sub

	Public Sub addForm(ByVal aWidth, aHeight, aName)
		If aWidth < 80 Then aWidth = 80
		On Error Resume Next
		Set mLWF = CreateObject("VPinMAME.WSHDlg") : If Err Then Exit Sub
		With mLWF
			.x = -1 : .y = -1 ' : .w = aWidth : .h = aHeight+60
			.Title = aName : .AddCtrl "OKBtn", -1, -1, 70, 25, "&Ok"
		End With
		mChkCount = 0 : mOptCount = 0
	End Sub

	Public Sub addChk(aLeft, aTop, aWidth, aNames)
		addChkBox 0, aLeft, aTop, aWidth, aNames
	End Sub
	Public Sub addChkExtra(aLeft, aTop, aWidth, aNames)
		addChkBox 1, aLeft, aTop, aWidth, aNames
	End Sub
	Public Sub addFrame(aLeft, aTop, aWidth, aHeading, aMask, aNames)
		addOptBox 0, aLeft, aTop, aWidth, aHeading, aMask, aNames
	End Sub
	Public Sub addFrameExtra(aLeft, aTop, aWidth, aHeading, aMask, aNames)
		addOptBox 1, aLeft, aTop, aWidth, aHeading, aMask, aNames
	End Sub

	Public Sub addLabel(aLeft, aTop, aWidth, aHeight, aCaption)
		If Not isObject(mLWF) Then Exit Sub
		mLWF.AddCtrl "Label", 10+aLeft, 5+aTop, aWidth, aHeight, aCaption
	End Sub

	Public Sub viewDips : viewDipsExtra 0 : End Sub
	Public Function viewDipsExtra(aExtra)
		Dim dips(1), ii, useDip
		If Not isObject(mLWF) Then Exit Function
		With Controller
			dips(0) = .Dip(0) + .Dip(1)*256 + .Dip(2)*65536 + (.Dip(3) And &H7f)*&H1000000
			If .Dip(3) And &H80 Then dips(0) = dips(0) Or &H80000000 'workaround for overflow error
		End With
		useDip = False : dips(1) = aExtra
		For ii = 1 To mChkCount + mOptCount
			mItems1(ii).Value = -((dips(mItems0(ii) And &H01) And mItems4(ii)) = mItems3(ii))
			If (mItems0(ii) And &H01) = 0 Then useDip = True
		Next
		If vpmVPVer >= 10800 Then ShowCursor = True
		mLWF.Show GetPlayerHWnd
		If vpmVPVer >= 10800 Then ShowCursor = False
		dips(0) = 0 : dips(1) = 0
		For ii = 1 To mChkCount + mOptCount
			If mItems1(ii).Value Then dips(mItems0(ii) And &H01) = dips(mItems0(ii) And &H01) Or mItems3(ii)
		Next
		If useDip Then
			With Controller
				.Dip(0) =  (dips(0) And 255)
				.Dip(1) = ((dips(0) And 65280)\256) And 255
				.Dip(2) = ((dips(0) And &H00ff0000)\65536) And 255
				.Dip(3) = ((dips(0) And &Hff000000)\&H01000000) And 255
			End With
		End If
		viewDipsExtra = dips(1)
	End Function
End Class

'--------------------
'	Impulse Plunger
'--------------------
Class cvpmImpulseP
	Private mEnabled, mBalls, mTrigger, mEntrySnd, mExitSnd, MExitSndBall
	Public X, Y, Strength, Res, Size, Solenoid, IMPowerOut, Time, mCount, Pull, IMPowerTrans, cFactor, Auto, RandomOut, SwitchNum, SwitchOn, BallOn

	Private Sub Class_Initialize
		Size = 1 : Strength = 0 : Solenoid = 0 : Res = 1 : IMPowerOut = 0 : Time = 0 : mCount = 0 : mEnabled = False
		Pull = 0 : IMPowerTrans = 0 : Auto = False : RandomOut = 0 : SwitchOn = 0 : SwitchNum = 0 : BallOn = 0
		Set mBalls = New cvpmDictionary
	End Sub

	Private Property Let NeedUpdate(aEnabled) : vpmTimer.EnableUpdate Me, True, aEnabled : End Property

	Public Sub InitImpulseP(aTrigger, aStrength, aTime)
		Dim tmp
		If vpmIsArray(aTrigger) Then Set tmp = aTrigger(0) Else Set tmp = aTrigger
		X = tmp.X : Y = tmp.Y : Size = tmp.Radius : vpmTimer.InitTimer tmp, True
		If IsArray(aTrigger) Then mTrigger = aTrigger Else Set mTrigger = aTrigger
		Strength = aStrength
		Res = 500
		Time = aTime
		If aTime = 0 Then
			Auto = True
		Else
			cFactor = (Res / Time) / 100
			Auto = False
		End If
	End Sub

	Public Sub CreateEvents(aName)
		If vpmCheckEvent(aName, Me) Then
			vpmBuildEvent mTrigger, "Hit", aName & ".AddBall ActiveBall"
			vpmBuildEvent mTrigger, "UnHit", aName & ".RemoveBall ActiveBall"
		End If
	End Sub


	Public Property Let PlungeOn(aEnabled) : mEnabled = aEnabled : End Property
	Public Property Get PlungeOn
		If Solenoid > 0 Then PlungeOn = Controller.Solenoid(Solenoid) Else PlungeOn = mEnabled
	End Property

	Public Sub AddBall(aBall)
		Dim mSwcopy
		With mBalls
			If .Exists(aBall) Then .Item(aBall) = .Item(aBall) + 1 Else .Add aBall, 1 : NeedUpdate = True
		End With
		If SwitchOn = True Then
			mSwcopy = SwitchNum
			Controller.Switch(mSwcopy) = 1
		End If
		BallOn = 1
	End Sub

	Public Sub RemoveBall(aBall)
		Dim mSwcopy
		With mBalls
			If .Exists(aBall) Then .Item(aBall) = .Item(aBall) - 1 : If .Item(aBall) <= 0 Then .Remove aBall
			NeedUpdate = (.Count > 0)
		End With
		If SwitchOn = True Then
			mSwcopy = SwitchNum
			Controller.Switch(mSwcopy) = 0
		End If
		BallOn = 0
	End Sub

	Public Property Get Balls : Balls = mBalls.Keys : End Property

	Public Sub Update
		Dim obj
		If pull = 1 and mCount < Res Then
			mCount = mCount + cFactor
			IMPowerTrans = mCount
			NeedUpdate = True
		Else
			IMPowerTrans = mCount
			NeedUpdate = False
		End If
		If PlungeOn Then
			On Error Resume Next
			For Each obj In mBalls.Keys
				If obj.X < 0 Or Err Then : mBalls.Remove obj : Else : PlungeBall obj : End If
			Next
			On Error Goto 0
		End If
	End Sub

	Public Sub PlungeBall(aBall)
		aBall.VelY = IMPowerOut
	End Sub

	Public Sub Random(aInput) ' Random Output Varience
		RandomOut = aInput
	End Sub

	Public Sub Fire	' Resets System and Transfer Power Value
		If Auto = True Then
		IMPowerOut = -Strength + ((Rnd) * RandomOut)
		Else
		IMPowerOut = -Strength * (IMPowerTrans + ((Rnd-0.5) * cFactor * RandomOut)) / Res
		End If
		PlungeOn = True
		Update
		PlungeOn = False
		Pull = 0 : IMPowerOut = 0 : IMPowerTrans = 0 : mCount = 0
		If BallOn = 1 Then : PlaySound mExitSndBall : Else : PlaySound mExitSnd : End If
	End Sub

	Public Sub AutoFire	' Auto-Fire Specific Call (so you don't have to change timing)
		IMPowerOut = -Strength + ((Rnd) * RandomOut)
		PlungeOn = True
		Update
		PlungeOn = False
		Pull = 0 : IMPowerOut = 0 : IMPowerTrans = 0 : mCount = 0
		If BallOn = 1 Then : PlaySound mExitSndBall : Else : PlaySound mExitSnd : End If
	End Sub

	Public Sub Pullback ' Pull Plunger
		Pull = 0 : IMPowerOut = 0 : IMPowerTrans = 0 : mCount = 0 ' reinitialize to be sure
		Pull = 1 : NeedUpdate = True
		PlaySound mEntrySnd
	End Sub

	Public Sub PullbackandRetract ' Pull Plunger and retract
		Pull = 0 : IMPowerOut = 0 : IMPowerTrans = 0 : mCount = 0 ' reinitialize to be sure
		Pull = 1 : NeedUpdate = True
		PlaySound mEntrySnd
	End Sub

	Public Sub Switch(aSw)
		SwitchOn = True
		SwitchNum = aSw
	End Sub

	Public Sub InitEntrySnd(aNoBall) : mEntrySnd = aNoBall : End Sub
	Public Sub InitExitSnd(aBall, aNoBall) : mExitSndBall = aBall : mExitSnd = aNoBall : End Sub
End Class

Set vpmTimer = New cvpmTimer
If LoadScript("NudgePlugIn.vbs") Then Set vpmNudge = New cvpmNudge2 Else Set vpmNudge = New cvpmNudge

'-------------
'cvpmFlips (FastFlips) 2 Beta 1
'-------------

'Redesigned to better support games from the solid-state flipper era, including previously unsupported games thanks to DJrobX and Stumblor!
'New Features:
'- Switches from script to rom control after a delay (100ms default, vpmflips.RomControlDelay)
'	-This works independently for each flipper, ex. the Thing Flip will not interfere even briefly with lower flippers
'- New Feature - vpmFlips.Enabled. vpmFlips.Enabled = True / False will enable / disable fastflips. (This does the same thing as vpmflipsSAM.RomControl)
'	-May be necessary to manually disable flippers for video mode on some games
'- New method to disable upper flippers without getting rom errors from the double action cab switches: call helper subs NoUpperLeftFlipper, NoUpperRightFlipper
'	-On SS games that reuse upper flipper COILS, call the appropriate helper subs from the table script: NoUpperLeftFlipper, NoUpperRightFlipper (for example AFM)
'	-On SS games that reuse upper flipper SWITCHES, you will still need the cSingleLFlip/cSingleRFlip lines!
'	-CSinglexFlip automatically disables flippers to retain legacy behavior

'Todo
'test delay - works okay but a little weird if longer than romcontroldelay
'test more tables
'test Red & Ted with the left side flippers
'test region safety
'update SAM.vbs ?

dim vpmFlips : set vpmFlips = New cvpmFlips2 : vpmFlips.Name = "vpmFlips"

Sub NoUpperLeftFlipper() : vpmFlips.FlipperSolNumber(2) = 0 : End Sub
Sub NoUpperRightFlipper() : vpmFlips.FlipperSolNumber(3) = 0 : End Sub

Function NullFunction(a) : End Function

Class cvpmFlips2 'test fastflips switches to rom control after 100ms or so delay
	Public Name, Delay, TiltObjects, Sol, DebugOn
	Public LagCompensation 'flag for solenoid jitter (may not be a problem anymore) set private

	Public FlipperSolNumber(3)	'Flipper Solenoid Number. By default these are set to use the Core constants. 0=left 1=right 2=Uleft 3=URight
	Public ButtonState(3)		'Key Flip State 'set private
	Public SolState(3)			'Rom Flip State	'set private

	'Public SubL, SubUL, SubR, SubUR 'may restore these to reduce nested calls. For now the script is compressed a bit.
	Public FlipperSub(3)	'Set to the flipper subs by .init
	Public FlipperSubRef(3)	'Set to the flipper subs by .init

	Public FlippersEnabled	'Flipper Circuit State (from the ROM)
	Public OnOff			'FastFlips Enabled. Separate from FlippersEnabled, which is the flipper circuit state	'private 'todo rename

	Public FlipAt(3)		'Flip Time in gametime	'private
	Public RomControlDelay	'Delay after flipping that Rom Controlled Flips are accepted (default 100ms)

	Public Initialized

	Private Sub Class_Initialize()
		dim idx :for idx = 0 to 3 :FlipperSub(idx) = "NullFunction" : Set FlipperSubRef(idx) = Nothing: OnOff=True: ButtonState(idx)=0:SolState(idx)=0: Next
		Delay=0: FlippersEnabled=0: DebugOn=0 : LagCompensation=0 : Sol=0 : TiltObjects=1
		RomControlDelay = 100	'RomControlDelay MS between switching to rom controlled flippers
		FlipperSolNumber(0)=sLLFlipper :FlipperSolNumber(1)=sLRFlipper :FlipperSolNumber(2)=sULFlipper : FlipperSolNumber(3)=sURFlipper
		Initialized = False
	End Sub

	Sub Init()	'called by vpmInit sub
		If Initialized Then
			MsgBox "Table script bug: cvpmFlips2.Init is called twice." & vblf & vblf & "Maybe 'vpmInit Me' is duplicated in the code ?"
			QuitPlayer 2 'CS_STOP_PLAY
		End If
		Initialized = True

		On Error Resume Next 'If there's no usesolenoids variable present, exit
			call eval(UseSolenoids) : if err then exit Sub
		On Error Goto 0
		err.clear

		'Set Solenoid
		On Error Resume Next
			'For some WPC games (IJ) that reuse upper flipper
			'switch numbers, and legacy fast flip code, disable
			'flippers if cSinglexFlip is set.
			If not cSingleLFlip Then
				if err.number = 0 then NoUpperLeftFlipper
			End If
			err.clear
			If not cSingleRFlip Then
				if err.number = 0 then NoUpperRightFlipper
			End If
			err.clear
		On Error Goto 0

		If UseSolenoids >= 2 Then
			On Error Resume Next
				If UseSolenoids > 2 Then
					Solenoid = UseSolenoids
				Else
					err.clear
					if IsEmpty(GameOnSolenoid) or Err then msgbox "VPMflips error: " & err.description
					if err = 500 then 'Error 500 - Variable not defined
						msgbox "UseSolenoids = 2 error!" & vbnewline & vbnewline & "GameOnSolenoid is not defined!" & vbnewline & _
						"System may be incompatible (Check the compatibility list) or your system scripts may be out of date"
					End If
					Solenoid = GameOnSolenoid
				End If
			On Error Goto 0

			'Set callbacks
			dim idx : for idx = 0 to 3
				If IsNumeric(FlipperSolNumber(idx)) then
					Callback(idx) = SolCallback(abs(FlipperSolNumber(idx)))
				end If
			Next

			'dim str
			'for idx = 0 to 3 : str = str & "Callback" & idx & ":" & Callback(idx) &vbnewline : Next
			'str = "init successful" &vbnewline& _
			'	"Sol=" & Solenoid & " " & sol &vbnewline& str
			'msgbox str
			'vpmFlips.DebugTestInit = True	'removed debug stuff for the moment
		End If
	End Sub

	'Index based callbacks...
	Public Property Let Callback(aIdx, ByVal aInput)
		if Not IsEmpty(aInput) then
			SolCallback(FlipperSolNumber(aIdx)) = name & ".RomFlip(" & aIdx & ")="
			SolCallbackInitialized = False
			FlipperSub(aIDX) = aInput 'hold old flipper subs
			Dim cbs: cbs = "Sub XXXFlipperSub_" & aIdx & "(state)" & vblf & aInput & " state" & vblf & "End Sub"
			ExecuteGlobal cbs
			Set FlipperSubRef(aIDX) = GetRef("XXXFlipperSub_" & aIdx)
		end if
	End Property
	Public Property Get Callback(aIdx) : CallBack = FlipperSub(aIDX) : End Property

	Public Property Let Enabled(ByVal aEnabled) 'improving choreography
		aEnabled = cBool(aEnabled)
		if aEnabled <> OnOff then 'disregard redundant updates
			OnOff = aEnabled
			dim idx, cb
			If aEnabled then 'Switch to ROM solenoid states or button states immediately
				For idx = 0 To 3
					Set cb = FlipperSubRef(idx)
					If SolState(idx) <> ButtonState(idx) And FlippersEnabled And Not cb is Nothing Then cb ButtonState(idx)
				Next
			Else
				For idx = 0 To 3
					Set cb = FlipperSubRef(idx)
					If ButtonState(idx) <> SolState(idx) And Not cb is Nothing Then cb SolState(idx)
				Next
			End If
		end If
	End Property
	Public Property Get Enabled : Enabled = OnOff : End Property

	Public Property Let Solenoid(aInput) : if isnumeric(aInput) then Sol = abs(aInput) : end if : End Property 'set solenoid
	Public Property Get Solenoid : Solenoid = sol : End Property

	Public Property Let Flip(aIdx, ByVal aEnabled) 'Key Flip: Indexed base flip... may keep may not
		aEnabled = abs(aEnabled) 'True / False is not region safe with execute. Convert to 1 or 0 instead.
		ButtonState(aIdx) = aEnabled 'track flipper button states: the game-on sol flips immediately if the button is held down
		'debug.print "Key Flip " & aIdx &" @ " & gametime & " FF ON: " & OnOff & " Circuit On? " & FlippersEnabled
		Dim cb: Set cb = FlipperSubRef(aIdx)
		If OnOff and FlippersEnabled or DebugOn then
			If Not cb is Nothing Then cb aEnabled
			FlipAt(aIdx) = GameTime
		end If
	End Property

	'call callbacks 'legacy
	Public Sub FlipL(aEnabled) : Flip(0)=aEnabled :End Sub : Public Sub FlipR(aEnabled) : Flip(1)=aEnabled :End Sub
	Public Sub FlipUL(aEnabled): Flip(2)=aEnabled :End Sub : Public Sub FlipUR(aEnabled): Flip(3)=aEnabled :End Sub

	Public Property Let RomFlip(aIdx, ByVal aEnabled)
		aEnabled = abs(aEnabled)
		SolState(aIdx) = aEnabled

		Dim cb: Set cb = FlipperSubRef(aIdx)
		If Not OnOff OR GameTime >= FlipAt(aIdx) + RomControlDelay And Not cb is Nothing Then
			cb aEnabled
			'tb.text = "Rom Flip " & aIdx & " state:" & aEnabled &vbnewline&_
			'GameTime & " >= " & FlipAt(aIdx) & "+" & RomControlDelay
			'debug.print "rom flip @ " & gametime & "solenoid:" & sol & ": " & FlippersEnabled
		end if
	End property

	Public Sub TiltSol(ByVal aEnabled) 'Handle solenoid / Delay (if delayinit)
		aEnabled = cBool(aEnabled)
		If delay > 0 and not aEnabled then 'handle delay
			vpmtimer.addtimer Delay, Name & ".FireDelay" & "'"
			LagCompensation = 1
		else
			If Delay > 0 then LagCompensation = 0
			EnableFlippers(aEnabled)
		end If
	End Sub

	Sub FireDelay() : If LagCompensation then EnableFlippers False End If : End Sub

	Public Sub EnableFlippers(ByVal aEnabled) 'private
		aEnabled = abs(aEnabled) 'Might fix TMNT issue with vpmnudge.solgameon?
		dim idx, cb
		If aEnabled Then
			For idx = 0 to 3
				Set cb = FlipperSubRef(idx)
				If Not cb is Nothing Then cb ButtonState(idx)
			Next
		End If
		FlippersEnabled = aEnabled
		If TiltObjects then vpmnudge.solgameon aEnabled
		If Not aEnabled then
			For idx = 0 To 3
				Set cb = FlipperSubRef(idx)
				If Not cb is Nothing Then cb 0
			Next
		End If
	End Sub

	'debug for finding sols
	Public Sub PrintSols() : Dim x, sols: sols=controller.solenoids: for x= 0 to uBound(sols) : if sols(x) then debug.print x & ":" & sols(x) end if : Next : End Sub

End Class

'---------------------------
' Check VP version running
'---------------------------
Private Function vpmCheckVPVer
	On Error Resume Next
	' a bug in VBS?: Err object is not cleared on Exit Function
	If VPBuildVersion < 0 Or Err Then vpmCheckVPVer = 5000 : Err.Clear : Exit Function
	If VPBuildVersion > 2806 and VPBuildVersion < 9999 Then
		vpmCheckVPVer = 6300
	ElseIf VPBuildVersion > 2721 and VPBuildVersion < 9999 Then
		vpmCheckVPVer = 6100
	ElseIf VPBuildVersion >= 920 and VPBuildVersion <= 999 Then
		vpmCheckVPVer = 9200
	ElseIf VPBuildVersion > 909 and VPBuildVersion <= 999 Then
		vpmCheckVPVer = 9100
	ElseIf VPBuildVersion >= 900 and VPBuildVersion <= 999 Then
		vpmCheckVPVer = 9000
	ElseIf VPBuildVersion >= 10000 Then
		vpmCheckVPVer = VPBuildVersion
	Else
		vpmCheckVPVer = 6000
	End If
End Function

'--------------------
' Initialise timers
'--------------------
Sub PulseTimer_Init	  : vpmTimer.InitTimer Me, False : End Sub
Sub PinMAMETimer_Init : Me.Interval = PinMAMEInterval : Me.Enabled = True : End Sub

'---------------------------------------------
' Init function called from Table_Init event
'---------------------------------------------
Public Sub vpmInit(aTable)
	If vpmVPVer >= 6000 Then
		On Error Resume Next
		If Not IsObject(GetRef(aTable.name & "_Paused")) Or Err Then Err.Clear : vpmBuildEvent aTable, "Paused", "Controller.Pause = True"
		If Not IsObject(GetRef(aTable.name & "_UnPaused")) Or Err Then Err.Clear : vpmBuildEvent aTable, "UnPaused", "Controller.Pause = False"
		If Not IsObject(GetRef(aTable.name & "_Exit")) Or Err Then Err.Clear : vpmBuildEvent aTable, "Exit", "Controller.Pause = False:Controller.Stop"
	End If
	
	' FIXME PROC does not support Modulated solenoid and will fail
	If UseModSol Then
		If Controller.Version >= 03060000 Then
			Controller.SolMask(2)=UseModSol ' 1 for modulated solenoids or 2 for new physical lamps/GI/AlphaNumSegments/solenoids
		ElseIf Controller.Version >= 02080000 Then
			Controller.SolMask(2)=1 ' legacy smoothed solenoids
			UseModSol=1
		Else
			MsgBox "Modulated Flashers/Solenoids not supported with this Visual PinMAME version (2.8 or newer is required)"
			UseModSol=0
		End If
	End If
	
	vpmFlips.Init
End Sub

' Exit function called in Table_Exit event
Public Sub vpmExit : End Sub
'------------------------------------------------------
' All classes call this function to create a ball
' Assign vpmCreateBall if you want a custom function
'------------------------------------------------------
Private Function vpmDefCreateBall(aKicker)
	If Not IsEmpty(vpmBallImage) Then aKicker.Createball.Image = vpmBallImage Else aKicker.Createball : End If
	Set vpmDefCreateBall = aKicker
End Function

Private Function vpmDefCreateBall2(aKicker)
	If Not IsEmpty(vpmBallImage) Then aKicker.Createsizedball(BSize).Image = vpmBallImage Else aKicker.Createsizedball(BSize) : End If
	Set vpmDefCreateBall2 = aKicker
End Function

Private Function vpmDefCreateBall3(aKicker)
	If Not IsEmpty(vpmBallImage) Then
		aKicker.CreateSizedBallWithMass(BSize,BMass).Image = vpmBallImage
	Else
		aKicker.CreateSizedBallWithMass BSize,BMass 'for whatever reason it doesn't work if using ()
	End If
	Set vpmDefCreateBall3 = aKicker
End Function

If vpmVPVer >= 10000 Then
	Set vpmCreateBall = GetRef("vpmDefCreateBall3")
ElseIf vpmVPVer >= 9100 Then
	Set vpmCreateBall = GetRef("vpmDefCreateBall2")
Else
	Set vpmCreateBall = GetRef("vpmDefCreateBall")
End If

Private vpmTrough ' Default Trough. Used to clear up missing balls

'-------------------
' Main Loop
'------------------
Private Const CHGNO = 0
Private Const CHGSTATE = 1
Private vpmTrueFalse : vpmTrueFalse = Array(" True", " False"," True")

Sub InitSolCallbacks
	If SolCallbackInitialized Then Exit Sub

	' Calling Execute can be an heavy operation depending on user setup as it seems that security programs like Microsoft Defender are triggered by this call
	' Therefore we add the callbacks to the script during vpmInit using ExecuteGlobal to prevent stutters during play
	Dim sol, cbs: cbs = ""
	For sol = 0 To UBound(SolCallback)
		Set SolCallbackRef(sol) = Nothing
		Set SolModCallbackRef(sol) = Nothing
		If SolCallback(sol) <> "" Then cbs = cbs & vblf & "Sub XXXSolCallback_" & sol & "(state)" & vblf & SolCallback(sol) & " state" & vblf & "End Sub"
		If SolModCallback(sol) <> "" Then cbs = cbs & vblf & "Sub XXXSolModCallback_" & sol & "(state)" & vblf & SolModCallback(sol) & " state" & vblf & "End Sub"
	Next
	If cbs <> "" Then ExecuteGlobal cbs
	For sol = 0 To UBound(SolCallback)
		If SolCallback(sol) <> "" Then Set SolCallbackRef(sol) = GetRef("XXXSolCallback_" & sol)
		If SolModCallback(sol) <> "" Then Set SolModCallbackRef(sol) = GetRef("XXXSolModCallback_" & sol)
	Next

	SolCallbackInitialized = True
End Sub

Sub vpmDoSolCallback(aNo, aEnabled)
	InitSolCallbacks
	Dim cb: Set cb = SolCallbackRef(aNo)
	If Not cb Is Nothing Then cb CBool(aEnabled)
End Sub

Sub vpmDoLampUpdate(aNo, aEnabled)
	On Error Resume Next ' FIXME remove as hiding error is not really a good idea since this leads to sharing tables with invalid script
	Lights(aNo).State = Abs(aEnabled)
	On Error Goto 0
End Sub

Sub PinMAMETimer_Timer
	Dim ChgLamp, ChgSol,ChgGI, ii, tmp, idx, ChgLed
	Dim DMDp
	Dim ChgNVRAM

	'Me.Enabled = False 'this was supposed to be some kind of weird mutex, disable it

	On Error Resume Next
		If UseDMD Then
			DMDp = Controller.RawDmdPixels
			If Not IsEmpty(DMDp) Then
				DMDWidth = Controller.RawDmdWidth
				DMDHeight = Controller.RawDmdHeight
				DMDPixels = DMDp
			End If
		ElseIf UseColoredDMD Then
			DMDp = Controller.RawDmdColoredPixels
			If Not IsEmpty(DMDp) Then
				DMDWidth = Controller.RawDmdWidth
				DMDHeight = Controller.RawDmdHeight
				DMDColoredPixels = DMDp
			End If
		End If
		If UseNVRAM Then
			If isObject(NVRAMCallback) Then
				ChgNVRAM = Controller.ChangedNVRAM 'Controller.NVRAM would deliver everything of the NVRAM all the time as 1D array
				If(Not IsEmpty(ChgNVRAM)) Then NVRAMCallback ChgNVRAM
			End If
		End If
		If UseLamps Then ChgLamp = Controller.ChangedLamps Else LampCallback
		If UsePdbLeds Then ChgLed = Controller.ChangedPDLeds Else PDLedCallback
		If UseSolenoids Then ChgSol = Controller.ChangedSolenoids
		If (Not GICallback is Nothing) Or (Not GICallback2 is Nothing) Then ChgGI = Controller.ChangedGIStrings
		MotorCallback
	On Error Goto 0

	Dim pwmScale: If UseModSol >= 2 Then pwmScale = 1.0 / 255.0 Else pwmScale = 1 ' User has activated physical output and expects a 0..1 value for solenoids/lamps/GI/AlphaNumSegments

	If Not IsEmpty(ChgSol) Then
		InitSolCallbacks
		Dim cb, nsol, state, bstate
		For ii = 0 To UBound(ChgSol)
			nsol = ChgSol(ii, 0)
			state = ChgSol(ii, 1) * pwmScale
			bstate = CBool(state >= 0.5)
			If bstate <> SolPrevState(nsol) Then
				SolPrevState(nsol) = bstate
				Set cb = SolCallbackRef(nsol)
				If Not cb Is Nothing Then cb bstate
			End If
			If UseModSol >= 1 Then
				Set cb = SolModCallbackRef(nsol)
				If Not cb Is Nothing Then cb state
			End If
			If UseSolenoids > 1 Then If nsol = vpmFlips.Solenoid Then vpmFlips.TiltSol bstate
		Next
	End If

	If Not IsEmpty(ChgLamp) Then
		On Error Resume Next
			For ii = 0 To UBound(ChgLamp)
				idx = ChgLamp(ii, 0)
				If IsArray(Lights(idx)) Then
					For Each tmp In Lights(idx) : tmp.State = ChgLamp(ii, 1) * pwmScale : Next
				Else
					Lights(idx).State = ChgLamp(ii, 1) * pwmScale
				End If
			Next
			For Each tmp In vpmMultiLights
				For ii = 1 To UBound(tmp) : tmp(ii).State = tmp(0).State : Next
			Next
			LampCallback
		On Error Goto 0
	End If

	If Not IsEmpty(ChgGI) Then
		For ii = 0 To UBound(ChgGI)
			If Not GICallback is Nothing Then GICallback ChgGI(ii, 0), CBool((ChgGI(ii, 1) * pwmScale) >= 0.5)
			If Not GICallback2 is Nothing Then GICallback2 ChgGI(ii, 0), ChgGI(ii, 1) * pwmScale
		Next
	End If

	If Not IsEmpty(ChgLed) Then
		On Error Resume Next
			For ii = 0 To UBound(ChgLed)
				Dim color,ledstate
				idx = ChgLed(ii, 0)
				color = ChgLed(ii, 1)
				if color = 0 Then ledstate = 0 : Else ledstate = 1: End If
				If IsArray(Lights(idx)) Then
					For Each tmp In Lights(idx) : tmp.Color = color : tmp.State = ledstate : Next
				Else
					Lights(idx).Color = color : Lights(idx).State = ledstate
				End If
			Next
			For Each tmp In vpmMultiLights
				For ii = 1 To UBound(tmp) : tmp(ii).Color = tmp(0).Color : tmp(ii).State = tmp(0).State : Next
			Next
			PDLedCallback
		On Error Goto 0
	End If
	'Me.Enabled = True 'this was supposed to be some kind of weird mutex, disable it
End Sub

'
' Private helper functions
'
Private Sub vpmPlaySound(aEnabled, aSound)
	If VarType(aSound) = vbString Then
		If aEnabled Then StopSound aSound : PlaySound aSound
	ElseIf aSound Then
		If aEnabled Then PlaySound SSolenoidOn Else PlaySound SSolenoidOff
	End If
End Sub

Private Sub vpmToggleObj(aObj, aEnabled)
	Dim mSwcopy
	Select Case TypeName(aObj)
		Case "Wall", "HitTarget"		   aObj.IsDropped = aEnabled
		Case "Bumper", "Light"			   aObj.State	  = Abs(aEnabled)
		Case "Kicker", "Trigger", "Timer"  aObj.Enabled	  = aEnabled
		Case "Gate"						   aObj.Open	  = aEnabled
		Case "Primitive", "Ramp", "Rubber", "Flasher" aObj.Visible = aEnabled
		Case "Integer"					   mSwcopy = aObj : Controller.Switch(mSwcopy) = aEnabled
		Case Else MsgBox "vpmToggleObj: Unhandled Object " & TypeName(aObj)
	End Select
End Sub

Private Function vpmCheckEvent(aName, aObj)
	vpmCheckEvent = True
	On Error Resume Next
	If Not Eval(aName) Is aObj Or Err Then MsgBox "CreateEvents: Wrong name " & aName : vpmCheckEvent = False
End Function

Private Sub vpmBuildEvent(aObj, aEvent, aTask)
	Dim obj, str
	str = "_" & aEvent & " : " & aTask & " : End Sub"
	If vpmIsArray(aObj) Then
		For Each obj In aObj : ExecuteGlobal "Sub " & obj.Name & str : Next
	Else
		ExecuteGlobal "Sub " & aObj.Name & str
	End If
End Sub

Private Function vpmIsCollection(aObj)
	vpmIsCollection = TypeName(aObj) = "Collection" Or TypeName(aObj) = "ICollection"
End Function
Private Function vpmIsArray(aObj)
	vpmIsArray = IsArray(aObj) Or vpmIsCollection(aObj)
End Function

Private Function vpmSetArray(aTo, aFrom)
	If IsArray(aFrom) Then
		aTo = aFrom : vpmSetArray = UBound(aFrom)
	ElseIf vpmIsCollection(aFrom) Then
		Set aTo = aFrom : vpmSetArray = aFrom.Count - 1
	Else
		aTo = Array(aFrom) : vpmSetArray = 0
	End If
End Function

Sub vpmCreateEvents(aHitObjs)
	Dim obj
	For Each obj In aHitObjs
		Select Case TypeName(obj)
			Case "Trigger"
				vpmBuildEvent obj, "Hit", "Controller.Switch(" & Obj.TimerInterval & ") = True"
				vpmBuildEvent obj, "UnHit", "Controller.Switch(" & Obj.TimerInterval & ") = False"
			Case "Wall"
				If obj.HasHitEvent Then
					vpmBuildEvent obj, "Hit", "vpmTimer.PulseSw " & Obj.TimerInterval
				Else
					vpmBuildEvent obj, "SlingShot", "vpmTimer.PulseSw " & Obj.TimerInterval
				End If
			Case "Bumper", "Gate", "Primitive", "HitTarget", "Rubber"
				vpmBuildEvent obj, "Hit", "vpmTimer.PulseSw " & Obj.TimerInterval
			Case "Spinner"
				vpmBuildEvent obj, "Spin", "vpmTimer.PulseSw " & Obj.TimerInterval
		End Select
	Next
End Sub

Sub vpmMapLights(aLights)
	Dim obj, str, ii, idx
	For Each obj In aLights
		idx = obj.TimerInterval
		If IsArray(Lights(idx)) Then
			str = "Lights(" & idx & ") = Array("
			For Each ii In Lights(idx) : str = str & ii.Name & "," : Next
			ExecuteGlobal str & obj.Name & ")"
		ElseIf IsObject(Lights(idx)) Then
			Lights(idx) = Array(Lights(idx),obj)
		Else
			Set Lights(idx) = obj
		End If
	Next
End Sub

Function vpmMoveBall(aBall, aFromKick, aToKick)
	With aToKick.CreateBall
		If TypeName(aBall) = "IBall" Then
			.Color = aBall.Color : .Image = aBall.Image
			If vpmVPVer >= 6000 Then
				.FrontDecal = aBall.FrontDecal : .BackDecal = aBall.BackDecal
'				.UserValue = aBall.UserValue
			End If
		End If
	End With
	aFromKick.DestroyBall : Set vpmMoveBall = aToKick
End Function

Sub vpmAddBall
	Dim Answer
	If IsObject(vpmTrough) Then
		If vpmVPVer >= 10800 Then ShowCursor = True
		Answer = MsgBox("Click YES to Add a ball to the Trough, NO Removes a ball from the Trough",vbYesNoCancel + vbQuestion + vbMsgBoxSetForeground)
		If vpmVPVer >= 10800 Then ShowCursor = False
		If Answer = vbYes Then vpmTrough.AddBall 0
		If Answer = vbNo Then vpmTrough.Balls=vpmTrough.Balls-1
	End If
End Sub

'----------------------------
' Generic solenoid handlers
'----------------------------
' ----- Flippers ------
Sub vpmSolFlipper(aFlip1, aFlip2, aEnabled)
	Dim oldStrength, oldSpeed ' only for pre-VP10
	If aEnabled Then
		PlaySound SFlipperOn : aFlip1.RotateToEnd : If Not aFlip2 Is Nothing Then aFlip2.RotateToEnd
	Else
		PlaySound SFlipperOff
		If vpmVPVer < 10000 Then
			oldStrength = aFlip1.Strength : aFlip1.Strength = conFlipRetStrength
			oldSpeed = aFlip1.Speed : aFlip1.Speed = conFlipRetSpeed
		End If
		aFlip1.RotateToStart
		If vpmVPVer < 10000 Then
			aFlip1.Strength = oldStrength
			aFlip1.Speed = oldSpeed
		End If
		If Not aFlip2 Is Nothing Then
			If vpmVPVer < 10000 Then
				oldStrength = aFlip2.Strength : aFlip2.Strength = conFlipRetStrength
				oldSpeed = aFlip2.Speed : aFlip2.Speed = conFlipRetSpeed
			End If
			aFlip2.RotateToStart
			If vpmVPVer < 10000 Then
				aFlip2.Strength = oldStrength
				aFlip2.Speed = oldSpeed
			End If
		End If
	End If
End Sub

' ----- Flippers With Speed Control ------
Sub vpmSolFlip2(aFlip1, aFlip2, aFlipSpeedUp, aFlipSpeedDn, aSnd, aEnabled) ' DEPRECATED, as VP10 does not feature speed on flippers anymore
	Dim oldStrength, oldSpeed
	If aEnabled Then
		If aSnd = true then : PlaySound SFlipperOn : End If
		If Not aFlipSpeedUp = 0 Then
			aFlip1.Speed = aFlipSpeedUp
			aFlip1.RotateToEnd
		Else
			aFlip1.RotateToEnd
		End If
		If Not aFlip2 Is Nothing Then
			If Not aFlipSpeedUp = 0 Then
				aFlip2.Speed = aFlipSpeedUp
				aFlip2.RotateToEnd
			Else
				aFlip2.RotateToEnd
			End If
		End If
	Else
		If aSnd = true then : PlaySound SFlipperOff : End If
		oldStrength = aFlip1.Strength
		aFlip1.Strength = conFlipRetStrength
		oldSpeed = aFlip1.Speed
		If Not aFlipSpeedDn = 0 Then
			aFlip1.Speed = aFlipSpeedDn
		Else
			aFlip1.Speed = conFlipRetSpeed
		End If
		aFlip1.RotateToStart : aFlip1.Strength = oldStrength : aFlip1.Speed = oldSpeed
		If Not aFlip2 Is Nothing Then
			oldStrength = aFlip2.Strength
			oldSpeed = aFlip2.Speed
			If Not aFlipSpeedDn = 0 Then
				aFlip2.Speed = aFlipSpeedDn
			Else
				aFlip2.Speed = conFlipRetSpeed
			End If
			aFlip2.Strength = conFlipRetStrength
			aFlip2.RotateToStart : aFlip2.Strength = oldStrength : aFlip2.Speed = oldSpeed
		End If
	End If
End Sub

' ------ Diverters ------
Sub vpmSolDiverter(aDiv, aSound, aEnabled)
	If aEnabled Then aDiv.RotateToEnd : Else aDiv.RotateToStart

'	vpmPlaySound aEnabled, aSound

	If VarType(aSound) = vbString Then
		If aEnabled Then
			StopSound aSound
			If isObject(aDiv) Then
				If VP8sound then
					PlaySound aSound
				ElseIf VP9sound then
					PlaySound aSound, 1, 1, CoreAudioPan(aDiv.x), 0
				Else
					PlaySound aSound, 1, 1, CoreAudioPan(aDiv.x), 0, 0, False, False, CoreAudioFade(aDiv.y)
				End If
			Else
				PlaySound aSound
			End If
		End If
	ElseIf aSound Then
		If aEnabled Then
			If isObject(aDiv) Then
				If VP8sound then
					PlaySound SSolenoidOn
				ElseIf VP9sound then
					PlaySound SSolenoidOn, 1, 1, CoreAudioPan(aDiv.x), 0
				Else
					PlaySound SSolenoidOn, 1, 1, CoreAudioPan(aDiv.x), 0, 0, False, False, CoreAudioFade(aDiv.y)
				End If
			Else
				PlaySound SSolenoidOn
			End If
		Else
			If isObject(aDiv) Then
				If VP8sound then
					PlaySound SSolenoidOff
				ElseIf VP9sound then
					PlaySound SSolenoidOff, 1, 1, CoreAudioPan(aDiv.x), 0
				Else
					PlaySound SSolenoidOff, 1, 1, CoreAudioPan(aDiv.x), 0, 0, False, False, CoreAudioFade(aDiv.y)
				End If
			Else
				PlaySound SSolenoidOff
			End If
		End If
	End If
End sub

' ------ Walls ------
Sub vpmSolWall(aWall, aSound, aEnabled)
	Dim obj
	If vpmIsArray(aWall) Then
		For Each obj In aWall : obj.IsDropped = aEnabled : Next
	Else
		aWall.IsDropped = aEnabled
	End If
	vpmPlaySound aEnabled, aSound
End Sub

Sub vpmSolToggleWall(aWall1, aWall2, aSound, aEnabled)
	Dim obj
	If vpmIsArray(aWall1) Then
		For Each obj In aWall1 : obj.IsDropped = aEnabled : Next
	Else
		aWall1.IsDropped = aEnabled
	End If
	If vpmIsArray(aWall2) Then
		For Each obj In aWall2 : obj.IsDropped = Not aEnabled : Next
	Else
		aWall2.IsDropped = Not aEnabled
	End If
	vpmPlaySound aEnabled, aSound
End Sub

' ------- Autoplunger ------
Sub vpmSolAutoPlunger(aPlung, aVar, aEnabled)
	Dim oldFire
	If aEnabled Then
		oldFire = aPlung.FireSpeed : aPlung.FireSpeed = oldFire * (100-aVar*(2*Rnd-1))/100
		PlaySound SSolenoidOn : aPlung.Fire : aPlung.FireSpeed = oldFire
	Else
		aPlung.Pullback
	End If
End Sub

' --------Autoplunger with Specified Sound To Play ---------
Sub vpmSolAutoPlungeS(aPlung, aSound, aVar, aEnabled)
	Dim oldFire
	If aEnabled Then
		oldFire = aPlung.FireSpeed : aPlung.FireSpeed = oldFire * (100-aVar*(2*Rnd-1))/100
		PlaySound aSound : aPlung.Fire : aPlung.FireSpeed = oldFire
	Else
		aPlung.Pullback
	End If
End Sub

' --------- Gate -----------
Sub vpmSolGate(aGate, aSound, aEnabled)
	Dim obj
	If vpmIsArray(aGate) Then
		For Each obj In aGate : obj.Open = aEnabled : Next
	Else
		aGate.Open = aEnabled
	End If
	vpmPlaySound aEnabled, aSound
End Sub

' ------ Sound Only -------
Sub vpmSolSound(aSound, aEnabled)
	If aEnabled Then StopSound aSound : PlaySound aSound
End Sub

' ------- Flashers --------
Sub vpmFlasher(aFlash, aEnabled)
	Dim obj
	If vpmIsArray(aFlash) Then
		For Each obj In aFlash : obj.State = Abs(aEnabled) : Next
	Else
		aFlash.State = Abs(aEnabled)
	End If
End Sub

'---- Generic object toggle ----
Sub vpmSolToggleObj(aObj1, aObj2, aSound, aEnabled)
	Dim obj
	If vpmIsArray(aObj1) Then
		If IsArray(aObj1(0)) Then
			For Each obj In aObj1(0) : vpmToggleObj obj, aEnabled	  : Next
			For Each obj In aObj1(1) : vpmToggleObj obj, Not aEnabled : Next
		Else
			For Each obj In aObj1	 : vpmToggleObj obj, aEnabled	  : Next
		End If
	ElseIf Not aObj1 Is Nothing Then
		vpmToggleObj aObj1, aEnabled
	End If
	If vpmIsArray(aObj2) Then
		If IsArray(aObj2(0)) Then
			For Each obj In aObj2(0) : vpmToggleObj obj, Not aEnabled : Next
			For Each obj In aObj2(1) : vpmToggleObj obj, aEnabled	  : Next
		Else
			For Each obj In aObj2	 : vpmToggleObj obj, Not aEnabled : Next
		End If
	ElseIf Not aObj2 Is Nothing Then
		vpmToggleObj aObj2, Not aEnabled
	End If
	vpmPlaySound aEnabled, aSound
End Sub

'
' Stubs to allow older games to still work
' These will be removed one day
'
Sub SolFlipper(f1,f2,e) : vpmSolFlipper f1,f2,e : End Sub
Sub SolDiverter(d,s,e) : vpmSolDiverter d,s,e : End Sub
Sub SolSound(s,e) : vpmSolSound s,e : End Sub
Sub Flasher(f,e) : vpmFlasher f,e : End Sub
Sub SolMagnet(m,e) : vpmSolMagnet m,e : End Sub
Sub SolAutoPlunger(p,e) : vpmSolAutoPlunger p,0,e : End Sub
Function KeyDownHandler(ByVal k) : KeyDownHandler = vpmKeyDown(k) : End Function
Function KeyUpHandler(ByVal k) : KeyUpHandler = vpmKeyUp(k) : End Function
Function KeyName(ByVal k) : KeyName = vpmKeyName(k) : End Function
Sub vpmSolMagnet(m,e) : m.Enabled = e : If Not e Then m.Kick 180,1 : End If : End Sub
Dim vpmBallImage : vpmBallImage = Empty ' Default ball properties
Dim vpmBallColour

'-- Flipper solenoids (all games)
Const sLRFlipper = 46
Const sLLFlipper = 48
Const sURFlipper = 34
Const sULFlipper = 36

' Convert keycode to readable string
Private keyNames1, keyNames2
keyNames1 = Array("Escape","1","2","3","4","5","6","7","8","9","0","Minus '-'",_
"Equals '='","Backspace","Tab","Q","W","E","R","T","Y","U","I","O","P","[","]",_
"Enter","Left Ctrl","A","S","D","F","G","H","J","K","L",";","'","`","Left Shift",_
"\","Z","X","C","V","B","N","M",",",".","/","Right Shift","*","Left Menu","Space",_
"Caps Lock","F1","F2","F3","F4","F5","F6","F7","F8","F9","F10","NumLock","ScrlLock",_
"Numpad 7","Numpad 8","Numpad 9","Numpad -","Numpad 4","Numpad 5","Numpad 6",_
"Numpad +","Numpad 1","Numpad 2","Numpad 3","Numpad 0","Numpad .","?","?","?",_
"F11","F12","F13","F14","F15")
keyNames2 = Array("Pause","?","Home","Up","PageUp","?","Left","?","Right","?",_
"End","Down","PageDown","Insert","Delete")

Function vpmKeyName(ByVal aKeycode)
	If aKeyCode-1 <= UBound(keyNames1) Then
		vpmKeyName = keyNames1(aKeyCode-1)
	ElseIf aKeyCode >= 197 And aKeyCode <= 211 Then
		vpmKeyName = keyNames2(aKeyCode-197)
	ElseIf aKeyCode = 184 Then
		vpmKeyName = "R.Alt"
	Else
		vpmKeyName = "?"
	End If
End Function

Private vpmSystemHelp
Private Sub vpmShowHelp
	Dim szKeyMsg
	szKeyMsg = "The following keys are defined: "				   & vbNewLine &_
			   "(American keyboard layout)"						   & vbNewLine &_
		vbNewLine & "Visual PinMAME keys:"						   & vbNewLine &_
		vpmKeyName(keyShowOpts)	  & vbTab & "Game options..."	   & vbNewLine &_
		vpmKeyName(keyShowKeys)	  & vbTab & "Keyboard settings..." & vbNewLine &_
		vpmKeyName(keyReset)	  & vbTab & "Reset emulation"	   & vbNewLine &_
		vpmKeyName(keyFrame)	  & vbTab & "Toggle Display lock"  & vbNewLine &_
		vpmKeyName(keyDoubleSize) & vbTab & "Toggle Display size"  & vbNewLine
	If IsObject(vpmShowDips) Then
		szKeyMsg = szKeyMsg & vpmKeyName(keyShowDips) & vbTab & "Show DIP Switch / Option Menu" & vbNewLine
	End If
	If IsObject(vpmTrough) Then
		szKeyMsg = szKeyMsg & vpmKeyName(keyAddBall) & vbTab & "Add / Remove Ball From Table" & vbNewLine
	End If
	szKeyMsg = szKeyMsg & vpmKeyName(keyBangBack) & vbTab & "Bang Back" & vbNewLine &_
		vbNewLine & vpmSystemHelp & vbNewLine
	If ExtraKeyHelp <> "" Then
		szKeyMsg = szKeyMsg & vbNewLine & "Game Specific keys:" &_
			vbNewLine & ExtraKeyHelp & vbNewLine
	End If
	szKeyMsg = szKeyMsg & vbNewLine & "Visual Pinball keys:"	 & vbNewLine &_
		vpmKeyName(LeftFlipperKey)	& vbTab & "Left Flipper"	 & vbNewLine &_
		vpmKeyName(RightFlipperKey) & vbTab & "Right Flipper"	 & vbNewLine &_
		vpmKeyName(LeftMagnaSave)	& vbTab & "Left Magna Save"	 & vbNewLine &_
		vpmKeyName(RightMagnaSave)	& vbTab & "Right Magna Save" & vbNewLine &_
		vpmKeyName(PlungerKey)		& vbTab & "Launch Ball"		 & vbNewLine &_
		vpmKeyName(StartGameKey)	& vbTab & "Start Button"	 & vbNewLine &_
		vpmKeyName(AddCreditKey)	& vbTab & "Insert Coin 1"	 & vbNewLine &_
		vpmKeyName(AddCreditKey2)	& vbTab & "Insert Coin 2"	 & vbNewLine &_
		vpmKeyName(ExitGame)		& vbTab & "Exit Game"		 & vbNewLine &_
		vpmKeyName(MechanicalTilt)	& vbTab & "Mechanical Tilt"	 & vbNewLine &_
		vpmKeyName(LeftTiltKey)		& vbTab & "Nudge from Left"	 & vbNewLine &_
		vpmKeyName(RightTiltKey)	& vbTab & "Nudge from Right" & vbNewLine &_
		vpmKeyName(CenterTiltKey)	& vbTab & "Nudge forward"	 & vbNewLine
	If vpmVPVer >= 10800 Then ShowCursor = True
	MsgBox szKeyMsg,vbOkOnly + vbMsgBoxSetForeground,"Keyboard Settings..."
	If vpmVPVer >= 10800 Then ShowCursor = False
End Sub

Private Sub vpmShowOptions
	If vpmVPVer >= 10800 Then ShowCursor = True
	Controller.ShowOptsDialog GetPlayerHWnd
	If vpmVPVer >= 10800 Then ShowCursor = False
End Sub

'added thanks to Koadic
Sub NVOffset(version) ' version 2 for dB2S compatibility
	Dim check,nvcheck,v,vv,nvpath,rom
	Set check = CreateObject("Scripting.FileSystemObject")
	Set nvcheck = CreateObject("WScript.Shell")
	nvpath = nvcheck.RegRead("HKCU\Software\Freeware\Visual PinMame\globals\nvram_directory") & "\"
	If Controller.Version >= 03050000 Then
		rom = controller.ROMName
	Else
		rom = controller.GameName
	End If
	For v=1 to 32 'check up to 32 possible versions using same rom, it's overkill, but could be changed to a lower number (requested for 32 NFL variations)
		If check.FileExists(nvpath & rom & " v" & v & ".txt") Then vv=v : exit For : End If
		vv=0
	Next
	If vv=version or version = 0 Then
		Exit Sub
	ElseIf vv=0 Then
		check.CreateTextFile nvpath & rom & " v" & version & ".txt", True
		Exit Sub
	Else
		check.moveFile nvpath & rom & " v" & vv & ".txt", nvpath & rom & " v" & version & ".txt"
		If check.FileExists(nvpath & rom & ".nv") Then
			check.copyFile nvpath & rom & ".nv", nvpath & rom & " v" & vv & ".nv", True
		End If
		If check.FileExists(nvpath & rom & " v" & version & ".nv") Then
			check.copyFile nvpath & rom & " v" & version & ".nv", nvpath & rom & ".nv", True
		End If
	End If
End Sub

Sub VPMVol
	Dim VolPM,VolPMNew
	VolPM = Controller.Games(controller.GameName).Settings.Value("volume")
	VolPMNew = InputBox ("Enter desired VPinMAME Volume Level (-32 to 0)","VPinMAME Volume",VolPM)
	If VolPMNew = "" Then Exit Sub
	If VolPMNew <= 0 and VolPMNew >= -32 Then
		Controller.Games(controller.GameName).Settings.Value("volume")= round(VolPMNew)
		msgbox "The Visual PinMAME Global Volume is now set to " & round(VolPMNew) & "db." & VbNewLine & VbNewLine & "Please reset Visual PinMAME (F3) to apply."
	Else
		msgbox "Entered value is out of range. Entry must be in the range of negative 32 to 0." & VbNewLine & VbNewLine & "Visual PinMAME Global Volume will remain set at " & VolPM & "."
	End If
End Sub

' Simple min/max functions
Function vpMin(a, b) : If a < b Then vpMin = a Else vpMin = b : End If : End Function
Function vpMax(a, b) : If a > b Then vpMax = a Else vpMax = b : End If : End Function

LoadScript("ledcontrol.vbs"):Err.Clear	' Checks for existance of ledcontrol.vbs and loads it if found, if found but no ledwiz installed, clear error to allow loading of table

LoadScript("GlobalPlugIn.vbs")			' Checks for existance of GlobalPlugIn.vbs and loads it if found, useful for adding
										' custom scripting that can be used for all tables instead of altering the core.vbs

Dim swidth, sheight, VP8sound, VP9sound
swidth = 950
sheight = 2100
VP8sound = False
VP9sound = False

On Error Resume Next
Err.Clear
If Version > 0 then
	If Version >= 10700 then 'no ActiveTable call until 10.7
		Dim soundtable
		Set soundtable = ActiveTable
		swidth = soundtable.Width
		sheight = soundtable.Height
	End If
	If Version < 10400 then 'no front/rear fade until 10.4
		VP9sound = True
	End If
End If

If Err.Number <> 0 then
	If VPBuildVersion >= 920 and VPBuildVersion <= 999 then 'no pan parameter in PlaySound until 9.2
		VP9sound = True
	Else
		VP8sound = True
	End If
	swidth = 950
	sheight = 2100
	Err.Clear
End If
On Error Goto 0

Private Function CoreAudioPan(xpar) 'calculates the audio pan of an table object using the actual table width or 950 for older versions
	If xpar < 0 then
		CoreAudioPan = -1.
	Else
		If xpar > swidth then
			CoreAudioPan = 1.
		Else
			CoreAudioPan = (xpar*2./swidth)-1.
		End If
	End If
End Function

Private Function CoreAudioFade(ypar) 'calculates the audio fade of an table object using the actual table length or 2100 for older versions
	If ypar < 0 then
		CoreAudioFade = -1.
	Else
		If ypar > sheight then
			CoreAudioFade = 1.
		Else
			CoreAudioFade = (ypar*2./sheight)-1.
		End If
	End If
End Function
