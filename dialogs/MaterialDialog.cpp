#include "StdAfx.h"
#include "resource.h"
#include "MaterialDialog.h"
#include "vpversion.h"
#include <rapidxml.hpp>
#include <rapidxml_print.hpp>
#include <fstream>
#include <sstream>

using namespace rapidxml;

typedef struct _tagSORTDATA
{
    HWND hwndList;
    int subItemIndex;
    int sortUpDown;
}SORTDATA;

extern SORTDATA SortData;
extern int CALLBACK MyCompProc( LPARAM lSortParam1, LPARAM lSortParam2, LPARAM lSortOption );

int MaterialDialog::m_columnSortOrder;
bool MaterialDialog::m_deletingItem;

void MaterialDialog::DisableAllMaterialDialogItems()
{
    ::EnableWindow(GetDlgItem( IDC_DIFFUSE_CHECK ).GetHwnd(), FALSE);
    ::EnableWindow(GetDlgItem(IDC_DIFFUSE_EDIT).GetHwnd(), FALSE);
    ::EnableWindow(GetDlgItem(IDC_GLOSSY_EDIT).GetHwnd(), FALSE);
    ::EnableWindow(GetDlgItem(IDC_GLOSSY_IMGLERP_EDIT).GetHwnd(), FALSE);
    ::EnableWindow(GetDlgItem(IDC_THICKNESS_EDIT).GetHwnd(), FALSE);
    ::EnableWindow(GetDlgItem(IDC_SPECULAR_EDIT).GetHwnd(), FALSE);
    ::EnableWindow(GetDlgItem(IDC_OPACITY_EDIT).GetHwnd(), FALSE);
    ::EnableWindow(GetDlgItem(IDC_OPACITY_CHECK).GetHwnd(), FALSE);
    ::EnableWindow(GetDlgItem(IDC_EDGEALPHA_EDIT).GetHwnd(), FALSE);
    ::EnableWindow(GetDlgItem(IDC_CLONE_BUTTON).GetHwnd(), FALSE);
    ::EnableWindow(GetDlgItem(IDC_RENAME).GetHwnd(), FALSE);
    ::EnableWindow(GetDlgItem(IDC_IMPORT).GetHwnd(), FALSE);
}

void MaterialDialog::EnableAllMaterialDialogItems()
{
   ::EnableWindow(GetDlgItem(IDC_DIFFUSE_CHECK).GetHwnd(), TRUE);
   ::EnableWindow(GetDlgItem(IDC_DIFFUSE_EDIT).GetHwnd(), TRUE);
   ::EnableWindow(GetDlgItem(IDC_GLOSSY_EDIT).GetHwnd(), TRUE);
   ::EnableWindow(GetDlgItem(IDC_GLOSSY_IMGLERP_EDIT).GetHwnd(), TRUE);
   ::EnableWindow(GetDlgItem(IDC_THICKNESS_EDIT).GetHwnd(), TRUE);
   ::EnableWindow(GetDlgItem(IDC_SPECULAR_EDIT).GetHwnd(), TRUE);
   ::EnableWindow(GetDlgItem(IDC_OPACITY_EDIT).GetHwnd(), TRUE);
   ::EnableWindow(GetDlgItem(IDC_OPACITY_CHECK).GetHwnd(), TRUE);
   ::EnableWindow(GetDlgItem(IDC_EDGEALPHA_EDIT).GetHwnd(), TRUE);
   ::EnableWindow(GetDlgItem(IDC_CLONE_BUTTON).GetHwnd(), TRUE);
   ::EnableWindow(GetDlgItem(IDC_RENAME).GetHwnd(), TRUE);
   ::EnableWindow(GetDlgItem(IDC_IMPORT).GetHwnd(), TRUE);
}

float MaterialDialog::getItemText(int id)
{
   CString textStr;
   float fv;
   textStr = GetDlgItemText(id);
   fv = sz2f((char*)textStr.c_str());
   return fv;
}

void MaterialDialog::setItemText(int id, float value)
{
   char textBuf[256] = { 0 };
   f2sz(value, textBuf);
   CString textStr(textBuf);
   SetDlgItemText(id, textStr);
}

MaterialDialog::MaterialDialog() : CDialog(IDD_MATERIALDIALOG)
{
    m_hMaterialList = NULL;
}

