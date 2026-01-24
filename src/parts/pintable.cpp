// license:GPLv3+

#include "core/stdafx.h"
#include "core/vpversion.h"
#include "parts/Sound.h"
#include "ui/resource.h"
#include "utils/hash.h"
#include <algorithm>
#include "utils/objloader.h"
#include "tinyxml2/tinyxml2.h"
#include <fstream>
#include <sstream>
#include "renderer/Shader.h"
#include "ui/VPXFileFeedback.h"
#ifndef __STANDALONE__
#include "ui/dialogs/VPXLoadFileProgressBar.h"
#include "ui/dialogs/VPXSaveFileProgressBar.h"
#include "FreeImage.h"
#endif
#include "ThreadPool.h"
#include "core/VPXPluginAPIImpl.h"
#include "core/ScriptGlobalTable.h"

#include "utils/ushock_output.h"
#include "ui/paintsur.h"

#include "ui/live/ingameui/InGameUIItem.h"

#ifdef __STANDALONE__
#include "mINI/ini.h"
#endif

#define HASHLENGTH 16

constexpr unsigned char TABLE_KEY[] = "Visual Pinball";
//constexpr unsigned char PARAPHRASE_KEY[] = { 0xB4, 0x0B, 0xBE, 0x37, 0xC3, 0x0C, 0x8E, 0xA1, 0x5A, 0x05, 0xDF, 0x1B, 0x2D, 0x02, 0xEF, 0x8D };

#if defined(__clang__)
static inline std::from_chars_result my_from_chars(const char* first, const char* last, float &value)
{
   char* e;
   const float c = std::strtof(first, &e);
   if(first != e)
      value = c;

   std::from_chars_result result;
   result.ptr = nullptr; //!!
   result.ec = (first == e) ? std::errc::invalid_argument : std::errc{}; //!!
   return result;
}
static inline std::from_chars_result my_from_chars(const char* first, const char* last, int &value)
{
   return std::from_chars(first,last,value);
}
#else
#define my_from_chars std::from_chars
#endif

PinTable::PinTable()
   : m_settings(&(g_pvp->m_settings))
   , m_undo(this)
{
   m_renderSolid = m_settings.GetEditor_RenderSolid();
   ClearMultiSel();

   SetDefaultPhysics(false);

   m_PhysicsMaxLoops = m_settings.GetPlayer_PhysicsMaxLoops();

   UpdateCurrentBGSet();
   m_currentBackglassMode = m_viewMode;

   CComObject<CodeViewer>::CreateInstance(&m_pcv);
   m_pcv->AddRef();
   m_pcv->Init((IScriptableHost*)this);
   m_pcv->Create(nullptr);

   CComObject<ScriptGlobalTable>::CreateInstance(&m_psgt);
   m_psgt->AddRef();
   m_psgt->Init(m_vpinball, this);

   Settings::SetTableOverride_Difficulty_Default(m_difficulty);
   m_globalDifficulty = m_settings.GetTableOverride_Difficulty();

   m_tblAutoStart = m_settings.GetPlayer_Autostart() * 10;
   m_tblAutoStartRetry = m_settings.GetPlayer_AutostartRetry() * 10;
   m_tblAutoStartEnabled = m_settings.GetPlayer_asenable();

   m_tblNudgeRead = Vertex2D(0.f,0.f);
   m_tblNudgePlumb = Vertex2D(0.f,0.f);
}

PinTable::~PinTable()
{
   m_textureMap.clear();
   m_materialMap.clear();
   m_lightMap.clear();
   m_renderprobeMap.clear();

   for (IEditable* edit : m_vedit)
      edit->Release();

   if (m_liveBaseTable == nullptr)
   { // Sounds, Fonts and images are owned by the editor's table, live table instances just use shallow copy, so don't release them
      for (size_t i = 0; i < m_vsound.size(); i++)
         delete m_vsound[i];

      for (size_t i = 0; i < m_vimage.size(); i++)
         delete m_vimage[i];
  
      for (size_t i = 0; i < m_vfont.size(); i++)
      {
         m_vfont[i]->UnRegister();
         delete m_vfont[i];
      }
   }

   for (size_t i = 0; i < m_vliveimage.size(); i++)
      delete m_vliveimage[i];

   for (size_t i = 0; i < m_materials.size(); ++i)
      delete m_materials[i];

   for (size_t i = 0; i < m_vrenderprobe.size(); ++i)
      delete m_vrenderprobe[i];

   for (int i = 0; i < m_vcollection.size(); i++)
      m_vcollection.ElementAt(i)->Release();

   m_pcv->CleanUpScriptEngine();
   m_pcv->Release();
   m_pcv = nullptr;

   m_psgt->Release();
   m_psgt = nullptr;

#ifndef __STANDALONE__
   if (m_hbmOffScreen)
      DeleteObject(m_hbmOffScreen);
#endif

   if (m_liveBaseTable)
      m_liveBaseTable->Release();
}

void PinTable::FVerifySaveToClose()
{
#ifndef __STANDALONE__
   if (!m_vAsyncHandles.empty())
   {
      /*const DWORD wait =*/ WaitForMultipleObjects((DWORD)m_vAsyncHandles.size(), m_vAsyncHandles.data(), TRUE, INFINITE);
      //m_vpinball->MessageBox("Async work items not done", nullptr, 0);

      // Close the remaining handles here, since the window messages will never be processed
      for (size_t i = 0; i < m_vAsyncHandles.size(); i++)
         CloseHandle(m_vAsyncHandles[i]);

      m_vpinball->SetActionCur(string());
   }
#endif
}

void PinTable::UpdatePropertyImageList()
{ 
#ifndef __STANDALONE__
    // just update the combo boxes in the property dialog
    g_pvp->GetPropertiesDocker()->GetContainProperties()->GetPropertyDialog()->UpdateTabs(m_vmultisel);
#endif
}

void PinTable::UpdatePropertyMaterialList()
{
#ifndef __STANDALONE__
    // just update the combo boxes in the property dialog
    g_pvp->GetPropertiesDocker()->GetContainProperties()->GetPropertyDialog()->UpdateTabs(m_vmultisel);
#endif
}

void PinTable::ClearForOverwrite()
{
   for (size_t i = 0; i < m_materials.size(); ++i)
      delete m_materials[i];
   m_materials.clear();

   for (size_t i = 0; i < m_vrenderprobe.size(); i++)
      delete m_vrenderprobe[i];
   m_vrenderprobe.clear();
}

void PinTable::InitBuiltinTable(const size_t tableId)
{
#ifndef __STANDALONE__
   string path;
   // Get our new table resource, get it to be opened as a storage, and open it like a normal file
   switch (tableId)
   {
   case ID_NEW_EXAMPLETABLE: path = "exampleTable.vpx"s; break;
   case ID_NEW_STRIPPEDTABLE: path = "strippedTable.vpx"s; break;
   case ID_NEW_LIGHTSEQTABLE: path = "lightSeqTable.vpx"s; break;
   case ID_NEW_BLANKTABLE:
   default: path = "blankTable.vpx"s;
   }
   m_glassTopHeight = m_glassBottomHeight = 210;
   for (int i = 0; i < 16; i++)
      m_rgcolorcustom[i] = RGB(0, 0, 0);
   LoadGameFromFilename(g_pvp->GetAppPath(VPinball::AppSubFolder::Assets, path).string());
   m_title = LocalString(IDS_TABLE).m_szbuffer /*"Table"*/ + std::to_string(m_vpinball->m_NextTableID);
   m_vpinball->m_NextTableID++;
   m_settings.SetIniPath(string());
   m_filename.clear();
#endif
}

void PinTable::SetDefaultView()
{
   FRect frect;
   GetViewRect(&frect);
   m_offset = frect.Center();
   m_zoom = 0.5f;
}

void PinTable::SetCaption(const string& szCaption)
{
#ifndef __STANDALONE__
   if (m_mdiTable != nullptr && m_mdiTable->IsWindow())
      m_mdiTable->SetWindowText(szCaption.c_str());
   m_pcv->SetCaption(szCaption);
#endif
}

void PinTable::SetMouseCapture()
{
#ifndef __STANDALONE__
   SetCapture();
#endif
}

int PinTable::ShowMessageBox(const char *text) const
{
   return m_mdiTable->MessageBox(text, "Visual Pinball", MB_YESNO);
}

POINT PinTable::GetScreenPoint() const
{
#ifndef __STANDALONE__
   CPoint pt = GetCursorPos();
   ScreenToClient(pt);
   return pt;
#else
   return POINT();
#endif
}

#define CLEAN_MATERIAL(pEditMaterial) \
{const ankerl::unordered_dense::map<string, Material*, StringHashFunctor, StringComparator>::const_iterator \
   it = m_materialMap.find(pEditMaterial); \
if (it == m_materialMap.end()) \
   pEditMaterial.clear();}

#define CLEAN_IMAGE(pEditImage) \
{const ankerl::unordered_dense::map<string, Texture*, StringHashFunctor, StringComparator>::const_iterator \
   it = m_textureMap.find(pEditImage); \
if (it == m_textureMap.end()) \
   pEditImage.clear();}

#define CLEAN_SURFACE(pEditSurface) \
{if (!pEditSurface.empty()) \
{ \
const wstring es = MakeWString(pEditSurface); \
bool found = false; \
for (size_t ie = 0; ie < m_vedit.size(); ie++) \
{ \
    const IEditable* const item = m_vedit[ie]; \
    if (item->GetItemType() == eItemSurface || item->GetItemType() == eItemRamp) \
    { \
        if (es == item->GetScriptable()->m_wzName) \
        { \
            found = true; \
            break; \
        } \
    } \
} \
if(!found) \
    pEditSurface.clear(); \
}}


void PinTable::InitTablePostLoad()
{
   PLOGI << "InitTablePostLoad"; // For profiling

   for (unsigned int i = 1; i < NUM_BG_SETS; ++i)
      if (mViewSetups[i].mFOV == FLT_MAX) // old table, copy FS and/or FSS settings over from old DT setting
      {
         mViewSetups[i] = mViewSetups[BG_DESKTOP];
         if (m_BG_image[i].empty() && i == BG_FSS) // copy image over for FSS mode
            m_BG_image[i] = m_BG_image[BG_DESKTOP];
      }

   Settings::SetTableOverride_Difficulty_Default(m_difficulty);
   m_globalDifficulty = m_settings.GetTableOverride_Difficulty();

   m_currentBackglassMode = m_viewMode;
   if (m_isFSSViewModeEnabled)
      m_currentBackglassMode = BG_FSS;

   m_hbmOffScreen = nullptr;
   m_dirtyDraw = true;

   SetDefaultView();

   m_pcv->AddItem(this, false);
   m_pcv->AddItem(m_psgt, true);
   m_pcv->AddItem(m_pcv->m_pdm, false);

   RemoveInvalidReferences();
}

// cleanup old bugs, i.e. currently buggy/non-existing material, image & surface names
// (also does the same for the <None> entries of droplists)
void PinTable::RemoveInvalidReferences()
{
   // set up the texture & material hashtables for faster access
   m_textureMap.clear();
   for (size_t i = 0; i < m_vimage.size(); i++)
       m_textureMap[m_vimage[i]->m_name] = m_vimage[i];
   m_materialMap.clear();
   for (size_t i = 0; i < m_materials.size(); i++)
       m_materialMap[m_materials[i]->m_name] = m_materials[i];

   for (size_t i = 0; i < m_vedit.size(); i++)
   {
        IEditable* const pEdit = m_vedit[i];
        if (pEdit == nullptr)
            continue;

        switch (pEdit->GetItemType())
        {
        case eItemPrimitive:
        {
            CLEAN_MATERIAL(((Primitive*)pEdit)->m_d.m_szMaterial);
            CLEAN_MATERIAL(((Primitive*)pEdit)->m_d.m_szPhysicsMaterial);
            CLEAN_IMAGE(((Primitive*)pEdit)->m_d.m_szImage);
            CLEAN_IMAGE(((Primitive*)pEdit)->m_d.m_szNormalMap);
            break;
        }
        case eItemRamp:
        {
            CLEAN_MATERIAL(((Ramp*)pEdit)->m_d.m_szMaterial);
            CLEAN_MATERIAL(((Ramp*)pEdit)->m_d.m_szPhysicsMaterial);
            CLEAN_IMAGE(((Ramp*)pEdit)->m_d.m_szImage);
            break;
        }
        case eItemSurface:
        {
            //CLEAN_MATERIAL(((Surface*)pEdit)->m_d.m_szMaterial);
            CLEAN_MATERIAL(((Surface*)pEdit)->m_d.m_szPhysicsMaterial);
            CLEAN_MATERIAL(((Surface*)pEdit)->m_d.m_szSideMaterial);
            CLEAN_MATERIAL(((Surface*)pEdit)->m_d.m_szTopMaterial);
            CLEAN_MATERIAL(((Surface*)pEdit)->m_d.m_szSlingShotMaterial);
            CLEAN_IMAGE(((Surface*)pEdit)->m_d.m_szImage);
            CLEAN_IMAGE(((Surface*)pEdit)->m_d.m_szSideImage);
            break;
        }
        case eItemDecal:
        {
            CLEAN_MATERIAL(((Decal*)pEdit)->m_d.m_szMaterial);
            //CLEAN_MATERIAL(((Decal*)pEdit)->m_d.m_szPhysicsMaterial);
            CLEAN_IMAGE(((Decal*)pEdit)->m_d.m_szImage);
            CLEAN_SURFACE(((Decal*)pEdit)->m_d.m_szSurface);
            break;
        }
        case eItemFlipper:
        {
            CLEAN_MATERIAL(((Flipper*)pEdit)->m_d.m_szMaterial);
            //CLEAN_MATERIAL(((Flipper*)pEdit)->m_d.m_szPhysicsMaterial);
            CLEAN_MATERIAL(((Flipper*)pEdit)->m_d.m_szRubberMaterial);
            CLEAN_IMAGE(((Flipper*)pEdit)->m_d.m_szImage);
            CLEAN_SURFACE(((Flipper*)pEdit)->m_d.m_szSurface);
            break;
        }
        case eItemHitTarget:
        {
            CLEAN_MATERIAL(((HitTarget*)pEdit)->m_d.m_szMaterial);
            CLEAN_MATERIAL(((HitTarget*)pEdit)->m_d.m_szPhysicsMaterial);
            CLEAN_IMAGE(((HitTarget*)pEdit)->m_d.m_szImage);
            break;
        }
        case eItemPlunger:
        {
            CLEAN_MATERIAL(((Plunger*)pEdit)->m_d.m_szMaterial);
            //CLEAN_MATERIAL(((Plunger*)pEdit)->m_d.m_szPhysicsMaterial);
            CLEAN_IMAGE(((Plunger*)pEdit)->m_d.m_szImage);
            CLEAN_SURFACE(((Plunger*)pEdit)->m_d.m_szSurface);
            break;
        }
        case eItemSpinner:
        {
            CLEAN_MATERIAL(((Spinner*)pEdit)->m_d.m_szMaterial);
            //CLEAN_MATERIAL(((Spinner*)pEdit)->m_d.m_szPhysicsMaterial);
            CLEAN_IMAGE(((Spinner*)pEdit)->m_d.m_szImage);
            CLEAN_SURFACE(((Spinner*)pEdit)->m_d.m_szSurface);
            break;
        }
        case eItemRubber:
        {
            CLEAN_MATERIAL(((Rubber*)pEdit)->m_d.m_szMaterial);
            CLEAN_MATERIAL(((Rubber*)pEdit)->m_d.m_szPhysicsMaterial);
            CLEAN_IMAGE(((Rubber*)pEdit)->m_d.m_szImage);
            break;
        }
        case eItemBumper:
        {
            //CLEAN_MATERIAL(((Bumper*)pEdit)->m_d.m_szMaterial);
            //CLEAN_MATERIAL(((Bumper*)pEdit)->m_d.m_szPhysicsMaterial);
            CLEAN_MATERIAL(((Bumper*)pEdit)->m_d.m_szCapMaterial);
            CLEAN_MATERIAL(((Bumper*)pEdit)->m_d.m_szBaseMaterial);
            CLEAN_MATERIAL(((Bumper*)pEdit)->m_d.m_szSkirtMaterial);
            CLEAN_MATERIAL(((Bumper*)pEdit)->m_d.m_szRingMaterial);
            CLEAN_SURFACE(((Bumper*)pEdit)->m_d.m_szSurface);
            break;
        }
        case eItemKicker:
        {
            CLEAN_MATERIAL(((Kicker*)pEdit)->m_d.m_szMaterial);
            //CLEAN_MATERIAL(((Kicker*)pEdit)->m_d.m_szPhysicsMaterial);
            CLEAN_SURFACE(((Kicker*)pEdit)->m_d.m_szSurface);
            break;
        }
        case eItemTrigger:
        {
            CLEAN_MATERIAL(((Trigger*)pEdit)->m_d.m_szMaterial);
            //CLEAN_MATERIAL(((Trigger*)pEdit)->m_d.m_szPhysicsMaterial);
            CLEAN_SURFACE(((Trigger*)pEdit)->m_d.m_szSurface);
            break;
        }
        case eItemDispReel:
        {
            CLEAN_IMAGE(((DispReel*)pEdit)->m_d.m_szImage);
            break;
        }
        case eItemFlasher:
        {
            CLEAN_IMAGE(((Flasher*)pEdit)->m_d.m_szImageA);
            CLEAN_IMAGE(((Flasher*)pEdit)->m_d.m_szImageB);
            break;
        }
        case eItemLight:
        {
            CLEAN_IMAGE(((Light*)pEdit)->m_d.m_szImage);
            CLEAN_SURFACE(((Light*)pEdit)->m_d.m_szSurface);
            break;
        }
        case eItemGate:
        {
            CLEAN_SURFACE(((Gate*)pEdit)->m_d.m_szSurface);
            break;
        }
        default:
        {
            break;
        }
        }
   }

   m_textureMap.clear();
   m_materialMap.clear();
}

bool PinTable::IsNameUnique(const wstring& wzName) const
{
   return m_pcv->m_vcvd.GetSortedIndex(wzName) == -1;
}

void PinTable::GetUniqueName(const ItemTypeEnum type, WCHAR *const wzUniqueName, const size_t wzUniqueName_maxlength) const
{
   WCHAR wzRoot[256];
   GetTypeNameForType(type, wzRoot);
   GetUniqueName(wzRoot, wzUniqueName, wzUniqueName_maxlength);
}

void PinTable::GetUniqueName(const wstring& wzRoot, WCHAR *const wzUniqueName, const size_t wzUniqueName_maxlength) const
{
   int suffix = 1;
   wstring wzName;
   do
   {
      wzName = (wzRoot.length() > wzUniqueName_maxlength - 3 ? wzRoot.substr(0, wzUniqueName_maxlength - 3) : wzRoot)
         + ((suffix <  10) ? (L"00" + std::to_wstring(suffix))
         :  (suffix < 100) ? (L"0"  + std::to_wstring(suffix))
         :                            std::to_wstring(suffix));
      suffix++;
   } while (!IsNameUnique(wzName) && suffix < 1000);
   wcsncpy_s(wzUniqueName, wzUniqueName_maxlength, wzName.c_str());
}

void PinTable::GetUniqueNamePasting(const int type, WCHAR * const wzUniqueName, const size_t wzUniqueName_maxlength) const
{
   //if the original name is not yet used, use that one (so there's nothing we have to do) 
   //otherwise add/increase the suffix until we find a name that's not used yet
   if (!IsNameUnique(wzUniqueName))
   {
      //first remove the existing suffix
      wstring input = wzUniqueName;
      size_t lastNonDigit = input.length();
      while (lastNonDigit > 0 && iswdigit(input[lastNonDigit - 1]))
         --lastNonDigit;
      GetUniqueName(input.substr(0, lastNonDigit), wzUniqueName, wzUniqueName_maxlength);
   }
}

void PinTable::UIRenderPass2(Sur * const psur)
{
#ifndef __STANDALONE__
   const CRect rc = GetClientRect();
   psur->SetFillColor(m_vpinball->m_backgroundColor);
   psur->SetBorderColor(-1, false, 0);

   FRect frect;
   GetViewRect(&frect);

   psur->Rectangle2(rc.left, rc.top, rc.right, rc.bottom);

   if (m_backdrop)
   {
      Texture * const ppi = GetImage((!m_vpinball->m_backglassView) ? m_image : m_BG_image[m_viewMode]);

      if (ppi && ppi->GetGDIBitmap())
      {
         CDC dc;
         dc.CreateCompatibleDC(nullptr);
         const CBitmap hbmOld = dc.SelectObject(ppi->GetGDIBitmap());

         psur->Image(frect.left, frect.top, frect.right, frect.bottom, dc.GetHDC(), ppi->m_width, ppi->m_height);

         dc.SelectObject(hbmOld);
      }
   }

   if (m_vpinball->m_backglassView)
   {
      Render3DProjection(psur);
   }

   for(const auto &ptr : m_vedit)
   {
      if (ptr->m_backglass == m_vpinball->m_backglassView && ptr->GetISelect()->m_isVisible)
        ptr->UIRenderPass1(psur);
   }

   if (m_grid && m_vpinball->m_gridSize > 0)
   {
      Vertex2D rlt = psur->ScreenToSurface(rc.left, rc.top);
      Vertex2D rrb = psur->ScreenToSurface(rc.right, rc.bottom);
      rlt.x = max(rlt.x, frect.left);
      rlt.y = max(rlt.y, frect.top);
      rrb.x = min(rrb.x, frect.right);
      rrb.y = min(rrb.y, frect.bottom);

      const float gridsize = (float)m_vpinball->m_gridSize;

      const int beginx = (int)(rlt.x / gridsize);
      const int lenx = (int)((rrb.x - rlt.x) / gridsize);//(((rc.right - rc.left)/m_zoom));
      const int beginy = (int)(rlt.y / gridsize);
      const int leny = (int)((rrb.y - rlt.y) / gridsize);//(((rc.bottom - rc.top)/m_zoom));

      psur->SetObject(nullptr); // Don't hit test gridlines

      psur->SetLineColor(RGB(190, 220, 240), false, 0);
      for (int i = 0; i < (lenx + 1); i++)
      {
         const float x = (float)(beginx + i)*gridsize;
         psur->Line(x, rlt.y, x, rrb.y);
      }

      for (int i = 0; i < (leny + 1); i++)
      {
         const float y = (float)(beginy + i)*gridsize;
         psur->Line(rlt.x, y, rrb.x, y);
      }
   }

   for(const auto &ptr : m_vedit)
   {
      if (ptr->m_backglass == m_vpinball->m_backglassView && ptr->GetISelect()->m_isVisible)
        ptr->UIRenderPass2(psur);
   }

   if (m_vpinball->m_backglassView) // Outline of the view, for when the grid is off
   {
      psur->SetObject(nullptr);
      psur->SetFillColor(-1);
      psur->SetBorderColor(RGB(0, 0, 0), false, 1);
      psur->Rectangle(0, 0, EDITOR_BG_WIDTH, EDITOR_BG_HEIGHT);
   }

   if (m_dragging)
   {
      psur->SetFillColor(-1);
      psur->SetBorderColor(RGB(0, 0, 0), true, 0);
      psur->Rectangle(m_rcDragRect.left, m_rcDragRect.top, m_rcDragRect.right, m_rcDragRect.bottom);
   }

   // display the layer string
   //    psur->SetObject(nullptr);
   //    SetTextColor( psur->m_hdc,RGB(180,180,180));
   //    char text[64];
   //    char number[8];
   //    strncpy_s( text, sizeof(text), "Layer_");
   //    _itoa_s(activeLayer+1, number, 10 );
   //    strcat_s( text, number);
   //    RECT textRect;
   //    SetRect( &textRect, rc.right-60,rc.top, rc.right, rc.top+30 );
   //    DrawText( psur->m_hdc, text, -1, &textRect, DT_LEFT);
   // 
   //    SetTextColor( psur->m_hdc,RGB(0,0,0));

   //   psur->DrawText( text,rc.left+10, rc.top, 90,20);
#endif
}

// draws the backdrop content
void PinTable::Render3DProjection(Sur * const psur)
{
   if (m_vedit.empty())
      return;

   // dummy coordinate system for backdrop view
   ModelViewProj mvp;
   if (mViewSetups[m_currentBackglassMode].mMode == VLM_WINDOW)
      mViewSetups[m_currentBackglassMode].SetWindowModeFromSettings(this);
   mViewSetups[m_currentBackglassMode].ComputeMVP(this, (float)EDITOR_BG_WIDTH / (float)EDITOR_BG_HEIGHT, false, mvp);

   Vertex3Ds rgvIn[8];
   rgvIn[0].x = m_left;  rgvIn[0].y = m_top;    rgvIn[0].z = 50.0f;
   rgvIn[1].x = m_left;  rgvIn[1].y = m_top;    rgvIn[1].z = m_glassTopHeight;
   rgvIn[2].x = m_right; rgvIn[2].y = m_top;    rgvIn[2].z = m_glassTopHeight;
   rgvIn[3].x = m_right; rgvIn[3].y = m_top;    rgvIn[3].z = 50.0f;
   rgvIn[4].x = m_right; rgvIn[4].y = m_bottom; rgvIn[4].z = 50.0f;
   rgvIn[5].x = m_right; rgvIn[5].y = m_bottom; rgvIn[5].z = m_glassBottomHeight;
   rgvIn[6].x = m_left;  rgvIn[6].y = m_bottom; rgvIn[6].z = m_glassBottomHeight;
   rgvIn[7].x = m_left;  rgvIn[7].y = m_bottom; rgvIn[7].z = 50.0f;

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
void PinTable::Paint(HDC hdc)
{
#ifndef __STANDALONE__
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
      Sur * const psur = new PaintSur(dc.GetHDC(), m_zoom, m_offset.x, m_offset.y, rc.right - rc.left, rc.bottom - rc.top, GetSelectedItem());
      UIRenderPass2(psur);

      delete psur;
   }

   BitBlt(hdc, rc.left, rc.top, rc.right, rc.bottom, dc.GetHDC(), 0, 0, SRCCOPY);

   dc.SelectObject(hbmOld);
#endif

   m_dirtyDraw = false;
}

