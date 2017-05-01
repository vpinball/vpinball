#include "StdAfx.h"
//#include "forsyth.h"
#include "objloader.h"

const float Guide::HIT_SHAPE_DETAIL_LEVEL = 7.0f;

Guide::Guide()
{
   m_menuid = IDR_SURFACEMENU;
   m_d.m_fCollidable = true;
   m_d.m_fVisible = true;
   dynamicVertexBuffer = 0;
   dynamicIndexBuffer = 0;
   dynamicVertexBufferRegenerate = true;
   m_d.m_depthBias = 0.0f;
   m_d.m_wireDiameter = 6.0f;
   m_propPosition = NULL;
   m_propPhysics = NULL;
   memset( m_d.m_szMaterial, 0, 32 );
   memset( m_d.m_szPhysicsMaterial, 0, 32 );
   m_d.m_fHitEvent = false;
   m_d.m_fOverwritePhysics=true;
   rgheightInit = NULL;
}

Guide::~Guide()
{
   if (dynamicVertexBuffer)
      dynamicVertexBuffer->release();

   if (dynamicIndexBuffer)
      dynamicIndexBuffer->release();

   if(rgheightInit)
       delete [] rgheightInit;
}


bool Guide::IsTransparent()
{
   return m_ptable->GetMaterial(m_d.m_szMaterial)->m_bOpacityActive;
}

HRESULT Guide::Init(PinTable *ptable, float x, float y, bool fromMouseClick)
{
   m_ptable = ptable;
   SetDefaults(fromMouseClick);
   m_d.m_fVisible = true;

   float length = 0.5f * GetRegStringAsFloatWithDefault("DefaultProps\\Ramp", "Length", 400.0f);

   CComObject<DragPoint> *pdp;
   CComObject<DragPoint>::CreateInstance(&pdp);
   if (pdp)
   {
      pdp->AddRef();
      pdp->Init(this, x, y + length);
      pdp->m_calcHeight = m_d.m_heightbottom;
      pdp->m_fSmooth = true;
      m_vdpoint.AddElement(pdp);
   }

   CComObject<DragPoint>::CreateInstance(&pdp);
   if (pdp)
   {
      pdp->AddRef();
      pdp->Init(this, x, y - length);
      pdp->m_calcHeight = m_d.m_heighttop;
      pdp->m_fSmooth = true;
      m_vdpoint.AddElement(pdp);
   }


   InitVBA(fTrue, 0, NULL);

   return S_OK;
}

void Guide::SetDefaults(bool fromMouseClick)
{
   static const char strKeyName[] = "DefaultProps\\Ramp";
   
   m_d.m_heightbottom = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "HeightBottom", 25.0f) : 25.0f;
   m_d.m_heighttop = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "HeightTop", 25.0f) : 25.0f;

   m_d.m_tdr.m_fTimerEnabled = fromMouseClick ? GetRegBoolWithDefault(strKeyName, "TimerEnabled", false) : false;
   m_d.m_tdr.m_TimerInterval = fromMouseClick ? GetRegIntWithDefault(strKeyName, "TimerInterval", 100) : 100;

   m_d.m_threshold = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "HitThreshold", 2.0f) : 2.0f;

   SetDefaultPhysics(fromMouseClick);

   m_d.m_fVisible = fromMouseClick ? GetRegBoolWithDefault(strKeyName, "Visible", true) : true;
   m_d.m_fCollidable = fromMouseClick ? GetRegBoolWithDefault(strKeyName, "Collidable", true) : true;
   m_d.m_fReflectionEnabled = fromMouseClick ? GetRegBoolWithDefault(strKeyName, "ReflectionEnabled", true) : true;

   m_d.m_wireDiameter = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "WireDiameter", 8.0f) : 8.0f;
}

void Guide::WriteRegDefaults()
{
   static const char strKeyName[] = "DefaultProps\\Guide";

   SetRegValueFloat(strKeyName, "HeightBottom", m_d.m_heightbottom);
   SetRegValueFloat(strKeyName, "HeightTop", m_d.m_heighttop);
   SetRegValueBool(strKeyName, "TimerEnabled", m_d.m_tdr.m_fTimerEnabled);
   SetRegValue(strKeyName, "TimerInterval", REG_DWORD, &m_d.m_tdr.m_TimerInterval, 4);
   SetRegValueBool(strKeyName, "HitEvent", m_d.m_fHitEvent);
   SetRegValueFloat(strKeyName, "HitThreshold", m_d.m_threshold);
   SetRegValueFloat(strKeyName, "Elasticity", m_d.m_elasticity);
   SetRegValueFloat(strKeyName, "Friction", m_d.m_friction);
   SetRegValueFloat(strKeyName, "Scatter", m_d.m_scatter);
   SetRegValueBool(strKeyName, "Collidable", m_d.m_fCollidable);
   SetRegValueBool(strKeyName, "Visible", m_d.m_fVisible);
   SetRegValueBool(strKeyName, "ReflectionEnabled", m_d.m_fReflectionEnabled);
   SetRegValueFloat(strKeyName, "WireDiameter", m_d.m_wireDiameter);
}

void Guide::GetPointDialogPanes(Vector<PropertyPane> *pvproppane)
{
   PropertyPane *pproppane;

   pproppane = new PropertyPane(IDD_PROPPOINT_VISUALS, IDS_VISUALS);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPPOINT_POSITION_WITHZ, IDS_POSITION);
   pvproppane->AddElement(pproppane);
}


void Guide::PreRender(Sur * const psur)
{
   //make 1 wire ramps look unique in editor - uses ramp color
   psur->SetFillColor(m_ptable->RenderSolid() ? g_pvp->m_fillColor : -1);
   psur->SetBorderColor(-1, false, 0);
   psur->SetObject(this);

   int cvertex;
   const Vertex2D * const rgvLocal = GetGuideVertex(cvertex, NULL, NULL, NULL, NULL, HIT_SHAPE_DETAIL_LEVEL, false, false);
   psur->Polygon(rgvLocal, cvertex * 2);

   delete[] rgvLocal;
}

