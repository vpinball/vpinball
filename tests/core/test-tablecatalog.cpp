// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "core/TableCatalog.h"

#include "doctest.h"

#include <fstream>

TEST_CASE("TableCatalog")
{
   const std::filesystem::path root = GetTestTmpDir() / "catalog"sv;
   std::filesystem::remove_all(root);

   const auto writeFile = [](const std::filesystem::path& path, const string& content = ""s)
   {
      std::filesystem::create_directories(path.parent_path());
      std::ofstream file(path, std::ios::binary);
      file << content;
   };

   // A table stored in its own folder, with an info file named after the folder and medias
   const std::filesystem::path folderA = root / "Attack From Mars (Bally 1995)"sv;
   writeFile(folderA / "Attack From Mars v1.0.vpx"s);
   writeFile(folderA / "Attack From Mars (Bally 1995).info"s, R"(
      {
         "Info": {
            "Title": "Attack From Mars",
            "Description": "The classic table",
            "Authors": ["Author One", "Author Two"],
            "Manufacturer": "Bally",
            "Year": 1995,
            "VPSId": "afm_113",
            "IPDBId": "3781"
         },
         "User": {
            "Rating": 9,
            "Favorite": 1,
            "LastRun": "2024-05-01T10:00:00Z",
            "StartCount": 42,
            "RunTime": 3600,
            "Tags": ["SS", "Classic"]
         },
         "VPX": {
            "Version": "1.0",
            "LastRun": "2024-05-02T10:00:00Z",
            "StartCount": 12,
            "RunTime": 1800
         }
      })");
   writeFile(folderA / "medias"sv / "(Playfield) Attack From Mars.png"s);
   writeFile(folderA / "medias"sv / "(Wheel) Attack From Mars.png"s);
   writeFile(folderA / "medias"sv / "(AudioLaunch) intro.mp3"s); // unsupported: audio is not rendered
   writeFile(folderA / "medias"sv / "(GameInfo) clip.f4v"s); // f4v is a supported video (PinballY convention)
   writeFile(folderA / "medias"sv / "(Playfield) clip.mp4"s); // video is favored over the still
   writeFile(folderA / "medias"sv / "(Topper) clip.mp4"s); // video media, played through the shared decoding service
   writeFile(folderA / "medias"sv / "(Playfield) decoy.vpx"s); // decoy: files inside medias are never tables
   // An APNG file: 'acTL' animation control chunk between IHDR and the first IDAT
   const string apng = "\x89PNG\r\n\x1a\n"s + "\x00\x00\x00\x0DIHDR"s + string(17, '\0')
      + "\x00\x00\x00\x08"
        "acTL"s
      + string(12, '\0') + "\x00\x00\x00\x00IDAT"s;
   writeFile(folderA / "medias"sv / "(Wheel) animated.png"s, apng); // animated image is favored over the still
   writeFile(folderA / "medias"sv / "(Other) alt.apng"s); // .apng is a supported extension

   // A loose table file with an info file named after it
   writeFile(root / "Medieval Madness.vpx"s);
   writeFile(root / "Medieval Madness.info"s, R"({ "Info": { "Title": "Medieval Madness", "Year": "1997" } })");

   // A table nested deeper, without any companion file
   writeFile(root / "nested"sv / "deep"sv / "Congo.vpx"s);

   // VPZ packs: a folder holding a manifest, and an archive file
   writeFile(root / "Funhouse Pack"sv / "manifest.json"s, "{}"s);
   writeFile(root / "The Addams Family.vpz"s);

   TableCatalog catalog;
   catalog.Scan({ root, root / "does-not-exist"sv });
   const auto& entries = catalog.GetEntries();

   REQUIRE(entries.size() == 5);

   const auto findByStem = [&entries](const string& stem) -> const TableCatalog::Entry*
   {
      for (const auto& entry : entries)
         if (PathToUTF8(entry.tablePath.stem()) == stem)
            return &entry;
      return nullptr;
   };

   SUBCASE("entries are sorted by display title")
   {
      CHECK(PathToUTF8(entries[0].tablePath.stem()) == "Attack From Mars v1.0"s);
      CHECK(PathToUTF8(entries[1].tablePath.stem()) == "Congo"s);
      CHECK(PathToUTF8(entries[2].tablePath.stem()) == "Funhouse Pack"s);
      CHECK(PathToUTF8(entries[3].tablePath.stem()) == "Medieval Madness"s);
      CHECK(PathToUTF8(entries[4].tablePath.stem()) == "The Addams Family"s);
   }

   SUBCASE("path scan defers info and media details")
   {
      TableCatalog pathCatalog;
      pathCatalog.ScanPaths({ root });
      const auto& pathEntries = pathCatalog.GetEntries();
      REQUIRE(pathEntries.size() == 5);
      const TableCatalog::Entry* entry = nullptr;
      size_t index = 0;
      for (size_t i = 0; i < pathEntries.size(); ++i)
         if (PathToUTF8(pathEntries[i].tablePath.stem()) == "Attack From Mars v1.0"s)
         {
            entry = &pathEntries[i];
            index = i;
         }
      REQUIRE(entry != nullptr);
      CHECK(entry->hasInfoFile);
      CHECK(!entry->detailsLoaded);
      CHECK(entry->title == "Attack From Mars v1.0"s);
      CHECK(entry->info.manufacturer.empty());
      CHECK(!entry->HasMedia(TableCatalog::MediaType::Playfield));

      pathCatalog.EnsureEntryDetails(index);
      CHECK(entry->detailsLoaded);
      CHECK(entry->title == "Attack From Mars"s);
      CHECK(entry->info.manufacturer == "Bally"s);
      CHECK(entry->HasMedia(TableCatalog::MediaType::Playfield));
   }

   SUBCASE("info file named after the folder is found and parsed")
   {
      const TableCatalog::Entry* entry = findByStem("Attack From Mars v1.0"s);
      REQUIRE(entry != nullptr);
      CHECK(entry->hasInfoFile);
      CHECK(!entry->isPack);
      CHECK(PathToUTF8(entry->infoPath.filename()) == "Attack From Mars (Bally 1995).info"s);
      CHECK(entry->title == "Attack From Mars"s);
      CHECK(entry->info.manufacturer == "Bally"s);
      CHECK(entry->info.year == 1995);
      CHECK(entry->info.authors.size() == 2);
      CHECK(entry->info.authors[1] == "Author Two"s);
      CHECK(entry->info.vpsId == "afm_113"s);
      CHECK(entry->info.ipdbId == "3781"s);
      CHECK(entry->info.description == "The classic table"s);
      CHECK(entry->user.rating == 9);
      CHECK(entry->user.favorite);
      CHECK(entry->user.startCount == 42);
      CHECK(entry->user.runTime == 3600);
      CHECK(entry->user.tags.size() == 2);
      CHECK(entry->vpx.version == "1.0"s);
      CHECK(entry->vpx.startCount == 12);
   }

   SUBCASE("medias are located per type")
   {
      const TableCatalog::Entry* entry = findByStem("Attack From Mars v1.0"s);
      REQUIRE(entry != nullptr);
      CHECK(entry->HasMedia(TableCatalog::MediaType::Playfield));
      CHECK(entry->HasMedia(TableCatalog::MediaType::Wheel));
      CHECK(!entry->HasMedia(TableCatalog::MediaType::Backglass));
      CHECK(!entry->HasMedia(TableCatalog::MediaType::AudioLaunch)); // dropped: unsupported audio format
      CHECK(entry->HasMedia(TableCatalog::MediaType::Topper));
      CHECK(PathToUTF8(entry->GetMedia(TableCatalog::MediaType::Playfield).filename()) == "(Playfield) clip.mp4"s); // the video wins over the still
      CHECK(PathToUTF8(entry->GetMedia(TableCatalog::MediaType::Wheel).filename()) == "(Wheel) animated.png"s); // the animated image wins over the still
      CHECK(PathToUTF8(entry->GetMedia(TableCatalog::MediaType::GameInfo).filename()) == "(GameInfo) clip.f4v"s);
      CHECK(PathToUTF8(entry->GetMedia(TableCatalog::MediaType::Topper).filename()) == "(Topper) clip.mp4"s);
      CHECK(PathToUTF8(entry->GetMedia(TableCatalog::MediaType::Other).filename()) == "(Other) alt.apng"s);
   }

   SUBCASE("animated images are detected by their content")
   {
      CHECK(TableCatalog::IsAnimatedImageFile(folderA / "medias"sv / "(Wheel) animated.png"s));
      CHECK(!TableCatalog::IsAnimatedImageFile(folderA / "medias"sv / "(Wheel) Attack From Mars.png"s));
      CHECK(!TableCatalog::IsAnimatedImageFile(folderA / "medias"sv / "(Other) alt.apng"s)); // empty file: no animation chunks
      CHECK(!TableCatalog::IsAnimatedImageFile(folderA / "medias"sv / "(Playfield) clip.mp4"s));
      CHECK(TableCatalog::IsAnimatedMediaFile(folderA / "medias"sv / "(Playfield) clip.mp4"s)); // videos count as animated medias
      CHECK(!TableCatalog::IsAnimatedMediaFile(folderA / "medias"sv / "(Wheel) Attack From Mars.png"s));
   }

   SUBCASE("info file named after the table file is found")
   {
      const TableCatalog::Entry* entry = findByStem("Medieval Madness"s);
      REQUIRE(entry != nullptr);
      CHECK(entry->hasInfoFile);
      CHECK(PathToUTF8(entry->infoPath.filename()) == "Medieval Madness.info"s);
      CHECK(entry->info.year == 1997); // year given as a string still parses
   }

   SUBCASE("tables without info file get the default companion path")
   {
      const TableCatalog::Entry* entry = findByStem("Congo"s);
      REQUIRE(entry != nullptr);
      CHECK(!entry->hasInfoFile);
      CHECK(entry->title == "Congo"s);
      CHECK(PathToUTF8(entry->infoPath.filename()) == "Congo.info"s);
   }

   SUBCASE("folder and archive VPZ packs are discovered")
   {
      const TableCatalog::Entry* folderPack = findByStem("Funhouse Pack"s);
      REQUIRE(folderPack != nullptr);
      CHECK(folderPack->isPack);
      const TableCatalog::Entry* zipPack = findByStem("The Addams Family"s);
      REQUIRE(zipPack != nullptr);
      CHECK(zipPack->isPack);
   }

   SUBCASE("edited info file round-trips and preserves other sections")
   {
      const TableCatalog::Entry* entry = findByStem("Attack From Mars v1.0"s);
      REQUIRE(entry != nullptr);
      TableCatalog::Entry edited = *entry;
      edited.info.title = "Attack From Mars (Remastered)"s;
      edited.info.description = "Edited description"s;
      edited.info.authors = { "New Author"s };
      edited.info.manufacturer = "Midway"s;
      edited.info.year = 1996;
      edited.user.rating = 7;
      edited.user.favorite = false;
      edited.user.tags = { "Edited"s };
      REQUIRE(TableCatalog::SaveInfoFile(edited));

      TableCatalog::Entry reloaded;
      reloaded.infoPath = entry->infoPath;
      reloaded.hasInfoFile = true;
      TableCatalog::ReadInfoFile(reloaded);
      CHECK(reloaded.info.title == "Attack From Mars (Remastered)"s);
      CHECK(reloaded.info.description == "Edited description"s);
      CHECK(reloaded.info.authors.size() == 1);
      CHECK(reloaded.info.manufacturer == "Midway"s);
      CHECK(reloaded.info.year == 1996);
      CHECK(reloaded.info.vpsId == "afm_113"s);
      CHECK(reloaded.user.rating == 7);
      CHECK(!reloaded.user.favorite);
      CHECK(reloaded.user.tags.size() == 1);
      // Engine maintained stats are preserved, not written back
      CHECK(reloaded.vpx.version == "1.0"s);
      CHECK(reloaded.vpx.startCount == 12);
      CHECK(reloaded.user.startCount == 42);
      CHECK(reloaded.user.lastRun == "2024-05-01T10:00:00Z"s);
   }

   SUBCASE("info file is created when missing")
   {
      const TableCatalog::Entry* entry = findByStem("Congo"s);
      REQUIRE(entry != nullptr);
      REQUIRE(!entry->hasInfoFile);
      TableCatalog::Entry edited = *entry;
      edited.info.title = "Congo (Williams 1995)"s;
      REQUIRE(TableCatalog::SaveInfoFile(edited));
      TableCatalog::Entry reloaded;
      reloaded.infoPath = entry->infoPath;
      TableCatalog::ReadInfoFile(reloaded);
      CHECK(reloaded.info.title == "Congo (Williams 1995)"s);
   }

   SUBCASE("path scanning reports incremental entry counts and cache folders")
   {
      writeFile(root / "nested"sv / "cache"sv / "Congo"sv / "used_textures.xml"s, "<textures/>"s);
      TableCatalog pathCatalog;
      vector<std::filesystem::path> cacheFolders;
      size_t callbacks = 0;
      size_t lastCount = 0;
      pathCatalog.ScanPaths(
         { root },
         [&](const size_t count)
         {
            ++callbacks;
            CHECK(count >= lastCount);
            lastCount = count;
         },
         &cacheFolders);
      CHECK(callbacks > 0);
      CHECK(lastCount == pathCatalog.GetEntries().size());
      REQUIRE(cacheFolders.size() == 1);
      CHECK(cacheFolders[0] == root / "nested"sv / "cache"sv);
   }

   SUBCASE("catalog cache round-trips scan details and rejects different roots")
   {
      TableCatalog fullCatalog;
      fullCatalog.Scan({ root });
      const std::filesystem::path cachePath = root / "cache"sv / "tables.cache"s;
      const vector<std::filesystem::path> roots { root };
      REQUIRE(fullCatalog.SaveCacheFile(cachePath, roots));

      TableCatalog cached;
      REQUIRE(cached.LoadCacheFile(cachePath, roots));
      REQUIRE(cached.GetEntries().size() == fullCatalog.GetEntries().size());
      for (size_t i = 0; i < cached.GetEntries().size(); ++i)
      {
         const TableCatalog::Entry& actual = cached.GetEntries()[i];
         const TableCatalog::Entry& expected = fullCatalog.GetEntries()[i];
         CHECK(actual.tablePath == expected.tablePath);
         CHECK(actual.detailsLoaded);
         CHECK(actual.title == expected.title);
         CHECK(actual.tableWriteTime == expected.tableWriteTime);
         CHECK(actual.medias[static_cast<size_t>(TableCatalog::MediaType::Wheel)] == expected.medias[static_cast<size_t>(TableCatalog::MediaType::Wheel)]);
      }

      TableCatalog wrongRoots;
      CHECK(!wrongRoots.LoadCacheFile(cachePath, { root / "elsewhere"sv }));
      CHECK(wrongRoots.GetEntries().empty());
   }

   SUBCASE("upsert reuses unchanged details and invalidates modified companions")
   {
      TableCatalog liveCatalog;
      liveCatalog.Scan({ root });
      const TableCatalog::Entry* before = findByStem("Medieval Madness"s);
      REQUIRE(before != nullptr);

      const std::filesystem::path infoPath = root / "Medieval Madness.info"s;
      writeFile(infoPath, R"({ "Info": { "Title": "Medieval Madness", "Manufacturer": "Williams", "Year": 1997 } })");
      std::error_code ec;
      std::filesystem::last_write_time(infoPath, std::filesystem::last_write_time(infoPath, ec) + std::chrono::seconds(2), ec);

      TableCatalog scanned;
      scanned.ScanPaths({ root });
      liveCatalog.UpsertEntries(scanned.GetEntries());
      REQUIRE(liveCatalog.GetEntries().size() == scanned.GetEntries().size());

      const auto findLive = [&liveCatalog](const string& stem) -> const TableCatalog::Entry*
      {
         for (const auto& entry : liveCatalog.GetEntries())
            if (PathToUTF8(entry.tablePath.stem()) == stem)
               return &entry;
         return nullptr;
      };
      const TableCatalog::Entry* unchanged = findLive("Attack From Mars v1.0"s);
      const TableCatalog::Entry* modified = findLive("Medieval Madness"s);
      REQUIRE(unchanged != nullptr);
      REQUIRE(modified != nullptr);
      CHECK(unchanged->detailsLoaded);
      CHECK(unchanged->title == "Attack From Mars"s);
      CHECK(!modified->detailsLoaded);
      CHECK(modified->title == "Medieval Madness"s);
      CHECK(modified->info.manufacturer.empty());

      vector<TableCatalog::Entry> withoutCongo = scanned.GetEntries();
      std::erase_if(withoutCongo, [](const TableCatalog::Entry& entry) { return PathToUTF8(entry.tablePath.stem()) == "Congo"s; });
      liveCatalog.ReconcileEntries(withoutCongo);
      CHECK(findLive("Congo"s) == nullptr);
      CHECK(findLive("Medieval Madness"s) != nullptr);
   }

   SUBCASE("table existence is validated instead of trusting catalog entries")
   {
      const TableCatalog::Entry* entry = findByStem("Medieval Madness"s);
      REQUIRE(entry != nullptr);
      CHECK(TableCatalog::TableExists(*entry));
      TableCatalog::Entry stale = *entry;
      stale.tablePath = root / "missing.vpx"s;
      CHECK(!TableCatalog::TableExists(stale));
   }

   SUBCASE("stale texture cache folders are removed only when enabled")
   {
      const std::filesystem::path valid = folderA / "cache"sv / "Attack From Mars v1.0"sv;
      const std::filesystem::path stale = folderA / "cache"sv / "Renamed Table"sv;
      writeFile(valid / "textures"sv / "texture.bin"s, "valid"s);
      writeFile(stale / "textures"sv / "texture.bin"s, "stale"s);

      TableCatalog::TextureCacheCleanupResult kept = TableCatalog::CleanupTextureCaches({ root }, entries, false, -1);
      CHECK(kept.staleRemoved == 0);
      CHECK(std::filesystem::exists(stale));

      const TableCatalog::TextureCacheCleanupResult cleaned = TableCatalog::CleanupTextureCaches({ root }, entries, true, -1);
      CHECK(cleaned.staleRemoved == 1);
      CHECK(cleaned.failedRemovals == 0);
      CHECK(std::filesystem::exists(valid));
      CHECK(!std::filesystem::exists(stale));
   }

   SUBCASE("texture cache limit keeps the most recently used tables")
   {
      const std::filesystem::path afm = folderA / "cache"sv / "Attack From Mars v1.0"sv;
      const std::filesystem::path mm = root / "cache"sv / "Medieval Madness"sv;
      const std::filesystem::path taf = root / "cache"sv / "The Addams Family"sv;
      writeFile(afm / "last_played"s, "1"s);
      writeFile(mm / "last_played"s, "2"s);
      writeFile(taf / "last_played"s, "3"s);
      const auto base = std::filesystem::file_time_type::clock::now();
      std::error_code ec;
      std::filesystem::last_write_time(afm / "last_played"s, base + std::chrono::seconds(1), ec);
      std::filesystem::last_write_time(mm / "last_played"s, base + std::chrono::seconds(2), ec);
      std::filesystem::last_write_time(taf / "last_played"s, base + std::chrono::seconds(3), ec);

      const TableCatalog::TextureCacheCleanupResult result = TableCatalog::CleanupTextureCaches({ root }, entries, true, 2);
      CHECK(result.overflowRemoved == 1);
      CHECK(result.failedRemovals == 0);
      CHECK(!std::filesystem::exists(afm));
      CHECK(std::filesystem::exists(mm));
      CHECK(std::filesystem::exists(taf));

      const TableCatalog::TextureCacheCleanupResult none = TableCatalog::CleanupTextureCaches({ root }, entries, true, 0);
      CHECK(none.overflowRemoved == 2);
      CHECK(!std::filesystem::exists(mm));
      CHECK(!std::filesystem::exists(taf));
   }
}
