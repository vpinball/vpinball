#pragma once

#include "EditableUIPart.h"
#include "parts/PartGroup.h"

namespace VPX::EditorUI
{

class PartGroupUIPart final : public EditableUIPart
{
public:
   explicit PartGroupUIPart(PartGroup* partGroup);
   ~PartGroupUIPart() override;

   IEditable* GetEditable() const override { return m_partGroup; }

   const string& GetOutlinerPath() const override { return m_outlinerPath; }
   void SetOutlinerPath(const string& path) override { m_outlinerPath = path; }

   TransformMask GetTransform(Matrix3D& transform) override;
   void SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) override;

   void Render(const EditorRenderContext& ctx) override;

   void UpdatePropertyPane(PropertyPane& props) override;

private:
   PartGroup* const m_partGroup;
   string m_outlinerPath;
};

}
