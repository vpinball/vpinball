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
      undo.SetCleanPoint(SaveDirtyState::Clean);
      undo.BeginUndo();
      undo.EndUndo();
      CHECK(table->GetDirtyState() != SaveDirtyState::Clean); // ending an undo record marks the table dirty
      CHECK_FALSE(undo.IsUndoPastCleanPoint());
      undo.SetCleanPoint(SaveDirtyState::Clean);
      CHECK(undo.IsUndoPastCleanPoint()); // record count matches the clean point again
      CHECK_FALSE(table->GetDirtyState() != SaveDirtyState::Clean);
   }

   SUBCASE("undoing a deletion restores the part")
   {
      PinUndo undo(table);
      Bumper* const bumper = Bumper::COMCreate();
      bumper->Init(100.f, 200.f, false);
      bumper->SetName("UndoDeleteBumper");
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
      bumper->SetName("UndoCreateBumper");
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
      bumper->SetName("UndoMarkBumper");
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
      for (int i = 0; i < 20; i++)
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
      CHECK(popped == 17); // The history keeps MAXUNDO (16) records, plus the slot of the last record begun
   }

   SUBCASE("redo reapplies an undone change")
   {
      PinUndo undo(table);
      Bumper* const bumper = Bumper::COMCreate();
      bumper->Init(100.f, 200.f, false);
      bumper->SetName("RedoBumper");
      bumper->m_d.m_radius = 45.f;
      table->AddPart(bumper);
      bumper->Release();

      undo.BeginUndo();
      undo.MarkForUndo(bumper, false);
      bumper->m_d.m_radius = 99.f;
      undo.EndUndo();
      CHECK_FALSE(undo.HasRedo());

      undo.Undo();
      CHECK(bumper->m_d.m_radius == 45.f);
      CHECK(undo.HasRedo());

      undo.Redo();
      CHECK(bumper->m_d.m_radius == 99.f);
      CHECK_FALSE(undo.HasRedo());
      CHECK(undo.HasUndo());
   }

   SUBCASE("redoing a deletion deletes the part again")
   {
      PinUndo undo(table);
      Bumper* const bumper = Bumper::COMCreate();
      bumper->Init(100.f, 200.f, false);
      bumper->SetName("RedoDeleteBumper");
      table->AddPart(bumper);
      bumper->Release();

      undo.BeginUndo();
      undo.MarkForDelete(bumper);
      table->RemovePart(bumper);
      undo.EndUndo();

      undo.Undo();
      CHECK(table->HasPart(bumper));
      undo.Redo();
      CHECK_FALSE(table->HasPart(bumper));
      undo.Undo(); // Clean up: leave the part in the table for the next subcase
      CHECK(table->HasPart(bumper));
   }

   SUBCASE("a new record begun clears the redo stack")
   {
      PinUndo undo(table);
      undo.BeginUndo();
      undo.EndUndo();
      undo.Undo();
      CHECK(undo.HasRedo());
      undo.BeginUndo();
      undo.EndUndo();
      CHECK_FALSE(undo.HasRedo());
   }

   SUBCASE("discard restores the redo stack stashed by the begun record")
   {
      PinUndo undo(table);
      undo.BeginUndo();
      undo.EndUndo();
      undo.Undo();
      CHECK(undo.HasRedo());
      undo.BeginUndo();
      undo.EndUndo();
      undo.Discard();
      CHECK(undo.HasRedo()); // The discarded record leaves no trace and its stash is restored
      undo.Redo();
      CHECK_FALSE(undo.HasRedo());
   }

   SUBCASE("undoing a non invertible action clears the redo stack")
   {
      PinUndo undo(table);
      int value = 0;
      undo.BeginUndo();
      undo.MarkForAction([&value]() { value = 1; }); // No snapshotForRedo: not redoable
      undo.EndUndo();
      undo.BeginUndo();
      undo.EndUndo();
      undo.Undo();
      CHECK(undo.HasRedo());
      undo.Undo(); // Applies the non invertible record
      CHECK_FALSE(undo.HasRedo());
   }

   SUBCASE("undoing an action with a redo snapshot is redoable")
   {
      PinUndo undo(table);
      int value = 0;
      undo.BeginUndo();
      // The snapshot is evaluated right before the undo: it returns the function restoring the state as it is at that time
      undo.MarkForAction([&value]() { value = 1; },
         [&value]()
         {
            const int was = value;
            return std::function<void()>([&value, was]() { value = was; });
         });
      undo.EndUndo();
      value = 5;
      undo.Undo();
      CHECK(value == 1);
      undo.Redo();
      CHECK(value == 5); // The snapshot taken at undo time is restored
   }

   SUBCASE("record labels")
   {
      PinUndo undo(table);
      Bumper* const bumper = Bumper::COMCreate();
      bumper->Init(100.f, 200.f, false);
      bumper->SetName("LabelBumper");
      table->AddPart(bumper);
      bumper->Release();

      undo.BeginUndo();
      undo.MarkForUndo(bumper, false);
      undo.EndUndo();
      CHECK(undo.UndoLabel(0) == "LabelBumper"); // Subject name of the only part of the record

      undo.SetFieldLabel("Radius");
      CHECK(undo.UndoLabel(0) == "Radius: LabelBumper");

      undo.SetLabel("Move bumper");
      CHECK(undo.UndoLabel(0) == "Move bumper"); // A full label wins over the field/subject composition

      undo.Undo();
      CHECK(undo.RedoLabel(0) == "Move bumper"); // The redo record keeps the label
   }

   SUBCASE("merge folds the last record into the previous one")
   {
      PinUndo undo(table);
      Bumper* const bumper = Bumper::COMCreate();
      bumper->Init(100.f, 200.f, false);
      bumper->SetName("MergeBumper");
      table->AddPart(bumper);
      bumper->Release();
      const float originalRadius = bumper->m_d.m_radius;

      undo.BeginUndo();
      undo.MarkForUndo(bumper, false);
      bumper->m_d.m_radius = 50.f;
      undo.EndUndo();
      undo.BeginUndo();
      undo.MarkForUndo(bumper, false);
      bumper->m_d.m_radius = 60.f;
      undo.EndUndo();
      const size_t countBefore = undo.UndoCount();
      CHECK(undo.MergeLastIntoPrevious());
      CHECK(undo.UndoCount() == countBefore - 1);
      undo.Undo(); // A single undo reverts both edits
      CHECK(bumper->m_d.m_radius == originalRadius);
   }

   SUBCASE("top record id changes with the history")
   {
      PinUndo undo(table);
      CHECK(undo.TopRecordId() == 0);
      undo.BeginUndo();
      undo.EndUndo();
      const uint64_t id = undo.TopRecordId();
      CHECK(id != 0);
      undo.Undo();
      CHECK(undo.TopRecordId() == 0);
      undo.Redo();
      CHECK(undo.TopRecordId() != id); // The record pushed by the redo is a new record
   }

   table->Release();
}
