// Versions
PropString(Version, VPinball, "VPX Version"s, "VPX version that saved this file"s, string(VP_VERSION_STRING_DIGITS));

// General Application settings
PropBool(Editor, EnableLog, "Enable Log"s, "Enable general logging to the vinball.log file"s, true);
PropBool(Editor, DisableHash, "Disable File Validation"s, "Disable file integrity validation (risky; but slightly faster loading)"s, false);

// Audio settings
PropInt(Player, MusicVolume, "Backglass Volume"s, "Main volume for music and sound played from the backglass speakers"s, 0, 100, 100);
PropInt(Player, SoundVolume, "Playfield Volume"s, "Main volume for mechanical sounds coming from the playfield"s, 0, 100, 100);
PropBool(Player, PlayMusic, "Enable Backglass"s, "Enable/Disable backglass game sound & music"s, true);
PropBoolBase(Player, PlaySound, "Enable Playfield"s, "Enable/Disable playfield mechanical sounds"s, false, true); // We use the complete macro as Win32 global PlaySound would conflict otherwise
PropStringDyn(Player, SoundDeviceBG, "Backglass Sound Device"s, "Select backglass sound device"s, ""s);
PropStringDyn(Player, SoundDevice, "Playfield Sound Device"s, "Select playfield sound device"s, ""s);
PropEnum(Player, Sound3D, "Playfield Output Mode"s, "Select how playfield sound is output to a speaker configuration"s, int /* VPX::SoundConfigTypes*/, 0 /* VPX::SoundConfigTypes::SNDCFG_SND3D2CH */,
   "2 Front channels"s, "2 Rear channels"s, "Up to 6 channels. Rear at lockbar"s, "Up to 6 channels. Front at lockbar"s, "6ch Side & Rear at lockbar. Legacy mixing"s, "6ch Side & Rear at lockbar. New mixing"s);

// Output (windows) settings
// Main window (a.k.a. playfield)
PropStringDyn(Player, PlayfieldDisplay, "Display"s, "Display used for the main Playfield window"s, ""s);
PropIntDyn(Player, PlayfieldWndX, "X Position"s, "Horizontal position of the window on the selected display"s, 0, 16384, 0);
PropIntDyn(Player, PlayfieldWndY, "Y Position"s, "Vertical position of the window on the selected display"s, 0, 16384, 0);
PropIntDyn(Player, PlayfieldWidth, "Width"s, "Width of the window"s, 0, 16384, 16384);
PropIntDyn(Player, PlayfieldHeight, "Height"s, "Height of the window"s, 0, 16384, 16384);
PropBoolDyn(Player, PlayfieldFullScreen, "Fullscreen"s, "Use fullscreen exclusive mode\nThis should be avoided unless you need to change the display resolution"s, false);
PropIntDyn(Player, PlayfieldFSWidth, "Width"s, "Fullscreen display mode width"s, 0, 16384, 16384);
PropIntDyn(Player, PlayfieldFSHeight, "Height"s, "Fullscreen display mode height"s, 0, 16384, 16384);
PropFloatDyn(Player, PlayfieldRefreshRate, "Refresh Rate"s, "Fullscreen display mode refresh rate"s, 0.f, 1000.f, 0.f);
PropIntDyn(Player, PlayfieldColorDepth, "Color Depth"s, "Fullscreen display mode color depth"s, 0, 64, 32);
// Backglass Window
PropEnumDyn(Backglass, BackglassOutput, "Output Mode"s, "Select between disabled, floating, or embedded in another window mode"s, int /* OutputMode */, 0 /* OM_DISABLED */, "Disabled"s,
   "Floating"s, "Embedded in playfield"s);
PropStringDyn(Backglass, BackglassDisplay, "Display"s, "Display used for the main Backglass window"s, ""s);
PropIntDyn(Backglass, BackglassWndX, "X Position"s, "Horizontal position of the window on the selected display"s, 0, 16384, 0);
PropIntDyn(Backglass, BackglassWndY, "Y Position"s, "Vertical position of the window on the selected display"s, 0, 16384, 0);
PropIntDyn(Backglass, BackglassWidth, "Width"s, "Width of the window"s, 0, 16384, 16384);
PropIntDyn(Backglass, BackglassHeight, "Height"s, "Height of the window"s, 0, 16384, 16384);
PropBoolDyn(Backglass, BackglassFullScreen, "Fullscreen"s, "Use fullscreen exclusive mode\nThis should be avoided unless you need to change the display resolution"s, false);
PropIntDyn(Backglass, BackglassFSWidth, "Width"s, "Fullscreen display mode width"s, 0, 16384, 16384);
PropIntDyn(Backglass, BackglassFSHeight, "Height"s, "Fullscreen display mode height"s, 0, 16384, 16384);
PropFloatDyn(Backglass, BackglassRefreshRate, "Fullscreen Refresh Rate"s, "Fullscreen display mode refresh rate"s, 0.f, 1000.f, 0.f);
PropIntDyn(Backglass, BackglassColorDepth, "Color Depth"s, "Fullscreen display mode color depth"s, 0, 64, 32);
// ScoreView Window
PropEnumDyn(ScoreView, ScoreViewOutput, "Output Mode"s, "Select between disabled, floating, or embedded in another window mode"s, int /* OutputMode */, 0 /* OM_DISABLED */, "Disabled"s,
   "Floating"s, "Embedded in playfield"s);
PropStringDyn(ScoreView, ScoreViewDisplay, "Display"s, "Display used for the main ScoreView window"s, ""s);
PropIntDyn(ScoreView, ScoreViewWndX, "X Position"s, "Horizontal position of the window on the selected display"s, 0, 16384, 0);
PropIntDyn(ScoreView, ScoreViewWndY, "Y Position"s, "Vertical position of the window on the selected display"s, 0, 16384, 0);
PropIntDyn(ScoreView, ScoreViewWidth, "Width"s, "Width of the window"s, 0, 16384, 16384);
PropIntDyn(ScoreView, ScoreViewHeight, "Height"s, "Height of the window"s, 0, 16384, 16384);
PropBoolDyn(ScoreView, ScoreViewFullScreen, "Fullscreen"s, "Use fullscreen exclusive mode\nThis should be avoided unless you need to change the display resolution"s, false);
PropIntDyn(ScoreView, ScoreViewFSWidth, "Width"s, "Fullscreen display mode width"s, 0, 16384, 16384);
PropIntDyn(ScoreView, ScoreViewFSHeight, "Height"s, "Fullscreen display mode height"s, 0, 16384, 16384);
PropFloatDyn(ScoreView, ScoreViewRefreshRate, "Fullscreen Refresh Rate"s, "Fullscreen display mode refresh rate"s, 0.f, 1000.f, 0.f);
PropIntDyn(ScoreView, ScoreViewColorDepth, "Color Depth"s, "Fullscreen display mode color depth"s, 0, 64, 32);
// Topper Window
PropEnumDyn(Topper, TopperOutput, "Output Mode"s, "Select between disabled, floating, or embedded in another window mode"s, int /* OutputMode */, 0 /* OM_DISABLED */, "Disabled"s, "Floating"s,
   "Embedded in playfield"s);
PropStringDyn(Topper, TopperDisplay, "Display"s, "Display used for the Topper window"s, ""s);
PropIntDyn(Topper, TopperWndX, "X Position"s, "Horizontal position of the Topper window on the selected display"s, 0, 16384, 0);
PropIntDyn(Topper, TopperWndY, "Y Position"s, "Vertical position of the Topper window on the selected display"s, 0, 16384, 0);
PropIntDyn(Topper, TopperWidth, "Width"s, "Width of the Topper window"s, 0, 16384, 16384);
PropIntDyn(Topper, TopperHeight, "Height"s, "Height of the Topper window"s, 0, 16384, 16384);
PropBoolDyn(Topper, TopperFullScreen, "Fullscreen"s, "Use fullscreen exclusive mode for the Topper window\nThis should be avoided unless you need to change the display resolution"s, false);
PropIntDyn(Topper, TopperFSWidth, "Width"s, "Fullscreen display mode width for the Topper window"s, 0, 16384, 16384);
PropIntDyn(Topper, TopperFSHeight, "Height"s, "Fullscreen display mode height for the Topper window"s, 0, 16384, 16384);
PropFloatDyn(Topper, TopperRefreshRate, "Fullscreen Refresh Rate"s, "Fullscreen display mode refresh rate"s, 0.f, 1000.f, 0.f);
PropIntDyn(Topper, TopperColorDepth, "Color Depth"s, "Fullscreen display mode color depth"s, 0, 64, 32);
// VR Preview Window
PropStringDyn(PlayerVR, PreviewDisplay, "Display"s, "Display used for the VR Preview window"s, ""s);
PropIntDyn(PlayerVR, PreviewWndX, "X Position"s, "Horizontal position of the window on the selected display"s, 0, 16384, 0);
PropIntDyn(PlayerVR, PreviewWndY, "Y Position"s, "Vertical position of the window on the selected display"s, 0, 16384, 0);
PropIntDyn(PlayerVR, PreviewWidth, "Width"s, "Width of the window"s, 0, 16384, 16384);
PropIntDyn(PlayerVR, PreviewHeight, "Height"s, "Height of the window"s, 0, 16384, 16384);
PropBoolDyn(PlayerVR, PreviewFullScreen, "Fullscreen"s, "Use fullscreen exclusive mode\nThis should be avoided unless you need to change the display resolution."s, false);
PropIntDyn(PlayerVR, PreviewFSWidth, "Width"s, "Fullscreen display mode width"s, 0, 16384, 16384);
PropIntDyn(PlayerVR, PreviewFSHeight, "Height"s, "Fullscreen display mode height"s, 0, 16384, 16384);
PropFloatDyn(PlayerVR, PreviewRefreshRate, "Fullscreen Refresh Rate"s, "Fullscreen display mode refresh rate"s, 0.f, 1000.f, 0.f);
PropIntDyn(PlayerVR, PreviewColorDepth, "Color Depth"s, "Fullscreen display mode color depth"s, 0, 64, 32);
// Array access (using VPXWindowId)
PropArray(Window, Mode, int, Enum, Int, m_propInvalid, m_propBackglass_BackglassOutput, m_propScoreView_ScoreViewOutput, m_propTopper_TopperOutput, m_propInvalid);
PropArray(Window, Display, string, String, String, m_propPlayer_PlayfieldDisplay, m_propBackglass_BackglassDisplay, m_propScoreView_ScoreViewDisplay, m_propTopper_TopperDisplay,
   m_propPlayerVR_PreviewDisplay);
PropArray(Window, WndX, int, Int, Int, m_propPlayer_PlayfieldWndX, m_propBackglass_BackglassWndX, m_propScoreView_ScoreViewWndX, m_propTopper_TopperWndX, m_propPlayerVR_PreviewWndX);
PropArray(Window, WndY, int, Int, Int, m_propPlayer_PlayfieldWndY, m_propBackglass_BackglassWndY, m_propScoreView_ScoreViewWndY, m_propTopper_TopperWndY, m_propPlayerVR_PreviewWndY);
PropArray(Window, Width, int, Int, Int, m_propPlayer_PlayfieldWidth, m_propBackglass_BackglassWidth, m_propScoreView_ScoreViewWidth, m_propTopper_TopperWidth, m_propPlayerVR_PreviewWidth);
PropArray(
   Window, Height, int, Int, Int, m_propPlayer_PlayfieldHeight, m_propBackglass_BackglassHeight, m_propScoreView_ScoreViewHeight, m_propTopper_TopperHeight, m_propPlayerVR_PreviewHeight);
PropArray(Window, FullScreen, bool, Bool, Int, m_propPlayer_PlayfieldFullScreen, m_propBackglass_BackglassFullScreen, m_propScoreView_ScoreViewFullScreen, m_propTopper_TopperFullScreen,
   m_propPlayerVR_PreviewFullScreen);
PropArray(Window, FSWidth, int, Int, Int, m_propPlayer_PlayfieldFSWidth, m_propBackglass_BackglassFSWidth, m_propScoreView_ScoreViewFSWidth, m_propTopper_TopperFSWidth,
   m_propPlayerVR_PreviewFSWidth);
PropArray(Window, FSHeight, int, Int, Int, m_propPlayer_PlayfieldFSHeight, m_propBackglass_BackglassFSHeight, m_propScoreView_ScoreViewFSHeight, m_propTopper_TopperFSHeight,
   m_propPlayerVR_PreviewFSHeight);
PropArray(Window, FSRefreshRate, float, Float, Float, m_propPlayer_PlayfieldRefreshRate, m_propBackglass_BackglassRefreshRate, m_propScoreView_ScoreViewRefreshRate,
   m_propTopper_TopperRefreshRate, m_propPlayerVR_PreviewRefreshRate);
PropArray(Window, FSColorDepth, int, Int, Int, m_propPlayer_PlayfieldColorDepth, m_propBackglass_BackglassColorDepth, m_propScoreView_ScoreViewColorDepth, m_propTopper_TopperColorDepth,
   m_propPlayerVR_PreviewColorDepth);

// Graphics synchronisation and latency reduction
#if defined(ENABLE_BGFX)
PropEnum(Player, SyncMode, "Synchronization"s, "None: No synchronization.\nVertical Sync: Synchronize on video sync, which avoids video tearing, but has higher input latency."s, int, 1, "No Sync"s, "Vertical Sync"s);
#else
PropEnum(Player, SyncMode, "Synchronization"s,
   "None: No synchronization.\nVertical Sync: Synchronize on video sync, which avoids video tearing, but has higher input latency.\nAdaptive Sync: Synchronize on video sync, "
   "except for late frames (below target FPS), also features higher input latency.\nFrame Pacing: same as adaptive sync, but with lower latency. Comes with the risk of introducing more stutters if the computer is not powerful enough."s,
   int, 1, "No Sync"s, "Vertical Sync"s, "Adaptive Sync"s, "Frame Pacing"s);
#endif
PropFloat(Player, MaxFramerate, "Limit Framerate"s,
   "-1 will limit FPS to the display refresh rate\n0 will not limit the display refresh rate\nOther values will limit the FPS to it (energy saving/less heat, framerate stability)"s, -1.f,
   1000.f, -1.f);
PropInt(Player, MaxPrerenderedFrames, "Max. Prerendered Frames"s, "Maximum number of 'frames in flight' (frames pushed to the GPU queue waiting for rendering).\nHigher values may lead to higher FPS, but at higher input latency.\nRecommended to be left at 0 (disabled/system default)"s, 0, 5, 0);
PropInt(Player, VisualLatencyCorrection, "Visual Latency Correction"s,
   "Leave at -1 to get default latency correction based on display frequency.\nIf you measured your setup latency using tools like Intel's PresentMon, enter the average latency in ms."s,
   -1, 200, -1);

// Graphics settings
#if defined(ENABLE_BGFX)
#ifdef __ANDROID__
/* PropEnum(Player, GfxBackend, "Graphics Backend"s, "Graphics API/backend used for rendering"s, int, bgfx::RendererType::OpenGLES, "Noop"s, "Agc"s, "Direct3D11"s, "Direct3D12"s, "Gnm"s,
   "Metal"s, "Nvn"s, "OpenGLES"s, "OpenGL"s, "Vulkan"s, "Default"s);*/
PropString(Player, GfxBackend, "Graphics Backend"s, "Graphics API/backend used for rendering"s, "OpenGLES"s);
#elif defined(__APPLE__)
/* PropEnum(Player, GfxBackend, "Graphics Backend"s, "Graphics API/backend used for rendering"s, int, bgfx::RendererType::Metal, "Noop"s, "Agc"s, "Direct3D11"s, "Direct3D12"s, "Gnm"s,
   "Metal"s,
   "Nvn"s, "OpenGLES"s, "OpenGL"s, "Vulkan"s, "Default"s);*/
PropString(Player, GfxBackend, "Graphics Backend"s, "Graphics API/backend used for rendering"s, "Metal"s);
#else
PropString(Player, GfxBackend, "Graphics Backend"s, "Graphics API/backend used for rendering"s, "Default"s);
/* PropEnum(Player, GfxBackend, "Graphics Backend"s, "Graphics API/backend used for rendering"s, int, bgfx::RendererType::Count, "Noop"s, "Agc"s, "Direct3D11"s, "Direct3D12"s, "Gnm"s,
   "Metal"s,
   "Nvn"s, "OpenGLES"s, "OpenGL"s, "Vulkan"s, "Default"s); */
#endif
#endif
PropEnum(Player, ShowFPS, "Show FPS"s, "Performance overlay display mode"s, int /* PerfUI::PerfMode */, 0, "Disable"s, "FPS"s, "Full"s);
PropBool(Player, SSRefl, "Additive Screen Space Reflection"s, "Add global reflection to the entire scene"s, false);
PropBool(Player, HDRDisableToneMapper, "Disable tonemapping on HDR display"s, "Do not perform tonemapping when rendering on a high dynamic range (HDR) capable monitor/gfxboard/OS"s, true);
PropFloat(Player, HDRGlobalExposure, "HDR Display Global Exposure"s, "Global exposure scale multiplier for high dynamic range (HDR) capable monitors"s, 0.f, 5.f, 1.f);
PropBool(Player, ForceBloomOff, "Disable Bloom"s, "Disable postprocessed bloom filter"s, false);
PropBool(Player, ForceMotionBlurOff, "Disable Motion Blur"s, "Disable postprocessed ball motion blur"s, false);
PropBool(Player, ForceAnisotropicFiltering, "Force Anisotropic Filtering"s, "Force anisotropic filtering for better rendering quality/texture clarity at the cost of a bit of performance"s, true);
PropBool(Player, CompressTextures, "Compress Textures"s, "Automatically compress textures at game startup (slow) for better performance"s, false);
PropBool(Player, UseNVidiaAPI, "Alternative Depth Buffer"s, "Use NVidia API to manage Depth Buffer on a DirectX 9 build. May solve some rendering issues"s, false);
PropBool(Player, SoftwareVertexProcessing, "Software Vertex Processing"s, "Activate this on a DirectX 9 build, if you have issues using an old Intel graphics chip"s, false);
PropBool(Player, DisableAO, "Disable Ambient Occlusion"s, ""s, false);
PropBool(Player, DynamicAO, "Dynamic Ambient Occlusion"s, ""s, true);
PropEnum(Player, PFReflection, "Reflection Quality"s,
   "Limit the quality of reflections for better performance.\n'Dynamic' is recommended and will give the best results, but may harm performance.\n'Static Only' has no performance cost (except for VR rendering).\nOther options feature different trade-offs between quality and performance."s,
   int, 5, "Disable Reflections"s, "Balls Only"s, "Static Only"s, "Static & Balls"s, "Static & Unsynced Dynamic"s, "Dynamic"s);
PropInt(Player, MaxTexDimension, "Maximum texture dimension"s, "Images sized above this limit will be automatically scaled down on load"s, 512, 16384, g_isMobile ? 1536 : 16384);
PropInt(Player, AlphaRampAccuracy, "Detail Level"s, "Level of detail for balls and ramps"s, 1, 10, 10);
PropEnum(Player, BGSet, "View Mode"s, "Select between desktop, cabinet or 'full single screen' viewing mode configurations (if a table has set them up correctly)"s, int, 0, "Desktop / Full Single Screen"s, "Cabinet"s, "Full Single Screen"s);
PropEnum(Player, CabinetAutofitMode, "Cabinet Autofit Mode"s, "Select between manual setup, automatic fitting without visual stretch or automatic fitting with table stretching"s, int, 0,
   "Manual"s, "Fit Table"s, "Fit Screen"s);
PropFloat(Player, CabinetAutofitPos, "Cabinet Autofit Pos"s, "Relative lower flipper bat position when using automatic view fitting (0% for bottom of screen, 100% for top)"s, 0.f, 0.2f, 0.05f);

// Aliasing & sharpening
PropFloat(Player, AAFactor, "Full Scene Anti Aliasing"s,
   "Enables brute-force Up/Downsampling (similar to DSR).\nThis delivers very good quality but has a significant impact on performance.\n200% means twice the resolution to be handled while rendering"s,
   0.5f, 2.f, 1.f);
PropEnum(Player, MSAASamples, "MSAA level"s,
   "Set the amount of MSAA samples.\nMSAA can help reduce geometry aliasing at the cost of performance and GPU memory.\nThis can improve image quality if not using supersampling"s, int,
   0, "Disabled"s, "4 Samples"s, "6 Samples"s, "8 Samples"s);
PropEnum(Player, FXAA, "Post processed antialiasing"s, "Select between different antialiasing techniques that offer different quality vs performance balances"s, int, 0, "Disabled"s,
   "Fast FXAA"s, "Standard FXAA"s, "Quality FXAA"s, "Fast NFAA"s, "Standard DLAA"s, "Quality SMAA"s, "Quality FAAA"s);
PropEnum(Player, Sharpen, "Post processed sharpening"s, "Select between different sharpening techniques that offer different quality vs performance balances"s, int, 0, "Disabled"s, "CAS"s,
   "Bilateral CAS"s);

// Ball rendering
PropBool(Player, BallAntiStretch, "Unstretch Ball"s, "Compensate ball stretching"s, false);
PropBool(Player, DisableLightingForBalls, "Disable Ball Lighting"s, "Disable lighting and reflection effects on balls, e.g. to help the visually handicapped"s, false);
PropBool(Player, BallTrail, "Ball Trail"s, "Legacy Ball Trails"s, false);
PropFloat(Player, BallTrailStrength, "Ball Trail Strength"s, "Strength of the fake Ball Trail"s, 0.f, 5.f, 0.5f);
PropBool(Player, OverwriteBallImage, "Overwrite ball image"s, "Allow to define images that will be used instead of the table's provided one"s, false);
PropString(Player, BallImage, "Ball image override"s, "Image to use for the ball instead of the table's provide one"s, ""s);
PropString(Player, DecalImage, "Decal image override"s, "Image to use for the ball's decal instead of the table's provide one"s, ""s);

// Misc player settings
PropBool(Player, TouchOverlay, "Touch Overlay"s, "Display an overlay showing touch regions"s, false);
PropBool(Player, EnableCameraModeFlyAround, "Legacy Fly Over Mode"s, "Enable moving camera when using the Tweak menu (legacy, replaced by LiveUI fly mode)"s, false);
PropBool(Player, DetectHang, "Detect Script Hang"s, ""s, false);
PropInt(Player, SecurityLevel, "Security Level"s, ""s, 0, 4, DEFAULT_SECURITY_LEVEL);
PropInt(Player, NumberOfTimesToShowTouchMessage, "NumberOfTimesToShowTouchMessage"s, "Number of times to re-display the touch display message"s, 0, 100, 10);
PropBool(Player, BAMHeadTracking, "BAM Headtracking"s, "Enable headtracking using the external BAM application.\nThis feature is experimental and unsupported"s, false);
PropBool(Player, Mirror, "Mirror"s, "Mirror the table (left <-> right)"s, false);
PropEnum(Player, CacheMode, "Cache Mode"s, "Use cache to limit stutters and speedup loading"s, int, 1, "Disabled"s, "Preload Textures"s);
PropEnum(Player, RumbleMode, "RumbleMode"s, "Use rumble motor(s) in attached input devices"s, int, 3, "Off"s, "Table only (N/A yet)"s, "Generic only (N/A yet)"s, "Table with generic fallback"s);
PropInt(Player, MinPhysLoopTime, "MinPhysLoopTime"s, ""s, 0, 1000, 0); // Legacy lag reduction hack (e.g. if script execution or physics takes very long, comes at the price of "slower" gameplay). Not supported by BGFX variant (due to its multithreaded loop)
PropIntUnbounded(Player, PhysicsMaxLoops, "Physics Max Loops"s,
   "Maximum number of physics iteration above which physics engine just skip to stay playable.\nThis is somewhat hacky, override table setup, and may cause gameplay issues. This should not be used anymore."s,
   (int)0xFFFFFFFFu);
PropIntUnbounded(Player, Autostart, "Autostart"s, ""s, 0);
PropIntUnbounded(Player, AutostartRetry, "AutostartRetry"s, ""s, 0);
PropBool(Player, asenable, "AutostartEnable"s, ""s, false);

