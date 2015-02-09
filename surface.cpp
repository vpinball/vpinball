#include "StdAfx.h"

Surface::Surface()
{
   m_menuid = IDR_SURFACEMENU;

   m_d.m_szImage[0] = 0;
   m_d.m_szSideImage[0] = 0;

   m_d.m_fCollidable = true;
   m_d.m_fSlingshotAnimation = true;
   m_d.m_fInner = true;
   m_d.m_fIsBottomSolid = false;
   slingshotVBuffer=0;
   sideVBuffer = 0;
   topVBuffer = 0;
   sideIBuffer = 0;
   m_propPhysics = NULL;
}

Surface::~Surface()
{
    FreeBuffers();
}

bool Surface::IsTransparent()
{
    Material *mat = 0;
    bool result=false;
    if( m_d.m_fSideVisible)
    {
        mat = m_ptable->GetMaterial(m_d.m_szSideMaterial);
        result = mat->m_bOpacityActive;
    }
    if( m_d.m_fVisible )
    {
        mat=m_ptable->GetMaterial(m_d.m_szTopMaterial);
        result=mat->m_bOpacityActive;
    }
    return result;
}

HRESULT Surface::Init(PinTable *ptable, float x, float y, bool fromMouseClick)
{
   m_ptable = ptable;
   IsWall = true;
   float width = 50.0f, length = 50.0f, fTmp;

   HRESULT hr = GetRegStringAsFloat("DefaultProps\\Wall", "Width", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      width = fTmp;

   hr = GetRegStringAsFloat("DefaultProps\\Wall", "Length", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      length = fTmp;

   CComObject<DragPoint> *pdp;
   CComObject<DragPoint>::CreateInstance(&pdp);
   if (pdp)
   {
      pdp->AddRef();
      pdp->Init(this, x-width, y-length);
      m_vdpoint.AddElement(pdp);
   }
   CComObject<DragPoint>::CreateInstance(&pdp);
   if (pdp)
   {
      pdp->AddRef();
      pdp->Init(this, x-width, y+length);
      m_vdpoint.AddElement(pdp);
   }
   CComObject<DragPoint>::CreateInstance(&pdp);
   if (pdp)
   {
      pdp->AddRef();
      pdp->Init(this, x+width, y+length);
      m_vdpoint.AddElement(pdp);
   }
   CComObject<DragPoint>::CreateInstance(&pdp);
   if (pdp)
   {
      pdp->AddRef();
      pdp->Init(this, x+width, y-length);
      m_vdpoint.AddElement(pdp);
   }

   SetDefaults(fromMouseClick);

   return InitVBA(fTrue, 0, NULL);
}

void Surface::WriteRegDefaults()
{
   const char * strKeyName = IsWall ? "DefaultProps\\Wall" : "DefaultProps\\Target";

   SetRegValueBool(strKeyName,"TimerEnabled", !!m_d.m_tdr.m_fTimerEnabled);
   SetRegValueInt(strKeyName,"TimerInterval", m_d.m_tdr.m_TimerInterval);
   SetRegValueBool(strKeyName,"HitEvent", m_d.m_fHitEvent);
   SetRegValueFloat(strKeyName,"HitThreshold", m_d.m_threshold);
   SetRegValueFloat(strKeyName,"SlingshotThreshold", m_d.m_slingshot_threshold);
   SetRegValueString(strKeyName,"TopImage", m_d.m_szImage);
   SetRegValueString(strKeyName,"SideImage", m_d.m_szSideImage);
   SetRegValueBool(strKeyName,"Droppable", m_d.m_fDroppable);
   SetRegValueBool(strKeyName,"Flipbook", m_d.m_fFlipbook);
   SetRegValueBool(strKeyName,"IsBottomSolid", m_d.m_fIsBottomSolid);
   SetRegValueFloat(strKeyName,"HeightBottom", m_d.m_heightbottom);
   SetRegValueFloat(strKeyName,"HeightTop", m_d.m_heighttop);
   SetRegValueBool(strKeyName,"DisplayTexture", m_d.m_fDisplayTexture);
   SetRegValueFloat(strKeyName,"SlingshotForce", m_d.m_slingshotforce);
   SetRegValueBool(strKeyName,"SlingshotAnimation", m_d.m_fSlingshotAnimation);
   SetRegValueFloat(strKeyName,"Elasticity", m_d.m_elasticity);
   SetRegValueFloat(strKeyName,"Friction", m_d.m_friction);
   SetRegValueFloat(strKeyName,"Scatter", m_d.m_scatter);
   SetRegValueBool(strKeyName,"Visible", m_d.m_fVisible);
   SetRegValueBool(strKeyName,"SideVisible", m_d.m_fSideVisible);
   SetRegValueBool(strKeyName,"Collidable", m_d.m_fCollidable);
}


HRESULT Surface::InitTarget(PinTable * const ptable, const float x, const float y, bool fromMouseClick)
{
   static const char strKeyName[] = "DefaultProps\\Target";

   m_ptable = ptable;
   IsWall = false;
   float width = 30.0f, length=6.0f, fTmp;

   HRESULT hr = GetRegStringAsFloat(strKeyName, "Width", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      width = fTmp;

   hr = GetRegStringAsFloat(strKeyName, "Length", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      length = fTmp;

   CComObject<DragPoint> *pdp;
   CComObject<DragPoint>::CreateInstance(&pdp);
   if (pdp)
   {
      pdp->AddRef();
      pdp->Init(this, x-width, y-length);
      m_vdpoint.AddElement(pdp);
   }
   CComObject<DragPoint>::CreateInstance(&pdp);
   if (pdp)
   {
      pdp->AddRef();
      pdp->Init(this, x-width, y+length);
      pdp->m_fAutoTexture = fFalse;
      pdp->m_texturecoord = 0.0f;
      m_vdpoint.AddElement(pdp);
   }
   CComObject<DragPoint>::CreateInstance(&pdp);
   if (pdp)
   {
      pdp->AddRef();
      pdp->Init(this, x+width, y+length);
      pdp->m_fAutoTexture = fFalse;
      pdp->m_texturecoord = 1.0f;
      m_vdpoint.AddElement(pdp);
   }
   CComObject<DragPoint>::CreateInstance(&pdp);
   if (pdp)
   {
      pdp->AddRef();
      pdp->Init(this, x+30.0f, y-6.0f);
      m_vdpoint.AddElement(pdp);
   }

   //SetDefaults();
   //Set seperate defaults for targets (SetDefaults sets the Wall defaults)

   m_d.m_tdr.m_fTimerEnabled = fromMouseClick ? GetRegBoolWithDefault(strKeyName,"TimerEnabled", false) : false;
   m_d.m_tdr.m_TimerInterval = fromMouseClick ? GetRegIntWithDefault(strKeyName,"TimerInterval", 100) : 100;
   m_d.m_fHitEvent = fromMouseClick ? GetRegBoolWithDefault(strKeyName,"HitEvent", true) : true;
   m_d.m_threshold = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName,"HitThreshold", 2.0f) : 2.0f;
   m_d.m_slingshot_threshold = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName,"SlingshotThreshold", 0.0f) : 0.0f;
   m_d.m_fInner = true; //!! Deprecated, do not use anymore

   hr = GetRegString(strKeyName,"TopImage", m_d.m_szImage, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szImage[0] = 0;

   hr = GetRegString(strKeyName,"SideImage", m_d.m_szSideImage, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szSideImage[0] = 0;

   m_d.m_fDroppable = fromMouseClick ? GetRegBoolWithDefault(strKeyName,"Droppable", false) : false;
   m_d.m_fFlipbook = fromMouseClick ? GetRegBoolWithDefault(strKeyName,"Flipbook", false) : false;
   m_d.m_fIsBottomSolid = fromMouseClick ? GetRegBoolWithDefault(strKeyName,"IsBottomSolid", true) : false;

   m_d.m_heightbottom = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName,"HeightBottom", 0.0f) : 0.0f;
   m_d.m_heighttop = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName,"HeightTop", 50.0f) : 50.0f;

   m_d.m_fDisplayTexture = fromMouseClick ? GetRegBoolWithDefault(strKeyName,"DisplayTexture", false) : false;
   m_d.m_slingshotforce = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName,"SlingshotForce", 80.0f) : 80.0f;
   m_d.m_fSlingshotAnimation = fromMouseClick ? GetRegBoolWithDefault(strKeyName,"SlingshotAnimation", true) : true;

   m_d.m_elasticity = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName,"Elasticity", 0.3f) : 0.3f;
   m_d.m_friction = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName,"Friction", 0.3f) : 0.3f;
   m_d.m_scatter = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName,"Scatter", 0) : 0;

   m_d.m_fVisible = fromMouseClick ? GetRegBoolWithDefault(strKeyName,"Visible", true) : true;
   m_d.m_fSideVisible = fromMouseClick ? GetRegBoolWithDefault(strKeyName,"SideVisible", true) : true;
   m_d.m_fCollidable = fromMouseClick ? GetRegBoolWithDefault(strKeyName,"Collidable", true) : true;

   return InitVBA(fTrue, 0, NULL);
}

