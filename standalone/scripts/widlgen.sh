#!/bin/bash

WINE_PATH=/Users/jmillard/vpx/wine

${WINE_PATH}/tools/widl/widl -m64 -o ../vpinball_standalone_i.h --nostdinc -Ldlls/\* -I${WINE_PATH}/include -D__WINESRC__ -D_UCRT ../../src/core/vpinball.idl
${WINE_PATH}/tools/widl/widl -m64 -u -o ../vpinball_standalone_i.c --nostdinc -Ldlls/\* -I${WINE_PATH}/include -D__WINESRC__ -D_UCRT ../../src/core/vpinball.idl
