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
//   together with a SourceChangeEvent (CTLPI_xxx_ON_SRC_CHG_MSG), allowing
//   to advertise a distributed list for each controller components.
// - Data provided as an answer to a GetSource message **MUST** remain valid
//   until the next SourceChangeEvent is broadcasted **AND** processed.
// - Sources are advertised with the function hooks that allow to request them.
//   Unless explicitely specified, these hooks are not thread safe and must be
//   called on the plugin API thread (the one calling plugin's Load/Unload).
// - Some source may have an overrideId that allow a plugin to provide a source
//   variant. It is up to the host to decide which one to use.
// - GetSource messages use the same design as Vulkan to evaluate the needed
//   array size: the count field is increased to the number of items while
//   only maxEntryCount are actually copied into the output array.
// - C++ Helpers are provided to limit boiler plate code, and ease using these
//   while enforcing threading safety.

#define CTLPI_NAMESPACE               "Controller"

// Generic structure used to identify a resource belonging to an endpoint (a single endpoint may only exposes one controller)
typedef union CtlResId
{
   struct
   {
      uint32_t endpointId;
      uint32_t resId;
   };
   uint64_t id;
} CtlResId;


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

#define CTLPI_STATE_FORMAT_UINT8              1
#define CTLPI_STATE_FORMAT_UINT16             2
#define CTLPI_STATE_FORMAT_UINT32             3
#define CTLPI_STATE_FORMAT_UINT64             4
#define CTLPI_STATE_FORMAT_INT8               5
#define CTLPI_STATE_FORMAT_INT16              6
#define CTLPI_STATE_FORMAT_INT32              7
#define CTLPI_STATE_FORMAT_INT64              8
#define CTLPI_STATE_FORMAT_FLOAT              9
#define CTLPI_STATE_FORMAT_DOUBLE            10
#define CTLPI_STATE_FORMAT_STRING            11 // 0 ended char string, UTF8 encoded, owned by the controller

#define CTLPI_STATE_TYPE_CUSTOM               0 // Custom game state, precise definition must be provided by the controller
#define CTLPI_STATE_TYPE_SWITCH               1 // Binary switch state: 0 for opened, non 0 for closed
#define CTLPI_STATE_TYPE_RELATIVE_BRIGHTNESS  2 // Relative linear brightness: linear perceived luminance normalized to the positive data range of the data format (0..1, 0..255,...)

typedef struct StateDef
{
   const char* name; // User friendly name, or null if not available, owned by the provider
   const char* desc; // User friendly description, or null if not available, owned by the provider
   uint32_t mappingId; // User friendly mapping id
   int dataFormat; // Data format, see CTLPI_STATE_FORMAT_xxx defines
   int semanticType; // Game state type, see CTLPI_STATE_TYPE_xxx defines
   void(MSGPIAPI* GetState)(CtlResId blockId, unsigned int stateIndex, void* pResult); // Pointer to function to request a state, thread safe, may be null, pResult points to a memblock corresponding to format
   void(MSGPIAPI* SetState)(CtlResId blockId, unsigned int stateIndex, const void* pValue); // Pointer to function to request a state change, thread safe, may be null, pResult points to a memblock corresponding to format (const char* for string)
} StateDef;

