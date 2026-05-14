// license:GPLv3+

#include "core/stdafx.h"
#include "pintable.h"

#include <algorithm>
#include <fstream>
#include <sstream>

#include "core/ScriptGlobalTable.h"
#include "core/vpversion.h"
#include "parts/ball.h"
#include "parts/bumper.h"
#include "parts/decal.h"
#include "parts/dispreel.h"
#include "parts/flasher.h"
#include "parts/flipper.h"
#include "parts/gate.h"
#include "parts/hittarget.h"
#include "parts/kicker.h"
#include "parts/light.h"
#include "parts/PartGroup.h"
#include "parts/plunger.h"
#include "parts/primitive.h"
#include "parts/ramp.h"
#include "parts/rubber.h"
#include "parts/Sound.h"
#include "parts/spinner.h"
#include "parts/surface.h"
#include "parts/textbox.h"
#include "parts/timer.h"
#include "parts/trigger.h"
#include "ThreadPool.h"
#include "tinyxml2/tinyxml2.h"
#include "ui/VPXFileFeedback.h"
#include "ui/win/codeview.h"
#include "ui/win/DragPointDialogs.h"
#include "ui/win/hitsur.h"
#include "ui/win/PinTableWnd.h"
#include "ui/win/resource.h"
#include "ui/win/WinEditor.h"
#include "utils/BiffReader.h"
#include "utils/BiffWriter.h"
#include "utils/hash.h"
#include "utils/objloader.h"
#include "utils/ushock_output.h"

