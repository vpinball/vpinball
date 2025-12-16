#pragma once

#include "EditableUIPart.h"
#include "parts/Material.h"

namespace VPX::EditorUI
{

class MaterialUIPart : public EditableUIPart
{
public:
   MaterialUIPart(Material* material);
   ~MaterialUIPart() override;

   IEditable* GetEditable() const override { return nullptr; }

   const string& GetOutlinerPath() const override { return m_outlinerPath; }
   void SetOutlinerPath(const string& path) override { m_outlinerPath = path; }

   void Render(const EditorRenderContext& ctx) override;
   
   void UpdatePropertyPane(PropertyPane& props) override;

private:
   Material* const m_material;
   string m_outlinerPath;
};

}