ISelect *PinTable::HitTest(const int x, const int y)
{
#ifndef __STANDALONE__
   const CDC dc;

   const CRect rc = GetClientRect();

   HitSur phs (dc.GetHDC(), m_zoom, m_offset.x, m_offset.y, rc.right - rc.left, rc.bottom - rc.top, x, y, this);
   HitSur phs2(dc.GetHDC(), m_zoom, m_offset.x, m_offset.y, rc.right - rc.left, rc.bottom - rc.top, x, y, this);

   m_allHitElements.clear();

   UIRenderPass2(&phs);

   for (size_t i = 0; i < m_vedit.size(); i++)
   {
      IEditable * const ptr = m_vedit[i];
      if (ptr->m_backglass == m_vpinball->m_backglassView)
      {
         ptr->UIRenderPass1(&phs2);
         ISelect* const tmp = phs2.m_pselected;
         if (FindIndexOf(m_allHitElements, tmp) == -1 && tmp != nullptr && tmp != this)
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
#else
   return nullptr;
#endif
}

void PinTable::SetDirtyDraw()
{
   if(g_pplayer)
       return;

   m_dirtyDraw = true;
#ifndef __STANDALONE__
   InvalidateRect(false);
#endif
}

PinTable* PinTable::CopyForPlay()
{
   const PinTable *src = this;
   CComObject<PinTable> *live_table;
   CComObject<PinTable>::CreateInstance(&live_table);
   live_table->AddRef();
   live_table->m_liveBaseTable = this;
   AddRef(); // as the live table holds a re ference on this

   CComObject<PinTable> *dst = live_table;
   #ifndef __STANDALONE__
      const size_t cchar = ::SendMessage(src->m_pcv->m_hwndScintilla, SCI_GETTEXTLENGTH, 0, 0);
      string script(cchar, '\0');
      ::SendMessage(m_pcv->m_hwndScintilla, SCI_GETTEXT, cchar + 1, (size_t)script.data());
      script = VPXPluginAPIImpl::GetInstance().ApplyScriptCOMObjectOverrides(script);
      ::SendMessage(dst->m_pcv->m_hwndScintilla, SCI_SETTEXT, 0, (size_t)script.c_str());
   #else
      dst->m_pcv->m_script_text = VPXPluginAPIImpl::GetInstance().ApplyScriptCOMObjectOverrides(src->m_pcv->m_script_text);
   #endif

   dst->m_settings.SetIniPath(src->m_settings.GetIniPath());
   dst->m_settings.Load(src->m_settings);

   dst->m_title = src->m_title;
   dst->m_filename = src->m_filename;
   dst->m_tableName = src->m_tableName;
   dst->m_author = src->m_author;
   dst->m_version = src->m_version;
   dst->m_releaseDate = src->m_releaseDate;
   dst->m_authorEMail = src->m_authorEMail;
   dst->m_webSite = src->m_webSite;
   dst->m_blurb = src->m_blurb;
   dst->m_description = src->m_description;
   dst->m_rules = src->m_rules;
   dst->m_screenShot = src->m_screenShot;
   dst->m_dateSaved = src->m_dateSaved;

   dst->m_left = src->m_left;
   dst->m_top = src->m_top;
   dst->m_right = src->m_right;
   dst->m_bottom = src->m_bottom;
   dst->m_overridePhysics = src->m_overridePhysics;
   dst->m_overridePhysicsFlipper = src->m_overridePhysicsFlipper;
   dst->m_Gravity = src->m_Gravity;
   dst->m_friction = src->m_friction;
   dst->m_elasticity = src->m_elasticity;
   dst->m_elasticityFalloff = src->m_elasticityFalloff;
   dst->m_scatter = src->m_scatter;
   dst->m_defaultScatter = src->m_defaultScatter;
   dst->m_nudgeTime = src->m_nudgeTime;
   dst->m_PhysicsMaxLoops = src->m_PhysicsMaxLoops;
   dst->m_renderEMReels = src->m_renderEMReels;
   dst->m_renderDecals = src->m_renderDecals;
   dst->m_offset = src->m_offset;
   dst->m_zoom = src->m_zoom;
   dst->m_angletiltMax = src->m_angletiltMax;
   dst->m_angletiltMin = src->m_angletiltMin;
   dst->m_image = src->m_image;
   dst->m_ImageBackdropNightDay = src->m_ImageBackdropNightDay;
   dst->m_imageColorGrade = src->m_imageColorGrade;
   dst->m_ballImage = src->m_ballImage;
   dst->m_ballSphericalMapping = src->m_ballSphericalMapping;
   dst->m_ballImageDecal = src->m_ballImageDecal;
   dst->m_envImage = src->m_envImage;
   dst->m_notesText = src->m_notesText;
   dst->m_screenShot = src->m_screenShot;
   dst->m_backdrop = src->m_backdrop;
   dst->m_glassBottomHeight = src->m_glassBottomHeight;
   dst->m_glassTopHeight = src->m_glassTopHeight;
   dst->m_playfieldMaterial = src->m_playfieldMaterial;
   dst->m_colorbackdrop = src->m_colorbackdrop;
   dst->m_difficulty = src->m_difficulty;
   Settings::SetTableOverride_Difficulty_Default(dst->m_difficulty);
   dst->m_globalDifficulty = m_settings.GetTableOverride_Difficulty();
   dst->m_lightAmbient = src->m_lightAmbient;
   dst->m_lightHeight = src->m_lightHeight;
   dst->m_lightRange = src->m_lightRange;
   dst->m_lightEmissionScale = src->m_lightEmissionScale;
   dst->m_envEmissionScale = src->m_envEmissionScale;
   dst->m_globalEmissionScale = src->m_globalEmissionScale;
   dst->m_AOScale = src->m_AOScale;
   dst->m_SSRScale = src->m_SSRScale;
   dst->m_TableSoundVolume = src->m_TableSoundVolume;
   dst->m_TableMusicVolume = src->m_TableMusicVolume;
   dst->m_playfieldReflectionStrength = src->m_playfieldReflectionStrength;
   dst->m_BallDecalMode = src->m_BallDecalMode;
   dst->m_ballPlayfieldReflectionStrength = src->m_ballPlayfieldReflectionStrength;
   dst->m_defaultBulbIntensityScaleOnBall = src->m_defaultBulbIntensityScaleOnBall;
   dst->m_grid = src->m_grid;
   dst->m_enableAO = src->m_enableAO;
   dst->m_enableSSR = src->m_enableSSR;
   dst->m_toneMapper = src->m_toneMapper;
   dst->m_exposure = src->m_exposure;
   dst->m_bloom_strength = src->m_bloom_strength;
   memcpy(dst->m_wzName, src->m_wzName, sizeof(src->m_wzName));

   dst->m_Light[0].emission = src->m_Light[0].emission;

   dst->m_isFSSViewModeEnabled = src->m_isFSSViewModeEnabled;
   dst->m_viewModeOverride = src->m_viewModeOverride;
   dst->m_viewMode = src->m_viewMode;
   dst->UpdateCurrentBGSet();
   dst->m_currentBackglassMode = src->m_currentBackglassMode;
   for (int i = 0; i < 3; i++)
   {
      dst->mViewSetups[i] = src->mViewSetups[i];
      dst->m_BG_image[i] = src->m_BG_image[i];
      dst->mViewSetups[i].ApplyTableOverrideSettings(m_settings, (ViewSetupID) i);
   }
   dst->m_materials.reserve(src->m_materials.size() + dst->m_materials.size());
   for (Material* srcMat : src->m_materials)
   {
      Material *mat = new Material(srcMat);
      dst->m_materials.push_back(mat);
      dst->m_startupToLive[srcMat] = mat;
      dst->m_liveToStartup[mat] = srcMat;
   }

   // Don't perform deep copy for these parts that can't be modified by the script
   dst->m_vimage.reserve(src->m_vimage.size() + dst->m_vimage.size());
   for (Texture* texture : src->m_vimage)
      dst->m_vimage.push_back(texture);
   dst->m_vsound.reserve(src->m_vsound.size() + dst->m_vsound.size());
   for (VPX::Sound* sound : src->m_vsound)
      dst->m_vsound.push_back(sound);
   dst->m_vfont.reserve(src->m_vfont.size() + dst->m_vfont.size());
   for (PinFont* font : src->m_vfont)
      dst->m_vfont.push_back(font);

   PLOGI << "Duplicating parts for live instance"; // For profiling
   for (IEditable* editable : src->m_vedit)
   {
      IEditable* edit_dst = nullptr;
      switch (editable->GetItemType())
      {
      case eItemBall:      edit_dst = static_cast<Ball*>(editable)->CopyForPlay(live_table); break;
      case eItemBumper:    edit_dst = static_cast<Bumper*>(editable)->CopyForPlay(live_table); break;
      case eItemDecal:     edit_dst = static_cast<Decal*>(editable)->CopyForPlay(live_table); break;
      case eItemDispReel:  edit_dst = static_cast<DispReel*>(editable)->CopyForPlay(live_table); break;
      case eItemFlasher:   edit_dst = static_cast<Flasher*>(editable)->CopyForPlay(live_table); break;
      case eItemFlipper:   edit_dst = static_cast<Flipper*>(editable)->CopyForPlay(live_table); break;
      case eItemGate:      edit_dst = static_cast<Gate*>(editable)->CopyForPlay(live_table); break;
      case eItemHitTarget: edit_dst = static_cast<HitTarget*>(editable)->CopyForPlay(live_table); break;
      case eItemKicker:    edit_dst = static_cast<Kicker*>(editable)->CopyForPlay(live_table); break;
      case eItemLight:     edit_dst = static_cast<Light*>(editable)->CopyForPlay(live_table); break;
      case eItemLightSeq:  edit_dst = static_cast<LightSeq*>(editable)->CopyForPlay(live_table); break;
      case eItemPartGroup: edit_dst = static_cast<PartGroup*>(editable)->CopyForPlay(live_table); break;
      case eItemPlunger:   edit_dst = static_cast<Plunger*>(editable)->CopyForPlay(live_table); break;
      case eItemPrimitive: edit_dst = static_cast<Primitive*>(editable)->CopyForPlay(live_table); break;
      case eItemRamp:      edit_dst = static_cast<Ramp*>(editable)->CopyForPlay(live_table); break;
      case eItemRubber:    edit_dst = static_cast<Rubber*>(editable)->CopyForPlay(live_table); break;
      case eItemSpinner:   edit_dst = static_cast<Spinner*>(editable)->CopyForPlay(live_table); break;
      case eItemSurface:   edit_dst = static_cast<Surface*>(editable)->CopyForPlay(live_table); break;
      case eItemTextbox:   edit_dst = static_cast<Textbox*>(editable)->CopyForPlay(live_table); break;
      case eItemTimer:     edit_dst = static_cast<Timer*>(editable)->CopyForPlay(live_table); break;
      case eItemTrigger:   edit_dst = static_cast<Trigger*>(editable)->CopyForPlay(live_table); break;
      default: assert(false); // Unexpected table part
      }
      if (editable->GetPartGroup())
      {
         PartGroup *dstParent = static_cast<PartGroup *>(dst->GetLiveFromStartup<IEditable>(editable->GetPartGroup()));
         assert(dstParent != nullptr);
         edit_dst->SetPartGroup(dstParent);
      }
      live_table->m_vedit.push_back(edit_dst);
      dst->m_startupToLive[editable] = edit_dst;
      dst->m_liveToStartup[edit_dst] = editable;
   }

   PLOGI << "Duplicating collections"; // For profiling
   live_table->m_vcollection.reserve(m_vcollection.size() + live_table->m_vcollection.size());
   for (int i = 0; i < m_vcollection.size(); i++)
   {
      CComObject<Collection> *pcol;
      CComObject<Collection>::CreateInstance(&pcol);
      pcol->AddRef();
      memcpy(pcol->m_wzName, m_vcollection[i].m_wzName, sizeof(pcol->m_wzName));
      pcol->m_fireEvents = m_vcollection[i].m_fireEvents;
      pcol->m_stopSingleEvents = m_vcollection[i].m_stopSingleEvents;
      pcol->m_groupElements = m_vcollection[i].m_groupElements;
      for (int j = 0; j < m_vcollection[i].m_visel.size(); ++j)
      {
         IEditable* ed = m_vcollection[i].m_visel[j].GetIEditable();
         if (dst->m_startupToLive.find(ed) != dst->m_startupToLive.end())
         {
            auto edit_item = (IEditable *)dst->m_startupToLive[ed];
            edit_item->m_vCollection.push_back(pcol);
            edit_item->m_viCollection.push_back(pcol->m_visel.size());
            pcol->m_visel.push_back(edit_item->GetISelect());
         }
      }
      live_table->m_vcollection.push_back(pcol);
      live_table->m_pcv->AddItem(pcol, false);
   }

   live_table->m_pcv->AddItem(live_table, false);
   live_table->m_pcv->AddItem(live_table->m_psgt, true);
   live_table->m_pcv->AddItem(live_table->m_pcv->m_pdm, false);

   #ifdef __STANDALONE__
      Textbox* const implicitDMD = (Textbox*)EditableRegistry::CreateAndInit(ItemTypeEnum::eItemTextbox, live_table, 0, 0);
      live_table->m_pcv->RemoveItem(implicitDMD->GetScriptable());
      wcsncpy_s(implicitDMD->m_wzName, std::size(implicitDMD->m_wzName), L"ImplicitDMD");
      implicitDMD->m_d.m_visible = false;
      implicitDMD->m_d.m_isDMD = true;
      implicitDMD->m_d.m_fontcolor = RGB(255, 165, 0);
      live_table->m_vedit.push_back(implicitDMD);
      live_table->m_pcv->AddItem(implicitDMD->GetScriptable(), false);
      PLOGI << "Implicit Textbox DMD added: name=" << MakeString(implicitDMD->m_wzName);

      Flasher* const implicitDMD2 = (Flasher*)EditableRegistry::CreateAndInit(ItemTypeEnum::eItemFlasher, live_table, 0, 0);
      live_table->m_pcv->RemoveItem(implicitDMD2->GetScriptable());
      wcsncpy_s(implicitDMD2->m_wzName, std::size(implicitDMD2->m_wzName), L"ImplicitDMD2");
      int dmdWidth = 128 * 4; // (658)
      int dmdHeight = 38 * 4; // (189)
      int x = ((live_table->m_right - live_table->m_left) - dmdWidth) / 2;
      int y = ((live_table->m_bottom - live_table->m_top) - dmdHeight) / 2;
      implicitDMD2->UpdatePoint(0, x, y);
      implicitDMD2->UpdatePoint(1, x, y + dmdHeight);
      implicitDMD2->UpdatePoint(2, x + dmdWidth, y + dmdHeight);
      implicitDMD2->UpdatePoint(3, x + dmdWidth, y);
      implicitDMD2->m_d.m_isVisible = false;
      implicitDMD2->m_d.m_renderMode = FlasherData::DMD;
      implicitDMD2->m_d.m_color = RGB(255, 255, 255);
      implicitDMD2->m_d.m_filter = Filter_Overlay;
      implicitDMD2->m_d.m_imagealignment = ImageModeWrap;
      implicitDMD2->m_d.m_alpha = 150;
      implicitDMD2->m_d.m_intensity_scale = 1;
      implicitDMD2->m_d.m_modulate_vs_add = 1;
      implicitDMD2->m_d.m_addBlend = true;
      live_table->m_vedit.push_back(implicitDMD2);
      live_table->m_pcv->AddItem(implicitDMD2->GetScriptable(), false);
      PLOGI << "Implicit Flasher DMD added: name=" << MakeString(implicitDMD2->m_wzName);
   #endif

   live_table->m_vrenderprobe.reserve(m_vrenderprobe.size() + live_table->m_vrenderprobe.size());
   for (size_t i = 0; i < m_vrenderprobe.size(); i++)
   {
      RenderProbe *rp = new RenderProbe();
      rp->SetRoughness(m_vrenderprobe[i]->GetRoughness());
      rp->SetType(m_vrenderprobe[i]->GetType());
      rp->SetName(m_vrenderprobe[i]->GetName());
      rp->SetReflectionMode(m_vrenderprobe[i]->GetReflectionMode());
      vec4 plane;
      m_vrenderprobe[i]->GetReflectionPlane(plane);
      rp->SetReflectionPlane(plane);
      rp->SetReflectionNoLightmaps(m_vrenderprobe[i]->GetReflectionNoLightmaps());
      live_table->m_vrenderprobe.push_back(rp);
      dst->m_startupToLive[m_vrenderprobe[i]] = rp;
      dst->m_liveToStartup[rp] = m_vrenderprobe[i];
   }

   // get the load path from the table filename
   const string szLoadDir = PathFromFilename(m_filename);
   // make sure the load directory is the active directory
   SetCurrentDirectory(szLoadDir.c_str());

   PLOGI << "Compiling script"; // For profiling

   live_table->m_pcv->m_scriptError = false;
   live_table->m_pcv->Compile(false);
   if (live_table->m_pcv->m_scriptError)
   {
      live_table->Release();
      return nullptr;
   }

   // set up the texture & material hashtables for faster access
   live_table->m_textureMap.clear();
   for (size_t i = 0; i < live_table->m_vimage.size(); i++)
      live_table->m_textureMap[live_table->m_vimage[i]->m_name] = live_table->m_vimage[i];
   live_table->m_materialMap.clear();
   for (size_t i = 0; i < live_table->m_materials.size(); i++)
      live_table->m_materialMap[live_table->m_materials[i]->m_name] = live_table->m_materials[i];
   live_table->m_lightMap.clear();
   for (size_t i = 0; i < live_table->m_vedit.size(); i++)
   {
      IEditable *const pe = live_table->m_vedit[i];
      if (pe->GetItemType() == ItemTypeEnum::eItemLight)
         live_table->m_lightMap[pe->GetName()] = (Light *)pe;
   }
   live_table->m_renderprobeMap.clear();
   for (size_t i = 0; i < live_table->m_vrenderprobe.size(); i++)
      live_table->m_renderprobeMap[live_table->m_vrenderprobe[i]->GetName()] = live_table->m_vrenderprobe[i];

   // parse the (optional) override-physics-sets that can be set globally
   if (live_table->m_overridePhysics)
   {
      live_table->m_fOverrideGravityConstant = GRAVITYCONST * m_settings.GetPlayer_TablePhysicsGravityConstant(live_table->m_overridePhysics - 1);
      live_table->m_fOverrideContactFriction = m_settings.GetPlayer_TablePhysicsContactFriction(live_table->m_overridePhysics - 1);
      live_table->m_fOverrideElasticity = m_settings.GetPlayer_TablePhysicsElasticity(live_table->m_overridePhysics - 1);
      live_table->m_fOverrideElasticityFalloff = m_settings.GetPlayer_TablePhysicsElasticityFalloff(live_table->m_overridePhysics - 1);
      live_table->m_fOverrideScatterAngle = m_settings.GetPlayer_TablePhysicsScatterAngle(live_table->m_overridePhysics - 1);
      live_table->m_fOverrideMinSlope = m_settings.GetPlayer_TablePhysicsMinSlope(live_table->m_overridePhysics - 1);
      live_table->m_fOverrideMaxSlope = m_settings.GetPlayer_TablePhysicsMaxSlope(live_table->m_overridePhysics - 1);
      const float fOverrideContactScatterAngle = m_settings.GetPlayer_TablePhysicsContactScatterAngle(live_table->m_overridePhysics - 1);
      c_hardScatter = ANGTORAD(live_table->m_overridePhysics ? fOverrideContactScatterAngle : live_table->m_defaultScatter);
   }

   return live_table;
}

HRESULT PinTable::TableSave()
{
   return Save(m_filename.empty());
}

HRESULT PinTable::SaveAs()
{
   return Save(true);
}

void PinTable::BeginAutoSaveCounter()
{
#ifndef __STANDALONE__
   if (m_vpinball->m_autosaveTime > 0)
       m_vpinball->SetTimer(VPinball::TIMER_ID_AUTOSAVE, m_vpinball->m_autosaveTime, nullptr);
#endif
}

void PinTable::EndAutoSaveCounter()
{
#ifndef __STANDALONE__
   m_vpinball->KillTimer(VPinball::TIMER_ID_AUTOSAVE);
#endif
}

void PinTable::AutoSave()
{
#ifndef __STANDALONE__
   if (m_sdsCurrentDirtyState <= eSaveAutosaved)
      return;

   m_vpinball->KillTimer(VPinball::TIMER_ID_AUTOSAVE);

   m_vpinball->SetActionCur(LocalString(IDS_AUTOSAVING).m_szbuffer);
   m_vpinball->SetCursorCur(nullptr, IDC_WAIT);

   FastIStorage * const pstgroot = new FastIStorage();
   pstgroot->AddRef();

   const HRESULT hr = SaveToStorage(pstgroot);

   m_undo.SetCleanPoint((SaveDirtyState)min((int)m_sdsDirtyProp, (int)eSaveAutosaved));
   m_pcv->SetClean((SaveDirtyState)min((int)m_sdsDirtyScript, (int)eSaveAutosaved));
   SetNonUndoableDirty((SaveDirtyState)min((int)m_sdsNonUndoableDirty, (int)eSaveAutosaved));

   AutoSavePackage * const pasp = new AutoSavePackage();
   pasp->pstg = pstgroot;
   pasp->tableindex = FindIndexOf(m_vpinball->m_vtable, (CComObject<PinTable> *)this);
   pasp->hwndtable = GetHwnd();

   if (hr == S_OK)
   {
      const HANDLE hEvent = m_vpinball->PostWorkToWorkerThread(COMPLETE_AUTOSAVE, (LPARAM)pasp);
      m_vAsyncHandles.push_back(hEvent);

      m_vpinball->SetActionCur("Completing AutoSave"s);
   }
   else
   {
      m_vpinball->SetActionCur(string());
   }

   m_vpinball->SetCursorCur(nullptr, IDC_ARROW);
#endif
}

HRESULT PinTable::Save(const bool saveAs)
{
#ifndef __STANDALONE__
   IStorage* pstgRoot;

   // Get file name if needed
   if (saveAs)
   {
      //need to get a file name
      OPENFILENAME ofn = {};
      ofn.lStructSize = sizeof(OPENFILENAME);
      ofn.hInstance = m_vpinball->theInstance;
      ofn.hwndOwner = m_vpinball->GetHwnd();
      // TEXT
      ofn.lpstrFilter = "Visual Pinball Tables (*.vpx)\0*.vpx\0";

      const size_t ptr = StrFindNoCase(m_filename, ".vpt"s);
      string filename_new = m_filename;
      if (ptr != string::npos)
         filename_new.replace(ptr+1, 3, "vpx");
      char fileName[MAXSTRING];
      strncpy_s(fileName, sizeof(fileName), filename_new.c_str());
      ofn.lpstrFile = fileName;
      ofn.nMaxFile = sizeof(fileName);
      ofn.lpstrDefExt = "vpx";
      ofn.Flags = OFN_NOREADONLYRETURN | OFN_CREATEPROMPT | OFN_OVERWRITEPROMPT | OFN_EXPLORER;

      {
      string szInitialDir;
      // First, use dir of current table
      const size_t index = m_filename.find_last_of(PATH_SEPARATOR_CHAR);
      if (index != string::npos)
         szInitialDir = m_filename.substr(0, index);
      // Or try with the standard last-used dir
      else
      {
         Settings::SetRecentDir_LoadDir_Default(m_vpinball->GetAppPath(VPinball::AppSubFolder::Tables).string() + PATH_SEPARATOR_CHAR);
         szInitialDir = m_settings.GetRecentDir_LoadDir();
      }
      ofn.lpstrInitialDir = szInitialDir.c_str();

      const int ret = GetSaveFileName(&ofn);
      // user cancelled
      if (ret == 0)
         return S_FALSE;
      }

      // assign user selected file name as new internal filename, and save as new default
      m_filename = fileName;
      g_pvp->m_settings.SetRecentDir_LoadDir(m_filename.substr(0, ofn.nFileOffset), false); // truncate after folder(s)

      {
         STGOPTIONS stg;
         stg.usVersion = 1;
         stg.reserved = 0;
         stg.ulSectorSize = 4096;

         HRESULT hr;
         if (FAILED(hr = StgCreateStorageEx(MakeWString(m_filename).c_str(), STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE,
            STGFMT_DOCFILE, 0, &stg, nullptr, IID_IStorage, (void**)&pstgRoot)))
         {
            m_mdiTable->MessageBox(LocalString(IDS_SAVEERROR).m_szbuffer, "Visual Pinball", MB_ICONERROR);
            return hr;
         }
      }

      m_title = TitleFromFilename(m_filename);
      SetCaption(m_title);
   }
   else
   {
      const size_t ptr = StrFindNoCase(m_filename, ".vpt"s);
      if (ptr != string::npos)
         m_filename.replace(ptr+1, 3, "vpx");

      STGOPTIONS stg;
      stg.usVersion = 1;
      stg.reserved = 0;
      stg.ulSectorSize = 4096;

      HRESULT hr;
      if (FAILED(hr = StgCreateStorageEx(MakeWString(m_filename).c_str(), STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE,
         STGFMT_DOCFILE, 0, &stg, nullptr, IID_IStorage, (void**)&pstgRoot)))
      {
         m_mdiTable->MessageBox(LocalString(IDS_SAVEERROR).m_szbuffer, "Visual Pinball", MB_ICONERROR);
         return hr;
      }
   }

   m_vpinball->SetActionCur(LocalString(IDS_SAVING).m_szbuffer);
   m_vpinball->SetCursorCur(nullptr, IDC_WAIT);

   RemoveInvalidReferences();

   const HRESULT hr = SaveToStorage(pstgRoot);

   if (SUCCEEDED(hr))
   {
      pstgRoot->Commit(STGC_DEFAULT);
      pstgRoot->Release();

      m_vpinball->SetActionCur(string());
      m_vpinball->SetCursorCur(nullptr, IDC_ARROW);

      m_undo.SetCleanPoint(eSaveClean);
      m_pcv->SetClean(eSaveClean);
      SetNonUndoableDirty(eSaveClean);
   }
#endif

   // Save user custom settings file (if any) along the table file
   // Force saving as we may have upgraded the table version (from pre 10.8 to 10.8) or changed the file path
   m_settings.SetModified(true);
   m_settings.SetIniPath(GetSettingsFileName());
   m_settings.Save();

   return S_OK;
}

HRESULT PinTable::SaveToStorage(IStorage *pstgRoot)
{
#ifndef __STANDALONE__
   VPXSaveFileProgressBar feedback(m_vpinball->theInstance, m_vpinball->m_hwndStatusBar, m_mdiTable);
#else
   VPXFileFeedback feedback;
#endif

   return SaveToStorage(pstgRoot, feedback);
}

HRESULT PinTable::SaveToStorage(IStorage *pstgRoot, VPXFileFeedback& feedback)
{
#ifndef __STANDALONE__
   m_savingActive = true;
   feedback.OperationStarted();

   // Hashing (to ensure file integrity)
   HCRYPTPROV hcp = NULL; // context
   HCRYPTHASH hch = NULL; // hash

   int foo = CryptAcquireContext(&hcp, nullptr, nullptr, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_NEWKEYSET/* | CRYPT_SILENT*/);
   foo = GetLastError();
   foo = CryptCreateHash(hcp, CALG_MD2, NULL, 0, &hch);
   foo = GetLastError();
   foo = CryptHashData(hch, (BYTE *)TABLE_KEY, 14, 0);
   foo = GetLastError();

   //

   const int ctotalitems = (int)(m_vedit.size() + m_vsound.size() + m_vimage.size() + m_vfont.size() + m_vcollection.size());
   int csaveditems = 0;

   feedback.AboutToProcessTable(ctotalitems);

   //first save our own data
   IStorage* pstgData;
   HRESULT hr;
   if (SUCCEEDED(hr = pstgRoot->CreateStorage(L"GameStg", STGM_DIRECT/*STGM_TRANSACTED*/ | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstgData)))
   {
      IStream *pstmGame;
      if (SUCCEEDED(hr = pstgData->CreateStream(L"GameData", STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstmGame)))
      {
         IStream *pstmItem;
         if (SUCCEEDED(hr = pstgData->CreateStream(L"Version", STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstmItem)))
         {
            int version = CURRENT_FILE_FORMAT_VERSION;
            CryptHashData(hch, (BYTE *)&version, sizeof(version), 0);
            ULONG writ;
            pstmItem->Write(&version, sizeof(version), &writ);
            pstmItem->Release();
            pstmItem = nullptr;
         }

         IStorage *pstgInfo;
         if (SUCCEEDED(hr = pstgRoot->CreateStorage(L"TableInfo", STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstgInfo)))
         {
            SaveInfo(pstgInfo, hch);

            if (SUCCEEDED(hr = pstgData->CreateStream(L"CustomInfoTags", STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstmItem)))
            {
               SaveCustomInfo(pstgInfo, pstmItem, hch);
               pstmItem->Release();
               pstmItem = nullptr;
            }

            pstgInfo->Release();
         }

         if (SUCCEEDED(hr = SaveData(pstmGame, hch, false)))
         {
            // Move PartGroup ahead of objects they contain, so that they are saved first
            std::stable_partition(m_vedit.begin(), m_vedit.end(), [](IEditable *p) { return p->GetItemType() == ItemTypeEnum::eItemPartGroup; });
            for (size_t i = 0; i < m_vedit.size(); i++)
            {
               const wstring wStmName = L"GameItem" + std::to_wstring(i);

               if (SUCCEEDED(hr = pstgData->CreateStream(wStmName.c_str(), STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstmItem)))
               {
                  ULONG writ;
                  IEditable *const piedit = m_vedit[i];
                  const ItemTypeEnum type = piedit->GetItemType();
                  pstmItem->Write(&type, sizeof(int), &writ);
                  hr = piedit->SaveData(pstmItem, NULL, false);
                  pstmItem->Release();
                  pstmItem = nullptr;
                  //if (FAILED(hr)) goto Error;
               }

               csaveditems++;
               feedback.ItemHasBeenProcessed((int)i + 1, (int)m_vedit.size());
            }

            for (size_t i = 0; i < m_vsound.size(); i++)
            {
               const wstring wStmName = L"Sound" + std::to_wstring(i);

               if (SUCCEEDED(hr = pstgData->CreateStream(wStmName.c_str(), STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstmItem)))
               {
                  m_vsound[i]->SaveToStream(pstmItem);
                  pstmItem->Release();
                  pstmItem = nullptr;
               }

               csaveditems++;
               feedback.SoundHasBeenProcessed((int)i + 1, (int)m_vsound.size());
            }

            for (size_t i = 0; i < m_vimage.size(); i++)
            {
               const wstring wStmName = L"Image" + std::to_wstring(i);

               if (SUCCEEDED(hr = pstgData->CreateStream(wStmName.c_str(), STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstmItem)))
               {
                  m_vimage[i]->SaveToStream(pstmItem, this);
                  pstmItem->Release();
                  pstmItem = nullptr;
               }

               csaveditems++;
               feedback.ImageHasBeenProcessed((int)i + 1, (int)m_vimage.size());
            }

            for (size_t i = 0; i < m_vfont.size(); i++)
            {
               const wstring wStmName = L"Font" + std::to_wstring(i);

               if (SUCCEEDED(hr = pstgData->CreateStream(wStmName.c_str(), STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstmItem)))
               {
                  m_vfont[i]->SaveToStream(pstmItem);
                  pstmItem->Release();
                  pstmItem = nullptr;
               }

               csaveditems++;
               feedback.FontHasBeenProcessed((int)i + 1, (int)m_vfont.size());
            }

            for (int i = 0; i < m_vcollection.size(); i++)
            {
               const wstring wStmName = L"Collection" + std::to_wstring(i);

               if (SUCCEEDED(hr = pstgData->CreateStream(wStmName.c_str(), STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstmItem)))
               {
                  m_vcollection[i].SaveData(pstmItem, hch, false);
                  pstmItem->Release();
                  pstmItem = nullptr;
               }

               csaveditems++;
               feedback.ItemHasBeenProcessed(i + 1, (int)m_vfont.size());
            }

         }
         pstmGame->Release();
      }

      feedback.Finalizing();

      // Authentication block
      BYTE hashval[256];
      DWORD hashlen = 256;
      foo = CryptGetHashParam(hch, HP_HASHSIZE, hashval, &hashlen, 0);
      hashlen = 256;
      foo = CryptGetHashParam(hch, HP_HASHVAL, hashval, &hashlen, 0);

      IStream* pstmItem;
      if (SUCCEEDED(hr = pstgData->CreateStream(L"MAC", STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstmItem)))
      {
         ULONG writ;
         //int version = CURRENT_FILE_FORMAT_VERSION;
         pstmItem->Write(hashval, hashlen, &writ);
         pstmItem->Release();
         pstmItem = nullptr;
      }

      foo = CryptDestroyHash(hch);
      foo = CryptReleaseContext(hcp, 0);
      // End Authentication block

      if (SUCCEEDED(hr))
         pstgData->Commit(STGC_DEFAULT);
      else
      {
         pstgData->Revert();
         pstgRoot->Revert();
         feedback.ErrorOccured(LocalString(IDS_SAVEERROR).m_szbuffer);
      }
      pstgData->Release();
   }

   feedback.Done();
   m_savingActive = false;

   return hr;
#else
   return 0L;
#endif
}

HRESULT PinTable::WriteInfoValue(IStorage* pstg, const wstring& wzName, const string& szValue, HCRYPTHASH hcrypthash)
{
#ifndef __STANDALONE__
   if (szValue.empty())
      return S_OK;

   IStream *pstm;
   HRESULT hr = pstg->CreateStream(wzName.c_str(), STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstm);
   if (FAILED(hr))
      return hr;

   ULONG writ;
   BiffWriter bw(pstm, hcrypthash);
   const wstring wzT = MakeWString(szValue);
   bw.WriteBytes(wzT.c_str(), static_cast<ULONG>(wzT.length() * sizeof(WCHAR)), &writ);
   pstm->Release();
   pstm = nullptr;
   return S_OK;
#else
   return 0L;
#endif
}


HRESULT PinTable::SaveInfo(IStorage* pstg, HCRYPTHASH hcrypthash)
{
#ifndef __STANDALONE__
   WriteInfoValue(pstg, L"TableName"s, m_tableName, hcrypthash);
   WriteInfoValue(pstg, L"AuthorName"s, m_author, hcrypthash);
   WriteInfoValue(pstg, L"TableVersion"s, m_version, hcrypthash);
   WriteInfoValue(pstg, L"ReleaseDate"s, m_releaseDate, hcrypthash);
   WriteInfoValue(pstg, L"AuthorEmail"s, m_authorEMail, hcrypthash);
   WriteInfoValue(pstg, L"AuthorWebSite"s, m_webSite, hcrypthash);
   WriteInfoValue(pstg, L"TableBlurb"s, m_blurb, hcrypthash);
   WriteInfoValue(pstg, L"TableDescription"s, m_description, hcrypthash);
   WriteInfoValue(pstg, L"TableRules"s, m_rules, hcrypthash);
   time_t hour_machine;
   time(&hour_machine);
   tm local_hour;
   localtime_s(&local_hour, &hour_machine);
   char buffer[256];
   asctime_s(buffer, &local_hour);
   buffer[strnlen_s(buffer,sizeof(buffer))-1] = '\0'; // remove line break
   WriteInfoValue(pstg, L"TableSaveDate"s, buffer, NULL);
   _itoa_s(++m_numTimesSaved, buffer, 10);
   WriteInfoValue(pstg, L"TableSaveRev"s, buffer, NULL);

   Texture * const pin = GetImage(m_screenShot);
   if (pin)
   {
      IStream *pstm;
      HRESULT hr;

      if (SUCCEEDED(hr = pstg->CreateStream(L"Screenshot", STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstm)))
      {
         BiffWriter bw(pstm, hcrypthash);
         ULONG writ;
         bw.WriteBytes(pin->GetFileRaw(), static_cast<ULONG>(pin->GetFileSize()), &writ);
         pstm->Release();
         pstm = nullptr;
      }
   }

   pstg->Commit(STGC_DEFAULT);
#endif

   return S_OK;
}


HRESULT PinTable::SaveCustomInfo(IStorage* pstg, IStream *pstmTags, HCRYPTHASH hcrypthash)
{
#ifndef __STANDALONE__
   BiffWriter bw(pstmTags, hcrypthash);

   for (size_t i = 0; i < m_vCustomInfoTag.size(); i++)
      bw.WriteString(FID(CUST), m_vCustomInfoTag[i]);

   bw.WriteTag(FID(ENDB));

   for (size_t i = 0; i < m_vCustomInfoTag.size(); i++)
   {
      const wstring wzName = MakeWString(m_vCustomInfoTag[i]);
      WriteInfoValue(pstg, wzName, m_vCustomInfoContent[i], hcrypthash);
   }

   pstg->Commit(STGC_DEFAULT);
#endif

   return S_OK;
}


HRESULT PinTable::ReadInfoValue(IStorage* pstg, const wstring& wzName, string &output, HCRYPTHASH hcrypthash)
{
   HRESULT hr;
   IStream *pstm;

   if (SUCCEEDED(hr = pstg->OpenStream(wzName.c_str(), nullptr, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstm)))
   {
      STATSTG ss;
      pstm->Stat(&ss, STATFLAG_NONAME);

#ifndef __STANDALONE__
      const int len = ss.cbSize.LowPart / (DWORD)sizeof(WCHAR);
#else
      const int len = ss.cbSize.LowPart / 2;
#endif
      WCHAR * const wzT = new WCHAR[len + 1];
      memset(wzT, 0, sizeof(WCHAR) * (len + 1));

      BiffReader br(pstm, nullptr, 0, hcrypthash, NULL);
#ifndef __STANDALONE__
      br.ReadBytes(wzT, ss.cbSize.LowPart);
#else
      char* ptr = (char*)wzT;
      for (int index = 0; index < len; index++) {
         br.ReadBytes(ptr, 2);
         ptr += sizeof(WCHAR);
      }
#endif
      wzT[len] = L'\0';
      output = MakeString(wzT);
      delete[] wzT;

      pstm->Release();
   }

   return hr;
}


HRESULT PinTable::LoadInfo(IStorage* pstg, HCRYPTHASH hcrypthash, int version)
{
   ReadInfoValue(pstg, L"TableName"s, m_tableName, hcrypthash);
   ReadInfoValue(pstg, L"AuthorName"s, m_author, hcrypthash);
   ReadInfoValue(pstg, L"TableVersion"s, m_version, hcrypthash);
   ReadInfoValue(pstg, L"ReleaseDate"s, m_releaseDate, hcrypthash);
   ReadInfoValue(pstg, L"AuthorEmail"s, m_authorEMail, hcrypthash);
   ReadInfoValue(pstg, L"AuthorWebSite"s, m_webSite, hcrypthash);
   ReadInfoValue(pstg, L"TableBlurb"s, m_blurb, hcrypthash);
   ReadInfoValue(pstg, L"TableDescription"s, m_description, hcrypthash);
   ReadInfoValue(pstg, L"TableRules"s, m_rules, hcrypthash);
   ReadInfoValue(pstg, L"TableSaveDate"s, m_dateSaved, NULL);

   string numTimesSaved;
   ReadInfoValue(pstg, L"TableSaveRev"s, numTimesSaved, NULL);
   m_numTimesSaved = 0;
   if (!numTimesSaved.empty())
      std::from_chars(numTimesSaved.c_str(), numTimesSaved.c_str() + numTimesSaved.length(), m_numTimesSaved);

   // Write the version to the registry.  This will be read later by the front end.
   {
      string optId = trim_string(m_tableName);
      std::replace_if(optId.begin(), optId.end(), [](char c) { return !isalnum(c) || c == '.' || c == '-'; }, '_');
      const auto propId
         = Settings::GetRegistry().Register(std::make_unique<VPX::Properties::StringPropertyDef>("Version"s, optId, "Table Version"s, "Last played version"s, false, m_version));
      g_pvp->m_settings.Set(propId, m_version, false);
   }

   HRESULT hr;
   IStream *pstm;

   if (SUCCEEDED(hr = pstg->OpenStream(L"Screenshot", nullptr, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstm)))
   {
      STATSTG ss;
      pstm->Stat(&ss, STATFLAG_NONAME);
      m_pbTempScreenshot = new PinBinary();

      m_pbTempScreenshot->m_buffer.resize(ss.cbSize.LowPart);

      BiffReader br(pstm, nullptr, 0, hcrypthash, NULL);
      br.ReadBytes(m_pbTempScreenshot->m_buffer.data(), static_cast<uint32_t>(m_pbTempScreenshot->m_buffer.size()));

      pstm->Release();
   }

   return hr;
}

HRESULT PinTable::LoadCustomInfo(IStorage* pstg, IStream *pstmTags, HCRYPTHASH hcrypthash, int version)
{
   BiffReader br(pstmTags, this, version, hcrypthash, NULL);
   const HRESULT hr = br.Load();

   for (size_t i = 0; i < m_vCustomInfoTag.size(); i++)
   {
      const wstring wzName = MakeWString(m_vCustomInfoTag[i]);

      string customInfo;
      ReadInfoValue(pstg, wzName, customInfo, hcrypthash);
      m_vCustomInfoContent.push_back(customInfo);
   }

   return hr;
}

HRESULT PinTable::SaveData(IStream* pstm, HCRYPTHASH hcrypthash, const bool saveForUndo)
{
   BiffWriter bw(pstm, hcrypthash);

   bw.WriteFloat(FID(LEFT), m_left);
   bw.WriteFloat(FID(TOPX), m_top);
   bw.WriteFloat(FID(RGHT), m_right);
   bw.WriteFloat(FID(BOTM), m_bottom);

   bw.WriteBool(FID(EFSS), m_isFSSViewModeEnabled);
   static constexpr int vsFields[NUM_BG_SETS][19] = { 
      { FID(VSM0), FID(ROTA), FID(INCL), FID(LAYB), FID(FOVX), FID(XLTX), FID(XLTY), FID(XLTZ), FID(SCLX), FID(SCLY), FID(SCLZ), FID(HOF0), FID(VOF0), FID(WTX0), FID(WTY0), FID(WTZ0), FID(WBX0), FID(WBY0), FID(WBZ0) },
      { FID(VSM1), FID(ROTF), FID(INCF), FID(LAYF), FID(FOVF), FID(XLFX), FID(XLFY), FID(XLFZ), FID(SCFX), FID(SCFY), FID(SCFZ), FID(HOF1), FID(VOF1), FID(WTX1), FID(WTY1), FID(WTZ1), FID(WBX1), FID(WBY1), FID(WBZ1) },
      { FID(VSM2), FID(ROFS), FID(INFS), FID(LAFS), FID(FOFS), FID(XLXS), FID(XLYS), FID(XLZS), FID(SCXS), FID(SCYS), FID(SCZS), FID(HOF2), FID(VOF2), FID(WTX2), FID(WTY2), FID(WTZ2), FID(WBX2), FID(WBY2), FID(WBZ2) },
   };
   for (int i = 0; i < 3; i++)
   {
      bw.WriteInt(vsFields[i][0], mViewSetups[i].mMode);
      bw.WriteFloat(vsFields[i][1], mViewSetups[i].mViewportRotation);
      bw.WriteFloat(vsFields[i][2], mViewSetups[i].mLookAt);
      bw.WriteFloat(vsFields[i][3], mViewSetups[i].mLayback);
      bw.WriteFloat(vsFields[i][4], mViewSetups[i].mFOV);
      bw.WriteFloat(vsFields[i][5], mViewSetups[i].mViewX);
      bw.WriteFloat(vsFields[i][6], mViewSetups[i].mViewY);
      bw.WriteFloat(vsFields[i][7], mViewSetups[i].mViewZ);
      bw.WriteFloat(vsFields[i][8], mViewSetups[i].mSceneScaleX);
      bw.WriteFloat(vsFields[i][9], mViewSetups[i].mSceneScaleY);
      bw.WriteFloat(vsFields[i][10], mViewSetups[i].mSceneScaleZ);
      bw.WriteFloat(vsFields[i][11], mViewSetups[i].mViewHOfs);
      bw.WriteFloat(vsFields[i][12], mViewSetups[i].mViewVOfs);
      bw.WriteFloat(vsFields[i][15], mViewSetups[i].mWindowTopZOfs);
      bw.WriteFloat(vsFields[i][18], mViewSetups[i].mWindowBottomZOfs);
   }

   bw.WriteInt(FID(ORRP), m_overridePhysics);
   bw.WriteBool(FID(ORPF), m_overridePhysicsFlipper);
   bw.WriteFloat(FID(GAVT), m_Gravity);
   bw.WriteFloat(FID(FRCT), m_friction);
   bw.WriteFloat(FID(ELAS), m_elasticity);
   bw.WriteFloat(FID(ELFA), m_elasticityFalloff);
   bw.WriteFloat(FID(PFSC), m_scatter);
   bw.WriteFloat(FID(SCAT), m_defaultScatter);
   bw.WriteFloat(FID(NDGT), m_nudgeTime);
   bw.WriteInt(FID(PHML), m_PhysicsMaxLoops);

   //bw.WriteFloat(FID(IMTCOL), m_transcolor);

   bw.WriteBool(FID(REEL), m_renderEMReels);
   bw.WriteBool(FID(DECL), m_renderDecals);

   bw.WriteFloat(FID(OFFX), m_offset.x);
   bw.WriteFloat(FID(OFFY), m_offset.y);

   bw.WriteFloat(FID(ZOOM), m_zoom);

   bw.WriteFloat(FID(SLPX), m_angletiltMax);
   bw.WriteFloat(FID(SLOP), m_angletiltMin);

   bw.WriteString(FID(IMAG), m_image);
   bw.WriteString(FID(BIMG), m_BG_image[0]);
   bw.WriteString(FID(BIMF), m_BG_image[1]);
   bw.WriteString(FID(BIMS), m_BG_image[2]);
   bw.WriteBool(FID(BIMN), m_ImageBackdropNightDay);
   bw.WriteString(FID(IMCG), m_imageColorGrade);
   bw.WriteString(FID(BLIM), m_ballImage);
   bw.WriteBool(FID(BLSM), m_ballSphericalMapping);
   bw.WriteString(FID(BLIF), m_ballImageDecal);
   bw.WriteString(FID(EIMG), m_envImage);
   bw.WriteString(FID(NOTX), m_notesText);

   bw.WriteString(FID(SSHT), m_screenShot);

   bw.WriteBool(FID(FBCK), m_backdrop);

   bw.WriteFloat(FID(GLAS), m_glassTopHeight);
   bw.WriteFloat(FID(GLAB), m_glassBottomHeight);

   bw.WriteString(FID(PLMA), m_playfieldMaterial);
   bw.WriteInt(FID(BCLR), m_colorbackdrop);

   bw.WriteFloat(FID(TDFT), m_difficulty);

   bw.WriteInt(FID(LZAM), m_lightAmbient);
   bw.WriteInt(FID(LZDI), m_Light[0].emission);
   bw.WriteFloat(FID(LZHI), m_lightHeight);
   bw.WriteFloat(FID(LZRA), m_lightRange);
   bw.WriteFloat(FID(LIES), m_lightEmissionScale);
   bw.WriteFloat(FID(ENES), m_envEmissionScale);
   bw.WriteFloat(FID(GLES), m_globalEmissionScale);
   bw.WriteFloat(FID(AOSC), m_AOScale);
   bw.WriteFloat(FID(SSSC), m_SSRScale);
   bw.WriteFloat(FID(CLBH), m_groundToLockbarHeight);

   bw.WriteFloat(FID(SVOL), m_TableSoundVolume);
   bw.WriteFloat(FID(MVOL), m_TableMusicVolume);

   bw.WriteInt(FID(PLST), quantizeUnsigned<8>(m_playfieldReflectionStrength));
   bw.WriteBool(FID(BDMO), m_BallDecalMode);
   bw.WriteFloat(FID(BPRS), m_ballPlayfieldReflectionStrength);
   bw.WriteFloat(FID(DBIS), m_defaultBulbIntensityScaleOnBall);
   bw.WriteBool(FID(GDAC), m_grid);

   bw.WriteInt(FID(UAOC), m_enableAO);
   bw.WriteInt(FID(USSR), m_enableSSR);
   bw.WriteInt(FID(TMAP), m_toneMapper);
   bw.WriteFloat(FID(EXPO), m_exposure);
   bw.WriteFloat(FID(BLST), m_bloom_strength);

   // Legacy material saving for backward compatibility
   bw.WriteInt(FID(MASI), (int)m_materials.size());
   if (!m_materials.empty())
   {
      vector<SaveMaterial> mats(m_materials.size());
      for (size_t i = 0; i < m_materials.size(); i++)
      {
         const Material* const m = m_materials[i];
         mats[i].cBase = m->m_cBase;
         mats[i].cGlossy = m->m_cGlossy;
         mats[i].cClearcoat = m->m_cClearcoat;
         mats[i].fWrapLighting = m->m_fWrapLighting;
         mats[i].fRoughness = m->m_fRoughness;
         mats[i].fGlossyImageLerp = 255 - quantizeUnsigned<8>(clamp(m->m_fGlossyImageLerp, 0.f, 1.f)); // '255 -' to be compatible with previous table versions
         mats[i].fThickness = quantizeUnsigned<8>(clamp(m->m_fThickness, 0.05f, 1.f)); // clamp with 0.05f to be compatible with previous table versions
         mats[i].fEdge = m->m_fEdge;
         mats[i].fOpacity = m->m_fOpacity;
         mats[i].bIsMetal = m->m_type == Material::MaterialType::METAL;
         mats[i].bOpacityActive_fEdgeAlpha = m->m_bOpacityActive ? 1 : 0;
         mats[i].bOpacityActive_fEdgeAlpha |= quantizeUnsigned<7>(clamp(m->m_fEdgeAlpha, 0.f, 1.f)) << 1;
         strncpy_s(mats[i].szName, sizeof(mats[i].szName), m->m_name.c_str());
         for (size_t c = strnlen_s(mats[i].szName, sizeof(mats[i].szName)); c < sizeof(mats[i].szName); ++c) // to avoid garbage after 0
             mats[i].szName[c] = 0;
      }
      bw.WriteStruct(FID(MATE), mats.data(), (int)(sizeof(SaveMaterial)*m_materials.size()));

      vector<SavePhysicsMaterial> phymats(m_materials.size());
      for (size_t i = 0; i < m_materials.size(); i++)
      {
          const Material* const m = m_materials[i];
          strncpy_s(phymats[i].szName, sizeof(phymats[i].szName), m->m_name.c_str());
          for (size_t c = strnlen_s(phymats[i].szName, sizeof(phymats[i].szName)); c < sizeof(phymats[i].szName); ++c) // to avoid garbage after 0
              phymats[i].szName[c] = '\0';
          phymats[i].fElasticity = m->m_fElasticity;
          phymats[i].fElasticityFallOff = m->m_fElasticityFalloff;
          phymats[i].fFriction = m->m_fFriction;
          phymats[i].fScatterAngle = m->m_fScatterAngle;
      }
      bw.WriteStruct(FID(PHMA), phymats.data(), (int)(sizeof(SavePhysicsMaterial)*m_materials.size()));
   }
   // 10.8+ material saving (this format supports new properties, can be extended in future versions, and does not perform quantizations)
   for (size_t i = 0; i < m_materials.size(); i++)
   {
      const size_t record_size = m_materials[i]->GetSaveSize();
      HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE, record_size);
      CComPtr<IStream> spStream;
      const HRESULT hr = ::CreateStreamOnHGlobal(hMem, FALSE, &spStream);
      m_materials[i]->SaveData(spStream, NULL, false);
      LPVOID pData = ::GlobalLock(hMem);
      bw.WriteStruct(FID(MATR), pData, (int)record_size);
      ::GlobalUnlock(hMem);
   }

   for (size_t i = 0; i < m_vrenderprobe.size(); i++)
   {
      // Save each render probe as a data blob inside the main gamedata.
      // This allows backward compatibility since the block will be blindly discarded on older versions, still hashing it.
      const int record_size = m_vrenderprobe[i]->GetSaveSize();
      HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE, record_size);
      CComPtr<IStream> spStream;
      const HRESULT hr = ::CreateStreamOnHGlobal(hMem, FALSE, &spStream);
      m_vrenderprobe[i]->SaveData(spStream, NULL, false);
      LPVOID pData = ::GlobalLock(hMem);
      bw.WriteStruct(FID(RPRB), pData, record_size);
      ::GlobalUnlock(hMem);
   }

   // HACK!!!! - Don't save special values when copying for undo.  For instance, don't reset the code.
   // Someday save these values into their own stream, used only when saving to file.

   if (hcrypthash != 0)
   {
      bw.WriteInt(FID(SEDT), (int)m_vedit.size());
      bw.WriteInt(FID(SSND), (int)m_vsound.size());
      bw.WriteInt(FID(SIMG), (int)m_vimage.size());
      bw.WriteInt(FID(SFNT), (int)m_vfont.size());
      bw.WriteInt(FID(SCOL), m_vcollection.size());

      bw.WriteWideString(FID(NAME), m_wzName);

      bw.WriteStruct(FID(CCUS), m_rgcolorcustom, sizeof(COLORREF) * 16);

      // save the script source code, incl. the computed hash to be able to check for file integrity on loading
      bw.WriteTag(FID(CODE));
      m_pcv->SaveToStream(pstm, hcrypthash);
   }

   bw.WriteInt(FID(TLCK), m_tablelocked);
   bw.WriteTag(FID(ENDB));

   return S_OK;
}

