#include "StdAfx.h"
#include "meshes/triggerSimpleMesh.h"
#include "meshes/triggerStarMesh.h"

Trigger::Trigger()
{
   m_ptriggerhitcircle = NULL;

   hitEvent = false;
   unhitEvent = false;
   doAnimation = false;
   moveDown = false;
   animHeightOffset=0.0f;

   m_hitEnabled = true;
   vertexBuffer = NULL;
   triggerIndexBuffer=NULL;
   triggerVertices=NULL;
   m_menuid = IDR_SURFACEMENU;
   m_propVisual = NULL;
}

Trigger::~Trigger()
{
   if( vertexBuffer )
   {
      vertexBuffer->release();
      vertexBuffer=0;
   }
   if( triggerIndexBuffer )
   {
       triggerIndexBuffer->release();
       triggerIndexBuffer=0;
   }
   if ( triggerVertices )
   {
       delete [] triggerVertices;
       triggerVertices=0;
   }
}


void Trigger::UpdateEditorView()
{
    if( m_d.m_shape!=TriggerNone )
    {
        Vertex3D_NoTex2 *meshVertices;
        float lengthX=30.0f;
        float lengthY=30.0f;
        float maxx=-FLT_MAX;        
        float minx=FLT_MAX;
        float maxy=-FLT_MAX;
        float miny=FLT_MAX;

        if( m_d.m_shape==TriggerWire )
        {
            numVertices=triggerSimpleNumVertices;
            numFaces=triggerSimpleNumFaces;
            faceIndices = triggerSimpleIndices;
            meshVertices = triggerSimple;
        }
        else if( m_d.m_shape==TriggerStar )
        {
            numVertices=triggerStarNumVertices;
            numFaces=triggerStarNumFaces;
            faceIndices = triggerStarIndices;
            meshVertices = triggerStar;
        }
        vertices.resize(numVertices);
        Matrix3D fullMatrix;
        fullMatrix.RotateZMatrix(ANGTORAD(m_d.m_rotation));
        for( int i=0;i<numVertices;i++ )
        {
            Vertex3Ds vert(meshVertices[i].x,meshVertices[i].y,meshVertices[i].z);
            fullMatrix.MultiplyVector(vert, vertices[i]);
            if ( m_d.m_shape!=TriggerStar )
            {
                vertices[i].x *= m_d.m_scaleX;
                vertices[i].y *= m_d.m_scaleY;
            }
            else
            {
                vertices[i].x *= m_d.m_radius;
                vertices[i].y *= m_d.m_radius;
            }
            vertices[i].x += m_d.m_vCenter.x;
            vertices[i].y += m_d.m_vCenter.y;
            if( vertices[i].x>maxx ) maxx=vertices[i].x;
            if( vertices[i].x<minx ) minx=vertices[i].x;
            if( vertices[i].y>maxy ) maxy=vertices[i].y;
            if( vertices[i].y<miny ) miny=vertices[i].y;
        }
    }
}

void Trigger::InitShape( float x, float y )
{
    float lengthX=30.0f;
    float lengthY=30.0f;
    UpdateEditorView();
    for (int i=0;i<m_vdpoint.size();i++)
    {
        m_vdpoint.ElementAt(i)->Release();
    }
    m_vdpoint.RemoveAllElements();
    // First time shape has been set to custom - set up some points
    CComObject<DragPoint> *pdp;
    CComObject<DragPoint>::CreateInstance(&pdp);
    if (pdp)
    {
        pdp->AddRef();
        pdp->Init(this, x-lengthX, y-lengthY);
        m_vdpoint.AddElement(pdp);
    }
    CComObject<DragPoint>::CreateInstance(&pdp);
    if (pdp)
    {
        pdp->AddRef();
        pdp->Init(this, x-lengthX, y+lengthY);
        m_vdpoint.AddElement(pdp);
    }
    CComObject<DragPoint>::CreateInstance(&pdp);
    if (pdp)
    {
        pdp->AddRef();
        pdp->Init(this, x+lengthX, y+lengthY);
        m_vdpoint.AddElement(pdp);
    }
    CComObject<DragPoint>::CreateInstance(&pdp);
    if (pdp)
    {
        pdp->AddRef();
        pdp->Init(this, x+lengthX, y-lengthY);
        m_vdpoint.AddElement(pdp);
    }
}

HRESULT Trigger::Init(PinTable *ptable, float x, float y, bool fromMouseClick)
{
   m_ptable = ptable;

   m_d.m_vCenter.x = x;
   m_d.m_vCenter.y = y;

   SetDefaults(fromMouseClick);
   if ( m_vdpoint.size()==0 )
       InitShape(x,y);

   return InitVBA(fTrue, 0, NULL);
}

