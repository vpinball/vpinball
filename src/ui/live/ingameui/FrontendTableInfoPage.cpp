// license:GPLv3+

#include "core/stdafx.h"
#include "FrontendTableInfoPage.h"

#include "core/player.h"
#include "ui/live/frontend/FrontendUI.h"
#include "ui/live/LiveUI.h"


namespace VPX::InGameUI
{

static string JoinList(const vector<string> &list)
{
   string csv;
   for (const string &item : list)
      csv += (csv.empty() ? "" : ", ") + item;
   return csv;
}

static vector<string> SplitList(const string &csv)
{
   vector<string> list;
   string item;
   std::stringstream ss(csv);
   while (std::getline(ss, item, ','))
   {
      const size_t first = item.find_first_not_of(" \t");
      const size_t last = item.find_last_not_of(" \t");
      if (first != string::npos)
         list.push_back(item.substr(first, last - first + 1));
   }
   return list;
}

static string PageTitle()
{
   // The page is built on each navigation: pick up the entry the wheel is on
   const TableCatalog::Entry *const selected = g_pplayer->m_liveUI->m_frontendUI.GetSelected();
   return selected != nullptr ? selected->title : "Table Info"s;
}

FrontendTableInfoPage::FrontendTableInfoPage()
   : InGameUIPage(PageTitle(), "Edit the table's '.info' companion file metadata and user fields."s, SaveMode::Global)
{
}

void FrontendTableInfoPage::BuildPage()
{
   VPX::Frontend::FrontendUI &frontend = m_player->m_liveUI->m_frontendUI;
   const TableCatalog::Entry *const selected = frontend.GetSelected();
   if (selected == nullptr)
      return;

   // Snapshot the entry on the first build only: a later rebuild must not revert pending edits
   if (m_tablePath != selected->tablePath)
   {
      m_tablePath = selected->tablePath;
      m_stored.title = m_edited.title = selected->info.title;
      m_stored.description = m_edited.description = selected->info.description;
      m_stored.manufacturer = m_edited.manufacturer = selected->info.manufacturer;
      m_stored.authors = m_edited.authors = JoinList(selected->info.authors);
      m_stored.tags = m_edited.tags = JoinList(selected->user.tags);
      m_stored.year = m_edited.year = selected->info.year;
      m_stored.rating = m_edited.rating = selected->user.rating;
      m_stored.favorite = m_edited.favorite = selected->user.favorite;
   }

   // The string items edit m_edited while their stored value (modified marker, undo) reads
   // m_stored: they are excluded from the defaults action which has no meaning on file fields
   const auto addString = [this](const string &propId, const string &label, string EditableInfo::*field)
   {
      InGameUIItem &item = AddItem(std::make_unique<InGameUIItem>(
         VPX::Properties::StringPropertyDef("TableInfo"s, propId, label, ""s, false, m_stored.*field), [this, field]() { return m_edited.*field; }, [this, field](const Settings &)
         { return m_stored.*field; }, [this, field](const string &, const string &v) { m_edited.*field = v; }, [](Settings &) {}, [](const string &, Settings &, bool) {}));
      item.m_excludeFromDefault = true;
   };
   const auto addInt = [this](const string &propId, const string &label, int EditableInfo::*field, const int min, const int max)
   {
      InGameUIItem &item = AddItem(std::make_unique<InGameUIItem>(
         VPX::Properties::IntPropertyDef("TableInfo"s, propId, label, ""s, false, min, max, m_stored.*field), "%d"s, [this, field]() { return m_edited.*field; },
         [this, field](const Settings &) { return m_stored.*field; }, [this, field](int, int v) { m_edited.*field = v; }, [](Settings &) {}, [](int, Settings &, bool) {}));
      item.m_excludeFromDefault = true;
   };

   addString("title"s, "Title"s, &EditableInfo::title);
   addString("description"s, "Description"s, &EditableInfo::description);
   addString("manufacturer"s, "Manufacturer"s, &EditableInfo::manufacturer);
   addInt("year"s, "Year"s, &EditableInfo::year, 0, 2100);
   addString("authors"s, "Authors (comma separated)"s, &EditableInfo::authors);
   addInt("rating"s, "Rating"s, &EditableInfo::rating, 0, 10);
   {
      InGameUIItem &item = AddItem(std::make_unique<InGameUIItem>(
         VPX::Properties::BoolPropertyDef("TableInfo"s, "favorite"s, "Favorite"s, ""s, false, m_stored.favorite), [this]() { return m_edited.favorite; },
         [this](const Settings &) { return m_stored.favorite; }, [this](bool v) { m_edited.favorite = v; }, [](Settings &) {}, [](bool, Settings &, bool) {}));
      item.m_excludeFromDefault = true;
   }
   addString("tags"s, "Tags (comma separated)"s, &EditableInfo::tags);
}

void FrontendTableInfoPage::SaveGlobally()
{
   VPX::Frontend::FrontendUI &frontend = m_player->m_liveUI->m_frontendUI;
   const TableCatalog::Entry *const selected = frontend.GetSelected();
   if (selected == nullptr)
      return;
   TableCatalog::Entry edited = *selected;
   edited.info.title = m_edited.title;
   edited.info.description = m_edited.description;
   edited.info.manufacturer = m_edited.manufacturer;
   edited.info.year = m_edited.year;
   edited.info.authors = SplitList(m_edited.authors);
   edited.user.rating = m_edited.rating;
   edited.user.favorite = m_edited.favorite;
   edited.user.tags = SplitList(m_edited.tags);
   if (TableCatalog::SaveInfoFile(edited))
   {
      m_stored = m_edited;
      m_player->m_liveUI->PushNotification("Table info saved"s, 2500);
   }
   else
      m_player->m_liveUI->PushNotification("Failed to write the .info file"s, 4000);
   frontend.Rescan();
}

}
