// license:GPLv3+

#pragma once

#include <SDL3/SDL_process.h>

#include <filesystem>

class PinTable;

namespace VPX::EditorUI
{

class EditorUI;

// Floating window holding the table script editor: a basic embedded text editor by
// default. Alternatively, when an external editor command is configured, the script
// can be written to a temporary file opened in that editor, and synchronized back to
// the table whenever the file changes on disk.
class ScriptPanel
{
public:
   explicit ScriptPanel(EditorUI &editor);
   ~ScriptPanel();

   void Show()
   {
      m_visible = true;
      m_focus = true;
   }
   void Close();
   bool IsVisible() const { return m_visible; }

   void Render();

private:
   void OpenInExternalEditor();
   void SyncExternalScript();
   void StopExternalSession();

   EditorUI &m_editor;
   bool m_visible = false;
   bool m_focus = false; // Focus the window on next render (Show was called while already open)

   string m_extEditorCmd; // External editor command, mirrored to the Editor.ExternalScriptEditor app setting
   bool m_extCmdLoaded = false;

   // External editing session: m_script_text of the armed table is written to a temporary file,
   // opened in the configured external editor, then written back whenever the file changes on disk
   PinTable *m_extTable = nullptr;
   std::filesystem::path m_extFile;
   std::filesystem::file_time_type m_extFileTime {};
   SDL_Process *m_extProcess = nullptr;
};

}