BOOL MaterialDialog::OnInitDialog()
{
   LVCOLUMN lvcol;
   m_hMaterialList = GetDlgItem(IDC_MATERIAL_LIST).GetHwnd();
   CCO(PinTable) *pt = (CCO(PinTable) *)g_pvp->GetActiveTable();

   m_columnSortOrder = 1;
   m_deletingItem = false;
   m_resizer.Initialize(*this, CRect(0, 0, 500, 600));
   m_resizer.AddChild(m_hMaterialList, topleft, RD_STRETCH_WIDTH | RD_STRETCH_HEIGHT);
   m_resizer.AddChild(GetDlgItem(IDC_DIFFUSE_CHECK).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC_BASE_COLOR).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC_GLOSSY_LAYER).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC_CLEARCOAR_LAYER).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC_OPACITY).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC_PHYSICS).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_COLOR).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_COLOR2).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_COLOR3).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC_WRAP).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_DIFFUSE_EDIT).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC_WRAP_TEXT).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC_GLOSSY_TEXT).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC_SHININESS).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC_GLOSSY_IMGLERP).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC_THICKNESS).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC_THICKNESS_TEXT).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_GLOSSY_EDIT).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_GLOSSY_IMGLERP_EDIT).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_THICKNESS_EDIT).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC_SHININESS_TEXT).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC_CLEARCOAT_TEXT).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC_EDGE_BRIGHTNESS).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_SPECULAR_EDIT).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC_BRIGHTNESS_TEXT).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC_AMOUNT).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_OPACITY_EDIT).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_OPACITY_CHECK).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC_EDGE_OPACITY).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_EDGEALPHA_EDIT).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC_EDGE_OPACITY_TEXT).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC_PHY_ELASTICITY).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_MAT_ELASTICITY).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC_PHY_ELASTICITY_FALLOFF).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_MAT_ELASTICITY_FALLOFF).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC_PHY_FRICTION).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_MAT_FRICTION).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC_PHY_SCATTER_ANGLE).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_MAT_SCATTER_ANGLE).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_ADD_BUTTON).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_CLONE_BUTTON).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_IMPORT).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_EXPORT).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_RENAME).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_DELETE_MATERIAL).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDOK).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDCANCEL).GetHwnd(), topright, 0);

   LoadPosition();
   ListView_SetExtendedListViewStyle(m_hMaterialList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
   lvcol.mask = LVCF_TEXT | LVCF_WIDTH;
   LocalString ls(IDS_NAME);
   lvcol.pszText = ls.m_szbuffer;// = "Name";
   lvcol.cx = 230;
   ListView_InsertColumn(m_hMaterialList, 0, &lvcol);

   lvcol.mask = LVCF_TEXT | LVCF_WIDTH;
   LocalString ls2(IDS_USED_IN_TABLE);
   lvcol.pszText = ls2.m_szbuffer;// = "Used in Table";
   lvcol.cx = 50;
   ListView_InsertColumn(m_hMaterialList, 1, &lvcol);
   pt->ListMaterials(m_hMaterialList);

   ListView_SetItemState(m_hMaterialList, 0, LVIS_SELECTED, LVIS_SELECTED)
   
   return TRUE;
}

