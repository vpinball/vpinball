// license:GPLv3+

#pragma once

#include "core/ScriptEngine.h"
#include "quickjs/quickjs.h"
#include "unordered_dense.h"

// JavaScript table script engine built on QuickJS-NG.
//
// Table objects (parts, collections, the global table, plugin objects, ...) are exposed to script as
// host objects wrapping their IDispatch. Property access resolves the member through GetIDsOfNames and
// decides between property and method by probing a DISPATCH_PROPERTYGET without arguments: a member
// that rejects it (DISP_E_UNKNOWNNAME, DISP_E_MEMBERNOTFOUND, DISP_E_BADPARAMCOUNT) is a method.
//
// The table script is evaluated as an ES module (imports resolve through the scripts folders). Events are
// dispatched by name: a handler registered with <item>.on("<EventName>", fn), an exported or global function
// called <ItemName>_<EventName>, is called with the item as 'this'. On standalone the generated COM proxies
// call the script dispatch by name, on Windows connection point sinks are advised.
class JSScriptEngine final : public IScriptEngine
{
public:
   JSScriptEngine();
   ~JSScriptEngine() override;

   ScriptLanguage GetLanguage() const override { return ScriptLanguage::JavaScript; }
   void Start(PinTable *table) override;
   void Stop(PinTable *table, bool interruptDirectly = false) override;
   using IScriptEngine::AddItem;
   void AddItem(const wstring &name, IDispatch *dispatch, const bool global) override;
   void RemoveItem(IScriptable *const piscript) override;
   void Evaluate(const string &script, bool isDebugStatement) override;
   bool HasError() const override { return m_hasError; }
   void GetScriptDispatch(IDispatch **ppdisp) const override;
   void SetScriptErrorHandler(const ErrorHandler &errorHandler) override { m_errorHandler = errorHandler; }
   void Dispose() override { delete this; }

   // Event dispatch by name (used by the script dispatch and the event sinks)
   int LookupHandler(const string &name);
   HRESULT CallHandler(int handlerId, DISPPARAMS *params, VARIANT *result);

private:
   enum class MemberKind : uint8_t
   {
      Unknown,
      Property,
      IndexedProperty, // property with arguments, called like a method but with DISPATCH_PROPERTYGET
      Method
   };
   struct Member;
   struct ComObject;
   class ScriptDispatch;
   class EventSink;

   struct NamedItem
   {
      string name;
      IDispatch *disp = nullptr;
      JSValue wrapper = JS_UNDEFINED;
      bool global = false;
#ifndef __STANDALONE__
      IConnectionPoint *connectionPoint = nullptr;
      DWORD cookie = 0;
      EventSink *sink = nullptr;
#endif
   };

   void RemoveNamedItem(NamedItem &item);

   // Wrapping and conversions
   JSValue Wrap(IDispatch *disp, const string &name = string());
   ComObject *GetComObject(JSValueConst obj) const;
   JSValue ToJS(const VARIANT &v);
   bool ToVariant(JSValueConst v, VARIANT &out);
   JSValue SafeArrayToJS(SAFEARRAY *sa);
   bool ArrayToSafeArray(JSValueConst arr, VARIANT &out);

   Member *ResolveMember(ComObject *co, const string &name);
   static string MemberKindKey(const ComObject *co, const string &lowerName);
   static bool IsNotAPropertyGet(HRESULT hr);
   HRESULT InvokeCom(IDispatch *disp, DISPID dispid, WORD flags, VARIANT *args, unsigned int nArgs, VARIANT *result, string &errorDesc);
   JSValue ThrowComError(const string &member, HRESULT hr, const string &desc);
   JSValue InvokeFromJS(ComObject *co, const string &name, WORD flags, int argc, JSValueConst *argv, bool allowPropertyGetFallback);

   // Error reporting
   void ReportException(ErrorType type);
   void ReportError(ErrorType type, JSValue exc);
   static char *ModuleNormalize(JSContext *ctx, const char *baseName, const char *name, void *opaque);
   static JSModuleDef *ModuleLoader(JSContext *ctx, const char *name, void *opaque);
   void PumpJobs();
   void MarkHandlersDirty() { m_globalFunctionsDirty = true; }
   void RefreshGlobalFunctions();

   // QuickJS class callbacks
   static void Finalizer(JSRuntime *rt, JSValueConst val);
   static void GCMark(JSRuntime *rt, JSValueConst val, JS_MarkFunc *mark_func);
   static JSValue GetProperty(JSContext *ctx, JSValueConst obj, JSAtom atom, JSValueConst receiver);
   static int SetProperty(JSContext *ctx, JSValueConst obj, JSAtom atom, JSValueConst value, JSValueConst receiver, int flags);
   static int HasProperty(JSContext *ctx, JSValueConst obj, JSAtom atom);
   static JSValue MethodCall(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv, int magic, JSValueConst *func_data);

   // Host functions
   static JSValue HostCreateObject(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
   static JSValue HostMsgBox(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
   static JSValue HostDebugPrint(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
   static JSValue HostConsole(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv, int magic);
   static JSValue HostOn(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv, int magic);
   static JSValue HostCall(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv, int magic);
   static JSValue HostEnum(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
   static JSValue HostName(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);

   static int InterruptHandler(JSRuntime *rt, void *opaque);
   static void PromiseRejectionTracker(JSContext *ctx, JSValueConst promise, JSValueConst reason, bool is_handled, void *opaque);

   JSRuntime *m_rt = nullptr;
   JSContext *m_ctx = nullptr;
   JSClassID m_classId = 0;
   JSValue m_helperProto = JS_UNDEFINED;
   JSValue m_globalObj = JS_UNDEFINED;
   JSValue m_tableModuleNs = JS_UNDEFINED; // namespace of the table script module
   ankerl::unordered_dense::map<string, string> m_moduleExports; // lower case -> exact export name

   vector<NamedItem> m_items;
   ankerl::unordered_dense::map<string, size_t> m_itemsByLowerName;

   // Handlers: id (index) -> lower case name
   vector<string> m_handlerNames;
   ankerl::unordered_dense::map<string, int> m_handlerIds;
   ankerl::unordered_dense::map<string, vector<JSValue>> m_listeners; // lower case "item_event" -> registered functions
   ankerl::unordered_dense::map<string, string> m_globalFunctions; // lower case -> exact global function name
   bool m_globalFunctionsDirty = true;

   ankerl::unordered_dense::map<string, MemberKind> m_memberKinds; // "<vtable>|<member>" -> probed kind, shared by objects of the same class

   ScriptDispatch *m_scriptDispatch = nullptr;
   ErrorHandler m_errorHandler;
   bool m_hasError = false;
   bool m_started = false;
   std::atomic<bool> m_interrupt = false;
};
