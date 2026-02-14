// license:GPLv3+

#pragma once

#include "parts/pintable.h"

class PinTableWnd : public CWnd
{
public:
   explicit PinTableWnd(WinEditor *vpxEditor, CComObject<PinTable> *table);
   ~PinTableWnd();

   void SetMDITable(PinTableMDI *const table) { m_mdiTable = table; }
   PinTableMDI *GetMDITable() const { return m_mdiTable; }

   ISelect *HitTest(const int x, const int y);

   #ifndef __STANDALONE__
   void SetMouseCursor();
   #endif
   void SetCaption(const string &caption);
   int ShowMessageBox(const char *text) const;

   void FillCollectionContextMenu(CMenu &mainMenu, CMenu &colSubMenu, ISelect *psel);
   void FillLayerContextMenu(CMenu &mainMenu, CMenu &layerSubMenu, ISelect *psel);

   void Redraw();
   void SetDefaultView();
   void GetViewRect(FRect *pfrect) const;
   void SetMyScrollInfo();
   POINT GetScreenPoint() const;
   void ExportBlueprint();
   bool GetDisplayGrid() const;
   void SetDisplayGrid(const bool display);
   bool GetDisplayBackdrop() const;
   void SetDisplayBackdrop(const bool backdrop);
   const Vertex2D &GetViewOffset() const;
   void SetViewOffset(const Vertex2D &offset);
   float GetZoom() const;
   void SetZoom(float zoom);

   void FVerifySaveToClose();
   void BeginAutoSaveCounter();
   void EndAutoSaveCounter();
   void AutoSave();

   void ShowSearchSelectDlg();

   void OnPartChanged(IEditable *part);

   CComObject<PinTable> *const m_table;
   
   std::unique_ptr<CodeViewer> m_pcv;

protected:
#ifndef __STANDALONE__
   // Overriden from CWnd
   void OnInitialUpdate() override final;
   BOOL OnEraseBkgnd(CDC &dc) override final;
   LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override final;
#endif

private:
#ifndef __STANDALONE__
   void OnLeftDoubleClick(int x, int y);
   void OnLeftButtonDown(const short x, const short y);
   void DoLeftButtonDown(int x, int y, bool zoomIn);
   void OnLeftButtonUp(int x, int y);
   void OnRightButtonDown(int x, int y);
   void OnRightButtonUp(int x, int y);
   void OnMouseMove(const int x, const int y);
   void OnMouseWheel(const short x, const short y, const short zDelta);
   void OnKeyDown(int key);
   void OnSize();
   void DoContextMenu(int x, int y, const int menuid, ISelect *psel);

   void Paint(HDC hdc);
   void Render3DProjection(Sur *const psur);
   void UIRenderPass2(Sur *const psur);
#endif

   WinEditor *const m_vpxEditor;
   PinTableMDI *m_mdiTable = nullptr;

   std::unique_ptr<class SearchSelectDialog> m_searchSelectDlg;

   bool m_moving = false;
   short2 m_oldMousePos;

   vector<HANDLE> m_vAsyncHandles;

   bool m_dirtyDraw = true; // Whether our background bitmap is up to date
   HBITMAP m_hbmOffScreen = nullptr; // Buffer for drawing the editor window

private:
   POINT m_ptLast {}; // Last point when dragging

};