void Surface::SetDefaults(bool fromMouseClick)
{
   static const char strKeyName[] = "DefaultProps\\Wall";

   HRESULT hr;

   m_d.m_tdr.m_fTimerEnabled = fromMouseClick ? GetRegBoolWithDefault(strKeyName,"TimerEnabled", false) : false;
   m_d.m_tdr.m_TimerInterval = fromMouseClick ? GetRegIntWithDefault(strKeyName,"TimerInterval", 100) : 100;
   m_d.m_fHitEvent = fromMouseClick ? GetRegBoolWithDefault(strKeyName,"HitEvent", false) : false;
   m_d.m_threshold = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName,"HitThreshold", 2.0f) : 2.0f;
   m_d.m_slingshot_threshold = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName,"SlingshotThreshold", 0.0f) : 0.0f;
   m_d.m_fInner = true; //!! Deprecated, do not use anymore

   hr = GetRegString(strKeyName,"TopImage", m_d.m_szImage, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szImage[0] = 0;

   hr = GetRegString(strKeyName,"SideImage", m_d.m_szSideImage, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szSideImage[0] = 0;

   m_d.m_fDroppable = fromMouseClick ? GetRegBoolWithDefault(strKeyName,"Droppable", false) : false;
   m_d.m_fFlipbook = fromMouseClick ? GetRegBoolWithDefault(strKeyName,"Flipbook", false) : false;
   m_d.m_fIsBottomSolid = fromMouseClick ? GetRegBoolWithDefault(strKeyName,"IsBottomSolid", true) : false;

   m_d.m_heightbottom = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName,"HeightBottom", 0.0f) : 0.0f;
   m_d.m_heighttop = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName,"HeightTop", 50.0f) : 50.0f;

   m_d.m_fDisplayTexture = fromMouseClick ? GetRegBoolWithDefault(strKeyName,"DisplayTexture", false) : false;
   m_d.m_slingshotforce = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName,"SlingshotForce", 80.0f) : 80.0f;
   m_d.m_fSlingshotAnimation = fromMouseClick ? GetRegBoolWithDefault(strKeyName,"SlingshotAnimation", true) : true;

   m_d.m_elasticity = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName,"Elasticity", 0.3f) : 0.3f;
   m_d.m_friction = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName,"Friction", 0.3f) : 0.3f;
   m_d.m_scatter = fromMouseClick ? GetRegStringAsFloatWithDefault(strKeyName,"Scatter", 0) : 0;

   m_d.m_fVisible = fromMouseClick ? GetRegBoolWithDefault(strKeyName,"Visible", true) : true;
   m_d.m_fSideVisible = fromMouseClick ? GetRegBoolWithDefault(strKeyName,"SideVisible", true) : true;
   m_d.m_fCollidable = fromMouseClick ? GetRegBoolWithDefault(strKeyName,"Collidable", true) : true;
}


void Surface::PreRender(Sur * const psur)
{
   psur->SetFillColor(m_ptable->RenderSolid() ? RGB(192,192,192) : -1);
   psur->SetObject(this);
   // Don't want border color to be over-ridden when selected - that will be drawn later
   psur->SetBorderColor(-1,false,0);

   std::vector<RenderVertex> vvertex;
   GetRgVertex(vvertex);

   Texture *ppi;
   if (m_ptable->RenderSolid() && m_d.m_fDisplayTexture && (ppi = m_ptable->GetImage(m_d.m_szImage)))
   {
      ppi->EnsureHBitmap();
      if (ppi->m_hbmGDIVersion)
         psur->PolygonImage(vvertex, ppi->m_hbmGDIVersion, m_ptable->m_left, m_ptable->m_top, m_ptable->m_right, m_ptable->m_bottom, ppi->m_width, ppi->m_height);
      else
      {
         // Do nothing for now to indicate to user that there is a problem
      }
   }
   else
      psur->Polygon(vvertex);
}

void Surface::Render(Sur * const psur)
{
   psur->SetFillColor(-1);
   psur->SetBorderColor(RGB(0,0,0),false,0);
   psur->SetObject(this); // For selected formatting
   psur->SetObject(NULL);

   {
       std::vector<RenderVertex> vvertex; //!! check/reuse from prerender
       GetRgVertex(vvertex);
       psur->Polygon(vvertex);
   }

   // if the item is selected then draw the dragpoints (or if we are always to draw dragpoints)
   bool fDrawDragpoints = ( (m_selectstate != eNotSelected) || g_pvp->m_fAlwaysDrawDragPoints );

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

   for (int i=0;i<m_vdpoint.Size();i++)
   {
      CComObject<DragPoint> * const pdp = m_vdpoint.ElementAt(i);
      psur->SetFillColor(-1);
      psur->SetBorderColor(RGB(255,0,0),false,0);

      if (pdp->m_fDragging)
      {
         //psur->SetFillColor(RGB(0,255,0));
         psur->SetBorderColor(RGB(0,255,0),false,0);
      }

      if (fDrawDragpoints)
      {
         psur->SetObject(pdp);
         psur->Ellipse2(pdp->m_v.x, pdp->m_v.y, 8);
      }

      if (pdp->m_fSlingshot)
      {
         psur->SetObject(NULL);
         const CComObject<DragPoint> * const pdp2 = m_vdpoint.ElementAt((i < m_vdpoint.Size()-1) ? (i+1) : 0);

         psur->SetLineColor(RGB(0,0,0),false,3);
         psur->Line(pdp->m_v.x, pdp->m_v.y, pdp2->m_v.x, pdp2->m_v.y);
      }
   }
}

void Surface::RenderBlueprint(Sur *psur)
{
   // Don't render dragpoints for blueprint
   psur->SetFillColor(-1);
   psur->SetBorderColor(RGB(0,0,0),false,0);
   psur->SetObject(this); // For selected formatting
   psur->SetObject(NULL);

   std::vector<RenderVertex> vvertex;
   GetRgVertex(vvertex);

   psur->Polygon(vvertex);
}

void Surface::GetTimers(Vector<HitTimer> * const pvht)
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

void Surface::GetHitShapes(Vector<HitObject> * const pvho)
{
   CurvesToShapes(pvho);

   m_fIsDropped = fFalse;
   m_fDisabled = false;
}

void Surface::GetHitShapesDebug(Vector<HitObject> * const pvho)
{
}