BOOL MaterialDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
   CCO(PinTable) *pt = (CCO(PinTable) *)g_pvp->GetActiveTable();
   UNREFERENCED_PARAMETER(lParam);
   switch (HIWORD(wParam))
   {
      case COLOR_CHANGED:
      {
         const int count = ListView_GetSelectedCount(m_hMaterialList);
         if (count > 0)
         {
            const size_t color = ::GetWindowLongPtr((HWND)lParam, GWLP_USERDATA);
            HWND hwndcolor1 = GetDlgItem(IDC_COLOR).GetHwnd();
            HWND hwndcolor2 = GetDlgItem(IDC_COLOR2).GetHwnd();
            HWND hwndcolor3 = GetDlgItem(IDC_COLOR3).GetHwnd();
            int sel = ListView_GetNextItem(m_hMaterialList, -1, LVNI_SELECTED);
            while (sel != -1)
            {
               LVITEM lvitem;
               lvitem.mask = LVIF_PARAM;
               lvitem.iItem = sel;
               lvitem.iSubItem = 0;
               ListView_GetItem(m_hMaterialList, &lvitem);
               Material * const pmat = (Material*)lvitem.lParam;
               if (hwndcolor1 == (HWND)lParam)
                  pmat->m_cBase = (COLORREF)color;
               else if (hwndcolor2 == (HWND)lParam)
                  pmat->m_cGlossy = (COLORREF)color;
               else if (hwndcolor3 == (HWND)lParam)
                  pmat->m_cClearcoat = (COLORREF)color;

               // The previous selection is now deleted, so look again from the top of the list
               sel = ListView_GetNextItem(m_hMaterialList, sel, LVNI_SELECTED);
            }
            pt->SetNonUndoableDirty(eSaveDirty);
         }
         break;
      }
   }

   switch (LOWORD(wParam))
   {
      case IDC_CLONE_BUTTON:
      {
         if (ListView_GetSelectedCount(m_hMaterialList))	// if some items are selected???
         {
            int sel = ListView_GetNextItem(m_hMaterialList, -1, LVNI_SELECTED);
            int selCount = ListView_GetSelectedCount(m_hMaterialList);
            if (sel == -1)
               break;

            while (sel != -1)
            {
               LVITEM lvitem;
               lvitem.mask = LVIF_PARAM;
               lvitem.iItem = sel;
               lvitem.iSubItem = 0;
               ListView_GetItem(m_hMaterialList, &lvitem);
               Material *pNewMat = new Material();
               Material * const pmat = (Material*)lvitem.lParam;
               pNewMat->m_bIsMetal = pmat->m_bIsMetal;
               pNewMat->m_bOpacityActive = pmat->m_bOpacityActive;
               pNewMat->m_cBase = pmat->m_cBase;
               pNewMat->m_cClearcoat = pmat->m_cClearcoat;
               pNewMat->m_cGlossy = pmat->m_cGlossy;
               pNewMat->m_fEdge = pmat->m_fEdge;
               pNewMat->m_fEdgeAlpha = pmat->m_fEdgeAlpha;
               pNewMat->m_fOpacity = pmat->m_fOpacity;
               pNewMat->m_fRoughness = pmat->m_fRoughness;
               pNewMat->m_fGlossyImageLerp = pmat->m_fGlossyImageLerp;
               pNewMat->m_fThickness = pmat->m_fThickness;
               pNewMat->m_fWrapLighting = pmat->m_fWrapLighting;
               memcpy(pNewMat->m_szName, pmat->m_szName, 32);

               pNewMat->m_fElasticity = pmat->m_fElasticity;
               pNewMat->m_fElasticityFalloff = pmat->m_fElasticityFalloff;
               pNewMat->m_fFriction = pmat->m_fFriction;
               pNewMat->m_fScatterAngle = pmat->m_fScatterAngle;
               pt->AddMaterial(pNewMat);
               pt->AddListMaterial(m_hMaterialList, pNewMat);

               sel = ListView_GetNextItem(m_hMaterialList, sel, LVNI_SELECTED);
            }
            g_pvp->m_sb.PopulateDropdowns(); // May need to update list of images
            g_pvp->m_sb.RefreshProperties();
            pt->SetNonUndoableDirty(eSaveDirty);
         }
         break;
      }
      case IDC_ADD_BUTTON:
      {
         Material *pmat = new Material();

         pt->AddMaterial(pmat);
         pt->AddListMaterial(m_hMaterialList, pmat);
         g_pvp->m_sb.PopulateDropdowns(); // May need to update list of images
         g_pvp->m_sb.RefreshProperties();
         pt->SetNonUndoableDirty(eSaveDirty);

         break;
      }
      case IDC_IMPORT:
      {
         char szFileName[4096];
         char szInitialDir[4096];
         szFileName[0] = '\0';

         OPENFILENAME ofn;
         ZeroMemory(&ofn, sizeof(OPENFILENAME));
         ofn.lStructSize = sizeof(OPENFILENAME);
         ofn.hInstance = g_hinst;
         ofn.hwndOwner = g_pvp->m_hwnd;

         ofn.lpstrFilter = "Material Files (.mat)\0*.mat\0";
         ofn.lpstrFile = szFileName;
         ofn.nMaxFile = 4096;
         ofn.lpstrDefExt = "mat";
         ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY | OFN_EXPLORER;

         HRESULT hr = GetRegString("RecentDir", "MaterialDir", szInitialDir, 4096);
         ofn.lpstrInitialDir = (hr == S_OK) ? szInitialDir : NULL;

         const int ret = GetOpenFileName(&ofn);

         if (ret)
         {
            int materialCount = 0;
            int versionNumber = 0;
            FILE *f;
            fopen_s(&f, ofn.lpstrFile, "rb");

            fread(&versionNumber, 1, 4, f);
            if (versionNumber != 1)
            {
               ShowError("Materials are not compatible with this version!");
               fclose(f);
               break;
            }
            fread(&materialCount, 1, 4, f);
            for (int i = 0; i < materialCount; i++)
            {
               Material *pmat = new Material();
               SaveMaterial mat;
               float physicsValue;

               fread(&mat, 1, sizeof(SaveMaterial), f);
               pmat->m_cBase = mat.cBase;
               pmat->m_cGlossy = mat.cGlossy;
               pmat->m_cClearcoat = mat.cClearcoat;
               pmat->m_fWrapLighting = mat.fWrapLighting;
               pmat->m_fRoughness = mat.fRoughness;
               pmat->m_fGlossyImageLerp = dequantizeUnsigned<8>(mat.fGlossyImageLerp);
               pmat->m_fThickness = dequantizeUnsigned<8>(mat.fThickness);
               pmat->m_fEdge = mat.fEdge;
               pmat->m_bIsMetal = mat.bIsMetal;
               pmat->m_fOpacity = mat.fOpacity;
               pmat->m_bOpacityActive = !!(mat.bOpacityActive_fEdgeAlpha & 1);
               pmat->m_fEdgeAlpha = dequantizeUnsigned<7>(mat.bOpacityActive_fEdgeAlpha >> 1);
               memcpy(pmat->m_szName, mat.szName, 32);

               fread(&physicsValue, 1, sizeof(float), f);
               pmat->m_fElasticity = physicsValue;
               fread(&physicsValue, 1, sizeof(float), f);
               pmat->m_fElasticityFalloff = physicsValue;
               fread(&physicsValue, 1, sizeof(float), f);
               pmat->m_fFriction = physicsValue;
               fread(&physicsValue, 1, sizeof(float), f);
               pmat->m_fScatterAngle = physicsValue;

               pt->AddMaterial(pmat);
               pt->AddListMaterial(m_hMaterialList, pmat);
            }
            fclose(f);
            SetRegValue("RecentDir", "MaterialDir", REG_SZ, szInitialDir, lstrlen(szInitialDir));
            pt->SetNonUndoableDirty(eSaveDirty);
            g_pvp->m_sb.PopulateDropdowns(); // May need to update list of images
            g_pvp->m_sb.RefreshProperties();
         }
         break;
      }
      case IDC_RENAME:
      {
         const int sel = ListView_GetNextItem(m_hMaterialList, -1, LVNI_SELECTED);
         if (sel != -1)
         {
            ::SetFocus(m_hMaterialList);
            ListView_EditLabel(m_hMaterialList, sel);
         }
         break;
      }
      case IDC_EXPORT:
      {
         if (ListView_GetSelectedCount(m_hMaterialList))	// if some items are selected???
         {
            char szFileName[4096];
            char szInitialDir[4096];
            int sel = ListView_GetNextItem(m_hMaterialList, -1, LVNI_SELECTED);
            int selCount = ListView_GetSelectedCount(m_hMaterialList);
            if (sel == -1)
               break;

            strcpy_s(szFileName, "Materials.mat");
            OPENFILENAME ofn;
            ZeroMemory(&ofn, sizeof(OPENFILENAME));
            ofn.lStructSize = sizeof(OPENFILENAME);
            ofn.hInstance = g_hinst;
            ofn.hwndOwner = g_pvp->m_hwnd;
            ofn.lpstrFile = szFileName;
            //TEXT
            ofn.lpstrFilter = "Material Files (.mat)\0*.mat\0";
            ofn.nMaxFile = 4096;
            ofn.lpstrDefExt = "mat";

            const HRESULT hr = GetRegString("RecentDir", "MaterialDir", szInitialDir, 4096);

            if (hr == S_OK)ofn.lpstrInitialDir = szInitialDir;
            else ofn.lpstrInitialDir = NULL;

            ofn.lpstrTitle = "Export materials";
            ofn.Flags = OFN_NOREADONLYRETURN | OFN_CREATEPROMPT | OFN_OVERWRITEPROMPT | OFN_EXPLORER;

            szInitialDir[ofn.nFileOffset] = 0;

            if (GetSaveFileName(&ofn))	//Get filename from user
            {
               FILE *f;
               fopen_s(&f, ofn.lpstrFile, "wb");
               const int MATERIAL_VERSION = 1;
               fwrite(&MATERIAL_VERSION, 1, 4, f);
               fwrite(&selCount, 1, 4, f);
               while (sel != -1)
               {
                  LVITEM lvitem;
                  lvitem.mask = LVIF_PARAM;
                  lvitem.iItem = sel;
                  lvitem.iSubItem = 0;
                  ListView_GetItem(m_hMaterialList, &lvitem);
                  Material * const pmat = (Material*)lvitem.lParam;
                  SaveMaterial mat;
                  mat.cBase = pmat->m_cBase;
                  mat.cGlossy = pmat->m_cGlossy;
                  mat.cClearcoat = pmat->m_cClearcoat;
                  mat.fRoughness = pmat->m_fRoughness;
                  mat.fGlossyImageLerp = quantizeUnsigned<8>(clamp(pmat->m_fGlossyImageLerp, 0.f, 1.f));
                  mat.fThickness = quantizeUnsigned<8>(clamp(pmat->m_fThickness, 0.f, 1.f));
                  mat.fEdge = pmat->m_fEdge;
                  mat.fWrapLighting = pmat->m_fWrapLighting;
                  mat.bIsMetal = pmat->m_bIsMetal;
                  mat.fOpacity = pmat->m_fOpacity;
                  mat.bOpacityActive_fEdgeAlpha = pmat->m_bOpacityActive ? 1 : 0;
                  mat.bOpacityActive_fEdgeAlpha |= quantizeUnsigned<7>(clamp(pmat->m_fEdgeAlpha, 0.f, 1.f)) << 1;
                  memcpy(mat.szName, pmat->m_szName, 32);
                  fwrite(&mat, 1, sizeof(SaveMaterial), f);
                  fwrite(&pmat->m_fElasticity, 1, sizeof(float), f);
                  fwrite(&pmat->m_fElasticityFalloff, 1, sizeof(float), f);
                  fwrite(&pmat->m_fFriction, 1, sizeof(float), f);
                  fwrite(&pmat->m_fScatterAngle, 1, sizeof(float), f);

                  sel = ListView_GetNextItem(m_hMaterialList, sel, LVNI_SELECTED);
               }
               fclose(f);
            }
            SetRegValue("RecentDir", "MaterialDir", REG_SZ, szInitialDir, lstrlen(szInitialDir));
         }
         break;
      }
      case IDC_DELETE_MATERIAL:
      {
         const int count = ListView_GetSelectedCount(m_hMaterialList);
         if (count > 0)
         {
            LocalString ls(IDS_REMOVEMATERIAL);
            const int ans = MessageBox(ls.m_szbuffer/*"Are you sure you want to remove this material?"*/, "Visual Pinball", MB_YESNO | MB_DEFBUTTON2);
            if (ans == IDYES)
            {
               m_deletingItem = true;
               int sel = ListView_GetNextItem(m_hMaterialList, -1, LVNI_SELECTED);
               int firstSelectedItemIdx = sel;
               while (sel != -1)
               {
                  LVITEM lvitem;
                  lvitem.mask = LVIF_PARAM;
                  lvitem.iItem = sel;
                  lvitem.iSubItem = 0;
                  ListView_GetItem(m_hMaterialList, &lvitem);
                  Material * const pmat = (Material*)lvitem.lParam;
                  ListView_DeleteItem(m_hMaterialList, sel);

                  pt->RemoveMaterial(pmat);

                  sel = ListView_GetNextItem(m_hMaterialList, -1, LVNI_SELECTED);
               }

               m_deletingItem = false;
               int newCount = ListView_GetItemCount(m_hMaterialList);
               int selectedCount = ListView_GetSelectedCount(m_hMaterialList);
               if(newCount > 0 && selectedCount == 0)
               {
                   if(firstSelectedItemIdx >= newCount) firstSelectedItemIdx = 0;
                   // The previous selection is now deleted, so look again from the top of the list
                   ListView_SetItemState(m_hMaterialList, firstSelectedItemIdx, LVIS_FOCUSED | LVIS_SELECTED, 0x00F);
               }
               else
               {
                   ListView_GetNextItem(m_hMaterialList, -1, LVNI_SELECTED);
               }
            }
            pt->SetNonUndoableDirty(eSaveDirty);
            g_pvp->m_sb.PopulateDropdowns();
            g_pvp->m_sb.RefreshProperties();
         }
         break;
      }
   }
   return TRUE;
}

