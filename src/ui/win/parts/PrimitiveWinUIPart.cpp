// license:GPLv3+

#include "core/stdafx.h"

#include "parts/primitive.h"
#include "ui/win/sur.h"
#include "ui/win/WinEditor.h"
#include "ui/win/parts/PrimitiveWinUIPart.h"

PrimitiveWinUIPart::PrimitiveWinUIPart(PinTableWnd* editor, Primitive* primitive)
   : m_editor(editor)
   , m_primitive(primitive)
{
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
      if ((m_primitive->m_d.m_edgeFactorUI <= 0.0f) || (m_primitive->m_d.m_edgeFactorUI >= 1.0f) || !m_primitive->m_d.m_use3DMesh)
      {
         if (!m_primitive->m_d.m_use3DMesh || (m_primitive->m_d.m_edgeFactorUI >= 1.0f) || (m_primitive->m_mesh.NumVertices() <= 100)) // small mesh: draw all triangles
         {
            for (size_t i = 0; i < m_primitive->m_mesh.NumIndices(); i += 3)
            {
               const Vertex3Ds* const A = &m_primitive->m_vertices[m_primitive->m_mesh.m_indices[i]];
               const Vertex3Ds* const B = &m_primitive->m_vertices[m_primitive->m_mesh.m_indices[i + 1]];
               const Vertex3Ds* const C = &m_primitive->m_vertices[m_primitive->m_mesh.m_indices[i + 2]];
               psur->Line(A->x, A->y, B->x, B->y);
               psur->Line(B->x, B->y, C->x, C->y);
               psur->Line(C->x, C->y, A->x, A->y);
            }
         }
         else // large mesh: draw a simplified mesh for performance reasons, does not approximate the shape well
         {
            if (m_primitive->m_mesh.NumIndices() > 0)
            {
               const size_t numPts = m_primitive->m_mesh.NumIndices() / 3 + 1;
               vector<Vertex2D> drawVertices(numPts);

               const Vertex3Ds& A = m_primitive->m_vertices[m_primitive->m_mesh.m_indices[0]];
               drawVertices[0] = Vertex2D(A.x, A.y);

               unsigned int o = 1;
               for (size_t i = 0; i < m_primitive->m_mesh.NumIndices(); i += 3, ++o)
               {
                  const Vertex3Ds& B = m_primitive->m_vertices[m_primitive->m_mesh.m_indices[i + 1]];
                  drawVertices[o] = Vertex2D(B.x, B.y);
               }

               psur->Polyline(drawVertices.data(), (int)drawVertices.size());
            }
         }
      }
      else
      {
         vector<Vertex2D> drawVertices;
         for (size_t i = 0; i < m_primitive->m_mesh.NumIndices(); i += 3)
         {
            const Vertex3Ds* const A = &m_primitive->m_vertices[m_primitive->m_mesh.m_indices[i]];
            const Vertex3Ds* const B = &m_primitive->m_vertices[m_primitive->m_mesh.m_indices[i + 1]];
            const Vertex3Ds* const C = &m_primitive->m_vertices[m_primitive->m_mesh.m_indices[i + 2]];
            const float An = m_primitive->m_normals[m_primitive->m_mesh.m_indices[i]];
            const float Bn = m_primitive->m_normals[m_primitive->m_mesh.m_indices[i + 1]];
            const float Cn = m_primitive->m_normals[m_primitive->m_mesh.m_indices[i + 2]];
            if (fabsf(An + Bn) < m_primitive->m_d.m_edgeFactorUI)
            {
               drawVertices.emplace_back(A->x, A->y);
               drawVertices.emplace_back(B->x, B->y);
            }
            if (fabsf(Bn + Cn) < m_primitive->m_d.m_edgeFactorUI)
            {
               drawVertices.emplace_back(B->x, B->y);
               drawVertices.emplace_back(C->x, C->y);
            }
            if (fabsf(Cn + An) < m_primitive->m_d.m_edgeFactorUI)
            {
               drawVertices.emplace_back(C->x, C->y);
               drawVertices.emplace_back(A->x, A->y);
            }
         }

         if (!drawVertices.empty())
            psur->Lines(drawVertices.data(), (int)(drawVertices.size() / 2));
      }
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
         vector<RenderVertex> vvertex;
         vvertex.reserve(m_primitive->m_mesh.NumIndices());
         for (size_t i = 0; i < m_primitive->m_mesh.NumIndices(); i += 3)
         {
            const Vertex3Ds* const A = &m_primitive->m_vertices[m_primitive->m_mesh.m_indices[i]];
            const Vertex3Ds* const B = &m_primitive->m_vertices[m_primitive->m_mesh.m_indices[i + 1]];
            const Vertex3Ds* const C = &m_primitive->m_vertices[m_primitive->m_mesh.m_indices[i + 2]];
            RenderVertex rvA;
            RenderVertex rvB;
            RenderVertex rvC;
            rvA.x = A->x;
            rvA.y = A->y;
            rvB.x = B->x;
            rvB.y = B->y;
            rvC.x = C->x;
            rvC.y = C->y;
            vvertex.push_back(rvC);
            vvertex.push_back(rvB);
            vvertex.push_back(rvA);
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
      for (size_t i = 0; i < m_primitive->m_mesh.NumIndices(); i += 3)
      {
         const Vertex3Ds* const A = &m_primitive->m_vertices[m_primitive->m_mesh.m_indices[i]];
         const Vertex3Ds* const B = &m_primitive->m_vertices[m_primitive->m_mesh.m_indices[i + 1]];
         const Vertex3Ds* const C = &m_primitive->m_vertices[m_primitive->m_mesh.m_indices[i + 2]];

         Vertex2D rv[3];
         rv[0].x = C->x;
         rv[0].y = C->y;
         rv[1].x = B->x;
         rv[1].y = B->y;
         rv[2].x = A->x;
         rv[2].y = A->y;
         psur->Polygon(rv, 3);
      }
      return;
   }
   if ((m_primitive->m_d.m_edgeFactorUI <= 0.0f) || (m_primitive->m_d.m_edgeFactorUI >= 1.0f) || !m_primitive->m_d.m_use3DMesh)
   {
      if (!m_primitive->m_d.m_use3DMesh || (m_primitive->m_d.m_edgeFactorUI >= 1.0f) || (m_primitive->m_mesh.NumVertices() <= 100)) // small mesh: draw all triangles
      {
         for (size_t i = 0; i < m_primitive->m_mesh.NumIndices(); i += 3)
         {
            const Vertex3Ds* const A = &m_primitive->m_vertices[m_primitive->m_mesh.m_indices[i]];
            const Vertex3Ds* const B = &m_primitive->m_vertices[m_primitive->m_mesh.m_indices[i + 1]];
            const Vertex3Ds* const C = &m_primitive->m_vertices[m_primitive->m_mesh.m_indices[i + 2]];
            psur->Line(A->x, A->y, B->x, B->y);
            psur->Line(B->x, B->y, C->x, C->y);
            psur->Line(C->x, C->y, A->x, A->y);
         }
      }
      else // large mesh: draw a simplified mesh for performance reasons, does not approximate the shape well
      {
         if (m_primitive->m_mesh.NumIndices() > 0)
         {
            const size_t numPts = m_primitive->m_mesh.NumIndices() / 3 + 1;
            vector<Vertex2D> drawVertices(numPts);

            const Vertex3Ds& A = m_primitive->m_vertices[m_primitive->m_mesh.m_indices[0]];
            drawVertices[0] = Vertex2D(A.x, A.y);

            unsigned int o = 1;
            for (size_t i = 0; i < m_primitive->m_mesh.NumIndices(); i += 3, ++o)
            {
               const Vertex3Ds& B = m_primitive->m_vertices[m_primitive->m_mesh.m_indices[i + 1]];
               drawVertices[o] = Vertex2D(B.x, B.y);
            }

            psur->Polyline(drawVertices.data(), (int)drawVertices.size());
         }
      }
   }
   else
   {
      vector<Vertex2D> drawVertices;
      for (size_t i = 0; i < m_primitive->m_mesh.NumIndices(); i += 3)
      {
         const Vertex3Ds* const A = &m_primitive->m_vertices[m_primitive->m_mesh.m_indices[i]];
         const Vertex3Ds* const B = &m_primitive->m_vertices[m_primitive->m_mesh.m_indices[i + 1]];
         const Vertex3Ds* const C = &m_primitive->m_vertices[m_primitive->m_mesh.m_indices[i + 2]];
         const float An = m_primitive->m_normals[m_primitive->m_mesh.m_indices[i]];
         const float Bn = m_primitive->m_normals[m_primitive->m_mesh.m_indices[i + 1]];
         const float Cn = m_primitive->m_normals[m_primitive->m_mesh.m_indices[i + 2]];
         if (fabsf(An + Bn) < m_primitive->m_d.m_edgeFactorUI)
         {
            drawVertices.emplace_back(A->x, A->y);
            drawVertices.emplace_back(B->x, B->y);
         }
         if (fabsf(Bn + Cn) < m_primitive->m_d.m_edgeFactorUI)
         {
            drawVertices.emplace_back(B->x, B->y);
            drawVertices.emplace_back(C->x, C->y);
         }
         if (fabsf(Cn + An) < m_primitive->m_d.m_edgeFactorUI)
         {
            drawVertices.emplace_back(C->x, C->y);
            drawVertices.emplace_back(A->x, A->y);
         }
      }

      if (!drawVertices.empty())
         psur->Lines(drawVertices.data(), (int)(drawVertices.size() / 2));
   }
}
