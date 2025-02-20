// license:GPLv3+

#include "core/stdafx.h"
#include "ui/resource.h"
#include "MaterialDialog.h"
#include <fstream>
#include "WhereUsedDialog.h"

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
WhereUsedDialog m_whereUsedDlg_Materials;

void MaterialDialog::EnableAllMaterialDialogItems(const BOOL e)
{
   GetDlgItem(IDC_MATERIAL_TYPE).EnableWindow(e);
   GetDlgItem(IDC_DIFFUSE_EDIT).EnableWindow(e);
   GetDlgItem(IDC_GLOSSY_EDIT).EnableWindow(e);
   GetDlgItem(IDC_GLOSSY_IMGLERP_EDIT).EnableWindow(e);
   GetDlgItem(IDC_THICKNESS_EDIT).EnableWindow(e);
   GetDlgItem(IDC_SPECULAR_EDIT).EnableWindow(e);
   GetDlgItem(IDC_OPACITY_EDIT).EnableWindow(e);
   GetDlgItem(IDC_OPACITY_CHECK).EnableWindow(e);
   GetDlgItem(IDC_EDGEALPHA_EDIT).EnableWindow(e);
   GetDlgItem(IDC_CLONE_BUTTON).EnableWindow(e);
   GetDlgItem(IDC_RENAME).EnableWindow(e);
   GetDlgItem(IDC_IMPORT).EnableWindow(e);
}

float MaterialDialog::getItemText(int id)
{
   return sz2f(GetDlgItemText(id).c_str());
}

void MaterialDialog::setItemText(int id, float value)
{
   SetDlgItemText(id, f2sz(value).c_str());
}

MaterialDialog::MaterialDialog() : CDialog(IDD_MATERIALDIALOG)
{
    m_hMaterialList = nullptr;
}

