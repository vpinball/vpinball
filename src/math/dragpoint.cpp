// license:GPLv3+

#include "core/stdafx.h"
#include "math/dragpoint.h"


DragPointCurve::~DragPointCurve() { ClearPoints(); }

void DragPointCurve::UpdateBounds() const
{
   if (m_boundsDirty)
   {
      m_boundsDirty = false;
      m_minBound.x = FLT_MAX;
      m_maxBound.x = -FLT_MAX;
      m_minBound.y = FLT_MAX;
      m_maxBound.y = -FLT_MAX;
      vector<RenderVertex> vvertex;
      GetRgVertex(vvertex);
      for (const auto &v : vvertex)
      {
         m_minBound.x = min(m_minBound.x, v.x);
         m_maxBound.x = max(m_maxBound.x, v.x);
         m_minBound.y = min(m_minBound.y, v.y);
         m_maxBound.y = max(m_maxBound.y, v.y);
      }
      /*
      for (const auto& v : m_dragpoints)
      {
         m_minBound.x = min(m_minBound.x, v->GetX());
         m_maxBound.x = max(m_maxBound.x, v->GetX());
         m_minBound.y = min(m_minBound.y, v->GetY());
         m_maxBound.y = max(m_maxBound.y, v->GetY());
      }
      */
      m_center = 0.5f * (m_maxBound + m_minBound);
   }
}

const Vertex2D &DragPointCurve::GetCenter() const
{
   UpdateBounds();
   return m_center;
}

const Vertex2D &DragPointCurve::GetMinBound() const
{
   UpdateBounds();
   return m_minBound;
}

const Vertex2D &DragPointCurve::GetMaxBound() const
{
   UpdateBounds();
   return m_maxBound;
}

void DragPointCurve::FlipPointY(const Vertex2D &pvCenter)
{
   for (const auto &v : m_dragpoints)
   {
      const float deltay = v->GetY() - pvCenter.y;
      v->SetY(v->GetY() - deltay * 2.0f);
   }
   ReverseOrder();
   OnPointsModified();
}

void DragPointCurve::FlipPointX(const Vertex2D &pvCenter)
{
   for (const auto &v : m_dragpoints)
   {
      const float deltax = v->GetX() - pvCenter.x;
      v->SetX(v->GetX() - deltax * 2.0f);
   }
   ReverseOrder();
   OnPointsModified();
}

void DragPointCurve::RotatePoints(const float ang, const Vertex2D &center)
{
   const float sn = sinf(ANGTORAD(ang));
   const float cs = cosf(ANGTORAD(ang));
   for (const auto &v : m_dragpoints)
   {
      const float dx = v->GetX() - center.x;
      const float dy = v->GetY() - center.y;
      const float dx2 = cs * dx - sn * dy;
      const float dy2 = cs * dy + sn * dx;
      v->SetX(center.x + dx2);
      v->SetY(center.y + dy2);
   }
   OnPointsModified();
}

void DragPointCurve::ScalePoints(const float scalex, const float scaley, const Vertex2D &center)
{
   for (const auto &v : m_dragpoints)
   {
      const float dx = (v->GetX() - center.x) * scalex;
      const float dy = (v->GetY() - center.y) * scaley;
      v->SetX(center.x + dx);
      v->SetY(center.y + dy);
   }
   OnPointsModified();
}

void DragPointCurve::TranslatePoints(const Vertex2D &offset)
{
   for (const auto &v : m_dragpoints)
   {
      v->SetX(v->GetX() + offset.x);
      v->SetY(v->GetY() + offset.y);
   }
   OnPointsModified();
}

void DragPointCurve::ReverseOrder()
{
   if (m_dragpoints.empty())
      return;

   // Reverse order of points (switches winding, reverses inside/outside)
   std::ranges::reverse(m_dragpoints.begin(), m_dragpoints.end());

   const bool slingshotTemp = m_dragpoints[0]->m_slingshot;

   for (size_t i = 0; i < m_dragpoints.size() - 1; i++)
   {
      DragPoint *const pdp1 = m_dragpoints[i].get();
      const DragPoint *const pdp2 = m_dragpoints[i + 1].get();

      pdp1->m_slingshot = pdp2->m_slingshot;
   }

   m_dragpoints[m_dragpoints.size() - 1]->m_slingshot = slingshotTemp;
}

// Ported at: VisualPinball.Engine/Math/DragPoint.cs