HRESULT PinTable::LoadGameFromFilename(const string& filename)
{
#ifndef __STANDALONE__
   VPXLoadFileProgressBar feedback(m_vpinball->theInstance, m_vpinball->m_hwndStatusBar);
#else
   VPXFileFeedback feedback;
#endif

   return LoadGameFromFilename(filename, feedback);
}

HRESULT PinTable::LoadGameFromFilename(const string& filename, VPXFileFeedback& feedback)
{
   if (filename.empty())
   {
      ShowError("Empty File Name String!");
      return S_FALSE;
   }

   PLOGI << "LoadGameFromFilename " + filename; // For profiling

   m_filename = filename;

   // Load user custom settings before actually loading the table for settings applying during load
   m_settings.SetIniPath(GetSettingsFileName());
   m_settings.Load(false);

   HRESULT hr;
   IStorage* pstgRoot;
   if (FAILED(hr = StgOpenStorage(MakeWString(m_filename).c_str(), nullptr, STGM_TRANSACTED | STGM_READ, nullptr, 0, &pstgRoot)))
   {
      char msg[MAXSTRING+32];
      sprintf_s(msg, sizeof(msg), "Error 0x%X loading \"%s\"", hr, m_filename.c_str());
      m_vpinball->MessageBox(msg, "Load Error", 0);
      return hr;
   }

   feedback.OperationStarted();

   //

   HCRYPTPROV hcp = NULL; // crypt context
   HCRYPTHASH hch = NULL; // hash for file integrity check
   HCRYPTHASH hchkey = NULL; // hash for decryption key derivation

   #ifndef __STANDALONE__
   // Hashing (to ensure file integrity), can be disabled for slightly faster loading (and then also matches standalone which cannot feature this)
   const bool hashValidation = !g_pvp->m_settings.GetEditor_DisableHash();
   int foo = CryptAcquireContext(&hcp, nullptr, nullptr, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_NEWKEYSET /* | CRYPT_SILENT*/);
   foo = GetLastError();
   if (hashValidation)
   {
      foo = CryptCreateHash(hcp, CALG_MD2, NULL, 0, &hch);
      foo = GetLastError();
      foo = CryptHashData(hch, (BYTE *)TABLE_KEY, 14, 0);
      foo = GetLastError();
   }
   // Decryption, for unlocking old VP8/VP9 tables that featured password protection (and that had script encryption set);
   // Create a key hash (we have to use a second hash as deriving a key from the
   // integrity hash actually modifies it, and thus it calculates the wrong hash)
   foo = CryptCreateHash(hcp, CALG_MD5, NULL, 0, &hchkey);
   foo = GetLastError();
   // Hash the password
   foo = CryptHashData(hchkey, (BYTE *)TABLE_KEY, 14, 0);
   foo = GetLastError();
   // Create a block cipher session key based on the hash of the password.
   // We need to figure out the file version before we can create the key
   #endif

   int loadfileversion = CURRENT_FILE_FORMAT_VERSION;

   //load our stuff first
   IStorage* pstgData;
   if (SUCCEEDED(hr = pstgRoot->OpenStorage(L"GameStg", nullptr, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, nullptr, 0, &pstgData)))
   {
      IStream *pstmGame;
      if (SUCCEEDED(hr = pstgData->OpenStream(L"GameData", nullptr, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmGame)))
      {
         HCRYPTKEY hkey = NULL;
         IStream *pstmVersion;
         if (SUCCEEDED(hr = pstgData->OpenStream(L"Version", nullptr, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmVersion)))
         {
            ULONG read;
            hr = pstmVersion->Read(&loadfileversion, sizeof(int), &read);
            #ifndef __STANDALONE__
               if (hch)
                  CryptHashData(hch, (BYTE *)&loadfileversion, sizeof(int), 0);
            #endif
            pstmVersion->Release();
            if (loadfileversion < 100) // Tech Beta 3 and below
            {
               pstmGame->Release();
               pstgData->Release();
               ShowError("Tables from Tech Beta 3 and below are not supported in this version.");
               feedback.Done();
               return E_FAIL;
            }
            if (loadfileversion > CURRENT_FILE_FORMAT_VERSION)
            {
               char errorMsg[256];
               errorMsg[0] = '\0';
               sprintf_s(errorMsg, sizeof(errorMsg), "This table was saved with file version %i.%02i and is newer than the supported file version %i.%02i!\nYou might get problems loading/playing it, so please update to the latest VPX at https://github.com/vpinball/vpinball/releases!", loadfileversion / 100, loadfileversion % 100, CURRENT_FILE_FORMAT_VERSION / 100, CURRENT_FILE_FORMAT_VERSION % 100);
               ShowError(errorMsg);
               /*
                              pstgRoot->Release();
                              pstmGame->Release();
                              pstgData->Release();
                              DestroyWindow(hwndProgressBar);
                              m_vpinball->SetCursorCur(nullptr, IDC_ARROW);
                              return -1;
               */
            }

            #ifndef __STANDALONE__
               // Create a block cipher session key based on the hash of the password.
               if (hchkey)
                  CryptDeriveKey(hcp, CALG_RC2, hchkey, (loadfileversion == 600) ? CRYPT_EXPORTABLE : (CRYPT_EXPORTABLE | 0x00280000), &hkey);
            #endif
         }

         IStorage* pstgInfo;
         if (SUCCEEDED(hr = pstgRoot->OpenStorage(L"TableInfo", nullptr, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, nullptr, 0, &pstgInfo)))
         {
            LoadInfo(pstgInfo, hch, loadfileversion);
            IStream* pstmItem;
            if (SUCCEEDED(hr = pstgData->OpenStream(L"CustomInfoTags", nullptr, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmItem)))
            {
               hr = LoadCustomInfo(pstgInfo, pstmItem, hch, loadfileversion);
               pstmItem->Release();
               pstmItem = nullptr;
            }
            pstgInfo->Release();
         }

         if (SUCCEEDED(hr = LoadData(pstmGame, loadfileversion, hch, (loadfileversion < NO_ENCRYPTION_FORMAT_VERSION) ? hkey : NULL)))
         {
            const int csubobj = m_loadTemp[0];
            const int csounds = m_loadTemp[1];
            const int ctextures = m_loadTemp[2];
            const int cfonts = m_loadTemp[3];
            const int ccollection = m_loadTemp[4];
            
            PLOGI << "LoadData loaded"; // For profiling

            const int ctotalitems = csubobj + csounds + ctextures + cfonts;
            feedback.AboutToProcessTable(ctotalitems);

            for (int i = 0; i < csubobj; i++)
            {
               const wstring wStmName = L"GameItem" + std::to_wstring(i);

               IStream* pstmItem;
               if (SUCCEEDED(hr = pstgData->OpenStream(wStmName.c_str(), nullptr, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmItem)))
               {
                  ULONG read;
                  ItemTypeEnum type;
                  pstmItem->Read(&type, sizeof(int), &read);

                  IEditable* const piedit = EditableRegistry::Create(type);
                  if (piedit)
                  {
                     hr = piedit->InitLoad(pstmItem, this, loadfileversion, (loadfileversion < 1000) ? hch : NULL, (loadfileversion < 1000) ? hkey : NULL); // 1000 (VP10 beta) removed the encryption //!! NO_ENCRYPTION_FORMAT_VERSION?
                     piedit->InitVBA(false, nullptr);
                     pstmItem->Release();
                     pstmItem = nullptr;
                     if (FAILED(hr)) break;

                     m_vedit.push_back(piedit);

                     //hr = piedit->InitPostLoad();
                  }
               }
               feedback.ItemHasBeenProcessed(i + 1, csubobj);
            }

            PLOGI << "GameItem loaded"; // For profiling

            for (int i = 0; i < csounds; i++)
            {
               const wstring wStmName = L"Sound" + std::to_wstring(i);

               IStream* pstmItem;
               if (SUCCEEDED(hr = pstgData->OpenStream(wStmName.c_str(), nullptr, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmItem)))
               {
                  VPX::Sound *pps = VPX::Sound::CreateFromStream(pstmItem, loadfileversion);
                  pstmItem->Release();
                  pstmItem = nullptr;
                  if (pps)
                  {
                     // search for duplicate names, do not load dupes
                     for (size_t i2 = 0; i2 < m_vsound.size(); ++i2)
                        if (m_vsound[i2]->GetName() == pps->GetName())
                        {
                           PLOGE << "Duplicate sound name found: " << pps->GetName() << ", not loading it!";
                           delete pps;
                           pps = nullptr;
                           break;
                        }
                     if (pps)
                        m_vsound.push_back(pps);
                  }
               }
               feedback.SoundHasBeenProcessed(i + 1, csounds);
            }

            assert(m_vimage.empty());
            m_vimage.resize(ctextures); // due to multithreaded loading do pre-allocation
            {
               ThreadPool pool(g_pvp->GetLogicalNumberOfProcessors());
               int count = 0;
               for (int i = 0; i < ctextures; i++)
               {
                  pool.enqueue([i, &feedback, loadfileversion, pstgData, this, &count, ctextures] {
                     const wstring wStmName = L"Image" + std::to_wstring(i);

                     IStream* pstmItem;
                     HRESULT hr;
                     if (FAILED(hr = pstgData->OpenStream(wStmName.c_str(), nullptr, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmItem)))
                        return hr;

                     m_vimage[i] = Texture::CreateFromStream(pstmItem, loadfileversion, this);
                     feedback.ImageHasBeenProcessed(++count, ctextures);
                     pstmItem->Release();
                     pstmItem = nullptr;
                     return hr;
                  });
               }
               pool.wait_until_empty();
               pool.wait_until_nothing_in_flight();
            }

            // search for duplicate names, delete dupes
            if (!m_vimage.empty())
               for (size_t i = 0; i < m_vimage.size() - 1; ++i)
                  for (size_t i2 = i+1; i2 < m_vimage.size(); ++i2)
                     if (m_vimage[i]->m_name == m_vimage[i2]->m_name && m_vimage[i]->GetFilePath() == m_vimage[i2]->GetFilePath())
                     {
                        m_vimage.erase(m_vimage.begin()+i2);
                        --i2;
                     }

            for (int i = 0; i < cfonts; i++)
            {
               const wstring wStmName = L"Font" + std::to_wstring(i);

               IStream* pstmItem;
               if (SUCCEEDED(hr = pstgData->OpenStream(wStmName.c_str(), nullptr, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmItem)))
               {
                  PinFont * const ppf = new PinFont();
                  ppf->LoadFromStream(pstmItem, loadfileversion);
                  m_vfont.push_back(ppf);
                  ppf->Register();
                  pstmItem->Release();
                  pstmItem = nullptr;
               }
               feedback.FontHasBeenProcessed(i + 1, cfonts);
            }

            for (int i = 0; i < ccollection; i++)
            {
               const wstring wStmName = L"Collection" + std::to_wstring(i);

               IStream* pstmItem;
               if (SUCCEEDED(hr = pstgData->OpenStream(wStmName.c_str(), nullptr, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmItem)))
               {
                  CComObject<Collection> *pcol;
                  CComObject<Collection>::CreateInstance(&pcol);
                  pcol->AddRef();
                  pcol->LoadData(pstmItem, loadfileversion, hch, (loadfileversion < NO_ENCRYPTION_FORMAT_VERSION) ? hkey : NULL);
                  m_vcollection.push_back(pcol);
                  m_pcv->AddItem((IScriptable *)pcol, false);
                  pstmItem->Release();
                  pstmItem = nullptr;
               }
               feedback.CollectionHasBeenProcessed(i + 1, ccollection);
            }

            for (size_t i = 0; i < m_vedit.size(); i++)
               m_vedit[i]->InitPostLoad();

            for (int i = 0; i < m_vcollection.size(); i++)
               m_vcollection[i].InitPostLoad(this);
         }
         pstmGame->Release();
         feedback.Finalizing();

         // Authentication block
         if (hch && loadfileversion > 40)
         {
            if (SUCCEEDED(hr = pstgData->OpenStream(L"MAC", nullptr, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmVersion)))
            {
               BYTE hashvalOld[256];
               ULONG read;
               hr = pstmVersion->Read(&hashvalOld, HASHLENGTH, &read);

               BYTE hashval[256];
               DWORD hashlen = 256;
               #ifndef __STANDALONE__
                  int foo2 = CryptGetHashParam(hch, HP_HASHSIZE, hashval, &hashlen, 0);
                  hashlen = 256;
                  foo2 = CryptGetHashParam(hch, HP_HASHVAL, hashval, &hashlen, 0);
                  foo2 = CryptDestroyHash(hch);
                  foo2 = CryptDestroyHash(hchkey);
                  foo2 = CryptDestroyKey(hkey);
                  foo2 = CryptReleaseContext(hcp, 0);
               #endif
               pstmVersion->Release();

               #ifndef __STANDALONE__
                  for (int i = 0; i < HASHLENGTH; i++)
                     if (hashval[i] != hashvalOld[i])
                     {
                        hr = APPX_E_BLOCK_HASH_INVALID;
                        break;
                     }
               #endif
            }
            else
               hr = APPX_E_CORRUPT_CONTENT; // Error
         }
         // End Authentication block

         if (loadfileversion < 1030) // the m_fGlossyImageLerp part was included first with 10.3, so set all previously saved materials to the old default
            for (size_t i = 0; i < m_materials.size(); ++i)
               m_materials[i]->m_fGlossyImageLerp = 1.f;

         if (loadfileversion < 1040) // the m_fThickness part was included first with 10.4, so set all previously saved materials to the old default
            for (size_t i = 0; i < m_materials.size(); ++i)
               m_materials[i]->m_fThickness = 0.05f;

         if (loadfileversion < 1072) // playfield meshes were always forced as collidable until 10.7.1
            for (size_t i = 0; i < m_vedit.size(); ++i)
               if (m_vedit[i]->GetItemType() == ItemTypeEnum::eItemPrimitive && (((Primitive *)m_vedit[i])->IsPlayfield()))
               {
                  Primitive* const prim = (Primitive *)m_vedit[i];
                  prim->put_IsToy(FTOVB(false));
                  prim->put_Collidable(FTOVB(true));
               }

         // reflections were hardcoded without render probe before 10.8.0
         RenderProbe *pf_reflection_probe = GetRenderProbe(RenderProbe::PLAYFIELD_REFLECTION_RENDERPROBE_NAME);
         if (pf_reflection_probe == nullptr)
         {
            pf_reflection_probe = new RenderProbe();
            pf_reflection_probe->SetName(RenderProbe::PLAYFIELD_REFLECTION_RENDERPROBE_NAME);
            pf_reflection_probe->SetReflectionMode(RenderProbe::ReflectionMode::REFL_DYNAMIC);
            m_vrenderprobe.push_back(pf_reflection_probe);
         }
         constexpr vec4 plane{0.f, 0.f, 1.f, 0.f};
         pf_reflection_probe->SetType(RenderProbe::PLANE_REFLECTION);
         pf_reflection_probe->SetReflectionPlane(plane);
         pf_reflection_probe->SetReflectionNoLightmaps(true);

         if (loadfileversion < 1080)
         {
            // Glass was horizontal before 10.8
            m_glassBottomHeight = m_glassTopHeight;

            for (size_t i = 0; i < m_vedit.size(); ++i)
            {
               if (m_vedit[i]->GetItemType() == ItemTypeEnum::eItemPrimitive && (((Primitive *)m_vedit[i])->m_d.m_disableLightingBelow != 1.0f))
               {
                  Primitive *const prim = (Primitive *)m_vedit[i];
                  // Before 10.8 alpha channel of texture was discarded if material transparency was 1, in turn leading to disabling lighting from below.
                  Material* mat = GetMaterial(prim->m_d.m_szMaterial);
                  if (mat && (!mat->m_bOpacityActive || mat->m_fOpacity == 1.0f))
                     prim->m_d.m_disableLightingBelow = 1.0f;
               }
               if (m_vedit[i]->GetItemType() == ItemTypeEnum::eItemPrimitive && (((Primitive *)m_vedit[i])->IsPlayfield()))
               {
                  Primitive* const prim = (Primitive *)m_vedit[i];
                  // playfield meshes were always processed as static until 10.8.0 (more precisely, directly rendered before everything else even in camera mode, then skipped when rendering all parts)
                  prim->m_d.m_staticRendering = true;
                  // since playfield were always rendered before bulb light buffer until 10.8, they would never have transmitted light
                  prim->m_d.m_disableLightingBelow = 1.0f;
                  // playfield meshes were always forced as visible until 10.8.0
                  prim->put_Visible(FTOVB(true));
                  // playfield meshes were always drawn before other transparent parts until 10.8.0
                  prim->m_d.m_depthBias = 100000.0f;
                  // playfield meshes did not handle backfaces until 10.8.0
                  prim->m_d.m_backfacesEnabled = false;
               }
               if (m_vedit[i]->GetItemType() == ItemTypeEnum::eItemLight)
               {
                  Light* const light = (Light *)m_vedit[i];
                  // Before 10.8, lights would never be reflected
                  light->m_d.m_reflectionEnabled = false;
                  // Before 10.8, lights did not have a z coordinate for the light emission point: classic lights where renderer at surface+0.1, bulb light at surface+halo height+0.1
                  // This needs to be preserved to avoid changing the light falloff curve, so we set up with the same definition (the 0.1 offset on z axis being applied when rendering to avoid z fighting)
                  light->m_d.m_height = light->m_d.m_BulbLight ? light->m_d.m_bulbHaloHeight : 0.0f;
                  if (!light->m_d.m_BulbLight)
                  {
                     // Before 10.8, classic light could not have a bulb mesh so force it off
                     light->m_d.m_showBulbMesh = false;
                     // Before 10.8, classic light could not have ball reflection so force it off
                     light->m_d.m_showReflectionOnBall = false;
                  }
                  // Before 10.8, bulb mesh visibility was combined with lightmap visibility (i.e. a hidden light could be reflecting but not have a bulb mesh). Note that light visible property was only accessible through script
                  if (!light->m_d.m_visible)
                     light->m_d.m_showBulbMesh = false;
               }
            }
         }

         // Since 10.8.1, layers have been replaced by groups with properties, remove temporary groups created during loading, and keep partgroups at the beginning of the list.
         std::stable_partition(m_vedit.begin(), m_vedit.end(), [](IEditable *p) { return p->GetItemType() == ItemTypeEnum::eItemPartGroup; });
         auto removeLegacyLayers = std::stable_partition(m_vedit.begin(), m_vedit.end(),
            [&](IEditable *editable)
            {
               if (editable->GetItemType() != eItemPartGroup)
                  return true;
               if (!editable->GetName().starts_with("Layer_"))
                  return true;
               auto v = std::ranges::find_if(m_vedit, [editable](const IEditable *e) { return e->GetPartGroup() == editable; });
               return v != m_vedit.end();
            });
         std::for_each(removeLegacyLayers, m_vedit.end(), [](IEditable *e) { e->Release(); });
         m_vedit.erase(removeLegacyLayers, m_vedit.end());

         if (loadfileversion < 1081)
         {
            // Rename layers that have been automatically converted to group if there aren't any name conflict (checking for collection objects, as well as script variable names)
            #ifndef __STANDALONE__
               const size_t scriptLength = ::SendMessage(m_pcv->m_hwndScintilla, SCI_GETTEXTLENGTH, 0, 0);
               string script(scriptLength, '\0');
               ::SendMessage(m_pcv->m_hwndScintilla, SCI_GETTEXT, scriptLength + 1, (LPARAM)script.data());
            #else
               string script = m_pcv->m_script_text;
            #endif
            StrToLower(script);
            std::ranges::for_each(m_vedit,
               [&](IEditable *editable)
               {
                  if (editable->GetItemType() != eItemPartGroup)
                     return;
                  const string name(editable->GetName());
                  if (!name.starts_with("Layer_"))
                     return;
                  const string shortName = name.substr(6);
                  const string shortNameLCase = lowerCase(shortName);
                  auto v = std::ranges::find_if(m_vedit, [&shortNameLCase](const IEditable *const e) { return lowerCase(e->GetName()) == shortNameLCase; });
                  if (v != m_vedit.end())
                     return; // Conflict with another part name
                  if ((shortName.find_first_not_of("0123456789") != std::string::npos) && script.find(shortNameLCase) != std::string::npos)
                     return; // (Potential) conflict with a script variable
                  for (int i = 0; i < m_vcollection.size(); i++)
                  {
                     if (lowerCase(MakeString(m_vcollection.ElementAt(i)->m_wzName)) == shortNameLCase)
                        return; // Conflict with a collection name
                  }
                  MultiByteToWideCharNull(CP_ACP, 0, shortName.c_str(), -1, editable->GetScriptable()->m_wzName, (int)std::size(editable->GetScriptable()->m_wzName));
               });
         }
         
         // Since 10.8.1, Flashers are allowed on a 2D backdrop, with advanced rendering capabilities.
         /* This code would replace a DMD textbox by a flasher. It is deactivated since it would break scripting (but does anyone script this ?)
         for (size_t i = 0; i < m_vedit.size(); ++i)
         {
            if (m_vedit[i]->GetItemType() == ItemTypeEnum::eItemTextbox)
            {
               Textbox *const textbox = (Textbox *)m_vedit[i];
               if (textbox->m_d.m_isDMD || StrFindNoCase(textbox->m_d.m_text, "DMD"s) != string::npos)
               {
                  if (textbox->GetScriptable())
                     m_pcv->RemoveItem(textbox->GetScriptable());
                  Flasher* const dmd = (Flasher *)EditableRegistry::CreateAndInit(ItemTypeEnum::eItemFlasher, this, 0, 0);
                  m_pcv->RemoveItem(dmd->GetScriptable());
                  wcsncpy_s(dmd->m_wzName, sizeof(dmd->m_wzName), textbox->m_wzName);
                  dmd->UpdatePoint(0, textbox->m_d.m_v1.x, textbox->m_d.m_v1.y);
                  dmd->UpdatePoint(1, textbox->m_d.m_v1.x, textbox->m_d.m_v2.y);
                  dmd->UpdatePoint(2, textbox->m_d.m_v2.x, textbox->m_d.m_v2.y);
                  dmd->UpdatePoint(3, textbox->m_d.m_v2.x, textbox->m_d.m_v1.y);
                  dmd->m_backglass = true;
                  dmd->m_d.m_isVisible = textbox->m_d.m_visible;
                  dmd->m_d.m_renderMode = FlasherData::DMD;
                  dmd->m_d.m_renderStyle = 0; // Legacy rendering style
                  dmd->m_d.m_imagealignment = ImageModeWrap;
                  dmd->m_d.m_color = textbox->m_d.m_fontcolor;
                  dmd->m_d.m_addBlend = false;
                  dmd->m_d.m_modulate_vs_add = 1.f; // Actually alpha
                  dmd->m_d.m_alpha = static_cast<int>(100.f * textbox->m_d.m_intensity_scale); // Actually brightness
                  dmd->m_d.m_intensity_scale = 1.f; // Actually brightness scale
                  dmd->m_vCollection.insert(dmd->m_vCollection.begin(), textbox->m_vCollection.begin(), textbox->m_vCollection.end());
                  for (Collection *const pcollection : textbox->m_vCollection)
                  {
                     pcollection->m_visel.find_erase(textbox->GetISelect());
                     pcollection->m_visel.push_back(dmd);
                  }
                  m_vedit[i] = dmd;
                  m_pcv->AddItem(dmd->GetScriptable(), false);
                  char *szT = MakeChar(dmd->m_wzName);
                  PLOGI << "Textbox used as DMD replaced by a flasher (name=" << szT << ')';
                  delete[] szT;
                  break;
               }
            }
         }*/

         // Do not consider properties converted to settings as changes to avoid creating an ini for each opened old table (they will be imported again as they are part of the VPX file)
         m_settings.SetModified(false);
      }
      pstgData->Release();
   }

   if (m_pbTempScreenshot) // For some reason, no image picked up the screenshot.  Not good; but we'll dump it to make sure it gets cleaned up
   {
      delete m_pbTempScreenshot;
      m_pbTempScreenshot = nullptr;
   }

   feedback.Done();

   pstgRoot->Release();

   return hr;
}

void PinTable::SetLoadDefaults()
{
   for (unsigned int i = 0; i < NUM_BG_SETS; ++i)
      m_BG_image[i].clear();
   m_imageColorGrade.clear();
   m_ballImage.clear();
   m_ballSphericalMapping = true;
   m_ballImageDecal.clear();
   m_ImageBackdropNightDay = false;
   m_envImage.clear();

   m_screenShot.clear();

   m_colorbackdrop = RGB(0x62, 0x6E, 0x8E);

   m_lightAmbient = RGB((int)(0.1 * 255), (int)(0.1 * 255), (int)(0.1 * 255));
   for (unsigned int i = 0; i < MAX_LIGHT_SOURCES; ++i)
   {
      m_Light[i].emission = RGB((int)(0.4 * 255), (int)(0.4 * 255), (int)(0.4 * 255));
      m_Light[i].pos = Vertex3Ds(0.f, 0.f, 400.0f);
   }

   m_lightHeight = 1000.0f;
   m_lightRange = 3000.0f;
   m_lightEmissionScale = 1000000.0f;
   m_globalEmissionScale = 1.0f;
   m_envEmissionScale = 10.0f;
   m_AOScale = 1.75f;
   m_SSRScale = 0.5f;

   m_angletiltMax = 6.0f;
   m_angletiltMin = 4.5f;

   m_playfieldReflectionStrength = 0.2f;

   m_ballPlayfieldReflectionStrength = 1.f;

   m_enableAO = true;
   m_enableSSR = true;
   m_toneMapper = ToneMapper::TM_REINHARD; // pre-10.8, this was the default

   m_bloom_strength = 1.0f;

   m_TableSoundVolume = 1.0f;
   m_TableMusicVolume = 1.0f;

   m_BallDecalMode = false;

   m_overridePhysicsFlipper = false;
}

HRESULT PinTable::LoadData(IStream* pstm, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   SetLoadDefaults();

   memset(m_loadTemp, 0, sizeof(m_loadTemp));

   BiffReader br(pstm, this, version, hcrypthash, hcryptkey);
   const HRESULT hr = br.Load();

   return hr;
}

bool PinTable::LoadToken(const int id, BiffReader * const pbr)
{
   const string INIFilename = GetSettingsFileName();
   const bool hasIni = !INIFilename.empty() && FileExists(INIFilename);
   switch(id)
   {
   case FID(PIID): { int pid; pbr->GetInt(&pid); } break;
   case FID(LEFT): pbr->GetFloat(m_left); break;
   case FID(TOPX): pbr->GetFloat(m_top); break;
   case FID(RGHT): pbr->GetFloat(m_right); break;
   case FID(BOTM): pbr->GetFloat(m_bottom); break;
   case FID(VSM0): pbr->GetInt(&mViewSetups[BG_DESKTOP].mMode); break;
   case FID(ROTA): pbr->GetFloat(mViewSetups[BG_DESKTOP].mViewportRotation); break;
   case FID(LAYB): pbr->GetFloat(mViewSetups[BG_DESKTOP].mLayback); break;
   case FID(INCL): pbr->GetFloat(mViewSetups[BG_DESKTOP].mLookAt); break;
   case FID(FOVX): pbr->GetFloat(mViewSetups[BG_DESKTOP].mFOV); break;
   case FID(SCLX): pbr->GetFloat(mViewSetups[BG_DESKTOP].mSceneScaleX); break;
   case FID(SCLY): pbr->GetFloat(mViewSetups[BG_DESKTOP].mSceneScaleY); break;
   case FID(SCLZ): pbr->GetFloat(mViewSetups[BG_DESKTOP].mSceneScaleZ); break;
   case FID(XLTX): pbr->GetFloat(mViewSetups[BG_DESKTOP].mViewX); break;
   case FID(XLTY): pbr->GetFloat(mViewSetups[BG_DESKTOP].mViewY); break;
   case FID(XLTZ): pbr->GetFloat(mViewSetups[BG_DESKTOP].mViewZ); break;
   case FID(HOF0): pbr->GetFloat(mViewSetups[BG_DESKTOP].mViewHOfs); break;
   case FID(VOF0): pbr->GetFloat(mViewSetups[BG_DESKTOP].mViewVOfs); break;
   case FID(WTZ0): pbr->GetFloat(mViewSetups[BG_DESKTOP].mWindowTopZOfs); break;
   case FID(WBZ0): pbr->GetFloat(mViewSetups[BG_DESKTOP].mWindowBottomZOfs); break;
   case FID(VSM1): pbr->GetInt(&mViewSetups[BG_FULLSCREEN].mMode); break;
   case FID(ROTF): pbr->GetFloat(mViewSetups[BG_FULLSCREEN].mViewportRotation); break;
   case FID(LAYF): pbr->GetFloat(mViewSetups[BG_FULLSCREEN].mLayback); break;
   case FID(INCF): pbr->GetFloat(mViewSetups[BG_FULLSCREEN].mLookAt); break;
   case FID(FOVF): pbr->GetFloat(mViewSetups[BG_FULLSCREEN].mFOV); break;
   case FID(SCFX): pbr->GetFloat(mViewSetups[BG_FULLSCREEN].mSceneScaleX); break;
   case FID(SCFY): pbr->GetFloat(mViewSetups[BG_FULLSCREEN].mSceneScaleY); break;
   case FID(SCFZ): pbr->GetFloat(mViewSetups[BG_FULLSCREEN].mSceneScaleZ); break;
   case FID(XLFX): pbr->GetFloat(mViewSetups[BG_FULLSCREEN].mViewX); break;
   case FID(XLFY): pbr->GetFloat(mViewSetups[BG_FULLSCREEN].mViewY); break;
   case FID(XLFZ): pbr->GetFloat(mViewSetups[BG_FULLSCREEN].mViewZ); break;
   case FID(HOF1): pbr->GetFloat(mViewSetups[BG_FULLSCREEN].mViewHOfs); break;
   case FID(VOF1): pbr->GetFloat(mViewSetups[BG_FULLSCREEN].mViewVOfs); break;
   case FID(WTZ1): pbr->GetFloat(mViewSetups[BG_FULLSCREEN].mWindowTopZOfs); break;
   case FID(WBZ1): pbr->GetFloat(mViewSetups[BG_FULLSCREEN].mWindowBottomZOfs); break;
   case FID(VSM2): pbr->GetInt(&mViewSetups[BG_FSS].mMode); break;
   case FID(ROFS): pbr->GetFloat(mViewSetups[BG_FSS].mViewportRotation); break;
   case FID(LAFS): pbr->GetFloat(mViewSetups[BG_FSS].mLayback); break;
   case FID(INFS): pbr->GetFloat(mViewSetups[BG_FSS].mLookAt); break;
   case FID(FOFS): pbr->GetFloat(mViewSetups[BG_FSS].mFOV); break;
   case FID(SCXS): pbr->GetFloat(mViewSetups[BG_FSS].mSceneScaleX); break;
   case FID(SCYS): pbr->GetFloat(mViewSetups[BG_FSS].mSceneScaleY); break;
   case FID(SCZS): pbr->GetFloat(mViewSetups[BG_FSS].mSceneScaleZ); break;
   case FID(XLXS): pbr->GetFloat(mViewSetups[BG_FSS].mViewX); break;
   case FID(XLYS): pbr->GetFloat(mViewSetups[BG_FSS].mViewY); break;
   case FID(XLZS): pbr->GetFloat(mViewSetups[BG_FSS].mViewZ); break;
   case FID(HOF2): pbr->GetFloat(mViewSetups[BG_FSS].mViewHOfs); break;
   case FID(VOF2): pbr->GetFloat(mViewSetups[BG_FSS].mViewVOfs); break;
   case FID(WTZ2): pbr->GetFloat(mViewSetups[BG_FSS].mWindowTopZOfs); break;
   case FID(WBZ2): pbr->GetFloat(mViewSetups[BG_FSS].mWindowBottomZOfs); break;
   case FID(EFSS): { pbr->GetBool(m_isFSSViewModeEnabled); UpdateCurrentBGSet(); break; }
   //case FID(VERS): pbr->GetString(szVersion); break;
   case FID(ORRP): pbr->GetInt(m_overridePhysics); break;
   case FID(ORPF): pbr->GetBool(m_overridePhysicsFlipper); break;
   case FID(GAVT): pbr->GetFloat(m_Gravity); break;
   case FID(FRCT): pbr->GetFloat(m_friction); break;
   case FID(ELAS): pbr->GetFloat(m_elasticity); break;
   case FID(ELFA): pbr->GetFloat(m_elasticityFalloff); break;
   case FID(PFSC): pbr->GetFloat(m_scatter); break;
   case FID(SCAT): pbr->GetFloat(m_defaultScatter); break;
   case FID(NDGT): pbr->GetFloat(m_nudgeTime); break;
   case FID(PHML):
   {
      pbr->GetInt(m_PhysicsMaxLoops);
      if (m_PhysicsMaxLoops == 0xFFFFFFFF)
         m_PhysicsMaxLoops = m_settings.GetPlayer_PhysicsMaxLoops();
      break;
   }
   case FID(DECL): pbr->GetBool(m_renderDecals); break;
   case FID(REEL): pbr->GetBool(m_renderEMReels); break;
   case FID(OFFX): pbr->GetFloat(m_offset.x); break;
   case FID(OFFY): pbr->GetFloat(m_offset.y); break;
   case FID(ZOOM): pbr->GetFloat(m_zoom); break;
   case FID(SLPX): pbr->GetFloat(m_angletiltMax); break;
   case FID(SLOP): pbr->GetFloat(m_angletiltMin); break;
   case FID(GLAS): pbr->GetFloat(m_glassTopHeight); break;
   case FID(GLAB): pbr->GetFloat(m_glassBottomHeight); break;
   case FID(IMAG): pbr->GetString(m_image); break;
   case FID(BLIM): pbr->GetString(m_ballImage); break;
   case FID(BLSM): pbr->GetBool(m_ballSphericalMapping); break;
   case FID(BLIF): pbr->GetString(m_ballImageDecal); break;
   case FID(SSHT): pbr->GetString(m_screenShot); break;
   case FID(FBCK): pbr->GetBool(m_backdrop); break;
   case FID(SEDT): pbr->GetInt(m_loadTemp[0]); break;
   case FID(SSND): pbr->GetInt(m_loadTemp[1]); break;
   case FID(SIMG): pbr->GetInt(m_loadTemp[2]); break;
   case FID(SFNT): pbr->GetInt(m_loadTemp[3]); break;
   case FID(SCOL): pbr->GetInt(m_loadTemp[4]); break;
   case FID(NAME): pbr->GetWideString(m_wzName, std::size(m_wzName)); break;
   case FID(BIMG): pbr->GetString(m_BG_image[0]); break;
   case FID(BIMF): pbr->GetString(m_BG_image[1]); break;
   case FID(BIMS): pbr->GetString(m_BG_image[2]); break;
   case FID(BIMN): pbr->GetBool(m_ImageBackdropNightDay); break;
   case FID(IMCG): pbr->GetString(m_imageColorGrade); break;
   case FID(EIMG): pbr->GetString(m_envImage); break;
   case FID(PLMA): pbr->GetString(m_playfieldMaterial); break;
   case FID(NOTX): pbr->GetString(m_notesText); break;
   case FID(LZAM): pbr->GetInt(m_lightAmbient); break;
   case FID(LZDI): pbr->GetInt(m_Light[0].emission); break;
   case FID(LZHI): pbr->GetFloat(m_lightHeight); break;
   case FID(LZRA): pbr->GetFloat(m_lightRange); break;
   case FID(LIES): pbr->GetFloat(m_lightEmissionScale); break;
   case FID(ENES): pbr->GetFloat(m_envEmissionScale); break;
   case FID(GLES): pbr->GetFloat(m_globalEmissionScale); break;
   case FID(AOSC): pbr->GetFloat(m_AOScale); break;
   case FID(SSSC): pbr->GetFloat(m_SSRScale); break;
   case FID(CLBH): pbr->GetFloat(m_groundToLockbarHeight); break;
   // Removed in 10.8 since we now directly define reflection in render probe. Table author can disable default playfield reflection by setting PF reflection strength to 0. Player uses app/table settings to tweak
   //case FID(BREF): pbr->GetInt(m_useReflectionForBalls); break;
   case FID(PLST):
   {
      int tmp;
      pbr->GetInt(tmp);
      m_playfieldReflectionStrength = dequantizeUnsigned<8>(tmp);
      break;
   }
   case FID(BTRA):
      if (!hasIni) // Before 10.8, user tweaks were stored in the table file (now moved to a user ini file), we import the legacy settings if there is no user ini file
      {
         int useTrailForBalls;
         pbr->GetInt(useTrailForBalls);
         if (useTrailForBalls != -1)
             m_settings.SetPlayer_BallTrail(useTrailForBalls == 1, true);
      }
      break;
   case FID(BTST):
      if (!hasIni) // Before 10.8, user tweaks were stored in the table file (now moved to a user ini file), we import the legacy settings if there is no user ini file
      {
         int ballTrailStrength;
         pbr->GetInt(ballTrailStrength);
         m_settings.SetPlayer_BallTrailStrength(dequantizeUnsigned<8>(ballTrailStrength), true);
      }
      break;
   case FID(BPRS): pbr->GetFloat(m_ballPlayfieldReflectionStrength); break;
   case FID(DBIS): pbr->GetFloat(m_defaultBulbIntensityScaleOnBall); break;
   case FID(UAAL):
      if (hasIni) // Before 10.8, user tweaks were stored in the table file (now moved to a user ini file), we import the legacy settings if there is no user ini file
      {
         int useAA;
         pbr->GetInt(useAA);
         if (useAA != -1)
             m_settings.SetPlayer_AAFactor(useAA == 0 ? 1.f : 2.f, true);
      }
      break;
   case FID(UAOC):
      {
         // Before 10.8, this setting could be set to -1, meaning override table definition using video options instead
         int useAO;
         pbr->GetInt(useAO);
         m_enableAO = useAO != 0;
      }
      break;
   case FID(USSR):
      {
         // Before 10.8, this setting could be set to -1, meaning override table definition using video options instead
         int useSSR;
         pbr->GetInt(useSSR);
         m_enableSSR = useSSR != 0;
      }
      break;
   case FID(TMAP): pbr->GetInt(&m_toneMapper); break;
   case FID(EXPO): pbr->GetFloat(m_exposure); break;
   case FID(UFXA):
      if (!hasIni) // Before 10.8, user tweaks were stored in the table file (now moved to a user ini file), we import the legacy settings if there is no user ini file
      {
         int fxaa;
         pbr->GetInt(fxaa);
         if (fxaa != -1)
            m_settings.SetPlayer_FXAA(fxaa, false);
      }
      break;
   case FID(BLST): pbr->GetFloat(m_bloom_strength); break;
   case FID(BCLR): pbr->GetInt(m_colorbackdrop); break;
   case FID(SECB): // old protection/encryption data
   {
      struct ProtectionData
      {
         int32_t fileversion;
         int32_t size;
         uint8_t paraphrase[16 + 8];
         uint32_t flags;
         int32_t keyversion;
         int32_t spare1;
         int32_t spare2;
      } protectionData;

      pbr->GetStruct(&protectionData, sizeof(ProtectionData));
      m_script_protected = ((protectionData.flags & DISABLE_EVERYTHING) == DISABLE_EVERYTHING) || ((protectionData.flags & DISABLE_SCRIPT_EDITING) == DISABLE_SCRIPT_EDITING);
      break;
   }
   case FID(CODE): // if the script is protected then we pass in the proper cryptokey into the code loadstream
   {
      m_pcv->LoadFromStream(pbr->m_pistream, pbr->m_hcrypthash, m_script_protected ? pbr->m_hcryptkey : NULL);
      break;
   }
   case FID(CCUS): pbr->GetStruct(m_rgcolorcustom, sizeof(COLORREF) * 16); break;
   case FID(TDFT): pbr->GetFloat(m_difficulty); break;
   case FID(CUST):
   {
      string tmp;
      pbr->GetString(tmp);
      m_vCustomInfoTag.push_back(tmp);
      break;
   }
   case FID(SVOL): pbr->GetFloat(m_TableSoundVolume); break;
   case FID(BDMO): pbr->GetBool(m_BallDecalMode); break;
   case FID(MVOL): pbr->GetFloat(m_TableMusicVolume); break;
   case FID(AVSY):
      if (!hasIni) // Before 10.8, user tweaks were stored in the table file (now moved to a user ini file), we import the legacy settings if there is no user ini file
      {
         int tableAdaptiveVSync;
         pbr->GetInt(tableAdaptiveVSync);
         if (tableAdaptiveVSync != -1)
         {
            switch (tableAdaptiveVSync)
            {
            case 0:
               m_settings.SetPlayer_MaxFramerate(0, true);
               m_settings.SetPlayer_SyncMode(VideoSyncMode::VSM_NONE, true);
               break;
            case 1:
               m_settings.SetPlayer_MaxFramerate(-1, true);
               m_settings.SetPlayer_SyncMode(VideoSyncMode::VSM_VSYNC, true);
               break;
            case 2:
               m_settings.SetPlayer_MaxFramerate(-1, true);
               m_settings.SetPlayer_SyncMode(VideoSyncMode::VSM_ADAPTIVE_VSYNC, true);
               break;
            default:
               m_settings.SetPlayer_MaxFramerate(static_cast<float>(tableAdaptiveVSync), true);
               m_settings.SetPlayer_SyncMode(VideoSyncMode::VSM_ADAPTIVE_VSYNC, true);
               break;
            }
         }
      }
      break;
   case FID(OGAC):
      if (!hasIni) // Before 10.8, user tweaks were stored in the table file (now moved to a user ini file), we import the legacy settings if there is no user ini file
      {
         bool overwriteGlobalDetailLevel;
         pbr->GetBool(overwriteGlobalDetailLevel);
         if (!overwriteGlobalDetailLevel)
            m_settings.ResetPlayer_AlphaRampAccuracy();
      }
      break;
   case FID(OGDN):
      if (!hasIni) // Before 10.8, user tweaks were stored in the table file (now moved to a user ini file), we import the legacy settings if there is no user ini file
      {
         // Global Day/Night was fairly convoluted:
         // - table would define the value
         // - user could select in video options to override this value by an automatic value
         // - table could then define to reject this user settings
         // - user could define in commandline to finally override the value
         // Now the logic is the same as all other settings:
         // - table defines the default value, then users define if they want to override this value (through app/table settings or commandline)
         bool overwriteGlobalDayNight;
         pbr->GetBool(overwriteGlobalDayNight);
         if (overwriteGlobalDayNight)
            m_settings.SetPlayer_OverrideTableEmissionScale(false, true);
      }
      break;
   case FID(GDAC): pbr->GetBool(m_grid); break;
   // Removed in 10.8 since we now directly define reflection in render probe. Table author can disable default playfield reflection by setting PF reflection strength to 0. Player uses app/table settings to tweak
   // case FID(REOP): pbr->GetBool(m_reflectElementsOnPlayfield); break;
   case FID(ARAC):
      if (!hasIni) // Before 10.8, user tweaks were stored in the table file (now moved to a user ini file), we import the legacy settings if there is no user ini file
      {
         int userDetailLevel; // The detail level was always saved **before** the override flag so we always load to settings, eventually deleting afterward
         pbr->GetInt(userDetailLevel);
         m_settings.SetPlayer_AlphaRampAccuracy(userDetailLevel, true);
      }
      break;
   case FID(MASI): pbr->GetInt(m_numMaterials); break;
   case FID(MATE):
   {
      vector<SaveMaterial> mats(m_numMaterials);
      pbr->GetStruct(mats.data(), (int)sizeof(SaveMaterial)*m_numMaterials);
      if (pbr->m_version < 1080
         || m_materials.empty()) // Also loads materials for 10.8+ tables if these were saved before the new material format was added. // This is hacky and should be removed when 10.9 is out (added to avoid loosing tables edited while 10.8 was in alpha)
      {
         m_materials.reserve(m_numMaterials + m_materials.size());
         for (int i = 0; i < m_numMaterials; i++)
         {
            Material * const pmat = new Material();
            pmat->m_cBase = mats[i].cBase;
            pmat->m_cGlossy = mats[i].cGlossy;
            pmat->m_cClearcoat = mats[i].cClearcoat;
            pmat->m_fWrapLighting = mats[i].fWrapLighting;
            pmat->m_fRoughness = mats[i].fRoughness;
            pmat->m_fGlossyImageLerp = 1.0f - dequantizeUnsigned<8>(mats[i].fGlossyImageLerp); //!! '1.0f -' to be compatible with previous table versions
            pmat->m_fThickness = (mats[i].fThickness == 0) ? 0.05f : dequantizeUnsigned<8>(mats[i].fThickness); //!! 0 -> 0.05f to be compatible with previous table versions
            pmat->m_fEdge = mats[i].fEdge;
            pmat->m_fOpacity = mats[i].fOpacity;
            pmat->m_type = mats[i].bIsMetal ? Material::MaterialType::METAL : Material::MaterialType::BASIC;
            pmat->m_bOpacityActive = !!(mats[i].bOpacityActive_fEdgeAlpha & 1);
            pmat->m_fEdgeAlpha = dequantizeUnsigned<7>(mats[i].bOpacityActive_fEdgeAlpha >> 1);
            pmat->m_name = mats[i].szName;
            m_materials.push_back(pmat);
         }
      }
      break;
   }
   case FID(PHMA):
   {
       vector<SavePhysicsMaterial> mats(m_numMaterials);
       pbr->GetStruct(mats.data(), (int)sizeof(SavePhysicsMaterial)*m_numMaterials);
       if (pbr->m_version < 1080
          || m_materials.size() == m_numMaterials) // Also loads materials for 10.8+ tables if these were saved before the new material format was added. // This is hacky and should be removed when 10.9 is out (added to avoid loosing tables edited while 10.8 was in alpha)
       {
          for (int i = 0; i < m_numMaterials; i++)
          {
              bool found = true;
              Material * pmat = GetMaterial(mats[i].szName);
              if (pmat == &m_vpinball->m_dummyMaterial)
              {
                  assert(!"SaveMaterial not found");
                  pmat = new Material();
                  pmat->m_name = mats[i].szName;
                  found = false;
              }
              pmat->m_fElasticity = mats[i].fElasticity;
              pmat->m_fElasticityFalloff = mats[i].fElasticityFallOff;
              pmat->m_fFriction = mats[i].fFriction;
              pmat->m_fScatterAngle = mats[i].fScatterAngle;
              if (!found)
                 m_materials.push_back(pmat);
          }
       }
       break;
   }
   case FID(MATR):
   {
      // Replace legacy materials with the new ones. // This is hacky and should be removed when 10.9 is out (added to avoid loosing tables edited while 10.8 was in alpha)
      if (pbr->m_version >= 1080 && m_materials.size() == m_numMaterials)
      {
         for (int i = 0; i < m_numMaterials; i++)
            delete m_materials[i];
         m_materials.clear();
      }
      const int record_size = pbr->GetBytesInRecordRemaining();
      HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE, record_size);
      LPVOID pData = ::GlobalLock(hMem);
      pbr->GetStruct(pData, record_size);
      ::GlobalUnlock(hMem);
      Material *mat = new Material();
      CComPtr<IStream> spStream;
#ifndef __STANDALONE__
      const HRESULT hr = ::CreateStreamOnHGlobal(hMem, FALSE, &spStream);
#else
      FastIStream fastStream;
      fastStream.m_rg = (char*)malloc(record_size);
      memcpy(fastStream.m_rg, (char*)hMem, record_size);
      fastStream.m_cSize = record_size;
      spStream.Attach(&fastStream);
#endif
      if (mat->LoadData(spStream, pbr->m_version, NULL, NULL) != S_OK)
      {
         assert(!"Invalid binary image file");
         delete mat;
         return false;
      }
      m_materials.push_back(mat);
      ::GlobalFree(hMem);
      spStream.Detach();
      break;
   }
   case FID(RPRB):
   {
      const int record_size = pbr->GetBytesInRecordRemaining();
      HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE, record_size);
      LPVOID pData = ::GlobalLock(hMem);
      pbr->GetStruct(pData, record_size);
      ::GlobalUnlock(hMem);
      RenderProbe *rpb = new RenderProbe();
      CComPtr<IStream> spStream;
#ifndef __STANDALONE__
      const HRESULT hr = ::CreateStreamOnHGlobal(hMem, FALSE, &spStream);
#else
      FastIStream fastStream;
      fastStream.m_rg = (char*)malloc(record_size);
      memcpy(fastStream.m_rg, (char*)hMem, record_size);
      fastStream.m_cSize = record_size;
      spStream.Attach(&fastStream);
#endif
      if (rpb->LoadData(spStream, pbr->m_version, NULL, NULL) != S_OK)
      {
         assert(!"Invalid binary image file");
         delete rpb;
         return false;
      }
      m_vrenderprobe.push_back(rpb);
      ::GlobalFree(hMem);
      spStream.Detach();
      break;
   }
   case FID(TLCK): pbr->GetInt(m_tablelocked); break;
   }
   return true;
}

