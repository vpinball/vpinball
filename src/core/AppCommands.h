// license:GPLv3+

#pragma once


class AppCommand
{
public:
   virtual ~AppCommand() = default;
   virtual void Execute() = 0;
};

class ShowInfoAndExitCommand : public AppCommand
{
public:
   ShowInfoAndExitCommand(const string& title, const string& message, int exitCode);
   ~ShowInfoAndExitCommand() override = default;
   void Execute() override;

private:
   const string m_title;
   const string m_message;
   const int m_exitCode;
};

class TableBasedCommand : public AppCommand
{
public:
   void SetTableIniFileName(const std::filesystem::path& tableIniFileName) { m_tableIniFileName = tableIniFileName; }

protected:
   explicit TableBasedCommand(const std::filesystem::path& tableFilename);
   CComObject<PinTable>* LoadTable();

   std::filesystem::path m_tableIniFileName;
   const std::filesystem::path m_tableFilename;
};

class ExportVBSCommand : public TableBasedCommand
{
public:
   explicit ExportVBSCommand(const std::filesystem::path& tableFilename);
   ~ExportVBSCommand() override = default;
   void Execute() override;
};

class ExportPOVCommand : public TableBasedCommand
{
public:
   explicit ExportPOVCommand(const std::filesystem::path& tableFilename);
   ~ExportPOVCommand() override = default;
   void Execute() override;
};

class PlayTableCommand : public TableBasedCommand
{
public:
   explicit PlayTableCommand(const std::filesystem::path& tableFilename);
   ~PlayTableCommand() override = default;
   void Execute() override;
};

class AuditTableCommand : public TableBasedCommand
{
public:
   explicit AuditTableCommand(const std::filesystem::path& tableFilename);
   ~AuditTableCommand() override = default;
   void Execute() override;
};

class PovEditCommand : public TableBasedCommand
{
public:
   explicit PovEditCommand(const std::filesystem::path& tableFilename);
   ~PovEditCommand() override = default;
   void Execute() override;
};

#ifndef __STANDALONE__
class Win32EditCommand : public TableBasedCommand
{
public:
   Win32EditCommand();
   explicit Win32EditCommand(const std::filesystem::path& tableFilename);
   ~Win32EditCommand() override = default;
   void Execute() override;

   // Legacy behavior, somewhat hacky/buggy as they rely on global options to get expected behavior
   bool m_minimized = false; // Run the editor minimized, usually in conjunction with the global option to select table on start
   bool m_disablePauseMenu = false; // Disable the pause menu in the editor, leading to escape key to exit
};
#endif

class LiveEditCommand : public TableBasedCommand
{
public:
   LiveEditCommand();
   explicit LiveEditCommand(const std::filesystem::path& tableFilename);
   ~LiveEditCommand() override = default;
   void Execute() override;
};

class CaptureAttractCommand : public TableBasedCommand
{
public:
   CaptureAttractCommand(const std::filesystem::path& tableFilename, int nFrames, int framesPerSecond, bool cutToLoop);
   ~CaptureAttractCommand() override = default;
   void Execute();

private:
   int m_nFrames = 0; // Number of frames to capture for attract mode capture, 0 = disabled
   int m_framesPerSecond = 0;
   bool m_cutToLoop = true;
};

class ValidateTournamentCommand : public TableBasedCommand
{
public:
   ValidateTournamentCommand(const std::filesystem::path& tableFilename, const std::filesystem::path& tournamentFilename);
   ~ValidateTournamentCommand() override = default;
   void Execute() override;

   const std::filesystem::path m_tournamentFilename;
};


class CommandLineProcessor
{
public:
   CommandLineProcessor() = default;

   // Apply command line parameters to the app, and prepare the command to execute (if any)
   void ProcessCommandLine();
   void ProcessCommandLine(int argc, const char* argv[]);

   std::unique_ptr<AppCommand> m_command;

private:
   static const char** CommandLineToArgvA(const char* const CmdLine, int* const _argc);
   std::filesystem::path GetPathFromArg(const string& arg);
   static string GetCommandLineHelp();
   static void OnCommandLineError(const string& title, const string& message);
};

