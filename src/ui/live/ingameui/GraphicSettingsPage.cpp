// license:GPLv3+

#include "core/stdafx.h"

#include "GraphicSettingsPage.h"

namespace VPX::InGameUI
{

GraphicSettingsPage::GraphicSettingsPage()
   : InGameUIPage("Graphic Settings"s, ""s, SaveMode::Both)
{
   BuildPage();
}

void GraphicSettingsPage::OnStaticRenderDirty()
{
   if (!m_staticPrepassDisabled)
   {
      m_player->m_renderer->DisableStaticPrePass(true);
      m_staticPrepassDisabled = true;
   }
   m_player->m_renderer->InitLayout();
}

void GraphicSettingsPage::BuildPage()
{
   ClearItems();

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

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "View mode"s));

   AddItem(std::make_unique<InGameUIItem>( //
      VPX::Properties::EnumPropertyDef(*Settings::GetPlayer_BGSet_Property(), m_player->m_ptable->GetViewMode()), //
      [this]() { return (int)m_player->m_ptable->GetViewMode(); }, // Live
      [this](Settings& settings) { return (int)settings.GetPlayer_BGSet(); }, // Stored
      [this](int, int v)
      {
         m_player->m_ptable->SetViewSetupOverride((ViewSetupID)v);
         OnStaticRenderDirty();
         BuildPage();
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


   //////////////////////////////////////////////////////////////////////////////////////////////////

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Graphics backend"s));

#ifdef ENABLE_BGFX
   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   bgfx::RendererType::Enum supportedRenderers[bgfx::RendererType::Count];
   if (const int nRendererSupported = bgfx::getSupportedRenderers(bgfx::RendererType::Count, supportedRenderers); nRendererSupported > 1)
   {
      static const string bgfxRendererNames[bgfx::RendererType::Count + 1]
         = { "Noop"s, "Agc"s, "Direct3D11"s, "Direct3D12"s, "Gnm"s, "Metal"s, "Nvn"s, "OpenGLES"s, "OpenGL"s, "Vulkan"s, "Default"s };
      vector<string> renderers;
      for (int i = 0; i < nRendererSupported; i++)
         if (supportedRenderers[i] != bgfx::RendererType::Noop)
#ifdef _DEBUG
            if (supportedRenderers[i] != bgfx::RendererType::Direct3D12)
#endif
               renderers.push_back(bgfxRendererNames[supportedRenderers[i]]);
      AddItem(std::make_unique<InGameUIItem>(
         VPX::Properties::EnumPropertyDef(""s, ""s, "Graphics Backend"s, ""s, false, 0, 0, renderers),
         [this, renderers]() { return max(0, FindIndexOf(renderers, m_player->m_ptable->m_settings.GetPlayer_GfxBackend())); }, // Live
         [this, renderers](Settings& settings) { return max(0, FindIndexOf(renderers, settings.GetPlayer_GfxBackend())); }, // Stored (same)
         [this, renderers](int, int v)
         {
            m_player->m_ptable->m_settings.SetPlayer_GfxBackend(renderers[v], false);
            m_notificationId = m_player->m_liveUI->PushNotification("This change will be applied after restarting the player."s, 3000, m_notificationId);
         },
         [](Settings&) { /* Nothing to do as this is directly persisted for the time being */ },
         [](int, Settings&, bool) { /* Nothing to do as this is directly persisted for the time being */ }));
   }
#endif

   //////////////////////////////////////////////////////////////////////////////////////////////////

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Display synchronization"s));

   // Sync modes:
   // - Hardware Synchronization (Hardware VSync)
   // - Hardware Synchronization (Adaptive Vsync)
   // - Hardware Synchronization (Frame Pacing) => 'fake' multithreading where VSYNC is done on an ancillary thread while continuously syncing game logic/rendering
   // - Synchronize to display FPS (Software VSync)
   // - Synchronize to user selected FPS
   // Sync: None / VSync / Adaptive VSync / Frame Paced VSync
   // Limit FPS: None / Display / User
   constexpr bool adaptiveVSyncSupported = isDX9 || isOpenGL;
   AddItem(std::make_unique<InGameUIItem>(
      VPX::Properties::EnumPropertyDef(""s, ""s, "Display Synchronization"s,
         "Select how frame generation is synchronized to display refresh rate.\nHardware synchronization is recommended for smoother gameplay.\nVSync stands for 'Vertical Synchronization'."s,
         false, 0, 0,
         vector { "Hardware VSync"s,
#if defined(ENABLE_DX9) || defined(ENABLE_OPENGL)
            "Hardware Adaptive VSync"s, "Hardware Frame Paced VSync"s,
#endif
            "Software VSync"s, "Custom FPS"s, "No synchronization"s }),
      [this]()
      {
         switch (m_player->GetVideoSyncMode())
         {
         case VideoSyncMode::VSM_VSYNC: return 0;
         case VideoSyncMode::VSM_ADAPTIVE_VSYNC: return 1;
         case VideoSyncMode::VSM_FRAME_PACING: return 2;
         case VideoSyncMode::VSM_NONE:
            if (m_player->GetTargetRefreshRate() == m_player->m_playfieldWnd->GetRefreshRate())
               return adaptiveVSyncSupported ? 3 : 1; // Main display refresh rate => Software VSync
            if (m_player->GetTargetRefreshRate() == 10000.f)
               return adaptiveVSyncSupported ? 5 : 3; // No synchronization
            return adaptiveVSyncSupported ? 4 : 2; // Custom FPS
         default: assert(false); return 0;
         }
      }, // Live
      [this](Settings& settings)
      {
         switch (settings.GetPlayer_SyncMode())
         {
         case VideoSyncMode::VSM_VSYNC: return 0;
         case VideoSyncMode::VSM_ADAPTIVE_VSYNC: return 1;
         case VideoSyncMode::VSM_FRAME_PACING: return 2;
         case VideoSyncMode::VSM_NONE:
            if (settings.GetPlayer_MaxFramerate() == m_player->m_playfieldWnd->GetRefreshRate())
               return adaptiveVSyncSupported ? 3 : 1; // Main display refresh rate => Software VSync
            if (settings.GetPlayer_MaxFramerate() == 10000.f)
               return adaptiveVSyncSupported ? 5 : 3; // No synchronization
            return adaptiveVSyncSupported ? 4 : 2; // Custom FPS
         default: assert(false); return 0;
         }
      }, // Stored
      [this](int, int v)
      {
         if (!adaptiveVSyncSupported && v > 0)
            v += 2;
         switch (v)
         {
         case 0:
            m_player->SetVideoSyncMode(VideoSyncMode::VSM_VSYNC);
            m_player->SetTargetRefreshRate(10000.f);
            break;
         case 1:
            m_player->SetVideoSyncMode(VideoSyncMode::VSM_ADAPTIVE_VSYNC);
            m_player->SetTargetRefreshRate(10000.f);
            break;
         case 2:
            m_player->SetVideoSyncMode(VideoSyncMode::VSM_FRAME_PACING);
            m_player->SetTargetRefreshRate(10000.f);
            break;
         case 3:
            m_player->SetVideoSyncMode(VideoSyncMode::VSM_NONE);
            m_player->SetTargetRefreshRate(m_player->m_playfieldWnd->GetRefreshRate());
            break;
         case 4:
            m_player->SetVideoSyncMode(VideoSyncMode::VSM_NONE);
            m_player->SetTargetRefreshRate(roundf(2.f * m_player->m_playfieldWnd->GetRefreshRate()));
            break;
         case 5:
            m_player->SetVideoSyncMode(VideoSyncMode::VSM_NONE);
            m_player->SetTargetRefreshRate(10000.f);
            break;
         }
         BuildPage();
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
         [this](Settings& settings) { return settings.GetPlayer_MaxFramerate(); }, // Stored
         [this](float, float v) { m_player->SetTargetRefreshRate(v == m_player->m_playfieldWnd->GetRefreshRate() ? v - 0.1f : v); }, // The player would interpret this as software VSync
         [](Settings&) { /* Nothing to do, as save is handled by the main combo */ }, [](float, Settings&, bool) { /* Nothing to do, as save is handled by the main combo */ }));
   }

#ifndef ENABLE_OPENGL
   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_MaxPrerenderedFrames, "%4d Frames"s, //
      [this]() { return m_player->m_ptable->m_settings.GetPlayer_MaxPrerenderedFrames(); }, //
      [this](int, int v)
      {
         m_player->m_ptable->m_settings.SetPlayer_MaxPrerenderedFrames(v, false);
         m_notificationId = m_player->m_liveUI->PushNotification("This change will be applied after restarting the player."s, 3000, m_notificationId);
      }));
#endif

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_VisualLatencyCorrection, "%4d ms"s, //
      [this]() { return m_player->m_renderer->m_renderDevice->GetVisualLatencyCorrection(); }, //
      [this](int, int v) { m_player->m_renderer->m_renderDevice->SetVisualLatencyCorrection(v); }));


   //////////////////////////////////////////////////////////////////////////////////////////////////

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Antialiasing quality"s));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_AAFactor, 100.f, "%4.1f %%"s, //
      [this]() { return m_player->m_ptable->m_settings.GetPlayer_AAFactor(); }, //
      [this](float, float v)
      {
         m_player->m_ptable->m_settings.SetPlayer_AAFactor(v, false);
         m_notificationId = m_player->m_liveUI->PushNotification("This change will be applied after restarting the player."s, 3000, m_notificationId);
      }));