void Surface::CurvesToShapes(Vector<HitObject> * const pvho)
{
   std::vector<RenderVertex> vvertex;
   GetRgVertex(vvertex);

   const int count = vvertex.size();
   Vertex3Ds * const rgv3Dt = new Vertex3Ds[count];
   Vertex3Ds * const rgv3Db = m_d.m_fIsBottomSolid ? new Vertex3Ds[count] : NULL;

   for (int i = 0; i < count; ++i)
   {
      const RenderVertex * const pv1 = &vvertex[i];

      rgv3Dt[i].x = pv1->x;
      rgv3Dt[i].y = pv1->y;
      rgv3Dt[i].z = m_d.m_heighttop+m_ptable->m_tableheight;

	  if(m_d.m_fIsBottomSolid)
	  {
		  rgv3Db[count-1-i].x = pv1->x;
		  rgv3Db[count-1-i].y = pv1->y;
		  rgv3Db[count-1-i].z = m_d.m_heightbottom+m_ptable->m_tableheight;
	  }

      const RenderVertex * const pv2 = &vvertex[(i + 1) % count];
      const RenderVertex * const pv3 = &vvertex[(i + 2) % count];

      AddLine(pvho, pv2, pv3, pv2->fSlingshot);
   }

   Hit3DPoly * const ph3dpolyt = new Hit3DPoly(rgv3Dt,count);
   SetupHitObject(pvho, ph3dpolyt);
   
   if(m_d.m_fIsBottomSolid)
   {
	   Hit3DPoly * const ph3dpolyb = new Hit3DPoly(rgv3Db,count);
	   SetupHitObject(pvho, ph3dpolyb);
   }
}

void Surface::SetupHitObject(Vector<HitObject> * pvho, HitObject * obj)
{
    obj->m_elasticity = m_d.m_elasticity;
    obj->SetFriction(m_d.m_friction);
    obj->m_scatter = ANGTORAD(m_d.m_scatter);
    obj->m_fEnabled = m_d.m_fCollidable;

    if (m_d.m_fHitEvent)
    {
        obj->m_pfe = (IFireEvents *)this;
        obj->m_threshold = m_d.m_threshold;
    }

    pvho->AddElement(obj);
    m_vhoCollidable.push_back(obj);	//remember hit components of wall
    if (m_d.m_fDroppable)
        m_vhoDrop.push_back(obj);
}

void Surface::AddLine(Vector<HitObject> * const pvho, const RenderVertex * const pv1, const RenderVertex * const pv2, const bool fSlingshot)
{
   LineSeg *plineseg;

   if (!fSlingshot)
   {
      plineseg = new LineSeg(*pv1, *pv2);
   }
   else
   {
      LineSegSlingshot * const plinesling = new LineSegSlingshot(*pv1, *pv2);
      plineseg = (LineSeg *)plinesling;

      plinesling->m_force = m_d.m_slingshotforce;
      plinesling->m_psurface = this;

      m_vlinesling.push_back(plinesling);
   }

   plineseg->m_rcHitRect.zlow = m_d.m_heightbottom+m_ptable->m_tableheight;
   plineseg->m_rcHitRect.zhigh = m_d.m_heighttop+m_ptable->m_tableheight;

   SetupHitObject(pvho, plineseg);

   if (fSlingshot)  // slingshots always have hit events
   {
       plineseg->m_pfe = (IFireEvents *)this;
       plineseg->m_threshold = m_d.m_threshold;
   }

   if (m_d.m_heightbottom != 0.f)
   {
       // add lower edge as a line
       Vertex3Ds v1(pv1->x, pv1->y, m_d.m_heightbottom+m_ptable->m_tableheight);
       Vertex3Ds v2(pv2->x, pv2->y, m_d.m_heightbottom+m_ptable->m_tableheight);
       SetupHitObject(pvho, new HitLine3D(v1, v2));
   }
   {
       // add upper edge as a line
       Vertex3Ds v1(pv1->x, pv1->y, m_d.m_heighttop+m_ptable->m_tableheight);
       Vertex3Ds v2(pv2->x, pv2->y, m_d.m_heighttop+m_ptable->m_tableheight);
       SetupHitObject(pvho, new HitLine3D(v1, v2));
   }

   // create vertical joint between the two line segments
   {
       SetupHitObject(pvho, new HitLineZ(*pv1, m_d.m_heightbottom+m_ptable->m_tableheight, m_d.m_heighttop+m_ptable->m_tableheight));

       // add upper and lower end points of line
       if (m_d.m_heightbottom != 0)
           SetupHitObject(pvho, new HitPoint(Vertex3Ds(pv1->x, pv1->y, m_d.m_heightbottom+m_ptable->m_tableheight)));
       SetupHitObject(pvho, new HitPoint(Vertex3Ds(pv1->x, pv1->y, m_d.m_heighttop+m_ptable->m_tableheight)));
   }
}

void Surface::GetBoundingVertices(Vector<Vertex3Ds> * const pvvertex3D)
{
   for (int i=0;i<8;i++)
   {
      Vertex3Ds * const pv = new Vertex3Ds();
      pv->x = i&1 ? m_ptable->m_right : m_ptable->m_left;
      pv->y = i&2 ? m_ptable->m_bottom : m_ptable->m_top;
      pv->z = i&4 ? m_d.m_heighttop : m_d.m_heightbottom;

      pvvertex3D->AddElement(pv);
   }
}

void Surface::EndPlay()
{
   IEditable::EndPlay();

   m_vlinesling.clear();
   m_vhoDrop.clear();
   m_vhoCollidable.clear();

   FreeBuffers();
}

void Surface::MoveOffset(const float dx, const float dy)
{
   for (int i=0;i<m_vdpoint.Size();i++)
   {
      CComObject<DragPoint> * const pdp = m_vdpoint.ElementAt(i);

      pdp->m_v.x += dx;
      pdp->m_v.y += dy;
   }

   m_ptable->SetDirtyDraw();
}

void Surface::PostRenderStatic(RenderDevice* pd3dDevice)
{
    TRACE_FUNCTION();

    RenderSlingshots((RenderDevice*)pd3dDevice);
    if (m_d.m_fDroppable || isDynamic)
    {
        if (!m_fIsDropped)
        {
            // Render wall raised.
            RenderWallsAtHeight((RenderDevice*)pd3dDevice, fFalse);
        }
        else    // is dropped
        {
            // if this wall is part of flipbook animation, do not render when dropped
            if (!m_d.m_fFlipbook)
            {
                // Render wall dropped (smashed to a pancake at bottom height).
                RenderWallsAtHeight((RenderDevice*)pd3dDevice, fTrue);
            }
        }
    }
}

