// license:GPLv3+

// implementation of the VPinball class.

#include "core/stdafx.h"
#include "ui/VPXFileFeedback.h"
#include "ui/resource.h"
#ifndef __STANDALONE__
#include "ui/dialogs/KeysConfigDialog.h"
#endif
#ifdef ENABLE_SDL_VIDEO
#include "imgui/imgui_impl_sdl3.h"
#endif

#ifdef __STANDALONE__
#include "standalone/Standalone.h"
#include <iostream>
#endif

#ifdef __LIBVPINBALL__
#include "standalone/VPinballLib.h"
#endif

#include <filesystem>
#ifndef __STANDALONE__
#include "FreeImage.h"
#else
#include "standalone/FreeImage.h"
#endif
#include "vpversion.h"

#if defined(IMSPANISH)
#define TOOLBAR_WIDTH 152
#elif defined(IMGERMAN)
#define TOOLBAR_WIDTH 152
#else
#define TOOLBAR_WIDTH 102 //98 //102
#endif

#define SCROLL_WIDTH GetSystemMetrics(SM_CXVSCROLL)

#define DOCKER_REGISTRY_KEY     "Visual Pinball\\VP10\\Editor"

#define RECENT_FIRST_MENU_IDM   5000           // ID of the first recent file list filename
#define OPEN_MDI_TABLE_IDM      IDW_FIRSTCHILD // ID of the first open table
#define LAST_MDI_TABLE_IDM      IDW_CHILD9

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
// menu locations
enum {
   FILEMENU = 0,
   EDITMENU,
   VIEWMENU,
   INSERTMENU,
   TABLEMENU,
   LAYERMENU,
   PREFMENU,
   WINDOWMENU,
   HELPMENU,
   NUM_MENUS
};

/*
TBButton:
typedef struct {
int       iBitmap;
int       idCommand;
BYTE      fsState;
BYTE      fsStyle;
DWORD_PTR dwData;
INT_PTR   iString;
} TBBUTTON, *PTBBUTTON, *LPTBBUTTON;
*/

INT_PTR CALLBACK FontManagerProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK SecurityOptionsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

typedef struct _tagSORTDATA
{
    HWND hwndList;
    int subItemIndex;
    int sortUpDown;
}SORTDATA;

SORTDATA SortData;

VPinball::VPinball()
{
   // DLL_API void DLL_CALLCONV FreeImage_Initialise(BOOL load_local_plugins_only FI_DEFAULT(FALSE)); // would only be needed if linking statically
   m_closing = false;
   m_unloadingTable = false;
   m_cref = 0;				//inits Reference Count for IUnknown Interface. Every com Object must 
   //implement this and StdMethods QueryInterface, AddRef and Release

   m_open_minimized = false;
   m_disable_pause_menu = false;
   m_povEdit = false;
   m_primaryDisplay = false;
   m_disEnableTrueFullscreen = -1;
   m_table_played_via_command_line = false;
   m_table_played_via_SelectTableOnStart = false;
   m_logicalNumberOfProcessors = -1;

   m_mouseCursorPosition.x = 0.0f;
   m_mouseCursorPosition.y = 0.0f;
   m_pcv = nullptr;			// no currently active code window

   m_NextTableID = 1;

   m_ptableActive = nullptr;

   m_workerthread = nullptr;//Workerthread - only for hanging scripts and autosave - will be created later

   m_ToolCur = IDC_SELECT;

   m_hbmInPlayMode = nullptr;

   GetMyPath();				//Store path of vpinball.exe in m_myPath and m_wMyPath
   GetMyPrefPath();			//Store preference path of vpinball.exe in m_myPrefPath

#ifndef __STANDALONE__
#ifdef _WIN64
   m_scintillaDll = LoadLibrary("SciLexerVP64.DLL");
#else
   m_scintillaDll = LoadLibrary("SciLexerVP.DLL");
#endif
   if (m_scintillaDll == nullptr)
   {
      assert(!"Could not load SciLexerVP");
   #ifdef _WIN64
       m_scintillaDll = LoadLibrary("SciLexer64.DLL");
   #else
       m_scintillaDll = LoadLibrary("SciLexer.DLL");
   #endif
       if (m_scintillaDll == nullptr)
       #ifdef _WIN64
           ShowError("Unable to load SciLexerVP64.DLL or SciLexer64.DLL");
       #else
           ShowError("Unable to load SciLexerVP.DLL or SciLexer.DLL");
       #endif
   }

   // register the PinSim::FrontEndControls name window message
   m_pinSimFrontEndControlsMsg = RegisterWindowMessageA("PinSim::FrontEndControls");
#endif
}

//deletes clipboard
//Releases Resources for Script editor
VPinball::~VPinball()
{
   // DLL_API void DLL_CALLCONV FreeImage_DeInitialise(); // would only be needed if linking statically
   SetClipboard(nullptr);
#ifndef __STANDALONE__
   FreeLibrary(m_scintillaDll);
#endif
}

//Store path of exe (without the exe's filename) in Class Variable
//Stores path as string in m_myPath (8 bit Ansi)
//Stores path as wstring in m_wMyPath (16 bit Unicode)
void VPinball::GetMyPath()
{
#ifndef __STANDALONE__
   string path = GetExecutablePath();
   const size_t pos = path.find_last_of(PATH_SEPARATOR_CHAR);
   m_myPath = pos != string::npos ? path.substr(0,pos + 1) : path;
#else
#ifdef __ANDROID__
   m_myPath = string(SDL_GetAndroidInternalStoragePath()) + PATH_SEPARATOR_CHAR;
#elif defined(__APPLE__) && defined(TARGET_OS_IOS) && TARGET_OS_IOS && !defined(__LIBVPINBALL__)
   char *szPath = SDL_GetPrefPath("../..", "Documents");
   m_myPath = szPath;
   SDL_free(szPath);
#elif defined(__APPLE__) && defined(TARGET_OS_TV) && TARGET_OS_TV
   char *szPath = SDL_GetPrefPath(NULL, "Documents");
   m_myPath = szPath;
   SDL_free(szPath);
#else
   const char* szPath = SDL_GetBasePath();
   m_myPath = szPath;
#endif
#endif

   // store 2x
   m_wMyPath = MakeWString(m_myPath);
}

void VPinball::GetMyPrefPath()
{
#ifdef _WIN32
   // Use standard Windows AppData directory (to avoid requesting write permissions, and behave correctly for Windows restore,...)
   // That would look something like: "C:\Users\bob\AppData\Roaming\VPinballX\"
   m_myPrefPath = string(GetAppDataPath()) + PATH_SEPARATOR_CHAR + "VPinballX" + PATH_SEPARATOR_CHAR;
#elif defined(__ANDROID__)
   char *szPrefPath = SDL_GetPrefPath(NULL, NULL);
   m_myPrefPath = szPrefPath;
   SDL_free(szPrefPath);
#elif defined(__APPLE__) && defined(TARGET_OS_IOS) && TARGET_OS_IOS
   char *szPrefPath = SDL_GetPrefPath("../..", "Documents");
   m_myPrefPath = szPrefPath;
   SDL_free(szPrefPath);
#elif defined(__APPLE__) && defined(TARGET_OS_TV) && TARGET_OS_TV
   char *szPrefPath = SDL_GetPrefPath(NULL, "Documents");
   m_myPrefPath = szPrefPath;
   SDL_free(szPrefPath);
#else
   m_myPrefPath = string(getenv("HOME")) + PATH_SEPARATOR_CHAR + ".vpinball" + PATH_SEPARATOR_CHAR;
#endif
   if (!DirExists(m_myPrefPath)) {
      std::error_code ec;
      if (std::filesystem::create_directory(m_myPrefPath, ec)) {
         PLOGI << "Pref path created: " << m_myPrefPath;
      }
      else {
         PLOGE << "Unable to create pref path: " << m_myPrefPath;
      }
   }
}

//Post Work to the worker Thread
//Creates Worker-Thread if not present
//See Worker::VPWorkerThreadStart for infos
//workid int for the type of message (COMPLETE_AUTOSAVE | HANG_SNOOP_START | HANG_SNOOP_STOP)
//Second Parameter for message (AutoSavePackage (see worker.h) if COMPLETE_AUTOSAVE, otherwise nullptr)
//returns Handle to Event that get ack. If event is finished (unsure)
HANDLE VPinball::PostWorkToWorkerThread(int workid, LPARAM lParam)
{
#ifndef __STANDALONE__
   // Check if Workerthread was created once, otherwise create
   if (!m_workerthread)
   {
      g_hWorkerStarted = CreateEvent(nullptr, TRUE, FALSE, nullptr);
      m_workerthread = (HANDLE)_beginthreadex(nullptr, 0, VPWorkerThreadStart, 0, 0, &m_workerthreadid);
      if (WaitForSingleObject(g_hWorkerStarted, 5000) == WAIT_TIMEOUT)
      {
      }
      SetThreadPriority(m_workerthread, THREAD_PRIORITY_LOWEST);
   }
   HANDLE hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
   PostThreadMessage(m_workerthreadid, workid, (WPARAM)hEvent, lParam);
   return hEvent;
#else
   return 0L;
#endif
}

void VPinball::SetAutoSaveMinutes(const int minutes)
{
   m_autosaveTime = (minutes <= 0) ? -1 : minutes * (60 * 1000); // convert to milliseconds
}

//Post Work to the worker Thread
//Creates Worker-Thread if not present
//See Worker::VPWorkerThreadStart for infos
//workid int for the type of message (COMPLETE_AUTOSAVE | HANG_SNOOP_START | HANG_SNOOP_STOP)
//Second Parameter for message (AutoSavePackage (see worker.h) if COMPLETE_AUTOSAVE, otherwise nullptr)
//returns Handle to Event that get ack. If event is finished (unsure)
void VPinball::InitTools()
{
   m_ToolCur = IDC_SELECT;
}

//Initializes Default Values of many variables (from Registry if keys are present).
//Registry Values under HKEY-CURRENT-USER/Software/Visual Pinball
//Deadzone, ShowDragPoints, DrawLightCenters,
//AutoSaveOn, AutoSaveTime, SecurityLevel
//Gets the last loaded Tables (List under File-Menu)
void VPinball::InitRegValues()
{
   m_alwaysDrawDragPoints = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "ShowDragPoints"s, false);
   m_alwaysDrawLightCenters = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "DrawLightCenters"s, false);
   m_gridSize = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "GridSize"s, 50);

   const bool autoSave = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "AutoSaveOn"s, true);
   if (autoSave)
   {
      m_autosaveTime = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "AutoSaveTime"s, AUTOSAVE_DEFAULT_TIME);
      SetAutoSaveMinutes(m_autosaveTime);
   }
   else
      m_autosaveTime = -1;

   m_securitylevel = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "SecurityLevel"s, DEFAULT_SECURITY_LEVEL);

   m_dummyMaterial.m_cBase = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "DefaultMaterialColor"s, 0xB469FF);
   m_elemSelectColor = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "ElementSelectColor"s, 0x00FF0000);
   m_elemSelectLockedColor = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "ElementSelectLockedColor"s, 0x00A7726D);
   m_backgroundColor = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "BackgroundColor"s, 0x008D8D8D);
   m_fillColor = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "FillColor"s, 0x00B1CFB3);

   if (m_securitylevel < eSecurityNone || m_securitylevel > eSecurityNoControls)
      m_securitylevel = eSecurityNoControls;

   m_recentTableList.clear();
   // get the list of the last n loaded tables
   for (int i = 0; i < LAST_OPENED_TABLE_COUNT; i++)
   {
      string szTableName;
      if (g_pvp->m_settings.LoadValue(Settings::RecentDir, "TableFileName" + std::to_string(i), szTableName))
         m_recentTableList.push_back(szTableName);
      else
         break;
   }

   m_convertToUnit = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "Units"s, 0);
}

void VPinball::AddMDITable(PinTableMDI* mdiTable) 
{
   AddMDIChild(mdiTable); 
}

void VPinball::SetClipboard(vector<IStream*> * const pvstm)
{
   for (size_t i = 0; i < m_vstmclipboard.size(); i++)
      m_vstmclipboard[i]->Release();
   m_vstmclipboard.clear();

   if (pvstm)
      for (size_t i = 0; i < pvstm->size(); i++)
         m_vstmclipboard.push_back((*pvstm)[i]);
}

void VPinball::SetCursorCur(HINSTANCE hInstance, LPCTSTR lpCursorName)
{
#ifndef __STANDALONE__
   const HCURSOR hcursor = LoadCursor(hInstance, lpCursorName);
   SetCursor(hcursor);
#endif
}

void VPinball::SetActionCur(const string& szaction)
{
#ifndef __STANDALONE__
   ::SendMessage(m_hwndStatusBar, SB_SETTEXT, 3 | 0, (size_t)szaction.c_str());
#endif
}

void VPinball::SetStatusBarElementInfo(const string& info)
{
#ifndef __STANDALONE__
   ::SendMessage(m_hwndStatusBar, SB_SETTEXT, 4 | 0, (size_t)info.c_str());
#endif
}

bool VPinball::OpenFileDialog(const string& initDir, vector<string>& filename, const char* const fileFilter, const char* const defaultExt, const DWORD flags, const string& windowTitle) //!! use this all over the place and move to some standard header
{
#ifndef __STANDALONE__
   CFileDialog fileDlg(TRUE, defaultExt, initDir.c_str(), nullptr, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER | flags, fileFilter); // OFN_EXPLORER needed, otherwise GetNextPathName buggy 
   if (!windowTitle.empty())
      fileDlg.SetTitle(windowTitle.c_str());
   if (fileDlg.DoModal(GetHwnd()) == IDOK)
   {
      int pos = 0;
      while (pos != -1)
         filename.emplace_back(fileDlg.GetNextPathName(pos));

      return true;
   }
   else
   {
      filename.emplace_back(string());

      return false;
   }
#else
   return false;
#endif
}

bool VPinball::SaveFileDialog(const string& initDir, vector<string>& filename, const char* const fileFilter, const char* const defaultExt, const DWORD flags, const string& windowTitle) //!! use this all over the place and move to some standard header
{
#ifndef __STANDALONE__
   CFileDialog fileDlg(FALSE, defaultExt, initDir.c_str(), nullptr, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER | flags, fileFilter); // OFN_EXPLORER needed, otherwise GetNextPathName buggy 
   if (!windowTitle.empty())
      fileDlg.SetTitle(windowTitle.c_str());
   if (fileDlg.DoModal(GetHwnd()) == IDOK)
   {
      int pos = 0;
      while (pos != -1)
         filename.emplace_back(fileDlg.GetNextPathName(pos));

      return true;
   }
   else
   {
      filename.emplace_back(string());

      return false;
   }
#else
   return false;
#endif
}

