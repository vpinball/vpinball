// license:GPLv3+

#pragma once

#include <wxx_dialog.h>
#include <wxx_listview.h>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "plugins/ResURIBuilder.h"

class VPXPluginAPIImpl;
class Settings;

// Modal dialog building a ctrl:// URI toward a controller-exposed source (Win32 variant
// of the live editor ResURI picker, both sharing the ResURIBuilder backend).
// Since the Win32 editor runs at design time without a Player, the dialog owns its
// own plugin host: a MsgPluginManager scanning the plugin folder and a
// VPXPluginAPIImpl providing the 'vpx' endpoint plugins may need.
class ResURIDialog final : public CDialog
{
public:
   ResURIDialog(PinballPlugin::ResURIKind kind, const std::string& currentUri, const std::function<void(const std::string&)>& apply, Settings& tableSettings,
      PinballPlugin::DisplayFamily displayFamily = PinballPlugin::DisplayFamily::Any, int semanticType = -1, bool writableOnly = false, int dataFormat = CTLPI_STATE_FORMAT_FLOAT);

protected:
   BOOL OnInitDialog() override;
   BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
   INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
   using Item = std::variant<PinballPlugin::ResURIBuilder::StateItem, PinballPlugin::ResURIBuilder::DisplayItem, PinballPlugin::ResURIBuilder::SegItem>;

   void RefreshPlugins();
   void RefreshItems();
   void UpdatePluginButtons();
   void UpdateSegCombo();
   void UpdateURI();
   void ApplyAndClose(const std::string& uri);
   void ShutdownPluginHost();
   std::string CurrentURI() const;
   bool SameItem(const Item& a, const Item& b) const;

   const PinballPlugin::ResURIKind m_kind;
   const PinballPlugin::DisplayFamily m_displayFamily;
   const int m_semanticType;
   const bool m_writableOnly;
   const int m_dataFormat;
   const std::string m_currentUri;
   const std::function<void(const std::string&)> m_apply;
   Settings& m_tableSettings;

   // Design-time plugin host (no Player exists while the Win32 editor is open)
   std::unique_ptr<MsgPI::MsgPluginManager> m_pluginManager;
   std::unique_ptr<VPXPluginAPIImpl> m_pluginAPI;
   std::unique_ptr<PinballPlugin::ResURIBuilder> m_builder;

   CListView m_pluginListView;
   CListView m_itemListView;

   std::vector<PinballPlugin::ResURIBuilder::PluginInfo> m_pluginList;
   std::vector<Item> m_itemList;
   int m_selectedPluginIndex = -1;
   int m_selectedItemIndex = -1;
   int m_segComboElements = -1;
};
