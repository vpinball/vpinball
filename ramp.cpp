#include "StdAfx.h"
//#include "forsyth.h"
#include "objloader.h"

Ramp::Ramp()
{
   m_menuid = IDR_SURFACEMENU;
   m_d.m_collidable = true;
   m_d.m_visible = true;
   m_dynamicVertexBuffer = 0;
   m_dynamicIndexBuffer = 0;
   m_dynamicVertexBuffer2 = 0;
   m_dynamicVertexBufferRegenerate = true;
   m_d.m_depthBias = 0.0f;
   m_d.m_wireDiameter = 6.0f;
   m_d.m_wireDistanceX = 38.0f;
   m_d.m_wireDistanceY = 88.0f;
   m_propPosition = NULL;
   m_propPhysics = NULL;
   m_d.m_hitEvent = false;
   m_d.m_overwritePhysics = true;
   m_rgheightInit = NULL;
}

Ramp::~Ramp()
{
   if (m_dynamicVertexBuffer)
      m_dynamicVertexBuffer->release();

   if (m_dynamicVertexBuffer2)
      m_dynamicVertexBuffer2->release();

   if (m_dynamicIndexBuffer)
      m_dynamicIndexBuffer->release();

   if (m_rgheightInit)
      delete[] m_rgheightInit;
}

void Ramp::UpdateStatusBarInfo()
{
   char tbuf[128];
   sprintf_s(tbuf, "TopH: %.03f | BottomH: %0.3f | TopW: %.03f | BottomW: %.03f | LeftW: %.03f | RightW: %.03f", m_vpinball->ConvertToUnit(m_d.m_heighttop), m_vpinball->ConvertToUnit(m_d.m_heightbottom),
       m_vpinball->ConvertToUnit(m_d.m_widthtop), m_vpinball->ConvertToUnit(m_d.m_widthbottom),
       m_vpinball->ConvertToUnit(m_d.m_leftwallheightvisible), m_vpinball->ConvertToUnit(m_d.m_rightwallheightvisible));
   m_vpinball->SetStatusBarUnitInfo(tbuf, true);
}


bool Ramp::IsTransparent() const
{
   return m_ptable->GetMaterial(m_d.m_szMaterial)->m_bOpacityActive;
}

HRESULT Ramp::Init(PinTable *ptable, float x, float y, bool fromMouseClick)
{
   m_ptable = ptable;
   SetDefaults(fromMouseClick);
   m_d.m_visible = true;

   float length = 0.5f * LoadValueFloatWithDefault("DefaultProps\\Ramp", "Length", 400.0f);

   CComObject<DragPoint> *pdp;
   CComObject<DragPoint>::CreateInstance(&pdp);
   if (pdp)
   {
      pdp->AddRef();
      pdp->Init(this, x, y + length, 0.f, true);
      pdp->m_calcHeight = m_d.m_heightbottom;
      m_vdpoint.push_back(pdp);
   }

   CComObject<DragPoint>::CreateInstance(&pdp);
   if (pdp)
   {
      pdp->AddRef();
      pdp->Init(this, x, y - length, 0.f, true);
      pdp->m_calcHeight = m_d.m_heighttop;
      m_vdpoint.push_back(pdp);
   }

   InitVBA(fTrue, 0, NULL);

   return S_OK;
}

void Ramp::SetDefaults(bool fromMouseClick)
{
   static const char strKeyName[] = "DefaultProps\\Ramp";
   
   m_d.m_heightbottom = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "HeightBottom", 0.0f) : 0.0f;
   m_d.m_heighttop = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "HeightTop", 50.0f) : 50.0f;
   m_d.m_widthbottom = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "WidthBottom", 75.0f) : 75.0f;
   m_d.m_widthtop = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "WidthTop", 60.0f) : 60.0f;
   m_d.m_type = fromMouseClick ? (RampType)LoadValueIntWithDefault(strKeyName, "RampType", RampTypeFlat) : RampTypeFlat;

   m_d.m_tdr.m_TimerEnabled = fromMouseClick ? LoadValueBoolWithDefault(strKeyName, "TimerEnabled", false) : false;
   m_d.m_tdr.m_TimerInterval = fromMouseClick ? LoadValueIntWithDefault(strKeyName, "TimerInterval", 100) : 100;

   char buf[MAXTOKEN] = { 0 };
   HRESULT hr = LoadValueString(strKeyName, "Image", buf, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szImage.clear();
   else
      m_d.m_szImage = buf;

   m_d.m_imagealignment = fromMouseClick ? (RampImageAlignment)LoadValueIntWithDefault(strKeyName, "ImageMode", ImageModeWorld) : ImageModeWorld;
   m_d.m_imageWalls = fromMouseClick ? LoadValueBoolWithDefault(strKeyName, "ImageWalls", true) : true;

   m_d.m_leftwallheight = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "LeftWallHeight", 62.0f) : 62.0f;
   m_d.m_rightwallheight = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "RightWallHeight", 62.0f) : 62.0f;
   m_d.m_leftwallheightvisible = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "LeftWallHeightVisible", 30.0f) : 30.0f;
   m_d.m_rightwallheightvisible = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "RightWallHeightVisible", 30.0f) : 30.0f;

   m_d.m_threshold = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "HitThreshold", 2.0f) : 2.0f;

   SetDefaultPhysics(fromMouseClick);

   m_d.m_visible = fromMouseClick ? LoadValueBoolWithDefault(strKeyName, "Visible", true) : true;
   m_d.m_collidable = fromMouseClick ? LoadValueBoolWithDefault(strKeyName, "Collidable", true) : true;
   m_d.m_reflectionEnabled = fromMouseClick ? LoadValueBoolWithDefault(strKeyName, "ReflectionEnabled", true) : true;

   m_d.m_wireDiameter = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "WireDiameter", 8.0f) : 8.0f;
   m_d.m_wireDistanceX = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "WireDistanceX", 38.0f) : 38.0f;
   m_d.m_wireDistanceY = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "WireDistanceY", 88.0f) : 88.0f;
}

void Ramp::WriteRegDefaults()
{
   static const char strKeyName[] = "DefaultProps\\Ramp";

   SaveValueFloat(strKeyName, "HeightBottom", m_d.m_heightbottom);
   SaveValueFloat(strKeyName, "HeightTop", m_d.m_heighttop);
   SaveValueFloat(strKeyName, "WidthBottom", m_d.m_widthbottom);
   SaveValueFloat(strKeyName, "WidthTop", m_d.m_widthtop);
   SaveValueInt(strKeyName, "RampType", m_d.m_type);
   SaveValueBool(strKeyName, "TimerEnabled", m_d.m_tdr.m_TimerEnabled);
   SaveValueInt(strKeyName, "TimerInterval", m_d.m_tdr.m_TimerInterval);
   SaveValueString(strKeyName, "Image", m_d.m_szImage);
   SaveValueInt(strKeyName, "ImageMode", m_d.m_imagealignment);
   SaveValueBool(strKeyName, "ImageWalls", m_d.m_imageWalls);
   SaveValueFloat(strKeyName, "LeftWallHeight", m_d.m_leftwallheight);
   SaveValueFloat(strKeyName, "RightWallHeight", m_d.m_rightwallheight);
   SaveValueFloat(strKeyName, "LeftWallHeightVisible", m_d.m_leftwallheightvisible);
   SaveValueFloat(strKeyName, "RightWallHeightVisible", m_d.m_rightwallheightvisible);
   SaveValueBool(strKeyName, "HitEvent", m_d.m_hitEvent);
   SaveValueFloat(strKeyName, "HitThreshold", m_d.m_threshold);
   SaveValueFloat(strKeyName, "Elasticity", m_d.m_elasticity);
   SaveValueFloat(strKeyName, "Friction", m_d.m_friction);
   SaveValueFloat(strKeyName, "Scatter", m_d.m_scatter);
   SaveValueBool(strKeyName, "Collidable", m_d.m_collidable);
   SaveValueBool(strKeyName, "Visible", m_d.m_visible);
   SaveValueBool(strKeyName, "ReflectionEnabled", m_d.m_reflectionEnabled);
   SaveValueFloat(strKeyName, "WireDiameter", m_d.m_wireDiameter);
   SaveValueFloat(strKeyName, "WireDistanceX", m_d.m_wireDistanceX);
   SaveValueFloat(strKeyName, "WireDistanceY", m_d.m_wireDistanceY);
}

void Ramp::UIRenderPass1(Sur * const psur)
{
   //make 1 wire ramps look unique in editor - uses ramp color
   psur->SetFillColor(m_ptable->RenderSolid() ? m_vpinball->m_fillColor : -1);
   psur->SetBorderColor(-1, false, 0);
   psur->SetObject(this);

   int cvertex;
   const Vertex2D * const rgvLocal = GetRampVertex(cvertex, NULL, NULL, NULL, NULL, HIT_SHAPE_DETAIL_LEVEL, false);
   psur->Polygon(rgvLocal, cvertex * 2);

   delete[] rgvLocal;
}

void Ramp::UIRenderPass2(Sur * const psur)
{
   psur->SetFillColor(-1);
   psur->SetBorderColor(RGB(0, 0, 0), false, 0);
   psur->SetLineColor(RGB(0, 0, 0), false, 0);
   psur->SetObject(this);
   psur->SetObject(NULL); // NULL so this won't be hit-tested

   bool *pfCross;
   Vertex2D *middlePoints;
   int cvertex;
   const Vertex2D * const rgvLocal = GetRampVertex(cvertex, NULL, &pfCross, NULL, &middlePoints, HIT_SHAPE_DETAIL_LEVEL, false);
   psur->Polygon(rgvLocal, cvertex * 2);

   if (isHabitrail())
   {
      psur->Polyline(middlePoints, cvertex);
      if (m_d.m_type == RampType4Wire || m_d.m_type == RampType3WireRight)
      {
         psur->SetLineColor(RGB(0, 0, 0), false, 3);
         psur->Polyline(rgvLocal, cvertex);
      }
      if (m_d.m_type == RampType4Wire || m_d.m_type == RampType3WireLeft)
      {
         psur->SetLineColor(RGB(0, 0, 0), false, 3);
         psur->Polyline(&rgvLocal[cvertex], cvertex);
      }
   }
   else
   {
      for (int i = 0; i < cvertex; i++)
         if (pfCross[i])
            psur->Line(rgvLocal[i].x, rgvLocal[i].y, rgvLocal[cvertex * 2 - i - 1].x, rgvLocal[cvertex * 2 - i - 1].y);
   }

   delete[] rgvLocal;
   delete[] pfCross;
   delete[] middlePoints;

   bool drawDragpoints = ((m_selectstate != eNotSelected) || m_vpinball->m_alwaysDrawDragPoints);
   // if the item is selected then draw the dragpoints (or if we are always to draw dragpoints)
   if (!drawDragpoints)
   {
      // if any of the drag points of this object are selected then draw all the dragpoints
      for (size_t i = 0; i < m_vdpoint.size(); i++)
      {
         const CComObject<DragPoint> * const pdp = m_vdpoint[i];
         if (pdp->m_selectstate != eNotSelected)
         {
            drawDragpoints = true;
            break;
         }
      }
   }

   if (drawDragpoints)
   {
      for (size_t i = 0; i < m_vdpoint.size(); i++)
      {
         CComObject<DragPoint> * const pdp = m_vdpoint[i];
         psur->SetFillColor(-1);
         psur->SetBorderColor(pdp->m_dragging ? RGB(0, 255, 0) : ((i == 0) ? RGB(0, 0, 255) : RGB(255, 0, 0)), false, 0);
         psur->SetObject(pdp);

         psur->Ellipse2(pdp->m_v.x, pdp->m_v.y, 8);
      }
   }
}

void Ramp::RenderBlueprint(Sur *psur, const bool solid)
{
   psur->SetFillColor(solid ? BLUEPRINT_SOLID_COLOR : -1);
   psur->SetBorderColor(RGB(0, 0, 0), false, 0);
   psur->SetLineColor(RGB(0, 0, 0), false, 0);
   psur->SetObject(this);
   psur->SetObject(NULL); // NULL so this won't be hit-tested

   bool *pfCross;
   Vertex2D *middlePoints;
   int cvertex;
   const Vertex2D * const rgvLocal = GetRampVertex(cvertex, NULL, &pfCross, NULL, &middlePoints, HIT_SHAPE_DETAIL_LEVEL, false);
   psur->Polygon(rgvLocal, cvertex * 2);

   if (isHabitrail())
   {
      psur->Polyline(middlePoints, cvertex - 1);
      if (m_d.m_type == RampType4Wire || m_d.m_type == RampType3WireRight)
      {
         psur->SetLineColor(RGB(0, 0, 0), false, 3);
         psur->Polyline(rgvLocal, cvertex);
      }
      if (m_d.m_type == RampType4Wire || m_d.m_type == RampType3WireLeft)
      {
         psur->SetLineColor(RGB(0, 0, 0), false, 3);
         psur->Polyline(&rgvLocal[cvertex], cvertex);
      }
   }

   for (int i = 0; i < cvertex; i++)
      if (pfCross[i])
         psur->Line(rgvLocal[i].x, rgvLocal[i].y, rgvLocal[cvertex * 2 - i - 1].x, rgvLocal[cvertex * 2 - i - 1].y);

   delete[] rgvLocal;
   delete[] pfCross;
   delete[] middlePoints;
}


