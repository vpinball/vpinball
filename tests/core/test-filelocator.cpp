// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "core/VPApp.h"
#include "parts/pintable.h"

#include "doctest.h"

TEST_CASE("FileLocator")
{
   const FileLocator& locator = g_app->m_fileLocator;

   SUBCASE("application paths")
   {
      CHECK(DirExists(locator.GetAppPath(FileLocator::AppSubFolder::Root)));
      CHECK(locator.GetAppPath(FileLocator::AppSubFolder::Root, "test-assets") == GetAssetPath());
      CHECK(locator.GetAppPath(FileLocator::AppSubFolder::Root, "some-file.txt") == locator.GetAppPath(FileLocator::AppSubFolder::Root) / "some-file.txt");
      CHECK(locator.GetAppPath(FileLocator::AppSubFolder::Scripts) == locator.GetAppPath(FileLocator::AppSubFolder::Root) / "scripts");
      CHECK_FALSE(locator.GetAppPath(FileLocator::AppSubFolder::Preferences).empty());
   }

   SUBCASE("script search")
   {
      CHECK(locator.SearchScript(nullptr, "no-such-script-file-xyz.vbs").empty());
      // 'test-assets' is deployed along the test binary, so it is found through the application path search
      CHECK_FALSE(locator.SearchScript(nullptr, std::filesystem::path("test-assets") / "test000-default-table.vpx").empty());
   }

   SUBCASE("table paths without a table file")
   {
      PinTable* const table = CreateTestTable(); // empty filename, so no folder along table
      const std::filesystem::path cachePath = locator.GetTablePath(table, FileLocator::TableSubFolder::Cache, false);
      const std::filesystem::path autoSavePath = locator.GetTablePath(table, FileLocator::TableSubFolder::AutoSave, false);
      if (locator.GetFileLayoutMode() == FileLocator::FileLayoutMode::AppPrefData)
      {
         // In app/pref/data mode, table files always live along the table file
         CHECK(cachePath.empty());
         CHECK(autoSavePath.empty());
      }
      else
      {
         // In legacy app-only mode, they are resolved inside application folders
         CHECK(cachePath == locator.GetAppPath(FileLocator::AppSubFolder::Preferences) / "Cache" / table->m_title);
         CHECK(autoSavePath == locator.GetAppPath(FileLocator::AppSubFolder::Root));
      }
      table->Release();
   }
}