void Trigger::SetDefaults(bool fromMouseClick)
{
   HRESULT hr;
   float fTmp;
   int iTmp;

   hr = GetRegStringAsFloat("DefaultProps\\Trigger","Radius", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
       m_d.m_radius = fTmp;
   else
       m_d.m_radius = 25.0f;

   hr = GetRegStringAsFloat("DefaultProps\\Trigger","Rotation", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
       m_d.m_rotation = fTmp;
   else
       m_d.m_rotation = 0.0f;

   hr = GetRegStringAsFloat("DefaultProps\\Trigger","ScaleX", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
       m_d.m_scaleX = fTmp;
   else
       m_d.m_scaleX = 1.0f;

   hr = GetRegStringAsFloat("DefaultProps\\Trigger","ScaleY", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
       m_d.m_scaleY = fTmp;
   else
       m_d.m_scaleY = 1.0f;

   hr = GetRegInt("DefaultProps\\Trigger","TimerEnabled", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_tdr.m_fTimerEnabled = iTmp == 0 ? fFalse : fTrue;
   else
      m_d.m_tdr.m_fTimerEnabled = fFalse;

   hr = GetRegInt("DefaultProps\\Trigger","TimerInterval", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_tdr.m_TimerInterval = iTmp;
   else
      m_d.m_tdr.m_TimerInterval = 100;

   hr = GetRegInt("DefaultProps\\Trigger","Enabled", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fEnabled = iTmp == 0 ? false : true;
   else
      m_d.m_fEnabled = true;

   hr = GetRegInt("DefaultProps\\Trigger","Visible", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fVisible = iTmp == 0 ? false : true;
   else
      m_d.m_fVisible = true;

   hr = GetRegStringAsFloat("DefaultProps\\Trigger","HitHeight", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_hit_height = fTmp;
   else
      m_d.m_hit_height = 50.0f;

   hr = GetRegInt("DefaultProps\\Trigger","Shape", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_shape = (enum TriggerShape)iTmp;
   else
      m_d.m_shape = TriggerWire;
   hr = GetRegString("DefaultProps\\Trigger","Surface", &m_d.m_szSurface, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szSurface[0] = 0;

   hr = GetRegStringAsFloat("DefaultProps\\trigger","AnimSpeed", &fTmp);
   m_d.m_animSpeed = (hr == S_OK) && fromMouseClick ? fTmp : 1.0f;
}

void Trigger::PreRender(Sur * const psur)
{
   if( m_vdpoint.size()==0 )
    InitShape(m_d.m_vCenter.x, m_d.m_vCenter.y);

   if( m_d.m_shape != TriggerStar )
   {
       psur->SetBorderColor(-1,false,0);
       psur->SetObject(this);

       psur->SetFillColor(m_ptable->RenderSolid() ? RGB(200,220,200) : -1);

       std::vector<RenderVertex> vvertex;
       GetRgVertex(vvertex);

       psur->Polygon(vvertex);
   }
   else
   {
       psur->SetFillColor(-1);
       psur->Ellipse(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_radius);
   }
}

void Trigger::Render(Sur * const psur)
{
   psur->SetLineColor(RGB(0,0,0), false, 0);
   psur->SetObject(this);
   psur->SetFillColor(-1);

   if ( m_d.m_shape != TriggerStar )
   {
       std::vector<RenderVertex> vvertex;
       GetRgVertex(vvertex);

       psur->SetObject(NULL);
       psur->SetBorderColor(RGB(0,180,0),false,1);

       psur->Polygon(vvertex);

       bool fDrawDragpoints = (m_selectstate != eNotSelected) || (g_pvp->m_fAlwaysDrawDragPoints);		//>>> added by chris
       // if the item is selected then draw the dragpoints (or if we are always to draw dragpoints)
       if ( !fDrawDragpoints )
       {
           // if any of the dragpoints of this object are selected then draw all the dragpoints
           for (int i=0;i<m_vdpoint.size();i++)
           {
               CComObject<DragPoint> * const pdp = m_vdpoint.ElementAt(i);
               if (pdp->m_selectstate != eNotSelected)
               {
                   fDrawDragpoints = true;
                   break;
               }
           }
       }

       if (fDrawDragpoints)
       {
           for (int i=0;i<m_vdpoint.size();i++)
           {
               CComObject<DragPoint> * const pdp = m_vdpoint.ElementAt(i);
               psur->SetFillColor(-1);
               psur->SetBorderColor(RGB(0,180,0),false,0);
               psur->SetObject(pdp); 

               if (pdp->m_fDragging)
                   psur->SetBorderColor(RGB(0,255,0),false,0);

               psur->Ellipse2(pdp->m_v.x, pdp->m_v.y, 8);
           }
       }
   }
   else
   {
       psur->SetBorderColor(RGB(0,180,0),false,1);

       psur->Line(m_d.m_vCenter.x - m_d.m_radius, m_d.m_vCenter.y, m_d.m_vCenter.x + m_d.m_radius, m_d.m_vCenter.y);
       psur->Line(m_d.m_vCenter.x, m_d.m_vCenter.y - m_d.m_radius, m_d.m_vCenter.x, m_d.m_vCenter.y + m_d.m_radius);

       const float r2 = m_d.m_radius * (float)sin(M_PI/4.0);

       psur->Line(m_d.m_vCenter.x - r2, m_d.m_vCenter.y - r2, m_d.m_vCenter.x + r2, m_d.m_vCenter.y + r2);
       psur->Line(m_d.m_vCenter.x - r2, m_d.m_vCenter.y + r2, m_d.m_vCenter.x + r2, m_d.m_vCenter.y - r2);
   }

   if( m_d.m_shape==TriggerWire )
   {
       if (numFaces > 0)
       {
           const size_t numPts = numFaces / 3 + 1;
           std::vector<Vertex2D> drawVertices(numPts);

           const Vertex3Ds& A = vertices[faceIndices[0]];
           drawVertices[0] = Vertex2D(A.x,A.y);

		   size_t o = 1;
           for (int i=0; i<numFaces; i+=3,++o)
           {
              const Vertex3Ds& B = vertices[faceIndices[i+1]];
              drawVertices[o] = Vertex2D(B.x,B.y);
           }

           psur->Polyline(&drawVertices[0], drawVertices.size());
       }
   }
}

void Trigger::RenderBlueprint(Sur *psur)
{
   psur->SetFillColor(-1);
   psur->SetBorderColor(RGB(0,0,0), false, 0);
   psur->SetObject(this);

   psur->Ellipse(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_radius);
}

void Trigger::GetTimers(Vector<HitTimer> * const pvht)
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

void Trigger::GetHitShapes(Vector<HitObject> * const pvho)
{
   m_hitEnabled = m_d.m_fEnabled;

   if (m_d.m_shape==TriggerStar )
   {
      const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);

      m_ptriggerhitcircle = new TriggerHitCircle();

      m_ptriggerhitcircle->m_fEnabled = m_d.m_fEnabled;
      m_ptriggerhitcircle->m_ObjType = eTrigger;
      m_ptriggerhitcircle->m_pObj = (void*) this;

      m_ptriggerhitcircle->m_pfe = NULL;

      m_ptriggerhitcircle->center.x = m_d.m_vCenter.x;
      m_ptriggerhitcircle->center.y = m_d.m_vCenter.y;
      m_ptriggerhitcircle->radius = m_d.m_radius;
      m_ptriggerhitcircle->zlow = height;
      m_ptriggerhitcircle->zhigh = height + m_d.m_hit_height;

      m_ptriggerhitcircle->m_ptrigger = this;

      pvho->AddElement(m_ptriggerhitcircle);
   }
   else
      CurvesToShapes(pvho);
}

void Trigger::GetHitShapesDebug(Vector<HitObject> * const pvho)
{
   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);
   m_hitEnabled = m_d.m_fEnabled;
   switch (m_d.m_shape)
   {
   case TriggerStar:
      {
         HitObject * const pho = CreateCircularHitPoly(m_d.m_vCenter.x, m_d.m_vCenter.y, height + 10, m_d.m_radius, 32);
         pho->m_ObjType = eTrigger;
         pho->m_pObj = (void*)this;

         pvho->AddElement(pho);			
         break;
      }

   default:
   case TriggerWire:
      {
         std::vector<RenderVertex> vvertex;
         GetRgVertex(vvertex);

         const int cvertex = vvertex.size();
         Vertex3Ds * const rgv3d = new Vertex3Ds[cvertex];

         for (int i=0;i<cvertex;i++)
         {
            rgv3d[i].x = vvertex[i].x;
            rgv3d[i].y = vvertex[i].y;
            rgv3d[i].z = height + (float)(PHYS_SKIN*2.0);
         }

         Hit3DPoly * const ph3dp = new Hit3DPoly(rgv3d, cvertex);
         ph3dp->m_ObjType = eTrigger;
         ph3dp->m_pObj = (void*) this;

         pvho->AddElement(ph3dp);
         //ph3dp->m_fEnabled = false;	//!! disable hit process on polygon body, only trigger edges 
         break;
      }
   }
}

void Trigger::CurvesToShapes(Vector<HitObject> * const pvho)
{
    const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);
    std::vector<RenderVertex> vvertex;
    GetRgVertex(vvertex);

    const int count = vvertex.size();
    RenderVertex * const rgv = new RenderVertex[count];
    Vertex3Ds * const rgv3D = new Vertex3Ds[count];

    for (int i=0;i<count;i++)
    {
        rgv[i] = vvertex[i];
        rgv3D[i].x = rgv[i].x;
        rgv3D[i].y = rgv[i].y;
        rgv3D[i].z = height + (float)(PHYS_SKIN*2.0);
    }
#if 1	
    for (int i=0;i<count;i++)	
    {
        RenderVertex * const pv1 = &rgv[i];
        RenderVertex * const pv2 = &rgv[(i < count-1) ? (i+1) : 0];
        RenderVertex * const pv3 = &rgv[(i < count-2) ? (i+2) : (i+2-count)];

        AddLine(pvho, pv2, pv3, pv1, height);
    } 
#endif

#if 1	
    Hit3DPoly * const ph3dpoly = new Hit3DPoly(rgv3D,count);
    ph3dpoly->m_ObjType = eTrigger;
    ph3dpoly->m_pObj = (void*) this;

    pvho->AddElement(ph3dpoly);
#else
    delete [] rgv3D;
#endif

    delete [] rgv;
}

void Trigger::AddLine(Vector<HitObject> * const pvho, const RenderVertex * const pv1, const RenderVertex * const pv2, const RenderVertex * const pv3, const float height)
{
   TriggerLineSeg * const plineseg = new TriggerLineSeg();

   plineseg->m_ptrigger = this;
   plineseg->m_ObjType = eTrigger;
   plineseg->m_pObj = (void*) this;

   plineseg->m_rcHitRect.zlow = height;
   plineseg->m_rcHitRect.zhigh = height + m_d.m_hit_height - 8.0f; //adjust for same hit height as circular

   plineseg->v1.x = pv1->x;
   plineseg->v1.y = pv1->y;
   plineseg->v2.x = pv2->x;
   plineseg->v2.y = pv2->y;

   pvho->AddElement(plineseg);

   plineseg->CalcNormal();
}

void Trigger::EndPlay()
{
   IEditable::EndPlay();

   if( vertexBuffer )
   {
      vertexBuffer->release();
      vertexBuffer=0;
   }
   if( triggerIndexBuffer )
   {
       triggerIndexBuffer->release();
       triggerIndexBuffer=0;
   }
   if ( triggerVertices )
   {
       delete [] triggerVertices;
       triggerVertices=0;
   }
   m_ptriggerhitcircle = NULL;
}

void Trigger::TriggerAnimationHit()
{
    hitEvent=true;
}

void Trigger::TriggerAnimationUnhit()
{
    unhitEvent=true;
}

void Trigger::PostRenderStatic(RenderDevice* pd3dDevice)
{
    const U32 old_time_msec = (m_d.m_time_msec < g_pplayer->m_time_msec) ? m_d.m_time_msec : g_pplayer->m_time_msec;
    m_d.m_time_msec = g_pplayer->m_time_msec;
    const float diff_time_msec = (float)(g_pplayer->m_time_msec-old_time_msec);

	if (!m_d.m_fVisible || m_d.m_shape==TriggerNone)
        return;
    
	const float animLimit = ( m_d.m_shape==TriggerStar ) ? m_d.m_radius/4.7f : 29.0f;
	const float limit = animLimit*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];

    pd3dDevice->basicShader->SetTechnique("basic_without_texture");

    Material *mat = m_ptable->GetMaterial(m_d.m_szMaterial);
    pd3dDevice->basicShader->SetMaterial(mat);

    pd3dDevice->SetRenderState(RenderDevice::DEPTHBIAS, 0);
    pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, TRUE);
    pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_CCW);

    if ( hitEvent )
    {
        doAnimation=true;
        hitEvent=false;
        unhitEvent=false;
        animHeightOffset=0.0f;
        moveDown=true;
    }
    if ( unhitEvent )
    {
        doAnimation=true;
        unhitEvent=false;
        hitEvent=false;
        animHeightOffset=limit;
        moveDown=false;
    }

    if (doAnimation)
    {
        float step = diff_time_msec*m_d.m_animSpeed*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];
		if(moveDown)
		    step = -step;
		animHeightOffset+=step;

        if( moveDown ) 
        {            
            if( animHeightOffset<=-limit )
            {
                animHeightOffset=-limit;
                doAnimation=false;
            }
        }
        else
        {
            if( animHeightOffset>=0.0f )
            {
                animHeightOffset=0.0f;
                doAnimation=false;
            }
        }

        Vertex3D_NoTex2 *buf;
        vertexBuffer->lock(0, 0, (void**)&buf, VertexBuffer::DISCARDCONTENTS);
        for( int i=0;i<numVertices;i++ )
        {
            buf[i].x = triggerVertices[i].x;
            buf[i].y = triggerVertices[i].y;
            buf[i].z = triggerVertices[i].z+animHeightOffset;
            buf[i].nx = triggerVertices[i].nx;
            buf[i].ny = triggerVertices[i].ny;
            buf[i].nz = triggerVertices[i].nz;
            buf[i].tu = triggerVertices[i].tu;
            buf[i].tv = triggerVertices[i].tv;
        }
        vertexBuffer->unlock();
    }

    pd3dDevice->basicShader->Begin(0);
    pd3dDevice->DrawIndexedPrimitiveVB( D3DPT_TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, vertexBuffer, 0, numVertices, triggerIndexBuffer, 0, numFaces );
    pd3dDevice->basicShader->End();
}

void Trigger::RenderSetup(RenderDevice* pd3dDevice)
{
   m_d.m_time_msec = g_pplayer->m_time_msec;

   hitEvent = false;
   unhitEvent = false;
   doAnimation = false;
   moveDown = false;
   animHeightOffset=0.0f;

   if (!m_d.m_fVisible || m_d.m_shape==TriggerNone)
      return;

   Vertex3D_NoTex2 *verts;
   if (m_d.m_shape==TriggerWire)
   {
       numVertices = triggerSimpleNumVertices;
       numFaces = triggerSimpleNumFaces;
       verts = triggerSimple;
	   if ( triggerVertices )
		   delete [] triggerVertices;
       triggerVertices = new Vertex3D_NoTex2[numVertices];
   }
   else if (m_d.m_shape==TriggerStar)
   {
       numVertices = triggerStarNumVertices;
       numFaces = triggerStarNumFaces;
       verts = triggerStar;
	   if ( triggerVertices )
	      delete [] triggerVertices;
       triggerVertices = new Vertex3D_NoTex2[numVertices];
   }

   Pin3D * const ppin3d = &g_pplayer->m_pin3d;
   const float baseHeight = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);

   if (triggerIndexBuffer)
       triggerIndexBuffer->release();
   triggerIndexBuffer = pd3dDevice->CreateAndFillIndexBuffer(numFaces, (m_d.m_shape==TriggerWire) ? triggerSimpleIndices : triggerStarIndices);

   if (vertexBuffer)
	   vertexBuffer->release();
   ppin3d->m_pd3dDevice->CreateVertexBuffer( numVertices, USAGE_DYNAMIC, MY_D3DFVF_NOTEX2_VERTEX, &vertexBuffer );
   NumVideoBytes += numVertices*sizeof(Vertex3D_NoTex2);

   Matrix3D fullMatrix;
   fullMatrix.RotateZMatrix(ANGTORAD(m_d.m_rotation));

   for( int i=0;i<numVertices;i++ )
   {
       Vertex3Ds vert(verts[i].x,verts[i].y,verts[i].z);
       vert = fullMatrix.MultiplyVector(vert);

       if ( m_d.m_shape!=TriggerStar )
       {
           triggerVertices[i].x = (vert.x*m_d.m_scaleX)+m_d.m_vCenter.x;
           triggerVertices[i].y = (vert.y*m_d.m_scaleY)+m_d.m_vCenter.y;
           triggerVertices[i].z = (vert.z*1.0f*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set])+baseHeight;
       }
       else
       {
           triggerVertices[i].x = (vert.x*m_d.m_radius)+m_d.m_vCenter.x;
           triggerVertices[i].y = (vert.y*m_d.m_radius)+m_d.m_vCenter.y;
           triggerVertices[i].z = (vert.z*m_d.m_radius*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set])+baseHeight;
       }
       vert = Vertex3Ds( verts[i].nx, verts[i].ny, verts[i].nz );
       vert = fullMatrix.MultiplyVectorNoTranslate(vert);
       triggerVertices[i].nx = vert.x;
       triggerVertices[i].ny = vert.y;
       triggerVertices[i].nz = vert.z;
       triggerVertices[i].tu = verts[i].tu;
       triggerVertices[i].tv = verts[i].tv;
   }

   Vertex3D_NoTex2 *buf;
   vertexBuffer->lock(0,0,(void**)&buf, VertexBuffer::WRITEONLY);
   memcpy( buf, triggerVertices, sizeof(Vertex3D_NoTex2)*numVertices );
   vertexBuffer->unlock();
}

void Trigger::RenderStatic(RenderDevice* pd3dDevice)
{
}

void Trigger::SetObjectPos()
{
   g_pvp->SetObjectPosCur(m_d.m_vCenter.x, m_d.m_vCenter.y);
}

void Trigger::MoveOffset(const float dx, const float dy)
{
   m_d.m_vCenter.x += dx;
   m_d.m_vCenter.y += dy;

   for (int i=0;i<m_vdpoint.size();i++)
   {
      CComObject<DragPoint> * const pdp = m_vdpoint.ElementAt(i);

      pdp->m_v.x += dx;
      pdp->m_v.y += dy;
   }
   UpdateEditorView();
   m_ptable->SetDirtyDraw();
}

void Trigger::GetPointCenter(Vertex2D * const pv) const
{
   *pv = m_d.m_vCenter;
}

void Trigger::PutPointCenter(const Vertex2D * const pv)
{
   m_d.m_vCenter = *pv;

   SetDirtyDraw();
}

void Trigger::EditMenu(HMENU hmenu)
{
   EnableMenuItem(hmenu, ID_WALLMENU_FLIP, MF_BYCOMMAND | MF_ENABLED);
   EnableMenuItem(hmenu, ID_WALLMENU_MIRROR, MF_BYCOMMAND | MF_ENABLED);
   EnableMenuItem(hmenu, ID_WALLMENU_ROTATE, MF_BYCOMMAND | MF_ENABLED);
   EnableMenuItem(hmenu, ID_WALLMENU_SCALE, MF_BYCOMMAND | MF_ENABLED);
   EnableMenuItem(hmenu, ID_WALLMENU_ADDPOINT, MF_BYCOMMAND | MF_ENABLED);
}

void Trigger::DoCommand(int icmd, int x, int y)
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

         const Vertex2D v = m_ptable->TransformPoint(x, y);

         std::vector<RenderVertex> vvertex;
         GetRgVertex(vvertex);

         int iSeg;
         Vertex2D vOut;
         ClosestPointOnPolygon(vvertex, v, &vOut, &iSeg, true);

         // Go through vertices (including iSeg itself) counting control points until iSeg
         int icp = 0;
         for (int i=0;i<(iSeg+1);i++)
            if (vvertex[i].fControlPoint)
               icp++;

         //if (icp == 0) // need to add point after the last point
         //icp = m_vdpoint.size();

         CComObject<DragPoint> *pdp;
         CComObject<DragPoint>::CreateInstance(&pdp);
         if (pdp)
         {
            pdp->AddRef();
            pdp->Init(this, vOut.x, vOut.y);
            m_vdpoint.InsertElementAt(pdp, icp); // push the second point forward, and replace it with this one.  Should work when index2 wraps.
         }

         SetDirtyDraw();

         STOPUNDO
      }
      break;
   }
}