void Guide::Render(Sur * const psur)
{
   psur->SetFillColor(-1);
   psur->SetBorderColor(RGB(0, 0, 0), false, 0);
   psur->SetLineColor(RGB(0, 0, 0), false, 0);
   psur->SetObject(this);
   psur->SetObject(NULL); // NULL so this won't be hit-tested

   bool *pfCross;
   Vertex2D *middlePoints;
   int cvertex;
   const Vertex2D * const rgvLocal = GetGuideVertex(cvertex, NULL, &pfCross, NULL, &middlePoints, HIT_SHAPE_DETAIL_LEVEL, false, false);
   psur->Polygon(rgvLocal, cvertex * 2);

   if (isHabitrail())
   {
      psur->Polyline(middlePoints, cvertex);
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

   bool fDrawDragpoints = ((m_selectstate != eNotSelected) || g_pvp->m_fAlwaysDrawDragPoints);
   // if the item is selected then draw the dragpoints (or if we are always to draw dragpoints)
   if (!fDrawDragpoints)
   {
      // if any of the dragpoints of this object are selected then draw all the dragpoints
      for (int i = 0; i < m_vdpoint.Size(); i++)
      {
         const CComObject<DragPoint> * const pdp = m_vdpoint.ElementAt(i);
         if (pdp->m_selectstate != eNotSelected)
         {
            fDrawDragpoints = true;
            break;
         }
      }
   }

   if (fDrawDragpoints)
   {
      const int len = m_vdpoint.Size();
      for (int i = 0; i < len; i++)
      {
         CComObject<DragPoint> * const pdp = m_vdpoint.ElementAt(i);
         psur->SetFillColor(-1);
         if (i == 0)
            psur->SetBorderColor(RGB(0, 0, 255), false, 0);
         else
            psur->SetBorderColor(RGB(255, 0, 0), false, 0);

         psur->SetObject(pdp);

         if (pdp->m_fDragging)
            psur->SetBorderColor(RGB(0, 255, 0), false, 0);

         psur->Ellipse2(pdp->m_v.x, pdp->m_v.y, 8);
      }
   }
}

void Guide::RenderOutline(Sur * const psur, const bool solid)
{
   if ( solid )
      psur->SetFillColor(BLUEPRINT_SOLID_COLOR);
   else
      psur->SetFillColor(-1);

   psur->SetBorderColor(RGB(0, 0, 0), false, 0);
   psur->SetLineColor(RGB(0, 0, 0), false, 0);
   psur->SetObject(this);
   psur->SetObject(NULL); // NULL so this won't be hit-tested

   bool *pfCross;
   Vertex2D *middlePoints;
   int cvertex;
   const Vertex2D * const rgvLocal = GetGuideVertex(cvertex, NULL, &pfCross, NULL, &middlePoints, HIT_SHAPE_DETAIL_LEVEL, false, false);
   psur->Polygon(rgvLocal, cvertex * 2);

   if (isHabitrail())
   {
      psur->Polyline(middlePoints, cvertex - 1);
   }

   for (int i = 0; i < cvertex; i++)
      if (pfCross[i])
         psur->Line(rgvLocal[i].x, rgvLocal[i].y, rgvLocal[cvertex * 2 - i - 1].x, rgvLocal[cvertex * 2 - i - 1].y);

   delete[] rgvLocal;
   delete[] pfCross;
   delete[] middlePoints;
}


void Guide::RenderBlueprint(Sur *psur, const bool solid)
{
   RenderOutline(psur, solid);
}

void Guide::GetBoundingVertices(std::vector<Vertex3Ds>& pvvertex3D)
{
   //!! meh, this is delivering something loosely related to the bounding vertices, but its only used in the cam fitting code so far, so keep for legacy reasons
   float *rgheight1;
   int cvertex;
   const Vertex2D * const rgvLocal = GetGuideVertex(cvertex, &rgheight1, NULL, NULL, NULL, HIT_SHAPE_DETAIL_LEVEL, false, true);

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
		   i & 1 ? bbox_min.x : bbox_max.x,
		   i & 2 ? bbox_min.y : bbox_max.y,
		   i & 4 ? bbox_min.z : bbox_max.z);

	   pvvertex3D.push_back(pv);
   }
}

void Guide::AssignHeightToControlPoint(const RenderVertex3D &v, float height)
{
   for (int i = 0; i < m_vdpoint.size(); i++)
   {
      if (m_vdpoint.ElementAt(i)->m_v.x == v.x && m_vdpoint.ElementAt(i)->m_v.y == v.y)
      {
         m_vdpoint.ElementAt(i)->m_calcHeight = height;
      }
   }
}
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
Vertex2D *Guide::GetGuideVertex(int &pcvertex, float ** const ppheight, bool ** const ppfCross, float ** const ppratio, Vertex2D ** const pMiddlePoints, const float _accuracy, const bool forRendering, const bool inc_width)
{
   std::vector<RenderVertex3D> vvertex;
   GetCentralCurve(vvertex, _accuracy);
   // vvertex are the 2D vertices forming the central curve of the ramp as seen from above

   const int cvertex = (int)vvertex.size();
   Vertex2D * const rgvLocal = new Vertex2D[(cvertex + 1) * 2];

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
      const RenderVertex3D & vprev = vvertex[(i>0) ? i - 1 : i];
      const RenderVertex3D & vnext = vvertex[(i < (cvertex - 1)) ? i + 1 : i];
      const RenderVertex3D & vmiddle = vvertex[i];

	  if (ppfCross)
		  (*ppfCross)[i] = vmiddle.fControlPoint;
	  
	  Vertex2D vnormal;
      {
         // Get normal at this point
         // Notice that these values equal the ones in the line
         // equation and could probably be substituted by them.
         Vertex2D v1normal(vprev.y - vmiddle.y, vmiddle.x - vprev.x);   // vector vmiddle-vprev rotated RIGHT
         Vertex2D v2normal(vmiddle.y - vnext.y, vnext.x - vmiddle.x);   // vector vnext-vmiddle rotated RIGHT

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
               const float C = -(A*(vprev.x - v1normal.x) + B*(vprev.y - v1normal.y));

               // Second line
               const float D = vnext.y - vmiddle.y;
               const float E = vmiddle.x - vnext.x;

               // Shift line along the normal
               const float F = -(D*(vnext.x - v2normal.x) + E*(vnext.y - v2normal.y));

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
      float widthcur = 60.0f; //percentage * (m_d.m_widthtop - m_d.m_widthbottom) + m_d.m_widthbottom;
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
      widthcur = m_d.m_wireDiameter;

      if (pMiddlePoints)
      {
         (*pMiddlePoints)[i] = Vertex2D(vmiddle.x, vmiddle.y) + vnormal;
      }
      rgvLocal[i] = Vertex2D(vmiddle.x, vmiddle.y) + (widthcur*0.5f) * vnormal;
      rgvLocal[cvertex * 2 - i - 1] = Vertex2D(vmiddle.x, vmiddle.y) - (widthcur*0.5f) * vnormal;
   }

   pcvertex = cvertex;
   return rgvLocal;
}

