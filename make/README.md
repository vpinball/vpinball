# How to build

## Windows / Visual Studio 2015 and newer: Quickstart variant, dependencies are downloaded, no CMake needed

For a pre-compiled download of all external/third-party dependencies:

Download `VPinballX-10.8.1-(revision)-(SHA1)-dev-third-party-windows-(x86/x64)-(Debug/Release).zip` from https://github.com/vpinball/vpinball/actions of a `vpinball` run,
and extract the content to the `third-party` folder in the top directory, overwriting all existing files.

Run the `create_vs_solution.bat` from this folder here and select the Visual Studio version that will be used. This will create `.sln`/`.proj` files in the top directory.

If DirectX9 rendering / compilation is wanted, install the Microsoft DirectX SDK from 2021 (currently at [DX SDK](https://www.microsoft.com/en-us/download/details.aspx?id=6812)).

## Windows / Visual Studio 2015 and newer: Local dependencies build

For a full download and local build of all external/third-party dependencies:

- Install a bash (tested to work with at least git-bash)
- Install CMake
- Run `external.sh` in `platforms/windows-(x86/x64)`
- Continue with next section

## CMake

Just use the provided `CMakeLists_X.txt` files.

Detailed build instructions for vpinball standalone (macOS / Linux / Android / iOS / ...) are available in the [standalone README](../standalone/README.md#compiling).

## Continuous Integration

Inspecting the [CI workflows](../.github/workflows) is also a good way to understand how everything is built.

*Note: PR's do not trigger continuous integration. We suggest you enable github actions on your fork.*