CDockProperty *VPinball::GetPropertiesDocker()
{
   #ifndef __STANDALONE__
      if (m_propertyDialog == nullptr || !m_dockProperties->IsWindow())
      {
         constexpr int dockStyle = DS_DOCKED_RIGHT | DS_CLIENTEDGE | DS_NO_CLOSE;
         m_dockProperties = (CDockProperty *)AddDockedChild(new CDockProperty, dockStyle, 280, IDD_PROPERTY_DIALOG);
         assert(m_dockProperties->GetContainer());
         m_dockProperties->GetContainer()->SetHideSingleTab(TRUE);
         m_propertyDialog = m_dockProperties->GetContainProperties()->GetPropertyDialog();
      }
   #endif
   return m_dockProperties;
}

CDockToolbar *VPinball::GetToolbarDocker()
{
   #ifndef __STANDALONE__
      if (m_dockToolbar == nullptr || !m_dockToolbar->IsWindow())
      {
         constexpr int dockStyle = DS_DOCKED_LEFT | DS_CLIENTEDGE | DS_NO_CLOSE;
         m_dockToolbar = (CDockToolbar *)AddDockedChild(new CDockToolbar, dockStyle, 110, IDD_TOOLBAR);
         assert(m_dockToolbar->GetContainer());
         m_dockToolbar->GetContainer()->SetHideSingleTab(TRUE);
         m_toolbarDialog = m_dockToolbar->GetContainToolbar()->GetToolbarDialog();
      }
   #endif
   return m_dockToolbar;
}

void VPinball::ResetAllDockers()
{
#ifndef __STANDALONE__
   const bool createNotes = m_dockNotes != nullptr;
   CloseAllDockers();
   // FIXME these are Windows only registry key. Move to g_pvp->m_settings. ?
   // DeleteSubKey("Editor\\Dock Windows"s); // Old Win32xx
   // DeleteSubKey("Editor\\Dock Settings"s);// Win32xx 9+
   CreateDocker();
   if (createNotes)
      GetDefaultNotesDocker();
#endif
}

CDockNotes* VPinball::GetDefaultNotesDocker()
{
#ifndef __STANDALONE__
   constexpr int dockStyle = DS_CLIENTEDGE;
   RECT rc;
   rc.left = 0;
   rc.top = 0;
   rc.right = 480;
   rc.bottom = 380;
   m_dockNotes = (CDockNotes*)AddUndockedChild(new CDockNotes, dockStyle, 200, rc, IDD_NOTES_DIALOG);
   assert(m_dockNotes->GetContainer());
   m_dockNotes->GetContainer()->SetHideSingleTab(TRUE);
   m_notesDialog = m_dockNotes->GetContainNotes()->GetNotesDialog();
#endif
   return m_dockNotes;
}

CDockNotes* VPinball::GetNotesDocker()
{
#ifndef __STANDALONE__
   if (m_dockNotes != nullptr && !m_dockNotes->IsWindowEnabled())
   {
      m_dockNotes->ShowWindow();
      m_dockNotes->Enable();
   }
#endif
   return m_dockNotes;
}

CDockLayers *VPinball::GetLayersDocker()
{
#ifndef __STANDALONE__
   if (m_dockLayers == nullptr || !m_dockLayers->IsWindow())
   {
      constexpr int dockStyle = DS_DOCKED_BOTTOM | DS_CLIENTEDGE | DS_NO_CLOSE;
      m_dockLayers = (CDockLayers *)GetPropertiesDocker()->AddDockedChild(new CDockLayers, dockStyle, 380, IDD_LAYERS);
      assert(m_dockLayers->GetContainer());
      m_dockLayers->GetContainer()->SetHideSingleTab(TRUE);
   }
#endif
   return m_dockLayers;
}

void VPinball::CreateDocker()
{
#ifndef __STANDALONE__
   LoadDockRegistrySettings(DOCKER_REGISTRY_KEY);
   GetPropertiesDocker()->GetContainer()->SetHideSingleTab(TRUE);
   GetLayersDocker()->GetContainer()->SetHideSingleTab(TRUE);
   GetToolbarDocker()->GetContainer()->SetHideSingleTab(TRUE);
#endif
}

void VPinball::SetPosCur(float x, float y)
{
#ifndef __STANDALONE__
   // display position 1st column in VP units
   char szT[256];
   sprintf_s(szT, sizeof(szT), "%.4f, %.4f", x, y);
   ::SendMessage(m_hwndStatusBar, SB_SETTEXT, 0 | 0, (size_t)szT);

   // display converted position in separate status
   if (m_convertToUnit != 2) 
   {
       switch (m_convertToUnit)
       {
           case 0:
               sprintf_s(szT, sizeof(szT), "%.2f, %.2f %s", ConvertToUnit(x), ConvertToUnit(y), " (inch)");
               break;
           case 1:
               sprintf_s(szT, sizeof(szT), "%.2f, %.2f %s", ConvertToUnit(x), ConvertToUnit(y), " (mm)");
               break;
           default:
               assert(!"wrong unit");
               break;
       }
       ::SendMessage(m_hwndStatusBar, SB_SETTEXT, 0 | 2, (size_t)szT);
   }

   m_mouseCursorPosition.x = x;
   m_mouseCursorPosition.y = y;
#endif
}

void VPinball::SetObjectPosCur(float x, float y)
{
   char szT[256];
   sprintf_s(szT, sizeof(szT), "%.4f, %.4f", x, y);
#ifndef __STANDALONE__
   ::SendMessage(m_hwndStatusBar, SB_SETTEXT, 1 | 0, (size_t)szT);
#endif
}

void VPinball::ClearObjectPosCur()
{
#ifndef __STANDALONE__
   ::SendMessage(m_hwndStatusBar, SB_SETTEXT, 1 | 0, (size_t)"");
#endif
}

float VPinball::ConvertToUnit(const float value) const
{
   switch (m_convertToUnit)
   {
      case 0:
        return vpUnitsToInches(value);
      case 1:
        return vpUnitsToMillimeters(value);
      case 2:
        return value;
   }
   return 0;
}

void VPinball::SetPropSel(VectorProtected<ISelect> &pvsel)
{
#ifndef __STANDALONE__
   if (m_propertyDialog && m_propertyDialog->IsWindow())
      m_propertyDialog->UpdateTabs(pvsel);
   CComObject<PinTable>* const pt = GetActiveTable();
   if (pt && !g_pplayer)
      pt->SetFocus();
#endif
}

CMenu VPinball::GetMainMenu(int id)
{
#ifndef __STANDALONE__
   const CMenu& cm = GetMenu();
   const int count = /*m_mainMenu*/cm.GetMenuItemCount();
   return /*m_mainMenu*/cm.GetSubMenu(id + ((count > NUM_MENUS) ? 1 : 0)); // MDI has added its stuff (table icon for first menu item)
#else
   return CMenu();
#endif
}

#ifndef __STANDALONE__
class InfoDialog final : public CDialog
{
public:
   InfoDialog(const string &message)
      : CDialog(IDD_INFOTEXT)
      , m_message(message)
   {
   }

   BOOL OnInitDialog() override
   {
      SetDlgItemText(IDC_INFOTEXT_EDIT, m_message.c_str());
      return TRUE;
   }

   string m_message;
};
#endif

bool VPinball::ParseCommand(const size_t code, const bool notify)
{
#ifndef __STANDALONE__
   // check if it's an Editable tool
   const ItemTypeEnum type = EditableRegistry::TypeFromToolID((int)code);
   if (type != eItemInvalid)
   {
      m_ToolCur = (int)code;

      if (notify) // accelerator - mouse can be over table already
      {
         POINT pt;
         GetCursorPos(&pt);
         SetCursorPos(pt.x, pt.y);
      }
      return true;
   }

   /* a MDI client window starts with ID OPEN_MDI_TABLE_IDM and is incremented by Windows if a new window(table) is loaded 
      if the user switches a table (multiple tables are loaded) support up to MAX_OPEN_TABLES loaded tables here */
   assert(MAX_OPEN_TABLES == (LAST_MDI_TABLE_IDM-OPEN_MDI_TABLE_IDM+1));
   if (code >= OPEN_MDI_TABLE_IDM && code <= LAST_MDI_TABLE_IDM)
   {
      /* close all dialogs if the table is changed to prevent further issues */
      CloseAllDialogs();

      const CMenu mainMenu = GetMenu();
      // mark selected table as checked, all others as unchecked
      for (unsigned int i = OPEN_MDI_TABLE_IDM; i <= LAST_MDI_TABLE_IDM; ++i)
         mainMenu.CheckMenuItem(i, MF_BYCOMMAND | ((i == code) ? MF_CHECKED : MF_UNCHECKED));
   }

   switch (code)
   {
   case IDM_NEW:
   case ID_NEW_BLANKTABLE:
   case ID_NEW_EXAMPLETABLE:
   case ID_NEW_STRIPPEDTABLE:
   case ID_NEW_LIGHTSEQTABLE:
   {
      OpenNewTable(code);
      return true;
   }
   case ID_DELETE:
   {
      ProcessDeleteElement();
      return true;
   }
   case ID_TABLE_CAMERAMODE:
   case ID_TABLE_LIVEEDIT:
   case ID_TABLE_PLAY:
   {
      DoPlay(code == ID_TABLE_PLAY ? 0 : code == ID_TABLE_CAMERAMODE ? 1 : 2);
      return true;
   }
   case ID_TABLE_LOCK:
   {
      CComObject<PinTable> *const ptCur = GetActiveTable();
      if (ptCur)
      {
         if (ptCur->IsLocked())
         {
            if (IDYES == MessageBox("This table is locked to avoid modification.\n\nYou do not need to unlock it to adjust settings like the camera or rendering options.\n\nAre you sure you want to unlock the table ?", "Table Unlocking", MB_YESNO | MB_ICONINFORMATION))
               ptCur->ToggleLock();
         }
         else if (!ptCur->IsLocked())
         {
            if (IDYES == MessageBox("This will lock the table to prevent unexpected modifications.\n\nAre you sure you want to lock the table ?", "Table locking", MB_YESNO | MB_ICONINFORMATION))
            {
               ptCur->ToggleLock();
               string msg = ptCur->AuditTable(true);
               InfoDialog info(msg);
               info.DoModal();
            }
         }
         ptCur->ClearMultiSel(nullptr);
         SetPropSel(ptCur->m_vmultisel);
         GetLayersListDialog()->ResetView();
         ToggleToolbar();
         SetEnableMenuItems();
         ptCur->SetDirtyDraw();
      }
      return true;
   }
   case ID_SCRIPT_SHOWIDE:
   case ID_EDIT_SCRIPT:
   {
      ToggleScriptEditor();
      return true;
   }
   case ID_EDIT_BACKGLASSVIEW:
   {
      ToggleBackglassView();
      return true;
   }
   case ID_EDIT_SEARCH:
   {
      ShowSearchSelect();
      return true;
   }
   case ID_EDIT_SETDEFAULTPHYSICS:
   {
      SetDefaultPhysics();
      break;
   }
   case ID_LOCK:
   {
      CComObject<PinTable> * const ptCur = GetActiveTable();
      if (ptCur)
         ptCur->LockElements();
      return true;
   }
   case ID_EDIT_DRAWINGORDER_HIT:
   {
      //DialogBoxParam(theInstance, MAKEINTRESOURCE(IDD_DRAWING_ORDER), m_hwnd, DrawingOrderProc, 0);
      ShowDrawingOrderDialog(false);
      return true;
   }
   case ID_EDIT_DRAWINGORDER_SELECT:
   {
      //DialogBoxParam(theInstance, MAKEINTRESOURCE(IDD_DRAWING_ORDER), m_hwnd, DrawingOrderProc, 0);
      ShowDrawingOrderDialog(true);
      return true;
   }
   case ID_VIEW_SOLID:
   case ID_VIEW_OUTLINE:
   {
      SetViewSolidOutline(code);
      return true;
   }
   case ID_VIEW_GRID:
   {
      ShowGridView();
      return true;
   }
   case ID_VIEW_BACKDROP:
   {
      ShowBackdropView();
      return true;
   }
   case IDC_SELECT:
   case ID_TABLE_MAGNIFY:
   {
      m_ToolCur = (int)code;
      if (notify) // accelerator - mouse can be over table already
      {
         POINT pt;
         GetCursorPos(&pt);
         SetCursorPos(pt.x, pt.y);
      }
      return true;
   }
   case ID_ADD_CTRL_POINT:
   {
      AddControlPoint();
      return true;
   }
   case ID_ADD_SMOOTH_CTRL_POINT:
   {
      AddSmoothControlPoint();
      return true;
   }
   case IDM_SAVE:
   {
      SaveTable(false);
      return true;
   }
   case IDM_SAVEAS:
   {
      SaveTable(true);
      return true;
   }

   case RECENT_FIRST_MENU_IDM:
   case RECENT_FIRST_MENU_IDM + 1:
   case RECENT_FIRST_MENU_IDM + 2:
   case RECENT_FIRST_MENU_IDM + 3:
   case RECENT_FIRST_MENU_IDM + 4:
   case RECENT_FIRST_MENU_IDM + 5:
   case RECENT_FIRST_MENU_IDM + 6:
   case RECENT_FIRST_MENU_IDM + 7:
   {
      OpenRecentFile(code);
      return true;
   }

   case IDM_OPEN:
   {
      LoadFile(true);
      return true;
   }
   case IDM_CLOSE:
   case SC_CLOSE:
   {
      CComObject<PinTable> *const ptCur = GetActiveTable();
      if (ptCur)
         CloseTable(ptCur);
      return true;
   }
   case IDC_COPY:
   {
      CopyPasteElement(COPY);
      return true;
   }
   case IDC_PASTE:
   {
      CopyPasteElement(PASTE);
      return true;
   }
   case IDC_PASTEAT:
   {
      CopyPasteElement(PASTE_AT);
      return true;
   }
   case ID_EDIT_UNDO:
   {
      CComObject<PinTable> * const ptCur = GetActiveTable();
      if (ptCur)
         ptCur->Undo();
      return true;
   }
   case ID_FILE_EXPORT_BLUEPRINT:
   {
      CComObject<PinTable> * const ptCur = GetActiveTable();
      if (ptCur)
         ptCur->ExportBlueprint();
      return true;
   }
   case ID_EXPORT_TABLEMESH:
   {
      CComObject<PinTable> * const ptCur = GetActiveTable();
      if (ptCur)
         ptCur->ExportTableMesh();
      return true;
   }
   case ID_IMPORT_BACKDROPPOV:
   {
      CComObject<PinTable> * const ptCur = GetActiveTable();
      if (ptCur)
         ptCur->ImportBackdropPOV(string());
      return true;
   }
   case ID_EXPORT_BACKDROPPOV:
   {
      CComObject<PinTable> * const ptCur = GetActiveTable();
      if (ptCur)
         ptCur->ExportBackdropPOV();
      return true;
   }
   case ID_FILE_EXIT:
   {
      PostMessage(WM_CLOSE, 0, 0);
      return true;
   }
   case ID_EDIT_AUDIOOPTIONS:
   {
      m_audioOptDialog.DoModal(GetHwnd());
      return true;
   }
   case ID_EDIT_PHYSICSOPTIONS:
   {
      m_physicsOptDialog.DoModal(GetHwnd());
      return true;
   }
   case ID_EDIT_EDITOROPTIONS:
   {
      m_editorOptDialog.DoModal(GetHwnd());
      // refresh editor options from the registry
      InitRegValues();
      // force a screen refresh (it an active table is loaded)
      CComObject<PinTable> * const ptCur = GetActiveTable();
      if (ptCur)
         ptCur->SetDirtyDraw();
      return true;
   }
   case ID_EDIT_VIDEOOPTIONS:
   {
      VideoOptionProperties * videoOptProperties = new VideoOptionProperties(GetHwnd());
      videoOptProperties->DoModal();
      delete videoOptProperties;
      return true;
   }
   #if defined(ENABLE_VR) || defined(ENABLE_XR)
      case ID_EDIT_VROPTIONS:
      {
         m_vrOptDialog.DoModal(GetHwnd());
         return true;
      }
   #endif
   case ID_TABLE_TABLEINFO:
   {
      CComObject<PinTable> * const ptCur = GetActiveTable();
      if (ptCur)
         m_tableInfoDialog.DoModal(GetHwnd());
      return true;
   }
   case IDM_IMAGE_EDITOR:
   case ID_TABLE_IMAGEMANAGER:
   {
      CComObject<PinTable> * const ptCur = GetActiveTable();
      if (ptCur)
         ShowSubDialog(m_imageMngDlg, true);
      return true;
   }
   case IDM_SOUND_EDITOR:
   case ID_TABLE_SOUNDMANAGER:
   {
      CComObject<PinTable> * const ptCur = GetActiveTable();
      if (ptCur)
         ShowSubDialog(m_soundMngDlg, true);
      return true;
   }
   case IDM_MATERIAL_EDITOR:
   case ID_TABLE_MATERIALMANAGER:
   {
      CComObject<PinTable> * const ptCur = GetActiveTable();
      if (ptCur)
         ShowSubDialog(m_materialDialog, true);
      return true;
   }
   case ID_TABLE_NOTES:
   {
      if (GetNotesDocker() == nullptr || !GetNotesDocker()->IsWindow())
         GetDefaultNotesDocker();
      else
         GetNotesDocker()->ShowWindow();
      return true;
   }
   case ID_TABLE_FONTMANAGER:
   {
      CComObject<PinTable> * const ptCur = GetActiveTable();
      if (ptCur)
         /*const DWORD foo =*/ DialogBoxParam(theInstance, MAKEINTRESOURCE(IDD_FONTDIALOG), GetHwnd(), FontManagerProc, (size_t)ptCur);
      return true;
   }
   case ID_TABLE_DIMENSIONMANAGER:
   {
      ShowSubDialog(m_dimensionDialog, true);
      return true;
   }
   case IDM_COLLECTION_EDITOR:
   case ID_TABLE_COLLECTIONMANAGER:
   {
      CComObject<PinTable> * const ptCur = GetActiveTable();
      if (ptCur)
         ShowSubDialog(m_collectionMngDlg, true);
      return true;
   }
   case ID_TABLE_RENDERPROBEMANAGER:
   {
      ShowSubDialog(m_renderProbeDialog, true);
      return true;
   }
   case ID_PREFERENCES_SECURITYOPTIONS:
   {
      DialogBoxParam(theInstance, MAKEINTRESOURCE(IDD_SECURITY_OPTIONS), GetHwnd(), SecurityOptionsProc, 0);

      // refresh editor options from the registry
      InitRegValues();
      return true;
   }
   case ID_EDIT_KEYS:
   {
      KeysConfigDialog * const keysConfigDlg = new KeysConfigDialog();
      keysConfigDlg->DoModal();
      delete keysConfigDlg;
      return true;
   }
   case ID_HELP_ABOUT:
   {
      ShowSubDialog(m_aboutDialog, true);
      return true;
   }
   case ID_WINDOW_CASCADE:
   {
      MDICascade();
      return true;
   }
   case ID_WINDOW_TILE:
   {
      MDITile();
      return true;
   }
   case ID_WINDOW_ARRANGEICONS:
   {
      MDIIconArrange();
      return true;
   }
   }
#endif
   return false;
}

