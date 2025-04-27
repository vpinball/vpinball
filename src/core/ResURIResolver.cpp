// license:GPLv3+

#include "core/stdafx.h"

#include "ResURIResolver.h"
#include "VPXPluginAPIImpl.h"

#include "simple-uri-parser/uri_parser.h"

ResURIResolver::ResURIResolver()
{
   m_getDmdSrcMsgId = VPXPluginAPIImpl::GetMsgID(CTLPI_NAMESPACE, CTLPI_GETDMD_SRC_MSG);
   m_getDmdMsgId = VPXPluginAPIImpl::GetMsgID(CTLPI_NAMESPACE, CTLPI_GETDMD_RENDER_MSG);
   m_onDmdChangedMsgId = VPXPluginAPIImpl::GetMsgID(CTLPI_NAMESPACE, CTLPI_ONDMD_SRC_CHG_MSG);
   MsgPluginManager::GetInstance().GetMsgAPI().SubscribeMsg(VPXPluginAPIImpl::GetInstance().GetVPXEndPointId(), m_onDmdChangedMsgId, OnDmdSrcChanged, this);

   m_getSegSrcMsgId = VPXPluginAPIImpl::GetMsgID(CTLPI_NAMESPACE, CTLPI_GETSEG_SRC_MSG);
   m_getSegMsgId = VPXPluginAPIImpl::GetMsgID(CTLPI_NAMESPACE, CTLPI_GETSEG_MSG);
   m_onSegChangedMsgId = VPXPluginAPIImpl::GetMsgID(CTLPI_NAMESPACE, CTLPI_ONSEG_SRC_CHG_MSG);
   MsgPluginManager::GetInstance().GetMsgAPI().SubscribeMsg(VPXPluginAPIImpl::GetInstance().GetVPXEndPointId(), m_onSegChangedMsgId, OnSegSrcChanged, this);

   m_getDevSrcMsgId = VPXPluginAPIImpl::GetMsgID(CTLPI_NAMESPACE, CTLPI_GETDEV_SRC_MSG);
   m_onDevChangedMsgId = VPXPluginAPIImpl::GetMsgID(CTLPI_NAMESPACE, CTLPI_ONDEV_SRC_CHG_MSG);
   MsgPluginManager::GetInstance().GetMsgAPI().SubscribeMsg(VPXPluginAPIImpl::GetInstance().GetVPXEndPointId(), m_onDevChangedMsgId, OnDevSrcChanged, this);
}

ResURIResolver::~ResURIResolver()
{
   OnDevSrcChanged(0, this, nullptr);
   MsgPluginManager::GetInstance().GetMsgAPI().UnsubscribeMsg(m_onDevChangedMsgId, OnDevSrcChanged);
   VPXPluginAPIImpl::ReleaseMsgID(m_onDevChangedMsgId);
   VPXPluginAPIImpl::ReleaseMsgID(m_getDevSrcMsgId);

   OnSegSrcChanged(0, this, nullptr);
   MsgPluginManager::GetInstance().GetMsgAPI().UnsubscribeMsg(m_onSegChangedMsgId, OnSegSrcChanged);
   VPXPluginAPIImpl::ReleaseMsgID(m_onSegChangedMsgId);
   VPXPluginAPIImpl::ReleaseMsgID(m_getSegSrcMsgId);
   VPXPluginAPIImpl::ReleaseMsgID(m_getSegMsgId);

   OnDmdSrcChanged(0, this, nullptr);
   MsgPluginManager::GetInstance().GetMsgAPI().UnsubscribeMsg(m_onDmdChangedMsgId, OnDmdSrcChanged);
   VPXPluginAPIImpl::ReleaseMsgID(m_onDmdChangedMsgId);
   VPXPluginAPIImpl::ReleaseMsgID(m_getDmdSrcMsgId);
   VPXPluginAPIImpl::ReleaseMsgID(m_getDmdMsgId);
}

void ResURIResolver::RequestVisualUpdate()
{
   // TODO only request display update if visual timestamp has been changed
   m_visualTimestamp++;
}

void ResURIResolver::RequestPhysicsUpdate()
{
   // TODO only request physic update if physics timestamp has been changed
   m_physicsTimestamp++;
}