// UI & input settings
PropInt(Player, Exitconfirm, "Direct Exit Length"s, "Length of a long ESC press that directly closes the app, (sadly) expressed in seconds * 60"s, 0, 30 * 60, 120);
PropString(Input, Devices, "Devices"s, "List of known devices"s, ""s);

// Nudge & Plumb settings
PropFloat(Player, NudgeOrientation0, "Sensor 1 - Orientation"s, "Define sensor orientation"s, 0.f, 360.f, 0.f);
PropFloat(Player, NudgeOrientation1, "Sensor 2 - Orientation"s, "Define sensor orientation"s, 0.f, 360.f, 0.f);
PropBool(Player, NudgeFilter0, "Sensor 1 - Use Filter"s, "Enable/Disable filtering acquired value to prevent noise"s, false);
PropBool(Player, NudgeFilter1, "Sensor 2 - Use Filter"s, "Enable/Disable filtering acquired value to prevent noise"s, false);
PropBool(Player, SimulatedPlumb, "Plumb simulation"s, "Enable/Disable mechanical Tilt plumb simulation"s, true);
PropFloat(Player, PlumbInertia, "Plumb Inertia"s, ""s, 0.001f, 1.f, 0.35f);
PropFloat(Player, PlumbThresholdAngle, "Plumb Threshold"s, "Define threshold angle at which a Tilt is caused"s, 5.0f, 60.f, 35.f);
PropBool(Player, EnableLegacyNudge, "Legacy Keyboard nudge"s, "Enable/Disable legacy keyboard nudge mode"s, false);
PropFloat(Player, LegacyNudgeStrength, "Legacy Nudge Strength"s, "Changes the visual effect/screen shaking when using the legacy keyboard nudging mode"s, 0.f, 90.f, 1.f);
PropFloat(Player, NudgeStrength, "Visual Nudge Strength"s, "Changes the visual effect/screen shaking when nudging the table"s, 0.f, 0.25f, 0.02f);
PropArray(Player, NudgeOrientation, float, Float, Float, m_propPlayer_NudgeOrientation0, m_propPlayer_NudgeOrientation1);
PropArray(Player, NudgeFilter, bool, Bool, Int, m_propPlayer_NudgeFilter0, m_propPlayer_NudgeFilter1);

// Plunger settings
PropBool(Player, PlungerRetract, "One Second Retract"s, "Enable retracting the plunger after a 1 second press when using the digital plunger emulation through keyboard or joystick button"s,
   false);
PropBool(Player, PlungerLinearSensor, "Linear Sensor"s, "Select between symmetric (linear) and assymetric sensor"s, false);
PropInt(Player, PlungerNormalize, "Plunger normalize override"s, "This value may be defined to override the table's plunger normalization"s, 0, 100,
   100); // Hacky: This should be a table override, not a player property as it overrides table data

// VR settings
PropEnum(PlayerVR, AskToTurnOn, "Enable VR"s, "Ask to turn on VR"s, int, 2, "Enabled"s, "Autodetect"s, "Disabled"s);
PropFloat(PlayerVR, Orientation, "View orientation"s, "VR view orientation"s, -180.f, 180.f, 0.f);
PropFloat(PlayerVR, TableX, "View Offset X"s, "VR view X offset"s, -100.f, 100.f, 0.f);
PropFloat(PlayerVR, TableY, "View Offset Y"s, "VR view Y offset"s, -100.f, 100.f, 0.f);
PropFloat(PlayerVR, TableZ, "View Offset Z"s, "VR view Z offset"s, -100.f, 100.f, 0.f);
PropBool(
   PlayerVR, UsePassthroughColor, "Color Keyed Passthrough"s, "Replace VR background by a user defined color, to allow color keyed passthrough (for example using Virtual Desktop)"s, false);
PropInt(PlayerVR, PassthroughColor, "Color Keyed Passthrough color"s, "Color that will replace the background"s, 0x000000, 0xFFFFFF, 0xBB4700);
PropEnum(Player, VRPreview, "Preview mode"s, "Select VR preview mode"s, int, 1, "Disabled"s, "Left Eye"s, "Right Eye"s, "Both Eyes"s);
PropBool(PlayerVR, ShrinkPreview, "Shrink preview"s, "Shrink VR preview"s, false);
PropFloatUnbounded(PlayerVR, ResFactor, "ResFactor"s, ""s, -1.f);
PropBool(Player, CaptureExternalDMD, "Capture External DMD"s, "Capture an external DMD Window and render it into the VR viewport.\nThis feature is deprecated and unsupported."s, false);
PropBool(Player, CapturePUP, "Capture PinUp Player"s, "Capture PinUp Player (PUP) Window and render it into the VR viewport.\nThis feature is deprecated and unsupported."s, false);
// Legacy OpenVR settings (to be removed)
PropEnum(PlayerVR, EyeFBFormat, "EyeFBFormat"s, "VR frame buffer format"s, int, 1, "RGB 8"s, "RGBA 8 (Recommended)"s, "RGB 16F"s, "RGBA 16F"s);
PropFloatUnbounded(PlayerVR, Slope, "Slope"s, "VR view slope"s, 6.5f);
PropBool(PlayerVR, ScaleToFixedWidth, "ScaleToFixedWidth"s, ""s, false);
PropFloatUnbounded(PlayerVR, ScaleAbsolute, "ScaleAbsolute"s, ""s, 55.f);
PropFloatUnbounded(PlayerVR, ScaleRelative, "ScaleRelative"s, ""s, 1.f);
PropFloatUnbounded(PlayerVR, NearPlane, "NearPlane"s, "VR near plane offset"s, 5.f);

// Physics override profiles
PropFloatUnbounded(Player, FlipperPhysicsMass0, "FlipperPhysicsMass0"s, ""s, 1.f);
PropFloatUnbounded(Player, FlipperPhysicsStrength0, "FlipperPhysicsStrength0"s, ""s, 2200.f);
PropFloatUnbounded(Player, FlipperPhysicsElasticity0, "FlipperPhysicsElasticity0"s, ""s, 0.8f);
PropFloatUnbounded(Player, FlipperPhysicsScatter0, "FlipperPhysicsScatter0"s, ""s, 0.f);
PropFloatUnbounded(Player, FlipperPhysicsReturnStrength0, "FlipperPhysicsReturnStrength0"s, ""s, 0.058f);
PropFloatUnbounded(Player, FlipperPhysicsElasticityFalloff0, "FlipperPhysicsElasticityFalloff0"s, ""s, 0.43f);
PropFloatUnbounded(Player, FlipperPhysicsFriction0, "FlipperPhysicsFriction0"s, ""s, 0.6f);
PropFloatUnbounded(Player, FlipperPhysicsCoilRampUp0, "FlipperPhysicsCoilRampUp0"s, ""s, 3.f);
PropFloatUnbounded(Player, FlipperPhysicsEOSTorque0, "FlipperPhysicsEOSTorque0"s, ""s, 0.75f);
PropFloatUnbounded(Player, FlipperPhysicsEOSTorqueAngle0, "FlipperPhysicsEOSTorqueAngle0"s, ""s, 6.f);
PropFloatUnbounded(Player, TablePhysicsGravityConstant0, "TablePhysicsGravityConstant0"s, ""s, DEFAULT_TABLE_GRAVITY);
PropFloatUnbounded(Player, TablePhysicsContactFriction0, "TablePhysicsContactFriction0"s, ""s, DEFAULT_TABLE_CONTACTFRICTION);
PropFloatUnbounded(Player, TablePhysicsElasticity0, "TablePhysicsElasticity0"s, ""s, DEFAULT_TABLE_ELASTICITY);
PropFloatUnbounded(Player, TablePhysicsElasticityFalloff0, "TablePhysicsElasticityFalloff0"s, ""s, DEFAULT_TABLE_ELASTICITY_FALLOFF);
PropFloatUnbounded(Player, TablePhysicsScatterAngle0, "TablePhysicsScatterAngle0"s, ""s, DEFAULT_TABLE_PFSCATTERANGLE);
PropFloatUnbounded(Player, TablePhysicsContactScatterAngle0, "TablePhysicsContactScatterAngle0"s, ""s, DEFAULT_TABLE_SCATTERANGLE);
PropFloatUnbounded(Player, TablePhysicsMinSlope0, "TablePhysicsMinSlope0"s, ""s, DEFAULT_TABLE_MIN_SLOPE);
PropFloatUnbounded(Player, TablePhysicsMaxSlope0, "TablePhysicsMaxSlope0"s, ""s, DEFAULT_TABLE_MAX_SLOPE);
PropString(Player, PhysicsSetName0, "PhysicsSetName0"s, ""s, "Set 1"s);

PropFloatUnbounded(Player, FlipperPhysicsMass1, "FlipperPhysicsMass1"s, ""s, 1.f);
PropFloatUnbounded(Player, FlipperPhysicsStrength1, "FlipperPhysicsStrength1"s, ""s, 2200.f);
PropFloatUnbounded(Player, FlipperPhysicsElasticity1, "FlipperPhysicsElasticity1"s, ""s, 0.8f);
PropFloatUnbounded(Player, FlipperPhysicsScatter1, "FlipperPhysicsScatter1"s, ""s, 0.f);
PropFloatUnbounded(Player, FlipperPhysicsReturnStrength1, "FlipperPhysicsReturnStrength1"s, ""s, 0.058f);
PropFloatUnbounded(Player, FlipperPhysicsElasticityFalloff1, "FlipperPhysicsElasticityFalloff1"s, ""s, 0.43f);
PropFloatUnbounded(Player, FlipperPhysicsFriction1, "FlipperPhysicsFriction1"s, ""s, 0.6f);
PropFloatUnbounded(Player, FlipperPhysicsCoilRampUp1, "FlipperPhysicsCoilRampUp1"s, ""s, 3.f);
PropFloatUnbounded(Player, FlipperPhysicsEOSTorque1, "FlipperPhysicsEOSTorque1"s, ""s, 0.75f);
PropFloatUnbounded(Player, FlipperPhysicsEOSTorqueAngle1, "FlipperPhysicsEOSTorqueAngle1"s, ""s, 6.f);
PropFloatUnbounded(Player, TablePhysicsGravityConstant1, "TablePhysicsGravityConstant1"s, ""s, DEFAULT_TABLE_GRAVITY);
PropFloatUnbounded(Player, TablePhysicsContactFriction1, "TablePhysicsContactFriction1"s, ""s, DEFAULT_TABLE_CONTACTFRICTION);
PropFloatUnbounded(Player, TablePhysicsElasticity1, "TablePhysicsElasticity1"s, ""s, DEFAULT_TABLE_ELASTICITY);
PropFloatUnbounded(Player, TablePhysicsElasticityFalloff1, "TablePhysicsElasticityFalloff1"s, ""s, DEFAULT_TABLE_ELASTICITY_FALLOFF);
PropFloatUnbounded(Player, TablePhysicsScatterAngle1, "TablePhysicsScatterAngle1"s, ""s, DEFAULT_TABLE_PFSCATTERANGLE);
PropFloatUnbounded(Player, TablePhysicsContactScatterAngle1, "TablePhysicsContactScatterAngle1"s, ""s, DEFAULT_TABLE_SCATTERANGLE);
PropFloatUnbounded(Player, TablePhysicsMinSlope1, "TablePhysicsMinSlope1"s, ""s, DEFAULT_TABLE_MIN_SLOPE);
PropFloatUnbounded(Player, TablePhysicsMaxSlope1, "TablePhysicsMaxSlope1"s, ""s, DEFAULT_TABLE_MAX_SLOPE);
PropString(Player, PhysicsSetName1, "PhysicsSetName1"s, ""s, "Set 2"s);

PropFloatUnbounded(Player, FlipperPhysicsMass2, "FlipperPhysicsMass2"s, ""s, 1.f);
PropFloatUnbounded(Player, FlipperPhysicsStrength2, "FlipperPhysicsStrength2"s, ""s, 2200.f);
PropFloatUnbounded(Player, FlipperPhysicsElasticity2, "FlipperPhysicsElasticity2"s, ""s, 0.8f);
PropFloatUnbounded(Player, FlipperPhysicsScatter2, "FlipperPhysicsScatter2"s, ""s, 0.f);
PropFloatUnbounded(Player, FlipperPhysicsReturnStrength2, "FlipperPhysicsReturnStrength2"s, ""s, 0.058f);
PropFloatUnbounded(Player, FlipperPhysicsElasticityFalloff2, "FlipperPhysicsElasticityFalloff2"s, ""s, 0.43f);
PropFloatUnbounded(Player, FlipperPhysicsFriction2, "FlipperPhysicsFriction2"s, ""s, 0.6f);
PropFloatUnbounded(Player, FlipperPhysicsCoilRampUp2, "FlipperPhysicsCoilRampUp2"s, ""s, 3.f);
PropFloatUnbounded(Player, FlipperPhysicsEOSTorque2, "FlipperPhysicsEOSTorque2"s, ""s, 0.75f);
PropFloatUnbounded(Player, FlipperPhysicsEOSTorqueAngle2, "FlipperPhysicsEOSTorqueAngle2"s, ""s, 6.f);
PropFloatUnbounded(Player, TablePhysicsGravityConstant2, "TablePhysicsGravityConstant2"s, ""s, DEFAULT_TABLE_GRAVITY);
PropFloatUnbounded(Player, TablePhysicsContactFriction2, "TablePhysicsContactFriction2"s, ""s, DEFAULT_TABLE_CONTACTFRICTION);
PropFloatUnbounded(Player, TablePhysicsElasticity2, "TablePhysicsElasticity2"s, ""s, DEFAULT_TABLE_ELASTICITY);
PropFloatUnbounded(Player, TablePhysicsElasticityFalloff2, "TablePhysicsElasticityFalloff2"s, ""s, DEFAULT_TABLE_ELASTICITY_FALLOFF);
PropFloatUnbounded(Player, TablePhysicsScatterAngle2, "TablePhysicsScatterAngle2"s, ""s, DEFAULT_TABLE_PFSCATTERANGLE);
PropFloatUnbounded(Player, TablePhysicsContactScatterAngle2, "TablePhysicsContactScatterAngle2"s, ""s, DEFAULT_TABLE_SCATTERANGLE);
PropFloatUnbounded(Player, TablePhysicsMinSlope2, "TablePhysicsMinSlope2"s, ""s, DEFAULT_TABLE_MIN_SLOPE);
PropFloatUnbounded(Player, TablePhysicsMaxSlope2, "TablePhysicsMaxSlope2"s, ""s, DEFAULT_TABLE_MAX_SLOPE);
PropString(Player, PhysicsSetName2, "PhysicsSetName2"s, ""s, "Set 3"s);

PropFloatUnbounded(Player, FlipperPhysicsMass3, "FlipperPhysicsMass3"s, ""s, 1.f);
PropFloatUnbounded(Player, FlipperPhysicsStrength3, "FlipperPhysicsStrength3"s, ""s, 2200.f);
PropFloatUnbounded(Player, FlipperPhysicsElasticity3, "FlipperPhysicsElasticity3"s, ""s, 0.8f);
PropFloatUnbounded(Player, FlipperPhysicsScatter3, "FlipperPhysicsScatter3"s, ""s, 0.f);
PropFloatUnbounded(Player, FlipperPhysicsReturnStrength3, "FlipperPhysicsReturnStrength3"s, ""s, 0.058f);
PropFloatUnbounded(Player, FlipperPhysicsElasticityFalloff3, "FlipperPhysicsElasticityFalloff3"s, ""s, 0.43f);
PropFloatUnbounded(Player, FlipperPhysicsFriction3, "FlipperPhysicsFriction3"s, ""s, 0.6f);
PropFloatUnbounded(Player, FlipperPhysicsCoilRampUp3, "FlipperPhysicsCoilRampUp3"s, ""s, 3.f);
PropFloatUnbounded(Player, FlipperPhysicsEOSTorque3, "FlipperPhysicsEOSTorque3"s, ""s, 0.75f);
PropFloatUnbounded(Player, FlipperPhysicsEOSTorqueAngle3, "FlipperPhysicsEOSTorqueAngle3"s, ""s, 6.f);
PropFloatUnbounded(Player, TablePhysicsGravityConstant3, "TablePhysicsGravityConstant3"s, ""s, DEFAULT_TABLE_GRAVITY);
PropFloatUnbounded(Player, TablePhysicsContactFriction3, "TablePhysicsContactFriction3"s, ""s, DEFAULT_TABLE_CONTACTFRICTION);
PropFloatUnbounded(Player, TablePhysicsElasticity3, "TablePhysicsElasticity3"s, ""s, DEFAULT_TABLE_ELASTICITY);
PropFloatUnbounded(Player, TablePhysicsElasticityFalloff3, "TablePhysicsElasticityFalloff3"s, ""s, DEFAULT_TABLE_ELASTICITY_FALLOFF);
PropFloatUnbounded(Player, TablePhysicsScatterAngle3, "TablePhysicsScatterAngle3"s, ""s, DEFAULT_TABLE_PFSCATTERANGLE);
PropFloatUnbounded(Player, TablePhysicsContactScatterAngle3, "TablePhysicsContactScatterAngle3"s, ""s, DEFAULT_TABLE_SCATTERANGLE);
PropFloatUnbounded(Player, TablePhysicsMinSlope3, "TablePhysicsMinSlope3"s, ""s, DEFAULT_TABLE_MIN_SLOPE);
PropFloatUnbounded(Player, TablePhysicsMaxSlope3, "TablePhysicsMaxSlope3"s, ""s, DEFAULT_TABLE_MAX_SLOPE);
PropString(Player, PhysicsSetName3, "PhysicsSetName3"s, ""s, "Set 4"s);

PropFloatUnbounded(Player, FlipperPhysicsMass4, "FlipperPhysicsMass4"s, ""s, 1.f);
PropFloatUnbounded(Player, FlipperPhysicsStrength4, "FlipperPhysicsStrength4"s, ""s, 2200.f);
PropFloatUnbounded(Player, FlipperPhysicsElasticity4, "FlipperPhysicsElasticity4"s, ""s, 0.8f);
PropFloatUnbounded(Player, FlipperPhysicsScatter4, "FlipperPhysicsScatter4"s, ""s, 0.f);
PropFloatUnbounded(Player, FlipperPhysicsReturnStrength4, "FlipperPhysicsReturnStrength4"s, ""s, 0.058f);
PropFloatUnbounded(Player, FlipperPhysicsElasticityFalloff4, "FlipperPhysicsElasticityFalloff4"s, ""s, 0.43f);
PropFloatUnbounded(Player, FlipperPhysicsFriction4, "FlipperPhysicsFriction4"s, ""s, 0.6f);
PropFloatUnbounded(Player, FlipperPhysicsCoilRampUp4, "FlipperPhysicsCoilRampUp4"s, ""s, 3.f);
PropFloatUnbounded(Player, FlipperPhysicsEOSTorque4, "FlipperPhysicsEOSTorque4"s, ""s, 0.75f);
PropFloatUnbounded(Player, FlipperPhysicsEOSTorqueAngle4, "FlipperPhysicsEOSTorqueAngle4"s, ""s, 6.f);
PropFloatUnbounded(Player, TablePhysicsGravityConstant4, "TablePhysicsGravityConstant4"s, ""s, DEFAULT_TABLE_GRAVITY);
PropFloatUnbounded(Player, TablePhysicsContactFriction4, "TablePhysicsContactFriction4"s, ""s, DEFAULT_TABLE_CONTACTFRICTION);
PropFloatUnbounded(Player, TablePhysicsElasticity4, "TablePhysicsElasticity4"s, ""s, DEFAULT_TABLE_ELASTICITY);
PropFloatUnbounded(Player, TablePhysicsElasticityFalloff4, "TablePhysicsElasticityFalloff4"s, ""s, DEFAULT_TABLE_ELASTICITY_FALLOFF);
PropFloatUnbounded(Player, TablePhysicsScatterAngle4, "TablePhysicsScatterAngle4"s, ""s, DEFAULT_TABLE_PFSCATTERANGLE);
PropFloatUnbounded(Player, TablePhysicsContactScatterAngle4, "TablePhysicsContactScatterAngle4"s, ""s, DEFAULT_TABLE_SCATTERANGLE);
PropFloatUnbounded(Player, TablePhysicsMinSlope4, "TablePhysicsMinSlope4"s, ""s, DEFAULT_TABLE_MIN_SLOPE);
PropFloatUnbounded(Player, TablePhysicsMaxSlope4, "TablePhysicsMaxSlope4"s, ""s, DEFAULT_TABLE_MAX_SLOPE);
PropString(Player, PhysicsSetName4, "PhysicsSetName4"s, ""s, "Set 5"s);

PropFloatUnbounded(Player, FlipperPhysicsMass5, "FlipperPhysicsMass5"s, ""s, 1.f);
PropFloatUnbounded(Player, FlipperPhysicsStrength5, "FlipperPhysicsStrength5"s, ""s, 2200.f);
PropFloatUnbounded(Player, FlipperPhysicsElasticity5, "FlipperPhysicsElasticity5"s, ""s, 0.8f);
PropFloatUnbounded(Player, FlipperPhysicsScatter5, "FlipperPhysicsScatter5"s, ""s, 0.f);
PropFloatUnbounded(Player, FlipperPhysicsReturnStrength5, "FlipperPhysicsReturnStrength5"s, ""s, 0.058f);
PropFloatUnbounded(Player, FlipperPhysicsElasticityFalloff5, "FlipperPhysicsElasticityFalloff5"s, ""s, 0.43f);
PropFloatUnbounded(Player, FlipperPhysicsFriction5, "FlipperPhysicsFriction5"s, ""s, 0.6f);
PropFloatUnbounded(Player, FlipperPhysicsCoilRampUp5, "FlipperPhysicsCoilRampUp5"s, ""s, 3.f);
PropFloatUnbounded(Player, FlipperPhysicsEOSTorque5, "FlipperPhysicsEOSTorque5"s, ""s, 0.75f);
PropFloatUnbounded(Player, FlipperPhysicsEOSTorqueAngle5, "FlipperPhysicsEOSTorqueAngle5"s, ""s, 6.f);
PropFloatUnbounded(Player, TablePhysicsGravityConstant5, "TablePhysicsGravityConstant5"s, ""s, DEFAULT_TABLE_GRAVITY);
PropFloatUnbounded(Player, TablePhysicsContactFriction5, "TablePhysicsContactFriction5"s, ""s, DEFAULT_TABLE_CONTACTFRICTION);
PropFloatUnbounded(Player, TablePhysicsElasticity5, "TablePhysicsElasticity5"s, ""s, DEFAULT_TABLE_ELASTICITY);
PropFloatUnbounded(Player, TablePhysicsElasticityFalloff5, "TablePhysicsElasticityFalloff5"s, ""s, DEFAULT_TABLE_ELASTICITY_FALLOFF);
PropFloatUnbounded(Player, TablePhysicsScatterAngle5, "TablePhysicsScatterAngle5"s, ""s, DEFAULT_TABLE_PFSCATTERANGLE);
PropFloatUnbounded(Player, TablePhysicsContactScatterAngle5, "TablePhysicsContactScatterAngle5"s, ""s, DEFAULT_TABLE_SCATTERANGLE);
PropFloatUnbounded(Player, TablePhysicsMinSlope5, "TablePhysicsMinSlope5"s, ""s, DEFAULT_TABLE_MIN_SLOPE);
PropFloatUnbounded(Player, TablePhysicsMaxSlope5, "TablePhysicsMaxSlope5"s, ""s, DEFAULT_TABLE_MAX_SLOPE);
PropString(Player, PhysicsSetName5, "PhysicsSetName5"s, ""s, "Set 6"s);

