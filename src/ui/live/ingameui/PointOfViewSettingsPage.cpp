// license:GPLv3+

#include "core/stdafx.h"

#include "PointOfViewSettingsPage.h"
#include "core/TableDB.h"

namespace VPX::InGameUI
{

PointOfViewSettingsPage::PointOfViewSettingsPage()
   : InGameUIPage("settings/pov", "Point of View", "Point of view's settings page:\nOptions to define rendering's point of view")
{
}

void PointOfViewSettingsPage::Open()
{
   assert(!m_opened);
   InGameUIPage::Open();
   m_opened = true;
   m_staticPrepassDisabled = false;
   const PinTable* const table = m_player->m_ptable;
   ViewSetupID vsId = table->m_BG_current_set;
   m_initialViewSetup = table->mViewSetups[vsId];
   const Settings& settings = GetSettings();
   m_playerPos.x = settings.LoadValueFloat(Settings::Player, "ScreenPlayerX"s);
   m_playerPos.y = settings.LoadValueFloat(Settings::Player, "ScreenPlayerY"s);
   m_playerPos.z = settings.LoadValueFloat(Settings::Player, "ScreenPlayerZ"s);
   m_initialPlayerPos = m_playerPos;
   BuildPage();
}

void PointOfViewSettingsPage::Close()
{
   assert(m_opened);
   m_opened = false;
   if (m_staticPrepassDisabled)
      m_player->m_renderer->DisableStaticPrePass(false);
}

void PointOfViewSettingsPage::Save()
{
   Settings& settings = GetSettings();
   const PinTable* const table = m_player->m_ptable;
   ViewSetupID vsId = table->m_BG_current_set;
   const string keyPrefix = vsId == BG_DESKTOP ? "ViewDT"s : vsId == BG_FSS ? "ViewFSS"s : "ViewCab"s;
   settings.DeleteValue(Settings::TableOverride, keyPrefix + "Mode");
   settings.DeleteValue(Settings::TableOverride, keyPrefix + "ScaleX");
   settings.DeleteValue(Settings::TableOverride, keyPrefix + "ScaleY");
   settings.DeleteValue(Settings::TableOverride, keyPrefix + "ScaleZ");
   settings.DeleteValue(Settings::TableOverride, keyPrefix + "Rotation");
   settings.DeleteValue(Settings::TableOverride, keyPrefix + "PlayerX");
   settings.DeleteValue(Settings::TableOverride, keyPrefix + "PlayerY");
   settings.DeleteValue(Settings::TableOverride, keyPrefix + "PlayerZ");
   settings.DeleteValue(Settings::TableOverride, keyPrefix + "LookAt");
   settings.DeleteValue(Settings::TableOverride, keyPrefix + "FOV");
   settings.DeleteValue(Settings::TableOverride, keyPrefix + "HOfs");
   settings.DeleteValue(Settings::TableOverride, keyPrefix + "VOfs");
   settings.DeleteValue(Settings::TableOverride, keyPrefix + "WindowTop");
   settings.DeleteValue(Settings::TableOverride, keyPrefix + "WindowBot");
   settings.DeleteValue(Settings::TableOverride, keyPrefix + "Layback");
   InGameUIPage::Save();

   // FIXME this should be part of the action, not of the ingameui
   if (g_pvp->m_povEdit)
      g_pvp->QuitPlayer(Player::CloseState::CS_CLOSE_APP);
}

void PointOfViewSettingsPage::ResetToInitialValues()
{
   InGameUIPage::ResetToInitialValues();

   // FIXME this should be part of the action, not of the ingameui
   if (g_pvp->m_povEdit)
      g_pvp->QuitPlayer(Player::CloseState::CS_CLOSE_APP);
}

void PointOfViewSettingsPage::OnPointOfViewChanged()
{
   assert(m_opened);
   if (!m_staticPrepassDisabled)
   {
      m_player->m_renderer->DisableStaticPrePass(true);
      m_staticPrepassDisabled = true;
   }
   m_player->m_renderer->InitLayout();
}

void PointOfViewSettingsPage::BuildPage()
{
   assert(m_opened);
   PinTable* table = m_player->m_ptable;

   ClearItems();

   ViewSetupID vsId = table->m_BG_current_set;
   ViewSetup& viewSetup = table->mViewSetups[vsId];
   const bool isLegacy = viewSetup.mMode == VLM_LEGACY;
   const float realToVirtual = viewSetup.GetRealToVirtualScale(table);
   const string keyPrefix = vsId == BG_DESKTOP ? "ViewDT"s : vsId == BG_FSS ? "ViewFSS"s : "ViewCab"s;

   // Evaluate a suitable default depending on overall view mode
   ViewSetup defViewSetup;
   if (const bool portrait = m_player->m_playfieldWnd->GetWidth() < m_player->m_playfieldWnd->GetHeight(); vsId == BG_DESKTOP && !portrait)
   { // Desktop
      defViewSetup.mMode = (ViewLayoutMode)g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "DesktopMode"s, VLM_CAMERA);
      defViewSetup.mViewX = CMTOVPU(g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "DesktopCamX"s, 0.f));
      defViewSetup.mViewY = CMTOVPU(g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "DesktopCamY"s, 20.f));
      defViewSetup.mViewZ = CMTOVPU(g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "DesktopCamZ"s, 70.f));
      defViewSetup.mSceneScaleX = g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "DesktopScaleX"s, 1.f);
      defViewSetup.mSceneScaleY = g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "DesktopScaleY"s, 1.f);
      defViewSetup.mSceneScaleZ = g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "DesktopScaleZ"s, 1.f);
      defViewSetup.mFOV = g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "DesktopFov"s, 50.f);
      defViewSetup.mLookAt = g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "DesktopLookAt"s, 25.0f);
      defViewSetup.mViewVOfs = g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "DesktopViewVOfs"s, 14.f);
   }
   else if (vsId == BG_DESKTOP || vsId == BG_FSS)
   { // FSS
      defViewSetup.mMode = (ViewLayoutMode)g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "FSSMode"s, VLM_CAMERA);
      defViewSetup.mViewX = CMTOVPU(g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "FSSCamX"s, 0.f));
      defViewSetup.mViewY = CMTOVPU(g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "FSSCamY"s, 20.f));
      defViewSetup.mViewZ = CMTOVPU(g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "FSSCamZ"s, 70.f));
      defViewSetup.mSceneScaleX = g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "FSSScaleX"s, 1.f);
      defViewSetup.mSceneScaleY = g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "FSSScaleY"s, 1.f);
      defViewSetup.mSceneScaleZ = g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "FSSScaleZ"s, 1.f);
      defViewSetup.mFOV = g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "FSSFov"s, 77.f);
      defViewSetup.mLookAt = g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "FSSLookAt"s, 50.0f);
      defViewSetup.mViewVOfs = g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultCamera, "FSSViewVOfs"s, 22.f);
   }
   else if (vsId == BG_FULLSCREEN)
   { // Cabinet mode
      const float screenWidth = g_pvp->m_settings.LoadValueFloat(Settings::Player, "ScreenWidth"s);
      const float screenHeight = g_pvp->m_settings.LoadValueFloat(Settings::Player, "ScreenHeight"s);
      if (screenWidth <= 1.f || screenHeight <= 1.f)
      {
         // TODO include a link to the cabinet setting page with screen size setup
         auto info = std::make_unique<InGameUIItem>("You must setup your screen size before using Window mode"s);
         AddItem(info);
         return;
      }
      else
      {
         float topHeight = table->m_glassTopHeight;
         float bottomHeight = table->m_glassBottomHeight;
         if (bottomHeight == topHeight)
         { // If table does not define the glass position (for table without it, when loading we set the glass as horizontal)
            TableDB db;
            db.Load();
            int bestSizeMatch = db.GetBestSizeMatch(table->GetTableWidth(), table->GetHeight(), topHeight);
            if (bestSizeMatch >= 0)
            {
               bottomHeight = INCHESTOVPU(db.m_data[bestSizeMatch].glassBottom);
               topHeight = INCHESTOVPU(db.m_data[bestSizeMatch].glassTop);
               /* FIXME m_liveUI.PushNotification("Missing glass position guessed to be " + std::to_string(db.m_data[bestSizeMatch].glassBottom) + "\" / "
                     + std::to_string(db.m_data[bestSizeMatch].glassTop) + "\" (" + db.m_data[bestSizeMatch].name + ')',
                  5000); */
            }
            else
            {
               // FIXME m_liveUI.PushNotification("The table is missing glass position and no good guess was found."s, 5000);
            }
         }
         const float scale = (screenHeight / table->GetTableWidth()) * (table->GetHeight() / screenWidth);
         const bool isFitted = (viewSetup.mViewHOfs == 0.f) && (viewSetup.mViewVOfs == -2.8f) && (viewSetup.mSceneScaleY == scale) && (viewSetup.mSceneScaleX == scale);
         defViewSetup.mMode = VLM_WINDOW;
         defViewSetup.mViewHOfs = 0.f;
         defViewSetup.mViewVOfs = isFitted ? 0.f : -2.8f;
         defViewSetup.mSceneScaleX = scale;
         defViewSetup.mSceneScaleY = isFitted ? 1.f : scale;
         defViewSetup.mWindowBottomZOfs = bottomHeight;
         defViewSetup.mWindowTopZOfs = topHeight;
      }
   }

   auto viewMode = std::make_unique<InGameUIItem>(
      "View mode"s,
      "Select between 'Legacy' (old rendering mode with visually incorrect stretchs), 'Camera' (classic camera, for desktop) and 'Window' (custom projection designed for cabinet users) rendering mode"s,
      vector({ "Legacy"s, "Camera"s, "Window"s }), static_cast<int>(defViewSetup.mMode), [this]() { return static_cast<int>(GetCurrentViewSetup().mMode); },
      [this](int prev, int v)
      {
         GetCurrentViewSetup().mMode = static_cast<ViewLayoutMode>(v);
         OnPointOfViewChanged();
         BuildPage();
      },
      [keyPrefix](int v, Settings& settings, bool isTableOverride) { settings.SaveValue(Settings::TableOverride, keyPrefix + "Mode", v, isTableOverride); });
   viewMode->SetInitialValue(m_initialViewSetup.mMode);

   auto lookAt = std::make_unique<InGameUIItem>(
      "Look at"s, "Relative point of playfield where the camera is looking at"s, 0.f, 100.f, 0.1f, defViewSetup.mLookAt, "%4.1f %%"s, [this]() { return GetCurrentViewSetup().mLookAt; },
      [this](float prev, float v)
      {
         GetCurrentViewSetup().mLookAt = v;
         OnPointOfViewChanged();
      },
      [keyPrefix](float v, Settings& settings, bool isTableOverride) { settings.SaveValue(Settings::TableOverride, keyPrefix + "LookAt", v, isTableOverride); });
   lookAt->SetInitialValue(m_initialViewSetup.mLookAt);

   auto inclination = std::make_unique<InGameUIItem>(
      "Inclination"s, "View inclination"s, 0.f, 90.f, 0.1f, defViewSetup.mLookAt, "%4.1f °"s, [this]() { return GetCurrentViewSetup().mLookAt; },
      [this](float prev, float v)
      {
         GetCurrentViewSetup().mLookAt = v;
         OnPointOfViewChanged();
      },
      [keyPrefix](float v, Settings& settings, bool isTableOverride) { settings.SaveValue(Settings::TableOverride, keyPrefix + "LookAt", v, isTableOverride); });
   inclination->SetInitialValue(m_initialViewSetup.mLookAt);

   auto fov = std::make_unique<InGameUIItem>(
      "Field Of View (overall scale)"s, "Global view scale (same as XYZ scale)"s, 25.f, 90.f, 0.1f, defViewSetup.mFOV, "%4.1f °"s, [this]() { return GetCurrentViewSetup().mFOV; },
      [this](float prev, float v)
      {
         GetCurrentViewSetup().mFOV = v;
         OnPointOfViewChanged();
      },
      [keyPrefix](float v, Settings& settings, bool isTableOverride) { settings.SaveValue(Settings::TableOverride, keyPrefix + "FOV", v, isTableOverride); });
   fov->SetInitialValue(m_initialViewSetup.mFOV);

   auto layback = std::make_unique<InGameUIItem>(
      "Layback"s, "Fake visual stretch of the table to give more depth"s, 0.f, 90.f, 0.1f, defViewSetup.mLayback, "%4.1f  "s, [this]() { return GetCurrentViewSetup().mLayback; },
      [this](float prev, float v)
      {
         GetCurrentViewSetup().mLayback = v;
         OnPointOfViewChanged();
      },
      [keyPrefix](float v, Settings& settings, bool isTableOverride) { settings.SaveValue(Settings::TableOverride, keyPrefix + "Layback", v, isTableOverride); });
   layback->SetInitialValue(m_initialViewSetup.mLayback);

   auto lockScale = std::make_unique<InGameUIItem>(
      "Lock XYZ Scale"s, "Scale all axis homogeneously (recommended)"s, true, 
      [this]() { return m_lockScale; },
      [this](bool v) { m_lockScale = v; },
      [](bool, const Settings&, bool) { /* UI state is not persisted */ });

   auto xScale = std::make_unique<InGameUIItem>(
      "Table X Scale"s, "Stretch the scene along the playfield width axis"s, 50.f, 150.f, 0.1f, 100.f * defViewSetup.mSceneScaleX / defViewSetup.GetRealToVirtualScale(table), "%4.1f %%"s,
      [this]()
      {
         const ViewSetup& viewSetup = GetCurrentViewSetup();
         const float realToVirtual = viewSetup.GetRealToVirtualScale(m_player->m_ptable);
         return 100.f * viewSetup.mSceneScaleX / realToVirtual;
      },
      [this](float prev, float v)
      {
         ViewSetup& viewSetup = GetCurrentViewSetup();
         const float realToVirtual = viewSetup.GetRealToVirtualScale(m_player->m_ptable);
         viewSetup.mSceneScaleX = v * realToVirtual / 100.f;
         if (m_lockScale && !IsResetting())
         {
            viewSetup.mSceneScaleY = clamp(viewSetup.mSceneScaleY + (v - prev) * realToVirtual / 100.f, 0.5f * realToVirtual, 1.5f * realToVirtual);
            viewSetup.mSceneScaleZ = clamp(viewSetup.mSceneScaleZ + (v - prev) * realToVirtual / 100.f, 0.5f * realToVirtual, 1.5f * realToVirtual);
         }
         OnPointOfViewChanged();
      },
      [keyPrefix](float v, Settings& settings, bool isTableOverride) { settings.SaveValue(Settings::TableOverride, keyPrefix + "ScaleX", v, isTableOverride); });
   xScale->SetInitialValue(100.f * m_initialViewSetup.mSceneScaleX / m_initialViewSetup.GetRealToVirtualScale(table));

   auto yScale = std::make_unique<InGameUIItem>(
      "Table Y Scale"s, "Stretch the scene along the playfield height axis"s, 50.f, 150.f, 0.1f, 100.f * defViewSetup.mSceneScaleY / defViewSetup.GetRealToVirtualScale(table), "%4.1f %%"s,
      [this]()
      {
         const ViewSetup& viewSetup = GetCurrentViewSetup();
         const float realToVirtual = viewSetup.GetRealToVirtualScale(m_player->m_ptable);
         return 100.f * viewSetup.mSceneScaleY / realToVirtual;
      },
      [this](float prev, float v)
      {
         ViewSetup& viewSetup = GetCurrentViewSetup();
         const float realToVirtual = viewSetup.GetRealToVirtualScale(m_player->m_ptable);
         viewSetup.mSceneScaleY = v * realToVirtual / 100.f;
         if (m_lockScale && !IsResetting())
         {
            viewSetup.mSceneScaleX = clamp(viewSetup.mSceneScaleX + (v - prev) * realToVirtual / 100.f, 0.5f * realToVirtual, 1.5f * realToVirtual);
            viewSetup.mSceneScaleZ = clamp(viewSetup.mSceneScaleZ + (v - prev) * realToVirtual / 100.f, 0.5f * realToVirtual, 1.5f * realToVirtual);
         }
         OnPointOfViewChanged();
      },
      [keyPrefix](float v, Settings& settings, bool isTableOverride) { settings.SaveValue(Settings::TableOverride, keyPrefix + "ScaleY", v, isTableOverride); });
   yScale->SetInitialValue(100.f * m_initialViewSetup.mSceneScaleY / m_initialViewSetup.GetRealToVirtualScale(table));

   auto zScale = std::make_unique<InGameUIItem>(
      "Table Z Scale"s, "Stretch the scene along the vertical axis (perpendicular to playfield)"s, 50.f, 150.f, 0.1f,
      100.f * defViewSetup.mSceneScaleZ / defViewSetup.GetRealToVirtualScale(table), "%4.1f %%"s,
      [this]()
      {
         const ViewSetup& viewSetup = GetCurrentViewSetup();
         const float realToVirtual = viewSetup.GetRealToVirtualScale(m_player->m_ptable);
         return 100.f * viewSetup.mSceneScaleZ / realToVirtual;
      },
      [this](float prev, float v)
      {
         ViewSetup& viewSetup = GetCurrentViewSetup();
         const float realToVirtual = viewSetup.GetRealToVirtualScale(m_player->m_ptable);
         viewSetup.mSceneScaleZ = v * realToVirtual / 100.f;
         if (m_lockScale && !IsResetting())
         {
            viewSetup.mSceneScaleX = clamp(viewSetup.mSceneScaleX + (v - prev) * realToVirtual / 100.f, 0.5f * realToVirtual, 1.5f * realToVirtual);
            viewSetup.mSceneScaleY = clamp(viewSetup.mSceneScaleY + (v - prev) * realToVirtual / 100.f, 0.5f * realToVirtual, 1.5f * realToVirtual);
         }
         OnPointOfViewChanged();
      },
      [keyPrefix](float v, Settings& settings, bool isTableOverride) { settings.SaveValue(Settings::TableOverride, keyPrefix + "ScaleZ", v, isTableOverride); });
   zScale->SetInitialValue(100.f * m_initialViewSetup.mSceneScaleZ / m_initialViewSetup.GetRealToVirtualScale(table));

   auto playerX = std::make_unique<InGameUIItem>(
      "Player X", "Player position in real world, expressed from the bottom center of the playfield, in centimeters"s, -30.f, 30.f, 0.1f, m_initialPlayerPos.x, "%4.1f cm"s,
      [this]() { return m_playerPos.x; },
      [this](float prev, float v)
      {
         PinTable* table = m_player->m_ptable;
         ViewSetup& viewSetup = GetCurrentViewSetup();
         m_playerPos.x = v;
         const float screenInclination = table->m_settings.LoadValueFloat(Settings::Player, "ScreenInclination"s);
         viewSetup.SetViewPosFromPlayerPosition(table, m_playerPos, screenInclination);
         OnPointOfViewChanged();
      },
      [keyPrefix](float v, Settings& settings, bool isTableOverride) { settings.SaveValue(Settings::Player, "ScreenPlayerX", v, isTableOverride); });
   playerX->SetInitialValue(m_initialPlayerPos.x);
   auto playerY = std::make_unique<InGameUIItem>(
      "Player Y", "Player position in real world, expressed from the bottom center of the playfield, in centimeters"s, -70.f, 30.f, 0.1f, m_initialPlayerPos.y, "%4.1f cm"s,
      [this]() { return m_playerPos.y; },
      [this](float prev, float v)
      {
         PinTable* table = m_player->m_ptable;
         ViewSetup& viewSetup = GetCurrentViewSetup();
         m_playerPos.y = v;
         const float screenInclination = table->m_settings.LoadValueFloat(Settings::Player, "ScreenInclination"s);
         viewSetup.SetViewPosFromPlayerPosition(table, m_playerPos, screenInclination);
         OnPointOfViewChanged();
      },
      [keyPrefix](float v, Settings& settings, bool isTableOverride) { settings.SaveValue(Settings::Player, "ScreenPlayerY", v, isTableOverride); });
   playerY->SetInitialValue(m_initialPlayerPos.y);
   auto playerZ = std::make_unique<InGameUIItem>(
      "Player Z", "Player position in real world, expressed from the bottom center of the playfield, in centimeters"s, 30.f, 100.f, 0.1f, m_initialPlayerPos.z, "%4.1f cm"s,
      [this]() { return m_playerPos.z; },
      [this](float prev, float v)
      {
         PinTable* table = m_player->m_ptable;
         ViewSetup& viewSetup = GetCurrentViewSetup();
         m_playerPos.z = v;
         const float screenInclination = table->m_settings.LoadValueFloat(Settings::Player, "ScreenInclination"s);
         viewSetup.SetViewPosFromPlayerPosition(table, m_playerPos, screenInclination);
         OnPointOfViewChanged();
      },
      [keyPrefix](float v, Settings& settings, bool isTableOverride) { settings.SaveValue(Settings::Player, "ScreenPlayerZ", v, isTableOverride); });
   playerZ->SetInitialValue(m_initialPlayerPos.z);

   auto viewX = std::make_unique<InGameUIItem>(
      isLegacy ? "X Offset" : "Camera X", "View point width offset, in centimeters"s, -30.f, 30.f, 0.1f, VPUTOCM(defViewSetup.mViewX), "%4.1f cm"s,
      [this]() { return VPUTOCM(GetCurrentViewSetup().mViewX); },
      [this](float prev, float v)
      {
         GetCurrentViewSetup().mViewX = CMTOVPU(v);
         OnPointOfViewChanged();
      },
      [keyPrefix](float v, Settings& settings, bool isTableOverride) { settings.SaveValue(Settings::TableOverride, keyPrefix + "PlayerX", CMTOVPU(v), isTableOverride); });
   viewX->SetInitialValue(VPUTOCM(m_initialViewSetup.mViewX));
   auto viewY = std::make_unique<InGameUIItem>(
      isLegacy ? "Y Offset" : "Camera Y", "View point height offset, in centimeters"s, -30.f, 100.f, 0.1f, VPUTOCM(defViewSetup.mViewY), "%4.1f cm"s,
      [this]() { return VPUTOCM(GetCurrentViewSetup().mViewY); },
      [this](float prev, float v)
      {
         GetCurrentViewSetup().mViewY = CMTOVPU(v);
         OnPointOfViewChanged();
      },
      [keyPrefix](float v, Settings& settings, bool isTableOverride) { settings.SaveValue(Settings::TableOverride, keyPrefix + "PlayerY", CMTOVPU(v), isTableOverride); });
   viewY->SetInitialValue(VPUTOCM(m_initialViewSetup.mViewY));
   auto viewZ = std::make_unique<InGameUIItem>(
      isLegacy ? "Z Offset" : "Camera Z", "View point vertical offset, in centimeters"s, 10.f, 100.f, 0.1f, VPUTOCM(defViewSetup.mViewZ), "%4.1f cm"s,
      [this]() { return VPUTOCM(GetCurrentViewSetup().mViewZ); },
      [this](float prev, float v)
      {
         GetCurrentViewSetup().mViewZ = CMTOVPU(v);
         OnPointOfViewChanged();
      },
      [keyPrefix](float v, Settings& settings, bool isTableOverride) { settings.SaveValue(Settings::TableOverride, keyPrefix + "PlayerZ", CMTOVPU(v), isTableOverride); });
   viewZ->SetInitialValue(VPUTOCM(m_initialViewSetup.mViewZ));

   auto hOfs = std::make_unique<InGameUIItem>(
      "Horizontal Offset", "Horizontal offset of the virtual table behind the screen 'window' in centimeters"s, -30.f, 30.f, 0.1f, defViewSetup.mViewHOfs, "%4.1f cm"s,
      [this]() { return GetCurrentViewSetup().mViewHOfs; },
      [this](float prev, float v)
      {
         GetCurrentViewSetup().mViewHOfs = v;
         OnPointOfViewChanged();
      },
      [keyPrefix](float v, Settings& settings, bool isTableOverride) { settings.SaveValue(Settings::TableOverride, keyPrefix + "HOfs", v, isTableOverride); });
   hOfs->SetInitialValue(m_initialViewSetup.mViewHOfs);
   auto vOfs = std::make_unique<InGameUIItem>(
      "Vertical Offset", "Vertical offset of the virtual table behind the screen 'window' in centimeters"s, -30.f, 30.f, 0.1f, defViewSetup.mViewVOfs, "%4.1f cm"s,
      [this]() { return GetCurrentViewSetup().mViewVOfs; },
      [this](float prev, float v)
      {
         GetCurrentViewSetup().mViewVOfs = v;
         OnPointOfViewChanged();
      },
      [keyPrefix](float v, Settings& settings, bool isTableOverride) { settings.SaveValue(Settings::TableOverride, keyPrefix + "VOfs", v, isTableOverride); });
   vOfs->SetInitialValue(m_initialViewSetup.mViewVOfs);

   auto wndTopZ = std::make_unique<InGameUIItem>(
      "Window Top Z Ofs.", "Distance between the 'window' (i.e. the screen) at the top of the playfield, in centimeters"s, 0.f, 50.f, 0.1f, VPUTOCM(defViewSetup.mWindowTopZOfs), "%4.1f cm"s,
      [this]() { return VPUTOCM(GetCurrentViewSetup().mWindowTopZOfs); },
      [this](float prev, float v)
      {
         GetCurrentViewSetup().mWindowTopZOfs = CMTOVPU(v);
         OnPointOfViewChanged();
      },
      [keyPrefix](float v, Settings& settings, bool isTableOverride) { settings.SaveValue(Settings::TableOverride, keyPrefix + "WindowTop", CMTOVPU(v), isTableOverride); });
   wndTopZ->SetInitialValue(VPUTOCM(m_initialViewSetup.mWindowTopZOfs));
   auto wndBotZ = std::make_unique<InGameUIItem>(
      "Window Bottom Z Ofs.", "Distance between the 'window' (i.e. the screen) at the bottom of the playfield, in centimeters"s, 0.f, 50.f, 0.1f, VPUTOCM(defViewSetup.mWindowBottomZOfs),
      "%4.1f cm"s, [this]() { return VPUTOCM(GetCurrentViewSetup().mWindowBottomZOfs); },
      [this](float prev, float v)
      {
         GetCurrentViewSetup().mWindowBottomZOfs = CMTOVPU(v);
         OnPointOfViewChanged();
      },
      [keyPrefix](float v, Settings& settings, bool isTableOverride) { settings.SaveValue(Settings::TableOverride, keyPrefix + "WindowBot", CMTOVPU(v), isTableOverride); });
   wndBotZ->SetInitialValue(VPUTOCM(m_initialViewSetup.mWindowBottomZOfs));

   AddItem(viewMode);
   switch (viewSetup.mMode)
   {
   case VLM_LEGACY:
      AddItem(inclination);
      AddItem(fov);
      AddItem(layback);
      AddItem(lockScale);
      AddItem(xScale);
      AddItem(yScale);
      AddItem(zScale);
      AddItem(viewX);
      AddItem(viewY);
      AddItem(viewZ);
      break;

   case VLM_CAMERA:
      AddItem(fov);
      AddItem(hOfs);
      AddItem(vOfs);
      AddItem(lockScale);
      AddItem(xScale);
      AddItem(yScale);
      AddItem(zScale);
      AddItem(lookAt);
      AddItem(viewX);
      AddItem(viewY);
      AddItem(viewZ);
      break;

   case VLM_WINDOW:
      AddItem(hOfs);
      AddItem(vOfs);
      AddItem(lockScale);
      AddItem(xScale);
      AddItem(yScale);
      AddItem(wndTopZ);
      AddItem(wndBotZ);
      AddItem(playerX);
      AddItem(playerY);
      AddItem(playerZ);
      break;
   }
}

};