// license:GPLv3+

// implementation of the VPinball class.

#include "core/stdafx.h"
#include "ui/VPXFileFeedback.h"
#include "ui/win/resource.h"
#ifndef __STANDALONE__
#include "ui/win/dialogs/PlayerOptionsDialog.h"
#endif

#ifdef __STANDALONE__
#include <iostream>
#endif

#ifdef __LIBVPINBALL__
#include "lib/src/VPinballLib.h"
#endif

#include <filesystem>
#ifndef __STANDALONE__
#include "FreeImage.h"
#else
#include "standalone/FreeImage.h"
#endif
#include "core/vpversion.h"

#include "ui/win/PinTableMDI.h"

#include "ui/win/worker.h"


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

WinEditor::WinEditor(HINSTANCE appInstance)
   : m_instance(appInstance)
{
   // DLL_API void DLL_CALLCONV FreeImage_Initialise(BOOL load_local_plugins_only FI_DEFAULT(FALSE)); // would only be needed if linking statically
   m_closing = false;
   m_unloadingTable = false;
   m_cref = 0;				//inits Reference Count for IUnknown Interface. Every com Object must 
   //implement this and StdMethods QueryInterface, AddRef and Release

   m_mouseCursorPosition.x = 0.0f;
   m_mouseCursorPosition.y = 0.0f;

   m_NextTableID = 1;

   m_workerthread = nullptr;//Workerthread - only for hanging scripts and autosave - will be created later

   m_ToolCur = IDC_SELECT;

   m_hbmInPlayMode = nullptr;

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

   wintimer_init();
}

//deletes clipboard
//Releases Resources for Script editor
WinEditor::~WinEditor()
{
   // DLL_API void DLL_CALLCONV FreeImage_DeInitialise(); // would only be needed if linking statically
   SetClipboard(nullptr);
#ifndef __STANDALONE__
   FreeLibrary(m_scintillaDll);
#endif
}


//Post Work to the worker Thread
//Creates Worker-Thread if not present
//See Worker::VPWorkerThreadStart for infos
//workid int for the type of message (COMPLETE_AUTOSAVE | HANG_SNOOP_START | HANG_SNOOP_STOP)
//Second Parameter for message (AutoSavePackage (see worker.h) if COMPLETE_AUTOSAVE, otherwise nullptr)
//returns Handle to Event that get ack. If event is finished (unsure)
HANDLE WinEditor::PostWorkToWorkerThread(int workid, LPARAM lParam)
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
   return nullptr;
#endif
}

void WinEditor::SetAutoSaveMinutes(const int minutes)
{
   m_autosaveTime = (minutes <= 0) ? -1 : minutes * (60 * 1000); // convert to milliseconds
}

//Post Work to the worker Thread
//Creates Worker-Thread if not present
//See Worker::VPWorkerThreadStart for infos
//workid int for the type of message (COMPLETE_AUTOSAVE | HANG_SNOOP_START | HANG_SNOOP_STOP)
//Second Parameter for message (AutoSavePackage (see worker.h) if COMPLETE_AUTOSAVE, otherwise nullptr)
//returns Handle to Event that get ack. If event is finished (unsure)
void WinEditor::InitTools()
{
   m_ToolCur = IDC_SELECT;
}

// Load editor behavior options from the settings
void WinEditor::LoadEditorSetupFromSettings()
{
   m_alwaysDrawDragPoints = g_app->m_settings.GetEditor_ShowDragPoints();
   m_alwaysDrawLightCenters = g_app->m_settings.GetEditor_DrawLightCenters();
   m_gridSize = g_app->m_settings.GetEditor_GridSize();

   const bool autoSave = g_app->m_settings.GetEditor_AutoSaveOn();
   if (autoSave)
   {
      m_autosaveTime = g_app->m_settings.GetEditor_AutoSaveTime();
      SetAutoSaveMinutes(m_autosaveTime);
   }
   else
      m_autosaveTime = -1;

   m_elemSelectColor = g_app->m_settings.GetEditor_ElementSelectColor();
   m_elemSelectLockedColor = g_app->m_settings.GetEditor_ElementSelectLockedColor();
   m_backgroundColor = g_app->m_settings.GetEditor_BackGroundColor();
   m_fillColor = g_app->m_settings.GetEditor_FillColor();

   m_recentTableList.clear();
   // get the list of the last n loaded tables
   for (int i = 0; i < LAST_OPENED_TABLE_COUNT; i++)
   {
      string szTableName = g_app->m_settings.GetRecentDir_TableFileName(i);
      if (!szTableName.empty())
         m_recentTableList.emplace_back(szTableName);
   }

   m_convertToUnit = g_app->m_settings.GetEditor_Units();
}

void WinEditor::SetClipboard(vector<IStream*> * const pvstm)
{
   for (size_t i = 0; i < m_vstmclipboard.size(); i++)
      m_vstmclipboard[i]->Release();
   m_vstmclipboard.clear();

   if (pvstm)
      for (size_t i = 0; i < pvstm->size(); i++)
         m_vstmclipboard.push_back((*pvstm)[i]);
}

void WinEditor::SetCursorCur(HINSTANCE hInstance, LPCTSTR lpCursorName)
{
#ifndef __STANDALONE__
   const HCURSOR hcursor = LoadCursor(hInstance, lpCursorName);
   SetCursor(hcursor);
#endif
}

void WinEditor::SetActionCur(const string& szaction)
{
#ifndef __STANDALONE__
   ::SendMessage(m_hwndStatusBar, SB_SETTEXT, 3 | 0, (size_t)szaction.c_str());
#endif
}