PropFloatUnbounded(Player, FlipperPhysicsMass6, "FlipperPhysicsMass6"s, ""s, 1.f);
PropFloatUnbounded(Player, FlipperPhysicsStrength6, "FlipperPhysicsStrength6"s, ""s, 2200.f);
PropFloatUnbounded(Player, FlipperPhysicsElasticity6, "FlipperPhysicsElasticity6"s, ""s, 0.8f);
PropFloatUnbounded(Player, FlipperPhysicsScatter6, "FlipperPhysicsScatter6"s, ""s, 0.f);
PropFloatUnbounded(Player, FlipperPhysicsReturnStrength6, "FlipperPhysicsReturnStrength6"s, ""s, 0.058f);
PropFloatUnbounded(Player, FlipperPhysicsElasticityFalloff6, "FlipperPhysicsElasticityFalloff6"s, ""s, 0.43f);
PropFloatUnbounded(Player, FlipperPhysicsFriction6, "FlipperPhysicsFriction6"s, ""s, 0.6f);
PropFloatUnbounded(Player, FlipperPhysicsCoilRampUp6, "FlipperPhysicsCoilRampUp6"s, ""s, 3.f);
PropFloatUnbounded(Player, FlipperPhysicsEOSTorque6, "FlipperPhysicsEOSTorque6"s, ""s, 0.75f);
PropFloatUnbounded(Player, FlipperPhysicsEOSTorqueAngle6, "FlipperPhysicsEOSTorqueAngle6"s, ""s, 6.f);
PropFloatUnbounded(Player, TablePhysicsGravityConstant6, "TablePhysicsGravityConstant6"s, ""s, DEFAULT_TABLE_GRAVITY);
PropFloatUnbounded(Player, TablePhysicsContactFriction6, "TablePhysicsContactFriction6"s, ""s, DEFAULT_TABLE_CONTACTFRICTION);
PropFloatUnbounded(Player, TablePhysicsElasticity6, "TablePhysicsElasticity6"s, ""s, DEFAULT_TABLE_ELASTICITY);
PropFloatUnbounded(Player, TablePhysicsElasticityFalloff6, "TablePhysicsElasticityFalloff6"s, ""s, DEFAULT_TABLE_ELASTICITY_FALLOFF);
PropFloatUnbounded(Player, TablePhysicsScatterAngle6, "TablePhysicsScatterAngle6"s, ""s, DEFAULT_TABLE_PFSCATTERANGLE);
PropFloatUnbounded(Player, TablePhysicsContactScatterAngle6, "TablePhysicsContactScatterAngle6"s, ""s, DEFAULT_TABLE_SCATTERANGLE);
PropFloatUnbounded(Player, TablePhysicsMinSlope6, "TablePhysicsMinSlope6"s, ""s, DEFAULT_TABLE_MIN_SLOPE);
PropFloatUnbounded(Player, TablePhysicsMaxSlope6, "TablePhysicsMaxSlope6"s, ""s, DEFAULT_TABLE_MAX_SLOPE);
PropString(Player, PhysicsSetName6, "PhysicsSetName6"s, ""s, "Set 7"s);

PropFloatUnbounded(Player, FlipperPhysicsMass7, "FlipperPhysicsMass7"s, ""s, 1.f);
PropFloatUnbounded(Player, FlipperPhysicsStrength7, "FlipperPhysicsStrength7"s, ""s, 2200.f);
PropFloatUnbounded(Player, FlipperPhysicsElasticity7, "FlipperPhysicsElasticity7"s, ""s, 0.8f);
PropFloatUnbounded(Player, FlipperPhysicsScatter7, "FlipperPhysicsScatter7"s, ""s, 0.f);
PropFloatUnbounded(Player, FlipperPhysicsReturnStrength7, "FlipperPhysicsReturnStrength7"s, ""s, 0.058f);
PropFloatUnbounded(Player, FlipperPhysicsElasticityFalloff7, "FlipperPhysicsElasticityFalloff7"s, ""s, 0.43f);
PropFloatUnbounded(Player, FlipperPhysicsFriction7, "FlipperPhysicsFriction7"s, ""s, 0.6f);
PropFloatUnbounded(Player, FlipperPhysicsCoilRampUp7, "FlipperPhysicsCoilRampUp7"s, ""s, 3.f);
PropFloatUnbounded(Player, FlipperPhysicsEOSTorque7, "FlipperPhysicsEOSTorque7"s, ""s, 0.75f);
PropFloatUnbounded(Player, FlipperPhysicsEOSTorqueAngle7, "FlipperPhysicsEOSTorqueAngle7"s, ""s, 6.f);
PropFloatUnbounded(Player, TablePhysicsGravityConstant7, "TablePhysicsGravityConstant7"s, ""s, DEFAULT_TABLE_GRAVITY);
PropFloatUnbounded(Player, TablePhysicsContactFriction7, "TablePhysicsContactFriction7"s, ""s, DEFAULT_TABLE_CONTACTFRICTION);
PropFloatUnbounded(Player, TablePhysicsElasticity7, "TablePhysicsElasticity7"s, ""s, DEFAULT_TABLE_ELASTICITY);
PropFloatUnbounded(Player, TablePhysicsElasticityFalloff7, "TablePhysicsElasticityFalloff7"s, ""s, DEFAULT_TABLE_ELASTICITY_FALLOFF);
PropFloatUnbounded(Player, TablePhysicsScatterAngle7, "TablePhysicsScatterAngle7"s, ""s, DEFAULT_TABLE_PFSCATTERANGLE);
PropFloatUnbounded(Player, TablePhysicsContactScatterAngle7, "TablePhysicsContactScatterAngle7"s, ""s, DEFAULT_TABLE_SCATTERANGLE);
PropFloatUnbounded(Player, TablePhysicsMinSlope7, "TablePhysicsMinSlope7"s, ""s, DEFAULT_TABLE_MIN_SLOPE);
PropFloatUnbounded(Player, TablePhysicsMaxSlope7, "TablePhysicsMaxSlope7"s, ""s, DEFAULT_TABLE_MAX_SLOPE);
PropString(Player, PhysicsSetName7, "PhysicsSetName7"s, ""s, "Set 8"s);

PropArray(Player, FlipperPhysicsMass, float, Float, Float, m_propPlayer_FlipperPhysicsMass0, m_propPlayer_FlipperPhysicsMass1, m_propPlayer_FlipperPhysicsMass2,
   m_propPlayer_FlipperPhysicsMass3, m_propPlayer_FlipperPhysicsMass4, m_propPlayer_FlipperPhysicsMass5, m_propPlayer_FlipperPhysicsMass6, m_propPlayer_FlipperPhysicsMass7);
PropArray(Player, FlipperPhysicsStrength, float, Float, Float, m_propPlayer_FlipperPhysicsStrength0, m_propPlayer_FlipperPhysicsStrength1, m_propPlayer_FlipperPhysicsStrength2,
   m_propPlayer_FlipperPhysicsStrength3, m_propPlayer_FlipperPhysicsStrength4, m_propPlayer_FlipperPhysicsStrength5, m_propPlayer_FlipperPhysicsStrength6,
   m_propPlayer_FlipperPhysicsStrength7);
PropArray(Player, FlipperPhysicsElasticity, float, Float, Float, m_propPlayer_FlipperPhysicsElasticity0, m_propPlayer_FlipperPhysicsElasticity1, m_propPlayer_FlipperPhysicsElasticity2,
   m_propPlayer_FlipperPhysicsElasticity3, m_propPlayer_FlipperPhysicsElasticity4, m_propPlayer_FlipperPhysicsElasticity5, m_propPlayer_FlipperPhysicsElasticity6,
   m_propPlayer_FlipperPhysicsElasticity7);
PropArray(Player, FlipperPhysicsScatter, float, Float, Float, m_propPlayer_FlipperPhysicsScatter0, m_propPlayer_FlipperPhysicsScatter1, m_propPlayer_FlipperPhysicsScatter2,
   m_propPlayer_FlipperPhysicsScatter3, m_propPlayer_FlipperPhysicsScatter4, m_propPlayer_FlipperPhysicsScatter5, m_propPlayer_FlipperPhysicsScatter6, m_propPlayer_FlipperPhysicsScatter7);
PropArray(Player, FlipperPhysicsReturnStrength, float, Float, Float, m_propPlayer_FlipperPhysicsReturnStrength0, m_propPlayer_FlipperPhysicsReturnStrength1,
   m_propPlayer_FlipperPhysicsReturnStrength2, m_propPlayer_FlipperPhysicsReturnStrength3, m_propPlayer_FlipperPhysicsReturnStrength4, m_propPlayer_FlipperPhysicsReturnStrength5,
   m_propPlayer_FlipperPhysicsReturnStrength6, m_propPlayer_FlipperPhysicsReturnStrength7);
PropArray(Player, FlipperPhysicsElasticityFalloff, float, Float, Float, m_propPlayer_FlipperPhysicsElasticityFalloff0, m_propPlayer_FlipperPhysicsElasticityFalloff1,
   m_propPlayer_FlipperPhysicsElasticityFalloff2, m_propPlayer_FlipperPhysicsElasticityFalloff3, m_propPlayer_FlipperPhysicsElasticityFalloff4, m_propPlayer_FlipperPhysicsElasticityFalloff5,
   m_propPlayer_FlipperPhysicsElasticityFalloff6, m_propPlayer_FlipperPhysicsElasticityFalloff7);
PropArray(Player, FlipperPhysicsFriction, float, Float, Float, m_propPlayer_FlipperPhysicsFriction0, m_propPlayer_FlipperPhysicsFriction1, m_propPlayer_FlipperPhysicsFriction2,
   m_propPlayer_FlipperPhysicsFriction3, m_propPlayer_FlipperPhysicsFriction4, m_propPlayer_FlipperPhysicsFriction5, m_propPlayer_FlipperPhysicsFriction6,
   m_propPlayer_FlipperPhysicsFriction7);
PropArray(Player, FlipperPhysicsCoilRampUp, float, Float, Float, m_propPlayer_FlipperPhysicsCoilRampUp0, m_propPlayer_FlipperPhysicsCoilRampUp1, m_propPlayer_FlipperPhysicsCoilRampUp2,
   m_propPlayer_FlipperPhysicsCoilRampUp3, m_propPlayer_FlipperPhysicsCoilRampUp4, m_propPlayer_FlipperPhysicsCoilRampUp5, m_propPlayer_FlipperPhysicsCoilRampUp6,
   m_propPlayer_FlipperPhysicsCoilRampUp7);
PropArray(Player, FlipperPhysicsEOSTorque, float, Float, Float, m_propPlayer_FlipperPhysicsEOSTorque0, m_propPlayer_FlipperPhysicsEOSTorque1, m_propPlayer_FlipperPhysicsEOSTorque2,
   m_propPlayer_FlipperPhysicsEOSTorque3, m_propPlayer_FlipperPhysicsEOSTorque4, m_propPlayer_FlipperPhysicsEOSTorque5, m_propPlayer_FlipperPhysicsEOSTorque6,
   m_propPlayer_FlipperPhysicsEOSTorque7);
PropArray(Player, FlipperPhysicsEOSTorqueAngle, float, Float, Float, m_propPlayer_FlipperPhysicsEOSTorqueAngle0, m_propPlayer_FlipperPhysicsEOSTorqueAngle1,
   m_propPlayer_FlipperPhysicsEOSTorqueAngle2, m_propPlayer_FlipperPhysicsEOSTorqueAngle3, m_propPlayer_FlipperPhysicsEOSTorqueAngle4, m_propPlayer_FlipperPhysicsEOSTorqueAngle5,
   m_propPlayer_FlipperPhysicsEOSTorqueAngle6, m_propPlayer_FlipperPhysicsEOSTorqueAngle7);

PropArray(Player, TablePhysicsGravityConstant, float, Float, Float, m_propPlayer_TablePhysicsGravityConstant0, m_propPlayer_TablePhysicsGravityConstant1,
   m_propPlayer_TablePhysicsGravityConstant2, m_propPlayer_TablePhysicsGravityConstant3, m_propPlayer_TablePhysicsGravityConstant4, m_propPlayer_TablePhysicsGravityConstant5,
   m_propPlayer_TablePhysicsGravityConstant6, m_propPlayer_TablePhysicsGravityConstant7);
PropArray(Player, TablePhysicsContactFriction, float, Float, Float, m_propPlayer_TablePhysicsContactFriction0, m_propPlayer_TablePhysicsContactFriction1,
   m_propPlayer_TablePhysicsContactFriction2, m_propPlayer_TablePhysicsContactFriction3, m_propPlayer_TablePhysicsContactFriction4, m_propPlayer_TablePhysicsContactFriction5,
   m_propPlayer_TablePhysicsContactFriction6, m_propPlayer_TablePhysicsContactFriction7);
PropArray(Player, TablePhysicsElasticity, float, Float, Float, m_propPlayer_TablePhysicsElasticity0, m_propPlayer_TablePhysicsElasticity1, m_propPlayer_TablePhysicsElasticity2,
   m_propPlayer_TablePhysicsElasticity3, m_propPlayer_TablePhysicsElasticity4, m_propPlayer_TablePhysicsElasticity5, m_propPlayer_TablePhysicsElasticity6,
   m_propPlayer_TablePhysicsElasticity7);
PropArray(Player, TablePhysicsElasticityFalloff, float, Float, Float, m_propPlayer_TablePhysicsElasticityFalloff0, m_propPlayer_TablePhysicsElasticityFalloff1,
   m_propPlayer_TablePhysicsElasticityFalloff2, m_propPlayer_TablePhysicsElasticityFalloff3, m_propPlayer_TablePhysicsElasticityFalloff4, m_propPlayer_TablePhysicsElasticityFalloff5,
   m_propPlayer_TablePhysicsElasticityFalloff6, m_propPlayer_TablePhysicsElasticityFalloff7);
PropArray(Player, TablePhysicsScatterAngle, float, Float, Float, m_propPlayer_TablePhysicsScatterAngle0, m_propPlayer_TablePhysicsScatterAngle1, m_propPlayer_TablePhysicsScatterAngle2,
   m_propPlayer_TablePhysicsScatterAngle3, m_propPlayer_TablePhysicsScatterAngle4, m_propPlayer_TablePhysicsScatterAngle5, m_propPlayer_TablePhysicsScatterAngle6,
   m_propPlayer_TablePhysicsScatterAngle7);
PropArray(Player, TablePhysicsContactScatterAngle, float, Float, Float, m_propPlayer_TablePhysicsContactScatterAngle0, m_propPlayer_TablePhysicsContactScatterAngle1,
   m_propPlayer_TablePhysicsContactScatterAngle2, m_propPlayer_TablePhysicsContactScatterAngle3, m_propPlayer_TablePhysicsContactScatterAngle4, m_propPlayer_TablePhysicsContactScatterAngle5,
   m_propPlayer_TablePhysicsContactScatterAngle6, m_propPlayer_TablePhysicsContactScatterAngle7);
PropArray(Player, TablePhysicsMinSlope, float, Float, Float, m_propPlayer_TablePhysicsMinSlope0, m_propPlayer_TablePhysicsMinSlope1, m_propPlayer_TablePhysicsMinSlope2,
   m_propPlayer_TablePhysicsMinSlope3, m_propPlayer_TablePhysicsMinSlope4, m_propPlayer_TablePhysicsMinSlope5, m_propPlayer_TablePhysicsMinSlope6, m_propPlayer_TablePhysicsMinSlope7);
PropArray(Player, TablePhysicsMaxSlope, float, Float, Float, m_propPlayer_TablePhysicsMaxSlope0, m_propPlayer_TablePhysicsMaxSlope1, m_propPlayer_TablePhysicsMaxSlope2,
   m_propPlayer_TablePhysicsMaxSlope3, m_propPlayer_TablePhysicsMaxSlope4, m_propPlayer_TablePhysicsMaxSlope5, m_propPlayer_TablePhysicsMaxSlope6, m_propPlayer_TablePhysicsMaxSlope7);
PropArray(Player, PhysicsSetName, string, String, String, m_propPlayer_PhysicsSetName0, m_propPlayer_PhysicsSetName1, m_propPlayer_PhysicsSetName2, m_propPlayer_PhysicsSetName3,
   m_propPlayer_PhysicsSetName4, m_propPlayer_PhysicsSetName5, m_propPlayer_PhysicsSetName6, m_propPlayer_PhysicsSetName7);

// Stereo settings
PropBool(Player, Stereo3DEnabled, "Enable Stereo Rendering"s, "Allow to temporarily disable stereo rendering"s, true);
PropEnum(Player, Stereo3D, "Stereo rendering"s, "Stereo rendering mode"s, StereoMode, 0, "Disabled"s, "Top / Bottom"s, "Interlaced (e.g. LG TVs)"s, "Flipped Interlaced (e.g. LG TVs)"s,
   "Side by Side"s, "Anaglyph Red/Cyan"s, "Anaglyph Green/Magenta"s, "Anaglyph Blue/Amber"s, "Anaglyph Cyan/Red"s, "Anaglyph Magenta/Green"s, "Anaglyph Amber/Blue"s, "Anaglyph Custom 1"s,
   "Anaglyph Custom 2"s, "Anaglyph Custom 3"s, "Anaglyph Custom 4"s);
PropFloat(Player, Stereo3DEyeSeparation, "Eye distance"s, "Physical distance (mm) between eyes"s, 5.f, 200.f, 63.f);
PropFloat(Player, Stereo3DBrightness, "Stereo Brightness"s, "Brightness adjustment applied to stereo rendering"s, 0.f, 2.f, 1.f);
PropFloat(Player, Stereo3DSaturation, "Stereo Saturation"s, "Saturation adjustment applied to stereo rendering"s, 0.f, 2.f, 1.f);
PropFloat(Player, Stereo3DDefocus, "Anaglyph Defocus"s, "Defocusing of the lesser eye to anaglyph stereo rendering"s, 0.f, 1.f, 0.f);
PropFloat(Player, Stereo3DLeftContrast, "Anaglyph Left Contrast"s, "Left eye contrast adjustment applied to anaglyph stereo rendering"s, 0.f, 2.f, 1.f);
PropFloat(Player, Stereo3DRightContrast, "Anaglyph Right Contrast"s, "Right eye contrast adjustment applied to anaglyph stereo rendering"s, 0.f, 2.f, 1.f);
PropEnum(Player, Anaglyph1Filter, "Anaglyph Filter"s, "Anaglyph filter"s, int, 2, "None"s, "Dubois"s, "Luminance"s, "Deghost"s);
PropEnum(Player, Anaglyph2Filter, "Anaglyph Filter"s, "Anaglyph filter"s, int, 2, "None"s, "Dubois"s, "Luminance"s, "Deghost"s);
PropEnum(Player, Anaglyph3Filter, "Anaglyph Filter"s, "Anaglyph filter"s, int, 2, "None"s, "Dubois"s, "Luminance"s, "Deghost"s);
PropEnum(Player, Anaglyph4Filter, "Anaglyph Filter"s, "Anaglyph filter"s, int, 2, "None"s, "Dubois"s, "Luminance"s, "Deghost"s);
PropEnum(Player, Anaglyph5Filter, "Anaglyph Filter"s, "Anaglyph filter"s, int, 2, "None"s, "Dubois"s, "Luminance"s, "Deghost"s);
PropEnum(Player, Anaglyph6Filter, "Anaglyph Filter"s, "Anaglyph filter"s, int, 2, "None"s, "Dubois"s, "Luminance"s, "Deghost"s);
PropEnum(Player, Anaglyph7Filter, "Anaglyph Filter"s, "Anaglyph filter"s, int, 2, "None"s, "Dubois"s, "Luminance"s, "Deghost"s);
PropEnum(Player, Anaglyph8Filter, "Anaglyph Filter"s, "Anaglyph filter"s, int, 2, "None"s, "Dubois"s, "Luminance"s, "Deghost"s);
PropEnum(Player, Anaglyph9Filter, "Anaglyph Filter"s, "Anaglyph filter"s, int, 2, "None"s, "Dubois"s, "Luminance"s, "Deghost"s);
PropEnum(Player, Anaglyph10Filter, "Anaglyph Filter"s, "Anaglyph filter applied to anaglyph profile #10"s, int, 0, "None"s, "Dubois"s, "Luminance"s, "Deghost"s);
PropArray(Player, AnaglyphFilter, int, Int, Int, m_propPlayer_Anaglyph1Filter, m_propPlayer_Anaglyph2Filter, m_propPlayer_Anaglyph3Filter, m_propPlayer_Anaglyph4Filter,
   m_propPlayer_Anaglyph5Filter, m_propPlayer_Anaglyph6Filter, m_propPlayer_Anaglyph7Filter, m_propPlayer_Anaglyph8Filter, m_propPlayer_Anaglyph9Filter, m_propPlayer_Anaglyph10Filter);
PropFloat(Player, Anaglyph1DynDesat, "Anaglyph Dyn. Desat"s, "Dynamic desaturation"s, 0.f, 1.f, 1.f);
PropFloat(Player, Anaglyph2DynDesat, "Anaglyph Dyn. Desat"s, "Dynamic desaturation"s, 0.f, 1.f, 1.f);
PropFloat(Player, Anaglyph3DynDesat, "Anaglyph Dyn. Desat"s, "Dynamic desaturation"s, 0.f, 1.f, 1.f);
PropFloat(Player, Anaglyph4DynDesat, "Anaglyph Dyn. Desat"s, "Dynamic desaturation"s, 0.f, 1.f, 1.f);
PropFloat(Player, Anaglyph5DynDesat, "Anaglyph Dyn. Desat"s, "Dynamic desaturation"s, 0.f, 1.f, 1.f);
PropFloat(Player, Anaglyph6DynDesat, "Anaglyph Dyn. Desat"s, "Dynamic desaturation"s, 0.f, 1.f, 1.f);
PropFloat(Player, Anaglyph7DynDesat, "Anaglyph Dyn. Desat"s, "Dynamic desaturation"s, 0.f, 1.f, 1.f);
PropFloat(Player, Anaglyph8DynDesat, "Anaglyph Dyn. Desat"s, "Dynamic desaturation"s, 0.f, 1.f, 1.f);
PropFloat(Player, Anaglyph9DynDesat, "Anaglyph Dyn. Desat"s, "Dynamic desaturation"s, 0.f, 1.f, 1.f);
PropFloat(Player, Anaglyph10DynDesat, "Anaglyph Dyn. Desat"s, "Dynamic desaturation"s, 0.f, 1.f, 1.f);
PropArray(Player, AnaglyphDynDesat, float, Float, Float, m_propPlayer_Anaglyph1DynDesat, m_propPlayer_Anaglyph2DynDesat, m_propPlayer_Anaglyph3DynDesat, m_propPlayer_Anaglyph4DynDesat,
   m_propPlayer_Anaglyph5DynDesat, m_propPlayer_Anaglyph6DynDesat, m_propPlayer_Anaglyph7DynDesat, m_propPlayer_Anaglyph8DynDesat, m_propPlayer_Anaglyph9DynDesat,
   m_propPlayer_Anaglyph10DynDesat);
PropFloat(Player, Anaglyph1Deghost, "Anaglyph Deghosting"s, "Deghosting level"s, 0.f, 1.f, 0.f);
PropFloat(Player, Anaglyph2Deghost, "Anaglyph Deghosting"s, "Deghosting level"s, 0.f, 1.f, 0.f);
PropFloat(Player, Anaglyph3Deghost, "Anaglyph Deghosting"s, "Deghosting level"s, 0.f, 1.f, 0.f);
PropFloat(Player, Anaglyph4Deghost, "Anaglyph Deghosting"s, "Deghosting level"s, 0.f, 1.f, 0.f);
PropFloat(Player, Anaglyph5Deghost, "Anaglyph Deghosting"s, "Deghosting level"s, 0.f, 1.f, 0.f);
PropFloat(Player, Anaglyph6Deghost, "Anaglyph Deghosting"s, "Deghosting level"s, 0.f, 1.f, 0.f);
PropFloat(Player, Anaglyph7Deghost, "Anaglyph Deghosting"s, "Deghosting level"s, 0.f, 1.f, 0.f);
PropFloat(Player, Anaglyph8Deghost, "Anaglyph Deghosting"s, "Deghosting level"s, 0.f, 1.f, 0.f);
PropFloat(Player, Anaglyph9Deghost, "Anaglyph Deghosting"s, "Deghosting level"s, 0.f, 1.f, 0.f);
PropFloat(Player, Anaglyph10Deghost, "Anaglyph Deghosting"s, "Deghosting level"s, 0.f, 1.f, 0.f);
PropArray(Player, AnaglyphDeghost, float, Float, Float, m_propPlayer_Anaglyph1Deghost, m_propPlayer_Anaglyph2Deghost, m_propPlayer_Anaglyph3Deghost, m_propPlayer_Anaglyph4Deghost,
   m_propPlayer_Anaglyph5Deghost, m_propPlayer_Anaglyph6Deghost, m_propPlayer_Anaglyph7Deghost, m_propPlayer_Anaglyph8Deghost, m_propPlayer_Anaglyph9Deghost, m_propPlayer_Anaglyph10Deghost);
