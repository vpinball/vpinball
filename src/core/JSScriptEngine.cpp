// license:GPLv3+

#include "core/stdafx.h"
#include "JSScriptEngine.h"

#include "core/VPApp.h"
#include "core/player.h"
#include "core/ScriptGlobalTable.h"
#include "core/DynamicScript.h"
#include "parts/Collection.h"
#include "parts/pintable.h"
#include "ui/live/LiveUI.h"
#include "ui/win/WinEditor.h"

#include <fstream>
#include <sstream>
#include <regex>

#ifdef __STANDALONE__
#include "libwinevbs/libwinevbs.h"
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
// Internal types

struct JSScriptEngine::Member
{
   DISPID dispid = DISPID_UNKNOWN;
   MemberKind kind = MemberKind::Unknown;
   JSValue fn = JS_UNDEFINED; // cached bound method
};

struct JSScriptEngine::ComObject
{
   IDispatch *disp = nullptr; // AddRef'ed, nullptr once the item was removed
   string name; // named item name, empty for objects returned by calls
   ankerl::unordered_dense::map<string, Member> members; // lower case member name -> member
};

// IDispatch handed to the host (see GetScriptDispatch): resolves and calls global script functions by name.
// This is how the standalone COM proxies fire events (Flipper::FireDispID calls <Name>_<Event>) and how the
// table calls OnBallBallCollision.
class JSScriptEngine::ScriptDispatch final : public IDispatch
{
public:
   explicit ScriptDispatch(JSScriptEngine *engine)
      : m_engine(engine)
   {
   }
   void Detach() { m_engine = nullptr; }

   HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject) override
   {
      if (riid == IID_IUnknown || riid == IID_IDispatch)
      {
         *ppvObject = static_cast<IDispatch *>(this);
         AddRef();
         return S_OK;
      }
      *ppvObject = nullptr;
      return E_NOINTERFACE;
   }
   ULONG STDMETHODCALLTYPE AddRef() override { return ++m_refCount; }
   ULONG STDMETHODCALLTYPE Release() override
   {
      const ULONG refCount = --m_refCount;
      if (refCount == 0)
         delete this;
      return refCount;
   }
   HRESULT STDMETHODCALLTYPE GetTypeInfoCount(UINT *pctinfo) override
   {
      *pctinfo = 0;
      return S_OK;
   }
   HRESULT STDMETHODCALLTYPE GetTypeInfo(UINT, LCID, ITypeInfo **ppTInfo) override
   {
      *ppTInfo = nullptr;
      return E_NOTIMPL;
   }
   HRESULT STDMETHODCALLTYPE GetIDsOfNames(REFIID, LPOLESTR *rgszNames, UINT cNames, LCID, DISPID *rgDispId) override
   {
      if (m_engine == nullptr || cNames < 1)
         return E_FAIL;
      const int id = m_engine->LookupHandler(MakeString(wstring(rgszNames[0])));
      if (id < 0)
         return DISP_E_UNKNOWNNAME;
      rgDispId[0] = id;
      return S_OK;
   }
   HRESULT STDMETHODCALLTYPE Invoke(DISPID dispIdMember, REFIID, LCID, WORD, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *, UINT *) override
   {
      if (m_engine == nullptr)
         return E_FAIL;
      return m_engine->CallHandler(dispIdMember, pDispParams, pVarResult);
   }

private:
   ULONG m_refCount = 1;
   JSScriptEngine *m_engine;
};

#ifndef __STANDALONE__
// Connection point sink used on Windows where parts fire events through ATL connection points.
// Note: this path is not exercised by the standalone build and is untested.
class JSScriptEngine::EventSink final : public IDispatch
{
public:
   EventSink(JSScriptEngine *engine, const string &itemName)
      : m_engine(engine)
      , m_itemName(lowerCase(itemName))
   {
   }
   void Detach() { m_engine = nullptr; }

   HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject) override
   {
      // Events are delivered through the source dispinterface, which is IDispatch based, so accept any IID
      *ppvObject = static_cast<IDispatch *>(this);
      AddRef();
      return S_OK;
   }
   ULONG STDMETHODCALLTYPE AddRef() override { return ++m_refCount; }
   ULONG STDMETHODCALLTYPE Release() override
   {
      const ULONG refCount = --m_refCount;
      if (refCount == 0)
         delete this;
      return refCount;
   }
   HRESULT STDMETHODCALLTYPE GetTypeInfoCount(UINT *pctinfo) override
   {
      *pctinfo = 0;
      return S_OK;
   }
   HRESULT STDMETHODCALLTYPE GetTypeInfo(UINT, LCID, ITypeInfo **ppTInfo) override
   {
      *ppTInfo = nullptr;
      return E_NOTIMPL;
   }
   HRESULT STDMETHODCALLTYPE GetIDsOfNames(REFIID, LPOLESTR *, UINT, LCID, DISPID *) override { return E_NOTIMPL; }
   HRESULT STDMETHODCALLTYPE Invoke(DISPID dispIdMember, REFIID, LCID, WORD, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *, UINT *) override
   {
      if (m_engine == nullptr)
         return E_FAIL;
      static const struct
      {
         DISPID dispid;
         const char *name;
      } events[] = {
         { DISPID_GameEvents_KeyDown, "keydown" }, { DISPID_GameEvents_KeyUp, "keyup" }, { DISPID_GameEvents_Init, "init" }, { DISPID_GameEvents_MusicDone, "musicdone" },
         { DISPID_GameEvents_Exit, "exit" }, { DISPID_GameEvents_Paused, "paused" }, { DISPID_GameEvents_UnPaused, "unpaused" }, { DISPID_GameEvents_OptionEvent, "optionevent" },
         { DISPID_GameEvents_SoundDone, "sounddone" }, { DISPID_SurfaceEvents_Slingshot, "slingshot" }, { DISPID_FlipperEvents_Collide, "collide" }, { DISPID_TimerEvents_Timer, "timer" },
         { DISPID_SpinnerEvents_Spin, "spin" }, { DISPID_TargetEvents_Dropped, "dropped" }, { DISPID_TargetEvents_Raised, "raised" }, { DISPID_LightSeqEvents_PlayDone, "playdone" },
         { DISPID_HitEvents_Hit, "hit" }, { DISPID_HitEvents_Unhit, "unhit" }, { DISPID_LimitEvents_EOS, "limiteos" }, { DISPID_LimitEvents_BOS, "limitbos" },
         { DISPID_AnimateEvents_Animate, "animate" },
      };
      for (const auto &evt : events)
         if (evt.dispid == dispIdMember)
         {
            const int id = m_engine->LookupHandler(m_itemName + '_' + evt.name);
            return id < 0 ? S_OK : m_engine->CallHandler(id, pDispParams, pVarResult);
         }
      return S_OK;
   }

private:
   ULONG m_refCount = 1;
   JSScriptEngine *m_engine;
   const string m_itemName;
};
#endif


///////////////////////////////////////////////////////////////////////////////////////////////////
// Engine lifecycle

static JSClassID s_comClassId = 0;

// Shared prototype of all host objects: helper methods that are not COM members
static const char *const s_helperProtoScript = R"JS(
(function() {
   return {
      on(event, fn) { __vpx_on(this, event, fn); return this; },
      off(event, fn) { __vpx_off(this, event, fn); return this; },
      addEventListener(event, fn) { __vpx_on(this, event, fn); },
      removeEventListener(event, fn) { __vpx_off(this, event, fn); },
      $call(name, ...args) { return __vpx_call(this, name, args); },
      $get(name, ...args) { return __vpx_get(this, name, args); },
      $set(name, ...args) { __vpx_set(this, name, args); },
      [Symbol.iterator]() { return __vpx_enum(this)[Symbol.iterator](); },
      toString() { return '[VPXObject ' + __vpx_name(this) + ']'; },
      get [Symbol.toStringTag]() { return 'VPXObject'; },
   };
})()
)JS";

