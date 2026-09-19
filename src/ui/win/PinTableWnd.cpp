// license:GPLv3+

#include "core/stdafx.h"
#include "PinTableWnd.h"

#include "core/editablereg.h"
#include "core/VPApp.h"
#include "core/vpversion.h"
#include "parts/Collection.h"
#include "parts/dragpoint.h"
#include "parts/flipper.h"
#include "parts/PartGroup.h"
#include "parts/primitive.h"
#include "renderer/Texture.h"
#include "ui/win/codeview.h"
#include "ui/win/hitrectsur.h"
#include "ui/win/hitsur.h"
#include "ui/win/paintsur.h"
#include "ui/win/PinTableMDI.h"
#include "ui/win/sur.h"
#include "ui/win/WinEditor.h"
#include "ui/win/worker.h"
#include "ui/win/WinUIPartRegistry.h"
#include "utils/BiffReader.h"
#include "utils/BiffWriter.h"

#ifndef __STANDALONE__
#include "ui/win/dialogs/SearchSelectDialog.h"
#include "ui/win/dialogs/Win32ProgressBar.h"
#include "FreeImage.h"
#else
class SearchSelectDialog { };
#endif


PinTableWnd::PinTableWnd(WinEditor *vpxEditor, CComObject<PinTable> *table)
   : m_table(table)
   , m_pcv(std::make_unique<CodeViewer>(table))
   , m_vpxEditor(vpxEditor)
#ifndef __STANDALONE__
   , m_tablePart(this, table)
#endif
{
   m_table->AddRef();
   m_table->m_tableEditor = this;
   m_pcv->Create(nullptr);
   ClearMultiSel();
   SetDefaultView();
#ifndef __STANDALONE__
   // Create the UI parts of any part added before the editor was attached
   for (IEditable *const part : m_table->GetParts())
      OnPartAdded(part);
#endif
}

PinTableWnd::~PinTableWnd()
{
   m_uiParts.clear();
   m_table->m_tableEditor = nullptr;
   m_table->Release();
#ifndef __STANDALONE__
   if (m_hbmOffScreen)
      DeleteObject(m_hbmOffScreen);
#endif
}

void PinTableWnd::SetCaption(const string &szCaption)
{
#ifndef __STANDALONE__
   if (m_mdiTable != nullptr && m_mdiTable->IsWindow())
      m_mdiTable->SetWindowText(szCaption.c_str());
   m_pcv->SetCaption(szCaption);
#endif
}

int PinTableWnd::ShowMessageBox(const char *text) const
{
#ifndef __STANDALONE__
   return m_mdiTable->MessageBox(text, "Visual Pinball", MB_YESNO);
#else
   return 0;
#endif
}

void PinTableWnd::Redraw()
{
#ifndef __STANDALONE__
   m_dirtyDraw = true;
   if (IsWindow())
      InvalidateRect(false);
#endif
}

void PinTableWnd::SetDefaultView()
{
   FRect frect;
   GetViewRect(&frect);
   SetViewOffset(frect.Center());
   SetZoom(0.5f);
}

bool PinTableWnd::GetDisplayGrid() const { return m_table->m_winEditorGrid; }

void PinTableWnd::SetDisplayGrid(const bool display)
{
   m_table->m_winEditorGrid = display;
   Redraw();
}
bool PinTableWnd::GetDisplayBackdrop() const { return m_table->m_winEditorBackdrop; }

void PinTableWnd::SetDisplayBackdrop(const bool backdrop)
{
   m_table->m_winEditorBackdrop = backdrop;
   Redraw();
}
const Vertex2D &PinTableWnd::GetViewOffset() const { return m_table->m_winEditorViewOffset; }

void PinTableWnd::SetViewOffset(const Vertex2D &offset) { m_table->m_winEditorViewOffset = offset; }

float PinTableWnd::GetZoom() const { return m_table->m_winEditorZoom; }

void PinTableWnd::SetZoom(float zoom)
{
   m_table->m_winEditorZoom = clamp(zoom, MIN_ZOOM, MAX_ZOOM);
   SetMyScrollInfo();
   Redraw();
}

Vertex2D PinTableWnd::TransformPoint(int x, int y) const
{
#ifndef __STANDALONE__
   const CRect rc = GetClientRect();
#else
   const CRect rc(m_table->m_left, m_table->m_top, m_table->m_right, m_table->m_bottom);
#endif
   const HitSur phs(GetZoom(), GetViewOffset().x, GetViewOffset().y, rc.right - rc.left, rc.bottom - rc.top, 0, 0, nullptr);

   const Vertex2D result = phs.ScreenToSurface(x, y);

   return result;
}

void PinTableWnd::GetViewRect(FRect *const pfrect) const
{
   if (!m_vpxEditor->m_desktopBackdropView)
   {
      pfrect->left = m_table->m_left;
      pfrect->top = m_table->m_top;
      pfrect->right = m_table->m_right;
      pfrect->bottom = m_table->m_bottom;
   }
   else
   {
      pfrect->left = 0;
      pfrect->top = 0;
      pfrect->right = EDITOR_BG_WIDTH;
      pfrect->bottom = EDITOR_BG_HEIGHT;
   }
}

void PinTableWnd::SetMyScrollInfo()
{
#ifndef __STANDALONE__
   if (!IsWindow())
      return;
   FRect frect;
   GetViewRect(&frect);

   const CRect rc = GetClientRect();

   const HitSur phs(GetZoom(), GetViewOffset().x, GetViewOffset().y, rc.right - rc.left, rc.bottom - rc.top, 0, 0, nullptr);

   Vertex2D rgv[2];
   rgv[0] = phs.ScreenToSurface(rc.left, rc.top);
   rgv[1] = phs.ScreenToSurface(rc.right, rc.bottom);

   SCROLLINFO si = {};
   si.cbSize = sizeof(SCROLLINFO);
   si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
   si.nMin = (int)min(frect.left, rgv[0].x);
   si.nMax = (int)max(frect.right, rgv[1].x);
   si.nPage = (int)(rgv[1].x - rgv[0].x);
   si.nPos = (int)(rgv[0].x);

   SetScrollInfo(SB_HORZ, si, true);

   si.nMin = (int)min(frect.top, rgv[0].y);
   si.nMax = (int)max(frect.bottom, rgv[1].y);
   si.nPage = (int)(rgv[1].y - rgv[0].y);
   si.nPos = (int)(rgv[0].y);

   SetScrollInfo(SB_VERT, si, true);
#endif
}

