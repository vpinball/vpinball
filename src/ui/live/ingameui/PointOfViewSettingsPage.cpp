// license:GPLv3+

#include "core/stdafx.h"

#include "PointOfViewSettingsPage.h"
#include "core/TableDB.h"

#include <regex>

namespace VPX::InGameUI
{

PointOfViewSettingsPage::PointOfViewSettingsPage()
   : InGameUIPage("Point of View Table Override"s, "Point of view's settings page:\nOptions to override the table's rendering's point of view"s, SaveMode::Table)
{
}

void PointOfViewSettingsPage::Open(bool isBackwardAnimation)
{
   assert(!m_opened);
   InGameUIPage::Open(isBackwardAnimation);
   m_opened = true;
   m_staticPrepassDisabled = false;
   const PinTable* const table = m_player->m_ptable;
   ViewSetupID vsId = table->GetViewMode();
   m_initialViewSetup = table->GetViewSetup();
   const Settings& settings = GetSettings();
   m_playerPos.x = settings.GetPlayer_ScreenPlayerX();
   m_playerPos.y = settings.GetPlayer_ScreenPlayerY();
   m_playerPos.z = settings.GetPlayer_ScreenPlayerZ();
   UpdateDefaults();
   BuildPage();
}

void PointOfViewSettingsPage::Close(bool isBackwardAnimation)
{
   assert(m_opened);
   InGameUIPage::Close(isBackwardAnimation);
   m_opened = false;
   if (m_staticPrepassDisabled)
      m_player->m_renderer->DisableStaticPrePass(false);
}

void PointOfViewSettingsPage::Save()
{
   InGameUIPage::Save();

   // FIXME this should be part of the action, not of the ingameui
   if (g_app->m_povEdit)
      g_pvp->QuitPlayer(Player::CloseState::CS_CLOSE_APP);
}

void PointOfViewSettingsPage::ResetToStoredValues()
{
   InGameUIPage::ResetToStoredValues();

   UpdateDefaults();
   BuildPage();

   // FIXME this should be part of the action, not of the ingameui
   if (g_app->m_povEdit)
      g_pvp->QuitPlayer(Player::CloseState::CS_CLOSE_APP);
}

void PointOfViewSettingsPage::ResetToDefaults()
{
   InGameUIPage::ResetToDefaults();
   ViewSetup& viewSetup = GetCurrentViewSetup();
   if (viewSetup.mMode == VLM_WINDOW)
   {
      const PinTable* table = m_player->m_ptable;
      const float screenInclination = table->m_settings.GetPlayer_ScreenInclination();
      viewSetup.SetViewPosFromPlayerPosition(table, m_playerPos, screenInclination);
   }
   OnPointOfViewChanged();
   UpdateDefaults();
   BuildPage();
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

VPX::Properties::PropertyRegistry::PropId PointOfViewSettingsPage::SelectProp(
   VPX::Properties::PropertyRegistry::PropId dt, VPX::Properties::PropertyRegistry::PropId fss, VPX::Properties::PropertyRegistry::PropId cab) const
{
   switch (m_player->m_ptable->GetViewMode())
   {
   case BG_DESKTOP: return dt;
   case BG_FSS: return fss;
   case BG_FULLSCREEN: return cab;
   default: assert(false); return dt;
   }
}

void PointOfViewSettingsPage::UpdateDefaults()
{
   ViewSetup defViewSetup;
   if (m_player->m_ptable->GetViewMode() == BG_FULLSCREEN)
   { // Cabinet mode
      const Settings& settings = GetSettings();
      const float screenWidth = settings.GetPlayer_ScreenWidth();
      const float screenHeight = settings.GetPlayer_ScreenHeight();
      if (screenWidth <= 1.f || screenHeight <= 1.f)
      {
         AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Info, "You must setup your screen size before using Window mode"s));
         AddItem(std::make_unique<InGameUIItem>("Cabinet Settings"s, ""s, "settings/cabinet"s));
         return;
      }
      const bool isFitted = (m_player->m_ptable->GetViewSetup().mViewHOfs == 0.f) && (m_player->m_ptable->GetViewSetup().mSceneScaleY == m_player->m_ptable->GetViewSetup().mSceneScaleX);
      defViewSetup.SetWindowAutofit(m_player->m_ptable, m_playerPos, m_player->m_renderer->GetDisplayAspectRatio(), 
         m_player->GetCabinetAutoFitPos(), isFitted,
         [this](string info) { m_glassNotifId = m_player->m_liveUI->PushNotification(info, 10000, m_glassNotifId); });
   }
   else if (const bool portrait = m_player->m_playfieldWnd->GetWidth() < m_player->m_playfieldWnd->GetHeight(); m_player->m_ptable->GetViewMode() == BG_DESKTOP && !portrait)
   { // Desktop
      const Settings& settings = GetSettings();
      defViewSetup.mMode = (ViewLayoutMode)settings.GetDefaultCamera_DesktopMode();
      defViewSetup.mViewX = CMTOVPU(settings.GetDefaultCamera_DesktopCamX());
      defViewSetup.mViewY = CMTOVPU(settings.GetDefaultCamera_DesktopCamY());
      defViewSetup.mViewZ = CMTOVPU(settings.GetDefaultCamera_DesktopCamZ());
      defViewSetup.mSceneScaleX = settings.GetDefaultCamera_DesktopScaleX();
      defViewSetup.mSceneScaleY = settings.GetDefaultCamera_DesktopScaleY();
      defViewSetup.mSceneScaleZ = settings.GetDefaultCamera_DesktopScaleZ();
      defViewSetup.mFOV = settings.GetDefaultCamera_DesktopFov();
      defViewSetup.mLookAt = settings.GetDefaultCamera_DesktopLookAt();
      defViewSetup.mViewVOfs = settings.GetDefaultCamera_DesktopViewVOfs();
   }
   else if (m_player->m_ptable->GetViewMode() == BG_DESKTOP || m_player->m_ptable->GetViewMode() == BG_FSS)
   { // FSS
      const Settings& settings = GetSettings();
      defViewSetup.mMode = (ViewLayoutMode)settings.GetDefaultCamera_FSSMode();
      defViewSetup.mViewX = CMTOVPU(settings.GetDefaultCamera_FSSCamX());
      defViewSetup.mViewY = CMTOVPU(settings.GetDefaultCamera_FSSCamY());
      defViewSetup.mViewZ = CMTOVPU(settings.GetDefaultCamera_FSSCamZ());
      defViewSetup.mSceneScaleX = settings.GetDefaultCamera_FSSScaleX();
      defViewSetup.mSceneScaleY = settings.GetDefaultCamera_FSSScaleY();
      defViewSetup.mSceneScaleZ = settings.GetDefaultCamera_FSSScaleZ();
      defViewSetup.mFOV = settings.GetDefaultCamera_FSSFov();
      defViewSetup.mLookAt = settings.GetDefaultCamera_FSSLookAt();
      defViewSetup.mViewVOfs = settings.GetDefaultCamera_FSSViewVOfs();
   }

