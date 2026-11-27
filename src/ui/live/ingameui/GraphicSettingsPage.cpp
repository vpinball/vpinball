// license:GPLv3+

#include "core/stdafx.h"
#include "GraphicSettingsPage.h"

#include "renderer/Renderer.h"
#include "ui/live/LiveUI.h"

namespace VPX::InGameUI
{

GraphicSettingsPage::GraphicSettingsPage()
   : InGameUIPage("Graphics Settings"s, ""s, SaveMode::Both)
{
}

void GraphicSettingsPage::Open(bool isBackwardAnimation)
{
   InGameUIPage::Open(isBackwardAnimation);
   m_staticPrepassDisabled = false;
   // Pending values are initialized from the stored settings: they are edited through the standard
   // live/save flow but only applied on the next start as they impact device/buffer creation.
   const Settings& settings = GetSettings();
   m_pendingAAFactor = settings.GetPlayer_AAFactor();
   m_pendingMSAASamples = settings.GetPlayer_MSAASamples();
}

void GraphicSettingsPage::Close(bool isBackwardAnimation)
{
   InGameUIPage::Close(isBackwardAnimation);
   if (m_staticPrepassDisabled)
      m_player->m_renderer->DisableStaticPrePass(false);
}

void GraphicSettingsPage::OnStaticRenderDirty()
{
   if (!m_staticPrepassDisabled)
   {
      m_player->m_renderer->DisableStaticPrePass(true);
      m_staticPrepassDisabled = true;
   }
   m_player->m_renderer->InvalidateLayout();
}

void GraphicSettingsPage::BuildPage()
{
#if defined(ENABLE_DX9)
   constexpr bool isDX9 = true;
   constexpr bool isOpenGL = false;
   constexpr bool isBGFX = false;
#elif defined(ENABLE_OPENGL)
   constexpr bool isDX9 = false;
   constexpr bool isOpenGL = true;
   constexpr bool isBGFX = false;
#elif defined(ENABLE_BGFX)
   constexpr bool isDX9 = false;
   constexpr bool isOpenGL = false;
   constexpr bool isBGFX = true;
#endif

   //////////////////////////////////////////////////////////////////////////////////////////////////
   if (!m_player->IsVR())
   {
      AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "View mode"s));

      // The setting is not a ViewSetupID: 0 = Desktop (FSS if the table supports it), 1 = Cabinet, 2 = Desktop without FSS (see PinTable::UpdateCurrentBGSet)
      const auto toBGSet = [this](ViewSetupID mode) { return mode == BG_FULLSCREEN ? 1 : (mode == BG_DESKTOP && m_player->m_ptable->IsFSSEnabled()) ? 2 : 0; };
      const auto fromBGSet = [this](int v) { return v == 1 ? BG_FULLSCREEN : (v == 0 && m_player->m_ptable->IsFSSEnabled()) ? BG_FSS : BG_DESKTOP; };
      AddItem(std::make_unique<InGameUIItem>( //
         VPX::Properties::EnumPropertyDef(*Settings::GetPlayer_BGSet_Property(), toBGSet(m_player->m_ptable->GetViewMode())), //
         [this, toBGSet]() { return toBGSet(m_player->m_ptable->GetViewMode()); }, // Live
         [this](const Settings& settings) { return (int)settings.GetPlayer_BGSet(); }, // Stored
         [this, fromBGSet](int, int v)
         {
            m_player->m_ptable->SetViewSetupOverride(fromBGSet(v));
            m_player->SetCabinetAutoFitMode(g_settingsService.GetActiveSettings().GetPlayer_CabinetAutofitMode());
            m_player->SetCabinetAutoFitPos(g_settingsService.GetActiveSettings().GetPlayer_CabinetAutofitPos());
            OnStaticRenderDirty();
            RequestRebuild();
         },
         [](Settings& settings) { settings.ResetPlayer_BGSet(); }, //
         [this](int v, Settings& settings, bool asTableOverride)
         {
            settings.SetPlayer_BGSet(v, asTableOverride);
            m_player->m_ptable->SetViewSetupOverride(ViewSetupID::BG_INVALID);
         }));

