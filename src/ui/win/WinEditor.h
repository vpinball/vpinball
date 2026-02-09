// license:GPLv3+

// interface for the VPinball class.

#pragma once

#include "core/Settings.h"
#include "renderer/RenderDevice.h"

#include "parts/Sound.h"
#ifndef __STANDALONE__
   #include <wxx_dockframe.h>
   #include "ui/dialogs/ImageDialog.h"
   #include "ui/dialogs/SoundDialog.h"
   #include "ui/dialogs/EditorOptionsDialog.h"
   #include "ui/dialogs/CollectionManagerDialog.h"
   #include "ui/dialogs/PhysicsOptionsDialog.h"
   #include "ui/dialogs/RenderProbeDialog.h"
   #include "ui/dialogs/TableInfoDialog.h"
   #include "ui/dialogs/DimensionDialog.h"
   #include "ui/dialogs/MaterialDialog.h"
   #include "ui/dialogs/SoundDialog.h"
   #include "ui/dialogs/AboutDialog.h"
   #include "ui/dialogs/DrawingOrderDialog.h"
   #include "ui/dialogs/ToolbarDialog.h"
   #include "ui/dialogs/LayersListDialog.h"
   #include "ui/dialogs/NotesDialog.h"
   #include "ui/properties/PropertyDialog.h"

   #define OVERRIDE override
#else
   #define OVERRIDE
#endif

class PinTable;
class PinTableMDI;
class VPXFileFeedback;

class WinEditor final : public CMDIDockFrame
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

   WinEditor(HINSTANCE appInstance);
   ~WinEditor() OVERRIDE;

   void ShowSubDialog(CDialog& dlg, const bool show);

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
   bool CanClose();
   void UpdateRecentFileList(const std::filesystem::path& filename);

public:
   void AddMDITable(PinTableMDI* mdiTable);
   CMenu GetMainMenu(int id);
   void CloseAllDialogs();
   void ToggleScriptEditor();
   void ToggleBackglassView();
   bool ParseCommand(const size_t code, const bool notify);
   void LoadEditorSetupFromSettings();

   class PinTableWnd* GetActiveTableEditor();
   CComObject<PinTable>* GetActiveTable();
   bool LoadFile(const bool updateEditor, VPXFileFeedback* feedback = nullptr);
   void LoadFileName(const string& szFileName, const bool updateEditor, VPXFileFeedback* feedback = nullptr);
   void SetClipboard(vector<IStream*> * const pvstm);

   void DoPlay(const int playMode);

   void SetPosCur(float x, float y);
   void SetObjectPosCur(float x, float y);
   void ClearObjectPosCur();
   float ConvertToUnit(const float value) const;
   void SetPropSel(VectorProtected<ISelect> &pvsel);

   void RenameEditable(IEditable* editable, const string& newName);

   void SetActionCur(const string& szaction);
   void SetCursorCur(HINSTANCE hInstance, LPCTSTR lpCursorName);

   STDMETHOD(QueryInterface)(REFIID riid, void** ppvObj);
   STDMETHOD_(ULONG, AddRef)();
   STDMETHOD_(ULONG, Release)();

   STDMETHOD(PlaySound)(BSTR bstr);

   STDMETHOD(FireKnocker)(int Count);
   STDMETHOD(QuitPlayer)(int CloseType);

   void CloseTable(PinTableWnd * ppt);

   void ToggleToolbar();
   void SetEnableMenuItems();

   HANDLE PostWorkToWorkerThread(int workid, LPARAM lParam);

   void SetAutoSaveMinutes(const int minutes);
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

#ifndef __STANDALONE__
    ::SendMessage(m_hwndStatusBar, SB_SETTEXT, 5 | 0, (size_t)textBuf.c_str());