void DragPointCurve::GetTextureCoords(const vector<RenderVertex> &vv, float **ppcoords) const
{
   vector<int> vitexpoints;
   vector<int> virenderpoints;
   bool noCoords = false;

   const int cpoints = (int)vv.size();
   int icontrolpoint = 0;

   *ppcoords = new float[cpoints];
   memset(*ppcoords, 0, sizeof(float) * cpoints);

   for (int i = 0; i < cpoints; ++i)
   {
      const RenderVertex *const prv = &vv[i];
      if (prv->controlPoint)
      {
         if (!m_dragpoints[icontrolpoint]->IsAutoTextureCoordinate())
         {
            vitexpoints.push_back(icontrolpoint);
            virenderpoints.push_back(i);
         }
         ++icontrolpoint;
      }
   }

   if (vitexpoints.empty())
   {
      // Special case - no texture coordinates were specified
      // Make them up starting at point 0
      vitexpoints.push_back(0);
      virenderpoints.push_back(0);

      noCoords = true;
   }

   // Wrap the array around so we cover the last section
   vitexpoints.push_back(vitexpoints[0] + (int)m_dragpoints.size());
   virenderpoints.push_back(virenderpoints[0] + cpoints);

   for (int i = 0; i < (int)vitexpoints.size() - 1; ++i)
   {
      const int startrenderpoint = virenderpoints[i] % cpoints;
      int endrenderpoint = virenderpoints[(i < cpoints - 1) ? (i + 1) : 0] % cpoints;

      float starttexcoord;
      float endtexcoord;
      if (noCoords)
      {
         starttexcoord = 0.0f;
         endtexcoord = 1.0f;
      }
      else
      {
         starttexcoord = m_dragpoints[vitexpoints[i] % m_dragpoints.size()]->GetTextureCoordinateU();
         endtexcoord = m_dragpoints[vitexpoints[i + 1] % m_dragpoints.size()]->GetTextureCoordinateU();
      }

      const float deltacoord = endtexcoord - starttexcoord;

      if (endrenderpoint <= startrenderpoint)
         endrenderpoint += cpoints;

      float totallength = 0.0f;
      for (int l = startrenderpoint; l < endrenderpoint; ++l)
      {
         const Vertex2D *const pv1 = &vv[l % cpoints];
         const Vertex2D *const pv2 = &vv[(l + 1) % cpoints];

         const float dx = pv1->x - pv2->x;
         const float dy = pv1->y - pv2->y;
         const float length = sqrtf(dx * dx + dy * dy);

         totallength += length;
      }

      float partiallength = 0.0f;
      for (int l = startrenderpoint; l < endrenderpoint; ++l)
      {
         const Vertex2D *const pv1 = &vv[l % cpoints];
         const Vertex2D *const pv2 = &vv[(l + 1) % cpoints];

         const float dx = pv1->x - pv2->x;
         const float dy = pv1->y - pv2->y;
         const float length = sqrtf(dx * dx + dy * dy);
         if (totallength == 0.0f)
            totallength = 1.0f;
         const float texcoord = partiallength / totallength;

         (*ppcoords)[l % cpoints] = (texcoord * deltacoord) + starttexcoord;

         partiallength += length;
      }
   }
}

void DragPointCurve::SavePoints(IObjectWriter &writer) const
{
   for (const auto &pdp : m_dragpoints)
   {
      writer.BeginObject(FID(DPNT), true, false);
      writer.WriteVector2(FID(VCEN), Vertex2D(pdp->GetX(), pdp->GetY()));
      writer.WriteFloat(FID(POSZ), pdp->GetZ());
      writer.WriteBool(FID(SMTH), pdp->IsSmooth());
      writer.WriteBool(FID(SLNG), pdp->m_slingshot);
      writer.WriteBool(FID(ATEX), pdp->IsAutoTextureCoordinate());
      writer.WriteFloat(FID(TEXC), pdp->GetTextureCoordinateU());
      writer.WriteBool(FID(LOCK), pdp->m_uiLocked);
      writer.WriteBool(FID(LVIS), pdp->m_uiVisible);
      writer.EndObject();
   }
}

void DragPointCurve::LoadPointToken(IObjectReader &reader)
{
   auto pdp = std::make_unique<DragPoint>(this, 0.f, 0.f, 0.f, false);
   reader.AsObject(
      [&pdp](int tag, IObjectReader &reader)
      {
         switch (tag)
         {
         case FID(VCEN):
         {
            auto v = reader.AsVector2();
            pdp->SetX(v.x);
            pdp->SetY(v.y);
            break;
         }
         case FID(POSZ): pdp->SetZ(reader.AsFloat()); break;
         case FID(SMTH): pdp->SetSmooth(reader.AsBool()); break;
         case FID(SLNG): pdp->m_slingshot = reader.AsBool(); break;
         case FID(ATEX): pdp->SetAutoTextureCoordinate(reader.AsBool()); break;
         case FID(TEXC): pdp->SetTextureCoordinateU(reader.AsFloat()); break;
         case FID(LOCK): pdp->m_uiLocked = reader.AsBool(); break;
         case FID(LVIS): pdp->m_uiVisible = reader.AsBool(); break;
         // Old save would wrongly save these fields which do not apply to dragpoint
         case FID(LAYR): reader.AsInt(); break;
         case FID(LANR): reader.AsString(); break;
         }
         return true;
      });
   PushPoint(std::move(pdp));
}

IEditable *DragPoint::GetIEditable() { return m_pcurve->GetIEditable(); }

const IEditable *DragPoint::GetIEditable() const { return m_pcurve->GetIEditable(); }

void DragPoint::Translate(const Vertex2D &offset)
{
   m_v.x += offset.x;
   m_v.y += offset.y;
}

Vertex2D DragPoint::GetCenter() const { return { m_v.x, m_v.y }; }

bool DragPoint::CanDelete() const { return (int)m_pcurve->GetPoints().size() > m_pcurve->GetMinimumPoints(); }

void DragPoint::Delete()
{
   if (CanDelete())
      m_pcurve->DeletePoint(this);
}

void DragPoint::ToggleSmooth()
{
   m_smooth = !m_smooth;
   const int index2 = (m_pcurve->GetPointIndex(this) - 1 + (int)m_pcurve->GetPoints().size()) % (int)m_pcurve->GetPoints().size();
   if (m_smooth && m_slingshot)
   {
      m_slingshot = false;
   }
   if (m_smooth && m_pcurve->GetPoints()[index2]->m_slingshot)
   {
      m_pcurve->GetPoints()[index2]->m_slingshot = false;
   }
}

void DragPoint::ToggleSlingshot()
{
   m_slingshot = !m_slingshot;
   if (m_slingshot)
   {
      m_smooth = false;
      const int index2 = (m_pcurve->GetPointIndex(this) + 1) % m_pcurve->GetPoints().size();
      m_pcurve->GetPoints()[index2]->m_smooth = false;
   }
}

