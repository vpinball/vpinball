#include "StdAfx.h"

Rubber::Rubber()
{
   m_menuid = IDR_SURFACEMENU;
   m_d.m_fCollidable = true;
   m_d.m_fVisible = true;
   staticVertexBuffer = 0;
   dynamicVertexBuffer = 0;
   dynamicIndexBuffer = 0;
   dynamicVertexBufferRegenerate = true;
   m_d.m_enableLightingImage = true;
   m_d.m_depthBias = 0.0f;
   m_d.m_wireDiameter=6.0f;
   m_d.m_wireDistanceX = 38.0f;
   m_d.m_wireDistanceY = 88.0f;
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

   float length = 0.5f * GetRegStringAsFloatWithDefault("DefaultProps\\Rubber", "Length", 400.0f);

   CComObject<DragPoint> *pdp;
   for( int i=8;i>0;i-- )
   {
       const float angle = (float)(M_PI*2.0/8.0)*(float)i;
       float xx = x + sinf(angle)*50.0f;
       float yy = y - cosf(angle)*50.0f;
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

   m_d.m_height = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName,"HeightTop", 25.0f) : 25.0f;
   m_d.m_width = fromMouseClick ? GetRegIntWithDefault(strKeyName,"WidthTop", 8) : 8;
   m_d.m_color = fromMouseClick ? GetRegIntWithDefault(strKeyName,"Color", RGB(50,200,50)) : RGB(50,200,50);

   m_d.m_tdr.m_fTimerEnabled = fromMouseClick ? GetRegBoolWithDefault(strKeyName,"TimerEnabled", false) : false;
   m_d.m_tdr.m_TimerInterval = fromMouseClick ? GetRegIntWithDefault(strKeyName,"TimerInterval", 100) : 100;

   hr = GetRegString(strKeyName,"Image", m_d.m_szImage, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szImage[0] = 0;

   m_d.m_fCastsShadow = fromMouseClick ? GetRegBoolWithDefault(strKeyName,"CastsShadow", true) : true;

   m_d.m_elasticity = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName,"Elasticity", 0.3f) : 0.3f;
   m_d.m_friction = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName,"Friction", 0) : 0;
   m_d.m_scatter = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName,"Scatter", 0) : 0;

   m_d.m_fVisible = fromMouseClick ? GetRegBoolWithDefault(strKeyName,"Visible", true) : true;
   m_d.m_fCollidable = fromMouseClick ? GetRegBoolWithDefault(strKeyName,"Collidable", true) : true;

   m_d.m_enableLightingImage = fromMouseClick ? GetRegBoolWithDefault(strKeyName,"EnableLightingOnImage", true) : true;

   m_d.m_wireDiameter = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName,"WireDiameter", 60.0f) : 60.0f;
   m_d.m_staticRendering = fromMouseClick ? GetRegBoolWithDefault(strKeyName,"EnableStaticRendering", true) : true;
}

void Rubber::WriteRegDefaults()
{
   static const char strKeyName[] = "DefaultProps\\Rubber";

   SetRegValueFloat(strKeyName,"HeightTop", m_d.m_height);
   SetRegValueInt(strKeyName,"Width", m_d.m_width);
   SetRegValueInt(strKeyName,"Color", m_d.m_color);
   SetRegValue(strKeyName,"TimerEnabled",REG_DWORD,&m_d.m_tdr.m_fTimerEnabled,4);
   SetRegValue(strKeyName,"TimerInterval",REG_DWORD,&m_d.m_tdr.m_TimerInterval,4);
   SetRegValue(strKeyName,"Image", REG_SZ, &m_d.m_szImage, strlen(m_d.m_szImage));
   SetRegValueBool(strKeyName,"ImageWalls",m_d.m_fImageWalls);
   SetRegValueBool(strKeyName,"CastsShadow",m_d.m_fCastsShadow);
   SetRegValueFloat(strKeyName,"Elasticity", m_d.m_elasticity);
   SetRegValueFloat(strKeyName,"Friction", m_d.m_friction);
   SetRegValueFloat(strKeyName,"Scatter", m_d.m_scatter);
   SetRegValueBool(strKeyName,"Collidable",m_d.m_fCollidable);
   SetRegValueBool(strKeyName,"Visible",m_d.m_fVisible);
   SetRegValueBool(strKeyName,"EnableLighingOnImage",m_d.m_enableLightingImage);
   SetRegValueFloat(strKeyName,"WireDiameter", m_d.m_wireDiameter);
   SetRegValueBool(strKeyName,"EnableStaticRendering",m_d.m_staticRendering);
}

void Rubber::GetPointDialogPanes(Vector<PropertyPane> *pvproppane)
{
   PropertyPane *pproppane;

   pproppane = new PropertyPane(IDD_PROPPOINT_VISUALS, IDS_VISUALS);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPPOINT_POSITION_WITHZ, IDS_POSITION);
   pvproppane->AddElement(pproppane);
}