void Surface::PrepareWallsAtHeight( RenderDevice* pd3dDevice )
{
   std::vector<RenderVertex> vvertex;
   GetRgVertex(vvertex);
   float *rgtexcoord = NULL;

   Texture * const pinSide = m_ptable->GetImage(m_d.m_szSideImage);
   if (pinSide)
      GetTextureCoords(vvertex, &rgtexcoord);

   numVertices = vvertex.size();
   Vertex2D * const rgnormal = new Vertex2D[numVertices];

   for (int i=0;i<numVertices;i++)
   {
      const RenderVertex * const pv1 = &vvertex[i];
      const RenderVertex * const pv2 = &vvertex[(i < numVertices-1) ? (i+1) : 0];
      const float dx = pv1->x - pv2->x;
      const float dy = pv1->y - pv2->y;

      const float inv_len = 1.0f/sqrtf(dx*dx + dy*dy);

      rgnormal[i].x = dy*inv_len;
      rgnormal[i].y = dx*inv_len;
   }

   if ( sideVBuffer )
   {
      sideVBuffer->release();
      sideVBuffer=0;
   }
   pd3dDevice->CreateVertexBuffer( numVertices*4, 0, MY_D3DFVF_NOTEX2_VERTEX, &sideVBuffer );
   Vertex3D_NoTex2 *verts;
   sideVBuffer->lock( 0, 0, (void**)&verts, VertexBuffer::WRITEONLY);

   int offset=0;
   // Render side
   for (int i=0;i<numVertices;i++, offset+=4)
   {
      const RenderVertex * const pv1 = &vvertex[i];
      const RenderVertex * const pv2 = &vvertex[(i < numVertices-1) ? (i+1) : 0];

      const int a = (i == 0) ? (numVertices-1) : (i-1);
      const int c = (i < numVertices-1) ? (i+1) : 0;

      Vertex2D vnormal[2];
      if (pv1->fSmooth)
      {
         vnormal[0].x = (rgnormal[a].x + rgnormal[i].x)*0.5f;
         vnormal[0].y = (rgnormal[a].y + rgnormal[i].y)*0.5f;
      }
      else
      {
         vnormal[0].x = rgnormal[i].x;
         vnormal[0].y = rgnormal[i].y;
      }

      if (pv2->fSmooth)
      {
         vnormal[1].x = (rgnormal[i].x + rgnormal[c].x)*0.5f;
         vnormal[1].y = (rgnormal[i].y + rgnormal[c].y)*0.5f;
      }
      else
      {
         vnormal[1].x = rgnormal[i].x;
         vnormal[1].y = rgnormal[i].y;
      }

      vnormal[0].Normalize();
      vnormal[1].Normalize();

      {
         verts[offset].x=pv1->x;     verts[offset].y=pv1->y;     verts[offset].z=m_d.m_heightbottom+m_ptable->m_tableheight;
         verts[offset+1].x=pv1->x;   verts[offset+1].y=pv1->y;   verts[offset+1].z=m_d.m_heighttop+m_ptable->m_tableheight;
         verts[offset+2].x=pv2->x;   verts[offset+2].y=pv2->y;   verts[offset+2].z=m_d.m_heighttop+m_ptable->m_tableheight;
         verts[offset+3].x=pv2->x;   verts[offset+3].y=pv2->y;   verts[offset+3].z=m_d.m_heightbottom+m_ptable->m_tableheight;
         if (pinSide)
         {
            verts[offset].tu = rgtexcoord[i];
            verts[offset].tv = 1.0f;

            verts[offset+1].tu = rgtexcoord[i];
            verts[offset+1].tv = 0;

            verts[offset+2].tu = rgtexcoord[c];
            verts[offset+2].tv = 0;

            verts[offset+3].tu = rgtexcoord[c];
            verts[offset+3].tv = 1.0f;
         }

         verts[offset].nx = verts[offset+1].nx = vnormal[0].x;
         verts[offset].ny = verts[offset+1].ny = -vnormal[0].y;
         verts[offset].nz = verts[offset+1].nz = 0;

         verts[offset+2].nx = verts[offset+3].nx = vnormal[1].x;
         verts[offset+2].ny = verts[offset+3].ny = -vnormal[1].y;
         verts[offset+2].nz = verts[offset+3].nz = 0;
      }
   }
   delete[] rgnormal;

   sideVBuffer->unlock();

   // prepare index buffer for sides
   {
       std::vector<WORD> rgi(numVertices*6);

       int offset2=0;
       for (int i=0; i<numVertices; i++, offset2+=4)
       {
           rgi[i*6  ] = offset2;
           rgi[i*6+1] = offset2+1;
           rgi[i*6+2] = offset2+2;
           rgi[i*6+3] = offset2;
           rgi[i*6+4] = offset2+2;
           rgi[i*6+5] = offset2+3;
       }

       if (sideIBuffer)
           sideIBuffer->release();
       sideIBuffer = pd3dDevice->CreateAndFillIndexBuffer(rgi);
   }

   // draw top
   SAFE_VECTOR_DELETE(rgtexcoord);
   if (m_d.m_fVisible)      //!! BUG? Visible could still be set later if rendered dynamically?
   {
      VectorVoid vpoly;

      for (int i=0;i<numVertices;i++)
         vpoly.AddElement((void *)i);

      Vector<Triangle> vtri;
      PolygonToTriangles(vvertex, &vpoly, &vtri);

	  const float heightNotDropped = m_d.m_heighttop;
      const float heightDropped = (m_d.m_heightbottom + 0.1f);

      const float inv_tablewidth = 1.0f/(m_ptable->m_right - m_ptable->m_left);
      const float inv_tableheight = 1.0f/(m_ptable->m_bottom - m_ptable->m_top);

      numPolys = vtri.Size();
      if( numPolys==0 )
      {         
         // no polys to render leave vertex buffer undefined 
         return;
      }

      if( topVBuffer )
         topVBuffer->release();
      pd3dDevice->CreateVertexBuffer( 2*numPolys*3, 0, MY_D3DFVF_NOTEX2_VERTEX, &topVBuffer );

	  Vertex3D_NoTex2 *buf;
      topVBuffer->lock(0, 0, (void**)&buf, VertexBuffer::WRITEONLY);
      Vertex3D_NoTex2 * vertsTop[2];
	  vertsTop[0] = buf;
	  vertsTop[1] = buf + 3*numPolys;

      offset=0;
      for (int i=0;i<vtri.Size();i++, offset+=3)
      {
         const Triangle * const ptri = vtri.ElementAt(i);

         const RenderVertex * const pv0 = &vvertex[ptri->a];
         const RenderVertex * const pv1 = &vvertex[ptri->b];
         const RenderVertex * const pv2 = &vvertex[ptri->c];

         {
            vertsTop[0][offset].x=pv0->x;   vertsTop[0][offset].y=pv0->y;   vertsTop[0][offset].z=heightNotDropped+m_ptable->m_tableheight;
            vertsTop[0][offset+2].x=pv1->x;   vertsTop[0][offset+2].y=pv1->y;   vertsTop[0][offset+2].z=heightNotDropped+m_ptable->m_tableheight;
            vertsTop[0][offset+1].x=pv2->x;   vertsTop[0][offset+1].y=pv2->y;   vertsTop[0][offset+1].z=heightNotDropped+m_ptable->m_tableheight;

            vertsTop[0][offset].tu = vertsTop[0][offset].x *inv_tablewidth;
            vertsTop[0][offset].tv = vertsTop[0][offset].y *inv_tableheight;
            vertsTop[0][offset+1].tu = vertsTop[0][offset+1].x *inv_tablewidth;
            vertsTop[0][offset+1].tv = vertsTop[0][offset+1].y *inv_tableheight;
            vertsTop[0][offset+2].tu = vertsTop[0][offset+2].x *inv_tablewidth;
            vertsTop[0][offset+2].tv = vertsTop[0][offset+2].y *inv_tableheight;

            memcpy( &vertsTop[1][offset], &vertsTop[0][offset], sizeof(Vertex3D_NoTex2)*3 );
            vertsTop[1][offset].z = heightDropped;
            vertsTop[1][offset+1].z = heightDropped;
            vertsTop[1][offset+2].z = heightDropped;
            
            for (int l=offset;l<offset+3;l++)
            {
               vertsTop[0][l].nx = 0;
               vertsTop[0][l].ny = 0;
               vertsTop[0][l].nz = 1.0f;
               vertsTop[1][l].nx = 0;
               vertsTop[1][l].ny = 0;
               vertsTop[1][l].nz = 1.0f;
            }
         }
         delete vtri.ElementAt(i);
      }

	  topVBuffer->unlock();
   }
}

static const WORD rgisling[36] = {0,1,2,0,2,3, 4+0,4+1,4+2,4+0,4+2,4+3, 8+0,8+1,8+2,8+0,8+2,8+3, 12+0,12+1,12+2,12+0,12+2,12+3, 16+0,16+1,16+2,16+0,16+2,16+3, 20+0,20+1,20+2,20+0,20+2,20+3};
static const WORD rgiSlingshot0[4] = {0,1,4,3};
static const WORD rgiSlingshot1[4] = {1,2,5,4};
static const WORD rgiSlingshot2[4] = {0,3,4,1};
static const WORD rgiSlingshot3[4] = {1,4,5,2};
static const WORD rgiSlingshot4[4] = {3,9,10,4};
static const WORD rgiSlingshot5[4] = {4,10,11,5};

