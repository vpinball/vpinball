Option Explicit
Const VPinMAMEDriverVer = 3.54
'=======================
' VPinMAME driver core.
'=======================
' New in 3.54 (Update by mfuegemann & nFozzy & Ninuzzu/Tom Tower & Toxie)
' - Added Class1812.vbs
' - Added inder_centaur.vbs
' - Restore basic functionality of cvpmDropTarget.CreateEvents for drop targets with an animation time (e.g. VP10 and newer)
' - Minor cleanups and code unifications for all machines
' - Add keyConfigurations to VPMKeys.vbs for Taito and also remap the hardcoded keycode '13' to keySoundDiag
'
' New in 3.53 (Update by Toxie)
' - Add more key mappings to help dialog
'
' New in 3.52 (Update by DJRobX & Toxie)
' - Change default interval of the PinMAME timer to -1 (frame-sync'ed) if VP10.2 (or newer) is running
' - Add modulated solenoids to support ROM controlled fading flashers:
'   To use, add "UseVPMModSol=True" to the table script
'   Also use SolModCallback instead of SolCallback to receive level changes as input: It will be a level from 0 to 255.
'   Just continue to use SolCallback if you only care about boolean values though, as it will only fire if level changes from on to off.
'   Note: vpmInit MUST BE CALLED or VPM will not switch modes (if you are only getting 0 and 1 from SolModCallback then that may be the issue)
'
' New in 3.51 (Update by mfuegemann & Arngrim & Toxie)
' - gts1.vbs dip fix
' - Add comments to cvpmDropTarget.CreateEvents: do not use this anymore in VP10 and above, as drop targets have an animation time nowadays
' - Change default interval of the PinMAME timer to 3 if VP10 (or newer) is running, and leave it at 1 for everything else
' - Fix missing SlingshotThreshold() when using VP8.X
' - (Controller.vbs changes)
'   - now its allowed to have each toy to be set to 0 (sound effect), 1 (DOF) or 2 (both)
'   - new DOF types: DOFFlippers, DOFTargets, DOFDropTargets
'   - all values are now stored in the registry (HKEY_CURRENT_USER\SOFTWARE\Visual Pinball\Controller\), and can also be changed from within VP10.2 and above
'   - InitializeOptions call added to the controller init, for tables that want to use this functionality during gameplay (options menu via F6)
'
' New in 3.50 (Update by Toxie & mfuegemann & Arngrim)
' - Added MAC.vbs & IronBalls.vbs & Lancelot.vbs & Antar.vbs
' - (Core changes)
'   - Increased NVOffset limit from 10 to 32
'   - Use temporary variables for Switch() calls to workaround current PROC issues
'   - Controller.vbs user-folder detection fix, and add simple PROC usage via LoadPROC (see Controller.vbs for details)
'   - Add UseVPMNVRAM = true to the table script (place before LoadVPM, or otherwise calling core.vbs)
'     to make changed content of NVRAM available (since last update) via the NVRAMCallback (delivers a three dimensional array with: location, new value, old value)
'     (requires VPM 2.7 or newer)
'
' New in 3.49 (Update by Arngrim)
' - Add new Controller.vbs to abstract DOF, B2S, VPM and EM controller loading, usage and sound/effect handling,
'   see Controller.vbs header on how to use it exactly
'
' New in 3.48 (Update by JimmyFingers)
' - (Core changes)
'   - Changed vpmNudge.TiltObj handling to use Bumper.Threshold / Wall.SlingshotThreshold temporary value changes rather than force / SlingshotStrength changes to disable tiltobj array objects
'   - There existed a bug in VP since at least the 9.x versions where the Wall.SlingshotStrength value being set by scripting during game play did change the value but the slingshot behaviour / "Slingshot Force" (from the editor) of the wall object did not change (i.e. did not have an effect); As a result the attempted disabling of bumpers and slingshots after a tilt event on supported games (that can send a relay for vpmNudge.SolGameOn ) would only work for the bumper objects
'   - Using thresholds instead also now has added benefit by not actually triggering the related _Hit or _Slingshot routines so animations, sound processing, and other potential nested subroutine calls will also not activate resulting in a better tilt simulation
'   Note: NudgePlugin option .vbs files were also updated as they contain and are reassigned the vpmNudge routines when invoked
'
' New in 3.47 (Update by Toxie)
' - (Core changes)
'   - Add UseVPMColoredDMD = true to the table script (place before LoadVPM, or otherwise calling core.vbs)
'     to automatically pass the raw colored DMD data (RGB from 0..255) from VPM to VP (see VP10+ for details on how to display it)
'
' New in 3.46 (Update by KieferSkunk)
' - (Core changes)
'   - Added two new classes: cvpmTrough and cvpmSaucer
'     - cvpmTrough takes over for cvpmBallStack in non-Saucer mode.
'       - Can handle any number of balls (no more "out of bounds" errors with lots of balls)
'       - Accurately simulates ball movement and switch interaction in a real trough
'     - cvpmSaucer takes over for cvpmBallStack in Saucer mode.
'     - cvpmBallStack is now considered "legacy" - kept for compatibility with existing tables.  (No changes)
'   - Updated vbsdoc.html with these new classes.
'   - Added two helper functions, vpMin(a, b) and vpMax(a, b).
'     - These each take two numbers (or strings) and return the lower or higher of the two (respectively).
'
' New in 3.45 (Update by KieferSkunk)
' - (Core changes)
'   - Rewrote cvpmDictionary as a wrapper around Microsoft's Scripting.Dictionary object.
'     This provides two major benefits:
'     (1) Improved performance: Keys are stored by hash/reference, not by index, and existence checks and key location are now O(1) instead of O(N) operations.
'     (2) Keys and Items can now both be primitive types or objects.  You can use integers, strings, etc. as keys, and you can use any object as an Item.
'         Note: The only restriction is that a Key cannot be a Scripting.Dictionary or an Array.
'   - cvpmTurnTable now smoothly changes speeds and directions.  You can adjust the following properties to change the turntable's behavior:
'     - MaxSpeed: Sets new maximum spin speed.  If motor is on, turntable will smoothly accelerate to new speed.
'     - SpinUp: Sets new spin-up rate.  If currently accelerating, turntable will accelerate at the new rate.
'     - SpinDown: Sets new spin-down rate.  If currently slowing to a stop, turntable will decelerate at the new rate.
'     - SpinCW: True for clockwise rotation, False for counter-clockwise.  If motor is on, switching this will smoothly reverse the turntable's direction.
'
' New in 3.44 (Update by Toxie)
' - (Core changes)
'	- Added ability to define default ball mass (in VP Units) inside table script.
'		Defaults to 1 unit if undefined. Example...
'			Const BallMass = 2 '(place before LoadVPM, or otherwise calling core.vbs)
'       Note that this should be used if changing the ball size via BallSize,
'       as the mass is of course proportional to the radius of the ball: m=k*r^3.
'       One can also use the diameter/size like in VP, so BallMass=k*BallSize^3 with k=1/125000.
'       Example: BallSize = 55, so BallMass = (55*55*55)/125000 = 1.331.
'   - Add UseVPMDMD = true to the table script (place before LoadVPM, or otherwise calling core.vbs)
'     to automatically pass the raw DMD data (levels from 0..100) from VPM to VP (see VP10+ for details on how to display it)
'   - Add toggleKeyCoinDoor in VPMKeys.vbs to choose between a real coindoor setup (e.g. cabinets) and the 'classic' on/off behaviour (e.g desktops/keyboards)
'   - Add inverseKeyCoinDoor in VPMKeys.vbs to in addition choose between the behaviour of a real coindoor switch (key pressed = closed, key not pressed = open)
'     or the inverted behaviour (key pressed = open, key not pressed = closed)
'   - Increase maximum number of balls/conMaxBalls to 13 and conStackSw to 8 (for Apollo 13), use InitSw8() then instead of InitSw()
'   - Deprecate vpmSolFlip2, as VP10 does not feature speed on flippers anymore
'
' New in 3.43 (Update by Koadic)
' - (Core Changes)
'	- Minor adjustment to vbs loading via LoadScript to account for files in nonstandard locations
'	- Fix minor bugs when loading some tables