void Trigger::FlipY(Vertex2D * const pvCenter)
{
    if( m_d.m_shape==TriggerNone )
       IHaveDragPoints::FlipPointY(pvCenter);
}

void Trigger::FlipX(Vertex2D * const pvCenter)
{
   if( m_d.m_shape==TriggerNone )
      IHaveDragPoints::FlipPointX(pvCenter);
}

void Trigger::Rotate(float ang, Vertex2D *pvCenter)
{
   if( m_d.m_shape==TriggerNone )
       IHaveDragPoints::RotatePoints(ang, pvCenter);
   else
   {
       GetIEditable()->BeginUndo();
       GetIEditable()->MarkForUndo();
       m_d.m_rotation=ang;
       GetIEditable()->EndUndo();
       UpdateEditorView();
       GetPTable()->SetDirtyDraw();
   }
}

void Trigger::Scale(float scalex, float scaley, Vertex2D *pvCenter)
{
    if( m_d.m_shape==TriggerNone )
       IHaveDragPoints::ScalePoints(scalex, scaley, pvCenter);
    else
    {
        GetIEditable()->BeginUndo();
        GetIEditable()->MarkForUndo();
        m_d.m_scaleX=scalex;
        m_d.m_scaleY=scaley;
        GetIEditable()->EndUndo();
        UpdateEditorView();
        GetPTable()->SetDirtyDraw();
  }
}

