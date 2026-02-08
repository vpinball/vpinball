// license:GPLv3+

// Implementation of WinMain (Windows with UI) or main (Standalone)

#include "core/stdafx.h"
#include "AppCommands.h"
#include "extern.h"
#include "core/TournamentFile.h"

#include <iostream>


ShowInfoAndExitCommand::ShowInfoAndExitCommand(const string& title, const string& message, int exitCode)
   : m_title(title)
   , m_message(message)
   , m_exitCode(exitCode)
{
}

void ShowInfoAndExitCommand::Execute()
{
   if (!m_title.empty())
      std::cout << m_title << "\n\n";
   if (!m_message.empty())
      std::cout << m_message << "\n\n";
#ifndef __STANDALONE__
   ::MessageBox(nullptr, m_message.c_str(), m_title.empty() ? "Visual Pinball" : m_title.c_str(), MB_OK);
#endif
   exit(m_exitCode);
}


TableBasedCommand::TableBasedCommand(const string& tableFilename)
   : m_tableFilename(tableFilename)
{
}

CComObject<PinTable>* TableBasedCommand::LoadTable()
{
   CComObject<PinTable>* table;
   CComObject<PinTable>::CreateInstance(&table);
   table->AddRef();
   table->LoadGameFromFilename(m_tableFilename);
   if (!m_tableIniFileName.empty() && FileExists(m_tableIniFileName))
      table->SetSettingsFileName(m_tableIniFileName);
   return table;
}


ExportVBSCommand::ExportVBSCommand(const string& tableFilename)
   : TableBasedCommand(tableFilename)
{
}

void ExportVBSCommand::Execute()
{
   CComObject<PinTable>* table = LoadTable();
   string scriptFilename = table->m_filename;
   if (ReplaceExtensionFromFilename(scriptFilename, "vbs"s))
      table->m_pcv->SaveToFile(scriptFilename);
}


ExportPOVCommand::ExportPOVCommand(const string& tableFilename)
   : TableBasedCommand(tableFilename)
{
}

void ExportPOVCommand::Execute()
{
   CComObject<PinTable>* table = LoadTable();
   for (int i = 0; i < 3; i++)
      table->mViewSetups[i].SaveToTableOverrideSettings(table->m_settings, (ViewSetupID)i);
   table->m_settings.Save();
}


PlayTableCommand::PlayTableCommand(const string& tableFilename)
   : TableBasedCommand(tableFilename)
{
}

void PlayTableCommand::Execute()
{
   CComObject<PinTable>* table = LoadTable();
   auto player = std::make_unique<Player>(table, Player::PlayMode::Play);
   player->GameLoop();
   player = nullptr;
   table->Release();
}


AuditTableCommand::AuditTableCommand(const string& tableFilename)
   : TableBasedCommand(tableFilename)
{
}

void AuditTableCommand::Execute()
{
   const CComObject<PinTable>* table = LoadTable();
   table->AuditTable(true);
}


PovEditCommand::PovEditCommand(const string& tableFilename)
   : TableBasedCommand(tableFilename)
{
}

void PovEditCommand::Execute()
{
   CComObject<PinTable>* table = LoadTable();
   auto player = std::make_unique<Player>(table, Player::PlayMode::EditPOV);
   player->GameLoop();
   player = nullptr;
   table->Release();
}


Win32EditCommand::Win32EditCommand()
   : Win32EditCommand(""s)
{
}

Win32EditCommand::Win32EditCommand(const string& tableFilename)
   : TableBasedCommand(tableFilename)
{
}

void Win32EditCommand::Execute()
{
   WinEditor vpxEditor(g_app->GetInstanceHandle());
   g_pvp = &vpxEditor;
   vpxEditor.m_open_minimized = m_minimized;
   vpxEditor.m_disable_pause_menu = m_disablePauseMenu;
   vpxEditor.Create(nullptr);
   vpxEditor.LoadEditorSetupFromSettings();
   if (!m_tableFilename.empty() && FileExists(m_tableFilename))
   {
      vpxEditor.LoadFileName(m_tableFilename, true);
      if (!m_tableIniFileName.empty() && FileExists(m_tableIniFileName) && vpxEditor.GetActiveTable())
         vpxEditor.GetActiveTable()->SetSettingsFileName(m_tableIniFileName);
   }
   else if (g_app->m_settings.GetEditor_SelectTableOnStart())
   {
      vpxEditor.m_table_played_via_SelectTableOnStart = vpxEditor.LoadFile(false);
   }
   g_app->m_msgLoop->MainMsgLoop();
   g_pvp = nullptr;
}