void VPinball::ToggleToolbar()
{
#ifndef __STANDALONE__
   if (m_toolbarDialog)
      m_toolbarDialog->EnableButtons();
#endif
}

void VPinball::DoPlay(const int playMode)
{
   if (g_pplayer)
      return; // Can't play twice

   CComObject<PinTable> *const table = GetActiveTable();
   if (table == nullptr)
      return;

#ifdef __LIBVPINBALL__
   VPinballLib::VPinball::SendEvent(VPinballLib::Event::Play, nullptr);
#endif

   PLOGI << "Starting Play mode [table: " << table->m_tableName << ", play mode: " << playMode << ']';
   ShowWindow(SW_HIDE);
   bool initError = false;
   if (false)
   {
      // Editor mode: create a player directly on the loaded table. Only user will be allowed to modify it (no scripting, animation, ...)
      new Player(table, table, playMode);
   }
   else
   {
      // Play mode: create a player on a (shallow) copy of the table, that will be animated by the script, animations, ...
      PinTable *live_table = table->CopyForPlay();
      if (live_table != nullptr)
      {
         table->EndAutoSaveCounter();
         new Player(table, live_table, playMode);
      }
   }

   // If we successfully created a player, switch to Player's main loop (needed to avoid interference between editor's Window Msg loop and player's specific msg loop, also Player has a fairly specific msg loop)
   if (g_pplayer == nullptr)
      initError = true;
   else
   {
      #ifdef ENABLE_SDL_VIDEO
      auto processWindowMessages = [&initError]()
      {
         const uint64_t startTick = usec();
         SDL_Event e;
         bool isPFWnd = true;
         static Vertex2D dragStart;
         static int dragging = 0;
         while (SDL_PollEvent(&e) != 0)
         {
            switch (e.type)
            {
            case SDL_EVENT_QUIT:
               g_pplayer->SetCloseState(Player::CloseState::CS_STOP_PLAY);
               break;
            case SDL_EVENT_WINDOW_FOCUS_GAINED:
               isPFWnd = SDL_GetWindowFromID(e.window.windowID) == g_pplayer->m_playfieldWnd->GetCore();
               g_pplayer->OnFocusChanged(true);
               break;
            case SDL_EVENT_WINDOW_FOCUS_LOST:
               isPFWnd = SDL_GetWindowFromID(e.window.windowID) == g_pplayer->m_playfieldWnd->GetCore();
               g_pplayer->OnFocusChanged(false);
               break;
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
               isPFWnd = SDL_GetWindowFromID(e.window.windowID) == g_pplayer->m_playfieldWnd->GetCore();
               g_pvp->QuitPlayer(Player::CloseState::CS_STOP_PLAY);
               break;
            case SDL_EVENT_KEY_UP:
            case SDL_EVENT_KEY_DOWN:
               isPFWnd = SDL_GetWindowFromID(e.key.windowID) == g_pplayer->m_playfieldWnd->GetCore();
               g_pplayer->ShowMouseCursor(false);
               break;
            case SDL_EVENT_TEXT_INPUT:
               isPFWnd = SDL_GetWindowFromID(e.text.windowID) == g_pplayer->m_playfieldWnd->GetCore();
               break;
            case SDL_EVENT_MOUSE_WHEEL:
               isPFWnd = SDL_GetWindowFromID(e.wheel.windowID) == g_pplayer->m_playfieldWnd->GetCore();
               break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP:
               isPFWnd = SDL_GetWindowFromID(e.button.windowID) == g_pplayer->m_playfieldWnd->GetCore();
               if (!isPFWnd)
               {
                  if (e.type == SDL_EVENT_MOUSE_BUTTON_UP)
                     dragging = 0;
                  else if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN && dragging == 0)
                     dragging = 1;
               }
               break;
            case SDL_EVENT_MOUSE_MOTION:
               isPFWnd = SDL_GetWindowFromID(e.motion.windowID) == g_pplayer->m_playfieldWnd->GetCore();
               if (isPFWnd) {
                  // We scale motion data since SDL expects DPI scaled points coordinates on Apple device, while it uses pixel coordinates on other devices (see SDL_WINDOWS_DPI_SCALING)
                  // For the time being, VPX always uses pixel coordinates, using setup obtained at window creation time.
                  e.motion.x *= SDL_GetWindowPixelDensity(g_pplayer->m_playfieldWnd->GetCore());
                  e.motion.y *= SDL_GetWindowPixelDensity(g_pplayer->m_playfieldWnd->GetCore());
                  static float m_lastcursorx = FLT_MAX, m_lastcursory = FLT_MAX;
                  if (m_lastcursorx != e.motion.x || m_lastcursory != e.motion.y)
                  {
                     m_lastcursorx = e.motion.x;
                     m_lastcursory = e.motion.y;
                     g_pplayer->ShowMouseCursor(true);
                  }
               }
               else if (dragging)
               {
                  // Handle dragging of auxiliary windows
                  SDL_Window *sdlWnd = SDL_GetWindowFromID(e.motion.windowID);
                  std::vector<VPX::Window*> windows = {
                     g_pplayer->m_scoreviewOutput.GetWindow(),
                     g_pplayer->m_backglassOutput.GetWindow(),
                     g_pplayer->m_topperOutput.GetWindow(),
                  };
                  for (VPX::Window* wnd : windows)
                  {
                     if (wnd && sdlWnd == wnd->GetCore())
                     {
                        int x, y;
                        wnd->GetPos(x, y);
                        Vertex2D click(x + e.motion.x, y + e.motion.y);
                        if (dragging > 1)
                           wnd->SetPos(static_cast<int>(x + click.x - dragStart.x), static_cast<int>(y + click.y - dragStart.y));
                        dragStart = click;
                        dragging = 2;
                        break;
                     }
                  }
               }
               break;
            }

            if (isPFWnd)
               ImGui_ImplSDL3_ProcessEvent(&e);

            #ifdef ENABLE_SDL_INPUT
            g_pplayer->m_pininput.HandleSDLEvent(e);
            #endif

            // Limit to 1ms of OS message processing per call
            if ((usec() - startTick) > 1000ull)
               break;
         }
      };

      #elif !defined(__STANDALONE__)
      auto processWindowMessages = [&initError]()
      {
         const uint64_t startTick = usec();
         MSG msg;
         while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
         {
            if (msg.message == WM_QUIT)
            {
               if (g_pplayer->GetCloseState() == Player::CS_PLAYING || g_pplayer->GetCloseState() == Player::CS_USER_INPUT)
                  g_pplayer->SetCloseState(Player::CS_STOP_PLAY);
               return;
            }
            try
            {
               bool consumed = false;
               if (g_pplayer->m_debugMode && g_pplayer->m_debuggerDialog.IsWindow())
                  consumed = !!g_pplayer->m_debuggerDialog.IsSubDialogMessage(msg);
               if (!consumed)
               {
                  TranslateMessage(&msg);
                  DispatchMessage(&msg);
               }
            }
            catch (...) // something failed on load/init
            {
               initError = true;
            }

            // Limit to 1ms of OS message processing per call
            if ((usec() - startTick) > 1000ull)
               break;
         }
      };
      #else
      auto processWindowMessages = []() {};
      #endif
      g_pplayer->GameLoop(processWindowMessages);

      #if (defined(__APPLE__) && (defined(TARGET_OS_IOS) && TARGET_OS_IOS))
         // iOS has its own game loop so that it can handle OS events (screenshots, etc)
         return;
      #endif

      delete g_pplayer;
      g_pplayer = nullptr;
   }

   ShowWindow(SW_SHOW);

   if (initError)
   {
      g_pvp->m_table_played_via_SelectTableOnStart = false;
   }

   #ifdef __LIBVPINBALL__
      VPinballLib::VPinball::SendEvent(VPinballLib::Event::Stopped, nullptr);
   #endif
}

bool VPinball::LoadFile(const bool updateEditor, VPXFileFeedback* feedback)
{
   string szInitialDir = g_pvp->m_settings.LoadValueWithDefault(Settings::RecentDir, "LoadDir"s, PATH_TABLES);

   vector<string> filename;
   if (!OpenFileDialog(szInitialDir, filename, "Visual Pinball Tables (*.vpx)\0*.vpx\0Old Visual Pinball Tables(*.vpt)\0*.vpt\0", "vpx", 0,
          !updateEditor ? "Select a Table to Play or press Cancel to enter Editor-Mode"s : string()))
      return false;

   const size_t index = filename[0].find_last_of(PATH_SEPARATOR_CHAR);
   if (index != string::npos)
      g_pvp->m_settings.SaveValue(Settings::RecentDir, "LoadDir"s, filename[0].substr(0, index));

   LoadFileName(filename[0], updateEditor, feedback);

   return true;
}