JSScriptEngine::JSScriptEngine()
{
   m_rt = JS_NewRuntime();
   JS_SetRuntimeOpaque(m_rt, this);
   JS_SetMaxStackSize(m_rt, 4 * 1024 * 1024);
   JS_SetInterruptHandler(m_rt, InterruptHandler, this);
   JS_SetHostPromiseRejectionTracker(m_rt, PromiseRejectionTracker, this);

   m_ctx = JS_NewContext(m_rt);
   JS_SetContextOpaque(m_ctx, this);

   JS_NewClassID(m_rt, &s_comClassId);
   static JSClassExoticMethods exotic = { nullptr, nullptr, nullptr, nullptr, HasProperty, GetProperty, SetProperty };
   static const JSClassDef classDef = { "VPXObject", Finalizer, GCMark, nullptr, &exotic };
   JS_NewClass(m_rt, s_comClassId, &classDef);
   m_classId = s_comClassId;

   m_globalObj = JS_GetGlobalObject(m_ctx);

   JS_SetModuleLoaderFunc(m_rt, ModuleNormalize, ModuleLoader, this);

   // Host functions
   JS_SetPropertyStr(m_ctx, m_globalObj, "CreateObject", JS_NewCFunction(m_ctx, HostCreateObject, "CreateObject", 1));
   JS_SetPropertyStr(m_ctx, m_globalObj, "MsgBox", JS_NewCFunction(m_ctx, HostMsgBox, "MsgBox", 1));
   JS_SetPropertyStr(m_ctx, m_globalObj, "__vpx_on", JS_NewCFunctionMagic(m_ctx, HostOn, "__vpx_on", 3, JS_CFUNC_generic_magic, 1));
   JS_SetPropertyStr(m_ctx, m_globalObj, "__vpx_off", JS_NewCFunctionMagic(m_ctx, HostOn, "__vpx_off", 3, JS_CFUNC_generic_magic, 0));
   JS_SetPropertyStr(m_ctx, m_globalObj, "__vpx_call", JS_NewCFunctionMagic(m_ctx, HostCall, "__vpx_call", 3, JS_CFUNC_generic_magic, DISPATCH_METHOD));
   JS_SetPropertyStr(m_ctx, m_globalObj, "__vpx_get", JS_NewCFunctionMagic(m_ctx, HostCall, "__vpx_get", 3, JS_CFUNC_generic_magic, DISPATCH_PROPERTYGET));
   JS_SetPropertyStr(m_ctx, m_globalObj, "__vpx_set", JS_NewCFunctionMagic(m_ctx, HostCall, "__vpx_set", 3, JS_CFUNC_generic_magic, DISPATCH_PROPERTYPUT));
   JS_SetPropertyStr(m_ctx, m_globalObj, "__vpx_enum", JS_NewCFunction(m_ctx, HostEnum, "__vpx_enum", 1));
   JS_SetPropertyStr(m_ctx, m_globalObj, "__vpx_name", JS_NewCFunction(m_ctx, HostName, "__vpx_name", 1));

   JSValue console = JS_NewObject(m_ctx);
   JS_SetPropertyStr(m_ctx, console, "log", JS_NewCFunctionMagic(m_ctx, HostConsole, "log", 1, JS_CFUNC_generic_magic, 0));
   JS_SetPropertyStr(m_ctx, console, "info", JS_NewCFunctionMagic(m_ctx, HostConsole, "info", 1, JS_CFUNC_generic_magic, 0));
   JS_SetPropertyStr(m_ctx, console, "debug", JS_NewCFunctionMagic(m_ctx, HostConsole, "debug", 1, JS_CFUNC_generic_magic, 1));
   JS_SetPropertyStr(m_ctx, console, "warn", JS_NewCFunctionMagic(m_ctx, HostConsole, "warn", 1, JS_CFUNC_generic_magic, 2));
   JS_SetPropertyStr(m_ctx, console, "error", JS_NewCFunctionMagic(m_ctx, HostConsole, "error", 1, JS_CFUNC_generic_magic, 3));
   JS_SetPropertyStr(m_ctx, m_globalObj, "console", console);

   // Debug.Print, like the VBScript engine's Debug object
   JSValue debug = JS_NewObject(m_ctx);
   JS_SetPropertyStr(m_ctx, debug, "Print", JS_NewCFunction(m_ctx, HostDebugPrint, "Print", 1));
   JS_SetPropertyStr(m_ctx, m_globalObj, "Debug", debug);

   m_helperProto = JS_Eval(m_ctx, s_helperProtoScript, strlen(s_helperProtoScript), "<vpx>", JS_EVAL_TYPE_GLOBAL);
   if (JS_IsException(m_helperProto))
   {
      ReportException(ErrorType::Compile);
      m_helperProto = JS_NewObject(m_ctx);
   }

   m_scriptDispatch = new ScriptDispatch(this);
}

JSScriptEngine::~JSScriptEngine()
{
   for (NamedItem &item : m_items)
      RemoveNamedItem(item);
   m_items.clear();
   m_itemsByLowerName.clear();

   for (auto &entry : m_listeners)
      for (JSValue fn : entry.second)
         JS_FreeValue(m_ctx, fn);
   m_listeners.clear();

   if (m_scriptDispatch)
   {
      m_scriptDispatch->Detach();
      m_scriptDispatch->Release();
      m_scriptDispatch = nullptr;
   }

   JS_FreeValue(m_ctx, m_tableModuleNs);
   JS_FreeValue(m_ctx, m_helperProto);
   JS_FreeValue(m_ctx, m_globalObj);
   JS_FreeContext(m_ctx);
   JS_FreeRuntime(m_rt);
}

void JSScriptEngine::Start(PinTable *table)
{
   AddItem(table, false);
   AddItem((ScriptGlobalTable *)table->m_psgt, true);
   for (int i = 0; i < table->m_vcollection.size(); i++)
      AddItem(&table->m_vcollection[i], false);
   for (auto editable : table->GetParts())
      if (editable->GetIScriptable())
         AddItem(editable->GetIScriptable(), false);
   m_started = true;
}

void JSScriptEngine::Stop(PinTable *table, bool interruptDirectly)
{
   if (interruptDirectly)
      m_interrupt = true;
   for (NamedItem &item : m_items)
      RemoveNamedItem(item);
   m_items.clear();
   m_itemsByLowerName.clear();
   m_started = false;
}

void JSScriptEngine::AddItem(const wstring &wname, IDispatch *dispatch, const bool global)
{
   const string name = MakeString(wname);
   const string lowerName = lowerCase(name);
   if (m_itemsByLowerName.contains(lowerName))
   {
      PLOGE << "Script item with name '" << name << "' already exists. Skipping addition of this item.";
      return;
   }

   NamedItem item;
   item.name = name;
   item.global = global;
   item.disp = dispatch;
   item.disp->AddRef();
   item.wrapper = Wrap(dispatch, name);
   if (global)
   {
      // Members of the global item are resolved as bare names: make the wrapper the prototype of the global object
      JS_SetPrototype(m_ctx, m_globalObj, item.wrapper);
   }
   else
   {
      JS_DefinePropertyValueStr(m_ctx, m_globalObj, name.c_str(), JS_DupValue(m_ctx, item.wrapper), JS_PROP_C_W_E);
   }

#ifndef __STANDALONE__
   // Advise an event sink on the first connection point of the object (parts have a single source interface)
   IConnectionPointContainer *cpc = nullptr;
   if (SUCCEEDED(dispatch->QueryInterface(IID_IConnectionPointContainer, (void **)&cpc)) && cpc)
   {
      IEnumConnectionPoints *cpEnum = nullptr;
      if (SUCCEEDED(cpc->EnumConnectionPoints(&cpEnum)) && cpEnum)
      {
         IConnectionPoint *cp = nullptr;
         ULONG fetched = 0;
         if (cpEnum->Next(1, &cp, &fetched) == S_OK && fetched == 1)
         {
            item.sink = new EventSink(this, name);
            if (SUCCEEDED(cp->Advise(item.sink, &item.cookie)))
               item.connectionPoint = cp;
            else
            {
               cp->Release();
               item.sink->Release();
               item.sink = nullptr;
            }
         }
         cpEnum->Release();
      }
      cpc->Release();
   }
#endif

   m_itemsByLowerName[lowerName] = m_items.size();
   m_items.push_back(item);
}