BOOL MaterialDialog::OnInitDialog()
{
   m_hMaterialList = GetDlgItem(IDC_MATERIAL_LIST).GetHwnd();
   CCO(PinTable) * const pt = g_pvp->GetActiveTable();

   m_columnSortOrder = 1;
   m_deletingItem = false;
   AttachItem(IDC_COLOR_BUTTON1, m_colorButton1);
   AttachItem(IDC_COLOR_BUTTON2, m_colorButton2);
   AttachItem(IDC_COLOR_BUTTON3, m_colorButton3);
   AttachItem(IDC_COLOR_BUTTON5, m_colorButton4);

   HWND hwnd = GetDlgItem(IDC_MATERIAL_TYPE).GetHwnd();
   SendMessage(hwnd, WM_SETREDRAW, FALSE, 0); // to speed up adding the entries :/
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Basic");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Metal");
   SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);

   m_resizer.Initialize(this->GetHwnd(), CRect(0, 0, 780, 520));
   m_resizer.AddChild(m_hMaterialList, CResizer::topleft, RD_STRETCH_WIDTH | RD_STRETCH_HEIGHT);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC1).GetHwnd(), CResizer::topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_MATERIAL_TYPE).GetHwnd(), CResizer::topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC_BASE_COLOR).GetHwnd(), CResizer::topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC_GLOSSY_LAYER).GetHwnd(), CResizer::topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC_CLEARCOAR_LAYER).GetHwnd(), CResizer::topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC_OPACITY).GetHwnd(), CResizer::topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC_PHYSICS).GetHwnd(), CResizer::topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_COLOR_BUTTON1).GetHwnd(), CResizer::topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_COLOR_BUTTON2).GetHwnd(), CResizer::topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_COLOR_BUTTON3).GetHwnd(), CResizer::topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_COLOR_BUTTON5).GetHwnd(), CResizer::topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC2).GetHwnd(), CResizer::topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC_WRAP).GetHwnd(), CResizer::topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_DIFFUSE_EDIT).GetHwnd(), CResizer::topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC_WRAP_TEXT).GetHwnd(), CResizer::topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC_GLOSSY_TEXT).GetHwnd(), CResizer::topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC_SHININESS).GetHwnd(), CResizer::topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC_GLOSSY_IMGLERP).GetHwnd(), CResizer::topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC_THICKNESS).GetHwnd(), CResizer::topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC_THICKNESS_TEXT).GetHwnd(), CResizer::topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_GLOSSY_EDIT).GetHwnd(), CResizer::topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_GLOSSY_IMGLERP_EDIT).GetHwnd(), CResizer::topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_THICKNESS_EDIT).GetHwnd(), CResizer::topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC_SHININESS_TEXT).GetHwnd(), CResizer::topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC_CLEARCOAT_TEXT).GetHwnd(), CResizer::topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC_EDGE_BRIGHTNESS).GetHwnd(), CResizer::topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_SPECULAR_EDIT).GetHwnd(), CResizer::topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC_BRIGHTNESS_TEXT).GetHwnd(), CResizer::topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC_AMOUNT).GetHwnd(), CResizer::topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_OPACITY_EDIT).GetHwnd(), CResizer::topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_OPACITY_CHECK).GetHwnd(), CResizer::topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC_EDGE_OPACITY).GetHwnd(), CResizer::topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_EDGEALPHA_EDIT).GetHwnd(), CResizer::topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC_EDGE_OPACITY_TEXT).GetHwnd(), CResizer::topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC_PHY_ELASTICITY).GetHwnd(), CResizer::topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_MAT_ELASTICITY).GetHwnd(), CResizer::topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC_PHY_ELASTICITY_FALLOFF).GetHwnd(), CResizer::topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_MAT_ELASTICITY_FALLOFF).GetHwnd(), CResizer::topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC_PHY_FRICTION).GetHwnd(), CResizer::topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_MAT_FRICTION).GetHwnd(), CResizer::topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_STATIC_PHY_SCATTER_ANGLE).GetHwnd(), CResizer::topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_MAT_SCATTER_ANGLE).GetHwnd(), CResizer::topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_ADD_BUTTON).GetHwnd(), CResizer::topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_CLONE_BUTTON).GetHwnd(), CResizer::topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_IMPORT).GetHwnd(), CResizer::topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_EXPORT).GetHwnd(), CResizer::topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_RENAME).GetHwnd(), CResizer::topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_DELETE_MATERIAL).GetHwnd(), CResizer::topright, 0);
   m_resizer.AddChild(GetDlgItem(IDC_SHOW_WHERE_USED).GetHwnd(), CResizer::topright, 0);
   m_resizer.AddChild(GetDlgItem(IDOK).GetHwnd(), CResizer::topright, 0);
   m_resizer.AddChild(GetDlgItem(IDCANCEL).GetHwnd(), CResizer::topright, 0);

   LoadPosition();
   ListView_SetExtendedListViewStyle(m_hMaterialList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
   LVCOLUMN lvcol = {};
   lvcol.mask = LVCF_TEXT | LVCF_WIDTH;
   const LocalString ls(IDS_NAME);
   lvcol.pszText = (LPSTR)ls.m_szbuffer; // = "Name";
   lvcol.cx = 230;
   ListView_InsertColumn(m_hMaterialList, 0, &lvcol);

   lvcol.mask = LVCF_TEXT | LVCF_WIDTH;
   const LocalString ls2(IDS_USED_IN_TABLE);
   lvcol.pszText = (LPSTR)ls2.m_szbuffer; // = "Used in Table";
   lvcol.cx = 50;
   ListView_InsertColumn(m_hMaterialList, 1, &lvcol);
   pt->ListMaterials(m_hMaterialList);

   ListView_SetItemState(m_hMaterialList, 0, LVIS_SELECTED, LVIS_SELECTED);
   GotoDlgCtrl(m_hMaterialList);

   return FALSE;
}

