#include "StdAfx.h"
#include "vpversion.h"

PinUndo::PinUndo()
{
   m_cUndoLayer = 0;
   m_sdsDirty = eSaveClean;
   m_cleanpoint = 0;
   m_startToPlay = false;
}

PinUndo::~PinUndo()
{
   for (size_t i = 0; i < m_vur.size(); ++i)
      delete m_vur[i];
}

void PinUndo::SetCleanPoint(const SaveDirtyState sds)
{
   if (sds == eSaveClean)
      m_cleanpoint = m_vur.size();
   m_sdsDirty = sds;
   m_ptable->SetDirty(sds);
}

void PinUndo::BeginUndo()
{ 
   if(g_pplayer)
       return;

   m_cUndoLayer++;

   if (m_cUndoLayer == 1)
   {
      if (m_vur.size() == MAXUNDO)
      {
         delete m_vur[0];
         m_vur.erase(m_vur.begin());
         m_cleanpoint--;
      }

      UndoRecord * const pur = new UndoRecord();

      m_vur.push_back(pur);
   }
}

void PinUndo::MarkForUndo(IEditable * const pie, const bool backupForPlay)
{
   if(g_pplayer)
      return;

   if(m_vur.size() == 0)
   {
      _ASSERTE(fFalse);
      return;
   }

   if (m_cUndoLayer <= 0)
   {
      _ASSERTE(fFalse);
      BeginUndo();
   }

   UndoRecord * const pur = m_vur[m_vur.size() - 1];

   pur->MarkForUndo(pie, backupForPlay);
}

void PinUndo::MarkForCreate(IEditable * const pie)
{
   if (m_vur.size() == 0)
   {
      _ASSERTE(fFalse);
      return;
   }

   if (m_cUndoLayer <= 0)
   {
      _ASSERTE(fFalse);
      BeginUndo();
   }

   UndoRecord * const pur = m_vur[m_vur.size() - 1];

   pur->MarkForCreate(pie);
}

void PinUndo::MarkForDelete(IEditable * const pie)
{
   if (m_vur.size() == 0)
   {
      _ASSERTE(fFalse);
      return;
   }

   if (m_cUndoLayer <= 0)
   {
      _ASSERTE(fFalse);
      BeginUndo();
   }

   UndoRecord * const pur = m_vur[m_vur.size() - 1];

   pur->MarkForDelete(pie);
}

void PinUndo::Undo()
{
   if (m_vur.size() == 0)
   {
      //_ASSERTE(fFalse);
      return;
   }

   if (m_cUndoLayer > 0)
   {
      _ASSERTE(fFalse);
      m_cUndoLayer = 0;
   }

   if (m_vur.size() == m_cleanpoint)
   {
      const LocalString ls(IDS_UNDOPASTSAVE);
      const int result = m_ptable->ShowMessageBox(ls.m_szbuffer);
      if (result != IDYES)
         return;
   }

   UndoRecord * const pur = m_vur[m_vur.size() - 1];

   for (size_t i = 0; i < pur->m_vieDelete.size(); i++)
      m_ptable->Undelete(pur->m_vieDelete[i]);

   pur->m_vieDelete.clear(); // Don't want these released when this record gets deleted

   for (size_t i = 0; i < pur->m_vstm.size(); i++)
   {
      IStream * const pstm = pur->m_vstm[i];

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
      pie->InitPostLoad();
      // Stream gets released when undo record is deleted
      //pstm->Release();
   }

   for (size_t i = 0; i<pur->m_vieCreate.size(); i++)
      m_ptable->Uncreate(pur->m_vieCreate[i]);

   RemoveFromVectorSingle(m_vur, pur);
   delete pur;

   if ((m_vur.size() == m_cleanpoint) && (m_sdsDirty > eSaveClean)) // UNDONE - how could we get here without m_fDirty being true?
   {
      m_sdsDirty = eSaveClean;
      m_ptable->SetDirty(eSaveClean);
   }
   else if (/*m_vur.size() < m_cleanpoint && */(m_sdsDirty < eSaveDirty))
   {
      // If we're not clean, we must be dirty (always process this case for autosave, even though to the user it wouldn't appear to matter)
      if (m_vur.size() < m_cleanpoint)
      {
         m_cleanpoint = -1; // Can't redo yet
      }
      m_sdsDirty = eSaveDirty;
      m_ptable->SetDirty(eSaveDirty);
   }
}

void PinUndo::EndUndo()
{
   if(g_pplayer)
      return;

   _ASSERTE(m_cUndoLayer > 0);
   if (m_cUndoLayer > 0)
   {
      m_cUndoLayer--;
   }

   if (m_cUndoLayer == 0 && (m_sdsDirty < eSaveDirty))
   {
      m_sdsDirty = eSaveDirty;
      if(!m_startToPlay) // undo history only due to backup? -> do not flag table as dirty
         m_ptable->SetDirty(eSaveDirty);
   }
}

UndoRecord::UndoRecord()
{
}

UndoRecord::~UndoRecord()
{
   for (size_t i = 0; i < m_vstm.size(); i++)
      m_vstm[i]->Release();

   for (size_t i = 0; i < m_vieDelete.size(); i++)
      m_vieDelete[i]->Release();
}

void UndoRecord::MarkForUndo(IEditable * const pie, const bool backupForPlay)
{
   if (FindIndexOf(m_vieMark, pie) != -1) // Been marked already
      return;

   if (FindIndexOf(m_vieCreate, pie) != -1) // Just created, so undo will delete it anyway
      return;

   m_vieMark.push_back(pie);

   FastIStream * const pstm = new FastIStream();
   pstm->AddRef();

   DWORD write;
   pstm->Write(&pie, sizeof(IEditable *), &write);

   pie->SaveData(pstm, NULL, true);

   m_vstm.push_back(pstm);
}

void UndoRecord::MarkForCreate(IEditable * const pie)
{
#ifdef _DEBUG
   if (FindIndexOf(m_vieCreate, pie) != -1) // Created twice?
   {
      _ASSERTE(fFalse);
      return;
   }
#endif

   m_vieCreate.push_back(pie);
}

void UndoRecord::MarkForDelete(IEditable * const pie)
{
#ifdef _DEBUG
   if (FindIndexOf(m_vieDelete, pie) != -1) // Already deleted - bad thing
   {
      _ASSERTE(fFalse);
      return;
   }
#endif

   const int pie_pos = FindIndexOf(m_vieCreate, pie);
   if (pie_pos != -1) // Created and deleted in the same step
   {
      // Just forget about it
      m_vieCreate.erase(m_vieCreate.begin() + pie_pos);
      RemoveFromVectorSingle(m_vieMark, pie);
      return;
   }

   m_vieDelete.push_back(pie);
}