void WinEditor::SetStatusBarElementInfo(const string& info)
{
#ifndef __STANDALONE__
   ::SendMessage(m_hwndStatusBar, SB_SETTEXT, 4 | 0, (size_t)info.c_str());
#endif
}

bool WinEditor::OpenFileDialog(const string& initDir, vector<string>& filename, const char* const fileFilter, const char* const defaultExt, const DWORD flags, const string& windowTitle) //!! use this all over the place and move to some standard header
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

bool WinEditor::SaveFileDialog(const string& initDir, vector<string>& filename, const char* const fileFilter, const char* const defaultExt, const DWORD flags, const string& windowTitle) //!! use this all over the place and move to some standard header
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

CDockProperty *WinEditor::GetPropertiesDocker()
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

CDockToolbar *WinEditor::GetToolbarDocker()
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

void WinEditor::ResetAllDockers()
{
#ifndef __STANDALONE__
   const bool createNotes = m_dockNotes != nullptr;
   CloseAllDockers();
   // FIXME these are Windows only registry key. Move to g_app->m_settings. ?
   // DeleteSubKey("Editor\\Dock Windows"s); // Old Win32xx
   // DeleteSubKey("Editor\\Dock Settings"s);// Win32xx 9+
   CreateDocker();
   if (createNotes)
      GetDefaultNotesDocker();
#endif
}

CDockNotes* WinEditor::GetDefaultNotesDocker()
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

CDockNotes* WinEditor::GetNotesDocker()
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

CDockLayers *WinEditor::GetLayersDocker()
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

void WinEditor::CreateDocker()
{
#ifndef __STANDALONE__
   LoadDockRegistrySettings(DOCKER_REGISTRY_KEY);
   GetPropertiesDocker()->GetContainer()->SetHideSingleTab(TRUE);
   GetLayersDocker()->GetContainer()->SetHideSingleTab(TRUE);
   GetToolbarDocker()->GetContainer()->SetHideSingleTab(TRUE);
#endif
}

void WinEditor::SetPosCur(float x, float y)
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

void WinEditor::SetObjectPosCur(float x, float y)
{
   char szT[256];
   sprintf_s(szT, sizeof(szT), "%.4f, %.4f", x, y);
#ifndef __STANDALONE__
   ::SendMessage(m_hwndStatusBar, SB_SETTEXT, 1 | 0, (size_t)szT);
#endif
}

void WinEditor::ClearObjectPosCur()
{
#ifndef __STANDALONE__
   ::SendMessage(m_hwndStatusBar, SB_SETTEXT, 1 | 0, (size_t)"");
#endif
}

float WinEditor::ConvertToUnit(const float value) const
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

void WinEditor::SetPropSel(VectorProtected<ISelect> &pvsel)
{
#ifndef __STANDALONE__
   if (m_propertyDialog && m_propertyDialog->IsWindow())
      m_propertyDialog->UpdateTabs(pvsel);
   if (const auto pt = GetActiveTableEditor(); pt && !g_pplayer)
      pt->SetFocus();
#endif
}

void WinEditor::RenameEditable(IEditable *editable, const string &name)
{
   const string oldName = MakeString(editable->GetScriptable()->m_wzName);
   editable->SetName(name);

#ifndef __STANDALONE__
   PinTable *const pt = editable->GetPTable();
   g_pvp->SetPropSel(pt->m_vmultisel);
   g_pvp->GetLayersListDialog()->Update();

   if (editable->GetItemType() == eItemSurface && g_pvp->MessageBox("Replace the name also in all table elements that use this surface?", "Replace", MB_ICONQUESTION | MB_YESNO) == IDYES)
   {
      for (IEditable *const pedit : pt->GetParts())
      {
         if (pedit->GetItemType() == ItemTypeEnum::eItemBumper && ((Bumper *)pedit)->m_d.m_szSurface == oldName)
            ((Bumper *)pedit)->m_d.m_szSurface = name;
         else if (pedit->GetItemType() == ItemTypeEnum::eItemDecal && ((Decal *)pedit)->m_d.m_szSurface == oldName)
            ((Decal *)pedit)->m_d.m_szSurface = name;
         else if (pedit->GetItemType() == ItemTypeEnum::eItemFlipper && ((Flipper *)pedit)->m_d.m_szSurface == oldName)
            ((Flipper *)pedit)->m_d.m_szSurface = name;
         else if (pedit->GetItemType() == ItemTypeEnum::eItemGate && ((Gate *)pedit)->m_d.m_szSurface == oldName)
            ((Gate *)pedit)->m_d.m_szSurface = name;
         else if (pedit->GetItemType() == ItemTypeEnum::eItemKicker && ((Kicker *)pedit)->m_d.m_szSurface == oldName)
            ((Kicker *)pedit)->m_d.m_szSurface = name;
         else if (pedit->GetItemType() == ItemTypeEnum::eItemLight && ((Light *)pedit)->m_d.m_szSurface == oldName)
            ((Light *)pedit)->m_d.m_szSurface = name;
         else if (pedit->GetItemType() == ItemTypeEnum::eItemPlunger && ((Plunger *)pedit)->m_d.m_szSurface == oldName)
            ((Plunger *)pedit)->m_d.m_szSurface = name;
         else if (pedit->GetItemType() == ItemTypeEnum::eItemSpinner && ((Spinner *)pedit)->m_d.m_szSurface == oldName)
            ((Spinner *)pedit)->m_d.m_szSurface = name;
         else if (pedit->GetItemType() == ItemTypeEnum::eItemTrigger && ((Trigger *)pedit)->m_d.m_szSurface == oldName)
            ((Trigger *)pedit)->m_d.m_szSurface = name;
      }
   }
#endif
}

CMenu WinEditor::GetMainMenu(int id)
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