BOOL MaterialDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
   CCO(PinTable) * const pt = g_pvp->GetActiveTable();
   UNREFERENCED_PARAMETER(lParam);

   switch (LOWORD(wParam))
   {
   case IDC_MATERIAL_TYPE:
   {
      if (HIWORD(wParam) == CBN_SELCHANGE && ListView_GetSelectedCount(m_hMaterialList) == 1)
      {
         LVITEM lvitem;
         lvitem.mask = LVIF_PARAM;
         lvitem.iItem = ListView_GetNextItem(m_hMaterialList, -1, LVNI_SELECTED);
         lvitem.iSubItem = 0;
         ListView_GetItem(m_hMaterialList, &lvitem);
         Material *const pmat = (Material *)lvitem.lParam;
         size_t type = SendDlgItemMessage(IDC_MATERIAL_TYPE, CB_GETCURSEL, 0, 0);
         if (pmat->m_type != type)
            pt->SetNonUndoableDirty(eSaveDirty);
         pmat->m_type = (Material::MaterialType)type;
         SetEditedMaterial(*pmat);
      }
      break;
   }
       case IDC_COLOR_BUTTON1:
       {
           CHOOSECOLOR cc = m_colorDialog.GetParameters();
           cc.Flags = CC_FULLOPEN | CC_RGBINIT;
           m_colorDialog.SetParameters(cc);
           m_colorDialog.SetCustomColors(pt->m_rgcolorcustom);

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
               Material * pmat = (Material*)lvitem.lParam;
               m_colorDialog.SetColor(pmat->m_cBase);
               if (m_colorDialog.DoModal(GetHwnd()) == IDOK)
               {
                   pmat->m_cBase = m_colorDialog.GetColor();
                   memcpy(pt->m_rgcolorcustom, m_colorDialog.GetCustomColors(), sizeof(pt->m_rgcolorcustom));
                   m_colorButton1.SetColor(pmat->m_cBase);
                   while (sel != -1)
                   {
                       sel = ListView_GetNextItem(m_hMaterialList, sel, LVNI_SELECTED);
                       if(sel!=-1)
                       {
                           lvitem.mask = LVIF_PARAM;
                           lvitem.iItem = sel;
                           lvitem.iSubItem = 0;
                           ListView_GetItem(m_hMaterialList, &lvitem);
                           pmat = (Material*)lvitem.lParam;
                           pmat->m_cBase = m_colorDialog.GetColor();
                       }
                   }
                   pt->SetNonUndoableDirty(eSaveDirty);
                   pt->UpdatePropertyMaterialList();
               }
           }
           break;
       }
       case IDC_COLOR_BUTTON2:
       {
           CHOOSECOLOR cc = m_colorDialog.GetParameters();
           cc.Flags = CC_FULLOPEN | CC_RGBINIT;
           m_colorDialog.SetParameters(cc);
           m_colorDialog.SetCustomColors(pt->m_rgcolorcustom);
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
               Material * pmat = (Material*)lvitem.lParam;
               m_colorDialog.SetColor(pmat->m_cGlossy);
               if (m_colorDialog.DoModal(GetHwnd()) == IDOK)
               {
                   pmat->m_cGlossy = m_colorDialog.GetColor();
                   memcpy(pt->m_rgcolorcustom, m_colorDialog.GetCustomColors(), sizeof(pt->m_rgcolorcustom));
                   m_colorButton2.SetColor(pmat->m_cGlossy);
                   while (sel != -1)
                   {
                       sel = ListView_GetNextItem(m_hMaterialList, sel, LVNI_SELECTED);
                       if(sel!=-1)
                       {
                           lvitem.mask = LVIF_PARAM;
                           lvitem.iItem = sel;
                           lvitem.iSubItem = 0;
                           ListView_GetItem(m_hMaterialList, &lvitem);
                           pmat = (Material*)lvitem.lParam;
                           pmat->m_cGlossy = m_colorDialog.GetColor();
                       }
                   }
                   pt->SetNonUndoableDirty(eSaveDirty);
                   pt->UpdatePropertyMaterialList();
               }
           }
           break;
       }
       case IDC_COLOR_BUTTON3:
       {
           CHOOSECOLOR cc = m_colorDialog.GetParameters();
           cc.Flags = CC_FULLOPEN | CC_RGBINIT;
           m_colorDialog.SetParameters(cc);
           m_colorDialog.SetCustomColors(pt->m_rgcolorcustom);
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
               Material * pmat = (Material*)lvitem.lParam;
               m_colorDialog.SetColor(pmat->m_cClearcoat);
               if (m_colorDialog.DoModal(GetHwnd()) == IDOK)
               {
                   memcpy(pt->m_rgcolorcustom, m_colorDialog.GetCustomColors(), sizeof(pt->m_rgcolorcustom));
                   pmat->m_cClearcoat = m_colorDialog.GetColor();
                   m_colorButton3.SetColor(pmat->m_cClearcoat);
                   while (sel != -1)
                   {
                       sel = ListView_GetNextItem(m_hMaterialList, sel, LVNI_SELECTED);
                       if(sel!=-1)
                       {
                           lvitem.mask = LVIF_PARAM;
                           lvitem.iItem = sel;
                           lvitem.iSubItem = 0;
                           ListView_GetItem(m_hMaterialList, &lvitem);
                           pmat = (Material*)lvitem.lParam;
                           pmat->m_cClearcoat = m_colorDialog.GetColor();
                       }
                   }
                   pt->SetNonUndoableDirty(eSaveDirty);
                   pt->UpdatePropertyMaterialList();
               }
           }
           break;
       }
       case IDC_COLOR_BUTTON5:
       {
           CHOOSECOLOR cc = m_colorDialog.GetParameters();
           cc.Flags = CC_FULLOPEN | CC_RGBINIT;
           m_colorDialog.SetParameters(cc);
           m_colorDialog.SetCustomColors(pt->m_rgcolorcustom);
           if (ListView_GetSelectedCount(m_hMaterialList)) // if some items are selected???
           {
               int sel = ListView_GetNextItem(m_hMaterialList, -1, LVNI_SELECTED);
               if (sel == -1)
                   break;

               LVITEM lvitem;
               lvitem.mask = LVIF_PARAM;
               lvitem.iItem = sel;
               lvitem.iSubItem = 0;
               ListView_GetItem(m_hMaterialList, &lvitem);
               Material *pmat = (Material *)lvitem.lParam;
               m_colorDialog.SetColor(pmat->m_cRefractionTint);
               if (m_colorDialog.DoModal(GetHwnd()) == IDOK)
               {
                   memcpy(pt->m_rgcolorcustom, m_colorDialog.GetCustomColors(), sizeof(pt->m_rgcolorcustom));
                   pmat->m_cRefractionTint = m_colorDialog.GetColor();
                   m_colorButton4.SetColor(pmat->m_cRefractionTint);
                   while (sel != -1)
                   {
                       sel = ListView_GetNextItem(m_hMaterialList, sel, LVNI_SELECTED);
                       if (sel != -1)
                       {
                           lvitem.mask = LVIF_PARAM;
                           lvitem.iItem = sel;
                           lvitem.iSubItem = 0;
                           ListView_GetItem(m_hMaterialList, &lvitem);
                           pmat = (Material *)lvitem.lParam;
                           pmat->m_cRefractionTint = m_colorDialog.GetColor();
                       }
                   }
                   pt->SetNonUndoableDirty(eSaveDirty);
                   pt->UpdatePropertyMaterialList();
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
            vector<Material *> toDuplicate;
            while (sel != -1)
            {
               LVITEM lvitem;
               lvitem.mask = LVIF_PARAM;
               lvitem.iItem = sel;
               lvitem.iSubItem = 0;
               ListView_GetItem(m_hMaterialList, &lvitem);
               toDuplicate.push_back((Material *)lvitem.lParam);
               sel = ListView_GetNextItem(m_hMaterialList, sel, LVNI_SELECTED);
            }
            for (Material* mat : toDuplicate)
            {
               Material *const pNewMat = new Material(mat);
               pt->AddMaterial(pNewMat);
               pt->AddListMaterial(m_hMaterialList, pNewMat);
            }
            pt->SetNonUndoableDirty(eSaveDirty);
            pt->UpdatePropertyMaterialList();
         }
         break;
      }
      case IDC_ADD_BUTTON:
      {
         Material * const pmat = new Material();

         pt->AddMaterial(pmat);
         pt->AddListMaterial(m_hMaterialList, pmat);
         pt->SetNonUndoableDirty(eSaveDirty);
         pt->UpdatePropertyMaterialList();

         break;
      }
      case IDC_IMPORT:
      {
         string szInitialDir = g_pvp->m_settings.LoadValueWithDefault(Settings::RecentDir, "MaterialDir"s, PATH_TABLES);

         vector<string> szFilename;
         if (g_pvp->OpenFileDialog(szInitialDir, szFilename, "Material Files (.mat)\0*.mat\0", "mat", 0))
         {
            int materialCount = 0;
            int versionNumber = 0;
            FILE *f;
            fopen_s(&f, szFilename[0].c_str(), "rb");

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

               Material *const pmat = new Material(mat.bIsMetal ? Material::METAL : Material::BASIC ,mat.fWrapLighting, mat.fRoughness, dequantizeUnsigned<8>(mat.fGlossyImageLerp), dequantizeUnsigned<8>(mat.fThickness),
                  mat.fEdge, dequantizeUnsigned<7>(mat.bOpacityActive_fEdgeAlpha >> 1), mat.fOpacity, mat.cBase, mat.cGlossy, mat.cClearcoat, !!(mat.bOpacityActive_fEdgeAlpha & 1),
                  elasticity, elasticityFalloff, friction, scatterAngle, 0xFFFFFFFF);
               pmat->m_szName = mat.szName;

               pt->AddMaterial(pmat);
               pt->AddListMaterial(m_hMaterialList, pmat);
            }
            fclose(f);

            const size_t index = szFilename[0].find_last_of(PATH_SEPARATOR_CHAR);
            if (index != string::npos)
               g_pvp->m_settings.SaveValue(Settings::RecentDir, "MaterialDir"s, szFilename[0].substr(0, index));

            pt->SetNonUndoableDirty(eSaveDirty);
            pt->UpdatePropertyMaterialList();
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
            int sel = ListView_GetNextItem(m_hMaterialList, -1, LVNI_SELECTED);
            const int selCount = ListView_GetSelectedCount(m_hMaterialList);
            if (sel == -1)
               break;

            char szFileName[MAXSTRING];
            strncpy_s(szFileName, "Materials.mat", sizeof(szFileName)-1);
            OPENFILENAME ofn = {};
            ofn.lStructSize = sizeof(OPENFILENAME);
            ofn.hInstance = g_pvp->theInstance;
            ofn.hwndOwner = g_pvp->GetHwnd();
            ofn.lpstrFile = szFileName;
            //TEXT
            ofn.lpstrFilter = "Material Files (.mat)\0*.mat\0";
            ofn.nMaxFile = sizeof(szFileName);
            ofn.lpstrDefExt = "mat";

            string szInitialDir = g_pvp->m_settings.LoadValueWithDefault(Settings::RecentDir, "MaterialDir"s, PATH_TABLES);

            ofn.lpstrInitialDir = szInitialDir.c_str();
            ofn.lpstrTitle = "Export materials";
            ofn.Flags = OFN_NOREADONLYRETURN | OFN_CREATEPROMPT | OFN_OVERWRITEPROMPT | OFN_EXPLORER;

            if (GetSaveFileName(&ofn))	//Get filename from user
            {
               FILE *f;
               fopen_s(&f, ofn.lpstrFile, "wb");
               constexpr int mv = MATERIAL_VERSION;
               fwrite(&mv, 4, 1, f);
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
                  mat.bIsMetal = pmat->m_type == Material::METAL;
                  mat.fOpacity = pmat->m_fOpacity;
                  mat.bOpacityActive_fEdgeAlpha = pmat->m_bOpacityActive ? 1 : 0;
                  mat.bOpacityActive_fEdgeAlpha |= quantizeUnsigned<7>(clamp(pmat->m_fEdgeAlpha, 0.f, 1.f)) << 1;
                  strncpy_s(mat.szName, pmat->m_szName.c_str(), sizeof(mat.szName)-1);

                  fwrite(&mat, sizeof(SaveMaterial), 1, f);
                  fwrite(&pmat->m_fElasticity, sizeof(float), 1, f);
                  fwrite(&pmat->m_fElasticityFalloff, sizeof(float), 1, f);
                  fwrite(&pmat->m_fFriction, sizeof(float), 1, f);
                  fwrite(&pmat->m_fScatterAngle, sizeof(float), 1, f);

                  sel = ListView_GetNextItem(m_hMaterialList, sel, LVNI_SELECTED);
               }
               fclose(f);

               const string szFilename(ofn.lpstrFile);
               const size_t index = szFilename.find_last_of(PATH_SEPARATOR_CHAR);
               if (index != string::npos)
               {
                   const string newInitDir(szFilename.substr(0, index));
                   g_pvp->m_settings.SaveValue(Settings::RecentDir, "MaterialDir"s, newInitDir);
               }
            }
         }
         break;
      }
      case IDC_SHOW_WHERE_USED: ShowWhereUsed(); break;
      case IDC_DELETE_MATERIAL:
      {
         const int count = ListView_GetSelectedCount(m_hMaterialList);
         if (count > 0)
         {
            const LocalString ls(IDS_REMOVEMATERIAL);
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
            pt->UpdatePropertyMaterialList();
         }
         break;
      }
      default: return FALSE;
   }
   return TRUE;
}

