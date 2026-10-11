// license:GPLv3+

#include "core/stdafx.h"
#include "ResURIPicker.h"

#include "core/player.h"
#include "core/VPXPluginAPIImpl.h"
#include "plugins/ResURIBuilder.h"
#include "ui/live/LiveUI.h"

#include <map>
#include <memory>
#include <optional>
#include <vector>

using namespace PinballPlugin;

namespace VPX::EditorUI
{

namespace
{

   struct PickerState
   {
      bool open = false;
      ResURIKind kind = ResURIKind::FloatState;
      std::function<void(const std::string&)> apply;
      std::unique_ptr<ResURIBuilder> builder;

      std::string currentUri;
      std::string filter;
      uint32_t selectedEndpoint = 0; // 0 = all endpoints
      int linkMode = 0; // 0 = name, 1 = mapping
      std::optional<ResURIBuilder::FloatItem> selected;
      std::string gameId;
      std::string status;
   };

   PickerState s_picker;

   const char* RunStatusMessage(int status)
   {
      switch (status)
      {
      case CTLPI_RUN_STATUS_OK: return "Controller started";
      case CTLPI_RUN_STATUS_UNSUPPORTED: return "This plugin does not support the run command";
      case CTLPI_RUN_STATUS_RUNNING: return "Discarded: a controller is already running";
      case CTLPI_RUN_STATUS_UNKNOWN_GAME: return "The game could not be resolved";
      default: return "The controller failed to start";
      }
   }

