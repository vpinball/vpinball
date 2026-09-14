// license:GPLv3+

#pragma once

#include "parts/pintable.h"
#include "ui/win/parts/TableWinUIPart.h"
#include "unordered_dense.h"

#include <memory>

class PinTableMDI;
class IWinUIPart;

class PinTableWnd : public CWnd
{
public:
   explicit PinTableWnd(WinEditor *vpxEditor, CComObject<PinTable> *table);
   ~PinTableWnd();

   void SetMDITable(PinTableMDI *const table) { m_mdiTable = table; }
   PinTableMDI *GetMDITable() const { return m_mdiTable; }

   ISelect *HitTest(const int x, const int y);

   void ClearMultiSel(ISelect *newSel = nullptr);
   bool MultiSelIsEmpty() const;
   ISelect *GetSelectedItem() const { return m_vmultisel.ElementAt(0); }
   void AddMultiSel(ISelect *psel, const bool add, const bool update, const bool contextClick);
   void RefreshProperties();
   void AssignSelectionToPartGroup(PartGroup *group);

#ifndef __STANDALONE__
   void SetMouseCursor();
   #endif
   void SetCaption(const string &caption);
   int ShowMessageBox(const char *text) const;

   void FillCollectionContextMenu(CMenu &mainMenu, CMenu &colSubMenu, ISelect *psel);
   void FillLayerContextMenu(CMenu &mainMenu, CMenu &layerSubMenu, ISelect *psel);

   void NewCollection(const HWND hwndListView, const bool fFromSelection);
   void ListCollections(HWND hwndListView);
   int AddListCollection(HWND hwndListView, CComObject<Collection> *pcol);

   void ImportFont(HWND hwndListView, const string &filename);
   void ListFonts(HWND hwndListView);
   int AddListBinary(HWND hwndListView, PinBinary *ppb);

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

   // Transform editor window coordinates to table coordinates
   Vertex2D TransformPoint(int x, int y) const;

   void FVerifySaveToClose();
   void BeginAutoSaveCounter();
   void EndAutoSaveCounter();
   void AutoSave();

   void ShowSearchSelectDlg();

   void OnPartChanged(IEditable *part);
   void OnPartAdded(IEditable *part);
   void OnPartRemoved(IEditable *part);

   // Returns the UI part owned by this editor for the given select, i.e. an entry of m_uiParts, m_tablePart for the table itself,
   // or a sub part of the owning part's UI part (drag points, light centers, ...). nullptr if none.
   IWinUIPart *GetUIPart(ISelect *select);
   IWinUIPart *GetUIPart(IEditable *part) { return GetUIPart(part ? part->GetISelect() : nullptr); }

   CComObject<PinTable> *const m_table;

   VectorProtected<ISelect> m_vmultisel;

   std::unique_ptr<class CodeViewer> m_pcv;

   ViewSetupID m_currentBackglassMode = ViewSetupID::BG_DESKTOP; // POV shown in the UI (not persisted)

   WinEditor *const m_vpxEditor;

#ifndef __STANDALONE__
   // UI part of the table itself. Unlike the other UI parts, it is not created through WinUIPartRegistry
   // but is a direct member of this editor, sharing its lifecycle.
   TableWinUIPart m_tablePart;
#endif

protected:
#ifndef __STANDALONE__
   // Overriden from CWnd
   void OnInitialUpdate() final;
   BOOL OnEraseBkgnd(CDC &dc) final;
   LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam) final;
#endif

private:
#ifndef __STANDALONE__
   void OnLeftDoubleClick(int x, int y);
   void OnLeftButtonDown(const short x, const short y);
   void DoLeftButtonDown(int x, int y, bool zoomIn);
   void UseTool(int x, int y, int tool);
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
   void RenderTable(Sur *const psur);
#endif

   PinTableMDI *m_mdiTable = nullptr;

   std::unique_ptr<class SearchSelectDialog> m_searchSelectDlg;

   bool m_moving = false;
   short2 m_oldMousePos;

   vector<HANDLE> m_vAsyncHandles;

   bool m_dirtyDraw = true; // Whether our background bitmap is up to date
   HBITMAP m_hbmOffScreen = nullptr; // Buffer for drawing the editor window

   // UI parts owned by this editor: one per entry of PinTable::m_vedit (keyed by IEditable::GetISelect()).
   // Kept in sync by OnPartAdded/OnPartRemoved. Sub selects (drag points, light centers) are owned by their parent's UI part (see IWinUIPart::GetSubPart).
   ankerl::unordered_dense::map<ISelect *, std::unique_ptr<IWinUIPart>> m_uiParts;

private:
   POINT m_ptLast {}; // Last point when dragging

};