static IndexBuffer* slingIBuffer = NULL;        // this is constant so we only have one global instance

void Surface::PrepareSlingshots( RenderDevice *pd3dDevice )
{
   Pin3D * const ppin3d = &g_pplayer->m_pin3d;

   const float slingbottom = (m_d.m_heighttop - m_d.m_heightbottom) * 0.2f + m_d.m_heightbottom;
   const float slingtop    = (m_d.m_heighttop - m_d.m_heightbottom) * 0.8f + m_d.m_heightbottom;

   Vertex3D_NoTex2 *buf;
   slingshotVBuffer->lock(0,0,(void**)&buf, VertexBuffer::WRITEONLY);
   int offset=0;
   for (unsigned i=0; i<m_vlinesling.size(); i++)
   {
      LineSegSlingshot * const plinesling = m_vlinesling[i];
      plinesling->m_slingshotanim.m_fAnimations = (m_d.m_fSlingshotAnimation != 0);

      Vertex3D_NoTex2 rgv3D[12];
      rgv3D[0].x = plinesling->v1.x;
      rgv3D[0].y = plinesling->v1.y;
      rgv3D[0].z = slingbottom+m_ptable->m_tableheight;

      rgv3D[1].x = (plinesling->v1.x + plinesling->v2.x)*0.5f + plinesling->normal.x*(m_d.m_slingshotforce * 0.25f);//40;//20;
      rgv3D[1].y = (plinesling->v1.y + plinesling->v2.y)*0.5f + plinesling->normal.y*(m_d.m_slingshotforce * 0.25f);//20;
      rgv3D[1].z = slingbottom+m_ptable->m_tableheight;

      rgv3D[2].x = plinesling->v2.x;
      rgv3D[2].y = plinesling->v2.y;
      rgv3D[2].z = slingbottom+m_ptable->m_tableheight;

      for (int l=0;l<3;l++)
      {
         rgv3D[l+3].x = rgv3D[l].x;
         rgv3D[l+3].y = rgv3D[l].y;
         rgv3D[l+3].z = slingtop+m_ptable->m_tableheight;
      }

      for (int l=0;l<6;l++)
      {
         rgv3D[l+6].x = rgv3D[l].x - plinesling->normal.x*5.0f;
         rgv3D[l+6].y = rgv3D[l].y - plinesling->normal.y*5.0f;
         rgv3D[l+6].z = rgv3D[l].z;
      }

      SetNormal(rgv3D, rgiSlingshot0, 4);
      buf[offset++] = rgv3D[rgiSlingshot0[0]];
      buf[offset++] = rgv3D[rgiSlingshot0[1]];
      buf[offset++] = rgv3D[rgiSlingshot0[2]];
      buf[offset++] = rgv3D[rgiSlingshot0[3]];

      SetNormal(rgv3D, rgiSlingshot1, 4);
      buf[offset++] = rgv3D[rgiSlingshot1[0]];
      buf[offset++] = rgv3D[rgiSlingshot1[1]];
      buf[offset++] = rgv3D[rgiSlingshot1[2]];
      buf[offset++] = rgv3D[rgiSlingshot1[3]];

      SetNormal(rgv3D, rgiSlingshot2, 4);
      buf[offset++] = rgv3D[rgiSlingshot2[0]];
      buf[offset++] = rgv3D[rgiSlingshot2[1]];
      buf[offset++] = rgv3D[rgiSlingshot2[2]];
      buf[offset++] = rgv3D[rgiSlingshot2[3]];

      SetNormal(rgv3D, rgiSlingshot3, 4);
      buf[offset++] = rgv3D[rgiSlingshot3[0]];
      buf[offset++] = rgv3D[rgiSlingshot3[1]];
      buf[offset++] = rgv3D[rgiSlingshot3[2]];
      buf[offset++] = rgv3D[rgiSlingshot3[3]];

      SetNormal(rgv3D, rgiSlingshot4, 4);
      buf[offset++] = rgv3D[rgiSlingshot4[0]];
      buf[offset++] = rgv3D[rgiSlingshot4[1]];
      buf[offset++] = rgv3D[rgiSlingshot4[2]];
      buf[offset++] = rgv3D[rgiSlingshot4[3]];

      SetNormal(rgv3D, rgiSlingshot5, 4);
      buf[offset++] = rgv3D[rgiSlingshot5[0]];
      buf[offset++] = rgv3D[rgiSlingshot5[1]];
      buf[offset++] = rgv3D[rgiSlingshot5[2]];
      buf[offset++] = rgv3D[rgiSlingshot5[3]];
   }

   slingshotVBuffer->unlock();

   if (!slingIBuffer)
       slingIBuffer = pd3dDevice->CreateAndFillIndexBuffer(36, rgisling);
}

void Surface::RenderSetup(RenderDevice* pd3dDevice)
{
   const float oldBottomHeight = m_d.m_heightbottom;
   const float oldTopHeight = m_d.m_heighttop;

   m_d.m_heightbottom *= m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];
   m_d.m_heighttop *= m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];
   if( !m_vlinesling.empty() )
   {
      if( !slingshotVBuffer )
         pd3dDevice->CreateVertexBuffer(m_vlinesling.size()*24, 0, MY_D3DFVF_NOTEX2_VERTEX, &slingshotVBuffer);

      PrepareSlingshots(pd3dDevice);
   }

   Material *mat=0;
   isDynamic=false;
   if ( m_d.m_fSideVisible )
   {
      mat=m_ptable->GetMaterial(m_d.m_szSideMaterial);
      if( mat->m_bOpacityActive )
         isDynamic=true;
   }
   if( m_d.m_fVisible )
   {
      mat=m_ptable->GetMaterial(m_d.m_szTopMaterial);
      if( mat->m_bOpacityActive )
         isDynamic=true;
   }

   // create all vertices for dropped and non-dropped surface
   PrepareWallsAtHeight( pd3dDevice );
   m_d.m_heightbottom = oldBottomHeight;
   m_d.m_heighttop = oldTopHeight;
}

void Surface::FreeBuffers()
{
   if( slingshotVBuffer )
   {
      slingshotVBuffer->release();
      slingshotVBuffer=0;
   }
   if( sideVBuffer )
   {
      sideVBuffer->release();
      sideVBuffer=0;
   }
   if (sideIBuffer)
   {
       sideIBuffer->release();
       sideIBuffer = 0;
   }
   if( topVBuffer )
   {
      topVBuffer->release();
      topVBuffer=0;
   }
   if (slingIBuffer)    // NB: global instance
   {
       slingIBuffer->release();
       slingIBuffer = 0;
   }
}

void Surface::RenderStatic(RenderDevice* pd3dDevice)
{
   RenderSlingshots((RenderDevice*)pd3dDevice);
   if (!m_d.m_fDroppable && !isDynamic)
   {
       RenderWallsAtHeight((RenderDevice*)pd3dDevice, fFalse);
       g_pplayer->m_pin3d.SetTexture(NULL);
   }
}

void Surface::RenderSlingshots(RenderDevice* pd3dDevice)
{
   Pin3D * const ppin3d = &g_pplayer->m_pin3d;

   if ( ! m_d.m_fSideVisible )
      return;
   pd3dDevice->SetVertexDeclaration( pd3dDevice->m_pVertexNormalTexelDeclaration );

   Material *mat = m_ptable->GetMaterial( m_d.m_szSlingShotMaterial);
   pd3dDevice->basicShader->SetMaterial(mat);

   for (unsigned i=0; i<m_vlinesling.size(); i++)
   {
      LineSegSlingshot * const plinesling = m_vlinesling[i];
      if (plinesling->m_slingshotanim.m_iframe != 1)
          continue;

      pd3dDevice->basicShader->SetTechnique("basic_without_texture");

      pd3dDevice->basicShader->Begin(0);
      pd3dDevice->DrawIndexedPrimitiveVB( D3DPT_TRIANGLELIST, slingshotVBuffer, i*24, 24, slingIBuffer, 0, 36);
      pd3dDevice->basicShader->End();
   }
}

