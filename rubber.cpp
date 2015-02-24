#include "StdAfx.h"

Rubber::Rubber()
{
   m_menuid = IDR_SURFACEMENU;
   m_d.m_fCollidable = true;
   m_d.m_fVisible = true;
   m_d.m_fHitEvent = false;
   staticVertexBuffer = 0;
   dynamicVertexBuffer = 0;
   dynamicIndexBuffer = 0;
   dynamicVertexBufferRegenerate = true;
   m_d.m_depthBias = 0.0f;
   m_propPhysics = NULL;
   m_propPosition = NULL;
   m_propVisual = NULL;
}

Rubber::~Rubber()
{
	if(staticVertexBuffer)
		staticVertexBuffer->release();

	if(dynamicVertexBuffer)
		dynamicVertexBuffer->release();

    if (dynamicIndexBuffer)
        dynamicIndexBuffer->release();
}

HRESULT Rubber::Init(PinTable *ptable, float x, float y, bool fromMouseClick)
{
   m_ptable = ptable;
   m_d.m_fVisible = true;

   //float length = 0.5f * GetRegStringAsFloatWithDefault("DefaultProps\\Rubber", "Length", 400.0f);

   CComObject<DragPoint> *pdp;
   for( int i=8;i>0;i-- )
   {
       const float angle = (float)(M_PI*2.0/8.0)*(float)i;
       const float xx = x + sinf(angle)*50.0f;
       const float yy = y - cosf(angle)*50.0f;
       CComObject<DragPoint>::CreateInstance(&pdp);
       if (pdp)
       {
           pdp->AddRef();
           pdp->Init(this, xx, yy);
           pdp->m_fSmooth = TRUE;
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

   HRESULT hr;

   m_d.m_height = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName,"Height", 25.0f) : 25.0f;
   m_d.m_thickness = fromMouseClick ? GetRegIntWithDefault(strKeyName,"Thickness", 8) : 8;

   m_d.m_tdr.m_fTimerEnabled = fromMouseClick ? GetRegBoolWithDefault(strKeyName,"TimerEnabled", false) : false;
   m_d.m_tdr.m_TimerInterval = fromMouseClick ? GetRegIntWithDefault(strKeyName,"TimerInterval", 100) : 100;

   hr = GetRegString(strKeyName,"Image", m_d.m_szImage, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szImage[0] = 0;

   m_d.m_fHitEvent = fromMouseClick ? GetRegBoolWithDefault(strKeyName, "HitEvent", true) : false;

   m_d.m_elasticity = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName,"Elasticity", 0.9f) : 0.9f;
   m_d.m_elasticityFalloff = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName,"ElasticityFalloff", 0.4f) : 0.4f;
   m_d.m_friction = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName,"Friction", 0.3f) : 0.3f;
   m_d.m_scatter = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName,"Scatter", 0) : 0;

   m_d.m_fVisible = fromMouseClick ? GetRegBoolWithDefault(strKeyName,"Visible", true) : true;
   m_d.m_fCollidable = fromMouseClick ? GetRegBoolWithDefault(strKeyName,"Collidable", true) : true;

   m_d.m_staticRendering = fromMouseClick ? GetRegBoolWithDefault(strKeyName,"EnableStaticRendering", true) : true;

   m_d.m_rotX = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName,"RotX", 0.0f) : 0.0f;
   m_d.m_rotY = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName,"RotY", 0.0f) : 0.0f;
}

void Rubber::WriteRegDefaults()
{
   static const char strKeyName[] = "DefaultProps\\Rubber";

   SetRegValueFloat(strKeyName,"Height", m_d.m_height);
   SetRegValueInt(strKeyName,"Thickness", m_d.m_thickness);
   SetRegValueBool(strKeyName, "HitEvent", m_d.m_fHitEvent);
   SetRegValue(strKeyName, "TimerEnabled", REG_DWORD, &m_d.m_tdr.m_fTimerEnabled, 4);
   SetRegValue(strKeyName,"TimerInterval",REG_DWORD,&m_d.m_tdr.m_TimerInterval,4);
   SetRegValue(strKeyName,"Image", REG_SZ, &m_d.m_szImage, lstrlen(m_d.m_szImage));
   SetRegValueFloat(strKeyName,"Elasticity", m_d.m_elasticity);
   SetRegValueFloat(strKeyName,"ElasticityFalloff", m_d.m_elasticityFalloff);
   SetRegValueFloat(strKeyName,"Friction", m_d.m_friction);
   SetRegValueFloat(strKeyName,"Scatter", m_d.m_scatter);
   SetRegValueBool(strKeyName,"Collidable",m_d.m_fCollidable);
   SetRegValueBool(strKeyName,"Visible",m_d.m_fVisible);
   SetRegValueBool(strKeyName,"EnableStaticRendering",m_d.m_staticRendering);
   SetRegValueFloat(strKeyName,"RotX", m_d.m_rotX);
   SetRegValueFloat(strKeyName,"RotY", m_d.m_rotY);
}

