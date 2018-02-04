#include "StdAfx.h"
//#include "forsyth.h"
#include "objloader.h"

Rubber::Rubber()
{
   m_menuid = IDR_SURFACEMENU;
   m_d.m_fCollidable = true;
   m_d.m_fVisible = true;
   m_d.m_fHitEvent = false;
   dynamicVertexBuffer = 0;
   dynamicIndexBuffer = 0;
   dynamicVertexBufferRegenerate = true;
   m_propPhysics = NULL;
   m_propPosition = NULL;
   m_propVisual = NULL;
   memset(m_d.m_szImage, 0, MAXTOKEN);
   memset(m_d.m_szMaterial, 0, 32);
   memset(m_d.m_szPhysicsMaterial, 0, 32);
   m_d.m_fOverwritePhysics=true;
   m_ptable = NULL;
   m_d.m_tdr.m_fTimerEnabled = false;
   m_d.m_tdr.m_TimerInterval = 0;
}

Rubber::~Rubber()
{
   if (dynamicVertexBuffer)
      dynamicVertexBuffer->release();

   if (dynamicIndexBuffer)
      dynamicIndexBuffer->release();
}

void Rubber::UpdateUnitsInfo()
{
   char tbuf[128];
   sprintf_s(tbuf, "Height: %.3f | Thickness: %.3f", g_pvp->ConvertToUnit(m_d.m_height), g_pvp->ConvertToUnit((float)m_d.m_thickness));
   g_pvp->SetStatusBarUnitInfo(tbuf);
}

HRESULT Rubber::Init(PinTable *ptable, float x, float y, bool fromMouseClick)
{
   m_ptable = ptable;
   m_d.m_fVisible = true;

   //float length = 0.5f * GetRegStringAsFloatWithDefault("DefaultProps\\Rubber", "Length", 400.0f);

   CComObject<DragPoint> *pdp;
   for (int i = 8; i > 0; i--)
   {
      const float angle = (float)(M_PI*2.0 / 8.0)*(float)i;
      const float xx = x + sinf(angle)*50.0f;
      const float yy = y - cosf(angle)*50.0f;
      CComObject<DragPoint>::CreateInstance(&pdp);
      if (pdp)
      {
         pdp->AddRef();
         pdp->Init(this, xx, yy);
         pdp->m_fSmooth = true;
         m_vdpoint.AddElement(pdp);
      }
   }

   SetDefaults(fromMouseClick);

   InitVBA(fTrue, 0, NULL);

   return S_OK;
}

void Rubber::SetDefaults(bool fromMouseClick)
{
   static const char strKeyName[] = "DefaultProps\\Rubber";

   m_d.m_height = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "Height", 25.0f) : 25.0f;
   m_d.m_thickness = fromMouseClick ? GetRegIntWithDefault(strKeyName, "Thickness", 8) : 8;

   m_d.m_tdr.m_fTimerEnabled = fromMouseClick ? GetRegBoolWithDefault(strKeyName, "TimerEnabled", false) : false;
   m_d.m_tdr.m_TimerInterval = fromMouseClick ? GetRegIntWithDefault(strKeyName, "TimerInterval", 100) : 100;

   const HRESULT hr = GetRegString(strKeyName, "Image", m_d.m_szImage, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szImage[0] = 0;

   m_d.m_fHitEvent = fromMouseClick ? GetRegBoolWithDefault(strKeyName, "HitEvent", false) : false;

   SetDefaultPhysics(fromMouseClick);

   m_d.m_fVisible = fromMouseClick ? GetRegBoolWithDefault(strKeyName, "Visible", true) : true;
   m_d.m_fCollidable = fromMouseClick ? GetRegBoolWithDefault(strKeyName, "Collidable", true) : true;

   m_d.m_staticRendering = fromMouseClick ? GetRegBoolWithDefault(strKeyName, "EnableStaticRendering", true) : true;
   m_d.m_showInEditor = fromMouseClick ? GetRegBoolWithDefault(strKeyName, "EnableShowInEditor", false) : false;

   m_d.m_rotX = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "RotX", 0.0f) : 0.0f;
   m_d.m_rotY = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "RotY", 0.0f) : 0.0f;
   m_d.m_rotZ = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "RotZ", 0.0f) : 0.0f;

   m_d.m_fReflectionEnabled = fromMouseClick ? GetRegBoolWithDefault(strKeyName, "ReflectionEnabled", true) : true;
}

void Rubber::WriteRegDefaults()
{
   static const char strKeyName[] = "DefaultProps\\Rubber";

   SetRegValueFloat(strKeyName, "Height", m_d.m_height);
   SetRegValueFloat(strKeyName, "HitHeight", m_d.m_hitHeight);
   SetRegValueInt(strKeyName, "Thickness", m_d.m_thickness);
   SetRegValueBool(strKeyName, "HitEvent", m_d.m_fHitEvent);
   SetRegValueBool(strKeyName, "TimerEnabled", m_d.m_tdr.m_fTimerEnabled);
   SetRegValue(strKeyName, "TimerInterval", REG_DWORD, &m_d.m_tdr.m_TimerInterval, 4);
   SetRegValue(strKeyName, "Image", REG_SZ, &m_d.m_szImage, lstrlen(m_d.m_szImage));
   SetRegValueFloat(strKeyName, "Elasticity", m_d.m_elasticity);
   SetRegValueFloat(strKeyName, "ElasticityFalloff", m_d.m_elasticityFalloff);
   SetRegValueFloat(strKeyName, "Friction", m_d.m_friction);
   SetRegValueFloat(strKeyName, "Scatter", m_d.m_scatter);
   SetRegValueBool(strKeyName, "Collidable", m_d.m_fCollidable);
   SetRegValueBool(strKeyName, "Visible", m_d.m_fVisible);
   SetRegValueBool(strKeyName, "EnableStaticRendering", m_d.m_staticRendering);
   SetRegValueBool(strKeyName, "EnableShowInEditor", m_d.m_showInEditor);
   SetRegValueFloat(strKeyName, "RotX", m_d.m_rotX);
   SetRegValueFloat(strKeyName, "RotY", m_d.m_rotY);
   SetRegValueFloat(strKeyName, "RotZ", m_d.m_rotZ);
   SetRegValueBool(strKeyName, "ReflectionEnabled", m_d.m_fReflectionEnabled);
}

void Rubber::GetPointDialogPanes(Vector<PropertyPane> *pvproppane)
{
   PropertyPane *pproppane;

   pproppane = new PropertyPane(IDD_PROPPOINT_VISUALS, IDS_VISUALS);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPPOINT_POSITION, IDS_POSITION);
   pvproppane->AddElement(pproppane);
}