#ifndef __STANDALONE__
#include "ui/win/dialogs/VPXLoadFileProgressBar.h"
#include "ui/win/dialogs/VPXSaveFileProgressBar.h"
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
   : m_settings(&(g_app->m_settings))
   , m_undo(this)
{
   m_renderSolid = m_settings.GetEditor_RenderSolid();
   ClearMultiSel();

   SetDefaultPhysics(false);

   m_PhysicsMaxLoops = m_settings.GetPlayer_PhysicsMaxLoops();

   UpdateCurrentBGSet();

   CComObject<ScriptGlobalTable>::CreateInstance(&m_psgt);
   m_psgt->AddRef();
   m_psgt->Init(this);
   m_scriptableNames[L"debug"s] = nullptr; // Debug global object (for Debug.Print)
   for (const wstring& methodName : m_psgt->GetMethodNames()) // Add all global methods as reserved keywords
      m_scriptableNames[lowerCase(methodName)] = nullptr;

   Settings::SetTableOverride_Difficulty_Default(m_difficulty);
   m_globalDifficulty = m_settings.GetTableOverride_Difficulty();

   m_tblAutoStart = m_settings.GetPlayer_Autostart() * 10;
   m_tblAutoStartRetry = m_settings.GetPlayer_AutostartRetry() * 10;
   m_tblAutoStartEnabled = m_settings.GetPlayer_asenable();

   m_tblNudgeRead = Vertex2D(0.f,0.f);
   m_tblNudgePlumb = Vertex2D(0.f,0.f);

   m_dummyMaterial = std::make_unique<Material>();
   m_dummyMaterial->m_cBase = g_app->m_settings.GetEditor_DefaultMaterialColor();
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

   m_psgt->Release();
   m_psgt = nullptr;

   if (m_liveBaseTable)
      m_liveBaseTable->Release();
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

void PinTable::SetMouseCapture()
{
#ifndef __STANDALONE__
   m_tableEditor->SetCapture();
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
for (const auto item : m_vedit) \
{ \
    if (item->GetItemType() == eItemSurface || item->GetItemType() == eItemRamp) \
    { \
        if (es == item->GetIScriptable()->m_wzName) \
        { \
            found = true; \
            break; \
        } \
    } \
} \
if(!found) \
    pEditSurface.clear(); \
}}


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

   for (auto pEdit : m_vedit)
   {
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

void PinTable::AddPart(IEditable *const part)
{
   assert(std::ranges::find(m_vedit, part) == m_vedit.end());
   assert(part->m_ptable == nullptr);
   part->AddRef();
   part->m_ptable = this;
   m_vedit.push_back(part);
   if (auto scriptable = part->GetIScriptable(); scriptable)
   {
      assert(!scriptable->m_wzName.empty());
      const auto id = lowerCase(scriptable->m_wzName);
      assert(m_scriptableNames.find(id) == m_scriptableNames.end());
      m_scriptableNames[id] = part;
      if (m_tableEditor)
         m_tableEditor->m_pcv->AddItem(scriptable, false);
   }
}

void PinTable::RemovePart(IEditable *const part)
{
   auto it2 = std::ranges::find(m_vedit, part);
   assert(it2 != m_vedit.end());
   assert(part->m_ptable == this);
   m_vedit.erase(it2);
   if (auto scriptable = part->GetIScriptable(); scriptable)
   {
      assert(!part->GetIScriptable()->m_wzName.empty());
      auto it = m_scriptableNames.find(lowerCase(scriptable->m_wzName));
      assert(it != m_scriptableNames.end());
      m_scriptableNames.erase(it);
      if (m_tableEditor)
         m_tableEditor->m_pcv->RemoveItem(scriptable);
   }
   part->m_ptable = nullptr;
   part->Release();
}

void PinTable::RenamePart(IEditable *const part, const wstring& newName)
{
   auto scriptable = part->GetIScriptable();
   assert(scriptable);
   assert(!scriptable->m_wzName.empty());
   auto it = m_scriptableNames.find(lowerCase(scriptable->m_wzName));
   assert(it != m_scriptableNames.end());
   m_scriptableNames.erase(it);
   const auto id = lowerCase(newName);
   assert(m_scriptableNames.find(id) == m_scriptableNames.end());
   m_scriptableNames[id] = part;
   scriptable->m_wzName = newName;
   if (m_tableEditor)
      m_tableEditor->m_pcv->ReplaceName(scriptable, newName);
}

void PinTable::MovePartToFront(IEditable* part)
{
   RemoveFromVectorSingle(m_vedit, part);
   m_vedit.push_back(part);
   SetDirtyDraw();
}

void PinTable::MovePartToBack(IEditable* part)
{
   RemoveFromVectorSingle(m_vedit, part);
   m_vedit.insert(m_vedit.begin(), part);
   SetDirtyDraw();
}

void PinTable::ReorderParts(bool isDrawingOrder)
{
   SetNonUndoableDirty(eSaveDirty);
   if (isDrawingOrder)
   {
      for (int i = m_vmultisel.size() - 1; i >= 0; i--)
      {
         IEditable *const pedit = m_vmultisel[i].GetIEditable();
         RemoveFromVectorSingle(m_vedit, pedit);
      }

      for (int i = m_vmultisel.size() - 1; i >= 0; i--)
      {
         IEditable *const pedit = m_vmultisel[i].GetIEditable();
         m_vedit.push_back(pedit);
      }
   }
   else
   {
      for (SSIZE_T i = m_allHitElements.size() - 1; i >= 0; i--)
      {
         IEditable *const pedit = m_allHitElements[i]->GetIEditable();
         RemoveFromVectorSingle(m_vedit, pedit);
      }

      for (SSIZE_T i = m_allHitElements.size() - 1; i >= 0; i--)
      {
         IEditable *const pedit = m_allHitElements[i]->GetIEditable();
         m_vedit.push_back(pedit);
      }
   }
}

void PinTable::AddCollection(Collection* collection)
{
   const auto id = lowerCase(collection->m_wzName);
   assert(m_scriptableNames.find(id) == m_scriptableNames.end());
   collection->AddRef();
   m_vcollection.push_back(collection);
   m_scriptableNames[id] = nullptr;
   if (m_tableEditor)
      m_tableEditor->m_pcv->AddItem((IScriptable *)collection, false);
}

void PinTable::RemoveCollection(Collection *collection)
{
#ifndef __STANDALONE__
   auto it = m_scriptableNames.find(lowerCase(collection->m_wzName));
   assert(it != m_scriptableNames.end());
   m_scriptableNames.erase(it);
   if (m_tableEditor)
      m_tableEditor->m_pcv->RemoveItem((IScriptable *)collection);
   m_vcollection.find_erase(collection);
   collection->Release();
#endif
}

void PinTable::RenameCollection(Collection *collection, const wstring &newName)
{
   assert(!collection->m_wzName.empty());
   auto it = m_scriptableNames.find(lowerCase(collection->m_wzName));
   assert(it != m_scriptableNames.end());
   m_scriptableNames.erase(it);
   const auto id = lowerCase(newName);
   assert(m_scriptableNames.find(id) == m_scriptableNames.end());
   m_scriptableNames[id] = nullptr;
   collection->m_wzName = newName;
   if (m_tableEditor)
      m_tableEditor->m_pcv->ReplaceName(collection, newName);
}

bool PinTable::IsNameUnique(const wstring &name) const
{return m_scriptableNames.find(lowerCase(name)) == m_scriptableNames.end(); }

void PinTable::GetUniqueName(const ItemTypeEnum type, wstring &wzUniqueName) const
{
   const wstring root = GetTypeNameForType(type);
   wzUniqueName = GetUniqueName(root);
}

wstring PinTable::GetUniqueName(const wstring &wzRoot) const
{
   int suffix = 1;
   wstring wzName;
   do
   {
      wzName = (wzRoot.length() > MAXNAMEBUFFER - 3 ? wzRoot.substr(0, MAXNAMEBUFFER - 3) : wzRoot)
         + ((suffix <  10) ? (L"00" + std::to_wstring(suffix))
         :  (suffix < 100) ? (L"0"  + std::to_wstring(suffix))
         :                            std::to_wstring(suffix));
      suffix++;
   } while (!IsNameUnique(wzName) && suffix < 1000);
   return wzName;
}

void PinTable::SetDirtyDraw()
{
   if (g_pplayer == nullptr && m_tableEditor != nullptr)
      m_tableEditor->Redraw();
}

PinTable* PinTable::CopyForPlay()
{
   PinTable * const src = this;
   CComObject<PinTable> *live_table;
   CComObject<PinTable>::CreateInstance(&live_table);
   live_table->AddRef();
   live_table->m_liveBaseTable = this;
   AddRef(); // as the live table holds a reference on this

   CComObject<PinTable> *dst = live_table;
   
   dst->m_original_table_script = src->m_original_table_script;
   dst->m_external_script_name = src->m_external_script_name;
   dst->m_script_text = src->m_script_text;

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
   dst->m_enableAO = src->m_enableAO;
   dst->m_enableSSR = src->m_enableSSR;
   dst->m_toneMapper = src->m_toneMapper;
   dst->m_exposure = src->m_exposure;
   dst->m_bloom_strength = src->m_bloom_strength;
   dst->m_wzName = src->m_wzName;

   dst->m_Light[0].emission = src->m_Light[0].emission;

   dst->m_isFSSViewModeEnabled = src->m_isFSSViewModeEnabled;
   dst->m_viewModeOverride = src->m_viewModeOverride;
   dst->UpdateCurrentBGSet();
   for (int i = 0; i < 3; i++)
   {
      dst->mViewSetups[i] = src->mViewSetups[i];
      dst->m_BG_image[i] = src->m_BG_image[i];
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
   for (IEditable* const editable : src->m_vedit)
   {
      IEditable* const edit_dst = editable->CopyForPlay();
      if (editable->GetPartGroup())
      {
         PartGroup *dstParent = static_cast<PartGroup *>(dst->GetLiveFromStartup<IEditable>(editable->GetPartGroup()));
         assert(dstParent != nullptr);
         edit_dst->SetPartGroup(dstParent);
      }
      live_table->AddPart(edit_dst);
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
      pcol->m_wzName = m_vcollection[i].m_wzName;
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
      live_table->AddCollection(pcol);
   }

   if (live_table->m_tableEditor)
   {
      live_table->m_tableEditor->m_pcv->AddItem(live_table, false);
      live_table->m_tableEditor->m_pcv->AddItem(live_table->m_psgt, true);
      //live_table->m_tableEditor->m_pcv->AddItem(live_table->m_tableEditor->m_pcv->m_pdm, false);
   }

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

   return live_table;
}

void PinTable::SetupLookUpTables(bool isPlaying)
{
   m_textureMap.clear();
   m_materialMap.clear();
   m_lightMap.clear();
   m_renderprobeMap.clear();
   if (isPlaying)
   {
      // set up the texture & material hashtables for faster access
      for (size_t i = 0; i < m_vimage.size(); i++)
         m_textureMap[m_vimage[i]->m_name] = m_vimage[i];
      for (size_t i = 0; i < m_materials.size(); i++)
         m_materialMap[m_materials[i]->m_name] = m_materials[i];
      for (auto pe : m_vedit)
         if (pe->GetItemType() == ItemTypeEnum::eItemLight)
            m_lightMap[pe->GetName()] = (Light *)pe;
      for (size_t i = 0; i < m_vrenderprobe.size(); i++)
         m_renderprobeMap[m_vrenderprobe[i]->GetName()] = m_vrenderprobe[i];
   }
}

HRESULT PinTable::Save()
{
#ifndef __STANDALONE__
   // Get file name if needed
   std::filesystem::path vpxPath = m_filename;
   vpxPath.replace_extension(".vpx");

   STGOPTIONS stg;
   stg.usVersion = 1;
   stg.reserved = 0;
   stg.ulSectorSize = 4096;

   HRESULT hr;
   IStorage* pstgRoot;
   if (FAILED(hr = StgCreateStorageEx(vpxPath.wstring().c_str(), STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE,
      STGFMT_DOCFILE, 0, &stg, nullptr, IID_IStorage, (void**)&pstgRoot)))
   {
      ShowError(LocalString(IDS_SAVEERROR).m_szbuffer);
      return hr;
   }

   m_vpinball->SetActionCur(LocalString(IDS_SAVING).m_szbuffer);
   m_vpinball->SetCursorCur(nullptr, IDC_WAIT);

   RemoveInvalidReferences();

   hr = SaveToStorage(pstgRoot);
   if (SUCCEEDED(hr))
   {
      pstgRoot->Commit(STGC_DEFAULT);
      pstgRoot->Release();

      m_undo.SetCleanPoint(eSaveClean);
      if (m_tableEditor)
         m_tableEditor->m_pcv->SetClean(eSaveClean);
      SetNonUndoableDirty(eSaveClean);
   }

   m_vpinball->SetActionCur(string());
   m_vpinball->SetCursorCur(nullptr, IDC_ARROW);
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
   VPXSaveFileProgressBar feedback(g_app->GetInstanceHandle(), m_vpinball->m_hwndStatusBar, m_tableEditor);
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

         BiffWriter writer(pstmGame, hch);
         Save(writer, false);
         if (!writer.HasError())
         {
            // Move PartGroup ahead of objects they contain, so that they are saved first
            std::ranges::stable_partition(m_vedit.begin(), m_vedit.end(), [](IEditable *p) { return p->GetItemType() == ItemTypeEnum::eItemPartGroup; });
            for (size_t i = 0; i < m_vedit.size(); i++)
            {
               const wstring wStmName = L"GameItem" + std::to_wstring(i);

               if (SUCCEEDED(hr = pstgData->CreateStream(wStmName.c_str(), STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstmItem)))
               {
                  ULONG writ;
                  IEditable *const piedit = m_vedit[i];
                  const ItemTypeEnum type = piedit->GetItemType();
                  pstmItem->Write(&type, sizeof(int), &writ);
                  BiffWriter writer(pstmItem, 0);
                  piedit->Save(writer, false);
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
                  BiffWriter imageWriter(pstmItem, 0);
                  m_vimage[i]->Save(imageWriter, this);
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
                  BiffWriter writer(pstmItem, 0);
                  m_vfont[i]->Save(writer);
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
                  BiffWriter writer(pstmItem, hch);
                  m_vcollection[i].Save(writer, false);
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

   BiffWriter writer(pstm, hcrypthash);
   const wstring wzT = MakeWString(szValue);

#if (WCHAR_T_SIZE == 4) // Linux, macOS
   const std::u16string wzT_utf16 = utf32_to_utf16(wzT);
   writer.WriteBytes(wzT_utf16.c_str(), static_cast<ULONG>(wzT_utf16.length() * 2));
#else // Windows
   writer.WriteBytes(wzT.c_str(), static_cast<ULONG>(wzT.length() * sizeof(WCHAR)));
#endif

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
   buffer[strnlen_s(buffer,std::size(buffer))-1] = '\0'; // remove line break
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
         BiffWriter writer(pstm, hcrypthash);
         writer.WriteBytes(pin->GetFileRaw(), static_cast<ULONG>(pin->GetFileSize()));
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
   BiffWriter writer(pstmTags, hcrypthash);
   for (size_t i = 0; i < m_vCustomInfoTag.size(); i++)
      writer.WriteString(FID(CUST), m_vCustomInfoTag[i]);
   writer.EndObject();

   for (size_t i = 0; i < m_vCustomInfoTag.size(); i++)
      WriteInfoValue(pstg, MakeWString(m_vCustomInfoTag[i]), m_vCustomInfoContent[i], hcrypthash);

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

#if (WCHAR_T_SIZE == 4)
      const int len = ss.cbSize.LowPart / 2;
#else
      const int len = ss.cbSize.LowPart / (DWORD)sizeof(WCHAR);
#endif
      BiffReader br(pstm, 0, hcrypthash, NULL);
#if (WCHAR_T_SIZE == 4)
      char16_t *const wzT_u16 = new char16_t[len + 1];
      memset(wzT_u16, 0, sizeof(char16_t) * (len + 1));
      br.ReadBytes(wzT_u16, ss.cbSize.LowPart);
      wzT_u16[len] = u'\0';
      output = MakeString(utf16_to_utf32(wzT_u16));
      delete[] wzT_u16;
#else
      WCHAR *const wzT = new WCHAR[len + 1];
      memset(wzT, 0, sizeof(WCHAR) * (len + 1));
      br.ReadBytes(wzT, ss.cbSize.LowPart);
      wzT[len] = L'\0';
      output = MakeString(wzT);
      delete[] wzT;
#endif

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
         = Settings::GetRegistry().Register(std::make_unique<VPX::Properties::StringPropertyDef>("Version"s, optId, "Table Version"s, "Last played version"s, true, m_version));
      g_app->m_settings.Set(propId, m_version, false);
   }

   HRESULT hr;
   IStream *pstm;

   if (SUCCEEDED(hr = pstg->OpenStream(L"Screenshot", nullptr, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstm)))
   {
      STATSTG ss;
      pstm->Stat(&ss, STATFLAG_NONAME);
      m_pbTempScreenshot = new PinBinary();
      m_pbTempScreenshot->m_buffer.resize(ss.cbSize.LowPart);
      BiffReader br(pstm, 0, hcrypthash, 0);
      br.ReadBytes(m_pbTempScreenshot->m_buffer.data(), static_cast<uint32_t>(m_pbTempScreenshot->m_buffer.size()));
      pstm->Release();
   }

   return hr;
}

HRESULT PinTable::LoadCustomInfo(IStorage* pstg, IStream *pstmTags, HCRYPTHASH hcrypthash, int version)
{
   m_vCustomInfoTag.clear();
   m_vCustomInfoContent.clear();
   BiffReader reader(pstmTags, version, hcrypthash, 0);
   reader.AsObject(
      [this](int tag, IObjectReader& reader)
      {
         if (tag == FID(CUST))
         {
            string tmp = reader.AsString();
            m_vCustomInfoTag.push_back(std::move(tmp));
         }
         return true;
      });
   for (const string& tag : m_vCustomInfoTag)
   {
      string customInfo;
      ReadInfoValue(pstg, MakeWString(tag), customInfo, hcrypthash);
      m_vCustomInfoContent.push_back(std::move(customInfo));
   }
   return S_OK;
}

void PinTable::Save(IObjectWriter& writer, const bool saveForUndo)
{
#ifndef __STANDALONE__
   writer.WriteFloat(FID(LEFT), m_left);
   writer.WriteFloat(FID(TOPX), m_top);
   writer.WriteFloat(FID(RGHT), m_right);
   writer.WriteFloat(FID(BOTM), m_bottom);

   writer.WriteBool(FID(EFSS), m_isFSSViewModeEnabled);
   static constexpr int vsFields[NUM_BG_SETS][19] = { 
      { FID(VSM0), FID(ROTA), FID(INCL), FID(LAYB), FID(FOVX), FID(XLTX), FID(XLTY), FID(XLTZ), FID(SCLX), FID(SCLY), FID(SCLZ), FID(HOF0), FID(VOF0), FID(WTX0), FID(WTY0), FID(WTZ0), FID(WBX0), FID(WBY0), FID(WBZ0) },
      { FID(VSM1), FID(ROTF), FID(INCF), FID(LAYF), FID(FOVF), FID(XLFX), FID(XLFY), FID(XLFZ), FID(SCFX), FID(SCFY), FID(SCFZ), FID(HOF1), FID(VOF1), FID(WTX1), FID(WTY1), FID(WTZ1), FID(WBX1), FID(WBY1), FID(WBZ1) },
      { FID(VSM2), FID(ROFS), FID(INFS), FID(LAFS), FID(FOFS), FID(XLXS), FID(XLYS), FID(XLZS), FID(SCXS), FID(SCYS), FID(SCZS), FID(HOF2), FID(VOF2), FID(WTX2), FID(WTY2), FID(WTZ2), FID(WBX2), FID(WBY2), FID(WBZ2) },
   };
   for (int i = 0; i < 3; i++)
   {
      writer.WriteInt(vsFields[i][0], mViewSetups[i].mMode);
      writer.WriteFloat(vsFields[i][1], mViewSetups[i].mViewportRotation);
      writer.WriteFloat(vsFields[i][2], mViewSetups[i].mLookAt);
      writer.WriteFloat(vsFields[i][3], mViewSetups[i].mLayback);
      writer.WriteFloat(vsFields[i][4], mViewSetups[i].mFOV);
      writer.WriteFloat(vsFields[i][5], mViewSetups[i].mViewX);
      writer.WriteFloat(vsFields[i][6], mViewSetups[i].mViewY);
      writer.WriteFloat(vsFields[i][7], mViewSetups[i].mViewZ);
      writer.WriteFloat(vsFields[i][8], mViewSetups[i].mSceneScaleX);
      writer.WriteFloat(vsFields[i][9], mViewSetups[i].mSceneScaleY);
      writer.WriteFloat(vsFields[i][10], mViewSetups[i].mSceneScaleZ);
      writer.WriteFloat(vsFields[i][11], mViewSetups[i].mViewHOfs);
      writer.WriteFloat(vsFields[i][12], mViewSetups[i].mViewVOfs);
      writer.WriteFloat(vsFields[i][15], mViewSetups[i].mWindowTopZOfs);
      writer.WriteFloat(vsFields[i][18], mViewSetups[i].mWindowBottomZOfs);
   }

   writer.WriteInt(FID(ORRP), m_overridePhysics);
   writer.WriteBool(FID(ORPF), m_overridePhysicsFlipper);
   writer.WriteFloat(FID(GAVT), m_Gravity);
   writer.WriteFloat(FID(FRCT), m_friction);
   writer.WriteFloat(FID(ELAS), m_elasticity);
   writer.WriteFloat(FID(ELFA), m_elasticityFalloff);
   writer.WriteFloat(FID(PFSC), m_scatter);
   writer.WriteFloat(FID(SCAT), m_defaultScatter);
   writer.WriteFloat(FID(NDGT), m_nudgeTime);
   writer.WriteInt(FID(PHML), m_PhysicsMaxLoops);

   //writer.WriteFloat(FID(IMTCOL), m_transcolor);

   writer.WriteBool(FID(REEL), m_renderEMReels);
   writer.WriteBool(FID(DECL), m_renderDecals);

   writer.WriteFloat(FID(OFFX), m_winEditorViewOffset.x);
   writer.WriteFloat(FID(OFFY), m_winEditorViewOffset.y);

   writer.WriteFloat(FID(ZOOM), m_winEditorZoom);

   writer.WriteFloat(FID(SLPX), m_angletiltMax);
   writer.WriteFloat(FID(SLOP), m_angletiltMin);

   writer.WriteString(FID(IMAG), m_image);
   writer.WriteString(FID(BIMG), m_BG_image[0]);
   writer.WriteString(FID(BIMF), m_BG_image[1]);
   writer.WriteString(FID(BIMS), m_BG_image[2]);
   writer.WriteBool(FID(BIMN), m_ImageBackdropNightDay);
   writer.WriteString(FID(IMCG), m_imageColorGrade);
   writer.WriteString(FID(BLIM), m_ballImage);
   writer.WriteBool(FID(BLSM), m_ballSphericalMapping);
   writer.WriteString(FID(BLIF), m_ballImageDecal);
   writer.WriteString(FID(EIMG), m_envImage);
   writer.WriteString(FID(NOTX), m_notesText);

   writer.WriteString(FID(SSHT), m_screenShot);

   writer.WriteBool(FID(FBCK), m_winEditorBackdrop);

   writer.WriteFloat(FID(GLAS), m_glassTopHeight);
   writer.WriteFloat(FID(GLAB), m_glassBottomHeight);

   writer.WriteString(FID(PLMA), m_playfieldMaterial);
   writer.WriteInt(FID(BCLR), m_colorbackdrop);

   writer.WriteFloat(FID(TDFT), m_difficulty);

   writer.WriteInt(FID(LZAM), m_lightAmbient);
   writer.WriteInt(FID(LZDI), m_Light[0].emission);
   writer.WriteFloat(FID(LZHI), m_lightHeight);
   writer.WriteFloat(FID(LZRA), m_lightRange);
   writer.WriteFloat(FID(LIES), m_lightEmissionScale);
   writer.WriteFloat(FID(ENES), m_envEmissionScale);
   writer.WriteFloat(FID(GLES), m_globalEmissionScale);
   writer.WriteFloat(FID(AOSC), m_AOScale);
   writer.WriteFloat(FID(SSSC), m_SSRScale);
   writer.WriteFloat(FID(CLBH), m_groundToLockbarHeight);

   writer.WriteFloat(FID(SVOL), m_TableSoundVolume);
   writer.WriteFloat(FID(MVOL), m_TableMusicVolume);

   writer.WriteInt(FID(PLST), quantizeUnsigned<8>(m_playfieldReflectionStrength));
   writer.WriteBool(FID(BDMO), m_BallDecalMode);
   writer.WriteFloat(FID(BPRS), m_ballPlayfieldReflectionStrength);
   writer.WriteFloat(FID(DBIS), m_defaultBulbIntensityScaleOnBall);
   writer.WriteBool(FID(GDAC), m_winEditorGrid);

   writer.WriteInt(FID(UAOC), m_enableAO);
   writer.WriteInt(FID(USSR), m_enableSSR);
   writer.WriteInt(FID(TMAP), m_toneMapper);
   writer.WriteFloat(FID(EXPO), m_exposure);
   writer.WriteFloat(FID(BLST), m_bloom_strength);

   // Legacy material saving for backward compatibility
   writer.WriteInt(FID(MASI), (int)m_materials.size());
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
         mats[i].fGlossyImageLerp = 255 - quantizeUnsigned<8>(saturate(m->m_fGlossyImageLerp)); // '255 -' to be compatible with previous table versions
         mats[i].fThickness = quantizeUnsigned<8>(clamp(m->m_fThickness, 0.05f, 1.f)); // clamp with 0.05f to be compatible with previous table versions
         mats[i].fEdge = m->m_fEdge;
         mats[i].fOpacity = m->m_fOpacity;
         mats[i].bIsMetal = m->m_type == Material::MaterialType::METAL;
         mats[i].bOpacityActive_fEdgeAlpha = m->m_bOpacityActive ? 1 : 0;
         mats[i].bOpacityActive_fEdgeAlpha |= quantizeUnsigned<7>(saturate(m->m_fEdgeAlpha)) << 1;
         strncpy_s(mats[i].szName, std::size(mats[i].szName), m->m_name.c_str());
         for (size_t c = strnlen_s(mats[i].szName, std::size(mats[i].szName)); c < std::size(mats[i].szName); ++c) // to avoid garbage after 0
             mats[i].szName[c] = '\0';
      }
      writer.WriteRaw(FID(MATE), mats.data(), (int)(sizeof(SaveMaterial)*m_materials.size()));

      vector<SavePhysicsMaterial> phymats(m_materials.size());
      for (size_t i = 0; i < m_materials.size(); i++)
      {
          const Material* const m = m_materials[i];
          strncpy_s(phymats[i].szName, std::size(phymats[i].szName), m->m_name.c_str());
          for (size_t c = strnlen_s(phymats[i].szName, std::size(phymats[i].szName)); c < std::size(phymats[i].szName); ++c) // to avoid garbage after 0
              phymats[i].szName[c] = '\0';
          phymats[i].fElasticity = m->m_fElasticity;
          phymats[i].fElasticityFallOff = m->m_fElasticityFalloff;
          phymats[i].fFriction = m->m_fFriction;
          phymats[i].fScatterAngle = m->m_fScatterAngle;
      }
      writer.WriteRaw(FID(PHMA), phymats.data(), (int)(sizeof(SavePhysicsMaterial)*m_materials.size()));
   }
   // 10.8+ material saving (this format supports new properties, can be extended in future versions, and does not perform quantization)
   for (size_t i = 0; i < m_materials.size(); i++)
      m_materials[i]->Save(writer, saveForUndo);

   for (size_t i = 0; i < m_vrenderprobe.size(); i++)
      m_vrenderprobe[i]->Save(writer, saveForUndo);

   // Don't save special values when copying for undo. For instance, don't reset the code.
   if (!saveForUndo)
   {
      writer.WriteInt(FID(SEDT), (int)m_vedit.size());
      writer.WriteInt(FID(SSND), (int)m_vsound.size());
      writer.WriteInt(FID(SIMG), (int)m_vimage.size());
      writer.WriteInt(FID(SFNT), (int)m_vfont.size());
      writer.WriteInt(FID(SCOL), m_vcollection.size());

      writer.WriteWideString(FID(NAME), m_wzName);

      writer.WriteRaw(FID(CCUS), m_rgcolorcustom, sizeof(COLORREF) * 16);

      string script = m_script_text;
      if (!m_external_script_name.empty())
      {
         std::ofstream file(m_external_script_name);
         if (file)
         {
            file.write(script.data(), script.size());
            file.close();
         }
         script = m_original_table_script;
      }
      writer.WriteScript(FID(CODE), script);
   }

   writer.WriteInt(FID(TLCK), m_tablelocked);
   writer.EndObject();
#endif
}

HRESULT PinTable::LoadGameFromFilename(const std::filesystem::path &filename)
{
#ifndef __STANDALONE__
   if (m_vpinball)
   {
      VPXLoadFileProgressBar feedback(g_app->GetInstanceHandle(), m_vpinball->m_hwndStatusBar);
      return LoadGameFromFilename(filename, feedback);
   }
#endif

   VPXFileFeedback feedback;
   return LoadGameFromFilename(filename, feedback);
}

HRESULT PinTable::LoadGameFromFilename(const std::filesystem::path &filename, VPXFileFeedback &feedback)
{
   if (filename.empty())
   {
      ShowError("Empty File Name String!");
      return S_FALSE;
   }

   PLOGI << "LoadGameFromFilename " + filename.string(); // For profiling

   m_filename = filename;

   // Load user custom settings before actually loading the table for settings applying during load
   if (const std::filesystem::path iniPath = GetSettingsFileName(); !iniPath.empty())
   {
      m_settings.SetIniPath(iniPath);
      if (FileExists(iniPath))
         m_settings.Load(false);
   }

   HRESULT hr;
   IStorage* pstgRoot;
   if (FAILED(hr = StgOpenStorage(m_filename.wstring().c_str(), nullptr, STGM_TRANSACTED | STGM_READ, nullptr, 0, &pstgRoot)))
   {
      const string msg = std::format("Error {:#010X} loading \"{}\"", static_cast<unsigned int>(hr), m_filename.string());
      ShowError(msg);
      return hr;
   }

   feedback.OperationStarted();

   //

   HCRYPTPROV hcp = NULL; // crypt context
   HCRYPTHASH hch = NULL; // hash for file integrity check
   HCRYPTHASH hchkey = NULL; // hash for decryption key derivation

   #ifndef __STANDALONE__
   // Hashing (to ensure file integrity), can be disabled for slightly faster loading (and then also matches standalone which cannot feature this)
   const bool hashValidation = !g_app->m_settings.GetEditor_DisableHash();
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
               const string errorMsg = std::format("This table was saved with file version {}.{:02d} and is newer than the supported file version {}.{:02d}!\nYou might get problems loading/playing it, so please update to the latest VPX at https://github.com/vpinball/vpinball/releases!", loadfileversion / 100, loadfileversion % 100, CURRENT_FILE_FORMAT_VERSION / 100, CURRENT_FILE_FORMAT_VERSION % 100);
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

         BiffReader tableReader(pstmGame, loadfileversion, hch, (loadfileversion < NO_ENCRYPTION_FORMAT_VERSION) ? hkey : NULL);
         Load(tableReader);
         if (!tableReader.HasError())
         {
            const int csubobj = m_loadTemp[0];
            const int csounds = m_loadTemp[1];
            const int ctextures = m_loadTemp[2];
            const int cfonts = m_loadTemp[3];
            const int ccollection = m_loadTemp[4];
            
            PLOGI << "PinTable Data loaded"; // For profiling

            feedback.AboutToProcessTable(csubobj + csounds + ctextures + cfonts);

            ThreadPool pool(g_app->GetLogicalNumberOfProcessors());
            vector<IEditable *> parts;
            parts.resize(csubobj);
            int nLoadedParts = 0;
            for (int i = 0; i < csubobj; i++)
            {
               pool.enqueue(
                  [i, &feedback, &parts, loadfileversion, pstgData, hch, hkey, this, &nLoadedParts, csubobj]
                  {
                     const wstring wStmName = L"GameItem" + std::to_wstring(i);

                     IStream *pstmItem;
                     HRESULT hr;
                     if (FAILED(hr = pstgData->OpenStream(wStmName.c_str(), nullptr, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmItem)))
                        return hr;

                     ULONG read;
                     ItemTypeEnum type;
                     pstmItem->Read(&type, sizeof(int), &read);

                     IEditable *const piedit = EditableRegistry::Create(type);
                     if (piedit == nullptr)
                        return E_FAIL;

                     piedit->m_onLoadExpectedPartGroup.clear();
                     BiffReader reader(pstmItem, loadfileversion, (loadfileversion < 1000) ? hch : NULL, (loadfileversion < 1000) ? hkey : NULL); // 1000 (VP10 beta) removed the encryption //!! NO_ENCRYPTION_FORMAT_VERSION?
                     piedit->Load(reader); 
                     pstmItem->Release();
                     pstmItem = nullptr;
                     if (reader.HasError())
                        return E_FAIL;

                     parts[i] = piedit;
                     nLoadedParts++;
                     return S_OK;
                  });
            }

            assert(m_vsound.empty());
            m_vsound.resize(csounds);
            int nLoadedSounds = 0;
            for (int i = 0; i < csounds; i++)
            {
               pool.enqueue(
                  [i, &feedback, loadfileversion, pstgData, this, &nLoadedSounds, csounds]
                  {
                     const wstring wStmName = L"Sound" + std::to_wstring(i);

                     IStream *pstmItem;
                     HRESULT hr;
                     if (FAILED(hr = pstgData->OpenStream(wStmName.c_str(), nullptr, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmItem)))
                        return hr;

                     VPX::Sound *pps = VPX::Sound::CreateFromStream(pstmItem, loadfileversion);
                     pstmItem->Release();
                     pstmItem = nullptr;
                     m_vsound[i] = pps;
                     nLoadedSounds++;
                     return hr;
                  });
            }

            assert(m_vimage.empty());
            m_vimage.resize(ctextures);
            int nLoadedImages = 0;
            for (int i = 0; i < ctextures; i++)
            {
               pool.enqueue(
                  [i, loadfileversion, pstgData, this, &nLoadedImages, ctextures]
                  {
                     const wstring wStmName = L"Image" + std::to_wstring(i);

                     IStream *pstmItem;
                     HRESULT hr;
                     if (FAILED(hr = pstgData->OpenStream(wStmName.c_str(), nullptr, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmItem)))
                        return hr;

                     BiffReader reader(pstmItem, loadfileversion, 0, 0);
                     m_vimage[i] = Texture::CreateFromObjectReader(reader, this);
                     pstmItem->Release();
                     pstmItem = nullptr;
                     nLoadedImages++;
                     return hr;
                  });
            }

            // Wait for dispatched tasks, updating the progress bar on UI thread
            const int totalToLoad = csubobj + csounds + ctextures;
            while (pool.has_work_in_flight())
            {
               SDL_Delay(10);
               feedback.LoadingProgressUpdated(nLoadedParts + nLoadedSounds + nLoadedImages, totalToLoad);
            };

            // Handle failed loading & duplicates
            if (!parts.empty())
            {
               // Process unnamed parts after named parts
               std::ranges::stable_partition(parts.begin(), parts.end(), [](IEditable *p) { return p && !p->GetIScriptable()->m_wzName.empty(); });
               for (size_t i = 0; i < parts.size(); ++i)
               {
                  IEditable * const part = parts[i];
                  if (part == nullptr)
                  {
                     PLOGE << "Failed to load one of the table parts";
                     parts.erase(parts.begin() + i);
                     --i;
                  }
                  else
                  {
                     // Decals used to not have a name, so we may have to provide an autogenerated one (still, some old files do have a name for decals somehow)
                     if (part->GetIScriptable()->m_wzName.empty() && part->GetItemType() == eItemDecal)
                        part->GetIScriptable()->m_wzName = GetUniqueName(L"Decal"s);
                     if (!IsNameUnique(part->GetIScriptable()->m_wzName))
                     {
                        const wstring oldName = part->GetIScriptable()->m_wzName;
                        part->GetIScriptable()->m_wzName = GetUniqueName(part->GetIScriptable()->m_wzName);
                        PLOGE << "Duplicate part name found: " << MakeString(oldName) << " renamed it to " << MakeString(part->GetIScriptable()->m_wzName);
                     }

                     AddPart(part);

                     // We used to have a hack taken from VPVR to display backglass in VR: an external window would be captured, then rendered on a primitive with an 
                     // image named backglassimage. We now have support for external renderer on flasher, so we replace these primitives by flashers.
                     // As this may cause script error if the original table would expect a primitive object and tweak properties not supported by flasher object, 
                     // we keep the original object. This is not perfect as the table script will not tweak this one, but at least, it makes updating table easy.
                     if (part->GetItemType() == eItemPrimitive && StrCompareNoCase(((Primitive *)part)->m_d.m_szImage, "backglassimage"s))
                     {
                        Primitive * const primitive = (Primitive *)part;
                        if (primitive->m_d.m_use3DMesh)
                        {
                           // We need to reduce the primitive to a flasher rectangle. The algorithm is:
                           // - to find the flasher plane using mesh's faces normals, favoring faces looking toward the player (a backfacing backglass is unlikely)
                           // - to find the plane position by considering the vertices nearest to the player (to discard back of the primitive if using a box instead of a rect)
                           // - to evaluate an axis align square in this plane and define a flasher accordingly (a rotated backglass is unlikely)
                           const Matrix3D& transform = primitive->RecalculateMatrices();
                           vector<vec3> vertices(primitive->m_mesh.m_vertices.size());
                           for (size_t i2 = 0; i2 < primitive->m_mesh.m_vertices.size(); i2++)
                              vertices[i2] = transform * primitive->m_mesh.m_vertices[i2];
                           vec3 planeNormal(0.f, 0.f, 0.f);
                           float planeNormalWeight = 0.f;
                           for (size_t i2 = 0; i2 < primitive->m_mesh.m_indices.size(); i2 += 3)
                           {
                              vec3 &a = vertices[primitive->m_mesh.m_indices[i2]];
                              vec3 &b = vertices[primitive->m_mesh.m_indices[i2 + 1]];
                              vec3 &c = vertices[primitive->m_mesh.m_indices[i2 + 2]];
                              vec3 ab(b.x - a.x, b.y - a.y, b.z - a.z);
                              vec3 ac(c.x - a.x, c.y - a.y, c.z - a.z);
                              vec3 n = CrossProduct(ac, ab);
                              n.Normalize();
                              const float weight = -n.z; //= n.Dot(vec3(0.f, 0.f, -1.f));
                              if (weight > 0.f)
                              {
                                 planeNormal += weight * n;
                                 planeNormalWeight += weight;
                              }
                           }

                           planeNormal.x = 0.f; // to simplify, we align the backglass X axis with the table (after all, backglasses should be facing the player)
                           if (const float normalLength = planeNormal.Length(); normalLength > 1e-5f)
                           {
                              planeNormal /= normalLength;

                              float planeDist = FLT_MAX;
                              for (const unsigned int idx : primitive->m_mesh.m_indices)
                                 planeDist = min(planeDist, planeNormal.Dot(vertices[idx]));

                              float minx = FLT_MAX; // min/max along the x axis
                              float miny = FLT_MAX; // min/max along planeYAxis
                              float maxx = FLT_MIN;
                              float maxy = FLT_MIN;
                              const vec3 planeYAxis(0.f,planeNormal.z,-planeNormal.y); //= CrossProduct(planeNormal, vec3(1.f, 0.f, 0.f));
                              for (const unsigned int idx : primitive->m_mesh.m_indices)
                                 if (const float proj = planeNormal.Dot(vertices[idx]); proj < planeDist + 1.f)
                                 {
                                    const float px = vertices[idx].x; // since we aligned the x axis, planeXAxis is (1, 0, 0)
                                    const float py = vertices[idx].Dot(planeYAxis);
                                    minx = min(minx, px);
                                    maxx = max(miny, px);
                                    miny = min(miny, py);
                                    maxy = max(maxy, py);
                                 }
                              const float backglassWidth = maxx - minx;
                              const float backglassHeight = maxy - miny;
                              if (backglassWidth > 0.f && backglassHeight > 0.f)
                              {
                                 Flasher *const backglass = (Flasher *)EditableRegistry::CreateAndInit(ItemTypeEnum::eItemFlasher, this, 0.f, 0.f);
                                 if (backglass)
                                 {
                                    backglass->m_wzName = GetUniqueName(primitive->GetWName());
                                    backglass->m_onLoadExpectedPartGroup = primitive->m_onLoadExpectedPartGroup;
                                    backglass->Scale(backglassWidth / 100.f, backglassHeight / 100.f, Vertex2D {}, true); // We should gather the base flasher size from the object instead of guessing its default value
                                    vec3 center = planeDist * planeNormal;
                                    center += (miny + 0.5f * backglassHeight) * planeYAxis;
                                    center.x += (minx + 0.5f * backglassWidth); // since planeXAxis is (1, 0, 0)
                                    backglass->Translate(Vertex2D(center.x, center.y));
                                    backglass->m_d.m_vCenter = Vertex2D(center.x, center.y);
                                    backglass->m_d.m_height = center.z;
                                    backglass->m_d.m_rotX = -180.f - RADTOANG(atan2(planeNormal.y, planeNormal.z)); // since planeXAxis is (1, 0, 0)
                                    backglass->m_d.m_renderMode = FlasherData::EXT_RENDER;
                                    backglass->m_d.m_renderStyle = VPXWindowId::VPXWINDOW_Backglass;
                                    backglass->m_d.m_depthBias = primitive->m_d.m_depthBias;
                                    backglass->m_d.m_isVisible = primitive->m_d.m_visible;
                                    primitive->m_d.m_visible = false;
                                    PLOGE << "Primitive '" << primitive->GetName() << "' used as a deprecated VR backglass was hidden and an external renderer flasher named '"
                                          << backglass->GetName() << "' was added. This may cause script issues.";
                                    AddPart(backglass);
                                    backglass->Release();
                                 }
                              }
                           }
                        }
                     }

                     part->Release();
                  }
               }
            }
            if (!m_vsound.empty())
               for (size_t i = 0; i < m_vsound.size(); ++i)
               {
                  const VPX::Sound *sound = m_vsound[i];
                  if (sound == nullptr)
                  {
                     PLOGE << "Failed to load one of the table sounds";
                     m_vsound.erase(m_vsound.begin() + i);
                     --i;
                  }
                  else if (i < m_vsound.size() - 1)
                  {
                     for (size_t i2 = i + 1; i2 < m_vsound.size(); ++i2)
                        if (sound->GetName() == m_vsound[i2]->GetName())
                        {
                           PLOGE << "Duplicate sound name found: " << sound->GetName() << ", dropping it!";
                           m_vsound.erase(m_vsound.begin() + i2);
                           --i2;
                        }
                  }
               }
            if (!m_vimage.empty())
               for (size_t i = 0; i < m_vimage.size(); ++i)
               {
                  const Texture * image = m_vimage[i];
                  if (image == nullptr)
                  {
                     PLOGE << "Failed to load one of the table images";
                     m_vimage.erase(m_vimage.begin() + i);
                     --i;
                  }
                  else if (i < m_vimage.size() - 1)
                  {
                     for (size_t i2 = i + 1; i2 < m_vimage.size(); ++i2)
                        if (image->m_name == m_vimage[i2]->m_name)
                        {
                           PLOGE << "Duplicate image name found: " << image->GetName() << ", dropping it!";
                           m_vimage.erase(m_vimage.begin() + i2);
                           --i2;
                        }
                  }
               }

            PLOGI << "Images, Sounds and Items loaded"; // For profiling

            for (int i = 0; i < cfonts; i++)
            {
               const wstring wStmName = L"Font" + std::to_wstring(i);

               IStream* pstmItem;
               if (SUCCEEDED(hr = pstgData->OpenStream(wStmName.c_str(), nullptr, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmItem)))
               {
                  PinFont * const ppf = new PinFont();
                  BiffReader reader(pstmItem, loadfileversion, 0, 0);
                  ppf->Load(reader);
                  m_vfont.push_back(ppf);
                  ppf->Register();
                  pstmItem->Release();
                  pstmItem = nullptr;
               }
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
                  BiffReader reader(pstmItem, loadfileversion, hch, (loadfileversion < NO_ENCRYPTION_FORMAT_VERSION) ? hkey : 0);
                  pcol->Load(reader);
                  AddCollection(pcol);
                  pcol->Release();
                  pstmItem->Release();
                  pstmItem = nullptr;
               }
            }

            // Resolve layer names once all part & collection names are known as they must be unique but this constraint was added in 10.8.1 when adding hierarchical PartGroup
            parts = GetParts();
            vector<string> functions;
            vector<string> identifiers;
            ParseScript(m_script_text, functions, identifiers, [](const string&, int) {});
            const wstring lowerCaseScript = lowerCase(MakeWString(m_script_text));
            for (auto part : parts)
            {
               if (const wstring& requestedLayerName = part->m_onLoadExpectedPartGroup; !requestedLayerName.empty())
               {
                  wstring layerName = requestedLayerName;
                  auto partGroupF = std::ranges::find_if(m_vedit,
                     [&layerName](const IEditable *editable) { return (editable->GetItemType() == ItemTypeEnum::eItemPartGroup) && (editable->GetIScriptable()->m_wzName == layerName); });
                  // If part group was not already added, we need to check if the name is conflicting with other editables, collections or script declarations
                  int renameIndex = 1;
                  bool layerPostpend = false;
                  while (partGroupF == m_vedit.end())
                  {
                     bool nameIsUnique = true
                        && IsNameUnique(layerName)
                        && std::ranges::find(functions, MakeString(lowerCase(layerName))) == functions.end()
                        && std::ranges::find(identifiers, MakeString(lowerCase(layerName))) == identifiers.end();
                     if (nameIsUnique)
                        break;

                     // Postpend "layer" to keep alphabetic order of layer
                     if (!layerPostpend && !layerName.ends_with(L"_Layer"))
                     {
                        layerPostpend = true; 
                        layerName += L"_Layer";
                     }
                     else
                     {
                        size_t lastNonDigit = layerName.length();
                        while (lastNonDigit > 0 && iswdigit(layerName[lastNonDigit - 1]))
                           lastNonDigit--;
                        if (lastNonDigit < layerName.length())
                        {
                           // If it ends by a number, then inc the number
                           std::wstring numberStr = layerName.substr(lastNonDigit);
                           const int number = std::stoi(numberStr);
                           layerName.resize(lastNonDigit); // base
                           renameIndex = max(renameIndex, number + 1);
                        }
                        else
                        {
                           // If not, add it
                           layerName += L"_";
                        }
                        layerName += std::format(L"{:3d}", renameIndex);
                        renameIndex += 1;
                     }

                     partGroupF = std::ranges::find_if(m_vedit,
                        [&layerName](const IEditable *editable) { return (editable->GetItemType() == ItemTypeEnum::eItemPartGroup) && (editable->GetIScriptable()->m_wzName == layerName); });
                  }
                  // Set or create implicit PartGroups (that is to say, PartGroups corresponding to legacy layers)
                  if (partGroupF != m_vedit.end())
                  {
                     part->SetPartGroup(static_cast<PartGroup *>(*partGroupF));
                  }
                  else if (PartGroup *const newGroup = static_cast<PartGroup *>(EditableRegistry::CreateAndInit(eItemPartGroup, this, 0, 0)); newGroup)
                  {
                     if (requestedLayerName != layerName)
                     {
                        PLOGI << "Layer name '" << MakeString(requestedLayerName) << "' was replaced by '" << MakeString(layerName)
                              << "' as this name is already used by another table element";
                     }
                     newGroup->m_wzName = layerName;
                     AddPart(newGroup);
                     part->SetPartGroup(newGroup);
                  }
               }
            }

            // Since 10.8.1, layers have been replaced by groups with properties, keep partgroups at the beginning of the editable list.
            std::ranges::stable_partition(m_vedit.begin(), m_vedit.end(), [](IEditable *p) { return p->GetItemType() == ItemTypeEnum::eItemPartGroup; });

            // Resolve collection parts
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
            for (auto pEdit : m_vedit)
               if (pEdit->GetItemType() == ItemTypeEnum::eItemPrimitive && (((Primitive *)pEdit)->IsPlayfield()))
               {
                  Primitive* const prim = (Primitive *)pEdit;
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

         if (loadfileversion < 1081)
         {
            // Rename layers that have been automatically converted to group if there aren't any name conflict (checking for collection objects, as well as script variable names)
            const string script = lowerCase(m_script_text);
            std::ranges::for_each(m_vedit,
               [&](IEditable *editable)
               {
                  if (editable->GetItemType() != eItemPartGroup)
                     return;
                  const wstring& name = editable->GetWName();
                  if (!name.starts_with(L"Layer_"))
                     return;
                  const wstring shortName = name.substr(6);
                  const wstring shortNameLCase = lowerCase(shortName);
                  const string shortNameLCaseS = MakeString(shortNameLCase);
                  auto v = std::ranges::find_if(m_vedit, [&shortNameLCase](const IEditable *const e) { return lowerCase(e->GetWName()) == shortNameLCase; });
                  if (v != m_vedit.end())
                     return; // Conflict with another part name
                  if ((shortName.find_first_not_of(L"0123456789") != std::string::npos) && script.find(shortNameLCaseS) != std::string::npos) //!!
                     return; // (Potential) conflict with a script variable
                  for (int i = 0; i < m_vcollection.size(); i++)
                  {
                     if (lowerCase(m_vcollection.ElementAt(i)->m_wzName) == shortNameLCase)
                        return; // Conflict with a collection name
                  }
                  RenamePart(editable, shortName);
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
                  RemovePart(textbox);
                  Flasher* const dmd = (Flasher *)EditableRegistry::CreateAndInit(ItemTypeEnum::eItemFlasher, this, 0, 0);
                  RemovePart(dmd);
                  dmd->m_wzName = textbox->m_wzName;
                  dmd->UpdatePoint(0, textbox->m_d.m_v1.x, textbox->m_d.m_v1.y);
                  dmd->UpdatePoint(1, textbox->m_d.m_v1.x, textbox->m_d.m_v2.y);
                  dmd->UpdatePoint(2, textbox->m_d.m_v2.x, textbox->m_d.m_v2.y);
                  dmd->UpdatePoint(3, textbox->m_d.m_v2.x, textbox->m_d.m_v1.y);
                  dmd->m_desktopBackdrop = true;
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
                  AddPart(dmd);
                  PLOGI << "Textbox used as DMD replaced by a flasher (name=" << dmd->m_wzName << ')';
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

   SetDirty(eSaveClean);

   m_title = TitleFromFilename(filename);
#ifndef __STANDALONE__
   const DWORD attr = GetFileAttributes(filename.string().c_str());
   if ((attr != INVALID_FILE_ATTRIBUTES) && (attr & FILE_ATTRIBUTE_READONLY))
      m_title += " [READ ONLY]"sv;
#endif

   PLOGI << "InitTablePostLoad"; // For profiling

   m_scriptableNames[lowerCase(m_wzName)] = this;

   for (unsigned int i = 1; i < NUM_BG_SETS; ++i)
      if (mViewSetups[i].mFOV == FLT_MAX) // old table, copy FS and/or FSS settings over from old DT setting
      {
         mViewSetups[i] = mViewSetups[BG_DESKTOP];
         if (m_BG_image[i].empty() && i == BG_FSS) // copy image over for FSS mode
            m_BG_image[i] = m_BG_image[BG_DESKTOP];
      }

   Settings::SetTableOverride_Difficulty_Default(m_difficulty);
   m_globalDifficulty = m_settings.GetTableOverride_Difficulty();

   RemoveInvalidReferences();

   std::filesystem::path tablePath = std::filesystem::path(filename).parent_path();
   std::filesystem::path tableFile = std::filesystem::path(filename).filename();
   
   // Auto-import POV settings, if it exists. This is kept for backward compatibility as POV settings
   // are now normal settings stored with others in app/table ini file. It will be only imported if there is no table ini file
   if (const std::filesystem::path filenameAuto = tablePath / tableFile.replace_extension(".pov"); !FileExists(GetSettingsFileName()) && FileExists(filenameAuto))
      ImportBackdropPOV(filenameAuto);
   else if (const std::filesystem::path filenameAuto2 = tablePath / "autopov.pov"sv; FileExists(filenameAuto2))
      ImportBackdropPOV(filenameAuto2);

   // auto-import VBS table script, if it exists...
   if (std::filesystem::path filenameAuto = g_app->m_fileLocator.SearchScript(this, tableFile.replace_extension(".vbs")); !filenameAuto.empty())
      LoadScriptOverride(filenameAuto);
   else
   {
      auto fn = tablePath.filename();
      fn += ".vbs"sv;
      std::filesystem::path folderVbs = tablePath / fn;
      folderVbs = find_case_insensitive_file_path(folderVbs);
      if (!folderVbs.empty())
         LoadScriptOverride(folderVbs);
   }
   m_sdsDirtyScript = eSaveClean;

   // auto-import VPP settings, if it exists...
   if (const std::filesystem::path filenameAuto = tablePath / tableFile.replace_extension(".vpp"); FileExists(filenameAuto)) // We check if there is a matching table vpp settings file first
      ImportVPP(filenameAuto);
   else if (const std::filesystem::path filenameAuto2 = tablePath / "autovpp.vpp"sv; FileExists(filenameAuto2)) // Otherwise, we seek for autovpp settings
      ImportVPP(filenameAuto2);

   if (m_tableEditor)
   {
      m_tableEditor->m_pcv->SetScript(m_script_text);
      m_tableEditor->m_pcv->AddItem(this, false);
      m_tableEditor->m_pcv->AddItem(m_psgt, true);
      //m_tableEditor->m_pcv->AddItem(m_pcv->m_pdm, false);
   }

   return hr;
}

void PinTable::LoadScriptOverride(const std::filesystem::path& scriptPath)
{
   std::ifstream file(scriptPath, std::ios::binary | std::ios::ate);
   if (!file) {
      PLOGE << "Failed to open script file";
      return;
   }
   
   std::streamsize size = file.tellg();
   file.seekg(0, std::ios::beg);
   std::vector<char> buffer((size_t)size);
   if (!file.read(buffer.data(), size)) {
      PLOGE << "Failed to read script file";
      return;
   }

   m_script_text = string_from_utf8_or_iso8859_1(buffer.data(), buffer.size());
   if (m_tableEditor)
      m_tableEditor->m_pcv->SetScript(m_script_text);

   m_external_script_name = scriptPath;
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

void PinTable::Load(IObjectReader& reader)
{
   SetLoadDefaults();
   memset(m_loadTemp, 0, sizeof(m_loadTemp));
   const std::filesystem::path INIFilename = GetSettingsFileName();
   const bool hasIni = !INIFilename.empty() && FileExists(INIFilename);
   reader.AsObject(
      [this, hasIni](int tag, IObjectReader &reader)
      {
         switch (tag)
         {
         case FID(PIID): reader.AsInt(); break;
         case FID(LEFT): m_left = reader.AsFloat(); break;
         case FID(TOPX): m_top = reader.AsFloat(); break;
         case FID(RGHT): m_right = reader.AsFloat(); break;
         case FID(BOTM): m_bottom = reader.AsFloat(); break;
         case FID(VSM0): mViewSetups[BG_DESKTOP].mMode = static_cast<ViewLayoutMode>(reader.AsInt()); break;
         case FID(ROTA): mViewSetups[BG_DESKTOP].mViewportRotation = reader.AsFloat(); break;
         case FID(LAYB): mViewSetups[BG_DESKTOP].mLayback = reader.AsFloat(); break;
         case FID(INCL): mViewSetups[BG_DESKTOP].mLookAt = reader.AsFloat(); break;
         case FID(FOVX): mViewSetups[BG_DESKTOP].mFOV = reader.AsFloat(); break;
         case FID(SCLX): mViewSetups[BG_DESKTOP].mSceneScaleX = reader.AsFloat(); break;
         case FID(SCLY): mViewSetups[BG_DESKTOP].mSceneScaleY = reader.AsFloat(); break;
         case FID(SCLZ): mViewSetups[BG_DESKTOP].mSceneScaleZ = reader.AsFloat(); break;
         case FID(XLTX): mViewSetups[BG_DESKTOP].mViewX = reader.AsFloat(); break;
         case FID(XLTY): mViewSetups[BG_DESKTOP].mViewY = reader.AsFloat(); break;
         case FID(XLTZ): mViewSetups[BG_DESKTOP].mViewZ = reader.AsFloat(); break;
         case FID(HOF0): mViewSetups[BG_DESKTOP].mViewHOfs = reader.AsFloat(); break;
         case FID(VOF0): mViewSetups[BG_DESKTOP].mViewVOfs = reader.AsFloat(); break;
         case FID(WTZ0): mViewSetups[BG_DESKTOP].mWindowTopZOfs = reader.AsFloat(); break;
         case FID(WBZ0): mViewSetups[BG_DESKTOP].mWindowBottomZOfs = reader.AsFloat(); break;
         case FID(VSM1): mViewSetups[BG_FULLSCREEN].mMode = static_cast<ViewLayoutMode>(reader.AsInt()); break;
         case FID(ROTF): mViewSetups[BG_FULLSCREEN].mViewportRotation = reader.AsFloat(); break;
         case FID(LAYF): mViewSetups[BG_FULLSCREEN].mLayback = reader.AsFloat(); break;
         case FID(INCF): mViewSetups[BG_FULLSCREEN].mLookAt = reader.AsFloat(); break;
         case FID(FOVF): mViewSetups[BG_FULLSCREEN].mFOV = reader.AsFloat(); break;
         case FID(SCFX): mViewSetups[BG_FULLSCREEN].mSceneScaleX = reader.AsFloat(); break;
         case FID(SCFY): mViewSetups[BG_FULLSCREEN].mSceneScaleY = reader.AsFloat(); break;
         case FID(SCFZ): mViewSetups[BG_FULLSCREEN].mSceneScaleZ = reader.AsFloat(); break;
         case FID(XLFX): mViewSetups[BG_FULLSCREEN].mViewX = reader.AsFloat(); break;
         case FID(XLFY): mViewSetups[BG_FULLSCREEN].mViewY = reader.AsFloat(); break;
         case FID(XLFZ): mViewSetups[BG_FULLSCREEN].mViewZ = reader.AsFloat(); break;
         case FID(HOF1): mViewSetups[BG_FULLSCREEN].mViewHOfs = reader.AsFloat(); break;
         case FID(VOF1): mViewSetups[BG_FULLSCREEN].mViewVOfs = reader.AsFloat(); break;
         case FID(WTZ1): mViewSetups[BG_FULLSCREEN].mWindowTopZOfs = reader.AsFloat(); break;
         case FID(WBZ1): mViewSetups[BG_FULLSCREEN].mWindowBottomZOfs = reader.AsFloat(); break;
         case FID(VSM2): mViewSetups[BG_FSS].mMode = static_cast<ViewLayoutMode>(reader.AsInt()); break;
         case FID(ROFS): mViewSetups[BG_FSS].mViewportRotation = reader.AsFloat(); break;
         case FID(LAFS): mViewSetups[BG_FSS].mLayback = reader.AsFloat(); break;
         case FID(INFS): mViewSetups[BG_FSS].mLookAt = reader.AsFloat(); break;
         case FID(FOFS): mViewSetups[BG_FSS].mFOV = reader.AsFloat(); break;
         case FID(SCXS): mViewSetups[BG_FSS].mSceneScaleX = reader.AsFloat(); break;
         case FID(SCYS): mViewSetups[BG_FSS].mSceneScaleY = reader.AsFloat(); break;
         case FID(SCZS): mViewSetups[BG_FSS].mSceneScaleZ = reader.AsFloat(); break;
         case FID(XLXS): mViewSetups[BG_FSS].mViewX = reader.AsFloat(); break;
         case FID(XLYS): mViewSetups[BG_FSS].mViewY = reader.AsFloat(); break;
         case FID(XLZS): mViewSetups[BG_FSS].mViewZ = reader.AsFloat(); break;
         case FID(HOF2): mViewSetups[BG_FSS].mViewHOfs = reader.AsFloat(); break;
         case FID(VOF2): mViewSetups[BG_FSS].mViewVOfs = reader.AsFloat(); break;
         case FID(WTZ2): mViewSetups[BG_FSS].mWindowTopZOfs = reader.AsFloat(); break;
         case FID(WBZ2): mViewSetups[BG_FSS].mWindowBottomZOfs = reader.AsFloat(); break;
         case FID(EFSS):
         {
            m_isFSSViewModeEnabled = reader.AsBool();
            UpdateCurrentBGSet();
            break;
         }
         //case FID(VERS): szVersion = reader.AsString(); break;
         case FID(ORRP): m_overridePhysics = reader.AsInt(); break;
         case FID(ORPF): m_overridePhysicsFlipper = reader.AsBool(); break;
         case FID(GAVT): m_Gravity = reader.AsFloat(); break;
         case FID(FRCT): m_friction = reader.AsFloat(); break;
         case FID(ELAS): m_elasticity = reader.AsFloat(); break;
         case FID(ELFA): m_elasticityFalloff = reader.AsFloat(); break;
         case FID(PFSC): m_scatter = reader.AsFloat(); break;
         case FID(SCAT): m_defaultScatter = reader.AsFloat(); break;
         case FID(NDGT): m_nudgeTime = reader.AsFloat(); break;
         case FID(PHML):
         {
            m_PhysicsMaxLoops = reader.AsInt();
            if (m_PhysicsMaxLoops == 0xFFFFFFFF)
               m_PhysicsMaxLoops = m_settings.GetPlayer_PhysicsMaxLoops();
            break;
         }
         case FID(DECL): m_renderDecals = reader.AsBool(); break;
         case FID(REEL): m_renderEMReels = reader.AsBool(); break;
         case FID(OFFX): m_winEditorViewOffset.x = reader.AsFloat(); break;
         case FID(OFFY): m_winEditorViewOffset.y = reader.AsFloat(); break;
         case FID(ZOOM): m_winEditorZoom = reader.AsFloat(); break;
         case FID(SLPX): m_angletiltMax = reader.AsFloat(); break;
         case FID(SLOP): m_angletiltMin = reader.AsFloat(); break;
         case FID(GLAS): m_glassTopHeight = reader.AsFloat(); break;
         case FID(GLAB): m_glassBottomHeight = reader.AsFloat(); break;
         case FID(IMAG): m_image = reader.AsString(); break;
         case FID(BLIM): m_ballImage = reader.AsString(); break;
         case FID(BLSM): m_ballSphericalMapping = reader.AsBool(); break;
         case FID(BLIF): m_ballImageDecal = reader.AsString(); break;
         case FID(SSHT): m_screenShot = reader.AsString(); break;
         case FID(FBCK): m_winEditorBackdrop = reader.AsBool(); break;
         case FID(SEDT): m_loadTemp[0] = reader.AsInt(); break;
         case FID(SSND): m_loadTemp[1] = reader.AsInt(); break;
         case FID(SIMG): m_loadTemp[2] = reader.AsInt(); break;
         case FID(SFNT): m_loadTemp[3] = reader.AsInt(); break;
         case FID(SCOL): m_loadTemp[4] = reader.AsInt(); break;
         case FID(NAME): m_wzName = reader.AsWideString(); break;
         case FID(BIMG): m_BG_image[0] = reader.AsString(); break;
         case FID(BIMF): m_BG_image[1] = reader.AsString(); break;
         case FID(BIMS): m_BG_image[2] = reader.AsString(); break;
         case FID(BIMN): m_ImageBackdropNightDay = reader.AsBool(); break;
         case FID(IMCG): m_imageColorGrade = reader.AsString(); break;
         case FID(EIMG): m_envImage = reader.AsString(); break;
         case FID(PLMA): m_playfieldMaterial = reader.AsString(); break;
         case FID(NOTX): m_notesText = reader.AsString(); break;
         case FID(LZAM): m_lightAmbient = reader.AsInt(); break;
         case FID(LZDI): m_Light[0].emission = reader.AsInt(); break;
         case FID(LZHI): m_lightHeight = reader.AsFloat(); break;
         case FID(LZRA): m_lightRange = reader.AsFloat(); break;
         case FID(LIES): m_lightEmissionScale = reader.AsFloat(); break;
         case FID(ENES): m_envEmissionScale = reader.AsFloat(); break;
         case FID(GLES): m_globalEmissionScale = reader.AsFloat(); break;
         case FID(AOSC): m_AOScale = reader.AsFloat(); break;
         case FID(SSSC): m_SSRScale = reader.AsFloat(); break;
         case FID(CLBH): m_groundToLockbarHeight = reader.AsFloat(); break;
         case FID(PLST):
            m_playfieldReflectionStrength = dequantizeUnsigned<8>(reader.AsInt());
            break;
         case FID(BTRA):
            // Before 10.8, user tweaks were stored in the table file (now moved to a user ini file), we import the legacy settings if there is no user ini file
            if (const int useTrailForBalls = reader.AsInt(); useTrailForBalls != -1 && !hasIni)
               m_settings.SetPlayer_BallTrail(useTrailForBalls == 1, true);
            break;
         case FID(BTST):
            // Before 10.8, user tweaks were stored in the table file (now moved to a user ini file), we import the legacy settings if there is no user ini file
            if (const int ballTrailStrength = reader.AsInt(); !hasIni) 
               m_settings.SetPlayer_BallTrailStrength(dequantizeUnsigned<8>(ballTrailStrength), true);
            break;
         case FID(BPRS): m_ballPlayfieldReflectionStrength = reader.AsFloat(); break;
         case FID(DBIS): m_defaultBulbIntensityScaleOnBall = reader.AsFloat(); break;
         case FID(UAAL):
            // Before 10.8, user tweaks were stored in the table file (now moved to a user ini file), we import the legacy settings if there is no user ini file
            if (const int useAA = reader.AsInt(); useAA != -1 && !hasIni)
               m_settings.SetPlayer_AAFactor(useAA == 0 ? 1.f : 2.f, true);
            break;
         case FID(UAOC):
            // Before 10.8, this setting could be set to -1, meaning override table definition using video options instead
            m_enableAO = reader.AsInt() != 0;
         break;
         case FID(USSR):
            // Before 10.8, this setting could be set to -1, meaning override table definition using video options instead
            m_enableSSR = reader.AsInt() != 0;
         break;
         case FID(TMAP): m_toneMapper = static_cast<ToneMapper>(reader.AsInt()); break;
         case FID(EXPO): m_exposure = reader.AsFloat(); break;
         case FID(UFXA):
            // Before 10.8, user tweaks were stored in the table file (now moved to a user ini file), we import the legacy settings if there is no user ini file
            if (const int fxaa = reader.AsInt(); fxaa != -1 && !hasIni)
               m_settings.SetPlayer_FXAA(fxaa, false);
            break;
         case FID(BLST): m_bloom_strength = reader.AsFloat(); break;
         case FID(BCLR): m_colorbackdrop = reader.AsInt(); break;
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
            reader.AsRaw(&protectionData, sizeof(ProtectionData));
            m_script_protected = ((protectionData.flags & DISABLE_EVERYTHING) == DISABLE_EVERYTHING) || ((protectionData.flags & DISABLE_SCRIPT_EDITING) == DISABLE_SCRIPT_EDITING);
            break;
         }
         case FID(CODE):
            m_original_table_script = reader.AsScript(m_script_protected); // save original script, in case an external vbs is loaded
            m_script_text = string_from_utf8_or_iso8859_1(m_original_table_script.c_str(), m_original_table_script.length());
            break;
         case FID(CCUS): reader.AsRaw(m_rgcolorcustom, sizeof(COLORREF) * 16); break;
         case FID(TDFT): m_difficulty = reader.AsFloat(); break;
         case FID(SVOL): m_TableSoundVolume = reader.AsFloat(); break;
         case FID(BDMO): m_BallDecalMode = reader.AsBool(); break;
         case FID(MVOL): m_TableMusicVolume = reader.AsFloat(); break;
         case FID(AVSY):
            // Before 10.8, user tweaks were stored in the table file (now moved to a user ini file), we import the legacy settings if there is no user ini file
            if (const int tableAdaptiveVSync = reader.AsInt(); tableAdaptiveVSync != -1 && !hasIni)
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
            break;
         case FID(OGAC):
            // Before 10.8, user tweaks were stored in the table file (now moved to a user ini file), we import the legacy settings if there is no user ini file
            if (const bool overwriteGlobalDetailLevel = reader.AsBool(); !overwriteGlobalDetailLevel && !hasIni)
               m_settings.ResetPlayer_AlphaRampAccuracy();
            break;
         case FID(OGDN):
            // Before 10.8, user tweaks were stored in the table file (now moved to a user ini file), we import the legacy settings if there is no user ini file
            // Global Day/Night was fairly convoluted:
            // - table would define the value
            // - user could select in video options to override this value by an automatic value
            // - table could then define to reject this user settings
            // - user could define in commandline to finally override the value
            // Now the logic is the same as all other settings:
            // - table defines the default value, then users define if they want to override this value (through app/table settings or commandline)
            if (const bool overwriteGlobalDayNight = reader.AsBool(); overwriteGlobalDayNight && !hasIni)
               m_settings.SetPlayer_OverrideTableEmissionScale(false, true);
            break;
         case FID(GDAC): m_winEditorGrid = reader.AsBool(); break;
         case FID(ARAC):
            // Before 10.8, user tweaks were stored in the table file (now moved to a user ini file), we import the legacy settings if there is no user ini file
            // The detail level was always saved **before** the override flag so we always load to settings, eventually deleting afterward
            if (const int userDetailLevel = reader.AsInt(); !hasIni) 
               m_settings.SetPlayer_AlphaRampAccuracy(userDetailLevel, true);
            break;
         case FID(MASI): m_numMaterials = reader.AsInt(); break;
         case FID(MATE):
         {
            vector<SaveMaterial> mats(m_numMaterials);
            reader.AsRaw(mats.data(), (int)sizeof(SaveMaterial) * m_numMaterials);
            // Also loads materials for 10.8+ tables if these were saved before the new material format was added.
            // This is hacky and should be removed when 10.9 is out (added to avoid loosing tables edited while 10.8 was in alpha)
            if (reader.GetVersion() < 1080 || m_materials.empty())
            {
               m_materials.reserve(m_numMaterials + m_materials.size());
               for (int i = 0; i < m_numMaterials; i++)
               {
                  Material *const pmat = new Material();
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
            reader.AsRaw(mats.data(), (int)sizeof(SavePhysicsMaterial) * m_numMaterials);
            // Also loads materials for 10.8+ tables if these were saved before the new material format was added.
            // This is hacky and should be removed when 10.9 is out (added to avoid loosing tables edited while 10.8 was in alpha)
            if (reader.GetVersion() < 1080 || m_materials.size() == m_numMaterials)
            {
               for (int i = 0; i < m_numMaterials; i++)
               {
                  bool found = true;
                  Material *pmat = GetMaterial(mats[i].szName);
                  if (pmat == m_dummyMaterial.get())
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
            // Replace legacy materials with the new ones.
            // This is hacky and should be removed when 10.9 is out (added to avoid loosing tables edited while 10.8 was in alpha)
            if (reader.GetVersion() >= 1080 && m_materials.size() == m_numMaterials)
            {
               for (int i = 0; i < m_numMaterials; i++)
                  delete m_materials[i];
               m_materials.clear();
            }

            Material *mat = new Material();
            mat->Load(reader);
            if (reader.HasError())
            {
               assert(!"Invalid binary image file");
               delete mat;
               return false;
            }
            m_materials.push_back(mat);
            break;
         }
         case FID(RPRB):
         {
            RenderProbe *rpb = new RenderProbe();
            rpb->Load(reader);
            if (reader.HasError())
            {
               assert(!"Invalid binary image file");
               delete rpb;
               return false;
            }
            m_vrenderprobe.push_back(rpb);
            break;
         }
         case FID(TLCK): m_tablelocked = reader.AsInt(); break;

         // Deprecated fields (kept for reference and to avoid reusing the same FID in future evolutions)
         case FID(REOP): reader.AsBool(); break; // Reflection on playfield (RenderProbes since 10.8)
         case FID(BREF): reader.AsInt(); break; // Enable ball reflection
         case FID(OGST): reader.AsBool(); break; // Overwrite global stereo
         case FID(MAXS): reader.AsFloat(); break; // Fake stereo max eye separation
         case FID(ZPD): reader.AsFloat(); break; // Fake stereo convergence distance
         case FID(STO): reader.AsFloat(); break; // Fake stereo 3D offset
         case FID(MPGC): reader.AsFloat(); break; // Plunger Normalize
         case FID(MPDF): reader.AsFloat(); break; // Plunger Filter
         case FID(TBLH): reader.AsFloat(); break; // Table Height
         }
         return true;
      });
}

bool PinTable::ExportSound(VPX::Sound *const pps, const std::filesystem::path &filename)
{
   if (StrCompareNoCase(pps->GetImportPath().extension().string(), filename.extension().string()))
   {
      if (pps->SaveToFile(filename))
         return true;
#ifndef __STANDALONE__
      ShowError("Can not Open/Create Sound file!");
   }
   else
      ShowError("File extension does not match, will not convert sound to other format!");
#else
   }
#endif

   return false;
}

void PinTable::ReImportSound(VPX::Sound *const pps, const std::filesystem::path &filename)
{
#ifndef __STANDALONE__
   vector<uint8_t> data = read_file(filename);
   if (!data.empty())
      pps->SetFromFileData(filename, data);
#endif
}


VPX::Sound *PinTable::ImportSound(const std::filesystem::path &filename)
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

   ListView_SetItemText_Safe(hwndListView, index, 1, ppb->m_path.string().c_str());

   return index;
#else
   return 0;
#endif
}

void PinTable::NewCollection(const HWND hwndListView, const bool fromSelection)
{
   CComObject<Collection> *pcol;
   CComObject<Collection>::CreateInstance(&pcol);
   pcol->AddRef();

   pcol->m_wzName = GetUniqueName(LocalStringW(IDS_COLLECTION).m_buffer);

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
               piedit->m_vCollection.push_back(pcol);
               piedit->m_viCollection.push_back(pcol->m_visel.size());
               pcol->m_visel.push_back(m_vmultisel.ElementAt(i));
            }
         }
      }
   }

   const int index = AddListCollection(hwndListView, pcol);

#ifndef __STANDALONE__
   ListView_SetItemState(hwndListView, index, LVIS_SELECTED, LVIS_SELECTED);
#endif

   AddCollection(pcol);
   pcol->Release();
}

int PinTable::AddListCollection(HWND hwndListView, CComObject<Collection> *pcol)
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
   ListView_SetItemText_Safe(hwndListView, index, 1, std::to_string(pcol->m_visel.size()).c_str());
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

void PinTable::AssignSelectionToPartGroup(PartGroup* group)
{
   STARTUNDO
   for (int t = 0; t < m_vmultisel.size(); t++)
   {
      ISelect *const psel = m_vmultisel.ElementAt(t);
      IEditable *const pedit = psel->GetIEditable();
      pedit->SetPartGroup(group);
      if (psel->IsUIVisible() && !group->m_uiVisible)
         psel->SetUIVisible(false);
      else if (!psel->IsUIVisible() && group->m_uiVisible)
         psel->SetUIVisible(true);
   }
   STOPUNDO
#ifndef __STANDALONE__
   g_pvp->GetLayersListDialog()->Update();
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
   const wstring wname = MakeWString(name);
   for (const auto pedit : m_vedit)
      if (wname == pedit->GetIScriptable()->m_wzName)
         return pedit;
   return nullptr;
}

bool PinTable::FMutilSelLocked()
{
   for (int i = 0; i < m_vmultisel.size(); i++)
      if (m_vmultisel[i].IsUILocked())
         return true;

   return false;
}

#ifndef __STANDALONE__
void PinTable::DoCommand(int icmd, int x, int y)
{
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
       case ID_WALLMENU_ROTATE: VPX::WinUI::RotatePointsDialog(this); break;
       case ID_WALLMENU_SCALE: VPX::WinUI::ScalePointsDialog(this); break;
       case ID_WALLMENU_TRANSLATE: VPX::WinUI::TranslatePointsDialog(this); break;
   }
}
#endif

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

const wstring& PinTable::GetCollectionNameByElement(const ISelect * const element) const
{
    for (int i = 0; i < m_vcollection.size(); i++)
        for (int t = 0; t < m_vcollection[i].m_visel.size(); t++)
            if (element == m_vcollection[i].m_visel.ElementAt(t))
                return m_vcollection[i].m_wzName;
    static wstring emptyString;
    return emptyString;
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
         return Vertex2D{FLT_MAX, FLT_MAX};
      if (fabs(v2.y - v1.y) < 0.01f)
         return Vertex2D{v1.x, v1.y};
      const float alpha = (y - v1.y) / (v2.y - v1.y);
      return Vertex2D{lerp(v1.x, v2.x, alpha), lerp(v1.y, v2.y, alpha)};
   };
   auto submitEdge2D = [this, &intersect2D, &submitVertex](const RenderVertex &v1, const RenderVertex &v2, float y, float z)
   {
      if (Vertex2D pt = intersect2D(v1, v2, y); pt.x != FLT_MAX)
         submitVertex(Vertex3Ds{pt.x, pt.y, z});
   };
   auto intersect3D = [](const Vertex3Ds &v1, const Vertex3Ds &v2, float y)
   {
      if ((v1.y < y - marginY && v2.y < y - marginY) || (v1.y > y + marginY && v2.y > y + marginY))
         return Vertex3Ds{FLT_MAX, FLT_MAX, FLT_MAX};
      if (fabs(v2.y - v1.y) < 0.01f)
         return Vertex3Ds{v1.x, v1.y, max(v1.z, v2.z)};
      const float alpha = (y - v1.y) / (v2.y - v1.y);
      return Vertex3Ds{lerp(v1.x, v2.x, alpha), lerp(v1.y, v2.y, alpha), lerp(v1.z, v2.z, alpha)};
   };
   auto submitEdge3D = [this, &intersect3D, &submitVertex](const Vertex3Ds &v1, const Vertex3Ds &v2, float y)
   {
      if (Vertex3Ds pt = intersect3D(v1, v2, y); pt.x != FLT_MAX)
         submitVertex(pt);
   };

   IEditable *upperEditableX = nullptr;
   IEditable *upperEditableY = nullptr;
   for (const auto edit : m_vedit)
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
            pedit->m_uiLocked = lock;
         }
      }
   }
   EndUndo();
   SetDirtyDraw();
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
   strncpy_s(szObjFileName, std::size(szObjFileName), m_filename.string().c_str());
   const size_t idx = m_filename.string().find_last_of('.');
   if (idx != string::npos && idx < std::size(szObjFileName))
      szObjFileName[idx] = '\0';
   OPENFILENAME ofn = {};
   ofn.lStructSize = sizeof(OPENFILENAME);
   ofn.hInstance = g_app->GetInstanceHandle();
   ofn.hwndOwner = m_vpinball->GetHwnd();
   // TEXT
   ofn.lpstrFilter = "Wavefront obj(*.obj)\0*.obj\0";
   ofn.lpstrFile = szObjFileName;
   ofn.nMaxFile = std::size(szObjFileName);
   ofn.lpstrDefExt = "obj";
   ofn.Flags = OFN_NOREADONLYRETURN | OFN_CREATEPROMPT | OFN_OVERWRITEPROMPT | OFN_EXPLORER;

   const int ret = GetSaveFileName(&ofn);

   // user cancelled
   if (ret == 0)
      return;// S_FALSE;
   const string filename = szObjFileName;

   ObjLoader loader;
   loader.ExportStart(filename);
   ExportMesh(loader);
   for (const auto pedit : m_vedit)
      if (pedit->m_uiVisible && pedit->m_desktopBackdrop == m_vpinball->m_desktopBackdropView)
         pedit->ExportMesh(loader);

   loader.ExportEnd();
   m_vpinball->MessageBox("Export finished!", "Info", MB_OK | MB_ICONEXCLAMATION);
