#pragma once

#include "EditableUIPart.h"
#include "parts/lightseq.h"

namespace VPX::EditorUI
{

class LightSeqUIPart : public EditableUIPart
{
public:
   explicit LightSeqUIPart(LightSeq* lightSeq);
   ~LightSeqUIPart() override;

   IEditable* GetEditable() const override { return m_lightSeq; }

   const string& GetOutlinerPath() const override { return m_outlinerPath; }
   void SetOutlinerPath(const string& path) override { m_outlinerPath = path; }

   TransformMask GetTransform(Matrix3D& transform) override;
   void SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) override;

   void Render(const EditorRenderContext& ctx) override;

   void UpdatePropertyPane(PropertyPane& props) override;

private:
   LightSeq* const m_lightSeq;
   string m_outlinerPath;
};

}