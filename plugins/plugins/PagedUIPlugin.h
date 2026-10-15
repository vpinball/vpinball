// license:GPLv3+

#pragma once

#include "MsgPlugin.h"

///////////////////////////////////////////////////////////////////////////////
// Paged declarative plugin user interface ("UI sheets")
//
// WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
// This interface is part of a work in progress and will evolve likely a lot
// before being considered stable. Do not use it, or if you do, use it knowing
// that you're plugin will be broken by the upcoming updates.
// WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
//
// A plugin may contribute one or more 'sheets': trees of pages filled with
// declarative items (labels, properties, actions, sub pages, ...). The host
// renders the sheets inside its own cross device user interface (the in game
// UI), so contributed interfaces work with keyboard/mouse, touch, cabinet
// buttons and VR controllers without the plugin doing any rendering or input
// handling of its own.
//
// This mechanism extends the flat plugin settings mechanism
// (MsgPluginAPI::RegisterSetting) into a full declarative UI surface: property
// items only differ by who owns their persistence, declared per item:
// - PAGEDUI_STORAGE_HOST: the value is persisted by the host in its settings
//   store (application wide and/or as per table overrides, decided by the
//   host). RegisterSetting is equivalent to appending a property item to the
//   plugin's implicit 'settings' sheet (a plugin may redefine its default page
//   by registering a sheet with the reserved id "settings").
// - PAGEDUI_STORAGE_PLUGIN: the value is fully owned by the plugin (documents
//   such as backglass layouts, media packs, display configurations, ...). The
//   host manages the editing session (BeginEdit when the sheet is opened,
//   CommitEdit when the user asks to save, EndEdit when it is closed) but
//   never knows how or where the plugin persists its data.
// A sheet may freely mix both kinds of items.
//
// Sheets are contributed through the PagedUIPluginAPI dispatch table, obtained
// by broadcasting the PAGEDUIPI_MSG_GET_API message with a pointer to a
// PagedUIPluginAPI* which the host fills in:
//   PagedUIPluginAPI* uiApi = nullptr;
//   msgApi->BroadcastMsg(endpointId, msgApi->GetMsgID(PAGEDUIPI_NAMESPACE, PAGEDUIPI_MSG_GET_API), &uiApi);
//
// Registration itself is the availability signal: a plugin registers a sheet
// when it should be usable (e.g. a table document sheet on OnGameStart) and
// unregisters it when it should disappear. The host lists whatever is
// registered.
//
// All sheet callbacks are invoked on the API thread. Item definitions are
// re-enumerated each time a page is (re)built, so a plugin may freely change
// the item list between calls (dynamic/conditional content) and request a
// rebuild through RequestSheetRefresh. Strings in item/page definitions only
// need to stay valid for the duration of the enumeration call.
//
// Ids (sheet, page, item) are restricted to alphanumeric characters, '.', '_'
// and '-'. For host stored items, item ids must be unique within the sheet
// as they are used as persistence keys.

#define PAGEDUIPI_NAMESPACE "PagedUI" // Namespace used for all paged UI message definitions
#define PAGEDUIPI_MSG_GET_API "GetAPI:1" // Get the paged UI plugin API

typedef enum
{
   PAGEDUI_STORAGE_HOST, // Host persists values in its settings store
   PAGEDUI_STORAGE_PLUGIN // Plugin fully owns data persistence
} PagedUIStorageMode;

typedef struct PagedUIPageInfo
{
   const char* id; // Unique id within the sheet. Page at index 0 is the root page.
   const char* title; // Page title (if NULL, sheet name is used for the root page, id for others)
   const char* info; // Optional text shown in the page footer area
} PagedUIPageInfo;

typedef enum
{
   // Structure
   PAGEDUI_ITEM_HEADER, // Section title with a separator line
   PAGEDUI_ITEM_INFO, // Non selectable wrapped text paragraph
   PAGEDUI_ITEM_MARKDOWN, // Non selectable text paragraph with markdown formatting
   // Editable properties
   PAGEDUI_ITEM_FLOAT, // float value, rendered as a slider
   PAGEDUI_ITEM_INT, // int value, rendered as a slider
   PAGEDUI_ITEM_ENUM, // int value in [minVal, minVal+nValues), rendered as a dropdown
   PAGEDUI_ITEM_BOOL, // boolean, rendered as a toggle
   PAGEDUI_ITEM_STRING, // text value (requires a keyboard; limited support on cabinet/VR)
   PAGEDUI_ITEM_FILE, // file path with a browse dialog (same input limits as STRING)
   PAGEDUI_ITEM_FOLDER, // folder path with a browse dialog (same input limits as STRING)
   // Interaction
   PAGEDUI_ITEM_ACTION, // Button triggering the Run callback
   PAGEDUI_ITEM_SUBPAGE // Navigation to another page of the same sheet
} PagedUIItemType;

#define PAGEDUI_ITEMF_HIDDEN 0x01 // Item is declared but not displayed
#define PAGEDUI_ITEMF_EXCLUDE_DEFAULTS 0x02 // Item is not reset by 'reset to defaults'
#define PAGEDUI_ITEMF_GLOBAL_ONLY 0x04 // Host storage: always saved globally, never as table override