void VPinball::LoadFileName(const string& filename, const bool updateEditor, VPXFileFeedback* feedback)
{
   if (m_vtable.size() == MAX_OPEN_TABLES)
   {
      ShowError("Maximum amount of tables already loaded and open.");
      return;
   }

   if (!FileExists(filename))
   {
      ShowError("File not found \"" + filename + '"');
      return;
   }

   m_currentTablePath = PathFromFilename(filename);

   CloseAllDialogs();

   PinTableMDI * const mdiTable = new PinTableMDI(this);
   CComObject<PinTable> * const ppt = mdiTable->GetTable();
   const HRESULT hr = feedback != nullptr ? ppt->LoadGameFromFilename(filename, *feedback) : ppt->LoadGameFromFilename(filename);

   const bool hashing_error = (hr == APPX_E_BLOCK_HASH_INVALID || hr == APPX_E_CORRUPT_CONTENT);
   if (hashing_error)
   {
      const LocalString ls(IDS_CORRUPTFILE);
      ShowError(ls.m_szbuffer);
   }

   if (!SUCCEEDED(hr) && !hashing_error)
   {
      ShowError("This file does not exist, or is corrupt and failed to load.");

      delete mdiTable;
   }
   else
   {
      m_vtable.push_back(ppt);

      ppt->m_title = TitleFromFilename(filename);
#ifndef __STANDALONE__
      const DWORD attr = GetFileAttributes(filename.c_str());
      if ((attr != INVALID_FILE_ATTRIBUTES) && (attr & FILE_ATTRIBUTE_READONLY))
         ppt->m_title += " [READ ONLY]";
#endif

#ifdef __STANDALONE__
      g_pvp->m_ptableActive = ppt;
#endif

      ppt->InitTablePostLoad();

      AddMDITable(mdiTable);

      // auto-import POV settings, if it exists. This is kept for backward compatibility as POV settings 
      // are now normal settings stored with others in app/table ini file. It will be only imported if no ini settings are defined
      bool hasTablePovSettings = false, hasAppPovSettings = false;
      static const string vsPrefix[3] = { "ViewDT"s, "ViewCab"s, "ViewFSS"s };
      static const char *vsFields[15] = { "Mode", "ScaleX", "ScaleY", "ScaleZ", "PlayerX", "PlayerY", "PlayerZ", "LookAt", "Rotation", "FOV", "Layback", "HOfs", "VOfs", "WindowTop", "WindowBot" };
      for (int i = 0; i < 3; i++)
         for (int j = 0; j < 15; j++)
         {
            hasTablePovSettings |= ppt->m_settings.HasValue(Settings::TableOverride, vsPrefix[i] + vsFields[j], false);
            hasAppPovSettings |= g_pvp->m_settings.HasValue(Settings::TableOverride, vsPrefix[i] + vsFields[j], false);
         }
      if (!hasTablePovSettings)
      {
         string filenameAuto = m_currentTablePath + ppt->m_title + ".pov";
         if (FileExists(filenameAuto)) // We check if there is a matching table pov settings file first
         {
            ppt->ImportBackdropPOV(filenameAuto);
         }
         else if (!hasAppPovSettings) // Otherwise, we seek for autopov settings (only if we do not already have settings in the app settings)
         {
            filenameAuto = m_currentTablePath + "autopov.pov";
            if (FileExists(filenameAuto))
               ppt->ImportBackdropPOV(filenameAuto);
         }
      }

      // auto-import VBS table script, if it exists...
      string filenameAuto = m_currentTablePath + ppt->m_title + ".vbs";
      if (FileExists(filenameAuto)) // We check if there is a matching table vbs first
         ppt->m_pcv->LoadFromFile(filenameAuto);
      else // Otherwise we seek in the Scripts folder
      {
         filenameAuto = m_myPath + "scripts" + PATH_SEPARATOR_CHAR + ppt->m_title + ".vbs";
         if (FileExists(filenameAuto))
            ppt->m_pcv->LoadFromFile(filenameAuto);
      }

      // auto-import VPP settings, if it exists...
      filenameAuto = m_currentTablePath + ppt->m_title + ".vpp";
      if (FileExists(filenameAuto)) // We check if there is a matching table vpp settings file first
         ppt->ImportVPP(filenameAuto);
      else // Otherwise, we seek for autovpp settings
      {
         filenameAuto = m_currentTablePath + "autovpp.vpp";
         if (FileExists(filenameAuto))
            ppt->ImportVPP(filenameAuto);
      }

      // get the load path from the filename
      g_pvp->m_settings.SaveValue(Settings::RecentDir, "LoadDir"s, m_currentTablePath);

      // make sure the load directory is the active directory
      SetCurrentDirectory(m_currentTablePath.c_str());
      UpdateRecentFileList(filename);

      PLOGI << "UI Post Load Start";

      ppt->AddMultiSel(ppt, false, true, false);
      ppt->SetDirty(eSaveClean);
      if (updateEditor)
      {
#ifndef __STANDALONE__
         GetLayersListDialog()->ResetView();
         ToggleToolbar();
         if (m_dockNotes != nullptr)
            m_dockNotes->Enable();

         SetFocus();

         const string& audit = ppt->AuditTable(true);
         if (audit.find(". Error:"s) != std::string::npos)
         {
            InfoDialog info("This table contains error(s) that need to be fixed to ensure correct play.\r\n\r\n" + audit);
            info.DoModal(GetHwnd());
         }
#endif
      }

      PLOGI << "UI Post Load End";
   }
}

CComObject<PinTable> *VPinball::GetActiveTable()
{
   PinTableMDI * const mdiTable = (PinTableMDI *)GetActiveMDIChild();
   if (mdiTable && !m_unloadingTable)
      return mdiTable->GetTable();
   return nullptr;
}

bool VPinball::CanClose()
{
   while (!m_vtable.empty())
   {
      if (!m_vtable[0]->GetMDITable()->CanClose())
         return false;

      CloseTable(m_vtable[0]);
   }

   return true;
}

void VPinball::CloseTable(const PinTable * const ppt)
{
#ifndef __STANDALONE__
   m_unloadingTable = true;
   ppt->GetMDITable()->SendMessage(WM_SYSCOMMAND, SC_CLOSE, 0);
   m_unloadingTable = false;

   if (GetAllMDIChildren().empty())
   {
      ToggleToolbar();
      if (m_propertyDialog && m_propertyDialog->IsWindow())
         m_propertyDialog->DeleteAllTabs();
      if (m_notesDialog && m_notesDialog->IsWindow())
         m_notesDialog->Disable();
   }
#else
    auto it = std::find_if(m_vtable.begin(), m_vtable.end(),
      [ppt](CComObject<PinTable>* obj) { return obj == ppt; });

    if (it != m_vtable.end()) {
       CComObject<PinTable>* obj = *it;
       m_vtable.erase(it);
       obj->Release();
    }
#endif
}

void VPinball::SetEnableMenuItems()
{
#ifndef __STANDALONE__
   CComObject<PinTable> * const ptCur = GetActiveTable();

   // Set menu item to the correct state
   const CMenu mainMenu = GetMenu();

   mainMenu.CheckMenuItem(ID_EDIT_BACKGLASSVIEW, MF_BYCOMMAND | (m_backglassView ? MF_CHECKED : MF_UNCHECKED));

   // is there a valid table??
   constexpr UINT grayed = MF_BYCOMMAND | MF_GRAYED, enabled = MF_BYCOMMAND | MF_ENABLED;
   if (ptCur)
   {
      mainMenu.CheckMenuItem(ID_EDIT_SCRIPT, MF_BYCOMMAND | ((ptCur->m_pcv != nullptr && ptCur->m_pcv->m_visible && !ptCur->m_pcv->m_minimized) ? MF_CHECKED : MF_UNCHECKED));

      mainMenu.EnableMenuItem(IDM_CLOSE, enabled);
      mainMenu.EnableMenuItem(ID_EDIT_UNDO, ptCur->IsLocked() ? grayed : enabled);
      mainMenu.EnableMenuItem(ID_EDIT_BACKGLASSVIEW, enabled);
      mainMenu.EnableMenuItem(ID_TABLE_PLAY, enabled);
      mainMenu.EnableMenuItem(ID_TABLE_CAMERAMODE, enabled);
      mainMenu.EnableMenuItem(ID_TABLE_LIVEEDIT, enabled);
      mainMenu.EnableMenuItem(ID_TABLE_LOCK, enabled);
      mainMenu.EnableMenuItem(ID_TABLE_MAGNIFY, enabled);
      mainMenu.EnableMenuItem(ID_TABLE_TABLEINFO, enabled);
      mainMenu.EnableMenuItem(ID_TABLE_DIMENSIONMANAGER, ptCur->IsLocked() ? grayed : enabled);
      mainMenu.EnableMenuItem(ID_TABLE_RENDERPROBEMANAGER, ptCur->IsLocked() ? grayed : enabled);
      mainMenu.EnableMenuItem(ID_EDIT_SEARCH, ptCur->IsLocked() ? grayed : enabled);
      mainMenu.EnableMenuItem(ID_EDIT_DRAWINGORDER_HIT, ptCur->IsLocked() ? grayed : enabled);
      mainMenu.EnableMenuItem(ID_EDIT_DRAWINGORDER_SELECT, ptCur->IsLocked() ? grayed : enabled);
      // enable/disable save options
      mainMenu.EnableMenuItem(IDM_SAVE, enabled);
      mainMenu.EnableMenuItem(IDM_SAVEAS, enabled);
      mainMenu.EnableMenuItem(ID_FILE_EXPORT_BLUEPRINT, enabled);
      mainMenu.EnableMenuItem(ID_EXPORT_TABLEMESH, enabled);
      mainMenu.EnableMenuItem(ID_EXPORT_BACKDROPPOV, enabled);
      mainMenu.EnableMenuItem(ID_IMPORT_BACKDROPPOV, ptCur->IsLocked() ? grayed : enabled);

      // enable/disable script option
      mainMenu.EnableMenuItem(ID_EDIT_SCRIPT, ptCur->IsLocked() ? grayed : enabled);

      // enable/disable managers options
      mainMenu.EnableMenuItem(ID_TABLE_SOUNDMANAGER, ptCur->IsLocked() ? grayed : enabled);
      mainMenu.EnableMenuItem(ID_TABLE_IMAGEMANAGER, ptCur->IsLocked() ? grayed : enabled);
      mainMenu.EnableMenuItem(ID_TABLE_FONTMANAGER, ptCur->IsLocked() ? grayed : enabled);
      mainMenu.EnableMenuItem(ID_TABLE_MATERIALMANAGER, ptCur->IsLocked() ? grayed : enabled);
      mainMenu.EnableMenuItem(ID_TABLE_COLLECTIONMANAGER, ptCur->IsLocked() ? grayed : enabled);

      // enable/disable editing options
      mainMenu.EnableMenuItem(ID_LOCK, ptCur->IsLocked() ? grayed : enabled);
      mainMenu.EnableMenuItem(IDC_COPY, ptCur->IsLocked() ? grayed : enabled);
      mainMenu.EnableMenuItem(IDC_PASTE, ptCur->IsLocked() ? grayed : enabled);
      mainMenu.EnableMenuItem(IDC_PASTEAT, ptCur->IsLocked() ? grayed : enabled);
      mainMenu.EnableMenuItem(ID_DELETE, ptCur->IsLocked() ? grayed : enabled);
      mainMenu.EnableMenuItem(ID_TABLE_NOTES, enabled);
      mainMenu.EnableMenuItem(ID_EDIT_SETDEFAULTPHYSICS, ptCur->IsLocked() ? grayed : enabled);

      // enable/disable insert tool
      mainMenu.EnableMenuItem(ID_INSERT_WALL, ptCur->IsLocked() ? grayed : enabled);
      mainMenu.EnableMenuItem(ID_INSERT_GATE, ptCur->IsLocked() ? grayed : enabled);
      mainMenu.EnableMenuItem(ID_INSERT_RAMP, ptCur->IsLocked() ? grayed : enabled);
      mainMenu.EnableMenuItem(ID_INSERT_FLIPPER, ptCur->IsLocked() ? grayed : enabled);
      mainMenu.EnableMenuItem(ID_INSERT_PLUNGER, ptCur->IsLocked() ? grayed : enabled);
      mainMenu.EnableMenuItem(ID_INSERT_BUMPER, ptCur->IsLocked() ? grayed : enabled);
      mainMenu.EnableMenuItem(ID_INSERT_BALL, ptCur->IsLocked() ? grayed : enabled);
      mainMenu.EnableMenuItem(ID_INSERT_SPINNER, ptCur->IsLocked() ? grayed : enabled);
      mainMenu.EnableMenuItem(ID_INSERT_TIMER, ptCur->IsLocked() ? grayed : enabled);
      mainMenu.EnableMenuItem(ID_INSERT_TRIGGER, ptCur->IsLocked() ? grayed : enabled);
      mainMenu.EnableMenuItem(ID_INSERT_LIGHT, ptCur->IsLocked() ? grayed : enabled);
      mainMenu.EnableMenuItem(ID_INSERT_KICKER, ptCur->IsLocked() ? grayed : enabled);
      mainMenu.EnableMenuItem(ID_INSERT_TARGET, ptCur->IsLocked() ? grayed : enabled);
      mainMenu.EnableMenuItem(ID_INSERT_DECAL, ptCur->IsLocked() ? grayed : enabled);
      mainMenu.EnableMenuItem(ID_INSERT_TEXTBOX, ptCur->IsLocked() ? grayed : enabled);
      mainMenu.EnableMenuItem(ID_INSERT_DISPREEL, ptCur->IsLocked() ? grayed : enabled);
      mainMenu.EnableMenuItem(ID_INSERT_LIGHTSEQ, ptCur->IsLocked() ? grayed : enabled);
      mainMenu.EnableMenuItem(ID_INSERT_PRIMITIVE, ptCur->IsLocked() ? grayed : enabled);
      mainMenu.EnableMenuItem(ID_INSERT_FLASHER, ptCur->IsLocked() ? grayed : enabled);
      mainMenu.EnableMenuItem(ID_INSERT_RUBBER, ptCur->IsLocked() ? grayed : enabled);

      mainMenu.CheckMenuItem(ID_VIEW_SOLID, MF_BYCOMMAND | (ptCur->RenderSolid() ? MF_CHECKED : MF_UNCHECKED));
      mainMenu.CheckMenuItem(ID_VIEW_OUTLINE, MF_BYCOMMAND | (ptCur->RenderSolid() ? MF_UNCHECKED : MF_CHECKED));

      mainMenu.CheckMenuItem(ID_VIEW_GRID, MF_BYCOMMAND | (ptCur->m_grid ? MF_CHECKED : MF_UNCHECKED));
      mainMenu.CheckMenuItem(ID_VIEW_BACKDROP, MF_BYCOMMAND | (ptCur->m_backdrop ? MF_CHECKED : MF_UNCHECKED));
   }
   else
   {
      /* no valid table, disable a few items */
      mainMenu.EnableMenuItem(IDM_CLOSE, grayed);
      mainMenu.EnableMenuItem(IDM_SAVE, grayed);
      mainMenu.EnableMenuItem(IDM_SAVEAS, grayed);
      mainMenu.EnableMenuItem(ID_FILE_EXPORT_BLUEPRINT, grayed);
      mainMenu.EnableMenuItem(ID_EXPORT_TABLEMESH, grayed);
      mainMenu.EnableMenuItem(ID_EXPORT_BACKDROPPOV, grayed);
      mainMenu.EnableMenuItem(ID_IMPORT_BACKDROPPOV, grayed);

      mainMenu.EnableMenuItem(ID_LOCK, grayed);
      mainMenu.EnableMenuItem(ID_EDIT_UNDO, grayed);
      mainMenu.EnableMenuItem(IDC_COPY, grayed);
      mainMenu.EnableMenuItem(IDC_PASTE, grayed);
      mainMenu.EnableMenuItem(IDC_PASTEAT, grayed);
      mainMenu.EnableMenuItem(ID_DELETE, grayed);
      mainMenu.EnableMenuItem(ID_EDIT_SCRIPT, grayed);
      mainMenu.EnableMenuItem(ID_EDIT_BACKGLASSVIEW, grayed);
      mainMenu.EnableMenuItem(ID_TABLE_NOTES, grayed);
      mainMenu.EnableMenuItem(ID_EDIT_SETDEFAULTPHYSICS, grayed);

      mainMenu.EnableMenuItem(ID_TABLE_PLAY, grayed);
      mainMenu.EnableMenuItem(ID_TABLE_CAMERAMODE, grayed);
      mainMenu.EnableMenuItem(ID_TABLE_LIVEEDIT, grayed);
      mainMenu.EnableMenuItem(ID_TABLE_LOCK, grayed);
      mainMenu.EnableMenuItem(ID_TABLE_SOUNDMANAGER, grayed);
      mainMenu.EnableMenuItem(ID_TABLE_IMAGEMANAGER, grayed);
      mainMenu.EnableMenuItem(ID_TABLE_FONTMANAGER, grayed);
      mainMenu.EnableMenuItem(ID_TABLE_MATERIALMANAGER, grayed);
      mainMenu.EnableMenuItem(ID_TABLE_COLLECTIONMANAGER, grayed);
      mainMenu.EnableMenuItem(ID_TABLE_DIMENSIONMANAGER, grayed);
      mainMenu.EnableMenuItem(ID_TABLE_RENDERPROBEMANAGER, grayed);
      mainMenu.EnableMenuItem(ID_TABLE_TABLEINFO, grayed);
      mainMenu.EnableMenuItem(ID_TABLE_MAGNIFY, grayed);
      mainMenu.EnableMenuItem(ID_EDIT_SEARCH, grayed);
      mainMenu.EnableMenuItem(ID_EDIT_DRAWINGORDER_HIT, grayed);
      mainMenu.EnableMenuItem(ID_EDIT_DRAWINGORDER_SELECT, grayed);

      // enable/disable insert tool
      mainMenu.EnableMenuItem(ID_INSERT_WALL, grayed);
      mainMenu.EnableMenuItem(ID_INSERT_GATE, grayed);
      mainMenu.EnableMenuItem(ID_INSERT_RAMP, grayed);
      mainMenu.EnableMenuItem(ID_INSERT_FLIPPER, grayed);
      mainMenu.EnableMenuItem(ID_INSERT_PLUNGER, grayed);
      mainMenu.EnableMenuItem(ID_INSERT_BUMPER, grayed);
      mainMenu.EnableMenuItem(ID_INSERT_BALL, grayed);
      mainMenu.EnableMenuItem(ID_INSERT_SPINNER, grayed);
      mainMenu.EnableMenuItem(ID_INSERT_TIMER, grayed);
      mainMenu.EnableMenuItem(ID_INSERT_TRIGGER, grayed);
      mainMenu.EnableMenuItem(ID_INSERT_LIGHT, grayed);
      mainMenu.EnableMenuItem(ID_INSERT_KICKER, grayed);
      mainMenu.EnableMenuItem(ID_INSERT_TARGET, grayed);
      mainMenu.EnableMenuItem(ID_INSERT_DECAL, grayed);
      mainMenu.EnableMenuItem(ID_INSERT_TEXTBOX, grayed);
      mainMenu.EnableMenuItem(ID_INSERT_DISPREEL, grayed);
      mainMenu.EnableMenuItem(ID_INSERT_LIGHTSEQ, grayed);
      mainMenu.EnableMenuItem(ID_INSERT_PRIMITIVE, grayed);
      mainMenu.EnableMenuItem(ID_INSERT_FLASHER, grayed);
      mainMenu.EnableMenuItem(ID_INSERT_RUBBER, grayed);
   }
#endif
}