void PinTableWnd::ExportBlueprint()
{
#ifndef __STANDALONE__
   //bool saveAs = true;
   //if (saveAs)
   //{
   //need to get a file name
   OPENFILENAME ofn = {};
   ofn.lStructSize = sizeof(OPENFILENAME);
   ofn.hInstance = g_app->GetInstanceHandle();
   ofn.hwndOwner = m_vpxEditor->GetHwnd();
   ofn.lpstrFilter = "PNG (.png)\0*.png;\0Bitmap (.bmp)\0*.bmp;\0TGA (.tga)\0*.tga;\0TIFF (.tiff/.tif)\0*.tiff;*.tif;\0WEBP (.webp)\0*.webp;\0";
   char szBlueprintFileName[MAXSTRING];
   strncpy_s(szBlueprintFileName, std::size(szBlueprintFileName), m_table->m_filename.string().c_str());
   const size_t idx = m_table->m_filename.string().find_last_of('.');
   if (idx != string::npos && idx < MAXSTRING)
      szBlueprintFileName[idx] = '\0';
   ofn.lpstrFile = szBlueprintFileName;
   ofn.nMaxFile = std::size(szBlueprintFileName);
   ofn.lpstrDefExt = "png";
   ofn.Flags = OFN_NOREADONLYRETURN | OFN_CREATEPROMPT | OFN_OVERWRITEPROMPT | OFN_EXPLORER;

   const int ret = GetSaveFileName(&ofn);

   // user cancelled
   if (ret == 0)
      return; // S_FALSE;
   //}

   const int result = m_vpxEditor->MessageBox("Do you want a solid blueprint?", "Export As Solid?", MB_YESNO);
   const bool solid = (result == IDYES);

   float tableheight, tablewidth;
   if (m_vpxEditor->m_desktopBackdropView)
   {
      tablewidth = (float)EDITOR_BG_WIDTH;
      tableheight = (float)EDITOR_BG_HEIGHT;
   }
   else
   {
      tablewidth = m_table->m_right - m_table->m_left;
      tableheight = m_table->m_bottom - m_table->m_top;
   }

   int bmwidth, bmheight;
   if (tableheight > tablewidth)
   {
      bmheight = 4096;
      bmwidth = (int)((tablewidth / tableheight) * (float)bmheight + 0.5f);
   }
   else
   {
      bmwidth = 4096;
      bmheight = (int)((tableheight / tablewidth) * (float)bmwidth + 0.5f);
   }

   int totallinebytes = bmwidth * 3;
   totallinebytes = (((totallinebytes - 1) / 4) + 1) * 4; // make multiple of four
#if 0
   HANDLE hfile = CreateFile(szBlueprintFileName, GENERIC_WRITE, FILE_SHARE_READ, nullptr,
      CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
   const int bmlinebuffer = totallinebytes - (bmwidth * 3);

   BITMAPFILEHEADER bmfh = {};
   bmfh.bfType = 'M' << 8 | 'B';
   bmfh.bfSize = sizeof(bmfh) + sizeof(BITMAPINFOHEADER) + totallinebytes*bmheight;
   bmfh.bfOffBits = (DWORD)sizeof(bmfh) + (DWORD)sizeof(BITMAPINFOHEADER);

   DWORD foo;
   WriteFile(hfile, &bmfh, sizeof(bmfh), &foo, nullptr);
#endif
   BITMAPINFO bmi = {};
   bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
   bmi.bmiHeader.biWidth = bmwidth;
   bmi.bmiHeader.biHeight = bmheight;
   bmi.bmiHeader.biPlanes = 1;
   bmi.bmiHeader.biBitCount = 24;
   bmi.bmiHeader.biCompression = BI_RGB;
   bmi.bmiHeader.biSizeImage = totallinebytes * bmheight;
#if 0
   WriteFile(hfile, &bmi, sizeof(BITMAPINFOHEADER), &foo, nullptr);
#endif

   CDC dc;
   dc.CreateCompatibleDC(nullptr);
   char *pbits;
   dc.CreateDIBSection(dc.GetHDC(), &bmi, DIB_RGB_COLORS, (void **)&pbits, nullptr, 0);

   {
      PaintSur psur((float)bmwidth / tablewidth, tablewidth * 0.5f, tableheight * 0.5f, bmwidth, bmheight, dc.GetHDC(), this, nullptr);

      dc.SelectObject(static_cast<HBRUSH>(dc.GetStockObject(WHITE_BRUSH)));
      dc.PatBlt(0, 0, bmwidth, bmheight, PATCOPY);

      if (m_vpxEditor->m_desktopBackdropView)
         Render3DProjection(&psur);

      for (const auto &ptr : m_table->GetParts())
      {
         if (ptr->IsUIVisible(false) && ptr->m_desktopBackdrop == m_vpxEditor->m_desktopBackdropView)
            if (IWinUIPart *const uiPart = GetUIPart(ptr))
               uiPart->RenderBlueprint(&psur, solid);
      }
   }

#if 0
   for (int i = 0; i < bmheight; i++)
      WriteFile(hfile, (pbits + ((i*bmwidth) * 3)), bmwidth * 3, &foo, nullptr);

   // For some reason to make our bitmap compatible with all programs,
   // We need to write out dummy bytes as if our totalwidthbytes had been
   // a multiple of 4.
   for (int i = 0; i < bmheight; i++)
      for (int l = 0; l < bmlinebuffer; l++)
         WriteFile(hfile, pbits, 1, &foo, nullptr);

   CloseHandle(hfile);
#else
   FIBITMAP *dib = FreeImage_Allocate(bmwidth, bmheight, 24);
   BYTE *const pdst = FreeImage_GetBits(dib);
   //const unsigned int pitch_dst = FreeImage_GetPitch(dib); //!! necessary?
   memcpy(pdst, pbits, (size_t)bmwidth * bmheight * 3);
   if (!FreeImage_Save(FreeImage_GetFIFFromFilename(szBlueprintFileName), dib, szBlueprintFileName, PNG_Z_BEST_COMPRESSION | BMP_SAVE_RLE))
      m_vpxEditor->MessageBox("Export failed!", "Blueprint Export", MB_OK | MB_ICONEXCLAMATION);
   else
#endif
      m_vpxEditor->MessageBox("Export finished!", "Blueprint Export", MB_OK);
#if 1
   FreeImage_Unload(dib);
#endif
#endif
}

void PinTableWnd::ImportBackdropPOV()
{
#ifndef __STANDALONE__
   if (m_table->IsLocked())
      return;
   const string &initialDir = m_table->m_settings.GetRecentDir_POVDir();
   vector<string> fileNames;
   if (!m_vpxEditor->OpenFileDialog(
          initialDir, fileNames, "User settings file (*.ini)\0*.ini\0Old POV file (*.pov)\0*.pov\0Legacy POV file(*.xml)\0*.xml\0", "ini", 0, "Import POV to table properties"s))
      return;
   const std::filesystem::path file = fileNames[0];
   if (file.has_parent_path())
      g_app->m_settings.SetRecentDir_POVDir(file.parent_path().string(), false);
   m_table->ImportBackdropPOV(file, false);
#endif
}

void PinTableWnd::ExportBackdropPOV()
{
#ifndef __STANDALONE__
   OPENFILENAME ofn = {};
   ofn.lStructSize = sizeof(OPENFILENAME);
   ofn.hInstance = g_app->GetInstanceHandle();
   ofn.hwndOwner = m_vpxEditor->GetHwnd();
   // TEXT
   ofn.lpstrFilter = "INI file(*.ini)\0*.ini\0";
   char szFileName[MAXSTRING];
   strncpy_s(szFileName, std::size(szFileName), m_table->m_filename.string().c_str());
   const size_t idx = m_table->m_filename.string().find_last_of('.');
   if (idx != string::npos && idx < std::size(szFileName))
      szFileName[idx] = '\0';
   ofn.lpstrFile = szFileName;
   ofn.nMaxFile = std::size(szFileName);
   ofn.lpstrDefExt = "ini";
   ofn.Flags = OFN_NOREADONLYRETURN | OFN_CREATEPROMPT | OFN_OVERWRITEPROMPT | OFN_EXPLORER;
   const int ret = GetSaveFileName(&ofn);
   // user cancelled
   if (ret == 0)
      return; // S_FALSE;
   m_table->ExportBackdropPOV(szFileName);
#endif
}

void PinTableWnd::ImportPhysics()
{
#ifndef __STANDALONE__
   const string &szInitialDir = m_table->m_settings.GetRecentDir_PhysicsDir();
   vector<string> filename;
   if (!m_vpxEditor->OpenFileDialog(szInitialDir, filename, "Visual Pinball Physics (*.vpp)\0*.vpp\0", "vpp", 0))
      return;

   const size_t index = filename[0].find_last_of(PATH_SEPARATOR_CHAR);
   if (index != string::npos)
      g_app->m_settings.SetRecentDir_PhysicsDir(filename[0].substr(0, index), false);

   m_table->ImportVPP(filename[0]);
#endif
}

void PinTableWnd::ExportPhysics()
{
#ifndef __STANDALONE__
   // The export uses the physics settings of the first flipper of the table
   Flipper *flipper = nullptr;
   for (IEditable *const part : m_table->GetParts())
   {
      if (part->GetItemType() == eItemFlipper)
      {
         flipper = (Flipper *)part;
         break;
      }
   }

   if (flipper == nullptr)
   {
      ShowError("No Flipper found to copy settings from");
      return;
   }

   char szFileName[MAXSTRING];
   strncpy_s(szFileName, std::size(szFileName), m_table->m_filename.string().c_str());
   const size_t idx = m_table->m_filename.string().find_last_of('.');
   if (idx != string::npos && idx < std::size(szFileName))
      szFileName[idx] = '\0';

   OPENFILENAME ofn = {};
   ofn.lStructSize = sizeof(OPENFILENAME);
   ofn.hInstance = g_app->GetInstanceHandle();
   ofn.hwndOwner = m_vpxEditor->GetHwnd();
   // TEXT
   ofn.lpstrFilter = "Visual Pinball Physics (*.vpp)\0*.vpp\0";
   ofn.lpstrFile = szFileName;
   ofn.nMaxFile = std::size(szFileName);
   ofn.lpstrDefExt = "vpp";
   ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

   string szInitialDir = m_table->m_settings.GetRecentDir_PhysicsDir();

   ofn.lpstrInitialDir = szInitialDir.c_str();

   const int ret = GetSaveFileName(&ofn);
   if (ret == 0)
      return;

   const string filename(ofn.lpstrFile);
   const size_t index = filename.find_last_of(PATH_SEPARATOR_CHAR);
   if (index != string::npos)
   {
      const string newInitDir(filename.substr(0, index));
      g_app->m_settings.SetRecentDir_PhysicsDir(newInitDir, false);
   }

   m_table->ExportVPP(ofn.lpstrFile, flipper);
#endif
}

#ifndef __STANDALONE__
void PinTableWnd::RenderTable(Sur *const psur)
{
   const CRect rc = GetClientRect();
   psur->SetFillColor(m_vpxEditor->m_backgroundColor);
   psur->SetBorderColor(-1, false, 0);
   psur->Rectangle2(rc.left, rc.top, rc.right, rc.bottom);

   FRect frect;
   GetViewRect(&frect);

   if (GetDisplayBackdrop())
   {
      Texture *const ppi = m_table->GetImage((!m_vpxEditor->m_desktopBackdropView) ? m_table->m_image : m_table->m_BG_image[m_table->GetViewMode()]);

      if (ppi && ppi->GetGDIBitmap())
      {
         CDC dc;
         dc.CreateCompatibleDC(nullptr);
         const CBitmap hbmOld = dc.SelectObject(ppi->GetGDIBitmap());

         psur->Image(frect.left, frect.top, frect.right, frect.bottom, dc.GetHDC(), ppi->m_width, ppi->m_height);

         dc.SelectObject(hbmOld);
      }
   }

   if (m_vpxEditor->m_desktopBackdropView)
   {
      Render3DProjection(psur);
   }

   for (const auto &ptr : m_table->GetParts())
   {
      if (ptr->m_desktopBackdrop == m_vpxEditor->m_desktopBackdropView && ptr->IsUIVisible(false))
         if (IWinUIPart *const uiPart = GetUIPart(ptr))
            uiPart->UIRenderPass1(psur);
   }

   if (GetDisplayGrid() && m_vpxEditor->m_gridSize > 0)
   {
      Vertex2D rlt = psur->ScreenToSurface(rc.left, rc.top);
      Vertex2D rrb = psur->ScreenToSurface(rc.right, rc.bottom);
      rlt.x = max(rlt.x, frect.left);
      rlt.y = max(rlt.y, frect.top);
      rrb.x = min(rrb.x, frect.right);
      rrb.y = min(rrb.y, frect.bottom);

      const float gridsize = (float)m_vpxEditor->m_gridSize;

      const int beginx = (int)(rlt.x / gridsize);
      const int lenx = (int)((rrb.x - rlt.x) / gridsize); //(((rc.right - rc.left)/m_zoom));
      const int beginy = (int)(rlt.y / gridsize);
      const int leny = (int)((rrb.y - rlt.y) / gridsize); //(((rc.bottom - rc.top)/m_zoom));

      psur->SetObject(nullptr); // Don't hit test gridlines

      psur->SetLineColor(RGB(190, 220, 240), false, 0);
      for (int i = 0; i < (lenx + 1); i++)
      {
         const float x = (float)(beginx + i) * gridsize;
         psur->Line(x, rlt.y, x, rrb.y);
      }

      for (int i = 0; i < (leny + 1); i++)
      {
         const float y = (float)(beginy + i) * gridsize;
         psur->Line(rlt.x, y, rrb.x, y);
      }
   }

   for (const auto &ptr : m_table->GetParts())
   {
      if (ptr->m_desktopBackdrop == m_vpxEditor->m_desktopBackdropView && ptr->IsUIVisible(false))
         if (IWinUIPart *const uiPart = GetUIPart(ptr))
            uiPart->UIRenderPass2(psur);
   }

   if (m_vpxEditor->m_desktopBackdropView) // Outline of the view, for when the grid is off
   {
      psur->SetObject(nullptr);
      psur->SetFillColor(-1);
      psur->SetBorderColor(RGB(0, 0, 0), false, 1);
      psur->Rectangle(0, 0, EDITOR_BG_WIDTH, EDITOR_BG_HEIGHT);
   }

   if (m_tablePart.m_dragging)
   {
      psur->SetFillColor(-1);
      psur->SetBorderColor(RGB(0, 0, 0), true, 0);
      psur->Rectangle(m_table->m_rcDragRect.left, m_table->m_rcDragRect.top, m_table->m_rcDragRect.right, m_table->m_rcDragRect.bottom);
   }

   // display the layer string
   //    psur->SetObject(nullptr);
   //    SetTextColor( psur->m_hdc,RGB(180,180,180));
   //    char text[64];
   //    char number[8];
   //    strncpy_s( text, std::size(text), "Layer_");
   //    _itoa_s(activeLayer+1, number, 10 );
   //    strcat_s( text, number);
   //    RECT textRect;
   //    SetRect( &textRect, rc.right-60,rc.top, rc.right, rc.top+30 );
   //    DrawText( psur->m_hdc, text, -1, &textRect, DT_LEFT);
   //
   //    SetTextColor( psur->m_hdc,RGB(0,0,0));

   //   psur->DrawText( text,rc.left+10, rc.top, 90,20);
}

// draws the backdrop content
void PinTableWnd::Render3DProjection(Sur *const psur)
{
   if (m_table->GetParts().empty())
      return;

   // dummy coordinate system for backdrop view
   ModelViewProj mvp;
   if (m_table->mViewSetups[m_currentBackglassMode].mMode == VLM_WINDOW)
      m_table->mViewSetups[m_currentBackglassMode].SetWindowModeFromSettings(m_table);
   m_table->mViewSetups[m_currentBackglassMode].ComputeMVP(m_table, (float)EDITOR_BG_WIDTH / (float)EDITOR_BG_HEIGHT, false, mvp);

   Vertex3Ds rgvIn[8];
   rgvIn[0].x = m_table->m_left;
   rgvIn[0].y = m_table->m_top;
   rgvIn[0].z = 50.0f;
   rgvIn[1].x = m_table->m_left;
   rgvIn[1].y = m_table->m_top;
   rgvIn[1].z = m_table->m_glassTopHeight;
   rgvIn[2].x = m_table->m_right;
   rgvIn[2].y = m_table->m_top;
   rgvIn[2].z = m_table->m_glassTopHeight;
   rgvIn[3].x = m_table->m_right;
   rgvIn[3].y = m_table->m_top;
   rgvIn[3].z = 50.0f;
   rgvIn[4].x = m_table->m_right;
   rgvIn[4].y = m_table->m_bottom;
   rgvIn[4].z = 50.0f;
   rgvIn[5].x = m_table->m_right;
   rgvIn[5].y = m_table->m_bottom;
   rgvIn[5].z = m_table->m_glassBottomHeight;
   rgvIn[6].x = m_table->m_left;
   rgvIn[6].y = m_table->m_bottom;
   rgvIn[6].z = m_table->m_glassBottomHeight;
   rgvIn[7].x = m_table->m_left;
   rgvIn[7].y = m_table->m_bottom;
   rgvIn[7].z = 50.0f;

   Vertex2D rgvOut[8];
   RECT viewport;
   viewport.left = 0;
   viewport.top = 0;
   viewport.right = EDITOR_BG_WIDTH;
   viewport.bottom = EDITOR_BG_HEIGHT;
   mvp.GetModelViewProj(0).TransformVertices(rgvIn, nullptr, 8, rgvOut, viewport);

   psur->SetFillColor(RGB(200, 200, 200));
   psur->SetBorderColor(-1, false, 0);
   psur->Polygon(rgvOut, 8);
}

// draws the main design screen
void PinTableWnd::Paint(HDC hdc)
{
   const CRect rc = GetClientRect();

   if (m_dirtyDraw)
   {
      if (m_hbmOffScreen)
      {
         DeleteObject(m_hbmOffScreen);
      }
      m_hbmOffScreen = CreateCompatibleBitmap(hdc, rc.right - rc.left, rc.bottom - rc.top);
   }

   CDC dc;
   dc.CreateCompatibleDC(hdc);

   const CBitmap hbmOld = dc.SelectObject(m_hbmOffScreen);

   if (m_dirtyDraw)
   {
      PaintSur psur(GetZoom(), GetViewOffset().x, GetViewOffset().y, rc.right - rc.left, rc.bottom - rc.top, dc.GetHDC(), this, m_vmultisel.empty() ? &m_tablePart : m_vmultisel[0]);
      RenderTable(&psur);
   }

   BitBlt(hdc, rc.left, rc.top, rc.right, rc.bottom, dc.GetHDC(), 0, 0, SRCCOPY);

   dc.SelectObject(hbmOld);

   m_dirtyDraw = false;
}
#endif


POINT PinTableWnd::GetScreenPoint() const
{
#ifndef __STANDALONE__
   CPoint pt = GetCursorPos();
   ScreenToClient(pt);
   return pt;
#else
   return POINT();
#endif
}

#ifndef __STANDALONE__

void PinTableWnd::OnInitialUpdate()
{
   BeginAutoSaveCounter();
   SetWindowText(m_table->m_filename.string().c_str());
   SetCaption(m_table->m_title);
   m_vpxEditor->SetEnableMenuItems();
}

BOOL PinTableWnd::OnEraseBkgnd(CDC &dc) { return TRUE; }

void PinTableWnd::OnSize()
{
   SetMyScrollInfo();
   Redraw();
}

LRESULT PinTableWnd::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   switch (uMsg)
   {
   case WM_SETCURSOR: SetMouseCursor(); return FinalWindowProc(uMsg, wParam, lParam);

   case WM_PAINT:
   {
      PAINTSTRUCT ps;
      const HDC hdc = BeginPaint(ps);
      Paint(hdc);
      EndPaint(ps);
      return FinalWindowProc(uMsg, wParam, lParam);
   }
   case WM_SIZE:
      OnSize();
      return FinalWindowProc(uMsg, wParam, lParam);

   case WM_LBUTTONDOWN:
   {
      const short x = (short)GET_X_LPARAM(lParam);
      const short y = (short)GET_Y_LPARAM(lParam);
      OnLeftButtonDown(x, y);
      return FinalWindowProc(uMsg, wParam, lParam);
   }
   case WM_LBUTTONDBLCLK:
   {
      const short x = (short)GET_X_LPARAM(lParam);
      const short y = (short)GET_Y_LPARAM(lParam);
      OnLeftDoubleClick(x, y);
      return FinalWindowProc(uMsg, wParam, lParam);
   }
   case WM_LBUTTONUP:
   {
      const short x = (short)GET_X_LPARAM(lParam);
      const short y = (short)GET_Y_LPARAM(lParam);
      OnLeftButtonUp(x, y);
      return FinalWindowProc(uMsg, wParam, lParam);
   }
   case WM_MOUSEMOVE:
   {
      const short x = (short)GET_X_LPARAM(lParam);
      const short y = (short)GET_Y_LPARAM(lParam);
      OnMouseMove(x, y);
      return FinalWindowProc(uMsg, wParam, lParam);
   }
   case WM_RBUTTONDOWN:
   {
      const short x = (short)GET_X_LPARAM(lParam);
      const short y = (short)GET_Y_LPARAM(lParam);
      OnRightButtonDown(x, y);
      return FinalWindowProc(uMsg, wParam, lParam);
   }
   case WM_CONTEXTMENU:
   {
      LONG x = GET_X_LPARAM(lParam);
      LONG y = GET_Y_LPARAM(lParam);
      POINT p;
      if (GetCursorPos(&p) && ScreenToClient(p))
      {
         x = p.x;
         y = p.y;
      }
      OnRightButtonUp(x, y);
      return FinalWindowProc(uMsg, wParam, lParam);
   }
   case WM_KEYDOWN:
   {
      OnKeyDown((int)wParam);
      return FinalWindowProc(uMsg, wParam, lParam);
   }
   case WM_HSCROLL:
   {
      SCROLLINFO si = {};
      si.cbSize = sizeof(SCROLLINFO);
      si.fMask = SIF_ALL;
      GetScrollInfo(SB_HORZ, si);
      switch (LOWORD(wParam))
      {
      case SB_LINELEFT: m_table->m_winEditorViewOffset.x -= si.nPage / 10; break;
      case SB_LINERIGHT: m_table->m_winEditorViewOffset.x += si.nPage / 10; break;
      case SB_PAGELEFT: m_table->m_winEditorViewOffset.x -= si.nPage / 2; break;
      case SB_PAGERIGHT: m_table->m_winEditorViewOffset.x += si.nPage / 2; break;
      case SB_THUMBTRACK:
      {
         const int delta = (int)(GetViewOffset().x - (float)si.nPos);
         m_table->m_winEditorViewOffset.x = (float)((short)HIWORD(wParam) + delta);
         break;
      }
      }
      Redraw();
      SetMyScrollInfo();
      return FinalWindowProc(uMsg, wParam, lParam);
   }
   case WM_VSCROLL:
   {
      SCROLLINFO si = {};
      si.cbSize = sizeof(SCROLLINFO);
      si.fMask = SIF_ALL;
      GetScrollInfo(SB_VERT, si);
      switch (LOWORD(wParam))
      {
      case SB_LINEUP: m_table->m_winEditorViewOffset.y -= si.nPage / 10; break;
      case SB_LINEDOWN: m_table->m_winEditorViewOffset.y += si.nPage / 10; break;
      case SB_PAGEUP: m_table->m_winEditorViewOffset.y -= si.nPage / 2; break;
      case SB_PAGEDOWN: m_table->m_winEditorViewOffset.y += si.nPage / 2; break;
      case SB_THUMBTRACK:
      {
         const int delta = (int)(GetViewOffset().y - (float)si.nPos);
         m_table->m_winEditorViewOffset.y = (float)((short)HIWORD(wParam) + delta);
         break;
      }
      }
      Redraw();
      SetMyScrollInfo();
      return FinalWindowProc(uMsg, wParam, lParam);
   }
   case WM_MOUSEWHEEL:
   {
      //zoom in/out by pressing CTRL+mouse wheel
      const short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
      OnMouseWheel(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), zDelta);
      return FinalWindowProc(uMsg, wParam, lParam);
   }
   case DONE_AUTOSAVE:
   {
      if (lParam == S_OK)
      {
         m_vpxEditor->SetActionCur(""s);
      }
      else
      {
         m_vpxEditor->SetActionCur("Autosave Failed"s);
      }
      BeginAutoSaveCounter();
      const HANDLE hEvent = (HANDLE)wParam;
      RemoveFromVectorSingle(m_vAsyncHandles, hEvent);
      CloseHandle(hEvent);
      return FinalWindowProc(uMsg, wParam, lParam);
   }
   default: break;
   }
   return WndProcDefault(uMsg, wParam, lParam);
}

