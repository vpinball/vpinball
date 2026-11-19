// license:GPLv3+

#pragma once

#include <SDL3/SDL_process.h>

#include <filesystem>
#include <unordered_set>

#include "EditorAreaView.h"
#include "core/ScriptInterpreter.h"
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

   void Update(); // Per frame housekeeping (external editor session, Win32 code view notification)
   void RenderPane(const AreaRect &rect, int leafId); // Renders the script editor inside an area leaf

   void ShowLine(int line); // Hosts the panel if needed and reveals the given (1-based) script line

   // Script engine error report (runtime errors during a play test, 1-based line/column)
   void OnScriptError(ScriptInterpreter::ErrorType type, int line, int column, const string &description, const vector<string> &stackDump);

   // The code editor has the keyboard focus this frame: the editor wide Ctrl+Space and Ctrl+F, which it uses itself, skip it
   bool HasKeyboardFocus() const;

private:
   void OpenInExternalEditor();
   void SyncExternalScript();
   void StopExternalSession();

   EditorUI &m_editor;
   bool m_focus = false; // Focus the pane on next render (Show was called while already open)

   // Embedded code editor hosting the table script (see third-party/include/textedit)
   TextEditor m_te;
   PinTable *m_teTable = nullptr; // Table whose script text is currently loaded in m_te
   bool m_teEdited = false; // The widget's document changed (typing, autocomplete, find/replace, menus, undo,...): synced after its render
   string m_teSynced; // Widget text last loaded from or written to the table script (line feeds only)
   string m_teSyncedScript; // Table script last loaded into or written from the widget: if the script differs, another editor changed
                            // it (Win32 code view, external editor, script file override) and the widget reloads it
   double m_notifyTableEditorAt = -1.0; // ImGui time of the last script write the Win32 code view was not told yet (-1: none)
   bool m_teCrlf = false; // The table script has CR line ends: written back with CR LF (the widget keeps line feeds only)
   bool m_teLossy = false; // The widget can not hold the script text unchanged: shown read-only so an edit can not rewrite it
   int m_teFocusFrame = -1; // ImGui frame in which the code editor last had the keyboard focus
   void LoadScript(const string &script, bool keepUndo);
   void SyncScript(PinTable *table);

   float m_fontZoom = 1.0f; // Ctrl+wheel zoom factor applied to the editor's mono font
   bool m_gotoLinePopup = false; // Open the go-to-line input on the next render
   int m_gotoLine = 1; // Last target of the go-to-line input

   // Script model + API metadata for dynamic highlighting and autocomplete
   ScriptModel m_model;
   ScriptApi m_api;
   TextEditor::AutoCompleteConfig m_acConfig;
   double m_modelDirtyAt = 0.0; // ImGui time of the last edit; the model reparses after a short delay
   bool m_modelDirty = true;
   void ReparseModel();
   void OnAutoComplete(TextEditor::AutoCompleteState &state);
   void OnContextMenu(TextEditor::PopupData &data);
   void OnTextHover(TextEditor::PopupData &data);

   // Word at position -> declaration: script symbols navigate, core.vbs/API names only describe
   const Symbol *FindDefinition(const std::string &word) const;
   void GoToDefinition(TextEditor::DocPos pos);
   void JumpToLine(size_t line);

   // Top bar menus and the editing actions they trigger
   void RenderViewMenu();
   void RenderEditMenu();
   void RenderFormatMenu();
   void RenderGotoLinePopup();
   void DuplicateLine();
   void AutoIndent(size_t first, size_t last);
   void ConvertIndentation(bool toTabs);

   // Parse/compile errors as gutter markers + squiggly underlines
   static constexpr size_t s_errorSquiggle = 1;
   static constexpr size_t s_blockSquiggle = 2; // Unbalanced block structure found by the script model (refreshed on each reparse)
   static constexpr size_t s_occurrenceSquiggle = 3; // Background behind the occurrences of a double clicked word
   void ReportError(int line, int column, const std::string &description);
   void ClearErrors();
   std::unordered_set<size_t> m_errorLines; // Lines with an error marker: an error repeated by the script (e.g. in a timer) is marked once
   void HighlightOccurrences(const std::string &word);

   // Error log (the Win32 code view's last error log + error dialog): script engine errors with their stack trace, newest
   // last; an error raised again (e.g. by a timer) is counted on its entry instead of being added again
   struct ErrorLogEntry
   {
      ScriptInterpreter::ErrorType type;
      int line; // 1-based, as reported
      int column;
      string description;
      vector<string> stackDump; // Most recent call first
      int count = 1;
   };
   vector<ErrorLogEntry> m_errorLog;
   bool m_errorLogOpen = true;
   void AddErrorLog(ScriptInterpreter::ErrorType type, int line, int column, const string &description, const vector<string> &stackDump);
   void RenderErrorLog();
   void CheckSyntax();
   string m_status; // Transient status shown under the title row (check results, def lookup)

   void RenderSymbolMenuItem(const Symbol &symbol); // One outliner entry (recursive for classes)

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
