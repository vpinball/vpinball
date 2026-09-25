// license:GPLv3+

// Definition of the DragPoint class

#pragma once

#include <algorithm>
#include <memory>

#include "core/ieditable.h"
#include "math/MeshUtils.h"

class DragPointCurve;

class DragPoint
{
public:
   DragPoint(DragPointCurve *pcurve, const float x, const float y, const float z, const bool smooth)
      : m_pcurve(pcurve)
      , m_v(x, y, z)
      , m_smooth(smooth)
   {
   }

   static inline constexpr ItemTypeEnum ItemType = eItemDragPoint;
   ItemTypeEnum GetItemType() const { return eItemDragPoint; }

   // Single point manipulation, used by the Win32 editor (moves only this point, not the parent part)
   void Translate(const Vertex2D &offset);
   Vertex2D GetCenter() const;

   IEditable *GetIEditable();
   const IEditable *GetIEditable() const;

   DragPointCurve *GetCurve() { return m_pcurve; }
   const DragPointCurve *GetCurve() const { return m_pcurve; }

   void ToggleSmooth();
   void ToggleSlingshot();

   // Can't allow less points than the user can recover from
   bool CanDelete() const;

   void Delete();

   // Point properties
   inline const Vertex3Ds &GetVertex() const { return m_v; }
   inline float GetX() const { return m_v.x; }
   inline void SetX(const float v) { m_v.x = v; }
   inline float GetY() const { return m_v.y; }
   inline void SetY(const float v) { m_v.y = v; }
   inline float GetZ() const { return m_v.z; }
   inline void SetZ(const float v) { m_v.z = v; }
   inline float GetCalcHeight() const { return m_calcHeight; }
   inline void SetCalcHeight(const float v) { m_calcHeight = v; }
   inline bool IsSmooth() const { return m_smooth; }
   inline void SetSmooth(const bool v) { m_smooth = v; }
   inline bool IsAutoTextureCoordinate() const { return m_autoTexture; }
   inline void SetAutoTextureCoordinate(const bool v) { m_autoTexture = v; }
   inline float GetTextureCoordinateU() const { return m_texturecoord; }
   inline void SetTextureCoordinateU(const float v) { m_texturecoord = v; }

   bool m_slingshot = false;
   bool m_uiLocked = false; // Can not be dragged in the editor
   bool m_uiVisible = true; // UI visibility (not the same as rendering visibility which is a member of part data)

private:
   DragPointCurve *const m_pcurve;
   Vertex3Ds m_v;
   float m_calcHeight = 0.f;
   float m_texturecoord = 0.f;
   bool m_smooth;
   bool m_autoTexture = true;
};

// A curve made of DragPoint control points
class DragPointCurve final
{
public:
   DragPointCurve(IEditable *owner, const int minPoints)
      : m_owner(owner)
      , m_minPoints(minPoints)
   {
   }

   ~DragPointCurve();

   IEditable *GetIEditable() { return m_owner; }
   const IEditable *GetIEditable() const { return m_owner; }

   PinTable *GetPTable() { return m_owner->GetPTable(); }
   const PinTable *GetPTable() const { return m_owner->GetPTable(); }

   int GetMinimumPoints() const { return m_minPoints; }

   void SavePoints(IObjectWriter &writer) const;
   void LoadPointToken(IObjectReader &reader);

   const Vertex2D &GetCenter() const;
   const Vertex2D &GetCentroid() const;
   const Vertex2D &GetMinBound() const;
   const Vertex2D &GetMaxBound() const;

   void FlipPointY(const Vertex2D &pvCenter);
   void FlipPointX(const Vertex2D &pvCenter);
   void RotatePoints(const float ang, const Vertex2D &center);
   void ScalePoints(const float scalex, const float scaley, const Vertex2D &center);
   void TranslatePoints(const Vertex2D &offset);
   void ReverseOrder();

   void GetTextureCoords(const vector<RenderVertex> &vv, float **ppcoords) const;