INT_PTR MaterialDialog::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    m_resizer.HandleMessage(uMsg, wParam, lParam);
    
   switch (uMsg)
   {
      case GET_COLOR_TABLE:
      {
         *((unsigned long **)lParam) = &g_pvp->dummyMaterial.m_cBase;
         return TRUE;
      }

      case WM_NOTIFY:
      {
         CCO(PinTable) *pt = (CCO(PinTable) *)g_pvp->GetActiveTable();
         LPNMHDR pnmhdr = (LPNMHDR)lParam;
         if (wParam == IDC_MATERIAL_LIST)
         {
            LPNMLISTVIEW lpnmListView = (LPNMLISTVIEW)lParam;
            if (lpnmListView->hdr.code == LVN_COLUMNCLICK)
            {
               const int columnNumber = lpnmListView->iSubItem;
               if (m_columnSortOrder == 1)
                  m_columnSortOrder = 0;
               else
                  m_columnSortOrder = 1;
               SortData.hwndList = m_hMaterialList;
               SortData.subItemIndex = columnNumber;
               SortData.sortUpDown = m_columnSortOrder;
               ListView_SortItems(SortData.hwndList, MyCompProc, &SortData);
            }
         }
         switch (pnmhdr->code)
         {
            case LVN_ENDLABELEDIT:
            {
               NMLVDISPINFO * const pinfo = (NMLVDISPINFO *)lParam;
               if (pinfo->item.pszText == NULL || pinfo->item.pszText[0] == '\0')
               {
                  return FALSE;
               }
               size_t len = strlen(pinfo->item.pszText);
               if(len > 31)
               {
                   ShowError("Material names can only be up to 31 characters long!");
                   return FALSE;
               }

               LVITEM lvitem;
               lvitem.mask = LVIF_PARAM;
               lvitem.iItem = pinfo->item.iItem;
               lvitem.iSubItem = 0;
               ListView_GetItem(m_hMaterialList, &lvitem);
               Material *pmat = (Material*)lvitem.lParam;

               if (pt->IsMaterialNameUnique(pinfo->item.pszText))
               {
                  strncpy_s(pmat->m_szName, pinfo->item.pszText, 31);
                  ListView_SetItemText(m_hMaterialList, pinfo->item.iItem, 0, pinfo->item.pszText);
               }
               else
               {
                  char textBuf[32];
                  int suffix = 1;
                  do
                  {
                     sprintf_s(textBuf, "%s%i", pinfo->item.pszText, suffix);
                     suffix++;
                  } while (!pt->IsMaterialNameUnique(textBuf));
                  strncpy_s(pmat->m_szName, textBuf, 31);
                  ListView_SetItemText(m_hMaterialList, pinfo->item.iItem, 0, pmat->m_szName);
               }
               g_pvp->m_sb.PopulateDropdowns();
               g_pvp->m_sb.RefreshProperties();
               pt->SetNonUndoableDirty(eSaveDirty);
               return TRUE;
            }
            case LVN_ITEMCHANGING:
            {
               if(m_deletingItem)
                   break;

               const int count = ListView_GetSelectedCount(m_hMaterialList);
               if (count > 1)
               {
                  DisableAllMaterialDialogItems();
                  break;
               }
               EnableAllMaterialDialogItems();

               NMLISTVIEW * const plistview = (LPNMLISTVIEW)lParam;
               if ((plistview->uNewState & LVIS_SELECTED) != (plistview->uOldState & LVIS_SELECTED))
               {
                  if ((plistview->uNewState & LVIS_SELECTED) == LVIS_SELECTED)
                  {
                     const int sel = plistview->iItem;
                     LVITEM lvitem;
                     lvitem.mask = LVIF_PARAM;
                     lvitem.iItem = sel;
                     lvitem.iSubItem = 0;
                     ListView_GetItem(m_hMaterialList, &lvitem);
                     Material * const pmat = (Material*)lvitem.lParam;
                     HWND hwndColor = GetDlgItem(IDC_COLOR).GetHwnd();
                     SendMessage(hwndColor, CHANGE_COLOR, 0, pmat->m_cBase);
                     hwndColor = GetDlgItem(IDC_COLOR2).GetHwnd();
                     SendMessage(hwndColor, CHANGE_COLOR, 0, pmat->m_cGlossy);
                     hwndColor = GetDlgItem(IDC_COLOR3).GetHwnd();
                     SendMessage(hwndColor, CHANGE_COLOR, 0, pmat->m_cClearcoat);
                     setItemText(IDC_DIFFUSE_EDIT, pmat->m_fWrapLighting);
                     setItemText(IDC_GLOSSY_EDIT, pmat->m_fRoughness);
                     setItemText(IDC_GLOSSY_IMGLERP_EDIT, pmat->m_fGlossyImageLerp);
                     setItemText(IDC_THICKNESS_EDIT, pmat->m_fThickness);
                     setItemText(IDC_SPECULAR_EDIT, pmat->m_fEdge);
                     setItemText(IDC_OPACITY_EDIT, pmat->m_fOpacity);
                     setItemText(IDC_EDGEALPHA_EDIT, pmat->m_fEdgeAlpha);

                     HWND checkboxHwnd = GetDlgItem(IDC_DIFFUSE_CHECK).GetHwnd();
                     SendMessage(checkboxHwnd, BM_SETCHECK, pmat->m_bIsMetal ? BST_CHECKED : BST_UNCHECKED, 0);
                     checkboxHwnd = GetDlgItem(IDC_OPACITY_CHECK).GetHwnd();
                     SendMessage(checkboxHwnd, BM_SETCHECK, pmat->m_bOpacityActive ? BST_CHECKED : BST_UNCHECKED, 0);

                     setItemText(IDC_MAT_ELASTICITY, pmat->m_fElasticity);
                     setItemText(IDC_MAT_ELASTICITY_FALLOFF, pmat->m_fElasticityFalloff);
                     setItemText(IDC_MAT_FRICTION, pmat->m_fFriction);
                     setItemText(IDC_MAT_SCATTER_ANGLE, pmat->m_fScatterAngle);

                     ::InvalidateRect(hwndColor, NULL, FALSE);
                  }
               }
               break;
            }
            case LVN_ITEMCHANGED:
            {
               const int count = ListView_GetSelectedCount(m_hMaterialList);

               if(m_deletingItem)
                   break;
               if (count > 1)
               {
                  DisableAllMaterialDialogItems();
                  break;
               }
               EnableAllMaterialDialogItems();
               NMLISTVIEW * const plistview = (LPNMLISTVIEW)lParam;
               const int sel = plistview->iItem;
               LVITEM lvitem;
               lvitem.mask = LVIF_PARAM;
               lvitem.iItem = sel;
               lvitem.iSubItem = 0;
               ListView_GetItem(m_hMaterialList, &lvitem);
               Material * const pmat = (Material*)lvitem.lParam;
               if ((plistview->uNewState & LVIS_SELECTED) == 0)
               {
                  float fv;
                  fv = saturate(getItemText(IDC_DIFFUSE_EDIT));
                  if (pmat->m_fWrapLighting != fv)
                     pt->SetNonUndoableDirty(eSaveDirty);
                  pmat->m_fWrapLighting = fv;
                  fv = saturate(getItemText(IDC_GLOSSY_EDIT));
                  if (pmat->m_fRoughness != fv)
                     pt->SetNonUndoableDirty(eSaveDirty);
                  pmat->m_fRoughness = fv;
                  fv = saturate(getItemText(IDC_GLOSSY_IMGLERP_EDIT));
                  if (pmat->m_fGlossyImageLerp != fv)
                     pt->SetNonUndoableDirty(eSaveDirty);
                  pmat->m_fGlossyImageLerp = fv;
                  fv = saturate(getItemText(IDC_THICKNESS_EDIT));
                  if (pmat->m_fThickness != fv)
                     pt->SetNonUndoableDirty(eSaveDirty);
                  pmat->m_fThickness = fv;
                  fv = saturate(getItemText(IDC_SPECULAR_EDIT));
                  if (pmat->m_fEdge != fv)
                     pt->SetNonUndoableDirty(eSaveDirty);
                  pmat->m_fEdge = fv;
                  fv = saturate(getItemText(IDC_OPACITY_EDIT));
                  if (pmat->m_fOpacity != fv)
                     pt->SetNonUndoableDirty(eSaveDirty);
                  pmat->m_fOpacity = fv;
                  size_t checked = SendDlgItemMessage(IDC_DIFFUSE_CHECK, BM_GETCHECK, 0, 0);
                  if (pmat->m_bIsMetal != (checked == 1))
                     pt->SetNonUndoableDirty(eSaveDirty);
                  pmat->m_bIsMetal = checked == 1;
                  checked = SendDlgItemMessage(IDC_OPACITY_CHECK, BM_GETCHECK, 0, 0);
                  if (pmat->m_bOpacityActive != (checked == 1))
                     pt->SetNonUndoableDirty(eSaveDirty);
                  pmat->m_bOpacityActive = checked == 1;
                  fv = saturate(getItemText(IDC_EDGEALPHA_EDIT));
                  if (pmat->m_fEdgeAlpha != fv)
                     pt->SetNonUndoableDirty(eSaveDirty);
                  pmat->m_fEdgeAlpha = fv;

                  fv = getItemText(IDC_MAT_ELASTICITY);
                  if (pmat->m_fElasticity != fv)
                     pt->SetNonUndoableDirty(eSaveDirty);
                  pmat->m_fElasticity = fv;
                  fv = getItemText(IDC_MAT_ELASTICITY_FALLOFF);
                  if (pmat->m_fElasticityFalloff != fv)
                     pt->SetNonUndoableDirty(eSaveDirty);
                  pmat->m_fElasticityFalloff = fv;
                  fv = getItemText(IDC_MAT_FRICTION);
                  if (pmat->m_fFriction != fv)
                     pt->SetNonUndoableDirty(eSaveDirty);
                  pmat->m_fFriction = fv;
                  fv = getItemText(IDC_MAT_SCATTER_ANGLE);
                  if (pmat->m_fScatterAngle != fv)
                     pt->SetNonUndoableDirty(eSaveDirty);
                  pmat->m_fScatterAngle = fv;
               }
               else if ((plistview->uOldState & LVIS_SELECTED) == 0)
               {
                  HWND hwndColor = GetDlgItem(IDC_COLOR).GetHwnd();
                  SendMessage(hwndColor, CHANGE_COLOR, 0, pmat->m_cBase);
                  hwndColor = GetDlgItem(IDC_COLOR2).GetHwnd();
                  SendMessage(hwndColor, CHANGE_COLOR, 0, pmat->m_cGlossy);
                  hwndColor = GetDlgItem(IDC_COLOR3).GetHwnd();
                  SendMessage(hwndColor, CHANGE_COLOR, 0, pmat->m_cClearcoat);
                  setItemText(IDC_DIFFUSE_EDIT, pmat->m_fWrapLighting);
                  setItemText(IDC_GLOSSY_EDIT, pmat->m_fRoughness);
                  setItemText(IDC_GLOSSY_IMGLERP_EDIT, pmat->m_fGlossyImageLerp);
                  setItemText(IDC_THICKNESS_EDIT, pmat->m_fThickness);
                  setItemText(IDC_SPECULAR_EDIT, pmat->m_fEdge);
                  setItemText(IDC_OPACITY_EDIT, pmat->m_fOpacity);

                  HWND checkboxHwnd = GetDlgItem(IDC_DIFFUSE_CHECK).GetHwnd();
                  SendMessage(checkboxHwnd, BM_SETCHECK, pmat->m_bIsMetal ? BST_CHECKED : BST_UNCHECKED, 0);
                  checkboxHwnd = GetDlgItem(IDC_OPACITY_CHECK).GetHwnd();
                  SendMessage(checkboxHwnd, BM_SETCHECK, pmat->m_bOpacityActive ? BST_CHECKED : BST_UNCHECKED, 0);
                  
                  setItemText(IDC_EDGEALPHA_EDIT, pmat->m_fEdgeAlpha);
                  setItemText(IDC_MAT_ELASTICITY, pmat->m_fElasticity);
                  setItemText(IDC_MAT_ELASTICITY_FALLOFF, pmat->m_fElasticityFalloff);
                  setItemText(IDC_MAT_FRICTION, pmat->m_fFriction);
                  setItemText(IDC_MAT_SCATTER_ANGLE, pmat->m_fScatterAngle);
               }
               break;
            }
         }
         break;
      }
      case WM_DRAWITEM:
      {
         DRAWITEMSTRUCT * const pdis = (DRAWITEMSTRUCT *)lParam;
         const int sel = ListView_GetNextItem(m_hMaterialList, -1, LVNI_SELECTED);
         if (sel != -1)
         {
            LVITEM lvitem;
            lvitem.mask = LVIF_PARAM;
            lvitem.iItem = sel;
            lvitem.iSubItem = 0;
            ListView_GetItem(m_hMaterialList, &lvitem);
         }
         else
         {
            // Nothing currently selected
         }
         return TRUE;
      }
   }
   return DialogProcDefault(uMsg, wParam, lParam);
}