void Ramp::GetBoundingVertices(std::vector<Vertex3Ds>& pvvertex3D)
{
   //!! meh, this is delivering something loosely related to the bounding vertices, but its only used in the cam fitting code so far, so keep for legacy reasons
   float *rgheight1;
   int cvertex;
   const Vertex2D * const rgvLocal = GetRampVertex(cvertex, &rgheight1, NULL, NULL, NULL, HIT_SHAPE_DETAIL_LEVEL, true);

   //pvvertex3D.reserve(pvvertex3D.size() + cvertex * 2);
   Vertex3Ds bbox_min(FLT_MAX, FLT_MAX, FLT_MAX);
   Vertex3Ds bbox_max(-FLT_MAX, -FLT_MAX, -FLT_MAX);
   for (int i = 0; i < cvertex; i++)
   {
      {
      const Vertex3Ds pv(rgvLocal[i].x,rgvLocal[i].y,rgheight1[i] + (float)(2.0*PHYS_SKIN)); // leave room for ball //!! use ballsize
      //pvvertex3D.push_back(pv);
	  bbox_min.x = min(bbox_min.x, pv.x);
	  bbox_min.y = min(bbox_min.y, pv.y);
	  bbox_min.z = min(bbox_min.z, pv.z);
	  bbox_max.x = max(bbox_max.x, pv.x);
	  bbox_max.y = max(bbox_max.y, pv.y);
	  bbox_max.z = max(bbox_max.z, pv.z);
	  }

      const Vertex3Ds pv(rgvLocal[cvertex * 2 - i - 1].x,rgvLocal[cvertex * 2 - i - 1].y,rgheight1[i] + (float)(2.0*PHYS_SKIN)); // leave room for ball //!! use ballsize
      //pvvertex3D.push_back(pv);
	  bbox_min.x = min(bbox_min.x, pv.x);
	  bbox_min.y = min(bbox_min.y, pv.y);
	  bbox_min.z = min(bbox_min.z, pv.z);
	  bbox_max.x = max(bbox_max.x, pv.x);
	  bbox_max.y = max(bbox_max.y, pv.y);
	  bbox_max.z = max(bbox_max.z, pv.z);
   }

   delete[] rgvLocal;
   delete[] rgheight1;

   // returns all 8 corners as this will be used for further transformations later-on
   for (int i = 0; i < 8; i++)
   {
	   const Vertex3Ds pv(
		   (i & 1) ? bbox_min.x : bbox_max.x,
		   (i & 2) ? bbox_min.y : bbox_max.y,
		   (i & 4) ? bbox_min.z : bbox_max.z);

	   pvvertex3D.push_back(pv);
   }
}

void Ramp::AssignHeightToControlPoint(const RenderVertex3D &v, const float height)
{
   for (size_t i = 0; i < m_vdpoint.size(); i++)
   {
      if (m_vdpoint[i]->m_v.x == v.x && m_vdpoint[i]->m_v.y == v.y)
         m_vdpoint[i]->m_calcHeight = height;
   }
}

//
// license:GPLv3+
// Ported at: VisualPinball.Engine/VPT/Ramp/RampMeshGenerator.cs
//

/*
 * Compute the vertices and additional information for the ramp shape.
 *
 * Output:
 *  pcvertex     - number of vertices for the central curve
 *  return value - size 2*cvertex, vertices forming the 2D outline of the ramp
 *                 order: first forward along right side of ramp, then backward along the left side
 *  ppheight     - size cvertex, height of the ramp at the i-th vertex
 *  ppfCross     - size cvertex, true if i-th vertex corresponds to a control point
 *  ppratio      - how far along the ramp length the i-th vertex is, 1=start=bottom, 0=end=top (??)
 */
Vertex2D *Ramp::GetRampVertex(int &pcvertex, float ** const ppheight, bool ** const ppfCross, float ** const ppratio, Vertex2D ** const pMiddlePoints, const float _accuracy, const bool inc_width)
{
   std::vector<RenderVertex3D> vvertex;
   GetCentralCurve(vvertex, _accuracy);
   // vvertex are the 2D vertices forming the central curve of the ramp as seen from above

   const int cvertex = (int)vvertex.size();
   pcvertex = cvertex;
   Vertex2D * const rgvLocal = new Vertex2D[cvertex * 2];

   if (ppheight)
   {
      *ppheight = new float[cvertex];
   }
   if (ppfCross)
   {
      *ppfCross = new bool[cvertex];
   }
   if (ppratio)
   {
      *ppratio = new float[cvertex];
   }
   if (pMiddlePoints)
   {
      *pMiddlePoints = new Vertex2D[cvertex];
   }

   // Compute an approximation to the length of the central curve
   // by adding up the lengths of the line segments.
   float totallength = 0;
   const float bottomHeight = m_d.m_heightbottom + m_ptable->m_tableheight;
   const float topHeight = m_d.m_heighttop + m_ptable->m_tableheight;

   for (int i = 0; i < (cvertex - 1); i++)
   {
      const RenderVertex3D & v1 = vvertex[i];
      const RenderVertex3D & v2 = vvertex[i + 1];

      const float dx = v1.x - v2.x;
      const float dy = v1.y - v2.y;
      const float length = sqrtf(dx*dx + dy*dy);

      totallength += length;
   }

   float currentlength = 0;
   for (int i = 0; i < cvertex; i++)
   {
      // clamp next and prev as ramps do not loop
      const RenderVertex3D & vprev = vvertex[(i > 0) ? i - 1 : i];
      const RenderVertex3D & vnext = vvertex[(i < (cvertex - 1)) ? i + 1 : i];
      const RenderVertex3D & vmiddle = vvertex[i];

      if (ppfCross)
         (*ppfCross)[i] = vmiddle.controlPoint;

      Vertex2D vnormal;
      {
         // Get normal at this point
         // Notice that these values equal the ones in the line
         // equation and could probably be substituted by them.
         Vertex2D v1normal(vprev.y - vmiddle.y, vmiddle.x - vprev.x);   // vector vmiddle-vprev rotated RIGHT
         Vertex2D v2normal(vmiddle.y - vnext.y, vnext.x - vmiddle.x);   // vector vnext-vmiddle rotated RIGHT

         // special handling for beginning and end of the ramp, as ramps do not loop
         if (i == (cvertex - 1))
         {
            v1normal.Normalize();
            vnormal = v1normal;
         }
         else if (i == 0)
         {
            v2normal.Normalize();
            vnormal = v2normal;
         }
         else
         {
            v1normal.Normalize();
            v2normal.Normalize();

            if (fabsf(v1normal.x - v2normal.x) < 0.0001f && fabsf(v1normal.y - v2normal.y) < 0.0001f)
            {
               // Two parallel segments
               vnormal = v1normal;
            }
            else
            {
               // Find intersection of the two edges meeting this points, but
               // shift those lines outwards along their normals

               // First line
               const float A = vprev.y - vmiddle.y;
               const float B = vmiddle.x - vprev.x;

               // Shift line along the normal
               const float C = A*(v1normal.x - vprev.x) + B*(v1normal.y - vprev.y);

               // Second line
               const float D = vnext.y - vmiddle.y;
               const float E = vmiddle.x - vnext.x;

               // Shift line along the normal
               const float F = D*(v2normal.x - vnext.x) + E*(v2normal.y - vnext.y);

               const float det = A*E - B*D;
               const float inv_det = (det != 0.0f) ? 1.0f / det : 0.0f;

               const float intersectx = (B*F - E*C)*inv_det;
               const float intersecty = (C*D - A*F)*inv_det;

               vnormal.x = vmiddle.x - intersectx;
               vnormal.y = vmiddle.y - intersecty;
            }
         }
      }

      // Update current length along the ramp.
      {
         const float dx = vprev.x - vmiddle.x;
         const float dy = vprev.y - vmiddle.y;
         const float length = sqrtf(dx*dx + dy*dy);

         currentlength += length;
      }

      const float percentage = currentlength / totallength;
      float widthcur = percentage * (m_d.m_widthtop - m_d.m_widthbottom) + m_d.m_widthbottom;
      if (ppheight)
      {
         (*ppheight)[i] = vmiddle.z + percentage * (topHeight - bottomHeight) + bottomHeight;
      }

      AssignHeightToControlPoint(vvertex[i], vmiddle.z + percentage * (topHeight - bottomHeight) + bottomHeight);
      if (ppratio)
      {
         (*ppratio)[i] = 1.0f - percentage;
      }
      // only change the width if we want to create vertices for rendering or for the editor
      // the collision engine uses flat type ramps
      if (isHabitrail() && m_d.m_type != RampType1Wire)
      {
         widthcur = m_d.m_wireDistanceX;
         if (inc_width)
            widthcur += 20.0f;
      }
      else if (m_d.m_type == RampType1Wire)
      {
         widthcur = m_d.m_wireDiameter;
      }

      if (pMiddlePoints)
      {
         (*pMiddlePoints)[i] = Vertex2D(vmiddle.x, vmiddle.y) + vnormal;
      }
      rgvLocal[i                  ] = Vertex2D(vmiddle.x, vmiddle.y) + (widthcur*0.5f) * vnormal;
      rgvLocal[cvertex * 2 - i - 1] = Vertex2D(vmiddle.x, vmiddle.y) - (widthcur*0.5f) * vnormal;
   }

   return rgvLocal;
}

// Ported at: VisualPinball.Engine/VPT/Ramp/Ramp.cs

float Ramp::GetSurfaceHeight(float x, float y) const
{
   std::vector<RenderVertex3D> vvertex;
   GetCentralCurve(vvertex);

   int iSeg;
   Vertex2D vOut;
   ClosestPointOnPolygon(vvertex, Vertex2D(x, y), vOut, iSeg, false);

   if (iSeg == -1)
      return 0.0f; // Object is not on ramp path

   // Go through vertices (including iSeg itself) counting control points until iSeg
   float totallength = 0.f;
   float startlength = 0.f;

   const int cvertex = (int)vvertex.size();
   for (int i2 = 1; i2 < cvertex; i2++)
   {
      const float dx = vvertex[i2].x - vvertex[i2 - 1].x;
      const float dy = vvertex[i2].y - vvertex[i2 - 1].y;
      const float len = sqrtf(dx*dx + dy*dy);
      if (i2 <= iSeg)
         startlength += len;

      totallength += len;
   }

   const float dx = vOut.x - vvertex[iSeg].x;
   const float dy = vOut.y - vvertex[iSeg].y;
   const float len = sqrtf(dx*dx + dy*dy);
   startlength += len; // Add the distance the object is between the two closest polyline segments.  Matters mostly for straight edges. Z does not respect that yet!

   const float topHeight = m_d.m_heighttop + m_ptable->m_tableheight;
   const float bottomHeight = m_d.m_heightbottom + m_ptable->m_tableheight;

   return vvertex[iSeg].z + (startlength / totallength) * (topHeight - bottomHeight) + bottomHeight;
}

//
// end of license:GPLv3+, back to 'old MAME'-like
//

void Ramp::GetTimers(vector<HitTimer*> &pvht)
{
   IEditable::BeginPlay();

   HitTimer * const pht = new HitTimer();
   pht->m_interval = m_d.m_tdr.m_TimerInterval >= 0 ? max(m_d.m_tdr.m_TimerInterval, MAX_TIMER_MSEC_INTERVAL) : -1;
   pht->m_nextfire = pht->m_interval;
   pht->m_pfe = (IFireEvents *)this;

   m_phittimer = pht;

   if (m_d.m_tdr.m_TimerEnabled)
      pvht.push_back(pht);
}

// Ported at: VisualPinball.Engine/VPT/Ramp/RampHitGenerator.cs