void Rubber::DrawRubberMesh(Sur * const psur)
{
   std::vector<Vertex2D> drawVertices;

   GenerateMesh(4);
   UpdateRubber(NULL, false, m_d.m_height);

   for (int i = 0; i < (int)ringIndices.size(); i += 3)
   {
      Vertex3Ds A = Vertex3Ds(m_vertices[ringIndices[i]].x, m_vertices[ringIndices[i]].y, m_vertices[ringIndices[i]].z);
      Vertex3Ds B = Vertex3Ds(m_vertices[ringIndices[i + 1]].x, m_vertices[ringIndices[i + 1]].y, m_vertices[ringIndices[i + 1]].z);
      Vertex3Ds C = Vertex3Ds(m_vertices[ringIndices[i + 2]].x, m_vertices[ringIndices[i + 2]].y, m_vertices[ringIndices[i + 2]].z);
      if (fabsf(m_vertices[ringIndices[i]].nz + m_vertices[ringIndices[i + 1]].nz) < 1.f)
      {
         drawVertices.push_back(Vertex2D(A.x, A.y));
         drawVertices.push_back(Vertex2D(B.x, B.y));
      }
      if (fabsf(m_vertices[ringIndices[i + 1]].nz + m_vertices[ringIndices[i + 2]].nz) < 1.f)
      {
         drawVertices.push_back(Vertex2D(B.x, B.y));
         drawVertices.push_back(Vertex2D(C.x, C.y));
      }
      if (fabsf(m_vertices[ringIndices[i + 2]].nz + m_vertices[ringIndices[i]].nz) < 1.f)
      {
         drawVertices.push_back(Vertex2D(C.x, C.y));
         drawVertices.push_back(Vertex2D(A.x, A.y));
      }
   }
   if (drawVertices.size() > 0)
      psur->Lines(drawVertices.data(), (int)(drawVertices.size() / 2));

}

void Rubber::PreRender(Sur * const psur)
{
   //make 1 wire ramps look unique in editor - uses ramp color
    psur->SetLineColor( RGB( 0, 0, 0 ), false, 0 );
   if (m_ptable->RenderSolid())
      psur->SetFillColor(RGB(192, 192, 192));
   else
      psur->SetFillColor(-1);
   psur->SetBorderColor(-1, false, 0);
   psur->SetObject(this);

   if (!m_d.m_showInEditor)
   {
      int cvertex;
      const Vertex2D * const rgvLocal = GetSplineVertex(cvertex, NULL, NULL);
      psur->Polygon(rgvLocal, cvertex* 2);
      delete[] rgvLocal;
   }
   else
   {
      DrawRubberMesh(psur);
   }
}

void Rubber::Render(Sur * const psur)
{
   psur->SetFillColor(-1);
   psur->SetBorderColor(RGB(0, 0, 0), false, 0);
   psur->SetLineColor(RGB(0, 0, 0), false, 0);
   psur->SetObject(this);
   psur->SetObject(NULL); // NULL so this won't be hit-tested

   if (!m_d.m_showInEditor)
   {
      int cvertex;
      bool *pfCross;
      const Vertex2D * const rgvLocal = GetSplineVertex(cvertex, &pfCross, NULL);

      psur->Polygon(rgvLocal, cvertex* 2);
      for (int i = 0; i < cvertex; i++)
         if (pfCross[i])
            psur->Line(rgvLocal[i].x, rgvLocal[i].y, rgvLocal[cvertex * 2 - i - 1].x, rgvLocal[cvertex * 2 - i - 1].y);

      delete[] rgvLocal;
      delete[] pfCross;
   }
   else
   {
      DrawRubberMesh(psur);
      // if y rotation is used don't show dragpoints
      return;
   }


   bool fDrawDragpoints = ((m_selectstate != eNotSelected) || (g_pvp->m_fAlwaysDrawDragPoints));

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
      for (int i = 0; i < m_vdpoint.Size(); i++)
      {
         CComObject<DragPoint> * const pdp = m_vdpoint.ElementAt(i);
         psur->SetFillColor(-1);
         psur->SetBorderColor(RGB(255, 0, 0), false, 0);
         psur->SetObject(pdp);

         if (pdp->m_fDragging)
            psur->SetBorderColor(RGB(0, 255, 0), false, 0);

         psur->Ellipse2(pdp->m_v.x, pdp->m_v.y, 8);
      }
   }
}

void Rubber::RenderOutline(Sur * const psur, const bool solid)
{
   if (solid)
      psur->SetFillColor(BLUEPRINT_SOLID_COLOR);
   else
      psur->SetFillColor(-1);

   psur->SetBorderColor(RGB(0, 0, 0), false, 0);
   psur->SetLineColor(RGB(0, 0, 0), false, 0);
   psur->SetObject(this);
   psur->SetObject(NULL); // NULL so this won't be hit-tested

   if (!m_d.m_showInEditor)
   {
      int cvertex;
      bool *pfCross;
      const Vertex2D * const rgvLocal = GetSplineVertex(cvertex, &pfCross, NULL);

      psur->Polygon(rgvLocal, cvertex* 2);
      for (int i = 0; i < cvertex; i++)
         if (pfCross[i])
            psur->Line(rgvLocal[i].x, rgvLocal[i].y, rgvLocal[cvertex * 2 - i - 1].x, rgvLocal[cvertex * 2 - i - 1].y);

      delete[] rgvLocal;
      delete[] pfCross;
   }
   else
   {
      DrawRubberMesh(psur);
   }
}

void Rubber::RenderBlueprint(Sur *psur, const bool solid)
{
   RenderOutline(psur, solid);
}

