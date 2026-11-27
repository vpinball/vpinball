# Third party libraries

## Third party libraries processed by external.sh build step

The version used for these libraries is defined by the SHA of the head commit in `platforms/config.sh`, used by the corresponding platform build script (`platforms/*/external.sh`). Their headers in `include/` are copied there by that build step.

### BX/BGFX/BIMG

- Upstream: https://github.com/bkaradzic/bgfx https://github.com/bkaradzic/bimg https://github.com/bkaradzic/bx
- Version: bgfx.cmake 1.172.9657-621 (https://github.com/bkaradzic/bgfx.cmake) + VPX patches from https://github.com/vpinball/bgfx (head at 0900e8e67fe0ab63ffdb21925c4ed794d9391fc2)
- License: BSD-2-Clause license

### FFmpeg

- Upstream: https://github.com/FFmpeg/FFmpeg
- Version: 9.1 (tag n9.1, 74e4393595cf2b5857f0da925c4e0dd74ea306e5)
- License: GPL-2.0, GPL-3.0, LGPL-3.0

### FreeImage

- Upstream: https://freeimage.sourceforge.io, built from the fork https://github.com/toxieainc/freeimage
- Version: head at d29c96999deb331181c44264e5338ec16d393c79
- License: FreeImage Public License or GPL

### libaltsound

- Upstream: https://github.com/vpinball/libaltsound
- Version: head at d1af7d8f83dbd22885f9ca548413b65ffb5dad3d
- License: 3-Clause BSD

### libdmdutil

- Upstream: https://github.com/vpinball/libdmdutil
- Version: head at 6b8f711a5346cf2e70562872ebc1a57b210906df
- License: GPLv3+

### libdof

- Upstream: https://github.com/vpinball/libdof
- Version: head at 3d0ad20a4be185aaa3d426e1abff01f9639a2f62
- License: GPLv3+

### libserum

- Upstream: https://github.com/PPUC/libserum
- Version: head at 295cba2a752eda790cb53c28a5fd6419897bf395
- License: GPLv2

### libvni

- Upstream: https://github.com/PPUC/libvni
- Version: head at eb910403834184e1d098bcc310054ac17a00b53d
- License: GPLv2

### libzedmd

- Upstream: https://github.com/PPUC/libzedmd
- Version: as pulled in by libdmdutil
- License: GPLv3

### libmysofa

- Upstream: https://github.com/hoene/libmysofa
- Version: head at 6cc5b15a73e9bd97810d03767082edda7f315881
- License: 3-Clause BSD

### libspatialaudio

- Upstream: https://github.com/videolan/libspatialaudio
- Version: head at d149ed9744fd399b835c6f2920511f8cbcfce5ea
- License: LGPL-2.1-or-later

### Ogg / Opus / Opusfile

- Upstream: https://github.com/xiph/ogg, https://github.com/xiph/opus, https://github.com/xiph/opusfile
- Version: ogg head at be05b13e98b048f0b5a0f5fa8ce514d56db5f822, opus 1.6.1 (22244de5a79bd1d6d623c32e72bf1954b56235be), opusfile head at 6dfd29e7adb87f2e193575fc3fa88cbf1a0b27df
- License: 3-Clause BSD

### OpenXR

- Upstream: https://github.com/KhronosGroup/OpenXR-SDK-Source
- Version: 1.1.65 (head at a9f14c416e383d05c142f5a2539a91fe3931b52a)
- License: Apache-2.0

### PinMAME

- Upstream: https://github.com/vpinball/pinmame
- Version: head at 309a17d882d098fe0a7cad88cb3b6ee6039f24e7
- License: 3-Clause BSD and old-MAME

### SDL / SDL Image / SDL TTF

- Upstream: https://github.com/libsdl-org/SDL, https://github.com/libsdl-org/SDL_image, https://github.com/libsdl-org/SDL_ttf
- Version: SDL 3.4.20, SDL_image 3.4.8, SDL_ttf 3.2.2
- License: Zlib

### zlib

- Upstream: https://github.com/madler/zlib
- Version: 1.4 (99623ef7097352da7993ba614161fd6a311e046e)
- License: Zlib



## Other third party libraries

### BAM

- Upstream: closed source except part shared inside VPX => not a third party ?
- Version: ?
- License: ?

### basis_universal BC6H/BC7 encoders (bc6h, bc7f)

- Upstream: https://github.com/BinomialLLC/basis_universal (self-contained extracts of its real-time bc6hf and bc7f encoders)
- Version: commit 99f52d6 (bc7f), ? (bc6h)
- License: Apache-2.0

### doctest

- Upstream: https://github.com/doctest/doctest
- Version: 2.5.3
- License: MIT

### Fonts

Droid Sans
- Upstream: https://www.fontsquirrel.com/fonts/droid-sans
- License: Apache 2 licensed

JetBrains Mono
- Upstream: https://github.com/JetBrains/JetBrainsMono
- License: SIL Open Font License 1.1

ForkAwesome
- Upstream: https://github.com/ForkAwesome/Fork-Awesome
- License: SIL OFL 1.1
Using generator from https://github.com/juliettef/IconFontCppHeaders

Liberation Sans
- Upstream: https://www.fontsquirrel.com/fonts/liberation-sans
- License: SIL Open Font License (OFL)

### Forsyth

- Upstream: ?
- Version: ?
- License: ?

### glad

- Upstream: https://glad.dav1d.de (https://github.com/Dav1dde/glad)
- Version: loaders generated with glad 2.0.8
- License: CC0-1.0 and Apache-2.0

### GPUProfiler

- Upstream: Derived from https://www.reedbeta.com/blog/gpu-profiling-101 blog post
- Version: ?
- License: Original code is BSD, adapted by VPX team

### Haze

- Upstream: https://github.com/chrisbanes/haze
- Version: 2.0.1
- License: Apache-2.0 license

### hid-report-parser

- Upstream: https://github.com/pasztorpisti/hid-report-parser + VPX modifications
- Version: ?
- License: MIT

### imgui

- Upstream: https://github.com/ocornut/imgui
- Version: 1.93.0 + VPX changes (see imgui/imgui.patch)
- License: MIT

### ImGuiColorTextEdit (textedit)

- Upstream: https://github.com/goossens/ImGuiColorTextEdit
- Version: head at 9f812a0b772e6c60c86c295a78a9fcdb2df6d59a + VPX changes (see textedit/UPSTREAM.md and textedit/textedit.patch)
- License: MIT

### imgui_markdown

- Upstream: https://github.com/juliettef/imgui_markdown
- Version: head at 68fb679b (2026-11) + VPX change: added `#include "imgui/imgui.h"`
- License: ZLib

### ImGuizmo

- Upstream: https://github.com/CedricGuillemet/ImGuizmo
- Version: master at 18cef5e0 (2026-08-08, after 1.10) + VPX changes (gizmo interaction fix)
- License: MIT

### implot

- Upstream: https://github.com/epezent/implot
- Version: 1.1 WIP (pre-release snapshot of 1.1) + VPX changes (see implot/implot.patch)
- License: MIT

### json

- Upstream: https://github.com/nlohmann/json
- Version: 3.12.0
- License: MIT

### mINI

- Upstream: https://github.com/metayeti/mINI
- Version: 0.9.20 + VPX changes: `ankerl::unordered_dense::map` as index map (std::unordered_map for MSVC <= 1935), added `INIMap::empty()`
- License: MIT

### miniaudio

- Upstream: https://github.com/mackron/miniaudio
- Version: 0.11.26 (+ VPX change: `ma_atomic_global_lock` made static)
- extras/stb_vorbis.c: https://github.com/nothings/stb 1.25, unmodified; extras/stb_vorbis_static.c is the same file with its public `stb_vorbis_*` API declared `static` (for the WMP plugin)
- License: Public Domain or MIT-0 (dual)

### miniD3D9

- Upstream: ?
- Version: ?
- License: ?

### miniz

- Upstream: https://github.com/richgel999/miniz
- Version: master at 2c50092 (after 3.1.2) + https://github.com/richgel999/miniz/pull/401 (not merged upstream yet), with its 4 options (MINIZ_ENABLE_DEFLATE64, MINIZ_ZIP_LENIENT_DATA_DESCRIPTORS, MINIZ_ZIP_BACKSLASH_SEPARATORS, MINIZ_ZIP_LONG_FILENAMES) enabled by default; VPX changes marked with `VPX`
- License: MIT

### monaco-editor

- Upstream: https://github.com/microsoft/monaco-editor
- Version: 0.55.1
- License: MIT

### Mongoose

- Upstream: https://github.com/cesanta/mongoose
- Version: 7.24
- License: GPL-2.0 or commercial (dual)

### NVAPI

- Upstream: https://github.com/NVIDIA/nvapi
- Version: ?
- License: Custom

### OpenVR

- Upstream: ?
- Version: ?
- License: ?

### PLog

- Upstream: https://github.com/SergiusTheBest/plog
- Version: master at 58919888 (after 1.1.11) + VPX change in Appenders/RollingFileAppender.h
- License: MIT

### Pole

- Upstream: ? + customizations
- Version: ?
- License: ?

### Progmesh

- Upstream: https://github.com/andandandand/progressive-mesh-reduction-with-edge-collapse
- Version: ?
- License: custom permissive

### qoi

- Upstream: https://github.com/phoboslab/qoi
- Version: master at 4ab68bbd (2025-04-28) + VPX changes
- License: MIT

### RSJparser

- Upstream: https://github.com/subh83/RSJp-cpp
- Version: master at c198b726 (2021-08-03) + VPX changes
- License: GPL-3

### ScaleFX

- Upstream: Custom implementation derived from shaders => not a third party ?
- Version: ?
- License: ?

### Scintilla

- Upstream: https://www.scintilla.org
- Version: ?
- License: Custom permissive license

### Serial

- Upstream: unclear if this is a lib or core code
- Version: ?
- License: ?

### simple-uri-parser

- Upstream: https://github.com/jholloc/simple-uri-parser
- Version: 1.0.0 (+ added standard includes)
- License: MIT

### sse2neon

- Upstream: https://github.com/DLTcollab/sse2neon
- Version: master at 8e1850c0 (2025-12-21, shortly before 1.9.1) + VPX changes: FORCE_INLINE defined as __forceinline, and the "only supports C++ compilation with this compiler" #error commented out
- License: MIT

### stb_image

- Upstream: https://github.com/nothings/stb (back from the nvpro-samples/stb fork, as upstream is maintained again and includes most of its fixes)
- Version: 2.34 + VPX changes, all marked with `// VPX`: indexed-color loading API from SDL (`stbi_load_*_with_palette`), plus the nvpro-samples/stb fixes not in 2.34 (memory-context callbacks zeroed, NULL checks after failed 8/16-bit conversion plus freeing its input on the 'too large' path, ftell/fseek error checks, tc16 zero-init), plus a fix for progressive JPEG decoding (upstream regression since 2.31, commit 3f9200a "Early-out in progressive JPEG decode", dropped the last blocks of a scan)
- License: MIT or Public Domain

### ThreadPool

- Upstream: https://github.com/log4cplus/ThreadPool
- Version: master at 91a18ace (2026-10-07) + VPX changes
- License: Custom permissive license

### tinygltf

- Upstream: https://github.com/syoyo/tinygltf
- Version: release branch at f9f49d35 (2025-10-30, between 2.9.6 and 2.9.7), unmodified
- License: MIT

### tinyxml2

- Upstream: https://github.com/leethomason/tinyxml2
- Version: 11.0.0 (+ GCC 15 fix)
- License: ZLib

### unordered-dense

- Upstream: https://github.com/martinus/unordered_dense
- Version: 5.3.3
- License: MIT

### Vulkan headers

- Upstream: https://github.com/KhronosGroup/Vulkan-Headers
- Version: 1.4.369 (include/vulkan and include/vk_video)
- License: Apache-2.0 OR MIT

### win32xx

- Upstream: https://sourceforge.net/projects/win32-framework
- Version: 10.3.0
- License: custom permissive (see copyright.txt)

### WinSDK

- Upstream: ?
- Version: ?
- License: unclear state

### xxHash

- Upstream: https://github.com/Cyan4973/xxHash
- Version: 0.8.4 (xxhash.h only, inlined with XXH_INLINE_ALL)
- License: BSD-2-Clause
