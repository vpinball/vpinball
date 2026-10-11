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

void AddColumn(HWND list, int index, int width, const char* title)
{
   LVCOLUMN lvc = {};
   lvc.mask = LVCF_TEXT | LVCF_WIDTH;
   lvc.cx = width;
   lvc.pszText = const_cast<LPSTR>(title);
   ListView_InsertColumn(list, index, &lvc);
}

} // namespace

ResURIDialog::ResURIDialog(ResURIKind kind, const std::string& currentUri, const std::function<void(const std::string&)>& apply, DisplayFamily displayFamily)
   : CDialog(IDD_RESURI_PICKER)
   , m_kind(kind)
   , m_displayFamily(displayFamily)
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
   switch (m_kind)
   {
   case ResURIKind::Display:
      AddColumn(m_itemListHwnd, 0, 140, "Source");
      AddColumn(m_itemListHwnd, 1, 70, "Size");
      AddColumn(m_itemListHwnd, 2, 80, "Type");
      AddColumn(m_itemListHwnd, 3, 55, "Plugin");
      break;
   case ResURIKind::SegDisplay:
      AddColumn(m_itemListHwnd, 0, 140, "Source");
      AddColumn(m_itemListHwnd, 1, 70, "Elements");
      AddColumn(m_itemListHwnd, 2, 55, "Plugin");
      break;
   default:
      AddColumn(m_itemListHwnd, 0, 140, "Name");
      AddColumn(m_itemListHwnd, 1, 50, "Map");
      AddColumn(m_itemListHwnd, 2, 90, "Group");
      AddColumn(m_itemListHwnd, 3, 55, "Plugin");
      AddColumn(m_itemListHwnd, 4, 40, "Value");
      break;
   }

   MsgPI::MsgPluginManager& manager = g_pplayer->m_pluginManager;
   m_builder = std::make_unique<ResURIBuilder>(&manager, &manager.GetMsgAPI(), g_pplayer->m_pluginAPI.GetVPXEndPointId());
   m_builder->onPluginEnableChanged = [](const std::string& pluginId, bool enabled)
   {
      if (const auto propId = Settings::GetRegistry().GetPropertyId("Plugin." + pluginId, "Enable"s); propId.has_value())
         g_pplayer->m_ptable->GetSettings().Set(propId.value(), enabled, false);
   };

   CheckRadioButton(IDC_RESURI_LINK_NAME, IDC_RESURI_LINK_MAPPING, IDC_RESURI_LINK_NAME);
   GetDlgItem(IDC_RESURI_LINK_NAME).ShowWindow(m_kind == ResURIKind::FloatState ? SW_SHOW : SW_HIDE);
   GetDlgItem(IDC_RESURI_LINK_MAPPING).ShowWindow(m_kind == ResURIKind::FloatState ? SW_SHOW : SW_HIDE);
   GetDlgItem(IDC_RESURI_SUB).ShowWindow(m_kind == ResURIKind::SegDisplay ? SW_SHOW : SW_HIDE);
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

   m_pluginList = m_builder->GetPlugins(m_kind == ResURIKind::Display);
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
      ListView_SetItemText(m_pluginListHwnd, idx, 1,
         const_cast<char*>(plugin.isDefaultDisplay ? "automatic"
               : plugin.loaded                     ? (plugin.runningGameId.empty() ? "idle" : "running")
                                                   : "disabled"));
      ListView_SetItemText(m_pluginListHwnd, idx, 2, const_cast<char*>(plugin.runningGameId.c_str()));
      ++idx;
   }
   if (m_selectedPluginIndex >= 0)
      ListView_SetItemState(m_pluginListHwnd, m_selectedPluginIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
}

