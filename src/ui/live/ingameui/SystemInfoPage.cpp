// license:GPLv3+

#include "core/stdafx.h"
#include "SystemInfoPage.h"

#include "core/VPApp.h"
#include "renderer/Renderer.h"


namespace VPX::InGameUI
{

SystemInfoPage::SystemInfoPage()
   : InGameUIPage("System Info"s, ""s, SaveMode::None)
{
}

void SystemInfoPage::BuildPage()
{
   std::ostringstream info;
   info << std::format(" *Visual Pinball*: {} \n", VP_VERSION_STRING_FULL_LITERAL);
   info << std::format(" *Logical CPU cores*: {}\n", g_app->GetLogicalNumberOfProcessors());
   info << std::format(" *GPU*: {} ({})\n", g_pplayer->m_renderer->m_renderDevice->m_GPU_name, g_pplayer->m_renderer->m_renderDevice->m_driver_name);
   // 2 reasons HDR can be off: display is not in HDR mode so no HDR10 backbuffer (see allowHDR10ColorSpace),
   // or it is, and something else declined (= video capture, anaglyph stereo, VR, or a backend without HDR10 swap chains)
   const VPX::Window* const outputWnd = g_pplayer->m_renderer->m_renderDevice->m_outputWnd[0];
   const char* const hdrState = outputWnd->IsWCGBackBuffer() ? "enabled"
                              : outputWnd->IsWCGDisplay()    ? "disabled (display is in HDR mode)"
                                                             : "disabled (display is not in HDR mode)";
   info << std::format(" *Display*: HDR {}, Refresh Rate: {} Hz, Resolution: {}x{}, Touch {}\n\n",
      hdrState, g_pplayer->m_playfieldWnd->GetRefreshRate(),
      outputWnd->GetPixelWidth(), outputWnd->GetPixelHeight(),
      (g_pplayer->m_pininput.HasTouchInput() ? "enabled" : "disabled"));
   // ImGui displays UTF-8
   info << std::format(" *App Root*: {}\n", PathToUTF8(g_app->m_fileLocator.GetAppPath(FileLocator::AppSubFolder::Root)));
   info << std::format(" *Settings*: {}\n", PathToUTF8(g_settingsService.GetAppSettings().GetIniPath()));
   info << std::format(" *Scripts*: {}\n", PathToUTF8(g_app->m_fileLocator.GetAppPath(FileLocator::AppSubFolder::Scripts, "core.vbs").parent_path()));
   info << std::format(" *Table Path*: {}\n", PathToUTF8(g_pplayer->m_ptable->m_filename));
   info << std::format(" *Table Data Path*: {}\n", PathToUTF8(g_app->m_fileLocator.GetTablePath(g_pplayer->m_ptable, FileLocator::TableSubFolder::Root, false)));

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Markdown, info.str()));
}

}
