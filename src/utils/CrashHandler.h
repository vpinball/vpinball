#pragma once

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
