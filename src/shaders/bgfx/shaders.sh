#!/bin/bash
# Regenerate the embedded BGFX shader headers (../bgfx_*.h) manually.
#
# The actual shader matrix and compile loop live in shaders.cmake, which is also
# what the CMake build (make/CMakeLists_shaders.txt) and the MSVC custom build
# step (make/vpx-core.vcxitems) run — edit that file, not this wrapper.
#
# Usage: ./shaders.sh [shaderc_path] [out_dir]
#   shaderc_path defaults to the host tool built by the platform external.sh
#   out_dir      defaults to ../bgfx headers dir for direct source-tree regen
#
# BACKENDS can be overridden, e.g. BACKENDS=mtl,essl,glsl,spv ./shaders.sh
# GROUP can limit generation to a single group, e.g. GROUP=basic ./shaders.sh

set -e
cd "$(dirname "$0")"

SHADERC="${1:-}"
OUT_DIR="${2:-$(cd .. && pwd)}"
WORK_DIR="${OUT_DIR}/.shader-frag"
BACKENDS="${BACKENDS:-mtl,essl,glsl,dxbc,dxil,spv}"

if [ -z "${SHADERC}" ]; then
   for c in \
      ../../../third-party/tools/windows-x64/shaderc.exe \
      ../../../third-party/tools/windows-x86/shaderc.exe \
      ../../../third-party/tools/macos-arm64/shaderc \
      ../../../third-party/tools/macos-x64/shaderc \
      ../../../third-party/tools/linux-x64/shaderc \
      ../../../third-party/tools/linux-aarch64/shaderc \
      ./shaderc ./shaderc.exe; do
      [ -x "$c" ] && SHADERC="$c" && break
   done
fi
[ -n "${SHADERC}" ] || { echo "shaderc not found: pass its path or run platforms/<platform>/external.sh" >&2; exit 1; }

# shaderc loads the DXC compiler next to its own executable for s_6_0 (DXIL)
export PATH="$(cd "$(dirname "${SHADERC}")" && pwd):${PATH}"

GROUP_ARG=""
[ -n "${GROUP:-}" ] && GROUP_ARG="-DGROUP=${GROUP}"

cmake \
   -DSHADERC="$(cd "$(dirname "${SHADERC}")" && pwd)/$(basename "${SHADERC}")" \
   -DOUT_DIR="${OUT_DIR}" \
   -DWORK_DIR="${WORK_DIR}" \
   -DBACKENDS="${BACKENDS}" \
   ${GROUP_ARG} \
   -P shaders.cmake
