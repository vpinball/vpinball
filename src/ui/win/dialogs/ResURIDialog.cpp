// license:GPLv3+

#include "core/stdafx.h"
#include "ResURIDialog.h"

#include "core/player.h"
#include "core/VPApp.h"
#include "core/VPXPluginAPIImpl.h"
#include "core/FileLocator.h"
#include "parts/pintable.h"
#include "ui/win/PinTableWnd.h"
#include "ui/win/resource.h"

#include <SDL3/SDL_loadso.h>

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

// Dynamic plugin module loader (same as the one used by Player for the play session)
class SDLModuleLoader final : public MsgPI::MsgModuleLoader
{
public:
   ~SDLModuleLoader() override = default;
   void* Link(const std::string& directory, const std::string& file) override
   {
#if defined(_MSC_VER) || defined(__MINGW32__)
      SetDllDirectoryW(MakeWString(directory).c_str()); // The paths are UTF-8
#endif
      void* dynamicModule = static_cast<void*>(SDL_LoadObject(file.c_str())); // SDL expects UTF-8
#if defined(_MSC_VER) || defined(__MINGW32__)
      SetDllDirectory(NULL);
#endif
      return dynamicModule;
   }
   void Unlink(void* dynamicModule) override { SDL_UnloadObject(static_cast<SDL_SharedObject*>(dynamicModule)); }
   void* GetFunction(void* dynamicModule, const std::string& functionName) override
   {
      return reinterpret_cast<void*>(SDL_LoadFunction(static_cast<SDL_SharedObject*>(dynamicModule), functionName.c_str()));
   }
};

} // namespace

ResURIDialog::ResURIDialog(ResURIKind kind, const std::string& currentUri, const std::function<void(const std::string&)>& apply, Settings& tableSettings, DisplayFamily displayFamily,
   const int semanticType, const bool writableOnly, const int dataFormat)
   : CDialog(IDD_RESURI_PICKER)
   , m_kind(kind)
   , m_displayFamily(displayFamily)
   , m_semanticType(semanticType)
   , m_writableOnly(writableOnly)
   , m_dataFormat(dataFormat)
   , m_currentUri(currentUri)
   , m_apply(apply)
   , m_tableSettings(tableSettings)
{
}

