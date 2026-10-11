// license:GPLv3+

#include "core/stdafx.h"
#include "ResURIDialog.h"

#include "core/player.h"
#include "core/VPXPluginAPIImpl.h"
#include "parts/pintable.h"
#include "ui/win/PinTableWnd.h"
#include "ui/win/resource.h"

using namespace PinballPlugin;

namespace
{

constexpr UINT_PTR RefreshTimerId = 1;

const char* RunStatusMessage(int status)
{
   switch (status)
   {
   case CTLPI_RUN_STATUS_OK: return "controller started";
   case CTLPI_RUN_STATUS_UNSUPPORTED: return "plugin does not support the run command";
   case CTLPI_RUN_STATUS_RUNNING: return "discarded: a controller is already running";
   case CTLPI_RUN_STATUS_UNKNOWN_GAME: return "the game could not be resolved";
   default: return "the controller failed to start";
   }
}

void AddColumn(HWND list, int index, int width, const char* title)
{
   LVCOLUMN lvc = {};
   lvc.mask = LVCF_TEXT | LVCF_WIDTH;
   lvc.cx = width;
   lvc.pszText = const_cast<LPSTR>(title);
   ListView_InsertColumn(list, index, &lvc);
}

} // namespace

ResURIDialog::ResURIDialog(ResURIKind kind, const std::string& currentUri, const std::function<void(const std::string&)>& apply)
   : CDialog(IDD_RESURI_PICKER)
   , m_kind(kind)
   , m_currentUri(currentUri)
   , m_apply(apply)
{
}

BOOL ResURIDialog::OnInitDialog()
{
   m_pluginListHwnd = GetDlgItem(IDC_RESURI_PLUGINS).GetHwnd();
   m_itemListHwnd = GetDlgItem(IDC_RESURI_ITEMS).GetHwnd();
   ListView_SetExtendedListViewStyle(m_pluginListHwnd, LVS_EX_FULLROWSELECT);
   ListView_SetExtendedListViewStyle(m_itemListHwnd, LVS_EX_FULLROWSELECT);
   AddColumn(m_pluginListHwnd, 0, 140, "Plugin");
   AddColumn(m_pluginListHwnd, 1, 90, "Status");
   AddColumn(m_pluginListHwnd, 2, 120, "Game");
   AddColumn(m_itemListHwnd, 0, 140, "Name");
   AddColumn(m_itemListHwnd, 1, 50, "Map");
   AddColumn(m_itemListHwnd, 2, 90, "Group");
   AddColumn(m_itemListHwnd, 3, 55, "Plugin");
   AddColumn(m_itemListHwnd, 4, 40, "Value");

   MsgPI::MsgPluginManager& manager = g_pplayer->m_pluginManager;
   m_builder = std::make_unique<ResURIBuilder>(&manager, &manager.GetMsgAPI(), g_pplayer->m_pluginAPI.GetVPXEndPointId());
   m_builder->onPluginEnableChanged = [](const std::string& pluginId, bool enabled)
   {
      if (const auto propId = Settings::GetRegistry().GetPropertyId("Plugin." + pluginId, "Enable"s); propId.has_value())
         g_pplayer->m_ptable->GetSettings().Set(propId.value(), enabled, false);
   };

   CheckRadioButton(IDC_RESURI_LINK_NAME, IDC_RESURI_LINK_MAPPING, IDC_RESURI_LINK_NAME);
   RefreshPlugins();
   RefreshItems();
   UpdatePluginButtons();
   UpdateURI();
   SetTimer(RefreshTimerId, 300, nullptr);
   return TRUE;
}