void PinTableWnd::SetMouseCursor()
{
   HINSTANCE hinst = g_app->GetInstanceHandle();
   static int oldTool = -1;

   if (oldTool != m_vpxEditor->m_ToolCur)
   {
      char *cursorid;
      if (m_vpxEditor->m_ToolCur == ID_TABLE_MAGNIFY)
      {
         cursorid = MAKEINTRESOURCE(IDC_MAGNIFY);
      }
      else if (m_vpxEditor->m_ToolCur == ID_INSERT_TARGET)
      {
         // special case for targets, which are particular walls
         cursorid = MAKEINTRESOURCE(IDC_TARGET);
      }
      else
      {
         const ItemTypeEnum type = WinUIPartRegistry::TypeFromToolID(m_vpxEditor->m_ToolCur);
         if (type != eItemInvalid)
            cursorid = MAKEINTRESOURCE(WinUIPartRegistry::GetCursorID(type));
         else
         {
            hinst = nullptr;
            cursorid = IDC_ARROW;
         }
      }
      const HCURSOR hcursor = LoadCursor(hinst, cursorid);
      SetClassLongPtr(GCLP_HCURSOR, (LONG_PTR)hcursor);
      SetCursor(hcursor);
      oldTool = m_vpxEditor->m_ToolCur;
   }
}

