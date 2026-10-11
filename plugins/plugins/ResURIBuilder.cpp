// license:GPLv3+

#include "ResURIBuilder.h"

#include <algorithm>
#include <format>

using namespace MsgPI;
using namespace std::string_literals;
using namespace PinballPlugin::Controller;

namespace PinballPlugin
{

ResURIBuilder::ResURIBuilder(MsgPluginManager* manager, const MsgPluginAPI* msgApi, uint32_t endpointId)
   : m_manager(manager)
   , m_msgApi(msgApi)
   , m_endpointId(endpointId)
   , m_getControllersMsgId(msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_CONTROLLERS_GET_MSG))
   , m_runCmdMsgId(msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_RUN_CMD))
   , m_runStopMsgId(msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_RUN_STOP))
   , m_stateSources(msgApi, endpointId, CTLPI_STATE_GET_SRC_MSG, CTLPI_STATE_ON_SRC_CHG_MSG, nullptr, nullptr, [this]() { m_sourcesChanged = true; })
   , m_displaySources(msgApi, endpointId, CTLPI_DISPLAY_GET_SRC_MSG, CTLPI_DISPLAY_ON_SRC_CHG_MSG, nullptr, nullptr, [this]() { m_sourcesChanged = true; })
   , m_segSources(msgApi, endpointId, CTLPI_SEG_GET_SRC_MSG, CTLPI_SEG_ON_SRC_CHG_MSG, nullptr, nullptr, [this]() { m_sourcesChanged = true; })
{
   m_stateSources.Subscribe();
   m_displaySources.Subscribe();
   m_segSources.Subscribe();
}

ResURIBuilder::~ResURIBuilder()
{
   m_segSources.Unsubscribe();
   m_displaySources.Unsubscribe();
   m_stateSources.Unsubscribe();
   m_msgApi->ReleaseMsgID(m_runStopMsgId);
   m_msgApi->ReleaseMsgID(m_runCmdMsgId);
   m_msgApi->ReleaseMsgID(m_getControllersMsgId);
}

std::string ResURIBuilder::PluginIdForEndpoint(uint32_t endpointId) const
{
   MsgEndpointInfo info {};
   m_msgApi->GetEndpointInfo(endpointId, &info);
   return info.id != nullptr ? info.id : std::string();
}

bool ResURIBuilder::MatchesFilter(const std::string& filter, std::initializer_list<std::string> candidates)
{
   if (filter.empty())
      return true;
   const auto contains = [&filter](const std::string& haystack)
   {
      return std::search(haystack.begin(), haystack.end(), filter.begin(), filter.end(),
                [](char a, char b) { return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b)); })
         != haystack.end();
   };
   return std::ranges::any_of(candidates, contains);
}

std::vector<ResURIBuilder::PluginInfo> ResURIBuilder::GetPlugins(bool withDefaultDisplay) const
{
   const std::vector<ControllerDef> controllers = GetCtrlItems<ControllerDef>(m_msgApi, m_endpointId, m_getControllersMsgId);

   std::vector<PluginInfo> plugins;
   if (withDefaultDisplay)
      plugins.push_back(
         { .id = "default"s, .name = "default (auto)"s, .description = "Automatic default display selection"s, .loaded = true, .endpointId = DefaultEndpointId, .isDefaultDisplay = true });
   for (const auto& plugin : m_manager->GetPlugins())
   {
      PluginInfo info;
      info.id = plugin->m_id;
      info.name = plugin->m_name;
      info.description = plugin->m_description;
      info.loaded = plugin->IsLoaded();
      info.endpointId = info.loaded ? plugin->m_endpointId : 0;
      if (info.loaded)
      {
         const auto ctl = std::ranges::find_if(controllers, [endpointId = info.endpointId](const ControllerDef& def) { return def.endpointId == endpointId; });
         if (ctl != controllers.end() && ctl->gameId != nullptr)
            info.runningGameId = ctl->gameId;
      }
      plugins.push_back(std::move(info));
   }
   return plugins;
}

std::vector<ResURIBuilder::StateItem> ResURIBuilder::GetStateItems(const std::string& filter, const int semanticType, const bool writableOnly, const int dataFormat) const
{
   return m_stateSources.With(
      [this, &filter, semanticType, writableOnly, dataFormat](const std::vector<StateSrcId>& sources)
      {
         std::vector<StateItem> items;
         for (const StateSrcId& src : sources)
         {
            const std::string pluginId = PluginIdForEndpoint(src.id.endpointId);
            const std::string groupName = src.name != nullptr ? src.name : std::string();
            for (unsigned int i = 0; i < src.nStates; ++i)
            {
               const StateDef& def = src.stateDefs[i];
               if (dataFormat >= 0 && def.dataFormat != dataFormat)
                  continue;
               if (semanticType >= 0 && def.semanticType != semanticType)
                  continue;
               if (writableOnly && def.SetState == nullptr)
                  continue;
               StateItem item;
               item.pluginId = pluginId;
               item.endpointId = src.id.endpointId;
               item.groupId = src.id.resId;
               item.groupName = groupName;
               item.mappingId = def.mappingId;
               item.name = def.name != nullptr ? def.name : std::string();
               item.desc = def.desc != nullptr ? def.desc : std::string();
               item.semanticType = def.semanticType;
               item.writable = def.SetState != nullptr;
               item.dataFormat = def.dataFormat;
               if (MatchesFilter(filter, { item.name, std::to_string(item.mappingId), item.groupName, item.pluginId }))
                  items.push_back(std::move(item));
            }
         }
         return items;
      });
}

