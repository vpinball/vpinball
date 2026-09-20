// license:GPLv3+
#pragma once

#include <any>
#include <functional>

class IEditable;
class PinTable;

class PinUndo final
{
public:
   PinUndo(PinTable* table);
   ~PinUndo();

   void BeginUndo();
   void MarkForUndo(IEditable * const pie, const bool saveForUndo = false);
   void MarkForCreate(IEditable *const pie);
   void MarkForDelete(IEditable *const pie);
   void EndUndo();
   // Undoes the last record, returning the editor state captured when it was begun (empty if none)
   std::any Undo();
   void Discard();

   bool HasUndo() const;
   bool IsUndoPastCleanPoint() const;
   void SetCleanPoint(const SaveDirtyState sds);

   // Registers the editor state capture callback: it is evaluated each time a new undo record is begun,
   // the returned state is stored in the record and given back by Undo() (e.g. to restore the selection)
   void SetEditorStateCapture(const std::function<std::any()> &capture) { m_editorStateCapture = capture; }

private:
   PinTable *const m_table;
   vector<std::unique_ptr<class UndoRecord>> m_undoRecords;
   int m_nUndoLayer = 0;
   SaveDirtyState m_dirtyState = eSaveClean;
   size_t m_cleanpoint = 0; // Undo record at which table is in a non-dirty state (if bigger than undo records size, clean state can not be reached)
   std::function<std::any()> m_editorStateCapture;
};