void PinTableWnd::SetMouseCapture() { SetCapture(); }

#endif

void PinTableWnd::ClearMultiSel(IWinUIPart *newSelPart)
{
   for (IWinUIPart *const part : m_vmultisel)
      part->m_selectstate = IWinUIPart::SelectState::NotSelected;

   //remove the clone of the multi selection in the smart browser class
   //to sync the clone and the actual multi-selection
   //it will be updated again on AddMultiSel() call
   m_vmultisel.clear();

#ifndef __STANDALONE__
   IWinUIPart *part = newSelPart;
   if (part == nullptr)
      part = &m_tablePart;
   if (part != nullptr)
   {
      m_vmultisel.push_back(part);
      part->m_selectstate = IWinUIPart::SelectState::Selected;
   }
#endif
}

bool PinTableWnd::MultiSelIsEmpty() const
{
   // empty selection means only the table itself is selected
   return m_vmultisel.empty() || (m_vmultisel.size() == 1 && m_vmultisel[0]->GetEditable() == m_table);
}

// 'update' tells us whether to go ahead and change the UI
// based on the new selection, or whether more stuff is coming
// down the pipe (speeds up drag-selection)
void PinTableWnd::AddMultiSel(IWinUIPart *const pselPart, const bool add, const bool update, const bool contextClick)
{
   if (pselPart == nullptr)
   {
      _ASSERTE(false); // Every selectable element is expected to have a UI part
      return;
   }
   const int index = FindIndexOf(m_vmultisel, pselPart);
   IWinUIPart *piSelect = nullptr;
   //_ASSERTE(m_vmultisel[0]->m_selectstate == IWinUIPart::SelectState::Selected);

   if (m_table->IsLocked())
      return;

   if (index == -1) // If we aren't selected yet, do that
   {
      _ASSERTE(pselPart->m_selectstate == IWinUIPart::SelectState::NotSelected);
      // If we non-shift click on an element outside the multi-select group, delete the old group
      // If the table is currently selected, deselect it - the table can not be part of a multi-select
      if (!add || MultiSelIsEmpty())
      {
         ClearMultiSel(pselPart);
         if (!add && !contextClick)
         {
            int colIndex = -1;
            int elemIndex = -1;
            if (m_table->GetCollectionIndex(pselPart->GetEditable(), colIndex, elemIndex))
            {
               CComObject<Collection> *col = m_table->GetCollections()[colIndex];
               if (col->m_groupElements)
               {
                  for (IEditable *const part : col->GetParts())
                  {
                     if (IWinUIPart *const uiPart = GetUIPart(part))
                     {
                        uiPart->m_selectstate = IWinUIPart::SelectState::MultiSelected;
                        // current element is already in m_vmultisel. (ClearMultiSel(psel) added it)
                        if (uiPart != pselPart)
                           m_vmultisel.push_back(uiPart);
                     }
                  }
               }
            }
         }
      }
      else
      {
         // Make this new selection the primary one for the group
         piSelect = m_vmultisel[0];
         if (piSelect != nullptr)
            piSelect->m_selectstate = IWinUIPart::SelectState::MultiSelected;
         m_vmultisel.insert(m_vmultisel.begin(), pselPart);
      }

      pselPart->m_selectstate = IWinUIPart::SelectState::Selected;

      if (update)
         m_table->SetDirtyDraw();
   }
   else if (add) // Take the element off the list
   {
      _ASSERTE(pselPart->m_selectstate != IWinUIPart::SelectState::NotSelected);
      m_vmultisel.erase(m_vmultisel.begin() + index);
      pselPart->m_selectstate = IWinUIPart::SelectState::NotSelected;
      if (m_vmultisel.empty())
      {
         // Have to have something selected
         ClearMultiSel();
      }
      // The main element might have changed
      piSelect = m_vmultisel.empty() ? nullptr : m_vmultisel[0];
      if (piSelect != nullptr)
         piSelect->m_selectstate = IWinUIPart::SelectState::Selected;

      if (update)
         m_table->SetDirtyDraw();
   }
   else if (m_vmultisel[0] != pselPart) // Object already in list - no change to selection, only to primary
   {
      int colIndex = -1;
      int elemIndex = -1;
      if (!m_table->GetCollectionIndex(pselPart->GetEditable(), colIndex, elemIndex))
      {
         _ASSERTE(pselPart->m_selectstate != IWinUIPart::SelectState::NotSelected);

         // Make this new selection the primary one for the group
         piSelect = m_vmultisel[0];
         if (piSelect != nullptr)
            piSelect->m_selectstate = IWinUIPart::SelectState::MultiSelected;
         m_vmultisel.erase(m_vmultisel.begin() + index);
         m_vmultisel.insert(m_vmultisel.begin(), pselPart);

         pselPart->m_selectstate = IWinUIPart::SelectState::Selected;
      }
      else
         ClearMultiSel(pselPart);

      if (update)
         m_table->SetDirtyDraw();
   }

   if (update)
   {
#ifndef __STANDALONE__
      m_vpxEditor->SetPropSel(m_vmultisel);
#endif
      if (!m_vmultisel.empty())
         m_vmultisel[0]->UpdateStatusBarInfo();
   }

   piSelect = m_vmultisel.empty() ? nullptr : m_vmultisel[0];
   if (piSelect && piSelect->GetEditable() && piSelect->GetEditable()->GetIScriptable())
   {
      string info = piSelect->GetEditable()->GetPathString(false);
      if (piSelect->GetItemType() == eItemPrimitive)
      {
         const Primitive *const prim = (Primitive *)piSelect->GetEditable();
         if (!prim->m_mesh.m_animationFrames.empty())
            info += " (animated " + std::to_string((uint32_t)prim->m_mesh.m_animationFrames.size() - 1) + " frames)";
      }
#ifndef __STANDALONE__
      m_vpxEditor->SetStatusBarElementInfo(info);
      m_pcv->SelectItem(piSelect->GetEditable()->GetIScriptable());
#endif
   }

#ifndef __STANDALONE__
   if (m_vpxEditor->GetLayersListDialog()->IsSyncedOnSelection())
      m_vpxEditor->GetLayersListDialog()->Update();
#endif
}

void PinTableWnd::RefreshProperties()
{
#ifndef __STANDALONE__
   m_vpxEditor->SetPropSel(m_vmultisel);
#endif
}

void PinTableWnd::UpdatePropertyImageList()
{
#ifndef __STANDALONE__
   // just update the combo boxes in the property dialog
   m_vpxEditor->GetPropertiesDocker()->GetContainProperties()->GetPropertyDialog()->UpdateTabs(GetMultiSelParts());
#endif
}

void PinTableWnd::UpdatePropertyMaterialList()
{
#ifndef __STANDALONE__
   // just update the combo boxes in the property dialog
   m_vpxEditor->GetPropertiesDocker()->GetContainProperties()->GetPropertyDialog()->UpdateTabs(GetMultiSelParts());
#endif
}

bool PinTableWnd::IsSubPartOfSelectedPart(const IWinUIPart *uiPart) const
{
   if (uiPart == nullptr || !uiPart->IsSubPart())
      return false;
   const IEditable *const owner = uiPart->GetEditable();
   for (const IWinUIPart *const part : m_vmultisel)
      if (!part->IsSubPart() && part->GetEditable() == owner)
         return true;
   return false;
}

vector<IWinUIPart *> PinTableWnd::GetSelectedParts() const { return m_vmultisel; }

void PinTableWnd::MoveSelection(const int from, const int to)
{
   if (from < 0 || to < 0 || from >= (int)m_vmultisel.size() || from == to)
      return;
   IWinUIPart *const part = m_vmultisel[from];
   m_vmultisel.erase(m_vmultisel.begin() + from);
   if (to >= (int)m_vmultisel.size())
      m_vmultisel.push_back(part);
   else
      m_vmultisel.insert(m_vmultisel.begin() + to, part);
}

void PinTableWnd::SelectItem(IScriptable *piscript)
{
   for (IEditable *const pedit : m_table->GetParts())
   {
      if (piscript == pedit->GetIScriptable())
      {
         if (IWinUIPart *const pisel = GetUIPart(pedit); pisel)
            AddMultiSel(pisel, false, true, false);
         break;
      }
   }
}

Vertex2D PinTableWnd::GetMultiSelCenter()
{
   float minx = FLT_MAX;
   float maxx = -FLT_MAX;
   float miny = FLT_MAX;
   float maxy = -FLT_MAX;

   for (IWinUIPart *const uiPart : m_vmultisel)
   {
      if (IsSubPartOfSelectedPart(uiPart))
         continue;
      const Vertex2D vCenter = uiPart->GetCenter();

      minx = min(minx, vCenter.x);
      maxx = max(maxx, vCenter.x);
      miny = min(miny, vCenter.y);
      maxy = max(maxy, vCenter.y);
   }

   return { (maxx + minx) * 0.5f, (maxy + miny) * 0.5f };
}

void PinTableWnd::FlipYMultiSel(const Vertex2D &pvCenter)
{
   m_table->BeginUndo();
   for (IWinUIPart *const uiPart : m_vmultisel)
   {
      if (IsSubPartOfSelectedPart(uiPart))
         continue;
      m_table->MarkForUndo(uiPart->GetEditable());
      if (uiPart->IsSubPart())
      {
         // Flip the sub element (drag point, light center) itself around the flip center
         uiPart->Translate(Vertex2D(0.f, -2.f * (uiPart->GetCenter().y - pvCenter.y)));
      }
      else
         uiPart->GetEditable()->FlipY(pvCenter);
   }
   m_table->EndUndo();
   m_table->SetDirtyDraw();
}