   void ClosePicker()
   {
      ImGui::CloseCurrentPopup();
      s_picker = PickerState {};
   }

} // namespace

void ResURIPicker::Open(ResURIKind kind, const std::string& currentUri, std::function<void(const std::string&)> apply)
{
   MsgPI::MsgPluginManager& manager = g_pplayer->m_pluginManager;
   s_picker = PickerState {};
   s_picker.open = true;
   s_picker.kind = kind;
   s_picker.apply = std::move(apply);
   s_picker.currentUri = currentUri;
   s_picker.builder = std::make_unique<ResURIBuilder>(&manager, &manager.GetMsgAPI(), g_pplayer->m_pluginAPI.GetVPXEndPointId());
   s_picker.builder->onPluginEnableChanged = [](const std::string& pluginId, bool enabled)
   {
      if (const auto propId = Settings::GetRegistry().GetPropertyId("Plugin." + pluginId, "Enable"s); propId.has_value())
         g_pplayer->m_ptable->GetSettings().Set(propId.value(), enabled, false);
   };
   ImGui::OpenPopup("Link to controller source");
}

void ResURIPicker::Render()
{
   if (!s_picker.open)
      return;

   const float dpi = g_pplayer->m_liveUI->GetDPI();
   ImGui::SetNextWindowSize(ImVec2(560.f * dpi, 540.f * dpi), ImGuiCond_FirstUseEver);
   bool open = true;
   if (!ImGui::BeginPopupModal("Link to controller source", &open))
   {
      // Popup was closed or the owning field is no longer rendered
      s_picker = PickerState {};
      return;
   }

   ResURIBuilder& builder = *s_picker.builder;

   // Plugin list with lifecycle commands
   ImGui::SeparatorText("Plugins");
   const float pluginListHeight = 130.f * dpi;
   if (ImGui::BeginChild("##plugins", ImVec2(0, pluginListHeight), ImGuiChildFlags_Borders))
   {
      for (const ResURIBuilder::PluginInfo& plugin : builder.GetPlugins())
      {
         ImGui::PushID(plugin.id.c_str());
         const bool isSelected = s_picker.selectedEndpoint == plugin.endpointId && plugin.endpointId != 0;
         if (ImGui::Selectable(plugin.name.c_str(), isSelected))
            s_picker.selectedEndpoint = isSelected ? 0 : plugin.endpointId;
         ImGui::SameLine(200.f * dpi);
         if (!plugin.loaded)
         {
            ImGui::TextDisabled("disabled");
            ImGui::SameLine();
            if (ImGui::SmallButton("Enable"))
               builder.LoadPlugin(plugin.id);
         }
         else if (!plugin.runningGameId.empty())
         {
            ImGui::Text("running: %s", plugin.runningGameId.c_str());
            ImGui::SameLine();
            if (ImGui::SmallButton("Stop"))
               builder.StopController(plugin.id);
            ImGui::SameLine();
            if (ImGui::SmallButton("Disable"))
            {
               if (g_pplayer->m_scriptInterpreter && g_pplayer->m_pluginAPI.IsScriptContributor(plugin.endpointId))
               {
                  g_pplayer->m_scriptInterpreter->Stop(g_pplayer->m_ptable);
                  ULONG refCount = g_pplayer->m_scriptInterpreter->Release();
                  g_pplayer->m_scriptInterpreter = nullptr;
                  assert(refCount == 0);
               }
               builder.UnloadPlugin(plugin.id);
            }
         }
         else
         {
            ImGui::SetNextItemWidth(140.f * dpi);
            ImGui::InputTextWithHint("##game", "game id", &s_picker.gameId);
            ImGui::SameLine();
            if (ImGui::SmallButton("Start"))
               s_picker.status = std::string(plugin.id) + ": " + RunStatusMessage(builder.RunController(plugin.id, s_picker.gameId));
            ImGui::SameLine();
            if (ImGui::SmallButton("Disable"))
            {
               if (g_pplayer->m_scriptInterpreter && g_pplayer->m_pluginAPI.IsScriptContributor(plugin.endpointId))
               {
                  g_pplayer->m_scriptInterpreter->Stop(g_pplayer->m_ptable);
                  ULONG refCount = g_pplayer->m_scriptInterpreter->Release();
                  g_pplayer->m_scriptInterpreter = nullptr;
                  assert(refCount == 0);
               }
               builder.UnloadPlugin(plugin.id);
            }
         }
         ImGui::PopID();
      }
      ImGui::EndChild();
   }

   // Source selection (FloatState kind only for now)
   ImGui::SeparatorText("States");
   ImGui::SetNextItemWidth(-1.f);
   ImGui::InputTextWithHint("##filter", "filter by name, mapping, group or plugin", &s_picker.filter);

   std::vector<ResURIBuilder::FloatItem> items = builder.GetFloatItems(s_picker.filter);
   if (s_picker.selectedEndpoint != 0)
      std::erase_if(items, [endpoint = s_picker.selectedEndpoint](const ResURIBuilder::FloatItem& item) { return item.endpointId != endpoint; });

   // Group items by plugin/group preserving source order
   std::map<std::pair<std::string, uint32_t>, std::pair<std::string, std::vector<const ResURIBuilder::FloatItem*>>> groups;
   for (const ResURIBuilder::FloatItem& item : items)
   {
      auto& group = groups[{ item.pluginId, item.groupId }];
      group.first = item.groupName;
      group.second.push_back(&item);
   }

   const float listHeight = ImGui::GetContentRegionAvail().y - 90.f * dpi;
   if (ImGui::BeginChild("##items", ImVec2(0, std::max(listHeight, 80.f * dpi)), ImGuiChildFlags_Borders))
   {
      if (items.empty())
         ImGui::TextDisabled("No matching state source. Load a plugin or start a controller to expose states.");
      for (const auto& [key, group] : groups)
      {
         ImGui::PushID((key.first + '#' + std::to_string(key.second)).c_str());
         const std::string header = std::format("{} - {} (group {})", key.first, group.first.empty() ? "States" : group.first, key.second);
         if (ImGui::CollapsingHeader(header.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
         {
            int rowId = 0;
            for (const ResURIBuilder::FloatItem* item : group.second)
            {
               ImGui::PushID(rowId++);
               const bool isSelected
                  = s_picker.selected && s_picker.selected->endpointId == item->endpointId && s_picker.selected->groupId == item->groupId && s_picker.selected->mappingId == item->mappingId;
               const std::string label = std::format("{}  (mapping {})", item->name.empty() ? "#" + std::to_string(item->mappingId) : item->name, item->mappingId);
               if (ImGui::Selectable(label.c_str(), isSelected))
                  s_picker.selected = *item;
               if (!item->desc.empty() && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
                  ImGui::SetTooltip("%s", item->desc.c_str());
               float value;
               if (builder.PeekValue(*item, value))
               {
                  ImGui::SameLine(320.f * dpi);
                  ImGui::Text("%5.2f", value);
               }
               ImGui::PopID();
            }
         }
         ImGui::PopID();
      }
      ImGui::EndChild();
   }

   // Link generation
   ImGui::RadioButton("Link by name", &s_picker.linkMode, 0);
   ImGui::SameLine();
   ImGui::RadioButton("Link by mapping", &s_picker.linkMode, 1);

   std::string uri;
   if (s_picker.selected)
      uri = builder.MakeStateURI(*s_picker.selected, s_picker.linkMode == 0 ? ResURIBuilder::StateLinkMode::Name : ResURIBuilder::StateLinkMode::Mapping);
   ImGui::BeginDisabled(uri.empty());
   ImGui::TextWrapped("%s", uri.empty() ? "Select a state to build a link" : uri.c_str());
   ImGui::EndDisabled();
   if (!s_picker.status.empty())
      ImGui::TextDisabled("%s", s_picker.status.c_str());

   ImGui::BeginDisabled(uri.empty());
   if (ImGui::Button("Use"))
   {
      s_picker.apply(uri);
      ClosePicker();
   }
   ImGui::EndDisabled();
   ImGui::SameLine();
   if (ImGui::Button("Clear"))
   {
      s_picker.apply("");
      ClosePicker();
   }
   ImGui::SameLine();
   if (ImGui::Button("Cancel"))
      ClosePicker();

   if (!open)
      ClosePicker();
   ImGui::EndPopup();
}

}