' New in 3.42 (Update by Koadic)
' - (Core Changes)
'	- Minor adjustment to vpmInit to unpause controller before stopping controller
'
' New in 3.41 (Update by Koadic)
' - (Core Changes)
'	- Modified vpmInit routine:
'		Added creation of _Exit routine to vpmInit to perform Controller.Stop (will retroactively effect all tables using vpmInit call)
'		Modified vpmInit to create _Paused, _UnPaused, and _Exit separately, so if any don't exit, they will be created individually
'		Modified Error handling to fix bug where vmpInit might throw "Invalid procedure call or argument" error
'		 and cause table not to work due to improper Table_Init scripting.
'	- Added 2 functions: CheckScript(file) and LoadScript(file) that can return True/False as well as the latter loading the script if true.
'	   These check for existance in either the Tables and Scripts directory and can return a boolean value as well as the LoadScript autoloading
'		the file, as opposed to my previous methods only checking the local folder containing the table being run. 
'	   CheckScript(file) checks for existance, and if found returns a True value
'	   LoadScript(file) checks for existance, and if found, loads specified file (via ExecuteGlobal GetTextFile(file)) and returns a True value
'		Examples:
'			If LoadScript("thefile.vbs") Then DoThisOtherThing	' If Loadscript found 'thefile' and loaded it (returned true) then do this other thing
'			LoadScript("somefile.vbs")							' Checks for 'somefile' and loads it if it exists
'	- Reworked CheckLEDWiz routine into generic LoadScript(file) routine to allow for better detection of script in the VP tables
'	   or scripts directory, not just current directory containing the table.
'	- Added ability to load NudgePlugIn.vbs and if found, it will be loaded and replace current default nudging class.
'		- This detection and autoloading can allow for 'on demand' replacement of other core components as well in the future.
'	- Added ability to load GlobalPlugIn.vbs containing any custom scripting the user wants loaded with the core.vbs (instead of modifying the core)
' -(Other Additions)
'	- Updated B2BCollision.vbs with vpmBallCreate method and renamed new file to B2B.vbs (to maintain compatiblity with tables using old file).
'
' New in 3.40 (Update by Koadic)
' - (Core Changes)
'	- Modified NVOffset routine to allow use of alternative controllers (like dB2S B2S.Server)
' New in 3.39 (Update by Koadic)
' - (Core Changes)
'	- Hopefully fixed bug introduced in 3.37 when using a VP version older than 9.0.10
' New in 3.38 (Update by Koadic)
' - (Core Changes)
'	- Added automatic detection of ledcontrol.vbs and enabling for LedWiz use, allowing concurrent use by both users and non users of an LedWiz
' New in 3.37 (Update by Koadic)
' - (Core Changes)
'	- Added ability to define default ballsize (in VP Units) inside table script.
'		Defaults to 50 vp units if undefined. Example...
'			Const BallSize = 47 '(place before LoadVPM, or otherwise calling core.vbs)	
' New in 3.36 (update courtesy of Koadic)
' - (Core Changes)
'   - Added VPMVol routine for allowing setting of Global VPM Volume (normally adjustable from '~' key, but otherwise unsaveable without this)
' - (System VBS Alterations)
'   - Added keyVPMVolume in VPMKeys.vbs, set to use the F12 key
'   - Added call to VPMVol routine in each system's .vbs file, allowing end-user to access the new routine
' New in 3.35 (Update courtesy of Koadic)
' - (Core Changes)
'   - Added NVOffset routine for allowing use of multiple nvram files per romset name
' New in 3.34 (Update by Destruk)
' - (System VBS Additions)
'   - Added Play2.vbs
' New in 3.33 (Update by Destruk)
' - (System VBS Additions)
'   - Added LTD.vbs
' New in 3.32 (Update by Destruk)
' - (System VBS Alterations)
'   - Added Playmatic Replay setting switches
' New in 3.31 (Update by Destruk)
' - (System VBS Additions)
'   - Added play1.vbs
' New in 3.30 (Update by Destruk)
' - (System VBS Additions)
'   - Added zacproto.vbs
' New in 3.29 (Update by Noah)
' - (System VBS Additions)
'   - Added jvh.vbs and ali.vbs by Destruk for Jac van Ham and Allied Leisure
' Corrected VPBuild Number for slingshots/bumpers and ball decals - Seeker
' New in 3.27 (Update by PD)
' - (System VBS Additions)
'   - Added gts1.vbs by Inkochnito for Gottlieb System 1
' New in 3.26 (Update by PD)
' - (Core Changes)
'    - Added "GICallback2" function to support Steve Ellenoff's new support in VPM for Dimming GI in WMS games
'      GICallback returns numeric values 0-8 instead of a boolean 0 or 1 (on/off) like GICallback does.
'      Existing tables will need to be altered to support dimming levels and need to use GICallback2 instead.
'      The old GICallback is left intact so older tables are not broken by the new code
'
' New in 3.25 (release 2) (Update by PD)
' - (Core Changes)
'    - Restored former flipper speed due to complaints about some tables having BTTF problem returned and a resolution
'      of arguments over the settings
'    - New Optional Flipper Code Added (vpmSolFlip2) that let's you specify both up and down-swing speeds in the script
'      plus the ability to turn flipper sounds on or off for that call
'      Format: vpmSolFlip2 (Flip1obj, Flip2obj, UpSpeed, DownSpeed, SoundOn, Enable) 
'
' New in 3.24 (Update by PD)
' - (Core Changes)
'    - Altered flipper code so the upswing defaults to your downswing (i.e. VBS no longer adds a different value)
'      (This change was done due to arguments over issues now resolved)
'    - I have decreased the return strength setting to be very low, though.  So any downswing hits (say from a ball
'      heading to the trough) won't get hit with any real power.  So, assuming you have a reasonably fast upswing,
'      you won't get any balls through the flipper and any balls hit by the underside won't get pegged anymore, which
'      is a more realistic behavior.
'
' New in 3.23 (Update by PD)
' - (System.vbs Additions)
'    - SlamtTilt definitions added to AlvinG and Capcom systems
'    - High Score Reset Switch Added to Williams System7 (S7.vbs)
'    - Sleic.vbs system added (courtesy of Destruk)
'    - Peper.vbs system added (courtesy of Destruk)
'    - Juegos.vbs system added (courtesy of Destruk)
'
' New in 3.22 (Update by PD)
' - (Core Changes)
'   - Outhole switch handling updated so it resets correctly with an F3 reset.  
'     This affects mostly Gottlieb System3 games (Thanks Racerxme for pointing this out)
'   - Flipper handling modified to have a low return strength setting so any balls under such flippers
'     won't get hit hard.  This allows the higher 'flipper fix' return speed without the associated hard hit issue.
' - (System.vbs Additions)
'   -Inder.vbs test switches updated (Thanks Peter)
'   -Bally.vbs swSoundDiag value changed to -6 (Thanks Racerxme)
'
' New in 3.21 (Update by PD)
' -(Core Changes)
'   - Attemped bug fix in the Impulse Plunger object that could cause weak plunges sometimes on full pulls
'   
' -(System.vbs Additions)
'   -Zac1.vbs has the program enable switch added to it (Thanks TomB)
'   -GamePlan.vbs has the accounting reset switch added to it (Thanks Incochnito)
'
' -(Other Additions)
'   -PD Light System VBS file updated to V5.5 (adds fading reel pop bumper handler and checklight function)
'
' New in 3.20 (Update by PD)
' -(System.vbs Additions)
'   -Apparently Atari2.vbs uses 81/83 for the flipper switches and Atar1.vbs uses 82/84 so this repairs
'    the Atari2.vbs file.
'
' New in 3.19 (Update by PD)
' -(System.vbs Additions)
'   - Fixed the swLLFlip and swLRFlip switch numbers in the Atari1.vbs, Atari2.vbs and Atari.vbs files
'     SolFlipper should now work with Atari tables using the updated file
'
' New in 3.18 (Update by PD)
' -(System.vbs Additions)
'   - Added Atari1.vbs and Atari2.vbs files (Thanks to Inkochnito).  
'     -The old Atari.vbs file is now obsolete, but included for backwards compatability with any existing tables
'      that may have used it. New Tables should use the appropriate Atari1.vbs or Atari2.vbs files.
'
' New in 3.17 (Update by PD)
' -(System.vbs Additions)
'   -Fixed wrong switch definition in Sys80.vbs for the self-test switch.  The operator menus should work now.
'    (Thanks to Inkochnito for pointing it out).
'   -Added inder.vbs, nuova.vbs, spinball.vbs and mrgame.vbs files (Thanks to Destruk)
'
' New in 3.16 (Update by PD)
' -(System.vbs Additions)
'   -Added "BeginModal" and "EndModal" statements to each system (required for latest versions of VP ( >V6.1) to
'    avoid problems during the VPM "F3" reset.
' -(Other Additions)
'   - PDLightSystem Core updated to version 5.4
'
' New in 3.15 (Update by PD)
' -(Core Additions)
'   - Added a new higher resolution Impulse Plunger Object 
'      (It uses a trigger to plunge the ball.  It can be a variable Manual Plunger or function as an Automatic Plunger)
'      (It also features random variance options and optional pull / plunge sounds)
'
' -(System.vbs Additions)
'   - Fixed wrong switch number for Tilt & Slam Tilt in Sega.vbs
'   - Added Master CPU Enter switch to S7.vbs for Dip Switch control in Williams System7
'
' -(Other Additions)
'   - Added PDLightSystem.vbs (V5.3) file to archive 
'     (open it with a text editor to see how to use it; it's called separately like the core file)
'
' New in 3.14 (Update by PD)
' -(System.vbs Additions)
'   - Added latest Zac1.vbs and Zac2.vbs files to archive
'
' New in 3.13 (Update by PD)
' -(Core Additions)
'   - Added Destruk's code to "Add" or "Remove" a ball from the table when "B" is pressed.
'   - Added "AutoplungeS" call which is the same as "Autoplunger" except it will play a specified sound when fired
'
' -(System.vbs Additions)
'   - Taito.vbs updated to fix service menu keys and default dip switch menu added
'   - Dip Switch / Option Menu "class" code added to all table VBS scripts to ease menu coding for table authors
'   - Fixed some labeling errors and organization and added a "Last Updated" version comment at the start of each file
'
' New in 3.12
'   - Made flipper return speed a constant conFlipRetSpeed
'   - set conFlipRetSpeed to 0.137 to reduce ball thru flipper problem
'
' New in 3.11
'   - Added a short delay between balls in the ballstacks to ensure
'     that the game registers the switches as off when balls are rolling
'     in the trough. All balls should probably move at the same time but it is
'     a bit tricky to implement without changing a lot of code.
'   - Removed support for the wshltdlg.dll since funtionality is in VPM now
' New in 3.10
'   - Public release
' Put this at the top of the table file
'LoadVPM "02000000", "xxx.VBS", 3.15
'Const cGameName    = "xxxx" ' PinMAME short game name
'Const UseSolenoids = True
'Const UseLamps     = True
''Standard sound
'Const SSolenoidOn  = "SolOn"       'Solenoid activates
'Const SSolenoidOff = "SolOff"      'Solenoid deactivates
'Const SFlipperOn   = "FlipperUp"   'Flipper activated
'Const SFlipperOff  = "FlipperDown" 'Flipper deactivated
'Const SCoin        = "Quarter"     'Coin inserted
''Callbacks
'Set LampCallback   = GetRef("UpdateMultipleLamps")
'Set GICallback     = GetRef("UpdateGI")  ' Original GI Callback (returns boolean on and off values only)
'Set GICallback2    = GetRef("UpdateGI")  ' New GI Callback supports Newer VPM Dimming GI and returns values numeric 0-8)
'Set MotorCallback  = GetRef("UpdateMotors")
'
'Sub LoadVPM(VPMver, VBSfile, VBSver)
'	On Error Resume Next
'		If ScriptEngineMajorVersion < 5 Then MsgBox "VB Script Engine 5.0 or higher required"
'		ExecuteGlobal GetTextFile(VBSfile)
'		If Err Then MsgBox "Unable to open " & VBSfile & ". Ensure that it is in the same folder as this table. " & vbNewLine & Err.Description : Err.Clear
'		Set Controller = CreateObject("VPinMAME.Controller")
'		If Err Then MsgBox "Can't Load VPinMAME." & vbNewLine & Err.Description
'		If VPMver>"" Then If Controller.Version < VPMver Or Err Then MsgBox "VPinMAME ver " & VPMver & " required." : Err.Clear
'		If VPinMAMEDriverVer < VBSver Or Err Then MsgBox VBSFile & " ver " & VBSver & " or higher required."
'End Sub
'
'Sub Table_KeyDown(ByVal keycode)
'	If vpmKeyDown(keycode) Then Exit Sub
'	If keycode = PlungerKey Then Plunger.Pullback
'End Sub
'Sub Table_KeyUp(ByVal keycode)
'	If vpmKeyUp(keycode) Then Exit Sub
'	If keycode = PlungerKey Then Plunger.Fire
'End Sub
'
'Const cCredits  = ""
'Sub Table_Init
'	vpmInit Me
'	On Error Resume Next
'		With Controller
'			.GameName = cGameName
'			If Err Then MsgBox "Can't start Game " & cGameName & vbNewLine & Err.Description : Exit Sub
'			.SplashInfoLine = cCredits
'			.HandleMechanics = 0
'			.ShowDMDOnly = True : .ShowFrame = False : .ShowTitle = False
'			.Run : If Err Then MsgBox Err.Description
'		End With
'	On Error Goto 0
'' Nudging
'	vpmNudge.TiltSwitch = swTilt
'	vpmNudge.Sensitivity = 5
'	vpmNudge.TiltObj = Array(Bumper1,Bumper2,LeftslingShot,RightslingShot)
'' Map switches and lamps
'	vpmCreateEvents colSwObjects ' collection of triggers etc
'	vpmMapLights    colLamps     ' collection of all lamps
'' Trough handler
'	Set bsTrough = New cvpmBallStack
'	bsTrough.InitNoTrough BallRelease, swOuthole, 90, 2
'	'or
'	bsTrough.InitSw swOuthole,swTrough1,swTrough2,0,0,0,0
'---------------------------------------------------------------
Dim Controller   ' VPinMAME Controller Object
Dim vpmTimer     ' Timer Object
Dim vpmNudge     ' Nudge handler Object
Dim Lights(200)  ' Put all lamps in an array for easier handling
' If more than one lamp is connected, fill this with an array of each light
Dim vpmMultiLights() : ReDim vpmMultiLights(0)
Private gNextMechNo : gNextMechNo = 0 ' keep track of created mech handlers (would be nice with static members)

