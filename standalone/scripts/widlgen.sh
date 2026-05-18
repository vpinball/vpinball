#!/bin/bash
#
# Regenerate the widl-compiled IDispatch interface files from
# src/core/vpinball.idl using Wine's widl. Overwrites
# standalone/vpinball_standalone_i.h and standalone/vpinball_standalone_i.c.
#
# WINE_PATH defaults to a wine checkout sitting next to this repo
# (one level outside the repo root); override it to use another location.
#
# Usage: ./standalone/scripts/widlgen.sh

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

WINE_PATH="${WINE_PATH:-$SCRIPT_DIR/../../../wine}"

${WINE_PATH}/tools/widl/widl -m64 -o ../vpinball_standalone_i.h --nostdinc -Ldlls/\* -I${WINE_PATH}/include -D__WINESRC__ -D_UCRT ../../src/core/vpinball.idl
${WINE_PATH}/tools/widl/widl -m64 -u -o ../vpinball_standalone_i.c --nostdinc -Ldlls/\* -I${WINE_PATH}/include -D__WINESRC__ -D_UCRT ../../src/core/vpinball.idl
