// license:GPLv3+

#include "core/stdafx.h"
#include "ScriptPanel.h"

#include "core/SettingsService.h"
#include "core/ScriptInterpreter.h"
#include "ui/live/IconsLucide.h"
#include "parts/pintable.h"
#include "ui/live/EditorUI.h"
#include "ui/live/ImGuiHelpers.h"
#include "ui/live/LiveUI.h"

#include "EditorTheme.h"
#include "textedit/VbsLex.h"
#include "textedit/languages/TextEditorVbs.h"

#include <SDL3/SDL_error.h>

#include "imgui/imgui_internal.h"
#include "imgui/imgui_stdlib.h"

#include <fstream>

namespace VPX::EditorUI
{

namespace
{

   // Code editor palette: dark base adjusted to the live editor theme (VS Code dark like syntax colors)
   TextEditor::Palette BuildScriptPalette()
   {
      TextEditor::Palette palette = TextEditor::GetDarkPalette();
      using C = TextEditor::Color;
      palette[static_cast<size_t>(C::text)] = EditorTheme::Text;
      palette[static_cast<size_t>(C::keyword)] = EditorTheme::Rgb(86, 156, 214);
      palette[static_cast<size_t>(C::declaration)] = EditorTheme::Rgb(78, 201, 176);
      palette[static_cast<size_t>(C::number)] = EditorTheme::Rgb(181, 206, 168);
      palette[static_cast<size_t>(C::string)] = EditorTheme::Rgb(206, 145, 120);
      palette[static_cast<size_t>(C::punctuation)] = EditorTheme::Text;
      palette[static_cast<size_t>(C::identifier)] = EditorTheme::Text;
      palette[static_cast<size_t>(C::knownIdentifier)] = EditorTheme::Rgb(220, 220, 170);
      palette[static_cast<size_t>(C::comment)] = EditorTheme::Rgb(106, 153, 85);
      palette[static_cast<size_t>(C::background)] = EditorTheme::Strip;
      palette[static_cast<size_t>(C::cursor)] = EditorTheme::Text;
      palette[static_cast<size_t>(C::selection)] = EditorTheme::Rgb(71, 114, 179, 110);
      palette[static_cast<size_t>(C::whitespace)] = EditorTheme::TextFaint;
      palette[static_cast<size_t>(C::matchingBracketBackground)] = EditorTheme::Raised;
      palette[static_cast<size_t>(C::matchingBracketActive)] = EditorTheme::Accent;
      palette[static_cast<size_t>(C::matchingBracketError)] = EditorTheme::Rgb(220, 80, 80);
      palette[static_cast<size_t>(C::lineNumber)] = EditorTheme::TextFaint;
      palette[static_cast<size_t>(C::currentLineNumber)] = EditorTheme::Text;
      palette[static_cast<size_t>(C::currentLineHighlight)] = EditorTheme::Rgb(255, 255, 255, 10);
      palette[static_cast<size_t>(C::currentLineHighlightBorder)] = EditorTheme::Rgb(0, 0, 0, 0);
      return palette;
   }

   // Snapshots of the language's built-in word sets, captured before dynamic script
   // symbols get merged in (see ReparseModel); first call must precede the first merge
   const std::unordered_set<std::string> &BaseDeclarations()
   {
      static const std::unordered_set<std::string> s = TextEditorVbs::getDefinition()->declarations;
      return s;
   }
   const std::unordered_set<std::string> &BaseIdentifiers()
   {
      static const std::unordered_set<std::string> s = TextEditorVbs::getDefinition()->identifiers;
      return s;
   }