bool PinTable::ExportSound(VPX::Sound *const pps, const string &filename)
{
   if (extension_from_path(pps->GetImportPath()) == extension_from_path(filename))
   {
      if (pps->SaveToFile(filename))
         return true;
#ifndef __STANDALONE__
      m_mdiTable->MessageBox("Can not Open/Create Sound file!", "Visual Pinball", MB_ICONERROR);
   }
   else
      m_mdiTable->MessageBox("File extension does not match, will not convert sound to other format!", "Visual Pinball", MB_ICONERROR);
#else
   }
#endif

   return false;
}

void PinTable::ReImportSound(VPX::Sound *const pps, const string &filename)
{
#ifndef __STANDALONE__
   vector<uint8_t> data = read_file(filename);
   if (!data.empty())
      pps->SetFromFileData(filename, data);
#endif
}


VPX::Sound *PinTable::ImportSound(const string &filename)
{
#ifndef __STANDALONE__
   VPX::Sound *const pps = VPX::Sound::CreateFromFile(filename);
   if (pps == nullptr)
      return nullptr;
   m_vsound.push_back(pps);
   return pps;
#else
   return nullptr;
#endif
}

void PinTable::RemoveSound(VPX::Sound *const pps)
{
   RemoveFromVectorSingle(m_vsound, pps);

   delete pps;
}

void PinTable::ImportFont(HWND hwndListView, const string& filename)
{
#ifndef __STANDALONE__
   PinFont * const ppb = new PinFont();

   ppb->ReadFromFile(filename);

   if (!ppb->m_buffer.empty())
   {
      m_vfont.push_back(ppb);
      const int index = AddListBinary(hwndListView, ppb);
      ListView_SetItemState(hwndListView, index, LVIS_SELECTED, LVIS_SELECTED);
      ppb->Register();
   }
#endif
}

void PinTable::RemoveFont(PinFont * const ppf)
{
   RemoveFromVectorSingle(m_vfont, ppf);

   ppf->UnRegister();
   delete ppf;
}

void PinTable::ListFonts(HWND hwndListView)
{
   for (size_t i = 0; i < m_vfont.size(); i++)
      AddListBinary(hwndListView, m_vfont[i]);
}

int PinTable::AddListBinary(HWND hwndListView, PinBinary *ppb)
{
#ifndef __STANDALONE__
   LVITEM lvitem;
   lvitem.mask = LVIF_DI_SETITEM | LVIF_TEXT | LVIF_PARAM;
   lvitem.iItem = 0;
   lvitem.iSubItem = 0;
   lvitem.pszText = (LPSTR)ppb->m_name.c_str();
   lvitem.lParam = (size_t)ppb;

   const int index = ListView_InsertItem(hwndListView, &lvitem);

   ListView_SetItemText(hwndListView, index, 1, (LPSTR)ppb->m_path.c_str());

   return index;
#else
   return 0L;
#endif
}

void PinTable::NewCollection(const HWND hwndListView, const bool fromSelection)
{
   CComObject<Collection> *pcol;
   CComObject<Collection>::CreateInstance(&pcol);
   pcol->AddRef();

   GetUniqueName(LocalStringW(IDS_COLLECTION).m_szbuffer, pcol->m_wzName, std::size(pcol->m_wzName));

   if (fromSelection && !MultiSelIsEmpty())
   {
      for (int i = 0; i < m_vmultisel.size(); i++)
      {
         ISelect * const pisel = m_vmultisel.ElementAt(i);
         IEditable * const piedit = pisel->GetIEditable();
         if (piedit)
         {
            if (piedit->GetISelect() == pisel) // Do this check so we don't put walls in a collection when we only have the control point selected
            {
               if (piedit->GetScriptable()) // check for scriptable because can't add decals to a collection - they have no name
               {
                  piedit->m_vCollection.push_back(pcol);
                  piedit->m_viCollection.push_back(pcol->m_visel.size());
                  pcol->m_visel.push_back(m_vmultisel.ElementAt(i));
               }
            }
         }
      }
   }

   const int index = AddListCollection(hwndListView, pcol);

#ifndef __STANDALONE__
   ListView_SetItemState(hwndListView, index, LVIS_SELECTED, LVIS_SELECTED);
#endif

   m_vcollection.push_back(pcol);
   m_pcv->AddItem((IScriptable *)pcol, false);
}

int PinTable::AddListCollection(HWND hwndListView, CComObject<Collection> *pcol)
{
#ifndef __STANDALONE__
   char * const szT = MakeChar(pcol->m_wzName);

   LVITEM lvitem;
   lvitem.mask = LVIF_DI_SETITEM | LVIF_TEXT | LVIF_PARAM;
   lvitem.iItem = 0;
   lvitem.iSubItem = 0;
   lvitem.pszText = szT;
   lvitem.lParam = (size_t)pcol;

   const int index = ListView_InsertItem(hwndListView, &lvitem);
   delete [] szT;

   string count = std::to_string(pcol->m_visel.size());
   ListView_SetItemText(hwndListView, index, 1, const_cast<char*>(count.c_str()));
   return index;
#else
   return 0;
#endif
}

void PinTable::ListCollections(HWND hwndListView)
{
   //ListView_DeleteAllItems(hwndListView);

   for (int i = 0; i < m_vcollection.size(); i++)
   {
      CComObject<Collection> * const pcol = m_vcollection.ElementAt(i);

      AddListCollection(hwndListView, pcol);
   }
}

void PinTable::RemoveCollection(CComObject<Collection> *pcol)
{
#ifndef __STANDALONE__
   m_pcv->RemoveItem((IScriptable *)pcol);
   m_vcollection.find_erase(pcol);
   pcol->Release();
#endif
}

void PinTable::MoveCollectionUp(CComObject<Collection> *pcol)
{
   const int idx = m_vcollection.find(pcol);
   assert(idx >= 0);
   m_vcollection.erase(idx);
   if (idx - 1 < 0)
      m_vcollection.push_back(pcol);
   else
      m_vcollection.insert(pcol, idx - 1);
}

FRect3D PinTable::GetBoundingBox() const
{
   FRect3D bbox;
   bbox.left = m_left;
   bbox.right = m_right;
   bbox.top = m_top;
   bbox.bottom = m_bottom;
   bbox.zlow = 0.f;
   bbox.zhigh = m_glassTopHeight;
   return bbox;
}

void PinTable::ComputeNearFarPlane(const vector<Vertex3Ds> &bounds, const Matrix3D &matWorldView, const float scale, float &zNear, float &zFar)
{
   zNear = FLT_MAX;
   zFar = -FLT_MAX;
   for (const Vertex3Ds &v : bounds)
   {
      const Vertex3Ds p = matWorldView.MultiplyVectorNoPerspective(v);
      if (p.z > 0.0f)
      {
         // Clip points behind the viewer (VR room have a lot of these)
         zNear = min(zNear, p.z);
         zFar = max(zFar, p.z);
      }
   }

   // Add a bit of margin
   zNear *= 0.9f;
   zFar *= 1.1f;
   // Clip to sensible value to fix tables with parts far far away breaking depth buffer precision
   zNear = max(zNear, scale * CMTOVPU(5.f)); // Avoid wasting depth buffer precision for parts too near to be useful
   zFar = clamp(zFar, zNear + 1.f, scale * CMTOVPU(100000.f)); // 1 km (yes some VR room do really need this...)
   // Could not reproduce, so I disabled it for the sake of avoiding to pass inc to the method which is not really meaningful here (we would have to compute it from the matWorldView)
   //!! magic threshold, otherwise kicker holes are missing for inclination ~0
   //if (fabsf(inc) < 0.0075f)
   //   zFar += 10.f;
   //PLOGD << "Near/Far plane: " << zNear << " to " << zFar;
}

void PinTable::ComputeNearFarPlane(const Matrix3D &matWorldView, const float scale, float &zNear, float &zFar) const
{
   // Adjust near/far plane for each projected bounding box
   vector<Vertex3Ds> bounds;
   bounds.reserve(m_vedit.size() * 8); // upper bound estimate
   for (IEditable *editable : m_vedit)
      editable->GetBoundingVertices(bounds, nullptr);

   ComputeNearFarPlane(bounds, matWorldView, scale, zNear, zFar);
}

void PinTable::MoveCollectionDown(CComObject<Collection> *pcol)
{
   const int idx = m_vcollection.find(pcol);
   assert(idx >= 0);
   m_vcollection.erase(idx);
   if (idx + 1 >= m_vcollection.size())
      m_vcollection.insert(pcol, 0);
   else
      m_vcollection.insert(pcol, idx + 1);
}

void PinTable::SetCollectionName(Collection *pcol, string name, HWND hwndList, int index)
{
#ifndef __STANDALONE__
   if (name.length() >= std::size(pcol->m_wzName))
      name.erase(std::size(pcol->m_wzName) - 1);
   const wstring wzT = MakeWString(name);
   if (m_pcv->ReplaceName((IScriptable *)pcol, wzT) == S_OK)
   {
      if (hwndList)
         ListView_SetItemText(hwndList, index, 0, (char*)name.c_str());
      wcsncpy_s(pcol->m_wzName, std::size(pcol->m_wzName), wzT.c_str());
   }
#endif
}

void PinTable::SetZoom(float zoom)
{
   m_zoom = zoom;
   SetMyScrollInfo();
}

void PinTable::GetViewRect(FRect * const pfrect) const
{
   if (!m_vpinball->m_backglassView)
   {
      pfrect->left = m_left;
      pfrect->top = m_top;
      pfrect->right = m_right;
      pfrect->bottom = m_bottom;
   }
   else
   {
      pfrect->left = 0;
      pfrect->top = 0;
      pfrect->right = EDITOR_BG_WIDTH;
      pfrect->bottom = EDITOR_BG_HEIGHT;
   }
}

void PinTable::SetMyScrollInfo()
{
#ifndef __STANDALONE__
   FRect frect;
   GetViewRect(&frect);

   const CRect rc = GetClientRect();

   const HitSur phs(nullptr, m_zoom, m_offset.x, m_offset.y, rc.right - rc.left, rc.bottom - rc.top, 0, 0, nullptr);

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

void PinTable::FireOptionEvent(OptionEventType eventType)
{
   int event;
   switch (eventType)
   {
   case OptionEventType::Initialized: event = 0; break;
   case OptionEventType::Changed: event = 1; break;
   case OptionEventType::Reseted: event = 2; break; // Legacy, unused (reset is now dispatched as a change)
   case OptionEventType::EndOfEdit: event = 3; break;
   }
   CComVariant rgvar[1] = { CComVariant(event) };
   DISPPARAMS dispparams = { rgvar, nullptr, 1, 0 };
   FireDispID(DISPID_GameEvents_OptionEvent, &dispparams);
}

void PinTable::DoLeftButtonDown(int x, int y, bool zoomIn)
{
#ifndef __STANDALONE__
   const int ksshift = GetKeyState(VK_SHIFT);
   const int ksctrl = GetKeyState(VK_CONTROL);

   // set the focus of the window so all keyboard and mouse inputs are processed.
   // (this fixes the problem of selecting a element on the properties dialog, clicking on a table
   // object and not being able to use the cursor keys/wheely mouse)
   m_vpinball->SetFocus();

   if ((m_vpinball->m_ToolCur == ID_TABLE_MAGNIFY) || (ksctrl & 0x80000000))
   {
      if (m_zoom < MAX_ZOOM)
      {
         m_offset = TransformPoint(x, y);

         SetZoom(m_zoom * (zoomIn ? 1.5f : 0.5f));

         SetDirtyDraw();
      }
   }
   else
   {
      ISelect * const pisel = HitTest(x, y);

      const bool add = ((ksshift & 0x80000000) != 0);

      if (pisel == (ISelect *)this && add)
      {
         // Can not include the table in multi-select
         // and table will not be unselected, because the
         // user might be drawing a box around other objects
         // to add them to the selection group
         OnLButtonDown(x, y); // Start the band select
         return;
      }

      AddMultiSel(pisel, add, true, false);

      m_moving = true;
      for (int i = 0; i < m_vmultisel.size(); i++)
      {
         ISelect *const pisel2 = m_vmultisel.ElementAt(i);
         if (pisel2)
            pisel2->OnLButtonDown(x, y);
      }
   }
#endif
}

void PinTable::OnLeftButtonUp(int x, int y)
{
#ifndef __STANDALONE__
   if (!m_dragging) // Not doing band select
   {
      for (int i = 0; i < m_vmultisel.size(); i++)
      {
         ISelect * const pisel = m_vmultisel.ElementAt(i);
         if (pisel)
            pisel->OnLButtonUp(x, y);
      }
      if (m_moving)
      {
          m_moving = false;
          m_vpinball->SetPropSel(m_vmultisel);
      }
   }
   else
   {
      OnLButtonUp(x, y);
   }
#endif
}

void PinTable::OnRightButtonDown(int x, int y)
{
#ifndef __STANDALONE__
   OnLeftButtonUp(x, y); //corrects issue with left mouse button being in 'stuck down' position on a control point or object - BDS

   const int ks = GetKeyState(VK_CONTROL);

   if ((m_vpinball->m_ToolCur == ID_TABLE_MAGNIFY) || (ks & 0x80000000))
   {
      if (m_zoom > MIN_ZOOM)
      {
         m_offset = TransformPoint(x, y);
         SetZoom(m_zoom * 0.5f);

         SetDirtyDraw();
      }
   }
   else
   {
      // keep the selection if clicking over a selected object, even if
      // the selected object is hidden behind other objects
      ISelect *hit = HitTest(x, y);
      for (int i = 0; i < m_vmultisel.size(); i++)
      {
         if (FindIndexOf(m_allHitElements, m_vmultisel.ElementAt(i)) != -1)
         {
            // found a selected item - keep the current selection set
            // by re-selecting this item (which will also promote it
            // to the head of the selection list)
            hit = m_vmultisel.ElementAt(i);
            break;
         }
      }

      // update the selection
      AddMultiSel(hit, false, true, false);
   }
#endif
}

void PinTable::FillCollectionContextMenu(CMenu &mainMenu, CMenu &colSubMenu, ISelect *psel)
{
#ifndef __STANDALONE__
    mainMenu.AppendMenu(MF_POPUP | MF_STRING, (size_t)colSubMenu.GetHandle(), LocalString(IDS_TO_COLLECTION).m_szbuffer);

    const int maxItems = m_vcollection.size() - 1;

    // run through all collections and list them in the context menu
    // the actual processing is done in ISelect::DoCommand() 
    for (int i = maxItems; i >= 0; i--)
    {
        char * const szT = MakeChar(m_vcollection[i].get_Name());

        UINT flags = MF_POPUP | MF_UNCHECKED;
        if ((maxItems-i) % 32 == 0) // add new column each 32 entries
           flags |= MF_MENUBREAK;
        colSubMenu.AppendMenu(flags, 0x40000 + i, szT);
        delete [] szT;
    }
    if (m_vmultisel.size() == 1)
    {
        for (int i = maxItems; i >= 0; i--)
            for (int t = 0; t < m_vcollection[i].m_visel.size(); t++)
                if (psel == m_vcollection[i].m_visel.ElementAt(t))
                    colSubMenu.CheckMenuItem(0x40000 + i, MF_CHECKED);
    }
    else
    {
        vector<int> allIndices;

        for (int t = 0; t < m_vmultisel.size(); t++)
        {
            const ISelect * const iSel = m_vmultisel.ElementAt(t);

            for (int i = maxItems; i >= 0; i--)
                for (int t2 = 0; t2 < m_vcollection[i].m_visel.size(); t2++)
                    if ((iSel == m_vcollection[i].m_visel.ElementAt(t2)))
                        allIndices.push_back(i);
        }
        for (size_t i = 0; i < allIndices.size(); i++)
            colSubMenu.CheckMenuItem(0x40000 + allIndices[i], MF_CHECKED);
    }
#endif
}

void PinTable::FillLayerContextMenu(CMenu &mainMenu, CMenu &layerSubMenu, ISelect *psel) 
{
#ifndef __STANDALONE__
   mainMenu.AppendMenu(MF_POPUP | MF_STRING, (size_t)layerSubMenu.GetHandle(), LocalString(IDS_ASSIGN_TO_LAYER2).m_szbuffer);
   int i = 0;
   for (IEditable *edit : m_vedit)
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

void PinTable::AssignSelectionToPartGroup(PartGroup* group)
{
#ifndef __STANDALONE__
   STARTUNDO
   for (int t = 0; t < m_vmultisel.size(); t++)
   {
      ISelect *const psel = m_vmultisel.ElementAt(t);
      IEditable *const pedit = psel->GetIEditable();
      pedit->SetPartGroup(group);
      if (psel->m_isVisible && !group->m_isVisible)
         psel->m_isVisible = false;
      else if (!psel->m_isVisible && group->m_isVisible)
         psel->m_isVisible = true;
   }
   STOPUNDO
   g_pvp->GetLayersListDialog()->Update();
#endif
}

void PinTable::DoContextMenu(int x, int y, const int menuid, ISelect *psel)
{
#ifndef __STANDALONE__
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

   psel->EditMenu(newMenu);

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

      FillLayerContextMenu(newMenu, layerSubMenu, psel);
      newMenu.AppendMenu(MF_STRING, ID_ASSIGN_TO_CURRENT_LAYER, LocalString(IDS_ASSIGN_TO_CURRENT_LAYER).m_szbuffer);
      FillCollectionContextMenu(newMenu, colSubMenu, psel);

      newMenu.AppendMenu(MF_STRING, ID_LOCK, LocalString(IDS_LOCK).m_szbuffer);

      newMenu.AppendMenu(MF_SEPARATOR, ~0u, "");
      newMenu.AppendMenu(MF_SEPARATOR, ~0u, "");
      // now list all elements that are stacked at the mouse pointer
      for (size_t i = 0; i < m_allHitElements.size(); i++)
      {
         if (!m_allHitElements[i]->GetIEditable()->GetISelect()->m_isVisible)
            continue;

         ISelect * const ptr = m_allHitElements[i];
         if (ptr)
         {
            IEditable * const pedit = m_allHitElements[i]->GetIEditable();
            if (pedit)
            {
               const string szTemp = GetElementName(pedit);
               if (!szTemp.empty())
               {
                  //!! what a hack!
                  // the element index of the allHitElements vector is encoded inside the ID of the context menu item
                  // I didn't find an easy way to identify the selected menu item of a context menu
                  // so the ID_SELECT_ELEMENT is the global ID for selecting an element from the list and the rest is
                  // added for finding the element out of the list
                  // the selection is done in ISelect::DoCommand()
                  const UINT_PTR id = 0x80000000 + ((UINT_PTR)i << 16) + ID_SELECT_ELEMENT;
                  newMenu.AppendMenu(MF_STRING, id, szTemp.c_str());
               }
            }
         }
      }
      bool locked = psel->m_locked;
      //!! HACK
      if (psel == this) // multi-select case
      {
         locked = FMutilSelLocked();
      }
      newMenu.CheckMenuItem(ID_LOCK, MF_BYCOMMAND | (locked ? MF_CHECKED : MF_UNCHECKED));
   }

   const int icmd = newMenu.TrackPopupMenuEx(TPM_RETURNCMD, pt.x, pt.y, m_mdiTable->GetHwnd(), nullptr);

   if (icmd != 0)
      psel->DoCommand(icmd, x, y);

   newMenu.Destroy();

   if (menuid != -1)
       mainMenu.Destroy();
#endif
}

string PinTable::GetElementName(IEditable *pedit)
{
   if (pedit)
      return pedit->GetName();
   return string();
}

IEditable *PinTable::GetElementByName(const char * const name) const
{
   for (size_t i = 0; i < m_vedit.size(); i++)
   {
      IEditable * const pedit = m_vedit[i];
      if (name == GetElementName(pedit))
         return pedit;
   }
   return nullptr;
}

bool PinTable::FMutilSelLocked()
{
   for (int i = 0; i < m_vmultisel.size(); i++)
      if (m_vmultisel[i].m_locked)
         return true;

   return false;
}

void PinTable::DoCommand(int icmd, int x, int y)
{
#ifndef __STANDALONE__
   if (((icmd & 0x000FFFFF) >= 0x40000) && ((icmd & 0x000FFFFF) < 0x40020))
   {
      UpdateCollection(icmd & 0x000000FF);
      return;
   }

   if ((icmd >= ID_ASSIGN_TO_LAYER1) && (icmd <= ID_ASSIGN_TO_LAYER1+NUM_ASSIGN_LAYERS-1))
   {
      int i = 0;
      for (IEditable *edit : m_vedit)
      {
         if (edit->GetItemType() == eItemPartGroup && edit->GetPartGroup() == nullptr)
         {
            i++;
            if (icmd == (ID_ASSIGN_TO_LAYER1 + i))
               AssignSelectionToPartGroup(static_cast<PartGroup *>(edit));
            if (i == NUM_ASSIGN_LAYERS)
               break;
         }
      }
      return;
   }

   if ((icmd & 0x0000FFFF) == ID_SELECT_ELEMENT)
   {
      const int i = (icmd & 0x00FF0000) >> 16;
      ISelect * const pisel = m_allHitElements[i];
      pisel->DoCommand(icmd, x, y);
      return;
   }

   switch (icmd)
   {
       case ID_DRAWINFRONT:
       case ID_DRAWINBACK:
       {
           for (int i = 0; i < m_vmultisel.size(); i++)
           {
               ISelect *const psel = m_vmultisel.ElementAt(i);
               _ASSERTE(psel != this); // Would make an infinite loop
               psel->DoCommand(icmd, x, y);
           }
           break;
       }
       case ID_ASSIGN_TO_CURRENT_LAYER: m_vpinball->GetLayersListDialog()->AssignToSelectedGroup(); break;
       case ID_EDIT_DRAWINGORDER_HIT: m_vpinball->ShowDrawingOrderDialog(false); break;
       case ID_EDIT_DRAWINGORDER_SELECT: m_vpinball->ShowDrawingOrderDialog(true); break;
       case ID_LOCK: LockElements(); break;
       case ID_WALLMENU_FLIP: FlipY(GetCenter()); break;
       case ID_WALLMENU_MIRROR: FlipX(GetCenter()); break;
       case IDC_COPY: Copy(x, y); break;
       case IDC_PASTE: Paste(false, x, y); break;
       case IDC_PASTEAT: Paste(true, x, y); break;
       case ID_WALLMENU_ROTATE: DialogBoxParam(m_vpinball->theInstance, MAKEINTRESOURCE(IDD_ROTATE), m_vpinball->GetHwnd(), RotateProc, (size_t)(ISelect *)this); break;
       case ID_WALLMENU_SCALE: DialogBoxParam(m_vpinball->theInstance, MAKEINTRESOURCE(IDD_SCALE), m_vpinball->GetHwnd(), ScaleProc, (size_t)(ISelect *)this); break;
       case ID_WALLMENU_TRANSLATE: DialogBoxParam(m_vpinball->theInstance, MAKEINTRESOURCE(IDD_TRANSLATE), m_vpinball->GetHwnd(), TranslateProc, (size_t)(ISelect *)this); break;
   }
#endif
}

void PinTable::UpdateCollection(const int index)
{
   if (index < m_vcollection.size())
   {
      if (!m_vmultisel.empty())
      {
         bool removeOnly = false;
         /* if the selection is part of the selected collection remove only these elements*/
         for (int t = 0; t < m_vmultisel.size(); t++)
         {
            ISelect * const ptr = m_vmultisel.ElementAt(t);
            for (int k = 0; k < m_vcollection[index].m_visel.size(); k++)
            {
               if (ptr == m_vcollection[index].m_visel.ElementAt(k))
               {
                  m_vcollection[index].m_visel.find_erase(ptr);
                  removeOnly = true;
                  break;
               }
            }
         }

         if (removeOnly)
            return;

         /*selected elements are not part of the selected collection and can be added*/
         for (int t = 0; t < m_vmultisel.size(); t++)
         {
            ISelect * const ptr = m_vmultisel.ElementAt(t);
            m_vcollection.ElementAt(index)->m_visel.push_back(ptr);
        }
      }
   }
}

bool PinTable::GetCollectionIndex(const ISelect * const element, int &collectionIndex, int &elementIndex)
{
   for (int i = 0; i < m_vcollection.size(); i++)
   {
      for (int t = 0; t < m_vcollection[i].m_visel.size(); t++)
      {
         if (element == m_vcollection[i].m_visel.ElementAt(t))
         {
            collectionIndex = i;
            elementIndex = t;
            return true;
         }
      }
   }
   return false;
}

const WCHAR *PinTable::GetCollectionNameByElement(const ISelect * const element) const
{
    for (int i = 0; i < m_vcollection.size(); i++)
        for (int t = 0; t < m_vcollection[i].m_visel.size(); t++)
            if (element == m_vcollection[i].m_visel.ElementAt(t))
                return m_vcollection[i].m_wzName;

    return nullptr;
}

Vertex2D PinTable::EvaluateGlassHeight() const
{
   Vertex2D result(0.f, 0.f);
   constexpr float marginX = INCHESTOVPU(1.0f);
   constexpr float marginY = INCHESTOVPU(0.1f);
   constexpr float marginZ = INCHESTOVPU(0.1f);
   auto submitVertex = [this, &result](const Vertex3Ds &v)
   {
      // Reject vertices below, or outside left and right limits
      if (v.z < -marginZ || v.x < m_left - marginX || v.x > m_right + marginX)
         return;
      // Bottom area (y around 2100)
      if (v.y >= m_bottom - marginY && v.y <= m_bottom + marginY && v.z <= INCHESTOVPU(4.f))
         result.x = max(result.x, v.z);
      // Top area (y = 0)
      if (v.y >= m_top - marginY && v.y <= m_top + marginY && v.z <= INCHESTOVPU(12.f))
         result.y = max(result.y, v.z);
   };
   auto intersect2D = [](const RenderVertex &v1, const RenderVertex &v2, float y)
   {
      if ((v1.y < y - marginY && v2.y < y - marginY) || (v1.y > y + marginY && v2.y > y + marginY))
         return Vertex2D(FLT_MAX, FLT_MAX);
      if (fabs(v2.y - v1.y) < 0.01f)
         return Vertex2D(v1.x, v1.y);
      const float alpha = (y - v1.y) / (v2.y - v1.y);
      return Vertex2D(lerp(v1.x, v2.x, alpha), lerp(v1.y, v2.y, alpha));
   };
   auto submitEdge2D = [this, &intersect2D, &submitVertex](const RenderVertex &v1, const RenderVertex &v2, float y, float z)
   {
      if (Vertex2D pt = intersect2D(v1, v2, y); pt.x != FLT_MAX)
         submitVertex(Vertex3Ds(pt.x, pt.y, z));
   };
   auto intersect3D = [](const Vertex3Ds &v1, const Vertex3Ds &v2, float y)
   {
      if ((v1.y < y - marginY && v2.y < y - marginY) || (v1.y > y + marginY && v2.y > y + marginY))
         return Vertex3Ds(FLT_MAX, FLT_MAX, FLT_MAX);
      if (fabs(v2.y - v1.y) < 0.01f)
         return Vertex3Ds(v1.x, v1.y, max(v1.z, v2.z));
      const float alpha = (y - v1.y) / (v2.y - v1.y);
      return Vertex3Ds(lerp(v1.x, v2.x, alpha), lerp(v1.y, v2.y, alpha), lerp(v1.z, v2.z, alpha));
   };
   auto submitEdge3D = [this, &intersect3D, &submitVertex](const Vertex3Ds &v1, const Vertex3Ds &v2, float y)
   {
      if (Vertex3Ds pt = intersect3D(v1, v2, y); pt.x != FLT_MAX)
         submitVertex(pt);
   };

   IEditable *upperEditableX = nullptr;
   IEditable *upperEditableY = nullptr;
   for (IEditable *edit : m_vedit)
   {
      if (edit->GetPartGroup() != nullptr && edit->GetPartGroup()->GetReferenceSpace() != PartGroupData::SpaceReference::SR_PLAYFIELD)
         continue;
      Vertex2D prevResult = result;
      switch (edit->GetItemType())
      {
      case eItemPrimitive:
         if (Primitive *const prim = static_cast<Primitive *>(edit); prim->m_d.m_use3DMesh && prim->m_d.m_visible)
         {
            const Matrix3D& modelMat = prim->RecalculateMatrices();
            for (size_t i = 0; i < prim->m_mesh.m_indices.size(); i+=3)
            {
               const Vertex3D_NoTex2 &va = prim->m_mesh.m_vertices[prim->m_mesh.m_indices[i + 0]];
               const Vertex3D_NoTex2 &vb = prim->m_mesh.m_vertices[prim->m_mesh.m_indices[i + 1]];
               const Vertex3D_NoTex2 &vc = prim->m_mesh.m_vertices[prim->m_mesh.m_indices[i + 2]];
               const Vertex3Ds a = modelMat * Vertex3Ds { va.x, va.y, va.z };
               const Vertex3Ds b = modelMat * Vertex3Ds { vb.x, vb.y, vb.z };
               const Vertex3Ds c = modelMat * Vertex3Ds { vc.x, vc.y, vc.z };
               submitEdge3D(a, b, m_top);
               submitEdge3D(b, c, m_top);
               submitEdge3D(c, a, m_top);
               submitEdge3D(a, b, m_bottom);
               submitEdge3D(b, c, m_bottom);
               submitEdge3D(c, a, m_bottom);
               // This would be easier, but this miss most of the inersection points
               //for (const Vertex3D_NoTex2 &v : prim->m_mesh.m_vertices)
               //   submitVertex(modelMat * Vertex3Ds { v.x, v.y, v.z });
            }
         }
         break;

      case eItemSurface:
         if (const Surface *const surf = static_cast<Surface *>(edit); surf->m_d.m_visible)
         {
            vector<RenderVertex> vertices;
            surf->GetRgVertex(vertices, true);
            const float h = max(surf->m_d.m_heightbottom, surf->m_d.m_heighttop);
            RenderVertex prev = vertices.back();
            for (const auto &v : vertices)
            {
               submitEdge2D(prev, v, m_bottom, h);
               submitEdge2D(prev, v, m_top, h);
               prev = v;
            }
         }
         break;

      case eItemRamp:
         if (const Ramp *const ramp = static_cast<Ramp *>(edit); ramp->m_d.m_visible)
         {
            vector<RenderVertex> vertices;
            ramp->GetRgVertex(vertices, false);
            RenderVertex prev = vertices.back();
            bool first = true; // Skip first as we do not loop
            for (const auto &v : vertices)
            {
               if (!first)
               {
                  if (Vertex2D pt = intersect2D(prev, v, m_bottom); pt.x != FLT_MAX)
                     submitVertex(Vertex3Ds(pt.x, pt.y, ramp->GetSurfaceHeight(pt.x, pt.y)));
                  if (Vertex2D pt = intersect2D(prev, v, m_top); pt.x != FLT_MAX)
                     submitVertex(Vertex3Ds(pt.x, pt.y, ramp->GetSurfaceHeight(pt.x, pt.y)));
               }
               first = false;
               prev = v;
            }
         }
         break;

      default:
         // Other parts are not considered
         continue;
      }
      if (prevResult.x != result.x)
         upperEditableX = edit;
      if (prevResult.y != result.y)
         upperEditableY = edit;
   }
   if (upperEditableX && upperEditableY)
   {
      PLOGI << "Evaluated glass height to " << VPUTOINCHES(result.x) << "\" (" << upperEditableX->GetName() << ") - " << VPUTOINCHES(result.y) << "\" (" << upperEditableY->GetName() << ')';
   }
   return result;
}

void PinTable::LockElements()
{
   BeginUndo();
   const bool lock = !FMutilSelLocked();
   for (int i = 0; i < m_vmultisel.size(); i++)
   {
      ISelect * const psel = m_vmultisel.ElementAt(i);
      if (psel)
      {
         IEditable * const pedit = psel->GetIEditable();
         if (pedit)
         {
            pedit->MarkForUndo();
            psel->m_locked = lock;
         }
      }
   }
   EndUndo();
   SetDirtyDraw();
}

LRESULT PinTable::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
#ifndef __STANDALONE__
    switch (uMsg)
    {
        case WM_SETCURSOR:
            SetMouseCursor();
            return FinalWindowProc(uMsg, wParam, lParam);
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
                case SB_LINELEFT:
                    m_offset.x -= si.nPage / 10;
                    break;
                case SB_LINERIGHT:
                    m_offset.x += si.nPage / 10;
                    break;
                case SB_PAGELEFT:
                    m_offset.x -= si.nPage / 2;
                    break;
                case SB_PAGERIGHT:
                    m_offset.x += si.nPage / 2;
                    break;
                case SB_THUMBTRACK:
                {
                    const int delta = (int)(m_offset.x - (float)si.nPos);
                    m_offset.x = (float)((short)HIWORD(wParam) + delta);
                    break;
                }
            }
            SetDirtyDraw();
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
                case SB_LINEUP:
                    m_offset.y -= si.nPage / 10;
                    break;
                case SB_LINEDOWN:
                    m_offset.y += si.nPage / 10;
                    break;
                case SB_PAGEUP:
                    m_offset.y -= si.nPage / 2;
                    break;
                case SB_PAGEDOWN:
                    m_offset.y += si.nPage / 2;
                    break;
                case SB_THUMBTRACK:
                {
                    const int delta = (int)(m_offset.y - (float)si.nPos);
                    m_offset.y = (float)((short)HIWORD(wParam) + delta);
                    break;
                }
            }
            SetDirtyDraw();
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
                m_vpinball->SetActionCur(""s);
            }
            else
            {
                m_vpinball->SetActionCur("Autosave Failed"s);
            }
            BeginAutoSaveCounter();
            const HANDLE hEvent = (HANDLE)wParam;
            RemoveFromVectorSingle(m_vAsyncHandles, hEvent);
            CloseHandle(hEvent);
            return FinalWindowProc(uMsg, wParam, lParam);
        }
        default:
            break;
    }
    return WndProcDefault(uMsg, wParam, lParam);
