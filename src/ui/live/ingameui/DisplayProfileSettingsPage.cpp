// license:GPLv3+

#include "core/stdafx.h"

#include "DisplayProfileSettingsPage.h"

#include "renderer/RenderCommand.h"

namespace VPX::InGameUI
{

DisplayProfileSettingsPage::DisplayProfileSettingsPage()
   : InGameUIPage("Display Profile Settings"s, ""s, SaveMode::Both)
   , m_dmdTexture(BaseTexture::Create(128, 32, BaseTexture::Format::BW_FP32))
{
   BuildPage();
}

void DisplayProfileSettingsPage::BuildPage()
{
   ClearItems();
   AddItem(std::make_unique<InGameUIItem>(
      VPX::Properties::EnumPropertyDef(""s, ""s, "Edited profile"s, "Select the DMD or alphanumeric segment display profile you cant ot adjust."s, false, 0, 0,
         vector<string> { //
            "DMD: Legacy VPX"s, "DMD: Neon Plasma"s, "DMD: Red LED"s, "DMD: Green LED"s, "DMD: Yellow LED"s, "DMD: Generic Plasma"s, "DMD: Generic LED"s, //
            "Alpha: Neon Plasma"s, "Alpha: Blue VFD"s, "Alpha: Green VFD"s, "Alpha: Red LED"s, "Alpha: Green LED"s, "Alpha: Yellow LED"s, "Alpha: Generic Plasma"s, "Alpha: Generic LED"s }),
      [this]() { return m_selectedProfile; }, // Live
      [this](Settings&) { return m_selectedProfile; }, // Stored
      [this](int, int v)
      {
         m_selectedProfile = v;
         BuildPage();
      },
      [](Settings&) { /* UI state, not persisted */ }, //
      [](int, Settings&, bool) { /* UI state, not persisted */ }));

   if (m_selectedProfile < 7)
      BuildDMDPage();
   else
      BuildAlphaPage();

   AddItem(std::make_unique<InGameUIItem>(
      VPX::Properties::FloatPropertyDef(""s, ""s, "Preview Brightness"s, "Adjust the brightness level of the preview."s, false, 0.1f, 5.f, 0.1f, 1.f), 1.f, "%4.1f"s, //
      [this]() { return m_previewBrightness; }, // Live
      [this](Settings&) { return m_previewBrightness; }, // Stored
      [this](float, float v) { m_previewBrightness = v; },
      [](Settings&) { /* UI state, not persisted */ }, //
      [](float, Settings&, bool) { /* UI state, not persisted */ }));
}

void DisplayProfileSettingsPage::BuildDMDPage()
{
   const int profile = m_selectedProfile;

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propDMD_ProfileLegacy[profile], //
      [this, profile]() { return m_player->m_renderer->m_dmdUseLegacyRenderer[profile]; }, //
      [this, profile](bool v) { m_player->m_renderer->m_dmdUseLegacyRenderer[profile] = v; }));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propDMD_ProfileDotBrightness[profile], 1.f, "%4.1f"s, //
      [this, profile]() { return m_player->m_renderer->m_dmdDotColor[profile].w; }, //
      [this, profile](float, float v) { m_player->m_renderer->m_dmdDotColor[profile].w = v; }));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propDMD_ProfileDotSize[profile], 1.f, "%4.2f"s, //
      [this, profile]() { return m_player->m_renderer->m_dmdDotProperties[profile].x; }, //
      [this, profile](float, float v) { m_player->m_renderer->m_dmdDotProperties[profile].x = v; }));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propDMD_ProfileDotSharpness[profile], 1.f, "%4.2f"s, //
      [this, profile]() { return m_player->m_renderer->m_dmdDotProperties[profile].y; }, //
      [this, profile](float, float v) { m_player->m_renderer->m_dmdDotProperties[profile].y = v; }));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propDMD_ProfileDiffuseGlow[profile], 1.f, "%4.1f"s, //
      [this, profile]() { return m_player->m_renderer->m_dmdDotProperties[profile].z; }, //
      [this, profile](float, float v) { m_player->m_renderer->m_dmdDotProperties[profile].z = v; }));

   // TODO it would be nice to implement a pincab friendly color picker
   m_srgbLit.r = static_cast<int>(sRGB(m_player->m_renderer->m_dmdDotColor[profile].x) * 255.f);
   m_srgbLit.g = static_cast<int>(sRGB(m_player->m_renderer->m_dmdDotColor[profile].y) * 255.f);
   m_srgbLit.b = static_cast<int>(sRGB(m_player->m_renderer->m_dmdDotColor[profile].z) * 255.f);
   AddItem(std::make_unique<InGameUIItem>(
      VPX::Properties::IntPropertyDef(""s, ""s, "Dot Tint Red"s, ""s, false, 0, 255, 128), "%3d / 255"s, //
      [this, profile]() { return m_srgbLit.r; }, //
      [this, profile](const Settings& settings) { return settings.GetDMD_ProfileDotTint(profile) & 0xFF; }, //
      [this, profile](int, int v)
      {
         m_srgbLit.r = v;
         m_player->m_renderer->m_dmdDotColor[profile].x = InvsRGB(static_cast<float>(v) / 255.f);
      }, //
      [profile](Settings& settings) { settings.ResetDMD_ProfileDotTint(profile); }, // we reset the 3 channels at once
      [profile](int v, Settings& settings, bool isTableOverride) { settings.SetDMD_ProfileDotTint(profile, (settings.GetDMD_ProfileDotTint(profile) & 0xFFFF00) | v, isTableOverride); }));
   AddItem(std::make_unique<InGameUIItem>(
      VPX::Properties::IntPropertyDef(""s, ""s, "Dot Tint Green"s, ""s, false, 0, 255, 128), "%3d / 255"s, //
      [this, profile]() { return m_srgbLit.g; }, //
      [this, profile](Settings& settings) { return (settings.GetDMD_ProfileDotTint(profile) >> 8) & 0xFF; }, //
      [this, profile](int, int v)
      {
         m_srgbLit.g = v;
         m_player->m_renderer->m_dmdDotColor[profile].y = InvsRGB(static_cast<float>(v) / 255.f);
      }, //
      [profile](Settings& settings) { settings.ResetDMD_ProfileDotTint(profile); }, // we reset the 3 channels at once
      [profile](int v, Settings& settings, bool isTableOverride)
      { settings.SetDMD_ProfileDotTint(profile, (settings.GetDMD_ProfileDotTint(profile) & 0xFF00FF) | (v << 8), isTableOverride); }));
   AddItem(std::make_unique<InGameUIItem>(
      VPX::Properties::IntPropertyDef(""s, ""s, "Dot Tint Blue"s, ""s, false, 0, 255, 128), "%3d / 255"s, //
      [this, profile]() { return m_srgbLit.b; }, //
      [this, profile](const Settings& settings) { return (settings.GetDMD_ProfileDotTint(profile) >> 16) & 0xFF; }, //
      [this, profile](int, int v)
      {
         m_srgbLit.b = v;
         m_player->m_renderer->m_dmdDotColor[profile].z = InvsRGB(static_cast<float>(v) / 255.f);
      }, //
      [profile](Settings& settings) { settings.ResetDMD_ProfileDotTint(profile); }, // we reset the 3 channels at once
      [profile](int v, Settings& settings, bool isTableOverride)
      { settings.SetDMD_ProfileDotTint(profile, (settings.GetDMD_ProfileDotTint(profile) & 0x00FFFF) | (v << 16), isTableOverride); }));


   // TODO it would be nice to implement a pincab friendly color picker
   m_srgbUnlit.r = static_cast<int>(sRGB(m_player->m_renderer->m_dmdUnlitDotColor[profile].x) * 255.f);
   m_srgbUnlit.g = static_cast<int>(sRGB(m_player->m_renderer->m_dmdUnlitDotColor[profile].y) * 255.f);
   m_srgbUnlit.b = static_cast<int>(sRGB(m_player->m_renderer->m_dmdUnlitDotColor[profile].z) * 255.f);
   AddItem(std::make_unique<InGameUIItem>(
      VPX::Properties::IntPropertyDef(""s, ""s, "Unlit Dot Color Red"s, ""s, false, 0, 255, 128), "%3d / 255"s, //
      [this, profile]() { return m_srgbUnlit.r; }, //
      [this, profile](const Settings& settings) { return settings.GetDMD_ProfileUnlitDotColor(profile) & 0xFF; }, //
      [this, profile](int, int v)
      {
         m_srgbUnlit.r = v;
         m_player->m_renderer->m_dmdUnlitDotColor[profile].x = InvsRGB(static_cast<float>(v) / 255.f);
      }, //
      [profile](Settings& settings) { settings.ResetDMD_ProfileUnlitDotColor(profile); }, // we reset the 3 channels at once
      [profile](int v, Settings& settings, bool isTableOverride)
      { settings.SetDMD_ProfileUnlitDotColor(profile, (settings.GetDMD_ProfileUnlitDotColor(profile) & 0xFFFF00) | v, isTableOverride); }));
   AddItem(std::make_unique<InGameUIItem>(
      VPX::Properties::IntPropertyDef(""s, ""s, "Unlit Dot Color Green"s, ""s, false, 0, 255, 128), "%3d / 255"s, //
      [this, profile]() { return m_srgbUnlit.g; }, //
      [this, profile](const Settings& settings) { return (settings.GetDMD_ProfileUnlitDotColor(profile) >> 8) & 0xFF; }, //
      [this, profile](int, int v)
      {
         m_srgbUnlit.g = v;
         m_player->m_renderer->m_dmdUnlitDotColor[profile].y = InvsRGB(static_cast<float>(v) / 255.f);
      }, //
      [profile](Settings& settings) { settings.ResetDMD_ProfileUnlitDotColor(profile); }, // we reset the 3 channels at once
      [profile](int v, Settings& settings, bool isTableOverride)
      { settings.SetDMD_ProfileUnlitDotColor(profile, (settings.GetDMD_ProfileUnlitDotColor(profile) & 0xFF00FF) | (v << 8), isTableOverride); }));
   AddItem(std::make_unique<InGameUIItem>(
      VPX::Properties::IntPropertyDef(""s, ""s, "Unlit Dot Color Blue"s, ""s, false, 0, 255, 128), "%3d / 255"s, //
      [this, profile]() { return m_srgbUnlit.b; }, //
      [this, profile](const Settings& settings) { return (settings.GetDMD_ProfileUnlitDotColor(profile) >> 16) & 0xFF; }, //
      [this, profile](int, int v)
      {
         m_srgbUnlit.b = v;
         m_player->m_renderer->m_dmdUnlitDotColor[profile].z = InvsRGB(static_cast<float>(v) / 255.f);
      }, //
      [profile](Settings& settings) { settings.ResetDMD_ProfileUnlitDotColor(profile); }, // we reset the 3 channels at once
      [profile](int v, Settings& settings, bool isTableOverride)
      { settings.SetDMD_ProfileUnlitDotColor(profile, (settings.GetDMD_ProfileUnlitDotColor(profile) & 0x00FFFF) | (v << 16), isTableOverride); }));
}

