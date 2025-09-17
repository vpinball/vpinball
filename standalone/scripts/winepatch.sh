#!/bin/bash

WINE_SRC="/Users/jmillard/vpx/wine"
WINE_VPX="/Users/jmillard/vpx/vpinball/standalone/inc/wine"

find $WINE_VPX -type f | while read i; do
  if [[ "$i" == *".DS_Store" ]]; then
     continue
  fi

  RELATIVE_PATH=${i#$WINE_VPX/}

  if [ "$RELATIVE_PATH" != "dlls/vbscript/parser.tab.c" ] &&
     [ "$RELATIVE_PATH" != "dlls/vbscript/parser.tab.h" ]; then
        FILE="$WINE_SRC/$RELATIVE_PATH"
        if [ -f "$FILE" ]; then
            touch "$i"
            PATCH=$(diff -u --label "$FILE" --label "$i" -w "$FILE" "$i")
            if [ ! -z "$PATCH" ]; then
               echo "$PATCH"
            fi
        fi
  fi
done