#else
   return 0L;
#endif
}

void PinTable::FlipY(const Vertex2D& pvCenter)
{
   BeginUndo();
   for (int i = 0; i < m_vmultisel.size(); i++)
      m_vmultisel[i].FlipY(pvCenter);
   EndUndo();
}

void PinTable::FlipX(const Vertex2D& pvCenter)
{
   BeginUndo();
   for (int i = 0; i < m_vmultisel.size(); i++)
      m_vmultisel[i].FlipX(pvCenter);
   EndUndo();
}

void PinTable::Rotate(const float ang, const Vertex2D& pvCenter, const bool useElementCenter)
{
   BeginUndo();
   for (int i = 0; i < m_vmultisel.size(); i++)
      m_vmultisel[i].Rotate(ang, pvCenter, useElementCenter);
   EndUndo();
}

void PinTable::Scale(const float scalex, const float scaley, const Vertex2D& pvCenter, const bool useElementCenter)
{
   BeginUndo();
   for (int i = 0; i < m_vmultisel.size(); i++)
      m_vmultisel[i].Scale(scalex, scaley, pvCenter, useElementCenter);
   EndUndo();
}

void PinTable::Translate(const Vertex2D &pvOffset)
{
   BeginUndo();
   for (int i = 0; i < m_vmultisel.size(); i++)
      m_vmultisel[i].Translate(pvOffset);
   EndUndo();
}

Vertex2D PinTable::GetCenter() const
{
   float minx = FLT_MAX;
   float maxx = -FLT_MAX;
   float miny = FLT_MAX;
   float maxy = -FLT_MAX;

   for (int i = 0; i < m_vmultisel.size(); i++)
   {
      const ISelect * const psel = m_vmultisel.ElementAt(i);
      const Vertex2D vCenter = psel->GetCenter();

      minx = min(minx, vCenter.x);
      maxx = max(maxx, vCenter.x);
      miny = min(miny, vCenter.y);
      maxy = max(maxy, vCenter.y);
      //tx += m_vdpoint[i]->m_v.x;
      //ty += m_vdpoint[i]->m_v.y;
   }

   return {(maxx + minx)*0.5f, (maxy + miny)*0.5f};
}

void PinTable::PutCenter(const Vertex2D& pv)
{
}

void PinTable::OnRightButtonUp(int x, int y)
{
#ifndef __STANDALONE__
   GetSelectedItem()->OnRButtonUp(x, y);

   const int ks = GetKeyState(VK_CONTROL);

   // Only bring up context menu if we weren't in magnify mode
   if (!((m_vpinball->m_ToolCur == ID_TABLE_MAGNIFY) || (ks & 0x80000000)))
   {
      if (m_vmultisel.size() > 1)
      {
         DoContextMenu(x, y, IDR_MULTIMENU, this);
      }
      else if (!MultiSelIsEmpty())
      {
         DoContextMenu(x, y, GetSelectedItem()->m_menuid, GetSelectedItem());
      }
      else
      {
         DoContextMenu(x, y, IDR_TABLEMENU, this);
      }
   }
#endif
}

void PinTable::DoMouseMove(int x, int y)
{
#ifndef __STANDALONE__
   const Vertex2D v = TransformPoint(x, y);

   m_vpinball->SetPosCur(v.x, v.y);

   if (!m_dragging) // Not doing band select
   {
       for (int i = 0; i < m_vmultisel.size(); i++)
         m_vmultisel[i].OnMouseMove(x, y);
   }
   else
      OnMouseMove(x, y);
#endif
}

void PinTable::OnLeftDoubleClick(int x, int y)
{
   //::SendMessage(m_vpinball->m_hwnd, WM_SIZE, 0, 0);
}

