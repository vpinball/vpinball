// license:GPLv3+

#pragma once

#include "AreaLayout.h"
#include "math/MeshUtils.h"
#include "math/vector.h"
#include "imgui/imgui.h"

class DragPoint;
class RenderDevice;
class Texture;
struct ImDrawList;

namespace VPX::EditorUI
{

class EditorUIPart;

// 2D drawing interface of the blueprint (top-down schematic) view: the live editor's
// counterpart of the win32 editor's Sur API (src/ui/win/sur.h, GDI bound and not reusable
// here). Parts draw their top view through these primitives once for the Fill pass (below
// the grid) and once for the Outline pass (above it), see EditorUIPart::RenderBlueprint.
// Two implementations drive the same part code:
// - BlueprintDrawContext: renders to an ImDrawList (the PaintSur counterpart)
// - BlueprintHitContext:  records hits for picking / box selection (HitSur / HitRectSur)
class BlueprintRenderContext
{
public:
   enum class Pass
   {
      Fill,
      Outline
   };
   virtual ~BlueprintRenderContext() = default;

   // Maps the canvas: zoom in display px per table unit, offset = table coordinate at the
   // rect center (same convention as the win32 view offset), dpi scales fixed sizes (pens, handles)
   void Setup(const AreaRect& rect, const float zoom, const Vertex2D& offset, const float dpi)
   {
      m_rect = rect;
      m_zoom = zoom;
      m_offset = offset;
      m_dpi = dpi;
   }

   ImVec2 SurfaceToScreen(const Vertex2D& v) const
   {
      return ImVec2(0.5f * (m_rect.x0 + m_rect.x1) + (v.x - m_offset.x) * m_zoom, //
         0.5f * (m_rect.y0 + m_rect.y1) + (v.y - m_offset.y) * m_zoom);
   }
   Vertex2D ScreenToSurface(const ImVec2& px) const
   {
      const float invZoom = (m_zoom != 0.f) ? (1.f / m_zoom) : 0.f;
      return Vertex2D(m_offset.x + (px.x - 0.5f * (m_rect.x0 + m_rect.x1)) * invZoom, //
         m_offset.y + (px.y - 0.5f * (m_rect.y0 + m_rect.y1)) * invZoom);
   }

   Pass GetPass() const { return m_pass; }
   void SetPass(const Pass pass) { m_pass = pass; }

   // Per-part state, set by the pane before each part's RenderBlueprint call
   bool m_partSelected = false;
   bool m_partActive = false; // Part is the active selection (stronger selection style)
   bool m_partLocked = false;
   const EditorUIPart* m_pointEditPart = nullptr; // Part in drag point edit mode (its points are always drawn)
   const vector<DragPoint*>* m_selectedPoints = nullptr; // Editor's point selection (point edit mode)
   const DragPoint* m_hotPoint = nullptr; // Hovered or dragged point (highlight)
   bool IsPointSelected(const DragPoint* point) const
   {
      return point != nullptr && (point == m_hotPoint || (m_selectedPoints != nullptr && std::ranges::find(*m_selectedPoints, point) != m_selectedPoints->end()));
   }

   // Pen state (win32 Sur semantics): rgb is a COLORREF (0x00BBGGRR), -1 selects no fill/pen
   virtual void SetFillColor(int rgb) const = 0;
   virtual void SetBorderColor(int rgb, bool dashed, int width) const = 0;
   virtual void SetLineColor(int rgb, bool dashed, int width) const = 0;
   // Marks which part the following primitives belong to: applies select/locked pen styling
   // (draw impl) or records hits to it (hit impl). point != nullptr marks a drag point handle.
   virtual void SetObject(const EditorUIPart* part, const DragPoint* point = nullptr) const = 0;

   // Table-space primitives (VP units)
   virtual void Line(float x, float y, float x2, float y2) const = 0;
   virtual void Rectangle(float x, float y, float x2, float y2) const = 0;
   virtual void Ellipse(float centerX, float centerY, float radius) const = 0;
   virtual void Polygon(const Vertex2D* rgv, int count) const = 0;
   virtual void Polygon(const vector<RenderVertex>& rgv) const = 0;
   virtual void PolygonImage(const vector<RenderVertex>& rgv, Texture* image, float left, float top, float right, float bottom) const = 0;
   virtual void Polyline(const Vertex2D* rgv, int count) const = 0;
   virtual void Lines(const Vertex2D* rgv, int count) const = 0;
   virtual void Arc(float x, float y, float radius, const Vertex2D& p1, const Vertex2D& p2) const = 0;
   virtual void Image(float x, float y, float x2, float y2, Texture* image) const = 0;

   // Screen-space primitives (sizes in display px, positions in table units except Rectangle2)
   virtual void Ellipse2(float centerX, float centerY, float radius) const = 0;
   virtual void Rectangle2(float x, float y, float x2, float y2) const = 0; // Full display coordinates

   // Selection styling colors (win32 editor settings), shared by the two implementations
   static int SelectColor();
   static int SelectLockedColor();

protected:
   AreaRect m_rect {};
   float m_zoom = 1.f;
   Vertex2D m_offset {};
   float m_dpi = 1.f;
   Pass m_pass = Pass::Fill;
};


// ImDrawList implementation (the PaintSur counterpart): draws through the pane's window
// draw list with the pen state translated to ImU32 colors. Textures come from the render
// device's texture manager (ImTextureID is a shared_ptr<Sampler> in this codebase).
class BlueprintDrawContext final : public BlueprintRenderContext
{
public:
   BlueprintDrawContext(RenderDevice* renderDevice, ImDrawList* drawList)
      : m_rd(renderDevice)
      , m_drawList(drawList)
   {
   }

