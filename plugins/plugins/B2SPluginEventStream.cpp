// license:GPLv3+

#include "B2SPluginEventStream.h"

#include "pinmame/PinMAMEPlugin.h"

#include <cmath>
#include <cstring>
#include <string>
using std::string;
using namespace std::string_literals;
using namespace std::string_view_literals;

using std::vector;

B2SPluginEventStream::B2SPluginEventStream(const MsgPluginAPI* msgApi, uint32_t endpointId, const std::function<void(char, int, int)>& eventHandler)
   : m_endpointId(endpointId) 
   , m_msgApi(msgApi)
   , m_eventHandler(eventHandler)
   , m_onControllersChangedId(msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_CONTROLLERS_ON_CHG_MSG))
   , m_getControllersId(msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_CONTROLLERS_GET_MSG))
   , m_getSegSrcId(m_msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_SEG_GET_SRC_MSG))
   , m_onSegSrcChangedId(m_msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_SEG_ON_SRC_CHG_MSG))
   , m_getStateSrcId(m_msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_STATE_GET_SRC_MSG))
   , m_onStateSrcChangedId(m_msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_STATE_ON_SRC_CHG_MSG))
   , m_getDmdSrcId(m_msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DISPLAY_GET_SRC_MSG))
   , m_onDmdSrcChangedId(m_msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DISPLAY_ON_SRC_CHG_MSG))
   , m_onSerumTriggerId(m_msgApi->GetMsgID("Serum", "OnDmdTrigger"))
   , m_onB2SStateChangeId(m_msgApi->GetMsgID("B2S", "OnStateChange"))
{
   m_msgApi->SubscribeMsg(m_endpointId, m_onControllersChangedId, OnControllersChanged, this);
   m_msgApi->SubscribeMsg(m_endpointId, m_onSegSrcChangedId, OnSegSrcChanged, this);
   m_msgApi->SubscribeMsg(m_endpointId, m_onStateSrcChangedId, OnStateSrcChanged, this);
   m_msgApi->SubscribeMsg(m_endpointId, m_onDmdSrcChangedId, OnDMDSrcChanged, this);
   m_msgApi->SubscribeMsg(m_endpointId, m_onSerumTriggerId, OnSerumTrigger, this);
   m_msgApi->SubscribeMsg(m_endpointId, m_onB2SStateChangeId, OnB2SStateChange, this);
   OnControllersChanged(m_onDmdSrcChangedId, this, nullptr);
   OnSegSrcChanged(m_onSegSrcChangedId, this, nullptr);
   OnStateSrcChanged(m_onStateSrcChangedId, this, nullptr);
   OnDMDSrcChanged(m_onDmdSrcChangedId, this, nullptr);

   m_thread = std::thread(&B2SPluginEventStream::StatePollingThread, this);
}

B2SPluginEventStream::~B2SPluginEventStream()
{
   m_isRunning = false;
   if (m_thread.joinable())
      m_thread.join();

   m_msgApi->UnsubscribeMsg(m_onControllersChangedId, OnControllersChanged, this);
   m_msgApi->UnsubscribeMsg(m_onSegSrcChangedId, OnSegSrcChanged, this);
   m_msgApi->UnsubscribeMsg(m_onStateSrcChangedId, OnStateSrcChanged, this);
   m_msgApi->UnsubscribeMsg(m_onDmdSrcChangedId, OnDMDSrcChanged, this);
   m_msgApi->UnsubscribeMsg(m_onSerumTriggerId, OnSerumTrigger, this);
   m_msgApi->UnsubscribeMsg(m_onB2SStateChangeId, OnB2SStateChange, this);
   delete[] m_b2sStateSrc.stateDefs;
   delete[] m_pmStateSrc.stateDefs;

   m_msgApi->ReleaseMsgID(m_onB2SStateChangeId);

   m_msgApi->ReleaseMsgID(m_onSerumTriggerId);

   m_msgApi->ReleaseMsgID(m_getStateSrcId);
   m_msgApi->ReleaseMsgID(m_onStateSrcChangedId);

   m_msgApi->ReleaseMsgID(m_getSegSrcId);
   m_msgApi->ReleaseMsgID(m_onSegSrcChangedId);

   m_msgApi->ReleaseMsgID(m_getDmdSrcId);
   m_msgApi->ReleaseMsgID(m_onDmdSrcChangedId);

   m_msgApi->ReleaseMsgID(m_getControllersId);
   m_msgApi->ReleaseMsgID(m_onControllersChangedId);
}