void MaterialDialog::SetEditedMaterial(const Material& mat)
{
   const int count = ListView_GetSelectedCount(m_hMaterialList);

   GetDlgItem(IDC_CLONE_BUTTON).EnableWindow(count == 1);
   GetDlgItem(IDC_RENAME).EnableWindow(count == 1);
   GetDlgItem(IDC_IMPORT).EnableWindow(count == 1);

   // Material type
   GetDlgItem(IDC_MATERIAL_TYPE).EnableWindow(count == 1);
   SendDlgItemMessage(IDC_MATERIAL_TYPE, CB_SETCURSEL, mat.m_type, 0);

   // Base
   m_colorButton1.EnableWindow(count == 1);
   GetDlgItem(IDC_DIFFUSE_EDIT).EnableWindow(count == 1);
   m_colorButton1.SetColor(mat.m_cBase);
   setItemText(IDC_DIFFUSE_EDIT, mat.m_fWrapLighting);

   // Glossy
   m_colorButton2.EnableWindow(mat.m_type != Material::METAL && count == 1);
   GetDlgItem(IDC_GLOSSY_IMGLERP_EDIT).EnableWindow(mat.m_type != Material::METAL && count == 1);
   m_colorButton2.SetColor(mat.m_cGlossy);
   setItemText(IDC_GLOSSY_IMGLERP_EDIT, mat.m_fGlossyImageLerp);

   // Roughness
   GetDlgItem(IDC_GLOSSY_EDIT).EnableWindow(count == 1);
   setItemText(IDC_GLOSSY_EDIT, mat.m_fRoughness);

   // Clearcoat
   m_colorButton3.EnableWindow(count == 1);
   m_colorButton3.SetColor(mat.m_cClearcoat);

   // Edge brightness
   GetDlgItem(IDC_SPECULAR_EDIT).EnableWindow(count == 1);
   setItemText(IDC_SPECULAR_EDIT, mat.m_fEdge);

   // Transparency
   GetDlgItem(IDC_OPACITY_CHECK).EnableWindow(count == 1);
   GetDlgItem(IDC_OPACITY_EDIT).EnableWindow(count == 1);
   GetDlgItem(IDC_EDGEALPHA_EDIT).EnableWindow(count == 1);
   GetDlgItem(IDC_THICKNESS_EDIT).EnableWindow(count == 1);
   m_colorButton4.EnableWindow(count == 1);
   SendDlgItemMessage(IDC_OPACITY_CHECK, BM_SETCHECK, mat.m_bOpacityActive ? BST_CHECKED : BST_UNCHECKED, 0);
   setItemText(IDC_OPACITY_EDIT, mat.m_fOpacity);
   setItemText(IDC_EDGEALPHA_EDIT, mat.m_fEdgeAlpha);
   setItemText(IDC_THICKNESS_EDIT, mat.m_fThickness);
   m_colorButton4.SetColor(mat.m_cRefractionTint);

   // Physics
   setItemText(IDC_MAT_ELASTICITY, mat.m_fElasticity);
   setItemText(IDC_MAT_ELASTICITY_FALLOFF, mat.m_fElasticityFalloff);
   setItemText(IDC_MAT_FRICTION, mat.m_fFriction);
   setItemText(IDC_MAT_SCATTER_ANGLE, mat.m_fScatterAngle);

   ::SetFocus(m_hMaterialList); // workaround focus issue, where the main UI window is in focus instead :/
}