void Rubber::PreRender(Sur * const psur)
{
   //make 1 wire ramps look unique in editor - uses ramp color
   if (m_ptable->RenderSolid())
       psur->SetFillColor( m_d.m_color );
   else
       psur->SetFillColor(-1);
   psur->SetBorderColor(-1,false,0);
   psur->SetObject(this);

   int cvertex;
   Vertex2D * const rgvLocal = GetSplineVertex(cvertex, NULL, NULL);

   Vertex2D *newLocal = new Vertex2D[(cvertex+1)*2];
   for( int i=0;i<=cvertex;i++ )
   {
       int i1=i;
       if( i==cvertex ) i1=0;
       newLocal[i].x = rgvLocal[i1].x;
       newLocal[i].y = rgvLocal[i1].y;

       newLocal[(cvertex+1)*2-i-1].x = rgvLocal[cvertex*2-i1-1].x;
       newLocal[(cvertex+1)*2-i-1].y = rgvLocal[cvertex*2-i1-1].y;
   }

   psur->Polygon(newLocal, (cvertex+1)*2);

   delete [] newLocal;
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
   Vertex2D *newLocal = new Vertex2D[(cvertex+1)*2];
   for( int i=0;i<=cvertex;i++ )
   {
       int i1=i;
       if( i==cvertex ) i1=0;
       newLocal[i].x = rgvLocal[i1].x;
       newLocal[i].y = rgvLocal[i1].y;

       newLocal[(cvertex+1)*2-i-1].x = rgvLocal[cvertex*2-i1-1].x;
       newLocal[(cvertex+1)*2-i-1].y = rgvLocal[cvertex*2-i1-1].y;
   }

   psur->Polygon(newLocal, (cvertex+1)*2);
   
   delete [] newLocal;

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
   const Vertex2D *rgvLocal= GetSplineVertex(cvertex, &pfCross, NULL);

   Vertex2D *newLocal = new Vertex2D[(cvertex+1)*2];
   for( int i=0;i<=cvertex;i++ )
   {
       int i1=i;
       if( i==cvertex ) i1=0;
       newLocal[i].x = rgvLocal[i1].x;
       newLocal[i].y = rgvLocal[i1].y;

       newLocal[(cvertex+1)*2-i-1].x = rgvLocal[cvertex*2-i1-1].x;
       newLocal[(cvertex+1)*2-i-1].y = rgvLocal[cvertex*2-i1-1].y;
   }

   psur->Polygon(newLocal, (cvertex+1)*2);

   delete [] newLocal;

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

void Rubber::RenderShadow(ShadowSur * const psur, const float height)
{
   if (!m_d.m_fCastsShadow || !m_ptable->m_fRenderShadows || !m_d.m_fVisible) 
      return; //skip render if not visible

   psur->SetFillColor(RGB(0,0,0));
   psur->SetBorderColor(-1,false,0);
   psur->SetLineColor(RGB(0,0,0),false,2);
   psur->SetObject(this);

   int cvertex;
   Vertex2D * const rgvLocal = GetSplineVertex(cvertex, NULL, NULL);

   // Find the range of vertices to draw a shadow for
   int startvertex = cvertex;
   int stopvertex = 0;
   const int range = (stopvertex - startvertex);

   if (range > 0)
   {
        Vertex2D * const rgv2 = new Vertex2D[range*2];
        float * const rgheight2 = new float[range*2];

        for (int i=0;i<range;i++)
        {
        rgv2[i] = rgvLocal[i + startvertex];
        rgv2[range*2 - i - 1] = rgvLocal[cvertex*2 - i - 1 - startvertex];
        rgheight2[i] = m_d.m_height;
        rgheight2[range*2 - i - 1] = m_d.m_height;
        }

        psur->PolygonSkew(rgv2, range*2, rgheight2);

        delete [] rgv2;
        delete [] rgheight2;
   }

   delete [] rgvLocal;
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
 *  ppheight     - size cvertex, height of the ramp at the i-th vertex
 *  ppfCross     - size cvertex, true if i-th vertex corresponds to a control point
 *  ppratio      - how far along the ramp length the i-th vertex is, 1=start=bottom, 0=end=top (??)
 */
Vertex2D *Rubber::GetSplineVertex(int &pcvertex, bool ** const ppfCross, Vertex2D **pMiddlePoints)
{
   Vector<RenderVertex> vvertex;
   GetCentralCurve(&vvertex);
   // vvertex are the 2D vertices forming the central curve of the ramp as seen from above

   const int cvertex = vvertex.size();
   Vertex2D * const rgvLocal = new Vertex2D[cvertex * 2];
   if (ppfCross)
   {
      *ppfCross = new bool[cvertex];
   }

   if( pMiddlePoints )
   {
       *pMiddlePoints = new Vertex2D[cvertex];
   }
   // Compute an approximation to the length of the central curve
   // by adding up the lengths of the line segments.
   float totallength = 0;
   for (int i=0; i<(cvertex-1); i++)
   {
      const RenderVertex & v1 = vvertex[i];
      const RenderVertex & v2 = vvertex[i+1];

      const float dx = v1.x - v2.x;
      const float dy = v1.y - v2.y;
      const float length = sqrtf(dx*dx + dy*dy);

      totallength += length;
   }

   float currentlength = 0;
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

      // Update current length along the ramp.
      {
         const float dx = vprev.x - vmiddle.x;
         const float dy = vprev.y - vmiddle.y;
         const float length = sqrtf(dx*dx + dy*dy);

         currentlength += length;
      }

      const float percentage = currentlength / totallength;
      const float widthcur = (float)m_d.m_width;

      if( pMiddlePoints )
      {
         (*pMiddlePoints)[i] = vmiddle;
      }
      rgvLocal[i] = vmiddle + (widthcur*0.5f) * vnormal;
      rgvLocal[cvertex*2 - i - 1] = vmiddle - (widthcur*0.5f) * vnormal;
   }

   if (ppfCross)
      for (int i=0;i<cvertex;i++)
         (*ppfCross)[i] = vvertex.ElementAt(i)->fControlPoint;

   for (int i=0;i<cvertex;i++)
      delete vvertex.ElementAt(i);

   pcvertex = cvertex;
   return rgvLocal;
}

/*
 * Get an approximation of the curve described by the control points of this ramp.
 */
void Rubber::GetCentralCurve(Vector<RenderVertex> * const pvv)
{
   IHaveDragPoints::GetRgVertex(pvv);
}

float Rubber::GetSurfaceHeight(float x, float y)
{
    Vector<RenderVertex> vvertex;
    GetCentralCurve(&vvertex);

    const int cvertex = vvertex.Size();

    int iSeg;
    Vertex2D vOut;
    ClosestPointOnPolygon(vvertex, Vertex2D(x,y), &vOut, &iSeg, false);

    // Go through vertices (including iSeg itself) counting control points until iSeg
    float totallength = 0.f;
    float startlength = 0.f;
    float zheight = 0.f;

    if (iSeg == -1)
    {
        //zheight = 0;
        goto HeightError;
        //return 0; // Object is not on ramp path
    }

    for (int i2=1;i2<cvertex;i2++)
    {
        const float dx = vvertex.ElementAt(i2)->x - vvertex.ElementAt(i2-1)->x;
        const float dy = vvertex.ElementAt(i2)->y - vvertex.ElementAt(i2-1)->y;
        const float len = sqrtf(dx*dx + dy*dy);
        if (i2 <= iSeg)
        {
            startlength += len;
        }
        totallength += len;
    }

    {
        const float dx = vOut.x - vvertex.ElementAt(iSeg)->x;
        const float dy = vOut.y - vvertex.ElementAt(iSeg)->y;
        const float len = sqrtf(dx*dx + dy*dy);
        startlength += len; // Add the distance the object is between the two closest polyline segments.  Matters mostly for straight edges.

        zheight = (startlength/totallength) * (m_d.m_height );
    }
HeightError:
    for (int i2=0;i2<cvertex;i2++)
        delete vvertex.ElementAt(i2);

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
   float wallheightright=(float)m_d.m_width, wallheightleft=(float)m_d.m_width;

   for (int i=0;i<(cvertex-1);i++)
   {
       const Vertex2D * const pv1 = (i>0) ? &rgvLocal[i-1] : NULL;
       const Vertex2D * const pv2 = &rgvLocal[i];
       const Vertex2D * const pv3 = &rgvLocal[i+1];
       const Vertex2D * const pv4 = (i<(cvertex-2)) ? &rgvLocal[i+2] : NULL;

#ifndef RAMPTEST
       AddLine(pvho, pv2, pv3, pv1, m_d.m_height, m_d.m_height+wallheightright);
       AddLine(pvho, pv3, pv2, pv4, m_d.m_height, m_d.m_height+wallheightright);
#else
       AddSideWall(pvho, pv2, pv3,m_d.m_height, m_d.m_height, wallheightright);
       AddSideWall(pvho, pv3, pv2,m_d.m_height, m_d.m_height, wallheightright);
#endif
   }

   for (int i=0;i<(cvertex-1);i++)
   {
       const Vertex2D * const pv1 = (i>0) ? &rgvLocal[cvertex + i - 1] : NULL;
       const Vertex2D * const pv2 = &rgvLocal[cvertex + i];
       const Vertex2D * const pv3 = &rgvLocal[cvertex + i + 1];
       const Vertex2D * const pv4 = (i<(cvertex-2)) ? &rgvLocal[cvertex + i + 2] : NULL;

#ifndef RAMPTEST
       AddLine(pvho, pv2, pv3, pv1, m_d.m_height, m_d.m_height + wallheightleft);
       AddLine(pvho, pv3, pv2, pv4, m_d.m_height, m_d.m_height + wallheightleft);
#else
       AddSideWall(pvho, pv2, pv3, m_d.m_height, m_d.m_height, wallheightleft);
       AddSideWall(pvho, pv3, pv2, m_d.m_height, m_d.m_height, wallheightleft);
#endif
   }

#ifndef RAMPTEST
   // Add hit triangles for the ramp floor.
   {
      HitTriangle *ph3dpolyOld = NULL;

      const Vertex2D *pv1;
      const Vertex2D *pv2;
      const Vertex2D *pv3;
      const Vertex2D *pv4;

      for (int i=0;i<(cvertex-1);i++)
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
            // left ramp floor triangle, CCW order
            Vertex3Ds rgv3D[3];
            rgv3D[0] = Vertex3Ds(pv2->x,pv2->y,m_d.m_height);
            rgv3D[1] = Vertex3Ds(pv1->x,pv1->y,m_d.m_height);
            rgv3D[2] = Vertex3Ds(pv3->x,pv3->y,m_d.m_height);

            HitTriangle * const ph3dpoly = new HitTriangle(rgv3D); //!! this is not efficient at all, use native triangle-soup directly somehow

            if (ph3dpoly->IsDegenerate())       // degenerate triangles happen if width is 0 at some point
            {
                delete ph3dpoly;
            }
            else
            {
                ph3dpoly->m_elasticity = m_d.m_elasticity;
                ph3dpoly->SetFriction(m_d.m_friction);
                ph3dpoly->m_scatter = ANGTORAD(m_d.m_scatter);

                ph3dpoly->m_fVisible = fTrue;

                pvho->AddElement(ph3dpoly);

                m_vhoCollidable.push_back(ph3dpoly);	//remember hit components of ramp
                ph3dpoly->m_fEnabled = m_d.m_fCollidable;

                if (ph3dpolyOld)
                    CheckJoint(pvho, ph3dpolyOld, ph3dpoly);

                ph3dpolyOld = ph3dpoly;
            }
         }

         // right ramp floor triangle, CCW order
         Vertex3Ds rgv3D[3];
         rgv3D[0] = Vertex3Ds(pv3->x,pv3->y,m_d.m_height);
         rgv3D[1] = Vertex3Ds(pv1->x,pv1->y,m_d.m_height);
         rgv3D[2] = Vertex3Ds(pv4->x,pv4->y,m_d.m_height);

         HitTriangle * const ph3dpoly = new HitTriangle(rgv3D);
         if (ph3dpoly->IsDegenerate())
         {
             delete ph3dpoly;
         }
         else
         {
             ph3dpoly->m_elasticity = m_d.m_elasticity;
             ph3dpoly->SetFriction(m_d.m_friction);
             ph3dpoly->m_scatter = ANGTORAD(m_d.m_scatter);

             ph3dpoly->m_fVisible = fTrue;

             pvho->AddElement(ph3dpoly);

             m_vhoCollidable.push_back(ph3dpoly);	//remember hit components of ramp
             ph3dpoly->m_fEnabled = m_d.m_fCollidable;

             if (ph3dpolyOld)
                 CheckJoint(pvho, ph3dpolyOld, ph3dpoly);
             ph3dpolyOld = ph3dpoly;
         }
      }

      Vertex3Ds rgv3D[3];
      rgv3D[0] = Vertex3Ds(pv4->x,pv4->y,m_d.m_height);
      rgv3D[1] = Vertex3Ds(pv3->x,pv3->y,m_d.m_height);
      rgv3D[2] = Vertex3Ds(pv1->x,pv1->y,m_d.m_height);
      ph3dpolyOld = new HitTriangle(rgv3D);

      CheckJoint(pvho, ph3dpolyOld, ph3dpolyOld);
      delete ph3dpolyOld;
      ph3dpolyOld = NULL;
   }

   // add outside bottom, 
   // joints at the intersections are not needed since the inner surface has them
   // this surface is identical... except for the direction of the normal face.
   // hence the joints protect both surface edges from having a fall through

   for (int i=0; i<(cvertex-1); i++)
   {
      // see sketch above
      const Vertex2D * const pv1 = &rgvLocal[i];
      const Vertex2D * const pv2 = &rgvLocal[cvertex*2 - i - 1];
      const Vertex2D * const pv3 = &rgvLocal[cvertex*2 - i - 2];
      const Vertex2D * const pv4 = &rgvLocal[i+1];

      {
         // left ramp triangle, order CW
         Vertex3Ds rgv3D[3];
         rgv3D[0] = Vertex3Ds(pv1->x,pv1->y,m_d.m_height);
         rgv3D[1] = Vertex3Ds(pv2->x,pv2->y,m_d.m_height);
         rgv3D[2] = Vertex3Ds(pv3->x,pv3->y,m_d.m_height);

         HitTriangle * const ph3dpoly = new HitTriangle(rgv3D);
         if (ph3dpoly->IsDegenerate())
         {
             delete ph3dpoly;
         }
         else
         {
             ph3dpoly->m_elasticity = m_d.m_elasticity;
             ph3dpoly->SetFriction(m_d.m_friction);
             ph3dpoly->m_scatter = ANGTORAD(m_d.m_scatter);

             pvho->AddElement(ph3dpoly);

             m_vhoCollidable.push_back(ph3dpoly);	//remember hit components of ramp
             ph3dpoly->m_fEnabled = m_d.m_fCollidable;
         }
      }

      // right ramp triangle, order CW
      Vertex3Ds rgv3D[3];
      rgv3D[0] = Vertex3Ds(pv3->x,pv3->y,m_d.m_height);
      rgv3D[1] = Vertex3Ds(pv4->x,pv4->y,m_d.m_height);
      rgv3D[2] = Vertex3Ds(pv1->x,pv1->y,m_d.m_height);

      HitTriangle * const ph3dpoly = new HitTriangle(rgv3D);
      if (ph3dpoly->IsDegenerate())
      {
          delete ph3dpoly;
      }
      else
      {
          ph3dpoly->m_elasticity = m_d.m_elasticity;
          ph3dpoly->SetFriction(m_d.m_friction);
          ph3dpoly->m_scatter = ANGTORAD(m_d.m_scatter);

          pvho->AddElement(ph3dpoly);

          m_vhoCollidable.push_back(ph3dpoly);	//remember hit components of ramp
          ph3dpoly->m_fEnabled = m_d.m_fCollidable;
      }
   }
