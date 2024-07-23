#include "stdafx.h"

#include "Segment.h"

Segment::Segment(const string& szName, float x, float y, float width, float height, float angle)
{
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
   if (m_pStyle)
      delete m_pStyle;

   if (m_pGlassPath)
      delete m_pGlassPath;

   if (m_pLightPath)
      delete m_pLightPath;

   if (m_pExternMatrix)
      delete m_pExternMatrix;

   if (m_pOwnMatrix)
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

void Segment::Draw(VP::RendererGraphics* pGraphics)
{
   SetTransform(pGraphics);
   GetGlassData();
   if (m_on) {
      pGraphics->SetColor(m_pStyle->GetGlassColorCenter(), m_pStyle->GetGlassAlphaCenter());
      PaintSegment(pGraphics, m_pGlassPath);
   }
   else {
      pGraphics->SetColor(m_pStyle->GetOffColor());
      PaintSegment(pGraphics, m_pGlassPath);
   }
   pGraphics->ResetTransform();
}

void Segment::DrawLight(VP::RendererGraphics* pGraphics)
{
   if (!m_on)
      return;

   SetTransform(pGraphics);
   GetLightData();
   pGraphics->SetColor(RGB(255, 255, 0));
   PaintSegment(pGraphics, m_pLightPath);
   pGraphics->ResetTransform();
}

void Segment::InitSegmentDot(float x, float y, float radius)
{
   m_szName = ".";
   m_points.push_back({ radius, radius });
   m_radius = radius;

   CreateLightData();

   m_pOwnMatrix = new VP::Matrix();
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

   m_pOwnMatrix = new VP::Matrix();
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
   if (SDL_FRectEmpty(&m_pStyle->GetBulbSize()))
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
         nRight = nWidth * 3.0f / 4.0f;
         break;
      case SegmentCap_MoreRight:
         nLeft = nWidth * 3.0f / 4.0f;
         nRight = nWidth / 4.0f;
         break;
    }
    nDelta = nLeft;
    nLeft = tan(nCapangle / 180.0f * M_PI) * nLeft;
    nRight = tan(nCapangle / 180.0f * M_PI) * nRight;
}
 
void Segment::PaintSegment(VP::RendererGraphics* pGraphics, GraphicsPath* pPath)
{
   if (m_pStyle->IsWireFrame())
      pGraphics->DrawPath(pPath);
   else
      pGraphics->FillPath(pPath);
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
   if (m_pGlassPath) {
      delete m_pGlassPath;
      m_pGlassPath = NULL;
   }
   if (m_pLightPath) {
      delete m_pLightPath;
      m_pLightPath = NULL;
   }
}

void Segment::SetTransform(VP::RendererGraphics* pGraphics)
{
   VP::Matrix* pMatrix;
   if (!m_pExternMatrix)
      pMatrix = new VP::Matrix();
   else
      pMatrix = m_pExternMatrix->Clone();
   pMatrix->Multiply(*m_pOwnMatrix);
   pGraphics->SetTransform(pMatrix);
   delete pMatrix;
}

void Segment::Transform(VP::Matrix* pMatrix)
{
   m_pExternMatrix = pMatrix->Clone();
}