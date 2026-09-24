// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "core/pinundo.h"
#include "parts/pintable.h"
#include "parts/bumper.h"

#include "doctest.h"

TEST_CASE("PinUndo")
{
   PinTable* const table = CreateTestTable();

   SUBCASE("empty stack has nothing to undo")
   {
      PinUndo undo(table);
      CHECK_FALSE(undo.HasUndo());
      CHECK_FALSE(undo.Undo().has_value());
   }

   SUBCASE("begin/end undo pushes a record")
   {
      PinUndo undo(table);
      undo.BeginUndo();
      undo.EndUndo();
      CHECK(undo.HasUndo());
   }

   SUBCASE("nested begin calls produce a single record")
   {
      PinUndo undo(table);
      undo.BeginUndo();
      undo.BeginUndo();
      undo.EndUndo();
      undo.EndUndo();
      undo.Undo();
      CHECK_FALSE(undo.HasUndo());
   }

   SUBCASE("editor state is captured on begin and returned by undo")
   {
      PinUndo undo(table);
      undo.SetEditorStateCapture([]() -> std::any { return 42; });
      undo.BeginUndo();
      undo.EndUndo();
      const std::any state = undo.Undo();
      REQUIRE(state.has_value());
      CHECK(std::any_cast<int>(state) == 42);
      CHECK_FALSE(undo.HasUndo());
   }

   SUBCASE("discard drops the last record without undoing it")
   {
      PinUndo undo(table);
      undo.BeginUndo();
      undo.EndUndo();
      undo.BeginUndo();
      undo.EndUndo();
      undo.Discard();
      undo.Undo();
      CHECK_FALSE(undo.HasUndo());
   }

   SUBCASE("clean point tracking")
   {
      PinUndo undo(table);
      undo.SetCleanPoint(eSaveClean);
      undo.BeginUndo();
      undo.EndUndo();
      CHECK(table->FDirty()); // ending an undo record marks the table dirty
      CHECK_FALSE(undo.IsUndoPastCleanPoint());
      undo.SetCleanPoint(eSaveClean);
      CHECK(undo.IsUndoPastCleanPoint()); // record count matches the clean point again
      CHECK_FALSE(table->FDirty());
   }

   SUBCASE("undoing a deletion restores the part")
   {
      PinUndo undo(table);
      Bumper* const bumper = Bumper::COMCreate();
      bumper->Init(100.f, 200.f, false);
      bumper->SetName(L"UndoDeleteBumper");
      table->AddPart(bumper);
      bumper->Release();

      undo.BeginUndo();
      undo.MarkForDelete(bumper);
      table->RemovePart(bumper);
      undo.EndUndo();
      CHECK_FALSE(table->HasPart(bumper));

      undo.Undo();
      CHECK(table->HasPart(bumper));
   }

   SUBCASE("undoing a creation removes the part")
   {
      PinUndo undo(table);
      Bumper* const bumper = Bumper::COMCreate();
      bumper->Init(100.f, 200.f, false);
      bumper->SetName(L"UndoCreateBumper");
      table->AddPart(bumper);
      // Keep the creation reference: Uncreate releases both the table's and the creator's

      undo.BeginUndo();
      undo.MarkForCreate(bumper);
      undo.EndUndo();

      undo.Undo();
      CHECK_FALSE(table->HasPart(bumper)); // the part was removed and fully released
   }

   SUBCASE("undoing a marked change restores the saved state")
   {
      PinUndo undo(table);
      Bumper* const bumper = Bumper::COMCreate();
      bumper->Init(100.f, 200.f, false);
      bumper->SetName(L"UndoMarkBumper");
      bumper->m_d.m_radius = 45.f;
      table->AddPart(bumper);
      bumper->Release();

      undo.BeginUndo();
      undo.MarkForUndo(bumper, false);
      bumper->m_d.m_radius = 99.f;
      undo.EndUndo();
      CHECK(bumper->m_d.m_radius == 99.f);

      undo.Undo();
      CHECK(bumper->m_d.m_radius == 45.f);
   }

   SUBCASE("undo stack is capped")
   {
      PinUndo undo(table);
      for (int i = 0; i < 20; i++) // MAXUNDO is 16
      {
         undo.BeginUndo();
         undo.EndUndo();
      }
      int popped = 0;
      while (undo.HasUndo())
      {
         undo.Undo();
         popped++;
      }
      CHECK(popped == 16);
   }

   table->Release();
}