void Ramp::GetHitShapes(vector<HitObject*> &pvho)
{
   float *rgheight1;
   int cvertex;
   const Vertex2D * const rgvLocal = GetRampVertex(cvertex, &rgheight1, NULL, NULL, NULL, HIT_SHAPE_DETAIL_LEVEL, true);

   float wallheightright, wallheightleft;

   if (m_d.m_type == RampTypeFlat)
   {
      wallheightright = m_d.m_rightwallheight;
      wallheightleft = m_d.m_leftwallheight;
   }
   else if (m_d.m_type == RampType1Wire) //add check for 1 wire
   {
      // backwards compatible physics
      wallheightright = 31.0f;
      wallheightleft = 31.0f;
   }
   else if (m_d.m_type == RampType2Wire)
   {
      // backwards compatible physics
      wallheightright = 31.0f;
      wallheightleft = 31.0f;
   }
   else if (m_d.m_type == RampType4Wire)
   {
      wallheightright = 62.0f;
      wallheightleft = 62.0f;
   }
   else if (m_d.m_type == RampType3WireRight)
   {
      wallheightright = 62.0f;
      wallheightleft = (float)(6 + 12.5);
   }
   else if (m_d.m_type == RampType3WireLeft)
   {
      wallheightright = (float)(6 + 12.5);
      wallheightleft = 62.0f;
   }
   else
   {
      ShowError("Unknown Ramp type");
      return;
   }

   // Add line segments for right ramp wall.
   if (wallheightright > 0.f)
   {
      for (int i = 0; i < (cvertex - 1); i++)
      {
         const Vertex2D &pv2 = rgvLocal[i];
         const Vertex2D &pv3 = rgvLocal[i + 1];

         AddWallLineSeg(pvho, pv2, pv3, (i > 0), rgheight1[i], rgheight1[i + 1], wallheightright);
         AddWallLineSeg(pvho, pv3, pv2, (i < (cvertex - 2)), rgheight1[i], rgheight1[i + 1], wallheightright);

         // add joints at start and end of right wall
         if (i == 0)
            AddJoint2D(pvho, pv2, rgheight1[0], rgheight1[0] + wallheightright);
         if (i == cvertex - 2)
            AddJoint2D(pvho, pv3, rgheight1[cvertex - 1], rgheight1[cvertex - 1] + wallheightright);
      }
   }

   // Add line segments for left ramp wall.
   if (wallheightleft > 0.f)
   {
      for (int i = 0; i < (cvertex - 1); i++)
      {
         const Vertex2D &pv2 = rgvLocal[cvertex + i];
         const Vertex2D &pv3 = rgvLocal[cvertex + i + 1];

         AddWallLineSeg(pvho, pv2, pv3, (i > 0), rgheight1[cvertex - i - 2], rgheight1[cvertex - i - 1], wallheightleft);
         AddWallLineSeg(pvho, pv3, pv2, (i < (cvertex - 2)), rgheight1[cvertex - i - 2], rgheight1[cvertex - i - 1], wallheightleft);

         // add joints at start and end of left wall
         if (i == 0)
            AddJoint2D(pvho, pv2, rgheight1[cvertex - 1], rgheight1[cvertex - 1] + wallheightleft);
         if (i == cvertex - 2)
            AddJoint2D(pvho, pv3, rgheight1[0], rgheight1[0] + wallheightleft);
      }
   }

   // Add hit triangles for the ramp floor.
   {
      HitTriangle *ph3dpolyOld = NULL;
      const Vertex2D *pv3, *pv4;

      for (int i = 0; i < (cvertex - 1); i++)
      {
         /*
          * Layout of one ramp quad seen from above, ramp direction is bottom to top:
          *
          *    3 - - 4
          *    | \   |
          *    |   \ |
          *    2 - - 1
          */
         const Vertex2D *pv1, *pv2;
         pv1 = &rgvLocal[i];                   // i-th right
         pv2 = &rgvLocal[cvertex * 2 - i - 1]; // i-th left
         pv3 = &rgvLocal[cvertex * 2 - i - 2]; // (i+1)-th left
         pv4 = &rgvLocal[i + 1];               // (i+1)-th right

         {
            // left ramp floor triangle, CCW order
            const Vertex3Ds rgv3D[3] = {
             Vertex3Ds(pv2->x, pv2->y, rgheight1[i]),
             Vertex3Ds(pv1->x, pv1->y, rgheight1[i]),
             Vertex3Ds(pv3->x, pv3->y, rgheight1[i + 1])};

            // add joint for starting edge of ramp
            if (i == 0)
               AddJoint(pvho, rgv3D[0], rgv3D[1]);

            // add joint for left edge
            AddJoint(pvho, rgv3D[0], rgv3D[2]);

            HitTriangle * const ph3dpoly = new HitTriangle(rgv3D); //!! this is not efficient at all, use native triangle-soup directly somehow

            if (ph3dpoly->IsDegenerate())       // degenerate triangles happen if width is 0 at some point
            {
               delete ph3dpoly;
            }
            else
            {
               SetupHitObject(pvho, ph3dpoly);

               CheckJoint(pvho, ph3dpolyOld, ph3dpoly);
               ph3dpolyOld = ph3dpoly;
            }
         }

         // right ramp floor triangle, CCW order
         const Vertex3Ds rgv3D[3] = {
          Vertex3Ds(pv3->x, pv3->y, rgheight1[i + 1]),
          Vertex3Ds(pv1->x, pv1->y, rgheight1[i]),
          Vertex3Ds(pv4->x, pv4->y, rgheight1[i + 1])};

         // add joint for right edge
         AddJoint(pvho, rgv3D[1], rgv3D[2]);

         HitTriangle * const ph3dpoly = new HitTriangle(rgv3D);
         if (ph3dpoly->IsDegenerate())
         {
            delete ph3dpoly;
         }
         else
         {
            SetupHitObject(pvho, ph3dpoly);

            CheckJoint(pvho, ph3dpolyOld, ph3dpoly);
            ph3dpolyOld = ph3dpoly;
         }
      }

      if (cvertex >= 2)
      {
         // add joint for final edge of ramp
         const Vertex3Ds v1(pv4->x, pv4->y, rgheight1[cvertex - 1]);
         const Vertex3Ds v2(pv3->x, pv3->y, rgheight1[cvertex - 1]);
         AddJoint(pvho, v1, v2);
      }
   }

   // add outside bottom, 
   // joints at the intersections are not needed since the inner surface has them
   // this surface is identical... except for the direction of the normal face.
   // hence the joints protect both surface edges from having a fall through

   for (int i = 0; i < (cvertex - 1); i++)
   {
      // see sketch above
      const Vertex2D& pv1 = rgvLocal[i];
      const Vertex2D& pv2 = rgvLocal[cvertex * 2 - i - 1];
      const Vertex2D& pv3 = rgvLocal[cvertex * 2 - i - 2];
      const Vertex2D& pv4 = rgvLocal[i + 1];

      {
         // left ramp triangle, order CW
         const Vertex3Ds rgv3D[3] = {
            Vertex3Ds(pv1.x, pv1.y, rgheight1[i]),
            Vertex3Ds(pv2.x, pv2.y, rgheight1[i]),
            Vertex3Ds(pv3.x, pv3.y, rgheight1[i + 1]) };

         HitTriangle * const ph3dpoly = new HitTriangle(rgv3D);
         if (ph3dpoly->IsDegenerate())
         {
            delete ph3dpoly;
         }
         else
         {
            SetupHitObject(pvho, ph3dpoly);
         }
      }

      // right ramp triangle, order CW
      const Vertex3Ds rgv3D[3] = {
         Vertex3Ds(pv3.x, pv3.y, rgheight1[i + 1]),
         Vertex3Ds(pv4.x, pv4.y, rgheight1[i + 1]),
         Vertex3Ds(pv1.x, pv1.y, rgheight1[i]) };

      HitTriangle * const ph3dpoly = new HitTriangle(rgv3D);
      if (ph3dpoly->IsDegenerate())
      {
         delete ph3dpoly;
      }
      else
      {
         SetupHitObject(pvho, ph3dpoly);
      }
   }

   delete[] rgheight1;
   delete[] rgvLocal;
}

void Ramp::GetHitShapesDebug(vector<HitObject*> &pvho)
{
}

void Ramp::CheckJoint(vector<HitObject*> &pvho, const HitTriangle * const ph3d1, const HitTriangle * const ph3d2)
{
   if (ph3d1)   // may be null in case of degenerate triangles
   {
      const Vertex3Ds vjointnormal = CrossProduct(ph3d1->m_normal, ph3d2->m_normal);
      if (vjointnormal.LengthSquared() < 1e-8f)
         return;  // coplanar triangles need no joints
   }

   // By convention of the calling function, points 1 [0] and 2 [1] of the second polygon will
   // be the common-edge points
   AddJoint(pvho, ph3d2->m_rgv[0], ph3d2->m_rgv[1]);
}

void Ramp::AddJoint(vector<HitObject*> &pvho, const Vertex3Ds& v1, const Vertex3Ds& v2)
{
   SetupHitObject(pvho, new HitLine3D(v1, v2));
}

void Ramp::AddJoint2D(vector<HitObject*> &pvho, const Vertex2D& p, const float zlow, const float zhigh)
{
   SetupHitObject(pvho, new HitLineZ(p, zlow, zhigh));
}

void Ramp::AddWallLineSeg(vector<HitObject*> &pvho, const Vertex2D &pv1, const Vertex2D &pv2, const bool pv3_exists, const float height1, const float height2, const float wallheight)
{
   //!! Hit-walls are still done via 2D line segments with only a single lower and upper border, so the wall will always reach below and above the actual ramp -between- two points of the ramp

   // Thus, subdivide until at some point the approximation error is 'subtle' enough so that one will usually not notice (i.e. dependent on ball size)
   if (height2-height1 > (float)(2.0*PHYS_SKIN)) //!! use ballsize
   {
      AddWallLineSeg(pvho, pv1, (pv1+pv2)*0.5f, pv3_exists, height1, (height1+height2)*0.5f, wallheight);
      AddWallLineSeg(pvho, (pv1+pv2)*0.5f, pv2, true,       (height1+height2)*0.5f, height2, wallheight);
   }
   else
   {
      LineSeg * const plineseg = new LineSeg(pv1, pv2, height1, height2+wallheight);

      SetupHitObject(pvho, plineseg);

      if (pv3_exists)
         AddJoint2D(pvho, pv1, height1, height2+wallheight);
   }
}

void Ramp::SetupHitObject(vector<HitObject*> &pvho, HitObject * obj)
{
   const Material * const mat = m_ptable->GetMaterial(m_d.m_szPhysicsMaterial);
   if (!m_d.m_overwritePhysics)
   {
      obj->m_elasticity = mat->m_fElasticity;
      obj->SetFriction(mat->m_fFriction);
      obj->m_scatter = ANGTORAD(mat->m_fScatterAngle);
   }
   else
   {
      obj->m_elasticity = m_d.m_elasticity;
      obj->SetFriction(m_d.m_friction);
      obj->m_scatter = ANGTORAD(m_d.m_scatter);
   }

   obj->m_threshold = m_d.m_threshold;
   // the ramp is of type ePrimitive for triggering the event in HitTriangle::Collide()
   obj->m_ObjType = ePrimitive;
   obj->m_enabled = m_d.m_collidable;
   obj->m_obj = (IFireEvents*)this;
   obj->m_fe = m_d.m_hitEvent;

   pvho.push_back(obj);
   m_vhoCollidable.push_back(obj); //remember hit components of primitive
}

//
// end of license:GPLv3+, back to 'old MAME'-like
//

void Ramp::EndPlay()
{
   IEditable::EndPlay();
   m_vhoCollidable.clear();

   if (m_dynamicVertexBuffer) {
      m_dynamicVertexBuffer->release();
      m_dynamicVertexBuffer = 0;
      m_dynamicVertexBufferRegenerate = true;
   }

   if (m_dynamicIndexBuffer) {
      m_dynamicIndexBuffer->release();
      m_dynamicIndexBuffer = 0;
   }
   if (m_dynamicVertexBuffer2) {
      m_dynamicVertexBuffer2->release();
      m_dynamicVertexBuffer2 = 0;
   }
}

float Ramp::GetDepth(const Vertex3Ds& viewDir) const
{
   // TODO: improve
   const Vertex2D center2D = GetCenter();
   const float centerZ = 0.5f * (m_d.m_heightbottom + m_d.m_heighttop);
   return m_d.m_depthBias + viewDir.x * center2D.x + viewDir.y * center2D.y + viewDir.z * centerZ;
}

//
// license:GPLv3+
// Ported at: VisualPinball.Engine/VPT/Ramp/RampHitGenerator.cs
//

bool Ramp::isHabitrail() const
{
   return m_d.m_type == RampType4Wire
       || m_d.m_type == RampType1Wire
       || m_d.m_type == RampType2Wire
       || m_d.m_type == RampType3WireLeft
       || m_d.m_type == RampType3WireRight;
}

//
// end of license:GPLv3+, back to 'old MAME'-like
//