typedef struct StateSrcId
{
   CtlResId id; // Unique Id of the state block
   const char* name; // User friendly name, or null if not available, owned by the provider
   const char* desc; // User friendly description, or null if not available, owned by the provider
   unsigned int nStates; // Number of states
   StateDef* stateDefs; // Pointer to a block of nStates StateDef, owned by the provider, valid until a src changed event is broadcasted
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
// CtrlItemProvider/CtrlItemConsumer allow to easily implement the shared & decentralized list of items exposed by plugins, following the GetSource/OnSourceChanged pattern, 
// with automatic subscription/unsubscription and thread safety. The main challenge here is that the list of items is made of individual plugin owned blocks changing on the 
// plugin API thread, while the datablock may be used from other threads.
// 
// To avoid any race condition:
// - adding or removing datablocks to the shared & decentralized list must be done on the plugin API thread, and the list must be locked while being modified or read
// - the datablocks must be valids before being advertised through the OnSourceChanged event and remain valid until after the next OnSourceChanged event is broadcasted & processed.
// - the datablocks must be served through the GetSource function pointer, starting from the processing of the OnSourceChanged event until before the deregistration OnSourceChanged event is processed.
//
#ifdef __cplusplus
#include <assert.h>
#include <exception>
#include <functional>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

inline bool operator==(const CtlResId& a, const CtlResId& b) { return a.id == b.id; }

inline bool operator==(const ControllerDef& a, const ControllerDef& b)
{
   return a.endpointId == b.endpointId //
      && a.gameId == b.gameId; // pointer identity, not string content
}

inline bool operator!=(const ControllerDef& a, const ControllerDef& b) { return !(a == b); }

inline bool operator==(const StateSrcId& a, const StateSrcId& b)
{
   return a.id == b.id //
      && a.name == b.name // pointer identity, not string content
      && a.desc == b.desc // pointer identity, not string content
      && a.nStates == b.nStates //
      && a.stateDefs == b.stateDefs; // pointer identity, not deep comparison
}

inline bool operator!=(const StateSrcId& a, const StateSrcId& b) { return !(a == b); }

inline bool operator==(const DisplaySrcId& a, const DisplaySrcId& b)
{
   return a.id == b.id //
      && a.groupId == b.groupId //
      && a.overrideId == b.overrideId //
      && a.width == b.width //
      && a.height == b.height //
      && a.hardware == b.hardware //
      && a.frameFormat == b.frameFormat //
      && a.GetRenderFrame == b.GetRenderFrame //
      && a.identifyFormat == b.identifyFormat //
      && a.GetIdentifyFrame == b.GetIdentifyFrame;
}

inline bool operator!=(const DisplaySrcId& a, const DisplaySrcId& b) { return !(a == b); }

inline bool operator==(const SegSrcId& a, const SegSrcId& b)
{
   if (a.id != b.id //
      || a.groupId != b.groupId //
      || a.hardware != b.hardware //
      || a.nElements != b.nElements //
      || a.GetState != b.GetState)
   {
      return false;
   }
   for (unsigned int i = 0; i < a.nElements; ++i)
   {
      if (a.elementType[i] != b.elementType[i])
         return false;
   }
   return true;
}

inline bool operator!=(const SegSrcId& a, const SegSrcId& b) { return !(a == b); }

inline bool operator==(const AudioSrcId& a, const AudioSrcId& b)
{
   return a.id == b.id //
      && a.overrideId == b.overrideId //
      && a.name == b.name // pointer identity, not string content
      && a.desc == b.desc // pointer identity, not string content
      && a.target == b.target;
}

inline bool operator!=(const AudioSrcId& a, const AudioSrcId& b)
{
    return !(a == b);
}

namespace PinballPlugin::Controller
{

// Extract get game from controller gameId (format is layout :: gameid)
inline std::string_view CtrlGetGameKey(const char* gameId)
{
   const std::string_view id(gameId);
   const size_t sep = id.find("::");
   return sep == std::string_view::npos ? id : id.substr(sep + 2);
}

template <class T> struct GetCtrlSrcMsg
{
   // Request
   unsigned int maxEntryCount; // see below
   // Response
   unsigned int count; // Number of entries, also position to put next entry, should be increased even if exceeding maxEntryCount to get the total count
   T* entries; // Pointer to an array of maxEntryCount entries to be filled
};

// Simple item provider helper, single threaded, tied to the MsgAPI thread
template <class T> class CtrlItemProvider
{
public:
   CtrlItemProvider(const MsgPluginAPI* msgApi, uint32_t endpointId, const char* getMsgName, const char* onChangeMsgName)
      : m_msgApi(msgApi)
      , m_endpointId(endpointId)
      , m_getMsgId(msgApi->GetMsgID(CTLPI_NAMESPACE, getMsgName))
      , m_onChangeMsgId(msgApi->GetMsgID(CTLPI_NAMESPACE, onChangeMsgName))
   {
   }

   ~CtrlItemProvider()
   {
      assert(std::this_thread::get_id() == m_threadLock);
      ClearItems();
      m_msgApi->ReleaseMsgID(m_getMsgId);
      m_msgApi->ReleaseMsgID(m_onChangeMsgId);
   }

   void SetItem(const T& item)
   {
      // Note that we must do 2 change broadcast as the first (clear) broadcast ensures that there aren't any other thread using 
      // the previous items before discarding them and advertising the new element. Doing it in a single pass would risk a threading
      // crash if previous element were used while being discarded and replaced.
      ClearItems();
      AddItem(item);
   }

   void AddItem(const T& item)
   {
      assert(std::this_thread::get_id() == m_threadLock);
      m_items.push_back(item);
      if (m_items.size() == 1)
         m_msgApi->SubscribeMsg(m_endpointId, m_getMsgId, OnGetItems, this);
      m_msgApi->BroadcastMsg(m_endpointId, m_onChangeMsgId, nullptr);
   }

   void AddItems(const std::vector<T>& list)
   {
      assert(std::this_thread::get_id() == m_threadLock);
      m_items.insert(m_items.end(), list.begin(), list.end());
      if (m_items.size() == list.size())
         m_msgApi->SubscribeMsg(m_endpointId, m_getMsgId, OnGetItems, this);
      m_msgApi->BroadcastMsg(m_endpointId, m_onChangeMsgId, nullptr);
   }

   void ClearItems()
   {
      assert(std::this_thread::get_id() == m_threadLock);
      if (m_items.empty())
         return;
      m_items.clear();
      m_msgApi->UnsubscribeMsg(m_getMsgId, OnGetItems, this);
      m_msgApi->BroadcastMsg(m_endpointId, m_onChangeMsgId, nullptr);
   }

   const std::vector<T>& GetItems() const
   {
      assert(std::this_thread::get_id() == m_threadLock);
      return m_items;
   }

private:
   static void OnGetItems(const unsigned int eventId, void* userData, void* msgData)
   {
      CtrlItemProvider<T>* me = static_cast<CtrlItemProvider<T>*>(userData);
      assert(std::this_thread::get_id() == me->m_threadLock);
      GetCtrlSrcMsg<T>* getMsg = static_cast<GetCtrlSrcMsg<T>*>(msgData);
      auto it = me->m_items.begin();
      while (it != me->m_items.end() && getMsg->count < getMsg->maxEntryCount)
      {
         getMsg->entries[getMsg->count] = *it;
         getMsg->count++;
         ++it;
      }
      getMsg->count += static_cast<unsigned int>(std::distance(it, me->m_items.end()));
   }

   const std::thread::id m_threadLock { std::this_thread::get_id() };
   const MsgPluginAPI* m_msgApi;
   const uint32_t m_endpointId;
   const unsigned int m_getMsgId;
   const unsigned int m_onChangeMsgId;

   std::vector<T> m_items;
};

// Gather a shared controller item list. Single threaded, tied to MsgAPI thread.
// May not be used for State and Display items which expose non thread safe getter/setter members
template <class T> static void GetCtrlItems(const MsgPluginAPI* msgApi, uint32_t endpointId, unsigned int getMsgId, std::vector<T>& list)
{
   GetCtrlSrcMsg<T> getMsg = { 0, 0, nullptr };
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

// Gather a shared controller item list. Single threaded, tied to MsgAPI thread.
// May not be used for State and Display items which expose non thread safe getter/setter members
template <class T> static std::vector<T> GetCtrlItems(const MsgPluginAPI* msgApi, uint32_t endpointId, unsigned int getMsgId)
{
   std::vector<T> list;
   GetCtrlSrcMsg<T> getMsg = { 0, 0, nullptr };
   msgApi->BroadcastMsg(endpointId, getMsgId, &getMsg);
   if (getMsg.count > 0)
   {
      list.resize(getMsg.count);
      getMsg = { getMsg.count, 0, list.data() };
      msgApi->BroadcastMsg(endpointId, getMsgId, &getMsg);
   }
   return list;
}

// Provide a consumer with a list of items gathered from distributed plugin, in a multithreaded context.
// . The list only mutates on the MsgApi thread, either reflecting change events, or during a `Subscribe`/`Unsubscribe` call
// . The list is empty until subscribed, after which it is automatically populated and kept up to date.
// . `Unsubscribe` **MUST** be called before destruction. During a call to `Unsubscribe`, the list mutates to an empty state.
// . List items are immutable and valid from a change event until the end of processing of the corresponding end-of-life
//   change event (Note that C++ prevents vector<const T> so this is not enforced, but is strictly required)
// . Items may only be accessed through the `With` method to guarantee proper synchronization against list change events.
// . When list content changes (always on the MsgApi thread):
//   - `onItemsAboutToChange` is called. Before returning, this method **MUST**:
//     . prevent client threads from starting new list-dependent operations;
//     . wait for all existing list-dependent operations to complete;
//     . discard all copied or borrowed data referring to the current items.
//   - `onItemsChanged` is called, allowing to resume processing.
// . Clients may cache copies of items or item-derived data between change events.
//   If the cached data borrows from provider-owned storage:
//   - it may only be accessed as part of a With() operation;
//   - it must be discarded by onItemsAboutToChange before that callback returns.
// . The filter and lifecycle callbacks **MUST NOT** throw.
template <class T> class CtrlItemConsumer
{
public:
   CtrlItemConsumer(const MsgPluginAPI* msgApi, uint32_t endpointId, const char* getMsgName, const char* onChangeMsgName,
      const std::function<void(std::vector<T>&)>& filterItems,
      const std::function<void()>& onItemsAboutToChange,
      const std::function<void()>& onItemsChanged)
      : m_msgApi(msgApi)
      , m_endpointId(endpointId)
      , m_getMsgId(msgApi->GetMsgID(CTLPI_NAMESPACE, getMsgName))
      , m_onChangeMsgId(msgApi->GetMsgID(CTLPI_NAMESPACE, onChangeMsgName))
      , m_filterItems(filterItems)
      , m_onItemsAboutToChange(onItemsAboutToChange)
      , m_onItemsChanged(onItemsChanged)
   {
   }

   CtrlItemConsumer(const CtrlItemConsumer&) = delete;
   CtrlItemConsumer& operator=(const CtrlItemConsumer&) = delete;
   CtrlItemConsumer(CtrlItemConsumer&&) = delete;
   CtrlItemConsumer& operator=(CtrlItemConsumer&&) = delete;

   [[nodiscard]] bool IsSubscribed() const
   {
      assert(std::this_thread::get_id() == m_msgApiThreadId);
      return m_subscribed;
   }
   
   void Subscribe() noexcept
   {
      assert(std::this_thread::get_id() == m_msgApiThreadId);
      assert(!m_subscribed);
      assert(!m_isUpdatingList);
      m_msgApi->SubscribeMsg(m_endpointId, m_onChangeMsgId, OnItemsChanged, this);
      m_subscribed = true;
      UpdateList();
   }

   void Refresh() noexcept
   {
      assert(std::this_thread::get_id() == m_msgApiThreadId);
      if (m_subscribed)
         UpdateList();
   }

   void Unsubscribe() noexcept
   {
      assert(std::this_thread::get_id() == m_msgApiThreadId);
      assert(m_subscribed);
      assert(!m_isUpdatingList);
      m_subscribed = false;
      m_msgApi->UnsubscribeMsg(m_onChangeMsgId, OnItemsChanged, this);
      {
         std::lock_guard lock(m_listMutex);
         if (m_items.empty())
            return;
      }
      try
      {
         if (m_onItemsAboutToChange)
            m_onItemsAboutToChange();
         {
            std::lock_guard lock(m_listMutex);
            m_items.clear();
         }
         if (m_onItemsChanged)
            m_onItemsChanged();
      }
      catch (...)
      {
         std::terminate();
      }
   }
   
   ~CtrlItemConsumer()
   {
      assert(std::this_thread::get_id() == m_msgApiThreadId);
      assert(!m_subscribed);
      m_msgApi->ReleaseMsgID(m_getMsgId);
      m_msgApi->ReleaseMsgID(m_onChangeMsgId);
   }
   
   template <typename Func> auto With(Func&& func) const -> decltype(func(std::declval<const std::vector<T>&>()))
   {
      std::lock_guard lock(m_listMutex);
      return std::forward<Func>(func)(m_items);
   }

private:
   void UpdateList() noexcept
   {
      assert(std::this_thread::get_id() == m_msgApiThreadId);
      assert(m_subscribed);

      // Coalesce reentrant UpdateList() calls to avoid recursive updates.
      if (m_isUpdatingList)
      {
         m_listUpdatePending = true;
         return;
      }

      m_isUpdatingList = true;
      m_listUpdatePending = true;
      while (m_subscribed && m_listUpdatePending)
      {
         m_listUpdatePending = false;

         std::vector<T> items = GetCtrlItems<T>(m_msgApi, m_endpointId, m_getMsgId);
         if (!items.empty() && m_filterItems)
            m_filterItems(items);
         if (!m_subscribed)
            break;

         {
            std::lock_guard lock(m_listMutex); // Not entirely needed as items are const (no modification through `With`) and list is only changed on the  MsgAPI thread
            if (m_items == items)
               continue;
         }
         
         if (m_onItemsAboutToChange)
            m_onItemsAboutToChange();
         if (!m_subscribed)
            break;
         {
            std::lock_guard lock(m_listMutex);
            m_items = std::move(items);
         }
         if (m_onItemsChanged)
            m_onItemsChanged();
      }
      m_isUpdatingList = false;
   }

   static void OnItemsChanged(const unsigned int, void* userData, void*) noexcept
   {
      CtrlItemConsumer<T>* me = static_cast<CtrlItemConsumer<T>*>(userData);
      me->UpdateList();
   }

   const std::thread::id m_msgApiThreadId { std::this_thread::get_id() };
   const MsgPluginAPI* const m_msgApi;
   const uint32_t m_endpointId;
   const unsigned int m_getMsgId;
   const unsigned int m_onChangeMsgId;
   const std::function<void(std::vector<T>&)> m_filterItems;
   const std::function<void()> m_onItemsAboutToChange;
   const std::function<void()> m_onItemsChanged;

   std::vector<T> m_items;
   mutable std::mutex m_listMutex;

   bool m_isUpdatingList = false;
   bool m_listUpdatePending = false;
   
   bool m_subscribed = false;
};

};

#endif