PropBool(Player, Anaglyph1sRGB, "Calibrated sRGB Display"s, "Calibrated sRGB Display"s, true);
PropBool(Player, Anaglyph2sRGB, "Calibrated sRGB Display"s, "Calibrated sRGB Display"s, true);
PropBool(Player, Anaglyph3sRGB, "Calibrated sRGB Display"s, "Calibrated sRGB Display"s, true);
PropBool(Player, Anaglyph4sRGB, "Calibrated sRGB Display"s, "Calibrated sRGB Display"s, true);
PropBool(Player, Anaglyph5sRGB, "Calibrated sRGB Display"s, "Calibrated sRGB Display"s, true);
PropBool(Player, Anaglyph6sRGB, "Calibrated sRGB Display"s, "Calibrated sRGB Display"s, true);
PropBool(Player, Anaglyph7sRGB, "Calibrated sRGB Display"s, "Calibrated sRGB Display"s, true);
PropBool(Player, Anaglyph8sRGB, "Calibrated sRGB Display"s, "Calibrated sRGB Display"s, true);
PropBool(Player, Anaglyph9sRGB, "Calibrated sRGB Display"s, "Calibrated sRGB Display"s, true);
PropBool(Player, Anaglyph10sRGB, "Calibrated sRGB Display"s, "Calibrated sRGB Display"s, true);
PropArray(Player, AnaglyphsRGB, bool, Int, Int, m_propPlayer_Anaglyph1sRGB, m_propPlayer_Anaglyph2sRGB, m_propPlayer_Anaglyph3sRGB, m_propPlayer_Anaglyph4sRGB, m_propPlayer_Anaglyph5sRGB,
   m_propPlayer_Anaglyph6sRGB, m_propPlayer_Anaglyph7sRGB, m_propPlayer_Anaglyph8sRGB, m_propPlayer_Anaglyph9sRGB, m_propPlayer_Anaglyph10sRGB);
// Red Cyan
PropFloat(Player, Anaglyph1LeftRed, "Anaglyph Left Red"s, "Anaglyph Left Eye Red Component"s, 0.f, 1.f, 0.95f);
PropFloat(Player, Anaglyph1LeftGreen, "Anaglyph Left Green"s, "Anaglyph Left Eye Green Component"s, 0.f, 1.f, 0.19f);
PropFloat(Player, Anaglyph1LeftBlue, "Anaglyph Left Blue"s, "Anaglyph Left Eye Blue Component"s, 0.f, 1.f, 0.07f);
PropFloat(Player, Anaglyph1RightRed, "Anaglyph Right Red"s, "Anaglyph Right Eye Red Component"s, 0.f, 1.f, 0.06f);
PropFloat(Player, Anaglyph1RightGreen, "Anaglyph Right Green"s, "Anaglyph Right Eye Green Component"s, 0.f, 1.f, 0.92f);
PropFloat(Player, Anaglyph1RightBlue, "Anaglyph Right Blue"s, "Anaglyph Right Eye Blue Component"s, 0.f, 1.f, 0.28f);
// Green Magenta
PropFloat(Player, Anaglyph2LeftRed, "Anaglyph Left Red"s, "Anaglyph Left Eye Red Component"s, 0.f, 1.f, 0.06f);
PropFloat(Player, Anaglyph2LeftGreen, "Anaglyph Left Green"s, "Anaglyph Left Eye Green Component"s, 0.f, 1.f, 0.96f);
PropFloat(Player, Anaglyph2LeftBlue, "Anaglyph Left Blue"s, "Anaglyph Left Eye Blue Component"s, 0.f, 1.f, 0.09f);
PropFloat(Player, Anaglyph2RightRed, "Anaglyph Right Red"s, "Anaglyph Right Eye Red Component"s, 0.f, 1.f, 0.61f);
PropFloat(Player, Anaglyph2RightGreen, "Anaglyph Right Green"s, "Anaglyph Right Eye Green Component"s, 0.f, 1.f, 0.16f);
PropFloat(Player, Anaglyph2RightBlue, "Anaglyph Right Blue"s, "Anaglyph Right Eye Blue Component"s, 0.f, 1.f, 0.66f);
// Blue Amber
PropFloat(Player, Anaglyph3LeftRed, "Anaglyph Left Red"s, "Anaglyph Left Eye Red Component"s, 0.f, 1.f, 0.05f);
PropFloat(Player, Anaglyph3LeftGreen, "Anaglyph Left Green"s, "Anaglyph Left Eye Green Component"s, 0.f, 1.f, 0.16f);
PropFloat(Player, Anaglyph3LeftBlue, "Anaglyph Left Blue"s, "Anaglyph Left Eye Blue Component"s, 0.f, 1.f, 0.96f);
PropFloat(Player, Anaglyph3RightRed, "Anaglyph Right Red"s, "Anaglyph Right Eye Red Component"s, 0.f, 1.f, 0.61f);
PropFloat(Player, Anaglyph3RightGreen, "Anaglyph Right Green"s, "Anaglyph Right Eye Green Component"s, 0.f, 1.f, 0.66f);
PropFloat(Player, Anaglyph3RightBlue, "Anaglyph Right Blue"s, "Anaglyph Right Eye Blue Component"s, 0.f, 1.f, 0.09f);
// Cyan Red
PropFloat(Player, Anaglyph4LeftRed, "Anaglyph Left Red"s, "Anaglyph Left Eye Red Component"s, 0.f, 1.f, 0.06f);
PropFloat(Player, Anaglyph4LeftGreen, "Anaglyph Left Green"s, "Anaglyph Left Eye Green Component"s, 0.f, 1.f, 0.92f);
PropFloat(Player, Anaglyph4LeftBlue, "Anaglyph Left Blue"s, "Anaglyph Left Eye Blue Component"s, 0.f, 1.f, 0.28f);
PropFloat(Player, Anaglyph4RightRed, "Anaglyph Right Red"s, "Anaglyph Right Eye Red Component"s, 0.f, 1.f, 0.95f);
PropFloat(Player, Anaglyph4RightGreen, "Anaglyph Right Green"s, "Anaglyph Right Eye Green Component"s, 0.f, 1.f, 0.19f);
PropFloat(Player, Anaglyph4RightBlue, "Anaglyph Right Blue"s, "Anaglyph Right Eye Blue Component"s, 0.f, 1.f, 0.07f);
// Magenta Green
PropFloat(Player, Anaglyph5LeftRed, "Anaglyph Left Red"s, "Anaglyph Left Eye Red Component"s, 0.f, 1.f, 0.61f);
PropFloat(Player, Anaglyph5LeftGreen, "Anaglyph Left Green"s, "Anaglyph Left Eye Green Component"s, 0.f, 1.f, 0.16f);
PropFloat(Player, Anaglyph5LeftBlue, "Anaglyph Left Blue"s, "Anaglyph Left Eye Blue Component"s, 0.f, 1.f, 0.66f);
PropFloat(Player, Anaglyph5RightRed, "Anaglyph Right Red"s, "Anaglyph Right Eye Red Component"s, 0.f, 1.f, 0.06f);
PropFloat(Player, Anaglyph5RightGreen, "Anaglyph Right Green"s, "Anaglyph Right Eye Green Component"s, 0.f, 1.f, 0.96f);
PropFloat(Player, Anaglyph5RightBlue, "Anaglyph Right Blue"s, "Anaglyph Right Eye Blue Component"s, 0.f, 1.f, 0.09f);
// Amber Blue
PropFloat(Player, Anaglyph6LeftRed, "Anaglyph Left Red"s, "Anaglyph Left Eye Red Component"s, 0.f, 1.f, 0.61f);
PropFloat(Player, Anaglyph6LeftGreen, "Anaglyph Left Green"s, "Anaglyph Left Eye Green Component"s, 0.f, 1.f, 0.66f);
PropFloat(Player, Anaglyph6LeftBlue, "Anaglyph Left Blue"s, "Anaglyph Left Eye Blue Component"s, 0.f, 1.f, 0.09f);
PropFloat(Player, Anaglyph6RightRed, "Anaglyph Right Red"s, "Anaglyph Right Eye Red Component"s, 0.f, 1.f, 0.05f);
PropFloat(Player, Anaglyph6RightGreen, "Anaglyph Right Green"s, "Anaglyph Right Eye Green Component"s, 0.f, 1.f, 0.16f);
PropFloat(Player, Anaglyph6RightBlue, "Anaglyph Right Blue"s, "Anaglyph Right Eye Blue Component"s, 0.f, 1.f, 0.96f);
// Red Cyan
PropFloat(Player, Anaglyph7LeftRed, "Anaglyph Left Red"s, "Anaglyph Left Eye Red Component"s, 0.f, 1.f, 0.95f);
PropFloat(Player, Anaglyph7LeftGreen, "Anaglyph Left Green"s, "Anaglyph Left Eye Green Component"s, 0.f, 1.f, 0.19f);
PropFloat(Player, Anaglyph7LeftBlue, "Anaglyph Left Blue"s, "Anaglyph Left Eye Blue Component"s, 0.f, 1.f, 0.07f);
PropFloat(Player, Anaglyph7RightRed, "Anaglyph Right Red"s, "Anaglyph Right Eye Red Component"s, 0.f, 1.f, 0.06f);
PropFloat(Player, Anaglyph7RightGreen, "Anaglyph Right Green"s, "Anaglyph Right Eye Green Component"s, 0.f, 1.f, 0.92f);
PropFloat(Player, Anaglyph7RightBlue, "Anaglyph Right Blue"s, "Anaglyph Right Eye Blue Component"s, 0.f, 1.f, 0.28f);
// Red Cyan
PropFloat(Player, Anaglyph8LeftRed, "Anaglyph Left Red"s, "Anaglyph Left Eye Red Component"s, 0.f, 1.f, 0.95f);
PropFloat(Player, Anaglyph8LeftGreen, "Anaglyph Left Green"s, "Anaglyph Left Eye Green Component"s, 0.f, 1.f, 0.19f);
PropFloat(Player, Anaglyph8LeftBlue, "Anaglyph Left Blue"s, "Anaglyph Left Eye Blue Component"s, 0.f, 1.f, 0.07f);
PropFloat(Player, Anaglyph8RightRed, "Anaglyph Right Red"s, "Anaglyph Right Eye Red Component"s, 0.f, 1.f, 0.06f);
PropFloat(Player, Anaglyph8RightGreen, "Anaglyph Right Green"s, "Anaglyph Right Eye Green Component"s, 0.f, 1.f, 0.92f);
PropFloat(Player, Anaglyph8RightBlue, "Anaglyph Right Blue"s, "Anaglyph Right Eye Blue Component"s, 0.f, 1.f, 0.28f);
// Red Cyan
PropFloat(Player, Anaglyph9LeftRed, "Anaglyph Left Red"s, "Anaglyph Left Eye Red Component"s, 0.f, 1.f, 0.95f);
PropFloat(Player, Anaglyph9LeftGreen, "Anaglyph Left Green"s, "Anaglyph Left Eye Green Component"s, 0.f, 1.f, 0.19f);
PropFloat(Player, Anaglyph9LeftBlue, "Anaglyph Left Blue"s, "Anaglyph Left Eye Blue Component"s, 0.f, 1.f, 0.07f);
PropFloat(Player, Anaglyph9RightRed, "Anaglyph Right Red"s, "Anaglyph Right Eye Red Component"s, 0.f, 1.f, 0.06f);
PropFloat(Player, Anaglyph9RightGreen, "Anaglyph Right Green"s, "Anaglyph Right Eye Green Component"s, 0.f, 1.f, 0.92f);
PropFloat(Player, Anaglyph9RightBlue, "Anaglyph Right Blue"s, "Anaglyph Right Eye Blue Component"s, 0.f, 1.f, 0.28f);
// Red Cyan
PropFloat(Player, Anaglyph10LeftRed, "Anaglyph Left Red"s, "Anaglyph Left Eye Red Component"s, 0.f, 1.f, 0.95f);
PropFloat(Player, Anaglyph10LeftGreen, "Anaglyph Left Green"s, "Anaglyph Left Eye Green Component"s, 0.f, 1.f, 0.19f);
PropFloat(Player, Anaglyph10LeftBlue, "Anaglyph Left Blue"s, "Anaglyph Left Eye Blue Component"s, 0.f, 1.f, 0.07f);
PropFloat(Player, Anaglyph10RightRed, "Anaglyph Right Red"s, "Anaglyph Right Eye Red Component"s, 0.f, 1.f, 0.06f);
PropFloat(Player, Anaglyph10RightGreen, "Anaglyph Right Green"s, "Anaglyph Right Eye Green Component"s, 0.f, 1.f, 0.92f);
PropFloat(Player, Anaglyph10RightBlue, "Anaglyph Right Blue"s, "Anaglyph Right Eye Blue Component"s, 0.f, 1.f, 0.28f);

PropArray(Player, AnaglyphLeftRed, float, Float, Float, m_propPlayer_Anaglyph1LeftRed, m_propPlayer_Anaglyph2LeftRed, m_propPlayer_Anaglyph3LeftRed, m_propPlayer_Anaglyph4LeftRed,
   m_propPlayer_Anaglyph5LeftRed, m_propPlayer_Anaglyph6LeftRed, m_propPlayer_Anaglyph7LeftRed, m_propPlayer_Anaglyph8LeftRed, m_propPlayer_Anaglyph9LeftRed, m_propPlayer_Anaglyph10LeftRed);
PropArray(Player, AnaglyphLeftGreen, float, Float, Float, m_propPlayer_Anaglyph1LeftGreen, m_propPlayer_Anaglyph2LeftGreen, m_propPlayer_Anaglyph3LeftGreen, m_propPlayer_Anaglyph4LeftGreen,
   m_propPlayer_Anaglyph5LeftGreen, m_propPlayer_Anaglyph6LeftGreen, m_propPlayer_Anaglyph7LeftGreen, m_propPlayer_Anaglyph8LeftGreen, m_propPlayer_Anaglyph9LeftGreen,
   m_propPlayer_Anaglyph10LeftGreen);
PropArray(Player, AnaglyphLeftBlue, float, Float, Float, m_propPlayer_Anaglyph1LeftBlue, m_propPlayer_Anaglyph2LeftBlue, m_propPlayer_Anaglyph3LeftBlue, m_propPlayer_Anaglyph4LeftBlue,
   m_propPlayer_Anaglyph5LeftBlue, m_propPlayer_Anaglyph6LeftBlue, m_propPlayer_Anaglyph7LeftBlue, m_propPlayer_Anaglyph8LeftBlue, m_propPlayer_Anaglyph9LeftBlue,
   m_propPlayer_Anaglyph10LeftBlue);
PropArray(Player, AnaglyphRightRed, float, Float, Float, m_propPlayer_Anaglyph1RightRed, m_propPlayer_Anaglyph2RightRed, m_propPlayer_Anaglyph3RightRed, m_propPlayer_Anaglyph4RightRed,
   m_propPlayer_Anaglyph5RightRed, m_propPlayer_Anaglyph6RightRed, m_propPlayer_Anaglyph7RightRed, m_propPlayer_Anaglyph8RightRed, m_propPlayer_Anaglyph9RightRed,
   m_propPlayer_Anaglyph10RightRed);
PropArray(Player, AnaglyphRightGreen, float, Float, Float, m_propPlayer_Anaglyph1RightGreen, m_propPlayer_Anaglyph2RightGreen, m_propPlayer_Anaglyph3RightGreen,
   m_propPlayer_Anaglyph4RightGreen, m_propPlayer_Anaglyph5RightGreen, m_propPlayer_Anaglyph6RightGreen, m_propPlayer_Anaglyph7RightGreen, m_propPlayer_Anaglyph8RightGreen,
   m_propPlayer_Anaglyph9RightGreen, m_propPlayer_Anaglyph10RightGreen);
PropArray(Player, AnaglyphRightBlue, float, Float, Float, m_propPlayer_Anaglyph1RightBlue, m_propPlayer_Anaglyph2RightBlue, m_propPlayer_Anaglyph3RightBlue, m_propPlayer_Anaglyph4RightBlue,
   m_propPlayer_Anaglyph5RightBlue, m_propPlayer_Anaglyph6RightBlue, m_propPlayer_Anaglyph7RightBlue, m_propPlayer_Anaglyph8RightBlue, m_propPlayer_Anaglyph9RightBlue,
   m_propPlayer_Anaglyph10RightBlue)

// Real world cabinet & player settings
PropFloat(Player, ScreenWidth, "Screen Width"s, "Physical width (cm) of the display area of the playfield (main) screen (width > height)"s, 5.f, 200.f, 95.89f);
PropFloat(Player, ScreenHeight, "Screen Height"s, "Physical height (cm) of the display area of the playfield (main) screen (width > height)"s, 5.f, 200.f, 53.94f);
PropFloat(Player, ScreenInclination, "Screen Inclination"s, "Inclination (degree) of the playfield (main) screen, 0 is horizontal"s, -30.f, 30.f, 0.f);
PropFloat(Player, LockbarWidth, "Lockbar Width"s, "Lockbar width in centimeters (measured on the cabinet)"s, 10.f, 150.f, 70.f);
PropFloat(Player, LockbarHeight, "Lockbar Height"s, "Lockbar height in centimeters (measured on the cabinet, from ground to top of lockbar)"s, 0.f, 250.f, 85.f);
PropFloatDyn(Player, ScreenPlayerX, "Player X"s, "Player X position in real world, expressed from the bottom center of the playfield, in centimeters"s, -30.f, 30.f, 0.f);
PropFloatDyn(Player, ScreenPlayerY, "Player Y"s, "Player Y position in real world, expressed from the bottom center of the playfield, in centimeters"s, -70.f, 30.f, -10.f);
PropFloatDyn(Player, ScreenPlayerZ, "Player Z"s, "Player Z position in real world, expressed from the bottom center of the playfield, in centimeters"s, 30.f, 100.f, 70.f);

// Overall scene lighting settings
PropBool(Player, OverrideTableEmissionScale, "Override Table Light Level"s, "Replace default table light level by a fixed/custom one"s, false);
PropFloatDyn(Player, EmissionScale, "Day/Night"s, "Select a custom ambient lighting level, ranging from daylight to night time"s, 0.f, 1.f, 1.f);
PropBool(Player, DynamicDayNight, "Use Automatic Light Level"s, "Automatically compute/match scene lighting based on geographic coordinates/localized sun position"s, false);
PropFloat(Player, Latitude, "Latitude"s, "Local geographic latitude used to derive the sun position for automatically computed/matching scene lighting"s, -90.f, 90.f, 52.52f);
PropFloat(Player, Longitude, "Longitude"s, "Local geographic longitude used to derive the sun position for automatically computed/matching scene lighting"s, -180.f, 180.f, 13.37f);

// Debugging & Live editing settings
PropBool(Editor, ThrowBallsAlwaysOn, "Throw Balls Always On"s, "Permanently enable 'throw ball' debugging mode"s, false);
PropBool(Editor, BallControlAlwaysOn, "Ball COntrol Always On"s, "Permanently enable 'ball control' debugging mode"s, false);

// Default camera setup
PropEnum(DefaultCamera, DesktopMode, "View mode"s,
   "Select between 'Legacy' (old rendering mode with visually incorrect stretchs), 'Camera' (classic camera, for desktop) and 'Window' (custom projection designed for cabinet users) rendering mode"s,
   int, 1, "Legacy"s, "Camera"s, "Window"s);
PropFloat(DefaultCamera, DesktopLookAt, "Look at"s, "Relative point of playfield where the camera is looking at"s, 0.f, 100.f, 25.f);
PropFloat(DefaultCamera, DesktopFov, "Field Of View (overall scale)"s, "Global view scale (same as XYZ scale)"s, 25.f, 90.f, 50.f);
PropFloat(DefaultCamera, DesktopScaleX, "Table X Scale"s, "Stretch the scene along the playfield width axis"s, 0.5f, 1.5f, 1.f);
PropFloat(DefaultCamera, DesktopScaleY, "Table Y Scale"s, "Stretch the scene along the playfield height axis"s, 0.5f, 1.5f, 1.f);
PropFloat(DefaultCamera, DesktopScaleZ, "Table Z Scale"s, "Stretch the scene along the vertical axis (perpendicular to playfield)"s, 0.5f, 1.5f, 1.f);
PropFloat(DefaultCamera, DesktopCamX, "Camera X"s, "View point width offset, in centimeters"s, -30.f, 30.f, 0.f);
PropFloat(DefaultCamera, DesktopCamY, "Camera Y"s, "View point height offset, in centimeters"s, -30.f, 100.f, 20.f);
PropFloat(DefaultCamera, DesktopCamZ, "Camera Z"s, "View point vertical offset, in centimeters"s, 10.f, 100.f, 70.f);
PropFloat(DefaultCamera, DesktopViewVOfs, "Vertical Offset"s, "Vertical offset of the virtual table behind the screen 'window' in centimeters"s, -20.f, 50.f, 14.f);
PropEnum(DefaultCamera, FSSMode, "View mode"s,
   "Select between 'Legacy' (old rendering mode with visually incorrect stretchs), 'Camera' (classic camera, for desktop) and 'Window' (custom projection designed for cabinet users) rendering mode"s,
   int, 1, "Legacy"s, "Camera"s, "Window"s);
PropFloat(DefaultCamera, FSSLookAt, "Look at"s, "Relative point of playfield where the camera is looking at"s, 0.f, 100.f, 50.f);
PropFloat(DefaultCamera, FSSFov, "Field Of View (overall scale)"s, "Global view scale (same as XYZ scale)"s, 25.f, 90.f, 77.f);
PropFloat(DefaultCamera, FSSScaleX, "Table X Scale"s, "Stretch the scene along the playfield width axis"s, 0.5f, 1.5f, 1.f);
PropFloat(DefaultCamera, FSSScaleY, "Table Y Scale"s, "Stretch the scene along the playfield height axis"s, 0.5f, 1.5f, 1.f);
PropFloat(DefaultCamera, FSSScaleZ, "Table Z Scale"s, "Stretch the scene along the vertical axis (perpendicular to playfield)"s, 0.5f, 1.5f, 1.f);
PropFloat(DefaultCamera, FSSCamX, "Camera X"s, "View point width offset, in centimeters"s, -30.f, 30.f, 0.f);
PropFloat(DefaultCamera, FSSCamY, "Camera Y"s, "View point height offset, in centimeters"s, -30.f, 100.f, 20.f);
PropFloat(DefaultCamera, FSSCamZ, "Camera Z"s, "View point vertical offset, in centimeters"s, 10.f, 100.f, 70.f);
PropFloat(DefaultCamera, FSSViewVOfs, "Vertical Offset"s, "Vertical offset of the virtual table behind the screen 'window' in centimeters"s, -20.f, 50.f, 22.f);

// Table override settings
// These properties are specials as they are meant to override table data at play time. They are not meant to be saved to the application setting file, but
// only to table override settings. Also, their defaults are redefined when a table is played, depending on the table data and UI options.
PropEnumDyn(TableOverride, ViewDTMode, "View mode"s,
   "Select between 'Legacy' (old rendering mode with visually incorrect stretchs), 'Camera' (classic camera, for desktop) and 'Window' (custom projection designed for cabinet users) rendering mode"s,
   int, 1, "Legacy"s, "Camera"s, "Window"s);
