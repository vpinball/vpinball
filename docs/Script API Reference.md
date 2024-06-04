# Script API Reference

This is a quick overview of all commands/variables VP knows. A lot of them do not have an effect when called from the script because they have an impact on the pre-rendering only.
Variables starting with a (*) can't be changed by the script (most can be read though) or don't have an effect.

Also see https://docs.orbitalpin.com/welcome for some more detailed help
As for coding light sequences for original tables, please check this helper app: https://github.com/horseyhorsey/VpLightSequencing

## Table of Content
- [Globals](#globals)
- API Reference per element
  - [Ball](#ball)
  - [Bumper](#bumper)
  - [Decal](#decal)
  - [EM Reel](#emreel)
  - [Flasher](#flasher)
  - [Flipper](#flipper)
  - [Gate](#gate)
  - [Kicker](#kicker)
  - [Light](#light)
  - [Plunger](#plunger)
  - [Primitive](#primitive)
  - [Ramp](#ramp)
  - [Rubber](#rubber)
  - [Spinner](#spinner)
  - [Table](#table)
  - [Target](#target)
  - [Textbox](#textbox)
  - [Timer](#timer)
  - [Trigger](#trigger)
  - [Wall](#wall)
- [Changelog](#changelog)



-------------------------------------------------------------------------------
## Globals

### Variables

| Name | Type | Description |
| ---- | ---- | ----------- |
| *FrameIndex | int | index of the current frame, incremented for each new frame |
| *GameTime | int | gets the current internal game time in milliseconds |
| *SystemTime | int | gets the current system time in milliseconds |
| | | |
| *GetCustomParam | int | 1..9, i.e. `GetCustomParam(1)` to get "param1" and `GetCustomParam(2)` to get "param2" from the command line "VPinballX.exe -c1 param1 -c2 param2" |
| | | |
| *NightDay | int | 0..100 value set in the UI (by the user) that defines if its night or day, so that the script can tweak table elements based on that value |
| | | |
| *LeftFlipperKey | int | |
| *RightFlipperKey | int | |
| *StagedLeftFlipperKey | int | |
| *StagedRightFlipperKey | int | |
| *LeftTiltKey | int | |
| *RightTiltKey | int | |
| *CenterTiltKey | int | |
| *PlungerKey | int | |
| *StartGameKey | int | |
| *AddCreditKey | int | |
| *AddCreditKey2 | int | |
| *LeftMagnaSave | int | |
| *RightMagnaSave | int | |
| *LockbarKey | int | |
| *JoyCustomKey(int index) | int | 1..4 to get the custom key mappings |
| | | |
| *ActiveBall | Ball | returns the current ball |
| *ActiveTable | Table | returns the currently played table element |
| | | |
| *ShowDT | bool | Desktop mode enabled (true) or Rotated Fullscreen mode enabled (false) |
| *ShowFSS | bool | Extended Backglass mode enabled (true) or not (false) |
| | | |
| *WindowWidth | int | get width of rendering window/fullscreen |
| *WindowHeight | int | get height of rendering window/fullscreen |
| | | |
| *Setting(string, string) | string | get the value of one of the VPX .ini settings |
| | | |
| DMDWidth | int | set width of DMD |
| DMDHeight | int | set height of DMD |
| DMDPixels | Variant | set array of (byte-)values (0..100), size needs to match the previously set width and height |
| DMDColoredPixels | Variant |  set array of (long-)values (0..255,0..255,0..255), size needs to match the previously set width and height (see example below) |
| | | |
| *RenderingMode | int | returns the rendering mode: 0 = Normal 2D, 1 = Stereo 3D, 2 = VR |
| DisableStaticPrerendering | bool | Note that when setting to false, if needed, VPX will update the static prerender which can be lengthy |

Example for using DMDColoredPixels:
```
Dim myDMDdata
myDMDdata = Array(CLng(0),CLng(&hFFFFFF),CLng(&hFFFFFF),CLng(0)) 'set 4 fully black/white pixels
DMDWidth = 2
DMDHeight = 2
DMDColoredPixels = myDMDdata
```

### Methods
- `Nudge(float Angle, float Force)`
- `NudgeGetCalibration/NudgeSetCalibration(int XMax, int YMax, int XGain, int YGain, int DeadZone, int TiltSensitivty)`
- `NudgeSensorStatus(VARIANT *XNudge, VARIANT *YNudge)`
- `NudgeTiltStatus(VARIANT *XPlumb, VARIANT *YPlumb, VARIANT *TiltPercent)`
- `PlaySound(string, int loopcount, float volume, float pan, float randompitch, int pitch, bool useexisting, bool restart, float front_rear_fade)`
 
  also allows to increase/decrease the frequency of an already playing samples, and in general to apply all the settings to an already playing sample, and to choose if to restart this playing sample from the beginning or not
  - loopcount chooses the amount of loops
  - volume is in 0..1
  - pan ranges from -1.0 (left) over 0.0 (both) to 1.0 (right)
  - randompitch ranges from 0.0 (no randomization) to 1.0 (vary between half speed to double speed)
  - pitch can be positive or negative and directly adds onto the standard sample frequency
  - useexisting is 0 or 1 (if no existing/playing copy of the sound is found, then a new one is created)
  - restart is 0 or 1 (only useful if useexisting is 1)
  - front_rear_fade is similar to pan but fades between the front and rear speakers
  - so f.e. PlaySound "FlipperUp",0,0.5,-1.0,1.0,1,1,0.0 would play the sound not looped (0), at half the volume (0.5), only on left speaker (-1.0), with varying pitch (1.0), it would reuse the same channel if it is already playing (1), restarts the sample (1), and plays it on the front speakers only (0.0)
- `StopSound(string)`
- `PlayMusic(string, float volume)` - volume 0..1, one can pass a full path, or just a filename (that is placed in the music subfolder)
- `MusicVolume(float volume)` - 0..1
- `EndMusic)`
- `FireKnocker(int Count)`
- `QuitPlayer(int CloseType)`
- `Version` - returns `VP_VERSION_MAJOR * 1000 + VP_VERSION_MINOR * 100 + VP_VERSION_REVISION`
- `VPBuildVersion` - same, deprecated
- `VersionMajor` - major VP version number
- `VersionMinor` - minor VP version number
- `VersionRevision` - VP version revision
- `GetBalls(Ball)` - returns all Balls on the table
- `GetElements(Editable)` - returns all Elements on the table
- `GetElementByName(string)`  - returns a certain Element, based on the name
- `UpdateMaterial(string, float wrapLighting, float roughness, float glossyImageLerp, float thickness, float edge, float edgeAlpha, float opacity,
               OLE_COLOR base, OLE_COLOR glossy, OLE_COLOR clearcoat, VARIANT_BOOL isMetal, VARIANT_BOOL opacityActive,
               float elasticity, float elasticityFalloff, float friction, float scatterAngle)` - updates all parameters of a material (same input ranges as used in the material editor)
- `GetMaterial(string, float wrapLighting, float roughness, float glossyImageLerp, float thickness, float edge, float edgeAlpha, float opacity,
               OLE_COLOR base, OLE_COLOR glossy, OLE_COLOR clearcoat, VARIANT_BOOL isMetal, VARIANT_BOOL opacityActive,
               float elasticity, float elasticityFalloff, float friction, float scatterAngle)` - get all parameters of a material
- `UpdateMaterialPhysics(string, float elasticity, float elasticityFalloff, float friction, float scatterAngle)` - update only the physics parameters of a material (same input ranges as used in the material editor)
- `GetMaterialPhysics(string, float elasticity, float elasticityFalloff, float friction, float scatterAngle)` - get only the physics parameters of a material
- `MaterialColor(string,OLE_COLOR)` - change only the base color of a material

- `GetSerialDevices(strings)` - returns all serial communication port names
- `OpenSerial(string)` - open serial communication for port 'name'
- `CloseSerial` - close serial communication
- `FlushSerial` - flush
- `SetupSerial(int baud, int bits, int parity, int stopbit, VARIANT_BOOL rts, VARIANT_BOOL dtr)` - setup the parameters of the serial communication
- `ReadSerial(int size, VARIANT pVal)` - read size bytes
- `WriteSerial(VARIANT pVal)` - write bytes

- `LoadValue(string,string,VARIANT)` - load a value for tablename,valuename,value
- `SaveValue(string,string,VARIANT)` - save a value for tablename,valuename,value

- `UserDirectory(string)` - returns path to user directory, e.g. C:\Visual Pinball\user
- `TablesDirectory(string)` - returns path to tables directory, e.g. C:\Visual Pinball\tables
- `MusicDirectory(optional string, string)` - returns path to music directory, e.g. C:\Visual Pinball\music, eventually appending the provided sub directory
- `ScriptsDirectory(string)` - returns path to scripts directory, e.g. C:\Visual Pinball\scripts
- `PlatformOS(string)` - returns OS that VPX is running on, e.g. windows,linux,android,macos,ios,tvos
- `PlatformCPU(string)` - returns CPU that VPX is running on, e.g. x86 or arm
- `PlatformBits(string)` - returns bitness of VPX executable, i.e. 32 or 64

- `ShowCursor(bool)` - en/disables mouse cursor

- `GetTextFile(string)` - returns content of text file

- `LoadTexture(string imageName, string fileName)` - load the file fileName into image imageName


-------------------------------------------------------------------------------
## Ball

### Variables
| Name | Type | Description |
| ---- | ---- | ----------- |
| Color | OLECOLOR | sets color of ball
| Mass | float | 
| Radius | float | 
| X | float | 
| Y | float | 
| Z | float | 
| VelX | float | 
| VelY | float | 
| VelZ | float | 
| *AngVelX | float |  note that writing to this is supported for legacy reasons, but is and was always ignored 
| *AngVelY | float |  note that writing to this is supported for legacy reasons, but is and was always ignored 
| *AngVelZ | float |  note that writing to this is supported for legacy reasons, but is and was always ignored 
| AngMomX | float | 
| AngMomY | float | 
| AngMomZ | float | 
| Image | string | 
| FrontDecal | string |  image of the ball front decal
| DecalMode | bool |  switch between using the ball decal as ball logo or ball 'scratches'
| ID | int | gets a unique ID for each ball created or allows to set an artificial one to distinguish different balls on the table
| BulbIntensityScale | float |  allows to dampen/scale the intensity of (bulb-)light reflections on each ball (e.g. to simulate shadowing in the ball lane, etc)
| ReflectionEnabled | bool |  if playfield reflects objects this can be used to enable/disable the reflection for a specific ball
| PlayfieldReflectionScale | float |  allows to dampen/scale the contribution of the playfield reflections on each ball (see BallPlayfieldReflectionScale() for the Table)
| ForceReflection | bool |  if enabled the ball will be reflection on the playfield even it's on a ramp or an upper playfield. (default=0)
| Visible | bool | changes if the ball is visible on the playfield or not (e.g. only physics simulated)
| UserValue | any | can store any user defined value for re-use later-on


-------------------------------------------------------------------------------
## Bumper

### Variables

| Name | Type | Description |
| ---- | ---- | ----------- |
| *Name | string | 
| *CapMaterial | string | 
| *BaseMaterial | string | 
| *SkirtMaterial | string | 
| *Radius | float | default=45.0
| *HeightScale  | float | default=90.0
| *Orientation | float |  
| RingDropOffset | float | An offset to influence the ring animation. The default move down limit is calculated by `(HeightScale*0.5) * Table_Scalez`. The final move down limit is calculated by `RingDropOffset + (HeightScale*0.5)* Table_Scalez`. For example: If the ring is moving too far down you have to try to set the RingDropOffset to -10.
| RingSpeed | float | 
| *CurrentRingOffset | float | Live position of the ring (see `RingDropOffset` and RingSpeed, default is 0..-45 for `RingDropOffset` is 0 and HeightScale is 90)
| *X | float |  X position
| *Y | float |  Y position
| *Surface | string | 
| Force | float | 
| Threshold | float | 
| *Scatter | float | 
| TimerEnabled | bool | 
| TimerInterval | int | set interval for triggering the timer (1 equals 1000 timer calls per second, 1000 equals 1 timer call per second, -1 makes it dependent on the FPS that a user can reach)
| UserValue | any |  can store any user defined value for re-use later-on
| *CapVisible | bool | default=true
| *BaseVisible | bool | default=true
| *ReflectionEnabled | bool | default=true
| EnableSkirtAnimation | bool | default=true, en/disables the skirt animation of a bumper, for example if something external is triggering the bumper (i.e. no ball contact with the skirt)
| HasHitEvent | bool | 
| Collidable | bool | 

### Methods
- `PlayHit()` - if called the bumper will play the hit animation without triggering the actual hit event.

### Events
- `Animate()` - Called when the visual properties of the bumper are animated (ring or skirt)


-------------------------------------------------------------------------------
## Decal

### Variables

| Name | Type | Description |
| ---- | ---- | ----------- |
| *Rotation | float | 
| *Width | float | 
| *Height | float | 
| *X | float | 
| *Y | float | 
| *Type | DecalType | 0=Text, 1=Image
| *Image | string |  texture name
| *Text | string | 
| *SizingType | SizingType | 0=AutoSize, 1=AutoWidth, 2=ManualSize
| *FontColor | OLECOLOR | 
| *Font | IFontDisp |
| *HasVerticalText | bool | 
| *Surface | string | 


-------------------------------------------------------------------------------
## EMReel

### Variables

| Name | Type | Description |
| ---- | ---- | ----------- |
| *Name | string | 
| Width | float | 
| Height | float | 
| X | float | 
| Y | float | 
| Range | float | 
| *BackColor | OLECOLOR | 
| *Image | string |  picture name
| Reels | float | 
| Spacing | float | 
| IsTransparent | bool | 
| Sound | string | 
| Steps | float | 
| TimerEnabled | bool | 
| TimerInterval | int | set interval for triggering the timer (1 equals 1000 timer calls per second, 1000 equals 1 timer call per second, -1 makes it dependent on the FPS that a user can reach)
| UpdateInterval | int | 
| UserValue | any | can store any user defined value for re-use later-on
| *UserImageGrid | bool | 
| ImagesPerGridRow | int | 
| Visible | bool | 

### Methods
- `AddValue(int value)`
- `ResetToZero()`
- `SpinReel(int ReelNumber, int PulseCount)`
- `SetValue(int value)`

### Events
- `Animate()` - Called when the visual properties of the reel are animated


-------------------------------------------------------------------------------
## Flasher

### Variables
| Name | Type | Description |
| ---- | ---- | ----------- |
| *Name | string | 
| TimerEnabled | bool | 
| TimerInterval | int | set interval for triggering the timer (1 equals 1000 timer calls per second, 1000 equals 1 timer call per second, -1 makes it dependent on the FPS that a user can reach)
| ImageA | string |  texture name for image A
| ImageB | string |  texture name for image B
| Filter | string |  use one of the following filters to blend ImageA > ImageB: 'None', 'Additive', 'Multiply', 'Overlay', 'Screen' (filter names are case sensitive!) this feature only works if you defined both images
| Amount | int | defines the filter amount how much of ImageB is filtered over ImageA in percent (0..100, can be set to >100 though)
| Mode | int | 0=ImageModeWorld, 1=ImageModeWrap
| *DisplayTexture | bool | 
| Opacity | int | lets you define the blend amount 0..100% (you can use values >100% though to for example enhance the strength of flashers in additive alpha blending mode)
| IntensityScale | float | sets the flashers brightness/emission scale, so that one can fade in/out all affected flashers with the same scaling factor
| Color - defines the color of the element in the editor. If you don't want to colorize the image set the color to blank white (RGB 255,255,255)
| Visible | bool |  shows/hides the flasher
| AddBlend | bool |  use additive alpha blending instead of classical alpha blending
| EnableDepthMask | bool | Enable/Disable depth masking
| ImageAlignment
| ModulateVsAdd | float | blends between modulating and additive when bulb is enabled (0..1)
| DMD | bool |  enable DMD image via script connection
| DMDWidth | int | set width of DMD
| DMDHeight | int | set height of DMD
| DMDPixels | Variant |  set array of (byte-)values (0..100), size needs to match the previously set width and height
| DMDColoredPixels | Variant |  set array of (long-)values (0..255,0..255,0..255), size needs to match the previously set width and height
| VideoCapWidth | int | width of the window content to capture
| VideoCapHeight | int | height of the window content to capture
| VideoCapUpdate | string |  uses the window title of the window content to capture, which is then copied into the flasher texture each time this function is called
| UserValue | any |  can store any user defined value for re-use later-on


-------------------------------------------------------------------------------
## Flipper

### Variables

| Name | Type | Description |
| ---- | ---- | ----------- |
| *X | float | 
| *Y | float | 
| *BaseRadius | float | 
| *EndRadius | float | 
| *Length | float | 
| *Height | float | 
| *StartAngle | float | 
| *EndAngle | float | 
| *FlipperRadiusMin | float | 
| EOSTorque | float |  This value/factor affects the way how long a flipper arm stays in the up/raised position even when you released the flipper button. On real pinballs it will take some time until the flipper solenoid loses it's power until the flipper drops down or is forced to move by a hitting ball. The default value is 0.75. For fine tuning you can change that but not higher than 1 or lower than 0.5 or the result will be totally unrealistic.
| EOSTorqueAngle | float |  This configures the range where the EOS Torque will be applied
| *CurrentAngle | float |  only readable
| | | |
| Friction | float |  0..1
| RampUp | float |  Coil ramp up speed. Set to 0 for fastest flipper response.
| Elasticity | float | 
| ElasticityFalloff | float | 
| Return | float | 
| Mass | float | 
| Strength | float | 
| Scatter | float | 
| OverridePhysics | int | overrides the flipper physics settings with pre defined settings defined under Preferences -> Physics Options. You can define up to 8 different physics settings for the table and the flippers. Save them to one of the 8 slots and by assigning the slot number to this OverridePhysics it overrides these settings. default=0 (means use table settings)
| TimerEnabled | bool | 
| TimerInterval | int | set interval for triggering the timer (1 equals 1000 timer calls per second, 1000 equals 1 timer call per second, -1 makes it dependent on the FPS that a user can reach)
| *Color | OLECOLOR | 
| *RubberColor | OLECOLOR | 
| *RubberThickness | float | 
| *Visible | bool | 
| *Enabled | bool | 
| *RubberHeight | float | 
| *RubberWidth | float | 
| *Surface | string | 
| UserValue | any | can store any user defined value for re-use later-on

### Methods
- `RotateToStart()`
- `RotateToEnd()`

### Events
- `Animate()` - Called when the visual properties of the flipper are animated (when rotating and its current angle changes)


-------------------------------------------------------------------------------
## Gate

### Variables

| Name | Type | Description |
| ---- | ---- | ----------- |
| *Name | string | 
| *X | float | 
| *Y | float | 
| *Length | float | 
| *Height | float | 
| *Rotation | float | 
| TimerEnabled | bool | 
| TimerInterval | int | set interval for triggering the timer (1 equals 1000 timer calls per second, 1000 equals 1 timer call per second, -1 makes it dependent on the FPS that a user can reach)
| Open | bool | 
| *Material | string | 
| Elasticity | float | 
| *Surface | string | 
| UserValue | any | can store any user defined value for re-use later-on
| *CloseAngle | float | 
| *OpenAngle | float | 
| Collidable | bool | 
| Friction | float |  0..1
| Damping | float |  0..1
| *Visible | bool | 
| ShowBracket | bool | 
| CurrentAngle | float |  only readable, it returns the current open angle
| GravityFactor | float |  usually between 1..3

### Methods
- `Move(int dir, float speed, float angle)` - dir(0/1)

### Events
- `Animate()` - Called when the visual properties of the gate are animated (when the wire rotates and current angle changes)


-------------------------------------------------------------------------------
## Kicker

### Variables
| Name | Type | Description |
| ---- | ---- | ----------- |
| *Name | string | 
| *X | float | 
| *Y | float | 
| TimerEnabled | bool | 
| TimerInterval | int |  set interval for triggering the timer (1 equals 1000 timer calls per second, 1000 equals 1 timer call per second, -1 makes it dependent on the FPS that a user can reach)
| Enabled | bool | 
| *DrawStyle | KickerType | 0=Hidden, 1=Hole, 2=Cup, 3=Invisible
| *Color | OLECOLOR | 
| *Surface | string | 
| UserValue | any | can store any user defined value for re-use later-on
| Scatter | float | 
| Fall Through | bool |  if checked the ball will fall through the surface assigned to the kicker, only if the surface is higher than the playfield
| *Legacy | bool | if checked the kicker will behave as an old VP9 kicker that means no distraction of the ball if it doesn't hit straight the kicker center
| HitAccuracy | float |  defines the accuracy how fast the ball gets caught when rolling over the kicker center (0.0-1.0)
| *HitHeight | float | 
| *Orientation | float | 
| *Radius | float | 
| LastCapturedBall | Ball | returns an instance to the last captured ball or the current captured ball. If no ball was captured before or was destroyed before an error message will be displayed.

### Methods
- `CreateBall()`
- `CreateSizedBall(float radius)`
- `CreateSizedBallWithMass(float radius, float mass)`
- `DestroyBall(int ballId)`
- `Kick(float angle, float speed, float inclination)`
- `KickXYZ(float angle, float speed, float inclination, float x, float y, float z)`
- `KickZ(float angle, float speed, float inclination, float heightz)`
- `BallCntOver(int)`


-------------------------------------------------------------------------------
## Light

### Variables
| Name | Type | Description |
| ---- | ---- | ----------- |
| *Name | string | 
| *X | float | 
| *Y | float | 
| *Shape | Shape | 0=Circle, 1=Custom
| Falloff | float | 
| FalloffPower | float | 
| State | float |  0=Fully off..1=Fully on; 2=Blinking. Note that if Light Sequencer is running, still returns the 'normal' state
| *GetInPlayState | float |  0=Fully off..1=Fully on; 2=Blinking. Note that if Light Sequencer is running, returns the current state set by the Light Sequencer, otherwise the 'normal' state
| *GetInPlayStateBool | bool |  0=Off, 1=On. Note that if Light Sequencer is running, returns the current state set by the Light Sequencer, otherwise the 'normal' state. 2=Blinking is mapped to the current state the light is actually in (0 or 1)
| *GetInPlayIntensity | float |  get the live intensity of the light, including its live fading
| Color | OLECOLOR | 'normal' color
| ColorFull | OLECOLOR | full brightness color
| Intensity | float |  sets the lights brightness/emission
| IntensityScale | float |  sets the lights brightness/emission scale, so that one can fade in/out all affected lights with the same scaling factor
| TransmissionScale | float |  sets the light scale for transmission, f.e. through playfield plastics or transparent ramps that are placed above the bulb light
| TimerEnabled | bool | 
| TimerInterval | int | set interval for triggering the timer (1 equals 1000 timer calls per second, 1000 equals 1 timer call per second, -1 makes it dependent on the FPS that a user can reach)
| BlinkPattern | string |  off/on pattern for the light to use while the interval is going: 1 is ON, 0 is OFF, for example 111000111000101010101
| BlinkInterval | int | 
| *Surface | string | 
| UserValue | any |  can store any user defined value for re-use later-on
| *Image | string | 
| ImageMode | bool |  if true, passthrough Image (as if it would be fully emissive), otherwise light image with global lighting using the attached Surface material
| DepthBias | float | 
| Fader | enum |  Fader used to fade intensity between states: 0 = None, 1 = Linear, 2 = Incandescent
| FadeSpeedUp | float |  speed to fade into on state (value is intensity per ms)
| FadeSpeedDown | float |  speed to fade into off state (value is intensity per ms)
| Bulb | bool | 
| ShowBulbMesh | bool | 
| StaticBulbMesh | bool |  
| ShowReflectionOnBall | bool |  in bulb mode, en/disable the reflection of the bulb on balls
| ScaleBulbMesh | float | 
| BulbModulateVsAdd | float |  blends between modulating and additive when bulb is enabled (0..1)
| BulbHaloHeight | float |  sets the halo height of a bulb light
| Visible | bool | 
| Shadows | int | Shadowing mode: 0=Off, 1=Raytraced ball shadows
| FilamentTemperature | float |  'blackbody' temperature of the filament according to the light current intensity, full intensity corresponding to a temperature of 2700K (value equal or below 1500K do not emit light)

### Methods
- `Duration(float startState, int durationTime, float endState)` - starts a light in startState (0..1 (Off..On); 2 (Blinking)) and leaves it in that state for the amount of milliseconds defined by durationTime until the light switches to the endState
                                                               
  e.g.: `light1.Duration 2, 1000, 1` (light starts to blink for 1000ms and then switches to ON)

### Events
- `Animate()` - Called when the visual properties of the light are animated (when its GetInPlayIntensity changes)


-------------------------------------------------------------------------------
## Plunger

### Variables
| Name | Type | Description |
| ---- | ---- | ----------- |
| *AnimFrames | int | Flat style only: the number of animation frames in the Image; the image must be arranged with all of the frames lined up horizontally, all the same size, with the fully forward image in the leftmost cell
| *AutoPlunger | bool | if set, this is a solenoid launcher rather than a traditional plunger
| *Color | OLECOLOR | surface color for 3D rendering; used only if no texture is specified via Image
| FireSpeed | float |  speed of firing when Fire() is called or mechanical plunger is released; controls both animation speed and ball launch strength
| *Image | string |  image used as texture for the 3D styles or as animation cell(s) for the flat style
| *MechPlunger | bool |  true if this simulation plunger can be controlled by a mechanical plunger input device attached to the system; if set, the ParkPosition is used as the rest position
| MechStrength | float |  controls strength of coupling to mechanical plunger position; using about the same value as FireSpeed usually works best
| MomentumXfer | float |  adjustment factor for launch strength; multiplied into final ball speed calculation on each launch, so 1.0 has no effect, and 2.0 doubles the strength
| *Name | string |  name of the object for scripting references
| ParkPosition | float |  distance back from fully forward position of resting position (as a fraction of the total length); used only if MechPlunger is enabled
| *PullSpeed | float |  speed of retraction when Pullback() is called (for pulling via keyboard interface)
| *RingDiam | float |  Custom type only: the diameter of the e-ring in the on-screen rendering, as a fraction of the Width setting
| *RingGap | float |  Custom type only: the distance between the tip and the e-ring in the on-screen rendering
| *RingThickness | float |  Custom type only: the thickness of the e-ring in the on-screen rendering
| *RodDiam | float |  Custom type only: the drawing diameter of the rod as a fraction of the Width setting
| *SpringDiam | float |  Custom type only: the diameter of the spring coil in the on-screen rendering, as a fraction of the Width setting
| *SpringEndLoops | float |  Custom type only: the number of stationary (non-stretching) "end loops" the spring makes in the on-screen rendering (fractional values are allowed)
| *SpringGauge | float |  Custom type only: the thickness of the spring wire in the on-screen rendering
| *SpringLoops | float |  Custom type only: the number of loops the spring makes around the rod within the displayed extent (fractional values are allowed, as partial loops are perfectly sensible)
| Stroke | float |  total travel distance; also controls the on-screen length (which is slightly longer than the travel distance, to accommodate the non-moving parts)
| ScatterVelocity | float |  amount of randomness added to ball velocity on firing
| *Shape | string |  Custom type only: a list of Distance/Diameter pairs separated by semicolons specifying the shape of the tip, as a series of circles centered on the plunger axis
| *Surface | string |  object for aligning vertical position of plunger; sits atop playfield if this is left blank
| TimerEnabled | bool |  enable the plunger's script timer
| TimerInterval | int |  interval for firing the plunger's script timer
| *Type | int |  the visual style of the plunger drawn on-screen
| UserValue | any |  can store any user defined value for re-use later-on
| *Visible | bool |  true if plunger is visible on-screen
| *Width | float |  on-screen width of the plunger on each side of the center-line (this acts like a radius, so the actual drawing width is twice this value)
| *X | float |  x position of center-line of plunger
| *Y | float |  y position of back end of plunger
| *ZAdjust | float |  moves the on-screen rendering vertically up (positive values) or down (negative values) from the default position determined by the Surface alignment

### Methods
- `Pullback()` - start retracting the plunger at the speed given by PullSpeed; continues automatically until Fire() is called
- `PullBackandRetract()` - start retracting the plunger at the linear speed given by PullSpeed; wait for 1 second at the maximum position; move back towards the resting position; continues automatically until Fire() is called. Can be used instead of Pullback()
- `Fire()` - fire the plunger from the current position (for an auto-plunger, immediately pulls back all the way and fires with full strength)
- `CreateBall(IBall **ball)` - create a new ball located at the tip of the plunger
- `Position()` - returns a float (0 to 25) with the current position of the (mechanical or digital) plunger
- `MotionDevice()` - returns a non-zero integer with the type of special input device attached, or 0 if none is attached (1=Pinball Wizard, 2=Ultracade, 3=SideWinder, 4=VirtuaPin plunger kit, 5=Generic/other)


-------------------------------------------------------------------------------
## Primitive

### Variables
| Name | Type | Description |
| ---- | ---- | ----------- |
| *Name | string | 
| Sides | float |  amount of sides for the old standard primitive
| TopVisible | bool |  toggle visibility
| TopColor | OLECOLOR | topColor of the old standard primitive (only valid if no image was set)
| SideColor | OLECOLOR | sideColor of the old standard primitive (only valid if no image was set)
| DrawTexturesInside | bool |  textures the inside of the old standard primitive
| Image | string |  texture name
| NormalMap | string |  normal map name
| X | float |  X position
| Y | float |  Y position
| Z | float |  Z position
| Size_X | float |  X size
| Size_Y | float |  Y size
| Size_Z | float |  Z size
| RotX | float |  rotates the primitive around it's X axis
| RotY | float |  rotates the primitive around it's Y axis
| RotZ | float |  rotates the primitive around it's Z axis
| TransX | float |  translates the primitive along it's X axis
| TransY | float |  translates the primitive along it's Y axis
| TransZ | float |  translates the primitive along it's Z axis
| ObjRotX | float |  rotates the X axis of the primitive's local coordinate system. This is used to orientate the object first, after that RotXYZ will rotate the object around this rotated coordinate system
| ObjRotY | float |  rotates the Y axis of the primitive's local coordinate system. This is used to orientate the object first, after that RotXYZ will rotate the object around this rotated coordinate system
| ObjRotZ | float |  rotates the Z axis of the primitive's local coordinate system. This is used to orientate the object first, after that RotXYZ will rotate the object around this rotated coordinate system
| *EnableStaticRendering | bool |  if set the primitive is only rendered once (like walls) any dynamic changes later won't have an effect. This is useful for objects like posts or static toys, because this won't have an impact of the perfomance
| HasHitEvent | bool |  enables the support for a hit event in the script (it's the same as for walls)
| Threshold | float |  hit threshold for firing the hit event
| HitThreshold | float |  holds the current hit threshold if a ball has hit the primitive
| Collidable | bool |  enables/disables collision detection, this can be used from within the script.
| Elasticity | float | 
| ElasticityFalloff | float | 
| Friction | float |  0..1 
| *Scatter | float | 
| *IsToy | bool |  disables collision handling and all physics preprocessing completely. It overwrites the Collidable-flag and can't be used from within the script. This option is useful if you have real toys or static primitives which are never hitable at all and are just there for the 'looks'.
| *ReflectionEnabled | bool |  default=true
| DisableLighting | bool | 
| BlendDisableLighting | float |  0..1, same as DisableLighting, but can blend between the two extremes
| BlendDisableLightingFromBelow | float |  0..1, similar to DisableLighting, but will block light coming from below (e.g. from light elements)
| BackfacesEnabled | bool |  default=false, if enabled will also show/render the backfacing triangles (if a transparent material is used)
| UserValue | any |  can store any user defined value for re-use later-on
| Opacity | float |  lets you define the blend amount 0..100% (you can use values >100% though to for example enhance the strength of primitives in additive alpha blending mode)
| Color - defines the color of the element in the editor. If you don't want to colorize the image set the color to blank white (RGB 255,255,255)
| AddBlend | bool |  use additive alpha blending instead of classical alpha blending (also disable depth buffer writing)
| RotAndTra0 | float |  Legacy. Replaced by RotX
| RotAndTra1 | float |  Legacy. Replaced by RotY
| RotAndTra2 | float |  Legacy. Replaced by RotZ
| RotAndTra3 | float |  Legacy. Replaced by TransX
| RotAndTra4 | float |  Legacy. Replaced by TransY
| RotAndTra5 | float |  Legacy. Replaced by TransZ
| RotAndTra6 | float |  Legacy. Replaced by ObjRotX
| RotAndTra7 | float |  Legacy. Replaced by ObjRotY
| RotAndTra8 | float |  Legacy. Replaced by ObjRotZ
| ReflectionProbe | string |  name of reflection probe to be used for mirror reflection
| RefractionProbe | string |  name of refraction probe to be used for transparency


### Methods
The following methods only applies to mesh primitives with an animation sequence
- `PlayAnim(float startFrame, float speed)` - start playing the sequence from 'startFrame' til the end with frame rate 'speed'
- `PlayAnimEndless(float speed)`            - start playing the sequence in a loop with frame rate 'speed'
- `StopAnim()`                              - stop the animation
- `ContinueAnim(float speed)`               - continue a stopped animation with frame rate 'speed'
- `ShowFrame(float frameNr)`                - show the mesh at frame 'frameNr', where 'frameNr' can also be a fractional frame number like 1.23. VP will interpolate between subframes.


-------------------------------------------------------------------------------
## Ramp

### Variables
| Name | Type | Description |
| ---- | ---- | ----------- |
| *Name | string | 
| HeightBottom | float | 
| HeightTop | float | 
| WidthBottom | float | 
| WidthTop | float | 
| *Type | RampType | 0=Flat, 1=4Wire, 2=2Wire, 3=3WireLeft, 4=3WireRight, 5=Wire
| Image | string |  texture name
| ImageAlignment(RampImageAlignment) - 0=ImageModeWorld, 1=ImageModeWrap
| HasWallImage | bool |  apply image to walls
| LeftWallHeight | float | 
| RightWallHeight | float | 
| UserValue | any |  can store any user defined value for re-use later-on
| VisibleLeftWallHeight | float | 
| VisibleRightWallHeight | float | 
| Elasticity | float | 
| HasHitEvent | bool |  enables the support for a hit event in the script (it's the same as for walls)
| Threshold | float |  hit threshold for firing the hit event
| Collidable | bool | 
| Visible | bool | 
| Friction | float |  0..1
| *Scatter | float | 
| *ReflectionEnabled | bool |  default=true


-------------------------------------------------------------------------------
## Rubber

### Variables
| Name | Type | Description |
| ---- | ---- | ----------- |
| *Name | string | 
| TimerEnabled | bool | 
| TimerInterval | int | set interval for triggering the timer (1 equals 1000 timer calls per second, 1000 equals 1 timer call per second, -1 makes it dependent on the FPS that a user can reach)
| *HasHitEvent | bool | 
| *Thickness | int | 
| *Collidable | bool | 
| *HitHeight | float | defines the collision height of a rubber (default is 25.0 midpoint of a standard ball with size 50)
| *Material | string | 
| Visible | bool | 
| *EnableStaticRendering | bool | if set the rubber is only rendered once (like walls) any dynamic changes later won't have an effect. This is useful for static rubbers like rubbers on posts. 
| Image | string |  texture name
| Elasticity | float | 
| ElasticityFalloff | float | 
| Friction | float |  0..1
| *Scatter | float | 
| RotX | float | angle to rotate the whole rubber around the X axis 
| RotY | float | angle to rotate the whole rubber around the Y axis 
| RotZ | float | angle to rotate the whole rubber around the Z axis 
| *EnableShowInEditor | bool | if set one cannot adjust the control points of the rubber but the rotation angles are applied to the rubber mesh in the editor.
| *ReflectionEnabled | bool | default=true
| UserValue | any | can store any user defined value for re-use later-on


-------------------------------------------------------------------------------
## Spinner

### Variables
| Name | Type | Description |
| ---- | ---- | ----------- |
| *Name | string | 
| *X | float | 
| *Y | float | 
| TimerEnabled | bool | 
| TimerInterval | int | set interval for triggering the timer (1 equals 1000 timer calls per second, 1000 equals 1 timer call per second, -1 makes it dependent on the FPS that a user can reach)
| *Length | float | 
| *Rotation | float | 
| *Height | float | 
| Damping | float |  0..1
| Image | string | 
| *Material | string | 
| *Surface | float | 
| UserValue | any |  can store any user defined value for re-use later-on
| *ShowBracket | bool | 
| *AngleMax | float | 
| *AngleMin | float | 
| Elasticity | float | 
| *Visible | bool | 
| CurrentAngle | float |  only readable
| *ReflectionEnabled | bool |  default=true

### Events
- `Animate()` - Called when the visual properties of the gate are animated (when the plate rotates and current angle changes)


-------------------------------------------------------------------------------
## Table

### Variables

| Name | Type | Description |
| ---- | ---- | ----------- |
| *Name | string |  table name
| *FileName | string | file name (without path and extension)
| *Width | float |  Playfield width. default=1000
| *Height | float |  Playfield height. default=2000
| *GlassHeight | float |  Height of Glass above Playfield
| *TableHeight | float | 
| | | |
| *Inclination | float |  Angle of Table (used for Display only). default=43
| *Rotation | float |  Rotation of Table (used mainly for FS/Cab tables: 270)
| *Scalex | float |  X Scale of Table
| *Scaley | float |  Y Scale of Table
| *Scalez | float |  Z Scale of Table
| *Xlatex | float |  X Offset of Table
| *Xlatey | float |  Y Offset of Table
| *Xlatez | float |  Z Offset of Table
| *Layback | float |  Skew of Table (used mainly for FS/Cab tables). default=0
| *FieldOfView | float | Perspective of Table. default=45
| | | |
| *BackglassMode | int | Which Backglass mode is active (0=Desktop,1=Fullscreen,2=FullSingleScreen)
| | | |
| *MaxSeparation | float |  Maximum separation/scale effect for Stereo 3D (ranges from 0..1)
| *ZPD | float |  Reads the depth where the Stereo 3D should happen (ranges from 0..1)
| *Offset | float |  Reads the constant offset for the Stereo 3D effect (ranges from 0..1)
| *PlayfieldColor | OLECOLOR | 
| *BackDropColor | OLECOLOR | 
| BackdropImage | string | 
| | | |
| *SlopeMax | float |  Angle of Playfield within Cabinet (used for Physics only)
| *SlopeMin | float |  Angle of Playfield within Cabinet (used for Physics only)
| *GlobalDifficulty | float |  Mainly chooses/weights between SlopeMax and SlopeMin, but also affects precision of flippers/scattering (0..100)
| Gravity | float |  default=0.86543
| Friction | float |  for the Playfield, default=0.0025
| Elasticity | float |  for the Playfield, default=0.2
| ElasticityFalloff | float |  for the Playfield, default=0
| Scatter | float |  scatter angle for the Playfield, default=0
| DefaultScatter | float |  default=0, overrides all elements that have scatter angle set to < 0
| *OverridePhysics | int |  overrides the table physics settings with pre defined settings defined under Preferences -> Physics Options. You can define up to 8 different physics settings for the table and the flippers. Save them to one of the 8 slots and by assigning the slot number to this OverridePhysics it overrides these settings. default=0 (means use table settings)
| | | |
| PhysicsLoopTime | int | the meaning of this has changed starting with 9.2.1: it specifies the maximum number of iterations spent in the physics update. by setting it f.e. to 1, the rendered frame updates will -always- match the physics updates, everything above 1 allows for multiple physics updates per frame (2, 3 or 4 can be good values for this, as it slows down the physics temporarily if the FPS drop below 50, 33 or 25 in these cases). default=-1 (which means infinite updates allowed)
| | | |
| BallImage | string |  
| BallFrontDecal | string |  image of the ball front decal
| *BallDecalMode | bool |  switch between using the ball decal as ball logo or ball 'scratches'
| | | |
| Image | string |  playfield image (this property may be written, but it will only be visually updated if the playfield is not statically rendered, using a non static playfield_mesh primitive)
| | | |
| *EnableAntialiasing | bool |  enables AA for this table if you have set 'Enable AntiAliasing' in the video options. default=false
| *EnableFXAA | bool |  enables FXAA for this table if you have set 'Enable FXAA' in the video options. default=false
| *RenderEMReels | bool |  default=true
| *RenderDecals | bool |  default=true
| *TableAdaptiveVSync | int | sets adaptive vsync option for this table (-1=use global setting, 0=off, 1=automatic, any other number is refresh rate in Hz)
| *BallReflection | | enable ball reflection on table. depending on the global video setting 'Reflect Ball on Playfield' this is on or off. Or it is defined specific for this table.
| PlayfieldReflectionStrength | int | defines the reflection strength of the (dynamic) table elements on the playfield (0-100)
| BallPlayfieldReflectionScale | float | scale/dampen the contribution of the playfield to the ball reflections
| DefaultBulbIntensityScale | float | scale/dampen the contribution of the bulbs lights on the ball. This is a default setting for all balls created on the the table. If you need ball specific intensity scaling you can change the intensity by using the BulbIntensityScale value on the ball
| *BallTrail | | enable ball trail/motion blur on table. depending on the global video setting 'Ball Trail/Motion Blur' this is on or off. Or it is defined specific for this table.
| *TrailStrength | int | defines the strength of the ball trail (0-100)
| *DetailLevel | int | defines the ramp accuracy for this table and overrides the global setting from the video options (range 0-10).
| | | |
| PlungerNormalize | int | Mech-Plunger component adjustment or weak spring, aging. default=100
| PlungerFilter | bool | default=false
| *Accelerometer | bool |  
| *AccelNormalMount | bool | 
| *AccelerometerAngle | float | 
| *JoltAmount | int | 
| *TiltAmount | int | 
| *JoltTriggerTime | int | 
| *TiltTriggerTime | int | 
| *DeadZone | int | 
| NudgeTime | float | 
| | | |
| *TableSoundVolume | int | Scales the global sound volume (in percent). default=100
| *TableMusicVolume | int | Scales the global music volume (in percent). default=100
| | | |
| *LightAmbient | OLECOLOR | changes the ambient light contribution for each material, please always try to keep this at full Black
| *Light0Emission | OLECOLOR | changes the light contribution for each material (currently light0 emission is copied to light1, too)
| LightEmissionScale | float | 
| EnvironmentEmissionScale | float | 
| AOScale | float | scaling factor when blending the ambient occlusion
| SSRScale | float | scaling factor when blending the screen space reflections
| BloomStrength | float | scaling factor when blurring the clipped framebuffer contribution (leave at 1 for kinda natural glow)
| *LightHeight | float | height of lights
| *LightRange | float | range of lights
| | | |
| ColorGradeImage | string | the 256x16 LUT texture used for the color grading post process
| | | |
| Option | float | Get/Set a custom option (persisted through run, adjustable by the user in Tweak mode). Arguments are: option name, minimum value, maximum value, step between valid values, default value, unit (0=None, 1=Percent), an optional arry of literal strings

### Callback

- `OnBallBallCollision(ball1, ball2, velocity)`

### Events
- `MusicDone()` - Called when the played music has ended.
- `OptionEvent(ByVal eventId)` - Called when an event regarding table option happens: 0 = initialize options (after Init event), 1 = an option has changed, 2 = options have been reseted, 3 = player just exited the Tweak UI



-------------------------------------------------------------------------------
## Target

### Variables
| Name | Type | Description |
| ---- | ---- | ----------- |
| *Name | string | 
| *X | float | 
| *Y | float | 
| *Z | float | 
| *HasHitEvent | bool | if set a hit event is fired when the ball hits the drop/hit target without animation. An additional dropped/raised event is fired for drop targets after the mesh animation is over.
| *Collidable | bool | 
| Threshold | float | hit threshold for firing the hit event
| HitThreshold | float | holds the current hit threshold if a ball has hit the target
| Visible | bool | 
| *Material | string | 
| Image | string |  texture name
| Elasticity | float | 
| ElasticityFalloff | float | 
| Friction | float |  0..1
| *Scatter | float | 
| *Orientation | float | angle to rotate the whole target around the Z axis 
| *ReflectionEnabled | bool | default=true
| DisableLighting | bool | 
| BlendDisableLighting | float |  0..1, same as DisableLighting, but can blend between the two extremes
| BlendDisableLightingFromBelow | float |  0..1, similar to DisableLighting, but will block light coming from below (e.g. from light elements)
| DropSpeed | float | the speed how fast the drop or hit target will move 
| *CurrentAnimOffset | float | the current visual animation: 0..-52 for drop targets, 0..-13 for hit targets
| RaiseDelay | int | defines a delay in milliseconds after the dropped target will start to raise to it's normal position
| IsDropped | bool | can only be used for drop targets if the drop target is raised or dropped
| TimerEnabled | bool | each target has a build-in timer which can be enabled by setting this to 1
| TimerInterval | int | set interval for triggering the timer (1 equals 1000 timer calls per second, 1000 equals 1 timer call per second, -1 makes it dependent on the FPS that a user can reach)
| UserValue | any | can store any user defined value for re-use later-on

### Events
- `Animate()` - Called when the visual properties of the reel are animated


-------------------------------------------------------------------------------
## Textbox

### Variables

| Name | Type | Description |
| ---- | ---- | ----------- |
| BackColor | OLECOLOR | 
| FontColor | OLECOLOR | 
| Text | string | 
| Font
| Name | string | 
| X | float | 
| Y | float | 
| Width | float | 
| Height | float | 
| Alignment
| IsTransparent
| TimerEnabled
| TimerInterval | int |  set interval for triggering the timer (1 equals 1000 timer calls per second, 1000 equals 1 timer call per second, -1 makes it dependent on the FPS that a user can reach)
| UserValue | any | can store any user defined value for re-use later-on
| IntensityScale | float |  scales the color of the textbox
| Visible | bool | 
| DMD | bool |  enable DMD image via script connection


-------------------------------------------------------------------------------
## Timer

### Variables

| Name | Type | Description |
| ---- | ---- | ----------- |
| ***Name*** | string | |
| Enabled    | bool   | |
| Interval   | int    | set interval for triggering the timer (1 equals 1000 timer calls per second, 1000 equals 1 timer call per second, -1 makes it dependent on the FPS that a user can reach) |
| UserValue  | any    | can store any user defined value for re-use later-on |


-------------------------------------------------------------------------------
## Trigger

### Variables
| Name | Type | Description |
| ---- | ---- | ----------- |
| *Name | string | 
| *X | float | 
| *Y | float | 
| *Radius | float | 
| TimerEnabled | bool | 
| TimerInterval | int |  set interval for triggering the timer (1 equals 1000 timer calls per second, 1000 equals 1 timer call per second, -1 makes it dependent on the FPS that a user can reach)
| *Enabled | bool | 
| *Visible | bool | 
| *TriggerShape | Shape | 0=None, 1=Wire, 2=Star
| Surface | string | 
| UserValue | any |  can store any user defined value for re-use later-on
| HitHeight | float | 
| Rotation | float | 
| AnimSpeed | float | 
| *CurrentAnimOffset | float |  Visual offset of trigger when ball passes over it (negative value, 0..-25 for Inder & Wire D triggers, 0..-60 for Wire C, 0..-1/5 of radius for star, 0..-1/10 of radius for buttons, 0..-32 otherwise) 
| WireThickness | float |  This option is only used for wire shape triggers. The thickness is a factor to scale the wire 0=normal thickness(default). To make it thicker use values like 0.5, 1, 1.5, 2...

### Methods
- `BallCntOver(int value)`
- `DestroyBall(int ballId)`

### Events
- `Animate()` - Called when the visual properties of the trigger are animated (when a ball passes over it)


-------------------------------------------------------------------------------
## Wall

### Variables
| Name | Type | Description |
| ---- | ---- | ----------- |
| *Name | string | 
| TimerEnabled | bool | 
| TimerInterval | int |  set interval for triggering the timer (1 equals 1000 timer calls per second, 1000 equals 1 timer call per second, -1 makes it dependent on the FPS that a user can reach)
| *HasHitEvent | bool | 
| Threshold | float | 
| *Image | string |  texture name
| *SideColor | OLECOLOR | 
| *HeightBottom | float | 
| *HeightTop | float | 
| *FaceColor | OLECOLOR | 
| *CanDrop | bool | 
| Collideable | bool | 
| IsBottomSolid | bool | 
| IsDropped | bool | 
| *DisplayTexture | bool | 
| Elasticity | float | 
| ElasticityFalloff | float | 
| *SideImage | string | 
| *Visible | bool | 
| Disabled | bool | 
| *SideVisible | bool | 
| UserValue | any |  can store any user defined value for re-use later-on
| *SlingshotColor | OLECOLOR | 
| SlingshotThreshold | float | 
| *SlingshotAnimation | bool | 
| *FlipbookAnimation | bool | 
| Friction | float |  0..1
| *Scatter | float | 
| *ReflectionEnabled | bool |  default=true
| DisableLighting | bool | 
| BlendDisableLighting | float |  0..1, same as `DisableLighting`, but can blend between the two extremes
| BlendDisableLightingFromBelow | float |  0..1, similar to `DisableLighting`, but will block light coming from below (e.g. from light elements)

### Methods
- `PlaySlingshotHit()` - if the wall has slingshots defined, a call to this function will play the hit animation without triggering the actual hit event.


-------------------------------------------------------------------------------
## Changelog

### 10.8.0
- add `LoadTexture` to the globals
- add `DisableStaticPrerendering` to the globals
- add `StagedLeftFlipperKey`, `StagedRightFlipperKey` and `JoyCustomKey`
- add `TablesDirectory`, `MusicDirectory`, `ScriptsDirectory`, `PlatformOS`, `PlatformCPU`, `PlatformBits` and `ShowCursor` to the globals
- add `Animate` event to Bumper, Light, Flipper, Gate, Spinner, Trigger, Targets and EMReel. This event is called when the object's visual properties are animated, in order to easily sync with custom visuals
- add `CurrentAnimOffset` to the Trigger and Target elements
- add `CurrentRingOffset` to the Bumper Element
- add support for 'modulated' lights by changing the previous light state (0=Off,1=On,2=Blinking) to float (see `State`, `GetInPlayState` and `Duration`), and allowing it to be also defined between 0 and 1 (2 remains the blinking state)
- add `FilamentTemperature` property to lights to get the 'blackbody' temperature of the filament according to the light current intensity, full intensity corresponding to a temperature of 2700K
- add support for multiple custom DMDs by adding `DMDWidth`/`DMDHeight`/`DMDPixels`/`DMDColoredPixels` to flashers. The flasher will use the provided data if any, or defaults to the table data (captured or directly defined)
- add `Shadows` property to Light element
- add `EnableDepthMask` to the primitive element
- `YieldTime` will now do nothing anymore
- add global `FrameIndex`
 
### 10.7.4
- add `Setting` property to the globals
- document missing `GetTextFile`

### 10.7.2
- add `GetInPlayIntensity` to the light element
- add `Opacity`, `Color`, `AddBlend` to the primitive element
- add `RenderingMode` to the globals

### 10.7.1
- internal release only

### 10.7
- add `GetSerialDevices`, `OpenSerial`, `CloseSerial`, `FlushSerial`, `SetupSerial`, `ReadSerial`, `WriteSerial` to the globals
- add `GetInPlayState`, `GetInPlayStateBool` to the light element
- add `VideoCapWidth`, `VideoCapHeight`, `VideoCapUpdate` to the flasher element
- document missing x,y,z to the target element
- add `GetMaterial` to the globals (counterpart to `UpdateMaterial`)
- add `UpdateMaterialPhysics` and `GetMaterialPhysics` to the globals
- add `ActiveTable` to the globals
- document missing `ActiveBall` global call
- add `PullBackandRetract` to the plunger element that can be used instead of PullBack in the KeyDown Sub.
  The plunger will move forth and back between the maximum and minimum position, allowing for a retry, if the desired firing position has been missed.
- add `ElasticityFalloff` to the wall element

### 10.6
- add `UpdateMaterial` to the globals
- fix potential problems for the light element for `Bulb`, `ImageMode`, `ShowBulbMesh`, `StaticBulbMesh`, `ShowReflectionOnBall` reads
- change `Position()` of the plunger element to return a float instead of an integer
- add support for non-mechanical/digital `Position()` plunger call
- add `EnableSkirtAnimation` to the bumper element
- add `GetCustomParam` to access custom command line parameters 
- add support to read out the current hit threshold if the ball hits a Primitive or a HitTarget

### 10.5
- `EOSTorque`, `EOSTorqueAngle` and `Scatter` of flipper element can now be changed by script
- document missing `DMDColoredPixels` and add simple example for DMD access
- add `SSRScale` to the table element
- extend X and Y of EMReel and textbox elements so that these can be changed by the script
- add `GravityFactor` to the gate element
- add `UserValue` to target and primitive elements
- add `BlendDisableLightingFromBelow` to target and wall elements
- add `RingDropOffset` to the bumper element

### 10.4
- support reading nudge data from table script, and reading/writing nudge calibration settings:
  NudgeGetCalibration/NudgeSetCalibration, NudgeSensorStatus, NudgeTiltStatus
- add AngVelX/Y/Z and AngMomX/Y/Z and DecalMode to the ball element
- document missing FrontDecal and UserValue for the ball element
- add Scatter to the bumper element
- add BlendDisableLightingFromBelow to the primitive element
- add WindowWidth and WindowHeight to globals
- document missing LoadValue and SaveValue global call
- correct/move some properties/methods from the table element to globals
- add BlendDisableLighting to target,primitive and wall elements
- document missing target and wall element call (DisableLighting)
- extend PlaySound with front_rear_fade to fade between front and rear speakers

### 10.3.1
- document missing primitive element call (DisableLighting)
 
### 10.3
- document some missing table element calls (all keys, including the new LockbarKey(), and UserDirectory())
- document some missing UserValue's and the timer element itself
- add HasHitEvent and Threshold to the ramp element
- add BackfacesEnabled to the primitive element
- add BackglassMode and MusicVolume and ShowFSS to the table element
- remove *FS variants from the table element (now one can use the normal Inclination,Rotation,etc calls and check the active mode via BackglassMode) 
- add Damping to the gate element
- change Friction to Damping (which was actually AntiFriction before, but never mentioned) for the spinner element (which also changes the input range from 0..100 to 0..1, and is inverse to before, e.g. "1-OldFriction/100")

### 10.2
- add GameTime and SystemTime to get the exact time in millisecond resolution
- extend TimerInterval of each element by a special '-1' value which makes it dependent on the FPS that a user can reach, so the timer is only triggered exactly once per frame
- add MaterialColor to the globals
- extend PlayMusic with optional volume (0..1) and better document PlaySound
- add Duration and BulbHaloHeight property to the light element
- add PlayAnim,PlayAnimEndless,StopAnim,ContinueAnim & ShowFrame to the primitive element for animation use
- add EOSTorqueAngle to the flipper element
- fix TimerEnabled and TimerInterval calls for the flasher element
- add Version, VersionMajor/Minor/Revision for the table element

### 10.1
- add DefaultBulbIntensityScale to the table element
- fix TimerEnabled and TimerInterval calls for the rubber element
- change RubberThickness,RubberHeight,RubberWidth to take floats instead of just integers for the rubber element
- add HitHeight property to the rubber element
- rename TorqueDamping to EOSTorque for the flipper element
- add Scatter to the flipper element
- add NormalMap to the primitive element
- add StaticBulbMesh to the light element
- add HasHitEvent and Collidable to the bumper element
- add Dropped/Raised for drop target element
- add RaiseDelay to target element
- change BallTrailStrength,GlobalDifficultyLevel and PlayfieldReflectionStrength to range 0..100
- add DMD to the textbox and flasher element
