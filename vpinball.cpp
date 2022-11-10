// VPinball.cpp: implementation of the VPinball class.
//
////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "KeysConfigDialog.h"

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

static volatile bool firstRun = true;

///<summary>
///VPinball Constructor
///<para>Init</para>
///</summary>
VPinball::VPinball()
{
   if ((fopen_s(&m_profile_file, "Profile.txt", "r") == 0) && m_profile_file)
   {
      fclose(m_profile_file);
      if ((fopen_s(&m_profile_file, "Profile.txt", "a") == 0) && m_profile_file)
      {
         const time_t curr = time(0);
         char buf[26];
         ctime_s(buf,sizeof(buf),&curr);
         fprintf(m_profile_file, "\nTrace from %s\n", buf);
      }
   }

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
   for (unsigned int i = 0; i < MAX_CUSTOM_PARAM_INDEX; ++i)
      m_customParameters[i] = nullptr;

   m_mouseCursorPosition.x = 0.0f;
   m_mouseCursorPosition.y = 0.0f;
   m_pcv = nullptr;			// no currently active code window

   m_NextTableID = 1;

   m_ptableActive = nullptr;

   m_workerthread = nullptr;//Workerthread - only for hanging scripts and autosave - will be created later

   m_ToolCur = IDC_SELECT;

   m_hbmInPlayMode = nullptr;

   GetMyPath();				//Store path of vpinball.exe in m_szMyPath and m_wzMyPath

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
}

///<summary>
///VPinball Destructor
///<para>deletes clipboard</para>
///<para>Releases Resources for Script editor</para>
///</summary>
VPinball::~VPinball()
{
   // DLL_API void DLL_CALLCONV FreeImage_DeInitialise(); // would only be needed if linking statically
   SetClipboard(nullptr);
   FreeLibrary(m_scintillaDll);

   if (m_profile_file)
       fclose(m_profile_file);
}

///<summary>
///Store path of exe (without the exe's filename) in Class Variable
///<para>Stores path as string in m_szMyPath (8 bit ansi)</para>
///<para>Stores path as wstring in m_wzMyPath (16 bit Unicode)</para>
///</summary>
void VPinball::GetMyPath()
{
   char szPath[MAXSTRING];
#ifdef _MSC_VER
   GetModuleFileName(nullptr, szPath, MAXSTRING);
#else
#ifdef __APPLE__
   uint32_t pathSize = (uint32_t)sizeof(szPath);
   _NSGetExecutablePath(szPath, &pathSize);
#else
   readlink( "/proc/self/exe", szPath, MAXSTRING );
#endif
#endif
   char *szEnd = szPath + lstrlen(szPath);

   // search for first backslash
   while (szEnd > szPath)
   {
      if (*szEnd == PATH_SEPARATOR_CHAR)
         break;
      szEnd--;
   }

   // truncate the filename
   *(szEnd + 1) = '\0'; // Get rid of exe name

   // store 2x
   m_szMyPath = szPath;
   WCHAR wzPath[MAXSTRING];
   MultiByteToWideCharNull(CP_ACP, 0, szPath, -1, wzPath, MAXSTRING);
   m_wzMyPath = wzPath;
}

///<summary>
///Ensure that worker thread exists
///<para>Starts worker Thread otherwise</para>
///</summary>
void VPinball::EnsureWorkerThread()
{
   if (!m_workerthread)
   {
      g_hWorkerStarted = CreateEvent(nullptr, TRUE, FALSE, nullptr);
      m_workerthread = (HANDLE)_beginthreadex(nullptr, 0, VPWorkerThreadStart, 0, 0, &m_workerthreadid);
      if (WaitForSingleObject(g_hWorkerStarted, 5000) == WAIT_TIMEOUT)
      {
      }
      SetThreadPriority(m_workerthread, THREAD_PRIORITY_LOWEST);
   }
}

///<summary>
///Post Work to the worker Thread
///<para>Creates Worker-Thread if not present</para>
///<para>See Worker::VPWorkerThreadStart for infos</para>
///<param name="workid">int for the type of message (COMPLETE_AUTOSAVE | HANG_SNOOP_START | HANG_SNOOP_STOP)</param>
///<param name="lParam">Second Parameter for message (AutoSavePackage (see worker.h) if COMPLETE_AUTOSAVE, otherwise nullptr)</param>
///<returns>Handle to Event that get ack. If event is finished (unsure)</returns>
///</summary>
HANDLE VPinball::PostWorkToWorkerThread(int workid, LPARAM lParam)
{
   EnsureWorkerThread();										// Check if Workerthread was created once, otherwise create

   HANDLE hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

   PostThreadMessage(m_workerthreadid, workid, (WPARAM)hEvent, lParam);

   return hEvent;
}

///<summary>
///Sets m_autosavetime
///<param name="minutes">int Minutes between autosave</param>
///</summary>
void VPinball::SetAutoSaveMinutes(const int minutes)
{
   m_autosaveTime = (minutes <= 0) ? -1 : minutes * (60 * 1000); // convert to milliseconds
}

///<summary>
///Post Work to the worker Thread
///<para>Creates Worker-Thread if not present</para>
///<para>See Worker::VPWorkerThreadStart for infos</para>
///<param name="workid">int for the type of message (COMPLETE_AUTOSAVE | HANG_SNOOP_START | HANG_SNOOP_STOP)</param>
///<param name="lparam">Second Parameter for message (AutoSavePackage (see worker.h) if COMPLETE_AUTOSAVE, otherwise nullptr)</param>
///<returns>Handle to Event that get ack. If event is finished (unsure)</returns>
///</summary>
void VPinball::InitTools()
{
   m_ToolCur = IDC_SELECT;
}

///<summary>
///Initializes Default Values of many variables (from Registry if keys are present).
///<para>Registry Values under HKEY-CURRENT-USER/Software/Visual Pinball</para>
///<para>Deadzone, ShowDragPoints, DrawLightCenters,</para>
///<para>AutoSaveOn, AutoSaveTime, SecurityLevel</para>
///<para>Gets the last loaded Tables (List under File-Menu)</para>
///</summary>
void VPinball::InitRegValues()
{
   const int deadz = LoadValueIntWithDefault(regKey[RegName::Player], "DeadZone"s, 0);
   SaveValueInt(regKey[RegName::Player], "DeadZone"s, deadz);

   m_alwaysDrawDragPoints = LoadValueBoolWithDefault(regKey[RegName::Editor], "ShowDragPoints"s, false);
   m_alwaysDrawLightCenters = LoadValueBoolWithDefault(regKey[RegName::Editor], "DrawLightCenters"s, false);
   m_gridSize = LoadValueIntWithDefault(regKey[RegName::Editor], "GridSize"s, 50);

   const bool autoSave = LoadValueBoolWithDefault(regKey[RegName::Editor], "AutoSaveOn"s, true);
   if (autoSave)
   {
      m_autosaveTime = LoadValueIntWithDefault(regKey[RegName::Editor], "AutoSaveTime"s, AUTOSAVE_DEFAULT_TIME);
      SetAutoSaveMinutes(m_autosaveTime);
   }
   else
      m_autosaveTime = -1;

   m_securitylevel = LoadValueIntWithDefault(regKey[RegName::Player], "SecurityLevel"s, DEFAULT_SECURITY_LEVEL);

   m_dummyMaterial.m_cBase = LoadValueIntWithDefault(regKey[RegName::Editor], "DefaultMaterialColor"s, 0xB469FF);
   m_elemSelectColor = LoadValueIntWithDefault(regKey[RegName::Editor], "ElementSelectColor"s, 0x00FF0000);
   m_elemSelectLockedColor = LoadValueIntWithDefault(regKey[RegName::Editor], "ElementSelectLockedColor"s, 0x00A7726D);
   m_backgroundColor = LoadValueIntWithDefault(regKey[RegName::Editor], "BackgroundColor"s, 0x008D8D8D);
   m_fillColor = LoadValueIntWithDefault(regKey[RegName::Editor], "FillColor"s, 0x00B1CFB3);

   if (m_securitylevel < eSecurityNone || m_securitylevel > eSecurityNoControls)
      m_securitylevel = eSecurityNoControls;

   m_recentTableList.clear();
   // get the list of the last n loaded tables
   for (int i = 0; i < LAST_OPENED_TABLE_COUNT; i++)
   {
      string szTableName;
      if (LoadValue(regKey[RegName::RecentDir], "TableFileName" + std::to_string(i), szTableName) == S_OK)
         m_recentTableList.push_back(szTableName);
      else
         break;
   }

   m_convertToUnit = LoadValueIntWithDefault(regKey[RegName::Editor], "Units"s, 0);
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
   const HCURSOR hcursor = LoadCursor(hInstance, lpCursorName);
   SetCursor(hcursor);
}