   // Update defaults
   auto setEnumDef = [this](VPX::Properties::PropertyRegistry::PropId dt, VPX::Properties::PropertyRegistry::PropId fss, VPX::Properties::PropertyRegistry::PropId cab, int def)
   { Settings::GetRegistry().Register(Settings::GetRegistry().GetEnumProperty(SelectProp(dt, fss, cab))->WithDefault(def)); };
   auto setFloatDef = [this](VPX::Properties::PropertyRegistry::PropId dt, VPX::Properties::PropertyRegistry::PropId fss, VPX::Properties::PropertyRegistry::PropId cab, float def)
   { Settings::GetRegistry().Register(Settings::GetRegistry().GetFloatProperty(SelectProp(dt, fss, cab))->WithDefault(def)); };
   setEnumDef(Settings::m_propTableOverride_ViewDTMode, Settings::m_propTableOverride_ViewFSSMode, Settings::m_propTableOverride_ViewCabMode, defViewSetup.mMode);
   setFloatDef(Settings::m_propTableOverride_ViewDTLookAt, Settings::m_propTableOverride_ViewFSSLookAt, Settings::m_propTableOverride_ViewCabLookAt, defViewSetup.mLookAt);
   setFloatDef(Settings::m_propTableOverride_ViewDTFOV, Settings::m_propTableOverride_ViewFSSFOV, Settings::m_propTableOverride_ViewCabFOV, defViewSetup.mFOV);
   setFloatDef(Settings::m_propTableOverride_ViewDTLayback, Settings::m_propTableOverride_ViewFSSLayback, Settings::m_propTableOverride_ViewCabLayback, defViewSetup.mLayback);
   setFloatDef(Settings::m_propTableOverride_ViewDTScaleX, Settings::m_propTableOverride_ViewFSSScaleX, Settings::m_propTableOverride_ViewCabScaleX, defViewSetup.mSceneScaleX);
   setFloatDef(Settings::m_propTableOverride_ViewDTScaleY, Settings::m_propTableOverride_ViewFSSScaleY, Settings::m_propTableOverride_ViewCabScaleY, defViewSetup.mSceneScaleY);
   setFloatDef(Settings::m_propTableOverride_ViewDTScaleZ, Settings::m_propTableOverride_ViewFSSScaleZ, Settings::m_propTableOverride_ViewCabScaleZ, defViewSetup.mSceneScaleZ);
   setFloatDef(Settings::m_propTableOverride_ViewDTPlayerX, Settings::m_propTableOverride_ViewFSSPlayerX, Settings::m_propTableOverride_ViewCabPlayerX, defViewSetup.mViewX);
   setFloatDef(Settings::m_propTableOverride_ViewDTPlayerY, Settings::m_propTableOverride_ViewFSSPlayerY, Settings::m_propTableOverride_ViewCabPlayerY, defViewSetup.mViewY);
   setFloatDef(Settings::m_propTableOverride_ViewDTPlayerZ, Settings::m_propTableOverride_ViewFSSPlayerZ, Settings::m_propTableOverride_ViewCabPlayerZ, defViewSetup.mViewZ);
   setFloatDef(Settings::m_propTableOverride_ViewDTHOfs, Settings::m_propTableOverride_ViewFSSHOfs, Settings::m_propTableOverride_ViewCabHOfs, defViewSetup.mViewHOfs);
   setFloatDef(Settings::m_propTableOverride_ViewDTVOfs, Settings::m_propTableOverride_ViewFSSVOfs, Settings::m_propTableOverride_ViewCabVOfs, defViewSetup.mViewVOfs);
   setFloatDef(Settings::m_propTableOverride_ViewDTWindowBot, Settings::m_propTableOverride_ViewFSSWindowBot, Settings::m_propTableOverride_ViewCabWindowBot, defViewSetup.mWindowBottomZOfs);
   setFloatDef(Settings::m_propTableOverride_ViewDTWindowTop, Settings::m_propTableOverride_ViewFSSWindowTop, Settings::m_propTableOverride_ViewCabWindowTop, defViewSetup.mWindowTopZOfs);
   setFloatDef(Settings::m_propTableOverride_ViewDTRotation, Settings::m_propTableOverride_ViewFSSRotation, Settings::m_propTableOverride_ViewCabRotation, defViewSetup.mViewportRotation);
}

