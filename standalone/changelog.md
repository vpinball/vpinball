# Visual Pinball Standalone Change Log

To keep up with all the changes in master, and make it easier to rebase, this branch is a single commit -- until most tables work with no vbs changes.

The downside of this approach is not accurately keeping track of history:

* 10/01/24
    * Bump Wine source to 9.18 [42af68b8](https://gitlab.winehq.org/wine/wine/-/tree/42af68b8b7cf066a1738c483e8ea1b5bf3b15887)
    * Update FFMPEG to 7.1.0

* 09/23/24
    * Added support for libvpinball for iOS (and in the future Android)

* 09/22/24
    * Re-enabled vbscript engine `release_script` 

* 08/14/24
    * Update FFMPEG to 7.0.2
    * Replace TTF_RenderUTF8_Blended_Wrapped errors with warnings for PuP labels

* 08/04/24
    * Disable B2S polling if table script checks solenoids, lamps, etc.
    * Add support for proxying PinMAME TimeFence through via B2S

* 06/20/24
    * Bump ci to macos-latest (arm64)
    * Send case insensitive pupvideos folder to libdmdutil
    * Add support for case insensitive AltColor and pinmame folders

* 06/15/24
    * Bump minimum macos to 14 to better support BGFX and future dev
    * Remove X11 dependencies from FFMPEG macos builds

* 06/14/24
    * Move logger setup/init to Logger class (@YellowLabrador)
    * Update standalone `android-project` to match 10.8.1 branch

* 06/07/24
    * Add support for older pup packs (@francisdb)

* 06/05/24
    * Implement [official](https://gitlab.winehq.org/wine/wine/-/merge_requests/5795) Implement putref_Item() method. (@francisdb)

* 06/03/24
    * Add support for updating PuP label font sizes using `fonth` (@francisdb)

* 06/02/24
    * Split PuP requests into `PUPPinDisplayRequest` and `PUPTriggerRequest`
    * Rework PuP startup to run after "Startup done"
    * Implement B2S commits af0c3cd, ca2e764, and fcbd891

* 05/31/24
    * Prevent empty captions from resetting PuP image labels (@francisdb)

* 05/30/24
    * Add support for multiline PuP labels
    * Replace `\\r` with `\n` in PuP label captions
    * Add support for loading PuP fonts by filename and family name
    * Fix PUPPinDisplay `GetRoot` (@francisdb, @toccata10)
    * Add support for PuP DMD, Playfield, and FullDMD windows

* 05/29/24
    * Refactor PUP again
    * Simplify PUPMediaPlayer. Add support for MP3 decoding
    * Move background player logic to PUPMediaManager 
    * Screens now get own copies of playlists and triggers

* 05/15/24
    * Refactor PuP to better support triggers
    * More work on PuP labels and screen ordering
    * Add wctype.h to compile under GCC 14 (@WildCoder)

* 05/09/24
    * Implement open panel for macOS builds (@kaicherry)

* 05/04/24
    * Bump Wine source to 9.8 [655de4b0](https://gitlab.winehq.org/wine/wine/-/tree/655de4b0bf09746cd163dc771abd2c0f3c777447)

* 05/03/24
    * Add support for PuP background frames

* 05/02/24
    * Add support for PUP LabelInit
    * Fix TTF initialization (@francisdb)

* 05/01/24
    * Implement case insensitive path helper (@WildCoder)
    * Add latest updates to B2S since initial port

* 04/30/24
    * Add support for PuP Shadowed and Outline labels

* 04/27/24
    * Add support for child PuP screens
    * Share `PUPManager` between `PUPPlugin` and `PUPPinDisplay`
    * Add support for PuP labels (@YellowLabrador)

* 04/22/24
    * Add non-fullscreen window positioning support

* 04/21/24
    * More PuP work. Work on repeating background videos

* 04/16/24
    * Change `PupVideos` to `pupvideos`
    * Update PuP audio decoding to support FFMPEG 4.4 (needed for Batocera)
    * Use playlist volume when playfile in trigger is not set

* 04/15/24
    * Start working on `PUPPlugin`
    * More work on `libdof` library

* 04/13/24
    * Update VBScript `DateDiff` to support strings (@francisdb)

* 04/03/24
    * Rename PuP files in support of future `PUPPlugin`

* 03/30/24
    * Add support for PUPCapture trigger matching (@mkalkbrenner)

* 03/23/24
    * Implement B2S `PluginHost` and `Plugin` logic
    * Added new `DOFPlugin` which calls new `libdof` library

* 03/21/24
    * Fix vbscript `GlobalJoin` when using empty values (@francisdb)

* 03/19/24
    * Preserve dot files in android assets (fixes missing pinmame folder)

* 03/18/24
    * Add support for `PinmameGetNVRAM` and `PinmameGetChangedNVRam`

* 03/11/24
    * Overhaul CI caching strategy for externals
    * Bump libpinmame and libdmdutil

* 03/03/24
    * Add manual unzip button in web server
    * Lock external windows prior to rendering (fixes TimonPumbaa)

* 03/01/24
    * Add support for unzipping uploads via web server

* 02/29/24
    * Fix PinMAME games with video displays from crashing (fixes MotorShow)
    * Allow external window registration after startup (fixes UT99CTGF_GE)

* 02/26/24
    * Add arm64 suffix to iOS and tvOS folders
    * Add arm64-v8a suffix to Android folder
    * Add workflows to build non-signed iOS ipa files
    * Add unique `CFBundleVersion` for iOS workflow builds
    * Split workflows into vpinball, vpinball-mobile, vpinball-sbc
    * Replace asset README.md files with .gitkeep files

* 02/24/24
    * Bump libdmdutil, libpinmame, and libzedmd

* 02/21/24
    * Clear external windows before renderering

* 02/20/24
    * Added `BackBufferScale` `[Standalone]` entry in `VPinballX.ini`
    * Add rotation support for all external windows (@walknight/@cth77)
    * Add Pinscape mappings to gamecontrollerdb.txt
    * Update to libdmdutil 0.3.0 (@mkalkbrenner, @toxie)

* 02/19/24
    * Revert support for simultaneous joystick and game controller
    * Add support for game controller mappings via gamecontrollerdb.txt
    
* 02/18/24
    * Update SDL2, SDL2_image, and SDL2_ttf
    * Add documentation on compiling linux version using Docker
    * Simplified MacOS dylibs in CMakeLists and CPack
    * Add analog input support for game controllers (@TheToon)
    * Add support for both a joystick and game controller

* 02/09/24
    * Added ball control support (@francisdb)

* 02/08/24
    * Fix random return codes on Linux (@francisdb)

* 02/07/24
    * Revert RenderMode. Redesign external windows again

* 02/04/24
    * Added RenderMode to external windows to solve stutter on MacOS

* 02/03/24
    * Add `WindowManager` to automatically handle external window z-order
    * Update libdmdutil to use new `VirtualDMD`

* 02/01/24
    * Bump PinMAME to 982662bec31a63c0e31423e49da5b57d574a3130
    * Revert normalizing PinMAME lamps to support PWM

* 01/28/24
    * Rework linux/sbc CMakeLists.txt to copy external libraries based on option

* 01/27/24
    * Fixed resolution switching on MacOS when using FullScreen = 1
    * Added `Available window fullscreen desktop resolutions` to `-listres`
    * Update PinMAME to a157ac7dc5a4020f7b4af6353b5809f4220ec3d3
    * Added new `VBScript Issues` document

* 01/21/24
    * Split out changelog from readme (@Somatik)
    * Update PinMAME to de133ce281e3cf1c6b0a1ca9595abcf0a19a0af8

* 01/19/24
    * Re-add SDL2_ttf to prep for PuP work

* 01/16/24
    * Revert PinMAME to 893da424f82797d9fa37854691505527353f088a
    * Update libdmdutil to 5d1b7728a031d991b62177d178d95bc135fe5a95

* 01/11/24
    * Handle tiny negative values in GLES codepath (@toxie)

* 01/07/24
    * Add path normalization to `GetFileAttributesW` and `CreateFileW` (fixes #1309)

* 01/04/24
    * Implement [official](https://gitlab.winehq.org/wine/wine/-/merge_requests/4778) scrrun/dictionary: Handle VT_EMPTY/VT_NULL keys

* 12/26/23
    * Switch to versioned libaltsound

* 12/17/23
    * Add cwd to relative paths for command line path arguments (fixes #1267)
    * Trim and normalize command line path arguments

* 12/15/23
    * Fixed 16-bit commands in libaltsound (fixes fh)

* 12/13/23
    * Preserve file attributes for linux/sbc github artifacts
    * Removed `setup.sh` script
    * Finished implementing Altsound

* 12/09/23
    * Start to implement Altsound
    * Bump Wine source to 9.0-rc1 [93f7ef86](https://gitlab.winehq.org/wine/wine/-/tree/93f7ef86701f0b5f0828c8e0c4581b00873a7676) (fixes Wine issue 55052)

* 12/01/23
    * Add caching to `external` folders for CI builds
    * Bump Wine source to 8.21 [369b540a](https://gitlab.winehq.org/wine/wine/-/tree/369b540abf32869df8e9b28e283d795ae92b6a05) (fixes Wine issues 55185, 55931, 55969)

* 11/29/23
    * Stop B2S timer prior to stopping PinMAME
    * Bump Wine source to 8.21 [bc13bda5](https://gitlab.winehq.org/wine/wine/-/tree/bc13bda5ee4edaafa7ba9472d41acbad50c42112)
    * Add window icon for linux builds (@francisdb)

* 11/27/23
    * Implement official oleaut32: VarMod() fixes
    * Stub PuP methods from 1.5 beta

* 11/25/23
    * Wine hack to clear result prior to VarMod (fixes StarTrek 1971)
    * Fix undefined symbol for iOS and tvOS builds (@jasonwambach)

* 11/23/23
    * Wine hack to support `Global_DateDiff` and `Global_DatePart` (partially fixes Iron Maiden)
    * Fix crash in B2S when score is longer than reel digits (fixes #1150)

* 11/21/23
    * Update FlexDMD `ResolveSrc` to normalize paths (fixes GOT)

* 11/20/23
    * Fix internally handled PinMAME mechs to return correct data (fixes MM)

* 11/19/23
    * Add support for MacOS 13.0

* 11/18/23
    * Add support for MacOS app bundles (signing, notarizing, and Game Mode)
    * Add support for directly launching VPX files on MacOS with a double click
    * Remove  `PinMAMEIniPath` `[Standalone]` entry in `VPinballX.ini`
    * Auto detect `pinmame` folder in current table directory

* 11/11/23
    * Attempt to find IP address on en1 for tvOS 17

* 11/07/23
    * Bump to SDL 2.28.5 and libserum 1.6.1

* 11/06/23
    * Fix to properly handle `FlexDMD.Resources.dmds.black.png` (fixes Futurama)

* 11/04/23
    * Log B2S and DMD window position changes (@ClarkKent)

* 11/01/23
    * Fixed B2S `CheckBulbs` to not re-add rom ids (fixes Bad Cats)

* 10/30/23
    * Fixed `PrefPath` command line option
    * Fix incorrect or missing segments when B2S timer polls VPinMAME (fixes Dracula)
    * Fix B2S incorrect `BringToFront` ordering (fixes Defender)

* 10/25/23
    * Add stereo support for OpenGL 4.1+ (@DeKay)

* 10/24/23
    * Reworked default joystick/game controller layout (@DeKay)
    * Fixed not loading `[Standalone]` settings from table ini

* 10/23/23
    * Fixed missing Hankin AlphaNumeric layout (@freezy, @toxie, @francisdb)
    * Added version information to logs (@ClarkKent)

* 10/18/23
    * Fix Android builds
    * Fix saving PinMAME nvram when B2S is enabled

* 10/17/23
    * Refactored `Graphics` and `GraphicsPath` to correctly draw B2S segments
    * Implemented B2S timer to poll VPinMAME when table doesn't (fixes Mousn)

* 10/12/23
    * Add safeguards when parsing invalid B2S base64 images

* 10/11/23
    * Added `B2SDualMode` `[Standalone]` entry in `VPinballX.ini`
    * Reverted `B2SReelBox` hack

* 10/10/23
    * Allow B2S and DMD windows to be draggable (@Somatik)
    * Add `setup.sh` script to github artifacts
    * Fix marking all animations as random while parsing B2S files
    * Fix B2S `SecondRomIDType` for GI Strings (fixes NF)

* 10/09/23
    * Implement [official](https://gitlab.winehq.org/wine/wine/-/merge_requests/3206) Support one-line sub/function
    * Implement Linux [rpath](https://github.com/vpinball/vpinball/pull/888) fix (@Somatik)
    * Hack `B2SReelBox` to roll properly

* 10/07/23
    * Fixed not loading all intermediate images for B2S reels

* 10/06/23
    * Updated FlexDMD to return `Nothing` for `NewImage`, `NewVideo` (fixes GOT)

* 10/05/23
    * Update DMD window to render only when a change occurs

* 10/04/23
    * Log error when bitmap font file not found for FlexDMD
    * Update all `external.sh` scripts to exit on any error (@Somatik)
    * Split `B2SFrameSkip` into `B2SBackglassFrameSkip` and `B2SDMDFrameSkip`
    * Fix B2S from not running CheckSolenoids, etc when no VPinMAME changes
    * Fix B2S image rotation precision and switch from clockwise to anticlockwise
    * Fix B2S from not starting rotations properly
    * Add `IsInvalidated` support for B2S. Now renders only when a change occurs

* 10/03/23
    * Moved B2S and DMD windowing out of player
    * Render B2SDMD background image and lights
    * Moved `Graphics` and `Timer` classes from `b2s` into `common`
    * Updated FlexDMD to use combined `Graphics` class
    * Cleaned up virtual / override in FlexDMD (fixes Sonic)

* 10/02/23
    * Removed some dead/unused code in B2S (`TimerAnimations`, etc.)

* 10/01/23
    * Work on some float/int precision issues in B2S (CC now lights up)

* 09/29/23
    * Fixed PinMAME `GIStrings` updates not updating B2S

* 09/28/23
    * Added support for B2S DMD
    * Reworked B2S settings in `VPinballX.ini`

* 09/26/23
    * Fix B2S VPinMAME solenoids (Bad Cats cat now spins)
    * Added log warnings with NULL fonts in FlexDMD

* 09/25/23
    * B2SAnimations are now working! Insanely huge thanks to @Scottacus64!
    * Added support for mechs in B2S (fixes Scared Stiff)
    * B2S::Timer callback refactor

* 09/23/23
    * Start on B2SAnimations

* 09/22/23
    * Fixed Dream7 LEDs not displaying for PinMAME games
    * Completely overhauled DMD logic

* 09/21/23
    * Add `DMD`, `DMD`, `DMDWidth`, `DMDHeight` `[Standalone]` entries in `VPinballX.ini`
    * Added support for separate PinMAME DMD window
    * Only show B2S and DMD windows when callbacks are registered
    * Fix typo in README (Thanks @dekay)

* 09/20/23
    * First pass at B2S Dream7 LEDs

* 09/11/23
    * More B2S work
    * Fix Android CI builds

* 09/08/23
    * Add `B2X`, `B2Y`, `B2Width`, `B2Height` `[Standalone]` entries in `VPinballX.ini`

* 09/06/23
    * Bump SDL2 to 2.28.3

* 09/05/23
    * Implement VPinMAMEController proxying in B2SBackglassServer
    * Add `[Standalone] B2S` entry in `VPinballX.ini`

* 09/04/23
    * Remove ATL from VPinMAMEController

* 09/03/23
    * Start to implement B2SBackglassServer

* 08/31/23
    * Fix created UltraDMD via NewUltraDMD (fixes SpaceRamp)

* 08/28/23
    * Bump Wine source to 8.14 [221d5aba](https://gitlab.winehq.org/wine/wine/-/tree/221d5aba3f19c1014dad30cad04c4b14c284d9d2)
    * Add support for GCC 13
    * Add `[Standalone] PinMAMEIniPath` and `[Standalone] VPRegPath` entries in `VPinballX.ini`
    * Fix FlexDMD crash when assigning bitmap to existing image

* 08/24/23
    * Fix per table `.ini` file support

* 08/18/23
    * Cleanup FlexDMD to closer match C#
    * Remove ATL from FlexDMD

* 08/16/23
    * Finish implementing FlexDMD actions (fixes flexdmd demo)

* 08/15/23
    * Start to implement FlexDMD actions

* 08/14/23
    * Fixed uninitialized `m_translateX`/`Y` and `m_clip` variables in FlexDMD (fixes Linux builds)
    * Cleaned up several warns when `-Wall` is enabled

* 08/13/23
    * Make `eExitGame` immediately quit (fix for Batocera)
    * Switch `eExitGame` to `eEscape` for iOS and Android

* 08/12/23
    * Implemented bitmap filters in FlexDMD (fixes bnbps)

* 08/10/23
    * Moved texture cache to prefences path
    * Removed `ImplicitTimer`
    * Temporary `GL_UNSIGNED_SHORT` to `GL_FLOAT` hack (fixes bloodmach insert)
    * Switch external scripts to use SHA for PinMAME instead of using master

* 08/04/23
    * Bump SDL2 to 2.28.2. Start to rearrange FlexDMD to match C#

* 08/01/23
    * Fixed `libSDL2_image-2.0.so.0` on RPI4 (@Jacky S)

* 07/31/23
    * Cleanup FlexDMD cached bitmaps and fonts on exit

* 07/29/23
    * Implement FlexDMD image and font caching via `AssetManager`

* 07/25/23
    * Implement tweening for UltraDMD (fixes Kiss)

* 07/20/23
    * Properly support [out] parameters in COM proxy files (fixes White Water)
    * Port more of FlexDMD to support UltraDMD

* 07/18/23
    * Added support for disabling "sound" in pinmame

* 07/17/23
    * Fix `CreateFileW` to support `dwCreationDisposition` (fixes Slayer)

* 07/16/23
    * Port more of FlexDMD to support UltraDMD

* 07/10/23
    * Added `-PrefPath` command-line option to override `$HOME/.vpinball`
    * Added `PinMAMEPath` entry in `VPinballX.ini` to override `$HOME/.pinmame`

* 07/08/23
    * Added support for enabling cheat in libpinmame (fixes tom14h)

* 07/06/23
    * Fixed physics set override generating zero gravity (fixes BBBB)

* 07/05/23
    * Open ZeDMD connection in a thread (fixes Rock)
    * Bump Android java files to SDL 2.18.2

* 07/03/23
    * Wine hack to support `numdecimalplaces` in `Global_Round` (@francisdb)

* 06/30/23
    * Start to implement UltraDMD

* 06/29/23
    * Update `GetTextFile` to also look in `m_currentTablePath` (KISS Balutito MOD partially working)

* 06/28/23
    * Moved PinMAME `.ini` files into `<preferences>/pinmame`
    * Added `VPReg.ini` and implmented `LoadValue` and `SaveValue` commands

* 06/27/23
    * Implmented AlphaNumeric to DMD support for `VPinMAMEController` and `FlexDMD`
    * Bump `libserum` to `1.6.0`

* 06/23/23
    * Added `normalize_path_separators` helper to use in FlexDMD, WMP, and AudioPlayer
    * Implemented `GetLocalTime` (Fixes `Global_Timer`)
    * Stub FlexDMD `Segments`

* 06/22/23
    * Added Keyboard section to this readme
    * Return `S_OK` for FlexDMD `GetLabel`, `GetImage` when value is `Nothing`
    * Fix multiline FlexDMD rendering

* 06/21/23
    * Implement `NewSoundCommands` in `VPinMAMEController`
    * More work on Clang vs GCC floating point compiler options
    * Implement [official](https://gitlab.winehq.org/wine/wine/-/merge_requests/3132) stack_pop_bool wine fix
    * Replace `E_NOTIMPL` with `PLOGW` for PuP (@francisdb)

* 06/19/23
    * Updated `VS_EYE` and `VS_OUT_EYE` shader macros to work on Android shader compilers
    * Switched `-ffp-model=strict` to `-ffp-contract=off` and `-ffast-math` (@toxie)

* 06/17/23
    * Added `-ffp-model=strict` to all `CMakeLists.txt` when building with Clang (Thanks @tamburro92!)

* 06/14/23
    * Revert all scrrun/dictionary Wine hacks including `get_num_hash`
    * Implement [official](https://gitlab.winehq.org/wine/wine/-/merge_requests/3064) scrrun/dictionary fixes
    * Fixed deadlock in FlexDMD on table exit when script runtime error
    * Move `SDL_Surface` from `Image` to `FlexDMDBitmap` to support copying
    * Implement reference counters for `FlexDMDBitmap` and `FlexDMDFont`

* 06/13/23
    * Wine hack to produce unique hashes for floats in `get_num_hash`
    * Updated `external.sh` scripts to use multiple CPUs

* 06/12/23
    * Added `GIString` and `GIStrings` to `VPinMAMEController`

* 06/10/23
    * Switch Plog `ConsoleAppender` to `ColorConsoleAppender`

* 06/08/23
    * Fix help command line argument (`-h`, `/h`, `-?`, etc)

* 06/05/23
    * Stub some more PuP methods (VoDoo's Carnival)
    * Transition ZeDMD to use `libzedmd`

* 06/04/23
    * Return `Nothing` when FlexDMD `NewImage` and `NewVideo` could not load asset
    * Stub `NewSoundCommands` in `VPinMAMEController` (Fixes LOTR)

* 06/02/23
    * Update `vpx` script to find tables in subdirectories

* 05/30/23
    * Bump Wine source to 8.9 [9ffeb262](https://gitlab.winehq.org/wine/wine/-/tree/9ffeb2622d087a6189ca916553529824791010c3)
    * Add web server connection URL to LiveUI

* 05/26/23
    * Renamed ZeDMD constants. Reduced maximum queued frames for Android
    * Removed software downscaling from ZeDMD
    * Improved Touch overlay
    * Web server - Added Toggle FPS
    * Add Launch Table to LiveUI for Android, iOS, and tvOS

* 05/25/23
    * Web server - Shutdown, New Folder, Delete, Edit, and Drag and Drop uploads
    * Web server - Add `WebServer`, `WebServerAddr`, and `WebServerPort` to `VPinballX.ini`
    * Table failures will now reset `LaunchTable` on iOS, tvOS, and Android

* 05/24/23
    * Added `[Standalone]` section to `VPinballX.ini`
    * Added support to configure ZeDMD settings on startup
    * Web server - added sort by name and size
    * Web server - added vpx activation button (sets `LaunchTable` value)

* 05/23/23
    * Work on embedded web server based on Mongoose
    * Add a Paths section to this readme (@francisdb)

* 05/22/23
    * Bump `libserum` to `1.5.1`. Rotations finally fixed!
    * Android, iOS, and tvOS, now start game using `LaunchTable` in `VPinballX.ini`

* 05/20/23
    * iOS and tvOS builds now copies all assets to `Documents` directory

* 05/17/23
    * Added scaling support for ZeDMD
    * Properly handle `\r` when parsing FlexDMD bitmap font files

* 05/14/23
    * Bring ZeDMDComm.cpp closer to C# version from DMD-Ext
    * Bump `libserum` to `1.5.0`

* 05/13/23
    * Fix -listres and -listsnd due to reworked plog init
    * Fix SDL fullscreen on MacOS

* 05/12/23
    * Implement `GlobalJoin` in Wine VBS engine (CyberRace)
    * Implement `Get_Length` for videos in FlexDMD (CyberRace)
    * Update Drop Target and Standup Target workarounds

* 05/10/23
    * Start to implement PuP

* 05/08/23
    * Added ZeDMD support to Android builds
    * Added support for touch nudging in mobile builds

* 05/01/23
    * Update HelpSplash to properly support multiline text
    * Update start help overlays to display for the correct amount of time

* 04/30/23
    * Switch `VPinMAMEController` to always use RAW mode
    * Rework Serum colorization
    * Allow DMD color to be set via ini file in `.pinmame/vpinball`

* 04/29/23
    * Updated `libpinmame` to use new user data pointer
    * Added ZeDMD support for PinMAME games

* 04/27/23
    * Added ZeDMD support
    * Implement RenderMode in FlexDMD

* 04/25/23
    * Finally fixed ImGui flickering issue in OpenGLES (Renderdoc / @Niwak)
    * Added FlexDMD asset folder to iOS and tvOS builds
    * Re-enable AO for OpenGLES builds and fix `GREY8` -> `GL_R8` (@Niwak)
    * Remove `System.Drawing.Common` stubs

* 04/23/23
    * Fix several FlexDMD label and image rendering issues

* 04/20/23
    * Start to implement FlexDMD labels
    * Fix Linux builds (@pablo99)

* 04/19/23
    * Add `--flexdmddir` option to `vpxm`
    * Move `vpinball.log` to `m_szMyPrefPath`
    * Fix FlexDMD images from not showing

* 04/17/23
    * Add `SDL_Image` for `FlexDMD` GIF support

* 04/14/23
    * More work on implementing FlexDMD
    * Update `DTArray` and `STArray` workarounds to use `DropTarget` and `StandupTarget` classes
    * Added `Working Tables` section

* 04/08/23
    * Fix `FormatNumber` to support VT_ERROR when argument is omitted
    * Stub more FlexDMD functions

* 04/06/23
    * Replace `create_directories` with `create_directory`

* 04/05/23
    * Move `VPinballX.ini` to `m_szMyPrefPath`
    * Revert `m_szMyPath` and `m_wzMyPath` not including trailing path separator
    * Fixed `get_SolMask` and `put_SolMask` to use new PWM support in PinMAME 3.6
    * Bump `SDL2` to `2.26.5`

* 04/04/23
    * Remove automatically including user directory in cmake files
    * Generate user folder on first run to fix iOS crash (fixes Cue Ball Wizard)
    * Update `m_szMyPath` and `m_wzMyPath` to not include trailing path separator (for consistency)

* 04/03/23
    * Inject live table with a flasher based DMD (`ImplicitDMD2`)
    * Inject live table with a convience timer (`ImplicitTimer`)
    * Bump `libserum` to `1.4.0`
    * Update to support fixed `BRIGHTNESS` vs `RAW` libpinmame DMD mode
    * Add buttons in Live UI popup menu to switch to Desktop / Cabinet / and FSS modes

* 03/26/23
    * Wine hack to add more types in `stack_pop_bool` (fixes SS, Big Indian, and Whirlwind)

* 03/24/23
    * Added rk3588 build
    * Split up CI to build SBC versions separately
    * Cleaned up instructions

* 03/23/23
    * Fix `core.vbs` patch to work in Windows builds
    * Added support for `Sound` and `SoundBG` in `VPinballX.ini`
    * Added rk3588 (Orange Pi 5) build instructions

* 03/22/23
    * Bump SDL to 2.26.4 and apply [keyboard fix](https://github.com/libsdl-org/SDL/commit/54ca4d387954e687db0d28758d43cf08a1cc1353) for MacOS builds
    * Bump SDL_ttf to 2.20.2 and remove MacOS harfbuzz patch

* 03/20/23
    * Fixed multiple items being added / overwritten to live table
    * Force implicit DMD to have name `ImplicitDMD`
    * Bump `libserum` to 1.4.0
    * Fixed table exit crash when `CodeViewer` is destroyed in Linux

* 03/19/23
    * Fixed `PoleStorage` to return correct `HRESULT` when file not found

* 03/17/23
    * Update Android to properly calculate DPI for Live UI
    * Fixed iOS Launch Screen issues in `CMakeLists.txt` and `Info.plist`
    * Fixed `implicitDMD` not being added to script engine
    * Added MacOS x64 builds
    * Added `m_serumColorized` flag to prevent rotations before colorize

* 03/16/23
    * Update `PlayMusic` to replace backslash with forward slash
    * Force quit on vbscript errors

* 03/15/23
    * Added support for rotations in `libserum`.
    * Added `-listsnd` command line option to dump out available sound devices

* 03/14/23
    * Added iOS launch storyboard to allow for fullscreen

* 03/13/23
    * More work on shutdown with updated LiveUI
    * Work on build on rk3588 (Orange Pi 5, Armbian)

* 03/11/23
    * Updated `vpxm` script to support NVRAM files
    * Updated to use new libpinmame `PinmameIsPaused`
    * Updated to properly quit on iOS, tvOS, and Android

* 03/10/23
    * Fixed converting `VARIANT_BOOL` to `BSTR` conversion crash on Linux
    * Remove `Debugger` in Live UI
    * Replace `Quit to Editor` with `Quit` in Live UI
    * Implement Pause and Resume
    * Add touch support for iOS and Android

* 03/09/23
    * Add support for [libserum](https://github.com/zesinger/libserum)
    * Bump SDL2 to 2.26.4

* 03/06/23
    * Work on properly shutting down table so `libpinmame` can write nvram files
    * Stub `ShowCursor` to fix compile errors
    * Hack Wine vbscript `clean_props` and `release_script` to prevent seg fault on table exit

* 03/05/23
    * Added support for `get_RawDmdColoredPixels`
    * Check pointers in `Decal` and `Textbox` destructors

* 03/01/23
    * Redesigned `PoleStorage`/`PoleStream` again to support multiple threads
    * Fixed `m_logicalNumberOfProcessors` support
    * Added `external_open_storage` method and moved `StgOpenStorage` to `wine.c`

* 02/27/23
    * Added `HighDPI` configuration option. ie: `<HighDPI>0</HighDPI>` disables on macos and iOS
    * Added `-listres` command line option to dump out available fullscreen resolutions
    * Fixed fullscreen support for systems with multiple displays
    * Use intensity values (`0-255`) in `get_ChangedLamps` only for SAM games

* 02/24/23
    * Fix crash when `exception.bstrDescription` is NULL
    * Fix `LocalStringW` and `GetUniqueName` not initializing with 0
    * Add an implicit DMD for tables that do not have one, ie `TextBox00`

* 02/24/23
    * Additional code cleanup
    * Replace `wine_stubs.c` with `wine.c`
    * Rework external logging code
    * Move cmake files into `standalone/cmake`

* 02/23/23
    * Work to get macos compiling with GCC instead of clang
    * More cleanup to `wine_stubs.c`

* 02/22/23
    * Migrate `main_standalone.h` into `main.h`

* 02/21/23
    * Stub more FlexDMD (Galaga now starts up)
    * Refactor `PoleStorage` and `PoleStream` to more closely match `IStream` IDL

* 02/20/23
    * Start to cleanup `main_standalone.h`
    * Move `PoleStorage` and `PoleStream` to separate class files
    * Use `dinput.h` from Wine
    * Move ATL support files to `standalone/inc/atl`
    * Replace `_stricmp` with `lstrcmpi` for consistency

* 02/19/23
    * Add support for pov
    * Stub out some more of FlexDMD
    * Stub out `_Bitmap` from `System.Drawing.Common`

* 02/18/23
    * Add `get_DIP`, `put_DIP`, `get_Lamp`, and `get_Solenoid` to `VPinMAMEController.` (Addams Family now works)

* 02/17/23
    * Rework iOS and tvOS builds to generate ipa files
    * Add output name switch to `vpxm`

* 02/16/23
    * Rework `IDLParserToC` generator to produce easier to read code
    * Optimize `IDictionary` and `IFileSystem3` proxies
    * Fix Linux build

* 02/15/23
    * Rework `IDLParserToCpp` generator to produce easier to read code
    * Implement [official](https://gitlab.winehq.org/wine/wine/-/merge_requests/2214) fix compile issue with non hex after concat without space (Fixes bnbps)

* 02/14/23
    * Bump Wine source to 8.1 [23c10c92](https://gitlab.winehq.org/wine/wine/-/tree/23c10c928b68918515b6ec195d90b09ef5936451)
    * Rework Wine changes to be as minimal as possible
    * Fix missing `Item` function for `Scripting.Dictionary` (Fixes Beavis and Butthead by @chugalaefoo)
    * Implement [official](https://gitlab.winehq.org/wine/wine/-/merge_requests/2175) fix VarAbs function for BSTR with positive values (Fixes Beavis and Butthead by @chugalaefoo)
    * Implement [official](https://gitlab.winehq.org/wine/wine/-/merge_requests/2188) Fix compile when statement after ElseIf or after separator (Fixed AC/DC compile issue)
    * Move `idl.sh` to `scripts/widlgen`
    * Update mobile and linux builds to use SDL 2.26.3

* 02/09/23
    * Implement [official](https://gitlab.winehq.org/wine/wine/-/merge_requests/2141) Fix memory leak in owned safearray iterator
    * Implement [official](https://gitlab.winehq.org/wine/wine/-/merge_requests/2142) Fix compile when colon follows Else on new line

* 02/08/23
    * Implement [official](https://gitlab.winehq.org/wine/wine/-/merge_requests/2132) Fix memory leak in interp_redim_preserve
    * Implement [official](https://gitlab.winehq.org/wine/wine/-/merge_requests/2131) Fix memory leak in Split()

* 02/07/23
    * Wine hack to fix memory leak with "owned" `SAFEARRAY` in `interp_newenum` and `interp_enumnext`
    * Add `EXTERNAL_DIR` and `APK` settings to `vpxm` script

* 02/06/23
    * Wine hack to fix memory leak in `interp_redim_preserve`

* 02/04/23
    * Fix several memory leaks (`GetRef` and `GlobalSplit`)
    * Fix issue where `CComVariant` would release additional reference after `Detach` in `Close`
    * Added an `AddRef` to `ITypeInfoImpl` which finally fixes deleting objects when ref count = 0!
    * Fixed Linux buffer overflows in `Global_FormatNumber`

* 01/31/23
    * Add `vpxm` script and move `vpx` script to `standalone/scripts`

* 01/29/23
    * Rework IDL generator to use binary search trees for `GetIDsOfNames` and `FireDispID`

* 01/28/23
    * Stub more of `IPinUpDisplay` interface

* 01/27/23
    * Start to implement `IPinUpDisplay` interface
    * Reorganize standalone includes from `/inc` to `/standalone/inc`

* 01/26/23
    * Wine hack to support `typename` for class objects. (Fixes lamps in BM)
    * Update Android to use SDL 2.26.2

* 01/25/23
    * More FlexDMD work (BM is alive!)
    * Replace `VariantCopy` with `VariantCopyInd` in proxies. (Fixes BM `swLeft/RightOrbTrigger2.uservalue` issues)
    * Wine hack to implement missing `GetLocale` and `SetLocale` functions
    * Tweak Game Controller support for iOS
    * Update iOS to use SDL 2.26.2

* 01/24/23
    * Add script to generate code from IDL files using WIDL
    * Rearrange some directories and clean up VPinMAME and WMP
    * Start work on implementing FlexDMD

* 01/21/23
    * Finished `WMPCore`, `WMPSettings`, and `WMPControls` (BM has working music)

* 01/19/23
    * More work on `WMPCore`, `WMPSettings`, and `WMPControls` interfaces

* 01/18/23
    * Skip detecting Siri Remote as `SDL_GameController` in tvOS
    * Set `MaxTexDimension` to `2048` and disable HIGHDPI for tvOS (Fixes AC/DC)
    * Disable Wine `TRACE` and `DEBUG` messages
    * More work on `WMPCore` interfaces

* 01/17/23
    * Wine `ElseIf` hack to not require `NL` after `Then` (Fixed AC/DC)
    * Start to implement `WMPCore`, `WMPControls`, and `WMPSettings`
    * Switch `SDL_Joystick` to `SDL_GameController`
    * Implement `SDL_GameController` rumble support
    * Remapped `Start` and `Cancel` for iOS and tvOS

* 01/16/23
    * Fix `RtlCompareUnicodeStrings` to properly compare wide strings with specified lengths
    * Wine `invoke_variant_prop` hack to support getting array variables in classes, ie: `If Lampz.IsLight(5) = true`
    * Wine `assign_value_script_ctx` hack to support setting array variables in classes, ie: `Lampz.IsLight(5) = true`
    * Implement `tvOS` version
    * Update joystick to match XBox controller layout

* 01/13/23
    * Wine `Global_CreateObject` hack to return `Nothing` instead of error when `CreateObject` fails. (Most scripts check for `Nothing`)
    * Cache `BASS_ATTRIB_FREQ`. (Partially fixes [#224](https://github.com/vpinball/vpinball/issues/224))
    * Attempt to fix BASS looping and restarts. (Partially fixes [#224](https://github.com/vpinball/vpinball/issues/224))
    * Cleanup `vpx` shell script. (Add to your path to run `VPinballX_GL` from anywhere)
    * Created an IDL enum parser to add enums to `ScriptGlobalTable`, ie: `SequencerState` (support VPin Workshop tables)
    * Updated Basic shader to fix reflections on OpenGLES (Thanks @Niwak!)
    * Fixed `get_ChangedLamps` to return modern intensity-levels for `lampNo > 80` (Fixes GI in TWD!)

* 01/12/23
    * Wine `do_icall` hack to support `GetRef` variable calls with no params, ie: `MotorCallback` (Fixes flippers in TWD)
    * Wine `is_matching_key` hack to support `VT_VARIANT|VT_BYREF`

* 01/11/23
    * Wine scrrun `dictionary_get_HashVal` hack to return hash value for `VT_VARIANT|VT_BYREF` (TWD now plays!)
    * Wine vbscript `UBound`, and `LBound` hack to return `0` when `VT_EMPTY` (MM now plays with zero script changes!)
    * Added additional logging for `Eval`, `Execute`, `ExecuteGlobal`

* 01/10/23
    * Redesign `GetRef` again, this time as IDispatch (T2 now plays with zero script changes!)
    * Added delay to make sure PinMAME is running before trying to initialize switches. (CFTBL now runs with zero script changes!)
    * Reworked wine debug logging to use `plog`

* 01/09/23
    * Implement `VPinMAMEController` mechs

* 01/08/23
    * More work on `IGames`, `IGame`, and `IGameSettings` (Rock now works with zero script changes!)

* 01/07/23
    * Stub VPinMAME interfaces (ie, `IRom`, `IRoms`)
    * Add new `libpinmame` methods for setting `SolMask` and `GetLEDs` (Rock renders Alpha Numeric display)

* 01/03/23
    * Fix RPI4 `libsdl2-2.0.so.0` library `SONAME`

* 01/02/23
    * Remove `RPATH` and replace with `$ORIGIN` for Linux/RPI builds
    * Replace `RGB_FP16` with `RGBA_FP16` for OpenGLES builds (RPI4 finally renders!)

* 01/01/23
    * Happy New Year!
    * Work on RPI4 builds

* 12/31/22
    * Fixed startup issues when calling `PinmameGetChangedGIs`/`PinmameGetChangedLamps` before PinMAME is fully running
    * Fixed `VPinMAMEController::get_Version` `BSTR` allocation
    * Fixed non-defaulted `m_capExtDMD` which lead to crashes or incorrect DMD Shader assignment

* 12/30/22
    * Work on Linux builds

* 12/29/22
    * Fix `get_ChangedLamps` to support libpinmame `255` state
    * Wine vbscript hack to allow statement separators after `ELSE` (fixes Cuphead)
    * Replaced `SDL_Log` with `plog` (`plog` moved to root directory)
    * Updated Android builds to recursively copy assets (ie to copy `pinmame/roms`)

* 12/28/22
    * Rework `GetRef` to fix `vpmCreateBall`
    * Convert `mQue` in `core.vbs` to multiple single dimension arrays
    * Updated `IDLParserToCpp` to use `VariantCopyInd` for `VARIANT` params
    * Update `Eval`, `Execute`, and `ExecuteGlobal` to convert `arg` to `BSTR` using `to_string`
    * More work on `VinMAMEController` (Rock/T2 partially working)

* 12/22/22
    * Rework macos builds to use external.sh and build SDL2, SDL2_ttf, FreeImage, and libpinmame
    * Fix music directory to be cross platform

* 12/21/22
    * Start to implement VPinMAMEController

* 12/20/22
    * Attempt to fix HIGHDPI rendering issues with decals and lights
    * Add shim to allow `controller.vbs` and `Controller.vbs` in `GetTextFile` for case sensitive filesystems
    * Update `check_script_collisions` to allow multiple `Dim`s of `B2SOn` and `Controller` in `ExecuteGlobal` (Fixes Volley)

* 12/17/22
    * Reworked `GetMyPath` to use `SDL_GetBasePath()`
    * Fixed repeating `SDL_KEYUP` and `SDL_KEYDOWN` events in Android build
    * Updated CI to make Android builds

* 12/15/22
    * Fixed several GLES 3.0 rendering issues
    * Added support for retina displays on MacOS and iOS via `SDL_WINDOW_ALLOW_HIGHDPI`

* 12/12/22
    * Added `android-project`
    * Cleaned up Android build instructions
    * Replaced `printf` and `std::cout` with `SDL_Log`

* 12/08/22
    * Started working on Android native library builds

* 12/03/22
    * Implement [official](https://gitlab.winehq.org/wine/wine/-/merge_requests/1561) Accept private and public const global declarations.
    * Implement [official](https://gitlab.winehq.org/wine/wine/-/merge_requests/1611) Handle "case" statements without separators.
    * Rework Wine debug stubs to fix strdup memory leak
    * Update shader #defines to support OpenGLES compilation
    * Replace shader integers with floats to support OpenGLES compilation

* 11/27/22
    * OpenGLES updates to allow iOS app to run - working imgui, sound, input, and mouse support.

* 11/24/22
    * Implement [official](https://gitlab.winehq.org/wine/wine/-/merge_requests/1534) Handle another variant of LTE/GTE tokens.

* 11/23/22
    * Implement [official](https://gitlab.winehq.org/wine/wine/-/merge_requests/1495) Add support for redim byref.
    * Add support for iOS simulator build (arm64 only)
    * Skip rendering when OpenGLES so app does not crash (plays, but no graphics)

* 11/20/22
    * `#ifndef __OPENGLES__` around code not available in OpenGLES
    * Implement [official](https://gitlab.winehq.org/wine/wine/-/merge_requests/1409) Handle index read access to array properties
    * More work on iOS build

* 11/16/22
    * Implement [official](https://gitlab.winehq.org/wine/wine/-/merge_requests/1385) Wine vbscript `Else` new line fix
    * Implement [official](https://gitlab.winehq.org/wine/wine/-/merge_requests/1368) Use CRT allocation functions
    * Start to work on iOS build
    * Partial Implement [official](https://gitlab.winehq.org/wine/wine/-/merge_requests/1391) Implement Format functions.
    * Rearrange CMake search paths and fix SDL2 includes for Linux
    * Move RapidXML to separate folder

* 11/14/22
    * Implement [official](https://gitlab.winehq.org/wine/wine/-/merge_requests/1347) Wine vbscript `Global_TypeName` fix
    * Update to glad 2
    * Add Linux x64 build to github workflow

* 11/12/22
    * Wine scrrun hack to allow dictionary hash values for `VT_VARIANT|VT_BYREF`.
    * Remove vbscript hack to allow `VT_SAFEARRAY` and fix idl parser to use `VT_VARIANT|VT_ARRAY`.
    * Remove vbscript `interp_newenum` hack support `for each...next` with `SAFEARRAY`
    * Rearrange `main_standalone.h` to use `GetDocumentation` in `InternalAddRef` and `InternalRelease`
    * Temporarily disable delete in `Release` until tracking down extra `InternalRelease`
    * Implement [official](https://gitlab.winehq.org/wine/wine/-/merge_requests/1328) Wine vbscript `Redim` lists fix
    * Implement [official](https://gitlab.winehq.org/wine/wine/-/merge_requests/1288) Wine vbscript `Global_Mid` fix
    * Implement [official](https://bugs.winehq.org/attachment.cgi?id=73461&action=diff) Wine vbscript `Global_TypeName` patch
    * Remove saving width and height in `VPinballX.ini` when exiting table

* 11/07/22
    * Wine vbscript hack to allow `Redim` lists
    * Wine vbscript `Global_FormatNumber` hack add `FormatNumber` support
    * Wine vbscript `Global_Mid` hack to allow `Mid` on non VT_BSTR
    * Wine vbscript `do_icall` hack to support `Get_Item` in `Collection` (Fixes Road Race)

* 11/04/22
    * Implement [official](https://gitlab.winehq.org/wine/wine/-/merge_requests/1246) Wine vbscript `Global_Rnd` and `Global_Randomize` fixes
    * Wine vbscript `Global_Eval` hack to fix `IsEmpty(Eval("BallSize"))`
    * Wine vbscript `Global_GetRef` hack to workaround `Set GICallback = GetRef("NullSub")`
    * Wine vbscript `parser.y` hack to allow `Private Const`
    * Wine vbscript `parser.y` hack to allow `Else If` on same line (Fixes Four Million B.C.)
    * Wine vbscript `parser.y` hack to allow `Case Else` without colon
    * Trim whitespace in `VPinballX.ini` and save player options when exiting table

* 11/02/22
    * First working non-example table with no vbscript errors (Grand Tour)
    * Wine vbscript `Global_TypeName` hack to fix `if TypeName(balls(x)) = "IBall" then`
    * Wine vbscript `invoke_variant_prop` hack to support array access `aObj.ModIn(x)`
    * Wine vbscript `interp_newenum` hack to support `for each...next` with `SAFEARRAY`
    * Implement `GetIDsOfNames` and `Invoke` for `Scripting.Dictionary` in Wine `scrrun`
    * Implement `GetIDsOfNames` and `Invoke` for `Scripting.FileSystemObject` in Wine `scrrun`
    * Implement stubs for Wine `ReadFile`, `WriteFile`, `GetFullPathNameW`, `GetFileAttributesW`, `CreateFileW` so tables can read and write data
    * Rework `Invoke` for voinball interfaces to not use `CComVariant` wrappers
    * Several updates to `IDLParserToC` and `IDLParserToCpp` including adding `GetDocumentation` to support `TypeName`
    * Do not define `_rtol` in Linux
