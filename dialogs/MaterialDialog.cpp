#include "stdafx.h"
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

void MaterialDialog::EnableAllMaterialDialogItems(const BOOL e)
{
   ::EnableWindow(GetDlgItem(IDC_DIFFUSE_CHECK).GetHwnd(), e);
   ::EnableWindow(GetDlgItem(IDC_DIFFUSE_EDIT).GetHwnd(), e);
   ::EnableWindow(GetDlgItem(IDC_GLOSSY_EDIT).GetHwnd(), e);
   ::EnableWindow(GetDlgItem(IDC_GLOSSY_IMGLERP_EDIT).GetHwnd(), e);
   ::EnableWindow(GetDlgItem(IDC_THICKNESS_EDIT).GetHwnd(), e);
   ::EnableWindow(GetDlgItem(IDC_SPECULAR_EDIT).GetHwnd(), e);
   ::EnableWindow(GetDlgItem(IDC_OPACITY_EDIT).GetHwnd(), e);
   ::EnableWindow(GetDlgItem(IDC_OPACITY_CHECK).GetHwnd(), e);
   ::EnableWindow(GetDlgItem(IDC_EDGEALPHA_EDIT).GetHwnd(), e);
   ::EnableWindow(GetDlgItem(IDC_CLONE_BUTTON).GetHwnd(), e);
   ::EnableWindow(GetDlgItem(IDC_RENAME).GetHwnd(), e);
   ::EnableWindow(GetDlgItem(IDC_IMPORT).GetHwnd(), e);
}

float MaterialDialog::getItemText(int id)
{
   const CString textStr(GetDlgItemText(id));
   const float fv = sz2f(textStr.c_str());
   return fv;
}

void MaterialDialog::setItemText(int id, float value)
{
   char textBuf[256] = { 0 };
   f2sz(value, textBuf);
   const CString textStr(textBuf);
   SetDlgItemText(id, textStr);
}

MaterialDialog::MaterialDialog() : CDialog(IDD_MATERIALDIALOG)
{
    m_hMaterialList = NULL;
}