' Callbacks
Dim SolCallback(68) ' Solenoids (parsed at Runtime)
Dim SolModCallback(68) ' Solenoid modulated callbacks (parsed at Runtime) 
Dim SolPrevState(68) ' When modulating solenoids are in use, needed to keep positive value levels from changing boolean state
Dim LampCallback    ' Called after lamps are updated
Dim GICallback      ' Called for each changed GI String
Dim GICallback2     ' Called for each changed GI String
Dim MotorCallback   ' Called after solenoids are updated
Dim vpmCreateBall   ' Called whenever a vpm class needs to create a ball
Dim BSize:If IsEmpty(Eval("BallSize"))=true Then BSize=25 Else BSize = BallSize/2
Dim BMass:If IsEmpty(Eval("BallMass"))=true Then BMass=1 Else BMass = BallMass
Dim UseDMD:If IsEmpty(Eval("UseVPMDMD"))=true Then UseDMD=false Else UseDMD = UseVPMDMD
Dim UseModSol:If IsEmpty(Eval("UseVPMModSol"))=true Then UseModSol=false Else UseModSol = UseVPMModSol
Dim UseColoredDMD:If IsEmpty(Eval("UseVPMColoredDMD"))=true Then UseColoredDMD=false Else UseColoredDMD = UseVPMColoredDMD
Dim UseNVRAM:If IsEmpty(Eval("UseVPMNVRAM"))=true Then UseNVRAM=false Else UseNVRAM = UseVPMNVRAM
Dim NVRAMCallback

' Assign Null Default Sub so script won't error if only one is defined in a script (should redefine in your script)
Set GICallback = GetRef("NullSub")
Set GICallback2 = GetRef("NullSub")