void DisplayProfileSettingsPage::BuildAlphaPage()
{
   const int profile = m_selectedProfile - 7;

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propAlpha_ProfileBrightness[profile], 1.f, "%4.1f"s, //
      [this, profile]() { return m_player->m_renderer->m_segColor[profile].w; }, //
      [this, profile](float, float v) { m_player->m_renderer->m_segColor[profile].w = v; }));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propAlpha_ProfileDiffuseGlow[profile], 1.f, "%4.1f"s, //
      [this, profile]() { return m_player->m_renderer->m_segUnlitColor[profile].w; }, //
      [this, profile](float, float v) { m_player->m_renderer->m_segUnlitColor[profile].w = v; }));

   // TODO it would be nice to implement a pincab friendly color picker
   m_srgbLit.r = static_cast<int>(sRGB(m_player->m_renderer->m_segColor[profile].x) * 255.f);
   m_srgbLit.g = static_cast<int>(sRGB(m_player->m_renderer->m_segColor[profile].y) * 255.f);
   m_srgbLit.b = static_cast<int>(sRGB(m_player->m_renderer->m_segColor[profile].z) * 255.f);
   AddItem(std::make_unique<InGameUIItem>(
      VPX::Properties::IntPropertyDef(""s, ""s, "Segment Tint Red"s, ""s, false, 0, 255, 128), "%3d / 255"s, //
      [this, profile]() { return m_srgbLit.r; }, //
      [this, profile](const Settings& settings) { return settings.GetAlpha_ProfileColor(profile) & 0xFF; }, //
      [this, profile](int, int v)
      {
         m_srgbLit.r = v;
         m_player->m_renderer->m_segColor[profile].x = InvsRGB(static_cast<float>(v) / 255.f);
      }, //
      [profile](Settings& settings) { settings.ResetAlpha_ProfileColor(profile); }, // we reset the 3 channels at once
      [profile](int v, Settings& settings, bool isTableOverride) { settings.SetAlpha_ProfileColor(profile, (settings.GetAlpha_ProfileColor(profile) & 0xFFFF00) | v, isTableOverride); }));
   AddItem(std::make_unique<InGameUIItem>(
      VPX::Properties::IntPropertyDef(""s, ""s, "Segment Tint Green"s, ""s, false, 0, 255, 128), "%3d / 255"s, //
      [this, profile]() { return m_srgbLit.g; }, //
      [this, profile](const Settings& settings) { return (settings.GetAlpha_ProfileColor(profile) >> 8) & 0xFF; }, //
      [this, profile](int, int v)
      {
         m_srgbLit.g = v;
         m_player->m_renderer->m_segColor[profile].y = InvsRGB(static_cast<float>(v) / 255.f);
      }, //
      [profile](Settings& settings) { settings.ResetAlpha_ProfileColor(profile); }, // we reset the 3 channels at once
      [profile](int v, Settings& settings, bool isTableOverride)
      { settings.SetAlpha_ProfileColor(profile, (settings.GetAlpha_ProfileColor(profile) & 0xFF00FF) | (v << 8), isTableOverride); }));
   AddItem(std::make_unique<InGameUIItem>(
      VPX::Properties::IntPropertyDef(""s, ""s, "Segment Tint Blue"s, ""s, false, 0, 255, 128), "%3d / 255"s, //
      [this, profile]() { return m_srgbLit.b; }, //
      [this, profile](const Settings& settings) { return (settings.GetAlpha_ProfileColor(profile) >> 16) & 0xFF; }, //
      [this, profile](int, int v)
      {
         m_srgbLit.b = v;
         m_player->m_renderer->m_segColor[profile].z = InvsRGB(static_cast<float>(v) / 255.f);
      }, //
      [profile](Settings& settings) { settings.ResetAlpha_ProfileColor(profile); }, // we reset the 3 channels at once
      [profile](int v, Settings& settings, bool isTableOverride)
      { settings.SetAlpha_ProfileColor(profile, (settings.GetAlpha_ProfileColor(profile) & 0x00FFFF) | (v << 16), isTableOverride); }));


   // TODO it would be nice to implement a pincab friendly color picker
   m_srgbUnlit.r = static_cast<int>(sRGB(m_player->m_renderer->m_segUnlitColor[profile].x) * 255.f);
   m_srgbUnlit.g = static_cast<int>(sRGB(m_player->m_renderer->m_segUnlitColor[profile].y) * 255.f);
   m_srgbUnlit.b = static_cast<int>(sRGB(m_player->m_renderer->m_segUnlitColor[profile].z) * 255.f);
   AddItem(std::make_unique<InGameUIItem>(
      VPX::Properties::IntPropertyDef(""s, ""s, "Unlit Segment Color Red"s, ""s, false, 0, 255, 128), "%3d / 255"s, //
      [this, profile]() { return m_srgbUnlit.r; }, //
      [this, profile](Settings& settings) { return settings.GetAlpha_ProfileUnlit(profile) & 0xFF; }, //
      [this, profile](int, int v)
      {
         m_srgbUnlit.r = v;
         m_player->m_renderer->m_segUnlitColor[profile].x = InvsRGB(static_cast<float>(v) / 255.f);
      }, //
      [profile](Settings& settings) { settings.ResetAlpha_ProfileUnlit(profile); }, // we reset the 3 channels at once
      [profile](int v, Settings& settings, bool isTableOverride) { settings.SetAlpha_ProfileUnlit(profile, (settings.GetAlpha_ProfileUnlit(profile) & 0xFFFF00) | v, isTableOverride); }));
   AddItem(std::make_unique<InGameUIItem>(
      VPX::Properties::IntPropertyDef(""s, ""s, "Unlit Segment Color Green"s, ""s, false, 0, 255, 128), "%3d / 255"s, //
      [this, profile]() { return m_srgbUnlit.g; }, //
      [this, profile](const Settings& settings) { return (settings.GetAlpha_ProfileUnlit(profile) >> 8) & 0xFF; }, //
      [this, profile](int, int v)
      {
         m_srgbUnlit.g = v;
         m_player->m_renderer->m_segUnlitColor[profile].y = InvsRGB(static_cast<float>(v) / 255.f);
      }, //
      [profile](Settings& settings) { settings.ResetAlpha_ProfileUnlit(profile); }, // we reset the 3 channels at once
      [profile](int v, Settings& settings, bool isTableOverride)
      { settings.SetAlpha_ProfileUnlit(profile, (settings.GetAlpha_ProfileUnlit(profile) & 0xFF00FF) | (v << 8), isTableOverride); }));
   AddItem(std::make_unique<InGameUIItem>(
      VPX::Properties::IntPropertyDef(""s, ""s, "Unlit Segment Color Blue"s, ""s, false, 0, 255, 128), "%3d / 255"s, //
      [this, profile]() { return m_srgbUnlit.b; }, //
      [this, profile](const Settings& settings) { return (settings.GetAlpha_ProfileUnlit(profile) >> 16) & 0xFF; }, //
      [this, profile](int, int v)
      {
         m_srgbUnlit.b = v;
         m_player->m_renderer->m_segUnlitColor[profile].z = InvsRGB(static_cast<float>(v) / 255.f);
      }, //
      [profile](Settings& settings) { settings.ResetAlpha_ProfileUnlit(profile); }, // we reset the 3 channels at once
      [profile](int v, Settings& settings, bool isTableOverride)
      { settings.SetAlpha_ProfileUnlit(profile, (settings.GetAlpha_ProfileUnlit(profile) & 0x00FFFF) | (v << 16), isTableOverride); }));
}