float Guide::GetSurfaceHeight(float x, float y)
{
   std::vector<RenderVertex3D> vvertex;
   GetCentralCurve(vvertex);

   int iSeg;
   Vertex2D vOut;
   ClosestPointOnPolygon(vvertex, Vertex2D(x, y), &vOut, &iSeg, false);

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

void Guide::GetTimers(Vector<HitTimer> * const pvht)
{
   IEditable::BeginPlay();

   HitTimer * const pht = new HitTimer();
   pht->m_interval = m_d.m_tdr.m_TimerInterval >= 0 ? max(m_d.m_tdr.m_TimerInterval, MAX_TIMER_MSEC_INTERVAL) : -1;
   pht->m_nextfire = pht->m_interval;
   pht->m_pfe = (IFireEvents *)this;

   m_phittimer = pht;

   if (m_d.m_tdr.m_fTimerEnabled)
      pvht->AddElement(pht);
}

void Guide::GetHitShapes(Vector<HitObject> * const pvho)
{
   float *rgheight1;
   int cvertex;
   Vertex2D * const rgvLocal = GetGuideVertex(cvertex, &rgheight1, NULL, NULL, NULL, HIT_SHAPE_DETAIL_LEVEL, false, true);

   float wallheightright, wallheightleft;

   // backwards compatible physics
   wallheightright = 31.0f;
   wallheightleft = 31.0f;

   // Add line segments for right ramp wall.
   if (wallheightright > 0.f)
   {
      for (int i = 0; i < (cvertex - 1); i++)
      {
         const Vertex2D &pv2 = rgvLocal[i];
         const Vertex2D &pv3 = rgvLocal[i + 1];

         AddLine(pvho, pv2, pv3, (i > 0), rgheight1[i], rgheight1[i + 1] + wallheightright);
         AddLine(pvho, pv3, pv2, (i < (cvertex - 2)), rgheight1[i], rgheight1[i + 1] + wallheightright);

         // add joints at start and end of right wall
         if (i == 0)
            AddJoint2D(pvho, pv2, rgheight1[0], rgheight1[0] + wallheightright);
         else if (i == cvertex - 2)
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

         AddLine(pvho, pv2, pv3, (i > 0), rgheight1[cvertex - i - 2], rgheight1[cvertex - i - 1] + wallheightleft);
         AddLine(pvho, pv3, pv2, (i < (cvertex - 2)), rgheight1[cvertex - i - 2], rgheight1[cvertex - i - 1] + wallheightleft);

         // add joints at start and end of left wall
         if (i == 0)
            AddJoint2D(pvho, pv2, rgheight1[cvertex - 1], rgheight1[cvertex - 1] + wallheightleft);
         else if (i == cvertex - 2)
            AddJoint2D(pvho, pv3, rgheight1[0], rgheight1[0] + wallheightleft);
      }
   }

   // Add hit triangles for the ramp floor.
   {
      HitTriangle *ph3dpolyOld = NULL;
      const Vertex2D *pv1, *pv2, *pv3, *pv4;

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
         pv1 = &rgvLocal[i];                    // i-th right
         pv2 = &rgvLocal[cvertex * 2 - i - 1];    // i-th left
         pv3 = &rgvLocal[cvertex * 2 - i - 2];    // (i+1)-th left
         pv4 = &rgvLocal[i + 1];                  // (i+1)-th right

         {
            // left ramp floor triangle, CCW order
            Vertex3Ds rgv3D[3];
            rgv3D[0] = Vertex3Ds(pv2->x, pv2->y, rgheight1[i]);
            rgv3D[1] = Vertex3Ds(pv1->x, pv1->y, rgheight1[i]);
            rgv3D[2] = Vertex3Ds(pv3->x, pv3->y, rgheight1[i + 1]);

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
         Vertex3Ds rgv3D[3];
         rgv3D[0] = Vertex3Ds(pv3->x, pv3->y, rgheight1[i + 1]);
         rgv3D[1] = Vertex3Ds(pv1->x, pv1->y, rgheight1[i]);
         rgv3D[2] = Vertex3Ds(pv4->x, pv4->y, rgheight1[i + 1]);

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
         Vertex3Ds v1(pv4->x, pv4->y, rgheight1[cvertex - 1]);
         Vertex3Ds v2(pv3->x, pv3->y, rgheight1[cvertex - 1]);
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
      const Vertex2D * const pv1 = &rgvLocal[i];
      const Vertex2D * const pv2 = &rgvLocal[cvertex * 2 - i - 1];
      const Vertex2D * const pv3 = &rgvLocal[cvertex * 2 - i - 2];
      const Vertex2D * const pv4 = &rgvLocal[i + 1];

      {
         // left ramp triangle, order CW
         const Vertex3Ds rgv3D[3] = {
            Vertex3Ds(pv1->x, pv1->y, rgheight1[i]),
            Vertex3Ds(pv2->x, pv2->y, rgheight1[i]),
            Vertex3Ds(pv3->x, pv3->y, rgheight1[i + 1]) };

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
         Vertex3Ds(pv3->x, pv3->y, rgheight1[i + 1]),
         Vertex3Ds(pv4->x, pv4->y, rgheight1[i + 1]),
         Vertex3Ds(pv1->x, pv1->y, rgheight1[i]) };

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

void Guide::GetHitShapesDebug(Vector<HitObject> * const pvho)
{
}

void Guide::CheckJoint(Vector<HitObject> * const pvho, const HitTriangle * const ph3d1, const HitTriangle * const ph3d2)
{
   if (ph3d1)   // may be null in case of degenerate triangles
   {
      const Vertex3Ds vjointnormal = CrossProduct(ph3d1->normal, ph3d2->normal);
      if (vjointnormal.LengthSquared() < 1e-8f)
         return;  // coplanar triangles need no joints
   }

   // By convention of the calling function, points 1 [0] and 2 [1] of the second polygon will
   // be the common-edge points
   AddJoint(pvho, ph3d2->m_rgv[0], ph3d2->m_rgv[1]);
}

void Guide::AddJoint(Vector<HitObject> * pvho, const Vertex3Ds& v1, const Vertex3Ds& v2)
{
   SetupHitObject(pvho, new HitLine3D(v1, v2));
}

void Guide::AddJoint2D(Vector<HitObject> * pvho, const Vertex2D& p, const float zlow, const float zhigh)
{
   SetupHitObject(pvho, new HitLineZ(p, zlow, zhigh));
}

void Guide::AddLine(Vector<HitObject> * const pvho, const Vertex2D &pv1, const Vertex2D &pv2, const bool pv3_exists, const float height1, const float height2)
{
   LineSeg * const plineseg = new LineSeg(pv1, pv2, height1, height2);

   SetupHitObject(pvho, plineseg);

   if (pv3_exists)
      AddJoint2D(pvho, pv1, height1, height2);
}

void Guide::SetupHitObject(Vector<HitObject> * pvho, HitObject * obj)
{
   const Material * const mat = m_ptable->GetMaterial( m_d.m_szPhysicsMaterial );
   if ( mat != NULL && !m_d.m_fOverwritePhysics )
   {
      obj->m_elasticity = mat->m_fElasticity;
      obj->SetFriction( mat->m_fFriction );
      obj->m_scatter = ANGTORAD( mat->m_fScatterAngle );
   }
   else
   {
      obj->m_elasticity = m_d.m_elasticity;
      obj->SetFriction( m_d.m_friction );
      obj->m_scatter = ANGTORAD( m_d.m_scatter );
   }

   obj->m_threshold = m_d.m_threshold;
   // the ramp is of type ePrimitive for triggering the event in HitTriangle::Collide()
   obj->m_ObjType = ePrimitive;
   obj->m_fEnabled = m_d.m_fCollidable;
   if (m_d.m_fHitEvent)
      obj->m_pfe = (IFireEvents *)this;
   else
      obj->m_pfe = NULL;

   pvho->AddElement(obj);
   m_vhoCollidable.push_back(obj); //remember hit components of primitive
}

void Guide::EndPlay()
{
   IEditable::EndPlay();
   m_vhoCollidable.clear();

   if (dynamicVertexBuffer) {
      dynamicVertexBuffer->release();
      dynamicVertexBuffer = 0;
      dynamicVertexBufferRegenerate = true;
   }

   if (dynamicIndexBuffer) {
      dynamicIndexBuffer->release();
      dynamicIndexBuffer = 0;
   }
}

float Guide::GetDepth(const Vertex3Ds& viewDir)
{
   // TODO: improve
   Vertex2D center2D;
   GetCenter(&center2D);
   const float centerZ = 0.5f * (m_d.m_heightbottom + m_d.m_heighttop);
   return m_d.m_depthBias + viewDir.x * center2D.x + viewDir.y * center2D.y + viewDir.z * centerZ;
}

bool Guide::isHabitrail() const
{
   return  true;
}

void Guide::RenderStaticHabitrail(RenderDevice* pd3dDevice, const Material * const mat)
{
   pd3dDevice->basicShader->SetMaterial(mat);

   pd3dDevice->SetRenderState(RenderDevice::DEPTHBIAS, 0);
   pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, TRUE);
   pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_NONE);

   pd3dDevice->basicShader->SetTechnique(mat->m_bIsMetal ? "basic_without_texture_isMetal" : "basic_without_texture_isNotMetal");
 
   pd3dDevice->basicShader->Begin(0);
   pd3dDevice->DrawIndexedPrimitiveVB(D3DPT_TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, dynamicVertexBuffer, 0, m_numVertices, dynamicIndexBuffer, 0, m_numIndices);
   pd3dDevice->basicShader->End();
}

void Guide::CreateWire(const int numRings, const int numSegments, const Vertex2D * const midPoints, Vertex3D_NoTex2 * const rgvbuf)
{
   Vertex3Ds prevB;
   for (int i = 0, index = 0; i < numRings; i++)
   {
      const int i2 = (i == (numRings - 1)) ? i : i + 1;
      const float height = rgheightInit[i];

      Vertex3Ds tangent(midPoints[i2].x - midPoints[i].x, midPoints[i2].y - midPoints[i].y, 0.0f);
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
         Vertex3Ds up(midPoints[i2].x + midPoints[i].x, midPoints[i2].y + midPoints[i].y, rgheightInit[i2] - height);
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

void Guide::GenerateWireMesh(Vertex3D_NoTex2 **meshBuf1, Vertex3D_NoTex2 **meshBuf2)
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
      accuracy = (int)(m_ptable->GetDetailLevel()*1.3f);
   }

   if(rgheightInit)
       delete [] rgheightInit;
   int splinePoints;
   const Vertex2D * const rgvLocal = GetGuideVertex(splinePoints, &rgheightInit, NULL, NULL, &middlePoints, -1, true, false);

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

   Vertex3D_NoTex2 *buf1 = *meshBuf1;

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

   delete[] rgvLocal;
   if (middlePoints)
      delete[] middlePoints;
   delete[] tmpPoints;
}

void Guide::prepareHabitrail(RenderDevice* pd3dDevice)
{
   dynamicVertexBufferRegenerate = false;
   Vertex3D_NoTex2 *tmpBuf1 = NULL;
   Vertex3D_NoTex2 *tmpBuf2 = NULL;
   GenerateWireMesh(&tmpBuf1, &tmpBuf2);

   if (dynamicVertexBuffer)
      dynamicVertexBuffer->release();

   pd3dDevice->CreateVertexBuffer(m_numVertices, 0, MY_D3DFVF_NOTEX2_VERTEX, &dynamicVertexBuffer); //!! use USAGE_DYNAMIC if it would actually be "really" dynamic

   // Draw the floor of the ramp.
   Vertex3D_NoTex2 *buf;
   dynamicVertexBuffer->lock(0, 0, (void**)&buf, VertexBuffer::WRITEONLY);
   memcpy(buf, tmpBuf1, sizeof(Vertex3D_NoTex2)*m_numVertices);
   dynamicVertexBuffer->unlock();

   if (dynamicIndexBuffer)
      dynamicIndexBuffer->release();

   dynamicIndexBuffer = pd3dDevice->CreateAndFillIndexBuffer(m_meshIndices);

   delete[] m_vertBuffer;
   delete[] tmpBuf1;

   m_meshIndices.clear();
}

void Guide::RenderSetup(RenderDevice* pd3dDevice)
{
   if (m_d.m_fVisible)
   {
      if (isHabitrail())
         prepareHabitrail(pd3dDevice);
      else
         GenerateVertexBuffer(pd3dDevice);
   }
}

void Guide::RenderStatic(RenderDevice* pd3dDevice)
{
   // return if not Visible
   if (!m_d.m_fVisible)
      return;

   if (m_ptable->m_fReflectionEnabled && !m_d.m_fReflectionEnabled)
      return;

   const Material * const mat = m_ptable->GetMaterial(m_d.m_szMaterial);

   // dont render alpha shaded ramps into static buffer, these are done per frame later-on
   if (mat->m_bOpacityActive)
      return;

   /* TODO: This is a misnomer right now, but clamp fixes some visual glitches (single-pixel lines)
    * with transparent textures. Probably the option should simply be renamed to ImageModeClamp,
    * since the texture coordinates always stay within [0,1] anyway.
    */
   //if (m_d.m_imagealignment == ImageModeWrap)
   //    pd3dDevice->SetTextureAddressMode(0, RenderDevice::TEX_CLAMP);

   if (isHabitrail())
      RenderStaticHabitrail(pd3dDevice, mat);
   else
      RenderRamp(pd3dDevice, mat);
}

void Guide::SetObjectPos()
{
   g_pvp->SetObjectPosCur(0, 0);
}

void Guide::MoveOffset(const float dx, const float dy)
{
   for (int i = 0; i < m_vdpoint.Size(); i++)
   {
      CComObject<DragPoint> * const pdp = m_vdpoint.ElementAt(i);

      pdp->m_v.x += dx;
      pdp->m_v.y += dy;
   }

   m_ptable->SetDirtyDraw();
}

void Guide::ClearForOverwrite()
{
   ClearPointsForOverwrite();
}

HRESULT Guide::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   BiffWriter bw(pstm, hcrypthash, hcryptkey);

   bw.WriteFloat(FID(HTBT), m_d.m_heightbottom);
   bw.WriteFloat(FID(HTTP), m_d.m_heighttop);
   bw.WriteString(FID(MATR), m_d.m_szMaterial);
   bw.WriteBool(FID(TMON), m_d.m_tdr.m_fTimerEnabled);
   bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
   bw.WriteWideString(FID(NAME), (WCHAR *)m_wzName);
   bw.WriteBool(FID(HTEV), m_d.m_fHitEvent);
   bw.WriteFloat(FID(THRS), m_d.m_threshold);
   bw.WriteFloat(FID(ELAS), m_d.m_elasticity);
   bw.WriteFloat(FID(RFCT), m_d.m_friction);
   bw.WriteFloat(FID(RSCT), m_d.m_scatter);
   bw.WriteBool(FID(CLDRP), m_d.m_fCollidable);
   bw.WriteBool(FID(RVIS), m_d.m_fVisible);
   bw.WriteFloat(FID(RADB), m_d.m_depthBias);
   bw.WriteFloat(FID(RADI), m_d.m_wireDiameter);
   bw.WriteBool(FID(REEN), m_d.m_fReflectionEnabled);
   bw.WriteString( FID( MAPH ), m_d.m_szPhysicsMaterial );
   bw.WriteBool( FID( OVPH ), m_d.m_fOverwritePhysics );

   ISelect::SaveData(pstm, hcrypthash, hcryptkey);

   bw.WriteTag(FID(PNTS));
   HRESULT hr;
   if (FAILED(hr = SavePointData(pstm, hcrypthash, hcryptkey)))
      return hr;

   bw.WriteTag(FID(ENDB));

   return S_OK;
}

HRESULT Guide::InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   SetDefaults(false);

   BiffReader br(pstm, this, pid, version, hcrypthash, hcryptkey);

   m_ptable = ptable;

   br.Load();
   return S_OK;
}