void Rubber::GetPointDialogPanes(Vector<PropertyPane> *pvproppane)
{
   PropertyPane *pproppane;

   pproppane = new PropertyPane(IDD_PROPPOINT_VISUALS, IDS_VISUALS);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPPOINT_POSITION, IDS_POSITION);
   pvproppane->AddElement(pproppane);
}


void Rubber::PreRender(Sur * const psur)
{
   //make 1 wire ramps look unique in editor - uses ramp color
   if (m_ptable->RenderSolid())
       psur->SetFillColor( RGB(192,192,192) );
   else
       psur->SetFillColor(-1);
   psur->SetBorderColor(-1,false,0);
   psur->SetObject(this);

   int cvertex;

   Vertex2D * const rgvLocal = GetSplineVertex(cvertex, NULL, NULL);
   psur->Polygon(rgvLocal, (cvertex)*2);
   delete [] rgvLocal;
}

void Rubber::Render(Sur * const psur)
{
   psur->SetFillColor(-1);
   psur->SetBorderColor(RGB(0,0,0),false,0);
   psur->SetLineColor(RGB(0,0,0),false,0);
   psur->SetObject(this);
   psur->SetObject(NULL); // NULL so this won't be hit-tested

   int cvertex;
   bool *pfCross;
   const Vertex2D *rgvLocal = GetSplineVertex(cvertex, &pfCross, NULL);
   psur->Polygon(rgvLocal, (cvertex)*2);
   for (int i=0;i<cvertex;i++)
       if (pfCross[i])
           psur->Line(rgvLocal[i].x, rgvLocal[i].y, rgvLocal[cvertex*2 - i - 1].x, rgvLocal[cvertex*2 - i - 1].y);
   
   delete [] rgvLocal;
   delete [] pfCross;

   bool fDrawDragpoints = ( (m_selectstate != eNotSelected) || (g_pvp->m_fAlwaysDrawDragPoints) );
   // if the item is selected then draw the dragpoints (or if we are always to draw dragpoints)
   if (!fDrawDragpoints)
   {
      // if any of the dragpoints of this object are selected then draw all the dragpoints
      for (int i=0;i<m_vdpoint.Size();i++)
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
      for (int i=0;i<m_vdpoint.Size();i++)
      {
         CComObject<DragPoint> * const pdp = m_vdpoint.ElementAt(i);
         psur->SetFillColor(-1);
         psur->SetBorderColor(RGB(255,0,0),false,0);
         psur->SetObject(pdp);

         if (pdp->m_fDragging)
            psur->SetBorderColor(RGB(0,255,0),false,0);

         psur->Ellipse2(pdp->m_v.x, pdp->m_v.y, 8);
      }
   }
}

void Rubber::RenderOutline(Sur * const psur)
{
   psur->SetFillColor(-1);
   psur->SetBorderColor(RGB(0,0,0),false,0);
   psur->SetLineColor(RGB(0,0,0),false,0);
   psur->SetObject(this);
   psur->SetObject(NULL); // NULL so this won't be hit-tested

   int cvertex;
   bool *pfCross;
   const Vertex2D * const rgvLocal = GetSplineVertex(cvertex, &pfCross, NULL);

   psur->Polygon(rgvLocal, (cvertex)*2);
   for (int i=0;i<cvertex;i++)
      if (pfCross[i])
         psur->Line(rgvLocal[i].x, rgvLocal[i].y, rgvLocal[cvertex*2 - i - 1].x, rgvLocal[cvertex*2 - i - 1].y);

   delete [] rgvLocal;
   delete [] pfCross;
}

void Rubber::RenderBlueprint(Sur *psur)
{
   RenderOutline(psur);
}