#endif
}

// Import Point of View file. This can be either:
// - a UI interaction from table author, loading to table **properties** after file selection,
// - without UI interaction, triggered to load user settings preference to table **settings**.
void PinTable::ImportBackdropPOV(const std::filesystem::path &filename)
{
   std::filesystem::path file = filename;
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
      if(file.has_parent_path())
         g_app->m_settings.SetRecentDir_POVDir(file.parent_path().string(), false);
#endif
   }

   const string ext = lowerCase(file.extension().string());

   static const string vsPrefix[3] = { "ViewDT"s, "ViewCab"s, "ViewFSS"s };
   static const char *vsFields[15] = { "Mode", "ScaleX", "ScaleY", "ScaleZ", "PlayerX", "PlayerY", "PlayerZ", "LookAt", "Rotation", "FOV", "Layback", "HOfs", "VOfs", "WindowTop", "WindowBot" };
   if (ext == ".ini")
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
   else if (ext == ".pov" || ext == ".xml")
   {
      tinyxml2::XMLDocument xmlDoc;
      try
      {
         std::stringstream buffer;
         std::ifstream myFile(file);
         buffer << myFile.rdbuf();
         myFile.close();
         const string& xml = buffer.str();
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
	ofn.hInstance = g_app->GetInstanceHandle();
	ofn.hwndOwner = m_vpinball->GetHwnd();
	// TEXT
	ofn.lpstrFilter = "INI file(*.ini)\0*.ini\0";
	char szFileName[MAXSTRING];
   strncpy_s(szFileName, std::size(szFileName), m_filename.string().c_str());
   const size_t idx = m_filename.string().find_last_of('.');
	if(idx != string::npos && idx < std::size(szFileName))
		szFileName[idx] = '\0';
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = std::size(szFileName);
	ofn.lpstrDefExt = "ini";
	ofn.Flags = OFN_NOREADONLYRETURN | OFN_CREATEPROMPT | OFN_OVERWRITEPROMPT | OFN_EXPLORER;
	const int ret = GetSaveFileName(&ofn);
	// user cancelled
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
         g_pplayer->m_liveUI->PushNotification("POV exported to " + iniFileName, 5000);
   }
   else if (g_pplayer)
   {
      g_pplayer->m_liveUI->PushNotification("POV was not exported to " + iniFileName + " (nothing to save)", 5000);
   }

   PLOGI << "View setup exported to '" << iniFileName << '\'';
}

