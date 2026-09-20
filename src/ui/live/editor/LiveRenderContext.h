// license:GPLv3+

#pragma once

#include "EditorUIPart.h"
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
   void DrawLine(const Vertex3Ds &a, const Vertex3Ds &b, ImU32 color) const override;
   void DrawCircle(const Vertex3Ds &center, const Vertex3Ds &x, const Vertex3Ds &y, float radius, ImU32 color) const override;
   void DrawHitObjects(IEditable *editable) const override;
   void DrawWireframe(IEditable *editable) const override;

   bool m_isSelected = false;
   bool m_isActive = false;

private:
   Player *m_player;
   ImDrawList *const m_drawlist;
   const ViewMode m_viewMode;
   const Renderer::ShadeMode m_shadeMode;
   const bool m_needsLiveTableSync;
};

}