BOOL Guide::LoadToken(int id, BiffReader *pbr)
{
   if (id == FID(PIID))
   {
      pbr->GetInt((int *)pbr->m_pdata);
   }
   else if (id == FID(HTBT))
   {
      pbr->GetFloat(&m_d.m_heightbottom);
   }
   else if (id == FID(HTTP))
   {
      pbr->GetFloat(&m_d.m_heighttop);
   }
   else if (id == FID(MATR))
   {
      pbr->GetString(m_d.m_szMaterial);
   }
   else if (id == FID(TMON))
   {
      pbr->GetBool(&m_d.m_tdr.m_fTimerEnabled);
   }
   else if (id == FID(TMIN))
   {
      pbr->GetInt(&m_d.m_tdr.m_TimerInterval);
   }
   else if (id == FID(NAME))
   {
      pbr->GetWideString((WCHAR *)m_wzName);
   }
   else if (id == FID(HTEV))
   {
      pbr->GetBool(&m_d.m_fHitEvent);
   }
   else if (id == FID(THRS))
   {
      pbr->GetFloat(&m_d.m_threshold);
   }
   else if (id == FID(ELAS))
   {
      pbr->GetFloat(&m_d.m_elasticity);
   }
   else if (id == FID(RFCT))
   {
      pbr->GetFloat(&m_d.m_friction);
   }
   else if (id == FID(RSCT))
   {
      pbr->GetFloat(&m_d.m_scatter);
   }
   else if (id == FID(CLDRP))
   {
      pbr->GetBool(&m_d.m_fCollidable);
   }
   else if (id == FID(RVIS))
   {
      pbr->GetBool(&m_d.m_fVisible);
   }
   else if (id == FID(REEN))
   {
      pbr->GetBool(&m_d.m_fReflectionEnabled);
   }
   else if (id == FID(RADB))
   {
      pbr->GetFloat(&m_d.m_depthBias);
   }
   else if (id == FID(RADI))
   {
      pbr->GetFloat(&m_d.m_wireDiameter);
   }
   else if ( id == FID( MAPH ) )
   {
       pbr->GetString( m_d.m_szPhysicsMaterial);
   }
   else if ( id == FID( OVPH ) )
   {
       pbr->GetBool( &m_d.m_fOverwritePhysics );
   }
   else
   {
      LoadPointToken(id, pbr, pbr->m_version);
      ISelect::LoadToken(id, pbr);
   }

   return fTrue;
}