void Surface::RenderWallsAtHeight( RenderDevice* pd3dDevice, BOOL fDrop)
{
    pd3dDevice->SetVertexDeclaration( pd3dDevice->m_pVertexNormalTexelDeclaration );

    g_pplayer->m_pin3d.EnableAlphaBlend(1,false);
    pd3dDevice->basicShader->PerformAlphaTest(true);
    pd3dDevice->basicShader->SetAlphaTestValue(128.0f / 255.0f);

	// render side
    if (m_d.m_fSideVisible && !fDrop && (numVertices > 0)) // Don't need to render walls if dropped
    {
        Material *mat = m_ptable->GetMaterial( m_d.m_szSideMaterial);
        pd3dDevice->basicShader->SetMaterial(mat);
        if (mat->m_bOpacityActive)
        {
           pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_NONE);
        }
        else
        {
           pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_CCW);
        }

        Texture * const pinSide = m_ptable->GetImage(m_d.m_szSideImage);
        if (pinSide)
        {
            pd3dDevice->basicShader->SetTexture("Texture0",pinSide);
            pd3dDevice->basicShader->SetTechnique("basic_with_texture");

            //g_pplayer->m_pin3d.SetTextureFilter( ePictureTexture, TEXTURE_MODE_TRILINEAR );
        }
        else
        {
            pd3dDevice->basicShader->SetTechnique("basic_without_texture");
        }

        // combine drawcalls into one (hopefully faster)
        pd3dDevice->basicShader->Begin(0);
        pd3dDevice->DrawIndexedPrimitiveVB( D3DPT_TRIANGLELIST, sideVBuffer, 0, numVertices*4, sideIBuffer, 0, numVertices*6);
        pd3dDevice->basicShader->End();  
    }

    // render top
    if (m_d.m_fVisible && (numPolys > 0))
    {
       Material *mat = m_ptable->GetMaterial( m_d.m_szTopMaterial);
       pd3dDevice->basicShader->SetMaterial(mat);
       if (mat->m_bOpacityActive)
       {
          pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_NONE);
       }
       else
       {
          pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_CCW);
       }

	   Texture * const pin = m_ptable->GetImage(m_d.m_szImage);
       if (pin)
       {
          pd3dDevice->basicShader->SetTexture("Texture0",pin);  
          pd3dDevice->basicShader->SetTechnique("basic_with_texture");

          //g_pplayer->m_pin3d.SetTextureFilter( ePictureTexture, TEXTURE_MODE_TRILINEAR );
       }
       else
       {
          pd3dDevice->basicShader->SetTechnique("basic_without_texture");
       }

       pd3dDevice->basicShader->Begin(0);
       pd3dDevice->DrawPrimitiveVB( D3DPT_TRIANGLELIST, topVBuffer, !fDrop ? 0 : 3*numPolys, numPolys*3);
       pd3dDevice->basicShader->End();  
    }

    // reset render states
    g_pplayer->m_pin3d.DisableAlphaBlend();
    pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_CCW);
}

void Surface::DoCommand(int icmd, int x, int y)
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

         Vertex2D vOut;
         int iSeg;
         ClosestPointOnPolygon(vvertex, v, &vOut, &iSeg, true);

         // Go through vertices (including iSeg itself) counting control points until iSeg
         int icp = 0;
         for (int i=0;i<(iSeg+1);i++)
            if (vvertex[i].fControlPoint)
               icp++;

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

void Surface::FlipY(Vertex2D * const pvCenter)
{
   IHaveDragPoints::FlipPointY(pvCenter);
}

void Surface::FlipX(Vertex2D * const pvCenter)
{
   IHaveDragPoints::FlipPointX(pvCenter);
}

void Surface::Rotate(float ang, Vertex2D *pvCenter)
{
   IHaveDragPoints::RotatePoints(ang, pvCenter);
}

void Surface::Scale(float scalex, float scaley, Vertex2D *pvCenter)
{
   IHaveDragPoints::ScalePoints(scalex, scaley, pvCenter);
}

void Surface::Translate(Vertex2D *pvOffset)
{
   IHaveDragPoints::TranslatePoints(pvOffset);
}

HRESULT Surface::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   BiffWriter bw(pstm, hcrypthash, hcryptkey);

   bw.WriteBool(FID(HTEV), m_d.m_fHitEvent);
   bw.WriteBool(FID(DROP), m_d.m_fDroppable);
   bw.WriteBool(FID(FLIP), m_d.m_fFlipbook);
   bw.WriteBool(FID(ISBS), m_d.m_fIsBottomSolid);
   bw.WriteBool(FID(CLDW), m_d.m_fCollidable);
   bw.WriteBool(FID(TMON), m_d.m_tdr.m_fTimerEnabled);
   bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
   bw.WriteFloat(FID(THRS), m_d.m_threshold);
   bw.WriteString(FID(IMAG), m_d.m_szImage);
   bw.WriteString(FID(SIMG), m_d.m_szSideImage);
   bw.WriteString(FID(SIMA), m_d.m_szSideMaterial);
   bw.WriteString(FID(TOMA), m_d.m_szTopMaterial);
   bw.WriteString(FID(SLMA), m_d.m_szSlingShotMaterial);
   bw.WriteFloat(FID(HTBT), m_d.m_heightbottom);
   bw.WriteFloat(FID(HTTP), m_d.m_heighttop);
   //bw.WriteBool(FID(INNR), m_d.m_fInner); //!! Deprecated
   bw.WriteWideString(FID(NAME), (WCHAR *)m_wzName);
   bw.WriteBool(FID(DSPT), m_d.m_fDisplayTexture);
   bw.WriteFloat(FID(SLGF), m_d.m_slingshotforce);
   bw.WriteFloat(FID(SLTH), m_d.m_slingshot_threshold);
   bw.WriteFloat(FID(ELAS), m_d.m_elasticity);
   bw.WriteFloat(FID(WFCT), m_d.m_friction);
   bw.WriteFloat(FID(WSCT), m_d.m_scatter);
   bw.WriteBool(FID(VSBL), m_d.m_fVisible);
   bw.WriteBool(FID(SLGA), m_d.m_fSlingshotAnimation);
   bw.WriteBool(FID(SVBL), m_d.m_fSideVisible);

   ISelect::SaveData(pstm, hcrypthash, hcryptkey);

   bw.WriteTag(FID(PNTS));
   HRESULT hr;
   if(FAILED(hr = SavePointData(pstm, hcrypthash, hcryptkey)))
      return hr;

   bw.WriteTag(FID(ENDB));

   return S_OK;	
}

void Surface::ClearForOverwrite()
{
   ClearPointsForOverwrite();
}