void Rubber::GetBoundingVertices(Vector<Vertex3Ds> * const pvvertex3D)
{
   int cvertex;
   const Vertex2D * const rgvLocal = GetSplineVertex(cvertex, NULL, NULL );

   for (int i=0;i<cvertex;i++)
   {
      {
         Vertex3Ds * const pv = new Vertex3Ds();
         pv->x = rgvLocal[i].x;
         pv->y = rgvLocal[i].y;
         pv->z = m_d.m_height+50.0f; // leave room for ball
         pvvertex3D->AddElement(pv);
      }

      Vertex3Ds * const pv = new Vertex3Ds();
      pv->x = rgvLocal[cvertex*2-i-1].x;
      pv->y = rgvLocal[cvertex*2-i-1].y;
      pv->z = m_d.m_height+50.0f; // leave room for ball
      pvvertex3D->AddElement(pv);
   }

   delete [] rgvLocal;
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
Vertex2D *Rubber::GetSplineVertex(int &pcvertex, bool ** const ppfCross, Vertex2D **pMiddlePoints)
{
   std::vector<RenderVertex> vvertex;
   GetCentralCurve(vvertex);
   // vvertex are the 2D vertices forming the central curve of the rubber as seen from above

   const int cvertex = vvertex.size();
   Vertex2D * const rgvLocal = new Vertex2D[(cvertex+1) * 2];

   if( pMiddlePoints )
   {
       *pMiddlePoints = new Vertex2D[cvertex+1];
   }

   for (int i=0; i<cvertex; i++)
   {
      const RenderVertex & vprev = vvertex[(i>0) ? i-1 : i];
      const RenderVertex & vnext = vvertex[(i < (cvertex-1)) ? i+1 : 0];
      const RenderVertex & vmiddle = vvertex[i];

      Vertex2D vnormal;
      {
         // Get normal at this point
         // Notice that these values equal the ones in the line
         // equation and could probably be substituted by them.
         Vertex2D v1normal(vprev.y - vmiddle.y, vmiddle.x - vprev.x);   // vector vmiddle-vprev rotated RIGHT
         Vertex2D v2normal(vmiddle.y - vnext.y, vnext.x - vmiddle.x);   // vector vnext-vmiddle rotated RIGHT

         if (i == (cvertex-1))
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
               v1normal.Normalize();
               vnormal = v1normal;
            }
            else
            {
               v1normal.Normalize();
               v2normal.Normalize();

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
               const float inv_det = (det != 0.0f) ? 1.0f/det : 0.0f;

               const float intersectx = (B*F-E*C)*inv_det;
               const float intersecty = (C*D-A*F)*inv_det;

               vnormal.x = vmiddle.x - intersectx;
               vnormal.y = vmiddle.y - intersecty;
            }
         }
      }

      const float widthcur = (float)m_d.m_thickness;

      if( pMiddlePoints )
         (*pMiddlePoints)[i] = vmiddle;

      rgvLocal[i] = vmiddle + (widthcur*0.5f) * vnormal;
      rgvLocal[(cvertex+1)*2 - i - 1] = vmiddle - (widthcur*0.5f) * vnormal;
      if ( i==0 )
      {
          rgvLocal[cvertex] = rgvLocal[0];
          rgvLocal[(cvertex+1)*2 - cvertex - 1] = rgvLocal[(cvertex+1)*2 - 1];
      }
   }

   if (ppfCross)
   {
      *ppfCross = new bool[cvertex+1];
      for (int i=0;i<cvertex;i++)
         (*ppfCross)[i] = vvertex[i].fControlPoint;
      (*ppfCross)[cvertex] = vvertex[0].fControlPoint;
   }
   if (pMiddlePoints)
   {
       (*pMiddlePoints)[cvertex] = (*pMiddlePoints)[0] ;
   }

   pcvertex = cvertex+1;
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

    const int cvertex = vvertex.size();

    int iSeg;
    Vertex2D vOut;
    ClosestPointOnPolygon(vvertex, Vertex2D(x,y), &vOut, &iSeg, false);

    // Go through vertices (including iSeg itself) counting control points until iSeg
    float totallength = 0.f;
    float startlength = 0.f;
    float zheight = 0.f;

    if (iSeg == -1)
    {
        return 0; // Object is not on ramp path
    }

    for (int i2=1;i2<cvertex;i2++)
    {
        const float dx = vvertex[i2].x - vvertex[i2-1].x;
        const float dy = vvertex[i2].y - vvertex[i2-1].y;
        const float len = sqrtf(dx*dx + dy*dy);
        if (i2 <= iSeg)
        {
            startlength += len;
        }
        totallength += len;
    }

    {
        const float dx = vOut.x - vvertex[iSeg].x;
        const float dy = vOut.y - vvertex[iSeg].y;
        const float len = sqrtf(dx*dx + dy*dy);
        startlength += len; // Add the distance the object is between the two closest polyline segments.  Matters mostly for straight edges.

        zheight = (startlength/totallength) * (m_d.m_height );
    }

    return zheight;
}