      if (m_player->m_ptable->GetViewMode() == ViewSetupID::BG_FULLSCREEN)
      {
         AddItem(std::make_unique<InGameUIItem>(
            Settings::m_propPlayer_CabinetAutofitMode, //
            [this]() { return m_player->GetCabinetAutoFitMode(); }, // Live
            [this](int, int v)
            {
               m_player->SetCabinetAutoFitMode(v);
               if (v != 0)
                  OnStaticRenderDirty();
            }));
         if (m_player->GetCabinetAutoFitMode() == 1)
         {
            AddItem(std::make_unique<InGameUIItem>(
               Settings::m_propPlayer_CabinetAutofitPos, 100.f, "%4.1f %%"s, //
               [this]() { return m_player->GetCabinetAutoFitPos(); },
               [this](float, float v)
               {
                  m_player->SetCabinetAutoFitPos(v);
                  OnStaticRenderDirty();
               }));
         }
      }
   }


   //////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef ENABLE_BGFX
   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   #if defined(ENABLE_XR) && BX_PLATFORM_ANDROID
   if (m_player->IsVR())
   {
      AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Graphics backend"s));
      AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Info, "The graphics backend is selected by the VR runtime: " + string(bgfx::getRendererName(bgfx::getRendererType()))));
   }
   else
   #endif
   if (vector<string> renderers = RenderDevice::GetSelectableBackendNames(); !renderers.empty())
   {
      AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Graphics backend"s));
      // "Default" first: an unset or unknown GfxBackend resolves to index 0 below (max(0, -1)), so without
      // it the dialog would show the first backend as selected.
      renderers.insert(renderers.begin(), "Default"s);
      AddItem(std::make_unique<InGameUIItem>(
         VPX::Properties::EnumPropertyDef(""s, ""s, "Graphics Backend"s, ""s, false, 0, 0, renderers),
         [this, renderers]() { return max(0, FindIndexOf(renderers, g_settingsService.GetActiveSettings().GetPlayer_GfxBackend())); }, // Live
         [this, renderers](const Settings& settings) { return max(0, FindIndexOf(renderers, settings.GetPlayer_GfxBackend())); }, // Stored (same)
         [this, renderers](int, int v)
         {
            g_settingsService.GetActiveSettings().SetPlayer_GfxBackend(renderers[v], false);
            NotifySavedForNextStart();
         },
         [](Settings&) { /* Nothing to do as this is directly persisted for the time being */ },
         [](int, Settings&, bool) { /* Nothing to do as this is directly persisted for the time being */ }));
   }