bool WinEditor::ParseCommand(const size_t code, const bool notify)
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
      //DialogBoxParam(m_instance, MAKEINTRESOURCE(IDD_DRAWING_ORDER), m_hwnd, DrawingOrderProc, 0);
      ShowDrawingOrderDialog(false);
      return true;
   }
   case ID_EDIT_DRAWINGORDER_SELECT:
   {
      //DialogBoxParam(m_instance, MAKEINTRESOURCE(IDD_DRAWING_ORDER), m_hwnd, DrawingOrderProc, 0);
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
      OpenRecentFile(code);
      return true;

   case IDM_OPEN:
      LoadFile(true);
      return true;

   case IDM_CLOSE:
   case SC_CLOSE:
      if (const auto ptCur = GetActiveTableEditor(); ptCur)
         CloseTable(ptCur);
      return true;

   case IDC_COPY:
      CopyPasteElement(COPY);
      return true;

   case IDC_PASTE:
      CopyPasteElement(PASTE);
      return true;

   case IDC_PASTEAT:
      CopyPasteElement(PASTE_AT);
      return true;

   case ID_EDIT_UNDO:
      if (CComObject<PinTable> *const ptCur = GetActiveTable(); ptCur)
         ptCur->Undo();
      return true;

   case ID_FILE_EXPORT_BLUEPRINT:
      if (const auto pt = GetActiveTableEditor(); pt)
         pt->ExportBlueprint();
      return true;

   case ID_EXPORT_TABLEMESH:
      if (CComObject<PinTable> *const ptCur = GetActiveTable(); ptCur)
         ptCur->ExportTableMesh();
      return true;

   case ID_IMPORT_BACKDROPPOV:
      if (CComObject<PinTable> *const ptCur = GetActiveTable(); ptCur)
         ptCur->ImportBackdropPOV(string());
      return true;

   case ID_EXPORT_BACKDROPPOV:
      if (CComObject<PinTable> *const ptCur = GetActiveTable(); ptCur)
         ptCur->ExportBackdropPOV();
      return true;

   case ID_FILE_EXIT:
      PostMessage(WM_CLOSE, 0, 0);
      return true;

   case ID_EDIT_PHYSICSOPTIONS:
      m_physicsOptDialog.DoModal(GetHwnd());
      return true;

   case ID_EDIT_EDITOROPTIONS:
      m_editorOptDialog.DoModal(GetHwnd());
      // refresh editor options that we may have changed
      LoadEditorSetupFromSettings();
      // force a screen refresh (it an active table is loaded)
      if (CComObject<PinTable> *const ptCur = GetActiveTable(); ptCur)
         ptCur->SetDirtyDraw();
      return true;

   case ID_EDIT_PLAYEROPTIONS:
   {
      PlayerOptionsDialog playerOptsgDlg;
      playerOptsgDlg.DoModal();
      return true;
   }

   case ID_TABLE_TABLEINFO:
      if (CComObject<PinTable> *const ptCur = GetActiveTable(); ptCur)
         m_tableInfoDialog.DoModal(GetHwnd());
      return true;

   case IDM_IMAGE_EDITOR:
   case ID_TABLE_IMAGEMANAGER:
      if (CComObject<PinTable> *const ptCur = GetActiveTable(); ptCur)
         ShowSubDialog(m_imageMngDlg, true);
      return true;

   case IDM_SOUND_EDITOR:
   case ID_TABLE_SOUNDMANAGER:
      if (CComObject<PinTable> *const ptCur = GetActiveTable(); ptCur)
         ShowSubDialog(m_soundMngDlg, true);
      return true;

   case IDM_MATERIAL_EDITOR:
   case ID_TABLE_MATERIALMANAGER:
      if (CComObject<PinTable> *const ptCur = GetActiveTable(); ptCur)
         ShowSubDialog(m_materialDialog, true);
      return true;

   case ID_TABLE_NOTES:
      if (GetNotesDocker() == nullptr || !GetNotesDocker()->IsWindow())
         GetDefaultNotesDocker();
      else
         GetNotesDocker()->ShowWindow();
      return true;

   case ID_TABLE_FONTMANAGER:
      if (CComObject<PinTable> *const ptCur = GetActiveTable(); ptCur)
         /*const DWORD foo =*/ DialogBoxParam(m_instance, MAKEINTRESOURCE(IDD_FONTDIALOG), GetHwnd(), FontManagerProc, (size_t)ptCur);
      return true;

   case ID_TABLE_DIMENSIONMANAGER:
      ShowSubDialog(m_dimensionDialog, true);
      return true;

   case IDM_COLLECTION_EDITOR:
   case ID_TABLE_COLLECTIONMANAGER:
      if (CComObject<PinTable> *const ptCur = GetActiveTable(); ptCur)
         ShowSubDialog(m_collectionMngDlg, true);
      return true;

   case ID_TABLE_RENDERPROBEMANAGER:
      ShowSubDialog(m_renderProbeDialog, true);
      return true;

   case ID_PREFERENCES_SECURITYOPTIONS:
   {
      DialogBoxParam(m_instance, MAKEINTRESOURCE(IDD_SECURITY_OPTIONS), GetHwnd(), SecurityOptionsProc, 0);
      // refresh editor options that we may have changed
      LoadEditorSetupFromSettings();
      return true;
   }

   case ID_HELP_ABOUT:
      ShowSubDialog(m_aboutDialog, true);
      return true;

   case ID_WINDOW_CASCADE:
      MDICascade();
      return true;

   case ID_WINDOW_TILE:
      MDITile();
      return true;

   case ID_WINDOW_ARRANGEICONS:
      MDIIconArrange();
      return true;
   }
