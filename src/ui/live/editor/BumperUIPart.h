#pragma once

#include "EditableUIPart.h"
#include "parts/bumper.h"

namespace VPX::EditorUI
{

class BumperUIPart : public EditableUIPart
{
public:
   explicit BumperUIPart(Bumper* bumper);
   ~BumperUIPart() override;

   IEditable* GetEditable() const override { return m_bumper; }

   const string& GetOutlinerPath() const override { return m_outlinerPath; }
   void SetOutlinerPath(const string& path) override { m_outlinerPath = path; }

   TransformMask GetTransform(Matrix3D& transform) override;
   void SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) override;

   void Render(const EditorRenderContext& ctx) override;

   void UpdatePropertyPane(PropertyPane& props) override;

private:
   Bumper* const m_bumper;
   string m_outlinerPath;
   bool m_baseVisible;
   bool m_capVisible;
   bool m_skirtVisible;
   bool m_ringVisible;
};

}