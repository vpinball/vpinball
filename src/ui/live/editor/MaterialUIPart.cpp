#pragma once

#include "core/stdafx.h"

#include "MaterialUIPart.h"

namespace VPX::EditorUI
{

MaterialUIPart::MaterialUIPart(Material* material)
   : m_material(material)
{
}

MaterialUIPart::~MaterialUIPart()
{
}

void MaterialUIPart::Render(const EditorRenderContext& ctx) {
}

void MaterialUIPart::UpdatePropertyPane(PropertyPane& props)
{
   // props.EditableHeader("Material", m_material);
   
   /* FIXME
   Material * const live_material = (Material *)(m_selection.is_live ? m_selection.uiPart : m_live_table->m_startupToLive[m_selection.uiPart]);
   Material * const startup_material = (Material *)(m_selection.is_live ? m_live_table->m_liveToStartup[m_selection.uiPart] : m_selection.uiPart);
   Material * const material = (is_live ? live_material : startup_material);
   m_liveUI.CenteredText("Material"s);
   string name = ((Material *)m_selection.uiPart)->m_name;
   ImGui::BeginDisabled(is_live); // Editing the name of a live item can break the script
   if (ImGui::InputText("Name", &name))
   {
      // FIXME add undo
      if (startup_material)
         startup_material->m_name = name;
   }
   ImGui::EndDisabled();
   ImGui::Separator();
   if (ImGui::CollapsingHeader("Visual", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      static const string matType[] = { "Default"s, "Metal"s };
      PropCombo("Type", m_table, is_live, startup_material ? (int *)&(startup_material->m_type) : nullptr, live_material ? (int *)&(live_material->m_type) : nullptr, std::size(matType), matType);
      if (material != nullptr)
      {
         PropRGB("Base Color", m_table, is_live, startup_material ? &(startup_material->m_cBase) : nullptr, live_material ? &(live_material->m_cBase) : nullptr);
         PropFloat("Wrap Lighting", m_table, is_live, startup_material ? &(startup_material->m_fWrapLighting) : nullptr, live_material ? &(live_material->m_fWrapLighting) : nullptr, 0.02f, 0.1f);
         if (material->m_type != Material::METAL)
         {
            PropRGB("Glossy Color", m_table, is_live, startup_material ? &(startup_material->m_cGlossy) : nullptr, live_material ? &(live_material->m_cGlossy) : nullptr);
            PropFloat("Glossy Image Lerp", m_table, is_live, startup_material ? &(startup_material->m_fGlossyImageLerp) : nullptr, live_material ? &(live_material->m_fGlossyImageLerp) : nullptr, 0.02f, 0.1f);
         }
         PropFloat("Shininess", m_table, is_live, startup_material ? &(startup_material->m_fRoughness) : nullptr, live_material ? &(live_material->m_fRoughness) : nullptr, 0.02f, 0.1f);
         PropRGB("Clearcoat Color", m_table, is_live, startup_material ? &(startup_material->m_cClearcoat) : nullptr, live_material ? &(live_material->m_cClearcoat) : nullptr);
         PropFloat("Edge Brightness", m_table, is_live, startup_material ? &(startup_material->m_fEdge) : nullptr, live_material ? &(live_material->m_fEdge) : nullptr, 0.02f, 0.1f);
      }
      ImGui::EndTable();
   }
   if (ImGui::CollapsingHeader("Transparency", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      PropCheckbox("Enable Transparency", m_table, is_live, startup_material ? &(startup_material->m_bOpacityActive) : nullptr, live_material ? &(live_material->m_bOpacityActive) : nullptr);
      PropFloat("Opacity", m_table, is_live, startup_material ? &(startup_material->m_fOpacity) : nullptr, live_material ? &(live_material->m_fOpacity) : nullptr, 0.02f, 0.1f);
      PropFloat("Edge Opacity", m_table, is_live, startup_material ? &(startup_material->m_fEdgeAlpha) : nullptr, live_material ? &(live_material->m_fEdgeAlpha) : nullptr, 0.02f, 0.1f);
      PropFloat("Thickness", m_table, is_live, startup_material ? &(startup_material->m_fThickness) : nullptr, live_material ? &(live_material->m_fThickness) : nullptr, 0.02f, 0.1f);
      PropRGB("Refraction Tint", m_table, is_live, startup_material ? &(startup_material->m_cRefractionTint) : nullptr, live_material ? &(live_material->m_cRefractionTint) : nullptr);
      ImGui::EndTable();
   }*/
}

}