// license:GPLv3+

#include "core/stdafx.h"
#include "ScriptEngine.h"

#include "core/ScriptInterpreter.h"

#include <regex>

ScriptLanguage DetectScriptLanguage(const string &script)
{
   // Find the first non blank line
   size_t start = 0;
   while (start < script.length())
   {
      const size_t end = script.find('\n', start);
      const string line = script.substr(start, end == string::npos ? string::npos : end - start);
      if (line.find_first_not_of(" \t\r") != string::npos)
      {
         static const std::regex re(R"(^\s*(?:'|//|#!|#|--)\s*vpx-language\s*[:=]\s*([A-Za-z]+))", std::regex::icase);
         std::smatch m;
         if (std::regex_search(line, m, re))
         {
            const string lang = lowerCase(m[1].str());
            if (lang == "js" || lang == "javascript")
               return ScriptLanguage::JavaScript;
         }
         return ScriptLanguage::VBScript;
      }
      if (end == string::npos)
         break;
      start = end + 1;
   }
   return ScriptLanguage::VBScript;
}

const char *GetScriptFileExtension(ScriptLanguage language)
{
   return language == ScriptLanguage::JavaScript ? ".js" : ".vbs";
}

IScriptEngine *CreateScriptEngine(ScriptLanguage language)
{
   if (language == ScriptLanguage::JavaScript)
      PLOGE << "Table script is JavaScript but this build has no JavaScript engine, falling back to VBScript";
   CComObject<ScriptInterpreter> *interpreter;
   CComObject<ScriptInterpreter>::CreateInstance(&interpreter);
   interpreter->AddRef();
   return interpreter;
}