typedef struct PagedUIItemDef
{
   PagedUIItemType type;
   const char* id; // Unique id within the sheet, stable across rebuilds (persistence key for host storage)
   const char* label; // Displayed label
   const char* tooltip; // Optional description shown in the footer when the item is highlighted
   unsigned int flags; // PAGEDUI_ITEMF_xxx
   int storage; // PagedUIStorageMode, only meaningful for property items
   void* context; // Opaque pointer passed back to this item's callbacks
   union
   {
      struct
      {
         float minVal, maxVal;
         float step; // If non zero, limit values to increments of step
         float defVal; // Default value (used by 'reset to defaults')
         float displayScale; // Value is multiplied by this for display (0 defaults to 1)
         const char* format; // printf style display format (may be NULL)
         float(MSGPIAPI* Get)(void* context);
         void(MSGPIAPI* Set)(void* context, float value);
      } floatProp;
      struct
      {
         int minVal, maxVal;
         int defVal;
         const char* format; // printf style display format (may be NULL)
         int(MSGPIAPI* Get)(void* context);
         void(MSGPIAPI* Set)(void* context, int value);
      } intProp;
      struct
      {
         int minVal; // Value of the first entry
         int nValues; // Number of entries
         const char** values; // nValues human readable labels (must stay valid during the enumeration call)
         int defVal;
         int(MSGPIAPI* Get)(void* context);
         void(MSGPIAPI* Set)(void* context, int value);
      } enumProp;
      struct
      {
         int defVal;
         int(MSGPIAPI* Get)(void* context);
         void(MSGPIAPI* Set)(void* context, int value);
      } boolProp;
      struct
      {
         const char* defVal;
         const char*(MSGPIAPI* Get)(void* context);
         void(MSGPIAPI* Set)(void* context, const char* value);
      } stringProp;
      struct
      {
         const char* defVal;
         const char* fileFilter; // Optional ';' separated list of allowed file extensions (e.g. "png;jpg"), may be NULL
         const char*(MSGPIAPI* Get)(void* context);
         void(MSGPIAPI* Set)(void* context, const char* value);
      } fileProp;
      struct
      {
         const char* defVal;
         const char*(MSGPIAPI* Get)(void* context);
         void(MSGPIAPI* Set)(void* context, const char* value);
      } folderProp;
      struct
      {
         void(MSGPIAPI* Run)(void* context);
      } action;
      struct
      {
         const char* pageId; // Id of a page of the same sheet
      } subpage;
   };
} PagedUIItemDef;

typedef struct PagedUISheetDef
{
   int version; // Must be 1
   const char* id; // Unique id of the sheet within the plugin ("settings" is reserved for the default settings page)
   const char* name; // Human readable name used in menus
   const char* description; // Human readable description
   void* context; // Opaque pointer passed back to all callbacks below

   // Page tree, re-enumerated each time a page is (re)built
   unsigned int(MSGPIAPI* GetPageCount)(void* context);
   void(MSGPIAPI* GetPageInfo)(void* context, unsigned int pageIndex, PagedUIPageInfo* info);
   unsigned int(MSGPIAPI* GetItemCount)(void* context, const char* pageId);
   void(MSGPIAPI* GetItems)(void* context, const char* pageId, PagedUIItemDef* items);

   // Edit session, only invoked when the sheet contains plugin stored items (ignored otherwise)
   void(MSGPIAPI* BeginEdit)(void* context); // The sheet is opened for editing; may be NULL
   int(MSGPIAPI* CommitEdit)(void* context); // Persist the document; return 0 on success
   void(MSGPIAPI* EndEdit)(void* context, int committed); // The last page of the sheet was closed; may be NULL
   int(MSGPIAPI* IsDirty)(void* context); // Optional non property dirty state, OR'ed with the
   // host computed item modifications; may be NULL

   // Non zero marks this sheet as the plugin's root: its root page (page index 0) is displayed as
   // the plugin's landing page and hosts the plugin enable/disable entry. At most one sheet should
   // be marked; when none is, the implicit or plugin-declared "settings" sheet is used as the root.
   int isRoot;
} PagedUISheetDef;

typedef struct PagedUIPluginAPI
{
   int version; // Must be 1. Included to allow extending the API with new functions at a later point in time

   // Registers a sheet. The host copies the definition (including its strings), so the provided struct may be
   // temporary. Registering a sheet with an already used id replaces the previous registration. For host
   // stored items, the host enumerates the items at registration to push persisted values to the plugin, so
   // the sheet must be enumerable when this is called.
   void(MSGPIAPI* RegisterSheet)(const uint32_t endpointId, const PagedUISheetDef* sheet);
   // Unregisters a previously registered sheet (no-op if unknown)
   void(MSGPIAPI* UnregisterSheet)(const uint32_t endpointId, const char* sheetId);
   // Requests the host to rebuild any open page of the sheet (and refresh menus). sheetId may be NULL to
   // request a global refresh. Use RunOnMainThread to call this from a worker thread.
   void(MSGPIAPI* RequestSheetRefresh)(const uint32_t endpointId, const char* sheetId);
} PagedUIPluginAPI;
