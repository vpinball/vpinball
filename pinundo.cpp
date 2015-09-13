#include "StdAfx.h"

PinUndo::PinUndo()
{
   m_cUndoLayer = 0;
   m_sdsDirty = eSaveClean;
   m_cleanpoint = 0;
}

PinUndo::~PinUndo()
{
   for (int i = 0; i < m_vur.Size(); ++i)
   {
      delete m_vur.ElementAt(i);
   }
   m_vur.RemoveAllElements();
}

void PinUndo::SetCleanPoint(SaveDirtyState sds)
{
   if (sds == eSaveClean)
   {
      m_cleanpoint = m_vur.Size();
   }
   m_sdsDirty = sds;
   m_ptable->SetDirty(sds);
}

void PinUndo::BeginUndo()
{
   m_cUndoLayer++;

   if (m_cUndoLayer == 1)
   {
      if (m_vur.Size() == MAXUNDO)
      {
         delete m_vur.ElementAt(0);
         m_vur.RemoveElementAt(0);
         m_cleanpoint--;
      }

      UndoRecord * const pur = new UndoRecord();

      m_vur.AddElement(pur);
   }
}

void PinUndo::MarkForUndo(IEditable *pie)
{
   if (m_vur.Size() == 0)
   {
      _ASSERTE(fFalse);
      return;
   }

   if (m_cUndoLayer <= 0)
   {
      _ASSERTE(fFalse);
      BeginUndo();
   }

   UndoRecord * const pur = m_vur.ElementAt(m_vur.Size() - 1);

   pur->MarkForUndo(pie);
}

void PinUndo::MarkForCreate(IEditable *pie)
{
   if (m_vur.Size() == 0)
   {
      _ASSERTE(fFalse);
      return;
   }

   if (m_cUndoLayer <= 0)
   {
      _ASSERTE(fFalse);
      BeginUndo();
   }

   UndoRecord * const pur = m_vur.ElementAt(m_vur.Size() - 1);

   pur->MarkForCreate(pie);
}

void PinUndo::MarkForDelete(IEditable *pie)
{
   if (m_vur.Size() == 0)
   {
      _ASSERTE(fFalse);
      return;
   }

   if (m_cUndoLayer <= 0)
   {
      _ASSERTE(fFalse);
      BeginUndo();
   }

   UndoRecord * const pur = m_vur.ElementAt(m_vur.Size() - 1);

   pur->MarkForDelete(pie);
}

void PinUndo::Undo()
{
   if (m_vur.Size() == 0)
   {
      //_ASSERTE(fFalse);
      return;
   }

   if (m_cUndoLayer > 0)
   {
      _ASSERTE(fFalse);
      m_cUndoLayer = 0;
   }

   if (m_vur.Size() == m_cleanpoint)
   {
      LocalString ls(IDS_UNDOPASTSAVE);
      const int result = MessageBox(m_ptable->m_hwnd, ls.m_szbuffer, "Visual Pinball", MB_YESNO);
      if (result != IDYES)
      {
         return;
      }
   }

   UndoRecord * const pur = m_vur.ElementAt(m_vur.Size() - 1);
   //IStorage *pstg = pur->m_pstg;

   for (int i = 0; i < pur->m_vieDelete.Size(); i++)
   {
      m_ptable->Undelete(pur->m_vieDelete.ElementAt(i));
   }

   pur->m_vieDelete.RemoveAllElements(); // Don't want these released when this record gets deleted

   for (int i = 0; i < pur->m_vstm.Size(); i++)
   {
      IStream * const pstm = pur->m_vstm.ElementAt(i);

      // Go back to beginning of stream to load
      LARGE_INTEGER foo;
      foo.QuadPart = 0;
      pstm->Seek(foo, STREAM_SEEK_SET, NULL);

      DWORD read;
      IEditable *pie;
      pstm->Read(&pie, sizeof(IEditable *), &read);

      pie->ClearForOverwrite();

      int foo2;
      pie->InitLoad(pstm, m_ptable, &foo2, CURRENT_FILE_FORMAT_VERSION, NULL, NULL);

      // Stream gets release when undo record is deleted
      //pstm->Release();
   }

   for (int i = 0; i<pur->m_vieCreate.Size(); i++)
   {
      m_ptable->Uncreate(pur->m_vieCreate.ElementAt(i));
   }

   m_vur.RemoveElement(pur);
   delete pur;

   if ((m_vur.Size() == m_cleanpoint) && (m_sdsDirty > eSaveClean)) // UNDONE - how could we get here without m_fDirty being true?
   {
      m_sdsDirty = eSaveClean;
      m_ptable->SetDirty(eSaveClean);
   }
   else if (/*m_vur.Size() < m_cleanpoint && */(m_sdsDirty < eSaveDirty))
   {
      // If we're not clean, we must be dirty (always process this case for autosave, even though to the user it wouldn't appear to matter)
      if (m_vur.Size() < m_cleanpoint)
      {
         m_cleanpoint = -1; // Can't redo yet
      }
      m_sdsDirty = eSaveDirty;
      m_ptable->SetDirty(eSaveDirty);
   }
}

void PinUndo::EndUndo()
{
   _ASSERTE(m_cUndoLayer > 0);
   if (m_cUndoLayer > 0)
   {
      m_cUndoLayer--;
   }

   if (m_cUndoLayer == 0 && (m_sdsDirty < eSaveDirty))
   {
      m_sdsDirty = eSaveDirty;
      m_ptable->SetDirty(eSaveDirty);
   }
}

UndoRecord::UndoRecord()
{
}

UndoRecord::~UndoRecord()
{
   for (int i = 0; i < m_vstm.Size(); i++)
   {
      m_vstm.ElementAt(i)->Release();
   }

   for (int i = 0; i < m_vieDelete.Size(); i++)
   {
      m_vieDelete.ElementAt(i)->Release();
   }
}

void UndoRecord::MarkForUndo(IEditable *pie)
{
   if (m_vieMark.IndexOf(pie) != -1) // Been marked already
   {
      return;
   }

   if (m_vieCreate.IndexOf(pie) != -1) // Just created, so undo will delete it anyway
   {
      return;
   }

   m_vieMark.AddElement(pie);

   FastIStream * const pstm = new FastIStream();
   pstm->AddRef();

   DWORD write;
   pstm->Write(&pie, sizeof(IEditable *), &write);

   pie->SaveData(pstm, NULL, NULL);

   m_vstm.AddElement(pstm);
}

void UndoRecord::MarkForCreate(IEditable *pie)
{
#ifdef _DEBUG
   if (m_vieCreate.IndexOf(pie) != -1) // Created twice?
   {
      _ASSERTE(fFalse);
      return;
   }
#endif

   m_vieCreate.AddElement(pie);
}

void UndoRecord::MarkForDelete(IEditable *pie)
{
#ifdef _DEBUG
   if (m_vieDelete.IndexOf(pie) != -1) // Already deleted - bad thing
   {
      _ASSERTE(fFalse);
      return;
   }
#endif

   if (m_vieCreate.IndexOf(pie) != -1) // Created and deleted in the same step
   {
      // Just forget about it
      m_vieCreate.RemoveElement(pie);
      m_vieMark.RemoveElement(pie);
      return;
   }

   m_vieDelete.AddElement(pie);
}
