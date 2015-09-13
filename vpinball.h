// VPinball.h: interface for the VPinball class.
//
//////////////////////////////////////////////////////////////////////
#pragma once
#if !defined(AFX_VPINBALL_H__4D32616D_55B5_4FE0_87D9_3D4CB0BE3C76__INCLUDED_)
#define AFX_VPINBALL_H__4D32616D_55B5_4FE0_87D9_3D4CB0BE3C76__INCLUDED_

#include "RenderDevice.h"

#define CURRENT_FILE_FORMAT_VERSION 1001

#define DEFAULT_SECURITY_LEVEL 0

#define LAST_OPENED_TABLE_COUNT	8


class PinTable;

class VPinball //:// public IVPinball, public COleAuto,
   //public IDispatchImpl<IVPinball, &IID_IVPinball, &LIBID_VPinballLib>
{
public:
   VPinball();
   virtual ~VPinball();

   void Quit();

   void Init();
   void RegisterClasses();
   void CreateSideBar();
   HWND CreateLayerToolbar(HWND hwndParent);
   HWND CreateToolbar(TBBUTTON *p_tbbutton, int count, HWND hwndParent);
   void CreateMDIClient();

   void ParseCommand(size_t code, HWND hwnd, size_t notify);
   void setLayerStatus(int layerNumber);

   CComObject<PinTable> *GetActiveTable();
   void InitTools();
   void InitRegValues();			//>>> added by Chris
   void LoadFile();
   void LoadFileName(char *szFileName);
   void SetClipboard(Vector<IStream> *pvstm);

   BOOL FCanClose();

   void DoPlay(bool _cameraMode = false);

   void SetPosCur(float x, float y);
   void SetObjectPosCur(float x, float y);
   void ClearObjectPosCur();
   void SetPropSel(Vector<ISelect> *pvsel);

   void SetActionCur(char *szaction);
   void SetCursorCur(HINSTANCE hInstance, LPCTSTR lpCursorName);

   void GetMyPath();
   void UpdateRecentFileList(char *szfilename);	//>>> added by chris

   STDMETHOD(QueryInterface)(REFIID riid, void** ppvObj);
   STDMETHOD_(ULONG, AddRef)();
   STDMETHOD_(ULONG, Release)();
   //inline IDispatch *GetDispatch() {return (IVPinball *)this;}

   HRESULT CheckTypeInfo(UINT itinfo, LCID lcid);
   ITypeInfo **GetTinfoClsAddr() { return &m_ptinfoCls; }
   ITypeInfo **GetTinfoIntAddr() { return &m_ptinfoInt; }

   //virtual IDispatch *GetPrimary() {return this->GetDispatch(); }

   virtual HRESULT GetTypeLibInfo(HINSTANCE *phinstOut, const GUID **pplibidOut,
      SHORT *pwMajLibOut, SHORT *pwMinLibOut,
      const CLSID **ppclsidOut, const IID **ppiidOut,
      ITypeLib ***ppptlOut);

   HRESULT MainMsgLoop();
   HRESULT ApcHost_OnIdle(BOOL* pfContinue);
   HRESULT ApcHost_OnTranslateMessage(MSG* pmsg, BOOL* pfConsumed);

   BOOL CloseTable(PinTable *ppt);

   void SetEnableToolbar();
   void SetEnablePalette();
   void ShowPermissionError();
   void SetEnableMenuItems();

   void EnsureWorkerThread();
   HANDLE PostWorkToWorkerThread(int workid, LPARAM lParam);

   void SetAutoSaveMinutes(int minutes);
   static void SetOpenMinimized();
   void ShowDrawingOrderDialog(bool select);

   ULONG m_cref;
   ITypeInfo *m_ptinfoCls;
   ITypeInfo *m_ptinfoInt;

   HWND m_hwnd;

   Vector< CComObject<PinTable> > m_vtable;
   CComObject<PinTable> *m_ptableActive;

   HWND m_hwndSideBar;
   HWND m_hwndSideBarScroll;
   HWND m_hwndSideBarLayers;
   HWND m_hwndWork;
   HWND m_hwndToolbarMain;
   HWND m_hwndToolbarPalette;
   HWND m_hwndToolbarLayers;
   HWND m_hwndStatusBar;

   int m_palettescroll;

   SmartBrowser m_sb;

   Vector<IStream> m_vstmclipboard;

   PinDirectSound m_pds; //!! delete as soon as all on BASS
   PinDirectSound *m_pbackglassds; //!! delete

   int m_ToolCur; // Palette button currently pressed

   int m_NextTableID; // counter to create next unique table name

   CodeViewer *m_pcv; // Currently active code window

   bool m_fBackglassView; // Whether viewing the playfield or screen layout

   bool m_fAlwaysDrawDragPoints;
   bool m_fAlwaysDrawLightCenters;
   int m_gridSize;

   int m_securitylevel;

   char m_szMyPath[MAX_PATH];
   WCHAR m_wzMyPath[MAX_PATH];

   STDMETHOD(PlaySound)(BSTR bstr);

   STDMETHOD(FireKnocker)(int Count);
   STDMETHOD(QuitPlayer)(int CloseType);

   int m_autosaveTime;
   static bool m_open_minimized;

   HMENU GetMainMenu(int id);

   bool m_fPropertiesFloating;
   Material dummyMaterial;

private:
   char m_szRecentTableList[LAST_OPENED_TABLE_COUNT + 1][MAX_PATH];

   HANDLE m_workerthread;
   unsigned int m_workerthreadid;

   HMODULE m_scintillaDll;
};

#endif // !defined(AFX_VPINBALL_H__4D32616D_55B5_4FE0_87D9_3D4CB0BE3C76__INCLUDED_)