void VPinball::SetActionCur(const string& szaction)
{
   SendMessage(m_hwndStatusBar, SB_SETTEXT, 3 | 0, (size_t)szaction.c_str());
}

void VPinball::SetStatusBarElementInfo(const string& info)
{
   SendMessage(m_hwndStatusBar, SB_SETTEXT, 4 | 0, (size_t)info.c_str());
}

bool VPinball::OpenFileDialog(const string& initDir, vector<string>& filename, const char* const fileFilter, const char* const defaultExt, const DWORD flags, const string& windowTitle) //!! use this all over the place and move to some standard header
{
   CFileDialog fileDlg(TRUE, defaultExt, initDir.c_str(), nullptr, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER | flags, fileFilter); // OFN_EXPLORER needed, otherwise GetNextPathName buggy 
   if (!windowTitle.empty())
      fileDlg.SetTitle(windowTitle.c_str());
   if (fileDlg.DoModal(*this) == IDOK)
   {
      int pos = 0;
      while (pos != -1)
         filename.emplace_back(fileDlg.GetNextPathName(pos));

      return true;
   }
   else
   {
      filename.push_back(string());

      return false;
   }
}

bool VPinball::SaveFileDialog(const string& initDir, vector<string>& filename, const char* const fileFilter, const char* const defaultExt, const DWORD flags, const string& windowTitle) //!! use this all over the place and move to some standard header
{
   CFileDialog fileDlg(FALSE, defaultExt, initDir.c_str(), nullptr, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER | flags, fileFilter); // OFN_EXPLORER needed, otherwise GetNextPathName buggy 
   if (!windowTitle.empty())
      fileDlg.SetTitle(windowTitle.c_str());
   if (fileDlg.DoModal(*this) == IDOK)
   {
      int pos = 0;
      while (pos != -1)
         filename.emplace_back(fileDlg.GetNextPathName(pos));

      return true;
   }
   else
   {
      filename.push_back(string());

      return false;
   }
}

CDockProperty *VPinball::GetDefaultPropertiesDocker()
{
   constexpr int dockStyle = DS_DOCKED_RIGHT | DS_CLIENTEDGE | DS_NO_CLOSE;
   m_dockProperties = (CDockProperty *)AddDockedChild(new CDockProperty, dockStyle, 280, IDD_PROPERTY_DIALOG);

   assert(m_dockProperties->GetContainer());
   m_dockProperties->GetContainer()->SetHideSingleTab(TRUE);
   m_propertyDialog = m_dockProperties->GetContainProperties()->GetPropertyDialog();
   return m_dockProperties;
}

CDockProperty *VPinball::GetPropertiesDocker()
{
   if (m_propertyDialog == nullptr || !m_dockProperties->IsWindow())
      return GetDefaultPropertiesDocker();

   return m_dockProperties;
}

CDockToolbar *VPinball::GetDefaultToolbarDocker()
{
   constexpr int dockStyle = DS_DOCKED_LEFT | DS_CLIENTEDGE | DS_NO_CLOSE;
   m_dockToolbar = (CDockToolbar *)AddDockedChild(new CDockToolbar, dockStyle, 110, IDD_TOOLBAR);
   assert(m_dockToolbar->GetContainer());
   m_dockToolbar->GetContainer()->SetHideSingleTab(TRUE);
   m_toolbarDialog = m_dockToolbar->GetContainToolbar()->GetToolbarDialog();

   return m_dockToolbar;
}

CDockToolbar *VPinball::GetToolbarDocker()
{
   if (m_dockToolbar == nullptr || !m_dockToolbar->IsWindow())
      return GetDefaultToolbarDocker();
   return m_dockToolbar;
}

void VPinball::ResetAllDockers()
{
   const bool createNotes = m_dockNotes != nullptr;
   CloseAllDockers();
   DeleteSubKey("Editor\\Dock Windows"s); // Old Win32xx
   DeleteSubKey("Editor\\Dock Settings"s);// Win32xx 9+
   CreateDocker();
   if (createNotes)
      GetDefaultNotesDocker();
}

CDockNotes* VPinball::GetDefaultNotesDocker()
{
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
   return m_dockNotes;
}

CDockNotes* VPinball::GetNotesDocker()
{
   if (m_dockNotes != nullptr && !m_dockNotes->IsWindowEnabled())
   {
      m_dockNotes->ShowWindow();
      m_dockNotes->Enable();
   }
   return m_dockNotes;
}

CDockLayers *VPinball::GetDefaultLayersDocker()
{
   constexpr int dockStyle = DS_DOCKED_BOTTOM | DS_CLIENTEDGE | DS_NO_CLOSE;
   m_dockLayers = (CDockLayers *)m_dockProperties->AddDockedChild(new CDockLayers, dockStyle, 380, IDD_LAYERS);

   assert(m_dockLayers->GetContainer());
   m_dockLayers->GetContainer()->SetHideSingleTab(TRUE);
   m_layersListDialog = m_dockLayers->GetContainLayers()->GetLayersDialog();

   return m_dockLayers;
}

CDockLayers *VPinball::GetLayersDocker()
{
   if (m_dockLayers == nullptr || !m_dockLayers->IsWindow())
      return GetDefaultLayersDocker();
   return m_dockLayers;
}

void VPinball::CreateDocker()
{
   if (m_open_minimized || !LoadDockRegistrySettings(DOCKER_REGISTRY_KEY))
   {
      GetPropertiesDocker();
      GetToolbarDocker();
      GetLayersDocker();
   }
   m_dockProperties->GetContainer()->SetHideSingleTab(TRUE);
   m_dockLayers->GetContainer()->SetHideSingleTab(TRUE);
   m_dockToolbar->GetContainer()->SetHideSingleTab(TRUE);
}

void VPinball::SetPosCur(float x, float y)
{
   // display position 1st column in VP units
   char szT[256];
   sprintf_s(szT, sizeof(szT), "%.4f, %.4f", x, y);
   SendMessage(m_hwndStatusBar, SB_SETTEXT, 0 | 0, (size_t)szT);

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
       SendMessage(m_hwndStatusBar, SB_SETTEXT, 0 | 2, (size_t)szT);
   }

   m_mouseCursorPosition.x = x;
   m_mouseCursorPosition.y = y;
}