void Rubber::GetBoundingVertices(std::vector<Vertex3Ds>& pvvertex3D)
{
   //!! meh, this is delivering something loosely related to the bounding vertices, but its only used in the cam fitting code so far, so keep for legacy reasons
   int cvertex;
   const Vertex2D * const rgvLocal = GetSplineVertex(cvertex, NULL, NULL);

   //pvvertex3D.reserve(pvvertex3D.size() + cvertex * 2);
   Vertex3Ds bbox_min(FLT_MAX, FLT_MAX, FLT_MAX);
   Vertex3Ds bbox_max(-FLT_MAX, -FLT_MAX, -FLT_MAX);
   for (int i = 0; i < cvertex; i++)
   {
      {
      const Vertex3Ds pv(rgvLocal[i].x,rgvLocal[i].y,m_d.m_height + (float)(2.0*PHYS_SKIN)); // leave room for ball //!! use ballsize
      //pvvertex3D.push_back(pv);
	  bbox_min.x = min(bbox_min.x, pv.x);
	  bbox_min.y = min(bbox_min.y, pv.y);
	  bbox_min.z = min(bbox_min.z, pv.z);
	  bbox_max.x = max(bbox_max.x, pv.x);
	  bbox_max.y = max(bbox_max.y, pv.y);
	  bbox_max.z = max(bbox_max.z, pv.z);
	  }

      const Vertex3Ds pv(rgvLocal[cvertex * 2 - i - 1].x,rgvLocal[cvertex * 2 - i - 1].y,m_d.m_height + (float)(2.0*PHYS_SKIN)); // leave room for ball //!! use ballsize
      //pvvertex3D.push_back(pv);
	  bbox_min.x = min(bbox_min.x, pv.x);
	  bbox_min.y = min(bbox_min.y, pv.y);
	  bbox_min.z = min(bbox_min.z, pv.z);
	  bbox_max.x = max(bbox_max.x, pv.x);
	  bbox_max.y = max(bbox_max.y, pv.y);
	  bbox_max.z = max(bbox_max.z, pv.z);
   }

   delete[] rgvLocal;

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

/*
 * Compute the vertices and additional information for the ramp shape.
 *
 * Output:
 *  pcvertex     - number of vertices for the central curve
 *  return value - size 2*cvertex, vertices forming the 2D outline of the ramp
 *                 order: first forward along right side of ramp, then backward along the left side
 *  ppfCross     - size cvertex, true if i-th vertex corresponds to a control point
 */
Vertex2D *Rubber::GetSplineVertex(int &pcvertex, bool ** const ppfCross, Vertex2D ** const pMiddlePoints)
{
   std::vector<RenderVertex> vvertex;
   GetCentralCurve(vvertex);
   // vvertex are the 2D vertices forming the central curve of the rubber as seen from above

   const int cvertex = (int)vvertex.size();
   Vertex2D * const rgvLocal = new Vertex2D[(cvertex + 1) * 2];

   if (pMiddlePoints)
   {
      *pMiddlePoints = new Vertex2D[cvertex + 1];
   }
   if (ppfCross)
   {
	   *ppfCross = new bool[cvertex + 1];
   }

   for (int i = 0; i < cvertex; i++)
   {
      const RenderVertex & vprev = vvertex[(i>0) ? i - 1 : i];
      const RenderVertex & vnext = vvertex[(i < (cvertex - 1)) ? i + 1 : 0];
      const RenderVertex & vmiddle = vvertex[i];

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

      const float widthcur = (float)m_d.m_thickness;

      if (pMiddlePoints)
         (*pMiddlePoints)[i] = vmiddle;

      rgvLocal[i] = vmiddle + (widthcur*0.5f) * vnormal;
      rgvLocal[(cvertex + 1) * 2 - i - 1] = vmiddle - (widthcur*0.5f) * vnormal;
      if (i == 0)
      {
         rgvLocal[cvertex] = rgvLocal[0];
         rgvLocal[(cvertex + 1) * 2 - cvertex - 1] = rgvLocal[(cvertex + 1) * 2 - 1];
      }
   }

   if (ppfCross)
   {
      (*ppfCross)[cvertex] = vvertex[0].fControlPoint;
   }
   if (pMiddlePoints)
   {
      (*pMiddlePoints)[cvertex] = (*pMiddlePoints)[0];
   }

   pcvertex = cvertex + 1;
   return rgvLocal;
}

/*
 * Get an approximation of the curve described by the control points of this ramp.
 */
void Rubber::GetCentralCurve(std::vector<RenderVertex> & vv)
{
   IHaveDragPoints::GetRgVertex(vv);
}

float Rubber::GetSurfaceHeight(float x, float y)
{
   std::vector<RenderVertex> vvertex;
   GetCentralCurve(vvertex);

   const int cvertex = (int)vvertex.size();

   int iSeg;
   Vertex2D vOut;
   ClosestPointOnPolygon(vvertex, Vertex2D(x, y), &vOut, &iSeg, false);

   // Go through vertices (including iSeg itself) counting control points until iSeg
   float totallength = 0.f;
   float startlength = 0.f;
   float zheight = 0.f;

   if (iSeg == -1)
   {
      return 0; // Object is not on ramp path
   }

   for (int i2 = 1; i2 < cvertex; i2++)
   {
      const float dx = vvertex[i2].x - vvertex[i2 - 1].x;
      const float dy = vvertex[i2].y - vvertex[i2 - 1].y;
      const float len = sqrtf(dx*dx + dy*dy);
      if (i2 <= iSeg)
         startlength += len;

      totallength += len;
   }

    {
       const float dx = vOut.x - vvertex[iSeg].x;
       const float dy = vOut.y - vvertex[iSeg].y;
       const float len = sqrtf(dx*dx + dy*dy);
       startlength += len; // Add the distance the object is between the two closest polyline segments.  Matters mostly for straight edges.

       zheight = (startlength / totallength) * m_d.m_height;
    }

    return zheight*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];
}

void Rubber::GetTimers(Vector<HitTimer> * const pvht)
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

void Rubber::GetHitShapes(Vector<HitObject> * const pvho)
{
   std::set< std::pair<unsigned, unsigned> > addedEdges;

   GenerateMesh(6, true); //!! adapt hacky code in the function if changing the "6" here
   UpdateRubber(NULL, false, m_d.m_hitHeight);

   // add collision triangles and edges
   for (unsigned i = 0; i < ringIndices.size(); i += 3)
   {
      Vertex3Ds rgv3D[3];
      // NB: HitTriangle wants CCW vertices, but for rendering we have them in CW order
      Vertex3D_NoTex2 *v = &m_vertices[ringIndices[i]];
      rgv3D[0] = Vertex3Ds(v->x, v->y, v->z);
      v = &m_vertices[ringIndices[i + 2]];
      rgv3D[1] = Vertex3Ds(v->x, v->y, v->z);
      v = &m_vertices[ringIndices[i + 1]];
      rgv3D[2] = Vertex3Ds(v->x, v->y, v->z);
      SetupHitObject(pvho, new HitTriangle(rgv3D));

      AddHitEdge(pvho, addedEdges, ringIndices[i], ringIndices[i + 2]);
      AddHitEdge(pvho, addedEdges, ringIndices[i + 2], ringIndices[i + 1]);
      AddHitEdge(pvho, addedEdges, ringIndices[i + 1], ringIndices[i]);
   }

   // add collision vertices
   for (unsigned i = 0; i < m_vertices.size(); ++i)
   {
      Vertex3Ds v = Vertex3Ds(m_vertices[i].x, m_vertices[i].y, m_vertices[i].z);
      SetupHitObject(pvho, new HitPoint(v));
   }
}

