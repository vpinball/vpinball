// license:GPLv3+

#include "core/stdafx.h"

#include "parts/primitive.h"
#include "ui/win/sur.h"
#include "ui/win/WinEditor.h"
#include "ui/win/parts/PrimitiveWinUIPart.h"

PrimitiveWinUIPart::PrimitiveWinUIPart(PinTableWnd* editor, Primitive* primitive)
   : IWinUIPart(editor, primitive)
   , m_primitive(primitive)
{
}

void PrimitiveWinUIPart::UpdateStatusBarObjectPos()
{
   SetStatusBarObjectPos(m_primitive->m_d.m_vPosition.x, m_primitive->m_d.m_vPosition.y);
}

void PrimitiveWinUIPart::UIRenderPass1(Sur* const psur)
{
}

void PrimitiveWinUIPart::UIRenderPass2(Sur* const psur)
{
   m_primitive->RecalculateMatrices();
   m_primitive->TransformVertices();

   psur->SetLineColor(RGB(0, 0, 0), false, 1);
   psur->SetObject(m_primitive);
   if (!m_primitive->m_d.m_displayTexture)
   {
      vector<Vertex2D> edges, polyline;
      m_primitive->GetEditorWireframe(edges, polyline);
      if (!edges.empty())
         psur->Lines(edges.data(), (int)(edges.size() / 2));
      if (!polyline.empty())
         psur->Polyline(polyline.data(), (int)polyline.size());
   }

   // draw center marker
   psur->SetLineColor(RGB(128, 128, 128), false, 1);
   psur->Line(m_primitive->m_d.m_vPosition.x - 10.0f, m_primitive->m_d.m_vPosition.y, m_primitive->m_d.m_vPosition.x + 10.0f, m_primitive->m_d.m_vPosition.y);
   psur->Line(m_primitive->m_d.m_vPosition.x, m_primitive->m_d.m_vPosition.y - 10.0f, m_primitive->m_d.m_vPosition.x, m_primitive->m_d.m_vPosition.y + 10.0f);

   if (m_primitive->m_d.m_displayTexture)
   {
      Texture* const ppi = m_primitive->m_ptable->GetImage(m_primitive->m_d.m_szImage);
      if (ppi && ppi->GetGDIBitmap())
      {
         vector<Vertex2D> triangles;
         m_primitive->GetEditorTriangles(triangles);
         vector<RenderVertex> vvertex;
         vvertex.reserve(triangles.size());
         for (const Vertex2D& v : triangles)
         {
            RenderVertex rv;
            rv.x = v.x;
            rv.y = v.y;
            vvertex.push_back(rv);
         }
         psur->PolygonImage(vvertex, ppi->GetGDIBitmap(), m_primitive->m_ptable->m_left, m_primitive->m_ptable->m_top, m_primitive->m_ptable->m_right, m_primitive->m_ptable->m_bottom, ppi->m_width, ppi->m_height);
      }
   }
}

void PrimitiveWinUIPart::RenderBlueprint(Sur* psur, const bool solid)
{
   psur->SetFillColor(solid ? BLUEPRINT_SOLID_COLOR : -1);
   psur->SetLineColor(RGB(0, 0, 0), false, 1);
   psur->SetObject(m_primitive);

   if (solid && m_primitive->m_d.m_use3DMesh)
   {
      vector<Vertex2D> triangles;
      m_primitive->GetEditorTriangles(triangles);
      for (size_t i = 0; i + 2 < triangles.size(); i += 3)
         psur->Polygon(&triangles[i], 3);
      return;
   }

   vector<Vertex2D> edges, polyline;
   m_primitive->GetEditorWireframe(edges, polyline);
   if (!edges.empty())
      psur->Lines(edges.data(), (int)(edges.size() / 2));
   if (!polyline.empty())
      psur->Polyline(polyline.data(), (int)polyline.size());
}
