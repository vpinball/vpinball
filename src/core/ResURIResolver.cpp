// license:GPLv3+

//#include "core/stdafx.h"

#define MINIMAL_DEF_H
#include "def.h"

#include "ResURIResolver.h"

#include <sstream>
using std::string;
using namespace std::string_literals;

#include "simple-uri-parser/uri_parser.h"

ResURIResolver::ResURIResolver(const MsgPluginAPI& msgAPI, unsigned int endpointId, bool trackDisplays, bool trackSegDisplays, bool trackInputs, bool trackDevices)
   : m_msgAPI(msgAPI)
   , m_endpointId(endpointId)
   , m_getDevSrcMsgId(trackDevices ? m_msgAPI.GetMsgID(CTLPI_NAMESPACE, CTLPI_DEVICE_GET_SRC_MSG) : 0)
   , m_onDevChangedMsgId(trackDevices ? m_msgAPI.GetMsgID(CTLPI_NAMESPACE, CTLPI_DEVICE_ON_SRC_CHG_MSG) : 0)
   , m_getInputSrcMsgId(trackInputs ? m_msgAPI.GetMsgID(CTLPI_NAMESPACE, CTLPI_INPUT_GET_SRC_MSG) : 0)
   , m_onInputChangedMsgId(trackInputs ? m_msgAPI.GetMsgID(CTLPI_NAMESPACE, CTLPI_INPUT_ON_SRC_CHG_MSG) : 0)
   , m_getSegSrcMsgId(trackSegDisplays ? m_msgAPI.GetMsgID(CTLPI_NAMESPACE, CTLPI_SEG_GET_SRC_MSG) : 0)
   , m_onSegChangedMsgId(trackSegDisplays ? m_msgAPI.GetMsgID(CTLPI_NAMESPACE, CTLPI_SEG_ON_SRC_CHG_MSG) : 0)
   , m_getDisplaySrcMsgId(trackDisplays ? m_msgAPI.GetMsgID(CTLPI_NAMESPACE, CTLPI_DISPLAY_GET_SRC_MSG) : 0)
   , m_onDisplayChangedMsgId(trackDisplays ? m_msgAPI.GetMsgID(CTLPI_NAMESPACE, CTLPI_DISPLAY_ON_SRC_CHG_MSG) : 0)
{
   if (trackDisplays)
   {
      m_msgAPI.SubscribeMsg(m_endpointId, m_onDisplayChangedMsgId, OnDisplaySrcChanged, this);
      OnDisplaySrcChanged(m_onDisplayChangedMsgId, this, nullptr);
   }
   if (trackSegDisplays)
   {
      m_msgAPI.SubscribeMsg(m_endpointId, m_onSegChangedMsgId, OnSegSrcChanged, this);
      OnSegSrcChanged(m_onSegChangedMsgId, this, nullptr);
   }
   if (trackInputs)
   {
      m_msgAPI.SubscribeMsg(m_endpointId, m_onInputChangedMsgId, OnInputSrcChanged, this);
      OnInputSrcChanged(m_onInputChangedMsgId, this, nullptr);
   }
   if (trackDevices)
   {
      m_msgAPI.SubscribeMsg(m_endpointId, m_onDevChangedMsgId, OnDevSrcChanged, this);
      OnDevSrcChanged(m_onDevChangedMsgId, this, nullptr);
   }
}

