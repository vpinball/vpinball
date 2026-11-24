// license:GPLv3+

#include "core/stdafx.h"
#include "FrontendMediaManagerPage.h"

#include "core/player.h"
#include "core/TableCatalog.h"
#include "ui/live/frontend/FrontendUI.h"
#include "ui/live/LiveUI.h"


namespace VPX::InGameUI
{

static string PageTitle()
{
   // The page is built on each navigation: pick up the entry the wheel is on
   const TableCatalog::Entry *const selected = g_pplayer->m_liveUI->m_frontendUI.GetSelected();
   return selected != nullptr ? selected->title : "Media Manager"s;
}

FrontendMediaManagerPage::FrontendMediaManagerPage()
   : InGameUIPage(PageTitle(), "Media files discovered alongside the table, with deletion."s, SaveMode::None)
{
}

void FrontendMediaManagerPage::BuildPage()
{
   VPX::Frontend::FrontendUI &frontend = m_player->m_liveUI->m_frontendUI;
   const TableCatalog::Entry *const selected = frontend.GetSelected();
   if (selected == nullptr)
      return;

   for (int type = 0; type < static_cast<int>(TableCatalog::MediaType::Count); type++)
   {
      const auto mediaType = static_cast<TableCatalog::MediaType>(type);
      const char *const typeName = TableCatalog::GetMediaTypeName(mediaType);
      if (selected->HasMedia(mediaType))
      {
         // Re-resolve the selection inside the action: deleting a file rescans the catalog,
         // which rebuilds the entries (and this page through the rebuild request)
         AddItem(std::make_unique<InGameUIItem>(std::format("Delete {}", typeName), PathToUTF8(selected->GetMedia(mediaType).filename()),
            [this, &frontend, mediaType]()
            {
               const TableCatalog::Entry *const sel = frontend.GetSelected();
               if (sel == nullptr || !sel->HasMedia(mediaType))
                  return;
               const std::filesystem::path file = sel->GetMedia(mediaType);
               std::error_code ec;
               if (std::filesystem::remove(file, ec))
                  m_player->m_liveUI->PushNotification(std::format("Deleted {}", PathToUTF8(file.filename())), 2500);
               else
                  m_player->m_liveUI->PushNotification(std::format("Failed to delete {}", PathToUTF8(file.filename())), 4000);
               frontend.Rescan();
               RequestRebuild();
            }));
      }
      else
         AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Info, std::format("{}  (none)", typeName)));
   }
}

}