void Rubber::AddHitEdge(Vector<HitObject> * pvho, std::set< std::pair<unsigned, unsigned> >& addedEdges, const unsigned i, const unsigned j)
{
   // create pair uniquely identifying the edge (i,j)
   std::pair<unsigned, unsigned> p(std::min(i, j), std::max(i, j));

   if (addedEdges.count(p) == 0)   // edge not yet added?
   {
      addedEdges.insert(p);
      const Vertex3Ds v1(m_vertices[i].x, m_vertices[i].y, m_vertices[i].z);
      const Vertex3Ds v2(m_vertices[j].x, m_vertices[j].y, m_vertices[j].z);
      SetupHitObject(pvho, new HitLine3D(v1, v2));
   }
}

void Rubber::SetupHitObject(Vector<HitObject> * pvho, HitObject * obj)
{
   const Material *const mat = m_ptable->GetMaterial( m_d.m_szPhysicsMaterial );
   if ( mat != NULL && !m_d.m_fOverwritePhysics )
   {
      obj->m_elasticity = mat->m_fElasticity;
      obj->m_elasticityFalloff = mat->m_fElasticityFalloff;
      obj->SetFriction( mat->m_fFriction );
      obj->m_scatter = ANGTORAD( mat->m_fScatterAngle );
   }
   else
   {
      obj->m_elasticity = m_d.m_elasticity;
      obj->m_elasticityFalloff = m_d.m_elasticityFalloff;
      obj->SetFriction( m_d.m_friction );
      obj->m_scatter = ANGTORAD( m_d.m_scatter );
   }
   obj->m_fEnabled = m_d.m_fCollidable;
   // the rubber is of type ePrimitive for triggering the event in HitTriangle::Collide()
   obj->m_ObjType = ePrimitive;
   // hard coded threshold for now
   obj->m_threshold = 2.0f;
   obj->m_obj = (IFireEvents *)this;
   if (m_d.m_fHitEvent)
      obj->m_fe = true;
   else
      obj->m_fe = false;

   pvho->AddElement(obj);
   m_vhoCollidable.push_back(obj);	//remember hit components of primitive
}

void Rubber::GetHitShapesDebug(Vector<HitObject> * const pvho)
{
}

void Rubber::EndPlay()
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

float Rubber::GetDepth(const Vertex3Ds& viewDir)
{
   Vertex2D center2D;
   GetCenter(&center2D);
   return viewDir.x * center2D.x + viewDir.y * center2D.y + viewDir.z * m_d.m_height;
}

void Rubber::RenderSetup(RenderDevice* pd3dDevice)
{
   GenerateVertexBuffer(pd3dDevice);
}

void Rubber::RenderStatic(RenderDevice* pd3dDevice)
{
   if (m_d.m_staticRendering)
   {
      if (m_ptable->m_fReflectionEnabled && !m_d.m_fReflectionEnabled)
         return;

      RenderObject(pd3dDevice);
   }
}

void Rubber::SetObjectPos()
{
   g_pvp->SetObjectPosCur(0, 0);
}

void Rubber::MoveOffset(const float dx, const float dy)
{
   for (int i = 0; i < m_vdpoint.Size(); i++)
   {
      CComObject<DragPoint> * const pdp = m_vdpoint.ElementAt(i);

      pdp->m_v.x += dx;
      pdp->m_v.y += dy;
   }

   m_ptable->SetDirtyDraw();
}

void Rubber::ClearForOverwrite()
{
   ClearPointsForOverwrite();
}

HRESULT Rubber::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   BiffWriter bw(pstm, hcrypthash, hcryptkey);

   bw.WriteFloat(FID(HTTP), m_d.m_height);
   bw.WriteFloat(FID(HTHI), m_d.m_hitHeight);
   bw.WriteInt(FID(WDTP), m_d.m_thickness);
   bw.WriteBool(FID(HTEV), m_d.m_fHitEvent);
   bw.WriteString(FID(MATR), m_d.m_szMaterial);
   bw.WriteBool(FID(TMON), m_d.m_tdr.m_fTimerEnabled);
   bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
   bw.WriteWideString(FID(NAME), (WCHAR *)m_wzName);
   bw.WriteString(FID(IMAG), m_d.m_szImage);
   bw.WriteFloat(FID(ELAS), m_d.m_elasticity);
   bw.WriteFloat(FID(ELFO), m_d.m_elasticityFalloff);
   bw.WriteFloat(FID(RFCT), m_d.m_friction);
   bw.WriteFloat(FID(RSCT), m_d.m_scatter);
   bw.WriteBool(FID(CLDRP), m_d.m_fCollidable);
   bw.WriteBool(FID(RVIS), m_d.m_fVisible);
   bw.WriteBool(FID(ESTR), m_d.m_staticRendering);
   bw.WriteBool(FID(ESIE), m_d.m_showInEditor);
   bw.WriteFloat(FID(ROTX), m_d.m_rotX);
   bw.WriteFloat(FID(ROTY), m_d.m_rotY);
   bw.WriteFloat(FID(ROTZ), m_d.m_rotZ);
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

HRESULT Rubber::InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   SetDefaults(false);
   m_d.m_hitHeight = -1.0f;
   BiffReader br(pstm, this, pid, version, hcrypthash, hcryptkey);

   m_ptable = ptable;

   br.Load();
   return S_OK;
}