ResURIResolver::~ResURIResolver()
{
   if (m_onInputChangedMsgId)
   {
      m_msgAPI.UnsubscribeMsg(m_onInputChangedMsgId, OnInputSrcChanged);
      m_msgAPI.ReleaseMsgID(m_onInputChangedMsgId);
      m_msgAPI.ReleaseMsgID(m_getInputSrcMsgId);
   }
   if (m_onDevChangedMsgId)
   {
      m_msgAPI.UnsubscribeMsg(m_onDevChangedMsgId, OnDevSrcChanged);
      m_msgAPI.ReleaseMsgID(m_onDevChangedMsgId);
      m_msgAPI.ReleaseMsgID(m_getDevSrcMsgId);
   }
   if (m_onSegChangedMsgId)
   {
      m_msgAPI.UnsubscribeMsg(m_onSegChangedMsgId, OnSegSrcChanged);
      m_msgAPI.ReleaseMsgID(m_onSegChangedMsgId);
      m_msgAPI.ReleaseMsgID(m_getSegSrcMsgId);
   }
   if (m_onDisplayChangedMsgId)
   {
      m_msgAPI.UnsubscribeMsg(m_onDisplayChangedMsgId, OnDisplaySrcChanged);
      m_msgAPI.ReleaseMsgID(m_onDisplayChangedMsgId);
      m_msgAPI.ReleaseMsgID(m_getDisplaySrcMsgId);
   }
}

void ResURIResolver::OnInputSrcChanged(const unsigned int msgId, void *userData, void *msgData)
{
   ResURIResolver* me = static_cast<ResURIResolver *>(userData);
   GetInputSrcMsg getSrcMsg = { 0, 0, nullptr };
   me->m_msgAPI.BroadcastMsg(me->m_endpointId, me->m_getInputSrcMsgId, &getSrcMsg);
   me->m_inputSources.clear();
   me->m_inputSources.resize(getSrcMsg.count);
   getSrcMsg = { getSrcMsg.count, 0, me->m_inputSources.data() };
   me->m_msgAPI.BroadcastMsg(me->m_endpointId, me->m_getInputSrcMsgId, &getSrcMsg);
   me->m_floatCache.clear();
}

void ResURIResolver::OnDevSrcChanged(const unsigned int msgId, void *userData, void *msgData)
{
   ResURIResolver* me = static_cast<ResURIResolver *>(userData);
   GetDevSrcMsg getSrcMsg = { 0, 0, nullptr };
   me->m_msgAPI.BroadcastMsg(me->m_endpointId, me->m_getDevSrcMsgId, &getSrcMsg);
   me->m_deviceSources.clear();
   me->m_deviceSources.resize(getSrcMsg.count);
   getSrcMsg = { getSrcMsg.count, 0, me->m_deviceSources.data() };
   me->m_msgAPI.BroadcastMsg(me->m_endpointId, me->m_getDevSrcMsgId, &getSrcMsg);
   me->m_floatCache.clear();
}

void ResURIResolver::OnSegSrcChanged(const unsigned int msgId, void *userData, void *msgData)
{
   ResURIResolver* me = static_cast<ResURIResolver *>(userData);
   GetSegSrcMsg getSrcMsg = { 0, 0, nullptr };
   me->m_msgAPI.BroadcastMsg(me->m_endpointId, me->m_getSegSrcMsgId, &getSrcMsg);
   me->m_segSources.clear();
   me->m_segSources.resize(getSrcMsg.count);
   getSrcMsg = { getSrcMsg.count, 0, me->m_segSources.data() };
   me->m_msgAPI.BroadcastMsg(me->m_endpointId, me->m_getSegSrcMsgId, &getSrcMsg);
   me->m_segCache.clear();
}

void ResURIResolver::OnDisplaySrcChanged(const unsigned int msgId, void *userData, void *msgData)
{
   ResURIResolver* me = static_cast<ResURIResolver *>(userData);
   GetDisplaySrcMsg getSrcMsg = { 0, 0, nullptr };
   me->m_msgAPI.BroadcastMsg(me->m_endpointId, me->m_getDisplaySrcMsgId, &getSrcMsg);
   me->m_displaySources.clear();
   me->m_displaySources.resize(getSrcMsg.count);
   getSrcMsg = { getSrcMsg.count, 0, me->m_displaySources.data() };
   me->m_msgAPI.BroadcastMsg(me->m_endpointId, me->m_getDisplaySrcMsgId, &getSrcMsg);
   me->m_displayCache.clear();
}