std::vector<ResURIBuilder::DisplayItem> ResURIBuilder::GetDisplayItems(const std::string& filter, DisplayFamily family, bool withDefault) const
{
   return m_displaySources.With(
      [this, &filter, family, withDefault](const std::vector<DisplaySrcId>& sources)
      {
         std::vector<DisplayItem> items;
         if (withDefault && MatchesFilter(filter, { "default"s, "automatic"s }))
         {
            DisplayItem item;
            item.pluginId = "default";
            item.endpointId = DefaultEndpointId;
            item.isDefault = true;
            item.defaultDmdOnly = family == DisplayFamily::Dmd ? 1 : family == DisplayFamily::NonDmd ? 0 : -1;
            items.push_back(std::move(item));
         }
         const auto isScreen = [](unsigned int hardware)
         {
            const unsigned int fam = hardware & CTLPI_DISPLAY_HARDWARE_FAMILY_MASK;
            return fam == CTLPI_DISPLAY_HARDWARE_CRT_DISPLAY || fam == CTLPI_DISPLAY_HARDWARE_LCD_DISPLAY;
         };
         for (const DisplaySrcId& src : sources)
         {
            if ((family == DisplayFamily::Dmd && isScreen(src.hardware)) || (family == DisplayFamily::NonDmd && !isScreen(src.hardware)))
               continue;
            DisplayItem item;
            item.pluginId = PluginIdForEndpoint(src.id.endpointId);
            item.endpointId = src.id.endpointId;
            item.resId = src.id.resId;
            item.width = src.width;
            item.height = src.height;
            item.hardware = src.hardware;
            item.frameFormat = src.frameFormat;
            if (MatchesFilter(filter, { item.pluginId, std::to_string(item.resId) }))
               items.push_back(std::move(item));
         }
         return items;
      });
}

std::vector<ResURIBuilder::SegItem> ResURIBuilder::GetSegItems(const std::string& filter) const
{
   return m_segSources.With(
      [this, &filter](const std::vector<SegSrcId>& sources)
      {
         std::vector<SegItem> items;
         for (const SegSrcId& src : sources)
         {
            SegItem item;
            item.pluginId = PluginIdForEndpoint(src.id.endpointId);
            item.endpointId = src.id.endpointId;
            item.resId = src.id.resId;
            item.nElements = src.nElements;
            if (MatchesFilter(filter, { item.pluginId, std::to_string(item.resId) }))
               items.push_back(std::move(item));
         }
         return items;
      });
}

std::string ResURIBuilder::MakeStateURI(const StateItem& item, StateLinkMode mode) const
{
   if (mode == StateLinkMode::Name && !item.name.empty())
      return std::format("ctrl://{}/state?group={}&name={}", item.pluginId, item.groupId, item.name);
   return std::format("ctrl://{}/state?group={}&mapping={}", item.pluginId, item.groupId, item.mappingId);
}

std::string ResURIBuilder::MakeDisplayURI(const DisplayItem& item) const
{
   if (item.isDefault)
      return item.defaultDmdOnly >= 0 ? std::format("ctrl://default/display?dmd_only={}", item.defaultDmdOnly) : "ctrl://default/display"s;
   return std::format("ctrl://{}/display?id={}", item.pluginId, item.resId);
}

std::string ResURIBuilder::MakeSegURI(const SegItem& item, int subElement) const
{
   if (subElement >= 0)
      return std::format("ctrl://{}/seg?id={}&sub={}", item.pluginId, item.resId, subElement);
   return std::format("ctrl://{}/seg?id={}", item.pluginId, item.resId);
}

bool ResURIBuilder::PeekValue(const StateItem& item, float& value) const
{
   return m_stateSources.With(
      [&item, &value](const std::vector<StateSrcId>& sources)
      {
         const auto src = std::ranges::find_if(sources, [&item](const StateSrcId& s) { return s.id.endpointId == item.endpointId && s.id.resId == item.groupId; });
         if (src == sources.end())
            return false;
         for (unsigned int i = 0; i < src->nStates; ++i)
         {
            if (const StateDef& def = src->stateDefs[i]; def.mappingId == item.mappingId && def.dataFormat == CTLPI_STATE_FORMAT_FLOAT && def.GetState != nullptr)
            {
               def.GetState(def.callContext, &value);
               return true;
            }
         }
         return false;
      });
}

bool ResURIBuilder::LoadPlugin(const std::string& pluginId)
{
   const auto plugin = m_manager->GetPlugin(pluginId);
   if (plugin == nullptr)
      return false;
   if (onPluginEnableChanged)
      onPluginEnableChanged(pluginId, true);
   m_manager->LoadPlugin(*plugin);
   return plugin->IsLoaded();
}

bool ResURIBuilder::UnloadPlugin(const std::string& pluginId)
{
   const auto plugin = m_manager->GetPlugin(pluginId);
   if (plugin == nullptr)
      return false;
   if (onPluginEnableChanged)
      onPluginEnableChanged(pluginId, false);
   m_manager->UnloadPlugin(*plugin);
   return !plugin->IsLoaded();
}

int ResURIBuilder::RunController(const std::string& pluginId, const std::string& gameId)
{
   const uint32_t target = m_msgApi->GetPluginEndpoint(pluginId.c_str());
   if (target == 0)
      return CTLPI_RUN_STATUS_UNSUPPORTED;
   RunCmdMsg msg { gameId.c_str(), CTLPI_RUN_STATUS_UNSUPPORTED };
   m_msgApi->SendMsg(m_endpointId, m_runCmdMsgId, target, &msg);
   return msg.status;
}

void ResURIBuilder::StopController(const std::string& pluginId)
{
   const uint32_t target = m_msgApi->GetPluginEndpoint(pluginId.c_str());
   if (target != 0)
      m_msgApi->SendMsg(m_endpointId, m_runStopMsgId, target, nullptr);
}

}
