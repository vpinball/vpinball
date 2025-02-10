// license:GPLv3+

#pragma once

#ifdef __cplusplus
 #include <cstdint>
#else
 #include <stdint.h>
#endif

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

// Generic structure used to identify a resource belonging to an endpoint
typedef union CtlResId
{
   struct {
      uint32_t endpointId;
      uint32_t resId;
   };
   uint64_t id;
} CtlResId;


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
#define CTLPI_GETDMD_FORMAT_LUM8              0
#define CTLPI_GETDMD_FORMAT_SRGB888           1
#define CTLPI_GETDMD_FORMAT_SRGB565           4

// Identify DMD frame formats
#define CTLPI_GETDMD_FORMAT_BITPLANE2         2
#define CTLPI_GETDMD_FORMAT_BITPLANE4         3

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
   CtlResId id;            // Unique Id of the frame stream
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
   CtlResId dmdId;       // Uniquely identify the DMD source, we only match on the original frame source
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

// Individual segment display layouts
typedef enum {
   CTLPI_GETSEG_LAYOUT_7,          //  7 segments
   CTLPI_GETSEG_LAYOUT_7C,         //  7 segments and comma
   CTLPI_GETSEG_LAYOUT_7D,         //  7 segments and dot
   CTLPI_GETSEG_LAYOUT_9,          //  9 segments
   CTLPI_GETSEG_LAYOUT_9C,         //  9 segments and comma
   CTLPI_GETSEG_LAYOUT_14,         // 14 segments
   CTLPI_GETSEG_LAYOUT_14D,        // 14 segments with dot
   CTLPI_GETSEG_LAYOUT_14DC,       // 14 segments with dot and comma
   CTLPI_GETSEG_LAYOUT_16,         // 16 segments (split top/bottom segments)
} SegElementType;

// Hardware type
#define CTLPI_GETSEG_HARDWARE_TYPE_MASK       0xFFFF0000
#define CTLPI_GETSEG_HARDWARE_MODEL_MASK      0x0000FFFF
#define CTLPI_GETSEG_HARDWARE_UNKNOWN         0x00000000
#define CTLPI_GETSEG_HARDWARE_NEON_PLASMA     0x00010000
#define CTLPI_GETSEG_HARDWARE_VFD_GREEN       0x00020000
#define CTLPI_GETSEG_HARDWARE_VFD_BLUE        0x00030000
#define CTLPI_GETSEG_HARDWARE_GTS1_4DIGIT     0x00030001 // Futaba VFD used for Gottlieb System 1 and 80A status display
#define CTLPI_GETSEG_HARDWARE_GTS1_6DIGIT     0x00030002 // Futaba VFD used for Gottlieb System 1 and 80 score display
#define CTLPI_GETSEG_HARDWARE_GTS80A_7DIGIT   0x00030003 // Futaba VFD used for Gottlieb System 80A score display
#define CTLPI_GETSEG_HARDWARE_GTS80B_20DIGIT  0x00030004 // Futaba VFD used for Gottlieb System 80B

#define CTLPI_SEG_MAX_DISP_ELEMENTS  32

typedef struct SegSrcId
{
   CtlResId id;                                             // Unique Id of the display group
   unsigned int nDisplaysInGroup;                           // Number of displays inside this group
   unsigned int displayIndex;                               // Index of the display inside the display group (0..nDisplaysInGroup-1)
   unsigned int hardware;                                   // Hardware hint, see CTLPI_GETSEG_HARDWARE_xxx
   unsigned int nElements;                                  // Number of individual elements forming this display
   SegElementType elementType[CTLPI_SEG_MAX_DISP_ELEMENTS]; // Type of each individual element forming this display
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
   CtlResId segId;            // Uniquely identify the segment source (a group of displays, each of them composed of multiple elements)
   // Response
   unsigned int frameId;      // Id that can be used to discard identical frames
   float* frame;              // Pointer to frame data (one relative luminance value per segment, 16 segments per element), null until a provider answers the request, owned by the provider
} GetSegMsg;



///////////////////////////////////////////////////////////////////////////////
//
// Audio streams (backglass, pinsound/altsound/gsound, ...)
//

// Broadcasted when an audio stream is made available, updated or ended
#define CTLPI_ONAUDIO_UPDATE_MSG "AudioUpdate"

#define CTLPI_AUDIO_SRC_BACKGLASS_MONO       0
#define CTLPI_AUDIO_SRC_BACKGLASS_STEREO     1

#define CTLPI_AUDIO_FORMAT_SAMPLE_INT16      0
#define CTLPI_AUDIO_FORMAT_SAMPLE_FLOAT      1

typedef struct AudioUpdateMsg
{
   CtlResId id;                  // Unique Id of the audio source
   unsigned int type;            // The type of audio source (see CTLPI_AUDIO_SRC_xxx)
   unsigned int format;          // The sample data format (see CTLPI_AUDIO_FORMAT_xxx)
   double sampleRate;            // The sample rate
   unsigned int bufferSize;      // The size of the audio buffer
   uint8_t* buffer;              // The sample data, or null for immediate stream destruction
} AudioUpdateMsg;