   void SetFillColor(int rgb) const override;
   void SetBorderColor(int rgb, bool dashed, int width) const override;
   void SetLineColor(int rgb, bool dashed, int width) const override;
   void SetObject(const EditorUIPart* part, const DragPoint* point = nullptr) const override;

   void Line(float x, float y, float x2, float y2) const override;
   void Rectangle(float x, float y, float x2, float y2) const override;
   void Ellipse(float centerX, float centerY, float radius) const override;
   void Polygon(const Vertex2D* rgv, int count) const override;
   void Polygon(const vector<RenderVertex>& rgv) const override;
   void PolygonImage(const vector<RenderVertex>& rgv, Texture* image, float left, float top, float right, float bottom) const override;
   void Polyline(const Vertex2D* rgv, int count) const override;
   void Lines(const Vertex2D* rgv, int count) const override;
   void Arc(float x, float y, float radius, const Vertex2D& p1, const Vertex2D& p2) const override;
   void Image(float x, float y, float x2, float y2, Texture* image) const override;
   void Ellipse2(float centerX, float centerY, float radius) const override;
   void Rectangle2(float x, float y, float x2, float y2) const override;

private:
   static ImU32 ToImU32(const int rgb) { return IM_COL32(rgb & 0xFF, (rgb >> 8) & 0xFF, (rgb >> 16) & 0xFF, 255); }
   void AddLine(const ImVec2& a, const ImVec2& b) const; // line pen honoring the dash flag
   void FillPolygon(const vector<RenderVertex>& rgv, const vector<ImVec2>& pts) const; // solid fill of a part outline (pts = screen-space rgv)
   ImTextureID TextureSampler(Texture* image) const;

   RenderDevice* const m_rd;
   ImDrawList* const m_drawList;

   mutable ImU32 m_fillColor = 0;
   mutable bool m_hasFill = false;
   mutable ImU32 m_borderColor = 0;
   mutable float m_borderWidth = 1.f;
   mutable bool m_borderDashed = false;
   mutable ImU32 m_lineColor = 0;
   mutable float m_lineWidth = 1.f;
   mutable bool m_lineDashed = false;
};


// Hit recording implementation (the HitSur / HitRectSur counterpart): the same part code is
// replayed and its primitives are tested instead of drawn. Point mode collects the parts
// under a screen point in draw order (topmost last, for click cycling); rect mode collects
// the parts whose drawn shapes are fully inside a screen rect (box select).
class BlueprintHitContext final : public BlueprintRenderContext
{
public:
   struct Hit
   {
      const EditorUIPart* part;
      const DragPoint* point; // Drag point handle hit (nullptr for part shapes)
      bool operator==(const Hit&) const = default;
   };

   void SetHitPoint(const ImVec2& px)
   {
      m_hitPt = px;
      m_rectMode = false;
   }
   void SetHitRect(const AreaRect& rect)
   {
      m_hitRect = rect;
      m_rectMode = true;
      m_rectInside.clear();
      m_rectPointInside.clear();
   }

   const vector<Hit>& GetHits() const { return m_hits; }
   const vector<const EditorUIPart*>& GetRectParts() const;
   const vector<const DragPoint*>& GetRectPoints() const;

   void SetFillColor(int rgb) const override { }
   void SetBorderColor(int rgb, bool dashed, int width) const override { }
   void SetLineColor(int rgb, bool dashed, int width) const override { }
   void SetObject(const EditorUIPart* part, const DragPoint* point = nullptr) const override;

   void Line(float x, float y, float x2, float y2) const override;
   void Rectangle(float x, float y, float x2, float y2) const override;
   void Ellipse(float centerX, float centerY, float radius) const override;
   void Polygon(const Vertex2D* rgv, int count) const override;
   void Polygon(const vector<RenderVertex>& rgv) const override;
   void PolygonImage(const vector<RenderVertex>& rgv, Texture* image, float left, float top, float right, float bottom) const override;
   void Polyline(const Vertex2D* rgv, int count) const override;
   void Lines(const Vertex2D* rgv, int count) const override;
   void Arc(float x, float y, float radius, const Vertex2D& p1, const Vertex2D& p2) const override { } // No-op like HitSur
   void Image(float x, float y, float x2, float y2, Texture* image) const override { }
   void Ellipse2(float centerX, float centerY, float radius) const override;
   void Rectangle2(float x, float y, float x2, float y2) const override { } // Screen decoration, not hit

private:
   void HitSegment(const ImVec2& a, const ImVec2& b) const;
   void HitPoly(const Vertex2D* rgv, int count) const;
   void HitDisc(const ImVec2& center, float radiusPx) const;
   void Record() const; // Point mode: registers (m_cur, m_curPoint), moved to back on repeat (topmost wins)
   void Inside(const bool inside) const; // Rect mode bookkeeping (FailObject semantics)
   bool PtInRect(const ImVec2& p) const;
   bool PtInPolygon(const Vertex2D* rgv, int count) const; // Ray crossing test on the screen point

   ImVec2 m_hitPt {};
   AreaRect m_hitRect {};
   bool m_rectMode = false;
   mutable const EditorUIPart* m_cur = nullptr;
   mutable const DragPoint* m_curPoint = nullptr;
   mutable vector<Hit> m_hits;
   // Rect mode: entry per part currently recorded (true while all its shapes stay inside)
   mutable std::unordered_map<const EditorUIPart*, bool> m_rectInside;
   mutable std::unordered_map<const DragPoint*, bool> m_rectPointInside;
   mutable vector<const EditorUIPart*> m_rectParts;
   mutable vector<const DragPoint*> m_rectPoints;
};

}