LiveEditCommand::LiveEditCommand()
   : LiveEditCommand(""s)
{
}

LiveEditCommand::LiveEditCommand(const string& tableFilename)
   : TableBasedCommand(tableFilename)
{
}

void LiveEditCommand::Execute()
{
   CComObject<PinTable>* table = LoadTable();
   auto player = std::make_unique<Player>(table, Player::PlayMode::FullEdit);
   player->GameLoop();
   player = nullptr;
   table->Release();
}


CaptureAttractCommand::CaptureAttractCommand(const string& tableFilename, int nFrames, int framesPerSecond, bool cutToLoop)
   : TableBasedCommand(tableFilename)
   , m_nFrames(nFrames)
   , m_framesPerSecond(framesPerSecond)
   , m_cutToLoop(cutToLoop)
{
}

void CaptureAttractCommand::Execute()
{
   PLOGI << "Video capture mode requested for " << m_nFrames << " frames at " << m_framesPerSecond << "FPS from table '" << m_tableFilename << "' " << (m_cutToLoop ? "with " : "without ")
         << "loop truncation";
   CComObject<PinTable>* table = LoadTable();
   auto player = std::make_unique<Player>(table, Player::PlayMode::CaptureAttract);
   player->GameLoop();
   player = nullptr;
   table->Release();
}


ValidateTournamentCommand::ValidateTournamentCommand(const string& tableFilename, const string& tournamentFilename)
   : TableBasedCommand(tableFilename)
   , m_tournamentFilename(tournamentFilename)
{
}

void ValidateTournamentCommand::Execute()
{
   VPX::TournamentFile::GenerateImageFromTournamentFile(m_tableFilename, m_tournamentFilename);
}



#ifdef __STANDALONE__
int g_argc;
const char **g_argv;
#endif

enum option_names
{
   OPTION_H,
   OPTION_HELP,
   OPTION_QMARK,
#ifndef __STANDALONE__
   OPTION_UNREGSERVER,
   OPTION_REGSERVER,
#endif
   OPTION_DISABLETRUEFULLSCREEN,
   OPTION_ENABLETRUEFULLSCREEN,
   OPTION_MINIMIZED,
   OPTION_EXTMINIMIZED,
   OPTION_GLES,
   OPTION_LESSCPUTHREADS,
   OPTION_EDIT,
#ifdef _DEBUG
   OPTION_LIVE_EDIT,
#endif
   OPTION_PLAY,
   OPTION_POVEDIT,
   OPTION_POV,
   OPTION_EXTRACTVBS,
   OPTION_INI,
   OPTION_TABLE_INI,
   OPTION_TOURNAMENT,
   OPTION_VERSION,
   OPTION_FRONTEND_EXIT,
   OPTION_AUDIT,
   OPTION_CAPTURE_ATTRACT,
   OPTION_CUSTOM1,
   OPTION_CUSTOM2,
   OPTION_CUSTOM3,
   OPTION_CUSTOM4,
   OPTION_CUSTOM5,
   OPTION_CUSTOM6,
   OPTION_CUSTOM7,
   OPTION_CUSTOM8,
   OPTION_CUSTOM9,
   OPTION_PREFPATH,
   OPTION_INVALID,
};
struct CommandLineOption
{
   option_names name;
   string arg;
   string desc;
};

