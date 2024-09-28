// license:GPLv3+

#pragma once

///////////////////////////////////////////////////////////////////////////////
// Core Virtual Pinball plugins
//
// WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
// This interface is part of a work in progress and will evolve likely a lot
// before being considered stable. Do not use it, or if you do, use it knowing
// that you're plugin will be broken by the upcoming updates.
// WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
//
// This file defines a few core identifiers, messages and struct to ease plugin
// collaboration



///////////////////////////////////////////////////////////////////////////////
// Generic main controller

#define CTLPI_NAMESPACE       "Controller"

#define CTLPI_MSG_GET_DMD     "GetDMD"

#define CTLPI_GETDMD_RENDER_FRAME    1 // Frame suitable for rendering
#define CTLPI_GETDMD_IDENTIFY_FRAME  2 // Frame suitable for frame identification (stable value)

#define CTLPI_GETDMD_FORMAT_LUM8       1
#define CTLPI_GETDMD_FORMAT_SRGB888    2
#define CTLPI_GETDMD_FORMAT_BITPLANE2  3
#define CTLPI_GETDMD_FORMAT_BITPLANE4  4
#define CTLPI_GETDMD_FORMAT_SRGB565    5

typedef struct
{
   // Request
   int dmdId;                 // -1 is global default DMD, positive value uniquely identify a DMD based on its controller id scheme
   unsigned int requestFlags; // Request option, for the time being either render of raw frame
   // Response
   unsigned int frameId;      // Id that can be used to detect identical frames
   unsigned int format;       // Frame data format, see CTLPI_GETDMD_FORMAT_xxx constants
   unsigned int width;        // Frame width
   unsigned int height;       // Frame height
   uint8_t* frame;            // Pointer to frame data, owned by the provider
} GetDmdMsg;