void PinTableWnd::FlipXMultiSel(const Vertex2D &pvCenter)
{
   m_table->BeginUndo();
   for (IWinUIPart *const uiPart : m_vmultisel)
   {
      if (IsSubPartOfSelectedPart(uiPart))
         continue;
      m_table->MarkForUndo(uiPart->GetEditable());
      if (uiPart->IsSubPart())
      {
         // Flip the sub element (drag point, light center) itself around the flip center
         uiPart->Translate(Vertex2D(-2.f * (uiPart->GetCenter().x - pvCenter.x), 0.f));
      }
      else
         uiPart->GetEditable()->FlipX(pvCenter);
   }
   m_table->EndUndo();
   m_table->SetDirtyDraw();
}

void PinTableWnd::RotateMultiSel(const float ang, const Vertex2D &pvCenter, const bool useElementCenter)
{
   m_table->BeginUndo();
   for (IWinUIPart *const uiPart : m_vmultisel)
   {
      if (IsSubPartOfSelectedPart(uiPart))
         continue;
      m_table->MarkForUndo(uiPart->GetEditable());
      if (uiPart->IsSubPart())
      {
         // Rotate the sub element (drag point, light center) itself around the rotation center
         const Vertex2D vCenter = uiPart->GetCenter();
         const float sn = sinf(ANGTORAD(ang));
         const float cs = cosf(ANGTORAD(ang));
         const float dx = vCenter.x - pvCenter.x;
         const float dy = vCenter.y - pvCenter.y;
         uiPart->Translate(Vertex2D(pvCenter.x + cs * dx - sn * dy - vCenter.x, pvCenter.y + cs * dy + sn * dx - vCenter.y));
      }
      else
         uiPart->GetEditable()->Rotate(ang, pvCenter, useElementCenter);
   }
   m_table->EndUndo();
   m_table->SetDirtyDraw();
}

void PinTableWnd::ScaleMultiSel(const float scalex, const float scaley, const Vertex2D &pvCenter, const bool useElementCenter)
{
   m_table->BeginUndo();
   for (IWinUIPart *const uiPart : m_vmultisel)
   {
      if (IsSubPartOfSelectedPart(uiPart))
         continue;
      m_table->MarkForUndo(uiPart->GetEditable());
      if (uiPart->IsSubPart())
      {
         // Scale the sub element (drag point, light center) position itself around the scale center
         const Vertex2D vCenter = uiPart->GetCenter();
         uiPart->Translate(Vertex2D(pvCenter.x + (vCenter.x - pvCenter.x) * scalex - vCenter.x, pvCenter.y + (vCenter.y - pvCenter.y) * scaley - vCenter.y));
      }
      else
         uiPart->GetEditable()->Scale(scalex, scaley, pvCenter, useElementCenter);
   }
   m_table->EndUndo();
   m_table->SetDirtyDraw();
}

void PinTableWnd::TranslateMultiSel(const Vertex2D &offset)
{
   m_table->BeginUndo();
   for (IWinUIPart *const uiPart : m_vmultisel)
   {
      if (IsSubPartOfSelectedPart(uiPart))
         continue;
      m_table->MarkForUndo(uiPart->GetEditable());
      uiPart->Translate(offset);
   }
   m_table->EndUndo();
   m_table->SetDirtyDraw();
}

bool PinTableWnd::FMutilSelLocked() const
{
   for (const IWinUIPart *const psel : GetSelectedParts())
      if (psel->GetEditable()->IsUILocked())
         return true;

   return false;
}

void PinTableWnd::LockElements()
{
   m_table->BeginUndo();
   const bool lock = !FMutilSelLocked();
   for (IWinUIPart *const psel : GetSelectedParts())
   {
      IEditable *const pedit = psel->GetEditable();
      if (pedit)
      {
         m_table->MarkForUndo(pedit);
         pedit->SetUILock(lock);
      }
   }
   m_table->EndUndo();
   m_table->SetDirtyDraw();
}

void PinTableWnd::AssignSelectionToPartGroup(PartGroup *group)
{
   m_table->BeginUndo();
   m_table->MarkForUndo(m_table);
   bool show = false, hide = false;
   for (const IEditable *const e : m_table->GetParts())
      if (e->GetPartGroup() == group)
      {
         show |= e->IsUIVisible(false);
         hide |= !e->IsUIVisible(false);
      }
   for (IWinUIPart *const uiPart : m_vmultisel)
   {
      IEditable *const pedit = uiPart->GetEditable();
      pedit->SetPartGroup(group);
      if (pedit->IsUIVisible(false) && hide && !show)
         pedit->SetUIVisible(false);
      else if (!pedit->IsUIVisible(false) && show && !hide)
         pedit->SetUIVisible(true);
   }
   m_table->EndUndo();
   m_table->SetDirtyDraw();
#ifndef __STANDALONE__
   m_vpxEditor->GetLayersListDialog()->Update();
#endif
}

void PinTableWnd::Copy(int x, int y)
{
#ifndef __STANDALONE__
   if (MultiSelIsEmpty()) // Can't copy table
      return;

   if (GetMultiSelCount() == 1)
   {
      // special check if the user selected a Control Point and wants to copy the coordinates
      IWinUIPart *const pItem = HitTest(x, y);
      if (pItem->GetItemType() == eItemDragPoint)
      {
         DragPoint *pPoint = pItem->GetDragPoint();
         pPoint->Copy();
         return;
      }
   }

   vector<IStream *> vstm;
   vector<IEditable *> copied;
   //m_vstmclipboard
   for (IWinUIPart *const psel : GetSelectedParts())
   {
      IEditable *const pe = psel->GetEditable();

      // Multi-select may contain a part together with its sub parts (drag points, light centers): copy each part only once
      if (FindIndexOf(copied, pe) != -1)
         continue;
      copied.push_back(pe);

      const HGLOBAL hglobal = GlobalAlloc(GMEM_MOVEABLE, 1);

      IStream *pstm;
      CreateStreamOnHGlobal(hglobal, TRUE, &pstm);

      const int type = pe->GetItemType();
      ULONG writ = 0;
      pstm->Write(&type, sizeof(int), &writ);

      BiffWriter writer(pstm, 0);
      pe->Save(writer, false);

      vstm.push_back(pstm);
   }

   m_vpxEditor->SetClipboard(&vstm);
#endif
}

void PinTableWnd::Paste(const bool atLocation, const int x, const int y)
{
#ifndef __STANDALONE__
   bool error = false;
   int cpasted = 0;

   if (GetMultiSelCount() == 1)
   {
      // User wants to paste the copied coordinates of a Control Point
      IWinUIPart *const pItem = HitTest(x, y);
      if (pItem->GetItemType() == eItemDragPoint)
      {
         DragPoint *const pPoint = pItem->GetDragPoint();
         pPoint->Paste();
         m_table->SetDirtyDraw();
         return;
      }
   }

   const IWinUIPart::AllowedViews currentView = m_vpxEditor->m_desktopBackdropView ? IWinUIPart::AllowedViews::Backglass : IWinUIPart::AllowedViews::Playfield;

   // Do a backwards loop, so that the primary selection we had when
   // copying will again be the primary selection, since it will be
   // selected last.  Purely cosmetic.
   for (SSIZE_T i = m_vpxEditor->m_vstmclipboard.size() - 1; i >= 0; i--)
   //for (size_t i=0; i<m_vpxEditor->m_vstmclipboard.size(); i++)
   {
      IStream *const pstm = m_vpxEditor->m_vstmclipboard[i];

      // Copy the stream data to memory
      LARGE_INTEGER offset {};
      ULARGE_INTEGER endPos {};
      pstm->Seek(offset, STREAM_SEEK_END, &endPos);
      pstm->Seek(offset, STREAM_SEEK_SET, nullptr);
      vector<uint8_t> data(static_cast<size_t>(endPos.QuadPart));
      ULONG read = 0;
      pstm->Read(data.data(), static_cast<ULONG>(data.size()), &read);

      const ItemTypeEnum type = *reinterpret_cast<const ItemTypeEnum *>(data.data());

      if (!IWinUIPart::IsViewAllowed(WinUIPartRegistry::GetAllowedViews(type), currentView))
      {
         error = true;
      }
      else
      {
         IEditable *const peditNew = EditableRegistry::Create(type);
         if (peditNew)
         {
            BiffReader reader(data.data() + sizeof(int), static_cast<uint32_t>(data.size() - sizeof(int)), CURRENT_FILE_FORMAT_VERSION, NULL, NULL);
            peditNew->Load(reader);
            peditNew->m_desktopBackdrop = m_vpxEditor->m_desktopBackdropView;
            //if the original name is not yet used, use that one (so there's nothing we have to do) otherwise add/increase the suffix until we find a name that's not used yet
            if (!m_table->IsNameUnique(peditNew->GetWName()))
            {
               //first remove the existing suffix
               const wstring input = peditNew->GetWName();
               size_t lastNonDigit = input.length();
               while (lastNonDigit > 0 && iswdigit(input[lastNonDigit - 1]))
                  --lastNonDigit;
               peditNew->SetName(m_table->GetUniqueName(input.substr(0, lastNonDigit)));
            }
            peditNew->SetPartGroup(m_vpxEditor->GetLayersListDialog()->GetSelectedPartGroup());

            m_table->AddPart(peditNew);

            AddMultiSel(GetUIPart(peditNew), (i != m_vpxEditor->m_vstmclipboard.size() - 1), true, false);
            cpasted++;
         }
         else
            error = true;
      }
   }
   m_vpxEditor->GetLayersListDialog()->Update();

   // Center view on newly created objects, if they are off the screen
   if ((cpasted > 0) && atLocation)
      TranslateMultiSel(TransformPoint(x, y) - GetMultiSelCenter());

   if (error)
      ShowError(LocalString(IDS_NOPASTEINVIEW).m_szbuffer);
#endif
}