void PinTable::SelectItem(IScriptable *piscript)
{
   for (const auto pedit : m_vedit)
   {
      if (piscript == pedit->GetIScriptable())
      {
         if (ISelect *const pisel = pedit->GetISelect(); pisel)
            AddMultiSel(pisel, false, true, false);
         break;
      }
   }
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

   if (m_tableEditor && sdsNewDirtyState != m_sdsCurrentDirtyState)
   {
      if (sdsNewDirtyState > eSaveClean)
         m_tableEditor->SetCaption(m_title + '*');
      else
         m_tableEditor->SetCaption(m_title);
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

   if (m_tableEditor)
      m_tableEditor->OnPartChanged(this);
}

void PinTable::Uncreate(IEditable *pie)
{
   if (pie->GetISelect()->m_selectstate != SelectState::NotSelected)
      AddMultiSel(pie->GetISelect(), true, true, false); // Remove the item from the multi-select list

   pie->GetISelect()->Uncreate();
   pie->Release();
}

void PinTable::Undelete(IEditable *pie)
{
   AddPart(pie);
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
       ISelect *const pItem = m_tableEditor->HitTest(x, y);
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

       BiffWriter writer(pstm, 0);
       pe->Save(writer, false);

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
       ISelect * const pItem = m_tableEditor->HitTest(x, y);
       if (pItem->GetItemType() == eItemDragPoint)
       {
           DragPoint * const pPoint = (DragPoint*)pItem;
           pPoint->Paste();
           SetDirtyDraw();
           return;
       }
   }

   const unsigned viewflag = (m_vpinball->m_desktopBackdropView ? VIEW_BACKGLASS : VIEW_PLAYFIELD);

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
            BiffReader reader(pstm, CURRENT_FILE_FORMAT_VERSION, NULL, NULL);
            peditNew->Load(reader);
            peditNew->m_desktopBackdrop = m_vpinball->m_desktopBackdropView;
            //if the original name is not yet used, use that one (so there's nothing we have to do) otherwise add/increase the suffix until we find a name that's not used yet
            if (!IsNameUnique(peditNew->GetWName()))
            {
               //first remove the existing suffix
               const wstring input = peditNew->GetWName();
               size_t lastNonDigit = input.length();
               while (lastNonDigit > 0 && iswdigit(input[lastNonDigit - 1]))
                  --lastNonDigit;
               peditNew->SetName(GetUniqueName(input.substr(0, lastNonDigit)));
            }
            peditNew->SetPartGroup(m_vpinball->GetLayersListDialog()->GetSelectedPartGroup());

            AddPart(peditNew);

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
      ShowError(LocalString(IDS_NOPASTEINVIEW).m_szbuffer);
#endif
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
      m_vmultisel[i].m_selectstate = SelectState::NotSelected;

   //remove the clone of the multi selection in the smart browser class
   //to sync the clone and the actual multi-selection 
   //it will be updated again on AddMultiSel() call
   m_vmultisel.clear();

   if (newSel == nullptr)
      newSel = this;
   m_vmultisel.push_back(newSel);
   newSel->m_selectstate = SelectState::Selected;
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
      _ASSERTE(psel->m_selectstate == SelectState::NotSelected);
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
                     col->m_visel[i].m_selectstate = SelectState::MultiSelected;
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
            piSelect->m_selectstate = SelectState::MultiSelected;
         m_vmultisel.insert(psel, 0);
      }

      psel->m_selectstate = SelectState::Selected;

      if (update)
         SetDirtyDraw();
   }
   else if (add) // Take the element off the list
   {
      _ASSERTE(psel->m_selectstate != SelectState::NotSelected);
      m_vmultisel.erase(index);
      psel->m_selectstate = SelectState::NotSelected;
      if (m_vmultisel.empty())
      {
         // Have to have something selected
         m_vmultisel.push_back((ISelect *)this);
      }
      // The main element might have changed
      piSelect = m_vmultisel.ElementAt(0);
      if (piSelect != nullptr)
         piSelect->m_selectstate = SelectState::Selected;

      if (update)
         SetDirtyDraw();
   }
   else if (m_vmultisel.ElementAt(0) != psel) // Object already in list - no change to selection, only to primary
   {
      int colIndex = -1;
      int elemIndex = -1;
      if (!GetCollectionIndex(psel, colIndex, elemIndex))
      {
         _ASSERTE(psel->m_selectstate != SelectState::NotSelected);

         // Make this new selection the primary one for the group
         piSelect = m_vmultisel.ElementAt(0);
         if (piSelect != nullptr)
            piSelect->m_selectstate = SelectState::MultiSelected;
         m_vmultisel.erase(index);
         m_vmultisel.insert(psel, 0);

         psel->m_selectstate = SelectState::Selected;
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
    if (piSelect && piSelect->GetIEditable() && piSelect->GetIEditable()->GetIScriptable())
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
        if (m_tableEditor)
            m_tableEditor->m_pcv->SelectItem(piSelect->GetIEditable()->GetIScriptable());
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
      const int ans = m_tableEditor->MessageBox(LocalString(IDS_DELETE_ELEMENTS).m_szbuffer /*"Selected elements are part of one or more collections.\nDo you really want to delete them?"*/,
         "Visual Pinball", MB_YESNO | MB_DEFBUTTON2);
      if (ans != IDYES)
         return;
   }

   for (size_t i = 0; i < m_vseldelete.size(); i++)
      if (m_vseldelete[i] != nullptr)
         m_vseldelete[i]->Delete();
   m_vpinball->GetLayersListDialog()->Update();
   // update properties to show the properties of the table
   m_vpinball->SetPropSel(m_vmultisel);
   if (m_tableEditor)
      m_tableEditor->OnPartChanged(this);

   SetDirtyDraw();
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
      if (auto scriptable = pie->GetIScriptable(); scriptable)
         GetUniqueName(type, scriptable->m_wzName);
      pie->m_desktopBackdrop = m_vpinball->m_desktopBackdropView;
      AddPart(pie);
      pie->SetPartGroup(m_vpinball->GetLayersListDialog()->GetSelectedPartGroup());
      m_vpinball->GetLayersListDialog()->Update();

      if (m_tableEditor)
         m_tableEditor->OnPartChanged(this);

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
   const CRect rc = m_tableEditor->GetClientRect();
#else
   const CRect rc(m_left, m_top, m_right, m_bottom);
#endif
   const HitSur phs(nullptr, m_tableEditor->GetZoom(), m_tableEditor->GetViewOffset().x, m_tableEditor->GetViewOffset().y, rc.right - rc.left, rc.bottom - rc.top, 0, 0, nullptr);

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

   m_tableEditor->SetCapture();

   SetDirtyDraw();
#endif
}

