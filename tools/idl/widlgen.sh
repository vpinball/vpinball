#!/bin/bash
#
# Regenerate the widl-compiled IDispatch interface files from
# src/core/vpinball.idl using Wine's widl. Overwrites
# src/core/generated/vpinball_standalone_i.h and
# src/core/generated/vpinball_standalone_i.c.
#
# WINE_PATH defaults to a wine checkout sitting next to this repo
# (one level outside the repo root); override it to use another location.
#
# Usage: ./tools/idl/widlgen.sh

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
OUT_DIR="$REPO_DIR/src/core/generated"

WINE_PATH="${WINE_PATH:-$REPO_DIR/../wine}"

${WINE_PATH}/tools/widl/widl -m64 -o "$OUT_DIR/vpinball_standalone_i.h" --nostdinc -Ldlls/\* -I${WINE_PATH}/include -D__WINESRC__ -D_UCRT "$REPO_DIR/src/core/vpinball.idl"
${WINE_PATH}/tools/widl/widl -m64 -u -o "$OUT_DIR/vpinball_standalone_i.c" --nostdinc -Ldlls/\* -I${WINE_PATH}/include -D__WINESRC__ -D_UCRT "$REPO_DIR/src/core/vpinball.idl"