void PinTableWnd::DeleteSelection()
{
#ifndef __STANDALONE__
   vector<IWinUIPart *> m_vseldelete;
   const vector<IWinUIPart *> selection = GetMultiSelParts();
   m_vseldelete.reserve(selection.size());

   for (IWinUIPart *const psel : selection)
   {
      // Sub parts (drag points, light centers) are deleted together with their owning part
      if (IsSubPartOfSelectedPart(psel))
         continue;
      // Can't delete these items yet - ClearMultiSel() will try to mark them as unselected
      m_vseldelete.push_back(psel);
      if (psel->GetItemType() == ItemTypeEnum::eItemPartGroup)
         for (const auto part : m_table->GetParts())
            if (part->GetPartGroup() == psel->GetEditable() && std::ranges::find(m_vseldelete, GetUIPart(part)) == m_vseldelete.end())
               m_vseldelete.push_back(GetUIPart(part));
   }

   ClearMultiSel();

   bool inCollection = false;
   for (IWinUIPart *const ptr : m_vseldelete)
   {
      for (auto pcol : m_table->GetCollections())
      {
         if (inCollection)
            break;
         for (const IEditable *const part : pcol->GetParts())
         {
            // Identify Editable in collection, as well as sub part of collection's editable (like light center for example)
            if (ptr->GetEditable() == part)
            {
               inCollection = true;
               break;
            }
         }
      }
   }
   if (inCollection)
   {
      const int ans = MessageBox(LocalString(IDS_DELETE_ELEMENTS).m_szbuffer /*"Selected elements are part of one or more collections.\nDo you really want to delete them?"*/,
         "Visual Pinball", MB_YESNO | MB_DEFBUTTON2);
      if (ans != IDYES)
         return;
   }

   m_table->BeginUndo();
   for (IWinUIPart *const ptr : m_vseldelete)
   {
      if (ptr->GetItemType() == ItemTypeEnum::eItemDragPoint)
      {
         // Deleting a drag point modifies its owning part
         if (DragPoint *const dpoint = ptr->GetDragPoint(); dpoint->CanDelete())
         {
            m_table->MarkForUndo(ptr->GetEditable());
            dpoint->Delete();
         }
      }
      else
      {
         IEditable *part = ptr->GetEditable();
         m_table->MarkForDelete(part);
         m_table->RemovePart(part);
         for (Collection *const pcollection : part->m_vCollection)
            pcollection->RemovePart(part);
      }
   }
   m_table->EndUndo();

   m_vpxEditor->GetLayersListDialog()->Update();
   // update properties to show the properties of the table
   RefreshProperties();
   OnPartChanged(m_table);

   m_table->SetDirtyDraw();
#endif
}

IWinUIPart *PinTableWnd::HitTest(const int x, const int y)
{
#ifdef __STANDALONE__
   return nullptr;
#else
   const CDC dc;

   const CRect rc = GetClientRect();

   HitSur phs(GetZoom(), GetViewOffset().x, GetViewOffset().y, rc.right - rc.left, rc.bottom - rc.top, x, y, &m_tablePart);
   HitSur phs2(GetZoom(), GetViewOffset().x, GetViewOffset().y, rc.right - rc.left, rc.bottom - rc.top, x, y, &m_tablePart);

   m_allHitElements.clear();

   RenderTable(&phs);

   for (IEditable *const ptr : m_table->GetParts())
   {
      if (ptr->m_desktopBackdrop == m_vpxEditor->m_desktopBackdropView)
      {
         if (IWinUIPart *const uiPart = GetUIPart(ptr))
            uiPart->UIRenderPass1(&phs2);
         IWinUIPart *const tmp = phs2.m_pselected;
         if (FindIndexOf(m_allHitElements, tmp) == -1 && tmp != nullptr && tmp != &m_tablePart)
         {
            m_allHitElements.push_back(tmp);
         }
      }
   }
   // it's possible that UIRenderPass1 doesn't find all elements (gates,plunger)
   // check here if everything was already stored in the list
   if (FindIndexOf(m_allHitElements, phs.m_pselected) == -1)
   {
      m_allHitElements.push_back(phs.m_pselected);
   }

   std::ranges::reverse(m_allHitElements.begin(), m_allHitElements.end());

   return phs.m_pselected;
#endif
}

#ifndef __STANDALONE__
void PinTableWnd::OnKeyDown(int key)
{
   const int shift = GetKeyState(VK_SHIFT) & 0x8000;
   //const int ctrl = GetKeyState(VK_CONTROL) & 0x8000;
   //const int alt = GetKeyState(VK_MENU) & 0x8000;

   switch (key)
   {
   case VK_DELETE: DeleteSelection(); break;

   case VK_LEFT:
   case VK_RIGHT:
   case VK_UP:
   case VK_DOWN:
   {
      m_table->BeginUndo();
      const float distance = shift ? 10.f : 1.f;
      for (IWinUIPart *const uiPart : m_vmultisel)
      {
         if (!IsSubPartOfSelectedPart(uiPart) && !uiPart->GetEditable()->IsUILocked()) // control points get lock info from parent - UNDONE - make this code snippet be in one place
         {
            switch (key)
            {
            case VK_LEFT:
               m_table->MarkForUndo(uiPart->GetEditable());
               uiPart->Translate(Vertex2D(-distance / GetZoom(), 0.f));
               break;

            case VK_RIGHT:
               m_table->MarkForUndo(uiPart->GetEditable());
               uiPart->Translate(Vertex2D(distance / GetZoom(), 0.f));
               break;

            case VK_UP:
               m_table->MarkForUndo(uiPart->GetEditable());
               uiPart->Translate(Vertex2D(0.f, -distance / GetZoom()));
               break;

            case VK_DOWN:
               m_table->MarkForUndo(uiPart->GetEditable());
               uiPart->Translate(Vertex2D(0.f, distance / GetZoom()));
               break;
            }
         }
      }
      m_table->EndUndo();
      Redraw();
   }
   break;
   }
}

void PinTableWnd::OnLeftDoubleClick(int x, int y)
{
   //::SendMessage(m_vpxEditor->m_hwnd, WM_SIZE, 0, 0);
}

void PinTableWnd::DoLeftButtonDown(int x, int y, bool zoomIn)
{
   const int ksshift = GetKeyState(VK_SHIFT);
   const int ksctrl = GetKeyState(VK_CONTROL);

   // set the focus of the window so all keyboard and mouse inputs are processed.
   // (this fixes the problem of selecting a element on the properties dialog, clicking on a table
   // object and not being able to use the cursor keys/wheely mouse)
   m_vpxEditor->SetFocus();

   m_ptLast.x = x;
   m_ptLast.y = y;

   if ((m_vpxEditor->m_ToolCur == ID_TABLE_MAGNIFY) || (ksctrl & 0x80000000))
   {
      SetViewOffset(TransformPoint(x, y));
      SetZoom(GetZoom() * (zoomIn ? 1.5f : 0.5f));
      Redraw();
   }
   else
   {
      IWinUIPart *const pisel = HitTest(x, y);

      const bool add = ((ksshift & 0x80000000) != 0);

      if (pisel == &m_tablePart && add)
      {
         // Can not include the table in multi-select
         // and table will not be unselected, because the
         // user might be drawing a box around other objects
         // to add them to the selection group
         m_tablePart.OnLButtonDown(x, y); // Start the band select
         return;
      }

      AddMultiSel(pisel, add, true, false);

      m_moving = true;
      for (IWinUIPart *const uiPart : m_vmultisel)
         // Skip the table's UI part: band select on empty space is handled above, a plain click must not start one
         if (uiPart != &m_tablePart)
            uiPart->OnLButtonDown(x, y);
   }
}

void PinTableWnd::UseTool(int x, int y, int tool)
{
   const Vertex2D v = TransformPoint(x, y);

   const ItemTypeEnum type = WinUIPartRegistry::TypeFromToolID(tool);
   IEditable *const pie = EditableRegistry::CreateAndInit(type, m_table, v.x, v.y);

   if (pie)
   {
      if (auto scriptable = pie->GetIScriptable(); scriptable)
         m_table->GetUniqueName(type, scriptable->m_wzName);
      pie->m_desktopBackdrop = m_vpxEditor->m_desktopBackdropView;
      m_table->AddPart(pie);
      pie->SetPartGroup(m_vpxEditor->GetLayersListDialog()->GetSelectedPartGroup());
      m_vpxEditor->GetLayersListDialog()->Update();

      OnPartChanged(m_table);

      m_table->BeginUndo();
      m_table->MarkForCreate(pie);
      m_table->EndUndo();
      AddMultiSel(GetUIPart(pie), false, true, false);
   }

   m_vpxEditor->ParseCommand(IDC_SELECT, false);
}

void PinTableWnd::OnLeftButtonDown(const short x, const short y)
{
   if ((m_vpxEditor->m_ToolCur == IDC_SELECT) || (m_vpxEditor->m_ToolCur == ID_TABLE_MAGNIFY))
   {
      DoLeftButtonDown(x, y, true);
   }
   else if (!m_table->IsLocked())
   {
      UseTool(x, y, m_vpxEditor->m_ToolCur);
   }
   SetFocus();
}

void PinTableWnd::OnLeftButtonUp(int x, int y)
{
   if (!m_tablePart.m_dragging) // Not doing band select
   {
      for (IWinUIPart *const uiPart : m_vmultisel)
         uiPart->OnLButtonUp(x, y);
      if (m_moving)
      {
         m_moving = false;
         m_vpxEditor->SetPropSel(m_vmultisel);
      }
   }
   else
   {
      m_tablePart.m_dragging = false;
      ReleaseCapture();
      if ((m_table->m_rcDragRect.left != m_table->m_rcDragRect.right) || (m_table->m_rcDragRect.top != m_table->m_rcDragRect.bottom))
      {
         vector<IWinUIPart *> vsel;

         const CDC &dc = m_mdiTable->GetDC();

         const CRect rc = m_mdiTable->GetClientRect();

         HitRectSur phrs(GetZoom(), GetViewOffset().x, GetViewOffset().y, rc.right - rc.left, rc.bottom - rc.top, &m_table->m_rcDragRect, &vsel);

         // Just want one rendering pass (no UIRenderPass1) so we don't select things twice
         RenderTable(&phrs);

         const int ksshift = GetKeyState(VK_SHIFT);
         const bool add = ((ksshift & 0x80000000) != 0);
         if (!add)
            ClearMultiSel();

         // Sub parts (drag points, light centers) are band-selected only when the band rectangle contains no main part
         bool subPartsOnly = true;
         for (const auto &ptr : vsel)
            subPartsOnly &= ptr->IsSubPart();

         if (!vsel.empty())
         {
            size_t lastItemForUpdate = -1;
            // first check which item is the last item to add to the multi selection
            for (size_t i = 0; i < vsel.size(); i++)
               if (vsel[i]->IsSubPart() == subPartsOnly)
                  lastItemForUpdate = i;

            for (size_t i = 0; i < vsel.size(); i++)
               if (vsel[i]->IsSubPart() == subPartsOnly)
                  AddMultiSel(vsel[i], true, (i == lastItemForUpdate), false); //last item updates the (multi-)selection in the editor
         }
      }
      Redraw();
   }
}

