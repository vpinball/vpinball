// license:GPLv3+

#pragma once

#include "core/TableAudit.h"

namespace VPX::EditorUI
{

class EditorUI;

// Floating 'Audit & Fix' window (Tools > Audit & Fix...): runs the table audit and presents
// its findings as a navigable list. Picking an item selects its subject in the editor (part,
// asset, script line or table properties); fixable items offer a button applying the fix
// inside an undo record, then the audit is run again since the items hold raw handles.
class AuditDialog
{
public:
   explicit AuditDialog(EditorUI &editor);

   void Show(); // Opens the window and runs the audit
   void Close();
   bool IsVisible() const { return m_visible; }
   void Render(); // Every frame while the editor UI runs
   void OnTableSwitch(); // The edited table changed: drop the stale items, re-run if still visible

private:
   void RunAudit();
   void SelectItem(int index); // 'Select on pick': applies the item's subject to the editor selection
   void RevealSubject(const TableAuditItem &item); // Selects the subject and reveals it (frame part, open script line)
   void ApplyFix(TableAuditItem &item);

   EditorUI &m_editor;
   bool m_visible = false;
   TableAudit m_audit;
   int m_selected = -1;
   bool m_scrollToSelected = false;
   uint64_t m_auditUndoId = 0; // PinUndo::TopRecordId() at audit time: the items' handles are stale once it changes
};

}