BOOL Rubber::LoadToken(int id, BiffReader *pbr)
{
   if (id == FID(PIID))
   {
      pbr->GetInt((int *)pbr->m_pdata);
   }
   else if (id == FID(HTTP))
   {
      pbr->GetFloat(&m_d.m_height);
   }
   else if (id == FID(HTHI))
   {
      pbr->GetFloat(&m_d.m_hitHeight);
   }
   else if (id == FID(WDTP))
   {
      pbr->GetInt(&m_d.m_thickness);
   }
   else if (id == FID(HTEV))
   {
      pbr->GetBool(&m_d.m_fHitEvent);
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
   else if (id == FID(IMAG))
   {
      pbr->GetString(m_d.m_szImage);
   }
   else if (id == FID(NAME))
   {
      pbr->GetWideString((WCHAR *)m_wzName);
   }
   else if (id == FID(ELAS))
   {
      pbr->GetFloat(&m_d.m_elasticity);
   }
   else if (id == FID(ELFO))
   {
      pbr->GetFloat(&m_d.m_elasticityFalloff);
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
   else if (id == FID(ESTR))
   {
      pbr->GetBool(&m_d.m_staticRendering);
   }
   else if (id == FID(ESIE))
   {
      pbr->GetBool(&m_d.m_showInEditor);
   }
   else if (id == FID(ROTX))
   {
      pbr->GetFloat(&m_d.m_rotX);
   }
   else if (id == FID(ROTY))
   {
      pbr->GetFloat(&m_d.m_rotY);
   }
   else if (id == FID(ROTZ))
   {
      pbr->GetFloat(&m_d.m_rotZ);
   }
   else if ( id == FID( MAPH ) )
   {
       pbr->GetString( m_d.m_szPhysicsMaterial );
   }
   else if ( id == FID( OVPH ) )
   {
       pbr->GetBool( &m_d.m_fOverwritePhysics);
   }
   else
   {
      LoadPointToken(id, pbr, pbr->m_version);
      ISelect::LoadToken(id, pbr);
   }
   return fTrue;
}

HRESULT Rubber::InitPostLoad()
{
   if (m_d.m_hitHeight == -1.0f)
   {
      m_d.m_hitHeight = m_d.m_height;
   }
   return S_OK;
}

void Rubber::DoCommand(int icmd, int x, int y)
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
      const Vertex2D v = m_ptable->TransformPoint(x, y);

      std::vector<RenderVertex> vvertex;
      GetCentralCurve(vvertex);

      Vertex2D vOut;
      int iSeg = -1;
      ClosestPointOnPolygon(vvertex, v, &vOut, &iSeg, true);

      // Go through vertices (including iSeg itself) counting control points until iSeg
      int icp = 0;
      for (int i = 0; i < (iSeg + 1); i++)
         if (vvertex[i].fControlPoint)
            icp++;

      // ClosestPointOnPolygon() couldn't find a point -> don't try to add a new point 
      // because that would lead to strange behavior
      if (iSeg == -1)
         return;

      //if (icp == 0) // need to add point after the last point
      //icp = m_vdpoint.Size();
      STARTUNDO

      CComObject<DragPoint> *pdp;
      CComObject<DragPoint>::CreateInstance(&pdp);
      if (pdp)
      {
         pdp->AddRef();
         pdp->Init(this, vOut.x, vOut.y);
         pdp->m_fSmooth = true; // Ramps are usually always smooth
         m_vdpoint.InsertElementAt(pdp, icp); // push the second point forward, and replace it with this one.  Should work when index2 wraps.
      }

      SetDirtyDraw();

      STOPUNDO
   }
   break;
   }
}

void Rubber::FlipY(Vertex2D * const pvCenter)
{
   IHaveDragPoints::FlipPointY(pvCenter);
}

void Rubber::FlipX(Vertex2D * const pvCenter)
{
   IHaveDragPoints::FlipPointX(pvCenter);
}

void Rubber::Rotate(float ang, Vertex2D *pvCenter, const bool useElementCenter)
{
   IHaveDragPoints::RotatePoints(ang, pvCenter, useElementCenter);
}

void Rubber::Scale(float scalex, float scaley, Vertex2D *pvCenter, const bool useElementsCenter)
{
   IHaveDragPoints::ScalePoints(scalex, scaley, pvCenter, useElementsCenter);
}

void Rubber::Translate(Vertex2D *pvOffset)
{
   IHaveDragPoints::TranslatePoints(pvOffset);
}

STDMETHODIMP Rubber::InterfaceSupportsErrorInfo(REFIID riid)
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


STDMETHODIMP Rubber::get_Height(float *pVal)
{
   *pVal = m_d.m_height;
   UpdateUnitsInfo();
   return S_OK;
}