void MaterialDialog::SaveEditedMaterial(Material& mat)
{
   CCO(PinTable) *const pt = g_pvp->GetActiveTable();
   float fv;
   fv = saturate(getItemText(IDC_DIFFUSE_EDIT));
   if (mat.m_fWrapLighting != fv)
         pt->SetNonUndoableDirty(eSaveDirty);
   mat.m_fWrapLighting = fv;
   fv = saturate(getItemText(IDC_GLOSSY_EDIT));
   if (mat.m_fRoughness != fv)
         pt->SetNonUndoableDirty(eSaveDirty);
   mat.m_fRoughness = fv;
   fv = saturate(getItemText(IDC_GLOSSY_IMGLERP_EDIT));
   if (mat.m_fGlossyImageLerp != fv)
         pt->SetNonUndoableDirty(eSaveDirty);
   mat.m_fGlossyImageLerp = fv;
   fv = saturate(getItemText(IDC_THICKNESS_EDIT));
   if (mat.m_fThickness != fv)
         pt->SetNonUndoableDirty(eSaveDirty);
   mat.m_fThickness = fv;
   fv = saturate(getItemText(IDC_SPECULAR_EDIT));
   if (mat.m_fEdge != fv)
         pt->SetNonUndoableDirty(eSaveDirty);
   mat.m_fEdge = fv;
   fv = saturate(getItemText(IDC_OPACITY_EDIT));
   if (mat.m_fOpacity != fv)
         pt->SetNonUndoableDirty(eSaveDirty);
   mat.m_fOpacity = fv;
   size_t type = SendDlgItemMessage(IDC_MATERIAL_TYPE, CB_GETCURSEL, 0, 0);
   if (mat.m_type != type)
         pt->SetNonUndoableDirty(eSaveDirty);
   mat.m_type = (Material::MaterialType)type;
   size_t checked = IsDlgButtonChecked(IDC_OPACITY_CHECK);
   if (mat.m_bOpacityActive != (checked == 1))
         pt->SetNonUndoableDirty(eSaveDirty);
   mat.m_bOpacityActive = (checked == 1);
   fv = saturate(getItemText(IDC_EDGEALPHA_EDIT));
   if (mat.m_fEdgeAlpha != fv)
         pt->SetNonUndoableDirty(eSaveDirty);
   mat.m_fEdgeAlpha = fv;

   fv = getItemText(IDC_MAT_ELASTICITY);
   if (mat.m_fElasticity != fv)
         pt->SetNonUndoableDirty(eSaveDirty);
   mat.m_fElasticity = fv;
   fv = getItemText(IDC_MAT_ELASTICITY_FALLOFF);
   if (mat.m_fElasticityFalloff != fv)
         pt->SetNonUndoableDirty(eSaveDirty);
   mat.m_fElasticityFalloff = fv;
   fv = getItemText(IDC_MAT_FRICTION);
   if (mat.m_fFriction != fv)
         pt->SetNonUndoableDirty(eSaveDirty);
   mat.m_fFriction = fv;
   fv = getItemText(IDC_MAT_SCATTER_ANGLE);
   if (mat.m_fScatterAngle != fv)
         pt->SetNonUndoableDirty(eSaveDirty);
   mat.m_fScatterAngle = fv;

   pt->UpdatePropertyMaterialList();
}