HRESULT PinTable::GetTypeName(BSTR *pVal) const
{
   const int stringid = (!m_vpinball->m_desktopBackdropView) ? IDS_TABLE : IDS_TB_BACKGLASS;
   const LocalStringW lsw(stringid);
   *pVal = SysAllocStringLen(lsw.m_buffer.c_str(),static_cast<UINT>(lsw.m_buffer.length()));
   return S_OK;
}

STDMETHODIMP PinTable::get_FileName(BSTR *pVal)
{
   *pVal = MakeWideBSTR(m_title);
   return S_OK;
}

STDMETHODIMP PinTable::get_Name(BSTR *pVal)
{
   *pVal = SysAllocStringLen(m_wzName.c_str(), static_cast<UINT>(m_wzName.length()));
   return S_OK;
}

STDMETHODIMP PinTable::put_Name(BSTR newVal)
{
   SetName(newVal);
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

   for (const auto pedit : m_vedit)
      if (pedit->GetItemType() == ItemTypeEnum::eItemLight && StrCompareNoCase(pedit->GetName(), szName))
         return (Light *)pedit;

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

Texture *PinTable::ImportImage(const std::filesystem::path &filename, const string &imagename)
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
      return m_dummyMaterial.get();

   // during playback, we use the hashtable for lookup
   if (!m_materialMap.empty())
   {
      const ankerl::unordered_dense::map<string, Material*, StringHashFunctor, StringComparator>::const_iterator
         it = m_materialMap.find(name);
      if (it != m_materialMap.end())
         return it->second;
      else
         return m_dummyMaterial.get();
   }

   for (size_t i = 0; i < m_materials.size(); i++)
      if(m_materials[i]->m_name == name)
         return m_materials[i];

   return m_dummyMaterial.get();
}