static const CommandLineOption options[] = {
   { OPTION_H, "h"s, string() },
   { OPTION_HELP, "help"s, string() },
   { OPTION_QMARK, "?"s, string() },
#ifndef __STANDALONE__
   { OPTION_UNREGSERVER, "UnregServer"s, "Unregister VP functions"s },
   { OPTION_REGSERVER, "RegServer"s,"Register VP functions"s },
#endif
   { OPTION_DISABLETRUEFULLSCREEN, "DisableTrueFullscreen"s, "Force-disable True Fullscreen setting [Deprecated, uses ini serttings instead]"s },
   { OPTION_ENABLETRUEFULLSCREEN, "EnableTrueFullscreen"s, "Force-enable True Fullscreen setting [Deprecated, uses ini serttings instead]"s },
   { OPTION_MINIMIZED, "Minimized"s, "Start the windows editor in the 'invisible' minimized window mode"s },
   { OPTION_EXTMINIMIZED, "ExtMinimized"s, "Start the windows editor in the 'invisible' minimized window mode, but with enabled Pause Menu"s },
   { OPTION_GLES, "GLES"s, "[value]  Overrides the global emission scale (day/night setting, value range: 0.115..0.925) [Deprecated, uses ini serttings instead]"s },
   { OPTION_LESSCPUTHREADS, "LessCPUthreads"s, "Limit the amount of parallel execution"s },
   { OPTION_EDIT, "Edit"s, "[filename]  Load file into VP"s },
#ifdef _DEBUG
   { OPTION_LIVE_EDIT, "LiveEdit"s, "[opt filename]  Start in live editor mode. if a filename is provided, loads it as the table to edit"s },
#endif
   { OPTION_PLAY, "Play"s, "[filename]  Load and play file"s },
   { OPTION_POVEDIT, "PovEdit"s, "[filename]  Load and run file in live editing mode, then export new pov on exit"s },
   { OPTION_POV, "Pov"s, "[filename]  Load, export pov and close"s },
   { OPTION_EXTRACTVBS, "ExtractVBS"s, "[filename]  Load, export table script and close"s },
   { OPTION_INI, "Ini"s, "[filename]  Use a custom settings file instead of loading it from the default location"s },
   { OPTION_TABLE_INI, "TableIni"s, "[filename]  Use a custom table settings file. This option is only available in conjunction with a command which specifies a table filename like Play, Edit,..."s },
   { OPTION_TOURNAMENT, "TournamentFile"s, "[table filename] [tournament filename]  Load a table and tournament file and convert to .png"s },
   { OPTION_VERSION, "v"s, "Displays the version"s },
   { OPTION_FRONTEND_EXIT, "exit"s, ""s }, // (ab)used by frontend, not handled by us
   { OPTION_AUDIT, "Audit"s, "[table filename] Audit the table"s },
   { OPTION_CAPTURE_ATTRACT, "CaptureAttract"s, "Capture an attract mode video"s },
   { OPTION_CUSTOM1, "c1"s, "Custom value 1"s },
   { OPTION_CUSTOM2, "c2"s, "Custom value 2"s },
   { OPTION_CUSTOM3, "c3"s, "Custom value 3"s },
   { OPTION_CUSTOM4, "c4"s, "Custom value 4"s },
   { OPTION_CUSTOM5, "c5"s, "Custom value 5"s },
   { OPTION_CUSTOM6, "c6"s, "Custom value 6"s },
   { OPTION_CUSTOM7, "c7"s, "Custom value 7"s },
   { OPTION_CUSTOM8, "c8"s, "Custom value 8"s },
   { OPTION_CUSTOM9, "c9"s, "Custom value 9"s },
   { OPTION_PREFPATH, "PrefPath"s, "[path]  Use a custom preferences path instead of default"s },
};

