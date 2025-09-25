#ifndef DEBUG_CRASH_HANDLER_H
#define DEBUG_CRASH_HANDLER_H

namespace rde
{
namespace CrashHandler
{
void Init();
// crash.txt by default
void SetCrashReportFileName(const string& name);
// crash.dmp by default
void SetMiniDumpFileName(const string& name);
}
}

#endif
