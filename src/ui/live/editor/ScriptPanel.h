// license:GPLv3+

#pragma once

#include <SDL3/SDL_process.h>

#include <filesystem>

#include "EditorAreaView.h"

class PinTable;

namespace VPX::EditorUI
{

class EditorUI;

// Table script editor pane: a basic embedded text editor by default. Alternatively, when an
// external editor command is configured, the script can be written to a temporary file opened
// in that editor, and synchronized back to the table whenever the file changes on disk.
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