string CommandLineProcessor::GetPathFromArg(const string& arg, bool setCurrentPath)
{
   #ifndef __STANDALONE__
      string path = arg;
      if ((arg[0] == '-') || (arg[0] == '/')) // Remove leading - or /
         path = arg.substr(1, arg.size() - 1);
   #else
      string path = trim_string(arg);
   #endif

   if (path[0] == '"') // Remove " "
      path = path.substr(1, path.size() - 1);

   #ifndef __STANDALONE__
      if (path[1] != ':') // Add current path
      {
         char szLoadDir[MAXSTRING];
         GetCurrentDirectory(MAXSTRING, szLoadDir);
         path = string(szLoadDir) + PATH_SEPARATOR_CHAR + path;
      }
   #else
      if (path[0] != '/') // Add current path
      {
         char szLoadDir[MAXSTRING];
         GetCurrentDirectory(MAXSTRING, szLoadDir);
         path = string(szLoadDir) + PATH_SEPARATOR_CHAR + path;
      }
   #endif
   else if (setCurrentPath) // Or set the current path from the arg
   {
      const string dir = PathFromFilename(path);
      SetCurrentDirectory(dir.c_str());
   }

   #ifdef __STANDALONE__
      path = std::filesystem::weakly_canonical(std::filesystem::path(path));
   #endif

   return path;
}

string CommandLineProcessor::GetCommandLineHelp()
{
   std::stringstream ss;
   for (const auto& opt : options)
   {
      if (opt.name >= OPTION_CUSTOM1 && opt.name <= OPTION_CUSTOM9)
         continue;
      ss << "-" << opt.arg << "  " << opt.desc << "\n";
   }
   ss << "\n\n-c1 [customparam] .. -c9 [customparam]  Custom user parameters that can be accessed in the script via GetCustomParam(X)";
   return ss.str();
}

void CommandLineProcessor::OnCommandLineError(const string& title, const string& message)
{
   #ifndef __STANDALONE__
      MessageBox(nullptr, message.c_str(), title.c_str(), MB_ICONERROR);
   #else
      std::cout << title << "\n\n" << message << "\n\n";
   #endif
}

void CommandLineProcessor::ProcessCommandLine()
{
#ifndef __STANDALONE__
   int nArgs;
   const char** szArglist = CommandLineToArgvA(GetCommandLine(), &nArgs);
   ProcessCommandLine(nArgs, szArglist);
   free(szArglist);
#else
   ProcessCommandLine(g_argc, g_argv);
#endif
}

