#pragma once

#include "git_version.h"

// Update VERSION_START_SHA in .github\workflows\vpinball.yml to point to the latest commit.
#define VP_VERSION_MAJOR    10 // X Digits
#define VP_VERSION_MINOR    8  // Max 2 Digits
#define VP_VERSION_REV      1  // Max 1 Digit

#define CURRENT_FILE_FORMAT_VERSION  1081
#define NO_ENCRYPTION_FORMAT_VERSION 1050
#define NEW_SOUND_FORMAT_VERSION     1031 // introduced surround option

#define _STR(x)    #x
#define STR(x)     _STR(x)

#if defined(ENABLE_OPENGL)
#ifndef __OPENGLES__
#define VP_FLAVOUR "OpenGL"
#define VP_FLAVOUR_SHORT "GL"
#else
#define VP_FLAVOUR "GLES"
#define VP_FLAVOUR_SHORT "GLES"
#endif
#elif defined(ENABLE_BGFX)
#define VP_FLAVOUR "BGFX"
#define VP_FLAVOUR_SHORT "BGFX"
#else
#define VP_FLAVOUR "DirectX"
#define VP_FLAVOUR_SHORT "DX"
#endif

#define VP_FILEDESCRIPTION "Visual Pinball " VP_FLAVOUR " " STR(VP_VERSION_MAJOR) "." STR(VP_VERSION_MINOR) "." STR(VP_VERSION_REV)
#define VP_PRODUCTNAME "Visual Pinball " VP_FLAVOUR_SHORT
#define VP_ORIGINALFILENAME "VPinballX_" VP_FLAVOUR_SHORT ".exe"

#define VP_VERSION_DIGITS VP_VERSION_MAJOR,VP_VERSION_MINOR,VP_VERSION_REV,GIT_REVISION
#define VP_VERSION_STRING_DIGITS STR(VP_VERSION_MAJOR) STR(VP_VERSION_MINOR) STR(VP_VERSION_REV) STR(GIT_REVISION)
#define VP_VERSION_STRING_POINTS STR(VP_VERSION_MAJOR) "." STR(VP_VERSION_MINOR) "." STR(VP_VERSION_REV) "." STR(GIT_REVISION)
#define VP_VERSION_STRING_COMMAS STR(VP_VERSION_MAJOR) ", " STR(VP_VERSION_MINOR) ", " STR(VP_VERSION_REV) ", " STR(GIT_REVISION)
#define VP_VERSION_STRING_POINTS_FULL STR(VP_VERSION_MAJOR) "." STR(VP_VERSION_MINOR) "." STR(VP_VERSION_REV) "." STR(GIT_REVISION) "." GIT_SHA

// Complete version string for log, crash handler,...
#define VP_VERSION_STRING_FULL_LITERAL "v" STR(VP_VERSION_MAJOR) "." STR(VP_VERSION_MINOR) "." STR(VP_VERSION_REV) \
	" Beta (Rev. " STR(GIT_REVISION) " (" GIT_SHA "), " GET_PLATFORM_OS " " VP_FLAVOUR_SHORT " " GET_PLATFORM_BITS "bits)"

#define PASTE2(a,b) a##b
#define PASTE(a,b) PASTE2(a,b)