void Ramp::RenderStaticHabitrail(const Material * const mat)
{
   RenderDevice * const pd3dDevice = g_pplayer->m_pin3d.m_pd3dPrimaryDevice;

   pd3dDevice->basicShader->SetMaterial(mat);

   pd3dDevice->SetRenderState(RenderDevice::DEPTHBIAS, 0);
   pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, RenderDevice::RS_TRUE);
   pd3dDevice->SetRenderState(RenderDevice::CULLMODE, RenderDevice::CULL_NONE);

   Texture * const pin = m_ptable->GetImage(m_d.m_szImage);
   if (!pin)
      pd3dDevice->basicShader->SetTechnique(mat->m_bIsMetal ? "basic_without_texture_isMetal" : "basic_without_texture_isNotMetal");
   else
   {
      pd3dDevice->basicShader->SetTexture("Texture0", pin, false);
      pd3dDevice->basicShader->SetTechnique(mat->m_bIsMetal ? "basic_with_texture_isMetal" : "basic_with_texture_isNotMetal");

      //g_pplayer->m_pin3d.SetTextureFilter(0, TEXTURE_MODE_TRILINEAR);
   }

   if (m_d.m_type == RampType2Wire)
   {
      Matrix3D matTrafo, matTemp;
      matTrafo.SetIdentity();
      matTrafo._43 = 3.0f;                // raise the wire a bit because the ball runs on a flat ramp physically
      g_pplayer->UpdateBasicShaderMatrix(matTrafo);
      pd3dDevice->basicShader->Begin(0);
      pd3dDevice->DrawIndexedPrimitiveVB(RenderDevice::TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, m_dynamicVertexBuffer, 0, m_numVertices, m_dynamicIndexBuffer, 0, m_numIndices);
      pd3dDevice->DrawIndexedPrimitiveVB(RenderDevice::TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, m_dynamicVertexBuffer2, 0, m_numVertices, m_dynamicIndexBuffer, 0, m_numIndices);
      pd3dDevice->basicShader->End();
      g_pplayer->UpdateBasicShaderMatrix();
   }
   else if (m_d.m_type == RampType4Wire)
   {
      Matrix3D matTrafo, matTemp;
      matTrafo.SetIdentity();
      matTrafo._43 = m_d.m_wireDistanceY*0.5f;
      g_pplayer->UpdateBasicShaderMatrix(matTrafo);
      pd3dDevice->basicShader->Begin(0);
      pd3dDevice->DrawIndexedPrimitiveVB(RenderDevice::TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, m_dynamicVertexBuffer, 0, m_numVertices, m_dynamicIndexBuffer, 0, m_numIndices);
      pd3dDevice->DrawIndexedPrimitiveVB(RenderDevice::TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, m_dynamicVertexBuffer2, 0, m_numVertices, m_dynamicIndexBuffer, 0, m_numIndices);
      pd3dDevice->basicShader->End();
      matTrafo.SetIdentity();
      matTrafo._43 = 3.0f;                // raise the wire a bit because the ball runs on a flat ramp physically
      g_pplayer->UpdateBasicShaderMatrix(matTrafo);
      pd3dDevice->basicShader->Begin(0);
      pd3dDevice->DrawIndexedPrimitiveVB(RenderDevice::TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, m_dynamicVertexBuffer, 0, m_numVertices, m_dynamicIndexBuffer, 0, m_numIndices);
      pd3dDevice->DrawIndexedPrimitiveVB(RenderDevice::TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, m_dynamicVertexBuffer2, 0, m_numVertices, m_dynamicIndexBuffer, 0, m_numIndices);
      pd3dDevice->basicShader->End();
      g_pplayer->UpdateBasicShaderMatrix();
   }
   else if (m_d.m_type == RampType3WireLeft)
   {
      Matrix3D matTrafo, matTemp;
      matTrafo.SetIdentity();
      matTrafo._43 = m_d.m_wireDistanceY*0.5f;
      g_pplayer->UpdateBasicShaderMatrix(matTrafo);
      pd3dDevice->basicShader->Begin(0);
      pd3dDevice->DrawIndexedPrimitiveVB(RenderDevice::TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, m_dynamicVertexBuffer2, 0, m_numVertices, m_dynamicIndexBuffer, 0, m_numIndices);
      pd3dDevice->basicShader->End();
      matTrafo.SetIdentity();
      matTrafo._43 = 3.0f;                // raise the wire a bit because the ball runs on a flat ramp physically
      g_pplayer->UpdateBasicShaderMatrix(matTrafo);
      pd3dDevice->basicShader->Begin(0);
      pd3dDevice->DrawIndexedPrimitiveVB(RenderDevice::TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, m_dynamicVertexBuffer, 0, m_numVertices, m_dynamicIndexBuffer, 0, m_numIndices);
      pd3dDevice->DrawIndexedPrimitiveVB(RenderDevice::TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, m_dynamicVertexBuffer2, 0, m_numVertices, m_dynamicIndexBuffer, 0, m_numIndices);
      pd3dDevice->basicShader->End();
      g_pplayer->UpdateBasicShaderMatrix();
   }
   else if (m_d.m_type == RampType3WireRight)
   {
      Matrix3D matTrafo, matTemp;
      matTrafo.SetIdentity();
      matTrafo._43 = m_d.m_wireDistanceY*0.5f;
      g_pplayer->UpdateBasicShaderMatrix(matTrafo);
      pd3dDevice->basicShader->Begin(0);
      pd3dDevice->DrawIndexedPrimitiveVB(RenderDevice::TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, m_dynamicVertexBuffer, 0, m_numVertices, m_dynamicIndexBuffer, 0, m_numIndices);
      pd3dDevice->basicShader->End();
      matTrafo.SetIdentity();
      matTrafo._43 = 3.0f;                // raise the wire a bit because the ball runs on a flat ramp physically
      g_pplayer->UpdateBasicShaderMatrix(matTrafo);
      pd3dDevice->basicShader->Begin(0);
      pd3dDevice->DrawIndexedPrimitiveVB(RenderDevice::TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, m_dynamicVertexBuffer, 0, m_numVertices, m_dynamicIndexBuffer, 0, m_numIndices);
      pd3dDevice->DrawIndexedPrimitiveVB(RenderDevice::TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, m_dynamicVertexBuffer2, 0, m_numVertices, m_dynamicIndexBuffer, 0, m_numIndices);
      pd3dDevice->basicShader->End();
      g_pplayer->UpdateBasicShaderMatrix();
   }
   else
   {
      pd3dDevice->basicShader->Begin(0);
      pd3dDevice->DrawIndexedPrimitiveVB(RenderDevice::TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, m_dynamicVertexBuffer, 0, m_numVertices, m_dynamicIndexBuffer, 0, m_numIndices);
      pd3dDevice->basicShader->End();
   }

   //pd3dDevice->SetRenderState(RenderDevice::CULLMODE, RenderDevice::CULL_CCW);
}

//
// license:GPLv3+
// Ported at: VisualPinball.Engine/VPT/Ramp/RampMeshGenerator.cs
//

void Ramp::CreateWire(const int numRings, const int numSegments, const Vertex2D * const midPoints, Vertex3D_NoTex2 * const rgvbuf)
{
   Vertex3Ds prevB;
   for (int i = 0, index = 0; i < numRings; i++)
   {
      const int i2 = (i == (numRings - 1)) ? i : i + 1;
      const float height = m_rgheightInit[i];

      Vertex3Ds tangent(midPoints[i2].x - midPoints[i].x, midPoints[i2].y - midPoints[i].y, m_rgheightInit[i2]- m_rgheightInit[i]);
      if (i == numRings - 1)
      {
         // for the last spline point use the previous tangent again, otherwise we won't see the complete wire (it stops one control point too early)
         tangent.x = midPoints[i].x - midPoints[i - 1].x;
         tangent.y = midPoints[i].y - midPoints[i - 1].y;
      }
	  Vertex3Ds binorm;
	  Vertex3Ds normal;
	  if (i == 0)
      {
         Vertex3Ds up(midPoints[i2].x + midPoints[i].x, midPoints[i2].y + midPoints[i].y, m_rgheightInit[i2] - height);
         normal = CrossProduct(tangent, up);     //normal
         binorm = CrossProduct(tangent, normal);
      }
      else
      {
         normal = CrossProduct(prevB, tangent);
         binorm = CrossProduct(tangent, normal);
      }
      binorm.Normalize();
      normal.Normalize();
      prevB = binorm;

      const float inv_numRings = 1.0f / (float)numRings;
      const float inv_numSegments = 1.0f / (float)numSegments;
	  const float u = (float)i*inv_numRings;
	  for (int j = 0; j < numSegments; j++, index++)
      {
         const float v = ((float)j + u)*inv_numSegments;
         const Vertex3Ds tmp = GetRotatedAxis((float)j*(360.0f*inv_numSegments), tangent, normal) * ((float)m_d.m_wireDiameter*0.5f);
         rgvbuf[index].x = midPoints[i].x + tmp.x;
         rgvbuf[index].y = midPoints[i].y + tmp.y;
         rgvbuf[index].z = height + tmp.z;
         //texel
         rgvbuf[index].tu = u;
         rgvbuf[index].tv = v;
         const Vertex3Ds n(rgvbuf[index].x - midPoints[i].x, rgvbuf[index].y - midPoints[i].y, rgvbuf[index].z - height);
         const float len = 1.0f / sqrtf(n.x*n.x + n.y*n.y + n.z*n.z);
         rgvbuf[index].nx = n.x*len;
         rgvbuf[index].ny = n.y*len;
         rgvbuf[index].nz = n.z*len;
      }
   }
}

void Ramp::GenerateWireMesh(Vertex3D_NoTex2 **meshBuf1, Vertex3D_NoTex2 **meshBuf2)
{
   Vertex2D * middlePoints = 0;
   int accuracy;
   if (m_ptable->GetDetailLevel() < 5)
   {
      accuracy = 6;
   }
   else if (m_ptable->GetDetailLevel() >= 5 && m_ptable->GetDetailLevel() < 8)
   {
      accuracy = 8;
   }
   else
   {
      accuracy = (int)((float)m_ptable->GetDetailLevel()*1.3f); // see below
   }

   // as solid ramps are rendered into the static buffer, always use maximum precision
   const Material * const mat = m_ptable->GetMaterial(m_d.m_szMaterial);
   if (!mat->m_bOpacityActive)
      accuracy = (int)(10.f*1.3f); // see above

   if (m_rgheightInit)
       delete [] m_rgheightInit;

   int splinePoints;
   const Vertex2D * const rgvLocal = GetRampVertex(splinePoints, &m_rgheightInit, NULL, NULL, (m_d.m_type != RampType1Wire) ? NULL : &middlePoints, -1, false);

   const int numRings = splinePoints;
   const int numSegments = accuracy;
   m_numVertices = numRings*numSegments;
   m_numIndices = 6 * m_numVertices; //m_numVertices*2+2;

   if (*meshBuf1 == NULL)
      *meshBuf1 = new Vertex3D_NoTex2[m_numVertices];

   Vertex2D *tmpPoints = new Vertex2D[splinePoints];
   for (int i = 0; i < splinePoints; i++)
      tmpPoints[i] = rgvLocal[splinePoints * 2 - i - 1];

   m_vertBuffer = new Vertex3D_NoTex2[m_numVertices];
   m_vertBuffer2 = NULL;
   m_meshIndices.resize(m_numIndices);

   if (m_d.m_type != RampType1Wire)
   {
      m_vertBuffer2 = new Vertex3D_NoTex2[m_numVertices];
      if (*meshBuf2 == NULL)
         *meshBuf2 = new Vertex3D_NoTex2[m_numVertices];
   }

   Vertex3D_NoTex2 *buf1 = *meshBuf1;

   if (m_d.m_type != RampType1Wire)
   {
      CreateWire(numRings, numSegments, rgvLocal, m_vertBuffer);
      CreateWire(numRings, numSegments, tmpPoints, m_vertBuffer2);
   }
   else
      CreateWire(numRings, numSegments, middlePoints, m_vertBuffer);

   // calculate faces
   for (int i = 0; i < numRings - 1; i++)
   {
      for (int j = 0; j < numSegments; j++)
      {
         int quad[4];
         quad[0] = i*numSegments + j;

         if (j != numSegments - 1)
            quad[1] = i*numSegments + j + 1;
         else
            quad[1] = i*numSegments;

         if (i != numRings - 1)
         {
            quad[2] = (i + 1)*numSegments + j;
            if (j != numSegments - 1)
               quad[3] = (i + 1)*numSegments + j + 1;
            else
               quad[3] = (i + 1)*numSegments;
         }
         else
         {
            quad[2] = j;
            if (j != numSegments - 1)
               quad[3] = j + 1;
            else
               quad[3] = 0;
         }

         const unsigned int offs = (i*numSegments + j) * 6;
         m_meshIndices[offs] = quad[0];
         m_meshIndices[offs + 1] = quad[1];
         m_meshIndices[offs + 2] = quad[2];
         m_meshIndices[offs + 3] = quad[3];
         m_meshIndices[offs + 4] = quad[2];
         m_meshIndices[offs + 5] = quad[1];
      }
   }

   memcpy(buf1, m_vertBuffer, sizeof(Vertex3D_NoTex2)*m_numVertices);

   if (m_d.m_type != RampType1Wire)
   {
      memcpy(*meshBuf2, m_vertBuffer2, sizeof(Vertex3D_NoTex2)*m_numVertices);
   }

   // not necessary to reorder
   /*WORD* const tmp = reorderForsyth(m_meshIndices, m_numVertices);
   if (tmp != NULL)
   {
   memcpy(m_meshIndices.data(), tmp, m_meshIndices.size()*sizeof(WORD));
   delete[] tmp;
   }*/

   delete[] rgvLocal;
   if (middlePoints)
      delete[] middlePoints;
   delete[] tmpPoints;
}