PropFloatDyn(TableOverride, ViewDTLookAt, "Look at"s, "Relative point of playfield where the camera is looking at"s, 0.f, 100.f, 25.f);
PropFloatDyn(TableOverride, ViewDTFOV, "Field Of View (overall scale)"s, "Global view scale (same as XYZ scale)"s, 25.f, 90.f, 50.f);
PropFloatDyn(TableOverride, ViewDTLayback, "Layback"s, "Fake visual stretch of the table to give more depth"s, -90.f, 90.f, 0.f);
PropFloatDyn(TableOverride, ViewDTScaleX, "Table X Scale"s, "Stretch the scene along the playfield width axis"s, 0.5f, 1.5f, 1.f);
PropFloatDyn(TableOverride, ViewDTScaleY, "Table Y Scale"s, "Stretch the scene along the playfield height axis"s, 0.5f, 1.5f, 1.f);
PropFloatDyn(TableOverride, ViewDTScaleZ, "Table Z Scale"s, "Stretch the scene along the vertical axis (perpendicular to playfield)"s, 0.5f, 1.5f, 1.f);
PropFloatDyn(TableOverride, ViewDTPlayerX, "Camera X"s, "View point width offset"s, CMTOVPU(-30.f), CMTOVPU(30.f), CMTOVPU(0.f));
PropFloatDyn(TableOverride, ViewDTPlayerY, "Camera Y"s, "View point height offset"s, CMTOVPU(-100.f), CMTOVPU(100.f), CMTOVPU(20.f));
PropFloatDyn(TableOverride, ViewDTPlayerZ, "Camera Z"s, "View point vertical offset"s, CMTOVPU(-100.f), CMTOVPU(100.f), CMTOVPU(70.f));
PropFloatDyn(TableOverride, ViewDTHOfs, "Horizontal Offset"s, "Horizontal offset of the virtual table behind the screen 'window'"s, -30.f, 30.f, 0.f);
PropFloatDyn(TableOverride, ViewDTVOfs, "Vertical Offset"s, "Vertical offset of the virtual table behind the screen 'window'"s, -20.f, 50.f, 0.f);
PropFloatDyn(TableOverride, ViewDTWindowTop, "Window Top Z Ofs."s, "Distance between the 'window' (i.e. the screen) at the top of the playfield"s, CMTOVPU(0.f), CMTOVPU(50.f), CMTOVPU(0.f));
PropFloatDyn(
   TableOverride, ViewDTWindowBot, "Window Bottom Z Ofs."s, "Distance between the 'window' (i.e. the screen) at the bottom of the playfield"s, CMTOVPU(0.f), CMTOVPU(50.f), CMTOVPU(0.f));
PropFloatSteppedDyn(TableOverride, ViewDTRotation, "Viewport Rotation"s, ""s, 0.f, 360.f, 90.0f, 0.f);

PropEnumDyn(TableOverride, ViewFSSMode, "View mode"s,
   "Select between 'Legacy' (old rendering mode with visually incorrect stretchs), 'Camera' (classic camera, for desktop) and 'Window' (custom projection designed for cabinet users) rendering mode"s,
   int, 1, "Legacy"s, "Camera"s, "Window"s);
PropFloatDyn(TableOverride, ViewFSSLookAt, "Look at"s, "Relative point of playfield where the camera is looking at"s, 0.f, 100.f, 0.f);
PropFloatDyn(TableOverride, ViewFSSFOV, "Field Of View (overall scale)"s, "Global view scale (same as XYZ scale)"s, 25.f, 90.f, 25.f);
PropFloatDyn(TableOverride, ViewFSSLayback, "Layback"s, "Fake visual stretch of the table to give more depth"s, -90.f, 90.f, 0.f);
PropFloatDyn(TableOverride, ViewFSSScaleX, "Table X Scale"s, "Stretch the scene along the playfield width axis"s, 0.5f, 1.5f, 1.f);
PropFloatDyn(TableOverride, ViewFSSScaleY, "Table Y Scale"s, "Stretch the scene along the playfield height axis"s, 0.5f, 1.5f, 1.f);
PropFloatDyn(TableOverride, ViewFSSScaleZ, "Table Z Scale"s, "Stretch the scene along the vertical axis (perpendicular to playfield)"s, 0.5f, 1.5f, 1.f);
PropFloatDyn(TableOverride, ViewFSSPlayerX, "Camera X"s, "View point width offset"s, CMTOVPU(-30.f), CMTOVPU(30.f), CMTOVPU(0.f));
PropFloatDyn(TableOverride, ViewFSSPlayerY, "Camera Y"s, "View point height offset"s, CMTOVPU(-100.f), CMTOVPU(100.f), CMTOVPU(0.f));
PropFloatDyn(TableOverride, ViewFSSPlayerZ, "Camera Z"s, "View point vertical offsets"s, CMTOVPU(-100.f), CMTOVPU(100.f), CMTOVPU(70.f));
PropFloatDyn(TableOverride, ViewFSSHOfs, "Horizontal Offset"s, "Horizontal offset of the virtual table behind the screen 'window'"s, -30.f, 30.f, 0.f);
PropFloatDyn(TableOverride, ViewFSSVOfs, "Vertical Offset"s, "Vertical offset of the virtual table behind the screen 'window'"s, -20.f, 50.f, 0.f);
PropFloatDyn(
   TableOverride, ViewFSSWindowTop, "Window Top Z Ofs."s, "Distance between the 'window' (i.e. the screen) at the top of the playfield"s, CMTOVPU(0.f), CMTOVPU(50.f), CMTOVPU(0.f));
PropFloatDyn(
   TableOverride, ViewFSSWindowBot, "Window Bottom Z Ofs."s, "Distance between the 'window' (i.e. the screen) at the bottom of the playfield"s, CMTOVPU(0.f), CMTOVPU(50.f), CMTOVPU(0.f));
PropFloatSteppedDyn(TableOverride, ViewFSSRotation, "Viewport Rotation"s, ""s, 0.f, 360.f, 90.0f, 0.f);

PropEnumDyn(TableOverride, ViewCabMode, "View mode"s,
   "Select between 'Legacy' (old rendering mode with visually incorrect stretchs), 'Camera' (classic camera, for desktop) and 'Window' (custom projection designed for cabinet users) rendering mode"s,
   int, 2, "Legacy"s, "Camera"s, "Window"s);
PropFloatDyn(TableOverride, ViewCabLookAt, "Look at"s, "Relative point of playfield where the camera is looking at"s, 0.f, 100.f, 0.f);
PropFloatDyn(TableOverride, ViewCabFOV, "Field Of View (overall scale)"s, "Global view scale (same as XYZ scale)"s, 25.f, 90.f, 25.f);
PropFloatDyn(TableOverride, ViewCabLayback, "Layback"s, "Fake visual stretch of the table to give more depth"s, 0.f, 90.f, 0.f);
PropFloatDyn(TableOverride, ViewCabScaleX, "Table X Scale"s, "Stretch the scene along the playfield width axis"s, 0.5f, 1.5f, 1.f);
PropFloatDyn(TableOverride, ViewCabScaleY, "Table Y Scale"s, "Stretch the scene along the playfield height axis"s, 0.5f, 1.5f, 1.f);
PropFloatDyn(TableOverride, ViewCabScaleZ, "Table Z Scale"s, "Stretch the scene along the vertical axis (perpendicular to playfield)"s, 0.5f, 1.5f, 1.f);
PropFloatDyn(TableOverride, ViewCabPlayerX, "Camera X"s, "View point width offset"s, CMTOVPU(-30.f), CMTOVPU(30.f), CMTOVPU(0.f));
PropFloatDyn(TableOverride, ViewCabPlayerY, "Camera Y"s, "View point height offset"s, CMTOVPU(-100.f), CMTOVPU(100.f), CMTOVPU(0.f));
PropFloatDyn(TableOverride, ViewCabPlayerZ, "Camera Z"s, "View point vertical offset"s, CMTOVPU(-100.f), CMTOVPU(100.f), CMTOVPU(70.f));
PropFloatDyn(TableOverride, ViewCabHOfs, "Horizontal Offset"s, "Horizontal offset of the virtual table behind the screen 'window'"s, -30.f, 30.f, 0.f);
PropFloatDyn(TableOverride, ViewCabVOfs, "Vertical Offset"s, "Vertical offset of the virtual table behind the screen 'window'"s, -20.f, 50.f, 0.f);
PropFloatDyn(
   TableOverride, ViewCabWindowTop, "Window Top Z Ofs."s, "Distance between the 'window' (i.e. the screen) at the top of the playfield"s, CMTOVPU(0.f), CMTOVPU(50.f), CMTOVPU(0.f));
PropFloatDyn(
   TableOverride, ViewCabWindowBot, "Window Bottom Z Ofs."s, "Distance between the 'window' (i.e. the screen) at the bottom of the playfield"s, CMTOVPU(0.f), CMTOVPU(50.f), CMTOVPU(0.f));
PropFloatSteppedDyn(TableOverride, ViewCabRotation, "Viewport Rotation"s, ""s, 0.f, 360.f, 90.0f, 0.f);

PropFloatDyn(TableOverride, Difficulty, "Difficulty"s, "Overall difficulty (affects slope, flipper size, ball trajectories scattering,...)"s, 0.f, 1.f, 1.f);
PropFloatDyn(TableOverride, Exposure, "Camera Exposure"s, "Overall brightness scale for the rendered scene/table"s, 0.f, 2.f, 1.f);

#ifdef ENABLE_BGFX
PropEnumDyn(TableOverride, ToneMapper, "Tonemapper"s, "Colors too bright to be displayed by low dynamic range monitors need to be mapped into a normalized range. Different mappers come at different tradeoffs, depending on each tables setup/lighting."s, int, 0, "Reinhard"s, "AgX"s, "Filmic"s, "Neutral"s,
   "AgX Punchy"s);
#else
PropEnumDyn(
   TableOverride, ToneMapper, "Tonemapper"s, "Colors too bright to be displayed by low dynamic range monitors need to be mapped into a normalized range. Different mappers come at different tradeoffs, depending on each tables setup/lighting."s, int, 0, "Reinhard"s, "AgX"s, "Filmic"s, "Neutral"s);
#endif

// DMD Display profiles
// Legacy
PropBool(DMD, Profile1Legacy, "Legacy Renderer"s, "Use legacy DMD renderer"s, true);
PropInt(DMD, Profile1DotTint, "Dot Tint"s, "Color of lit dots"s, 0x000000, 0xFFFFFF, 0x2D52FF);
PropInt(DMD, Profile1UnlitDotColor, "Unlit Dot Tint"s, "Color of unlit dots"s, 0x000000, 0xFFFFFF, 0x404040);
PropFloat(DMD, Profile1DotBrightness, "Dot Brightness"s, "Dot brightness level"s, 0.001f, 100.f, 25.f);
PropFloat(DMD, Profile1DotSize, "Dot Size"s, "Dot size"s, 0.001f, 1.f, 0.75f);
PropFloat(DMD, Profile1DotSharpness, "Dot Sharpness"s, "Sharpness of the dots (from round to squares)"s, 0.f, 1.f, 0.90f);
PropFloat(DMD, Profile1DiffuseGlow, "DMD Diffuse Glow"s, "Glow of dots on the back of the DMD (between dots, against glass)"s, 0.f, 10.f, 0.1f);
// Classic Neon plasma DMD
PropBool(DMD, Profile2Legacy, "Legacy Renderer"s, "Use legacy DMD renderer"s, false);
PropInt(DMD, Profile2DotTint, "Dot Tint"s, "Color of lit dots"s, 0x000000, 0xFFFFFF, 0x2D52FF);
PropInt(DMD, Profile2UnlitDotColor, "Unlit Dot Tint"s, "Color of unlit dots"s, 0x000000, 0xFFFFFF, 0x404040);
PropFloat(DMD, Profile2DotBrightness, "Dot Brightness"s, "Dot brightness level"s, 0.001f, 100.f, 25.f);
PropFloat(DMD, Profile2DotSize, "Dot Size"s, "Dot size"s, 0.001f, 1.f, 0.6f);
PropFloat(DMD, Profile2DotSharpness, "Dot Sharpness"s, "Sharpness of the dots (from round to squares)"s, 0.f, 1.f, 0.90f);
PropFloat(DMD, Profile2DiffuseGlow, "DMD Diffuse Glow"s, "Glow of dots on the back of the DMD (between dots, against glass)"s, 0.f, 10.f, 0.1f);
// Red Led DMD (used after RoHS regulation entry into force)
PropBool(DMD, Profile3Legacy, "Legacy Renderer"s, "Use legacy DMD renderer"s, false);
PropInt(DMD, Profile3DotTint, "Dot Tint"s, "Color of lit dots"s, 0x000000, 0xFFFFFF, 0x1523FF);
PropInt(DMD, Profile3UnlitDotColor, "Unlit Dot Tint"s, "Color of unlit dots"s, 0x000000, 0xFFFFFF, 0x404040);
PropFloat(DMD, Profile3DotBrightness, "Dot Brightness"s, "Dot brightness level"s, 0.001f, 100.f, 25.f);
PropFloat(DMD, Profile3DotSize, "Dot Size"s, "Dot size"s, 0.001f, 1.f, 0.75f);
PropFloat(DMD, Profile3DotSharpness, "Dot Sharpness"s, "Sharpness of the dots (from round to squares)"s, 0.f, 1.f, 0.90f);
PropFloat(DMD, Profile3DiffuseGlow, "DMD Diffuse Glow"s, "Glow of dots on the back of the DMD (between dots, against glass)"s, 0.f, 10.f, 0.1f);
// Green Led
PropBool(DMD, Profile4Legacy, "Legacy Renderer"s, "Use legacy DMD renderer"s, false);
PropInt(DMD, Profile4DotTint, "Dot Tint"s, "Color of lit dots"s, 0x000000, 0xFFFFFF, 0x23FF15);
PropInt(DMD, Profile4UnlitDotColor, "Unlit Dot Tint"s, "Color of unlit dots"s, 0x000000, 0xFFFFFF, 0x404040);
PropFloat(DMD, Profile4DotBrightness, "Dot Brightness"s, "Dot brightness level"s, 0.001f, 100.f, 25.f);
PropFloat(DMD, Profile4DotSize, "Dot Size"s, "Dot size"s, 0.001f, 1.f, 0.75f);
PropFloat(DMD, Profile4DotSharpness, "Dot Sharpness"s, "Sharpness of the dots (from round to squares)"s, 0.f, 1.f, 0.90f);
PropFloat(DMD, Profile4DiffuseGlow, "DMD Diffuse Glow"s, "Glow of dots on the back of the DMD (between dots, against glass)"s, 0.f, 10.f, 0.1f);
// Yellow Led
PropBool(DMD, Profile5Legacy, "Legacy Renderer"s, "Use legacy DMD renderer"s, false);
PropInt(DMD, Profile5DotTint, "Dot Tint"s, "Color of lit dots"s, 0x000000, 0xFFFFFF, 0x23FFFF);
PropInt(DMD, Profile5UnlitDotColor, "Unlit Dot Tint"s, "Color of unlit dots"s, 0x000000, 0xFFFFFF, 0x404040);
PropFloat(DMD, Profile5DotBrightness, "Dot Brightness"s, "Dot brightness level"s, 0.001f, 100.f, 25.f);
PropFloat(DMD, Profile5DotSize, "Dot Size"s, "Dot size"s, 0.001f, 1.f, 0.75f);
PropFloat(DMD, Profile5DotSharpness, "Dot Sharpness"s, "Sharpness of the dots (from round to squares)"s, 0.f, 1.f, 0.90f);
PropFloat(DMD, Profile5DiffuseGlow, "DMD Diffuse Glow"s, "Glow of dots on the back of the DMD (between dots, against glass)"s, 0.f, 10.f, 0.1f);
// Generic Plasma
PropBool(DMD, Profile6Legacy, "Legacy Renderer"s, "Use legacy DMD renderer"s, false);
PropInt(DMD, Profile6DotTint, "Dot Tint"s, "Color of lit dots"s, 0x000000, 0xFFFFFF, 0xFFFFFF);
PropInt(DMD, Profile6UnlitDotColor, "Unlit Dot Tint"s, "Color of unlit dots"s, 0x000000, 0xFFFFFF, 0x404040);
PropFloat(DMD, Profile6DotBrightness, "Dot Brightness"s, "Dot brightness level"s, 0.001f, 100.f, 25.f);
PropFloat(DMD, Profile6DotSize, "Dot Size"s, "Dot size"s, 0.001f, 1.f, 0.6f);
PropFloat(DMD, Profile6DotSharpness, "Dot Sharpness"s, "Sharpness of the dots (from round to squares)"s, 0.f, 1.f, 0.90f);
PropFloat(DMD, Profile6DiffuseGlow, "DMD Diffuse Glow"s, "Glow of dots on the back of the DMD (between dots, against glass)"s, 0.f, 10.f, 0.1f);
// Generic Led
PropBool(DMD, Profile7Legacy, "Legacy Renderer"s, "Use legacy DMD renderer"s, false);
PropInt(DMD, Profile7DotTint, "Dot Tint"s, "Color of lit dots"s, 0x000000, 0xFFFFFF, 0xFFFFFF);
PropInt(DMD, Profile7UnlitDotColor, "Unlit Dot Tint"s, "Color of unlit dots"s, 0x000000, 0xFFFFFF, 0x404040);
PropFloat(DMD, Profile7DotBrightness, "Dot Brightness"s, "Dot brightness level"s, 0.001f, 100.f, 25.f);
PropFloat(DMD, Profile7DotSize, "Dot Size"s, "Dot size"s, 0.001f, 1.f, 0.75f);
PropFloat(DMD, Profile7DotSharpness, "Dot Sharpness"s, "Sharpness of the dots (from round to squares)"s, 0.f, 1.f, 0.90f);
PropFloat(DMD, Profile7DiffuseGlow, "DMD Diffuse Glow"s, "Glow of dots on the back of the DMD (between dots, against glass)"s, 0.f, 10.f, 0.1f);
// Array access
PropArray(DMD, ProfileLegacy, bool, Int, Int, m_propDMD_Profile1Legacy, m_propDMD_Profile2Legacy, m_propDMD_Profile3Legacy, m_propDMD_Profile4Legacy, m_propDMD_Profile5Legacy,
   m_propDMD_Profile6Legacy, m_propDMD_Profile7Legacy);
PropArray(DMD, ProfileDotTint, int, Int, Int, m_propDMD_Profile1DotTint, m_propDMD_Profile2DotTint, m_propDMD_Profile3DotTint, m_propDMD_Profile4DotTint, m_propDMD_Profile5DotTint,
   m_propDMD_Profile6DotTint, m_propDMD_Profile7DotTint);
PropArray(DMD, ProfileUnlitDotColor, int, Int, Int, m_propDMD_Profile1UnlitDotColor, m_propDMD_Profile2UnlitDotColor, m_propDMD_Profile3UnlitDotColor, m_propDMD_Profile4UnlitDotColor,
   m_propDMD_Profile5UnlitDotColor, m_propDMD_Profile6UnlitDotColor, m_propDMD_Profile7UnlitDotColor);
PropArray(DMD, ProfileDotBrightness, float, Float, Float, m_propDMD_Profile1DotBrightness, m_propDMD_Profile2DotBrightness, m_propDMD_Profile3DotBrightness, m_propDMD_Profile4DotBrightness,
   m_propDMD_Profile5DotBrightness, m_propDMD_Profile6DotBrightness, m_propDMD_Profile7DotBrightness);
PropArray(DMD, ProfileDotSize, float, Float, Float, m_propDMD_Profile1DotSize, m_propDMD_Profile2DotSize, m_propDMD_Profile3DotSize, m_propDMD_Profile4DotSize, m_propDMD_Profile5DotSize,
   m_propDMD_Profile6DotSize, m_propDMD_Profile7DotSize);
PropArray(DMD, ProfileDotSharpness, float, Float, Float, m_propDMD_Profile1DotSharpness, m_propDMD_Profile2DotSharpness, m_propDMD_Profile3DotSharpness, m_propDMD_Profile4DotSharpness,
   m_propDMD_Profile5DotSharpness, m_propDMD_Profile6DotSharpness, m_propDMD_Profile7DotSharpness);
PropArray(DMD, ProfileDiffuseGlow, float, Float, Float, m_propDMD_Profile1DiffuseGlow, m_propDMD_Profile2DiffuseGlow, m_propDMD_Profile3DiffuseGlow, m_propDMD_Profile4DiffuseGlow,
   m_propDMD_Profile5DiffuseGlow, m_propDMD_Profile6DiffuseGlow, m_propDMD_Profile7DiffuseGlow);

// Alphanumeric Segment Display profiles
// Neon Plasma
PropInt(Alpha, Profile1Color, "Color"s, "Color of lit segments"s, 0x000000, 0xFFFFFF, 0x001E96FF); // 0x002D52FF
PropInt(Alpha, Profile1Unlit, "Unlit Color"s, "Color of unlit segments"s, 0x000000, 0xFFFFFF, 0x404040);
PropFloat(Alpha, Profile1Brightness, "Brightness"s, "Brightness level"s, 0.001f, 100.f, 5.f);
PropFloat(Alpha, Profile1DiffuseGlow, "Diffuse Glow"s, "Glow of segments on the back of the display (between segments, against glass)"s, 0.f, 10.f, 1.f);
// VFD Blueish
PropInt(Alpha, Profile2Color, "Color"s, "Color of lit segments"s, 0x000000, 0xFFFFFF, 0x00FFEF3F); // 0x00FFEFBF
PropInt(Alpha, Profile2Unlit, "Unlit Color"s, "Color of unlit segments"s, 0x000000, 0xFFFFFF, 0x404040);
PropFloat(Alpha, Profile2Brightness, "Brightness"s, "Brightness level"s, 0.001f, 100.f, 5.f);
PropFloat(Alpha, Profile2DiffuseGlow, "Diffuse Glow"s, "Glow of segments on the back of the display (between segments, against glass)"s, 0.f, 10.f, 1.f);
// VFD Greenish
PropInt(Alpha, Profile3Color, "Color"s, "Color of lit segments"s, 0x000000, 0xFFFFFF, 0x00ECFF48);
PropInt(Alpha, Profile3Unlit, "Unlit Color"s, "Color of unlit segments"s, 0x000000, 0xFFFFFF, 0x404040);
PropFloat(Alpha, Profile3Brightness, "Brightness"s, "Brightness level"s, 0.001f, 100.f, 5.f);
PropFloat(Alpha, Profile3DiffuseGlow, "Diffuse Glow"s, "Glow of segments on the back of the display (between segments, against glass)"s, 0.f, 10.f, 1.f);
// Red Led
PropInt(Alpha, Profile4Color, "Color"s, "Color of lit segments"s, 0x000000, 0xFFFFFF, 0x001523FF);
PropInt(Alpha, Profile4Unlit, "Unlit Color"s, "Color of unlit segments"s, 0x000000, 0xFFFFFF, 0x404040);
PropFloat(Alpha, Profile4Brightness, "Brightness"s, "Brightness level"s, 0.001f, 100.f, 5.f);
PropFloat(Alpha, Profile4DiffuseGlow, "Diffuse Glow"s, "Glow of segments on the back of the display (between segments, against glass)"s, 0.f, 10.f, 1.f);
// Green Led
PropInt(Alpha, Profile5Color, "Color"s, "Color of lit segments"s, 0x000000, 0xFFFFFF, 0x0023FF15);
PropInt(Alpha, Profile5Unlit, "Unlit Color"s, "Color of unlit segments"s, 0x000000, 0xFFFFFF, 0x404040);
PropFloat(Alpha, Profile5Brightness, "Brightness"s, "Brightness level"s, 0.001f, 100.f, 5.f);
PropFloat(Alpha, Profile5DiffuseGlow, "Diffuse Glow"s, "Glow of segments on the back of the display (between segments, against glass)"s, 0.f, 10.f, 1.f);
// Yellow Led
PropInt(Alpha, Profile6Color, "Color"s, "Color of lit segments"s, 0x000000, 0xFFFFFF, 0x0023FFFF);
PropInt(Alpha, Profile6Unlit, "Unlit Color"s, "Color of unlit segments"s, 0x000000, 0xFFFFFF, 0x404040);
PropFloat(Alpha, Profile6Brightness, "Brightness"s, "Brightness level"s, 0.001f, 100.f, 5.f);
PropFloat(Alpha, Profile6DiffuseGlow, "Diffuse Glow"s, "Glow of segments on the back of the display (between segments, against glass)"s, 0.f, 10.f, 1.f);
// Generic Plasma
PropInt(Alpha, Profile7Color, "Color"s, "Color of lit segments"s, 0x000000, 0xFFFFFF, 0x00FFFFFF);
PropInt(Alpha, Profile7Unlit, "Unlit Color"s, "Color of unlit segments"s, 0x000000, 0xFFFFFF, 0x404040);
PropFloat(Alpha, Profile7Brightness, "Brightness"s, "Brightness level"s, 0.001f, 100.f, 5.f);
PropFloat(Alpha, Profile7DiffuseGlow, "Diffuse Glow"s, "Glow of segments on the back of the display (between segments, against glass)"s, 0.f, 10.f, 1.f);
// Generic Led
PropInt(Alpha, Profile8Color, "Color"s, "Color of lit segments"s, 0x000000, 0xFFFFFF, 0x00FFFFFF);
PropInt(Alpha, Profile8Unlit, "Unlit Color"s, "Color of unlit segments"s, 0x000000, 0xFFFFFF, 0x404040);
PropFloat(Alpha, Profile8Brightness, "Brightness"s, "Brightness level"s, 0.001f, 100.f, 5.f);
PropFloat(Alpha, Profile8DiffuseGlow, "Diffuse Glow"s, "Glow of segments on the back of the display (between segments, against glass)"s, 0.f, 10.f, 1.f);
// Array access
PropArray(Alpha, ProfileColor, int, Int, Int, m_propAlpha_Profile1Color, m_propAlpha_Profile2Color, m_propAlpha_Profile3Color, m_propAlpha_Profile4Color, m_propAlpha_Profile5Color,
   m_propAlpha_Profile6Color, m_propAlpha_Profile7Color, m_propAlpha_Profile8Color);
