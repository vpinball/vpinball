// license:GPLv3+

#include "DOFStreamEvent.h"

#include <cstring>
#include <string>
using std::string;
using namespace std::string_literals;

using std::vector;

namespace PUP
{

DOFEventStream::DOFEventStream(const MsgPluginAPI* msgApi, uint32_t endpointId, const std::function<void(char, int, int)>& eventHandler)
   : m_endpointId(endpointId) 
   , m_msgApi(msgApi)
   , m_eventHandler(eventHandler)
{
   // Subscribe to message bus events
   m_getDmdSrcId = m_msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DISPLAY_GET_SRC_MSG);
   m_onDmdSrcChangedId = m_msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DISPLAY_ON_SRC_CHG_MSG);
   m_onSerumTriggerId = m_msgApi->GetMsgID("Serum", "OnDmdTrigger");
   m_onDmdTriggerId = m_msgApi->GetMsgID("PinUp", "OnDmdTrigger");

   m_getDevSrcId = m_msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DEVICE_GET_SRC_MSG);
   m_onDevSrcChangedId = m_msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DEVICE_ON_SRC_CHG_MSG);

   m_getInputSrcId = m_msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_INPUT_GET_SRC_MSG);
   m_onInputSrcChangedId = m_msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_INPUT_ON_SRC_CHG_MSG);

   m_msgApi->SubscribeMsg(m_endpointId, m_onDmdSrcChangedId, OnDMDSrcChanged, this);
   m_msgApi->SubscribeMsg(m_endpointId, m_onDevSrcChangedId, OnDevSrcChanged, this);
   m_msgApi->SubscribeMsg(m_endpointId, m_onInputSrcChangedId, OnInputSrcChanged, this);
   m_msgApi->SubscribeMsg(m_endpointId, m_onSerumTriggerId, OnSerumTrigger, this);
   OnDMDSrcChanged(m_onDmdSrcChangedId, this, nullptr);
   OnDevSrcChanged(m_onDevSrcChangedId, this, nullptr);
   OnInputSrcChanged(m_onInputSrcChangedId, this, nullptr);

   m_thread = std::thread(&DOFEventStream::StatePollingThread, this);
}

DOFEventStream::~DOFEventStream()
{
   m_isRunning = false;
   if (m_thread.joinable())
      m_thread.join();

   for (unsigned int i = 0; i < m_b2sDevSrc.nDevices; i++)
      m_b2sDevSrc.SetChangeCallback(i, 0, OnB2SStateChg, this);

   m_msgApi->UnsubscribeMsg(m_onDmdSrcChangedId, OnDMDSrcChanged);
   m_msgApi->UnsubscribeMsg(m_onDevSrcChangedId, OnDevSrcChanged);
   m_msgApi->UnsubscribeMsg(m_onInputSrcChangedId, OnInputSrcChanged);
   m_msgApi->UnsubscribeMsg(m_onSerumTriggerId, OnSerumTrigger);
   delete[] m_b2sInputSrc.inputDefs;
   delete[] m_b2sDevSrc.deviceDefs;
   delete[] m_pmInputSrc.inputDefs;
   delete[] m_pmDevSrc.deviceDefs;

   m_msgApi->ReleaseMsgID(m_getDevSrcId);
   m_msgApi->ReleaseMsgID(m_onDevSrcChangedId);

   m_msgApi->ReleaseMsgID(m_getInputSrcId);
   m_msgApi->ReleaseMsgID(m_onInputSrcChangedId);

   m_msgApi->ReleaseMsgID(m_getDmdSrcId);
   m_msgApi->ReleaseMsgID(m_onDmdSrcChangedId);
   m_msgApi->ReleaseMsgID(m_onSerumTriggerId);
   m_msgApi->ReleaseMsgID(m_onDmdTriggerId);
}

void DOFEventStream::SetDMDHandler(const std::function<DisplaySrcId(const GetDisplaySrcMsg&)>& select, const std::function<int(const DisplaySrcId&, const uint8_t*)>& process)
{
   m_selectDmd = select;
   m_processDmd = process;
   OnDMDSrcChanged(m_onDmdSrcChangedId, this, nullptr);
}

// Broadcasted by Serum plugin when frame triggers are identified
void DOFEventStream::OnSerumTrigger(const unsigned int eventId, void* userData, void* eventData)
{
   auto me = static_cast<DOFEventStream*>(userData);
   auto trigger = static_cast<unsigned int*>(eventData);
   me->QueueEvent('D', static_cast<int>(*trigger), 1);
}