void VPinball::SetObjectPosCur(float x, float y)
{
   char szT[256];
   sprintf_s(szT, sizeof(szT), "%.4f, %.4f", x, y);
   SendMessage(m_hwndStatusBar, SB_SETTEXT, 1 | 0, (size_t)szT);
}

void VPinball::ClearObjectPosCur()
{
   SendMessage(m_hwndStatusBar, SB_SETTEXT, 1 | 0, (size_t)"");
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
   if (m_propertyDialog && m_propertyDialog->IsWindow())
      m_propertyDialog->UpdateTabs(pvsel);
   CComObject<PinTable>* const pt = GetActiveTable();
   if (pt)
      pt->SetFocus();
}

CMenu VPinball::GetMainMenu(int id)
{
   const int count = m_mainMenu.GetMenuItemCount();
   return m_mainMenu.GetSubMenu(id + ((count > NUM_MENUS) ? 1 : 0)); // MDI has added its stuff (table icon for first menu item)
}

bool VPinball::ParseCommand(const size_t code, const bool notify)
{
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
   case ID_TABLE_PLAY:
   {
      DoPlay(code == ID_TABLE_CAMERAMODE);
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
         ptCur->ExportBackdropPOV(string());
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
      m_videoOptDialog.DoModal(GetHwnd());
      return true;
   }
#ifdef ENABLE_SDL
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
   return false;
}

void VPinball::ReInitSound()
{
   for (size_t i = 0; i < m_vtable.size(); i++)
   {
      const PinTable * const ptT = m_vtable[i];
      for (size_t j = 0; j < ptT->m_vsound.size(); j++)
         ptT->m_vsound[j]->UnInitialize();
   }
   m_ps.ReInitPinDirectSound(GetHwnd());
   for (size_t i = 0; i < m_vtable.size(); i++)
   {
      const PinTable * const ptT = m_vtable[i];
      for (size_t j = 0; j < ptT->m_vsound.size(); j++)
         ptT->m_vsound[j]->ReInitialize();
   }
}

void VPinball::ToggleToolbar()
{
   if (m_toolbarDialog)
      m_toolbarDialog->EnableButtons();
}

void VPinball::DoPlay(const bool _cameraMode)
{
   ProfileLog("DoPlay"s);

   NumVideoBytes = 0;
   CComObject<PinTable> * const ptCur = GetActiveTable();
   if (ptCur)
      ptCur->Play(_cameraMode);
}

bool VPinball::LoadFile(const bool updateEditor)
{
   string szInitialDir;
   HRESULT hr = LoadValue(regKey[RegName::RecentDir], "LoadDir"s, szInitialDir);
   if (hr != S_OK)
      szInitialDir = "c:\\Visual Pinball\\tables\\";

   vector<string> szFileName;
   if (!OpenFileDialog(szInitialDir, szFileName, "Visual Pinball Tables (*.vpx)\0*.vpx\0Old Visual Pinball Tables(*.vpt)\0*.vpt\0", "vpx", 0,
          !updateEditor ? "Select a Table to Play or press Cancel to enter Editor-Mode"s : string()))
      return false;

   const size_t index = szFileName[0].find_last_of('\\');
   if (index != string::npos)
      hr = SaveValue(regKey[RegName::RecentDir], "LoadDir"s, szFileName[0].substr(0, index));

   LoadFileName(szFileName[0], updateEditor);

   return true;
}

void VPinball::LoadFileName(const string& szFileName, const bool updateEditor)
{
   if (m_vtable.size() == MAX_OPEN_TABLES)
   {
      ShowError("Maximum amount of tables already loaded and open.");
      return;
   }

   if (firstRun)
      OnInitialUpdate();

   m_currentTablePath = PathFromFilename(szFileName);
   CloseAllDialogs();

   PinTableMDI * const mdiTable = new PinTableMDI(this);
   CComObject<PinTable> * const ppt = mdiTable->GetTable();
   const HRESULT hr = ppt->LoadGameFromFilename(szFileName);

   if (!SUCCEEDED(hr))
   {
      if (hr == E_ACCESSDENIED)
      {
         const LocalString ls(IDS_CORRUPTFILE);
         ShowError(ls.m_szbuffer);
      }

      delete mdiTable;
   }
   else
   {
      m_vtable.push_back(ppt);

      ppt->m_szTitle = TitleFromFilename(szFileName);
      ppt->InitTablePostLoad();

      AddMDITable(mdiTable);

      // auto-import POV settings, if it exists...
      string szFileNameAuto = m_currentTablePath + ppt->m_szTitle + ".pov";
      if (Exists(szFileNameAuto)) // We check if there is a matching table pov settings file first
         ppt->ImportBackdropPOV(szFileNameAuto);
      else // Otherwise, we seek for autopov settings
      {
         szFileNameAuto = m_currentTablePath + "autopov.pov";
         if (Exists(szFileNameAuto))
            ppt->ImportBackdropPOV(szFileNameAuto);
      }

      // auto-import VBS table script, if it exists...
      szFileNameAuto = m_currentTablePath + ppt->m_szTitle + ".vbs";
      if (Exists(szFileNameAuto)) // We check if there is a matching table vbs first
         ppt->m_pcv->LoadFromFile(szFileNameAuto);
      else // Otherwise we seek in the Scripts folder
      {
         szFileNameAuto = m_szMyPath + "scripts" + PATH_SEPARATOR_CHAR + ppt->m_szTitle + ".vbs";
         if (Exists(szFileNameAuto))
            ppt->m_pcv->LoadFromFile(szFileNameAuto);
      }

      // auto-import VPP settings, if it exists...
      szFileNameAuto = m_currentTablePath + ppt->m_szTitle + ".vpp";
      if (Exists(szFileNameAuto)) // We check if there is a matching table vpp settings file first
         ppt->ImportVPP(szFileNameAuto);
      else // Otherwise, we seek for autovpp settings
      {
         szFileNameAuto = m_currentTablePath + "autovpp.vpp";
         if (Exists(szFileNameAuto))
            ppt->ImportVPP(szFileNameAuto);
      }

      // get the load path from the filename
      SaveValue(regKey[RegName::RecentDir], "LoadDir"s, m_currentTablePath);

      // make sure the load directory is the active directory
      SetCurrentDirectory(m_currentTablePath.c_str());
      UpdateRecentFileList(szFileName);

      ProfileLog("UI Post Load Start"s);

      ppt->AddMultiSel(ppt, false, true, false);
      ppt->SetDirty(eSaveClean);
      if (updateEditor)
      {
         GetLayersListDialog()->CollapseLayers();
         GetLayersListDialog()->ExpandLayers();
         ToggleToolbar();
         if (m_dockNotes != nullptr)
            m_dockNotes->Enable();

         SetFocus();
      }

      ProfileLog("UI Post Load End"s);
   }
}

CComObject<PinTable> *VPinball::GetActiveTable()
{
   PinTableMDI * const mdiTable = (PinTableMDI *)GetActiveMDIChild();
   if (mdiTable && !m_unloadingTable)
      return (CComObject<PinTable>*)mdiTable->GetTable();
   return nullptr;
}

bool VPinball::CanClose()
{
   while (m_vtable.size())
   {
      if (!m_vtable[0]->GetMDITable()->CanClose())
         return false;

      CloseTable(m_vtable[0]);
   }

   return true;
}

