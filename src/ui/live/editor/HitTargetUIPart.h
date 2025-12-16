#pragma once

#include "EditableUIPart.h"
#include "parts/hittarget.h"

namespace VPX::EditorUI
{

class HitTargetUIPart : public EditableUIPart
{
public:
   explicit HitTargetUIPart(HitTarget* hitTarget);
   ~HitTargetUIPart() override;

   IEditable* GetEditable() const override { return m_hitTarget; }

   const string& GetOutlinerPath() const override { return m_outlinerPath; }
   void SetOutlinerPath(const string& path) override { m_outlinerPath = path; }

   TransformMask GetTransform(Matrix3D& transform) override;
   void SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) override;

   void Render(const EditorRenderContext& ctx) override;

   void UpdatePropertyPane(PropertyPane& props) override;

private:
   HitTarget* const m_hitTarget;
   string m_outlinerPath;
};

}