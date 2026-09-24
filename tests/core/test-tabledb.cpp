// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "core/TableDB.h"

#include "doctest.h"

TEST_CASE("TableDB")
{
   TableDB db;

   SUBCASE("empty database has no match") { CHECK(db.GetBestSizeMatch(INCHESTOVPU(20.25f), INCHESTOVPU(42.f), INCHESTOVPU(15.f)) == -1); }

   // Entry fields: name, width, height, glassBottom, glassTop, comment, minYear, maxYear (sizes in inches)
   SUBCASE("exact and nearest match")
   {
      db.m_data = { { "Widebody"s, 23.25f, 46.f, 2.375f, 15.f, ""s, 0, 9999 }, { "Standard"s, 20.25f, 42.f, 2.375f, 15.f, ""s, 0, 9999 } };

      CHECK(db.GetBestSizeMatch(INCHESTOVPU(23.25f), INCHESTOVPU(46.f), INCHESTOVPU(15.f)) == 0);
      CHECK(db.GetBestSizeMatch(INCHESTOVPU(20.25f), INCHESTOVPU(42.f), INCHESTOVPU(15.f)) == 1);
      // A bit off still resolves to the closest entry
      CHECK(db.GetBestSizeMatch(INCHESTOVPU(20.5f), INCHESTOVPU(42.5f), INCHESTOVPU(15.f)) == 1);
   }

   SUBCASE("year hint penalizes out of range entries")
   {
      db.m_data = { { "Old"s, 20.25f, 42.f, 2.375f, 15.f, ""s, 1970, 1985 }, { "New"s, 21.25f, 43.f, 2.375f, 15.f, ""s, 1990, 2010 } };

      // The query is slightly closer to 'Old', but the x10 penalty for out of range years flips the ranking
      const float w = INCHESTOVPU(20.7f), h = INCHESTOVPU(42.5f), top = INCHESTOVPU(15.f);
      CHECK(db.GetBestSizeMatch(w, h, top, -1.f, 1980) == 0);
      CHECK(db.GetBestSizeMatch(w, h, top, -1.f, 2000) == 1);
      // No hint or out of range hint: the closest entry wins
      CHECK(db.GetBestSizeMatch(w, h, top) == 0);
      CHECK(db.GetBestSizeMatch(w, h, top, -1.f, 1899) == 0);
   }

   SUBCASE("bottom glass height is only used when provided")
   {
      db.m_data = { { "A"s, 20.25f, 42.f, 2.375f, 15.f, ""s, 0, 9999 }, { "B"s, 20.25f, 42.f, 8.f, 15.f, ""s, 0, 9999 } };

      const float w = INCHESTOVPU(20.25f), h = INCHESTOVPU(42.f), top = INCHESTOVPU(15.f), bottom = INCHESTOVPU(8.f);
      CHECK(db.GetBestSizeMatch(w, h, top, bottom) == 1);
      CHECK(db.GetBestSizeMatch(w, h, top) == 0); // bottom glass ignored: first entry wins the tie
   }
}