void PinTable::AddMaterial(Material * const pmat)
{
   if (pmat->m_name.empty() || pmat->m_name == "dummyMaterial")
      pmat->m_name = "Material"sv;

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
   ListView_SetItemText_Safe(hwndListView, index, 1, usedStringNo);
   if(pmat->m_name == m_playfieldMaterial)
   {
      ListView_SetItemText_Safe(hwndListView, index, 1, usedStringYes);
   }
   else
   {
      for (const auto pEdit : m_vedit)
      {
         bool inUse = false;
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
            ListView_SetItemText_Safe(hwndListView, index, 1, usedStringYes);
            break;
         }
      }//for
   }
   return index;
#else
   return 0;
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

void PinTable::ParseScript(const string& script, vector<string>& functions, vector<string>& identifiers, const std::function<void(const string&, int)>& onDuplicate) const
{
   // Ultra basic parser to get a (somewhat) valid list of referenced parts
   const char *const szText = m_script_text.c_str();
   const char *wordStart = nullptr;
   const char *wordPos = szText;
   string inClass;
   bool nextIsFunc = false, nextIsEnd = false, nextIsClass = false, isInString = false, isInComment = false;
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
         else if (wordPos[0] == ' ' || wordPos[0] == '\r' || wordPos[0] == '\t' || wordPos[0] == '\n' || wordPos[0] == '.' || wordPos[0] == ':' || wordPos[0] == '(' || wordPos[0] == ')'
            || wordPos[0] == '[' || wordPos[0] == ']')
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
                           onDuplicate(word, line);
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
}

