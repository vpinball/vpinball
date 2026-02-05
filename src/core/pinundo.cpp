// license:GPLv3+

#include "core/stdafx.h"
#include "vpversion.h"

PinUndo::PinUndo(PinTable *table)
   : m_table(table)
{
}

void PinUndo::SetCleanPoint(const SaveDirtyState sds)
{
   if (sds == eSaveClean)
      m_cleanpoint = m_undoRecords.size();
   m_sdsDirty = sds;
   m_table->SetDirty(sds);
}

void PinUndo::BeginUndo()
{
   if (m_table->m_liveBaseTable)
      return;

   m_cUndoLayer++;
   if (m_cUndoLayer == 1)
   {
      if (m_undoRecords.size() == MAXUNDO)
      {
         m_undoRecords.erase(m_undoRecords.begin());
         m_cleanpoint--;
      }
      m_undoRecords.push_back(std::make_unique<UndoRecord>());
   }
}

void PinUndo::MarkForUndo(IEditable *const pie, const bool saveForUndo)
{
   if (m_table->m_liveBaseTable)
      return;

   assert(!m_undoRecords.empty());
   assert(m_cUndoLayer > 0);
   if (m_undoRecords.empty())
      return;
   if (m_cUndoLayer <= 0)
      BeginUndo();

   m_undoRecords.back()->MarkForUndo(pie, saveForUndo);
}

void PinUndo::MarkForCreate(IEditable *const pie)
{
   if (m_table->m_liveBaseTable)
      return;

   assert(!m_undoRecords.empty());
   assert(m_cUndoLayer > 0);
   if (m_undoRecords.empty())
      return;
   if (m_cUndoLayer <= 0)
      BeginUndo();

   m_undoRecords.back()->MarkForCreate(pie);
}

void PinUndo::MarkForDelete(IEditable *const pie)
{
   if (m_table->m_liveBaseTable)
      return;

   assert(!m_undoRecords.empty());
   assert(m_cUndoLayer > 0);
   if (m_undoRecords.empty())
      return;
   if (m_cUndoLayer <= 0)
      BeginUndo();

   m_undoRecords.back()->MarkForDelete(pie);
}

void PinUndo::Undo(bool discard)
{
   if (m_undoRecords.empty())
      return;

   assert(m_cUndoLayer == 0);
   while (m_cUndoLayer > 0)
      EndUndo();

   if (g_pplayer == nullptr && m_undoRecords.size() == m_cleanpoint)
   {
      const int result = m_table->m_tableEditor->ShowMessageBox(LocalString(IDS_UNDOPASTSAVE).m_szbuffer);
      if (result != IDYES)
         return;
   }

   if (!discard)
   {
      for (IEditable *editable : m_undoRecords.back()->m_vieDelete)
      {
         m_table->Undelete(editable);
         editable->AddRef(); // As undelete does not add the reference on the undeleted part (should be fixed there ?)
      }

      for (IStream *const pstm : m_undoRecords.back()->m_vstm)
      {
         // Go back to beginning of stream to load
         LARGE_INTEGER foo;
         foo.QuadPart = 0;
         pstm->Seek(foo, STREAM_SEEK_SET, nullptr);

         DWORD read;
         IEditable *pie;
         pstm->Read(&pie, sizeof(IEditable *), &read);
         pie->ClearForOverwrite();

         pie->InitLoad(pstm, m_table, CURRENT_FILE_FORMAT_VERSION, 0, 0);
         pie->InitPostLoad();
         if (g_pplayer)
         {
            if (pie->GetIHitable())
               g_pplayer->m_renderer->ReinitRenderable(pie->GetIHitable());
            g_pplayer->m_physics->Update(pie);
         }
      }

      for (size_t i = 0; i < m_undoRecords.back()->m_vieCreate.size(); i++)
         m_table->Uncreate(m_undoRecords.back()->m_vieCreate[i]);
   }
   m_undoRecords.pop_back();

   if ((m_undoRecords.size() == m_cleanpoint) && (m_sdsDirty > eSaveClean)) // UNDONE - how could we get here without m_fDirty being true?
   {
      m_sdsDirty = eSaveClean;
      m_table->SetDirty(eSaveClean);
   }
   else if (/*m_vur.size() < m_cleanpoint && */ (m_sdsDirty < eSaveDirty))
   {
      // If we're not clean, we must be dirty (always process this case for autosave, even though to the user it wouldn't appear to matter)
      if (m_undoRecords.size() < m_cleanpoint)
      {
         m_cleanpoint = -1; // Can't redo yet
      }
      m_sdsDirty = eSaveDirty;
      m_table->SetDirty(eSaveDirty);
   }
}

void PinUndo::EndUndo()
{
   if (m_table->m_liveBaseTable)
      return;

   assert(m_cUndoLayer > 0);
   if (m_cUndoLayer > 0)
      m_cUndoLayer--;
   if (m_cUndoLayer == 0 && (m_sdsDirty < eSaveDirty))
   {
      m_sdsDirty = eSaveDirty;
      m_table->SetDirty(eSaveDirty);
   }
}

UndoRecord::UndoRecord() { }

UndoRecord::~UndoRecord()
{
   for (FastIStream *stream : m_vstm)
      stream->Release();

   for (IEditable *editable : m_vieDelete)
      editable->Release();
}

void UndoRecord::MarkForUndo(IEditable *const pie, const bool saveForUndo)
{
#ifndef __STANDALONE__
   if (FindIndexOf(m_vieMark, pie) != -1) // Been marked already
      return;

   if (FindIndexOf(m_vieCreate, pie) != -1) // Just created, so undo will delete it anyway
      return;

   m_vieMark.push_back(pie);

   FastIStream *const pstm = new FastIStream();
   pstm->AddRef();

   DWORD write;
   pstm->Write(&pie, sizeof(IEditable *), &write);

   pie->SaveData(pstm, 0, true);

   m_vstm.push_back(pstm);
#endif
}

void UndoRecord::MarkForCreate(IEditable *const pie)
{
   assert(FindIndexOf(m_vieCreate, pie) == -1); // Created twice?
   m_vieCreate.push_back(pie);
}

void UndoRecord::MarkForDelete(IEditable *const pie)
{
   assert(FindIndexOf(m_vieDelete, pie) == -1); // Already deleted - bad thing
   
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
