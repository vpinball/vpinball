// license:GPLv3+

#pragma once

#include "parts/pintable.h"


class PinTableWnd : public CWnd
{
public:
   explicit PinTableWnd(VPinball * vpxEditor, CComObject<PinTable> *table);
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
   bool GetDisplayGrid() const { return m_grid; }
   void SetDisplayGrid(const bool display) { m_grid = display; }
   bool GetDisplayBackdrop() const { return m_backdrop; }
   void SetDisplayBackdrop(const bool backdrop) { m_backdrop = backdrop; }
   const Vertex2D& GetViewOffset() const { return m_offset; }
   void SetViewOffset(const Vertex2D& offset) { m_offset = offset; }
   float GetZoom() const { return m_zoom; }
   void SetZoom(float zoom);

   void FVerifySaveToClose();
   void BeginAutoSaveCounter();
   void EndAutoSaveCounter();
   void AutoSave();

   CComObject<PinTable> *const m_table;
   
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

   VPinball * const m_vpxEditor;
   PinTableMDI *m_mdiTable = nullptr;

   Vertex2D m_offset;
   float m_zoom;
   bool m_grid = true; // Display grid or not
   bool m_backdrop = true;

   bool m_moving = false;
   short2 m_oldMousePos;

   vector<HANDLE> m_vAsyncHandles;

   bool m_dirtyDraw = true; // Whether our background bitmap is up to date
   HBITMAP m_hbmOffScreen = nullptr; // Buffer for drawing the editor window
};