#endif
   return false;
}

void WinEditor::ToggleToolbar()
{
#ifndef __STANDALONE__
   if (m_toolbarDialog)
      m_toolbarDialog->EnableButtons();
#endif
}

void WinEditor::DoPlay(const int playMode)
{
   if (g_pplayer)
      return; // Can't play twice

   PinTableWnd *const tableEditor = GetActiveTableEditor();
   CComObject<PinTable> *const table = GetActiveTable();
   if (tableEditor == nullptr || table == nullptr)
   {
      m_table_played_via_SelectTableOnStart = false;
      return;
   }

   PLOGI << "Starting Play mode [table: " << table->m_tableName << ", play mode: " << playMode << ']';
   bool initError = false;

   // Create the player on a (shallow) copy of the table, that will be animated by the script, animations, ...
   PinTable *live_table = table->CopyForPlay();
   if (live_table == nullptr)
   {
      m_table_played_via_SelectTableOnStart = false;
      return;
   }
   switch (playMode)
   {
   case 0: new Player(live_table, Player::PlayMode::Play); break;
   case 1: new Player(live_table, Player::PlayMode::EditPOV); break;
   case 2: new Player(live_table, Player::PlayMode::LiveEdit); break;
   default: assert(false); break;
   }

   if (g_pplayer == nullptr)
   {
      m_table_played_via_SelectTableOnStart = false;
      return;
   }

#ifndef __STANDALONE__
   tableEditor->EndAutoSaveCounter();
   GetPropertiesDocker()->EnableWindow(FALSE);
   GetLayersDocker()->EnableWindow(FALSE);
   GetToolbarDocker()->EnableWindow(FALSE);
   if (GetNotesDocker() != nullptr)
      GetNotesDocker()->EnableWindow(FALSE);
   if (const auto pt = GetActiveTableEditor(); pt)
      pt->EnableWindow(FALSE);
#endif

   // Switch to Player's main loop (needed to avoid interference between editor's Window Msg loop and player's specific msg loop, also Player has a fairly specific msg loop)
   g_pplayer->GameLoop();
   delete g_pplayer;
   assert(g_pplayer == nullptr);

   // The table settings may have been edited during play (camera, rendering, ...), so copy them back to the editor table's settings
   table->m_settings.Load(live_table->m_settings);
   table->m_settings.SetModified(live_table->m_settings.IsModified());
   live_table->Release();

#ifndef __STANDALONE__
   GetPropertiesDocker()->EnableWindow();
   GetLayersDocker()->EnableWindow();
   GetToolbarDocker()->EnableWindow();
   if (GetNotesDocker() != nullptr)
      GetNotesDocker()->EnableWindow();
   ToggleToolbar();
   ShowWindow(SW_SHOW);
   SetForegroundWindow();

   table->SetDirtyDraw();
   table->RefreshProperties();
   tableEditor->BeginAutoSaveCounter();
   tableEditor->EnableWindow();
   tableEditor->SetFocus();
   tableEditor->SetActiveWindow();
#endif

   // If the table was played via the "Select Table on Start" option, then close the table and propose to load another one
   if (m_table_played_via_SelectTableOnStart)
   {
      CloseTable(tableEditor);
      m_table_played_via_SelectTableOnStart = LoadFile(false);
      if (m_table_played_via_SelectTableOnStart)
         DoPlay(0);
   }
}

bool WinEditor::LoadFile(const bool updateEditor, VPXFileFeedback* feedback)
{
   const string& szInitialDir = g_app->m_settings.GetRecentDir_LoadDir();

   vector<string> filename;
   if (!OpenFileDialog(szInitialDir, filename, "Visual Pinball Tables (*.vpx)\0*.vpx\0Old Visual Pinball Tables(*.vpt)\0*.vpt\0", "vpx", 0,
          !updateEditor ? "Select a Table to Play or press Cancel to enter Editor-Mode"s : string()))
      return false;

   const size_t index = filename[0].find_last_of(PATH_SEPARATOR_CHAR);
   if (index != string::npos)
      g_app->m_settings.SetRecentDir_LoadDir(filename[0].substr(0, index), false);

   LoadFileName(filename[0], updateEditor, feedback);

   return true;
}

