// license:GPLv3+

#pragma once

#include "InGameUIPage.h"
#include "core/TableCatalog.h"

namespace VPX::InGameUI
{

// The frontend's table metadata editor: the Info and User sections of the catalog entry's '.info'
// companion file, edited through the standard property items. The values as stored in the file are
// kept along the edited ones so the modified markers and the undo action compare against what is
// actually on disk (not the application settings the property items usually persist to).
class FrontendTableInfoPage final : public InGameUIPage
{
public:
   FrontendTableInfoPage();

private:
   void BuildPage() override;
   // SaveMode::Global only wires the save action item: the write targets the entry's '.info' file
   void SaveGlobally() override;

   // The fields exposed for edition, the comma separated lists held as plain strings
   struct EditableInfo
   {
      string title, description, manufacturer, authors, tags;
      int year = 0, rating = 0;
      bool favorite = false;
   };
   EditableInfo m_edited;
   EditableInfo m_stored;
   std::filesystem::path m_tablePath; // Entry the snapshot was taken from (guards against rebuilds)
};

}