void DOFEventStream::OnDMDSrcChanged(const unsigned int eventId, void* userData, void* eventData)
{
   auto me = static_cast<DOFEventStream*>(userData);
   std::lock_guard lock(me->m_pollSrcMutex);
   me->m_dmdId.id.id = 0;
   GetDisplaySrcMsg getSrcMsg = { 0, 0, nullptr };
   me->m_msgApi->BroadcastMsg(me->m_endpointId, me->m_getDmdSrcId, &getSrcMsg);
   getSrcMsg = { getSrcMsg.count, 0, new DisplaySrcId[getSrcMsg.count] };
   me->m_msgApi->BroadcastMsg(me->m_endpointId, me->m_getDmdSrcId, &getSrcMsg);
   me->m_dmdId = me->m_selectDmd(getSrcMsg);
   me->m_lastDmdFrameId = 0;
   delete[] getSrcMsg.entries;
}

void DOFEventStream::OnDevSrcChanged(const unsigned int eventId, void* userData, void* eventData)
{
   auto me = static_cast<DOFEventStream*>(userData);
   std::unique_lock lock(me->m_pollSrcMutex);

   // PinMAME controller
   delete[] me->m_pmDevSrc.deviceDefs;
   memset(&me->m_pmDevSrc, 0, sizeof(me->m_pmDevSrc));
   me->m_pmDeviceState.clear();
   if (unsigned int pinmameEndPoint = me->m_msgApi->GetPluginEndpoint("PinMAME"); pinmameEndPoint)
   {
      GetDevSrcMsg getSrcMsg = { 1, 0, &me->m_pmDevSrc };
      me->m_msgApi->SendMsg(me->m_endpointId, me->m_getDevSrcId, pinmameEndPoint, &getSrcMsg);
      if (getSrcMsg.count && me->m_pmDevSrc.deviceDefs)
      {
         // Copy device definitions
         DeviceDef* devices = new DeviceDef[me->m_pmDevSrc.nDevices];
         memcpy(devices, me->m_pmDevSrc.deviceDefs, me->m_pmDevSrc.nDevices * sizeof(DeviceDef));
         me->m_pmDevSrc.deviceDefs = devices;
         me->m_pmDeviceState.resize(me->m_pmDevSrc.nDevices, -1);
      }
   }

   // B2S controller
   delete[] me->m_b2sDevSrc.deviceDefs;
   memset(&me->m_b2sDevSrc, 0, sizeof(me->m_b2sDevSrc));
   if (unsigned int b2sEndPoint = me->m_msgApi->GetPluginEndpoint("B2S"); b2sEndPoint)
   {
      GetDevSrcMsg getSrcMsg = { 1, 0, &me->m_b2sDevSrc };
      me->m_msgApi->SendMsg(me->m_endpointId, me->m_getDevSrcId, b2sEndPoint, &getSrcMsg);
      if (getSrcMsg.count && me->m_b2sDevSrc.deviceDefs)
      {
         // Copy device definitions and register state change listener
         DeviceDef* devices = new DeviceDef[me->m_b2sDevSrc.nDevices];
         memcpy(devices, me->m_b2sDevSrc.deviceDefs, me->m_b2sDevSrc.nDevices * sizeof(DeviceDef));
         me->m_b2sDevSrc.deviceDefs = devices;
         for (unsigned int i = 0; i < me->m_b2sDevSrc.nDevices; i++)
            me->m_b2sDevSrc.SetChangeCallback(i, 1, OnB2SStateChg, me);
      }
   }

   lock.unlock();
   for (unsigned int i = 0; i < me->m_b2sDevSrc.nDevices; i++)
      me->QueueEvent('E', static_cast<int>(me->m_b2sDevSrc.deviceDefs[i].deviceId), me->m_b2sDevSrc.GetFloatState(i) > 0.5f ? 1 : 0);
}

void MSGPIAPI DOFEventStream::OnB2SStateChg(unsigned int index, void* context)
{
   auto me = static_cast<DOFEventStream*>(context);
   me->QueueEvent('E', static_cast<int>(me->m_b2sDevSrc.deviceDefs[index].deviceId), me->m_b2sDevSrc.GetFloatState(index) > 0.5f ? 1 : 0);
}