void WinEditor::LoadFileName(const string& filename, const bool updateEditor, VPXFileFeedback* feedback)
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

   CloseAllDialogs();

   PinTableMDI * const mdiTable = new PinTableMDI(this);
   PinTableWnd *const ppt = mdiTable->GetTableWnd();
   const HRESULT hr = feedback != nullptr ? ppt->m_table->LoadGameFromFilename(filename, *feedback) : ppt->m_table->LoadGameFromFilename(filename);

   const bool hashing_error = (hr == APPX_E_BLOCK_HASH_INVALID || hr == APPX_E_CORRUPT_CONTENT);
   if (hashing_error)
      ShowError(LocalString(IDS_CORRUPTFILE).m_szbuffer);

   if (!SUCCEEDED(hr) && !hashing_error)
   {
      ShowError("This file does not exist, or is corrupt and failed to load.");

      delete mdiTable;
   }
   else
   {
      m_vtable.push_back(ppt);

#ifdef __STANDALONE__
      m_ptableActive = ppt->m_table;
#endif

      AddMDIChild(mdiTable);

      // make sure the load directory is the active directory
      const string tablePath = PathFromFilename(filename);
      SetCurrentDirectory(tablePath.c_str());

      PLOGI << "UI Post Load Start";

      g_app->m_settings.SetRecentDir_LoadDir(tablePath, false);
      UpdateRecentFileList(filename);

      ppt->m_table->AddMultiSel(ppt->m_table, false, true, false);
      if (updateEditor)
      {
#ifndef __STANDALONE__
         GetLayersListDialog()->ResetView();
         ToggleToolbar();
         if (m_dockNotes != nullptr)
            m_dockNotes->Enable();

         SetFocus();

         const string &audit = ppt->m_table->AuditTable(true);
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

PinTableWnd* WinEditor::GetActiveTableEditor()
{
   if (const auto mdiTable = (PinTableMDI *)GetActiveMDIChild(); mdiTable && !m_unloadingTable)
      return mdiTable->GetTableWnd();
   return nullptr;
}

CComObject<PinTable>* WinEditor::GetActiveTable()
{
   if (const auto mdiTable = (PinTableMDI *)GetActiveMDIChild(); mdiTable && !m_unloadingTable && mdiTable->GetTableWnd())
      return mdiTable->GetTableWnd()->m_table;
   return nullptr;
}

bool WinEditor::CanClose()
{
   while (!m_vtable.empty())
   {
      if (!m_vtable[0]->GetMDITable()->CanClose())
         return false;

      CloseTable(m_vtable[0]);
   }

   return true;
}

void WinEditor::CloseTable(PinTableWnd * ppt)
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
    PinTableMDI* mdiTable = ppt->GetMDITable();
    if (mdiTable)
        RemoveMDIChild(mdiTable);

    RemoveFromVectorSingle(m_vtable, ppt);
#endif
}

void WinEditor::SetEnableMenuItems()
{
#ifndef __STANDALONE__
   const auto editor = GetActiveTableEditor();

   // Set menu item to the correct state
   const CMenu mainMenu = GetMenu();

   mainMenu.CheckMenuItem(ID_EDIT_BACKGLASSVIEW, MF_BYCOMMAND | (m_backglassView ? MF_CHECKED : MF_UNCHECKED));

   // is there a valid table??
   constexpr UINT grayed = MF_BYCOMMAND | MF_GRAYED, enabled = MF_BYCOMMAND | MF_ENABLED;
   if (editor)
   {
      CComObject<PinTable> *const ptCur = editor->m_table;
      mainMenu.CheckMenuItem(ID_EDIT_SCRIPT, MF_BYCOMMAND | ((editor->m_pcv != nullptr && editor->m_pcv->m_visible && !editor->m_pcv->m_minimized) ? MF_CHECKED : MF_UNCHECKED));

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

      mainMenu.CheckMenuItem(ID_VIEW_GRID, MF_BYCOMMAND | (editor->GetDisplayGrid() ? MF_CHECKED : MF_UNCHECKED));
      mainMenu.CheckMenuItem(ID_VIEW_BACKDROP, MF_BYCOMMAND | (editor->GetDisplayBackdrop() ? MF_CHECKED : MF_UNCHECKED));
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

void WinEditor::UpdateRecentFileList(const std::filesystem::path &filename)
{
   const size_t old_count = m_recentTableList.size();

   // if the loaded file name is a valid one then add it to the top of the list
   if (!filename.empty())
   {
      vector<string> newList;
      newList.push_back(filename.string());

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
         g_app->m_settings.SetRecentDir_TableFileName(i, tableName, false);

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
         const string recentMenuname = '&' + std::to_string(i+1) + "  " + string_replace_all(m_recentTableList[i], '&', "&&"s);

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

STDMETHODIMP WinEditor::QueryInterface(REFIID iid, void **ppvObjOut)
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


STDMETHODIMP_(ULONG) WinEditor::AddRef()
{
   //!! ?? ASSERT(m_cref, "bad m_cref");
   return ++m_cref;
}

STDMETHODIMP_(ULONG) WinEditor::Release()
{
   assert(m_cref);
   m_cref--; // FIXME we are doing refcounting without memory management

   return m_cref;
}

void WinEditor::PreCreate(CREATESTRUCT& cs)
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

void WinEditor::PreRegisterClass(WNDCLASS& wc)
{
#ifndef __STANDALONE__
   wc.hIcon = LoadIcon(m_instance, MAKEINTRESOURCE(IDI_VPINBALL));
   wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
#endif
   wc.style = CS_DBLCLKS; //CS_NOCLOSE | CS_OWNDC;
   wc.lpszClassName = _T("VPinball");
   wc.lpszMenuName = _T("IDR_APPMENU");
}

void WinEditor::OnClose()
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
         g_app->m_settings.SetEditor_WindowLeft((int)winpl.rcNormalPosition.left, false);
         g_app->m_settings.SetEditor_WindowTop((int)winpl.rcNormalPosition.top, false);
         g_app->m_settings.SetEditor_WindowRight((int)winpl.rcNormalPosition.right, false);
         g_app->m_settings.SetEditor_WindowBottom((int)winpl.rcNormalPosition.bottom, false);
         g_app->m_settings.SetEditor_WindowMaximized(!!IsZoomed(), false);
      }
      if (!IsIconic()) // otherwise the window/dock settings are screwed up and have to be manually restored each time
         SaveDockRegistrySettings(DOCKER_REGISTRY_KEY);

      CWnd::OnClose();
   }
#endif
}

void WinEditor::OnDestroy()
{
#ifndef __STANDALONE__
    PostMessage(WM_QUIT, 0, 0);
#endif
}

void WinEditor::ShowSubDialog(CDialog &dlg, const bool show)
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


int WinEditor::OnCreate(CREATESTRUCT& cs)
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

   SetWindowText(LocalString(IDS_PROJNAME).m_szbuffer);

   CreateDocker();

   return result;
#else
   return 0;
#endif
}

LRESULT WinEditor::OnPaint(UINT msg, WPARAM wparam, LPARAM lparam)
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

// Called when window is initially updated to be able to perform initial setup of the window and its children
void WinEditor::OnInitialUpdate()
{
   PLOGI << "OnInitialUpdate";

   LoadEditorSetupFromSettings();

#ifndef __STANDALONE__
   SetAccelerators(IDR_VPACCEL);

   m_hwndStatusBar = CreateStatusWindow(WS_CHILD | WS_VISIBLE, "", GetHwnd(), 1); // Create Status Line at the bottom

   static constexpr int foo[6] = { 120, 240, 400, 600, 800, 1400 };
   ::SendMessage(m_hwndStatusBar, SB_SETPARTS, 6, (size_t)foo); // Initialize Status bar with 6 empty cells

   SendMessage(WM_SIZE, 0, 0);         // Make our window relay itself out

   const int left = g_app->m_settings.GetEditor_WindowLeft();
   const int top = g_app->m_settings.GetEditor_WindowTop();
   const int right = g_app->m_settings.GetEditor_WindowRight();
   const int bottom = g_app->m_settings.GetEditor_WindowBottom();
   const bool maximized = g_app->m_settings.GetEditor_WindowMaximized();
   if (right > left && bottom > top)
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
      else if (maximized)
         winpl.showCmd |= SW_MAXIMIZE;
      else
         winpl.showCmd |= SW_SHOWNORMAL;

      SetWindowPlacement(winpl);
   }

   ShowWindow(SW_SHOW);

   // Load 'in playing mode' image for UI
   m_hbmInPlayMode = (HBITMAP)LoadImage(m_instance, MAKEINTRESOURCE(IDB_INPLAYMODE), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
#endif

   UpdateRecentFileList(string()); // update the recent loaded file list

   //   InitTools();
   //   SetForegroundWindow();
   SetEnableMenuItems();
}

BOOL WinEditor::OnCommand(WPARAM wparam, LPARAM lparam)
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

#ifndef __STANDALONE__
BOOL WinEditor::PreTranslateMessage(MSG& msg)
{
   if (msg.message >= WM_KEYFIRST && msg.message <= WM_KEYLAST)
   {
      if (msg.message == WM_KEYDOWN && msg.wParam == VK_F2)
      { // Hardcoded F2 accelerator to avoid bypassing F2 as a label edit in layer list tree view (as win32xx processes accelerators first, then PreTranslateMessage following window hierarchy)
         ParseCommand(IDM_SOUND_EDITOR, false);
         return true;
      }
   }
   return false;
}
#endif

LRESULT WinEditor::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
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
         if (const auto ptCur = GetActiveTableEditor(); ptCur)
            ptCur->SetMouseCursor();
      }
      return FinalWindowProc(uMsg, wParam, lParam);
   }
   case WM_TIMER:
   {
      if (const auto ptCur = GetActiveTableEditor(); ptCur)
      {
         switch (wParam)
         {
         case TIMER_ID_AUTOSAVE:
            ptCur->AutoSave();
            break;

         case TIMER_ID_CLOSE_TABLE:
            KillTimer(TIMER_ID_CLOSE_TABLE);
            CloseTable(ptCur);
            return 0;
         }
         return FinalWindowProc(uMsg, wParam, lParam);
      }
      break;
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
   return 0;
#endif
}

