// license:GPLv3+

#pragma once


class AppCommand
{
public:
   virtual ~AppCommand() = 0;
   virtual void Execute() = 0;
};

class ShowInfoAndExitCommand : public AppCommand
{
public:
   ~ShowInfoAndExitCommand() override = default;
   ShowInfoAndExitCommand(const string& title, const string& message, int exitCode);
   void Execute() override;

private:
   const string m_title;
   const string m_message;
   const int m_exitCode;
};

class TableBasedCommand : public AppCommand
{
public:
   void SetTableIniFileName(const string& tableIniFileName) { m_tableIniFileName = tableIniFileName; }

protected:
   TableBasedCommand(const string& tableFilename)
      : m_tableFilename(tableFilename)
   {
   }

   string m_tableIniFileName;
   const string m_tableFilename;
};

class ExportVBSCommand : public TableBasedCommand
{
public:
   ExportVBSCommand(const string& tableFilename);
   ~ExportVBSCommand() override = default;
   void Execute() override { }
};

class ExportPOVCommand : public TableBasedCommand
{
public:
   ExportPOVCommand(const string& tableFilename);
   ~ExportPOVCommand() override = default;
   void Execute() override { }
};

class PlayTableCommand : public TableBasedCommand
{
public:
   PlayTableCommand(const string& tableFilename);
   ~PlayTableCommand() override = default;
   void Execute() override { }
};

class AuditTableCommand : public TableBasedCommand
{
public:
   AuditTableCommand(const string& tableFilename);
   ~AuditTableCommand() override = default;
   void Execute() override { }
};

class PovEditCommand : public TableBasedCommand
{
public:
   PovEditCommand(const string& tableFilename);
   ~PovEditCommand() override = default;
   void Execute() override { }
};

class Win32EditCommand : public TableBasedCommand
{
public:
   Win32EditCommand();
   Win32EditCommand(const string& tableFilename);
   ~Win32EditCommand() override = default;
   void Execute() override { }
};

class LiveEditCommand : public TableBasedCommand
{
public:
   LiveEditCommand();
   LiveEditCommand(const string& tableFilename);
   ~LiveEditCommand() override = default;
   void Execute() override { }
};

class CaptureAttractCommand : public TableBasedCommand
{
public:
   CaptureAttractCommand(const string& tableFilename, int captureAttract, int captureAttractFPS, bool captureAttractLoop);
   ~CaptureAttractCommand() override = default;
   void Execute();

   int m_captureAttract = 0; // Number of frames to capture for attract mode capture, 0 = disabled
   int m_captureAttractFPS = 0;
   bool m_captureAttractLoop = true;
};

class ValidateTournamentCommand : public TableBasedCommand
{
public:
   ValidateTournamentCommand(const string& tableFilename, const string& tournamentFilename);
   ~ValidateTournamentCommand() override = default;
   void Execute() override;

   const string m_tournamentFilename;
};

class CommandLineProcessor
{
public:
   CommandLineProcessor() = default;

   // Apply command line parameters to the app, and prepare the command to execute (if any)
   void ProcessCommandLine();
   void ProcessCommandLine(int argc, const char* argv[]);

   // command line parameters
   /* int m_captureAttract = 0; // Number of frames to capture for attract mode capture, 0 = disabled
   int m_captureAttractFPS = 0;
   bool m_captureAttractLoop = true;
   bool m_open_minimized = false;
   bool m_disable_pause_menu = false;
   bool m_povEdit = false; // table should be run in camera mode to change the POV (and then export that on exit), nothing else
   bool m_table_played_via_command_line = false;
   volatile bool m_table_played_via_SelectTableOnStart = false;
   bool m_run = true; // Should we run the main win32 UI or Player or just exit ?
   bool m_play = false;
   bool m_liveedit = false;
   bool m_extractPov = false;
   bool m_extractScript = false;
   bool m_audit = false;
   string m_tableFilename;
   string m_tableIniFileName;
   string m_tournamentFilename; // if not empty, tournament mode is/can be active
   */
   std::unique_ptr<AppCommand> m_command;

private:
   static const char** CommandLineToArgvA(const char* const CmdLine, int* const _argc);
   string GetPathFromArg(const string& arg, bool setCurrentPath);
   static string GetCommandLineHelp();
   static void OnCommandLineError(const string& title, const string& message);
};

