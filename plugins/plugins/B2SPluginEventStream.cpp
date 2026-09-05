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
   , m_getSegSrcId(m_msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_SEG_GET_SRC_MSG))
   , m_onSegSrcChangedId(m_msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_SEG_ON_SRC_CHG_MSG))
   , m_getDmdSrcId(m_msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DISPLAY_GET_SRC_MSG))
   , m_onDmdSrcChangedId(m_msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DISPLAY_ON_SRC_CHG_MSG))
   , m_onSerumTriggerId(m_msgApi->GetMsgID("Serum", "OnDmdTrigger:1"))
   , m_onB2SStateChangeId(m_msgApi->GetMsgID("B2S", "OnStateChange:1"))
   , m_controllers(
        msgApi, endpointId, CTLPI_CONTROLLERS_GET_MSG, CTLPI_CONTROLLERS_ON_CHG_MSG,
        [this](std::vector<ControllerDef>& controllers)
        {
           const string pinmamePrefix(PMPI_GAMEID_PREFIX);
           const string b2sPrefix("b2s::");
           std::erase_if(controllers,
              [this, &pinmamePrefix, &b2sPrefix](const ControllerDef& controller)
              {
                 const std::string_view gameId = controller.gameId;
                 return !gameId.starts_with(pinmamePrefix) && !gameId.starts_with(b2sPrefix);
              });
        },
        [this]() {
           if (m_stateSources.IsSubscribed())
              m_stateSources.Unsubscribe();
        },
        [this]()
        {
           m_controllers.With(
              [this](const std::vector<ControllerDef>& controllers)
              {
                 m_b2sEndPoint = 0;
                 m_pinmameEndPoint = 0;
                 const string pinmamePrefix(PMPI_GAMEID_PREFIX);
                 const string b2sPrefix("b2s::");
                 for (const auto& controller : controllers)
                 {
                    string gameId = controller.gameId;
                    if (m_pinmameEndPoint == 0 && gameId.starts_with(pinmamePrefix))
                       m_pinmameEndPoint = controller.endpointId;
                    else if (m_b2sEndPoint == 0 && gameId.starts_with(b2sPrefix))
                       m_b2sEndPoint = controller.endpointId;
                    if (m_pinmameEndPoint != 0 && m_b2sEndPoint != 0)
                       break;
                 }
              });
           OnSegSrcChanged(m_onSegSrcChangedId, this, nullptr);
           if (m_pinmameEndPoint != 0)
              m_stateSources.Subscribe();
        })
   , m_stateSources(
        msgApi, endpointId, CTLPI_STATE_GET_SRC_MSG, CTLPI_STATE_ON_SRC_CHG_MSG,
        [this](std::vector<StateSrcId>& stateSources) { std::erase_if(stateSources, [this](const StateSrcId& src) { return src.id.endpointId != m_pinmameEndPoint; }); }, //
        nullptr, // onItemsAboutToChange
        [this]()
        {
           for (auto& buffer : m_pmStates)
              buffer.clear();
        })
   {
   m_msgApi->SubscribeMsg(m_endpointId, m_onSegSrcChangedId, OnSegSrcChanged, this);
   m_msgApi->SubscribeMsg(m_endpointId, m_onDmdSrcChangedId, OnDMDSrcChanged, this);
   m_msgApi->SubscribeMsg(m_endpointId, m_onSerumTriggerId, OnSerumTrigger, this);
   m_msgApi->SubscribeMsg(m_endpointId, m_onB2SStateChangeId, OnB2SStateChange, this);
   OnSegSrcChanged(m_onSegSrcChangedId, this, nullptr);
   OnDMDSrcChanged(m_onDmdSrcChangedId, this, nullptr);
   m_controllers.Subscribe();

   m_thread = std::thread(&B2SPluginEventStream::StatePollingThread, this);
}