void PointOfViewSettingsPage::BuildPage()
{
   assert(m_opened);
   const PinTable* table = m_player->m_ptable;

   ClearItems();

   ViewSetupID vsId = table->GetViewMode();
   const ViewSetup& viewSetup = table->GetViewSetup();
   //const bool isLegacy = viewSetup.mMode == VLM_LEGACY;
   const string keyPrefix = vsId == BG_DESKTOP ? "ViewDT"s : vsId == BG_FSS ? "ViewFSS"s : "ViewCab"s;

   auto viewMode = std::make_unique<InGameUIItem>(
      SelectProp(Settings::m_propTableOverride_ViewDTMode, Settings::m_propTableOverride_ViewFSSMode, Settings::m_propTableOverride_ViewCabMode),
      [this]() { return static_cast<int>(GetCurrentViewSetup().mMode); },
      [this](int, int v)
      {
         GetCurrentViewSetup().mMode = static_cast<ViewLayoutMode>(v);
         OnPointOfViewChanged();
         UpdateDefaults();
         BuildPage();
      });

   auto lookAt = std::make_unique<InGameUIItem>(
      SelectProp(Settings::m_propTableOverride_ViewDTLookAt, Settings::m_propTableOverride_ViewFSSLookAt, Settings::m_propTableOverride_ViewCabLookAt), 1.f, "%4.1f %%"s,
      [this]() { return GetCurrentViewSetup().mLookAt; },
      [this](float, float v)
      {
         GetCurrentViewSetup().mLookAt = v;
         OnPointOfViewChanged();
      });

   auto fov = std::make_unique<InGameUIItem>(
      SelectProp(Settings::m_propTableOverride_ViewDTFOV, Settings::m_propTableOverride_ViewFSSFOV, Settings::m_propTableOverride_ViewCabFOV), 1.f, "%4.1f deg"s, //
      [this]() { return GetCurrentViewSetup().mFOV; }, //
      [this](float, float v)
      {
         GetCurrentViewSetup().mFOV = v;
         OnPointOfViewChanged();
         BuildPage();
      });

   auto layback = std::make_unique<InGameUIItem>(
      SelectProp(Settings::m_propTableOverride_ViewDTLayback, Settings::m_propTableOverride_ViewFSSLayback, Settings::m_propTableOverride_ViewCabLayback), 1.f, "%4.1f  "s,
      [this]() { return GetCurrentViewSetup().mLayback; },
      [this](float, float v)
      {
         GetCurrentViewSetup().mLayback = v;
         OnPointOfViewChanged();
         BuildPage();
      });

   auto lockScale = std::make_unique<InGameUIItem>(
      VPX::Properties::BoolPropertyDef(""s, ""s, "Lock XYZ Scale"s, "Scale all axis homogeneously (recommended)"s, false, true), //
      [this]() { return m_lockScale; }, //
      [this](Settings&) { return m_lockScale; }, //
      [this](bool v) { m_lockScale = v; }, [](Settings&) { /* UI state, not persisted */ }, //
      [](bool, const Settings&, bool) { /* UI state, not persisted */ });

   const float zoomDisplayScale = 100.f / viewSetup.GetRealToVirtualScale(table);
   auto xScale = std::make_unique<InGameUIItem>(
      SelectProp(Settings::m_propTableOverride_ViewDTScaleX, Settings::m_propTableOverride_ViewFSSScaleX, Settings::m_propTableOverride_ViewCabScaleX), zoomDisplayScale, "%4.1f %%"s,
      [this]() { return GetCurrentViewSetup().mSceneScaleX; },
      [this](float prev, float v)
      {
         ViewSetup& vs = GetCurrentViewSetup();
         vs.mSceneScaleX = v;
         if (m_lockScale && !IsResetting())
         {
            vs.mSceneScaleY = clamp(vs.mSceneScaleY + (v - prev), 0.5f, 1.5f);
            vs.mSceneScaleZ = clamp(vs.mSceneScaleZ + (v - prev), 0.5f, 1.5f);
         }
         OnPointOfViewChanged();
         BuildPage();
      });
   auto yScale = std::make_unique<InGameUIItem>(
      SelectProp(Settings::m_propTableOverride_ViewDTScaleY, Settings::m_propTableOverride_ViewFSSScaleY, Settings::m_propTableOverride_ViewCabScaleY), zoomDisplayScale, "%4.1f %%"s,
      [this]() { return GetCurrentViewSetup().mSceneScaleY; },
      [this](float prev, float v)
      {
         ViewSetup& vs = GetCurrentViewSetup();
         vs.mSceneScaleY = v;
         if (m_lockScale && !IsResetting())
         {
            vs.mSceneScaleX = clamp(vs.mSceneScaleX + (v - prev), 0.5f, 1.5f);
            vs.mSceneScaleZ = clamp(vs.mSceneScaleZ + (v - prev), 0.5f, 1.5f);
         }
         OnPointOfViewChanged();
         BuildPage();
      });
   auto zScale = std::make_unique<InGameUIItem>(
      SelectProp(Settings::m_propTableOverride_ViewDTScaleZ, Settings::m_propTableOverride_ViewFSSScaleZ, Settings::m_propTableOverride_ViewCabScaleZ), zoomDisplayScale, "%4.1f %%"s,
      [this]() { return GetCurrentViewSetup().mSceneScaleZ; },
      [this](float prev, float v)
      {
         ViewSetup& vs = GetCurrentViewSetup();
         vs.mSceneScaleZ = v;
         if (m_lockScale && !IsResetting())
         {
            vs.mSceneScaleX = clamp(vs.mSceneScaleX + (v - prev), 0.5f, 1.5f);
            vs.mSceneScaleY = clamp(vs.mSceneScaleY + (v - prev), 0.5f, 1.5f);
         }
         OnPointOfViewChanged();
         BuildPage();
      });

   Settings::SetPlayer_ScreenPlayerX_Default(m_player->m_ptable->m_settings.GetPlayer_ScreenPlayerX());
   auto playerX = std::make_unique<InGameUIItem>(
      Settings::m_propPlayer_ScreenPlayerX, 1.f, "%4.1f cm"s, //
      [this]() { return m_playerPos.x; },
      [this](float, float v)
      {
         m_playerPos.x = v;
         const float screenInclination = m_player->m_ptable->m_settings.GetPlayer_ScreenInclination();
         GetCurrentViewSetup().SetViewPosFromPlayerPosition(m_player->m_ptable, m_playerPos, screenInclination);
         OnPointOfViewChanged();
         BuildPage();
      });

   Settings::SetPlayer_ScreenPlayerY_Default(m_player->m_ptable->m_settings.GetPlayer_ScreenPlayerY());
   auto playerY = std::make_unique<InGameUIItem>(
      Settings::m_propPlayer_ScreenPlayerY, 1.f, "%4.1f cm"s, //
      [this]() { return m_playerPos.y; },
      [this](float, float v)
      {
         m_playerPos.y = v;
         const float screenInclination = m_player->m_ptable->m_settings.GetPlayer_ScreenInclination();
         GetCurrentViewSetup().SetViewPosFromPlayerPosition(m_player->m_ptable, m_playerPos, screenInclination);
         OnPointOfViewChanged();
         BuildPage();
      });

   Settings::SetPlayer_ScreenPlayerZ_Default(m_player->m_ptable->m_settings.GetPlayer_ScreenPlayerZ());
   auto playerZ = std::make_unique<InGameUIItem>(
      Settings::m_propPlayer_ScreenPlayerZ, 1.f, "%4.1f cm"s, //
      [this]() { return m_playerPos.z; },
      [this](float, float v)
      {
         m_playerPos.z = v;
         const float screenInclination = m_player->m_ptable->m_settings.GetPlayer_ScreenInclination();
         GetCurrentViewSetup().SetViewPosFromPlayerPosition(m_player->m_ptable, m_playerPos, screenInclination);
         OnPointOfViewChanged();
         BuildPage();
      });

   auto viewX = std::make_unique<InGameUIItem>(
      SelectProp(Settings::m_propTableOverride_ViewDTPlayerX, Settings::m_propTableOverride_ViewFSSPlayerX, Settings::m_propTableOverride_ViewCabPlayerX), VPUTOCM(1.f), "%4.1f cm"s, //
      [this]() { return GetCurrentViewSetup().mViewX; },
      [this](float, float v)
      {
         GetCurrentViewSetup().mViewX = v;
         OnPointOfViewChanged();
         BuildPage();
      });

   auto viewY = std::make_unique<InGameUIItem>(
      SelectProp(Settings::m_propTableOverride_ViewDTPlayerY, Settings::m_propTableOverride_ViewFSSPlayerY, Settings::m_propTableOverride_ViewCabPlayerY), VPUTOCM(1.f), "%4.1f cm"s, //
      [this]() { return GetCurrentViewSetup().mViewY; },
      [this](float, float v)
      {
         GetCurrentViewSetup().mViewY = v;
         OnPointOfViewChanged();
         BuildPage();
      });

   auto viewZ = std::make_unique<InGameUIItem>(
      SelectProp(Settings::m_propTableOverride_ViewDTPlayerZ, Settings::m_propTableOverride_ViewFSSPlayerZ, Settings::m_propTableOverride_ViewCabPlayerZ), VPUTOCM(1.f), "%4.1f cm"s, //
      [this]() { return GetCurrentViewSetup().mViewZ; },
      [this](float, float v)
      {
         GetCurrentViewSetup().mViewZ = v;
         OnPointOfViewChanged();
         BuildPage();
      });

   auto hOfs = std::make_unique<InGameUIItem>(
      SelectProp(Settings::m_propTableOverride_ViewDTHOfs, Settings::m_propTableOverride_ViewFSSHOfs, Settings::m_propTableOverride_ViewCabHOfs), 1.f, "%4.1f cm"s, //
      [this]() { return GetCurrentViewSetup().mViewHOfs; },
      [this](float, float v)
      {
         GetCurrentViewSetup().mViewHOfs = v;
         OnPointOfViewChanged();
         BuildPage();
      });

   auto vOfs = std::make_unique<InGameUIItem>(
      SelectProp(Settings::m_propTableOverride_ViewDTVOfs, Settings::m_propTableOverride_ViewFSSVOfs, Settings::m_propTableOverride_ViewCabVOfs), 1.f, "%4.1f cm"s, //
      [this]() { return GetCurrentViewSetup().mViewVOfs; },
      [this](float, float v)
      {
         GetCurrentViewSetup().mViewVOfs = v;
         OnPointOfViewChanged();
         BuildPage();
      });

   auto wndTopZ = std::make_unique<InGameUIItem>(
      SelectProp(Settings::m_propTableOverride_ViewDTWindowTop, Settings::m_propTableOverride_ViewFSSWindowTop, Settings::m_propTableOverride_ViewCabWindowTop), VPUTOCM(1.f), "%4.1f cm"s, //
      [this]() { return GetCurrentViewSetup().mWindowTopZOfs; },
      [this](float, float v)
      {
         GetCurrentViewSetup().mWindowTopZOfs = v;
         OnPointOfViewChanged();
         BuildPage(); // As it changes the real to virtual world scale
      });

   auto wndBotZ = std::make_unique<InGameUIItem>(
      SelectProp(Settings::m_propTableOverride_ViewDTWindowBot, Settings::m_propTableOverride_ViewFSSWindowBot, Settings::m_propTableOverride_ViewCabWindowBot), VPUTOCM(1.f), "%4.1f cm"s, //
      [this]() { return GetCurrentViewSetup().mWindowBottomZOfs; },
      [this](float, float v)
      {
         GetCurrentViewSetup().mWindowBottomZOfs = v;
         OnPointOfViewChanged();
         BuildPage(); // As it changes the real to virtual world scale
      });

   auto vpRotation = std::make_unique<InGameUIItem>(
      SelectProp(Settings::m_propTableOverride_ViewDTRotation, Settings::m_propTableOverride_ViewFSSRotation, Settings::m_propTableOverride_ViewCabRotation), 1.f, "%3.0f deg"s, //
      [this]() { return GetCurrentViewSetup().mViewportRotation; }, //
      [this](float, float v)
      {
         GetCurrentViewSetup().mViewportRotation = v;
         OnPointOfViewChanged();
         BuildPage();
      });

   AddItem(std::move(viewMode));
   switch (viewSetup.mMode)
   {
   case VLM_LEGACY:
      AddItem(std::move(lookAt)); // Actually inclination
      AddItem(std::move(fov));
      AddItem(std::move(layback));
      AddItem(std::move(lockScale));
      AddItem(std::move(xScale));
      AddItem(std::move(yScale));
      AddItem(std::move(zScale));
      AddItem(std::move(viewX));
      AddItem(std::move(viewY));
      AddItem(std::move(viewZ));
      AddItem(std::move(vpRotation));
      break;

   case VLM_CAMERA:
      AddItem(std::move(fov));
      AddItem(std::move(hOfs));
      AddItem(std::move(vOfs));
      AddItem(std::move(lockScale));
      AddItem(std::move(xScale));
      AddItem(std::move(yScale));
      AddItem(std::move(zScale));
      AddItem(std::move(lookAt));
      AddItem(std::move(viewX));
      AddItem(std::move(viewY));
      AddItem(std::move(viewZ));
      AddItem(std::move(vpRotation));
      break;

   case VLM_WINDOW:
      AddItem(std::make_unique<InGameUIItem>(
         Settings::m_propPlayer_CabinetAutofitMode, //
         [this]() { return m_player->GetCabinetAutoFitMode(); }, // Live
         [this](int, int v)
         {
            m_player->SetCabinetAutoFitMode(v);
            if (v != 0)
               OnPointOfViewChanged();
            BuildPage();
         }));
      if (m_player->GetCabinetAutoFitMode() == 0)
      {
         AddItem(std::move(hOfs));
         AddItem(std::move(vOfs));
         AddItem(std::move(lockScale));
         AddItem(std::move(xScale));
         AddItem(std::move(yScale));
         AddItem(std::move(wndTopZ));
         AddItem(std::move(wndBotZ));
         AddItem(std::move(playerX));
         AddItem(std::move(playerY));
         AddItem(std::move(playerZ));
         AddItem(std::move(vpRotation));
      }
      else if (m_player->GetCabinetAutoFitMode() == 1)
      {
         AddItem(std::make_unique<InGameUIItem>(
            Settings::m_propPlayer_CabinetAutofitPos, 100.f, "%4.1f %%"s, //
            [this]() { return m_player->GetCabinetAutoFitPos(); },
            [this](float, float v)
            {
               m_player->SetCabinetAutoFitPos(v);
               OnPointOfViewChanged();
            }));
      }
      break;
   }
}

}
