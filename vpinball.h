// VPinball.h: interface for the VPinball class.
//
//////////////////////////////////////////////////////////////////////
#pragma once
#if !defined(AFX_VPINBALL_H__4D32616D_55B5_4FE0_87D9_3D4CB0BE3C76__INCLUDED_)
#define AFX_VPINBALL_H__4D32616D_55B5_4FE0_87D9_3D4CB0BE3C76__INCLUDED_

#include <wxx_dockframe.h>
#include "RenderDevice.h"
#include "ImageDialog.h"
#include "SoundDialog.h"
#include "EditorOptionsDialog.h"
#include "VideoOptionsDialog.h"
#include "AudioOptionsDialog.h"
#include "CollectionManagerDialog.h"
#include "PhysicsOptionsDialog.h"
#include "TableInfoDialog.h"
#include "DimensionDialog.h"
#include "MaterialDialog.h"
#include "SoundDialog.h"
#include "AboutDialog.h"
#include "DrawingOrderDialog.h"
#include "ToolbarDialog.h"
#include "LayersListDialog.h"
#include "NotesDialog.h"
#include "Properties/PropertyDialog.h"

#define CURRENT_FILE_FORMAT_VERSION 1060
#define NO_ENCRYPTION_FORMAT_VERSION 1050
#define NEW_SOUND_FORMAT_VERSION 1031 // introduced surround option

#define DEFAULT_SECURITY_LEVEL 0

#define LAST_OPENED_TABLE_COUNT	8
#define MAX_CUSTOM_PARAM_INDEX 10


class PinTable;
class PinTableMDI;

class VPinball : public CMDIDockFrame
{
public:
    enum TIMER_IDS
    {
        TIMER_ID_AUTOSAVE = 12345,
        TIMER_ID_CLOSE_TABLE = 12346
    };

    enum CopyPasteModes
    {
        COPY = 0,
        PASTE = 1,
        PASTE_AT = 2
    };
    
   VPinball();
   virtual ~VPinball();
   void Quit();

   void ShowSubDialog(CDialog& dlg);

private:
   void ShowSearchSelect();
   void SetDefaultPhysics();
   void SetViewSolidOutline(size_t viewId);
   void ShowGridView();
   void ShowBackdropView();
   void AddControlPoint();
   void AddSmoothControlPoint();
   void SaveTable(const bool saveAs);
   void OpenNewTable(size_t tableId);
   void ProcessDeleteElement();
   void OpenRecentFile(const size_t menuId);
   void CopyPasteElement(const CopyPasteModes mode);
   void InitTools();
   void InitRegValues();
   bool CanClose();
   void GetMyPath();
   void UpdateRecentFileList(const string& szfilename);

   bool ApcHost_OnTranslateMessage(MSG* pmsg);
   bool processKeyInputForDialogs(MSG *pmsg);


public:
   void AddMDITable(PinTableMDI* mdiTable);
   CMenu GetMainMenu(int id);
   void CloseAllDialogs();
   void ToggleScriptEditor();
   void ToggleBackglassView();
   BOOL ParseCommand(size_t code, size_t notify);
   void ReInitSound();

   CComObject<PinTable> *GetActiveTable();
   bool LoadFile();
   void LoadFileName(const string& szFileName, const bool updateEditor);
   void SetClipboard(vector<IStream*> * const pvstm);

   void DoPlay(const bool _cameraMode);

   void SetPosCur(float x, float y);
   void SetObjectPosCur(float x, float y);
   void ClearObjectPosCur();
   float ConvertToUnit(const float value) const;
   void SetPropSel(VectorProtected<ISelect> &pvsel);

   void SetActionCur(const string& szaction);
   void SetCursorCur(HINSTANCE hInstance, LPCTSTR lpCursorName);

   STDMETHOD(QueryInterface)(REFIID riid, void** ppvObj);
   STDMETHOD_(ULONG, AddRef)();
   STDMETHOD_(ULONG, Release)();

   STDMETHOD(PlaySound)(BSTR bstr);

   STDMETHOD(FireKnocker)(int Count);
   STDMETHOD(QuitPlayer)(int CloseType);

   void MainMsgLoop();

   bool CloseTable(PinTable * const ppt);

   void ToggleToolbar();
   void SetEnableMenuItems();

   void EnsureWorkerThread();
   HANDLE PostWorkToWorkerThread(int workid, LPARAM lParam);

   void SetAutoSaveMinutes(const int minutes);
   static void SetOpenMinimized();
   void ShowDrawingOrderDialog(bool select);

   void SetStatusBarElementInfo(const string& info);
   void SetStatusBarUnitInfo(const string& info, const bool isUnit) // inlined, in the hope that string conversions will be skipped in case of early out in here
   {
    if (g_pplayer)
        return;

    string textBuf;

    if (!info.empty())
    {
       textBuf = info;
       if(isUnit)
       {
           switch(m_convertToUnit)
           {
               case 0:
               {
                   textBuf += " (inch)";
                   break;
               }
               case 1:
               {
                   textBuf += " (mm)";
                   break;
               }
               case 2:
               {
                   textBuf += " (VPUnits)";
                   break;
               }
               default:
                   assert(!"wrong unit");
               break;
           }
       }
    }

    SendMessage(m_hwndStatusBar, SB_SETTEXT, 5 | 0, (size_t)textBuf.c_str());
   }