void B2SPluginEventStream::SetDMDHandler(const std::function<DisplaySrcId(const GetDisplaySrcMsg&)>& select, const std::function<int(const DisplaySrcId&, const uint8_t*)>& process)
{
   m_selectDmd = select;
   m_processDmd = process;
   OnDMDSrcChanged(m_onDmdSrcChangedId, this, nullptr);
}

void B2SPluginEventStream::OnControllersChanged(const unsigned int eventId, void* userData, void* eventData)
{
   auto me = static_cast<B2SPluginEventStream*>(userData);

   // Search for PinMAME and B2S controllers
   unsigned int pinmameEndPoint = 0;
   unsigned int b2sEndPoint = 0;
   GetControllersMsg getControllersMsg = { 0, 0, nullptr };
   me->m_msgApi->BroadcastMsg(me->m_endpointId, me->m_getControllersId, &getControllersMsg);
   if (getControllersMsg.count > 0)
   {
      const string pinmamePrefix(PMPI_GAMEID_PREFIX);
      const string b2sPrefix("b2s::");
      vector<ControllerDef> controllers(getControllersMsg.count);
      getControllersMsg = { getControllersMsg.count, 0, controllers.data() };
      me->m_msgApi->BroadcastMsg(me->m_endpointId, me->m_getControllersId, &getControllersMsg);
      for (const auto& controller : controllers)
      {
         string gameId = controller.gameId;
         if (gameId.starts_with(pinmamePrefix))
            pinmameEndPoint = controller.ctrlEndpointId;
         else if (gameId.starts_with(b2sPrefix))
            b2sEndPoint = controller.ctrlEndpointId;
         if (pinmameEndPoint != 0 && b2sEndPoint != 0)
            break;
      }
   }

   if (me->m_pinmameEndPoint == pinmameEndPoint && me->m_b2sEndPoint == b2sEndPoint)
      return;
   
   const bool pmChanged = me->m_pinmameEndPoint != pinmameEndPoint;
   me->m_pinmameEndPoint = pinmameEndPoint;
   me->m_b2sEndPoint = b2sEndPoint;
   if (pmChanged)
      OnSegSrcChanged(me->m_onSegSrcChangedId, me, nullptr);
   OnStateSrcChanged(me->m_onStateSrcChangedId, me, nullptr);
}

