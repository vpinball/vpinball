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
// collaboration around common controller's data.
//

#define CTLPI_NAMESPACE                       "Controller"


///////////////////////////////////////////////////////////////////////////////
//
// Dot Matrix Displays
//
// DMD collaboration messages are designed to support (at least) the following
// use cases:
// - allow searching for the available DMD sources, suitable for frame 
//   identification and frame rendering, allowing for sources to appear and 
//   disappear at runtime
// - declare and provide a DMD source (like PinMame, FlexDMD, alphanumeric to
//   DMD renderer, UltraDMD, VPinSpa,...)
// - declare and provide improved variants of a DMD frame source, for example
//   providing upscaling or colorization support.
//

// Broadcasted when a DMD source has been added, changed or removed, there is no message data
#define CTLPI_ONDMD_SRC_CHG_MSG               "OnDMDSrcChange"

// Request subscribers to fill up an array with the list of DMD sources, message data is a pointer to a GetDmdSrcMsg structure
#define CTLPI_GETDMD_SRC_MSG                  "GetDMDSrc"

// Request subscribers for a DMD frame suited for rendering (best visual), message data is a pointer to a GetDmdMsg structure
#define CTLPI_GETDMD_RENDER_MSG               "GetDMD"

// Request subscribers for a DMD frame suited for frame identification (stable encoding), message data is a pointer to a GetDmdMsg structure
#define CTLPI_GETDMD_IDENTIFY_MSG             "GetDMDIdentify"

// Render DMD frame formats
#define CTLPI_GETDMD_FORMAT_LUM8              1
#define CTLPI_GETDMD_FORMAT_SRGB888           2
#define CTLPI_GETDMD_FORMAT_SRGB565           5

// Identify DMD frame formats
#define CTLPI_GETDMD_FORMAT_BITPLANE2         3
#define CTLPI_GETDMD_FORMAT_BITPLANE4         4

// Hardware type
#define CTLPI_GETDMD_HARDWARE_TYPE_MASK       0xFFFF0000
#define CTLPI_GETDMD_HARDWARE_MODEL_MASK      0x0000FFFF
#define CTLPI_GETDMD_HARDWARE_UNKNOWN         0x00000000
#define CTLPI_GETDMD_HARDWARE_NEON_PLASMA     0x00010000
#define CTLPI_GETDMD_HARDWARE_RED_LED         0x00020000
#define CTLPI_GETDMD_HARDWARE_RGB_LED         0x00030000


// Structure uniquely identifying a DMD source. Note that a DMD frame stream can be reported multiple times with
// different size/format thanks to frame improvment like colorization or upscaler.
typedef struct DmdSrcId
{
   unsigned int id;        // Unique Id of the frame stream
   unsigned int width;     //
   unsigned int height;    //
   unsigned int format;    //
   unsigned int hardware;  // The hardware type
} DmdSrcId;

typedef struct GetDmdSrcMsg
{
   // Request
   unsigned int maxEntryCount; // see below
   // Response
   unsigned int count;         // Number of entries, also position to put next entry
   DmdSrcId* entries;          // Pointer to an array of maxEntryCount entries to be filled
} GetDmdSrcMsg;

typedef struct GetDmdMsg
{
   // Request that must match one of the DMD source definitions reported by GetDmdSrcMsg
   DmdSrcId dmdId;            // Uniquely identify the DMD source (frame stream, size and format)
   // Response
   unsigned int frameId;      // Id that can be used to discard identical frames
   unsigned char* frame;      // Pointer to frame data, null until a provider answers the request, owned by the provider
} GetDmdMsg;

typedef struct GetRawDmdMsg
{
   // Request that must match one of the DMD source definitions reported by GetDmdSrcMsg
   unsigned int dmdId;   // Uniquely identify the DMD source, we only match on the original frame source
   // Response
   unsigned int width;   //
   unsigned int height;  //
   unsigned int format;  //
   unsigned int frameId; // Id that can be used to discard identical frames
   unsigned char* frame; // Pointer to frame data, null until a provider answers the request, owned by the provider
} GetRawDmdMsg;



///////////////////////////////////////////////////////////////////////////////
//
// Segment displays
//

// Broadcasted when an alpha numeric source has been added, changed or removed, there is no message data
#define CTLPI_ONSEG_SRC_CHG_MSG        "OnSegSrcChange"

// Request subscribers to fill up an array with the list of alpha numeric sources, message data is a pointer to a GetSegSrcMsg structure
#define CTLPI_GETSEG_SRC_MSG           "GetSegSrc"

// Request subscribers for a alpha numeric state suited for rendering (best visual), message data is a pointer to a GetSegMsg structure
#define CTLPI_GETSEG_MSG               "GetSeg"

// Segment display layouts
#define CTLPI_GETSEG_LAYOUT_16         0 // 16 segments
#define CTLPI_GETSEG_LAYOUT_16R        1 // 16 segments with comma and period reversed
#define CTLPI_GETSEG_LAYOUT_10         2 // 9  segments and comma
#define CTLPI_GETSEG_LAYOUT_9          3 // 9  segments
#define CTLPI_GETSEG_LAYOUT_8          4 // 7  segments and comma
#define CTLPI_GETSEG_LAYOUT_8D         5 // 7  segments and period
#define CTLPI_GETSEG_LAYOUT_7          6 // 7  segments
#define CTLPI_GETSEG_LAYOUT_87         7 // 7  segments, comma every three
#define CTLPI_GETSEG_LAYOUT_87F        8 // 7  segments, forced comma every three
#define CTLPI_GETSEG_LAYOUT_98         9 // 9  segments, comma every three
#define CTLPI_GETSEG_LAYOUT_98F       10 // 9  segments, forced comma every three
#define CTLPI_GETSEG_LAYOUT_7S        11 // 7  segments, small
#define CTLPI_GETSEG_LAYOUT_7SC       12 // 7  segments, small, with comma
#define CTLPI_GETSEG_LAYOUT_16S       13 // 16 segments with split top and bottom line

// Hardware type
#define CTLPI_GETSEG_HARDWARE_TYPE_MASK       0xFFFF0000
#define CTLPI_GETSEG_HARDWARE_MODEL_MASK      0x0000FFFF
#define CTLPI_GETSEG_HARDWARE_UNKNOWN         0x00000000
#define CTLPI_GETSEG_HARDWARE_NEON_PLASMA     0x00010000
#define CTLPI_GETSEG_HARDWARE_VFD_GREEN       0x00020000
#define CTLPI_GETSEG_HARDWARE_VFD_BLUE        0x00030000


// Structure uniquely identifying a segment display
typedef struct SegSrcId
{
   unsigned int id;        // Unique Id of the frame stream
   unsigned int dispCount; // Number of elements composing the display
   unsigned int segLayout; // Segment layout
   unsigned int hardware;  // Hardware type
} SegSrcId;

typedef struct GetSegSrcMsg
{
   // Request
   unsigned int maxEntryCount; // see below
   // Response
   unsigned int count;         // Number of entries, also position to put next entry
   SegSrcId* entries;          // Pointer to an array of maxEntryCount entries to be filled
} GetSegSrcMsg;

typedef struct GetSegMsg
{
   // Request that must match one of the Segment source definitions reported by GetSegSrcMsg
   SegSrcId segId;            // Uniquely identify the segment source
   // Response
   unsigned int frameId;      // Id that can be used to discard identical frames
   float* frame;              // Pointer to frame data (one relative luminance value per segment), null until a provider answers the request, owned by the provider
} GetSegMsg;

