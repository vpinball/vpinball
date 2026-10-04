#!/bin/bash
#
# Regenerate the IDispatch proxy stub from src/core/vpinball.idl using the
# Java parser under tools/idl/parser. Overwrites
# src/core/generated/vpinball_standalone_i_proxy.cpp.
#
# Usage: ./tools/idl/genproxy.sh

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

cd "$SCRIPT_DIR/parser"
gradle run --quiet