BOOL ResURIDialog::OnInitDialog()
{
   AttachItem(IDC_RESURI_PLUGINS, m_pluginListView);
   AttachItem(IDC_RESURI_ITEMS, m_itemListView);
   m_pluginListView.SetExtendedStyle(LVS_EX_FULLROWSELECT);
   m_itemListView.SetExtendedStyle(LVS_EX_FULLROWSELECT);
   m_pluginListView.InsertColumn(0, "Plugin", LVCFMT_LEFT, 160);
   m_pluginListView.InsertColumn(1, "Status", LVCFMT_LEFT, 90);
   m_pluginListView.InsertColumn(2, "Game", LVCFMT_LEFT, 125);
   switch (m_kind)
   {
   case ResURIKind::Display:
      m_itemListView.InsertColumn(0, "Source", LVCFMT_LEFT, 160);
      m_itemListView.InsertColumn(1, "Size", LVCFMT_LEFT, 75);
      m_itemListView.InsertColumn(2, "Type", LVCFMT_LEFT, 85);
      m_itemListView.InsertColumn(3, "Plugin", LVCFMT_LEFT, 120);
      break;
   case ResURIKind::SegDisplay:
      m_itemListView.InsertColumn(0, "Source", LVCFMT_LEFT, 160);
      m_itemListView.InsertColumn(1, "Elements", LVCFMT_LEFT, 80);
      m_itemListView.InsertColumn(2, "Plugin", LVCFMT_LEFT, 120);
      break;
   default:
      m_itemListView.InsertColumn(0, "Name", LVCFMT_LEFT, 160);
      m_itemListView.InsertColumn(1, "Map", LVCFMT_LEFT, 60);
      m_itemListView.InsertColumn(2, "Group", LVCFMT_LEFT, 110);
      m_itemListView.InsertColumn(3, "Plugin", LVCFMT_LEFT, 120);
      break;
   }

   // No Player exists at design time: the dialog hosts its own plugin manager and VPX endpoint.
   m_pluginManager = std::make_unique<MsgPI::MsgPluginManager>();
   m_pluginAPI = std::make_unique<VPXPluginAPIImpl>(*m_pluginManager, &m_tableSettings);
   m_pluginManager->ScanPluginFolder(std::make_shared<SDLModuleLoader>(), g_app->m_fileLocator.GetAppPath(FileLocator::AppSubFolder::Plugins), [](MsgPI::MsgPlugin&) { });
   // Mirror the play session: register the enable property and load plugins enabled in the table settings
   for (const auto& plugin : m_pluginManager->GetPlugins())
   {
      VPX::Properties::PropertyRegistry::PropId enableId;
      if (const auto existing = Settings::GetRegistry().GetPropertyId("Plugin." + plugin->m_id, "Enable"s); existing.has_value())
         enableId = existing.value();
      else
         enableId = Settings::GetRegistry().Register(
            std::make_unique<VPX::Properties::BoolPropertyDef>("Plugin." + plugin->m_id, "Enable"s, "Enable"s, "Enable/Disable plugin '" + plugin->m_name + '\'', true, false));
      if (m_tableSettings.GetBool(enableId))
         plugin->Load(&m_pluginManager->GetMsgAPI());
   }
   m_builder = std::make_unique<ResURIBuilder>(m_pluginManager.get(), &m_pluginManager->GetMsgAPI(), m_pluginAPI->GetVPXEndPointId());
   m_builder->onPluginEnableChanged = [this](const std::string& pluginId, bool enabled)
   {
      // Same persistence as the play session: Plugin.<id>.Enable in the table settings (registered on demand)
      VPX::Properties::PropertyRegistry::PropId enableId;
      if (const auto existing = Settings::GetRegistry().GetPropertyId("Plugin." + pluginId, "Enable"s); existing.has_value())
         enableId = existing.value();
      else
         enableId = Settings::GetRegistry().Register(
            std::make_unique<VPX::Properties::BoolPropertyDef>("Plugin." + pluginId, "Enable"s, "Enable"s, "Enable/Disable plugin '" + pluginId + '\'', true, false));
      m_tableSettings.Set(enableId, enabled, false);
   };

   CheckRadioButton(IDC_RESURI_LINK_NAME, IDC_RESURI_LINK_MAPPING, IDC_RESURI_LINK_MAPPING);
   GetDlgItem(IDC_RESURI_LINK_NAME).ShowWindow(m_kind == ResURIKind::State ? SW_SHOW : SW_HIDE);
   GetDlgItem(IDC_RESURI_LINK_MAPPING).ShowWindow(m_kind == ResURIKind::State ? SW_SHOW : SW_HIDE);
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
   if (!m_builder)
      return;
   std::vector<ResURIBuilder::PluginInfo> newList = m_builder->GetPlugins(m_kind == ResURIKind::Display);
   const bool unchanged = newList.size() == m_pluginList.size()
      && std::equal(newList.begin(), newList.end(), m_pluginList.begin(), [](const ResURIBuilder::PluginInfo& a, const ResURIBuilder::PluginInfo& b)
         { return a.id == b.id && a.loaded == b.loaded && a.runningGameId == b.runningGameId && a.isDefaultDisplay == b.isDefaultDisplay; });
   if (unchanged)
      return; // Rebuilding the ListView resets the scroll position and steals the selection

   std::string selectedId;
   if (m_selectedPluginIndex >= 0 && m_selectedPluginIndex < static_cast<int>(m_pluginList.size()))
      selectedId = m_pluginList[m_selectedPluginIndex].id;

   m_pluginList = std::move(newList);
   m_pluginListView.DeleteAllItems();
   m_selectedPluginIndex = -1;
   int idx = 0;
   for (const auto& plugin : m_pluginList)
   {
      if (plugin.id == selectedId)
         m_selectedPluginIndex = idx;
      m_pluginListView.InsertItem(idx, plugin.name.c_str());
      m_pluginListView.SetItemText(idx, 1, plugin.isDefaultDisplay ? "automatic" : plugin.loaded ? (plugin.runningGameId.empty() ? "idle" : "running") : "disabled");
      m_pluginListView.SetItemText(idx, 2, plugin.runningGameId.c_str());
      ++idx;
   }
   if (m_selectedPluginIndex >= 0)
      m_pluginListView.SetItemState(m_selectedPluginIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
}

bool ResURIDialog::SameItem(const Item& a, const Item& b) const
{
   if (a.index() != b.index())
      return false;
   if (const auto* fa = std::get_if<ResURIBuilder::StateItem>(&a))
   {
      const auto& fb = std::get<ResURIBuilder::StateItem>(b);
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
   if (!m_builder)
      return;
   const std::string filter = GetDlgItemText(IDC_RESURI_FILTER).GetString();

   std::optional<Item> previous;
   if (m_selectedItemIndex >= 0 && m_selectedItemIndex < static_cast<int>(m_itemList.size()))
      previous = m_itemList[m_selectedItemIndex];

   std::string pluginId;
   const int selectedPlugin = m_selectedPluginIndex;
   if (selectedPlugin >= 0 && selectedPlugin < static_cast<int>(m_pluginList.size()))
      pluginId = m_pluginList[selectedPlugin].id;

   std::vector<Item> newList;
   if (m_kind == ResURIKind::Display)
      for (const auto& item : m_builder->GetDisplayItems(filter, m_displayFamily, true))
      {
         if (pluginId.empty() || item.pluginId == pluginId)
            newList.push_back(item);
      }
   else if (m_kind == ResURIKind::SegDisplay)
      for (const auto& item : m_builder->GetSegItems(filter))
      {
         if (pluginId.empty() || item.pluginId == pluginId)
            newList.push_back(item);
      }
   else
      for (const auto& item : m_builder->GetStateItems(filter, m_semanticType, m_writableOnly, m_dataFormat))
      {
         if (pluginId.empty() || item.pluginId == pluginId)
            newList.push_back(item);
      }

   const bool unchanged = newList.size() == m_itemList.size()
      && std::equal(newList.begin(), newList.end(), m_itemList.begin(),
         [](const Item& a, const Item& b)
         {
            if (a.index() != b.index())
               return false;
            if (const auto* fa = std::get_if<ResURIBuilder::StateItem>(&a))
            {
               const auto& fb = std::get<ResURIBuilder::StateItem>(b);
               return fa->pluginId == fb.pluginId && fa->endpointId == fb.endpointId && fa->groupId == fb.groupId && fa->groupName == fb.groupName && fa->mappingId == fb.mappingId
                  && fa->name == fb.name;
            }
            if (const auto* da = std::get_if<ResURIBuilder::DisplayItem>(&a))
            {
               const auto& db = std::get<ResURIBuilder::DisplayItem>(b);
               return da->pluginId == db.pluginId && da->endpointId == db.endpointId && da->resId == db.resId && da->width == db.width && da->height == db.height
                  && da->hardware == db.hardware && da->isDefault == db.isDefault && da->defaultDmdOnly == db.defaultDmdOnly;
            }
            const auto& sa = std::get<ResURIBuilder::SegItem>(a);
            const auto& sb = std::get<ResURIBuilder::SegItem>(b);
            return sa.pluginId == sb.pluginId && sa.endpointId == sb.endpointId && sa.resId == sb.resId && sa.nElements == sb.nElements;
         });
   if (unchanged)
      return; // Rebuilding the ListView resets the scroll position and steals the selection

   m_itemList = std::move(newList);
   m_itemListView.DeleteAllItems();
   m_selectedItemIndex = -1;
   int idx = 0;
   for (const auto& item : m_itemList)
   {
      if (previous.has_value() && SameItem(item, *previous))
         m_selectedItemIndex = idx;
      if (const auto* floatItem = std::get_if<ResURIBuilder::StateItem>(&item))
      {
         const std::string name = floatItem->name.empty() ? "#" + std::to_string(floatItem->mappingId) : floatItem->name;
         m_itemListView.InsertItem(idx, name.c_str());
         m_itemListView.SetItemText(idx, 1, std::to_string(floatItem->mappingId).c_str());
         m_itemListView.SetItemText(idx, 2, std::format("{} ({})", floatItem->groupName, floatItem->groupId).c_str());
         m_itemListView.SetItemText(idx, 3, floatItem->pluginId.c_str());
      }
      else if (const auto* displayItem = std::get_if<ResURIBuilder::DisplayItem>(&item))
      {
         m_itemListView.InsertItem(idx, displayItem->isDefault ? "default - automatic selection" : std::format("#{}", displayItem->resId).c_str());
         if (!displayItem->isDefault)
         {
            m_itemListView.SetItemText(idx, 1, std::format("{}x{}", displayItem->width, displayItem->height).c_str());
            m_itemListView.SetItemText(idx, 2, HardwareLabel(displayItem->hardware));
         }
         m_itemListView.SetItemText(idx, 3, displayItem->pluginId.c_str());
      }
      else
      {
         const auto& segItem = std::get<ResURIBuilder::SegItem>(item);
         m_itemListView.InsertItem(idx, std::format("#{}", segItem.resId).c_str());
         m_itemListView.SetItemText(idx, 1, std::to_string(segItem.nElements).c_str());
         m_itemListView.SetItemText(idx, 2, segItem.pluginId.c_str());
      }
      ++idx;
   }
   if (m_selectedItemIndex >= 0)
      m_itemListView.SetItemState(m_selectedItemIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
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
   if (const auto* floatItem = std::get_if<ResURIBuilder::StateItem>(&item))
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
   ShutdownPluginHost();
   EndDialog(IDOK);
}

void ResURIDialog::ShutdownPluginHost()
{
   m_builder.reset(); // must unsubscribe before its message endpoints go away
   if (m_pluginManager)
      m_pluginManager->UnloadPlugins();
   m_pluginAPI.reset();
   m_pluginManager.reset();
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
         if (g_pplayer && g_pplayer->m_scriptInterpreter && m_pluginAPI->IsScriptContributor(plugin.endpointId))
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
            if (hdr->hwndFrom == m_pluginListView.GetHwnd())
            {
               m_selectedPluginIndex = lv->iItem;
               UpdatePluginButtons();
               RefreshItems();
            }
            else if (hdr->hwndFrom == m_itemListView.GetHwnd())
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
         if (m_pluginManager)
            m_pluginManager->ProcessAsyncCallbacks();
         RefreshPlugins();
         if (m_builder && m_builder->ConsumeSourcesChanged())
            RefreshItems();
         UpdatePluginButtons();
      }
      break;
   case WM_DESTROY:
      KillTimer(RefreshTimerId);
      ShutdownPluginHost();
      break;
   }
   return DialogProcDefault(uMsg, wParam, lParam);
}
