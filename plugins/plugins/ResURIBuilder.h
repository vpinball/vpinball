// license:GPLv3+

#pragma once

#include "MsgPluginManager.h"
#include "ControllerPlugin.h"

#include <functional>
#include <initializer_list>
#include <string>
#include <vector>

namespace PinballPlugin
{

// UI-agnostic backend shared by the ResURI picker dialogs (live editor & Win32).
// Enumerates plugins, running controllers and the sources they expose through the
// Controller plugin API, generates ctrl:// URIs and drives plugin/controller lifecycle.
// Must be created, used and destroyed on the message API thread.
class ResURIBuilder
{
public:
   enum class StateLinkMode
   {
      Name,
      Mapping
   };

   struct PluginInfo
   {
      std::string id;
      std::string name;
      std::string description;
      bool loaded = false;
      uint32_t endpointId = 0;
      std::string runningGameId; // Empty when the endpoint has no running controller
   };

   struct FloatItem
   {
      std::string pluginId;
      uint32_t endpointId = 0;
      uint32_t groupId = 0;
      std::string groupName;
      uint32_t mappingId = 0;
      std::string name;
      std::string desc;
      int semanticType = 0;
   };

   struct DisplayItem
   {
      std::string pluginId;
      uint32_t endpointId = 0;
      uint32_t resId = 0;
      unsigned int width = 0;
      unsigned int height = 0;
      unsigned int hardware = 0;
      unsigned int frameFormat = 0;
   };

   struct SegItem
   {
      std::string pluginId;
      uint32_t endpointId = 0;
      uint32_t resId = 0;
      unsigned int nElements = 0;
   };

   ResURIBuilder(MsgPI::MsgPluginManager* manager, const MsgPluginAPI* msgApi, uint32_t endpointId);
   ~ResURIBuilder();

   ResURIBuilder(const ResURIBuilder&) = delete;
   ResURIBuilder& operator=(const ResURIBuilder&) = delete;

   // True once after each controller source list change (state/display/seg)
   bool ConsumeSourcesChanged()
   {
      const bool v = m_sourcesChanged;
      m_sourcesChanged = false;
      return v;
   }

   std::vector<PluginInfo> GetPlugins() const;

   // Case-insensitive substring filter over the item name, mapping id, group name and plugin id
   std::vector<FloatItem> GetFloatItems(const std::string& filter = {}) const;
   std::vector<DisplayItem> GetDisplayItems(const std::string& filter = {}) const;
   std::vector<SegItem> GetSegItems(const std::string& filter = {}) const;

   std::string MakeStateURI(const FloatItem& item, StateLinkMode mode) const;
   std::string MakeDisplayURI(const DisplayItem& item) const;
   std::string MakeSegURI(const SegItem& item, int subElement) const; // subElement < 0 produces no 'sub' parameter

   // Reads the current state value through the provider getter. False if the state vanished or has no getter.
   bool PeekValue(const FloatItem& item, float& value) const;

   // Called when the picker enables/disables a plugin, allowing the caller to persist the choice (e.g. table settings)
   std::function<void(const std::string& pluginId, bool enabled)> onPluginEnableChanged;

   bool LoadPlugin(const std::string& pluginId); // Returns plugin->IsLoaded() after the operation
   bool UnloadPlugin(const std::string& pluginId); // Returns !plugin->IsLoaded() after the operation
   int RunController(const std::string& pluginId, const std::string& gameId); // Returns the RunCmdMsg status code
   void StopController(const std::string& pluginId);

private:
   std::string PluginIdForEndpoint(uint32_t endpointId) const;
   static bool MatchesFilter(const std::string& filter, std::initializer_list<std::string> candidates);

   MsgPI::MsgPluginManager* const m_manager;
   const MsgPluginAPI* const m_msgApi;
   const uint32_t m_endpointId;
   const unsigned int m_getControllersMsgId;
   const unsigned int m_runCmdMsgId;
   const unsigned int m_runStopMsgId;

   bool m_sourcesChanged = false;

   PinballPlugin::Controller::CtrlItemConsumer<StateSrcId> m_stateSources;
   PinballPlugin::Controller::CtrlItemConsumer<DisplaySrcId> m_displaySources;
   PinballPlugin::Controller::CtrlItemConsumer<SegSrcId> m_segSources;
};

}