PropArray(Alpha, ProfileUnlit, int, Int, Int, m_propAlpha_Profile1Unlit, m_propAlpha_Profile2Unlit, m_propAlpha_Profile3Unlit, m_propAlpha_Profile4Unlit, m_propAlpha_Profile5Unlit,
   m_propAlpha_Profile6Unlit, m_propAlpha_Profile7Unlit, m_propAlpha_Profile8Unlit);
PropArray(Alpha, ProfileBrightness, float, Float, Float, m_propAlpha_Profile1Brightness, m_propAlpha_Profile2Brightness, m_propAlpha_Profile3Brightness, m_propAlpha_Profile4Brightness,
   m_propAlpha_Profile5Brightness, m_propAlpha_Profile6Brightness, m_propAlpha_Profile7Brightness, m_propAlpha_Profile8Brightness);
PropArray(Alpha, ProfileDiffuseGlow, float, Float, Float, m_propAlpha_Profile1DiffuseGlow, m_propAlpha_Profile2DiffuseGlow, m_propAlpha_Profile3DiffuseGlow, m_propAlpha_Profile4DiffuseGlow,
   m_propAlpha_Profile5DiffuseGlow, m_propAlpha_Profile6DiffuseGlow, m_propAlpha_Profile7DiffuseGlow, m_propAlpha_Profile8DiffuseGlow);

// Recent directory
PropStringDyn(RecentDir, ImportDir, "Import directory"s, ""s, ""s);
PropStringDyn(RecentDir, LoadDir, "Table directory"s, ""s, ""s);
PropStringDyn(RecentDir, FontDir, "Font directory"s, ""s, ""s);
PropStringDyn(RecentDir, PhysicsDir, "Physics directory"s, ""s, ""s);
PropStringDyn(RecentDir, ImageDir, "Image directory"s, ""s, ""s);
PropStringDyn(RecentDir, MaterialDir, "Material directory"s, ""s, ""s);
PropStringDyn(RecentDir, SoundDir, "Sound directory"s, ""s, ""s);
PropStringDyn(RecentDir, POVDir, "POV directory"s, ""s, ""s);
PropString(RecentDir, TableFileName0, ""s, ""s, ""s);
PropString(RecentDir, TableFileName1, ""s, ""s, ""s);
PropString(RecentDir, TableFileName2, ""s, ""s, ""s);
PropString(RecentDir, TableFileName3, ""s, ""s, ""s);
PropString(RecentDir, TableFileName4, ""s, ""s, ""s);
PropString(RecentDir, TableFileName5, ""s, ""s, ""s);
PropString(RecentDir, TableFileName6, ""s, ""s, ""s);
PropString(RecentDir, TableFileName7, ""s, ""s, ""s);
PropArray(RecentDir, TableFileName, string, String, String, m_propRecentDir_TableFileName0, m_propRecentDir_TableFileName1, m_propRecentDir_TableFileName2, m_propRecentDir_TableFileName3,
   m_propRecentDir_TableFileName4, m_propRecentDir_TableFileName5, m_propRecentDir_TableFileName6, m_propRecentDir_TableFileName7);

// Controller: legacy VPinMAME B2S/DOF settings, exposed in VPX UI, then provided through Windows registry (replaced by plugin settings)
PropEnum(Controller, DOFContactors, "DOFContactors"s, ""s, int, 0, "Sound FX"s, "DOF"s, "Both"s);
PropEnum(Controller, DOFKnocker, "DOFKnocker"s, ""s, int, 0, "Sound FX"s, "DOF"s, "Both"s);
PropEnum(Controller, DOFChimes, "DOFChimes"s, ""s, int, 0, "Sound FX"s, "DOF"s, "Both"s);
PropEnum(Controller, DOFBell, "DOFBell"s, ""s, int, 0, "Sound FX"s, "DOF"s, "Both"s);
PropEnum(Controller, DOFGear, "DOFGear"s, ""s, int, 0, "Sound FX"s, "DOF"s, "Both"s);
PropEnum(Controller, DOFShaker, "DOFShaker"s, ""s, int, 0, "Sound FX"s, "DOF"s, "Both"s);
PropEnum(Controller, DOFFlippers, "DOFFlippers"s, ""s, int, 0, "Sound FX"s, "DOF"s, "Both"s);
PropEnum(Controller, DOFTargets, "DOFTargets"s, ""s, int, 0, "Sound FX"s, "DOF"s, "Both"s);
PropEnum(Controller, DOFDropTargets, "DOFDropTargets"s, ""s, int, 0, "Sound FX"s, "DOF"s, "Both"s);
PropBool(Controller, ForceDisableB2S, "ForceDisableB2S"s, ""s, false);

// Parts Defaults: Balls
PropFloat(DefaultPropsBall, Mass, "Ball Mass"s, ""s, 0.1f, 2.f, 1.f);
PropFloat(DefaultPropsBall, Radius, "Ball Radius"s, ""s, 0.1f, 50.f, 25.f);
PropBool(DefaultPropsBall, ForceReflection, "Force Reflection"s, ""s, false);
PropBool(DefaultPropsBall, DecalMode, "Decal Mode"s, ""s, false);
PropString(DefaultPropsBall, Image, "Ball Image"s, ""s, ""s);
PropString(DefaultPropsBall, DecalImage, "Decal Image"s, ""s, ""s);
PropFloat(DefaultPropsBall, BulbIntensityScale, "Bulb Reflection Intensity Scale"s, ""s, 0.f, 10.f, 1.f);
PropFloat(DefaultPropsBall, PFReflStrength, "Playfield Reflection Strength"s, ""s, 0.f, 10.f, 1.f);
PropInt(DefaultPropsBall, Color, "Color"s, ""s, 0x000000, 0xFFFFFF, 0xFFFFFF);
PropBool(DefaultPropsBall, SphereMap, "Use Sphere Mapping"s, "Use sphere mapped 3D texturing"s, true);
PropBool(DefaultPropsBall, ReflectionEnabled, "Reflection Enabled"s, ""s, true);
PropBool(DefaultPropsBall, TimerEnabled, "Timer Enabled"s, ""s, false);
PropInt(DefaultPropsBall, TimerInterval, "Timer Interval"s, ""s, -2, 10000, 100);

// Parts Defaults: Bumper
PropFloatUnbounded(DefaultPropsBumper, Radius, "Radius"s, ""s, 45.f);
PropFloatUnbounded(DefaultPropsBumper, HeightScale, "HeightScale"s, ""s, 90.f);
PropFloatUnbounded(DefaultPropsBumper, RingSpeed, "RingSpeed"s, ""s, 0.5f);
PropFloatUnbounded(DefaultPropsBumper, Orientation, "Orientation"s, ""s, 0.f);
PropFloatUnbounded(DefaultPropsBumper, Threshold, "Threshold"s, ""s, 1.f);
PropString(DefaultPropsBumper, Surface, "Surface"s, ""s, ""s);
PropBool(DefaultPropsBumper, CapVisible, "CapVisible"s, ""s, true);
PropBool(DefaultPropsBumper, BaseVisible, "BaseVisible"s, ""s, true);
PropBool(DefaultPropsBumper, RingVisible, "RingVisible"s, ""s, true);
PropBool(DefaultPropsBumper, SkirtVisible, "SkirtVisible"s, ""s, true);
PropBool(DefaultPropsBumper, HasHitEvent, "HasHitEvent"s, ""s, true);
PropBool(DefaultPropsBumper, Collidable, "Collidable"s, ""s, true);
PropFloatUnbounded(DefaultPropsBumper, Force, "Force"s, ""s, 15.f);
PropFloatUnbounded(DefaultPropsBumper, Scatter, "Scatter"s, ""s, 0.f);
PropBool(DefaultPropsBumper, ReflectionEnabled, "Reflection Enabled"s, ""s, true);
PropBool(DefaultPropsBumper, TimerEnabled, "Timer Enabled"s, ""s, false);
PropInt(DefaultPropsBumper, TimerInterval, "Timer Interval"s, ""s, -2, 10000, 100);

// Parts Defaults: Decal
PropFloatUnbounded(DefaultPropsDecal, Width, "Width"s, ""s, 100.f);
PropFloatUnbounded(DefaultPropsDecal, Height, "Height"s, ""s, 100.f);
PropFloatUnbounded(DefaultPropsDecal, Rotation, "Rotation"s, ""s, 0.f);
PropString(DefaultPropsDecal, Image, "Image"s, ""s, ""s);
PropString(DefaultPropsDecal, Surface, "Surface"s, ""s, ""s);
PropEnum(DefaultPropsDecal, DecalType, "DecalType"s, ""s, DecalType, DecalImage, "DecalText"s, "DecalImage"s);
PropString(DefaultPropsDecal, Text, "Text"s, ""s, ""s);
PropEnum(DefaultPropsDecal, Sizing, "Sizing"s, ""s, SizingType, ManualSize, "AutoSize"s, "AutoWidth"s, "ManualSize"s);
PropIntUnbounded(DefaultPropsDecal, Color, "Color"s, ""s, 0x000000);
PropBool(DefaultPropsDecal, VerticalText, "VerticalText"s, ""s, false);
PropFloatUnbounded(DefaultPropsDecal, FontSize, "FontSize"s, ""s, 14.25f);
PropString(DefaultPropsDecal, FontName, "FontName"s, ""s, "Arial Black"s);
PropInt(DefaultPropsDecal, FontWeight, "FontWeight"s, ""s, 0, 900, 400); // FW_NORMAL
PropIntUnbounded(DefaultPropsDecal, FontCharSet, "FontCharSet"s, ""s, 0);
PropBool(DefaultPropsDecal, FontItalic, "FontItalic"s, ""s, false);
PropBool(DefaultPropsDecal, FontUnderline, "FontUnderline"s, ""s, false);
PropBool(DefaultPropsDecal, FontStrikeThrough, "FontStrikeThrough"s, ""s, false);

// Parts Defaults: DispReel
PropString(DefaultPropsDispReel, Image, "Image"s, ""s, ""s);
PropString(DefaultPropsDispReel, Sound, "Sound"s, ""s, ""s);
PropBool(DefaultPropsDispReel, UseImageGrid, "UseImageGrid"s, ""s, false);
PropBool(DefaultPropsDispReel, Visible, "Visible"s, ""s, true);
PropIntUnbounded(DefaultPropsDispReel, ImagesPerRow, "ImagesPerRow"s, ""s, 1);
PropBool(DefaultPropsDispReel, Transparent, "Transparent"s, ""s, false);
PropIntUnbounded(DefaultPropsDispReel, ReelCount, "ReelCount"s, ""s, 5);
PropFloatUnbounded(DefaultPropsDispReel, Width, "Width"s, ""s, 30.f);
PropFloatUnbounded(DefaultPropsDispReel, Height, "Height"s, ""s, 40.f);
PropFloatUnbounded(DefaultPropsDispReel, ReelSpacing, "ReelSpacing"s, ""s, 4.f);
PropIntUnbounded(DefaultPropsDispReel, MotorSteps, "MotorSteps"s, ""s, 2);
PropIntUnbounded(DefaultPropsDispReel, DigitRange, "DigitRange"s, ""s, 9);
PropIntUnbounded(DefaultPropsDispReel, UpdateInterval, "UpdateInterval"s, ""s, 50);
PropInt(DefaultPropsDispReel, BackColor, "BackColor"s, ""s, 0x000000, 0xFFFFFF, RGB(64, 64, 64));
PropBool(DefaultPropsDispReel, TimerEnabled, "Timer Enabled"s, ""s, false);
PropInt(DefaultPropsDispReel, TimerInterval, "Timer Interval"s, ""s, -2, 10000, 100);

// Parts Defaults: Flasher
PropFloatUnbounded(DefaultPropsFlasher, Height, "Height"s, ""s, 50.f);
PropFloatUnbounded(DefaultPropsFlasher, RotX, "RotX"s, ""s, 0.f);
PropFloatUnbounded(DefaultPropsFlasher, RotY, "RotY"s, ""s, 0.f);
PropFloatUnbounded(DefaultPropsFlasher, RotZ, "RotZ"s, ""s, 0.f);
PropInt(DefaultPropsFlasher, Color, "Color"s, ""s, 0x000000, 0xFFFFFF, RGB(50, 200, 50));
PropString(DefaultPropsFlasher, ImageA, "ImageA"s, ""s, ""s);
PropString(DefaultPropsFlasher, ImageB, "ImageB"s, ""s, ""s);
PropIntUnbounded(DefaultPropsFlasher, Opacity, "Opacity"s, ""s, 100);
PropFloatUnbounded(DefaultPropsFlasher, ModulateVsAdd, "ModulateVsAdd"s, ""s, 0.9f);
PropIntUnbounded(DefaultPropsFlasher, FilterAmount, "FilterAmount"s, ""s, 100);
PropBool(DefaultPropsFlasher, Visible, "Visible"s, ""s, true);
PropBool(DefaultPropsFlasher, AddBlend, "AddBlend"s, ""s, false);
PropEnum(DefaultPropsFlasher, RenderMode, "RenderMode"s, ""s, int, 0, "FLASHER"s, "DMD"s, "DISPLAY"s, "ALPHASEG"s);
PropBool(DefaultPropsFlasher, DisplayTexture, "DisplayTexture"s, ""s, false);
PropEnum(DefaultPropsFlasher, ImageMode, "ImageMode"s, ""s, RampImageAlignment, ImageModeWrap, "ImageModeWorld"s, "ImageModeWrap"s);
PropEnum(DefaultPropsFlasher, Filter, "Filter"s, ""s, Filters, Filter_Overlay, "Filter_None"s, "Filter_Additive"s, "Filter_Overlay"s, "Filter_Multiply"s, "Filter_Screen"s);
PropBool(DefaultPropsFlasher, TimerEnabled, "Timer Enabled"s, ""s, false);
PropInt(DefaultPropsFlasher, TimerInterval, "Timer Interval"s, ""s, -2, 10000, 100);

// Parts Defaults: Flipper
PropFloatUnbounded(DefaultPropsFlipper, StartAngle, "StartAngle"s, ""s, 121.f);
PropFloatUnbounded(DefaultPropsFlipper, EndAngle, "EndAngle"s, ""s, 70.f);
PropFloatUnbounded(DefaultPropsFlipper, BaseRadius, "BaseRadius"s, ""s, 21.5f); // 15
PropFloatUnbounded(DefaultPropsFlipper, EndRadius, "EndRadius"s, ""s, 13.f); // 6
PropFloatUnbounded(DefaultPropsFlipper, Length, "Length"s, ""s, 130.f); // 80
PropFloatUnbounded(DefaultPropsFlipper, MaxDifLength, "MaxDifLength"s, ""s, 0.f);
PropIntUnbounded(DefaultPropsFlipper, Color, "Color"s, ""s, (int)RGB(255, 255, 255));
PropIntUnbounded(DefaultPropsFlipper, RubberColor, "RubberColor"s, ""s, (int)RGB(128, 50, 50));
PropString(DefaultPropsFlipper, Surface, "Surface"s, ""s, ""s);
PropFloatUnbounded(DefaultPropsFlipper, Height, "Height"s, ""s, 50.f);
PropFloatUnbounded(DefaultPropsFlipper, RubberThickness, "RubberThickness"s, ""s, 7.f);
PropFloatUnbounded(DefaultPropsFlipper, RubberHeight, "RubberHeight"s, ""s, 19.f);
PropFloatUnbounded(DefaultPropsFlipper, RubberWidth, "RubberWidth"s, ""s, 24.f);
PropBool(DefaultPropsFlipper, Visible, "Visible"s, ""s, true);
PropBool(DefaultPropsFlipper, Enabled, "Enabled"s, ""s, true);
PropFloatUnbounded(DefaultPropsFlipper, Scatter, "Scatter"s, ""s, 0.f);
PropFloatUnbounded(DefaultPropsFlipper, Strength, "Strength"s, ""s, 2200.f);
PropFloatUnbounded(DefaultPropsFlipper, EOSTorque, "EOSTorque"s, ""s, 0.75f);
PropFloatUnbounded(DefaultPropsFlipper, EOSTorqueAngle, "EOSTorqueAngle"s, ""s, 6.f);
PropFloatUnbounded(DefaultPropsFlipper, ReturnStrength, "ReturnStrength"s, ""s, 0.058f);
PropFloatUnbounded(DefaultPropsFlipper, Mass, "Mass"s, ""s, 1.f); // previously Mass was called Speed
PropFloatUnbounded(DefaultPropsFlipper, Elasticity, "Elasticity"s, ""s, 0.8f);
PropFloatUnbounded(DefaultPropsFlipper, ElasticityFalloff, "ElasticityFalloff"s, ""s, 0.43f);
PropFloatUnbounded(DefaultPropsFlipper, Friction, "Friction"s, ""s, 0.6f);
PropFloatUnbounded(DefaultPropsFlipper, RampUp, "RampUp"s, ""s, 3.0f);
PropBool(DefaultPropsFlipper, ReflectionEnabled, "Reflection Enabled"s, ""s, true);
PropBool(DefaultPropsFlipper, TimerEnabled, "Timer Enabled"s, ""s, false);
PropInt(DefaultPropsFlipper, TimerInterval, "Timer Interval"s, ""s, -2, 10000, 100);

// Parts Defaults: Gate
PropFloatUnbounded(DefaultPropsGate, Length, "Length"s, ""s, 100.f);
PropFloatUnbounded(DefaultPropsGate, Height, "Height"s, ""s, 50.f);
PropFloatUnbounded(DefaultPropsGate, Rotation, "Rotation"s, ""s, -90.f);
PropBool(DefaultPropsGate, ShowBracket, "ShowBracket"s, ""s, true);
PropEnum1(DefaultPropsGate, GateType, "GateType"s, ""s, GateType, GateWireW, "GateWireW"s, "GateWireRectangle"s, "GatePlate"s, "GateLongPlate"s);
PropBool(DefaultPropsGate, Collidable, "Collidable"s, ""s, true);
PropFloatUnbounded(DefaultPropsGate, AngleMin, "AngleMin"s, ""s, 0.f);
PropFloatUnbounded(DefaultPropsGate, AngleMax, "AngleMax"s, ""s, (float)(M_PI / 2.0));
PropBool(DefaultPropsGate, Visible, "Visible"s, ""s, true);
PropString(DefaultPropsGate, Surface, "Surface"s, ""s, ""s);
PropBool(DefaultPropsGate, TwoWay, "TwoWay"s, ""s, true);
PropFloatUnbounded(DefaultPropsGate, Elasticity, "Elasticity"s, ""s, 0.3f);
PropFloatUnbounded(DefaultPropsGate, Friction, "Friction"s, ""s, 0.02f);
PropFloatUnbounded(DefaultPropsGate, AntiFriction, "AntiFriction"s, ""s, 0.985f);
PropFloatUnbounded(DefaultPropsGate, Scatter, "Scatter"s, ""s, 0.f);
PropFloatUnbounded(DefaultPropsGate, GravityFactor, "GravityFactor"s, ""s, 0.25f);
PropBool(DefaultPropsGate, ReflectionEnabled, "Reflection Enabled"s, ""s, true);
PropBool(DefaultPropsGate, TimerEnabled, "Timer Enabled"s, ""s, false);
PropInt(DefaultPropsGate, TimerInterval, "Timer Interval"s, ""s, -2, 10000, 100);

// Parts Defaults: HitTarget
PropBool(DefaultPropsHitTarget, LegacyMode, "LegacyMode"s, ""s, false);
PropBool(DefaultPropsHitTarget, HitEvent, "HitEvent"s, ""s, true);
PropBool(DefaultPropsHitTarget, Visible, "Visible"s, ""s, true);
PropBool(DefaultPropsHitTarget, IsDropped, "IsDropped"s, ""s, false);
PropFloatUnbounded(DefaultPropsHitTarget, Position_Z, "Position_Z"s, ""s, 0.f);
PropFloatUnbounded(DefaultPropsHitTarget, ScaleX, "ScaleX"s, ""s, 32.f);
PropFloatUnbounded(DefaultPropsHitTarget, ScaleY, "ScaleY"s, ""s, 32.f);
PropFloatUnbounded(DefaultPropsHitTarget, ScaleZ, "ScaleZ"s, ""s, 32.f);
PropFloatUnbounded(DefaultPropsHitTarget, Orientation, "Orientation"s, ""s, 0.f);
PropString(DefaultPropsHitTarget, Image, "Image"s, ""s, ""s);
PropEnum1(DefaultPropsHitTarget, TargetType, "TargetType"s, ""s, TargetType, DropTargetSimple, "DropTargetBeveled"s, "DropTargetSimple"s, "HitTargetRound"s, "HitTargetRectangle"s,
   "HitFatTargetRectangle"s, "HitFatTargetSquare"s, "DropTargetFlatSimple"s, "HitFatTargetSlim"s, "HitTargetSlim"s);
PropFloatUnbounded(DefaultPropsHitTarget, HitThreshold, "HitThreshold"s, ""s, 2.f);
PropFloatUnbounded(DefaultPropsHitTarget, DropSpeed, "DropSpeed"s, ""s, 0.2f); // Default should depend on target type: 0.5 for beveled, simple & flat simple, 0.2 otherwise
PropBool(DefaultPropsHitTarget, Collidable, "Collidable"s, ""s, true);
PropFloatUnbounded(DefaultPropsHitTarget, DisableLighting, "DisableLighting"s, ""s, 0.f);
PropFloatUnbounded(DefaultPropsHitTarget, DisableLightingBelow, "DisableLightingBelow"s, ""s, 1.f);
PropIntUnbounded(DefaultPropsHitTarget, RaiseDelay, "RaiseDelay"s, ""s, 100);
PropFloatUnbounded(DefaultPropsHitTarget, Elasticity, "Elasticity"s, ""s, 0.35f);
PropFloatUnbounded(DefaultPropsHitTarget, ElasticityFalloff, "ElasticityFalloff"s, ""s, 0.5f);
PropFloatUnbounded(DefaultPropsHitTarget, Friction, "Friction"s, ""s, 0.2f);
PropFloatUnbounded(DefaultPropsHitTarget, Scatter, "Scatter"s, ""s, 5.f);
PropBool(DefaultPropsHitTarget, ReflectionEnabled, "Reflection Enabled"s, ""s, true);
PropBool(DefaultPropsHitTarget, TimerEnabled, "Timer Enabled"s, ""s, false);
PropInt(DefaultPropsHitTarget, TimerInterval, "Timer Interval"s, ""s, -2, 10000, 100);

// Parts Defaults: Kicker
PropFloatUnbounded(DefaultPropsKicker, Radius, "Radius"s, ""s, 25.f);
PropBool(DefaultPropsKicker, Enabled, "Enabled"s, ""s, true);
PropFloatUnbounded(DefaultPropsKicker, HitAccuracy, "HitAccuracy"s, ""s, 0.5f);
PropFloatUnbounded(DefaultPropsKicker, HitHeight, "HitHeight"s, ""s, 35.f);
PropFloatUnbounded(DefaultPropsKicker, Orientation, "Orientation"s, ""s, 0.f);
PropString(DefaultPropsKicker, Surface, "Surface"s, ""s, ""s);
PropEnum(DefaultPropsKicker, KickerType, "KickerType"s, ""s, KickerType, KickerHole, "KickerInvisible"s, "KickerHole"s, "KickerCup"s, "KickerHoleSimple"s, "KickerWilliams"s,
   "KickerGottlieb"s, "KickerCup2"s);