void CommandLineProcessor::ProcessCommandLine(int nArgs, const char* szArglist[])
{
   g_app->SetSettingsFileName(""s);

   string tableIniFileName;
   bool win32EditorMinimized = false;
   bool win32EditorExtMinimized = false;
   bool defaultToWin32Editor = true;
   std::vector<std::unique_ptr<AppCommand>> commands;
   for (int i = 1; i < nArgs; ++i) // skip szArglist[0], contains executable name
   {
      option_names opt = option_names::OPTION_INVALID;
      for (size_t i2 = 0; i2 < std::size(options); ++i2)
      {
         if (StrCompareNoCase(szArglist[i], '-' + options[i2].arg) || StrCompareNoCase(szArglist[i], '/' + options[i2].arg))
         {
            opt = static_cast<option_names>(i2);
            break;
         }
      }
      switch (opt)
      {
      case OPTION_INVALID:
         // If the only parameter passed is a vpx table, consider it as /play command
         if ((nArgs == 2) && (i == 1))
         {
            string filename = GetPathFromArg(szArglist[i], false);
            if (extension_from_path(filename) == "vpx" && FileExists(filename))
            {
               commands.push_back(std::make_unique<PlayTableCommand>(filename));
               i++;
            }
            break;
         }
         // ignore stuff (directories) that is passed in via frontends (not considered as invalid)
         if ((szArglist[i][0] != '-') && (szArglist[i][0] != '/'))
            break;
         // Really an invalid parameter, show help and exit
         OnCommandLineError("Visual Pinball Usage"s, "Invalid Parameter "s + szArglist[i] + "\n\nValid Parameters are:\n\n" + GetCommandLineHelp());
         exit(1);
         break;

      case OPTION_INI:
         if (i + 1 >= nArgs)
         {
            OnCommandLineError("Command Line Error"s, "Option '"s + szArglist[i] + "' must be followed by a valid setting file path");
            exit(1);
         }
         g_app->SetSettingsFileName(GetPathFromArg(szArglist[i + 1], false));
         i++;
         break;

      case OPTION_LESSCPUTHREADS:
         g_app->LimitMultiThreading();
         break;

      // FIXME remove as this is now handled by the ini system
      case OPTION_DISABLETRUEFULLSCREEN:
         g_app->m_disEnableTrueFullscreen = 0;
         break;

      // FIXME remove as this is now handled by the ini system
      case OPTION_ENABLETRUEFULLSCREEN:
         g_app->m_disEnableTrueFullscreen = 1;
         break;

      // FIXME remove as this is now handled by the ini system
      case OPTION_GLES: // global emission scale parameter handling
         if (i + 1 < nArgs)
         {
            const char* lpszStr;
            if ((szArglist[i + 1][0] == '-') || (szArglist[i + 1][0] == '/'))
               lpszStr = szArglist[i + 1] + 1;
            else
               lpszStr = szArglist[i + 1];
            g_app->m_fgles = clamp(sz2f(lpszStr), 0.115f, 0.925f);
            g_app->m_bgles = true;
         }
         else
         {
            OnCommandLineError("Command Line Error"s, "Missing global emission scale adjustment"s);
         }
         break;

      case OPTION_CUSTOM1:
      case OPTION_CUSTOM2:
      case OPTION_CUSTOM3:
      case OPTION_CUSTOM4:
      case OPTION_CUSTOM5:
      case OPTION_CUSTOM6:
      case OPTION_CUSTOM7:
      case OPTION_CUSTOM8:
      case OPTION_CUSTOM9:
         if ((i + 1 < nArgs) && (opt - OPTION_CUSTOM1) <= 9)
         {
            g_app->m_customParameters[opt - OPTION_CUSTOM1] = MakeWString(szArglist[i + 1]);
            ++i; // two params processed
         }
         else
         {
            OnCommandLineError("Command Line Error"s, "Missing custom option value after /c..."s);
         }
         break;

      #ifndef __STANDALONE__
      case OPTION_UNREGSERVER:
         defaultToWin32Editor = false;
         VPApp::m_module.UpdateRegistryFromResource(IDR_VPINBALL, FALSE);
         if (VPApp::m_module.UnregisterServer(TRUE) != S_OK)
            ShowError("Unregister VP functions failed");
         exit(0);
         break;

      case OPTION_REGSERVER:
         defaultToWin32Editor = false;
         VPApp::m_module.UpdateRegistryFromResource(IDR_VPINBALL, TRUE);
         if (VPApp::m_module.RegisterServer(TRUE) != S_OK)
            ShowError("Register VP functions failed");
         exit(0);
         break;
      #endif

      case OPTION_PREFPATH:
      {
         if (i + 1 >= nArgs)
         {
            OnCommandLineError("Command Line Error"s, "Option '"s + szArglist[i] + "' must be followed by a valid folder path");
            exit(1);
         }
         string path = GetPathFromArg(szArglist[i + 1], false);
         if (!DirExists(path))
         {
            std::error_code ec;
            if (std::filesystem::create_directories(path, ec))
            {
               PLOGI << "Pref path created: " << path;
            }
            else
            {
               PLOGE << "Unable to create pref path: " << path;
            }
         }
         g_app->m_fileLocator.SetPrefPath(path);
         i++;
         break;
      }

      case OPTION_MINIMIZED:
         win32EditorMinimized = true;
         break;

      case OPTION_EXTMINIMIZED:
         win32EditorExtMinimized = true;
         break;

      case OPTION_H:
      case OPTION_HELP:
      case OPTION_QMARK:
         commands.push_back(std::make_unique<ShowInfoAndExitCommand>("Visual Pinball Usage"s, GetCommandLineHelp(), 0));
         break;

      case OPTION_VERSION:
         commands.push_back(std::make_unique<ShowInfoAndExitCommand>("", "Visual Pinball "s + VP_VERSION_STRING_FULL_LITERAL, 0));
         break;

      #ifdef _DEBUG
      case OPTION_LIVE_EDIT:
         if (i + 1 < nArgs)
         {
            const string tableFileName = GetPathFromArg(szArglist[i + 1], false);
            if (FileExists(tableFileName))
            {
               commands.push_back(std::make_unique<LiveEditCommand>(tableFileName));
               i++;
            }
            else
            {
               commands.push_back(std::make_unique<LiveEditCommand>());
            }
         }
         else
         {
            commands.push_back(std::make_unique<LiveEditCommand>());
         }
         break;
      #endif

      case OPTION_POVEDIT:
      case OPTION_PLAY:
      case OPTION_EDIT:
      case OPTION_AUDIT:
      case OPTION_POV:
      case OPTION_EXTRACTVBS:
      {
         if (i + 1 >= nArgs)
         {
            OnCommandLineError("Command Line Error"s, "Option '"s + szArglist[i] + "' must be followed by a valid table file path");
            exit(1);
         }
         const string tableFileName = GetPathFromArg(szArglist[i + 1], false);
         i++;

         if (!FileExists(tableFileName))
         {
            OnCommandLineError("Command Line Error"s, "Table file '" + tableFileName + "' was not found");
            exit(1);
         }
         else
         {
            switch (opt)
            {
            case OPTION_POVEDIT: commands.push_back(std ::make_unique<PovEditCommand>(tableFileName)); break;
            case OPTION_PLAY: commands.push_back(std ::make_unique<PlayTableCommand>(tableFileName)); break;
            case OPTION_EDIT: commands.push_back(std ::make_unique<Win32EditCommand>(tableFileName)); break;
            case OPTION_AUDIT: commands.push_back(std ::make_unique<AuditTableCommand>(tableFileName)); break;
            case OPTION_POV: commands.push_back(std ::make_unique<ExportPOVCommand>(tableFileName)); break;
            case OPTION_EXTRACTVBS: commands.push_back(std ::make_unique<ExportVBSCommand>(tableFileName)); break;
            }
         }
         break;
      }

      case OPTION_CAPTURE_ATTRACT:
      {
         if (i + 3 >= nArgs)
         {
            OnCommandLineError("Command Line Error"s, "Option '"s + szArglist[i] + "' must be followed by the number of frames to capture, the framerate and a valid table file path");
            exit(1);
         }
         int captureAttract = 0;
         if (!try_parse_int(szArglist[i + 1], captureAttract) || captureAttract <= 0)
         {
            OnCommandLineError("Command Line Error"s, "Invalid number of frames"s);
            exit(1);
         }
         int captureAttractFPS = 0;
         if (!try_parse_int(szArglist[i + 2], captureAttractFPS) || captureAttractFPS <= 0)
         {
            OnCommandLineError("Command Line Error"s, "Invalid framerate"s);
            exit(1);
         }
         const string tableFileName = GetPathFromArg(szArglist[i + 3], false);
         if (!FileExists(tableFileName))
         {
            OnCommandLineError("Command Line Error"s, "Table file '" + tableFileName + "' was not found");
            exit(1);
         }
         bool captureAttractLoop = true;
         if (i + 4 < nArgs && StrCompareNoCase("noloop"s, szArglist[i + 4]))
         {
            captureAttractLoop = false;
            i += 4;
         }
         else
         {
            captureAttractLoop = true;
            i += 3;
         }
         commands.push_back(std::make_unique<CaptureAttractCommand>(tableFileName, captureAttract, captureAttractFPS, captureAttractLoop));
         break;
      }

      case OPTION_TOURNAMENT:
      {
         if (i + 2 >= nArgs)
         {
            OnCommandLineError("Command Line Error"s, "Option '"s + szArglist[i] + "' must be followed by a valid table file path and a valid tournament file path");
            exit(1);
         }
         const string tableFileName = GetPathFromArg(szArglist[i + 1], false);
         i++;
         if (!FileExists(tableFileName))
         {
            OnCommandLineError("Command Line Error"s, "Table file '" + tableFileName + "' was not found");
            exit(1);
         }
         const string tournamentFileName = GetPathFromArg(szArglist[i + 2], false);
         i++;
         if (!FileExists(tournamentFileName))
         {
            OnCommandLineError("Command Line Error"s, "Tournament file '" + tournamentFileName + "' was not found");
            exit(1);
         }
         commands.push_back(std::make_unique<ValidateTournamentCommand>(tableFileName, tournamentFileName));
         break;
      }

      case OPTION_TABLE_INI:
         if (i + 1 >= nArgs)
         {
            OnCommandLineError("Command Line Error"s, "Option '"s + szArglist[i] + "' must be followed by a valid setting file path");
            exit(1);
         }
         tableIniFileName = GetPathFromArg(szArglist[i + 1], false);
         i++;
         break;

      default:
         assert(false);
         break;
      }
   }

   if (defaultToWin32Editor && commands.empty())
      commands.push_back(std::make_unique<Win32EditCommand>());
   if (commands.size() > 1)
   {
      OnCommandLineError("Command Line Error"s, "Multiple command options specified. Please specify only one");
      exit(1);
   }
   if (commands.empty())
      return;
   m_command = std::move(commands[0]);

   if (win32EditorMinimized)
   {
      if (auto win32EditCmd = dynamic_cast<Win32EditCommand*>(m_command.get()); win32EditCmd)
      {
         win32EditCmd->m_minimized = true;
         win32EditCmd->m_disablePauseMenu = true;
      }
      else
      {
         OnCommandLineError("Command Line Error"s, "Option '"s + options[OPTION_MINIMIZED].arg + "' must be used in conjunction with a command that uses the Win32 editor");
         exit(1);
      }
   }

   if (win32EditorExtMinimized)
   {
      if (auto win32EditCmd = dynamic_cast<Win32EditCommand*>(m_command.get()); win32EditCmd)
      {
         win32EditCmd->m_minimized = true;
         win32EditCmd->m_disablePauseMenu = false;
      }
      else
      {
         OnCommandLineError("Command Line Error"s, "Option '"s + options[OPTION_EXTMINIMIZED].arg + "' must be used in conjunction with a command that uses the Win32 editor");
         exit(1);
      }
   }

   if (!tableIniFileName.empty())
   {
      auto tableCmd = dynamic_cast<TableBasedCommand*>(m_command.get());
      if (tableCmd == nullptr)
      {
         OnCommandLineError("Command Line Error"s, "Option '"s + options[OPTION_TABLE_INI].arg + "' must be used in conjunction with a command that specifies a table filename");
         exit(1);
      }
      if (!FileExists(tableIniFileName))
      {
         OnCommandLineError("Command Line Error"s, "Table ini file '" + tableIniFileName + "' was not found");
         exit(1);
      }
      tableCmd->SetTableIniFileName(tableIniFileName);
   }
}

