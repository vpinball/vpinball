# Visual Pinball Standalone

*An open source **cross platform** pinball table simulator.*

This sub-project of VPinballX is designed to run on non-Windows platforms.

[![Watch the video](https://img.youtube.com/vi/jK3TbGvTuIA/0.jpg)](https://www.youtube.com/watch?v=xjkgzIVL_QU)

## Background

Visual Pinball was built over 20 years ago using Microsoft technologies such as OLE Compound Documents, DirectX, VBScript, COM, and ATL -- many of which have no cross platform equivalents or support.

To make a successful port, we would need tackle several tasks:

> ***OLE Compound Documents***
> - Visual Pinball Table (`.vpx`) files are stored in [Compound File Binary Format](https://en.wikipedia.org/wiki/Compound_File_Binary_Format).
>
> **Solution**
> - Implement [POLE - portable library for structured storage](https://dimin.net/software/pole/) to load `.vpx` files.
> - Create `PoleStorage` wrapper to match `IStorage` interface.
> - Create `PoleStream` wrapper to match `IStream` interface.

> ***Windows Registry***
> - Visual Pinball stores and retrieves its settings from the Windows registry.
>
> **Solution**
> - Use the `ENABLE_INI` preprocessor definition which uses [RapidXML](https://github.com/timniederhausen/rapidxml) to save and retrieve settings from a file.

> ***VBScript***
> - Visual Pinball uses [VBScript](https://learn.microsoft.com/en-us/previous-versions/t0aew7h6(v=vs.85)) as the scripting engine for tables.
>
> **Solution**
> - Leverage the VBScript engine from [Wine](https://github.com/wine-mirror/wine/tree/master/dlls/vbscript).
> - Fix bugs in Wine's VBScript engine:
>   - [Bug 53644](https://bugs.winehq.org/show_bug.cgi?id=53644) - vbscript can not compile classes with lists of private / public / dim declarations
>   - ~~[Bug 53670](https://bugs.winehq.org/show_bug.cgi?id=53670) - vbscript can not compile if expressions with reversed gte, lte, (=>, =<)~~
>   - ~~[Bug 53676](https://bugs.winehq.org/show_bug.cgi?id=53676) - vbscript can not exec_script - invalid number of arguments for Randomize~~
>   - ~~[Bug 53678](https://bugs.winehq.org/show_bug.cgi?id=53678) - vbscript can not compile CaseClausules that do not use a colon~~
>   - ~~[Bug 53766](https://bugs.winehq.org/show_bug.cgi?id=53766) - vbscript fails to handle SAFEARRAY assignment, access, UBounds, LBounds~~
>   - [Bug 53767](https://bugs.winehq.org/show_bug.cgi?id=53767) - vbscript fails to handle ReDim when variable is not yet created
>   - ~~[Bug 53782](https://bugs.winehq.org/show_bug.cgi?id=53782) - vbscript can not compile ReDim with list of variables~~
>   - ~~[Bug 53783](https://bugs.winehq.org/show_bug.cgi?id=53783) - vbscript can not compile private const expressions~~
>   - ~~[Bug 53807](https://bugs.winehq.org/show_bug.cgi?id=53807) - vbscript fails to redim original array in function when passed byref~~
>   - [Bug 53844](https://bugs.winehq.org/show_bug.cgi?id=53844) - vbscript invoke_vbdisp not handling let property correctly for VT_DISPATCH arguments
>   - ~~[Bug 53866](https://bugs.winehq.org/show_bug.cgi?id=53866) - vbscript fails to handle SAFEARRAY in for...each~~
>   - ~~[Bug 53867](https://bugs.winehq.org/show_bug.cgi?id=53867) - vbscript fails to retrieve property array by index~~
>   - ~~[Bug 53868](https://bugs.winehq.org/show_bug.cgi?id=53868) - vbscript fails to return TypeName for VT_DISPATCH~~
>   - ~~[Bug 53873](https://bugs.winehq.org/show_bug.cgi?id=53873) - vbscript fails to compile Else If when If is on same line~~
>   - [Bug 53877](https://bugs.winehq.org/show_bug.cgi?id=53877) - vbscript compile_assignment assertion when assigning multidimensional array by indices
>   - ~~[Bug 53888](https://bugs.winehq.org/show_bug.cgi?id=53888) - vbscript does not allow Mid on non VT_BSTR~~
>   - [Bug 53889](https://bugs.winehq.org/show_bug.cgi?id=53889) - vbscript does not support Get_Item call on IDispatch objects
>   - [Bug 54177](https://bugs.winehq.org/show_bug.cgi?id=54177) - vbscript fails to compile sub call when argument expression contains multiplication
>   - [Bug 54221](https://bugs.winehq.org/show_bug.cgi?id=54221) - vbscript: missing support for GetRef
>   - ~~[Bug 54234](https://bugs.winehq.org/show_bug.cgi?id=54234) - vbscript fails to compile when colon follows Else in If...Else~~
>   - [Bug 54291](https://bugs.winehq.org/show_bug.cgi?id=54291) - vbscript stuck in endless for loop when UBound on Empty and On Error Resume Next
>   - ~~[Bug 54456](https://bugs.winehq.org/show_bug.cgi?id=54456) - vbscript memory leak in For Each with SafeArray as group~~
>   - ~~[Bug 54457](https://bugs.winehq.org/show_bug.cgi?id=54457) - vbscript memory leaks in interp_redim_preserve~~
>   - ~~[Bug 54458](https://bugs.winehq.org/show_bug.cgi?id=54458) - vbscript memory leaks in Global_Split~~
>   - ~~[Bug 54489](https://bugs.winehq.org/show_bug.cgi?id=54489) - vbscript Abs on BSTR returns invalid value~~
>   - ~~[Bug 54490](https://bugs.winehq.org/show_bug.cgi?id=54490) - vbscript fails to compile when statement follows ElseIf~~
>   - ~~[Bug 54493](https://bugs.winehq.org/show_bug.cgi?id=54493) - vbscript fails to compile concat when used without space and expression begins with H~~
>   - ~~[Bug 54731](https://bugs.winehq.org/show_bug.cgi?id=54731) - vbscript: stack_pop_bool doesn't support floats or ole color~~
>   - ~~[Bug 54978](https://bugs.winehq.org/show_bug.cgi?id=54978) - vbscript fails to compile Sub when End Sub on same line~~
>   - [Bug 55006](https://bugs.winehq.org/show_bug.cgi?id=55006) - vbscript single line if else without else body fails compilation
>   - [Bug 55037](https://bugs.winehq.org/show_bug.cgi?id=55037) - vbscript: Colon on new line after Then fails
>   - ~~[Bug 55042](https://bugs.winehq.org/show_bug.cgi?id=55042) - IDictionary::Add() fails to add entries with numerical keys that have the same hashes~~
>   - ~~[Bug 55052](https://bugs.winehq.org/show_bug.cgi?id=55052) - For loop where right bound is string coercion issue~~
>   - [Bug 55093](https://bugs.winehq.org/show_bug.cgi?id=55093) - vbscript: if boolean condition should work without braces
>   - ~~[Bug 55185](https://bugs.winehq.org/show_bug.cgi?id=55185) - vbscript round does not handle numdecimalplaces argument~~
>   - ~~[Bug 55931](https://bugs.winehq.org/show_bug.cgi?id=55931) - vbscript: empty MOD 100000 returns garbage instead of 0~~
>   - ~~[Bug 55969](https://bugs.winehq.org/show_bug.cgi?id=55969) - vbscript fails to return TypeName for Nothing~~

> - Add support for `Scripting.FileSystemObject` and `Scripting.Dictionary` leveraging Wine's `scrrun` code.
> - Add support for `E_NOTIMPL` commands to Wine's VBScript engine:
>   - `Execute`
>   - `ExecuteGlobal`
>   - `Eval`
>   - `GetRef`

> ***COM / ATL***
> - Visual Pinball uses [COM](https://learn.microsoft.com/en-us/windows/win32/com/the-component-object-model) as a bridge between the scripting engine and itself. The scripting engine uses COM to access other components such as [VPinMAME](https://github.com/vpinball/pinmame) and [B2S.Server](https://github.com/vpinball/b2s-backglass).
> - Visual Pinball uses [ATL](https://learn.microsoft.com/en-us/cpp/atl/active-template-library-atl-concepts?view=msvc-170) to dispatch events from itself to the scripting engine, ex: `DISPID_GameEvents_Init` and `DISPID_HitEvents_Hit`
>
> **Solution**
> - Leverage just enough Wine source code to set up the platform to match Windows, ex: `wchar_t` is 2 bytes in Windows while 4 bytes on most other platforms. (`_WINE_UNICODE_NATIVE_`)
> - Provide stubs for as much COM and ATL code as possible by using Wine and [ReactOS](https://github.com/reactos/reactos/tree/master/sdk/lib/atl) source code.
> - Embed Wine's VBScript engine instead of trying to replicate Windows [RPC](https://learn.microsoft.com/en-us/windows/win32/rpc/rpc-start-page).
> - Replace Windows `typelib` magic with class methods for `GetIDsOfNames`, `Invoke`, `FireDispID`, and `GetDocumentation`
> - Create two `idlparser` utilities that read [Interface Definition File](https://learn.microsoft.com/en-us/windows/win32/midl/interface-definition-idl-file) and automatically generates C and C++ code for `GetIDsOfNames`, `Invoke`, `FireDispID`, and `GetDocumentation`
> - Embed `libpinmame` to provide support for PinMAME based games. [Future]

> ***Graphics***
> - Visual Pinball uses either DirectX or OpenGL for graphics.
>
> **Solution**
> - MacOS uses OpenGL 4.1.
> - Linux (Ubuntu 22.04) uses OpenGL 4.6.
> - Android, iOS, tvOS, Raspberry PI, and RK3588 use OpenGLES 3.0.

> ***Sound***
> - Visual Pinball uses DirectSound for playing `.wav` files and [BASS](http://www.un4seen.com/) for playing all other audio files.
>
> **Solution**
> - Use the `ONLY_USE_BASS` preprocessor definition which uses BASS for all audio files including `.wav` files.

> ***Input***
> - Visual Pinball uses XInput and either DirectInput 8 or SDL2 for getting keyboard and joystick info.
>
> **Solution**
> - Make a `SDLK_TO_DIK` translation table that converts SDL Keys to DirectInput Keys.
> - Add additional support for `SDL_KEYUP` and `SDL_KEYDOWN` events.

> ***Updating***
> - The Visual Pinball repository is constantly updated with new features and bug fixes.
>
> **Solution**
> - Updates must be easy to rebase, so all changes must be wrapped in:
>
>   ```
>   #ifdef __STANDALONE__
>   #endif
>   ```
>   or
>   ```
>   #ifndef __STANDALONE__
>   #else
>   #endif
>   ```

## Compiling

### MacOS (arm64)

In a terminal execute the following:
```
brew install cmake bison curl
export PATH="$(brew --prefix bison)/bin:$PATH"
git clone -b standalone https://github.com/vpinball/vpinball
cd vpinball/standalone/macos-arm64
./external.sh
cd ../..
cp standalone/cmake/CMakeLists_gl-macos-arm64.txt CMakeLists.txt
cmake -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build -- -j$(sysctl -n hw.ncpu)
```

*Note:* Make sure `SDL2`, `SDL2_image`, and `freeimage` are not installed by `brew`, otherwise you may have compile issues

### MacOS (x64)

In a terminal execute the following:
```
brew install cmake bison curl
export PATH="$(brew --prefix bison)/bin:$PATH"
git clone -b standalone https://github.com/vpinball/vpinball
cd vpinball/standalone/macos-x64
./external.sh
cd ../..
cp standalone/cmake/CMakeLists_gl-macos-x64.txt CMakeLists.txt
cmake -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build -- -j$(sysctl -n hw.ncpu)
```

*Note:* Make sure `SDL2`, `SDL2_image`, and `freeimage` are not installed by `brew`, otherwise you may have compile issues

### iOS (arm64)

In a terminal execute the following:
```
brew install cmake bison curl ios-deploy fastlane
export PATH="$(brew --prefix bison)/bin:$PATH"
git clone -b standalone https://github.com/vpinball/vpinball
cd vpinball/standalone/ios
./external.sh
cd ../..
cp standalone/cmake/CMakeLists_gl-ios-arm64.txt CMakeLists.txt
cmake -G Xcode -B XCode
open XCode/vpinball.xcodeproj
```

### tvOS

In a terminal execute the following:
```
brew install cmake bison curl ios-deploy fastlane
export PATH="$(brew --prefix bison)/bin:$PATH"
git clone -b standalone https://github.com/vpinball/vpinball
cd vpinball/standalone/tvos
./external.sh
cd ../..
cp standalone/cmake/CMakeLists_gl-tvos-arm64.txt CMakeLists.txt
cmake -G Xcode -B XCode
open XCode/vpinball.xcodeproj
```

*Note:* BASS for tvOS is not publicly available for open source projects. You will need to buy an iOS license. More information can be found [here](https://www.un4seen.com/).

### Android (arm64-v8a)

```
brew install cmake bison curl
export PATH="$(brew --prefix bison)/bin:$PATH"
export JAVA_HOME=$(/usr/libexec/java_home -v 11.0.16.1)
export ANDROID_HOME=/Users/jmillard/Library/Android/sdk
export ANDROID_NDK=/Users/jmillard/Library/Android/sdk/ndk/25.1.8937393
export ANDROID_NDK_HOME=/Users/jmillard/Library/Android/sdk/ndk/25.1.8937393
git clone -b standalone https://github.com/vpinball/vpinball
cd vpinball/standalone/android
./external.sh
cd ../..
cp standalone/cmake/CMakeLists_gl-android-arm64-v8a.txt CMakeLists.txt
cmake -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build -- -j$(sysctl -n hw.ncpu)
cd standalone/android/android-project
./gradlew installDebug
$ANDROID_HOME/platform-tools/adb shell am start -n org.vpinball.app/org.vpinball.app.VPinballActivity
$ANDROID_HOME/platform-tools/adb -d logcat org.vpinball.app
```

### Linux (Ubuntu 22.04)

In a terminal execute the following:
```
sudo apt install git build-essential cmake bison curl zlib1g-dev libdrm-dev libgbm-dev libglu1-mesa-dev libegl-dev libudev-dev libx11-dev libxrandr-dev
git clone -b standalone https://github.com/vpinball/vpinball
cd vpinball/standalone/linux
./external.sh
cd ../..
cp standalone/cmake/CMakeLists_gl-linux-x64.txt CMakeLists.txt
cmake -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build -- -j$(nproc)
```

### Linux (Fedora 37)

In a terminal execute the following:
```
sudo dnf groupinstall "Development Tools"
sudo dnf install gcc-c++ cmake bison curl
git clone -b standalone https://github.com/vpinball/vpinball
cd vpinball/standalone/linux
./external.sh
cd ../..
cp standalone/cmake/CMakeLists_gl-linux-x64.txt CMakeLists.txt
cmake -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build -- -j$(nproc)
```

### RK3588 (Armbian)

Start with a [Armbian 23.02 Jammy LI](https://www.armbian.com/orangepi-5/) image and execute the following:

In a terminal execute the following:
```
sudo add-apt-repository ppa:liujianfeng1994/panfork-mesa
sudo add-apt-repository ppa:liujianfeng1994/rockchip-multimedia
sudo apt update
sudo apt dist-upgrade
sudo apt install mali-g610-firmware rockchip-multimedia-config
sudo apt-get install git pkg-config cmake bison zlib1g-dev libdrm-dev libgbm-dev libgles2-mesa-dev libgles2-mesa libudev-dev
git clone -b standalone https://github.com/vpinball/vpinball
cd vpinball/standalone/rk3588
./external.sh
cd ../..
cp standalone/cmake/CMakeLists_gl-rk3588-aarch64.txt CMakeLists.txt
cmake -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build
```

### Raspberry Pi 4

Start with a [Raspberry Pi OS Lite (64-Bit)](https://www.raspberrypi.com/software/operating-systems/#raspberry-pi-os-64-bit) image and execute the following:
```
sudo apt-get install git pkg-config cmake bison zlib1g-dev libdrm-dev libgbm-dev libgles2-mesa-dev libgles2-mesa libudev-dev
git clone -b standalone https://github.com/vpinball/vpinball
cd vpinball/standalone/rpi
./external.sh
cd ../..
cp standalone/cmake/CMakeLists_gl-rpi-aarch64.txt CMakeLists.txt
cmake -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build
```

## Running

Go to the build directory:
```
cd build
```

To list all command line arguments:
```
./VPinballX_GL -h
```

To launch a table, execute the following:
```
./VPinballX_GL -play <table.vpx>
```

To launch a table in windowed mode, execute the following:
```
./VPinballX_GL -DisableTrueFullscreen -play <table.vpx>
```

To extract a table script, execute the following:
```
./VPinballX_GL -extractvbs <table.vpx>
```

To list all available fullscreen resolutions, execute the following: 
```
./VPinballX_GL -listres
```

To list all available audio devices, execute the following:
```
./VPinballX_GL -listsnd
```

## Joystick

The joystick is currently mapped to an XBox controller layout.

| Action | Button |
| --- | --- |
| Left Flipper | `Left Shoulder` |
| Right Flipper | `Right Shoulder` |
| Left Magna Save | `Left Stick` |
| Right Magna Save | `Right Stick` |
| Center Tilt | `D-pad Up` |
| Left Tilt | `D-pad Left` |
| Right Tilt | `D-pad Right` |
| Plunger | `D-pad Down` |
| Add Credit | `A` |
| Start | `B` |
| FPS | `X` |
| Cancel | `Y` |
| Fire | `Guide` |

## Keyboard

If you want to customize the keyboard input, update the `; Keyboard input mappings` section in the `VPinballX.ini` file.

The settings are decimal values based on DirectInput `DIK_` values found [here](https://gitlab.winehq.org/wine/wine/-/blob/master/include/dinput.h#L470-607).

| Key | Value | Notes |
| --- | --- | --- |
| ESCAPE | 1 | |
| 1 | 2 | |
| 2 | 3 | |
| 3 | 4 | |
| 4 | 5 | |
| 5 | 6 | |
| 6 | 7 | |
| 7 | 8 | |
| 8 | 9 | |
| 9 | 10 | |
| 0 | 11 | |
| MINUS | 12 | - on main keyboard |
| EQUALS | 13 | |
| BACK | 14 | Backspace |
| TAB | 15 | |
| Q | 16 | |
| W | 17 | |
| E | 18 | |
| R | 19 | |
| T | 20 | |
| Y | 21 | |
| U | 22 | |
| I | 23 | |
| O | 24 | |
| P | 25 | |
| LBRACKET | 26 | |
| RBRACKET | 27 | |
| RETURN | 28 | Enter on main keyboard |
| LCONTROL | 29 | |
| A | 30 | |
| S | 31 | |
| D | 32 | |
| F | 33 | |
| G | 34 | |
| H | 35 | |
| J | 36 | |
| K | 37 | |
| L | 38 | |
| SEMICOLON | 39 | |
| APOSTROPHE | 40 | |
| GRAVE | 41 | |
| LSHIFT | 42 | |
| BACKSLASH | 43 | |
| Z | 44 | |
| X | 45 | |
| C | 46 | |
| V | 47 | |
| B | 48 | |
| N | 49 | |
| M | 50 | |
| COMMA | 51 | |
| PERIOD | 52 | . on main keyboard |
| SLASH | 53 | / on main keyboard |
| RSHIFT | 54 | |
| MULTIPLY | 55 | * on numeric keypad |
| LMENU | 56 | Left Alt |
| SPACE | 57 | |
| CAPITAL | 58 | |
| F1 | 59 | |
| F2 | 60 | |
| F3 | 61 | |
| F4 | 62 | |
| F5 | 63 | |
| F6 | 64 | |
| F7 | 65 | |
| F8 | 66 | |
| F9 | 67 | |
| F10 | 68 | |
| NUMLOCK | 69 | |
| SCROLL | 70 | Scroll Lock |
| NUMPAD7 | 71 | |
| NUMPAD8 | 72 | |
| NUMPAD9 | 73 | |
| SUBTRACT | 74 | - on numeric keypad |
| NUMPAD4 | 75 | |
| NUMPAD5 | 76 | |
| NUMPAD6 | 77 | |
| ADD | 78 | + on numeric keypad |
| NUMPAD1 | 79 | |
| NUMPAD2 | 80 | |
| NUMPAD3 | 81 | |
| NUMPAD0 | 82 | |
| DECIMAL | 83 | . on numeric keypad |
| F11 | 87 | |
| F12 | 88 | |
| F13 | 100 | |
| F14 | 101 | |
| F15 | 102 | |
| NUMPADEQUALS | 141 | = on numeric keypad |
| AT | 145 | |
| COLON | 146 | |
| UNDERLINE | 147 | |
| STOP | 149 | |
| NUMPADENTER | 156 | Enter on numeric keypad |
| RCONTROL | 157 | |
| NUMPADCOMMA | 179 | , on numeric keypad |
| DIVIDE | 181 | / on numeric keypad |
| RMENU | 184 | Right Alt |
| PAUSE | 197 | Pause |
| HOME | 199 | Home on arrow keypad |
| UP | 200 | Up Arrow on arrow keypad |
| PRIOR | 201 | Page Up on arrow keypad |
| LEFT | 203 | Left Arrow on arrow keypad |
| RIGHT | 205 | Right Arrow on arrow keypad |
| END | 207 | End on arrow keypad |
| DOWN | 208 | Down Arrow on arrow keypad |
| NEXT | 209 | Page Down on arrow keypad |
| INSERT | 210 | Insert on arrow keypad |
| DELETE | 211 | Delete on arrow keypad |
| LWIN | 219 | Left Windows |
| RWIN | 220 | Right Windows |

## ZeDMD

MacOS, Linux, and Android builds support [ZeDMD](https://www.pincabpassion.net/t14796-zedmd-installation-english) displays.

## Paths

Here are some important paths and files for MacOS and Linux:

| Path | Notes |
| --- | --- |
`$HOME/.vpinball/user` | VPinball user directory |
`$HOME/.vpinball/music` | VPinball music directory |
`$HOME/.vpinball/pinmame` | VPinball generated PinMAME settings, ex. `sound`, `cheat`, `dmd_red` |
`$HOME/.vpinball/VPinballX.ini` | VPinball settings (created on first run) |
`$HOME/.vpinball/VPReg.ini` | <sup>*</sup>VPinball table settings |
`$HOME/.vpinball/vpinball.log` | VPinball log file |
`$HOME/.pinmame` | <sup>**</sup>PinMAME root directory |
`$HOME/.pinmame/roms` | <sup>**</sup>PinMAME ROMs directory |
`$HOME/.pinmame/nvram` | <sup>**</sup>PinMAME NVRAM directory  |
`$HOME/.pinmame/altcolor` | <sup>**</sup>Serum colorizations directory `<rom_name>/<rom.cRZ>` |
`$HOME/.pinmame/ini` | <sup>**</sup>VPinball PinMAME settings directory |

Notes:

- The command-line option `-PrefPath <path>` can be used to override `$HOME/.vpinball`
- <sup>*</sup>The `VPRegPath` entry in the `[Standalone]` section of `VPinballX.ini` can be used to override the VPinball table settings. If set to `./`, the current table path will be used.
- <sup>**</sup>The `PinMAMEPath` entry in the `[Standalone]` section of `VPinballX.ini` will only be used if no `pinmame` folder exists in the current table directory. If not set `$HOME/.pinmame` will be used.

## Debugging

Debugging can be done using [Visual Studio Code](https://code.visualstudio.com/).

### MacOS

Perform the steps outlined above in *Compiling* and *Running*.

In Visual Studio Code:
  - Install the [`C/C++ Extension Pack`](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools-extension-pack) extension.
  - Open the `vpinball` folder.
  - If prompted, select the latest version of clang, ex: `Clang 14.0.0 arm64-apple-darwin21.6.0`
  - Go to `Settings` -> `CMake: Debug Config` and click `Edit in settings.json`
  - Update `settings.json` with:
  ```
      "cmake.debugConfig": {
         "args": [ "-DisableTrueFullscreen", "-Play", "assets/exampleTable.vpx" ],
      }
  ```
  - Click the bug button (to the left of the play button) in the bottom bar

### Raspberry Pi 4

Perform the steps outlined above in *Compiling* and *Running*.

Since we are using the Lite version of Raspberry Pi OS, there is no X11 to run the full version Visual Studio Code. Instead will we use the [Remote Development](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.vscode-remote-extensionpack) extension in Visual Studio Code running on a host computer.

**NOTE**: The 2GB version of the Raspberry Pi 4 can easily run out of memory and become unresponsive.

Make sure SSH is configured and enabled on your Raspberry Pi.

On the host computer, in Visual Studio Code:
  - Install the [`Remote Development`](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.vscode-remote-extensionpack) extension.
  - Select `Remote-SSH: Connect to Host...` from the Command Palette (F1, ⇧⌘P)
  - Enter the `user@address` of the Raspberry Pi.
  - Once connected, install the [`C/C++ Extension Pack`](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools-extension-pack) extension.
  - If prompted, select the latest version of gcc, ex: `GCC 10.2.1 aarch64-linux-gnu`
  - Go to `Settings` (`Remote SSH`) -> `CMake: Debug Config` and click `Edit in settings.json`
  - Update `settings.json` with:
  ```
    "cmake.parallelJobs": 1,
    "cmake.generator": "Unix Makefiles",
    "cmake.debugConfig": {
        "args": [ "-Play", "assets/exampleTable.vpx" ],
    }
  ```
  - Click the bug button (to the left of the play button) in the bottom bar

### Change Log

To keep up with all the changes in master, and make it easier to rebase, this branch is a single commit -- until most tables work with no vbs changes.

The downside of this approach is not accurately keeping track of history:

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
  * Fixed missing Hankin AlphaNumeric layout (@freezy, @toxie, @somatik)
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
  * Wine hack to support `numdecimalplaces` in `Global_Round` (@somatik)

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
  * Replace `E_NOTIMPL` with `PLOGW` for PuP (@somatik)

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
  * Add a Paths section to this readme (@somatik)

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

## Workarounds

### My game is not displaying a DMD

Many newer tables render the DMD inside Visual Pinball's window using Textbox objects. Other tables will rely on positioning VPinMAME or FlexDMD's application window over Visual Pinball's window. Since standalone does not support rendering a separate VPinMAME or FlexDMD window, you need to determine if the table already has a Textbox with the visible property set to false. You can do this by searching the Live Editor. Some possible names to search for are: `Textbox01`, `Textbox001`, `Scorebox`, `Scoretext`, `DMD`.

Standalone also adds an `ImplicitDMD` object to all tables just for this specific reason. To enable it, you'll need to add some code to the table script.

For tables that use VPinMAME:

```
Dim UseVPMDMD : UseVPMDMD = true 
' Dim UseVPMColoredDMD : UseVPMColoredDMD = true

Sub ImplicitDMD_Init
   Me.x = 30
   Me.y = 30
   Me.width = 128 * 2
   Me.height = 32 * 2
   Me.fontColor = RGB(255, 0, 0)
   Me.visible = true
   Me.intensityScale = 1.5
End Sub
```

Notes:

- This must appear before the `LoadVPM` call. 
- Always check the table for existing `UseVPMDMD` or `UseVPMColoredDMD` variables.

For tables that use FlexDMD:

```
Sub ImplicitDMD_Init
   Me.x = 30
   Me.y = 30
   Me.width = 128 * 2
   Me.height = 32 * 2
   Me.fontColor = RGB(255, 255, 255)
   Me.visible = true
   Me.intensityScale = 1.5
   Me.timerenabled = true
End Sub

Sub ImplicitDMD_Timer
   Dim DMDp: DMDp = FlexDMD.DMDColoredPixels
   If Not IsEmpty(DMDp) Then
      DMDWidth = FlexDMD.Width
      DMDHeight = FlexDMD.Height
      DMDColoredPixels = DMDp
   End If
End Sub
```

Notes:

- Replace the `FlexDMD` variable accordingly.
- Replace `DMDColoredPixels` with `DMDPixels` if your FlexDMD is not RGB24.


### DTArray (Drop Targets)

Wine's vbscript engine cannot handle multi-dimension array assignments. For example:

```
DTArray(i)(4) = DTAnimate(DTArray(i)(0),DTArray(i)(1),DTArray(i)(2),DTArray(i)(3),DTArray(i)(4))
```

1) Switch to use the `DropTarget` class, by adding the following code:

```
Class DropTarget
  Private m_primary, m_secondary, m_prim, m_sw, m_animate, m_isDropped

  Public Property Get Primary(): Set Primary = m_primary: End Property
  Public Property Let Primary(input): Set m_primary = input: End Property

  Public Property Get Secondary(): Set Secondary = m_secondary: End Property
  Public Property Let Secondary(input): Set m_secondary = input: End Property

  Public Property Get Prim(): Set Prim = m_prim: End Property
  Public Property Let Prim(input): Set m_prim = input: End Property

  Public Property Get Sw(): Sw = m_sw: End Property
  Public Property Let Sw(input): m_sw = input: End Property

  Public Property Get Animate(): Animate = m_animate: End Property
  Public Property Let Animate(input): m_animate = input: End Property

  Public Property Get IsDropped(): IsDropped = m_isDropped: End Property
  Public Property Let IsDropped(input): m_isDropped = input: End Property

  Public default Function init(primary, secondary, prim, sw, animate, isDropped)
    Set m_primary = primary
    Set m_secondary = secondary
    Set m_prim = prim
    m_sw = sw
    m_animate = animate
    m_isDropped = isDropped

    Set Init = Me
  End Function
End Class
```

2) Update the DT definitions to use `DropTarget` instead of `Array`:

```
DT7 = Array(dt1, dt1a, pdt1, 7, 0, false)
DT27 = Array(dt2, dt2a, pdt2, 27, 0, false)
DT37 = Array(dt3, dt3a, pdt3, 37, 0, false)
```

becomes:

```
Set DT7 = (new DropTarget)(dt1, dt1a, pdt1, 7, 0, false)
Set DT27 = (new DropTarget)(dt2, dt2a, pdt2, 27, 0, false)
Set DT37 = (new DropTarget)(dt3, dt3a, pdt3, 37, 0, false)
```

3) Search and replace:

| From | To | Vi |
| --- | --- | --- |
| `DTArray(i)(0)` | `DTArray(i).primary` | `:%s/DTArray(i)(0)/DTArray(i).primary/g` |
| `DTArray(i)(1)` | `DTArray(i).secondary` | `:%s/DTArray(i)(1)/DTArray(i).secondary/g` |
| `DTArray(i)(2)` | `DTArray(i).prim` | `:%s/DTArray(i)(2)/DTArray(i).prim/g` |
| `DTArray(i)(3)` | `DTArray(i).sw` | `:%s/DTArray(i)(3)/DTArray(i).sw/g` |
| `DTArray(i)(4)` | `DTArray(i).animate` | `:%s/DTArray(i)(4)/DTArray(i).animate/g` |
| `DTArray(i)(5)` | `DTArray(i).isDropped` | `:%s/DTArray(i)(5)/DTArray(i).isDropped/g` |
| `DTArray(ind)(5)` | `DTArray(ind).isDropped` | `:%s/DTArray(ind)(5)/DTArray(ind).isDropped/g` |

### STArray (Standup Targets)

Wine's vbscript engine cannot handle multi-dimension array assignments. For example:

```
STArray(i)(3) = STCheckHit(Activeball,STArray(i)(0))
```

1) Switch to use the `StandupTarget` class, by adding the following code:

```
Class StandupTarget
  Private m_primary, m_prim, m_sw, m_animate

  Public Property Get Primary(): Set Primary = m_primary: End Property
  Public Property Let Primary(input): Set m_primary = input: End Property

  Public Property Get Prim(): Set Prim = m_prim: End Property
  Public Property Let Prim(input): Set m_prim = input: End Property

  Public Property Get Sw(): Sw = m_sw: End Property
  Public Property Let Sw(input): m_sw = input: End Property

  Public Property Get Animate(): Animate = m_animate: End Property
  Public Property Let Animate(input): m_animate = input: End Property

  Public default Function init(primary, prim, sw, animate)
    Set m_primary = primary
    Set m_prim = prim
    m_sw = sw
    m_animate = animate

    Set Init = Me
  End Function
End Class
```

2) Update the ST definitions to use `StandupTarget` instead of `Array`:

```
ST41 = Array(sw41, Target_Rect_Fat_011_BM_Lit_Room, 41, 0)
ST42 = Array(sw42, Target_Rect_Fat_010_BM_Lit_Room, 42, 0)
ST43 = Array(sw43, Target_Rect_Fat_005_BM_Lit_Room, 43, 0)
```

becomes:

```
Set ST41 = (new StandupTarget)(sw41, Target_Rect_Fat_011_BM_Lit_Room, 41, 0)
Set ST42 = (new StandupTarget)(sw42, Target_Rect_Fat_010_BM_Lit_Room, 42, 0)
Set ST43 = (new StandupTarget)(sw43, Target_Rect_Fat_005_BM_Lit_Room, 43, 0)
```

3) Search and replace:

| From | To | Vi |
| --- | --- | --- |
| `STArray(i)(0)` | `STArray(i).primary` | `:%s/STArray(i)(0)/STArray(i).primary/g` |
| `STArray(i)(1)` | `STArray(i).prim` | `:%s/STArray(i)(1)/STArray(i).prim/g` |
| `STArray(i)(2)` | `STArray(i).sw` | `:%s/STArray(i)(2)/STArray(i).sw/g` |
| `STArray(i)(3)` | `STArray(i).animate` | `:%s/STArray(i)(3)/STArray(i).animate/g` |

### BSize and BMass Constants

Some scripts define `BSize` and `BMass` as `Const` and `core.vbs` re-defines as `Dim`. If this causes errors:

1) Replace the following:

```
Const BSize = 25
Const BMass = 1.7
```

with

```
Dim BSize : BSize = 25
Dim BMass : BMass = 1.7
```

or convert to use:

```
Const BallSize = 50
Const BallMass = 1
```

## Miscellaneous

To generate the COM proxy files, we need IDL files. Visual Pinball and Visual PinMAME have IDL files in their repositories. FlexDMD, B2SServer, and PuP do not supply IDL files as they are either C#, VB, or closed source.

To generate an IDL file from a DLL or EXE:

- Open `Windows Developer Command Prompt`

```
tlbexp C:\Users\jsm17\OneDrive\Desktop\B2SBackglassServer.dll 
```

- Open `oleview` (x64) in `"C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\oleview.exe"`

- `File` > `View TypeLib...` > Select `B2SBackglassServer.tlb`

- `File` > `Save As...` > Enter `B2S.idl`

- The saved IDL file will be encoded in UTF-16. On MacOS or Linux convert the file to UTF-8:

```
iconv -f utf-16 -t utf-8 B2S.idl > B2S.idl_tmp
mv B2S.idl_tmp B2S.idl
```

To build header file for VPinMAME.idl: (Thanks to @bshanks and @gcenx83)

```
brew install llvm 
git clone git://source.winehq.org/git/wine.git
git clone git@github.com:vpinball/pinmame.git
cd wine
export PATH=/opt/homebrew/opt/llvm/bin:/opt/homebrew/opt/bison/bin:/System/Cryptexes/App/usr/bin:/usr/bin:/bin:/usr/sbin:/sbin
#export PATH="$(brew --prefix llvm)/bin:$PATH"
#export PATH="$(brew --prefix bison)/bin:$PATH"
./configure --without-freetype
make -j10
tools/widl/widl -o ../vpinmame_i.h --nostdinc -Ldlls/\* -Iinclude -D__WINESRC__ -D_UCRT ../pinmame/src/win32com/VPinMAME.idl
```

## Table Patches

Some older tables need to have their scripts patched in order to run.

If you find a table that does not work, please check the [vpx-standalone-scripts](https://github.com/jsm174/vpx-standalone-scripts) repository.

## Shoutouts

Wine and the amazing people who support the VBScript engine
- Robert Wilhelm
- Nikolay Sivov
- Jacek Caban  

The people who make this such an exciting hobby to be a part of
- @Apophis, @Bord, @ClarkKent, @Cupiii, @ecurtz, @freezy, @Iaaki, @Lucidish, @mkalkbrenner, @Niwak, @onevox, @Scottacus64, @Somatik, @superhac, @Thalamus, @toxie, @wylte, @Zedrummer
- and the rest of the Visual Pinball community!
