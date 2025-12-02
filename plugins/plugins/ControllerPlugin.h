// license:GPLv3+

#pragma once

#ifdef __cplusplus
 #include <cstdint>
#else
 #include <stdint.h>
#endif

#include "MsgPlugin.h"

///////////////////////////////////////////////////////////////////////////////
// Generic pinball controller plugin
//
// WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
// This interface is part of a work in progress and will evolve likely a lot
// before being considered stable. Do not use it, or if you do, use it knowing
// that you're plugin will be broken by the upcoming updates.
// WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
//
// This file defines a few core identifiers, messages and struct to ease plugin
// collaboration around 4 common controller's state data:
// - Binary inputs,
// - Controlled devices (not to be confused with binary outputs which are not exposed),
// - Alphanumeric segment displays,
// - Matrix displays (dot matrix, CRT,...).
// 
// The overall design is based around a service discovery approach: a GetSource
// message is defined for each feature CTLPI_xxx_GET_SRC_MSG), together with a
// SourceChangeEvent (CTLPI_xxx_ON_SRC_CHG_MSG). Sources are advertised with the 
// function hooks that allow to request their state. These function hooks may or
// may not be thread safe as advertised in their definition, and must be used 
// and implemented accordingly.
//
// Audio stream output is also supported through a simple broadcast message.
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

// Generic device state definition, used for inputs and devices
typedef struct DeviceDef
{
   char* name; // User friendly name, or null if not available, owned by the provider
   union // User friendly unique mapping id, note that while unique, this id may appear multiple times if a device state is mirrored
   {
      struct
      {
         uint16_t groupId;
         uint16_t deviceId;
      };
      uint32_t mappingId;
   };
} DeviceDef;


///////////////////////////////////////////////////////////////////////////////
//
// Controller state
//

#define CTLPI_EVT_ON_GAME_START       "OnGameStart"       // Broadcasted when controller starts, msgData is a pointer to a CtlOnGameStartMsg struct
#define CTLPI_EVT_ON_GAME_END         "OnGameEnd"         // Broadcasted when controller ends

struct CtlOnGameStartMsg
{
   const char* gameId;
};



///////////////////////////////////////////////////////////////////////////////
//
// Controller input state
//

// Broadcasted after an input source has been added, modified or removed, there is no message data
#define CTLPI_INPUT_ON_SRC_CHG_MSG "OnInputsChanged"

// Request subscribers to fill up an array with the list of input blocks, message data is a pointer to a GetInputSrcMsg structure
#define CTLPI_INPUT_GET_SRC_MSG    "GetInputs"

typedef struct InputSrcId
{
   CtlResId id;                                                                   // Unique Id of the input block
   unsigned int nInputs;                                                          // Number of inputs
   DeviceDef* inputDefs;                                                          // Pointer to a block of nInputs DeviceDef, owned by the provider, valid until a src changed event is broadcasted
   int(MSGPIAPI* GetInputState)(const unsigned int inputIndex);                   // Pointer to function to request an input state
   void(MSGPIAPI* SetInputState)(const unsigned int inputIndex, const int isSet); // Pointer to function to request an input state change
} InputSrcId;

typedef struct GetInputSrcMsg
{
   // Request
   unsigned int maxEntryCount; // see below
   // Response
   unsigned int count;         // Number of entries, also position to put next entry, should be increased even if exceeding maxEntryCount to get the total count
   InputSrcId* entries;        // Pointer to an array of maxEntryCount entries to be filled
} GetInputSrcMsg;



///////////////////////////////////////////////////////////////////////////////
//
// Controlled device states
//

// Broadcasted after a controlled device source has been added, modified or removed, there is no message data
#define CTLPI_DEVICE_ON_SRC_CHG_MSG "OnDevicesChanged"

// Request subscribers to fill up an array with the list of controlled device sources, message data is a pointer to a GetDevSrcMsg structure
#define CTLPI_DEVICE_GET_SRC_MSG    "GetDevices"

typedef struct DevSrcId
{
   CtlResId id;                                                      // Unique Id of the controlled device block
   unsigned int nDevices;                                            // Number of device properties in this block
   DeviceDef* deviceDefs;                                            // Pointer to a block of nDevices DeviceDef, owned by the provider, valid until a src changed event is broadcasted
   uint8_t (MSGPIAPI* GetByteState)(const unsigned int deviceIndex); // Get the state of a device property
   float (MSGPIAPI* GetFloatState)(const unsigned int deviceIndex);  // Get the state of a device property
} DevSrcId;