#endif

   delete [] rgvLocal;
}

void Rubber::GetHitShapesDebug(Vector<HitObject> * const pvho)
{
}

void Rubber::AddSideWall(Vector<HitObject> * const pvho, const Vertex2D * const pv1, const Vertex2D * const pv2, const float height1, const float height2, const float wallheight)
{
   Vertex3Ds * const rgv3D = new Vertex3Ds[4];
   rgv3D[0] = Vertex3Ds(pv1->x,pv1->y,height1 - (float)PHYS_SKIN);
   rgv3D[1] = Vertex3Ds(pv2->x,pv2->y,height2 - (float)PHYS_SKIN);
   rgv3D[2] = Vertex3Ds(pv2->x + WALLTILT,pv2->y + WALLTILT,height2 + wallheight);
   rgv3D[3] = Vertex3Ds(pv1->x + WALLTILT,pv1->y + WALLTILT,height1 + wallheight);

   Hit3DPoly * const ph3dpoly = new Hit3DPoly(rgv3D,4); //!!
   ph3dpoly->m_elasticity = m_d.m_elasticity;
   ph3dpoly->SetFriction(m_d.m_friction);
   ph3dpoly->m_scatter = ANGTORAD(m_d.m_scatter);

   pvho->AddElement(ph3dpoly);

   m_vhoCollidable.push_back(ph3dpoly);	//remember hit components of ramp
   ph3dpoly->m_fEnabled = m_d.m_fCollidable;
}

