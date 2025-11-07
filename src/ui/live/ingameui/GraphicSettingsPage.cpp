// license:GPLv3+

#include "core/stdafx.h"

#include "GraphicSettingsPage.h"

namespace VPX::InGameUI
{

GraphicSettingsPage::GraphicSettingsPage()
   : InGameUIPage("Graphic Settings"s, ""s, SaveMode::Both)
{
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
         VPX::Properties::EnumPropertyDef(""s, ""s, "Graphics Backend"s, ""s, 0, 0, renderers),
         [this, renderers]() { return max(0, FindIndexOf(renderers, m_player->m_ptable->m_settings.GetPlayer_GfxBackend())); }, // Live
         [this, renderers]() { return max(0, FindIndexOf(renderers, m_player->m_ptable->m_settings.GetPlayer_GfxBackend())); }, // Stored (same)
         [this, renderers](int, int v) {
            m_player->m_ptable->m_settings.SetPlayer_GfxBackend(renderers[v], false);
            m_player->m_liveUI->PushNotification("This change will be applied after restarting the player."s, 3000);
         },
         [](Settings&) { /* Nothing to do as this is directly persisted for the time being */ },
         [](int, Settings&, bool) { /* Nothing to do as this is directly persisted for the time being */ }));
   }
   #endif

   //////////////////////////////////////////////////////////////////////////////////////////////////

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Display synchronization"s));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_SyncMode, //
      [this]() { return m_player->m_ptable->m_settings.GetPlayer_SyncMode(); }, //
      [this](int, int v)
      {
         m_player->m_ptable->m_settings.SetPlayer_SyncMode(v, false);
         m_player->m_liveUI->PushNotification("This change will be applied after restarting the player."s, 3000);
      }));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_MaxFramerate, "%3d FPS"s, //
      [this]()
      {
         float target = m_player->GetTargetRefreshRate();
         if (target == m_player->m_playfieldWnd->GetRefreshRate())
            return -1; // Main display refresh rate
         if (target == 10000.f)
            return 0; // Unlimited
         return static_cast<int>(target);
      }, //
      [this](int, int v) { m_player->SetTargetRefreshRate(v); }));

   #ifndef ENABLE_OPENGL
   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_MaxPrerenderedFrames, "%4d Frames"s, //
      [this]() { return m_player->m_ptable->m_settings.GetPlayer_MaxPrerenderedFrames(); }, //
      [this](int, int v)
      {
         m_player->m_ptable->m_settings.SetPlayer_MaxPrerenderedFrames(v, false);
         m_player->m_liveUI->PushNotification("This change will be applied after restarting the player."s, 3000);
      }));
   #endif

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_VisualLatencyCorrection, "%4d ms"s, //
      [this]() { return m_player->m_ptable->m_settings.GetPlayer_VisualLatencyCorrection(); }, //
      [this](int, int v)
      {
         m_player->m_ptable->m_settings.SetPlayer_VisualLatencyCorrection(v, false);
         m_player->m_liveUI->PushNotification("This change will be applied after restarting the player."s, 3000);
      }));


   //////////////////////////////////////////////////////////////////////////////////////////////////

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Antialiasing quality"s));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_AAFactor, 100.f, "%4.1f %%"s, //
      [this]() { return m_player->m_ptable->m_settings.GetPlayer_AAFactor(); }, //
      [this](float, float v)
      {
         m_player->m_ptable->m_settings.SetPlayer_AAFactor(v, false);
         m_player->m_liveUI->PushNotification("This change will be applied after restarting the player."s, 3000);
      }));

   #ifdef ENABLE_OPENGL
   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_MSAASamples, //
      [this]() { return m_player->m_ptable->m_settings.GetPlayer_MSAASamples(); }, //
      [this](int, int v)
      {
         m_player->m_ptable->m_settings.SetPlayer_MSAASamples(v, false);
         m_player->m_liveUI->PushNotification("This change will be applied after restarting the player."s, 3000);
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
      VPX::Properties::EnumPropertyDef(""s, ""s, "Ambient Occlusion"s, "Limit the quality of ambient occlusion for better performance.\r\nDynamic is the better with contact shadows for dynamic objects but higher performance requirements"s, 0, 0, vector { "Disabled"s, "Static"s, "Dynamic"s }),
      [this]() {
         if (m_player->m_ptable->m_settings.GetPlayer_DisableAO())
            return 0;
         if (m_player->m_ptable->m_settings.GetPlayer_DynamicAO())
            return 2;
         return 1;
      }, //
      [this]()
      {
         if (m_player->m_ptable->m_settings.GetPlayer_DisableAO())
            return 0;
         if (m_player->m_ptable->m_settings.GetPlayer_DynamicAO())
            return 2;
         return 1;
      }, //
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
         m_player->m_liveUI->PushNotification("This change will be applied after restarting the player."s, 3000);
      }));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>(
      Settings::m_propPlayer_PFReflection, //
      [this]() { return m_player->m_ptable->m_settings.GetPlayer_PFReflection(); }, //
      [this](int, int v)
      {
         m_player->m_ptable->m_settings.SetPlayer_PFReflection(v, false);
         m_player->m_liveUI->PushNotification("This change will be applied after restarting the player."s, 3000);
      }));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   // Maybe setup a combo with a few preset values ?
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_AlphaRampAccuracy, "%4d"s, //
      [this]() { return m_player->m_ptable->m_settings.GetPlayer_AlphaRampAccuracy(); }, //
      [this](int, int v)
      {
         m_player->m_ptable->m_settings.SetPlayer_AlphaRampAccuracy(v, false);
         m_player->m_liveUI->PushNotification("This change will be applied after restarting the player."s, 3000);
      }));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_HDRDisableToneMapper, //
      [this]() { return m_player->m_renderer->m_HDRforceDisableToneMapper; }, //
      [this](bool v) { m_player->m_renderer->m_HDRforceDisableToneMapper = v; }));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_HDRGlobalExposure, 1.f, "%4.2f"s, //
      [this]() { return m_player->m_ptable->m_settings.GetPlayer_HDRGlobalExposure(); }, //
      [this](float, float v) {
         m_player->m_ptable->m_settings.SetPlayer_HDRGlobalExposure(v, false); 
         m_player->m_liveUI->PushNotification("This change will be applied after restarting the player."s, 3000);
      }));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_CompressTextures, //
      [this]() { return m_player->m_ptable->m_settings.GetPlayer_CompressTextures(); }, //
      [this](bool v)
      {
         m_player->m_ptable->m_settings.SetPlayer_CompressTextures(v, false);
         m_player->m_liveUI->PushNotification("This change will be applied after restarting the player."s, 3000);
      }));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_UseNVidiaAPI, //
      [this]() { return m_player->m_ptable->m_settings.GetPlayer_UseNVidiaAPI(); }, //
      [this](bool v)
      {
         m_player->m_ptable->m_settings.SetPlayer_UseNVidiaAPI(v, false);
         m_player->m_liveUI->PushNotification("This change will be applied after restarting the player."s, 3000);
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
         m_player->m_liveUI->PushNotification("This change will be applied after restarting the player."s, 3000);
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
