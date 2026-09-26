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

### No audio with Linux release binaries on NixOS

If a table runs without sound and the log repeatedly reports
`Failed to create stream: Parameter 'dst_spec->format' is invalid`, SDL may
be unable to load an audio backend. On NixOS, a running PipeWire service does
not necessarily mean that the bundled SDL library can find
`libpipewire-0.3.so.0`.

If that library is available at
`/run/current-system/sw/lib/libpipewire-0.3.so.0`, try launching from the
extracted VPX directory with:

```bash
LD_LIBRARY_PATH="/run/current-system/sw/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}" \
  ./VPinballX_BGFX -play /path/to/table.vpx
```

For a persistent fix limited to the bundled SDL library, back it up once and
add the library search path with `patchelf` (run from the same directory):

```bash
cp -pL --no-clobber libSDL3.so.0 libSDL3.so.0.before-nixos-audio-fix
nix shell nixpkgs#patchelf --command patchelf \
  --add-rpath /run/current-system/sw/lib "$(readlink -f libSDL3.so.0)"
```

This preserves any existing library search paths. Restart VPX normally and
check that the log contains `Audio device initialized` with `Driver: pipewire`.
If it is still silent, check which output device the log names and its volume
in the system sound settings. Replacing the bundled SDL library during an
update may require applying the fix again.

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
