#!/bin/bash
#
# Regenerate the IDispatch proxy stub from src/core/vpinball.idl using the
# Java parser under standalone/idl/parser. Overwrites
# standalone/vpinball_standalone_i_proxy.cpp.
#
# Usage: ./standalone/scripts/genproxy.sh

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PARSER_DIR="$SCRIPT_DIR/../idl/parser"

cd "$PARSER_DIR"
gradle run --quiet
