// license:GPLv3+

#pragma once

#include <optional>

class Material;
class Texture;
class RenderProbe;
namespace VPX
{
class Sound;
struct SoundSpec;
}

namespace VPX::EditorUI
{

class EditorUI;
class PropertyPane;

// Right side panel displaying the properties of the current selection (table,
// camera view setup, image, material, render probe, sound or editable part).
class PropertiesPanel
{
public:
   explicit PropertiesPanel(EditorUI &editor)
      : m_editor(editor)
   {
   }

   void Render(float topBarHeight);

   // Width of the panel in logical pixels (scaled by the UI DPI)
   static constexpr float PaneWidth = 280.f;

private:
   void TableProperties(PropertyPane &props);
   void ImageProperties(PropertyPane &props, Texture *image);
   void RenderProbeProperties(PropertyPane &props, RenderProbe *probe);
   void CameraProperties(PropertyPane &props, int bgSet);
   void MaterialProperties(PropertyPane &props, Material *material);
   void SoundProperties(PropertyPane &props, VPX::Sound *sound);

   EditorUI &m_editor;
   bool m_selectLiveTab = true;

   // Sound preview state: the sound whose playback was started from the pane, and whether
   // playback was actually observed (PlaySound commands are dispatched asynchronously)
   VPX::Sound *m_playingSound = nullptr;
   bool m_playingSoundObserved = false;
   VPX::Sound *m_soundInfoFor = nullptr;
   std::optional<VPX::SoundSpec> m_soundInfo;
};

}