void Rubber::GetTimers(Vector<HitTimer> * const pvht)
{
   IEditable::BeginPlay();

   HitTimer * const pht = new HitTimer();
   pht->m_interval = m_d.m_tdr.m_TimerInterval;
   pht->m_nextfire = pht->m_interval;
   pht->m_pfe = (IFireEvents *)this;

   m_phittimer = pht;

   if (m_d.m_tdr.m_fTimerEnabled)
      pvht->AddElement(pht);
}

void Rubber::GetHitShapes(Vector<HitObject> * const pvho)
{
   int cvertex;
   Vertex2D * const rgvLocal = GetSplineVertex(cvertex, NULL, NULL);

   const float height = m_d.m_height + m_ptable->m_tableheight;
   const float topheight = height + (float)m_d.m_thickness;

   for (int i=0; i<(cvertex-1); i++)
   {
       const Vertex2D & v1 = rgvLocal[i];
       const Vertex2D & v2 = rgvLocal[i+1];

       AddLine(pvho, v1, v2, height, topheight);
       AddLine(pvho, v2, v1, height, topheight);

       AddJoint2D(pvho, v1, height, topheight);
   }

   for (int i=0; i<(cvertex-1); i++)
   {
       const Vertex2D & v1 = rgvLocal[cvertex + i];
       const Vertex2D & v2 = rgvLocal[cvertex + i + 1];

       AddLine(pvho, v1, v2, height, topheight);
       AddLine(pvho, v2, v1, height, topheight);

       AddJoint2D(pvho, v1, height, topheight);
   }

   // Add hit triangles for the top of the rubber object.
   {
      const Vertex2D *pv1,*pv2,*pv3,*pv4;

      for (int i=0; i<(cvertex-1); i++)
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
         pv2 = &rgvLocal[cvertex*2 - i - 1];    // i-th left
         pv3 = &rgvLocal[cvertex*2 - i - 2];    // (i+1)-th left
         pv4 = &rgvLocal[i+1];                  // (i+1)-th right

         {
            const Vertex3Ds rgv3D[3] = {
				Vertex3Ds(pv2->x, pv2->y, topheight),
				Vertex3Ds(pv1->x, pv1->y, topheight),
				Vertex3Ds(pv3->x, pv3->y, topheight)};

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
            }
         }

         const Vertex3Ds rgv3D[3] = {
			Vertex3Ds(pv3->x, pv3->y, topheight),
			Vertex3Ds(pv1->x, pv1->y, topheight),
			Vertex3Ds(pv4->x, pv4->y, topheight)};

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
         }
      }
   }

   // Add hit triangles for the bottom of the rubber object.

   for (int i=0; i<(cvertex-1); i++)
   {
      // see sketch above
      const Vertex2D * const pv1 = &rgvLocal[i];
      const Vertex2D * const pv2 = &rgvLocal[cvertex*2 - i - 1];
      const Vertex2D * const pv3 = &rgvLocal[cvertex*2 - i - 2];
      const Vertex2D * const pv4 = &rgvLocal[i+1];

      {
         // left ramp triangle, order CW
         const Vertex3Ds rgv3D[3] = {
			Vertex3Ds(pv1->x, pv1->y, height),
			Vertex3Ds(pv2->x, pv2->y, height),
			Vertex3Ds(pv3->x, pv3->y, height)};

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
		Vertex3Ds(pv3->x, pv3->y, height),
		Vertex3Ds(pv4->x, pv4->y, height),
		Vertex3Ds(pv1->x, pv1->y, height)};

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

   delete [] rgvLocal;
}

void Rubber::GetHitShapesDebug(Vector<HitObject> * const pvho)
{
}

void Rubber::AddJoint(Vector<HitObject> * pvho, const Vertex3Ds& v1, const Vertex3Ds& v2)
{
    SetupHitObject(pvho, new HitLine3D(v1, v2));
}