PropBool(DefaultPropsKicker, FallThrough, "FallThrough"s, ""s, false);
PropBool(DefaultPropsKicker, Legacy, "Legacy"s, ""s, true);
PropFloatUnbounded(DefaultPropsKicker, Scatter, "Scatter"s, ""s, 0.f);
PropBool(DefaultPropsKicker, TimerEnabled, "Timer Enabled"s, ""s, false);
PropInt(DefaultPropsKicker, TimerInterval, "Timer Interval"s, ""s, -2, 10000, 100);

// Parts Defaults: Light
PropFloatUnbounded(DefaultPropsLight, Falloff, "Falloff"s, ""s, 50.f);
PropFloatUnbounded(DefaultPropsLight, FalloffPower, "FalloffPower"s, ""s, 2.f);
PropFloatUnbounded(DefaultPropsLight, LightState, "LightState"s, ""s, 0.f);
PropInt(DefaultPropsLight, Color, "Color"s, ""s, 0x000000, 0xFFFFFF, RGB(255, 169, 87)); // Default to 2700K incandescent bulb
PropInt(DefaultPropsLight, ColorFull, "ColorFull"s, ""s, 0x000000, 0xFFFFFF, RGB(255, 169, 87)); // Default to 2700K incandescent bulb (burst is useless since VPX is HDR)
PropString(DefaultPropsLight, OffImage, "OffImage"s, ""s, ""s);
PropString(DefaultPropsLight, BlinkPattern, "BlinkPattern"s, ""s, "10"s);
PropIntUnbounded(DefaultPropsLight, BlinkInterval, "BlinkInterval"s, ""s, 125);
PropFloatUnbounded(DefaultPropsLight, Intensity, "Intensity"s, ""s, 10.f);
PropFloatUnbounded(DefaultPropsLight, TransmissionScale, "TransmissionScale"s, ""s, 0.f); // FIXME used to be 0.5 as a default unless from click, then 0
PropString(DefaultPropsLight, Surface, "Surface"s, ""s, ""s);
PropFloatUnbounded(DefaultPropsLight, FadeSpeedUp, "FadeSpeedUp"s, ""s, 10.f * (float)(1.0 / 200.0)); // Default: 200ms up (slow incandescent bulb)
PropFloatUnbounded(DefaultPropsLight, FadeSpeedDown, "FadeSpeedDown"s, ""s, 10.f * (float)(1.0 / 500.0)); // Default: 500ms down (slow incandescent bulb)
PropBool(DefaultPropsLight, Bulb, "Bulb"s, ""s, false);
PropBool(DefaultPropsLight, ImageMode, "ImageMode"s, ""s, false);
PropBool(DefaultPropsLight, ShowBulbMesh, "ShowBulbMesh"s, ""s, false);
PropBool(DefaultPropsLight, StaticBulbMesh, "StaticBulbMesh"s, ""s, true);
PropBool(DefaultPropsLight, ShowReflectionOnBall, "ShowReflectionOnBall"s, ""s, true);
PropFloatUnbounded(DefaultPropsLight, ScaleBulbMesh, "ScaleBulbMesh"s, ""s, 20.f);
PropFloatUnbounded(DefaultPropsLight, BulbModulateVsAdd, "BulbModulateVsAdd"s, ""s, 0.9f);
PropFloatUnbounded(DefaultPropsLight, BulbHaloHeight, "BulbHaloHeight"s, ""s, 28.f);
PropBool(DefaultPropsLight, ReflectionEnabled, "Reflection Enabled"s, ""s, true);
PropBool(DefaultPropsLight, TimerEnabled, "Timer Enabled"s, ""s, false);
PropInt(DefaultPropsLight, TimerInterval, "Timer Interval"s, ""s, -2, 10000, 100);

// Parts Defaults: LightSeq
PropIntUnbounded(DefaultPropsLightSeq, UpdateInterval, "UpdateInterval"s, ""s, 25);
PropString(DefaultPropsLightSeq, Collection, "Collection"s, ""s, ""s);
PropFloatUnbounded(DefaultPropsLightSeq, CenterX, "CenterX"s, ""s, (float)(EDITOR_BG_WIDTH / 2));
PropFloatUnbounded(DefaultPropsLightSeq, CenterY, "CenterY"s, ""s, (float)((2 * EDITOR_BG_WIDTH) / 2));
PropBool(DefaultPropsLightSeq, TimerEnabled, "Timer Enabled"s, ""s, false);
PropInt(DefaultPropsLightSeq, TimerInterval, "Timer Interval"s, ""s, -2, 10000, 100);

// Parts Defaults: PartGroup
PropBool(DefaultPropsPartGroup, TimerEnabled, "Timer Enabled"s, ""s, false);
PropInt(DefaultPropsPartGroup, TimerInterval, "Timer Interval"s, ""s, -2, 10000, 100);

// Parts Defaults: Plunger
PropFloatUnbounded(DefaultPropsPlunger, Height, "Height"s, ""s, 20.f);
PropFloatUnbounded(DefaultPropsPlunger, Width, "Width"s, ""s, 25.f);
PropFloatUnbounded(DefaultPropsPlunger, ZAdjust, "ZAdjust"s, ""s, 0.f);
PropFloatUnbounded(DefaultPropsPlunger, Stroke, "Stroke"s, ""s, 80.f);
PropFloatUnbounded(DefaultPropsPlunger, PullSpeed, "PullSpeed"s, ""s, 5.f);
PropEnum1(DefaultPropsPlunger, PlungerType, "PlungerType"s, ""s, PlungerType, PlungerTypeModern, "PlungerTypeModern"s, "PlungerTypeFlat"s, "PlungerTypeCustom"s);
PropInt(DefaultPropsPlunger, Color, "Color"s, ""s, 0x000000, 0xFFFFFF, RGB(76, 76, 76));
PropString(DefaultPropsPlunger, Image, "Image"s, ""s, ""s);
PropIntUnbounded(DefaultPropsPlunger, AnimFrames, "AnimFrames"s, ""s, 1);
PropString(DefaultPropsPlunger, Surface, "Surface"s, ""s, ""s);
PropBool(DefaultPropsPlunger, MechPlunger, "MechPlunger"s, ""s, false); // plungers require selection for mechanical input
PropBool(DefaultPropsPlunger, AutoPlunger, "AutoPlunger"s, ""s, false);
PropBool(DefaultPropsPlunger, Visible, "Visible"s, ""s, true);
PropString(DefaultPropsPlunger, CustomTipShape, "CustomTipShape"s, ""s, "0 .34; 2 .6; 3 .64; 5 .7; 7 .84; 8 .88; 9 .9; 11 .92; 14 .92; 39 .84"s);
PropFloatUnbounded(DefaultPropsPlunger, CustomRodDiam, "CustomRodDiam"s, ""s, 0.60f);
PropFloatUnbounded(DefaultPropsPlunger, CustomRingGap, "CustomRingGap"s, ""s, 2.0f);
PropFloatUnbounded(DefaultPropsPlunger, CustomRingDiam, "CustomRingDiam"s, ""s, 0.94f);
PropFloatUnbounded(DefaultPropsPlunger, CustomRingWidth, "CustomRingWidth"s, ""s, 3.0f);
PropFloatUnbounded(DefaultPropsPlunger, CustomSpringDiam, "CustomSpringDiam"s, ""s, 0.77f);
PropFloatUnbounded(DefaultPropsPlunger, CustomSpringGauge, "CustomSpringGauge"s, ""s, 1.38f);
PropFloatUnbounded(DefaultPropsPlunger, CustomSpringLoops, "CustomSpringLoops"s, ""s, 8.0f);
PropFloatUnbounded(DefaultPropsPlunger, CustomSpringEndLoops, "CustomSpringEndLoops"s, ""s, 2.5f);
PropFloatUnbounded(DefaultPropsPlunger, ReleaseSpeed, "ReleaseSpeed"s, ""s, 80.f);
PropFloatUnbounded(DefaultPropsPlunger, MechStrength, "MechStrength"s, ""s, 85.f);
PropFloatUnbounded(DefaultPropsPlunger, ParkPosition, "ParkPosition"s, ""s,
   (float)(0.5 / 3.0)); // typical mechanical plunger has 3 inch stroke and 0.5 inch rest position //!! 0.01f better for some HW-plungers, but this seems to be rather a firmware/config issue
PropFloatUnbounded(DefaultPropsPlunger, ScatterVelocity, "ScatterVelocity"s, ""s, 0.f);
PropFloatUnbounded(DefaultPropsPlunger, MomentumXfer, "MomentumXfer"s, ""s, 1.f);
PropBool(DefaultPropsPlunger, ReflectionEnabled, "Reflection Enabled"s, ""s, true);
PropBool(DefaultPropsPlunger, TimerEnabled, "Timer Enabled"s, ""s, false);
PropInt(DefaultPropsPlunger, TimerInterval, "Timer Interval"s, ""s, -2, 10000, 100);

// Parts Defaults: Primitive
PropInt(DefaultPropsPrimitive, Sides, "Sides"s, ""s, 0, 100 /* Max_Primitive_Sides */, 4);
PropInt(DefaultPropsPrimitive, SideColor, "SideColor"s, ""s, 0x000000, 0xFFFFFF, RGB(150, 150, 150));
PropBool(DefaultPropsPrimitive, Visible, "Visible"s, ""s, true);
PropBool(DefaultPropsPrimitive, StaticRendering, "StaticRendering"s, ""s, true);
PropBool(DefaultPropsPrimitive, DrawTexturesInside, "DrawTexturesInside"s, ""s, false);
PropFloatUnbounded(DefaultPropsPrimitive, Position_Z, "Position_Z"s, ""s, 0.f);
PropFloatUnbounded(DefaultPropsPrimitive, Size_X, "Size_X"s, ""s, 100.f);
PropFloatUnbounded(DefaultPropsPrimitive, Size_Y, "Size_Y"s, ""s, 100.f);
PropFloatUnbounded(DefaultPropsPrimitive, Size_Z, "Size_Z"s, ""s, 100.f);
PropFloatUnbounded(DefaultPropsPrimitive, RotAndTra0, "RotAndTra0"s, ""s, 0.f);
PropFloatUnbounded(DefaultPropsPrimitive, RotAndTra1, "RotAndTra1"s, ""s, 0.f);
PropFloatUnbounded(DefaultPropsPrimitive, RotAndTra2, "RotAndTra2"s, ""s, 0.f);
PropFloatUnbounded(DefaultPropsPrimitive, RotAndTra3, "RotAndTra3"s, ""s, 0.f);
PropFloatUnbounded(DefaultPropsPrimitive, RotAndTra4, "RotAndTra4"s, ""s, 0.f);
PropFloatUnbounded(DefaultPropsPrimitive, RotAndTra5, "RotAndTra5"s, ""s, 0.f);
PropFloatUnbounded(DefaultPropsPrimitive, RotAndTra6, "RotAndTra6"s, ""s, 0.f);
PropFloatUnbounded(DefaultPropsPrimitive, RotAndTra7, "RotAndTra7"s, ""s, 0.f);
PropFloatUnbounded(DefaultPropsPrimitive, RotAndTra8, "RotAndTra8"s, ""s, 0.f);
PropString(DefaultPropsPrimitive, Image, "Image"s, ""s, ""s);
PropString(DefaultPropsPrimitive, NormalMap, "NormalMap"s, ""s, ""s);
PropBool(DefaultPropsPrimitive, HitEvent, "HitEvent"s, ""s, false);
PropFloatUnbounded(DefaultPropsPrimitive, HitThreshold, "HitThreshold"s, ""s, 2.f);
PropFloatUnbounded(DefaultPropsPrimitive, Opacity, "Opacity"s, ""s, 100.f);
PropBool(DefaultPropsPrimitive, AddBlend, "AddBlend"s, ""s, false);
PropBool(DefaultPropsPrimitive, DepthMask, "DepthMask"s, ""s, true);
PropInt(DefaultPropsPrimitive, Color, "Color"s, ""s, 0x000000, 0xFFFFFF, 0xFFFFFF);
PropFloatUnbounded(DefaultPropsPrimitive, EdgeFactorUI, "EdgeFactorUI"s, ""s, 0.25f);
PropFloatUnbounded(DefaultPropsPrimitive, CollisionReductionFactor, "CollisionReductionFactor"s, ""s, 0.f);
PropBool(DefaultPropsPrimitive, Collidable, "Collidable"s, ""s, true);
PropBool(DefaultPropsPrimitive, IsToy, "IsToy"s, ""s, false);
PropFloatUnbounded(DefaultPropsPrimitive, DisableLighting, "DisableLighting"s, ""s, 0.f);
PropFloatUnbounded(DefaultPropsPrimitive, DisableLightingBelow, "DisableLightingBelow"s, ""s, 1.f);
PropBool(DefaultPropsPrimitive, BackfacesEnabled, "BackfacesEnabled"s, ""s, false);
PropBool(DefaultPropsPrimitive, DisplayTexture, "DisplayTexture"s, ""s, false);
PropBool(DefaultPropsPrimitive, ObjectSpaceNormalMap, "ObjectSpaceNormalMap"s, ""s, false);
PropFloatUnbounded(DefaultPropsPrimitive, Elasticity, "Elasticity"s, ""s, 0.3f);
PropFloatUnbounded(DefaultPropsPrimitive, ElasticityFalloff, "ElasticityFalloff"s, ""s, 0.5f);
PropFloatUnbounded(DefaultPropsPrimitive, Friction, "Friction"s, ""s, 0.3f);
PropFloatUnbounded(DefaultPropsPrimitive, Scatter, "Scatter"s, ""s, 0.f);
PropBool(DefaultPropsPrimitive, ReflectionEnabled, "Reflection Enabled"s, ""s, true);
PropBool(DefaultPropsPrimitive, TimerEnabled, "Timer Enabled"s, ""s, false);
PropInt(DefaultPropsPrimitive, TimerInterval, "Timer Interval"s, ""s, -2, 10000, 100);

// Parts Defaults: Ramp
PropFloatUnbounded(DefaultPropsRamp, Length, "Length"s, ""s, 400.f);
PropFloatUnbounded(DefaultPropsRamp, HeightBottom, "HeightBottom"s, ""s, 0.f);
PropFloatUnbounded(DefaultPropsRamp, HeightTop, "HeightTop"s, ""s, 50.f);
PropFloatUnbounded(DefaultPropsRamp, WidthBottom, "WidthBottom"s, ""s, 75.f);
PropFloatUnbounded(DefaultPropsRamp, WidthTop, "WidthTop"s, ""s, 60.f);
PropEnum(
   DefaultPropsRamp, RampType, "RampType"s, ""s, RampType, RampTypeFlat, "RampTypeFlat"s, "RampType4Wire"s, "RampType2Wire"s, "RampType3WireLeft"s, "RampType3WireRight"s, "RampType1Wire"s);
PropString(DefaultPropsRamp, Image, "Image"s, ""s, ""s);
PropEnum(DefaultPropsRamp, ImageMode, "ImageMode"s, ""s, RampImageAlignment, ImageModeWorld, "ImageModeWorld"s, "ImageModeWrap"s);
PropBool(DefaultPropsRamp, ImageWalls, "ImageWalls"s, ""s, true);
PropFloatUnbounded(DefaultPropsRamp, LeftWallHeight, "LeftWallHeight"s, ""s, 62.f);
PropFloatUnbounded(DefaultPropsRamp, RightWallHeight, "RightWallHeight"s, ""s, 62.f);
PropFloatUnbounded(DefaultPropsRamp, LeftWallHeightVisible, "LeftWallHeightVisible"s, ""s, 30.f);
PropFloatUnbounded(DefaultPropsRamp, RightWallHeightVisible, "RightWallHeightVisible"s, ""s, 30.f);
PropBool(DefaultPropsRamp, HitEvent, "HitEvent"s, ""s, false);
PropFloatUnbounded(DefaultPropsRamp, HitThreshold, "HitThreshold"s, ""s, 2.f);
PropBool(DefaultPropsRamp, Visible, "Visible"s, ""s, true);
PropBool(DefaultPropsRamp, Collidable, "Collidable"s, ""s, true);
PropFloatUnbounded(DefaultPropsRamp, WireDiameter, "WireDiameter"s, ""s, 8.f);
PropFloatUnbounded(DefaultPropsRamp, WireDistanceX, "WireDistanceX"s, ""s, 38.f);
PropFloatUnbounded(DefaultPropsRamp, WireDistanceY, "WireDistanceY"s, ""s, 88.f);
PropFloatUnbounded(DefaultPropsRamp, Elasticity, "Elasticity"s, ""s, 0.3f);
PropFloatUnbounded(DefaultPropsRamp, Friction, "Friction"s, ""s, 0.3f);
PropFloatUnbounded(DefaultPropsRamp, Scatter, "Scatter"s, ""s, 0.f);
PropBool(DefaultPropsRamp, ReflectionEnabled, "Reflection Enabled"s, ""s, true);
PropBool(DefaultPropsRamp, TimerEnabled, "Timer Enabled"s, ""s, false);
PropInt(DefaultPropsRamp, TimerInterval, "Timer Interval"s, ""s, -2, 10000, 100);

// Parts Defaults: Rubber
PropFloatUnbounded(DefaultPropsRubber, HitHeight, "HitHeight"s, ""s, 25.f);
PropFloatUnbounded(DefaultPropsRubber, Height, "Height"s, ""s, 25.f);
PropIntUnbounded(DefaultPropsRubber, Thickness, "Thickness"s, ""s, 8);
PropString(DefaultPropsRubber, Image, "Image"s, ""s, ""s);
PropBool(DefaultPropsRubber, HitEvent, "HitEvent"s, ""s, false);
PropBool(DefaultPropsRubber, Visible, "Visible"s, ""s, true);
PropBool(DefaultPropsRubber, Collidable, "Collidable"s, ""s, true);
PropBool(DefaultPropsRubber, EnableStaticRendering, "EnableStaticRendering"s, ""s, true);
PropBool(DefaultPropsRubber, EnableShowInEditor, "EnableShowInEditor"s, ""s, false);
PropFloatUnbounded(DefaultPropsRubber, RotX, "RotX"s, ""s, 0.f);
PropFloatUnbounded(DefaultPropsRubber, RotY, "RotY"s, ""s, 0.f);
PropFloatUnbounded(DefaultPropsRubber, RotZ, "RotZ"s, ""s, 0.f);
PropBool(DefaultPropsRubber, ReflectionEnabled, "Reflection Enabled"s, ""s, true);
PropFloatUnbounded(DefaultPropsRubber, Elasticity, "Elasticity"s, ""s, 0.8f);
PropFloatUnbounded(DefaultPropsRubber, ElasticityFalloff, "ElasticityFalloff"s, ""s, 0.3f);
PropFloatUnbounded(DefaultPropsRubber, Friction, "Friction"s, ""s, 0.6f);
PropFloatUnbounded(DefaultPropsRubber, Scatter, "Scatter"s, ""s, 5.f);
PropBool(DefaultPropsRubber, TimerEnabled, "Timer Enabled"s, ""s, false);
PropInt(DefaultPropsRubber, TimerInterval, "Timer Interval"s, ""s, -2, 10000, 100);

// Parts Defaults: Spinner
PropFloatUnbounded(DefaultPropsSpinner, Length, "Length"s, ""s, 80.f);
PropFloatUnbounded(DefaultPropsSpinner, Rotation, "Rotation"s, ""s, 0.f);
PropBool(DefaultPropsSpinner, ShowBracket, "ShowBracket"s, ""s, true);
PropFloatUnbounded(DefaultPropsSpinner, Height, "Height"s, ""s, 60.f); // Note: this property used to be an int (scaled by 1000)
PropFloatUnbounded(DefaultPropsSpinner, AngleMax, "AngleMax"s, ""s, 0.f);
PropFloatUnbounded(DefaultPropsSpinner, AngleMin, "AngleMin"s, ""s, 0.3f);
PropBool(DefaultPropsSpinner, Visible, "Visible"s, ""s, true);
PropString(DefaultPropsSpinner, Image, "Image"s, ""s, ""s);
PropString(DefaultPropsSpinner, Surface, "Surface"s, ""s, ""s);
PropFloatUnbounded(DefaultPropsSpinner, Elasticity, "Elasticity"s, ""s, 0.3f);
PropFloatUnbounded(DefaultPropsSpinner, AntiFriction, "AntiFriction"s, ""s, 0.9879f);
PropBool(DefaultPropsSpinner, ReflectionEnabled, "Reflection Enabled"s, ""s, true);
PropBool(DefaultPropsSpinner, TimerEnabled, "Timer Enabled"s, ""s, false);
PropInt(DefaultPropsSpinner, TimerInterval, "Timer Interval"s, ""s, -2, 10000, 100);

// Parts Defaults: Surface => Wall or Target
PropFloatUnbounded(DefaultPropsWall, Width, "Width"s, ""s, 50.f);
PropFloatUnbounded(DefaultPropsWall, Length, "Length"s, ""s, 50.f);
PropFloatUnbounded(DefaultPropsWall, Elasticity, "Elasticity"s, ""s, 0.3f);
PropFloatUnbounded(DefaultPropsWall, ElasticityFallOff, "ElasticityFallOff"s, ""s, 0.0f);
PropFloatUnbounded(DefaultPropsWall, Friction, "Friction"s, ""s, 0.3f);
PropFloatUnbounded(DefaultPropsWall, Scatter, "Scatter"s, ""s, 0.f);
PropBool(DefaultPropsWall, HitEvent, "HitEvent"s, ""s, false);
PropFloatUnbounded(DefaultPropsWall, HitThreshold, "HitThreshold"s, ""s, 2.0f);
PropFloatUnbounded(DefaultPropsWall, SlingshotThreshold, "SlingshotThreshold"s, ""s, 0.0f);
PropString(DefaultPropsWall, TopImage, "TopImage"s, ""s, ""s);
PropString(DefaultPropsWall, SideImage, "SideImage"s, ""s, ""s);
PropBool(DefaultPropsWall, Droppable, "Droppable"s, ""s, false);
PropBool(DefaultPropsWall, Flipbook, "Flipbook"s, ""s, false);
PropBool(DefaultPropsWall, IsBottomSolid, "IsBottomSolid"s, ""s, false);
PropFloatUnbounded(DefaultPropsWall, HeightBottom, "HeightBottom"s, ""s, 0.f);
PropFloatUnbounded(DefaultPropsWall, HeightTop, "HeightTop"s, ""s, 50.f);
PropBool(DefaultPropsWall, DisplayTexture, "DisplayTexture"s, ""s, true);
PropFloatUnbounded(DefaultPropsWall, SlingshotForce, "SlingshotForce"s, ""s, 80.f);
PropBool(DefaultPropsWall, SlingshotAnimation, "SlingshotAnimation"s, ""s, true);
PropBool(DefaultPropsWall, Visible, "Visible"s, ""s, true);
PropBool(DefaultPropsWall, SideVisible, "SideVisible"s, ""s, true);
PropBool(DefaultPropsWall, Collidable, "Collidable"s, ""s, true);
PropFloatUnbounded(DefaultPropsWall, DisableLighting, "DisableLighting"s, ""s, 0.f);
PropFloatUnbounded(DefaultPropsWall, DisableLightingBelow, "DisableLightingBelow"s, ""s, 1.f);
PropBool(DefaultPropsWall, ReflectionEnabled, "Reflection Enabled"s, ""s, true);
PropBool(DefaultPropsWall, TimerEnabled, "Timer Enabled"s, ""s, false);
PropInt(DefaultPropsWall, TimerInterval, "Timer Interval"s, ""s, -2, 10000, 100);