   bool OpenFileDialog(const char* const initDir, std::vector<std::string>& filename, const char* const fileFilter, const char* const defaultExt, const DWORD flags);
   bool SaveFileDialog(const char* const initDir, std::vector<std::string>& filename, const char* const fileFilter, const char* const defaultExt, const DWORD flags);

   CDockProperty* GetPropertiesDocker();
   CDockToolbar *GetToolbarDocker();
   CDockNotes* GetNotesDocker();
   CDockLayers* GetLayersDocker();
   void CreateDocker();
   LayersListDialog *GetLayersListDialog()
   {
       return m_layersListDialog;
   }
   bool IsClosing() const { return m_closing; }

   ULONG m_cref;

   HINSTANCE theInstance;

   vector< CComObject<PinTable>* > m_vtable;
   CComObject<PinTable> *m_ptableActive;

//    HWND m_hwndSideBar;
//    HWND m_hwndSideBarScroll;
//    HWND m_hwndSideBarLayers;
//    HWND m_hwndWork;
//    HWND m_hwndToolbarMain;
//    HWND m_hwndToolbarPalette;
//    HWND m_hwndToolbarLayers;
   HWND m_hwndStatusBar;

   int m_palettescroll;

   //SmartBrowser m_sb;

   vector<IStream*> m_vstmclipboard;

   AudioMusicPlayer m_ps;

   int m_ToolCur; // Palette button currently pressed

   int m_NextTableID; // counter to create next unique table name

   CodeViewer *m_pcv; // Currently active code window

   bool m_backglassView; // Whether viewing the playfield or screen layout

   bool m_alwaysDrawDragPoints;
   bool m_alwaysDrawLightCenters;
   int m_gridSize;
   int m_convertToUnit; //0=Inches, 1=Millimeters, 2=VPUnits

   int m_securitylevel;

   string m_szMyPath;
   std::wstring m_wzMyPath;
   string m_currentTablePath;

   int m_autosaveTime;
   static bool m_open_minimized;

   Material m_dummyMaterial;
   COLORREF m_elemSelectColor;
   COLORREF m_elemSelectLockedColor;
   COLORREF m_backgroundColor;
   COLORREF m_fillColor;
   Vertex2D m_mouseCursorPosition;

   static WCHAR *m_customParameters[MAX_CUSTOM_PARAM_INDEX];

protected:
   virtual void PreCreate(CREATESTRUCT& cs);
   virtual void PreRegisterClass(WNDCLASS& wc);
   virtual void OnClose();
   virtual void OnDestroy();
   virtual int  OnCreate(CREATESTRUCT& cs);
   virtual LRESULT OnPaint(UINT msg, WPARAM wparam, LPARAM lparam);
   virtual void OnInitialUpdate();
   virtual BOOL OnCommand(WPARAM wparam, LPARAM lparam);
   virtual LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
   virtual LRESULT OnMDIActivated(UINT msg, WPARAM wparam, LPARAM lparam);
   virtual LRESULT OnMDIDestroyed(UINT msg, WPARAM wparam, LPARAM lparam);
   virtual CDocker *NewDockerFromID(int id);

private:

   CDockProperty *GetDefaultPropertiesDocker();
   CDockLayers *GetDefaultLayersDocker();
   CDockToolbar* GetDefaultToolbarDocker();
   CDockNotes* GetDefaultNotesDocker();
   bool m_unloadingTable;
   CMenu m_mainMenu;
   std::vector<std::string> m_recentTableList;
   //char m_szRecentTableList[LAST_OPENED_TABLE_COUNT + 1][MAX_PATH];

   HANDLE m_workerthread;
   unsigned int m_workerthreadid;
   bool    m_closing;
   HMODULE m_scintillaDll;

   ImageDialog m_imageMngDlg;
   SoundDialog m_soundMngDlg;
   AudioOptionsDialog m_audioOptDialog;
   VideoOptionsDialog m_videoOptDialog;
   EditorOptionsDialog m_editorOptDialog;
   CollectionManagerDialog m_collectionMngDlg;
   PhysicsOptionsDialog m_physicsOptDialog;
   TableInfoDialog m_tableInfoDialog;
   DimensionDialog m_dimensionDialog;
   MaterialDialog m_materialDialog;
   AboutDialog m_aboutDialog;
   ToolbarDialog *m_toolbarDialog = nullptr;
   PropertyDialog *m_propertyDialog = nullptr;
   CDockToolbar *m_dockToolbar = nullptr;
   CDockProperty *m_dockProperties = nullptr;
   LayersListDialog *m_layersListDialog = nullptr;
   CDockLayers *m_dockLayers = nullptr;
   NotesDialog *m_notesDialog = nullptr;
   CDockNotes* m_dockNotes = nullptr;
};

#endif // !defined(AFX_VPINBALL_H__4D32616D_55B5_4FE0_87D9_3D4CB0BE3C76__INCLUDED_)
