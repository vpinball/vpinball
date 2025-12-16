#pragma once

#include "EditableUIPart.h"
#include "parts/ball.h"

namespace VPX::EditorUI
{

class BallUIPart : public EditableUIPart
{
public:
   BallUIPart(Ball* ball);
   ~BallUIPart() override;

   IEditable* GetEditable() const override { return m_ball; }

   const string& GetOutlinerPath() const override { return m_outlinerPath; }
   void SetOutlinerPath(const string& path) override { m_outlinerPath = path; }

   TransformMask GetTransform(Matrix3D& transform) override;
   void SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) override;

   void Render(const EditorRenderContext& ctx) override;
   
   void UpdatePropertyPane(PropertyPane& props) override;

private:
   Ball* const m_ball;
   string m_outlinerPath;
   bool m_visible;
};

}