// license:GPLv3+

#include "core/stdafx.h"
#include "ResURIPicker.h"

#include "core/player.h"
#include "core/VPXPluginAPIImpl.h"
#include "plugins/ResURIBuilder.h"
#include "ui/live/LiveUI.h"

#include <memory>
#include <optional>
#include <variant>
#include <vector>

using namespace PinballPlugin;

namespace VPX::EditorUI
{

namespace
{

   using SelectedItem = std::variant<std::monostate, ResURIBuilder::StateItem, ResURIBuilder::DisplayItem, ResURIBuilder::SegItem>;

   struct PickerState
   {
      bool open = false;
      ResURIKind kind = ResURIKind::State;
      DisplayFamily displayFamily = DisplayFamily::Any;
      std::function<void(const std::string&)> apply;
      std::unique_ptr<ResURIBuilder> builder;

      std::string currentUri;
      std::string filter;
      std::string selectedPlugin; // Empty = no selection / all plugins
      int linkMode = 1; // 0 = name, 1 = mapping
      int semanticType = -1; // -1 = all state types (see CTLPI_STATE_TYPE_xxx)
      bool writableOnly = false; // true = only states accepting SetState calls
      int dataFormat = CTLPI_STATE_FORMAT_FLOAT; // -1 = all formats (see CTLPI_STATE_FORMAT_xxx)
      int subElement = -1; // -1 = whole segment display
      SelectedItem selected;
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

   void DisablePlugin(ResURIBuilder& builder, const ResURIBuilder::PluginInfo& plugin)
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

   void ClosePicker()
   {
      ImGui::CloseCurrentPopup();
      s_picker = PickerState {};
   }

   const char* SectionTitle(ResURIKind kind)
   {
      switch (kind)
      {
      case ResURIKind::Display: return "Displays";
      case ResURIKind::SegDisplay: return "Segment displays";
      default: return "States";
      }
   }

   const char* FilterHint(ResURIKind kind)
   {
      switch (kind)
      {
      case ResURIKind::Display:
      case ResURIKind::SegDisplay: return "filter by plugin or id";
      default: return "filter by name, mapping, group or plugin";
      }
   }

   const char* HardwareLabel(unsigned int hardware)
   {
      switch (hardware & CTLPI_DISPLAY_HARDWARE_FAMILY_MASK)
      {
      case CTLPI_DISPLAY_HARDWARE_CRT_DISPLAY: return "CRT";
      case CTLPI_DISPLAY_HARDWARE_LCD_DISPLAY: return "LCD";
      case CTLPI_DISPLAY_HARDWARE_NEON_PLASMA: return "Plasma";
      case CTLPI_DISPLAY_HARDWARE_RED_LED: return "Red LED";
      case CTLPI_DISPLAY_HARDWARE_RGB_LED: return "RGB LED";
      default: return "Unknown";
      }
   }

} // namespace

void ResURIPicker::Open(
   ResURIKind kind, const std::string& currentUri, std::function<void(const std::string&)> apply, DisplayFamily displayFamily, int semanticType, bool writableOnly, int dataFormat)
{
   MsgPI::MsgPluginManager& manager = g_pplayer->m_pluginManager;
   s_picker = PickerState {};
   s_picker.open = true;
   s_picker.kind = kind;
   s_picker.displayFamily = displayFamily;
   s_picker.semanticType = semanticType;
   s_picker.writableOnly = writableOnly;
   s_picker.dataFormat = dataFormat;
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

   // Plugin list, then lifecycle commands applied to the selected plugin
   ImGui::SeparatorText("Plugins");
   const float pluginListHeight = 110.f * dpi;
   const std::vector<ResURIBuilder::PluginInfo> plugins = builder.GetPlugins(s_picker.kind == ResURIKind::Display);
   if (ImGui::BeginTable("##plugins", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, ImVec2(0, pluginListHeight)))
   {
      ImGui::TableSetupColumn("Plugin");
      ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthFixed, 90.f * dpi);
      ImGui::TableSetupColumn("Game");
      ImGui::TableHeadersRow();
      for (const ResURIBuilder::PluginInfo& plugin : plugins)
      {
         ImGui::PushID(plugin.id.c_str());
         ImGui::TableNextRow();
         ImGui::TableNextColumn();
         const bool isSelected = s_picker.selectedPlugin == plugin.id;
         if (ImGui::Selectable(plugin.name.c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns))
            s_picker.selectedPlugin = isSelected ? ""s : plugin.id;
         ImGui::TableNextColumn();
         if (plugin.isDefaultDisplay)
            ImGui::TextDisabled("automatic");
         else if (!plugin.loaded)
            ImGui::TextDisabled("disabled");
         else if (!plugin.runningGameId.empty())
            ImGui::Text("running");
         else
            ImGui::TextDisabled("idle");
         ImGui::TableNextColumn();
         if (!plugin.runningGameId.empty())
            ImGui::TextUnformatted(plugin.runningGameId.c_str());
         ImGui::PopID();
      }
      ImGui::EndTable();
   }