string PinTable::AuditTable(bool log) const
{
   // Perform a simple table audit (disable lighting vs static, script reference of static parts, png vs webp, hdr vs exr,...)
   std::stringstream ss;

   vector<string> functions;
   vector<string> identifiers;
   ParseScript(m_script_text, functions, identifiers,
      [&ss](const string &functionName, int line) { ss << ". Error: Duplicate declaration of '" << functionName << "' in script at line " << line << "\r\n"; });

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
      switch (type)
      {
      case eItemBall:
      // eItemDecal
      case eItemPartGroup:
      // eItemPrimitive
      case eItemTextbox:
      case eItemDispReel:
      case eItemSurface:
      case eItemTimer:
      case eItemLight:
      case eItemRamp:
      case eItemPlunger:
      case eItemSpinner:
      case eItemTrigger:
      case eItemKicker:
      case eItemRubber:
      case eItemFlasher:
      case eItemLightSeq:
      case eItemHitTarget:
      case eItemBumper:
      case eItemFlipper:
      case eItemGate:
         if (part->m_timerEnabled && part->m_timerInterval != -1 && part->m_timerInterval != -2 && part->m_timerInterval < 17)
            ss << ". Warning: Part '" << part->GetName() << "' uses a timer with a very short period of " << part->m_timerInterval << "ms, below a 60FPS framerate. This will likely cause stutters and the table will not support 'frame pacing'.\r\n";
         break;
      // eItemTable
      // eItemLightCenter
      // eItemDragPoint
      // eItemCollection
      default: break;
      }

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
   for (const auto sound : m_vsound)
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

   ListView_SetItemText_Safe(hwndListView, index, 1, szValue1.c_str());

   return index;
#else
   return 0;
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

      const wstring tmp = LocalStringW(IDS_NONE).m_buffer;
      rgstr[0] = (WCHAR *)CoTaskMemAlloc((tmp.length()+1) * sizeof(WCHAR));
      wcsncpy_s(rgstr[0], tmp.length()+1, tmp.c_str());
      rgdw[0] = ~0u;

      for (size_t ivar = 0; ivar < cvar; ivar++)
      {
         const int cwch = (int)m_vimage[ivar]->m_name.length() + 1;
         WCHAR* const wzDst = (WCHAR *)CoTaskMemAlloc(cwch*sizeof(WCHAR));
         if (wzDst == nullptr)
            ShowError("DISPID_Image alloc failed");
         else
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

      const wstring tmp = LocalStringW(IDS_NONE).m_buffer;
      rgstr[0] = (WCHAR *)CoTaskMemAlloc((tmp.length()+1) * sizeof(WCHAR));
      wcsncpy_s(rgstr[0], tmp.length()+1, tmp.c_str());
      rgdw[0] = ~0u;

      for (size_t ivar = 0; ivar < cvar; ivar++)
      {
         const int cwch = (int)m_materials[ivar]->m_name.length() + 1;
         WCHAR* const wzDst = (WCHAR *)CoTaskMemAlloc(cwch*sizeof(WCHAR));
         if (wzDst == nullptr)
            ShowError("IDC_MATERIAL_COMBO alloc failed");
         else
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
         else
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
         const size_t len = m_vcollection[(int)ivar].m_wzName.length();
         rgstr[ivar + 1] = (WCHAR *)CoTaskMemAlloc((len + 1) * sizeof(WCHAR));
         if (rgstr[ivar + 1] == nullptr)
            ShowError("DISPID_Collection alloc failed (1)");
         else
         {
            memcpy(rgstr[ivar + 1], m_vcollection[(int)ivar].m_wzName.c_str(), len * sizeof(WCHAR));
            rgstr[ivar + 1][len] = L'\0';
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

      for (const auto edit : m_vedit)
         if (edit->GetItemType() == eItemSurface ||
            (ramps && edit->GetItemType() == eItemRamp) ||
            //!! **************** warning **********************
            // added to render to surface of DMD style lights and emreels
            // but no checks are being performed at moment:
            (flashers && edit->GetItemType() == eItemFlasher))
            cvar++;

      rgstr = (WCHAR **)CoTaskMemAlloc((cvar + 1) * sizeof(WCHAR *));
      rgdw = (uint32_t *)CoTaskMemAlloc((cvar + 1) * sizeof(uint32_t));

      cvar = 0;

      WCHAR *wzDst = (WCHAR *)CoTaskMemAlloc(7 * sizeof(WCHAR));
      if (wzDst == nullptr)
         ShowError("DISPID_Surface alloc failed (0)");
      else
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
            // but no checks are being performed at the moment:
            (flashers && m_vedit[ivar]->GetItemType() == eItemFlasher))
         {
            const wstring& sname = m_vedit[ivar]->GetIScriptable()->m_wzName;

            const size_t len = sname.length();
            //wzDst = ::SysAllocString(bstr);

            wzDst = (WCHAR *)CoTaskMemAlloc((len+1) * sizeof(WCHAR));
            if (wzDst == nullptr)
               ShowError("DISPID_Surface alloc failed (1)");
            else
            {
               memcpy(wzDst, sname.c_str(), len * sizeof(WCHAR));
               wzDst[len] = L'\0';
            }
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
         wzDst = new WCHAR[1];
         wzDst[0] = L'\0';
      }
      else
         wzDst = MakeWide(m_vimage[dwCookie]->m_name);
   }
   break;
   case IDC_MATERIAL_COMBO:
   case IDC_MATERIAL_COMBO2:
   case IDC_MATERIAL_COMBO3:
   case IDC_MATERIAL_COMBO4:
   {
      if (dwCookie == -1)
      {
         wzDst = new WCHAR[1];
         wzDst[0] = L'\0';
      }
      else
         wzDst = MakeWide(m_materials[dwCookie]->m_name);
      break;
   }
   case DISPID_Sound:
   {
      if (dwCookie == -1)
      {
         wzDst = new WCHAR[1];
         wzDst[0] = L'\0';
      }
      else
         wzDst = MakeWide(m_vsound[dwCookie]->GetName());
   }
   break;
   case DISPID_Collection:
   {
      if (dwCookie == -1)
      {
         wzDst = new WCHAR[1];
         wzDst[0] = L'\0';
      }
      else
      {
         const size_t len = m_vcollection[(int)dwCookie].m_wzName.length();
         wzDst = new WCHAR[len+1];
         memcpy(wzDst, m_vcollection[(int)dwCookie].m_wzName.c_str(), len * sizeof(WCHAR));
         wzDst[len] = L'\0';
      }
   }
   break;
   case IDC_EFFECT_COMBO:
   {
      const int idx = (dwCookie == -1) ? 0 : dwCookie;
      static const wstring filterNames[5] = { L"None"s, L"Additive"s, L"Multiply"s, L"Overlay"s, L"Screen"s };
      const size_t cwch = filterNames[idx].length() + 1;
      wzDst = new WCHAR[cwch];
      wcsncpy_s(wzDst, cwch, filterNames[idx].c_str());
      break;
   }
   case DISPID_Surface:
   {
      if (dwCookie == -1)
      {
         wzDst = new WCHAR[1];
         wzDst[0] = L'\0';
      }
      else
      {
         const wstring& sname = m_vedit[dwCookie]->GetIScriptable()->m_wzName;
         const size_t len = sname.length();
         wzDst = new WCHAR[len+1];
         memcpy(wzDst, sname.c_str(), len * sizeof(WCHAR));
         wzDst[len] = L'\0';
      }
   }
   break;
   }

   CComVariant var(wzDst);

   delete [] wzDst;

   return var.Detach(pVarOut);
}

