// license:GPLv3+

#pragma once

#include <wxx_dialog.h>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "plugins/ResURIBuilder.h"

// Modal dialog building a ctrl:// URI toward a controller-exposed source (Win32 variant
// of the live editor ResURI picker, both sharing the ResURIBuilder backend).
class ResURIDialog final : public CDialog
{
public:
   ResURIDialog(PinballPlugin::ResURIKind kind, const std::string& currentUri, const std::function<void(const std::string&)>& apply);

protected:
   BOOL OnInitDialog() override;
   BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
   INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
   void RefreshPlugins();
   void RefreshItems();
   void UpdatePluginButtons();
   void UpdateURI();
   void ApplyAndClose(const std::string& uri);

   const PinballPlugin::ResURIKind m_kind;
   const std::string m_currentUri;
   const std::function<void(const std::string&)> m_apply;
   std::unique_ptr<PinballPlugin::ResURIBuilder> m_builder;

   HWND m_pluginListHwnd = nullptr;
   HWND m_itemListHwnd = nullptr;

   std::vector<PinballPlugin::ResURIBuilder::PluginInfo> m_pluginList;
   std::vector<PinballPlugin::ResURIBuilder::FloatItem> m_itemList;
   int m_selectedPluginIndex = -1;
   int m_selectedItemIndex = -1;
};
