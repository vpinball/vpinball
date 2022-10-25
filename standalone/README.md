# Visual Pinball Standalone

*An open source **cross platform** pinball table simulator.*

This project is a modified version of [VPinballX](https://github.com/vpinball/vpinball) that is designed to run on non-Windows platforms.

[![Watch the video](https://img.youtube.com/vi/jK3TbGvTuIA/0.jpg)](https://www.youtube.com/watch?v=jK3TbGvTuIA)

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
>   - [Bug 53670](https://bugs.winehq.org/show_bug.cgi?id=53670) - vbscript can not compile if expressions with reversed gte, lte, (=>, =<)
>   - [Bug 53676](https://bugs.winehq.org/show_bug.cgi?id=53676) - vbscript can not exec_script - invalid number of arguments for Randomize
>   - [Bug 53678](https://bugs.winehq.org/show_bug.cgi?id=53678) - vbscript can not compile CaseClausules that do not use a colon
>   - [Bug 53766](https://bugs.winehq.org/show_bug.cgi?id=53766) - vbscript fails to handle SAFEARRAY assignment, access, UBounds, LBounds
>   - [Bug 53767](https://bugs.winehq.org/show_bug.cgi?id=53767) - vbscript fails to handle ReDim when variable is not yet created
>   - [Bug 53782](https://bugs.winehq.org/show_bug.cgi?id=53782) - vbscript can not compile ReDim with list of variables
>   - [Bug 53783](https://bugs.winehq.org/show_bug.cgi?id=53783) - vbscript can not compile private const expressions
>   - [Bug 53807](https://bugs.winehq.org/show_bug.cgi?id=53807) - vbscript fails to redim original array in function when passed byref
> - Add support for `E_NOTIMPL` commands to Wine's VBScript engine:
>   - `rnd`
>   - `randomize`
>   - `execute global`
>   - `eval`

> ***COM / ATL***
> - Visual Pinball uses [COM](https://learn.microsoft.com/en-us/windows/win32/com/the-component-object-model) as a bridge between the scripting engine and itself. The scripting engine uses COM to access other components such as [VPinMAME](https://github.com/vpinball/pinmame) and [B2S.Server](https://github.com/vpinball/b2s-backglass).
> - Visual Pinball uses [ATL](https://learn.microsoft.com/en-us/cpp/atl/active-template-library-atl-concepts?view=msvc-170) to dispatch events from itself to the scripting engine, ex: `DISPID_GameEvents_Init` and `DISPID_HitEvents_Hit`
>
> **Solution**
> - Leverage just enough Wine source code to set up the platform to match Windows, ex: `wchar_t` is 2 bytes in Windows while 4 bytes on most other platforms. (`_WINE_UNICODE_NATIVE_`)
> - Provide stubs for as much COM and ATL code as possible by using Wine and [ReactOS](https://github.com/reactos/reactos/tree/master/sdk/lib/atl) source code.
> - Embed Wine's VBScript engine instead of trying to replicate Windows [RPC](https://learn.microsoft.com/en-us/windows/win32/rpc/rpc-start-page).
> - Replace Windows `typelib` magic with class methods for `GetIDsOfNames`, `Invoke`, and `FireDispID`
> - Create an helper application (`idlparser`) that reads the `vpinball.idl` [Interface Definition File](https://learn.microsoft.com/en-us/windows/win32/midl/interface-definition-idl-file) and automatically generates code for `GetIDsOfNames`, `Invoke`, and `FireDispID` 
> - Embed `libpinmame` to provide support for PinMAME based games. [Future]

> ***Graphics***
> - Visual Pinball uses either DirectX or OpenGL for graphics.
>
> **Solution**
> - MacOS supports OpenGL 4.1, so shaders need to be modified.
> - Raspberry Pi 4 supports OpenGLES 3.1, so shaders need to be modified. [Future]
> - Linux (Ubuntu 22.04) supports OpenGL 4.6

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

### MacOS

In a terminal execute the following:
```
brew install cmake sdl2 sdl2_ttf freeimage bison
export PATH="$(brew --prefix bison)/bin:$PATH"
git clone -b standalone https://github.com/vpinball/vpinball
cd vpinball 
cp cmake/CMakeLists_gl-osx-x64.txt CMakeLists.txt
cmake -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build
```

### Linux (Ubuntu 22.04)

In a terminal execute the following:
```
sudo apt install git cmake build-essential libsdl2-dev libsdl2-ttf-dev libfreetype-dev libfreeimage-dev bison 
git clone -b standalone https://github.com/vpinball/vpinball
cd vpinball 
cp cmake/CMakeLists_gl-linux-x64.txt CMakeLists.txt
cmake -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build
```

### Raspberry Pi 4

Start with a [Raspberry Pi OS Lite (64-Bit)](https://www.raspberrypi.com/software/operating-systems/#raspberry-pi-os-64-bit) image and execute the following:
```
sudo apt-get install git cmake libgles2-mesa libgles2-mesa-dev libsdl2-dev libsdl2-ttf-dev libfreetype-dev libfreeimage-dev bison
git clone -b standalone https://github.com/vpinball/vpinball
cd vpinball 
cp cmake/CMakeLists_gl-linux-arm64.txt CMakeLists.txt
cmake -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build
```

## Running

To launch the example table, execute the following:
```
cd build
./VPinballX_GL -play ../res/exampleTable.vpx
```

**NOTE**: The Raspberry Pi version will crash fail as we will need to implement OpenGLES 3.1.

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
         "args": [ "-DisableTrueFullscreen", "-Play", "../res/exampleTable.vpx" ],
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
        "args": [ "-Play", "../res/exampleTable.vpx" ],
    }
  ```
  - Click the bug button (to the left of the play button) in the bottom bar 

### Miscellaneous

- To build header file for VPinMAME.idl

```
brew install llvm
git clone git://source.winehq.org/git/wine.git
git clone git@github.com:vpinball/pinmame.git
cd wine
export MACOSX_DEPLOYMENT_TARGET="10.14"
export LDFLAGS="-Wl,-rpath,/opt/X11/lib"
export PATH="$(brew --prefix llvm)/bin:$PATH"
export PATH="$(brew --prefix bison)/bin:$PATH"
./configure
make -j10
tools/widl/widl -o ../vpinmame.h --nostdinc -Ldlls/\* -Iinclude -D__WINESRC__ -D_UCRT ../pinmame/src/win32com/VPinMAME.idl
```

### Shoutouts

- Wine and the amazing people who support the VBScript engine
  - Robert Wilhelm
  - Nikolay Sivov 

- The people who make this such an exciting hobby to be a part of
  - @Bord, @Cupiii, @ecurtz, @freezy, @Iaaki, @Lucidish, @mkalkbrenner, @Niwak, @Scottacus64, @Thalamus, @toxie
  - and the rest of the Visual Pinball community!
