// license:GPLv3+

#pragma once

#include <SDL3/SDL_process.h>

#include <filesystem>

#include "EditorAreaView.h"
#include "textedit/TextEditor.h"
#include "script/ScriptAnalysis.h"
#include "script/ScriptApi.h"

class PinTable;

namespace VPX::EditorUI
{

class EditorUI;

// Table script editor pane: an embedded code editor (ImGuiColorTextEdit) by default.
// Alternatively, when an external editor command is configured, the script can be written to
// a temporary file opened in that editor, and synchronized back to the table whenever the
// file changes on disk.
class ScriptPanel
{
public:
   explicit ScriptPanel(EditorUI &editor);
   ~ScriptPanel();

   void Show(); // Hosts the panel in an area leaf (splitting the focused one)
   bool IsVisible() const;

   void Update(); // Per frame housekeeping (external editor session)
   void RenderPane(const AreaRect &rect, int leafId); // Renders the script editor inside an area leaf

private:
   void OpenInExternalEditor();
   void SyncExternalScript();
   void StopExternalSession();

   EditorUI &m_editor;
   bool m_focus = false; // Focus the pane on next render (Show was called while already open)

   // Embedded code editor hosting the table script (see third-party/include/textedit)
   TextEditor m_te;
   PinTable *m_teTable = nullptr; // Table whose script text is currently loaded in m_te
   bool m_teReload = false; // Script text changed outside the widget: push it on next render

   // Script model + API metadata for dynamic highlighting and autocomplete
   ScriptModel m_model;
   ScriptApi m_api;
   TextEditor::AutoCompleteConfig m_acConfig;
   double m_modelDirtyAt = 0.0; // ImGui time of the last edit; the model reparses after a short delay
   bool m_modelDirty = true;
   void ReparseModel();
   void OnAutoComplete(TextEditor::AutoCompleteState &state);

   string m_extEditorCmd; // External editor command, mirrored to the Editor.ExternalScriptEditor app setting
   bool m_extCmdLoaded = false;

   // External editing session: m_script_text of the armed table is written to a temporary file,
   // opened in the configured external editor, then written back whenever the file changes on disk
   PinTable *m_extTable = nullptr;
   std::filesystem::path m_extFile;
   std::filesystem::file_time_type m_extFileTime {};
   SDL_Process *m_extProcess = nullptr;
};

// Area view hosting the script editor in a resizable leaf (the panel keeps the shared state)
class ScriptPane final : public EditorAreaView
{
public:
   explicit ScriptPane(EditorUI &editor)
      : EditorAreaView(editor)
   {
   }
   AreaType GetType() const override { return AreaType::Script; }
   void RenderPane(const AreaRect &rect, int leafId) override;
   void OnUnhosted() override;
   void AppendStatusHints(vector<StatusHint> &hints) const override;
};
}
