#pragma once

#include "EditorUIPart.h"
#include "parts/primitive.h"

namespace VPX::EditorUI
{

class PrimitiveUIPart final : public EditableUIPart<Primitive>
{
public:
   explicit PrimitiveUIPart(Primitive* primitive);

   TransformMask GetTransform(Matrix3D& transform) override;
   void SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) override;

   void RenderOverlay(const EditorRenderContext& ctx) override;

   void UpdatePropertyPane(PropertyPane& props) override;

private:
   void ImportMesh();
   void ExportMesh();

   bool m_meshUnitsMeters = false;
   std::weak_ptr<string> m_pendingMeshImport;
   std::weak_ptr<string> m_pendingMeshExport;
};

}