void PinTable::ExportBlueprint()
{
#ifndef __STANDALONE__
   //bool saveAs = true;
   //if (saveAs)
   //{
      //need to get a file name
      OPENFILENAME ofn = {};
      ofn.lStructSize = sizeof(OPENFILENAME);
      ofn.hInstance = m_vpinball->theInstance;
      ofn.hwndOwner = m_vpinball->GetHwnd();
      ofn.lpstrFilter = "PNG (.png)\0*.png;\0Bitmap (.bmp)\0*.bmp;\0TGA (.tga)\0*.tga;\0TIFF (.tiff/.tif)\0*.tiff;*.tif;\0WEBP (.webp)\0*.webp;\0";
      char szBlueprintFileName[MAXSTRING];
      strncpy_s(szBlueprintFileName, sizeof(szBlueprintFileName), m_filename.c_str());
      const size_t idx = m_filename.find_last_of('.');
      if (idx != string::npos && idx < MAXSTRING)
          szBlueprintFileName[idx] = '\0';
      ofn.lpstrFile = szBlueprintFileName;
      ofn.nMaxFile = sizeof(szBlueprintFileName);
      ofn.lpstrDefExt = "png";
      ofn.Flags = OFN_NOREADONLYRETURN | OFN_CREATEPROMPT | OFN_OVERWRITEPROMPT | OFN_EXPLORER;

      const int ret = GetSaveFileName(&ofn);

      // user cancelled
      if (ret == 0)
         return;// S_FALSE;
   //}

   const int result = m_vpinball->MessageBox("Do you want a solid blueprint?", "Export As Solid?", MB_YESNO);
   const bool solid = (result == IDYES);

   float tableheight, tablewidth;
   if (m_vpinball->m_backglassView)
   {
      tablewidth = (float)EDITOR_BG_WIDTH;
      tableheight = (float)EDITOR_BG_HEIGHT;
   }
   else
   {
      tablewidth = m_right - m_left;
      tableheight = m_bottom - m_top;
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
   bmi.bmiHeader.biSizeImage = totallinebytes*bmheight;
#if 0
   WriteFile(hfile, &bmi, sizeof(BITMAPINFOHEADER), &foo, nullptr);
#endif

   CDC dc;
   dc.CreateCompatibleDC(nullptr);
   char *pbits;
   dc.CreateDIBSection(dc.GetHDC(), &bmi, DIB_RGB_COLORS, (void **)&pbits, nullptr, 0);

   {
   PaintSur psur(dc.GetHDC(), (float)bmwidth / tablewidth, tablewidth*0.5f, tableheight*0.5f, bmwidth, bmheight, nullptr);

   dc.SelectObject(static_cast<HBRUSH>(dc.GetStockObject(WHITE_BRUSH)));
   dc.PatBlt(0, 0, bmwidth, bmheight, PATCOPY);

   if (m_vpinball->m_backglassView)
      Render3DProjection(&psur);

   for(const auto &ptr : m_vedit)
   {
      if (ptr->GetISelect()->m_isVisible && ptr->m_backglass == m_vpinball->m_backglassView)
         ptr->RenderBlueprint(&psur, solid);
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
   FIBITMAP * dib = FreeImage_Allocate(bmwidth, bmheight, 24);
   BYTE * const pdst = FreeImage_GetBits(dib);
   //const unsigned int pitch_dst = FreeImage_GetPitch(dib); //!! necessary?
   memcpy(pdst, pbits, (size_t)bmwidth*bmheight * 3);
   if (!FreeImage_Save(FreeImage_GetFIFFromFilename(szBlueprintFileName), dib, szBlueprintFileName, PNG_Z_BEST_COMPRESSION | BMP_SAVE_RLE))
       m_vpinball->MessageBox("Export failed!", "Blueprint Export", MB_OK | MB_ICONEXCLAMATION);
   else
#endif
       m_vpinball->MessageBox("Export finished!", "Blueprint Export", MB_OK);
#if 1
   FreeImage_Unload(dib);
#endif
#endif
}

void PinTable::ExportMesh(ObjLoader& loader)
{
   const string name = MakeString(m_wzName);

   Vertex3D_NoTex2 rgv[7];
   rgv[0].x = m_left;     rgv[0].y = m_top;      rgv[0].z = 0.f;
   rgv[1].x = m_right;    rgv[1].y = m_top;      rgv[1].z = 0.f;
   rgv[2].x = m_right;    rgv[2].y = m_bottom;   rgv[2].z = 0.f;
   rgv[3].x = m_left;     rgv[3].y = m_bottom;   rgv[3].z = 0.f;

   // These next 4 vertices are used just to set the extents
   rgv[4].x = m_left;     rgv[4].y = m_top;      rgv[4].z = 50.0f;
   rgv[5].x = m_left;     rgv[5].y = m_bottom;   rgv[5].z = 50.0f;
   rgv[6].x = m_right;    rgv[6].y = m_bottom;   rgv[6].z = 50.0f;
   //rgv[7].x = m_right;    rgv[7].y = m_top;      rgv[7].z = 50.0f;

   for (int i = 0; i < 4; ++i)
   {
      rgv[i].nx = 0;
      rgv[i].ny = 0;
      rgv[i].nz = 1.0f;

      rgv[i].tv = (i & 2) ? 1.0f : 0.f;
      rgv[i].tu = (i == 1 || i == 2) ? 1.0f : 0.f;
   }

   static constexpr WORD playfieldPolyIndices[10] = { 0, 1, 3, 0, 3, 2, 2, 3, 5, 6 };

   Vertex3D_NoTex2 buffer[4 + 7];
   unsigned int offs = 0;
   for (unsigned int y = 0; y <= 1; ++y)
      for (unsigned int x = 0; x <= 1; ++x, ++offs)
      {
         buffer[offs].x = (x & 1) ? rgv[1].x : rgv[0].x;
         buffer[offs].y = (y & 1) ? rgv[2].y : rgv[0].y;
         buffer[offs].z = rgv[0].z;

         buffer[offs].tu = (x & 1) ? rgv[1].tu : rgv[0].tu;
         buffer[offs].tv = (y & 1) ? rgv[2].tv : rgv[0].tv;

         buffer[offs].nx = rgv[0].nx;
         buffer[offs].ny = rgv[0].ny;
         buffer[offs].nz = rgv[0].nz;
      }

   SetNormal(rgv, playfieldPolyIndices + 6, 4);

   loader.WriteObjectName(name);
   loader.WriteVertexInfo(buffer, 4);
   const Material * const mat = GetMaterial(m_playfieldMaterial);
   loader.WriteMaterial(m_playfieldMaterial, string(), mat);
   loader.UseTexture(m_playfieldMaterial);
   loader.WriteFaceInfoList(playfieldPolyIndices, 6);
   loader.UpdateFaceOffset(4);
}

void PinTable::ExportTableMesh()
{
#ifndef __STANDALONE__
   char szObjFileName[MAXSTRING];
   strncpy_s(szObjFileName, sizeof(szObjFileName), m_filename.c_str());
   const size_t idx = m_filename.find_last_of('.');
   if (idx != string::npos && idx < MAXSTRING)
      szObjFileName[idx] = '\0';
   OPENFILENAME ofn = {};
   ofn.lStructSize = sizeof(OPENFILENAME);
   ofn.hInstance = m_vpinball->theInstance;
   ofn.hwndOwner = m_vpinball->GetHwnd();
   // TEXT
   ofn.lpstrFilter = "Wavefront obj(*.obj)\0*.obj\0";
   ofn.lpstrFile = szObjFileName;
   ofn.nMaxFile = sizeof(szObjFileName);
   ofn.lpstrDefExt = "obj";
   ofn.Flags = OFN_NOREADONLYRETURN | OFN_CREATEPROMPT | OFN_OVERWRITEPROMPT | OFN_EXPLORER;

   const int ret = GetSaveFileName(&ofn);

   // user canceled
   if (ret == 0)
      return;// S_FALSE;
   const string filename = string(szObjFileName);

   ObjLoader loader;
   loader.ExportStart(filename);
   ExportMesh(loader);
   for (size_t i = 0; i < m_vedit.size(); i++)
   {
      IEditable * const ptr = m_vedit[i];
      if (ptr->GetISelect()->m_isVisible && ptr->m_backglass == m_vpinball->m_backglassView)
         ptr->ExportMesh(loader);
   }
   loader.ExportEnd();
   m_vpinball->MessageBox("Export finished!", "Info", MB_OK | MB_ICONEXCLAMATION);
#endif
}

// Import Point of View file. This can be either:
// - a UI interaction from table author, loading to table **properties** after file selection,
// - without UI interaction, triggered to load user settings preference to table **settings**.
void PinTable::ImportBackdropPOV(const string &filename)
{
   string file = filename;
   const bool toUserSettings = !filename.empty();
   const bool wasModified = m_settings.IsModified();
   if (!toUserSettings)
   {
      if (IsLocked())
         return;
#ifndef __STANDALONE__
      const string& initialDir = m_settings.GetRecentDir_POVDir();
      vector<string> fileNames;
      if (!m_vpinball->OpenFileDialog(initialDir, fileNames, 
         "User settings file (*.ini)\0*.ini\0Old POV file (*.pov)\0*.pov\0Legacy POV file(*.xml)\0*.xml\0",
         "ini", 0, toUserSettings ? "Import POV to user settings"s : "Import POV to table properties"s))
         return;
      file = fileNames[0];
      const size_t index = file.find_last_of(PATH_SEPARATOR_CHAR);
      if (index != string::npos)
         g_pvp->m_settings.SetRecentDir_POVDir(file.substr(0, index), false);
#endif
   }

   const string ext = lowerCase(ExtensionFromFilename(file));

   static const string vsPrefix[3] = { "ViewDT"s, "ViewCab"s, "ViewFSS"s };
   static const char *vsFields[15] = { "Mode", "ScaleX", "ScaleY", "ScaleZ", "PlayerX", "PlayerY", "PlayerZ", "LookAt", "Rotation", "FOV", "Layback", "HOfs", "VOfs", "WindowTop", "WindowBot" };
   if (ext == "ini")
   {
      Settings settings;
      settings.SetIniPath(file);
      settings.Load(false);
      for (int id = 0; id < 3; id++)
      {
         const string &keyPrefix = vsPrefix[id];
         if (toUserSettings)
         {
            ViewSetup vs;
            vs.ApplyTableOverrideSettings(settings, (ViewSetupID)id);
            vs.SaveToTableOverrideSettings(settings, (ViewSetupID)id);
         }
         else
            mViewSetups[id].ApplyTableOverrideSettings(settings, (ViewSetupID)id);
      }
   }
   else if (ext == "pov" || ext == "xml")
   {
      tinyxml2::XMLDocument xmlDoc;
      try
      {
         std::stringstream buffer;
         std::ifstream myFile(file);
         buffer << myFile.rdbuf();
         myFile.close();
         const string xml = buffer.str();
         if (xmlDoc.Parse(xml.c_str()))
         {
            ShowError("Error parsing POV XML file");
            return;
         }
         auto root = xmlDoc.FirstChildElement("POV");
         if (root == nullptr)
         {
            ShowError("Error parsing POV XML file: root 'POV' element is missing");
            xmlDoc.Clear();
            return;
         }
         #define POV_FIELD(name, type, settingField, fieldtype, field) \
         { \
            const auto node = section->FirstChildElement(name); \
            if (node != nullptr) \
            { \
               const char * const t = node->GetText(); \
               if (t) \
               { \
               type value; \
               if(my_from_chars(t,t+strlen(t),value).ec == std::errc{}) \
               { \
               if (toUserSettings) \
                  m_settings.Set(Settings::GetRegistry().GetPropertyId("TableOverride"s, keyPrefix + (settingField)).value(), value, true); \
               else \
                  field = (fieldtype)value; \
               } \
               } \
            } \
         }
         static const string sections[] = { "desktop"s, "fullscreen"s, "fullsinglescreen"s };
         for (int i = 0; i < 3; i++)
         {
            const auto section = root->FirstChildElement(sections[i].c_str());
            if (section)
            {
               const string &keyPrefix = vsPrefix[i];
               POV_FIELD("inclination", float, "LookAt", float, mViewSetups[i].mLookAt);
               POV_FIELD("fov", float, "FOV", float, mViewSetups[i].mFOV);
               POV_FIELD("layback", float, "Layback", float, mViewSetups[i].mLayback);
               POV_FIELD("lookat", float, "LookAt", float, mViewSetups[i].mLookAt);
               POV_FIELD("rotation", float, "Rotation", float, mViewSetups[i].mViewportRotation);
               POV_FIELD("xscale", float, "ScaleX", float, mViewSetups[i].mSceneScaleX);
               POV_FIELD("yscale", float, "ScaleY", float, mViewSetups[i].mSceneScaleY);
               POV_FIELD("zscale", float, "ScaleZ", float, mViewSetups[i].mSceneScaleZ);
               POV_FIELD("xoffset", float, "PlayerX", float, mViewSetups[i].mViewX);
               POV_FIELD("yoffset", float, "PlayerY", float, mViewSetups[i].mViewY);
               POV_FIELD("zoffset", float, "PlayerZ", float, mViewSetups[i].mViewZ);
               POV_FIELD("ViewHOfs", float, "HOfs", float, mViewSetups[i].mViewHOfs);
               POV_FIELD("ViewVOfs", float, "VOfs", float, mViewSetups[i].mViewVOfs);
               POV_FIELD("WindowTopZOfs", float, "WindowTop", float, mViewSetups[i].mWindowTopZOfs);
               POV_FIELD("WindowBottomZOfs", float, "WindowBot", float, mViewSetups[i].mWindowBottomZOfs);
               POV_FIELD("LayoutMode", int, "Mode", ViewLayoutMode, mViewSetups[i].mMode);
            }
         }
         #undef POV_FIELD
         if (toUserSettings)
         {
            const auto section = root->FirstChildElement("customsettings");
            if (section)
            {
               #define POV_FIELD(name, type, savecondition) \
               { \
                  const auto node = section->FirstChildElement(name); \
                  if (node != nullptr) \
                  { \
                     const char * const t = node->GetText(); \
                     if (t) \
                     { \
                     type value; \
                     if(my_from_chars(t,t+strlen(t),value).ec == std::errc{}) \
                     { \
                     savecondition; \
                     } \
                     } \
                  } \
               }

               POV_FIELD("postprocAA", int, if (value > -1) m_settings.SetPlayer_AAFactor(value == 0 ? 1.f : 2.f, true)); // remap to new AA
               POV_FIELD("postprocAA", int, if(value > -1) m_settings.SetPlayer_FXAA(value == 1 ? Standard_FXAA : Disabled, true));
               POV_FIELD("ingameAO", int, if(value != -1) m_settings.SetPlayer_DisableAO(value == 0, true));
               POV_FIELD("ScSpReflect", int, if(value != -1) m_settings.SetPlayer_SSRefl(value != 0, true));
               //POV_FIELD("FPSLimiter", int, tableAdaptiveVSync, );
               {
               const auto node = section->FirstChildElement("FPSLimiter");
               if (node)
               {
                  int tableAdaptiveVSync;
                  sscanf_s(node->GetText(), "%i", &tableAdaptiveVSync);
                  if (tableAdaptiveVSync != -1)
                  {
                     switch (tableAdaptiveVSync)
                     {
                     case 0:
                        m_settings.SetPlayer_MaxFramerate(0, true);
                        m_settings.SetPlayer_SyncMode(VideoSyncMode::VSM_NONE, true);
                        break;
                     case 1:
                        m_settings.SetPlayer_MaxFramerate(-1, true);
                        m_settings.SetPlayer_SyncMode(VideoSyncMode::VSM_VSYNC, true);
                        break;
                     case 2:
                        m_settings.SetPlayer_MaxFramerate(-1, true);
                        m_settings.SetPlayer_SyncMode(VideoSyncMode::VSM_ADAPTIVE_VSYNC, true);
                        break;
                     default:
                        m_settings.SetPlayer_MaxFramerate(static_cast<float>(tableAdaptiveVSync), true);
                        m_settings.SetPlayer_SyncMode(VideoSyncMode::VSM_ADAPTIVE_VSYNC, true);
                        break;
                     }
                  }
               }
               }
               POV_FIELD("BallTrail", int, if(value != -1) m_settings.SetPlayer_BallTrail(value == 1, true));
               POV_FIELD("BallTrailStrength", float, m_settings.SetPlayer_BallTrailStrength(value, true));
               //int overwriteGlobalDetailLevel = (int)m_overwriteGlobalDetailLevel;
               //POV_FIELD("OverwriteDetailsLevel", "%i", overwriteGlobalDetailLevel);
               {
               const auto node = section->FirstChildElement("OverwriteDetailsLevel");
               if (node)
               {
                  int val;
                  sscanf_s(node->GetText(), "%i", &val);
                  if (val == 1)
                     POV_FIELD("DetailsLevel", int, m_settings.SetPlayer_AlphaRampAccuracy(value, true));
               }
               }
               {
               const auto node2 = section->FirstChildElement("OverwriteNightDay");
               if (node2)
               {
                  int val;
                  sscanf_s(node2->GetText(), "%i", &val);
                  //m_overwriteGlobalDayNight = (val == 1);
                  if (val == 1)
                  {
                     m_settings.SetPlayer_OverrideTableEmissionScale(true, true);
                     POV_FIELD("NightDayLevel", float, m_settings.SetPlayer_EmissionScale(value / 100.f, true));
                  }
               }
               }
               POV_FIELD("GameplayDifficulty", float, m_settings.SetTableOverride_Difficulty(value / 100.f, true));
               POV_FIELD("SoundVolume", int, m_settings.SetPlayer_SoundVolume(value, true));
               POV_FIELD("MusicVolume", int, m_settings.SetPlayer_MusicVolume(value, true));
               // FIXME these are the last 3 settings which were not ported to the setting API
               // - for physics set, since they can be applied at the part level, for each flipper
               // - for ball reflection, since I don't think that matters and there is no obvious way
               //POV_FIELD("BallReflection", "%i", m_useReflectionForBalls); // removed in 10.8
               /* node = section->FirstChildElement("PhysicsSet");
               if (node)
                  sscanf_s(node->GetText(), "%i", &m_overridePhysics);
               node = section->FirstChildElement("IncludeFlipperPhysics");
               if (node)
               {
                  int value;
                  sscanf_s(node->GetText(), "%i", &value);
                  m_overridePhysicsFlipper = (value == 1);
               } */
               #undef POV_FIELD
            }
         }
      }
      catch (...)
      {
         ShowError("Error parsing POV XML file");
      }
      xmlDoc.Clear();
   }

   // If loaded without UI interaction, do not mark settings as modified
   if (!filename.empty())
      m_settings.SetModified(wasModified);

   // update properties UI
   if (!toUserSettings)
      SetNonUndoableDirty(eSaveDirty);
   m_vpinball->SetPropSel(m_vmultisel);
}

// Select file and export the point of view definition
void PinTable::ExportBackdropPOV() const
{
   string iniFileName;
#ifndef __STANDALONE__
	OPENFILENAME ofn = {};
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hInstance = m_vpinball->theInstance;
	ofn.hwndOwner = m_vpinball->GetHwnd();
	// TEXT
	ofn.lpstrFilter = "INI file(*.ini)\0*.ini\0";
	char szFileName[MAXSTRING];
	strncpy_s(szFileName, sizeof(szFileName), m_filename.c_str());
	const size_t idx = m_filename.find_last_of('.');
	if(idx != string::npos && idx < MAXSTRING)
		szFileName[idx] = '\0';
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = sizeof(szFileName);
	ofn.lpstrDefExt = "ini";
	ofn.Flags = OFN_NOREADONLYRETURN | OFN_CREATEPROMPT | OFN_OVERWRITEPROMPT | OFN_EXPLORER;
	const int ret = GetSaveFileName(&ofn);
	// user canceled
	if (ret == 0)
		return;// S_FALSE;
	iniFileName = szFileName;
#endif

   // Save view setups (only overriden properties if we are given a reference view setup set)
   Settings settings;
   for (int i = 0; i < 3; i++)
      mViewSetups[i].SaveToTableOverrideSettings(settings, (ViewSetupID)i);
   if (settings.IsModified())
   {
      settings.SetIniPath(iniFileName);
      settings.Save();
      if (g_pplayer)
         g_pplayer->m_liveUI->PushNotification("POV exported to "s.append(iniFileName), 5000);
   }
   else if (g_pplayer)
   {
      g_pplayer->m_liveUI->PushNotification("POV was not exported to " + iniFileName + " (nothing to save)", 5000);
   }

   PLOGI << "View setup exported to '" << iniFileName << '\'';
}

void PinTable::SelectItem(IScriptable *piscript)
{
   ISelect * const pisel = piscript->GetISelect();
   if (pisel)
      AddMultiSel(pisel, false, true, false);
}

void PinTable::DoCodeViewCommand(int command)
{
   g_pvp->ParseCommand(command, false);
}

void PinTable::SetDirtyScript(SaveDirtyState sds)
{
   m_sdsDirtyScript = sds;
   CheckDirty();
}

void PinTable::SetDirty(SaveDirtyState sds)
{
   m_sdsDirtyProp = sds;
   CheckDirty();
}

void PinTable::SetNonUndoableDirty(SaveDirtyState sds)
{
   m_sdsNonUndoableDirty = sds;
   CheckDirty();
}

void PinTable::CheckDirty()
{
   const SaveDirtyState sdsNewDirtyState = (SaveDirtyState)max(max((int)m_sdsDirtyProp, (int)m_sdsDirtyScript), (int)m_sdsNonUndoableDirty);

   if (sdsNewDirtyState != m_sdsCurrentDirtyState)
   {
      if (sdsNewDirtyState > eSaveClean)
         SetCaption(m_title + '*');
      else
         SetCaption(m_title);
   }

   m_sdsCurrentDirtyState = sdsNewDirtyState;
}

bool PinTable::FDirty() const
{
   return (m_sdsCurrentDirtyState > eSaveClean);
}

void PinTable::BeginUndo()
{
   m_undo.BeginUndo();
}

void PinTable::EndUndo()
{
   m_undo.EndUndo();
}

void PinTable::Undo()
{
   m_undo.Undo();

   SetDirtyDraw();
   SetMyScrollInfo();

#ifndef __STANDALONE__
   if (m_searchSelectDlg.IsWindow())
      m_searchSelectDlg.Update();
   m_vpinball->GetLayersListDialog()->Update();
#endif
}

void PinTable::Uncreate(IEditable *pie)
{
   if (pie->GetISelect()->m_selectstate != eNotSelected)
      AddMultiSel(pie->GetISelect(), true, true, false); // Remove the item from the multi-select list

   pie->GetISelect()->Uncreate();
   pie->Release();
}

void PinTable::Undelete(IEditable *pie)
{
   m_vedit.push_back(pie);
   pie->Undelete();
   SetDirtyDraw();
}

void PinTable::Copy(int x, int y)
{
#ifndef __STANDALONE__
   if (MultiSelIsEmpty()) // Can't copy table
      return;

   if (m_vmultisel.size() == 1)
   {
       // special check if the user selected a Control Point and wants to copy the coordinates
       ISelect *const pItem = HitTest(x, y);
       if (pItem->GetItemType() == eItemDragPoint)
       {
           DragPoint *pPoint = (DragPoint*)pItem;
           pPoint->Copy();
           return;
       }
   }

   vector<IStream*> vstm;
   //m_vstmclipboard
   for (int i = 0; i < m_vmultisel.size(); i++)
   {
       const HGLOBAL hglobal = GlobalAlloc(GMEM_MOVEABLE, 1);

       IStream *pstm;
       CreateStreamOnHGlobal(hglobal, TRUE, &pstm);

       IEditable * const pe = m_vmultisel[i].GetIEditable();

       ////////!! BUG!  With multi-select, if you have multiple dragpoints on
       //////// a surface selected, the surface will get copied multiple times
       const int type = pe->GetItemType();
       ULONG writ = 0;
       pstm->Write(&type, sizeof(int), &writ);

       pe->SaveData(pstm, NULL, false);

       vstm.push_back(pstm);
   }

   m_vpinball->SetClipboard(&vstm);
#endif
}

void PinTable::Paste(const bool atLocation, const int x, const int y)
{
#ifndef __STANDALONE__
   bool error = false;
   int cpasted = 0;

   if (m_vmultisel.size() == 1)
   {
       // User wants to paste the copied coordinates of a Control Point
       ISelect * const pItem = HitTest(x, y);
       if (pItem->GetItemType() == eItemDragPoint)
       {
           DragPoint * const pPoint = (DragPoint*)pItem;
           pPoint->Paste();
           SetDirtyDraw();
           return;
       }
   }

   const unsigned viewflag = (m_vpinball->m_backglassView ? VIEW_BACKGLASS : VIEW_PLAYFIELD);

   // Do a backwards loop, so that the primary selection we had when
   // copying will again be the primary selection, since it will be
   // selected last.  Purely cosmetic.
   for (SSIZE_T i = m_vpinball->m_vstmclipboard.size() - 1; i >= 0; i--)
   //for (size_t i=0; i<m_vpinball->m_vstmclipboard.size(); i++)
   {
      IStream* const pstm = m_vpinball->m_vstmclipboard[i];

      // Go back to beginning of stream to load
      LARGE_INTEGER foo;
      foo.QuadPart = 0;
      pstm->Seek(foo, STREAM_SEEK_SET, nullptr);

      ULONG writ = 0;
      ItemTypeEnum type;
      /*const HRESULT hr =*/ pstm->Read(&type, sizeof(int), &writ);

      if (!(EditableRegistry::GetAllowedViews(type) & viewflag))
      {
         error = true;
      }
      else
      {
         IEditable* const peditNew = EditableRegistry::Create(type);
         if (peditNew)
         {
            peditNew->InitLoad(pstm, this, CURRENT_FILE_FORMAT_VERSION, NULL, NULL);

            if (type != eItemDecal)
            {
               GetUniqueNamePasting(type, peditNew->GetScriptable()->m_wzName, std::size(peditNew->GetScriptable()->m_wzName));
               peditNew->InitVBA(true, peditNew->GetScriptable()->m_wzName);
            }

            peditNew->InitPostLoad();
            peditNew->m_backglass = m_vpinball->m_backglassView;

            peditNew->SetPartGroup(m_vpinball->GetLayersListDialog()->GetSelectedPartGroup());

            m_vedit.push_back(peditNew);

            AddMultiSel(peditNew->GetISelect(), (i != m_vpinball->m_vstmclipboard.size() - 1), true, false);
            cpasted++;
         }
         else
            error = true;
      }
   }
   m_vpinball->GetLayersListDialog()->Update();

   // Center view on newly created objects, if they are off the screen
   if ((cpasted > 0) && atLocation)
      Translate(TransformPoint(x, y) - GetCenter());

   if (error)
      m_mdiTable->MessageBox(LocalString(IDS_NOPASTEINVIEW).m_szbuffer, "Visual Pinball", 0);
#endif
}

HRESULT PinTable::InitLoad(IStream *pstm, PinTable *ptable, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   SetDefaults(false);

   memset(m_loadTemp, 0, sizeof(m_loadTemp));

   LoadData(pstm, version, hcrypthash, hcryptkey);

   return S_OK;
}

void PinTable::SetDefaultPhysics(const bool fromMouseClick)
{
   m_Gravity = 0.97f*GRAVITYCONST;

   m_friction = DEFAULT_TABLE_CONTACTFRICTION;
   m_elasticity = DEFAULT_TABLE_ELASTICITY;
   m_elasticityFalloff = DEFAULT_TABLE_ELASTICITY_FALLOFF;
   m_scatter = DEFAULT_TABLE_PFSCATTERANGLE;
}

void PinTable::ClearMultiSel(ISelect* newSel)
{
   for (int i = 0; i < m_vmultisel.size(); i++)
      m_vmultisel[i].m_selectstate = eNotSelected;

   //remove the clone of the multi selection in the smart browser class
   //to sync the clone and the actual multi-selection 
   //it will be updated again on AddMultiSel() call
   m_vmultisel.clear();

   if (newSel == nullptr)
      newSel = this;
   m_vmultisel.push_back(newSel);
   newSel->m_selectstate = eSelected;
}

bool PinTable::MultiSelIsEmpty() const
{
   // empty selection means only the table itself is selected
   return (m_vmultisel.size() == 1 && m_vmultisel.ElementAt(0) == this);
}

// 'update' tells us whether to go ahead and change the UI
// based on the new selection, or whether more stuff is coming
// down the pipe (speeds up drag-selection)
void PinTable::AddMultiSel(ISelect *psel, const bool add, const bool update, const bool contextClick)
{
   const int index = m_vmultisel.find(psel);
   ISelect *piSelect = nullptr;
   //_ASSERTE(m_vmultisel[0].m_selectstate == eSelected);

   if (IsLocked())
      return;

   if (index == -1) // If we aren't selected yet, do that
   {
      _ASSERTE(psel->m_selectstate == eNotSelected);
      // If we non-shift click on an element outside the multi-select group, delete the old group
      // If the table is currently selected, deselect it - the table can not be part of a multi-select
      if (!add || MultiSelIsEmpty())
      {
         ClearMultiSel(psel);
         if (!add && !contextClick)
         {
            int colIndex = -1;
            int elemIndex = -1;
            if (GetCollectionIndex(psel, colIndex, elemIndex))
            {
               CComObject<Collection> *col = m_vcollection.ElementAt(colIndex);
               if (col->m_groupElements)
               {
                  for (int i = 0; i < col->m_visel.size(); i++)
                  {
                     col->m_visel[i].m_selectstate = eMultiSelected;
                     // current element is already in m_vmultisel. (ClearMultiSel(psel) added it)
                     if (col->m_visel.ElementAt(i) != psel)
                        m_vmultisel.push_back(&col->m_visel[i]);
                  }
               }
            }
         }
      }
      else
      {
         // Make this new selection the primary one for the group
         piSelect = m_vmultisel.ElementAt(0);
         if (piSelect != nullptr)
            piSelect->m_selectstate = eMultiSelected;
         m_vmultisel.insert(psel, 0);
      }

      psel->m_selectstate = eSelected;

      if (update)
         SetDirtyDraw();
   }
   else if (add) // Take the element off the list
   {
      _ASSERTE(psel->m_selectstate != eNotSelected);
      m_vmultisel.erase(index);
      psel->m_selectstate = eNotSelected;
      if (m_vmultisel.empty())
      {
         // Have to have something selected
         m_vmultisel.push_back((ISelect *)this);
      }
      // The main element might have changed
      piSelect = m_vmultisel.ElementAt(0);
      if (piSelect != nullptr)
         piSelect->m_selectstate = eSelected;

      if (update)
         SetDirtyDraw();
   }
   else if (m_vmultisel.ElementAt(0) != psel) // Object already in list - no change to selection, only to primary
   {
      int colIndex = -1;
      int elemIndex = -1;
      if (!GetCollectionIndex(psel, colIndex, elemIndex))
      {
         _ASSERTE(psel->m_selectstate != eNotSelected);

         // Make this new selection the primary one for the group
         piSelect = m_vmultisel.ElementAt(0);
         if (piSelect != nullptr)
            piSelect->m_selectstate = eMultiSelected;
         m_vmultisel.erase(index);
         m_vmultisel.insert(psel, 0);

         psel->m_selectstate = eSelected;
      }
      else
         ClearMultiSel(psel);

      if (update)
         SetDirtyDraw();
   }

   if (update)
   {
#ifndef __STANDALONE__
       m_vpinball->SetPropSel(m_vmultisel);
#endif
       m_vmultisel[0].UpdateStatusBarInfo();
   }

    piSelect = m_vmultisel.ElementAt(0);
    if (piSelect && piSelect->GetIEditable() && piSelect->GetIEditable()->GetScriptable())
    {
        string info = piSelect->GetIEditable()->GetPathString(false);
        if (piSelect->GetItemType() == eItemPrimitive)
        {
            const Primitive *const prim = (Primitive *)piSelect;
            if (!prim->m_mesh.m_animationFrames.empty())
                info += " (animated " + std::to_string((uint32_t)prim->m_mesh.m_animationFrames.size() - 1) + " frames)";
        }
#ifndef __STANDALONE__
        m_vpinball->SetStatusBarElementInfo(info);
        m_pcv->SelectItem(piSelect->GetIEditable()->GetScriptable());
#endif
    }

#ifndef __STANDALONE__
   if (m_vpinball->GetLayersListDialog()->IsSyncedOnSelection())
      m_vpinball->GetLayersListDialog()->Update();
#endif
}

void PinTable::RefreshProperties()
{
#ifndef __STANDALONE__
   m_vpinball->SetPropSel(m_vmultisel);
#endif
}

void PinTable::OnDelete()
{
#ifndef __STANDALONE__
   vector<ISelect*> m_vseldelete;
   m_vseldelete.reserve(m_vmultisel.size());

   for (int i = 0; i < m_vmultisel.size(); i++)
   {
      // Can't delete these items yet - ClearMultiSel() will try to mark them as unselected
      m_vseldelete.push_back(m_vmultisel.ElementAt(i));
   }

   ClearMultiSel();

   bool inCollection = false;
   for (size_t t = 0; t < m_vseldelete.size() && !inCollection; t++)
   {
      const ISelect * const ptr = m_vseldelete[t];
      for (int i = 0; i < m_vcollection.size() && !inCollection; i++)
      {
         for (int k = 0; k < m_vcollection[i].m_visel.size(); k++)
         {
            // Identify Editable in collection, as well as sub part of collection's editable (like light center for example)
            if (ptr == m_vcollection[i].m_visel.ElementAt(k) || ptr->GetIEditable() == m_vcollection[i].m_visel.ElementAt(k)->GetIEditable())
            {
               inCollection = true;
               break;
            }
         }
      }
   }
   if (inCollection)
   {
      const int ans = m_mdiTable->MessageBox(LocalString(IDS_DELETE_ELEMENTS).m_szbuffer/*"Selected elements are part of one or more collections.\nDo you really want to delete them?"*/, "Visual Pinball", MB_YESNO | MB_DEFBUTTON2);
      if (ans != IDYES)
         return;
   }

   for (size_t i = 0; i < m_vseldelete.size(); i++)
      if (m_vseldelete[i] != nullptr)
         m_vseldelete[i]->Delete();
   m_vpinball->GetLayersListDialog()->Update();
   // update properties to show the properties of the table
   m_vpinball->SetPropSel(m_vmultisel);
   if (m_searchSelectDlg.IsWindow())
      m_searchSelectDlg.Update();

   SetDirtyDraw();
#endif
}

void PinTable::OnKeyDown(int key)
{
#ifndef __STANDALONE__
   const int shift = GetKeyState(VK_SHIFT) & 0x8000;
   //const int ctrl = GetKeyState(VK_CONTROL) & 0x8000;
   //const int alt = GetKeyState(VK_MENU) & 0x8000;

   switch (key)
   {
   case VK_DELETE: OnDelete(); break;

   case VK_LEFT:
   case VK_RIGHT:
   case VK_UP:
   case VK_DOWN:
   {
      BeginUndo();
      const float distance = shift ? 10.f : 1.f;
      for (int i = 0; i < m_vmultisel.size(); i++)
      {
         ISelect *const pisel = m_vmultisel.ElementAt(i);
         if (!pisel->GetIEditable()->GetISelect()->m_locked) // control points get lock info from parent - UNDONE - make this code snippet be in one place
         {
            switch (key)
            {
            case VK_LEFT:
               pisel->GetIEditable()->MarkForUndo();
               pisel->MoveOffset(-distance / m_zoom, 0);
               break;

            case VK_RIGHT:
               pisel->GetIEditable()->MarkForUndo();
               pisel->MoveOffset(distance / m_zoom, 0);
               break;

            case VK_UP:
               pisel->GetIEditable()->MarkForUndo();
               pisel->MoveOffset(0, -distance / m_zoom);
               break;

            case VK_DOWN:
               pisel->GetIEditable()->MarkForUndo();
               pisel->MoveOffset(0, distance / m_zoom);
               break;
            }
         }
      }
      EndUndo();
      SetDirtyDraw();
   }
   break;
   }
#endif
}

void PinTable::UseTool(int x, int y, int tool)
{
#ifndef __STANDALONE__
   const Vertex2D v = TransformPoint(x, y);

   const ItemTypeEnum type = EditableRegistry::TypeFromToolID(tool);
   IEditable * const pie = EditableRegistry::CreateAndInit(type, this, v.x, v.y);

   if (pie)
   {
      pie->m_backglass = m_vpinball->m_backglassView;
      m_vedit.push_back(pie);
      pie->SetPartGroup(m_vpinball->GetLayersListDialog()->GetSelectedPartGroup());
      m_vpinball->GetLayersListDialog()->Update();

      if (m_searchSelectDlg.IsWindow())
         m_searchSelectDlg.Update();

      BeginUndo();
      m_undo.MarkForCreate(pie);
      EndUndo();
      AddMultiSel(pie->GetISelect(), false, true, false);
   }

   m_vpinball->ParseCommand(IDC_SELECT, false);
#endif
}

Vertex2D PinTable::TransformPoint(int x, int y) const
{
#ifndef __STANDALONE__
   const CRect rc = GetClientRect();
#else
   const CRect rc(m_left, m_top, m_right, m_bottom);
#endif
   const HitSur phs(nullptr, m_zoom, m_offset.x, m_offset.y, rc.right - rc.left, rc.bottom - rc.top, 0, 0, nullptr);

   const Vertex2D result = phs.ScreenToSurface(x, y);

   return result;
}

void PinTable::OnLButtonDown(int x, int y)
{
#ifndef __STANDALONE__
   const Vertex2D v = TransformPoint(x, y);

   m_rcDragRect.left = v.x;
   m_rcDragRect.right = v.x;
   m_rcDragRect.top = v.y;
   m_rcDragRect.bottom = v.y;

   m_dragging = true;

   SetCapture();

   SetDirtyDraw();
#endif
}

void PinTable::OnLButtonUp(int x, int y)
{
#ifndef __STANDALONE__
   if (m_dragging)
   {
      m_dragging = false;
      ReleaseCapture();
      if ((m_rcDragRect.left != m_rcDragRect.right) || (m_rcDragRect.top != m_rcDragRect.bottom))
      {
         vector<ISelect*> vsel;

         const CDC &dc = m_mdiTable->GetDC();

         const CRect rc = m_mdiTable->GetClientRect();

         HitRectSur * const phrs = new HitRectSur(dc.GetHDC(), m_zoom, m_offset.x, m_offset.y, rc.right - rc.left, rc.bottom - rc.top, &m_rcDragRect, &vsel);

         // Just want one rendering pass (no UIRenderPass1) so we don't select things twice
         UIRenderPass2(phrs);

         const int ksshift = GetKeyState(VK_SHIFT);
         const bool add = ((ksshift & 0x80000000) != 0);
         if (!add)
            ClearMultiSel();

         int minlevel = INT_MAX;

         for(const auto &ptr : vsel)
            minlevel = min(minlevel, ptr->GetSelectLevel());

         if (!vsel.empty())
         {
            size_t lastItemForUpdate = -1;
            // first check which item is the last item to add to the multi selection
            for (size_t i = 0; i < vsel.size(); i++)
               if (vsel[i]->GetSelectLevel() == minlevel)
                  lastItemForUpdate = i;

            for (size_t i = 0; i < vsel.size(); i++)
               if (vsel[i]->GetSelectLevel() == minlevel)
                  AddMultiSel(vsel[i], true, (i == lastItemForUpdate), false); //last item updates the (multi-)selection in the editor
         }

         delete phrs;
      }
   }
   SetDirtyDraw();
#endif
}

void PinTable::OnMouseMove(int x, int y)
{
   const Vertex2D v = TransformPoint(x, y);

   m_rcDragRect.right = v.x;
   m_rcDragRect.bottom = v.y;

   if (m_dragging)
      SetDirtyDraw();
}

HRESULT PinTable::GetTypeName(BSTR *pVal) const
{
   const int stringid = (!m_vpinball->m_backglassView) ? IDS_TABLE : IDS_TB_BACKGLASS;
   *pVal = SysAllocString(LocalStringW(stringid).m_szbuffer);
   return S_OK;
}

STDMETHODIMP PinTable::get_FileName(BSTR *pVal)
{
   *pVal = MakeWideBSTR(m_title);
   return S_OK;
}

const WCHAR *PinTable::get_Name() const
{
   return m_wzName;
}

STDMETHODIMP PinTable::get_Name(BSTR *pVal)
{
   *pVal = SysAllocString(m_wzName);
   return S_OK;
}

STDMETHODIMP PinTable::put_Name(BSTR newVal)
{
   const wstring newName = newVal;
   if (newName.empty() || newName.length() >= std::size(m_wzName))
      return E_FAIL;

   STARTUNDO
   if (m_pcv->ReplaceName((IScriptable *)this, newName) == S_OK)
      wcsncpy_s(m_wzName, std::size(m_wzName), newVal);
   STOPUNDO

   return S_OK;
}

VPX::Sound *PinTable::GetSound(const string &name) const
{
   auto sound = std::ranges::find_if(m_vsound, [&](const VPX::Sound *const ps) { return StrCompareNoCase(ps->GetName(), name); });
   if (sound != m_vsound.end())
      return *sound;
   return nullptr;
}

STDMETHODIMP PinTable::PlaySound(BSTR soundName, int loopcount, float volume, float pan, float randompitch, int pitch, VARIANT_BOOL usesame, VARIANT_BOOL restart, float front_rear_fade)
{
   if (g_pplayer == nullptr || !g_pplayer->m_PlaySound)
      return S_OK;
   const string name = MakeString(soundName);
   if (StrCompareNoCase("knock"s, name) || StrCompareNoCase("knocker"s, name)) // FIXME remove or port to plugin
      ushock_output_knock();
   VPX::Sound *const sound = GetSound(name);
   if (sound)
   {
      g_pplayer->m_audioPlayer->PlaySound(sound, volume, randompitch, pitch, pan, front_rear_fade, loopcount, VBTOb(usesame), VBTOb(restart));
   }
   else if (!name.empty() && !m_loggedSoundErrors.contains(name))
   {
      m_loggedSoundErrors.insert(name);
      PLOGW << "Request to play \"" << name << "\", but sound was not found.";
   }
   return S_OK;
}

STDMETHODIMP PinTable::StopSound(BSTR soundName)
{
   if (g_pplayer == nullptr || !g_pplayer->m_PlaySound)
      return S_OK;
   const string name = MakeString(soundName);
   VPX::Sound *sound = GetSound(name);
   if (sound)
   {
      g_pplayer->m_audioPlayer->StopSound(sound);
   }
   else if (!name.empty() && !m_loggedSoundErrors.contains(name))
   {
      m_loggedSoundErrors.insert(name);
      PLOGW << "Request to stop \"" << name << "\", but sound was not found.";
   }

   return S_OK;
}

RenderProbe *PinTable::GetRenderProbe(const string &szName) const
{
   if (szName.empty())
      return nullptr;

   // during playback, we use the hashtable for lookup
   if (!m_renderprobeMap.empty())
   {
      const ankerl::unordered_dense::map<string, RenderProbe *, StringHashFunctor, StringComparator>::const_iterator it = m_renderprobeMap.find(szName);
      if (it != m_renderprobeMap.end())
         return it->second;
      else
         return nullptr;
   }

   for (size_t i = 0; i < m_vrenderprobe.size(); i++)
      if (StrCompareNoCase(m_vrenderprobe[i]->GetName(), szName))
         return m_vrenderprobe[i];

   return nullptr;
}

Light *PinTable::GetLight(const string &szName) const
{
   if (szName.empty())
      return nullptr;

   // during playback, we use the hashtable for lookup
   if (!m_lightMap.empty())
   {
      const ankerl::unordered_dense::map<string, Light *, StringHashFunctor, StringComparator>::const_iterator it = m_lightMap.find(szName);
      if (it != m_lightMap.end())
         return it->second;
      else
         return nullptr;
   }

   for (size_t i = 0; i < m_vedit.size(); i++)
   {
      IEditable *const pe = m_vedit[i];
      if (pe->GetItemType() == ItemTypeEnum::eItemLight && StrCompareNoCase(pe->GetName(), szName))
         return (Light *)pe;
   }

   return nullptr;
}

Texture* PinTable::GetImage(const string &szName) const
{
   if (szName.empty())
      return nullptr;

   // during playback, we use the hashtable for lookup
   if (!m_textureMap.empty())
   {
      const ankerl::unordered_dense::map<string, Texture*, StringHashFunctor, StringComparator>::const_iterator
         it = m_textureMap.find(szName);
      if (it != m_textureMap.end())
         return it->second;
      else
         return nullptr;
   }

   for (auto image : m_vimage)
      if (StrCompareNoCase(image->m_name, szName))
         return image;

   return nullptr;
}

bool PinTable::ExportImage(const Texture * const ppi, const string &filename)
{
   return ppi->SaveFile(filename);
}

Texture *PinTable::ImportImage(const string &filename, const string &imagename)
{
   Texture *existing = nullptr;
   if (!imagename.empty())
      existing = GetImage(imagename);

   Texture *image = Texture::CreateFromFile(filename);
   if (image == nullptr)
   {
      delete image;
      return nullptr;
   }

   if (!imagename.empty())
      image->m_name = imagename;

   if (existing)
   {
      RemoveFromVectorSingle(m_vimage, existing);
      if (m_liveBaseTable)
         RemoveFromVectorSingle(m_vliveimage, existing);
      image->m_alphaTestValue = existing->m_alphaTestValue;
      delete existing;
   }

   m_vimage.push_back(image);
   if (m_liveBaseTable)
   {
      m_textureMap[image->m_name] = image;
      m_vliveimage.push_back(image);
   }
   return image;
}

void PinTable::RemoveImage(Texture * const ppi)
{
   RemoveFromVectorSingle(m_vimage, ppi);

   delete ppi;
}

void PinTable::ListMaterials(HWND hwndListView)
{
   for (size_t i = 0; i < m_materials.size(); i++)
      AddListMaterial(hwndListView, m_materials[i]);
}

bool PinTable::IsMaterialNameUnique(const string &name) const
{
   for (size_t i = 0; i < m_materials.size(); i++)
      if(m_materials[i]->m_name == name)
         return false;

   return true;
}


Material* PinTable::GetMaterial(const string &name) const
{
   if (name.empty())
      return &m_vpinball->m_dummyMaterial;

   // during playback, we use the hashtable for lookup
   if (!m_materialMap.empty())
   {
      const ankerl::unordered_dense::map<string, Material*, StringHashFunctor, StringComparator>::const_iterator
         it = m_materialMap.find(name);
      if (it != m_materialMap.end())
         return it->second;
      else
         return &m_vpinball->m_dummyMaterial;
   }

   for (size_t i = 0; i < m_materials.size(); i++)
      if(m_materials[i]->m_name == name)
         return m_materials[i];

   return &m_vpinball->m_dummyMaterial;
}

void PinTable::AddMaterial(Material * const pmat)
{
   if (pmat->m_name.empty() || pmat->m_name == "dummyMaterial")
      pmat->m_name = "Material"s;

   if (!IsMaterialNameUnique(pmat->m_name) || pmat->m_name == "Material")
   {
      int suffix = 1;
      string textBuf;
      do
      {
         textBuf = pmat->m_name + std::to_string(suffix);
         suffix++;
      } while (!IsMaterialNameUnique(textBuf));
      pmat->m_name = textBuf;
   }

   m_materials.push_back(pmat);
}

int PinTable::AddListMaterial(HWND hwndListView, Material * const pmat)
{
#ifndef __STANDALONE__
   constexpr char usedStringYes[] = "X";
   constexpr char usedStringNo[] = " ";

   LVITEM lvitem;
   lvitem.mask = LVIF_DI_SETITEM | LVIF_TEXT | LVIF_PARAM;
   lvitem.iItem = 0;
   lvitem.iSubItem = 0;
   lvitem.pszText = (LPSTR)pmat->m_name.c_str();
   lvitem.lParam = (size_t)pmat;

   const int index = ListView_InsertItem(hwndListView, &lvitem);
   ListView_SetItemText(hwndListView, index, 1, (LPSTR)usedStringNo);
   if(pmat->m_name == m_playfieldMaterial)
   {
      ListView_SetItemText(hwndListView, index, 1, (LPSTR)usedStringYes);
   }
   else
   {
      for (size_t i = 0; i < m_vedit.size(); i++)
      {
         bool inUse = false;
         IEditable * const pEdit = m_vedit[i];
         if (pEdit == nullptr)
            continue;

         switch (pEdit->GetItemType())
         {
         case eItemPrimitive:
         {
            const Primitive * const pPrim = (Primitive*)pEdit;
            if (StrCompareNoCase(pPrim->m_d.m_szMaterial, pmat->m_name) || StrCompareNoCase(pPrim->m_d.m_szPhysicsMaterial, pmat->m_name))
               inUse = true;
            break;
         }
         case eItemRamp:
         {
            const Ramp * const pRamp = (Ramp*)pEdit;
            if (StrCompareNoCase(pRamp->m_d.m_szMaterial, pmat->m_name) || StrCompareNoCase(pRamp->m_d.m_szPhysicsMaterial, pmat->m_name))
               inUse = true;
            break;
         }
         case eItemSurface:
         {
            const Surface * const pSurf = (Surface*)pEdit;
            if (StrCompareNoCase(pSurf->m_d.m_szPhysicsMaterial, pmat->m_name) || StrCompareNoCase(pSurf->m_d.m_szSideMaterial, pmat->m_name) || StrCompareNoCase(pSurf->m_d.m_szTopMaterial, pmat->m_name) || StrCompareNoCase(pSurf->m_d.m_szSlingShotMaterial, pmat->m_name))
               inUse = true;
            break;
         }
         case eItemDecal:
         {
            const Decal * const pDecal = (Decal*)pEdit;
            if (StrCompareNoCase(pDecal->m_d.m_szMaterial, pmat->m_name))
               inUse = true;
            break;
         }
         case eItemFlipper:
         {
            const Flipper * const pFlip = (Flipper*)pEdit;
            if (StrCompareNoCase(pFlip->m_d.m_szRubberMaterial, pmat->m_name) || StrCompareNoCase(pFlip->m_d.m_szMaterial, pmat->m_name))
               inUse = true;
            break;
         }
         case eItemHitTarget:
         {
            const HitTarget * const pHit = (HitTarget*)pEdit;
            if (StrCompareNoCase(pHit->m_d.m_szMaterial, pmat->m_name) || StrCompareNoCase(pHit->m_d.m_szPhysicsMaterial, pmat->m_name))
               inUse = true;
            break;
         }
         case eItemPlunger:
         {
            const Plunger * const pPlung = (Plunger*)pEdit;
            if (StrCompareNoCase(pPlung->m_d.m_szMaterial, pmat->m_name))
               inUse = true;
            break;
         }
         case eItemSpinner:
         {
            const Spinner * const pSpin = (Spinner*)pEdit;
            if (StrCompareNoCase(pSpin->m_d.m_szMaterial, pmat->m_name))
               inUse = true;
            break;
         }
         case eItemRubber:
         {
            const Rubber * const pRub = (Rubber*)pEdit;
            if (StrCompareNoCase(pRub->m_d.m_szMaterial, pmat->m_name) || StrCompareNoCase(pRub->m_d.m_szPhysicsMaterial, pmat->m_name))
               inUse = true;
            break;
         }
         case eItemBumper:
         {
            const Bumper * const pBump = (Bumper*)pEdit;
            if (StrCompareNoCase(pBump->m_d.m_szCapMaterial, pmat->m_name) || StrCompareNoCase(pBump->m_d.m_szBaseMaterial, pmat->m_name) ||
                StrCompareNoCase(pBump->m_d.m_szSkirtMaterial, pmat->m_name) || StrCompareNoCase(pBump->m_d.m_szRingMaterial, pmat->m_name))
               inUse = true;
            break;
         }
         case eItemKicker:
         {
            const Kicker * const pKick = (Kicker*)pEdit;
            if (StrCompareNoCase(pKick->m_d.m_szMaterial, pmat->m_name))
               inUse = true;
            break;
         }
         case eItemTrigger:
         {
            const Trigger * const pTrig = (Trigger*)pEdit;
            if (StrCompareNoCase(pTrig->m_d.m_szMaterial, pmat->m_name))
               inUse = true;
            break;
         }
         default:
            break;
         }

         if (inUse)
         {
            ListView_SetItemText(hwndListView, index, 1, (LPSTR)usedStringYes);
            break;
         }
      }//for
   }
   return index;
#else
   return 0L;
#endif
}

void PinTable::RemoveMaterial(Material * const pmat)
{
   RemoveFromVectorSingle(m_materials, pmat);

   delete pmat;
}

bool PinTable::GetImageLink(const Texture * const ppi) const
{
   return StrCompareNoCase(ppi->m_name, m_screenShot);
}

PinBinary *PinTable::GetImageLinkBinary(const int id)
{
   switch (id)
   {
   case 1: //Screenshot
      // Transfer ownership of the screenshot pinbinary blob to the image
      PinBinary * const pbT = m_pbTempScreenshot;
      m_pbTempScreenshot = nullptr;
      return pbT;
      break;
   }

   return nullptr;
}

string PinTable::AuditTable(bool log) const
{
   // Perform a simple table audit (disable lighting vs static, script reference of static parts, png vs webp, hdr vs exr,...)
   std::stringstream ss;

   // Ultra basic parser to get a (somewhat) valid list of referenced parts
   #ifndef __STANDALONE__
      const size_t cchar = ::SendMessage(m_pcv->m_hwndScintilla, SCI_GETTEXTLENGTH, 0, 0);
      char * const szText = new char[cchar + 1];
      ::SendMessage(m_pcv->m_hwndScintilla, SCI_GETTEXT, cchar + 1, (LPARAM)szText);
   #else
      const char * const szText = (char*)m_pcv->m_script_text.c_str();
   #endif
   const char *wordStart = nullptr;
   const char *wordPos = szText;
   string inClass;
   bool nextIsFunc = false, nextIsEnd = false, nextIsClass = false, isInString = false, isInComment = false;
   vector<string> functions, identifiers;
   int line = 0;
   while (wordPos[0] != '\0')
   {
      if (isInComment)
      {
         // skip
      }
      else if (wordPos[0] == '"')
         isInString = !isInString;
      else if (!isInString)
      {
         // Detect comments
         if (wordPos[0] == '\'')
            isInComment = true;
         // Split identifiers (eventually class/function identifier)
         else if (wordPos[0] == ' ' || wordPos[0] == '\r' || wordPos[0] == '\t' || wordPos[0] == '\n' || wordPos[0] == '.' 
               || wordPos[0] == ':' || wordPos[0] == '('  || wordPos[0] == ')'  || wordPos[0] == '['  || wordPos[0] == ']')
         {
            if (wordStart)
            {
               string word(wordStart, (int)(wordPos - wordStart));
               StrToLower(word);
               if (word == "end" || word == "exit")
               {
                  nextIsFunc = false;
                  nextIsEnd = true;
               }
               else if (word == "class")
               {
                  if (nextIsEnd)
                     inClass.clear();
                  nextIsClass = !nextIsEnd;
                  nextIsEnd = false;
               }
               else if (word == "function" || word == "sub")
               {
                  nextIsFunc = !nextIsEnd;
                  nextIsEnd = false;
               }
               else
               {
                  if (word[0] >= 'a' && word[0] <= 'z')
                  {
                     if (nextIsClass)
                        inClass = word;
                     else if (nextIsFunc)
                     {
                        //ss << "- " << word << ", line=" << (line + 1) << ", class=" << inClass << "\r\n";
                        if (FindIndexOf(functions, inClass + '.' + word) != -1)
                           ss << ". Error: Duplicate declaration of '" << word << "' in script at line " << line << "\r\n";
                        else
                           functions.push_back(inClass + '.' + word);
                     }
                     else if (FindIndexOf(identifiers, word) == -1)
                        identifiers.push_back(word);
                  }
                  nextIsFunc = false;
                  nextIsEnd = false;
                  nextIsClass = false;
               }
            }
            wordStart = nullptr;
         }
         else if (wordStart == nullptr)
            wordStart = wordPos;
      }
      // Next line
      if (wordPos[0] == '\n')
      {
         isInComment = false;
         line++;
      }

      wordPos++;
   }

   #ifndef __STANDALONE__
      delete[] szText;
   #endif

   if (FindIndexOf(identifiers, "execute"s) != -1)
      ss << ". Warning: Scripts seems to use the 'Execute' command. This command triggers computer security checks and will likely cause stutters during play.\r\n";

   if (m_glassBottomHeight > m_glassTopHeight)
      ss << ". Warning: Glass height seems invalid: bottom is higher than top\r\n";

   if (m_glassBottomHeight < INCHESTOVPU(2) || m_glassTopHeight < INCHESTOVPU(2))
      ss << ". Warning: Glass height seems invalid: glass is below 2\"\r\n";

   if (m_ballSphericalMapping)
      ss << ". Warning: Ball uses legacy 'spherical mapping', it will be rendered like a 2D object and therefore will look bad in VR, stereo or headtracking\r\n";

   // Search for inconsistencies in the table parts
   bool hasPulseTimer = false, hasPinMameTimer = false;
   for (const auto part : m_vedit)
   {
      auto type = part->GetItemType();
      Primitive *const prim = type == eItemPrimitive ? (Primitive *)part : nullptr;
      Light *const light = type == eItemLight ? (Light *)part : nullptr;
      Surface *const surf = type == eItemSurface ? (Surface *)part : nullptr;
      Textbox *const textbox = type == eItemTextbox ? (Textbox *)part : nullptr;

      // Referencing a static object from script (ok if it is for reading properties, not for writing)
      if (type == eItemPrimitive && prim->m_d.m_staticRendering && FindIndexOf(identifiers, prim->GetName()) != -1)
         ss << ". Warning: Primitive '" << prim->GetName() << "' seems to be referenced from the script while it is marked as static (most properties of a static object may not be modified at runtime).\r\n";

      if (type == eItemTextbox && (textbox->m_d.m_isDMD || StrFindNoCase(textbox->m_d.m_text, "DMD"s) != string::npos))
         ss << ". Warning: legacy Textbox '" << textbox->GetName() << "' is used for DMD rendering. It should be replaced by a flasher to get better rendering.\r\n";

      if (type == eItemTimer) {
         const string name = lowerCase(((Timer *)part)->GetName());
         hasPulseTimer |= name == "pulsetimer";
         hasPinMameTimer |= name == "pinmametimer";
      }

      // Warning on very fast timers (lower than 5ms)
      TimerDataRoot *tdr = nullptr;
      switch (type)
      {
      // case eItemPrimitive: tdr = &prim->m_d.m_tdr; break; // Note: primitives have timers, but they are not exposed to the UI
      case eItemSurface: tdr = &surf->m_d.m_tdr; break;
      case eItemTimer: tdr = &((Timer *)part)->m_d.m_tdr; break;
      case eItemLight: tdr = &((Light *)part)->m_d.m_tdr; break;
      case eItemRamp: tdr = &((Ramp *)part)->m_d.m_tdr; break;
      case eItemPlunger: tdr = &((Plunger *)part)->m_d.m_tdr; break;
      case eItemSpinner: tdr = &((Spinner *)part)->m_d.m_tdr; break;
      case eItemTrigger: tdr = &((Trigger *)part)->m_d.m_tdr; break;
      case eItemKicker: tdr = &((Kicker *)part)->m_d.m_tdr; break;
      case eItemRubber: tdr = &((Rubber *)part)->m_d.m_tdr; break;
      case eItemFlasher: tdr = &((Flasher *)part)->m_d.m_tdr; break;
      case eItemLightSeq: tdr = &((LightSeq *)part)->m_d.m_tdr; break;
      case eItemHitTarget: tdr = &((HitTarget *)part)->m_d.m_tdr; break;
      case eItemBumper: tdr = &((Bumper *)part)->m_d.m_tdr; break;
      case eItemFlipper: tdr = &((Flipper *)part)->m_d.m_tdr; break;
      case eItemGate: tdr = &((Gate *)part)->m_d.m_tdr; break;
      default: break;
      }
      if (tdr && tdr->m_TimerEnabled && tdr->m_TimerInterval != -1 && tdr->m_TimerInterval != -2 && tdr->m_TimerInterval < 17)
         ss << ". Warning: Part '" << part->GetName() << "' uses a timer with a very short period of " << tdr->m_TimerInterval << "ms, below a 60FPS framerate. This will likely cause stutters and the table will not support 'frame pacing'.\r\n";

      if (type == eItemPrimitive && prim->m_d.m_visible
         && prim->m_d.m_disableLightingBelow != 1.f && !prim->m_d.m_staticRendering
         && (!GetMaterial(prim->m_d.m_szMaterial)->m_bOpacityActive || GetMaterial(prim->m_d.m_szMaterial)->m_fOpacity == 1.f)
         && (GetImage(prim->m_d.m_szImage) == nullptr || GetImage(prim->m_d.m_szImage)->IsOpaque()))
         ss << ". Warning: Primitive '" << prim->GetName() << "' uses translucency (lighting from below) while it is fully opaque. Translucency will be discarded.\r\n";

      if (type == eItemLight && light->m_d.m_intensity < 0.f)
         ss << ". Error: Light '" << light->GetName() << "' has a negative intensity.\r\n";
      if (type == eItemLight && light->m_d.m_intensity_scale < 0.f)
         ss << ". Error: Light '" << light->GetName() << "' has a negative intensity scale.\r\n";

      // Disabled as this is now enforced in the rendering
      // Enabling translucency (light from below) won't work with static parts: otherwise the rendering will be different in VR/Headtracked vs desktop modes. It also needs a non opaque alpha.
      //if (type == eItemPrimitive && prim->m_d.m_disableLightingBelow != 1.f && prim->m_d.m_staticRendering)
      //   ss << ". Warning: Primitive '" << prim->GetName() << "' has translucency enabled but is also marked as static. Translucency will not be applied on desktop, and it will look different between VR/headtracked and desktop.\r\n";
      //if (type == eItemSurface && surf->m_d.m_disableLightingBelow != 1.f && surf->StaticRendering())
      //   ss << ". Warning: Wall '" << surf->GetName() << "' has translucency enabled but will be statically rendered (not droppable with opaque materials). Translucency will not be applied on desktop, and it will look different between VR/headtracked and desktop.\r\n";
   }

   if ((FindIndexOf(identifiers, "loadvpm"s) != -1) || (FindIndexOf(identifiers, "loadvpmalt"s) != -1))
   {
      if (!hasPinMameTimer)
         ss << ". Warning: VPM controller is used but table is missing a Timer object named 'PinMAMETimer'.\r\n";
      if (FindIndexOf(identifiers, "vpminit"s) == -1)
         ss << ". Warning: VPM controller is used but vpmInit is not called. pause/resume/exit will likely exhibit bugs and physic outputs won't be supported.\r\n";
   }

   if (!hasPulseTimer && (FindIndexOf(identifiers, "vpmTimer"s) != -1))
      ss << ". Warning: script uses 'vpmTimer' but table is missing a Timer object named 'PulseTimer'. vpmTimer will not work as expected.\r\n";

   auto audioPlayer = std::make_unique<VPX::AudioPlayer>(m_settings.GetPlayer_SoundDeviceBG(), m_settings.GetPlayer_SoundDevice(), static_cast<VPX::SoundConfigTypes>(m_settings.GetPlayer_Sound3D()));
   for (auto sound : m_vsound)
   {
      auto specs = audioPlayer->GetSoundInformations(sound);
      if (specs.nChannels > 1 && sound->GetOutputTarget() == VPX::SNDOUT_TABLE)
         ss << ". Error: sound '" << sound->GetName() << "' is used for playfield physical sound but has multiple channels (not mono).\r\n ";
   }
   audioPlayer = nullptr;

   if (ss.str().empty())
      ss << "No issue identified.\r\n";

   // Also output a log of the table file content to allow easier size optimization
   size_t totalSize = 0, totalGpuSize = 0;
   for (const auto sound : m_vsound)
   {
      //ss << "  . Sound: '" << sound->m_name << "', size: " << SizeToReadable(sound->m_cdata) << "\r\n";
      totalSize += sound->GetFileSize();
   }
   ss << ". Total sound size: " << SizeToReadable(totalSize) << "\r\n";

   totalSize = 0;
   for (const auto image : m_vimage)
   {
      size_t imageSize = image->GetFileSize();
      size_t gpuSize = image->GetEstimatedGPUSize();
      //ss << "  . Image: '" << image->m_name << "', size: " << SizeToReadable(imageSize) << ", GPU mem size: " << SizeToReadable(gpuSize) << "\r\n";
      totalSize += imageSize;
      totalGpuSize += gpuSize;
   }
   ss << ". Total image size: " << SizeToReadable(totalSize) << " in VPX file, at least " << SizeToReadable(totalGpuSize) << " in GPU memory when played\r\n";

   int nPrimTris = 0, primMemSize = 0;
   for (const auto part : m_vedit)
      if (part->GetItemType() == eItemPrimitive && ((Primitive *)part)->m_d.m_use3DMesh /* && ((Primitive *)part)->m_d.m_visible */ )
      {
         primMemSize += (((Primitive *)part)->m_mesh.NumIndices() > 65536 ? 4 : 2) * (int) ((Primitive *)part)->m_mesh.NumIndices();
         primMemSize += (int) ((Primitive *)part)->m_mesh.NumVertices() * sizeof(Vertex3D_NoTex2);
         nPrimTris += (int) ((Primitive *)part)->m_mesh.NumIndices() / 3;
      }
   ss << ". Total number of faces used in primitives: " << nPrimTris << ", needing " << SizeToReadable(primMemSize) << " in GPU memory when played\r\n";

   const string msg = "Table audit:\r\n" + ss.str();
   if (log)
   {
      string msg2 = msg;
      std::erase(msg2, '\r');
      PLOGI << trim_string(msg2);
   }
   return msg;
}

void PinTable::ListCustomInfo(HWND hwndListView)
{
   for (size_t i = 0; i < m_vCustomInfoTag.size(); i++)
      AddListItem(hwndListView, m_vCustomInfoTag[i], m_vCustomInfoContent[i], NULL);
}

int PinTable::AddListItem(HWND hwndListView, const string& szName, const string& szValue1, LPARAM lparam)
{
#ifndef __STANDALONE__
   LVITEM lvitem;
   lvitem.mask = LVIF_DI_SETITEM | LVIF_TEXT | LVIF_PARAM;
   lvitem.iItem = 0;
   lvitem.iSubItem = 0;
   lvitem.pszText = (LPSTR)szName.c_str();
   lvitem.lParam = lparam;

   const int index = ListView_InsertItem(hwndListView, &lvitem);

   ListView_SetItemText(hwndListView, index, 1, (char*)szValue1.c_str());

   return index;
#else
   return 0L;
#endif
}

STDMETHODIMP PinTable::get_Image(BSTR *pVal)
{
   *pVal = MakeWideBSTR(m_image);
   return S_OK;
}

STDMETHODIMP PinTable::put_Image(BSTR newVal)
{
   const string szImage = MakeString(newVal);
   const Texture * const tex = GetImage(szImage);
   if (tex && tex->IsHDR())
   {
       ShowError("Cannot use a HDR image (.exr/.hdr) here");
       return E_FAIL;
   }

   STARTUNDO
   m_image = szImage;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::GetPredefinedStrings(DISPID dispID, CALPOLESTR *pcaStringsOut, CADWORD *pcaCookiesOut)
{
   return GetPredefinedStrings(dispID, pcaStringsOut, pcaCookiesOut, nullptr);
}

STDMETHODIMP PinTable::GetPredefinedValue(DISPID dispID, DWORD dwCookie, VARIANT *pVarOut)
{
   return GetPredefinedValue(dispID, dwCookie, pVarOut, nullptr);
}

STDMETHODIMP PinTable::GetPredefinedStrings(DISPID dispID, CALPOLESTR *pcaStringsOut, CADWORD *pcaCookiesOut, IEditable *piedit)
{
   if (pcaStringsOut == nullptr || pcaCookiesOut == nullptr)
      return E_POINTER;

   size_t cvar;
   WCHAR **rgstr;
   uint32_t *rgdw;

   switch (dispID)
   {
   case DISPID_Image:
   case DISPID_Image2:
   case DISPID_Image3:
   case DISPID_Image4:
   case DISPID_Image5:
   case DISPID_Image6:
   case DISPID_Image7:
   case DISPID_Image8:
   {
      cvar = m_vimage.size();

      rgstr = (WCHAR **)CoTaskMemAlloc((cvar + 1) * sizeof(WCHAR *));
      rgdw = (uint32_t *)CoTaskMemAlloc((cvar + 1) * sizeof(uint32_t));

      WCHAR *wzDst = (WCHAR *)CoTaskMemAlloc(7 * sizeof(WCHAR));
      // TEXT
      MultiByteToWideCharNull(CP_ACP, 0, LocalString(IDS_NONE).m_szbuffer, -1, wzDst, 7);
      rgstr[0] = wzDst;
      rgdw[0] = ~0u;

      for (size_t ivar = 0; ivar < cvar; ivar++)
      {
         const int cwch = (int)m_vimage[ivar]->m_name.length() + 1;
         wzDst = (WCHAR *)CoTaskMemAlloc(cwch*sizeof(WCHAR));
         if (wzDst == nullptr)
            ShowError("DISPID_Image alloc failed");
         MultiByteToWideCharNull(CP_ACP, 0, m_vimage[ivar]->m_name.c_str(), -1, wzDst, cwch);

         //MsoWzCopy(szSrc,szDst);
         rgstr[ivar + 1] = wzDst;
         rgdw[ivar + 1] = (uint32_t)ivar;
      }
      cvar++;
   }
   break;
   case IDC_MATERIAL_COMBO:
   case IDC_MATERIAL_COMBO2:
   case IDC_MATERIAL_COMBO3:
   case IDC_MATERIAL_COMBO4:
   {
      cvar = m_materials.size();
      rgstr = (WCHAR **)CoTaskMemAlloc((cvar + 1) * sizeof(WCHAR *));
      rgdw = (uint32_t *)CoTaskMemAlloc((cvar + 1) * sizeof(uint32_t));

      WCHAR *wzDst = (WCHAR *)CoTaskMemAlloc(7 * sizeof(WCHAR));
      // TEXT
      MultiByteToWideCharNull(CP_ACP, 0, LocalString(IDS_NONE).m_szbuffer, -1, wzDst, 7);
      rgstr[0] = wzDst;
      rgdw[0] = ~0u;

      for (size_t ivar = 0; ivar < cvar; ivar++)
      {
         const int cwch = (int)m_materials[ivar]->m_name.length() + 1;
         wzDst = (WCHAR *)CoTaskMemAlloc(cwch*sizeof(WCHAR));
         if (wzDst == nullptr)
            ShowError("IDC_MATERIAL_COMBO alloc failed");
         MultiByteToWideCharNull(CP_ACP, 0, m_materials[ivar]->m_name.c_str(), -1, wzDst, cwch);

         //MsoWzCopy(szSrc,szDst);
         rgstr[ivar + 1] = wzDst;
         rgdw[ivar + 1] = (uint32_t)ivar;
      }
      cvar++;
      break;
   }
   case DISPID_Sound:
   {
      cvar = m_vsound.size();

      rgstr = (WCHAR **)CoTaskMemAlloc((cvar + 1) * sizeof(WCHAR *));
      rgdw = (uint32_t *)CoTaskMemAlloc((cvar + 1) * sizeof(uint32_t));

      rgstr[0] = (WCHAR *)CoTaskMemAlloc(7 * sizeof(WCHAR));
      wcsncpy_s(rgstr[0], 7, L"<None>");
      rgdw[0] = ~0u;

      for (size_t ivar = 0; ivar < cvar; ivar++)
      {
         const int cwch = (int)m_vsound[ivar]->GetName().length() + 1;
         rgstr[ivar + 1] = (WCHAR *)CoTaskMemAlloc(cwch * sizeof(WCHAR));
         if (rgstr[ivar + 1] == nullptr)
            ShowError("DISPID_Sound alloc failed");
         MultiByteToWideCharNull(CP_ACP, 0, m_vsound[ivar]->GetName().c_str(), -1, rgstr[ivar + 1], cwch);

         //MsoWzCopy(szSrc,szDst);
         rgdw[ivar + 1] = (uint32_t)ivar;
      }
      cvar++;
   }
   break;

   case DISPID_Collection:
   {
      cvar = m_vcollection.size();

      rgstr = (WCHAR **)CoTaskMemAlloc((cvar + 1) * sizeof(WCHAR *));
      rgdw = (uint32_t *)CoTaskMemAlloc((cvar + 1) * sizeof(uint32_t));

      rgstr[0] = (WCHAR *)CoTaskMemAlloc(7 * sizeof(WCHAR));
      wcsncpy_s(rgstr[0], 7, L"<None>");
      rgdw[0] = ~0u;

      for (size_t ivar = 0; ivar < cvar; ivar++)
      {
         constexpr DWORD cwch = sizeof(m_vcollection[(int)ivar].m_wzName) + sizeof(DWORD); //!! +DWORD?
         rgstr[ivar + 1] = (WCHAR *)CoTaskMemAlloc(cwch);
         if (rgstr[ivar + 1] == nullptr)
            ShowError("DISPID_Collection alloc failed (1)");
         else
         {
            memset(rgstr[ivar + 1], 0, cwch);
            memcpy(rgstr[ivar + 1], m_vcollection[(int)ivar].m_wzName, sizeof(m_vcollection[(int)ivar].m_wzName));
         }
         rgdw[ivar + 1] = (uint32_t)ivar;
      }
      cvar++;
   }
   break;

   case DISPID_Surface:
   {
      cvar = 0;

      constexpr bool ramps = true;
      constexpr bool flashers = true;

      for (size_t ivar = 0; ivar < m_vedit.size(); ivar++)
         if (m_vedit[ivar]->GetItemType() == eItemSurface ||
            (ramps && m_vedit[ivar]->GetItemType() == eItemRamp) ||
            //!! **************** warning **********************
            // added to render to surface of DMD style lights and emreels
            // but no checks are being performed at moment:
            (flashers && m_vedit[ivar]->GetItemType() == eItemFlasher))
            cvar++;

      rgstr = (WCHAR **)CoTaskMemAlloc((cvar + 1) * sizeof(WCHAR *));
      rgdw = (uint32_t *)CoTaskMemAlloc((cvar + 1) * sizeof(uint32_t));

      cvar = 0;

      WCHAR *wzDst = (WCHAR *)CoTaskMemAlloc(7 * sizeof(WCHAR));
      if (wzDst == nullptr)
         ShowError("DISPID_Surface alloc failed (0)");
      // TEXT
      wcsncpy_s(wzDst, 7, L"<None>");
      rgstr[cvar] = wzDst;
      rgdw[cvar] = ~0u;
      cvar++;

      for (size_t ivar = 0; ivar < m_vedit.size(); ivar++)
      {
         if (m_vedit[ivar]->GetItemType() == eItemSurface ||
            (ramps && m_vedit[ivar]->GetItemType() == eItemRamp) ||
            //!! **************** warning **********************
            // added to render to surface of DMD style lights and emreels
            // but no checks are being performed at moment:
            (flashers && m_vedit[ivar]->GetItemType() == eItemFlasher))
         {
            const wstring sname = m_vedit[ivar]->GetScriptable()->m_wzName;

            const size_t cwch = sname.length() + 1;
            //wzDst = ::SysAllocString(bstr);

            wzDst = (WCHAR *)CoTaskMemAlloc(cwch*sizeof(WCHAR));
            if (wzDst == nullptr)
               ShowError("DISPID_Surface alloc failed (1)");

            wcsncpy_s(wzDst, cwch, sname.c_str());
            rgstr[cvar] = wzDst;
            rgdw[cvar] = (uint32_t)ivar;
            cvar++;
         }
      }
   }
   break;
   case IDC_EFFECT_COMBO:
   {
      cvar = 5;

      rgstr = (WCHAR **)CoTaskMemAlloc(cvar* sizeof(WCHAR *));
      rgdw = (uint32_t *)CoTaskMemAlloc(cvar * sizeof(uint32_t));

      rgstr[0] = (WCHAR *)CoTaskMemAlloc(5 * sizeof(WCHAR));
      wcsncpy_s(rgstr[0], 5, L"None");
      rgdw[0] = ~0u;
      rgstr[1] = (WCHAR *)CoTaskMemAlloc(9 * sizeof(WCHAR));
      wcsncpy_s(rgstr[1], 9, L"Additive");
      rgdw[1] = 1;
      rgstr[2] = (WCHAR *)CoTaskMemAlloc(9 * sizeof(WCHAR));
      wcsncpy_s(rgstr[2], 9, L"Multiply");
      rgdw[2] = 2;
      rgstr[3] = (WCHAR *)CoTaskMemAlloc(8 * sizeof(WCHAR));
      wcsncpy_s(rgstr[3], 8, L"Overlay");
      rgdw[3] = 3;
      rgstr[4] = (WCHAR *)CoTaskMemAlloc(7 * sizeof(WCHAR));
      wcsncpy_s(rgstr[4], 7, L"Screen");
      rgdw[4] = 4;

      break;
   }
   default:
      return E_NOTIMPL;
   }

   pcaStringsOut->cElems = (int)cvar;
   pcaStringsOut->pElems = rgstr;

   pcaCookiesOut->cElems = (int)cvar;
   pcaCookiesOut->pElems = (DWORD*)rgdw;

   return S_OK;
}

STDMETHODIMP PinTable::GetPredefinedValue(DISPID dispID, DWORD dwCookie, VARIANT *pVarOut, IEditable *piedit)
{
   WCHAR *wzDst;

   switch (dispID)
   {
   default: return E_NOTIMPL;

   case DISPID_Image:
   case DISPID_Image2:
   case DISPID_Image3:
   case DISPID_Image4:
   case DISPID_Image5:
   case DISPID_Image6:
   case DISPID_Image7:
   case DISPID_Image8:
   {
      if (dwCookie == -1)
      {
         wzDst = (WCHAR *)malloc(1 * sizeof(WCHAR));
         wzDst[0] = L'\0';
      }
      else
      {
         const int cwch = MultiByteToWideChar(CP_ACP, 0, m_vimage[dwCookie]->m_name.c_str(), -1, nullptr, 0); //(int)m_vimage[dwCookie]->m_name.length() + 1;
         wzDst = (WCHAR *)malloc(cwch*sizeof(WCHAR));
         MultiByteToWideChar(CP_ACP, 0, m_vimage[dwCookie]->m_name.c_str(), -1, wzDst, cwch);
      }
   }
   break;
   case IDC_MATERIAL_COMBO:
   case IDC_MATERIAL_COMBO2:
   case IDC_MATERIAL_COMBO3:
   case IDC_MATERIAL_COMBO4:
   {
      if (dwCookie == -1)
      {
         wzDst = (WCHAR *)malloc(1 * sizeof(WCHAR));
         wzDst[0] = L'\0';
      }
      else
      {
         const int cwch = MultiByteToWideChar(CP_ACP, 0, m_materials[dwCookie]->m_name.c_str(), -1, nullptr, 0); //(int)m_materials[dwCookie]->m_name.length() + 1;
         wzDst = (WCHAR *)malloc(cwch*sizeof(WCHAR));
         MultiByteToWideChar(CP_ACP, 0, m_materials[dwCookie]->m_name.c_str(), -1, wzDst, cwch);
      }
      break;
   }
   case DISPID_Sound:
   {
      if (dwCookie == -1)
      {
         wzDst = (WCHAR *)malloc(1 * sizeof(WCHAR));
         if (wzDst == nullptr)
             ShowError("DISPID_Sound alloc failed");
         wzDst[0] = L'\0';
      }
      else
      {
         const int cwch = MultiByteToWideChar(CP_ACP, 0, m_vsound[dwCookie]->GetName().c_str(), -1, nullptr, 0); //(int)m_vsound[dwCookie]->m_name.length() + 1;
         wzDst = (WCHAR *)malloc(cwch*sizeof(WCHAR));
         if (wzDst == nullptr)
             ShowError("DISPID_Sound alloc failed");
         MultiByteToWideChar(CP_ACP, 0, m_vsound[dwCookie]->GetName().c_str(), -1, wzDst, cwch);
      }
   }
   break;
   case DISPID_Collection:
   {
      if (dwCookie == -1)
      {
         wzDst = (WCHAR *)malloc(1 * sizeof(WCHAR));
         wzDst[0] = L'\0';
      }
      else
      {
         constexpr size_t cwch = sizeof(m_vcollection[dwCookie].m_wzName) + sizeof(DWORD); //!! +DWORD?
         wzDst = (WCHAR *)malloc(cwch);
         if (wzDst == nullptr)
            ShowError("DISPID_Collection alloc failed (2)");
         else
            memcpy(wzDst, m_vcollection[dwCookie].m_wzName, cwch - sizeof(DWORD)); //!! see above
      }
   }
   break;
   case IDC_EFFECT_COMBO:
   {
      const int idx = (dwCookie == -1) ? 0 : dwCookie;
      static const wstring filterNames[5] = { L"None"s, L"Additive"s, L"Multiply"s, L"Overlay"s, L"Screen"s };
      const size_t cwch = filterNames[idx].length() + 1;
      wzDst = (WCHAR *)malloc(cwch*sizeof(WCHAR));
      wcsncpy_s(wzDst, cwch, filterNames[idx].c_str());
      break;
   }
   case DISPID_Surface:
   {
      if (dwCookie == -1)
      {
         wzDst = (WCHAR *)malloc(1 * sizeof(WCHAR));
         wzDst[0] = L'\0';
      }
      else
      {
         const wstring sname = m_vedit[dwCookie]->GetScriptable()->m_wzName;

         const size_t cwch = sname.length() + 1;
         //wzDst = ::SysAllocString(sname);

         wzDst = (WCHAR *)malloc(cwch*sizeof(WCHAR));
         if (wzDst == nullptr)
            ShowError("DISPID_Surface alloc failed (2)");
         else
            wcsncpy_s(wzDst, cwch, sname.c_str());
      }
   }
   break;
   }

   CComVariant var(wzDst);

   free(wzDst);

   return var.Detach(pVarOut);
}

float PinTable::GetSurfaceHeight(const string& name, float x, float y) const
{
   if (!name.empty())
   {
   const wstring wname = MakeWString(name);
   for (size_t i = 0; i < m_vedit.size(); i++)
   {
      const IEditable * const item = m_vedit[i];
      if (item->GetItemType() == eItemSurface || item->GetItemType() == eItemRamp)
      {
         if (wname == item->GetScriptable()->m_wzName)
         {
            if (item->GetItemType() == eItemSurface)
               return ((const Surface *)item)->m_d.m_heighttop;
            else //if (item->GetItemType() == eItemRamp)
               return ((const Ramp *)item)->GetSurfaceHeight(x, y);
         }
      }
   }
   }

   return 0.f;
}

Material* PinTable::GetSurfaceMaterial(const string& name) const
{
   if (!name.empty())
   {
   const wstring wname = MakeWString(name);
   for (size_t i = 0; i < m_vedit.size(); i++)
   {
      const IEditable * const item = m_vedit[i];
      if (item->GetItemType() == eItemSurface || item->GetItemType() == eItemRamp)
      {
         if (wname == item->GetScriptable()->m_wzName)
         {
            if (item->GetItemType() == eItemSurface)
               return GetMaterial(((const Surface *)item)->m_d.m_szTopMaterial);
            else //if (item->GetItemType() == eItemRamp)
               return GetMaterial(((const Ramp *)item)->m_d.m_szMaterial);
         }
      }
   }
   }

   return GetMaterial(m_playfieldMaterial);
}

Texture* PinTable::GetSurfaceImage(const string& name) const
{
   if (!name.empty())
   {
   const wstring wname = MakeWString(name);
   for (size_t i = 0; i < m_vedit.size(); i++)
   {
      const IEditable * const item = m_vedit[i];
      if (item->GetItemType() == eItemSurface || item->GetItemType() == eItemRamp)
      {
         if (wname == item->GetScriptable()->m_wzName)
         {
            if (item->GetItemType() == eItemSurface)
               return GetImage(((const Surface *)item)->m_d.m_szImage);
            else //if (item->GetItemType() == eItemRamp)
               return GetImage(((const Ramp *)item)->m_d.m_szImage);
         }
      }
   }
   }

   return GetImage(m_image);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP PinTable::get_DisplayGrid(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_grid);
   return S_OK;
}

STDMETHODIMP PinTable::put_DisplayGrid(VARIANT_BOOL newVal)
{
   STARTUNDO
   m_grid = VBTOb(newVal);
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_DisplayBackdrop(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_backdrop);
   return S_OK;
}

STDMETHODIMP PinTable::put_DisplayBackdrop(VARIANT_BOOL newVal)
{
   STARTUNDO
   m_backdrop = VBTOb(newVal);
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_GlassHeight(float *pVal)
{
   *pVal = m_glassTopHeight;
   return S_OK;
}

STDMETHODIMP PinTable::put_GlassHeight(float newVal)
{
   STARTUNDO
   m_glassTopHeight = newVal;
   STOPUNDO

   return S_OK;
}

float PinTable::GetTableWidth() const
{
   return m_right - m_left;
}

void PinTable::SetTableWidth(const float value)
{
   m_right = value;
}

STDMETHODIMP PinTable::get_Width(float *pVal)
{
   *pVal = GetTableWidth();

   return S_OK;
}

STDMETHODIMP PinTable::put_Width(float newVal)
{
   STARTUNDO
   SetTableWidth(newVal);
   STOPUNDO

   SetMyScrollInfo();

   return S_OK;
}

float PinTable::GetHeight() const
{
   return m_bottom - m_top;
}

void PinTable::SetHeight(const float value)
{
   m_bottom = value;
}

float PinTable::GetPlayfieldSlope() const
{
   return lerp(m_angletiltMin, m_angletiltMax, m_globalDifficulty);
}

float PinTable::GetPlayfieldOverridenSlope() const
{
   return lerp(m_fOverrideMinSlope, m_fOverrideMaxSlope, m_globalDifficulty);
}

STDMETHODIMP PinTable::get_Height(float *pVal)
{
   *pVal = GetHeight();

   return S_OK;
}

STDMETHODIMP PinTable::put_Height(float newVal)
{
   STARTUNDO
   SetHeight(newVal);
   STOPUNDO

   SetMyScrollInfo();

   return S_OK;
}

STDMETHODIMP PinTable::get_PlayfieldMaterial(BSTR *pVal)
{
   *pVal = MakeWideBSTR(m_playfieldMaterial);
   return S_OK;
}

STDMETHODIMP PinTable::put_PlayfieldMaterial(BSTR newVal)
{
   STARTUNDO
   m_playfieldMaterial = MakeString(newVal);
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_LightAmbient(OLE_COLOR *pVal)
{
   *pVal = m_lightAmbient;
   return S_OK;
}

STDMETHODIMP PinTable::put_LightAmbient(OLE_COLOR newVal)
{
   STARTUNDO
   m_lightAmbient = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Light0Emission(OLE_COLOR *pVal)
{
   *pVal = m_Light[0].emission;
   return S_OK;
}

STDMETHODIMP PinTable::put_Light0Emission(OLE_COLOR newVal)
{
   STARTUNDO
   m_Light[0].emission = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_LightHeight(float *pVal)
{
   *pVal = m_lightHeight;
   return S_OK;
}

STDMETHODIMP PinTable::put_LightHeight(float newVal)
{
   STARTUNDO
   m_lightHeight = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_LightRange(float *pVal)
{
   *pVal = m_lightRange;
   return S_OK;
}

STDMETHODIMP PinTable::put_LightRange(float newVal)
{
   STARTUNDO
   m_lightRange = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_LightEmissionScale(float *pVal)
{
   *pVal = m_lightEmissionScale;
   return S_OK;
}

STDMETHODIMP PinTable::put_LightEmissionScale(float newVal)
{
   STARTUNDO
   m_lightEmissionScale = newVal;
   STOPUNDO

   return S_OK;
}

int PinTable::GetGlobalEmissionScale() const
{
   return quantizeUnsignedPercent(m_globalEmissionScale);
}

void PinTable::SetGlobalEmissionScale(const int value)
{
   m_globalEmissionScale = dequantizeUnsignedPercent(value);
}

STDMETHODIMP PinTable::get_NightDay(int *pVal)
{
   *pVal = GetGlobalEmissionScale();
   return S_OK;
}

STDMETHODIMP PinTable::put_NightDay(int newVal)
{
   STARTUNDO
   SetGlobalEmissionScale(newVal);
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_AOScale(float *pVal)
{
   *pVal = m_AOScale;
   return S_OK;
}

STDMETHODIMP PinTable::put_AOScale(float newVal)
{
   STARTUNDO
   m_AOScale = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_SSRScale(float *pVal)
{
   *pVal = m_SSRScale;
   return S_OK;
}

STDMETHODIMP PinTable::put_SSRScale(float newVal)
{
   STARTUNDO
   m_SSRScale = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_EnvironmentEmissionScale(float *pVal)
{
   *pVal = m_envEmissionScale;
   return S_OK;
}

STDMETHODIMP PinTable::put_EnvironmentEmissionScale(float newVal)
{
   STARTUNDO
   m_envEmissionScale = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_BallReflection(UserDefaultOnOff *pVal)
{
   // FIXME Deprecated
   *pVal = UserDefaultOnOff::On; //(UserDefaultOnOff) m_useReflectionForBalls;
   return S_OK;
}

STDMETHODIMP PinTable::put_BallReflection(UserDefaultOnOff newVal)
{
   // FIXME Deprecated
   //STARTUNDO
   //m_useReflectionForBalls = (int)newVal;
   //STOPUNDO

   return S_OK;
}

int PinTable::GetPlayfieldReflectionStrength() const
{
   return quantizeUnsignedPercent(m_playfieldReflectionStrength);
}

void PinTable::SetPlayfieldReflectionStrength(const int value)
{
   m_playfieldReflectionStrength = dequantizeUnsignedPercent(value);
}

STDMETHODIMP PinTable::get_PlayfieldReflectionStrength(int *pVal)
{
   *pVal = GetPlayfieldReflectionStrength();

   return S_OK;
}

STDMETHODIMP PinTable::put_PlayfieldReflectionStrength(int newVal)
{
   STARTUNDO
   SetPlayfieldReflectionStrength(newVal);
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_BallTrail(UserDefaultOnOff *pVal)
{
   *pVal = (g_pplayer && g_pplayer->m_renderer->m_trailForBalls) ? UserDefaultOnOff::On : UserDefaultOnOff::Off;
   return S_OK;
}

STDMETHODIMP PinTable::put_BallTrail(UserDefaultOnOff newVal)
{
   if (g_pplayer)
      g_pplayer->m_renderer->m_trailForBalls = (newVal == UserDefaultOnOff::On) || ((newVal == UserDefaultOnOff::Default) && m_settings.GetPlayer_BallTrail());
   return S_OK;
}

STDMETHODIMP PinTable::get_TrailStrength(int *pVal)
{
   *pVal = static_cast<int>(100.f * (g_pplayer ? g_pplayer->m_renderer->m_ballTrailStrength : m_settings.GetPlayer_BallTrailStrength()));
   return S_OK;
}

STDMETHODIMP PinTable::put_TrailStrength(int newVal)
{
   if (g_pplayer)
      g_pplayer->m_renderer->m_ballTrailStrength = static_cast<float>(newVal) / 100.f;
   return S_OK;
}

STDMETHODIMP PinTable::get_BallPlayfieldReflectionScale(float *pVal)
{
   *pVal = m_ballPlayfieldReflectionStrength;
   return S_OK;
}

STDMETHODIMP PinTable::put_BallPlayfieldReflectionScale(float newVal)
{
   STARTUNDO
   m_ballPlayfieldReflectionStrength = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_DefaultBulbIntensityScale(float *pVal)
{
   *pVal = m_defaultBulbIntensityScaleOnBall;
   return S_OK;
}

STDMETHODIMP PinTable::put_DefaultBulbIntensityScale(float newVal)
{
   STARTUNDO
   m_defaultBulbIntensityScaleOnBall = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_BloomStrength(float *pVal)
{
   *pVal = m_bloom_strength;
   return S_OK;
}

STDMETHODIMP PinTable::put_BloomStrength(float newVal)
{
   STARTUNDO
   m_bloom_strength = newVal;
   STOPUNDO

   return S_OK;
}

int PinTable::GetTableSoundVolume() const
{
   return quantizeUnsignedPercent(m_TableSoundVolume);
}

void PinTable::SetTableSoundVolume(const int value)
{
   m_TableSoundVolume = dequantizeUnsignedPercent(value);
}

STDMETHODIMP PinTable::get_TableSoundVolume(int *pVal)
{
   *pVal = GetTableSoundVolume();

   return S_OK;
}

STDMETHODIMP PinTable::put_TableSoundVolume(int newVal)
{
   STARTUNDO
   SetTableSoundVolume(newVal);
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_DetailLevel(int *pVal)
{
   *pVal = m_settings.GetPlayer_AlphaRampAccuracy();
   return S_OK;
}

STDMETHODIMP PinTable::put_DetailLevel(int newVal)
{
   m_settings.SetPlayer_AlphaRampAccuracy(newVal, true);
   return S_OK;
}

STDMETHODIMP PinTable::get_BallDecalMode(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_BallDecalMode);
   return S_OK;
}

STDMETHODIMP PinTable::put_BallDecalMode(VARIANT_BOOL newVal)
{
   STARTUNDO
   m_BallDecalMode = VBTOb(newVal);
   STOPUNDO

   return S_OK;
}

int PinTable::GetTableMusicVolume() const
{
   return quantizeUnsignedPercent(m_TableMusicVolume);
}

void PinTable::SetTableMusicVolume(const int value)
{
   m_TableMusicVolume = dequantizeUnsignedPercent(value);
}

STDMETHODIMP PinTable::get_TableMusicVolume(int *pVal)
{
   *pVal = GetTableMusicVolume();
   return S_OK;
}

STDMETHODIMP PinTable::put_TableMusicVolume(int newVal)
{
   STARTUNDO
   SetTableMusicVolume(newVal);
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_BackdropColor(OLE_COLOR *pVal)
{
   *pVal = m_colorbackdrop;
   return S_OK;
}

STDMETHODIMP PinTable::put_BackdropColor(OLE_COLOR newVal)
{
   STARTUNDO
   m_colorbackdrop = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_BackdropImageApplyNightDay(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_ImageBackdropNightDay);
   return S_OK;
}

STDMETHODIMP PinTable::put_BackdropImageApplyNightDay(VARIANT_BOOL newVal)
{
   STARTUNDO
   m_ImageBackdropNightDay = VBTOb(newVal);
   STOPUNDO

   return S_OK;
}

bool PinTable::IsFSSEnabled() const
{
   return m_isFSSViewModeEnabled;
}

void PinTable::EnableFSS(const bool enable)
{
   m_isFSSViewModeEnabled = enable;
   UpdateCurrentBGSet();
}

void PinTable::UpdateCurrentBGSet()
{
   if (m_viewModeOverride != BG_INVALID)
      m_viewMode = m_viewModeOverride;
   else
   {
      switch (m_settings.GetPlayer_BGSet())
      {
      case 0: m_viewMode = m_isFSSViewModeEnabled ? BG_FSS : BG_DESKTOP; break; // Desktop mode (FSS if table supports it, usual dekstop otherwise)
      case 1: m_viewMode = BG_FULLSCREEN; break; // Cabinet mode
      case 2: m_viewMode = BG_DESKTOP; break; // Desktop mode with FSS disabled (forced desktop)
      }
   }
}

STDMETHODIMP PinTable::get_ShowFSS(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(IsFSSEnabled());
   return S_OK;
}

STDMETHODIMP PinTable::get_BackdropImage_DT(BSTR *pVal)
{
   *pVal = MakeWideBSTR(m_BG_image[0]);
   return S_OK;
}

STDMETHODIMP PinTable::put_BackdropImage_DT(BSTR newVal) //!! HDR??
{
   STARTUNDO
   m_BG_image[0] = MakeString(newVal);
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_BackdropImage_FS(BSTR *pVal)
{
   *pVal = MakeWideBSTR(m_BG_image[1]);
   return S_OK;
}

STDMETHODIMP PinTable::put_BackdropImage_FS(BSTR newVal) //!! HDR??
{
   STARTUNDO
   m_BG_image[1] = MakeString(newVal);
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_BackdropImage_FSS(BSTR *pVal)
{
   *pVal = MakeWideBSTR(m_BG_image[2]);
   return S_OK;
}

STDMETHODIMP PinTable::put_BackdropImage_FSS(BSTR newVal) //!! HDR??
{
   STARTUNDO
   m_BG_image[2] = MakeString(newVal);
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_ColorGradeImage(BSTR *pVal)
{
   *pVal = MakeWideBSTR(m_imageColorGrade);
   return S_OK;
}

STDMETHODIMP PinTable::put_ColorGradeImage(BSTR newVal)
{
   const string szImage = MakeString(newVal);
   const Texture * const tex = GetImage(szImage);
   if (tex && (tex->m_width != 256 || tex->m_height != 16))
   {
       ShowError("Wrong image size, needs to be 256x16 resolution");
       return E_FAIL;
   }

   STARTUNDO
   m_imageColorGrade = szImage;
   STOPUNDO

   return S_OK;
}

float PinTable::GetGravity() const
{
   return m_Gravity * (float)(1.0 / GRAVITYCONST);
}

void PinTable::SetGravity(const float value)
{
   m_Gravity = value * GRAVITYCONST;
}

STDMETHODIMP PinTable::get_Gravity(float *pVal)
{
   *pVal = GetGravity();

   return S_OK;
}

STDMETHODIMP PinTable::put_Gravity(float newVal)
{
   if (newVal < 0.f) newVal = 0.f;

   if (g_pplayer)
   {
      SetGravity(newVal);

      const float slope = (m_overridePhysics ? GetPlayfieldOverridenSlope() : GetPlayfieldSlope());
      const float strength = (m_overridePhysics ? m_fOverrideGravityConstant : m_Gravity);
      g_pplayer->m_physics->SetGravity(slope, strength);
   }
   else
   {
      STARTUNDO
      SetGravity(newVal);
      STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP PinTable::get_Friction(float *pVal)
{
   *pVal = m_friction;
   return S_OK;
}

void PinTable::SetFriction(const float value)
{
   m_friction = clamp(value, 0.0f, 1.0f);
}

STDMETHODIMP PinTable::put_Friction(float newVal)
{
   STARTUNDO
   SetFriction(newVal);
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Elasticity(float *pVal)
{
   *pVal = m_elasticity;
   return S_OK;
}

STDMETHODIMP PinTable::put_Elasticity(float newVal)
{
   STARTUNDO
   m_elasticity = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_ElasticityFalloff(float *pVal)
{
   *pVal = m_elasticityFalloff;
   return S_OK;
}

STDMETHODIMP PinTable::put_ElasticityFalloff(float newVal)
{
   STARTUNDO
   m_elasticityFalloff = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Scatter(float *pVal)
{
   *pVal = m_scatter;
   return S_OK;
}

STDMETHODIMP PinTable::put_Scatter(float newVal)
{
   STARTUNDO
   m_scatter = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_DefaultScatter(float *pVal)
{
   *pVal = m_defaultScatter;
   return S_OK;
}

STDMETHODIMP PinTable::put_DefaultScatter(float newVal)
{
   STARTUNDO
   m_defaultScatter = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_NudgeTime(float *pVal)
{
   *pVal = m_nudgeTime;
   return S_OK;
}

STDMETHODIMP PinTable::put_NudgeTime(float newVal)
{
   STARTUNDO
   m_nudgeTime = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_PhysicsLoopTime(int *pVal)
{
   *pVal = m_PhysicsMaxLoops;
   return S_OK;
}

STDMETHODIMP PinTable::put_PhysicsLoopTime(int newVal)
{
   STARTUNDO
   m_PhysicsMaxLoops = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_BackglassMode(BackglassIndex *pVal)
{
   *pVal = static_cast<BackglassIndex>(static_cast<int>(m_currentBackglassMode) + static_cast<int>(DESKTOP));
   return S_OK;
}

STDMETHODIMP PinTable::put_BackglassMode(BackglassIndex pVal)
{
   m_currentBackglassMode = (ViewSetupID)(pVal - DESKTOP);
   return S_OK;
}

STDMETHODIMP PinTable::get_FieldOfView(float *pVal)
{
   *pVal = mViewSetups[m_currentBackglassMode].mFOV;
   return S_OK;
}

STDMETHODIMP PinTable::put_FieldOfView(float newVal)
{
   STARTUNDO
   mViewSetups[m_currentBackglassMode].mFOV = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Inclination(float *pVal)
{
   *pVal = mViewSetups[m_currentBackglassMode].mLookAt;
   return S_OK;
}

STDMETHODIMP PinTable::put_Inclination(float newVal)
{
   STARTUNDO
   mViewSetups[m_currentBackglassMode].mLookAt = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Layback(float *pVal)
{
   *pVal = mViewSetups[m_currentBackglassMode].mLayback;
   return S_OK;
}

STDMETHODIMP PinTable::put_Layback(float newVal)
{
   STARTUNDO
   mViewSetups[m_currentBackglassMode].mLayback = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Rotation(float *pVal)
{
   *pVal = mViewSetups[m_currentBackglassMode].mViewportRotation;
   return S_OK;
}

STDMETHODIMP PinTable::put_Rotation(float newVal)
{
   STARTUNDO
   mViewSetups[m_currentBackglassMode].mViewportRotation = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Scalex(float *pVal)
{
   *pVal = mViewSetups[m_currentBackglassMode].mSceneScaleX;
   return S_OK;
}

STDMETHODIMP PinTable::put_Scalex(float newVal)
{
   STARTUNDO
   mViewSetups[m_currentBackglassMode].mSceneScaleX = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Scaley(float *pVal)
{
   *pVal = mViewSetups[m_currentBackglassMode].mSceneScaleY;
   return S_OK;
}

STDMETHODIMP PinTable::put_Scaley(float newVal)
{
   STARTUNDO
   mViewSetups[m_currentBackglassMode].mSceneScaleY = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Scalez(float *pVal)
{
   *pVal = mViewSetups[m_currentBackglassMode].mSceneScaleZ;
   return S_OK;
}

STDMETHODIMP PinTable::put_Scalez(float newVal)
{
   STARTUNDO
   mViewSetups[m_currentBackglassMode].mSceneScaleZ = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Xlatex(float *pVal)
{
   *pVal = mViewSetups[m_currentBackglassMode].mViewX;
   return S_OK;
}

STDMETHODIMP PinTable::put_Xlatex(float newVal)
{
   STARTUNDO
   mViewSetups[m_currentBackglassMode].mViewX = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Xlatey(float *pVal)
{
   *pVal = mViewSetups[m_currentBackglassMode].mViewY;
   return S_OK;
}

STDMETHODIMP PinTable::put_Xlatey(float newVal)
{
   STARTUNDO
   mViewSetups[m_currentBackglassMode].mViewY = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Xlatez(float *pVal)
{
   *pVal = mViewSetups[m_currentBackglassMode].mViewZ;
   return S_OK;
}

STDMETHODIMP PinTable::put_Xlatez(float newVal)
{
   STARTUNDO
   mViewSetups[m_currentBackglassMode].mViewZ = newVal;
   STOPUNDO

   return S_OK;
}


STDMETHODIMP PinTable::get_SlopeMax(float *pVal)
{
   *pVal = m_angletiltMax;
   return S_OK;
}

STDMETHODIMP PinTable::put_SlopeMax(float newVal)
{
   if (g_pplayer)
   {
      m_angletiltMax = newVal;
      g_pplayer->m_physics->SetGravity(GetPlayfieldSlope(), m_overridePhysics ? m_fOverrideGravityConstant : m_Gravity);
   }
   else
   {
      STARTUNDO
      m_angletiltMax = newVal;
      STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP PinTable::get_SlopeMin(float *pVal)
{
   *pVal = m_angletiltMin;
   return S_OK;
}

STDMETHODIMP PinTable::put_SlopeMin(float newVal)
{
   if (g_pplayer)
   {
      m_angletiltMin = newVal;
      g_pplayer->m_physics->SetGravity(GetPlayfieldSlope(), m_overridePhysics ? m_fOverrideGravityConstant : m_Gravity);
   }
   else
   {
      STARTUNDO
      m_angletiltMin = newVal;
      STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP PinTable::get_BallImage(BSTR *pVal)
{
   *pVal = MakeWideBSTR(m_ballImage);
   return S_OK;
}

STDMETHODIMP PinTable::put_BallImage(BSTR newVal)
{
   STARTUNDO
   m_ballImage = MakeString(newVal);
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_EnvironmentImage(BSTR *pVal)
{
   *pVal = MakeWideBSTR(m_envImage);
   return S_OK;
}

STDMETHODIMP PinTable::put_EnvironmentImage(BSTR newVal)
{
   const string szImage = MakeString(newVal);
   const Texture * const tex = GetImage(szImage);
   if (tex && (tex->m_width != tex->m_height*2))
   {
       ShowError("Wrong image size, needs to be 2x width in comparison to height");
       return E_FAIL;
   }

   STARTUNDO
   m_envImage = szImage;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_EnableSSR(UserDefaultOnOff *pVal)
{
   *pVal = m_enableSSR ? UserDefaultOnOff::On : UserDefaultOnOff::Off;
   return S_OK;
}

STDMETHODIMP PinTable::put_EnableSSR(UserDefaultOnOff newVal)
{
   if (newVal == UserDefaultOnOff::Default)
      return S_FAIL;
   STARTUNDO
   m_enableSSR = (int)newVal;
   STOPUNDO
   return S_OK;
}

STDMETHODIMP PinTable::get_EnableAO(UserDefaultOnOff *pVal)
{
   *pVal = m_enableAO ? UserDefaultOnOff::On : UserDefaultOnOff::Off;
   return S_OK;
}

STDMETHODIMP PinTable::put_EnableAO(UserDefaultOnOff newVal)
{
   if (newVal == UserDefaultOnOff::Default)
      return S_FAIL;
   STARTUNDO
   m_enableAO = (int)newVal;
   STOPUNDO
   return S_OK;
}

STDMETHODIMP PinTable::get_OverridePhysics(PhysicsSet *pVal)
{
   *pVal = (PhysicsSet)m_overridePhysics;
   return S_OK;
}

STDMETHODIMP PinTable::put_OverridePhysics(PhysicsSet newVal)
{
   STARTUNDO
   m_overridePhysics = (int)newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_OverridePhysicsFlippers(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_overridePhysicsFlipper);
   return S_OK;
}

STDMETHODIMP PinTable::put_OverridePhysicsFlippers(VARIANT_BOOL newVal)
{
   STARTUNDO
   m_overridePhysicsFlipper = VBTOb(newVal);
   STOPUNDO

   return S_OK;
}

//

STDMETHODIMP PinTable::ImportPhysics()
{
   const string& szInitialDir = m_settings.GetRecentDir_PhysicsDir();
   vector<string> filename;
   if (!m_vpinball->OpenFileDialog(szInitialDir, filename, "Visual Pinball Physics (*.vpp)\0*.vpp\0", "vpp", 0))
      return S_OK;

   const size_t index = filename[0].find_last_of(PATH_SEPARATOR_CHAR);
   if (index != string::npos)
      g_pvp->m_settings.SetRecentDir_PhysicsDir(filename[0].substr(0, index), false);

   ImportVPP(filename[0]);

   return S_OK;
}

std::array<string,18> PinTable::VPPelementNames{"gravityConstant"s, "contactFriction"s, "elasticity"s, "elasticityFalloff"s, "playfieldScatter"s, "defaultElementScatter"s, "playfieldminslope"s, "playfieldmaxslope"s,
                               /*flippers:*/    "speed"s, "strength"s, "elasticity"s, "scatter"s, "eosTorque"s, "eosTorqueAngle"s, "returnStrength"s, "elasticityFalloff"s, "friction"s, "coilRampUp"s};

void PinTable::ImportVPP(const string& filename)
{
   tinyxml2::XMLDocument xmlDoc;
   try
   {
      std::stringstream buffer;
      std::ifstream myFile(filename);
      buffer << myFile.rdbuf();
      myFile.close();
      const string xml = buffer.str();

      if (xmlDoc.Parse(xml.c_str()))
      {
         ShowError("Error parsing VPP XML file");
         return;
      }
      const auto root = xmlDoc.FirstChildElement("physics");
      const auto physTab = root->FirstChildElement("table");
      const auto physFlip = root->FirstChildElement("flipper");

      float FlipperPhysicsMass, FlipperPhysicsStrength, FlipperPhysicsElasticity, FlipperPhysicsScatter, FlipperPhysicsTorqueDamping, FlipperPhysicsTorqueDampingAngle, FlipperPhysicsReturnStrength, FlipperPhysicsElasticityFalloff, FlipperPhysicsFriction, FlipperPhysicsCoilRampUp;
      for(size_t i = 0; i < std::size(VPPelementNames); ++i)
      {
         const tinyxml2::XMLElement* el = ((i <= 7) ? physTab : physFlip)->FirstChildElement(VPPelementNames[i].c_str());
         if(el != nullptr)
         {
            const char * const t = el->GetText();
            if (t)
            {
               const float val = sz2f(t);
               switch(i)
               {
               case 0:  put_Gravity(val); break;
               case 1:  put_Friction(val); break;
               case 2:  put_Elasticity(val); break;
               case 3:  put_ElasticityFalloff(val); break;
               case 4:  put_Scatter(val); break;
               case 5:  put_DefaultScatter(val); break;
               case 6:  put_SlopeMin(val); break;
               case 7:  put_SlopeMax(val); break;
               case 8:  FlipperPhysicsMass = val; break;
               case 9:  FlipperPhysicsStrength = val; break;
               case 10: FlipperPhysicsElasticity = val; break;
               case 11: FlipperPhysicsScatter = val; break;
               case 12: FlipperPhysicsTorqueDamping = val; break;
               case 13: FlipperPhysicsTorqueDampingAngle = val; break;
               case 14: FlipperPhysicsReturnStrength = val; break;
               case 15: FlipperPhysicsElasticityFalloff = val; break;
               case 16: FlipperPhysicsFriction = val; break;
               case 17: FlipperPhysicsCoilRampUp = val; break;
               }
            }
         }
         else
         {
            if(i <= 5) //until "defaultElementScatter"
               ShowError(VPPelementNames[i] + " is missing");
            else if(i == 6) //"playfieldminslope"
               put_SlopeMin(DEFAULT_TABLE_MIN_SLOPE); //was added lateron, so don't error
            else if(i == 7) //"playfieldmaxslope"
               put_SlopeMax(DEFAULT_TABLE_MAX_SLOPE); //was added lateron, so don't error
            else //flipper fields
               ShowError("flipper " + VPPelementNames[i] + " is missing");

            //flipper fields need defaults
            switch(i)
            {
            case 8:  FlipperPhysicsMass = 0.0f; break;
            case 9:  FlipperPhysicsStrength = 0.0f; break;
            case 10: FlipperPhysicsElasticity = 0.0f; break;
            case 11: FlipperPhysicsScatter = 0.0f; break;
            case 12: FlipperPhysicsTorqueDamping = 0.0f; break;
            case 13: FlipperPhysicsTorqueDampingAngle = 0.0f; break;
            case 14: FlipperPhysicsReturnStrength = 0.0f; break;
            case 15: FlipperPhysicsElasticityFalloff = 0.0f; break;
            case 16: FlipperPhysicsFriction = 0.0f; break;
            case 17: FlipperPhysicsCoilRampUp = 0.0f; break;
            default: break;
            }
         }
      }

      //assign flipper fields to all flipper elements
      for (size_t i = 0; i < m_vedit.size(); i++)
      if (m_vedit[i]->GetItemType() == eItemFlipper)
      {
         Flipper * const flipper = (Flipper *)m_vedit[i];
         flipper->put_Mass(FlipperPhysicsMass); // was speed previously
         flipper->put_Strength(FlipperPhysicsStrength);
         flipper->put_Elasticity(FlipperPhysicsElasticity);
         flipper->put_Return(FlipperPhysicsReturnStrength);
         flipper->put_ElasticityFalloff(FlipperPhysicsElasticityFalloff);
         flipper->put_Friction(FlipperPhysicsFriction);
         flipper->put_RampUp(FlipperPhysicsCoilRampUp);
         flipper->put_Scatter(FlipperPhysicsScatter);
         flipper->put_EOSTorque(FlipperPhysicsTorqueDamping);
         flipper->put_EOSTorqueAngle(FlipperPhysicsTorqueDampingAngle);
      }
   }
   catch (...)
   {
      ShowError("Error parsing physics settings file");
   }
   xmlDoc.Clear();
}

#define EXPORT_VPP_ELEMENT(getter, idx, tab) \
   { \
   float value; \
   getter(&value); \
   const auto node = xmlDoc.NewElement(VPPelementNames[idx].c_str()); \
   node->SetText(f2sz(value, false).c_str()); \
   tab->InsertEndChild(node); \
}

STDMETHODIMP PinTable::ExportPhysics()
{
#ifndef __STANDALONE__
   bool foundflipper = false;
   size_t i;
   for (i = 0; i < m_vedit.size(); i++)
   {
      if (m_vedit[i]->GetItemType() == eItemFlipper)
      {
         foundflipper = true;
         break;
      }
   }

   if (!foundflipper)
   {
      ShowError("No Flipper found to copy settings from");
      return S_OK;
   }

   Flipper * const flipper = (Flipper *)m_vedit[i];

   char szFileName[MAXSTRING];
   strncpy_s(szFileName, sizeof(szFileName), m_filename.c_str());
   const size_t idx = m_filename.find_last_of('.');
   if (idx != string::npos && idx < MAXSTRING)
      szFileName[idx] = '\0';

   OPENFILENAME ofn = {};
   ofn.lStructSize = sizeof(OPENFILENAME);
   ofn.hInstance = m_vpinball->theInstance;
   ofn.hwndOwner = m_vpinball->GetHwnd();
   // TEXT
   ofn.lpstrFilter = "Visual Pinball Physics (*.vpp)\0*.vpp\0";
   ofn.lpstrFile = szFileName;
   ofn.nMaxFile = sizeof(szFileName);
   ofn.lpstrDefExt = "vpp";
   ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

   string szInitialDir = m_settings.GetRecentDir_PhysicsDir();

   ofn.lpstrInitialDir = szInitialDir.c_str();

   const int ret = GetSaveFileName(&ofn);
   if (ret == 0)
      return S_OK;

   const string filename(ofn.lpstrFile);
   const size_t index = filename.find_last_of(PATH_SEPARATOR_CHAR);
   if (index != string::npos)
   {
       const string newInitDir(filename.substr(0, index));
       g_pvp->m_settings.SetRecentDir_PhysicsDir(newInitDir, false);
   }

   tinyxml2::XMLDocument xmlDoc;

   auto root = xmlDoc.NewElement("physics");
   auto physFlip = xmlDoc.NewElement("flipper");
   auto physTab = xmlDoc.NewElement("table");

   EXPORT_VPP_ELEMENT(get_Gravity, 0, physTab);
   EXPORT_VPP_ELEMENT(get_Friction, 1, physTab);
   EXPORT_VPP_ELEMENT(get_Elasticity, 2, physTab);
   EXPORT_VPP_ELEMENT(get_ElasticityFalloff, 3, physTab);
   EXPORT_VPP_ELEMENT(get_Scatter, 4, physTab);
   EXPORT_VPP_ELEMENT(get_DefaultScatter, 5, physTab);
   EXPORT_VPP_ELEMENT(get_SlopeMin, 6, physTab);
   EXPORT_VPP_ELEMENT(get_SlopeMax, 7, physTab);

   // flippers
   EXPORT_VPP_ELEMENT(flipper->get_Mass, 8, physFlip); // was speed
   EXPORT_VPP_ELEMENT(flipper->get_Strength, 9, physFlip);
   EXPORT_VPP_ELEMENT(flipper->get_Elasticity, 10, physFlip);
   EXPORT_VPP_ELEMENT(flipper->get_Scatter, 11, physFlip);
   EXPORT_VPP_ELEMENT(flipper->get_EOSTorque, 12, physFlip);
   EXPORT_VPP_ELEMENT(flipper->get_EOSTorqueAngle, 13, physFlip);
   EXPORT_VPP_ELEMENT(flipper->get_Return, 14, physFlip);
   EXPORT_VPP_ELEMENT(flipper->get_ElasticityFalloff, 15, physFlip);
   EXPORT_VPP_ELEMENT(flipper->get_Friction, 16, physFlip);
   EXPORT_VPP_ELEMENT(flipper->get_RampUp, 17, physFlip);

   const auto settingName = xmlDoc.NewElement("name");
   settingName->SetText(m_title.c_str());
   root->InsertEndChild(settingName);
   root->InsertEndChild(physTab);
   root->InsertEndChild(physFlip);
   xmlDoc.InsertEndChild(xmlDoc.NewDeclaration());
   xmlDoc.InsertEndChild(root);

   tinyxml2::XMLPrinter prn;
   xmlDoc.Print(&prn);

   std::ofstream myfile(ofn.lpstrFile);
   myfile << prn.CStr();
   myfile.close();
#endif

   return S_OK;
}

//

STDMETHODIMP PinTable::get_EnableDecals(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_renderDecals);
   return S_OK;
}

STDMETHODIMP PinTable::put_EnableDecals(VARIANT_BOOL newVal)
{
   STARTUNDO
   m_renderDecals = VBTOb(newVal);
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_ShowDT(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_viewMode == BG_DESKTOP || m_viewMode == BG_FSS); // DT & FSS
   return S_OK;
}

STDMETHODIMP PinTable::get_EnableEMReels(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_renderEMReels);
   return S_OK;
}

STDMETHODIMP PinTable::put_EnableEMReels(VARIANT_BOOL newVal)
{
   STARTUNDO
   m_renderEMReels = VBTOb(newVal);
   STOPUNDO

   return S_OK;
}

float PinTable::GetGlobalDifficulty() const
{
   return m_globalDifficulty * 100.f;
}

void PinTable::SetGlobalDifficulty(const float value)
{
   m_difficulty = value;
   Settings::SetTableOverride_Difficulty_Default(m_difficulty);
   m_globalDifficulty = m_settings.GetTableOverride_Difficulty();
}

STDMETHODIMP PinTable::get_GlobalDifficulty(float *pVal)
{
   *pVal = GetGlobalDifficulty();
   return S_OK;
}

STDMETHODIMP PinTable::put_GlobalDifficulty(float newVal)
{
   if (!g_pplayer) // VP Editor
   {
       STARTUNDO
       SetGlobalDifficulty(newVal);
       STOPUNDO
   }

   return S_OK;
}

// FIXME deprecated
STDMETHODIMP PinTable::get_Accelerometer(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(false); // Used to be setting Player/PBWEnabled
   return S_OK;
}

// FIXME deprecated
STDMETHODIMP PinTable::put_Accelerometer(VARIANT_BOOL newVal)
{
   return S_OK;
}

// FIXME deprecated
STDMETHODIMP PinTable::get_AccelNormalMount(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(true); // Used to be setting Player/PBWNormalMount
   return S_OK;
}

// FIXME deprecated
STDMETHODIMP PinTable::put_AccelNormalMount(VARIANT_BOOL newVal)
{
   return S_OK;
}

// FIXME deprecated
STDMETHODIMP PinTable::get_AccelerometerAngle(float *pVal)
{
   *pVal = 0.f; // Used to be setting Player/PBWRotationValue
   return S_OK;
}

// FIXME deprecated
STDMETHODIMP PinTable::put_AccelerometerAngle(float newVal)
{
   return S_OK;
}

// FIXME deprecated
STDMETHODIMP PinTable::get_DeadZone(int *pVal)
{
   *pVal = 0; // Used to be setting Player/DeadZone
   return S_OK;
}

STDMETHODIMP PinTable::put_DeadZone(int newVal)
{
   return S_OK;
}

STDMETHODIMP PinTable::get_BallFrontDecal(BSTR *pVal)
{
   *pVal = MakeWideBSTR(m_ballImageDecal);
   return S_OK;
}

STDMETHODIMP PinTable::put_BallFrontDecal(BSTR newVal)
{
   const string szImage = MakeString(newVal);
   const Texture * const tex = GetImage(szImage);
   if (tex && tex->IsHDR())
   {
       ShowError("Cannot use a HDR image (.exr/.hdr) here");
       return E_FAIL;
   }

   STARTUNDO
   m_ballImageDecal = szImage;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::FireKnocker(int Count)
{
   if (g_pplayer)
      ushock_output_knock(Count);

   return S_OK;
}

STDMETHODIMP PinTable::QuitPlayer(int CloseType)
{
   return m_vpinball->QuitPlayer(CloseType);
}

STDMETHODIMP PinTable::get_Version(int *pVal)
{
   *pVal = VP_VERSION_MAJOR * 1000 + VP_VERSION_MINOR * 100 + VP_VERSION_REV;
   return S_OK;
}

STDMETHODIMP PinTable::get_VPBuildVersion(double *pVal)
{
   *pVal = VP_VERSION_MAJOR * 1000 + VP_VERSION_MINOR * 100 + VP_VERSION_REV + GIT_REVISION / 10000.0;
   return S_OK;
}

STDMETHODIMP PinTable::get_VersionMajor(int *pVal)
{
   *pVal = VP_VERSION_MAJOR;
   return S_OK;
}

STDMETHODIMP PinTable::get_VersionMinor(int *pVal)
{
   *pVal = VP_VERSION_MINOR;
   return S_OK;
}

STDMETHODIMP PinTable::get_VersionRevision(int *pVal)
{
   *pVal = VP_VERSION_REV;
   return S_OK;
}

std::optional<VPX::Properties::PropertyRegistry::PropId> PinTable::RegisterOption(
   BSTR optionName, float minValue, float maxValue, float step, float defaultValue, int unit, /*[optional][in]*/ VARIANT values)
{
   if (V_VT(&values) != VT_ERROR && V_VT(&values) != VT_EMPTY && V_VT(&values) != (VT_ARRAY | VT_VARIANT))
      return std::nullopt;
   if (minValue >= maxValue || step <= 0.f || defaultValue < minValue || defaultValue > maxValue)
      return std::nullopt;

   const string name = MakeString(optionName);

   // Prevent invalid characters in the option id
   string optId = trim_string(name);
   std::replace_if(optId.begin(), optId.end(), [](char c) { return !isalnum(c) || c == '.' || c == '-'; }, '_');

   for (const auto& option : m_tableOptions)
   {
      const VPX::Properties::PropertyDef *prop = Settings::GetRegistry().GetProperty(option.id);
      if (prop->m_propId == optId)
      {
         // Update or validate item (re)definition ?
         return option.id;
      }
   }

   vector<string> literals;
   if (V_VT(&values) == (VT_ARRAY | VT_VARIANT))
   {
      if (V_VT(&values) != (VT_ARRAY | VT_VARIANT) || step != 1.f || (minValue - (float)(int)minValue) != 0.f || (maxValue - (float)(int)maxValue) != 0.f)
         return std::nullopt;
      const int nValues = 1 + (int)maxValue - (int)minValue;
      SAFEARRAY *psa = V_ARRAY(&values);
      LONG lbound, ubound;
      if (SafeArrayGetLBound(psa, 1, &lbound) != S_OK || SafeArrayGetUBound(psa, 1, &ubound) != S_OK || ubound != lbound + nValues - 1)
         return std::nullopt;
      VARIANT *p;
      SafeArrayAccessData(psa, (void **)&p);
      literals.reserve(nValues);
      for (int i = 0; i < nValues; i++)
         literals.push_back(MakeString(V_BSTR(&p[i])));
      SafeArrayUnaccessData(psa);
   }

   const bool isPercent = (unit == 1);
   if (!literals.empty())
   {
      // Detect & implement On/Off, True/False, Hide/Show as a toggle
      if (literals.size() == 2)
      {
         string first = lowerCase(trim_string(literals[0]));
         string second = lowerCase(trim_string(literals[1]));
         if ((first == "off" && second == "on") || (first == "hide" && second == "show") || (first == "false" && second == "true"))
         {
            const auto propId = Settings::GetRegistry().Register(std::make_unique<VPX::Properties::BoolPropertyDef>("TableOption"s, optId, name, ""s, true, defaultValue != minValue));
            m_tableOptions.emplace_back(propId, 1.f, ""s, m_settings.GetBool(propId) ? 1.f : 0.f);
            return propId;
         }
         else if ((first == "on" && second == "off") || (first == "show" && second == "hide") || (first == "true" && second == "false"))
         {
            const auto propId = Settings::GetRegistry().Register(std::make_unique<VPX::Properties::BoolPropertyDef>("TableOption"s, optId, name, ""s, true, defaultValue != minValue));
            m_tableOptions.emplace_back(propId, -1.f, ""s, m_settings.GetBool(propId) ? 1.f : 0.f);
            return propId;
         }
      }
      auto prop = std::make_unique<VPX::Properties::EnumPropertyDef>("TableOption"s, optId, name, ""s, true, static_cast<int>(minValue), static_cast<int>(defaultValue), literals);
      const auto propId = Settings::GetRegistry().Register(std::move(prop));
      m_tableOptions.emplace_back(propId, 1.f, ""s, static_cast<float>(m_settings.GetInt(propId)));
      return propId;
   }
   else if (round(step) == 1.f && round(minValue) == minValue)
   {
      auto prop
         = std::make_unique<VPX::Properties::IntPropertyDef>("TableOption"s, optId, name, ""s, true, static_cast<int>(minValue), static_cast<int>(maxValue), static_cast<int>(defaultValue));
      const auto propId = Settings::GetRegistry().Register(std::move(prop));
      m_tableOptions.emplace_back(propId, isPercent ? 100.f : 1.f, isPercent ? "%3d %%"s : "%d"s, static_cast<float>(m_settings.GetInt(propId)));
      return propId;
   }
   else
   {
      auto prop = std::make_unique<VPX::Properties::FloatPropertyDef>("TableOption"s, optId, name, ""s, true, minValue, maxValue, step, defaultValue);
      const auto propId = Settings::GetRegistry().Register(std::move(prop));
      m_tableOptions.emplace_back(propId, isPercent ? 100.f : 1.f, isPercent ? "%4.1f %%"s : "%4.1f"s, m_settings.GetFloat(propId));
      return propId;
   }

   return std::nullopt;
}

const vector<PinTable::TableOption>& PinTable::GetOptions() const
{
   return m_tableOptions;
}

void PinTable::SetOptionLiveValue(VPX::Properties::PropertyRegistry::PropId prop, float value)
{
   for (auto &option : m_tableOptions)
   {
      if ((option.id.type == prop.type) && (option.id.index == prop.index))
      {
         option.value = value;
         FireOptionEvent(OptionEventType::Changed);
         return;
      }
   }
}

STDMETHODIMP PinTable::get_Option(BSTR optionName, float minValue, float maxValue, float step, float defaultValue, int unit, /*[optional][in]*/ VARIANT values, /*[out, retval]*/ float* param)
{
   auto prop = RegisterOption(optionName, minValue, maxValue, step, defaultValue, unit, values);
   if (!prop.has_value())
      return E_FAIL;
   for (const auto& option : m_tableOptions)
   {
      if ((option.id.type == prop.value().type) && (option.id.index == prop.value().index))
      {
         *param = option.value;
         return S_OK;
      }
   }
   return E_FAIL;
}

STDMETHODIMP PinTable::put_Option(BSTR optionName, float minValue, float maxValue, float step, float defaultValue, int unit, /*[optional][in]*/ VARIANT values, /*[in]*/ float val)
{
   auto prop = RegisterOption(optionName, minValue, maxValue, step, defaultValue, unit, values);
   if (!prop.has_value())
      return E_FAIL;
   m_settings.Set(prop.value(), val, true);
   return S_OK;
}

void PinTable::InvokeBallBallCollisionCallback(const HitBall *b1, const HitBall *b2, float hitVelocity)
{
   if (g_pplayer)
   {
      CComPtr<IDispatch> disp;
      m_pcv->m_pScript->GetScriptDispatch(nullptr, &disp);

      static wchar_t FnName[] = L"OnBallBallCollision";
      LPOLESTR fnNames = FnName;

      DISPID dispid;
      const HRESULT hr = disp->GetIDsOfNames(IID_NULL, &fnNames, 1, 0, &dispid);

      if (SUCCEEDED(hr))  // did we find the collision callback function?
      {
         // note: arguments are passed in reverse order
         CComVariant rgvar[3] = {
            CComVariant(hitVelocity),
            CComVariant(static_cast<IDispatch*>(b2->m_pBall)),
            CComVariant(static_cast<IDispatch*>(b1->m_pBall))
         };
         DISPPARAMS dispparams = { rgvar, nullptr, 3, 0 };

         disp->Invoke(dispid, IID_NULL, 0, DISPATCH_METHOD, &dispparams, nullptr, nullptr, nullptr);
      }
   }
}

void PinTable::OnInitialUpdate()
{
    PLOGI << "PinTable OnInitialUpdate"; // For profiling

#ifndef __STANDALONE__
    BeginAutoSaveCounter();
    SetWindowText(m_filename.c_str());
    SetCaption(m_title);
    m_vpinball->SetEnableMenuItems();
#endif
}

BOOL PinTable::OnEraseBkgnd(CDC& dc)
{
   return TRUE;
}

void PinTable::SetMouseCursor()
{
#ifndef __STANDALONE__
    HINSTANCE hinst = m_vpinball->theInstance;
    static int oldTool = -1;

    if(oldTool!=m_vpinball->m_ToolCur)
    {
        char *cursorid;
        if (m_vpinball->m_ToolCur == ID_TABLE_MAGNIFY)
        {
            cursorid = MAKEINTRESOURCE(IDC_MAGNIFY);
        }
        else if (m_vpinball->m_ToolCur == ID_INSERT_TARGET)
        {
            // special case for targets, which are particular walls
            cursorid = MAKEINTRESOURCE(IDC_TARGET);
        }
        else
        {
            const ItemTypeEnum type = EditableRegistry::TypeFromToolID(m_vpinball->m_ToolCur);
            if (type != eItemInvalid)
                cursorid = MAKEINTRESOURCE(EditableRegistry::GetCursorID(type));
            else
            {
                hinst = nullptr;
                cursorid = IDC_ARROW;
            }
        }
        const HCURSOR hcursor = LoadCursor(hinst, cursorid);
        SetClassLongPtr(GCLP_HCURSOR, (LONG_PTR)hcursor);
        SetCursor(hcursor);
        oldTool = m_vpinball->m_ToolCur;
    }
#endif
}

void PinTable::OnLeftButtonDown(const short x, const short y)
{
#ifndef __STANDALONE__
    if ((m_vpinball->m_ToolCur == IDC_SELECT) || (m_vpinball->m_ToolCur == ID_TABLE_MAGNIFY))
    {
        DoLeftButtonDown(x, y, true);
    }
    else if (!IsLocked())
    {
        UseTool(x, y, m_vpinball->m_ToolCur);
    }
    SetFocus();
#endif
}

void PinTable::OnMouseMove(const short x, const short y)
{
#ifndef __STANDALONE__
    const bool middleMouseButtonPressed = ((GetKeyState(VK_MBUTTON) & 0x100) != 0);  //((GetKeyState(VK_MENU) & 0x80000000) != 0);
    if (middleMouseButtonPressed)
    {
        // panning feature starts here...if the user holds the middle mouse button and moves the mouse 
        // everything is moved in the direction of the mouse was moved
        const int dx = abs(m_oldMousePos.x - x);
        const int dy = abs(m_oldMousePos.y - y);
        if (m_oldMousePos.x > x) m_offset.x += (float)dx;
        if (m_oldMousePos.x < x) m_offset.x -= (float)dx;
        if (m_oldMousePos.y > y) m_offset.y += (float)dy;
        if (m_oldMousePos.y < y) m_offset.y -= (float)dy;

        SetDirtyDraw();
        SetMyScrollInfo();

        m_oldMousePos.x = x;
        m_oldMousePos.y = y;
        return;
    }

    DoMouseMove(x, y);
    m_oldMousePos.x = x;
    m_oldMousePos.y = y;
#endif
}

void PinTable::OnMouseWheel(const short x, const short y, const short zDelta)
{
#ifndef __STANDALONE__
    const int ksctrl = GetKeyState(VK_CONTROL);
    if (ksctrl & 0x80000000)
    {
        POINT curpt;
        curpt.x = x;
        curpt.y = y;
        m_mdiTable->ScreenToClient(curpt);
        const short x2 = (short)curpt.x;
        const short y2 = (short)curpt.y;
        if ((m_vpinball->m_ToolCur == IDC_SELECT) || (m_vpinball->m_ToolCur == ID_TABLE_MAGNIFY))
        {
            DoLeftButtonDown(x2, y2, zDelta != -120);
        }
    }
    else
    {
        m_offset.y -= (float)zDelta / m_zoom; // change to orientation to match windows default
        SetDirtyDraw();
        SetMyScrollInfo();
    }
#endif
}

void PinTable::OnSize()
{
    SetMyScrollInfo();
    SetDirtyDraw();
}

void PinTable::ShowWhereImagesUsed(vector<WhereUsedInfo> &vWhereUsed)
{
   for (size_t i = 0; i < m_vimage.size(); i++)
      ShowWhereImageUsed(vWhereUsed, m_vimage[i]);
}

// also change decal special cases below when changing this snippet
#define INSERT_WHERE_USED(x) \
{ \
   whereUsed.searchObjectName = searchObjectName; \
   whereUsed.whereUsedObjectname = m_vedit[i]->GetName(); \
   whereUsed.whereUsedPropertyName = (x); \
   vWhereUsed.push_back(whereUsed); \
}

void PinTable::ShowWhereImageUsed(vector<WhereUsedInfo> &vWhereUsed, Texture *const ppi)
{
   for (size_t i = 0; i < m_vedit.size(); i++)
   {
      WhereUsedInfo whereUsed;
      const IEditable *const pEdit = m_vedit[i];
      if (pEdit == nullptr)
      {
         continue;
      }
      const string& searchObjectName = ppi->m_name; //searchObjectName will be an image or material that we want to find table objects that are using it.

      switch (pEdit->GetItemType())
      {
      case eItemDispReel:
      {
         const DispReel *const pReel = (const DispReel *)pEdit;
         if (StrCompareNoCase(pReel->m_d.m_szImage, searchObjectName))
            INSERT_WHERE_USED("Image"s);
         break;
      }
      case eItemPrimitive:
      {
         const Primitive *const pPrim = (const Primitive *)pEdit;
         const bool image = StrCompareNoCase(pPrim->m_d.m_szImage, searchObjectName);
         if (image || StrCompareNoCase(pPrim->m_d.m_szNormalMap, searchObjectName))
            INSERT_WHERE_USED(image ? "Image"s : "Normal Map"s);
         break;
      }
      case eItemRamp:
      {
         const Ramp *const pRamp = (const Ramp *)pEdit;
         if (StrCompareNoCase(pRamp->m_d.m_szImage, searchObjectName))
            INSERT_WHERE_USED("Image"s);
         break;
      }
      case eItemSurface:
      {
         const Surface *const pSurf = (const Surface *)pEdit;
         const bool image = StrCompareNoCase(pSurf->m_d.m_szImage, searchObjectName);
         if (image || StrCompareNoCase(pSurf->m_d.m_szSideImage, searchObjectName))
            INSERT_WHERE_USED(image ? "Image"s : "Side Image"s);
         break;
      }
      case eItemDecal:
      {
         const Decal *const pDecal = (const Decal *)pEdit;
         if (StrCompareNoCase(pDecal->m_d.m_szImage, searchObjectName))
            INSERT_WHERE_USED("Image"s);
         break;
      }
      case eItemFlasher:
      {
         const Flasher *const pFlash = (const Flasher *)pEdit;
         const bool imageA = StrCompareNoCase(pFlash->m_d.m_szImageA, searchObjectName);
         if (imageA || StrCompareNoCase(pFlash->m_d.m_szImageB, searchObjectName))
            INSERT_WHERE_USED(imageA ? "ImageA"s : "ImageB"s);
         break;
      }
      case eItemFlipper:
      {
         const Flipper *const pFlip = (const Flipper *)pEdit;
         if (StrCompareNoCase(pFlip->m_d.m_szImage, searchObjectName))
            INSERT_WHERE_USED("Image"s);
         break;
      }
      case eItemHitTarget:
      {
         const HitTarget *const pHit = (const HitTarget *)pEdit;
         if (StrCompareNoCase(pHit->m_d.m_szImage, searchObjectName))
            INSERT_WHERE_USED("Image"s);
         break;
      }
      case eItemLight:
      {
         const Light *const pLight = (const Light *)pEdit;
         if (StrCompareNoCase(pLight->m_d.m_szImage, searchObjectName))
            INSERT_WHERE_USED("Image"s);
         break;
      }
      case eItemPlunger:
      {
         const Plunger *const pPlung = (const Plunger *)pEdit;
         if (StrCompareNoCase(pPlung->m_d.m_szImage, searchObjectName))
            INSERT_WHERE_USED("Image"s);
         break;
      }
      case eItemRubber:
      {
         const Rubber *const pRub = (const Rubber *)pEdit;
         if (StrCompareNoCase(pRub->m_d.m_szImage, searchObjectName))
            INSERT_WHERE_USED("Image"s);
         break;
      }
      case eItemSpinner:
      {
         const Spinner *const pSpin = (const Spinner *)pEdit;
         if (StrCompareNoCase(pSpin->m_d.m_szImage, searchObjectName))
            INSERT_WHERE_USED("Image"s);
         break;
      }
      default:
      {
         break;
      }
      }
   }
}

void PinTable::ShowWhereMaterialsUsed(vector<WhereUsedInfo> &vWhereUsed)
{
   for (size_t i = 0; i < m_materials.size(); i++)
      ShowWhereMaterialUsed(vWhereUsed, m_materials[i]);
}

void PinTable::ShowWhereMaterialUsed(vector<WhereUsedInfo> &vWhereUsed, Material *const ppi)
{
   for (size_t i = 0; i < m_vedit.size(); i++)
   {
      WhereUsedInfo whereUsed;
      const IEditable *const pEdit = m_vedit[i];
      if (pEdit == nullptr)
      {
         continue;
      }
      const string& searchObjectName = ppi->m_name; //searchObjectName will be an image or material that we want to find table objects that are using it.

      switch (pEdit->GetItemType())
      {
      case eItemBumper:
      {
         const Bumper *const pBumper = (const Bumper *)pEdit;
         const bool capmat   = StrCompareNoCase(pBumper->m_d.m_szCapMaterial, searchObjectName);
         const bool basemat  = StrCompareNoCase(pBumper->m_d.m_szBaseMaterial, searchObjectName);
         const bool skirtmat = StrCompareNoCase(pBumper->m_d.m_szSkirtMaterial, searchObjectName);
         if (capmat || basemat || skirtmat || StrCompareNoCase(pBumper->m_d.m_szRingMaterial, searchObjectName))
            INSERT_WHERE_USED(capmat ? "Cap Material"s : (basemat ? "Base Material"s : (skirtmat ? "Skirt Material"s : "Ring Material"s)));
         break;
      }
      case eItemPrimitive:
      {
         const Primitive *const pPrim = (const Primitive *)pEdit;
         const bool mat = StrCompareNoCase(pPrim->m_d.m_szMaterial, searchObjectName);
         if (mat || StrCompareNoCase(pPrim->m_d.m_szPhysicsMaterial, searchObjectName))
            INSERT_WHERE_USED(mat ? "Material"s : "Physics Material"s);
         break;
      }
      case eItemRamp:
      {
         const Ramp *const pRamp = (const Ramp *)pEdit;
         const bool mat = StrCompareNoCase(pRamp->m_d.m_szMaterial, searchObjectName);
         if (mat || StrCompareNoCase(pRamp->m_d.m_szPhysicsMaterial, searchObjectName))
            INSERT_WHERE_USED(mat ? "Material"s : "Physics Material"s);
         break;
      }
      case eItemSurface: //'Wall' table objects are surfaces
      {
         const Surface *const pSurf = (const Surface *)pEdit;
         const bool topmat   = StrCompareNoCase(pSurf->m_d.m_szTopMaterial, searchObjectName);
         const bool sidemat  = StrCompareNoCase(pSurf->m_d.m_szSideMaterial, searchObjectName);
         const bool slingmat = StrCompareNoCase(pSurf->m_d.m_szSlingShotMaterial, searchObjectName);
         if (topmat || sidemat || slingmat || StrCompareNoCase(pSurf->m_d.m_szPhysicsMaterial, searchObjectName))
            INSERT_WHERE_USED(topmat ? "Top Material"s : (sidemat ? "Side Material"s : (slingmat ? "Slingshot Material"s : "Physics Material"s)));
         break;
      }
      case eItemDecal:
      {
         const Decal *const pDecal = (const Decal *)pEdit;
         if (StrCompareNoCase(pDecal->m_d.m_szMaterial, searchObjectName))
            INSERT_WHERE_USED("Material"s);
         break;
      }
      case eItemFlipper:
      {
         const Flipper *const pFlip = (const Flipper *)pEdit;
         const bool mat = StrCompareNoCase(pFlip->m_d.m_szMaterial, searchObjectName);
         if (mat || StrCompareNoCase(pFlip->m_d.m_szRubberMaterial, searchObjectName))
            INSERT_WHERE_USED(mat ? "Material"s : "Rubber Material"s);
         break;
      }
      case eItemHitTarget:
      {
         const HitTarget *const pHit = (const HitTarget *)pEdit;
         const bool mat = StrCompareNoCase(pHit->m_d.m_szMaterial, searchObjectName);
         if (mat || StrCompareNoCase(pHit->m_d.m_szPhysicsMaterial, searchObjectName))
            INSERT_WHERE_USED(mat ? "Material"s : "Physics Material"s);
         break;
      }
      case eItemPlunger:
      {
         const Plunger *const pPlung = (const Plunger *)pEdit;
         if (StrCompareNoCase(pPlung->m_d.m_szMaterial, searchObjectName))
            INSERT_WHERE_USED("Material"s);
         break;
      }
      case eItemRubber:
      {
         const Rubber *const pRub = (const Rubber *)pEdit;
         const bool mat = StrCompareNoCase(pRub->m_d.m_szMaterial, searchObjectName);
         if (mat || StrCompareNoCase(pRub->m_d.m_szPhysicsMaterial, searchObjectName))
            INSERT_WHERE_USED(mat ? "Material"s : "Physics Material"s);
         break;
      }
      case eItemSpinner:
      {
         const Spinner *const pSpin = (const Spinner *)pEdit;
         const bool mat = StrCompareNoCase(pSpin->m_d.m_szMaterial, searchObjectName);
         if (mat || StrCompareNoCase(pSpin->m_d.m_szPhysicsMaterial, searchObjectName))
            INSERT_WHERE_USED(mat ? "Material"s : "Physics Material"s);
         break;
      }
      case eItemKicker:
      {
         const Kicker *const pKicker = (const Kicker *)pEdit;
         if (StrCompareNoCase(pKicker->m_d.m_szMaterial, searchObjectName))
            INSERT_WHERE_USED("Material"s);
         break;
      }
      case eItemTrigger:
      {
         const Trigger *const pTrigger = (const Trigger *)pEdit;
         if (StrCompareNoCase(pTrigger->m_d.m_szMaterial, searchObjectName))
            INSERT_WHERE_USED("Material"s);
         break;
      }
      default:
      {
         break;
      }
      }
   }
}



////////////////////////////////////////////////////////////////////////////////////////////////
// Deprecated properties exposed to scripting API

STDMETHODIMP PinTable::get_ReflectElementsOnPlayfield(VARIANT_BOOL *pVal)
{
   PLOGE << "ReflectElementsOnPlayfield is deprecated";
   *pVal = FTOVB(true);
   return S_OK;
}

STDMETHODIMP PinTable::put_ReflectElementsOnPlayfield(VARIANT_BOOL newVal)
{
   PLOGE << "ReflectElementsOnPlayfield is deprecated";
   return S_OK;
}

STDMETHODIMP PinTable::get_YieldTime(LONG *pVal)
{
   PLOGE << "YieldTime is deprecated";
   if (!g_pplayer)
   {
      *pVal = NULL;
      return E_FAIL;
   }
   else
      *pVal = 0;
   return S_OK;
}

STDMETHODIMP PinTable::put_YieldTime(LONG newVal)
{
   PLOGE << "YieldTime is deprecated";
   if (!g_pplayer)
      return E_FAIL;
   return S_OK;
}

STDMETHODIMP PinTable::get_TableHeight(float *pVal)
{
   PLOGE << "TableHeight is deprecated";
   *pVal = 0.f;
   return S_OK;
}

STDMETHODIMP PinTable::put_TableHeight(float newVal)
{
   PLOGE << "TableHeight is deprecated";
   return S_OK;
}

STDMETHODIMP PinTable::get_TableAdaptiveVSync(int *pVal)
{
   PLOGE << "TableAdaptiveVSync is deprecated";
   *pVal = -1;
   return S_OK;
}

STDMETHODIMP PinTable::put_TableAdaptiveVSync(int newVal)
{
   PLOGE << "TableAdaptiveVSync is deprecated";
   return S_OK;
}

STDMETHODIMP PinTable::get_GlobalAlphaAcc(VARIANT_BOOL *pVal)
{
   PLOGE << "GlobalAlphaAcc is deprecated";
   *pVal = (VARIANT_BOOL)-1;
   return S_OK;
}

STDMETHODIMP PinTable::put_GlobalAlphaAcc(VARIANT_BOOL newVal)
{
   PLOGE << "GlobalAlphaAcc is deprecated";
   return S_OK;
}

STDMETHODIMP PinTable::get_GlobalDayNight(VARIANT_BOOL *pVal)
{
   PLOGE << "GlobalDayNight is deprecated";
   *pVal = (VARIANT_BOOL)0;
   return S_OK;
}

STDMETHODIMP PinTable::put_GlobalDayNight(VARIANT_BOOL newVal)
{
   PLOGE << "GlobalDayNight is deprecated";
   return S_OK;
}

STDMETHODIMP PinTable::get_GlobalStereo3D(VARIANT_BOOL *pVal)
{
   PLOGE << "GlobalStereo3D is deprecated";
   *pVal = (VARIANT_BOOL)0;
   return S_OK;
}

STDMETHODIMP PinTable::put_GlobalStereo3D(VARIANT_BOOL newVal)
{
   PLOGE << "GlobalStereo3D is deprecated";
   return S_OK;
}

STDMETHODIMP PinTable::get_MaxSeparation(float *pVal)
{
   PLOGE << "MaxSeparation is deprecated";
   *pVal = 0.f;
   return S_OK;
}

STDMETHODIMP PinTable::put_MaxSeparation(float newVal)
{
   PLOGE << "MaxSeparation is deprecated";
   return S_OK;
}

STDMETHODIMP PinTable::get_ZPD(float *pVal)
{
   PLOGE << "ZPD is deprecated";
   *pVal = 0.f;
   return S_OK;
}

STDMETHODIMP PinTable::put_ZPD(float newVal)
{
   PLOGE << "ZPD is deprecated";
   return S_OK;
}

STDMETHODIMP PinTable::get_Offset(float *pVal)
{
   PLOGE << "3D Offset is deprecated";
   *pVal = 0.f;
   return S_OK;
}

STDMETHODIMP PinTable::put_Offset(float newVal)
{
   PLOGE << "3D Offset is deprecated";
   return S_OK;
}

STDMETHODIMP PinTable::get_PlungerFilter(VARIANT_BOOL *pVal)
{
   PLOGE << "PlungerFilter is deprecated";
   *pVal = (VARIANT_BOOL)0;
   return S_OK;
}

STDMETHODIMP PinTable::put_PlungerFilter(VARIANT_BOOL newVal)
{
   PLOGE << "PlungerFilter is deprecated";
   return S_OK;
}

// Changing AA & FXAA is somewhat wrong as it changes the setting for all time, and is not implemented while playing, so this is just a No-Op
STDMETHODIMP PinTable::get_EnableAntialiasing(UserDefaultOnOff *pVal)
{
   *pVal = UserDefaultOnOff::Default;
   return S_OK;
}

// Changing AA & FXAA is somewhat wrong as it changes the setting for all time, and is not implemented while playing, so this is just a No-Op
STDMETHODIMP PinTable::put_EnableAntialiasing(UserDefaultOnOff newVal)
{
   return S_OK;
}

// Changing AA & FXAA is somewhat wrong as it changes the setting for all time, and is not implemented while playing, so this is just a No-Op
STDMETHODIMP PinTable::get_EnableFXAA(FXAASettings *pVal)
{
   *pVal = FXAASettings::Defaults;
   return S_OK;
}

// Changing AA & FXAA is somewhat wrong as it changes the setting for all time, and is not implemented while playing, so this is just a No-Op
STDMETHODIMP PinTable::put_EnableFXAA(FXAASettings newVal)
{
   return S_OK;
}

STDMETHODIMP PinTable::get_PlungerNormalize(int *pVal)
{
   *pVal = 100;
   return S_OK;
}

STDMETHODIMP PinTable::put_PlungerNormalize(int newVal)
{
   return S_OK;
}