void VPinball::CloseTable(const PinTable * const ppt)
{
   m_unloadingTable = true;
   ppt->GetMDITable()->SendMessage(WM_SYSCOMMAND, SC_CLOSE, 0);
   m_unloadingTable = false;

   const vector<MDIChildPtr> allChildren = GetAllMDIChildren();
   if (allChildren.empty())
   {
      ToggleToolbar();
      if (m_propertyDialog && m_propertyDialog->IsWindow())
         m_propertyDialog->DeleteAllTabs();
      if (m_notesDialog && m_notesDialog->IsWindow())
         m_notesDialog->Disable();
   }
}

void VPinball::SetEnableMenuItems()
{
   CComObject<PinTable> * const ptCur = GetActiveTable();

   // Set menu item to the correct state
   const CMenu mainMenu = GetMenu();

   mainMenu.CheckMenuItem(ID_EDIT_BACKGLASSVIEW, MF_BYCOMMAND | (m_backglassView ? MF_CHECKED : MF_UNCHECKED));

   // is there a valid table??
   if (ptCur)
   {
      mainMenu.CheckMenuItem(ID_EDIT_SCRIPT, MF_BYCOMMAND | ((ptCur->m_pcv != nullptr && ptCur->m_pcv->m_visible && !ptCur->m_pcv->m_minimized) ? MF_CHECKED : MF_UNCHECKED));

      mainMenu.EnableMenuItem(IDM_CLOSE, MF_BYCOMMAND | MF_ENABLED);
      mainMenu.EnableMenuItem(ID_EDIT_UNDO, MF_BYCOMMAND | MF_ENABLED);
      mainMenu.EnableMenuItem(ID_EDIT_BACKGLASSVIEW, MF_BYCOMMAND | MF_ENABLED);
      mainMenu.EnableMenuItem(ID_TABLE_PLAY, MF_BYCOMMAND | MF_ENABLED);
      mainMenu.EnableMenuItem(ID_TABLE_CAMERAMODE, MF_BYCOMMAND | MF_ENABLED);
      mainMenu.EnableMenuItem(ID_TABLE_MAGNIFY, MF_BYCOMMAND | MF_ENABLED);
      mainMenu.EnableMenuItem(ID_TABLE_TABLEINFO, MF_BYCOMMAND | MF_ENABLED);
      mainMenu.EnableMenuItem(ID_TABLE_RENDERPROBEMANAGER, MF_BYCOMMAND | MF_ENABLED);
      mainMenu.EnableMenuItem(ID_EDIT_SEARCH, MF_BYCOMMAND | MF_ENABLED);
      mainMenu.EnableMenuItem(ID_EDIT_DRAWINGORDER_HIT, MF_BYCOMMAND | MF_ENABLED);
      mainMenu.EnableMenuItem(ID_EDIT_DRAWINGORDER_SELECT, MF_BYCOMMAND | MF_ENABLED);
      // enable/disable save options
      constexpr UINT flags = MF_BYCOMMAND | MF_ENABLED;
      mainMenu.EnableMenuItem(IDM_SAVE, flags);
      mainMenu.EnableMenuItem(IDM_SAVEAS, flags);
      mainMenu.EnableMenuItem(ID_FILE_EXPORT_BLUEPRINT, flags);
      mainMenu.EnableMenuItem(ID_EXPORT_TABLEMESH, flags);
      mainMenu.EnableMenuItem(ID_EXPORT_BACKDROPPOV, flags);
      mainMenu.EnableMenuItem(ID_IMPORT_BACKDROPPOV, flags);

      // enable/disable script option
      mainMenu.EnableMenuItem(ID_EDIT_SCRIPT, flags);

      // enable/disable managers options
      mainMenu.EnableMenuItem(ID_TABLE_SOUNDMANAGER, flags);
      mainMenu.EnableMenuItem(ID_TABLE_IMAGEMANAGER, flags);
      mainMenu.EnableMenuItem(ID_TABLE_FONTMANAGER, flags);
      mainMenu.EnableMenuItem(ID_TABLE_MATERIALMANAGER, flags);
      mainMenu.EnableMenuItem(ID_TABLE_COLLECTIONMANAGER, flags);

      // enable/disable editing options
      mainMenu.EnableMenuItem(IDC_COPY, flags);
      mainMenu.EnableMenuItem(IDC_PASTE, flags);
      mainMenu.EnableMenuItem(IDC_PASTEAT, flags);
      mainMenu.EnableMenuItem(ID_DELETE, flags);
      mainMenu.EnableMenuItem(ID_TABLE_NOTES, flags);

      mainMenu.CheckMenuItem(ID_VIEW_SOLID, MF_BYCOMMAND | (ptCur->RenderSolid() ? MF_CHECKED : MF_UNCHECKED));
      mainMenu.CheckMenuItem(ID_VIEW_OUTLINE, MF_BYCOMMAND | (ptCur->RenderSolid() ? MF_UNCHECKED : MF_CHECKED));

      mainMenu.CheckMenuItem(ID_VIEW_GRID, MF_BYCOMMAND | (ptCur->m_grid ? MF_CHECKED : MF_UNCHECKED));
      mainMenu.CheckMenuItem(ID_VIEW_BACKDROP, MF_BYCOMMAND | (ptCur->m_backdrop ? MF_CHECKED : MF_UNCHECKED));
   }
   else
   {
      /* no valid table, disable a few items */
      mainMenu.EnableMenuItem(IDM_CLOSE, MF_BYCOMMAND | MF_GRAYED);
      mainMenu.EnableMenuItem(IDM_SAVE, MF_BYCOMMAND | MF_GRAYED);
      mainMenu.EnableMenuItem(IDM_SAVEAS, MF_BYCOMMAND | MF_GRAYED);
      mainMenu.EnableMenuItem(ID_FILE_EXPORT_BLUEPRINT, MF_BYCOMMAND | MF_GRAYED);
      mainMenu.EnableMenuItem(ID_EXPORT_TABLEMESH, MF_BYCOMMAND | MF_GRAYED);
      mainMenu.EnableMenuItem(ID_EXPORT_BACKDROPPOV, MF_BYCOMMAND | MF_GRAYED);
      mainMenu.EnableMenuItem(ID_IMPORT_BACKDROPPOV, MF_BYCOMMAND | MF_GRAYED);

      mainMenu.EnableMenuItem(ID_EDIT_UNDO, MF_BYCOMMAND | MF_GRAYED);
      mainMenu.EnableMenuItem(IDC_COPY, MF_BYCOMMAND | MF_GRAYED);
      mainMenu.EnableMenuItem(IDC_PASTE, MF_BYCOMMAND | MF_GRAYED);
      mainMenu.EnableMenuItem(IDC_PASTEAT, MF_BYCOMMAND | MF_GRAYED);
      mainMenu.EnableMenuItem(ID_DELETE, MF_BYCOMMAND | MF_GRAYED);
      mainMenu.EnableMenuItem(ID_EDIT_SCRIPT, MF_BYCOMMAND | MF_GRAYED);
      mainMenu.EnableMenuItem(ID_EDIT_BACKGLASSVIEW, MF_BYCOMMAND | MF_GRAYED);
      mainMenu.EnableMenuItem(ID_TABLE_NOTES, MF_BYCOMMAND | MF_GRAYED);

      mainMenu.EnableMenuItem(ID_TABLE_PLAY, MF_BYCOMMAND | MF_GRAYED);
      mainMenu.EnableMenuItem(ID_TABLE_CAMERAMODE, MF_BYCOMMAND | MF_GRAYED);
      mainMenu.EnableMenuItem(ID_TABLE_SOUNDMANAGER, MF_BYCOMMAND | MF_GRAYED);
      mainMenu.EnableMenuItem(ID_TABLE_IMAGEMANAGER, MF_BYCOMMAND | MF_GRAYED);
      mainMenu.EnableMenuItem(ID_TABLE_FONTMANAGER, MF_BYCOMMAND | MF_GRAYED);
      mainMenu.EnableMenuItem(ID_TABLE_MATERIALMANAGER, MF_BYCOMMAND | MF_GRAYED);
      mainMenu.EnableMenuItem(ID_TABLE_COLLECTIONMANAGER, MF_BYCOMMAND | MF_GRAYED);
      mainMenu.EnableMenuItem(ID_TABLE_RENDERPROBEMANAGER, MF_BYCOMMAND | MF_GRAYED);
      mainMenu.EnableMenuItem(ID_TABLE_TABLEINFO, MF_BYCOMMAND | MF_GRAYED);
      mainMenu.EnableMenuItem(ID_TABLE_MAGNIFY, MF_BYCOMMAND | MF_GRAYED);
      mainMenu.EnableMenuItem(ID_EDIT_SEARCH, MF_BYCOMMAND | MF_GRAYED);
      mainMenu.EnableMenuItem(ID_EDIT_DRAWINGORDER_HIT, MF_BYCOMMAND | MF_GRAYED);
      mainMenu.EnableMenuItem(ID_EDIT_DRAWINGORDER_SELECT, MF_BYCOMMAND | MF_GRAYED);
   }
}

