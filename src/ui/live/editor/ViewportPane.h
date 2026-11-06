// license:GPLv3+

#pragma once

#include "EditorAreaView.h"
#include "EditorUIPart.h" // ViewMode
#include "math/matrix.h"
#include "renderer/Renderer.h" // Renderer::ShadeMode

class RenderView;

namespace VPX::EditorUI
{

// 3D View pane: a leaf hosting a live render of the table. Each instance owns its own
// camera and rendering setup through a dedicated RenderView registered to the renderer
// while the leaf is shown, drawing straight to the pane's area of the shared backbuffer.
//
// The camera state of the pane receiving the interactions (the "edit" viewport) is bound
// into the editor's camera fields so that the existing camera code keeps working on it;
// on unbind it is stored back into the pane. Panes adopt the editor's camera the first
// time they are bound (split panes clone the active camera, like Blender).
class ViewportPane final : public EditorAreaView
{
public:
   explicit ViewportPane(EditorUI& editor);
   ~ViewportPane() override;

   AreaType GetType() const override { return AreaType::Viewport; }
   void RenderPane(const AreaRect& rect, int leafId) override;
   void OnUnhosted() override;

   RenderView* GetView() const { return m_view; }
   void SetRendering(const bool on)
   {
      if (m_view != nullptr)
         m_view->m_renderScene = on;
   }

   void CaptureCamera(); // Editor camera fields -> this pane's stored state (on unbind / unhost)
   void ReleaseCamera(); // This pane's stored state -> editor camera fields (on bind)

   int m_leafId = -1; // Leaf hosting this pane, set in RenderPane

   // Image interaction state, set in RenderPane and read by the editor for input routing
   bool m_hovered = false;
   bool m_active = false;
   AreaRect m_imageRect {}; // Display-space rect of the rendered image, matching the view's output area

   // Stored camera and render setup (while this pane is the edit viewport, the live state is in the editor fields)
   ViewMode m_camMode = ViewMode::EditorCam;
   bool m_perspectiveCam = true;
   Renderer::ShadeMode m_shadeMode = Renderer::ShadeMode::Default;
   float m_camAspect = 1.f; // Aspect ratio the stored projection was authored for, re-fitted on pane resize
   int m_predefinedView = 0; // EditorUI::PredefinedView (kept as int: the enum is private to the editor)
   bool m_orbitLock = false;
   Matrix3D m_camView, m_camProj;
   float m_camDistance = 1000.f;
   bool m_cameraValid = false;

private:
   RenderView* m_view = nullptr; // Owned through the renderer's extra views while set
};

}