void DOFEventStream::OnInputSrcChanged(const unsigned int eventId, void* userData, void* eventData)
{
   auto me = static_cast<DOFEventStream*>(userData);
   std::lock_guard lock(me->m_pollSrcMutex);

   delete[] me->m_pmInputSrc.inputDefs;
   memset(&me->m_pmInputSrc, 0, sizeof(me->m_pmInputSrc));
   me->m_pmSwitchState.clear();
   if (unsigned int pinmameEndPoint = me->m_msgApi->GetPluginEndpoint("PinMAME"); pinmameEndPoint)
   {
      GetInputSrcMsg getSrcMsg = { 1, 0, &me->m_pmInputSrc };
      me->m_msgApi->SendMsg(me->m_endpointId, me->m_getInputSrcId, pinmameEndPoint, &getSrcMsg);
      if (getSrcMsg.count && me->m_pmInputSrc.inputDefs)
      {
         // Copy device definitions
         DeviceDef* devices = new DeviceDef[me->m_pmInputSrc.nInputs];
         memcpy(devices, me->m_pmInputSrc.inputDefs, me->m_pmInputSrc.nInputs * sizeof(DeviceDef));
         me->m_pmInputSrc.inputDefs = devices;
         me->m_pmSwitchState.resize(me->m_pmInputSrc.nInputs, 2);
      }
   }

   delete[] me->m_b2sInputSrc.inputDefs;
   memset(&me->m_b2sInputSrc, 0, sizeof(me->m_b2sInputSrc));
   if (unsigned int b2sEndPoint = me->m_msgApi->GetPluginEndpoint("B2S"); b2sEndPoint)
   {
      GetInputSrcMsg getSrcMsg = { 1, 0, &me->m_b2sInputSrc };
      me->m_msgApi->SendMsg(me->m_endpointId, me->m_getInputSrcId, b2sEndPoint, &getSrcMsg);
      if (getSrcMsg.count && me->m_b2sInputSrc.inputDefs)
      {
         // Copy device definitions
         DeviceDef* devices = new DeviceDef[me->m_b2sInputSrc.nInputs];
         memcpy(devices, me->m_b2sInputSrc.inputDefs, me->m_b2sInputSrc.nInputs * sizeof(DeviceDef));
         me->m_b2sInputSrc.inputDefs = devices;
      }
   }
}

// Update thread that poll state based sources (DMD, lamps,...) at 60Hz
void DOFEventStream::StatePollingThread()
{
   //SetThreadName("DOFEventStream.StatePollThread"s);
   while (m_isRunning)
   {
      std::this_thread::sleep_for(std::chrono::microseconds(16666));
      if (!m_isRunning)
         break;

      if (m_dmdId.id.id != 0)
      {
         DisplayFrame dmdFrame = m_dmdId.GetIdentifyFrame(m_dmdId.id);
         if (dmdFrame.frameId != m_lastDmdFrameId)
         {
            m_lastDmdFrameId = dmdFrame.frameId;
            const int dmdTrigger = m_processDmd(m_dmdId, static_cast<const uint8_t*>(dmdFrame.frame));
            if (dmdTrigger >= 0)
               QueueEvent('D', dmdTrigger, 1);
         }
      }

      // W: PinMAME switch events
      // TODO implement switch state event in PinMAME
      for (unsigned int i = 0; i < m_pmInputSrc.nInputs; i++)
      {
         if (m_pmInputSrc.inputDefs[i].groupId == 0x0001)
         {
            const int state = m_pmInputSrc.GetInputState(i) ? 1 : 0;
            if (m_pmSwitchState[i] != state)
            {
               QueueEvent('W', m_pmInputSrc.inputDefs[i].deviceId, state);
               m_pmSwitchState[i] = state;
            }
         }
      }

      // L: PinMAME lamp state
      // N: PinMAME mech state
      // S: PinMAME solenoid state
      // G: PinMAME GI state
      for (unsigned int i = 0; i < m_pmDevSrc.nDevices; i++)
      {
         const int state = m_pmDevSrc.GetFloatState(i) > 0.5f ? 1 : 0;
         if (m_pmDeviceState[i] != state)
         {
            m_pmDeviceState[i] = state;
            switch (m_pmDevSrc.deviceDefs[i].groupId & 0xFF00)
            {
            case 0x0000: QueueEvent('S', m_pmDevSrc.deviceDefs[i].deviceId, state); break;
            case 0x0100: QueueEvent('G', m_pmDevSrc.deviceDefs[i].deviceId, state); break;
            case 0x0200: QueueEvent('L', m_pmDevSrc.deviceDefs[i].deviceId, state); break;
            case 0x0300: QueueEvent('N', m_pmDevSrc.deviceDefs[i].deviceId, state); break;
            }
         }
      }

      // D: PinMAME Segment display state, DMD frame identification Id implemented above, and internal PUP D0 startup event implemented in PUP)
      // TODO implement PinMAME segment display state event

      // E: B2S Controller generic input state (B2SSetData / B2SPulseData)
      // Nothing to do: directly handled through state change callback

      // B: B2S Controller score digit
      // TODO implement

      // C: B2S Controller score
      // TODO implement
   }
}

}