void Rubber::AddJoint2D(Vector<HitObject> * pvho, const Vertex2D& p, float zlow, float zhigh)
{
    SetupHitObject(pvho, new HitLineZ(p, zlow, zhigh));
}

void Rubber::SetupHitObject(Vector<HitObject> * pvho, HitObject * obj)
{
    obj->m_elasticity = m_d.m_elasticity;
    obj->m_elasticityFalloff = m_d.m_elasticityFalloff;
    obj->SetFriction(m_d.m_friction);
    obj->m_scatter = ANGTORAD(m_d.m_scatter);
    obj->m_fEnabled = m_d.m_fCollidable;

    obj->m_pfe = m_d.m_fHitEvent ? static_cast<IFireEvents*>(this) : NULL;

    pvho->AddElement(obj);
    m_vhoCollidable.push_back(obj);	//remember hit components of ramp
}

void Rubber::AddLine(Vector<HitObject> * const pvho, const Vertex2D & v1, const Vertex2D & v2, const float height1, const float height2)
{
   LineSeg * const plineseg = new LineSeg(v1, v2);
   SetupHitObject(pvho, plineseg);

   plineseg->m_rcHitRect.zlow = height1;
   plineseg->m_rcHitRect.zhigh = height2;
}

void Rubber::EndPlay()
{
    IEditable::EndPlay();
    m_vhoCollidable.clear();

   	if(staticVertexBuffer) {
		staticVertexBuffer->release();
		staticVertexBuffer = 0;
	}

	if(dynamicVertexBuffer) {
		dynamicVertexBuffer->release();
		dynamicVertexBuffer = 0;
		dynamicVertexBufferRegenerate = true;
	}

	if(dynamicIndexBuffer) {
		dynamicIndexBuffer->release();
		dynamicIndexBuffer = 0;
    }
}

float Rubber::GetDepth(const Vertex3Ds& viewDir) 
{
    // TODO: improve
    Vertex2D center2D;
    GetCenter(&center2D);
    const float centerZ = 0.5f * m_d.m_height;
    return m_d.m_depthBias + viewDir.x * center2D.x + viewDir.y * center2D.y + viewDir.z * centerZ;
}

void Rubber::RenderSetup(RenderDevice* pd3dDevice)
{
   GenerateVertexBuffer(pd3dDevice);
}

void Rubber::RenderStatic(RenderDevice* pd3dDevice)
{	
   if( m_d.m_staticRendering )
   {
      RenderObject(pd3dDevice);
   }
}

void Rubber::SetObjectPos()
{
   g_pvp->SetObjectPosCur(0, 0);
}

void Rubber::MoveOffset(const float dx, const float dy)
{
   for (int i=0;i<m_vdpoint.Size();i++)
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
   bw.WriteFloat(FID(RADB), m_d.m_depthBias);
   bw.WriteBool(FID(ESTR), m_d.m_staticRendering);
   bw.WriteFloat(FID(ROTX), m_d.m_rotX);
   bw.WriteFloat(FID(ROTY), m_d.m_rotY);

   ISelect::SaveData(pstm, hcrypthash, hcryptkey);

   bw.WriteTag(FID(PNTS));
   HRESULT hr;
   if(FAILED(hr = SavePointData(pstm, hcrypthash, hcryptkey)))
      return hr;

   bw.WriteTag(FID(ENDB));

   return S_OK;
}

