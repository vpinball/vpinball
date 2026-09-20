// license:GPLv3+

#include "core/stdafx.h"

#include "pinundo.h"

#include "vpversion.h"
#include "parts/ball.h"
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

   vector<std::unique_ptr<InMemStream>> m_vstm;
   vector<IEditable *> m_vieCreate;
   vector<IEditable *> m_vieDelete;
   vector<IEditable *> m_vieMark;
   std::any m_editorState; // Editor provided state (e.g. selection) captured when the record was begun
};


UndoRecord::UndoRecord() { }

UndoRecord::~UndoRecord()
{
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

   auto pstm = std::make_unique<InMemStream>();

   pstm->Write(&pie, sizeof(IEditable *));

   BiffWriter writer(pstm.get(), 0);
   pie->Save(writer, true);

   m_vstm.push_back(std::move(pstm));
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
      if (m_editorStateCapture)
         m_undoRecords.back()->m_editorState = m_editorStateCapture();
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

std::any PinUndo::Undo()
{
   assert(m_nUndoLayer == 0);
   while (m_nUndoLayer > 0)
      EndUndo();

   if (!HasUndo())
      return {};

   for (IEditable *editable : m_undoRecords.back()->m_vieDelete)
   {
      m_table->Undelete(editable);
      editable->AddRef(); // As undelete does not add the reference on the undeleted part (should be fixed there ?)
   }

   for (const auto &pstm : m_undoRecords.back()->m_vstm)
   {
      IEditable *const pie = *reinterpret_cast<IEditable *const *>(pstm->Data());
      pie->ClearForOverwrite();

      // Note that we do not process the loaded PartGroup parenting. This is not an issue as we do not support undoing reparenting (yet)
      BiffReader reader(pstm->Data() + sizeof(IEditable *), static_cast<uint32_t>(pstm->Size() - sizeof(IEditable *)), CURRENT_FILE_FORMAT_VERSION, 0, 0);
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
   {
      IEditable *const pie = m_undoRecords.back()->m_vieCreate[i];
      if (g_pplayer && (g_pplayer->m_ptable == m_table))
      {
         // The part was created through the LiveUI and owns player side resources: release them before removing it
         if (pie->GetItemType() == eItemBall)
            RemoveFromVectorSingle(g_pplayer->m_vball, static_cast<Ball *>(pie));
         if (pie->GetIHitable())
            g_pplayer->m_physics->Remove(pie);
         if (pie->m_phittimer)
            g_pplayer->TimerRelease(pie);
         pie->AddRef(); // Keep the part alive until its deferred render release has been processed
         g_pplayer->m_renderer->m_renderDevice->AddEndOfFrameCmd(
            [pie]()
            {
               if (pie->GetIRenderable())
                  pie->GetIRenderable()->RenderRelease();
               pie->Release();
            });
      }
      m_table->Uncreate(pie);
   }

   std::any editorState = std::move(m_undoRecords.back()->m_editorState);
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

   return editorState;
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