void MaterialDialog::OnOK()
{
   CCO(PinTable) *pt = (CCO(PinTable) *)g_pvp->GetActiveTable();
   const int count = ListView_GetSelectedCount(m_hMaterialList);
   if (count > 0)
   {
      int sel = ListView_GetNextItem(m_hMaterialList, -1, LVNI_SELECTED);
      while (sel != -1)
      {
         LVITEM lvitem;
         lvitem.mask = LVIF_PARAM;
         lvitem.iItem = sel;
         lvitem.iSubItem = 0;
         ListView_GetItem(m_hMaterialList, &lvitem);
         Material * const pmat = (Material*)lvitem.lParam;
         float fv;
         fv = getItemText(IDC_DIFFUSE_EDIT);
         if (pmat->m_fWrapLighting != fv)
            pt->SetNonUndoableDirty(eSaveDirty);
         pmat->m_fWrapLighting = fv;

         fv = getItemText(IDC_GLOSSY_EDIT);
         if (pmat->m_fRoughness != fv)
            pt->SetNonUndoableDirty(eSaveDirty);
         pmat->m_fRoughness = fv;

         fv = getItemText(IDC_GLOSSY_IMGLERP_EDIT);
         if (pmat->m_fGlossyImageLerp != fv)
            pt->SetNonUndoableDirty(eSaveDirty);
         pmat->m_fGlossyImageLerp = fv;

         fv = getItemText(IDC_THICKNESS_EDIT);
         if (pmat->m_fThickness != fv)
            pt->SetNonUndoableDirty(eSaveDirty);
         pmat->m_fThickness = fv;

         fv = getItemText(IDC_SPECULAR_EDIT);
         if (pmat->m_fEdge != fv)
            pt->SetNonUndoableDirty(eSaveDirty);
         pmat->m_fEdge = fv;

         fv = getItemText(IDC_OPACITY_EDIT);
         if (pmat->m_fOpacity != fv)
            pt->SetNonUndoableDirty(eSaveDirty);
         pmat->m_fOpacity = fv;

         size_t checked = SendDlgItemMessage(IDC_DIFFUSE_CHECK, BM_GETCHECK, 0, 0);
         if (pmat->m_bIsMetal != (checked == 1))
            pt->SetNonUndoableDirty(eSaveDirty);
         pmat->m_bIsMetal = checked == 1;

         checked = SendDlgItemMessage(IDC_OPACITY_CHECK, BM_GETCHECK, 0, 0);
         if (pmat->m_bOpacityActive != (checked == 1))
            pt->SetNonUndoableDirty(eSaveDirty);
         pmat->m_bOpacityActive = checked == 1;

         fv = getItemText(IDC_EDGEALPHA_EDIT);
         if (pmat->m_fEdgeAlpha != fv)
            pt->SetNonUndoableDirty(eSaveDirty);
         pmat->m_fEdgeAlpha = fv;

         fv = getItemText(IDC_MAT_ELASTICITY);
         if (pmat->m_fElasticity != fv)
            pt->SetNonUndoableDirty(eSaveDirty);
         pmat->m_fElasticity = fv;

         fv = getItemText(IDC_MAT_ELASTICITY_FALLOFF);
         if (pmat->m_fElasticityFalloff != fv)
            pt->SetNonUndoableDirty(eSaveDirty);
         pmat->m_fElasticityFalloff = fv;

         fv = getItemText(IDC_MAT_FRICTION);
         if (pmat->m_fFriction != fv)
            pt->SetNonUndoableDirty(eSaveDirty);
         pmat->m_fFriction = fv;

         fv = getItemText(IDC_MAT_SCATTER_ANGLE);
         if (pmat->m_fScatterAngle != fv)
            pt->SetNonUndoableDirty(eSaveDirty);
         pmat->m_fScatterAngle = fv;

         // The previous selection is now deleted, so look again from the top of the list
         sel = ListView_GetNextItem(m_hMaterialList, sel, LVNI_SELECTED);
      }
   }
   SavePosition();
   CDialog::OnOK();
}

