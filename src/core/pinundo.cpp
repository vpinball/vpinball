// license:GPLv3+

#include "core/stdafx.h"

#include "pinundo.h"

#include "vpversion.h"
#include "parts/pintable.h"
#include "ui/live/LiveUI.h"
#include "utils/fileio.h"
#include "utils/BiffReader.h"
#include "utils/BiffWriter.h"

#define MAXUNDO 16


class UndoRecord final
{
public:
   UndoRecord();
   ~UndoRecord();

   void MarkForUndo(IEditable *const pie, const bool saveForUndo);
   void MarkForCreate(IEditable *const pie);
   void MarkForDelete(IEditable *const pie);

   vector<FastIStream *> m_vstm;
   vector<IEditable *> m_vieCreate;
   vector<IEditable *> m_vieDelete;
   vector<IEditable *> m_vieMark;
};


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

   BiffWriter writer(pstm, 0);
   pie->Save(writer, true);

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
   pie->AddRef();
}



PinUndo::PinUndo(PinTable *table)
   : m_table(table)
{
}

PinUndo::~PinUndo() { }

bool PinUndo::HasUndo() const { return !m_undoRecords.empty(); }

bool PinUndo::IsUndoPastCleanPoint() const { return !m_undoRecords.empty() && m_undoRecords.size() == m_cleanpoint; }

void PinUndo::SetCleanPoint(const SaveDirtyState sds)
{
   if (sds == eSaveClean)
      m_cleanpoint = m_undoRecords.size();
   m_dirtyState = sds;
   m_table->SetDirty(sds);
}

void PinUndo::BeginUndo()
{
   m_nUndoLayer++;
   if (m_nUndoLayer == 1)
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
   assert(!m_undoRecords.empty());
   assert(m_nUndoLayer > 0);
   if (m_undoRecords.empty())
      return;
   if (m_nUndoLayer <= 0)
      BeginUndo();

   m_undoRecords.back()->MarkForUndo(pie, saveForUndo);
}

void PinUndo::MarkForCreate(IEditable *const pie)
{
   assert(!m_undoRecords.empty());
   assert(m_nUndoLayer > 0);
   if (m_undoRecords.empty())
      return;
   if (m_nUndoLayer <= 0)
      BeginUndo();

   m_undoRecords.back()->MarkForCreate(pie);
}

void PinUndo::MarkForDelete(IEditable *const pie)
{
   assert(!m_undoRecords.empty());
   assert(m_nUndoLayer > 0);
   if (m_undoRecords.empty())
      return;
   if (m_nUndoLayer <= 0)
      BeginUndo();

   m_undoRecords.back()->MarkForDelete(pie);
}

void PinUndo::Undo()
{
   assert(m_nUndoLayer == 0);
   while (m_nUndoLayer > 0)
      EndUndo();

   if (!HasUndo())
      return;

   for (IEditable *editable : m_undoRecords.back()->m_vieDelete)
   {
      m_table->Undelete(editable);
      editable->AddRef(); // As undelete does not add the reference on the undeleted part (should be fixed there ?)
   }

   for (FastIStream *const pstm : m_undoRecords.back()->m_vstm)
   {
      IEditable *const pie = *reinterpret_cast<IEditable *const *>(pstm->m_rg);
      pie->ClearForOverwrite();

      // Note that we do not process the loaded PartGroup parenting. This is not an issue as we do not support undoing reparenting (yet)
      BiffReader reader(
         reinterpret_cast<const uint8_t *>(pstm->m_rg) + sizeof(IEditable *), pstm->m_cSize - static_cast<unsigned int>(sizeof(IEditable *)), CURRENT_FILE_FORMAT_VERSION, 0, 0);
      pie->Load(reader);
      if (g_pplayer)
      {
         if (pie->GetIRenderable())
            g_pplayer->m_renderer->ReinitRenderable(pie->GetIRenderable());
         if (pie->GetIHitable())
            g_pplayer->m_physics->Update(pie);
      }
   }

   for (size_t i = 0; i < m_undoRecords.back()->m_vieCreate.size(); i++)
      m_table->Uncreate(m_undoRecords.back()->m_vieCreate[i]);

   m_undoRecords.pop_back();

   if ((m_undoRecords.size() == m_cleanpoint) && (m_dirtyState > eSaveClean)) // UNDONE - how could we get here without m_fDirty being true?
   {
      m_dirtyState = eSaveClean;
      m_table->SetDirty(eSaveClean);
   }
   else if (/*m_vur.size() < m_cleanpoint && */ (m_dirtyState < eSaveDirty))
   {
      // If we're not clean, we must be dirty (always process this case for autosave, even though to the user it wouldn't appear to matter)
      if (m_undoRecords.size() < m_cleanpoint)
      {
         m_cleanpoint = -1; // Can't redo yet
      }
      m_dirtyState = eSaveDirty;
      m_table->SetDirty(eSaveDirty);
   }
}

void PinUndo::Discard()
{
   assert(m_nUndoLayer == 0);
   while (m_nUndoLayer > 0)
      EndUndo();

   if (!HasUndo())
      return;

   m_undoRecords.pop_back();
}


void PinUndo::EndUndo()
{
   assert(m_nUndoLayer > 0);
   if (m_nUndoLayer > 0)
      m_nUndoLayer--;
   if (m_nUndoLayer == 0 && (m_dirtyState < eSaveDirty))
   {
      m_dirtyState = eSaveDirty;
      m_table->SetDirty(eSaveDirty);
   }
}