void ResURIResolver::OnDevSrcChanged(const unsigned int msgId, void *userData, void *msgData)
{
   ResURIResolver *me = static_cast<ResURIResolver *>(userData);
   for (IOSrcId cache : me->m_controllerDevices)
      delete cache.deviceDefs;
   me->m_controllerDevices.clear();
   me->m_outputCache.clear();
}

float ResURIResolver::GetControllerOutput(CtlResId id, const unsigned int outputId)
{
   IOSrcId *source = nullptr;

   auto existingSource = std::ranges::find_if(m_controllerDevices.begin(), m_controllerDevices.end(), [id](const IOSrcId &cd) { return cd.id.id == id.id; });
   if (existingSource != m_controllerDevices.end())
      source = &(*existingSource);
   else
   {
      GetDevSrcMsg getSrcMsg = { 1024, 0, new IOSrcId[1024] };
      VPXPluginAPIImpl::GetInstance().BroadcastVPXMsg(m_getDevSrcMsgId, &getSrcMsg);
      for (unsigned int i = 0; i < getSrcMsg.count; i++)
      {
         if (getSrcMsg.entries[i].id.id == id.id)
         {
            m_controllerDevices.push_back(getSrcMsg.entries[i]);
            source = &m_controllerDevices.back();
            *source = getSrcMsg.entries[i];
            if (source->deviceDefs)
            {
               source->deviceDefs = new DeviceDef[source->nDevices];
               memcpy(source->deviceDefs, getSrcMsg.entries[i].deviceDefs, source->nDevices * sizeof(DeviceDef));
            }
         }
      }
      delete[] getSrcMsg.entries;
      if (source == nullptr)
         return 0.f;
   }

   /* for (unsigned int i = 0; i < source->nDevices; i++)
      if (source->deviceDefs[i].mappingId == outputId)
         return source->GetState(i);*/

   return 0.f;
}

void ResURIResolver::OnSegSrcChanged(const unsigned int msgId, void *userData, void *msgData)
{
   ResURIResolver *me = static_cast<ResURIResolver *>(userData);
   for (ControllerSegDisplay &display : me->m_controllerSegDisplays)
      delete[] display.frame;
   me->m_controllerSegDisplays.clear();
   me->m_segCache.clear();
   me->m_defaultSegId = { 0 };
}

ResURIResolver::ControllerSegDisplay ResURIResolver::GetControllerSegDisplay(CtlResId id)
{
   ControllerSegDisplay* display = nullptr;
   if (id.id == 0)
   {
      if (m_defaultSegId.id == 0)
      {
         GetSegSrcMsg getSrcMsg = { 1024, 0, new SegSrcId[1024] };
         VPXPluginAPIImpl::GetInstance().BroadcastVPXMsg(m_getSegSrcMsgId, &getSrcMsg);
         if (getSrcMsg.count == 0)
         {
            delete[] getSrcMsg.entries;
            return { { 0 }, 0, nullptr };
         }
         m_defaultSegId = getSrcMsg.entries[0].id;
         delete[] getSrcMsg.entries;
      }
      id = m_defaultSegId;
   }

   auto pCD = std::ranges::find_if(m_controllerSegDisplays.begin(), m_controllerSegDisplays.end(), [id](const ControllerSegDisplay &cd) { return cd.segId.id == id.id; });
   if (pCD != m_controllerSegDisplays.end())
      display = &(*pCD);
   else
   {
      // Search for the requested display (eventually resulting in an empty entry if not found)
      GetSegSrcMsg getSrcMsg = { 1024, 0, new SegSrcId[1024] };
      VPXPluginAPIImpl::GetInstance().BroadcastVPXMsg(m_getSegSrcMsgId, &getSrcMsg);
      m_controllerSegDisplays.push_back({id, 0, nullptr});
      display = &m_controllerSegDisplays.back();
      for (unsigned int i = 0; i < getSrcMsg.count; i++)
      {
         if (getSrcMsg.entries[0].id.id == id.id)
         {
            display->displays.emplace_back(&getSrcMsg.entries[i].elementType[0], &getSrcMsg.entries[i].elementType[getSrcMsg.entries[i].nElements]);
            display->nElements += getSrcMsg.entries[i].nElements;
         }
      }
      display->frame = new float[16 * display->nElements];
      delete[] getSrcMsg.entries;
   }

   // Obtain frame from controller plugin
   GetSegMsg getMsg = { display->segId, 0, nullptr };
   VPXPluginAPIImpl::GetInstance().BroadcastVPXMsg(m_getSegMsgId, &getMsg);
   if (getMsg.frame == nullptr)
      return { display->segId, 0, nullptr };
   memcpy(display->frame, getMsg.frame, display->nElements * 16 * sizeof(float));
   return *display;
}

