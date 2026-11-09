// license:GPLv3+

#include "core/stdafx.h"
#include "BlueprintRenderContext.h"

#include "core/Settings.h"
#include "renderer/Texture.h"
#include "renderer/RenderDevice.h"
#include "renderer/TextureManager.h"
#include "utils/color.h"

namespace VPX::EditorUI
{

int BlueprintRenderContext::SelectColor() { return (int)g_settingsService.GetAppSettings().GetEditor_ElementSelectColor(); }

int BlueprintRenderContext::SelectLockedColor() { return (int)g_settingsService.GetAppSettings().GetEditor_ElementSelectLockedColor(); }

// ---------------------------------------------------------------------------------------------------------------------
// Draw context (PaintSur counterpart)

void BlueprintDrawContext::SetFillColor(const int rgb) const
{
   m_hasFill = rgb != -1;
   m_fillColor = m_hasFill ? ToImU32(rgb) : 0;
}

void BlueprintDrawContext::SetBorderColor(const int rgb, const bool dashed, const int width) const
{
   m_borderColor = (rgb != -1) ? ToImU32(rgb) : 0;
   m_borderWidth = (rgb != -1) ? max(1.f, (float)width) * m_dpi : 0.f;
   m_borderDashed = dashed;
}

void BlueprintDrawContext::SetLineColor(const int rgb, const bool dashed, const int width) const
{
   m_lineColor = (rgb != -1) ? ToImU32(rgb) : 0;
   m_lineWidth = (rgb != -1) ? max(1.f, (float)width) * m_dpi : 0.f;
   m_lineDashed = dashed;
}

void BlueprintDrawContext::SetObject(const EditorUIPart* part, const DragPoint* point) const
{
   if (part == nullptr)
      return;
   if (point != nullptr)
   {
      // Drag point handle styling (win32 PaintSur: selected points get a bright fill)
      m_hasFill = IsPointSelected(point);
      m_fillColor = ToImU32(RGB(150, 200, 255));
      return;
   }
   if (m_partLocked)
   {
      // Locked parts use the locked color; a selected one gets the thicker pen (win32
      // PaintSur: 4 for the active selection, 3 for multi-selected, 1 when unselected)
      const ImU32 color = ToImU32(SelectLockedColor());
      m_borderColor = color;
      m_lineColor = color;
      m_borderWidth = m_lineWidth = (m_partSelected ? (m_partActive ? 4.f : 3.f) : 1.f) * m_dpi;
      m_borderDashed = m_lineDashed = false;
   }
   else if (m_partSelected)
   {
      const ImU32 color = ToImU32(SelectColor());
      m_borderColor = color;
      m_lineColor = color;
      m_borderWidth = m_lineWidth = (m_partActive ? 4.f : 3.f) * m_dpi;
      m_borderDashed = m_lineDashed = false;
   }
}

void BlueprintDrawContext::AddLine(const ImVec2& a, const ImVec2& b) const
{
   if (m_lineWidth <= 0.f)
      return;
   if (!m_lineDashed)
   {
      m_drawList->AddLine(a, b, m_lineColor, m_lineWidth);
      return;
   }
   // Emulate the win32 PS_DOT pen: ~4px dashes with ~4px gaps
   const ImVec2 d(b.x - a.x, b.y - a.y);
   const float len = sqrtf(d.x * d.x + d.y * d.y);
   const float step = 8.f * m_dpi;
   for (float t = 0.f; t < len; t += 2.f * step)
   {
      const float t1 = min(t + step, len);
      m_drawList->AddLine(ImVec2(a.x + d.x * t / len, a.y + d.y * t / len), ImVec2(a.x + d.x * t1 / len, a.y + d.y * t1 / len), m_lineColor, m_lineWidth);
   }
}

void BlueprintDrawContext::Line(const float x, const float y, const float x2, const float y2) const { AddLine(SurfaceToScreen(Vertex2D(x, y)), SurfaceToScreen(Vertex2D(x2, y2))); }

void BlueprintDrawContext::Rectangle(const float x, const float y, const float x2, const float y2) const
{
   const ImVec2 a = SurfaceToScreen(Vertex2D(x, y)), b = SurfaceToScreen(Vertex2D(x2, y2));
   if (m_hasFill)
      m_drawList->AddRectFilled(a, b, m_fillColor);
   if (m_borderWidth > 0.f)
      m_drawList->AddRect(a, b, m_borderColor, 0.f, 0, m_borderWidth);
}

void BlueprintDrawContext::Ellipse(const float centerX, const float centerY, const float radius) const
{
   const ImVec2 c = SurfaceToScreen(Vertex2D(centerX, centerY));
   const float r = radius * m_zoom;
   if (m_hasFill)
      m_drawList->AddCircleFilled(c, r, m_fillColor);
   if (m_borderWidth > 0.f)
      m_drawList->AddCircle(c, r, m_borderColor, 0, m_borderWidth);
}

// Solid fill of a part outline. The win32 view fills through GDI's even-odd rule, which
// leaves the holes of self-overlapping outlines (donut shaped parts, walls with holes,
// closed loop rubbers/ramps, ...) empty; ImGui's AddConcavePolyFilled only supports
// simple contours and fills them. The triangulation of the part render meshes handles
// these outlines, so fill with the triangles it produces.
void BlueprintDrawContext::FillPolygon(const vector<RenderVertex>& rgv, const vector<ImVec2>& pts) const
{
   vector<unsigned int> poly(rgv.size()), tris;
   for (size_t i = 0; i < rgv.size(); ++i)
      poly[i] = (unsigned int)i;
   PolygonToTriangles(rgv, poly, tris, true);
   if (tris.empty())
   {
      // Outline too degenerate to triangulate: keep ImGui's fill rather than drawing nothing
      m_drawList->AddConcavePolyFilled(pts.data(), (int)pts.size(), m_fillColor);
      return;
   }
   const ImVec2 uv = ImGui::GetFontTexUvWhitePixel();
   m_drawList->PrimReserve((int)tris.size(), (int)tris.size());
   for (const unsigned int idx : tris)
   {
      m_drawList->PrimWriteIdx((ImDrawIdx)m_drawList->_VtxCurrentIdx);
      m_drawList->PrimWriteVtx(pts[idx], uv, m_fillColor);
   }
}

void BlueprintDrawContext::Polygon(const Vertex2D* const rgv, const int count) const
{
   if (rgv == nullptr || count < 3)
      return;
   vector<ImVec2> pts(count);
   for (int i = 0; i < count; ++i)
      pts[i] = SurfaceToScreen(rgv[i]);
   if (m_hasFill)
   {
      vector<RenderVertex> rv(count);
      for (int i = 0; i < count; ++i)
      {
         rv[i].x = rgv[i].x;
         rv[i].y = rgv[i].y;
      }
      FillPolygon(rv, pts);
   }
   if (m_borderWidth > 0.f)
   {
      m_drawList->AddPolyline(pts.data(), count, m_borderColor, m_borderWidth, ImDrawFlags_Closed);
      if (count == 4) // win32 quirk: 4-vertex polygons also get the X diagonals
      {
         m_drawList->AddLine(pts[0], pts[2], m_borderColor, m_borderWidth);
         m_drawList->AddLine(pts[1], pts[3], m_borderColor, m_borderWidth);
      }
   }
}

void BlueprintDrawContext::Polygon(const vector<RenderVertex>& rgv) const
{
   if (rgv.size() < 3)
      return;
   vector<ImVec2> pts(rgv.size());
   for (size_t i = 0; i < rgv.size(); ++i)
      pts[i] = SurfaceToScreen(Vertex2D(rgv[i].x, rgv[i].y));
   if (m_hasFill)
      FillPolygon(rgv, pts);
   if (m_borderWidth > 0.f)
   {
      m_drawList->AddPolyline(pts.data(), (int)pts.size(), m_borderColor, m_borderWidth, ImDrawFlags_Closed);
      if (rgv.size() == 4)
      {
         m_drawList->AddLine(pts[0], pts[2], m_borderColor, m_borderWidth);
         m_drawList->AddLine(pts[1], pts[3], m_borderColor, m_borderWidth);
      }
   }
}

ImTextureID BlueprintDrawContext::TextureSampler(Texture* const image) const { return image != nullptr ? m_rd->m_texMan.LoadTexture(image, false) : nullptr; }

void BlueprintDrawContext::PolygonImage(const vector<RenderVertex>& rgv, Texture* const image, const float left, const float top, const float right, const float bottom) const
{
   const ImTextureID sampler = TextureSampler(image);
   if (sampler == nullptr || rgv.size() < 3 || right == left || bottom == top)
   {
      Polygon(rgv);
      return;
   }
   vector<unsigned int> poly(rgv.size()), tris;
   for (size_t i = 0; i < rgv.size(); ++i)
      poly[i] = (unsigned int)i;
   PolygonToTriangles(rgv, poly, tris, true);
   if (tris.empty())
   {
      Polygon(rgv);
      return;
   }
   const float invW = 1.f / (right - left), invH = 1.f / (bottom - top);
   m_drawList->PushTexture(ImTextureRef(sampler));
   m_drawList->PrimReserve((int)tris.size(), (int)tris.size());
   for (const unsigned int idx : tris)
   {
      const ImVec2 p = SurfaceToScreen(Vertex2D(rgv[idx].x, rgv[idx].y));
      m_drawList->PrimWriteIdx((ImDrawIdx)m_drawList->_VtxCurrentIdx);
      m_drawList->PrimWriteVtx(p, ImVec2((rgv[idx].x - left) * invW, (rgv[idx].y - top) * invH), IM_COL32_WHITE);
   }
   m_drawList->PopTexture();
}

void BlueprintDrawContext::Polyline(const Vertex2D* const rgv, const int count) const
{
   if (rgv == nullptr || count < 2 || m_lineWidth <= 0.f)
      return;
   vector<ImVec2> pts(count);
   for (int i = 0; i < count; ++i)
      pts[i] = SurfaceToScreen(rgv[i]);
   m_drawList->AddPolyline(pts.data(), count, m_lineColor, m_lineWidth, 0);
}

void BlueprintDrawContext::Lines(const Vertex2D* const rgv, const int count) const
{
   if (rgv == nullptr || count < 1 || m_lineWidth <= 0.f)
      return;
   for (int i = 0; i < count * 2 - 1; i += 2)
      AddLine(SurfaceToScreen(rgv[i]), SurfaceToScreen(rgv[i + 1]));
}

void BlueprintDrawContext::Arc(const float x, const float y, const float radius, const Vertex2D& p1, const Vertex2D& p2) const
{
   if (m_lineWidth <= 0.f || radius <= 0.f)
      return;
   // GDI Arc: counterclockwise from the p1 ray to the p2 ray. With y-down screen space the
   // covered angle range is [a1 - span, a1] with span = a1 - a2 normalized to [0, 2pi)
   const float a1 = atan2f(p1.y - y, p1.x - x);
   const float a2 = atan2f(p2.y - y, p2.x - x);
   float span = a1 - a2;
   while (span < 0.f)
      span += 2.f * (float)M_PI;
   while (span >= 2.f * (float)M_PI)
      span -= 2.f * (float)M_PI;
   const ImVec2 c = SurfaceToScreen(Vertex2D(x, y));
   const float r = radius * m_zoom;
   const int n = max(4, (int)(span * r / 4.f) + 1);
   for (int i = 0; i < n; ++i)
   {
      const float t0 = a1 - span + span * (float)i / (float)n;
      const float t1 = a1 - span + span * (float)(i + 1) / (float)n;
      m_drawList->AddLine(ImVec2(c.x + r * cosf(t0), c.y + r * sinf(t0)), ImVec2(c.x + r * cosf(t1), c.y + r * sinf(t1)), m_lineColor, m_lineWidth);
   }
}

void BlueprintDrawContext::Image(const float x, const float y, const float x2, const float y2, Texture* const image) const
{
   const ImTextureID sampler = TextureSampler(image);
   if (sampler == nullptr)
      return;
   m_drawList->AddImage(ImTextureRef(sampler), SurfaceToScreen(Vertex2D(x, y)), SurfaceToScreen(Vertex2D(x2, y2)));
}

void BlueprintDrawContext::Ellipse2(const float centerX, const float centerY, const float radius) const
{
   const ImVec2 c = SurfaceToScreen(Vertex2D(centerX, centerY));
   const float r = radius * m_dpi;
   if (m_hasFill)
      m_drawList->AddCircleFilled(c, r, m_fillColor);
   if (m_borderWidth > 0.f)
      m_drawList->AddCircle(c, r, m_borderColor, 0, m_borderWidth);
}

void BlueprintDrawContext::Rectangle2(const float x, const float y, const float x2, const float y2) const
{
   if (m_hasFill)
      m_drawList->AddRectFilled(ImVec2(x, y), ImVec2(x2, y2), m_fillColor);
   if (m_borderWidth > 0.f)
      m_drawList->AddRect(ImVec2(x, y), ImVec2(x2, y2), m_borderColor, 0.f, 0, m_borderWidth);
}

// ---------------------------------------------------------------------------------------------------------------------
// Hit context (HitSur / HitRectSur counterpart)

const vector<const EditorUIPart*>& BlueprintHitContext::GetRectParts() const
{
   m_rectParts.clear();
   for (const auto& [part, inside] : m_rectInside)
      if (inside)
         m_rectParts.push_back(part);
   return m_rectParts;
}

const vector<const DragPoint*>& BlueprintHitContext::GetRectPoints() const
{
   m_rectPoints.clear();
   for (const auto& [point, inside] : m_rectPointInside)
      if (inside)
         m_rectPoints.push_back(point);
   return m_rectPoints;
}

void BlueprintHitContext::SetObject(const EditorUIPart* part, const DragPoint* point) const
{
   m_cur = part;
   m_curPoint = point;
   if (part != nullptr && m_rectInside.find(part) == m_rectInside.end())
      m_rectInside[part] = true;
   if (point != nullptr && m_rectPointInside.find(point) == m_rectPointInside.end())
      m_rectPointInside[point] = true;
}

void BlueprintHitContext::Record() const
{
   if (m_cur == nullptr)
      return;
   const Hit hit { m_cur, m_curPoint };
   std::erase(m_hits, hit);
   m_hits.push_back(hit);
}

void BlueprintHitContext::Inside(const bool inside) const
{
   if (inside)
      return;
   if (m_cur != nullptr)
      m_rectInside[m_cur] = false;
   if (m_curPoint != nullptr)
      m_rectPointInside[m_curPoint] = false;
}

bool BlueprintHitContext::PtInRect(const ImVec2& p) const { return p.x >= m_hitRect.x0 && p.x <= m_hitRect.x1 && p.y >= m_hitRect.y0 && p.y <= m_hitRect.y1; }

bool BlueprintHitContext::PtInPolygon(const Vertex2D* const rgv, const int count) const
{
   // HitSur's ray crossing test (screen space)
   ImVec2 p1 = SurfaceToScreen(rgv[count - 1]);
   bool hx1 = m_hitPt.x >= p1.x, hy1 = m_hitPt.y > p1.y;
   int crosscount = 0;
   for (int i = 0; i < count; ++i)
   {
      const float x2 = p1.x, y2 = p1.y;
      const bool hx2 = hx1, hy2 = hy1;
      p1 = SurfaceToScreen(rgv[i]);
      hx1 = m_hitPt.x >= p1.x;
      hy1 = m_hitPt.y > p1.y;
      if ((p1.y == y2) || (!hy1 && !hy2) || (hy1 && hy2) || (hx1 && hx2))
         continue;
      if (!hx1 && !hx2)
      {
         crosscount ^= 1;
         continue;
      }
      if (x2 == p1.x)
      {
         if (!hx2)
            crosscount ^= 1;
         continue;
      }
      if (x2 - (y2 - m_hitPt.y) * (p1.x - x2) / (p1.y - y2) > m_hitPt.x)
         crosscount ^= 1;
   }
   return (crosscount & 1) != 0;
}

void BlueprintHitContext::HitSegment(const ImVec2& a, const ImVec2& b) const
{
   // Distance from point to segment within a 4px window (HitSur semantics)
   const ImVec2 d(b.x - a.x, b.y - a.y);
   const float len2 = d.x * d.x + d.y * d.y;
   float t = 0.f;
   if (len2 > 1e-6f)
      t = clamp(((m_hitPt.x - a.x) * d.x + (m_hitPt.y - a.y) * d.y) / len2, 0.f, 1.f);
   const float dx = m_hitPt.x - (a.x + t * d.x);
   const float dy = m_hitPt.y - (a.y + t * d.y);
   if (dx * dx + dy * dy <= 16.f)
      Record();
}

void BlueprintHitContext::HitPoly(const Vertex2D* const rgv, const int count) const
{
   if (m_cur == nullptr || rgv == nullptr || count < 3)
      return;
   if (m_rectMode)
   {
      bool inside = true;
      for (int i = 0; i < count && inside; ++i)
         inside = PtInRect(SurfaceToScreen(rgv[i]));
      Inside(inside);
   }
   else if (PtInPolygon(rgv, count))
      Record();
}

void BlueprintHitContext::HitDisc(const ImVec2& center, const float radiusPx) const
{
   if (m_cur == nullptr)
      return;
   if (m_rectMode)
   {
      const bool inside = PtInRect(ImVec2(center.x - radiusPx, center.y - radiusPx)) && PtInRect(ImVec2(center.x + radiusPx, center.y + radiusPx));
      Inside(inside);
   }
   else
   {
      const float dx = m_hitPt.x - center.x, dy = m_hitPt.y - center.y;
      if (dx * dx + dy * dy <= radiusPx * radiusPx)
         Record();
   }
}

void BlueprintHitContext::Line(const float x, const float y, const float x2, const float y2) const
{
   if (m_cur == nullptr)
      return;
   const ImVec2 a = SurfaceToScreen(Vertex2D(x, y)), b = SurfaceToScreen(Vertex2D(x2, y2));
   if (m_rectMode)
      Inside(PtInRect(a) && PtInRect(b));
   else
      HitSegment(a, b);
}

void BlueprintHitContext::Rectangle(const float x, const float y, const float x2, const float y2) const
{
   if (m_cur == nullptr)
      return;
   const Vertex2D rgv[4] = { Vertex2D(x, y), Vertex2D(x2, y), Vertex2D(x2, y2), Vertex2D(x, y2) };
   HitPoly(rgv, 4);
}

void BlueprintHitContext::Ellipse(const float centerX, const float centerY, const float radius) const { HitDisc(SurfaceToScreen(Vertex2D(centerX, centerY)), radius * m_zoom); }

void BlueprintHitContext::Polygon(const Vertex2D* const rgv, const int count) const { HitPoly(rgv, count); }

void BlueprintHitContext::Polygon(const vector<RenderVertex>& rgv) const
{
   if (rgv.empty())
      return;
   vector<Vertex2D> pts(rgv.size());
   for (size_t i = 0; i < rgv.size(); ++i)
      pts[i] = Vertex2D(rgv[i].x, rgv[i].y);
   HitPoly(pts.data(), (int)pts.size());
}

void BlueprintHitContext::PolygonImage(const vector<RenderVertex>& rgv, Texture*, const float, const float, const float, const float) const { Polygon(rgv); }

void BlueprintHitContext::Polyline(const Vertex2D* const rgv, const int count) const
{
   if (m_cur == nullptr || rgv == nullptr || count < 2)
      return;
   if (m_rectMode)
   {
      bool inside = true;
      for (int i = 0; i < count && inside; ++i)
         inside = PtInRect(SurfaceToScreen(rgv[i]));
      Inside(inside);
   }
   else
      for (int i = 0; i + 1 < count; ++i)
         HitSegment(SurfaceToScreen(rgv[i]), SurfaceToScreen(rgv[i + 1]));
}

void BlueprintHitContext::Lines(const Vertex2D* const rgv, const int count) const
{
   if (m_cur == nullptr || rgv == nullptr || count < 1)
      return;
   if (m_rectMode)
   {
      bool inside = true;
      for (int i = 0; i < count * 2 && inside; ++i)
         inside = PtInRect(SurfaceToScreen(rgv[i]));
      Inside(inside);
   }
   else
      for (int i = 0; i < count * 2 - 1; i += 2)
         HitSegment(SurfaceToScreen(rgv[i]), SurfaceToScreen(rgv[i + 1]));
}

void BlueprintHitContext::Ellipse2(const float centerX, const float centerY, const float radius) const { HitDisc(SurfaceToScreen(Vertex2D(centerX, centerY)), radius * m_dpi); }

}