void Rubber::CheckJoint(Vector<HitObject> * const pvho, const HitTriangle * const ph3d1, const HitTriangle * const ph3d2)
{
   Vertex3Ds vjointnormal = CrossProduct(ph3d1->normal, ph3d2->normal);
   //vjointnormal.x = ph3d1->normal.x + ph3d2->normal.x;
   //vjointnormal.y = ph3d1->normal.y + ph3d2->normal.y;
   //vjointnormal.z = ph3d1->normal.z + ph3d2->normal.z;

   const float sqrlength = vjointnormal.x * vjointnormal.x + vjointnormal.y * vjointnormal.y + vjointnormal.z * vjointnormal.z;
   if (sqrlength < 1.0e-8f) return;

   const float inv_length = 1.0f/sqrtf(sqrlength);
   vjointnormal.x *= inv_length;
   vjointnormal.y *= inv_length;
   vjointnormal.z *= inv_length;

   // By convention of the calling function, points 1 [0] and 2 [1] of the second polygon will
   // be the common-edge points

   Hit3DCylinder * const ph3dc = new Hit3DCylinder(&ph3d2->m_rgv[0], &ph3d2->m_rgv[1], &vjointnormal);
   ph3dc->m_elasticity = m_d.m_elasticity;
   ph3dc->SetFriction(m_d.m_friction);
   ph3dc->m_scatter = ANGTORAD(m_d.m_scatter);
   pvho->AddElement(ph3dc);

   m_vhoCollidable.push_back(ph3dc);	//remember hit components of ramp
   ph3dc->m_fEnabled = m_d.m_fCollidable;
}