   const auto selectedPlugin = std::ranges::find_if(plugins, [](const ResURIBuilder::PluginInfo& p) { return p.id == s_picker.selectedPlugin && !p.isDefaultDisplay; });
   if (selectedPlugin != plugins.end())
   {
      if (!selectedPlugin->loaded)
      {
         if (ImGui::Button("Enable"))
            builder.LoadPlugin(selectedPlugin->id);
         ImGui::SameLine();
         ImGui::TextDisabled("%s", selectedPlugin->name.c_str());
      }
      else
      {
         if (ImGui::Button("Disable"))
            DisablePlugin(builder, *selectedPlugin);
         if (selectedPlugin->runningGameId.empty())
         {
            ImGui::SameLine();
            ImGui::SetNextItemWidth(160.f * dpi);
            ImGui::InputTextWithHint("##game", "game id", &s_picker.gameId);
            ImGui::SameLine();
            if (ImGui::Button("Start"))
               s_picker.status = std::string(selectedPlugin->id) + ": " + RunStatusMessage(builder.RunController(selectedPlugin->id, s_picker.gameId));
         }
         else
         {
            ImGui::SameLine();
            if (ImGui::Button("Stop"))
               builder.StopController(selectedPlugin->id);
         }
         ImGui::SameLine();
         ImGui::TextDisabled("%s", selectedPlugin->name.c_str());
      }
   }

   // Source selection
   ImGui::SeparatorText(SectionTitle(s_picker.kind));
   ImGui::SetNextItemWidth(-1.f);
   ImGui::InputTextWithHint("##filter", FilterHint(s_picker.kind), &s_picker.filter);

   const float listHeight = ImGui::GetContentRegionAvail().y - 90.f * dpi;
   if (s_picker.kind == ResURIKind::State)
   {
      std::vector<ResURIBuilder::StateItem> items = builder.GetStateItems(s_picker.filter, s_picker.semanticType, s_picker.writableOnly, s_picker.dataFormat);
      if (!s_picker.selectedPlugin.empty())
         std::erase_if(items, [](const ResURIBuilder::StateItem& item) { return item.pluginId != s_picker.selectedPlugin; });
      if (ImGui::BeginTable("##items", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, ImVec2(0, listHeight)))
      {
         ImGui::TableSetupColumn("Name");
         ImGui::TableSetupColumn("Map", ImGuiTableColumnFlags_WidthFixed, 50.f * dpi);
         ImGui::TableSetupColumn("Group");
         ImGui::TableSetupColumn("Plugin", ImGuiTableColumnFlags_WidthFixed, 90.f * dpi);
         ImGui::TableHeadersRow();
         int rowId = 0;
         for (const ResURIBuilder::StateItem& item : items)
         {
            ImGui::PushID(rowId++);
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            const auto* sel = std::get_if<ResURIBuilder::StateItem>(&s_picker.selected);
            const bool isSelected = sel != nullptr && sel->endpointId == item.endpointId && sel->groupId == item.groupId && sel->mappingId == item.mappingId;
            const std::string name = item.name.empty() ? "#" + std::to_string(item.mappingId) : item.name;
            if (ImGui::Selectable(name.c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns))
               s_picker.selected = item;
            if (!item.desc.empty() && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
               ImGui::SetTooltip("%s", item.desc.c_str());
            ImGui::TableNextColumn();
            ImGui::Text("%u", item.mappingId);
            ImGui::TableNextColumn();
            ImGui::Text("%s (%u)", item.groupName.c_str(), item.groupId);
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(item.pluginId.c_str());
            ImGui::PopID();
         }
         ImGui::EndTable();
      }
      if (items.empty())
         ImGui::TextDisabled("No matching state link. Load a plugin or start a controller to expose states.");
   }
   else if (s_picker.kind == ResURIKind::Display)
   {
      std::vector<ResURIBuilder::DisplayItem> items = builder.GetDisplayItems(s_picker.filter, s_picker.displayFamily, true);
      if (!s_picker.selectedPlugin.empty())
         std::erase_if(items, [](const ResURIBuilder::DisplayItem& item) { return item.pluginId != s_picker.selectedPlugin; });
      if (ImGui::BeginTable("##items", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, ImVec2(0, listHeight)))
      {
         ImGui::TableSetupColumn("Source");
         ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, 75.f * dpi);
         ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 85.f * dpi);
         ImGui::TableSetupColumn("Plugin", ImGuiTableColumnFlags_WidthFixed, 90.f * dpi);
         ImGui::TableHeadersRow();
         int rowId = 0;
         for (const ResURIBuilder::DisplayItem& item : items)
         {
            ImGui::PushID(rowId++);
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            const auto* sel = std::get_if<ResURIBuilder::DisplayItem>(&s_picker.selected);
            const bool isSelected = sel != nullptr && sel->endpointId == item.endpointId && sel->resId == item.resId && sel->isDefault == item.isDefault;
            const std::string name = item.isDefault ? "default - automatic selection"s : std::format("#{}", item.resId);
            if (ImGui::Selectable(name.c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns))
               s_picker.selected = item;
            if (!item.isDefault)
            {
               ImGui::TableNextColumn();
               ImGui::Text("%ux%u", item.width, item.height);
               ImGui::TableNextColumn();
               ImGui::TextUnformatted(HardwareLabel(item.hardware));
            }
            else
            {
               ImGui::TableNextColumn();
               ImGui::TableNextColumn();
            }
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(item.pluginId.c_str());
            ImGui::PopID();
         }
         ImGui::EndTable();
      }
      if (items.empty())
         ImGui::TextDisabled("No matching display source. Load a plugin or start a controller to expose displays.");
   }
   else // ResURIKind::SegDisplay
   {
      std::vector<ResURIBuilder::SegItem> items = builder.GetSegItems(s_picker.filter);
      if (!s_picker.selectedPlugin.empty())
         std::erase_if(items, [](const ResURIBuilder::SegItem& item) { return item.pluginId != s_picker.selectedPlugin; });
      if (ImGui::BeginTable("##items", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, ImVec2(0, listHeight)))
      {
         ImGui::TableSetupColumn("Source");
         ImGui::TableSetupColumn("Elements", ImGuiTableColumnFlags_WidthFixed, 80.f * dpi);
         ImGui::TableSetupColumn("Plugin", ImGuiTableColumnFlags_WidthFixed, 90.f * dpi);
         ImGui::TableHeadersRow();
         int rowId = 0;
         for (const ResURIBuilder::SegItem& item : items)
         {
            ImGui::PushID(rowId++);
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            const auto* sel = std::get_if<ResURIBuilder::SegItem>(&s_picker.selected);
            const bool isSelected = sel != nullptr && sel->endpointId == item.endpointId && sel->resId == item.resId;
            const std::string name = std::format("#{}", item.resId);
            if (ImGui::Selectable(name.c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns))
            {
               s_picker.selected = item;
               if (item.nElements <= 1)
                  s_picker.subElement = -1;
            }
            ImGui::TableNextColumn();
            ImGui::Text("%u", item.nElements);
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(item.pluginId.c_str());
            ImGui::PopID();
         }
         ImGui::EndTable();
      }
      if (items.empty())
         ImGui::TextDisabled("No matching segment display source. Load a plugin or start a controller to expose displays.");
   }

