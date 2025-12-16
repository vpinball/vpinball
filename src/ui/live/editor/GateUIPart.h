#pragma once

#include "EditableUIPart.h"
#include "parts/gate.h"

namespace VPX::EditorUI
{

class GateUIPart : public EditableUIPart
{
public:
   explicit GateUIPart(Gate* gate);
   ~GateUIPart() override;

   IEditable* GetEditable() const override { return m_gate; }

   const string& GetOutlinerPath() const override { return m_outlinerPath; }
   void SetOutlinerPath(const string& path) override { m_outlinerPath = path; }

   TransformMask GetTransform(Matrix3D& transform) override;
   void SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) override;

   void Render(const EditorRenderContext& ctx) override;

   void UpdatePropertyPane(PropertyPane& props) override;

private:
   Gate* const m_gate;
   string m_outlinerPath;
   bool m_visible;
};

}