float ResURIResolver::GetFloatState(const string &link)
{
   const auto &cache = m_floatCache.find(link);
   if (cache != m_floatCache.end())
      return cache->second(link);

   floatCacheLambda lambda = nullptr;
   const auto uri = uri::parse_uri(link);
   if (uri.error != uri::Error::None)
   {
      // FIXME log PLOGE << "Invalid resource URI: " << link;
   }
   else if (uri.scheme == "ctrl")
   {
      if (uri.authority.host == "default")
      {
         // Which definitions do we want to give for this (if any) ?
      }
      else
      {
         const unsigned int plugin = m_msgAPI.GetPluginEndpoint(uri.authority.host.c_str());
         if (plugin)
         {
            int resId = 0;
            auto resIdPart = uri.query.find("id"s);
            if (resIdPart != uri.query.end())
               try_parse_int(resIdPart->second, resId);

            if (uri.path == "/device")
            {
               auto ioSource = std::ranges::find_if(m_deviceSources.begin(), m_deviceSources.end(), [plugin, resId](const DevSrcId &cd) { return cd.id.endpointId == plugin && cd.id.resId == resId; });
               if (ioSource != m_deviceSources.end())
               {
                  int ioId = 0;
                  auto ioIdPart = uri.query.find("io"s);
                  if (ioIdPart != uri.query.end())
                     try_parse_int(ioIdPart->second, ioId);
                  
                  int ioIndex = -1;
                  for (unsigned int i = 0; i < ioSource->nDevices; i++)
                     if (ioSource->deviceDefs[i].mappingId == ioId)
                        ioIndex = i;
                     
                  if (ioIndex >= 0)
                     lambda = [ioSource, ioIndex](const string &) -> float { return ioSource->GetFloatState(ioIndex); };
               }
            }
            else if (uri.path == "/input")
            {
               auto ioSource = std::ranges::find_if(m_inputSources.begin(), m_inputSources.end(), [plugin, resId](const InputSrcId &cd) { return cd.id.endpointId == plugin && cd.id.resId == resId; });
               if (ioSource != m_inputSources.end())
               {
                  int ioId = 0;
                  auto ioIdPart = uri.query.find("io"s);
                  if (ioIdPart != uri.query.end())
                     try_parse_int(ioIdPart->second, ioId);
                  
                  int ioIndex = -1;
                  for (unsigned int i = 0; i < ioSource->nInputs; i++)
                     if (ioSource->inputDefs[i].mappingId == ioId)
                        ioIndex = i;
                     
                  if (ioIndex >= 0)
                     lambda = [ioSource, ioIndex](const string &) -> float { return static_cast<float>(ioSource->GetInputState(ioIndex)); };
               }
            }
            else if (uri.path == "/display")
            {
               // TODO implement (to access individual dots, useful for small LED matrices)
            }
         }
      }
   }

   if (lambda == nullptr)
      lambda = [](const string &) -> float { return 0.f; };
   m_floatCache[link] = lambda;
   return lambda(link);
}

