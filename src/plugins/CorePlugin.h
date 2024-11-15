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

#define CTLPI_NAMESPACE                       "Controller"

// Request subscribers to fill up an array with the list of DMD sources
#define CTLPI_GETDMD_SRC_MSG                  "GetDMDSource"

// Request subscribers for a DMD frame suited for rendering (best visual)
#define CTLPI_GETDMD_RENDER_MSG               "GetDMD"

// Request subscribers for a DMD frame suited for frame identification (stable encoding)
#define CTLPI_GETDMD_IDENTIFY_MSG             "GetDMDIdentify"

// GetDMD request flags
#define CTLPI_GETDMD_FLAG_RENDER_SIZE_REQ     1  // Frame size of Render DMD is fixed by requester
#define CTLPI_GETDMD_FLAG_RENDER_FMT_REQ      2  // Format of Render DMD is fixed by requester

// Render DMD frame formats
#define CTLPI_GETDMD_FORMAT_LUM8              1
#define CTLPI_GETDMD_FORMAT_SRGB888           2
#define CTLPI_GETDMD_FORMAT_SRGB565           5

// Identify DMD frame formats
#define CTLPI_GETDMD_FORMAT_BITPLANE2         3
#define CTLPI_GETDMD_FORMAT_BITPLANE4         4

typedef struct GetDmdSrcEntry
{
   unsigned int dmdId;  // Unique Id of the DMD, built with source endpointId in the upper word, and controller local id in the lowest word. A DMD may be reported mutiple times with the same dmdId but different size/format thanks to frame improvment like colorization.
   unsigned int width;  //
   unsigned int height; //
   unsigned int format; //
} GetDmdSrcEntry;

typedef struct GetDmdSrcMsg
{
   // Request
   unsigned int maxEntryCount; // see below
   // Response
   unsigned int count;
   GetDmdSrcEntry* entries;   // Pointer to an array of maxEntryCount entries, to be filled starting from empty (width/height=0) slots from beginning of array
} GetDmdSrcMsg;

typedef struct GetDmdMsg
{
   // Request
   unsigned int dmdId;        // Uniquely identify a DMD (see GetDmdSrcMsg)
   unsigned int requestFlags; // Request options, see CTLPI_GETDMD_FLAG_xxx constants
   // Request & Response
   unsigned int width;        // Frame width, always defined on response, valid on request if CTLPI_GETDMD_FLAG_RENDER_SIZE_REQ is set
   unsigned int height;       // Frame height, always defined on response, valid on request if CTLPI_GETDMD_FLAG_RENDER_SIZE_REQ is set
   unsigned int format;       // Frame data format, see CTLPI_GETDMD_FORMAT_xxx constants, always defined on response, valid on request if CTLPI_GETDMD_FLAG_RENDER_FMT_REQ is set
   // Response
   unsigned int frameId;      // Id that can be used to discard identical frames
   char* frame;               // Pointer to frame data, null until a provider answers the request, owned by the provider
} GetDmdMsg;
