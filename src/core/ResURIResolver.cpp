// license:GPLv3+

#include "core/stdafx.h"
#include "ResURIResolver.h"
#include "simple-uri-parser/uri_parser.h"

ResURIResolver::ResURIResolver()
{
}

void ResURIResolver::ClearCache()
{
   m_texCache.clear();
   m_segCache.clear();
}

BaseTexture *ResURIResolver::GetDisplay(const string &link, const IEditable *context)
{
   const auto &cache = m_texCache.find(link);
   if (cache != m_texCache.end())
      return cache->second(link, context);
   if (link.empty())
      return nullptr;
   
   texCacheLambda lambda = nullptr;
   auto uri = uri::parse_uri(link);
   if (uri.error != uri::Error::None)
   {
      PLOGE << "Invalid ressource URI: " << link;
   }
   else if (uri.scheme == "default")
   {
      if (uri.path == "/dmd")
      {
         lambda = [](const string &, const IEditable *context) -> BaseTexture * { return g_pplayer->GetControllerDisplay({ 0, 0 }).frame; };
      }
   }
   else if (uri.scheme == "script") // TODO remove as this is a temporary hook to request script DMD until VPX provides it on the plugin bus as other DMD providers, including flasher's DMD
   {
      if (uri.path == "/dmd")
      {
         lambda = [](const string &, const IEditable * context) -> BaseTexture * { 
            if (context && context->GetItemType() == ItemTypeEnum::eItemFlasher)
            {
               const Flasher *flasher = static_cast<const Flasher *>(context);
               if (flasher->m_dmdFrame != nullptr)
                  return flasher->m_dmdFrame;
            }
            return g_pplayer ? g_pplayer->m_dmdFrame : nullptr;
         };
      }
   }
   else if (uri.scheme == "plugin")
   {
      uint32_t endpointId = 0;
      auto plugin = MsgPluginManager::GetInstance().GetPlugin(uri.authority.host);
      if (plugin.get())
         endpointId = plugin->m_endpointId;
      if ((endpointId != 0)
         && (uri.path == "/getstate")
         && (std::find_if(uri.query.begin(), uri.query.end(), [](const auto &a) { return a.first == "src"; }) != uri.query.end())
         && (uri.query.at("src") == "display"))
      {
         int displayId;
         if (std::find_if(uri.query.begin(), uri.query.end(), [](const auto &a) { return a.first == "id"; }) != uri.query.end())
            try_parse_int(uri.query.at("id"), displayId);
         lambda = [endpointId, displayId](const string &, const IEditable *context) -> BaseTexture * { return g_pplayer->GetControllerDisplay({ endpointId, static_cast<uint32_t>(displayId) }).frame; };
      }
   }

   if (lambda == nullptr)
      lambda = [](const string &, const IEditable *) -> BaseTexture * { return nullptr; };
   m_texCache[link] = lambda;
   return lambda(link, context);
}

ResURIResolver::SegDisplay ResURIResolver::GetSegDisplay(const string &link, const IEditable *context)
{
   const auto &cache = m_segCache.find(link);
   if (cache != m_segCache.end())
      return cache->second(link, context);
   if (link.empty())
      return { nullptr };

   segCacheLambda lambda = nullptr;
   auto uri = uri::parse_uri(link);
   if (uri.error != uri::Error::None)
   {
      PLOGE << "Invalid ressource URI: " << link;
   }
   else if (uri.scheme == "default")
   {
      if (uri.path == "/alpha")
      {
         int subId = -1;
         if (std::find_if(uri.query.begin(), uri.query.end(), [](const auto &a) { return a.first == "sub"; }) != uri.query.end())
            try_parse_int(uri.query.at("sub"), subId);
         lambda = [subId](const string &, const IEditable *context) -> SegDisplay {
            if (subId < 0)
               return { nullptr };
            Player::ControllerSegDisplay display = g_pplayer->GetControllerSegDisplay({ 0, 0 });
            if ((display.frame == nullptr) || (subId >= (int)display.displays.size()))
               return { nullptr };
            float *data = display.frame;
            for (int i = 0; i < subId; i++)
               data += display.displays[i].size() * 16;
            return { data, display.displays[subId] };
         };
      }
   }
   else if (uri.scheme == "plugin")
   {
      uint32_t endpointId = 0;
      auto plugin = MsgPluginManager::GetInstance().GetPlugin(uri.authority.host);
      if (plugin.get())
         endpointId = plugin->m_endpointId;
      if ((endpointId != 0)
         && (uri.path == "/getstate")
         && (std::find_if(uri.query.begin(), uri.query.end(), [](const auto &a) { return a.first == "src"; }) != uri.query.end())
         && (uri.query.at("src") == "alpha"))
      {
         int displayId = -1;
         if (std::find_if(uri.query.begin(), uri.query.end(), [](const auto &a) { return a.first == "id"; }) != uri.query.end())
            try_parse_int(uri.query.at("id"), displayId);
         int subId = -1;
         if (std::find_if(uri.query.begin(), uri.query.end(), [](const auto &a) { return a.first == "sub"; }) != uri.query.end())
            try_parse_int(uri.query.at("sub"), subId);
         lambda = [endpointId, displayId, subId](const string &, const IEditable *context) -> SegDisplay
         {
            if (subId < 0)
               return { nullptr };
            Player::ControllerSegDisplay display = g_pplayer->GetControllerSegDisplay({ endpointId, static_cast<uint32_t>(displayId) });
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
      lambda = [](const string &, const IEditable *) -> SegDisplay { return { nullptr }; };
   m_segCache[link] = lambda;
   return lambda(link, context);
}