   template <typename T> void GetRgVertex(vector<T> &vv, const bool loop = true, const float accuracy = 4.f) const // 4 = maximum precision that we allow for
   {
      static const int Dim = T::Dim; // for now, this is always 2 or 3

      const int cpoint = (int)m_dragpoints.size();
      const int endpoint = loop ? cpoint : cpoint - 1;

      T rendv2;

      for (int i = 0; i < endpoint; i++)
      {
         const DragPoint *const pdp1 = m_dragpoints[i].get();
         const DragPoint *const pdp2 = m_dragpoints[(i < cpoint - 1) ? (i + 1) : 0].get();

         if ((pdp1->GetX() == pdp2->GetX()) && (pdp1->GetY() == pdp2->GetY()) && (pdp1->GetZ() == pdp2->GetZ()))
         {
            // Special case - two points coincide
            continue;
         }

         int iprev = (pdp1->IsSmooth() ? i - 1 : i);
         if (iprev < 0)
            iprev = (loop ? cpoint - 1 : 0);

         int inext = (pdp2->IsSmooth() ? i + 2 : i + 1);
         if (inext >= cpoint)
            inext = (loop ? inext - cpoint : cpoint - 1);

         const DragPoint *const pdp0 = m_dragpoints[iprev].get();
         const DragPoint *const pdp3 = m_dragpoints[inext].get();

         CatmullCurve<Dim> cc;
         cc.SetCurve(pdp0->GetVertex(), pdp1->GetVertex(), pdp2->GetVertex(), pdp3->GetVertex());

         T rendv1;

         rendv1.set(pdp1->GetVertex());
         rendv1.smooth = pdp1->IsSmooth();
         rendv1.slingshot = pdp1->m_slingshot;
         rendv1.controlPoint = true;

         // Properties of last point don't matter, because it won't be added to the list on this pass (it'll get added as the first point of the next curve)
         rendv2.set(pdp2->GetVertex());

         RecurseSmoothLine(cc, 0.f, 1.f, rendv1, rendv2, vv, accuracy);
      }

      if (!loop)
      {
         // Add the very last point to the list because nobody else added it
         rendv2.smooth = true;
         rendv2.slingshot = false;
         rendv2.controlPoint = false;
         vv.push_back(rendv2);
      }
   }

   void ClearPoints()
   {
      m_dragpoints.clear();
      OnPointsModified();
   }

   void DeletePoint(DragPoint *point)
   {
      const auto it = std::ranges::find_if(m_dragpoints, [point](const std::unique_ptr<DragPoint> &p) { return p.get() == point; });
      if (it != m_dragpoints.end())
         m_dragpoints.erase(it);
      OnPointsModified();
   }

   void PushPoint(std::unique_ptr<DragPoint> pdp)
   {
      m_dragpoints.push_back(std::move(pdp));
      OnPointsModified();
   }

   void InsertPoint(size_t pos, std::unique_ptr<DragPoint> pdp)
   {
      m_dragpoints.insert(m_dragpoints.begin() + pos, std::move(pdp));
      OnPointsModified();
   }

   int GetPointIndex(const DragPoint *point) const
   {
      const auto it = std::ranges::find_if(m_dragpoints, [point](const std::unique_ptr<DragPoint> &p) { return p.get() == point; });
      return (it == m_dragpoints.end()) ? -1 : static_cast<int>(it - m_dragpoints.begin());
   }

   void OnPointsModified() { m_boundsDirty = true; }

   const vector<std::unique_ptr<DragPoint>> &GetPoints() const { return m_dragpoints; }

private:
   IEditable *const m_owner;
   const int m_minPoints;

   vector<std::unique_ptr<DragPoint>> m_dragpoints;

   // Lazily updated bounds, center & centroid
   void UpdateBoundsAndCentroid() const;
   mutable bool m_boundsDirty = true;
   mutable Vertex2D m_minBound;
   mutable Vertex2D m_maxBound;
   mutable Vertex2D m_center;
   mutable Vertex2D m_centroid;
};