void JSScriptEngine::RemoveNamedItem(NamedItem &item)
{
   if (item.disp == nullptr)
      return;
#ifndef __STANDALONE__
   if (item.connectionPoint)
   {
      item.connectionPoint->Unadvise(item.cookie);
      item.connectionPoint->Release();
      item.connectionPoint = nullptr;
   }
   if (item.sink)
   {
      item.sink->Detach();
      item.sink->Release();
      item.sink = nullptr;
   }
#endif
   if (item.global)
   {
      JSValue objectProto = JS_GetPropertyStr(m_ctx, m_globalObj, "Object");
      JSValue proto = JS_GetPropertyStr(m_ctx, objectProto, "prototype");
      JS_SetPrototype(m_ctx, m_globalObj, proto);
      JS_FreeValue(m_ctx, proto);
      JS_FreeValue(m_ctx, objectProto);
   }
   else
   {
      JSAtom atom = JS_NewAtom(m_ctx, item.name.c_str());
      JS_DeleteProperty(m_ctx, m_globalObj, atom, 0);
      JS_FreeAtom(m_ctx, atom);
   }
   // Invalidate the wrapper: script may still hold references to it
   if (ComObject *co = GetComObject(item.wrapper); co && co->disp)
   {
      co->disp->Release();
      co->disp = nullptr;
   }
   JS_FreeValue(m_ctx, item.wrapper);
   item.wrapper = JS_UNDEFINED;
   item.disp->Release();
   item.disp = nullptr;
}

void JSScriptEngine::RemoveItem(IScriptable *const piscript)
{
   const string lowerName = lowerCase(MakeString(piscript->get_Name()));
   const auto it = m_itemsByLowerName.find(lowerName);
   if (it == m_itemsByLowerName.end())
      return;
   RemoveNamedItem(m_items[it->second]);
   m_itemsByLowerName.erase(it);
}

void JSScriptEngine::Evaluate(const string &script, bool isDebugStatement)
{
   if (isDebugStatement)
   {
      // Debug console statements are evaluated in the global scope
      const JSValue result = JS_Eval(m_ctx, script.c_str(), script.length(), "<debug>", JS_EVAL_TYPE_GLOBAL);
      MarkHandlersDirty();
      if (JS_IsException(result))
         ReportException(ErrorType::DebugConsole);
      else if (!JS_IsUndefined(result))
      {
         const char *str = JS_ToCString(m_ctx, result);
         PLOGI << "Script.Print '" << (str ? str : "") << '\'';
         JS_FreeCString(m_ctx, str);
      }
      JS_FreeValue(m_ctx, result);
      PumpJobs();
      return;
   }

   // The table script is an ES module: strict mode, imports from the scripts folders, top level await.
   // Its exports are inspected for event handlers.
   const JSValue moduleFunc = JS_Eval(m_ctx, script.c_str(), script.length(), "<table>", JS_EVAL_TYPE_MODULE | JS_EVAL_FLAG_COMPILE_ONLY);
   if (JS_IsException(moduleFunc))
   {
      ReportException(ErrorType::Compile);
      return;
   }
   JSModuleDef *const module = static_cast<JSModuleDef *>(JS_VALUE_GET_PTR(moduleFunc));
   const JSValue promise = JS_EvalFunction(m_ctx, moduleFunc); // takes ownership of moduleFunc
   MarkHandlersDirty();
   if (JS_IsException(promise))
      ReportException(ErrorType::Compile);
   else
   {
      PumpJobs();
      switch (JS_PromiseState(m_ctx, promise))
      {
      case JS_PROMISE_REJECTED: ReportError(ErrorType::Compile, JS_PromiseResult(m_ctx, promise)); break;
      case JS_PROMISE_PENDING: ReportError(ErrorType::Compile, JS_NewString(m_ctx, "Table script did not finish evaluating (pending top level await)")); break;
      default: break;
      }
      JS_FreeValue(m_ctx, promise);
   }
   JS_FreeValue(m_ctx, m_tableModuleNs);
   m_tableModuleNs = JS_GetModuleNamespace(m_ctx, module);
   m_moduleExports.clear();
   if (JS_IsObject(m_tableModuleNs))
   {
      JSPropertyEnum *tab = nullptr;
      uint32_t len = 0;
      if (JS_GetOwnPropertyNames(m_ctx, &tab, &len, m_tableModuleNs, JS_GPN_STRING_MASK) >= 0)
      {
         for (uint32_t i = 0; i < len; i++)
         {
            const char *name = JS_AtomToCString(m_ctx, tab[i].atom);
            if (name)
               m_moduleExports[lowerCase(string(name))] = name;
            JS_FreeCString(m_ctx, name);
         }
         JS_FreePropertyEnum(m_ctx, tab, len);
      }
   }
   PumpJobs();
}

// Module specifiers: "./x.js" and "../x.js" are resolved against the importing module, bare names ("controller.js")
// are searched in the table, user and application scripts folders when the module is loaded.
char *JSScriptEngine::ModuleNormalize(JSContext *ctx, const char *baseName, const char *name, void *opaque)
{
   const string specifier(name);
   if (specifier.starts_with("./") || specifier.starts_with("../"))
   {
      const std::filesystem::path base(baseName);
      const std::filesystem::path resolved = (base.parent_path() / specifier).lexically_normal();
      return js_strdup(ctx, resolved.string().c_str());
   }
   return js_strdup(ctx, name);
}

JSModuleDef *JSScriptEngine::ModuleLoader(JSContext *ctx, const char *name, void *opaque)
{
   std::filesystem::path file(normalize_path_separators(name));
   if (!file.is_absolute() || !std::filesystem::exists(file))
   {
      const PinTable *const table = g_pplayer ? g_pplayer->m_ptable : g_pvp ? g_pvp->GetActiveTable() : nullptr;
      file = g_app->m_fileLocator.SearchScript(table, file);
   }
   if (file.empty())
   {
      JS_ThrowReferenceError(ctx, "could not find module '%s'", name);
      return nullptr;
   }
   std::ifstream scriptFile(file, std::ifstream::in);
   if (!scriptFile.is_open())
   {
      JS_ThrowReferenceError(ctx, "could not read module '%s'", file.string().c_str());
      return nullptr;
   }
   std::stringstream buffer;
   buffer << scriptFile.rdbuf();
   const string content = buffer.str();
   PLOGI << "Loading module: " << file.string();
   const JSValue moduleFunc = JS_Eval(ctx, content.c_str(), content.length(), name, JS_EVAL_TYPE_MODULE | JS_EVAL_FLAG_COMPILE_ONLY);
   if (JS_IsException(moduleFunc))
      return nullptr;
   JSModuleDef *const module = static_cast<JSModuleDef *>(JS_VALUE_GET_PTR(moduleFunc));
   JS_FreeValue(ctx, moduleFunc);
   return module;
}