void VPinball::UpdateRecentFileList(const string& filename)
{
   const size_t old_count = m_recentTableList.size();

   // if the loaded file name is a valid one then add it to the top of the list
   if (!filename.empty())
   {
      vector<string> newList;
      newList.push_back(filename);

      for (const string &tableName : m_recentTableList)
      {
         if (tableName != newList[0]) // does this file name already exist in the list?
            newList.push_back(tableName);
      }

      m_recentTableList.clear();

      int i = 0;
      for (const string &tableName : newList)
      {
         m_recentTableList.push_back(tableName);
         // write entry to the registry
         g_pvp->m_settings.SaveValue(Settings::RecentDir, "TableFileName" + std::to_string(i), tableName);

         if (++i == LAST_OPENED_TABLE_COUNT)
            break;
      }
   }

   // update the file menu to contain the last n recent loaded files
   // must be at least 1 recent file in the list
   if (!m_recentTableList.empty())
   {
#ifndef __STANDALONE__
      // update the file menu to contain the last n recent loaded files
      const CMenu menuFile = GetMainMenu(FILEMENU);

      // delete all the recent file IDM's and the separator from this menu
      for (UINT i = RECENT_FIRST_MENU_IDM; i <= RECENT_FIRST_MENU_IDM+(UINT)old_count; i++)
         menuFile.DeleteMenu(i, MF_BYCOMMAND);

      // get the number of entries in the file menu
      // insert the items before the EXIT menu (assuming it is the last entry)
      int count = menuFile.GetMenuItemCount() - 1;

      // add in the list of recently accessed files
      for (size_t i = 0; i < m_recentTableList.size(); i++)
      {
         // now search for filenames with & and replace with && so that these display correctly, and add shortcut 1..X in front
         const string recentMenuname = '&' + std::to_string(i+1) + "  " + string_replace_all(m_recentTableList[i], "&"s, "&&"s);

         // set the IDM of this menu item
         // set up the menu info block
         MENUITEMINFO menuInfo = {};
         menuInfo.cbSize = sizeof(MENUITEMINFO);
         menuInfo.fMask = MIIM_ID | MIIM_STRING | MIIM_STATE;
         menuInfo.fState = MFS_ENABLED;
         menuInfo.wID = RECENT_FIRST_MENU_IDM + (UINT)i;
         menuInfo.dwTypeData = (char*)recentMenuname.c_str();
         menuInfo.cch = (UINT)recentMenuname.length();

         menuFile.InsertMenuItem(count, menuInfo, TRUE);
         //or: menuFile.InsertMenu(count, MF_BYPOSITION | MF_ENABLED | MF_STRING, RECENT_FIRST_MENU_IDM + (UINT)i, recentMenuname);
         count++;
      }

      // add a separator onto the end
      MENUITEMINFO menuInfo = {};
      menuInfo.cbSize = sizeof(MENUITEMINFO);
      menuInfo.fMask = MIIM_ID | MIIM_TYPE | MIIM_STATE;
      menuInfo.fState = MFS_ENABLED;
      menuInfo.fType = MFT_SEPARATOR;
      menuInfo.wID = RECENT_FIRST_MENU_IDM + (UINT)m_recentTableList.size();

      menuFile.InsertMenuItem(count, menuInfo, TRUE);

      // update the menu bar
      DrawMenuBar();
#endif
   }
}

STDMETHODIMP VPinball::QueryInterface(REFIID iid, void **ppvObjOut)
{
   if (!ppvObjOut)
      return E_INVALIDARG;

   *ppvObjOut = nullptr;

   if (*ppvObjOut)
   {
      this->AddRef();
      return S_OK;
   }

   return E_NOINTERFACE;
}


STDMETHODIMP_(ULONG) VPinball::AddRef()
{
   //!! ?? ASSERT(m_cref, "bad m_cref");
   return ++m_cref;
}

STDMETHODIMP_(ULONG) VPinball::Release()
{
   assert(m_cref);
   m_cref--; // FIXME we are doing refcounting without memory management

   return m_cref;
}

void VPinball::PreCreate(CREATESTRUCT& cs)
{
   // do the base class stuff
   CWnd::PreCreate(cs);
   const int screenwidth = GetSystemMetrics(SM_CXSCREEN);  // width of primary monitor
   const int screenheight = GetSystemMetrics(SM_CYSCREEN); // height of primary monitor

   const int x = (screenwidth - MAIN_WINDOW_WIDTH) / 2;
   const int y = (screenheight - MAIN_WINDOW_HEIGHT) / 2;
   constexpr int width = MAIN_WINDOW_WIDTH;
   constexpr int height = MAIN_WINDOW_HEIGHT;

   cs.x = x; // set initial window placement
   cs.y = y;
   cs.cx = width;
   cs.cy = height;
   // specify a title bar and border with a window-menu on the title bar
   cs.style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_SIZEBOX | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
   cs.dwExStyle = WS_EX_CLIENTEDGE
              | WS_EX_CONTROLPARENT   // TAB key navigation
      //      | WS_EX_CONTEXTHELP     // doesn't work if WS_MINIMIZEBOX
                                      // or WS_MAXIMIZEBOX is specified
      ;
}

void VPinball::PreRegisterClass(WNDCLASS& wc)
{
#ifndef __STANDALONE__
   wc.hIcon = LoadIcon(theInstance, MAKEINTRESOURCE(IDI_VPINBALL));
   wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
#endif
   wc.style = CS_DBLCLKS; //CS_NOCLOSE | CS_OWNDC;
   wc.lpszClassName = _T("VPinball");
   wc.lpszMenuName = _T("IDR_APPMENU");
}

void VPinball::OnClose()
{
   // Reject close if player was not closed before
   if (g_pplayer)
      return;

   CComObject<PinTable> * const ptable = GetActiveTable();
   m_closing = true;

   while (ShowCursor(FALSE) >= 0);
   while (ShowCursor(TRUE) < 0);

   if (ptable)
      while (ptable->m_savingActive)
         Sleep(THREADS_PAUSE);

#ifndef __STANDALONE__
   const bool canClose = CanClose();
   if (canClose)
   {
      WINDOWPLACEMENT winpl;
      winpl.length = sizeof(winpl);

      if (GetWindowPlacement(winpl))
      {
         g_pvp->m_settings.SaveValue(Settings::Editor, "WindowLeft"s, (int)winpl.rcNormalPosition.left);
         g_pvp->m_settings.SaveValue(Settings::Editor, "WindowTop"s, (int)winpl.rcNormalPosition.top);
         g_pvp->m_settings.SaveValue(Settings::Editor, "WindowRight"s, (int)winpl.rcNormalPosition.right);
         g_pvp->m_settings.SaveValue(Settings::Editor, "WindowBottom"s, (int)winpl.rcNormalPosition.bottom);

         g_pvp->m_settings.SaveValue(Settings::Editor, "WindowMaximized"s, !!IsZoomed());
      }
      if (!m_open_minimized) // otherwise the window/dock settings are screwed up and have to be manually restored each time
         SaveDockRegistrySettings(DOCKER_REGISTRY_KEY);

      CWnd::OnClose();
   }
#endif
}

void VPinball::OnDestroy()
{
#ifndef __STANDALONE__
    PostMessage(WM_QUIT, 0, 0);
#endif
}

void VPinball::ShowSubDialog(CDialog &dlg, const bool show)
{
#ifndef __STANDALONE__
   if (!dlg.IsWindow())
   {
      dlg.Create(GetHwnd());
      dlg.ShowWindow(show ? SW_SHOWNORMAL : SW_HIDE);
   }
   else
      dlg.SetForegroundWindow();
#endif
}

void VPinball::SetLogicalNumberOfProcessors(int procNumber)
{
   m_logicalNumberOfProcessors = procNumber;
}

int VPinball::GetLogicalNumberOfProcessors() const
{
   if (m_logicalNumberOfProcessors < 1) {
      PLOGE << "Invalid number of processor " << m_logicalNumberOfProcessors << ". Fallback to single processor.";
      return 1;
   }

   return m_logicalNumberOfProcessors;
}

int VPinball::OnCreate(CREATESTRUCT& cs)
{
#ifndef __STANDALONE__
   // OnCreate controls the way the frame is created.
   // Overriding CFrame::OnCreate is optional.
   // Uncomment the lines below to change frame options.

   //     UseIndicatorStatus(FALSE);    // Don't show keyboard indicators in the StatusBar
   //     UseMenuStatus(FALSE);         // Don't show menu descriptions in the StatusBar
   //     UseReBar(FALSE);              // Don't use a ReBar
   //     UseStatusBar(FALSE);          // Don't use a StatusBar
   //     UseThemes(FALSE);             // Don't use themes
   UseToolBar(FALSE);            // Don't use a ToolBar

   //m_mainMenu.LoadMenu(_T("IDR_APPMENU"));

   //SetFrameMenu(m_mainMenu);

   const int result = CMDIDockFrame::OnCreate(cs);

   char szName[256];
   LoadString(theInstance, IDS_PROJNAME, szName, sizeof(szName));
   SetWindowText(szName);

   CreateDocker();

   return result;
#else
   return 0;
#endif
}

LRESULT VPinball::OnPaint(UINT msg, WPARAM wparam, LPARAM lparam)
{
#ifndef __STANDALONE__
   PAINTSTRUCT ps;
   const HDC hdc = BeginPaint(ps);
   const CRect rc = GetClientRect();
   SelectObject(hdc, GetStockObject(WHITE_BRUSH));
   PatBlt(hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, PATCOPY);
   EndPaint(ps);
#endif
   return 0;
}

