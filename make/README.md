# How to build

## CMake

Just use the provided `CMakeLists_X.txt` files.

Detailed build instructions for vpinball standalone (macOS / Linux / Anddroid / iOS / ...) are available in the [standalone README](../standalone/README.md#compiling).

## Windows / Visual Studio 2015 and newer

Run the `create_vs_solution.bat` in this folder. This will create `.sln`/`.proj` files in the top directory.
If needed (DX9 compatible VPX compile), also install the Microsoft DirectX SDK from 2021 (currently at [DX SDK](https://www.microsoft.com/en-us/download/details.aspx?id=6812)).

## Continuous Integration

Inspecting the [CI workflows](../.github/workflows) is also a good way to understand how everything is built.

*Note: PR's do not trigger continuous integration. We suggest you enable github actions on your fork.*
