# How to build

## Quickstart Windows / Visual Studio 2015 (and newer) variant: dependencies are downloaded, no CMake needed

For a pre-compiled download of all external/third-party dependencies:

Download `VPinballX-10.8.1-(revision)-(SHA1)-dev-third-party-windows-(x86/x64)-(Debug/Release).zip` from [Actions](https://github.com/vpinball/vpinball/actions) of a `vpinball` run,
and extract the content to the `third-party` folder in the top directory, overwriting all existing files.

Run the `create_vs_solution.bat` from this folder here and select the Visual Studio version that will be used. This will create `.sln`/`.proj` files in the top directory.

If DirectX9 rendering / compilation is wanted, install the Microsoft DirectX SDK from 2021 (currently at [DX SDK](https://www.microsoft.com/en-us/download/details.aspx?id=6812)).


Otherwise, using the full build system via [CMake](https://cmake.org/):

## Build tools and libraries

Depending on the platform you are on you will need to install additional build tools and libraries.

### Windows

* install Bash (e.g. [git-bash](https://gitforwindows.org/))
* install [CMake](https://cmake.org/)
* install [build tools v143 for Visual Studio](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022) or the full Visual Studio 2022 package ([Express/Community](https://visualstudio.microsoft.com/de/vs/express/) is enough)
* install the Microsoft DirectX SDK from 2021 (currently at [DX SDK](https://www.microsoft.com/en-us/download/details.aspx?id=6812)) for DirectX9 rendering / compilation support.

### Other platforms

Detailed setup instructions for Visual Pinball Standalone (macOS / Linux / Android / iOS / ...) are available in the [standalone README](../standalone/README.md#compiling).

## External dependencies

Some external dependencies are required to build Visual Pinball. To fetch and build these, we have provided a script for each platform.

Check the `platforms` directory for the available platforms.

```bash
./platforms/YOUR_PLATFORM/external.sh
```

## CMake

Each platform/target combination has a `CMakeLists_[target]_[platform].txt` file in the `make` directory. You copy this file to `CMakeLists.txt` at the root of the project and use CMake to generate project files for your platform.

* `platform` should be the same one as you used to build the external dependencies.
* `target` can be one of the following:
  * `bgfx` (recommended) - uses [bgfx](https://github.com/bkaradzic/bgfx) to support multiple rendering backends
  * `gl` - OpenGL
  * `dx9` - DirectX 9

An example for Linux X64 with bgfx:

```bash
cp make/CMakeLists_bgfx-linux-x64.txt CMakeLists.txt
cmake -DCMAKE_BUILD_TYPE=Release -B build
# the -j flag is optional and specifies the number of parallel jobs to run 
# which in this case is the number of CPU cores (nproc command on Linux).
cmake --build build -- -j$(nproc)
# make sure that everything runs as expected
./build/VPinballX_BGFX -play ./build/assets/exampleTable.vpx
```

## Continuous Integration

Inspecting the [CI workflows](../.github/workflows) is also a good way to understand how everything is built.

*Note: PR's do not trigger continuous integration. We suggest you enable github actions on your fork.*
