#pragma once

#include "EditableUIPart.h"
#include "parts/image.h"

namespace VPX::EditorUI
{

class ImageUIPart : public EditableUIPart
{
public:
   ImageUIPart(Image* image);
   ~ImageUIPart() override;

   IEditable* GetEditable() const override { return m_image; }

   const string& GetOutlinerPath() const override { return m_outlinerPath; }
   void SetOutlinerPath(const string& path) override { m_outlinerPath = path; }

   void Render(const EditorRenderContext& ctx) override;
   
   void UpdatePropertyPane(PropertyPane& props) override;

private:
   Image* const m_image;
   string m_outlinerPath;
};

}