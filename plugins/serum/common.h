#pragma once

#include <string>
using std::string;

#ifdef _MSC_VER
#define PATH_SEPARATOR_CHAR '\\'
#define PATH_SEPARATOR_WCHAR L'\\'
#else
#define PATH_SEPARATOR_CHAR '/'
#define PATH_SEPARATOR_WCHAR L'/'
#endif

string find_directory_case_insensitive(const string& szParentPath, const string& szDirName);
