// license:GPLv3+

#pragma once

#include <wxx_dialog.h>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "plugins/ResURIBuilder.h"

// Modal dialog building a ctrl:// URI toward a controller-exposed source (Win32 variant
// of the live editor ResURI picker, both sharing the ResURIBuilder backend).
class ResURIDialog final : public CDialog
{
public:
   ResURIDialog(PinballPlugin::ResURIKind kind, const std::string& currentUri, const std::function<void(const std::string&)>& apply,
      PinballPlugin::DisplayFamily displayFamily = PinballPlugin::DisplayFamily::Any);

protected:
   BOOL OnInitDialog() override;
   BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
   INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
   using Item = std::variant<PinballPlugin::ResURIBuilder::FloatItem, PinballPlugin::ResURIBuilder::DisplayItem, PinballPlugin::ResURIBuilder::SegItem>;

   void RefreshPlugins();
   void RefreshItems();
   void UpdatePluginButtons();
   void UpdateSegCombo();
   void UpdateURI();
   void ApplyAndClose(const std::string& uri);
   std::string CurrentURI() const;
   bool SameItem(const Item& a, const Item& b) const;

   const PinballPlugin::ResURIKind m_kind;
   const PinballPlugin::DisplayFamily m_displayFamily;
   const std::string m_currentUri;
   const std::function<void(const std::string&)> m_apply;
   std::unique_ptr<PinballPlugin::ResURIBuilder> m_builder;

   HWND m_pluginListHwnd = nullptr;
   HWND m_itemListHwnd = nullptr;

   std::vector<PinballPlugin::ResURIBuilder::PluginInfo> m_pluginList;
   std::vector<Item> m_itemList;
   int m_selectedPluginIndex = -1;
   int m_selectedItemIndex = -1;
   int m_segComboElements = -1;
};