void VPinball::OnInitialUpdate()
{
   wintimer_init();                    // calibrate the timer routines

   PLOGI << "OnInitialUpdate";

   InitRegValues(); // get default values from registry

#ifndef __STANDALONE__
   SetAccelerators(IDR_VPACCEL);

   m_hwndStatusBar = CreateStatusWindow(WS_CHILD | WS_VISIBLE, "", GetHwnd(), 1); // Create Status Line at the bottom

   constexpr int foo[6] = { 120, 240, 400, 600, 800, 1400 };
   ::SendMessage(m_hwndStatusBar, SB_SETPARTS, 6, (size_t)foo); // Initialize Status bar with 6 empty cells

   SendMessage(WM_SIZE, 0, 0);         // Make our window relay itself out

   int left, top, right, bottom;
   BOOL maximized;

   const bool hrleft = g_pvp->m_settings.LoadValue(Settings::Editor, "WindowLeft"s, left);
   const bool hrtop = g_pvp->m_settings.LoadValue(Settings::Editor, "WindowTop"s, top);
   const bool hrright = g_pvp->m_settings.LoadValue(Settings::Editor, "WindowRight"s, right);
   const bool hrbottom = g_pvp->m_settings.LoadValue(Settings::Editor, "WindowBottom"s, bottom);

   const bool hrmax = g_pvp->m_settings.LoadValue(Settings::Editor, "WindowMaximized"s, maximized);

   if (hrleft && hrtop && hrright && hrbottom)
   {
      WINDOWPLACEMENT winpl = {};
      winpl.length = sizeof(WINDOWPLACEMENT);

      GetWindowPlacement(winpl);

      winpl.rcNormalPosition.left = left;
      winpl.rcNormalPosition.top = top;
      winpl.rcNormalPosition.right = right;
      winpl.rcNormalPosition.bottom = bottom;

      if (m_open_minimized)
         winpl.showCmd |= SW_MINIMIZE;
      else if (hrmax == S_OK && maximized)
         winpl.showCmd |= SW_MAXIMIZE;
      else
         winpl.showCmd |= SW_SHOWNORMAL;

      SetWindowPlacement(winpl);
   }

   ShowWindow(SW_SHOW);

   // Load 'in playing mode' image for UI
   m_hbmInPlayMode = (HBITMAP)LoadImage(theInstance, MAKEINTRESOURCE(IDB_INPLAYMODE), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
#endif

   UpdateRecentFileList(string()); // update the recent loaded file list

   //   InitTools();
   //   SetForegroundWindow();
   SetEnableMenuItems();
}

BOOL VPinball::OnCommand(WPARAM wparam, LPARAM lparam)
{
#ifndef __STANDALONE__
   if (!ParseCommand(LOWORD(wparam), HIWORD(wparam) == 1))
   {
      const auto mdiTable = GetActiveMDIChild();
      if (mdiTable)
         mdiTable->SendMessage(WM_COMMAND, wparam, lparam);
      return FALSE;
   }
#endif
   return TRUE;
}

LRESULT VPinball::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
#ifndef __STANDALONE__
   switch (uMsg)
   {
   case WM_ACTIVATE:
      if (LOWORD(wParam) != WA_INACTIVE)
         GetApp()->SetAccelerators(GetFrameAccel(), GetHwnd());
      break;

   case WM_KEYUP:
   {
      if (wParam == VK_ESCAPE)
      {
         if (m_ToolCur != IDC_SELECT)
            m_ToolCur = IDC_SELECT;
         CComObject<PinTable>* const ptCur = GetActiveTable();
         if (ptCur)
            ptCur->SetMouseCursor();
      }
      return FinalWindowProc(uMsg, wParam, lParam);
   }
   case WM_TIMER:
   {
      CComObject<PinTable>* const ptCur = GetActiveTable();
      if (!ptCur)
         break;

      switch (wParam)
      {
      case TIMER_ID_AUTOSAVE:
      {
         ptCur->AutoSave();
         break;
      }

      case TIMER_ID_CLOSE_TABLE:
      {
         KillTimer(TIMER_ID_CLOSE_TABLE);
         CloseTable(ptCur);
         return 0;
      }
      }
      return FinalWindowProc(uMsg, wParam, lParam);
   }
   case WM_SIZE:
   {
      ::SendMessage(m_hwndStatusBar, WM_SIZE, 0, 0);
      break;
   }
   case WM_HELP:
   {
      ShowSubDialog(m_aboutDialog, true);
      return FinalWindowProc(uMsg, wParam, lParam);
   }

   default:
      // check for the PinSim::FrontEndControls registered message
      if (uMsg == m_pinSimFrontEndControlsMsg)
         return OnFrontEndControlsMsg(wParam, lParam);

      // not handled
      break;
   }
   return WndProcDefault(uMsg, wParam, lParam);
#else
   return 0L;
#endif
}

// See http://mjrnet.org/pinscape/PinSimFrontEndControls/PinSimFrontEndControls.htm
LRESULT VPinball::OnFrontEndControlsMsg(WPARAM wParam, LPARAM lParam)
{
#ifndef __STANDALONE__
   // the WPARAM contains a sub-command code telling us the specific action
   // the caller is requesting
   switch (wParam)
   {
   case 1:
      // IS_HANDLER_WINDOW
      // Return the current supported interface version (1)
      return 1;

   case 2:
      // CLOSE_APP
      // Close the player and exit the program
      QuitPlayer(Player::CloseState::CS_CLOSE_APP);

      // If we're showing a modal dialog, close it.  Closing one dialog
      // can trigger opening another, so iterate this until no modal
      // dialogs are found (but limit the iterations, in case we're in
      // some kind of pathological situation where the dialogs are
      // displayed in an infinite loop).
      for (int tries = 0 ; tries < 20 ; ++tries)
      {
          // enumerate the windows on this thread
          struct WindowLister
          {
              WindowLister()
              {
                  EnumThreadWindows(GetCurrentThreadId(), [](HWND hwnd, LPARAM param) -> BOOL 
                  {
                     // check for an enabled, visible model dialog window (window class "#32770")
                     char cls[128];
                     if (::IsWindowVisible(hwnd) && ::IsWindowEnabled(hwnd)
                         && ::RealGetWindowClassA(hwnd, cls, std::size(cls)) != 0
                         && strcmp(cls, "#32770") == 0)
                     {
                        // close it by sending IDCANCEL
                        DWORD_PTR result;
                        ::SendMessageTimeout(hwnd, WM_COMMAND, IDCANCEL, 0, SMTO_ABORTIFHUNG, 100, &result);

                        // note that we found at least one window to close
                        reinterpret_cast<WindowLister*>(param)->closed = true;
                     }

                     // continue the iteration
                     return TRUE;
                  }, reinterpret_cast<LPARAM>(this));
              }
              bool closed = false;
          };
          WindowLister wl;

          // if we didn't find anything to close, nothing changed during this
          // iteration, so we don't need to keep looping
          if (!wl.closed)
              break;
      }

      // handled
      return 1;

   case 3:
      // GAME_TO_FOREGROUND
      // If a player is running, bring its main window to the foreground
      if (g_pplayer != nullptr && g_pplayer->m_playfieldWnd != nullptr)
         g_pplayer->m_playfieldWnd->RaiseAndFocus(true);

      // handled
      return 1;

   case 4:
      // QUERY_GAME_HWND
      // If a player is running, return its main window handle
      return (g_pplayer != nullptr && g_pplayer->m_playfieldWnd != nullptr) ?
         reinterpret_cast<DWORD_PTR>(g_pplayer->m_playfieldWnd->GetNativeHWND()) : 0;
   }
#endif // __STANDALONE__

   // not handled
   return 0;
}

LRESULT VPinball::OnMDIActivated(UINT msg, WPARAM wparam, LPARAM lparam)
{
#ifndef __STANDALONE__
   if (m_dockNotes != nullptr)
      m_dockNotes->Refresh();
   return CMDIFrameT::OnMDIActivated(msg, wparam, lparam);
#else 
   return 0L;
#endif
}

LRESULT VPinball::OnMDIDestroyed(UINT msg, WPARAM wparam, LPARAM lparam)
{
#ifndef __STANDALONE__
   if (GetAllMDIChildren().size() == 1)
      GetLayersListDialog()->SetActiveTable(nullptr);
   return CMDIFrameT::OnMDIDestroyed(msg, wparam, lparam);
#else
   return 0L;
#endif
}

#ifndef __STANDALONE__
Win32xx::DockPtr VPinball::NewDockerFromID(int id)
{
   switch (id)
   {
   case IDD_PROPERTY_DIALOG:
      assert(m_dockProperties == nullptr);
      m_dockProperties = new CDockProperty();
      m_propertyDialog = m_dockProperties->GetContainProperties()->GetPropertyDialog();
      return DockPtr(m_dockProperties);
   case IDD_TOOLBAR:
      assert(m_dockToolbar == nullptr);
      m_dockToolbar = new CDockToolbar();
      m_toolbarDialog = m_dockToolbar->GetContainToolbar()->GetToolbarDialog();
      return DockPtr(m_dockToolbar);
   case IDD_LAYERS:
      assert(m_dockLayers == nullptr);
      m_dockLayers = new CDockLayers();
      return DockPtr(m_dockLayers);
   //   case IDD_NOTES_DIALOG:
   //   {
   //      if (m_dockNotes == nullptr)
   //      {
   //         m_dockNotes = new CDockNotes();
   //         m_notesDialog = m_dockNotes->GetContainNotes()->GetNotesDialog();
   //      }
   //      return DockPtr(m_dockNotes);
   //   }
   }
   return nullptr;
}
#endif

STDMETHODIMP VPinball::PlaySound(BSTR bstr)
{
   if (g_pplayer) g_pplayer->m_ptable->PlaySound(bstr, 0, 1.f, 0.f, 0.f, 0, VARIANT_FALSE, VARIANT_TRUE, 0.f);

   return S_OK;
}

STDMETHODIMP VPinball::FireKnocker(int Count)
{
   if (g_pplayer) g_pplayer->m_ptable->FireKnocker(Count);

   return S_OK;
}

STDMETHODIMP VPinball::QuitPlayer(int CloseType)
{
   if (g_pplayer)
   {
      g_pplayer->SetCloseState((Player::CloseState)CloseType);
   }
   else
   {
#ifndef __STANDALONE__
      PostMessage(WM_CLOSE, 0, 0);
#endif
   }

   return S_OK;
}

int CALLBACK MyCompProc(LPARAM lSortParam1, LPARAM lSortParam2, LPARAM lSortOption)
{
#ifndef __STANDALONE__
   LVFINDINFO lvf;
   const SORTDATA *lpsd = (SORTDATA *)lSortOption;

   lvf.flags = LVFI_PARAM;
   lvf.lParam = lSortParam1;
   const int nItem1 = ListView_FindItem(lpsd->hwndList, -1, &lvf);

   lvf.lParam = lSortParam2;
   const int nItem2 = ListView_FindItem(lpsd->hwndList, -1, &lvf);

   char buf1[MAX_PATH], buf2[MAX_PATH];
   ListView_GetItemText(lpsd->hwndList, nItem1, lpsd->subItemIndex, buf1, std::size(buf1));
   ListView_GetItemText(lpsd->hwndList, nItem2, lpsd->subItemIndex, buf2, std::size(buf2));

   if (nItem2 == -1 || nItem1 == -1)
      return 0;
   if (lpsd->sortUpDown == 1)
      return (lstrcmpi(buf1, buf2));
   else
      return (lstrcmpi(buf1, buf2) * -1);
#else
   return 0;
#endif
}

int CALLBACK MyCompProcIntValues(LPARAM lSortParam1, LPARAM lSortParam2, LPARAM lSortOption)
{
#ifndef __STANDALONE__
   LVFINDINFO lvf;
   const SORTDATA * const lpsd = (SORTDATA *)lSortOption;

   lvf.flags = LVFI_PARAM;
   lvf.lParam = lSortParam1;
   const int nItem1 = ListView_FindItem(lpsd->hwndList, -1, &lvf);
   lvf.lParam = lSortParam2;
   const int nItem2 = ListView_FindItem(lpsd->hwndList, -1, &lvf);

   char buf1[64] = {'\0'};
   char buf2[64] = {'\0'};
   ListView_GetItemText(lpsd->hwndList, nItem1, lpsd->subItemIndex, buf1, std::size(buf1));
   ListView_GetItemText(lpsd->hwndList, nItem2, lpsd->subItemIndex, buf2, std::size(buf2));
   int value1, value2;
   if(!try_parse_int(string(buf1), value1))
      value1 = 0;
   if(!try_parse_int(string(buf2), value2))
      value2 = 0;

   if (lpsd->sortUpDown == 1)
      return (value1 - value2);
   else
      return (value2 - value1);
#else
   return 0;
#endif
}

int CALLBACK MyCompProcMemValues(LPARAM lSortParam1, LPARAM lSortParam2, LPARAM lSortOption)
{
   const SORTDATA * const lpsd = (SORTDATA *)lSortOption;
   const Texture * const t1 = (Texture *)lSortParam1;
   const Texture * const t2 = (Texture *)lSortParam2;
   const size_t t1_size = t1->GetEstimatedGPUSize();
   const size_t t2_size = t2->GetEstimatedGPUSize();
   if (lpsd->sortUpDown == 1)
      return (int)(t1_size - t2_size);
   else
      return (int)(t2_size - t1_size);
}

static constexpr int rgDlgIDFromSecurityLevel[] = { IDC_ACTIVEX0, IDC_ACTIVEX1, IDC_ACTIVEX2, IDC_ACTIVEX3, IDC_ACTIVEX4 };

INT_PTR CALLBACK SecurityOptionsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
#ifndef __STANDALONE__
   switch (uMsg)
   {
   case WM_INITDIALOG:
   {
      const HWND hwndParent = GetParent(hwndDlg);
      RECT rcDlg;
      RECT rcMain;
      GetWindowRect(hwndParent, &rcMain);
      GetWindowRect(hwndDlg, &rcDlg);

      SetWindowPos(hwndDlg, nullptr,
         (rcMain.right + rcMain.left) / 2 - (rcDlg.right - rcDlg.left) / 2,
         (rcMain.bottom + rcMain.top) / 2 - (rcDlg.bottom - rcDlg.top) / 2,
         0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE/* | SWP_NOMOVE*/);

      int security = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "SecurityLevel"s, DEFAULT_SECURITY_LEVEL);
      if (security < 0 || security > 4)
         security = 0;

      const int buttonid = rgDlgIDFromSecurityLevel[security];

      SendMessage(GetDlgItem(hwndDlg, buttonid), BM_SETCHECK, BST_CHECKED, 0);

      const bool hangdetect = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "DetectHang"s, false);
      SendMessage(GetDlgItem(hwndDlg, IDC_HANGDETECT), BM_SETCHECK, hangdetect ? BST_CHECKED : BST_UNCHECKED, 0);

      return TRUE;
   }
   break;

   case WM_COMMAND:
   {
      switch (HIWORD(wParam))
      {
      case BN_CLICKED:
         switch (LOWORD(wParam))
         {
         case IDOK:
         {
            for (int i = 0; i < 5; i++)
            {
               const size_t checked = SendMessage(GetDlgItem(hwndDlg, rgDlgIDFromSecurityLevel[i]), BM_GETCHECK, 0, 0);
               if (checked == BST_CHECKED)
                  g_pvp->m_settings.SaveValue(Settings::Player, "SecurityLevel"s, i);
            }

            const bool hangdetect = (SendMessage(GetDlgItem(hwndDlg, IDC_HANGDETECT), BM_GETCHECK, 0, 0) != 0);
            g_pvp->m_settings.SaveValue(Settings::Player, "DetectHang"s, hangdetect);

            EndDialog(hwndDlg, TRUE);
         }
         break;

         case IDCANCEL:
            EndDialog(hwndDlg, FALSE);
            break;
         }
      }
   }
   break;

   case WM_CLOSE:
      EndDialog(hwndDlg, FALSE);
      break;
   }