void ResURIResolver::OnDmdSrcChanged(const unsigned int msgId, void *userData, void *msgData)
{
   ResURIResolver * me = static_cast<ResURIResolver *>(userData);
   for (ControllerDisplay &display : me->m_controllerDisplays)
   {
      if (g_pplayer && display.frame)
      {
         BaseTexture *tex = display.frame;
         g_pplayer->m_renderer->m_renderDevice->AddEndOfFrameCmd([tex]
            {
               g_pplayer->m_renderer->m_renderDevice->m_texMan.UnloadTexture(tex);
               delete tex;
            });
         display.frame = nullptr;
      }
   }
   me->m_controllerDisplays.clear();
   me->m_texCache.clear();
   me->m_defaultDmdId = { 0 };
}

ResURIResolver::ControllerDisplay ResURIResolver::GetControllerDisplay(CtlResId id)
{
   ControllerDisplay* display = nullptr;
   if (id.id == 0)
   {
      // FIXME script should be declared as other DMD and priorized during selection
      // Script DMD takes precedence over plugin DMD
      if (g_pplayer->m_dmdFrame)
         return { { 0 }, g_pplayer->m_dmdFrameId, g_pplayer->m_dmdFrame }; // FIXME 0 id is wrong here

      if (m_defaultDmdId.id.id == 0)
      {
         bool dmdFound = false;
         unsigned int largest = 128;
         GetDmdSrcMsg getSrcMsg = { 1024, 0, new DmdSrcId[1024] };
         VPXPluginAPIImpl::GetInstance().BroadcastVPXMsg(m_getDmdSrcMsgId, &getSrcMsg);
         for (unsigned int i = 0; i < getSrcMsg.count; i++)
         {
            if ((getSrcMsg.entries[i].width >= largest) // Select a large DMD
               && (m_defaultDmdId.format == 0 || getSrcMsg.entries[i].format != CTLPI_GETDMD_FORMAT_LUM8)) // Prefer color over monochrome
            {
               m_defaultDmdId = getSrcMsg.entries[i];
               largest = getSrcMsg.entries[i].width;
               dmdFound = true;
            }
         }
         delete[] getSrcMsg.entries;
         if (!dmdFound)
            return { { 0 }, -1, nullptr };
      }
      id = m_defaultDmdId.id;
   }

   // FIXME this only match on the frame source id, not on the other properties (size/format)
   auto pCD = std::ranges::find_if(m_controllerDisplays.begin(), m_controllerDisplays.end(), [id](const ControllerDisplay &cd) { return cd.dmdId.id.id == id.id; });
   if (pCD != m_controllerDisplays.end())
      display = &(*pCD);
   else
   {
      // Search for the requested DMD
      bool dmdFound = false;
      DmdSrcId dmdId = { 0 };
      GetDmdSrcMsg getSrcMsg = { 1024, 0, new DmdSrcId[1024] };
      VPXPluginAPIImpl::GetInstance().BroadcastVPXMsg(m_getDmdSrcMsgId, &getSrcMsg);
      for (unsigned int i = 0; i < getSrcMsg.count; i++)
      {
         if ((getSrcMsg.entries[i].id.id == id.id) && (dmdId.format == 0 || getSrcMsg.entries[i].format != CTLPI_GETDMD_FORMAT_LUM8)) // Prefer color over monochrome
         {
            dmdId = getSrcMsg.entries[i];
            dmdFound = true;
         }
      }
      delete[] getSrcMsg.entries;
      if (!dmdFound)
         return { { 0 }, -1, nullptr };
      m_controllerDisplays.push_back({dmdId, -1, nullptr});
      display = &m_controllerDisplays.back();
   }

   // Obtain DMD frame from controller plugin
   GetDmdMsg getMsg = { display->dmdId, 0, nullptr };
   VPXPluginAPIImpl::GetInstance().BroadcastVPXMsg(m_getDmdMsgId, &getMsg);
   if (getMsg.frame == nullptr)
      return { display->dmdId, -1, nullptr };

   // Requesting the DMD may have triggered colorization and display list to be modified, so the display pointer may be bad at this point
   pCD = std::ranges::find_if(m_controllerDisplays.begin(), m_controllerDisplays.end(), [&](const ControllerDisplay &cd) { return memcmp(&cd.dmdId, &getMsg.dmdId, sizeof(DmdSrcId)) == 0; });
   if (pCD == m_controllerDisplays.end())
      return { display->dmdId, -1, nullptr };
   else
      display = &(*pCD);

   // (re) Create DMD texture
   const BaseTexture::Format format = display->dmdId.format == CTLPI_GETDMD_FORMAT_LUM8 ? BaseTexture::BW : BaseTexture::SRGBA;
   if (display->frame == nullptr || display->frame->width() != display->dmdId.width || display->frame->height() != display->dmdId.height || display->frame->m_format != format)
   {
      // Delay texture deletion since it may be used by the render frame which is processed asynchronously. If so, deleting would cause a deadlock & invalid access
      BaseTexture *tex = display->frame;
      g_pplayer->m_renderer->m_renderDevice->AddEndOfFrameCmd([tex]
         {
            g_pplayer->m_renderer->m_renderDevice->m_texMan.UnloadTexture(tex);
            delete tex; 
         });
      display->frame = new BaseTexture(display->dmdId.width, display->dmdId.height, format);
      display->frame->SetIsOpaque(true);
      display->frameId = -1;
   }

   // Copy DMD frame, eventually converting it
   if (display->frameId != getMsg.frameId)
   {
      display->frameId = getMsg.frameId;
      const int size = display->dmdId.width * display->dmdId.height;
      if (display->dmdId.format == CTLPI_GETDMD_FORMAT_LUM8)
         memcpy(display->frame->data(), getMsg.frame, size);
      else if (display->dmdId.format == CTLPI_GETDMD_FORMAT_SRGB565)
      {
         static constexpr UINT8 lum32[] = { 0, 8, 16, 25, 33, 41, 49, 58, 66, 74, 82, 90, 99, 107, 115, 123, 132, 140, 148, 156, 165, 173, 181, 189, 197, 206, 214, 222, 230, 239, 247, 255 };
         static constexpr UINT8 lum64[] = { 0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 45, 49, 53, 57, 61, 65, 69, 73, 77, 81, 85, 89, 93, 97, 101, 105, 109, 113, 117, 121, 125, 130, 134, 138, 142, 146, 150, 154, 158, 162, 166, 170, 174, 178, 182, 186, 190, 194, 198, 202, 206, 210, 215, 219, 223, 227, 231, 235, 239, 243, 247, 251, 255 };
         DWORD *const data = reinterpret_cast<DWORD *>(display->frame->data());
         const uint16_t * const frame = reinterpret_cast<uint16_t *>(getMsg.frame);
         for (int ofs = 0; ofs < size; ofs++)
         {
            const uint16_t rgb565 = frame[ofs];
            data[ofs] = 0xFF000000 | (lum32[rgb565 & 0x1F] << 16) | (lum64[(rgb565 >> 5) & 0x3F] << 8) | lum32[(rgb565 >> 11) & 0x1F];
         }
      }
      else if (display->dmdId.format == CTLPI_GETDMD_FORMAT_SRGB888)
      {
         DWORD *const data = reinterpret_cast<DWORD *>(display->frame->data());
         for (int ofs = 0; ofs < size; ofs++)
            data[ofs] = 0xFF000000 | (getMsg.frame[ofs * 3 + 2] << 16) | (getMsg.frame[ofs * 3 + 1] << 8) | getMsg.frame[ofs * 3];
      }
      g_pplayer->m_renderer->m_renderDevice->m_texMan.SetDirty(display->frame);
   }

   return *display;
}

