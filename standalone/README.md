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

> ***VBScript***
> - Visual Pinball uses [VBScript](https://learn.microsoft.com/en-us/previous-versions/t0aew7h6(v=vs.85)) as the scripting engine for tables.
>
> **Solution**
> - Leverage the VBScript engine from [Wine](https://github.com/wine-mirror/wine/tree/master/dlls/vbscript).
> - Fix bugs in Wine's VBScript engine:
>   - ~~[Bug 53644](https://bugs.winehq.org/show_bug.cgi?id=53644) - vbscript can not compile classes with lists of private / public / dim declarations~~
>   - ~~[Bug 53670](https://bugs.winehq.org/show_bug.cgi?id=53670) - vbscript can not compile if expressions with reversed gte, lte, (=>, =<)~~
>   - ~~[Bug 53676](https://bugs.winehq.org/show_bug.cgi?id=53676) - vbscript can not exec_script - invalid number of arguments for Randomize~~
>   - ~~[Bug 53678](https://bugs.winehq.org/show_bug.cgi?id=53678) - vbscript can not compile CaseClausules that do not use a colon~~
>   - ~~[Bug 53766](https://bugs.winehq.org/show_bug.cgi?id=53766) - vbscript fails to handle SAFEARRAY assignment, access, UBounds, LBounds~~
>   - ~~[Bug 53767](https://bugs.winehq.org/show_bug.cgi?id=53767) - vbscript fails to handle ReDim when variable is not yet created~~
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
>   - ~~[Bug 56139](https://bugs.winehq.org/show_bug.cgi?id=56139) - scrrun: Dictionary does not allow storing at key Undefined~~
>   - [Bug 56280](https://bugs.winehq.org/show_bug.cgi?id=56280) - vbscript: String coerced to Integer instead of Long?
>   - [Bug 56281](https://bugs.winehq.org/show_bug.cgi?id=56281) - vbscript: String number converted to ascii value instead of parsed value
>   - ~~[Bug 56464](https://bugs.winehq.org/show_bug.cgi?id=56464) - vbscript: Join on array with "empty" items fails~~
>   - [Bug 56480](https://bugs.winehq.org/show_bug.cgi?id=56480) - vbscript: underscore line continue issues
>   - ~~[Bug 56781](https://bugs.winehq.org/show_bug.cgi?id=56781) - srcrrun: Dictionary setting item to object fails~~
>   - [Bug 56931](https://bugs.winehq.org/show_bug.cgi?id=56931) - vbscript: Const used before declaration fails (explicit)
>   - [Bug 57511](https://bugs.winehq.org/show_bug.cgi?id=57511) - vbscript: For loop where loop var is not defined throws error without context
>   - ~~[Bug 57563](https://bugs.winehq.org/show_bug.cgi?id=57563) - vbscript: mid() throws when passed VT_EMPTY instead of returning empty string~~
>   - [Bug 58051](https://bugs.winehq.org/show_bug.cgi?id=58051) - vbscript: Dictionary direct Keys/Items access causes parse error
>   - [Bug 58056](https://bugs.winehq.org/show_bug.cgi?id=58056) - vbscript: Directly indexing a Split returns Empty
>   - [Bug 58248](https://bugs.winehq.org/show_bug.cgi?id=58248) - vbscript: Me(Idx) fails to compile

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


## Building

Refer to [make README](../make/README.md#compiling). 

## Running

### Example Table (Initial Test)

**Linux:**
```
./VPinballX_BGFX -play ./build/assets/exampleTable.vpx
```

**macOS:**
```
/Applications/VPinballX_BGFX.app/Contents/MacOS/VPinballX_BGFX -play /Applications/VPinballX_BGFX.app/Contents/Resources/assets/exampleTable.vpx
```

### Command Line Options

**Linux:**
```
./VPinballX_BGFX -h
./VPinballX_BGFX -play <table.vpx>
./bVPinballX_BGFX -extractvbs <table.vpx>
```

**macOS:**
```
/Applications/VPinballX_BGFX.app/Contents/MacOS/VPinballX_BGFX -h
/Applications/VPinballX_BGFX.app/Contents/MacOS/VPinballX_BGFX -play <table.vpx>
/Applications/VPinballX_BGFX.app/Contents/MacOS/VPinballX_BGFX -extractvbs <table.vpx>
```

## Debugging

Debugging can be done using [Visual Studio Code](https://code.visualstudio.com/).

### MacOS

Perform the steps outlined above in *Compiling* and *Running*.

In Visual Studio Code:
  - Install the [`C/C++ Extension Pack`](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools-extension-pack) extension.
  - Open the `vpinball` folder.
  - If prompted, select the latest version of clang, ex: `Clang 17.0.0 arm64-apple-darwin25.0.0`
  - Go to `Settings` -> `CMake: Debug Config` and click `Edit in settings.json`
  - Update `settings.json` with:
  ```
      "cmake.debugConfig": {
         "args": [ "-play", "assets/exampleTable.vpx" ],
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
        "args": [ "-play", "assets/exampleTable.vpx" ],
    }
  ```
  - Click the bug button (to the left of the play button) in the bottom bar

## VBScript Issues

See [vbscript-issues](docs/vbscript.md) for more information.

## Table Patches

Some older tables need to have their scripts patched in order to run.

If you find a table that does not work, please check the [vpx-standalone-scripts](https://github.com/jsm174/vpx-standalone-scripts) repository.

## Shoutouts

Wine and the amazing people who support the VBScript engine
- Robert Wilhelm
- Nikolay Sivov
- Jacek Caban  

The people who make this such an exciting hobby to be a part of
- @Apophis, @Bord, @ClarkKent, @Cupiii, @DeKay, @ecurtz, @evilwraith, @freezy, @gerhard_b, @Iaaki, @Lucidish, @mkalkbrenner, @Niwak, @n-i-x, @onevox, @Scottacus64, @Somatik, @superhac, @Thalamus, @toxie, @wylte, @Zedrummer
- and the rest of the Visual Pinball community!
