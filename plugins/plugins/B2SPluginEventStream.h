// license:GPLv3+

#pragma once

#include "plugins/ControllerPlugin.h"

#include <array>
#include <atomic>
#include <vector>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <condition_variable>
#include <functional>

// DMD event source: a plugin advertising that it identifies DMD frames for a
// controller and broadcasts the corresponding triggers on an event message.
// Advertised like the other controller resources (GetSrc/OnSrcChanged messages
// resolved in the CTLPI_NAMESPACE namespace by the CtrlItem helpers).
//
// This is currently a custom contract owned by the Serum plugin, as a first
// step before eventually moving it to the common Controller API. Declared here
// rather than shared with that plugin, as consuming this must not mean
// depending on its source. It is part of the wire contract, so it changes in
// both places or neither.
#define DMDESPI_GET_SRC_MSG "GetDmdEventSrc:1"
#define DMDESPI_ON_SRC_CHG_MSG "OnDmdEventSrcChanged:1"

typedef struct DMDEventSrcId
{
   ControllerDef covered; // Controller whose DMD frames this source identifies
   const char* triggerEventName; // Trigger event message name, in the "Serum" message namespace, carrying a pointer to an unsigned int (trigger id) as message data
} DMDEventSrcId;

inline bool operator==(const DMDEventSrcId& a, const DMDEventSrcId& b)
{
   return a.covered == b.covered //
      && a.triggerEventName == b.triggerEventName; // pointer identity, not string content
}

inline bool operator!=(const DMDEventSrcId& a, const DMDEventSrcId& b) { return !(a == b); }

// Manager that creates an event stream from state polling and event listening
// Events are identified by type letter, an id and a state, corresponding to B2S plugin events
// Note that depending on the event type, multiple identical events may happen (these are not states)
// W: PinMAME switch events
// N: PinMAME mech state
// L: PinMAME lamp state
// S: PinMAME solenoid state
// G: PinMAME GI state
// D: PinMAME Segment display state, DMD frame identification (PUP & Serum), PUP's D0 internal startup event
// E: B2S Controller generic input state (B2SSetData / B2SPulseData)
// B: B2S Controller score digit
// C: B2S Controller score
class B2SPluginEventStream
{
public:
   B2SPluginEventStream(const MsgPluginAPI* msgApi, uint32_t endpointId, const ControllerDef& controller, const std::function<void(char, int, int)>& eventHandler);
   ~B2SPluginEventStream();

   void SetDMDHandler(const std::function<DisplaySrcId(const GetDisplaySrcMsg&)>& select, const std::function<int(const DisplaySrcId&, const uint8_t*)>& process);

   // Called when frame identification changes hands, and once on installation
   // with the current state. This file is shared between plugins whose logging
   // lives in their own namespace, so it cannot report this itself.
   void SetDmdIdentificationHandler(const std::function<void(bool)>& onChanged);

   void QueueEvent(char c, int id, int value) const { m_eventHandler(c, id, value); }

   // True while a Serum colorization identifies DMD frames for the running
   // game, in which case this stream does not identify them itself and 'D'
   // events come from Serum. A consumer whose content depends on frame
   // identification can use this to tell "nothing matched" apart from "nothing
   // is matching".
   [[nodiscard]] bool IsDmdIdentifiedBySerum() const { return m_serumIdentifiesFrames.load(std::memory_order_relaxed); }

private:
   const uint32_t m_endpointId;
   const MsgPluginAPI* const m_msgApi;

   std::function<void(char, int, int)> m_eventHandler;
   std::function<void(bool)> m_onDmdIdentificationChanged = [](bool) { };

   bool m_isRunning = true;
   std::thread m_thread;
   std::mutex m_pollSrcMutex;
   void StatePollingThread();

   // Controller this stream is bound to, selected and given by the owner
   const uint32_t m_controllerEndPoint;

   PinballPlugin::Controller::CtrlItemConsumer<DMDEventSrcId> m_dmdEventSources;
   void OnDmdEventSourcesChanged();
   // Written on the MsgAPI thread when the DMD event source list changes, read
   // by the polling thread on every frame, so not a plain bool.
   std::atomic<bool> m_serumIdentifiesFrames { false };

   PinballPlugin::Controller::CtrlItemConsumer<StateSrcId> m_stateSources;
   std::array<std::vector<int>, 5> m_pmStates;

   const unsigned int m_onSegSrcChangedId = 0;
   const unsigned int m_getSegSrcId = 0;
   std::vector<SegSrcId> m_pmSegSrc;
   std::vector<uint16_t> m_pmLastSegFrame;
   std::vector<unsigned int> m_pmLastSegFrameId;
   static void OnSegSrcChanged(const unsigned int eventId, void* userData, void* eventData);

   const unsigned int m_onDmdSrcChangedId = 0;
   const unsigned int m_getDmdSrcId = 0;
   DisplaySrcId m_dmdId { };
   unsigned int m_lastDmdFrameId = 0;
   std::function<DisplaySrcId(const GetDisplaySrcMsg&)> m_selectDmd = [](const GetDisplaySrcMsg&) { return DisplaySrcId {}; };
   std::function<int(const DisplaySrcId&, const uint8_t*)> m_processDmd = [](const DisplaySrcId&, const uint8_t*) { return -1; };
   static void OnDMDSrcChanged(const unsigned int eventId, void* userData, void* eventData);

   const unsigned int m_onB2SStateChangeId = 0;
   static void OnB2SStateChange(const unsigned int eventId, void* userData, void* eventData);

   // Subscription to the selected DMD event source's trigger message, bound
   // dynamically as matching sources appear and disappear
   std::string m_dmdTriggerName;
   unsigned int m_dmdTriggerMsgId = 0;
   static void OnDmdTrigger(const unsigned int eventId, void* userData, void* eventData);
};