HRESULT Rubber::InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   SetDefaults(false);

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
   else if (id == FID(ESTR))
   {
      pbr->GetBool(&m_d.m_staticRendering);
   }
   else if (id == FID(RADB))
   {
      pbr->GetFloat(&m_d.m_depthBias);
   }
   else if (id == FID(ROTX))
   {
       pbr->GetFloat(&m_d.m_rotX);
   }
   else if (id == FID(ROTY))
   {
       pbr->GetFloat(&m_d.m_rotY);
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
         int iSeg=-1;
         ClosestPointOnPolygon(vvertex, v, &vOut, &iSeg, true);

         // Go through vertices (including iSeg itself) counting control points until iSeg
         int icp = 0;
         for (int i=0;i<(iSeg+1);i++)
            if (vvertex[i].fControlPoint)
               icp++;

         // ClosestPointOnPolygon() couldn't find a point -> don't try to add a new point 
         // because that would lead to strange behavior
         if( iSeg==-1 )
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
            pdp->m_fSmooth = fTrue; // Ramps are usually always smooth
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

void Rubber::Rotate(float ang, Vertex2D *pvCenter)
{
   IHaveDragPoints::RotatePoints(ang, pvCenter);
}

void Rubber::Scale(float scalex, float scaley, Vertex2D *pvCenter)
{
   IHaveDragPoints::ScalePoints(scalex, scaley, pvCenter);
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

   for (size_t i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
      if (InlineIsEqualGUID(*arr[i],riid))
         return S_OK;

   return S_FALSE;
}


STDMETHODIMP Rubber::get_Height(float *pVal)
{
   *pVal = m_d.m_height;

   return S_OK;
}

STDMETHODIMP Rubber::put_Height(float newVal)
{
   if(m_d.m_height != newVal)
   {
	   STARTUNDO

	   m_d.m_height = newVal;
	   dynamicVertexBufferRegenerate = true;

	   STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Rubber::get_Thickness(int *pVal)
{
   *pVal = m_d.m_thickness;

   return S_OK;
}

STDMETHODIMP Rubber::put_Thickness(int newVal)
{
   if(m_d.m_thickness != newVal)
   {
	   STARTUNDO

	   m_d.m_thickness = newVal;
	   dynamicVertexBufferRegenerate = true;

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
   WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_szImage, 32, NULL, NULL);

   if(strcmp(m_szImage,m_d.m_szImage) != 0)
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
   STARTUNDO

   if (newVal > 1.0f) newVal = 1.0f;
      else if (newVal < 0.f) newVal = 0.f;
 
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
   BOOL fNewVal = VBTOF(newVal);	
   if (!g_pplayer)
   {	
      STARTUNDO

      m_d.m_fCollidable = !!fNewVal;		

      STOPUNDO
   }
   else
	   for (unsigned i=0; i < m_vhoCollidable.size(); i++)
	      m_vhoCollidable[i]->m_fEnabled = VBTOF(fNewVal);	//copy to hit checking on enities composing the object

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
    if( g_pplayer && m_d.m_staticRendering )
    {
        ShowError("Rubber is static! visible not supported!");
    }
    m_d.m_fVisible = VBTOF(newVal);
    STOPUNDO

    return S_OK;
}

STDMETHODIMP Rubber::get_DepthBias(float *pVal)
{
   *pVal = m_d.m_depthBias;

   return S_OK;
}

STDMETHODIMP Rubber::put_DepthBias(float newVal)
{
   if(m_d.m_depthBias != newVal)
   {
      STARTUNDO

      m_d.m_depthBias = newVal;
      dynamicVertexBufferRegenerate = true;

      STOPUNDO
   }

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

STDMETHODIMP Rubber::get_RotX(float *pVal)
{
    *pVal = m_d.m_rotX;

    return S_OK;
}

STDMETHODIMP Rubber::put_RotX(float newVal)
{
    if(m_d.m_rotX != newVal)
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
    if(m_d.m_rotY != newVal)
    {
        STARTUNDO

        m_d.m_rotY = newVal;
        dynamicVertexBufferRegenerate = true;

        STOPUNDO
    }

    return S_OK;
}

void Rubber::RenderObject(RenderDevice *pd3dDevice)
{
   TRACE_FUNCTION();

   // don't render if invisible or not a transparent ramp
   if (!m_d.m_fVisible )
      return;

   if ( m_d.m_thickness==0 )
   {
      dynamicVertexBufferRegenerate=false;
      return;
   }
   pd3dDevice->SetTextureAddressMode(0, RenderDevice::TEX_CLAMP);

   Material *mat = m_ptable->GetMaterial( m_d.m_szMaterial);
   pd3dDevice->basicShader->SetMaterial(mat);
   {
      Pin3D * const ppin3d = &g_pplayer->m_pin3d;
      Texture * const pin = m_ptable->GetImage(m_d.m_szImage);

      if (pin)
      {
         pd3dDevice->basicShader->PerformAlphaTest(true);
         pd3dDevice->basicShader->SetAlphaTestValue(1.0f / 255.0f);
         pd3dDevice->basicShader->SetTexture("Texture0", pin);
         pd3dDevice->basicShader->SetTechnique("basic_with_texture");
      }
      else
      {
          pd3dDevice->basicShader->SetTechnique("basic_without_texture");
      }

      if (dynamicVertexBufferRegenerate)
         UpdateRubber(pd3dDevice);

      pd3dDevice->basicShader->Begin(0);
      pd3dDevice->DrawIndexedPrimitiveVB(D3DPT_TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, dynamicVertexBuffer, 0, m_numVertices, dynamicIndexBuffer, 0, m_numIndices);
      pd3dDevice->basicShader->End();  
   }
}

// Always called each frame to render over everything else (along with primitives)
// Same code as RenderStatic (with the exception of the alpha tests).
// Also has less drawing calls by bundling seperate calls.
void Rubber::PostRenderStatic(RenderDevice* pd3dDevice)
{
   if ( !m_d.m_staticRendering )
   {
      RenderObject(pd3dDevice);
   }
}

void Rubber::GenerateVertexBuffer(RenderDevice* pd3dDevice)
{
    dynamicVertexBufferRegenerate = true;

    int accuracy=1;
    if( m_ptable->GetDetailLevel()<5 )
    {
       accuracy=6;
    }
    else if (m_ptable->GetDetailLevel()>=5 && m_ptable->GetDetailLevel()<8)
    {
       accuracy=8;
    }
    else
    {
       accuracy=(int)(m_ptable->GetDetailLevel()*1.3f);
    }

    Vertex2D * middlePoints = 0;
    const Vertex2D *rgvLocal = GetSplineVertex(splinePoints, NULL, &middlePoints);
    const int numRings=splinePoints-1;
    const int numSegments=accuracy;
    m_numVertices=(numRings)*(numSegments);
    m_numIndices = 6*m_numVertices;//m_numVertices*2+2;

    if (dynamicVertexBuffer)
        dynamicVertexBuffer->release();
    pd3dDevice->CreateVertexBuffer(m_numVertices, m_d.m_staticRendering ? 0 : USAGE_DYNAMIC, MY_D3DFVF_NOTEX2_VERTEX, &dynamicVertexBuffer);

    m_vertices.resize(m_numVertices);
    std::vector<WORD> rgibuf( m_numIndices );
    const float height = m_d.m_height+m_ptable->m_tableheight;

    Vertex3Ds prevB;
    Vertex3Ds binorm;
    Vertex3Ds normal;
    for( int i=0, index=0; i<numRings; i++ )
    {
        const int i2= (i==numRings-1) ? 0: i+1;

        Vertex3Ds tangent( middlePoints[i2].x-middlePoints[i].x, middlePoints[i2].y-middlePoints[i].y, 0.0f);
        
        if ( i==0 )
        {
            Vertex3Ds up( middlePoints[i2].x+middlePoints[i].x, middlePoints[i2].y+middlePoints[i].y, height*2.f);
            normal = CrossProduct(tangent,up);     //normal
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
        int si=index;
        for( int j=0;j<numSegments;j++,index++)
        {
            const float u=(float)i/numRings;
            const float v=((float)j+u)/numSegments;
            const float u_angle = u*(float)(2.0*M_PI);
            const float v_angle = v*(float)(2.0*M_PI);
            const Vertex3Ds tmp = GetRotatedAxis( (float)j*(360.0f/numSegments), tangent, normal )
							      * ((float)m_d.m_thickness*0.5f);
            m_vertices[index].x = middlePoints[i].x+tmp.x;
            m_vertices[index].y = middlePoints[i].y+tmp.y;
            m_vertices[index].z = height     +tmp.z;
            //texel
            m_vertices[index].tu = u;
            m_vertices[index].tv = v;
        }
    }
    // calculate faces
    for( int i=0;i<numRings;i++ )
    {
        for( int j=0;j<numSegments;j++ )
        {
            int quad[4];
            quad[0] = i*numSegments+j;

            if( j!=numSegments-1 )
                quad[1] = i*numSegments+j+1;
            else
                quad[1] = i*numSegments;

            if( i!=numRings-1 )
            {
                quad[2] = (i+1)*numSegments+j;
                if( j!=numSegments-1)
                    quad[3]=(i+1)*numSegments+j+1;
                else
                    quad[3]=(i+1)*numSegments;  
            }
            else
            {
                quad[2] = j;
                if(j!=numSegments-1)
                    quad[3] = j+1;
                else
                    quad[3] = 0;
            }
            rgibuf[(i*numSegments+j)*6  ] = quad[0];
            rgibuf[(i*numSegments+j)*6+1] = quad[1];
            rgibuf[(i*numSegments+j)*6+2] = quad[2];
            rgibuf[(i*numSegments+j)*6+3] = quad[3];
            rgibuf[(i*numSegments+j)*6+4] = quad[2];
            rgibuf[(i*numSegments+j)*6+5] = quad[1];
        }
    }
    //calculate normals
    for( int i=0;i<m_numIndices;i+=3)
    {
        SetNormal( &m_vertices[0], &rgibuf[i], 3);
    }

    Matrix3D fullMatrix;
    Matrix3D transMat,rotMat;

    float maxx=FLT_MIN;
    float minx=FLT_MAX;
    float maxy=FLT_MIN;
    float miny=FLT_MAX;
    float maxz=FLT_MIN;
    float minz=FLT_MAX;

    for( int i=0;i<m_numVertices;i++ )
    {
        if ( maxx<m_vertices[i].x ) maxx=m_vertices[i].x;
        if ( minx>m_vertices[i].x ) minx=m_vertices[i].x;
        if ( maxy<m_vertices[i].y ) maxy=m_vertices[i].y;
        if ( miny>m_vertices[i].y ) miny=m_vertices[i].y;
        if ( maxz<m_vertices[i].z ) maxz=m_vertices[i].z;
        if ( minz>m_vertices[i].z ) minz=m_vertices[i].z;
    }
    middlePoint.x = (maxx+minx)*0.5f;
    middlePoint.y = (maxy+miny)*0.5f;
    middlePoint.z = (maxz+minz)*0.5f;

    // Draw the floor of the ramp.
    Vertex3D_NoTex2 *buf;
    dynamicVertexBuffer->lock(0,0,(void**)&buf, VertexBuffer::WRITEONLY);
    memcpy( &buf[0], &m_vertices[0], sizeof(Vertex3D_NoTex2)*m_numVertices );
    dynamicVertexBuffer->unlock();

    if (dynamicIndexBuffer)
        dynamicIndexBuffer->release();
    dynamicIndexBuffer = pd3dDevice->CreateAndFillIndexBuffer( rgibuf );

    delete [] rgvLocal;
    delete [] middlePoints;
}

void Rubber::UpdateRubber( RenderDevice *pd3dDevice )
{
    Matrix3D fullMatrix;
    Matrix3D transMat,rotMat;
    const float height = m_d.m_height+m_ptable->m_tableheight;

    fullMatrix.SetIdentity();
    rotMat.RotateYMatrix(ANGTORAD(m_d.m_rotY));
    rotMat.Multiply(fullMatrix, fullMatrix);
    rotMat.RotateXMatrix(ANGTORAD(m_d.m_rotX));
    rotMat.Multiply(fullMatrix, fullMatrix);

    Vertex3D_NoTex2 *buf;

    if(m_d.m_staticRendering)
        dynamicVertexBuffer->lock(0,0,(void**)&buf, VertexBuffer::WRITEONLY);
    else
        dynamicVertexBuffer->lock(0,0,(void**)&buf, VertexBuffer::DISCARDCONTENTS);


    for( int i=0;i<m_numVertices;i++ )
    {
        Vertex3Ds vert(m_vertices[i].x-middlePoint.x, m_vertices[i].y-middlePoint.y, m_vertices[i].z-middlePoint.z);
        vert = fullMatrix.MultiplyVector(vert);
        buf[i].x = vert.x+middlePoint.x;
        buf[i].y = vert.y+middlePoint.y;
        buf[i].z = vert.z+height;
        vert = Vertex3Ds( m_vertices[i].nx, m_vertices[i].ny, m_vertices[i].nz );
        vert = fullMatrix.MultiplyVectorNoTranslate(vert);
        buf[i].nx = vert.x;
        buf[i].ny = vert.y;
        buf[i].nz = vert.z;
        buf[i].tu = m_vertices[i].tu;
        buf[i].tv = m_vertices[i].tv;
    }
    dynamicVertexBuffer->unlock();
    dynamicVertexBufferRegenerate=false;
}

void Rubber::UpdatePropertyPanes()
{
    if ( m_propVisual==NULL || m_propPosition==NULL || m_propPhysics==NULL )
        return;

    if ( !m_d.m_fCollidable )
    {
        EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd,110), FALSE);
        EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd,114), FALSE);
        EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd,115), FALSE);
    }
    else
    {
        EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd,110), TRUE);
        EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd,114), TRUE);
        EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd,115), TRUE);
    }
}