void ResURIDialog::RefreshPlugins()
{
   std::string selectedId;
   if (m_selectedPluginIndex >= 0 && m_selectedPluginIndex < static_cast<int>(m_pluginList.size()))
      selectedId = m_pluginList[m_selectedPluginIndex].id;

   m_pluginList = m_builder->GetPlugins();
   ListView_DeleteAllItems(m_pluginListHwnd);
   m_selectedPluginIndex = -1;
   int idx = 0;
   for (const auto& plugin : m_pluginList)
   {
      if (plugin.id == selectedId)
         m_selectedPluginIndex = idx;
      LVITEM lv = {};
      lv.mask = LVIF_TEXT | LVIF_PARAM;
      lv.iItem = idx;
      lv.pszText = const_cast<char*>(plugin.name.c_str());
      lv.lParam = idx;
      ListView_InsertItem(m_pluginListHwnd, &lv);
      ListView_SetItemText(m_pluginListHwnd, idx, 1, const_cast<char*>(plugin.loaded ? (plugin.runningGameId.empty() ? "idle" : "running") : "disabled"));
      ListView_SetItemText(m_pluginListHwnd, idx, 2, const_cast<char*>(plugin.runningGameId.c_str()));
      ++idx;
   }
   if (m_selectedPluginIndex >= 0)
      ListView_SetItemState(m_pluginListHwnd, m_selectedPluginIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
}

void ResURIDialog::RefreshItems()
{
   const std::string filter = GetDlgItemText(IDC_RESURI_FILTER).GetString();

   uint32_t selEndpoint = 0;
   uint32_t selGroup = 0;
   uint32_t selMapping = 0;
   const bool hadSelection = m_selectedItemIndex >= 0 && m_selectedItemIndex < static_cast<int>(m_itemList.size());
   if (hadSelection)
   {
      selEndpoint = m_itemList[m_selectedItemIndex].endpointId;
      selGroup = m_itemList[m_selectedItemIndex].groupId;
      selMapping = m_itemList[m_selectedItemIndex].mappingId;
   }

   m_itemList = m_builder->GetFloatItems(filter);
   const int selectedPlugin = m_selectedPluginIndex;
   if (selectedPlugin >= 0 && selectedPlugin < static_cast<int>(m_pluginList.size()))
   {
      const uint32_t endpoint = m_pluginList[selectedPlugin].endpointId;
      if (endpoint != 0)
         std::erase_if(m_itemList, [endpoint](const ResURIBuilder::FloatItem& item) { return item.endpointId != endpoint; });
   }

   ListView_DeleteAllItems(m_itemListHwnd);
   m_selectedItemIndex = -1;
   int idx = 0;
   for (const auto& item : m_itemList)
   {
      if (hadSelection && item.endpointId == selEndpoint && item.groupId == selGroup && item.mappingId == selMapping)
         m_selectedItemIndex = idx;
      LVITEM lv = {};
      lv.mask = LVIF_TEXT | LVIF_PARAM;
      lv.iItem = idx;
      const std::string name = item.name.empty() ? "#" + std::to_string(item.mappingId) : item.name;
      lv.pszText = const_cast<char*>(name.c_str());
      lv.lParam = idx;
      ListView_InsertItem(m_itemListHwnd, &lv);
      ListView_SetItemText(m_itemListHwnd, idx, 1, const_cast<char*>(std::to_string(item.mappingId).c_str()));
      const std::string group = std::format("{} ({})", item.groupName, item.groupId);
      ListView_SetItemText(m_itemListHwnd, idx, 2, const_cast<char*>(group.c_str()));
      ListView_SetItemText(m_itemListHwnd, idx, 3, const_cast<char*>(item.pluginId.c_str()));
      float value;
      if (m_builder->PeekValue(item, value))
         ListView_SetItemText(m_itemListHwnd, idx, 4, const_cast<char*>(std::format("{:.2f}", value).c_str()));
      ++idx;
   }
   if (m_selectedItemIndex >= 0)
      ListView_SetItemState(m_itemListHwnd, m_selectedItemIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
   UpdateURI();
}

void ResURIDialog::UpdatePluginButtons()
{
   const bool hasSel = m_selectedPluginIndex >= 0 && m_selectedPluginIndex < static_cast<int>(m_pluginList.size());
   bool loaded = false;
   bool running = false;
   if (hasSel)
   {
      loaded = m_pluginList[m_selectedPluginIndex].loaded;
      running = !m_pluginList[m_selectedPluginIndex].runningGameId.empty();
   }
   GetDlgItem(IDC_RESURI_ENABLE).EnableWindow(hasSel && !loaded);
   GetDlgItem(IDC_RESURI_DISABLE).EnableWindow(hasSel && loaded);
   GetDlgItem(IDC_RESURI_GAME).EnableWindow(hasSel && loaded && !running);
   GetDlgItem(IDC_RESURI_START).EnableWindow(hasSel && loaded && !running);
   GetDlgItem(IDC_RESURI_STOP).EnableWindow(hasSel && loaded && running);
}

void ResURIDialog::UpdateURI()
{
   std::string uri;
   if (m_selectedItemIndex >= 0 && m_selectedItemIndex < static_cast<int>(m_itemList.size()))
   {
      const auto mode = IsDlgButtonChecked(IDC_RESURI_LINK_NAME) == BST_CHECKED ? ResURIBuilder::StateLinkMode::Name : ResURIBuilder::StateLinkMode::Mapping;
      uri = m_builder->MakeStateURI(m_itemList[m_selectedItemIndex], mode);
   }
   GetDlgItem(IDC_RESURI_URI).SetWindowText(uri.c_str());
   GetDlgItem(IDOK).EnableWindow(!uri.empty());
}

void ResURIDialog::ApplyAndClose(const std::string& uri)
{
   if (m_apply)
      m_apply(uri);
   m_builder.reset(); // must unsubscribe before the dialog object goes away
   EndDialog(IDOK);
}

BOOL ResURIDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
   UNREFERENCED_PARAMETER(lParam);
   switch (LOWORD(wParam))
   {
   case IDC_RESURI_ENABLE:
      if (m_selectedPluginIndex >= 0)
         m_builder->LoadPlugin(m_pluginList[m_selectedPluginIndex].id);
      RefreshPlugins();
      UpdatePluginButtons();
      return TRUE;
   case IDC_RESURI_DISABLE:
      if (m_selectedPluginIndex >= 0)
      {
         const auto& plugin = m_pluginList[m_selectedPluginIndex];
         if (g_pplayer->m_scriptInterpreter && g_pplayer->m_pluginAPI.IsScriptContributor(plugin.endpointId))
         {
            g_pplayer->m_scriptInterpreter->Stop(g_pplayer->m_ptable);
            ULONG refCount = g_pplayer->m_scriptInterpreter->Release();
            g_pplayer->m_scriptInterpreter = nullptr;
            assert(refCount == 0);
         }
         m_builder->UnloadPlugin(plugin.id);
      }
      RefreshPlugins();
      UpdatePluginButtons();
      return TRUE;
   case IDC_RESURI_START:
      if (m_selectedPluginIndex >= 0)
      {
         const std::string game = GetDlgItemText(IDC_RESURI_GAME).GetString();
         const int status = m_builder->RunController(m_pluginList[m_selectedPluginIndex].id, game);
         GetDlgItem(IDC_RESURI_STATUS).SetWindowText((m_pluginList[m_selectedPluginIndex].id + ": " + RunStatusMessage(status)).c_str());
      }
      RefreshPlugins();
      UpdatePluginButtons();
      return TRUE;
   case IDC_RESURI_STOP:
      if (m_selectedPluginIndex >= 0)
         m_builder->StopController(m_pluginList[m_selectedPluginIndex].id);
      RefreshPlugins();
      UpdatePluginButtons();
      return TRUE;
   case IDC_RESURI_FILTER:
      if (HIWORD(wParam) == EN_CHANGE)
      {
         RefreshItems();
         return TRUE;
      }
      break;
   case IDC_RESURI_LINK_NAME:
   case IDC_RESURI_LINK_MAPPING: UpdateURI(); return TRUE;
   case IDC_RESURI_CLEAR: ApplyAndClose(""); return TRUE;
   case IDOK:
      if (m_selectedItemIndex >= 0 && m_selectedItemIndex < static_cast<int>(m_itemList.size()))
      {
         const auto mode = IsDlgButtonChecked(IDC_RESURI_LINK_NAME) == BST_CHECKED ? ResURIBuilder::StateLinkMode::Name : ResURIBuilder::StateLinkMode::Mapping;
         ApplyAndClose(m_builder->MakeStateURI(m_itemList[m_selectedItemIndex], mode));
      }
      return TRUE;
   default: break;
   }
   return FALSE;
}

INT_PTR ResURIDialog::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   switch (uMsg)
   {
   case WM_NOTIFY:
   {
      const LPNMHDR hdr = reinterpret_cast<LPNMHDR>(lParam);
      if (hdr->code == LVN_ITEMCHANGED)
      {
         const LPNMLISTVIEW lv = reinterpret_cast<LPNMLISTVIEW>(lParam);
         if ((lv->uNewState & LVIS_SELECTED) != 0 && (lv->uOldState & LVIS_SELECTED) == 0)
         {
            if (hdr->hwndFrom == m_pluginListHwnd)
            {
               m_selectedPluginIndex = lv->iItem;
               UpdatePluginButtons();
               RefreshItems();
            }
            else if (hdr->hwndFrom == m_itemListHwnd)
            {
               m_selectedItemIndex = lv->iItem;
               UpdateURI();
            }
         }
      }
      break;
   }
   case WM_TIMER:
      if (wParam == RefreshTimerId)
      {
         RefreshPlugins();
         if (m_builder && m_builder->ConsumeSourcesChanged())
            RefreshItems();
         UpdatePluginButtons();
      }
      break;
   case WM_DESTROY:
      KillTimer(RefreshTimerId);
      m_builder.reset();
      break;
   }
   return DialogProcDefault(uMsg, wParam, lParam);
}
