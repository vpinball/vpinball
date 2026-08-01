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
// collaboration around audio streaming and 3 common controller's state data:
// - Machine state (switch, controlled device, logic game states),
// - Alphanumeric segment displays,
// - Matrix displays (dot matrix, CRT,...).
// 
// The design is based around a simple service discovery:
// - a GetSource message is defined for each feature CTLPI_xxx_GET_SRC_MSG), 
//   together with a SourceChangeEvent (CTLPI_xxx_ON_SRC_CHG_MSG).
// - Data provided as an answer to a GetSource message **MUST** remain valid
//   until the next SourceChangeEvent is broadcasted.
// - Sources are advertised with the function hooks that allow to request them.
//   Unless explicitely specified, these hooks are not thread safe and must be
//   called on the plugin API thread (the one calling plugin's Load/Unload).
// - Some source may have an overrideId that allow a plugin to provide a source
//   variant. It is up to the host to decide which one to use.
// - GetSource messages use the same design as Vulkan to evaluate the needed
//   array size: the count field is increased to the number of items while
//   only maxEntryCount are actually copied into the output array.

#define CTLPI_NAMESPACE               "Controller"

// Generic structure used to identify a resource belonging to an endpoint (a single endpoint may only exposes one controller)
typedef union CtlResId
{
   struct {
      uint32_t endpointId;
      uint32_t resId;
   };
   uint64_t id;
} CtlResId;

typedef struct GetCtrlSrcMsg
{
   // Request
   unsigned int maxEntryCount; // see below
   // Response
   unsigned int count; // Number of entries, also position to put next entry, should be increased even if exceeding maxEntryCount to get the total count
   void* entries; // Pointer to an array of maxEntryCount entries to be filled
} GetCtrlSrcMsg;


///////////////////////////////////////////////////////////////////////////////
//
// Controllers
//

// Broadcasted after a controller has been added, modified or removed, there is no message data
#define CTLPI_CONTROLLERS_ON_CHG_MSG    "OnControllersChanged"

// Request subscribers to fill up an array with the list of controller definition blocks, message data is a pointer to a GetControllersMsg structure
#define CTLPI_CONTROLLERS_GET_MSG       "GetControllers"

typedef struct ControllerDef
{
   uint32_t endpointId;     // Note that an endpoint may only expose one controller (a plugin may implement multiple endpoint if needed)
   const char* gameId;      // Must be unique and allow to identify what is emulated and how it is exposed, not who is the controller emulating it (never null)
} ControllerDef;

typedef struct GetControllersMsg
{
   // Request
   unsigned int maxEntryCount; // see below
   // Response
   unsigned int count;         // Number of entries, also position to put next entry, should be increased even if exceeding maxEntryCount to get the total count
   ControllerDef* entries;     // Pointer to an array of maxEntryCount entries to be filled
} GetControllersMsg;


///////////////////////////////////////////////////////////////////////////////
//
// Game states
//

// Broadcasted after a controller state source has been added, modified or removed, there is no message data
#define CTLPI_STATE_ON_SRC_CHG_MSG    "OnStateSrcChanged"

// Request subscribers to fill up an array with the list of state blocks, message data is a pointer to a GetStateSrcMsg structure
#define CTLPI_STATE_GET_SRC_MSG       "GetStateSrc"

typedef struct StateGroupDef
{
   const char* name; // User friendly name, or null if not available, owned by the provider
   const char* desc; // User friendly description, or null if not available, owned by the provider
   uint16_t id;
} StateGroupDef;

#define CTLPI_STATE_TYPE_UINT8            0x0001
#define CTLPI_STATE_TYPE_UINT16           0x0002
#define CTLPI_STATE_TYPE_UINT32           0x0004
#define CTLPI_STATE_TYPE_UINT64           0x0008
#define CTLPI_STATE_TYPE_INT8             0x0010
#define CTLPI_STATE_TYPE_INT16            0x0020
#define CTLPI_STATE_TYPE_INT32            0x0040
#define CTLPI_STATE_TYPE_INT64            0x0080
#define CTLPI_STATE_TYPE_FLOAT            0x0100
#define CTLPI_STATE_TYPE_DOUBLE           0x0200
#define CTLPI_STATE_TYPE_STRING           0x0400 // 0 ended char string, UTF8 encoded, owned by the controller