void PinTableWnd::OnRightButtonDown(int x, int y)
{
   OnLeftButtonUp(x, y); //corrects issue with left mouse button being in 'stuck down' position on a control point or object - BDS

   const int ks = GetKeyState(VK_CONTROL);

   if ((m_vpxEditor->m_ToolCur == ID_TABLE_MAGNIFY) || (ks & 0x80000000))
   {
      SetViewOffset(TransformPoint(x, y));
      SetZoom(GetZoom() * 0.5f);
      Redraw();
   }
   else
   {
      // keep the selection if clicking over a selected object, even if
      // the selected object is hidden behind other objects
      IWinUIPart *hit = HitTest(x, y);
      for (IWinUIPart *const uiPart : m_vmultisel)
      {
         if (FindIndexOf(m_allHitElements, uiPart) != -1)
         {
            // found a selected item - keep the current selection set
            // by re-selecting this item (which will also promote it
            // to the head of the selection list)
            hit = uiPart;
            break;
         }
      }

      // update the selection
      AddMultiSel(hit, false, true, false);
   }
}

void PinTableWnd::OnRightButtonUp(int x, int y)
{
   const int ks = GetKeyState(VK_CONTROL);

   // Only bring up context menu if we weren't in magnify mode
   if (!((m_vpxEditor->m_ToolCur == ID_TABLE_MAGNIFY) || (ks & 0x80000000)))
   {
      if (m_vmultisel.size() > 1)
      {
         DoContextMenu(x, y, IDR_MULTIMENU, &m_tablePart);
      }
      else if (!MultiSelIsEmpty())
      {
         IWinUIPart *const uiPart = m_vmultisel[0];
         DoContextMenu(x, y, uiPart->GetMenuId(), uiPart);
      }
      else
      {
         DoContextMenu(x, y, IDR_TABLEMENU, &m_tablePart);
      }
   }
}

void PinTableWnd::OnMouseMove(const int x, const int y)
{
   if (const bool middleMouseButtonPressed = ((GetKeyState(VK_MBUTTON) & 0x100) != 0); middleMouseButtonPressed)
   {
      // If the user holds the middle mouse button and moves the mouse everything is moved in the direction of the mouse was moved
      const float inv_zoom = 1.0f / GetZoom();
      m_table->m_winEditorViewOffset.x += static_cast<float>(m_oldMousePos.x - x) * inv_zoom;
      m_table->m_winEditorViewOffset.y += static_cast<float>(m_oldMousePos.y - y) * inv_zoom;
      //SetMyScrollInfo();
      Redraw();
   }
   else
   {
      const Vertex2D v = TransformPoint(x, y);
      m_vpxEditor->SetPosCur(v.x, v.y);

      if (!m_tablePart.m_dragging) // Not doing band select
      {
         if ((x != m_ptLast.x) || (y != m_ptLast.y))
         {
            for (IWinUIPart *const uiPart : m_vmultisel)
            {
               if (uiPart->m_dragging && !IsSubPartOfSelectedPart(uiPart) && !uiPart->GetEditable()->IsUILocked()) // For drag points, follow the lock of the parent
               {
                  if (!uiPart->m_markedForUndo)
                  {
                     uiPart->m_markedForUndo = true;
                     uiPart->GetEditable()->GetPTable()->BeginUndo();
                     m_table->MarkForUndo(uiPart->GetEditable());
                  }

                  const float inv_zoom = 1.0f / GetZoom();
                  uiPart->Translate(Vertex2D((float)(x - m_ptLast.x) * inv_zoom, (float)(y - m_ptLast.y) * inv_zoom));
                  uiPart->UpdateStatusBarObjectPos();
                  Redraw();
               }
            }
            m_ptLast.x = x;
            m_ptLast.y = y;
         }
      }
      else
      {
         const Vertex2D vec = TransformPoint(x, y);
         m_table->m_rcDragRect.right = vec.x;
         m_table->m_rcDragRect.bottom = vec.y;
         Redraw();
      }
   }

   m_oldMousePos.x = x;
   m_oldMousePos.y = y;
}

void PinTableWnd::OnMouseWheel(const short x, const short y, const short zDelta)
{
   const int ksctrl = GetKeyState(VK_CONTROL);
   if (ksctrl & 0x80000000)
   {
      POINT curpt;
      curpt.x = x;
      curpt.y = y;
      m_mdiTable->ScreenToClient(curpt);
      const short x2 = (short)curpt.x;
      const short y2 = (short)curpt.y;
      if ((m_vpxEditor->m_ToolCur == IDC_SELECT) || (m_vpxEditor->m_ToolCur == ID_TABLE_MAGNIFY))
      {
         DoLeftButtonDown(x2, y2, zDelta != -120);
      }
   }
   else
   {
      m_table->m_winEditorViewOffset.y -= (float)zDelta / GetZoom(); // change to orientation to match windows default
      Redraw();
      SetMyScrollInfo();
   }
}
#endif

void PinTableWnd::FillCollectionContextMenu(CMenu &mainMenu, CMenu &colSubMenu, IWinUIPart *psel)
{
#ifndef __STANDALONE__
   mainMenu.AppendMenu(MF_POPUP | MF_STRING, (size_t)colSubMenu.GetHandle(), LocalString(IDS_TO_COLLECTION).m_szbuffer);

   const int maxItems = (int)m_table->GetCollections().size() - 1;

   // run through all collections and list them in the context menu
   // the actual processing is done in IWinUIPart::DoCommand()
   for (int i = maxItems; i >= 0; i--)
   {
      UINT flags = MF_POPUP | MF_UNCHECKED;
      if ((maxItems - i) % 32 == 0) // add new column each 32 entries
         flags |= MF_MENUBREAK;
      colSubMenu.AppendMenu(flags, 0x40000 + i, MakeString(m_table->GetCollections()[i]->get_Name()).c_str());
   }
   if (m_vmultisel.size() == 1)
   {
      if (!psel->IsSubPart())
         for (int i = maxItems; i >= 0; i--)
            for (const IEditable *const part : m_table->GetCollections()[i]->GetParts())
               if (psel->GetEditable() == part)
                  colSubMenu.CheckMenuItem(0x40000 + i, MF_CHECKED);
   }
   else
   {
      vector<int> allIndices;

      for (const IWinUIPart *const uiPart : m_vmultisel)
      {
         if (uiPart->IsSubPart())
            continue;
         const IEditable *const editable = uiPart->GetEditable();

         for (int i = maxItems; i >= 0; i--)
            for (const IEditable *const part : m_table->GetCollections()[i]->GetParts())
               if (editable == part)
                  allIndices.push_back(i);
      }
      for (size_t i = 0; i < allIndices.size(); i++)
         colSubMenu.CheckMenuItem(0x40000 + allIndices[i], MF_CHECKED);
   }
#endif
}

void PinTableWnd::NewCollection(const HWND hwndListView, const bool fromSelection)
{
   CComObject<Collection> *pcol;
   CComObject<Collection>::CreateInstance(&pcol);
   pcol->AddRef();

   pcol->m_wzName = m_table->GetUniqueName(LocalStringW(IDS_COLLECTION).m_buffer);

   if (fromSelection && !MultiSelIsEmpty())
   {
      for (IWinUIPart *const uiPart : m_vmultisel)
      {
         IEditable *const piedit = uiPart->GetEditable();
         if (piedit)
         {
            if (!uiPart->IsSubPart()) // Do this check so we don't put walls in a collection when we only have the control point selected
            {
               piedit->m_vCollection.push_back(pcol);
               piedit->m_viCollection.push_back(static_cast<int>(pcol->GetParts().size()));
               pcol->AddPart(piedit);
            }
         }
      }
   }

   const int index = AddListCollection(hwndListView, pcol);

#ifndef __STANDALONE__
   ListView_SetItemState(hwndListView, index, LVIS_SELECTED, LVIS_SELECTED);
#endif

   m_table->AddCollection(pcol);
   pcol->Release();
}

int PinTableWnd::AddListCollection(HWND hwndListView, CComObject<Collection> *pcol)
{
#ifndef __STANDALONE__
   LVITEM lvitem;
   lvitem.mask = LVIF_DI_SETITEM | LVIF_TEXT | LVIF_PARAM;
   lvitem.iItem = 0;
   lvitem.iSubItem = 0;
   string name = MakeString(pcol->m_wzName);
   lvitem.pszText = name.data();
   lvitem.lParam = (size_t)pcol;

   const int index = ListView_InsertItem(hwndListView, &lvitem);
   ListView_SetItemText_Safe(hwndListView, index, 1, std::to_string(pcol->GetParts().size()).c_str());
   return index;
#else
   return 0;
#endif
}

void PinTableWnd::ListCollections(HWND hwndListView)
{
   //ListView_DeleteAllItems(hwndListView);

   for (auto pcol : m_table->GetCollections())
      AddListCollection(hwndListView, pcol);
}

void PinTableWnd::ImportFont(HWND hwndListView, const string &filename)
{
#ifndef __STANDALONE__
   PinFont *const ppb = new PinFont();

   ppb->ReadFromFile(filename);

   if (!ppb->m_buffer.empty())
   {
      m_table->AddFont(ppb);
      const int index = AddListBinary(hwndListView, ppb);
      ListView_SetItemState(hwndListView, index, LVIS_SELECTED, LVIS_SELECTED);
      ppb->Register();
   }
   else
      delete ppb;
#endif
}

void PinTableWnd::ListFonts(HWND hwndListView)
{
   for (PinFont *font : m_table->GetFontList())
      AddListBinary(hwndListView, font);
}

int PinTableWnd::AddListBinary(HWND hwndListView, PinBinary *ppb)
{
#ifndef __STANDALONE__
   LVITEM lvitem;
   lvitem.mask = LVIF_DI_SETITEM | LVIF_TEXT | LVIF_PARAM;
   lvitem.iItem = 0;
   lvitem.iSubItem = 0;
   lvitem.pszText = (LPSTR)ppb->m_name.c_str();
   lvitem.lParam = (size_t)ppb;

   const int index = ListView_InsertItem(hwndListView, &lvitem);

   ListView_SetItemText_Safe(hwndListView, index, 1, ppb->m_path.string().c_str());

   return index;
#else
   return 0;
#endif
}

void PinTableWnd::FillLayerContextMenu(CMenu &mainMenu, CMenu &layerSubMenu, IWinUIPart *psel)
{
#ifndef __STANDALONE__
   mainMenu.AppendMenu(MF_POPUP | MF_STRING, (size_t)layerSubMenu.GetHandle(), LocalString(IDS_ASSIGN_TO_LAYER2).m_szbuffer);
   int i = 0;
   for (const IEditable * const edit : m_table->GetParts())
   {
      if (edit->GetItemType() == eItemPartGroup && edit->GetPartGroup() == nullptr)
      {
         layerSubMenu.AppendMenu(MF_STRING, ID_ASSIGN_TO_LAYER1 + i, edit->GetName().c_str());
         i++;
         if (i == NUM_ASSIGN_LAYERS)
            break;
      }
   }
#endif
}