#endif

   return FALSE;
}

INT_PTR CALLBACK FontManagerProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
#ifndef __STANDALONE__
   CCO(PinTable) *pt = (CCO(PinTable) *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

   switch (uMsg)
   {
   case WM_INITDIALOG:
   {
      SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

      LVCOLUMN lvcol = {};
      lvcol.mask = LVCF_TEXT | LVCF_WIDTH;
      const LocalString ls(IDS_NAME);
      lvcol.pszText = (LPSTR)ls.m_szbuffer; // = "Name";
      lvcol.cx = 100;
      ListView_InsertColumn(GetDlgItem(hwndDlg, IDC_SOUNDLIST), 0, &lvcol);

      const LocalString ls2(IDS_IMPORTPATH);
      lvcol.pszText = (LPSTR)ls2.m_szbuffer; // = "Import Path";
      lvcol.cx = 200;
      ListView_InsertColumn(GetDlgItem(hwndDlg, IDC_SOUNDLIST), 1, &lvcol);

      pt = (CCO(PinTable) *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

      pt->ListFonts(GetDlgItem(hwndDlg, IDC_SOUNDLIST));

      return TRUE;
   }
   break;

   case WM_CLOSE:
      EndDialog(hwndDlg, FALSE);
      break;

   case WM_COMMAND:
      switch (HIWORD(wParam))
      {
      case BN_CLICKED:
         switch (LOWORD(wParam))
         {
         case IDOK:
            EndDialog(hwndDlg, TRUE);
            break;

         case IDCANCEL:
            EndDialog(hwndDlg, FALSE);
            break;

         case IDC_IMPORT:
         {
            string szInitialDir = g_pvp->m_settings.LoadValueWithDefault(Settings::RecentDir, "FontDir"s, PATH_TABLES);

            vector<string> filename;
            if (g_pvp->OpenFileDialog(szInitialDir, filename, "Font Files (*.ttf)\0*.ttf\0", "ttf", 0))
            {
               const size_t index = filename[0].find_last_of(PATH_SEPARATOR_CHAR);
               if (index != string::npos)
                  g_pvp->m_settings.SaveValue(Settings::RecentDir, "FontDir"s, filename[0].substr(0, index));

               pt->ImportFont(GetDlgItem(hwndDlg, IDC_SOUNDLIST), filename[0]);
            }
         }
         break;

         /*case IDC_RENAME:
         {
         int sel = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), -1, LVNI_SELECTED);
         if (sel != -1)
         {
         SetFocus(GetDlgItem(hwndDlg, IDC_SOUNDLIST));
         HWND hwndFoo = ListView_EditLabel(GetDlgItem(hwndDlg, IDC_SOUNDLIST), sel);
         }
         }
         break;*/

         case IDC_DELETE:
         {
            const int sel = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), -1, LVNI_SELECTED);
            if (sel != -1)
            {
               // TEXT
               const int ans = MessageBox(hwndDlg, "Are you sure you want to remove this font?", "Confirm Deletion", MB_YESNO | MB_DEFBUTTON2);
               if (ans == IDYES)
               {
                  LVITEM lvitem;
                  lvitem.mask = LVIF_PARAM;
                  lvitem.iItem = sel;
                  lvitem.iSubItem = 0;
                  ListView_GetItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), &lvitem);
                  PinFont * const ppf = (PinFont *)lvitem.lParam;
                  ListView_DeleteItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), sel);
                  pt->RemoveFont(ppf);
               }
            }
         }
         break;
         }
         break;
      }
      break;
   }
#endif

   return FALSE;
}


void VPinball::ShowDrawingOrderDialog(bool select)
{
#ifndef __STANDALONE__
   DrawingOrderDialog orderDlg(select);
   orderDlg.DoModal();
#endif
}

void VPinball::CloseAllDialogs()
{
#ifndef __STANDALONE__
   if (m_imageMngDlg.IsWindow())
      m_imageMngDlg.Destroy();
   if (m_soundMngDlg.IsWindow())
      m_soundMngDlg.Destroy();
   if (m_audioOptDialog.IsWindow())
      m_audioOptDialog.Destroy();
   if (m_editorOptDialog.IsWindow())
      m_editorOptDialog.Destroy();
   if (m_collectionMngDlg.IsWindow())
      m_collectionMngDlg.Destroy();
   if (m_physicsOptDialog.IsWindow())
      m_physicsOptDialog.Destroy();
   if (m_tableInfoDialog.IsWindow())
      m_tableInfoDialog.Destroy();
   if (m_dimensionDialog.IsWindow())
      m_dimensionDialog.Destroy();
   if (m_renderProbeDialog.IsWindow())
      m_renderProbeDialog.Destroy();
   if (m_materialDialog.IsWindow())
      m_materialDialog.Destroy();
   if (m_aboutDialog.IsWindow())
      m_aboutDialog.Destroy();
   #if defined(ENABLE_VR) || defined(ENABLE_XR)
      if (m_vrOptDialog.IsWindow())
         m_vrOptDialog.Destroy();
   #endif
#endif
}

void VPinball::ToggleBackglassView()
{
   const bool show = !m_backglassView;
   m_backglassView = show;

   for (size_t i = 0; i < m_vtable.size(); i++)
   {
      PinTable * const ptT = m_vtable[i];
      ptT->SetDefaultView();
      ptT->SetDirtyDraw();
   }

   CComObject<PinTable> * const ptCur = GetActiveTable();
   if (ptCur)
      // Set selection to something in the new view (unless hiding table elements)
      ptCur->AddMultiSel((ISelect *)ptCur, false, true, false);

   ToggleToolbar();
}

void VPinball::ToggleScriptEditor()
{
   CComObject<PinTable> * const ptCur = GetActiveTable();
   if (ptCur)
   {
#ifndef __STANDALONE__
      const bool alwaysViewScript = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "AlwaysViewScript"s, false);

      ptCur->m_pcv->SetVisible(alwaysViewScript || !(ptCur->m_pcv->m_visible && !ptCur->m_pcv->m_minimized));
#endif

      //SendMessage(m_hwndToolbarMain, TB_CHECKBUTTON, ID_EDIT_SCRIPT, MAKELONG(ptCur->m_pcv->m_visible && !ptCur->m_pcv->m_minimized, 0));
   }
}

void VPinball::ShowSearchSelect()
{
   CComObject<PinTable> * const ptCur = GetActiveTable();
   if (ptCur)
   {
#ifndef __STANDALONE__
      if (!ptCur->m_searchSelectDlg.IsWindow())
      {
         ptCur->m_searchSelectDlg.Create(GetHwnd());

         const string windowName = "Search/Select Element - " + ptCur->m_filename;
         ptCur->m_searchSelectDlg.SetWindowText(windowName.c_str());

         ptCur->m_searchSelectDlg.ShowWindow();
      }
      else
      {
         ptCur->m_searchSelectDlg.ShowWindow();
         ptCur->m_searchSelectDlg.SetForegroundWindow();
      }
#endif
   }
}

void VPinball::SetDefaultPhysics()
{
   CComObject<PinTable> * const ptCur = GetActiveTable();
   if (ptCur)
   {
      const LocalString ls(IDS_DEFAULTPHYSICS);
      const int answ = MessageBox(ls.m_szbuffer, "Continue?", MB_YESNO | MB_ICONWARNING);
      if (answ == IDYES)
      {
         ptCur->BeginUndo();
         for (int i = 0; i < ptCur->m_vmultisel.size(); i++)
            ptCur->m_vmultisel[i].SetDefaultPhysics(true);
         ptCur->EndUndo();
      }
   }
}

void VPinball::SetViewSolidOutline(size_t viewId)
{
   CComObject<PinTable> * const ptCur = GetActiveTable();
   if (ptCur)
   {
#ifndef __STANDALONE__
      ptCur->m_renderSolid = (viewId == ID_VIEW_SOLID);
      GetMenu().CheckMenuItem(ID_VIEW_SOLID, MF_BYCOMMAND | (ptCur->RenderSolid() ? MF_CHECKED : MF_UNCHECKED));
      GetMenu().CheckMenuItem(ID_VIEW_OUTLINE, MF_BYCOMMAND | (ptCur->RenderSolid() ? MF_UNCHECKED : MF_CHECKED));

      ptCur->SetDirtyDraw();
      g_pvp->m_settings.SaveValue(Settings::Editor, "RenderSolid"s, ptCur->m_renderSolid);
#endif
   }
}

void VPinball::ShowGridView()
{
   CComObject<PinTable> * const ptCur = GetActiveTable();
   if (ptCur)
   {
#ifndef __STANDALONE__
      ptCur->put_DisplayGrid(FTOVB(!ptCur->m_grid));
      GetMenu().CheckMenuItem(ID_VIEW_GRID, MF_BYCOMMAND | (ptCur->m_grid ? MF_CHECKED : MF_UNCHECKED));
#endif
   }
}

void VPinball::ShowBackdropView()
{
   CComObject<PinTable> * const ptCur = GetActiveTable();
   if (ptCur)
   {
#ifndef __STANDALONE__
      ptCur->put_DisplayBackdrop(FTOVB(!ptCur->m_backdrop));
      GetMenu().CheckMenuItem(ID_VIEW_BACKDROP, MF_BYCOMMAND | (ptCur->m_backdrop ? MF_CHECKED : MF_UNCHECKED));
#endif
   }
}

void VPinball::AddControlPoint()
{
   CComObject<PinTable> * const ptCur = GetActiveTable();
   if (ptCur == nullptr)
      return;

   if (!ptCur->m_vmultisel.empty())
   {
      ISelect * const psel = ptCur->m_vmultisel.ElementAt(0);
      if (psel != nullptr)
      {
         const POINT pt = ptCur->GetScreenPoint();
         switch (psel->GetItemType())
         {
         case eItemRamp:
         {
            Ramp * const pRamp = (Ramp *)psel;
            pRamp->AddPoint(pt.x, pt.y, false);
            break;
         }
         case eItemLight:
         {
            Light * const pLight = (Light *)psel;
            pLight->AddPoint(pt.x, pt.y, false);
            break;
         }
         case eItemSurface:
         {
            Surface * const pSurf = (Surface *)psel;
            pSurf->AddPoint(pt.x, pt.y, false);
            break;
         }
         case eItemRubber:
         {
            Rubber * const pRub = (Rubber *)psel;
            pRub->AddPoint(pt.x, pt.y, false);
            break;
         }
         default:
            break;
         }
      } //if (psel != nullptr)
   }
}

void VPinball::AddSmoothControlPoint()
{
   CComObject<PinTable> * const ptCur = GetActiveTable();
   if (ptCur == nullptr)
      return;

   if (!ptCur->m_vmultisel.empty())
   {
      ISelect * const psel = ptCur->m_vmultisel.ElementAt(0);
      if (psel != nullptr)
      {
         const POINT pt = ptCur->GetScreenPoint();
         switch (psel->GetItemType())
         {
         case eItemRamp:
         {
            Ramp * const pRamp = (Ramp *)psel;
            pRamp->AddPoint(pt.x, pt.y, true);
            break;
         }
         case eItemLight:
         {
            Light * const pLight = (Light *)psel;
            pLight->AddPoint(pt.x, pt.y, true);
            break;
         }
         case eItemSurface:
         {
            Surface * const pSurf = (Surface *)psel;
            pSurf->AddPoint(pt.x, pt.y, true);
            break;
         }
         case eItemRubber:
         {
            Rubber * const pRub = (Rubber *)psel;
            pRub->AddPoint(pt.x, pt.y, true);
            break;
         }
         default:
            break;
         }
      }
   }
}

void VPinball::SaveTable(const bool saveAs)
{
   CComObject<PinTable> * const ptCur = GetActiveTable();
   if (ptCur)
   {
      HRESULT hr;
      if (saveAs)
         hr = ptCur->SaveAs();
      else
         hr = ptCur->TableSave();

      if (hr == S_OK)
         UpdateRecentFileList(ptCur->m_filename);
   }
}

void VPinball::OpenNewTable(size_t tableId)
{
   if (m_vtable.size() == MAX_OPEN_TABLES)
   {
      ShowError("Maximum amount of tables already loaded and open.");
      return;
   }

#ifndef __STANDALONE__
   PinTableMDI * const mdiTable = new PinTableMDI(this);
   CComObject<PinTable>* const ppt = mdiTable->GetTable();
   m_vtable.push_back(ppt);
   ppt->InitBuiltinTable(tableId);
   ppt->InitTablePostLoad();

   AddMDITable(mdiTable);
   mdiTable->GetTable()->AddMultiSel(mdiTable->GetTable(), false, true, false);
   GetLayersListDialog()->ResetView();
   ToggleToolbar();
   if (m_dockNotes != nullptr)
      m_dockNotes->Enable();

   SetFocus();
#endif
}

void VPinball::ProcessDeleteElement()
{
   CComObject<PinTable> * const ptCur = GetActiveTable();
   if (ptCur)
      ptCur->OnDelete();
}

void VPinball::OpenRecentFile(const size_t menuId)
{
   // get the index into the recent list menu
   const size_t Index = menuId - RECENT_FIRST_MENU_IDM;
   // copy it into a temporary string so it can be correctly processed
   LoadFileName(m_recentTableList[Index], true);
}

void VPinball::CopyPasteElement(const CopyPasteModes mode)
{
   CComObject<PinTable> * const ptCur = GetActiveTable();
   if (ptCur && !ptCur->IsLocked())
   {
      const POINT ptCursor = ptCur->GetScreenPoint();
      switch (mode)
      {
      case COPY:
      {
         ptCur->Copy(ptCursor.x, ptCursor.y);
         break;
      }
      case PASTE:
      {
         ptCur->Paste(false, ptCursor.x, ptCursor.y);
         break;
      }
      case PASTE_AT:
      {
         ptCur->Paste(true, ptCursor.x, ptCursor.y);
         break;
      }
      default:
         break;
      }
   }
}

//

static inline string GetTextFileFromDirectory(const string& filename, const string& dirname)
{
   string szPath;
   if (!dirname.empty())
      szPath = g_pvp->m_myPath + dirname;
   // else: use current directory
   return szPath + filename;
}

static constexpr uint8_t lookupRev[16] = {
0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe,
0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf };

static inline uint8_t reverse(const uint8_t n)
{
   return (lookupRev[n & 0x0f] << 4) | lookupRev[n >> 4];
}