typedef union CtlStateId
{
   struct {
      uint32_t groupId;
      uint32_t stateId;
   };
   uint64_t mappingId;
} CtlStateId;

typedef struct StateDef
{
   const char* name; // User friendly name, or null if not available, owned by the provider
   const char* desc; // User friendly description, or null if not available, owned by the provider
   CtlStateId id; // User friendly unique mapping id
   int typeMask; // State's supported data type mask. This are the only data types that can be requested/writed, see CTLPI_STATE_TYPE_xxx defines
   int writable; // Non 0 if the state is writable
} StateDef;

typedef struct StateSrcId
{
   CtlResId id; // Unique Id of the input block
   unsigned int nGroups; // Number of groups
   StateGroupDef* groupDefs; // Pointer to a block of nGroups StateGroupDef, owned by the provider, valid until a src changed event is broadcasted
   unsigned int nStates; // Number of states
   StateDef* stateDefs; // Pointer to a block of nStates StateDef, owned by the provider, valid until a src changed event is broadcasted
   int(MSGPIAPI* GetState)(unsigned int inputIndex, int type, void* pResult); // Pointer to function to request a state, thread safe, may be null, returns non 0 on error, pResult points to a memblock corresponding to type
   int(MSGPIAPI* SetState)(unsigned int inputIndex, int type, void* pResult); // Pointer to function to request a state change, thread safe, may be null, returns non 0 on error, pResult points to a memblock corresponding to type
} StateSrcId;

typedef struct GetStateSrcMsg
{
   // Request
   unsigned int maxEntryCount; // see below
   // Response
   unsigned int count; // Number of entries, also position to put next entry, should be increased even if exceeding maxEntryCount to get the total count
   StateSrcId* entries; // Pointer to an array of maxEntryCount entries to be filled
} GetStateSrcMsg;


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
//   example providing upscaling or colorization support, replacing a DMD by
//   an LCD animated display,...
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
#define CTLPI_DISPLAY_HARDWARE_CRT_DISPLAY        0x00040000
#define CTLPI_DISPLAY_HARDWARE_LCD_DISPLAY        0x00050000


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
         uint16_t hardwareModel;
         uint16_t hardwareFamily;
      };
      uint32_t hardware;                                                    // Hardware hint. See CTLPI_DISPLAY_HARDWARE_xxx
   };

   // Render frames, suitable for presenting to the user, but not meant to be backward compatible
   unsigned int frameFormat;                                                // See CTLPI_DISPLAY_FORMAT_xxx
   DisplayFrame(MSGPIAPI* GetRenderFrame)(const CtlResId id);              // Get the display frame. Thread safe. Returned value is not null, owned by the source, in the format defined by frameFormat

   // Identify frames, do not implement the full display emulation but suitable for stable and backward compatible frame identification
   // They are optional and all sources do not implement this feature. If implemented, all fields must be defined, otherwise they must all be 0/null
   unsigned int identifyFormat;                                             // See CTLPI_DISPLAY_ID_FORMAT_xxx 
   DisplayFrame(MSGPIAPI* GetIdentifyFrame)(const CtlResId id);            // Get the last identify frame. Thread safe. Returned value is not null, owned by the source, in the format defined by identifyFormat
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
   SegDisplayFrame(MSGPIAPI* GetState)(const CtlResId id); // Get the display state (one relative luminance value per segment, 16 segments per element, owned by provider), thread safe
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
// An endpoint may expose multiple audio sources. The audio sources are likely
// to be exposed to the user for mixer levels. Audio sources may be overriden
// (similar to displays) allowing plugins to upgrade/replace audio sources of
// other plugins.
// 
// An audio source must be referenced by each audio stream to allow the host
// to handle overiding, global mixer, and routing to the right output.