PropFloatUnbounded(DefaultPropsTarget, Width, "Width"s, ""s, 50.f);
PropFloatUnbounded(DefaultPropsTarget, Length, "Length"s, ""s, 50.f);
PropFloatUnbounded(DefaultPropsTarget, Elasticity, "Elasticity"s, ""s, 0.3f);
PropFloatUnbounded(DefaultPropsTarget, ElasticityFallOff, "ElasticityFallOff"s, ""s, 0.0f);
PropFloatUnbounded(DefaultPropsTarget, Friction, "Friction"s, ""s, 0.3f);
PropFloatUnbounded(DefaultPropsTarget, Scatter, "Scatter"s, ""s, 0.f);
PropBool(DefaultPropsTarget, HitEvent, "HitEvent"s, ""s, false);
PropFloatUnbounded(DefaultPropsTarget, HitThreshold, "HitThreshold"s, ""s, 2.0f);
PropFloatUnbounded(DefaultPropsTarget, SlingshotThreshold, "SlingshotThreshold"s, ""s, 0.0f);
PropString(DefaultPropsTarget, TopImage, "TopImage"s, ""s, ""s);
PropString(DefaultPropsTarget, SideImage, "SideImage"s, ""s, ""s);
PropBool(DefaultPropsTarget, Droppable, "Droppable"s, ""s, false);
PropBool(DefaultPropsTarget, Flipbook, "Flipbook"s, ""s, false);
PropBool(DefaultPropsTarget, IsBottomSolid, "IsBottomSolid"s, ""s, false);
PropFloatUnbounded(DefaultPropsTarget, HeightBottom, "HeightBottom"s, ""s, 0.f);
PropFloatUnbounded(DefaultPropsTarget, HeightTop, "HeightTop"s, ""s, 50.f);
PropBool(DefaultPropsTarget, DisplayTexture, "DisplayTexture"s, ""s, true);
PropFloatUnbounded(DefaultPropsTarget, SlingshotForce, "SlingshotForce"s, ""s, 80.f);
PropBool(DefaultPropsTarget, SlingshotAnimation, "SlingshotAnimation"s, ""s, true);
PropBool(DefaultPropsTarget, Visible, "Visible"s, ""s, true);
PropBool(DefaultPropsTarget, SideVisible, "SideVisible"s, ""s, true);
PropBool(DefaultPropsTarget, Collidable, "Collidable"s, ""s, true);
PropFloatUnbounded(DefaultPropsTarget, DisableLighting, "DisableLighting"s, ""s, 0.f);
PropFloatUnbounded(DefaultPropsTarget, DisableLightingBelow, "DisableLightingBelow"s, ""s, 1.f);
PropBool(DefaultPropsTarget, ReflectionEnabled, "Reflection Enabled"s, ""s, true);
PropBool(DefaultPropsTarget, TimerEnabled, "Timer Enabled"s, ""s, false);
PropInt(DefaultPropsTarget, TimerInterval, "Timer Interval"s, ""s, -2, 10000, 100);

// Parts Defaults: Textbox
PropFloatUnbounded(DefaultPropsTextbox, Width, "Width"s, ""s, 100.f);
PropFloatUnbounded(DefaultPropsTextbox, Height, "Height"s, ""s, 50.f);
PropIntUnbounded(DefaultPropsTextbox, BackColor, "BackColor"s, ""s, 0x000000);
PropIntUnbounded(DefaultPropsTextbox, FontColor, "FontColor"s, ""s, 0xFFFFFF);
PropFloatUnbounded(DefaultPropsTextbox, IntensityScale, "IntensityScale"s, ""s, 1.f);
PropEnum(DefaultPropsTextbox, TextAlignment, "TextAlignment"s, ""s, TextAlignment, TextAlignRight, "TextAlignLeft"s, "TextAlignCenter"s, "TextAlignRight"s);
PropBool(DefaultPropsTextbox, Transparent, "Transparent"s, ""s, false);
PropBool(DefaultPropsTextbox, DMD, "DMD"s, ""s, false);
PropString(DefaultPropsTextbox, Text, "Text"s, ""s, ""s);
PropFloatUnbounded(DefaultPropsTextbox, FontSize, "FontSize"s, ""s, 14.25f);
PropString(DefaultPropsTextbox, FontName, "FontName"s, ""s, "Arial Black"s);
PropInt(DefaultPropsTextbox, FontWeight, "FontWeight"s, ""s, 0, 900, 400); // FW_NORMAL
PropIntUnbounded(DefaultPropsTextbox, FontCharSet, "FontCharSet"s, ""s, 0);
PropBool(DefaultPropsTextbox, FontItalic, "FontItalic"s, ""s, false);
PropBool(DefaultPropsTextbox, FontUnderline, "FontUnderline"s, ""s, false);
PropBool(DefaultPropsTextbox, FontStrikeThrough, "FontStrikeThrough"s, ""s, false);
PropBool(DefaultPropsTextbox, TimerEnabled, "Timer Enabled"s, ""s, false);
PropInt(DefaultPropsTextbox, TimerInterval, "Timer Interval"s, ""s, -2, 10000, 100);

// Parts Defaults: Timer
PropBool(DefaultPropsTimer, TimerEnabled, "Timer Enabled"s, ""s, false);
PropInt(DefaultPropsTimer, TimerInterval, "Timer Interval"s, ""s, -2, 10000, 100);

// Parts Defaults: Trigger
PropFloatUnbounded(DefaultPropsTrigger, Radius, "Radius"s, ""s, 25.f);
PropFloatUnbounded(DefaultPropsTrigger, Rotation, "Rotation"s, ""s, 0.f);
PropFloatUnbounded(DefaultPropsTrigger, WireThickness, "WireThickness"s, ""s, 0.f);
PropFloatUnbounded(DefaultPropsTrigger, ScaleX, "ScaleX"s, ""s, 1.f);
PropFloatUnbounded(DefaultPropsTrigger, ScaleY, "ScaleY"s, ""s, 1.f);
PropBool(DefaultPropsTrigger, Enabled, "Enabled"s, ""s, true);
PropBool(DefaultPropsTrigger, Visible, "Visible"s, ""s, true);
PropFloatUnbounded(DefaultPropsTrigger, HitHeight, "HitHeight"s, ""s, 50.f);
PropEnum(DefaultPropsTrigger, Shape, "Shape"s, ""s, TriggerShape, TriggerWireA, "None"s, "Wire A"s, "Star"s, "Wire B"s, "Button"s, "Wire C"s, "Wire D"s, "Inder"s);
PropString(DefaultPropsTrigger, Surface, "Surface"s, ""s, ""s);
PropFloatUnbounded(DefaultPropsTrigger, AnimSpeed, "AnimSpeed"s, ""s, 1.f);
PropBool(DefaultPropsTrigger, ReflectionEnabled, "Reflection Enabled"s, ""s, true);
PropBool(DefaultPropsTrigger, TimerEnabled, "Timer Enabled"s, ""s, false);
PropInt(DefaultPropsTrigger, TimerInterval, "Timer Interval"s, ""s, -2, 10000, 100);

// Default core plugins enable state
PropBoolDyn(PluginB2SLegacy, Enable, "Enable"s, "Enable legacy B2S plugin"s, g_isStandalone);
PropBoolDyn(PluginDMDUtil, Enable, "Enable"s, "Enable DMDUtil plugin"s, false);
PropBoolDyn(PluginFlexDMD, Enable, "Enable"s, "Enable FlexDMD plugin"s, g_isStandalone);
PropBoolDyn(PluginPinMAME, Enable, "Enable"s, "Enable PinMAME plugin"s, g_isStandalone);
PropBoolDyn(PluginPUP, Enable, "Enable"s, "Enable PinUp player plugin"s, g_isMobile);
PropBoolDyn(PluginScoreView, Enable, "Enable"s, "Enable ScoreView player plugin"s, g_isStandalone);
PropBoolDyn(PluginSerum, Enable, "Enable"s, "Enable Serum plugin"s, g_isStandalone);
PropBoolDyn(PluginWMP, Enable, "Enable"s, "Enable WMP plugin"s, g_isStandalone);

// Standalone
PropEnumWithMin(Standalone, RenderingModeOverride, "Override rendering mode"s, ""s, int, -1, g_isMobile ? 2 : -1, "Default"s, "2D"s, "Stereo 3D"s, "VR"s);
PropBool(Standalone, Haptics, "Haptics"s, ""s, g_isMobile);
PropBool(Standalone, ResetLogOnPlay, "Reset Log on Play"s, ""s, g_isMobile);
PropString(Standalone, VPRegPath, "VPRegPath"s, ""s, ""s);

// Editor settings
PropIntUnbounded(Editor, WindowLeft, "WindowLeft"s, "Main window left"s, -1);
PropIntUnbounded(Editor, WindowTop, "WindowTop"s, "Main window top"s, -1);
PropIntUnbounded(Editor, WindowRight, "WindowRight"s, "Main window right"s, -1);
PropIntUnbounded(Editor, WindowBottom, "WindowBottom"s, "Main window bottom"s, -1);
PropBool(Editor, WindowMaximized, "WindowMaximized"s, "Main window maximized"s, false);
PropIntUnbounded(Editor, ImageMngPosX, "ImageMngPosX"s, "Image Manager window X position"s, 0);
PropIntUnbounded(Editor, ImageMngPosY, "ImageMngPosY"s, "Image Manager window Y position"s, 0);
PropIntUnbounded(Editor, ImageMngWidth, "ImageMngWidth"s, "Image Manager window width"s, 1000);
PropIntUnbounded(Editor, ImageMngHeight, "ImageMngHeight"s, "Image Manager window height"s, 800);
PropIntUnbounded(Editor, CodeViewPosX, "CodeViewPosX"s, "Script window X position"s, 0);
PropIntUnbounded(Editor, CodeViewPosY, "CodeViewPosY"s, "Script window Y position"s, 0);
PropIntUnbounded(Editor, CodeViewPosWidth, "CodeViewPosWidth"s, "Script window width"s, 640);
PropIntUnbounded(Editor, CodeViewPosHeight, "CodeViewPosHeight"s, "Script window height"s, 490);
PropIntUnbounded(Editor, DebuggerPosX, "DebuggerPosX"s, "Debugger window X position"s, 0);
PropIntUnbounded(Editor, DebuggerPosY, "DebuggerPosY"s, "Debugger window Y position"s, 0);
PropIntUnbounded(Editor, DebuggerWidth, "DebuggerWidth"s, "Debugger window width"s, 1000);
PropIntUnbounded(Editor, DebuggerHeight, "DebuggerHeight"s, "Debugger window height"s, 800);
PropIntUnbounded(Editor, MaterialMngPosX, "MaterialMngPosX"s, "Material Manager window X position"s, 0);
PropIntUnbounded(Editor, MaterialMngPosY, "MaterialMngPosY"s, "Material Manager window Y position"s, 0);
PropIntUnbounded(Editor, MaterialMngWidth, "MaterialMngWidth"s, "Material Manager window width"s, 1000);
PropIntUnbounded(Editor, MaterialMngHeight, "MaterialMngHeight"s, "Material Manager window height"s, 800);
PropIntUnbounded(Editor, RenderProbePosX, "RenderProbePosX"s, "Render Probe Manager window X position"s, 0);
PropIntUnbounded(Editor, RenderProbePosY, "RenderProbePosY"s, "Render Probe Manager window Y position"s, 0);
PropIntUnbounded(Editor, RenderProbeWidth, "RenderProbeWidth"s, "Render Probe Manager window width"s, 1000);
PropIntUnbounded(Editor, RenderProbeHeight, "RenderProbeHeight"s, "Render Probe Manager window height"s, 800);
PropIntUnbounded(Editor, SearchSelectPosX, "SearchSelectPosX"s, "Search/Select window X position"s, 0);
PropIntUnbounded(Editor, SearchSelectPosY, "SearchSelectPosY"s, "Search/Select window Y position"s, 0);
PropIntUnbounded(Editor, SearchSelectWidth, "SearchSelectWidth"s, "Search/Select window width"s, 640);
PropIntUnbounded(Editor, SearchSelectHeight, "SearchSelectHeight"s, "Search/Select window height"s, 400);
PropIntUnbounded(Editor, SoundMngPosX, "SoundMngPosX"s, "Sound Manager window X position"s, 0);
PropIntUnbounded(Editor, SoundMngPosY, "SoundMngPosY"s, "Sound Manager window Y position"s, 0);
PropIntUnbounded(Editor, SoundMngWidth, "SoundMngWidth"s, "Sound Manager window width"s, 1000);
PropIntUnbounded(Editor, SoundMngHeight, "SoundMngHeight"s, "Sound Manager window height"s, 800);
PropIntUnbounded(Editor, WhereUsedPosX, "WhereUsedPosX"s, "Where-used window X position"s, 0);
PropIntUnbounded(Editor, WhereUsedPosY, "WhereUsedPosY"s, "Where-used window Y position"s, 0);
PropIntUnbounded(Editor, WhereUsedWidth, "WhereUsedWidth"s, "Where-used window width"s, 1000);
PropIntUnbounded(Editor, WhereUsedHeight, "WhereUsedHeight"s, "Where-used window height"s, 800);
PropIntUnbounded(Editor, CollectionMngPosX, "CollectionMngPosX"s, "Collection Manager window X position"s, 0);
PropIntUnbounded(Editor, CollectionMngPosY, "CollectionMngPosY"s, "Collection Manager window Y position"s, 0);
PropBool(Editor, SelectTableOnStart, "SelectTableOnStart"s, "Open a file/table select dialog on editor startup"s, true);
PropBool(Editor, SelectTableOnPlayerClose, "SelectTableOnPlayerClose"s, "Open a file/table select dialog on player close"s, true); // FIXME does not seem to be used anywhere
PropBool(Editor, ShowDragPoints, "ShowDragPoints"s, "Show Drag points in viewport/editor"s, false);
PropBool(Editor, DrawLightCenters, "DrawLightCenters"s, "Show Light centers in viewport/editor"s, false);
PropBool(Editor, AutoSaveOn, "AutoSaveOn"s, "Enable Autosave for table(s)"s, true);
PropIntUnbounded(Editor, AutoSaveTime, "AutoSaveTime"s, "Time for the Autosave intervals"s, 10);
PropIntUnbounded(Editor, GridSize, "GridSize"s, "Grid size in viewport/editor"s, 50);
PropBool(Editor, GroupElementsInCollection, "GroupElementsInCollection"s, "Group Elements in a collection in viewport/editor"s, true);
PropBool(Editor, LogScriptOutput, "LogScriptOutput"s, "Enable script logging output"s, true);
PropInt(Editor, DefaultMaterialColor, "DefaultMaterialColor"s, "Default material color in viewport/editor"s, 0x000000, 0xFFFFFF, 0xB469FF);
PropInt(Editor, ElementSelectColor, "ElementSelectColor"s, "Element selection color in viewport/editor"s, 0x000000, 0xFFFFFF, 0x00FF0000);
PropInt(Editor, ElementSelectLockedColor, "ElementSelectLockedColor"s, "Locked Element selection color in viewport/editor"s, 0x000000, 0xFFFFFF, 0x00A7726D);
PropInt(Editor, BackGroundColor, "BackGroundColor"s, "Background color in viewport/editor"s, 0x000000, 0xFFFFFF, 0x008D8D8D);
PropInt(Editor, FillColor, "FillColor"s, "Fill color in viewport/editor"s, 0x000000, 0xFFFFFF, 0x00B1CFB3);
PropEnum(Editor, Units, "Units"s, "Unit used in viewport/editor"s, int, 0, "Inches"s, "Millimeters"s, "VP Units"s);
PropBool(Editor, AlwaysViewScript, "AlwaysViewScript"s, "Always view Script window"s, false);
PropFloatUnbounded(Editor, ThrowBallMass, "ThrowBallMass"s, "Mass of thrown ball in 'throw ball' debugging mode"s, 1.f);
PropIntUnbounded(Editor, ThrowBallSize, "ThrowBallSize"s, "Size of thrown ball in 'throw ball' debugging mode"s, 50);
PropBool(Editor, RenderSolid, "RenderSolid"s, "Render solid in viewport/editor"s, true);

// Code View settings
PropInt(CVEdit, BackGroundColor, "BackGroundColor"s, ""s, 0x000000, 0xFFFFFF, RGB(255, 255, 255));
PropInt(CVEdit, BackGroundSelectionColor, "BackGroundSelectionColor"s, ""s, 0x000000, 0xFFFFFF, RGB(192, 192, 192));
PropBool(CVEdit, DisplayAutoComplete, "DisplayAutoComplete"s, ""s, true);
PropIntUnbounded(CVEdit, DisplayAutoCompleteAfter, "DisplayAutoCompleteAfter"s, ""s, 1);
PropBool(CVEdit, DwellDisplay, "DwellDisplay"s, ""s, true);
PropBool(CVEdit, DwellHelp, "DwellHelp"s, ""s, true);
PropIntUnbounded(CVEdit, DwellDisplayTime, "DwellDisplayTime"s, ""s, 700);

PropBool(CVEdit, EverythingElse, "EverythingElse"s, ""s, true);
PropInt(CVEdit, EverythingElse_color, "EverythingElse_color"s, ""s, 0x000000, 0xFFFFFF, RGB(0, 0, 0));
PropIntUnbounded(CVEdit, EverythingElse_FontPointSize, "EverythingElse_FontPointSize"s, ""s, 10);
PropString(CVEdit, EverythingElse_Font, "EverythingElse_Font"s, ""s, ""s);
PropInt(CVEdit, EverythingElse_FontWeight, "EverythingElse_FontWeight"s, ""s, 0, 1000, 400);
PropBool(CVEdit, EverythingElse_FontItalic, "EverythingElse_FontItalic"s, ""s, false);
PropBool(CVEdit, EverythingElse_FontUnderline, "EverythingElse_FontUnderline"s, ""s, false);
PropBool(CVEdit, EverythingElse_FontStrike, "EverythingElse"s, ""s, false);

PropBool(CVEdit, Default, "Default"s, ""s, true);
PropInt(CVEdit, Default_color, "Default_color"s, ""s, 0x000000, 0xFFFFFF, RGB(0, 0, 0));
PropIntUnbounded(CVEdit, Default_FontPointSize, "Default_FontPointSize"s, ""s, 10);
PropString(CVEdit, Default_Font, "Default_Font"s, ""s, ""s);
PropInt(CVEdit, Default_FontWeight, "Default_FontWeight"s, ""s, 0, 1000, 400);
PropBool(CVEdit, Default_FontItalic, "Default_FontItalic"s, ""s, false);
PropBool(CVEdit, Default_FontUnderline, "Default_FontUnderline"s, ""s, false);
PropBool(CVEdit, Default_FontStrike, "Default"s, ""s, false);

PropBool(CVEdit, ShowVBS, "ShowVBS"s, ""s, true);
PropInt(CVEdit, ShowVBS_color, "ShowVBS_color"s, ""s, 0x000000, 0xFFFFFF, RGB(0, 0, 0));
PropIntUnbounded(CVEdit, ShowVBS_FontPointSize, "ShowVBS_FontPointSize"s, ""s, 10);
PropString(CVEdit, ShowVBS_Font, "ShowVBS_Font"s, ""s, ""s);
PropInt(CVEdit, ShowVBS_FontWeight, "ShowVBS_FontWeight"s, ""s, 0, 1000, 400);
PropBool(CVEdit, ShowVBS_FontItalic, "ShowVBS_FontItalic"s, ""s, false);
PropBool(CVEdit, ShowVBS_FontUnderline, "ShowVBS_FontUnderline"s, ""s, false);
PropBool(CVEdit, ShowVBS_FontStrike, "ShowVBS"s, ""s, false);

PropBool(CVEdit, ShowComponents, "ShowComponents"s, ""s, true);
PropInt(CVEdit, ShowComponents_color, "ShowComponents_color"s, ""s, 0x000000, 0xFFFFFF, RGB(0, 0, 0));
PropIntUnbounded(CVEdit, ShowComponents_FontPointSize, "ShowComponents_FontPointSize"s, ""s, 10);
PropString(CVEdit, ShowComponents_Font, "ShowComponents_Font"s, ""s, ""s);
PropInt(CVEdit, ShowComponents_FontWeight, "ShowComponents_FontWeight"s, ""s, 0, 1000, 400);
PropBool(CVEdit, ShowComponents_FontItalic, "ShowComponents_FontItalic"s, ""s, false);
PropBool(CVEdit, ShowComponents_FontUnderline, "ShowComponents_FontUnderline"s, ""s, false);
PropBool(CVEdit, ShowComponents_FontStrike, "ShowComponents"s, ""s, false);

PropBool(CVEdit, ShowSubs, "ShowSubs"s, ""s, true);
PropInt(CVEdit, ShowSubs_color, "ShowSubs_color"s, ""s, 0x000000, 0xFFFFFF, RGB(120, 0, 120));
PropIntUnbounded(CVEdit, ShowSubs_FontPointSize, "ShowSubs_FontPointSize"s, ""s, 10);
PropString(CVEdit, ShowSubs_Font, "ShowSubs_Font"s, ""s, ""s);
PropInt(CVEdit, ShowSubs_FontWeight, "ShowSubs_FontWeight"s, ""s, 0, 1000, 400);
PropBool(CVEdit, ShowSubs_FontItalic, "ShowSubs_FontItalic"s, ""s, false);
PropBool(CVEdit, ShowSubs_FontUnderline, "ShowSubs_FontUnderline"s, ""s, false);
PropBool(CVEdit, ShowSubs_FontStrike, "ShowSubs"s, ""s, false);

PropBool(CVEdit, ShowRemarks, "ShowRemarks"s, ""s, true);
PropInt(CVEdit, ShowRemarks_color, "ShowRemarks_color"s, ""s, 0x000000, 0xFFFFFF, RGB(0, 120, 0));
PropIntUnbounded(CVEdit, ShowRemarks_FontPointSize, "ShowRemarks_FontPointSize"s, ""s, 10);
PropString(CVEdit, ShowRemarks_Font, "ShowRemarks_Font"s, ""s, ""s);
PropInt(CVEdit, ShowRemarks_FontWeight, "ShowRemarks_FontWeight"s, ""s, 0, 1000, 400);
PropBool(CVEdit, ShowRemarks_FontItalic, "ShowRemarks_FontItalic"s, ""s, false);
PropBool(CVEdit, ShowRemarks_FontUnderline, "ShowRemarks_FontUnderline"s, ""s, false);
PropBool(CVEdit, ShowRemarks_FontStrike, "ShowRemarks"s, ""s, false);

PropBool(CVEdit, ShowLiterals, "ShowLiterals"s, ""s, true);
PropInt(CVEdit, ShowLiterals_color, "ShowLiterals_color"s, ""s, 0x000000, 0xFFFFFF, RGB(0, 120, 160));
PropIntUnbounded(CVEdit, ShowLiterals_FontPointSize, "ShowLiterals_FontPointSize"s, ""s, 10);
PropString(CVEdit, ShowLiterals_Font, "ShowLiterals_Font"s, ""s, ""s);
PropInt(CVEdit, ShowLiterals_FontWeight, "ShowLiterals_FontWeight"s, ""s, 0, 1000, 400);
PropBool(CVEdit, ShowLiterals_FontItalic, "ShowLiterals_FontItalic"s, ""s, false);
PropBool(CVEdit, ShowLiterals_FontUnderline, "ShowLiterals_FontUnderline"s, ""s, false);
PropBool(CVEdit, ShowLiterals_FontStrike, "ShowLiterals"s, ""s, false);

PropBool(CVEdit, ShowVPcore, "ShowVPcore"s, ""s, true);
PropInt(CVEdit, ShowVPcore_color, "ShowVPcore_color"s, ""s, 0x000000, 0xFFFFFF, RGB(200, 50, 60));
PropIntUnbounded(CVEdit, ShowVPcore_FontPointSize, "ShowVPcore_FontPointSize"s, ""s, 10);
PropString(CVEdit, ShowVPcore_Font, "ShowVPcore_Font"s, ""s, ""s);
PropInt(CVEdit, ShowVPcore_FontWeight, "ShowVPcore_FontWeight"s, ""s, 0, 1000, 400);
PropBool(CVEdit, ShowVPcore_FontItalic, "ShowVPcore_FontItalic"s, ""s, false);
PropBool(CVEdit, ShowVPcore_FontUnderline, "ShowVPcore_FontUnderline"s, ""s, false);
PropBool(CVEdit, ShowVPcore_FontStrike, "ShowVPcore"s, ""s, false);
