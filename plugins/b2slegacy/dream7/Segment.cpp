#include "../common.h"
#include <cmath>

#include "Segment.h"

namespace B2SLegacy {

Segment::Segment(const string& szName, float x, float y, float width, float height, float angle)
{
   InitSegment(szName, x, y, width, height, angle, SegmentCap_Standard, SegmentCap_Standard, 45);
}

Segment::Segment(const string& szName, float x, float y, float width, float height, float angle, SegmentCap topcap, SegmentCap bottomcap)
{
   InitSegment(szName, x, y, width, height, angle, topcap, bottomcap, 45);
}

Segment::Segment(const string& szName, float x, float y, float width, float height, float angle, SegmentCap topcap, SegmentCap bottomcap, float capangle)
{
   InitSegment(szName, x, y, width, height, angle, topcap, bottomcap, capangle);
}

Segment::Segment(float x, float y, float radius)
{
   InitSegmentDot(x, y, radius);
}

Segment::~Segment()
{
   delete m_pGlassPath;
   delete m_pLightPath;
   delete m_pLightBrush;
}

GraphicsPath* Segment::GetGlassPath()
{
   GetGlassData();
   return m_pGlassPath;
}

void Segment::AssignStyle()
{
   ResetCacheData();
}

void Segment::Draw(VPXGraphics* pRenderer)
{
   SetTransform(pRenderer);
   GetGlassData();
   if (m_on) {
      PathGradientBrush brush(m_pGlassPath);
      brush.SetCenterColor(m_pStyle->GetGlassColorCenter(), m_pStyle->GetGlassAlphaCenter());
      brush.SetSurroundColor(m_pStyle->GetGlassColor(), (uint8_t)m_pStyle->GetGlassAlpha());
      brush.SetFocusScales(m_focusScales);
      PaintSegment(pRenderer, brush, RGB(255, 0, 0), m_pGlassPath);
   }
   else {
      const SolidBrush brush(m_pStyle->GetOffColor());
      PaintSegment(pRenderer, brush, RGB(169, 169, 169), m_pGlassPath);
   }
   pRenderer->ResetTransform();
}

void Segment::DrawLight(VPXGraphics* pRenderer)
{
   if (!m_on)
      return;

   SetTransform(pRenderer);
   GetLightData();
   PaintSegment(pRenderer, *m_pLightBrush, RGB(255, 255, 0), m_pLightPath);
   pRenderer->ResetTransform();
}

void Segment::InitSegmentDot(float x, float y, float radius)
{
   m_szName = "."sv;
   m_points.emplace_back(radius, radius);
   m_radius = radius;

   CreateLightData();

   m_ownMatrix.Translate(x, y);
}

void Segment::InitSegment(const string& szName, float x, float y, float width, float height, float angle, SegmentCap topcap, SegmentCap bottomcap, float capangle)
{
   m_szName = szName;
   SDL_FRect nBounds = { 0, 0, width, height };
   float topleft;
   float topright;
   float topdelta;
   float bottomleft;
   float bottomright;
   float bottomdelta;
   LeftRightFromCap(topcap, width, capangle, topleft, topright, topdelta);
   LeftRightFromCap(bottomcap, width, capangle, bottomleft, bottomright, bottomdelta);

   m_points.emplace_back(nBounds.x + topdelta, nBounds.y);
   m_points.emplace_back(nBounds.x + nBounds.w, nBounds.y + (topcap == SegmentCap_Flat ? 0 : topright));
   m_points.emplace_back(nBounds.x + nBounds.w, nBounds.y + nBounds.h - (bottomcap == SegmentCap_Flat ? 0 : bottomright));
   m_points.emplace_back(nBounds.x + bottomdelta, nBounds.y + nBounds.h);
   m_points.emplace_back(nBounds.x, nBounds.y + nBounds.h - (bottomcap == SegmentCap_Flat ? 0 : bottomleft));
   m_points.emplace_back(nBounds.x, nBounds.y + (topcap == SegmentCap_Flat ? 0 : topleft));

   m_angle = angle;
   CreateLightData();

   m_ownMatrix.Translate(x, y);
   m_ownMatrix.Rotate(angle);
}

void Segment::CreateLightData()
{
   if (!m_pStyle || m_glow == m_pStyle->GetGlow())
      return;
   m_glow = m_pStyle->GetGlow();
   if (m_radius > 0) {
      // extra light for the dot
      m_lightDot = { m_points[0].x - m_glow, m_points[0].y - m_glow, m_radius + m_glow * 2, m_radius + m_glow * 2 };
   }
   else {
      m_lights.clear();
      m_lights.emplace_back(m_points[0].x, m_points[0].y - m_glow);
      m_lights.emplace_back(m_points[1].x + m_glow, m_points[1].y);
      m_lights.emplace_back(m_points[2].x + m_glow, m_points[2].y);
      m_lights.emplace_back(m_points[3].x, m_points[3].y + m_glow);
      m_lights.emplace_back(m_points[4].x - m_glow, m_points[4].y);
      m_lights.emplace_back(m_points[5].x - m_glow, m_points[5].y);
   }
}

void Segment::SetBulbSize()
{
   if (SDL_RectEmptyFloat(&m_pStyle->GetBulbSize()))
      m_focusScales = { 0.0f, 0.0f };
   else {
      if (m_angle == -90.0f)
         m_focusScales = { m_pStyle->GetBulbSize().h, m_pStyle->GetBulbSize().w };
      else if (m_angle == 0.0f)
         m_focusScales = { m_pStyle->GetBulbSize().w, m_pStyle->GetBulbSize().h };
      else {
         float nDiag = (m_pStyle->GetBulbSize().w + m_pStyle->GetBulbSize().h) / 2.0f;
         m_focusScales = { nDiag, nDiag };
      }
   }
}

void Segment::LeftRightFromCap(SegmentCap nCap, float nWidth, float nCapangle, float& nLeft, float& nRight, float& nDelta)
{
   switch (nCap) {
      case SegmentCap_Standard:
      case SegmentCap_Flat:
         nLeft = nWidth / 2.0f;
         nRight = nWidth / 2.0f;
         break;
      case SegmentCap_Left:
         nLeft = 0.0f;
         nRight = nWidth;
         break;
      case SegmentCap_Right:
         nLeft = nWidth;
         nRight = 0.0f;
         break;
      case SegmentCap_MoreLeft:
         nLeft = nWidth / 4.0f;
         nRight = nWidth * (float)(3.0 / 4.0);
         break;
      case SegmentCap_MoreRight:
         nLeft = nWidth * (float)(3.0 / 4.0);
         nRight = nWidth / 4.0f;
         break;
    }
    nDelta = nLeft;
    const float tmp = tanf(nCapangle * (float)(M_PI / 180.0));
    nLeft *= tmp;
    nRight *= tmp;
}

void Segment::PaintSegment(VPXGraphics* pRenderer, const Brush& pBrush, uint32_t penColor, const GraphicsPath* const __restrict pPath)
{
   if (m_pStyle->IsWireFrame()) {
      pRenderer->SetColor(penColor);
      pRenderer->DrawPath(*pPath);
   }
   else
      pRenderer->FillPath(pBrush, *pPath);
}

void Segment::GetGlassData()
{
   if (!m_pGlassPath) {
      m_pGlassPath = new GraphicsPath();
      if (m_radius > 0)
         m_pGlassPath->AddEllipse(m_points[0].x, m_points[0].y, m_radius, m_radius);
      else
         m_pGlassPath->AddPolygon(&m_points);
      SetBulbSize();
   }
}

void Segment::GetLightData()
{
   if (!m_pLightPath) {
      CreateLightData();
      m_pLightPath = new GraphicsPath();
      if (m_radius > 0)
         m_pLightPath->AddEllipse(m_lightDot);
      else {
         m_pLightPath->AddClosedCurve(&m_lights, 0.5);
      }
      SetBulbSize();
   }
   if (!m_pLightBrush) {
      m_pLightBrush = new PathGradientBrush(m_pLightPath);
      m_pLightBrush->SetSurroundColor(RGB(255, 255, 255), 0);
      m_pLightBrush->SetCenterColor(m_pStyle->GetLightColor(), 255);
      if (m_focusScales.x != 0.0f || m_focusScales.y != 0.0f)
         m_pLightBrush->SetFocusScales(m_focusScales);
   }
}

void Segment::ResetCacheData()
{
   delete m_pGlassPath;
   m_pGlassPath = nullptr;

   delete m_pLightPath;
   m_pLightPath = nullptr;

   delete m_pLightBrush;
   m_pLightBrush = nullptr;
}

void Segment::SetTransform(VPXGraphics* pRenderer)
{
   // m_externMatrix defaults to identity, so multiplying is a no-op until a parent transform is assigned via Transform()
   Matrix pMatrix = m_externMatrix;
   pMatrix.Multiply(m_ownMatrix);
   pRenderer->SetTransform(pMatrix);
}

void Segment::Transform(Matrix* pMatrix)
{
   m_externMatrix = *pMatrix;
}


}