// Broadcasted after an audio source has been added, modified or removed, there is no message data
#define CTLPI_AUDIO_ON_SRC_CHG_MSG "OnAudioSrcChanged"

// Request subscribers to fill up an array with the list of audio sources, message data is a pointer to a GetAudioSrcMsg structure
#define CTLPI_AUDIO_GET_SRC_MSG    "GetAudioSrc"

// Broadcasted when an audio stream is updated with new samples
#define CTLPI_AUDIO_ON_UPDATE_MSG  "AudioUpdate"

#define CTLPI_AUDIO_TARGET_BACKGLASS         0

#define CTLPI_AUDIO_FORMAT_CHANNEL_MONO      0
#define CTLPI_AUDIO_FORMAT_CHANNEL_STEREO    1

#define CTLPI_AUDIO_FORMAT_SAMPLE_INT16      0
#define CTLPI_AUDIO_FORMAT_SAMPLE_FLOAT      1

typedef struct AudioSrcId
{
   CtlResId id;                  // Unique Id of the audio source
   CtlResId overrideId;          // If this source overrides another source, id of the overridden source, 0 otherwise
   const char* name;             // User friendly name of this source, owned by controller
   const char* desc;             // User friendly description of this source, owned by controller
   unsigned int target;          // Audio output target (see CTLPI_AUDIO_TARGET_xxx)
} AudioSrcId;

typedef struct GetAudioSrcMsg
{
   // Request
   unsigned int maxEntryCount;   // see below
   // Response
   unsigned int count;           // Number of entries, also position to put next entry, should be increased even if exceeding maxEntryCount to get the total count
   AudioSrcId* entries;          // Pointer to an array of maxEntryCount entries to be filled
} GetAudioSrcMsg;

// This message can be sent for 3 use cases:
// - New audio stream: all fields must be defined/not null
// - Enqueueing in an existing stream: bufferSize & buffer and volume must be defined (other fields are ignored)
// - Destroying an existing stream: buffer must be null (other fields are ignored)
// For all these use cases, source and stream must always be defined and valid.
typedef struct AudioUpdateMsg
{
   CtlResId sourceId;            // Unique Id of the audio source
   CtlResId streamId;            // Unique Id of this stream
   unsigned int channelFormat;   // The type of audio source (CTLPI_AUDIO_FORMAT_CHANNEL_xxx)
   unsigned int sampleFormat;    // The sample data format (see CTLPI_AUDIO_FORMAT_SAMPLE_xxx)
   double sampleRate;            // The sample rate
   float volume; // Volume to be applied on this stream
   unsigned int bufferSize;      // The size of the audio buffer
   uint8_t* buffer;              // The sample data, or null for immediate stream destruction
} AudioUpdateMsg;




///////////////////////////////////////////////////////////////////////////////
//
// The following helper macros are designed to easily use the API in standard C++
//
#ifdef __cplusplus
#include <vector>

template <class T> static void GetCtrlItems(const MsgPluginAPI* msgApi, uint32_t endpointId, unsigned int getMsgId, std::vector<T>& list)
{
   GetCtrlSrcMsg getMsg = { 0, 0, nullptr };
   msgApi->BroadcastMsg(endpointId, getMsgId, &getMsg);
   if (getMsg.count > 0)
   {
      list.resize(getMsg.count);
      getMsg = { getMsg.count, 0, list.data() };
      msgApi->BroadcastMsg(endpointId, getMsgId, &getMsg);
   }
   else
   {
      list.clear();
   }
}

template <class T> static std::vector<T> GetCtrlItems(const MsgPluginAPI* msgApi, uint32_t endpointId, unsigned int getMsgId)
{
   std::vector<T> list;
   GetCtrlSrcMsg getMsg = { 0, 0, nullptr };
   msgApi->BroadcastMsg(endpointId, getMsgId, &getMsg);
   if (getMsg.count > 0)
   {
      list.resize(getMsg.count);
      getMsg = { getMsg.count, 0, list.data() };
      msgApi->BroadcastMsg(endpointId, getMsgId, &getMsg);
   }
   return list;
}
#endif
