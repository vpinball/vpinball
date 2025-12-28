// license:GPLv3+

// interface for the PinUndo class.

#pragma once

#define MAXUNDO 16

class IEditable;
class PinTable;

class UndoRecord final
{
public:
   UndoRecord();
   ~UndoRecord();

   void MarkForUndo(IEditable *const pie, const bool saveForUndo);
   void MarkForCreate(IEditable *const pie);
   void MarkForDelete(IEditable *const pie);

   vector<FastIStream*> m_vstm;
   vector<IEditable*> m_vieCreate;
   vector<IEditable*> m_vieDelete;

private:
   vector<IEditable*> m_vieMark;
};

class PinUndo final
{
public:
   PinUndo(PinTable* table);

   void BeginUndo();
   void MarkForUndo(IEditable * const pie, const bool saveForUndo = false);
   void MarkForCreate(IEditable *const pie);
   void MarkForDelete(IEditable *const pie);
   void EndUndo();
   void Undo(bool discard = false);

   void SetCleanPoint(const SaveDirtyState sds);

private:
   PinTable *const m_table;
   vector<std::unique_ptr<UndoRecord>> m_undoRecords;
   int m_cUndoLayer = 0;
   SaveDirtyState m_sdsDirty = eSaveClean; // Dirty flag for saving on close
   size_t m_cleanpoint = 0; // Undo record at which table is in a non-dirty state (if bigger than undo records size, clean state can not be reached)
};