//
// end of license:GPLv3+, back to 'old MAME'-like
//

void Ramp::PrepareHabitrail()
{
   m_dynamicVertexBufferRegenerate = false;
   Vertex3D_NoTex2 *tmpBuf1 = NULL;
   Vertex3D_NoTex2 *tmpBuf2 = NULL;
   GenerateWireMesh(&tmpBuf1, &tmpBuf2);

   if (m_dynamicVertexBuffer)
      m_dynamicVertexBuffer->release();
   if (m_dynamicVertexBuffer2)
      m_dynamicVertexBuffer2->release();

   RenderDevice * const pd3dDevice = g_pplayer->m_pin3d.m_pd3dPrimaryDevice;

   pd3dDevice->CreateVertexBuffer(m_numVertices, 0, MY_D3DFVF_NOTEX2_VERTEX, &m_dynamicVertexBuffer); //!! use USAGE_DYNAMIC if it would actually be "really" dynamic
   if (m_d.m_type != RampType1Wire)
      pd3dDevice->CreateVertexBuffer(m_numVertices, 0, MY_D3DFVF_NOTEX2_VERTEX, &m_dynamicVertexBuffer2); //!! use USAGE_DYNAMIC if it would actually be "really" dynamic

   // Draw the floor of the ramp.
   Vertex3D_NoTex2 *buf;
   m_dynamicVertexBuffer->lock(0, 0, (void**)&buf, VertexBuffer::WRITEONLY);
   memcpy(buf, tmpBuf1, sizeof(Vertex3D_NoTex2)*m_numVertices);
   m_dynamicVertexBuffer->unlock();

   if (m_d.m_type != RampType1Wire)
   {
      Vertex3D_NoTex2 *buf2;
      m_dynamicVertexBuffer2->lock(0, 0, (void**)&buf2, VertexBuffer::WRITEONLY);
      memcpy(buf2, tmpBuf2, sizeof(Vertex3D_NoTex2)*m_numVertices);
      m_dynamicVertexBuffer2->unlock();
   }

   if (m_dynamicIndexBuffer)
      m_dynamicIndexBuffer->release();

   m_dynamicIndexBuffer = pd3dDevice->CreateAndFillIndexBuffer(m_meshIndices);

   delete[] m_vertBuffer;
   delete[] tmpBuf1;
   if (m_d.m_type != RampType1Wire)
   {
      delete[] m_vertBuffer2;
      delete[] tmpBuf2;
   }

   m_meshIndices.clear();
}

void Ramp::RenderSetup()
{
   if (m_d.m_visible)
   {
      if (isHabitrail())
         PrepareHabitrail();
      else
         GenerateVertexBuffer();
   }
}

void Ramp::RenderStatic()
{
   // return if not Visible
   if (!m_d.m_visible)
      return;

   if (m_ptable->m_reflectionEnabled && !m_d.m_reflectionEnabled)
      return;

   const Material * const mat = m_ptable->GetMaterial(m_d.m_szMaterial);

   // dont render transparent ramps into static buffer, these are done per frame later-on
   if (mat->m_bOpacityActive)
      return;

   /* TODO: This is a misnomer right now, but clamp fixes some visual glitches (single-pixel lines)
    * with transparent textures. Probably the option should simply be renamed to ImageModeClamp,
    * since the texture coordinates always stay within [0,1] anyway.
    */
   //if (m_d.m_imagealignment == ImageModeWrap)
   //    pd3dDevice->SetTextureAddressMode(0, RenderDevice::TEX_CLAMP);

   if (isHabitrail())
      RenderStaticHabitrail(mat);
   else
      RenderRamp(mat);
}

void Ramp::SetObjectPos()
{
    m_vpinball->SetObjectPosCur(0, 0);
}

void Ramp::MoveOffset(const float dx, const float dy)
{
   for (size_t i = 0; i < m_vdpoint.size(); i++)
   {
      CComObject<DragPoint> * const pdp = m_vdpoint[i];

      pdp->m_v.x += dx;
      pdp->m_v.y += dy;
   }
}

void Ramp::ClearForOverwrite()
{
   ClearPointsForOverwrite();
}

HRESULT Ramp::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, const bool backupForPlay)
{
   BiffWriter bw(pstm, hcrypthash);

   bw.WriteFloat(FID(HTBT), m_d.m_heightbottom);
   bw.WriteFloat(FID(HTTP), m_d.m_heighttop);
   bw.WriteFloat(FID(WDBT), m_d.m_widthbottom);
   bw.WriteFloat(FID(WDTP), m_d.m_widthtop);
   bw.WriteString(FID(MATR), m_d.m_szMaterial);
   bw.WriteBool(FID(TMON), m_d.m_tdr.m_TimerEnabled);
   bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
   bw.WriteInt(FID(TYPE), m_d.m_type);
   bw.WriteWideString(FID(NAME), m_wzName);
   bw.WriteString(FID(IMAG), m_d.m_szImage);
   bw.WriteInt(FID(ALGN), m_d.m_imagealignment);
   bw.WriteBool(FID(IMGW), m_d.m_imageWalls);
   bw.WriteFloat(FID(WLHL), m_d.m_leftwallheight);
   bw.WriteFloat(FID(WLHR), m_d.m_rightwallheight);
   bw.WriteFloat(FID(WVHL), m_d.m_leftwallheightvisible);
   bw.WriteFloat(FID(WVHR), m_d.m_rightwallheightvisible);
   bw.WriteBool(FID(HTEV), m_d.m_hitEvent);
   bw.WriteFloat(FID(THRS), m_d.m_threshold);
   bw.WriteFloat(FID(ELAS), m_d.m_elasticity);
   bw.WriteFloat(FID(RFCT), m_d.m_friction);
   bw.WriteFloat(FID(RSCT), m_d.m_scatter);
   bw.WriteBool(FID(CLDR), m_d.m_collidable);
   bw.WriteBool(FID(RVIS), m_d.m_visible);
   bw.WriteFloat(FID(RADB), m_d.m_depthBias);
   bw.WriteFloat(FID(RADI), m_d.m_wireDiameter);
   bw.WriteFloat(FID(RADX), m_d.m_wireDistanceX);
   bw.WriteFloat(FID(RADY), m_d.m_wireDistanceY);
   bw.WriteBool(FID(REEN), m_d.m_reflectionEnabled);
   bw.WriteString(FID(MAPH), m_d.m_szPhysicsMaterial);
   bw.WriteBool(FID(OVPH), m_d.m_overwritePhysics);

   ISelect::SaveData(pstm, hcrypthash);

   bw.WriteTag(FID(PNTS));
   HRESULT hr;
   if (FAILED(hr = SavePointData(pstm, hcrypthash)))
      return hr;

   bw.WriteTag(FID(ENDB));

   return S_OK;
}

HRESULT Ramp::InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   SetDefaults(false);

   BiffReader br(pstm, this, pid, version, hcrypthash, hcryptkey);

   m_ptable = ptable;

   br.Load();
   return S_OK;
}

bool Ramp::LoadToken(const int id, BiffReader * const pbr)
{
   switch(id)
   {
   case FID(PIID): pbr->GetInt((int *)pbr->m_pdata); break;
   case FID(HTBT): pbr->GetFloat(&m_d.m_heightbottom); break;
   case FID(HTTP): pbr->GetFloat(&m_d.m_heighttop); break;
   case FID(WDBT): pbr->GetFloat(&m_d.m_widthbottom); break;
   case FID(WDTP): pbr->GetFloat(&m_d.m_widthtop); break;
   case FID(MATR): pbr->GetString(m_d.m_szMaterial); break;
   case FID(TMON): pbr->GetBool(&m_d.m_tdr.m_TimerEnabled); break;
   case FID(TMIN): pbr->GetInt(&m_d.m_tdr.m_TimerInterval); break;
   case FID(TYPE): pbr->GetInt(&m_d.m_type); break;
   case FID(IMAG): pbr->GetString(m_d.m_szImage); break;
   case FID(ALGN): pbr->GetInt(&m_d.m_imagealignment); break;
   case FID(IMGW): pbr->GetBool(&m_d.m_imageWalls); break;
   case FID(NAME): pbr->GetWideString(m_wzName); break;
   case FID(WLHL): pbr->GetFloat(&m_d.m_leftwallheight); break;
   case FID(WLHR): pbr->GetFloat(&m_d.m_rightwallheight); break;
   case FID(WVHL): pbr->GetFloat(&m_d.m_leftwallheightvisible); break;
   case FID(WVHR): pbr->GetFloat(&m_d.m_rightwallheightvisible); break;
   case FID(HTEV): pbr->GetBool(&m_d.m_hitEvent); break;
   case FID(THRS): pbr->GetFloat(&m_d.m_threshold); break;
   case FID(ELAS): pbr->GetFloat(&m_d.m_elasticity); break;
   case FID(RFCT): pbr->GetFloat(&m_d.m_friction); break;
   case FID(RSCT): pbr->GetFloat(&m_d.m_scatter); break;
   case FID(CLDR): pbr->GetBool(&m_d.m_collidable); break;
   case FID(RVIS): pbr->GetBool(&m_d.m_visible); break;
   case FID(REEN): pbr->GetBool(&m_d.m_reflectionEnabled); break;
   case FID(RADB): pbr->GetFloat(&m_d.m_depthBias); break;
   case FID(RADI): pbr->GetFloat(&m_d.m_wireDiameter); break;
   case FID(RADX): pbr->GetFloat(&m_d.m_wireDistanceX); break;
   case FID(RADY): pbr->GetFloat(&m_d.m_wireDistanceY); break;
   case FID(MAPH): pbr->GetString(m_d.m_szPhysicsMaterial); break;
   case FID(OVPH): pbr->GetBool(&m_d.m_overwritePhysics); break;
   default:
   {
      LoadPointToken(id, pbr, pbr->m_version);
      ISelect::LoadToken(id, pbr);
      break;
   }
   }
   return true;
}

HRESULT Ramp::InitPostLoad()
{
   return S_OK;
}

void Ramp::AddPoint(int x, int y, const bool smooth)
{
   STARTUNDO
   const Vertex2D v = m_ptable->TransformPoint(x, y);

   std::vector<RenderVertex3D> vvertex;
   GetCentralCurve(vvertex);

   int iSeg;
   Vertex2D vOut;
   ClosestPointOnPolygon(vvertex, v, vOut, iSeg, false);

   // Go through vertices (including iSeg itself) counting control points until iSeg
   int icp = 0;
   for (int i = 0; i < (iSeg + 1); i++)
      if (vvertex[i].controlPoint)
         icp++;

   //if (icp == 0) // need to add point after the last point
   //icp = m_vdpoint.size();

   CComObject<DragPoint> *pdp;
   CComObject<DragPoint>::CreateInstance(&pdp);
   if (pdp)
   {
      pdp->AddRef();
      pdp->Init(this, vOut.x, vOut.y, (vvertex[max(iSeg - 1, 0)].z + vvertex[min(iSeg + 1, (int)vvertex.size() - 1)].z)*0.5f, smooth); // Ramps are usually always smooth
      m_vdpoint.insert(m_vdpoint.begin() + icp, pdp); // push the second point forward, and replace it with this one.  Should work when index2 wraps.
   }

   STOPUNDO
}

void Ramp::DoCommand(int icmd, int x, int y)
{
   ISelect::DoCommand(icmd, x, y);

   switch (icmd)
   {
   case ID_WALLMENU_FLIP:
      FlipPointY(GetPointCenter());
      break;

   case ID_WALLMENU_MIRROR:
      FlipPointX(GetPointCenter());
      break;

   case ID_WALLMENU_ROTATE:
      RotateDialog();
      break;

   case ID_WALLMENU_SCALE:
      ScaleDialog();
      break;

   case ID_WALLMENU_TRANSLATE:
      TranslateDialog();
      break;

   case ID_WALLMENU_ADDPOINT:
   {
      AddPoint(x, y, true);
   }
   break;
   }
}

void Ramp::FlipY(const Vertex2D& pvCenter)
{
   IHaveDragPoints::FlipPointY(pvCenter);
}