BOOL MaterialDialog::OnInitDialog()
{
   m_hMaterialList = GetDlgItem(IDC_MATERIAL_LIST).GetHwnd();
   CCO(PinTable) * const pt = g_pvp->GetActiveTable();

   m_columnSortOrder = 1;
   m_deletingItem = false;
   m_resizer.Initialize(*this, CRect(0, 0, 500, 600));
   AttachItem(IDC_COLOR_BUTTON1, m_colorButton1);
   AttachItem(IDC_COLOR_BUTTON2, m_colorButton2);
   AttachItem(IDC_COLOR_BUTTON3, m_colorButton3);
   m_resizer.AddChild(m_hMaterialList, topleft, RD_STRETCH_WIDTH | RD_STRETCH_HEIGHT);
   m_resizer.AddChild(GetDlgItem(IDC_DIFFUSE_CHECK).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC_BASE_COLOR).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC_GLOSSY_LAYER).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC_CLEARCOAR_LAYER).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC_OPACITY).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC_PHYSICS).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_COLOR_BUTTON1).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_COLOR_BUTTON2).GetHwnd(), topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_COLOR_BUTTON3).GetHwnd(), topright, 0);
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
   LVCOLUMN lvcol;
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
   CCO(PinTable) * const pt = g_pvp->GetActiveTable();
   UNREFERENCED_PARAMETER(lParam);

   switch (LOWORD(wParam))
   {
       case IDC_COLOR_BUTTON1:
       {
           CHOOSECOLOR cc = m_colorDialog.GetParameters();
           cc.Flags = CC_FULLOPEN | CC_RGBINIT;
           m_colorDialog.SetParameters(cc);

           if (ListView_GetSelectedCount(m_hMaterialList))	// if some items are selected???
           {
               int sel = ListView_GetNextItem(m_hMaterialList, -1, LVNI_SELECTED);
               if (sel == -1)
                   break;
               
               LVITEM lvitem;
               lvitem.mask = LVIF_PARAM;
               lvitem.iItem = sel;
               lvitem.iSubItem = 0;
               ListView_GetItem(m_hMaterialList, &lvitem);
               Material * const pmat = (Material*)lvitem.lParam;
               m_colorDialog.SetColor(pmat->m_cBase);
               if (m_colorDialog.DoModal(GetHwnd()) == IDOK)
               {
                   pmat->m_cBase = m_colorDialog.GetColor();
                   m_colorButton1.SetColor(pmat->m_cBase);
                   while (sel != -1)
                   {
                       sel = ListView_GetNextItem(m_hMaterialList, sel, LVNI_SELECTED);
                       if(sel!=-1)
                       {
                           LVITEM lvitem;
                           lvitem.mask = LVIF_PARAM;
                           lvitem.iItem = sel;
                           lvitem.iSubItem = 0;
                           ListView_GetItem(m_hMaterialList, &lvitem);
                           Material * const pmat = (Material*)lvitem.lParam;
                           pmat->m_cBase = m_colorDialog.GetColor();
                       }
                   }
                   pt->SetNonUndoableDirty(eSaveDirty);
               }
           }
           break;
       }
       case IDC_COLOR_BUTTON2:
       {
           CHOOSECOLOR cc = m_colorDialog.GetParameters();
           cc.Flags = CC_FULLOPEN | CC_RGBINIT;
           m_colorDialog.SetParameters(cc);

           if (ListView_GetSelectedCount(m_hMaterialList))	// if some items are selected???
           {
               int sel = ListView_GetNextItem(m_hMaterialList, -1, LVNI_SELECTED);
               if (sel == -1)
                   break;

               LVITEM lvitem;
               lvitem.mask = LVIF_PARAM;
               lvitem.iItem = sel;
               lvitem.iSubItem = 0;
               ListView_GetItem(m_hMaterialList, &lvitem);
               Material * const pmat = (Material*)lvitem.lParam;
               m_colorDialog.SetColor(pmat->m_cGlossy);
               if (m_colorDialog.DoModal(GetHwnd()) == IDOK)
               {
                   pmat->m_cGlossy = m_colorDialog.GetColor();
                   m_colorButton2.SetColor(pmat->m_cGlossy);
                   while (sel != -1)
                   {
                       sel = ListView_GetNextItem(m_hMaterialList, sel, LVNI_SELECTED);
                       if(sel!=-1)
                       {
                           LVITEM lvitem;
                           lvitem.mask = LVIF_PARAM;
                           lvitem.iItem = sel;
                           lvitem.iSubItem = 0;
                           ListView_GetItem(m_hMaterialList, &lvitem);
                           Material * const pmat = (Material*)lvitem.lParam;
                           pmat->m_cGlossy = m_colorDialog.GetColor();
                       }
                   }
                   pt->SetNonUndoableDirty(eSaveDirty);
               }
           }
           break;
       }
       case IDC_COLOR_BUTTON3:
       {
           CHOOSECOLOR cc = m_colorDialog.GetParameters();
           cc.Flags = CC_FULLOPEN | CC_RGBINIT;
           m_colorDialog.SetParameters(cc);

           if (ListView_GetSelectedCount(m_hMaterialList))	// if some items are selected???
           {
               int sel = ListView_GetNextItem(m_hMaterialList, -1, LVNI_SELECTED);
               if (sel == -1)
                   break;

               LVITEM lvitem;
               lvitem.mask = LVIF_PARAM;
               lvitem.iItem = sel;
               lvitem.iSubItem = 0;
               ListView_GetItem(m_hMaterialList, &lvitem);
               Material * const pmat = (Material*)lvitem.lParam;
               m_colorDialog.SetColor(pmat->m_cClearcoat);
               if (m_colorDialog.DoModal(GetHwnd()) == IDOK)
               {
                   pmat->m_cClearcoat = m_colorDialog.GetColor();
                   m_colorButton3.SetColor(pmat->m_cClearcoat);
                   while (sel != -1)
                   {
                       sel = ListView_GetNextItem(m_hMaterialList, sel, LVNI_SELECTED);
                       if(sel!=-1)
                       {
                           LVITEM lvitem;
                           lvitem.mask = LVIF_PARAM;
                           lvitem.iItem = sel;
                           lvitem.iSubItem = 0;
                           ListView_GetItem(m_hMaterialList, &lvitem);
                           Material * const pmat = (Material*)lvitem.lParam;
                           pmat->m_cClearcoat = m_colorDialog.GetColor();
                       }
                   }
                   pt->SetNonUndoableDirty(eSaveDirty);
               }
           }
           break;
       }

      case IDC_CLONE_BUTTON:
      {
         if (ListView_GetSelectedCount(m_hMaterialList))	// if some items are selected???
         {
            int sel = ListView_GetNextItem(m_hMaterialList, -1, LVNI_SELECTED);
            if (sel == -1)
               break;

            while (sel != -1)
            {
               LVITEM lvitem;
               lvitem.mask = LVIF_PARAM;
               lvitem.iItem = sel;
               lvitem.iSubItem = 0;
               ListView_GetItem(m_hMaterialList, &lvitem);
               Material * const pNewMat = new Material((Material*)lvitem.lParam);
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
         Material * const pmat = new Material();

         pt->AddMaterial(pmat);
         pt->AddListMaterial(m_hMaterialList, pmat);
         g_pvp->m_sb.PopulateDropdowns(); // May need to update list of images
         g_pvp->m_sb.RefreshProperties();
         pt->SetNonUndoableDirty(eSaveDirty);

         break;
      }
      case IDC_IMPORT:
      {
         char szFileName[MAXSTRING];
         char szInitialDir[MAXSTRING];
         szFileName[0] = '\0';

         OPENFILENAME ofn;
         ZeroMemory(&ofn, sizeof(OPENFILENAME));
         ofn.lStructSize = sizeof(OPENFILENAME);
         ofn.hInstance = g_hinst;
         ofn.hwndOwner = g_pvp->m_hwnd;

         ofn.lpstrFilter = "Material Files (.mat)\0*.mat\0";
         ofn.lpstrFile = szFileName;
         ofn.nMaxFile = MAXSTRING;
         ofn.lpstrDefExt = "mat";
         ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY | OFN_EXPLORER;

         HRESULT hr = LoadValueString("RecentDir", "MaterialDir", szInitialDir, MAXSTRING);
         ofn.lpstrInitialDir = (hr == S_OK) ? szInitialDir : NULL;

         const int ret = GetOpenFileName(&ofn);

         if (ret)
         {
            int materialCount = 0;
            int versionNumber = 0;
            FILE *f;
            fopen_s(&f, ofn.lpstrFile, "rb");

            fread(&versionNumber, 4, 1, f);
            if (versionNumber != 1)
            {
               ShowError("Materials are not compatible with this version!");
               fclose(f);
               break;
            }
            fread(&materialCount, 4, 1, f);
            for (int i = 0; i < materialCount; i++)
            {
               SaveMaterial mat;
               fread(&mat, sizeof(SaveMaterial), 1, f);

               float elasticity,elasticityFalloff,friction,scatterAngle;
               fread(&elasticity, sizeof(float), 1, f);
               fread(&elasticityFalloff, sizeof(float), 1, f);
               fread(&friction, sizeof(float), 1, f);
               fread(&scatterAngle, sizeof(float), 1, f);

               Material * const pmat = new Material(mat.fWrapLighting, mat.fRoughness, dequantizeUnsigned<8>(mat.fGlossyImageLerp), dequantizeUnsigned<8>(mat.fThickness), mat.fEdge, dequantizeUnsigned<7>(mat.bOpacityActive_fEdgeAlpha >> 1), mat.fOpacity, mat.cBase, mat.cGlossy, mat.cClearcoat, mat.bIsMetal, !!(mat.bOpacityActive_fEdgeAlpha & 1), elasticity, elasticityFalloff, friction, scatterAngle);
               memcpy(pmat->m_szName, mat.szName, MAXNAMEBUFFER);

               pt->AddMaterial(pmat);
               pt->AddListMaterial(m_hMaterialList, pmat);
            }
            fclose(f);
            SaveValueString("RecentDir", "MaterialDir", szInitialDir);
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
            char szFileName[MAXSTRING];
            char szInitialDir[MAXSTRING];
            int sel = ListView_GetNextItem(m_hMaterialList, -1, LVNI_SELECTED);
            const int selCount = ListView_GetSelectedCount(m_hMaterialList);
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
            ofn.nMaxFile = MAXSTRING;
            ofn.lpstrDefExt = "mat";

            const HRESULT hr = LoadValueString("RecentDir", "MaterialDir", szInitialDir, MAXSTRING);

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
               fwrite(&MATERIAL_VERSION, 4, 1, f);
               fwrite(&selCount, 4, 1, f);
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
                  memcpy(mat.szName, pmat->m_szName, MAXNAMEBUFFER);

                  fwrite(&mat, sizeof(SaveMaterial), 1, f);
                  fwrite(&pmat->m_fElasticity, sizeof(float), 1, f);
                  fwrite(&pmat->m_fElasticityFalloff, sizeof(float), 1, f);
                  fwrite(&pmat->m_fFriction, sizeof(float), 1, f);
                  fwrite(&pmat->m_fScatterAngle, sizeof(float), 1, f);

                  sel = ListView_GetNextItem(m_hMaterialList, sel, LVNI_SELECTED);
               }
               fclose(f);
            }
            SaveValueString("RecentDir", "MaterialDir", szInitialDir);
         }
         break;
      }
      case IDC_DELETE_MATERIAL:
      {
         const int count = ListView_GetSelectedCount(m_hMaterialList);
         if (count > 0)
         {
            LocalString ls(IDS_REMOVEMATERIAL);
            const int ans = MessageBox(ls.m_szbuffer/*"Are you sure you want to remove this material?"*/, "Confirm Deletion", MB_YESNO | MB_DEFBUTTON2);
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
               const int newCount = ListView_GetItemCount(m_hMaterialList);
               const int selectedCount = ListView_GetSelectedCount(m_hMaterialList);
               if (newCount > 0 && selectedCount == 0)
               {
                   if (firstSelectedItemIdx >= newCount) firstSelectedItemIdx = 0;
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
      case WM_NOTIFY:
      {
         CCO(PinTable) *const pt = g_pvp->GetActiveTable();
         const LPNMHDR pnmhdr = (LPNMHDR)lParam;
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
               if (len > 31)
               {
                   ShowError("Material names can only be up to 31 characters long!");
                   return FALSE;
               }

               LVITEM lvitem;
               lvitem.mask = LVIF_PARAM;
               lvitem.iItem = pinfo->item.iItem;
               lvitem.iSubItem = 0;
               ListView_GetItem(m_hMaterialList, &lvitem);
               Material * const pmat = (Material*)lvitem.lParam;

               if (pt->IsMaterialNameUnique(pinfo->item.pszText))
               {
                  strncpy_s(pmat->m_szName, pinfo->item.pszText, 31);
                  ListView_SetItemText(m_hMaterialList, pinfo->item.iItem, 0, pinfo->item.pszText);
               }
               else
               {
                  char textBuf[MAXNAMEBUFFER];
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
               if (m_deletingItem)
                   break;

               const int count = ListView_GetSelectedCount(m_hMaterialList);
               if (count > 1)
               {
                  EnableAllMaterialDialogItems(FALSE);
                  break;
               }
               EnableAllMaterialDialogItems(TRUE);

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
                     m_colorButton1.SetColor(pmat->m_cBase);
                     m_colorButton2.SetColor(pmat->m_cGlossy);
                     m_colorButton3.SetColor(pmat->m_cClearcoat);
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
                  }
               }
               break;
            }
            case LVN_ITEMCHANGED:
            {
               const int count = ListView_GetSelectedCount(m_hMaterialList);

               if (m_deletingItem)
                   break;
               if (count > 1)
               {
                  EnableAllMaterialDialogItems(FALSE);
                  break;
               }
               EnableAllMaterialDialogItems(TRUE);

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
                  m_colorButton1.SetColor(pmat->m_cBase);
                  m_colorButton2.SetColor(pmat->m_cGlossy);
                  m_colorButton3.SetColor(pmat->m_cClearcoat);
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
         //DRAWITEMSTRUCT * const pdis = (DRAWITEMSTRUCT *)lParam;
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
    
         LPDRAWITEMSTRUCT lpDrawItemStruct = reinterpret_cast<LPDRAWITEMSTRUCT>(lParam);
         UINT nID = static_cast<UINT>(wParam);
         if (nID == IDC_COLOR_BUTTON1)
         {
             m_colorButton1.DrawItem(lpDrawItemStruct);
         }
         else if (nID == IDC_COLOR_BUTTON2)
         {
             m_colorButton2.DrawItem(lpDrawItemStruct);
         }
         else if (nID == IDC_COLOR_BUTTON3)
         {
             m_colorButton3.DrawItem(lpDrawItemStruct);
         }

         return TRUE;
      }
   }
   return DialogProcDefault(uMsg, wParam, lParam);
}

void MaterialDialog::OnOK()
{
   CCO(PinTable) * const pt = g_pvp->GetActiveTable();
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
    const int x = LoadValueIntWithDefault("Editor", "MaterialMngPosX", 0);
    const int y = LoadValueIntWithDefault("Editor", "MaterialMngPosY", 0);

    const int w = LoadValueIntWithDefault("Editor", "MaterialMngWidth", 1000);
    const int h = LoadValueIntWithDefault("Editor", "MaterialMngHeight", 800);
    SetWindowPos(NULL, x, y, w, h, SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE);
}

void MaterialDialog::SavePosition()
{
    int w, h;
    CRect rect = GetWindowRect();

    SaveValueInt("Editor", "MaterialMngPosX", rect.left);
    SaveValueInt("Editor", "MaterialMngPosY", rect.top);
    w = rect.right - rect.left;
    SaveValueInt("Editor", "MaterialMngWidth", w);
    h = rect.bottom - rect.top;
    SaveValueInt("Editor", "MaterialMngHeight", h);
}
