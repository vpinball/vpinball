// license:GPLv3+

// interface for the PinUndo class.

#pragma once

#define MAXUNDO 16

class IEditable;
class PinTable;

class UndoRecord
{
public:
   UndoRecord();
   virtual ~UndoRecord();

   void MarkForUndo(IEditable *const pie, const bool saveForUndo);
   void MarkForCreate(IEditable *const pie);
   void MarkForDelete(IEditable *const pie);

   vector<FastIStream*> m_vstm;
   vector<IEditable*> m_vieCreate;
   vector<IEditable*> m_vieDelete;

private:
   vector<IEditable*> m_vieMark;
};

class PinUndo
{
public:
   PinUndo();
   virtual ~PinUndo();

   void BeginUndo();
   void MarkForUndo(IEditable * const pie, const bool saveForUndo = false);
   void MarkForCreate(IEditable *const pie);
   void MarkForDelete(IEditable *const pie);
   void EndUndo();
   void Undo();

   void SetCleanPoint(const SaveDirtyState sds);

   PinTable *m_ptable;

private:
   vector<UndoRecord*> m_vur;

   int m_cUndoLayer;

   SaveDirtyState m_sdsDirty; // Dirty flag for saving on close

   size_t m_cleanpoint; // Undo record at which table is in a non-dirty state.  When negative, clean state can not be reached
};
