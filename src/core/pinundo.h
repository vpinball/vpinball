// license:GPLv3+
#pragma once

#include <any>
#include <functional>

class IEditable;
class PinTable;
class UndoRecord;

class PinUndo final
{
public:
   PinUndo(PinTable* table);
   ~PinUndo();

   void BeginUndo();
   void MarkForUndo(IEditable * const pie, const bool saveForUndo = true);
   void MarkForCreate(IEditable *const pie);
   void MarkForDelete(IEditable *const pie);
   // Records a change that is not a part edit (asset, script, table property): the callback restores the previous state when the record is undone
   // undo restores the state from before the change. snapshotForRedo (optional) makes the record redoable: it is called right before undo, and
   // returns the function restoring the state as it is at that time (which becomes the redo of this action, with the same symmetry on redo).
   // A record with an action without snapshotForRedo can not be redone: undoing it clears the redo records.
   void MarkForAction(std::function<void()> undo, std::function<std::function<void()>()> snapshotForRedo = nullptr);
   void EndUndo();
   // Undoes the last record, returning the editor state captured when it was begun (empty if none). The undone record can be redone
   // (until a new record is begun) unless keepForRedo is false, which is used to cancel the record that has just been begun
   std::any Undo(bool keepForRedo = true);
   // Redoes the last undone record, returning the editor state captured when it was undone (empty if none)
   std::any Redo();
   void Discard();

   // History labels for the undo history (index 0: the next record to undo / redo, then going away from it)
   void SetLabel(const string &label); // Complete label of the last record (being recorded, or just ended) (e.g. "Move 3 parts")
   void SetFieldLabel(const string &field); // Edited field, shown after the name of the part(s) of the record (e.g. "Rim Width: PBR_DOME_f125")
   size_t UndoCount() const;
   size_t RedoCount() const;
   string UndoLabel(size_t index) const;
   string RedoLabel(size_t index) const;

   bool HasUndo() const;
   bool HasRedo() const;
   // True if undoing the next record clears the redo records (it holds actions that can not snapshot their state). A multi-step undo stops before such a record.
   bool NextUndoClearsRedo() const;
   // Unique id of the record that would be undone next (0 if none): changes whenever a record is pushed, undone, redone or dropped, so
   // that the editor can tell that the history moved since it noted the id (discarded records leave no trace)
   uint64_t TopRecordId() const;
   // Folds the last record into the one before it, so that a single undo reverts both (e.g. paste then grab of a duplicate). Only for a last
   // record made of part edits. Returns false (nothing done) otherwise.
   bool MergeLastIntoPrevious();
   bool IsUndoPastCleanPoint() const;
   void SetCleanPoint(const SaveDirtyState sds);

   // Registers the editor state capture callback: it is evaluated each time a new undo record is begun,
   // the returned state is stored in the record and given back by Undo() (e.g. to restore the selection)
   void SetEditorStateCapture(const std::function<std::any()> &capture) { m_editorStateCapture = capture; }

private:
   static constexpr size_t NoCleanPoint = static_cast<size_t>(-1);
   std::any ApplyRecord(UndoRecord &record, bool buildInverse, std::unique_ptr<UndoRecord> &inverse);
   void ClearRedo();
   static bool IsInvertible(const UndoRecord &record);
   void UpdateDirtyAfterMove();

   PinTable *const m_table;
   vector<std::unique_ptr<UndoRecord>> m_undoRecords;
   vector<std::unique_ptr<UndoRecord>> m_redoRecords; // Undone records (the last one is the next to redo)
   uint64_t m_lastRecordId = 0;
   int m_nUndoLayer = 0;
   SaveDirtyState m_dirtyState = SaveDirtyState::Clean;
   size_t m_cleanpoint = 0; // Undo record at which table is in a non-dirty state (if bigger than undo records size, clean state can not be reached)
   std::function<std::any()> m_editorStateCapture;
};
