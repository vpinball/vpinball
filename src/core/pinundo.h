// license:GPLv3+
#pragma once

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
   void Undo();
   void Discard();

   bool HasUndo() const;
   bool IsUndoPastCleanPoint() const;
   void SetCleanPoint(const SaveDirtyState sds);

private:
   PinTable *const m_table;
   vector<std::unique_ptr<class UndoRecord>> m_undoRecords;
   int m_nUndoLayer = 0;
   SaveDirtyState m_dirtyState = eSaveClean;
   size_t m_cleanpoint = 0; // Undo record at which table is in a non-dirty state (if bigger than undo records size, clean state can not be reached)
};