ResURIResolver::SegDisplayState ResURIResolver::GetSegDisplayState(const string &link)
{
   const auto &cache = m_segCache.find(link);
   if (cache != m_segCache.end())
      return cache->second(link);

   segCacheLambda lambda = nullptr;
   const auto uri = uri::parse_uri(link);
   if (uri.error != uri::Error::None)
   {
      // FIXME log PLOGE << "Invalid resource URI: " << link;
   }
   else if ((uri.scheme == "ctrl") && (uri.path == "/seg"))
   {
      SegSrcId *segSource = nullptr;
      if (uri.authority.host == "default")
      {
         if (!m_segSources.empty())
         {
            CtlResId group = m_segSources[0].groupId;
            int index = 0;
            auto indexPart = uri.query.find("id"s); // id is used as index inside selected display group (which expects plugins to report displays in a stable order, should we sort by resId first ?)
            if (indexPart != uri.query.end())
               try_parse_int(indexPart->second, index);
            for (SegSrcId& source : m_segSources)
            {
               if (source.groupId.id == group.id)
               {
                  index--;
                  if (index < 0)
                  {
                     segSource = &source;
                     break;
                  }
               }
            }
         }
      }
      else
      {
         const unsigned int plugin = m_msgAPI.GetPluginEndpoint(uri.authority.host.c_str());
         if (plugin)
         {
            int resId = 0;
            auto resIdPart = uri.query.find("id"s);
            if (resIdPart != uri.query.end())
               try_parse_int(resIdPart->second, resId);

            auto source = std::ranges::find_if(m_segSources.begin(), m_segSources.end(), 
               [plugin, resId](const SegSrcId &cd) { return cd.id.endpointId == plugin && cd.id.resId == resId; });
            if (source != m_segSources.end())
               segSource = &*source;
         }
      }
      if (segSource)
      {
         int subId = -1;
         auto subIdPart = uri.query.find("sub"s);
         if (subIdPart != uri.query.end())
            try_parse_int(subIdPart->second, subId);

         if (subId < 0)
         {
            lambda = [segSource, subIdPart](const string &) -> SegDisplayState { return { &*segSource, segSource->GetState(segSource->id) }; };
         }
         else if (subId < static_cast<int>(segSource->nElements))
         {
            SegSrcId subSegSrc = *segSource;
            subSegSrc.GetState = nullptr;
            subSegSrc.nElements = 1;
            subSegSrc.elementType[0] = segSource->elementType[subId];
            lambda = [segSource, subSegSrc, subId](const string &) -> SegDisplayState
            {
               SegDisplayFrame state = segSource->GetState(segSource->id);
               return { &subSegSrc, { state.frameId, state.frame + subId * 16 } };
            };
         }
      }
   }

   if (lambda == nullptr)
      lambda = [](const string &) -> SegDisplayState { return { nullptr, { 0, nullptr} }; };
   m_segCache[link] = lambda;
   return lambda(link);
}

ResURIResolver::DisplayState ResURIResolver::GetDisplayState(const string &link)
{
   const auto &cache = m_displayCache.find(link);
   if (cache != m_displayCache.end())
      return cache->second(link);

   displayCacheLambda lambda = nullptr;
   const auto uri = uri::parse_uri(link);
   if (uri.error != uri::Error::None)
   {
      // FIXME log PLOGE << "Invalid resource URI: " << link;
   }
   else if ((uri.scheme == "ctrl") && (uri.path == "/display"))
   {
      DisplaySrcId* displaySource = nullptr;
      if (uri.authority.host == "default")
      {
         for (auto& source : m_displaySources)
         {
            if (displaySource == nullptr                                                                            // Priority 1: Find at least a display
               || (displaySource->width < source.width)                                                             // Priority 2: Favor highest resolution display
               || (displaySource->width == source.width && displaySource->frameFormat == CTLPI_DISPLAY_FORMAT_LUM8) // Priority 3: Favor color over monochrome
               || (displaySource->width == source.width && source.frameFormat == CTLPI_DISPLAY_FORMAT_SRGB888))     // Priority 4: Favor RGB8 over other formats
               displaySource = &source;
         }
      }
      else
      {
         const unsigned int plugin = m_msgAPI.GetPluginEndpoint(uri.authority.host.c_str());
         if (plugin)
         {
            int resId = 0;
            auto resIdPart = uri.query.find("id"s);
            if (resIdPart != uri.query.end())
               try_parse_int(resIdPart->second, resId);

            auto source = std::ranges::find_if(m_displaySources.begin(), m_displaySources.end(), 
               [plugin, resId](const DisplaySrcId &cd) { return cd.id.endpointId == plugin && cd.id.resId == resId; });
            if (source != m_displaySources.end())
               displaySource = &*source;
         }
      }
      if (displaySource != nullptr)
         lambda = [displaySource](const string &) -> DisplayState { return { displaySource, displaySource->GetRenderFrame(displaySource->id)}; };
   }

   if (lambda == nullptr)
      lambda = [](const string &) -> DisplayState { return { nullptr, { 0, nullptr} }; };
   m_displayCache[link] = lambda;
   return lambda(link);
}