// See http://mjrnet.org/pinscape/PinSimFrontEndControls/PinSimFrontEndControls.htm
LRESULT WinEditor::OnFrontEndControlsMsg(WPARAM wParam, LPARAM lParam)
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
                         && cls == "#32770"s)
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
         g_pplayer->m_playfieldWnd->RaiseAndFocus();

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

LRESULT WinEditor::OnMDIActivated(UINT msg, WPARAM wparam, LPARAM lparam)
{
#ifndef __STANDALONE__
   if (m_dockNotes != nullptr)
      m_dockNotes->Refresh();
   return CMDIFrameT::OnMDIActivated(msg, wparam, lparam);
#else 
   return 0;
#endif
}

LRESULT WinEditor::OnMDIDestroyed(UINT msg, WPARAM wparam, LPARAM lparam)
{
#ifndef __STANDALONE__
   if (GetAllMDIChildren().size() == 1)
      GetLayersListDialog()->SetActiveTable(nullptr);
   return CMDIFrameT::OnMDIDestroyed(msg, wparam, lparam);
#else
   return 0;
#endif
}

#ifndef __STANDALONE__
Win32xx::DockPtr WinEditor::NewDockerFromID(int id)
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

STDMETHODIMP WinEditor::PlaySound(BSTR bstr)
{
   if (g_pplayer) g_pplayer->m_ptable->PlaySound(bstr, 0, 1.f, 0.f, 0.f, 0, VARIANT_FALSE, VARIANT_TRUE, 0.f);

   return S_OK;
}

STDMETHODIMP WinEditor::FireKnocker(int Count)
{
   if (g_pplayer) g_pplayer->m_ptable->FireKnocker(Count);

   return S_OK;
}

