// license:GPLv3+

#pragma once

#include "core/Scriptable.h"

class PinTable;

enum class ScriptLanguage
{
   VBScript,
   JavaScript
};

// Host side interface to a table script engine. The engine receives the table objects as named items,
// evaluates the table script and reports errors through a single callback.
class IScriptEngine
{
public:
   enum class ErrorType
   {
      Compile,
      Runtime,
      DebugConsole
   };
   using ErrorHandler = std::function<void(ErrorType, int, int, const string &, const vector<string> &)>;

   virtual ~IScriptEngine() = default;

   virtual ScriptLanguage GetLanguage() const = 0;

   virtual void Start(PinTable *table) = 0;
   virtual void Stop(PinTable *table, bool interruptDirectly = false) = 0;
   void AddItem(IScriptable *scriptable, const bool global) { AddItem(scriptable->get_Name(), scriptable->GetIDispatch(), global); }
   virtual void AddItem(const wstring &name, IDispatch *dispatch, const bool global) = 0;
   virtual void RemoveItem(IScriptable *const piscript) = 0;
   virtual void Evaluate(const string &script, bool isDebugStatement) = 0;
   virtual bool HasError() const = 0;
   virtual void GetScriptDispatch(IDispatch **ppdisp) const = 0;
   virtual void SetScriptErrorHandler(const ErrorHandler &errorHandler) = 0;

   // Release the engine (engines manage their own lifetime, e.g. COM refcounting for the VBScript engine)
   virtual void Dispose() = 0;
};

// Language is selected by a directive on the first non blank line of the script, e.g. '// vpx-language: js'.
// Without directive, the script is VBScript.
ScriptLanguage DetectScriptLanguage(const string &script);
const char *GetScriptFileExtension(ScriptLanguage language);

IScriptEngine *CreateScriptEngine(ScriptLanguage language);