void VPinball::UpdateRecentFileList(const string& szfilename)
{
   const size_t old_count = m_recentTableList.size();

   // if the loaded file name is a valid one then add it to the top of the list
   if (!szfilename.empty())
   {
      vector<string> newList;
      newList.push_back(szfilename);

      for (const string &tableName : m_recentTableList)
      {
         if (tableName != newList[0]) // does this file name aready exist in the list?
            newList.push_back(tableName);
      }

      m_recentTableList.clear();

      int i = 0;
      for (const string &tableName : newList)
      {
         m_recentTableList.push_back(tableName);
         // write entry to the registry
         SaveValue(regKey[RegName::RecentDir], "TableFileName" + std::to_string(i), tableName);

         if (++i == LAST_OPENED_TABLE_COUNT)
            break;
      }
   }

   // update the file menu to contain the last n recent loaded files
   // must be at least 1 recent file in the list
   if (!m_recentTableList.empty())
   {
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
         // now search for filenames with & and replace with && so that these display correctly
         const char * const ns = replace(m_recentTableList[i].c_str(), "&", "&&");
         char recentMenuname[MAX_PATH];
         snprintf(recentMenuname, MAX_PATH-1, "&%i  %s", (int)i+1, ns);
         delete[] ns;

         // set the IDM of this menu item
         // set up the menu info block
         MENUITEMINFO menuInfo = {};
         menuInfo.cbSize = GetSizeofMenuItemInfo();
         menuInfo.fMask = MIIM_ID | MIIM_STRING | MIIM_STATE;
         menuInfo.fState = MFS_ENABLED;
         menuInfo.wID = RECENT_FIRST_MENU_IDM + (UINT)i;
         menuInfo.dwTypeData = recentMenuname;
         menuInfo.cch = (UINT)strlen(recentMenuname);

         menuFile.InsertMenuItem(count, menuInfo, TRUE);
         //or: menuFile.InsertMenu(count, MF_BYPOSITION | MF_ENABLED | MF_STRING, RECENT_FIRST_MENU_IDM + (UINT)i, recentMenuname);
         count++;
      }

      // add a separator onto the end
      MENUITEMINFO menuInfo = {};
      menuInfo.cbSize = GetSizeofMenuItemInfo();
      menuInfo.fMask = MIIM_ID | MIIM_TYPE | MIIM_STATE;
      menuInfo.fState = MFS_ENABLED;
      menuInfo.fType = MFT_SEPARATOR;
      menuInfo.wID = RECENT_FIRST_MENU_IDM + (UINT)m_recentTableList.size();

      menuFile.InsertMenuItem(count, menuInfo, TRUE);

      // update the menu bar
      DrawMenuBar();
   }
}

bool VPinball::processKeyInputForDialogs(MSG *pmsg)
{
   bool consumed = false;
   if (m_ptableActive)
   {
      //const int keyPressed = LOWORD(pmsg->wParam);
      if (m_materialDialog.IsWindow())
         consumed = !!m_materialDialog.IsDialogMessage(*pmsg);
      if (!consumed && m_imageMngDlg.IsWindow())
         consumed = !!m_imageMngDlg.IsDialogMessage(*pmsg);
      if (!consumed && m_soundMngDlg.IsWindow())
         consumed = !!m_soundMngDlg.IsDialogMessage(*pmsg);
      if (!consumed && m_collectionMngDlg.IsWindow())
         consumed = !!m_collectionMngDlg.IsDialogMessage(*pmsg);
      if (!consumed && m_dimensionDialog.IsWindow())
         consumed = !!m_dimensionDialog.IsDialogMessage(*pmsg);
      if (!consumed && m_renderProbeDialog.IsWindow())
         consumed = !!m_renderProbeDialog.IsDialogMessage(*pmsg);

      const HWND activeHwnd = ::GetFocus();
      if (!consumed && m_toolbarDialog && (activeHwnd == m_toolbarDialog->GetHwnd()))
         consumed = m_toolbarDialog->PreTranslateMessage(pmsg);
      if (!consumed && m_propertyDialog)
         consumed = m_propertyDialog->PreTranslateMessage(pmsg);
      if (!consumed && m_layersListDialog)
         consumed = m_layersListDialog->PreTranslateMessage(pmsg);
      if (!consumed && m_notesDialog && activeHwnd == m_notesDialog->GetHwnd())
         consumed = m_notesDialog->PreTranslateMessage(pmsg);
   }
   return consumed;
}

static int GetZOrder(HWND hWnd)
{
   int z = 0;
   for (HWND h = hWnd; h != nullptr; h = GetWindow(h, GW_HWNDPREV))
      z++;
   return z;
}

bool VPinball::ApcHost_OnTranslateMessage(MSG* pmsg)
{
   bool consumed = false;

   if (g_pplayer == nullptr)
   {
      // check if message must be processed by the code editor
      if (m_pcv && (GetZOrder(m_pcv->GetHwnd()) < GetZOrder(GetHwnd())))
      {
         consumed = m_pcv->PreTranslateMessage(pmsg);

         if (m_pcv->m_hwndFind && ::IsDialogMessage(m_pcv->m_hwndFind, pmsg))
            consumed = true;
      }

      if (!consumed)
         // check/process events for other dialogs (material/sound/image manager, toolbar, properties)
         consumed = processKeyInputForDialogs(pmsg);

      if (!consumed)
         consumed = !!TranslateAccelerator(GetHwnd(), g_haccel, pmsg);

      if (!consumed)
         TranslateMessage(pmsg);
   }
   else
   {
      consumed = false;
      if (g_pplayer->m_debugMode)
      {
         if (g_pplayer->m_debuggerDialog.IsWindow())
            consumed = !!g_pplayer->m_debuggerDialog.IsSubDialogMessage(*pmsg);
      }
   }

   return consumed;
}