   std::string LowerWord(const std::string_view s)
   {
      std::string r(s);
      for (char &c : r)
         c = static_cast<char>(VbsLex::ToLower(c));
      return r;
   }
}

ScriptPanel::ScriptPanel(EditorUI &editor)
   : m_editor(editor)
{
   m_te.SetLanguage(TextEditorVbs::getDefinition());
   m_te.SetPalette(BuildScriptPalette());
   m_te.SetShowLineNumbersEnabled(true);
   m_te.SetLineFoldingEnabled(true);
   m_te.SetCompletePairedGlyphs(true);
   m_te.SetShowCurrentLineHighlightEnabled(true);
   m_te.SetInsertSpacesOnTabs(false);
   m_te.SetFindReplaceEnabled(true);

   // autocomplete: trigger on typing and on '.' for member completion; Ctrl+Space manually
   m_acConfig.triggerCharacters = ".";
   m_acConfig.callback = [this](TextEditor::AutoCompleteState &state) { OnAutoComplete(state); };
   m_acConfig.userData = this;
   m_te.SetAutoCompleteConfig(&m_acConfig);

   // right-click "go to definition" and hover tooltips on symbols
   m_te.SetTextContextMenuCallback([this](TextEditor::PopupData &data) { OnContextMenu(data); });
   m_te.SetTextHoverCallback([this](TextEditor::PopupData &data) { OnTextHover(data); });

   BaseDeclarations(); // pin the base word snapshots before the first dynamic merge
   BaseIdentifiers();
}

// Rebuild the script model/API index and push the dynamic identifier sets into the
// language so script symbols, part names and core.vbs API names get their own color
void ScriptPanel::ReparseModel()
{
   const TextEditor::Language *lang = TextEditorVbs::getDefinition();
   m_model.Parse(m_editor.m_table->m_script_text);
   m_api.Refresh(m_editor.m_table, m_editor.m_table->m_script_text);

   TextEditor::Language *mutableLang = const_cast<TextEditor::Language *>(lang);
   mutableLang->declarations = BaseDeclarations();
   mutableLang->identifiers = BaseIdentifiers();
   for (const Symbol &s : m_model.Symbols())
      (s.kind == SymbolKind::Class ? mutableLang->declarations : mutableLang->identifiers).insert(s.lowerName);
   for (const Symbol &s : m_api.Globals())
      mutableLang->identifiers.insert(s.lowerName);
   m_te.SetLanguage(lang); // re-colorize with the updated sets
}

// Fills the widget's suggestion list: "x." before the cursor completes x's members
// (script classes, `Set x = New C` variables, core.vbs classes, table parts), a bare
// word completes from keywords, builtins, script symbols and API globals
void ScriptPanel::OnAutoComplete(TextEditor::AutoCompleteState &state)
{
   if (state.inComment || state.inString)
      return;

   const std::string ctx = m_te.GetContext(state.searchTermStart, [](ImWchar c) { return VbsLex::IsIdentChar(c) || c == '.'; });
   const size_t lastDot = ctx.rfind('.');
   const std::string chain = lastDot == std::string::npos ? std::string() : ctx.substr(0, lastDot);
   const std::string prefix = LowerWord(state.searchTerm);

   std::vector<std::pair<std::string, std::string>> cands;
   std::unordered_set<std::string> seen;
   const auto add = [&](std::string suggestion, std::string label)
   {
      const std::string key = LowerWord(suggestion);
      if ((!prefix.empty() && key.compare(0, prefix.size(), prefix) != 0) || !seen.insert(key).second)
         return;
      cands.emplace_back(std::move(suggestion), std::move(label));
   };
   const auto addSymbol = [&](const Symbol &s)
   {
      if (s.name.empty())
         return;
      std::string label = s.signature.empty() ? s.name : s.signature;
      label += "  \xe2\x80\x94 "; // em dash
      label += SymbolKindName(s.kind);
      add(s.name, std::move(label));
   };

   if (!chain.empty())
   {
      // member context: resolve the chain's first segment to something with members
      // (script classes, `Set x = ...` variables, core.vbs classes, COM parts)
      const std::string first = LowerWord(chain.substr(0, chain.find('.')));
      for (const Symbol *s : m_api.MembersOf(first, m_model))
         addSymbol(*s);
   }
   else
   {
      // "part_" prefix offers the part's event handler names (Sub Part_Event)
      const size_t underscore = state.searchTerm.find('_');
      if (underscore != std::string::npos && underscore > 0)
      {
         if (const ScriptApiInterface *iface = m_api.InterfaceOfName(state.searchTerm.substr(0, underscore)); iface != nullptr)
         {
            const std::string partName = state.searchTerm.substr(0, underscore);
            for (int i = 0; i < iface->eventCount; ++i)
               add(partName + "_" + iface->events[i].name, partName + "_" + iface->events[i].name + "(" + iface->events[i].signature + ")  \xe2\x80\x94 event handler");
         }
      }
      for (const std::string &kw : TextEditorVbs::getDefinition()->keywords)
         add(kw, kw + "  \xe2\x80\x94 keyword");
      for (const std::string &w : BaseIdentifiers())
         add(w, w + "  \xe2\x80\x94 builtin");
      for (const Symbol &s : m_model.Symbols())
         addSymbol(s);
      for (const Symbol &s : m_api.Globals())
         if (!s.name.empty())
            add(s.name, s.name + "  \xe2\x80\x94 " + s.signature);
   }

   std::sort(cands.begin(), cands.end(), [](const auto &a, const auto &b) { return a.first < b.first; });
   state.suggestions.reserve(cands.size());
   state.labels.reserve(cands.size());
   for (auto &[suggestion, label] : cands)
   {
      state.suggestions.push_back(std::move(suggestion));
      state.labels.push_back(std::move(label));
   }
}

// Symbol lookup for go-to-definition: only the table script's own symbols are
// navigable (core.vbs symbols and COM members have no script location)
const Symbol *ScriptPanel::FindDefinition(const std::string &word) const { return m_model.Find(word); }

void ScriptPanel::JumpToLine(const size_t line)
{
   m_te.SetCursor(TextEditor::DocPos(line, 0));
   m_te.ScrollToLine(line, TextEditor::Scroll::alignMiddle);
   m_te.SetFocus();
}

void ScriptPanel::GoToDefinition(const TextEditor::DocPos pos)
{
   const std::string ctx = m_te.GetContext(pos, [](ImWchar c) { return VbsLex::IsIdentChar(c) || c == '.'; });
   const size_t lastDot = ctx.rfind('.');
   if (lastDot != std::string::npos)
   {
      // member context: navigate only when the member is declared in this script
      // (symbols pointing into the table script's own model have a real line number)
      const std::string first = LowerWord(ctx.substr(0, ctx.find('.')));
      const std::string word = ctx.substr(lastDot + 1);
      const std::vector<Symbol> &scriptSymbols = m_model.Symbols();
      for (const Symbol *s : m_api.MembersOf(first, m_model))
         if (s->lowerName == LowerWord(word))
         {
            if (!scriptSymbols.empty() && s >= scriptSymbols.data() && s < scriptSymbols.data() + scriptSymbols.size())
            {
               JumpToLine(static_cast<size_t>(s->line));
               return;
            }
            m_status = s->signature;
            return;
         }
      m_status = "No definition for '" + word + "'";
      return;
   }

   if (const Symbol *s = FindDefinition(ctx); s != nullptr)
   {
      JumpToLine(static_cast<size_t>(s->line));
      return;
   }
   if (const Symbol *s = m_api.CoreModel().Find(ctx); s != nullptr)
   {
      m_status = "'" + ctx + "' is defined in the builtin scripts (" + s->signature + ")";
      return;
   }
   if (const std::string iface = m_api.InterfaceOf(ctx); !iface.empty())
   {
      m_status = "'" + ctx + "' is a script object (" + iface + ")";
      return;
   }
   m_status = "No definition for '" + ctx + "'";
}

void ScriptPanel::OnContextMenu(TextEditor::PopupData &data)
{
   const std::string word = m_te.GetContext(data.pos, [](ImWchar c) { return VbsLex::IsIdentChar(c) || c == '.'; });
   if (ImGui::MenuItem(ICON_LC_ARROW_RIGHT "  Go to definition", "F12", false, !word.empty()))
      GoToDefinition(data.pos);
}

void ScriptPanel::OnTextHover(TextEditor::PopupData &data)
{
   const auto describe = [this](const Symbol *s, const char *origin)
   {
      if (!s->signature.empty())
         ImGui::TextUnformatted(s->signature.c_str());
      else
         ImGui::TextUnformatted(s->name.c_str());
      ImGui::TextDisabled("%s%s", SymbolKindName(s->kind), origin);
      if (!s->comment.empty())
         ImGui::TextUnformatted(s->comment.c_str());
   };

   const std::string ctx = m_te.GetContext(data.pos, [](ImWchar c) { return VbsLex::IsIdentChar(c) || c == '.'; });
   const size_t lastDot = ctx.rfind('.');
   if (lastDot != std::string::npos)
   {
      const std::string first = LowerWord(ctx.substr(0, ctx.find('.')));
      const std::string word = ctx.substr(lastDot + 1);
      const std::string lower = LowerWord(word);
      for (const Symbol *s : m_api.MembersOf(first, m_model))
         if (s->lowerName == lower)
         {
            describe(s, "");
            return;
         }
      ImGui::CloseCurrentPopup(); // nothing known about this member
      return;
   }

   if (const Symbol *s = m_model.Find(ctx); s != nullptr)
   {
      describe(s, "");
      return;
   }
   if (const Symbol *s = m_api.CoreModel().Find(ctx); s != nullptr)
   {
      describe(s, " (builtin scripts)");
      return;
   }
   for (const Symbol &s : m_api.Globals())
      if (s.lowerName == LowerWord(ctx))
      {
         ImGui::TextUnformatted(s.name.c_str());
         ImGui::TextDisabled("script object (%s)", s.signature.c_str());
         return;
      }
   ImGui::CloseCurrentPopup(); // no info for this word: don't flash an empty popup
}

// Gutter marker + squiggly underline for one script engine diagnostic (1-based line)
void ScriptPanel::ReportError(const int line, const int column, const std::string &description)
{
   const size_t l = static_cast<size_t>(line > 0 ? line - 1 : 0);
   const size_t c = static_cast<size_t>(column > 0 ? column : 0);
   const ImU32 red = EditorTheme::Rgb(240, 80, 80);
   m_te.AddMarker(l, red, red, description, description);
   m_te.AddSquiggle(TextEditor::DocPos(l, c), TextEditor::DocPos(l, c + 8), s_errorSquiggle, red, description);
}

void ScriptPanel::ClearErrors()
{
   m_te.ClearMarkers();
   m_te.ClearSquiggles(s_errorSquiggle);
}

// Runtime error during a play test (or a compile check): mark it in the editor
void ScriptPanel::OnScriptError(const int line, const int column, const string &description) { ReportError(line, column, description); }

// Full compile pass through the script engine (same mechanism as the Win32
// editor's CheckSyntax): parse errors land in ReportError via the error handler
void ScriptPanel::CheckSyntax()
{
   ClearErrors();
   int errorCount = 0;
   std::string firstError;
   CComObject<ScriptInterpreter> *interpreter;
   CComObject<ScriptInterpreter>::CreateInstance(&interpreter);
   interpreter->AddRef();
   interpreter->Start(m_editor.m_table);
   interpreter->SetScriptErrorHandler(
      [this, &errorCount, &firstError](ScriptInterpreter::ErrorType type, int line, int column, const string &description, const vector<string> &)
      {
         if (type != ScriptInterpreter::ErrorType::DebugConsole)
         {
            ++errorCount;
            if (firstError.empty())
               firstError = description;
            ReportError(line, column, description);
         }
      });
   interpreter->Evaluate(m_editor.m_table->m_script_text, false);
   const bool hasError = interpreter->HasError();
   interpreter->Stop(m_editor.m_table);
   interpreter->Release();
   m_status = hasError ? std::to_string(errorCount) + " error(s): " + firstError : "Syntax OK";
}

// Outliner menu entry: classes open a submenu of their members, everything else
// is a leaf item that jumps to the declaration line
void ScriptPanel::RenderSymbolMenuItem(const Symbol &symbol)
{
   const std::string label = symbol.name + "  \xe2\x80\x94 " + SymbolKindName(symbol.kind) + "##" + std::to_string(symbol.line);
   if (symbol.children.empty())
   {
      if (ImGui::MenuItem(label.c_str()))
         JumpToLine(static_cast<size_t>(symbol.line));
      return;
   }
   if (ImGui::BeginMenu(label.c_str()))
   {
      for (const int child : symbol.children)
         RenderSymbolMenuItem(m_model.Symbols()[static_cast<size_t>(child)]);
      ImGui::EndMenu();
   }
}

ScriptPanel::~ScriptPanel() { StopExternalSession(); }

void ScriptPanel::Show()
{
   m_focus = true;
   if (!IsVisible())
      m_editor.ShowAreaEditor(AreaType::Script);
}

bool ScriptPanel::IsVisible() const { return m_editor.m_layout.IsHosted(AreaType::Script); }

// Split a command line into arguments, honoring double quotes (e.g. "C:\My Editor\editor.exe" --wait)
static vector<string> SplitCommandLine(const string &cmd)
{
   vector<string> args;
   string arg;
   bool quoted = false;
   for (const char c : cmd)
   {
      if (c == '"')
         quoted = !quoted;
      else if (c == ' ' && !quoted)
      {
         if (!arg.empty())
         {
            args.push_back(arg);
            arg.clear();
         }
      }
      else
         arg += c;
   }
   if (!arg.empty())
      args.push_back(arg);
   return args;
}

void ScriptPanel::OpenInExternalEditor()
{
   if (m_extEditorCmd.empty() || m_editor.IsInspectMode() || m_editor.m_table->IsLocked())
      return;

   // Write the script of the edited (base) table to the watched temporary file
   PinTable *const baseTable = m_editor.m_table->m_liveBaseTable ? m_editor.m_table->m_liveBaseTable : m_editor.m_table;
   if (m_extTable != baseTable)
   {
      string name = baseTable->m_tableName;
      for (char &c : name)
         if (!isalnum(static_cast<unsigned char>(c)) && c != '-' && c != '_')
            c = '_';
      std::error_code ec;
      const std::filesystem::path tempDir = std::filesystem::temp_directory_path(ec);
      if (ec)
      {
         m_editor.m_liveUI.PushNotification("Failed to locate a temporary folder for the script file"s, 10000);
         return;
      }
      m_extTable = baseTable;
      m_extFile = tempDir / ("vpinball_"s + (name.empty() ? "table"s : name) + ".vbs"s);
   }
   {
      std::ofstream file(m_extFile, std::ios::binary | std::ios::trunc);
      if (!file)
      {
         m_editor.m_liveUI.PushNotification("Failed to write the script file '"s + PathToUTF8(m_extFile) + '\'', 10000);
         return;
      }
      file.write(m_extTable->m_script_text.data(), static_cast<std::streamsize>(m_extTable->m_script_text.size()));
   }
   std::error_code ec;
   m_extFileTime = std::filesystem::last_write_time(m_extFile, ec);
   if (ec)
      m_extFileTime = {};

   // Launch the external editor: '{file}' is replaced by the script file path, appended when absent
   vector<string> args = SplitCommandLine(m_extEditorCmd);
   if (args.empty())
      return;
   const string fileArg = PathToUTF8(m_extFile);
   bool fileInArgs = false;
   for (string &arg : args)
   {
      if (const size_t pos = arg.find("{file}"sv); pos != string::npos)
      {
         arg.replace(pos, "{file}"sv.size(), fileArg);
         fileInArgs = true;
      }
   }
   if (!fileInArgs)
      args.push_back(fileArg);
   vector<const char *> argv;
   argv.reserve(args.size() + 1);
   for (const string &arg : args)
      argv.push_back(arg.c_str());
   argv.push_back(nullptr);
   SDL_Process *const process = SDL_CreateProcess(argv.data(), false);
   if (process == nullptr)
   {
      m_editor.m_liveUI.PushNotification("Failed to launch external editor '"s + args.front() + "': "s + SDL_GetError(), 10000);
      return;
   }
   if (m_extProcess != nullptr)
      SDL_DestroyProcess(m_extProcess); // Releases the tracking object, the previous editor process keeps running
   m_extProcess = process;
}

void ScriptPanel::SyncExternalScript()
{
   if (m_extTable == nullptr || m_extFile.empty())
      return;
   std::ifstream file(m_extFile, std::ios::binary | std::ios::ate);
   if (!file)
      return;
   const std::streamsize size = file.tellg();
   file.seekg(0, std::ios::beg);
   vector<char> buffer(static_cast<size_t>(size));
   if (size > 0 && !file.read(buffer.data(), size))
      return;
   const size_t bom = (buffer.size() >= 3 && memcmp(buffer.data(), "\xEF\xBB\xBF", 3) == 0) ? 3 : 0; // UTF-8 BOM
   string script = string_from_utf8_or_cp1252(buffer.data() + bom, buffer.size() - bom);
   if (script != m_extTable->m_script_text)
   {
      m_extTable->m_script_text = script;
      m_extTable->SetDirtyScript(SaveDirtyState::Dirty);
      if (m_teTable == m_extTable)
         m_teReload = true; // push the new text into the embedded editor (ReplaceAllText keeps the undo history)
   }
}

void ScriptPanel::StopExternalSession()
{
   if (m_extProcess != nullptr)
   {
      SDL_DestroyProcess(m_extProcess); // Releases the tracking object, the editor process keeps running
      m_extProcess = nullptr;
   }
   m_extTable = nullptr;
   m_extFile.clear();
}

void ScriptPanel::Update()
{
   // External editing session housekeeping (kept running while the pane is closed):
   // the session is bound to the edited (base) table, dropped when the table is replaced
   PinTable *const baseTable = m_editor.m_table->m_liveBaseTable ? m_editor.m_table->m_liveBaseTable : m_editor.m_table;
   if (m_extTable != nullptr && m_extTable != baseTable)
      StopExternalSession();
   if (m_extProcess != nullptr)
   {
      int exitCode = 0;
      if (SDL_WaitProcess(m_extProcess, false, &exitCode))
      {
         SyncExternalScript(); // Apply the editor's last save before releasing it
         SDL_DestroyProcess(m_extProcess);
         m_extProcess = nullptr;
      }
   }
   if (m_extTable != nullptr && !m_extFile.empty())
   {
      std::error_code ec;
      const auto fileTime = std::filesystem::last_write_time(m_extFile, ec);
      if (!ec && fileTime != m_extFileTime)
      {
         m_extFileTime = fileTime;
         SyncExternalScript();
      }
   }
}

void ScriptPanel::RenderPane(const AreaRect &rect, const int leafId)
{
   char name[40];
   snprintf(name, sizeof(name), "##Pane%d", leafId);
   ImGui::SetNextWindowPos(ImVec2(rect.x0, rect.y0));
   ImGui::SetNextWindowSize(ImVec2(rect.Width(), rect.Height()));
   ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
   ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
   ImGui::Begin(name, nullptr,
      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus);
   ImGui::PushID(leafId);
   m_editor.RenderAreaTypeButton(leafId);
   ImGui::SameLine();
   ImGui::AlignTextToFramePadding();
   ImGui::TextDisabled("Table Script");

   // Right side of the title row: editor options popover
   const float optW = IconButtonInkSize(ICON_LC_SLIDERS_HORIZONTAL).x;
   const float rightX = rect.Width() - ImGui::GetStyle().WindowPadding.x - optW;
   ImGui::SameLine();
   if (rightX > ImGui::GetCursorPosX())
      ImGui::SetCursorPosX(rightX);
   if (IconButtonInk(ICON_LC_SLIDERS_HORIZONTAL, "##ScriptOptions"))
      ImGui::OpenPopup("Script Options");
   if (const MenuPopupPadding menuPad; ImGui::BeginPopup("Script Options"))
   {
      Settings &settings = g_settingsService.GetAppSettings();
      bool wordWrap = settings.GetEditor_ScriptEditorWordWrap();
      if (ImGui::Checkbox("Word wrap", &wordWrap))
         settings.SetEditor_ScriptEditorWordWrap(wordWrap, false);
      bool minimap = settings.GetEditor_ScriptEditorShowMinimap();
      if (ImGui::Checkbox("Minimap", &minimap))
         settings.SetEditor_ScriptEditorShowMinimap(minimap, false);
      bool whitespace = settings.GetEditor_ScriptEditorShowWhitespace();
      if (ImGui::Checkbox("Show whitespace", &whitespace))
         settings.SetEditor_ScriptEditorShowWhitespace(whitespace, false);

      // Outliner: jump to any declaration (classes expose their members as submenus)
      if (ImGui::BeginMenu("Go to symbol"))
      {
         for (const Symbol *s : m_model.TopLevel())
            RenderSymbolMenuItem(*s);
         ImGui::EndMenu();
      }
      if (ImGui::MenuItem("Check syntax", "F7"))
         CheckSyntax();

      // Event handler stub insertion: one submenu per script-visible object that has
      // events (the table's own script name, parts, collections); already implemented
      // handlers are listed disabled
      const bool canEdit = !m_editor.IsInspectMode() && !m_editor.m_table->IsLocked() && m_extTable == nullptr;
      ImGui::BeginDisabled(!canEdit);
      if (ImGui::BeginMenu("Insert event handler"))
      {
         for (const Symbol &global : m_api.Globals())
         {
            if (global.lowerName == "me" || global.lowerName == "debug" || global.signature.empty())
               continue;
            const ScriptApiInterface *iface = ScriptApi::FindInterface(global.signature);
            if (iface == nullptr || iface->eventCount == 0)
               continue;
            if (ImGui::BeginMenu(global.name.c_str()))
            {
               for (int e = 0; e < iface->eventCount; ++e)
               {
                  const ScriptApiEntry &event = iface->events[e];
                  const std::string handlerName = global.name + "_" + event.name;
                  const bool exists = m_model.Find(handlerName) != nullptr;
                  const std::string label = handlerName + "(" + event.signature + ")" + (exists ? " (defined)" : "");
                  if (ImGui::MenuItem(label.c_str(), nullptr, false, !exists))
                  {
                     const TextEditor::DocPos pos = m_te.GetMainCursorPosition();
                     m_te.ReplaceTextInCurrentCursor("Sub " + handlerName + "(" + event.signature + ")\n\t\nEnd Sub\n");
                     m_te.SetCursor(TextEditor::DocPos(pos.line + 1, 1));
                  }
               }
               ImGui::EndMenu();
            }
         }
         ImGui::EndMenu();
      }
      ImGui::EndDisabled();
      ImGui::EndPopup();
   }
   ImGui::Separator();

   if (m_focus)
   {
      m_focus = false;
      ImGui::SetWindowFocus();
      m_te.SetFocus();
   }
   if (!m_extCmdLoaded)
   {
      m_extCmdLoaded = true;
      m_extEditorCmd = g_settingsService.GetAppSettings().GetEditor_ExternalScriptEditor();
   }

   {
      // The embedded editor is read-only while inspecting a play test, when the table is
      // locked, and while an external editing session is armed (the file on disk wins)
      const bool readOnly = m_editor.IsInspectMode() || m_editor.m_table->IsLocked() || m_extTable != nullptr;
      if (m_editor.IsInspectMode())
         ImGui::TextDisabled("Inspecting a play test session: the script is read-only");
      else if (m_editor.m_table->IsLocked())
         ImGui::TextDisabled("The table is locked: the script is read-only");
      if (!m_editor.m_table->m_external_script_name.empty())
         ImGui::TextDisabled("External script file: %s", PathToUTF8(m_editor.m_table->m_external_script_name).c_str());
      if (m_extTable != nullptr)
         ImGui::TextDisabled("Editing externally: %s", PathToUTF8(m_extFile).c_str());

      // External editor: user configured command launched on a temporary copy of the script
      ImGui::AlignTextToFramePadding();
      ImGui::TextUnformatted("External editor:");
      ImGui::SameLine();
      const float buttonWidth = IconButtonInkSize(ICON_LC_EXTERNAL_LINK).x;
      ImGui::SetNextItemWidth(-buttonWidth - ImGui::GetStyle().ItemSpacing.x);
      ImGui::InputTextWithHint("##exteditor", "Editor command, e.g. notepad++ {file}", &m_extEditorCmd);
      if (ImGui::IsItemDeactivatedAfterEdit())
         g_settingsService.GetAppSettings().SetEditor_ExternalScriptEditor(m_extEditorCmd, false);
      ImGui::SameLine();
      const bool canOpenExternal = !readOnly && !m_extEditorCmd.empty();
      ImGui::BeginDisabled(!canOpenExternal);
      if (IconButtonInk(ICON_LC_EXTERNAL_LINK, "##OpenExternal"))
         OpenInExternalEditor();
      ImGui::EndDisabled();
      if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
         ImGui::SetTooltip("%s", canOpenExternal ? "Open the script in the external editor" : "Configure an external editor command to open the script in it");
      if (!m_status.empty())
         ImGui::TextDisabled("%s", m_status.c_str());

      // (Re)load the editor content when the edited table changes or when the script text
      // was modified outside the widget (external editor file sync)
      PinTable *const table = m_editor.m_table;
      if (m_teTable != table)
      {
         m_teTable = table;
         m_te.SetText(table->m_script_text);
         m_modelDirty = true;
         m_modelDirtyAt = 0.0;
      }
      else if (m_teReload)
      {
         m_teReload = false;
         m_te.ReplaceAllText(table->m_script_text);
      }

      const Settings &settings = g_settingsService.GetAppSettings();
      m_te.SetTabSize(static_cast<size_t>(settings.GetEditor_ScriptEditorTabSize()));
      m_te.SetAutoIndentEnabled(settings.GetEditor_ScriptEditorAutoIndent());
      const bool wordWrap = settings.GetEditor_ScriptEditorWordWrap();
      m_te.SetWordWrapEnabled(wordWrap);
      m_te.SetShowMiniMapEnabled(settings.GetEditor_ScriptEditorShowMinimap());
      m_te.SetShowWhitespacesEnabled(settings.GetEditor_ScriptEditorShowWhitespace());
      m_te.SetReadOnlyEnabled(readOnly);

      // Rebuild the script model shortly after the last edit (drives dynamic coloring
      // and autocomplete) — cheap enough to keep simple, debounced per keystroke burst
      if (m_modelDirty && ImGui::GetTime() - m_modelDirtyAt > 0.25)
      {
         m_modelDirty = false;
         ReparseModel();
      }

      // Embedded code editor, applied to the table script on every edit. Esc releases the
      // editor focus, but only after the widget's transient UI (autocomplete popup,
      // find/replace window, secondary cursors) had its chance to consume it.
      const bool transientActive = m_te.HasActiveTransient();
      ImGui::PushFont(m_editor.m_liveUI.GetMonoFont(), m_editor.m_liveUI.GetMonoFont()->LegacySize);
      const ImGuiWindowFlags teFlags = wordWrap ? ImGuiWindowFlags_NoMove : (ImGuiWindowFlags_NoMove | ImGuiWindowFlags_HorizontalScrollbar);
      const bool changed = m_te.Render("##script", ImVec2(-FLT_MIN, -FLT_MIN), ImGuiChildFlags_None, teFlags);
      ImGui::PopFont();
      if (changed)
      {
         const string text = m_te.GetText();
         if (text != table->m_script_text)
         {
            table->m_script_text = text;
            table->SetDirtyScript(SaveDirtyState::Dirty);
            m_modelDirty = true;
            m_modelDirtyAt = ImGui::GetTime();
            m_status.clear();
            ClearErrors(); // diagnostics point at stale positions after an edit
         }
      }
      if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows))
      {
         if (ImGui::IsKeyPressed(ImGuiKey_F12, false))
            GoToDefinition(m_te.GetMainCursorPosition());
         else if (ImGui::IsKeyPressed(ImGuiKey_F7, false))
            CheckSyntax();
         else if (ImGui::IsKeyPressed(ImGuiKey_Escape, false) && !transientActive)
         {
            ImGui::SetWindowFocus(static_cast<const char *>(nullptr));
            m_editor.m_suppressNextEscape = true;
         }
      }
   }
   ImGui::PopID();
   ImGui::End();
   ImGui::PopStyleVar(2);
}

void ScriptPane::RenderPane(const AreaRect &rect, const int leafId) { m_editor.m_scriptPanel.RenderPane(rect, leafId); }

void ScriptPane::OnUnhosted() { }

void ScriptPane::AppendStatusHints(vector<StatusHint> &hints) const
{
   // Same save label as the status bar fallback: a play test inspection saves the live changes back to the base table
   hints = { { "Esc", "Leave Editor Field" }, { StatusModKey + "+S"s, m_editor.IsInspectMode() ? "Save Changes" : "Save Table" } };
}
}
