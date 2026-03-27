# Visual Pinball Standalone

*An open source **cross platform** pinball table simulator.*

This sub-project of VPinballX is designed to run on non-Windows platforms.

[![Watch the video](https://img.youtube.com/vi/jK3TbGvTuIA/0.jpg)](https://www.youtube.com/watch?v=xjkgzIVL_QU)

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

## Table Patches

Some older tables need to have their scripts patched in order to run.

If you find a table that does not work, please check the [vpx-standalone-scripts](https://github.com/jsm174/vpx-standalone-scripts) repository.

## Shoutouts

The people who make this such an exciting hobby to be a part of
- @Apophis, @Bord, @ClarkKent, @Cupiii, @DeKay, @ecurtz, @evilwraith, @freezy, @gerhard_b, @Iaaki, @Lucidish, @mkalkbrenner, @Niwak, @n-i-x, @onevox, @Scottacus64, @Somatik, @superhac, @Thalamus, @toxie, @wylte, @Zedrummer
- and the rest of the Visual Pinball community!