INT_PTR MaterialDialog::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   m_resizer.HandleMessage(uMsg, wParam, lParam);

   switch (uMsg)
   {
      case WM_DESTROY:
      {
         if (m_whereUsedDlg_Materials.IsWindow())
            {
            m_whereUsedDlg_Materials.Destroy(); //Need to destroy the 'WhereUsed' dialog when ImageDialog is closed.  Otherwise there can be a run time error when loading a new table.
            }
      }
      break;
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
               if (pinfo->item.pszText == nullptr || pinfo->item.pszText[0] == '\0')
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
                  pmat->m_szName = pinfo->item.pszText;
                  ListView_SetItemText(m_hMaterialList, pinfo->item.iItem, 0, pinfo->item.pszText);
               }
               else
               {
                  string textBuf;
                  int suffix = 1;
                  do
                  {
                     textBuf = pinfo->item.pszText + std::to_string(suffix);
                     suffix++;
                  } while (!pt->IsMaterialNameUnique(textBuf));
                  pmat->m_szName = textBuf;
                  ListView_SetItemText(m_hMaterialList, pinfo->item.iItem, 0, (LPSTR)pmat->m_szName.c_str());
               }
               pt->SetNonUndoableDirty(eSaveDirty);
               pt->UpdatePropertyMaterialList();
               return TRUE;
            }
            case LVN_ITEMCHANGING:
            {
               if (m_deletingItem)
                   break;
               if (ListView_GetSelectedCount(m_hMaterialList) > 1)
               {
                  EnableAllMaterialDialogItems(FALSE);
                  break;
               }
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
                     SetEditedMaterial(*pmat);
                  }
               }
               break;
            }
            case LVN_ITEMCHANGED:
            {
               if (m_deletingItem)
                   break;
               if (ListView_GetSelectedCount(m_hMaterialList) > 1)
               {
                  EnableAllMaterialDialogItems(FALSE);
                  break;
               }
               NMLISTVIEW * const plistview = (LPNMLISTVIEW)lParam;
               const int sel = plistview->iItem;
               LVITEM lvitem;
               lvitem.mask = LVIF_PARAM;
               lvitem.iItem = sel;
               lvitem.iSubItem = 0;
               ListView_GetItem(m_hMaterialList, &lvitem);
               Material * const pmat = (Material*)lvitem.lParam;
               if ((plistview->uNewState & LVIS_SELECTED) == 0)
                  SaveEditedMaterial(*pmat);
               else if ((plistview->uOldState & LVIS_SELECTED) == 0)
                  SetEditedMaterial(*pmat);
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
         const UINT nID = static_cast<UINT>(wParam);
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
         else if (nID == IDC_COLOR_BUTTON5)
         {
             m_colorButton4.DrawItem(lpDrawItemStruct);
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

         size_t type = SendDlgItemMessage(IDC_MATERIAL_TYPE, CB_SETCURSEL, pmat->m_type, 0);
         if (pmat->m_type != type)
            pt->SetNonUndoableDirty(eSaveDirty);
         pmat->m_type = (Material::MaterialType)type;

         size_t checked = IsDlgButtonChecked(IDC_OPACITY_CHECK);
         if (pmat->m_bOpacityActive != (checked == 1))
            pt->SetNonUndoableDirty(eSaveDirty);
         pmat->m_bOpacityActive = (checked == 1);

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

         pt->UpdatePropertyMaterialList();
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
   const int x = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "MaterialMngPosX"s, 0);
   const int y = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "MaterialMngPosY"s, 0);
   const int w = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "MaterialMngWidth"s, 1000);
   const int h = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "MaterialMngHeight"s, 800);
   POINT p { x, y };
   if (MonitorFromPoint(p, MONITOR_DEFAULTTONULL) != NULL) // Do not apply if point is offscreen
      SetWindowPos(nullptr, x, y, w, h, SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE);
}

void MaterialDialog::SavePosition()
{
    const CRect rect = GetWindowRect();

    g_pvp->m_settings.SaveValue(Settings::Editor, "MaterialMngPosX"s, (int)rect.left);
    g_pvp->m_settings.SaveValue(Settings::Editor, "MaterialMngPosY"s, (int)rect.top);
    const int w = rect.right - rect.left;
    g_pvp->m_settings.SaveValue(Settings::Editor, "MaterialMngWidth"s, w);
    const int h = rect.bottom - rect.top;
    g_pvp->m_settings.SaveValue(Settings::Editor, "MaterialMngHeight"s, h);
}

void MaterialDialog::ShowWhereUsed()
{
    CCO(PinTable) *const ptCur = g_pvp->GetActiveTable();
    if (ptCur)
    {
      m_whereUsedDlg_Materials.m_whereUsedSource = MATERIALS;
      if (m_whereUsedDlg_Materials.DoModal() == IDOK)
      {
         SetFocus();
      }
    }
}
