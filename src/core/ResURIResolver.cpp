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
         lambda = [](const string &) -> BaseTexture * { return g_pplayer->GetControllerDisplay({ 0, 0 }).frame; };
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
            lambda = [endpointId, displayId](const string &) -> BaseTexture * { return g_pplayer->GetControllerDisplay({ endpointId, static_cast<uint32_t>(displayId) }).frame; };
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
               lambda = [subId, eltId](const string &) -> SegDisplay
               {
                  Player::ControllerSegDisplay display = g_pplayer->GetControllerSegDisplay({ 0, 0 });
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
               lambda = [subId](const string &) -> SegDisplay
               {
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
            lambda = [endpointId, displayId, subId](const string &) -> SegDisplay
            {
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
      lambda = [](const string &) -> SegDisplay { return { nullptr }; };
   m_segCache[link] = lambda;
   return lambda(link);
}