HRESULT Guide::InitPostLoad()
{
   return S_OK;
}

void Guide::AddPoint(int x, int y, const bool smooth)
{
   STARTUNDO
   const Vertex2D v = m_ptable->TransformPoint(x, y);

   std::vector<RenderVertex3D> vvertex;
   GetCentralCurve(vvertex);

   int iSeg;
   Vertex2D vOut;
   ClosestPointOnPolygon(vvertex, v, &vOut, &iSeg, false);

   // Go through vertices (including iSeg itself) counting control points until iSeg
   int icp = 0;
   for (int i = 0; i < (iSeg + 1); i++)
      if (vvertex[i].fControlPoint)
         icp++;

   //if (icp == 0) // need to add point after the last point
   //icp = m_vdpoint.Size();

   CComObject<DragPoint> *pdp;
   CComObject<DragPoint>::CreateInstance(&pdp);
   if (pdp)
   {
      pdp->AddRef();
      pdp->Init(this, vOut.x, vOut.y, (vvertex[max(iSeg - 1, 0)].z + vvertex[min(iSeg + 1, (int)vvertex.size() - 1)].z)*0.5f);
      pdp->m_calcHeight = 0.0f;
      pdp->m_fSmooth = smooth; // Ramps are usually always smooth
      m_vdpoint.InsertElementAt(pdp, icp); // push the second point forward, and replace it with this one.  Should work when index2 wraps.
   }

   SetDirtyDraw();

   STOPUNDO
}