' Game specific info
Dim ExtraKeyHelp    ' Help string for game specific keys
Dim vpmShowDips     ' Show DIPs function
'-----------------------------------------------------------------------------
' These helper functions require the following objects on the table:
'   PinMAMETimer   : Timer object
'   PulseTimer     : Timer object
'
' Available classes:
' ------------------
' cvpmTimer (Object = vpmTimer)
'   (Public)  .PulseSwitch   - pulse switch and call callback after delay (default)
'   (Public)  .PulseSw       - pulse switch
'   (Public)  .AddTimer      - call callback after delay
'   (Public)  .Reset         - Re-set all ballStacks
'   (Friend)  .InitTimer     - initialise fast or slow timer
'   (Friend)  .EnableUpdate  - Add/remove automatic update for an instance
'   (Private) .Update        - called from slow timer
'   (Private) .FastUpdate    - called from fast timer
'   (Friend)  .AddResetObj   - Add object that needs to catch reset
'
' cvpmTrough (Create as many as needed)
'   (Public) .IsTrough         - Get or Set whether this trough is the default trough (first trough sets this by default)
'   (Public) .Size             - Get or Set total number of balls trough can hold
'   (Public) .EntrySw          - Set switch number for trough entry (if any) - eg. Outhole
'   (Public) .AddSw            - Assign a switch at a specific slot
'   (Public) .InitSwitches     - Set trough switches using an array, from exit slot back toward entrance.
'   (Public) .InitExit         - Setup exit kicker, force and direction
'   (Public) .InitExitVariance - Modify exit kick direction and force (+/-, min force = 1)
'   (Public) .InitEntrySounds  - Sounds to play when a ball enters the trough
'   (Public) .InitExitSounds   - Sounds to play when the exit kicker fires
'   (Public) .CreateEvents     - Auto-generate hit events for VP entry kicker(s) associated with this trough
'   (Public) .MaxBallsPerKick  - Set maximum number of balls to kick out (default 1)
'   (Public) .MaxSlotsPerKick  - Set maximum slots from which to get balls when kicking out (default 1)
'   (Public) .Balls            - Get current balls in trough, or set initial number of balls in trough
'   (Public) .BallsPending     - Get number of balls waiting in trough entry
'   (Public) .Reset            - Reset and update all trough switches
'   (Friend) .Update           - Called from vpmTimer to update ball positions and switches
'   (Public) .AddBall          - Add a ball to the trough from a kicker.  If kicker is the exit kicker, stacks ball at exit.
'   (Public) .SolIn            - Solenoid handler for entry solenoid
'   (Public) .SolOut           - Solenoid handler for exit solenoid
'
' cvpmSaucer (Create as many as needed)
'   (Public) .InitKicker       - Setup main kicker, switch, exit direction and force (including Z force)
'   (Public) .InitExitVariance - Modify kick direction and force (+/-, min force = 1)
'   (Public) .InitAltKick      - Set alternate direction and force (including Z force) - for saucers with two kickers
'   (Public) .InitSounds       - Sounds to play when a ball enters the saucer or the kicker fires
'   (Public) .CreateEvents     - Auto-generate hit event for VP kicker(s) associated with this saucer
'   (Public) .AddBall          - Add a ball to the saucer from a kicker.
'   (Public) .HasBall          - True if the saucer is occupied.
'   (Public) .solOut           - Fire the primary exit kicker.  Ejects ball if one is present.
'   (Public) .solOutAlt        - Fire the secondary exit kicker.  Ejects ball with alternate forces if present.
'
' cvpmBallStack (DEPRECATED, but create as many as needed)
'   (Public) .InitSw        - init switches used in stack
'   (Public) .InitSaucer    - init saucer
'   (Public) .InitNoTrough  - init a single ball, no trough handler
'   (Public) .InitKick      - init exit kicker
'   (Public) .InitAltKick   - init second kickout direction
'   (Public) .CreateEvents  - Create addball events for kickers
'   (Public) .KickZ         - Z axis kickout angle (radians)
'   (Public) .KickBalls     - Maximum number of balls kicked out at the same time
'   (Public) .KickForceVar  - Initial ExitKicker Force value varies by this much (+/-, minimum force = 1)
'   (Public) .KickAngleVar  - ExitKicker Angle value varies by this much (+/-)
'   (Public) .BallColour    - Set ball colour
'   (Public) .TempBallImage  - Set ball image for next ball only
'   (Public) .TempBallColour - Set ball colour for next ball only
'   (Public) .BallImage     - Set ball image
'   (Public) .InitAddSnd    - Sounds when ball enters stack
'   (Public) .InitEntrySnd  - Sounds for Entry kicker
'   (Public) .InitExitSnd   - Sounds for Exit kicker
'   (Public) .AddBall       - add ball in "kicker" to stack
'   (Public) .SolIn         - Solenoid handler for entry solenoid
'   (Public) .EntrySol_On   - entry solenoid fired
'   (Public) .SolOut        - Solenoid handler for exit solenoid
'   (Public) .SolOutAlt     - Solenoid handler for exit solenoid 2nd direction
'   (Public) .ExitSol_On    - exit solenoid fired
'   (Public) .ExitAltSol_On - 2nd exit solenoid fired
'   (Public) .Balls         - get/set number of balls in stack (default)
'   (Public) .BallsPending  - get number of balls waiting to come in to stack
'   (Public) .IsTrough      - Specify that this is the main ball trough
'   (Public) .Reset         - reset and update all ballstack switches
'   (Friend) .Update        - Update ball positions (from vpmTimer class)
'  Obsolete
'   (Public) .SolExit       - exit solenoid handler
'   (Public) .SolEntry      - Entry solenoid handler
'   (Public) .InitProxy     - Init proxy switch

' cvpmNudge (Object = vpmNudge)
'   Hopefully we can add a real pendulum simulator in the future
'   (Public)  .TiltSwitch   - set tilt switch
'   (Public)  .Senitivity   - Set tiltsensitivity (0-10)
'   (Public)  .TiltObj      - Set objects affected by tilt
'   (Public)  .DoNudge dir,power  - Nudge table
'   (Public)  .SolGameOn    - Game On solenoid handler
'   (Private) .Update       - Handle tilting
'
' cvpmDropTarget (create as many as needed)
'   (Public)  .InitDrop     - initialise DropTarget bank
'   (Public)  .CreateEvents - Create Hit events
'   (Public)  .InitSnd      - sound to use for targets
'   (Public)  .AnyUpSw      - Set AnyUp switch
'   (Public)  .AllDownSw    - Set all down switch
'   (Public)  .AllDown      - All targets down?
'   (Public)  .Hit          - A target had been hit
'   (Public)  .SolHit       - Solenoid handler for dropping a target
'   (Public)  .SolUnHit     - Solenoid handler for raising a target
'   (Public)  .SolDropDown  - Solenoid handler for Bank down
'   (Public)  .SolDropUp    - Solenoid handler for Bank reset
'   (Public)  .DropSol_On   - Reset target bank
'   (Friend)  .SetAllDn     - check alldown & anyup switches
'
' cvpmMagnet (create as many as needed)
'   (Public)  .InitMagnet   - initialise magnet
'   (Public)  .CreateEvents - Create Hit/Unhit events
'   (Public)  .Solenoid     - Set solenoid that controls magnet
'   (Public)  .GrabCenter   - Magnet grabs ball at center
'   (Public)  .MagnetOn     - Turn magnet on and off
'   (Public)  .X            - Move magnet
'   (Public)  .Y            - Move magnet
'   (Public)  .Strength     - Change strength
'   (Public)  .Size         - Change magnet reach
'   (Public)  .AddBall      - A ball has come within range
'   (Public)  .RemoveBall   - A ball is out of reach for the magnet
'   (Public)  .Balls        - Balls currently within magnets reach
'   (Public)  .AttractBall  - attract ball to magnet
'   (Private) .Update       - update all balls (called from timer)
'   (Private) .Reset        - handle emulation reset
'  Obsolete
'   (Public)  .Range        - Change magnet reach

' cvpmTurnTable (create as many as needed)
'   (Public)  .InitTurnTable - initialise turntable
'   (Public)  .CreateEvents  - Create Hit/Unhit events
'   (Public)  .MaxSpeed      - Maximum speed
'   (Public)  .SpinUp        - Speedup acceleration
'   (Public)  .SpinDown      - Retardation
'   (Public)  .Speed         - Current speed
'   (Public)  .MotorOn       - Motor On/Off
'   (Public)  .SpinCW        - Control direction
'   (Public)  .SolMotorState - Motor on/off solenoid handler
'   (Public)  .AddBall       - A ball has come withing range
'   (Public)  .RemoveBall    - A ball is out of reach for the magnet
'   (Public)  .Balls         - Balls currently within magnets reach
'   (Public)  .AffectBall    - affect a ball
'   (Private) .Update        - update all balls (called from timer)
'   (Private) .Reset         - handle emulation reset