const char** CommandLineProcessor::CommandLineToArgvA(const char* const CmdLine, int* const _argc)
{
   const size_t len = strlen(CmdLine);
   size_t i = ((len + 2) / 2) * sizeof(void*) + sizeof(void*);

   const char** argv = (const char**)malloc(i + (len + 2) * sizeof(char));
   char* _argv = (char*)(((unsigned char*)argv) + i);

   int argc = 0;
   argv[argc] = _argv;
   bool in_QM = false;
   bool in_TEXT = false;
   bool in_SPACE = true;
   i = 0;
   size_t j = 0;

   char a;
   while ((a = CmdLine[i])) {
      if (in_QM) {
         if (a == '\"') {
            in_QM = false;
         }
         else {
            _argv[j] = a;
            j++;
         }
      }
      else {
         switch (a) {
         case '\"':
            in_QM = true;
            in_TEXT = true;
            if (in_SPACE) {
               argv[argc] = _argv + j;
               argc++;
            }
            in_SPACE = false;
            break;
         case ' ':
         case '\t':
         case '\n':
         case '\r':
            if (in_TEXT) {
               _argv[j] = '\0';
               j++;
            }
            in_TEXT = false;
            in_SPACE = true;
            break;
         default:
            in_TEXT = true;
            if (in_SPACE) {
               argv[argc] = _argv + j;
               argc++;
            }
            _argv[j] = a;
            j++;
            in_SPACE = false;
            break;
         }
      }
      i++;
   }
   _argv[j] = '\0';
   argv[argc] = nullptr;

   (*_argc) = argc;
   return argv;
}
