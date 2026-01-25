# How to build

## Quickstart Windows / Visual Studio 2019 (and newer) variant: dependencies are downloaded, no CMake needed

For a pre-compiled download of all external/third-party dependencies:

Download `VPinballX-10.8.1-(revision)-(SHA1)-dev-third-party-windows-(x86/x64)-(Debug/Release).zip` from [Actions](https://github.com/vpinball/vpinball/actions) of a `vpinball` run,
and extract the content to the `third-party` folder in the top directory, overwriting all existing files.

Run the `create_vs_solution.bat` from this folder here and select the Visual Studio version that will be used. This will create `.sln`/`.proj` files in the `.build/vsproject` directory.

If DirectX9 rendering / compilation is wanted, you may need to install the Microsoft DirectX SDK from 2021 (currently at [DX SDK](https://www.microsoft.com/en-us/download/details.aspx?id=6812)),
but it should also work without it.


Otherwise, using the full build system via [CMake](https://cmake.org/):

## Build tools and libraries

Depending on the platform you are on you will need to install additional build tools and libraries.

### Windows

* install Bash (e.g. [git-bash](https://gitforwindows.org/))
* install [CMake](https://cmake.org/)
* install [build tools v145 for Visual Studio](https://aka.ms/vs/stable/vs_BuildTools.exe) or the full Visual Studio 2026 package ([Community](https://visualstudio.microsoft.com/de/vs/community/) version is enough, note that VS 2019 and 2022 are also still supported)
* install [MSYS2](https://www.msys2.org/)
* optionally install the Microsoft DirectX SDK from 2021 (currently at [DX SDK](https://www.microsoft.com/en-us/download/details.aspx?id=6812)) for DirectX9 rendering / compilation support, should not be necessary.

## External dependencies

Some external dependencies are required to build Visual Pinball. To fetch and build these, we have provided a script for each platform. This script must be run from the root of project.

```bash
platforms/[platform]/external.sh
```

Note that you may need to point bash to the tools folder via `PATH`, e.g. `/c/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/MSBuild/Current/Bin`.

## Building via CMake

Each target/platform combination has a `CMakeLists_[target]_[platform].txt` file in the `make` directory. Copy this file to `CMakeLists.txt` at the root of the project.

* `target` can be one of the following:
  * `bgfx` (recommended) - uses [bgfx](https://github.com/bkaradzic/bgfx) to support multiple rendering backends
  * `gl` - OpenGL
  * `dx9` - DirectX 9
* `platform` should be the same one as you used to build the external dependencies.

```bash
cp make/CMakeLists_[target]_[platform].txt CMakeLists.txt
```

#### Supported Platforms

<details open>
<summary>windows-x64</summary>

```
pacman -S --noconfirm make diffutils yasm mingw-w64-x86_64-gcc mingw-w64-x86_64-zlib mingw-w64-x86_64-libwinpthread mingw-w64-x86_64-libiconv mingw-w64-x86_64-cmake
platforms/windows-x64/external.sh
cp make/CMakeLists_bgfx-windows-x64.txt CMakeLists.txt
cmake -G "Visual Studio 17 2022" -A x64 -B build
cmake --build build --config Release
```
</details>

<details open>
<summary>windows-x86</summary>

```
pacman -S --noconfirm make diffutils yasm mingw-w64-i686-gcc mingw-w64-i686-zlib mingw-w64-i686-libwinpthread mingw-w64-i686-libiconv mingw-w64-i686-cmake
platforms/windows-x86/external.sh
cp make/CMakeLists_bgfx-windows-x86.txt CMakeLists.txt
cmake -G "Visual Studio 17 2022" -A Win32 -B build
cmake --build build --config Release
```
</details>

<details>
<summary>macos-arm64</summary>

```
sudo xcode-select --switch /Applications/Xcode.app/Contents/Developer
brew install autoconf automake libtool cmake bison curl
export PATH="$(brew --prefix bison)/bin:$PATH"
platforms/macos-arm64/external.sh
cp make/CMakeLists_bgfx-macos-arm64.txt CMakeLists.txt
cmake -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build -- -j$(sysctl -n hw.ncpu)

build/VPinballX_BGFX.app/Contents/MacOS/VPinballX_BGFX -play src/assets/exampleTable.vpx
```
</details>

<details>
<summary>macos-x64</summary>

```
sudo xcode-select --switch /Applications/Xcode.app/Contents/Developer
brew install autoconf automake libtool cmake nasm bison curl
export PATH="$(brew --prefix bison)/bin:$PATH"
platforms/macos-x64/external.sh
cp make/CMakeLists_bgfx-macos-x64.txt CMakeLists.txt
cmake -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build -- -j$(sysctl -n hw.ncpu)

build/VPinballX_BGFX.app/Contents/MacOS/VPinballX_BGFX -play src/assets/exampleTable.vpx
```
</details>

<details>
<summary>linux-x64 (Ubuntu)</summary>

```
sudo apt-get update
sudo apt install git build-essential pkg-config autoconf automake libtool cmake nasm bison curl zlib1g-dev libdrm-dev libgbm-dev libglu1-mesa-dev libegl-dev libgl1-mesa-dev libwayland-dev libwayland-egl-backend-dev libudev-dev libx11-dev libxcursor-dev libxi-dev libxss-dev libxtst-dev libxkbcommon-dev libxrandr-dev libasound2-dev libpipewire-0.3-dev
platforms/linux-x64/external.sh
cp make/CMakeLists_bgfx-linux-x64.txt CMakeLists.txt
cmake -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build -- -j$(nproc)

build/VPinballX_BGFX -play src/assets/exampleTable.vpx
```
</details>

<details>
<summary>ios-arm64 (and ios-simulator-arm64)</summary>

```
sudo xcode-select --switch /Applications/Xcode.app/Contents/Developer
brew install cmake bison curl
export PATH="$(brew --prefix bison)/bin:$PATH"
platforms/ios-arm64/external.sh
#platforms/ios-simulator-arm64/external.sh
cp make/CMakeLists_bgfx_lib.txt CMakeLists.txt
cmake -DPLATFORM=ios -DARCH=arm64 -DCMAKE_BUILD_TYPE=Release -B build/ios-arm64
cmake --build build/ios-arm64 -- -j$(sysctl -n hw.ncpu)
#cmake -DPLATFORM=ios-simulator -DARCH=arm64 -DCMAKE_BUILD_TYPE=Release -B build/ios-simulator-arm64
#cmake --build build/ios-simulator-arm64 -- -j$(sysctl -n hw.ncpu)

open standalone/ios/VPinball.xcodeproj
```
</details>

<details>
<summary>android-arm64-v8a (Mobile)</summary>

```
brew install cmake bison curl
export PATH="$(brew --prefix bison)/bin:$PATH"
export JAVA_HOME=$(/usr/libexec/java_home -v 21)
export ANDROID_HOME=/Users/jmillard/Library/Android/sdk
export ANDROID_NDK=/Users/jmillard/Library/Android/sdk/ndk/28.2.13676358
export ANDROID_NDK_HOME=/Users/jmillard/Library/Android/sdk/ndk/28.2.13676358
platforms/android-arm64-v8a/external.sh
cp make/CMakeLists_bgfx_lib.txt CMakeLists.txt
cmake -DPLATFORM=android -DARCH=arm64-v8a -DCMAKE_BUILD_TYPE=Release -B build/android-arm64-v8a
cmake --build build/android-arm64-v8a -- -j$(sysctl -n hw.ncpu)
cd standalone/android
./gradlew assembleMobileDebug
```
</details>

<details>
<summary>android-arm64-v8a (Quest)</summary>

```
brew install cmake bison curl
export PATH="$(brew --prefix bison)/bin:$PATH"
export JAVA_HOME=$(/usr/libexec/java_home -v 21)
export ANDROID_HOME=/Users/jmillard/Library/Android/sdk
export ANDROID_NDK=/Users/jmillard/Library/Android/sdk/ndk/28.2.13676358
export ANDROID_NDK_HOME=/Users/jmillard/Library/Android/sdk/ndk/28.2.13676358
platforms/android-arm64-v8a/external.sh
cp make/CMakeLists_bgfx_lib.txt CMakeLists.txt
cmake -DPLATFORM=android -DARCH=arm64-v8a -DENABLE_XR=ON -DCMAKE_BUILD_TYPE=Release -B build/android-arm64-v8a
cmake --build build/android-arm64-v8a -- -j$(sysctl -n hw.ncpu)
cd standalone/android
./gradlew assembleQuestDebug
```
</details>

#### Unsupported Platforms

<details>
<summary>linux-x64 (Fedora)</summary>

```
sudo dnf install @development-tools
sudo dnf install gcc-c++ pkg-config autoconf automake libtool cmake nasm bison curl systemd-devel mesa-libGL-devel libX11-devel libXext-devel libXcursor-devel libXi-devel libXScrnSaver-devel libXtst-devel libxkbcommon-devel libxkbcommon-x11-devel libXrandr-devel zlib-ng-compat-static zlib-ng-compat-devel wayland-devel alsa-lib-devel pipewire-devel
platforms/linux-x64/external.sh
cp make/CMakeLists_bgfx-linux-x64.txt CMakeLists.txt
cmake -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build -- -j$(nproc)

build/VPinballX_BGFX -play src/assets/exampleTable.vpx
```
</details>


<details>
<summary>linux-aarch64 (Fedora Asahi Remix 41 - Apple silicon)</summary>

```
sudo dnf install @development-tools
sudo dnf install gcc-c++ pkg-config autoconf automake libtool cmake nasm bison curl systemd-devel mesa-libGL-devel libX11-devel libXext-devel ibXcursor-devel libXi-devel libXScrnSaver-devel libXtst-devel libxkbcommon-devel libxkbcommon-x11-devel zlib-ng-compat-static zlib-ng-compat-devel wayland-devel
platforms/linux-aarch64/external.sh
cp make/CMakeLists_bgfx-linux-aarch64.txt CMakeLists.txt
cmake -DBUILD_RK3588=ON -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build -- -j$(nproc)

build/VPinballX_BGFX -play src/assets/exampleTable.vpx
```
</details>

<details>
<summary>linux-aarch64 (RPI)</summary>

> Last version of Raspberry Pi OS (Trixie) is needed to be able to compile

> [!NOTE]
> Instructions for unsupported platforms are provided as-is and are not officially maintained by the team. Community contributions to improve and update them are welcome.

```
sudo apt-get update
sudo apt install git pkg-config autoconf automake libtool cmake bison zlib1g-dev libdrm-dev libgbm-dev libgles2-mesa-dev libudev-dev libx11-dev libxcursor-dev libxi-dev libxss-dev libxtst-dev libxkbcommon-dev libxrandr-dev libasound2-dev libpipewire-0.3-dev libwayland-dev autotools-dev libdrm-etnaviv1 libegl-dev libglvnd-core-dev libltdl-dev libspa-0.2-dev libxrender-dev cmake-data libdrm-freedreno1 libffi-dev libglvnd-dev libpciaccess-dev libwayland-bin m4 libcap-dev libdrm-tegra0 libgles-dev libjsoncpp26 librhash1 libxfixes-dev libgpiod-dev
platforms/linux-aarch64/external.sh
cp make/CMakeLists_gl-linux-aarch64.txt CMakeLists.txt
cmake -DBUILD_RPI=ON -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build -- -j$(nproc)

build/VPinballX_GL -play src/assets/exampleTable.vpx
```
</details>

<details>
<summary>linux-aarch64 (RK3588)</summary>

```
sudo apt-get update
sudo apt install git pkg-config autoconf automake libtool cmake bison zlib1g-dev libdrm-dev libgbm-dev libgles2-mesa-dev libgles2-mesa libudev-dev libx11-dev libxcursor-dev libxi-dev libxss-dev libxtst-dev libxkbcommon-dev libxrandr-dev libasound2-dev libpipewire-0.3-dev
platforms/linux-aarch64/external.sh
cp make/CMakeLists_bgfx-linux-aarch64.txt CMakeLists.txt
cmake -DBUILD_RK3588=ON -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build -- -j$(nproc)

build/VPinballX_BGFX -play src/assets/exampleTable.vpx
```
</details>

> [!NOTE]
> Instructions for unsupported platforms are provided as-is and are not officially maintained by the team. Community contributions to improve and update them are welcome.

## Continuous Integration

Inspecting the [CI workflows](../.github/workflows) is also a good way to understand how everything is built.

> [!NOTE]
> Due to the large storage requirements and build times for external dependencies, pull requests currently do not trigger continuous integration. We recommend enabling GitHub Actions on your fork.
