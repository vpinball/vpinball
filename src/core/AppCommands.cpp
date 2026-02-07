// license:GPLv3+

// Implementation of WinMain (Windows with UI) or main (Standalone)

#include "core/stdafx.h"
#include "AppCommands.h"
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


CaptureAttractCommand::CaptureAttractCommand(const string& tableFileName, int captureAttract, int captureAttractFPS, bool captureAttractLoop)
   : TableBasedCommand(tableFileName)
   , m_captureAttract(captureAttract)
   , m_captureAttractFPS(captureAttractFPS)
   , m_captureAttractLoop(captureAttractLoop)
{
}

void CaptureAttractCommand::Execute()
{

}


ValidateTournamentCommand::ValidateTournamentCommand(const string& tableFileName, const string& tournamentFilename)
   : TableBasedCommand(tableFileName)
   , m_tournamentFilename(tournamentFilename)
{
   //
}

void ValidateTournamentCommand::Execute()
{
   //
   //m_vpxEditor.GenerateImageFromTournamentFile(m_vpxEditor.m_ptableActive->m_filename, m_commandLineProcessor.m_tournamentFileName);
}




#ifdef __STANDALONE__
int g_argc;
const char **g_argv;
#endif

static const string options[] = { // keep in sync with option_names & option_descs!
   "h"s,
   "Help"s,
   "?"s,
   "UnregServer"s,
   "RegServer"s,
   "DisableTrueFullscreen"s,
   "EnableTrueFullscreen"s,
   "Minimized"s,
   "ExtMinimized"s,
   "GLES"s,
   "LessCPUthreads"s,
   "Edit"s,
   #ifdef _DEBUG
   "LiveEdit"s,
   #endif
   "Play"s,
   "PovEdit"s,
   "Pov"s,
   "ExtractVBS"s,
   "Ini"s,
   "TableIni"s,
   "TournamentFile"s,
   "v"s,
   "exit"s, // (ab)used by frontend, not handled by us
   "Audit"s,
   "CaptureAttract"s,
   "c1"s,
   "c2"s,
   "c3"s,
   "c4"s,
   "c5"s,
   "c6"s,
   "c7"s,
   "c8"s,
   "c9"s,
   "PrefPath"s,
   ""s
};
static const string option_descs[] =
{
   string(),
   string(),
   string(),
   "Unregister VP functions"s,
   "Register VP functions"s,
   "Force-disable True Fullscreen setting"s,
   "Force-enable True Fullscreen setting"s,
   "Start VP in the 'invisible' minimized window mode"s,
   "Start VP in the 'invisible' minimized window mode, but with enabled Pause Menu"s,
   "[value]  Overrides the global emission scale (day/night setting, value range: 0.115..0.925)"s,
   "Limit the amount of parallel execution"s,
   "[filename]  Load file into VP"s,
   #ifdef _DEBUG
   "Start VP in live editor mode. if a filename is provided, loads it as the table to edit instead of the default table"s,
   #endif
   "[filename]  Load and play file"s,
   "[filename]  Load and run file in live editing mode, then export new pov on exit"s,
   "[filename]  Load, export pov and close"s,
   "[filename]  Load, export table script and close"s,
   "[filename]  Use a custom settings file instead of loading it from the default location"s,
   "[filename]  Use a custom table settings file. This option is only available in conjunction with a command which specifies a table filename like Play, Edit,..."s,
   "[table filename] [tournament filename]  Load a table and tournament file and convert to .png"s,
   "Displays the version"s,
   string(),
   "[table filename] Audit the table"s,
   "Capture an attract mode video"s,
   "Custom value 1"s,
   "Custom value 2"s,
   "Custom value 3"s,
   "Custom value 4"s,
   "Custom value 5"s,
   "Custom value 6"s,
   "Custom value 7"s,
   "Custom value 8"s,
   "Custom value 9"s,
   "[path]  Use a custom preferences path instead of default"s,
   ""s
};
enum option_names
{
   OPTION_H,
   OPTION_HELP,
   OPTION_QMARK,
   OPTION_UNREGSERVER,
   OPTION_REGSERVER,
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
   return
   #ifndef __STANDALONE__
      "-"    +options[OPTION_UNREGSERVER]+          "  "+option_descs[OPTION_UNREGSERVER]+
      "\n-"  +options[OPTION_REGSERVER]+            "  "+option_descs[OPTION_REGSERVER]+
      "\n\n"+
   #endif
      "-"    +options[OPTION_DISABLETRUEFULLSCREEN]+"  "+option_descs[OPTION_DISABLETRUEFULLSCREEN]+
      "\n-"  +options[OPTION_ENABLETRUEFULLSCREEN]+ "  "+option_descs[OPTION_ENABLETRUEFULLSCREEN]+
      "\n-"  +options[OPTION_MINIMIZED]+            "  "+option_descs[OPTION_MINIMIZED]+
      "\n-"  +options[OPTION_EXTMINIMIZED]+         "  "+option_descs[OPTION_EXTMINIMIZED]+
      "\n\n-"+options[OPTION_GLES]+                 "  "+option_descs[OPTION_GLES]+
      "\n\n-"+options[OPTION_LESSCPUTHREADS]+       "  "+option_descs[OPTION_LESSCPUTHREADS]+
      "\n\n-"+options[OPTION_EDIT]+                 "  "+option_descs[OPTION_EDIT]+
      #ifdef _DEBUG
      "\n-"  +options[OPTION_LIVE_EDIT]+            "  "+option_descs[OPTION_LIVE_EDIT]+
      #endif
      "\n-"  +options[OPTION_PLAY]+                 "  "+option_descs[OPTION_PLAY]+
      "\n-"  +options[OPTION_POVEDIT]+              "  "+option_descs[OPTION_POVEDIT]+
      "\n-"  +options[OPTION_POV]+                  "  "+option_descs[OPTION_POV]+
      "\n-"  +options[OPTION_AUDIT]+                "  "+option_descs[OPTION_AUDIT]+
      "\n-"  +options[OPTION_EXTRACTVBS]+           "  "+option_descs[OPTION_EXTRACTVBS]+
      "\n-"  +options[OPTION_INI]+                  "  "+option_descs[OPTION_INI]+
      "\n-"  +options[OPTION_TABLE_INI]+            "  "+option_descs[OPTION_TABLE_INI]+
      "\n\n-"+options[OPTION_TOURNAMENT]+           "  "+option_descs[OPTION_TOURNAMENT]+
      "\n\n-"+options[OPTION_VERSION]+              "  "+option_descs[OPTION_VERSION]+
      "\n\n-"+options[OPTION_CAPTURE_ATTRACT]+      "  "+option_descs[OPTION_CAPTURE_ATTRACT]+
      "\n\n-"+options[OPTION_PREFPATH]+             "  "+option_descs[OPTION_PREFPATH]+
      "\n\n-c1 [customparam] .. -c9 [customparam]  Custom user parameters that can be accessed in the script via GetCustomParam(X)";
}