typedef struct GetDevSrcMsg
{
   // Request
   unsigned int maxEntryCount; // see below
   // Response
   unsigned int count;         // Number of entries, also position to put next entry, should be increased even if exceeding maxEntryCount to get the total count
   DevSrcId* entries;          // Pointer to an array of maxEntryCount entries to be filled
} GetDevSrcMsg;


///////////////////////////////////////////////////////////////////////////////
//
// CRT and Dot Matrix Displays (i.e. displays made of a regular matrix of dots)
//
// API is designed to support (at least) the following use cases:
// - allow searching for the available display sources, suitable for frame 
//   identification and/or frame rendering, allowing for sources to appear and 
//   disappear at runtime (like PinMAME, FlexDMD, alphanumeric to DMD renderer,
//   UltraDMD, VPinSpa,...)
// - declare and provide improved variants of a display frame source, for 
//   example providing upscaling or colorization support.
//

// Broadcasted after a display source has been added, modified or removed, there is no message data
#define CTLPI_DISPLAY_ON_SRC_CHG_MSG              "OnDisplaysChanged"

// Request subscribers to fill up an array with the list of display sources, message data is a pointer to a GetDisplaySrcMsg structure
#define CTLPI_DISPLAY_GET_SRC_MSG                 "GetDisplays"

// Render frame formats
#define CTLPI_DISPLAY_FORMAT_LUM32F               1u
#define CTLPI_DISPLAY_FORMAT_SRGB888              2u
#define CTLPI_DISPLAY_FORMAT_SRGB565              3u

// Identify frame formats
#define CTLPI_DISPLAY_ID_FORMAT_BITPLANE2         1u
#define CTLPI_DISPLAY_ID_FORMAT_BITPLANE4         2u

// Hardware families & models
#define CTLPI_DISPLAY_HARDWARE_FAMILY_MASK        0xFFFF0000
#define CTLPI_DISPLAY_HARDWARE_MODEL_MASK         0x0000FFFF
#define CTLPI_DISPLAY_HARDWARE_UNKNOWN            0x00000000
#define CTLPI_DISPLAY_HARDWARE_NEON_PLASMA        0x00010000
#define CTLPI_DISPLAY_HARDWARE_RED_LED            0x00020000
#define CTLPI_DISPLAY_HARDWARE_STERN_520_5052_05  0x00020001
#define CTLPI_DISPLAY_HARDWARE_STERN_520_5052_15  0x00020002
#define CTLPI_DISPLAY_HARDWARE_RGB_LED            0x00030000


typedef struct DisplayFrame
{
   unsigned int frameId;
   const void* frame;
} DisplayFrame;

typedef struct DisplaySrcId
{
   CtlResId id;                                                             // Unique Id of the display
   CtlResId groupId;                                                        // Unique Id of the display group
   CtlResId overrideId;                                                     // If this source overrides another source, id of the overriden source, 0 otherwise
   unsigned int width;                                                      // 
   unsigned int height;                                                     // 
   union {
      struct {
         uint16_t hardwareFamily;
         uint16_t hardwareModel;
      };
      uint32_t hardware;                                                    // Hardware hint. See CTLPI_DISPLAY_HARDWARE_xxx
   };

   // Render frames, suitable for presenting to the user, but not meant to be backward compatible
   unsigned int frameFormat;                                                // See CTLPI_DISPLAY_FORMAT_xxx
   DisplayFrame (MSGPIAPI* GetRenderFrame)(const CtlResId id);              // Get the display frame. Returned value is not null, owned by the source, in the format defined by frameFormat

   // Identify frames, do not implement the full display emulation but suitable for stable and backward compatible frame identification
   // They are optional and all sources do not implement this feature. If implemented, all fields must be defined, otherwise they must all be 0/null
   unsigned int identifyFormat;                                             // See CTLPI_DISPLAY_ID_FORMAT_xxx 
   DisplayFrame (MSGPIAPI* GetIdentifyFrame)(const CtlResId id);            // Get the last identify frame. Returned value is not null, owned by the source, in the format defined by identifyFormat
} DisplaySrcId;

typedef struct GetDisplaySrcMsg
{
   // Request
   unsigned int maxEntryCount; // see below
   // Response
   unsigned int count;         // Number of entries, also position to put next entry, should be increased even if exceeding maxEntryCount to get the total count
   DisplaySrcId* entries;      // Pointer to an array of maxEntryCount entries to be filled
} GetDisplaySrcMsg;