void VPinball::MainMsgLoop()
{
   for (;;)
   {
      MSG msg;
      if (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE))
      {
         if (msg.message == WM_QUIT)
            break;

         try {
            const bool consumed = ApcHost_OnTranslateMessage(&msg);
            if (!consumed)
               DispatchMessage(&msg);
         }
         catch (...) // something failed on load/init
         {
            auto player = g_pplayer;
            g_pplayer = nullptr;
            delete player;

            g_pvp->m_ptableActive->HandleLoadFailure();
         }
      }
      else
      {
         if (g_pplayer && !g_pplayer->m_pause)
            g_pplayer->Render(); // always render on idle
         else
         {
            // if player has been closed in the meantime, check if we should display the file open dialog again to select/play the next table
            if (m_table_played_via_SelectTableOnStart && (!g_pplayer || !g_pplayer->m_pause))
            {
               // first close the current table
               CComObject<PinTable>* const pt = GetActiveTable();
               if (pt)
                  CloseTable(pt);
               // then select the new one, and if one was selected, play it
               m_table_played_via_SelectTableOnStart = LoadFile(false);
               if (m_table_played_via_SelectTableOnStart)
                  DoPlay(false);
            }

            WaitMessage(); // otherwise wait for input
         }
      }
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
   m_cref--;

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
   wc.hIcon = LoadIcon(theInstance, MAKEINTRESOURCE(IDI_VPINBALL));
   wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
   wc.style = CS_DBLCLKS; //CS_NOCLOSE | CS_OWNDC;
   wc.lpszClassName = _T("VPinball");
   //wc.lpszMenuName = MAKEINTRESOURCE(IDR_APPMENU);
}

void VPinball::OnClose()
{
   CComObject<PinTable> * const ptable = GetActiveTable();
   m_closing = true;

   if (ptable)
      while (ptable->m_savingActive)
         Sleep(THREADS_PAUSE);

   if (g_pplayer)
      g_pplayer->SendMessage(WM_CLOSE, 0, 0);

   const bool canClose = CanClose();
   if (canClose)
   {
      WINDOWPLACEMENT winpl;
      winpl.length = sizeof(winpl);

      if (GetWindowPlacement(winpl))
      {
         SaveValueInt(regKey[RegName::Editor], "WindowLeft"s, winpl.rcNormalPosition.left);
         SaveValueInt(regKey[RegName::Editor], "WindowTop"s, winpl.rcNormalPosition.top);
         SaveValueInt(regKey[RegName::Editor], "WindowRight"s, winpl.rcNormalPosition.right);
         SaveValueInt(regKey[RegName::Editor], "WindowBottom"s, winpl.rcNormalPosition.bottom);

         SaveValueBool(regKey[RegName::Editor], "WindowMaximized"s, !!IsZoomed());
      }
      if (!m_open_minimized) // otherwise the window/dock settings are screwed up and have to be manually restored each time
         SaveDockRegistrySettings(DOCKER_REGISTRY_KEY);

      CWnd::OnClose();
   }
}

void VPinball::OnDestroy()
{
    PostMessage(WM_QUIT, 0, 0);
}

void VPinball::ShowSubDialog(CDialog &dlg, const bool show)
{
   if (!dlg.IsWindow())
   {
      dlg.Create(GetHwnd());
      dlg.ShowWindow(show ? SW_SHOWNORMAL : SW_HIDE);
   }
   else
      dlg.SetForegroundWindow();
}

int VPinball::OnCreate(CREATESTRUCT& cs)
{
   // OnCreate controls the way the frame is created.
   // Overriding CFrame::OnCreate is optional.
   // Uncomment the lines below to change frame options.

   //     UseIndicatorStatus(FALSE);    // Don't show keyboard indicators in the StatusBar
   //     UseMenuStatus(FALSE);         // Don't show menu descriptions in the StatusBar
   //     UseReBar(FALSE);              // Don't use a ReBar
   //     UseStatusBar(FALSE);          // Don't use a StatusBar
   //     UseThemes(FALSE);             // Don't use themes
   //     UseToolBar(FALSE);            // Don't use a ToolBar

   m_mainMenu.LoadMenu(_T("IDR_APPMENU"));

   SetFrameMenu(m_mainMenu.GetHandle());

   const int result = CMDIDockFrame::OnCreate(cs);

   char szName[256];
   LoadString(theInstance, IDS_PROJNAME, szName, 256);
   SetWindowText(szName);

   return result;
}

LRESULT VPinball::OnPaint(UINT msg, WPARAM wparam, LPARAM lparam)
{
   PAINTSTRUCT ps;
   const HDC hdc = BeginPaint(ps);
   const CRect rc = GetClientRect();
   SelectObject(hdc, GetStockObject(WHITE_BRUSH));
   PatBlt(hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, PATCOPY);
   EndPaint(ps);
   return 0;
}

void VPinball::OnInitialUpdate()
{
   if (!firstRun)
      return;

   wintimer_init();                    // calibrate the timer routines

   ProfileLog("OnInitialUpdate"s);

   constexpr int foo[6] = { 120, 240, 400, 600, 800, 1400 };

   m_hwndStatusBar = CreateStatusWindow(WS_CHILD | WS_VISIBLE,
                                        "",
                                        GetHwnd(),
                                        1);                     // Create Status Line at the bottom

   ::SendMessage(m_hwndStatusBar, SB_SETPARTS, 6, (size_t)foo); // Initialize Status bar with 6 empty cells

   InitRegValues();                    // get default values from registry

   SendMessage(WM_SIZE, 0, 0);	        // Make our window relay itself out

   m_ps.InitPinDirectSound(GetHwnd());

   m_backglassView = false;            // we are viewing Pinfield and not the backglass at first

   UpdateRecentFileList(string());     // update the recent loaded file list

   int left, top, right, bottom;
   BOOL maximized;

   const HRESULT hrleft = LoadValue(regKey[RegName::Editor], "WindowLeft"s, left);
   const HRESULT hrtop = LoadValue(regKey[RegName::Editor], "WindowTop"s, top);
   const HRESULT hrright = LoadValue(regKey[RegName::Editor], "WindowRight"s, right);
   const HRESULT hrbottom = LoadValue(regKey[RegName::Editor], "WindowBottom"s, bottom);

   const HRESULT hrmax = LoadValue(regKey[RegName::Editor], "WindowMaximized"s, maximized);

   if (hrleft == S_OK && hrtop == S_OK && hrright == S_OK && hrbottom == S_OK)
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
#ifdef SLINTF
   // see slintf.cpp
   slintf_init(); // initialize debug console (can be popupped by the following command)
   slintf_popup_console();
   slintf("Debug output:\n");
#endif

   CreateDocker();
   ShowWindow(SW_SHOW);
//   InitTools();
//   SetForegroundWindow();
   SetEnableMenuItems();

   // Load 'in playing mode' image for UI

   m_hbmInPlayMode = (HBITMAP)LoadImage(theInstance, MAKEINTRESOURCE(IDB_INPLAYMODE), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);

   // all done, let's go

   firstRun = false;
}

BOOL VPinball::OnCommand(WPARAM wparam, LPARAM lparam)
{
   if (!ParseCommand(LOWORD(wparam), HIWORD(wparam) == 1))
   {
      const auto mdiTable = GetActiveMDIChild();
      if (mdiTable)
         mdiTable->SendMessage(WM_COMMAND, wparam, lparam);
      return FALSE;
   }
   return TRUE;
}

