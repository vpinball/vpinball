// license:GPLv3+

#pragma once

#include "plugins/ControllerPlugin.h"

#include <vector>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <functional>

namespace PUP {

// Manager that creates an event stream from state polling and event listening
// Events are identified by type letter, an id and a state, corresponding to DOF events
// Note that depending on the event type, multiple identical events may happen (these are not states)
// W: PinMAME switch events
// N: PinMAME mech state
// L: PinMAME lamp state
// S: PinMAME solenoid state
// G: PinMAME GI state
// D: PinMAME Segment display state, also DMD frame identification Id implemented above, and also internal D0 startup event in PUP
// E: B2S Controller generic input state (B2SSetData / B2SPulseData)
// B: B2S Controller score digit
// C: B2S Controller score
class DOFEventStream
{
public:
   DOFEventStream(const MsgPluginAPI* msgApi, uint32_t endpointId, const std::function<void(char, int, int)>& eventHandler);
   ~DOFEventStream();

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

   unsigned int m_onDmdSrcChangedId = 0;
   unsigned int m_getDmdSrcId = 0;
   unsigned int m_onDevSrcChangedId = 0;
   unsigned int m_getDevSrcId = 0;
   unsigned int m_onInputSrcChangedId = 0;
   unsigned int m_getInputSrcId = 0;
   unsigned int m_onSerumTriggerId = 0;
   unsigned int m_onDmdTriggerId = 0;

   DevSrcId m_pmDevSrc {};
   std::vector<int> m_pmDeviceState;
   InputSrcId m_pmInputSrc {};
   std::vector<int> m_pmSwitchState;

   InputSrcId m_b2sInputSrc {};
   std::vector<int> m_b2sSwitchState;
   DevSrcId m_b2sDevSrc {};
   static void MSGPIAPI OnB2SStateChg(unsigned int index, void* context);

   static void OnDMDSrcChanged(const unsigned int eventId, void* userData, void* eventData);
   unsigned int m_lastDmdFrameId = 0;
   DisplaySrcId m_dmdId {};
   std::function<DisplaySrcId(const GetDisplaySrcMsg&)> m_selectDmd = [](const GetDisplaySrcMsg&) { return DisplaySrcId {}; };
   std::function<int(const DisplaySrcId&, const uint8_t*)> m_processDmd = [](const DisplaySrcId&, const uint8_t*) { return -1; };

   static void OnSerumTrigger(const unsigned int eventId, void* userData, void* eventData);
   static void OnDevSrcChanged(const unsigned int eventId, void* userData, void* eventData);
   static void OnInputSrcChanged(const unsigned int eventId, void* userData, void* eventData);
};

}
