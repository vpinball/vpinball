// license:GPLv3+

#pragma once

#include "plugins/ControllerPlugin.h"

#include <array>
#include <atomic>
#include <vector>
#include <mutex>
#include <string_view>
#include <thread>
#include <condition_variable>
#include <functional>

// What the Serum plugin advertises as a ControllerDef while it holds a
// colorization that identifies DMD frames, suffixed with the game id. It is not
// a controller: it is a statement that somebody else is already matching every
// frame, and that its findings arrive on "Serum"/"OnDmdTrigger:1".
//
// Declared here rather than shared with the Serum plugin, which has its own
// copy: consuming this must not mean depending on that plugin's source. It is
// part of the wire contract, so it changes in both places or neither.
inline constexpr std::string_view serumGameIdPrefix = "serum::";

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
   B2SPluginEventStream(const MsgPluginAPI* msgApi, uint32_t endpointId, const std::function<void(char, int, int)>& eventHandler);
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

   PinballPlugin::Controller::CtrlItemConsumer<ControllerDef> m_controllers;
   unsigned int m_pinmameEndPoint = 0;
   unsigned int m_b2sEndPoint = 0;
   // Written on the MsgAPI thread when the controller list changes, read by the
   // polling thread on every frame, so not a plain bool.
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

   const unsigned int m_onSerumTriggerId = 0;
   static void OnSerumTrigger(const unsigned int eventId, void* userData, void* eventData);
};
