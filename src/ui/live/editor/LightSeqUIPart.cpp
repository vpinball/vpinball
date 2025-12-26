#pragma once

#include "core/stdafx.h"

#include "LightSeqUIPart.h"

namespace VPX::EditorUI
{

LightSeqUIPart::LightSeqUIPart(LightSeq* lightSeq)
   : m_lightSeq(lightSeq)
{
}

LightSeqUIPart::~LightSeqUIPart() { }

LightSeqUIPart::TransformMask LightSeqUIPart::GetTransform(Matrix3D& transform)
{
   return TM_None;
}

void LightSeqUIPart::SetTransform(const vec3& pos, const vec3& scale, const vec3& rot)
{ 
}

void LightSeqUIPart::Render(const EditorRenderContext& ctx) { }

void LightSeqUIPart::UpdatePropertyPane(PropertyPane& props)
{
   props.EditableHeader("LightSeq", m_lightSeq);
   
   if (props.BeginSection("Visual"s))
   {
      props.EndSection();
   }
   
   if (props.BeginSection("Position"s))
   {
      // Missing position
      props.EndSection();
   }
   
   props.TimerSection<LightSeq>(m_lightSeq, [](LightSeq* obj) { return &(obj->m_d.m_tdr); });
}

}