' cvpmMech (create as many as needed)
'   (Public)  .Sol1, Sol2    - Controlling solenoids
'   (Public)  .MType         - type of mechanics
'   (Public)  .Length, Steps
'   (Public)  .Acc, Ret      - Acceleration, retardation
'   (Public)  .AddSw         - Automatically controlled switches
'   (Public)  .AddPulseSw    - Automatically pulsed switches
'   (Public)  .Callback      - Update graphics function
'   (Public)  .Start         - Start mechanics handler
'   (Public)  .Position      - Current position
'   (Public)  .Speed         - Current Speed
'   (Private) .Update
'   (Private) .Reset
'
' cvpmCaptiveBall (create as many as needed)
'   (Public)  .InitCaptive   - Initialise captive balls
'   (Public)  .CreateEvents  - Create events for captive ball
'   (Public)  .ForceTrans    - Amount of force tranferred to captive ball (0-1)
'   (Public)  .MinForce      - Minimum force applied to the ball
'   (Public)  .NailedBalls   - Number of "nailed" balls infront of captive ball
'   (Public)  .RestSwitch    - Switch activated when ball is in rest position
'   (Public)  .Start         - Create moving ball etc.
'   (Public)  .TrigHit       - trigger in front of ball hit (or unhit)
'   (Public)  .BallHit       - Wall in front of ball hit
'   (Public)  .BallReturn    - Captive ball has returned to kicker
'   (Private) .Reset
'
' cvpmVLock (create as many as needed)
'   (Public)  .InitVLock     - Initialise the visible ball stack
'   (Public)  .ExitDir       - Balls exit angle (like kickers)
'   (Public)  .ExitForce     - Force of balls kicked out
'   (Public)  .KickForceVar  - Vary kickout force
'   (Public)  .InitSnd       - Sounds to make on kickout
'   (Public)  .Balls         - Number of balls in Lock
'   (Public)  .SolExit       - Solenoid event
'   (Public)  .CreateEvents  - Create events needed
'   (Public)  .TrigHit       - called from trigger hit event
'   (Public)  .TrigUnhit     - called from trigger unhit event
'   (Public)  .KickHit       - called from kicier hit event
'
' cvpmDips (create as many as needed) => (Dip Switch And/Or Table Options Menu)
'   (Public)  .AddForm       - create a form (AKA dialogue)
'   (Public)  .AddChk        - add a chckbox
'   (Public)  .AddChkExtra   -   -  "" -     for non-dip settings
'   (Public)  .AddFrame      - add a frame with checkboxes or option buttons
'   (Public)  .AddFrameExtra -  - "" - for non-dip settings
'   (Public)  .AddLabel      - add a label (text string)
'   (Public)  .ViewDips      - Show form
'   (Public)  .ViewDipsExtra -  - "" -  with non-dip settings
'
' cvpmImpulseP (create as many as needed) => (Impulse Plunger Object using a Trigger to Plunge Manual/Auto)
'   (Public)  .InitImpulseP - Initialise Impulse Plunger Object (Trigger, Plunger Power, Time to Full Plunge [0 = Auto])
'   (Public)  .CreateEvents - Create Hit/Unhit events
'   (Public)  .Strength     - Change plunger strength
'   (Public)  .Time         - Change plunger time (in seconds) to full plunger strength (0 = Auto Plunger)
'   (Public)  .Pullback     - Pull the plunger back
'   (Public)  .Fire	    - Fires / Releases the Plunger (Manual or Auto depending on Timing Value given)
'   (Public)  .AutoFire	    - Fires / Releases the Plunger at Maximum Strength +/- Random variation (i.e. Instant Auto)
'   (Public)  .Switch	    - Switch Number to activate when ball is sitting on plunger trigger (if any)
'   (Public)  .Random       - Sets the multiplier level of random variance to add (0 = No Variance / Default) 
'   (Public)  .InitEntrySnd - Plays Sound as Plunger is Pulled Back
'   (Public)  .InitExitSnd  - Plays Sound as Plunger is Fired (WithBall,WithoutBall)
'
' Generic solenoid handlers:
' --------------------------
' vpmSolFlipper flipObj1, flipObj2  		- "flips flippers". Set unused to Nothing
' vpmSolFlip2   flipObj1, flipObj2, flipSpeedUp, flipSpeedDn, sndOn).  Set unused to Nothing
' vpmSolDiverter divObj, sound      		- open/close diverter (flipper) with/without sound
' vpmSolWall wallObj, sound            		- Raise/Drop wall with/without sound
' vpmSolToggleWall wall1, wall2, sound		- Toggle between two walls
' vpmSolToggleObj obj1,obj2,sound   		- Toggle any objects
' vpmSolAutoPlunger plungerObj, var, enabled  	- Autoplunger/kickback
' vpmSolAutoPlungeS plungerObj, sound, var, enabled - Autoplunger/kickback With Specified Sound To Play
' vpmSolGate obj, sound             		- Open/close gate
' vpmSolSound sound                 		- Play sound only
' vpmFlasher flashObj               		- Flashes flasher
'
' Generating events:
' ------------------
' vpmCreateEvents
' cpmCreateLights
'
' Variables declared (to be filled in):
' ---------------------------------------
' SolCallback()  - handler for each solenoid
' Lights()       - Lamps
'
' Constants used (must be defined):
' ---------------------------------
' UseSolenoids   - Update solenoids
' MotorCallback  - Called once every update for mechanics or custom sol handler
' UseLamps       - Update lamps
' LampCallback   - Sub to call after lamps are updated
'                  (or every update if UseLamps is false)
' GICallback     - Sub to call to update GI strings
' GICallback2    - Sub to call to update GI strings
' SFlipperOn     - Flipper activate sound
' SFlipperOff    - Flipper deactivate sound
' SSolenoidOn    - Solenoid activate sound
' SSolenoidOff   - Solenoid deactivate sound
' SCoin          - Coin Sound
' ExtraKeyHelp   - Game specific keys in help window
'
' Exported variables:
' -------------------
' vpmTimer      - Timer class for PulseSwitch etc
' vpmNudge      - Class for table nudge handling
'-----------------------------------------------------
Private Function PinMAMEInterval
        If VPBuildVersion >= 10200 Then
                PinMAMEInterval = -1 ' VP10.2 introduced special frame-sync'ed timers
        Else
            If VPBuildVersion >= 10000 Then
                PinMAMEInterval = 3  ' as old VP9 timers pretended to run at 1000Hz but actually did only a max of 100Hz (e.g. corresponding nowadays to interval=10), we do something inbetween for VP10+ by default
            Else
                PinMAMEInterval = 1
            End If
        End If
End Function

Private Const conStackSw    = 8  ' Stack switches
Private Const conMaxBalls   = 13 ' Because of Apollo 13
Private Const conMaxTimers  = 20 ' Spinners can generate a lot of timers
Private Const conTimerPulse = 40 ' Timer runs at 25Hz
Private Const conFastTicks  = 4  ' Fast is 4 times per timer pulse
Private Const conMaxSwHit   = 5  ' Don't stack up more than 5 events for each switch

' DEPRECATED Flipper constants:
Private Const conFlipRetStrength = 0.01  ' Flipper return strength
Private Const conFlipRetSpeed    = 0.137 ' Flipper return speed