void Ramp::FlipX(const Vertex2D& pvCenter)
{
   IHaveDragPoints::FlipPointX(pvCenter);
}

void Ramp::Rotate(const float ang, const Vertex2D& pvCenter, const bool useElementCenter)
{
   IHaveDragPoints::RotatePoints(ang, pvCenter, useElementCenter);
}

void Ramp::Scale(const float scalex, const float scaley, const Vertex2D& pvCenter, const bool useElementCenter)
{
   IHaveDragPoints::ScalePoints(scalex, scaley, pvCenter, useElementCenter);
}

void Ramp::Translate(const Vertex2D &pvOffset)
{
   IHaveDragPoints::TranslatePoints(pvOffset);
}

STDMETHODIMP Ramp::InterfaceSupportsErrorInfo(REFIID riid)
{
   static const IID* arr[] =
   {
      &IID_IRamp
   };

   for (size_t i = 0; i < sizeof(arr) / sizeof(arr[0]); i++)
      if (InlineIsEqualGUID(*arr[i], riid))
         return S_OK;

   return S_FALSE;
}

STDMETHODIMP Ramp::get_HeightBottom(float *pVal)
{
   *pVal = m_d.m_heightbottom;

   return S_OK;
}

STDMETHODIMP Ramp::put_HeightBottom(float newVal)
{
   if (m_d.m_heightbottom != newVal)
   {
      m_d.m_heightbottom = newVal;
      m_dynamicVertexBufferRegenerate = true;

   }

   return S_OK;
}

STDMETHODIMP Ramp::get_HeightTop(float *pVal)
{
   *pVal = m_d.m_heighttop;

   return S_OK;
}

STDMETHODIMP Ramp::put_HeightTop(float newVal)
{
   if (m_d.m_heighttop != newVal)
   {
      m_d.m_heighttop = newVal;
      m_dynamicVertexBufferRegenerate = true;

   }

   return S_OK;
}

STDMETHODIMP Ramp::get_WidthBottom(float *pVal)
{
   *pVal = m_d.m_widthbottom;

   return S_OK;
}

STDMETHODIMP Ramp::put_WidthBottom(float newVal)
{
   if (m_d.m_widthbottom != newVal)
   {
      m_d.m_widthbottom = newVal;
      m_dynamicVertexBufferRegenerate = true;
   }

   return S_OK;
}

STDMETHODIMP Ramp::get_WidthTop(float *pVal)
{
   *pVal = m_d.m_widthtop;
   return S_OK;
}

STDMETHODIMP Ramp::put_WidthTop(float newVal)
{
   if (m_d.m_widthtop != newVal)
   {
      m_d.m_widthtop = newVal;
      m_dynamicVertexBufferRegenerate = true;
   }

   return S_OK;
}

STDMETHODIMP Ramp::get_Material(BSTR *pVal)
{
   WCHAR wz[MAXNAMEBUFFER];
   MultiByteToWideCharNull(CP_ACP, 0, m_d.m_szMaterial.c_str(), -1, wz, MAXNAMEBUFFER);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Ramp::put_Material(BSTR newVal)
{
   char buf[MAXNAMEBUFFER];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, buf, MAXNAMEBUFFER, NULL, NULL);
   m_d.m_szMaterial = buf;

   return S_OK;
}

STDMETHODIMP Ramp::get_Type(RampType *pVal)
{
   *pVal = m_d.m_type;

   return S_OK;
}

STDMETHODIMP Ramp::put_Type(RampType newVal)
{
   if(m_d.m_type != newVal)
   {
      m_d.m_type = newVal;
      m_dynamicVertexBufferRegenerate = true;
   }

   return S_OK;
}

STDMETHODIMP Ramp::get_Image(BSTR *pVal)
{
   WCHAR wz[MAXTOKEN];
   MultiByteToWideCharNull(CP_ACP, 0, m_d.m_szImage.c_str(), -1, wz, MAXTOKEN);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Ramp::put_Image(BSTR newVal)
{
   char szImage[MAXTOKEN];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, szImage, MAXTOKEN, NULL, NULL);
   const Texture * const tex = m_ptable->GetImage(szImage);
   if (tex && tex->IsHDR())
   {
       ShowError("Cannot use a HDR image (.exr/.hdr) here");
       return E_FAIL;
   }

   if(_stricmp(szImage, m_d.m_szImage.c_str()) != 0)
   {
      m_d.m_szImage = szImage;
      m_dynamicVertexBufferRegenerate = true;
   }

   return S_OK;
}

STDMETHODIMP Ramp::get_ImageAlignment(RampImageAlignment *pVal)
{
   *pVal = m_d.m_imagealignment;

   return S_OK;
}

STDMETHODIMP Ramp::put_ImageAlignment(RampImageAlignment newVal)
{
   if (m_d.m_imagealignment != newVal)
   {
      m_d.m_imagealignment = newVal;
      m_dynamicVertexBufferRegenerate = true;
   }

   return S_OK;
}

STDMETHODIMP Ramp::get_HasWallImage(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_imageWalls);

   return S_OK;
}

STDMETHODIMP Ramp::put_HasWallImage(VARIANT_BOOL newVal)
{
   if (m_d.m_imageWalls != VBTOb(newVal))
   {
      m_d.m_imageWalls = VBTOb(newVal);
      m_dynamicVertexBufferRegenerate = true;
   }

   return S_OK;
}

STDMETHODIMP Ramp::get_LeftWallHeight(float *pVal)
{
   *pVal = m_d.m_leftwallheight;

   return S_OK;
}

STDMETHODIMP Ramp::put_LeftWallHeight(float newVal)
{
   const float nv = newVal < 0.f ? 0.f : newVal;

   if (m_d.m_leftwallheight != nv)
   {
      m_d.m_leftwallheight = nv;
      m_dynamicVertexBufferRegenerate = true;
   }

   return S_OK;
}

STDMETHODIMP Ramp::get_RightWallHeight(float *pVal)
{
   *pVal = m_d.m_rightwallheight;

   return S_OK;
}

STDMETHODIMP Ramp::put_RightWallHeight(float newVal)
{
   const float nv = newVal < 0.f ? 0.f : newVal;

   if (m_d.m_rightwallheight != nv)
   {
      m_d.m_rightwallheight = nv;
      m_dynamicVertexBufferRegenerate = true;
   }

   return S_OK;
}

STDMETHODIMP Ramp::get_VisibleLeftWallHeight(float *pVal)
{
   *pVal = m_d.m_leftwallheightvisible;

   return S_OK;
}

STDMETHODIMP Ramp::put_VisibleLeftWallHeight(float newVal)
{
   const float nv = newVal < 0.f ? 0.f : newVal;

   if (m_d.m_leftwallheightvisible != nv)
   {
      m_d.m_leftwallheightvisible = nv;
      m_dynamicVertexBufferRegenerate = true;
   }

   return S_OK;
}

STDMETHODIMP Ramp::get_VisibleRightWallHeight(float *pVal)
{
   *pVal = m_d.m_rightwallheightvisible;

   return S_OK;
}

STDMETHODIMP Ramp::put_VisibleRightWallHeight(float newVal)
{
   const float nv = newVal < 0.f ? 0.f : newVal;

   if (m_d.m_rightwallheightvisible != nv)
   {
      m_d.m_rightwallheightvisible = nv;
      m_dynamicVertexBufferRegenerate = true;
   }

   return S_OK;
}

STDMETHODIMP Ramp::get_Elasticity(float *pVal)
{
   *pVal = m_d.m_elasticity;

   return S_OK;
}

STDMETHODIMP Ramp::put_Elasticity(float newVal)
{
   m_d.m_elasticity = newVal;

   return S_OK;
}

STDMETHODIMP Ramp::get_Friction(float *pVal)
{
   *pVal = m_d.m_friction;

   return S_OK;
}

STDMETHODIMP Ramp::put_Friction(float newVal)
{
   newVal = clamp(newVal, 0.f, 1.f);
   m_d.m_friction = newVal;

   return S_OK;
}

STDMETHODIMP Ramp::get_Scatter(float *pVal)
{
   *pVal = m_d.m_scatter;

   return S_OK;
}

STDMETHODIMP Ramp::put_Scatter(float newVal)
{
   m_d.m_scatter = newVal;

   return S_OK;
}

STDMETHODIMP Ramp::get_Collidable(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB((!g_pplayer) ? m_d.m_collidable : m_vhoCollidable[0]->m_enabled);

   return S_OK;
}

STDMETHODIMP Ramp::put_Collidable(VARIANT_BOOL newVal)
{
   const bool val = VBTOb(newVal);
   if (!g_pplayer)
      m_d.m_collidable = val;
   else
   {
       if (m_vhoCollidable.size() > 0 && m_vhoCollidable[0]->m_enabled != val)
           for (size_t i = 0; i < m_vhoCollidable.size(); i++) //!! costly
               m_vhoCollidable[i]->m_enabled = val; //copy to hit checking on entities composing the object
   }

   return S_OK;
}

STDMETHODIMP Ramp::get_HasHitEvent(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_hitEvent);

   return S_OK;
}

STDMETHODIMP Ramp::put_HasHitEvent(VARIANT_BOOL newVal)
{
   m_d.m_hitEvent = VBTOb(newVal);

   return S_OK;
}

STDMETHODIMP Ramp::get_Threshold(float *pVal)
{
   *pVal = m_d.m_threshold;

   return S_OK;
}

STDMETHODIMP Ramp::put_Threshold(float newVal)
{
   m_d.m_threshold = newVal;

   return S_OK;
}


STDMETHODIMP Ramp::get_Visible(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_visible);

   return S_OK;
}

STDMETHODIMP Ramp::put_Visible(VARIANT_BOOL newVal)
{
   m_d.m_visible = VBTOb(newVal);

   return S_OK;
}

STDMETHODIMP Ramp::get_ReflectionEnabled(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_reflectionEnabled);

   return S_OK;
}

STDMETHODIMP Ramp::put_ReflectionEnabled(VARIANT_BOOL newVal)
{
   m_d.m_reflectionEnabled = VBTOb(newVal);

   return S_OK;
}

STDMETHODIMP Ramp::get_DepthBias(float *pVal)
{
   *pVal = m_d.m_depthBias;

   return S_OK;
}

STDMETHODIMP Ramp::put_DepthBias(float newVal)
{
   if (m_d.m_depthBias != newVal)
   {
      m_d.m_depthBias = newVal;
      m_dynamicVertexBufferRegenerate = true;
   }

   return S_OK;
}

STDMETHODIMP Ramp::get_WireDiameter(float *pVal)
{
   *pVal = m_d.m_wireDiameter;

   return S_OK;
}

STDMETHODIMP Ramp::put_WireDiameter(float newVal)
{
   m_d.m_wireDiameter = newVal;

   return S_OK;
}

STDMETHODIMP Ramp::get_WireDistanceX(float *pVal)
{
   *pVal = m_d.m_wireDistanceX;

   return S_OK;
}

STDMETHODIMP Ramp::put_WireDistanceX(float newVal)
{
   m_d.m_wireDistanceX = newVal;

   return S_OK;
}

STDMETHODIMP Ramp::get_WireDistanceY(float *pVal)
{
   *pVal = m_d.m_wireDistanceY;

   return S_OK;
}

STDMETHODIMP Ramp::put_WireDistanceY(float newVal)
{
   m_d.m_wireDistanceY = newVal;

   return S_OK;
}

STDMETHODIMP Ramp::get_PhysicsMaterial(BSTR *pVal)
{
    WCHAR wz[MAXNAMEBUFFER];
    MultiByteToWideCharNull(CP_ACP, 0, m_d.m_szPhysicsMaterial.c_str(), -1, wz, MAXNAMEBUFFER);
    *pVal = SysAllocString(wz);

    return S_OK;
}

STDMETHODIMP Ramp::put_PhysicsMaterial(BSTR newVal)
{
    char buf[MAXNAMEBUFFER];
    WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, buf, MAXNAMEBUFFER, NULL, NULL);
    m_d.m_szPhysicsMaterial = buf;

    return S_OK;
}

STDMETHODIMP Ramp::get_OverwritePhysics(VARIANT_BOOL *pVal)
{
    *pVal = FTOVB(m_d.m_overwritePhysics);

    return S_OK;
}

STDMETHODIMP Ramp::put_OverwritePhysics(VARIANT_BOOL newVal)
{
    m_d.m_overwritePhysics = VBTOb(newVal);

    return S_OK;
}