void Trigger::Translate(Vertex2D *pvOffset)
{
   if ( m_d.m_shape==TriggerNone)
      IHaveDragPoints::TranslatePoints(pvOffset);
   else
   {
      GetIEditable()->BeginUndo();
      GetIEditable()->MarkForUndo();
      MoveOffset(pvOffset->x, pvOffset->y);
      GetIEditable()->EndUndo();
      GetPTable()->SetDirtyDraw();
   }
}

HRESULT Trigger::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   BiffWriter bw(pstm, hcrypthash, hcryptkey);

   bw.WriteStruct(FID(VCEN), &m_d.m_vCenter, sizeof(Vertex2D));
   bw.WriteFloat(FID(RADI), m_d.m_radius);
   bw.WriteFloat(FID(ROTA), m_d.m_rotation);
   bw.WriteFloat(FID(SCAX), m_d.m_scaleX);
   bw.WriteFloat(FID(SCAY), m_d.m_scaleY);
   bw.WriteBool(FID(TMON), m_d.m_tdr.m_fTimerEnabled);
   bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
   bw.WriteString(FID(SURF), m_d.m_szSurface);
   bw.WriteString(FID(MATR), m_d.m_szMaterial);
   bw.WriteBool(FID(EBLD), m_d.m_fEnabled);
   bw.WriteBool(FID(VSBL), m_d.m_fVisible);
   bw.WriteFloat(FID(THOT), m_d.m_hit_height);
   bw.WriteWideString(FID(NAME), (WCHAR *)m_wzName);
   bw.WriteInt(FID(SHAP), m_d.m_shape);
   bw.WriteFloat(FID(ANSP), m_d.m_animSpeed);

   ISelect::SaveData(pstm, hcrypthash, hcryptkey);

   HRESULT hr;
   if(FAILED(hr = SavePointData(pstm, hcrypthash, hcryptkey)))
      return hr;

   bw.WriteTag(FID(ENDB));

   return S_OK;
}