Function CheckScript(file) 'Checks Tables and Scripts directories for specified vbs file, and if it exitst, will load it.
	CheckScript = False
  On Error Resume Next
	Dim TablesDirectory:TablesDirectory = Left(UserDirectory,InStrRev(UserDirectory,"\",InStrRev(UserDirectory,"\")-1))&"Tables\"
	Dim ScriptsDirectory:ScriptsDirectory = Left(UserDirectory,InStrRev(UserDirectory,"\",InStrRev(UserDirectory,"\")-1))&"Scripts\"
	dim check:Set check = CreateObject("Scripting.FileSystemObject")
	If check.FileExists(tablesdirectory & file) Or check.FileExists(scriptsdirectory & file) Or check.FileExists(file) Then CheckScript = True
  On Error Goto 0
End Function

Function LoadScript(file) 'Checks Tables and Scripts directories for specified vbs file, and if it exitst, will load it.
	LoadScript = False
  On Error Resume Next
	If CheckScript(file) Then ExecuteGlobal GetTextFile(file):LoadScript = True
  On Error Goto 0
End Function

' Dictionary
' At one point, Microsoft had made Scripting.Dictionary "unsafe for scripting", but it's
' been a long time since that was true.  So now, to maintain compatibility with all tables
' and scripts that use cvpmDictionary, this class is now a simple wrapper around Microsoft's
' more efficient implementation.
Class cvpmDictionary
    Private mDict
	Private Sub Class_Initialize : Set mDict = CreateObject("Scripting.Dictionary") : End Sub

    ' DEPRECATED: MS Dictionaries are not index-based.  Use "Exists" method instead.
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
	    ' This function is (and always has been) a no-op.  Previous definition
	    ' just looked up aKey in the keys list, and if found, set the key to itself.
	End Property

	Public Sub Add(aKey, aItem)
	    If IsObject(aItem) Then
	        Set mDict(aKey) = aItem
	    Else
	        mDict(aKey) = aItem
	    End If
	End Sub

	Public Sub Remove(aKey)      : mDict.Remove(aKey)          : End Sub
	Public Sub      RemoveAll    : mDict.RemoveAll             : End Sub
	Public Function Exists(aKey) : Exists = mDict.Exists(aKey) : End Function
	Public Function Items        : Items  = mDict.Items        : End Function
	Public Function Keys         : Keys   = mDict.Keys         : End Function
End Class

'--------------------
'       Timer
'--------------------
Class cvpmTimer
	Private mQue, mNow, mTimers
	Private mSlowUpdates, mFastUpdates, mResets, mFastTimer

	Private Sub Class_Initialize
		ReDim mQue(conMaxTimers) : mNow = 0 : mTimers = 0
		Set mSlowUpdates = New cvpmDictionary
		Set mFastUpdates = New cvpmDictionary
		Set mResets      = New cvpmDictionary
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
			If mQue(ii)(0) <= mNow Then
				If mQue(ii)(1) = 0 Then
					If isObject(mQue(ii)(3)) Then
						Call mQue(ii)(3)(mQue(ii)(2))
					ElseIf varType(mQue(ii)(3)) = vbString Then
						If mQue(ii)(3) > "" Then Execute mQue(ii)(3) & " " & mQue(ii)(2) & " "
					End If
					mTimers = mTimers - 1
					For jj = ii To mTimers : mQue(jj) = mQue(jj+1) : Next : ii = ii - 1
				ElseIf mQue(ii)(1) = 1 Then
					mQuecopy = mQue(ii)(2)
					Controller.Switch(mQuecopy) = False
					mQue(ii)(0) = mNow + mQue(ii)(4) : mQue(ii)(1) = 0
				Else '2
					mQuecopy = mQue(ii)(2)
					Controller.Switch(mQuecopy) = True
					mQue(ii)(1) = 1
				End If
			End If
			ii = ii + 1
		Loop
	End Sub

	Public Sub AddResetObj(aObj)  : mResets.Add aObj, 0 : End Sub

	Public Sub PulseSw(aSwNo) : PulseSwitch aSwNo, 0, 0 : End Sub

	Public Default Sub PulseSwitch(aSwNo, aDelay, aCallback)
		Dim ii, count, last
		count = 0
		For ii = 1 To mTimers
			If mQue(ii)(1) > 0 And mQue(ii)(2) = aSwNo Then count = count + 1 : last = ii
		Next
		If count >= conMaxSwHit Or mTimers = conMaxTimers Then Exit Sub
		mTimers = mTimers + 1 : mQue(mTimers) = Array(mNow, 2, aSwNo, aCallback, aDelay\conTimerPulse)
		If count Then mQue(mTimers)(0) = mQue(last)(0) + mQue(last)(1)
	End Sub

	Public Sub AddTimer(aDelay, aCallback)
		If mTimers = conMaxTimers Then Exit Sub
		mTimers = mTimers + 1
		mQue(mTimers) = Array(mNow + aDelay \ conTimerPulse, 0, 0, aCallback)
	End Sub
	
	Public Sub AddTimer2(aDelay, aCallback, aID)
		If mTimers = conMaxTimers Then Exit Sub
		mTimers = mTimers + 1
		mQue(mTimers) = Array(mNow + aDelay \ conTimerPulse, 0, aID, aCallback)
	End Sub
End Class

'--------------------
'     Trough
'--------------------
Class cvpmTrough
    ' Takes over for older cvpmBallStack in "trough mode".  Theory of operation:
    ' A trough can hold up to N balls, and has N*2 "slots".  A ball effectively takes
    ' up two slots, so no two adjacent slots (0 and 1) can be occupied at the same time.
    ' Switches are assigned to even slots only, which means that as balls move through
    ' the trough, each switch is allowed to flip between open and closed.
    ' Slot 0 is the exit, and can have additional balls "stacked" on it, simulating balls
    ' falling onto the exit kicker instead of coming in from the entrance.  Extra balls
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
        For ii = 0 to UBound(mSlot) : mSlot(ii) = 0 : Next   ' All slots empty to start
        For ii = 0 to UBound(mSw)   : mSw(ii) = 0   : Next   ' All switches unassigned to start.
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

    ' MaxBallsPerKick: Kick up to N balls total per exit kick.  Balls are only kicked from Slot 0.
    ' StackExitBalls: Automatically stack up to N balls in Slot 0 regardless of where they came from.

    ' Example: Subway where exit kicker is on the same level as the trough and a ball can
    ' come in from the exit: StackExitBalls = 1, MaxBallsPerKick = 2.  If Slot 0 has 1
    ' ball and Slot 1 is occupied, only one ball will be kicked.  If Slot 0 has 2 or more
    ' balls, it'll kick out 2 balls.

    ' Example: Twilight Zone Slot Kicker: Kicker is below trough, so if a ball is in the
    ' exit chute, another ball can fall into the chute as well whether it came in from the
    ' exit (Slot Machine) or any other entrance (Piano, Camera).  In both cases, the kicker
    ' will eject 2 balls at once.  Set StackExitBalls = 2, maxBallsPerKick = 2 to simulate.

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
        ' First, stack exit slot.  (Note, we may get a negative number. vpMin/vpMax prevent that.)
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

	Private Sub UpdateDebugBox   ' Requires a textbox named DebugBox
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
            If mSlot(ii) Then               ' Ball in this slot.
				canMove = False

                ' Can this ball move?  (Slot 0 = no)
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
                    AdvanceBalls = True       ' Mark balls as having moved.
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
        ' in the entry queue.  In a kicker-gated trough, the entry switch will remain
        ' pressed down, usually resulting in the machine retrying the load.  In a fall-in
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
                ' Ball fell in from exit.  Stack it up on Slot 0.
                mSlot(0) = mSlot(0) + 1
                NeedUpdate = True
                UpdateTroughSwitches
                addDone = True
            End If
        End If

        If Not addDone Then
            ' Ball came in from entrance.  Queue it up for entry.
            mBallsInEntry = mBallsInEntry + 1
		If mEntrySw > 0 Then
			mSwcopy = mEntrySw
			' Trough has an entry kicker.  Ball will not enter trough
			' until the entry solenoid is fired.
			Controller.Switch(mSwcopy) = True
	    	End If
            NeedUpdate = True
	    End If

		PlaySound mSounds.Item("add")
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
				kForce = vpMax(1, mExitForce + (Rnd - 0.5) * mForceVar * (0.8 * iiBall))   ' Dampen force a bit on subsequent balls.

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
				PlaySound mSounds.Item("exitBall")
				UpdateTroughSwitches
				NeedUpdate = True
			Else
				PlaySound mSounds.Item("exit")
			End If
		End If
 	End Sub
 	Public Sub ExitSol_On : solOut(true) : End Sub
End Class

'--------------------
'     Saucer
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
		PlaySound mSounds.Item("add")
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
            PlaySound mSounds.Item("exitBall")
        Else
            PlaySound mSounds.Item("exit")
        End If
    End Sub
End Class

'--------------------
'     BallStack (DEPRECATED/LEGACY)
'     Known issues:
'     - Adding more balls than conMaxBalls will crash the script.
'     - If there are more balls in trough than are ever used in a game (eg. Bride of Pinbot),
'       one or more trough switches will be permanently stuck down and may result in a ROM test report.
'     - Trough does not handle stacking balls at exit.
'     - Saucer mode is essentially a hack on top of the trough logic.
'--------------------
Class cvpmBallStack
	Private mSw(), mEntrySw, mBalls, mBallIn, mBallPos(), mSaucer, mBallsMoving
	Private mInitKicker, mExitKicker, mExitDir, mExitForce
	Private mExitDir2, mExitForce2
	Private mEntrySnd, mEntrySndBall, mExitSnd, mExitSndBall, mAddSnd
	Public KickZ, KickBalls, KickForceVar, KickAngleVar

	Private Sub Class_Initialize
		ReDim mSw(conStackSw), mBallPos(conMaxBalls)
		mBallIn = 0 : mBalls = 0 : mExitKicker = 0 : mInitKicker = 0 : mBallsMoving = False
		KickBalls = 1 : mSaucer = False : mExitDir = 0 : mExitForce = 0
		mExitDir2 = 0 : mExitForce2 = 0 : KickZ = 0 : KickForceVar = 0 : KickAngleVar = 0
		mAddSnd = 0 : mEntrySnd = 0 : mEntrySndBall = 0 : mExitSnd = 0 : mExitSndBall = 0
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
		PlaySound mAddSnd
	End Sub

	' A bug in the script engine forces the "End If" at the end
	Public Sub SolIn(aEnabled)     : If aEnabled Then KickIn        : End If : End Sub
	Public Sub SolOut(aEnabled)    : If aEnabled Then KickOut False : End If : End Sub
	Public Sub SolOutAlt(aEnabled) : If aEnabled Then KickOut True  : End If : End Sub
	Public Sub EntrySol_On   : KickIn        : End Sub
	Public Sub ExitSol_On    : KickOut False : End Sub
	Public Sub ExitAltSol_On : KickOut True  : End Sub

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
		If mBalls Then PlaySound mExitSndBall Else PlaySound mExitSnd : Exit Sub
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
					vpmTimer.AddTimer (ii-1)*200, "vpmCreateBall(" & mExitKicker.Name & ").Kick " &_
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
	Public Sub InitExitSnd(aBall, aNoBall)  : mExitSndBall = aBall  : mExitSnd = aNoBall  : End Sub
	Public Sub InitAddSnd(aSnd) : mAddSnd = aSnd : End Sub

	Public Property Let Balls(aBalls)
		Dim ii
		For ii = 1 To conStackSw
			SetSw ii, False : mBallPos(ii) = conStackSw + 1
		Next
		If mSaucer And aBalls > 0 And mBalls = 0 Then vpmCreateBall mExitKicker
		mBalls = aBalls : NeedUpdate = True
	End Property

	Public Default Property Get Balls : Balls = mBalls         : End Property
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
'       Nudge
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
			If vpmVPVer >= 90 and TypeName(mSlingBump(ii)) = "Wall" Then mForce(ii) = mSlingBump(ii).SlingshotThreshold
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

'--------------------
'    DropTarget
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
					if TypeName(obj2) = "HitTarget" Then 	'if object in array is a Target, use .Dropped
						vpmBuildEvent obj2, "Dropped", aName & ".Hit " & ii	'Droptarget_Dropped : DTbank.Hit 1 : End Sub
					else
						If obj2.HasHitEvent Then vpmBuildEvent obj2, "Hit", aName & ".Hit " & ii
					End If
				Next
			Else
				if TypeName(obj1) = "HitTarget" Then 	'if object in array is a Target, use .Dropped
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
		vpmSolWall mDropObj(aNo-1), mDropSnd, True
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
		PlaySound mRaiseSnd : vpmSolWall mDropObj(aNo-1), False, False
		mSwcopy = mDropSw(aNo-1)
		Controller.Switch(mSwcopy) = False
		mAllDn = False : CheckAllDn False
	End Sub

	Public Sub SolDropDown(aEnabled)
		Dim mSwcopy
		Dim ii : If Not aEnabled Then Exit Sub
		PlaySound mDropSnd
		For Each ii In mDropObj : vpmSolWall ii, False, True : Next
		For Each ii In mDropSw  : mSwcopy = ii : Controller.Switch(mSwcopy) = True : Next
		mAllDn = True : CheckAllDn True
	End Sub

	Public Sub SolDropUp(aEnabled)
		Dim mSwcopy
		Dim ii : If Not aEnabled Then Exit Sub
		PlaySound mRaiseSnd
		For Each ii In mDropObj : vpmSolWall ii, False, False : Next
		For Each ii In mDropSw  : mSwcopy = ii : Controller.Switch(mSwcopy) = False : Next
		mAllDn = False : CheckAllDn False
	End Sub

	Public Sub DropSol_On : SolDropUp True : End Sub
End Class

'--------------------
'       Magnet
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
	Public Property Get Range        : Range = Size : End Property
End Class

'--------------------
'     Turntable
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
'     Mech
'--------------------
Const vpmMechLinear    = &H00
Const vpmMechNonLinear = &H01
Const vpmMechCircle    = &H00
Const vpmMechStopEnd   = &H02
Const vpmMechReverse   = &H04
Const vpmMechOneSol    = &H00
Const vpmMechOneDirSol = &H10
Const vpmMechTwoDirSol = &H20
Const vpmMechStepSol   = &H40
Const vpmMechSlow      = &H00
Const vpmMechFast      = &H80
Const vpmMechStepSw    = &H00
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
	Public Property Get Speed    : Speed = Controller.GetMech(-mMechNo)   : End Property
	Public Property Let Callback(aCallBack) : Set mCallback = aCallBack : End Property

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
'   Captive Ball
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
		mBallDir = aBallDir : mBallCos = Cos(aBallDir * 3.1415927/180) : mBallSin = Sin(aBallDir * 3.1415927/180)
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
'   Visible Locks
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
		If mBalls > 0 Then PlaySound mBallSnd : Else PlaySound mNoBallSnd : Exit Sub
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
'   View Dips
'--------------------
Class cvpmDips
	Private mLWF, mChkCount, mOptCount, mItems()

	Private Sub Class_Initialize
		ReDim mItems(100)
	End Sub

	Private Sub addChkBox(aType, aLeft, aTop, aWidth, aNames)
		Dim ii, obj
		If Not isObject(mLWF) Then Exit Sub
		For ii = 0 To UBound(aNames) Step 2
			Set obj = mLWF.AddCtrl("chkBox", 10+aLeft, 5+aTop+ii*7, aWidth, 14, aNames(ii))
			mChkCount = mChkCount + 1 : mItems(mChkCount+mOptCount) = Array(aType, obj, mChkCount, aNames(ii+1), aNames(ii+1))
		Next
	End Sub

	Private Sub addOptBox(aType, aLeft, aTop, aWidth, aHeading, aMask, aNames)
		Dim ii, obj
		If Not isObject(mLWF) Then Exit Sub
		mLWF.AddCtrl "Frame", 10+aLeft, 5+aTop, 10+aWidth, 7*UBound(aNames)+25, aHeading
		If aMask Then
			For ii = 0 To UBound(aNames) Step 2
				Set obj = mLWF.AddCtrl("OptBtn", 10+aLeft+5, 5+aTop+ii*7+14, aWidth, 14, aNames(ii))
				mOptCount = mOptCount + 1 : mItems(mChkCount+mOptCount) = Array(aType+2,obj,mOptCount,aNames(ii+1),aMask)
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
			mItems(ii)(1).Value = -((dips(mItems(ii)(0) And &H01) And mItems(ii)(4)) = mItems(ii)(3))
			If (mItems(ii)(0) And &H01) = 0 Then useDip = True
		Next
		mLWF.Show GetPlayerHWnd
		dips(0) = 0 : dips(1) = 0
		For ii = 1 To mChkCount + mOptCount
			If mItems(ii)(1).Value Then dips(mItems(ii)(0) And &H01) = dips(mItems(ii)(0) And &H01) Or mItems(ii)(3)
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
'   Impulse Plunger
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

	Public Sub Fire	    	  ' Resets System and Transfer Power Value
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

	Public Sub AutoFire	  ' Auto-Fire Specific Call (so you don't have to change timing)
		IMPowerOut = -Strength + ((Rnd) * RandomOut)
		PlungeOn = True
		Update
		PlungeOn = False
		Pull = 0 : IMPowerOut = 0 : IMPowerTrans = 0 : mCount = 0
		If BallOn = 1 Then : PlaySound mExitSndBall : Else : PlaySound mExitSnd : End If
	End Sub
	
	Public Sub Pullback     ' Pull Plunger
		Pull = 0 : IMPowerOut = 0 : IMPowerTrans = 0 : mCount = 0 ' reinitialize to be sure
		Pull = 1 : NeedUpdate = True
    		PlaySound mEntrySnd
	End Sub
	
	Public Sub Switch(aSw)
		SwitchOn = True
		SwitchNum = aSw
	End Sub
	
    Public Sub InitEntrySnd(aNoBall) : mEntrySnd = aNoBall : End Sub
    Public Sub InitExitSnd(aBall, aNoBall)  : mExitSndBall = aBall  : mExitSnd = aNoBall  : End Sub
End Class

Set vpmTimer = New cvpmTimer
If LoadScript("NudgePlugIn.vbs") Then Set vpmNudge = New cvpmNudge2 Else Set vpmNudge = New cvpmNudge

'---------------------------
' Check VP version running
'---------------------------
Private Function vpmCheckVPVer
	On Error Resume Next
	' a bug in VBS?: Err object is not cleared on Exit Function
	If VPBuildVersion < 0 Or Err Then vpmCheckVPVer = 50 : Err.Clear : Exit Function
	If VPBuildVersion > 2806 and VPBuildVersion < 9999 Then
		vpmCheckVPVer = 63
	ElseIf VPBuildVersion > 2721 and VPBuildVersion < 9999 Then
		vpmCheckVPVer = 61
	ElseIf VPBuildVersion >= 900 and VPBuildVersion <= 999 Then
		vpmCheckVPVer = 90
	ElseIf VPBuildVersion >= 10000 Then
		vpmCheckVPVer = 100
	Else
		vpmCheckVPVer = 60
	End If
End Function
Private vpmVPVer : vpmVPVer = vpmCheckVPVer()
'--------------------
' Initialise timers
'--------------------
Sub PulseTimer_Init   : vpmTimer.InitTimer Me, False : End Sub
Sub PinMAMETimer_Init : Me.Interval = PinMAMEInterval : Me.Enabled = True : End Sub

'---------------------------------------------
' Init function called from Table_Init event
'---------------------------------------------
Public Sub vpmInit(aTable)
	Set vpmTable = aTable
	If vpmVPVer >= 60 Then
	  On Error Resume Next
		If Not IsObject(GetRef(aTable.name & "_Paused")) Or Err Then Err.Clear : vpmBuildEvent aTable, "Paused", "Controller.Pause = True"
		If Not IsObject(GetRef(aTable.name & "_UnPaused")) Or Err Then Err.Clear : vpmBuildEvent aTable, "UnPaused", "Controller.Pause = False"
		If Not IsObject(GetRef(aTable.name & "_Exit")) Or Err Then Err.Clear : vpmBuildEvent aTable, "Exit", "Controller.Pause = False:Controller.Stop"
	End If
	if UseModSol Then
		If Controller.Version >= 02080000 Then
		  Controller.SolMask(2)=1
		Else
		  MsgBox "Modulated Flashers/Solenoids not supported with this Visual PinMAME version (2.8 or newer is required)"
		End If
	End If
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
		aKicker.CreateSizedBallWithMass BSize,BMass ' for whatever reason it doesn't work if using ()
	End If
	Set vpmDefCreateBall3 = aKicker
End Function

If VPBuildVersion >= 10000 Then
	Set vpmCreateBall = GetRef("vpmDefCreateBall3")
ElseIf VPBuildVersion > 909 And vpmVPVer >= 90 Then
        Set vpmCreateBall = GetRef("vpmDefCreateBall2")
Else
	Set vpmCreateBall = GetRef("vpmDefCreateBall")
End If

Private vpmTrough ' Default Trough. Used to clear up missing balls
Private vpmTable  ' Table object

'-------------------
' Main Loop
'------------------
Private Const CHGNO = 0
Private Const CHGSTATE = 1
Private vpmTrueFalse : vpmTrueFalse = Array(" True", " False"," True")

Sub vpmDoSolCallback(aNo, aEnabled)
	If SolCallback(aNo) <> "" Then Execute SolCallback(aNo) & vpmTrueFalse(aEnabled+1)
End Sub

Sub vpmDoLampUpdate(aNo, aEnabled)
	On Error Resume Next : Lights(aNo).State = Abs(aEnabled)
End Sub

Sub PinMAMETimer_Timer
	Dim ChgLamp,ChgSol,ChgGI, ii, tmp, idx, nsol, solon
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
		If UseSolenoids Then ChgSol = Controller.ChangedSolenoids
		If isObject(GICallback) or isObject(GICallback2) Then ChgGI = Controller.ChangedGIStrings
		MotorCallback
	On Error Goto 0
	If Not IsEmpty(ChgLamp) Then
		On Error Resume Next
			For ii = 0 To UBound(ChgLamp)
				idx = chgLamp(ii, 0)
				If IsArray(Lights(idx)) Then
					For Each tmp In Lights(idx) : tmp.State = ChgLamp(ii, 1) : Next
				Else
					Lights(idx).State = ChgLamp(ii, 1)
				End If
			Next
			For Each tmp In vpmMultiLights
				For ii = 1 To UBound(tmp) : tmp(ii).State = tmp(0).State : Next
			Next
			LampCallback
		On Error Goto 0
	End If
	If Not IsEmpty(ChgSol) Then
		For ii = 0 To UBound(ChgSol)
			nsol = ChgSol(ii, 0)
			tmp = SolCallback(nsol)
			solon = ChgSol(ii, 1)
			If solon > 1 Then solon = 1
			If UseModSol Then
				If solon <> SolPrevState(nsol) Then
					SolPrevState(nsol) = solon
					If tmp <> "" Then Execute tmp & vpmTrueFalse(solon+1)
				End If
				tmp = SolModCallback(nsol)
				If tmp <> "" Then Execute tmp & " " & ChgSol(ii, 1)
			Else
				If tmp <> "" Then Execute tmp & vpmTrueFalse(solon+1)
			End If
		Next
	End If
	If Not IsEmpty(ChgGI) Then
		For ii = 0 To UBound(ChgGI)
			GICallback ChgGI(ii, 0), CBool(ChgGI(ii, 1))
			GICallback2 ChgGI(ii, 0), ChgGI(ii, 1)
		Next
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
		Case "Wall"                        aObj.IsDropped = aEnabled
		Case "Bumper", "Light"             aObj.State     = Abs(aEnabled)
		Case "Kicker", "Trigger", "Timer"  aObj.Enabled   = aEnabled
		Case "Gate"                        aObj.Open      = aEnabled
		Case "Integer"                     mSwcopy = aObj : Controller.Switch(mSwcopy) = aEnabled
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
	vpmIsCollection =  TypeName(aObj) = "Collection" Or TypeName(aObj) = "ICollection"
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
			Case "Bumper", "Gate"
				vpmBuildEvent obj, "Hit","vpmTimer.PulseSw " & Obj.TimerInterval
			Case "Spinner"
				vpmBuildEvent obj, "Spin","vpmTimer.PulseSw " & Obj.TimerInterval
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
			.Color = aBall.Color   : .Image = aBall.Image
			If vpmVPVer >= 60 Then
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
			Answer=MsgBox("Click YES to Add a ball to the Trough, NO Removes a ball from the Trough",vbYesNoCancel + vbQuestion)
		If Answer = vbYes Then vpmTrough.AddBall 0
		If Answer = vbNo Then vpmTrough.Balls=vpmTrough.Balls-1
	End If
End Sub

'----------------------------
' Generic solenoid handlers
'----------------------------
' ----- Flippers ------
Sub vpmSolFlipper(aFlip1, aFlip2, aEnabled)
	Dim oldStrength, oldSpeed
	If aEnabled Then
		PlaySound SFlipperOn : aFlip1.RotateToEnd : If Not aFlip2 Is Nothing Then aFlip2.RotateToEnd
	Else
		PlaySound SFlipperOff
        If VPBuildVersion < 10000 Then
			oldStrength = aFlip1.Strength : aFlip1.Strength = conFlipRetStrength
            oldSpeed = aFlip1.Speed : aFlip1.Speed = conFlipRetSpeed
        End If
		aFlip1.RotateToStart
        If VPBuildVersion < 10000 Then
		    aFlip1.Strength = oldStrength
            aFlip1.Speed = oldSpeed
        End If
		If Not aFlip2 Is Nothing Then
            If VPBuildVersion < 10000 Then
				oldStrength = aFlip2.Strength : aFlip2.Strength = conFlipRetStrength
                oldSpeed = aFlip2.Speed : aFlip2.Speed = conFlipRetSpeed
            End If
			aFlip2.RotateToStart
            If VPBuildVersion < 10000 Then
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
	vpmPlaySound aEnabled, aSound
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
			For Each obj In aObj1(0) : vpmToggleObj obj, aEnabled     : Next
			For Each obj In aObj1(1) : vpmToggleObj obj, Not aEnabled : Next
		Else
			For Each obj In aObj1    : vpmToggleObj obj, aEnabled     : Next
		End If
	ElseIf Not aObj1 Is Nothing Then
		vpmToggleObj aObj1, aEnabled
	End If
	If vpmIsArray(aObj2) Then
		If IsArray(aObj2(0)) Then
			For Each obj In aObj2(0) : vpmToggleObj obj, Not aEnabled : Next
			For Each obj In aObj2(1) : vpmToggleObj obj, aEnabled     : Next
		Else
			For Each obj In aObj2    : vpmToggleObj obj, Not aEnabled : Next
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
	szKeyMsg = "The following keys are defined: "                  & vbNewLine &_
	           "(American keyboard layout)"                        & vbNewLine &_
		vbNewLine & "Visual PinMAME keys:"                         & vbNewLine &_
		vpmKeyName(keyShowOpts)   & vbTab & "Game options..."      & vbNewLine &_
		vpmKeyName(keyShowKeys)   & vbTab & "Keyboard settings..." & vbNewLine &_
		vpmKeyName(keyReset)      & vbTab & "Reset emulation"      & vbNewLine &_
		vpmKeyName(keyFrame)      & vbTab & "Toggle Display lock"  & vbNewLine &_
		vpmKeyName(keyDoubleSize) & vbTab & "Toggle Display size"  & vbNewLine
	If IsObject(vpmShowDips) Then
			szKeyMsg = szKeyMsg & vpmKeyName(keyShowDips)   & vbTab & "Show DIP Switch / Option Menu" & vbNewLine
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
	szKeyMsg = szKeyMsg & vbNewLine & "Visual Pinball keys:"     & vbNewLine &_
		vpmKeyName(LeftFlipperKey)  & vbTab & "Left Flipper"     & vbNewLine &_
		vpmKeyName(RightFlipperKey) & vbTab & "Right Flipper"    & vbNewLine &_
		vpmKeyName(LeftMagnaSave)   & vbTab & "Left Magna Save"  & vbNewLine &_
		vpmKeyName(RightMagnaSave)  & vbTab & "Right Magna Save" & vbNewLine &_
		vpmKeyName(PlungerKey)      & vbTab & "Launch Ball"      & vbNewLine &_
		vpmKeyName(StartGameKey)    & vbTab & "Start Button"     & vbNewLine &_
		vpmKeyName(AddCreditKey)    & vbTab & "Insert Coin 1"    & vbNewLine &_
		vpmKeyName(AddCreditKey2)   & vbTab & "Insert Coin 2"    & vbNewLine &_
		vpmKeyName(ExitGame)        & vbTab & "Exit Game"        & vbNewLine &_
		vpmKeyName(MechanicalTilt)  & vbTab & "Mechanical Tilt"  & vbNewLine &_
		vpmKeyName(LeftTiltKey)     & vbTab & "Nudge from Left"  & vbNewLine &_
		vpmKeyName(RightTiltKey)    & vbTab & "Nudge from Right" & vbNewLine &_
		vpmKeyName(CenterTiltKey)   & vbTab & "Nudge forward"    & vbNewLine
	MsgBox szKeyMsg,vbOkOnly,"Keyboard Settings..."
End Sub

Private Sub NullSub(no,enabled) 
'Place Holder Sub
End Sub

'added thanks to Koadic
Sub NVOffset(version) ' version 2 for dB2S compatibility
	Dim check,nvcheck,v,vv,nvpath,rom
	Set check = CreateObject("Scripting.FileSystemObject")
	Set nvcheck = CreateObject("WScript.Shell")
	nvpath = nvcheck.RegRead("HKCU\Software\Freeware\Visual PinMame\globals\nvram_directory") & "\"
	rom = controller.gamename
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
	dim VolPM,VolPMNew
	VolPM = Controller.Games(controller.GameName).Settings.Value("volume")
	VolPMNew = InputBox ("Enter desired VPinMame Volume Level (-32 to 0)","VPinMame Volume",VolPM)
	If VolPMNew = "" Then Exit Sub
	If VolPMNew <=0 and VolPMNew >= -32 Then
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

LoadScript("GlobalPlugIn.vbs") 			' Checks for existance of GlobalPlugIn.vbs and loads it if found, useful for adding
										' custom scripting that can be used for all tables instead of altering the core.vbs