// license:GPLv3+

#include "ResURIResolver.h"

#include "simple-uri-parser/uri_parser.h"

#include <cassert>
#include <sstream>
#include <charconv>
#include <format>
using std::string;
using namespace std::string_literals;
using namespace std::string_view_literals;

namespace PinballPlugin
{

ResURIResolver::ResURIResolver(const MsgPluginAPI &msgAPI, unsigned int endpointId, bool trackDisplays, bool trackSegDisplays, bool trackStates)
   : m_msgAPI(msgAPI)
   , m_endpointId(endpointId)
{
   if (trackDisplays)
   {
      m_displaySources = std::make_unique<PinballPlugin::Controller::CtrlItemConsumer<DisplaySrcId>>(
         &m_msgAPI, endpointId, CTLPI_DISPLAY_GET_SRC_MSG, CTLPI_DISPLAY_ON_SRC_CHG_MSG,
         nullptr, // No filtering
         [this]() { m_displayCache.clear(); },
         nullptr); // No setup
      m_displaySources->SelectItems(true);
   }
   if (trackSegDisplays)
   {
      m_segSources = std::make_unique<PinballPlugin::Controller::CtrlItemConsumer<SegSrcId>>(
         &m_msgAPI, endpointId, CTLPI_SEG_GET_SRC_MSG, CTLPI_SEG_ON_SRC_CHG_MSG,
         nullptr, // No filtering
         [this]() { m_segCache.clear(); },
         nullptr); // No setup
      m_segSources->SelectItems(true);
   }
   if (trackStates)
   {
      m_stateSources = std::make_unique<PinballPlugin::Controller::CtrlItemConsumer<StateSrcId>>(
         &m_msgAPI, endpointId, CTLPI_STATE_GET_SRC_MSG, CTLPI_STATE_ON_SRC_CHG_MSG,
         nullptr, // No filtering
         [this]() { m_floatCache.clear(); },
         nullptr); // No setup
      m_stateSources->SelectItems(true);
   }
}

ResURIResolver::~ResURIResolver()
{
   // The consumers invoke their on-change callbacks from their destructor, and
   // those callbacks clear the caches below. Members are destroyed in reverse
   // declaration order, which would destroy each cache before its consumer,
   // so release the consumers explicitly while the caches are still alive.
   m_displaySources.reset();
   m_segSources.reset();
   m_stateSources.reset();
}

string ResURIResolver::trim_string(const string &str)
{
   size_t start = 0;
   size_t end = str.length();
   while (start < end && (str[start] == ' ' || str[start] == '\t' || str[start] == '\r' || str[start] == '\n'))
      ++start;
   while (end > start && (str[end - 1] == ' ' || str[end - 1] == '\t' || str[end - 1] == '\r' || str[end - 1] == '\n'))
      --end;
   return str.substr(start, end - start);
}

// trims leading whitespace or similar
bool ResURIResolver::try_parse_int(const string &str, int &value)
{
   const string tmp = trim_string(str);
   return (std::from_chars(tmp.c_str(), tmp.c_str() + tmp.length(), value).ec == std::errc { });
}

float ResURIResolver::GetFloatState(const string &link)
{
   return m_stateSources->With(
      [this, link](const std::vector<StateSrcId> &sources)
      {
         if (const auto &cache = m_floatCache.find(link); cache != m_floatCache.end())
            return cache->second(link);

         const auto &uri = uri::parse_uri(link);

         unsigned int endpoint = 0;
         if (uri.error == uri::Error::None)
         {
            if (uri.scheme == "ctrl")
            {
               if (uri.authority.host == "default")
               {
                  // Which definitions do we want to give for this (if any) ?
               }
               else
               {
                  endpoint = m_msgAPI.GetPluginEndpoint(uri.authority.host.c_str());
               }
            }
         }
         if (endpoint == 0)
         {
            // PLOGE << "Invalid resource URI: " << link;
            m_floatCache[link] = [](const string &) { return 0.f; };
            return 0.f;
         }

         floatCacheLambda lambda = nullptr;
         if (uri.path == "/state")
         {
            auto grpPart = uri.query.find("group"s);
            int group = 0;
            if (grpPart != uri.query.end() && try_parse_int(grpPart->second, group))
            {
               int mapping = 0;
               if (auto mappingPart = uri.query.find("mapping"s); mappingPart != uri.query.end() && try_parse_int(mappingPart->second, mapping))
               {
                  // Select by group + mapping
                  auto stateBlock
                     = std::ranges::find_if(sources.begin(), sources.end(), [endpoint, group](const StateSrcId &src) { return src.id.endpointId == endpoint && src.id.resId == group; });
                  if (stateBlock != sources.end())
                  {
                     for (unsigned int i = 0; i < stateBlock->nStates; i++)
                     {
                        if (const StateDef &def = stateBlock->stateDefs[i]; def.dataFormat == CTLPI_STATE_FORMAT_FLOAT && def.GetState != nullptr && def.mappingId == mapping)
                        {
                           lambda = [getter = def.GetState, id = stateBlock->id, i](const string &)
                           {
                              float value;
                              getter(id, i, &value);
                              return value;
                           };
                           break;
                        }
                     }
                  }
               }
               else if (auto namePart = uri.query.find("name"s); namePart != uri.query.end())
               {
                  // Select by group + name
                  const string name = namePart->second;
                  auto stateBlock
                     = std::ranges::find_if(sources.begin(), sources.end(), [endpoint, group](const StateSrcId &src) { return src.id.endpointId == endpoint && src.id.resId == group; });
                  if (stateBlock != sources.end())
                  {
                     for (unsigned int i = 0; i < stateBlock->nStates; i++)
                     {
                        if (const StateDef &def = stateBlock->stateDefs[i];
                           def.dataFormat == CTLPI_STATE_FORMAT_FLOAT && def.GetState != nullptr && def.name != nullptr && string(def.name) == name)
                        {
                           lambda = [getter = def.GetState, id = stateBlock->id, i](const string &)
                           {
                              float value;
                              getter(id, i, &value);
                              return value;
                           };
                           break;
                        }
                     }
                  }
               }
            }
         }
         else if (uri.path == "/display")
         {
            // TODO implement (to access individual dots, useful for small LED matrices), considering that this would require us to sync on display source as well
         }

         if (lambda == nullptr)
            lambda = [](const string &) { return 0.f; };
         m_floatCache[link] = lambda;
         return lambda(link);
      });
}

ResURIResolver::SegDisplayState ResURIResolver::GetSegDisplayState(const string &link)
{
   return m_segSources->With(
      [this, link](const std::vector<SegSrcId> &sources)
      {
         if (const auto &cache = m_segCache.find(link); cache != m_segCache.end())
            return cache->second(link);

         const auto &uri = uri::parse_uri(link);

         unsigned int endpoint = 0;
         if (uri.error == uri::Error::None)
         {
            if (uri.scheme == "ctrl")
            {
               if (uri.authority.host == "default")
               {
                  // TODO We just get the first display in the list which highly depends on the setup. Implement something more stable based on the available displays.
                  if (!sources.empty())
                     endpoint = sources.back().groupId.endpointId;
               }
               else
               {
                  endpoint = m_msgAPI.GetPluginEndpoint(uri.authority.host.c_str());
               }
            }
         }
         if (endpoint == 0)
         {
            // PLOGE << "Invalid resource URI: " << link;
            m_segCache[link] = [](const string &) { return SegDisplayState { nullptr, { 0, nullptr } }; };
            return SegDisplayState { nullptr, { 0, nullptr } };
         }

         segCacheLambda lambda = nullptr;
         if (uri.path == "/seg")
         {
            int resId = 0;
            if (auto resIdPart = uri.query.find("id"s); resIdPart != uri.query.end() && try_parse_int(resIdPart->second, resId))
            {
               auto segSrc = std::ranges::find_if(sources.begin(), sources.end(), [endpoint, resId](const SegSrcId &src) { return src.id.endpointId == endpoint && src.id.resId == resId; });
               const SegSrcId *segSource = segSrc == sources.end() ? nullptr : std::to_address(segSrc);
               if (segSource)
               {
                  int subId = 0;
                  if (auto subIdPart = uri.query.find("sub"s); subIdPart != uri.query.end())
                  {
                     if (try_parse_int(subIdPart->second, subId) && subId < static_cast<int>(segSource->nElements))
                     {
                        SegSrcId subSegSrc = *segSource;
                        subSegSrc.GetState = nullptr;
                        subSegSrc.nElements = 1;
                        subSegSrc.elementType[0] = segSource->elementType[subId];
                        lambda = [segSource, subSegSrc, subId](const string &)
                        {
                           SegDisplayFrame state = segSource->GetState(segSource->id);
                           return SegDisplayState { &subSegSrc, { state.frameId, state.frame + subId * 16 } };
                        };
                     }
                  }
                  else
                  {
                     lambda = [segSource](const string &) { return SegDisplayState { segSource, segSource->GetState(segSource->id) }; };
                  }
               }
            }
         }

         if (lambda == nullptr)
            lambda = [](const string &) { return SegDisplayState { nullptr, { 0, nullptr } }; };
         m_segCache[link] = lambda;
         return lambda(link);
      });
}

std::string ResURIResolver::DumpDisplaySources() const
{
   return m_displaySources->With(
      [](const std::vector<DisplaySrcId> &items)
      {
         std::stringstream ss;
         for (const auto &source : items)
            ss << std::format("Id:{}.{} Override:{}.{} {}x{} {}\n", source.id.endpointId, source.id.resId, source.overrideId.endpointId, source.overrideId.resId, source.width, source.height,
               source.frameFormat);
         return ss.str();
      });
}

const DisplaySrcId *ResURIResolver::GetDefaultDisplaySource(const std::vector<DisplaySrcId> &sources)
{
   const DisplaySrcId *displaySource = nullptr;
   unsigned int dsSize = 0;
   for (const auto &source : sources)
   {
      const unsigned int sSize = source.width * source.height;
      if (
         // Priority 1: Find at least one display if any (size > 0)
         displaySource == nullptr
         // Priority 2: Favor the highest resolution display
         || (dsSize < sSize)
         // Priority 3: Favor color over monochrome
         || (dsSize == sSize && displaySource->frameFormat != source.frameFormat && displaySource->frameFormat == CTLPI_DISPLAY_FORMAT_LUM32F)
         // Priority 4: Favor RGB8 over other formats
         || (dsSize == sSize && displaySource->frameFormat != source.frameFormat && source.frameFormat == CTLPI_DISPLAY_FORMAT_SRGB888)
         // Priority 5: Favor the first source provided by an endpoint
         || (dsSize == sSize && displaySource->frameFormat == source.frameFormat && displaySource->id.resId > source.id.resId))
      {
         displaySource = &source;
         dsSize = sSize;
      }
   }
   return displaySource;
}

// 'ctrl://default/display' resolves to e.g. a Pinball 2000 set's CRT, that being the only display it has, so every
// dot matrix element would otherwise draw a 640x480 picture through its DMD related shader(s). Filtered here rather than
// in GetDefaultDisplaySource() so that URI keeps meaning what it documents, the default DMD *or* display
ResURIResolver::DisplayState ResURIResolver::GetDmdDisplayState(const string &link)
{
   const DisplayState state = GetDisplayState(link);
   if (state.source != nullptr)
   {
      const unsigned int family = state.source->hardware & CTLPI_DISPLAY_HARDWARE_FAMILY_MASK;
      if ((family == CTLPI_DISPLAY_HARDWARE_CRT_DISPLAY) || (family == CTLPI_DISPLAY_HARDWARE_LCD_DISPLAY))
         return { };
   }
   return state;
}

ResURIResolver::DisplayState ResURIResolver::GetDisplayState(const string &link)
{
   return m_displaySources->With(
      [this, link](const std::vector<DisplaySrcId> &sources)
      {
         if (const auto &cache = m_displayCache.find(link); cache != m_displayCache.end())
            return cache->second(link);

         displayCacheLambda lambda = nullptr;
         if (const auto &uri = uri::parse_uri(link); uri.error != uri::Error::None)
         {
            // FIXME log PLOGE << "Invalid resource URI: " << link;
         }
         else if ((uri.scheme == "ctrl") && (uri.path == "/display"))
         {
            const DisplaySrcId *displaySource = nullptr;
            bool walkDownOverrides = true;
            if (uri.authority.host == "default")
            {
               displaySource = GetDefaultDisplaySource(sources);
            }
            else
            {
               const unsigned int plugin = m_msgAPI.GetPluginEndpoint(uri.authority.host.c_str());
               if (plugin)
               {
                  int resId = 0;
                  if (auto resIdPart = uri.query.find("id"s); resIdPart != uri.query.end())
                     try_parse_int(resIdPart->second, resId);

                  auto source = std::ranges::find_if(sources.begin(), sources.end(), [plugin, resId](const DisplaySrcId &cd) { return cd.id.endpointId == plugin && cd.id.resId == resId; });
                  if (source != sources.end())
                     displaySource = std::to_address(source);
               }
            }

            // Select the tail of the override chain if any
            // TODO allow to enable/disable overrides & handle situations where a source has multiple overrides (add a selection heuristic)
            while (walkDownOverrides && displaySource != nullptr)
            {
               walkDownOverrides = false;
               for (auto &source : sources)
               {
                  if (source.overrideId.id == displaySource->id.id)
                  {
                     displaySource = &source;
                     walkDownOverrides = true;
                     break;
                  }
               }
            }

            if (displaySource != nullptr)
               lambda = [displaySource](const string &) { return DisplayState { displaySource, displaySource->GetRenderFrame(displaySource->id) }; };
         }

         if (lambda == nullptr)
            lambda = [](const string &) { return DisplayState { nullptr, { 0, nullptr } }; };
         m_displayCache[link] = lambda;
         return lambda(link);
      });
}

};