HRESULT Surface::InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   SetDefaults(false);
   BiffReader br(pstm, this, pid, version, hcrypthash, hcryptkey);

   m_ptable = ptable;

   br.Load();

   // Pure backwards-compatibility code:
   // On some tables, the outer wall is still modelled/copy-pasted 'inside-out',
   // this tries to compensate for that
   if(!m_d.m_fInner) {
      const int cvertex = m_vdpoint.Size();

      float miny = FLT_MAX;
      int minyindex=0;

      // Find smallest y point - use it to connect with surrounding border
      for (int i=0;i<cvertex;i++)
      {
         float y;
         m_vdpoint.ElementAt(i)->get_Y(&y);
         if (y < miny)
         {
            miny = y;
            minyindex = i;
         }
      }

      float tmpx;
      m_vdpoint.ElementAt(minyindex)->get_X(&tmpx);
      const float tmpy = miny /*- 1.0f*/; // put tiny gap in to avoid errors

      // swap list around
      for (int i=0;i<cvertex/2;i++) {
         CComObject<DragPoint> * const pdp = m_vdpoint.ElementAt(i);
         m_vdpoint.ReplaceElementAt(m_vdpoint.ElementAt(cvertex-1-i), i);
         m_vdpoint.ReplaceElementAt(pdp, cvertex-1-i);
      }

      CComObject<DragPoint> *pdp;

      CComObject<DragPoint>::CreateInstance(&pdp);
      if (pdp)
      {
         pdp->AddRef();
         pdp->Init(this, m_ptable->m_left, m_ptable->m_top);
         m_vdpoint.InsertElementAt(pdp, cvertex-minyindex-1);
      }
      CComObject<DragPoint>::CreateInstance(&pdp);
      if (pdp)
      {
         pdp->AddRef();
         pdp->Init(this, m_ptable->m_right, m_ptable->m_top);
         m_vdpoint.InsertElementAt(pdp, cvertex-minyindex-1);
      }
      CComObject<DragPoint>::CreateInstance(&pdp);
      if (pdp)
      {
         pdp->AddRef();
         pdp->Init(this, m_ptable->m_right+1.0f, m_ptable->m_bottom); //!! +1 needed for whatever reason (triangulation screwed up)
         m_vdpoint.InsertElementAt(pdp, cvertex-minyindex-1);
      }
      CComObject<DragPoint>::CreateInstance(&pdp);
      if (pdp)
      {
         pdp->AddRef();
         pdp->Init(this, m_ptable->m_left, m_ptable->m_bottom);
         m_vdpoint.InsertElementAt(pdp, cvertex-minyindex-1);
      }
      CComObject<DragPoint>::CreateInstance(&pdp);
      if (pdp)
      {
         pdp->AddRef();
         pdp->Init(this, m_ptable->m_left-1.0f, m_ptable->m_top); //!! -1 needed for whatever reason (triangulation screwed up)
         m_vdpoint.InsertElementAt(pdp, cvertex-minyindex-1);
      }
      CComObject<DragPoint>::CreateInstance(&pdp);
      if (pdp)
      {
         pdp->AddRef();
         pdp->Init(this, tmpx, tmpy);
         m_vdpoint.InsertElementAt(pdp, cvertex-minyindex-1);
      }

      m_d.m_fInner = true;
   }

   return S_OK;
}

BOOL Surface::LoadToken(int id, BiffReader *pbr)
{
   if (id == FID(PIID))
   {
      pbr->GetInt((int *)pbr->m_pdata);
   }
   else if (id == FID(HTEV))
   {
      pbr->GetBool(&m_d.m_fHitEvent);
   }
   else if (id == FID(DROP))
   {
      pbr->GetBool(&m_d.m_fDroppable);
   }
   else if (id == FID(FLIP))
   {
      pbr->GetBool(&m_d.m_fFlipbook);
   }
   else if (id == FID(ISBS))
   {
      pbr->GetBool(&m_d.m_fIsBottomSolid);
   }
   else if (id == FID(CLDW))
   {
      pbr->GetBool(&m_d.m_fCollidable); 
   }
   else if (id == FID(TMON))
   {
      pbr->GetBool(&m_d.m_tdr.m_fTimerEnabled);
   }
   else if (id == FID(TMIN))
   {
      pbr->GetInt(&m_d.m_tdr.m_TimerInterval);
   }
   else if (id == FID(THRS))
   {
      pbr->GetFloat(&m_d.m_threshold);
   }
   else if (id == FID(IMAG))
   {
      pbr->GetString(m_d.m_szImage);
   }
   else if (id == FID(SIMG))
   {
      pbr->GetString(m_d.m_szSideImage);
   }
   else if (id == FID(SIMA))
   {
      pbr->GetString(m_d.m_szSideMaterial);
   }
   else if (id == FID(TOMA))
   {
      pbr->GetString(m_d.m_szTopMaterial);
   }
   else if (id == FID(SLMA))
   {
      pbr->GetString(m_d.m_szSlingShotMaterial);
   }
   else if (id == FID(HTBT))
   {
      pbr->GetFloat(&m_d.m_heightbottom);
   }
   else if (id == FID(HTTP))
   {
      pbr->GetFloat(&m_d.m_heighttop);
   }
   else if (id == FID(INNR))
   {
      //!! Deprecated, do not use anymore
      pbr->GetBool(&m_d.m_fInner);
   }
   else if (id == FID(NAME))
   {
      pbr->GetWideString((WCHAR *)m_wzName);
   }
   else if (id == FID(DSPT))
   {
      pbr->GetBool(&m_d.m_fDisplayTexture);
   }
   else if (id == FID(SLGF))
   {
      pbr->GetFloat(&m_d.m_slingshotforce);
   }
   else if (id == FID(SLTH))
   {
      pbr->GetFloat(&m_d.m_slingshot_threshold);
   }
   else if (id == FID(ELAS))
   {
      pbr->GetFloat(&m_d.m_elasticity);
   }
   else if (id == FID(WFCT))
   {
      pbr->GetFloat(&m_d.m_friction);
   }
   else if (id == FID(WSCT))
   {
      pbr->GetFloat(&m_d.m_scatter);
   }
   else if (id == FID(VSBL))
   {
      pbr->GetBool(&m_d.m_fVisible);
   }
   else if (id == FID(SLGA))
   {
      pbr->GetBool(&m_d.m_fSlingshotAnimation);
   }
   else if (id == FID(SVBL))
   {
      pbr->GetBool(&m_d.m_fSideVisible);
   }
   else
   {
      LoadPointToken(id, pbr, pbr->m_version);
      ISelect::LoadToken(id, pbr);
   }
   return fTrue;
}

HRESULT Surface::InitPostLoad()
{
   return S_OK;
}

STDMETHODIMP Surface::get_HasHitEvent(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fHitEvent);

   return S_OK;
}

