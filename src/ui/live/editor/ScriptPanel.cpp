// license:GPLv3+

#include "core/stdafx.h"
#include "ScriptPanel.h"

#include "core/SettingsService.h"
#include "ui/live/IconsLucide.h"
#include "parts/pintable.h"
#include "ui/live/EditorUI.h"
#include "ui/live/ImGuiHelpers.h"
#include "ui/live/LiveUI.h"

#include <SDL3/SDL_error.h>

#include "imgui/imgui_internal.h"
#include "imgui/imgui_stdlib.h"

#include <fstream>

namespace VPX::EditorUI
{

ScriptPanel::ScriptPanel(EditorUI &editor)
   : m_editor(editor)
{
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
   ImGui::Separator();

   if (m_focus)
   {
      m_focus = false;
      ImGui::SetWindowFocus();
   }
   if (!m_extCmdLoaded)
   {
      m_extCmdLoaded = true;
      m_extEditorCmd = g_settingsService.GetAppSettings().GetEditor_ExternalScriptEditor();
   }

   {
      const bool readOnly = m_editor.IsInspectMode() || m_editor.m_table->IsLocked();
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

      // Esc must not revert the typing: ImGui restores the text the field had when it was activated (the edits are applied on
      // every keystroke), so the field is deactivated before it sees the key. The text is kept, the focus leaves the editor,
      // and the Esc is not an editor Esc.
      if (ImGui::GetActiveID() == ImGui::GetID("##script") && ImGui::IsKeyPressed(ImGuiKey_Escape, false))
      {
         ImGui::ClearActiveID();
         m_editor.m_suppressNextEscape = true;
      }
      // Basic embedded text editor, applied to the table script on every edit
      ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput;
      if (readOnly)
         flags |= ImGuiInputTextFlags_ReadOnly;
      if (ImGui::InputTextMultiline("##script", &m_editor.m_table->m_script_text, ImVec2(-FLT_MIN, -FLT_MIN), flags))
         m_editor.m_table->SetDirtyScript(SaveDirtyState::Dirty);
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
