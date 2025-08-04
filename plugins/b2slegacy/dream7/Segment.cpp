#include "../common.h"
#include <cmath>

#include "Segment.h"

namespace B2SLegacy {

Segment::Segment(const string& szName, float x, float y, float width, float height, float angle)
{
   m_on = false;
   m_glow = -1.0f;
   m_radius = 0.0f;
   m_pStyle = NULL;
   m_pGlassPath = NULL;
   m_pLightPath = NULL;
   m_pExternMatrix = NULL;
   m_pOwnMatrix = NULL;

   InitSegment(szName, x, y, width, height, angle, SegmentCap_Standard, SegmentCap_Standard, 45);
}

Segment::Segment(const string& szName, float x, float y, float width, float height, float angle, SegmentCap topcap, SegmentCap bottomcap)
{
   m_on = false;
   m_glow = -1.0f;
   m_radius = 0.0f;
   m_pStyle = NULL;
   m_pGlassPath = NULL;
   m_pLightPath = NULL;
   m_pExternMatrix = NULL;
   m_pOwnMatrix = NULL;

   InitSegment(szName, x, y, width, height, angle, topcap, bottomcap, 45);
}

Segment::Segment(const string& szName, float x, float y, float width, float height, float angle, SegmentCap topcap, SegmentCap bottomcap, float capangle)
{
   m_on = false;
   m_glow = -1.0f;
   m_radius = 0.0f;
   m_pStyle = NULL;
   m_pGlassPath = NULL;
   m_pLightPath = NULL;
   m_pExternMatrix = NULL;
   m_pOwnMatrix = NULL;

   InitSegment(szName, x, y, width, height, angle, topcap, bottomcap, capangle);
}

Segment::Segment(float x, float y, float radius)
{
   m_on = false;
   m_glow = -1.0f;
   m_pStyle = NULL;
   m_pGlassPath = NULL;
   m_pLightPath = NULL;
   m_pExternMatrix = NULL;
   m_pOwnMatrix = NULL;

   InitSegmentDot(x, y, radius);
}

Segment::~Segment()
{
   delete m_pGlassPath;
   delete m_pLightPath;
   delete m_pExternMatrix;
   delete m_pOwnMatrix;
}

GraphicsPath* Segment::GetGlassPath()
{
   GetGlassData();
   return m_pGlassPath;
}

GraphicsPath* Segment::GetGlassPathTransformed()
{
   GraphicsPath* pPath = GetGlassPath()->Clone();
   pPath->Transform(m_pOwnMatrix);
   return pPath;
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
      uint32_t color = m_pStyle->GetGlassColorCenter();
      uint8_t alpha = m_pStyle->GetGlassAlphaCenter();
      pRenderer->SetColor(color, alpha);
      PaintSegment(pRenderer, m_pGlassPath);
   }
   else {
      uint32_t color = m_pStyle->GetOffColor();
      pRenderer->SetColor(color);
      PaintSegment(pRenderer, m_pGlassPath);
   }
   pRenderer->ResetTransform();
}

void Segment::DrawLight(VPXGraphics* pRenderer)
{
   if (!m_on)
      return;

   SetTransform(pRenderer);
   GetLightData();
   pRenderer->SetColor(RGB(255, 255, 0));
   PaintSegment(pRenderer, m_pLightPath);
   pRenderer->ResetTransform();
}

void Segment::InitSegmentDot(float x, float y, float radius)
{
   m_szName = ".";
   m_points.push_back({ radius, radius });
   m_radius = radius;

   CreateLightData();

   m_pOwnMatrix = new Matrix();
   m_pOwnMatrix->Translate(x, y);
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

   m_points.push_back({ nBounds.x + topdelta, nBounds.y });
   m_points.push_back({ nBounds.x + nBounds.w, nBounds.y + (topcap == SegmentCap_Flat ? 0 : topright) });
   m_points.push_back({ nBounds.x + nBounds.w, nBounds.y + nBounds.h - (bottomcap == SegmentCap_Flat ? 0 : bottomright) });
   m_points.push_back({ nBounds.x + bottomdelta, nBounds.y + nBounds.h });
   m_points.push_back({ nBounds.x, nBounds.y + nBounds.h - (bottomcap == SegmentCap_Flat ? 0 : bottomleft) });
   m_points.push_back({ nBounds.x, nBounds.y + (topcap == SegmentCap_Flat ? 0 : topleft) });

   m_angle = angle;
   CreateLightData();

   m_pOwnMatrix = new Matrix();
   m_pOwnMatrix->Translate(x, y);
   m_pOwnMatrix->Rotate(angle);
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
      m_lights.push_back({ m_points[0].x, m_points[0].y - m_glow });
      m_lights.push_back({ m_points[1].x + m_glow, m_points[1].y });
      m_lights.push_back({ m_points[2].x + m_glow, m_points[2].y });
      m_lights.push_back({ m_points[3].x, m_points[3].y + m_glow });
      m_lights.push_back({ m_points[4].x - m_glow, m_points[4].y });
      m_lights.push_back({ m_points[5].x - m_glow, m_points[5].y });
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
    nLeft *= tanf(nCapangle * (float)(M_PI / 180.0));
    nRight *= tanf(nCapangle * (float)(M_PI / 180.0));
}

void Segment::PaintSegment(VPXGraphics* pRenderer, GraphicsPath* pPath)
{
   if (m_pStyle->IsWireFrame())
      pRenderer->DrawPath(pPath);
   else
      pRenderer->FillPath(pPath);
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
}

void Segment::ResetCacheData()
{
   delete m_pGlassPath;
   m_pGlassPath = NULL;

   delete m_pLightPath;
   m_pLightPath = NULL;
}

void Segment::SetTransform(VPXGraphics* pRenderer)
{
   Matrix* pMatrix;
   if (!m_pExternMatrix)
      pMatrix = new Matrix();
   else
      pMatrix = m_pExternMatrix->Clone();
   pMatrix->Multiply(*m_pOwnMatrix);
   pRenderer->SetTransform(pMatrix);
   delete pMatrix;
}

void Segment::Transform(Matrix* pMatrix)
{
   delete m_pExternMatrix;
   m_pExternMatrix = pMatrix->Clone();
}


}