static unsigned int GenerateTournamentFileInternal(uint8_t *const dmd_data, const unsigned int dmd_size, const string& tablefile, unsigned int& tablefileChecksum, unsigned int& vpxChecksum, unsigned int& scriptsChecksum)
{
   tablefileChecksum = vpxChecksum = scriptsChecksum = 0;
   unsigned int dmd_data_c = 0;

   FILE *f;
   if (fopen_s(&f, tablefile.c_str(), "rb") == 0 && f)
   {
      uint8_t tmp[4096];
      size_t r;
      while ((r = fread(tmp, 1, sizeof(tmp), f))) //!! also include MD5 at end?
      {
         for (unsigned int i = 0; i < r; ++i)
         {
            dmd_data[dmd_data_c++] ^= reverse(tmp[i]);
            if (dmd_data_c == dmd_size)
               dmd_data_c = 0;
         }

         uint32_t md5[4];
         generateMD5(tmp, r, (uint8_t*)md5);
         for (unsigned int i = 0; i < 4; ++i)
            tablefileChecksum ^= md5[i];
      }
      fclose(f);
   }
   else
   {
      if (g_pplayer && g_pplayer->m_liveUI)
         g_pplayer->m_liveUI->PushNotification("Cannot open Table"s, 4000);
      else
         ShowError("Cannot open Table");
      return ~0u;
   }

   //

#ifndef __STANDALONE__
   const size_t cchar = SendMessage(g_pvp->GetActiveTable()->m_pcv->m_hwndScintilla, SCI_GETTEXTLENGTH, 0, 0);
   char * const szText = new char[cchar + 1];
   SendMessage(g_pvp->GetActiveTable()->m_pcv->m_hwndScintilla, SCI_GETTEXT, cchar + 1, (size_t)szText);
#else
   const size_t cchar = g_pvp->GetActiveTable()->m_pcv->m_script_text.length();
   char * const szText = new char[cchar + 1];
   strncpy_s(szText, g_pvp->GetActiveTable()->m_pcv->m_script_text.c_str(), cchar);
#endif

   for(size_t i = 0; i < cchar; ++i)
      szText[i] = cLower(szText[i]);

   const char* textPos = szText;
   const char* const textEnd = szText+cchar;
   vector<string> vbsFiles;
   while(textPos < textEnd)
   {
      const char* const textFound = StrStrI(textPos,".vbs\"");
      if(textFound == nullptr)
         break;
      textPos = textFound+1;

      const char* textFoundStart = textFound;
      while(*textFoundStart != '"')
         --textFoundStart;

      vbsFiles.emplace_back(textFoundStart+1,textFound+3-textFoundStart);
   }

   delete [] szText;

   vbsFiles.push_back("core.vbs"s);

   for(const auto& i3 : vbsFiles)
      for(size_t i2 = 0; i2 < std::size(defaultFileNameSearch); ++i2)
         if(fopen_s(&f, GetTextFileFromDirectory(defaultFileNameSearch[i2] + i3, defaultPathSearch[i2]).c_str(), "rb") == 0 && f)
         {
            uint8_t tmp[4096];
            size_t r;
            while ((r = fread(tmp, 1, sizeof(tmp), f))) //!! also include MD5 at end?
            {
               for (unsigned int i = 0; i < r; ++i)
               {
                  dmd_data[dmd_data_c++] ^= reverse(tmp[i]);
                  if (dmd_data_c == dmd_size)
                     dmd_data_c = 0;
               }

               uint32_t md5[4];
               generateMD5(tmp, r, (uint8_t *)md5);
               for (unsigned int i = 0; i < 4; ++i)
                  scriptsChecksum ^= md5[i];
            }
            fclose(f);

            break;
         }

   //

   char path[MAXSTRING];
#ifdef _MSC_VER
   GetModuleFileName(nullptr, path, MAXSTRING);
#elif defined(__APPLE__) //!! ??
   const char* szPath = SDL_GetBasePath();
   strcpy_s(path, sizeof(path), szPath);
#else
   const ssize_t len = ::readlink("/proc/self/exe", path, sizeof(path)-1);
   if (len != -1)
      path[len] = '\0';
#endif
   if(fopen_s(&f, path, "rb") == 0 && f)
   {
      uint8_t tmp[4096];
      size_t r;
      while ((r = fread(tmp, 1, sizeof(tmp), f))) //!! also include MD5 at end?
      {
         for (unsigned int i = 0; i < r; ++i)
         {
            dmd_data[dmd_data_c++] ^= reverse(tmp[i]);
            if (dmd_data_c == dmd_size)
               dmd_data_c = 0;
         }

         uint32_t md5[4];
         generateMD5(tmp, r, (uint8_t*)md5);
         for (unsigned int i = 0; i < 4; ++i)
            vpxChecksum ^= md5[i];
      }
      fclose(f);
   }
   else
   {
      if (g_pplayer && g_pplayer->m_liveUI)
         g_pplayer->m_liveUI->PushNotification("Cannot open Executable"s, 4000);
      else
         ShowError("Cannot open Executable");
      return ~0u;
   }

#if defined(_M_IX86) || defined(_M_X64) || defined(_M_AMD64) || defined(__i386__) || defined(__i386) || defined(__i486__) || defined(__i486) || defined(i386) || defined(__ia64__) || defined(__x86_64__)
   _mm_sfence();
#else // for now arm only
   __atomic_thread_fence(__ATOMIC_SEQ_CST);
#endif

   return dmd_data_c;
}

static void GenerateTournamentFileInternal2(uint8_t *const dmd_data, const unsigned int dmd_size, unsigned int dmd_data_c)
{
   uint8_t *fs = (uint8_t*)&GenerateTournamentFileInternal;
   uint8_t *fe = fs;
   while (true)
   {
#if defined(_M_IX86) || defined(_M_X64) || defined(_M_AMD64) || defined(__i386__) || defined(__i386) || defined(__i486__) || defined(__i486) || defined(i386) || defined(__ia64__) || defined(__x86_64__)
      if (fe[0] == 0x0F && fe[1] == 0xAE && fe[2] == 0xF8)
#else // for now arm only
      if (fe[0] == 0xD5 && fe[1] == 0x03 && fe[2] == 0x3B && fe[3] == 0xBF)
#endif
         break;
      fe++;
   } //!! also include MD5 ?

   while (fs < fe)
   {
      dmd_data[dmd_data_c++] ^= reverse(*fs);
      fs++;
      if (dmd_data_c == dmd_size)
         dmd_data_c = 0;
   }
}

void VPinball::GenerateTournamentFile()
{
   unsigned int dmd_size = g_pplayer->m_dmdSize.x * g_pplayer->m_dmdSize.y;
   if (dmd_size == 0)
   {
      g_pplayer->m_liveUI->PushNotification("Tournament file export requires a valid DMD script connection to PinMAME via 'UseVPM(Colored)DMD = True'"s, 4000);
      return;
   }

   uint8_t *const dmd_data = new uint8_t[dmd_size + 16];
   if (g_pplayer->m_dmdFrame->m_format == BaseTexture::BW)
      memcpy(dmd_data, g_pplayer->m_dmdFrame->data(), dmd_size);
   else if (g_pplayer->m_dmdFrame->m_format == BaseTexture::RGBA)
   {
      const uint32_t *const __restrict data = (uint32_t *)g_pplayer->m_dmdFrame->data();
      for (unsigned int i = 0; i < dmd_size; ++i)
         dmd_data[i] = ((data[i] & 0xFF) + ((data[i] >> 8) & 0xFF) + ((data[i] >> 16) & 0xFF))/3;
   }
   generateMD5(dmd_data, dmd_size, dmd_data + dmd_size);
   dmd_size += 16;
   unsigned int tablefileChecksum, vpxChecksum, scriptsChecksum;
   const unsigned int res = GenerateTournamentFileInternal(dmd_data, dmd_size, GetActiveTable()->m_filename, tablefileChecksum, vpxChecksum, scriptsChecksum);
   if (res == ~0u)
      return;
   GenerateTournamentFileInternal2(dmd_data, dmd_size, res);

   FILE *f;
   if (fopen_s(&f, (g_pvp->GetActiveTable()->m_filename + ".txt").c_str(), "w") == 0 && f)
   {
      fprintf(f, "%03X", g_pplayer->m_dmdSize.x);
      fprintf(f, "%03X", g_pplayer->m_dmdSize.y);
      fprintf(f, "%01X", GET_PLATFORM_CPU_ENUM);
      fprintf(f, "%01X", GET_PLATFORM_BITS_ENUM);
      fprintf(f, "%01X", GET_PLATFORM_OS_ENUM);
      fprintf(f, "%01X", GET_PLATFORM_RENDERER_ENUM);
      fprintf(f, "%01X", VP_VERSION_MAJOR);
      fprintf(f, "%01X", VP_VERSION_MINOR);
      fprintf(f, "%01X", VP_VERSION_REV);
      fprintf(f, "%04X", GIT_REVISION);
      fprintf(f, "%08X", tablefileChecksum);
      fprintf(f, "%08X", vpxChecksum);
      fprintf(f, "%08X", scriptsChecksum);
      for (unsigned int i = 0; i < dmd_size; ++i)
         fprintf(f,"%02X",dmd_data[i]);
      fclose(f);

      g_pplayer->m_liveUI->PushNotification("Tournament file saved as " + g_pvp->GetActiveTable()->m_filename + ".txt", 4000);
   }
   else
      g_pplayer->m_liveUI->PushNotification("Cannot save Tournament file"s, 4000);

   delete[] dmd_data;
}

void VPinball::GenerateImageFromTournamentFile(const string &tablefile, const string &txtfile)
{
   unsigned int x = 0, y = 0, dmd_size = 0, cpu = 0, bits = 0, os = 0, renderer = 0, major = 0, minor = 0, rev = 0, git_rev = 0;
   unsigned int tablefileChecksum_in = 0, vpxChecksum_in = 0, scriptsChecksum_in = 0;
   vector<uint8_t> dmd_data;
   FILE *f;
   if (fopen_s(&f, txtfile.c_str(), "r") == 0 && f)
   {
      bool error = false;
      error |= fscanf_s(f, "%03X", &x) != 1;
      error |= fscanf_s(f, "%03X", &y) != 1;
      error |= fscanf_s(f, "%01X", &cpu) != 1;
      error |= fscanf_s(f, "%01X", &bits) != 1;
      error |= fscanf_s(f, "%01X", &os) != 1;
      error |= fscanf_s(f, "%01X", &renderer) != 1;
      error |= fscanf_s(f, "%01X", &major) != 1;
      error |= fscanf_s(f, "%01X", &minor) != 1;
      error |= fscanf_s(f, "%01X", &rev) != 1;
      error |= fscanf_s(f, "%04X", &git_rev) != 1;
      error |= fscanf_s(f, "%08X", &tablefileChecksum_in) != 1;
      error |= fscanf_s(f, "%08X", &vpxChecksum_in) != 1;
      error |= fscanf_s(f, "%08X", &scriptsChecksum_in) != 1;
      dmd_size = x * y + 16;
      dmd_data.resize(dmd_size);
      for (unsigned int i = 0; i < dmd_size; ++i)
      {
         unsigned int v;
         error |= fscanf_s(f, "%02X", &v) != 1;
         dmd_data[i] = v;
      }
      fclose(f);

      if (error)
      {
         ShowError("Error parsing Tournament file");
         return;
      }
   }
   else
   {
      ShowError("Cannot open Tournament file");
      return;
   }

   if (cpu != GET_PLATFORM_CPU_ENUM || bits != GET_PLATFORM_BITS_ENUM || os != GET_PLATFORM_OS_ENUM || renderer != GET_PLATFORM_RENDERER_ENUM)
   {
      ShowError("Cannot decode Tournament file\nas the setup differs:\nEncoder: " + platform_cpu[cpu] + ' ' + platform_bits[bits] + "bits " + platform_os[os] + ' ' + platform_renderer[renderer] + "\nDecoder: "+ platform_cpu[GET_PLATFORM_CPU_ENUM] + ' ' + platform_bits[GET_PLATFORM_BITS_ENUM] + "bits " + platform_os[GET_PLATFORM_OS_ENUM] + ' ' + platform_renderer[GET_PLATFORM_RENDERER_ENUM]);
      return;
   }

   if (major != VP_VERSION_MAJOR || minor != VP_VERSION_MINOR || rev != VP_VERSION_REV || git_rev != GIT_REVISION)
   {
      ShowError("Cannot decode Tournament file\nas the VP version differs:\nEncoder: " + std::to_string(major) + '.' + std::to_string(minor) + '.' + std::to_string(rev) + " rev. " + std::to_string(git_rev) + "\nDecoder: "+ std::to_string(VP_VERSION_MAJOR) + '.' + std::to_string(VP_VERSION_MINOR) + '.' + std::to_string(VP_VERSION_REV) + " rev. " + std::to_string(GIT_REVISION));
      return;
   }

   unsigned int tablefileChecksum, vpxChecksum, scriptsChecksum;
   const unsigned int res = GenerateTournamentFileInternal(dmd_data.data(), dmd_size, tablefile, tablefileChecksum, vpxChecksum, scriptsChecksum);
   if (res == ~0u)
   {
      return;
   }
   if (tablefileChecksum != tablefileChecksum_in)
   {
      ShowError("Cannot decode Tournament file\nas the table version differs"s);
      return;
   }
   if (vpxChecksum != vpxChecksum_in)
   {
      ShowError("Cannot decode Tournament file\nas VP was modified"s);
      return;
   }
   if (scriptsChecksum != scriptsChecksum_in)
   {
      ShowError("Cannot decode Tournament file\nas scripts version differs"s);
      return;
   }
   GenerateTournamentFileInternal2(dmd_data.data(), dmd_size, res);
   uint8_t md5[16];
   generateMD5(dmd_data.data(), dmd_size - 16, md5);
   if (memcmp(dmd_data.data() + (dmd_size - 16), md5, 16) != 0)
   {
      ShowError("Corrupt Tournament file or non-matching table-version or modified VP used to encode");
      return;
   }

   FIBITMAP *dib = FreeImage_Allocate(x, y, 8);
   BYTE *const pdst = FreeImage_GetBits(dib);
   //const unsigned int pitch_dst = FreeImage_GetPitch(dib); //!! needed?
   for (unsigned int j = 0; j < y; j++)
      for (unsigned int i = 0; i < x; i++)
         pdst[i + (y-1-j)*x] = dmd_data[i+j*x]; // flip y-axis for image output
   if (!FreeImage_Save(FIF_PNG, dib, (txtfile + ".png").c_str(), PNG_Z_BEST_COMPRESSION))
      ShowError("Tournament file converted image could not be saved");
   FreeImage_Unload(dib);
}