#endif

   //////////////////////////////////////////////////////////////////////////////////////////////////

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Display synchronization"s));

   // Sync modes:
   // - Hardware VSync
   // - Adaptive Vsync (hardware or software): hardware VSync if not late
   // - Frame Pacing: produce frames at a pace that correspond to the GPU rendering to limit input and display latency (not blocking on GPU flip, not filling GPU frame in flight queue)
   // - Software VSync: synchronize to display FPS
   // - User selected FPS
   // Sync: None / VSync / Adaptive VSync / Frame Paced VSync
   // Limit FPS: None / Display / User
   constexpr bool adaptiveVSyncSupported = isDX9 || isOpenGL;
   AddItem(std::make_unique<InGameUIItem>(
      VPX::Properties::EnumPropertyDef(""s, ""s, "Display Synchronization"s,
         "Select how frame generation is synchronized to display refresh rate.\nHardware synchronization is recommended for smoother gameplay.\nVSync stands for 'Vertical Synchronization'."s,
         false, 0, 0,
         vector { "Hardware VSync"s,
#if defined(ENABLE_OPENGL)
            "Hardware Adaptive VSync"s,
#elif defined(ENABLE_DX9)
            "Software Adaptive VSync"s,
#endif
            "Frame Pacing"s, "Software VSync"s, "Custom FPS"s, "No synchronization"s }),
      [this]()
      {
         switch (m_player->GetVideoSyncMode())
         {
         case VideoSyncMode::VSM_VSYNC: return 0;
         case VideoSyncMode::VSM_ADAPTIVE_VSYNC: return 1;
         case VideoSyncMode::VSM_FRAME_PACING: return adaptiveVSyncSupported ? 2 : 1;
         case VideoSyncMode::VSM_NONE:
            if (m_player->GetTargetRefreshRate() == m_player->m_playfieldWnd->GetRefreshRate())
               return adaptiveVSyncSupported ? 3 : 2; // Main display refresh rate => Software VSync
            if (m_player->GetTargetRefreshRate() == 10000.f)
               return adaptiveVSyncSupported ? 5 : 4; // No synchronization
            return adaptiveVSyncSupported ? 4 : 3; // Custom FPS
         default: assert(false); return 0;
         }
      }, // Live
      [this](const Settings& settings)
      {
         switch (settings.GetPlayer_SyncMode())
         {
         case VideoSyncMode::VSM_VSYNC: return 0;
         case VideoSyncMode::VSM_ADAPTIVE_VSYNC: return 1;
         case VideoSyncMode::VSM_FRAME_PACING: return adaptiveVSyncSupported ? 2 : 1;
         case VideoSyncMode::VSM_NONE:
            if (settings.GetPlayer_MaxFramerate() == m_player->m_playfieldWnd->GetRefreshRate())
               return adaptiveVSyncSupported ? 3 : 2; // Main display refresh rate => Software VSync
            if (settings.GetPlayer_MaxFramerate() == 10000.f)
               return adaptiveVSyncSupported ? 5 : 4; // No synchronization
            return adaptiveVSyncSupported ? 4 : 3; // Custom FPS
         default: assert(false); return 0;
         }
      }, // Stored
      [this](int, int v)
      {
         if (!adaptiveVSyncSupported && v > 0)
            v += 1;
         switch (v)
         {
         case 0: // Hardware VSync
            m_player->SetVideoSyncMode(VideoSyncMode::VSM_VSYNC);
            m_player->SetTargetRefreshRate(10000.f);
            break;
         case 1: // Adaptive Sync
            m_player->SetVideoSyncMode(VideoSyncMode::VSM_ADAPTIVE_VSYNC);
            m_player->SetTargetRefreshRate(10000.f);
            break;
         case 2: // Frame Pacing
            m_player->SetVideoSyncMode(VideoSyncMode::VSM_FRAME_PACING);
            m_player->SetTargetRefreshRate(10000.f);
            break;
         case 3: // Software VSync
            m_player->SetVideoSyncMode(VideoSyncMode::VSM_NONE);
            m_player->SetTargetRefreshRate(m_player->m_playfieldWnd->GetRefreshRate());
            break;
         case 4: // Custom FPS
            m_player->SetVideoSyncMode(VideoSyncMode::VSM_NONE);
            m_player->SetTargetRefreshRate(roundf(2.f * m_player->m_playfieldWnd->GetRefreshRate()));
            break;
         case 5: // No synchronization
            m_player->SetVideoSyncMode(VideoSyncMode::VSM_NONE);
            m_player->SetTargetRefreshRate(10000.f);
            break;
         }
         RequestRebuild();
         if (isDX9 || isOpenGL)
            m_notificationId = m_player->m_liveUI->PushNotification("Note that some changes will only be applied after restarting the player."s, 3000, m_notificationId);
      },
      [](Settings& settings)
      {
         settings.ResetPlayer_SyncMode();
         settings.ResetPlayer_MaxFramerate();
      }, // Reset
      [this](int, Settings& settings, bool asTableOverride)
      {
         settings.SetPlayer_SyncMode(m_player->GetVideoSyncMode(), asTableOverride);
         settings.SetPlayer_MaxFramerate(m_player->GetTargetRefreshRate(), asTableOverride);
      })); // Save
   if (const float maxFPS = m_player->GetTargetRefreshRate();
      m_player->GetVideoSyncMode() == VideoSyncMode::VSM_NONE && maxFPS != m_player->m_playfieldWnd->GetRefreshRate() && maxFPS != 10000.f)
   {
      AddItem(std::make_unique<InGameUIItem>(
         VPX::Properties::FloatPropertyDef(""s, ""s, "Custom FPS"s,
            "Select a custom 'frame per second' rate.\nNote that this is not recommended and will result in a bad gameplay experience."s, false, 24.f, 1000.f, 1.f,
            roundf(m_player->m_playfieldWnd->GetRefreshRate() - 1.f)),
         1.f, "%4.1f", //
         [this]() { return m_player->GetTargetRefreshRate(); }, // Live
         [this](const Settings& settings) { return settings.GetPlayer_MaxFramerate(); }, // Stored
         [this](float, float v) { m_player->SetTargetRefreshRate(v == m_player->m_playfieldWnd->GetRefreshRate() ? v - 0.1f : v); }, // The player would interpret this as software VSync
         [](Settings&) { /* Nothing to do, as save is handled by the main combo */ }, [](float, Settings&, bool) { /* Nothing to do, as save is handled by the main combo */ }));
   }

#ifndef ENABLE_OPENGL
   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_MaxPrerenderedFrames, "%4d Frames"s, //
      [this]() { return g_settingsService.GetActiveSettings().GetPlayer_MaxPrerenderedFrames(); }, //
      [this](int, int v)
      {
         g_settingsService.GetActiveSettings().SetPlayer_MaxPrerenderedFrames(v, false);
         NotifySavedForNextStart();
      }));
