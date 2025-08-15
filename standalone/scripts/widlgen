#!/bin/bash

WINE_PATH=/Users/jmillard/wine

${WINE_PATH}/tools/widl/widl -m64 -o ../vpinball_standalone_i.h --nostdinc -Ldlls/\* -I${WINE_PATH}/include -D__WINESRC__ -D_UCRT ../../src/core/vpinball.idl
${WINE_PATH}/tools/widl/widl -m64 -u -o ../vpinball_standalone_i.c --nostdinc -Ldlls/\* -I${WINE_PATH}/include -D__WINESRC__ -D_UCRT ../../src/core/vpinball.idl

${WINE_PATH}/tools/widl/widl -m64 -o ../inc/b2s/b2s_i.h --nostdinc -Ldlls/\* -I${WINE_PATH}/include -D__WINESRC__ -D_UCRT ../inc/b2s/B2S.idl
${WINE_PATH}/tools/widl/widl -m64 -u -o ../inc/b2s/b2s_i.c --nostdinc -Ldlls/\* -I${WINE_PATH}/include -D__WINESRC__ -D_UCRT ../inc/b2s/B2S.idl