void Guide::DoCommand(int icmd, int x, int y)
{
   ISelect::DoCommand(icmd, x, y);

   switch (icmd)
   {
   case ID_WALLMENU_FLIP:
   {
      Vertex2D vCenter;
      GetPointCenter(&vCenter);
      FlipPointY(&vCenter);
   }
   break;

   case ID_WALLMENU_MIRROR:
   {
      Vertex2D vCenter;
      GetPointCenter(&vCenter);
      FlipPointX(&vCenter);
   }
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

void Guide::FlipY(Vertex2D * const pvCenter)
{
   IHaveDragPoints::FlipPointY(pvCenter);
}

void Guide::FlipX(Vertex2D * const pvCenter)
{
   IHaveDragPoints::FlipPointX(pvCenter);
}

void Guide::Rotate(float ang, Vertex2D *pvCenter)
{
   IHaveDragPoints::RotatePoints(ang, pvCenter);
}

void Guide::Scale(float scalex, float scaley, Vertex2D *pvCenter)
{
   IHaveDragPoints::ScalePoints(scalex, scaley, pvCenter);
}

void Guide::Translate(Vertex2D *pvOffset)
{
   IHaveDragPoints::TranslatePoints(pvOffset);
}

STDMETHODIMP Guide::InterfaceSupportsErrorInfo(REFIID riid)
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

STDMETHODIMP Guide::get_HeightBottom(float *pVal)
{
   *pVal = m_d.m_heightbottom;

   return S_OK;
}

STDMETHODIMP Guide::put_HeightBottom(float newVal)
{
   if (m_d.m_heightbottom != newVal)
   {
      STARTUNDO

      m_d.m_heightbottom = newVal;
      dynamicVertexBufferRegenerate = true;

      STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Guide::get_HeightTop(float *pVal)
{
   *pVal = m_d.m_heighttop;

   return S_OK;
}

STDMETHODIMP Guide::put_HeightTop(float newVal)
{
   if (m_d.m_heighttop != newVal)
   {
      STARTUNDO

      m_d.m_heighttop = newVal;
      dynamicVertexBufferRegenerate = true;

      STOPUNDO
   }

   return S_OK;
}


STDMETHODIMP Guide::get_Material(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_d.m_szMaterial, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Guide::put_Material(BSTR newVal)
{
   STARTUNDO

   WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szMaterial, 32, NULL, NULL);

   STOPUNDO

   return S_OK;
}

void Guide::GetDialogPanes(Vector<PropertyPane> *pvproppane)
{
   PropertyPane *pproppane;

   pproppane = new PropertyPane(IDD_PROP_NAME, NULL);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPGUIDE_VISUALS, IDS_VISUALS);
   pvproppane->AddElement(pproppane);

   m_propPosition = new PropertyPane(IDD_PROPGUIDE_POSITION, IDS_POSITION);
   pvproppane->AddElement(m_propPosition);

   m_propPhysics = new PropertyPane(IDD_PROPGUIDE_PHYSICS, IDS_PHYSICS);
   pvproppane->AddElement(m_propPhysics);

   pproppane = new PropertyPane(IDD_PROP_TIMER, IDS_MISC);
   pvproppane->AddElement(pproppane);
}

STDMETHODIMP Guide::get_Elasticity(float *pVal)
{
   *pVal = m_d.m_elasticity;

   return S_OK;
}

STDMETHODIMP Guide::put_Elasticity(float newVal)
{
   STARTUNDO

   m_d.m_elasticity = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Guide::get_Friction(float *pVal)
{
   *pVal = m_d.m_friction;

   return S_OK;
}

STDMETHODIMP Guide::put_Friction(float newVal)
{
   newVal = clamp(newVal, 0.f, 1.f);

   STARTUNDO

   m_d.m_friction = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Guide::get_Scatter(float *pVal)
{
   *pVal = m_d.m_scatter;

   return S_OK;
}

STDMETHODIMP Guide::put_Scatter(float newVal)
{
   STARTUNDO

   m_d.m_scatter = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Guide::get_Collidable(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB((!g_pplayer) ? m_d.m_fCollidable : m_vhoCollidable[0]->m_fEnabled);

   return S_OK;
}

STDMETHODIMP Guide::put_Collidable(VARIANT_BOOL newVal)
{
   const BOOL fNewVal = VBTOF(newVal);
   if (!g_pplayer)
   {
      STARTUNDO

      m_d.m_fCollidable = !!fNewVal;

      STOPUNDO
   }
   else
   {
       const bool b = !!fNewVal;
       if (m_vhoCollidable.size() > 0 && m_vhoCollidable[0]->m_fEnabled != b)
           for (size_t i = 0; i < m_vhoCollidable.size(); i++) //!! costly
               m_vhoCollidable[i]->m_fEnabled = b; //copy to hit checking on entities composing the object
   }

   return S_OK;
}

STDMETHODIMP Guide::get_HasHitEvent(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fHitEvent);

   return S_OK;
}

STDMETHODIMP Guide::put_HasHitEvent(VARIANT_BOOL newVal)
{
   STARTUNDO

   m_d.m_fHitEvent = VBTOF(newVal);

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Guide::get_Threshold(float *pVal)
{
   *pVal = m_d.m_threshold;

   return S_OK;
}

STDMETHODIMP Guide::put_Threshold(float newVal)
{
   STARTUNDO

   m_d.m_threshold = newVal;

   STOPUNDO

   return S_OK;
}


STDMETHODIMP Guide::get_Visible(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fVisible);

   return S_OK;
}

STDMETHODIMP Guide::put_Visible(VARIANT_BOOL newVal)
{
   STARTUNDO
   m_d.m_fVisible = VBTOF(newVal);
   STOPUNDO

   return S_OK;
}

STDMETHODIMP Guide::get_ReflectionEnabled(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fReflectionEnabled);

   return S_OK;
}

STDMETHODIMP Guide::put_ReflectionEnabled(VARIANT_BOOL newVal)
{
   STARTUNDO

   m_d.m_fReflectionEnabled = VBTOF(newVal);

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Guide::get_DepthBias(float *pVal)
{
   *pVal = m_d.m_depthBias;

   return S_OK;
}

STDMETHODIMP Guide::put_DepthBias(float newVal)
{
   if (m_d.m_depthBias != newVal)
   {
      STARTUNDO

      m_d.m_depthBias = newVal;
      dynamicVertexBufferRegenerate = true;

      STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Guide::get_WireDiameter(float *pVal)
{
   *pVal = m_d.m_wireDiameter;

   return S_OK;
}

STDMETHODIMP Guide::put_WireDiameter(float newVal)
{
   if (m_d.m_wireDiameter != newVal)
   {
      STARTUNDO

      m_d.m_wireDiameter = newVal;

      STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Guide::get_PhysicsMaterial( BSTR *pVal )
{
    WCHAR wz[512];

    MultiByteToWideChar( CP_ACP, 0, m_d.m_szPhysicsMaterial, -1, wz, 32 );
    *pVal = SysAllocString( wz );

    return S_OK;
}

STDMETHODIMP Guide::put_PhysicsMaterial( BSTR newVal )
{
    STARTUNDO

    WideCharToMultiByte( CP_ACP, 0, newVal, -1, m_d.m_szPhysicsMaterial, 32, NULL, NULL );

    STOPUNDO

    return S_OK;
}

STDMETHODIMP Guide::get_OverwritePhysics( VARIANT_BOOL *pVal )
{
    *pVal = (VARIANT_BOOL)FTOVB( m_d.m_fOverwritePhysics );

    return S_OK;
}

STDMETHODIMP Guide::put_OverwritePhysics( VARIANT_BOOL newVal )
{
    STARTUNDO

    m_d.m_fOverwritePhysics = VBTOF( newVal );

    STOPUNDO

    return S_OK;
}


void Guide::ExportMesh(FILE *f)
{
   char name[MAX_PATH];
   if (m_d.m_fVisible)
   {
      WideCharToMultiByte(CP_ACP, 0, m_wzName, -1, name, MAX_PATH, NULL, NULL);
      {
         Vertex3D_NoTex2 *tmpBuf1 = NULL, *tmpBuf2 = NULL;
         GenerateWireMesh(&tmpBuf1, &tmpBuf2);
         const Material * const mat = m_ptable->GetMaterial(m_d.m_szMaterial);
         WaveFrontObj_WriteObjectName(f, name);
         WaveFrontObj_WriteVertexInfo(f, tmpBuf1, m_numVertices);
         WaveFrontObj_WriteMaterial(m_d.m_szMaterial, NULL, mat);
         WaveFrontObj_UseTexture(f, m_d.m_szMaterial);
         WaveFrontObj_WriteFaceInfo(f, m_meshIndices);
         WaveFrontObj_UpdateFaceOffset(m_numVertices);

         delete[] tmpBuf1;
      }
   }
}

void Guide::RenderRamp(RenderDevice *pd3dDevice, const Material * const mat)
{
   if (!mat)
      return;

   // see the comment in RenderStatic() above
   RenderStaticHabitrail(pd3dDevice, mat);
}

// Always called each frame to render over everything else (along with primitives)
// Same code as RenderStatic (with the exception of the alpha tests).
// Also has less drawing calls by bundling seperate calls.
void Guide::PostRenderStatic(RenderDevice* pd3dDevice)
{
   TRACE_FUNCTION();

   const Material * const mat = m_ptable->GetMaterial(m_d.m_szMaterial);
   // don't render if invisible or not a transparent ramp
   if (!m_d.m_fVisible || (!mat->m_bOpacityActive))
      return;
   if (m_ptable->m_fReflectionEnabled && !m_d.m_fReflectionEnabled)
      return;

   RenderRamp(pd3dDevice, mat);
}

void Guide::GenerateRampMesh(Vertex3D_NoTex2 **meshBuf)
{
   float *rgheight;
   float *rgratio = NULL;
   const Vertex2D * const rgvLocal = GetGuideVertex(rampVertex, &rgheight, NULL, &rgratio, NULL, -1, true, true);

   const float inv_tablewidth = 1.0f / (m_ptable->m_right - m_ptable->m_left);
   const float inv_tableheight = 1.0f / (m_ptable->m_bottom - m_ptable->m_top);

   m_numVertices = rampVertex * 2;
   const unsigned int rgioffset = (rampVertex - 1) * 6;
   m_numIndices = rgioffset * 3; // to draw the full ramp in one go (could only use *1, and draw three times with offsets into vertices)

   if (*meshBuf == NULL)
   {
      *meshBuf = new Vertex3D_NoTex2[m_numVertices * 3];
   }
   Vertex3D_NoTex2 * const buf = *meshBuf;

   m_vertBuffer = new Vertex3D_NoTex2[m_numVertices];
   m_meshIndices.resize(m_numIndices);
   for (int i = 0; i < rampVertex; i++)
   {
      Vertex3D_NoTex2 * const rgv3D = m_vertBuffer + i * 2;
      rgv3D[0].x = rgvLocal[i].x;
      rgv3D[0].y = rgvLocal[i].y;
      rgv3D[0].z = rgheight[i] * m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];

      rgv3D[1].x = rgvLocal[rampVertex * 2 - i - 1].x;
      rgv3D[1].y = rgvLocal[rampVertex * 2 - i - 1].y;
      rgv3D[1].z = rgv3D[0].z;

      rgv3D[0].tu = 0.0f;
      rgv3D[0].tv = 0.0f;
      rgv3D[1].tu = 0.0f;
      rgv3D[1].tv = 0.0f;

      if (i == rampVertex - 1)
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
   ComputeNormals(m_vertBuffer, m_numVertices, m_meshIndices.data(), (rampVertex - 1) * 6);

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

   delete[] m_vertBuffer;
   delete[] rgvLocal;
   delete[] rgheight;
   if (rgratio)
      delete[] rgratio;
}

void Guide::GenerateVertexBuffer(RenderDevice* pd3dDevice)
{
   dynamicVertexBufferRegenerate = false;

   Vertex3D_NoTex2 *tmpBuffer = NULL;
   GenerateRampMesh(&tmpBuffer);

   if (dynamicVertexBuffer)
      dynamicVertexBuffer->release();

   pd3dDevice->CreateVertexBuffer(m_numVertices * 3, 0, MY_D3DFVF_NOTEX2_VERTEX, &dynamicVertexBuffer); //!! use USAGE_DYNAMIC if it would actually be "really" dynamic

   Vertex3D_NoTex2 *buf;
   dynamicVertexBuffer->lock(0, 0, (void**)&buf, VertexBuffer::WRITEONLY);
   memcpy(buf, tmpBuffer, sizeof(Vertex3D_NoTex2)*m_numVertices * 3);
   dynamicVertexBuffer->unlock();

   if (dynamicIndexBuffer)
      dynamicIndexBuffer->release();

   dynamicIndexBuffer = pd3dDevice->CreateAndFillIndexBuffer(m_meshIndices);
   delete[] tmpBuffer;
}

void Guide::UpdatePropertyPanes()
{
   if (m_propPosition == NULL || m_propPhysics == NULL)
      return;

   EnableWindow(GetDlgItem(m_propPosition->dialogHwnd, 3), FALSE);
   EnableWindow(GetDlgItem(m_propPosition->dialogHwnd, 4), FALSE);
   EnableWindow(GetDlgItem(m_propPosition->dialogHwnd, 108), FALSE);
   EnableWindow(GetDlgItem(m_propPosition->dialogHwnd, 109), FALSE);
   EnableWindow(GetDlgItem(m_propPosition->dialogHwnd, IDC_WIRE_DIAMETER), TRUE);
   EnableWindow(GetDlgItem(m_propPosition->dialogHwnd, IDC_WIRE_DISTX), TRUE);
   EnableWindow(GetDlgItem(m_propPosition->dialogHwnd, IDC_WIRE_DISTY), TRUE);

   if (!m_d.m_fCollidable)
   {
      EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, 10), FALSE);
      EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, 11), FALSE);
      EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, 110), FALSE);
      EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, 114), FALSE);
      EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, 115), FALSE);
      EnableWindow( GetDlgItem( m_propPhysics->dialogHwnd, IDC_MATERIAL_COMBO4 ), FALSE );
      EnableWindow( GetDlgItem( m_propPhysics->dialogHwnd, IDC_OVERWRITE_MATERIAL_SETTINGS ), FALSE );
   }
   else
   {
      EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, 10), TRUE);
      EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, 11), TRUE);
      EnableWindow( GetDlgItem( m_propPhysics->dialogHwnd, IDC_OVERWRITE_MATERIAL_SETTINGS ), TRUE );
      if( m_d.m_fOverwritePhysics)
      {
          EnableWindow( GetDlgItem( m_propPhysics->dialogHwnd, IDC_MATERIAL_COMBO4 ), FALSE );
          EnableWindow( GetDlgItem( m_propPhysics->dialogHwnd, 110 ), TRUE );
          EnableWindow( GetDlgItem( m_propPhysics->dialogHwnd, 114 ), TRUE );
          EnableWindow( GetDlgItem( m_propPhysics->dialogHwnd, 115 ), TRUE );
      }
      else
      {
          EnableWindow( GetDlgItem( m_propPhysics->dialogHwnd, IDC_MATERIAL_COMBO4 ), TRUE );
          EnableWindow( GetDlgItem( m_propPhysics->dialogHwnd, 110 ), FALSE );
          EnableWindow( GetDlgItem( m_propPhysics->dialogHwnd, 114 ), FALSE );
          EnableWindow( GetDlgItem( m_propPhysics->dialogHwnd, 115 ), FALSE );
      }
   }
}

void Guide::SetDefaultPhysics(bool fromMouseClick)
{
   static const char strKeyName[] = "DefaultProps\\Ramp";
   m_d.m_elasticity = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "Elasticity", 0.3f) : 0.3f;
   m_d.m_friction = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "Friction", 0.3f) : 0.3f;
   m_d.m_scatter = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "Scatter", 0) : 0;
}