LRESULT VPinball::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   switch (uMsg)
   {
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
   case UWM_UPDATECOMMAND:
      return FinalWindowProc(uMsg, wParam, lParam);
   }
   return WndProcDefault(uMsg, wParam, lParam);
}

LRESULT VPinball::OnMDIActivated(UINT msg, WPARAM wparam, LPARAM lparam)
{
   if (m_dockLayers != nullptr)
      m_dockLayers->GetContainLayers()->GetLayersDialog()->UpdateLayerList();
   if (m_dockNotes != nullptr)
      m_dockNotes->Refresh();
   return CMDIFrameT::OnMDIActivated(msg, wparam, lparam);
}

LRESULT VPinball::OnMDIDestroyed(UINT msg, WPARAM wparam, LPARAM lparam)
{
   if (GetAllMDIChildren().size() == 1)
   {
      GetLayersListDialog()->ClearList();
      GetLayersListDialog()->SetActiveTable(nullptr);
   }
   return CMDIFrameT::OnMDIDestroyed(msg, wparam, lparam);
}

Win32xx::CDocker *VPinball::NewDockerFromID(int id)
{
   switch (id)
   {
   case IDD_PROPERTY_DIALOG:
   {
      if (m_dockProperties == nullptr)
      {
         m_dockProperties = new CDockProperty();
         m_propertyDialog = m_dockProperties->GetContainProperties()->GetPropertyDialog();
      }
      return m_dockProperties;
   }
   case IDD_TOOLBAR:
   {
      if (m_dockToolbar == nullptr)
      {
         m_dockToolbar = new CDockToolbar();
         m_toolbarDialog = m_dockToolbar->GetContainToolbar()->GetToolbarDialog();
      }
      return m_dockToolbar;
   }
   case IDD_LAYERS:
   {
      if (m_dockLayers == nullptr)
      {
         m_dockLayers = new CDockLayers();
         m_layersListDialog = m_dockLayers->GetContainLayers()->GetLayersDialog();
      }
      return m_dockLayers;
   }
//   case IDD_NOTES_DIALOG:
//   {
//      if (m_dockNotes == nullptr)
//      {
//         m_dockNotes = new CDockNotes();
//         m_notesDialog = m_dockNotes->GetContainNotes()->GetNotesDialog();
//      }
//      return m_dockToolbar;
//   }
   }
   return nullptr;
}

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
   if (g_pplayer) g_pplayer->m_ptable->QuitPlayer(CloseType);

   return S_OK;
}

void VPinball::Quit()
{
   if (g_pplayer) {
      g_pplayer->m_closeDown = true;
      g_pplayer->m_closeType = 1;
   }
   else
      PostMessage(WM_CLOSE, 0, 0);
}

int CALLBACK MyCompProc(LPARAM lSortParam1, LPARAM lSortParam2, LPARAM lSortOption)
{
   LVFINDINFO lvf;
   char buf1[MAX_PATH], buf2[MAX_PATH];

   const SORTDATA *lpsd = (SORTDATA *)lSortOption;

   lvf.flags = LVFI_PARAM;
   lvf.lParam = lSortParam1;
   const int nItem1 = ListView_FindItem(lpsd->hwndList, -1, &lvf);

   lvf.lParam = lSortParam2;
   const int nItem2 = ListView_FindItem(lpsd->hwndList, -1, &lvf);

   ListView_GetItemText(lpsd->hwndList, nItem1, lpsd->subItemIndex, buf1, sizeof(buf1));

   ListView_GetItemText(lpsd->hwndList, nItem2, lpsd->subItemIndex, buf2, sizeof(buf2));
   if (nItem2 == -1 || nItem1 == -1)
      return 0;
   if (lpsd->sortUpDown == 1)
      return (_stricmp(buf1, buf2));
   else
      return (_stricmp(buf1, buf2) * -1);
}

int CALLBACK MyCompProcIntValues(LPARAM lSortParam1, LPARAM lSortParam2, LPARAM lSortOption)
{
   LVFINDINFO lvf;
   const SORTDATA * const lpsd = (SORTDATA *)lSortOption;

   lvf.flags = LVFI_PARAM;
   lvf.lParam = lSortParam1;
   const int nItem1 = ListView_FindItem(lpsd->hwndList, -1, &lvf);

   lvf.lParam = lSortParam2;
   const int nItem2 = ListView_FindItem(lpsd->hwndList, -1, &lvf);

   char buf1[20], buf2[20];
   ListView_GetItemText(lpsd->hwndList, nItem1, lpsd->subItemIndex, buf1, sizeof(buf1));
   ListView_GetItemText(lpsd->hwndList, nItem2, lpsd->subItemIndex, buf2, sizeof(buf2));

   int value1, value2;
   sscanf_s(buf1, "%i", &value1);
   sscanf_s(buf2, "%i", &value2);

   if (lpsd->sortUpDown == 1)
      return (value1 - value2);
   else
      return (value2 - value1);
}

int CALLBACK MyCompProcMemValues(LPARAM lSortParam1, LPARAM lSortParam2, LPARAM lSortOption)
{
   const SORTDATA * const lpsd = (SORTDATA *)lSortOption;
   const Texture * const t1 = (Texture *)lSortParam1;
   const Texture * const t2 = (Texture *)lSortParam2;
   const unsigned int t1_size = t1->m_pdsBuffer == nullptr ? 0 : t1->m_pdsBuffer->height() * t1->m_pdsBuffer->pitch();
   const unsigned int t2_size = t2->m_pdsBuffer == nullptr ? 0 : t2->m_pdsBuffer->height() * t2->m_pdsBuffer->pitch();
   if (lpsd->sortUpDown == 1)
      return (int)(t1_size - t2_size);
   else
      return (int)(t2_size - t1_size);
}

static constexpr int rgDlgIDFromSecurityLevel[] = { IDC_ACTIVEX0, IDC_ACTIVEX1, IDC_ACTIVEX2, IDC_ACTIVEX3, IDC_ACTIVEX4 };

INT_PTR CALLBACK SecurityOptionsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
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

      int security = LoadValueIntWithDefault(regKey[RegName::Player], "SecurityLevel"s, DEFAULT_SECURITY_LEVEL);
      if (security < 0 || security > 4)
         security = 0;

      const int buttonid = rgDlgIDFromSecurityLevel[security];

      SendMessage(GetDlgItem(hwndDlg, buttonid), BM_SETCHECK, BST_CHECKED, 0);

      const bool hangdetect = LoadValueBoolWithDefault(regKey[RegName::Player], "DetectHang"s, false);
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
                  SaveValueInt(regKey[RegName::Player], "SecurityLevel"s, i);
            }

            const bool hangdetect = (SendMessage(GetDlgItem(hwndDlg, IDC_HANGDETECT), BM_GETCHECK, 0, 0) != 0);
            SaveValueBool(regKey[RegName::Player], "DetectHang"s, hangdetect);

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

   return FALSE;
}