void JSScriptEngine::GetScriptDispatch(IDispatch **ppdisp) const
{
   *ppdisp = m_scriptDispatch;
   if (m_scriptDispatch)
      m_scriptDispatch->AddRef();
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Event handlers

void JSScriptEngine::RefreshGlobalFunctions()
{
   if (!m_globalFunctionsDirty)
      return;
   m_globalFunctionsDirty = false;
   m_globalFunctions.clear();
   JSPropertyEnum *tab = nullptr;
   uint32_t len = 0;
   if (JS_GetOwnPropertyNames(m_ctx, &tab, &len, m_globalObj, JS_GPN_STRING_MASK) < 0)
      return;
   for (uint32_t i = 0; i < len; i++)
   {
      JSValue v = JS_GetProperty(m_ctx, m_globalObj, tab[i].atom);
      if (JS_IsFunction(m_ctx, v))
      {
         const char *name = JS_AtomToCString(m_ctx, tab[i].atom);
         if (name)
            m_globalFunctions[lowerCase(string(name))] = name;
         JS_FreeCString(m_ctx, name);
      }
      JS_FreeValue(m_ctx, v);
   }
   JS_FreePropertyEnum(m_ctx, tab, len);
}

int JSScriptEngine::LookupHandler(const string &name)
{
   const string lowerName = lowerCase(name);
   if (const auto it = m_handlerIds.find(lowerName); it != m_handlerIds.end())
      return it->second;
   bool found = m_listeners.contains(lowerName) || m_moduleExports.contains(lowerName);
   if (!found)
   {
      RefreshGlobalFunctions();
      found = m_globalFunctions.contains(lowerName);
   }
   if (!found)
      return -1;
   const int id = static_cast<int>(m_handlerNames.size());
   m_handlerNames.push_back(lowerName);
   m_handlerIds[lowerName] = id;
   return id;
}

HRESULT JSScriptEngine::CallHandler(int handlerId, DISPPARAMS *params, VARIANT *result)
{
   if (handlerId < 0 || handlerId >= static_cast<int>(m_handlerNames.size()))
      return DISP_E_MEMBERNOTFOUND;
   const string &lowerName = m_handlerNames[handlerId];

   // Arguments are passed in reverse order in DISPPARAMS
   const unsigned int nArgs = params ? params->cArgs : 0;
   vector<JSValue> args(nArgs);
   for (unsigned int i = 0; i < nArgs; i++)
      args[i] = ToJS(params->rgvarg[nArgs - 1 - i]);

   // 'this' is the item the event belongs to (name before the last underscore)
   JSValue thisVal = JS_UNDEFINED;
   if (const size_t sep = lowerName.rfind('_'); sep != string::npos)
      if (const auto it = m_itemsByLowerName.find(lowerName.substr(0, sep)); it != m_itemsByLowerName.end())
         thisVal = m_items[it->second].wrapper;

   bool called = false;
   JSValue ret = JS_UNDEFINED;
   if (const auto it = m_listeners.find(lowerName); it != m_listeners.end())
   {
      const vector<JSValue> listeners = it->second; // copy, a listener may unregister itself
      for (JSValue fn : listeners)
      {
         JSValue r = JS_Call(m_ctx, fn, thisVal, static_cast<int>(nArgs), args.data());
         if (JS_IsException(r))
            ReportException(ErrorType::Runtime);
         JS_FreeValue(m_ctx, r);
         called = true;
      }
   }
   // Exported handler of the table module, else a global function
   JSValue fn = JS_UNDEFINED;
   if (const auto it = m_moduleExports.find(lowerName); it != m_moduleExports.end())
      fn = JS_GetPropertyStr(m_ctx, m_tableModuleNs, it->second.c_str());
   if (!JS_IsFunction(m_ctx, fn))
   {
      JS_FreeValue(m_ctx, fn);
      fn = JS_UNDEFINED;
      RefreshGlobalFunctions();
      if (const auto it = m_globalFunctions.find(lowerName); it != m_globalFunctions.end())
         fn = JS_GetPropertyStr(m_ctx, m_globalObj, it->second.c_str());
   }
   {
      if (JS_IsFunction(m_ctx, fn))
      {
         ret = JS_Call(m_ctx, fn, thisVal, static_cast<int>(nArgs), args.data());
         if (JS_IsException(ret))
         {
            ReportException(ErrorType::Runtime);
            ret = JS_UNDEFINED;
         }
         called = true;
      }
      JS_FreeValue(m_ctx, fn);
   }

   for (JSValue v : args)
      JS_FreeValue(m_ctx, v);
   if (result)
   {
      VariantInit(result);
      if (!JS_IsUndefined(ret) && !ToVariant(ret, *result))
         ReportException(ErrorType::Runtime);
   }
   JS_FreeValue(m_ctx, ret);
   PumpJobs();
   return called ? S_OK : DISP_E_MEMBERNOTFOUND;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Wrapping and conversions

JSScriptEngine::ComObject *JSScriptEngine::GetComObject(JSValueConst obj) const
{
   return static_cast<ComObject *>(JS_GetOpaque(obj, m_classId));
}

JSValue JSScriptEngine::Wrap(IDispatch *disp, const string &name)
{
   if (disp == nullptr)
      return JS_NULL;
   JSValue obj = JS_NewObjectProtoClass(m_ctx, m_helperProto, m_classId);
   if (JS_IsException(obj))
      return obj;
   ComObject *co = new ComObject();
   co->disp = disp;
   co->disp->AddRef();
   co->name = name;
   JS_SetOpaque(obj, co);
   return obj;
}

void JSScriptEngine::Finalizer(JSRuntime *rt, JSValueConst val)
{
   ComObject *co = static_cast<ComObject *>(JS_GetOpaque(val, s_comClassId));
   if (co == nullptr)
      return;
   for (auto &member : co->members)
      JS_FreeValueRT(rt, member.second.fn);
   if (co->disp)
      co->disp->Release();
   delete co;
}

// Cached bound methods reference the object, so let the cycle collector see them
void JSScriptEngine::GCMark(JSRuntime *rt, JSValueConst val, JS_MarkFunc *mark_func)
{
   ComObject *co = static_cast<ComObject *>(JS_GetOpaque(val, s_comClassId));
   if (co == nullptr)
      return;
   for (auto &member : co->members)
      JS_MarkValue(rt, member.second.fn, mark_func);
}

// A SAFEARRAY becomes an array; a multi dimensional one an array of arrays (ChangedLamps and friends are n x 2)
JSValue JSScriptEngine::SafeArrayToJS(SAFEARRAY *sa)
{
   JSValue arr = JS_NewArray(m_ctx);
   if (sa == nullptr)
      return arr;
   const UINT dims = SafeArrayGetDim(sa);
   if (dims == 0 || dims > 4)
      return arr;
   VARTYPE vt = VT_EMPTY;
   SafeArrayGetVartype(sa, &vt);
   LONG lBounds[4] = {}, uBounds[4] = {};
   for (UINT d = 1; d <= dims; d++)
   {
      SafeArrayGetLBound(sa, d, &lBounds[d - 1]);
      SafeArrayGetUBound(sa, d, &uBounds[d - 1]);
   }
   LONG indices[4] = {};
   const std::function<void(JSValue, UINT)> fill = [&](JSValue target, UINT dim)
   {
      uint32_t n = 0;
      for (LONG i = lBounds[dim]; i <= uBounds[dim]; i++)
      {
         indices[dim] = i;
         if (dim + 1 < dims)
         {
            JSValue sub = JS_NewArray(m_ctx);
            fill(sub, dim + 1);
            JS_SetPropertyUint32(m_ctx, target, n++, sub);
            continue;
         }
         VARIANT e;
         VariantInit(&e);
         if (vt == VT_VARIANT)
            SafeArrayGetElement(sa, indices, &e);
         else
         {
            V_VT(&e) = vt;
            SafeArrayGetElement(sa, indices, &V_I4(&e)); // all union members share the same address
         }
         JS_SetPropertyUint32(m_ctx, target, n++, ToJS(e));
         VariantClear(&e);
      }
   };
   fill(arr, 0);
   return arr;
}

JSValue JSScriptEngine::ToJS(const VARIANT &v)
{
   if (V_VT(&v) & VT_BYREF)
   {
      VARIANT tmp;
      VariantInit(&tmp);
      if (FAILED(VariantCopyInd(&tmp, &v)))
         return JS_UNDEFINED;
      JSValue r = ToJS(tmp);
      VariantClear(&tmp);
      return r;
   }
   if (V_VT(&v) & VT_ARRAY)
      return SafeArrayToJS(V_ARRAY(&v));
   switch (V_VT(&v))
   {
   case VT_EMPTY: return JS_UNDEFINED;
   case VT_NULL: return JS_NULL;
   case VT_BOOL: return JS_NewBool(m_ctx, V_BOOL(&v) != VARIANT_FALSE);
   case VT_I1: return JS_NewInt32(m_ctx, V_I1(&v));
   case VT_UI1: return JS_NewInt32(m_ctx, V_UI1(&v));
   case VT_I2: return JS_NewInt32(m_ctx, V_I2(&v));
   case VT_UI2: return JS_NewInt32(m_ctx, V_UI2(&v));
   case VT_I4: return JS_NewInt32(m_ctx, V_I4(&v));
   case VT_INT: return JS_NewInt32(m_ctx, V_INT(&v));
   case VT_UI4: return JS_NewInt64(m_ctx, V_UI4(&v));
   case VT_UINT: return JS_NewInt64(m_ctx, V_UINT(&v));
   case VT_I8: return JS_NewInt64(m_ctx, V_I8(&v));
   case VT_UI8: return JS_NewFloat64(m_ctx, static_cast<double>(V_UI8(&v)));
   case VT_R4: return JS_NewFloat64(m_ctx, V_R4(&v));
   case VT_R8: return JS_NewFloat64(m_ctx, V_R8(&v));
   case VT_DATE: return JS_NewFloat64(m_ctx, V_DATE(&v));
   case VT_BSTR:
   {
      const string s = V_BSTR(&v) ? MakeString(V_BSTR(&v)) : string();
      return JS_NewStringLen(m_ctx, s.c_str(), s.length());
   }
   case VT_DISPATCH: return Wrap(V_DISPATCH(&v));
   case VT_UNKNOWN:
   {
      if (V_UNKNOWN(&v) == nullptr)
         return JS_NULL;
      IDispatch *disp = nullptr;
      if (SUCCEEDED(V_UNKNOWN(&v)->QueryInterface(IID_IDispatch, (void **)&disp)) && disp)
      {
         JSValue r = Wrap(disp);
         disp->Release();
         return r;
      }
      return JS_NULL;
   }
   case VT_ERROR: return JS_UNDEFINED;
   default:
   {
      VARIANT tmp;
      VariantInit(&tmp);
      if (SUCCEEDED(VariantChangeType(&tmp, &v, 0, VT_R8)))
         return JS_NewFloat64(m_ctx, V_R8(&tmp));
      PLOGE << "Unsupported VARIANT type " << V_VT(&v) << " passed to JavaScript";
      return JS_UNDEFINED;
   }
   }
}

bool JSScriptEngine::ArrayToSafeArray(JSValueConst arr, VARIANT &out)
{
   int64_t len = 0;
   if (JS_GetLength(m_ctx, arr, &len) < 0)
      return false;
   SAFEARRAY *sa = SafeArrayCreateVector(VT_VARIANT, 0, static_cast<ULONG>(len));
   for (LONG i = 0; i < len; i++)
   {
      JSValue e = JS_GetPropertyUint32(m_ctx, arr, static_cast<uint32_t>(i));
      VARIANT ve;
      VariantInit(&ve);
      const bool ok = ToVariant(e, ve);
      JS_FreeValue(m_ctx, e);
      if (!ok)
      {
         SafeArrayDestroy(sa);
         return false;
      }
      SafeArrayPutElement(sa, &i, &ve);
      VariantClear(&ve);
   }
   V_VT(&out) = VT_ARRAY | VT_VARIANT;
   V_ARRAY(&out) = sa;
   return true;
}

bool JSScriptEngine::ToVariant(JSValueConst v, VARIANT &out)
{
   VariantInit(&out);
   if (JS_IsUndefined(v))
      return true;
   if (JS_IsNull(v))
   {
      V_VT(&out) = VT_NULL;
      return true;
   }
   if (JS_IsBool(v))
   {
      V_VT(&out) = VT_BOOL;
      V_BOOL(&out) = JS_ToBool(m_ctx, v) ? VARIANT_TRUE : VARIANT_FALSE;
      return true;
   }
   if (JS_IsNumber(v))
   {
      if (JS_VALUE_GET_TAG(v) == JS_TAG_INT)
      {
         int32_t i = 0;
         JS_ToInt32(m_ctx, &i, v);
         V_VT(&out) = VT_I4;
         V_I4(&out) = i;
      }
      else
      {
         V_VT(&out) = VT_R8;
         JS_ToFloat64(m_ctx, &V_R8(&out), v);
      }
      return true;
   }
   if (JS_IsString(v))
   {
      const char *str = JS_ToCString(m_ctx, v);
      if (str == nullptr)
         return false;
      V_VT(&out) = VT_BSTR;
      V_BSTR(&out) = MakeWideBSTR(string(str), CP_UTF8);
      JS_FreeCString(m_ctx, str);
      return true;
   }
   if (ComObject *co = GetComObject(v); co)
   {
      if (co->disp == nullptr)
      {
         JS_ThrowTypeError(m_ctx, "VPX object '%s' was destroyed", co->name.c_str());
         return false;
      }
      V_VT(&out) = VT_DISPATCH;
      V_DISPATCH(&out) = co->disp;
      co->disp->AddRef();
      return true;
   }
   if (JS_IsArray(v))
      return ArrayToSafeArray(v, out);
   if (JS_IsObject(v))
   {
      // Last resort: string conversion (Date, plain objects, ...)
      const char *str = JS_ToCString(m_ctx, v);
      if (str == nullptr)
         return false;
      V_VT(&out) = VT_BSTR;
      V_BSTR(&out) = MakeWideBSTR(string(str), CP_UTF8);
      JS_FreeCString(m_ctx, str);
      return true;
   }
   JS_ThrowTypeError(m_ctx, "Unsupported JavaScript value passed to VPX");
   return false;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// COM invocation

JSScriptEngine::Member *JSScriptEngine::ResolveMember(ComObject *co, const string &name)
{
   const string lowerName = lowerCase(name);
   if (const auto it = co->members.find(lowerName); it != co->members.end())
      return &it->second;
   if (co->disp == nullptr)
      return nullptr;
   // Pass a real BSTR: some IDispatch implementations (plugin objects) read the name through SysStringLen
   BSTR bname = MakeWideBSTR(name, CP_UTF8);
   LPOLESTR names[1] = { bname };
   DISPID dispid = DISPID_UNKNOWN;
   const HRESULT hr = co->disp->GetIDsOfNames(IID_NULL, names, 1, LOCALE_USER_DEFAULT, &dispid);
   SysFreeString(bname);
   if (FAILED(hr))
      return nullptr;
   Member &m = co->members[lowerName];
   m.dispid = dispid;
   // Plugin objects carry static type information, no need to probe them
   if (const DynamicDispatch *const dd = dynamic_cast<const DynamicDispatch *>(co->disp); dd != nullptr)
   {
      switch (dd->GetMemberKind(dispid))
      {
      case 1: m.kind = MemberKind::Property; break;
      case 2: m.kind = MemberKind::IndexedProperty; break;
      case 3: m.kind = MemberKind::Method; break;
      default: break;
      }
   }
   else if (const auto it = m_memberKinds.find(MemberKindKey(co, lowerName)); it != m_memberKinds.end())
      m.kind = it->second;
   return &m;
}

// Objects of the same class share their IDispatch vtable, so the probed member kind can be shared too
string JSScriptEngine::MemberKindKey(const ComObject *co, const string &lowerName)
{
   return std::to_string(reinterpret_cast<uintptr_t>(*reinterpret_cast<void *const *>(co->disp))) + '|' + lowerName;
}

bool JSScriptEngine::IsNotAPropertyGet(HRESULT hr)
{
   return hr == DISP_E_UNKNOWNNAME || hr == DISP_E_MEMBERNOTFOUND || hr == DISP_E_BADPARAMCOUNT || hr == DISP_E_PARAMNOTOPTIONAL;
}

HRESULT JSScriptEngine::InvokeCom(IDispatch *disp, DISPID dispid, WORD flags, VARIANT *args, unsigned int nArgs, VARIANT *result, string &errorDesc)
{
   DISPPARAMS dp = { args, nullptr, nArgs, 0 };
   DISPID putId = DISPID_PROPERTYPUT;
   if (flags & (DISPATCH_PROPERTYPUT | DISPATCH_PROPERTYPUTREF))
   {
      dp.rgdispidNamedArgs = &putId;
      dp.cNamedArgs = 1;
   }
   EXCEPINFO ei = {};
   UINT argErr = 0;
   const HRESULT hr = disp->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, flags, &dp, result, &ei, &argErr);
   if (FAILED(hr))
   {
      if (ei.bstrDescription)
         errorDesc = MakeString(ei.bstrDescription);
      SysFreeString(ei.bstrDescription);
      SysFreeString(ei.bstrSource);
      SysFreeString(ei.bstrHelpFile);
   }
   return hr;
}

JSValue JSScriptEngine::ThrowComError(const string &member, HRESULT hr, const string &desc)
{
   std::stringstream ss;
   ss << member << ": ";
   if (!desc.empty())
      ss << desc;
   else
   {
#ifdef __STANDALONE__
      ss << libwinevbs_hresult_name(hr);
#else
      ss << "HRESULT 0x" << std::hex << static_cast<unsigned int>(hr);
#endif
   }
   JSValue err = JS_NewError(m_ctx);
   JS_DefinePropertyValueStr(m_ctx, err, "message", JS_NewString(m_ctx, ss.str().c_str()), JS_PROP_WRITABLE | JS_PROP_CONFIGURABLE);
   return JS_Throw(m_ctx, err);
}

// Invokes member 'name' of 'co' with the given JS arguments.
JSValue JSScriptEngine::InvokeFromJS(ComObject *co, const string &name, WORD flags, int argc, JSValueConst *argv, bool allowPropertyGetFallback)
{
   if (co->disp == nullptr)
      return JS_ThrowTypeError(m_ctx, "VPX object '%s' was destroyed", co->name.c_str());
   Member *m = ResolveMember(co, name);
   if (m == nullptr)
      return JS_ThrowTypeError(m_ctx, "'%s' is not a member of VPX object '%s'", name.c_str(), co->name.c_str());

   // Arguments are passed in reverse order
   vector<VARIANT> args(argc);
   for (int i = 0; i < argc; i++)
   {
      if (!ToVariant(argv[i], args[argc - 1 - i]))
      {
         for (int j = argc - 1 - i + 1; j < argc; j++)
            VariantClear(&args[j]);
         return JS_EXCEPTION;
      }
   }

   if (m->kind == MemberKind::IndexedProperty && (flags & DISPATCH_METHOD))
      flags = DISPATCH_PROPERTYGET;
   VARIANT result;
   VariantInit(&result);
   string desc;
   HRESULT hr = InvokeCom(co->disp, m->dispid, flags, args.data(), argc, &result, desc);
   if (allowPropertyGetFallback && IsNotAPropertyGet(hr) && (flags & DISPATCH_METHOD))
   {
      // Not a method: a property with arguments (Collection.Item(i), VPXActionKey(n)). Remember it for the class.
      hr = InvokeCom(co->disp, m->dispid, DISPATCH_PROPERTYGET, args.data(), argc, &result, desc);
      if (SUCCEEDED(hr))
      {
         m->kind = MemberKind::IndexedProperty;
         if (co->disp)
            m_memberKinds[MemberKindKey(co, lowerCase(name))] = MemberKind::IndexedProperty;
      }
   }
   for (VARIANT &a : args)
      VariantClear(&a);
   if (FAILED(hr))
   {
      VariantClear(&result);
      return ThrowComError(co->name.empty() ? name : co->name + '.' + name, hr, desc);
   }
   JSValue r = ToJS(result);
   VariantClear(&result);
   return r;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// QuickJS class callbacks

static bool IsArrayIndexName(const char *name)
{
   if (*name == '\0')
      return false;
   for (const char *p = name; *p; p++)
      if (*p < '0' || *p > '9')
         return false;
   return true;
}

JSValue JSScriptEngine::GetProperty(JSContext *ctx, JSValueConst obj, JSAtom atom, JSValueConst receiver)
{
   JSScriptEngine *engine = static_cast<JSScriptEngine *>(JS_GetContextOpaque(ctx));
   ComObject *co = engine->GetComObject(obj);
   if (co == nullptr)
      return JS_UNDEFINED;

   JSValue atomVal = JS_AtomToValue(ctx, atom);
   const bool isSymbol = JS_IsSymbol(atomVal);
   JS_FreeValue(ctx, atomVal);
   if (isSymbol)
      return JS_GetProperty(ctx, engine->m_helperProto, atom);

   const char *name = JS_AtomToCString(ctx, atom);
   if (name == nullptr)
      return JS_EXCEPTION;
   const string sname(name);
   JS_FreeCString(ctx, name);

   // Helper members ($call, $get, $set, ...) never go to the object
   if (sname[0] == '$')
      return JS_GetProperty(ctx, engine->m_helperProto, atom);

   if (co->disp == nullptr)
      return JS_ThrowTypeError(ctx, "VPX object '%s' was destroyed", co->name.c_str());

   // Numeric index: Item(index) (collections)
   if (IsArrayIndexName(sname.c_str()))
   {
      JSValue idx = JS_NewInt32(ctx, atoi(sname.c_str()));
      JSValue r = engine->InvokeFromJS(co, "Item", DISPATCH_PROPERTYGET, 1, &idx, false);
      JS_FreeValue(ctx, idx);
      return r;
   }

   Member *m = engine->ResolveMember(co, sname);
   if (m == nullptr)
   {
      // Unresolved global name: named items are also reachable case insensitively, like in VBScript
      if (JS_VALUE_GET_PTR(receiver) == JS_VALUE_GET_PTR(engine->m_globalObj))
         if (const auto it = engine->m_itemsByLowerName.find(lowerCase(sname)); it != engine->m_itemsByLowerName.end())
            return JS_DupValue(ctx, engine->m_items[it->second].wrapper);
      return JS_GetProperty(ctx, engine->m_helperProto, atom); // helper methods (on, off, $call, ...) or undefined
   }

   if (m->kind != MemberKind::Method && m->kind != MemberKind::IndexedProperty)
   {
      VARIANT result;
      VariantInit(&result);
      string desc;
      const HRESULT hr = engine->InvokeCom(co->disp, m->dispid, DISPATCH_PROPERTYGET, nullptr, 0, &result, desc);
      if (SUCCEEDED(hr))
      {
         if (m->kind == MemberKind::Unknown)
            engine->m_memberKinds[MemberKindKey(co, lowerCase(sname))] = MemberKind::Property;
         m->kind = MemberKind::Property;
         JSValue r = engine->ToJS(result);
         VariantClear(&result);
         return r;
      }
      VariantClear(&result);
      if (m->kind == MemberKind::Property || !IsNotAPropertyGet(hr))
         return engine->ThrowComError(co->name.empty() ? sname : co->name + '.' + sname, hr, desc);
      m->kind = MemberKind::Method;
      engine->m_memberKinds[MemberKindKey(co, lowerCase(sname))] = MemberKind::Method;
   }

   if (JS_IsUndefined(m->fn))
   {
      // Bind the object itself: bare calls of global members (PlaySound(...)) resolve through the global prototype
      JSValue data[2] = { JS_NewString(ctx, sname.c_str()), JS_DupValue(ctx, obj) };
      m->fn = JS_NewCFunctionData(ctx, MethodCall, 0, 0, 2, data);
      JS_FreeValue(ctx, data[0]);
      JS_FreeValue(ctx, data[1]);
   }
   return JS_DupValue(ctx, m->fn);
}

JSValue JSScriptEngine::MethodCall(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv, int magic, JSValueConst *func_data)
{
   JSScriptEngine *engine = static_cast<JSScriptEngine *>(JS_GetContextOpaque(ctx));
   const char *name = JS_ToCString(ctx, func_data[0]);
   const string sname(name ? name : "");
   JS_FreeCString(ctx, name);
   ComObject *co = engine->GetComObject(func_data[1]);
   if (co == nullptr)
      return JS_ThrowTypeError(ctx, "'%s' must be called on a VPX object", sname.c_str());
   return engine->InvokeFromJS(co, sname, DISPATCH_METHOD, argc, argv, true);
}

int JSScriptEngine::SetProperty(JSContext *ctx, JSValueConst obj, JSAtom atom, JSValueConst value, JSValueConst receiver, int flags)
{
   JSScriptEngine *engine = static_cast<JSScriptEngine *>(JS_GetContextOpaque(ctx));
   ComObject *co = engine->GetComObject(obj);
   Member *m = nullptr;
   if (co != nullptr && co->disp != nullptr)
   {
      JSValue atomVal = JS_AtomToValue(ctx, atom);
      const bool isSymbol = JS_IsSymbol(atomVal);
      JS_FreeValue(ctx, atomVal);
      if (!isSymbol)
      {
         const char *name = JS_AtomToCString(ctx, atom);
         if (name)
            m = engine->ResolveMember(co, name);
         JS_FreeCString(ctx, name);
      }
   }
   if (m == nullptr)
   {
      // Not a COM member: define a plain property on the receiver (script globals end up here through the global prototype)
      return JS_DefinePropertyValue(ctx, receiver, atom, JS_DupValue(ctx, value), JS_PROP_C_W_E) < 0 ? -1 : 1;
   }
   VARIANT arg;
   if (!engine->ToVariant(value, arg))
      return -1;
   VARIANT result;
   VariantInit(&result);
   string desc;
   HRESULT hr = engine->InvokeCom(co->disp, m->dispid, DISPATCH_PROPERTYPUT, &arg, 1, &result, desc);
   if (FAILED(hr) && V_VT(&arg) == VT_DISPATCH)
      hr = engine->InvokeCom(co->disp, m->dispid, DISPATCH_PROPERTYPUTREF, &arg, 1, &result, desc);
   VariantClear(&result);
   VariantClear(&arg);
   if (FAILED(hr))
   {
      const char *name = JS_AtomToCString(ctx, atom);
      engine->ThrowComError(co->name + '.' + (name ? name : "?"), hr, desc);
      JS_FreeCString(ctx, name);
      return -1;
   }
   return 1;
}

int JSScriptEngine::HasProperty(JSContext *ctx, JSValueConst obj, JSAtom atom)
{
   JSScriptEngine *engine = static_cast<JSScriptEngine *>(JS_GetContextOpaque(ctx));
   ComObject *co = engine->GetComObject(obj);
   if (co != nullptr && co->disp != nullptr)
   {
      JSValue atomVal = JS_AtomToValue(ctx, atom);
      const bool isSymbol = JS_IsSymbol(atomVal);
      JS_FreeValue(ctx, atomVal);
      if (!isSymbol)
      {
         const char *name = JS_AtomToCString(ctx, atom);
         const bool found = name && engine->ResolveMember(co, name) != nullptr;
         JS_FreeCString(ctx, name);
         if (found)
            return 1;
      }
   }
   return JS_HasProperty(ctx, engine->m_helperProto, atom);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Host functions

static string ArgToString(JSContext *ctx, JSValueConst v)
{
   const char *str = JS_ToCString(ctx, v);
   string s(str ? str : "");
   JS_FreeCString(ctx, str);
   return s;
}

// CreateObject("ProgID"): plugin provided objects first, then (Windows only) real COM objects
JSValue JSScriptEngine::HostCreateObject(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
   JSScriptEngine *engine = static_cast<JSScriptEngine *>(JS_GetContextOpaque(ctx));
   if (argc < 1)
      return JS_ThrowTypeError(ctx, "CreateObject: missing class name");
   const string classId = ArgToString(ctx, argv[0]);
   if (g_pplayer)
   {
      if (IDispatch *disp = g_pplayer->m_pluginAPI.CreateCOMPluginObject(classId); disp)
      {
         JSValue r = engine->Wrap(disp);
         disp->Release();
         return r;
      }
   }
#ifndef __STANDALONE__
   CLSID clsid;
   IDispatch *disp = nullptr;
   const wstring wClassId = MakeWString(classId);
   if (SUCCEEDED(CLSIDFromProgID(wClassId.c_str(), &clsid)) && SUCCEEDED(CoCreateInstance(clsid, nullptr, CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER, IID_IDispatch, (void **)&disp)) && disp)
   {
      JSValue r = engine->Wrap(disp);
      disp->Release();
      return r;
   }
#endif
   return JS_ThrowReferenceError(ctx, "CreateObject: no plugin or COM server provides '%s'", classId.c_str());
}

JSValue JSScriptEngine::HostMsgBox(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
   const string text = argc > 0 ? ArgToString(ctx, argv[0]) : string();
   PLOGI << "MsgBox: " << text;
   if (g_pplayer && g_pplayer->m_liveUI)
      g_pplayer->m_liveUI->PushNotification(text, 5000);
   return JS_NewInt32(ctx, 1);
}

JSValue JSScriptEngine::HostDebugPrint(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
   if (g_pplayer && g_pplayer->m_ptable->IsLocked())
      return JS_UNDEFINED;
   if (!g_app->m_settings.GetGlobal_EnableLog() || !g_app->m_settings.GetGlobal_LogScriptOutput())
      return JS_UNDEFINED;
   PLOGI << "Script.Print '" << (argc > 0 ? ArgToString(ctx, argv[0]) : string()) << '\'';
   return JS_UNDEFINED;
}

JSValue JSScriptEngine::HostConsole(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv, int magic)
{
   string line;
   for (int i = 0; i < argc; i++)
   {
      if (i > 0)
         line += ' ';
      line += ArgToString(ctx, argv[i]);
   }
   switch (magic)
   {
   case 1: PLOGD << "console: " << line; break;
   case 2: PLOGW << "console: " << line; break;
   case 3: PLOGE << "console: " << line; break;
   default: PLOGI << "console: " << line; break;
   }
   return JS_UNDEFINED;
}

// __vpx_on(obj, event, fn) / __vpx_off(obj, event, fn)
JSValue JSScriptEngine::HostOn(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv, int magic)
{
   JSScriptEngine *engine = static_cast<JSScriptEngine *>(JS_GetContextOpaque(ctx));
   if (argc < 2)
      return JS_ThrowTypeError(ctx, "on/off: expected an event name");
   ComObject *co = engine->GetComObject(argv[0]);
   if (co == nullptr || co->name.empty())
      return JS_ThrowTypeError(ctx, "on/off: only named table objects fire events");
   const string key = lowerCase(co->name + '_' + ArgToString(ctx, argv[1]));
   vector<JSValue> &listeners = engine->m_listeners[key];
   if (magic == 1)
   {
      if (argc < 3 || !JS_IsFunction(ctx, argv[2]))
         return JS_ThrowTypeError(ctx, "on: expected a function");
      listeners.push_back(JS_DupValue(ctx, argv[2]));
   }
   else
   {
      for (auto it = listeners.begin(); it != listeners.end();)
      {
         if (argc < 3 || JS_IsUndefined(argv[2]) || JS_VALUE_GET_PTR(*it) == JS_VALUE_GET_PTR(argv[2]))
         {
            JS_FreeValue(ctx, *it);
            it = listeners.erase(it);
         }
         else
            ++it;
      }
      if (listeners.empty())
         engine->m_listeners.erase(key);
   }
   engine->MarkHandlersDirty();
   return JS_UNDEFINED;
}

// __vpx_call(obj, name, args) / __vpx_get(obj, name, args) / __vpx_set(obj, name, [value])
JSValue JSScriptEngine::HostCall(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv, int magic)
{
   JSScriptEngine *engine = static_cast<JSScriptEngine *>(JS_GetContextOpaque(ctx));
   if (argc < 2)
      return JS_ThrowTypeError(ctx, "expected an object and a member name");
   ComObject *co = engine->GetComObject(argv[0]);
   if (co == nullptr)
      return JS_ThrowTypeError(ctx, "expected a VPX object");
   const string name = ArgToString(ctx, argv[1]);
   vector<JSValue> args;
   if (argc > 2 && JS_IsArray(argv[2]))
   {
      int64_t len = 0;
      JS_GetLength(ctx, argv[2], &len);
      for (int64_t i = 0; i < len; i++)
         args.push_back(JS_GetPropertyUint32(ctx, argv[2], static_cast<uint32_t>(i)));
   }
   JSValue r = engine->InvokeFromJS(co, name, static_cast<WORD>(magic), static_cast<int>(args.size()), args.data(), magic == DISPATCH_METHOD);
   for (JSValue v : args)
      JS_FreeValue(ctx, v);
   return r;
}

// __vpx_enum(obj): collects the items of a COM enumerable (_NewEnum) into an array
JSValue JSScriptEngine::HostEnum(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
   JSScriptEngine *engine = static_cast<JSScriptEngine *>(JS_GetContextOpaque(ctx));
   ComObject *co = argc > 0 ? engine->GetComObject(argv[0]) : nullptr;
   if (co == nullptr || co->disp == nullptr)
      return JS_ThrowTypeError(ctx, "object is not iterable");
   VARIANT result;
   VariantInit(&result);
   string desc;
   const HRESULT hr = engine->InvokeCom(co->disp, DISPID_NEWENUM, DISPATCH_PROPERTYGET | DISPATCH_METHOD, nullptr, 0, &result, desc);
   if (FAILED(hr))
   {
      VariantClear(&result);
      return engine->ThrowComError(co->name + " is not iterable", hr, desc);
   }
   IEnumVARIANT *enumVariant = nullptr;
   IUnknown *unk = (V_VT(&result) == VT_UNKNOWN) ? V_UNKNOWN(&result) : (V_VT(&result) == VT_DISPATCH) ? V_DISPATCH(&result) : nullptr;
   if (unk)
      unk->QueryInterface(IID_IEnumVARIANT, (void **)&enumVariant);
   VariantClear(&result);
   JSValue arr = JS_NewArray(ctx);
   if (enumVariant == nullptr)
      return arr;
   uint32_t n = 0;
   while (true)
   {
      VARIANT item;
      VariantInit(&item);
      ULONG fetched = 0;
      if (enumVariant->Next(1, &item, &fetched) != S_OK || fetched == 0)
         break;
      JS_SetPropertyUint32(ctx, arr, n++, engine->ToJS(item));
      VariantClear(&item);
   }
   enumVariant->Release();
   return arr;
}

JSValue JSScriptEngine::HostName(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
   JSScriptEngine *engine = static_cast<JSScriptEngine *>(JS_GetContextOpaque(ctx));
   ComObject *co = argc > 0 ? engine->GetComObject(argv[0]) : nullptr;
   return JS_NewString(ctx, co ? co->name.c_str() : "");
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Errors, interruption and jobs

int JSScriptEngine::InterruptHandler(JSRuntime *rt, void *opaque)
{
   return static_cast<JSScriptEngine *>(opaque)->m_interrupt ? 1 : 0;
}

void JSScriptEngine::PromiseRejectionTracker(JSContext *ctx, JSValueConst promise, JSValueConst reason, bool is_handled, void *opaque)
{
   if (is_handled)
      return;
   JSScriptEngine *engine = static_cast<JSScriptEngine *>(opaque);
   const string desc = "Unhandled promise rejection: " + ArgToString(ctx, reason);
   PLOGE << desc;
   engine->m_hasError = true;
   if (engine->m_errorHandler)
      engine->m_errorHandler(ErrorType::Runtime, 0, 0, desc, vector<string>());
}

void JSScriptEngine::PumpJobs()
{
   JSContext *ctx;
   int r;
   while ((r = JS_ExecutePendingJob(m_rt, &ctx)) > 0)
   {
   }
   if (r < 0)
      ReportException(ErrorType::Runtime);
}

void JSScriptEngine::ReportException(ErrorType type)
{
   ReportError(type, JS_GetException(m_ctx));
}

// Reports an error value (takes ownership of it)
void JSScriptEngine::ReportError(ErrorType type, JSValue exc)
{
   string desc;
   int line = 0, column = 0;
   vector<string> stack;
   if (JS_IsObject(exc))
   {
      JSValue name = JS_GetPropertyStr(m_ctx, exc, "name");
      JSValue message = JS_GetPropertyStr(m_ctx, exc, "message");
      const string sname = JS_IsUndefined(name) ? string() : ArgToString(m_ctx, name);
      desc = (sname.empty() ? string() : sname + ": ") + (JS_IsUndefined(message) ? string() : ArgToString(m_ctx, message));
      JS_FreeValue(m_ctx, name);
      JS_FreeValue(m_ctx, message);
      if (sname == "SyntaxError")
         type = ErrorType::Compile;

      JSValue lineNumber = JS_GetPropertyStr(m_ctx, exc, "lineNumber");
      if (JS_IsNumber(lineNumber))
         JS_ToInt32(m_ctx, &line, lineNumber);
      JS_FreeValue(m_ctx, lineNumber);
      JSValue columnNumber = JS_GetPropertyStr(m_ctx, exc, "columnNumber");
      if (JS_IsNumber(columnNumber))
         JS_ToInt32(m_ctx, &column, columnNumber);
      JS_FreeValue(m_ctx, columnNumber);

      JSValue stackVal = JS_GetPropertyStr(m_ctx, exc, "stack");
      if (JS_IsString(stackVal))
      {
         std::stringstream ss(ArgToString(m_ctx, stackVal));
         string frame;
         static const std::regex position(R"(:(\d+):(\d+)\)?\s*$)");
         while (std::getline(ss, frame))
         {
            if (frame.empty())
               continue;
            if (line == 0)
            {
               std::smatch m;
               if (std::regex_search(frame, m, position))
               {
                  line = std::stoi(m[1].str());
                  column = std::stoi(m[2].str());
               }
            }
            stack.push_back(frame);
         }
      }
      JS_FreeValue(m_ctx, stackVal);
   }
   else
      desc = ArgToString(m_ctx, exc);
   if (JS_IsUncatchableError(exc))
      desc = "Script interrupted: " + desc;
   JS_FreeValue(m_ctx, exc);

   m_hasError = true;
   if (m_errorHandler)
      m_errorHandler(type, line, column, desc, stack);
   else
      PLOGE << "Script error: " << desc;
}
