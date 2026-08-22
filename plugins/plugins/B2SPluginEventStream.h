// license:GPLv3+

#pragma once

#include "plugins/ControllerPlugin.h"

#include <array>
#include <vector>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <functional>

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

   void QueueEvent(char c, int id, int value) const { m_eventHandler(c, id, value); }

private:
   const uint32_t m_endpointId;
   const MsgPluginAPI* const m_msgApi;

   std::function<void(char, int, int)> m_eventHandler;

   bool m_isRunning = true;
   std::thread m_thread;
   std::mutex m_pollSrcMutex;
   void StatePollingThread();

   const unsigned int m_onControllersChangedId;
   const unsigned int m_getControllersId;
   static void OnControllersChanged(const unsigned int eventId, void* userData, void* eventData);
   unsigned int m_pinmameEndPoint = 0;
   unsigned int m_b2sEndPoint = 0;

   const unsigned int m_onSegSrcChangedId = 0;
   const unsigned int m_getSegSrcId = 0;
   std::vector<SegSrcId> m_pmSegSrc;
   std::vector<uint16_t> m_pmLastSegFrame;
   std::vector<unsigned int> m_pmLastSegFrameId;
   static void OnSegSrcChanged(const unsigned int eventId, void* userData, void* eventData);

   PinballPlugin::Controller::CtrlItemConsumer<StateSrcId> m_stateSources;
   std::array<std::vector<int>, 5> m_pmStates;

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