B2SPluginEventStream::~B2SPluginEventStream()
{
   m_isRunning = false;
   if (m_thread.joinable())
      m_thread.join();

   m_controllers.Unsubscribe();
   assert(!m_stateSources.IsSubscribed());

   m_msgApi->UnsubscribeMsg(m_onSegSrcChangedId, OnSegSrcChanged, this);
   m_msgApi->UnsubscribeMsg(m_onDmdSrcChangedId, OnDMDSrcChanged, this);
   m_msgApi->UnsubscribeMsg(m_onSerumTriggerId, OnSerumTrigger, this);
   m_msgApi->UnsubscribeMsg(m_onB2SStateChangeId, OnB2SStateChange, this);

   m_msgApi->ReleaseMsgID(m_onB2SStateChangeId);

   m_msgApi->ReleaseMsgID(m_onSerumTriggerId);

   m_msgApi->ReleaseMsgID(m_getSegSrcId);
   m_msgApi->ReleaseMsgID(m_onSegSrcChangedId);

   m_msgApi->ReleaseMsgID(m_getDmdSrcId);
   m_msgApi->ReleaseMsgID(m_onDmdSrcChangedId);
}

void B2SPluginEventStream::SetDMDHandler(const std::function<DisplaySrcId(const GetDisplaySrcMsg&)>& select, const std::function<int(const DisplaySrcId&, const uint8_t*)>& process)
{
   m_selectDmd = select;
   m_processDmd = process;
   OnDMDSrcChanged(m_onDmdSrcChangedId, this, nullptr);
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
      me->m_pmSegSrc.resize(std::min(getSrcMsg.count, static_cast<unsigned int>(me->m_pmSegSrc.size())));
      me->m_pmLastSegFrameId.resize(me->m_pmSegSrc.size());
      size_t nElements = 0;
      for (const auto& segSrc : me->m_pmSegSrc)
         nElements += segSrc.nElements;
      me->m_pmLastSegFrame.resize(nElements);
   }
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
         DisplayFrame dmdFrame = m_dmdId.GetIdentifyFrame(m_dmdId.callContext);
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
         if (const SegDisplayFrame segFrame = segSrc.GetState(segSrc.callContext); segFrame.frameId != m_pmLastSegFrameId[segIndex])
         {
            m_pmLastSegFrameId[segIndex] = segFrame.frameId;
            for (unsigned int i = 0; i < segSrc.nElements; i++)
            {
               uint16_t elementState = 0;
               for (int j = 0; j < 16; j++)
                  if (segFrame.frame[i * 16 + j] > 0.5f)
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
      m_stateSources.With([&](const std::vector<StateSrcId>& stateSources) {
         for (const StateSrcId& src : stateSources)
         {
            int bufferIndex = 0;
            char eventType = 0;
            switch (src.id.resId)
            {
            case PMPI_GROUP_VPM_SOLENOID:
               eventType = 'S';
               bufferIndex = 0;
               break;
            case PMPI_GROUP_VPM_GI:
               eventType = 'G';
               bufferIndex = 1;
               break;
            case PMPI_GROUP_VPM_LAMP:
               eventType = 'L';
               bufferIndex = 2;
               break;
            case PMPI_GROUP_VPM_MECH:
               eventType = 'N';
               bufferIndex = 3;
               break;
            case PMPI_GROUP_SWITCH:
               eventType = 'W';
               bufferIndex = 4;
               break;
            default: continue;
            }
            if (m_pmStates[bufferIndex].size() < src.nStates)
               m_pmStates[bufferIndex].resize(src.nStates, -1);
            for (unsigned int i = 0; i < src.nStates; i++)
            {
               int state = 0;
               auto& def = src.stateDefs[i];
               if (src.stateDefs[i].dataFormat == CTLPI_STATE_FORMAT_UINT8 && src.stateDefs[i].GetState != nullptr)
               {
                  uint8_t byteState = 0;
                  def.GetState(def.callContext, &byteState);
                  state = static_cast<int>(byteState);
               }
               else if (src.stateDefs[i].dataFormat == CTLPI_STATE_FORMAT_INT32 && src.stateDefs[i].GetState != nullptr) // For PinMAME Mechs
               {
                  int32_t int32State = 0;
                  def.GetState(def.callContext, &int32State);
                  state = static_cast<int>(int32State);
               }
               else
               {
                  continue;
               }
               if (src.id.resId == PMPI_GROUP_SWITCH) // B2S convert switch values to 0/1
                  state = state != 0 ? 1 : 0;
               if (m_pmStates[bufferIndex][i] != state)
               {
                  m_pmStates[bufferIndex][i] = state;
                  QueueEvent(eventType, src.stateDefs[i].mappingId, state);
               }
            }
         }
      });
   }
}
