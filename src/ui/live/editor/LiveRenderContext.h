// license:GPLv3+

#pragma once

#include "parts/EditorUIPart.h"
#include "renderer/Renderer.h"

class Player;

namespace VPX::EditorUI
{

// EditorRenderContext implementation used by the live editor, both when contributing
// to the 3D scene (no draw list) and when drawing 2D overlays on an ImGui draw list.
class LiveRenderContext final : public EditorRenderContext
{
public:
   LiveRenderContext(Player *player, ImDrawList *drawlist, ViewMode viewMode, Renderer::ShadeMode shadeMode, bool needsLiveTableSync);
   ~LiveRenderContext() override = default;

   bool NeedsLiveTableSync() const override { return m_needsLiveTableSync; }
   ImU32 GetColor(bool selected) const override { return selected ? (m_isActive ? IM_COL32(255, 128, 0, 255) : IM_COL32(192, 96, 0, 255)) : IM_COL32_BLACK; };
   bool IsSelected() const override { return m_isSelected; }
   bool IsShowInvisible() const override;
   ViewMode GetViewMode() const override { return m_viewMode; }
   ImDrawList *GetDrawList() const override { return m_drawlist; }

   ImVec2 Project(const Vertex3Ds &v) const override;
   // Makes Project use the given view-projection matrix and map the result into a display-space
   // rect (viewport pane rendering): origin is the rect's top left in display px, scale converts
   // render px to display px, halfW/H is the render space half size
   void SetPaneProjection(const Matrix3D &mvp, const ImVec2 &origin, const ImVec2 &scale, float halfW, float halfH);
   void DrawLine(const Vertex3Ds &a, const Vertex3Ds &b, ImU32 color) const override;
   void DrawCircle(const Vertex3Ds &center, const Vertex3Ds &x, const Vertex3Ds &y, float radius, ImU32 color) const override;
   void DrawHitObjects(IEditable *editable) const override;
   void DrawWireframe(IEditable *editable) const override;

   bool m_isSelected = false;
   bool m_isActive = false;
   int m_selectionDisplay = 0; // 0 = edges only, 1 = edges and tint, 2 = hidden (Editor/NavSelectionDisplay)

private:
   Player *m_player;
   ImDrawList *const m_drawlist;
   const ViewMode m_viewMode;
   const Renderer::ShadeMode m_shadeMode;
   const bool m_needsLiveTableSync;
   Matrix3D m_paneMvp {}; // Pane projection override (see SetPaneProjection), used when m_hasPaneMvp
   bool m_hasPaneMvp = false;
   ImVec2 m_projOrigin { 0.f, 0.f };
   ImVec2 m_projScale { 1.f, 1.f };
   float m_projHalfW = 0.f, m_projHalfH = 0.f;
};

}