void DisplayProfileSettingsPage::Render(float elapsed)
{
   InGameUIPage::Render(elapsed);

   float posx, posy;
   float width, height;

   if (m_selectedProfile < 7)
   {
      width = GetWindowSize().x;
      height = width / 4.f;
      posx = GetWindowPos().x;
      posy = GetWindowPos().y - ImGui::GetStyle().ItemSpacing.y - height;

      memset(m_dmdTexture->data(), 0, 128 * 32 * 4);
      m_particles.resize(128);
      for (auto& particle : m_particles)
      {
         particle.life += elapsed;
         if (particle.life < particle.lifespan)
         {
            static_cast<float*>(m_dmdTexture->data())[particle.pos] = saturate(1.f - 3.f * fabs(particle.life / particle.lifespan - 0.66f));
         }
         else
         {
            particle.pos = static_cast<int>(rand_mt_01() * (float)(128 * 32));
            particle.lifespan = 0.5f + rand_mt_01() * 1.f;
            particle.life = 0.f;
         }
      }
      BaseTexture::Update(m_dmdTexture, 128, 32, BaseTexture::Format::BW_FP32, m_dmdTexture->data());

      m_player->m_renderer->SetupDMDRender(m_selectedProfile, true, vec3(1.f, 1.f, 1.f), m_previewBrightness, m_dmdTexture, 1.0f, Renderer::Reinhard, nullptr, vec4(0.f, 0.f, 0.f, 0.f),
         vec3(1.f, 1.f, 1.f), 0.f, nullptr, vec4(0.f, 0.f, 1.f, 1.f), vec3(0.f, 0.f, 0.f));
   }
   else
   {
      height = GetWindowSize().x / 4.f;
      width = height * 96.f / 128.f;
      posx = GetWindowPos().x + (GetWindowSize().x - width) / 2.f;
      posy = GetWindowPos().y - ImGui::GetStyle().ItemSpacing.y - height;

      float frame[16];
      for (int i = 0; i < 16; i++)
         frame[i] = 1.f;

      m_player->m_renderer->m_renderDevice->SetRenderState(RenderState::BLENDOP, RenderState::BLENDOP_MAX);
      m_player->m_renderer->m_renderDevice->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_TRUE);
      m_player->m_renderer->m_renderDevice->SetRenderState(RenderState::SRCBLEND, RenderState::SRC_ALPHA);
      m_player->m_renderer->m_renderDevice->SetRenderState(RenderState::DESTBLEND, RenderState::ONE);
      m_player->m_renderer->m_renderDevice->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
      Renderer::SegmentFamily family = Renderer::SegmentFamily::Generic;
      SegElementType type = SegElementType::CTLPI_SEG_LAYOUT_14D;
      switch (m_selectedProfile - 7)
      {
      case 0: // Neon Plasma
         family = Renderer::SegmentFamily::Williams;
         type = SegElementType::CTLPI_SEG_LAYOUT_7C;
         break;
      case 1: // VFD Blueish
         family = Renderer::SegmentFamily::Gottlieb;
         type = SegElementType::CTLPI_SEG_LAYOUT_7;
         break;
      case 2: // VFD Greenish
         family = Renderer::SegmentFamily::Gottlieb;
         type = SegElementType::CTLPI_SEG_LAYOUT_14DC;
         break;
      case 3: // Red LED
         family = Renderer::SegmentFamily::Bally;
         type = SegElementType::CTLPI_SEG_LAYOUT_7C;
         break;
      }
      g_pplayer->m_renderer->SetupSegmentRenderer(m_selectedProfile - 7, true, vec3(1.f, 1.f, 1.f), m_previewBrightness, family, type, frame, Renderer::Reinhard, nullptr,
         vec4(0.f, 0.f, 0.f, 0.f), vec3(1.f, 1.f, 1.f), 0.f, nullptr, vec4(0.f, 0.f, 1.f, 1.f), vec3(0.f, 0.f, 0.f));
   }

   ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
   draw_list->AddRectFilled(ImVec2(posx, posy), ImVec2(posx + width, posy + height), IM_COL32_BLACK);

   Vertex3D_NoTex2 vertices[4] = //
      {
         { 1.f, 1.f, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f }, //
         { 0.f, 1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f }, //
         { 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 1.f, 0.f }, //
         { 0.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f } //
      };
   const float ow = static_cast<float>(m_player->m_renderer->m_renderDevice->GetCurrentRenderTarget()->GetWidth());
   const float oh = static_cast<float>(m_player->m_renderer->m_renderDevice->GetCurrentRenderTarget()->GetHeight());
   for (unsigned int i = 0; i < 4; ++i)
   {
      vertices[i].x = (vertices[i].x * width + posx) * 2.0f / ow - 1.0f;
      vertices[i].y = 1.0f - (vertices[i].y * height + posy) * 2.0f / oh;
   }
   Matrix3D matWorldViewProj[2];
   matWorldViewProj[0] = Matrix3D::MatrixIdentity();
   matWorldViewProj[1] = Matrix3D::MatrixIdentity();
   m_player->m_renderer->m_renderDevice->m_DMDShader->SetMatrix(SHADER_matWorldViewProj, &matWorldViewProj[0], m_player->m_renderer->m_renderDevice->GetCurrentRenderTarget()->m_nLayers);
   m_player->m_renderer->m_renderDevice->DrawTexturedQuad(m_player->m_renderer->m_renderDevice->m_DMDShader, vertices);
   m_player->m_renderer->m_renderDevice->GetCurrentPass()->m_commands.back()->SetTransparent(true);
   m_player->m_renderer->m_renderDevice->GetCurrentPass()->m_commands.back()->SetDepth(-10000.f);
   m_player->m_renderer->UpdateBasicShaderMatrix();
}

}