void Ramp::ExportMesh(FILE *f)
{
   if (m_d.m_visible)
   {
      char name[sizeof(m_wzName)/sizeof(m_wzName[0])];
      WideCharToMultiByteNull(CP_ACP, 0, m_wzName, -1, name, sizeof(name), NULL, NULL);
      WaveFrontObj_WriteObjectName(f, name);
      if (!isHabitrail())
      {
         Vertex3D_NoTex2 *rampMesh = NULL;
         GenerateRampMesh(&rampMesh);
         const Material * const mat = m_ptable->GetMaterial(m_d.m_szMaterial);
         const int listLength = (m_rampVertex - 1) * 6;
         unsigned int numVers = m_numVertices * 3;
         if (m_d.m_rightwallheightvisible == 0.0f && m_d.m_leftwallheightvisible == 0.0f)
            numVers = m_numVertices;
         WaveFrontObj_WriteVertexInfo(f, rampMesh, numVers);
         WaveFrontObj_WriteMaterial(m_d.m_szMaterial, string(), mat);
         WaveFrontObj_UseTexture(f, m_d.m_szMaterial);

         if (m_d.m_rightwallheightvisible != 0.f && m_d.m_leftwallheightvisible != 0.f)
         {
            WaveFrontObj_WriteFaceInfoList(f, m_meshIndices.data(), listLength * 3);
            WaveFrontObj_UpdateFaceOffset(numVers);
         }
         else
         {
            if (m_d.m_rightwallheightvisible != 0.0f)
            {
               WaveFrontObj_WriteFaceInfoList(f, m_meshIndices.data(), listLength * 2);
               WaveFrontObj_UpdateFaceOffset(m_numVertices * 3);
            }
            else if (m_d.m_leftwallheightvisible != 0.0f)
            {
               WaveFrontObj_WriteFaceInfoList(f, m_meshIndices.data(), listLength);
               WaveFrontObj_UpdateFaceOffset(m_numVertices * 2);
               WaveFrontObj_WriteFaceInfoList(f, m_meshIndices.data(), listLength);
               WaveFrontObj_UpdateFaceOffset(m_numVertices);
            }
            else
            {
               WaveFrontObj_WriteFaceInfoList(f, m_meshIndices.data(), listLength);
               WaveFrontObj_UpdateFaceOffset(m_numVertices);
            }
         }
         delete[] rampMesh;
      }
      else
      {
         Vertex3D_NoTex2 *tmpBuf1 = NULL, *tmpBuf2 = NULL;
         GenerateWireMesh(&tmpBuf1, &tmpBuf2);
         const Material * const mat = m_ptable->GetMaterial(m_d.m_szMaterial);
         if (m_d.m_type == RampType1Wire)
         {
            WaveFrontObj_WriteVertexInfo(f, tmpBuf1, m_numVertices);
            WaveFrontObj_WriteMaterial(m_d.m_szMaterial, string(), mat);
            WaveFrontObj_UseTexture(f, m_d.m_szMaterial);
            WaveFrontObj_WriteFaceInfo(f, m_meshIndices);
            WaveFrontObj_UpdateFaceOffset(m_numVertices);
         }
         else if (m_d.m_type == RampType2Wire)
         {
            Vertex3D_NoTex2 * const tmp = new Vertex3D_NoTex2[m_numVertices * 2];
            memcpy(tmp, tmpBuf1, sizeof(Vertex3D_NoTex2)*m_numVertices);
            memcpy(&tmp[m_numVertices], tmpBuf2, sizeof(Vertex3D_NoTex2)*m_numVertices);
            for (int i = 0; i < m_numVertices * 2; i++)
               tmp[i].z += 3.0f;
            WaveFrontObj_WriteVertexInfo(f, tmp, m_numVertices * 2);
            delete[] tmp;
            WaveFrontObj_WriteFaceInfo(f, m_meshIndices);
            WaveFrontObj_WriteMaterial(m_d.m_szMaterial, string(), mat);
            WaveFrontObj_UseTexture(f, m_d.m_szMaterial);
            WORD * const idx = new WORD[m_meshIndices.size()];
            for (size_t i = 0; i < m_meshIndices.size(); i++)
               idx[i] = m_meshIndices[i] + m_numVertices;
            WaveFrontObj_WriteFaceInfoList(f, idx, (unsigned int)m_meshIndices.size());
            WaveFrontObj_UpdateFaceOffset(m_numVertices * 2);
            delete[] idx;
         }
         else if (m_d.m_type == RampType4Wire)
         {
            Vertex3D_NoTex2 * const tmp = new Vertex3D_NoTex2[m_numVertices * 4];
            memcpy(tmp, tmpBuf1, sizeof(Vertex3D_NoTex2)*m_numVertices);
            memcpy(&tmp[m_numVertices], tmpBuf2, sizeof(Vertex3D_NoTex2)*m_numVertices);
            memcpy(&tmp[m_numVertices * 2], tmpBuf1, sizeof(Vertex3D_NoTex2)*m_numVertices);
            memcpy(&tmp[m_numVertices * 3], tmpBuf2, sizeof(Vertex3D_NoTex2)*m_numVertices);
            for (int i = 0; i < m_numVertices * 2; i++)
               tmp[i].z += m_d.m_wireDistanceY*0.5f;

            for (int i = m_numVertices * 2; i < m_numVertices * 4; i++)
               tmp[i].z += 3.0f;
            WaveFrontObj_WriteVertexInfo(f, tmp, m_numVertices * 4);
            delete[] tmp;
            WaveFrontObj_WriteMaterial(m_d.m_szMaterial, string(), mat);
            WaveFrontObj_UseTexture(f, m_d.m_szMaterial);
            WaveFrontObj_WriteFaceInfo(f, m_meshIndices);
            WORD * const idx = new WORD[m_meshIndices.size()];
            for (size_t i = 0; i < m_meshIndices.size(); i++)
               idx[i] = m_meshIndices[i] + m_numVertices;
            WaveFrontObj_WriteFaceInfoList(f, idx, (unsigned int)m_meshIndices.size());
            for (size_t i = 0; i < m_meshIndices.size(); i++)
               idx[i] = m_meshIndices[i] + m_numVertices * 2;
            WaveFrontObj_WriteFaceInfoList(f, idx, (unsigned int)m_meshIndices.size());
            for (size_t i = 0; i < m_meshIndices.size(); i++)
               idx[i] = m_meshIndices[i] + m_numVertices * 3;
            WaveFrontObj_WriteFaceInfoList(f, idx, (unsigned int)m_meshIndices.size());
            WaveFrontObj_UpdateFaceOffset(m_numVertices * 4);
            delete[] idx;
         }
         else if (m_d.m_type == RampType3WireLeft)
         {
            Vertex3D_NoTex2 * const tmp = new Vertex3D_NoTex2[m_numVertices * 3];
            memcpy(tmp, tmpBuf2, sizeof(Vertex3D_NoTex2)*m_numVertices);
            memcpy(&tmp[m_numVertices], tmpBuf1, sizeof(Vertex3D_NoTex2)*m_numVertices);
            memcpy(&tmp[m_numVertices * 2], tmpBuf2, sizeof(Vertex3D_NoTex2)*m_numVertices);
            for (int i = 0; i < m_numVertices; i++)
               tmp[i].z += m_d.m_wireDistanceY*0.5f;

            for (int i = m_numVertices; i < m_numVertices * 3; i++)
               tmp[i].z += 3.0f;
            WaveFrontObj_WriteVertexInfo(f, tmp, m_numVertices * 3);
            delete[] tmp;
            WaveFrontObj_WriteMaterial(m_d.m_szMaterial, string(), mat);
            WaveFrontObj_UseTexture(f, m_d.m_szMaterial);
            WaveFrontObj_WriteFaceInfo(f, m_meshIndices);
            WORD * const idx = new WORD[m_meshIndices.size()];
            for (size_t i = 0; i < m_meshIndices.size(); i++)
               idx[i] = m_meshIndices[i] + m_numVertices;
            WaveFrontObj_WriteFaceInfoList(f, idx, (unsigned int)m_meshIndices.size());
            for (size_t i = 0; i < m_meshIndices.size(); i++)
               idx[i] = m_meshIndices[i] + m_numVertices * 2;
            WaveFrontObj_WriteFaceInfoList(f, idx, (unsigned int)m_meshIndices.size());
            WaveFrontObj_UpdateFaceOffset(m_numVertices * 3);
            delete[] idx;
         }
         else if (m_d.m_type == RampType3WireRight)
         {
            Vertex3D_NoTex2 * const tmp = new Vertex3D_NoTex2[m_numVertices * 3];
            memcpy(tmp, tmpBuf1, sizeof(Vertex3D_NoTex2)*m_numVertices);
            memcpy(&tmp[m_numVertices], tmpBuf1, sizeof(Vertex3D_NoTex2)*m_numVertices);
            memcpy(&tmp[m_numVertices * 2], tmpBuf2, sizeof(Vertex3D_NoTex2)*m_numVertices);
            for (int i = 0; i < m_numVertices; i++)
               tmp[i].z += m_d.m_wireDistanceY*0.5f;

            for (int i = m_numVertices; i < m_numVertices * 3; i++)
               tmp[i].z += 3.0f;
            WaveFrontObj_WriteVertexInfo(f, tmp, m_numVertices * 3);
            delete[] tmp;
            WaveFrontObj_WriteMaterial(m_d.m_szMaterial, string(), mat);
            WaveFrontObj_UseTexture(f, m_d.m_szMaterial);
            WaveFrontObj_WriteFaceInfo(f, m_meshIndices);
            WORD * const idx = new WORD[m_meshIndices.size()];
            for (size_t i = 0; i < m_meshIndices.size(); i++)
               idx[i] = m_meshIndices[i] + m_numVertices;
            WaveFrontObj_WriteFaceInfoList(f, idx, (unsigned int)m_meshIndices.size());
            for (size_t i = 0; i < m_meshIndices.size(); i++)
               idx[i] = m_meshIndices[i] + m_numVertices * 2;
            WaveFrontObj_WriteFaceInfoList(f, idx, (unsigned int)m_meshIndices.size());
            WaveFrontObj_UpdateFaceOffset(m_numVertices * 3);
            delete[] idx;
         }

         delete[] tmpBuf1;
         if (m_d.m_type != RampType1Wire)
            delete[] tmpBuf2;
      }
   }
}

void Ramp::RenderRamp(const Material * const mat)
{
   if (!mat)
      return;

   if (m_d.m_widthbottom == 0.0f && m_d.m_widthtop == 0.0f)
   {
      m_dynamicVertexBufferRegenerate = false;
      return;
   }

   RenderDevice * const pd3dDevice = g_pplayer->m_pin3d.m_pd3dPrimaryDevice;

   // see the comment in RenderStatic() above
   if (m_d.m_imagealignment == ImageModeWrap)
      pd3dDevice->SetTextureAddressMode(0, RenderDevice::TEX_CLAMP);

   if (isHabitrail())
      RenderStaticHabitrail(mat);
   else
   {
      if (!m_dynamicVertexBuffer || m_dynamicVertexBufferRegenerate)
         GenerateVertexBuffer();

      pd3dDevice->basicShader->SetMaterial(mat);

      pd3dDevice->SetRenderState(RenderDevice::DEPTHBIAS, 0);
      pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, RenderDevice::RS_TRUE);
      pd3dDevice->SetRenderState(RenderDevice::CULLMODE, RenderDevice::CULL_NONE); // as both floor and walls are thinwalled

      //Pin3D * const ppin3d = &g_pplayer->m_pin3d;
      Texture * const pin = m_ptable->GetImage(m_d.m_szImage);

      if (pin)
      {
         pd3dDevice->basicShader->SetTechnique(mat->m_bIsMetal ? "basic_with_texture_isMetal" : "basic_with_texture_isNotMetal");
         pd3dDevice->basicShader->SetTexture("Texture0", pin, false);
         pd3dDevice->basicShader->SetAlphaTestValue(pin->m_alphaTestValue * (float)(1.0 / 255.0));

         //ppin3d->SetTextureFilter ( 0, TEXTURE_MODE_TRILINEAR );
      }
      else
         pd3dDevice->basicShader->SetTechnique(mat->m_bIsMetal ? "basic_without_texture_isMetal" : "basic_without_texture_isNotMetal");

      //ppin3d->EnableAlphaBlend( false ); //!! not necessary anymore

      if (m_d.m_rightwallheightvisible != 0.f && m_d.m_leftwallheightvisible != 0.f && (!pin || m_d.m_imageWalls))
      {
         // both walls with image and floor
         pd3dDevice->basicShader->Begin(0);
         pd3dDevice->DrawIndexedPrimitiveVB(RenderDevice::TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, m_dynamicVertexBuffer, 0, m_numVertices * 3, m_dynamicIndexBuffer, 0, (m_rampVertex - 1) * 6 * 3);
         pd3dDevice->basicShader->End();
      }
      else
      {
         // only floor
         pd3dDevice->basicShader->Begin(0);
         pd3dDevice->DrawIndexedPrimitiveVB(RenderDevice::TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, m_dynamicVertexBuffer, 0, m_numVertices, m_dynamicIndexBuffer, 0, (m_rampVertex - 1) * 6);

         if (m_d.m_rightwallheightvisible != 0.f || m_d.m_leftwallheightvisible != 0.f)
         {
            if (pin && !m_d.m_imageWalls)
            {
               pd3dDevice->basicShader->End();
               pd3dDevice->basicShader->SetTechnique(mat->m_bIsMetal ? "basic_without_texture_isMetal" : "basic_without_texture_isNotMetal");
               pd3dDevice->basicShader->Begin(0);
            }

            if (m_d.m_rightwallheightvisible != 0.f && m_d.m_leftwallheightvisible != 0.f) //only render left & right side if the height is >0
               pd3dDevice->DrawIndexedPrimitiveVB(RenderDevice::TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, m_dynamicVertexBuffer, m_numVertices, m_numVertices * 2, m_dynamicIndexBuffer, 0, (m_rampVertex - 1) * 6 * 2);
            else
            {
               if (m_d.m_rightwallheightvisible != 0.f) //only render right side if the height is >0
                  pd3dDevice->DrawIndexedPrimitiveVB(RenderDevice::TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, m_dynamicVertexBuffer, m_numVertices, m_numVertices, m_dynamicIndexBuffer, 0, (m_rampVertex - 1) * 6);

               if (m_d.m_leftwallheightvisible != 0.f) //only render left side if the height is >0
                  pd3dDevice->DrawIndexedPrimitiveVB(RenderDevice::TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, m_dynamicVertexBuffer, m_numVertices * 2, m_numVertices, m_dynamicIndexBuffer, 0, (m_rampVertex - 1) * 6);
            }
         }

         pd3dDevice->basicShader->End();
      }

      //g_pplayer->m_pin3d.DisableAlphaBlend(); //!! not necessary anymore
   }
}