#endif
   }

   bool OpenFileDialog(const string& initDir, vector<string>& filename, const char* const fileFilter, const char* const defaultExt, const DWORD flags, const string& windowTitle = string());
   bool SaveFileDialog(const string& initDir, vector<string>& filename, const char* const fileFilter, const char* const defaultExt, const DWORD flags, const string& windowTitle = string());

   CDockProperty* GetPropertiesDocker();
   CDockToolbar *GetToolbarDocker();
   CDockNotes* GetNotesDocker();
   CDockLayers* GetLayersDocker();
   void ResetAllDockers();

   void DestroyNotesDocker()
   {
      m_notesDialog = nullptr;
      m_dockNotes = nullptr;
   }
   void CreateDocker();
   #ifndef __STANDALONE__
   LayersListDialog* GetLayersListDialog() { return GetLayersDocker()->GetContainLayers()->GetLayersDialog(); }
   #endif
   bool IsClosing() const { return m_closing; }

   ULONG m_cref;

   // registered window message ID for PinSim::FrontEndControls
   // (http://mjrnet.org/pinscape/PinSimFrontEndControls/PinSimFrontEndControls.htm)
   UINT m_pinSimFrontEndControlsMsg;

   // handler for PinSim::FrontEndControls messages
   LRESULT OnFrontEndControlsMsg(WPARAM wParam, LPARAM lParam);

   vector<PinTableWnd*> m_vtable;
   CComObject<PinTable> *m_ptableActive = nullptr;

   bool m_table_played_via_SelectTableOnStart = false;

//    HWND m_hwndToolbarMain;
   HWND m_hwndStatusBar;

   int m_palettescroll;

   vector<IStream*> m_vstmclipboard;

   int m_ToolCur; // palette button currently pressed

   int m_NextTableID; // counter to create next unique table name

   CodeViewer *m_pcv; // currently active code window

   bool m_backglassView = false; // whether viewing the playfield or screen layout

   bool m_alwaysDrawDragPoints;
   bool m_alwaysDrawLightCenters;
   int m_gridSize;
   int m_convertToUnit; // 0=Inches, 1=Millimeters, 2=VPUnits

public:

   int m_autosaveTime;

   COLORREF m_elemSelectColor;
   COLORREF m_elemSelectLockedColor;
   COLORREF m_backgroundColor;
   COLORREF m_fillColor;
   Vertex2D m_mouseCursorPosition;

   HBITMAP m_hbmInPlayMode;

   bool m_open_minimized = false;
   bool m_disable_pause_menu = false;

protected:
   void PreCreate(CREATESTRUCT& cs) override;
   void PreRegisterClass(WNDCLASS& wc) override;
   void OnClose() override;
   void OnDestroy() OVERRIDE;
   int  OnCreate(CREATESTRUCT& cs) override;
   LRESULT OnPaint(UINT msg, WPARAM wparam, LPARAM lparam) OVERRIDE;
   void OnInitialUpdate() override;
   BOOL OnCommand(WPARAM wparam, LPARAM lparam) override;
   LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
   LRESULT OnMDIActivated(UINT msg, WPARAM wparam, LPARAM lparam) OVERRIDE;
   LRESULT OnMDIDestroyed(UINT msg, WPARAM wparam, LPARAM lparam) OVERRIDE;
#ifndef __STANDALONE__
   BOOL PreTranslateMessage(MSG& msg) override;
   DockPtr NewDockerFromID(int id) override;
#endif

private:
   const HINSTANCE m_instance;

   CDockNotes *GetDefaultNotesDocker();

   volatile bool m_unloadingTable;
   //CMenu m_mainMenu;
   vector<string> m_recentTableList;

   HANDLE  m_workerthread;
   unsigned int m_workerthreadid;
   bool    m_closing;
   HMODULE m_scintillaDll;

   ImageDialog m_imageMngDlg;
   SoundDialog m_soundMngDlg;
   EditorOptionsDialog m_editorOptDialog;
   CollectionManagerDialog m_collectionMngDlg;
   PhysicsOptionsDialog m_physicsOptDialog;
   TableInfoDialog m_tableInfoDialog;
   DimensionDialog m_dimensionDialog;
   RenderProbeDialog m_renderProbeDialog;
   MaterialDialog m_materialDialog;
   AboutDialog m_aboutDialog;

   ToolbarDialog *m_toolbarDialog = nullptr;
   PropertyDialog *m_propertyDialog = nullptr;
   CDockToolbar *m_dockToolbar = nullptr;
   CDockProperty *m_dockProperties = nullptr;
   CDockLayers *m_dockLayers = nullptr;
   NotesDialog *m_notesDialog = nullptr;
   CDockNotes* m_dockNotes = nullptr;
};
