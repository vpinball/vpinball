// license:GPLv3+

#pragma once

class Material;
class Texture;
class RenderProbe;

namespace VPX::EditorUI
{

class EditorUI;
class PropertyPane;

// Right side panel displaying the properties of the current selection (table,
// camera view setup, image, material, render probe or editable part).
class PropertiesPanel
{
public:
   explicit PropertiesPanel(EditorUI &editor)
      : m_editor(editor)
   {
   }

   void Render(float topBarHeight);

private:
   void TableProperties(PropertyPane &props);
   void ImageProperties(PropertyPane &props, Texture *image);
   void RenderProbeProperties(PropertyPane &props, RenderProbe *probe);
   void CameraProperties(PropertyPane &props, int bgSet);
   void MaterialProperties(PropertyPane &props, Material *material);

   EditorUI &m_editor;
   bool m_selectLiveTab = true;
};

}