float ResURIResolver::GetOutput(const string &link)
{
   const auto &cache = m_outputCache.find(link);
   if (g_pplayer == nullptr) // For the time being, this API may only be used in the player
      return 0.f;
   if (cache != m_outputCache.end())
      return cache->second(link);
   if (link.empty())
      return 0.f;

   outputCacheLambda lambda = nullptr;
   const auto uri = uri::parse_uri(link);
   if (uri.error != uri::Error::None)
   {
      PLOGE << "Invalid resource URI: " << link;
   }
   else if (uri.scheme == "display")
   {
      // TODO implement pixel access
   }
   else if (uri.scheme == "plugin")
   {
      uint32_t endpointId = 0;
      auto plugin = MsgPluginManager::GetInstance().GetPlugin(uri.authority.host);
      if (plugin.get())
         endpointId = plugin->m_endpointId;
      if ((endpointId != 0) && (uri.path == "/getstate"))
      {
         auto src = uri.query.find("src"s);
         if (src != uri.query.end() && src->second == "output")
         {
            int displayId = 0;
            auto dispId = uri.query.find("id"s);
            if (dispId != uri.query.end())
               try_parse_int(dispId->second, displayId);
            int ouputId = 0;
            auto outId = uri.query.find("io"s);
            if (outId != uri.query.end())
               try_parse_int(outId->second, ouputId);
            // TODO implement property access (luminance, tint, ...)
            lambda = [this, endpointId, displayId, ouputId](const string &) -> float { return GetControllerOutput({ endpointId, static_cast<uint32_t>(displayId) }, ouputId); };
         }
      }
   }

   if (lambda == nullptr)
      lambda = [](const string &) -> float { return 0.f; };
   m_outputCache[link] = lambda;
   return lambda(link);
}