STDMETHODIMP WinEditor::QuitPlayer(int CloseType)
{
   if (g_pplayer)
   {
      g_pplayer->SetCloseState((Player::CloseState)CloseType);
   }
#ifndef __STANDALONE__
   else
      PostMessage(WM_CLOSE, 0, 0);
#endif

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
      return ( lstrcmpi(buf1, buf2));
   else
      return (-lstrcmpi(buf1, buf2));
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

   char buf1[64] = {};
   char buf2[64] = {};
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

      int security = g_app->m_settings.GetPlayer_SecurityLevel();
      if (security < 0 || security > 4)
         security = 0;

      const int buttonid = rgDlgIDFromSecurityLevel[security];

      SendMessage(GetDlgItem(hwndDlg, buttonid), BM_SETCHECK, BST_CHECKED, 0);

      const bool hangdetect = g_app->m_settings.GetPlayer_DetectHang();
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
                  g_app->m_settings.SetPlayer_SecurityLevel(i, false);
            }

            const bool hangdetect = (SendMessage(GetDlgItem(hwndDlg, IDC_HANGDETECT), BM_GETCHECK, 0, 0) != 0);
            g_app->m_settings.SetPlayer_DetectHang(hangdetect, false);

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
            const string& szInitialDir = g_app->m_settings.GetRecentDir_FontDir();
            vector<string> filename;
            if (g_pvp->OpenFileDialog(szInitialDir, filename, "Font Files (*.ttf)\0*.ttf\0", "ttf", 0))
            {
               const size_t index = filename[0].find_last_of(PATH_SEPARATOR_CHAR);
               if (index != string::npos)
                  g_app->m_settings.SetRecentDir_FontDir(filename[0].substr(0, index), false);

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


void WinEditor::ShowDrawingOrderDialog(bool select)
{
#ifndef __STANDALONE__
   DrawingOrderDialog orderDlg(select);
   orderDlg.DoModal();
#endif
}

void WinEditor::CloseAllDialogs()
{
#ifndef __STANDALONE__
   if (m_imageMngDlg.IsWindow())
      m_imageMngDlg.Destroy();
   if (m_soundMngDlg.IsWindow())
      m_soundMngDlg.Destroy();
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
#endif
}

void WinEditor::ToggleBackglassView()
{
   const bool show = !m_backglassView;
   m_backglassView = show;

   for (const auto ptT : m_vtable)
   {
      ptT->SetDefaultView();
      ptT->m_table->SetDirtyDraw();
   }

   CComObject<PinTable> * const ptCur = GetActiveTable();
   if (ptCur)
      // Set selection to something in the new view (unless hiding table elements)
      ptCur->AddMultiSel((ISelect *)ptCur, false, true, false);

   ToggleToolbar();
}

void WinEditor::ToggleScriptEditor()
{
#ifndef __STANDALONE__
   const auto editor = GetActiveTableEditor();
   if (editor)
   {
      const bool alwaysViewScript = g_app->m_settings.GetEditor_AlwaysViewScript();
      editor->m_pcv->SetVisible(alwaysViewScript || !(editor->m_pcv->m_visible && !editor->m_pcv->m_minimized));
      //SendMessage(m_hwndToolbarMain, TB_CHECKBUTTON, ID_EDIT_SCRIPT, MAKELONG(editor->m_pcv->m_visible && !editor->m_pcv->m_minimized, 0));
   }
#endif
}

void WinEditor::ShowSearchSelect()
{
#ifndef __STANDALONE__
   if (PinTableWnd *const ptCur = GetActiveTableEditor(); ptCur)
      ptCur->ShowSearchSelectDlg();
#endif
}

void WinEditor::SetDefaultPhysics()
{
   CComObject<PinTable> * const ptCur = GetActiveTable();
   if (ptCur)
   {
      const int answ = MessageBox(LocalString(IDS_DEFAULTPHYSICS).m_szbuffer, "Continue?", MB_YESNO | MB_ICONWARNING);
      if (answ == IDYES)
      {
         ptCur->BeginUndo();
         for (int i = 0; i < ptCur->m_vmultisel.size(); i++)
            ptCur->m_vmultisel[i].SetDefaultPhysics(true);
         ptCur->EndUndo();
      }
   }
}

void WinEditor::SetViewSolidOutline(size_t viewId)
{
#ifndef __STANDALONE__
   CComObject<PinTable> * const ptCur = GetActiveTable();
   if (ptCur)
   {
      ptCur->m_renderSolid = (viewId == ID_VIEW_SOLID);
      GetMenu().CheckMenuItem(ID_VIEW_SOLID, MF_BYCOMMAND | (ptCur->RenderSolid() ? MF_CHECKED : MF_UNCHECKED));
      GetMenu().CheckMenuItem(ID_VIEW_OUTLINE, MF_BYCOMMAND | (ptCur->RenderSolid() ? MF_UNCHECKED : MF_CHECKED));

      ptCur->SetDirtyDraw();
      g_app->m_settings.SetEditor_RenderSolid(ptCur->m_renderSolid, false);
   }
#endif
}

void WinEditor::ShowGridView()
{
#ifndef __STANDALONE__
   auto ptCur = GetActiveTableEditor();
   if (ptCur)
   {
      ptCur->SetDisplayGrid(!ptCur->GetDisplayGrid());
      GetMenu().CheckMenuItem(ID_VIEW_GRID, MF_BYCOMMAND | (ptCur->GetDisplayGrid() ? MF_CHECKED : MF_UNCHECKED));
   }
#endif
}

void WinEditor::ShowBackdropView()
{
#ifndef __STANDALONE__
   if (const auto ptCur = GetActiveTableEditor(); ptCur)
   {
      ptCur->SetDisplayBackdrop(!ptCur->GetDisplayBackdrop());
      GetMenu().CheckMenuItem(ID_VIEW_BACKDROP, MF_BYCOMMAND | (ptCur->GetDisplayBackdrop() ? MF_CHECKED : MF_UNCHECKED));
   }
#endif
}

void WinEditor::AddControlPoint()
{
   const auto ptCur = GetActiveTableEditor();
   if (ptCur == nullptr)
      return;

   if (!ptCur->m_table->m_vmultisel.empty())
   {
      ISelect * const psel = ptCur->m_table->m_vmultisel.ElementAt(0);
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

void WinEditor::AddSmoothControlPoint()
{
   const auto ptCur = GetActiveTableEditor();
   if (ptCur == nullptr)
      return;

   if (!ptCur->m_table->m_vmultisel.empty())
   {
      ISelect *const psel = ptCur->m_table->m_vmultisel.ElementAt(0);
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

void WinEditor::SaveTable(const bool saveAs)
{
#ifndef __STANDALONE__
   CComObject<PinTable> *const ptCur = GetActiveTable();
   if (ptCur == nullptr)
      return;
   
   HRESULT hr;
   if (saveAs || ptCur->m_filename.empty())
   {
      //need to get a file name
      OPENFILENAME ofn = {};
      ofn.lStructSize = sizeof(OPENFILENAME);
      ofn.hInstance = g_app->GetInstanceHandle();
      ofn.hwndOwner = GetHwnd();
      // TEXT
      ofn.lpstrFilter = "Visual Pinball Tables (*.vpx)\0*.vpx\0";

      std::filesystem::path vpxPath = ptCur->m_filename;
      vpxPath.replace_extension(".vpx");
      char fileName[MAXSTRING];
      strncpy_s(fileName, sizeof(fileName), vpxPath.string().c_str());
      ofn.lpstrFile = fileName;
      ofn.nMaxFile = sizeof(fileName);
      ofn.lpstrDefExt = "vpx";
      ofn.Flags = OFN_NOREADONLYRETURN | OFN_CREATEPROMPT | OFN_OVERWRITEPROMPT | OFN_EXPLORER;

      {
         string szInitialDir;
         // First, use dir of current table
         std::filesystem::path currentTablePath = ptCur->m_filename.parent_path();
         if (!currentTablePath.empty())
            szInitialDir = currentTablePath.string();
         // Or try with the standard last-used dir
         else
         {
            Settings::SetRecentDir_LoadDir_Default(g_app->m_fileLocator.GetAppPath(FileLocator::AppSubFolder::Tables).string() + PATH_SEPARATOR_CHAR);
            szInitialDir = ptCur->m_settings.GetRecentDir_LoadDir();
         }
         ofn.lpstrInitialDir = szInitialDir.c_str();

         const int ret = GetSaveFileName(&ofn);
         // user cancelled
         if (ret == 0)
            return;
      }

      // assign user selected file name as new internal filename, and save as new default
      ptCur->m_filename = fileName;
      ptCur->m_title = TitleFromFilename(ptCur->m_filename.filename().string());
      g_app->m_settings.SetRecentDir_LoadDir(ptCur->m_filename.parent_path().string(), false); // truncate after folder(s)
      SetCaption(ptCur->m_title.c_str());
   }

   hr = ptCur->Save();
   if (hr == S_OK)
      UpdateRecentFileList(ptCur->m_filename);
#endif
}

void WinEditor::OpenNewTable(size_t tableId)
{
   if (m_vtable.size() == MAX_OPEN_TABLES)
   {
      ShowError("Maximum amount of tables already loaded and open.");
      return;
   }

#ifndef __STANDALONE__
   PinTableMDI * const mdiTable = new PinTableMDI(this);
   CComObject<PinTable> *const ppt = mdiTable->GetTable();
   string path;
   switch (tableId)
   {
   case ID_NEW_EXAMPLETABLE: path = "exampleTable.vpx"s; break;
   case ID_NEW_STRIPPEDTABLE: path = "strippedTable.vpx"s; break;
   case ID_NEW_LIGHTSEQTABLE: path = "lightSeqTable.vpx"s; break;
   case ID_NEW_BLANKTABLE:
   default: path = "blankTable.vpx"s;
   }
   ppt->m_glassTopHeight = ppt->m_glassBottomHeight = 210;
   for (int i = 0; i < 16; i++)
      ppt->m_rgcolorcustom[i] = RGB(0, 0, 0);
   ppt->LoadGameFromFilename(g_app->m_fileLocator.GetAppPath(FileLocator::AppSubFolder::Assets, path).string());
   ppt->m_title = LocalString(IDS_TABLE).m_szbuffer /*"Table"*/ + std::to_string(m_NextTableID);
   m_NextTableID++;
   ppt->m_settings.SetIniPath(std::filesystem::path());
   ppt->m_filename.clear();

   m_vtable.push_back(mdiTable->GetTableWnd());
   AddMDIChild(mdiTable);
   mdiTable->GetTable()->AddMultiSel(mdiTable->GetTable(), false, true, false);
   GetLayersListDialog()->ResetView();
   ToggleToolbar();
   if (m_dockNotes != nullptr)
      m_dockNotes->Enable();

   SetFocus();
#endif
}

void WinEditor::ProcessDeleteElement()
{
   CComObject<PinTable> * const ptCur = GetActiveTable();
   if (ptCur)
      ptCur->OnDelete();
}

void WinEditor::OpenRecentFile(const size_t menuId)
{
   // get the index into the recent list menu
   const size_t Index = menuId - RECENT_FIRST_MENU_IDM;
   // copy it into a temporary string so it can be correctly processed
   LoadFileName(m_recentTableList[Index], true);
}

void WinEditor::CopyPasteElement(const CopyPasteModes mode)
{
   const auto ptCur = GetActiveTableEditor();
   if (ptCur && !ptCur->m_table->IsLocked())
   {
      const POINT ptCursor = ptCur->GetScreenPoint();
      switch (mode)
      {
      case COPY:
      {
         ptCur->m_table->Copy(ptCursor.x, ptCursor.y);
         break;
      }
      case PASTE:
      {
         ptCur->m_table->Paste(false, ptCursor.x, ptCursor.y);
         break;
      }
      case PASTE_AT:
      {
         ptCur->m_table->Paste(true, ptCursor.x, ptCursor.y);
         break;
      }
      default:
         break;
      }
   }
}