INT_PTR CALLBACK FontManagerProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   CCO(PinTable) *pt = (CCO(PinTable) *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

   switch (uMsg)
   {
   case WM_INITDIALOG:
   {
      SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

      LVCOLUMN lvcol;
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
            string szInitialDir;
            const HRESULT hr = LoadValue(regKey[RegName::RecentDir], "FontDir"s, szInitialDir);
            if (hr != S_OK)
               szInitialDir = "c:\\Visual Pinball\\tables\\";

            vector<string> szFileName;
            if (g_pvp->OpenFileDialog(szInitialDir, szFileName, "Font Files (*.ttf)\0*.ttf\0", "ttf", 0))
            {
               const size_t index = szFileName[0].find_last_of('\\');
               if (index != string::npos)
                  SaveValue(regKey[RegName::RecentDir], "FontDir"s, szFileName[0].substr(0, index));

               pt->ImportFont(GetDlgItem(hwndDlg, IDC_SOUNDLIST), szFileName[0]);
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

         /*case IDC_REIMPORT:
         {
         const int ans = MessageBox(hwndDlg, "Are you sure you want to replace this image with a new one?", "Confirm Reimport", MB_YESNO | MB_DEFBUTTON2);
         if (ans == IDYES)
         {
         const int sel = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), -1, LVNI_SELECTED);
         if (sel != -1)
         {
         LVITEM lvitem;
         lvitem.mask = LVIF_PARAM;
         lvitem.iItem = sel;
         lvitem.iSubItem = 0;
         ListView_GetItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), &lvitem);
         PinImage * const ppi = (PinImage *)lvitem.lParam;

         pt->ReImportImage(ppi, ppi->m_szPath);

         // Display new image
         InvalidateRect(GetDlgItem(hwndDlg, IDC_PICTUREPREVIEW), nullptr, fTrue);
         }
         }
         }
         break;

         case IDC_REIMPORTFROM:
         {
         const int sel = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), -1, LVNI_SELECTED);
         if (sel != -1)
         {
         char szFileName[MAXSTRING];

         const int ans = MessageBox(hwndDlg, "Are you sure you want to replace this image with a new one?", "Confirm Reimport", MB_YESNO | MB_DEFBUTTON2);
         if (ans == IDYES)
         {
         szFileName[0] = '\0';

         OPENFILENAME ofn = {};
         ofn.lStructSize = sizeof(OPENFILENAME);
         ofn.hInstance = g_pvp->theInstance;
         ofn.hwndOwner = g_pvp->m_hwnd;
         ofn.lpstrFilter = "Font Files (*.ttf)\0*.ttf\0";
         ofn.lpstrFile = szFileName;
         ofn.nMaxFile = sizeof(szFileName);
         ofn.lpstrDefExt = "ttf";
         ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

         const int ret = GetOpenFileName(&ofn);

         if (ret)
         {
         LVITEM lvitem;
         lvitem.mask = LVIF_PARAM;
         lvitem.iItem = sel;
         lvitem.iSubItem = 0;
         ListView_GetItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), &lvitem);
         PinImage * const ppi = (PinImage *)lvitem.lParam;

         pt->ReImportImage(ppi, ofn.lpstrFile);

         // Display new image
         InvalidateRect(GetDlgItem(hwndDlg, IDC_PICTUREPREVIEW), nullptr, fTrue);
         }
         }
         }
         }
         break;*/
         }
         break;
      }
      break;
   }

   return FALSE;
}


void VPinball::ShowDrawingOrderDialog(bool select)
{
   DrawingOrderDialog orderDlg(select);
   orderDlg.DoModal();
}

void VPinball::CloseAllDialogs()
{
   if (m_imageMngDlg.IsWindow())
      m_imageMngDlg.Destroy();
   if (m_soundMngDlg.IsWindow())
      m_soundMngDlg.Destroy();
   if (m_audioOptDialog.IsWindow())
      m_audioOptDialog.Destroy();
   if (m_editorOptDialog.IsWindow())
      m_editorOptDialog.Destroy();
   if (m_videoOptDialog.IsWindow())
      m_videoOptDialog.Destroy();
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
#ifdef ENABLE_SDL
   if (m_vrOptDialog.IsWindow())
      m_vrOptDialog.Destroy();
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
      const bool alwaysViewScript = LoadValueBoolWithDefault(regKey[RegName::Editor], "AlwaysViewScript"s, false);

      ptCur->m_pcv->SetVisible(alwaysViewScript || !(ptCur->m_pcv->m_visible && !ptCur->m_pcv->m_minimized));

      //SendMessage(m_hwndToolbarMain, TB_CHECKBUTTON, ID_EDIT_SCRIPT, MAKELONG(ptCur->m_pcv->m_visible && !ptCur->m_pcv->m_minimized, 0));
   }
}

void VPinball::ShowSearchSelect()
{
   CComObject<PinTable> * const ptCur = GetActiveTable();
   if (ptCur)
   {
      if (!ptCur->m_searchSelectDlg.IsWindow())
      {
         ptCur->m_searchSelectDlg.Create(GetHwnd());

         const string windowName = "Search/Select Element - " + ptCur->m_szFileName;
         ptCur->m_searchSelectDlg.SetWindowText(windowName.c_str());

         ptCur->m_searchSelectDlg.ShowWindow();
      }
      else
      {
         ptCur->m_searchSelectDlg.ShowWindow();
         ptCur->m_searchSelectDlg.SetForegroundWindow();
      }
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
      ptCur->m_renderSolid = (viewId == ID_VIEW_SOLID);
      GetMenu().CheckMenuItem(ID_VIEW_SOLID, MF_BYCOMMAND | (ptCur->RenderSolid() ? MF_CHECKED : MF_UNCHECKED));
      GetMenu().CheckMenuItem(ID_VIEW_OUTLINE, MF_BYCOMMAND | (ptCur->RenderSolid() ? MF_UNCHECKED : MF_CHECKED));

      ptCur->SetDirtyDraw();
      SaveValueBool(regKey[RegName::Editor], "RenderSolid"s, ptCur->m_renderSolid);
   }
}

void VPinball::ShowGridView()
{
   CComObject<PinTable> * const ptCur = GetActiveTable();
   if (ptCur)
   {
      ptCur->put_DisplayGrid(FTOVB(!ptCur->m_grid));
      GetMenu().CheckMenuItem(ID_VIEW_GRID, MF_BYCOMMAND | (ptCur->m_grid ? MF_CHECKED : MF_UNCHECKED));
   }
}

void VPinball::ShowBackdropView()
{
   CComObject<PinTable> * const ptCur = GetActiveTable();
   if (ptCur)
   {
      ptCur->put_DisplayBackdrop(FTOVB(!ptCur->m_backdrop));
      GetMenu().CheckMenuItem(ID_VIEW_BACKDROP, MF_BYCOMMAND | (ptCur->m_backdrop ? MF_CHECKED : MF_UNCHECKED));
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
         UpdateRecentFileList(ptCur->m_szFileName);
   }
}

void VPinball::OpenNewTable(size_t tableId)
{
   if (m_vtable.size() == MAX_OPEN_TABLES)
   {
      ShowError("Maximum amount of tables already loaded and open.");
      return;
   }

   PinTableMDI * const mdiTable = new PinTableMDI(this);
   CComObject<PinTable>* const ppt = mdiTable->GetTable();
   m_vtable.push_back(ppt);
   ppt->InitBuiltinTable(tableId);
   ppt->InitTablePostLoad();

   AddMDITable(mdiTable);
   mdiTable->GetTable()->AddMultiSel(mdiTable->GetTable(), false, true, false);
   GetLayersListDialog()->CollapseLayers();
   GetLayersListDialog()->ExpandLayers();
   ToggleToolbar();
   if (m_dockNotes != nullptr)
      m_dockNotes->Enable();

   SetFocus();
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
   if (ptCur)
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

void VPinball::ProfileLog(const string& msg)
{
   if (m_profile_file)
   {
      const double sec = msec() * (1.0/1000.0);
      fprintf(m_profile_file, "%.3f : %s\n", sec, msg.c_str());
   }
}