void Rubber::AddLine(Vector<HitObject> * const pvho, const Vertex2D * const pv1, const Vertex2D * const pv2, const Vertex2D * const pv3, const float height1, const float height2)
{
   LineSeg * const plineseg = new LineSeg();
   plineseg->m_elasticity = m_d.m_elasticity;
   plineseg->SetFriction(m_d.m_friction);
   plineseg->m_scatter = ANGTORAD(m_d.m_scatter);

   plineseg->m_pfe = NULL;

   plineseg->m_rcHitRect.zlow = height1;
   plineseg->m_rcHitRect.zhigh = height2;

   plineseg->v1 = *pv1;
   plineseg->v2 = *pv2;

   pvho->AddElement(plineseg);

   m_vhoCollidable.push_back(plineseg);	//remember hit components of ramp
   plineseg->m_fEnabled = m_d.m_fCollidable;

   plineseg->CalcNormal();

   const Vertex2D vt1(pv1->x - pv2->x, pv1->y - pv2->y);

   if (pv3)
   {
      const Vertex2D vt2(pv1->x - pv3->x, pv1->y - pv3->y);

      const float dot = vt1.x*vt2.y - vt1.y*vt2.x;

      if (dot < 0) // Inside edges don't need joint hit-testing (dot == 0 continuous segments should mathematically never hit)
      {
         Joint * const pjoint = new Joint();
         pjoint->m_elasticity = m_d.m_elasticity;
         pjoint->SetFriction(m_d.m_friction);
         pjoint->m_scatter = ANGTORAD(m_d.m_scatter);

         pjoint->m_pfe = NULL;

         pjoint->m_rcHitRect.zlow = height1;
         pjoint->m_rcHitRect.zhigh = height2;

         pjoint->center = *pv1;
         pvho->AddElement(pjoint);

         m_vhoCollidable.push_back(pjoint);	//remember hit components of ramp
         pjoint->m_fEnabled = m_d.m_fCollidable;

         // Set up line normal
         const float inv_length = 1.0f/sqrtf(vt2.x * vt2.x + vt2.y * vt2.y);
         pjoint->normal.x = plineseg->normal.x - vt2.y *inv_length;
         pjoint->normal.y = vt2.x *inv_length + plineseg->normal.y;

         // Set up line normal
         const float inv_length2 = 1.0f/sqrtf(pjoint->normal.x * pjoint->normal.x + pjoint->normal.y * pjoint->normal.y);
         pjoint->normal.x *= inv_length2;
         pjoint->normal.y *= inv_length2;
      }
   }
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

static const WORD rgicrosssection[] = {
   0,1,16,
   1,17,16,
   1,2,17,
   2,18,17,
   2,3,18,
   3,19,18,
   3,0,19,
   0,16,19,

   8,9,24,
   9,25,24,
   9,10,25,
   10,26,25,
   10,11,26,
   11,27,26,
   11,8,27,
   8,24,27,

   4,5,20,
   5,21,20,
   5,6,21,
   6,22,21,
   6,7,22,
   7,23,22,
   7,4,23,
   4,20,23,

   12,13,28,
   13,29,28,
   13,14,29,
   14,30,29,
   14,15,30,
   15,31,30,
   15,12,31,
   12,28,31
};


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

   solidMaterial.setColor( 1.0f, m_d.m_color );

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
   bw.WriteInt(FID(WDTP), m_d.m_width);
   bw.WriteInt(FID(COLR), m_d.m_color);
   bw.WriteBool(FID(TMON), m_d.m_tdr.m_fTimerEnabled);
   bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
   bw.WriteWideString(FID(NAME), (WCHAR *)m_wzName);
   bw.WriteString(FID(IMAG), m_d.m_szImage);
   bw.WriteBool(FID(IMGW), m_d.m_fImageWalls);
   bw.WriteBool(FID(CSHD), m_d.m_fCastsShadow);
   bw.WriteFloat(FID(ELAS), m_d.m_elasticity);
   bw.WriteFloat(FID(RFCT), m_d.m_friction);
   bw.WriteFloat(FID(RSCT), m_d.m_scatter);
   bw.WriteBool(FID(CLDRP), m_d.m_fCollidable);
   bw.WriteBool(FID(RVIS), m_d.m_fVisible);
   bw.WriteBool(FID(ERLI), m_d.m_enableLightingImage);
   bw.WriteFloat(FID(RADB), m_d.m_depthBias);
   bw.WriteFloat(FID(RADI), m_d.m_wireDiameter);
   bw.WriteFloat(FID(RADX), m_d.m_wireDistanceX);
   bw.WriteFloat(FID(RADY), m_d.m_wireDistanceY);
   bw.WriteBool(FID(ESTR), m_d.m_staticRendering);

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
      pbr->GetInt(&m_d.m_width);
   }
   else if (id == FID(COLR))
   {
      pbr->GetInt(&m_d.m_color);
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
   else if (id == FID(IMGW))
   {
      pbr->GetBool(&m_d.m_fImageWalls);
   }
   else if (id == FID(CSHD))
   {
      pbr->GetBool(&m_d.m_fCastsShadow);
   }
   else if (id == FID(NAME))
   {
      pbr->GetWideString((WCHAR *)m_wzName);
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
   else if (id == FID(ERLI))
   {
      pbr->GetBool(&m_d.m_enableLightingImage);
   }
   else if (id == FID(ESTR))
   {
      pbr->GetBool(&m_d.m_staticRendering);
   }
   else if (id == FID(RADB))
   {
      pbr->GetFloat(&m_d.m_depthBias);
   }
   else if (id == FID(RADI))
   {
       pbr->GetFloat(&m_d.m_wireDiameter);
   }
   else if (id == FID(RADX))
   {
       pbr->GetFloat(&m_d.m_wireDistanceX);
   }
   else if (id == FID(RADY))
   {
       pbr->GetFloat(&m_d.m_wireDistanceY);
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
         STARTUNDO

         RECT rc;
         GetClientRect(m_ptable->m_hwnd, &rc);

         HitSur * const phs = new HitSur(NULL, m_ptable->m_zoom, m_ptable->m_offsetx, m_ptable->m_offsety, rc.right - rc.left, rc.bottom - rc.top, 0, 0, NULL);

         const Vertex2D v = phs->ScreenToSurface(x, y);
         delete phs;

         Vector<RenderVertex> vvertex;
         GetCentralCurve(&vvertex);

         const int cvertex = vvertex.size();
         Vertex2D vOut;
         int iSeg;
         ClosestPointOnPolygon(vvertex, v, &vOut, &iSeg, false);

         // Go through vertices (including iSeg itself) counting control points until iSeg
         int icp = 0;
         for (int i=0;i<(iSeg+1);i++)
            if (vvertex.ElementAt(i)->fControlPoint)
               icp++;

         for (int i=0;i<cvertex;i++)
            delete vvertex.ElementAt(i);

         //if (icp == 0) // need to add point after the last point
         //icp = m_vdpoint.Size();

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


STDMETHODIMP Rubber::get_Width(int *pVal)
{
   *pVal = m_d.m_width;

   return S_OK;
}

STDMETHODIMP Rubber::put_Width(int newVal)
{
   if(m_d.m_width != newVal)
   {
	   STARTUNDO

	   m_d.m_width = newVal;
	   dynamicVertexBufferRegenerate = true;

	   STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Rubber::get_Color(OLE_COLOR *pVal)
{
   *pVal = m_d.m_color;

   return S_OK;
}

STDMETHODIMP Rubber::put_Color(OLE_COLOR newVal)
{
   if(m_d.m_color != newVal)
   {
	   STARTUNDO

	   m_d.m_color = newVal;

	   STOPUNDO
   }

   return S_OK;
}

void Rubber::GetDialogPanes(Vector<PropertyPane> *pvproppane)
{
   PropertyPane *pproppane;

   pproppane = new PropertyPane(IDD_PROP_NAME, NULL);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPRUBBER_VISUALS, IDS_VISUALS);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPRUBBER_POSITION, IDS_POSITION);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPRUBBER_PHYSICS, IDS_PHYSICS);
   pvproppane->AddElement(pproppane);

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

STDMETHODIMP Rubber::get_CastsShadow(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fCastsShadow);

   return S_OK;
}

STDMETHODIMP Rubber::put_CastsShadow(VARIANT_BOOL newVal)
{
   STARTUNDO
   
   m_d.m_fCastsShadow = VBTOF(newVal);
   
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
	      m_vhoCollidable[i]->m_fEnabled = fNewVal;	//copy to hit checking on enities composing the object

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

STDMETHODIMP Rubber::get_EnableLightingImage(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_enableLightingImage);

   return S_OK;
}

STDMETHODIMP Rubber::put_EnableLightingImage(VARIANT_BOOL newVal)
{
   STARTUNDO

   m_d.m_enableLightingImage= VBTOF(newVal);

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


void Rubber::RenderObject(RenderDevice *pd3dDevice)
{
   TRACE_FUNCTION();

   // don't render if invisible or not a transparent ramp
   if (!m_d.m_fVisible )
      return;

   if ( m_d.m_width==0 )
   {
      dynamicVertexBufferRegenerate=false;
      return;
   }
   pd3dDevice->SetTextureAddressMode(ePictureTexture, RenderDevice::TEX_CLAMP);

   solidMaterial.setColor(1.0f, m_d.m_color );

   {
      Pin3D * const ppin3d = &g_pplayer->m_pin3d;
      Texture * const pin = m_ptable->GetImage(m_d.m_szImage);

      if (pin)
      {
         pin->CreateAlphaChannel();
         pin->Set( ePictureTexture );

         ppin3d->SetTextureFilter ( ePictureTexture, TEXTURE_MODE_TRILINEAR );

         pd3dDevice->SetMaterial(textureMaterial);
         if ( !m_d.m_enableLightingImage )
            pd3dDevice->SetRenderState( RenderDevice::LIGHTING, FALSE );
      }
      else
      {
         ppin3d->SetTexture(NULL);
         pd3dDevice->SetMaterial(solidMaterial);
      }

      if (!dynamicVertexBuffer || dynamicVertexBufferRegenerate)
         GenerateVertexBuffer(pd3dDevice);

      int idx=0;

      unsigned int offset=0;
      pd3dDevice->DrawIndexedPrimitiveVB(D3DPT_TRIANGLELIST, dynamicVertexBuffer, offset, m_numVertices, dynamicIndexBuffer, 0, m_numIndices);
      offset += m_numVertices;

      if ( !m_d.m_enableLightingImage && pin!=NULL )
         pd3dDevice->SetRenderState( RenderDevice::LIGHTING, TRUE );
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

Vertex3Ds GetRotatedAxis( float angle, Vertex3Ds &axis, Vertex3Ds &temp)
{
    if(angle==0.0)
        return Vertex3Ds(0.0f,0.0f,0.0f);
    
    axis.Normalize();
    Vertex3Ds u=axis;

    Vertex3Ds rotMatrixRow0, rotMatrixRow1, rotMatrixRow2;

    float sinAngle=sinf((float)M_PI*angle/180);
    float cosAngle=cosf((float)M_PI*angle/180);
    float oneMinusCosAngle=1.0f-cosAngle;

    rotMatrixRow0.x=(u.x)*(u.x) + cosAngle*(1-(u.x)*(u.x));
    rotMatrixRow0.y=(u.x)*(u.y)*(oneMinusCosAngle) - sinAngle*u.z;
    rotMatrixRow0.z=(u.x)*(u.z)*(oneMinusCosAngle) + sinAngle*u.y;

    rotMatrixRow1.x=(u.x)*(u.y)*(oneMinusCosAngle) + sinAngle*u.z;
    rotMatrixRow1.y=(u.y)*(u.y) + cosAngle*(1-(u.y)*(u.y));
    rotMatrixRow1.z=(u.y)*(u.z)*(oneMinusCosAngle) - sinAngle*u.x;

    rotMatrixRow2.x=(u.x)*(u.z)*(oneMinusCosAngle) - sinAngle*u.y;
    rotMatrixRow2.y=(u.y)*(u.z)*(oneMinusCosAngle) + sinAngle*u.x;
    rotMatrixRow2.z=(u.z)*(u.z) + cosAngle*(1-(u.z)*(u.z));

    return Vertex3Ds(temp.Dot(rotMatrixRow0), temp.Dot(rotMatrixRow1), temp.Dot(rotMatrixRow2));
}
void Rubber::GenerateVertexBuffer(RenderDevice* pd3dDevice)
{
    dynamicVertexBufferRegenerate = false;
    Texture * const pin = m_ptable->GetImage(m_d.m_szImage);
    Vertex2D *middlePoints=0;
    int accuracy=1;
    if( m_ptable->GetAlphaRampsAccuracy()<5 )
    {
       accuracy=6;
    }
    else if (m_ptable->GetAlphaRampsAccuracy()>=5 && m_ptable->GetAlphaRampsAccuracy()<8)
    {
       accuracy=8;
    }
    else
    {
       accuracy=(int)(m_ptable->GetAlphaRampsAccuracy()*1.3f);
    }

    const Vertex2D *rgvLocal = GetSplineVertex(splinePoints, NULL, &middlePoints);
    const int numRings=splinePoints;
    const int numSegments=accuracy;
    const float inv_tablewidth = 1.0f/(m_ptable->m_right - m_ptable->m_left);
    const float inv_tableheight = 1.0f/(m_ptable->m_bottom - m_ptable->m_top);

    m_numVertices=(numRings)*(numSegments);
    m_numIndices = 6*m_numVertices;//m_numVertices*2+2;

    if (dynamicVertexBuffer)
        dynamicVertexBuffer->release();

    pd3dDevice->CreateVertexBuffer(m_numVertices, 0, MY_D3DFVF_NOTEX2_VERTEX, &dynamicVertexBuffer);

    Vertex3D_NoTex2 *buf;
    dynamicVertexBuffer->lock(0,0,(void**)&buf, VertexBuffer::WRITEONLY);

    Vertex3D_NoTex2* rgvbuf = new Vertex3D_NoTex2[m_numVertices];
    std::vector<WORD> rgibuf( m_numIndices );
    
    Vertex3Ds prevB;
    Vertex3Ds binorm;
    Vertex3Ds normal;
    for( int i=0, index=0; i<numRings; i++ )
    {
        const int i2= (i==numRings-1) ? 0: i+1;

        Vertex3Ds tangent( middlePoints[i2].x-middlePoints[i].x, middlePoints[i2].y-middlePoints[i].y, 0.0f);
        
        if ( i==0 )
        {
            Vertex3Ds up( middlePoints[i2].x+middlePoints[i].x, middlePoints[i2].y+middlePoints[i].y, m_d.m_height*2);
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
            float u=(float)i/numRings;
            float v=(float)(j+u)/numSegments;
            float u_angle = u*2.0f*(float)M_PI;
            float v_angle = v*2.0f*(float)M_PI;
            Vertex3Ds tmp=GetRotatedAxis( j*360.0f/numSegments, tangent, normal );
            tmp *= (float)m_d.m_width;
            rgvbuf[index].x = middlePoints[i].x+tmp.x;
            rgvbuf[index].y = middlePoints[i].y+tmp.y;
            rgvbuf[index].z = m_d.m_height     +tmp.z;
            //texel
            rgvbuf[index].tu = u;
            rgvbuf[index].tv = v;
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
        SetNormal( rgvbuf, &rgibuf[i], 3);
    }
    // Draw the floor of the ramp.
    memcpy( &buf[0], &rgvbuf[0], sizeof(Vertex3D_NoTex2)*m_numVertices );
    dynamicVertexBuffer->unlock();

    if (dynamicIndexBuffer)
        dynamicIndexBuffer->release();
    dynamicIndexBuffer = pd3dDevice->CreateAndFillIndexBuffer( rgibuf );


    delete [] rgvbuf;
    delete [] rgvLocal;
    delete [] middlePoints;
}