#ifdef ENABLE_OPENGL
   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_MSAASamples, //
      [this]() { return m_player->m_ptable->m_settings.GetPlayer_MSAASamples(); }, //
      [this](int, int v)
      {
         m_player->m_ptable->m_settings.SetPlayer_MSAASamples(v, false);
         m_notificationId = m_player->m_liveUI->PushNotification("This change will be applied after restarting the player."s, 3000, m_notificationId);
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

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Miscellaneous features"s));

   if (m_player->m_ptable->m_enableSSR && m_player->m_ptable->m_SSRScale > 0.f && m_player->m_renderer->m_renderDevice->DepthBufferReadBackAvailable())
      AddItem(std::make_unique<InGameUIItem>( //
         Settings::m_propPlayer_SSRefl, //
         [this]() { return m_player->m_renderer->HasAdditiveScreenSpaceReflection(); }, //
         [this](bool v) { m_player->m_renderer->EnableAdditiveScreenSpaceReflection(v); }));

   //////////////////////////////////////////////////////////////////////////////////////////////////

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Performance & Troubleshooting"s));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>(
      VPX::Properties::EnumPropertyDef(""s, ""s, "Ambient Occlusion"s,
         "Limit the quality of ambient occlusion for better performance.\r\nDynamic is the better with contact shadows for dynamic objects but higher performance requirements"s, false, 0, 0,
         vector { "Disabled"s, "Static"s, "Dynamic"s }),
      [this]()
      {
         if (m_player->m_ptable->m_settings.GetPlayer_DisableAO())
            return 0;
         if (m_player->m_ptable->m_settings.GetPlayer_DynamicAO())
            return 2;
         return 1;
      }, // Live
      [this](Settings& settings)
      {
         if (settings.GetPlayer_DisableAO())
            return 0;
         if (settings.GetPlayer_DynamicAO())
            return 2;
         return 1;
      }, // Stored
      [this](int, int v)
      {
         m_player->m_ptable->m_settings.ResetPlayer_DisableAO();
         m_player->m_ptable->m_settings.ResetPlayer_DynamicAO();
         m_player->m_ptable->m_settings.SetPlayer_DisableAO(v == 0, false);
         m_player->m_ptable->m_settings.SetPlayer_DynamicAO(v == 2, false);
      },
      [](Settings&) { /* Nothing to do as this is directly persisted for the time being */ },
      [](int, Settings&, bool) { /* Nothing to do as this is directly persisted for the time being */ }));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   // Maybe setup a combo with a few preset values ?
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_MaxTexDimension, "%4d"s, //
      [this]() { return m_player->m_ptable->m_settings.GetPlayer_MaxTexDimension(); }, //
      [this](int, int v)
      {
         m_player->m_ptable->m_settings.SetPlayer_MaxTexDimension(v, false);
         m_notificationId = m_player->m_liveUI->PushNotification("This change will be applied after restarting the player."s, 3000, m_notificationId);
      }));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>(
      Settings::m_propPlayer_PFReflection, //
      [this]() { return m_player->m_ptable->m_settings.GetPlayer_PFReflection(); }, //
      [this](int, int v)
      {
         m_player->m_ptable->m_settings.SetPlayer_PFReflection(v, false);
         m_notificationId = m_player->m_liveUI->PushNotification("This change will be applied after restarting the player."s, 3000, m_notificationId);
      }));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   // Maybe setup a combo with a few preset values ?
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_AlphaRampAccuracy, "%4d"s, //
      [this]() { return m_player->m_ptable->m_settings.GetPlayer_AlphaRampAccuracy(); }, //
      [this](int, int v)
      {
         m_player->m_ptable->m_settings.SetPlayer_AlphaRampAccuracy(v, false);
         m_notificationId = m_player->m_liveUI->PushNotification("This change will be applied after restarting the player."s, 3000, m_notificationId);
      }));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_HDRDisableToneMapper, //
      [this]() { return m_player->m_renderer->m_HDRforceDisableToneMapper; }, //
      [this](bool v) { m_player->m_renderer->m_HDRforceDisableToneMapper = v; }));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_HDRGlobalExposure, 1.f, "%4.2f"s, //
      [this]() { return m_player->m_ptable->m_settings.GetPlayer_HDRGlobalExposure(); }, //
      [this](float, float v)
      {
         m_player->m_ptable->m_settings.SetPlayer_HDRGlobalExposure(v, false);
         m_notificationId = m_player->m_liveUI->PushNotification("This change will be applied after restarting the player."s, 3000, m_notificationId);
      }));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_CompressTextures, //
      [this]() { return m_player->m_ptable->m_settings.GetPlayer_CompressTextures(); }, //
      [this](bool v)
      {
         m_player->m_ptable->m_settings.SetPlayer_CompressTextures(v, false);
         m_notificationId = m_player->m_liveUI->PushNotification("This change will be applied after restarting the player."s, 3000, m_notificationId);
      }));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_UseNVidiaAPI, //
      [this]() { return m_player->m_ptable->m_settings.GetPlayer_UseNVidiaAPI(); }, //
      [this](bool v)
      {
         m_player->m_ptable->m_settings.SetPlayer_UseNVidiaAPI(v, false);
         m_notificationId = m_player->m_liveUI->PushNotification("This change will be applied after restarting the player."s, 3000, m_notificationId);
      }));

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
      [this]() { return m_player->m_ptable->m_settings.GetPlayer_SoftwareVertexProcessing(); }, //
      [this](bool v)
      {
         m_player->m_ptable->m_settings.SetPlayer_SoftwareVertexProcessing(v, false);
         m_notificationId = m_player->m_liveUI->PushNotification("This change will be applied after restarting the player."s, 3000, m_notificationId);
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

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_BallTrail, //
      [this]() { return m_player->m_renderer->m_trailForBalls; }, //
      [this](bool v) { m_player->m_renderer->m_trailForBalls = v; }));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_BallTrailStrength, 1.f, "%4.2f"s, //
      [this]() { return m_player->m_renderer->m_ballTrailStrength; }, //
      [this](float, float v) { m_player->m_renderer->m_ballTrailStrength = v; }));
}

}