STDMETHODIMP Surface::put_HasHitEvent(VARIANT_BOOL newVal)
{
   STARTUNDO

   m_d.m_fHitEvent = VBTOF(newVal);

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Surface::get_Threshold(float *pVal)
{
   *pVal = m_d.m_threshold;

   return S_OK;
}

STDMETHODIMP Surface::put_Threshold(float newVal)
{
   STARTUNDO

   m_d.m_threshold = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Surface::get_Image(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_d.m_szImage, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Surface::put_Image(BSTR newVal)
{
   STARTUNDO

   WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szImage, 32, NULL, NULL);

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Surface::get_SideMaterial(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_d.m_szSideMaterial, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Surface::put_SideMaterial(BSTR newVal)
{
   STARTUNDO

   WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szSideMaterial, 32, NULL, NULL);

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Surface::get_SlingshotMaterial(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_d.m_szSlingShotMaterial, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Surface::put_SlingshotMaterial(BSTR newVal)
{
   STARTUNDO

   WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szSlingShotMaterial, 32, NULL, NULL);

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Surface::get_ImageAlignment(ImageAlignment *pVal)
{
   // not used (anymore?)
   *pVal = ImageAlignCenter;
   return S_OK;
}

STDMETHODIMP Surface::put_ImageAlignment(ImageAlignment newVal)
{
   // not used (anymore?)
   return S_OK;
}

STDMETHODIMP Surface::get_HeightBottom(float *pVal)
{
   *pVal = m_d.m_heightbottom;

   return S_OK;
}

STDMETHODIMP Surface::put_HeightBottom(float newVal)
{
   STARTUNDO

   m_d.m_heightbottom = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Surface::get_HeightTop(float *pVal)
{
   *pVal = m_d.m_heighttop;

   return S_OK;
}

STDMETHODIMP Surface::put_HeightTop(float newVal)
{
   STARTUNDO

   m_d.m_heighttop = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Surface::get_TopMaterial(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_d.m_szTopMaterial, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Surface::put_TopMaterial(BSTR newVal)
{
   STARTUNDO

   WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szTopMaterial, 32, NULL, NULL);

   STOPUNDO

   return S_OK;
}

void Surface::GetDialogPanes(Vector<PropertyPane> *pvproppane)
{
   PropertyPane *pproppane;

   pproppane = new PropertyPane(IDD_PROP_NAME, NULL);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPWALL_VISUALS, IDS_VISUALS);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPWALL_POSITION, IDS_POSITION);
   pvproppane->AddElement(pproppane);

   m_propPhysics = new PropertyPane(IDD_PROPWALL_PHYSICS, IDS_STATE);
   pvproppane->AddElement(m_propPhysics);

   pproppane = new PropertyPane(IDD_PROP_TIMER, IDS_MISC);
   pvproppane->AddElement(pproppane);
}

void Surface::GetPointDialogPanes(Vector<PropertyPane> *pvproppane)
{
   PropertyPane *pproppane;

   pproppane = new PropertyPane(IDD_PROPPOINT_VISUALSWTEX, IDS_VISUALS);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPPOINT_POSITION, IDS_POSITION);
   pvproppane->AddElement(pproppane);
}

STDMETHODIMP Surface::get_CanDrop(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fDroppable);

   return S_OK;
}

STDMETHODIMP Surface::put_CanDrop(VARIANT_BOOL newVal)
{
   STARTUNDO

   m_d.m_fDroppable = VBTOF(newVal);

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Surface::get_FlipbookAnimation(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fFlipbook);

   return S_OK;
}

STDMETHODIMP Surface::put_FlipbookAnimation(VARIANT_BOOL newVal)
{
   STARTUNDO

   m_d.m_fFlipbook = VBTOF(newVal);

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Surface::get_IsBottomSolid(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fIsBottomSolid);

   return S_OK;
}

STDMETHODIMP Surface::put_IsBottomSolid(VARIANT_BOOL newVal)
{
   STARTUNDO

   m_d.m_fIsBottomSolid = VBTOF(newVal);

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Surface::get_IsDropped(VARIANT_BOOL *pVal)
{
   if (!g_pplayer)
      return E_FAIL;

   *pVal = (VARIANT_BOOL)FTOVB(m_fIsDropped);

   return S_OK;
}

STDMETHODIMP Surface::put_IsDropped(VARIANT_BOOL newVal)
{
   if (!g_pplayer || !m_d.m_fDroppable)
      return E_FAIL;

   const BOOL fNewVal = VBTOF(newVal);

   if (m_fIsDropped != fNewVal)
   {
      m_fIsDropped = fNewVal;

      for (unsigned i=0; i<m_vhoDrop.size(); i++)
         m_vhoDrop[i]->m_fEnabled = !m_fIsDropped && m_d.m_fCollidable; //disable hit on enities composing the object 
   }

   return S_OK;
}

STDMETHODIMP Surface::get_DisplayTexture(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fDisplayTexture);

   return S_OK;
}

STDMETHODIMP Surface::put_DisplayTexture(VARIANT_BOOL newVal)
{
   STARTUNDO

   m_d.m_fDisplayTexture = VBTOF(newVal);

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Surface::get_SlingshotStrength(float *pVal)
{
   *pVal = m_d.m_slingshotforce*(float)(1.0/10.0);

   // Force value divided by 10 to make it look more like flipper strength value

   return S_OK;
}

STDMETHODIMP Surface::put_SlingshotStrength(float newVal)
{
   STARTUNDO

   m_d.m_slingshotforce = newVal*10.0f;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Surface::get_Elasticity(float *pVal)
{
   *pVal = m_d.m_elasticity;

   return S_OK;
}

STDMETHODIMP Surface::put_Elasticity(float newVal)
{
   STARTUNDO

   m_d.m_elasticity = newVal;

   STOPUNDO

   return S_OK;
}


STDMETHODIMP Surface::get_Friction(float *pVal)
{
   *pVal = m_d.m_friction;

   return S_OK;
}

STDMETHODIMP Surface::put_Friction(float newVal)
{
   STARTUNDO

   if (newVal > 1.0f) newVal = 1.0f;
      else if (newVal < 0.f) newVal = 0.f;

   m_d.m_friction = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Surface::get_Scatter(float *pVal)
{
   *pVal = m_d.m_scatter;

   return S_OK;
}

STDMETHODIMP Surface::put_Scatter(float newVal)
{
   STARTUNDO

   m_d.m_scatter = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Surface::get_Visible(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fVisible);

   return S_OK;
}

STDMETHODIMP Surface::put_Visible(VARIANT_BOOL newVal)
{
   STARTUNDO

   m_d.m_fVisible = VBTOF(newVal);

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Surface::get_SideImage(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_d.m_szSideImage, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Surface::put_SideImage(BSTR newVal)
{
   STARTUNDO

   WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szSideImage, 32, NULL, NULL);

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Surface::get_Disabled(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_fDisabled);

   return S_OK;
}

STDMETHODIMP Surface::put_Disabled(VARIANT_BOOL newVal)
{
   STARTUNDO

   m_fDisabled = VBTOF(newVal);

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Surface::get_SideVisible(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fSideVisible);

   return S_OK;
}

STDMETHODIMP Surface::put_SideVisible(VARIANT_BOOL newVal)
{
   STARTUNDO

   m_d.m_fSideVisible = VBTOF(newVal);

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Surface::get_Collidable(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fCollidable);

   return S_OK;
}

STDMETHODIMP Surface::put_Collidable(VARIANT_BOOL newVal)
{
   const BOOL fNewVal = VBTOF(newVal);	

   STARTUNDO

   m_d.m_fCollidable = VBTOF(fNewVal);

   for (unsigned i=0; i<m_vhoCollidable.size(); i++)
   {
      if (m_d.m_fDroppable) m_vhoCollidable[i]->m_fEnabled = fNewVal && !m_fIsDropped;
      else m_vhoCollidable[i]->m_fEnabled = VBTOF(fNewVal); //copy to hit checking on enities composing the object 
   }	

   STOPUNDO

   return S_OK;
}


/////////////////////////////////////////////////////////////

STDMETHODIMP Surface::get_SlingshotThreshold(float *pVal)
{
   *pVal = m_d.m_slingshot_threshold;

   return S_OK;
}

STDMETHODIMP Surface::put_SlingshotThreshold(float newVal)
{
   STARTUNDO

   m_d.m_slingshot_threshold = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Surface::get_SlingshotAnimation(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fSlingshotAnimation);

   return S_OK;
}

STDMETHODIMP Surface::put_SlingshotAnimation(VARIANT_BOOL newVal)
{
   STARTUNDO

   m_d.m_fSlingshotAnimation = VBTOF(newVal);

   STOPUNDO

   return S_OK;
}

void Surface::UpdatePropertyPanes()
{
    if ( m_propPhysics==NULL )
        return;

    if ( !m_d.m_fCollidable )
    {
        EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd,3), FALSE);
        EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd,4), FALSE);
        EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd,14), FALSE);
        EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd,111), FALSE);
        EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd,15), FALSE);
        EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd,114), FALSE);
        EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd,115), FALSE);
        EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd,116), FALSE);
    }
    else
    {
        EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd,3), TRUE);
        if ( m_d.m_fHitEvent )
            EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd,4), TRUE);
        else
            EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd,4), FALSE);

        EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd,14), TRUE);
        EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd,111), TRUE);
        EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd,15), TRUE);
        EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd,114), TRUE);
        EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd,115), TRUE);
        EnableWindow(GetDlgItem(m_propPhysics->dialogHwnd,116), TRUE);
    }
}