float PinTable::GetSurfaceHeight(const string& name, float x, float y) const
{
   if (name.empty())
      return 0.f;

   const wstring wname = MakeWString(name);
   for (const auto item : m_vedit)
   {
      const ItemTypeEnum type = item->GetItemType();
      if ((type == eItemSurface || type == eItemRamp) && (wname == item->GetIScriptable()->m_wzName))
         return type == eItemSurface ? static_cast<const Surface *>(item)->m_d.m_heighttop : static_cast<const Ramp *>(item)->GetSurfaceHeight(x, y);
   }

   PLOGE << "Failed to find part '" << name << "' to set other part height";
   return 0.f;
}

Material* PinTable::GetSurfaceMaterial(const wstring& name) const
{
   if (name.empty())
      return GetMaterial(m_playfieldMaterial);

   for (const auto item : m_vedit)
   {
      const ItemTypeEnum type = item->GetItemType();
      if ((type == eItemSurface || type == eItemRamp) && (name == item->GetIScriptable()->m_wzName))
         return GetMaterial(type == eItemSurface ? static_cast<const Surface *>(item)->m_d.m_szTopMaterial : static_cast<const Ramp *>(item)->m_d.m_szMaterial);
   }

   PLOGE << "Failed to find part '" << MakeString(name) << "' to set other part material";
   return GetMaterial(m_playfieldMaterial);
}

Texture* PinTable::GetSurfaceImage(const wstring& name) const
{
   if (name.empty())
      return GetImage(m_image);

   for (const auto item : m_vedit)
   {
      const ItemTypeEnum type = item->GetItemType();
      if ((type == eItemSurface || type == eItemRamp) && (name == item->GetIScriptable()->m_wzName))
         return GetImage(type == eItemSurface ? static_cast<const Surface *>(item)->m_d.m_szImage : static_cast<const Ramp *>(item)->m_d.m_szImage);
   }

   PLOGE << "Failed to find part '" << MakeString(name) << "' to set other part image";
   return GetImage(m_image);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP PinTable::get_DisplayGrid(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_tableEditor->GetDisplayGrid());
   return S_OK;
}

STDMETHODIMP PinTable::put_DisplayGrid(VARIANT_BOOL newVal)
{
   STARTUNDO
   m_tableEditor->SetDisplayGrid(VBTOb(newVal));
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_DisplayBackdrop(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_tableEditor->GetDisplayBackdrop());
   return S_OK;
}

STDMETHODIMP PinTable::put_DisplayBackdrop(VARIANT_BOOL newVal)
{
   STARTUNDO
   m_tableEditor->SetDisplayBackdrop(VBTOb(newVal));
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

   m_tableEditor->SetMyScrollInfo();
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

   m_tableEditor->SetMyScrollInfo();
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
   m_friction = saturate(value);
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
      return E_FAIL;
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
      return E_FAIL;
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
      g_app->m_settings.SetRecentDir_PhysicsDir(filename[0].substr(0, index), false);

   ImportVPP(filename[0]);

   return S_OK;
}

std::array<string,18> PinTable::VPPelementNames{"gravityConstant"s, "contactFriction"s, "elasticity"s, "elasticityFalloff"s, "playfieldScatter"s, "defaultElementScatter"s, "playfieldminslope"s, "playfieldmaxslope"s,
                               /*flippers:*/    "speed"s, "strength"s, "elasticity"s, "scatter"s, "eosTorque"s, "eosTorqueAngle"s, "returnStrength"s, "elasticityFalloff"s, "friction"s, "coilRampUp"s};

void PinTable::ImportVPP(const std::filesystem::path &filename)
{
   tinyxml2::XMLDocument xmlDoc;
   try
   {
      std::stringstream buffer;
      std::ifstream myFile(filename);
      buffer << myFile.rdbuf();
      myFile.close();
      const string& xml = buffer.str();

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
   strncpy_s(szFileName, std::size(szFileName), m_filename.string().c_str());
   const size_t idx = m_filename.string().find_last_of('.');
   if (idx != string::npos && idx < std::size(szFileName))
      szFileName[idx] = '\0';

   OPENFILENAME ofn = {};
   ofn.lStructSize = sizeof(OPENFILENAME);
   ofn.hInstance = g_app->GetInstanceHandle();
   ofn.hwndOwner = m_vpinball->GetHwnd();
   // TEXT
   ofn.lpstrFilter = "Visual Pinball Physics (*.vpp)\0*.vpp\0";
   ofn.lpstrFile = szFileName;
   ofn.nMaxFile = std::size(szFileName);
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
      g_app->m_settings.SetRecentDir_PhysicsDir(newInitDir, false);
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
   if (g_pplayer)
      g_pplayer->SetCloseState((Player::CloseState)CloseType);
   return S_OK;
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
   if (g_pplayer && g_pplayer->m_scriptInterpreter)
   {
      CComPtr<IDispatch> disp;
      g_pplayer->m_scriptInterpreter->GetScriptDispatch(&disp);

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

void PinTable::ShowWhereImagesUsed(vector<WhereUsedInfo> &vWhereUsed)
{
   for (size_t i = 0; i < m_vimage.size(); i++)
      ShowWhereImageUsed(vWhereUsed, m_vimage[i]);
}

// also change decal special cases below when changing this snippet
#define INSERT_WHERE_USED(x) \
{ \
   whereUsed.searchObjectName = searchObjectName; \
   whereUsed.whereUsedObjectname = pEdit->GetName(); \
   whereUsed.whereUsedPropertyName = (x); \
   vWhereUsed.push_back(whereUsed); \
}

void PinTable::ShowWhereImageUsed(vector<WhereUsedInfo> &vWhereUsed, Texture *const ppi)
{
   for (const auto pEdit : m_vedit)
   {
      if (pEdit == nullptr)
         continue;

      WhereUsedInfo whereUsed;
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
   for (const auto pEdit : m_vedit)
   {
      if (pEdit == nullptr)
         continue;

      WhereUsedInfo whereUsed;
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
// From here on, only deprecated properties exposed to scripting API

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
   *pVal = 0;
   if (!g_pplayer)
      return E_FAIL;
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

STDMETHODIMP PinTable::get_PlungerNormalize(int *pVal)
{
   PLOGE << "PlungerNormalize is deprecated";
   *pVal = 100;
   return S_OK;
}

STDMETHODIMP PinTable::put_PlungerNormalize(int newVal)
{
   PLOGE << "PlungerNormalize is deprecated";
   return S_OK;
}

// Changing AA & FXAA is somewhat wrong as it changes the setting for all time, and is not implemented while playing, so this is just a No-Op
STDMETHODIMP PinTable::get_EnableAntialiasing(UserDefaultOnOff *pVal)
{
   PLOGE << "EnableAntialiasing is deprecated";
   *pVal = UserDefaultOnOff::Default;
   return S_OK;
}

// Changing AA & FXAA is somewhat wrong as it changes the setting for all time, and is not implemented while playing, so this is just a No-Op
STDMETHODIMP PinTable::put_EnableAntialiasing(UserDefaultOnOff newVal)
{
   PLOGE << "EnableAntialiasing is deprecated";
   return S_OK;
}

// Changing AA & FXAA is somewhat wrong as it changes the setting for all time, and is not implemented while playing, so this is just a No-Op
STDMETHODIMP PinTable::get_EnableFXAA(FXAASettings *pVal)
{
   PLOGE << "EnableFXAA is deprecated";
   *pVal = FXAASettings::Defaults;
   return S_OK;
}

// Changing AA & FXAA is somewhat wrong as it changes the setting for all time, and is not implemented while playing, so this is just a No-Op
STDMETHODIMP PinTable::put_EnableFXAA(FXAASettings newVal)
{
   PLOGE << "EnableFXAA is deprecated";
   return S_OK;
}

// All the following will not work as expected as they never took into account static rendering (and meanwhile camera setup has changed)

STDMETHODIMP PinTable::get_BackglassMode(BackglassIndex *pVal)
{
   PLOGE << "BackglassMode is deprecated";
   *pVal = static_cast<BackglassIndex>(static_cast<int>(m_viewMode) + static_cast<int>(DESKTOP));
   return S_OK;
}

STDMETHODIMP PinTable::put_BackglassMode(BackglassIndex pVal)
{
   PLOGE << "BackglassMode is deprecated";
   m_viewMode = (ViewSetupID)(pVal - DESKTOP);
   return S_OK;
}

STDMETHODIMP PinTable::get_FieldOfView(float *pVal)
{
   PLOGE << "FieldOfView is deprecated";
   *pVal = mViewSetups[m_viewMode].mFOV;
   return S_OK;
}

STDMETHODIMP PinTable::put_FieldOfView(float newVal)
{
   PLOGE << "FieldOfView is deprecated";
   STARTUNDO
   mViewSetups[m_viewMode].mFOV = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Inclination(float *pVal)
{
   PLOGE << "Inclination is deprecated";
   *pVal = mViewSetups[m_viewMode].mLookAt;
   return S_OK;
}

STDMETHODIMP PinTable::put_Inclination(float newVal)
{
   PLOGE << "Inclination is deprecated";
   STARTUNDO
   mViewSetups[m_viewMode].mLookAt = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Layback(float *pVal)
{
   PLOGE << "Layback is deprecated";
   *pVal = mViewSetups[m_viewMode].mLayback;
   return S_OK;
}

STDMETHODIMP PinTable::put_Layback(float newVal)
{
   PLOGE << "Layback is deprecated";
   STARTUNDO
   mViewSetups[m_viewMode].mLayback = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Rotation(float *pVal)
{
   PLOGE << "Rotation is deprecated";
   *pVal = mViewSetups[m_viewMode].mViewportRotation;
   return S_OK;
}

STDMETHODIMP PinTable::put_Rotation(float newVal)
{
   PLOGE << "Rotation is deprecated";
   STARTUNDO
   mViewSetups[m_viewMode].mViewportRotation = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Scalex(float *pVal)
{
   PLOGE << "Scalex is deprecated";
   *pVal = mViewSetups[m_viewMode].mSceneScaleX;
   return S_OK;
}

STDMETHODIMP PinTable::put_Scalex(float newVal)
{
   PLOGE << "Scalex is deprecated";
   STARTUNDO
   mViewSetups[m_viewMode].mSceneScaleX = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Scaley(float *pVal)
{
   PLOGE << "Scaley is deprecated";
   *pVal = mViewSetups[m_viewMode].mSceneScaleY;
   return S_OK;
}

STDMETHODIMP PinTable::put_Scaley(float newVal)
{
   PLOGE << "Scaley is deprecated";
   STARTUNDO
   mViewSetups[m_viewMode].mSceneScaleY = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Scalez(float *pVal)
{
   PLOGE << "Scalez is deprecated";
   *pVal = mViewSetups[m_viewMode].mSceneScaleZ;
   return S_OK;
}

STDMETHODIMP PinTable::put_Scalez(float newVal)
{
   PLOGE << "Scalez is deprecated";
   STARTUNDO
   mViewSetups[m_viewMode].mSceneScaleZ = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Xlatex(float *pVal)
{
   PLOGE << "Xlatex is deprecated";
   *pVal = mViewSetups[m_viewMode].mViewX;
   return S_OK;
}

STDMETHODIMP PinTable::put_Xlatex(float newVal)
{
   PLOGE << "Xlatex is deprecated";
   STARTUNDO
   mViewSetups[m_viewMode].mViewX = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Xlatey(float *pVal)
{
   PLOGE << "Xlatey is deprecated";
   *pVal = mViewSetups[m_viewMode].mViewY;
   return S_OK;
}

STDMETHODIMP PinTable::put_Xlatey(float newVal)
{
   PLOGE << "Xlatey is deprecated";
   STARTUNDO
   mViewSetups[m_viewMode].mViewY = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Xlatez(float *pVal)
{
   PLOGE << "Xlatez is deprecated";
   *pVal = mViewSetups[m_viewMode].mViewZ;
   return S_OK;
}

STDMETHODIMP PinTable::put_Xlatez(float newVal)
{
   PLOGE << "Xlatez is deprecated";
   STARTUNDO
   mViewSetups[m_viewMode].mViewZ = newVal;
   STOPUNDO

   return S_OK;
}
