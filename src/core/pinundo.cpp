// license:GPLv3+

#include "core/stdafx.h"

#include "pinundo.h"

#include "vpversion.h"
#include "parts/ball.h"
#include "parts/PartGroup.h"
#include "parts/pintable.h"
#include "ui/live/LiveUI.h"
#include "utils/fileio.h"
#include "utils/BiffReader.h"
#include "utils/BiffWriter.h"

#define MAXUNDO 16 // Kept records, plus the one being recorded


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
#ifdef VPX_ENABLE_WIN32_EDITOR
   if (FindIndexOf(m_vieMark, pie) != -1) // Been marked already
      return;

   if (FindIndexOf(m_vieCreate, pie) != -1) // Just created, so undo will delete it anyway
      return;

   m_vieMark.push_back(pie);

   auto pstm = std::make_unique<InMemStream>();

   pstm->Write(&pie, sizeof(IEditable *));

   BiffWriter writer(pstm.get(), nullptr);
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
      // Only drop the oldest record once the history exceeds its size, so that a record that ends up being discarded (see Discard) does not cost a real one
      if (m_undoRecords.size() > MAXUNDO)
      {
         m_undoRecords.erase(m_undoRecords.begin());
         m_cleanpoint--;
      }
      m_dirtyStateAtBegin = m_dirtyState;
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
      m_table->Undelete(editable); // Adds the table reference; the record's one is released with the record below
      if (g_pplayer && (g_pplayer->m_ptable == m_table))
      {
         // Deleted through the LiveUI: restore the player side resources it released (same setup as part creation)
         g_pplayer->TimerSetup(editable);
         if (IRenderable *const renderable = editable->GetIRenderable(); renderable)
            renderable->RenderSetup(g_pplayer->m_renderer.get());
         if (editable->GetIHitable())
            g_pplayer->m_physics->Add(editable);
      }
   }

   for (const auto &pstm : m_undoRecords.back()->m_vstm)
   {
      IEditable *const pie = *reinterpret_cast<IEditable *const *>(pstm->Data());
      IScriptable *const scriptable = pie->GetIScriptable();
      const string nameBefore = scriptable ? scriptable->m_name : string();
      pie->ClearForOverwrite();

      // Process the loaded PartGroup parenting to support undoing reparenting
      pie->m_onLoadExpectedPartGroup.clear();
      BiffReader reader(pstm->Data() + sizeof(IEditable *), static_cast<uint32_t>(pstm->Size() - sizeof(IEditable *)), CURRENT_FILE_FORMAT_VERSION, nullptr, 0);
      pie->Load(reader);

      // Load writes the name directly: apply a restored name through the table to keep its name registry (and code view) in sync
      if (scriptable && scriptable->m_name != nameBefore && m_table->HasRegisteredName(pie))
      {
         const string restoredName = scriptable->m_name;
         scriptable->m_name = nameBefore;
         if (lowerCase(restoredName) == lowerCase(nameBefore) || m_table->IsNameUnique(restoredName))
            m_table->RenamePart(pie, restoredName);
         else
            PLOGW << "Undo could not restore the name '" << restoredName << "' of '" << nameBefore << "' as it is now used by another part";
      }
      // The record holds the name of the part's group (empty when it had none, which is only valid for part groups)
      if (!pie->m_onLoadExpectedPartGroup.empty())
      {
         const string groupName = pie->m_onLoadExpectedPartGroup;
         for (IEditable *const edit : m_table->GetParts())
            if (edit->GetItemType() == eItemPartGroup && StrCompareNoCase(edit->GetIScriptable()->m_name, groupName))
            {
               pie->SetPartGroup(static_cast<PartGroup *>(edit));
               break;
            }
      }
      else if (pie->GetItemType() == eItemPartGroup)
         pie->SetPartGroup(nullptr);
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

   // A discarded record leaves no trace: restore the dirty state it changed when ended
   if (m_dirtyState != m_dirtyStateAtBegin)
   {
      m_dirtyState = m_dirtyStateAtBegin;
      m_table->SetDirty(m_dirtyState);
   }
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