void Trigger::ClearForOverwrite()
{
   ClearPointsForOverwrite();
}

void Trigger::WriteRegDefaults()
{
   SetRegValue("DefaultProps\\Trigger","TimerEnabled",REG_DWORD,&m_d.m_tdr.m_fTimerEnabled,4);
   SetRegValue("DefaultProps\\Trigger","TimerInterval", REG_DWORD, &m_d.m_tdr.m_TimerInterval, 4);
   SetRegValueBool("DefaultProps\\Trigger","Enabled",m_d.m_fEnabled);
   SetRegValueBool("DefaultProps\\Trigger","Visible",m_d.m_fVisible);
   SetRegValueFloat("DefaultProps\\Trigger","HitHeight", m_d.m_hit_height);
   SetRegValueFloat("DefaultProps\\Trigger","Radius", m_d.m_radius);
   SetRegValueFloat("DefaultProps\\Trigger","Rotation", m_d.m_rotation);
   SetRegValueFloat("DefaultProps\\Trigger","ScaleX", m_d.m_scaleX);
   SetRegValueFloat("DefaultProps\\Trigger","ScaleY", m_d.m_scaleY);
   SetRegValue("DefaultProps\\Trigger","Shape",REG_DWORD,&m_d.m_shape,4);
   SetRegValue("DefaultProps\\Trigger","Surface", REG_SZ, &m_d.m_szSurface,lstrlen(m_d.m_szSurface));
   SetRegValueFloat("DefaultProps\\Trigger","AnimSpeed", m_d.m_animSpeed);

}