#endif


   //////////////////////////////////////////////////////////////////////////////////////////////////

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Antialiasing quality"s));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_AAFactor, 100.f, "%4.1f %%"s, //
      [this]() { return m_pendingAAFactor; }, //
      [this](float, float v)
      {
         m_pendingAAFactor = v;
         NotifyApplyOnNextStart();
      }));

#if defined(ENABLE_OPENGL) || defined(ENABLE_BGFX)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_MSAASamples, //
      [this]() { return m_pendingMSAASamples; }, //
      [this](int, int v)
      {
         m_pendingMSAASamples = v;
         NotifyApplyOnNextStart();
      }));
#endif

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_FXAA, //
      [this]() { return m_player->m_renderer->m_FXAA; }, //
      [this](int, int v) { m_player->m_renderer->m_FXAA = v; }));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_Sharpen, //
      [this]() { return m_player->m_renderer->m_sharpen; }, //
      [this](int, int v) { m_player->m_renderer->m_sharpen = v; }));

   //////////////////////////////////////////////////////////////////////////////////////////////////

   if (m_player->m_ptable->m_enableSSR && m_player->m_ptable->m_SSRScale > 0.f && m_player->m_renderer->m_renderDevice->DepthBufferReadBackAvailable())
   {
      AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Miscellaneous features"s));
      AddItem(std::make_unique<InGameUIItem>( //
         Settings::m_propPlayer_SSRefl, //
         [this]() { return m_player->m_renderer->HasAdditiveScreenSpaceReflection(); }, //
         [this](bool v) { m_player->m_renderer->EnableAdditiveScreenSpaceReflection(v); }));
   }

   //////////////////////////////////////////////////////////////////////////////////////////////////

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Performance & Troubleshooting"s));

   AddItem(std::make_unique<InGameUIItem>(
      VPX::Properties::EnumPropertyDef(""s, ""s, "Ambient Occlusion"s,
         "Limit the quality of ambient occlusion for better performance.\r\nDynamic is the better with contact shadows for dynamic objects but higher performance requirements"s, false, 0, 0,
         vector { "Disabled"s, "Static"s, "Dynamic"s }),
      [this]() { return m_player->m_renderer->GetSelectedAOMode(); }, // Live
      [this](const Settings& settings)
      {
         if (settings.GetPlayer_DisableAO())
            return 0;
         if (settings.GetPlayer_DynamicAO())
            return 2;
         return 1;
      }, // Stored
      [this](int, int v) { m_player->m_renderer->SetAOMode(v); },
      [](Settings& settings)
      {
         settings.ResetPlayer_DisableAO();
         settings.ResetPlayer_DynamicAO();
      },
      [](int v, Settings& settings, bool asTableOverride)
      {
         settings.SetPlayer_DisableAO(v == 0, asTableOverride);
         settings.SetPlayer_DynamicAO(v == 2, asTableOverride);
      }));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   // Maybe setup a combo with a few preset values ?
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_MaxTexDimension, "%4d"s, //
      [this]() { return g_settingsService.GetActiveSettings().GetPlayer_MaxTexDimension(); }, //
      [this](int, int v)
      {
         g_settingsService.GetActiveSettings().SetPlayer_MaxTexDimension(v, false);
         NotifySavedForNextStart();
      }));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_PFReflection, //
      [this]() { return (int)m_player->m_renderer->GetPFReflectionMode(); }, //
      [this](int, int v) { m_player->m_renderer->SetPFReflectionMode((RenderProbe::ReflectionMode)v); }));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   // Maybe setup a combo with a few preset values ?
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_AlphaRampAccuracy, "%4d"s, //
      [this]() { return g_settingsService.GetActiveSettings().GetPlayer_AlphaRampAccuracy(); }, //
      [this](int, int v)
      {
         g_settingsService.GetActiveSettings().SetPlayer_AlphaRampAccuracy(v, false);
         NotifySavedForNextStart();
      }));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_HDRDisableToneMapper, //
      [this]() { return m_player->m_renderer->m_HDRforceDisableToneMapper; }, //
      [this](bool v) { m_player->m_renderer->m_HDRforceDisableToneMapper = v; }));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_HDRGlobalExposure, 1.f, "%4.2f"s, //
      [this]() { return g_settingsService.GetActiveSettings().GetPlayer_HDRGlobalExposure(); }, //
      [this](float, float v)
      {
         g_settingsService.GetActiveSettings().SetPlayer_HDRGlobalExposure(v, false);
         NotifySavedForNextStart();
      }));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