///////////////////////////////////////////////////////////////////////////////
//
// Segment displays
//

// Broadcasted after an alpha numeric source has been added, modified or removed, there is no message data
#define CTLPI_SEG_ON_SRC_CHG_MSG        "OnSegDisplaysChanged"

// Request subscribers to fill up an array with the list of alpha numeric sources, message data is a pointer to a GetSegSrcMsg structure
#define CTLPI_SEG_GET_SRC_MSG           "GetSegDisplays"

// Individual segment display layouts
typedef enum {
   CTLPI_SEG_LAYOUT_7,          //  7 segments
   CTLPI_SEG_LAYOUT_7C,         //  7 segments and comma
   CTLPI_SEG_LAYOUT_7D,         //  7 segments and dot
   CTLPI_SEG_LAYOUT_9,          //  9 segments
   CTLPI_SEG_LAYOUT_9C,         //  9 segments and comma
   CTLPI_SEG_LAYOUT_14,         // 14 segments
   CTLPI_SEG_LAYOUT_14D,        // 14 segments with dot
   CTLPI_SEG_LAYOUT_14DC,       // 14 segments with dot and comma
   CTLPI_SEG_LAYOUT_16,         // 16 segments (split top/bottom segments)
} SegElementType;

// Hardware families & models
#define CTLPI_SEG_HARDWARE_FAMILY_MASK     0xFFFF0000
#define CTLPI_SEG_HARDWARE_MODEL_MASK      0x0000FFFF
#define CTLPI_SEG_HARDWARE_UNKNOWN         0x00000000
#define CTLPI_SEG_HARDWARE_NEON_PLASMA     0x00010000
#define CTLPI_SEG_HARDWARE_VFD_GREEN       0x00020000
#define CTLPI_SEG_HARDWARE_VFD_BLUE        0x00030000
#define CTLPI_SEG_HARDWARE_GTS1_4DIGIT     0x00030001 // Futaba VFD used for Gottlieb System 1 and 80A status display
#define CTLPI_SEG_HARDWARE_GTS1_6DIGIT     0x00030002 // Futaba VFD used for Gottlieb System 1 and 80 score display
#define CTLPI_SEG_HARDWARE_GTS80A_7DIGIT   0x00030003 // Futaba VFD used for Gottlieb System 80A score display
#define CTLPI_SEG_HARDWARE_GTS80B_20DIGIT  0x00030004 // Futaba VFD used for Gottlieb System 80B
#define CTLPI_SEG_HARDWARE_LED_RED         0x00040000

// Maximum number of individual elements forming a display
#define CTLPI_SEG_MAX_DISP_ELEMENTS  32

typedef struct SegDisplayFrame
{
   unsigned int frameId;
   const float* frame;
} SegDisplayFrame;

typedef struct SegSrcId
{
   CtlResId id;                                             // Unique Id of the display
   CtlResId groupId;                                        // Unique Id of the display group
   union {
      struct {
         uint16_t hardwareFamily;
         uint16_t hardwareModel;
      };
      uint32_t hardware;                                    // Hardware hint. See CTLPI_SEG_HARDWARE_xxx
   };
   unsigned int nElements;                                  // Number of individual elements forming this display
   SegElementType elementType[CTLPI_SEG_MAX_DISP_ELEMENTS]; // Type of each individual element forming this display (0..nElements-1)
   SegDisplayFrame (MSGPIAPI* GetState)(const CtlResId id); // Get the display state (one relative luminance value per segment, 16 segments per element, owned by provider)
} SegSrcId;

typedef struct GetSegSrcMsg
{
   // Request
   unsigned int maxEntryCount; // see below
   // Response
   unsigned int count;         // Number of entries, also position to put next entry, should be increased even if exceeding maxEntryCount to get the total count
   SegSrcId* entries;          // Pointer to an array of maxEntryCount entries to be filled
} GetSegSrcMsg;


///////////////////////////////////////////////////////////////////////////////
//
// Audio streams (backglass, pinsound/altsound/gsound, ...)
//

// Broadcasted when an audio stream is made available, updated or ended
#define CTLPI_AUDIO_ON_UPDATE_MSG "AudioUpdate"

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
   float volume;                 // Dynamic playback volume (0..1)
   unsigned int bufferSize;      // The size of the audio buffer
   uint8_t* buffer;              // The sample data, or null for immediate stream destruction
} AudioUpdateMsg;