void MaterialDialog::OnClose()
{
    SavePosition();
    CDialog::OnCancel();
}

void MaterialDialog::LoadPosition()
{
    int x, y, w, h;
    HRESULT hr;

    hr = GetRegInt("Editor", "MaterialMngPosX", &x);
    if (hr != S_OK)
        x = 0;
    hr = GetRegInt("Editor", "MaterialMngPosY", &y);
    if (hr != S_OK)
        y = 0;

    w = GetRegIntWithDefault("Editor", "MaterialMngWidth", 1000);
    h = GetRegIntWithDefault("Editor", "MaterialMngHeight", 800);
    SetWindowPos(NULL, x, y, w, h, SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE);
}

void MaterialDialog::SavePosition()
{
    int w, h;
    CRect rect = GetWindowRect();

    (void)SetRegValue("Editor", "MaterialMngPosX", REG_DWORD, &rect.left, 4);
    (void)SetRegValue("Editor", "MaterialMngPosY", REG_DWORD, &rect.top, 4);
    w = rect.right - rect.left;
    (void)SetRegValue("Editor", "MaterialMngWidth", REG_DWORD, &w, 4);
    h = rect.bottom - rect.top;
    (void)SetRegValue("Editor", "MaterialMngHeight", REG_DWORD, &h, 4);
}