STDMETHODIMP Rubber::put_Height(float newVal)
{
   if (m_d.m_height != newVal)
   {
      STARTUNDO

      m_d.m_height = newVal;
      dynamicVertexBufferRegenerate = true;
      UpdateUnitsInfo();

      STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Rubber::get_HitHeight(float *pVal)
{
   *pVal = m_d.m_hitHeight;

   return S_OK;
}

STDMETHODIMP Rubber::put_HitHeight(float newVal)
{
   if (m_d.m_hitHeight != newVal)
   {
      STARTUNDO

      m_d.m_hitHeight = newVal;

      STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Rubber::get_Thickness(int *pVal)
{
   *pVal = m_d.m_thickness;
   UpdateUnitsInfo();

   return S_OK;
}

STDMETHODIMP Rubber::put_Thickness(int newVal)
{
   if (m_d.m_thickness != newVal)
   {
      STARTUNDO

      m_d.m_thickness = newVal;
      dynamicVertexBufferRegenerate = true;
      UpdateUnitsInfo();

      STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Rubber::get_Material(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_d.m_szMaterial, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Rubber::put_Material(BSTR newVal)
{
   STARTUNDO

   WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szMaterial, 32, NULL, NULL);

   STOPUNDO

   return S_OK;
}

void Rubber::GetDialogPanes(Vector<PropertyPane> *pvproppane)
{
   PropertyPane *pproppane;

   pproppane = new PropertyPane(IDD_PROP_NAME, NULL);
   pvproppane->AddElement(pproppane);

   m_propVisual = new PropertyPane(IDD_PROPRUBBER_VISUALS, IDS_VISUALS);
   pvproppane->AddElement(m_propVisual);

   m_propPosition = new PropertyPane(IDD_PROPRUBBER_POSITION, IDS_POSITION);
   pvproppane->AddElement(m_propPosition);

   m_propPhysics = new PropertyPane(IDD_PROPRUBBER_PHYSICS, IDS_PHYSICS);
   pvproppane->AddElement(m_propPhysics);

   pproppane = new PropertyPane(IDD_PROP_TIMER, IDS_MISC);
   pvproppane->AddElement(pproppane);
}

STDMETHODIMP Rubber::get_Image(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_d.m_szImage, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Rubber::put_Image(BSTR newVal)
{
   char m_szImage[MAXTOKEN];
   memset(m_szImage, 0, MAXTOKEN);

   WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_szImage, 32, NULL, NULL);
   const Texture * const tex = m_ptable->GetImage(m_szImage);
   if(tex && tex->IsHDR())
   {
       ShowError("Cannot use a HDR image (.exr/.hdr) here");
       return E_FAIL;
   }

   if (strcmp(m_szImage, m_d.m_szImage) != 0)
   {
      STARTUNDO

      strcpy_s(m_d.m_szImage, MAXTOKEN, m_szImage);
      dynamicVertexBufferRegenerate = true;

      STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Rubber::get_HasHitEvent(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fHitEvent);

   return S_OK;
}

STDMETHODIMP Rubber::put_HasHitEvent(VARIANT_BOOL newVal)
{
   STARTUNDO

   m_d.m_fHitEvent = VBTOF(newVal);

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Rubber::get_Elasticity(float *pVal)
{
   *pVal = m_d.m_elasticity;

   return S_OK;
}

STDMETHODIMP Rubber::put_Elasticity(float newVal)
{
   STARTUNDO
   m_d.m_elasticity = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP Rubber::get_ElasticityFalloff(float *pVal)
{
   *pVal = m_d.m_elasticityFalloff;

   return S_OK;
}

STDMETHODIMP Rubber::put_ElasticityFalloff(float newVal)
{
   STARTUNDO
   m_d.m_elasticityFalloff = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP Rubber::get_Friction(float *pVal)
{
   *pVal = m_d.m_friction;

   return S_OK;
}

STDMETHODIMP Rubber::put_Friction(float newVal)
{
   newVal = clamp(newVal, 0.f, 1.f);

   STARTUNDO

   m_d.m_friction = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Rubber::get_Scatter(float *pVal)
{
   *pVal = m_d.m_scatter;

   return S_OK;
}

STDMETHODIMP Rubber::put_Scatter(float newVal)
{
   STARTUNDO

   m_d.m_scatter = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Rubber::get_Collidable(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB((!g_pplayer) ? m_d.m_fCollidable : m_vhoCollidable[0]->m_fEnabled);

   return S_OK;
}

STDMETHODIMP Rubber::put_Collidable(VARIANT_BOOL newVal)
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

STDMETHODIMP Rubber::get_Visible(VARIANT_BOOL *pVal) //temporary value of object
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fVisible);

   return S_OK;
}

STDMETHODIMP Rubber::put_Visible(VARIANT_BOOL newVal)
{
   STARTUNDO
   if (g_pplayer && m_d.m_staticRendering)
      ShowError("Rubber is static! Visible property not supported!");
   m_d.m_fVisible = VBTOF(newVal);
   STOPUNDO

   return S_OK;
}

STDMETHODIMP Rubber::get_EnableStaticRendering(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_staticRendering);

   return S_OK;
}

STDMETHODIMP Rubber::put_EnableStaticRendering(VARIANT_BOOL newVal)
{
   STARTUNDO

   m_d.m_staticRendering = VBTOF(newVal);

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Rubber::get_EnableShowInEditor(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_showInEditor);

   return S_OK;
}

STDMETHODIMP Rubber::put_EnableShowInEditor(VARIANT_BOOL newVal)
{
   STARTUNDO

   m_d.m_showInEditor = VBTOF(newVal);

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Rubber::get_ReflectionEnabled(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fReflectionEnabled);

   return S_OK;
}

STDMETHODIMP Rubber::put_ReflectionEnabled(VARIANT_BOOL newVal)
{
   STARTUNDO

   m_d.m_fReflectionEnabled = VBTOF(newVal);

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Rubber::get_RotX(float *pVal)
{
   *pVal = m_d.m_rotX;

   return S_OK;
}

STDMETHODIMP Rubber::put_RotX(float newVal)
{
   if (m_d.m_rotX != newVal)
   {
      STARTUNDO

      m_d.m_rotX = newVal;
      dynamicVertexBufferRegenerate = true;

      STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Rubber::get_RotY(float *pVal)
{
   *pVal = m_d.m_rotY;

   return S_OK;
}

STDMETHODIMP Rubber::put_RotY(float newVal)
{
   if (m_d.m_rotY != newVal)
   {
      STARTUNDO

      m_d.m_rotY = newVal;
      dynamicVertexBufferRegenerate = true;

      STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Rubber::get_RotZ(float *pVal)
{
   *pVal = m_d.m_rotZ;

   return S_OK;
}

STDMETHODIMP Rubber::put_RotZ(float newVal)
{
   if (m_d.m_rotZ != newVal)
   {
      STARTUNDO

      m_d.m_rotZ = newVal;
      dynamicVertexBufferRegenerate = true;

      STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Rubber::get_PhysicsMaterial( BSTR *pVal )
{
    WCHAR wz[512];

    MultiByteToWideChar( CP_ACP, 0, m_d.m_szPhysicsMaterial, -1, wz, 32 );
    *pVal = SysAllocString( wz );

    return S_OK;
}

STDMETHODIMP Rubber::put_PhysicsMaterial( BSTR newVal )
{
    STARTUNDO

    WideCharToMultiByte( CP_ACP, 0, newVal, -1, m_d.m_szPhysicsMaterial, 32, NULL, NULL );

    STOPUNDO

    return S_OK;
}

STDMETHODIMP Rubber::get_OverwritePhysics( VARIANT_BOOL *pVal )
{
    *pVal = (VARIANT_BOOL)FTOVB( m_d.m_fOverwritePhysics );

    return S_OK;
}

STDMETHODIMP Rubber::put_OverwritePhysics( VARIANT_BOOL newVal )
{
    STARTUNDO

    m_d.m_fOverwritePhysics = VBTOF( newVal );

    STOPUNDO

    return S_OK;
}


void Rubber::RenderObject(RenderDevice * const pd3dDevice)
{
   TRACE_FUNCTION();

   // don't render if invisible or not a transparent ramp
   if (!m_d.m_fVisible)
      return;

   if (m_d.m_thickness == 0)
   {
      dynamicVertexBufferRegenerate = false;
      return;
   }

   if (dynamicVertexBufferRegenerate)
       UpdateRubber(pd3dDevice, true, m_d.m_height);

   pd3dDevice->SetTextureAddressMode(0, RenderDevice::TEX_CLAMP);

   const Material * const mat = m_ptable->GetMaterial(m_d.m_szMaterial);
   pd3dDevice->basicShader->SetMaterial(mat);

   pd3dDevice->SetRenderState(RenderDevice::DEPTHBIAS, 0);
   pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, TRUE);
   pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_CCW);

   Texture * const pin = m_ptable->GetImage(m_d.m_szImage);
   if (pin)
   {
      pd3dDevice->basicShader->SetTechnique(mat->m_bIsMetal ? "basic_with_texture_isMetal" : "basic_with_texture_isNotMetal");
      pd3dDevice->basicShader->SetTexture("Texture0", pin);
      pd3dDevice->basicShader->SetAlphaTestValue(pin->m_alphaTestValue * (float)(1.0 / 255.0));
   }
   else
      pd3dDevice->basicShader->SetTechnique(mat->m_bIsMetal ? "basic_without_texture_isMetal" : "basic_without_texture_isNotMetal");

   pd3dDevice->basicShader->Begin(0);
   pd3dDevice->DrawIndexedPrimitiveVB(D3DPT_TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, dynamicVertexBuffer, 0, m_numVertices, dynamicIndexBuffer, 0, m_numIndices);
   pd3dDevice->basicShader->End();
}

// Always called each frame to render over everything else (along with primitives)
// Same code as RenderStatic (with the exception of the alpha tests).
// Also has less drawing calls by bundling seperate calls.
void Rubber::PostRenderStatic(RenderDevice* pd3dDevice)
{
   if (!m_d.m_staticRendering)
   {
      if (m_ptable->m_fReflectionEnabled && !m_d.m_fReflectionEnabled)
         return;

      RenderObject(pd3dDevice);
   }
}

void Rubber::ExportMesh(FILE *f)
{
   char name[MAX_PATH];
   if (m_d.m_fVisible)
   {

      WideCharToMultiByte(CP_ACP, 0, m_wzName, -1, name, MAX_PATH, NULL, NULL);
      GenerateMesh();
      UpdateRubber(NULL, false, m_d.m_height);

      WaveFrontObj_WriteObjectName(f, name);
      WaveFrontObj_WriteVertexInfo(f, m_vertices.data(), m_numVertices);
      const Material * const mat = m_ptable->GetMaterial(m_d.m_szMaterial);
      WaveFrontObj_WriteMaterial(m_d.m_szMaterial, NULL, mat);
      WaveFrontObj_UseTexture(f, m_d.m_szMaterial);
      WaveFrontObj_WriteFaceInfo(f, ringIndices);
      WaveFrontObj_UpdateFaceOffset(m_numVertices);
   }
}

void Rubber::GenerateMesh(const int _accuracy, const bool createHitShape)
{
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
   if (_accuracy != -1)
      accuracy = _accuracy;

   Vertex2D * middlePoints = 0;
   int splinePoints;
   const Vertex2D * const rgvLocal = GetSplineVertex(splinePoints, NULL, &middlePoints);
   const int numRings = splinePoints - 1;
   const int numSegments = accuracy;
   m_numVertices = numRings*numSegments;
   m_numIndices = 6 * m_numVertices;//m_numVertices*2+2;

   m_vertices.resize(m_numVertices);
   ringIndices.resize(m_numIndices);
   const float height = m_d.m_hitHeight + m_ptable->m_tableheight;

   Vertex3Ds prevB;
   const float invNR = 1.0f / (float)numRings;
   const float invNS = 1.0f / (float)numSegments;
   for (int i = 0, index = 0; i < numRings; i++)
   {
      const int i2 = (i == numRings - 1) ? 0 : i + 1;

      const Vertex3Ds tangent(middlePoints[i2].x - middlePoints[i].x, middlePoints[i2].y - middlePoints[i].y, 0.0f);

	  Vertex3Ds binorm;
	  Vertex3Ds normal;
	  if (i == 0)
      {
         const Vertex3Ds up(middlePoints[i2].x + middlePoints[i].x, middlePoints[i2].y + middlePoints[i].y, height*2.f);
         normal = Vertex3Ds(tangent.y * up.z, - tangent.x * up.z, tangent.x * up.y - tangent.y * up.x); // = CrossProduct(tangent, up)
         binorm = Vertex3Ds(tangent.y * normal.z, - tangent.x * normal.z, tangent.x * normal.y - tangent.y * normal.x); // = CrossProduct(tangent, normal)
      }
      else
      {
         normal = CrossProduct(prevB, tangent);
         binorm = CrossProduct(tangent, normal);
      }
      binorm.Normalize();
      normal.Normalize();
      prevB = binorm;
      const float u = (float)i * invNR;
      for (int j = 0; j < numSegments; j++, index++)
      {
         const float v = ((float)j + u) * invNS;
         Vertex3Ds tmp = GetRotatedAxis((float)j*(360.0f * invNS), tangent, normal) * ((float)m_d.m_thickness*0.5f);
         m_vertices[index].x = middlePoints[i].x + tmp.x;
         m_vertices[index].y = middlePoints[i].y + tmp.y;
         if (createHitShape && (j==0 || j==3)) //!! hack, adapt if changing detail level for hitshape
         {
             // for a hit shape create a more rectangle mesh and not a smooth one
             tmp.z *= 0.6f;
         }
         m_vertices[index].z = height + tmp.z;
         //texel
         m_vertices[index].tu = u;
         m_vertices[index].tv = v;
      }
   }

   // calculate faces
   for (int i = 0; i < numRings; i++)
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
         ringIndices[(i*numSegments + j) * 6    ] = quad[0];
         ringIndices[(i*numSegments + j) * 6 + 1] = quad[1];
         ringIndices[(i*numSegments + j) * 6 + 2] = quad[2];
         ringIndices[(i*numSegments + j) * 6 + 3] = quad[3];
         ringIndices[(i*numSegments + j) * 6 + 4] = quad[2];
         ringIndices[(i*numSegments + j) * 6 + 5] = quad[1];
      }
   }

   ComputeNormals(m_vertices.data(), m_numVertices, ringIndices.data(), m_numIndices);

   float maxx = FLT_MIN;
   float minx = FLT_MAX;
   float maxy = FLT_MIN;
   float miny = FLT_MAX;
   float maxz = FLT_MIN;
   float minz = FLT_MAX;
   for (int i = 0; i < m_numVertices; i++)
   {
      if (maxx < m_vertices[i].x) maxx = m_vertices[i].x;
      if (minx > m_vertices[i].x) minx = m_vertices[i].x;
      if (maxy < m_vertices[i].y) maxy = m_vertices[i].y;
      if (miny > m_vertices[i].y) miny = m_vertices[i].y;
      if (maxz < m_vertices[i].z) maxz = m_vertices[i].z;
      if (minz > m_vertices[i].z) minz = m_vertices[i].z;
   }
   middlePoint.x = (maxx + minx)*0.5f;
   middlePoint.y = (maxy + miny)*0.5f;
   middlePoint.z = (maxz + minz)*0.5f;

   // not necessary to reorder
   /*WORD* tmp = reorderForsyth(ringIndices.data(), ringIndices.size() / 3, m_numVertices);
   if (tmp != NULL)
   {
   memcpy(ringIndices.data(), tmp, ringIndices.size()*sizeof(WORD));
   delete[] tmp;
   }*/

   delete[] rgvLocal;
   delete[] middlePoints;
}

void Rubber::GenerateVertexBuffer(RenderDevice* pd3dDevice)
{
   dynamicVertexBufferRegenerate = true;

   GenerateMesh();

   if (dynamicVertexBuffer)
      dynamicVertexBuffer->release();
   pd3dDevice->CreateVertexBuffer(m_numVertices, m_d.m_staticRendering ? 0 : USAGE_DYNAMIC, MY_D3DFVF_NOTEX2_VERTEX, &dynamicVertexBuffer);

   // Draw the floor of the ramp.
   Vertex3D_NoTex2 *buf;
   dynamicVertexBuffer->lock(0, 0, (void**)&buf, m_d.m_staticRendering ? VertexBuffer::WRITEONLY : VertexBuffer::DISCARDCONTENTS);
   memcpy(buf, m_vertices.data(), sizeof(Vertex3D_NoTex2)*m_numVertices);
   dynamicVertexBuffer->unlock();

   if (dynamicIndexBuffer)
      dynamicIndexBuffer->release();

   dynamicIndexBuffer = pd3dDevice->CreateAndFillIndexBuffer(ringIndices);
}

void Rubber::UpdateRubber(RenderDevice * const pd3dDevice, const bool updateVB, const float height)
{
   Matrix3D fullMatrix,tempMat;
   fullMatrix.RotateZMatrix(ANGTORAD(m_d.m_rotZ));
   tempMat.RotateYMatrix(ANGTORAD(m_d.m_rotY));
   tempMat.Multiply(fullMatrix, fullMatrix);
   tempMat.RotateXMatrix(ANGTORAD(m_d.m_rotX));
   tempMat.Multiply(fullMatrix, fullMatrix);

   Matrix3D vertMatrix;
   tempMat.SetTranslation(-middlePoint.x, -middlePoint.y, -middlePoint.z);
   fullMatrix.Multiply(tempMat, vertMatrix);
   tempMat.SetScaling(1.f, 1.f, m_ptable->m_BG_scalez[m_ptable->m_BG_current_set]);
   tempMat.Multiply(vertMatrix, vertMatrix);
   if ( height==m_d.m_hitHeight )   // do not z-scale the hit mesh
      tempMat.SetTranslation(middlePoint.x, middlePoint.y, height + m_ptable->m_tableheight);
   else
      tempMat.SetTranslation(middlePoint.x, middlePoint.y, height*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set] + m_ptable->m_tableheight);
   tempMat.Multiply(vertMatrix, vertMatrix);

   Vertex3D_NoTex2 *buf;
   if (updateVB)
      dynamicVertexBuffer->lock(0, 0, (void**)&buf, m_d.m_staticRendering ? VertexBuffer::WRITEONLY : VertexBuffer::DISCARDCONTENTS);
   else
      buf = m_vertices.data();

   for (int i = 0; i < m_numVertices; i++)
   {
      Vertex3Ds vert(m_vertices[i].x, m_vertices[i].y, m_vertices[i].z);
      vert = vertMatrix.MultiplyVector(vert);
      buf[i].x = vert.x;
      buf[i].y = vert.y;
      buf[i].z = vert.z;

      vert = Vertex3Ds(m_vertices[i].nx, m_vertices[i].ny, m_vertices[i].nz);
      vert = fullMatrix.MultiplyVectorNoTranslate(vert);
      buf[i].nx = vert.x;
      buf[i].ny = vert.y;
      buf[i].nz = vert.z;
      buf[i].tu = m_vertices[i].tu;
      buf[i].tv = m_vertices[i].tv;
   }

   if (updateVB)
   {
      dynamicVertexBuffer->unlock();
      dynamicVertexBufferRegenerate = false;
   }
}

void Rubber::UpdatePropertyPanes()
{
   if (m_propVisual == NULL || m_propPosition == NULL || m_propPhysics == NULL)
      return;

   if (!m_d.m_fCollidable)
   {
      EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, 110), FALSE);
      EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, 114), FALSE);
      EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, 115), FALSE);
      EnableWindow( GetDlgItem( m_propPhysics->dialogHwnd, 116 ), FALSE );
      EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd, 120), FALSE);
      EnableWindow( GetDlgItem( m_propPhysics->dialogHwnd, IDC_MATERIAL_COMBO4 ), FALSE );
      EnableWindow( GetDlgItem( m_propPhysics->dialogHwnd, IDC_OVERWRITE_MATERIAL_SETTINGS ), FALSE );
   }
   else
   {
      EnableWindow( GetDlgItem( m_propPhysics->dialogHwnd, IDC_OVERWRITE_MATERIAL_SETTINGS ), TRUE );
      EnableWindow( GetDlgItem( m_propPhysics->dialogHwnd, 116 ), TRUE );
      if( m_d.m_fOverwritePhysics )
      {
          EnableWindow( GetDlgItem( m_propPhysics->dialogHwnd, IDC_MATERIAL_COMBO4 ), FALSE );
          EnableWindow( GetDlgItem( m_propPhysics->dialogHwnd, 110 ), TRUE );
          EnableWindow( GetDlgItem( m_propPhysics->dialogHwnd, 114 ), TRUE );
          EnableWindow( GetDlgItem( m_propPhysics->dialogHwnd, 115 ), TRUE );
          EnableWindow( GetDlgItem( m_propPhysics->dialogHwnd, 120 ), TRUE );
      }
      else
      {
          EnableWindow( GetDlgItem( m_propPhysics->dialogHwnd, IDC_MATERIAL_COMBO4 ), TRUE );
          EnableWindow( GetDlgItem( m_propPhysics->dialogHwnd, 110 ), FALSE );
          EnableWindow( GetDlgItem( m_propPhysics->dialogHwnd, 114 ), FALSE );
          EnableWindow( GetDlgItem( m_propPhysics->dialogHwnd, 115 ), FALSE );
          EnableWindow( GetDlgItem( m_propPhysics->dialogHwnd, 120 ), FALSE );
      }
   }
}

void Rubber::SetDefaultPhysics(bool fromMouseClick)
{
   static const char strKeyName[] = "DefaultProps\\Rubber";

   m_d.m_elasticity = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "Elasticity", 0.8f) : 0.8f;
   m_d.m_elasticityFalloff = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "ElasticityFalloff", 0.3f) : 0.3f;
   m_d.m_friction = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "Friction", 0.6f) : 0.6f;
   m_d.m_scatter = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "Scatter", 5) : 5;
   m_d.m_hitHeight = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName, "HitHeight", 25.0f) : 25.0f;
}