void CommandLineProcessor::OnCommandLineError(const string& title, const string& message)
{
   #ifndef __STANDALONE__
      ::MessageBox(NULL, message.c_str(), title.c_str(), MB_ICONERROR);
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

   m_command = nullptr;
   #ifndef __STANDALONE__
   m_command = std::make_unique<Win32EditCommand>();
   #endif

   string tableIniFileName;

   for (int i = 1; i < nArgs; ++i) // skip szArglist[0], contains executable name
   {
      option_names opt = option_names::OPTION_INVALID;
      for (size_t i2 = 0; i2 < std::size(options); ++i2)
      {
         if  (StrCompareNoCase(szArglist[i], '-' + options[i2]) || StrCompareNoCase(szArglist[i], '/' + options[i2]))
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
               m_command = std::make_unique<PlayTableCommand>(filename);
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
      {
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
      }

      #ifndef __STANDALONE__
      case OPTION_UNREGSERVER:
      {
         g_app->m_module.UpdateRegistryFromResource(IDR_VPINBALL, FALSE);
         const HRESULT ret = g_app->m_module.UnregisterServer(TRUE);
         if (ret != S_OK)
            ShowError("Unregister VP functions failed");
         m_command = nullptr;
         break;
      }

      case OPTION_REGSERVER:
      {
         g_app->m_module.UpdateRegistryFromResource(IDR_VPINBALL, TRUE);
         const HRESULT ret = g_app->m_module.RegisterServer(TRUE);
         if (ret != S_OK)
            ShowError("Register VP functions failed");
         m_command = nullptr;
         break;
      }
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
         //m_open_minimized = true;
         //m_disable_pause_menu = true;
         break;

      case OPTION_EXTMINIMIZED:
         //m_open_minimized = true;
         break;

      case OPTION_H:
      case OPTION_HELP:
      case OPTION_QMARK:
      {
         m_command = std::make_unique<ShowInfoAndExitCommand>("Visual Pinball Usage"s, GetCommandLineHelp(), 0);
         break;
      }

      case OPTION_VERSION:
      {
         m_command = std::make_unique<ShowInfoAndExitCommand>("", "Visual Pinball "s + VP_VERSION_STRING_FULL_LITERAL, 0);
         break;
      }

      #ifdef _DEBUG
         if (i + 1 < nArgs)
         {
            const string tableFileName = GetPathFromArg(szArglist[i + 1], false);
            if (FileExists(tableFileName))
            {
               m_command = std::make_unique<LiveEditCommand>(tableFileName);
               i++;
            }
            else
            {
               m_command = std::make_unique<LiveEditCommand>();
            }
         }
         else
         {
            m_command = std::make_unique<LiveEditCommand>();
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
            case OPTION_POVEDIT: m_command = std ::make_unique<PovEditCommand>(tableFileName); break;
            case OPTION_PLAY: m_command = std ::make_unique<PlayTableCommand>(tableFileName); break;
            case OPTION_EDIT: m_command = std ::make_unique<Win32EditCommand>(tableFileName); break;
            case OPTION_AUDIT: m_command = std ::make_unique<AuditTableCommand>(tableFileName); break;
            case OPTION_POV: m_command = std ::make_unique<ExportPOVCommand>(tableFileName); break;
            case OPTION_EXTRACTVBS: m_command = std ::make_unique<ExportVBSCommand>(tableFileName); break;
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
         m_command = std::make_unique<CaptureAttractCommand>(tableFileName, captureAttract, captureAttractFPS, captureAttractLoop);
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
         m_command = std::make_unique<ValidateTournamentCommand>(tableFileName, tournamentFileName);
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

   if (!tableIniFileName.empty())
   {
      TableBasedCommand* tableCmd = m_command == nullptr ? nullptr : dynamic_cast<TableBasedCommand*>(m_command.get());
      if (tableCmd == nullptr)
      {
         OnCommandLineError("Command Line Error"s, "Option '"s + options[OPTION_TABLE_INI] + "' must be used in conjunction with a command that specifies a table filename");
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