HRESULT Trigger::InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   SetDefaults(false);

   BiffReader br(pstm, this, pid, version, hcrypthash, hcryptkey);

   m_ptable = ptable;

   br.Load();
   UpdateEditorView();
   return S_OK;
}

BOOL Trigger::LoadToken(int id, BiffReader *pbr)
{
   if (id == FID(PIID))
   {
      pbr->GetInt((int *)pbr->m_pdata);
   }
   else if (id == FID(VCEN))
   {
      pbr->GetStruct(&m_d.m_vCenter, sizeof(Vertex2D));
   }
   else if (id == FID(RADI))
   {
       pbr->GetFloat(&m_d.m_radius);
   }
   else if (id == FID(ROTA))
   {
       pbr->GetFloat(&m_d.m_rotation);
   }
   else if (id == FID(SCAX))
   {
       pbr->GetFloat(&m_d.m_scaleX);
   }
   else if (id == FID(SCAY))
   {
       pbr->GetFloat(&m_d.m_scaleY);
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
   else if (id == FID(SURF))
   {
      pbr->GetString(m_d.m_szSurface);
   }
   else if (id == FID(EBLD))
   {
      pbr->GetBool(&m_d.m_fEnabled);
   }
   else if (id == FID(THOT))
   {
      pbr->GetFloat(&m_d.m_hit_height);
   }
   else if (id == FID(VSBL))
   {
      pbr->GetBool(&m_d.m_fVisible);
   }
   else if (id == FID(SHAP))
   {
      pbr->GetInt(&m_d.m_shape);
   }
   else if (id == FID(ANSP))
   {
       pbr->GetFloat(&m_d.m_animSpeed);
   }
   else if (id == FID(NAME))
   {
      pbr->GetWideString((WCHAR *)m_wzName);
   }
   else
   {
      LoadPointToken(id, pbr, pbr->m_version);
      ISelect::LoadToken(id, pbr);
   }
   return fTrue;
}

HRESULT Trigger::InitPostLoad()
{
   UpdateEditorView();
   return S_OK;
}

STDMETHODIMP Trigger::InterfaceSupportsErrorInfo(REFIID riid)
{
   static const IID* arr[] =
   {
      &IID_ITrigger,
   };

   for (size_t i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
   {
      if (InlineIsEqualGUID(*arr[i],riid))
         return S_OK;
   }
   return S_FALSE;
}

STDMETHODIMP Trigger::get_Radius(float *pVal)
{
   *pVal = m_d.m_radius;

   return S_OK;
}

STDMETHODIMP Trigger::put_Radius(float newVal)
{
   STARTUNDO

      m_d.m_radius = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Trigger::get_X(float *pVal)
{
   *pVal = m_d.m_vCenter.x;

   return S_OK;
}

STDMETHODIMP Trigger::put_X(float newVal)
{
   STARTUNDO

      m_d.m_vCenter.x = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Trigger::get_Y(float *pVal)
{
   *pVal = m_d.m_vCenter.y;

   return S_OK;
}

STDMETHODIMP Trigger::put_Y(float newVal)
{
   STARTUNDO

      m_d.m_vCenter.y = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Trigger::get_Surface(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_d.m_szSurface, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Trigger::put_Surface(BSTR newVal)
{
   STARTUNDO

      WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szSurface, 32, NULL, NULL);

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Trigger::get_Enabled(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB((g_pplayer) ? m_hitEnabled : m_d.m_fEnabled);

   return S_OK;
}

STDMETHODIMP Trigger::put_Enabled(VARIANT_BOOL newVal)
{
   if (g_pplayer)
   {
      m_hitEnabled = VBTOF(newVal);

      if (m_ptriggerhitcircle) m_ptriggerhitcircle->m_fEnabled = m_hitEnabled;
   }
   else 
   {
      STARTUNDO

      m_d.m_fEnabled = VBTOF(newVal);
      m_hitEnabled = m_d.m_fEnabled;

      STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Trigger::get_Visible(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fVisible);

   return S_OK;
}

STDMETHODIMP Trigger::put_Visible(VARIANT_BOOL newVal)
{
   STARTUNDO

      m_d.m_fVisible = VBTOF(newVal);

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Trigger::BallCntOver(int *pVal)
{
   int cnt = 0;

   if (g_pplayer)
   {
      for (unsigned i = 0; i < g_pplayer->m_vball.size(); i++)
      {
         Ball * const pball = g_pplayer->m_vball[i];

         if (pball->m_vpVolObjs && pball->m_vpVolObjs->IndexOf(this) >= 0)
         {
            g_pplayer->m_pactiveball = pball;	// set active ball for scriptor
            ++cnt;
         }
      }		
   }

   *pVal = cnt;
   return S_OK;
}

STDMETHODIMP Trigger::DestroyBall(int *pVal)
{
   int cnt = 0;

   if (g_pplayer)
   {
      for (unsigned i = 0; i < g_pplayer->m_vball.size(); i++)
      {
         Ball * const pball = g_pplayer->m_vball[i];

         int j;
         if (pball->m_vpVolObjs && (j = pball->m_vpVolObjs->IndexOf(this)) >= 0)
         {
            ++cnt;
            pball->m_vpVolObjs->RemoveElementAt(j);
            g_pplayer->DestroyBall(pball); // inside trigger volume?
         }
      }
   }

   if (pVal) *pVal = cnt;

   return S_OK;
}

STDMETHODIMP Trigger::get_HitHeight(float *pVal)
{
    *pVal = m_d.m_hit_height;

    return S_OK;
}

STDMETHODIMP Trigger::put_HitHeight(float newVal)
{
    STARTUNDO

        m_d.m_hit_height = newVal;

    STOPUNDO

        return S_OK;
}

STDMETHODIMP Trigger::get_Rotation(float *pVal)
{
    *pVal = m_d.m_rotation;

    return S_OK;
}

STDMETHODIMP Trigger::put_Rotation(float newVal)
{
    STARTUNDO

        m_d.m_rotation = newVal;
        UpdateEditorView();

    STOPUNDO

        return S_OK;
}

STDMETHODIMP Trigger::get_AnimSpeed(float *pVal)
{
    *pVal = m_d.m_animSpeed;

    return S_OK;
}

STDMETHODIMP Trigger::put_AnimSpeed(float newVal)
{
    STARTUNDO

      m_d.m_animSpeed = newVal;

    STOPUNDO

        return S_OK;
}


STDMETHODIMP Trigger::get_Material(BSTR *pVal)
{
    WCHAR wz[512];

    MultiByteToWideChar(CP_ACP, 0, m_d.m_szMaterial, -1, wz, 32);
    *pVal = SysAllocString(wz);

    return S_OK;
}

STDMETHODIMP Trigger::put_Material(BSTR newVal)
{
    STARTUNDO
        WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szMaterial, 32, NULL, NULL);
    STOPUNDO

        return S_OK;
}

void Trigger::GetDialogPanes(Vector<PropertyPane> *pvproppane)
{
   PropertyPane *pproppane;

   pproppane = new PropertyPane(IDD_PROP_NAME, NULL);
   pvproppane->AddElement(pproppane);

   m_propVisual = new PropertyPane(IDD_PROPTRIGGER_VISUALS, IDS_VISUALS);
   pvproppane->AddElement(m_propVisual);

   pproppane = new PropertyPane(IDD_PROPLIGHT_POSITION, IDS_POSITION);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPTRIGGER_STATE, IDS_STATE);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROP_TIMER, IDS_MISC);
   pvproppane->AddElement(pproppane);
}

STDMETHODIMP Trigger::get_TriggerShape(TriggerShape *pVal)
{
   *pVal = m_d.m_shape;

   return S_OK;
}


STDMETHODIMP Trigger::put_TriggerShape(TriggerShape newVal)
{
   STARTUNDO
      m_d.m_shape = newVal;
   STOPUNDO
	   UpdateEditorView();
      return S_OK;
}

void Trigger::UpdatePropertyPanes()
{
    if ( m_propVisual==NULL )
        return;

    if ( m_d.m_shape==TriggerStar )
    {
        EnableWindow(GetDlgItem(m_propVisual->dialogHwnd,IDC_STAR_RADIUS_EDIT), TRUE);
        EnableWindow(GetDlgItem(m_propVisual->dialogHwnd,IDC_ROTATION_EDIT), TRUE);
        EnableWindow(GetDlgItem(m_propVisual->dialogHwnd,IDC_RINGSPEED_EDIT), TRUE);
    }
    else if( m_d.m_shape==TriggerWire)
    {
        EnableWindow(GetDlgItem(m_propVisual->dialogHwnd,IDC_STAR_RADIUS_EDIT), FALSE);
        EnableWindow(GetDlgItem(m_propVisual->dialogHwnd,IDC_ROTATION_EDIT), TRUE);
        EnableWindow(GetDlgItem(m_propVisual->dialogHwnd,IDC_RINGSPEED_EDIT), TRUE);
    }
    else
    {
        EnableWindow(GetDlgItem(m_propVisual->dialogHwnd,IDC_STAR_RADIUS_EDIT), FALSE);
        EnableWindow(GetDlgItem(m_propVisual->dialogHwnd,IDC_ROTATION_EDIT), FALSE);
        EnableWindow(GetDlgItem(m_propVisual->dialogHwnd,IDC_RINGSPEED_EDIT), FALSE);
    }
}
