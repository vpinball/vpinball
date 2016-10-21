#pragma once

#define VP_VERSION_MAJOR    9
#define VP_VERSION_MINOR    9
#define VP_VERSION_REV      3

#define _STR(x)    #x
#define STR(x)     _STR(x)

#define VP_VERSION_STRING STR(VP_VERSION_MAJOR) "." STR(VP_VERSION_MINOR) "." STR(VP_VERSION_REV)
#define VP_VERSION_STRING_COMMAS STR(VP_VERSION_MAJOR) ", " STR(VP_VERSION_MINOR) ", " STR(VP_VERSION_REV) ", 0"

#define PASTE2(a,b) a##b
#define PASTE(a,b) PASTE2(a,b)