#ifndef __STANDALONE__
void PinTableWnd::DoContextMenu(int x, int y, const int menuid, IWinUIPart *const uiPart)
{
   POINT pt;
   pt.x = x;
   pt.y = y;
   m_mdiTable->ClientToScreen(pt);

   CMenu mainMenu;
   CMenu newMenu;

   mainMenu.LoadMenu(menuid);
   if (menuid != -1)
      newMenu = mainMenu.GetSubMenu(0);
   else
      newMenu.CreatePopupMenu();

   if (uiPart)
      uiPart->EditMenu(newMenu);

   if (menuid != IDR_POINTMENU && menuid != IDR_TABLEMENU && menuid != IDR_POINTMENU_SMOOTH)
   {

      if (newMenu.GetMenuItemCount() > 0)
         newMenu.AppendMenu(MF_SEPARATOR, ~0u, "");

      CMenu assignLayerMenu;
      CMenu colSubMenu;
      CMenu layerSubMenu;

      assignLayerMenu.CreatePopupMenu();
      colSubMenu.CreatePopupMenu();
      layerSubMenu.CreatePopupMenu();

      // TEXT
      newMenu.AppendMenu(MF_STRING, IDC_COPY, LocalString(IDS_COPY_ELEMENT).m_szbuffer);
      newMenu.AppendMenu(MF_STRING, IDC_PASTE, LocalString(IDS_PASTE_ELEMENT).m_szbuffer);
      newMenu.AppendMenu(MF_STRING, IDC_PASTEAT, LocalString(IDS_PASTE_AT_ELEMENT).m_szbuffer);

      newMenu.AppendMenu(MF_SEPARATOR, ~0u, "");

      newMenu.AppendMenu(MF_STRING, ID_EDIT_DRAWINGORDER_HIT, LocalString(IDS_DRAWING_ORDER_HIT).m_szbuffer);
      newMenu.AppendMenu(MF_STRING, ID_EDIT_DRAWINGORDER_SELECT, LocalString(IDS_DRAWING_ORDER_SELECT).m_szbuffer);

      newMenu.AppendMenu(MF_STRING, ID_DRAWINFRONT, LocalString(IDS_DRAWINFRONT).m_szbuffer);
      newMenu.AppendMenu(MF_STRING, ID_DRAWINBACK, LocalString(IDS_DRAWINBACK).m_szbuffer);

      newMenu.AppendMenu(MF_STRING, ID_SETASDEFAULT, LocalString(IDS_SETASDEFAULT).m_szbuffer);

      newMenu.AppendMenu(MF_SEPARATOR, ~0u, "");

      FillLayerContextMenu(newMenu, layerSubMenu, uiPart);
      layerSubMenu.Detach();
      newMenu.AppendMenu(MF_STRING, ID_ASSIGN_TO_CURRENT_LAYER, LocalString(IDS_ASSIGN_TO_CURRENT_LAYER).m_szbuffer);
      FillCollectionContextMenu(newMenu, colSubMenu, uiPart);
      colSubMenu.Detach();

      newMenu.AppendMenu(MF_STRING, ID_LOCK, LocalString(IDS_LOCK).m_szbuffer);

      newMenu.AppendMenu(MF_SEPARATOR, ~0u, "");
      newMenu.AppendMenu(MF_SEPARATOR, ~0u, "");
      // now list all elements that are stacked at the mouse pointer
      for (size_t i = 0; i < m_allHitElements.size(); i++)
      {
         if (!m_allHitElements[i]->GetEditable()->IsUIVisible(false))
            continue;

         IWinUIPart *const ptr = m_allHitElements[i];
         if (ptr)
         {
            IEditable *const pedit = m_allHitElements[i]->GetEditable();
            if (pedit)
            {
               const string szTemp = pedit->GetName();
               if (!szTemp.empty())
               {
                  //!! what a hack!
                  // the element index of the allHitElements vector is encoded inside the ID of the context menu item
                  // I didn't find an easy way to identify the selected menu item of a context menu
                  // so the ID_SELECT_ELEMENT is the global ID for selecting an element from the list and the rest is
                  // added for finding the element out of the list
                  // the selection is done in IWinUIPart::DoCommand()
                  const UINT_PTR id = 0x80000000 + ((UINT_PTR)i << 16) + ID_SELECT_ELEMENT;
                  newMenu.AppendMenu(MF_STRING, id, szTemp.c_str());
               }
            }
         }
      }
      bool locked = uiPart->GetEditable()->IsUILocked();
      //!! HACK
      if (uiPart->GetEditable() == m_table) // multi-select case
      {
         locked = FMutilSelLocked();
      }
      newMenu.CheckMenuItem(ID_LOCK, MF_BYCOMMAND | (locked ? MF_CHECKED : MF_UNCHECKED));
   }

   const int icmd = newMenu.TrackPopupMenuEx(TPM_RETURNCMD, pt.x, pt.y, m_mdiTable->GetHwnd(), nullptr);

   if (icmd != 0 && uiPart)
      uiPart->DoCommand(icmd, x, y);

   newMenu.Destroy();

   if (menuid != -1)
      mainMenu.Destroy();
}
#endif


void PinTableWnd::BeginAutoSaveCounter()
{
#ifndef __STANDALONE__
   if (m_vpxEditor->m_autosaveTime > 0)
      m_vpxEditor->SetTimer(WinEditor::TIMER_ID_AUTOSAVE, m_vpxEditor->m_autosaveTime, nullptr);
#endif
}

void PinTableWnd::EndAutoSaveCounter()
{
#ifndef __STANDALONE__
   m_vpxEditor->KillTimer(WinEditor::TIMER_ID_AUTOSAVE);
#endif
}

void PinTableWnd::AutoSave()
{
#ifndef __STANDALONE__
   if (m_table->m_sdsCurrentDirtyState <= eSaveAutosaved)
      return;

   m_vpxEditor->KillTimer(WinEditor::TIMER_ID_AUTOSAVE);

   m_vpxEditor->SetActionCur(LocalString(IDS_AUTOSAVING).m_szbuffer);
   m_vpxEditor->SetCursorCur(IDC_WAIT);

   FastIStorage *const pstgroot = new FastIStorage();
   pstgroot->AddRef();

   Win32ProgressBar feedback(g_app->GetInstanceHandle(), m_vpxEditor->m_hwndStatusBar);
   const HRESULT hr = m_table->SaveToStorage(pstgroot, feedback);

   m_table->SetCleanPoint((SaveDirtyState)min((int)m_table->m_sdsDirtyProp, (int)eSaveAutosaved));
   m_pcv->SetClean((SaveDirtyState)min((int)m_table->m_sdsDirtyScript, (int)eSaveAutosaved));
   m_table->SetNonUndoableDirty((SaveDirtyState)min((int)m_table->m_sdsNonUndoableDirty, (int)eSaveAutosaved));

   AutoSavePackage *const pasp = new AutoSavePackage();
   pasp->pstg = pstgroot;
   pasp->tableindex = FindIndexOf(m_vpxEditor->m_vtable, this);
   pasp->hwndtable = GetHwnd();
   pasp->table = m_table;

   if (hr == S_OK)
   {
      const HANDLE hEvent = m_vpxEditor->PostWorkToWorkerThread(COMPLETE_AUTOSAVE, (LPARAM)pasp);
      m_vAsyncHandles.push_back(hEvent);

      m_vpxEditor->SetActionCur("Completing AutoSave"s);
   }
   else
   {
      m_vpxEditor->SetActionCur(string());
   }

   m_vpxEditor->SetCursorCur(IDC_ARROW);
#endif
}

void PinTableWnd::FVerifySaveToClose()
{
#ifndef __STANDALONE__
   if (!m_vAsyncHandles.empty())
   {
      /*const DWORD wait =*/WaitForMultipleObjects((DWORD)m_vAsyncHandles.size(), m_vAsyncHandles.data(), TRUE, INFINITE);
      //m_vpinball->MessageBox("Async work items not done", nullptr, 0);

      // Close the remaining handles here, since the window messages will never be processed
      for (size_t i = 0; i < m_vAsyncHandles.size(); i++)
         CloseHandle(m_vAsyncHandles[i]);

      m_vpxEditor->SetActionCur(string());
   }
#endif
}

void PinTableWnd::OnPartAdded(IEditable *part)
{
#ifndef __STANDALONE__
   if (std::unique_ptr<IWinUIPart> uiPart = WinUIPartRegistry::Create(this, part))
      m_uiParts[part] = std::move(uiPart);
#endif
}

void PinTableWnd::OnPartRemoved(IEditable *part)
{
#ifndef __STANDALONE__
   // The selection stores raw pointers to UI parts: remove the part and its sub parts (drag points, light centers)
   // before destroying the corresponding UI part
   for (int i = (int)m_vmultisel.size() - 1; i >= 0; i--)
      if (m_vmultisel[i]->GetEditable() == part)
         m_vmultisel.erase(m_vmultisel.begin() + i);
   if (m_vmultisel.empty())
      ClearMultiSel();
   m_uiParts.erase(part);
#endif
}

IWinUIPart *PinTableWnd::GetUIPart(IEditable *part)
{
   if (part == nullptr)
      return nullptr;
#ifndef __STANDALONE__
   // The UI part of the table itself is not stored in the map but directly owned by this editor (m_tablePart)
   if (part == m_table)
      return &m_tablePart;
#endif
   const auto it = m_uiParts.find(part);
   return it != m_uiParts.end() ? it->second.get() : nullptr;
}

IWinUIPart *PinTableWnd::GetUIPart(DragPoint *point)
{
   if (point == nullptr)
      return nullptr;
   IWinUIPart *const ownerPart = GetUIPart(point->GetIEditable());
   return ownerPart ? ownerPart->GetSubPart(point) : nullptr;
}

void PinTableWnd::OnPartChanged(IEditable *part)
{
#ifndef __STANDALONE__
   switch (part->GetItemType())
   {
   case eItemTable:
      Redraw();
      SetMyScrollInfo();
      m_vpxEditor->GetLayersListDialog()->Update();
      if (m_searchSelectDlg)
         m_searchSelectDlg->Update();
      break;

   default:
      // Not yet implemented
      assert(false);
   }
#endif
}

void PinTableWnd::ShowSearchSelectDlg()
{
#ifndef __STANDALONE__
   if (m_searchSelectDlg == nullptr || !m_searchSelectDlg->IsWindow())
   {
      m_searchSelectDlg = std::make_unique<SearchSelectDialog>(this);
      m_searchSelectDlg->Create(GetHwnd());
   }
   m_searchSelectDlg->ShowWindow();
   m_searchSelectDlg->SetForegroundWindow();
#endif
}
