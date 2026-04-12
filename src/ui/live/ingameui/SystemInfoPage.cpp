// license:GPLv3+

#include "core/stdafx.h"
#include "SystemInfoPage.h"

namespace VPX::InGameUI
{

SystemInfoPage::SystemInfoPage()
   : InGameUIPage("System Info"s, ""s, SaveMode::None)
{

   std::ostringstream info;
   info << std::format(" *Visual Pinball*: {} \n", VP_VERSION_STRING_FULL_LITERAL);
   info << std::format(" *Logical CPU cores*: {}\n", g_app->GetLogicalNumberOfProcessors());
   info << std::format(" *GPU*: {} ({})\n", g_pplayer->m_renderer->m_renderDevice->m_GPU_name, g_pplayer->m_renderer->m_renderDevice->m_driver_name);
   info << std::format(" *Display*: HDR {}, Refresh Rate: {} Hz, Resolution: {}x{}, Touch {}\n\n",
      (g_pplayer->m_renderer->m_renderDevice->m_outputWnd[0]->IsWCGBackBuffer() ? "enabled" : "disabled"), g_pplayer->m_playfieldWnd->GetRefreshRate(),
      g_pplayer->m_renderer->m_renderDevice->m_outputWnd[0]->GetPixelWidth(), g_pplayer->m_renderer->m_renderDevice->m_outputWnd[0]->GetPixelHeight(),
      (g_pplayer->m_pininput.HasTouchInput() ? "enabled" : "disabled"));
   info << std::format(" *App Root*: {}\n", g_app->m_fileLocator.GetAppPath(FileLocator::AppSubFolder::Root).string());
   info << std::format(" *Settings*: {}\n", g_app->GetSettingsFileName().string());
   info << std::format(" *Scripts*: {}\n", g_app->m_fileLocator.GetAppPath(FileLocator::AppSubFolder::Scripts).string());
   info << std::format(" *Table Path*: {}\n", g_app->m_fileLocator.GetTablePath(g_pplayer->m_ptable, FileLocator::TableSubFolder::Root, false).string());

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Markdown, info.str()));
}

}