   // Sub element selection for segment displays
   if (const auto* sel = std::get_if<ResURIBuilder::SegItem>(&s_picker.selected); s_picker.kind == ResURIKind::SegDisplay && sel != nullptr && sel->nElements > 1)
   {
      if (s_picker.subElement >= static_cast<int>(sel->nElements))
         s_picker.subElement = -1;
      ImGui::SetNextItemWidth(200.f * dpi);
      if (ImGui::BeginCombo("Element", s_picker.subElement < 0 ? "whole display" : std::to_string(s_picker.subElement).c_str()))
      {
         if (ImGui::Selectable("whole display", s_picker.subElement < 0))
            s_picker.subElement = -1;
         for (unsigned int i = 0; i < sel->nElements; ++i)
         {
            if (ImGui::Selectable(std::to_string(i).c_str(), s_picker.subElement == static_cast<int>(i)))
               s_picker.subElement = static_cast<int>(i);
         }
         ImGui::EndCombo();
      }
   }
   else if (s_picker.kind == ResURIKind::State)
   {
      ImGui::RadioButton("Link by name", &s_picker.linkMode, 0);
      ImGui::SameLine();
      ImGui::RadioButton("Link by mapping", &s_picker.linkMode, 1);
   }

   // Link generation
   std::string uri;
   if (const auto* item = std::get_if<ResURIBuilder::StateItem>(&s_picker.selected))
      uri = builder.MakeStateURI(*item, s_picker.linkMode == 0 ? ResURIBuilder::StateLinkMode::Name : ResURIBuilder::StateLinkMode::Mapping);
   else if (const auto* item = std::get_if<ResURIBuilder::DisplayItem>(&s_picker.selected))
      uri = builder.MakeDisplayURI(*item);
   else if (const auto* item = std::get_if<ResURIBuilder::SegItem>(&s_picker.selected))
      uri = builder.MakeSegURI(*item, s_picker.subElement);
   ImGui::BeginDisabled(uri.empty());
   ImGui::TextWrapped("%s", uri.empty() ? "Select a source to build a link" : uri.c_str());
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