BaseTexture *ResURIResolver::GetDisplay(const string &link)
{
   const auto &cache = m_texCache.find(link);
   if (g_pplayer == nullptr) // For the time being, this API may only be used in the player
      return nullptr;
   if (cache != m_texCache.end())
      return cache->second(link);
   if (link.empty())
      return nullptr;

   texCacheLambda lambda = nullptr;
   const auto uri = uri::parse_uri(link);
   if (uri.error != uri::Error::None)
   {
      PLOGE << "Invalid resource URI: " << link;
   }
   else if (uri.scheme == "default")
   {
      if (uri.path == "/dmd")
      {
         lambda = [this](const string &) -> BaseTexture * { return GetControllerDisplay({ 0, 0 }).frame; };
      }
   }
   else if (uri.scheme == "script")
   {
      if (uri.path == "/dmd")
      {
         auto src = uri.query.find("src"s);
         if (src != uri.query.end())
         {
            if (strcmp(src->second.c_str(), g_pplayer->m_ptable->GetElementName(g_pplayer->m_ptable)) == 0)
            {
               lambda = [](const string &) -> BaseTexture * { return g_pplayer->m_dmdFrame; };
            }
            else
            {
               IEditable *edit = g_pplayer->m_ptable->GetElementByName(src->second.c_str());
               if (edit && edit->GetItemType() == eItemFlasher)
               {
                  lambda = [edit](const string &) -> BaseTexture * { return static_cast<const Flasher *>(edit)->m_dmdFrame; };
               }
               else
               {
                  PLOGE << "Invalid resource URI: " << link;
               }
            }
         }
         else
         {
            lambda = [](const string &) -> BaseTexture * { return g_pplayer->m_dmdFrame; };
         }
      }
   }
   else if (uri.scheme == "plugin")
   {
      uint32_t endpointId = 0;
      auto plugin = MsgPluginManager::GetInstance().GetPlugin(uri.authority.host);
      if (plugin.get())
         endpointId = plugin->m_endpointId;
      if ((endpointId != 0) && (uri.path == "/getstate"))
      {
         auto src = uri.query.find("src"s);
         if (src != uri.query.end() && src->second == "display")
         {
            int displayId = 0;
            auto dispId = uri.query.find("id"s);
            if (dispId != uri.query.end())
               try_parse_int(dispId->second, displayId);
            lambda = [this, endpointId, displayId](const string &) -> BaseTexture * { return GetControllerDisplay({ endpointId, static_cast<uint32_t>(displayId) }).frame; };
         }
      }
   }

   if (lambda == nullptr)
      lambda = [](const string &) -> BaseTexture * { return nullptr; };
   m_texCache[link] = lambda;
   return lambda(link);
}

