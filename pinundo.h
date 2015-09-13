// PinUndo.h: interface for the PinUndo class.
//
//////////////////////////////////////////////////////////////////////
#pragma once
#if !defined(AFX_PINUNDO_H__F1136F22_51FB_4AC8_B7FC_89A5E148DD7B__INCLUDED_)
#define AFX_PINUNDO_H__F1136F22_51FB_4AC8_B7FC_89A5E148DD7B__INCLUDED_

#define MAXUNDO 16

class IEditable;
class PinTable;

class UndoRecord
{
public:
   UndoRecord();
   virtual ~UndoRecord();

   void MarkForUndo(IEditable *pie);
   void MarkForCreate(IEditable *pie);
   void MarkForDelete(IEditable *pie);

   //IStorage *m_pstg;
   Vector<FastIStream> m_vstm;
   Vector<IEditable> m_vieMark;
   Vector<IEditable> m_vieCreate;
   Vector<IEditable> m_vieDelete;
};

class PinUndo
{
public:
   PinUndo();
   virtual ~PinUndo();

   Vector<UndoRecord> m_vur;

   void BeginUndo();
   void MarkForUndo(IEditable *pie);
   void MarkForCreate(IEditable *pie);
   void MarkForDelete(IEditable *pie);
   void EndUndo();
   void Undo();

   void SetCleanPoint(SaveDirtyState sds);

   int m_cUndoLayer;

   SaveDirtyState m_sdsDirty; // Dirty flag for saving on close

   PinTable *m_ptable;

   int m_cleanpoint; // Undo record at which table is in a non-dirty state.  When negative, clean state can not be reached
};

#endif // !defined(AFX_PINUNDO_H__F1136F22_51FB_4AC8_B7FC_89A5E148DD7B__INCLUDED_)