void B2SPluginEventStream::OnDMDSrcChanged(const unsigned int eventId, void* userData, void* eventData)
{
   auto me = static_cast<B2SPluginEventStream*>(userData);
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

void B2SPluginEventStream::OnSegSrcChanged(const unsigned int eventId, void* userData, void* eventData)
{
   auto me = static_cast<B2SPluginEventStream*>(userData);
   std::lock_guard lock(me->m_pollSrcMutex);
   
   me->m_pmSegSrc.clear();
   me->m_pmLastSegFrame.clear();
   me->m_pmLastSegFrameId.clear();
   if (me->m_pinmameEndPoint)
   {
      me->m_pmSegSrc.resize(1024);
      GetSegSrcMsg getSrcMsg = { static_cast<unsigned int>(me->m_pmSegSrc.size()), 0, me->m_pmSegSrc.data() };
      me->m_msgApi->SendMsg(me->m_endpointId, me->m_getSegSrcId, me->m_pinmameEndPoint, &getSrcMsg);
      me->m_pmSegSrc.resize(getSrcMsg.count);
      me->m_pmLastSegFrame.resize(getSrcMsg.count);
      me->m_pmLastSegFrameId.resize(getSrcMsg.count);
   }
}

void B2SPluginEventStream::OnStateSrcChanged(const unsigned int eventId, void* userData, void* eventData)
{
   auto me = static_cast<B2SPluginEventStream*>(userData);
   std::unique_lock lock(me->m_pollSrcMutex);

   // PinMAME controller
   delete[] me->m_pmStateSrc.stateDefs;
   memset(&me->m_pmStateSrc, 0, sizeof(me->m_pmStateSrc));
   me->m_pmStates.clear();
   if (me->m_pinmameEndPoint)
   {
      GetStateSrcMsg getSrcMsg = { 1, 0, &me->m_pmStateSrc };
      me->m_msgApi->SendMsg(me->m_endpointId, me->m_getStateSrcId, me->m_pinmameEndPoint, &getSrcMsg);
      if (getSrcMsg.count && me->m_pmStateSrc.stateDefs)
      {
         // Copy device definitions
         StateDef* devices = new StateDef[me->m_pmStateSrc.nStates];
         memcpy(devices, me->m_pmStateSrc.stateDefs, me->m_pmStateSrc.nStates * sizeof(StateDef));
         me->m_pmStateSrc.stateDefs = devices;
         me->m_pmStates.resize(me->m_pmStateSrc.nStates, -1);
      }
   }

   // B2S controller
   delete[] me->m_b2sStateSrc.stateDefs;
   memset(&me->m_b2sStateSrc, 0, sizeof(me->m_b2sStateSrc));
   if (me->m_b2sEndPoint)
   {
      GetStateSrcMsg getSrcMsg = { 1, 0, &me->m_b2sStateSrc };
      me->m_msgApi->SendMsg(me->m_endpointId, me->m_getStateSrcId, me->m_b2sEndPoint, &getSrcMsg);
      if (getSrcMsg.count && me->m_b2sStateSrc.stateDefs)
      {
         // Copy device definitions
         StateDef* devices = new StateDef[me->m_b2sStateSrc.nStates];
         memcpy(devices, me->m_b2sStateSrc.stateDefs, me->m_b2sStateSrc.nStates * sizeof(StateDef));
         me->m_b2sStateSrc.stateDefs = devices;
      }
   }

   lock.unlock();
}

// Broadcasted by B2S controllers when internal game states are changed (active player, scores, ...)
void B2SPluginEventStream::OnB2SStateChange(const unsigned int eventId, void* userData, void* eventData)
{
   auto me = static_cast<B2SPluginEventStream*>(userData);
   struct B2SPluginEvent
   {
      uint8_t type;
      int32_t index;
      int32_t value;
   };
   B2SPluginEvent* event = static_cast<B2SPluginEvent*>(eventData);
   me->QueueEvent(event->type, event->index, event->value);
}

// Broadcasted by Serum plugin when frame triggers are identified
void B2SPluginEventStream::OnSerumTrigger(const unsigned int eventId, void* userData, void* eventData)
{
   auto me = static_cast<B2SPluginEventStream*>(userData);
   auto trigger = static_cast<unsigned int*>(eventData);
   me->QueueEvent('D', static_cast<int>(*trigger), 1);
   me->QueueEvent('D', static_cast<int>(*trigger), 0);
}

// Update thread that poll analog sources (lamps, solenoids, ...) at 60Hz
void B2SPluginEventStream::StatePollingThread()
{
   //SetThreadName("B2SPluginEventStream.StatePollThread"s);
   while (m_isRunning)
   {
      std::this_thread::sleep_for(std::chrono::microseconds(16666));
      if (!m_isRunning)
         break;

      std::lock_guard lock(m_pollSrcMutex);

      // D: DMD frame identification
      if (m_dmdId.id.id != 0)
      {
         DisplayFrame dmdFrame = m_dmdId.GetIdentifyFrame(m_dmdId.id);
         if (dmdFrame.frame && dmdFrame.frameId != m_lastDmdFrameId)
         {
            m_lastDmdFrameId = dmdFrame.frameId;
            const int dmdTrigger = m_processDmd(m_dmdId, static_cast<const uint8_t*>(dmdFrame.frame));
            if (dmdTrigger > 0)
            {
               QueueEvent('D', dmdTrigger, 1);
               QueueEvent('D', dmdTrigger, 0);
            }
         }
      }
      
      // D: PinMAME Segment display state
      int segIndex = 0;
      int segDisplayIndex = 0;
      for (const auto& segSrc : m_pmSegSrc)
      {
         if (const SegDisplayFrame segFrame = segSrc.GetState(segSrc.id); segFrame.frameId != m_pmLastSegFrameId[segIndex])
         {
            m_pmLastSegFrameId[segIndex] = segFrame.frameId;
            for (unsigned int i = 0; i < segSrc.nElements; i++)
            {
               uint16_t elementState = 0;
               for (int j = 0; j < 16; j++)
                  if (segFrame.frame[j] > 0.5f)
                     elementState |= 1u << j;
               if (elementState != m_pmLastSegFrame[segDisplayIndex])
               {
                  m_pmLastSegFrame[segDisplayIndex] = elementState;
                  QueueEvent('D', segDisplayIndex, elementState);
               }
               segDisplayIndex++;
            }
         }
         else
         {
            segDisplayIndex += segSrc.nElements;
         }
         segIndex++;
      }

      // We are recreating B2S COM behavior: rely on VPinMAME changed states and broadcast them as is to plugins
      // (B2S itself does further processing to handle modulated outputs but the byte/int32 state is broadcasted to plugin without this processing)
      for (unsigned int i = 0; i < m_pmStateSrc.nStates; i++)
      {
         int state = 0;
         if (m_pmStateSrc.stateDefs[i].typeMask & CTLPI_STATE_TYPE_UINT8)
         {
            uint8_t byteState = 0;
            m_pmStateSrc.GetState(i, CTLPI_STATE_TYPE_UINT8, &byteState);
            state = static_cast<int>(byteState);
         }
         else if (m_pmStateSrc.stateDefs[i].typeMask & CTLPI_STATE_TYPE_INT32) // For PinMAME Mechs
         {
            int32_t int32State = 0;
            m_pmStateSrc.GetState(i, CTLPI_STATE_TYPE_INT32, &int32State);
            state = static_cast<int>(int32State);
         }
         // B2S convert switch values to 0/1
         if ((m_pmStateSrc.stateDefs[i].id.groupId & PMPI_GROUP_MASK) == PMPI_GROUP_SWITCH)
            state = state != 0 ? 1 : 0;
         if (m_pmStates[i] != state)
         {
            m_pmStates[i] = state;
            switch (m_pmStateSrc.stateDefs[i].id.groupId & PMPI_GROUP_MASK)
            {
            case PMPI_GROUP_SOLENOID: QueueEvent('S', m_pmStateSrc.stateDefs[i].id.stateId, state); break;
            case PMPI_GROUP_GI: QueueEvent('G', m_pmStateSrc.stateDefs[i].id.stateId, state); break;
            case PMPI_GROUP_LAMP: QueueEvent('L', m_pmStateSrc.stateDefs[i].id.stateId, state); break;
            case PMPI_GROUP_MECH: QueueEvent('N', m_pmStateSrc.stateDefs[i].id.stateId, state); break;
            case PMPI_GROUP_SWITCH: QueueEvent('W', m_pmStateSrc.stateDefs[i].id.stateId, state); break;
            }
         }
      }
   }
}