#if defined(ENABLE_BGFX)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_CompressTextures, //
      [this]() { return g_settingsService.GetActiveSettings().GetPlayer_CompressTextures(); }, //
      [this](bool v)
      {
         g_settingsService.GetActiveSettings().SetPlayer_CompressTextures(v, false);
         NotifySavedForNextStart();
      }));
#endif

#ifdef ENABLE_DX9
   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_UseNVidiaAPI, //
      [this]() { return g_settingsService.GetActiveSettings().GetPlayer_UseNVidiaAPI(); }, //
      [this](bool v)
      {
         g_settingsService.GetActiveSettings().SetPlayer_UseNVidiaAPI(v, false);
         NotifySavedForNextStart();
      }));
#endif

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_ForceAnisotropicFiltering, //
      [this]() { return m_player->m_renderer->UseAnisoFiltering(); }, //
      [this](bool v) { m_player->m_renderer->SetAnisoFiltering(v); }));

   // FIXME this conflicts with HDR forcing bloom off
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_ForceBloomOff, //
      [this]() { return m_player->m_renderer->m_bloomOff; }, //
      [this](bool v) { m_player->m_renderer->m_bloomOff = v; }));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_ForceMotionBlurOff, //
      [this]() { return m_player->m_renderer->m_motionBlurOff; }, //
      [this](bool v) { m_player->m_renderer->m_motionBlurOff = v; }));

#ifdef ENABLE_DX9
   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_SoftwareVertexProcessing, //
      [this]() { return g_settingsService.GetActiveSettings().GetPlayer_SoftwareVertexProcessing(); }, //
      [this](bool v)
      {
         g_settingsService.GetActiveSettings().SetPlayer_SoftwareVertexProcessing(v, false);
         NotifySavedForNextStart();
      }));
#endif

   //////////////////////////////////////////////////////////////////////////////////////////////////

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Ball Rendering"s));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_BallAntiStretch, //
      [this]() { return m_player->m_renderer->m_ballAntiStretch; }, //
      [this](bool v) { m_player->m_renderer->m_ballAntiStretch = v; }));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_DisableLightingForBalls, //
      [this]() { return m_player->m_renderer->IsBallLightingDisabled(); }, //
      [this](bool v) { m_player->m_renderer->DisableBallLighting(v); }));

   // Applied when a table is loaded (PinTable::ApplyLoadedVersionFixups), so saved for the next start
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_LegacyTableBallShadows, //
      []() { return g_settingsService.GetActiveSettings().GetPlayer_LegacyTableBallShadows(); }, //
      [this](bool v)
      {
         g_settingsService.GetActiveSettings().SetPlayer_LegacyTableBallShadows(v, false);
         NotifySavedForNextStart();
      }));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_BallTrail, //
      [this]() { return m_player->m_renderer->m_trailForBalls; }, //
      [this](bool v) { m_player->m_renderer->m_trailForBalls = v; }));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_BallTrailStrength, 1.f, "%4.2f"s, //
      [this]() { return m_player->m_renderer->m_ballTrailStrength; }, //
      [this](float, float v) { m_player->m_renderer->m_ballTrailStrength = v; }));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_OverwriteBallImage, //
      [this]() { return m_player->m_renderer->m_overwriteBallImages; }, //
      [this](bool v)
      {
         // Keeps the images in use (possibly edited, not saved yet); the balls follow live
         Renderer *const renderer = m_player->m_renderer.get();
         renderer->SetBallImageOverride(v, renderer->m_ballImagePath, renderer->m_decalImagePath);
         RequestRebuild();
      }));

   if (m_player->m_renderer->m_overwriteBallImages)
   {
      // The items show and edit the images the renderer uses (the live value of the live/save flow)
      AddItem(std::make_unique<InGameUIItem>(
         Settings::m_propPlayer_BallImage, //
         [this]() { return m_player->m_renderer->m_ballImagePath; }, //
         [this](const string&, const string& v)
         {
            Renderer *const renderer = m_player->m_renderer.get();
            renderer->SetBallImageOverride(renderer->m_overwriteBallImages, v, renderer->m_decalImagePath);
         }));

      AddItem(std::make_unique<InGameUIItem>(
         Settings::m_propPlayer_DecalImage, //
         [this]() { return m_player->m_renderer->m_decalImagePath; }, //
         [this](const string&, const string& v)
         {
            Renderer *const renderer = m_player->m_renderer.get();
            renderer->SetBallImageOverride(renderer->m_overwriteBallImages, renderer->m_ballImagePath, v);
         }));
   }
}
}