ResURIResolver::SegDisplay ResURIResolver::GetSegDisplay(const string &link)
{
   const auto &cache = m_segCache.find(link);
   if (cache != m_segCache.end())
      return cache->second(link);
   if (link.empty())
      return { nullptr };

   segCacheLambda lambda = nullptr;
   const auto uri = uri::parse_uri(link);
   if (uri.error != uri::Error::None)
   {
      PLOGE << "Invalid resource URI: " << link;
   }
   else if (uri.scheme == "default")
   {
      if (uri.path == "/alpha")
      {
         int subId = -1;
         auto sub = uri.query.find("sub"s);
         if (sub != uri.query.end())
            try_parse_int(sub->second.c_str(), subId);
         if (subId >= 0)
         {
            int eltId = -1;
            auto elt = uri.query.find("elt"s);
            if (elt != uri.query.end())
               try_parse_int(elt->second.c_str(), eltId);
            if (eltId >= 0)
            {
               // Single element of a display
               lambda = [this, subId, eltId](const string &) -> SegDisplay
               {
                  ResURIResolver::ControllerSegDisplay display = GetControllerSegDisplay({ 0, 0 });
                  if ((display.frame == nullptr) || (subId >= (int)display.displays.size()))
                     return { nullptr };
                  float *data = display.frame;
                  for (int i = 0; i < subId; i++)
                     data += display.displays[i].size() * 16;
                  if (eltId >= (int)display.displays[subId].size())
                     return { nullptr };
                  data += eltId * 16;
                  return { data, { display.displays[subId][eltId] } };
               };
            }
            else
            {
               // Display, made up of a group of elements
               lambda = [this, subId](const string &) -> SegDisplay
               {
                  ResURIResolver::ControllerSegDisplay display = GetControllerSegDisplay({ 0, 0 });
                  if ((display.frame == nullptr) || (subId >= (int)display.displays.size()))
                     return { nullptr };
                  float *data = display.frame;
                  for (int i = 0; i < subId; i++)
                     data += display.displays[i].size() * 16;
                  return { data, display.displays[subId] };
               };
            }
         }
      }
   }
   else if (uri.scheme == "script")
   {

   }
   else if (uri.scheme == "plugin")
   {
      uint32_t endpointId = 0;
      auto plugin = MsgPluginManager::GetInstance().GetPlugin(uri.authority.host);
      if (plugin.get())
         endpointId = plugin->m_endpointId;
      if ((endpointId != 0)
         && (uri.path == "/getstate")
         && (std::ranges::find_if(uri.query.begin(), uri.query.end(), [](const auto &a) { return a.first == "src"; }) != uri.query.end())
         && (uri.query.at("src"s) == "alpha"))
      {
         int displayId = -1;
         if (std::ranges::find_if(uri.query.begin(), uri.query.end(), [](const auto &a) { return a.first == "id"; }) != uri.query.end())
            try_parse_int(uri.query.at("id"s), displayId);
         int subId = -1;
         if (std::ranges::find_if(uri.query.begin(), uri.query.end(), [](const auto &a) { return a.first == "sub"; }) != uri.query.end())
            try_parse_int(uri.query.at("sub"s), subId);
         if (subId >= 0)
            lambda = [this, endpointId, displayId, subId](const string &) -> SegDisplay
            {
               ResURIResolver::ControllerSegDisplay display = GetControllerSegDisplay({ endpointId, static_cast<uint32_t>(displayId) });
               if ((display.frame == nullptr) || (subId >= (int)display.displays.size()))
                  return { nullptr };
               float *data = display.frame;
               for (int i = 0; i < subId; i++)
                  data += display.displays[i].size() * 16;
               return { data, display.displays[subId] };
            };
      }
   }

   if (lambda == nullptr)
      lambda = [](const string &) -> SegDisplay { return { nullptr }; };
   m_segCache[link] = lambda;
   return lambda(link);
}