// Always called each frame to render over everything else (along with primitives)
// Same code as RenderStatic (with the exception of the alpha tests).
// Also has less drawing calls by bundling seperate calls.
void Ramp::RenderDynamic()
{
   TRACE_FUNCTION();

   const Material * const mat = m_ptable->GetMaterial(m_d.m_szMaterial);
   // don't render if invisible or not a transparent ramp
   if (!m_d.m_visible || (!mat->m_bOpacityActive))
      return;
   if (m_ptable->m_reflectionEnabled && !m_d.m_reflectionEnabled)
      return;

   RenderRamp(mat);
}

//
// license:GPLv3+
// Ported at: VisualPinball.Engine/VPT/Ramp/RampMeshGenerator.cs
//

void Ramp::GenerateRampMesh(Vertex3D_NoTex2 **meshBuf)
{
   const Texture * const pin = m_ptable->GetImage(m_d.m_szImage);
   float *rgheight;
   float *rgratio = NULL;
   const Vertex2D * const rgvLocal = GetRampVertex(m_rampVertex, &rgheight, NULL, (m_d.m_imagealignment == ImageModeWorld) ? NULL : &rgratio, NULL, -1, true);

   const float inv_tablewidth = 1.0f / (m_ptable->m_right - m_ptable->m_left);
   const float inv_tableheight = 1.0f / (m_ptable->m_bottom - m_ptable->m_top);

   m_numVertices = m_rampVertex * 2;
   const unsigned int rgioffset = (m_rampVertex - 1) * 6;
   m_numIndices = rgioffset * 3; // to draw the full ramp in one go (could only use *1, and draw three times with offsets into vertices)

   if (*meshBuf == NULL)
   {
      *meshBuf = new Vertex3D_NoTex2[m_numVertices * 3];
   }
   Vertex3D_NoTex2 * const buf = *meshBuf;

   m_vertBuffer = new Vertex3D_NoTex2[m_numVertices];
   m_meshIndices.resize(m_numIndices);
   for (int i = 0; i < m_rampVertex; i++)
   {
      Vertex3D_NoTex2 * const rgv3D = m_vertBuffer + i * 2;
      rgv3D[0].x = rgvLocal[i].x;
      rgv3D[0].y = rgvLocal[i].y;
      rgv3D[0].z = rgheight[i] * m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];

      rgv3D[1].x = rgvLocal[m_rampVertex * 2 - i - 1].x;
      rgv3D[1].y = rgvLocal[m_rampVertex * 2 - i - 1].y;
      rgv3D[1].z = rgv3D[0].z;

      if (pin)
      {
         if (m_d.m_imagealignment == ImageModeWorld)
         {
            rgv3D[0].tu = rgv3D[0].x * inv_tablewidth;
            rgv3D[0].tv = rgv3D[0].y * inv_tableheight;
            rgv3D[1].tu = rgv3D[1].x * inv_tablewidth;
            rgv3D[1].tv = rgv3D[1].y * inv_tableheight;
         }
         else
         {
            rgv3D[0].tu = 1.0f;
            rgv3D[0].tv = rgratio[i];
            rgv3D[1].tu = 0;
            rgv3D[1].tv = rgratio[i];
         }
      }
      else
      {
         rgv3D[0].tu = 0.0f;
         rgv3D[0].tv = 0.0f;
         rgv3D[1].tu = 0.0f;
         rgv3D[1].tv = 0.0f;
      }

      if (i == m_rampVertex - 1)
         break;

      //floor
      unsigned int offs = i * 6;
      m_meshIndices[offs] = i * 2;
      m_meshIndices[offs + 1] = i * 2 + 1;
      m_meshIndices[offs + 2] = i * 2 + 3;
      m_meshIndices[offs + 3] = i * 2;
      m_meshIndices[offs + 4] = i * 2 + 3;
      m_meshIndices[offs + 5] = i * 2 + 2;

      //walls
      offs += rgioffset;
      m_meshIndices[offs] = i * 2 + m_numVertices;
      m_meshIndices[offs + 1] = i * 2 + m_numVertices + 1;
      m_meshIndices[offs + 2] = i * 2 + m_numVertices + 3;
      m_meshIndices[offs + 3] = i * 2 + m_numVertices;
      m_meshIndices[offs + 4] = i * 2 + m_numVertices + 3;
      m_meshIndices[offs + 5] = i * 2 + m_numVertices + 2;

      offs += rgioffset;
      m_meshIndices[offs] = i * 2 + m_numVertices * 2;
      m_meshIndices[offs + 1] = i * 2 + m_numVertices * 2 + 1;
      m_meshIndices[offs + 2] = i * 2 + m_numVertices * 2 + 3;
      m_meshIndices[offs + 3] = i * 2 + m_numVertices * 2;
      m_meshIndices[offs + 4] = i * 2 + m_numVertices * 2 + 3;
      m_meshIndices[offs + 5] = i * 2 + m_numVertices * 2 + 2;
   }
   ComputeNormals(m_vertBuffer, m_numVertices, m_meshIndices.data(), (m_rampVertex - 1) * 6);

   // Flip Normals if pointing downwards instead of upwards, not necessary anymore //!! hacky, do it correct somehow else
   /*for (int i = 0; i < rampVertex; i++)
       for (int j = 0; j < 2; ++j) if (m_vertBuffer[i * 2 + j].nz < 0.0f) {
       m_vertBuffer[i * 2 + j].nx = -m_vertBuffer[i * 2 + j].nx;
       m_vertBuffer[i * 2 + j].ny = -m_vertBuffer[i * 2 + j].ny;
       m_vertBuffer[i * 2 + j].nz = -m_vertBuffer[i * 2 + j].nz;
       }*/

   unsigned int offset = 0;
   memcpy(&buf[offset], m_vertBuffer, sizeof(Vertex3D_NoTex2)*m_numVertices);
   offset += m_numVertices;

   // only calculate vertices if one or both sides are visible (!=0)
   if (m_d.m_leftwallheightvisible != 0.f || m_d.m_rightwallheightvisible != 0.f)
   {
      for (int i = 0; i < m_rampVertex; i++)
      {
         Vertex3D_NoTex2 * const rgv3D = m_vertBuffer + i * 2;

         rgv3D[1].x = rgvLocal[i].x;
         rgv3D[1].y = rgvLocal[i].y;
         rgv3D[1].z = (rgheight[i] + m_d.m_rightwallheightvisible)*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];

         if (pin && m_d.m_imageWalls)
         {
            if (m_d.m_imagealignment == ImageModeWorld)
            {
               rgv3D[0].tu = rgv3D[0].x * inv_tablewidth;
               rgv3D[0].tv = rgv3D[0].y * inv_tableheight;
            }
            else
            {
               rgv3D[0].tu = 0;
               rgv3D[0].tv = rgratio[i];
            }

            rgv3D[1].tu = rgv3D[0].tu;
            rgv3D[1].tv = rgv3D[0].tv;
         }
         else
         {
            rgv3D[0].tu = 0.0f;
            rgv3D[0].tv = 0.0f;
            rgv3D[1].tu = 0.0f;
            rgv3D[1].tv = 0.0f;
         }
      }
      ComputeNormals(m_vertBuffer, m_numVertices, m_meshIndices.data(), (m_rampVertex - 1) * 6);
      memcpy(&buf[offset], m_vertBuffer, sizeof(Vertex3D_NoTex2)*m_numVertices);
      offset += m_numVertices;

      for (int i = 0; i < m_rampVertex; i++)
      {
         Vertex3D_NoTex2 * const rgv3D = m_vertBuffer + i * 2;
         rgv3D[0].x = rgvLocal[m_rampVertex * 2 - i - 1].x;
         rgv3D[0].y = rgvLocal[m_rampVertex * 2 - i - 1].y;
         rgv3D[0].z = rgheight[i] * m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];

         rgv3D[1].x = rgv3D[0].x;
         rgv3D[1].y = rgv3D[0].y;
         rgv3D[1].z = (rgheight[i] + m_d.m_leftwallheightvisible)*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];

         if (pin && m_d.m_imageWalls)
         {
            if (m_d.m_imagealignment == ImageModeWorld)
            {
               rgv3D[0].tu = rgv3D[0].x * inv_tablewidth;
               rgv3D[0].tv = rgv3D[0].y * inv_tableheight;
            }
            else
            {
               rgv3D[0].tu = 0;
               rgv3D[0].tv = rgratio[i];
            }

            rgv3D[1].tu = rgv3D[0].tu;
            rgv3D[1].tv = rgv3D[0].tv;
         }
         else
         {
            rgv3D[0].tu = 0.0f;
            rgv3D[0].tv = 0.0f;
            rgv3D[1].tu = 0.0f;
            rgv3D[1].tv = 0.0f;
         }
      }
      ComputeNormals(m_vertBuffer, m_numVertices, m_meshIndices.data(), (m_rampVertex - 1) * 6);
      memcpy(&buf[offset], m_vertBuffer, sizeof(Vertex3D_NoTex2)*m_numVertices);
   }

   delete[] m_vertBuffer;
   delete[] rgvLocal;
   delete[] rgheight;
   if (rgratio)
      delete[] rgratio;
}

//
// end of license:GPLv3+, back to 'old MAME'-like
//

void Ramp::GenerateVertexBuffer()
{
   m_dynamicVertexBufferRegenerate = false;

   Vertex3D_NoTex2 *tmpBuffer = NULL;
   GenerateRampMesh(&tmpBuffer);

   if (m_dynamicVertexBuffer)
      m_dynamicVertexBuffer->release();

   RenderDevice * const pd3dDevice = g_pplayer->m_pin3d.m_pd3dPrimaryDevice;

   pd3dDevice->CreateVertexBuffer(m_numVertices * 3, 0, MY_D3DFVF_NOTEX2_VERTEX, &m_dynamicVertexBuffer); //!! use USAGE_DYNAMIC if it would actually be "really" dynamic

   Vertex3D_NoTex2 *buf;
   m_dynamicVertexBuffer->lock(0, 0, (void**)&buf, VertexBuffer::WRITEONLY);
   memcpy(buf, tmpBuffer, sizeof(Vertex3D_NoTex2)*m_numVertices * 3);
   m_dynamicVertexBuffer->unlock();

   // not necessary to reorder //!! also potentially unsafe, as walls can be disabled, so order is important!
   /*WORD* const tmp = reorderForsyth(m_meshIndices, m_numVertices * 3);
   if (tmp != NULL)
   {
   memcpy(m_meshIndices.data(), tmp, m_meshIndices.size()*sizeof(WORD));
   delete[] tmp;
   }*/

   if (m_dynamicIndexBuffer)
      m_dynamicIndexBuffer->release();

   m_dynamicIndexBuffer = pd3dDevice->CreateAndFillIndexBuffer(m_meshIndices);
   delete[] tmpBuffer;
}

void Ramp::SetDefaultPhysics(bool fromMouseClick)
{
   static const char strKeyName[] = "DefaultProps\\Ramp";
   m_d.m_elasticity = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "Elasticity", 0.3f) : 0.3f;
   m_d.m_friction = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "Friction", 0.3f) : 0.3f;
   m_d.m_scatter = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "Scatter", 0) : 0;
}