bool ResURIDialog::SameItem(const Item& a, const Item& b) const
{
   if (a.index() != b.index())
      return false;
   if (const auto* fa = std::get_if<ResURIBuilder::FloatItem>(&a))
   {
      const auto& fb = std::get<ResURIBuilder::FloatItem>(b);
      return fa->endpointId == fb.endpointId && fa->groupId == fb.groupId && fa->mappingId == fb.mappingId;
   }
   if (const auto* da = std::get_if<ResURIBuilder::DisplayItem>(&a))
   {
      const auto& db = std::get<ResURIBuilder::DisplayItem>(b);
      return da->endpointId == db.endpointId && da->resId == db.resId && da->isDefault == db.isDefault;
   }
   const auto& sa = std::get<ResURIBuilder::SegItem>(a);
   const auto& sb = std::get<ResURIBuilder::SegItem>(b);
   return sa.endpointId == sb.endpointId && sa.resId == sb.resId;
}

void ResURIDialog::RefreshItems()
{
   const std::string filter = GetDlgItemText(IDC_RESURI_FILTER).GetString();

   std::optional<Item> previous;
   if (m_selectedItemIndex >= 0 && m_selectedItemIndex < static_cast<int>(m_itemList.size()))
      previous = m_itemList[m_selectedItemIndex];

   uint32_t endpoint = 0;
   const int selectedPlugin = m_selectedPluginIndex;
   if (selectedPlugin >= 0 && selectedPlugin < static_cast<int>(m_pluginList.size()))
      endpoint = m_pluginList[selectedPlugin].endpointId;

   m_itemList.clear();
   if (m_kind == ResURIKind::Display)
      for (const auto& item : m_builder->GetDisplayItems(filter, m_displayFamily, true))
      {
         if (endpoint == 0 || item.endpointId == endpoint)
            m_itemList.push_back(item);
      }
   else if (m_kind == ResURIKind::SegDisplay)
      for (const auto& item : m_builder->GetSegItems(filter))
      {
         if (endpoint == 0 || item.endpointId == endpoint)
            m_itemList.push_back(item);
      }
   else
      for (const auto& item : m_builder->GetFloatItems(filter))
      {
         if (endpoint == 0 || item.endpointId == endpoint)
            m_itemList.push_back(item);
      }

   ListView_DeleteAllItems(m_itemListHwnd);
   m_selectedItemIndex = -1;
   int idx = 0;
   for (const auto& item : m_itemList)
   {
      if (previous.has_value() && SameItem(item, *previous))
         m_selectedItemIndex = idx;
      LVITEM lv = {};
      lv.mask = LVIF_TEXT | LVIF_PARAM;
      lv.iItem = idx;
      lv.lParam = idx;
      if (const auto* floatItem = std::get_if<ResURIBuilder::FloatItem>(&item))
      {
         const std::string name = floatItem->name.empty() ? "#" + std::to_string(floatItem->mappingId) : floatItem->name;
         lv.pszText = const_cast<char*>(name.c_str());
         ListView_InsertItem(m_itemListHwnd, &lv);
         ListView_SetItemText(m_itemListHwnd, idx, 1, const_cast<char*>(std::to_string(floatItem->mappingId).c_str()));
         const std::string group = std::format("{} ({})", floatItem->groupName, floatItem->groupId);
         ListView_SetItemText(m_itemListHwnd, idx, 2, const_cast<char*>(group.c_str()));
         ListView_SetItemText(m_itemListHwnd, idx, 3, const_cast<char*>(floatItem->pluginId.c_str()));
         float value;
         if (m_builder->PeekValue(*floatItem, value))
            ListView_SetItemText(m_itemListHwnd, idx, 4, const_cast<char*>(std::format("{:.2f}", value).c_str()));
      }
      else if (const auto* displayItem = std::get_if<ResURIBuilder::DisplayItem>(&item))
      {
         const std::string name = displayItem->isDefault ? "default - automatic selection"s : "#" + std::to_string(displayItem->resId);
         lv.pszText = const_cast<char*>(name.c_str());
         ListView_InsertItem(m_itemListHwnd, &lv);
         if (!displayItem->isDefault)
         {
            ListView_SetItemText(m_itemListHwnd, idx, 1, const_cast<char*>(std::format("{}x{}", displayItem->width, displayItem->height).c_str()));
            ListView_SetItemText(m_itemListHwnd, idx, 2, const_cast<char*>(HardwareLabel(displayItem->hardware)));
         }
         ListView_SetItemText(m_itemListHwnd, idx, 3, const_cast<char*>(displayItem->pluginId.c_str()));
      }
      else
      {
         const auto& segItem = std::get<ResURIBuilder::SegItem>(item);
         const std::string name = "#" + std::to_string(segItem.resId);
         lv.pszText = const_cast<char*>(name.c_str());
         ListView_InsertItem(m_itemListHwnd, &lv);
         ListView_SetItemText(m_itemListHwnd, idx, 1, const_cast<char*>(std::to_string(segItem.nElements).c_str()));
         ListView_SetItemText(m_itemListHwnd, idx, 2, const_cast<char*>(segItem.pluginId.c_str()));
      }
      ++idx;
   }
   if (m_selectedItemIndex >= 0)
      ListView_SetItemState(m_itemListHwnd, m_selectedItemIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
   UpdateURI();
}

void ResURIDialog::UpdatePluginButtons()
{
   const bool hasSel = m_selectedPluginIndex >= 0 && m_selectedPluginIndex < static_cast<int>(m_pluginList.size()) && !m_pluginList[m_selectedPluginIndex].isDefaultDisplay;
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

std::string ResURIDialog::CurrentURI() const
{
   if (m_selectedItemIndex < 0 || m_selectedItemIndex >= static_cast<int>(m_itemList.size()))
      return {};
   const Item& item = m_itemList[m_selectedItemIndex];
   if (const auto* floatItem = std::get_if<ResURIBuilder::FloatItem>(&item))
   {
      const auto mode = IsDlgButtonChecked(IDC_RESURI_LINK_NAME) == BST_CHECKED ? ResURIBuilder::StateLinkMode::Name : ResURIBuilder::StateLinkMode::Mapping;
      return m_builder->MakeStateURI(*floatItem, mode);
   }
   if (const auto* displayItem = std::get_if<ResURIBuilder::DisplayItem>(&item))
      return m_builder->MakeDisplayURI(*displayItem);
   const int sel = static_cast<int>(SendDlgItemMessage(IDC_RESURI_SUB, CB_GETCURSEL, 0, 0));
   return m_builder->MakeSegURI(std::get<ResURIBuilder::SegItem>(item), sel <= 0 ? -1 : sel - 1); // 0 = whole display, n>0 = element n-1
}

void ResURIDialog::UpdateSegCombo()
{
   if (m_kind != ResURIKind::SegDisplay)
      return;
   const auto* sel
      = (m_selectedItemIndex >= 0 && m_selectedItemIndex < static_cast<int>(m_itemList.size())) ? std::get_if<ResURIBuilder::SegItem>(&m_itemList[m_selectedItemIndex]) : nullptr;
   const int nElements = sel != nullptr ? static_cast<int>(sel->nElements) : 0;
   if (nElements == m_segComboElements)
      return;
   m_segComboElements = nElements;
   SendDlgItemMessage(IDC_RESURI_SUB, CB_RESETCONTENT, 0, 0);
   SendDlgItemMessage(IDC_RESURI_SUB, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>("whole display"));
   for (int i = 0; i < nElements; ++i)
      SendDlgItemMessage(IDC_RESURI_SUB, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(std::to_string(i).c_str()));
   SendDlgItemMessage(IDC_RESURI_SUB, CB_SETCURSEL, 0, 0);
}

void ResURIDialog::UpdateURI()
{
   UpdateSegCombo();
   const std::string uri = CurrentURI();
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
      if (m_selectedPluginIndex >= 0 && !m_pluginList[m_selectedPluginIndex].isDefaultDisplay)
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
   case IDC_RESURI_SUB: UpdateURI(); return TRUE;
   case IDC_RESURI_LINK_NAME:
   case IDC_RESURI_LINK_MAPPING: UpdateURI(); return TRUE;
   case IDC_RESURI_CLEAR: ApplyAndClose(""); return TRUE;
   case IDOK:
      if (m_selectedItemIndex >= 0)
         ApplyAndClose(CurrentURI());
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
